// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <ShlObj.h>
#include <ShlObj_core.h>
#include "includes.hpp"
#include "utils\ctx.hpp"
#include "utils\recv.h"
#include "utils\imports.h"
#include "skinchanger\SkinChanger.h"
#include "byte\fonts.h"
#include "cheats\misc\prediction_system.h"
#include "utils\multithreading\threading.h"
#include "cheats\networking\networking.h"

__forceinline void setup_addresses();
__forceinline void setup_convars();
__forceinline void setup_directory();
__forceinline void setup_netvars();
__forceinline void setup_skins();
__forceinline void setup_renders();
__forceinline void setup_materials();
__forceinline void setup_hooks();
__forceinline void setup_steam();

void main(PVOID base)
{
	DWORD undefeated;
	AddFontMemResourceEx(reinterpret_cast<void*>(undefeated_font), sizeof(undefeated_font), nullptr, &undefeated);

	/* wait until serverbrowser.dll has loaded in (it's the last dll to be loaded in CS:GO) */

	while (!IFH(GetModuleHandle)(crypt_str("serverbrowser.dll")))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	/* SETUP START-UP. */

	setup_addresses();
	setup_convars();
	setup_directory();

	/* DO CONFIGURATION. */

	cfg_manager->setup();

	/* SETUP UTENSILS. */

	key_binds::get().initialize_key_binds();

	/* SETUP SYSTEM. */

	setup_renders();
	setup_netvars();
	setup_skins();
	setup_sounds();

	/* SETUP UTILITY. */

	setup_hooks();
	setup_steam();

	/* BUILD SEED TABLE. */

	networking::get().build_seed_table();

	/* FINISHING. */

	Netvars::Netvars();
	setup_materials();

	return ExitThread(EXIT_SUCCESS);
}

__forceinline void setup_addresses()
{
	/* RETURN ADDRESS CHECK BYPASS. */

	const char* modules[]{ crypt_str("client.dll"), crypt_str("engine.dll"), crypt_str("server.dll"), crypt_str("studiorender.dll"), crypt_str("materialsystem.dll") };
	long long patch = 0x69690004C201B0;
	for (auto base : modules) WriteProcessMemory(GetCurrentProcess(), (LPVOID)util::FindSignature(base, crypt_str("55 8B EC 56 8B F1 33 C0 57 8B 7D 08")), &patch, 5, 0);

	/* SET FOR SIGNATURES AND INDEXES. */

	g_ctx.indexes =
	{
		17, // 0 -- get_pred_desc_map
		158, // 1 -- is_player
		75, // 2 -- set_model_index
		461, // 3 -- get_csweapon_info
		483, // 4 -- get_inaccuracy
		453, // 5 -- get_spread
		484, // 6 -- update_accuracy_penalty
		247, // 7 -- send_viewmodel_matching_sequence
		27, // 8 -- chat_print
		123, // 9 -- draw_filled_rect_array
		169 // 10 -- get_shoot_position ( interpolated )
	};

	/* SET FOR HOOKING. */

	g_ctx.addresses.hooks.getforeignfallbackfontname = util::FindSignature(crypt_str("vguimatsurface.dll"), crypt_str("80 3D ? ? ? ? ? 74 06 B8"));
	g_ctx.addresses.hooks.setupbones = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 8B")); // C_BaseAnimating::SetupBones()
	g_ctx.addresses.hooks.doextraboneprocessing = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 81 ? ? ? ? ? 53 56 8B F1 57 89 74 24 1C")); // C_CSPlayer::DoExtraBoneProcessing()
	g_ctx.addresses.hooks.standardblendingrules = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85")); // C_BaseAnimating::StandardBlendingRules()
	g_ctx.addresses.hooks.updateclientsideanimations = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B F1 80 BE ? ? 00 00 00 74 36")); // UpdateClientSideAnimations()
	g_ctx.addresses.hooks.physicssimulate = util::FindSignature(crypt_str("client.dll"), crypt_str("56 8B F1 8B ? ? ? ? ? 83 F9 FF 74 23")); // C_BasePlayer::PhysicsSimulate
	g_ctx.addresses.hooks.modifyeyeposition = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14")); // CCSGOPlayerAnimState::ModifyEyePosition()
	g_ctx.addresses.hooks.calcviewbob = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9")); // CBasePlayer::CalcViewBob()
	g_ctx.addresses.hooks.buildtransformations = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 28 8B")); // C_BaseAnimating::BuildTransformations()
	g_ctx.addresses.hooks.geteyeangles = util::FindSignature(crypt_str("client.dll"), crypt_str("56 8B F1 85 F6 74 32"));
	g_ctx.addresses.hooks.shouldskipanimationframe = util::FindSignature(crypt_str("client.dll"), crypt_str("57 8B F9 8B 07 8B ? ? ? ? ? FF D0 84 C0 75 02")); // C_BaseAnimating::ShouldSkipAnimationFrame()
	g_ctx.addresses.hooks.checkfilecrcswithserver = util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 81 ? ? ? ? ? 53 8B D9 89 5D F8")); // CClientState::CheckFileCRCsWithServer()
	g_ctx.addresses.hooks.processinterpolatedlist = util::FindSignature(crypt_str("client.dll"), crypt_str("0F ? ? ? ? ? ? 3D ? ? ? ? 74 3F")); // C_BaseEntity::ProcessInterpolatedList()
	g_ctx.addresses.hooks.filesystem = util::FindSignature(crypt_str("engine.dll"), crypt_str("8B 0D ? ? ? ? 8D 95 ? ? ? ? 6A 00 C6")) + 0x2;
	g_ctx.addresses.hooks.clmove = util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 81 EC 64 01 00 00 53 56 8A F9"));
	g_ctx.addresses.hooks.checkforsequencechange = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 53 8B 5D 08 56 8B F1 57 85"));
	g_ctx.addresses.hooks.estimateabsvelocity = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 0C 56 8B F1 85 F6")) + 0x2 / 4;
	g_ctx.addresses.hooks.calcview = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 14 53 56 57 FF 75 18")); // CalculateView()
	g_ctx.addresses.hooks.processmovement = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B 75 08 57 8B F9 C7"));
	g_ctx.addresses.hooks.setupaliveloop = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B 71 60 83 BE 9C 29 00 00 00 0F 84 9C"));
	
	/* SET FOR UTILITY. */

	g_ctx.addresses.utility.direct3ddevice9 = util::FindSignature(crypt_str("shaderapidx9.dll"), crypt_str("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 0x1;
	g_ctx.addresses.utility.viewrenderbeams = util::FindSignature(crypt_str("client.dll"), crypt_str("A1 ? ? ? ? 56 8B F1 B9 ? ? ? ? FF 50 08")) + 0x1;
	g_ctx.addresses.utility.glowobjectmanager = util::FindSignature(crypt_str("client.dll"), crypt_str("0F 11 05 ?? ?? ?? ?? 83 C8 01")) + 0x3;
	g_ctx.addresses.utility.movehelper = util::FindSignature(crypt_str("client.dll"), crypt_str("8B 0D ?? ?? ?? ?? 8B 46 08 68")) + 0x2;
	g_ctx.addresses.utility.input = util::FindSignature(crypt_str("client.dll"), crypt_str("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10")) + 0x1;
	g_ctx.addresses.utility.playerresource = util::FindSignature(crypt_str("client.dll"), crypt_str("8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7")) + 0x2;
	g_ctx.addresses.utility.gamerules = util::FindSignature(crypt_str("client.dll"), crypt_str("A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6")) + 0x1;
	g_ctx.addresses.utility.postprocessing = util::FindSignature(crypt_str("client.dll"), crypt_str("80 3D ? ? ? ? ? 53 56 57 0F 85")) + 0x2;
	g_ctx.addresses.utility.ccsplayerrenderablevftable = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C")) + 0x4E;

	/* SET FOR RETURN ADDRESS. */

	g_ctx.addresses.return_to.get_client_model_renderable = util::FindSignature(crypt_str("client.dll"), crypt_str("85 C0 75 54 8B 0D ? ? ? ?"));
	g_ctx.addresses.return_to.cam_think = util::FindSignature(crypt_str("client.dll"), crypt_str("85 C0 75 30 38 86"));
	g_ctx.addresses.return_to.accumulate_layers = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 75 0D F6 87"));
	g_ctx.addresses.return_to.setup_velocity = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0"));
	g_ctx.addresses.return_to.extrapolation = util::FindSignature(crypt_str("client.dll"), crypt_str("0F B6 0D ? ? ? ? 84 C0 0F 44 CF 88 0D ? ? ? ? A0"));
	g_ctx.addresses.return_to.maintain_sequence_transitions = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 74 17 8B 87"));
	g_ctx.addresses.return_to.setupbones_timing = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05"));
	g_ctx.addresses.return_to.loadout_allowed = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 75 05 B0 01 5F"));
	g_ctx.addresses.return_to.cbaseplayer_eyeposition_and_vectors = util::FindSignature(crypt_str("client.dll"), crypt_str("8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5"));
	g_ctx.addresses.return_to.cbaseplayer_thirdperson_yaw_vectors = util::FindSignature(crypt_str("client.dll"), crypt_str("8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?"));
	g_ctx.addresses.return_to.cbaseplayer_thirdperson_pitch_vectors = util::FindSignature(crypt_str("client.dll"), crypt_str("F3 0F 10 55 ? 51 8B 8E ? ? ? ?"));

	/* -------------------- */

	g_ctx.addresses.allow_extrapolation = util::FindSignature(crypt_str("client.dll"), crypt_str("A2 ? ? ? ? 8B 45 E8")) + 0x1;
	g_ctx.addresses.smoke_count = util::FindSignature(crypt_str("client.dll"), crypt_str("A3 ? ? ? ? 57 8B CB")) + 0x1;
	g_ctx.addresses.clear_notices = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 0C 53 56 8B 71")); // ClearDeathNotices()
	g_ctx.addresses.set_abs_angles = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1")); // SetAbsAngles()
	g_ctx.addresses.set_abs_origin = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8 ? ? ? ? 8B 7D")); // C_BaseEntity::SetAbsOrigin()
	g_ctx.addresses.set_abs_velocity = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3")); // UNUSED
	g_ctx.addresses.lookup_bone = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 56 8B F1 57 83 ? ? ? ? ? ? 75")); // LookupBone()
	g_ctx.addresses.attachment_helper = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 48 53 8B 5D")); // C_BaseAnimating::SetupBones_AttachmentHelper()
	g_ctx.addresses.has_c4 = util::FindSignature(crypt_str("client.dll"), crypt_str("56 8B F1 85 F6 74 31")); // C_CSPlayer::HasC4()
	g_ctx.addresses.invalidate_bone_cache = util::FindSignature(crypt_str("client.dll"), crypt_str("80 ? ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 ? ? ? ? ? ? ? ? ? 89 ? ? ? ? ? C3")); // C_BaseAnimating::InvalidateBoneCache()
	g_ctx.addresses.invalidate_physics_recursive = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3")); // C_CSPlayer::InvalidatePhysicsRecursive()
	g_ctx.addresses.unknown_function = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 57 8B F9 8B ? ? ? ? ? 8B C6")); // C_BaseEntity::CheckHasThinkFunction()
	g_ctx.addresses.physics_run_think = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 10 53 56 57 8B F9 8B ? ? ? ? ? C1")); // physics_run_think()
	g_ctx.addresses.sequence_activity = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 8B 5D 08 56 8B F1 83")); // GetSequenceActivity()
	g_ctx.addresses.prediction_random_seed = util::FindSignature(crypt_str("client.dll"), crypt_str("8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04")) + 0x2;
	g_ctx.addresses.prediction_player = util::FindSignature(crypt_str("client.dll"), crypt_str("89 35 ? ? ? ? F3 0F 10 48 20")) + 0x2;
	g_ctx.addresses.post_think_v_physics = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 81 ? ? ? ? ? 53 8B D9 56 57 83 ? ? ? ? ? ? 0F")); // PostThinkVPhysics()
	g_ctx.addresses.simulate_player_simulated_entities = util::FindSignature(crypt_str("client.dll"), crypt_str("56 8B F1 57 8B ? ? ? ? ? 83 EF 01 78 74")); // SimulatePlayerSimulatedEntities()
	g_ctx.addresses.rank_reveal = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 A1 ? ? ? ? 85 C0 75 37"));
	g_ctx.addresses.clip_ray_to_hitbox = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F")); // ClipRayToHitbox()
	g_ctx.addresses.clip_trace_to_players = util::FindSignature(crypt_str("client.dll"), crypt_str("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC D8 ? ? ? 0F 57 C9"));
	g_ctx.addresses.write_user_cmd = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 51 53 56 8B D9")); // WriteUserCmd()
	g_ctx.addresses.clmove_choke_clamp = util::FindSignature(crypt_str("engine.dll"), crypt_str("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC")) + 0x1;
	g_ctx.addresses.set_clantag = util::FindSignature(crypt_str("engine.dll"), crypt_str("53 56 57 8B DA 8B F9 FF")); // ChangeClantag()
	g_ctx.addresses.find_hud_element = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39")); // FindHudElement()
	g_ctx.addresses.game_hud = util::FindSignature(crypt_str("client.dll"), crypt_str("B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 5D 08")) + 0x1;
	g_ctx.addresses.is_breakable = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B F1 85 F6 74 68")); // IsEntityBreakable()
	g_ctx.addresses.trace_filter_simple = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3D;
	g_ctx.addresses.trace_filter_skip_two_entities = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 81 EC BC 00 00 00 56 8B F1 8B 86")) + 0x226;
	g_ctx.addresses.load_skybox = util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 81 ? ? ? ? ? 56 57 8B F9 C7")); // R_LoadNamedSkys()
	g_ctx.addresses.line_goes_through_smoke = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0")); // LineGoesThroughSmoke()
	g_ctx.addresses.update_clientside_animations = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B F1 80 ? ? ? ? ? ? 74 36")); //C_CSPlayer::UpdateClientSideAnimation()
	g_ctx.addresses.reset_animation_state = util::FindSignature(crypt_str("client.dll"), crypt_str("56 6A 01 68 ? ? ? ? 8B F1")); // CCSGOPlayerAnimState::Reset()
	g_ctx.addresses.update_animation_state = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3")); // CCSGOPlayerAnimState::Update()
	g_ctx.addresses.create_animation_state = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 8B F1 B9 ? ? ? ? C7")); // CreateAnimationState()
	g_ctx.addresses.equip = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9"));
	g_ctx.addresses.enable_bone_cache_invalidation = util::FindSignature(crypt_str("client.dll"), crypt_str("C6 05 ? ? ? ? ? 89 47 70")) + 2;
	g_ctx.addresses.get_layer_sequence_cycle_rate = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 57 FF 75 0C 8B 7D 08 8B F1 57 E8")); // C_CSPlayer::GetLayerSequenceCycleRate()
	g_ctx.addresses.key_values = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 33 C0 C7 45"));
	g_ctx.addresses.load_from_buffer = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89"));
	g_ctx.addresses.last_command = util::FindSignature(crypt_str("client.dll"), crypt_str("8D 8E ? ? ? ? 89 5C 24 3C")) + 0x2;
	g_ctx.addresses.is_self_animating = util::FindSignature(crypt_str("client.dll"), crypt_str("80 ? ? ? ? ? ? 74 03 B0 01 C3 0F"));
	g_ctx.addresses.get_shotgun_spread = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 10 56 8B 75 08 8D")); // GetShotgunSpread()
}

__forceinline void setup_convars()
{
	/* GAME RELATED. */

	g_ctx.convars.developer = m_cvar()->FindVar(crypt_str("developer"));

	g_ctx.convars.sensitivity = m_cvar()->FindVar(crypt_str("sensitivity"));

	g_ctx.convars.m_pitch = m_cvar()->FindVar(crypt_str("m_pitch"));
	g_ctx.convars.m_yaw = m_cvar()->FindVar(crypt_str("m_yaw"));

	/* DRAWING RELATED. */

	g_ctx.convars.r_drawmodelstatsoverlay = m_cvar()->FindVar(crypt_str("r_drawmodelstatsoverlay"));
	g_ctx.convars.r_3dsky = m_cvar()->FindVar(crypt_str("r_3dsky"));
	g_ctx.convars.r_drawspecificstaticprop = m_cvar()->FindVar(crypt_str("r_drawspecificstaticprop"));
	g_ctx.convars.r_shadows = m_cvar()->FindVar(crypt_str("r_shadows"));
	g_ctx.convars.r_modelAmbientMin = m_cvar()->FindVar(crypt_str("r_modelAmbientMin"));
	g_ctx.convars.r_jiggle_bones = m_cvar()->FindVar(crypt_str("r_jiggle_bones"));

	/* CONSOLE RELATED. */

	g_ctx.convars.con_filter_enable = m_cvar()->FindVar(crypt_str("con_filter_enable"));
	g_ctx.convars.con_filter_text = m_cvar()->FindVar(crypt_str("con_filter_text"));

	/* AUTOWALL RELATED. */

	g_ctx.convars.ff_damage_reduction_bullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
	g_ctx.convars.ff_damage_bullet_penetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

	/* MATERIAL RELATED. */

	g_ctx.convars.mat_fullbright = m_cvar()->FindVar(crypt_str("mat_fullbright"));

	/* VIEWMODEL RELATED. */

	g_ctx.convars.viewmodel_fov = m_cvar()->FindVar(crypt_str("viewmodel_fov"));
	g_ctx.convars.viewmodel_offset_x = m_cvar()->FindVar(crypt_str("viewmodel_offset_x"));
	g_ctx.convars.viewmodel_offset_y = m_cvar()->FindVar(crypt_str("viewmodel_offset_y"));
	g_ctx.convars.viewmodel_offset_z = m_cvar()->FindVar(crypt_str("viewmodel_offset_z"));

	/* FOG RELATED. */

	g_ctx.convars.fog_override = m_cvar()->FindVar(crypt_str("fog_override"));
	g_ctx.convars.fog_start = m_cvar()->FindVar(crypt_str("fog_start"));
	g_ctx.convars.fog_end = m_cvar()->FindVar(crypt_str("fog_end"));
	g_ctx.convars.fog_maxdensity = m_cvar()->FindVar(crypt_str("fog_maxdensity"));
	g_ctx.convars.fog_color = m_cvar()->FindVar(crypt_str("fog_color"));

	/* WEAPON RELATED. */

	g_ctx.convars.mp_c4timer = m_cvar()->FindVar(crypt_str("mp_c4timer"));
	g_ctx.convars.mp_damage_scale_ct_head = m_cvar()->FindVar(crypt_str("mp_damage_scale_ct_head"));
	g_ctx.convars.mp_damage_scale_ct_body = m_cvar()->FindVar(crypt_str("mp_damage_scale_ct_body"));
	g_ctx.convars.mp_damage_scale_t_head = m_cvar()->FindVar(crypt_str("mp_damage_scale_t_head"));
	g_ctx.convars.mp_damage_scale_t_body = m_cvar()->FindVar(crypt_str("mp_damage_scale_t_body"));

	g_ctx.convars.weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));
	g_ctx.convars.weapon_accuracy_nospread = m_cvar()->FindVar(crypt_str("weapon_accuracy_nospread"));
	g_ctx.convars.weapon_debug_spread_show = m_cvar()->FindVar(crypt_str("weapon_debug_spread_show"));
	g_ctx.convars.weapon_molotov_maxdetonateslope = m_cvar()->FindVar(crypt_str("weapon_molotov_maxdetonateslope"));
	g_ctx.convars.weapon_accuracy_shotgun_spread_patterns = m_cvar()->FindVar(crypt_str("weapon_accuracy_shotgun_spread_patterns"));

	g_ctx.convars.molotov_throw_detonate_time = m_cvar()->FindVar(crypt_str("molotov_throw_detonate_time"));

	/* CLIENT RELATED. */

	g_ctx.convars.cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
	g_ctx.convars.cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));
	g_ctx.convars.cl_upspeed = m_cvar()->FindVar(crypt_str("cl_upspeed"));
	g_ctx.convars.cl_updaterate = m_cvar()->FindVar(crypt_str("cl_updaterate"));
	g_ctx.convars.cl_interp_ratio = m_cvar()->FindVar(crypt_str("cl_interp_ratio"));
	g_ctx.convars.cl_interp = m_cvar()->FindVar(crypt_str("cl_interp"));
	g_ctx.convars.cl_mouseenable = m_cvar()->FindVar(crypt_str("cl_mouseenable"));

	g_ctx.convars.cl_csm_static_prop_shadows = m_cvar()->FindVar(crypt_str("cl_csm_static_prop_shadows"));
	g_ctx.convars.cl_csm_shadows = m_cvar()->FindVar(crypt_str("cl_csm_shadows"));
	g_ctx.convars.cl_csm_world_shadows = m_cvar()->FindVar(crypt_str("cl_csm_world_shadows"));
	g_ctx.convars.cl_foot_contact_shadows = m_cvar()->FindVar(crypt_str("cl_foot_contact_shadows"));
	g_ctx.convars.cl_csm_viewmodel_shadows = m_cvar()->FindVar(crypt_str("cl_csm_viewmodel_shadows"));
	g_ctx.convars.cl_csm_rope_shadows = m_cvar()->FindVar(crypt_str("cl_csm_rope_shadows"));
	g_ctx.convars.cl_csm_sprite_shadows = m_cvar()->FindVar(crypt_str("cl_csm_sprite_shadows"));

	/* SERVER RELATED. */

	g_ctx.convars.sv_cheats = m_cvar()->FindVar(crypt_str("sv_cheats"));
	g_ctx.convars.sv_minupdaterate = m_cvar()->FindVar(crypt_str("sv_minupdaterate"));
	g_ctx.convars.sv_maxupdaterate = m_cvar()->FindVar(crypt_str("sv_maxupdaterate"));
	g_ctx.convars.sv_client_min_interp_ratio = m_cvar()->FindVar(crypt_str("sv_client_min_interp_ratio"));
	g_ctx.convars.sv_client_max_interp_ratio = m_cvar()->FindVar(crypt_str("sv_client_max_interp_ratio"));
	g_ctx.convars.sv_skyname = m_cvar()->FindVar(crypt_str("sv_skyname"));
	g_ctx.convars.sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));
	g_ctx.convars.sv_gravity = m_cvar()->FindVar(crypt_str("sv_gravity"));
	g_ctx.convars.sv_jump_impulse = m_cvar()->FindVar(crypt_str("sv_jump_impulse"));
	g_ctx.convars.sv_clip_penetration_traces_to_players = m_cvar()->FindVar(crypt_str("sv_clip_penetration_traces_to_players"));

	/* ------------------- */

	g_ctx.convars.weapon_debug_spread_show->m_nFlags &= ~FCVAR_CHEAT;

	g_ctx.convars.viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
	g_ctx.convars.viewmodel_offset_x->m_fnChangeCallbacks.m_Size = 0;
	g_ctx.convars.viewmodel_offset_y->m_fnChangeCallbacks.m_Size = 0;
	g_ctx.convars.viewmodel_offset_z->m_fnChangeCallbacks.m_Size = 0;
}

__forceinline void setup_directory()
{
	static TCHAR path[MAX_PATH]; std::string folder;
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path);
	folder = std::string(path) + crypt_str("\\naval\\");
	CreateDirectory(folder.c_str(), 0);
}

__forceinline void setup_netvars()
{
	if (!netvars::get().tables.empty())
		netvars::get().tables.clear();

	auto client = m_client()->GetAllClasses();

	if (!client)
		return;

	while (client)
	{
		auto recvTable = client->m_pRecvTable;

		if (recvTable)
			netvars::get().tables.emplace(std::string(client->m_pNetworkName), recvTable);

		client = client->m_pNext;
	}
}

__forceinline void setup_skins()
{
	auto items = std::ifstream(crypt_str("csgo/scripts/items/items_game_cdn.txt"));
	auto gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();
	memory.initialize();

	for (auto i = 0; i <= memory.itemSchema()->paintKits.lastElement; i++)
	{
		auto paintKit = memory.itemSchema()->paintKits.memory[i].value;

		if (paintKit->id == 9001)
			continue;

		auto itemName = m_localize()->FindSafe(paintKit->itemName.buffer + 1);
		auto itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

		if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
		{
			if (paintKit->id < 10000)
			{
				if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } + '='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } + '=') == std::string::npos)
				{
					if (auto weaponName = gameItems.rfind(crypt_str("weapon_"), pos); weaponName != std::string::npos)
					{
						name.back() = ' ';
						name += '(' + gameItems.substr(weaponName + 7, pos - weaponName - 7) + ')';
					}
				}
				SkinChanger::skinKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
			else
			{
				std::string_view gloveName{ paintKit->name.buffer };
				name.back() = ' ';
				name += '(' + std::string{ gloveName.substr(0, gloveName.find('_')) } + ')';
				SkinChanger::gloveKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
		}
	}

	std::sort(SkinChanger::skinKits.begin(), SkinChanger::skinKits.end());
	std::sort(SkinChanger::gloveKits.begin(), SkinChanger::gloveKits.end());
}

__forceinline void setup_renders()
{
	static auto create_font = [](const char* name, int size, int weight, DWORD flags) -> vgui::HFont
	{
		g_ctx.last_font_name = name;

		auto font = m_surface()->FontCreate();
		m_surface()->SetFontGlyphSet(font, name, size, weight, NULL, NULL, flags);

		return font;
	};

	fonts[LOGS] = create_font(crypt_str("Lucida Console"), 12, FW_MEDIUM, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
	fonts[ESP] = create_font(crypt_str("Small Fonts"), 8, FW_NORMAL, FONTFLAG_OUTLINE | FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
	fonts[NAME] = create_font(crypt_str("Verdana"), 12, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[WEAPON_ICON_FONT] = create_font(crypt_str("undefeated"), 14, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[INDICATORFONT] = create_font(crypt_str("Verdana"), 25, FW_HEAVY, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[DAMAGE_MARKER] = create_font(crypt_str("Verdana"), 16, FW_HEAVY, FONTFLAG_OUTLINE);
	fonts[WATERMARK] = create_font(crypt_str("Verdana"), 13, FW_MEDIUM, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
	fonts[SMALL_ESP] = create_font(crypt_str("Small Fonts"), 8, FW_NORMAL, FONTFLAG_OUTLINE | FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);

	if (!g_ctx.last_font_name.empty())
		g_ctx.last_font_name.clear();

	m_engine()->GetScreenSize(g_ctx.globals.screen_width, g_ctx.globals.screen_height);
}

void setup_materials()
{
	g_ctx.chams.regular =
		util::create_material(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#"));

	g_ctx.chams.metallic =
		util::create_material(true, crypt_str(R"#("VertexLitGeneric" 
			{
				"$basetexture"				"vgui/white" 
				"$ignorez"					"0" 
				"$envmap"					"env_cubemap" 
				"$normalmapalphaenvmapmask" "1" 
				"$envmapcontrast"			"1" 
				"$nofog"					"1" 
				"$model"					"1" 
				"$nocull" 					"0" 
				"$selfillum" 				"1" 
				"$halflambert"				"1" 
				"$znearer" 					"0" 
				"$flat" 					"1"
		        "$wireframe"				"0"
			}
		)#"));

	g_ctx.chams.flat =
		util::create_material(false, crypt_str(R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#"));

	g_ctx.chams.glow =
		util::create_material(true, crypt_str(R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"0.8" 
			}
		)#"));

	g_ctx.chams.animated = 
		util::create_material(true, crypt_str(R"#("VertexLitGeneric"
		    {
		        "$basetexture"				"dev/zone_warning"
		        "$additive"					"1"
		        "$envmap"					"editor/cube_vertigo"
		        "$envmaptint"				"[0 0.5 0.55]"
		        "$envmapfresnel"			"1"
		        "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
		        "$alpha"					"1"
   
		        Proxies
		        {
		            TextureScroll
		            {
		                "texturescrollvar"			"$baseTextureTransform"
		                "texturescrollrate"			"0.25"
		                "texturescrollangle"		"270"
		            }
		            Sine
		            {
		                "sineperiod"				"2"
		                "sinemin"					"0.1"
		                "resultVar"					"$envmapfresnelminmaxexp[1]"
		            }
		        }
		    }
		)#"));

	g_ctx.chams.pulse = m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/dogtags/dogtags_outline"), nullptr);
	g_ctx.chams.crystal = m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/crystal_clear"), nullptr);
	g_ctx.chams.glass = m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr);
	g_ctx.chams.circuit = m_materialsystem()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr);
	g_ctx.chams.golden = m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/wildfire_gold/wildfire_gold_detail"), nullptr);
}

__forceinline void setup_hooks()
{
	while (!(INIT::Window = IFH(FindWindow)(crypt_str("Valve001"), nullptr)))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	INIT::OldWindow = (WNDPROC)IFH(SetWindowLongPtr)(INIT::Window, GWL_WNDPROC, (LONG_PTR)hooks::Hooked_WndProc);

	hooks::client_hook = new vmthook(reinterpret_cast<DWORD**>(m_client()));
	hooks::panel_hook = new vmthook(reinterpret_cast<DWORD**>(m_panel())); //-V1032
	hooks::clientmode_hook = new vmthook(reinterpret_cast<DWORD**>(m_clientmode()));
	hooks::inputinternal_hook = new vmthook(reinterpret_cast<DWORD**>(m_inputinternal())); //-V114
	hooks::engine_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()));
	hooks::renderview_hook = new vmthook(reinterpret_cast<DWORD**>(m_renderview()));
	hooks::materialsys_hook = new vmthook(reinterpret_cast<DWORD**>(m_materialsystem())); //-V1032
	hooks::modelrender_hook = new vmthook(reinterpret_cast<DWORD**>(m_modelrender()));
	hooks::prediction_hook = new vmthook(reinterpret_cast<DWORD**>(m_prediction()));
	hooks::surface_hook = new vmthook(reinterpret_cast<DWORD**>(m_surface()));
	hooks::bspquery_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()->GetBSPTreeQuery()));
	hooks::prediction_hook = new vmthook(reinterpret_cast<DWORD**>(m_prediction())); //-V1032
	hooks::trace_hook = new vmthook(reinterpret_cast<DWORD**>(m_trace()));
	hooks::directx_hook = new vmthook(reinterpret_cast<DWORD**>(m_device()));
	hooks::r_drawmodelstatsoverlay_hook = new vmthook(reinterpret_cast<DWORD**>(g_ctx.convars.r_drawmodelstatsoverlay));
	hooks::sv_cheats_hook = new vmthook(reinterpret_cast<DWORD**>(g_ctx.convars.sv_cheats));
	hooks::filesystem_hook = new vmthook(reinterpret_cast<DWORD**>(g_ctx.addresses.hooks.filesystem));
	
	/* DETOUR HOOKS. */

	hooks::original_getforeignfallbackfontname = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.getforeignfallbackfontname, (PBYTE)hooks::hooked_getforeignfallbackfontname); //-V206
	hooks::original_setupbones = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.setupbones, (PBYTE)hooks::hooked_setupbones); //-V206
	hooks::original_doextrabonesprocessing = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.doextraboneprocessing, (PBYTE)hooks::hooked_doextrabonesprocessing); //-V206
	hooks::original_standardblendingrules = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.standardblendingrules, (PBYTE)hooks::hooked_standardblendingrules); //-V206
	hooks::original_updateclientsideanimation = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.updateclientsideanimations, (PBYTE)hooks::hooked_updateclientsideanimation); //-V206
	hooks::original_estimateabsvelocity = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.estimateabsvelocity, (PBYTE)hooks::hooked_estimateabsvelocity);
	hooks::original_physicssimulate = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.physicssimulate, (PBYTE)hooks::hooked_physicssimulate);
	hooks::original_calcviewmodelbob = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.calcviewbob, (PBYTE)hooks::hooked_calcviewmodelbob);
	hooks::original_buildtransformations = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.buildtransformations, (PBYTE)hooks::hooked_buildtransformations);
	hooks::original_geteyeangles = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.geteyeangles, (PBYTE)hooks::hooked_geteyeangles);
	hooks::original_calcview = (DWORD)DetourFunction((PBYTE)g_ctx.addresses.hooks.calcview, (PBYTE)hooks::hooked_calcview);
	hooks::original_processinterpolatedlist = (DWORD)DetourFunction((byte*)g_ctx.addresses.hooks.processinterpolatedlist, (byte*)hooks::processinterpolatedlist); //-V206
	hooks::original_clmove = (DWORD)DetourFunction((byte*)g_ctx.addresses.hooks.clmove, (byte*)hooks::hooked_clmove);
	hooks::original_checkforsequencechange = (DWORD)DetourFunction((byte*)g_ctx.addresses.hooks.checkforsequencechange, (byte*)hooks::hooked_checkforsequencechange);
	DetourFunction((PBYTE)g_ctx.addresses.hooks.shouldskipanimationframe, (PBYTE)hooks::hooked_shouldskipanimframe);
	DetourFunction((PBYTE)g_ctx.addresses.hooks.checkfilecrcswithserver, (PBYTE)hooks::hooked_checkfilecrcswithserver);
	DetourFunction((PBYTE)g_ctx.addresses.hooks.modifyeyeposition, (PBYTE)hooks::hooked_modifyeyeposition);
	DetourFunction((PBYTE)g_ctx.addresses.hooks.setupaliveloop, (PBYTE)hooks::hooked_setupaliveloop);

	/* ------------- */

	/* VMT HOOKS. */

	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_createmove_naked), 22);  //-V107 //-V221
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_fsn), 37); //-V107 //-V221
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_writeusercmddeltatobuffer), 24); //-V107 //-V221
	
	hooks::clientstate_hook = new vmthook(reinterpret_cast<DWORD**>((CClientState*)(uint32_t(m_clientstate()) + 0x8)));
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_packetstart), 5); //-V107 //-V221
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_packetend), 6); //-V107 //-V221

	hooks::panel_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_painttraverse), 41); //-V107 //-V221

	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_postscreeneffects), 44); //-V107 //-V221
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_overrideview), 18); //-V107 //-V221
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_drawfog), 17); //-V107 //-V221
	
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_setkeycodestate), 91); //-V107 //-V221
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_setmousecodestate), 92); //-V107 //-V221

	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_isconnected), 27); //-V107 //-V221
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_getscreenaspectratio), 101); //-V107 //-V221
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_ishltv), 93); //-V107 //-V221
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_ispaused), 90); //-V107 //-V221

	hooks::renderview_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_sceneend), 9); //-V107 //-V221

	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_beginframe), 42); //-V107 //-V221
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_getmaterial), 84); //-V107 //-V221

	hooks::modelrender_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_dme), 21); //-V107 //-V221

	hooks::prediction_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_runcommand), 19);
	
	hooks::surface_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_lockcursor), 67); //-V107 //-V221
	
	hooks::r_drawmodelstatsoverlay_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_r_drawmodelstatsoverlay), 13);  //-V107 //-V221
	hooks::sv_cheats_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_sv_cheats), 13);  //-V107 //-V221

	hooks::bspquery_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_listleavesinbox), 6); //-V107 //-V221

	hooks::filesystem_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_loosefileallowed), 128); //-V107 //-V221

	hooks::prediction_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_inprediction), 14); //-V107 //-V221

	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_clip_ray_collideable), 4); //-V107 //-V221
	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_trace_ray), 5); //-V107 //-V221

	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene_Reset), 16); //-V107 //-V221
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_present), 17); //-V107 //-V221
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene), 42); //-V107 //-V221

	/* ------------- */

	fakelag::get().SetMoveChokeClampLimit();
	hooks::hooked_events.RegisterSelf();
}

__forceinline void setup_steam()
{
	typedef uint32_t SteamPipeHandle;
	typedef uint32_t SteamUserHandle;

	SteamUserHandle hSteamUser = reinterpret_cast<HSteamUser(__cdecl*)(void)>(GetProcAddress(GetModuleHandle(crypt_str("steam_api.dll")), crypt_str("SteamAPI_GetHSteamUser")))();
	SteamPipeHandle hSteamPipe = reinterpret_cast<HSteamPipe(__cdecl*)(void)>(GetProcAddress(GetModuleHandle(crypt_str("steam_api.dll")), crypt_str("SteamAPI_GetHSteamPipe")))();
	
	SteamClient = reinterpret_cast<ISteamClient * (__cdecl*)(void)>(GetProcAddress(GetModuleHandle(crypt_str("steam_api.dll")), crypt_str("SteamClient")))();
	SteamGameCoordinator = (ISteamGameCoordinator*)SteamClient->GetISteamGenericInterface(hSteamUser, hSteamPipe, crypt_str("SteamGameCoordinator001"));
	SteamUser = (ISteamUser*)SteamClient->GetISteamUser(hSteamUser, hSteamPipe, crypt_str("SteamUser019"));
	SteamFriends = SteamClient->GetISteamFriends(hSteamUser, hSteamPipe, crypt_str("SteamFriends015"));
	SteamUtils = SteamClient->GetISteamUtils(hSteamPipe, crypt_str("SteamUtils009"));
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	if (dwReason != DLL_PROCESS_ATTACH)
		return FALSE;

	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)(main), hModule, NULL, NULL);
	if (hThread)
		CloseHandle(hThread);

	return TRUE;
}
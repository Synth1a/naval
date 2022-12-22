// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\misc\misc.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\ragebot\aim.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\lagcompensation\local_animations.h"
#include "..\..\cheats\ragebot\shots.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\visuals\player_esp.h"
#include "..\..\cheats\visuals\dormant_esp.h"
#include "..\..\cheats\visuals\world_esp.h"
#include "..\..\cheats\visuals\nightmode.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\networking\networking.h"
#include "..\..\skinchanger\SkinChanger.h"

using FrameStageNotify_t = void(__stdcall*)(ClientFrameStage_t);

void weather()
{
	static ClientClass* client_class = nullptr;

	if (!client_class)
		client_class = m_client()->GetAllClasses();

	while (client_class)
	{
		if (client_class->m_ClassID == CPrecipitation)
			break;

		client_class = client_class->m_pNext;
	}

	if (!client_class)
		return;

	auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
	auto serial = math::random_int(0, 4095);

	g_ctx.globals.m_networkable = client_class->m_pCreateFn(entry, serial);

	if (!g_ctx.globals.m_networkable)
		return;

	auto m_precipitation = g_ctx.globals.m_networkable->GetIClientUnknown()->GetBaseEntity();

	if (!m_precipitation)
		return;

	g_ctx.globals.m_networkable->PreDataUpdate(0);
	g_ctx.globals.m_networkable->OnPreDataChanged(0);

	static auto m_nPrecipType = netvars::get().get_offset(crypt_str("CPrecipitation"), crypt_str("m_nPrecipType"));
	static auto m_vecMins = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMins"));
	static auto m_vecMaxs = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMaxs"));

	*(int*)(uintptr_t(m_precipitation) + m_nPrecipType) = 0;
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMaxs) = Vector(32768.0f, 32768.0f, 32768.0f);
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMins) = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->GetCollideable()->OBBMaxs() = Vector(32768.0f, 32768.0f, 32768.0f);
	m_precipitation->GetCollideable()->OBBMins() = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->set_abs_origin((m_precipitation->GetCollideable()->OBBMins() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f);
	m_precipitation->m_vecOrigin() = (m_precipitation->GetCollideable()->OBBMaxs() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f;

	m_precipitation->OnDataChanged(0);
	m_precipitation->PostDataUpdate(0);
}

void remove_smoke()
{
	std::vector <IMaterial*> smoke_materials =
	{
		m_materialsystem()->FindMaterial(crypt_arr("effects/overlaysmoke"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/beam_smoke_01"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/particle_smokegrenade"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/particle_smokegrenade1"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/particle_smokegrenade2"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/particle_smokegrenade3"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/particle_smokegrenade_sc"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smoke1/smoke1"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smoke1/smoke1_ash"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smoke1/smoke1_nearcull"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smoke1/smoke1_nearcull2"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smoke1/smoke1_snow"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smokesprites_0001"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/smokestack"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_emods"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_emods_impactdust"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_fire"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_nearcull"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_nearcull_fog"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_nearcull_nodepth"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev1_smokegrenade"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev4_emods_nocull"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev4_nearcull"), nullptr),
		m_materialsystem()->FindMaterial(crypt_arr("particle/vistasmokev1/vistasmokev4_nocull"), nullptr)
	};

	static bool last_value = false;
	if (last_value != g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SMOKE])
	{
		last_value = g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SMOKE];

		for (auto material : smoke_materials)
		{
			if (!material)
				continue;

			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, last_value);
		}
	}

	if (g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SMOKE])
	{
		static auto smoke_count = *reinterpret_cast<uint32_t**>(g_ctx.addresses.smoke_count);
		*(int*)smoke_count = 0;
	}
}

void rain()
{
	static auto rain = false;

	if (rain != g_cfg.esp.rain || g_ctx.globals.should_update_weather)
	{
		rain = g_cfg.esp.rain;

		if (g_ctx.globals.m_networkable)
		{
			g_ctx.globals.m_networkable->Release();
			g_ctx.globals.m_networkable = nullptr;
		}

		if (rain)
			weather();

		g_ctx.globals.should_update_weather = false;
	}
}

void remove_shadows()
{
	if (g_cfg.esp.removals[REMOVALS_SHADOWS]) {
		g_ctx.convars.cl_csm_static_prop_shadows->SetValue(FALSE);
		g_ctx.convars.cl_csm_shadows->SetValue(FALSE);
		g_ctx.convars.cl_csm_world_shadows->SetValue(FALSE);
		g_ctx.convars.cl_foot_contact_shadows->SetValue(FALSE);
		g_ctx.convars.cl_csm_viewmodel_shadows->SetValue(FALSE);
		g_ctx.convars.cl_csm_rope_shadows->SetValue(FALSE);
		g_ctx.convars.cl_csm_sprite_shadows->SetValue(FALSE);
		g_ctx.convars.r_shadows->SetValue(FALSE);
	}
	else {
		g_ctx.convars.cl_csm_static_prop_shadows->SetValue(TRUE);
		g_ctx.convars.cl_csm_shadows->SetValue(TRUE);
		g_ctx.convars.cl_csm_world_shadows->SetValue(TRUE);
		g_ctx.convars.cl_foot_contact_shadows->SetValue(TRUE);
		g_ctx.convars.cl_csm_viewmodel_shadows->SetValue(TRUE);
		g_ctx.convars.cl_csm_rope_shadows->SetValue(TRUE);
		g_ctx.convars.cl_csm_sprite_shadows->SetValue(TRUE);
		g_ctx.convars.r_shadows->SetValue(TRUE);
	}
}

void clear_notice()
{
	static DWORD* death_notice = nullptr;

	if (g_ctx.local()->is_alive())
	{
		if (!death_notice)
			death_notice = (DWORD*)util::FindHudElement(crypt_str("CCSGO_HudDeathNotice"));

		if (death_notice)
		{
			auto local_death_notice = (float*)((uintptr_t)death_notice + 0x50);

			if (local_death_notice)
				*local_death_notice = g_cfg.esp.preserve_killfeed ? FLT_MAX : 1.5f;

			if (g_ctx.globals.should_clear_death_notices)
			{
				g_ctx.globals.should_clear_death_notices = false;

				using Fn = void(__thiscall*)(uintptr_t);
				static auto clear_notices = (Fn)g_ctx.addresses.clear_notices;

				clear_notices((uintptr_t)death_notice - 0x14);
			}
		}
	}
	else
		death_notice = 0;
}

void __stdcall hooks::hooked_fsn(ClientFrameStage_t stage)
{
	static auto original_fn = client_hook->get_func_address <FrameStageNotify_t>(37);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!g_ctx.available())
	{
		nightmode::get().clear_stored_materials();
		animation_system::get().clear_stored_data();
		shots::get().clear_stored_data();

		return original_fn(stage);
	}

	if (g_ctx.globals.updating_skins && m_clientstate()->iDeltaTick > 0) //-V807
		g_ctx.globals.updating_skins = false;

	switch (stage)
	{
	case FRAME_UNDEFINED:
	{
	}break;
	case FRAME_START:
	{
	}break;
	case FRAME_NET_UPDATE_START:
	{
	}break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
	{
		SkinChanger::run();

		for (auto i = 1; i < m_globals()->m_maxclients; ++i) {
			auto player = (player_t*)m_entitylist()->GetClientEntity(i);
			if (!player->valid(false) || player == g_ctx.local())
				continue;

			player->set_abs_origin(player->m_vecOrigin());
		}
	}break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
	{

	}break;
	case FRAME_NET_UPDATE_END:
	{
		// if miss log processing.
		shots::get().on_fsn();
	}break;
	case FRAME_RENDER_START:
	{
		if (g_cfg.esp.client_bullet_impacts)
		{
			static auto last_count = 0;
			auto& client_impact_list = *(CUtlVector <client_hit_verify_t>*)((uintptr_t)g_ctx.local() + 0x11C50);

			for (auto i = client_impact_list.Count(); i > last_count; --i)
				m_debugoverlay()->BoxOverlay(client_impact_list[i - 1].position, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f), QAngle(0.0f, 0.0f, 0.0f), g_cfg.esp.client_bullet_impacts_color.r(), g_cfg.esp.client_bullet_impacts_color.g(), g_cfg.esp.client_bullet_impacts_color.b(), g_cfg.esp.client_bullet_impacts_color.a(), 4.0f);

			if (client_impact_list.Count() != last_count)
				last_count = client_impact_list.Count();
		}

		remove_smoke();

		misc::get().remove_player_patches();
		misc::get().ragdolls();

		if (g_cfg.esp.removals[REMOVALS_FLASH] && g_ctx.local()->m_flFlashDuration() && g_cfg.player.enable) //-V807
			g_ctx.local()->m_flFlashDuration() = 0.0f;

		if (*(bool*)m_postprocessing() != (g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_POSTPROCESSING] && (!g_cfg.esp.world_modulation || !g_cfg.esp.exposure)))
			*(bool*)m_postprocessing() = g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_POSTPROCESSING] && (!g_cfg.esp.world_modulation || !g_cfg.esp.exposure);

		auto get_original_scope = false;

		if (g_ctx.local()->is_alive())
		{
			g_ctx.globals.in_thirdperson = g_cfg.keybinds.key[THIRDPERSON_KEYBIND].active;

			if (g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SCOPE])
			{
				auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

				if (weapon)
				{
					get_original_scope = true;

					g_ctx.globals.scoped = g_ctx.local()->m_bIsScoped() && weapon->m_zoomLevel();
					g_ctx.local()->m_bIsScoped() = weapon->m_zoomLevel();
				}
			}
		}

		if (!get_original_scope)
			g_ctx.globals.scoped = g_ctx.local()->m_bIsScoped();
	}break;
	case FRAME_RENDER_END:
	{
		if (g_ctx.convars.r_drawspecificstaticprop->GetBool())
			g_ctx.convars.r_drawspecificstaticprop->SetValue(FALSE);

		if (g_ctx.globals.change_materials)
		{
			if (g_cfg.esp.nightmode && g_cfg.player.enable)
				nightmode::get().apply();
			else
				nightmode::get().remove();

			g_ctx.globals.change_materials = false;
		}

		remove_shadows();
		worldesp::get().skybox_changer();
		worldesp::get().fog_changer();

		misc::get().FullBright();
		misc::get().ViewModel();

		if (g_cfg.esp.world_modulation && g_cfg.esp.ambient && g_ctx.convars.r_modelAmbientMin->GetFloat() != g_cfg.esp.ambient * 0.05f) //-V550
			g_ctx.convars.r_modelAmbientMin->SetValue(g_cfg.esp.ambient * 0.05f);
		else if ((!g_cfg.esp.world_modulation || !g_cfg.esp.ambient) && g_ctx.convars.r_modelAmbientMin->GetFloat())
			g_ctx.convars.r_modelAmbientMin->SetValue(0.0f);
	}break;
	default:
		break;
	}

	networking::get().process_interpolation(stage, false);
	original_fn(stage);
	networking::get().process_interpolation(stage, true);

	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
	{
		engineprediction::get().adjust_viewmodel_data();
	}break;
	case FRAME_NET_UPDATE_END:
	{
		// if lagcompensation.
		for (auto i = 1; i < m_globals()->m_maxclients; ++i) {
			auto player = (player_t*)m_entitylist()->GetClientEntity(i);
			if (!player || player == g_ctx.local())
				continue;

			AimPlayer* data = &aimbot::get().m_players[i];
			data->OnNetUpdate(player);
		}

		lagcompensation::get().PostPlayerUpdate();

		// if rain processing.
		rain();
	}break;
	default:
		break;
	}

	clear_notice();
}
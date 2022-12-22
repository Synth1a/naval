// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\lagcompensation\local_animations.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\ragebot\aim.h"
#include "..\..\cheats\networking\networking.h"
#include "..\..\cheats\exploits\exploits.h"

/* HOOK DETOURS. */

_declspec(noinline)bool hooks::setupbones_detour(void* ecx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time)
{
	auto player = reinterpret_cast<player_t*>(uintptr_t(ecx) - 0x4);

	if (!player->valid(false))
		return ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);

	if (g_ctx.animations.m_update_bones)
		return ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else if (bone_world_out)
	{
		if (player->EntIndex() == g_ctx.local()->EntIndex())
			local_animations::get().get_cached_matrix(player, bone_world_out);
		else
			animation_system::get().get_cached_matrix(player, bone_world_out);
	}

	return true;
}

_declspec(noinline)void hooks::standardblendingrules_detour(player_t* player, int i, c_studio_hdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask)
{
	// return to original if we are not in game or wait till we find the players.
	if (!player->valid(false))
		return ((StandardBlendingRulesFn)original_standardblendingrules)(player, hdr, pos, q, curtime, boneMask);

	// backup effects.
	auto backup_effects = player->m_fEffects();

	// enable extrapolation(disabling interpolation).
	if (!(player->m_fEffects() & 8))
		player->m_fEffects() |= 8;

	((StandardBlendingRulesFn)original_standardblendingrules)(player, hdr, pos, q, curtime, boneMask);

	// disable extrapolation after hooks(enabling interpolation).
	if (player->m_fEffects() & 8)
		player->m_fEffects() = backup_effects;
}

_declspec(noinline)void hooks::doextrabonesprocessing_detour(player_t* player, c_studio_hdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context)
{

}

_declspec(noinline)void hooks::updateclientsideanimation_detour(player_t* player)
{
	// we are not in game and also there is no entitites!
	// if the player is not valid, just fallback.
	if (!player->valid(false))
		return reinterpret_cast<UpdateClientSideAnimationFn>(original_updateclientsideanimation)(player);

	//if (player->EntIndex() != g_ctx.local()->EntIndex())
	//	player->set_abs_origin(player->m_vecOrigin());

	// handling clientsided animations on localplayer.
	if (!g_ctx.animations.m_update_animations)
	{
		if (player->EntIndex() == g_ctx.local()->EntIndex())
			local_animations::get().on_update_clientside_animation(player);
		else
			animation_system::get().on_update_clientside_animation(player);

		return;
	}

	return reinterpret_cast<UpdateClientSideAnimationFn>(original_updateclientsideanimation)(player);
}


_declspec(noinline)void hooks::estimateabsvelocity_detour(player_t* player, Vector& velocity)
{
	// don't run the hook if we are not in game or there is no player at all.
	if (!player->valid(false) || player->EntIndex() != g_ctx.local()->EntIndex())
		return reinterpret_cast<EstimateAbsVelocityFn>(original_estimateabsvelocity)(player, velocity);

	// no interpolation, do not estimate abs velocity.
	if (player->m_fEffects() & 8)
		return;

	// player is abusing tickbase, do not estimate abs velocity
	if (player->m_flOldSimulationTime() > player->m_flSimulationTime())
		return;

	// all good to go, you can estimate the player abs velocity
	return reinterpret_cast<EstimateAbsVelocityFn>(original_estimateabsvelocity)(player, velocity);
}

_declspec(noinline)void hooks::physicssimulate_detour(player_t* player)
{
	// don't run the codes if we are not in game, or there is no player, or we are not alive, or the target player is not us.
	if (!player->valid(false) || player->EntIndex() != g_ctx.local()->EntIndex())
		return ((PhysicsSimulateFn)original_physicssimulate)(player);

	// simulation ticks and if it is on par with our tickcount then don't run the code since the prediction have been done.
	int sim_tick = *(int*)((uintptr_t)(player) + 0x2AC);
	if (sim_tick == m_globals()->m_tickcount)
		return;

	// don't run the code if we didn't even need to process simulation.
	C_CommandContext* cmd_ctx = reinterpret_cast <C_CommandContext*>((uintptr_t)(g_ctx.local()) + 0x350C);
	if (!cmd_ctx || !cmd_ctx->m_need_processing)
		return;

	aimbot::get().AdjustRevolverData(cmd_ctx->m_command_number, cmd_ctx->m_command.m_buttons);

	((PhysicsSimulateFn)original_physicssimulate)(player);

	engineprediction::get().store_viewmodel_data();
	return networking::get().store_netvar_data(cmd_ctx->m_command_number);
}

_declspec(noinline)void hooks::modifyeyeposition_detour(c_baseplayeranimationstate* state, Vector& position)
{
	// don't run the code if we are not in game.
	if (!state->m_pBasePlayer->valid(false) || !g_ctx.globals.in_createmove)
		return;

	if (!state->m_bLanding && state->m_flAnimDuckAmount == 0.f)
	{
		state->m_bSmoothHeightValid = false;
		state->m_flCameraSmoothHeight = INT_MAX;
		return;
	}

	auto head_pos = state->m_pBasePlayer->m_CachedBoneData().Base()[state->m_pBasePlayer->lookup_bone(crypt_str("head_0"))].at(3);
	head_pos.z += 1.7f;

	if (position.z > head_pos.z)
	{
		const auto v21 = abs(position.z - head_pos.z);
		const auto v22 = (v21 - 4.0) * 0.16666667;
		float v23;

		if (v22 >= 0.0)
			v23 = fminf(v22, 1.0);
		else
			v23 = 0.0;

		position.z = (head_pos.z - position.z) * (v23 * v23 * 3.0 - v23 * v23 * 2.0 * v23) + position.z;
	}
}

_declspec(noinline)void hooks::calcviewmodelbob_detour(player_t* player, Vector& position)
{
	if (!player->valid(false) || !g_cfg.esp.removals[REMOVALS_LANDING_BOB] || player->EntIndex() != g_ctx.local()->EntIndex() || !player->is_alive())
		return ((CalcViewmodelBobFn)original_calcviewmodelbob)(player, position);
}

_declspec(noinline)void hooks::calcview_detour(player_t* player, Vector& eye_origin, Vector& eye_angles, float& z_near, float& z_far, float& fov)
{
	// don't run the codes if we are not in game or there is no player and if the player is not us.
	if (!player->valid(false) || player->EntIndex() != g_ctx.local()->EntIndex())
		return ((CalcViewFn)original_calcview)(player, eye_origin, eye_angles, z_near, z_far, fov);

	// backup data for removals.
	const auto aim_punch = player->m_aimPunchAngle();
	const auto view_punch = player->m_viewPunchAngle();
	const auto should_use_new_animstate = player->m_bShouldUseNewAnimState();

	// apply removals if we enable the settings.
	if (g_cfg.esp.removals[REMOVALS_RECOIL] && g_cfg.player.enable) 
	{
		player->m_aimPunchAngle() = ZERO;
		player->m_viewPunchAngle() = ZERO;
	}

	player->m_bShouldUseNewAnimState() = false;
	((CalcViewFn)original_calcview)(player, eye_origin, eye_angles, z_near, z_far, fov);
	player->m_bShouldUseNewAnimState() = should_use_new_animstate;

	// restore data.
	if (g_cfg.esp.removals[REMOVALS_RECOIL] && g_cfg.player.enable)
	{
		player->m_aimPunchAngle() = aim_punch;
		player->m_viewPunchAngle() = view_punch;
	}
}

_declspec(noinline)void hooks::buildtransformations_detour(player_t* player, void* hdr, void* pos, void* q, const void* camera_transform, int bone_mask, void* bone_computed) {
	if (!player->valid(false))
		return ((BuildTransformationsFn)original_buildtransformations)(player, hdr, pos, q, camera_transform, bone_mask, bone_computed);

	if (player->EntIndex() == m_engine()->GetLocalPlayer()) 
	{
		g_ctx.globals.buildtransformations_player = player;
		g_ctx.globals.buildtransformations_vector = player->GetRenderAngles();
	}

	((BuildTransformationsFn)original_buildtransformations)(player, hdr, pos, q, camera_transform, bone_mask, bone_computed);
	g_ctx.globals.buildtransformations_player = nullptr;
}

_declspec(noinline)Vector* hooks::geteyeangles_detour(player_t* player)
{
	if (_ReturnAddress() == (int*)g_ctx.addresses.return_to.cbaseplayer_eyeposition_and_vectors || _ReturnAddress() == (int*)g_ctx.addresses.return_to.cbaseplayer_thirdperson_yaw_vectors || _ReturnAddress() == (int*)g_ctx.addresses.return_to.cbaseplayer_thirdperson_pitch_vectors)
		return ((GetEyeAnglesFn)original_geteyeangles)(player);

	if (!player || player != g_ctx.globals.buildtransformations_player)
		return ((GetEyeAnglesFn)original_geteyeangles)(player);

	g_ctx.globals.buildtransformations_player = nullptr; 
	return &g_ctx.globals.buildtransformations_vector;
}

_declspec(noinline)void hooks::checkforsequencechange_detour(void* ecx, void* hdr, int cur_sequence, bool force_new_sequence, bool interpolate)
{
	return reinterpret_cast<CheckForSequenceFn>(original_checkforsequencechange)(ecx, hdr, cur_sequence, force_new_sequence, false);
}

_declspec(noinline)void hooks::setupaliveloop_detour(c_baseplayeranimationstate* animstate)
{
	if (!animstate->m_pBasePlayer->valid(false))
		return;

	AnimationLayer* aliveloop = &animstate->m_pBasePlayer->get_animlayers()[ANIMATION_LAYER_ALIVELOOP];
	if (!aliveloop)
		return;

	if (animstate->m_pBasePlayer->sequence_activity(aliveloop->m_nSequence) != ACT_CSGO_ALIVE_LOOP)
	{
		animstate->set_layer_sequence(aliveloop, ACT_CSGO_ALIVE_LOOP);
		animstate->set_layer_cycle(aliveloop, math::random_float(0.0f, 1.0f));
		animstate->set_layer_rate(aliveloop, animstate->m_pBasePlayer->get_layer_sequence_cycle_rate(aliveloop, aliveloop->m_nSequence) * math::random_float(0.8, 1.1f));
	}
	else
	{
		float retain_cycle = aliveloop->m_flCycle;
		if (animstate->m_pWeapon != animstate->m_pWeaponLast)
		{
			animstate->set_layer_sequence(aliveloop, ACT_CSGO_ALIVE_LOOP);
			animstate->set_layer_cycle(aliveloop, retain_cycle);
		}
		else if (animstate->is_layer_sequence_finished(aliveloop, animstate->m_flLastUpdateIncrement))
			animstate->set_layer_rate(aliveloop, animstate->m_pBasePlayer->get_layer_sequence_cycle_rate(aliveloop, aliveloop->m_nSequence) * math::random_float(0.8, 1.1f));
		else
			animstate->set_layer_weight(aliveloop, math::remap_val_clamped(animstate->m_flSpeedAsPortionOfRunTopSpeed, 0.55f, 0.9f, 1.0f, 0.0f));
	}

	return animstate->increment_layer_cycle(aliveloop, true);
}

int hooks::processinterpolatedlist()
{
	// don't run interpolation.
	static auto allow_extrapolation = *(bool**)(g_ctx.addresses.allow_extrapolation);

	if (allow_extrapolation)
		*allow_extrapolation = false;

	return ((ProcessInterpolatedListFn)original_processinterpolatedlist)();
}

/* HOOKING. */

bool __fastcall hooks::hooked_shouldskipanimframe()
{
	return false;
}

bool __fastcall hooks::hooked_setupbones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time)
{
	return setupbones_detour(ecx, bone_world_out, max_bones, bone_mask, current_time);
}

void __fastcall hooks::hooked_standardblendingrules(player_t* player, int i, c_studio_hdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask)
{
	return standardblendingrules_detour(player, i, hdr, pos, q, curtime, boneMask);
}

void __fastcall hooks::hooked_doextrabonesprocessing(player_t* player, void* edx, c_studio_hdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context)
{
	return doextrabonesprocessing_detour(player, hdr, pos, q, matrix, bone_list, context);
}

void __fastcall hooks::hooked_updateclientsideanimation(player_t* player)
{
	return updateclientsideanimation_detour(player);
}

void __fastcall hooks::hooked_estimateabsvelocity(player_t* player, void* edx, Vector& velocity)
{
	return estimateabsvelocity_detour(player, velocity);
}

void __fastcall hooks::hooked_physicssimulate(player_t* player)
{
	return physicssimulate_detour(player);
}

void __fastcall hooks::hooked_modifyeyeposition(c_baseplayeranimationstate* state, void* edx, Vector& position)
{
	return modifyeyeposition_detour(state, position);
}

void __fastcall hooks::hooked_calcviewmodelbob(player_t* player, void* edx, Vector& position)
{
	return calcviewmodelbob_detour(player, position);
}

void __fastcall hooks::hooked_calcview(player_t* player, void* edx, Vector& eye_origin, Vector& eye_angles, float& z_near, float& z_far, float& fov)
{
	return calcview_detour(player, eye_origin, eye_angles, z_near, z_far, fov);
}

void __fastcall hooks::hooked_buildtransformations(player_t* player, void* edx, void* hdr, void* pos, void* q, const void* camera_transform, int bone_mask, void* bone_computed)
{
	return buildtransformations_detour(player, hdr, pos, q, camera_transform, bone_mask, bone_computed);
}

Vector* __fastcall hooks::hooked_geteyeangles(player_t* player, void* edx)
{
	return geteyeangles_detour(player);
}

void __fastcall hooks::hooked_checkforsequencechange(void* ecx, void* edx, void* hdr, int cur_sequence, bool force_new_sequence, bool interpolate)
{
	return checkforsequencechange_detour(ecx, hdr, cur_sequence, force_new_sequence, interpolate);
}

void __fastcall hooks::hooked_setupaliveloop(c_baseplayeranimationstate* animstate)
{
	return setupaliveloop_detour(animstate);
}
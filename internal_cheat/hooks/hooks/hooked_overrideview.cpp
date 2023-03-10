// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\lagcompensation\local_animations.h"

using OverrideView_t = void(__stdcall*)(CViewSetup*);

void thirdperson(bool fakeducking);

void __stdcall hooks::hooked_overrideview(CViewSetup* viewsetup)
{
	static auto original_fn = clientmode_hook->get_func_address <OverrideView_t> (18);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!viewsetup)
		return original_fn(viewsetup);

	if (g_ctx.local())
	{
		static auto fakeducking = false;

		if (!fakeducking && g_ctx.globals.fakeducking)
			fakeducking = true;
		else if (fakeducking && !g_ctx.globals.fakeducking && (!g_ctx.local()->get_animation_state()->m_flAnimDuckAmount || g_ctx.local()->get_animation_state()->m_flAnimDuckAmount == 1.0f)) //-V550
			fakeducking = false;

		if (!g_ctx.local()->is_alive()) //-V807
			fakeducking = false;

		auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

		if (weapon)
		{
			auto is_scoped = g_ctx.globals.scoped && weapon->is_scopable() && weapon->m_zoomLevel();

			if (g_cfg.player.enable) {
				if (g_cfg.esp.fov && !is_scoped) {
					viewsetup->fov = g_cfg.esp.fov;
				}

				if (g_cfg.esp.fov_while_zoomed && g_cfg.esp.fov && is_scoped) {
					if (weapon->m_zoomLevel() == 1)
						viewsetup->fov = g_cfg.esp.fov;
					else if (g_cfg.esp.fov > 17 && weapon->m_zoomLevel() == 2)
						viewsetup->fov = g_cfg.esp.fov - 15;
				}
			}

			auto viewmodel = (entity_t*)m_entitylist()->GetClientEntityFromHandle(g_ctx.local()->m_hViewModel());

			if (viewmodel)
			{
				auto eyeAng = viewsetup->angles;
				eyeAng.z -= (float)g_cfg.esp.viewmodel_roll;

				viewmodel->set_abs_angles(eyeAng);
			}

			if (weapon->is_grenade() && g_cfg.esp.grenade_prediction && g_cfg.player.enable)
				GrenadePrediction::get().View(viewsetup, weapon);
		}

		if (g_cfg.player.enable && (g_cfg.keybinds.key[THIRDPERSON_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[THIRDPERSON_KEYBIND].key < KEY_MAX || g_cfg.misc.thirdperson_when_spectating))
			thirdperson(fakeducking);
		else
		{
			g_ctx.globals.in_thirdperson = false;
			m_input()->m_fCameraInThirdPerson = false;
		}
	
		/* fakeduck animation. */

		// useful data to be used later.
		static bool in_transition = false;
		
		// if we are fakeducking.
		if (fakeducking)
		{
			// we are in transition and now we run the animation using frametime to make our viewoffset increased till the limit (standing viewoffset with magical number).
			in_transition = true;
			g_ctx.globals.fakeduck_view += m_globals()->m_frametime * 150;

			// if the maxduck reached the limit (standing viwoffset with magical number), clamping system.
			if (g_ctx.globals.fakeduck_view > m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f)
				g_ctx.globals.fakeduck_view = m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f;
		}
		// or if we are not fakeducking.
		else
		{
			// don't enable transition or else the code will always running that will break the viewoffset. this code also will makes unducking viewoffset look smooth after you fakeducks.
			if (g_ctx.globals.fakeduck_view > g_ctx.local()->m_vecViewOffset().z)
				g_ctx.globals.fakeduck_view -= m_globals()->m_frametime * 150;

			// if maxduck number reached the limit when we are not fakeducking (normal viewwoffset), clamping system.
			if (g_ctx.globals.fakeduck_view < g_ctx.local()->m_vecViewOffset().z) {
				g_ctx.globals.fakeduck_view = g_ctx.local()->m_vecViewOffset().z;

				// transition is done, no need to do any animations anymore.
				in_transition = false;
			}
		}

		// only set our viewsetup origin when we are in transition (just like the thirdperson that leaked here from my cheat, hmmm.).
		if (in_transition)
			viewsetup->origin = g_ctx.local()->GetAbsOrigin() + Vector(0.0f, 0.0f, g_ctx.globals.fakeduck_view);

		original_fn(viewsetup);
	}
	else
		return original_fn(viewsetup);
}

void Thirdperson_Init(bool fakeducking, float progress) {
	/* our current fraction. */
	static float current_fraction = 0.0f;

	auto distance = ((float)g_cfg.misc.thirdperson_distance) * progress;
	Vector angles, inverse_angles;

	// get camera angles.
	m_engine()->GetViewAngles(angles);
	m_engine()->GetViewAngles(inverse_angles);

	// cam_idealdist convar.
	inverse_angles.z = distance;

	// set camera direction.
	Vector forward, right, up;
	math::angle_vectors(inverse_angles, &forward, &right, &up);

	// various fixes to camera when fakeducking.
	auto eye_pos = fakeducking ? g_ctx.local()->GetAbsOrigin() + m_gamemovement()->GetPlayerViewOffset(false) : g_ctx.local()->GetAbsOrigin() + g_ctx.local()->m_vecViewOffset();
	auto offset = eye_pos + forward * -distance + right + up;

	// setup trace filter and trace.
	CTraceFilterWorldOnly filter;
	trace_t tr;

	// tracing to camera angles.
	m_trace()->TraceRay(Ray_t(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f)), MASK_SOLID, &filter, &tr);

	// interpolate camera speed if something behind our camera.
	if (current_fraction > tr.fraction)
		current_fraction = tr.fraction;
	else if (current_fraction > 0.9999f)
		current_fraction = 1.0f;

	// adapt distance to travel time.
	current_fraction = math::interpolate(current_fraction, tr.fraction, m_globals()->m_frametime * 10.0f);
	angles.z = distance * current_fraction;

	// override camera angles.
	m_input()->m_vecCameraOffset = angles;
}

void thirdperson(bool fakeducking)
{
	/* thirdperson code. */
	{
		static float progress;
		static bool in_transition;
		static auto in_thirdperson = false;

		if (!in_thirdperson && g_ctx.globals.in_thirdperson)
		{
			in_thirdperson = true;
		}
		else if (in_thirdperson && !g_ctx.globals.in_thirdperson)
			in_thirdperson = false;

		if (g_ctx.local()->is_alive() && in_thirdperson)
		{
			in_transition = false;

			if (!m_input()->m_fCameraInThirdPerson)
			{
				m_input()->m_fCameraInThirdPerson = true;
			}
		}
		else
		{
			progress -= m_globals()->m_frametime * 8.f + (progress / 100);
			progress = math::clamp(progress, 0.f, 1.f);

			if (!progress)
				m_input()->m_fCameraInThirdPerson = false;
			else
			{
				in_transition = true;
				m_input()->m_fCameraInThirdPerson = true;
			}
		}

		if (m_input()->m_fCameraInThirdPerson && !in_transition)
		{
			progress += m_globals()->m_frametime * 8.f + (progress / 100);
			progress = math::clamp(progress, 0.f, 1.f);
		}

		Thirdperson_Init(fakeducking, progress);
	}

	/* thirdperson on death code. */
	{
		static auto require_reset = false;

		if (g_ctx.local()->is_alive())
		{
			require_reset = false;
			return;
		}

		if (g_cfg.misc.thirdperson_when_spectating)
		{
			if (require_reset)
				g_ctx.local()->m_iObserverMode() = OBS_MODE_CHASE;

			if (g_ctx.local()->m_iObserverMode() == OBS_MODE_IN_EYE)
				require_reset = true;
		}
	}
}
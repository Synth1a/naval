// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\ui\menu.h"
#include "..\..\cheats\visuals\player_esp.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\visuals\world_esp.h"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\visuals\bullet_tracers.h"
#include "..\..\cheats\visuals\dormant_esp.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\misc\misc.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\lagcompensation\local_animations.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\ragebot\aim.h"
#include "..\..\cheats\networking\networking.h"

using PaintTraverse_t = void(__thiscall*)(void*, vgui::VPANEL, bool, bool);

void __fastcall hooks::hooked_painttraverse(void* ecx, void* edx, vgui::VPANEL panel, bool force_repaint, bool allow_force)
{
	static auto original_fn = panel_hook->get_func_address <PaintTraverse_t> (41);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true); //-V807
	
	static auto set_console = true;
	if (set_console)
	{
		set_console = false;

		g_ctx.convars.developer->SetValue(FALSE); //-V807
		g_ctx.convars.r_jiggle_bones->SetValue(FALSE);

		g_ctx.convars.con_filter_enable->SetValue(TRUE);
		g_ctx.convars.con_filter_text->SetValue(crypt_str(""));
		m_engine()->ExecuteClientCmd(crypt_str("clear"));
	}

	static auto log_value = true;
	if (log_value != g_cfg.misc.show_default_log)
	{
		log_value = g_cfg.misc.show_default_log;

		if (log_value)
			g_ctx.convars.con_filter_text->SetValue(crypt_str(""));
		else
			g_ctx.convars.con_filter_text->SetValue(crypt_str("IrWL5106TZZKNFPz4P4Gl3pSN?J370f5hi373ZjPg%VOVh6lN"));
	}

	static vgui::VPANEL panel_id = 0;

	static auto in_game = false;
	if (!in_game && m_engine()->IsInGame())
	{
		in_game = true;

		g_ctx.globals.should_update_beam_index = true;
		g_ctx.globals.should_update_playerresource = true;
		g_ctx.globals.should_update_gamerules = true;
		g_ctx.globals.should_update_weather = true;
		g_ctx.globals.bomb_timer_enable = true;

		g_ctx.globals.should_remove_smoke = false;
	}
	else if (in_game && !m_engine()->IsInGame())
	{
		in_game = false;

		engineprediction::get().reset_data();
		local_animations::get().reset_data();
		antiaim::get().reset_data();
		networking::get().reset_data();
		fakelag::get().reset_data();
		otheresp::get().reset_data();
		c_dormant_esp::get().reset_data();
		playeresp::get().reset_data();

		if (!SkinChanger::model_indexes.empty())
			SkinChanger::model_indexes.clear();

		if (!SkinChanger::player_model_indexes.empty())
			SkinChanger::player_model_indexes.clear();

		g_ctx.globals.last_aimbot_shot = 0;
		g_ctx.globals.kills = 0;

		g_ctx.globals.backup_model = false;

		g_ctx.globals.m_networkable = nullptr;
	}

	if (m_engine()->IsTakingScreenshot() && g_cfg.misc.anti_screenshot)
		return;

	static uint32_t HudZoomPanel = 0;

	if (!HudZoomPanel)
		if (!strcmp(crypt_str("HudZoom"), m_panel()->GetName(panel)))
			HudZoomPanel = panel;

	if (HudZoomPanel == panel && g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SCOPE])
		return;

	original_fn(ecx, panel, force_repaint, allow_force);

	if (!panel_id)
	{
		auto panelName = m_panel()->GetName(panel);

		if (!strcmp(panelName, crypt_str("MatSystemTopPanel")))
			panel_id = panel;
	}

	if (panel_id == panel)
	{
		if (g_ctx.available())
		{
			static auto alive = false;

			if (!alive && g_ctx.local()->is_alive())
			{
				alive = true;
				g_ctx.globals.should_clear_death_notices = true;
			}
			else if (alive && !g_ctx.local()->is_alive())
			{
				alive = false;

				local_animations::get().local_data.fake_animstate = nullptr;

				g_ctx.globals.weapon = nullptr;
				g_ctx.globals.should_choke_packet = false;
				g_ctx.globals.should_send_packet = false;
				g_ctx.globals.kills = 0;
				g_ctx.globals.should_buy = 3;
			}

			if (g_cfg.player.enable)
			{
				worldesp::get().paint_traverse();
				playeresp::get().paint_traverse();
			}

			misc::get().zeus_range();
			misc::get().desync_arrows();

			auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

			if (weapon->is_grenade() && g_cfg.esp.grenade_prediction && g_cfg.player.enable)
				GrenadePrediction::get().Paint();

			auto is_scoped = g_ctx.globals.scoped && weapon->is_scopable() && weapon->m_zoomLevel();

			if (g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SCOPE] && is_scoped)
			{
				render::get().line(g_ctx.globals.screen_width / 2, 0, g_ctx.globals.screen_width / 2, g_ctx.globals.screen_height, Color::Black);
				render::get().line(0, g_ctx.globals.screen_height / 2, g_ctx.globals.screen_width, g_ctx.globals.screen_height / 2, Color::Black);
			}

			if (g_ctx.local()->is_alive())
			{
				if (c_menu::get().public_alpha > 0.15f && g_cfg.legitbot.enabled)
				{
					if (g_cfg.legitbot.weapon[g_ctx.globals.current_weapon].fov)
					{
						float radius = tanf(DEG2RAD(g_cfg.legitbot.weapon[g_ctx.globals.current_weapon].fov) / 2) / tanf(DEG2RAD(90 + g_cfg.esp.fov) / 2) * g_ctx.globals.screen_width;
						render::get().circle_filled(g_ctx.globals.screen_width / 2, g_ctx.globals.screen_height / 2, 60, radius, Color(235, 235, 235, c_menu::get().public_alpha * 0.68));
						render::get().circle(g_ctx.globals.screen_width / 2, g_ctx.globals.screen_height / 2, 60, radius, Color(235, 235, 235, c_menu::get().public_alpha * 0.8));
					}

					if (g_cfg.legitbot.weapon[g_ctx.globals.current_weapon].silent_fov)
					{
						float silent_radius = tanf(DEG2RAD(g_cfg.legitbot.weapon[g_ctx.globals.current_weapon].silent_fov) / 2) / tanf(DEG2RAD(90 + g_cfg.esp.fov) / 2) * g_ctx.globals.screen_width;
						render::get().circle_filled(g_ctx.globals.screen_width / 2, g_ctx.globals.screen_height / 2, 60, silent_radius, Color(15, 235, 15, c_menu::get().public_alpha * 0.68));
						render::get().circle(g_ctx.globals.screen_width / 2, g_ctx.globals.screen_height / 2, 60, silent_radius, Color(15, 235, 15, c_menu::get().public_alpha * 0.8));
					}
				}
			}

			if (g_cfg.player.enable)
				otheresp::get().hitmarker_paint();

			if (g_cfg.player.enable && g_cfg.esp.damage_marker)
				otheresp::get().damage_marker_paint();

			otheresp::get().penetration_reticle();
			otheresp::get().force_crosshair();
			otheresp::get().automatic_peek_indicator();

			misc::get().ChatSpamer();

			bullettracers::get().draw_beams();
		}

		eventlogs::get().paint_traverse();
		misc::get().NightmodeFix();
		otheresp::get().indicators();
		otheresp::get().draw_indicators();
	}
}
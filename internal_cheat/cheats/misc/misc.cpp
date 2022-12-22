// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "misc.h"
#include "logs.h"
#include "fakelag.h"
#include "prediction_system.h"
#include "..\ui\menu.h"
#include "..\ragebot\aim.h"
#include "..\visuals\world_esp.h"
#include "..\networking\networking.h"
#include <ShlObj_core.h>

std::string misc::get_config_direction()
{
	std::string folder; static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, 0x001A, NULL, NULL, path)))
		folder = std::string(path) + crypt_str("\\akcent.xyz\\configs\\");

	CreateDirectory(folder.c_str(), NULL);
	return folder;
}

void misc::load_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	return eventlogs::get().add(crypt_str("loaded ") + c_menu::get().configuration_files.at(g_cfg.selected_config) + crypt_str(" config"), false);
}

void misc::save_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->save(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	return eventlogs::get().add(crypt_str("saved ") + c_menu::get().configuration_files.at(g_cfg.selected_config) + crypt_str(" config"), false);
}

void misc::remove_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->remove(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	c_menu::get().configuration_files = cfg_manager->files;

	if (g_cfg.selected_config >= c_menu::get().configuration_files.size())
		g_cfg.selected_config = c_menu::get().configuration_files.size() - 1; //-V103

	for (auto& current : c_menu::get().configuration_files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);

	return eventlogs::get().add(crypt_str("removed ") + c_menu::get().configuration_files.at(g_cfg.selected_config) + crypt_str(" config"), false);
}

void misc::add_config()
{
	auto empty = true;

	for (auto current : g_cfg.new_config_name)
	{
		if (current != ' ')
		{
			empty = false;
			break;
		}
	}

	if (empty)
		g_cfg.new_config_name = crypt_str("config");

	if (g_cfg.new_config_name.find(crypt_str(".cfg")) == std::string::npos)
		g_cfg.new_config_name += crypt_str(".cfg");

	cfg_manager->save(g_cfg.new_config_name);
	cfg_manager->config_files();

	g_cfg.selected_config = cfg_manager->files.size() - 1; //-V103
	c_menu::get().configuration_files = cfg_manager->files;

	for (auto& current : c_menu::get().configuration_files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);

	return eventlogs::get().add(crypt_str("added ") + g_cfg.new_config_name + crypt_str(" config"), false);
}

void misc::init_penetration()
{
	if (!g_cfg.esp.penetration_reticle)
		return;

	penetration::PenetrationOutput_t tmp_pen_data;
	Vector m_forward_dir = ZERO;

	math::angle_vectors(g_ctx.globals.original_viewangles, m_forward_dir);

	// run autowall once for penetration crosshair if we have an appropriate weapon.
	if (!g_ctx.globals.weapon->is_non_aim() && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER) {
		penetration::PenetrationInput_t in;
		in.m_from = g_ctx.local();
		in.m_target = nullptr;
		in.m_pos = g_ctx.globals.eye_pos + (m_forward_dir * g_ctx.globals.weapon->get_csweapon_info()->flRange);
		in.m_damage = 1.f;
		in.m_damage_pen = 1.f;
		in.m_can_pen = true;

		// run autowall.
		penetration::get().run(&in, &tmp_pen_data);
	}

	// set pen data for penetration crosshair.
	this->pen_data = tmp_pen_data;
}

void misc::unlockhiddenconvars()
{
	auto p = **reinterpret_cast<ConCommandBase***>(reinterpret_cast<DWORD>(m_cvar()) + 0x34);
	
	for (auto c = p->m_pNext; c != nullptr; c = c->m_pNext) {
		c->RemoveFlags(FCVAR_DEVELOPMENTONLY);
		c->RemoveFlags(FCVAR_HIDDEN);
	}
}

void misc::mouse_delta_fix(CUserCmd* m_pcmd)
{
	if (!m_pcmd)
		return;

	static Vector delta_viewangles = ZERO;
	Vector delta = (m_pcmd->m_viewangles - delta_viewangles).Clamp();

	if (!g_ctx.convars.sensitivity)
		return;

	if (delta.x != 0.f) 
	{
		if (!g_ctx.convars.m_pitch)
			return;

		int final_dy = static_cast<int>((delta.x / g_ctx.convars.m_pitch->GetFloat()) / g_ctx.convars.sensitivity->GetFloat());
		if (final_dy <= 32767)
		{
			if (final_dy >= -32768) 
			{
				if (final_dy >= 1 || final_dy < 0) 
				{
					if (final_dy <= -1)
						final_dy = final_dy;
					else
						final_dy = -1;
				}
				else
				{
					final_dy = 1;
				}
			}
			else 
			{
				final_dy = 32768;
			}
		}
		else 
		{
			final_dy = 32767;
		}

		m_pcmd->m_mousedy = static_cast<short>(final_dy);
	}

	if (delta.y != 0.f) 
	{
		if (!g_ctx.convars.m_yaw)
			return;

		int final_dx = static_cast<int>((delta.y / g_ctx.convars.m_yaw->GetFloat()) / g_ctx.convars.sensitivity->GetFloat());
		if (final_dx <= 32767)
		{
			if (final_dx >= -32768) {
				if (final_dx >= 1 || final_dx < 0) 
				{
					if (final_dx <= -1)
						final_dx = final_dx;
					else
						final_dx = -1;
				}
				else 
				{
					final_dx = 1;
				}
			}
			else 
			{
				final_dx = 32768;
			}
		}
		else
		{
			final_dx = 32767;
		}

		m_pcmd->m_mousedx = static_cast<short>(final_dx);
	}

	delta_viewangles = m_pcmd->m_viewangles;
}

void misc::NoDuck(CUserCmd* cmd)
{
	if (!g_cfg.misc.noduck)
		return;

	if (m_gamerules()->m_bIsValveDS())
		return;

	cmd->m_buttons |= IN_BULLRUSH;
}

void misc::ChatSpamer()
{
	if (!g_cfg.misc.chat)
		return;

	static std::string chatspam[] = 
	{ 
		crypt_str("naval - discord.gg/QrVU3efyKK"),
		crypt_str("get naval now! - discord.gg/QrVU3efyKK"),
		crypt_str("naval for the win - discord.gg/QrVU3efyKK"),
		crypt_str("get good get naval - discord.gg/QrVU3efyKK"),
	};

	static auto lastspammed = 0;

	if (GetTickCount() - lastspammed > 800)
	{
		lastspammed = GetTickCount();

		srand(m_globals()->m_tickcount);
		std::string msg = crypt_str("say ") + chatspam[rand() % 4];

		m_engine()->ExecuteClientCmd(msg.c_str());
	}
}

void misc::AutoCrouch(CUserCmd* cmd)
{
	if (fakelag::get().condition)
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().get_netvars(cmd->m_command_number).m_fFlags & FL_ONGROUND))
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (m_gamerules()->m_bIsValveDS())
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (!g_cfg.keybinds.key[FAKE_DUCK_KEYBIND].active)
	{
		g_ctx.globals.fakeducking = false;
		return;
	}

	if (!g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands != 7)
		return;

	if (m_clientstate()->iChokedCommands >= 7)
		cmd->m_buttons |= IN_DUCK;
	else
		cmd->m_buttons &= ~IN_DUCK;

	g_ctx.globals.fakeducking = true;
}

void misc::SlideWalk(CUserCmd* cmd)
{
	if (!g_ctx.local()->is_alive()) //-V807
		return;

	if (g_ctx.local()->get_move_type() == MOVETYPE_LADDER)
		return;

	if (antiaim::get().condition(cmd, true) && g_cfg.misc.leg_movement == LEG_MOVEMENT_FORCE_SLIDE)
	{
		if (cmd->m_forwardmove > 0.0f)
		{
			cmd->m_buttons |= IN_BACK;
			cmd->m_buttons &= ~IN_FORWARD;
		}
		else if (cmd->m_forwardmove < 0.0f)
		{
			cmd->m_buttons |= IN_FORWARD;
			cmd->m_buttons &= ~IN_BACK;
		}

		if (cmd->m_sidemove > 0.0f)
		{
			cmd->m_buttons |= IN_MOVELEFT;
			cmd->m_buttons &= ~IN_MOVERIGHT;
		}
		else if (cmd->m_sidemove < 0.0f)
		{
			cmd->m_buttons |= IN_MOVERIGHT;
			cmd->m_buttons &= ~IN_MOVELEFT;
		}
	}
	else
	{
		auto buttons = cmd->m_buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

		if (g_cfg.misc.leg_movement == LEG_MOVEMENT_FORCE_SLIDE)
		{
			if (cmd->m_forwardmove <= 0.0f)
				buttons |= IN_BACK;
			else
				buttons |= IN_FORWARD;

			if (cmd->m_sidemove > 0.0f)
				goto LABEL_15;
			else if (cmd->m_sidemove >= 0.0f)
				goto LABEL_18;

			goto LABEL_17;
		}
		else
			goto LABEL_18;

		if (cmd->m_forwardmove <= 0.0f) //-V779
			buttons |= IN_FORWARD;
		else
			buttons |= IN_BACK;

		if (cmd->m_sidemove > 0.0f)
		{
		LABEL_17:
			buttons |= IN_MOVELEFT;
			goto LABEL_18;
		}

		if (cmd->m_sidemove < 0.0f)
			LABEL_15:

		buttons |= IN_MOVERIGHT;

	LABEL_18:
		cmd->m_buttons = buttons;
	}
}

void misc::remove_player_patches()
{
	bool check = 
		(g_cfg.player.enable && 
			(g_cfg.player.type[ENEMY].chams[PLAYER_CHAMS_VISIBLE] ||
				g_cfg.player.type[TEAM].chams[PLAYER_CHAMS_VISIBLE] ||
				g_cfg.player.type[ENEMY].chams[PLAYER_CHAMS_INVISIBLE] ||
				g_cfg.player.type[TEAM].chams[PLAYER_CHAMS_INVISIBLE] ||
				g_cfg.player.backtrack_chams)
		);

	if (check) {

		for (auto i = 1; i <= m_globals()->m_maxclients; ++i)
		{
			auto e = (player_t*)m_entitylist()->GetClientEntity(i);

			if (!e->valid(false, false))
				continue;

			for (size_t patchIndex = 0; patchIndex < 5; ++patchIndex)
			{
				e->m_vecPlayerPatchEconIndices()[patchIndex] = ZERO;
			}
		}
	}
}

void misc::automatic_peek(CUserCmd* cmd, float& wish_yaw)
{
	if (!g_ctx.globals.weapon->is_non_aim() && g_cfg.keybinds.key[AUTO_PEEK_KEYBIND].active)
	{
		if (g_ctx.globals.start_position.IsZero())
		{
			g_ctx.globals.start_position = g_ctx.local()->GetAbsOrigin();

			if (!(engineprediction::get().get_netvars(cmd->m_command_number).m_fFlags & FL_ONGROUND))
			{
				CTraceFilterWorldAndPropsOnly filter;
				CGameTrace trace;

				m_trace()->TraceRay(Ray_t(g_ctx.globals.start_position, g_ctx.globals.start_position - Vector(0.0f, 0.0f, 1000.0f)), MASK_SOLID, &filter, &trace);
				
				if (trace.fraction < 1.0f)
					g_ctx.globals.start_position = trace.endpos + Vector(0.0f, 0.0f, 2.0f);
			}

			g_ctx.globals.autopeek_position = g_ctx.globals.start_position;
		}
		else
		{
			auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

			if (cmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
				g_ctx.globals.fired_shot = true;

			if (g_ctx.globals.fired_shot)
			{
				auto current_position = g_ctx.local()->GetAbsOrigin();
				auto difference = current_position - g_ctx.globals.start_position;

				const auto choked_ticks = (cmd->m_command_number % 2) != 1 ? (14 - m_clientstate()->iChokedCommands) : m_clientstate()->iChokedCommands;

				if (difference.Length2D() > 5.0f)
				{
					auto angle = math::calculate_angle(current_position, g_ctx.globals.start_position);
					wish_yaw = angle.y;

					cmd->m_forwardmove = g_ctx.convars.cl_forwardspeed->GetFloat() - (1.2f * choked_ticks);
					cmd->m_sidemove = 0;
				}
				else
				{
					g_ctx.globals.fired_shot = false;
					g_ctx.globals.start_position.Zero();
				}
			}
		}
	}
	else
	{
		g_ctx.globals.fired_shot = false;
		g_ctx.globals.start_position.Zero();

		if (!g_ctx.globals.in_autopeek)
			g_ctx.globals.autopeek_position.Zero();
	}
}

void misc::fast_ladder(CUserCmd* m_pcmd)
{
	float up_down;

	if (g_ctx.local()->get_move_type() == MOVETYPE_LADDER) {

		if (m_pcmd->m_viewangles.x < 30)
			up_down = -89.0f;
		else
			up_down = 89.0f;

		if (m_pcmd->m_buttons & IN_FORWARD && !GetAsyncKeyState(0x41) && !GetAsyncKeyState(0x44))
		{
			m_pcmd->m_viewangles.x = up_down;

			if (m_pcmd->m_viewangles.y > 135 && m_pcmd->m_viewangles.y < 180 || m_pcmd->m_viewangles.y < -135 && m_pcmd->m_viewangles.y > -180.0f) {
				if (m_pcmd->m_viewangles.y != -89.0f || !(m_pcmd->m_buttons & IN_MOVERIGHT)) {
					m_pcmd->m_viewangles.y = -89.0f;
					m_pcmd->m_buttons |= IN_MOVERIGHT;
				}
			}
			else if (m_pcmd->m_viewangles.y < 135 && m_pcmd->m_viewangles.y > 90 || m_pcmd->m_viewangles.y < 90 && m_pcmd->m_viewangles.y > 45) {
				if (m_pcmd->m_viewangles.y != 179.150f || !(m_pcmd->m_buttons & IN_MOVERIGHT)) {
					m_pcmd->m_viewangles.y = 179.150f;
					m_pcmd->m_buttons |= IN_MOVERIGHT;
				}
			}
			else if (m_pcmd->m_viewangles.y > -90 && m_pcmd->m_viewangles.y < -45 || m_pcmd->m_viewangles.y < -90 && m_pcmd->m_viewangles.y > -135) {
				if (m_pcmd->m_viewangles.y != 0.20f || !(m_pcmd->m_buttons & IN_MOVERIGHT)) {
					m_pcmd->m_viewangles.y = 0.20f;
					m_pcmd->m_buttons |= IN_MOVERIGHT;
				}
			}
			else if (m_pcmd->m_viewangles.y < -0 && m_pcmd->m_viewangles.y > -45 || m_pcmd->m_viewangles.y < 45 && m_pcmd->m_viewangles.y > 0) {
				if (m_pcmd->m_viewangles.y != 89.20f || !(m_pcmd->m_buttons & IN_MOVERIGHT)) {
					m_pcmd->m_viewangles.y = 89.20f;
					m_pcmd->m_buttons |= IN_MOVERIGHT;
				}
			}
		}
	}
}

void misc::ViewModel()
{
	if (g_cfg.esp.viewmodel_fov)
	{
		auto viewFOV = (float)g_cfg.esp.viewmodel_fov + 68.0f;
		
		if (g_ctx.convars.viewmodel_fov->GetFloat() != viewFOV) //-V550
			g_ctx.convars.viewmodel_fov->SetValue(viewFOV);
	}
	
	if (g_cfg.esp.viewmodel_x)
	{
		auto viewX = (float)g_cfg.esp.viewmodel_x / 2.0f;
		
		if (g_ctx.convars.viewmodel_offset_x->GetFloat() != viewX) //-V550
			g_ctx.convars.viewmodel_offset_x->SetValue(viewX);
	}

	if (g_cfg.esp.viewmodel_y)
	{
		auto viewY = (float)g_cfg.esp.viewmodel_y / 2.0f;
		
		if (g_ctx.convars.viewmodel_offset_y->GetFloat() != viewY) //-V550
			g_ctx.convars.viewmodel_offset_y->SetValue(viewY);
		
	}

	if (g_cfg.esp.viewmodel_z)
	{
		auto viewZ = (float)g_cfg.esp.viewmodel_z / 2.0f;
		
		if (g_ctx.convars.viewmodel_offset_z->GetFloat() != viewZ) //-V550
			g_ctx.convars.viewmodel_offset_z->SetValue(viewZ);
	}
}

void misc::FullBright()
{		
	if (!g_cfg.player.enable)
		return;

	if (g_ctx.convars.mat_fullbright->GetBool() != g_cfg.esp.bright)
		g_ctx.convars.mat_fullbright->SetValue(g_cfg.esp.bright);
}

void misc::PovArrows(player_t* e, Color color)
{
	const int screen_center_x = g_ctx.globals.screen_width / 2, screen_center_y = g_ctx.globals.screen_height / 2;

	Vector screen_point, viewangles;
	m_debugoverlay()->ScreenPosition(e->m_vecOrigin(), screen_point);

	if (screen_point.x < 0 || screen_point.y < 0 || screen_point.x > g_ctx.globals.screen_width || screen_point.y > g_ctx.globals.screen_height)
	{
		m_engine()->GetViewAngles(viewangles);
		float desired_offset = viewangles.y - math::calculate_angle(g_ctx.globals.eye_pos, e->m_vecOrigin()).y - 90;

		const auto angle_yaw_rad1 = DEG2RAD(desired_offset);

		const auto new_point_x =
			screen_center_x + ((((g_ctx.globals.screen_width - (g_cfg.player.size * 3)) * .5f) * (g_cfg.player.distance / 100.0f)) * cosf(angle_yaw_rad1)) + (int)(6.0f * (((float)g_cfg.player.distance - 4.f) / 16.0f));

		const auto new_point_y =
			screen_center_y + ((((g_ctx.globals.screen_height - (g_cfg.player.size * 3)) * .5f) * (g_cfg.player.distance / 100.0f)) * sinf(angle_yaw_rad1));

		std::array< Vector2D, 3 > points
		{
			Vector2D(new_point_x - g_cfg.player.size, new_point_y - g_cfg.player.size),
			Vector2D(new_point_x + g_cfg.player.size, new_point_y),
			Vector2D(new_point_x - g_cfg.player.size, new_point_y + g_cfg.player.size)
		};

		math::rotate_triangle(points, desired_offset);
		render::get().triangle(points.at(0), points.at(1), points.at(2), color);
	}
}

void misc::buybot()
{
	if (g_cfg.misc.buybot_enable && g_ctx.globals.should_buy)
	{
		--g_ctx.globals.should_buy;

		if (!g_ctx.globals.should_buy)
		{
			std::string buy;

			switch (g_cfg.misc.buybot1)
			{
			case 1:
				buy += crypt_str("buy g3sg1; ");
				break;
			case 2:
				buy += crypt_str("buy awp; ");
				break;
			case 3:
				buy += crypt_str("buy ssg08; ");
				break;
			}

			switch (g_cfg.misc.buybot2)
			{
			case 1:
				buy += crypt_str("buy elite; ");
				break;
			case 2:
				buy += crypt_str("buy deagle; buy revolver; ");
				break;
			}

			if (g_cfg.misc.buybot3[BUY_ARMOR])
				buy += crypt_str("buy vesthelm; buy vest; ");

			if (g_cfg.misc.buybot3[BUY_TASER])
				buy += crypt_str("buy taser; ");

			if (g_cfg.misc.buybot3[BUY_GRENADES])
				buy += crypt_str("buy molotov; buy hegrenade; buy smokegrenade;");

			if (g_cfg.misc.buybot3[BUY_DEFUSER])
				buy += crypt_str("buy defuser; ");

			m_engine()->ExecuteClientCmd(buy.c_str());
		}
	}
}

void misc::zeus_range()
{
	if (!g_cfg.player.enable)
		return;

	if (!g_cfg.esp.taser_range)
		return;

	if (!g_ctx.local()->is_alive())  //-V807
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (weapon->m_iItemDefinitionIndex() != WEAPON_TASER)
		return;

	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	Vector shoot_pos = g_ctx.local()->get_shoot_position();
	shoot_pos.z -= 30;

	render::get().Draw3DRainbowCircle(shoot_pos, weapon_info->flRange, 1);
}

void misc::NightmodeFix()
{
	static auto in_game = false;

	if (m_engine()->IsInGame() && !in_game)
	{
		in_game = true;

		g_ctx.globals.change_materials = true;
		worldesp::get().changed = true;

		worldesp::get().backup_skybox = g_ctx.convars.sv_skyname->GetString();
		worldesp::get().backup_threedsky = g_ctx.convars.r_3dsky->GetInt();
		return;
	}
	else if (!m_engine()->IsInGame() && in_game)
		in_game = false;

	static auto player_enable = g_cfg.player.enable;

	if (player_enable != g_cfg.player.enable)
	{
		player_enable = g_cfg.player.enable;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting = g_cfg.esp.nightmode;

	if (setting != g_cfg.esp.nightmode)
	{
		setting = g_cfg.esp.nightmode;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting_world = g_cfg.esp.world_color;

	if (setting_world != g_cfg.esp.world_color)
	{
		setting_world = g_cfg.esp.world_color;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting_props = g_cfg.esp.props_color;

	if (setting_props != g_cfg.esp.props_color)
	{
		setting_props = g_cfg.esp.props_color;
		g_ctx.globals.change_materials = true;
	}
}

void misc::desync_arrows()
{
	if (!g_ctx.local()->is_alive())
		return;

	if (!g_cfg.ragebot.enable)
		return;

	if (!g_cfg.antiaim.enable)
		return;

	if ((g_cfg.keybinds.key[MANUAL_BACK_KEYBIND].key <= KEY_NONE || g_cfg.keybinds.key[MANUAL_BACK_KEYBIND].key >= KEY_MAX) && (g_cfg.keybinds.key[MANUAL_LEFT_KEYBIND].key <= KEY_NONE || g_cfg.keybinds.key[MANUAL_LEFT_KEYBIND].key >= KEY_MAX) && (g_cfg.keybinds.key[MANUAL_RIGHT_KEYBIND].key <= KEY_NONE || g_cfg.keybinds.key[MANUAL_RIGHT_KEYBIND].key >= KEY_MAX))
		antiaim::get().manual_side = SIDE_NONE;

	if (!g_cfg.antiaim.flip_indicator)
		return;

	static auto alpha = 1.0f;
	static auto switch_alpha = false;

	if (alpha <= 0.0f || alpha >= 1.0f)
		switch_alpha = !switch_alpha;

	alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
	alpha = math::clamp(alpha, 0.0f, 1.0f);

	auto color = g_cfg.antiaim.flip_indicator_color;
	color.SetAlpha((int)(min(255.0f * alpha, color.a())));

	if (antiaim::get().manual_side == SIDE_BACK)
		render::get().triangle(Vector2D(g_ctx.globals.screen_width / 2, g_ctx.globals.screen_height / 2 + 80), Vector2D(g_ctx.globals.screen_width / 2 - 10, g_ctx.globals.screen_height / 2 + 60), Vector2D(g_ctx.globals.screen_width / 2 + 10, g_ctx.globals.screen_height / 2 + 60), color);
	else if (antiaim::get().manual_side == SIDE_LEFT)
		render::get().triangle(Vector2D(g_ctx.globals.screen_width / 2 - 55, g_ctx.globals.screen_height / 2 + 10), Vector2D(g_ctx.globals.screen_width / 2 - 75, g_ctx.globals.screen_height / 2), Vector2D(g_ctx.globals.screen_width / 2 - 55, g_ctx.globals.screen_height / 2 - 10), color);
	else if (antiaim::get().manual_side == SIDE_RIGHT)
		render::get().triangle(Vector2D(g_ctx.globals.screen_width / 2 + 55, g_ctx.globals.screen_height / 2 - 10), Vector2D(g_ctx.globals.screen_width / 2 + 75, g_ctx.globals.screen_height / 2), Vector2D(g_ctx.globals.screen_width / 2 + 55, g_ctx.globals.screen_height / 2 + 10), color);
}

void misc::ragdolls()
{
	if (!g_cfg.misc.ragdolls)
		return;

	for (auto i = 1; i <= m_entitylist()->GetHighestEntityIndex(); ++i)
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != CCSRagdoll)
			continue;

		auto ragdoll = (ragdoll_t*)e;
		ragdoll->m_vecForce().z = 500000.0f;
	}
}

void misc::rank_reveal()
{
	if (!g_cfg.misc.rank_reveal)
		return;

	using RankReveal_t = bool(__cdecl*)(int*);
	static auto Fn = (RankReveal_t)(g_ctx.addresses.rank_reveal);

	int array[3] = 
	{
		0,
		0,
		0
	};

	Fn(array);
}

void misc::fast_stop(CUserCmd* m_pcmd, float wish_yaw)
{
	if (!g_cfg.misc.fast_stop)
		return;

	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND))
		return;

	Vector velocity = g_ctx.local()->m_vecVelocity();
	Vector direction;

	math::vector_angles(velocity, direction);
	float speed = velocity.Length2D();

	direction.y = wish_yaw - direction.y;

	Vector forward;
	math::angle_vectors(direction, forward);

	Vector right = (forward + 0.217812) * -speed;
	Vector left = (forward + -0.217812) * -speed;

	Vector move_forward = (forward + 0.217812) * -speed;
	Vector move_backward = (forward + -0.217812) * -speed;

	if (speed > 21.5f) 
	{
		if (!(movement.in_moveleft))
		{
			m_pcmd->m_sidemove += +left.y;
		}

		if (!(movement.in_moveright))
		{
			m_pcmd->m_sidemove -= -right.y;
		}

		if (!(movement.in_forward))
		{
			if (movement.in_moveleft || movement.in_moveright)
				return;

			m_pcmd->m_forwardmove += +move_forward.x;
		}

		if (!(movement.in_back))
		{
			if (movement.in_moveleft || movement.in_moveright)
				return;

			m_pcmd->m_forwardmove -= -move_backward.x;
		}
	}
}
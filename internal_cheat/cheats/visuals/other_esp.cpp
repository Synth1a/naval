// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "other_esp.h"
#include "..\ui\menu.h"
#include "..\autowall\penetration.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "..\misc\prediction_system.h"
#include "..\lagcompensation\local_animations.h"
#include "..\networking\networking.h"

void ConvertBGRtoRGB(const uint8_t* rgba, uint8_t* out, const size_t size)
{
	auto in = reinterpret_cast<const uint32_t*>(rgba);
	auto buf = reinterpret_cast<uint32_t*>(out);
	for (auto i = 0u; i < (size / 4); ++i)
	{
		const auto pixel = *in++;
		*buf++ = (pixel & 0xFF00FF00) | ((pixel & 0xFF0000) >> 16) | ((pixel & 0xFF) << 16);
	}
}

LPDIRECT3DTEXTURE9 get_steam_image(CSteamID SteamId)
{
	LPDIRECT3DTEXTURE9 output_image;

	int iImage = SteamFriends->GetSmallFriendAvatar(SteamId);

	if (iImage == -1)
		return nullptr;

	uint32 uAvatarWidth, uAvatarHeight;

	if (!SteamUtils->GetImageSize(iImage, &uAvatarWidth, &uAvatarHeight))
		return nullptr;

	const int uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	uint8* pAvatarRGBA = new uint8[uImageSizeInBytes];

	if (!SteamUtils->GetImageRGBA(iImage, pAvatarRGBA, uImageSizeInBytes))
	{
		delete[] pAvatarRGBA;
		return nullptr;
	}

	auto res = g_ctx.globals.draw_device->CreateTexture(uAvatarWidth, uAvatarHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &output_image, nullptr);

	std::vector<uint8_t> texData;
	texData.resize(uAvatarWidth * uAvatarHeight * 4u);

	ConvertBGRtoRGB(pAvatarRGBA, texData.data(), uAvatarWidth * uAvatarHeight * 4u);

	D3DLOCKED_RECT rect;

	res = output_image->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);
	auto src = texData.data();
	auto dst = reinterpret_cast<uint8_t*>(rect.pBits);

	for (auto y = 0u; y < uAvatarHeight; ++y)
	{
		std::copy(src, src + (uAvatarWidth * 4), dst);

		src += uAvatarWidth * 4;
		dst += rect.Pitch;
	}
	res = output_image->UnlockRect(0);
	delete[] pAvatarRGBA;

	return output_image;
}

void otheresp::initialize_spectators(spectators_list* spect, int i)
{
	if (!g_ctx.available())
	{
		spect->active_user = false;
		player_avatar[i] = nullptr;

		return;
	}

	player_t* entity = (player_t*)m_entitylist()->GetClientEntity(i);
	if (!entity || entity == g_ctx.local() || entity->IsDormant())
	{
		spect->active_user = false;
		return;
	}

	player_info_t info;
	if (!m_engine()->GetPlayerInfo(i, &info))
	{
		spect->active_user = false;
		player_avatar[i] = nullptr;

		return;
	}

	if (!player_avatar[i])
		player_avatar[i] = get_steam_image(CSteamID((uint64)info.steamID64));

	CBaseHandle obs = entity->m_hObserverTarget();
	player_t* spec = (player_t*)m_entitylist()->GetClientEntityFromHandle(obs);

	if (spec)
	{
		if (g_ctx.local()->is_alive())
		{
			if (spec->EntIndex() != g_ctx.local()->EntIndex())
			{
				spect->active_user = false;
				return;
			}

			spect->active_user = true;
			spect->user_name = info.szName;
			spect->user_profile = entity->is_bot() ? nullptr : player_avatar[entity->EntIndex()];
		}
		else
		{
			CBaseHandle local_obs = g_ctx.local()->m_hObserverTarget();
			player_t* local_spec = (player_t*)m_entitylist()->GetClientEntityFromHandle(local_obs);

			if (!local_spec)
			{
				spect->active_user = false;
				return;
			}

			if (spec->EntIndex() != local_spec->EntIndex())
			{
				spect->active_user = false;
				return;
			}

			spect->active_user = true;
			spect->user_name = info.szName;
			spect->user_profile = entity->is_bot() ? nullptr : player_avatar[entity->EntIndex()];
		}
	}
	else
		spect->active_user = false;
}

void otheresp::spectator_list()
{
	// NOTE: ALWAYS USE THE OLDEST STORED SYSTEM, NOT THE CURRENT ONE OR THE ANIMATION WILL NOT APPEARS SINCE THE ANIMATION
	// IS NOT APPLIED YET BEFORE THE CURRENT STORED SYSTEM.

	if (!g_cfg.misc.spectators_list)
		return;

	static std::vector<spectators_list> old_spectator;

	// initialize our spectator before working with something else.
	for (int i = 1; i < m_globals()->m_maxclients; ++i)
		initialize_spectators(&spectator[i], i);

	// if our old spectator is empty.
	if (old_spectator.empty())
	{
		// resize it to 65 since it's the max and copy our first vector of our current spectator.
		old_spectator.resize(65);
		std::memcpy(&*old_spectator.begin(), spectator, sizeof(spectator));

		// if old spectator was an active user then we can add that user.
		for (int i = 1; i < m_globals()->m_maxclients; ++i)
		{
			if (old_spectator[i].active_user)
				spect_list.add(spectator[i].user_name, crypt_str(""), i, true);
		}
	}

	// another loops.
	for (int i = 1; i < m_globals()->m_maxclients; ++i)
	{
		// if our old spectator was not an active user but our current is an active user then remove the user.
		if (!old_spectator[i].active_user && spectator[i].active_user)
			spect_list.remove(spectator[i].user_name, crypt_str(""), i, true, spectator[i].user_profile);

		// except if our old spectator was an active user but our current is not then add it.
		else if (old_spectator[i].active_user && !spectator[i].active_user)
			spect_list.add(spectator[i].user_name, crypt_str(""), i, true, spectator[i].user_profile);
	}

	// data to be used later.
	static ImVec2 old_pos;

	// run the code if there are differences.
	if (g_cfg.menu.spectators_list.x != old_pos.x || g_cfg.menu.spectators_list.y != old_pos.y)
		ImGui::SetNextWindowPos(ImVec2(g_cfg.menu.spectators_list.x, g_cfg.menu.spectators_list.y));

	// run the drawing codes if we keybind indicators are enabled.
	// set window size with the height added by our binds size so it's dynamical.
	ImGui::SetNextWindowSize(ImVec2(140 + spect_list.last_text_size, 20 + spect_list.last_size + 20));

	// draw our spectator list system.
	ImGui::Begin(crypt_str("##SPECTATORS"), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
	{
		// store position for config.
		old_pos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

		// push our font.
		ImGui::PushFont(g_ctx.fonts.keybind_font);

		// data to be used later.
		auto s = ImVec2(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y * 2);
		auto p = ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x, ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y);
		auto draw = ImGui::GetBackgroundDrawList();

		{//main.
			std::vector<std::string> spect = { crypt_str("Spect"), crypt_str("ators") };
			ImVec2 spect_size[2] = { ImGui::CalcTextSize(spect.at(0).c_str()), ImGui::CalcTextSize(spect.at(1).c_str()) };

			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + 20), ImColor(30, 31, 32, 200));
			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + 3), ImColor(g_cfg.menu.menu_theme.r(), g_cfg.menu.menu_theme.g(), g_cfg.menu.menu_theme.b()), 12, ImDrawCornerFlags_Top);
			draw->AddText(ImVec2(p.x + 10, p.y + 11 - spect_size[0].y / 2), ImColor(g_cfg.menu.menu_theme.r(), g_cfg.menu.menu_theme.g(), g_cfg.menu.menu_theme.b()), spect.at(0).c_str());
			draw->AddText(ImVec2(p.x + 10 + spect_size[0].x, p.y + 11 - spect_size[0].y / 2), ImColor(220, 220, 220), spect.at(1).c_str());
		}
		{//spectator list.
			spect_list.render(p, s, draw);
		}

		// pop our font now.
		ImGui::PopFont();
	}

	// if our spectator list position is not the same as old position then we set it to current window position.
	if (g_cfg.menu.spectators_list.x != old_pos.x || g_cfg.menu.spectators_list.y != old_pos.y)
	{
		g_cfg.menu.spectators_list.x = ImGui::GetWindowPos().x;
		g_cfg.menu.spectators_list.y = ImGui::GetWindowPos().y;

		// storing another position since now it's the same.
		old_pos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
	}

	// finish our code.
	ImGui::End();

	std::memcpy(&*old_spectator.begin(), spectator, sizeof(spectator));
}

void otheresp::keybind_list()
{
	// NOTE: ALWAYS USE THE OLDEST STORED SYSTEM, NOT THE CURRENT ONE OR THE ANIMATION WILL NOT APPEARS SINCE THE ANIMATION
	// IS NOT APPLIED YET BEFORE THE CURRENT STORED SYSTEM.

	if (!g_cfg.menu.keybinds)
		return;

	static std::vector<key_bind> old_binds;
	static int old_manual_side;

	// if our old binds is empty.
	if (old_binds.empty())
	{
		// resize it to keybind max.
		old_binds.resize(KEYBIND_MAX);

		// set our old data.
		old_manual_side = antiaim::get().manual_side;
		std::memcpy(&*old_binds.begin(), g_cfg.keybinds.key, sizeof(g_cfg.keybinds.key));

		// loop through keybind max number.
		for (int i = 0; i < KEYBIND_MAX; ++i)
		{
			// do not draw thirdperson.
			if (i == THIRDPERSON_KEYBIND)
				continue;

			// manual anti-aim indicators.
			if (i > HIDE_SHOTS_KEYBIND && i < FLIP_DESYNC_KEYBIND)
			{
				switch (i)
				{
				case MANUAL_BACK_KEYBIND: // back.
					if (antiaim::get().manual_side == SIDE_BACK)
						bind_list.add(crypt_str("Manual"), crypt_str("back"), i);
					break;

				case MANUAL_LEFT_KEYBIND: // left.
					if (antiaim::get().manual_side == SIDE_LEFT)
						bind_list.add(crypt_str("Manual"), crypt_str("left"), i);
					break;

				case MANUAL_RIGHT_KEYBIND: // right.
					if (antiaim::get().manual_side == SIDE_RIGHT)
						bind_list.add(crypt_str("Manual"), crypt_str("right"), i);
					break;
				}

				// dont run the code below if we are on this specific binds.
				continue;
			}

			// draws normal binds.
			if (old_binds[i].active)
				bind_list.add(std::string(get_bind_name(i)), get_bind_type(g_cfg.keybinds.key[i].mode), i);
		}
	}

	// another loops through keybind max number.
	for (int i = 0; i < KEYBIND_MAX; ++i)
	{
		// do not draw thirdperson.
		if (i == THIRDPERSON_KEYBIND)
			continue;

		// manual anti-aim indicators.
		if (i > HIDE_SHOTS_KEYBIND && i < FLIP_DESYNC_KEYBIND)
		{
			switch (i)
			{
			case MANUAL_BACK_KEYBIND: // back.
				if (old_manual_side != SIDE_BACK && antiaim::get().manual_side == SIDE_BACK)
					bind_list.remove(crypt_str("Manual"), crypt_str("back"), i);
				else if (old_manual_side == SIDE_BACK && antiaim::get().manual_side != SIDE_BACK)
					bind_list.add(crypt_str("Manual"), crypt_str("back"), i);
				break;

			case MANUAL_LEFT_KEYBIND: // left.
				if (old_manual_side != SIDE_LEFT && antiaim::get().manual_side == SIDE_LEFT)
					bind_list.remove(crypt_str("Manual"), crypt_str("left"), i);
				else if (old_manual_side == SIDE_LEFT && antiaim::get().manual_side != SIDE_LEFT)
					bind_list.add(crypt_str("Manual"), crypt_str("left"), i);
				break;

			case MANUAL_RIGHT_KEYBIND: // right.
				if (old_manual_side != SIDE_RIGHT && antiaim::get().manual_side == SIDE_RIGHT)
					bind_list.remove(crypt_str("Manual"), crypt_str("right"), i);
				else if (old_manual_side == SIDE_RIGHT && antiaim::get().manual_side != SIDE_RIGHT)
					bind_list.add(crypt_str("Manual"), crypt_str("right"), i);
				break;
			}

			// dont run the code below if we are on this specific binds.
			continue;
		}

		// if our old binds are not active but our current are active then remove our bind list.
		if (!old_binds[i].active && g_cfg.keybinds.key[i].active)
			bind_list.remove(std::string(get_bind_name(i)), get_bind_type(g_cfg.keybinds.key[i].mode), i);

		// except if our old binds are active but our current are not active then we add our bind list.
		else if (old_binds[i].active && !g_cfg.keybinds.key[i].active)
			bind_list.add(std::string(get_bind_name(i)), get_bind_type(g_cfg.keybinds.key[i].mode), i);
	}

	// data to be used later.
	static ImVec2 old_pos;

	// run the code if there are differences.
	if (g_cfg.menu.keybinds_list.x != old_pos.x || g_cfg.menu.keybinds_list.y != old_pos.y)
		ImGui::SetNextWindowPos(ImVec2(g_cfg.menu.keybinds_list.x, g_cfg.menu.keybinds_list.y));

	// run the drawing codes if we keybind indicators are enabled.
	// set window size with the height added by our binds size so it's dynamical.
	ImGui::SetNextWindowSize(ImVec2(140 + bind_list.last_text_size, 20 + bind_list.last_size + 20));

	// draw our keybinds system.
	ImGui::Begin(crypt_str("##KEY_BINDS"), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
	{
		// store position for config.
		old_pos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

		// push our font.
		ImGui::PushFont(g_ctx.fonts.keybind_font);

		// data to be used later.
		auto s = ImVec2(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y * 2);
		auto p = ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x, ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y);
		auto draw = ImGui::GetBackgroundDrawList();

		{//main
			std::vector<std::string> keybind = { crypt_str("Key"), crypt_str("binds") };
			ImVec2 keybind_size[2] = { ImGui::CalcTextSize(keybind.at(0).c_str()), ImGui::CalcTextSize(keybind.at(1).c_str()) };

			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + 20), ImColor(30, 31, 32, 200));
			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + 3), ImColor(g_cfg.menu.menu_theme.r(), g_cfg.menu.menu_theme.g(), g_cfg.menu.menu_theme.b()), 12, ImDrawCornerFlags_Top);
			draw->AddText(ImVec2(p.x + 10, p.y + 11 - keybind_size[0].y / 2), ImColor(g_cfg.menu.menu_theme.r(), g_cfg.menu.menu_theme.g(), g_cfg.menu.menu_theme.b()), keybind.at(0).c_str());
			draw->AddText(ImVec2(p.x + 10 + keybind_size[0].x, p.y + 11 - keybind_size[0].y / 2), ImColor(220, 220, 220), keybind.at(1).c_str());
		}
		{//bind
			bind_list.render(p, s, draw);
		}

		// pop our font now.
		ImGui::PopFont();
	}

	// if our keybind list position is not the same as old position then we set it to current window position.
	if (g_cfg.menu.keybinds_list.x != old_pos.x || g_cfg.menu.keybinds_list.y != old_pos.y)
	{
		g_cfg.menu.keybinds_list.x = ImGui::GetWindowPos().x;
		g_cfg.menu.keybinds_list.y = ImGui::GetWindowPos().y;

		// storing another position since now it's the same.
		old_pos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
	}

	// finish our code.
	ImGui::End();

	old_manual_side = antiaim::get().manual_side;
	std::memcpy(&*old_binds.begin(), g_cfg.keybinds.key, sizeof(g_cfg.keybinds.key));
}

void otheresp::watermark()
{
	if (!g_cfg.menu.watermark)
		return;

	// texts for watermark.
	std::string texts = crypt_str("naval / ");

	// if we are connected to server, try to show additional information, if not. just display not connected text.
	if (m_engine()->IsConnected())
		texts += crypt_str("delay: ") + std::to_string(networking::get().ping()) + crypt_str("ms / ") + std::to_string((int)networking::get().tickrate()) + crypt_str("tick");
	else
		texts += crypt_str(" not connected");

	// calculate our text size.
	int textlen = g_ctx.fonts.generic_font->CalcTextSizeA(12.0f, FLT_MAX, NULL, texts.c_str()).x + 25;

	// draw.
	auto draw_list = ImGui::GetOverlayDrawList();
	
	draw_list->AddRectFilled(ImVec2((g_ctx.globals.screen_width / g_cfg.menu.dpi_scale) - textlen, 11), ImVec2((g_ctx.globals.screen_width / g_cfg.menu.dpi_scale) - 10, 31), ImColor(30, 31, 32, 200));
	draw_list->AddRectFilled(ImVec2((g_ctx.globals.screen_width / g_cfg.menu.dpi_scale) - textlen, 11), ImVec2((g_ctx.globals.screen_width / g_cfg.menu.dpi_scale) - 10, 14), ImColor(g_cfg.menu.menu_theme.r(), g_cfg.menu.menu_theme.g(), g_cfg.menu.menu_theme.b()), 3, ImDrawCornerFlags_Top);

	// use our font.
	ImGui::PushFont(g_ctx.fonts.generic_font);

	// draw our texts.
	draw_list->AddText(ImVec2((g_ctx.globals.screen_width / g_cfg.menu.dpi_scale) - textlen + 7, 15), ImColor(255, 255, 255, 255), texts.c_str());
	
	// now pop our font.
	ImGui::PopFont();
}

void otheresp::penetration_reticle()
{
	if (!g_cfg.player.enable)
		return;

	if (!g_cfg.esp.penetration_reticle)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto color = Color(255, 0, 0, g_cfg.esp.penetration_reticle_type ? 255 : 40);

	bool valid_player_hit = (misc::get().pen_data.m_target && misc::get().pen_data.m_target->m_iTeamNum() != g_ctx.local()->m_iTeamNum());

	if (valid_player_hit)
		color = Color(255, 255, 0, g_cfg.esp.penetration_reticle_type ? 255 : 40);
	else if (misc::get().pen_data.m_pen)
		color = Color(0, 255, 0, g_cfg.esp.penetration_reticle_type ? 255 : 40);

	if (g_cfg.esp.penetration_reticle_type == 0)
	{
		CGameTrace enterTrace;
		Ray_t ray;

		Vector viewangles;
		m_engine()->GetViewAngles(viewangles);

		Vector direction;
		math::angle_vectors(viewangles, direction);

		Vector start = g_ctx.local()->get_shoot_position(true);

		auto maxrange = weapon->get_csweapon_info()->flRange * 2;
		Vector end = start + (direction * maxrange);

		uint32_t filter_[4] =
		{
			*(uint32_t*)(g_ctx.addresses.trace_filter_simple),
			(uint32_t)g_ctx.local(),
			0,
			0
		};

		m_trace()->TraceRay(Ray_t(start, end), MASK_SHOT | CONTENTS_GRATE, (ITraceFilter*)&filter_, &enterTrace);

		render::get().filled_rect_world(direction, enterTrace, color, 5);
	}
	else
	{
		render::get().rect_filled((g_ctx.globals.screen_width / 2) - 2, (g_ctx.globals.screen_height / 2) - 2, 5, 5, Color(0, 0, 0, 150));
		render::get().rect_filled((g_ctx.globals.screen_width / 2) - 1, (g_ctx.globals.screen_height / 2) - 1, 3, 3, color);
	}
}

void otheresp::force_crosshair()
{
	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto is_scoped = g_ctx.globals.scoped && weapon->is_scopable() && weapon->m_zoomLevel();
	g_ctx.convars.weapon_debug_spread_show->SetValue(is_scoped || !g_cfg.esp.force_crosshair || !g_cfg.player.enable ? 0 : 3);
}

void otheresp::indicators()
{
	if (!g_ctx.local()->is_alive()) //-V807
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (g_cfg.esp.indicators[INDICATOR_FAKE] && (antiaim::get().type == ANTIAIM_LEGIT || g_cfg.antiaim.type[antiaim::get().type].desync))
	{
		auto color = Color(130, 20, 20);
		auto animstate = g_ctx.local()->get_animation_state();

		if (animstate && local_animations::get().local_data.fake_animstate)
		{
			auto delta = fabs(math::normalize_yaw(animstate->m_flFootYaw - local_animations::get().local_data.fake_animstate->m_flFootYaw));
			auto desync_delta = max(g_ctx.local()->get_max_desync_delta(), 58.0f);

			color = Color(130, 20 + (int)(min(delta / desync_delta, 1.0f) * 150.0f), 20);
		}

		m_indicators.push_back(m_indicator(crypt_str("FAKE"), color));
	}

	if (g_cfg.esp.indicators[INDICATOR_DESYNC_SIDE] && (antiaim::get().type == ANTIAIM_LEGIT && g_cfg.antiaim.desync == 1 || antiaim::get().type != ANTIAIM_LEGIT && g_cfg.antiaim.type[antiaim::get().type].desync == 1) && !antiaim::get().condition(g_ctx.get_command()))
	{
		auto side = antiaim::get().desync_angle > 0.0f ? crypt_str("RIGHT") : crypt_str("LEFT");

		if (antiaim::get().type == ANTIAIM_LEGIT)
			side = antiaim::get().desync_angle > 0.0f ? crypt_str("LEFT") : crypt_str("RIGHT");

		m_indicators.push_back(m_indicator(side, Color(130, 170, 20)));
	}

	auto choke_indicator = false;

	if (g_cfg.esp.indicators[INDICATOR_CHOKE] && !fakelag::get().condition)
	{
		m_indicators.push_back(m_indicator((crypt_str("CHOKE: ") + std::to_string(fakelag::get().max_choke)), Color(130, 170, 20)));
		choke_indicator = true;
	}

	if (g_cfg.esp.indicators[INDICATOR_DAMAGE] && g_cfg.keybinds.key[DAMAGE_OVERRIDE_KEYBIND].active && !weapon->is_non_aim())
	{
		if (g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].minimum_override_damage > 100)
			m_indicators.push_back(m_indicator((crypt_str("DAMAGE: HP + ") + std::to_string(g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].minimum_override_damage - 100)), Color(130, 170, 20)));
		else
			m_indicators.push_back(m_indicator((crypt_str("DAMAGE: ") + std::to_string(g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].minimum_override_damage)), Color(130, 170, 20)));
	}

	if (g_cfg.esp.indicators[INDICATOR_SAFE_POINTS] && g_cfg.keybinds.key[SAFE_POINT_KEYBIND].active && !weapon->is_non_aim())
		m_indicators.push_back(m_indicator(crypt_str("SAFE POINTS"), Color(130, 170, 20)));

	if (g_cfg.esp.indicators[INDICATOR_BODY_AIM] && g_cfg.keybinds.key[BODY_AIM_KEYBIND].active && !weapon->is_non_aim())
		m_indicators.push_back(m_indicator(crypt_str("BODY AIM"), Color(130, 170, 20)));

	if (g_cfg.esp.indicators[INDICATOR_RESOLVER_OVERRIDE] && g_cfg.keybinds.key[RESOLVER_OVERRIDE_KEYBIND].active)
		m_indicators.push_back(m_indicator(crypt_str("OVERRIDE"), Color::White));

	if (choke_indicator)
		return;

	if (g_cfg.esp.indicators[INDICATOR_DT] && g_cfg.ragebot.double_tap && g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].key < KEY_MAX && g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].active)
		m_indicators.push_back(m_indicator(crypt_str("DT"), !g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && !weapon->is_grenade() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && weapon->can_fire(false) ? Color(130, 170, 20) : Color(130, 20, 20)));

	if (g_cfg.esp.indicators[INDICATOR_HS] && g_cfg.antiaim.hide_shots && g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].key < KEY_MAX && g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].active)
		m_indicators.push_back(m_indicator(crypt_str("HS"), !g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check ? Color(130, 170, 20) : Color(130, 20, 20)));
}

void otheresp::draw_indicators()
{
	if (!g_ctx.local()->is_alive()) //-V807
		return;

	auto h = g_ctx.globals.screen_height - 325;

	for (auto& indicator : m_indicators)
	{
		render::get().text(fonts[INDICATORFONT], 27, h, indicator.m_color, HFONT_CENTERED_Y, indicator.m_text.c_str());
		h -= 25;
	}

	if (!m_indicators.empty())
		m_indicators.clear();
}

void otheresp::hitmarker_paint()
{
	if (!g_cfg.esp.hitmarker[0] && !g_cfg.esp.hitmarker[1])
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if (!g_ctx.local()->is_alive())
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if (hitmarker.hurt_time + 0.7f > m_globals()->m_curtime)
	{
		if (g_cfg.esp.hitmarker[0])
		{
			auto alpha = (int)((hitmarker.hurt_time + 0.7f - m_globals()->m_curtime) * 255.0f);
			hitmarker.hurt_color.SetAlpha(alpha);

			auto offset = 7.0f - (float)alpha / 255.0f * 7.0f;

			render::get().line(g_ctx.globals.screen_width / 2 + 5 + offset, g_ctx.globals.screen_height / 2 - 5 - offset, g_ctx.globals.screen_width / 2 + 12 + offset, g_ctx.globals.screen_height / 2 - 12 - offset, hitmarker.hurt_color);
			render::get().line(g_ctx.globals.screen_width / 2 + 5 + offset, g_ctx.globals.screen_height / 2 + 5 + offset, g_ctx.globals.screen_width / 2 + 12 + offset, g_ctx.globals.screen_height / 2 + 12 + offset, hitmarker.hurt_color);
			render::get().line(g_ctx.globals.screen_width / 2 - 5 - offset, g_ctx.globals.screen_height / 2 + 5 + offset, g_ctx.globals.screen_width / 2 - 12 - offset, g_ctx.globals.screen_height / 2 + 12 + offset, hitmarker.hurt_color);
			render::get().line(g_ctx.globals.screen_width / 2 - 5 - offset, g_ctx.globals.screen_height / 2 - 5 - offset, g_ctx.globals.screen_width / 2 - 12 - offset, g_ctx.globals.screen_height / 2 - 12 - offset, hitmarker.hurt_color);
		}

		if (g_cfg.esp.hitmarker[1])
		{
			Vector world;

			if (math::world_to_screen(hitmarker.point, world))
			{
				auto alpha = (int)((hitmarker.hurt_time + 0.7f - m_globals()->m_curtime) * 255.0f);
				hitmarker.hurt_color.SetAlpha(alpha);

				auto offset = 7.0f - (float)alpha / 255.0f * 7.0f;

				render::get().line(world.x + 5 + offset, world.y - 5 - offset, world.x + 12 + offset, world.y - 12 - offset, hitmarker.hurt_color);
				render::get().line(world.x + 5 + offset, world.y + 5 + offset, world.x + 12 + offset, world.y + 12 + offset, hitmarker.hurt_color);
				render::get().line(world.x - 5 - offset, world.y + 5 + offset, world.x - 12 - offset, world.y + 12 + offset, hitmarker.hurt_color);
				render::get().line(world.x - 5 - offset, world.y - 5 - offset, world.x - 12 - offset, world.y - 12 - offset, hitmarker.hurt_color);
			}
		}
	}
}

void otheresp::damage_marker_paint()
{
	for (auto i = 1; i < m_globals()->m_maxclients; ++i) //-V807
	{
		if (damage_marker[i].hurt_time + 2.0f > m_globals()->m_curtime)
		{
			Vector screen;

			if (!math::world_to_screen(damage_marker[i].position, screen))
				continue;

			auto alpha = (int)((damage_marker[i].hurt_time + 2.0f - m_globals()->m_curtime) * 127.5f);
			damage_marker[i].hurt_color.SetAlpha(alpha);

			render::get().text(fonts[DAMAGE_MARKER], screen.x, screen.y, damage_marker[i].hurt_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("%i"), damage_marker[i].damage);
		}
	}
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device);

void otheresp::spread_crosshair()
{
	if (!g_cfg.player.enable)
		return;

	if (!g_cfg.esp.show_spread)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	draw_circe((float)g_ctx.globals.screen_width * 0.5f, (float)g_ctx.globals.screen_height * 0.5f, weapon->get_inaccuracy() * 500.0f, 50, D3DCOLOR_RGBA(g_cfg.esp.show_spread_color.r(), g_cfg.esp.show_spread_color.g(), g_cfg.esp.show_spread_color.b(), g_cfg.esp.show_spread_color.a()), D3DCOLOR_RGBA(0, 0, 0, 0), g_ctx.globals.draw_device);
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2 = nullptr;
	std::vector <CUSTOMVERTEX2> circle(resolution + 2);

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0.0f;

	circle[0].rhw = 1.0f;
	circle[0].color = color2;

	for (auto i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - radius * sin(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0.0f;

		circle[i].rhw = 1.0f;
		circle[i].color = color;
	}

	device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX2), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, nullptr); //-V107

	if (!g_pVB2)
		return;

	void* pVertices;

	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX2), (void**)&pVertices, 0); //-V107
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX2));
	g_pVB2->Unlock();

	device->SetTexture(0, nullptr);
	device->SetPixelShader(nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX2));
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);

	g_pVB2->Release();
}

void otheresp::automatic_peek_indicator()
{
	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	static auto position = ZERO;

	if (!g_ctx.globals.start_position.IsZero())
		position = g_ctx.globals.autopeek_position;

	if (position.IsZero())
		return;

	static float progress;
	static bool in_transition;

	if (!weapon->is_non_aim() && g_cfg.keybinds.key[AUTO_PEEK_KEYBIND].active)
	{
		in_transition = false;

		if (!g_ctx.globals.in_autopeek)
		{
			g_ctx.globals.in_autopeek = true;
		}
	}
	else
	{
		progress -= m_globals()->m_frametime * 8.f + (progress / 100);
		progress = math::clamp(progress, 0.f, 1.f);

		if (!progress)
			g_ctx.globals.in_autopeek = false;
		else
		{
			in_transition = true;
			g_ctx.globals.in_autopeek = true;
		}
	}

	if (g_ctx.globals.in_autopeek && !in_transition)
	{
		progress += m_globals()->m_frametime * 8.f + (progress / 100);
		progress = math::clamp(progress, 0.f, 1.f);
	}

	render::get().Draw3DFilledCircle(position, 15.0f * progress, g_ctx.globals.fired_shot ? Color(30, 240, 30, 130) : Color(240, 30, 30, 130), 1);
}

void otheresp::reset_data()
{
	this->hitmarker.point = ZERO;
	this->hitmarker.hurt_time = FLT_MIN;

	if (!this->m_indicators.empty())
		this->m_indicators.clear();

	std::memset(this->damage_marker, 0, sizeof(this->damage_marker));
}
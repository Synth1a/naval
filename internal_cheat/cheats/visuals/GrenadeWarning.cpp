// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "GrenadeWarning.h"
#include "..\misc\prediction_system.h"

static const char* index_to_grenade_name_icon(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE: return crypt_arr("k"); break;
	case WEAPON_HEGRENADE: return crypt_arr("j"); break;
	case WEAPON_MOLOTOV: return crypt_arr("l"); break;
	case WEAPON_INCGRENADE: return crypt_arr("n"); break;
	}
}

void draw_arc(int x, int y, int radius, int start_angle, int percent, int thickness, Color color) {
	auto precision = (2 * M_PI) / 30;
	auto step = M_PI / 180;
	auto inner = radius - thickness;
	auto end_angle = (start_angle + percent) * step;
	auto start_angles = (start_angle * M_PI) / 180;

	for (; radius > inner; --radius) {
		for (auto angle = start_angles; angle < end_angle; angle += precision) {
			auto cx = std::round(x + radius * std::cos(angle));
			auto cy = std::round(y + radius * std::sin(angle));

			auto cx2 = std::round(x + radius * std::cos(angle + precision));
			auto cy2 = std::round(y + radius * std::sin(angle + precision));

			render::get().line(cx, cy, cx2, cy2, color);
		}
	}
}

void DrawBeamPaw(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS; //TE_BEAMPOINTS
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = crypt_str("sprites/purplelaser1.vmt");//sprites/purplelaser1.vmt
	beamInfo.m_pszHaloName = crypt_str("sprites/purplelaser1.vmt");//sprites/purplelaser1.vmt
	beamInfo.m_flHaloScale = 0;//0
	beamInfo.m_flWidth = g_cfg.esp.proximity_tracers_width;//11
	beamInfo.m_flEndWidth = g_cfg.esp.proximity_tracers_end_width;//11
	beamInfo.m_flFadeLength = 1.0f;
	beamInfo.m_flAmplitude = 2.3;
	beamInfo.m_flBrightness = 255.f;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;//40
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 0.03f;
	beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM; //FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM

	Beam_t* myBeam = m_viewrenderbeams()->CreateBeamPoints(beamInfo);

	if (myBeam)
		m_viewrenderbeams()->DrawBeam(myBeam);
}

bool grenadewarning::data_t::draw() const
{
	if (m_path.size() <= 1 || m_globals()->m_curtime >= m_expire_time)
		return false;

	Vector vLocalOrigin = g_ctx.local()->m_vecOrigin();
	
	float distance = vLocalOrigin.DistTo(m_origin) / 12;

	if (distance > 200.f)
		return false;

	static auto size = Vector2D(35.0f, 5.0f);

	int alpha_damage = 0;

	if (m_index == WEAPON_HEGRENADE && distance <= 20)
		alpha_damage = 255 - 255 * (distance / 20);
	else if ((m_index == WEAPON_MOLOTOV || m_index == WEAPON_INCGRENADE) && distance <= 15)
		alpha_damage = 255 - 255 * (distance / 15);

	float percent = ((m_expire_time - m_globals()->m_curtime) / TICKS_TO_TIME(m_tick));
	auto prev_screen = ZERO;
	auto prev_on_screen = math::world_to_screen(std::get< Vector >(m_path.front()), prev_screen);

	Vector screenPos;
	Vector vExplodeOrigin = std::get< Vector >(m_path.back());

	if (!g_ctx.local()->is_alive())
		vLocalOrigin = m_input()->m_vecCameraOffset;

	for (auto i = 1; i < m_path.size(); ++i)
	{
		auto cur_screen = ZERO, last_cur_screen = ZERO;
		const auto cur_on_screen = math::world_to_screen(std::get< Vector >(m_path.at(i)), cur_screen);
		
		if (prev_on_screen && cur_on_screen)
		{
			if (g_cfg.esp.grenade_proximity_tracers_mode != 0)
			{
				auto color = g_cfg.esp.grenade_proximity_tracers_colors;

				if (g_cfg.esp.grenade_proximity_tracers_rainbow)
					color = Color::FromHSB((360 / m_path.size() * i) / 360.f, 0.9f, 0.8f);

				if (g_cfg.esp.grenade_proximity_tracers_mode == 1) 
					render::get().line((int)prev_screen.x, (int)prev_screen.y, (int)cur_screen.x, (int)cur_screen.y, color, 1);
				else if (g_cfg.esp.grenade_proximity_tracers_mode == 2) 
					DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), color);
			}
		}

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
	}

	render::get().circle_filled(prev_screen.x, prev_screen.y - size.y * 0.5f, 60, 20, g_cfg.esp.grenade_proximity_warning_inner_color);
	render::get().circle_filled(prev_screen.x, prev_screen.y - size.y * 0.5f, 60, 20, Color(g_cfg.esp.grenade_proximity_warning_inner_danger_color.r(), g_cfg.esp.grenade_proximity_warning_inner_danger_color.g(), g_cfg.esp.grenade_proximity_warning_inner_danger_color.b(), alpha_damage));
	draw_arc(prev_screen.x, prev_screen.y - size.y * 0.5f, 20, 0, 360 * percent, 2, g_cfg.esp.grenade_proximity_warning_progress_color);
	render::get().text(fonts[WEAPON_ICON_FONT], prev_screen.x, prev_screen.y - size.y * 0.5f, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, index_to_grenade_name_icon(m_index));

	if (g_cfg.esp.offscreen_proximity)
	{
		const int screen_center_x = g_ctx.globals.screen_width / 2, screen_center_y = g_ctx.globals.screen_height / 2;

		Vector screen_point, viewangles;
		m_debugoverlay()->ScreenPosition(vExplodeOrigin, screen_point);

		if (screen_point.x < 0 || screen_point.y < 0 || screen_point.x > g_ctx.globals.screen_width || screen_point.y > g_ctx.globals.screen_height)
		{
			m_engine()->GetViewAngles(viewangles);
			float desired_offset = viewangles.y - math::calculate_angle(g_ctx.globals.eye_pos, vExplodeOrigin).y - 90;

			const auto angle_yaw_rad1 = DEG2RAD(desired_offset);

			const auto new_point_x =
				screen_center_x + ((((g_ctx.globals.screen_width - (59 * 3)) * .5f) * (59 / 100.0f)) * cosf(angle_yaw_rad1)) + (int)(6.0f * (((float)59 - 4.f) / 16.0f));

			const auto new_point_y =
				screen_center_y + ((((g_ctx.globals.screen_height - (59 * 3)) * .5f) * (59 / 100.0f)) * sinf(angle_yaw_rad1));

			render::get().circle_filled(new_point_x, new_point_y - size.y * 0.5f, 60, 20, g_cfg.esp.grenade_proximity_warning_inner_color);
			render::get().circle_filled(new_point_x, new_point_y - size.y * 0.5f, 60, 20, Color(g_cfg.esp.grenade_proximity_warning_inner_danger_color.r(), g_cfg.esp.grenade_proximity_warning_inner_danger_color.g(), g_cfg.esp.grenade_proximity_warning_inner_danger_color.b(), alpha_damage));
			draw_arc(new_point_x, new_point_y - size.y * 0.5f, 20, 0, 360 * percent, 2, g_cfg.esp.grenade_proximity_warning_progress_color);
			render::get().text(fonts[WEAPON_ICON_FONT], new_point_x, new_point_y - size.y * 0.5f, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, index_to_grenade_name_icon(m_index));
		}
	}

	return true;
}

void grenadewarning::do_warning(projectile_t* entity)
{
	auto& predicted_nades = this->get_list();

	static auto last_server_tick = m_clientstate()->m_iClockDriftMgr.m_nServerTick;

	if (last_server_tick != m_clientstate()->m_iClockDriftMgr.m_nServerTick) {
		if (!predicted_nades.empty())
			predicted_nades.clear();

		last_server_tick = m_clientstate()->m_iClockDriftMgr.m_nServerTick;
	}

	if (entity->IsDormant() || !g_cfg.esp.grenade_proximity_warning)
		return;

	const auto client_class = entity->GetClientClass();
	if (!client_class
		|| client_class->m_ClassID != CMolotovProjectile && client_class->m_ClassID != CBaseCSGrenadeProjectile)
		return;

	if (client_class->m_ClassID == CBaseCSGrenadeProjectile) {
		const auto model = entity->GetModel();
		if (!model)
			return;

		const auto studio_model = m_modelinfo()->GetStudioModel(model);
		if (!studio_model
			|| std::string_view(studio_model->szName).find(crypt_str("fraggrenade")) == std::string::npos)
			return;
	}

	const auto handle = entity->GetRefEHandle().ToLong();

	if (entity->m_nExplodeEffectTickBegin() || !entity->m_hThrower().IsValid() || (entity->m_hThrower().Get()->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && entity->m_hThrower().Get() != g_ctx.local()))
	{
		predicted_nades.erase(handle);
		return;
	}

	if (predicted_nades.find(handle) == predicted_nades.end()) {
		predicted_nades.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(handle),
			std::forward_as_tuple(
				entity->m_hThrower().Get(),
				client_class->m_ClassID == CMolotovProjectile ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
				entity->m_vecOrigin(), entity->m_vecVelocity(),
				entity->m_flSpawnTime(), TIME_TO_TICKS(reinterpret_cast<player_t*>(entity)->m_flSimulationTime() - entity->m_flSpawnTime())
			)
		);
	}

	if (predicted_nades.at(handle).draw())
		return;

	predicted_nades.erase(handle);
}

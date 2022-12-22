// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "world_esp.h"
#include "GrenadeWarning.h"

// Used to sort Vectors in ccw order about a pivot.
static float ccw(const Vector& a, const Vector& b, const Vector& c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

struct ccwSorter {
	const Vector& pivot;

	ccwSorter(const Vector& inPivot) : pivot(inPivot) { }

	bool operator()(const Vector& a, const Vector& b) {
		return ccw(pivot, a, b) < 0;
	}
};

static bool is_left_of(const Vector& a, const Vector& b) {
	return (a.x < b.x || (a.x == b.x && a.y < b.y));
}

static std::vector<Vector> gift_wrapping(std::vector<Vector> v) {
	std::vector<Vector> hull;

	// There must be at least 3 points
	if (v.size() < 3)
		return hull;

	// Move the leftmost Vector to the beginning of our vector.
	// It will be the first Vector in our convext hull.
	std::swap(v[0], *min_element(v.begin(), v.end(), is_left_of));
	
	// Repeatedly find the first ccw Vector from our last hull Vector
	// and put it at the front of our array. 
	// Stop when we see our first Vector again.
	do {
		hull.push_back(v[0]);
		std::swap(v[0], *min_element(v.begin() + 1, v.end(), ccwSorter(v[0])));
	} while (v[0].x != hull[0].x && v[0].y != hull[0].y);

	return hull;
}

void worldesp::paint_traverse()
{
	skybox_changer();

	for (int i = 1; i <= m_entitylist()->GetHighestEntityIndex(); i++)  //-V807
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_player())
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		switch (client_class->m_ClassID)
		{
		case CEnvTonemapController:
			world_modulation(e);
			break;
		case CInferno:
			molotov_timer(e);
			break;
		case CSmokeGrenadeProjectile:
			smoke_timer(e);
			break;
		case CPlantedC4:
			bomb_timer(e);
			break;
		case CC4:
			if (g_cfg.esp.bomb_timer)
			{
				auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(e->m_hOwnerEntity());
				auto screen = ZERO;

				if (!owner->is_player())
				{
					if (math::world_to_screen(e->m_vecOrigin(), screen))
						render::get().text(fonts[ESP], screen.x, screen.y, Color(215, 20, 20), HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("BOMB"));
				}
			}

			break;
		default:
			grenadewarning::get().do_warning((projectile_t*)e);

			grenade_projectiles(e);
			if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor) //-V648
				dropped_weapons(e);

			break;
		}
	}
}

void worldesp::skybox_changer()
{
	static auto load_skybox = reinterpret_cast<void(__fastcall*)(const char*)>(g_ctx.addresses.load_skybox);
	static auto threed_skycvar = g_ctx.convars.r_3dsky;

	auto skybox_name = backup_skybox;
	auto threed_sky = backup_threedsky;

	switch (g_cfg.esp.skybox)
	{
	case 1:
		skybox_name = crypt_str("cs_tibet");
		break;
	case 2:
		skybox_name = crypt_str("cs_baggage_skybox_");
		break;
	case 3:
		skybox_name = crypt_str("italy");
		break;
	case 4:
		skybox_name = crypt_str("jungle");
		break;
	case 5:
		skybox_name = crypt_str("office");
		break;
	case 6:
		skybox_name = crypt_str("sky_cs15_daylight01_hdr");
		break;
	case 7:
		skybox_name = crypt_str("sky_cs15_daylight02_hdr");
		break;
	case 8:
		skybox_name = crypt_str("vertigoblue_hdr");
		break;
	case 9:
		skybox_name = crypt_str("vertigo");
		break;
	case 10:
		skybox_name = crypt_str("sky_day02_05_hdr");
		break;
	case 11:
		skybox_name = crypt_str("nukeblank");
		break;
	case 12:
		skybox_name = crypt_str("sky_venice");
		break;
	case 13:
		skybox_name = crypt_str("sky_cs15_daylight03_hdr");
		break;
	case 14:
		skybox_name = crypt_str("sky_cs15_daylight04_hdr");
		break;
	case 15:
		skybox_name = crypt_str("sky_csgo_cloudy01");
		break;
	case 16:
		skybox_name = crypt_str("sky_csgo_night02");
		break;
	case 17:
		skybox_name = crypt_str("sky_csgo_night02b");
		break;
	case 18:
		skybox_name = crypt_str("sky_csgo_night_flat");
		break;
	case 19:
		skybox_name = crypt_str("sky_dust");
		break;
	case 20:
		skybox_name = crypt_str("vietnam");
		break;
	case 21:
		skybox_name = g_cfg.esp.custom_skybox;
		threed_sky = 0;
		break;
	}

	static auto skybox_number = 0;
	static auto old_skybox_name = skybox_name;

	static auto color_r = (unsigned char)255;
	static auto color_g = (unsigned char)255;
	static auto color_b = (unsigned char)255;

	if (skybox_number != g_cfg.esp.skybox)
	{
		changed = true;
		skybox_number = g_cfg.esp.skybox;
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != g_cfg.esp.skybox_color[0])
	{
		changed = true;
		color_r = g_cfg.esp.skybox_color[0];
	}
	else if (color_g != g_cfg.esp.skybox_color[1])
	{
		changed = true;
		color_g = g_cfg.esp.skybox_color[1];
	}
	else if (color_b != g_cfg.esp.skybox_color[2])
	{
		changed = true;
		color_b = g_cfg.esp.skybox_color[2];
	}

	if (changed)
	{
		changed = false;

		load_skybox(skybox_name.c_str());
		threed_skycvar->SetValue(threed_sky);

		auto materialsystem = m_materialsystem();

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), crypt_str("SkyBox")))
				material->ColorModulate(g_cfg.esp.skybox_color[0] / 255.0f, g_cfg.esp.skybox_color[1] / 255.0f, g_cfg.esp.skybox_color[2] / 255.0f);
		}
	}
}

void worldesp::fog_changer()
{
	if (!g_cfg.esp.fog)
	{
		if (g_ctx.convars.fog_override->GetBool())
			g_ctx.convars.fog_override->SetValue(FALSE);

		return;
	}

	if (!g_ctx.convars.fog_override->GetBool())
		g_ctx.convars.fog_override->SetValue(TRUE);

	if (g_ctx.convars.fog_start->GetInt())
		g_ctx.convars.fog_start->SetValue(0);

	if (g_ctx.convars.fog_end->GetInt() != g_cfg.esp.fog_distance)
		g_ctx.convars.fog_end->SetValue(g_cfg.esp.fog_distance);

	if (g_ctx.convars.fog_maxdensity->GetFloat() != (float)g_cfg.esp.fog_density * 0.01f) //-V550
		g_ctx.convars.fog_maxdensity->SetValue((float)g_cfg.esp.fog_density * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, crypt_str("%i %i %i"), g_cfg.esp.fog_color.r(), g_cfg.esp.fog_color.g(), g_cfg.esp.fog_color.b());

	if (strcmp(g_ctx.convars.fog_color->GetString(), buffer_color)) //-V526
		g_ctx.convars.fog_color->SetValue(buffer_color);
}

void worldesp::world_modulation(entity_t* entity)
{
	if (!g_cfg.esp.world_modulation)
		return;

	entity->set_m_bUseCustomBloomScale(TRUE);
	entity->set_m_flCustomBloomScale(g_cfg.esp.bloom * 0.01f);

	entity->set_m_bUseCustomAutoExposureMin(TRUE);
	entity->set_m_flCustomAutoExposureMin(g_cfg.esp.exposure * 0.001f);

	entity->set_m_bUseCustomAutoExposureMax(TRUE);
	entity->set_m_flCustomAutoExposureMax(g_cfg.esp.exposure * 0.001f);
}

void worldesp::molotov_timer(entity_t* entity)
{
	const auto inferno = reinterpret_cast<inferno_t*>(entity);
	if (!inferno)
		return;

	if (g_cfg.esp.inferno_radius)
	{
		switch (g_cfg.esp.inferno_radius_type)
		{
		case 0:
		{
			std::vector<Vector> points;

			/* get each individual inferno position. */
			for (auto i = 0; i <= inferno->m_fireCount(); ++i)
			{
				if (!inferno->m_bFireIsBurning()[i])
					continue;

				points.emplace_back(inferno->m_vecOrigin() + Vector(inferno->m_fireXDelta()[i], inferno->m_fireYDelta()[i], inferno->m_fireZDelta()[i]));
			}

			/* add the inferno position with largest possible inferno width so it's showing accurate radius. */
			auto flame_circumference = [&] 
			{
				std::vector<Vector> new_points;

				for (auto i = 0; i < points.size(); ++i)
				{
					const auto& pos = points[i];

					for (auto j = 0; j <= 3; j++)
					{
						auto p = j * (360 / 4) * (M_PI / 200);
						new_points.emplace_back(pos + Vector(std::cos(p) * 60.f, std::sin(p) * 60.f, 0.f));
					}
				}

				return new_points;
			}();

			/* we only wanted to draw the points on the edge, use giftwrap algorithm. */
			std::vector<Vector> gift_wrapped = gift_wrapping(flame_circumference);

			/* transforms world position to screen position. */
			std::vector<Vertex_t> vertices;

			for (auto i = 0; i < gift_wrapped.size(); ++i)
			{
				const auto& pos = gift_wrapped[i];
				
				auto screen_pos = ZERO;
				if (!math::world_to_screen(pos, screen_pos))
					return;

				vertices.emplace_back(Vector2D(screen_pos.x, screen_pos.y));
			}

			/* draw our radius. */
			static int texture_id = m_surface()->CreateNewTextureID(true);

			m_surface()->DrawSetTexture(texture_id);
			m_surface()->DrawSetColor(Color(g_cfg.esp.inferno_radius_color.r(), g_cfg.esp.inferno_radius_color.g(), g_cfg.esp.inferno_radius_color.b(), g_cfg.esp.inferno_radius_color.a()));
			m_surface()->DrawTexturedPolygon(vertices.size(), vertices.data());
		}break;
		case 1:
		{
			render::get().Draw3DFilledCircle(inferno->m_vecOrigin(), 120.f, g_cfg.esp.inferno_radius_color, 1);
		}break;
		default:
			break;
		}
	}

	if (!g_cfg.esp.molotov_timer)
		return;

	auto screen_origin = ZERO;
	if (!math::world_to_screen(inferno->m_vecOrigin(), screen_origin))
		return;

	const auto factor = (inferno->get_spawn_time() + inferno_t::get_expiry_time() - m_globals()->m_curtime) / inferno_t::get_expiry_time();
	const auto size = Vector2D(40.0f, 5.0f);
	
	render::get().rect_filled(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, size.x, size.y, Color(37, 37, 37, g_cfg.esp.molotov_timer_color.a()));
	render::get().rect_filled(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, (size.x - size.y) * factor, size.y, g_cfg.esp.molotov_timer_color);
	render::get().rect(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, size.x, size.y, Color(37, 37, 37, g_cfg.esp.molotov_timer_color.a()));
	
	render::get().text(fonts[ESP], screen_origin.x, screen_origin.y - size.y * 0.5f - 5.0f, g_cfg.esp.molotov_timer_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("FIRE"));
}

void worldesp::smoke_timer(entity_t* entity)
{
	const auto smoke = reinterpret_cast<smoke_t*>(entity);
	if (!smoke)
		return;

	if (!smoke->m_bDidSmokeEffect())
		return;

	if (!smoke->m_nSmokeEffectTickBegin())
		return;

	if (g_cfg.esp.smoke_radius)
		render::get().Draw3DFilledCircle(smoke->m_vecOrigin(), 144.0f, g_cfg.esp.smoke_radius_color, 1);

	if (!g_cfg.esp.smoke_timer)
		return;

	auto screen_origin = ZERO;
	if (!math::world_to_screen(smoke->m_vecOrigin(), screen_origin))
		return;

	auto factor = (TICKS_TO_TIME(smoke->m_nSmokeEffectTickBegin()) + smoke_t::get_expiry_time() - m_globals()->m_curtime) / smoke_t::get_expiry_time();
	static auto size = Vector2D(40.0f, 5.0f);

	render::get().rect_filled(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, size.x, size.y, Color(37, 37, 37, g_cfg.esp.smoke_timer_color.a()));
	render::get().rect_filled(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, (size.x - size.y) * factor, size.y, g_cfg.esp.smoke_timer_color);
	render::get().rect(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5, size.x, size.y, Color(37, 37, 37, g_cfg.esp.smoke_timer_color.a()));

	render::get().text(fonts[ESP], screen_origin.x, screen_origin.y - size.y * 0.5f - 5.0f, g_cfg.esp.smoke_timer_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("SMOKE"));
}

void worldesp::grenade_projectiles(entity_t* entity)
{
	if (!g_cfg.esp.projectiles)
		return;

	auto client_class = entity->GetClientClass();

	if (!client_class)
		return;

	auto model = entity->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	player_t* owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(entity->m_hOwnerEntity());

	auto name = (std::string)studio_model->szName;

	if (name.find(crypt_str("thrown")) != std::string::npos ||
		client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile)
	{
		const auto& grenade_origin = entity->m_vecOrigin();
		auto grenade_position = ZERO;

		if (!math::world_to_screen(grenade_origin, grenade_position))
			return;

		std::string grenade_name, grenade_icon;

		if (name.find(crypt_str("flashbang")) != std::string::npos)
		{
			grenade_name = crypt_str("FLASHBANG");
			grenade_icon = crypt_str("i");
		}
		else if (name.find(crypt_str("smokegrenade")) != std::string::npos)
		{
			grenade_name = crypt_str("SMOKE");
			grenade_icon = crypt_str("k");
		}
		else if (name.find(crypt_str("incendiarygrenade")) != std::string::npos)
		{
			grenade_name = crypt_str("INCENDIARY");
			grenade_icon = crypt_str("n");
		}
		else if (name.find(crypt_str("molotov")) != std::string::npos)
		{
			grenade_name = crypt_str("MOLOTOV");
			grenade_icon = crypt_str("l");
		}
		else if (name.find(crypt_str("fraggrenade")) != std::string::npos)
		{
			grenade_name = crypt_str("HE GRENADE");
			grenade_icon = crypt_str("j");
		}
		else if (name.find(crypt_str("decoy")) != std::string::npos)
		{
			grenade_name = crypt_str("DECOY");
			grenade_icon = crypt_str("m");
		}
		else
			return;

		Box box;

		if (util::get_bbox(entity, box, false))
		{
			if (g_cfg.esp.grenade_esp[GRENADE_BOX])
			{
				auto outline_alpha = (int)(255 * 0.6f);

				Color outline_color
				{
					0,
					0,
					0,
					outline_alpha
				};

				render::get().rect(box.x, box.y, box.w, box.h, g_cfg.esp.grenade_box_color);
				render::get().rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, outline_color);
				render::get().rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, outline_color);

				if (g_cfg.esp.grenade_esp[GRENADE_ICON])
					render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y - 21, g_cfg.esp.projectiles_color, HFONT_CENTERED_X, grenade_icon.c_str());

				if (g_cfg.esp.grenade_esp[GRENADE_TEXT])
					render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, g_cfg.esp.projectiles_color, HFONT_CENTERED_X, grenade_name.c_str());
			}
			else
			{
				if (g_cfg.esp.grenade_esp[GRENADE_ICON] && g_cfg.esp.grenade_esp[GRENADE_TEXT])
				{
					render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y + box.h / 2 - 10, g_cfg.esp.projectiles_color, HFONT_CENTERED_X, grenade_icon.c_str());
					render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 7, g_cfg.esp.projectiles_color, HFONT_CENTERED_X, grenade_name.c_str());
				}
				else
				{
					if (g_cfg.esp.grenade_esp[GRENADE_ICON])
						render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y + box.h / 2, g_cfg.esp.projectiles_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, grenade_icon.c_str());

					if (g_cfg.esp.grenade_esp[GRENADE_TEXT])
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2, g_cfg.esp.projectiles_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, grenade_name.c_str());
				}
			}
		}
	}
	else
	{
		auto model = entity->GetModel();

		if (!model)
			return;

		auto studio_model = m_modelinfo()->GetStudioModel(model);

		if (!studio_model)
			return;

		auto name = (std::string)studio_model->szName;

		if (name.find(crypt_str("dropped")) != std::string::npos)
		{
			auto weapon = (weapon_t*)entity; //-V1027
			Box box;

			if (util::get_bbox(weapon, box, false))
			{
				auto offset = 0;

				if (g_cfg.esp.weapon[WEAPON_BOX])
				{
					auto outline_alpha = (int)(255 * 0.6f);

					Color outline_color
					{
						0,
						0,
						0,
						outline_alpha
					};

					render::get().rect(box.x, box.y, box.w, box.h, g_cfg.esp.box_color);
					render::get().rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, outline_color);
					render::get().rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, outline_color);

					if (g_cfg.esp.weapon[WEAPON_ICON])
					{
						render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y - 14, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());
						offset = 14;
					}

					if (g_cfg.esp.weapon[WEAPON_TEXT])
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

					if (g_cfg.esp.weapon[WEAPON_DISTANCE])
					{
						auto distance = g_ctx.local()->m_vecOrigin().DistTo(weapon->m_vecOrigin()) / 12.0f;
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y - 13 - offset, g_cfg.esp.weapon_color, HFONT_CENTERED_X, crypt_str("%i FT"), (int)distance);
					}
				}
				else
				{
					if (g_cfg.esp.weapon[WEAPON_ICON])
						render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y + box.h / 2 - 7, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());

					if (g_cfg.esp.weapon[WEAPON_TEXT])
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 6, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

					if (g_cfg.esp.weapon[WEAPON_DISTANCE])
					{
						auto distance = g_ctx.local()->m_vecOrigin().DistTo(weapon->m_vecOrigin()) / 12.0f;

						if (g_cfg.esp.weapon[WEAPON_ICON] && g_cfg.esp.weapon[WEAPON_TEXT])
							offset = 21;
						else if (g_cfg.esp.weapon[WEAPON_ICON])
							offset = 21;
						else if (g_cfg.esp.weapon[WEAPON_TEXT])
							offset = 8;

						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 - offset, g_cfg.esp.weapon_color, HFONT_CENTERED_X, crypt_str("%i FT"), (int)distance);
					}
				}
			}
		}
	}
}

void worldesp::bomb_timer(entity_t* entity)
{
	if (!g_cfg.esp.bomb_timer)
		return;

	if (!g_ctx.globals.bomb_timer_enable)
		return;

	auto bomb = (CCSBomb*)entity;

	auto c4timer = g_ctx.convars.mp_c4timer->GetFloat();
	auto bomb_timer = bomb->m_flC4Blow() - m_globals()->m_curtime;

	if (bomb_timer < 0.0f)
		return;


	auto factor = bomb_timer / c4timer * g_ctx.globals.screen_height;

	auto red_factor = (int)(255.0f - bomb_timer / c4timer * 255.0f);
	auto green_factor = (int)(bomb_timer / c4timer * 255.0f);

	render::get().rect_filled(0, g_ctx.globals.screen_height - factor, 26, factor, Color(red_factor, green_factor, 0, 100));

	auto text_position = g_ctx.globals.screen_height - factor + 11;

	if (text_position > g_ctx.globals.screen_height - 9)
		text_position = g_ctx.globals.screen_height - 9;

	render::get().text(fonts[ESP], 13, text_position, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("%0.1f"), bomb_timer);

	Vector screen;

	if (math::world_to_screen(entity->m_vecOrigin(), screen))
		render::get().text(fonts[ESP], screen.x, screen.y, Color(red_factor, green_factor, 0), HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("BOMB"));

}

void worldesp::dropped_weapons(entity_t* entity)
{
	auto weapon = (weapon_t*)entity; //-V1027
	auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(weapon->m_hOwnerEntity());

	if (owner->is_player())
		return;

	Box box;

	if (util::get_bbox(weapon, box, false))
	{
		auto offset = 0;

		if (g_cfg.esp.weapon[WEAPON_BOX])
		{
			auto outline_alpha = (int)(255 * 0.6f);

			Color outline_color
			{
				0,
				0,
				0,
				outline_alpha
			};

			render::get().rect(box.x, box.y, box.w, box.h, g_cfg.esp.box_color);
			render::get().rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, outline_color);
			render::get().rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, outline_color);

			if (g_cfg.esp.weapon[WEAPON_ICON])
			{
				render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y - 14, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());
				offset = 14;
			}

			if (g_cfg.esp.weapon[WEAPON_TEXT])
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

			if (g_cfg.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				render::get().rect_filled(box.x - 1, box.y + box.h + 14, box.w + 2, 4, inner_back_color);
				render::get().rect_filled(box.x, box.y + box.h + 15, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, g_cfg.esp.weapon_ammo_color);
			}

			if (g_cfg.esp.weapon[WEAPON_DISTANCE])
			{
				auto distance = g_ctx.local()->m_vecOrigin().DistTo(weapon->m_vecOrigin()) / 12.0f;
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y - 13 - offset, g_cfg.esp.weapon_color, HFONT_CENTERED_X, crypt_str("%i FT"), (int)distance);
			}
		}
		else
		{
			if (g_cfg.esp.weapon[WEAPON_ICON])
				render::get().text(fonts[WEAPON_ICON_FONT], box.x + box.w / 2, box.y + box.h / 2 - 7, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());

			if (g_cfg.esp.weapon[WEAPON_TEXT])
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 6, g_cfg.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

			if (g_cfg.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				static auto pos = 0;

				if (g_cfg.esp.weapon[WEAPON_ICON] && g_cfg.esp.weapon[WEAPON_TEXT])
					pos = 19;
				else if (g_cfg.esp.weapon[WEAPON_ICON])
					pos = 8;
				else if (g_cfg.esp.weapon[WEAPON_TEXT])
					pos = 19;

				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				render::get().rect_filled(box.x - 1, box.y + box.h / 2 + pos - 1, box.w + 2, 4, inner_back_color);
				render::get().rect_filled(box.x, box.y + box.h / 2 + pos, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, g_cfg.esp.weapon_ammo_color);
			}

			if (g_cfg.esp.weapon[WEAPON_DISTANCE])
			{
				auto distance = g_ctx.local()->m_vecOrigin().DistTo(weapon->m_vecOrigin()) / 12.0f;

				if (g_cfg.esp.weapon[WEAPON_ICON] && g_cfg.esp.weapon[WEAPON_TEXT])
					offset = 21;
				else if (g_cfg.esp.weapon[WEAPON_ICON])
					offset = 21;
				else if (g_cfg.esp.weapon[WEAPON_TEXT])
					offset = 8;

				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 - offset, g_cfg.esp.weapon_color, HFONT_CENTERED_X, crypt_str("%i FT"), (int)distance);
			}
		}
	}
}
// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <ShlObj_core.h>
#include <unordered_map>

#include "menu.h"
#include "dpi_scale.h"
#include "..\misc\misc.h"
#include "..\..\byte\constchars.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\sdk\steam\steam_api.h"
#include "..\..\ImGui\misc\background.h"

void c_menu::setup_style()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark();

	style.ScrollbarSize = 3.f;
	style.ScrollbarRounding = 12.f;

	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); i++) {
		if (!this->all_skins[i])
			this->all_skins[i] = util::get_skin_preview(this->get_weapon_name_from_id(i, (i == 0 || i == 1) ? g_cfg.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_cfg.skins.skinChanger.at(i).skin_name, g_ctx.globals.draw_device);
	}

	this->once = true;
}

void c_menu::draw_background()
{
	/* DATA. */
	const int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize;

	/* BEGIN BACKGROUND. */
	ImGui::Begin(crypt_str("##menu_background"), 0, flags);
	{
		/* INITIALIZE. */
		ImGui::SetWindowSize(ImVec2(g_ctx.globals.screen_width, g_ctx.globals.screen_height));
		ImGui::SetWindowPos(ImVec2(0, 0));

		const auto draw_list = ImGui::GetBackgroundDrawList();
		const ImVec2 pos(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

		/* DRAW OUR BACKGROUND. */
		draw_list->AddRectFilled(pos, ImVec2(pos.x + g_ctx.globals.screen_width, pos.y + g_ctx.globals.screen_height), ImColor(ImVec4(0.f, 0.f, 0.f, this->public_alpha - 0.5f)));

		ImGui::PushFont(g_ctx.fonts.large_generic_font);
		draw_list->AddText(ImVec2(5, 5), ImColor(ImVec4(1.f, 1.f, 1.f, this->public_alpha)), "naval / private release / version: none");
		ImGui::PopFont();
	}
	ImGui::End();
}

void c_menu::draw(bool is_open)
{
	// note - shyx; since this menu code is so goofy. to positioning (nice word) a child with the different position but same height must use 
	// SetCursorPos(first child size + first child pos - how many spacing you wanted, first child height) instead of SameLine. 

	// alpha stuff.
	static float m_alpha = 0.0002f;
	m_alpha = math::clamp(m_alpha + (3.f * ImGui::GetIO().DeltaTime * (is_open ? 1.f : -1.f)), 0.0001f, 1.f);

	// alpha used for other thing that is not inside this function (draw).
	public_alpha = m_alpha;

	// setup our menu style, just do it once. 
	if (!this->once)
		this->setup_style();

	// don't run the code if we reach transparency.
	if (m_alpha <= 0.0001f)
		return;

	// this is for the menu background, just a nice grey covering the entire screen.
	this->draw_background();

	// menu data used later.
	this->menu_x = 50;
	this->menu_y = 430;

	// who the fuck want to scroll the main menu?.
	const int flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	
	// the menu code is in here.
	ImGui::Begin(crypt_str("##start_menu"), 0, flags);
	{
		// set our menu size with the data we have.
		ImGui::SetWindowSize(ImVec2(this->menu_x, this->menu_y));
		
		const auto drawlist = ImGui::GetWindowDrawList();
		const auto position = ImGui::GetWindowPos();

		// draw some little nice decorations.
		this->draw_decorations(drawlist, position);

		// draw another tab or whatever you call is, for the function like ragebot, legitbot, and the other stuff.
		ImGui::SetNextWindowSize(ImVec2(this->menu_x + 435, this->menu_y));
		ImGui::SetNextWindowPos(ImVec2(position.x + this->menu_x - 10, position.y));

		ImGui::Begin(crypt_str("##functions_menu"), 0, flags | ImGuiWindowFlags_NoMove);
		{
			// our another tab data.
			const auto function_drawlist = ImGui::GetWindowDrawList();
			const auto function_position = ImGui::GetWindowPos();
			const auto x = this->menu_x + 435, y = this->menu_y;

			// draw a simple background. 
			function_drawlist->AddRectFilled(function_position, ImVec2(function_position.x + x, function_position.y + y), ImColor(ImVec4(0.12f, 0.12f, 0.13f, public_alpha - 0.22f)), 13.f, ImDrawCornerFlags_Right);

			// draw some little nice decorations.
			this->draw_functions_decorations(function_drawlist, function_position);

			// this is where everything started.
			this->draw_functions();
		}
		// end our another tab.
		ImGui::End();
	}
	// end our main tab.
	ImGui::End();
}

void c_menu::draw_decorations(ImDrawList* draw_list, ImVec2 pos)
{
	// our data for the decorations.
	const int x = this->menu_x, y = this->menu_y;

	// draw our simple background.
	draw_list->AddRectFilled(pos, ImVec2(pos.x + x, pos.y + y), ImColor(ImVec4(g_cfg.menu.menu_theme.r() / 255.f, g_cfg.menu.menu_theme.g() / 255.f, g_cfg.menu.menu_theme.b() / 255.f, this->public_alpha)), 13.f, ImDrawCornerFlags_Left);

	// shadow. (eh this is so ghetto).
	draw_list->AddRectFilledMultiColor(ImVec2(pos.x + x - 11, pos.y), ImVec2(pos.x + x, pos.y + y), ImColor(ImVec4(0.f, 0.f, 0.f, 0.f)), ImColor(ImVec4(0.f, 0.f, 0.f, public_alpha - 0.3)), ImColor(ImVec4(0.f, 0.f, 0.f, public_alpha - 0.3)), ImColor(ImVec4(0.f, 0.f, 0.f, 0.f)));

	// do our tab thing.
	ImGui::SetCursorPos(ImVec2(9, 5));
	ImGui::BeginGroup();

	if (ImGui::Tab(crypt_str("##Ragebot"), crypt_str("A"), this->tab_choosen == 0))
		this->tab_choosen = 0;
	if (ImGui::Tab(crypt_str("##Legitbot"), crypt_str("B"), this->tab_choosen == 1))
		this->tab_choosen = 1;
	if (ImGui::Tab(crypt_str("##Visuals"), crypt_str("C"), this->tab_choosen == 2, true)) // visual tab got a special treatment.
		this->tab_choosen = 2;
	if (ImGui::Tab(crypt_str("##Skins"), crypt_str("G"), this->tab_choosen == 3))
		this->tab_choosen = 3;
	if (ImGui::Tab(crypt_str("##Misc"), crypt_str("D"), this->tab_choosen == 4))
		this->tab_choosen = 4;
	if (ImGui::Tab(crypt_str("##Configs"), crypt_str("E"), this->tab_choosen == 5))
		this->tab_choosen = 5;

	ImGui::EndGroup();
}

void c_menu::draw_functions()
{
	switch (this->tab_choosen)
	{
	case 0:
		if (this->rage_sub == 0)
			this->draw_rage_main_tab();
		else
			this->draw_rage_antiaim_tab();

		break;
	case 1:
		this->draw_legit_main_tab();
		break;
	case 2:

		if (this->visual_subsub == 0)
		{
			if (this->visual_sub != 3)
				this->draw_visual_player_esp_tab();
			else
				this->draw_visual_other_esp_tab();
		}
		else
		{
			if (this->visual_sub != 3)
				this->draw_visual_player_chams_tab();
			else
				this->draw_visual_other_chams_tab();
		}

		break;
	case 3:
		this->draw_skinchanger_tab();
		break;
	case 4:
		this->draw_miscellaneous_tab();
		break;
	case 5:
		this->draw_configuration_tab();
		break;
	default:
		break;
	}
}

void c_menu::draw_functions_decorations(ImDrawList* draw_list, ImVec2 pos)
{
	// our data for the decorations.
	const int x = this->menu_x + 435, y = this->menu_y;

	switch (this->tab_choosen)
	{
	case 0: // rage tab.
		// draw our simple topbar.
		draw_list->AddRectFilled(pos, ImVec2(pos.x + x, pos.y + 30), ImColor(ImVec4(g_cfg.menu.menu_theme.r() / 255.f, g_cfg.menu.menu_theme.g() / 255.f, g_cfg.menu.menu_theme.b() / 255.f, public_alpha)), 13.f, ImDrawCornerFlags_TopRight);

		ImGui::SetCursorPos(ImVec2(23, -2));
		ImGui::BeginGroup();

		if (this->rage_sub == 0)
		{
			// HEAVY PISTOL (REVOLVER & DEAGLE).
			if (ImGui::SubSubTab(crypt_str("J"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 0))
				hooks::rage_weapon = 0;

			// PISTOL (DUAL ELITE, USP, ETC).
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("D"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 1))
				hooks::rage_weapon = 1;

			// SMG (P90, MP7, MAC10).
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("P"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 2))
				hooks::rage_weapon = 2;

			// RIFLES (M4A1, AK47).
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("W"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 3))
				hooks::rage_weapon = 3;

			// AUTOSNIPER (SCAR20, G3SG1).
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("Y"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 4))
				hooks::rage_weapon = 4;

			// SCOUT (SSG08).
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("a"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 5))
				hooks::rage_weapon = 5;

			// AWP (awp).
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("Z"), g_ctx.fonts.weapon_icon, hooks::rage_weapon == 6))
				hooks::rage_weapon = 6;
		}
		else
		{
			// Standing.
			if (ImGui::SubSubTab(crypt_str("Standing"), g_ctx.fonts.generic_font, this->antiaim_type == 0))
				this->antiaim_type = 0;

			// Slow walk.
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("Slow walk"), g_ctx.fonts.generic_font, this->antiaim_type == 1))
				this->antiaim_type = 1;

			// Moving.
			ImGui::SameLine(0, 30);
			if (ImGui::SubSubTab(crypt_str("Moving"), g_ctx.fonts.generic_font, this->antiaim_type == 2))
				this->antiaim_type = 2;

			// In air.
			ImGui::SameLine(0, 28);
			if (ImGui::SubSubTab(crypt_str("In air"), g_ctx.fonts.generic_font, this->antiaim_type == 3))
				this->antiaim_type = 3;
		}
		ImGui::EndGroup();

		// draw our simple bottom bar.
		draw_list->AddRectFilled(ImVec2(pos.x, pos.y + y - 30), ImVec2(pos.x + x, pos.y + y - 30 + 30), ImColor(ImVec4(g_cfg.menu.menu_theme.r() / 255.f, g_cfg.menu.menu_theme.g() / 255.f, g_cfg.menu.menu_theme.b() / 255.f, public_alpha)), 13.f, ImDrawCornerFlags_BotRight);

		ImGui::SetCursorPos(ImVec2(23, y - 32));
		ImGui::BeginGroup();

		// Main.
		if (ImGui::SubSubTab(crypt_str("Main"), g_ctx.fonts.generic_font, this->rage_sub == 0))
			this->rage_sub = 0;

		// Anti-aim.
		ImGui::SameLine(0, 30);
		if (ImGui::SubSubTab(crypt_str("Anti-aim"), g_ctx.fonts.generic_font, this->rage_sub == 1))
			this->rage_sub = 1;

		ImGui::EndGroup();
		break;
	case 1: // legit tab.
		// draw our simple topbar.
		draw_list->AddRectFilled(pos, ImVec2(pos.x + x, pos.y + 30), ImColor(ImVec4(g_cfg.menu.menu_theme.r() / 255.f, g_cfg.menu.menu_theme.g() / 255.f, g_cfg.menu.menu_theme.b() / 255.f, public_alpha)), 13.f, ImDrawCornerFlags_TopRight);

		ImGui::SetCursorPos(ImVec2(23, -2));
		ImGui::BeginGroup();

		if (ImGui::SubSubTab(crypt_str("A"), g_ctx.fonts.weapon_icon, hooks::legit_weapon == 0)) // DEAGLE.
			hooks::legit_weapon = 0;

		ImGui::SameLine(0, 30);

		if (ImGui::SubSubTab(crypt_str("D"), g_ctx.fonts.weapon_icon, hooks::legit_weapon == 1)) // PISTOLS.
			hooks::legit_weapon = 1;

		ImGui::SameLine(0, 30);

		if (ImGui::SubSubTab(crypt_str("W"), g_ctx.fonts.weapon_icon, hooks::legit_weapon == 2)) // RIFLES.
			hooks::legit_weapon = 2;

		ImGui::SameLine(0, 30);

		if (ImGui::SubSubTab(crypt_str("P"), g_ctx.fonts.weapon_icon, hooks::legit_weapon == 3)) // SMG.
			hooks::legit_weapon = 3;

		ImGui::SameLine(0, 30);

		if (ImGui::SubSubTab(crypt_str("Z"), g_ctx.fonts.weapon_icon, hooks::legit_weapon == 4)) // SNIPERS.
			hooks::legit_weapon = 4;

		ImGui::SameLine(0, 30);

		if (ImGui::SubSubTab(crypt_str("f"), g_ctx.fonts.weapon_icon, hooks::legit_weapon == 5)) // HEAVY.
			hooks::legit_weapon = 5;

		ImGui::EndGroup();
		break;
	case 2: // visual tab.
		// draw our simple topbar.
		draw_list->AddRectFilled(pos, ImVec2(pos.x + x, pos.y + 30), ImColor(ImVec4(g_cfg.menu.menu_theme.r() / 255.f, g_cfg.menu.menu_theme.g() / 255.f, g_cfg.menu.menu_theme.b() / 255.f, public_alpha)), 13.f, ImDrawCornerFlags_TopRight);

		ImGui::SetCursorPos(ImVec2(23, -2));
		ImGui::BeginGroup();

		// ESP.
		if (ImGui::SubSubTab(crypt_str("ESP"), g_ctx.fonts.generic_font, this->visual_subsub == 0))
			this->visual_subsub = 0;

		// Chams.
		ImGui::SameLine(0, 30);
		if (ImGui::SubSubTab(crypt_str("Chams"), g_ctx.fonts.generic_font, this->visual_subsub == 1))
			this->visual_subsub = 1;

		ImGui::EndGroup();

		// draw our simple bottom bar.
		draw_list->AddRectFilled(ImVec2(pos.x, pos.y + y - 30), ImVec2(pos.x + x, pos.y + y - 30 + 30), ImColor(ImVec4(g_cfg.menu.menu_theme.r() / 255.f, g_cfg.menu.menu_theme.g() / 255.f, g_cfg.menu.menu_theme.b() / 255.f, public_alpha)), 13.f, ImDrawCornerFlags_BotRight);

		ImGui::SetCursorPos(ImVec2(23, y - 32));
		ImGui::BeginGroup();

		// Player.
		if (ImGui::SubSubTab(crypt_str("Enemy"), g_ctx.fonts.generic_font, this->visual_sub == ENEMY))
			this->visual_sub = ENEMY;

		// Teammate.
		ImGui::SameLine(0, 30);
		if (ImGui::SubSubTab(crypt_str("Teammate"), g_ctx.fonts.generic_font, this->visual_sub == TEAM))
			this->visual_sub = TEAM;

		// Local.
		ImGui::SameLine(0, 30);
		if (ImGui::SubSubTab(crypt_str("Local"), g_ctx.fonts.generic_font, this->visual_sub == LOCAL))
			this->visual_sub = LOCAL;

		// Other.
		ImGui::SameLine(0, 30);
		if (ImGui::SubSubTab(crypt_str("Other"), g_ctx.fonts.generic_font, this->visual_sub == 3))
			this->visual_sub = 3;

		ImGui::EndGroup();
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	default:
		break;
	}
}

void c_menu::draw_rage_main_tab()
{
	/* LEFT. */

	/* General. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild(std::string(this->get_rage_group() + crypt_str("general")).c_str(), ImVec2(230, 163));

	ImGui::Checkbox(crypt_str("Master switch"), &g_cfg.ragebot.enable);
	
	if (g_cfg.ragebot.enable)
		g_cfg.legitbot.enabled = false;
	
	ImGui::Checkbox(crypt_str("Silent aim"), &g_cfg.ragebot.silent_aim);
	ImGui::Checkbox(crypt_str("Automatic penetration"), &g_cfg.ragebot.autowall);
	ImGui::Checkbox(crypt_str("Automatic fire"), &g_cfg.ragebot.autoshoot);
	ImGui::Checkbox(crypt_str("Automatic scope"), &g_cfg.ragebot.autoscope);
	ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.ragebot.field_of_view, 1, 180);
	ImGui::Combo(crypt_str("Target sorting"), &g_cfg.ragebot.weapon[hooks::rage_weapon].selection_type, selection, IM_ARRAYSIZE(selection));
	ImGui::Keybind(crypt_str("Override resolver"), &g_cfg.keybinds.key[RESOLVER_OVERRIDE_KEYBIND], crypt_str("##KEYBIND_RESOLVER_OVERRIDE"));

	ImGui::EndChild();

	/* Hitboxes. */
	ImGui::SetCursorPos(ImVec2(10, 203));
	ImGui::BeginChild(std::string(this->get_rage_group() + crypt_str("hitboxes")).c_str(), ImVec2(230, 190));

	ImGui::MultiCombo(crypt_str("Hitscan"), hitboxes, g_cfg.ragebot.weapon[hooks::rage_weapon].hitboxes, IM_ARRAYSIZE(hitboxes));
	ImGui::MultiCombo(crypt_str("Multipoint"), multipoint_hitboxes, g_cfg.ragebot.weapon[hooks::rage_weapon].multipoint_hitboxes, IM_ARRAYSIZE(multipoint_hitboxes));
	ImGui::Combo(crypt_str("Priority"), &g_cfg.ragebot.weapon[hooks::rage_weapon].preferred_hitbox, prioritized_hitbox, IM_ARRAYSIZE(prioritized_hitbox));
	ImGui::MultiCombo(crypt_str("Prefer bodyaim"), prefer_bodyaim_options, g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim, IM_ARRAYSIZE(prefer_bodyaim_options));
	ImGui::MultiCombo(crypt_str("Always bodyaim"), force_bodyaim_options, g_cfg.ragebot.weapon[hooks::rage_weapon].always_body_aim, IM_ARRAYSIZE(force_bodyaim_options));
	ImGui::Keybind(crypt_str("Force bodyaim"), &g_cfg.keybinds.key[BODY_AIM_KEYBIND], crypt_str("##KEYBIND_FORCE_BODY_AIM"));
	
	if (g_cfg.ragebot.weapon[hooks::rage_weapon].always_body_aim.at(ALWAYS_BAIM_HEATH_BELOW))
		ImGui::SliderInt(crypt_str("Bodyaim if HP under"), &g_cfg.ragebot.weapon[hooks::rage_weapon].always_baim_if_hp_below, 1, 100);
	
	ImGui::Checkbox(crypt_str("Static multipoint scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale);
	if (g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale)
	{
		ImGui::SliderFloat(crypt_str("Head multipoint scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale, 0.0f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("None"));
		ImGui::SliderFloat(crypt_str("Body multipoint scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale, 0.0f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("None"));
	}
	
	ImGui::Checkbox(crypt_str("Ignore limbs"), &g_cfg.ragebot.weapon[hooks::rage_weapon].ignore_limbs);
	
	ImGui::Checkbox(crypt_str("Enable bodyaim after X miss"), &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses);
	if (g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses)
		ImGui::SliderInt(crypt_str("Bodyaim after X miss"), &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses_amount, 1, 12);

	ImGui::EndChild();

	/* RIGHT. */

	/* Accuracy. */
	ImGui::SetCursorPos(ImVec2(245, 35));
	ImGui::BeginChild(std::string(this->get_rage_group() + crypt_str("accuracy")).c_str(), ImVec2(230, 201));

	ImGui::SliderInt(crypt_str("Hit chance"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 1, 100);
	
	if (g_cfg.ragebot.autowall)
		ImGui::SliderInt(crypt_str("Minimum damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_damage, 1, 120, true);
	
	ImGui::SliderInt(crypt_str("Visible minimum damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_visible_damage, 1, 120, true);
	
	ImGui::Keybind(crypt_str("Damage override"), &g_cfg.keybinds.key[DAMAGE_OVERRIDE_KEYBIND], crypt_str("##KEYBIND_DAMAGE_OVERRIDE"));
	if (g_cfg.keybinds.key[DAMAGE_OVERRIDE_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[DAMAGE_OVERRIDE_KEYBIND].key < KEY_MAX)
		ImGui::SliderInt(crypt_str("Override minimum damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_override_damage, 1, 120);
	
	ImGui::Checkbox(crypt_str("Automatic stop"), &g_cfg.ragebot.weapon[hooks::rage_weapon].autostop);
	if (g_cfg.ragebot.weapon[hooks::rage_weapon].autostop)
		ImGui::MultiCombo(crypt_str("Automatic stop conditions"), autostop_modifiers, g_cfg.ragebot.weapon[hooks::rage_weapon].autostop_modifiers, IM_ARRAYSIZE(autostop_modifiers));
	
	ImGui::Checkbox(crypt_str("Prefer safepoints"), &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_safe_points);
	ImGui::Keybind(crypt_str("Force safepoints"), &g_cfg.keybinds.key[SAFE_POINT_KEYBIND], crypt_str("##KEYBIND_FORCE_SAFE_POINTS"));
	
	ImGui::EndChild();

	/* Exploits. */
	ImGui::SetCursorPos(ImVec2(245, 241));
	ImGui::BeginChild(std::string(this->get_rage_group() + crypt_str("exploits")).c_str(), ImVec2(230, 152));

	ImGui::Checkbox(crypt_str("Double tap"), &g_cfg.ragebot.double_tap);
	
	ImGui::Keybind(crypt_str("Double tap key"), &g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND], crypt_str("##KEYBIND_DOUBLETAP"));
	if (g_cfg.ragebot.double_tap)
		ImGui::Checkbox(crypt_str("Slow teleport"), &g_cfg.ragebot.slow_teleport);
	
	ImGui::Checkbox(crypt_str("Hide shots"), &g_cfg.antiaim.hide_shots);
	ImGui::Keybind(crypt_str("Hide shots key"), &g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND], crypt_str("##KEYBIND_HIDESHOTS"));

	ImGui::EndChild();
}

void c_menu::draw_rage_antiaim_tab()
{
	/* LEFT. */

	/* General. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild(std::string(this->get_antiaim_group() + crypt_str("anti-aim")).c_str(), ImVec2(230, 165));

	ImGui::Checkbox(crypt_str("Master switch"), &g_cfg.antiaim.enable);
	ImGui::Combo(crypt_str("Pitch"), &g_cfg.antiaim.type[this->antiaim_type].pitch, pitch, IM_ARRAYSIZE(pitch));
	
	ImGui::Combo(crypt_str("Real yaw"), &g_cfg.antiaim.type[this->antiaim_type].yaw, yaw, IM_ARRAYSIZE(yaw));
	if (g_cfg.antiaim.type[this->antiaim_type].yaw)
	{
		ImGui::SliderInt(g_cfg.antiaim.type[this->antiaim_type].yaw == 1 ? crypt_str("Jitter wdith") : crypt_str("Spin wdith"), &g_cfg.antiaim.type[this->antiaim_type].range, 1, 180);
		if (g_cfg.antiaim.type[this->antiaim_type].yaw == 2)
			ImGui::SliderInt(crypt_str("Spin speed"), &g_cfg.antiaim.type[this->antiaim_type].speed, 1, 15);
	}

	ImGui::Combo(crypt_str("Base angle"), &g_cfg.antiaim.type[this->antiaim_type].base_angle, baseangle, IM_ARRAYSIZE(baseangle));
	
	ImGui::Keybind(crypt_str("Manual back"), &g_cfg.keybinds.key[MANUAL_BACK_KEYBIND], crypt_str("##KEYBIND_INVERT_BACK"));
	ImGui::Keybind(crypt_str("Manual left"), &g_cfg.keybinds.key[MANUAL_LEFT_KEYBIND], crypt_str("##KEYBIND_INVERT_LEFT"));
	ImGui::Keybind(crypt_str("Manual right"), &g_cfg.keybinds.key[MANUAL_RIGHT_KEYBIND], crypt_str("##KEYBIND_INVERT_RIGHT"));
	if (g_cfg.keybinds.key[MANUAL_BACK_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[MANUAL_BACK_KEYBIND].key < KEY_MAX || g_cfg.keybinds.key[MANUAL_LEFT_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[MANUAL_LEFT_KEYBIND].key < KEY_MAX || g_cfg.keybinds.key[MANUAL_RIGHT_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[MANUAL_RIGHT_KEYBIND].key < KEY_MAX)
	{
		ImGui::Checkbox(crypt_str("Manual indicator"), &g_cfg.antiaim.flip_indicator);
		ImGui::ColorEdit(crypt_str("Indicator colour"), &g_cfg.antiaim.flip_indicator_color, ImGuiColorEditFlags_NoAlpha);
	}

	ImGui::EndChild();

	/* Fakelag. */
	ImGui::SetCursorPos(ImVec2(10, 205));
	ImGui::BeginChild(crypt_str("Fake lag"), ImVec2(230, 188));

	ImGui::Checkbox(crypt_str("Enable"), &g_cfg.antiaim.fakelag);
	if (g_cfg.antiaim.fakelag)
	{
		ImGui::Combo(crypt_str("Fake lag mode"), &g_cfg.antiaim.fakelag_type, fakelags, IM_ARRAYSIZE(fakelags));
		ImGui::SliderInt(crypt_str("Limit"), &g_cfg.antiaim.fakelag_amount, 1, 14);
		ImGui::MultiCombo(crypt_str("Fake lag condition"), lagstrigger, g_cfg.antiaim.fakelag_enablers, IM_ARRAYSIZE(lagstrigger));

		auto enabled_fakelag_triggers = false;

		for (auto i = 0; i < IM_ARRAYSIZE(lagstrigger); i++)
			if (g_cfg.antiaim.fakelag_enablers[i])
				enabled_fakelag_triggers = true;

		if (enabled_fakelag_triggers)
			ImGui::SliderInt(crypt_str("Condition limit"), &g_cfg.antiaim.triggers_fakelag_amount, 1, 14);
	}
	
	ImGui::EndChild();

	/* RIGHT. */

	/* Details. */
	ImGui::SetCursorPos(ImVec2(245, 35));
	ImGui::BeginChild(std::string(this->get_antiaim_group() + crypt_str("details")).c_str(), ImVec2(230, 358));

	ImGui::Combo(crypt_str("Desync"), &g_cfg.antiaim.type[this->antiaim_type].desync, desync, IM_ARRAYSIZE(desync));
	if (g_cfg.antiaim.type[this->antiaim_type].desync)
	{
		if (this->antiaim_type == ANTIAIM_STAND)
			ImGui::Combo(crypt_str("Lowerbody yaw type"), &g_cfg.antiaim.lby_type, lby_type, IM_ARRAYSIZE(lby_type));
		
		if (this->antiaim_type != ANTIAIM_STAND || !g_cfg.antiaim.lby_type)
		{
			ImGui::SliderInt(crypt_str("Desync width"), &g_cfg.antiaim.type[this->antiaim_type].desync_range, 1, 60);
			ImGui::SliderInt(crypt_str("Yaw lean"), &g_cfg.antiaim.type[this->antiaim_type].body_lean, 0, 100);
		}
		
		if (g_cfg.antiaim.type[this->antiaim_type].desync == 1)
			ImGui::Keybind(crypt_str("Desync inverter"), &g_cfg.keybinds.key[FLIP_DESYNC_KEYBIND], crypt_str("##HOTKEY_INVERT_DESYNC"));
		
		if (g_cfg.keybinds.key[FLIP_DESYNC_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[FLIP_DESYNC_KEYBIND].key < KEY_MAX)
		{
			ImGui::SliderInt(crypt_str("Inverted yaw lean"), &g_cfg.antiaim.type[this->antiaim_type].inverted_body_lean, 0, 100);
			ImGui::SliderInt(crypt_str("Inverted desync width"), &g_cfg.antiaim.type[this->antiaim_type].inverted_desync_range, 1, 60);
		}
	}
	
	ImGui::EndChild();
}

void c_menu::draw_legit_main_tab()
{
	/* LEFT. */

	/* General. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild(std::string(this->get_legit_group() + crypt_str("general")).c_str(), ImVec2(230, 199));

	ImGui::Checkbox(crypt_str("Master switch"), &g_cfg.legitbot.enabled);
	ImGui::Keybind(crypt_str("Legit key"), &g_cfg.keybinds.key[LEGIT_BOT_KEYBIND], crypt_str("##KEYBIND_LEGIT"));

	if (g_cfg.legitbot.enabled)
		g_cfg.ragebot.enable = false;

	ImGui::Checkbox(crypt_str("Fire at teammates"), &g_cfg.legitbot.friendly_fire);
	ImGui::Checkbox(crypt_str("Automatic pistols"), &g_cfg.legitbot.autopistol);

	ImGui::Checkbox(crypt_str("Automatic scope"), &g_cfg.legitbot.autoscope);
	if (g_cfg.legitbot.autoscope)
		ImGui::Checkbox(crypt_str("Unscope after shot"), &g_cfg.legitbot.unscope);

	ImGui::Checkbox(crypt_str("Automatic stop"), &g_cfg.legitbot.weapon[hooks::legit_weapon].auto_stop);
	ImGui::Checkbox(crypt_str("Enable on zoom only if sniper"), &g_cfg.legitbot.sniper_in_zoom_only);
	ImGui::Checkbox(crypt_str("Do if in air"), &g_cfg.legitbot.do_if_local_in_air);
	ImGui::Checkbox(crypt_str("Ignore flashes"), &g_cfg.legitbot.do_if_local_flashed);
	ImGui::Checkbox(crypt_str("Ignore smokes"), &g_cfg.legitbot.do_if_enemy_in_smoke);

	ImGui::EndChild();

	/* Field of view. */
	ImGui::SetCursorPos(ImVec2(10, 239));
	ImGui::BeginChild(std::string(this->get_legit_group() + crypt_str("field of view")).c_str(), ImVec2(230, 183));

	ImGui::Combo(crypt_str("Field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].fov_type, legit_fov, IM_ARRAYSIZE(legit_fov));
	ImGui::SliderFloat(crypt_str("Field of view amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].fov, 0.f, 30.f, crypt_str("%.2f"));
	ImGui::SliderFloat(crypt_str("Silent field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].silent_fov, 0.f, 30.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].silent_fov ? crypt_str("None") : crypt_str("%.2f"))); //-V550

	ImGui::EndChild();

	/* RIGHT. */

	/* Accuracy. */
	ImGui::SetCursorPos(ImVec2(245, 35));
	ImGui::BeginChild(std::string(this->get_legit_group() + crypt_str("accuracy")).c_str(), ImVec2(230, 137));

	ImGui::Combo(crypt_str("Smoothing type"), &g_cfg.legitbot.weapon[hooks::legit_weapon].smooth_type, legit_smooth, IM_ARRAYSIZE(legit_smooth));
	ImGui::SliderFloat(crypt_str("Smoothing amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].smooth, 1.f, 12.f, crypt_str("%.1f"));
	ImGui::Combo(crypt_str("Recoil type"), &g_cfg.legitbot.weapon[hooks::legit_weapon].rcs_type, rcs_type, IM_ARRAYSIZE(rcs_type));
	ImGui::SliderFloat(crypt_str("Recoil amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].rcs, 0.f, 100.f, crypt_str("%.0f%%"), 1.f);
	ImGui::SliderFloat(crypt_str("Recoil custom field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov, 0.f, 30.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov ? crypt_str("None") : crypt_str("%.2f"))); //-V550
	ImGui::SliderFloat(crypt_str("Recoil custom smoothing"), &g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth, 0.f, 12.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth ? crypt_str("None") : crypt_str("%.1f"))); //-V550
	ImGui::SliderInt(crypt_str("Automatic penetration damage"), &g_cfg.legitbot.weapon[hooks::legit_weapon].awall_dmg, 0, 100, false, (!g_cfg.legitbot.weapon[hooks::legit_weapon].awall_dmg ? crypt_str("None") : crypt_str("%d")));
	ImGui::SliderFloat(crypt_str("Delay between target switch"), &g_cfg.legitbot.weapon[hooks::legit_weapon].target_switch_delay, 0.f, 1.f);

	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(245, 177));
	ImGui::BeginChild(std::string(this->get_legit_group() + crypt_str("hitbox")).c_str(), ImVec2(230, 81));

	ImGui::SetCursorPosY(-10);
	ImGui::Combo(crypt_str(""), &g_cfg.legitbot.weapon[hooks::legit_weapon].priority, legit_hitbox, IM_ARRAYSIZE(legit_hitbox));

	ImGui::EndChild();

	/* Triggerbot. */
	ImGui::SetCursorPos(ImVec2(245, 263));
	ImGui::BeginChild(std::string(this->get_legit_group() + crypt_str("triggerbot")).c_str(), ImVec2(230, 159));

	ImGui::Keybind(crypt_str("Trigger key"), &g_cfg.keybinds.key[AUTO_FIRE_KEYBIND], crypt_str("##KEYBIND_TRIGGERBOT"));
	ImGui::SliderInt(crypt_str("Trigger hit chance"), &g_cfg.legitbot.weapon[hooks::legit_weapon].autofire_hitchance, 0, 100, false, (!g_cfg.legitbot.weapon[hooks::legit_weapon].autofire_hitchance ? crypt_str("None") : crypt_str("%d")));
	ImGui::SliderInt(crypt_str("Trigger delay between shots"), &g_cfg.legitbot.autofire_delay, 0, 12, false, (!g_cfg.legitbot.autofire_delay ? crypt_str("None") : (g_cfg.legitbot.autofire_delay == 1 ? crypt_str("%d tick") : crypt_str("%d ticks"))));

	ImGui::EndChild();
}

void c_menu::draw_visual_player_esp_tab()
{
	/* LEFT. */

	/* General. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild(std::string(this->get_esp_group() + crypt_str("general")).c_str(), ImVec2(230, 358));

	ImGui::Checkbox(crypt_str("Master switch"), &g_cfg.player.enable);
	ImGui::Checkbox(crypt_str("Box"), &g_cfg.player.type[this->visual_sub].box);
	ImGui::ColorEdit(crypt_str("Box colour"), &g_cfg.player.type[this->visual_sub].box_color);
	ImGui::Checkbox(crypt_str("Name"), &g_cfg.player.type[this->visual_sub].name);
	ImGui::ColorEdit(crypt_str("Name colour"), &g_cfg.player.type[this->visual_sub].name_color);
	ImGui::Checkbox(crypt_str("Health bar"), &g_cfg.player.type[this->visual_sub].health);
	
	ImGui::Checkbox(crypt_str("Custom health bar color"), &g_cfg.player.type[this->visual_sub].custom_health_color);
	if (g_cfg.player.type[this->visual_sub].custom_health_color)
		ImGui::ColorEdit(crypt_str("Health bar colour"), &g_cfg.player.type[this->visual_sub].health_color);
	
	ImGui::MultiCombo(crypt_str("Weapon"), weaponplayer, g_cfg.player.type[this->visual_sub].weapon, IM_ARRAYSIZE(weaponplayer));
	if (g_cfg.player.type[this->visual_sub].weapon[WEAPON_ICON] || g_cfg.player.type[this->visual_sub].weapon[WEAPON_TEXT])
	{
		ImGui::ColorEdit(crypt_str("Weapon colour"), &g_cfg.player.type[this->visual_sub].weapon_color);
	}

	ImGui::MultiCombo(crypt_str("Flags"), flags, g_cfg.player.type[this->visual_sub].flags, IM_ARRAYSIZE(flags));
	ImGui::Checkbox(crypt_str("Ammo bar"), &g_cfg.player.type[this->visual_sub].ammo);
	ImGui::ColorEdit(crypt_str("Ammo bar colour"), &g_cfg.player.type[this->visual_sub].ammobar_color);

	ImGui::EndChild();

	/* RIGHT. */

	/* Offscreen (only in enemy). */
	if (this->visual_sub == ENEMY || this->visual_sub == LOCAL)
	{
		auto additional_or_offscreen = 137;
		if (this->visual_sub == LOCAL)
			additional_or_offscreen = 158;

		ImGui::SetCursorPos(ImVec2(245, 35));
		ImGui::BeginChild(std::string(this->get_esp_group() + (this->visual_sub == ENEMY ? crypt_str("offscreen") : crypt_str("additional"))).c_str(), ImVec2(230, additional_or_offscreen));

		if (this->visual_sub == ENEMY)
		{
			ImGui::Checkbox(crypt_str("Offscreen arrows"), &g_cfg.player.arrows);
			ImGui::ColorEdit(crypt_str("Offscreen arrows colour"), &g_cfg.player.arrows_color);
			if (g_cfg.player.arrows)
			{
				ImGui::SliderInt(crypt_str("Offscreen arrows distance"), &g_cfg.player.distance, 1, 100);
				ImGui::SliderInt(crypt_str("Offscreen arrows size"), &g_cfg.player.size, 1, 100);
			}
		}
		else
		{
			ImGui::Keybind(crypt_str("Thirdperson camera"), &g_cfg.keybinds.key[THIRDPERSON_KEYBIND], crypt_str("##KEYBIND_THIRDPERSON"));

			ImGui::Checkbox(crypt_str("Force thirdperson when spectating"), &g_cfg.misc.thirdperson_when_spectating);
			if (g_cfg.keybinds.key[THIRDPERSON_KEYBIND].key > KEY_NONE && g_cfg.keybinds.key[THIRDPERSON_KEYBIND].key < KEY_MAX)
				ImGui::SliderInt(crypt_str("Thirdperson camera distance"), &g_cfg.misc.thirdperson_distance, 100, 300);

			ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.esp.fov, 0, 150);
			ImGui::Checkbox(crypt_str("Field of view while zoomed"), &g_cfg.esp.fov_while_zoomed);

			ImGui::Checkbox(crypt_str("Client bullet impacts"), &g_cfg.esp.client_bullet_impacts);
			ImGui::ColorEdit(crypt_str("Client bullet impacts colour"), &g_cfg.esp.client_bullet_impacts_color);

			ImGui::Checkbox(crypt_str("Server impacts"), &g_cfg.esp.server_bullet_impacts);
			ImGui::ColorEdit(crypt_str("Server bullet impacts colour"), &g_cfg.esp.server_bullet_impacts_color);

			ImGui::Checkbox(crypt_str("Local bullet tracer"), &g_cfg.esp.bullet_tracer);
			ImGui::ColorEdit(crypt_str("Bullet tracer colour"), &g_cfg.esp.bullet_tracer_color);
		}

		ImGui::EndChild();
	}

	auto details_pos = 35;
	auto details_height = 358;

	if (this->visual_sub == ENEMY)
	{
		details_pos = 177;
		details_height = 216;
	}
	else if (this->visual_sub == LOCAL)
	{
		details_pos = 198;
		details_height = 195;
	}

	/* Details. */
	ImGui::SetCursorPos(ImVec2(245, details_pos));
	ImGui::BeginChild(std::string(this->get_esp_group() + crypt_str("details")).c_str(), ImVec2(230, details_height));

	ImGui::Checkbox(crypt_str("Footstep ring"), &g_cfg.player.type[this->visual_sub].footsteps);
	ImGui::ColorEdit(crypt_str("Footstep ring colour"), &g_cfg.player.type[this->visual_sub].footsteps_color);

	if (g_cfg.player.type[this->visual_sub].footsteps)
	{
		ImGui::SliderInt(crypt_str("Footstep ring thickness"), &g_cfg.player.type[this->visual_sub].thickness, 1, 10);
		ImGui::SliderInt(crypt_str("Footstep ring radius"), &g_cfg.player.type[this->visual_sub].radius, 50, 500);
	}

	if (this->visual_sub == ENEMY || this->visual_sub == TEAM)
	{
		ImGui::Checkbox(crypt_str("Snap lines"), &g_cfg.player.type[this->visual_sub].snap_lines);
		ImGui::ColorEdit(crypt_str("Snap lines colour"), &g_cfg.player.type[this->visual_sub].snap_lines_color);

		if (this->visual_sub == ENEMY)
		{
			ImGui::Checkbox(crypt_str("Chams on shot"), &g_cfg.player.shot_record_chams);
			ImGui::ColorEdit(crypt_str("Chams on shot colour"), &g_cfg.player.shot_record_color);
			ImGui::SliderInt(crypt_str("Chams on shot duration"), &g_cfg.player.shot_record_duration, 0, 10);

			ImGui::Checkbox(crypt_str("Enemy bullet tracer"), &g_cfg.esp.enemy_bullet_tracer);
			ImGui::ColorEdit(crypt_str("Enemy bullet tracer colour"), &g_cfg.esp.enemy_bullet_tracer_color);
		}
	}

	ImGui::Checkbox(crypt_str("Skeleton chams"), &g_cfg.player.type[this->visual_sub].skeleton);
	ImGui::ColorEdit(crypt_str("Skeleton chams colour"), &g_cfg.player.type[this->visual_sub].skeleton_color);

	ImGui::EndChild();
}

void c_menu::draw_visual_player_chams_tab()
{
	/* LEFT. */

	/* General. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild(std::string(this->get_esp_group() + crypt_str("chams")).c_str(), ImVec2(230, 358));

	if (this->visual_sub == LOCAL)
		ImGui::Combo(crypt_str("Type"), &g_cfg.player.local_chams_type, local_chams_type, IM_ARRAYSIZE(local_chams_type));

	if (this->visual_sub != LOCAL || !g_cfg.player.local_chams_type)
		ImGui::MultiCombo(crypt_str("Chams"), g_cfg.player.type[this->visual_sub].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis, g_cfg.player.type[this->visual_sub].chams, g_cfg.player.type[this->visual_sub].chams[PLAYER_CHAMS_VISIBLE] ? IM_ARRAYSIZE(chamsvisact) : IM_ARRAYSIZE(chamsvis));

	if (g_cfg.player.type[this->visual_sub].chams[PLAYER_CHAMS_VISIBLE] || this->visual_sub == LOCAL && g_cfg.player.local_chams_type) //-V648
	{
		if (this->visual_sub == LOCAL && g_cfg.player.local_chams_type)
		{
			ImGui::Checkbox(crypt_str("Desync chams"), &g_cfg.player.fake_chams_enable);
			ImGui::Checkbox(crypt_str("Uninterpolated desync origin"), &g_cfg.player.visualize_lag);
			ImGui::Checkbox(crypt_str("Desync double layered"), &g_cfg.player.layered);
			ImGui::Combo(crypt_str("Desync material"), &g_cfg.player.fake_chams_type, chamstype, IM_ARRAYSIZE(chamstype));
			
			if (g_cfg.player.fake_chams_type != 6)
				ImGui::Checkbox(crypt_str("Desync double material"), &g_cfg.player.fake_double_material);

			ImGui::Checkbox(crypt_str("Desync animated material"), &g_cfg.player.fake_animated_material);
		}
		else
		{
			ImGui::Combo(crypt_str("Player chams material"), &g_cfg.player.type[this->visual_sub].chams_type, chamstype, IM_ARRAYSIZE(chamstype));

			if (g_cfg.player.type[this->visual_sub].chams_type != 6)
				ImGui::Checkbox(crypt_str("Double chams material"), &g_cfg.player.type[this->visual_sub].double_material);

			ImGui::Checkbox(crypt_str("Animated chams material"), &g_cfg.player.type[this->visual_sub].animated_material);

			if (this->visual_sub == ENEMY)
			{
				ImGui::Checkbox(crypt_str("History chams"), &g_cfg.player.backtrack_chams);

				if (g_cfg.player.backtrack_chams)
					ImGui::Combo(crypt_str("History chams material"), &g_cfg.player.backtrack_chams_material, chamstype, IM_ARRAYSIZE(chamstype));
			}
		}
	}

	if (this->visual_sub == ENEMY || this->visual_sub == TEAM)
	{
		ImGui::Checkbox(crypt_str("Ragdoll chams"), &g_cfg.player.type[this->visual_sub].ragdoll_chams);

		if (g_cfg.player.type[this->visual_sub].ragdoll_chams)
			ImGui::Combo(crypt_str("Ragdoll chams material"), &g_cfg.player.type[this->visual_sub].ragdoll_chams_material, chamstype, IM_ARRAYSIZE(chamstype));
	}
	else if (!g_cfg.player.local_chams_type)
	{
		ImGui::Checkbox(crypt_str("Transparency in scope"), &g_cfg.player.transparency_in_scope);

		if (g_cfg.player.transparency_in_scope)
			ImGui::SliderFloat(crypt_str("Alpha"), &g_cfg.player.transparency_in_scope_amount, 0.0f, 1.0f);
	}

	ImGui::Checkbox(crypt_str("Glow"), &g_cfg.player.type[this->visual_sub].glow);

	if (g_cfg.player.type[this->visual_sub].glow)
		ImGui::Combo(crypt_str("Glow type"), &g_cfg.player.type[this->visual_sub].glow_type, glowtype, IM_ARRAYSIZE(glowtype));

	ImGui::EndChild();

	/* RIGHT. */

	/* Colour. */
	ImGui::SetCursorPos(ImVec2(245, 35));
	ImGui::BeginChild(std::string(this->get_esp_group() + crypt_str("chams colour")).c_str(), ImVec2(230, 358));

	if (g_cfg.player.type[this->visual_sub].chams[PLAYER_CHAMS_VISIBLE] || this->visual_sub == LOCAL && g_cfg.player.local_chams_type)
	{
		if (this->visual_sub == LOCAL && g_cfg.player.local_chams_type)
		{
			ImGui::ColorEdit(crypt_str("Desync colour"), &g_cfg.player.fake_chams_color);
			ImGui::ColorEdit(crypt_str("Desync double material colour"), &g_cfg.player.fake_double_material_color);
			ImGui::ColorEdit(crypt_str("Desync animated material colour"), &g_cfg.player.fake_animated_material_color);
		}
		else
		{
			ImGui::ColorEdit(crypt_str("Visible chams colour"), &g_cfg.player.type[this->visual_sub].chams_color);
			ImGui::ColorEdit(crypt_str("Behind wall chams colour"), &g_cfg.player.type[this->visual_sub].xqz_color);
			ImGui::ColorEdit(crypt_str("Double material chams colour"), &g_cfg.player.type[this->visual_sub].double_material_color);

			ImGui::ColorEdit(crypt_str("Animated material chams colour"), &g_cfg.player.type[this->visual_sub].animated_material_color);
			
			if (this->visual_sub == ENEMY)
				ImGui::ColorEdit(crypt_str("History chams colour"), &g_cfg.player.backtrack_chams_color);
		}
	}
	
	if (this->visual_sub == ENEMY || this->visual_sub == TEAM)
		ImGui::ColorEdit(crypt_str("Ragdoll chams colour"), &g_cfg.player.type[this->visual_sub].ragdoll_chams_color);
	
	ImGui::ColorEdit(crypt_str("Glow colour"), &g_cfg.player.type[this->visual_sub].glow_color);
	
	ImGui::EndChild();
}

void c_menu::draw_visual_other_esp_tab()
{
	/* LEFT. */

	/* World. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild("World", ImVec2(230, 190));

	ImGui::Checkbox(crypt_str("Master switch"), &g_cfg.player.enable);
	ImGui::Checkbox(crypt_str("Rain"), &g_cfg.esp.rain);
	ImGui::Checkbox(crypt_str("Full bright"), &g_cfg.esp.bright);
	ImGui::Combo(crypt_str("Skybox"), &g_cfg.esp.skybox, skybox, IM_ARRAYSIZE(skybox));
	ImGui::ColorEdit(crypt_str("Skybox colour"), &g_cfg.esp.skybox_color, ImGuiColorEditFlags_NoAlpha);

	if (g_cfg.esp.skybox == 21)
	{
		static char sky_custom[64] = "\0";

		if (!g_cfg.esp.custom_skybox.empty())
			strcpy_s(sky_custom, sizeof(sky_custom), g_cfg.esp.custom_skybox.c_str());

		if (ImGui::InputText(crypt_str("Custom skybox name"), sky_custom, sizeof(sky_custom)))
			g_cfg.esp.custom_skybox = sky_custom;
	}
	ImGui::Checkbox(crypt_str("World color modulation"), &g_cfg.esp.nightmode);

	if (g_cfg.esp.nightmode)
	{
		ImGui::ColorEdit(crypt_str("World modulation color"), &g_cfg.esp.world_color);
		ImGui::ColorEdit(crypt_str("World props color"), &g_cfg.esp.props_color);
	}

	ImGui::Checkbox(crypt_str("World modulation"), &g_cfg.esp.world_modulation);
	if (g_cfg.esp.world_modulation)
	{
		ImGui::SliderFloat(crypt_str("World bloom"), &g_cfg.esp.bloom, 0.0f, 750.0f);
		ImGui::SliderFloat(crypt_str("World exposure"), &g_cfg.esp.exposure, 0.0f, 2000.0f);
		ImGui::SliderFloat(crypt_str("World ambient"), &g_cfg.esp.ambient, 0.0f, 1500.0f);
	}

	ImGui::Checkbox(crypt_str("Fog modulation"), &g_cfg.esp.fog);
	if (g_cfg.esp.fog)
	{
		ImGui::SliderInt(crypt_str("Fog distance"), &g_cfg.esp.fog_distance, 0, 2500);
		ImGui::SliderInt(crypt_str("Fog density"), &g_cfg.esp.fog_density, 0, 100);
		ImGui::ColorEdit(crypt_str("Fog colour"), &g_cfg.esp.fog_color, ImGuiColorEditFlags_NoAlpha);
	}

	ImGui::EndChild();

	/* Grenade. */
	ImGui::SetCursorPos(ImVec2(10, 230));
	ImGui::BeginChild("Grenade", ImVec2(230, 163));

	ImGui::Checkbox(crypt_str("Trajectory"), &g_cfg.esp.grenade_prediction);
	if (g_cfg.esp.grenade_prediction)
	{
		ImGui::Checkbox(crypt_str("Only show trajectory when click"), &g_cfg.esp.on_click);
		ImGui::ColorEdit(crypt_str("Trajectory line colour"), &g_cfg.esp.grenade_prediction_color);
		ImGui::ColorEdit(crypt_str("Trajectory circle colour"), &g_cfg.esp.grenade_prediction_tracer_color);
	}

	ImGui::Checkbox(crypt_str("Proximity warning"), &g_cfg.esp.grenade_proximity_warning);
	if (g_cfg.esp.grenade_proximity_warning)
	{
		ImGui::Combo(crypt_str("Proximity tracer"), &g_cfg.esp.grenade_proximity_tracers_mode, proximity_tracers_mode, IM_ARRAYSIZE(proximity_tracers_mode));
		
		if (g_cfg.esp.grenade_proximity_tracers_mode != 0)
		{
			ImGui::Checkbox(crypt_str("Rainbow proximity tracer"), &g_cfg.esp.grenade_proximity_tracers_rainbow);
			
			if (!g_cfg.esp.grenade_proximity_tracers_rainbow)
				ImGui::ColorEdit(crypt_str("Proximity tracers colour"), &g_cfg.esp.grenade_proximity_tracers_colors, ImGuiColorEditFlags_NoAlpha);
			
			if (g_cfg.esp.grenade_proximity_tracers_mode == 2)
			{

				ImGui::SliderInt(crypt_str("Proximity width"), &g_cfg.esp.proximity_tracers_width, 0.0f, 20.f);
				ImGui::SliderInt(crypt_str("Proximity end width"), &g_cfg.esp.proximity_tracers_end_width, 0.f, 20.f);
			}
		}

		ImGui::Checkbox(crypt_str("Offscreen proximity"), &g_cfg.esp.offscreen_proximity);
		ImGui::ColorEdit(crypt_str("Offscreen proximity colour"), &g_cfg.esp.grenade_proximity_warning_inner_color);
		ImGui::ColorEdit(crypt_str("Offscreen proximity danger colour"), &g_cfg.esp.grenade_proximity_warning_inner_danger_color, ImGuiColorEditFlags_NoAlpha);
		ImGui::ColorEdit(crypt_str("Offscreen proximity progress colour"), &g_cfg.esp.grenade_proximity_warning_progress_color);
	}
	
	if (g_cfg.esp.projectiles)
		ImGui::MultiCombo(crypt_str("Grenade ESP"), proj_combo, g_cfg.esp.grenade_esp, IM_ARRAYSIZE(proj_combo));
	
	if (g_cfg.esp.grenade_esp[GRENADE_ICON] || g_cfg.esp.grenade_esp[GRENADE_TEXT])
		ImGui::ColorEdit(crypt_str("ESP text colour"), &g_cfg.esp.projectiles_color);
	
	if (g_cfg.esp.grenade_esp[GRENADE_BOX])
		ImGui::ColorEdit(crypt_str("ESP box colour"), &g_cfg.esp.grenade_box_color);
	
	if (g_cfg.esp.grenade_esp[GRENADE_GLOW])
		ImGui::ColorEdit(crypt_str("Glow colour"), &g_cfg.esp.grenade_glow_color);
	
	ImGui::Checkbox(crypt_str("Inferno timer"), &g_cfg.esp.molotov_timer);
	ImGui::ColorEdit(crypt_str("Inferno timer colour"), &g_cfg.esp.molotov_timer_color);
	ImGui::Checkbox(crypt_str("Inferno radius"), &g_cfg.esp.inferno_radius);
	if (g_cfg.esp.inferno_radius)
	{
		ImGui::Combo(crypt_str("Inferno radius type"), &g_cfg.esp.inferno_radius_type, radiustype, IM_ARRAYSIZE(radiustype));
		ImGui::ColorEdit(crypt_str("Inferno radius colour"), &g_cfg.esp.inferno_radius_color);
	}
	ImGui::Checkbox(crypt_str("Smoke timer"), &g_cfg.esp.smoke_timer);
	ImGui::ColorEdit(crypt_str("Smoke timer colour"), &g_cfg.esp.smoke_timer_color);
	ImGui::Checkbox(crypt_str("Smoke radius"), &g_cfg.esp.smoke_radius);
	if (g_cfg.esp.smoke_radius)
		ImGui::ColorEdit(crypt_str("Smoke radius colour"), &g_cfg.esp.smoke_radius_color);

	ImGui::EndChild();

	/* RIGHT. */

	/* Details. */
	ImGui::SetCursorPos(ImVec2(245, 35));
	ImGui::BeginChild(crypt_str("Details"), ImVec2(230, 168));

	ImGui::MultiCombo(crypt_str("Indicators"), indicators, g_cfg.esp.indicators, IM_ARRAYSIZE(indicators));
	ImGui::MultiCombo(crypt_str("Removals"), removals, g_cfg.esp.removals, IM_ARRAYSIZE(removals));
	ImGui::Checkbox(crypt_str("Force game crosshair"), &g_cfg.esp.force_crosshair);
	ImGui::Checkbox(crypt_str("Taser circle range"), &g_cfg.esp.taser_range);
	ImGui::Checkbox(crypt_str("Spread circle"), &g_cfg.esp.show_spread);
	ImGui::ColorEdit(crypt_str("Spread circle colour"), &g_cfg.esp.show_spread_color);
	ImGui::Checkbox(crypt_str("Penetration crosshair"), &g_cfg.esp.penetration_reticle);
	ImGui::Combo(crypt_str("Penetration crosshair type"), &g_cfg.esp.penetration_reticle_type, reticletype, IM_ARRAYSIZE(reticletype));

	ImGui::EndChild();

	/* Viewmodel. */
	ImGui::SetCursorPos(ImVec2(245, 208));
	ImGui::BeginChild(crypt_str("Viewmodel"), ImVec2(230, 185));

	ImGui::Checkbox(crypt_str("Play rare animations"), &g_cfg.skins.rare_animations);
	ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.esp.viewmodel_fov, 0, 89);
	ImGui::SliderInt(crypt_str("X"), &g_cfg.esp.viewmodel_x, -50, 50);
	ImGui::SliderInt(crypt_str("Y"), &g_cfg.esp.viewmodel_y, -50, 50);
	ImGui::SliderInt(crypt_str("Z"), &g_cfg.esp.viewmodel_z, -50, 50);
	ImGui::SliderInt(crypt_str("Roll"), &g_cfg.esp.viewmodel_roll, -180, 180);

	ImGui::EndChild();
}

void c_menu::draw_visual_other_chams_tab()
{
	/* LEFT. */

	/* Chams. */
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild(crypt_str("Chams"), ImVec2(230, 358));

	ImGui::Checkbox(crypt_str("Arms chams"), &g_cfg.esp.arms_chams);
	
	ImGui::Combo(crypt_str("Arm chams material"), &g_cfg.esp.arms_chams_type, chamstype, IM_ARRAYSIZE(chamstype));
	if (g_cfg.esp.arms_chams_type != 6)
		ImGui::Checkbox(crypt_str("Arm chams double material"), &g_cfg.esp.arms_double_material);

	ImGui::Checkbox(crypt_str("Arm chams animated material"), &g_cfg.esp.arms_animated_material);
	ImGui::Checkbox(crypt_str("Dropped chams"), &g_cfg.esp.weapon_chams);
	
	ImGui::Combo(crypt_str("Dropped chams material"), &g_cfg.esp.weapon_chams_type, chamstype, IM_ARRAYSIZE(chamstype));
	if (g_cfg.esp.weapon_chams_type != 6)
		ImGui::Checkbox(crypt_str("Dropped chams double material"), &g_cfg.esp.weapon_double_material);

	ImGui::Checkbox(crypt_str("Dropped chams animated material"), &g_cfg.esp.weapon_animated_material);

	ImGui::EndChild();

	/* RIGHT. */

	/* Colour. */
	ImGui::SetCursorPos(ImVec2(245, 35));
	ImGui::BeginChild(crypt_str("Chams colour"), ImVec2(230, 358));

	ImGui::ColorEdit(crypt_str("Arms chams colour"), &g_cfg.esp.arms_chams_color);
	ImGui::ColorEdit(crypt_str("Arm chams double colour"), &g_cfg.esp.arms_double_material_color);
	ImGui::ColorEdit(crypt_str("Arm chams animated colour"), &g_cfg.esp.arms_animated_material_color);
	ImGui::ColorEdit(crypt_str("Dropped chams colour"), &g_cfg.esp.weapon_chams_color);
	ImGui::ColorEdit(crypt_str("Dropped chams double colour"), &g_cfg.esp.weapon_double_material_color);
	ImGui::ColorEdit(crypt_str("Dropped chams animated colour"), &g_cfg.esp.weapon_animated_material_color);

	ImGui::EndChild();
}

void c_menu::draw_skinchanger_tab()
{
	/* animation. */
	static bool drugs = false;
	static bool active_animation = false;
	static bool preview_reverse = false;
	static float switch_alpha = 1.f;
	static int next_id = -1;

	if (active_animation)
	{
		if (preview_reverse)
		{
			if (switch_alpha == 1.f)
			{
				preview_reverse = false;
				active_animation = false;
			}

			switch_alpha = math::clamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
		else
		{
			if (switch_alpha == 0.01f)
				preview_reverse = true;

			switch_alpha = math::clamp(switch_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
	}
	else
		switch_alpha = math::clamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

	if (this->current_profile == -1)
	{
		ImGui::SetCursorPos(ImVec2(10, 5));

		ImGui::BeginChild(crypt_str("Skin changer"), ImVec2(465, 417));

		ImGui::SetCursorPos(ImVec2(16, 25));
		ImGui::BeginGroup();

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f * this->public_alpha * switch_alpha);

		auto same_line_counter = 0;

		for (auto i = 0; i < g_cfg.skins.skinChanger.size(); i++)
		{
			if (!this->all_skins[i])
			{
				g_cfg.skins.skinChanger.at(i).update();
				this->all_skins[i] = util::get_skin_preview( this->get_weapon_name_from_id(i, (i == 0 || i == 1) ? g_cfg.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_cfg.skins.skinChanger.at(i).skin_name, g_ctx.globals.draw_device);
			}

			if (ImGui::ImageButton(this->all_skins[i], ImVec2(70, 61)))
			{
				next_id = i;
				active_animation = true;
			}

			if (active_animation && preview_reverse)
			{
				ImGui::SetScrollY(0);
				this->current_profile = next_id;
			}

			if (same_line_counter < 4)
			{
				ImGui::SameLine();
				same_line_counter++;
			}
			else
			{
				ImGui::CustomSpacing(8.f);
				same_line_counter = 0;
			}
		}
		ImGui::PopStyleVar();
		ImGui::EndGroup();
		ImGui::EndChild();
	}

	/* Skin changer shit. */
	if (this->current_profile != -1)
	{
		static bool need_update[36];
		static bool leave;

		if (!this->all_skins[this->current_profile] || need_update[this->current_profile])
		{
			this->all_skins[this->current_profile] = util::get_skin_preview(this->get_weapon_name_from_id(this->current_profile, (this->current_profile == 0 || this->current_profile == 1) ? g_cfg.skins.skinChanger.at(this->current_profile).definition_override_vector_index : -1, this->current_profile == 0).c_str(), g_cfg.skins.skinChanger.at(this->current_profile).skin_name, g_ctx.globals.draw_device);
			need_update[this->current_profile] = false;
		}

		auto& selected_entry = g_cfg.skins.skinChanger[this->current_profile];
		selected_entry.itemIdIndex = this->current_profile;

		std::string weapon_name = crypt_str("");
		std::string grouping_tab = crypt_str("");

		switch (this->current_profile)
		{
		case 0:
		{
			grouping_tab = crypt_str("Knife model");
			weapon_name = game_data::knife_names[selected_entry.definition_override_vector_index].name;
			break;
		}
		case 1:
		{
			grouping_tab = crypt_str("Glove model");
			weapon_name = game_data::glove_names[selected_entry.definition_override_vector_index].name;
			break;
		}
		default:
		{
			grouping_tab = crypt_str("Weapon model");
			weapon_name = game_data::weapon_names[this->current_profile].name;
			break;
		}
		}

		static char search_skins[64] = "\0";
		static auto item_index = selected_entry.paint_kit_vector_index;

		if (this->current_profile == 0 || this->current_profile == 1)
		{
			ImGui::SetCursorPos(ImVec2(10, 5));
			ImGui::BeginChild(grouping_tab.c_str(), ImVec2(230, 81));

			if (this->current_profile == 0)
			{
				ImGui::SetCursorPosY(-10);
				if (ImGui::Combo(crypt_str(""), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = game_data::knife_names[idx].name;
						return true;
					}, nullptr, IM_ARRAYSIZE(game_data::knife_names)))
					need_update[this->current_profile] = true; 
			}
			else if (this->current_profile == 1)
			{
				ImGui::SetCursorPosY(-10);
				if (ImGui::Combo(crypt_str(""), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = game_data::glove_names[idx].name;
						return true;
					}, nullptr, IM_ARRAYSIZE(game_data::glove_names)))
				{
					item_index = 0;
					need_update[this->current_profile] = true;
				}
			}
			ImGui::EndChild();
		}
		else
			selected_entry.definition_override_vector_index = 0;

		/* Skin changer stuff. */
		ImGui::SetCursorPos(ImVec2(10, (this->current_profile == 0 || this->current_profile == 1) ? 91 : 5));
		ImGui::BeginChild(weapon_name.c_str(), ImVec2(230, (this->current_profile == 0 || this->current_profile == 1) ? 331 : 417));

		if (this->current_profile != 1)
			if (ImGui::InputText(crypt_str("Search"), search_skins, sizeof(search_skins)))
				item_index = -1;

		auto main_kits = this->current_profile == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;
		auto display_index = 0;

		SkinChanger::displayKits = main_kits;

		if (this->current_profile == 1)
		{
			for (auto i = 0; i < main_kits.size(); i++)
			{
				auto main_name = main_kits.at(i).name;

				for (auto i = 0; i < main_name.size(); i++)
					if (iswalpha((main_name.at(i))))
						main_name.at(i) = towlower(main_name.at(i));

				char search_name[64] = "\0";;

				if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, crypt_str("Hydra")))
					strcpy_s(search_name, sizeof(search_name), crypt_str("Bloodhound"));
				else
					strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

				for (auto i = 0; i < sizeof(search_name); i++)
					if (iswalpha(search_name[i]))
						search_name[i] = towlower(search_name[i]);

				if (main_name.find(search_name) != std::string::npos)
				{
					SkinChanger::displayKits.at(display_index) = main_kits.at(i);
					display_index++;
				}
			}

			SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
		}
		else
		{
			if (strcmp(search_skins, crypt_str("")))
			{
				for (auto i = 0; i < main_kits.size(); i++)
				{
					auto main_name = main_kits.at(i).name;

					for (auto i = 0; i < main_name.size(); i++)
						if (iswalpha(main_name.at(i)))
							main_name.at(i) = towlower(main_name.at(i));

					char search_name[64] = "\0";;
					strcpy_s(search_name, sizeof(search_name), search_skins);

					for (auto i = 0; i < sizeof(search_name); i++)
						if (iswalpha(search_name[i]))
							search_name[i] = towlower(search_name[i]);

					if (main_name.find(search_name) != std::string::npos)
					{
						SkinChanger::displayKits.at(display_index) = main_kits.at(i);
						display_index++;
					}
				}

				SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
			}
			else
				item_index = selected_entry.paint_kit_vector_index;
		}

		ImGui::PushItemWidth(210);

		if (!SkinChanger::displayKits.empty())
		{
			auto max_number = 10;

			if (this->current_profile == 1)
				max_number = 12;

			if (ImGui::ListBox(crypt_str(""), &item_index, [](void* data, int idx, const char** out_text) //-V107
				{
					while (SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")) != std::string::npos) //-V807
						SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")), 2, crypt_str("Рµ"));

					*out_text = SkinChanger::displayKits.at(idx).name.c_str();
					return true;
				}, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > max_number ? max_number : SkinChanger::displayKits.size()) || !this->all_skins[this->current_profile])
			{
				SkinChanger::scheduleHudUpdate();
				need_update[this->current_profile] = true;

				auto i = 0;

				while (i < main_kits.size())
				{
					if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
					{
						selected_entry.paint_kit_vector_index = i;
						break;
					}

					i++;
				}
			}
		}

		ImGui::PopItemWidth();

		if (this->current_profile != 0 && this->current_profile != 1)
		{
			if (ImGui::InputInt(crypt_str("Seed"), &selected_entry.seed, 1, 100))
				SkinChanger::scheduleHudUpdate();

			if (ImGui::SliderFloat(crypt_str("Wear"), &selected_entry.wear, 0.0f, 1.0f))
				drugs = true;
			else if (drugs)
			{
				SkinChanger::scheduleHudUpdate();
				drugs = false;
			}
		}

		ImGui::EndChild();

		/* RIGHT. */
		ImGui::SetCursorPos(ImVec2(245, 5));

		/* Preview.*/
		ImGui::BeginChild(crypt_str("Preview"), ImVec2(230, 160));

		ImGui::SetCursorPosY(25);
		ImGui::SetCursorPosX(60);
		ImGui::Image(this->all_skins[this->current_profile], ImVec2(95, 77.5), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, c_menu::get().public_alpha));

		ImGui::EndChild();

		/* Additional.*/
		ImGui::SetCursorPos(ImVec2(245, 170));
		ImGui::BeginChild(crypt_str("Additional"), ImVec2(230, 252));

		if (this->current_profile == 0 || this->current_profile == 1)
		{
			if (ImGui::InputInt(crypt_str("Seed"), &selected_entry.seed, 1, 100))
				SkinChanger::scheduleHudUpdate();

			if (ImGui::SliderFloat(crypt_str("Wear"), &selected_entry.wear, 0.0f, 1.0f))
				drugs = true;
			else if (drugs)
			{
				SkinChanger::scheduleHudUpdate();
				drugs = false;
			}
		}

		if (ImGui::InputInt(crypt_str("StatTrak"), &selected_entry.stat_trak, 1, 15))
			SkinChanger::scheduleHudUpdate();

		if (ImGui::Combo(crypt_str("Quality"), &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = game_data::quality_names[idx].name;
				return true;
			}, nullptr, IM_ARRAYSIZE(game_data::quality_names)))
			SkinChanger::scheduleHudUpdate();

			if (this->current_profile != 1)
			{
				if (!g_cfg.skins.custom_name_tag[this->current_profile].empty())
					strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), g_cfg.skins.custom_name_tag[this->current_profile].c_str());

				if (ImGui::InputText(crypt_str("Name tag"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
				{
					g_cfg.skins.custom_name_tag[this->current_profile] = selected_entry.custom_name;
					SkinChanger::scheduleHudUpdate();
				}
			}

		if (ImGui::CustomButton(crypt_str("Close tab"), crypt_str("##CLOSE__SKING")))
		{
			active_animation = true;
			next_id = -1;
			leave = true;
		}

		ImGui::EndChild();

		selected_entry.update();

		if (leave && (preview_reverse || !active_animation))
		{
			ImGui::SetScrollY(0);
			this->current_profile = next_id;
			leave = false;
		}
	}
}

void c_menu::draw_miscellaneous_tab()
{
	/* LEFT. */

	/* General.*/
	ImGui::SetCursorPos(ImVec2(10, 5));
	ImGui::BeginChild(crypt_str("General"), ImVec2(230, 153));

	if (ImGui::CustomButton(crypt_str("Show hidden convars"), crypt_str("##UNLOCKHIDDENCONVARS")))
		misc::get().unlockhiddenconvars();

	ImGui::Checkbox(crypt_str("Anti untrusted"), &g_cfg.misc.anti_untrusted);
	ImGui::Checkbox(crypt_str("Inventory bypass"), &g_cfg.misc.inventory_access);
	ImGui::Checkbox(crypt_str("Preserve killfeed"), &g_cfg.esp.preserve_killfeed);
	ImGui::Checkbox(crypt_str("Override aspect ratio"), &g_cfg.esp.aspect_ratio);
	if (g_cfg.esp.aspect_ratio)
		ImGui::SliderFloat(crypt_str("Aspect ratio amount"), &g_cfg.esp.aspect_ratio_amount, 1.0f, 2.0f);

	ImGui::Combo(crypt_str("Hitsound"), &g_cfg.esp.hitsound, sounds, IM_ARRAYSIZE(sounds));
	if (g_cfg.esp.hitsound == 7)
	{
		static char hitsound_custom[64] = "\0";

		if (!g_cfg.esp.custom_hitsound.empty())
			strcpy_s(hitsound_custom, sizeof(hitsound_custom), g_cfg.esp.custom_hitsound.c_str());

		if (ImGui::InputText(crypt_str("Custom hitsound name"), hitsound_custom, sizeof(hitsound_custom)))
			g_cfg.esp.custom_hitsound = hitsound_custom;
	}

	ImGui::Checkbox(crypt_str("Killsound"), &g_cfg.esp.killsound);

	ImGui::EndChild();

	/* General.*/
	ImGui::SetCursorPos(ImVec2(10, 163));
	ImGui::BeginChild(crypt_str("Movement"), ImVec2(230, 259));

	ImGui::Keybind(crypt_str("Onshot retreat"), &g_cfg.keybinds.key[AUTO_PEEK_KEYBIND], crypt_str("##KEYBIND_AUTOPEEK"));
	ImGui::Keybind(crypt_str("Edge jump"), &g_cfg.keybinds.key[EDGE_JUMP_KEYBIND], crypt_str("##KEYBIND_EDGEJUMP"));

	ImGui::Combo(crypt_str("Leg movement"), &g_cfg.misc.leg_movement, leg_movement, IM_ARRAYSIZE(leg_movement));

	ImGui::Checkbox(crypt_str("Bunny hop"), &g_cfg.misc.bunnyhop);
	ImGui::Checkbox(crypt_str("Duck in air"), &g_cfg.misc.crouch_in_air);
	ImGui::Combo(crypt_str("Automatic strafe"), &g_cfg.misc.airstrafe, strafes, IM_ARRAYSIZE(strafes));

	ImGui::Checkbox(crypt_str("Remove duck cooldown"), &g_cfg.misc.noduck);
	if (g_cfg.misc.noduck)
		ImGui::Keybind(crypt_str("Fake duck"), &g_cfg.keybinds.key[FAKE_DUCK_KEYBIND], crypt_str("##KEYBIND_FAKEDUCK"));

	ImGui::Checkbox(crypt_str("Fast stop"), &g_cfg.misc.fast_stop);

	ImGui::Keybind(crypt_str("Slow walk"), &g_cfg.keybinds.key[SLOW_WALK_KEYBIND], crypt_str("##KEYBIND_SLOWWALK"));

	ImGui::Combo(crypt_str("Slow walk type"), &g_cfg.misc.slowwalk_type, slowwalk_type, IM_ARRAYSIZE(slowwalk_type));
	if (g_cfg.misc.slowwalk_type)
		ImGui::SliderInt(crypt_str("Slow walk speed"), &g_cfg.misc.slowwalk_speed, 1, 180, false, crypt_str("%du/s"));

	ImGui::EndChild();

	/* RIGHT. */
	ImGui::SetCursorPos(ImVec2(245, 5));
	ImGui::BeginChild(crypt_str("Automatic buy"), ImVec2(230, 228));

	ImGui::Checkbox(crypt_str("Enable"), &g_cfg.misc.buybot_enable);

	ImGui::Combo(crypt_str("Primary"), &g_cfg.misc.buybot1, mainwep, IM_ARRAYSIZE(mainwep));
	ImGui::Combo(crypt_str("Pistols"), &g_cfg.misc.buybot2, secwep, IM_ARRAYSIZE(secwep));
	ImGui::MultiCombo(crypt_str("Utility"), grenades, g_cfg.misc.buybot3, IM_ARRAYSIZE(grenades));

	ImGui::EndChild();

	/* Details. */
	ImGui::SetCursorPos(ImVec2(245, 238));
	ImGui::BeginChild(crypt_str("Details"), ImVec2(230, 184));

	ImGui::Checkbox(crypt_str("Watermark"), &g_cfg.menu.watermark);
	ImGui::Checkbox(crypt_str("Keybind list"), &g_cfg.menu.keybinds);
	ImGui::Checkbox(crypt_str("Spectator list"), &g_cfg.misc.spectators_list);

	ImGui::Combo(crypt_str("Model changer (T)"), &g_cfg.player.player_model_t, player_model_t, IM_ARRAYSIZE(player_model_t));
	ImGui::Combo(crypt_str("Model changer (CT)"), &g_cfg.player.player_model_ct, player_model_ct, IM_ARRAYSIZE(player_model_ct));

	ImGui::MultiCombo(crypt_str("Event log"), events, g_cfg.misc.events_to_log, IM_ARRAYSIZE(events));
	ImGui::MultiCombo(crypt_str("Event log output"), events_output, g_cfg.misc.log_output, IM_ARRAYSIZE(events_output));

	if (g_cfg.misc.events_to_log[EVENTLOG_HIT] || g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] || g_cfg.misc.events_to_log[EVENTLOG_BOMB])
		ImGui::ColorEdit(crypt_str("Event logs colour"), &g_cfg.misc.log_color);

	ImGui::Checkbox(crypt_str("Show event logs in console"), &g_cfg.misc.show_default_log);

	ImGui::Checkbox(crypt_str("Disable interface on screenshot"), &g_cfg.misc.anti_screenshot);
	ImGui::Checkbox(crypt_str("Clan tag changer"), &g_cfg.misc.clantag_spammer);
	ImGui::Checkbox(crypt_str("Chat spam"), &g_cfg.misc.chat);

	ImGui::EndChild();
}

void c_menu::draw_configuration_tab()
{
	static auto should_update = true;

	static auto next_delete = false;
	static auto prenext_delete = false;
	static auto clicked_sure_del = false;
	static auto delete_time = m_globals()->m_realtime;
	static auto delete_alpha = 1.0f;

	static auto next_save = false;
	static auto prenext_save = false;
	static auto clicked_sure = false;
	static auto save_time = m_globals()->m_realtime;
	static auto save_alpha = 1.0f;

	if (should_update)
	{
		should_update = false;

		cfg_manager->config_files();
		this->configuration_files = cfg_manager->files;

		for (auto& current : this->configuration_files)
			if (current.size() > 2)
				current.erase(current.size() - 3, 3);
	}

	/* LEFT. */
	/* Configuration. */

	ImGui::SetCursorPos(ImVec2(10, 5));
	ImGui::BeginChild(crypt_str("Configuration"), ImVec2(230, 417));

	ImGui::ColorEdit(crypt_str("Theme colour"), &g_cfg.menu.menu_theme, ImGuiColorEditFlags_NoAlpha);

	ImGui::Combo(crypt_str("DPI scaling"), &g_cfg.menu.dpi_selection, dpi_scale_type, IM_ARRAYSIZE(dpi_scale_type));

	if (ImGui::CustomButton(crypt_str("Open configuration folder"), crypt_str("##CONFIG__FOLDER")))
	{
		std::string folder;

		auto get_dir = [&folder]() -> void
		{
			static TCHAR path[MAX_PATH];

			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
				folder = std::string(path) + crypt_str("\\naval\\configs\\");

			CreateDirectory(folder.c_str(), NULL);
		};

		get_dir();
		ShellExecute(NULL, crypt_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	if (ImGui::CustomButton(crypt_str("Refresh configuration"), crypt_str("##CONFIG__REFRESH")))
	{
		cfg_manager->config_files();
		this->configuration_files = cfg_manager->files;

		for (auto& current : this->configuration_files)
			if (current.size() > 2)
				current.erase(current.size() - 3, 3);
	}

	static char config_name[64] = "\0";

	ImGui::InputText(crypt_str("New configuration name"), config_name, sizeof(config_name));

	if (ImGui::CustomButton(crypt_str("Create new configuration"), crypt_str("##CONFIG__CREATE")))
	{
		g_cfg.new_config_name = config_name;
		misc::get().add_config();
	}

	ImGui::EndChild();

	/* RIGHT. */
	/* Configuration list. */

	ImGui::SetCursorPos(ImVec2(245, 5));
	ImGui::BeginChild(crypt_str("Configuration list"), ImVec2(230, 417));

	ImGui::PushItemWidth(210);
	ImGui::ListBoxConfigArray(crypt_str(""), &g_cfg.selected_config, this->configuration_files, 7);
	ImGui::PopItemWidth();

	if (ImGui::CustomButton(crypt_str("Load configuration"), crypt_str("##CONFIG__LOAD")))
		misc::get().load_config();

	save_alpha = math::clamp(save_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_save ? 1.f : -1.f)), 0.01f, 1.f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, save_alpha * this->public_alpha * 1.f);

	if (!next_save)
	{
		clicked_sure = false;

		if (prenext_save && save_alpha <= 0.01f)
			next_save = true;

		if (ImGui::CustomButton(crypt_str("Save configuration"), crypt_str("##CONFIG__SAVE")))
		{
			save_time = m_globals()->m_realtime;
			prenext_save = true;
		}
	}
	else
	{
		if (prenext_save && save_alpha <= 0.01f)
		{
			prenext_save = false;
			next_save = !clicked_sure;
		}

		if (ImGui::CustomButton(crypt_str("Are you sure?"), crypt_str("##AREYOUSURE__SAVE")))
		{
			misc::get().save_config();
			prenext_save = true;
			clicked_sure = true;
		}

		if (!clicked_sure && m_globals()->m_realtime > save_time + 1.5f)
		{
			prenext_save = true;
			clicked_sure = true;
		}
	}

	ImGui::PopStyleVar();

	delete_alpha = math::clamp(delete_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_delete ? 1.f : -1.f)), 0.01f, 1.f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, delete_alpha * this->public_alpha * 1.f);

	if (!next_delete)
	{
		clicked_sure_del = false;

		if (prenext_delete && delete_alpha <= 0.01f)
			next_delete = true;

		if (ImGui::CustomButton(crypt_str("Remove configuration"), crypt_str("##CONFIG__delete")))
		{
			delete_time = m_globals()->m_realtime;
			prenext_delete = true;
		}
	}
	else
	{
		if (prenext_delete && delete_alpha <= 0.01f)
		{
			prenext_delete = false;
			next_delete = !clicked_sure_del;
		}

		if (ImGui::CustomButton(crypt_str("Are you sure?"), crypt_str("##AREYOUSURE__delete")))
		{
			misc::get().remove_config();
			prenext_delete = true;
			clicked_sure_del = true;
		}

		if (!clicked_sure_del && m_globals()->m_realtime > delete_time + 1.5f)
		{
			prenext_delete = true;
			clicked_sure_del = true;
		}
	}

	ImGui::PopStyleVar();

	ImGui::EndChild();
}
#pragma once
#include "..\..\includes.hpp"
#include <comdef.h>

class c_menu : public singleton<c_menu> {
public:
	std::vector <std::string> configuration_files;

	float public_alpha = 0.f;

	void draw(bool is_open);
private:
	int menu_x = 0, menu_y = 0, tab_choosen = 0, rage_sub = 0, antiaim_type = 0, visual_sub = 0, visual_subsub = 0, current_profile = -1;
	bool once = false;
	IDirect3DTexture9* all_skins[36];

	// important.
	void setup_style();

	// things.
	void draw_background();
	void draw_decorations(ImDrawList* draw_list, ImVec2 pos);
	void draw_functions();
	void draw_functions_decorations(ImDrawList* draw_list, ImVec2 pos);

	/* TABS. */

	// ragebot.
	void draw_rage_main_tab();
	void draw_rage_antiaim_tab();

	// legitbot.
	void draw_legit_main_tab();

	// visual.
	void draw_visual_player_esp_tab();
	void draw_visual_player_chams_tab();
	void draw_visual_other_esp_tab();
	void draw_visual_other_chams_tab();

	// skinchanger.
	void draw_skinchanger_tab();

	// miscellaneous.
	void draw_miscellaneous_tab();

	// config.
	void draw_configuration_tab();
private:
	std::string get_weapon_name_from_id(int id, int custom_index = -1, bool knife = true)
	{
		if (custom_index > -1)
		{
			if (knife)
			{
				switch (custom_index)
				{
				case 0: return crypt_str("weapon_knife");
				case 1: return crypt_str("weapon_bayonet");
				case 2: return crypt_str("weapon_knife_css");
				case 3: return crypt_str("weapon_knife_skeleton");
				case 4: return crypt_str("weapon_knife_outdoor");
				case 5: return crypt_str("weapon_knife_cord");
				case 6: return crypt_str("weapon_knife_canis");
				case 7: return crypt_str("weapon_knife_flip");
				case 8: return crypt_str("weapon_knife_gut");
				case 9: return crypt_str("weapon_knife_karambit");
				case 10: return crypt_str("weapon_knife_m9_bayonet");
				case 11: return crypt_str("weapon_knife_tactical");
				case 12: return crypt_str("weapon_knife_falchion");
				case 13: return crypt_str("weapon_knife_survival_bowie");
				case 14: return crypt_str("weapon_knife_butterfly");
				case 15: return crypt_str("weapon_knife_push");
				case 16: return crypt_str("weapon_knife_ursus");
				case 17: return crypt_str("weapon_knife_gypsy_jackknife");
				case 18: return crypt_str("weapon_knife_stiletto");
				case 19: return crypt_str("weapon_knife_widowmaker");
				}
			}
			else
			{
				switch (custom_index)
				{
				case 0: return crypt_str("ct_gloves"); 
				case 1: return crypt_str("studded_bloodhound_gloves");
				case 2: return crypt_str("t_gloves");
				case 3: return crypt_str("ct_gloves");
				case 4: return crypt_str("sporty_gloves");
				case 5: return crypt_str("slick_gloves");
				case 6: return crypt_str("leather_handwraps");
				case 7: return crypt_str("motorcycle_gloves");
				case 8: return crypt_str("specialist_gloves");
				case 9: return crypt_str("studded_hydra_gloves");
				}
			}
		}
		else
		{
			switch (id)
			{
			case 0: return crypt_str("knife");
			case 1: return crypt_str("gloves");
			case 2: return crypt_str("weapon_ak47");
			case 3: return crypt_str("weapon_aug");
			case 4: return crypt_str("weapon_awp");
			case 5: return crypt_str("weapon_cz75a");
			case 6: return crypt_str("weapon_deagle");
			case 7: return crypt_str("weapon_elite");
			case 8: return crypt_str("weapon_famas");
			case 9: return crypt_str("weapon_fiveseven");
			case 10: return crypt_str("weapon_g3sg1");
			case 11: return crypt_str("weapon_galilar");
			case 12: return crypt_str("weapon_glock");
			case 13: return crypt_str("weapon_m249");
			case 14: return crypt_str("weapon_m4a1_silencer");
			case 15: return crypt_str("weapon_m4a1");
			case 16: return crypt_str("weapon_mac10");
			case 17: return crypt_str("weapon_mag7");
			case 18: return crypt_str("weapon_mp5sd");
			case 19: return crypt_str("weapon_mp7");
			case 20: return crypt_str("weapon_mp9");
			case 21: return crypt_str("weapon_negev");
			case 22: return crypt_str("weapon_nova");
			case 23: return crypt_str("weapon_hkp2000");
			case 24: return crypt_str("weapon_p250");
			case 25: return crypt_str("weapon_p90");
			case 26: return crypt_str("weapon_bizon");
			case 27: return crypt_str("weapon_revolver");
			case 28: return crypt_str("weapon_sawedoff");
			case 29: return crypt_str("weapon_scar20");
			case 30: return crypt_str("weapon_ssg08");
			case 31: return crypt_str("weapon_sg556");
			case 32: return crypt_str("weapon_tec9");
			case 33: return crypt_str("weapon_ump45");
			case 34: return crypt_str("weapon_usp_silencer");
			case 35: return crypt_str("weapon_xm1014");
			default: return crypt_str("unknown");
			}
		}
	}

	std::string get_rage_group()
	{
		std::string weapon_group = crypt_str("");

		switch (hooks::rage_weapon)
		{
		case 0:
			weapon_group = crypt_str("Heavy-pistols ");
			break;
		case 1:
			weapon_group = crypt_str("Pistols ");
			break;
		case 2:
			weapon_group = crypt_str("Sub-machine guns ");
			break;
		case 3:
			weapon_group = crypt_str("Rifles ");
			break;
		case 4:
			weapon_group = crypt_str("Auto-snipers ");
			break;
		case 5:
			weapon_group = crypt_str("Scouts ");
			break;
		case 6:
			weapon_group = crypt_str("AWPs ");
			break;
		default:
			weapon_group = crypt_str("");
			break;
		}

		return weapon_group;
	}

	std::string get_antiaim_group()
	{
		std::string antiaim_group = crypt_str("");

		switch (this->antiaim_type)
		{
		case 0:
			antiaim_group = crypt_str("Standing ");
			break;
		case 1:
			antiaim_group = crypt_str("Slow walk ");
			break;
		case 2:
			antiaim_group = crypt_str("Moving ");
			break;
		case 3:
			antiaim_group = crypt_str("In air ");
			break;
		default:
			antiaim_group = crypt_str("");
			break;
		}

		return antiaim_group;
	}

	std::string get_legit_group()
	{
		std::string weapon_group = crypt_str(" ");

		switch (hooks::legit_weapon)
		{
		case 0:
			weapon_group = crypt_str("Deagle ");
			break;
		case 1:
			weapon_group = crypt_str("Pistols ");
			break;
		case 2:
			weapon_group = crypt_str("Rifles ");
			break;
		case 3:
			weapon_group = crypt_str("Sub-machine guns ");
			break;
		case 4:
			weapon_group = crypt_str("Snipers ");
			break;
		case 5:
			weapon_group = crypt_str("Heavy ");
			break;
		default:
			weapon_group = crypt_str(" ");
			break;
		}

		return weapon_group;
	}

	std::string get_esp_group()
	{
		std::string esp_group = crypt_str(" ");

		switch (this->visual_sub)
		{
		case 0:
			esp_group = crypt_str("Enemy ");
			break;
		case 1:
			esp_group = crypt_str("Teammate ");
			break;
		case 2:
			esp_group = crypt_str("Local ");
			break;
		case 3:
			esp_group = crypt_str("Other ");
			break;
		default:
			esp_group = crypt_str(" ");
			break;
		}

		return esp_group;
	}
};

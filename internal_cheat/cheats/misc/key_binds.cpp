// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "key_binds.h"
#include "..\exploits\exploits.h"
#include "..\..\includes.hpp"
#include "misc.h"

void key_binds::update_key_bind(key_bind* key_bind, int key_bind_id)
{
	auto is_button_down = util::is_button_down(key_bind->key);

	switch (key_bind->mode)
	{
	case HOLD:
		switch (key_bind_id)
		{
		case DOUBLE_TAP_KEYBIND:
			key_bind->active = is_button_down;
			exploitsystem::get().double_tap_key = is_button_down;

			if (exploitsystem::get().double_tap_key && g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].key != g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].key)
			{
				g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].active = false;
				exploitsystem::get().hide_shots_key = false;
			}

			break;
		case HIDE_SHOTS_KEYBIND:
			key_bind->active = is_button_down;
			exploitsystem::get().hide_shots_key = is_button_down;

			if (exploitsystem::get().hide_shots_key && g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].key != g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].key)
			{
				g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].active = false;
				exploitsystem::get().double_tap_key = false;
			}

			break;
		case MANUAL_BACK_KEYBIND:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_BACK;
			else if (antiaim::get().manual_side == SIDE_BACK)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		case MANUAL_LEFT_KEYBIND:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_LEFT;
			else if (antiaim::get().manual_side == SIDE_LEFT)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		case MANUAL_RIGHT_KEYBIND:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_RIGHT;
			else if (antiaim::get().manual_side == SIDE_RIGHT)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		default:
			key_bind->active = is_button_down;
			break;
		}
		
		break;
	case TOGGLE:
		if (!key_bind->holding && is_button_down)
		{
			switch (key_bind_id)
			{
			case DOUBLE_TAP_KEYBIND:
				key_bind->active = !key_bind->active;
				exploitsystem::get().double_tap_key = !exploitsystem::get().double_tap_key;

				if (exploitsystem::get().double_tap_key && g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].key != g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].key)
				{
					g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].active = false;
					exploitsystem::get().hide_shots_key = false;
				}

				break;
			case HIDE_SHOTS_KEYBIND:
				key_bind->active = !key_bind->active;
				exploitsystem::get().hide_shots_key = !exploitsystem::get().hide_shots_key;

				if (exploitsystem::get().hide_shots_key && g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].key != g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].key)
				{
					g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].active = false;
					exploitsystem::get().double_tap_key = false;
				}

				break;
			case MANUAL_BACK_KEYBIND:
				if (antiaim::get().manual_side == SIDE_BACK)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_BACK;

				break;
			case MANUAL_LEFT_KEYBIND:
				if (antiaim::get().manual_side == SIDE_LEFT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_LEFT;

				break;
			case MANUAL_RIGHT_KEYBIND:
				if (antiaim::get().manual_side == SIDE_RIGHT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_RIGHT;

				break;
			default:
				key_bind->active = !key_bind->active;
				break;
			}

			key_bind->holding = true;
		}
		else if (key_bind->holding && !is_button_down)
			key_bind->holding = false;

		break;
	}
}

void key_binds::initialize_key_binds()
{
	for (auto i = 0; i < KEYBIND_MAX; i++)
	{
		g_cfg.keybinds.key[i].active = false;

		if (i == DOUBLE_TAP_KEYBIND || i >= HIDE_SHOTS_KEYBIND && i <= THIRDPERSON_KEYBIND || i == RESOLVER_OVERRIDE_KEYBIND) //-V648
			g_cfg.keybinds.key[i].mode = TOGGLE;
		else
			g_cfg.keybinds.key[i].mode = HOLD;
	}
}

void key_binds::update_key_binds()
{
	for (auto i = 0; i < KEYBIND_MAX; i++)
		update_key_bind(&g_cfg.keybinds.key[i], i);
}
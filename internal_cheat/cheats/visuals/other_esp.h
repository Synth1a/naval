#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
struct m_indicator
{
	std::string m_text;
	Color m_color;

	m_indicator(const char* text, Color color) :
		m_text(text), m_color(color)
	{

	}
	m_indicator(std::string text, Color color) :
		m_text(text), m_color(color)
	{

	}
	
};
class otheresp : public singleton< otheresp >
{
private:
	class c_dynamic_list {
	private:
		const float animtime = 0.2f;
		struct c_iterator {
			float time;
			bool in;
			std::string name;
			std::string type;
			int idx;
			bool for_spectators;
			LPDIRECT3DTEXTURE9 user_image;
		};
		std::map<uint32_t, c_iterator> elements;
	public:
		int last_size = 0;
		int last_text_size = 0;

		void add(std::string name, std::string type, int idx = -1, bool for_spectators = false, LPDIRECT3DTEXTURE9 steam_image = nullptr) {
			elements[idx] = c_iterator{ g_ctx.globals.absolute_time() , true, name, type, idx, for_spectators, steam_image };
		}
		void remove(std::string name, std::string type, int idx = -1, bool for_spectators = false, LPDIRECT3DTEXTURE9 steam_image = nullptr) {
			elements[idx] = c_iterator{ g_ctx.globals.absolute_time() , false, name, type, idx, for_spectators, steam_image };
		}
		void render(const ImVec2& pos, ImVec2 total_size, ImDrawList* drawlist) {
			int cursor = 0;

			auto easeOutQuad = [](float x) {
				return 1 - (1 - x) * (1 - x);
			};

			auto easeInQuad = [](float x) {
				return x * x;
			};

			last_text_size = 0;

			if (last_size > 0)
				drawlist->AddRectFilled(ImVec2(pos.x, pos.y + 20), ImVec2(pos.x + total_size.x, pos.y + last_size + 20), ImColor(30, 31, 32, 110), 4, ImDrawCornerFlags_Bot);

			for (auto& el : elements) {
				if (!el.second.for_spectators) // don't run this code if we are using this for spectators list.
					if (strstr(el.second.name.c_str(), crypt_str("?")) || strstr(el.second.type.c_str(), crypt_str("?")))
						continue;

				float time_difference = g_ctx.globals.absolute_time() - el.second.time;
				float current_animation = math::clamp(time_difference / animtime, 0.f, 1.f);

				if (el.second.in)
					current_animation = 1.f - current_animation;

				if (current_animation <= 0.f)
					continue;

				ImVec2 text_size = ImGui::CalcTextSize(el.second.name.c_str());
				ImVec2 text_size2 = ImVec2(0, 0);

				if (!el.second.for_spectators)
					text_size2 = ImGui::CalcTextSize(el.second.type.c_str());

				if ((text_size.x - (el.second.for_spectators ? 50 : 20)) * easeInQuad(current_animation) > last_text_size)
					last_text_size = (text_size.x - (el.second.for_spectators ? 50 : 20)) * easeInQuad(current_animation);

				IDirect3DTexture9* image = nullptr;

				// don't run this code if we are using this for spectators list.
				if (el.second.for_spectators)
				{
					image = el.second.user_image ? el.second.user_image : nullptr;

					if (image)
						drawlist->AddImage(image, ImVec2(pos.x + 7, pos.y + 19 + cursor + 3), ImVec2(pos.x + 7 + 16, pos.y + 19 + 16 + cursor + 3), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, int(255 * easeOutQuad(current_animation))));
					else
						drawlist->AddCircleFilled(ImVec2(pos.x + 7, pos.y + 19 + cursor + 10), 3, ImColor(130, 132, 170, int(255 * easeOutQuad(current_animation))), 24);
				}
				else
				{
					if (g_cfg.keybinds.key[el.second.idx].mode)
						drawlist->AddCircleFilled(ImVec2(pos.x + 7, pos.y + 19 + cursor + 10), 3, ImColor(130, 132, 170, int(255 * easeOutQuad(current_animation))), 24);
					else
						drawlist->AddCircle(ImVec2(pos.x + 7, pos.y + 19 + cursor + 10), 3, ImColor(130, 132, 170, int(255 * easeOutQuad(current_animation))), 24);
				}

				drawlist->AddText(ImVec2(pos.x + (el.second.for_spectators && image ? 26 : 15), pos.y + 22 + cursor), ImColor(220, 220, 255, int(255 * easeOutQuad(current_animation))), el.second.name.c_str());
				if (!el.second.for_spectators)
					drawlist->AddText(ImVec2(pos.x + (total_size.x - 5 - text_size2.x), pos.y + 22 + cursor), ImColor(220, 220, 255, int(255 * easeOutQuad(current_animation))), el.second.type.c_str());

				cursor += 20 * easeOutQuad(current_animation);
			}
			last_size = cursor;
		}
	};

	c_dynamic_list bind_list;
	c_dynamic_list spect_list;

	struct spectators_list
	{
		bool active_user = false;
		std::string user_name = crypt_str("?");
		LPDIRECT3DTEXTURE9 user_profile = nullptr;

		spectators_list(bool is_active = false, std::string active_name = crypt_str(""), LPDIRECT3DTEXTURE9 active_profile = nullptr)
		{
			this->active_user = is_active;
			this->user_name = active_name;
			this->user_profile = active_profile;
		}
	};

	LPDIRECT3DTEXTURE9 player_avatar[65];
	spectators_list spectator[65];

	std::string get_bind_type(int type) {
		switch (type)
		{
		case 0: return crypt_str("hold");
		case 1: return crypt_str("toggle");
		default: return crypt_str("?");
		}
	}

	std::string get_bind_name(int id) {
		switch (id)
		{
		case AUTO_FIRE_KEYBIND: return crypt_str("Trigger bot");
		case LEGIT_BOT_KEYBIND: return crypt_str("Legit bot");
		case DOUBLE_TAP_KEYBIND: return crypt_str("Double tap");
		case SAFE_POINT_KEYBIND: return crypt_str("Safe point");
		case DAMAGE_OVERRIDE_KEYBIND: return crypt_str("Damage override");
		case HIDE_SHOTS_KEYBIND: return crypt_str("Hide shot");
		case MANUAL_BACK_KEYBIND: return crypt_str("Manual back");
		case MANUAL_LEFT_KEYBIND: return crypt_str("Manual left");
		case MANUAL_RIGHT_KEYBIND: return crypt_str("Manual right");
		case FLIP_DESYNC_KEYBIND: return crypt_str("Desync inverter");
		case THIRDPERSON_KEYBIND: return crypt_str("Third person");
		case AUTO_PEEK_KEYBIND: return crypt_str("Onshot retreat");
		case EDGE_JUMP_KEYBIND: return crypt_str("Edge jump");
		case FAKE_DUCK_KEYBIND: return crypt_str("Fake duck");
		case SLOW_WALK_KEYBIND: return crypt_str("Slow walk");
		case BODY_AIM_KEYBIND: return crypt_str("Body aim");
		case RESOLVER_OVERRIDE_KEYBIND: return crypt_str("Resolver override");
		default: return crypt_str("?");
		}
	}

	void initialize_spectators(spectators_list* spect, int i);
public:
	void spectator_list();
	void keybind_list();
	void watermark();
	void penetration_reticle();
	void force_crosshair();
	void indicators();
	void draw_indicators();
	void hitmarker_paint();
	void damage_marker_paint();
	void spread_crosshair();
	void automatic_peek_indicator();
	void reset_data();

	struct Hitmarker
	{
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		Vector point = ZERO;
	} hitmarker;

	struct Damage_marker
	{
		Vector position = ZERO;
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		int damage = -1;
		int hitgroup = -1;

		void reset()
		{
			position.Zero();
			hurt_time = FLT_MIN;
			hurt_color = Color::White;
			damage = -1;
			hitgroup = -1;
		}
	} damage_marker[65];
	std::vector<m_indicator> m_indicators;
};
#pragma once
#include "..\ImGui\imgui.h"
#include "..\includes.hpp"
#include "..\utils\crypt_str.h"
#include <mutex>

class player_t;
class weapon_t;
class CUserCmd;

class ctx_t  //-V730
{
	CUserCmd* m_pcmd = nullptr;
public:
	struct Chams
	{
		IMaterial* regular;
		IMaterial* metallic;
		IMaterial* flat;
		IMaterial* pulse;
		IMaterial* crystal;
		IMaterial* glass;
		IMaterial* circuit;
		IMaterial* golden;
		IMaterial* glow;
		IMaterial* animated;
	} chams;

	struct Convars
	{
		ConVar* developer;

		ConVar* sensitivity;

		ConVar* m_pitch;
		ConVar* m_yaw;

		ConVar* r_drawmodelstatsoverlay;
		ConVar* r_3dsky;
		ConVar* r_drawspecificstaticprop;
		ConVar* r_shadows;
		ConVar* r_modelAmbientMin;
		ConVar* r_jiggle_bones;

		ConVar* con_filter_enable;
		ConVar* con_filter_text;

		ConVar* ff_damage_reduction_bullets;
		ConVar* ff_damage_bullet_penetration;

		ConVar* mat_fullbright;

		ConVar* viewmodel_fov;
		ConVar* viewmodel_offset_x;
		ConVar* viewmodel_offset_y;
		ConVar* viewmodel_offset_z;

		ConVar* fog_override;
		ConVar* fog_start;
		ConVar* fog_end;
		ConVar* fog_maxdensity;
		ConVar* fog_color;

		ConVar* mp_c4timer;
		ConVar* mp_damage_scale_ct_head;
		ConVar* mp_damage_scale_ct_body;
		ConVar* mp_damage_scale_t_head;
		ConVar* mp_damage_scale_t_body;

		ConVar* weapon_recoil_scale;
		ConVar* weapon_accuracy_nospread;
		ConVar* weapon_debug_spread_show;
		ConVar* weapon_molotov_maxdetonateslope;
		ConVar* weapon_accuracy_shotgun_spread_patterns;

		ConVar* molotov_throw_detonate_time;

		ConVar* cl_forwardspeed;
		ConVar* cl_sidespeed;
		ConVar* cl_upspeed;
		ConVar* cl_updaterate;
		ConVar* cl_interp_ratio;
		ConVar* cl_interp;
		ConVar* cl_mouseenable;
	
		ConVar* cl_csm_static_prop_shadows;
		ConVar* cl_csm_shadows;
		ConVar* cl_csm_world_shadows;
		ConVar* cl_foot_contact_shadows;
		ConVar* cl_csm_viewmodel_shadows;
		ConVar* cl_csm_rope_shadows;
		ConVar* cl_csm_sprite_shadows;

		ConVar* sv_cheats;
		ConVar* sv_minupdaterate;
		ConVar* sv_maxupdaterate;
		ConVar* sv_client_min_interp_ratio;
		ConVar* sv_client_max_interp_ratio;
		ConVar* sv_skyname;
		ConVar* sv_maxunlag;
		ConVar* sv_gravity;
		ConVar* sv_jump_impulse;
		ConVar* sv_clip_penetration_traces_to_players;

	} convars;

	struct Address
	{
		struct ReturnTo
		{
			uint64_t setup_velocity;
			uint64_t accumulate_layers;
			uint64_t get_client_model_renderable;
			uint64_t cam_think;
			uint64_t extrapolation;
			uint64_t maintain_sequence_transitions;
			uint64_t setupbones_timing;
			uint64_t loadout_allowed;
			uint64_t cbaseplayer_eyeposition_and_vectors;
			uint64_t cbaseplayer_thirdperson_yaw_vectors;
			uint64_t cbaseplayer_thirdperson_pitch_vectors;
		} return_to;

		struct Utility
		{
			uint64_t direct3ddevice9;
			uint64_t viewrenderbeams;
			uint64_t glowobjectmanager;
			uint64_t movehelper;
			uint64_t input;
			uint64_t playerresource;
			uint64_t gamerules;
			uint64_t postprocessing;
			uint64_t ccsplayerrenderablevftable;
		} utility;

		struct Hooks
		{
			uint64_t getforeignfallbackfontname;
			uint64_t setupbones;
			uint64_t doextraboneprocessing;
			uint64_t standardblendingrules;
			uint64_t updateclientsideanimations;
			uint64_t physicssimulate;
			uint64_t modifyeyeposition;
			uint64_t calcviewbob;
			uint64_t buildtransformations;
			uint64_t geteyeangles;
			uint64_t shouldskipanimationframe;
			uint64_t checkfilecrcswithserver;
			uint64_t processinterpolatedlist;
			uint64_t filesystem;
			uint64_t clmove;
			uint64_t checkforsequencechange;
			uint64_t estimateabsvelocity;
			uint64_t calcview;
			uint64_t processmovement;
			uint64_t setupaliveloop;
		} hooks;

		uint64_t allow_extrapolation;
		uint64_t smoke_count;
		uint64_t clear_notices;
		uint64_t set_abs_angles;
		uint64_t set_abs_origin;
		uint64_t set_abs_velocity;
		uint64_t lookup_bone;
		uint64_t attachment_helper;
		uint64_t has_c4;
		uint64_t invalidate_bone_cache;
		uint64_t invalidate_physics_recursive;
		uint64_t unknown_function;
		uint64_t physics_run_think;
		uint64_t sequence_activity;
		uint64_t prediction_random_seed;
		uint64_t prediction_player;
		uint64_t post_think_v_physics;
		uint64_t simulate_player_simulated_entities;
		uint64_t rank_reveal;
		uint64_t clip_ray_to_hitbox;
		uint64_t clip_trace_to_players;
		uint64_t write_user_cmd;
		uint64_t clmove_choke_clamp;
		uint64_t set_clantag;
		uint64_t find_hud_element;
		uint64_t game_hud;
		uint64_t is_breakable;
		uint64_t trace_filter_simple;
		uint64_t trace_filter_skip_two_entities;
		uint64_t load_skybox;
		uint64_t line_goes_through_smoke;
		uint64_t update_clientside_animations;
		uint64_t reset_animation_state;
		uint64_t update_animation_state;
		uint64_t create_animation_state;
		uint64_t equip;
		uint64_t enable_bone_cache_invalidation;
		uint64_t get_layer_sequence_cycle_rate;
		uint64_t key_values;
		uint64_t load_from_buffer;
		uint64_t last_command;
		uint64_t is_self_animating;
		uint64_t get_shotgun_spread;
	} addresses;

	struct Animations
	{
		bool m_update_bones = false;
		bool m_update_animations = false;

		struct Local
		{
			bool m_real_matrix_ret = false;

			matrix3x4_t m_fake_matrix[MAXSTUDIOBONES];
			matrix3x4_t m_real_matrix[MAXSTUDIOBONES];
		} local;

	} animations;

	struct Fonts
	{
		ImFont* keybind_font = nullptr;
		ImFont* generic_font = nullptr;
		ImFont* large_generic_font = nullptr;
		ImFont* weapon_icon = nullptr;
		ImFont* tab_icon = nullptr;
	} fonts;

	struct Globals  //-V730
	{		
		bool focused_on_input = false;
		bool fired_shot = false;
		bool force_send_packet = false;
		bool exploits = false;
		bool scoped = false;
		bool aimbot_working = false;
		bool revolver_working = false;
		bool slowwalking = false;
		bool change_materials = false;
		bool in_thirdperson = true;
		bool fakeducking = false;
		bool should_choke_packet = false;
		bool should_send_packet = false;
		bool bomb_timer_enable = false;
		bool backup_model = false;
		bool should_remove_smoke = false;
		bool should_update_beam_index = false;
		bool should_clear_death_notices = false;
		bool should_update_playerresource = false;
		bool should_update_gamerules = false;
		bool should_recharge = false;
		bool updating_skins = false;
		bool should_update_weather = false;
		bool in_autopeek = false;
		bool in_createmove = false;
		bool autowalling = false;
		
		int screen_width = 0;
		int screen_height = 0;
		int m_ragebot_shot_nr = 0;
		int m_out_sequence_nr = 0;
		int current_weapon = 0;
		int last_aimbot_shot = 0;
		int kills = 0;
		int should_buy = 0;
		int ticks_allowed = 0;
		int next_tickbase_shift = 0;
		int tickbase_shift = 0;
		int fixed_tickbase = 0;
		int backup_tickbase = 0;
		int ticks_choke = 0;

		float next_lby_update = 0.0f;
		float original_forwardmove = 0.0f;
		float original_sidemove = 0.0f;
		float fakeduck_view = 0.0f;
		float absolute_time() {
			return (float)(clock() / (float)1000.f);
		}

		Vector eye_pos = ZERO;
		Vector last_eye_pos = ZERO;
		Vector start_position = ZERO;
		Vector autopeek_position = ZERO;
		Vector original_viewangles = ZERO;
		Vector dormant_origin[65];
		Vector local_origin[128];

		weapon_t* weapon = nullptr;

		player_t* buildtransformations_player = nullptr;
		Vector buildtransformations_vector = ZERO;

		IClientNetworkable* m_networkable = nullptr;

		std::vector <std::string> events;

		IDirect3DDevice9* draw_device;

		ImVec4 menu_color;
	} globals;

	std::vector <int> indexes;
	std::string last_font_name;

	bool available();
	bool* send_packet = false;

	void set_command(CUserCmd* cmd) 
	{ 
		m_pcmd = cmd;
	}

	player_t* local(player_t* e = nullptr, bool initialization = false);
	CUserCmd* get_command();
};

extern ctx_t g_ctx;
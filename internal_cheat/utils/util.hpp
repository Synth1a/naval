#pragma once

#include "..\includes.hpp"
#include "..\sdk\math\Vector.hpp"
#include "..\sdk\misc\CUserCmd.hpp"
#include "crypt_str.h"

class attributableitem_t;
class entity_t;
class player_t;
class matrix3x4_t;
class IClientEntity;
class CGameTrace;
class IMaterial;
class CTraceFilter;
class c_baseplayeranimationstate;

struct datamap_t;

struct Box 
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
};

struct HPInfo
{
	int hp = -1;
	int hp_difference = 0;
	float hp_difference_time = 0.0f;
};

struct SoundInfo
{
	float last_time = FLT_MIN;
	Vector origin = ZERO;
};

namespace util
{
	uintptr_t find_pattern(const char* module_name, const char* pattern, const char* mask);
	uint64_t FindSignature(const char* szModule, const char* szSignature);
	IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device);
	IMaterial* apply_materials(int materials);
	IMaterial* create_material(bool lit, const std::string& material_data);

	void RotateMovement(CUserCmd* cmd, float yaw);
	void movement_fix(Vector& wish_angle, CUserCmd* m_pcmd);
	void create_state(c_baseplayeranimationstate* state, player_t* e);
	void update_state(c_baseplayeranimationstate* state, const Vector& angles);
	void reset_state(c_baseplayeranimationstate* state);
	void copy_command(CUserCmd* cmd, int tickbase_shift);
	void color_modulate(float color[3], IMaterial* material);
	void fn_equip(attributableitem_t* item, player_t* owner);

	bool visible(const Vector & start, const Vector & end, entity_t * entity, player_t * from);
	bool is_button_down(int code);
	bool is_breakable_entity(IClientEntity* e);
	bool get_bbox(entity_t* e, Box& box, bool player_esp);
	bool is_valid_hitgroup(int index);
	bool get_backtrack_matrix(player_t* player, matrix3x4_t* out);

	int get_hitbox_by_hitgroup(int index);
	int epoch_time();

	unsigned int find_in_datamap(datamap_t * map, const char *name);
	
	float get_interpolation();
	
	DWORD* FindHudElement(const char* szHudName);

	template <class Type>
	static Type hook_manual(uintptr_t* vftable, uint32_t index, Type fnNew) 
	{
		DWORD OldProtect;
		Type fnOld = (Type)vftable[index]; //-V108 //-V202

		VirtualProtect((void*)(vftable + index * sizeof(Type)), sizeof(Type), PAGE_EXECUTE_READWRITE, &OldProtect); //-V2001 //-V104 //-V206
		vftable[index] = (uintptr_t)fnNew; //-V108
		VirtualProtect((void*)(vftable + index * sizeof(Type)), sizeof(Type), OldProtect, &OldProtect); //-V2001 //-V104 //-V206

		return fnOld;
	}
}
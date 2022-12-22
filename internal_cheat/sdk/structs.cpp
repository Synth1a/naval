// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <array>
#include "structs.hpp"
#include "..\cheats\misc\misc.h"
#include "..\cheats\misc\logs.h"
#include "..\cheats\networking\networking.h"
#include "..\cheats\lagcompensation\local_animations.h"

datamap_t* entity_t::GetPredDescMap()
{
	typedef datamap_t* (__thiscall* Fn)(void*);
	return call_virtual< Fn >(this, g_ctx.indexes.at(0))(this);
}

bool entity_t::is_player()
{
	if (!this) //-V704
		return false;

	typedef bool(__thiscall* Fn)(void*);
	return get_virtual<Fn>(this, g_ctx.indexes.at(1))(this);
} // CRASH.

void entity_t::set_model_index(int index)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(PVOID, int);
	return get_virtual<Fn>(this, g_ctx.indexes.at(2))(this, index);
} // CRASH.

void entity_t::set_abs_angles(const Vector& angle)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(g_ctx.addresses.set_abs_angles);

	return fn(this, angle);
}

void entity_t::set_abs_origin(const Vector& origin)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(g_ctx.addresses.set_abs_origin);

	return fn(this, origin);
}

weapon_info_t* weapon_t::get_csweapon_info()
{
	if (!this) //-V704
		return nullptr;

	using Fn = weapon_info_t * (__thiscall*)(void*);
	return call_virtual<Fn>(this, g_ctx.indexes.at(3))(this);
}

float weapon_t::get_inaccuracy()
{
	if (!this) //-V704
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, g_ctx.indexes.at(4))(this);
}

float weapon_t::get_spread()
{
	if (!this) //-V704
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, g_ctx.indexes.at(5))(this);
}

void weapon_t::update_accuracy_penality()
{
	if (!this) //-V704
		return;

	call_virtual<void(__thiscall*)(void*)>(this, g_ctx.indexes.at(6))(this);
}

bool weapon_t::is_empty()
{
	if (!this) //-V704
		return true;

	return m_iClip1() <= 0;
}

bool weapon_t::can_fire(bool check_revolver)
{
	if (!this) //-V704
		return false;

	if (!is_non_aim() && is_empty())
		return false;

	auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(m_hOwnerEntity());

	if (owner == g_ctx.local() && antiaim::get().freeze_check)
		return false;

	if (!owner->valid(false))
		return false;

	if (owner->m_bIsDefusing())
		return false;

	auto server_time = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);

	if (server_time < this->m_flNextPrimaryAttack())
		return false;

	if (server_time < owner->m_flNextAttack())
		return false;

	if (check_revolver && this->m_iItemDefinitionIndex() == WEAPON_REVOLVER && this->m_flPostponeFireReadyTime() >= server_time)
		return false;

	return true;
}

int weapon_t::get_weapon_group(bool rage)
{
	if (!this) //-V704
		return -1;

	if (rage)
	{
		if (m_iItemDefinitionIndex() == WEAPON_REVOLVER || m_iItemDefinitionIndex() == WEAPON_DEAGLE)
			return 0;
		else if (is_pistol())
			return 1;
		else if (is_smg())
			return 2;
		else if (is_rifle())
			return 3;
		else if (m_iItemDefinitionIndex() == WEAPON_SCAR20 || m_iItemDefinitionIndex() == WEAPON_G3SG1)
			return 4;
		else if (m_iItemDefinitionIndex() == WEAPON_SSG08)
			return 5;
		else if (m_iItemDefinitionIndex() == WEAPON_AWP)
			return 6;
		else if (is_shotgun())
			return 7;
	}
	else
	{
		if (m_iItemDefinitionIndex() == WEAPON_DEAGLE)
			return 0;
		if (is_pistol())
			return 1;
		else if (is_rifle())
			return 2;
		else if (is_smg())
			return 3;
		else if (is_sniper())
			return 4;
		else if (is_shotgun())
			return 5;
	}

	return -1;
}

bool weapon_t::is_rifle()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AK47 || weapon_id == WEAPON_M4A1 || weapon_id == WEAPON_M4A1_SILENCER || weapon_id == WEAPON_GALILAR ||
		weapon_id == WEAPON_FAMAS || weapon_id == WEAPON_AUG || weapon_id == WEAPON_SG553;
}

bool weapon_t::is_smg()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_MAC10 || weapon_id == WEAPON_MP7 || weapon_id == WEAPON_MP9 || weapon_id == WEAPON_P90 ||
		weapon_id == WEAPON_BIZON || weapon_id == WEAPON_UMP45 || weapon_id == WEAPON_MP5SD;
}

bool weapon_t::is_shotgun()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_XM1014 || weapon_id == WEAPON_NOVA || weapon_id == WEAPON_SAWEDOFF || weapon_id == WEAPON_MAG7 || weapon_id == WEAPON_M249 || weapon_id == WEAPON_NEGEV;
}

bool weapon_t::is_pistol()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_DEAGLE || weapon_id == WEAPON_ELITE || weapon_id == WEAPON_FIVESEVEN || weapon_id == WEAPON_P250 ||
		weapon_id == WEAPON_GLOCK || weapon_id == WEAPON_HKP2000 || weapon_id == WEAPON_CZ75A || weapon_id == WEAPON_USP_SILENCER || weapon_id == WEAPON_TEC9 || weapon_id == WEAPON_REVOLVER;
}

bool weapon_t::is_sniper()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AWP || weapon_id == WEAPON_SCAR20 || weapon_id == WEAPON_G3SG1 || weapon_id == WEAPON_SSG08;
}

bool weapon_t::is_grenade()
{
	if (!this) //-V704
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE || idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV || idx == WEAPON_DECOY || idx == WEAPON_INCGRENADE;
}

bool weapon_t::is_knife()
{
	if (!this) //-V704
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
		|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET
		|| idx == WEAPON_KNIFE_PUSH || idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL
		|| idx == WEAPON_KNIFEGG || idx == WEAPON_KNIFE_GHOST || idx == WEAPON_KNIFE_GYPSY_JACKKNIFE || idx == WEAPON_KNIFE_STILETTO
		|| idx == WEAPON_KNIFE_URSUS || idx == WEAPON_KNIFE_WIDOWMAKER || idx == WEAPON_KNIFE_CSS || idx == WEAPON_KNIFE_CANIS
		|| idx == WEAPON_KNIFE_CORD || idx == WEAPON_KNIFE_OUTDOOR || idx == WEAPON_KNIFE_SKELETON;
}

bool weapon_t::is_scopable()
{
	if (!this)
		return false;

	auto idx = m_iItemDefinitionIndex();

	return idx == WEAPON_SCAR20 || idx == WEAPON_G3SG1 || idx == WEAPON_SSG08 || idx == WEAPON_AWP || idx == WEAPON_AUG || idx == WEAPON_SG553;
}

using GetShotgunSpread_t = void(__stdcall*)(int, int, int, float*, float*);

Vector weapon_t::calculate_spread(int seed, float inaccuracy, float spread, bool revolver2) {
	weapon_info_t* wep_info;
	int        item_def_index;
	float      recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

	// if we have no bullets, we have no spread.
	wep_info = get_csweapon_info();
	if (!wep_info || !wep_info->iBullets)
		return ZERO;

	// get some data for later.
	item_def_index = m_iItemDefinitionIndex();
	recoil_index = m_flRecoilIndex();

	// generate needed floats.
	r1 = std::get<0>(networking::get().computed_seeds[seed]);
	r2 = std::get<1>(networking::get().computed_seeds[seed]);

	if (g_ctx.convars.weapon_accuracy_shotgun_spread_patterns->GetInt() > 0)
		((GetShotgunSpread_t)g_ctx.addresses.get_shotgun_spread)(item_def_index, 0, 0 + wep_info->iBullets * recoil_index, &r4, &r3);
	else {
		r3 = std::get<0>(networking::get().computed_seeds[seed]);
		r4 = std::get<1>(networking::get().computed_seeds[seed]);
	}

	// revolver secondary spread.
	if (item_def_index == WEAPON_REVOLVER && revolver2) {
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}

	// negev spread.
	else if (item_def_index == WEAPON_NEGEV && recoil_index < 3.f) {
		for (int i = 3; i > recoil_index; --i) {
			r1 *= r1;
			r3 *= r3;
		}

		r1 = 1.f - r1;
		r3 = 1.f - r3;
	}

	// get needed sine / cosine values.
	c1 = std::cos(r2);
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

	// calculate spread vector.
	return {
		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
		0.f
	};
}

bool weapon_t::is_non_aim(bool disable_knife)
{
	if (!this) //-V704
		return true;

	auto idx = m_iItemDefinitionIndex();

	if (idx == WEAPON_C4 || idx == WEAPON_HEALTHSHOT)
		return true;

	if (disable_knife)
		if (is_knife())
			return true;

	if (is_grenade())
		return true;

	return false;
}

bool weapon_t::can_double_tap()
{
	if (!this) //-V704
		return false;

	if (is_non_aim())
		return false;

	auto idx = m_iItemDefinitionIndex();

	if (idx == WEAPON_TASER || idx == WEAPON_REVOLVER)
		return false;

	return true;
}

int weapon_t::get_max_tickbase_shift()
{
	auto idx = m_iItemDefinitionIndex();
	auto max_tickbase_shift = 0;

	switch (idx)
	{
	case WEAPON_M249:
	case WEAPON_MAC10:
	case WEAPON_P90:
	case WEAPON_MP5SD:
	case WEAPON_NEGEV:
	case WEAPON_MP9:
		max_tickbase_shift = 5;
		break;
	case WEAPON_ELITE:
	case WEAPON_UMP45:
	case WEAPON_BIZON:
	case WEAPON_TEC9:
	case WEAPON_MP7:
		max_tickbase_shift = 6;
		break;
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_CZ75A:
		max_tickbase_shift = 7;
		break;
	case WEAPON_FIVESEVEN:
	case WEAPON_GLOCK:
	case WEAPON_P250:
	case WEAPON_SG553:
		max_tickbase_shift = 8;
		break;
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		max_tickbase_shift = 9;
		break;
	case WEAPON_DEAGLE:
	case WEAPON_G3SG1:
	case WEAPON_SCAR20:
		max_tickbase_shift = 13;
		break;
	default:
		max_tickbase_shift = m_gamerules()->m_bIsValveDS() ? 6 : 13;
		break;
	}

	if (m_gamerules()->m_bIsValveDS())
		max_tickbase_shift = min(max_tickbase_shift, 6);

	return max_tickbase_shift;
}

const char* weapon_t::get_icon()
{
	if (!this) //-V704
		return crypt_arr(" ");

	switch (m_iItemDefinitionIndex()) {
	case WEAPON_KNIFE_BAYONET:
		return crypt_arr("1");
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return crypt_arr("7");
	case WEAPON_KNIFE_BUTTERFLY:
		return crypt_arr("8");
	case WEAPON_KNIFE:
		return crypt_arr("]");
	case WEAPON_KNIFE_FALCHION:
		return crypt_arr("0");
	case WEAPON_KNIFE_FLIP:
		return crypt_arr("2");
	case WEAPON_KNIFE_GUT:
		return crypt_arr("3");
	case WEAPON_KNIFE_KARAMBIT:
		return crypt_arr("4");
	case WEAPON_KNIFE_M9_BAYONET:
		return crypt_arr("5");
	case WEAPON_KNIFE_T:
		return crypt_arr("[");
	case WEAPON_KNIFE_TACTICAL:
		return crypt_arr("6");
	case WEAPON_KNIFE_PUSH:
		return crypt_arr("]");
	case WEAPON_DEAGLE:
		return crypt_arr("A");
	case WEAPON_ELITE:
		return crypt_arr("B");
	case WEAPON_FIVESEVEN:
		return crypt_arr("C");
	case WEAPON_GLOCK:
		return crypt_arr("D");
	case WEAPON_HKP2000:
		return crypt_arr("E");
	case WEAPON_P250:
		return crypt_arr("F");
	case WEAPON_USP_SILENCER:
		return crypt_arr("G");
	case WEAPON_TEC9:
		return crypt_arr("H");
	case WEAPON_REVOLVER:
		return crypt_arr("J");
	case WEAPON_MAC10:
		return crypt_arr("K");
	case WEAPON_UMP45:
		return crypt_arr("L");
	case WEAPON_BIZON:
		return crypt_arr("M");
	case WEAPON_MP7:
		return crypt_arr("N");
	case WEAPON_MP9:
		return crypt_arr("O");
	case WEAPON_P90:
		return crypt_arr("P");
	case WEAPON_GALILAR:
		return crypt_arr("Q");
	case WEAPON_FAMAS:
		return crypt_arr("R");
	case WEAPON_M4A1_SILENCER:
		return crypt_arr("S");
	case WEAPON_M4A1:
		return crypt_arr("T");
	case WEAPON_AUG:
		return crypt_arr("U");
	case WEAPON_SG553:
		return crypt_arr("V");
	case WEAPON_AK47:
		return crypt_arr("W");
	case WEAPON_G3SG1:
		return crypt_arr("X");
	case WEAPON_SCAR20:
		return crypt_arr("Y");
	case WEAPON_AWP:
		return crypt_arr("Z");
	case WEAPON_SSG08:
		return crypt_arr("a");
	case WEAPON_XM1014:
		return crypt_arr("b");
	case WEAPON_SAWEDOFF:
		return crypt_arr("c");
	case WEAPON_MAG7:
		return crypt_arr("d");
	case WEAPON_NOVA:
		return crypt_arr("e");
	case WEAPON_NEGEV:
		return crypt_arr("f");
	case WEAPON_M249:
		return crypt_arr("g");
	case WEAPON_TASER:
		return crypt_arr("h");
	case WEAPON_FLASHBANG:
		return crypt_arr("i");
	case WEAPON_HEGRENADE:
		return crypt_arr("j");
	case WEAPON_SMOKEGRENADE:
		return crypt_arr("k");
	case WEAPON_MOLOTOV:
		return crypt_arr("l");
	case WEAPON_DECOY:
		return crypt_arr("m");
	case WEAPON_INCGRENADE:
		return crypt_arr("n");
	case WEAPON_C4:
		return crypt_arr("o");
	case WEAPON_CZ75A:
		return crypt_arr("I");
	default:
		return crypt_arr(" ");
	}
}

std::string weapon_t::get_name()
{
	if (!this) //-V704
		return " ";

	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE:
		return "KNIFE";
	case WEAPON_KNIFE_T:
		return "KNIFE";
	case WEAPON_KNIFE_BAYONET:
		return "KNIFE";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "KNIFE";
	case WEAPON_KNIFE_BUTTERFLY:
		return "KNIFE";
	case WEAPON_KNIFE_FALCHION:
		return "KNIFE";
	case WEAPON_KNIFE_FLIP:
		return "KNIFE";
	case WEAPON_KNIFE_GUT:
		return "KNIFE";
	case WEAPON_KNIFE_KARAMBIT:
		return "KNIFE";
	case WEAPON_KNIFE_M9_BAYONET:
		return "KNIFE";
	case WEAPON_KNIFE_TACTICAL:
		return "KNIFE";
	case WEAPON_KNIFE_PUSH:
		return "KNIFE";
	case WEAPON_DEAGLE:
		return "DEAGLE";
	case WEAPON_ELITE:
		return "DUAL BERETTAS";
	case WEAPON_FIVESEVEN:
		return "FIVE-SEVEN";
	case WEAPON_GLOCK:
		return "GLOCK 18";
	case WEAPON_HKP2000:
		return "P2000";
	case WEAPON_P250:
		return "P250";
	case WEAPON_USP_SILENCER:
		return "USP-S";
	case WEAPON_TEC9:
		return "TEC-9";
	case WEAPON_REVOLVER:
		return "REVOLVER";
	case WEAPON_MAC10:
		return "MAC-10";
	case WEAPON_UMP45:
		return "UMP-45";
	case WEAPON_BIZON:
		return "PP-BIZON";
	case WEAPON_MP7:
		return "MP7";
	case WEAPON_MP9:
		return "MP9";
	case WEAPON_P90:
		return "P90";
	case WEAPON_GALILAR:
		return "GALIL AR";
	case WEAPON_FAMAS:
		return "FAMAS";
	case WEAPON_M4A1_SILENCER:
		return "M4A1-S";
	case WEAPON_M4A1:
		return "M4A4";
	case WEAPON_AUG:
		return "AUG";
	case WEAPON_SG553:
		return "SG 553";
	case WEAPON_AK47:
		return "AK-47";
	case WEAPON_G3SG1:
		return "G3SG1";
	case WEAPON_SCAR20:
		return "SCAR-20";
	case WEAPON_AWP:
		return "AWP";
	case WEAPON_SSG08:
		return "SSG 08";
	case WEAPON_XM1014:
		return "XM1014";
	case WEAPON_SAWEDOFF:
		return "SAWED-OFF";
	case WEAPON_MAG7:
		return "MAG-7";
	case WEAPON_NOVA:
		return "NOVA";
	case WEAPON_NEGEV:
		return "NEGEV";
	case WEAPON_M249:
		return "M249";
	case WEAPON_TASER:
		return "ZEUS X27";
	case WEAPON_FLASHBANG:
		return "FLASHBANG";
	case WEAPON_HEGRENADE:
		return "HE GRENADE";
	case WEAPON_SMOKEGRENADE:
		return "SMOKE";
	case WEAPON_MOLOTOV:
		return "MOLOTOV";
	case WEAPON_DECOY:
		return "DECOY";
	case WEAPON_INCGRENADE:
		return "INCENDIARY";
	case WEAPON_C4:
		return "C4";
	case WEAPON_CZ75A:
		return "CZ75-AUTO";
	default:
		return " ";
	}
}

Vector player_t::world_space_center()
{
	if (!this)
		return ZERO;

	const auto collideable = this->GetCollideable();
	if (!collideable)
		return ZERO;

	const auto origin = this->m_vecOrigin();

	auto mins = this->m_vecMins() + origin;
	auto maxs = this->m_vecMaxs() + origin;

	auto size = maxs - mins;
	size /= 2.0f;
	size += mins;

	return size;
}

std::array <float, 24>& entity_t::m_flPoseParameter()
{
	static auto _m_flPoseParameter = netvars::get().get_offset(crypt_str("CCSPlayer"), crypt_str("m_flPoseParameter"));
	return *(std::array <float, 24>*)((uintptr_t)this + _m_flPoseParameter);
}

std::array<Vector, 5>& player_t::m_vecPlayerPatchEconIndices()
{
	static int _m_vecPlayerPatchEconIndices = netvars::get().get_offset(crypt_str("CCSPlayer"), crypt_str("m_vecPlayerPatchEconIndices"));
	return *(std::array<Vector, 5>*)((uintptr_t)this + _m_vecPlayerPatchEconIndices);
}

int& player_t::m_nComputedLODframe()
{
	static auto m_flFlashDuration = netvars::get().get_offset(crypt_str("CCSPlayer"), crypt_str("m_flFlashDuration"));
	return *(int*)(uintptr_t(this) + m_flFlashDuration - 0x10);
}

int& player_t::m_nFinalPredictedTick() {
	return *(int*)((uintptr_t)(this) + 0x3444);
};

Vector player_t::m_aimPunchAngleScaled()
{
	if (!this)
		return ZERO;

	return m_aimPunchAngle() * g_ctx.convars.weapon_recoil_scale->GetFloat();
}

int player_t::lookup_bone(const char* szName)
{
	if (!this)
		return -1;

	static auto fn = reinterpret_cast <int(__thiscall*)(void*, const char*)> (g_ctx.addresses.lookup_bone);
	return fn(this, szName);
}

Vector player_t::get_shoot_position(bool interpolated)
{
	auto result = ZERO;

	if (!this)
		return result;

	if (!interpolated)
		result = m_vecOrigin() + m_vecViewOffset();
	else
		call_virtual<Vector& (__thiscall*)(void*, Vector*)>(this, g_ctx.indexes.at(10))(this, &result);

	return result;
}

void player_t::attachment_helper()
{
	if (!this)
		return;

	using fn = void(__thiscall*)(player_t*, c_studio_hdr*);
	static fn attachement_helper = (fn)g_ctx.addresses.attachment_helper;
	attachement_helper(this, this->m_pStudioHdr());
}

const matrix3x4_t& entity_t::m_rgflCoordinateFrame()
{
	static auto _m_rgflCoordinateFrame = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_CollisionGroup")) - 0x30;
	return *(matrix3x4_t*)((uintptr_t)this + _m_rgflCoordinateFrame);
}

bool player_t::setup_bones(matrix3x4_t* pBoneToWorldOut, bool safe_matrix)
{
	if (!this)
		return false;

	AnimationLayer backup_layers[13];

	const float flCurTime = m_globals()->m_curtime;
	const float flRealTime = m_globals()->m_realtime;
	const float flFrameTime = m_globals()->m_frametime;
	const float flAbsFrameTime = m_globals()->m_absoluteframetime;
	const int iFrameCount = m_globals()->m_framecount;
	const int iTickCount = m_globals()->m_tickcount;
	const float flInterpolation = m_globals()->m_interpolation_amount;

	m_globals()->m_curtime = m_globals()->m_realtime = this->m_flSimulationTime();
	m_globals()->m_frametime = m_globals()->m_absoluteframetime = m_globals()->m_intervalpertick;
	m_globals()->m_framecount = INT_MAX;
	m_globals()->m_tickcount = TIME_TO_TICKS(m_globals()->m_realtime);
	m_globals()->m_interpolation_amount = 0.f;

	const uint32_t iClientEffects = this->m_nClientEffects();
	const uint32_t iOcclusionFramecount = this->m_iOcclusionFramecount();
	const uint32_t iOcclusionFlags = this->m_iOcclusionFlags();
	const uint32_t nLastSkipFramecount = this->m_nLastSkipFramecount();
	const uint32_t iEffects = this->m_fEffects();
	const bool bMaintainSequenceTransition = this->m_bMaintainSequenceTransition();
	const Vector vecAbsOrigin = this->GetAbsOrigin();

	int iMask = BONE_USED_BY_ANYTHING;
	if (safe_matrix)
		iMask = BONE_USED_BY_HITBOX;

	this->copy_animlayers(backup_layers);
	this->invalidate_bone_cache();
	this->m_BoneAccessor()->m_ReadableBones = this->m_BoneAccessor()->m_WritableBones = NULL;

	if (this->get_animation_state())
		this->get_animation_state()->m_pWeaponLast = this->get_animation_state()->m_pWeapon;

	this->m_iOcclusionFramecount() = this->m_iOcclusionFlags() = this->m_nLastSkipFramecount() = NULL;

	if (this != g_ctx.local())
		this->set_abs_origin(this->m_vecOrigin());

	this->m_fEffects() |= 8;
	this->m_nClientEffects() |= 2;
	this->m_bMaintainSequenceTransition() = false;

	this->get_animlayers()[ANIMATION_LAYER_LEAN].m_flWeight = 0.0f;
	if (safe_matrix)
		this->get_animlayers()[ANIMATION_LAYER_ADJUST].m_pOwner = NULL;
	else if (this == g_ctx.local())
	{
		if (this->sequence_activity(this->get_animlayers()[ANIMATION_LAYER_ADJUST].m_nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST)
		{
			this->get_animlayers()[ANIMATION_LAYER_ADJUST].m_flCycle = 0.0f;
			this->get_animlayers()[ANIMATION_LAYER_ADJUST].m_flWeight = 0.0f;
		}
	}

	g_ctx.animations.m_update_bones = true;
	auto res = this->SetupBones(pBoneToWorldOut, MAXSTUDIOBONES, iMask, 0.f);
	g_ctx.animations.m_update_bones = false;

	this->m_bMaintainSequenceTransition() = bMaintainSequenceTransition;
	this->m_nClientEffects() = iClientEffects;
	this->m_fEffects() = iEffects;
	this->m_nLastSkipFramecount() = nLastSkipFramecount;
	this->m_iOcclusionFlags() = iOcclusionFlags;
	this->m_iOcclusionFramecount() = iOcclusionFramecount;

	if (this != g_ctx.local())
		this->set_abs_origin(vecAbsOrigin);

	this->set_animlayers(backup_layers);

	m_globals()->m_curtime = flCurTime;
	m_globals()->m_realtime = flRealTime;
	m_globals()->m_frametime = flFrameTime;
	m_globals()->m_absoluteframetime = flAbsFrameTime;
	m_globals()->m_framecount = iFrameCount;
	m_globals()->m_tickcount = iTickCount;
	m_globals()->m_interpolation_amount = flInterpolation;

	return res;
}

bool player_t::is_alive()
{
	if (!this) //-V704
		return false;

	if (m_iTeamNum() != 2 && m_iTeamNum() != 3)
		return false;

	if (m_lifeState() != LIFE_ALIVE)
		return false;

	return true;
}

bool player_t::has_c4()
{
	if (!this)
		return false;

	static auto fn = reinterpret_cast <bool(__thiscall*)(void*)> (g_ctx.addresses.has_c4);
	return fn(this);
}

int	player_t::get_move_type()
{
	if (!this) //-V704
		return 0;

	return *(int*)((uintptr_t)this + 0x25C);
}

int player_t::get_hitbox_bone_id(int hitbox_id)
{
	if (!this) //-V704
		return -1;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return -1;

	auto hitbox_set = hdr->pHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return -1;

	auto hitbox = hitbox_set->pHitbox(hitbox_id);

	if (!hitbox)
		return -1;

	return hitbox->bone;
}

Vector player_t::hitbox_position(int hitbox_id)
{
	if (!this) //-V704
		return ZERO;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return ZERO;

	auto hitbox_set = hdr->pHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return ZERO;

	auto hitbox = hitbox_set->pHitbox(hitbox_id);

	if (!hitbox)
		return ZERO;

	if (!m_CachedBoneData().Base())
		return ZERO;

	Vector min, max;

	math::vector_transform(hitbox->bbmin, m_CachedBoneData().Base()[hitbox->bone], min);
	math::vector_transform(hitbox->bbmax, m_CachedBoneData().Base()[hitbox->bone], max);

	return (min + max) * 0.5f;
}

Vector player_t::hitbox_position_matrix(int hitbox_id, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	// this thing crashed because the matrix is empty ( wtf? ).
	if (!this)
		return ZERO;

	const model_t* model = GetModel();
	if (!model)
		return ZERO;

	studiohdr_t* hdr = m_modelinfo()->GetStudioModel(model);
	if (!hdr)
		return ZERO;

	mstudiohitboxset_t* hitbox_set = hdr->pHitboxSet(m_nHitboxSet());
	if (!hitbox_set)
		return ZERO;

	mstudiobbox_t* hitbox = hitbox_set->pHitbox(hitbox_id);
	if (!hitbox)
		return ZERO;

	Vector min, max;
	math::vector_transform(hitbox->bbmin, matrix[hitbox->bone], min);
	math::vector_transform(hitbox->bbmax, matrix[hitbox->bone], max);

	return (min + max) * 0.5f;
}

CUtlVector <matrix3x4_t>& player_t::m_CachedBoneData()
{
	return *(CUtlVector<matrix3x4_t>*)(uintptr_t(this) + 0x2914);
}

CBoneAccessor* player_t::m_BoneAccessor()
{
	static auto m_nForceBone = netvars::get().get_offset(crypt_str("CBaseAnimating"), crypt_str("m_nForceBone"));
	static auto BoneAccessor = m_nForceBone + 0x1C;

	return (CBoneAccessor*)((uintptr_t)this + BoneAccessor);
}

void player_t::invalidate_bone_cache()
{
	if (!this) //-V704
		return;

	m_flLastBoneSetupTime() = -FLT_MAX;
	m_iMostRecentModelBoneCounter() = UINT_MAX;
}

void player_t::set_abs_velocity(const Vector& velocity)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(g_ctx.addresses.set_abs_velocity);

	return fn(this, velocity);
}

Vector& player_t::get_render_angles()
{
	if (!this) //-V704
		return ZERO;

	static auto deadflag = netvars::get().get_offset(crypt_str("CBasePlayer"), crypt_str("deadflag"));
	return *(Vector*)(uintptr_t(this) + (deadflag + 0x4));
}

void player_t::set_render_angles(const Vector& angles)
{
	if (!this) //-V704
		return;

	static auto deadflag = netvars::get().get_offset(crypt_str("CBasePlayer"), crypt_str("deadflag"));
	*(Vector*)(uintptr_t(this) + (deadflag + 0x4)) = angles;
}

uint32_t& player_t::m_iMostRecentModelBoneCounter()
{
	static auto most_recent_model_bone_counter = *(uintptr_t*)(g_ctx.addresses.invalidate_bone_cache + 0x1B);

	return *(uint32_t*)((uintptr_t)this + most_recent_model_bone_counter);
}

float& player_t::m_flLastBoneSetupTime()
{
	static auto last_bone_setup_time = *(uintptr_t*)(g_ctx.addresses.invalidate_bone_cache + 0x11);
	return *(float*)((uintptr_t)this + last_bone_setup_time);
}

int* player_t::m_nImpulse()
{
	static std::uintptr_t m_nImpulse = util::find_in_datamap(GetPredDescMap(), crypt_str("m_nImpulse"));
	return (int*)((std::uintptr_t)this + m_nImpulse);
}

int* player_t::m_nButtons()
{
	static std::uintptr_t m_nButtons = util::find_in_datamap(GetPredDescMap(), crypt_str("m_nButtons"));
	return (int*)((std::uintptr_t)this + m_nButtons);
}

int& player_t::m_afButtonLast()
{
	static std::uintptr_t m_afButtonLast = util::find_in_datamap(GetPredDescMap(), crypt_str("m_afButtonLast"));
	return *(int*)((std::uintptr_t)this + m_afButtonLast);
}

int& player_t::m_afButtonPressed()
{
	static std::uintptr_t m_afButtonPressed = util::find_in_datamap(GetPredDescMap(), crypt_str("m_afButtonPressed"));
	return *(int*)((std::uintptr_t)this + m_afButtonPressed);
}

int& player_t::m_afButtonReleased()
{
	static std::uintptr_t m_afButtonReleased = util::find_in_datamap(GetPredDescMap(), crypt_str("m_afButtonReleased"));
	return *(int*)((std::uintptr_t)this + m_afButtonReleased);
}

int* player_t::m_nNextThinkTick()
{
	static int m_nNextThinkTick = netvars::get().get_offset(crypt_str("CBasePlayer"), crypt_str("m_nNextThinkTick"));
	return (int*)((uintptr_t)this + m_nNextThinkTick);
}

void player_t::unknown_function()
{
	if (!this)
		return;

	static auto fn = reinterpret_cast<void(__thiscall*)(int)>(g_ctx.addresses.unknown_function);
	fn(0);
}

bool player_t::physics_run_think(int index)
{
	if (!this)
		return false;

	static auto physics_run_think_fn = reinterpret_cast <bool(__thiscall*)(void*, int)> (g_ctx.addresses.physics_run_think);
	return physics_run_think_fn(this, index);
}

bool player_t::is_bot()
{
	if (!this)
		return false;

	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(EntIndex(), &player_info))
		return false;

	return player_info.fakeplayer;
}

VarMapping_t* player_t::var_mapping()
{
	return reinterpret_cast<VarMapping_t*>((DWORD)this + 0x24);
}

bool player_t::valid(bool check_team, bool check_dormant)
{
	if (!this) //-V704
		return false;

	if (!g_ctx.local())
		return false;

	if (!is_alive())
		return false;

	if (!is_player())
		return false;

	if (IsDormant() && check_dormant)
		return false;

	if (check_team && g_ctx.local()->m_iTeamNum() == m_iTeamNum())
		return false;

	return true;
}

int player_t::animlayer_count()
{
	if (!this) //-V704
		return 0;

	return *(int*)((DWORD)this + 0x299C);
}

AnimationLayer* player_t::get_animlayers()
{
	return *(AnimationLayer**)((DWORD)this + 0x2990);
}

int player_t::sequence_activity(int sequence)
{
	if (!this)
		return -1;

	auto model = this->GetModel();

	if (!model)
		return -1;

	auto hdr = m_modelinfo()->GetStudioModel(model);

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(g_ctx.addresses.sequence_activity);
	return get_sequence_activity(this, hdr, sequence);
}

c_baseplayeranimationstate* player_t::get_animation_state()
{
	return *reinterpret_cast<c_baseplayeranimationstate**>(reinterpret_cast<void*>(uintptr_t(this) + 0x9960));
}

CUserCmd*& player_t::m_pCurrentCommand()
{
	static auto m_hConstraintEntity = netvars::get().get_offset(crypt_str("CBasePlayer"), crypt_str("m_hViewEntity"));
	return *reinterpret_cast<CUserCmd**>(uintptr_t(this) + m_hConstraintEntity - 0x4);
}

CUserCmd& player_t::m_LastCmd()
{
	static const uintptr_t uLastCommandOffset = *reinterpret_cast<uintptr_t*>(g_ctx.addresses.last_command);
	return *reinterpret_cast<CUserCmd*>(reinterpret_cast<uintptr_t>(this) + uLastCommandOffset);
}

c_studio_hdr* player_t::m_pStudioHdr()
{
	if (!this)
		return nullptr;

	return *(c_studio_hdr**)((uintptr_t)this + 0x2950);
}

int& weapon_t::m_Activity()
{
	static auto m_iActivity = util::find_in_datamap(GetPredDescMap(), crypt_str("m_Activity"));
	return *(int*)(uintptr_t(this) + m_iActivity);
}

void player_t::update_clientside_animation()
{
	if (!this)
		return;

	return reinterpret_cast <void(__thiscall*)(void*)> (g_ctx.addresses.update_clientside_animations)(this);
}

uint32_t& player_t::m_fEffects()
{
	static auto m_fEffects = util::find_in_datamap(GetPredDescMap(), crypt_str("m_fEffects"));
	return *(uint32_t*)(uintptr_t(this) + m_fEffects);
}

uint32_t& player_t::m_iEFlags()
{
	static auto m_iEFlags = util::find_in_datamap(GetPredDescMap(), crypt_str("m_iEFlags"));
	return *(uint32_t*)(uintptr_t(this) + m_iEFlags);
}

Vector& player_t::m_vecBaseVelocity()
{
	static auto m_vecBaseVelocity = util::find_in_datamap(GetPredDescMap(), crypt_str("m_vecBaseVelocity"));
	return *(Vector*)(uintptr_t(this) + m_vecBaseVelocity);
}

float player_t::get_max_player_speed()
{
	auto weapon = (this == g_ctx.local() ? g_ctx.globals.weapon : m_hActiveWeapon().Get());
	auto is_scoped = (this == g_ctx.local() ? g_ctx.globals.scoped : m_bIsScoped());

	if (weapon)
	{
		auto weapon_data = weapon->get_csweapon_info();
		if (weapon_data)
			return is_scoped ? weapon_data->flMaxPlayerSpeedAlt : weapon_data->flMaxPlayerSpeed;
	}

	return 260.0f;
}

float& player_t::m_surfaceFriction()
{
	static auto m_surfaceFriction = util::find_in_datamap(GetPredDescMap(), crypt_str("m_surfaceFriction"));
	return *(float*)(uintptr_t(this) + m_surfaceFriction);
}

Vector& player_t::m_vecAbsVelocity()
{
	if (!this) //-V704
		return ZERO;

	static auto m_vecAbsVelocity = util::find_in_datamap(GetPredDescMap(), crypt_str("m_vecAbsVelocity"));
	return *(Vector*)(uintptr_t(this) + m_vecAbsVelocity);
}

float player_t::get_max_desync_delta()
{
	if (!this) //-V704
		return 0.0f;

	auto animstate = get_animation_state();

	if (!animstate)
		return 0.0f;

	auto speedfactor = math::clamp(animstate->m_flSpeedAsPortionOfWalkTopSpeed, 0.0f, 1.0f);
	auto avg_speedfactor = (animstate->m_flWalkToRunTransition * -0.3f - 0.2f) * speedfactor + 1.0f;

	auto duck_amount = animstate->m_flAnimDuckAmount;

	if (duck_amount) //-V550
	{
		auto max_velocity = math::clamp(animstate->m_flSpeedAsPortionOfCrouchTopSpeed, 0.0f, 1.0f);
		auto duck_speed = duck_amount * max_velocity;

		avg_speedfactor += duck_speed * (0.5f - avg_speedfactor);
	}

	return animstate->m_flAimYawMax * avg_speedfactor;
}

void player_t::invalidate_physics_recursive(int change_flags)
{
	if (!this)
		return;

	reinterpret_cast <void(__thiscall*)(void*, int)> (g_ctx.addresses.invalidate_physics_recursive)(this, change_flags);
}

float& viewmodel_t::m_flCycle()
{
	static auto m_flCycle = util::find_in_datamap(GetPredDescMap(), crypt_str("m_flCycle"));
	return *(float*)(uintptr_t(this) + m_flCycle);
}

float& viewmodel_t::m_flAnimTime()
{
	static auto m_flAnimTime = util::find_in_datamap(GetPredDescMap(), crypt_str("m_flAnimTime"));
	return *(float*)(uintptr_t(this) + m_flAnimTime);
}

void viewmodel_t::SendViewModelMatchingSequence(int sequence)
{
	using Fn = void(__thiscall*)(void*, int);
	call_virtual <Fn>(this, g_ctx.indexes.at(7))(this, sequence);
}

void CHudChat::chat_print(const char* fmt, ...)
{
	char msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, 1024, fmt, args);
	call_virtual <void(__cdecl*)(void*, int, int, const char*, ...)>(this, g_ctx.indexes.at(8))(this, 0, 0, fmt);
	va_end(args);
}

void player_t::copy_poseparameter(float* poses)
{
	if (!this)
		return;

	std::memcpy(poses, this->m_flPoseParameter().data(), 24 * sizeof(float));
}

void player_t::set_poseparameter(float* poses)
{
	if (!this)
		return;

	std::memcpy(this->m_flPoseParameter().data(), poses, 24 * sizeof(float));
}

void player_t::copy_animlayers(AnimationLayer* layers)
{
	if (!this)
		return;

	std::memcpy(layers, this->get_animlayers(), this->animlayer_count() * sizeof(AnimationLayer));
}

void player_t::set_animlayers(AnimationLayer* layers)
{
	if (!this)
		return;

	std::memcpy(this->get_animlayers(), layers, this->animlayer_count() * sizeof(AnimationLayer));
}

void player_t::set_animation_state(c_baseplayeranimationstate* state)
{
	if (!this)
		return;

	*reinterpret_cast<c_baseplayeranimationstate**>(reinterpret_cast<void*>(uintptr_t(this) + 0x9960)) = state;
}

void c_baseplayeranimationstate::set_layer_sequence(AnimationLayer* animlayer, int activity)
{
	int sequence = this->select_sequence_from_activity_modifier(activity);
	if (sequence < 2)
		return;

	animlayer->m_nSequence = sequence;
	animlayer->m_flPlaybackRate = m_pBasePlayer->get_layer_sequence_cycle_rate(animlayer, sequence);
	animlayer->m_flCycle = animlayer->m_flWeight = 0.0f;
}

int c_baseplayeranimationstate::select_sequence_from_activity_modifier(int activity)
{
	bool is_player_ducked = m_flAnimDuckAmount > 0.55f;
	bool is_player_running = m_flSpeedAsPortionOfWalkTopSpeed > 0.25f;

	int layer_sequence = -1;
	switch (activity)
	{
	case ACT_CSGO_JUMP:
	{
		layer_sequence = 15 + int(is_player_running);
		if (is_player_ducked)
			layer_sequence = 17 + int(is_player_running);
	}
	break;
	case ACT_CSGO_ALIVE_LOOP:
	{
		layer_sequence = 8;
		if (m_pWeaponLast != m_pWeapon)
			layer_sequence = 9;
	}
	break;
	case ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING:
	{
		layer_sequence = 6;
	}
	break;
	case ACT_CSGO_FALL:
	{
		layer_sequence = 14;
	}
	break;
	case ACT_CSGO_IDLE_TURN_BALANCEADJUST:
	{
		layer_sequence = 4;
	}
	break;
	case ACT_CSGO_LAND_LIGHT:
	{
		layer_sequence = 20;
		if (is_player_running)
			layer_sequence = 22;

		if (is_player_ducked)
		{
			layer_sequence = 21;
			if (is_player_running)
				layer_sequence = 19;
		}
	}
	break;
	case ACT_CSGO_LAND_HEAVY:
	{
		layer_sequence = 23;
		if (is_player_ducked)
			layer_sequence = 24;
	}
	break;
	case ACT_CSGO_CLIMB_LADDER:
	{
		layer_sequence = 13;
	}
	break;
	default: break;
	}

	return layer_sequence;
}

void c_baseplayeranimationstate::increment_layer_cycle(AnimationLayer* layer, bool is_loop)
{
	float new_cycle = (layer->m_flPlaybackRate * this->m_flLastUpdateIncrement) + layer->m_flCycle;
	if (!is_loop && new_cycle >= 1.0f)
		new_cycle = 0.999f;

	new_cycle -= (int)(new_cycle);
	if (new_cycle < 0.0f)
		new_cycle += 1.0f;

	if (new_cycle > 1.0f)
		new_cycle -= 1.0f;

	layer->m_flCycle = new_cycle;
}

bool c_baseplayeranimationstate::is_layer_sequence_finished(AnimationLayer* layer, float time)
{
	return (layer->m_flPlaybackRate * time) + layer->m_flCycle >= 1.0f;
}

void c_baseplayeranimationstate::set_layer_cycle(AnimationLayer* animlayer, float cycle)
{
	if (animlayer)
		animlayer->m_flCycle = cycle;
}

void c_baseplayeranimationstate::set_layer_rate(AnimationLayer* animlayer, float rate)
{
	if (animlayer)
		animlayer->m_flPlaybackRate = rate;
}

void c_baseplayeranimationstate::set_layer_weight(AnimationLayer* animlayer, float weight)
{
	if (animlayer)
		animlayer->m_flWeight = weight;
}
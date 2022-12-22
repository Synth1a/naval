// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "util.hpp"
#include "..\cheats\visuals\player_esp.h"
#include "..\cheats\lagcompensation\animation_system.h"
#include "..\cheats\networking\networking.h"
#include "..\cheats\exploits\exploits.h"
#include "..\cheats\misc\misc.h"
#include <thread>
#include <utils/imports.h>

#define INRANGE(x, a, b) (x >= a && x <= b)  //-V1003
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0)) //-V1003
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

namespace util
{
	int epoch_time()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	uintptr_t find_pattern(const char* module_name, const char* pattern, const char* mask)
	{
		MODULEINFO module_info = {};
		K32GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module_name), &module_info, sizeof(MODULEINFO));
		const auto address = reinterpret_cast<std::uint8_t*>(module_info.lpBaseOfDll);
		const auto size = module_info.SizeOfImage;
		std::vector < std::pair < std::uint8_t, bool>> signature;
		for (auto i = 0u; mask[i]; i++)
			signature.emplace_back(std::make_pair(pattern[i], mask[i] == 'x'));
		auto ret = std::search(address, address + size, signature.begin(), signature.end(),
			[](std::uint8_t curr, std::pair<std::uint8_t, bool> curr_pattern)
		{
			return (!curr_pattern.second) || curr == curr_pattern.first;
		});
		return ret == address + size ? 0 : std::uintptr_t(ret);
	}

	uint64_t FindSignature(const char* szModule, const char* szSignature)
	{
		MODULEINFO modInfo;
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(szModule), &modInfo, sizeof(MODULEINFO));

		uintptr_t startAddress = (DWORD)modInfo.lpBaseOfDll; //-V101 //-V220
		uintptr_t endAddress = startAddress + modInfo.SizeOfImage;

		const char* pat = szSignature;
		uintptr_t firstMatch = 0;

		for (auto pCur = startAddress; pCur < endAddress; pCur++)
		{
			if (!*pat)
				return firstMatch;

			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GETBYTE(pat))
			{
				if (!firstMatch)
					firstMatch = pCur;

				if (!pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;
				else
					pat += 2;
			}
			else
			{
				pat = szSignature;
				firstMatch = 0;
			}
		}

		return 0;
	}

	IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
	{
		IDirect3DTexture9* skin_image = nullptr;
		std::string vpk_path;

		if (strcmp(weapon_name, crypt_str("unknown")) && strcmp(weapon_name, crypt_str("knife")) && strcmp(weapon_name, crypt_str("gloves"))) //-V526
		{
			if (skin_name.empty() || skin_name == crypt_str("default"))
				vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");
			else
				vpk_path = crypt_str("resource/flash/econ/default_generated/") + std::string(weapon_name) + crypt_str("_") + std::string(skin_name) + crypt_str("_light_large.png");
		}
		else
		{
			if (!strcmp(weapon_name, crypt_str("knife")))
				vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
			else if (!strcmp(weapon_name, crypt_str("gloves")))
				vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
			else if (!strcmp(weapon_name, crypt_str("unknown")))
				vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

		}
		const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));
		if (handle)
		{
			int file_len = m_basefilesys()->Size(handle);
			char* image = new char[file_len]; //-V121

			m_basefilesys()->Read(image, file_len, handle);
			m_basefilesys()->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}

		if (!skin_image)
		{
			std::string vpk_path;

			if (strstr(weapon_name, crypt_str("bloodhound")) != NULL || strstr(weapon_name, crypt_str("hydra")) != NULL)
				vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
			else
				vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");

			const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));

			if (handle)
			{
				int file_len = m_basefilesys()->Size(handle);
				char* image = new char[file_len]; //-V121

				m_basefilesys()->Read(image, file_len, handle);
				m_basefilesys()->Close(handle);

				D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
				delete[] image;
			}
		}

		return skin_image;
	}

	bool visible(const Vector& start, const Vector& end, entity_t* entity, player_t* from)
	{
		trace_t trace;

		CTraceFilter filter;
		filter.pSkip = from;

		g_ctx.globals.autowalling = true;
		m_trace()->TraceRay(Ray_t(start, end), MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);
		g_ctx.globals.autowalling = false;

		return trace.hit_entity == entity || trace.fraction == 1.0f; //-V550
	}

	bool is_button_down(int code)
	{
		if (code <= KEY_NONE || code >= KEY_MAX)
			return false;

		if (!m_engine()->IsActiveApp())
			return false;

		if (m_engine()->Con_IsVisible())
			return false;

		if (!g_ctx.convars.cl_mouseenable->GetBool())
			return false;

		return m_inputsys()->IsButtonDown((ButtonCode_t)code);
	}

	IMaterial* create_material(bool lit, const std::string& material_data)
	{
		static auto created = 0;
		std::string type = lit ? crypt_str("VertexLitGeneric") : crypt_str("UnlitGeneric");

		auto matname = crypt_str("NAVAL_") + std::to_string(created);
		++created;

		auto keyValues = new KeyValues(matname.c_str());

		using KeyValuesFn = void(__thiscall*)(void*, const char*, int, int);
		reinterpret_cast <KeyValuesFn> (g_ctx.addresses.key_values)(keyValues, type.c_str(), 0, 0);

		using LoadFromBufferFn = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*);
		reinterpret_cast <LoadFromBufferFn> (g_ctx.addresses.load_from_buffer)(keyValues, matname.c_str(), material_data.c_str(), nullptr, nullptr, nullptr);

		auto material = m_materialsystem()->CreateMaterial(matname.c_str(), keyValues);
		material->IncrementReferenceCount();

		return material;
	}

	void movement_fix(Vector& wish_angle, CUserCmd* m_pcmd)
	{
		Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
		auto viewangles = m_pcmd->m_viewangles;
		viewangles.Normalized();

		math::angle_vectors(wish_angle, &view_fwd, &view_right, &view_up);
		math::angle_vectors(viewangles, &cmd_fwd, &cmd_right, &cmd_up);

		float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
		float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
		float v12 = sqrtf(view_up.z * view_up.z);

		Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
		Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
		Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

		float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
		float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
		float v18 = sqrtf(cmd_up.z * cmd_up.z);

		Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
		Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
		Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

		float v22 = norm_view_fwd.x * m_pcmd->m_forwardmove;
		float v26 = norm_view_fwd.y * m_pcmd->m_forwardmove;
		float v28 = norm_view_fwd.z * m_pcmd->m_forwardmove;
		float v24 = norm_view_right.x * m_pcmd->m_sidemove;
		float v23 = norm_view_right.y * m_pcmd->m_sidemove;
		float v25 = norm_view_right.z * m_pcmd->m_sidemove;
		float v30 = norm_view_up.x * m_pcmd->m_upmove;
		float v27 = norm_view_up.z * m_pcmd->m_upmove;
		float v29 = norm_view_up.y * m_pcmd->m_upmove;

		m_pcmd->m_forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
			+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
			+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
		m_pcmd->m_sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
			+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
			+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
		m_pcmd->m_upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
			+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
			+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

		m_pcmd->m_forwardmove = math::clamp(m_pcmd->m_forwardmove, -g_ctx.convars.cl_forwardspeed->GetFloat(), g_ctx.convars.cl_forwardspeed->GetFloat());
		m_pcmd->m_sidemove = math::clamp(m_pcmd->m_sidemove, -g_ctx.convars.cl_sidespeed->GetFloat(), g_ctx.convars.cl_sidespeed->GetFloat());
		m_pcmd->m_upmove = math::clamp(m_pcmd->m_upmove, -g_ctx.convars.cl_upspeed->GetFloat(), g_ctx.convars.cl_upspeed->GetFloat());
	}

	unsigned int find_in_datamap(datamap_t* map, const char *name)
	{
		while (map)
		{
			for (auto i = 0; i < map->dataNumFields; ++i)
			{
				if (!map->dataDesc[i].fieldName)
					continue;

				if (!strcmp(name, map->dataDesc[i].fieldName))
					return map->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (map->dataDesc[i].fieldType == FIELD_EMBEDDED)
				{
					if (map->dataDesc[i].td)
					{
						unsigned int offset;

						if (offset = find_in_datamap(map->dataDesc[i].td, name))
							return offset;
					}
				}
			}

			map = map->baseMap;
		}

		return 0;
	}

	bool get_bbox(entity_t* ent, Box& box, bool player_esp)
	{
		if (player_esp) 
		{
			const auto min = ent->m_vecMins();
			const auto max = ent->m_vecMaxs();

			Vector dir, vF, vR, vU;

			m_engine()->GetViewAngles(dir);
			dir.x = 0;
			dir.z = 0;
			math::angle_vectors(dir, &vF, &vR, &vU);

			auto zh = vU * max.z + vF * max.y + vR * min.x; // = Front left front
			auto e = vU * max.z + vF * max.y + vR * max.x; //  = Front right front
			auto d = vU * max.z + vF * min.y + vR * min.x; //  = Front left back
			auto c = vU * max.z + vF * min.y + vR * max.x; //  = Front right back

			auto g = vU * min.z + vF * max.y + vR * min.x; //  = Bottom left front
			auto f = vU * min.z + vF * max.y + vR * max.x; //  = Bottom right front
			auto a = vU * min.z + vF * min.y + vR * min.x; //  = Bottom left back
			auto b = vU * min.z + vF * min.y + vR * max.x; //  = Bottom right back*-

			Vector pointList[] = {
				a,
				b,
				c,
				d,
				e,
				f,
				g,
				zh,
			};

			Vector transformed[ARRAYSIZE(pointList)];

			for (int i = 0; i < ARRAYSIZE(pointList); i++)
			{
				pointList[i] += ent->GetAbsOrigin();

				if (!math::world_to_screen(pointList[i], transformed[i]))
					return false;
			}

			float left = FLT_MAX;
			float top = -FLT_MAX;
			float right = -FLT_MAX;
			float bottom = FLT_MAX;
			for (int i = 0; i < ARRAYSIZE(pointList); i++) {
				if (left > transformed[i].x)
					left = transformed[i].x;
				if (top < transformed[i].y)
					top = transformed[i].y;
				if (right < transformed[i].x)
					right = transformed[i].x;
				if (bottom > transformed[i].y)
					bottom = transformed[i].y;
			}

			box.x = left;
			box.y = bottom;
			box.w = right - left;
			box.h = top - bottom;

			return true;
		}
		else
		{
			const auto m_rgflCoordinateFrame = ent->m_rgflCoordinateFrame();
			const auto min = ent->m_vecMins();
			const auto max = ent->m_vecMaxs();

			Vector points[8] =
			{
				Vector(min.x, min.y, min.z),
				Vector(min.x, max.y, min.z),
				Vector(max.x, max.y, min.z),
				Vector(max.x, min.y, min.z),
				Vector(max.x, max.y, max.z),
				Vector(min.x, max.y, max.z),
				Vector(min.x, min.y, max.z),
				Vector(max.x, min.y, max.z)
			};

			Vector pointsTransformed[8];

			for (auto i = 0; i < 8; i++)
				math::vector_transform(points[i], m_rgflCoordinateFrame, pointsTransformed[i]);

			Vector pos = ent->GetAbsOrigin();
			Vector flb;
			Vector brt;
			Vector blb;
			Vector frt;
			Vector frb;
			Vector brb;
			Vector blt;
			Vector flt;

			auto bFlb = math::world_to_screen(pointsTransformed[3], flb);
			auto bBrt = math::world_to_screen(pointsTransformed[5], brt);
			auto bBlb = math::world_to_screen(pointsTransformed[0], blb);
			auto bFrt = math::world_to_screen(pointsTransformed[4], frt);
			auto bFrb = math::world_to_screen(pointsTransformed[2], frb);
			auto bBrb = math::world_to_screen(pointsTransformed[1], brb);
			auto bBlt = math::world_to_screen(pointsTransformed[6], blt);
			auto bFlt = math::world_to_screen(pointsTransformed[7], flt);

			if (!bFlb && !bBrt && !bBlb && !bFrt && !bFrb && !bBrb && !bBlt && !bFlt)
				return false;

			Vector arr[8] =
			{
				flb,
				brt,
				blb,
				frt,
				frb,
				brb,
				blt,
				flt
			};

			auto left = flb.x;
			auto top = flb.y;
			auto right = flb.x;
			auto bottom = flb.y;

			for (auto i = 1; i < 8; i++)
			{
				if (left > arr[i].x)
					left = arr[i].x;
				if (top < arr[i].y)
					top = arr[i].y;
				if (right < arr[i].x)
					right = arr[i].x;
				if (bottom > arr[i].y)
					bottom = arr[i].y;
			}

			box.x = left;
			box.y = bottom;
			box.w = right - left;
			box.h = top - bottom;

			return true;
		}
	}

	void RotateMovement(CUserCmd* cmd, float yaw)
	{
		Vector viewangles;
		m_engine()->GetViewAngles(viewangles);

		float rotation = DEG2RAD(viewangles.y - yaw);

		float cos_rot = cos(rotation);
		float sin_rot = sin(rotation);

		float new_forwardmove = cos_rot * cmd->m_forwardmove - sin_rot * cmd->m_sidemove;
		float new_sidemove = sin_rot * cmd->m_forwardmove + cos_rot * cmd->m_sidemove;

		cmd->m_forwardmove = new_forwardmove;
		cmd->m_sidemove = new_sidemove;
	}

	void color_modulate(float color[3], IMaterial* material)
	{
		auto found = false;
		auto var = material->FindVar(crypt_str("$envmaptint"), &found);

		if (found)
			var->set_vec_value(color[0], color[1], color[2]);

		m_renderview()->SetColorModulation(color[0], color[1], color[2]);
	}

	IMaterial* apply_materials(int materials)
	{
		switch (materials)
		{
		case 0: return g_ctx.chams.regular; break;
		case 1: return g_ctx.chams.metallic; break;
		case 2: return g_ctx.chams.flat; break;
		case 3: return g_ctx.chams.pulse; break;
		case 4: return g_ctx.chams.crystal; break;
		case 5: return g_ctx.chams.glass; break;
		case 6: return g_ctx.chams.circuit; break;
		case 7: return g_ctx.chams.golden; break;
		case 8: return g_ctx.chams.glow; break;
		case 9: return g_ctx.chams.animated; break;
		default:
			return g_ctx.chams.regular; break;
		}
	}

	void create_state(c_baseplayeranimationstate* state, player_t* e)
	{
		using Fn = void(__thiscall*)(c_baseplayeranimationstate*, player_t*);
		static auto fn = reinterpret_cast <Fn> (g_ctx.addresses.create_animation_state);

		fn(state, e);
	}

	void update_state(c_baseplayeranimationstate* state, const Vector& angles)
	{
		using Fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
		static auto fn = reinterpret_cast <Fn> (g_ctx.addresses.update_animation_state);

		fn(state, nullptr, 0.0f, angles[1], angles[0], nullptr);
	}

	void reset_state(c_baseplayeranimationstate* state)
	{
		using Fn = void(__thiscall*)(c_baseplayeranimationstate*);
		static auto fn = reinterpret_cast <Fn> (g_ctx.addresses.reset_animation_state);

		fn(state);
	}

	bool is_breakable_entity(IClientEntity* e)
	{
		if (!e || !e->EntIndex())
			return false;

		auto take_damage = *(uintptr_t*)((uintptr_t)g_ctx.addresses.is_breakable + 0x26);
		auto backup = *(uint8_t*)((uintptr_t)e + take_damage);

		auto client_class = e->GetClientClass();
		auto network_name = client_class->m_pNetworkName;

		if (!strcmp(network_name, crypt_str("CBreakableSurface")))
			*(uint8_t*)((uintptr_t)e + take_damage) = DAMAGE_YES;
		else if (!strcmp(network_name, crypt_str("CBaseDoor")) || !strcmp(network_name, crypt_str("CDynamicProp")))
			*(uint8_t*)((uintptr_t)e + take_damage) = DAMAGE_NO;

		using Fn = bool(__thiscall*)(IClientEntity*);
		auto result = ((Fn)g_ctx.addresses.is_breakable)(e);

		*(uint8_t*)((uintptr_t)e + take_damage) = backup;
		return result;
	}

	void copy_command(CUserCmd* cmd, int tickbase_shift)
	{
		if (g_cfg.ragebot.slow_teleport)
		{
			Vector angResistance = ZERO;
			math::vector_angles((g_ctx.local()->m_vecVelocity() * -1.f), angResistance);

			angResistance.y = cmd->m_viewangles.y - angResistance.y;
			angResistance.x = cmd->m_viewangles.x - angResistance.x;

			Vector vecResistance = ZERO;
			math::angle_vectors(angResistance, vecResistance);

			cmd->m_forwardmove = std::clamp(vecResistance.x, -450.f, 450.0f);
			cmd->m_sidemove = std::clamp(vecResistance.y, -450.f, 450.0f);
		}
		else
		{
			if (cmd->m_sidemove > 5.0f)
				cmd->m_sidemove = 450.0f;
			else if (cmd->m_sidemove < -5.0f)
				cmd->m_sidemove = -450.0f;

			if (cmd->m_forwardmove > 5.0f)
				cmd->m_forwardmove = 450.0f;
			else if (cmd->m_forwardmove < -5.0f)
				cmd->m_forwardmove = -450.0f;
		}

		misc::get().automatic_peek(cmd, g_ctx.globals.original_viewangles.y);
		util::movement_fix(g_ctx.globals.original_viewangles, cmd);

		auto commands_to_add = 0;

		do
		{
			auto sequence_number = commands_to_add + cmd->m_command_number;

			auto command = m_input()->GetUserCmd(sequence_number);
			auto verified_command = m_input()->GetVerifiedUserCmd(sequence_number);

			memcpy(command, cmd, sizeof(CUserCmd));

			if (command->m_tickcount != INT_MAX && m_clientstate()->iDeltaTick > 0)
				m_prediction()->Update(
					m_clientstate()->iDeltaTick, true,
					m_clientstate()->nLastCommandAck,
					m_clientstate()->nLastOutgoingCommand + m_clientstate()->iChokedCommands
				);

			command->m_command_number = sequence_number;
			command->m_predicted = command->m_tickcount != INT_MAX;

			++m_clientstate()->iChokedCommands; //-V807

			if (m_clientstate()->pNetChannel)
			{
				++m_clientstate()->pNetChannel->m_nChokedPackets;
				++m_clientstate()->pNetChannel->m_nOutSequenceNr;
			}

			math::normalize_angles(command->m_viewangles);

			memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd)); //-V598
			verified_command->m_crc = command->GetChecksum();

			++commands_to_add;
		} while (commands_to_add != tickbase_shift);

		m_prediction()->m_bPreviousAckHadErrors() = true;
		m_prediction()->m_iPredictedCommands() = 0;
	}

	float get_interpolation()
	{
		auto updaterate = math::clamp(g_ctx.convars.cl_updaterate->GetFloat(), g_ctx.convars.sv_minupdaterate->GetFloat(), g_ctx.convars.sv_maxupdaterate->GetFloat());
		auto lerp_ratio = math::clamp(g_ctx.convars.cl_interp_ratio->GetFloat(), g_ctx.convars.sv_client_min_interp_ratio->GetFloat(), g_ctx.convars.sv_client_max_interp_ratio->GetFloat());

		return math::clamp(lerp_ratio / updaterate, g_ctx.convars.cl_interp->GetFloat(), 1.0f);
	}

	DWORD* FindHudElement(const char* szHudName)
	{
		return (DWORD*)(((DWORD(__thiscall*)(void*, const char*))(g_ctx.addresses.find_hud_element))(*reinterpret_cast <PDWORD*>(g_ctx.addresses.game_hud), szHudName));
	}

	void fn_equip(attributableitem_t* item, player_t* owner) {
		if (!owner)
			return;

		if (!item)
			return;

		static auto fnEquip_s = reinterpret_cast<int(__thiscall*)(void*, void*)>(g_ctx.addresses.equip);
		fnEquip_s(item, owner);
	}

	bool is_valid_hitgroup(int index)
	{
		if ((index >= HITGROUP_HEAD && index <= HITGROUP_RIGHTLEG) || index == HITGROUP_GEAR)
			return true;

		return false;
	}

	int get_hitbox_by_hitgroup(int index)
	{
		switch (index)
		{
		case HITGROUP_HEAD:
			return HITBOX_HEAD;
		case HITGROUP_CHEST:
			return HITBOX_CHEST;
		case HITGROUP_STOMACH:
			return HITBOX_STOMACH;
		case HITGROUP_LEFTARM:
			return HITBOX_LEFT_HAND;
		case HITGROUP_RIGHTARM:
			return HITBOX_RIGHT_HAND;
		case HITGROUP_LEFTLEG:
			return HITBOX_RIGHT_CALF;
		case HITGROUP_RIGHTLEG:
			return HITBOX_LEFT_CALF;
		default:
			return HITBOX_PELVIS;
		}
	}

	bool get_backtrack_matrix(player_t* player, matrix3x4_t* out)
	{
		if (!g_cfg.ragebot.enable && !g_cfg.legitbot.enabled)
			return false;

		const auto oldest = lagcompensation::get().GetOldestRecord(player);

		if (!oldest.has_value() || oldest.value()->m_bDormant || (oldest.value()->m_pEntity && oldest.value()->m_pEntity->m_bGunGameImmunity()))
			return false;

		auto next = oldest.value()->m_vecOrigin;
		auto time_next = oldest.value()->m_flSimulationTime;

		float total_latency = networking::get().average_outgoing + networking::get().average_incoming;
		total_latency = math::clamp(total_latency, 0.f, g_ctx.convars.sv_maxunlag->GetFloat());

		float correct = total_latency + get_interpolation();
		float time_delta = time_next - oldest.value()->m_flSimulationTime;
		float deadtime = oldest.value()->m_flSimulationTime + correct + time_delta;

		float curtime = g_ctx.local()->is_alive() ? TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) : m_globals()->m_curtime;
		float delta = deadtime - curtime;

		float mul = 1.f / time_delta;
		auto lerp = math::interpolate(next, oldest.value()->m_vecOrigin, std::clamp(delta * mul, 0.f, 1.f));

		matrix3x4_t ret[MAXSTUDIOBONES];

		std::memcpy(ret, oldest.value()->m_pMatrix.main, sizeof(ret));

		for (int i = 0; i < MAXSTUDIOBONES; ++i) {
			auto matrix_delta = oldest.value()->m_pMatrix.main[i].GetOrigin() - oldest.value()->m_vecOrigin;
			ret[i].SetOrigin(matrix_delta + lerp);
		}

		std::memcpy(out, ret, sizeof(ret));
		return true;
	}
}
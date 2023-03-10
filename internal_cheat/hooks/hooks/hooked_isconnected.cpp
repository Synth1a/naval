// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"

using IsConnected_t = bool(__fastcall*)(void*);

bool __fastcall hooks::hooked_isconnected(void* ecx, void* edx)
{
	static auto original_fn = engine_hook->get_func_address <IsConnected_t>(27);

	if (g_cfg.misc.inventory_access && _ReturnAddress() == (void*)g_ctx.addresses.return_to.loadout_allowed)
		return false;

	return original_fn(ecx);
 }

using GetScreenAspectRatio_t = float(__thiscall*)(void*, int, int);

float __fastcall hooks::hooked_getscreenaspectratio(void* ecx, void* edx, int width, int height)
{
	static auto original_fn = engine_hook->get_func_address <GetScreenAspectRatio_t>(101);

	if (!g_cfg.esp.aspect_ratio)
		return original_fn(ecx, width, height);

	return g_cfg.esp.aspect_ratio_amount;
}
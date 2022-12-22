// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\misc\logs.h"

using IsHLTV_t = bool(__thiscall*)(void*);

bool __fastcall hooks::hooked_ishltv(void* ecx, void* edx) 
{
	static auto original_fn = engine_hook->get_func_address <IsHLTV_t>(93);

	if (_ReturnAddress() == (void*)g_ctx.addresses.return_to.accumulate_layers || _ReturnAddress() == (void*)g_ctx.addresses.return_to.setup_velocity)
		return true;

	return original_fn(ecx);
}
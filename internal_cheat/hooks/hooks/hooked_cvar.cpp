// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"

using r_drawmodelstatsoverlay_t = int(__fastcall*)(ICvar*);
using sv_cheats_t = bool(__fastcall*)(ICvar*);

int __fastcall hooks::hooked_r_drawmodelstatsoverlay(ICvar* ecx, void* edx)
{
	// this is to fix some entities are not being drawn in drawmodelexecute, as an example: ragdolls.
	static auto original_fn = r_drawmodelstatsoverlay_hook->get_func_address <r_drawmodelstatsoverlay_t> (13);

	if (_ReturnAddress() == (void*)g_ctx.addresses.return_to.get_client_model_renderable)
		return 1;

	return original_fn(ecx);
}

bool __fastcall hooks::hooked_sv_cheats(ICvar* ecx, void* edx)
{
	// since i can't figure out on how to fix UpdateVisibilityAllEntities to work ( it's crashing. ) maybe i'll just hook it with cam_think as if it was the return address to this.
	static auto original_fn = sv_cheats_hook->get_func_address <sv_cheats_t>(13);

	if (_ReturnAddress() == (void*)g_ctx.addresses.return_to.cam_think)
		return true;

	return original_fn(ecx);
}
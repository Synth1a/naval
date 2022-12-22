// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\networking\networking.h"

using PacketStart_t = void(__thiscall*)(void*, int, int);

void __fastcall hooks::hooked_packetstart(void* ecx, void* edx, int incoming, int outgoing)
{
	static auto original_fn = clientstate_hook->get_func_address <PacketStart_t>(5);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (engineprediction::get().should_process_packetstart(outgoing))
		return original_fn(ecx, incoming, outgoing);
}

using PacketEnd_t = void(__thiscall*)(void*);

void __fastcall hooks::hooked_packetend(void* ecx, void* edx)
{
	static auto original_fn = clientstate_hook->get_func_address <PacketEnd_t>(6);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	networking::get().on_packetend((CClientState*)(ecx));
	return original_fn(ecx);
}

void __fastcall hooks::hooked_checkfilecrcswithserver(void* ecx, void* edx)
{
    
}

bool __fastcall hooks::hooked_loosefileallowed(void* ecx, void* edx)
{
    return true;
}
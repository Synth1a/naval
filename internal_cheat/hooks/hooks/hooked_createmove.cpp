// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\ragebot\aim.h"
#include "..\..\cheats\ragebot\antiaim.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\legitbot\legitbot.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\misc\bunnyhop.h"
#include "..\..\cheats\misc\airstrafe.h"
#include "..\..\cheats\misc\spammers.h"
#include "..\..\cheats\misc\misc.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\fakewalk\slowwalk.h"
#include "..\..\cheats\lagcompensation\local_animations.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\networking\networking.h"
#include "..\..\cheats\autowall\penetration.h"
#include "..\..\cheats\exploits\exploits.h"

using CreateMove_t = void(__thiscall*)(IBaseClientDLL*, int, float, bool);

void __stdcall hooked_createmove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
{
	static auto original_fn = hooks::client_hook->get_func_address <CreateMove_t>(22);
	original_fn(m_client(), sequence_number, input_sample_frametime, active);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	CUserCmd* m_pcmd = m_input()->GetUserCmd(sequence_number);
	CVerifiedUserCmd* verified = m_input()->GetVerifiedUserCmd(sequence_number);

	if (!verified || !networking::get().setup_packet(sequence_number, &bSendPacket))
		return original_fn(m_client(), sequence_number, input_sample_frametime, active);

	// don't run createmove if we are not in game.
	if (!g_ctx.available())
		return original_fn(m_client(), sequence_number, input_sample_frametime, active);

	// run codes no matter if we die.
	misc::get().rank_reveal();
	//misc::get().mouse_delta_fix(m_pcmd);
	spammers::get().clan_tag();

	// codes that are not supposed to run if we die.
	if (!g_ctx.local()->is_alive())
		return original_fn(m_client(), sequence_number, input_sample_frametime, active);

	// createmove weapon and also checks if we don't have guns.
	g_ctx.globals.weapon = g_ctx.local()->m_hActiveWeapon().Get();
	if (!g_ctx.globals.weapon)
		return original_fn(m_client(), sequence_number, input_sample_frametime, active);

	// we are in createmove right now.
	g_ctx.globals.in_createmove = true;

	// start our new command.
	g_ctx.set_command(m_pcmd);

	if (exploitsystem::get().m_shift_data.m_shifting)
	{
		// start our new branch of createmove.
		networking::get().start_move(m_pcmd, bSendPacket);

		// preparing stuff.
		engineprediction::get().update();
		engineprediction::get().store_netvars(m_pcmd->m_command_number);

		// do miscellaneous stuff.
		if (g_cfg.misc.bunnyhop)
			bunnyhop::get().create_move();

		if (g_cfg.misc.airstrafe)
			airstrafe::get().create_move(m_pcmd, g_ctx.globals.original_viewangles.y);

		misc::get().SlideWalk(m_pcmd);
		misc::get().NoDuck(m_pcmd);
		misc::get().AutoCrouch(m_pcmd);
		GrenadePrediction::get().Tick(m_pcmd->m_buttons);

		// do crouching in air.
		if (g_cfg.misc.crouch_in_air && !(g_ctx.local()->m_fFlags() & FL_ONGROUND))
			m_pcmd->m_buttons |= IN_DUCK;

		// predicting commands.
		engineprediction::get().start(m_pcmd);

		g_ctx.globals.eye_pos = g_ctx.local()->get_shoot_position();

		g_ctx.globals.weapon->update_accuracy_penality();

		// penetration crosshair.
		misc::get().init_penetration();

		// edge jump, i honestly don't know what does this do.
		if (g_cfg.keybinds.key[EDGE_JUMP_KEYBIND].active && engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && !(g_ctx.local()->m_fFlags() & FL_ONGROUND)) //-V807
			m_pcmd->m_buttons |= IN_JUMP;

		// fast stop (stops faster when you are not using any movement button).
		misc::get().fast_stop(m_pcmd, g_ctx.globals.original_viewangles.y);

		// slow walk.
		if (!g_cfg.misc.slowwalk_type)
		{
			if (g_cfg.keybinds.key[SLOW_WALK_KEYBIND].active)
				slowwalk::get().create_move(m_pcmd);
			if (g_cfg.ragebot.enable && !g_ctx.globals.weapon->is_non_aim() && engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND)
				slowwalk::get().create_move(m_pcmd, 0.95f + 0.003125f * (16 - m_clientstate()->iChokedCommands));
		}
		else
		{
			if (g_cfg.keybinds.key[SLOW_WALK_KEYBIND].active)
				slowwalk::get().create_move(m_pcmd, g_cfg.misc.slowwalk_speed);
		}

		// automatic fallback.
		misc::get().automatic_peek(m_pcmd, g_ctx.globals.original_viewangles.y);

		// anti-aim code.
		antiaim::get().create_move(m_pcmd, bSendPacket);

		// code about packet or something something.
		networking::get().packet_cycle(m_pcmd, bSendPacket);

		// end our prediction.
		engineprediction::get().finish(m_pcmd);

		// anti untrusted, prevent weird viewangles.
		if (g_cfg.misc.anti_untrusted)
			math::normalize_angles(m_pcmd->m_viewangles);
		else
		{
			m_pcmd->m_viewangles.y = math::normalize_yaw(m_pcmd->m_viewangles.y);
			m_pcmd->m_viewangles.z = 0.0f;
		}

		// fix our movement so we still walk facing our camera.
		util::movement_fix(g_ctx.globals.original_viewangles, m_pcmd);

		// we are not supposed to send another packet here.
		if (bSendPacket && g_ctx.globals.should_send_packet)
			g_ctx.globals.should_send_packet = false;

		// buy bot at last. 
		misc::get().buybot();

		if (bSendPacket)
			engineprediction::get().command_list.emplace_back(m_pcmd->m_command_number);

		// store our animation data for later usage in animation fix.
		local_animations::get().store_animations_data(m_pcmd, bSendPacket);

		// we are no longer in createmove.
		g_ctx.globals.in_createmove = false;

		verified->m_cmd = *m_pcmd;
		verified->m_crc = m_pcmd->GetChecksum();

		return;
	}

	// start our new branch of createmove.
	networking::get().start_move(m_pcmd, bSendPacket);

	// preparing stuff.
	engineprediction::get().update();
	engineprediction::get().store_netvars(m_pcmd->m_command_number);

	// do miscellaneous stuff.
	if (g_cfg.misc.bunnyhop)
		bunnyhop::get().create_move();

	if (g_cfg.misc.airstrafe)
		airstrafe::get().create_move(m_pcmd, g_ctx.globals.original_viewangles.y);

	misc::get().SlideWalk(m_pcmd);
	misc::get().NoDuck(m_pcmd);
	misc::get().AutoCrouch(m_pcmd);
	GrenadePrediction::get().Tick(m_pcmd->m_buttons);

	// do crouching in air.
	if (g_cfg.misc.crouch_in_air && !(g_ctx.local()->m_fFlags() & FL_ONGROUND))
		m_pcmd->m_buttons |= IN_DUCK;

	// predicting commands.
	engineprediction::get().start(m_pcmd);

	g_ctx.globals.eye_pos = g_ctx.local()->get_shoot_position();

	g_ctx.globals.weapon->update_accuracy_penality();

	// penetration crosshair.
	misc::get().init_penetration();

	// edge jump, i honestly don't know what does this do.
	if (g_cfg.keybinds.key[EDGE_JUMP_KEYBIND].active && engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && !(g_ctx.local()->m_fFlags() & FL_ONGROUND)) //-V807
		m_pcmd->m_buttons |= IN_JUMP;

	// fast stop (stops faster when you are not using any movement button).
	misc::get().fast_stop(m_pcmd, g_ctx.globals.original_viewangles.y);

	// slow walk.
	if (!g_cfg.misc.slowwalk_type)
	{
		if (g_cfg.keybinds.key[SLOW_WALK_KEYBIND].active)
			slowwalk::get().create_move(m_pcmd);
		if (g_cfg.ragebot.enable && !g_ctx.globals.weapon->is_non_aim() && engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND)
			slowwalk::get().create_move(m_pcmd, 0.95f + 0.003125f * (16 - m_clientstate()->iChokedCommands));
	}
	else
	{
		if (g_cfg.keybinds.key[SLOW_WALK_KEYBIND].active)
			slowwalk::get().create_move(m_pcmd, g_cfg.misc.slowwalk_speed);
	}

	// fakelag code.
	if (!g_ctx.globals.should_recharge)
		fakelag::get().Createmove(m_pcmd, bSendPacket);

	// run our essentials.
	aimbot::get().think(m_pcmd);
	legit_bot::get().createmove(m_pcmd);

	// automatic fallback.
	misc::get().automatic_peek(m_pcmd, g_ctx.globals.original_viewangles.y);

	// anti-aim code.
	antiaim::get().create_move(m_pcmd, bSendPacket);

	// code about packet or something something.
	networking::get().packet_cycle(m_pcmd, bSendPacket);

	// run our auto revolver here, i don't know why.
	aimbot::get().AutoRevolver(m_pcmd);

	// end our prediction.
	engineprediction::get().finish(m_pcmd);

	// anti untrusted, prevent weird viewangles.
	if (g_cfg.misc.anti_untrusted)
		math::normalize_angles(m_pcmd->m_viewangles);
	else
	{
		m_pcmd->m_viewangles.y = math::normalize_yaw(m_pcmd->m_viewangles.y);
		m_pcmd->m_viewangles.z = 0.0f;
	}

	// fix our movement so we still walk facing our camera.
	util::movement_fix(g_ctx.globals.original_viewangles, m_pcmd);

	if (g_ctx.globals.should_recharge)
		bSendPacket = true;

	// we are not supposed to send another packet here.
	if (bSendPacket && g_ctx.globals.should_send_packet)
		g_ctx.globals.should_send_packet = false;

	// buy bot at last. 
	misc::get().buybot();

	if (bSendPacket)
		engineprediction::get().command_list.emplace_back(m_pcmd->m_command_number);

	// store our animation data for later usage in animation fix.
	local_animations::get().store_animations_data(m_pcmd, bSendPacket);

	// we are no longer in createmove.
	g_ctx.globals.in_createmove = false;

	verified->m_cmd = *m_pcmd;
	verified->m_crc = m_pcmd->GetChecksum();

	return;
}

__declspec(naked) void __stdcall hooks::hooked_createmove_naked(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		push ebx
		push esp
		push dword ptr[esp + 20]
		push dword ptr[esp + 0Ch + 8]
		push dword ptr[esp + 10h + 4]
		call hooked_createmove
		pop ebx
		retn 0Ch
	}
}

void __cdecl hooks::hooked_clmove(float_t frametime, bool final_tick)
{
	networking::get().start_network();

	if (g_ctx.globals.should_recharge)
	{
		++exploitsystem::get().m_shift_data.m_charge_amount;
		if (++g_ctx.globals.ticks_allowed >= 16)
			g_ctx.globals.should_recharge = false;

		return;
	}

	((ClMoveFn)original_clmove)(frametime, final_tick);

	// holy shit, uncharge (we simulate extra commands to do this).
	if (!g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].active && !g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].active) 
	{
		if (exploitsystem::get().m_shift_data.m_charge_amount > 0 && (!g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].active && !g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].active))
		{
			exploitsystem::get().m_shift_data.m_shifting = true;

			for (int i = exploitsystem::get().m_shift_data.m_charge_amount; i > 0; exploitsystem::get().m_shift_data.m_charge_amount--, i--)
				((ClMoveFn)original_clmove)(frametime, final_tick);
		}

		exploitsystem::get().m_shift_data.m_shifting = false;
		return;
	}
}
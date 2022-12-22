// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "networking.h"
#include "..\misc\misc.h"
#include "..\ragebot\aim.h"
#include "..\exploits\exploits.h"

inline bool is_vector_valid(Vector vecOriginal, Vector vecCurrent)
{
	Vector vecDelta = vecOriginal - vecCurrent;
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(vecDelta[i]) > 0.03125f)
			return false;
	}

	return true;
}

inline bool is_float_valid(float_t flOriginal, float_t flCurrent)
{
	if (fabsf(flOriginal - flCurrent) > 0.03125f)
		return false;

	return true;
}

void networking::store_netvar_data(int command_number)
{
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_nTickbase = g_ctx.local()->m_nTickBase();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flDuckAmount = g_ctx.local()->m_flDuckAmount();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flDuckSpeed = g_ctx.local()->m_flDuckSpeed();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_vecOrigin = g_ctx.local()->m_vecOrigin();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_vecVelocity = g_ctx.local()->m_vecVelocity();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_vecBaseVelocity = g_ctx.local()->m_vecBaseVelocity();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flFallVelocity = g_ctx.local()->m_flFallVelocity();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_vecViewOffset = g_ctx.local()->m_vecViewOffset();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_angAimPunchAngle = g_ctx.local()->m_aimPunchAngle();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_vecAimPunchAngleVel = g_ctx.local()->m_aimPunchAngleVel();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_angViewPunchAngle = g_ctx.local()->m_viewPunchAngle();

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();
	if (!weapon)
	{
		this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flRecoilIndex = 0.0f;
		this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flAccuracyPenalty = 0.0f;
		this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flPostponeFireReadyTime = 0.0f;

		return;
	}

	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flRecoilIndex = weapon->m_flRecoilIndex();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flAccuracyPenalty = weapon->m_fAccuracyPenalty();
	this->compress_data[command_number % MULTIPLAYER_BACKUP].m_flPostponeFireReadyTime = weapon->m_flPostponeFireReadyTime();
}

void networking::restore_netvar_data(int command_number)
{
	volatile auto netvars = &this->compress_data[command_number % MULTIPLAYER_BACKUP];

	if (netvars->m_nTickbase != g_ctx.local()->m_nTickBase())
		return;

	if (is_vector_valid(netvars->m_vecVelocity, g_ctx.local()->m_vecVelocity()))
		g_ctx.local()->m_vecVelocity() = netvars->m_vecVelocity;

	if (is_vector_valid(netvars->m_vecBaseVelocity, g_ctx.local()->m_vecBaseVelocity()))
		g_ctx.local()->m_vecBaseVelocity() = netvars->m_vecBaseVelocity;

	if (is_vector_valid(netvars->m_angAimPunchAngle, g_ctx.local()->m_aimPunchAngle()))
		g_ctx.local()->m_aimPunchAngle() = netvars->m_angAimPunchAngle;

	if (is_vector_valid(netvars->m_vecAimPunchAngleVel, g_ctx.local()->m_aimPunchAngleVel()))
		g_ctx.local()->m_aimPunchAngleVel() = netvars->m_vecAimPunchAngleVel;

	if (is_vector_valid(netvars->m_angViewPunchAngle, g_ctx.local()->m_viewPunchAngle()))
		g_ctx.local()->m_viewPunchAngle() = netvars->m_angViewPunchAngle;

	if (is_float_valid(netvars->m_flFallVelocity, g_ctx.local()->m_flFallVelocity()))
		g_ctx.local()->m_flFallVelocity() = netvars->m_flFallVelocity;

	if (is_float_valid(netvars->m_flDuckAmount, g_ctx.local()->m_flDuckAmount()))
		g_ctx.local()->m_flDuckAmount() = netvars->m_flDuckAmount;

	if (is_float_valid(netvars->m_flDuckSpeed, g_ctx.local()->m_flDuckSpeed()))
		g_ctx.local()->m_flDuckSpeed() = netvars->m_flDuckSpeed;

	if (g_ctx.local()->m_hActiveWeapon().Get())
	{
		if (is_float_valid(netvars->m_flAccuracyPenalty, g_ctx.local()->m_hActiveWeapon().Get()->m_fAccuracyPenalty()))
			g_ctx.local()->m_hActiveWeapon().Get()->m_fAccuracyPenalty() = netvars->m_flAccuracyPenalty;

		if (is_float_valid(netvars->m_flRecoilIndex, g_ctx.local()->m_hActiveWeapon().Get()->m_flRecoilIndex()))
			g_ctx.local()->m_hActiveWeapon().Get()->m_flRecoilIndex() = netvars->m_flRecoilIndex;
	}

	if (g_ctx.local()->m_fFlags() & FL_ONGROUND)
		g_ctx.local()->m_flFallVelocity() = 0.0f;
}

void networking::start_move(CUserCmd* m_pcmd, bool& bSendPacket)
{
	if (g_ctx.globals.ticks_allowed < 16 && (exploitsystem::get().double_tap_enabled && exploitsystem::get().double_tap_key || exploitsystem::get().hide_shots_enabled && exploitsystem::get().hide_shots_key))
		g_ctx.globals.should_recharge = true;

	g_ctx.globals.backup_tickbase = g_ctx.local()->m_nTickBase();

	if (g_ctx.globals.next_tickbase_shift)
		g_ctx.globals.fixed_tickbase = g_ctx.local()->m_nTickBase() - g_ctx.globals.next_tickbase_shift;
	else
		g_ctx.globals.fixed_tickbase = g_ctx.globals.backup_tickbase;

	if (hooks::menu_open && g_ctx.globals.focused_on_input)
	{
		m_pcmd->m_buttons = 0;
		m_pcmd->m_forwardmove = 0.0f;
		m_pcmd->m_sidemove = 0.0f;
		m_pcmd->m_upmove = 0.0f;
	}

	if (hooks::menu_open)
	{
		m_pcmd->m_buttons &= ~IN_ATTACK;
		m_pcmd->m_buttons &= ~IN_ATTACK2;
	}

	if (m_pcmd->m_buttons & IN_ATTACK2 && g_cfg.ragebot.enable && g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		m_pcmd->m_buttons &= ~IN_ATTACK2;

	if (g_cfg.ragebot.enable && !g_ctx.globals.weapon->can_fire(true))
	{
		if (m_pcmd->m_buttons & IN_ATTACK && !g_ctx.globals.weapon->is_non_aim() && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			m_pcmd->m_buttons &= ~IN_ATTACK;
		else if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && (g_ctx.globals.weapon->is_knife() || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_HEALTHSHOT)
		{
			if (m_pcmd->m_buttons & IN_ATTACK)
				m_pcmd->m_buttons &= ~IN_ATTACK;

			if (m_pcmd->m_buttons & IN_ATTACK2)
				m_pcmd->m_buttons &= ~IN_ATTACK2;
		}
	}

	if (m_pcmd->m_buttons & IN_FORWARD && m_pcmd->m_buttons & IN_BACK)
	{
		m_pcmd->m_buttons &= ~IN_FORWARD;
		m_pcmd->m_buttons &= ~IN_BACK;
	}

	if (m_pcmd->m_buttons & IN_MOVELEFT && m_pcmd->m_buttons & IN_MOVERIGHT)
	{
		m_pcmd->m_buttons &= ~IN_MOVELEFT;
		m_pcmd->m_buttons &= ~IN_MOVERIGHT;
	}

	g_ctx.globals.tickbase_shift = 0;
	g_ctx.globals.force_send_packet = false;
	g_ctx.globals.exploits = (g_cfg.keybinds.key[DOUBLE_TAP_KEYBIND].active && exploitsystem::get().double_tap_key) || (g_cfg.keybinds.key[HIDE_SHOTS_KEYBIND].active && exploitsystem::get().hide_shots_key);
	g_ctx.globals.current_weapon = g_ctx.globals.weapon->get_weapon_group(g_cfg.ragebot.enable);
	g_ctx.globals.original_forwardmove = math::clamp(m_pcmd->m_forwardmove, -g_ctx.convars.cl_forwardspeed->GetFloat(), g_ctx.convars.cl_forwardspeed->GetFloat());
	g_ctx.globals.original_sidemove = math::clamp(m_pcmd->m_sidemove, -g_ctx.convars.cl_sidespeed->GetFloat(), g_ctx.convars.cl_sidespeed->GetFloat());
	g_ctx.globals.original_viewangles = m_pcmd->m_viewangles;
	g_ctx.globals.slowwalking = false;
	
	antiaim::get().breaking_lby = false;

	misc::get().movement.in_forward = m_pcmd->m_buttons & IN_FORWARD;
	misc::get().movement.in_back = m_pcmd->m_buttons & IN_BACK;
	misc::get().movement.in_right = m_pcmd->m_buttons & IN_RIGHT;
	misc::get().movement.in_left = m_pcmd->m_buttons & IN_LEFT;
	misc::get().movement.in_moveright = m_pcmd->m_buttons & IN_MOVERIGHT;
	misc::get().movement.in_moveleft = m_pcmd->m_buttons & IN_MOVELEFT;
	misc::get().movement.in_jump = m_pcmd->m_buttons & IN_JUMP;
}

void networking::packet_cycle(CUserCmd* m_pcmd, bool& bSendPacket)
{
	if (m_gamerules()->m_bIsValveDS() && m_clientstate()->iChokedCommands >= 6) //-V648
	{
		bSendPacket = true;
		fakelag::get().started_peeking = false;
	}
	else if (!m_gamerules()->m_bIsValveDS() && m_clientstate()->iChokedCommands >= 16) //-V648
	{
		bSendPacket = true;
		fakelag::get().started_peeking = false;
	}

	if (g_ctx.globals.should_send_packet)
	{
		g_ctx.globals.force_send_packet = true;
		bSendPacket = true;
		fakelag::get().started_peeking = false;
	}

	if (g_ctx.globals.should_choke_packet)
	{
		g_ctx.globals.should_choke_packet = false;
		g_ctx.globals.should_send_packet = true;
		bSendPacket = false;
	}

	if (!g_ctx.globals.weapon->is_non_aim())
	{
		auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

		if (m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
		{
			if (g_cfg.ragebot.enable)
				m_pcmd->m_viewangles -= g_ctx.local()->m_aimPunchAngleScaled();

			if (!g_ctx.globals.fakeducking) {
				g_ctx.globals.force_send_packet = true;
				bSendPacket = true;
				fakelag::get().started_peeking = false;
			}

			g_ctx.globals.last_eye_pos = g_ctx.globals.eye_pos;
			g_ctx.globals.m_ragebot_shot_nr = m_pcmd->m_command_number;
		}
	}
	else if (g_ctx.globals.weapon->is_knife() && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2))
	{
		if (!g_ctx.globals.fakeducking) {
			g_ctx.globals.force_send_packet = true;
			bSendPacket = true;
			fakelag::get().started_peeking = false;
		}

		g_ctx.globals.m_ragebot_shot_nr = m_pcmd->m_command_number;
	}

	if (g_ctx.globals.fakeducking)
		g_ctx.globals.force_send_packet = bSendPacket;

	auto backup_ticks_allowed = g_ctx.globals.ticks_allowed;

	if (exploitsystem::get().double_tap(m_pcmd, bSendPacket))
		exploitsystem::get().hide_shots(m_pcmd, false);
	else
	{
		g_ctx.globals.ticks_allowed = backup_ticks_allowed;
		exploitsystem::get().hide_shots(m_pcmd, true);
	}
}

bool networking::setup_packet(int sequence_number, bool* pbSendPacket)
{
	CUserCmd* m_pcmd = m_input()->GetUserCmd(sequence_number);

	if (!m_pcmd || !m_pcmd->m_command_number)
		return false;

	if (!g_ctx.local()->is_alive())
		return false;

	g_ctx.send_packet = pbSendPacket;
	return true;
}

int networking::framerate()
{
	static auto framerate = 0.0f;
	framerate = 0.9f * framerate + 0.1f * m_globals()->m_absoluteframetime;

	if (framerate <= 0.0f)
		framerate = 1.0f;

	return (int)(1.0f / framerate);
}

void networking::build_seed_table()
{
	for (auto i = 0; i <= 255; i++) {
		math::random_seed(i + 1);

		const auto pi_seed = math::random_float(0.f, M_PI_2);

		computed_seeds.emplace_back(math::random_float(0.f, 1.f), pi_seed);
	}
}

void networking::on_packetend(CClientState* client_state)
{
	if (!g_ctx.available())
		return;

	if (!g_ctx.local()->is_alive())
		return;

	if (*(int*)((uintptr_t)client_state + 0x164) != *(int*)((uintptr_t)client_state + 0x16C))
		return;

	networking::get().restore_netvar_data(m_clientstate()->nLastCommandAck);
}

int networking::ping()
{
	if (!g_ctx.available())
		return 0;

	auto latency = m_engine()->IsPlayingDemo() ? 0.0f : this->flow_outgoing;

	if (latency)
		latency -= 0.5f / g_ctx.convars.cl_updaterate->GetFloat();

	return (int)(max(0.0f, latency) * 1000.0f);
}

float networking::tickrate()
{
	if (!m_engine()->IsConnected())
		return 0.0f;

	return (1.0f / m_globals()->m_intervalpertick);
}

int networking::server_tick()
{
	int extra_choke = 0;

	if (g_ctx.globals.fakeducking)
		extra_choke = 14 - m_clientstate()->iChokedCommands;

	return m_globals()->m_tickcount + TIME_TO_TICKS(this->latency) + extra_choke - g_ctx.globals.next_tickbase_shift;
}

void networking::start_network()
{
	if (!g_ctx.available())
		return;

	if (m_engine()->GetNetChannelInfo())
	{
		this->flow_outgoing = m_engine()->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		this->flow_incoming = m_engine()->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);

		this->average_outgoing = m_engine()->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING);
		this->average_incoming = m_engine()->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING);

		this->latency = this->flow_outgoing + this->flow_incoming;
	}
}

void networking::process_interpolation(ClientFrameStage_t Stage, bool bPostFrame)
{
	if (Stage != ClientFrameStage_t::FRAME_RENDER_START)
		return;

	if (!g_ctx.available() || !g_ctx.local()->is_alive())
		return;

	if (!bPostFrame)
	{
		this->final_predicted_tick = g_ctx.local()->m_nFinalPredictedTick();
		this->interp = m_globals()->m_interpolation_amount;

		g_ctx.local()->m_nFinalPredictedTick() = g_ctx.local()->m_nTickBase();

		if (g_ctx.globals.should_recharge || exploitsystem::get().m_shift_data.m_shifting)
			m_globals()->m_interpolation_amount = 0.0f;

		return;
	}

	g_ctx.local()->m_nFinalPredictedTick() = this->final_predicted_tick;
	m_globals()->m_interpolation_amount = 0.0f;
}

void networking::reset_data()
{
	this->latency = 0.f;
	this->flow_incoming = 0.f;
	this->flow_outgoing = 0.f;
	this->average_outgoing = 0.f;
	this->average_incoming = 0.f;
	this->interp = 0.f;

	this->final_predicted_tick = 0;

	std::memset(this->compress_data, 0, sizeof(this->compress_data));
}
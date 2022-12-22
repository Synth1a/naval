// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\ragebot\aim.h"
#include "fakelag.h"
#include "misc.h"
#include "prediction_system.h"
#include "logs.h"
#include "..\autowall\penetration.h"

void fakelag::Fakelag(CUserCmd* m_pcmd, bool& bSendPacket)
{
	if (g_cfg.antiaim.fakelag && !g_ctx.globals.exploits)
	{
		static auto force_choke = false;

		if (force_choke)
		{
			force_choke = false;
			bSendPacket = true;
			return;
		}

		if (engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && !(g_ctx.local()->m_fFlags() & FL_ONGROUND))
		{
			force_choke = true;
			bSendPacket = false;
			return;
		}
	}

	static auto fluctuate_ticks = 0;
	static auto switch_ticks = false;
	static auto random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);

	auto choked = m_clientstate()->iChokedCommands; //-V807
	auto flags = engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags; //-V807
	auto velocity = engineprediction::get().get_netvars(m_pcmd->m_command_number).m_vecVelocity.Length(); //-V807
	auto velocity2d = engineprediction::get().get_netvars(m_pcmd->m_command_number).m_vecVelocity.Length2D();

	auto max_speed = g_ctx.local()->get_max_player_speed();
	
	auto reloading = false;

	if (g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_RELOAD])
	{
		auto animlayer = g_ctx.local()->get_animlayers()[ANIMATION_LAYER_WEAPON_ACTION];

		if (animlayer.m_nSequence)
		{
			auto activity = g_ctx.local()->sequence_activity(animlayer.m_nSequence);
			reloading = activity == ACT_CSGO_RELOAD && animlayer.m_flWeight;
		}
	}

	switch (g_cfg.antiaim.fakelag_type)
	{
	case 0:
		this->max_choke = g_cfg.antiaim.triggers_fakelag_amount;
		break;
	case 1:
		this->max_choke = random_factor;
		break;
	case 2:
		if (velocity2d >= 5.0f)
		{
			auto dynamic_factor = std::ceilf(64.0f / (velocity2d * m_globals()->m_intervalpertick));

			if (dynamic_factor > 16)
				dynamic_factor = g_cfg.antiaim.triggers_fakelag_amount;

			this->max_choke = dynamic_factor;
		}
		else
			this->max_choke = g_cfg.antiaim.triggers_fakelag_amount;
		break;
	case 3:
		this->max_choke = fluctuate_ticks;
		break;
	}

	if (m_gamerules()->m_bIsValveDS()) //-V807
		this->max_choke = m_engine()->IsVoiceRecording() ? 1 : min(this->max_choke, 6);

	if (engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND && !m_gamerules()->m_bIsValveDS() && g_cfg.keybinds.key[FAKE_DUCK_KEYBIND].active) //-V807
	{
		this->max_choke = 14;

		if (choked < this->max_choke)
			bSendPacket = false;
		else
			bSendPacket = true;
	}
	else
	{
		if (g_cfg.ragebot.enable && g_ctx.globals.current_weapon != -1 && !g_ctx.globals.exploits && g_cfg.antiaim.fakelag && g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_PEEK] && g_cfg.antiaim.triggers_fakelag_amount > 6 && !this->started_peeking && velocity >= 5.0f)
		{
			if (aimbot::get().is_peeking_enemy((float)g_cfg.antiaim.triggers_fakelag_amount * 0.5f))
			{
				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
				this->started_peeking = true;

				return;
			}
		}

		if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag && !(engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND) && g_cfg.antiaim.fakelag_enablers[FAKELAG_IN_AIR])
		{
			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag && (engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND) && g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_LAND])
		{
			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag && g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_PEEK] && this->started_peeking)
		{
			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag && g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_SHOT] && (m_pcmd->m_buttons & IN_ATTACK) && g_ctx.globals.weapon->can_fire(true))
		{
			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag && g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_RELOAD] && reloading)
		{
			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag && g_cfg.antiaim.fakelag_enablers[FAKELAG_ON_VELOCITY_CHANGE] && abs(velocity - g_ctx.local()->m_vecVelocity().Length()) > 5.0f)
		{
			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.triggers_fakelag_amount : max(g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (!g_ctx.globals.exploits && g_cfg.antiaim.fakelag)
		{
			this->max_choke = g_cfg.antiaim.fakelag_amount;

			if (m_gamerules()->m_bIsValveDS())
				this->max_choke = min(this->max_choke, 6);

			if (choked < this->max_choke)
				bSendPacket = false;
			else
			{
				this->started_peeking = false;

				random_factor = min(rand() % 16 + 1, g_cfg.antiaim.fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? g_cfg.antiaim.fakelag_amount : max(g_cfg.antiaim.fakelag_amount - 2, 1);

				bSendPacket = true;
			}
		}
		else if (g_ctx.globals.exploits || !antiaim::get().condition(m_pcmd, false) && (antiaim::get().type == ANTIAIM_LEGIT || g_cfg.antiaim.type[antiaim::get().type].desync)) //-V648
		{
			this->condition = true;
			this->started_peeking = false;

			if (choked < 1)
				bSendPacket = false;
			else
				bSendPacket = true;
		}
		else
			this->condition = true;
	}

	if (this->force_ticks_allowed)
		return;

	return this->ForceTicksAllowedForProcessing(bSendPacket);
}

void fakelag::ForceTicksAllowedForProcessing(bool& bSendPacket)
{
	bSendPacket = false;
	if (m_clientstate()->iChokedCommands < 14)
		return;

	this->force_ticks_allowed = true;
}

void fakelag::SetMoveChokeClampLimit()
{
	unsigned long protect = 0;

	VirtualProtect((void*)g_ctx.addresses.clmove_choke_clamp, 4, PAGE_EXECUTE_READWRITE, &protect);
	*(uint32_t*)g_ctx.addresses.clmove_choke_clamp = 62;
	VirtualProtect((void*)g_ctx.addresses.clmove_choke_clamp, 4, protect, &protect);
}

void fakelag::Createmove(CUserCmd* m_pcmd, bool& bSendPacket)
{
	if (this->FakelagCondition(m_pcmd))
		return;

	Fakelag(m_pcmd, bSendPacket);
}

bool fakelag::FakelagCondition(CUserCmd* m_pcmd)
{
	this->condition = false;

	if (g_ctx.local()->m_bGunGameImmunity() || g_ctx.local()->m_fFlags() & FL_FROZEN)
		this->condition = true;

	if (antiaim::get().freeze_check)
		this->condition = true;

	return this->condition;
}

void fakelag::reset_data()
{
	this->condition = false;
	this->started_peeking = false;
	this->force_ticks_allowed = false;

	this->max_choke = 0;
}
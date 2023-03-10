// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "slowwalk.h"

#define FastSqrt(x)	(sqrt)(x)
#define Square(x) ((x)*(x))

void slowwalk::create_move(CUserCmd* m_pcmd, float custom_speed)
{
	if (!(engineprediction::get().get_netvars(m_pcmd->m_command_number).m_fFlags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND))
		return;

	auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	if (!g_cfg.misc.slowwalk_type)
	{
		if (custom_speed == -1.0f) //-V550
			g_ctx.globals.slowwalking = true;

		auto modifier = custom_speed == -1.0f ? 0.3f : custom_speed; //-V550
		auto max_speed = modifier * (g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);

		if (!g_ctx.globals.weapon->is_non_aim())
		{
			auto move_length = sqrt(m_pcmd->m_sidemove * m_pcmd->m_sidemove + m_pcmd->m_forwardmove * m_pcmd->m_forwardmove);

			auto forwardmove = m_pcmd->m_forwardmove / move_length;
			auto sidemove = m_pcmd->m_sidemove / move_length;

			if (move_length > max_speed)
			{
				if (max_speed + 1.0f < g_ctx.local()->m_vecVelocity().Length2D())
				{
					m_pcmd->m_forwardmove = 0.0f;
					m_pcmd->m_sidemove = 0.0f;
				}
				else
				{
					m_pcmd->m_sidemove = max_speed * sidemove;
					m_pcmd->m_forwardmove = max_speed * forwardmove;
				}
			}
		}
		else
		{
			auto forwardmove = m_pcmd->m_forwardmove;
			auto sidemove = m_pcmd->m_sidemove;

			auto move_length = sqrt(sidemove * sidemove + forwardmove * forwardmove);
			auto move_length_backup = move_length;

			if (move_length > 110.0f)
			{
				m_pcmd->m_forwardmove = forwardmove / move_length_backup * 110.0f;
				move_length = sidemove / move_length_backup * 110.0f;
				m_pcmd->m_sidemove = sidemove / move_length_backup * 110.0f;
			}
		}
	}
	else
	{
		if (custom_speed <= 0.f)
			return;

		float min_speed = (float)(FastSqrt(Square(m_pcmd->m_forwardmove) + Square(m_pcmd->m_sidemove) + Square(m_pcmd->m_upmove)));
		if (min_speed <= 0.f)
			return;

		g_ctx.globals.slowwalking = true;

		if (m_pcmd->m_buttons & IN_DUCK || g_ctx.globals.fakeducking)
			custom_speed *= 2.94117647f;

		if (min_speed <= custom_speed)
			return;

		float kys = custom_speed / min_speed;

		m_pcmd->m_forwardmove *= kys;
		m_pcmd->m_sidemove *= kys;
		m_pcmd->m_upmove *= kys;
	}
}
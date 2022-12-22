// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\ragebot\aim.h"

float build_server_abs_yaw(player_t* m_player, float angle)
{
	Vector velocity = m_player->m_vecVelocity();
	auto anim_state = m_player->get_animation_state();
	float m_flEyeYaw = angle;
	float m_flGoalFeetYaw = 0.f;

	float eye_feet_delta = math::angle_diff(m_flEyeYaw, m_flGoalFeetYaw);

	static auto get_smoothed_velocity = [](float min_delta, Vector a, Vector b) {
		Vector delta = a - b;
		float delta_length = delta.Length();

		if (delta_length <= min_delta)
		{
			Vector result;

			if (-min_delta <= delta_length)
				return a;
			else
			{
				float iradius = 1.0f / (delta_length + FLT_EPSILON);
				return b - ((delta * iradius) * min_delta);
			}
		}
		else
		{
			float iradius = 1.0f / (delta_length + FLT_EPSILON);
			return b + ((delta * iradius) * min_delta);
		}
	};

	float spd = velocity.LengthSqr();

	if (spd > std::powf(1.2f * 260.0f, 2.f))
	{
		Vector velocity_normalized = velocity.Normalized();
		velocity = velocity_normalized * (1.2f * 260.0f);
	}

	float m_flChokedTime = anim_state->m_flLastUpdateTime;
	float v25 = math::clamp(m_player->m_flDuckAmount() + anim_state->m_flDuckAdditional, 0.0f, 1.0f);
	float v26 = anim_state->m_flAnimDuckAmount;
	float v27 = m_flChokedTime * 6.0f;
	float v28;

	// clamp
	if ((v25 - v26) <= v27) {
		if (-v27 <= (v25 - v26))
			v28 = v25;
		else
			v28 = v26 - v27;
	}
	else {
		v28 = v26 + v27;
	}

	float flDuckAmount = math::clamp(v28, 0.0f, 1.0f);

	Vector animationVelocity = get_smoothed_velocity(m_flChokedTime * 2000.0f, velocity, m_player->m_vecVelocity());
	float speed = std::fminf(animationVelocity.Length(), 260.0f);

	float flMaxMovementSpeed = 260.0f;

	weapon_t* pWeapon = m_player->m_hActiveWeapon().Get();

	if (pWeapon && pWeapon->get_csweapon_info())
		flMaxMovementSpeed = std::fmaxf(pWeapon->get_csweapon_info()->flMaxPlayerSpeedAlt, 0.001f);

	float flRunningSpeed = speed / (flMaxMovementSpeed * 0.520f);
	float flDuckingSpeed = speed / (flMaxMovementSpeed * 0.340f);

	flRunningSpeed = std::clamp(flRunningSpeed, 0.0f, 1.0f);

	float flYawModifier = (((anim_state->m_flWalkToRunTransition * -0.30000001) - 0.19999999) * flRunningSpeed) + 1.0f;

	if (flDuckAmount > 0.0f)
	{
		float flDuckingSpeed = std::clamp(flDuckingSpeed, 0.0f, 1.0f);
		flYawModifier += (flDuckAmount * flDuckingSpeed) * (0.5f - flYawModifier);
	}

	const float v60 = -58.f;
	const float v61 = 58.f;

	float flMinYawModifier = v60 * flYawModifier;
	float flMaxYawModifier = v61 * flYawModifier;

	if (eye_feet_delta <= flMaxYawModifier)
	{
		if (flMinYawModifier > eye_feet_delta)
			m_flGoalFeetYaw = fabs(flMinYawModifier) + m_flEyeYaw;
	}
	else
	{
		m_flGoalFeetYaw = m_flEyeYaw - fabs(flMaxYawModifier);
	}

	m_flGoalFeetYaw = math::normalize_yaw(m_flGoalFeetYaw);

	if (speed > 0.1f || fabs(velocity.z) > 100.0f)
	{
		m_flGoalFeetYaw = math::approach_angle(
			m_flEyeYaw,
			m_flGoalFeetYaw,
			((anim_state->m_flWalkToRunTransition * 20.0f) + 30.0f)
			* m_flChokedTime);
	}
	else
	{
		m_flGoalFeetYaw = math::approach_angle(
			m_player->m_flLowerBodyYawTarget(),
			m_flGoalFeetYaw,
			m_flChokedTime * 100.0f);
	}

	return m_flGoalFeetYaw;
}

void resolver::Resolve(player_t* player, lagcompensation::LagRecord_t* record, lagcompensation::LagRecord_t* prev_record) {
	if (!g_cfg.ragebot.enable)
		return;

    if (!g_ctx.local()->is_alive())
        return;

	if (record->m_bIsFakePlayer)
		return;

	if (player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
		return;

	if (!record->m_pState)
		return;

	if (!prev_record)
		return;

	const auto info = animation_system::get().get_animation_info(record->m_pEntity);
	if (!info)
		return;

	AimPlayer* data = &aimbot::get().m_players[player->EntIndex()];

    // useful data to be used later.
    float speed = record->m_vecVelocity.Length2D();

    // resolver sides.
    int left = g_cfg.keybinds.key[RESOLVER_OVERRIDE_KEYBIND].active ? 1 : -1,
        right = g_cfg.keybinds.key[RESOLVER_OVERRIDE_KEYBIND].active ? -1 : 1;

    // lby resolver :flushed:.

	if (speed <= 0.1f)
	{
		if (record->m_pLayers[ANIMATION_LAYER_ADJUST].m_flWeight == 0.0f && record->m_pLayers[ANIMATION_LAYER_ADJUST].m_flCycle == 0.0f)
		{
			auto lby_delta = math::angle_diff(record->m_angEyeAngles.y - record->m_flLowerBodyYawTarget, 360.0f);
			info->m_iResolverSide = (2 * (lby_delta <= 0.0) - 1) ? right : left;
		}
	}
	// desync detection resolver.
	else if (prev_record && !int(record->m_pLayers[ANIMATION_LAYER_LEAN].m_flWeight * 1000.f))
	{
		// if our feet layer integer is still the same as the previous one then it's valid.
		if (int(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight * 1000.f) == int(prev_record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight * 1000.f))
		{
			// animlayers data.
			float delta_zero = std::fabsf(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->m_pResolveLayers[SAFEPOINT_CENTER][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate); // middle
			float delta_negative = std::fabsf(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->m_pResolveLayers[SAFEPOINT_LEFT][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate); // left
			float delta_positive = std::fabsf(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->m_pResolveLayers[SAFEPOINT_RIGHT][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate); // right

			// if right is greater than left.
			if (delta_zero < delta_positive || delta_negative <= delta_positive || int(delta_positive * 1000.f)) 
			{
				if (delta_zero >= delta_negative && delta_positive > delta_negative && !int(delta_negative * 1000.f))
					info->m_iResolverSide = right;
			}
			// if it's not.
			else
				info->m_iResolverSide = left;
		}
	}

    // bruteforce.
    switch (data->m_missed_shots % 3) {
    case 0: // default
		player->get_animation_state()->m_flFootYaw = record->m_angEyeAngles.y + 58.f * info->m_iResolverSide;
        break;
    case 1: // reverse
		player->get_animation_state()->m_flFootYaw = record->m_angEyeAngles.y + 58.f * -info->m_iResolverSide;
        break; 
    case 2: // middle
		player->get_animation_state()->m_flFootYaw = record->m_angEyeAngles.y;
        break;
    }
}
// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\ragebot\aim.h"

void resolver::SetMode(lagcompensation::LagRecord_t* record)
{
	// the resolver has 5 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player.
	if (!g_cfg.ragebot.enable || record->m_bIsFakePlayer || !g_ctx.local()->is_alive() || record->m_pEntity->m_iTeamNum() == g_ctx.local()->m_iTeamNum()) 
	{
		record->m_iResolveMode = Modes::RESOLVE_NONE;
		return;
	}

	// if not on ground.
	if (!(record->m_fFlags & FL_ONGROUND))
		record->m_iResolveMode = Modes::RESOLVE_AIR;

	// if sideways.
	else if (record->m_bIsLeft || record->m_bIsRight)
		record->m_iResolveMode = Modes::RESOLVE_SIDEWAYS;

	// if on ground and fast moving.
	else if (record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight > 0.75f)
		record->m_iResolveMode = Modes::RESOLVE_RUN;

	// if on ground and moving.
	else if (record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight > 0.f)
		record->m_iResolveMode = Modes::RESOLVE_WALK;

	// if on ground and not moving.
	else
		record->m_iResolveMode = Modes::RESOLVE_STAND;
}

void resolver::ResolveAngles(player_t* player, lagcompensation::LagRecord_t* record, lagcompensation::LagRecord_t* previous_record) {
	AimPlayer* data = &aimbot::get().m_players[player->EntIndex()];

	// next up mark this record with a resolver mode that will be used.
	this->SetMode(record);

	if (record->m_iResolveMode == Modes::RESOLVE_NONE)
	{
		record->m_iResolveSide = data->m_type = data->m_type_before_bruteforce = data->m_side = data->m_side_before_bruteforce = NULL;
		record->m_flResolveStrengh = data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 0.f;
		return;
	}

	if (!record->m_bDidShot) 
	{
		if (!data->m_missed_shots)
		{
			if (record->m_iResolveMode != Modes::RESOLVE_STAND)
			{
				if (record->m_iResolveMode != Modes::RESOLVE_RUN)
				{
					if (record->m_iResolveMode != Modes::RESOLVE_SIDEWAYS) 
					{
						if (record->m_iResolveMode != Modes::RESOLVE_AIR)
						{
							// default.
							data->m_type = data->m_type_before_bruteforce = 1;
							data->m_side = data->m_side_before_bruteforce = 1;
							data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 0.f;
						}
						else
						{
							// default.
							data->m_type = data->m_type_before_bruteforce = 2;
							data->m_side = data->m_side_before_bruteforce = 1;
							data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 0.f;
						}
					}
					else
					{
						// default.
						data->m_type = data->m_type_before_bruteforce = 3;
						data->m_side = data->m_side_before_bruteforce = 1;
						data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 0.f;
					}
				}
				else
				{
					// default.
					data->m_type = data->m_type_before_bruteforce = 4;
					data->m_side = data->m_side_before_bruteforce = 1;
					data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 0.f;
				}

				// animlayers.				
				// compare networked layer to processed layer w/ m_flFootYaw lerped with positive delta.
				float delta_positive = std::fabsf(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->m_pResolveLayers[SAFEPOINT_RIGHT][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate);

				// compare networked layer to processed layer w/ m_flFootYaw lerped with negative delta.
				float delta_negative = std::fabsf(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->m_pResolveLayers[SAFEPOINT_LEFT][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate);

				// compare networked layer to processed layer.
				float delta_zero = std::fabsf(record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->m_pResolveLayers[SAFEPOINT_CENTER][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate);

				// check if we have a previous record stored we can compare to, if we are currently running animations related to body lean and check if the movement speed (movement layer playback rate) of both records match.
				if (previous_record && !((int)record->m_pLayers[ANIMATION_LAYER_LEAN].m_flWeight * 1000.f) && ((int)record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight * 1000.f) == ((int)previous_record->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight * 1000.f)) 
				{
					if (delta_zero < delta_positive || delta_negative <= delta_positive || int(delta_positive * 1000.f)) 
					{
						if (delta_zero >= delta_negative && delta_positive > delta_negative && !int(delta_negative * 1000.f))
						{
							// success, record is desyncing towards positive yaw.
							data->m_type = data->m_type_before_bruteforce = 6;
							data->m_side_before_bruteforce = 1;
							data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 60.f;
						}
					}
					else 
					{
						// record is desyncing towards negative yaw.
						data->m_type = data->m_type_before_bruteforce = 6;
						data->m_side_before_bruteforce = -1;
						data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 60.f;
					}
				}

				// last hit.
				if (data->m_last_side_hit != 0) 
				{
					data->m_type = data->m_type_before_bruteforce = 7;
					data->m_side_before_bruteforce = data->m_last_side_hit;
					data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = data->m_last_resolve_strenght_hit;
				}
			}
			else
			{
				// default.
				data->m_type = data->m_type_before_bruteforce = 8;
				data->m_side_before_bruteforce = 1;
				data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 60.f;

				// delta.
				if (record->m_pLayers[ANIMATION_LAYER_ADJUST].m_flWeight == 0.f && record->m_pLayers[ANIMATION_LAYER_ADJUST].m_flCycle == 0.f) 
				{
					data->m_type = data->m_type_before_bruteforce = 9;
					data->m_side_before_bruteforce = 2 * int(record->m_angEyeAngles.y - record->m_flOriginalGoalFeetYaw >= 0.f) - 1;
					data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 60.f;
				}				
			}

			// low pitch.
			if (record->m_angEyeAngles.x < 75.f && data->m_type_before_bruteforce != 6) 
			{
				data->m_type = data->m_type_before_bruteforce = 10;
				data->m_side_before_bruteforce = 1;
				data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce = 0.f;
			}
		}
		else
			data->m_type = 11;

		if (record->m_iResolveMode == Modes::RESOLVE_RUN || record->m_iResolveMode == Modes::RESOLVE_AIR) 
		{
			if (data->m_type_before_bruteforce == 6 || data->m_type_before_bruteforce == 7 || data->m_type_before_bruteforce == 8 || data->m_type_before_bruteforce == 9)
			{
				switch (data->m_missed_shots % 3)
				{
				case 0:
					data->m_side = data->m_side_before_bruteforce;
					data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce;
					break;
				case 1:
					data->m_side = NULL;
					data->m_resolve_strenght = 0.f;
					break;
				case 2:
					data->m_side = -data->m_side_before_bruteforce;
					data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce;
					break;
				}
			}
			else {
				switch (data->m_missed_shots % 5) 
				{
				case 0:
					data->m_side = data->m_side_before_bruteforce;
					data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce;
					break;
				case 1:
					data->m_side = -data->m_side_before_bruteforce;
					data->m_resolve_strenght = 30.f;
					break;
				case 2:
					data->m_side = data->m_side_before_bruteforce;
					data->m_resolve_strenght = 30.f;
					break;
				case 3:
					data->m_side = data->m_side_before_bruteforce;
					data->m_resolve_strenght = 60.f;
					break;
				case 4:
					data->m_side = -data->m_side_before_bruteforce;
					data->m_resolve_strenght = 60.f;
					break;
				}
			}
		}
		else if (data->m_type_before_bruteforce == 8 || data->m_type_before_bruteforce == 9) 
		{
			switch (data->m_missed_shots % 3) {
			case 0:
				data->m_side = data->m_side_before_bruteforce;
				data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce;
				break;
			case 1:
				data->m_side = -data->m_side_before_bruteforce;
				data->m_resolve_strenght = 60.f;
				break;
			case 2:
				data->m_side = NULL;
				data->m_resolve_strenght = 0.f;
				break;
			}
		}
		else if (data->m_type_before_bruteforce == 6 || data->m_type_before_bruteforce == 7)
		{
			switch (data->m_missed_shots % 5) {
			case 0:
				data->m_side = data->m_side_before_bruteforce;
				data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce;
				break;
			case 1:
				data->m_side = -data->m_side_before_bruteforce;
				data->m_resolve_strenght = 60.f;
				break;
			case 2:
				data->m_side = data->m_side_before_bruteforce;
				data->m_resolve_strenght = 60.f;
				break;
			case 3:
				data->m_side = -data->m_side_before_bruteforce;
				data->m_resolve_strenght = 30.f;
				break;
			case 4:
				break;
			}
		}		
		else 
		{
			switch (data->m_missed_shots % 5) {
			case 0:
				data->m_side = data->m_side_before_bruteforce;
				data->m_resolve_strenght = data->m_resolve_strenght_before_bruteforce;
				break;
			case 1:
				data->m_side = -data->m_side_before_bruteforce;
				data->m_resolve_strenght = 60.f;
				break;
			case 2:
				data->m_side = data->m_side_before_bruteforce;
				data->m_resolve_strenght = 30.f;
				break;
			case 3:
				data->m_side = -data->m_side_before_bruteforce;
				data->m_resolve_strenght = 30.f;
				break;
			case 4:
				data->m_side = NULL;
				data->m_resolve_strenght = 0.f;
				break;
			}
		}
	}
	else 
	{
		data->m_type = 12;
		data->m_side = NULL;
		data->m_resolve_strenght = 0.f;
	}

	// save some info.
	record->m_iResolveType = data->m_type;
	record->m_iResolveSide = data->m_side;
	record->m_flResolveStrengh = data->m_resolve_strenght;
}
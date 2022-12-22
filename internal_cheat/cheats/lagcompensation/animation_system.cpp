// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\misc\prediction_system.h"
#include "..\networking\networking.h"

lagcompensation::LagRecord_t::LagRecord_t(player_t* pEntity) 
{
	// save data for players.
	const auto pWeapon = pEntity->m_hActiveWeapon().Get();

	// data that will be used later.
	this->m_pEntity = pEntity;
	m_iEntIndex = pEntity->EntIndex();
	m_bDormant = pEntity->IsDormant();
	m_bIsFakePlayer = pEntity->is_bot();
	m_vecVelocity = pEntity->m_vecVelocity();
	m_vecOrigin = pEntity->m_vecOrigin();
	m_vecAbsOrigin = pEntity->GetAbsOrigin();
	m_vecMins = pEntity->m_vecMins();
	m_vecMaxs = pEntity->m_vecMaxs();
	pEntity->copy_animlayers(m_pLayers);
	pEntity->copy_poseparameter(m_pPoses);
	m_flBackWardAngle = math::calculate_angle(m_vecOrigin, g_ctx.local()->m_vecOrigin()).y + 180.f;
	m_flLeftAngle = m_flBackWardAngle + 90.f;
	m_flRightAngle = m_flBackWardAngle - 90.f;
	m_bIsLeft = std::fabsf(m_angEyeAngles.y - m_flLeftAngle) < 45.f;
	m_bIsRight = std::fabsf(m_angEyeAngles.y - m_flRightAngle) < 45.f;
	m_pState = pEntity->get_animation_state() ? pEntity->get_animation_state() : nullptr;
	m_flSimulationTime = pEntity->m_flSimulationTime();
	m_flMaxBodyRotation = m_pState ? pEntity->get_max_desync_delta() : FLT_MAX;
	m_flInterpTime = 0.f;
	m_flLastShotTime = pWeapon ? pWeapon->m_fLastShotTime() : 0.f;
	m_flDuck = pEntity->m_flDuckAmount();
	m_flLowerBodyYawTarget = pEntity->m_flLowerBodyYawTarget();
	m_flOriginalGoalFeetYaw = m_pState ? m_pState->m_flFootYaw : 0.f;
	m_angEyeAngles = pEntity->m_angEyeAngles();
	m_angAbsAngles = pEntity->GetAbsAngles();
	m_fFlags = pEntity->m_fFlags();
	m_iEFlags = pEntity->m_iEFlags();
	m_iEffects = pEntity->m_fEffects();
	m_iChoked = TIME_TO_TICKS(m_flSimulationTime - pEntity->m_flOldSimulationTime());

	if (m_iChoked > 17)
		m_iChoked = 1;

	if (m_bIsFakePlayer || m_iChoked < 1)
		m_iChoked = 1;
}

lagcompensation::LagRecord_t::LagRecord_t(player_t* pEntity, Vector vecLastReliableAngle) : LagRecord_t(pEntity) 
{
	this->m_angLastReliableAngle = vecLastReliableAngle;
}

bool lagcompensation::LagRecord_t::IsValid(float flSimulationTime, float flRange)
{
	// use prediction curtime for this.
	float curtime = g_ctx.local()->is_alive() ? TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) : m_globals()->m_curtime;

	// correct is the amount of time we have to correct game time,
	float correct = util::get_interpolation() + networking::get().flow_outgoing;

	// stupid fake latency goes into the incoming latency.
	float in = networking::get().flow_incoming;
	correct += in;

	// check bounds [ 0, sv_maxunlag ]
	math::clamp(correct, 0.f, g_ctx.convars.sv_maxunlag->GetFloat());

	// calculate difference between tick sent by player and our latency based tick.
	// ensure this record isn't too old.
	return std::abs(correct - (curtime - flSimulationTime)) <= flRange - m_globals()->m_intervalpertick;
}

void lagcompensation::LagRecord_t::BuildBones(matrix3x4_t* matrix, int mask) 
{
	animation_system::get().m_CachedMatrixRetr[m_pEntity->EntIndex()] = m_pEntity->setup_bones(matrix);
	
	for (int i = NULL; i < MAXSTUDIOBONES; i++)
		animation_system::get().m_BoneOrigins[m_pEntity->EntIndex()][i] = m_pEntity->GetAbsOrigin() - matrix[i].GetOrigin();

	std::memcpy(animation_system::get().m_CachedMatrix[m_pEntity->EntIndex()], matrix, m_pEntity->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
}

void lagcompensation::LagRecord_t::Restore(player_t* player) 
{
	// restore player data before modifying it.
	player->m_vecVelocity() = m_vecVelocity;
	player->m_fFlags() = m_fFlags;
	player->m_iEFlags() = m_iEFlags;
	player->m_flDuckAmount() = m_flDuck;
	player->set_animlayers(m_pLayers);
	player->m_flLowerBodyYawTarget() = m_flLowerBodyYawTarget;
	player->m_vecOrigin() = m_vecOrigin;
	player->set_abs_origin(m_vecAbsOrigin);
}

void lagcompensation::LagRecord_t::Apply(player_t* player)
{
	// apply data when using lagcompensation (and apply the stored data with this too).
	player->set_poseparameter(m_pPoses);
	player->m_angEyeAngles() = m_angEyeAngles;
	player->m_vecVelocity() = player->m_vecAbsVelocity() = m_vecVelocity;
	player->m_flLowerBodyYawTarget() = m_flLowerBodyYawTarget;
	player->m_flDuckAmount() = m_flDuck;
	player->m_fFlags() = m_fFlags;
	player->m_vecOrigin() = m_vecOrigin;
	player->set_abs_origin(m_vecOrigin);

	// set animstate to our state since we want to use lagcomp data instead of client data.
	if (player->get_animation_state())
		player->set_animation_state(m_pState);
}

void lagcompensation::PostPlayerUpdate() 
{
	for (auto it = animation_system::get().m_ulAnimationInfo.begin(); it != animation_system::get().m_ulAnimationInfo.end();) {
		auto player = (player_t*)m_entitylist()->GetClientEntityFromHandle(it->first);

		if (!player->valid(false) || player != it->second.m_pEntity)
		{
			if (player)
				g_ctx.animations.m_update_animations = player->m_bClientSideAnimation() = true;

			it = animation_system::get().m_ulAnimationInfo.erase(it);
		}
		else
			it = next(it);
	}

	for (auto i = 1; i < m_globals()->m_maxclients; ++i) {
		const auto entity = (player_t*)m_entitylist()->GetClientEntity(i);

		if (!entity->valid(false, false))
			continue;

		if (entity == g_ctx.local())
			continue;

		if (entity->IsDormant()) {
			animation_system::get().m_bLeftDormancy[i] = true;
			continue;
		}

		if (animation_system::get().m_ulAnimationInfo.find(entity->GetRefEHandle().ToLong()) == animation_system::get().m_ulAnimationInfo.end())
			animation_system::get().m_ulAnimationInfo.insert_or_assign(entity->GetRefEHandle().ToLong(), animation_system::AnimationInfo_t(entity, {}));
	}

	for (auto& ulAnimInfo : animation_system::get().m_ulAnimationInfo) {
		animation_system::AnimationInfo_t& pRecord = ulAnimInfo.second;
		const auto pEntity = pRecord.m_pEntity;
	
		for (auto i = pRecord.m_pRecords.rbegin(); i != pRecord.m_pRecords.rend();) 
		{
			if (m_globals()->m_curtime - i->m_flSimulationTime > 1.2f)
				i = decltype(i) { ulAnimInfo.second.m_pRecords.erase(next(i).base()) };
			else
				i = next(i);
		}

		// nice anti-exploits method, lol.
		// oh no, their old simtime is higher than their current simtime, that means they are using exploits!.

		if (pEntity->m_flOldSimulationTime() > pEntity->m_flSimulationTime())
		{
			if (!pRecord.m_pRecords.empty())
				pRecord.m_pRecords.clear();

			continue;
		}

		if (pRecord.m_flLastSpawnTime != pEntity->m_flSpawnTime()) 
		{
			const auto state = pEntity->get_animation_state();
			if (state) 
				util::reset_state(state);
			
			pRecord.m_flLastSpawnTime = pEntity->m_flSpawnTime();
		}

		// they are breaking lagcompensation, remove all records.
		if (!pRecord.m_pRecords.empty() && (pEntity->m_vecOrigin() - pRecord.m_pRecords.front().m_vecOrigin).LengthSqr() > 4096.f)
			pRecord.m_pRecords.clear();

		const auto pWeapon = pEntity->m_hActiveWeapon().Get();

		lagcompensation::LagRecord_t pBackupRecord = lagcompensation::LagRecord_t(pEntity);
		pBackupRecord.Apply(pEntity);

		lagcompensation::LagRecord_t* pPreviousRecord = nullptr;

		if (!pRecord.m_pRecords.empty()) {
			if (pRecord.m_pRecords.front().m_bDormant)
				animation_system::get().m_bLeftDormancy[pEntity->EntIndex()] = true;
			else 
			{
				animation_system::get().m_bLeftDormancy[pEntity->EntIndex()] = false;

				if (TIME_TO_TICKS(pEntity->m_flSimulationTime() - pRecord.m_pRecords.front().m_flSimulationTime) <= 17)
				{
					pPreviousRecord = &pRecord.m_pRecords.front();
					pRecord.m_PreviousRecord = pRecord.m_pRecords.front();
				}
			}
		}

		const bool bShot = pWeapon && pPreviousRecord && pWeapon->m_fLastShotTime() > pPreviousRecord->m_flSimulationTime && pWeapon->m_fLastShotTime() <= pEntity->m_flSimulationTime();

		if (!bShot) {
			ulAnimInfo.second.m_vecLastReliableAngle = pEntity->m_angEyeAngles();
		}

		auto& pCurrentRecord = pRecord.m_pRecords.emplace_front(pEntity, ulAnimInfo.second.m_vecLastReliableAngle);

		if (pPreviousRecord) {
			if (pPreviousRecord->m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flCycle == pCurrentRecord.m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flCycle)
				continue;

			pCurrentRecord.m_iChoked = TIME_TO_TICKS(pCurrentRecord.m_flSimulationTime - pPreviousRecord->m_flSimulationTime);

			if (pCurrentRecord.m_iChoked > 17)
				pCurrentRecord.m_iChoked = 1;

			if (pCurrentRecord.m_bIsFakePlayer || pCurrentRecord.m_iChoked < 1)
				pCurrentRecord.m_iChoked = 1;

			float flCurrentCycle = pCurrentRecord.m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flCycle;
			float flPreviousCycle = pPreviousRecord->m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flCycle;

			if (flCurrentCycle != flPreviousCycle) {
				float flCurrentRate = pCurrentRecord.m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;
				float flPreviousRate = pPreviousRecord->m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;

				if (flCurrentRate == flPreviousRate) {
					if (flPreviousCycle > flCurrentCycle)
						flCurrentCycle += 1.f;

					int iCycleTiming = TIME_TO_TICKS(flCurrentCycle - flPreviousCycle);
					if (iCycleTiming <= 17 && iCycleTiming > pCurrentRecord.m_iChoked)
						pCurrentRecord.m_iChoked = iCycleTiming;
				}
			}

			if (!(pCurrentRecord.m_fFlags & FL_ONGROUND))
			{
				// fix velocity.
				// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
				pCurrentRecord.m_vecVelocity = (pCurrentRecord.m_vecOrigin - pPreviousRecord->m_vecOrigin) / pCurrentRecord.m_iChoked;

				float flWeight = 1.f - pCurrentRecord.m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flWeight;

				if (flWeight > 0.f) {
					float flCurrentRate = pCurrentRecord.m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;
					float flPreviousRate = pPreviousRecord->m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;

					if (flCurrentRate == flPreviousRate) {
						int iCurrentSequence = pCurrentRecord.m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_nSequence;
						int iPreviousSequence = pPreviousRecord->m_pLayers[ANIMATION_LAYER_ALIVELOOP].m_nSequence;

						if (iCurrentSequence == iPreviousSequence) {
							float flSpeedNormalized = flWeight * 0.34999996f + 0.55f;

							if (flSpeedNormalized > 0.f) {
								float flMaxSpeed = 260.f;
						
								// get player weapon.
								auto m_weapon = pEntity->m_hActiveWeapon().Get();

								if (m_weapon)
								{
									// get wpninfo.
									auto m_weapon_info = m_weapon->get_csweapon_info();

									// get the max possible speed.
									if (m_weapon_info)
										flMaxSpeed = (pEntity->m_bIsScoped() ? m_weapon_info->flMaxPlayerSpeedAlt : m_weapon_info->flMaxPlayerSpeed);
								}

								float flSpeed = flSpeedNormalized * flMaxSpeed;
								if (pCurrentRecord.m_vecVelocity.Length2D() > 0.f && flSpeed > 0.f) {
									pCurrentRecord.m_vecVelocity.x /= pCurrentRecord.m_vecVelocity.Length2D() / flSpeed;
									pCurrentRecord.m_vecVelocity.y /= pCurrentRecord.m_vecVelocity.Length2D() / flSpeed;
								}
							}
						}
					}
				}

				pCurrentRecord.m_vecVelocity.z -= g_ctx.convars.sv_gravity->GetFloat() * 0.5f * TICKS_TO_TIME(pCurrentRecord.m_iChoked);
			}
			else
				pCurrentRecord.m_vecVelocity.z = 0.f;

			// update shot info.
			pCurrentRecord.m_bDidShot = pCurrentRecord.m_flLastShotTime > pPreviousRecord->m_flSimulationTime && pCurrentRecord.m_flLastShotTime <= pCurrentRecord.m_flSimulationTime;
		}

		animation_system::get().UpdateSafeAnimation(pEntity, pCurrentRecord.m_pMatrix.left, pCurrentRecord.m_pResolveLayers[SAFEPOINT_LEFT], pCurrentRecord.m_angEyeAngles.y - 58.f);
		animation_system::get().UpdateSafeAnimation(pEntity, pCurrentRecord.m_pMatrix.center, pCurrentRecord.m_pResolveLayers[SAFEPOINT_CENTER], pCurrentRecord.m_angEyeAngles.y);
		animation_system::get().UpdateSafeAnimation(pEntity, pCurrentRecord.m_pMatrix.right, pCurrentRecord.m_pResolveLayers[SAFEPOINT_RIGHT], pCurrentRecord.m_angEyeAngles.y + 58.f);

		resolver::get().ResolveAngles(pEntity, &pCurrentRecord, pPreviousRecord);

		pRecord.UpdateAnimations(&pCurrentRecord, pPreviousRecord);

		pEntity->set_animlayers(pBackupRecord.m_pLayers);

		pCurrentRecord.BuildBones(pCurrentRecord.m_pMatrix.main, BONE_USED_BY_ANYTHING);

		pBackupRecord.Restore(pEntity);
		pRecord.m_LatestRecord = pCurrentRecord;
	}
}


std::optional<lagcompensation::LagRecord_t*> lagcompensation::GetLatestRecord(player_t* pEntity) {
	const auto pInfo = animation_system::get().m_ulAnimationInfo.find(pEntity->GetRefEHandle().ToLong());
	if (pInfo == animation_system::get().m_ulAnimationInfo.end() || pInfo->second.m_pRecords.empty()) {
		return std::nullopt;
	}

	LagRecord_t* first_invalid = nullptr;

	for (auto it = pInfo->second.m_pRecords.begin(); it != pInfo->second.m_pRecords.end(); it = next(it)) {

		if (!first_invalid)
			first_invalid = &*it;

		if (it->IsValid(it->m_flSimulationTime) && TIME_TO_TICKS(fabsf(it->m_flSimulationTime - pEntity->m_flSimulationTime())) < 25)
			return &*it;
	}

	if (first_invalid)
		return first_invalid;
	else
		return std::nullopt;
}

std::optional<lagcompensation::LagRecord_t*> lagcompensation::GetOldestRecord(player_t* pEntity) {
	const auto pInfo = animation_system::get().m_ulAnimationInfo.find(pEntity->GetRefEHandle().ToLong());
	if (pInfo == animation_system::get().m_ulAnimationInfo.end() || pInfo->second.m_pRecords.empty()) {
		return std::nullopt;
	}

	for (auto it = pInfo->second.m_pRecords.rbegin(); it != pInfo->second.m_pRecords.rend(); it = next(it)) {
		if (it->IsValid(it->m_flSimulationTime) && TIME_TO_TICKS(fabsf(it->m_flSimulationTime - pEntity->m_flSimulationTime())) < 25)
			return &*it;
	}

	return std::nullopt;
}

void animation_system::AnimationInfo_t::UpdateAnimations(lagcompensation::LagRecord_t* pRecord, lagcompensation::LagRecord_t* pPreviousRecord) {
	// make a backup of globals.
	const float realtime = m_globals()->m_realtime;
	const float curtime = m_globals()->m_curtime;
	const float abs_frametime = m_globals()->m_absoluteframetime;
	const float frametime = m_globals()->m_frametime;
	const int frame = m_globals()->m_framecount;
	const int tick_count = m_globals()->m_tickcount;
	const float interp_amt = m_globals()->m_interpolation_amount;

	// backup stuff that we do not want to fuck with.
	float flLowerBodyYaw = m_pEntity->m_flLowerBodyYawTarget();
	float flDuckAmount = m_pEntity->m_flDuckAmount();
	int fFlags = m_pEntity->m_fFlags();
	int iEFlags = m_pEntity->m_iEFlags();

	if (animation_system::get().m_bLeftDormancy[pRecord->m_iEntIndex]) {
		float flLastUpdateTime = pRecord->m_flSimulationTime - m_globals()->m_intervalpertick;

		if (m_pEntity->m_fFlags() & FL_ONGROUND) {
			m_pEntity->get_animation_state()->m_bLanding = false;
			m_pEntity->get_animation_state()->m_bOnGround = true;

			float flLandTime = 0.f;
			if (pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flCycle > 0.f && pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flPlaybackRate > 0.f) {
				int iLandActivity = m_pEntity->sequence_activity(pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_nSequence);

				if (iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY) {
					flLandTime = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flCycle / pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flPlaybackRate;

					if (flLandTime > 0.f)
						flLastUpdateTime = pRecord->m_flSimulationTime - flLandTime;
				}
			}

			pRecord->m_vecVelocity.z = 0.f;
		}
		else {
			float flJumpTime = 0.f;
			if (pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle > 0.f && pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate > 0.f) {
				int iJumpActivity = m_pEntity->sequence_activity(pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_nSequence);

				if (iJumpActivity == ACT_CSGO_JUMP) {
					flJumpTime = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle / pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate;

					if (flJumpTime > 0.f)
						flLastUpdateTime = pRecord->m_flSimulationTime - flJumpTime;
				}
			}

			m_pEntity->get_animation_state()->m_bOnGround = false;
			m_pEntity->get_animation_state()->m_flDurationInAir = flJumpTime - m_globals()->m_intervalpertick;
		}

		float flWeight = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight;
		if (pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate < 0.00001f)
			pRecord->m_vecVelocity = ZERO;
		else if (flWeight > 0.f && flWeight < 0.95f) {
			float flMaxSpeed = 260.f;
			// get player weapon.
			auto m_weapon = m_pEntity->m_hActiveWeapon().Get();

			if (m_weapon)
			{
				// get wpninfo.
				auto m_weapon_info = m_weapon->get_csweapon_info();

				// get the max possible speed whilest player are still accurate.
				if (m_weapon_info)
					flMaxSpeed = (m_pEntity->m_bIsScoped() ? m_weapon_info->flMaxPlayerSpeedAlt : m_weapon_info->flMaxPlayerSpeed);
			}

			float flPostVelocityLength = pRecord->m_vecVelocity.Length();

			if (flPostVelocityLength > 0.f) {
				float flMaxSpeedMultiply = 1.f;

				// FL_DUCKING & FL_AIMDUCKING - fully ducked.
				// !FL_DUCKING & !FL_AIMDUCKING - fully unducked.
				// FL_DUCKING & !FL_AIMDUCKING - previously fully ducked, unducking in progress.
				// !FL_DUCKING & FL_AIMDUCKING - previously fully unducked, ducking in progress.
				if (m_pEntity->m_fFlags() & (FL_DUCKING | FL_AIMDUCKING))
					flMaxSpeedMultiply = 0.34f;
				else if (m_pEntity->m_bIsWalking())
					flMaxSpeedMultiply = 0.52f;

				pRecord->m_vecVelocity.x = (pRecord->m_vecVelocity.x / flPostVelocityLength) * (flWeight * (flMaxSpeed * flMaxSpeedMultiply));
				pRecord->m_vecVelocity.y = (pRecord->m_vecVelocity.y / flPostVelocityLength) * (flWeight * (flMaxSpeed * flMaxSpeedMultiply));
			}
		}

		m_pEntity->get_animation_state()->m_flLastUpdateTime = flLastUpdateTime;
	}

	if (pPreviousRecord) {
		m_pEntity->get_animation_state()->m_flStrafeChangeCycle = pPreviousRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_flCycle;
		m_pEntity->get_animation_state()->m_flStrafeChangeWeight = pPreviousRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_flWeight;
		m_pEntity->get_animation_state()->m_nStrafeSequence = pPreviousRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_nSequence;
		m_pEntity->get_animation_state()->m_flPrimaryCycle = pPreviousRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flCycle;
		m_pEntity->get_animation_state()->m_flMoveWeight = pPreviousRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight;
		m_pEntity->get_animation_state()->m_flAccelerationWeight = pPreviousRecord->m_pLayers[ANIMATION_LAYER_LEAN].m_flWeight;
		m_pEntity->set_animlayers(pPreviousRecord->m_pLayers);
	}
	else {
		m_pEntity->get_animation_state()->m_flStrafeChangeCycle = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_flCycle;
		m_pEntity->get_animation_state()->m_flStrafeChangeWeight = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_flWeight;
		m_pEntity->get_animation_state()->m_nStrafeSequence = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_nSequence;
		m_pEntity->get_animation_state()->m_flPrimaryCycle = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flCycle;
		m_pEntity->get_animation_state()->m_flMoveWeight = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight;
		m_pEntity->get_animation_state()->m_flAccelerationWeight = pRecord->m_pLayers[ANIMATION_LAYER_LEAN].m_flWeight;
		m_pEntity->set_animlayers(pRecord->m_pLayers);
	}

	if (pPreviousRecord && pRecord->m_iChoked > 1) {
		int iActivityTick = 0;
		int iActivityType = 0;

		if (pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flWeight > 0.f && pPreviousRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flWeight <= 0.f) {
			int iLandSequence = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_nSequence;

			if (iLandSequence > 2) {
				int iLandActivity = m_pEntity->sequence_activity(iLandSequence);

				if (iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY) {
					float flCurrentCycle = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flCycle;
					float flCurrentRate = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flPlaybackRate;

					if (flCurrentCycle > 0.f && flCurrentRate > 0.f) {
						float flLandTime = (flCurrentCycle / flCurrentRate);

						if (flLandTime > 0.f) {
							iActivityTick = TIME_TO_TICKS(pRecord->m_flSimulationTime - flLandTime) + 1;
							iActivityType = ACTIVITY_LAND;
						}
					}
				}
			}
		}

		if (pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle > 0.f && pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate > 0.f) {
			int iJumpSequence = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_nSequence;

			if (iJumpSequence > 2) {
				int iJumpActivity = m_pEntity->sequence_activity(iJumpSequence);

				if (iJumpActivity == ACT_CSGO_JUMP) {
					float flCurrentCycle = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle;
					float flCurrentRate = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate;

					if (flCurrentCycle > 0.f && flCurrentRate > 0.f) {
						float flJumpTime = (flCurrentCycle / flCurrentRate);

						if (flJumpTime > 0.f) {
							iActivityTick = TIME_TO_TICKS(pRecord->m_flSimulationTime - flJumpTime) + 1;
							iActivityType = ACTIVITY_JUMP;
						}
					}
				}
			}
		}

		for (int m_iSimulationTick = 1; m_iSimulationTick <= pRecord->m_iChoked; m_iSimulationTick++) {
			float m_flSimulationTime = pPreviousRecord->m_flSimulationTime + TICKS_TO_TIME(m_iSimulationTick);

			// fixes for networked players.
			m_globals()->m_curtime = m_globals()->m_realtime = m_flSimulationTime;
			m_globals()->m_frametime = m_globals()->m_absoluteframetime = m_globals()->m_intervalpertick;
			m_globals()->m_framecount = m_globals()->m_tickcount = TIME_TO_TICKS(m_globals()->m_realtime);
			m_globals()->m_interpolation_amount = 0.f;

			// lerp duck amt.
			m_pEntity->m_flDuckAmount() = math::interpolate(pPreviousRecord->m_flDuck, pRecord->m_flDuck, m_iSimulationTick, pRecord->m_iChoked);

			// lerp velocity.
			m_pEntity->m_vecVelocity() = m_pEntity->m_vecAbsVelocity() = math::interpolate(pPreviousRecord->m_vecVelocity, pRecord->m_vecVelocity, m_iSimulationTick, pRecord->m_iChoked);

			if (m_iSimulationTick < pRecord->m_iChoked)
			{
				int iCurrentSimulationTick = TIME_TO_TICKS(m_flSimulationTime);

				if (iActivityType > ACTIVITY_NONE) {
					bool bIsOnGround = m_pEntity->m_fFlags() & FL_ONGROUND;

					if (iActivityType == ACTIVITY_JUMP) {
						if (iCurrentSimulationTick == iActivityTick - 1)
							bIsOnGround = true;
						else if (iCurrentSimulationTick == iActivityTick)
						{
							// reset animation layer.
							m_pEntity->get_animlayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle = 0.f;
							m_pEntity->get_animlayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_nSequence = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_nSequence;
							m_pEntity->get_animlayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate;

							// reset player ground state.
							bIsOnGround = false;
						}

					}
					else if (iActivityType == ACTIVITY_LAND) {
						if (iCurrentSimulationTick == iActivityTick - 1)
							bIsOnGround = false;
						else if (iCurrentSimulationTick == iActivityTick)
						{
							// reset animation layer.
							m_pEntity->get_animlayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flCycle = 0.f;
							m_pEntity->get_animlayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_nSequence = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_nSequence;
							m_pEntity->get_animlayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flPlaybackRate = pRecord->m_pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flPlaybackRate;

							// reset player ground state.
							bIsOnGround = true;
						}
					}

					if (bIsOnGround)
						m_pEntity->m_fFlags() |= FL_ONGROUND;
					else
						m_pEntity->m_fFlags() &= ~FL_ONGROUND;
				}

				// rotate player.
				if (pRecord->m_iResolveMode != resolver::Modes::RESOLVE_NONE) {
					m_pEntity->get_animation_state()->m_flFootYaw = math::normalize_yaw(pRecord->m_angEyeAngles.y + (pRecord->m_iResolveSide * pRecord->m_flResolveStrengh));
				}
			}
			else
			{
				// set stuff before animating.
				m_pEntity->m_vecVelocity() = m_pEntity->m_vecAbsVelocity() = pRecord->m_vecVelocity;
				m_pEntity->m_flDuckAmount() = pRecord->m_flDuck;
				m_pEntity->m_fFlags() = pRecord->m_fFlags;
			}

			// fix animating in same frame.
			if (m_pEntity->get_animation_state()->m_nLastUpdateFrame >= m_globals()->m_framecount)
				m_pEntity->get_animation_state()->m_nLastUpdateFrame = m_globals()->m_framecount - 1;

			// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
			bool bClientSideAnimation = m_pEntity->m_bClientSideAnimation();
			m_pEntity->m_bClientSideAnimation() = true;

			for (int i = NULL; i < 13; i++)
				m_pEntity->get_animlayers()[i].m_pOwner = m_pEntity;

			g_ctx.animations.m_update_animations = true;
			m_pEntity->update_clientside_animation();
			g_ctx.animations.m_update_animations = false;

			m_pEntity->m_bClientSideAnimation() = bClientSideAnimation;
		}
	}
	else
	{
		// fixes for networked players.
		m_globals()->m_curtime = m_globals()->m_realtime = pRecord->m_flSimulationTime;
		m_globals()->m_frametime = m_globals()->m_absoluteframetime = m_globals()->m_intervalpertick;
		m_globals()->m_framecount = m_globals()->m_tickcount = TIME_TO_TICKS(m_globals()->m_realtime);
		m_globals()->m_interpolation_amount = 0.f;

		// set velocity.
		m_pEntity->m_vecVelocity() = m_pEntity->m_vecAbsVelocity() = pRecord->m_vecVelocity;

		if (pRecord->m_iResolveMode != resolver::Modes::RESOLVE_NONE) {
			m_pEntity->get_animation_state()->m_flFootYaw = math::normalize_yaw(pRecord->m_angEyeAngles.y + (pRecord->m_iResolveSide * pRecord->m_flResolveStrengh));
		}

		// fix animating in same frame.
		if (m_pEntity->get_animation_state()->m_nLastUpdateFrame >= m_globals()->m_framecount)
			m_pEntity->get_animation_state()->m_nLastUpdateFrame = m_globals()->m_framecount - 1;

		// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
		bool bClientSideAnimation = m_pEntity->m_bClientSideAnimation();
		m_pEntity->m_bClientSideAnimation() = true;

		for (int i = NULL; i < 13; i++)
			m_pEntity->get_animlayers()[i].m_pOwner = m_pEntity;

		g_ctx.animations.m_update_animations = true;
		m_pEntity->update_clientside_animation();
		g_ctx.animations.m_update_animations = false;

		m_pEntity->m_bClientSideAnimation() = bClientSideAnimation;
	}

	// restore backup data.
	m_pEntity->m_flLowerBodyYawTarget() = flLowerBodyYaw;
	m_pEntity->m_flDuckAmount() = flDuckAmount;
	m_pEntity->m_iEFlags() = iEFlags;
	m_pEntity->m_fFlags() = fFlags;

	// restore globals.
	m_globals()->m_curtime = curtime;
	m_globals()->m_realtime = realtime;
	m_globals()->m_frametime = frametime;
	m_globals()->m_absoluteframetime = abs_frametime;
	m_globals()->m_framecount = frame;
	m_globals()->m_tickcount = tick_count;
	m_globals()->m_interpolation_amount = interp_amt;

	// invalidate physics.
	m_pEntity->invalidate_physics_recursive(ANIMATION_CHANGED);
}

void animation_system::UpdateSafeAnimation(player_t* pEntity, matrix3x4_t* matrix, AnimationLayer* layers, float angles)
{
	if (!g_cfg.ragebot.enable)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	if (pEntity->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
		return;

	if (pEntity->is_bot())
		return;

	const auto backup_state = pEntity->get_animation_state();

	pEntity->get_animation_state()->m_flFootYaw = math::normalize_yaw(angles);

	bool bClientSideAnimation = pEntity->m_bClientSideAnimation();
	pEntity->m_bClientSideAnimation() = true;

	for (int iLayer = 0; iLayer < 13; ++iLayer)
		pEntity->get_animlayers()[iLayer].m_pOwner = pEntity;

	g_ctx.animations.m_update_animations = true;
	pEntity->update_clientside_animation();
	g_ctx.animations.m_update_animations = false;

	pEntity->m_bClientSideAnimation() = bClientSideAnimation;

	pEntity->copy_animlayers(layers);
	pEntity->setup_bones(matrix, true);
	pEntity->set_animation_state(backup_state);
}

void animation_system::get_cached_matrix(player_t* player, matrix3x4_t* matrix)
{
	if (this->m_CachedMatrixRetr[player->EntIndex()])
		std::memcpy(matrix, this->m_CachedMatrix[player->EntIndex()], player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
}

void animation_system::on_update_clientside_animation(player_t* player)
{
	for (int i = NULL; i < MAXSTUDIOBONES; i++)
		this->m_CachedMatrix[player->EntIndex()][i].SetOrigin(player->GetAbsOrigin() - this->m_BoneOrigins[player->EntIndex()][i]);

	std::memcpy(player->m_CachedBoneData().Base(), this->m_CachedMatrix[player->EntIndex()], player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
	std::memcpy(player->m_BoneAccessor()->GetBoneArrayForWrite(), this->m_CachedMatrix[player->EntIndex()], player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	return player->attachment_helper();
}

animation_system::AnimationInfo_t* animation_system::get_animation_info(player_t* player) {
	auto pInfo = this->m_ulAnimationInfo.find(player->GetRefEHandle().ToLong());
	if (pInfo == this->m_ulAnimationInfo.end()) 
		return nullptr;
	
	return &pInfo->second;
}

void animation_system::clear_stored_data()
{
	if (!this->m_ulAnimationInfo.empty())
		this->m_ulAnimationInfo.clear();
}
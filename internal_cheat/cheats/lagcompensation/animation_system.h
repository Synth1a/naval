#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
#include <optional>

enum ADVANCED_ACTIVITY : int
{
	ACTIVITY_NONE = 0,
	ACTIVITY_JUMP,
	ACTIVITY_LAND
};

enum
{
	MAIN,
	NONE,
	FIRST,
	SECOND
};

enum
{
	SAFEPOINT_LEFT,
	SAFEPOINT_CENTER,
	SAFEPOINT_RIGHT
};

struct matrixes
{
	matrix3x4_t main[MAXSTUDIOBONES];
	matrix3x4_t center[MAXSTUDIOBONES];
	matrix3x4_t left[MAXSTUDIOBONES];
	matrix3x4_t right[MAXSTUDIOBONES];
};

class lagcompensation : public singleton <lagcompensation> {
public:
	struct LagRecord_t {
		LagRecord_t() = default;

		explicit LagRecord_t(player_t* pEntity);
		explicit LagRecord_t(player_t* pEntity, Vector vecLastReliableAngle);

		void Restore(player_t* pEntity);
		void Apply(player_t* pEntity);
		bool IsValid(float flSimulationTime, const float flRange = 0.2f);
		void BuildBones(matrix3x4_t* matrix, int mask);

		player_t* m_pEntity;
		int m_iEntIndex;

		matrixes m_pMatrix;

		bool m_bDormant;
		bool m_bIsLeft;
		bool m_bIsRight;
		bool m_bIsFakePlayer;
		
		Vector m_vecVelocity;
		Vector m_vecOrigin;
		Vector m_vecAbsOrigin;
		Vector m_vecMins;
		Vector m_vecMaxs;

		AnimationLayer m_pLayers[13];
		AnimationLayer m_pResolveLayers[3][15];
		float m_pPoses[24];

		float m_flBackWardAngle;
		float m_flLeftAngle;
		float m_flRightAngle;
		float m_flAnimSpeed;

		c_baseplayeranimationstate* m_pState;

		float m_flSimulationTime;
		float m_flMaxBodyRotation;
		float m_flInterpTime;
		float m_flDuck;
		float m_flLowerBodyYawTarget;
		float m_flOriginalGoalFeetYaw;
		float m_flLastShotTime;
		float m_flSpawnTime;

		Vector m_angLastReliableAngle;
		Vector m_angEyeAngles;
		Vector m_angAbsAngles;

		CBaseHandle m_ulEntHandle;

		int m_fFlags;
		int m_iEFlags;
		int m_iEffects;
		int m_iChoked;

		int m_iResolveMode;
		int m_iResolveType;
		int m_iResolveSide;
		float m_flResolveStrengh;
		
		bool m_bDidShot;
		bool m_bPrefer = false;
		bool m_bAllowAnimationUpdate;
		bool m_bAnimatePlayer;
	};

	void PostPlayerUpdate();

	std::optional<LagRecord_t*> GetLatestRecord(player_t* pEntity);
	std::optional<LagRecord_t*> GetOldestRecord(player_t* pEntity);
};

class animation_system : public singleton <animation_system> {
public:
	struct AnimationInfo_t {
		AnimationInfo_t(player_t* pEntity, std::deque<lagcompensation::LagRecord_t> pRecords)
			: m_pEntity(pEntity), m_pRecords(std::move(pRecords)), m_flLastSpawnTime(0) {
		}

		void UpdateAnimations(lagcompensation::LagRecord_t* pRecord, lagcompensation::LagRecord_t* pPreviousRecord);
		
		player_t* m_pEntity;
		std::deque<lagcompensation::LagRecord_t> m_pRecords;

		lagcompensation::LagRecord_t m_LatestRecord;
		lagcompensation::LagRecord_t m_PreviousRecord;

		float m_flLastSpawnTime{ };

		float m_flPreviousAngle{ };

		float m_flBrute{ };
		float m_flLatestDelta{ };
		float m_flBestSide{ };
		float m_flFinalResolveAngle{ };

		float m_flOldYaw{ };

		float m_flLastPinPulled{ };

		int m_iResolverSide{ };

		bool m_bWalking{ };
		bool m_bUsingMaxDesync{ };

		Vector m_vecLastReliableAngle;
	};
	
	std::unordered_map<unsigned long, AnimationInfo_t> m_ulAnimationInfo;

	AnimationInfo_t* get_animation_info(player_t* player);

	Vector m_BoneOrigins[65][MAXSTUDIOBONES];
	matrix3x4_t m_CachedMatrix[65][MAXSTUDIOBONES];
	bool m_CachedMatrixRetr[65];
	bool m_bLeftDormancy[65];

	void get_cached_matrix(player_t* player, matrix3x4_t* matrix);
	void on_update_clientside_animation(player_t* player);
	void UpdateSafeAnimation(player_t* pEntity, matrix3x4_t* matrix, AnimationLayer* layers, float angles);
	
	void clear_stored_data();
};

class optimized_adjust_data
{
public:
	int i;
	player_t* player;

	float simulation_time;
	float duck_amount;

	Vector angles;
	Vector origin;

	optimized_adjust_data() //-V730
	{
		reset();
	}

	void reset()
	{
		i = 0;
		player = nullptr;

		simulation_time = 0.0f;
		duck_amount = 0.0f;

		angles.Zero();
		origin.Zero();
	}
};

class resolver : public singleton <resolver>
{
public:
	void Resolve(player_t* e, lagcompensation::LagRecord_t* record, lagcompensation::LagRecord_t* prev_record);
public:
	enum Modes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_STAND,
		RESOLVE_WALK,
		RESOLVE_RUN,
		RESOLVE_AIR,
		RESOLVE_SIDEWAYS
	};

public:
	void SetMode(lagcompensation::LagRecord_t* record);
	void ResolveAngles(player_t* player, lagcompensation::LagRecord_t* record, lagcompensation::LagRecord_t* previous_record);
};

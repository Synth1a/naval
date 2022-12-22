#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

struct Netvars_data
{
	CHandle< entity_t > m_hGroundEntity;

	float m_flPostponeFireReadyTime = 0.f;
	float m_flRecoilIndex = 0.f;
	float m_flAccuracyPenalty = 0.f;
	float m_flDuckAmount = 0.f;
	float m_flDuckSpeed = 0.f;
	float m_flFallVelocity = 0.f;

	int m_nRenderMode = 0;
	int m_nTickbase = 0;
	int m_fFlags = 0;

	Vector m_vecOrigin = ZERO;
	Vector m_vecVelocity = ZERO;
	Vector m_vecBaseVelocity = ZERO;
	Vector m_vecViewOffset = ZERO;
	Vector m_vecAimPunchAngleVel = ZERO;

	Vector m_angAimPunchAngle = ZERO;
	Vector m_angViewPunchAngle = ZERO;
};

class engineprediction : public singleton <engineprediction>
{
	struct Backup_data
	{
		float recoil_index = 0.0f;
		float accuracy = 0.0f;
	};

	struct Prediction_data
	{
		bool old_in_prediction;
		bool old_first_prediction;

		float old_curtime;
		float old_frametime;

		unsigned int* prediction_random_seed = nullptr;
		entity_t** prediction_player = nullptr;
	};

	float m_flCycle = 0.f;
	float m_flAnimTime = 0.f;

	int m_iSequence = 0;
	int m_iAnimationParity = 0;

	void start_command(CUserCmd* cmd);
	void finish_command();
	void update_button_state(CUserCmd* cmd);
public:
	Netvars_data netvars_data[MULTIPLAYER_BACKUP];

	Netvars_data get_netvars(int nCommand) 
	{ 
		return this->netvars_data[nCommand % MULTIPLAYER_BACKUP]; 
	};

	Backup_data backup_data;
	Prediction_data prediction_data;

	void store_netvars(int command_number);
	void restore_netvars(int command_number);

	void update();
	void start(CUserCmd* cmd);
	void finish(CUserCmd* cmd);
	void store_viewmodel_data();
	void adjust_viewmodel_data();

	void reset_data();

	bool should_process_packetstart(int outgoing);

	std::deque <int> command_list = {};
};
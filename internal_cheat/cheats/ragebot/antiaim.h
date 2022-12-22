#pragma once

#include "..\autowall\penetration.h"
#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class antiaim : public singleton <antiaim>
{
private:
	void initialize(CUserCmd* m_pcmd);
public:
	void create_move(CUserCmd* m_pcmd, bool& bSendPacket);
	float get_pitch(CUserCmd* m_pcmd, bool& bSendPacket);
	float get_yaw(CUserCmd* m_pcmd, bool& bSendPacket);
	bool condition(CUserCmd* m_pcmd, bool dynamic_check = true);

	float at_targets();
	bool automatic_direction();
	void freestanding(CUserCmd* m_pcmd);

	void reset_data();

	int type = 0;
	int manual_side = -1;
	int final_manual_side = -1;
	int lby_type = 0;
	bool flip = false;
	bool freeze_check = false;
	bool breaking_lby = false;
	float desync_angle = 0.0f;
};

enum 
{
	SIDE_NONE = -1,
	SIDE_BACK,
	SIDE_LEFT,
	SIDE_RIGHT
};
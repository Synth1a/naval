#pragma once

#include "..\..\includes.hpp"
#include "..\ragebot\antiaim.h"

class fakelag : public singleton <fakelag>
{
private:
	bool force_ticks_allowed = false;
public:
	void Fakelag(CUserCmd* m_pcmd, bool& bSendPacket);
	void ForceTicksAllowedForProcessing(bool& bSendPacket);
	void SetMoveChokeClampLimit();
	void Createmove(CUserCmd* m_pcmd, bool& bSendPacket);
	bool FakelagCondition(CUserCmd* m_pcmd);
	void reset_data();

	bool condition = true;
	bool started_peeking = false;

	int max_choke = 0;
};
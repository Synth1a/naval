#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
#include "..\misc\prediction_system.h"

class networking : public singleton <networking>
{
private:
	Netvars_data compress_data[MULTIPLAYER_BACKUP];

	int final_predicted_tick = 0;
	float interp = 0.0f;
public:
	std::vector<std::pair<float, float>> computed_seeds;

	float latency;
	float flow_outgoing;
	float flow_incoming;
	float average_outgoing;
	float average_incoming;

	void store_netvar_data(int nCommand);
	void restore_netvar_data(int nCommand);

	void start_move(CUserCmd* m_pcmd, bool& bSendPacket);
	void packet_cycle(CUserCmd* m_pcmd, bool& bSendPacket);
	bool setup_packet(int sequence_number, bool* pbSendPacket);
	int ping();
	int framerate();
	float tickrate();
	int server_tick();
	void build_seed_table();
	void on_packetend(CClientState* client_state);
	void start_network();
	void process_interpolation(ClientFrameStage_t Stage, bool bPostFrame);
	void reset_data();
};
#pragma once

#include <cstdint>

#include "..\math\Vector.hpp"
#include "..\misc\bf_write.h"

#define GenDefineVFunc(...) ( this, __VA_ARGS__ ); }
#define VFUNC( index, func, sig ) auto func { return call_virtual< sig >( this, index ) GenDefineVFunc

class C_EventInfo
{
public:
	enum
	{
		EVENT_INDEX_BITS = 8,
		EVENT_DATA_LEN_BITS = 11,
		MAX_EVENT_DATA = 192,
	};

	inline C_EventInfo()
	{
		m_iClassID = 0;
		m_flFireDelay = 0.0f;
		m_nFlags = 0;
		m_pSendTable = 0;
		m_pClientClass = 0;
		m_iPacked = 0;
	}

	short m_iClassID;
	short pad;
	float m_flFireDelay;
	const void* m_pSendTable;
	const void* m_pClientClass;
	int m_iPacked;
	int m_nFlags;
	int m_aFilters[8];
	C_EventInfo* m_pNext;
};

class CClockDriftMgr
{
public:
	enum
	{
		// This controls how much it smoothes out the samples from the server.
		NUM_CLOCKDRIFT_SAMPLES = 16
	};

	// This holds how many ticks the client is ahead each time we get a server tick.
	// We average these together to get our estimate of how far ahead we are.
	float m_ClockOffsets[NUM_CLOCKDRIFT_SAMPLES]; //0x0128
	int m_iCurClockOffset; // 0x0168

	int m_nServerTick; // 0x016C		// Last-received tick from the server.
	int m_nClientTick; // 0x0170		// The client's own tick counter (specifically, for interpolation during rendering).
		// The server may be on a slightly different tick and the client will drift towards it.
}; //Size: 76

class INetChannel
{
public:
	char pad_0x0000[0x18]; //0x0000
	__int32 m_nOutSequenceNr; //0x0018 
	__int32 m_nInSequenceNr; //0x001C 
	__int32 m_nOutSequenceNrAck; //0x0020 
	__int32 m_nOutReliableState; //0x0024 
	__int32 m_nInReliableState; //0x0028 
	__int32 m_nChokedPackets; //0x002C

	void Transmit(bool onlyreliable)
	{
		using Fn = bool(__thiscall*)(void*, bool);
		call_virtual<Fn>(this, 49)(this, onlyreliable);
	}

	void send_datagram()
	{
		using Fn = int(__thiscall*)(void*, void*);
		call_virtual<Fn>(this, 46)(this, 0);
	}

	void SetTimeOut(float seconds)
	{
		using Fn = void(__thiscall*)(void*, float);
		return call_virtual<Fn>(this, 4)(this, seconds);
	}

	int RequestFile(const char* filename)
	{
		using Fn = int(__thiscall*)(void*, const char*);
		return call_virtual<Fn>(this, 62)(this, filename);
	}
};

class INetMessage
{
public:
	virtual	~INetMessage() {};

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).

	virtual void	SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
	virtual void	SetReliable(bool state) = 0;	// set to true if it's a reliable message

	virtual bool	Process(void) = 0; // calles the recently set handler to process this message

	virtual	bool	ReadFromBuffer(bf_read& buffer) = 0; // returns true if parsing was OK
	virtual	bool	WriteToBuffer(bf_write& buffer) = 0;	// returns true if writing was OK

	virtual bool	IsReliable(void) const = 0;  // true, if message needs reliable handling

	virtual int				GetType(void) const = 0; // returns module specific header tag eg svc_serverinfo
	virtual int				GetGroup(void) const = 0;	// returns net message group of this message
	virtual const char* GetName(void) const = 0;	// returns network message name, eg "svc_serverinfo"
	virtual INetChannel* GetNetChannel(void) const = 0;
	virtual const char* ToString(void) const = 0; // returns a human readable string about message content
};

class CClientState
{
public:
	char _0x0000[156];
	INetChannel* pNetChannel; //0x009C 
	__int32 iChallengeNr; //0x00A0 
	char _0x00A4[100];
	__int32 iSignonState; //0x0108 
	char _0x010C[8];
	float flNextCmdTime; //0x0114 
	__int32 iServerCount; //0x0118
	__int32 iCurrentSequence; //0x011C
	char pad_0120[8]; //0x0120
	CClockDriftMgr m_iClockDriftMgr; //0x0128
	int iDeltaTick; //0x0174 
	bool bPaused; //0x0178 
	char _0x017C[12];
	char szLevelName[260]; //0x0188 
	char szLevelNameShort[40]; //0x028C 
	char szGroupName[40]; //0x02B4 
	char szSecondName[32]; //0x02DC 
	char _0x02FC[140];
	__int32 iMaxClients; //0x0388 
	char _0x038C[18820];
	float flLastServerTickTime; //0x4D10 
	bool bInSimulation; //0x4D14 
	__int32 iOldTickCount; //0x4D18 
	float pad[2];
	float flTickRemainder; //0x4D1C 
	float flFrameTime; //0x4D20 
	__int32 nLastOutgoingCommand; //0x4D24 
	__int32 iChokedCommands; //0x4D28 
	__int32 nLastCommandAck; //0x4D2C 
	__int32	last_server_tick;	// 0x4D30 same update pattern as last_command_ack, but with server ticks
	__int32 iCommandAck; //0x4D34
	__int32 m_iSoundSequence; //0x4D38 
	char _0x4D38[80];
	Vector angViewPoint; //0x4D88 
	char _0x4D94[0xC4]; //0x4D94
public:
	bool& m_bIsHLTV() {
		return *(bool*)(uintptr_t(this) + 0x4D48);
	}

	C_EventInfo* m_aEvents() {
		return *(C_EventInfo**)((uintptr_t)(this) + (0x4E74));
	}
};
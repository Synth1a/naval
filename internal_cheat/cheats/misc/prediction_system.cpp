// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "prediction_system.h"
#include "..\lagcompensation\local_animations.h"

void engineprediction::store_netvars(int command_number)
{
	auto data = &this->netvars_data[command_number % MULTIPLAYER_BACKUP];

	data->m_fFlags = g_ctx.local()->m_fFlags();
	data->m_hGroundEntity = g_ctx.local()->m_hGroundEntity().Get();
	data->m_flDuckAmount = g_ctx.local()->m_flDuckAmount();
	data->m_flDuckSpeed = g_ctx.local()->m_flDuckSpeed();
	data->m_vecOrigin = g_ctx.local()->m_vecOrigin();
	data->m_vecVelocity = g_ctx.local()->m_vecVelocity();
	data->m_vecBaseVelocity = g_ctx.local()->m_vecBaseVelocity();
	data->m_flFallVelocity = g_ctx.local()->m_flFallVelocity();
	data->m_vecViewOffset = g_ctx.local()->m_vecViewOffset();
	data->m_angAimPunchAngle = g_ctx.local()->m_aimPunchAngle();
	data->m_vecAimPunchAngleVel = g_ctx.local()->m_aimPunchAngleVel();
	data->m_angViewPunchAngle = g_ctx.local()->m_viewPunchAngle();

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();
	if (!weapon)
	{
		data->m_flRecoilIndex = 0.0f;
		data->m_flAccuracyPenalty = 0.0f;

		return;
	}

	data->m_flRecoilIndex = weapon->m_flRecoilIndex();
	data->m_flAccuracyPenalty = weapon->m_fAccuracyPenalty();
}

void engineprediction::restore_netvars(int command_number)
{
	auto data = &this->netvars_data[command_number % MULTIPLAYER_BACKUP];

	g_ctx.local()->m_fFlags() = data->m_fFlags;
	g_ctx.local()->m_hGroundEntity() = data->m_hGroundEntity.Get();
	g_ctx.local()->m_flDuckAmount() = data->m_flDuckAmount;
	g_ctx.local()->m_flDuckSpeed() = data->m_flDuckSpeed;
	g_ctx.local()->m_vecOrigin() = data->m_vecOrigin;
	g_ctx.local()->m_vecVelocity() = data->m_vecVelocity;
	g_ctx.local()->m_vecBaseVelocity() = data->m_vecBaseVelocity;
	g_ctx.local()->m_flFallVelocity() = data->m_flFallVelocity;
	g_ctx.local()->m_vecViewOffset() = data->m_vecViewOffset;
	g_ctx.local()->m_aimPunchAngle() = data->m_angAimPunchAngle;
	g_ctx.local()->m_aimPunchAngleVel() = data->m_vecAimPunchAngleVel;
	g_ctx.local()->m_viewPunchAngle() = data->m_angViewPunchAngle;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	weapon->m_flRecoilIndex() = data->m_flRecoilIndex;
	weapon->m_fAccuracyPenalty() = data->m_flAccuracyPenalty;
}

void engineprediction::update()
{
	return m_prediction()->Update(m_clientstate()->iDeltaTick, m_clientstate()->iDeltaTick > 0, m_clientstate()->nLastCommandAck, m_clientstate()->iChokedCommands + m_clientstate()->nLastOutgoingCommand);
}

void engineprediction::start_command(CUserCmd* cmd)
{
	// backup prediction data.
	this->prediction_data.old_curtime = m_globals()->m_curtime;
	this->prediction_data.old_frametime = m_globals()->m_frametime;

	this->prediction_data.old_in_prediction = m_prediction()->InPrediction;
	this->prediction_data.old_first_prediction = m_prediction()->IsFirstTimePredicted;

	m_prediction()->IsFirstTimePredicted = false;
	m_prediction()->InPrediction = true;

	g_ctx.local()->m_pCurrentCommand() = cmd;
	g_ctx.local()->m_LastCmd() = *cmd;

	if (!this->prediction_data.prediction_random_seed)
		this->prediction_data.prediction_random_seed = *reinterpret_cast<unsigned int**>(g_ctx.addresses.prediction_random_seed);

	*this->prediction_data.prediction_random_seed = cmd->m_random_seed;

	if (!this->prediction_data.prediction_player)
		this->prediction_data.prediction_player = *reinterpret_cast<entity_t***>(g_ctx.addresses.prediction_player);

	*this->prediction_data.prediction_player = g_ctx.local();
}

void engineprediction::finish_command()
{
	*prediction_data.prediction_random_seed = -1;
	*prediction_data.prediction_player = nullptr;
}

void engineprediction::update_button_state(CUserCmd* cmd)
{
	cmd->m_buttons |= g_ctx.local()->m_iButtonForced();
	cmd->m_buttons &= ~(g_ctx.local()->m_iButtonDisabled());

	auto buttons = cmd->m_buttons;
	auto player_buttons = *g_ctx.local()->m_nButtons();
	auto buttons_changed = buttons ^ player_buttons;

	g_ctx.local()->m_afButtonLast() = player_buttons;
	*g_ctx.local()->m_nButtons() = buttons;
	g_ctx.local()->m_afButtonPressed() = buttons & buttons_changed;
	g_ctx.local()->m_afButtonReleased() = buttons_changed & ~buttons;
}

void engineprediction::start(CUserCmd* cmd)
{
	this->start_command(cmd);

	// proper.
	m_globals()->m_curtime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);
	m_globals()->m_frametime = m_prediction()->EnginePaused ? 0 : m_globals()->m_intervalpertick;

	m_gamemovement()->StartTrackPredictionErrors(g_ctx.local());

	m_movehelper()->set_host(g_ctx.local());

	CMoveData move_data;
	memset(&move_data, 0, sizeof(CMoveData));

	m_prediction()->SetupMove(g_ctx.local(), cmd, m_movehelper(), &move_data);

	m_gamemovement()->ProcessMovement(g_ctx.local(), &move_data);

	local_animations::get().run(cmd);

	m_prediction()->FinishMove(g_ctx.local(), cmd, &move_data);
}

void engineprediction::finish(CUserCmd* cmd)
{
	m_gamemovement()->FinishTrackPredictionErrors(g_ctx.local());

	m_movehelper()->set_host(nullptr);

	this->finish_command();

	m_globals()->m_curtime = prediction_data.old_curtime;
	m_globals()->m_frametime = prediction_data.old_frametime;

}

void engineprediction::reset_data()
{
	std::memset(this->netvars_data, 0, sizeof(this->netvars_data));

	this->prediction_data.old_curtime = 0.0f;
	this->prediction_data.old_frametime = 0.0f;
	this->prediction_data.old_in_prediction = false;
	this->prediction_data.old_first_prediction = false;

	this->backup_data.recoil_index = 0.f;
	this->backup_data.accuracy = 0.f;

	this->m_iAnimationParity = 0;
	this->m_iSequence = 0;
	this->m_flCycle = 0.f;
	this->m_flAnimTime = 0.f;

	if (!this->command_list.empty())
		this->command_list.clear();
}

void engineprediction::store_viewmodel_data()
{
	auto viewmodel = g_ctx.local()->m_hViewModel().Get();
	if (!viewmodel)
		return;

	this->m_iAnimationParity = viewmodel->m_nAnimationParity();
	this->m_iSequence = viewmodel->m_nSequence();
	this->m_flCycle = viewmodel->m_flCycle();
	this->m_flAnimTime = viewmodel->m_flAnimTime();
}

void engineprediction::adjust_viewmodel_data()
{
	auto viewmodel = g_ctx.local()->m_hViewModel().Get();
	if (!viewmodel)
		return;

	if (this->m_iSequence != viewmodel->m_nSequence() || this->m_iAnimationParity != viewmodel->m_nAnimationParity())
		return;

	viewmodel->m_flCycle() = this->m_flCycle;
	viewmodel->m_flAnimTime() = this->m_flAnimTime;
}

bool engineprediction::should_process_packetstart(int outgoing)
{
	if (!g_ctx.available())
		return true;

	if (!g_ctx.local()->is_alive())
		return true;

	for (auto cmd = this->command_list.begin(); cmd != this->command_list.end(); cmd++)
	{
		if (*cmd != outgoing)
			continue;

		this->command_list.erase(cmd);
		return true;
	}

	return false;
}
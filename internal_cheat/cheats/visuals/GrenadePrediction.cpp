// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "GrenadePrediction.h"
#include "..\misc\prediction_system.h"

void GrenadePrediction::Tick(int buttons)
{
	this->data.throw_strength = math::clamp(g_ctx.globals.weapon->m_flThrowStrength(), 0.f, 1.f);
	this->data.act = ACT_NONE;

	auto in_attack = buttons & IN_ATTACK;
	auto in_attack2 = buttons & IN_ATTACK2;

	if (in_attack || in_attack2)
	{
		if (in_attack && in_attack2)
			this->data.act = ACT_LOB;
		else if (!in_attack)
			this->data.act = ACT_DROP;
		else
			this->data.act = ACT_THROW;
	}
	else if (!g_cfg.esp.on_click)
		this->data.act = ACT_THROW;
}

void GrenadePrediction::View(CViewSetup* setup, weapon_t* weapon)
{
	bool attack, attack2;

	if (g_ctx.local()->is_alive() && g_ctx.get_command())
	{
		attack = (g_ctx.get_command()->m_buttons & IN_ATTACK);
		attack2 = (g_ctx.get_command()->m_buttons & IN_ATTACK2);

		if (!antiaim::get().freeze_check && this->data.act != ACT_NONE && this->data.throw_strength >= -1.f)
		{
			this->data.type = weapon->m_iItemDefinitionIndex();
			this->Simulate(setup);
		}
		else
			this->data.type = 0;
	}
}

inline float CSGO_Armor(float flDamage, int ArmorValue) {
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}

void GrenadePrediction::Paint()
{
	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (path.size() < 2)
		return;

	CTraceFilter filter;
	filter.pSkip = g_ctx.local();

	std::pair <float, player_t*> target{ 0.f, nullptr };

	Vector prev = path[0];

	for (int i = 1; i < m_globals()->m_maxclients; ++i) {
		player_t* player = (player_t*)m_entitylist()->GetClientEntity(i);
		if (!player->valid(true))
			continue;

		// get center of mass for player.
		auto origin = player->m_vecOrigin();

		auto min = player->m_vecMins() + origin;
		auto max = player->m_vecMaxs() + origin;

		auto center = min + (max - min) * 0.5f;

		// get delta between center of mass and final nade pos.
		auto delta = center - path[path.size() - 1];

		if (weapon->m_iItemDefinitionIndex() == WEAPON_HEGRENADE) {

			// is within damage radius?
			if (delta.Length() > 350.f)
				continue;

			Vector NadeScreen;
			math::world_to_screen(path[path.size() - 1], NadeScreen);

			// main hitbox, that takes damage
			Vector vPelvis = player->hitbox_position(HITBOX_PELVIS);
			trace_t ptr;
			m_trace()->TraceRay(Ray_t(path[path.size() - 1], vPelvis), MASK_SHOT, &filter, &ptr);
			//trace to it

			if (ptr.hit_entity == player) {
				Vector PelvisScreen;

				math::world_to_screen(vPelvis, PelvisScreen);

				// some magic values by VaLvO
				static float a = 105.0f;
				static float b = 25.0f;
				static float c = 140.0f;

				float d = ((delta.Length() - b) / c);
				float flDamage = a * exp(-d * d);

				// do main damage calculation here
				auto dmg = max(static_cast<int>(ceilf(CSGO_Armor(flDamage, player->m_ArmorValue()))), 0);

				// clip max damage.
				dmg = min(dmg, (player->m_ArmorValue() > 0) ? 57 : 98);
				// max damage proof - https://counterstrike.fandom.com/wiki/HE_Grenade 

				// if we have target with more damage
				if (dmg > target.first) {
					target.first = dmg;
					target.second = player;
				}
			}
		}
	}

	// we have a target for damage.
	if (target.second && weapon->m_iItemDefinitionIndex() == WEAPON_HEGRENADE && (!g_cfg.esp.on_click || g_ctx.get_command()->m_buttons & IN_ATTACK || g_ctx.get_command()->m_buttons & IN_ATTACK2)) {
		Vector screen;

		if (math::world_to_screen(target.second->hitbox_position(HITBOX_HEAD), screen))
			render::get().text(fonts[ESP], screen.x - 4, screen.y - 28, (int)target.first >= target.second->m_iHealth() ? Color::Red : Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, std::to_string((int)target.first).c_str());
	}

	if (this->data.type && path.size() > 1)
	{
		Vector nadeStart, nadeEnd;

		Vector endpos = path[path.size() - 1];

		if (weapon->m_iItemDefinitionIndex() == WEAPON_MOLOTOV || weapon->m_iItemDefinitionIndex() == WEAPON_INCGRENADE)
			render::get().Draw3DCircle(endpos, 120, g_cfg.esp.grenade_prediction_tracer_color, 1);
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_SMOKEGRENADE)
			render::get().Draw3DCircle(endpos, 144, g_cfg.esp.grenade_prediction_tracer_color, 1);
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_HEGRENADE)
			render::get().Draw3DCircle(endpos, 384, g_cfg.esp.grenade_prediction_tracer_color, 1);
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_FLASHBANG)
			render::get().Draw3DCircle(endpos, 180, g_cfg.esp.grenade_prediction_tracer_color, 1);
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_DECOY)
			render::get().Draw3DCircle(endpos, 60, g_cfg.esp.grenade_prediction_tracer_color, 1);

		for (auto it = path.begin(); it != path.end(); ++it)
		{
			if (math::world_to_screen(prev, nadeStart) && math::world_to_screen(*it, nadeEnd))
			{
				render::get().line((int)nadeStart.x, (int)nadeStart.y, (int)nadeEnd.x, (int)nadeEnd.y, g_cfg.esp.grenade_prediction_color, 1);
			}
			prev = *it;
		}
	}
}

void GrenadePrediction::Setup(Vector& vecSrc, Vector& vecThrow, const Vector& viewangles)
{
	Vector angThrow = viewangles;
	float pitch = math::normalize_pitch(angThrow.x);

	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.x = a;

	float b = 750.0f * 0.9f;

	math::clamp(b, 15.f, 750.f);

	b *= ((this->data.throw_strength * 0.7f) + 0.3f);

	Vector vForward, vRight, vUp;
	math::angle_vectors(angThrow, &vForward, &vRight, &vUp);

	vecSrc = g_ctx.globals.eye_pos;
	vecSrc.z += (this->data.throw_strength * 12.f) - 12.f;

	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f;

	TraceHull(vecSrc, vecDest, tr);

	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	vecThrow = g_ctx.local()->m_vecVelocity(); vecThrow *= 1.25f;
	vecThrow += vForward * b;
}

void GrenadePrediction::Simulate(CViewSetup* setup)
{
	Vector vecSrc, vecThrow;
	Vector angles; m_engine()->GetViewAngles(angles);
	Setup(vecSrc, vecThrow, angles);

	float interval = m_globals()->m_intervalpertick;
	int logstep = (int)(0.05f / interval);
	int logtimer = 0;

	if (!path.empty())
		path.clear();

	for (auto i = 0; i < 4096; ++i)
	{
		if (!logtimer)
			path.push_back(vecSrc);

		int s = Step(vecSrc, vecThrow, i, interval);

		if (s & 1)
			break;

		if (s & 2 || logtimer >= logstep)
			logtimer = 0;
		else
			++logtimer;

		if (vecThrow.IsZero())
			break;
	}

	path.push_back(vecSrc);
}

int GrenadePrediction::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
{
	Vector move; AddGravityMove(move, vecThrow, interval, false);
	trace_t tr; PushEntity(vecSrc, move, tr);

	int result = 0;

	if (CheckDetonate(vecThrow, tr, tick, interval))
		result |= 1;

	if (tr.fraction != 1.0f) //-V550
	{
		result |= 2;
		ResolveFlyCollisionCustom(tr, vecThrow, move, interval);
	}

	vecSrc = tr.endpos;
	return result;
}

bool GrenadePrediction::CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
{
	auto time = TICKS_TO_TIME(tick);

	switch (this->data.type)
	{
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return time >= 1.5f && !(tick % TIME_TO_TICKS(0.2f));

	case WEAPON_SMOKEGRENADE:
		return vecThrow.Length() <= 0.1f && !(tick % TIME_TO_TICKS(0.2f));

	case WEAPON_DECOY:
		return vecThrow.Length() <= 0.2f && !(tick % TIME_TO_TICKS(0.2f));

	case WEAPON_MOLOTOV:
	case WEAPON_FIREBOMB:
		// detonate when hitting the floor.
		if (tr.fraction != 1.f && (std::cos(DEG2RAD(g_ctx.convars.weapon_molotov_maxdetonateslope->GetFloat())) <= tr.plane.normal.z)) //-V550
			return true;

		// detonate if we have traveled for too long.
		// checked every 0.1s
		return time >= g_ctx.convars.molotov_throw_detonate_time->GetFloat() && !(tick % TIME_TO_TICKS(0.1f));

	default:
		return false;
	}

	return false;
}

void GrenadePrediction::TraceHull(Vector& src, Vector& end, trace_t& tr)
{
	CTraceFilterWorldAndPropsOnly filter;

	m_trace()->TraceRay(Ray_t(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f)), 0x200400B, &filter, &tr);
}

void GrenadePrediction::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	// gravity for grenades.
	float gravity = g_ctx.convars.sv_gravity->GetFloat() * 0.4f;

	// move one tick using current velocity.
	move.x = vel.x * m_globals()->m_intervalpertick; //-V807
	move.y = vel.y * m_globals()->m_intervalpertick;

	// apply linear acceleration due to gravity.
	// calculate new z velocity.
	float z = vel.z - (gravity * m_globals()->m_intervalpertick);

	// apply velocity to move, the average of the new and the old.
	move.z = ((vel.z + z) / 2.f) * m_globals()->m_intervalpertick;

	// write back new gravity corrected z-velocity.
	vel.z = z;
}

void GrenadePrediction::PushEntity(Vector& src, const Vector& move, trace_t& tr)
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;
	TraceHull(src, vecAbsEnd, tr);
}

void GrenadePrediction::ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, const Vector& move, float interval)
{
	if (tr.hit_entity) 
	{
		if (util::is_breakable_entity(tr.hit_entity))
		{
			auto client_class = tr.hit_entity->GetClientClass();

			if (!client_class)
				return;

			auto network_name = client_class->m_pNetworkName;

			if (strcmp(network_name, crypt_str("CFuncBrush")) && strcmp(network_name, crypt_str("CBaseDoor")) && strcmp(network_name, crypt_str("CCSPlayer")) && strcmp(network_name, crypt_str("CBaseEntity"))) //-V526
			{
				// move object.
				PushEntity(tr.endpos, move, tr);

				// deduct velocity penalty.
				vecVelocity *= 0.4f;
				return;
			}
		}
	}

	float flSurfaceElasticity = 1.0, flGrenadeElasticity = 0.45f;
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f;

	if (flSpeedSqr < flMinSpeedSqr)
	{
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	if (tr.plane.normal.z > 0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval);
		PushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
		vecVelocity = vecAbsVelocity;
}

int GrenadePrediction::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float backoff, change, angle;
	int   i, blocked;

	blocked = 0;
	angle = normal[2];

	if (angle > 0) blocked |= 1;
	if (!angle) blocked |= 2; //-V550

	backoff = in.Dot(normal) * overbounce;
	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	return blocked;
}
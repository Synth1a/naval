#include "aim.h"

void aimbot::knife(CUserCmd* m_pcmd) {
	struct KnifeTarget_t { bool stab; Vector angle; lagcompensation::LagRecord_t* record; };
	KnifeTarget_t target{};

	// we have no targets.
	if( m_targets.empty( ) )
		return;

	// iterate all targets.
	for (const auto& t : m_targets) {
		if (!t->m_player)
			continue;
		// this target has no records
		// should never happen since it wouldnt be a target then.
		//if( t->m_records.empty( ) )
		//	continue;

		// see if target broke lagcompensation.
		/*if(g_lagcompensation.StartPrediction( t ) ) {
			LagRecord* front = t->m_records.front( ).get( );

			front->cache( );

			// trace with front.
			for( const auto& a : m_knife_ang ) {

				// check if we can knife.
				if( !CanKnife( front, a, target.stab ) )
					continue;

				// set target data.
				target.angle  = a;
				target.record = front;
				break;
			}
		}

		// we can history aim.
		else {*/

		const auto best = lagcompensation::get().GetLatestRecord(t->m_player);
		if (!best.has_value())
			continue;

		//best->cache( );

		// trace with best.
		for (const auto& a : m_knife_ang) {

			// check if we can knife.
			if (!CanKnife(best.value(), a, target.stab))
				continue;

			// set target data.
			target.angle = a;
			target.record = best.value();
			break;
		}

		const auto last = lagcompensation::get().GetOldestRecord(t->m_player);
		if (!last.has_value() || last.value() == best.value())
			continue;

		//last->cache( );

		// trace with last.
		for (const auto& a : m_knife_ang) {

			// check if we can knife.
			if (!CanKnife(last.value(), a, target.stab))
				continue;

			// set target data.
			target.angle = a;
			target.record = last.value();
			break;
		}
		//}

		// target player has been found already.
		if (target.record)
			break;
	}

	// we found a target.
	// set out data and choke.
	if( target.record ) {
		// set target tick.
		m_pcmd->m_tickcount = TIME_TO_TICKS( target.record->m_flSimulationTime + util::get_interpolation() );

		// set view angles.
		m_pcmd->m_viewangles = target.angle;

		// set attack1 or attack2.
		m_pcmd->m_buttons |= target.stab ? IN_ATTACK2 : IN_ATTACK;
	}
}

bool aimbot::CanKnife(lagcompensation::LagRecord_t* record, Vector angle, bool& stab ) {
	// convert target angle to direction.
	Vector forward;
	math::angle_vectors( angle, forward );

	// see if we can hit the player with full range
	// this means no stab.
	CGameTrace trace;
	KnifeTrace( forward, false, &trace );

	// we hit smthing else than we were looking for.
	if( !g_ctx.globals.weapon || !trace.hit_entity || trace.hit_entity != record->m_pEntity )
		return false;

	bool armor = record->m_pEntity->m_ArmorValue( ) > 0;
	bool first = g_ctx.globals.weapon->m_flNextPrimaryAttack( ) + 0.4f < m_globals()->m_curtime;
	bool back  = KnifeIsBehind( record );

	int stab_dmg  = m_knife_dmg.stab[ armor ][ back ];
	int slash_dmg = m_knife_dmg.swing[ first ][ armor ][ back ];
	int swing_dmg = m_knife_dmg.swing[ false ][ armor ][ back ];

	// smart knifebot.
	int health = record->m_pEntity->m_iHealth( );
	if( health <= slash_dmg )
		stab = false;

	else if( health <= stab_dmg )
		stab = true;

	else if( health > ( slash_dmg + swing_dmg + stab_dmg ) )
		stab = true;

	else
		stab = false;

	// damage wise a stab would be sufficient here.
	if( stab && !KnifeTrace( forward, true, &trace ) )
		return false;

	return true;
}

bool aimbot::KnifeTrace( Vector dir, bool stab, CGameTrace* trace ) {
	float range = stab ? 32.f : 48.f;

	Vector start = g_ctx.globals.eye_pos;
	Vector end   = start + ( dir * range );

	uint32_t filter_[4] =
	{
		*(uint32_t*)(g_ctx.addresses.trace_filter_simple),
		(uint32_t)g_ctx.local(),
		0,
		0
	};

	m_trace()->TraceRay(Ray_t( start, end ), MASK_SOLID, (ITraceFilter*)&filter_, trace);

	// if the above failed try a hull trace.
	if( trace->fraction >= 1.f ) {
		m_trace()->TraceRay(Ray_t( start, end, { -16.f, -16.f, -18.f }, { 16.f, 16.f, 18.f } ), MASK_SOLID, (ITraceFilter*)&filter_, trace);
		return trace->fraction < 1.f;
	}

	return true;
}

bool aimbot::KnifeIsBehind( lagcompensation::LagRecord_t* record ) {
	Vector delta = (record->m_vecOrigin - g_ctx.globals.eye_pos);
	delta.NormalizeInPlace();
	delta.z = 0.f;

	Vector target;
	math::angle_vectors( record->m_angAbsAngles, target );
	target.z = 0.f;

	return delta.Dot( target ) > 0.475f;
}
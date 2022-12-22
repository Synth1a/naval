#include "..\..\includes.hpp"

void IEngineTrace::TraceLine(const Vector& src, const Vector& dst, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace) {
    static auto trace_filter_simple = g_ctx.addresses.trace_filter_simple;

    std::uintptr_t filter[4] = {
        *reinterpret_cast<std::uintptr_t*>(trace_filter_simple),
        reinterpret_cast<std::uintptr_t>(entity),
        collision_group,
        0
    };

    m_trace()->TraceRay(Ray_t(src, dst), mask, reinterpret_cast<CTraceFilter*>(&filter), trace);
}

void IEngineTrace::TraceHull(const Vector& src, const Vector& dst, const Vector& mins, const Vector& maxs, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace) {
	static auto trace_filter_simple = g_ctx.addresses.trace_filter_simple;

	std::uintptr_t filter[4] = {
		*reinterpret_cast<std::uintptr_t*>(trace_filter_simple),
		reinterpret_cast<std::uintptr_t>(entity),
		collision_group,
		0
	};

	m_trace()->TraceRay(Ray_t(src, dst, mins, maxs), mask, reinterpret_cast<CTraceFilter*>(&filter), trace);
}
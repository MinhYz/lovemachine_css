#pragma once
#include "game classes.h"
#include "trace shit.h"

#define MASK_SHOT_CSS (0x46004003)

bool is_visible(centity* entity, cvector to)
{
	if (to.IsZero() || !entity || !entity->valid()) return false;

	centity* local_ent = (global::local && global::local->valid()) ? global::local : global::local_observed;
	if (!local_ent || !local_ent->valid()) return false;

	cvector from = local_ent->get_eye_pos();
	if (from.IsZero()) return false;

	itracefilter filter;
	filter.skip = local_ent;
	filter.target = entity;

	ray_t ray;
	ray.Init(from, to);

	trace_t trace;
	_engine_trace->trace_ray(ray, MASK_SHOT_CSS, &filter, &trace);

	return (trace.m_pEnt == entity || trace.fraction >= 0.98f);
}

bool is_hitbox_visible(centity* entity, int id = 0, matrix3x4_t* matrix = nullptr)
{
	return is_visible(entity, entity->get_hitbox(id, matrix, global::curtime));
}
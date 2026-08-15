#pragma once
#if defined(__GNUG__) || defined(__clang__)
#include_next <math.h>
#else
#include <math.h>
#endif
#include <cmath>
#include <algorithm>
using std::max;
using std::min;

#include "includes.h"
#include "vector.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define rad2deg( x )  ( (float)(x) * (float)(180.f / (float)M_PI) )
#define deg2rad( x )  ( (float)(x) * (float)((float)M_PI / 180.f) )

void normalize_angle(qangle& angle)
{
	//while (angle.x > 89.f) angle.x -= 180.f;
	//while (angle.x < -89.f) angle.x += 180.f;
	angle.x = max(-89.f, min(angle.x, 89.f));
	while (angle.y > 180.f) angle.y -= 360.f;
	while (angle.y < -180.f) angle.y += 360.f;
}

#ifdef _WIN32
typedef void (*RandomSeedFn)(int iSeed);
typedef float (*RandomFloatFn)(float flMinVal, float flMaxVal);

inline void RandomSeed(int iSeed)
{
	static RandomSeedFn pRandomSeed = (RandomSeedFn)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed");
	if (pRandomSeed) pRandomSeed(iSeed);
	else srand(iSeed);
}

inline float RandomFloat(float flMinVal, float flMaxVal)
{
	static RandomFloatFn pRandomFloat = (RandomFloatFn)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat");
	if (pRandomFloat) return pRandomFloat(flMinVal, flMaxVal);
	return flMinVal + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (flMaxVal - flMinVal)));
}
#else
inline void RandomSeed(int iSeed) { srand(iSeed); }
inline float RandomFloat(float flMinVal, float flMaxVal)
{
	return flMinVal + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (flMaxVal - flMinVal)));
}
#endif

inline void VectorAngles(const Vector& forward, qangle& angles)
{
	float tmp, yaw, pitch;

	if (forward[1] == 0.0f && forward[0] == 0.0f)
	{
		yaw = 0.0f;
		if (forward[2] > 0.0f)
			pitch = 270.0f;
		else
			pitch = 90.0f;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180.0f / (float)M_PI);
		if (yaw < 0.0f)
			yaw += 360.0f;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180.0f / (float)M_PI);
		if (pitch < 0.0f)
			pitch += 360.0f;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.0f;

	normalize_angle(angles);
}

void vectorangles(const cvector& forward, qangle& angles)
{
	VectorAngles(forward, angles);
}

void sincos(float radians, float* sine, float* cosine)
{
	*sine = sinf(radians);
	*cosine = cosf(radians);
}

void anglevectors(const qangle& angles, cvector* forward)
{
	float sp, sy, cp, cy;

	sincos(deg2rad(angles.y), &sy, &cy);
	sincos(deg2rad(angles.x), &sp, &cp);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

inline void AngleVectors(const Vector& angles, Vector* forward, Vector* right = nullptr, Vector* up = nullptr)
{
	float sp, sy, sr, cp, cy, cr;

	sincos(deg2rad(angles.x), &sp, &cp);
	sincos(deg2rad(angles.y), &sy, &cy);
	sincos(deg2rad(angles.z), &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}
	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}
	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

inline float VectorNormalize(Vector& v)
{
	float l = v.Length();
	if (l != 0.0f) { v /= l; } else { v.x = v.y = 0.0f; v.z = 1.0f; }
	return l;
}

qangle calc_angle(cvector src, cvector dst)
{
	qangle angles;
	cvector delta = src - dst;
	vectorangles(delta, angles);
	delta.Normalize();
	//normalize_angle(delta); // delta.Normalize(); Ã§Ã Ã·Ã¥Ã¬ Ã­Ã³Ã¦Ã­Ã  Ã½Ã²Ã  Ã´Ã³Ã­ÃªÃ¶Ã¨Ã¿ Ã§Ã¤Ã¥Ã±Ã¼?
	return angles;
}

float get_fov(const qangle& viewAngle, const qangle& aimAngle)
{
	Vector ang, aim;

	anglevectors(viewAngle, &aim);
	anglevectors(aimAngle, &ang);

	return rad2deg(acos(aim.Dot(ang) / aim.LengthSqr()));
}

struct matrix3x4_t;
void vector_transform(cvector in, const matrix3x4_t& transform, cvector& out);

template<typename t>
t math_clamp(t pmin, t val, t pmax)
{
	return max(pmin, min(val, pmax));
}
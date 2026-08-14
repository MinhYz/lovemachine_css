#pragma once
#include "game def's.h"
#include "game shit.h"
#include "settings.h"
#include "global.h"

namespace rage
{
	inline void normalize_angles(Vector& angles)
	{
		while (angles.x > 89.0f) angles.x -= 180.0f;
		while (angles.x < -89.0f) angles.x += 180.0f;
		while (angles.y > 180.0f) angles.y -= 360.0f;
		while (angles.y < -180.0f) angles.y += 360.0f;
		angles.z = 0.0f;
	}

	inline void fix_movement(cusercmd* cmd, Vector old_angles)
	{
		Vector forward, right, up;
		AngleVectors(old_angles, &forward, &right, &up);
		forward.z = 0.f; right.z = 0.f;
		VectorNormalize(forward); VectorNormalize(right);

		Vector fwd_new, right_new, up_new;
		AngleVectors(cmd->viewangles, &fwd_new, &right_new, &up_new);
		fwd_new.z = 0.f; right_new.z = 0.f;
		VectorNormalize(fwd_new); VectorNormalize(right_new);

		Vector wish_dir = forward * cmd->forwardmove + right * cmd->sidemove;
		cmd->forwardmove = wish_dir.Dot(fwd_new);
		cmd->sidemove = wish_dir.Dot(right_new);
	}

	inline void anti_aim()
	{
		if (!sets->rage.enabled && !sets->rage.spinbot && sets->rage.pitch_aa == 0 && sets->rage.yaw_aa == 0)
			return;

		if (!global::cmd || !global::local || !global::local->valid())
			return;

		if (global::cmd->buttons & IN_ATTACK)
			return;

		Vector old_angles = global::cmd->viewangles;

		// Pitch Anti-Aim
		if (sets->rage.pitch_aa == 1) // Emotion (89°)
			global::cmd->viewangles.x = 89.0f;
		else if (sets->rage.pitch_aa == 2) // Up (-89°)
			global::cmd->viewangles.x = -89.0f;
		else if (sets->rage.pitch_aa == 3) // Zero (0°)
			global::cmd->viewangles.x = 0.0f;

		// Yaw Anti-Aim / Spinbot
		if (sets->rage.spinbot || sets->rage.yaw_aa == 2) // Spinbot
		{
			static float spin_angle = 0.0f;
			spin_angle += sets->rage.spin_speed;
			if (spin_angle > 180.0f) spin_angle -= 360.0f;
			if (spin_angle < -180.0f) spin_angle += 360.0f;
			global::cmd->viewangles.y = spin_angle;
		}
		else if (sets->rage.yaw_aa == 1) // Backward (180°)
		{
			global::cmd->viewangles.y += 180.0f;
		}
		else if (sets->rage.yaw_aa == 3) // Jitter
		{
			static bool jitter_flip = false;
			jitter_flip = !jitter_flip;
			global::cmd->viewangles.y += jitter_flip ? 90.0f : -90.0f;
		}
		else if (sets->rage.yaw_aa == 4) // Sideways (90°)
		{
			global::cmd->viewangles.y += 90.0f;
		}

		normalize_angles(global::cmd->viewangles);
		fix_movement(global::cmd, old_angles);
	}

	namespace aimbot
	{
		inline int best_id = -1;
		inline float best_fov = 180.f;
		
		inline void start()
		{

		}

		inline void loop(int id, centity* entity)
		{

		}

		inline void end()
		{

		}
	}
}
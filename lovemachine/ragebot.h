#pragma once
#include "game def's.h"
#include "game shit.h"
#include "settings.h"
#include "global.h"
#include "math.h"
#include <algorithm>

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
		if (!cmd) return;

		Vector forward, right, up;
		AngleVectors(old_angles, &forward, &right, &up);
		forward.z = 0.f; right.z = 0.f;
		VectorNormalize(forward); VectorNormalize(right);

		Vector fwd_new, right_new, up_new;
		AngleVectors(cmd->viewangles, &fwd_new, &right_new, &up_new);
		fwd_new.z = 0.f; right_new.z = 0.f;
		VectorNormalize(fwd_new); VectorNormalize(right_new);

		Vector wish_dir = (forward * cmd->forwardmove) + (right * cmd->sidemove);
		float fwd_scale = (fwd_new.x * fwd_new.x + fwd_new.y * fwd_new.y);
		float right_scale = (right_new.x * right_new.x + right_new.y * right_new.y);

		if (fwd_scale > 0.0001f)
			cmd->forwardmove = (wish_dir.x * fwd_new.x + wish_dir.y * fwd_new.y) / fwd_scale;
		if (right_scale > 0.0001f)
			cmd->sidemove = (wish_dir.x * right_new.x + wish_dir.y * right_new.y) / right_scale;

		cmd->forwardmove = (cmd->forwardmove < -450.0f) ? -450.0f : ((cmd->forwardmove > 450.0f) ? 450.0f : cmd->forwardmove);
		cmd->sidemove = (cmd->sidemove < -450.0f) ? -450.0f : ((cmd->sidemove > 450.0f) ? 450.0f : cmd->sidemove);
	}

	inline void autostop(cusercmd* cmd, Vector orig_angles)
	{
		if (!cmd || !global::local || !global::local->valid()) return;

		// Only autostop when on ground, do not kill mid-air bunnyhopping speed
		if (!(global::local->get_flags() & FL_ONGROUND)) return;

		cvector vel = global::local->get_velocity();
		float speed = vel.Length2D();

		if (speed > 15.0f)
		{
			qangle move_ang;
			VectorAngles(vel * -1.0f, move_ang);
			move_ang.y = orig_angles.y - move_ang.y;

			Vector forward;
			AngleVectors(move_ang, &forward, nullptr, nullptr);
			Vector stop_dir = forward * 450.0f;

			cmd->forwardmove = (stop_dir.x < -450.0f) ? -450.0f : ((stop_dir.x > 450.0f) ? 450.0f : stop_dir.x);
			cmd->sidemove = (stop_dir.y < -450.0f) ? -450.0f : ((stop_dir.y > 450.0f) ? 450.0f : stop_dir.y);
		}
		else
		{
			cmd->forwardmove = 0.0f;
			cmd->sidemove = 0.0f;
		}
	}

	inline void apply_nospread_norecoil(cusercmd* pCmd, centity* pLocal)
	{
		if (!pCmd || !pLocal || !pLocal->valid()) return;

		auto pWeapon = pLocal->get_weapon();
		if (!pWeapon || pWeapon->get_clip1() <= 0) return;

		if (!(pCmd->buttons & IN_ATTACK)) return;

		// 1. NO RECOIL (XÓA ĐỘ GIẬT)
		qangle punchAngle = pLocal->get_punch();
		float mult_x = (sets->legit.aim.rcs[0] > 0.0f) ? sets->legit.aim.rcs[0] : 2.0f;
		float mult_y = (sets->legit.aim.rcs[1] > 0.0f) ? sets->legit.aim.rcs[1] : 2.0f;

		qangle currentAngles = pCmd->viewangles;
		currentAngles.x -= punchAngle.x * (sets->rage.enabled ? 2.0f : mult_x);
		currentAngles.y -= punchAngle.y * (sets->rage.enabled ? 2.0f : mult_y);
		currentAngles.z = 0.0f;

		// 2. NO SPREAD (XÓA ĐỘ LỆCH ĐẠN THEO THUẬT TOÁN CSS SPREAD/CONE)
		pWeapon->update_accuracy_penalty();
		float flSpread = pWeapon->get_spread();
		float flCone = pWeapon->get_cone();

		int seed = (pCmd->random_seed & 255) + 1;
		RandomSeed(seed);

		float rand1 = RandomFloat(0.0f, 1.0f);
		float pi1   = RandomFloat(0.0f, 2.0f * (float)M_PI);
		float rand2 = RandomFloat(0.0f, 1.0f);
		float pi2   = RandomFloat(0.0f, 2.0f * (float)M_PI);

		float spreadX = rand1 * flSpread * cosf(pi1) + rand2 * flCone * cosf(pi2);
		float spreadY = rand1 * flSpread * sinf(pi1) + rand2 * flCone * sinf(pi2);

		Vector forward, right, up;
		AngleVectors(currentAngles, &forward, &right, &up);

		Vector spreadDir = forward + (right * -spreadX) + (up * -spreadY);
		VectorNormalize(spreadDir);

		qangle compensatedAngles;
		VectorAngles(spreadDir, compensatedAngles);

		pCmd->viewangles = compensatedAngles;
		normalize_angles(pCmd->viewangles);
	}

	inline void standalone_rcs()
	{
		apply_nospread_norecoil(global::cmd, global::local);
	}

	inline bool magic_bullet(Vector orig_angles)
	{
		if (!sets->rage.enabled && !sets->rage.magic_bullet)
			return false;

		if (!global::cmd || !global::local || !global::local->valid() || !_ent_list || !_engine)
			return false;

		if (global::cmd->buttons & IN_RELOAD)
			return false;

		auto weapon = global::local->get_weapon();
		if (!weapon || weapon->get_clip1() <= 0)
			return false;

		bool is_attacking = (global::cmd->buttons & IN_ATTACK) != 0;
		if (!is_attacking && !sets->rage.autoshoot)
			return false;

		cvector local_eye = global::local->get_eye_pos();
		if (local_eye.IsZero()) return false;

		int max_clients = _engine->get_max_clients();

		centity* best_target = nullptr;
		cvector best_target_pos(0, 0, 0);
		float best_priority = -999999.0f;

		weaponinfo_t winfo;
		if (sets->rage.autowall)
		{
			get_weapon_info(weapon->get_weaponid(), weapon->is_silenced(), winfo);
		}

		for (int i = 1; i <= max_clients; i++)
		{
			if (i == global::local_id) continue;
			centity* enemy = _ent_list->get_centity(i);
			if (!enemy || IsBadReadPtr(enemy, sizeof(centity)) || !enemy->valid()) continue;
			if (!sets->rage.friends && enemy->get_team() == global::local->get_team()) continue;

			matrix3x4_t matrix[128];
			if (!enemy->get_hitbox_matrix(matrix, global::curtime)) continue;

			struct HitboxCandidate {
				cvector pos;
				float score_bonus;
			};

			std::vector<HitboxCandidate> candidates;

			// Head multi-point (Center + Multi-angles)
			cvector head_center = enemy->get_hitbox(hitbox_head, matrix);
			if (!head_center.IsZero())
			{
				candidates.push_back({ head_center, 5000.0f });
				candidates.push_back({ head_center + cvector(0, 0, 3.2f), 5000.0f }); // Top of head
				candidates.push_back({ head_center + cvector(2.0f, 0, 0), 4800.0f });
				candidates.push_back({ head_center + cvector(-2.0f, 0, 0), 4800.0f });
				candidates.push_back({ head_center + cvector(0, 2.0f, 0), 4800.0f });
				candidates.push_back({ head_center + cvector(0, -2.0f, 0), 4800.0f });
			}

			// Neck & Body Hitboxes
			cvector neck = enemy->get_hitbox(hitbox_neck, matrix);
			if (!neck.IsZero()) candidates.push_back({ neck, 3000.0f });

			cvector upper_chest = enemy->get_hitbox(hitbox_upper_chest, matrix);
			if (!upper_chest.IsZero()) candidates.push_back({ upper_chest, 2000.0f });

			cvector chest = enemy->get_hitbox(hitbox_chest, matrix);
			if (!chest.IsZero()) candidates.push_back({ chest, 1500.0f });

			cvector stomach = enemy->get_hitbox(hitbox_stomach, matrix);
			if (!stomach.IsZero()) candidates.push_back({ stomach, 1200.0f });

			cvector pelvis = enemy->get_hitbox(hitbox_pelvis, matrix);
			if (!pelvis.IsZero()) candidates.push_back({ pelvis, 1000.0f });

			// Limbs
			cvector l_arm = enemy->get_hitbox(hitbox_l_up_arm, matrix);
			if (!l_arm.IsZero()) candidates.push_back({ l_arm, 500.0f });

			cvector r_arm = enemy->get_hitbox(hitbox_r_up_arm, matrix);
			if (!r_arm.IsZero()) candidates.push_back({ r_arm, 500.0f });

			cvector l_leg = enemy->get_hitbox(hitbox_l_up_leg, matrix);
			if (!l_leg.IsZero()) candidates.push_back({ l_leg, 300.0f });

			cvector r_leg = enemy->get_hitbox(hitbox_r_up_leg, matrix);
			if (!r_leg.IsZero()) candidates.push_back({ r_leg, 300.0f });

			cvector found_target_pos(0, 0, 0);
			float found_score = -999999.0f;

			for (const auto& cand : candidates)
			{
				bool hittable = false;
				if (is_visible(enemy, cand.pos))
				{
					hittable = true;
				}
				else if (sets->rage.autowall)
				{
					if (autowall::is_penetrable(winfo, local_eye, cand.pos))
					{
						hittable = true;
					}
				}

				if (hittable)
				{
					found_target_pos = cand.pos;
					found_score = cand.score_bonus;
					break;
				}
			}

			if (found_target_pos.IsZero()) continue;

			float dist = (found_target_pos - local_eye).Length();
			float enemy_hp = (float)enemy->get_hp();
			float priority = found_score - (dist * 0.5f) - (enemy_hp * 2.0f);

			if (priority > best_priority)
			{
				best_priority = priority;
				best_target = enemy;
				best_target_pos = found_target_pos;
			}
		}

		if (best_target && !best_target_pos.IsZero())
		{
			qangle aim_angle = calc_angle(local_eye, best_target_pos);
			normalize_angles(aim_angle);

			global::cmd->viewangles = aim_angle;
			global::cmd->buttons |= IN_ATTACK;

			// Lag compensation backtrack tick count
			float sim_time = best_target->get_simulation_time();
			if (sim_time > 0.0f && _globals && _globals->interval_per_tick > 0.0f)
			{
				global::cmd->tick_count = (int)(0.5f + sim_time / _globals->interval_per_tick);
			}

			// Active counter-strafing autostop to guarantee 100% laser accuracy
			if (sets->rage.autostop)
			{
				autostop(global::cmd, orig_angles);
			}

			// Apply NoRecoil + NoSpread for 100% pin-point headshots
			apply_nospread_norecoil(global::cmd, global::local);
			return true;
		}

		return false;
	}

	inline void norecoil()
	{
		if ((!sets->misc.norecoil && !sets->legit.aim.norecoil) || !global::local || !global::cmd) return;
		if (global::cmd->buttons & IN_ATTACK)
		{
			qangle punch = global::local->get_punch();
			global::cmd->viewangles.x -= punch.x * 2.0f;
			global::cmd->viewangles.y -= punch.y * 2.0f;
		}
	}

	inline void anti_aim()
	{
		if (!sets->rage.anti_aim && !sets->rage.spinbot && sets->rage.pitch_aa == 0 && sets->rage.yaw_aa == 0)
			return;

		if (!global::cmd || !global::local || !global::local->valid())
			return;

		if (global::cmd->buttons & IN_ATTACK)
			return;

		// 1. Pitch Anti-Aim
		if (sets->rage.pitch_aa == 1) // Down / Emotion (89°)
			global::cmd->viewangles.x = 89.0f;
		else if (sets->rage.pitch_aa == 2) // Up (-89°)
			global::cmd->viewangles.x = -89.0f;
		else if (sets->rage.pitch_aa == 3) // Zero (0°)
			global::cmd->viewangles.x = 0.0f;

		// 2. Yaw Anti-Aim (1: Backwards 180°, 2: Spinbot 360°, 3: Jitter ±90°, 4: Sideways 90°)
		static float spin_angle = 0.0f;

		if (sets->rage.yaw_aa == 1) // Backward (180°)
		{
			global::cmd->viewangles.y += 180.0f;
		}
		else if (sets->rage.yaw_aa == 2 || sets->rage.spinbot) // Spinbot (Continuous 360° rotation)
		{
			float speed = sets->rage.spin_speed > 0.f ? sets->rage.spin_speed : 25.0f;
			spin_angle += speed;
			if (spin_angle > 180.0f) spin_angle -= 360.0f;
			if (spin_angle < -180.0f) spin_angle += 360.0f;
			global::cmd->viewangles.y = spin_angle;
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

		// Apply spinbot/anti-aim angles directly to local player render angle so thirdperson visibly rotates
		if (global::local && global::local->valid() && offsets::angles)
		{
			*(qangle*)((DWORD)global::local + offsets::angles) = global::cmd->viewangles;
		}
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
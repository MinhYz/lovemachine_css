#pragma once
#include "game shit.h"
#include "game def's.h"
#include "game classes.h"
#include "settings.h"
#include "surface.h"
#include "d3d.h"

namespace events
{
	namespace hitmarker
	{
		inline int damage_did = 0;
		inline int health_left = 0;
		inline int killstreak = 0;
		inline int killstreak2 = 0;
		inline str text = "KILLED";
		inline float timer = 0.f;
		inline float kill_timer = 0.f;

		inline void draw_box(int pos_x, int pos_y, string text_str, int alpha)
		{
			d3d::font::draw(d3d::font::hitmarker_small, pos_x, pos_y, color(20, 255, 20, alpha), DT_CENTER | DT_VCENTER, text_str.c_str());
		}

		inline void on_fire_event(igameevent* event, const char* name)
		{
			if (strcmp(name, "player_hurt") != 0)
				return;

			health_left = event->get_int("health");
			if (_engine->get_player_for_userid(event->get_int("userid")) == global::local_id && health_left == 0)
			{
				killstreak = 0;
			}

			if (_engine->get_player_for_userid(event->get_int("attacker")) != global::local_id)
				return;

			if (sets->misc.killshot && health_left == 0)
			{
				killstreak2++;
				_engine->clientcmd_unrestricted(str("say Streak Of " + to_str(killstreak2) + "!").c_str());
			}

			if (sets->visuals.enabled && sets->visuals.hitmarker)
			{
				damage_did += event->get_int("dmg_health");
			}

			if (health_left == 0)
			{
				if (sets->visuals.enabled && sets->visuals.hitmarker)
				{
					killstreak++;
					auto weapon = event->get_string("weapon");
					if (strstr(weapon, "knife")) text = "KNIFED";
					else if (event->get_int("hitgroup") == 1) text = "HEADSHOT";
					else if (strstr(weapon, "grenade") || strstr(weapon, "flash")) text = "GRENADE";
					else text = "KILLED";
					if (killstreak > 1) text += " X" + std::to_string(killstreak);
					kill_timer = global::realtime + 3.f;
				}
				legit::aimbot::kill_delay = sets->legit.aim.kill_delay != 0.f ? global::curtime + sets->legit.aim.kill_delay : 0.f;
			}
			timer = global::realtime + 4.f;
		}
		
		inline void on_draw()
		{
			if (!sets->misc.killshot)
			{
				killstreak = 0;
			}

			if (!sets->visuals.hitmarker)
			{
				damage_did = 0;
				health_left = 0;
				killstreak = 0;
				timer = 0.f;
				kill_timer = 0.f;
				return;
			}

			if (timer == 0.f && kill_timer == 0.f) return;

			int centerx = global::screen.right / 2;
			int centery = global::screen.bottom / 2;

			if ((timer - global::realtime + 1.f) > 0.f)
			{
				float percent = timer - global::realtime + 1.f;
				int alpha = (int)(70.f * percent);
				string s_did = "did " + to_string(damage_did) + " hp";
				string s_left = "left " + to_string(health_left) + " hp";
				draw_box(centerx - 150, centery, s_did, alpha);
				draw_box(centerx + 150, centery, s_left, alpha);
			}
			else
			{
				damage_did = 0;
				health_left = 0;
				killstreak = 0;
				timer = 0.f;
			}

			if ((kill_timer - global::realtime + 0.1f) > 0.f)
			{
				float percent = kill_timer - global::realtime + 1.f;
				int alpha = (int)(127.f * percent);
				int y = (int)((centery / 10.f * percent) - (centery / 4.f));
				d3d::font::draw(d3d::font::hitmarker_big, centerx, y, color(255, 20, 20, alpha), DT_CENTER | DT_VCENTER, text.c_str());
			}
			else
			{
				kill_timer = 0.f;
			}
		}
	}

	namespace bomb_timer
	{
		inline bool planted = false;
		inline float explosion_time = 0.f;
		inline float defuse_time = 0.f;
		inline float f_exp_time = 0.f;
		inline float f_def_time = 0.f;

		inline void on_fire_event(igameevent* event, const char* name)
		{
			if (strcmp(name, "bomb_planted") == 0) {
				planted = true;
				explosion_time = 0.f;
				defuse_time = 0.f;
			} else if (strcmp(name, "bomb_exploded") == 0 || strcmp(name, "bomb_defused") == 0) {
				planted = false;
				explosion_time = 0.f;
				defuse_time = 0.f;
			} else if (strcmp(name, "bomb_begindefuse") == 0) {
				f_def_time = event->get_bool("haskit") ? 5.f : 10.f;
				defuse_time = global::curtime + f_def_time;
			} else if (strcmp(name, "bomb_abortdefuse") == 0) {
				defuse_time = 0.f;
			}
		}

		inline void on_draw()
		{
			if (!sets->visuals.enabled || !sets->visuals.bomb_timer || explosion_time == 0.f || !planted || f_exp_time <= 0.f) return;

			float percent = ((explosion_time - global::curtime) / f_exp_time);
			int x = (int)(global::screen.right * percent);

			d3d::prim::filled_box(0, 0, x, 20, color((int)(255.f - (255.f * percent)), (int)(255.f * percent), 0));
			d3d::font::draw(d3d::font::hitmarker_small, x + 5, 20, color((int)(255.f * percent), (int)(255.f - (255.f * percent)), 0), DT_CENTER, explosion_time > 0.f ? "%.1f" : "exploded", (explosion_time - global::curtime));
			
			if (defuse_time == 0.f || f_def_time <= 0.f) return;

			float percent2 = ((defuse_time - global::curtime) / f_def_time);
			percent = ((defuse_time - global::curtime) / f_exp_time);
			x = (int)(global::screen.right * percent);

			d3d::prim::filled_box(0, 0, x, 20, color(0, (int)(255.f - (255.f * percent2)), (int)(255.f * percent2)));
			d3d::font::draw(d3d::font::hitmarker_small, x + 5, 10, color(0, (int)(255.f * percent2), (int)(255.f - (255.f * percent2))), DT_CENTER | DT_VCENTER, "%.1f", (defuse_time - global::curtime));
		}
	}

	inline void on_fire_event(igameevent* event, const char* name)
	{
		if (strcmp(name, "round_start") == 0)
		{
			hitmarker::killstreak2 = 0;
			hitmarker::timer = hitmarker::kill_timer = bomb_timer::defuse_time = bomb_timer::explosion_time = legit::aimbot::kill_delay = 0.f;
			bomb_timer::planted = false;
			server::sounds.clear();
		}

		hitmarker::on_fire_event(event, name);
		bomb_timer::on_fire_event(event, name);
	}

	inline void on_draw()
	{
		hitmarker::on_draw();
		bomb_timer::on_draw();
	}
}
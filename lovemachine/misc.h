#pragma once
#include "game shit.h"
#include "game classes.h"
#include "game def's.h"
#include "settings.h"
#include "net shit.h"
#include "draw.h"

//ofstream myfile;

namespace misc
{
	namespace fake_ping
	{
		struct state_t
		{
			state_t(int in_seq, int in_rel, float time)
			{
				this->in_seq = in_seq;
				this->in_rel = in_rel;
				this->time = time;
			}

			int in_seq, in_rel;
			float time;
		};

		deque<state_t> states;

		void start(CNetChan* netchannel)
		{
			states.push_front({ netchannel->m_nInSequenceNr, netchannel->m_nInReliableState, _globals->realtime });
			// 2048????
			if (states.size() > 2048) states.pop_back();
		}

		void end(CNetChan* netchannel)
		{
			for (auto state : states)
			{
				//cout << "realtime: " << _globals->realtime << ", state.time: " << state.time << ", delta: " << (_globals->realtime - state.time)  << endl;
				if ((_globals->realtime - state.time) > (float)((float)sets->misc.fake_ping / 1000.f))
				{
					netchannel->m_nInSequenceNr = state.in_seq;
					netchannel->m_nInReliableState = state.in_rel;
					break; // ???
				}
			}
		}
	}

	void autopistol()
	{
		/*if (cvar(antismac).value)
		{
			return;
			if (!global::weapon->is_pistol())
				return;

			static bool last_shot = false;
			static bool should_fake = false;
			static bool disable = false;

			if (!last_shot && should_fake)
			{
				should_fake = false;
				disable = true;
				_engine->clientcmd_unrestricted("+attack");
			}
			else if (global::cmd->buttons & IN_ATTACK && global::key[VK_LBUTTON])
			{
				if (global::curtime >= global::weapon->next_primary_attack() &&
					(cvar(ap_percent).value >= 97 || ((rand() % 100) >= (100 - cvar(ap_percent).value))))
				{
					last_shot = true;
					should_fake = true;
				}
				else
				{
					global::cmd->buttons &= ~IN_ATTACK;
					last_shot = false;
				}

				disable = true;
			}
			else if (disable)
			{
				last_shot = false;
				should_fake = false;
				disable = false;
				_engine->clientcmd_unrestricted("-attack");
			}
		}
		else
		{*/

		static bool undo = false;

		if (!(global::cmd->buttons & IN_ATTACK) /*|| !global::key[VK_LBUTTON]*/ || !global::weapon || server::local.type != weap_pistol || global::curtime >= global::weapon->next_primary_attack() || (cvar(antismac).value && undo))
		{
			//if (undo)
			//{
			//	send_mouse(MOUSEEVENTF_RIGHTUP);
			//	undo = false;
			//}
			return;
		}

		//mouse_input(MOUSEEVENTF_LEFTUP);
		const auto id = global::weapon->get_weaponid();
		if (cvar(antismac).value /*&& id != weapon_usp && id != weapon_glock*/)
		{
			//unpress_left_mouse_key();

			//send_key(VK_LBUTTON, false);
			//send_mouse(MOUSEEVENTF_RIGHTDOWN);
			//undo = true;

			//if (global::weapon->get_weaponid() != weapon_usp && global::weapon->get_weaponid() != weapon_glock)
				//send_key(VK_RBUTTON, true);
		}
		else
		{
			global::cmd->buttons &= ~IN_ATTACK;
		}

		//global::cmd->command_number -= 1;
		//global::cmd->buttons &= ~IN_ATTACK;
		//}
	}

	inline void autostrafer()
	{
		if (!global::cmd || !global::local || !global::local->valid()) return;

		int movetype = global::local->get_movetype();
		if (movetype == 9 || movetype == 8) return;

		if (global::local->get_flags() & FL_ONGROUND) return;

		cvector velocity = global::local->get_velocity();
		float speed = velocity.Length2D();

		if (speed < 1.0f) return;

		static float old_yaw = 0.0f;
		float current_yaw = global::cmd->viewangles.y;
		float yaw_delta = current_yaw - old_yaw;
		old_yaw = current_yaw;

		while (yaw_delta > 180.0f) yaw_delta -= 360.0f;
		while (yaw_delta < -180.0f) yaw_delta += 360.0f;

		if (fabsf(yaw_delta) > 0.05f)
		{
			global::cmd->sidemove = (yaw_delta > 0.0f) ? -450.0f : 450.0f;
		}
		else if (global::cmd->mousedx != 0)
		{
			global::cmd->sidemove = (global::cmd->mousedx < 0) ? -450.0f : 450.0f;
		}
	}

	inline void autojump()
	{
		if (!global::cmd || !global::local || !global::local->valid()) return;

		int movetype = global::local->get_movetype();
		if (movetype == 9 || movetype == 8) return;

		if (global::cmd->buttons & IN_JUMP)
		{
			static bool bWasOnGround = false;
			int flags = global::local->get_flags();
			bool bIsOnGround = (flags & FL_ONGROUND) != 0;

			if (!bIsOnGround && !bWasOnGround)
			{
				global::cmd->buttons &= ~IN_JUMP;
			}
			bWasOnGround = bIsOnGround;
		}
	}

	inline bool shooting()
	{
		if ((global::weapon && global::weapon->get_weaponid() != weapon_none) &&
			((global::cmd->buttons & IN_ATTACK && global::weapon->next_primary_attack() <= global::curtime) ||
			(global::weapon->get_weaponid() == weapon_knife && global::cmd->buttons & IN_ATTACK2 && global::weapon->next_secondary_attack() <= global::curtime)))
			return true;

		return false;
	}

	void lag()
	{
		if (!sets->misc.fakelag_enabled)
		{
			global::sendpacket = true;
			return;
		}

		static int choked_ticks = 0;
		int target_choke = sets->misc.fakelag_limit > 0 ? sets->misc.fakelag_limit : 14;
		if (sets->misc.fakelag_random > 0)
		{
			target_choke = max(1, target_choke - (rand() % (sets->misc.fakelag_random + 1)));
		}

		if (shooting() || choked_ticks >= target_choke)
		{
			choked_ticks = 0;
			global::sendpacket = true;
		}
		else
		{
			global::sendpacket = false;
			choked_ticks++;
		}
	}

	void nightmode()
	{
		static bool in_nm = false;
		if (in_nm != sets->visuals.nightmode || global::map_changed)
		{
			in_nm = sets->visuals.nightmode;
			if (!_mat_sys) return;

			for (auto handle = _mat_sys->first_material(); handle != _mat_sys->invalid_material(); handle = _mat_sys->next_material(handle))
			{
				auto material = _mat_sys->get_material(handle);
				if (!material || IsBadReadPtr(material, sizeof(imaterial))) continue;

				auto group = material->get_texture_group_name();
				if (!group || IsBadReadPtr((void*)group, 1)) continue;

				if (strstr(group, "World") || strstr(group, "StaticProp") || strstr(group, "SkyBox"))
				{
					if (in_nm)
					{
						material->colour_modulate(0.15f, 0.15f, 0.25f);
					}
					else
					{
						material->colour_modulate(1.0f, 1.0f, 1.0f);
					}
				}
			}
		}
	}

	void remove()
	{
		// 1. Instant 100% No Flash via NetVars
		if (global::local && global::local->valid() && sets->visuals.remove[1])
		{
			if (offsets::flash_max_alpha)
				*(float*)((DWORD)global::local + offsets::flash_max_alpha) = 0.0f;
			if (offsets::flash_duration)
				*(float*)((DWORD)global::local + offsets::flash_duration) = 0.0f;
		}

		// 2. Comprehensive No Smoke Material System
		static bool st_smoke = false;
		if (st_smoke != sets->visuals.remove[0] || global::map_changed)
		{
			const char* smoke_materials[] = {
				"particle/vistasmokev1/vistasmokev1",
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_nearcull",
				"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
				"particle/smokesprites_0001",
				"particle/smokesprites_0002",
				"particle/smokesprites_0003",
				"particle/smokesprites_0004",
				"particle/smokesprites_0005",
				"particle/smokesprites_0006",
				"particle/smokesprites_0007",
				"particle/smokesprites_0008",
				"particle/smokesprites_0009",
				"particle/smokesprites_0010",
				"particle/smokesprites_0011",
				"particle/smokesprites_0012",
				"particle/smokesprites_0013",
				"particle/smokesprites_0014",
				"particle/smokesprites_0015",
				"particle/smokesprites_0016",
				"particle/particle_smokegrenade",
				"particle/particle_smokegrenade1",
				"particle/particle_smokegrenade2",
				"particle/particle_smokegrenade3"
			};

			for (const char* mat_name : smoke_materials)
			{
				if (!_mat_sys) continue;
				auto mat = _mat_sys->find_material(mat_name, "ClientEffect textures");
				if (mat && !IsBadReadPtr(mat, sizeof(DWORD)))
				{
					mat->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, sets->visuals.remove[0]);
					mat->set_materialvar_flag(MATERIAL_VAR_WIREFRAME, sets->visuals.remove[0]);
				}
				auto mat_other = _mat_sys->find_material(mat_name, "Other textures");
				if (mat_other && !IsBadReadPtr(mat_other, sizeof(DWORD)))
				{
					mat_other->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, sets->visuals.remove[0]);
					mat_other->set_materialvar_flag(MATERIAL_VAR_WIREFRAME, sets->visuals.remove[0]);
				}
			}
			st_smoke = sets->visuals.remove[0];
		}

		// 3. Flash overlay materials
		static bool st_flash = false;
		if (st_flash != sets->visuals.remove[1] || global::map_changed)
		{
			if (_mat_sys)
			{
				auto material = _mat_sys->find_material("effects/flashbang", "ClientEffect textures");
				auto material2 = _mat_sys->find_material("effects/flashbang_white", "ClientEffect textures");

				if (material && !IsBadReadPtr(material, sizeof(DWORD)))
					material->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, sets->visuals.remove[1]);
				if (material2 && !IsBadReadPtr(material2, sizeof(DWORD)))
					material2->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, sets->visuals.remove[1]);
			}
			st_flash = sets->visuals.remove[1];
		}
		global::map_changed = false;
	}

	void slow_walk()
	{
		if (!sets->misc.slow_walk || !global::cmd || !global::local || !global::local->valid())
			return;

		float speed = sets->misc.slow_walk_speed > 0.0f ? sets->misc.slow_walk_speed : 35.0f;
		float move_len = sqrtf(global::cmd->forwardmove * global::cmd->forwardmove + global::cmd->sidemove * global::cmd->sidemove);
		if (move_len > speed)
		{
			float ratio = speed / move_len;
			global::cmd->forwardmove *= ratio;
			global::cmd->sidemove *= ratio;
		}
	}

	inline void fake_duck()
	{
		if (!sets->misc.fake_duck || !global::cmd || !global::local || !global::local->valid())
			return;

		if (!(global::local->get_flags() & FL_ONGROUND))
			return;

		// Compensate movement speed
		if (global::cmd->forwardmove != 0.0f || global::cmd->sidemove != 0.0f)
		{
			float move_len = sqrtf(global::cmd->forwardmove * global::cmd->forwardmove + global::cmd->sidemove * global::cmd->sidemove);
			if (move_len > 0.0f && move_len < 400.0f)
			{
				float scale = 450.0f / move_len;
				global::cmd->forwardmove *= scale;
				global::cmd->sidemove *= scale;
			}
		}

		// Stable Fake Duck Choking without bouncing
		if (global::chocked_packets >= 14)
		{
			global::sendpacket = true;
			global::cmd->buttons |= IN_DUCK;
		}
		else
		{
			global::sendpacket = false;
			if (global::chocked_packets < 7)
			{
				global::cmd->buttons &= ~IN_DUCK;
			}
			else
			{
				global::cmd->buttons |= IN_DUCK;
			}
		}
	}

	void pure_bypass()
	{
		if (!_cvar) return;
		auto sv_pure = _cvar->find_var("sv_pure");
		if (sv_pure)
		{
			sv_pure->set_value(0);
		}
	}

	void run()
	{
		if (sets->misc.pure_bypass) pure_bypass();
		if (!global::local || !global::local->valid()) return;

		if (sets->misc.fl_spam_always || sets->misc.fl_spam.is() == bind_true)
			_engine->clientcmd_unrestricted("impulse 100");

		short lag_bind = sets->misc.lag_spam.is();
		if (sets->misc.lag_factor > 0 && (lag_bind == bind_no_key || lag_bind == bind_true))
			lag();
		else
			global::sendpacket = true;

		if (!*global::lock_cursor) return;

		if (sets->misc.autopistol) autopistol();

		if (sets->misc.autojump) autojump();

		if (sets->misc.autostrafer) autostrafer();

		if (sets->misc.slow_walk) slow_walk();

		if (sets->misc.fake_duck) fake_duck();
		
		if ((sets->misc.airstuck.is() == bind_true ||
			(sets->misc.slowmotion.is() == bind_true && ((GetTickCount64() % sets->misc.sm_speed) == 0))) &&
			!shooting()) 
			global::cmd->tick_count += 200;

		misc::draw::run();
		nightmode();
	}
}
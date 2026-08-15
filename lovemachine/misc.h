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

	void autostrafer()
	{
		if ((global::local->get_flags() & FL_ONGROUND) || global::cmd->mousedx == 0)
			return;

		if (cvar(antismac).value)
		{
			return;
		}

		global::cmd->sidemove = global::cmd->mousedx < 0 ? -400.0f : 400.0f;

		//if (!cvar(antismac).value)
		/*{
			//static bool disable = false;
			if ((global::local->get_flags() & FL_ONGROUND) || !global::key[VK_SPACE] || global::cmd->mousedy == 0.f)
			{
				//if (disable)
				//{
				//	_engine->clientcmd_unrestricted("-forward");
				//	_engine->clientcmd_unrestricted("-back");
				//	_engine->clientcmd_unrestricted("-moveleft");
				//	_engine->clientcmd_unrestricted("-moveright");
				
				//	disable = false;
				//}
				return;
			}

			if (cvar(antismac).value)
			{
				global::cmd->mousedy < 0 ? send_key(0x41, true) : send_key(0x44, true);
				global::cmd->mousedy < 0 ? send_key(0x44, false) : send_key(0x41, false);
			}
			else global::cmd->sidemove = global::cmd->mousedy < 0.f ? -400 : 400;*/

			/*if (global::key[0x41]) // A
			{
				if (global::cmd->buttons & IN_MOVELEFT)
				{
					global::cmd->command_number -= 1;
					global::cmd->buttons &= ~IN_MOVELEFT;
				}
				if (global::cmd->mousedx < -1) send_key(0x53, true);//_engine->clientcmd_unrestricted("+back");
				else if (global::cmd->mousedx > 1) send_key(0x57, true); //_engine->clientcmd_unrestricted("+forward");

				//disable = true;
			}
			else if (global::key[0x53]) // S // íå èäåàëüíî
			{
				if (global::cmd->buttons & IN_BACK)
				{
					global::cmd->command_number -= 1;
					global::cmd->buttons &= ~IN_BACK;
				}
				if (global::cmd->mousedx < -1) send_key(0x44, true);//_engine->clientcmd_unrestricted("+moveright");
				else if (global::cmd->mousedx > 1) send_key(0x41, true);//_engine->clientcmd_unrestricted("+moveleft");

				//disable = true;
			}
			else if (global::key[0x44]) // D // íå èäåàëüíî
			{
				if (global::cmd->buttons & IN_MOVERIGHT)
				{
					global::cmd->command_number -= 1;
					global::cmd->buttons &= ~IN_MOVERIGHT;
				}
				if (global::cmd->mousedx < -1) send_key(0x57, true);//_engine->clientcmd_unrestricted("+forward");
				else if (global::cmd->mousedx > 1) send_key(0x53, true);//_engine->clientcmd_unrestricted("+back");

				//disable = true;
			}
			else // W èëè ïî äåôîëòó âïåðåä
			{
				//if (global::key[0x57]) _engine->clientcmd_unrestricted("-forward");
				if (global::cmd->buttons & IN_FORWARD)
				{
					global::cmd->command_number -= 1;
					global::cmd->buttons &= ~IN_FORWARD;
				}
				if (global::cmd->mousedx < -1) send_key(0x41, true);//_engine->clientcmd_unrestricted("+moveleft");
				else if (global::cmd->mousedx > 1) send_key(0x44, true);//_engine->clientcmd_unrestricted("+moveright");

				//disable = true;
			}*/
		//}
	}

	void autojump()
	{
		if (!global::cmd || !global::local || !global::local->valid()) return;
		if (global::cmd->buttons & IN_JUMP)
		{
			if (!(global::local->get_flags() & FL_ONGROUND))
			{
				global::cmd->buttons &= ~IN_JUMP;
			}
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
		static bool st_remove[2] = { false, false };

		if (st_remove[0] != sets->visuals.remove[0] || global::map_changed)
		{
			//auto material = _mat_sys->find_material(/*"particle/smokesprites_0015"*//*"particle/smokesprites_0015"*/"particle/smoke1/smoke1_nearcull2", "Other textures");
			//auto material2 = _mat_sys->find_material(/*"particle/smoke1/smoke1"*/"particle/vistasmokev1/vistasmokev1_nearcull", "Other textures");
			auto material3 = _mat_sys->find_material("particle/smokesprites_0001"/*"particle/vistasmokev1/vistasmokev1_nearcull"*/, "ClientEffect textures");
			auto material4 = _mat_sys->find_material("particle/particle_smokegrenade", "ClientEffect textures");
			auto material5 = _mat_sys->find_material("particle/particle_smokegrenade1", "ClientEffect textures");

			if (sets->visuals.remove[0])
			{
				//material->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
				//material2->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
				material3->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
				material4->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
				material5->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
			}
			else
			{
				//material->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
				//material2->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
				material3->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
				material4->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
				material5->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
			}

			st_remove[0] = sets->visuals.remove[0];
		}
		
		if (st_remove[1] != sets->visuals.remove[1] || global::map_changed)
		{
			auto material = _mat_sys->find_material("effects/flashbang", "ClientEffect textures");
			auto material2 = _mat_sys->find_material("effects/flashbang_white", "ClientEffect textures");

			if (sets->visuals.remove[1])
			{
				material->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
				material2->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, true);
			}
			else
			{
				material->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
				material2->set_materialvar_flag(MATERIAL_VAR_NO_DRAW, false);
			}

			st_remove[1] = sets->visuals.remove[1];
		}

		global::map_changed = false;
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
		pure_bypass();
		if (!global::local->valid()) return;

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
		
		if ((sets->misc.airstuck.is() == bind_true ||
			(sets->misc.slowmotion.is() == bind_true && ((GetTickCount64() % sets->misc.sm_speed) == 0))) &&
			!shooting()) 
			global::cmd->tick_count += 200;

		misc::draw::run();

		// Thirdperson logic (Bypass sv_cheats for LAN & Multiplayer servers)
		static bool last_tp_state = false;
		if (sets->visuals.thirdperson != last_tp_state)
		{
			last_tp_state = sets->visuals.thirdperson;
			auto sv_cheats = _cvar ? _cvar->find_var("sv_cheats") : nullptr;
			if (sv_cheats)
			{
				int old_val = sv_cheats->m_nValue;
				sv_cheats->m_nValue = 1;
				if (sets->visuals.thirdperson)
					_engine->clientcmd_unrestricted("thirdperson");
				else
					_engine->clientcmd_unrestricted("firstperson");
				sv_cheats->m_nValue = old_val;
			}
			else
			{
				if (sets->visuals.thirdperson)
					_engine->clientcmd_unrestricted("thirdperson");
				else
					_engine->clientcmd_unrestricted("firstperson");
			}
		}

		if (sets->visuals.thirdperson && _input)
		{
			*_input->m_fCameraInThirdPerson() = true;
			_input->m_vecCameraOffset()->z = sets->visuals.thirdperson_dist;
			if (sets->visuals.thirdperson_reverse)
			{
				_input->m_vecCameraOffset()->y = 180.0f;
			}
			else
			{
				_input->m_vecCameraOffset()->y = 0.0f;
			}
		}
		else if (_input)
		{
			*_input->m_fCameraInThirdPerson() = false;
		}

		nightmode();
	}
}
#pragma once
#include "includes.h"
#include "definitions.h"
#include "color.h"
#include "vector.h"

class cusercmd;
class centity;
class cweapon;

namespace global
{
	inline hwnd window;
	inline hmodule dll;
	inline bool key[0xFE + 1];
	inline bool key_do[0xFE + 1];
	inline bool key_click[0xFE + 1];
	inline float key_timer[0xFE + 1];
	inline point mouse;
	inline rect screen;
	inline cusercmd* cmd = nullptr;
	inline bool map_changed = false;
	inline bool* lock_cursor = nullptr;
	inline bool sendpacket = false;
	inline int chocked_packets = 0;
	inline int local_id = 0;
	inline centity* local = nullptr;
	inline cweapon* weapon = nullptr;
	inline centity* local_observed = nullptr;
	inline float curtime = 0.0f;
	inline float realtime = 0.0f;
	inline bool unhook = false;
	inline qangle last_sent_angles = qangle(0, 0, 0);
}

namespace server
{
	inline int max_players = 0;

	struct sound
	{
		cvector position;
		float time;
		color col;
	};

	inline deque<sound> sounds;

	struct local_struct
	{
		int type = 0;
	};
	inline local_struct local;

	struct player_struct
	{
	};
	inline player_struct players[64];
}
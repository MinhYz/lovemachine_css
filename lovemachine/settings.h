#pragma once
#include "includes.h"
#include "global.h"
#include "color.h"
#include "cvars.h"

enum bind_is : short
{
	bind_no_key = -1,
	bind_false = 0,
	bind_true = 1
};

struct bind_t
{
	short is()
	{
		if (key == 0)
			return bind_no_key;
		if ((hold == 1 && global::key[key]) || (hold == 0 && global::key_do[key]))
			return bind_true;
		return bind_false;
	}

	bool hold;
	int key;
};

class settings
{
public:
	settings()
	{
		//cvars.setup();
		//TODO : ÃªÃ®Ã­Ã´Ã¨Ã£Ã¨
		//CreateDirectory(L"C:/lovemachine", NULL);
		//CreateDirectory(L"C:/lovemachine/configs", NULL);
	}

	struct
	{
		bool opened = false, hovered = false, console = false, panic = false; // TODO : Ã¢Ã¬Ã¥Ã±Ã²Ã® panic Ã«Ã³Ã·Ã¸Ã¥ Ã±Ã¤Ã¥Ã«Ã Ã²Ã¼ Ã Ã­Ã«Ã®Ã Ã¤ Ã·Ã¨Ã²Ã 
		int cont_hovered = -1;
		int x = 100, y = 100, cur_tab = 0;
		int menu_key = VK_INSERT;
	} menu;

	struct
	{
		bool opened = false, hovered = false;
		bool style[6] = { false, true, false, true, true, true };
		int x = 400, y = 100, mx = 200, my = 40;
	} info;

	struct
	{
		bool opened = false, hovered = false;
		int x = 670, y = 300, mx = 150, my = 30;
	} spec;

	struct
	{
		bool enabled = false;
		bool friends = false;
		bool knifebot = false;

		struct
		{
			bool enabled = true;
			bool hitbox[5] = { true, false, true, false, false };
			bool style[4] = { false, true, true, true };
			int ticks = 5;
		} backtrack;

		struct
		{
			bool hitbox[5] = { true, true, true, false, false };
			float fov = 4.f;
			float smooth[2] = { 37.9f, 38.5f };
			float rcs[2] = { 2.f, 2.f };
			float humanize[2] = { 1.6f, 1.2f };
			float kill_delay = 0.5f;
			float shot_delay = 0.15f;

			// Disable Aimbot Conditions (Neverlose menu-introduction)
			bool disable_flashed = false;
			bool disable_in_smoke = false;
			bool disable_in_jump = false;

			// Visual Draw FOV
			bool draw_fov = false;

			// First vs Spray/Other Bullets
			bool use_first_bullet_settings = true;
			float other_fov = 6.0f;
			float other_smooth[2] = { 25.0f, 25.0f };

			// Recoil Control (RCS)
			bool enable_rcs = true;
			bool standalone_rcs = false;

			// Legit Autowall Minimum Damage
			float autowall_min_damage = 10.0f;

			// Cranium Aimbot Extensions
			bool auto_crouch = false;
			bool predicted_position = false;
			bool norecoil = false;
			bool nospread = false;
			int silent_mode = 0; // 0: Off, 1: Client Side, 2: Server Side, 3: Perfect Silent
			int fov_selection = 0; // 0: Static, 1: Dynamic, 2: Distance-Based
			int aim_usage = 0; // 0: Always, 1: On Key Hold, 2: On Attack
		} aim;

		struct
		{
			bool _enabled = false;
			bind_t bind = { false, 0 };
			bool hitbox[5] = { true, true, true, false, false };
			float delay = 0.07f;
		} trigger;
	} legit;

	struct
	{
		bool enabled = false;
		bool friends = false;
		bool autowall = false;
		bool autoscope = false;
		bool autoshoot = false;
		bool autostop = false;
		bool silent = false;
		bool hitbox[5] = { true, false, true, false, false };
		float hitchance = 0.f;

		// CS:S Compatible Neverlose Additions
		float min_damage_visible = 10.0f;
		float min_damage_autowall = 15.0f;
		int body_aim_mode = 0; // 0: Default, 1: Prefer, 2: Force
		bool override_resolver = false;

		// Anti-Aim / Spinbot / Magic Bullet
		bool spinbot = false;
		int spinbot_mode = 0; // 0: Disabled, 1: Server-Side, 2: Client-Side
		bool magic_bullet = false;
		float spin_speed = 25.0f;
		float spin_speed_client = 25.0f;
		float spin_speed_server = 45.0f;
		int pitch_aa = 0;
		int yaw_aa = 0;
	} rage;

	struct
	{
		bool enabled = false;
		bool friends = false;
		int chams = 0;
		bool chams_style[3] = { false, false, false };
		bool other_chams[3] = { false, false, false };
		bool other_style[5] = { false, false, false, false, false };
		color chams_t = color(150, 255, 13);
		color chams_ct = color(50, 255, 150);
		int crosshair = 0;
		bool esp_filter[6] = { true, true, true, true, true, true };
		bool esp_show[6] = { true, true, true, true, true, true };
		bool esp_bar[4] = { true, true, true, true };
		bool esp_check[2] = { true, true };
		color esp_t = color(210, 35, 16);
		color esp_ct = color(50, 90, 210);
		bool fade = true;
		bool defuser_only_if_need = false;
		int ak47_skin = 0;
		int deagle_skin = 0;
		int knife_skin = 0;
		bool other_skins[2] = { false, false };
		bool remove[2] = { false, false };
		bool hitmarker = false;
		bool bomb_timer = false;

		// Viewmodel FOV (Arm/Hand Length)
		float viewmodel_fov = 90.0f;

		// Asian Hat (Mũ Asian / Rice Hat)
		bool asian_hat = false;
		color asian_hat_color = color(255, 200, 50);
		float asian_hat_size = 20.0f;
		float asian_hat_height = 10.0f;

		// Skeleton & Nightmode & Advanced ESP (Neverlose)
		bool skeleton = true;
		bool nightmode = false;
		bool offscreen_esp = false;
		float oof_size = 15.0f;
		float oof_radius = 120.0f;
		bool footstep_rings = false;
		bool armor_bar = true;
		bool ammo_bar = true;
		bool flag_hk = true;
		bool flag_scoped = true;
		bool flag_reloading = true;
		bool flag_flashed = true;
		bool sound_esp = false;
		bool glow_esp = false;

		// World Modulation & Atmosphere (Neverlose Image 4)
		float asus_walls = 100.0f; // 0-100% Wall Opacity (ASUS Wallhack)
		float asus_props = 100.0f; // 0-100% Props Opacity
		int skybox_mode = 0;       // 0: Default, 1: Night, 2: Baggage, 3: Cold, 4: Clear
		bool enable_fog = false;
		float fog_start = 0.0f;
		float fog_end = 2000.0f;
		float fog_density = 0.5f;

		// Crosshairs, Indicators & FX (Neverlose Image 4)
		bool noscope_crosshair = false;
		bool autowall_crosshair = false;
		bool damage_indicator = false;
		bool hit_sound = false;
		float hit_sound_volume = 50.0f;
		bool bullet_impacts = false;

		// Grenades & C4 Bomb (Neverlose Image 2 & 3)
		bool grenade_esp = true;
		bool grenade_trajectory = false;
		bool grenade_warning = false;
		bool bomb_esp = true;
		bool bomb_defuse_radius = false;

		// Viewmodel XYZ Offsets & Camera (Neverlose Image 5)
		float viewmodel_x = 0.0f;
		float viewmodel_y = 0.0f;
		float viewmodel_z = 0.0f;
		bool force_fov_in_scope = false;
		bool remove_scope = false;
		bool aspect_ratio_override = false;
		float aspect_ratio_val = 1.77f;

		// Customizable ESP Bar & Element Positions
		int health_bar_pos = 0; // 0: Left, 1: Right, 2: Top, 3: Bottom
		int armor_bar_pos = 1;  // 0: Left, 1: Right, 2: Top, 3: Bottom
		int name_pos = 0;       // 0: Top, 1: Bottom, 2: Right, 3: Left
		int weapon_pos = 1;     // 0: Top, 1: Bottom, 2: Right, 3: Left
		int hp_text_style = 0;  // 0: Next to Bar, 1: Inside Bar, 2: Bottom Text

		// Thirdperson & Inverted Angle
		bool thirdperson = false;
		float thirdperson_dist = 120.0f;
		bool thirdperson_reverse = false;
	} visuals;

	struct
	{
		bool autopistol = false;
		bool autojump = false;
		bool autostrafer = false;
		int aj_percent = 50;
		bool fl_spam_always = false;
		int fake_ping = 0;
		bool killshot = false;
		int lag_mode = 0;
		bind_t lag_spam = { false, 0 };
		int lag_factor = 0;
		bind_t fl_spam = { false, 0 };
		bind_t airstuck = { false, 0 };
		bind_t slowmotion = { false, 0 };
		int sm_speed = 4;
		bind_t record = { false, VK_NUMPAD7 };
		bind_t play = { false, VK_NUMPAD8 };

		// Neverlose CS:S Movement & Anti-Aim Exploits
		bool fakelag_enabled = false;
		int fakelag_limit = 14;
		int fakelag_random = 0;
		bool slow_walk = false;
		float slow_walk_speed = 35.0f;
		bool fake_duck = false;

		// Cranium Misc Extensions
		bool norecoil = false;
		bool pure_bypass = false;      // SV_Pure 1/2 Bypass
		bool fast_ladder = false;      // Fast Ladder Climb
		bool antismac = false;         // Anti SMAC Server Bypass
		bool circle_strafe = false;    // Circle Strafe Helper
		bind_t circle_strafe_bind = { false, 'T' };
		bool edge_jump = false;        // Edge Jump Auto Helper
		bind_t edge_jump_bind = { false, 0 };

		int draw_mode = 0;
		float draw_time = 0.1f;
		color draw_color = color(0, 0, 120);
		bind_t draw_start = { false, VK_XBUTTON1 };
		bind_t draw_clear = { false, VK_XBUTTON2 };
	} misc;
};

inline settings* sets = new settings();
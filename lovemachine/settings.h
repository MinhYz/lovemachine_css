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
		int menu_key_idx = 0; // 0: INSERT, 1: DELETE, 2: HOME, 3: END, 4: TILDE (~), 5: F11, 6: F12, 7: RSHIFT
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

		// Anti-Aim (Pitch & Yaw Angles including Spinbot)
		bool anti_aim = false;
		bool spinbot = false;
		int pitch_aa = 0; // 0: Off, 1: Down (Emotion 89°), 2: Up (-89°), 3: Zero (0°)
		int yaw_aa = 0;   // 0: Off, 1: Backwards (180°), 2: Spinbot (360°), 3: Jitter (±90°), 4: Sideways (90°)
		float spin_speed = 25.0f;
		bool magic_bullet = false;
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

		// Viewmodel FOV & Hand/Arm Color Customization
		float viewmodel_fov = 90.0f;
		float viewmodel_offset_x = 0.0f;
		float viewmodel_offset_y = 0.0f;
		float viewmodel_offset_z = 0.0f;
		int hand_chams = 0; // 0: Disabled, 1: Solid Color, 2: Metallic Shaded, 3: Wireframe
		color hand_color = color(255, 40, 90);
		float fov = 0.0f;

		// Custom 3D Character Model (Cissia ZZZ / Phoenix)
		bool enable_custom_model = false;
		bool custom_model_local_only = true;
		int model_selection = 0;
		char custom_model_path_input[256] = "models/player/cissia_zzz.mdl";

		// Skeleton & Nightmode & Advanced ESP
		bool skeleton = true;
		bool nightmode = false;
		float asus_walls = 100.0f;
		bool offscreen_esp = false;
		float oof_size = 15.0f;
		float oof_radius = 120.0f;
		bool footstep_rings = false;
		bool flag_hk = true;
		bool flag_scoped = true;
		bool flag_reloading = true;
		bool flag_flashed = true;
		bool sound_esp = false;

		// 3D Head Accessories & Attachments (Unified Selection)
		int head_accessory = 0; // 0: Disabled, 1: Asian Rice Hat, 2: Angel Halo, 3: Devil Horns, 4: Royal Crown, 5: Cyber Cat Ears
		color head_accessory_color = color(230, 215, 175);
		float head_accessory_size = 20.0f;
		float head_accessory_height = 10.0f;

		bool asian_hat = false;
		color asian_hat_color = color(230, 215, 175);
		float asian_hat_size = 20.0f;
		float asian_hat_height = 10.0f;

		bool halo_ring = false;
		color halo_color = color(255, 230, 80);
		float halo_radius = 12.0f;

		bool devil_horns = false;
		color devil_horns_color = color(255, 25, 40);
		float devil_horns_size = 10.0f;

		bool crown = false;
		color crown_color = color(255, 215, 0);
		float crown_size = 14.0f;

		bool cat_ears = false;
		color cat_ears_color = color(255, 105, 180);
		float cat_ears_size = 10.0f;

		// 3D Body & Ground FX (Energy Wings, Magic Circle Runes)
		bool energy_wings = false;
		color energy_wings_color = color(190, 20, 35);
		float energy_wings_size = 30.0f;

		bool magic_circle = false;
		color magic_circle_color = color(0, 220, 255);
		float magic_circle_size = 35.0f;

		// 3D Laser Bullet Tracers & Bullet Impact Rings
		bool bullet_tracers = false;
		color bullet_tracers_color = color(0, 235, 255);
		float bullet_tracers_duration = 2.5f;
		bool impact_rings = false;
		color impact_rings_color = color(255, 60, 120);

		// 3D Weapon Laser Sight (Nòng súng phát tia laser 3D)
		bool laser_sight = false;
		color laser_sight_color = color(255, 0, 80);
		float laser_sight_length = 1500.0f;

		// Kill Visual Effects (Hiệu ứng hạ gục kẻ địch)
		int kill_effect = 1; // 0: Off, 1: 3D Lightning Strike, 2: Blood Particle Fountain, 3: Ascending Skull, 4: Cyber Implosion
		color kill_effect_color = color(0, 255, 255);

		// 3D Dropped Items & C4 Sky Beams (Cột sáng đồ rơi)
		bool item_light_beams = false;
		color item_light_beams_color = color(0, 200, 255);

		// 3D Hitbox Capsule on Hit (Lồng hitbox trúng đạn)
		bool hit_capsules = false;
		color hit_capsules_color = color(255, 255, 255);
		float hit_capsules_duration = 2.0f;

		// Screen Hit Pulse & Vignette FX (Hiệu ứng viền màn hình khi bắn)
		bool screen_hit_pulse = false;
		color screen_hit_pulse_color = color(0, 255, 180);

		// Grenade Trajectory & FOV Circle
		bool grenade_trajectory = false;
		color grenade_trajectory_color = color(255, 50, 50);
		bool grenade_warning = false;
		bool fov_circle = false;
		color fov_circle_color = color(255, 255, 255);

		// Floating 3D Damage Indicator Numbers
		bool damage_indicator = false;
		color damage_indicator_color = color(255, 220, 0);

		// Customizable ESP Bar & Element Positions
		int health_bar_pos = 0; // 0: Left, 1: Right, 2: Top, 3: Bottom
		int armor_bar_pos = 1;  // 0: Left, 1: Right, 2: Top, 3: Bottom
		int name_pos = 0;       // 0: Top, 1: Bottom, 2: Right, 3: Left
		int weapon_pos = 1;     // 0: Top, 1: Bottom, 2: Right, 3: Left
		int hp_text_style = 0;  // 0: Next to Bar, 1: Inside Bar, 2: Bottom Text

		// Thirdperson & Inverted Angle & Dynamic Trail Modes
		bool thirdperson = false;
		float thirdperson_dist = 120.0f;
		bool thirdperson_reverse = false;
		bool rainbow_trail = false;
		int trail_mode = 1; // 0: Off, 1: Rainbow Wave, 2: Electric Cyan, 3: Inferno Fire, 4: Cyber Violet Plasma
		float trail_length = 30.0f;
		float rainbow_trail_speed = 1.0f;
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
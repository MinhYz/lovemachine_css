#pragma once
#include "includes.h"
#include "definitions.h"
#include "console.h"
#include "memory.h"
#include "global.h"
#include "interfaces.h"
#include "game def's.h"
#include "game shit.h"
#include "game classes.h"
#include "d3d.h"
#include "menu.h"
#include "legit.h"
#include "models.h"
#include "esp.h"
#include "misc.h"
#include "ragebot.h"
#include "events.h"
#include "surface.h"
#include "net shit.h"
#include "font_astrium.h"

using namespace d3d;
using namespace game;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hooks
{	
	wndproc o_wndproc;
	LRESULT __stdcall wndproc_hook(hwnd wnd, uint msg, wparam w_param, lparam l_param)
	{
		global::realtime = (float)GetTickCount64() / 1000.f;

		// Toggle Menu Hotkey (Default INSERT or selected key)
		if (msg == WM_KEYDOWN && !(l_param & 0x40000000))
		{
			if (Menu::is_binding_key)
			{
				if (w_param != VK_ESCAPE)
				{
					sets->menu.menu_key = (int)w_param;
				}
				Menu::is_binding_key = false;
			}
			else if (w_param == static_cast<WPARAM>(sets->menu.menu_key))
			{
				Menu::show_menu = !Menu::show_menu;
				sets->menu.opened = Menu::show_menu;
			}
		}

		// Track input states for cheat logic
		switch (msg)
		{
		case WM_LBUTTONDOWN:
			global::key[VK_LBUTTON] = true;
			if (global::key_timer[VK_LBUTTON] == 0.f)
				global::key_timer[VK_LBUTTON] = global::realtime;
			break;
		case WM_LBUTTONUP:
			global::key[VK_LBUTTON] = false;
			global::key_timer[VK_LBUTTON] = 0.f;
			break;
		case WM_RBUTTONDOWN:
			global::key[VK_RBUTTON] = true;
			break;
		case WM_RBUTTONUP:
			global::key[VK_RBUTTON] = false;
			break;
		case WM_MBUTTONDOWN:
			global::key[VK_MBUTTON] = true;
			break;
		case WM_MBUTTONUP:
			global::key[VK_MBUTTON] = false;
			break;
		case WM_XBUTTONDOWN:
			global::key[VK_XBUTTON1 + (GET_XBUTTON_WPARAM(w_param) - 1)] = true;
			break;
		case WM_XBUTTONUP:
			global::key[VK_XBUTTON1 + (GET_XBUTTON_WPARAM(w_param) - 1)] = false;
			break;
		case WM_KEYDOWN:
			global::key[w_param] = true;
			break;
		case WM_KEYUP:
			global::key[w_param] = false;
			break;
		case WM_MOUSEMOVE:
			global::mouse.x = (signed short)(l_param);
			global::mouse.y = (signed short)(l_param >> 16);
			break;
		default: break;
		}

		// When menu is OPEN, pass inputs to ImGui and trap them from game
		if (Menu::show_menu && !sets->menu.panic)
		{
			if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(wnd, msg, w_param, l_param))
				return true;
			return true;
		}

		// When menu is CLOSED, let all inputs pass straight to game Engine
		return CallWindowProc(o_wndproc, wnd, msg, w_param, l_param);
	}

	bool key[0xFE + 1];
	memory::vthook* d3d9;
	using endscene_fn = long(__stdcall*)(IDirect3DDevice9* device);
	endscene_fn o_endscene;
	long __stdcall endscene_hook(IDirect3DDevice9* device)
	{
		static bool once = true, fonts = false;

		if (once)
		{
			d3d::device = device;
			console::write_hex("/d3d/ device", (dword)device, darkgreen);
			d3d::font::setup(d3d::font::tab, "Gotham Pro Medium", 24, FW_DONTCARE);
			d3d::font::setup(d3d::font::cont, "Gotham Pro", 15, fw_medium);
			d3d::font::setup(d3d::font::hitmarker_big, "Gotham Pro Black", 30, FW_BOLD);
			d3d::font::setup(d3d::font::hitmarker_small, "Gotham Pro Medium", 19, FW_BOLD);
			d3d::font::setup(d3d::font::esp, "Gotham Pro Medium", 16, FW_DONTCARE);
			d3d::font::reset();
			global::screen = get_screen_size();

			// Initialize ImGui DX9 & Win32 Backends
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigDebugHighlightIdConflicts = false;

			// Check font paths
			const char* font_dirs[] = {
				"scripts/fonts/",
				"assets/fonts/",
				"cstrike/fonts/",
				"../scripts/fonts/",
				"../assets/fonts/"
			};

			std::string found_font_dir = "";
			for (const char* dir : font_dirs)
			{
				std::string test_file = std::string(dir) + "AstriumTabs.ttf";
				FILE* f = fopen(test_file.c_str(), "rb");
				if (f)
				{
					fclose(f);
					found_font_dir = dir;
					break;
				}
			}

			if (!found_font_dir.empty())
			{
				// 1. Base Main Font
				ImFontConfig font_cfg;
				font_cfg.OversampleH = 2;
				font_cfg.OversampleV = 2;
				font_cfg.PixelSnapH = true;
				std::string f_main = found_font_dir + "Museo500.ttf";
				Menu::font_main = io.Fonts->AddFontFromFileTTF(f_main.c_str(), 14.0f, &font_cfg);
				if (!Menu::font_main)
				{
					std::string f_goth = found_font_dir + "GothamPro.ttf";
					Menu::font_main = io.Fonts->AddFontFromFileTTF(f_goth.c_str(), 14.0f, &font_cfg);
				}

				// 2. FontAwesome Icon Merge into Default Font
				ImFontConfig fa_cfg;
				fa_cfg.MergeMode = true;
				fa_cfg.PixelSnapH = true;
				fa_cfg.OversampleH = 2;
				fa_cfg.OversampleV = 2;
				static const ImWchar fa_ranges[] = { 0xf000, 0xf976, 0 };
				std::string f_fa = found_font_dir + "FontAwesome.ttf";
				io.Fonts->AddFontFromFileTTF(f_fa.c_str(), 13.0f, &fa_cfg, fa_ranges);

				// 3. Large Brand Header Font
				ImFontConfig brand_cfg;
				brand_cfg.OversampleH = 2;
				brand_cfg.OversampleV = 2;
				brand_cfg.PixelSnapH = true;
				std::string f_brand = found_font_dir + "Museo900.ttf";
				Menu::font_brand_title = io.Fonts->AddFontFromFileTTF(f_brand.c_str(), 24.0f, &brand_cfg);

				// 4. Gamesense Vector Icon Font (AstriumTabs)
				ImFontConfig skeet_cfg;
				skeet_cfg.OversampleH = 2;
				skeet_cfg.OversampleV = 2;
				skeet_cfg.PixelSnapH = true;
				static const ImWchar skeet_ranges[] = { 0x0020, 0x00FF, 0 };
				std::string f_skeet = found_font_dir + "AstriumTabs.ttf";
				Menu::font_skeet_icons = io.Fonts->AddFontFromFileTTF(f_skeet.c_str(), 22.0f, &skeet_cfg, skeet_ranges);
			}

			if (!Menu::font_skeet_icons)
			{
				ImFontConfig skeet_cfg;
				skeet_cfg.OversampleH = 2;
				skeet_cfg.OversampleV = 2;
				skeet_cfg.PixelSnapH = true;
				skeet_cfg.FontDataOwnedByAtlas = false;
				static const ImWchar skeet_ranges[] = { 0x0020, 0x00FF, 0 };
				Menu::font_skeet_icons = io.Fonts->AddFontFromMemoryTTF((void*)assets_fonts_AstriumTabs_ttf, sizeof(assets_fonts_AstriumTabs_ttf), 22.0f, &skeet_cfg, skeet_ranges);
			}

			if (!Menu::font_main)
			{
				Menu::font_main = io.Fonts->AddFontDefault();
			}

			ImGui_ImplWin32_Init(global::window);
			ImGui_ImplDX9_Init(device);
			Menu::SetupStyle();

			once = false;
		}

		if (sets->menu.panic)
		{
			sets->menu.opened = false;

			if (global::key[VK_NUMPAD5])
				sets->menu.panic = false;

			return o_endscene(device);
		}

		if (_engine && !_engine->in_game() && !global::map_changed)
		{
			events::bomb_timer::defuse_time = events::bomb_timer::explosion_time = events::hitmarker::timer = events::hitmarker::kill_timer = 0.f;
			//ZeroMemory(legit::backtrack::records, sizeof(legit::backtrack::records));
			legit::backtrack::draw = false;
			for (int id = 0; id < 64; id++)
				for (int tick = 0; tick < 10; tick++)
					legit::backtrack::records[id][tick].valid = false;
			legit::backtrack::best_record.valid = false;
			if (!fonts)
			{
				surf::font::setup(surf::font::esp, "Gotham Pro", 17, fw_normal, ff_antialias | ff_dropshadow);
				fonts = true;
			}
			global::map_changed = true;
		}

		//TODO : Ã³Ã±Ã®Ã¢Ã¥Ã°Ã¸Ã¥Ã­Ã±Ã²Ã¢Ã®Ã¢Ã Ã²Ã¼
		for (int key_id = 0; key_id < 0xFE + 1; key_id++)
		{
			if (key[key_id] != global::key[key_id])
			{
				key[key_id] = global::key[key_id];

				global::key_click[key_id] = global::key[key_id];
			}
			else
			{
				global::key_click[key_id] = false;
			}

			if (global::key_click[key_id])
			{
				global::key_do[key_id] = !global::key_do[key_id];
			}
		}

		/*if (_engine->is_screenshoting())
		{
			console::write("TAKING SCREENSHOT", red);
			return o_endscene(device);
		}*/

		/*if (_engine && _engine->is_connected() && _engine->in_game() && global::cmd)
		{
			auto point = cvector(10, 10, 10);
			cvector screen;
			auto result = w2s_keybode(point, screen);//_debug_overlay->screen_position(point, screen);
			//cout << "w2s result : " << result << ", screen.x : " << screen.x << ", screen.y : " << screen.y << endl;
			if (result == 1 && (obj::in_range(0, 0, screen.x, screen.y, global::screen.right, global::screen.bottom)))//w2s(point, screen))
			{
				prim::filled_box(screen.x - 5, screen.y - 5, screen.x + 5, screen.y + 5, color(255, 0, 0));
			}
		}*/

		//if (_engine->is_connected() && _engine->in_game() && global::cmd && global::cmd->command_number != 0 && global::local)
		//{
			/*cvector angle = global::cmd->viewangles;
			static auto stangle = angle;

			if (stangle != angle)
			{
				auto pmatrix = _engine->w2s_matrix();
				for (int i = 0; i < 4; i++)
					for (int j = 0; j < 4; j++)
					{
						global::w2s_matrix[i][j] = pmatrix[i][j];
						cout << "matrix[" << i << "][" << j << "] -> " << global::w2s_matrix[i][j] << ((i == 0 && j == 3) ? "\n" : ", ");
					}

				stangle = angle;
			}*/

		static bool st_crosshair = true;

		if (global::local && global::local->valid() && (sets->visuals.crosshair == 1 || (sets->visuals.crosshair == 2 && server::local.type == weap_snip)))//&& ((sets->visuals.crosshair == 1 && global::weapon->get_weaponid() != weapon_none) || (sets->visuals.crosshair == 2 && global::weapon->is_sniper())))
		{
			if (st_crosshair)
			{
				_engine->clientcmd_unrestricted("crosshair 0");
				st_crosshair = false;
			}

			int centerx = global::screen.right / 2;
			int centery = global::screen.bottom / 2;

			d3d::prim::filled_box(centerx - 5, centery, centerx + 6, centery + 1, color::lm());
			d3d::prim::filled_box(centerx, centery - 5, centerx + 1, centery + 6, color::lm());
		}
		else
		{
			if (!st_crosshair)
			{
				_engine->clientcmd_unrestricted("crosshair 1");
				st_crosshair = true;
			}
		}

		//if (sets->visuals.enabled)
		//	esp::run();
	//}

	//if (!_engine->in_game()) global::map_changed = true;

	/*if (sets->legit.enabled && (sets->legit.backtrack.enabled || (sets->legit.aim.fov > 0.f && sets->legit.aim.kill_delay > 0.f)))
	{
		static bool reset = true;

		if (reset && (!_engine->is_connected() || !_engine->in_game() || !global::local || !global::local->valid()))
		{
			ZeroMemory(legit::backtrack::records, sizeof(legit::backtrack::records));
			legit::backtrack::best_record = legit::backtrack::crecord();
			legit::backtrack::best_fov = 9999.f;
			legit::aimbot::kill_delay = 0.f;

			reset = false;
		}
		else if (!reset && _engine->is_connected() && _engine->in_game() && global::local && global::local->valid())
		{
			reset = true;
		}
	}*/

		legit::trigger::draw();

		if (sets->visuals.enabled && _engine->in_game() && global::local)
		{
			//esp::draw();
			events::on_draw();
		}

		if (Menu::show_menu || sets->menu.opened)
		{
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			Menu::Render();

			ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		}

		return o_endscene(device);
	}

	using reset_fn = long(__stdcall*)(IDirect3DDevice9 * device, D3DPRESENT_PARAMETERS * pp);
	reset_fn o_reset;
	long __stdcall reset_hook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
	{
		console::write("d3d reset", darkred);

		ImGui_ImplDX9_InvalidateDeviceObjects();
		d3d::font::restore();
		auto result = o_reset(device, pp);
		ImGui_ImplDX9_CreateDeviceObjects();
		d3d::font::reset();
		surf::font::setup(surf::font::esp, "Gotham Pro", 17, fw_normal, ff_antialias | ff_dropshadow);
		global::screen = get_screen_size();

		return result;
	}

	convar* sv_cheats = nullptr;

	memory::vthook* panel;
	using painttraverse_fn = void(__stdcall*)(unsigned int vguiPanel, bool forceRepaint, bool allowForce);
	painttraverse_fn o_painttraverse;
	void __stdcall painttraverse_hook(unsigned int vguiPanel, bool forceRepaint, bool allowForce)
	{
		o_painttraverse(vguiPanel, forceRepaint, allowForce);

		static unsigned int top_panel = 0;
		auto name = _panel->get_name(vguiPanel);
		//myfile << name << endl;
		if (top_panel == 0 && !strcmp(name, "MatSystemTopPanel"))
		{
			top_panel = vguiPanel;
			//surf::font::setup(surf::font::esp, "Gotham Pro Medium", 17, fw_normal, ff_antialias | ff_dropshadow);
			//surf::font::setup(surf::font::esp_big, "Gotham Pro Black", 30, fw_semibold, ff_antialias);
		}

		//if (!sv_cheats) sv_cheats = _cvar->find_var("sv_cheats");
		//else surf::font::draw(surf::font::esp_big, 100, 100, color::ptext(), null, to_str(sv_cheats->m_nValue).c_str());

		if (top_panel == 0 || top_panel != vguiPanel || !_engine->in_game() || sets->menu.panic) return;

		global::local_id = _engine->get_local_id();
		if (global::local_id > 0 && _ent_list)
			global::local = _ent_list->get_centity(global::local_id);

		if (sets->misc.draw_mode == 0) misc::draw::draw();

		if (!sets->visuals.enabled) return;

		esp::draw();

		/*const auto matrix = _engine->w2s_matrix();
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				global::w2s_matrix[i][j] = matrix[i][j];*/

		/*for (int id = 0; id < _engine->get_max_clients()/*_ent_list->get_highest_entity_index()*//*; id++)
		{
			auto entity = _ent_list->get_centity(id);
			if (!entity || !entity->valid() || entity->is_dormant()/* || entity->get_origin().IsZero()*//*) continue;

			//auto class_id = entity->get_clientnetworkable()->get_clientclass()->class_id;

			//if (class_id == CCSPlayer)
			//{
			//	server::max_players = id + 1;

				//if (entity->valid())
				//{
					auto weapon = entity->get_weapon();
					if (weapon)
					{
						auto type = weapon->get_type();
						if (type != weap_max)
						{
							if (id == global::local_id)
							{
								server::local.type = type;
							}
							else if (sets->visuals.enabled && sets->visuals.esp_filter[0] && sets->visuals.esp_show[3])
							{
								if (sets->visuals.esp_bar[3]) server::players[id].clip = weapon->get_clip1();
								server::players[id].weapon = weapon->get_name();
								server::players[id].valid = true;
							}
						}
						else
						{
							if (id == global::local_id) server::local.type = weap_max;
							else
							{
								server::players[id].clip = -1;
								server::players[id].weapon = "none";
							}
						}
					}
					//server::players[id].health = entity->get_clientnetworkable()->get_clientclass()->class_id;
				//}
			//}
			//else
			//{
			//	server::max_entity = id + 1;

			//	if (sets->visuals.enabled && sets->visuals.fancy_w2s)
			//	{
			//		server::entity[id].coord_frame = entity->get_coord_frame();
			//		server::entity[id].valid = true;
			//	}
			//}
		}*/

		/*for (int id = 0; id < min(256, _ent_list->get_highest_entity_index()); id++)
		{
			auto entity = _ent_list->get_centity(id);
			if (!entity || entity->get_origin().IsZero())
			{
				server::players[id].valid = false;
				server::entity[id].valid = false;
				continue;
			}

			//auto class_id = entity->get_clientnetworkable()->get_clientclass()->class_id;

			if (id < 32 && entity->valid())
			{
				auto weapon = entity->get_weapon();
				if (weapon)
				{
					auto type = weapon->get_type();
					if (type != weap_max)
					{
						if (id == global::local_id)
						{
							server::local.type = type;
						}
						else
						{
							server::players[id].clip = weapon->get_clip1();
							server::players[id].weapon = weapon->get_name();
							server::players[id].valid = true;
						}
					}
				}
			}
			else if (id > 32)
			{
				if (sets->visuals.enabled && sets->visuals.fancy_w2s)
				{
					server::entity[id].coord_frame = entity->get_coord_frame();
					server::entity[id].valid = true;
				}
			}
			else if (id < 32)
			{
				server::players[id].valid = false;
			}
		}*/

		// Ã¿ Ã¯Ã»Ã²Ã Ã«Ã±Ã¿, Ã­Ã® Ã­Ã¨Ã·Ã¥Ã£Ã® Ã­Ã¥ Ã¢Ã»Ã¸Ã«Ã®
		/*static unsigned int correct_vgui = NULL;
		if (correct_vgui == NULL)
		{
			const char* vguipanel_name = game::interfaces::panel->get_name(vguiPanel);

			if (strstr(vguipanel_name, "Mat"))
			{
				console::write("/painttraverse/ correct vguipanel_name => " + string(vguipanel_name), green);
				correct_vgui = vguiPanel;
				pt_hooked = true;
			}
			else
			{
				console::write("/painttraverse/ current vguipanel_name => " + string(vguipanel_name), red);
				return;
			}
		}*/

		/*
		vguiPanel
		0x
		*/

		//cout << vguiPanel << endl;

		//game::interfaces::surface->set_color(0, 255, 0, 255);
		//game::interfaces::surface->filled_rect(1, 1, 100, 200);
	}

	// thx catalinadragan22 (c) // uc
	// TODO: Ã¨Ã§Ã³Ã·Ã¨Ã²Ã¼ Ã¯Ã®Ã¤Ã°Ã®Ã¡Ã­Ã¥Ã¥
	memory::vthook* netchannel;
	using send_datagram_fn = int(__thiscall*)(CNetChan*, /*void*,*/ void*);
	send_datagram_fn o_send_datagram;
	int __fastcall send_datagram_hook(CNetChan* netchannel, void*, void* datagram)
	{
		if (sets->misc.fake_ping == 0 || !_engine->in_game() /*|| datagram*/)
			return o_send_datagram(netchannel, datagram);

		auto instate = netchannel->m_nInReliableState;
		auto in_sequencenr = netchannel->m_nInSequenceNr;

		misc::fake_ping::start(netchannel);
		misc::fake_ping::end(netchannel);

		int ret = o_send_datagram(netchannel, datagram);

		netchannel->m_nInReliableState = instate;
		netchannel->m_nInSequenceNr = in_sequencenr;

		return ret;
	}

	memory::vthook* client;
	using create_move_fn = void(__stdcall*)(int sequence_number, float input_sample_frametime, bool active);
	create_move_fn o_create_move;
	void __stdcall create_move_hook(int sequence_number,			// sequence_number of this cmd
									float input_sample_frametime,	// Frametime for mouse input sampling
									bool active)
	{
		o_create_move(sequence_number, input_sample_frametime, active);

		if (!_input)
		{
			return;
		}

		//Ã¢Ã»Ã§Ã»Ã¢Ã Ã¥Ã² Ã®Ã¸Ã¨Ã¡ÃªÃ³ Ã¨Ã§-Ã§Ã  Ã±Ã«Ã¨Ã¸ÃªÃ®Ã¬ Ã·Ã Ã±Ã²Ã»Ãµ Ã§Ã Ã¯Ã°Ã®Ã±Ã®Ã¢
		//cusercmd* cmd = _input->get_usercmd(sequence_number);

		/*if (global::cmd && global::cmd->command_number != 0)
		{
			cout << "global::cmd : " << global::cmd << endl;
			//cout << "global::cmd->buttons : " << global::cmd->buttons << endl;
			//if (global::cmd->buttons & IN_ATTACK)
			//	cout << "shot" << endl;
		}*/

		byte* p_sendpacket = nullptr;
		DWORD* p_ebp = nullptr;
		__asm mov p_ebp, ebp;

		if (p_ebp)
		{
			DWORD ebp_val = *p_ebp;
			if (ebp_val > 0x10000 && ebp_val < 0x7FFE0000)
			{
				p_sendpacket = (byte*)(ebp_val - 0x1);
				global::sendpacket = *p_sendpacket;
			}
		}

		c_verified_usercmd* verified_usercmd = nullptr;
		if (_input)
		{
			DWORD usercmd_ptr = *(DWORD*)((DWORD)_input + USERCMDOFFSET);
			if (usercmd_ptr > 0x10000)
			{
				global::cmd = &((cusercmd*)usercmd_ptr)[sequence_number % MULTIPLAYER_BACKUP];
			}
			DWORD verified_ptr = *(DWORD*)((DWORD)_input + VERIFIEDCMDOFFSET);
			if (verified_ptr > 0x10000)
			{
				verified_usercmd = &((c_verified_usercmd*)verified_ptr)[sequence_number % MULTIPLAYER_BACKUP];
			}
		}

		if (!sets->menu.panic && _engine->is_connected() && _engine->in_game() && global::cmd && global::cmd->command_number != 0 &&
			(global::local_id = _engine->get_local_id()) > 0 && (global::local = _ent_list->get_centity(global::local_id)))
		{
			Vector orig_angles = global::cmd->viewangles;
			if ((global::map_changed || sets->misc.fake_ping == 0) && netchannel)
			{
				netchannel->unhook();
				netchannel = null;
				o_send_datagram = null;
			}
			else if (_clientstate && _clientstate->m_NetChannel && sets->misc.fake_ping > 0 && !netchannel)
			{
				netchannel = new memory::vthook((dword**)_clientstate->m_NetChannel);
				if (netchannel)
				{
					o_send_datagram = (send_datagram_fn)netchannel->hook_function((dword)send_datagram_hook, 46);
				}
			}

			misc::remove();
			_globals = interfaces::pl_info_manager->get_globalvars();
			global::curtime = (float)global::local->get_tickbase() * _globals->interval_per_tick;
			global::weapon = global::local->get_weapon();
			auto observed = global::local->get_spec_player();
			global::local_observed = (global::local->valid() || observed == nullptr || global::local->get_spec_mode() != 4) ? global::local : observed;
			//weaponinfo_t w;
			//get_weapon_info(global::weapon->get_weaponid(), global::weapon->is_silenced(), w);
			//console::write(str(to_str(w.fPenetrationDistance) + " " + to_str(w.fPenetrationPower) + " " + to_str(w.iBulletType)), color::enabled());
			//cout << dec << w.fPenetrationDistance << " " << w.fPenetrationPower << " " << w.iBulletType << endl;
			
			//obj::spectators::target = global::local->valid() ? global::local : global::local->get_spec_player();
			//auto p_weapon = global::local->get_weapon();
			//auto p_data = p_weapon->get_data();
			//cout << "p_data.name : " << p_data.szClassName << endl;
			//global::weapon = p_weapon;
			//if ((global::weapon = global::local->get_weapon()))
			//	global::wpn_data = global::weapon->get_data();
			//auto id = global::weapon->GetWeaponID();
			//cout << "weapon_id : " << id << endl;
			//cout << "name : " << global::local->get_weapon()->get_data().szClassName << endl;
			//cout << "curtime : " << global::curtime << endl;
			//cout << "next_prim_attack : " << global::weapon->next_primary_attack() << endl;

			//kolonote:
			//css fix for head triggering (bbox_maxs z component is too small)
			//credits: me, wav
			// (c) iwebz kolo
			// TODO: Ã­Ã¥ Ã°Ã Ã¡Ã®Ã²Ã Ã¥Ã², Ã«Ã®Ã¬Ã Ã¥Ã²Ã±Ã¿ Ã¢Ã¨Ã§ Ã·Ã¥Ãª, Ã±ÃªÃ®Ã°Ã¥Ã¥ Ã¢Ã±Ã¥Ã£Ã® - Ã·Ã²Ã®-Ã²Ã® Ã³Ã±Ã²Ã Ã°Ã¥Ã«Ã®
			/*for (INT ax = 1; ax <= _engine->get_max_clients(); ax++)
			{
				centity* pBaseEntity = _ent_list->get_centity(ax);

				if (!pBaseEntity
					|| !pBaseEntity->valid()
					|| pBaseEntity == global::local)
					continue;

				PVOID pCollisionProperty = pBaseEntity->GetCollisionProperty();

				PFLOAT pfvecMaxsZ = (PFLOAT)((DWORD)pCollisionProperty + 0x1C);//vecMaxs.z
				PFLOAT pfvecMaxsZTwo = (PFLOAT)((DWORD)pCollisionProperty + 0x34);//vecMaxs2.z ???

				Vector vMini, vMaxi;
				pBaseEntity->get_render_bounds(vMini, vMaxi);

				if (*pfvecMaxsZ == vMaxi.z && *pfvecMaxsZTwo == vMaxi.z)
					continue;

				*pfvecMaxsZ = vMaxi.z;
				*pfvecMaxsZTwo = vMaxi.z;

				Vector vecSize;
				VectorSubtract(vMaxi, vMini, vecSize);
				float fNewRadius = vecSize.Length() * 0.5f;

				*(PFLOAT)((DWORD)pCollisionProperty + 0x38) = fNewRadius;//m_flRadius

				pBaseEntity->add_eflags(0x4000);
			}*/

			if ((sets->legit.enabled && (sets->legit.aim.fov > 0.f || sets->legit.backtrack.enabled || sets->legit.knifebot)) || sets->info.opened || sets->spec.opened || (sets->visuals.enabled && ((sets->visuals.esp_filter[0] && sets->visuals.esp_show[2]) || sets->visuals.crosshair == 2)))
			{
				server::local.type = global::weapon->get_type();

				legit::start();

				for (int id = 0; id < _engine->get_max_clients(); id++)
				{
					if (id == global::local_id)
					{
						//server::players[id].valid = false;
						continue;
					}

					centity* entity = _ent_list->get_centity(id);
					player_info_t pinfo;
					if (!entity || entity->is_dormant() || !_engine->get_playerinfo(id, &pinfo))
					{
						//server::players[id].valid = false;
						//ZeroMemory(legit::backtrack::records[id], sizeof(legit::backtrack::records[id]));
						continue;
					}

					legit::loop(id, entity);

					/*if (sets->visuals.enabled && sets->visuals.esp_filter[0] && sets->visuals.esp_show[3])
					{
						if (entity->valid())
						{
							auto weapon = entity->get_weapon();
							if (weapon)
							{
								auto type = weapon->get_type();
								if (type != weap_max)
								{
									if (sets->visuals.esp_bar[3]) server::players[id].clip = weapon->get_clip1();
									server::players[id].weapon = weapon->get_name();
									server::players[id].valid = true;
								}
								else
								{
									server::players[id].clip = -1;
									server::players[id].weapon = "none";
								}
							}
							else
							{
								server::players[id].clip = -1;
								server::players[id].weapon = "none";
							}
						}
					}*/

					/*server::max_players = id + 1;
					if (sets->info.opened)
					{
						auto weapon = entity->get_weapon();
						server::players[id].health = entity->get_hp();
						server::players[id].clip = weapon ? weapon->get_clip1() : -1;
						server::players[id].team = entity->get_team();
						server::players[id].lifestate = entity->get_life_state();
						server::players[id].weapon = weapon ? weapon->get_name() : "invalid";
					}
					server::players[id].name = pinfo.name;
					if (sets->spec.opened) server::players[id].spec_player = entity->get_spec_player();
					server::players[id].valid = true;*/
				}
			
				legit::end();
			}


			//auto cl_interp = _cvar->FindVar("cl_interp");
			//if (cl_interp->m_nValue > 5) _cvar->ConsoleColorPrintf(color(0, 255, 0), "cl_interp %i", cl_interp->m_nValue);
			//else _cvar->ConsoleColorPrintf(color(255, 0, 0), "cl_interp %i", cl_interp->m_nValue);
			//cout << "cl_interp " << cl_interp->m_fValue << endl;

			misc::run();
			legit::knifebot::run();

			bool shot_fired = false;
			if (sets->rage.enabled || sets->rage.magic_bullet)
			{
				shot_fired = rage::magic_bullet(orig_angles);
			}

			if (!shot_fired && !(global::cmd->buttons & (IN_ATTACK | IN_ATTACK2)))
			{
				rage::anti_aim();
			}
			else if (!shot_fired && (global::cmd->buttons & IN_ATTACK))
			{
				rage::standalone_rcs();
			}

			rage::fix_movement(global::cmd, orig_angles);
			rage::normalize_angles(global::cmd->viewangles);
			global::last_sent_angles = global::cmd->viewangles;
			misc::draw::clear(true);
		}
		else
		{
			misc::draw::clear(false);
		}

		if (!global::sendpacket)
		{
			global::chocked_packets++;
			if (global::chocked_packets > 14)
			{
				global::sendpacket = true;
				global::chocked_packets = 0;
			}
		}
		else global::chocked_packets = 0;

		if (verified_usercmd && global::cmd)
		{
			verified_usercmd->m_cmd = *global::cmd;
			verified_usercmd->m_crc = global::cmd->GetChecksum();
		}

		if (p_sendpacket)
		{
			*p_sendpacket = global::sendpacket;
		}
	}

	using frame_stage_notify_fn = void(__stdcall*)(clientframestage_t stage);
	frame_stage_notify_fn o_frame_stage_notify;
	void __stdcall frame_stage_notify_hook(clientframestage_t stage)
	{
		if (_engine && _engine->is_connected() && _engine->in_game() && global::local && global::local->valid() && !sets->menu.panic)
		{
			if (stage == FRAME_RENDER_START)
			{
				if (global::local->get_life_state() == 0 && (sets->rage.anti_aim || sets->rage.spinbot || sets->rage.pitch_aa != 0 || sets->rage.yaw_aa != 0))
				{
					if (!(global::cmd && (global::cmd->buttons & IN_ATTACK)))
					{
						if (offsets::angles)
						{
							*(qangle*)((DWORD)global::local + offsets::angles) = global::last_sent_angles;
						}
						if (offsets::rotation)
						{
							qangle render_rot = global::last_sent_angles;
							render_rot.x = 0.0f; // Body yaw rotates in 3D, pitch is handled by pose parameter
							*(qangle*)((DWORD)global::local + offsets::rotation) = render_rot;
						}
						if (offsets::pose_parameters)
						{
							float* poses = (float*)((DWORD)global::local + offsets::pose_parameters);
							float pitch = global::last_sent_angles.x;
							poses[0] = (pitch + 90.0f) / 180.0f;
							poses[11] = (pitch + 90.0f) / 180.0f;
						}
					}
				}

				// Custom 3D Player Model Changer (Applied on Render Frame)
				if (sets->visuals.enable_custom_model && !ModelMgr::model_entries.empty() && _model_info)
				{
					int sel = sets->visuals.model_selection;
					if (sel >= 0 && sel < (int)ModelMgr::model_entries.size())
					{
						const char* m_path = ModelMgr::model_entries[sel].model_path.c_str();
						int custom_idx = _model_info->get_model_index(m_path);
						if (custom_idx > 0)
						{
							if (sets->visuals.custom_model_local_only)
							{
								global::local->set_model_index(custom_idx);
							}
							else if (_ent_list)
							{
								for (int i = 1; i <= _engine->get_max_clients(); i++)
								{
									centity* ent = _ent_list->get_centity(i);
									if (ent && ent->valid() && (sets->visuals.friends || ent->get_team() != global::local->get_team()))
									{
										ent->set_model_index(custom_idx);
									}
								}
							}
						}
					}
				}
			}
			else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			{
				// Custom 3D Player Model Changer (Applied on Network Update)
				if (sets->visuals.enable_custom_model && !ModelMgr::model_entries.empty() && _model_info)
				{
					int sel = sets->visuals.model_selection;
					if (sel >= 0 && sel < (int)ModelMgr::model_entries.size())
					{
						const char* m_path = ModelMgr::model_entries[sel].model_path.c_str();
						int custom_idx = _model_info->get_model_index(m_path);
						if (custom_idx > 0 && sets->visuals.custom_model_local_only)
						{
							global::local->set_model_index(custom_idx);
						}
					}
				}
			}
		}

		if (o_frame_stage_notify)
			o_frame_stage_notify(stage);
	}

	memory::vthook* input;
	using get_usercmd_fn = cusercmd*(__stdcall*)(int sequence_number);
	get_usercmd_fn o_get_usercmd;
	cusercmd* __stdcall get_usercmd_hook(int sequence_number)
	{
		return &(*(cusercmd**)((DWORD)_input + USERCMDOFFSET))[sequence_number % MULTIPLAYER_BACKUP];
	}

	memory::vthook* model_render;
	using draw_model_execute_fn = void(__stdcall*)(const DrawModelState_t & state, const ModelRenderInfo_t & p_info, matrix3x4_t * p_custom_bone_to_world);
	draw_model_execute_fn o_draw_model_execute;
	void __stdcall draw_model_execute_hook(const DrawModelState_t& state, const ModelRenderInfo_t& p_info, matrix3x4_t* p_custom_bone_to_world)
	{
		model_render->unhook();

		models::run(state, p_info, p_custom_bone_to_world);
		
		model_render->rehook();
	}

	inline void bypass_cheats_thirdperson()
	{
		if (!_cvar) return;
		static bool bypassed = false;
		if (!bypassed)
		{
			auto sv_cheats = _cvar->find_var("sv_cheats");
			if (sv_cheats)
			{
				*(int*)((DWORD)sv_cheats + 0x14) &= ~FCVAR_CHEAT;
				*(int*)((DWORD)sv_cheats + 0x14) &= ~FCVAR_NOT_CONNECTED;
				*(int*)((DWORD)sv_cheats + 0x14) &= ~FCVAR_REPLICATED;
			}

			auto cmd_tp = _cvar->FindCommand("thirdperson");
			if (cmd_tp) *(int*)((DWORD)cmd_tp + 0x14) &= ~FCVAR_CHEAT;

			auto cmd_fp = _cvar->FindCommand("firstperson");
			if (cmd_fp) *(int*)((DWORD)cmd_fp + 0x14) &= ~FCVAR_CHEAT;

			auto cmd_cam = _cvar->FindCommand("cam_command");
			if (cmd_cam) *(int*)((DWORD)cmd_cam + 0x14) &= ~FCVAR_CHEAT;

			auto cam_idealdist = _cvar->find_var("cam_idealdist");
			if (cam_idealdist) *(int*)((DWORD)cam_idealdist + 0x14) &= ~FCVAR_CHEAT;

			auto cam_idealyaw = _cvar->find_var("cam_idealyaw");
			if (cam_idealyaw) *(int*)((DWORD)cam_idealyaw + 0x14) &= ~FCVAR_CHEAT;

			auto cam_idealpitch = _cvar->find_var("cam_idealpitch");
			if (cam_idealpitch) *(int*)((DWORD)cam_idealpitch + 0x14) &= ~FCVAR_CHEAT;

			auto cam_collision = _cvar->find_var("cam_collision");
			if (cam_collision) *(int*)((DWORD)cam_collision + 0x14) &= ~FCVAR_CHEAT;

			bypassed = true;
		}
	}

	memory::vthook* clientmode;
	using override_view_fn = void(__stdcall*)(cviewsetup* p_setup);
	override_view_fn o_override_view;
	void __stdcall override_view_hook(cviewsetup* p_setup)
	{
		if (o_override_view)
			o_override_view(p_setup);

		if (p_setup && _engine && _engine->in_game() && global::local && global::local->valid() && !sets->menu.panic)
		{
			if (sets->visuals.fov > 0.0f)
				p_setup->fov = sets->visuals.fov;

			if (sets->misc.fake_duck)
			{
				p_setup->origin.z = global::local->get_origin().z + 64.0f;
			}

			static bool prev_tp_state = false;
			if (sets->visuals.thirdperson != prev_tp_state)
			{
				bypass_cheats_thirdperson();
				if (sets->visuals.thirdperson)
				{
					_engine->clientcmd_unrestricted("thirdperson");
				}
				else
				{
					_engine->clientcmd_unrestricted("firstperson");
				}
				prev_tp_state = sets->visuals.thirdperson;
			}

			if (sets->visuals.thirdperson)
			{
				qangle cam_angles = p_setup->angles;
				if (sets->visuals.thirdperson_reverse)
				{
					cam_angles.y += 180.0f;
				}

				Vector forward, right, up;
				AngleVectors(cam_angles, &forward, &right, &up);

				Vector eye_pos = global::local->get_eye_pos();
				float dist = sets->visuals.thirdperson_dist > 10.0f ? sets->visuals.thirdperson_dist : 120.0f;
				Vector target_pos = eye_pos - (forward * dist);

				if (_engine_trace)
				{
					trace_t tr;
					ray_t ray;
					ray.Init(eye_pos, target_pos, Vector(-6, -6, -6), Vector(6, 6, 6));
					itracefilter filter;
					filter.skip = global::local;
					_engine_trace->trace_ray(ray, MASK_SOLID, &filter, &tr);

					if (tr.fraction < 1.0f)
					{
						target_pos = eye_pos - (forward * (dist * tr.fraction * 0.9f));
					}
				}

				p_setup->origin = target_pos;
				p_setup->angles = cam_angles;
			}
		}
	}

	using get_vm_fov_fn = float(__stdcall*)();
	get_vm_fov_fn o_get_vm_fov;
	float __stdcall get_vm_fov_hook()
	{
		if (!sets->menu.panic && sets->visuals.viewmodel_fov > 0.0f)
			return sets->visuals.viewmodel_fov;
		return ((cvar(vm_fov).value > 0 && !sets->menu.panic) ? cvar(vm_fov).value : o_get_vm_fov());
	}

	memory::vthook* surface;
	using lock_cursor_fn = void(__fastcall*)(isurface*);
	lock_cursor_fn o_lock_cursor;
	void __fastcall lock_cursor_hook(isurface* surface, void*)
	{
		//surface->set_color(255, 255, 255, 255);
		//surface->filled_rect(0, 0, 100, 100);

		if (sets->menu.opened)
		{
			surface->unlock_cursor();//63
			//surface->set_cursor(dc_arrow);//set 54
			*global::lock_cursor = false;
		}
		else
		{
			*global::lock_cursor = true;
			//surface->set_cursor(cursor_code::dc_none);
			o_lock_cursor(surface);
		}
	}

	memory::vthook* engine_sound;
	using emit_sound_fn = void(__stdcall*)(irecipientfilter&, int, int, const char*, float, float, int, int, int, const Vector*, const Vector*, void*, bool, float, int);
	emit_sound_fn o_emit_sound;
	void __stdcall emit_sound_hook(irecipientfilter& filter, int entity_index, int channel, const char* sample,
		float volume, float attenuation, int flags, int pitch, int special_dsp,
		const Vector* origin, const Vector* direction, void* shit, bool update_positions, float soundtime, int speakerentity)
	{
		//cout << "name : " << sample << endl;

		if ((sets->visuals.sound_esp || sets->visuals.footstep_rings || sets->visuals.esp_show[4] || sets->visuals.esp_show[5]) && entity_index >= 0 && entity_index < 64 && entity_index != global::local_id)
		{
			auto player = _ent_list->get_centity(entity_index);
			if (player && player->valid())
			{
				if (sets->visuals.friends || player->get_team() != global::local->get_team())
				{
					if (strstr(sample, "footstep") || strstr(sample, "step") || strstr(sample, "player/footsteps"))
					{
						server::sound snd;
						if (origin && !origin->IsZero())
						{
							snd.position = *origin;
						}
						else
						{
							snd.position = player->get_abs_origin();
						}
						snd.time = global::curtime;
						snd.col = (player->get_team() == 2) ? color(255, 100, 50) : color(50, 150, 255);
						server::sounds.push_back(snd);
					}
					else if (sample[0] == ')' && sample[1] == 'w' && sample[2] == 'e') // weapon gunshot
					{
						server::sound snd;
						snd.position = player->get_abs_origin();
						snd.time = global::curtime;
						snd.col = color(255, 0, 0);
						server::sounds.push_back(snd);
					}
				}
			}
		}

		o_emit_sound(filter, entity_index, channel, sample, volume, attenuation, flags, pitch, special_dsp, origin, direction, shit, update_positions, soundtime, speakerentity);
	}

	memory::vthook* event_manager;
	using fire_event_clientside_fn = bool(__stdcall*)(igameevent*);
	fire_event_clientside_fn o_fire_event_clientside;
	bool __stdcall fire_event_clientside_hook(igameevent* p_event)
	{
		if (p_event)
		{
			const char* name = p_event->get_name();
			if (name)
			{
				if (strcmp(name, "bullet_impact") == 0)
				{
					int userid = p_event->get_int("userid");
					if (_engine && _engine->get_player_for_userid(userid) == global::local_id && global::local && global::local->valid())
					{
						cvector impact_pos(p_event->get_float("x"), p_event->get_float("y"), p_event->get_float("z"));
						cvector eye_pos = global::local->get_eye_pos();
						esp::add_bullet_tracer(eye_pos, impact_pos, sets->visuals.bullet_tracers_color);
					}
				}
				else if (strcmp(name, "player_hurt") == 0)
				{
					int attacker_id = _engine ? _engine->get_player_for_userid(p_event->get_int("attacker")) : 0;
					if (attacker_id == global::local_id)
					{
						esp::trigger_screen_hit_pulse();
						int victim_id = _engine ? _engine->get_player_for_userid(p_event->get_int("userid")) : 0;
						centity* victim = _ent_list ? _ent_list->get_entity(victim_id) : nullptr;
						if (victim)
						{
							cvector hit_pos = victim->get_eye_pos();
							int dmg = p_event->get_int("dmg_health");
							bool head = (p_event->get_int("hitgroup") == 1);
							esp::add_damage_indicator(hit_pos, dmg, head);
						}
					}
				}
				else if (strcmp(name, "player_death") == 0)
				{
					int attacker_id = _engine ? _engine->get_player_for_userid(p_event->get_int("attacker")) : 0;
					if (attacker_id == global::local_id)
					{
						int victim_id = _engine ? _engine->get_player_for_userid(p_event->get_int("userid")) : 0;
						centity* victim = _ent_list ? _ent_list->get_entity(victim_id) : nullptr;
						if (victim)
						{
							cvector death_pos = victim->get_origin();
							esp::add_kill_effect(death_pos, sets->visuals.kill_effect, sets->visuals.kill_effect_color);
						}
					}
				}
				events::on_fire_event(p_event, name);
			}
		}

		return o_fire_event_clientside(p_event);
	}

	void do_them()
	{
		std::ofstream log("C:/lovemachine_log.txt", std::ios::app);

		if (global::window) {
			o_wndproc = reinterpret_cast<wndproc>(SetWindowLongPtr(global::window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndproc_hook)));
			log << "[+] o_wndproc hooked: " << std::hex << (DWORD)o_wndproc << std::endl;
		}

		if (game::signatures::d3d9_device && !IsBadReadPtr((void*)game::signatures::d3d9_device, sizeof(DWORD))) {
			d3d9 = new memory::vthook((dword**)game::signatures::d3d9_device);
			o_endscene = (endscene_fn)d3d9->hook_function((dword)endscene_hook, 42);
			o_reset = (reset_fn)d3d9->hook_function((dword)reset_hook, 16);
			log << "[+] d3d9 hooked" << std::endl;
		}

		if (_panel && !IsBadReadPtr((void*)_panel, sizeof(DWORD))) {
			panel = new memory::vthook((dword**)_panel);
			o_painttraverse = (painttraverse_fn)panel->hook_function((dword)painttraverse_hook, 41);
			log << "[+] panel hooked" << std::endl;
		}

		if (_surface && !IsBadReadPtr((void*)_surface, sizeof(DWORD))) {
			surface = new memory::vthook((dword**)_surface);
			o_lock_cursor = (lock_cursor_fn)surface->hook_function((dword)lock_cursor_hook, 62);
			log << "[+] surface hooked" << std::endl;
		}

		if (_client && !IsBadReadPtr((void*)_client, sizeof(DWORD))) {
			client = new memory::vthook((dword**)_client);
			log << "[+] client hooked" << std::endl;

			o_create_move = (create_move_fn)client->hook_function((dword)create_move_hook, 21);
			log << "[+] create_move hooked" << std::endl;

			o_frame_stage_notify = (frame_stage_notify_fn)client->hook_function((dword)frame_stage_notify_hook, 35);
			log << "[+] frame_stage_notify hooked" << std::endl;
		}

		if (_model_render && !IsBadReadPtr((void*)_model_render, sizeof(DWORD))) {
			model_render = new memory::vthook((dword**)_model_render);
			o_draw_model_execute = (draw_model_execute_fn)model_render->hook_function((dword)draw_model_execute_hook, 19);
			log << "[+] model_render hooked" << std::endl;
		}

		if (_clientmode && !IsBadReadPtr((void*)_clientmode, sizeof(DWORD))) {
			clientmode = new memory::vthook((dword**)_clientmode);
			o_override_view = (override_view_fn)clientmode->hook_function((dword)override_view_hook, 16);
			o_get_vm_fov = (get_vm_fov_fn)clientmode->hook_function((dword)get_vm_fov_hook, 32);
			log << "[+] clientmode hooked" << std::endl;
		}

		if (_engine_sound && !IsBadReadPtr((void*)_engine_sound, sizeof(DWORD))) {
			engine_sound = new memory::vthook((dword**)_engine_sound);
			o_emit_sound = (emit_sound_fn)engine_sound->hook_function((dword)emit_sound_hook, 4);
			log << "[+] engine_sound hooked" << std::endl;
		}

		if (_event_manager && !IsBadReadPtr((void*)_event_manager, sizeof(DWORD))) {
			event_manager = new memory::vthook((dword**)_event_manager);
			o_fire_event_clientside = (fire_event_clientside_fn)event_manager->hook_function((dword)fire_event_clientside_hook, 10);
			log << "[+] event_manager hooked" << std::endl;
		}
	}

	void remove()
	{
		console::write("removing wndproc_hook");
		SetWindowLongPtr(global::window, GWLP_WNDPROC, reinterpret_cast<long>(o_wndproc));

		console::write("removing other hooks");
		d3d9->unhook();
		panel->unhook();
		surface->unhook();
		client->unhook();
		input->unhook();
		model_render->unhook();
		clientmode->unhook();

		if (netchannel)
		{
			console::write("removing send_datagram_hook");
			netchannel->unhook();
		}		
	}
}

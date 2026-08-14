#pragma once
#include "includes.h"
#include "definitions.h"
#include "memory.h"
#include "console.h"
#include "interfaces.h"

namespace game
{
	namespace interfaces
	{
		ivpanel* panel;
		isurface* surface;
		icliententitylist* ent_list;
		ivengineclient* engine; // client
		ibaseclientdll* client; // base client dll
		ivdebugoverlay* debug_overlay;
		imaterialsystem* mat_sys;
		ivmodelrender* model_render;
		ivmodelinfo* model_info; // client
		ivrenderview* render_view;
		iplayerinfomanager* pl_info_manager;
		ienginetrace* engine_trace;
		ienginesound* engine_sound;
		igameeventmanager* event_manager;
		icvar* cvar;
		cglobalvars* globals;
		cinput* input;
		iphysicssurfaceprops* physics;

		template<typename T>
		T* CaptureInterface(const char* moduleName, const char* interfaceName)
		{
			T* ptr = memory::pinterface<T>(moduleName, interfaceName);
			if (!ptr)
			{
				console::write("[-] ERROR: Failed to capture interface '" + std::string(interfaceName) + "' from '" + std::string(moduleName) + "'", darkred);
			}
			else
			{
				console::write_hex("[+] " + std::string(interfaceName), (dword)ptr, darkgreen);
			}
			return ptr;
		}

		void find_them()
		{
			panel = CaptureInterface<ivpanel>("vgui2.dll", "VGUI_Panel009");
			surface = CaptureInterface<isurface>("vguimatsurface.dll", "VGUI_Surface030");
			ent_list = CaptureInterface<icliententitylist>("client.dll", "VClientEntityList003");
			engine = CaptureInterface<ivengineclient>("engine.dll", "VEngineClient014");
			client = CaptureInterface<ibaseclientdll>("client.dll", "VClient017");

			if (client) console::write_hex("/function/ client->get_all_classes()", (dword)client->get_all_classes(), darkgreen);

			debug_overlay = CaptureInterface<ivdebugoverlay>("engine.dll", "VDebugOverlay003");
			mat_sys = CaptureInterface<imaterialsystem>("materialsystem.dll", "VMaterialSystem080");
			model_render = CaptureInterface<ivmodelrender>("engine.dll", "VEngineModel016");
			model_info = CaptureInterface<ivmodelinfo>("engine.dll", "VModelInfoClient006");
			render_view = CaptureInterface<ivrenderview>("engine.dll", "VEngineRenderView014");
			pl_info_manager = CaptureInterface<iplayerinfomanager>("server.dll", "PlayerInfoManager002");
			engine_trace = CaptureInterface<ienginetrace>("engine.dll", "EngineTraceClient003");
			cvar = CaptureInterface<icvar>("vstdlib.dll", "VEngineCvar004");
			engine_sound = CaptureInterface<ienginesound>("engine.dll", "IEngineSoundClient003");
			event_manager = CaptureInterface<igameeventmanager>("engine.dll", "GAMEEVENTSMANAGER002");
			physics = CaptureInterface<iphysicssurfaceprops>("vphysics.dll", "VPhysicsSurfaceProps001");
		}
	}

	namespace signatures
	{
		dword d3d9_device;
		dword clientmode;
		dword bullet_params;
		get_data_fn get_wpn_data;
		cclientstate* clientstate;

		typedef void (*ClipTraceToPlayers_t)(const Vector&, const Vector&, unsigned int, itracefilter*, trace_t*);
		ClipTraceToPlayers_t  ClipTraceToPlayers;

		void find_them()
		{
			dword pat_d3d = memory::pattern("shaderapidx9.dll", "A1 ? ? ? ? 8D 53 08");
			if (pat_d3d && !IsBadReadPtr((void*)(pat_d3d + 0x1), sizeof(DWORD)))
			{
				DWORD ptr1 = *(DWORD*)(pat_d3d + 0x1);
				if (ptr1 && !IsBadReadPtr((void*)ptr1, sizeof(DWORD)))
				{
					d3d9_device = *(DWORD*)ptr1;
					console::write_hex("[+] d3d9_device", d3d9_device, darkgreen);
				}
			}

			dword pat_cm = memory::pattern("client.dll", "8B 0D ? ? ? ? 8B 01 5D FF 60 28 CC");
			if (pat_cm && !IsBadReadPtr((void*)(pat_cm + 0x2), sizeof(DWORD)))
			{
				DWORD ptr2 = *(DWORD*)(pat_cm + 0x2);
				if (ptr2 && !IsBadReadPtr((void*)ptr2, sizeof(DWORD)))
				{
					clientmode = *(DWORD*)ptr2;
					console::write_hex("[+] clientmode", clientmode, darkgreen);
				}
			}

			bullet_params = memory::pattern("client.dll", "55 8B EC 56 8B 75 08 68 ? ? ? ? 56 E8 ? ? ? ? 83 C4 08 84 C0");
			console::write_hex("[+] bullet_params", bullet_params, darkgreen);

			dword weapon_data = memory::pattern("client.dll", "0F B7 81 ? ? ? ? 50 E8 ? ? ? ? 83 C4 04 C3");
			console::write_hex("[+] weapon_data", weapon_data, darkgreen);
			if (weapon_data) get_wpn_data = (get_data_fn)(weapon_data);

			dword lock_cursor_pat = memory::pattern("vguimatsurface.dll", "A3 ? ? ? ? C6 05");
			if (lock_cursor_pat && !IsBadReadPtr((void*)(lock_cursor_pat + 0x7), sizeof(DWORD)))
			{
				dword lock_cursor = lock_cursor_pat + 0x7;
				if (lock_cursor && !IsBadReadPtr((void*)lock_cursor, sizeof(DWORD)))
				{
					global::lock_cursor = *(bool**)(lock_cursor);
					console::write_hex("[+] lock_cursor", (dword)global::lock_cursor, darkgreen);
				}
			}

			dword cliptracetoplayers = memory::pattern("client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 18");
			if (cliptracetoplayers)
			{
				ClipTraceToPlayers = (ClipTraceToPlayers_t)(cliptracetoplayers);
				console::write_hex("[+] ClipTraceToPlayers", (dword)ClipTraceToPlayers, darkgreen);
			}
		}
	}

	void find()
	{
		interfaces::find_them();
		signatures::find_them();
	}
}

#define _panel game::interfaces::panel
#define _surface game::interfaces::surface
#define _ent_list game::interfaces::ent_list
#define _engine game::interfaces::engine
#define _client game::interfaces::client
#define _debug_overlay game::interfaces::debug_overlay
#define _mat_sys game::interfaces::mat_sys
#define _model_render game::interfaces::model_render
#define _model_info game::interfaces::model_info
#define _render_view game::interfaces::render_view
#define _engine_trace game::interfaces::engine_trace
#define _engine_sound game::interfaces::engine_sound
#define _event_manager game::interfaces::event_manager
#define _cvar game::interfaces::cvar
#define _globals game::interfaces::globals
#define _input game::interfaces::input
#define _clientmode game::signatures::clientmode
#define _phys game::interfaces::physics
//#define _clientstate game::signatures::clientstate
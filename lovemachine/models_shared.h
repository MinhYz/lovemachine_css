#pragma once
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

struct CustomModelEntry {
	std::string display_name;
	std::string model_path;
};

namespace ModelMgr
{
	inline std::vector<CustomModelEntry> model_entries = {
		{ "Phoenix (T)", "models/player/t_phoenix.mdl" },
		{ "Leet (T)", "models/player/t_leet.mdl" },
		{ "SAS (CT)", "models/player/ct_sas.mdl" },
		{ "GIGN (CT)", "models/player/ct_gign.mdl" },
		{ "Hostage 01", "models/characters/hostage_01.mdl" },
		{ "Akame (Akame ga Kill)", "models/player/legion/akame/akame_fix.mdl" },
		{ "Katarina", "models/player/knifelemon/katarina.mdl" },
		{ "Serah (Final Fantasy)", "models/player/hhp227/tenten_loveff/serah.mdl" },
		{ "Harley Quinn", "models/player/slow/arkham_asylum/harley_quinn/slow.mdl" },
		{ "Ayumi (X-Blades)", "models/player/slow/ayumi/slow.mdl" },
		{ "Saki Anime (CT)", "models/player/stenli/saki_ct.mdl" },
		{ "Saki Anime (T)", "models/player/stenli/saki_t.mdl" },
		{ "Jill Valentine (RE1)", "models/player/pil/re1/jill_re1/jill_sandwich_pil1.mdl" },
		{ "Iron Man Mark 3", "models/player/techknow/ironman_v3/ironman3.mdl" },
		{ "Captain America", "models/player/techknow/cpt_america/cpt_a.mdl" },
		{ "Batman (MK vs DC)", "models/player/slow/jamis/mkvsdcu/batman/slow_pub_v2.mdl" },
		{ "Venom (WoS)", "models/player/slow/jamis/venom_wos/slow_v2.mdl" },
		{ "Nanosuit (Crysis)", "models/player/slow/nanosuit/slow_nanosuit.mdl" },
		{ "Corvo (Dishonored)", "models/player/vad36dishonored/corvo.mdl" },
		{ "Lolli Juliet", "models/player/vad36lollipop/lolli_new.mdl" },
		{ "Cissia ZZZ (Zenless Zone Zero)", "models/sneaky_holy/neps/powered_by_nidegg/best_zombie_escape_server/thick_snake/cissia_zzz.mdl" }
	};

#ifdef _WIN32
	inline void ScanFolderForModelsRecursive(const std::string& base_folder, const std::string& current_rel_dir)
	{
		std::string search_path = base_folder + "\\" + (current_rel_dir.empty() ? "" : current_rel_dir + "\\") + "*";
		WIN32_FIND_DATAA fd;
		HANDLE hFind = FindFirstFileA(search_path.c_str(), &fd);
		if (hFind == INVALID_HANDLE_VALUE) return;

		do {
			if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;

			std::string sub_rel = current_rel_dir.empty() ? fd.cFileName : (current_rel_dir + "/" + fd.cFileName);

			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				ScanFolderForModelsRecursive(base_folder, sub_rel);
			}
			else
			{
				std::string fn(fd.cFileName);
				if (fn.length() > 4 && fn.substr(fn.length() - 4) == ".mdl")
				{
					std::string full_model_path = sub_rel;
					if (base_folder.find("models") != std::string::npos)
						full_model_path = "models/" + sub_rel;

					std::string display = fd.cFileName;
					display = display.substr(0, display.length() - 4);
					
					bool exists = false;
					for (auto& it : model_entries)
					{
						if (it.model_path == full_model_path) { exists = true; break; }
					}
					if (!exists)
					{
						model_entries.push_back({ display, full_model_path });
					}
				}
			}
		} while (FindNextFileA(hFind, &fd));
		FindClose(hFind);
	}
#endif

	inline void RefreshDynamicModels()
	{
#ifdef _WIN32
		static bool scanned = false;
		if (!scanned)
		{
			scanned = true;
			ScanFolderForModelsRecursive("cstrike\\models", "");
			ScanFolderForModelsRecursive("cstrike\\script\\models", "");
			ScanFolderForModelsRecursive("cstrike\\scripts\\models", "");
			ScanFolderForModelsRecursive("cstrike\\script", "");
			ScanFolderForModelsRecursive("cstrike\\scripts", "");
			ScanFolderForModelsRecursive("cstrike_downloads\\models", "");
			ScanFolderForModelsRecursive("cstrike\\download\\models", "");
			ScanFolderForModelsRecursive("cstrike\\custom", "");
			ScanFolderForModelsRecursive("cstrike_custom\\models", "");
		}
#endif
	}
}

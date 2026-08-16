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
					std::string full_model_path = "models/" + sub_rel;
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
		}
#endif
	}
}

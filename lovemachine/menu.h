#pragma once
#include "imgui.h"
#ifdef _WIN32
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#endif
#include "settings.h"
#include "configs.h"

namespace Menu
{
	enum UITheme
	{
		THEME_SKEET = 0,
		THEME_CYBERPUNK,
		THEME_ONYX,
		THEME_NEVERLOSE
	};

	enum UILayout
	{
		LAYOUT_SKEET = 0,
		LAYOUT_ONETAP,
		LAYOUT_FATALITY,
		LAYOUT_NEVERLOSE
	};

	extern bool show_menu;
	extern bool is_binding_key;
	extern int current_tab;
	extern int current_theme;
	extern int current_layout;

	void SetupStyle();
	void ApplyTheme(int theme_id);
	void Render();
}
#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
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
		THEME_NEVERLOSE,
		THEME_FATALITY
	};

	enum UILayout
	{
		LAYOUT_GAMESENSE = 0, // Gamesense (Skeet Classic)
		LAYOUT_NEVERLOSE,     // Neverlose Official
		LAYOUT_ATERNOS,       // Release 16: Imgui Aternos (3D Skeleton Visualizer)
		LAYOUT_SYNTHETIC,     // Release 19: Synthetic / Space Galaxy Honeycomb
		LAYOUT_FATALITY       // Fatality.win Native UI Layout (Deep Crimson / Cyber Pink)
	};

	extern bool show_menu;
	extern bool is_binding_key;
	extern int current_tab;
	extern int current_theme;
	extern int current_layout;
	extern ImFont* font_skeet_icons;
	extern ImFont* font_main;
	extern ImFont* font_brand_title;

	void SetupStyle();
	void ApplyTheme(int theme_id);
	void Render();
}
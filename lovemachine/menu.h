#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include "settings.h"
#include "configs.h"

namespace Menu
{
	extern bool show_menu;
	extern int current_tab;

	void SetupStyle();
	void Render();
}
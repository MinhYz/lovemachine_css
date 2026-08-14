#include "menu.h"
#include <vector>
#include <string>

namespace Menu
{
	bool show_menu = true;
	int current_tab = 0;

	// Helper for ImGui ColorEdit3 mapped to custom color struct
	static bool ColorEdit3Custom(const char* label, color& col)
	{
		col.r = max(0, min(col.r, 255));
		col.g = max(0, min(col.g, 255));
		col.b = max(0, min(col.b, 255));
		float c[3] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f };
		if (ImGui::ColorEdit3(label, c))
		{
			col.r = static_cast<int>(c[0] * 255.0f);
			col.g = static_cast<int>(c[1] * 255.0f);
			col.b = static_cast<int>(c[2] * 255.0f);
			return true;
		}
		return false;
	}

	void SetupStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		
		// Geometry & Roundings - Ultra Clean Minimalist Theme
		style.WindowPadding     = ImVec2(16.0f, 16.0f);
		style.FramePadding      = ImVec2(10.0f, 6.0f);
		style.ItemSpacing       = ImVec2(12.0f, 10.0f);
		style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
		style.WindowRounding    = 10.0f;
		style.ChildRounding     = 8.0f;
		style.FrameRounding     = 6.0f;
		style.PopupRounding     = 8.0f;
		style.ScrollbarRounding = 6.0f;
		style.GrabRounding      = 5.0f;
		style.TabRounding       = 6.0f;
		
		style.WindowBorderSize  = 1.0f;
		style.ChildBorderSize   = 1.0f;
		style.FrameBorderSize   = 0.0f;
		style.PopupBorderSize   = 1.0f;

		// Color Palette - Sleek Onyx & Electric Purple
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Text]                  = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
		colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.58f, 1.00f);
		colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.06f, 0.08f, 0.98f);
		colors[ImGuiCol_ChildBg]               = ImVec4(0.09f, 0.09f, 0.12f, 0.90f);
		colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.10f, 0.13f, 0.98f);
		colors[ImGuiCol_Border]                = ImVec4(0.20f, 0.16f, 0.30f, 0.50f);
		colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg]               = ImVec4(0.13f, 0.13f, 0.18f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.20f, 0.16f, 0.28f, 1.00f);
		colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.20f, 0.36f, 1.00f);
		colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.11f, 1.00f);
		colors[ImGuiCol_TitleBgActive]         = ImVec4(0.14f, 0.11f, 0.22f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.06f, 0.06f, 0.08f, 0.75f);
		colors[ImGuiCol_MenuBarBg]             = ImVec4(0.08f, 0.08f, 0.11f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.06f, 0.06f, 0.08f, 0.60f);
		colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.16f, 0.30f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.30f, 0.23f, 0.44f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.40f, 0.30f, 0.56f, 1.00f);
		colors[ImGuiCol_CheckMark]             = ImVec4(0.68f, 0.42f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrab]            = ImVec4(0.58f, 0.34f, 0.90f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.72f, 0.44f, 1.00f, 1.00f);
		colors[ImGuiCol_Button]                = ImVec4(0.15f, 0.13f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered]         = ImVec4(0.30f, 0.20f, 0.46f, 1.00f);
		colors[ImGuiCol_ButtonActive]          = ImVec4(0.40f, 0.26f, 0.60f, 1.00f);
		colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.15f, 0.32f, 1.00f);
		colors[ImGuiCol_HeaderHovered]         = ImVec4(0.30f, 0.22f, 0.46f, 1.00f);
		colors[ImGuiCol_HeaderActive]          = ImVec4(0.40f, 0.28f, 0.60f, 1.00f);
		colors[ImGuiCol_Separator]             = ImVec4(0.20f, 0.16f, 0.30f, 0.50f);
		colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.58f, 0.34f, 0.90f, 0.78f);
		colors[ImGuiCol_SeparatorActive]       = ImVec4(0.72f, 0.44f, 1.00f, 1.00f);
	}

	void Render()
	{
		if (!show_menu) return;

		ImGui::SetNextWindowSize(ImVec2(920, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin("lovemachine_css - External ImGui UI", &show_menu, ImGuiWindowFlags_NoCollapse);

		// =========================================================================
		// SLEEK LEFT VERTICAL SIDEBAR NAVIGATION
		// =========================================================================
		ImGui::BeginChild("LeftSidebar", ImVec2(190, 0), true);

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), " LOVEMACHINE");
		ImGui::TextDisabled("  Counter-Strike: Source");
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		const char* tabs[] = {
			"  Aimbot & Rage",
			"  Visuals & ESP",
			"  Misc & Utilities",
			"  Nightmode & World",
			"  Settings & Configs"
		};

		for (int i = 0; i < 5; ++i)
		{
			bool is_selected = (current_tab == i);
			if (is_selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.22f, 0.55f, 1.00f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.26f, 0.65f, 1.00f));
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.16f, 0.60f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.18f, 0.32f, 1.00f));
			}

			if (ImGui::Button(tabs[i], ImVec2(174, 38)))
			{
				current_tab = i;
			}
			ImGui::PopStyleColor(2);
			ImGui::Spacing();
		}

		ImGui::EndChild();

		ImGui::SameLine();

		// =========================================================================
		// MAIN CONTENT CANVAS
		// =========================================================================
		ImGui::BeginChild("MainCanvas", ImVec2(0, 0), false);
		ImGui::PushID(current_tab);

		// =========================================================================
		// TAB 0: AIMBOT & RAGE (SPINBOT / ANTI-AIM / CRANIUM EXTENSIONS)
		// =========================================================================
		if (current_tab == 0)
		{
			ImGui::Columns(2, "AimbotCols", false);

			// --- Card 1: Legit Aimbot & Prediction / Recoil ---
			ImGui::BeginChild("LegitCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Legit Aimbot Configuration");
			ImGui::Separator();

			ImGui::Checkbox("Enable Legit Aimbot", &sets->legit.enabled);
			ImGui::Checkbox("Target Teammates / Friends", &sets->legit.friends);
			ImGui::Checkbox("Auto Knifebot", &sets->legit.knifebot);

			static int aim_hitbox = 0;
			const char* hitbox_list[] = { "Head Only", "Chest", "Pelvis", "Closest (All)" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Target Hitbox", &aim_hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));

			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.48f, 0.88f, 1.00f, 1.00f), "Aimbot Accuracy & Assists");
			ImGui::Checkbox("Auto Crouch", &sets->legit.aim.auto_crouch);
			ImGui::Checkbox("Engine Position Prediction", &sets->legit.aim.predicted_position);
			ImGui::Checkbox("NoRecoil (Remove Recoil)", &sets->legit.aim.norecoil);
			ImGui::Checkbox("NoSpread (Remove Bullet Spread)", &sets->legit.aim.nospread);

			ImGui::Spacing();
			ImGui::TextDisabled("Aimbot Modes & Triggers");
			const char* silent_modes[] = { "Off", "Client Side", "Server Side", "Perfect Silent" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Silent Aim Mode", &sets->legit.aim.silent_mode, silent_modes, IM_ARRAYSIZE(silent_modes));

			const char* fov_modes[] = { "Static FOV", "Dynamic FOV", "Distance-Based" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("FOV Selection Mode", &sets->legit.aim.fov_selection, fov_modes, IM_ARRAYSIZE(fov_modes));

			const char* usage_modes[] = { "Always Active", "On Key Hold", "On Attack Only" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Usage Condition", &sets->legit.aim.aim_usage, usage_modes, IM_ARRAYSIZE(usage_modes));

			ImGui::Spacing();
			ImGui::TextDisabled("Aim Parameters");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("FOV Angle", &sets->legit.aim.fov, 0.0f, 30.0f, "%.1f deg");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Smooth X", &sets->legit.aim.smooth[0], 0.0f, 50.0f, "%.1f");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Smooth Y", &sets->legit.aim.smooth[1], 0.0f, 50.0f, "%.1f");

			ImGui::EndChild();
			ImGui::NextColumn();

			// --- Card 2: Rage & Spinbot / Anti-Aim ---
			ImGui::BeginChild("RageCard", ImVec2(0, 0), true);

			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Ragebot Engine");
			ImGui::Separator();
			ImGui::Checkbox("Enable Ragebot", &sets->rage.enabled);
			ImGui::Checkbox("Magic Bullet (Hit Any Target & Penetrate Walls)", &sets->rage.magic_bullet);
			ImGui::Checkbox("Autowall Penetration", &sets->rage.autowall);
			ImGui::Checkbox("Autoshoot", &sets->rage.autoshoot);
			ImGui::Checkbox("Autostop Movement", &sets->rage.autostop);
			ImGui::Checkbox("Silent Aim", &sets->rage.silent);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Hitchance %", &sets->rage.hitchance, 0.0f, 1.0f, "%.2f");

			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.98f, 0.48f, 0.65f, 1.00f), "Spinbot & Anti-Aim (AA)");
			ImGui::Separator();
			ImGui::Checkbox("Enable Spinbot", &sets->rage.spinbot);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Spin Speed", &sets->rage.spin_speed, 1.0f, 100.0f, "%.0f deg");

			const char* pitch_aa_items[] = { "Off", "Emotion (-89Â° Down)", "Up (89Â°)", "Zero (0Â°)" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Pitch Anti-Aim", &sets->rage.pitch_aa, pitch_aa_items, IM_ARRAYSIZE(pitch_aa_items));

			const char* yaw_aa_items[] = { "Off", "Backward (180Â°)", "Spinbot", "Jitter", "Sideways (90Â°)" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Yaw Anti-Aim", &sets->rage.yaw_aa, yaw_aa_items, IM_ARRAYSIZE(yaw_aa_items));

			ImGui::Spacing();
			ImGui::TextDisabled("Triggerbot & Backtrack");
			ImGui::Checkbox("Enable Triggerbot", &sets->legit.trigger._enabled);
			ImGui::Checkbox("Enable Backtrack", &sets->legit.backtrack.enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("Backtrack Ticks", &sets->legit.backtrack.ticks, 1, 14);

			ImGui::EndChild();
			ImGui::Columns(1);
		}

		// =========================================================================
		// TAB 1: VISUALS / ESP (ASIAN HAT & REVERSE THIRDPERSON)
		// =========================================================================
		else if (current_tab == 1)
		{
			ImGui::Columns(2, "VisualsCols", false);

			// --- Card 1: Player ESP & Tracing Options ---
			ImGui::BeginChild("PlayerEspCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Player ESP & Tracing Logic");
			ImGui::Separator();

			ImGui::Checkbox("Enable Visuals", &sets->visuals.enabled);
			ImGui::Checkbox("Draw Teammates", &sets->visuals.friends);
			ImGui::Checkbox("Draw Occluded / Wall Traced Enemies", &sets->visuals.esp_check[0]);
			ImGui::SameLine(); ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Keeps ESP active for enemies behind walls or behind your view.");

			ImGui::Checkbox("Fade Offscreen Enemies", &sets->visuals.fade);

			ImGui::Spacing();
			ImGui::TextDisabled("ESP Elements & Indicators");
			ImGui::Checkbox("Player Names", &sets->visuals.esp_show[0]);
			ImGui::Checkbox("Bounding Box (2D)", &sets->visuals.esp_show[1]);
			ImGui::Checkbox("Skeleton ESP (Bones)", &sets->visuals.skeleton);
			ImGui::Checkbox("Active Weapon", &sets->visuals.esp_show[2]);
			ImGui::Checkbox("Snaplines to Enemies", &sets->visuals.esp_show[3]);
			ImGui::Checkbox("Footstep Indicators", &sets->visuals.esp_show[4]);
			ImGui::Checkbox("Bullet Tracers", &sets->visuals.esp_show[5]);
			ImGui::Checkbox("Health Bar", &sets->visuals.esp_bar[0]);

			ImGui::Spacing();
			ImGui::TextDisabled("Team ESP Colors");
			ColorEdit3Custom("Terrorist Color", sets->visuals.esp_t);
			ColorEdit3Custom("Counter-Terrorist Color", sets->visuals.esp_ct);

			// Thirdperson & Reverse View Section
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.48f, 0.88f, 1.00f, 1.00f), "Thirdperson View");
			ImGui::Separator();
			ImGui::Checkbox("Enable Thirdperson Camera", &sets->visuals.thirdperson);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Camera Distance", &sets->visuals.thirdperson_dist, 30.0f, 300.0f, "%.0f px");
			ImGui::Checkbox("Reverse Camera Angle (180° Look Backward)", &sets->visuals.thirdperson_reverse);

			ImGui::EndChild();
			ImGui::NextColumn();

			// --- Card 2: Asian Hat & Model Chams ---
			ImGui::BeginChild("AsianHatChamsCard", ImVec2(0, 0), true);

			// Asian Hat Section
			ImGui::TextColored(ImVec4(0.98f, 0.82f, 0.35f, 1.00f), "Asian Hat (3D Conical / Rice Hat)");
			ImGui::Separator();
			ImGui::Checkbox("Enable Asian Hat (All Players & Self)", &sets->visuals.asian_hat);
			ColorEdit3Custom("Hat Color", sets->visuals.asian_hat_color);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Hat Radius Size", &sets->visuals.asian_hat_size, 5.0f, 40.0f, "%.1f");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Hat Height", &sets->visuals.asian_hat_height, 2.0f, 30.0f, "%.1f");

			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Model Chams Customization");
			ImGui::Separator();

			const char* chams_types[] = { "Disabled", "Flat Solid", "Textured", "Material Metallic" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Chams Mode", &sets->visuals.chams, chams_types, IM_ARRAYSIZE(chams_types));

			ColorEdit3Custom("Terrorist Chams Color", sets->visuals.chams_t);
			ColorEdit3Custom("Counter-Terrorist Chams Color", sets->visuals.chams_ct);

			ImGui::Checkbox("Invisible Chams (X-Ray)", &sets->visuals.chams_style[0]);
			ImGui::Checkbox("Shine Overlay Effect", &sets->visuals.chams_style[1]);
			ImGui::Checkbox("Glow Outline Effect", &sets->visuals.chams_style[2]);

			ImGui::Spacing();
			ImGui::TextDisabled("Removals & FX");
			ImGui::Checkbox("Remove Smoke Effect", &sets->visuals.remove[0]);
			ImGui::Checkbox("Remove Flash Effect", &sets->visuals.remove[1]);
			ImGui::Checkbox("Hitmarker Indicator", &sets->visuals.hitmarker);

			ImGui::EndChild();
			ImGui::Columns(1);
		}

		// =========================================================================
		// TAB 2: MISC & UTILITIES
		// =========================================================================
		else if (current_tab == 2)
		{
			ImGui::Columns(2, "MiscCols", false);

			// --- Card 1: Movement Automations ---
			ImGui::BeginChild("MovementCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Movement & Recoil Assists");
			ImGui::Separator();

			ImGui::Checkbox("Auto Pistol Firing", &sets->misc.autopistol);
			ImGui::Checkbox("BunnyHop (Max Speed Auto Jump)", &sets->misc.autojump);
			ImGui::Checkbox("Auto Strafer Helper", &sets->misc.autostrafer);
			ImGui::Checkbox("No Recoil (No Spread / Pitch Recoil Control)", &sets->misc.norecoil);
			ImGui::Checkbox("Killshot Announcement", &sets->misc.killshot);

			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.48f, 0.88f, 1.00f, 1.00f), "Advanced Movement Assists");
			ImGui::Checkbox("Fast Ladder Climb", &sets->misc.fast_ladder);
			ImGui::Checkbox("Circle Strafe Helper", &sets->misc.circle_strafe);
			ImGui::Checkbox("Edge Jump Helper", &sets->misc.edge_jump);

			ImGui::EndChild();
			ImGui::NextColumn();

			// --- Card 2: Bypasses & On-Screen Drawing ---
			ImGui::BeginChild("BypassesCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.98f, 0.48f, 0.65f, 1.00f), "Server Bypasses & Protection");
			ImGui::Separator();

			ImGui::Checkbox("Pure Bypass (SV_Pure 1/2)", &sets->misc.pure_bypass);
			ImGui::Checkbox("Anti SMAC (Server Anti-Cheat Protection)", &sets->misc.antismac);

			ImGui::EndChild();
			ImGui::Columns(1);
		}

		// =========================================================================
		// TAB 3: NIGHTMODE & WORLD
		// =========================================================================
		else if (current_tab == 3)
		{
			ImGui::Columns(2, "WorldCols", false);

			// --- Card 1: Atmosphere ---
			ImGui::BeginChild("AtmosphereCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Atmosphere & World Lighting");
			ImGui::Separator();

			ImGui::Checkbox("Enable Nightmode & Skybox Modulation", &sets->visuals.nightmode);

			ImGui::EndChild();
			ImGui::NextColumn();

			// --- Card 2: Geometry & Fog ---
			ImGui::BeginChild("GeometryCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "World Geometry Modulation");
			ImGui::Separator();

			ImGui::TextDisabled("World & StaticProp Texture Modulation Active.");

			ImGui::EndChild();
			ImGui::Columns(1);
		}

		// =========================================================================
		// TAB 4: SETTINGS & CONFIGS
		// =========================================================================
		else if (current_tab == 4)
		{
			ImGui::Columns(2, "ConfigsCols", false);

			// --- Card 1: Manager ---
			ImGui::BeginChild("ManagerCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Profile & Hotkey Manager");
			ImGui::Separator();

			const char* key_names[] = { "INSERT", "DELETE", "HOME", "END", "F11" };
			const int key_codes[] = { VK_INSERT, VK_DELETE, VK_HOME, VK_END, VK_F11 };
			static int current_key_idx = 0;

			ImGui::SetNextItemWidth(170);
			if (ImGui::Combo("Menu Toggle Key", &current_key_idx, key_names, IM_ARRAYSIZE(key_names)))
			{
				sets->menu.menu_key = key_codes[current_key_idx];
			}

			static char config_name_buf[64] = "default_preset";
			ImGui::SetNextItemWidth(200);
			ImGui::InputText("Config Profile Name", config_name_buf, IM_ARRAYSIZE(config_name_buf));

			ImGui::Spacing();
			if (ImGui::Button("Save Profile", ImVec2(130, 32)))
			{
				if (configs::write(config_name_buf))
				{
					ImGui::OpenPopup("PopupConfigSaved");
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Load Profile", ImVec2(130, 32)))
			{
				if (configs::parse(config_name_buf))
				{
					ImGui::OpenPopup("PopupConfigLoaded");
				}
			}

			if (ImGui::BeginPopup("PopupConfigSaved"))
			{
				ImGui::Text("Config saved: C:/lovemachine/configs/%s.txt", config_name_buf);
				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("PopupConfigLoaded"))
			{
				ImGui::Text("Config loaded: C:/lovemachine/configs/%s.txt", config_name_buf);
				ImGui::EndPopup();
			}

			ImGui::Spacing();
			ImGui::Separator();
			if (ImGui::Button("Reset All to Defaults", ImVec2(270, 30)))
			{
				*sets = settings();
			}

			ImGui::EndChild();
			ImGui::NextColumn();

			// --- Card 2: Saved Profiles List ---
			ImGui::BeginChild("FilesCard", ImVec2(0, 0), true);
			ImGui::TextColored(ImVec4(0.78f, 0.48f, 1.00f, 1.00f), "Saved Profiles Directory");
			ImGui::Separator();

			auto available_configs = configs::parse_configs();
			if (available_configs.empty())
			{
				ImGui::TextDisabled("No configs found in C:/lovemachine/configs/");
				ImGui::TextDisabled("Type a profile name on the left and click 'Save Profile'.");
			}
			else
			{
				static int selected_cfg = -1;
				for (int i = 0; i < static_cast<int>(available_configs.size()); ++i)
				{
					if (ImGui::Selectable(available_configs[i], selected_cfg == i))
					{
						selected_cfg = i;
						snprintf(config_name_buf, sizeof(config_name_buf), "%s", available_configs[i]);
					}
				}
			}

			ImGui::EndChild();
			ImGui::Columns(1);
		}

		ImGui::PopID();
		ImGui::EndChild();

		ImGui::End();
	}
}

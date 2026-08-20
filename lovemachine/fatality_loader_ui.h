#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace FatalityLoaderUI
{
	// Loader State Enums
	enum LoaderStatus
	{
		STATUS_IDLE = 0,
		STATUS_WAITING_FOR_GAME,
		STATUS_INITIALIZING,
		STATUS_VERIFYING_SECURITY,
		STATUS_MANUAL_MAPPING,
		STATUS_SUCCESSFULLY_INJECTED,
		STATUS_ERROR
	};

	struct LogEntry
	{
		std::string time;
		std::string text;
		ImU32 color;
	};

	inline bool show_loader = true;
	inline int current_status = STATUS_WAITING_FOR_GAME;
	inline float inject_progress = 0.0f;
	inline bool process_found = false;
	inline int target_pid = 0;
	inline std::string username = "MinhG";
	inline std::string subscription = "Lifetime VIP";
	inline std::string expiry_date = "Never (Lifetime)";
	inline std::string selected_game = "Counter-Strike: Source (x86)";
	inline std::vector<LogEntry> logs;

	inline void AddLog(const std::string& text, ImU32 col = IM_COL32(200, 200, 210, 255))
	{
		logs.push_back({ "12:00:00", text, col });
		if (logs.size() > 50) logs.erase(logs.begin());
	}

	inline void RenderLoader(bool* p_open = nullptr)
	{
		if (!show_loader) return;

		ImGuiIO& io = ImGui::GetIO();
		float time = (float)ImGui::GetTime();

		// Fatality Color Palette
		ImU32 col_bg_dark = IM_COL32(14, 14, 18, 255);
		ImU32 col_bg_panel = IM_COL32(20, 20, 26, 255);
		ImU32 col_bg_card = IM_COL32(26, 26, 34, 255);
		ImU32 col_crimson_main = IM_COL32(235, 20, 45, 255);
		ImU32 col_crimson_glow = IM_COL32(255, 30, 60, 120);
		ImU32 col_border = IM_COL32(42, 42, 54, 255);
		ImU32 col_text_dim = IM_COL32(140, 140, 160, 255);
		ImU32 col_text_bright = IM_COL32(245, 245, 250, 255);
		ImU32 col_accent_green = IM_COL32(46, 204, 113, 255);

		ImGui::SetNextWindowSize(ImVec2(680, 480), ImGuiCond_FirstUseEver);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.055f, 0.055f, 0.071f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.165f, 0.165f, 0.212f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		if (ImGui::Begin("FatalityLoaderWindow", p_open, flags))
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetWindowPos();
			ImVec2 size = ImGui::GetWindowSize();

			// 1. Top Radiant Accent Glow Strip
			float pulse = 0.5f + 0.5f * std::sin(time * 2.5f);
			draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 3.0f), IM_COL32(235, 20 + (int)(pulse * 30), 45, 255), 8.0f, ImDrawFlags_RoundCornersTop);

			// 2. Custom Title Bar
			ImVec2 title_bar_min = pos;
			ImVec2 title_bar_max = ImVec2(pos.x + size.x, pos.y + 42.0f);
			draw_list->AddRectFilled(title_bar_min, title_bar_max, IM_COL32(18, 18, 24, 255), 8.0f, ImDrawFlags_RoundCornersTop);
			draw_list->AddLine(ImVec2(pos.x, pos.y + 42.0f), ImVec2(pos.x + size.x, pos.y + 42.0f), col_border, 1.0f);

			// Logo & Title
			draw_list->AddCircleFilled(ImVec2(pos.x + 22.0f, pos.y + 21.0f), 6.0f, col_crimson_main);
			draw_list->AddCircleFilled(ImVec2(pos.x + 22.0f, pos.y + 21.0f), 3.0f, IM_COL32(255, 255, 255, 255));
			draw_list->AddText(ImVec2(pos.x + 36.0f, pos.y + 13.0f), col_text_bright, "LOVEMACHINE");
			draw_list->AddText(ImVec2(pos.x + 138.0f, pos.y + 14.0f), col_text_dim, "// CLIENT LOADER v3.2");

			// Close & Minimize Buttons
			ImVec2 close_btn_min = ImVec2(pos.x + size.x - 36.0f, pos.y + 8.0f);
			ImVec2 close_btn_max = ImVec2(pos.x + size.x - 12.0f, pos.y + 34.0f);
			bool close_hovered = ImGui::IsMouseHoveringRect(close_btn_min, close_btn_max);
			draw_list->AddRectFilled(close_btn_min, close_btn_max, close_hovered ? col_crimson_main : IM_COL32(28, 28, 38, 255), 4.0f);
			draw_list->AddText(ImVec2(close_btn_min.x + 8.0f, close_btn_min.y + 5.0f), col_text_bright, "X");
			if (close_hovered && ImGui::IsMouseClicked(0))
			{
				show_loader = false;
				if (p_open) *p_open = false;
			}

			// Main Content Area
			ImGui::SetCursorPos(ImVec2(16.0f, 54.0f));
			ImGui::BeginGroup();
			{
				// Left Panel: User Profile & Target Game
				ImGui::BeginChild("LeftPanel", ImVec2(240, 410), false);
				{
					// User Profile Card
					ImVec2 p_min = ImGui::GetCursorScreenPos();
					ImVec2 p_max = ImVec2(p_min.x + 240, p_min.y + 110);
					draw_list->AddRectFilled(p_min, p_max, col_bg_card, 6.0f);
					draw_list->AddRect(p_min, p_max, col_border, 6.0f);

					// User Avatar Icon
					ImVec2 avatar_center(p_min.x + 36.0f, p_min.y + 36.0f);
					draw_list->AddCircleFilled(avatar_center, 20.0f, IM_COL32(38, 38, 50, 255));
					draw_list->AddCircle(avatar_center, 20.0f, col_crimson_main, 24, 2.0f);
					draw_list->AddText(ImVec2(avatar_center.x - 6.0f, avatar_center.y - 8.0f), col_text_bright, "U");

					// Online indicator
					draw_list->AddCircleFilled(ImVec2(avatar_center.x + 14.0f, avatar_center.y + 14.0f), 5.0f, col_accent_green);

					// User details
					draw_list->AddText(ImVec2(p_min.x + 68.0f, p_min.y + 18.0f), col_text_bright, username.c_str());
					draw_list->AddText(ImVec2(p_min.x + 68.0f, p_min.y + 36.0f), col_crimson_main, subscription.c_str());
					draw_list->AddText(ImVec2(p_min.x + 14.0f, p_min.y + 75.0f), col_text_dim, "HWID Status:");
					draw_list->AddText(ImVec2(p_min.x + 100.0f, p_min.y + 75.0f), col_accent_green, "Synchronized");

					ImGui::Dummy(ImVec2(0, 118));

					// Target Software Selector
					ImVec2 g_min = ImGui::GetCursorScreenPos();
					ImVec2 g_max = ImVec2(g_min.x + 240, g_min.y + 276);
					draw_list->AddRectFilled(g_min, g_max, col_bg_card, 6.0f);
					draw_list->AddRect(g_min, g_max, col_border, 6.0f);

					draw_list->AddText(ImVec2(g_min.x + 14.0f, g_min.y + 14.0f), col_text_bright, "TARGET SOFTWARE");
					draw_list->AddLine(ImVec2(g_min.x + 14.0f, g_min.y + 34.0f), ImVec2(g_max.x - 14.0f, g_min.y + 34.0f), col_border);

					// Game Selection Radio Item
					ImVec2 item_min(g_min.x + 10.0f, g_min.y + 44.0f);
					ImVec2 item_max(g_max.x - 10.0f, g_min.y + 110.0f);
					draw_list->AddRectFilled(item_min, item_max, IM_COL32(34, 34, 46, 255), 4.0f);
					draw_list->AddRect(item_min, item_max, col_crimson_main, 4.0f, 0, 1.5f);

					draw_list->AddText(ImVec2(item_min.x + 12.0f, item_min.y + 10.0f), col_text_bright, "Counter-Strike: Source");
					draw_list->AddText(ImVec2(item_min.x + 12.0f, item_min.y + 28.0f), col_text_dim, "Architecture: x86 (32-bit)");
					draw_list->AddText(ImVec2(item_min.x + 12.0f, item_min.y + 44.0f), col_accent_green, "Status: VAC Undetected");

					// Process Detector Badge
					ImVec2 proc_min(g_min.x + 10.0f, g_min.y + 124.0f);
					ImVec2 proc_max(g_max.x - 10.0f, g_min.y + 180.0f);
					draw_list->AddRectFilled(proc_min, proc_max, IM_COL32(20, 20, 26, 255), 4.0f);
					draw_list->AddRect(proc_min, proc_max, col_border, 4.0f);

					draw_list->AddText(ImVec2(proc_min.x + 10.0f, proc_min.y + 8.0f), col_text_dim, "Target Process: hl2.exe");
					if (process_found)
					{
						draw_list->AddText(ImVec2(proc_min.x + 10.0f, proc_min.y + 26.0f), col_accent_green, "Process Detected (Ready)");
					}
					else
					{
						float scan_pulse = 0.5f + 0.5f * std::sin(time * 4.0f);
						draw_list->AddText(ImVec2(proc_min.x + 10.0f, proc_min.y + 26.0f), IM_COL32(255, 180, 20, 200 + (int)(scan_pulse * 55)), "Waiting for game process...");
					}

					// Auto-Inject Checkbox
					ImGui::SetCursorPos(ImVec2(12.0f, 320.0f));
					static bool auto_inject = true;
					ImGui::Checkbox("Auto-inject on game startup", &auto_inject);
				}
				ImGui::EndChild();

				ImGui::SameLine(0, 16.0f);

				// Right Panel: Action & Diagnostic Console
				ImGui::BeginChild("RightPanel", ImVec2(392, 410), false);
				{
					// Action Button (Launch / Inject)
					ImVec2 btn_min = ImGui::GetCursorScreenPos();
					ImVec2 btn_max = ImVec2(btn_min.x + 392, btn_min.y + 58.0f);
					bool btn_hovered = ImGui::IsMouseHoveringRect(btn_min, btn_max);

					ImU32 btn_col = btn_hovered ? IM_COL32(255, 30, 60, 255) : col_crimson_main;
					draw_list->AddRectFilled(btn_min, btn_max, btn_col, 6.0f);
					// Pulsing glow when hovered
					if (btn_hovered)
					{
						draw_list->AddRect(ImVec2(btn_min.x - 2, btn_min.y - 2), ImVec2(btn_max.x + 2, btn_max.y + 2), col_crimson_glow, 8.0f, 0, 2.0f);
					}

					const char* btn_label = "LOAD & INJECT LOVEMACHINE";
					if (current_status == STATUS_MANUAL_MAPPING) btn_label = "MANUAL MAPPING DLL...";
					else if (current_status == STATUS_SUCCESSFULLY_INJECTED) btn_label = "INJECTION SUCCESSFUL!";
					
					ImVec2 text_sz = ImGui::CalcTextSize(btn_label);
					draw_list->AddText(ImVec2(btn_min.x + (392 - text_sz.x) * 0.5f, btn_min.y + 20.0f), col_text_bright, btn_label);

					if (btn_hovered && ImGui::IsMouseClicked(0))
					{
						current_status = STATUS_MANUAL_MAPPING;
						inject_progress = 0.1f;
						AddLog("[+] User clicked Load & Inject.", col_text_bright);
						AddLog("[*] Initializing bypass security layers...", col_crimson_main);
					}

					ImGui::Dummy(ImVec2(0, 68));

					// Animated Injection Progress Bar
					if (current_status == STATUS_MANUAL_MAPPING || current_status == STATUS_SUCCESSFULLY_INJECTED)
					{
						float next_prog = inject_progress + io.DeltaTime * 0.8f;
						inject_progress = (next_prog > 1.0f) ? 1.0f : next_prog;
						if (inject_progress >= 1.0f && current_status == STATUS_MANUAL_MAPPING)
						{
							current_status = STATUS_SUCCESSFULLY_INJECTED;
							AddLog("[+] Memory section allocated at 0x7FFA0000", col_text_dim);
							AddLog("[+] Relocations patched successfully.", col_text_dim);
							AddLog("[+] DllMain entry point executed.", col_accent_green);
							AddLog("[SUCCESS] Cheat successfully attached to game!", col_accent_green);
						}

						ImVec2 bar_min = ImGui::GetCursorScreenPos();
						ImVec2 bar_max = ImVec2(bar_min.x + 392, bar_min.y + 10.0f);
						draw_list->AddRectFilled(bar_min, bar_max, IM_COL32(26, 26, 34, 255), 4.0f);
						draw_list->AddRectFilled(bar_min, ImVec2(bar_min.x + 392 * inject_progress, bar_max.y), col_crimson_main, 4.0f);
						ImGui::Dummy(ImVec2(0, 16));
					}

					// Diagnostic Console Log Terminal
					ImVec2 term_min = ImGui::GetCursorScreenPos();
					ImVec2 term_max = ImVec2(term_min.x + 392, term_min.y + 310);
					draw_list->AddRectFilled(term_min, term_max, IM_COL32(10, 10, 14, 255), 6.0f);
					draw_list->AddRect(term_min, term_max, col_border, 6.0f);

					// Terminal Header
					draw_list->AddRectFilled(term_min, ImVec2(term_max.x, term_min.y + 26.0f), IM_COL32(18, 18, 24, 255), 6.0f, ImDrawFlags_RoundCornersTop);
					draw_list->AddText(ImVec2(term_min.x + 10.0f, term_min.y + 6.0f), col_text_dim, "DIAGNOSTIC LOG CONSOLE");

					// Log Entries Box
					ImGui::SetCursorPos(ImVec2(10.0f, 106.0f));
					ImGui::BeginChild("LogScroll", ImVec2(372, 280), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					{
						if (logs.empty())
						{
							AddLog("[INFO] LoveMachine Client Loader initialized.", col_text_dim);
							AddLog("[INFO] Checking authentication session... OK", col_accent_green);
							AddLog("[INFO] Ready to inject target DLL: lovemachine.dll", col_text_bright);
						}

						for (const auto& log : logs)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, log.color);
							ImGui::TextUnformatted(log.text.c_str());
							ImGui::PopStyleColor();
						}

						if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
							ImGui::SetScrollHereY(1.0f);
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}
			ImGui::EndGroup();
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(2);
	}
}

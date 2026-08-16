#include "menu.h"
#include "hashes.hpp"
#include "imgui_internal.h"
#include "configs.h"
#include "phoenix_mesh.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace Menu
{
	bool show_menu = false;
	bool is_binding_key = false;
	int current_tab = 3; // Default to Players ESP
	int current_theme = THEME_SKEET;
	int current_layout = LAYOUT_GAMESENSE;
	ImFont* font_skeet_icons = nullptr;
	ImFont* font_main = nullptr;
	ImFont* font_brand_title = nullptr;

	// Global UI Customization States & Theme Tokens
	static float ui_anim_speed = 1.0f;
	static bool ui_compact_mode = true;
	static ImVec4 ui_accent_color = ImVec4(0.635f, 0.847f, 0.098f, 1.00f); // Gamesense Emerald Accent (#A2D819)

	// Helper for ImGui ColorEdit3 mapped to custom color struct
	static bool ColorEdit3Custom(const char* label, color& col)
	{
		col.r = std::max(0, std::min(col.r, 255));
		col.g = std::max(0, std::min(col.g, 255));
		col.b = std::max(0, std::min(col.b, 255));
		float c[3] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f };
		if (ImGui::ColorEdit3(label, c, ImGuiColorEditFlags_NoInputs))
		{
			col.r = static_cast<int>(c[0] * 255.0f);
			col.g = static_cast<int>(c[1] * 255.0f);
			col.b = static_cast<int>(c[2] * 255.0f);
			return true;
		}
		return false;
	}

	// Modern Dynamic Checkbox / Animated Switch
	static bool AnimatedSwitch(const char* label, bool* v)
	{
		if (current_layout != LAYOUT_NEVERLOSE)
		{
			// Authentic crisp square checkbox with custom theme accent
			return ImGui::Checkbox(label, v);
		}

		// Authentic Neverlose Switch for LAYOUT_NEVERLOSE
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		const float height = 18.0f;
		const float width = 36.0f;
		const float radius = height * 0.5f;

		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0 ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + ImMax(label_size.y, height)));
		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id)) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) *v = !*v;

		static std::unordered_map<ImGuiID, float> anim_map;
		float& anim_t = anim_map[id];
		float target_t = *v ? 1.0f : 0.0f;
		float dt = ImGui::GetIO().DeltaTime * 16.0f * ui_anim_speed;
		anim_t += (target_t - anim_t) * dt;
		if (std::abs(target_t - anim_t) < 0.001f) anim_t = target_t;

		ImU32 bg_off = IM_COL32(22, 28, 42, 240);
		ImU32 bg_on = IM_COL32((int)(ui_accent_color.x * 255), (int)(ui_accent_color.y * 255), (int)(ui_accent_color.z * 255), 255);
		if (hovered && !*v) {
			bg_off = IM_COL32(32, 42, 64, 255);
		}

		int cr = (int)((1.0f - anim_t) * ((bg_off >> 0) & 0xFF) + anim_t * ((bg_on >> 0) & 0xFF));
		int cg = (int)((1.0f - anim_t) * ((bg_off >> 8) & 0xFF) + anim_t * ((bg_on >> 8) & 0xFF));
		int cb = (int)((1.0f - anim_t) * ((bg_off >> 16) & 0xFF) + anim_t * ((bg_on >> 16) & 0xFF));
		ImU32 bg_col = IM_COL32(cr, cg, cb, 255);

		const ImRect switch_bb(pos, ImVec2(pos.x + width, pos.y + height));
		window->DrawList->AddRectFilled(switch_bb.Min, switch_bb.Max, bg_col, radius);
		window->DrawList->AddRect(switch_bb.Min, switch_bb.Max, IM_COL32(255, 255, 255, 20), radius);

		if (anim_t > 0.01f)
		{
			window->DrawList->AddRect(
				ImVec2(switch_bb.Min.x - 1.5f, switch_bb.Min.y - 1.5f),
				ImVec2(switch_bb.Max.x + 1.5f, switch_bb.Max.y + 1.5f),
				IM_COL32(cr, cg, cb, (int)(110 * anim_t)), radius + 1.5f, 0, 1.5f
			);
		}

		float knob_x = pos.x + radius + anim_t * (width - radius * 2.0f);
		window->DrawList->AddCircleFilled(ImVec2(knob_x, pos.y + radius), radius - 2.5f, IM_COL32(255, 255, 255, 255));

		if (label_size.x > 0.0f)
			ImGui::RenderText(ImVec2(pos.x + width + style.ItemInnerSpacing.x + 2.0f, pos.y + (height - label_size.y) * 0.5f), label);

		return pressed;
	}

	// Interactive Real-Time High-Contrast 3D ESP Live Preview Renderer
	static void RenderEspLivePreview(ImVec2 size, int preview_type = 0)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 cursor = ImGui::GetCursorScreenPos();

		// 1. Sleek Glassmorphism Card Container Background
		draw_list->AddRectFilled(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), IM_COL32(10, 14, 24, 245), 12.0f);
		draw_list->AddRect(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), IM_COL32(0, 210, 255, 45), 12.0f, 0, 1.5f);
		draw_list->AddRect(ImVec2(cursor.x + 1.0f, cursor.y + 1.0f), ImVec2(cursor.x + size.x - 1.0f, cursor.y + size.y - 1.0f), IM_COL32(255, 255, 255, 12), 11.0f);

		// Grid Lines Accent for 3D depth feeling
		for (float gx = cursor.x + 20.0f; gx < cursor.x + size.x - 10.0f; gx += 30.0f)
			draw_list->AddLine(ImVec2(gx, cursor.y + 35.0f), ImVec2(gx, cursor.y + size.y - 10.0f), IM_COL32(20, 30, 48, 60), 1.0f);
		for (float gy = cursor.y + 40.0f; gy < cursor.y + size.y - 10.0f; gy += 30.0f)
			draw_list->AddLine(ImVec2(cursor.x + 10.0f, gy), ImVec2(cursor.x + size.x - 10.0f, gy), IM_COL32(20, 30, 48, 60), 1.0f);

		// Dynamic Header Label
		std::string header_txt = (preview_type == 0) ? 
			(sets->visuals.enable_custom_model && sets->visuals.model_selection == 1 ? "3D Model: Cissia (Zenless Zone Zero)" : "Interactive 3D Player ESP Preview") 
			: (preview_type == 1 ? "Interactive 3D Weapon Preview" : "Interactive 3D C4 & Grenade Preview");
		draw_list->AddText(ImVec2(cursor.x + 16, cursor.y + 12), IM_COL32(0, 220, 255, 255), header_txt.c_str());
		draw_list->AddLine(ImVec2(cursor.x + 14, cursor.y + 32), ImVec2(cursor.x + size.x - 14, cursor.y + 32), IM_COL32(35, 50, 80, 180), 1.0f);

		float center_x = cursor.x + size.x * 0.5f;
		float center_y = cursor.y + size.y * 0.52f;

		// =========================================================================
		// TYPE 0: HIGH-CONTRAST VIBRANT 3D PLAYER MODEL ESP PREVIEW
		// =========================================================================
		if (preview_type == 0)
		{
			float model_h = size.y * 0.56f;
			float model_w = model_h * 0.44f;

			ImVec2 box_min(center_x - model_w * 0.5f, center_y - model_h * 0.5f);
			ImVec2 box_max(center_x + model_w * 0.5f, center_y + model_h * 0.5f);

			float scale = model_h / 80.0f; // Scale 3D model height to fit box

			// Smooth 3D rotation animation
			static float model_angle = 0.0f;
			model_angle += ImGui::GetIO().DeltaTime * 0.55f * ui_anim_speed;
			float cos_a = std::cos(model_angle);
			float sin_a = std::sin(model_angle);

			// Transform & Project 3D Vertices to 2D Screen Space
			static std::vector<ImVec2> projected_pts(PHOENIX_VERTEX_COUNT);
			static std::vector<float> proj_depth(PHOENIX_VERTEX_COUNT);

			for (int i = 0; i < PHOENIX_VERTEX_COUNT; i++)
			{
				const auto& v = g_phoenix_vertices[i];
				float rx = v.x * cos_a - v.y * sin_a;
				float ry = v.x * sin_a + v.y * cos_a;
				float rz = v.z;

				projected_pts[i] = ImVec2(center_x + rx * scale, center_y + (38.0f - rz) * scale);
				proj_depth[i] = ry;
			}

			// Depth-sorted Triangle Render Structure (Back-to-Front Painter's Algorithm)
			struct TriangleDraw
			{
				ImVec2 p0, p1, p2;
				float depth;
				ImU32 fill_color;
			};

			static std::vector<TriangleDraw> tri_list;
			tri_list.clear();
			tri_list.reserve(PHOENIX_INDEX_COUNT / 3);

			bool is_chams = (sets->visuals.chams > 0);

			for (int i = 0; i < PHOENIX_INDEX_COUNT; i += 3)
			{
				uint16_t i0 = g_phoenix_indices[i];
				uint16_t i1 = g_phoenix_indices[i + 1];
				uint16_t i2 = g_phoenix_indices[i + 2];

				ImVec2 p0 = projected_pts[i0];
				ImVec2 p1 = projected_pts[i1];
				ImVec2 p2 = projected_pts[i2];

				// Backface culling in 2D projection
				float cross = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
				if (cross > 0.0f)
				{
					float avg_depth = proj_depth[i0] + proj_depth[i1] + proj_depth[i2];
					// High-contrast vibrant lighting multiplier (0.65f to 1.15f)
					float light = ImMin(1.15f, ImMax(0.65f, cross * 0.0025f));

					ImU32 fill_col;
					if (sets->visuals.chams == 1) // 1. Flat Colored (100% Solid Flat Unshaded Vibrant Material)
					{
						fill_col = IM_COL32(sets->visuals.chams_t.r, sets->visuals.chams_t.g, sets->visuals.chams_t.b, 255);
					}
					else if (sets->visuals.chams == 2) // 2. Material Shaded (Rich 3D Phong Specular & Glossy Sheen)
					{
						float specular = powf(ImMax(0.0f, (light - 0.70f) / 0.45f), 3.0f) * 75.0f;
						int cr = ImMin(255, (int)(sets->visuals.chams_t.r * (0.60f + light * 0.50f) + specular));
						int cg = ImMin(255, (int)(sets->visuals.chams_t.g * (0.60f + light * 0.50f) + specular));
						int cb = ImMin(255, (int)(sets->visuals.chams_t.b * (0.60f + light * 0.50f) + specular));
						fill_col = IM_COL32(cr, cg, cb, 255);
					}
					else if (sets->visuals.chams == 3) // 3. Wireframe (Holographic Cyber Body)
					{
						fill_col = IM_COL32(12, 16, 24, 180);
					}
					else // 0. Disabled (Standard CS:S Phoenix 3D Textured Model)
					{
						// High-contrast crisp texture colors sampled from t_phoenix_baseColor.png
						int tex_r = (g_phoenix_vertices[i0].r + g_phoenix_vertices[i1].r + g_phoenix_vertices[i2].r) / 3;
						int tex_g = (g_phoenix_vertices[i0].g + g_phoenix_vertices[i1].g + g_phoenix_vertices[i2].g) / 3;
						int tex_b = (g_phoenix_vertices[i0].b + g_phoenix_vertices[i1].b + g_phoenix_vertices[i2].b) / 3;

						float light_mult = 0.70f + light * 0.50f;
						int cr = ImMin(255, (int)(tex_r * light_mult));
						int cg = ImMin(255, (int)(tex_g * light_mult));
						int cb = ImMin(255, (int)(tex_b * light_mult));

						fill_col = IM_COL32(cr, cg, cb, 255);
					}

					tri_list.push_back({ p0, p1, p2, avg_depth, fill_col });
				}
			}

			// Painter's Algorithm: Sort Triangles Back-to-Front by Depth
			std::sort(tri_list.begin(), tri_list.end(), [](const TriangleDraw& a, const TriangleDraw& b) {
				return a.depth < b.depth;
			});

			// Render Triangles
			for (const auto& tri : tri_list)
			{
				draw_list->AddTriangleFilled(tri.p0, tri.p1, tri.p2, tri.fill_color);
			}

			// Mode-specific Edge Overlay
			if (sets->visuals.chams == 3) // Wireframe: Glowing Neon Polyline Mesh
			{
				for (const auto& tri : tri_list)
				{
					draw_list->AddTriangle(tri.p0, tri.p1, tri.p2, IM_COL32(sets->visuals.chams_t.r, sets->visuals.chams_t.g, sets->visuals.chams_t.b, 240), 1.2f);
				}
			}
			else if (sets->visuals.chams == 1) // Flat: Crisp silhouette borders
			{
				for (const auto& tri : tri_list)
				{
					draw_list->AddTriangle(tri.p0, tri.p1, tri.p2, IM_COL32(10, 10, 15, 60), 0.7f);
				}
			}
			else if (sets->visuals.chams == 2) // Shaded: Glossy rim highlight
			{
				for (const auto& tri : tri_list)
				{
					draw_list->AddTriangle(tri.p0, tri.p1, tri.p2, IM_COL32(255, 255, 255, 40), 0.8f);
				}
			}

			// Exact 3D Skeleton Joint Tracking anchored to actual 3D Mesh Vertices of tr_phoenix
			ImVec2 j_head = projected_pts[2709];
			ImVec2 j_neck = projected_pts[1022];
			ImVec2 j_l_shoulder = projected_pts[1792];
			ImVec2 j_r_shoulder = projected_pts[1070];
			ImVec2 j_l_elbow = projected_pts[680];
			ImVec2 j_r_elbow = projected_pts[1290];
			ImVec2 j_l_hand = projected_pts[520];
			ImVec2 j_r_hand = projected_pts[1465];
			ImVec2 j_pelvis = projected_pts[1632];
			ImVec2 j_l_knee = projected_pts[1906];
			ImVec2 j_r_knee = projected_pts[1691];
			ImVec2 j_l_foot = projected_pts[2370];
			ImVec2 j_r_foot = projected_pts[981];

			// 3D Skeleton ESP (100% Perfectly Attached to rotating character mesh)
			if (sets->visuals.skeleton)
			{
				ImU32 skel_col = IM_COL32(sets->visuals.esp_t.r, sets->visuals.esp_t.g, sets->visuals.esp_t.b, 255);
				draw_list->AddLine(j_head, j_neck, skel_col, 2.5f);
				draw_list->AddLine(j_neck, j_pelvis, skel_col, 2.5f);
				draw_list->AddLine(j_neck, j_l_shoulder, skel_col, 2.5f);
				draw_list->AddLine(j_l_shoulder, j_l_elbow, skel_col, 2.0f);
				draw_list->AddLine(j_l_elbow, j_l_hand, skel_col, 2.0f);
				draw_list->AddLine(j_neck, j_r_shoulder, skel_col, 2.5f);
				draw_list->AddLine(j_r_shoulder, j_r_elbow, skel_col, 2.0f);
				draw_list->AddLine(j_r_elbow, j_r_hand, skel_col, 2.0f);
				draw_list->AddLine(j_pelvis, j_l_knee, skel_col, 2.5f);
				draw_list->AddLine(j_l_knee, j_l_foot, skel_col, 2.5f);
				draw_list->AddLine(j_pelvis, j_r_knee, skel_col, 2.5f);
				draw_list->AddLine(j_r_knee, j_r_foot, skel_col, 2.5f);

				// Glowing Neon Joint Spheres
				ImU32 joint_glow = IM_COL32(255, 255, 255, 255);
				draw_list->AddCircleFilled(j_head, 4.0f, joint_glow);
				draw_list->AddCircleFilled(j_neck, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_l_shoulder, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_r_shoulder, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_l_elbow, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_r_elbow, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_l_hand, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_r_hand, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_pelvis, 3.5f, joint_glow);
				draw_list->AddCircleFilled(j_l_knee, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_r_knee, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_l_foot, 3.0f, joint_glow);
				draw_list->AddCircleFilled(j_r_foot, 3.0f, joint_glow);
			}

			// 3D Conical Asian Rice Hat (Anchored right above top head vertex 2674)
			if (sets->visuals.asian_hat)
			{
				float hat_h = sets->visuals.asian_hat_height * 2.2f;
				float hat_r = sets->visuals.asian_hat_size * 1.4f;

				const auto& hv = g_phoenix_vertices[2674];
				float rx = hv.x * cos_a - hv.y * sin_a;
				float rz = hv.z + hat_h;
				ImVec2 apex(center_x + rx * scale, center_y + (38.0f - rz) * scale);

				ImU32 hat_fill = IM_COL32(sets->visuals.asian_hat_color.r, sets->visuals.asian_hat_color.g, sets->visuals.asian_hat_color.b, 175);
				ImU32 hat_outline = IM_COL32(sets->visuals.asian_hat_color.r, sets->visuals.asian_hat_color.g, sets->visuals.asian_hat_color.b, 255);

				int num_pts = 16;
				std::vector<ImVec2> base_pts(num_pts);
				for (int i = 0; i < num_pts; i++)
				{
					float rad = (float)i * (2.0f * 3.14159f / (float)num_pts);
					float bx = hv.x + std::cos(rad) * hat_r;
					float by = hv.y + std::sin(rad) * hat_r;
					float bz = hv.z - 2.0f;

					float brx = bx * cos_a - by * sin_a;
					base_pts[i] = ImVec2(center_x + brx * scale, center_y + (38.0f - bz) * scale);
				}

				for (int i = 0; i < num_pts; i++)
				{
					ImVec2 p1 = base_pts[i];
					ImVec2 p2 = base_pts[(i + 1) % num_pts];
					draw_list->AddTriangleFilled(apex, p1, p2, hat_fill);
					draw_list->AddTriangle(apex, p1, p2, hat_outline, 1.2f);
				}

				draw_list->AddCircleFilled(apex, 4.0f, IM_COL32(255, 255, 255, 255));
			}

			// --- DYNAMICALLY POSITIONED 2D ESP INDICATORS (INTERACTIVE CLICKABLE) ---
			ImU32 esp_col = IM_COL32(sets->visuals.esp_t.r, sets->visuals.esp_t.g, sets->visuals.esp_t.b, 255);
			ImVec2 mouse_pos = ImGui::GetIO().MousePos;
			bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

			// Instruction banner for interactive 3D model positioning
			draw_list->AddText(ImVec2(cursor.x + 16, cursor.y + size.y - 24), IM_COL32(0, 220, 255, 220), "Click Name/HP/AP/Weapon on 3D Model to change position!");

			// 2D Bounding Box around player
			if (sets->visuals.esp_show[0])
			{
				draw_list->AddRect(box_min, box_max, esp_col, 0, 0, 1.8f);
				draw_list->AddRect(ImVec2(box_min.x - 1, box_min.y - 1), ImVec2(box_max.x + 1, box_max.y + 1), IM_COL32(0, 0, 0, 180), 0, 0, 1.0f);
			}

			// Player Name Tag (Interactive Clickable)
			if (sets->visuals.esp_show[0])
			{
				const char* p_name = "Enemy_Player [14.2m]";
				ImVec2 name_sz = ImGui::CalcTextSize(p_name);
				ImVec2 name_pos;

				if (sets->visuals.name_pos == 0) // Top
					name_pos = ImVec2(center_x - name_sz.x * 0.5f, box_min.y - name_sz.y - 4.0f);
				else if (sets->visuals.name_pos == 1) // Bottom
					name_pos = ImVec2(center_x - name_sz.x * 0.5f, box_max.y + 4.0f);
				else if (sets->visuals.name_pos == 2) // Left
					name_pos = ImVec2(box_min.x - name_sz.x - 8.0f, center_y - name_sz.y * 0.5f);
				else // Right
					name_pos = ImVec2(box_max.x + 8.0f, center_y - name_sz.y * 0.5f);

				ImRect name_bb(ImVec2(name_pos.x - 4, name_pos.y - 2), ImVec2(name_pos.x + name_sz.x + 4, name_pos.y + name_sz.y + 2));
				bool name_hovered = name_bb.Contains(mouse_pos);
				if (name_hovered)
				{
					draw_list->AddRectFilled(name_bb.Min, name_bb.Max, IM_COL32(0, 200, 255, 250), 4.0f);
					if (mouse_clicked) { sets->visuals.name_pos = (sets->visuals.name_pos + 1) % 4; }
				}
				else
				{
					draw_list->AddRectFilled(name_bb.Min, name_bb.Max, IM_COL32(12, 16, 26, 220), 4.0f);
				}
				draw_list->AddText(name_pos, IM_COL32(255, 255, 255, 255), p_name);
			}

			// Health Bar (Interactive Clickable)
			if (sets->visuals.esp_show[1])
			{
				float hp_pct = 0.85f;
				ImU32 hp_col_top = IM_COL32(100, 255, 120, 255);
				ImU32 hp_col_bot = IM_COL32(255, 200, 40, 255);

				if (sets->visuals.health_bar_pos == 0 || sets->visuals.health_bar_pos == 1) // Left or Right
				{
					float x_off = (sets->visuals.health_bar_pos == 0) ? (box_min.x - 8.0f) : (box_max.x + 4.0f);
					ImVec2 bar_min(x_off - 4.0f, box_min.y);
					ImVec2 bar_max(x_off, box_max.y);

					ImRect hp_bb(ImVec2(bar_min.x - 4, bar_min.y - 2), ImVec2(bar_max.x + 4, bar_max.y + 2));
					bool hp_hovered = hp_bb.Contains(mouse_pos);
					if (hp_hovered && mouse_clicked) { sets->visuals.health_bar_pos = (sets->visuals.health_bar_pos + 1) % 4; }

					draw_list->AddRectFilled(bar_min, bar_max, IM_COL32(10, 12, 18, 230), 2.0f);
					draw_list->AddRect(ImVec2(bar_min.x - 1.0f, bar_min.y - 1.0f), ImVec2(bar_max.x + 1.0f, bar_max.y + 1.0f), hp_hovered ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 0, 0, 220), 2.0f, 0, 1.0f);

					float fill_h = (bar_max.y - bar_min.y) * hp_pct;
					ImVec2 fill_min(bar_min.x, bar_max.y - fill_h);
					draw_list->AddRectFilledMultiColor(fill_min, bar_max, hp_col_top, hp_col_top, hp_col_bot, hp_col_bot);

					draw_list->AddText(ImVec2(bar_min.x - 18.0f, fill_min.y - 5.0f), IM_COL32(255, 255, 255, 255), "85");
				}
				else // Top or Bottom
				{
					float y_off = (sets->visuals.health_bar_pos == 2) ? (box_min.y - 8.0f) : (box_max.y + 4.0f);
					ImVec2 bar_min(box_min.x, y_off);
					ImVec2 bar_max(box_max.x, y_off + 4.0f);

					ImRect hp_bb(ImVec2(bar_min.x - 2, bar_min.y - 4), ImVec2(bar_max.x + 2, bar_max.y + 4));
					bool hp_hovered = hp_bb.Contains(mouse_pos);
					if (hp_hovered && mouse_clicked) { sets->visuals.health_bar_pos = (sets->visuals.health_bar_pos + 1) % 4; }

					draw_list->AddRectFilled(bar_min, bar_max, IM_COL32(10, 12, 18, 230), 2.0f);
					draw_list->AddRect(ImVec2(bar_min.x - 1.0f, bar_min.y - 1.0f), ImVec2(bar_max.x + 1.0f, bar_max.y + 1.0f), hp_hovered ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 0, 0, 220), 2.0f, 0, 1.0f);

					float fill_w = (bar_max.x - bar_min.x) * hp_pct;
					ImVec2 fill_max(bar_min.x + fill_w, bar_max.y);
					draw_list->AddRectFilledMultiColor(bar_min, fill_max, hp_col_top, hp_col_top, hp_col_bot, hp_col_bot);
				}
			}

			// Armor Bar (Interactive Clickable)
			if (sets->visuals.esp_show[1])
			{
				float ap_pct = 1.00f;
				ImU32 ap_col_top = IM_COL32(0, 200, 255, 255);
				ImU32 ap_col_bot = IM_COL32(100, 120, 255, 255);

				if (sets->visuals.armor_bar_pos == 0 || sets->visuals.armor_bar_pos == 1) // Left or Right
				{
					float x_off = (sets->visuals.armor_bar_pos == 0) ? (box_min.x - 16.0f) : (box_max.x + 12.0f);
					ImVec2 bar_min(x_off - 4.0f, box_min.y);
					ImVec2 bar_max(x_off, box_max.y);

					ImRect ap_bb(ImVec2(bar_min.x - 4, bar_min.y - 2), ImVec2(bar_max.x + 4, bar_max.y + 2));
					bool ap_hovered = ap_bb.Contains(mouse_pos);
					if (ap_hovered && mouse_clicked) { sets->visuals.armor_bar_pos = (sets->visuals.armor_bar_pos + 1) % 4; }

					draw_list->AddRectFilled(bar_min, bar_max, IM_COL32(10, 12, 18, 230), 2.0f);
					draw_list->AddRect(ImVec2(bar_min.x - 1.0f, bar_min.y - 1.0f), ImVec2(bar_max.x + 1.0f, bar_max.y + 1.0f), ap_hovered ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 0, 0, 220), 2.0f, 0, 1.0f);

					float fill_h = (bar_max.y - bar_min.y) * ap_pct;
					ImVec2 fill_min(bar_min.x, bar_max.y - fill_h);
					draw_list->AddRectFilledMultiColor(fill_min, bar_max, ap_col_top, ap_col_top, ap_col_bot, ap_col_bot);

					float badge_x = (sets->visuals.armor_bar_pos == 0) ? (bar_min.x - 14.0f) : (bar_max.x + 3.0f);
					draw_list->AddText(ImVec2(badge_x, bar_min.y - 2.0f), IM_COL32(0, 200, 255, 255), "HK");
				}
				else
				{
					float y_off = (sets->visuals.armor_bar_pos == 2) ? (box_min.y - 13.0f) : (box_max.y + 9.0f);
					ImVec2 bar_min(box_min.x, y_off);
					ImVec2 bar_max(box_max.x, y_off + 4.0f);

					ImRect ap_bb(ImVec2(bar_min.x - 2, bar_min.y - 4), ImVec2(bar_max.x + 2, bar_max.y + 4));
					bool ap_hovered = ap_bb.Contains(mouse_pos);
					if (ap_hovered && mouse_clicked) { sets->visuals.armor_bar_pos = (sets->visuals.armor_bar_pos + 1) % 4; }

					draw_list->AddRectFilled(bar_min, bar_max, IM_COL32(10, 12, 18, 230), 2.0f);
					draw_list->AddRect(ImVec2(bar_min.x - 1.0f, bar_min.y - 1.0f), ImVec2(bar_max.x + 1.0f, bar_max.y + 1.0f), ap_hovered ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 0, 0, 220), 2.0f, 0, 1.0f);

					float fill_w = (bar_max.x - bar_min.x) * ap_pct;
					ImVec2 fill_max(bar_min.x + fill_w, bar_max.y);
					draw_list->AddRectFilledMultiColor(bar_min, fill_max, ap_col_top, ap_col_top, ap_col_bot, ap_col_bot);
				}
			}

			// Active Weapon Text (Interactive Clickable)
			if (sets->visuals.esp_show[2])
			{
				const char* wpn_txt = "AK-47 | Redline [30/90]";
				ImVec2 wpn_sz = ImGui::CalcTextSize(wpn_txt);
				ImVec2 wpn_pos;

				if (sets->visuals.weapon_pos == 0) // Top
					wpn_pos = ImVec2(center_x - wpn_sz.x * 0.5f, box_min.y - wpn_sz.y - 22.0f);
				else if (sets->visuals.weapon_pos == 1) // Bottom
					wpn_pos = ImVec2(center_x - wpn_sz.x * 0.5f, box_max.y + 5.0f);
				else if (sets->visuals.weapon_pos == 2) // Left
					wpn_pos = ImVec2(box_min.x - wpn_sz.x - 14.0f, center_y + 10.0f);
				else // Right
					wpn_pos = ImVec2(box_max.x + 14.0f, center_y + 10.0f);

				ImRect wpn_bb(ImVec2(wpn_pos.x - 4, wpn_pos.y - 2), ImVec2(wpn_pos.x + wpn_sz.x + 4, wpn_pos.y + wpn_sz.y + 2));
				bool wpn_hovered = wpn_bb.Contains(mouse_pos);
				if (wpn_hovered)
				{
					draw_list->AddRectFilled(wpn_bb.Min, wpn_bb.Max, IM_COL32(255, 200, 50, 240), 4.0f);
					if (mouse_clicked) { sets->visuals.weapon_pos = (sets->visuals.weapon_pos + 1) % 4; }
				}
				else
				{
					draw_list->AddRectFilled(wpn_bb.Min, wpn_bb.Max, IM_COL32(12, 16, 26, 220), 4.0f);
				}
				draw_list->AddText(wpn_pos, IM_COL32(255, 255, 255, 255), wpn_txt);
			}

			// Snaplines
			if (sets->visuals.esp_show[3])
			{
				draw_list->AddLine(ImVec2(center_x, cursor.y + size.y - 12.0f), ImVec2(center_x, box_max.y), esp_col, 1.5f);
			}
		}

		ImGui::Dummy(size);
	}

	// =========================================================================
	// THEME DESIGN SYSTEM (AUTHENTIC TO SCREENSHOTS)
	// =========================================================================

	void ApplyTheme(int theme_id)
	{
		current_theme = theme_id;
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding     = ImVec2(12.0f, 12.0f);
		style.FramePadding      = ImVec2(8.0f, 4.0f);
		style.ItemSpacing       = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing  = ImVec2(6.0f, 6.0f);
		style.PopupBorderSize   = 1.0f;

		switch (theme_id)
		{
		case THEME_SKEET: // 2. Gamesense / Skeet (Screenshot 3)
			ui_accent_color = ImVec4(0.635f, 0.847f, 0.098f, 1.00f); // Lime Green #A2D819
			style.WindowRounding    = 4.0f;
			style.ChildRounding     = 2.0f;
			style.FrameRounding     = 2.0f;
			style.PopupRounding     = 2.0f;
			style.ScrollbarRounding = 2.0f;
			style.GrabRounding      = 1.0f;
			style.TabRounding       = 2.0f;
			style.WindowBorderSize  = 1.0f;
			style.ChildBorderSize   = 1.0f;
			style.FrameBorderSize   = 1.0f;

			style.Colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
			style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.42f, 0.48f, 1.00f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.07f, 0.07f, 0.08f, 0.98f);
			style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.09f, 0.09f, 0.10f, 0.98f);
			style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.09f, 0.98f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
			style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.07f, 0.07f, 0.08f, 0.60f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = ui_accent_color;
			style.Colors[ImGuiCol_CheckMark]             = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrab]            = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.75f, 0.95f, 0.20f, 1.00f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
			style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
			style.Colors[ImGuiCol_ButtonActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Header]                = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
			style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Separator]             = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_SeparatorHovered]      = ui_accent_color;
			style.Colors[ImGuiCol_SeparatorActive]       = ui_accent_color;
			break;

		case THEME_CYBERPUNK: // 3. Onetap v4 (Screenshot 4)
			ui_accent_color = ImVec4(1.00f, 0.25f, 0.25f, 1.00f); // Onetap Red / Orange Accent
			style.WindowRounding    = 8.0f;
			style.ChildRounding     = 6.0f;
			style.FrameRounding     = 4.0f;
			style.PopupRounding     = 6.0f;
			style.ScrollbarRounding = 4.0f;
			style.GrabRounding      = 3.0f;
			style.TabRounding       = 4.0f;
			style.WindowBorderSize  = 1.0f;
			style.ChildBorderSize   = 1.0f;
			style.FrameBorderSize   = 0.0f;

			style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
			style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.45f, 0.48f, 0.52f, 1.00f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.08f, 0.09f, 0.11f, 0.98f);
			style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.11f, 0.12f, 0.14f, 0.95f);
			style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.11f, 0.13f, 0.98f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
			style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
			style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);
			style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.11f, 0.12f, 0.14f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.09f, 0.11f, 0.60f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.24f, 0.26f, 0.30f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = ui_accent_color;
			style.Colors[ImGuiCol_CheckMark]             = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrab]            = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 0.40f, 0.40f, 1.00f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.20f, 0.22f, 0.26f, 1.00f);
			style.Colors[ImGuiCol_ButtonActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Header]                = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Separator]             = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_SeparatorHovered]      = ui_accent_color;
			style.Colors[ImGuiCol_SeparatorActive]       = ui_accent_color;
			break;

		case THEME_ONYX: // 4. Fatality.win (Screenshot 5)
			ui_accent_color = ImVec4(0.96f, 0.20f, 0.38f, 1.00f); // Fatality Crimson Pink #F53361
			style.WindowRounding    = 8.0f;
			style.ChildRounding     = 6.0f;
			style.FrameRounding     = 4.0f;
			style.PopupRounding     = 6.0f;
			style.ScrollbarRounding = 4.0f;
			style.GrabRounding      = 3.0f;
			style.TabRounding       = 4.0f;
			style.WindowBorderSize  = 1.0f;
			style.ChildBorderSize   = 1.0f;
			style.FrameBorderSize   = 0.0f;

			style.Colors[ImGuiCol_Text]                  = ImVec4(0.94f, 0.94f, 0.96f, 1.00f);
			style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.08f, 0.08f, 0.09f, 0.98f);
			style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.10f, 0.10f, 0.11f, 0.96f);
			style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.09f, 0.09f, 0.10f, 0.98f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
			style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.08f, 0.09f, 0.60f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = ui_accent_color;
			style.Colors[ImGuiCol_CheckMark]             = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrab]            = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 0.35f, 0.50f, 1.00f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
			style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
			style.Colors[ImGuiCol_ButtonActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Header]                = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
			style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Separator]             = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			style.Colors[ImGuiCol_SeparatorHovered]      = ui_accent_color;
			style.Colors[ImGuiCol_SeparatorActive]       = ui_accent_color;
			break;

		case THEME_NEVERLOSE:
		default: // 1. Neverlose 2.0 (Screenshots 1 & 2)
			ui_accent_color = ImVec4(0.35f, 0.50f, 0.98f, 1.00f); // Neverlose Royal Blue #5980FA
			style.WindowRounding    = 14.0f;
			style.ChildRounding     = 10.0f;
			style.FrameRounding     = 6.0f;
			style.PopupRounding     = 8.0f;
			style.ScrollbarRounding = 6.0f;
			style.GrabRounding      = 5.0f;
			style.TabRounding       = 6.0f;
			style.WindowBorderSize  = 1.0f;
			style.ChildBorderSize   = 1.0f;
			style.FrameBorderSize   = 0.0f;

			style.Colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.94f, 0.98f, 1.00f);
			style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.44f, 0.48f, 0.56f, 1.00f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.10f, 0.13f, 0.98f);
			style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.11f, 0.13f, 0.17f, 0.95f);
			style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.12f, 0.15f, 0.98f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.16f, 0.18f, 0.23f, 1.00f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.21f, 0.28f, 1.00f);
			style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.26f, 0.34f, 1.00f);
			style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.09f, 0.10f, 0.13f, 1.00f);
			style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.09f, 0.10f, 0.13f, 0.60f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.18f, 0.21f, 0.28f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.24f, 0.28f, 0.36f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = ui_accent_color;
			style.Colors[ImGuiCol_CheckMark]             = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrab]            = ui_accent_color;
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.45f, 0.65f, 1.00f, 1.00f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
			style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.20f, 0.24f, 0.32f, 1.00f);
			style.Colors[ImGuiCol_ButtonActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Header]                = ImVec4(0.16f, 0.19f, 0.26f, 1.00f);
			style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.22f, 0.26f, 0.35f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive]          = ui_accent_color;
			style.Colors[ImGuiCol_Separator]             = ImVec4(0.16f, 0.18f, 0.23f, 1.00f);
			style.Colors[ImGuiCol_SeparatorHovered]      = ui_accent_color;
			style.Colors[ImGuiCol_SeparatorActive]       = ui_accent_color;
			break;
		}
	}

	static void BeginGroupbox(const char* title, float height = 0.0f)
	{
		ImGui::PushID(title);
		ImGuiChildFlags flags = ImGuiChildFlags_Borders;
		if (height <= 0.0f)
			flags |= ImGuiChildFlags_AutoResizeY;
		ImGui::BeginChild("##grpbox", ImVec2(0, height), flags);
		ImGui::TextColored(ui_accent_color, "%s", title);
		ImGui::Separator();
		ImGui::Spacing();
	}

	static void EndGroupbox()
	{
		ImGui::EndChild();
		ImGui::Spacing();
		ImGui::PopID();
	}

	static void RenderRagebotTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// Column 1: Aimbot Logic & Targeting
		ImGui::BeginChild("RageCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("MAIN");
			AnimatedSwitch("Enable Ragebot##rage_main", &sets->rage.enabled);
			AnimatedSwitch("Auto Knife (Knifebot)##rage_kb", &sets->legit.knifebot);
			AnimatedSwitch("Silent Aim##rage_silent", &sets->rage.silent);
			AnimatedSwitch("Automatic Fire##rage_autoshoot", &sets->rage.autoshoot);
			AnimatedSwitch("Aim Through Walls##rage_autowall", &sets->rage.autowall);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Field of View##rage_fov", &sets->rage.hitchance, 0.0f, 180.0f, "%.1f°");
			EndGroupbox();

			BeginGroupbox("SELECTION");
			const char* hit_modes[] = { "Hit Chance", "Damage", "Distance" };
			static int sel_hit = 0;
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Target##rage_target", &sel_hit, hit_modes, IM_ARRAYSIZE(hit_modes));
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Target Selection & Resolver
		ImGui::BeginChild("RageCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("OTHER");
			const char* hist_modes[] = { "Low", "Medium", "High" };
			static int sel_hist = 0;
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("History##rage_hist", &sel_hist, hist_modes, IM_ARRAYSIZE(hist_modes));
			AnimatedSwitch("Delay Shot##rage_delay", &sets->rage.autostop);
			AnimatedSwitch("Remove Recoil##rage_rcs", &sets->rage.silent);
			AnimatedSwitch("Remove Spread##rage_nospread", &sets->rage.autoscope);
			AnimatedSwitch("Duck Peek Assist##rage_fakeduck", &sets->misc.fake_duck);
			AnimatedSwitch("Quick Peek Assist##rage_slowwalk", &sets->misc.slow_walk);
			AnimatedSwitch("Double Tap##rage_dt", &sets->rage.magic_bullet);
			EndGroupbox();

			BeginGroupbox("ANTI-AIM");
			AnimatedSwitch("Enable Anti-Aim##rage_aa_enable", &sets->rage.spinbot);
			const char* pitch_modes[] = { "Off", "Down", "Up", "Zero" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Pitch##rage_pitch", &sets->rage.pitch_aa, pitch_modes, IM_ARRAYSIZE(pitch_modes));
			EndGroupbox();
		}
		ImGui::EndChild();
	}

	static void RenderAntiAimTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// Column 1: Pitch & Yaw Angles
		ImGui::BeginChild("AntiAimCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("SPINBOT & ANGLES");
			AnimatedSwitch("Enable Spinbot", &sets->rage.spinbot);
			
			const char* pitch_modes[] = { "Off", "Down (Emotion)", "Up (Fakeping)", "Zero / Untrusted" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Pitch Angle", &sets->rage.pitch_aa, pitch_modes, IM_ARRAYSIZE(pitch_modes));

			const char* yaw_modes[] = { "Off", "Backwards (180°)", "Spinbot", "Jitter", "Sideways" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Yaw Angle", &sets->rage.yaw_aa, yaw_modes, IM_ARRAYSIZE(yaw_modes));

			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Spin Speed", &sets->rage.spin_speed, 1.0f, 100.0f, "%.0f deg/s");
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Desync & Fake Lag
		ImGui::BeginChild("AntiAimCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("DESYNC & FAKE LAG");
			AnimatedSwitch("Enable FakeLag Engine", &sets->misc.fakelag_enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("FakeLag Limit", &sets->misc.fakelag_limit, 1, 16, "%d ticks");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("FakeLag Variance", &sets->misc.fakelag_random, 0, 8, "%d ticks");
			AnimatedSwitch("Fake Duck Assist", &sets->misc.fake_duck);
			EndGroupbox();
		}
		ImGui::EndChild();
	}

	static void RenderLegitbotTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// Column 1: Aimbot & Recoil Control (RCS)
		ImGui::BeginChild("LegitCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Aim Assistance & Trigger");
			AnimatedSwitch("Enable Legitbot", &sets->legit.enabled);
			AnimatedSwitch("Auto Knife (Knifebot)", &sets->legit.knifebot);
			AnimatedSwitch("Automatic Fire (Autopistol)", &sets->misc.autopistol);
			AnimatedSwitch("Triggerbot", &sets->legit.trigger._enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Trigger Delay", &sets->legit.trigger.delay, 0.0f, 1.0f, "%.2f s");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Aimbot Field of View", &sets->legit.aim.fov, 0.0f, 30.0f, "%.1f°");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Smooth Amount", &sets->legit.aim.smooth[0], 0.0f, 100.0f, "%.1f");
			EndGroupbox();

			BeginGroupbox("Recoil Control System (RCS)");
			AnimatedSwitch("Enable RCS", &sets->legit.aim.enable_rcs);
			AnimatedSwitch("Standalone RCS", &sets->legit.aim.standalone_rcs);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("RCS Pitch Mult", &sets->legit.aim.rcs[0], 0.0f, 2.0f, "%.2fx");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("RCS Yaw Mult", &sets->legit.aim.rcs[1], 0.0f, 2.0f, "%.2fx");
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Backtrack & Hitbox Filters
		ImGui::BeginChild("LegitCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Backtrack Engine");
			AnimatedSwitch("Enable Backtrack", &sets->legit.backtrack.enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("Backtrack Ticks", &sets->legit.backtrack.ticks, 1, 14, "%d ticks");
			AnimatedSwitch("Draw Backtrack Records", &sets->legit.backtrack.style[0]);
			EndGroupbox();

			BeginGroupbox("Target Hitbox Filter");
			AnimatedSwitch("Head Hitbox", &sets->legit.aim.hitbox[0]);
			AnimatedSwitch("Chest Hitbox", &sets->legit.aim.hitbox[1]);
			AnimatedSwitch("Pelvis Hitbox", &sets->legit.aim.hitbox[2]);
			AnimatedSwitch("Arms Hitbox", &sets->legit.aim.hitbox[3]);
			AnimatedSwitch("Legs Hitbox", &sets->legit.aim.hitbox[4]);
			EndGroupbox();
		}
		ImGui::EndChild();
	}

	static void RenderPlayersEspTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// =========================================================================
		// CỘT 1: PLAYER INFO (BARS, FLAGS, CHAMS, ASIAN HAT)
		// =========================================================================
		ImGui::BeginChild("EspCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Player Info");
			AnimatedSwitch("Enable Visuals Engine", &sets->visuals.enabled);
			AnimatedSwitch("Draw Teammates", &sets->visuals.friends);
			AnimatedSwitch("Player Names", &sets->visuals.esp_show[0]);
			AnimatedSwitch("2D Bounding Box", &sets->visuals.esp_show[1]);
			AnimatedSwitch("Skeleton ESP (Bones)", &sets->visuals.skeleton);
			AnimatedSwitch("Health Bar", &sets->visuals.esp_bar[0]);
			AnimatedSwitch("Armor Bar", &sets->visuals.esp_bar[1]);
			AnimatedSwitch("Ammo Bar", &sets->visuals.esp_bar[2]);
			AnimatedSwitch("Active Weapon Text", &sets->visuals.esp_show[2]);
			AnimatedSwitch("Snaplines to Enemy", &sets->visuals.esp_show[3]);
			EndGroupbox();

			BeginGroupbox("Player Flags");
			AnimatedSwitch("Flag [HK] (Helmet/Kevlar)", &sets->visuals.flag_hk);
			AnimatedSwitch("Flag [SCOPED]", &sets->visuals.flag_scoped);
			AnimatedSwitch("Flag [RELOADING]", &sets->visuals.flag_reloading);
			AnimatedSwitch("Flag [FLASHED]", &sets->visuals.flag_flashed);
			EndGroupbox();

			BeginGroupbox("World & Removals");
			AnimatedSwitch("Remove Smoke (No Smoke)", &sets->visuals.remove[0]);
			AnimatedSwitch("Remove Flash (No Flash)", &sets->visuals.remove[1]);
			AnimatedSwitch("Grenades ESP", &sets->visuals.esp_filter[2]);
			AnimatedSwitch("C4 Bomb ESP", &sets->visuals.esp_filter[3]);
			EndGroupbox();

			BeginGroupbox("Player Chams");
			const char* chams_modes[] = { "Disabled", "Flat Colored", "Material Shaded", "Wireframe" };
			ImGui::SetNextItemWidth(160);
			ImGui::Combo("Chams Mode", &sets->visuals.chams, chams_modes, IM_ARRAYSIZE(chams_modes));
			ColorEdit3Custom("Terrorist (T) Color", sets->visuals.chams_t);
			ColorEdit3Custom("Counter-Terrorist (CT) Color", sets->visuals.chams_ct);
			EndGroupbox();

			BeginGroupbox("Custom 3D Player Model");
			AnimatedSwitch("Add / Apply Custom 3D Model", &sets->visuals.enable_custom_model);
			if (sets->visuals.enable_custom_model)
			{
				AnimatedSwitch("Local Player Only (Only You)", &sets->visuals.custom_model_local_only);
			}
			const char* custom_models[] = {
				"Phoenix Terrorist (T)",
				"Leet Krew (T)",
				"SAS Gasmask (CT)",
				"GIGN SWAT (CT)",
				"Hostage (Scientist)",
				"Cissia ZZZ (Zenless Zone Zero)"
			};
			ImGui::SetNextItemWidth(180);
			ImGui::Combo("Character Model", &sets->visuals.model_selection, custom_models, IM_ARRAYSIZE(custom_models));
			EndGroupbox();

			BeginGroupbox("3D Asian Rice Hat");
			AnimatedSwitch("Enable Asian Rice Hat", &sets->visuals.asian_hat);
			ColorEdit3Custom("Hat Color", sets->visuals.asian_hat_color);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Hat Radius Size", &sets->visuals.asian_hat_size, 10.0f, 40.0f, "%.1f");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Hat Cone Height", &sets->visuals.asian_hat_height, 2.0f, 25.0f, "%.1f");
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Advanced & Extra
		ImGui::BeginChild("EspCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Advanced & Extra");
			AnimatedSwitch("OOF Arrows (Offscreen ESP)", &sets->visuals.offscreen_esp);
			if (sets->visuals.offscreen_esp)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Arrow Size", &sets->visuals.oof_size, 5.0f, 30.0f, "%.0f px");
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Screen Radius", &sets->visuals.oof_radius, 40.0f, 300.0f, "%.0f px");
			}
			AnimatedSwitch("Footstep Rings", &sets->visuals.footstep_rings);
			AnimatedSwitch("Footstep Sound ESP", &sets->visuals.sound_esp);
			AnimatedSwitch("Hitmarker FX", &sets->visuals.hitmarker);
			AnimatedSwitch("Nightmode (Dark Map)", &sets->visuals.nightmode);
			if (sets->visuals.nightmode)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Wall Opacity", &sets->visuals.asus_walls, 0.0f, 100.0f, "%.0f%%");
			}
			EndGroupbox();

			BeginGroupbox("Camera & Trail FX");
			AnimatedSwitch("Thirdperson (No sv_cheats)", &sets->visuals.thirdperson);
			if (sets->visuals.thirdperson)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Camera Distance", &sets->visuals.thirdperson_dist, 30.0f, 300.0f, "%.0f u");
				AnimatedSwitch("Reverse Angle (Look Back)", &sets->visuals.thirdperson_reverse);
			}
			AnimatedSwitch("Rainbow Trail", &sets->visuals.rainbow_trail);
			if (sets->visuals.rainbow_trail)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Trail Speed", &sets->visuals.rainbow_trail_speed, 0.2f, 3.0f, "%.1fx");
			}
			EndGroupbox();

			// 3D Live Interactive Model Preview
			RenderEspLivePreview(ImVec2(col_w, 360), 0);
		}
		ImGui::EndChild();
	}

	static void RenderMiscExploitsTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// Column 1: Movement & Character Exploits
		ImGui::BeginChild("MiscCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Movement System");
			AnimatedSwitch("Bunnyhop (Auto-Jump)", &sets->misc.autojump);
			AnimatedSwitch("Auto Strafer", &sets->misc.autostrafer);
			AnimatedSwitch("Auto Pistol", &sets->misc.autopistol);
			AnimatedSwitch("Slowwalk", &sets->misc.slow_walk);
			AnimatedSwitch("Fake Duck", &sets->misc.fake_duck);
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Network & Server Bypasses
		ImGui::BeginChild("MiscCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Network & Latency");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("Fake Ping Latency", &sets->misc.fake_ping, 0, 500, "%d ms");
			EndGroupbox();

			BeginGroupbox("Server Enforcement Bypasses");
			AnimatedSwitch("SV_Pure 1/2 Bypass", &sets->misc.pure_bypass);
			AnimatedSwitch("Anti-SMAC Server Bypass", &sets->misc.antismac);
			EndGroupbox();
		}
		ImGui::EndChild();
	}

	static void RenderSettingsTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// Column 1: Structural UI Layout Presets (24 LAYOUTS: Phobia, Neverlose, Gamesense, & Releases 1-21)
		ImGui::BeginChild("SettingsCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Structural UI Architecture");
			const char* layout_names[] = {
				"1. Gamesense (Skeet Classic)",
				"2. Neverlose (Official HUD)",
				"3. [Release 16] Imgui Aternos (3D Skeleton Visualizer)",
				"4. [Release 19] Synthetic (Space Galaxy Honeycomb)"
			};
			ImGui::SetNextItemWidth(230);
			if (ImGui::Combo("UI Architecture", &current_layout, layout_names, IM_ARRAYSIZE(layout_names)))
			{
				if (current_layout == LAYOUT_GAMESENSE)
					ApplyTheme(THEME_SKEET);
				else if (current_layout == LAYOUT_NEVERLOSE)
					ApplyTheme(THEME_NEVERLOSE);
				else if (current_layout == LAYOUT_ATERNOS)
					ApplyTheme(THEME_ONYX);
				else
					ApplyTheme(THEME_CYBERPUNK);
			}
			ImGui::Spacing();
			ImGui::TextWrapped("Select between Gamesense, Neverlose, Aternos 3D, and Synthetic Honeycomb!");
			EndGroupbox();

			BeginGroupbox("Color Theme Presets");
			const char* theme_names[] = {
				"Gamesense (Skeet Emerald)",
				"Cyberpunk (Neon Pink)",
				"Onyx Stealth (Crimson OLED)",
				"Neverlose (Electric Cyan)"
			};
			ImGui::SetNextItemWidth(190);
			if (ImGui::Combo("Color Theme", &current_theme, theme_names, IM_ARRAYSIZE(theme_names)))
			{
				ApplyTheme(current_theme);
			}
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Layout Performance & Profiles
		ImGui::BeginChild("SettingsCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Menu Toggle Key");
			char key_btn_text[64];
			if (is_binding_key)
			{
				snprintf(key_btn_text, sizeof(key_btn_text), "[ Press any key... ]");
				for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
				{
					if (ImGui::IsKeyPressed((ImGuiKey)k, false))
					{
						if (k != ImGuiKey_Escape)
						{
							int vk = 0x2D;
							if (k >= ImGuiKey_F1 && k <= ImGuiKey_F12) vk = 0x70 + (k - ImGuiKey_F1);
							else if (k >= ImGuiKey_0 && k <= ImGuiKey_9) vk = 0x30 + (k - ImGuiKey_0);
							else if (k >= ImGuiKey_A && k <= ImGuiKey_Z) vk = 0x41 + (k - ImGuiKey_A);
							else if (k == ImGuiKey_Insert) vk = 0x2D;
							else if (k == ImGuiKey_Delete) vk = 0x2E;
							else if (k == ImGuiKey_Home) vk = 0x24;
							else if (k == ImGuiKey_End) vk = 0x23;
							else if (k == ImGuiKey_PageUp) vk = 0x21;
							else if (k == ImGuiKey_PageDown) vk = 0x22;
							else if (k == ImGuiKey_GraveAccent) vk = 0xC0;
							else if (k == ImGuiKey_RightShift) vk = 0xA1;
							else if (k == ImGuiKey_LeftShift) vk = 0xA0;
							else if (k == ImGuiKey_RightCtrl) vk = 0xA3;
							else if (k == ImGuiKey_LeftCtrl) vk = 0xA2;
							else if (k == ImGuiKey_Tab) vk = 0x09;
							else vk = k; // fallback
							sets->menu.menu_key = vk;
						}
						is_binding_key = false;
						break;
					}
				}
			}
			else
			{
				const char* kname = "INSERT";
				int vk = sets->menu.menu_key;
				switch (vk)
				{
				case 0x2D: kname = "INSERT"; break;
				case 0x2E: kname = "DELETE"; break;
				case 0x24: kname = "HOME"; break;
				case 0x23: kname = "END"; break;
				case 0x21: kname = "PAGE UP"; break;
				case 0x22: kname = "PAGE DOWN"; break;
				case 0x70: kname = "F1"; break;
				case 0x71: kname = "F2"; break;
				case 0x72: kname = "F3"; break;
				case 0x73: kname = "F4"; break;
				case 0x74: kname = "F5"; break;
				case 0x75: kname = "F6"; break;
				case 0x76: kname = "F7"; break;
				case 0x77: kname = "F8"; break;
				case 0x78: kname = "F9"; break;
				case 0x79: kname = "F10"; break;
				case 0x7A: kname = "F11"; break;
				case 0x7B: kname = "F12"; break;
				case 0xC0: kname = "TILDE (~)"; break;
				case 0xA1: kname = "RIGHT SHIFT"; break;
				case 0xA0: kname = "LEFT SHIFT"; break;
				case 0xA3: kname = "RIGHT CTRL"; break;
				case 0xA2: kname = "LEFT CTRL"; break;
				case 0x09: kname = "TAB"; break;
				default:
					if (vk >= 0x41 && vk <= 0x5A) { static char b[2] = {0,0}; b[0] = (char)vk; kname = b; }
					else if (vk >= 0x30 && vk <= 0x39) { static char b[2] = {0,0}; b[0] = (char)vk; kname = b; }
					else if (vk >= ImGuiKey_NamedKey_BEGIN && vk < ImGuiKey_NamedKey_END) {
						const char* n = ImGui::GetKeyName((ImGuiKey)vk);
						if (n && strlen(n) > 0) kname = n;
					}
					break;
				}
				snprintf(key_btn_text, sizeof(key_btn_text), "[ %s ]", kname);
			}

			if (ImGui::Button(key_btn_text, ImVec2(170, 30)))
			{
				is_binding_key = !is_binding_key;
			}
			ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Click button above & press any key");
			EndGroupbox();

			BeginGroupbox("Animation & Performance");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("UI Animation Speed", &ui_anim_speed, 0.5f, 3.0f, "%.1fx");
			EndGroupbox();

			BeginGroupbox("Config Profile Management");
			if (ImGui::Button(" Save Config Profile ", ImVec2(150, 32)))
			{
				configs::write("global_profile");
				ImGui::OpenPopup("PopupConfigSavedSettings");
			}
			if (ImGui::BeginPopup("PopupConfigSavedSettings"))
			{
				ImGui::Text("Config profile saved successfully!");
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button(" Load Config Profile ", ImVec2(150, 32)))
			{
				configs::parse("global_profile");
			}
			EndGroupbox();
		}
		ImGui::EndChild();
	}

	// Helper to render active tab content
	static void RenderCurrentTabContent(int tab_id, float alpha = 1.0f)
	{
		switch (tab_id)
		{
		case 0: RenderRagebotTab(alpha); break;
		case 1: RenderAntiAimTab(alpha); break;
		case 2: RenderLegitbotTab(alpha); break;
		case 3: RenderPlayersEspTab(alpha); break;
		case 4: RenderMiscExploitsTab(alpha); break;
		case 5: default: RenderSettingsTab(alpha); break;
		}
	}

	// =========================================================================
	// 4 STRUCTURAL LAYOUT RENDERERS (EXACT TO SCREENSHOTS 1, 2, 3, 4, 5)
	// =========================================================================

	// LAYOUT 1: GAMESENSE / SKEET CLASSIC (SCREENSHOT 3)
	static void RenderSkeetLayout()
	{
		ImGui::SetNextWindowPos(ImVec2(100, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(860, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin("LOVEMACHINE CS:S - Gamesense Skeet", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float win_w = ImGui::GetWindowWidth();

		// Top Rainbow Accent Line (Red -> Yellow -> Green -> Cyan -> Blue -> Purple)
		draw->AddRectFilledMultiColor(pos, ImVec2(pos.x + win_w * 0.5f, pos.y + 3.0f),
			IM_COL32(255, 50, 50, 255), IM_COL32(255, 220, 0, 255),
			IM_COL32(160, 220, 20, 255), IM_COL32(0, 210, 255, 255));
		draw->AddRectFilledMultiColor(ImVec2(pos.x + win_w * 0.5f, pos.y), ImVec2(pos.x + win_w, pos.y + 3.0f),
			IM_COL32(0, 210, 255, 255), IM_COL32(140, 60, 255, 255),
			IM_COL32(255, 50, 150, 255), IM_COL32(255, 50, 50, 255));

		// Left Vertical Icon Strip Column (SenseUI / Gamesense Standard)
		ImGui::SetCursorPos(ImVec2(8, 12));
		ImGui::BeginChild("SkeetIconSidebar", ImVec2(58, 0), true);
		struct IconTabMap { const char* icon_char; const char* fallback; int tab_id; };
		const IconTabMap skeet_icons[] = {
			{ "C", "(R)", 0 }, // Ragebot (Gun icon in AstriumTabs)
			{ "I", "(S)", 1 }, // Anti-Aim / Spinbot (Spinner icon)
			{ "D", "(L)", 2 }, // Legitbot (Target crosshair)
			{ "E", "(V)", 3 }, // Visuals (Eye icon)
			{ "G", "(M)", 4 }, // Misc (Module/Toolbox icon)
			{ "F", "(S)", 5 }  // Settings (Official Gear icon in AstriumTabs)
		};

		for (int i = 0; i < IM_ARRAYSIZE(skeet_icons); i++)
		{
			bool is_sel = (current_tab == skeet_icons[i].tab_id);
			if (is_sel)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ui_accent_color.x * 0.25f, ui_accent_color.y * 0.25f, ui_accent_color.z * 0.25f, 0.90f));
				ImGui::PushStyleColor(ImGuiCol_Text, ui_accent_color);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.08f, 0.70f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
			}

			if (font_skeet_icons) ImGui::PushFont(font_skeet_icons);
			const char* label = font_skeet_icons ? skeet_icons[i].icon_char : skeet_icons[i].fallback;
			if (ImGui::Button(label, ImVec2(42, 38)))
			{
				current_tab = skeet_icons[i].tab_id;
			}
			if (font_skeet_icons) ImGui::PopFont();

			ImGui::PopStyleColor(2);
			ImGui::Spacing();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Main Content Area
		ImGui::BeginChild("SkeetMainBody", ImVec2(0, 0), true);
		RenderCurrentTabContent(current_tab, 1.0f);
		ImGui::EndChild();

		ImGui::End();
	}

	// =========================================================================
	// AUTHENTIC NEVERLOSE 2.0 / 3.0 UI ENGINE (FULLY FUNCTIONAL & RESPONSIVE)
	// =========================================================================
	struct NL_Color {
		float r, g, b, a;
		ImColor to_im_color(float alpha = 1.f) const {
			return ImColor(r, g, b, (a * ImGui::GetStyle().Alpha) * alpha);
		}
		ImVec4 to_vec4(float alpha = 1.f) const {
			return ImVec4(r, g, b, (a * ImGui::GetStyle().Alpha) * alpha);
		}
	};

	struct NL_Gui {
		float m_anim = 1.0f;
		int m_preset = 0;

		NL_Color accent_color = { 0.00f, 0.82f, 1.00f, 1.00f }; // Neverlose Cyan #00D2FF
		NL_Color text = { 1.00f, 1.00f, 1.00f, 1.00f };
		NL_Color text_disabled = { 0.51f, 0.52f, 0.56f, 1.00f };
		NL_Color border = { 1.00f, 1.00f, 1.00f, 0.08f };
		NL_Color frame_inactive = { 0.023f, 0.039f, 0.070f, 1.00f };
		NL_Color frame_active = { 0.043f, 0.070f, 0.137f, 1.00f };
		NL_Color button = { 0.031f, 0.035f, 0.058f, 1.00f };
		NL_Color group_box_bg = { 0.019f, 0.035f, 0.062f, 1.00f };

		void group_title(const char* name) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.82f, 1.00f, 0.70f));
			ImGui::Text("%s", name);
			ImGui::PopStyleColor();
		}

		bool tab(const char* icon, const char* label, bool selected) {
			auto window = ImGui::GetCurrentWindow();
			auto id = window->GetID(label);
			auto label_size = ImGui::CalcTextSize(label, 0, true);
			auto pos = window->DC.CursorPos;
			auto draw = window->DrawList;

			ImRect bb(pos, ImVec2(pos.x + ImGui::GetWindowWidth() - 14, pos.y + 34));
			ImGui::ItemAdd(bb, id);
			ImGui::ItemSize(bb, ImGui::GetStyle().FramePadding.y);

			bool hovered, held;
			bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

			static std::unordered_map<ImGuiID, float> values;
			auto it = values.find(id);
			if (it == values.end()) {
				values.insert({ id, 0.f });
				it = values.find(id);
			}
			it->second = ImLerp(it->second, (selected ? 1.f : (hovered ? 0.35f : 0.f)), 0.12f);

			if (it->second > 0.01f) {
				draw->AddRectFilled(bb.Min, bb.Max, frame_active.to_im_color(0.85f * it->second), 5);
				if (selected) {
					draw->AddRectFilled(bb.Min, ImVec2(bb.Min.x + 3.5f, bb.Max.y), accent_color.to_im_color(), 2);
				}
			}

			draw->AddText(ImVec2(bb.Min.x + 10, bb.GetCenter().y - label_size.y * 0.5f), accent_color.to_im_color(selected ? 1.0f : 0.70f), icon);
			draw->AddText(ImVec2(bb.Min.x + 36, bb.GetCenter().y - label_size.y * 0.5f), selected ? text.to_im_color() : text_disabled.to_im_color(), label);

			return pressed;
		}

		bool subtab(const char* label, bool selected, int count, ImDrawFlags flags = 0) {
			auto window = ImGui::GetCurrentWindow();
			auto id = window->GetID(label);
			auto label_size = ImGui::CalcTextSize(label, 0, true);
			auto pos = window->DC.CursorPos;
			auto draw = window->DrawList;

			ImRect bb(pos, ImVec2(pos.x + ImGui::GetWindowWidth() / (float)count, pos.y + ImGui::GetWindowHeight()));
			ImGui::ItemAdd(bb, id);
			ImGui::ItemSize(bb, ImGui::GetStyle().FramePadding.y);

			bool hovered, held;
			bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

			static std::unordered_map<ImGuiID, float> values;
			auto it = values.find(id);
			if (it == values.end()) {
				values.insert({ id, 0.f });
				it = values.find(id);
			}
			it->second = ImLerp(it->second, (selected ? 1.f : (hovered ? 0.35f : 0.f)), 0.12f);

			if (it->second > 0.01f) {
				draw->AddRectFilled(bb.Min, bb.Max, frame_active.to_im_color(0.95f * it->second), 4, flags);
			}

			draw->AddText(ImVec2(bb.GetCenter().x - label_size.x * 0.5f, bb.GetCenter().y - label_size.y * 0.5f), selected ? accent_color.to_im_color() : text_disabled.to_im_color(), label);
			return pressed;
		}
	};
	static NL_Gui nl_gui;

	// LAYOUT 1: NEVERLOSE 2.0 / 3.0 (AUTHENTIC)
	static void RenderNeverloseLayout()
	{
		ImGui::SetNextWindowPos(ImVec2(100, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(940, 660), ImGuiCond_FirstUseEver);
		ImGui::Begin("LOVEMACHINE CS:S - Neverlose 2.0", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		auto window = ImGui::GetCurrentWindow();
		auto draw = window->DrawList;
		auto pos = window->Pos;
		auto size = window->Size;

		nl_gui.m_anim = ImLerp(nl_gui.m_anim, 1.0f, 0.08f);

		// 1. Left Sidebar Navigation (Neverlose-last styling)
		ImGui::SetCursorPos(ImVec2(10, 10));
		ImGui::BeginChild("LeftSidebarNL", ImVec2(175, size.y - 20), true);

		// Neverlose Logo with glow and Museo900 font
		draw->AddText(font_brand_title ? font_brand_title : ImGui::GetFont(), 20.0f, ImVec2(pos.x + 24, pos.y + 14), nl_gui.accent_color.to_im_color(), "NEVERLOSE");
		draw->AddText(font_brand_title ? font_brand_title : ImGui::GetFont(), 20.0f, ImVec2(pos.x + 23, pos.y + 14), ImGui::GetColorU32(ImGuiCol_Text), "NEVERLOSE");

		ImGui::SetCursorPos(ImVec2(6, 45));
		ImGui::BeginChild("##nltabs", ImVec2(163, size.y - 120), false);

		nl_gui.group_title("AIMBOT");
		if (nl_gui.tab(ICON_FA_CROSSHAIRS, "Ragebot", current_tab == 0)) { current_tab = 0; nl_gui.m_anim = 0.f; }
		if (nl_gui.tab(ICON_FA_GHOST, "Anti-Aim", current_tab == 1)) { current_tab = 1; nl_gui.m_anim = 0.f; }
		if (nl_gui.tab(ICON_FA_MOUSE, "Legitbot", current_tab == 2)) { current_tab = 2; nl_gui.m_anim = 0.f; }

		ImGui::Spacing(); ImGui::Spacing();
		nl_gui.group_title("VISUALS");
		if (nl_gui.tab(ICON_FA_USER, "Players ESP", current_tab == 3)) { current_tab = 3; nl_gui.m_anim = 0.f; }

		ImGui::Spacing(); ImGui::Spacing();
		nl_gui.group_title("MISCELLANEOUS");
		if (nl_gui.tab(ICON_FA_HAMMER, "Main Exploits", current_tab == 4)) { current_tab = 4; nl_gui.m_anim = 0.f; }
		if (nl_gui.tab(ICON_FA_COG, "Settings", current_tab == 5)) { current_tab = 5; nl_gui.m_anim = 0.f; }

		ImGui::EndChild();

		// User Profile Banner at bottom (Neverlose-last)
		ImVec2 prof_pos = ImVec2(pos.x + 10, pos.y + size.y - 52);
		draw->AddLine(prof_pos, ImVec2(prof_pos.x + 175, prof_pos.y), nl_gui.border.to_im_color(0.5f));
		draw->AddCircleFilled(ImVec2(prof_pos.x + 22, prof_pos.y + 26), 14.0f, nl_gui.frame_active.to_im_color());
		draw->AddCircleFilled(ImVec2(prof_pos.x + 32, prof_pos.y + 34), 4.0f, IM_COL32(30, 220, 100, 255)); // Online indicator
		draw->AddText(ImVec2(prof_pos.x + 44, prof_pos.y + 16), nl_gui.text.to_im_color(), "MinhYz");
		draw->AddText(ImVec2(prof_pos.x + 44, prof_pos.y + 30), nl_gui.text_disabled.to_im_color(), "Till: ");
		draw->AddText(ImVec2(prof_pos.x + 44 + ImGui::CalcTextSize("Till: ").x, prof_pos.y + 30), nl_gui.accent_color.to_im_color(), "Lifetime");

		ImGui::EndChild();

		ImGui::SameLine();

		// 2. Right Main Panel
		ImGui::BeginChild("NeverloseMainBody", ImVec2(0, 0), true);

		// Top Subtabs Bar & Presets
		ImGui::SetCursorPos(ImVec2(12, 10));
		static bool config_saved_notify = false;
		static float notify_timer = 0.0f;
		if (ImGui::Button(ICON_FA_SAVE " Save Config", ImVec2(115, 26)))
		{
			configs::write("neverlose_profile");
			config_saved_notify = true;
			notify_timer = ImGui::GetTime() + 3.0f;
		}
		if (config_saved_notify && ImGui::GetTime() < notify_timer)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), ICON_FA_CHECK " Saved!");
		}

		ImGui::SameLine();

		// Responsive Dynamic Subtabs based on active domain
		struct SubtabDef { const char* name; int target_tab; };
		std::vector<SubtabDef> active_subtabs;
		if (current_tab <= 2) // Aimbot Domain
		{
			active_subtabs = { { "Ragebot", 0 }, { "Anti-Aim", 1 }, { "Legitbot", 2 } };
		}
		else if (current_tab == 3) // Visuals Domain
		{
			active_subtabs = { { "ESP", 3 }, { "Chams", 3 }, { "World", 3 } };
		}
		else // Misc / Settings Domain
		{
			active_subtabs = { { "Exploits", 4 }, { "Movement", 4 }, { "Settings", 5 } };
		}

		ImGui::BeginChild("##nl_subtabs", ImVec2(290, 26), false);
		ImVec2 sub_pos = ImGui::GetWindowPos();
		ImVec2 sub_sz = ImGui::GetWindowSize();
		draw->AddRectFilled(sub_pos, ImVec2(sub_pos.x + sub_sz.x, sub_pos.y + sub_sz.y), nl_gui.button.to_im_color(), 4);
		draw->AddRect(sub_pos, ImVec2(sub_pos.x + sub_sz.x, sub_pos.y + sub_sz.y), nl_gui.border.to_im_color(), 4);

		for (int i = 0; i < (int)active_subtabs.size(); i++) {
			bool is_active_sub = (current_tab == active_subtabs[i].target_tab);
			if (nl_gui.subtab(active_subtabs[i].name, is_active_sub, (int)active_subtabs.size(),
				i == 0 ? ImDrawFlags_RoundCornersLeft : i == (int)active_subtabs.size() - 1 ? ImDrawFlags_RoundCornersRight : 0))
			{
				current_tab = active_subtabs[i].target_tab;
				nl_gui.m_anim = 0.f;
			}
			if (i < (int)active_subtabs.size() - 1) ImGui::SameLine();
		}
		ImGui::EndChild();

		// Interactive Preset Combo Selector
		ImGui::SameLine(ImGui::GetWindowWidth() - 280);
		const char* nl_presets[] = {
			"1. Balance HvH (Optimal)",
			"2. Legit Aimbot (Closet)",
			"3. Full Rage HvH (Aggressive)",
			"4. Scout & AWP God"
		};
		ImGui::SetNextItemWidth(265);
		if (ImGui::Combo("##nl_preset_sel", &nl_gui.m_preset, nl_presets, IM_ARRAYSIZE(nl_presets)))
		{
			// Instant complete preset configuration setup
			if (nl_gui.m_preset == 0) // 1. Balance HvH (Neverlose)
			{
				sets->legit.enabled = false;
				sets->rage.autoshoot = true;
				sets->rage.autoscope = true;
				sets->rage.autostop = true;
				sets->rage.silent = true;
				sets->rage.autowall = true;
				sets->rage.hitchance = 78.0f;
				sets->rage.min_damage_visible = 40.0f;
				sets->rage.min_damage_autowall = 28.0f;
				sets->rage.hitbox[0] = true; sets->rage.hitbox[1] = true; sets->rage.hitbox[2] = true; sets->rage.hitbox[3] = true;
				sets->rage.body_aim_mode = 1; // Prefer body aim
				sets->rage.spinbot = true;
				sets->rage.spinbot_mode = 1;
				sets->rage.spin_speed = 35.0f;
				sets->rage.pitch_aa = 1;
				sets->rage.yaw_aa = 2;
				sets->legit.backtrack.enabled = true;
				sets->legit.backtrack.ticks = 12;
				sets->visuals.enabled = true;
				sets->visuals.skeleton = true;
				sets->visuals.chams = 2; // Material shaded chams
				sets->visuals.nightmode = true;
				sets->misc.autojump = true;
				sets->misc.autostrafer = true;
			}
			else if (nl_gui.m_preset == 1) // 2. Legit Aimbot (Closet)
			{
				sets->rage.autoshoot = false;
				sets->rage.silent = false;
				sets->rage.spinbot = false;
				sets->rage.autowall = false;
				sets->rage.autostop = false;
				sets->legit.enabled = true;
				sets->legit.aim.fov = 2.8f;
				sets->legit.aim.smooth[0] = 32.0f; sets->legit.aim.smooth[1] = 36.0f;
				sets->legit.aim.rcs[0] = 2.0f; sets->legit.aim.rcs[1] = 2.0f;
				sets->legit.aim.enable_rcs = true;
				sets->legit.aim.humanize[0] = 1.8f; sets->legit.aim.humanize[1] = 1.4f;
				sets->legit.aim.hitbox[0] = true; sets->legit.aim.hitbox[1] = true; sets->legit.aim.hitbox[2] = true;
				sets->legit.backtrack.enabled = true;
				sets->legit.backtrack.ticks = 8;
				sets->visuals.enabled = true;
				sets->visuals.chams = 1; // Flat clean chams
				sets->visuals.nightmode = false;
				sets->misc.autojump = true;
				sets->misc.autostrafer = false;
			}
			else if (nl_gui.m_preset == 2) // 3. Full Rage HvH
			{
				sets->legit.enabled = false;
				sets->rage.autoshoot = true;
				sets->rage.autoscope = true;
				sets->rage.autostop = true;
				sets->rage.silent = true;
				sets->rage.autowall = true;
				sets->rage.hitchance = 88.0f;
				sets->rage.min_damage_visible = 90.0f;
				sets->rage.min_damage_autowall = 55.0f;
				sets->rage.body_aim_mode = 2; // Force body aim
				sets->rage.spinbot = true;
				sets->rage.spinbot_mode = 1;
				sets->rage.spin_speed = 50.0f;
				sets->visuals.enabled = true;
				sets->visuals.chams = 2;
				sets->visuals.nightmode = true;
			}
			else // 4. Scout & AWP God
			{
				sets->legit.enabled = false;
				sets->rage.autoshoot = true;
				sets->rage.autoscope = true;
				sets->rage.autostop = true;
				sets->rage.silent = true;
				sets->rage.autowall = true;
				sets->rage.hitchance = 94.0f;
				sets->rage.min_damage_visible = 100.0f;
				sets->rage.min_damage_autowall = 85.0f;
				sets->rage.hitbox[0] = true; sets->rage.hitbox[2] = true;
				sets->rage.spinbot = true;
				sets->rage.spin_speed = 40.0f;
				sets->visuals.enabled = true;
				sets->visuals.chams = 3; // Wireframe chams
			}
		}

		ImGui::Separator();
		ImGui::Spacing();

		// Animated Main Tab Content Area
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, nl_gui.m_anim);
		RenderCurrentTabContent(current_tab, nl_gui.m_anim);
		ImGui::PopStyleVar();

		ImGui::EndChild();

		ImGui::End();
	}

	// =========================================================================
	// 21 DYNAMIC CUSTOM LAYOUTS FROM IMGUIMENU-S (RELEASES 1 - 21)
	// =========================================================================
	struct ImGuiReleaseMeta {
		const char* title;
		const char* brand_tag;
		ImVec4 primary_accent;
		ImVec4 bg_color;
		bool is_horizontal_topbar;
	};

	static const ImGuiReleaseMeta imgui_release_styles[] = {
		{ "1.rar: Orange Ember Dark", "EMBER", ImVec4(1.00f, 0.47f, 0.00f, 1.0f), ImVec4(0.09f, 0.08f, 0.07f, 1.0f), true },          // 1.rar
		{ "2.rar: 1011 Cyber Lime", "1011", ImVec4(0.64f, 0.90f, 0.21f, 1.0f), ImVec4(0.08f, 0.08f, 0.09f, 1.0f), false },            // 2.rar
		{ "3.rar: Ev0m Glass Blue", "EV0M", ImVec4(0.22f, 0.74f, 0.97f, 1.0f), ImVec4(0.05f, 0.08f, 0.14f, 0.92f), false },           // 3.rar
		{ "4.rar: Clean Pure White", "LIGHT", ImVec4(0.39f, 0.40f, 0.95f, 1.0f), ImVec4(0.96f, 0.97f, 0.98f, 1.0f), false },          // 4.rar
		{ "5.rar: Byman Crimson Red", "BYMAN.RED", ImVec4(0.94f, 0.27f, 0.27f, 1.0f), ImVec4(0.08f, 0.05f, 0.05f, 1.0f), false },      // 5.rar
		{ "6.rar: Byman Azure Blue", "BYMAN.BLU", ImVec4(0.23f, 0.51f, 0.96f, 1.0f), ImVec4(0.05f, 0.07f, 0.11f, 1.0f), true },        // 6.rar
		{ "7.rar: ImGui Masters Dark", "MASTERS", ImVec4(0.80f, 0.80f, 0.85f, 1.0f), ImVec4(0.08f, 0.08f, 0.09f, 1.0f), false },       // 7.rar
		{ "8.rar: Phobia Rose Pink", "PHOBIA", ImVec4(0.90f, 0.60f, 0.61f, 1.0f), ImVec4(0.10f, 0.10f, 0.10f, 1.0f), false },          // 8.rar
		{ "9.rar: Forest Green & Pink", "FOREST", ImVec4(0.13f, 0.77f, 0.37f, 1.0f), ImVec4(0.06f, 0.09f, 0.07f, 1.0f), true },        // 9.rar
		{ "10.rar: DirectX11 Blue Slate", "DX11.SLATE", ImVec4(0.38f, 0.65f, 0.98f, 1.0f), ImVec4(0.07f, 0.08f, 0.10f, 1.0f), true },  // 10.rar
		{ "11.rar: Pandora Uno Purple", "PANDORA", ImVec4(0.66f, 0.33f, 0.97f, 1.0f), ImVec4(0.07f, 0.05f, 0.10f, 1.0f), false },       // 11.rar
		{ "12.rar: Neverlose V2 Blue", "NEVERLOSE", ImVec4(0.01f, 0.52f, 0.78f, 1.0f), ImVec4(0.03f, 0.05f, 0.09f, 1.0f), false },      // 12.rar
		{ "13.rar: Dark Slate Blue", "SLATE", ImVec4(0.20f, 0.55f, 0.95f, 1.0f), ImVec4(0.08f, 0.09f, 0.12f, 1.0f), false },           // 13.rar
		{ "14.rar: Frameless Modern", "MODERN", ImVec4(0.00f, 0.70f, 1.00f, 1.0f), ImVec4(0.07f, 0.07f, 0.09f, 1.0f), false },          // 14.rar
		{ "15.rar: Squadbounce Model", "SQUAD", ImVec4(0.55f, 0.36f, 0.96f, 1.0f), ImVec4(0.09f, 0.07f, 0.12f, 1.0f), false },          // 15.rar
		{ "16.rar: Aternos 3D Skeleton", "ATERNOS", ImVec4(0.02f, 0.71f, 0.83f, 1.0f), ImVec4(0.06f, 0.07f, 0.10f, 1.0f), false },      // 16.rar
		{ "17.rar: Weave Free Orange", "WEAVE", ImVec4(0.98f, 0.57f, 0.24f, 1.0f), ImVec4(0.08f, 0.08f, 0.08f, 1.0f), true },          // 17.rar
		{ "18.rar: Acidtech Toxic Lime", "ACIDTECH", ImVec4(0.52f, 0.80f, 0.09f, 1.0f), ImVec4(0.07f, 0.09f, 0.06f, 1.0f), true },       // 18.rar
		{ "19.rar: Space Galaxy Stars", "GALAXY", ImVec4(0.39f, 0.40f, 0.95f, 1.0f), ImVec4(0.05f, 0.04f, 0.08f, 1.0f), false },        // 19.rar
		{ "20.rar: Catrine Minimal", "CATRINE", ImVec4(0.22f, 0.74f, 0.97f, 1.0f), ImVec4(0.08f, 0.08f, 0.09f, 1.0f), false },          // 20.rar
		{ "21.rar: Yzs / d0td Hot Pink", "YZS.GLOW", ImVec4(0.96f, 0.25f, 0.37f, 1.0f), ImVec4(0.07f, 0.04f, 0.09f, 1.0f), false }     // 21.rar
	};

	static void RenderPhobiaLayout();

	// =========================================================================
	// 16. IMGUI ATERNOS (Release 16 - 100% Matching Screenshot 1)
	// =========================================================================
	static void RenderAternosLayout()
	{
		ImGui::SetNextWindowPos(ImVec2(40, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(760, 520), ImGuiCond_FirstUseEver);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.10f, 0.98f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.16f, 0.20f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.13f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.00f, 0.87f, 0.72f, 1.00f)); // Mint/Cyan #00DFB8
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.00f, 0.87f, 0.72f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.20f, 1.00f, 0.85f, 1.00f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);

		ImGui::Begin("LOVEMACHINE CS:S - Aternos###AternosWnd", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		auto draw = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		// Left Sidebar (200px)
		draw->AddRectFilled(pos, ImVec2(pos.x + 195, pos.y + size.y), IM_COL32(11, 14, 18, 255), 8.0f, ImDrawFlags_RoundCornersLeft);
		draw->AddLine(ImVec2(pos.x + 195, pos.y), ImVec2(pos.x + 195, pos.y + size.y), IM_COL32(24, 30, 38, 255), 1.0f);

		struct AternosTab { const char* icon; const char* title; const char* desc; int id; };
		const AternosTab at_tabs[] = {
			{ ICON_FA_CROSSHAIRS, "Aimbot", "Shooting is more accur...", 0 },
			{ ICON_FA_EYE, "Visuals", "Changing the look and...", 1 },
			{ ICON_FA_PAINT_BRUSH, "Skins", "Changing characters, w...", 2 },
			{ ICON_FA_COG, "Configs", "Function customization...", 3 },
			{ ICON_FA_SLIDERS_H, "Settings", "Changing the paramet...", 4 }
		};

		static int at_selected_tab = 0;
		ImGui::SetCursorPos(ImVec2(10, 15));
		for (int i = 0; i < (int)IM_ARRAYSIZE(at_tabs); i++)
		{
			bool is_sel = (at_selected_tab == i);
			ImVec2 cur = ImGui::GetCursorScreenPos();

			if (is_sel)
			{
				draw->AddRectFilled(cur, ImVec2(cur.x + 175, cur.y + 54), IM_COL32(16, 26, 32, 255), 6.0f);
				draw->AddRect(cur, ImVec2(cur.x + 175, cur.y + 54), IM_COL32(0, 223, 184, 180), 6.0f);
			}

			// Draw icon inside small box
			draw->AddRectFilled(ImVec2(cur.x + 8, cur.y + 12), ImVec2(cur.x + 38, cur.y + 42), is_sel ? IM_COL32(0, 223, 184, 40) : IM_COL32(20, 25, 32, 255), 6.0f);
			draw->AddText(ImVec2(cur.x + 16, cur.y + 19), is_sel ? IM_COL32(0, 223, 184, 255) : IM_COL32(110, 120, 135, 255), at_tabs[i].icon);

			// Title & Desc
			draw->AddText(ImVec2(cur.x + 46, cur.y + 12), is_sel ? IM_COL32(255, 255, 255, 255) : IM_COL32(140, 150, 165, 255), at_tabs[i].title);
			draw->AddText(ImVec2(cur.x + 46, cur.y + 30), IM_COL32(90, 100, 115, 255), at_tabs[i].desc);

			if (ImGui::InvisibleButton(at_tabs[i].title, ImVec2(175, 54)))
			{
				at_selected_tab = i;
				current_tab = (i == 0) ? 0 : ((i == 1) ? 3 : ((i == 2) ? 4 : 5));
			}
			ImGui::Spacing();
		}

		// Main Content Area
		ImGui::SetCursorPos(ImVec2(210, 15));
		ImGui::BeginChild("AtMainBody", ImVec2(size.x - 225, size.y - 30), false);
		RenderCurrentTabContent(current_tab, 1.0f);
		ImGui::EndChild();

		ImGui::End();

		// Separate Window: ESP PREVIEW (Exact match to Screenshot 1)
		ImGui::SetNextWindowPos(ImVec2(815, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(310, 520), ImGuiCond_FirstUseEver);
		ImGui::Begin("LOVEMACHINE CS:S - ESP Preview###AtEspPreviewWnd", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		{
			auto esp_draw = ImGui::GetWindowDrawList();
			ImVec2 ep = ImGui::GetWindowPos();
			ImVec2 es = ImGui::GetWindowSize();

			// Header Title
			esp_draw->AddText(font_brand_title ? font_brand_title : ImGui::GetFont(), 18.0f, ImVec2(ep.x + 18, ep.y + 16), IM_COL32(255, 255, 255, 255), "ESP PREVIEW");

			// 3D Player Mannequin Representation
			RenderEspLivePreview(ImVec2(es.x - 30, 360), 0);

			// ESP Overlay Labels around Player
			esp_draw->AddText(ImVec2(ep.x + 50, ep.y + 60), IM_COL32(74, 222, 128, 255), "95%");
			esp_draw->AddText(ImVec2(ep.x + 130, ep.y + 60), IM_COL32(255, 255, 255, 255), "Nickname");
			esp_draw->AddText(ImVec2(ep.x + 245, ep.y + 80), IM_COL32(239, 68, 68, 255), "Bomb");
			esp_draw->AddText(ImVec2(ep.x + 245, ep.y + 105), IM_COL32(250, 204, 21, 255), "50$");
			esp_draw->AddText(ImVec2(ep.x + 245, ep.y + 130), IM_COL32(249, 115, 22, 255), "C4");
			esp_draw->AddText(ImVec2(ep.x + 245, ep.y + 155), IM_COL32(0, 223, 184, 255), "HIT");

			// Bottom filter pill buttons
			ImGui::SetCursorPos(ImVec2(15, 410));
			ImGui::BeginGroup();
			{
				ImGui::TextColored(ImVec4(0.6f, 0.7f, 0.8f, 1.0f), "  Zoomed       Weapon");
				static bool btn_nick = false, btn_wep = false, btn_zoom = true, btn_bomb = true, btn_hp = false;
				if (ImGui::Button("Nickname", ImVec2(60, 22))) btn_nick = !btn_nick; ImGui::SameLine();
				if (ImGui::Button("Weapon", ImVec2(55, 22))) btn_wep = !btn_wep; ImGui::SameLine();
				if (ImGui::Button("Zoom", ImVec2(45, 22))) btn_zoom = !btn_zoom; ImGui::SameLine();
				if (ImGui::Button("Bomb", ImVec2(45, 22))) btn_bomb = !btn_bomb; ImGui::SameLine();
				if (ImGui::Button("HP", ImVec2(35, 22))) btn_hp = !btn_hp;
				ImGui::Spacing();
				ImGui::Button("Money", ImVec2(50, 22)); ImGui::SameLine();
				ImGui::Button("Hit", ImVec2(35, 22)); ImGui::SameLine();
				ImGui::Button("Box", ImVec2(35, 22)); ImGui::SameLine();
				ImGui::Button("C4", ImVec2(35, 22));
			}
			ImGui::EndGroup();
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(6);
	}

	// =========================================================================
	// 19. SYNTHETIC / SPACE GALAXY (Release 19 - 100% Matching Screenshot 2)
	// =========================================================================
	// =========================================================================
	// 19. SYNTHETIC / SPACE GALAXY (Release 19 - 100% Interactive & Matching Screenshot 2)
	// =========================================================================
	static int synth_active_tab = 0;
	static bool synth_keybind_popup = false;
	static const char* synth_keybind_target = "Enable ragebot";

	static void RenderSyntheticGalaxyLayout()
	{
		auto draw_bg = ImGui::GetBackgroundDrawList();
		ImVec2 screen_sz = ImGui::GetIO().DisplaySize;

		// 1. Top Right Status Watermark Pill
		draw_bg->AddRectFilled(ImVec2(screen_sz.x - 370, 16), ImVec2(screen_sz.x - 20, 46), IM_COL32(18, 16, 24, 230), 15.0f);
		draw_bg->AddRect(ImVec2(screen_sz.x - 370, 16), ImVec2(screen_sz.x - 20, 46), IM_COL32(45, 40, 60, 200), 15.0f);
		draw_bg->AddText(ImVec2(screen_sz.x - 355, 22), IM_COL32(255, 255, 255, 255), "SYNTHETIC");
		draw_bg->AddText(ImVec2(screen_sz.x - 265, 22), IM_COL32(130, 120, 150, 255), "•  Server  •  144FPS  •  65PING  •  12:15PM");

		// 2. Top Left Honeycomb Selection Window (7 Hexagons: 2 - 3 - 2)
		ImGui::SetNextWindowPos(ImVec2(15, 60), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(180, 200), ImGuiCond_FirstUseEver);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::Begin("LOVEMACHINE CS:S - Synthetic Selection###SynthSelectionWnd", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
		{
			auto sel_draw = ImGui::GetWindowDrawList();
			auto RenderHexButton = [&](const char* icon, const char* id_name, int id, ImVec2 cur_pos) {
				ImGui::SetCursorPos(cur_pos);
				ImVec2 sp = ImGui::GetCursorScreenPos();
				ImVec2 center(sp.x + 22, sp.y + 22);
				float r = 20.0f;
				bool is_sel = (synth_active_tab == id);

				ImVec2 pts[6];
				for (int i = 0; i < 6; i++)
				{
					float angle = (i * 60.0f) * 3.14159265f / 180.0f;
					pts[i] = ImVec2(center.x + r * cosf(angle), center.y + r * sinf(angle));
				}

				ImGui::InvisibleButton(id_name, ImVec2(44, 44));
				bool hovered = ImGui::IsItemHovered();
				if (ImGui::IsItemClicked()) synth_active_tab = id;

				sel_draw->AddConvexPolyFilled(pts, 6, is_sel ? IM_COL32(45, 28, 70, 255) : (hovered ? IM_COL32(32, 26, 44, 255) : IM_COL32(18, 16, 24, 255)));
				sel_draw->AddPolyline(pts, 6, is_sel ? IM_COL32(168, 85, 247, 255) : (hovered ? IM_COL32(130, 90, 180, 255) : IM_COL32(45, 38, 55, 255)), ImDrawFlags_Closed, is_sel ? 2.0f : 1.5f);
				sel_draw->AddText(ImVec2(center.x - 7, center.y - 7), is_sel ? IM_COL32(235, 205, 255, 255) : IM_COL32(140, 125, 160, 255), icon);
			};

			// 7 Hexagons arranged in authentic Honeycomb formation
			RenderHexButton(ICON_FA_SKULL, "##HexRage", 0, ImVec2(34, 5));
			RenderHexButton(ICON_FA_SHIELD_ALT, "##HexAA", 1, ImVec2(82, 5));

			RenderHexButton(ICON_FA_CROSSHAIRS, "##HexLegit", 2, ImVec2(10, 48));
			RenderHexButton(ICON_FA_COMPACT_DISC, "##HexVis", 3, ImVec2(58, 48));
			RenderHexButton(ICON_FA_FOLDER, "##HexMisc", 4, ImVec2(106, 48));

			RenderHexButton(ICON_FA_CHART_BAR, "##HexCfg", 5, ImVec2(34, 91));
			RenderHexButton(ICON_FA_COG, "##HexSet", 6, ImVec2(82, 91));
		}
		ImGui::End();
		ImGui::PopStyleColor(2);

		// 3. Center Cheat Window (780 x 540)
		ImGui::SetNextWindowPos(ImVec2((screen_sz.x - 780) * 0.5f, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(780, 540), ImGuiCond_FirstUseEver);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.05f, 0.08f, 0.98f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.14f, 0.24f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.07f, 0.11f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.66f, 0.33f, 0.97f, 1.00f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

		ImGui::Begin("LOVEMACHINE CS:S - Synthetic###SyntheticWnd", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		auto draw = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		// Vertical Left Strip with Logo and "S Y N T H E T I C"
		draw->AddLine(ImVec2(pos.x + 55, pos.y), ImVec2(pos.x + 55, pos.y + size.y), IM_COL32(35, 28, 48, 255), 1.0f);
		draw->AddText(ImVec2(pos.x + 22, pos.y + 80), IM_COL32(140, 120, 160, 255), "S\n\nY\n\nN\n\nT\n\nH\n\nE\n\nT\n\nI\n\nC");
		draw->AddText(font_brand_title ? font_brand_title : ImGui::GetFont(), 32.0f, ImVec2(pos.x + 12, pos.y + 240), IM_COL32(168, 85, 247, 255), ICON_FA_INFINITY);

		// Helper to draw Fully Interactive Equalizer Bars Slider
		auto DrawEqualizerSlider = [&](const char* label, float* val, float min_v, float max_v, const char* fmt) {
			ImGui::TextColored(ImVec4(0.88f, 0.88f, 0.92f, 1.0f), "%s", label);
			ImGui::SameLine(175);
			char val_buf[32];
			snprintf(val_buf, sizeof(val_buf), fmt, *val);
			ImGui::TextColored(ImVec4(0.70f, 0.40f, 1.00f, 1.0f), "%s", val_buf);
			ImGui::SameLine(225);

			ImVec2 cur = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton((std::string("##eq_") + label).c_str(), ImVec2(80, 18));

			bool active = ImGui::IsItemActive();
			if (active && ImGui::GetIO().MouseDown[0])
			{
				float mouse_x = ImGui::GetIO().MousePos.x;
				float frac = ImClamp((mouse_x - cur.x) / 80.0f, 0.0f, 1.0f);
				*val = min_v + frac * (max_v - min_v);
			}

			float current_frac = ImClamp((*val - min_v) / (max_v - min_v), 0.0f, 1.0f);
			for (int b = 0; b < 11; b++)
			{
				float b_frac = (float)b / 11.0f;
				bool lit = (b_frac <= current_frac);
				float h = 6.0f + ((b * 3) % 7);
				draw->AddRectFilled(ImVec2(cur.x + b * 7, cur.y + 14 - h), ImVec2(cur.x + b * 7 + 4, cur.y + 14), lit ? IM_COL32(168, 85, 247, 255) : IM_COL32(40, 32, 50, 255), 1.0f);
			}
			ImGui::NewLine();
		};

		// Helper to render Gear Button
		auto RenderGearBtn = [&](const char* target_name) {
			ImGui::SameLine(280);
			if (ImGui::Button((std::string(ICON_FA_COG "##gear_") + target_name).c_str(), ImVec2(24, 22)))
			{
				synth_keybind_target = target_name;
				synth_keybind_popup = true;
			}
		};

		// Honeycomb Tab Routing to full functional tab bodies
		ImGui::SetCursorPos(ImVec2(75, 20));
		ImGui::BeginChild("SynthBodyPanel", ImVec2(size.x - 90, size.y - 40), false);
		int target_subtab = (synth_active_tab == 0) ? 0 : ((synth_active_tab == 1) ? 1 : ((synth_active_tab == 2) ? 2 : ((synth_active_tab == 3) ? 3 : ((synth_active_tab == 4) ? 4 : 5))));
		RenderCurrentTabContent(target_subtab, 1.0f);
		ImGui::EndChild();

		// Floating Keybind Popup Window (When clicking [gear])
		if (synth_keybind_popup)
		{
			ImGui::SetNextWindowSize(ImVec2(240, 160));
			ImGui::SetNextWindowPos(ImVec2(pos.x + size.x * 0.5f - 120, pos.y + size.y * 0.5f - 80));
			ImGui::Begin("Keybind Config###SynthKeybindModal", &synth_keybind_popup, ImGuiWindowFlags_NoCollapse);
			{
				ImGui::TextColored(ImVec4(0.7f, 0.4f, 1.0f, 1.0f), "Bind: %s", synth_keybind_target);
				ImGui::Separator();
				static int bind_mode = 0;
				ImGui::RadioButton("Toggle", &bind_mode, 0); ImGui::SameLine();
				ImGui::RadioButton("Hold", &bind_mode, 1);
				static bool show_in_binds = true;
				ImGui::Checkbox("Show in Keybinds list", &show_in_binds);
				if (ImGui::Button("Close", ImVec2(220, 24))) synth_keybind_popup = false;
			}
			ImGui::End();
		}

		ImGui::End();
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(4);
	}

	void SetupStyle()
	{
		ApplyTheme(current_theme);
	}

	void Render()
	{
		// Global toggle hotkey check (checks every frame)
		if (!is_binding_key && sets)
		{
			for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
			{
				if (ImGui::IsKeyPressed((ImGuiKey)k, false))
				{
					int vk = 0x2D;
					if (k >= ImGuiKey_F1 && k <= ImGuiKey_F12) vk = 0x70 + (k - ImGuiKey_F1);
					else if (k >= ImGuiKey_0 && k <= ImGuiKey_9) vk = 0x30 + (k - ImGuiKey_0);
					else if (k >= ImGuiKey_A && k <= ImGuiKey_Z) vk = 0x41 + (k - ImGuiKey_A);
					else if (k == ImGuiKey_Insert) vk = 0x2D;
					else if (k == ImGuiKey_Delete) vk = 0x2E;
					else if (k == ImGuiKey_Home) vk = 0x24;
					else if (k == ImGuiKey_End) vk = 0x23;
					else if (k == ImGuiKey_PageUp) vk = 0x21;
					else if (k == ImGuiKey_PageDown) vk = 0x22;
					else if (k == ImGuiKey_GraveAccent) vk = 0xC0;
					else if (k == ImGuiKey_RightShift) vk = 0xA1;
					else if (k == ImGuiKey_LeftShift) vk = 0xA0;
					else if (k == ImGuiKey_RightCtrl) vk = 0xA3;
					else if (k == ImGuiKey_LeftCtrl) vk = 0xA2;
					else if (k == ImGuiKey_Tab) vk = 0x09;
					else vk = k;

					if (vk == sets->menu.menu_key || vk == 0x2D || k == sets->menu.menu_key || k == ImGuiKey_Insert)
					{
						show_menu = !show_menu;
						sets->menu.opened = show_menu;
						break;
					}
				}
			}
		}

		if (!show_menu) return;

		static int prev_layout = -1;
		if (current_layout != prev_layout)
		{
			prev_layout = current_layout;
			if (current_layout == LAYOUT_GAMESENSE)
			{
				ApplyTheme(THEME_SKEET);
				ui_accent_color = ImVec4(0.15f, 0.85f, 0.45f, 1.0f);
			}
			else if (current_layout == LAYOUT_NEVERLOSE)
			{
				ApplyTheme(THEME_NEVERLOSE);
				ui_accent_color = ImVec4(0.00f, 0.82f, 1.00f, 1.0f);
			}
			else if (current_layout == LAYOUT_ATERNOS)
			{
				ApplyTheme(THEME_ONYX);
				ui_accent_color = ImVec4(0.00f, 0.87f, 0.72f, 1.0f);
			}
			else
			{
				ApplyTheme(THEME_CYBERPUNK);
				ui_accent_color = ImVec4(0.66f, 0.33f, 0.97f, 1.0f);
			}
		}

		switch (current_layout)
		{
		case LAYOUT_GAMESENSE:
			RenderSkeetLayout();
			break;
		case LAYOUT_NEVERLOSE:
			RenderNeverloseLayout();
			break;
		case LAYOUT_ATERNOS:
			RenderAternosLayout();
			break;
		case LAYOUT_SYNTHETIC:
			RenderSyntheticGalaxyLayout();
			break;
		default:
			RenderSkeetLayout();
			break;
		}
	}
}

#include "menu.h"
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
	int current_tab = 3; // Default to Players ESP
	int current_theme = THEME_SKEET;
	int current_layout = LAYOUT_SKEET;

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

	// Modern Glassmorphism Animated Switch (Rounded Pill Toggle Switch)
	static bool AnimatedSwitch(const char* label, bool* v)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		const float height = 20.0f;
		const float width = 38.0f;
		const float radius = height * 0.5f;

		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0 ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + ImMax(label_size.y, height)));
		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id)) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) *v = !*v;

		// Smooth animation interpolation per control ID
		static std::unordered_map<ImGuiID, float> anim_map;
		float& anim_t = anim_map[id];
		float target_t = *v ? 1.0f : 0.0f;
		float dt = ImGui::GetIO().DeltaTime * 16.0f * ui_anim_speed;
		anim_t += (target_t - anim_t) * dt;
		if (std::abs(target_t - anim_t) < 0.001f) anim_t = target_t;

		// Dynamic color interpolation
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

		// Glow aura on active state
		if (anim_t > 0.01f)
		{
			window->DrawList->AddRect(
				ImVec2(switch_bb.Min.x - 1.5f, switch_bb.Min.y - 1.5f),
				ImVec2(switch_bb.Max.x + 1.5f, switch_bb.Max.y + 1.5f),
				IM_COL32(cr, cg, cb, (int)(110 * anim_t)), radius + 1.5f, 0, 1.5f
			);
		}

		// Sliding knob position
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

		// Header Label
		const char* header_txt = (preview_type == 0) ? "Interactive 3D Player ESP Preview" : (preview_type == 1 ? "Interactive 3D Weapon Preview" : "Interactive 3D C4 & Grenade Preview");
		draw_list->AddText(ImVec2(cursor.x + 16, cursor.y + 12), IM_COL32(0, 220, 255, 255), header_txt);
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
			model_angle += ImGui::GetIO().DeltaTime * 0.45f;
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
					if (is_chams)
					{
						int cr = ImMin(255, (int)(sets->visuals.chams_t.r * light));
						int cg = ImMin(255, (int)(sets->visuals.chams_t.g * light));
						int cb = ImMin(255, (int)(sets->visuals.chams_t.b * light));
						fill_col = IM_COL32(cr, cg, cb, 255);
					}
					else
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

			// Render 100% Solid Opaque Triangles (Zero see-through transparency!)
			for (const auto& tri : tri_list)
			{
				draw_list->AddTriangleFilled(tri.p0, tri.p1, tri.p2, tri.fill_color);
			}

			// Holographic Cyan Rim Contour Highlight
			for (const auto& tri : tri_list)
			{
				draw_list->AddTriangle(tri.p0, tri.p1, tri.p2, IM_COL32(0, 210, 255, 20), 0.8f);
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
		ImGuiChildFlags flags = ImGuiChildFlags_Borders;
		if (height <= 0.0f)
			flags |= ImGuiChildFlags_AutoResizeY;
		ImGui::BeginChild(title, ImVec2(0, height), flags);
		ImGui::TextColored(ui_accent_color, "%s", title);
		ImGui::Separator();
		ImGui::Spacing();
	}

	static void EndGroupbox()
	{
		ImGui::EndChild();
		ImGui::Spacing();
	}

	static void RenderRagebotTab(float tab_alpha)
	{
		float col_w = (ImGui::GetContentRegionAvail().x - 10.0f) * 0.5f;

		// Column 1: Aimbot Logic & Targeting
		ImGui::BeginChild("RageCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("MAIN");
			AnimatedSwitch("Enabled", &sets->rage.enabled);
			AnimatedSwitch("Silent Aim", &sets->rage.silent);
			AnimatedSwitch("Automatic Fire", &sets->rage.autoshoot);
			AnimatedSwitch("Aim Through Walls", &sets->rage.autowall);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Field of View", &sets->rage.hitchance, 0.0f, 180.0f, "%.1f°");
			EndGroupbox();

			BeginGroupbox("SELECTION");
			const char* hit_modes[] = { "Hit Chance", "Damage", "Distance" };
			static int sel_hit = 0;
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Target", &sel_hit, hit_modes, IM_ARRAYSIZE(hit_modes));
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
			ImGui::Combo("History", &sel_hist, hist_modes, IM_ARRAYSIZE(hist_modes));
			AnimatedSwitch("Delay Shot", &sets->rage.autostop);
			AnimatedSwitch("Remove Recoil", &sets->rage.silent);
			AnimatedSwitch("Remove Spread", &sets->rage.autoscope);
			AnimatedSwitch("Duck Peek Assist", &sets->misc.fake_duck);
			AnimatedSwitch("Quick Peek Assist", &sets->misc.slow_walk);
			AnimatedSwitch("Double Tap", &sets->rage.magic_bullet);
			EndGroupbox();

			BeginGroupbox("ANTI-AIM");
			AnimatedSwitch("Enabled", &sets->rage.spinbot);
			const char* pitch_modes[] = { "Off", "Down", "Up", "Zero" };
			ImGui::SetNextItemWidth(170);
			ImGui::Combo("Pitch", &sets->rage.pitch_aa, pitch_modes, IM_ARRAYSIZE(pitch_modes));
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
			BeginGroupbox("ANTI-AIM ANGLES");
			AnimatedSwitch("Enable Anti-Aim / Spinbot", &sets->rage.spinbot);
			
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
			BeginGroupbox("Legit Aimbot Master");
			AnimatedSwitch("Enable Legitbot", &sets->legit.enabled);
			AnimatedSwitch("Draw FOV Circle", &sets->legit.aim.draw_fov);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("FOV (First Bullet)", &sets->legit.aim.fov, 0.1f, 30.0f, "%.1f deg");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Smooth (First Bullet)", &sets->legit.aim.smooth[0], 1.0f, 100.0f, "%.1f");
			AnimatedSwitch("Use First Bullet Settings for Spray", &sets->legit.aim.use_first_bullet_settings);
			if (!sets->legit.aim.use_first_bullet_settings)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("FOV (Spray Bullets)", &sets->legit.aim.other_fov, 0.1f, 30.0f, "%.1f deg");
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Smooth (Spray Bullets)", &sets->legit.aim.other_smooth[0], 1.0f, 100.0f, "%.1f");
			}
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

		// Column 2: Backtrack, Conditions & Triggerbot
		ImGui::BeginChild("LegitCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Legit Backtracking");
			AnimatedSwitch("Enable Backtrack", &sets->legit.backtrack.enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("Backtrack Ticks", &sets->legit.backtrack.ticks, 1, 12, "%d ticks");
			EndGroupbox();

			BeginGroupbox("Disable Aimbot Conditions");
			AnimatedSwitch("Disable when Flashed", &sets->legit.aim.disable_flashed);
			AnimatedSwitch("Disable inside Smoke", &sets->legit.aim.disable_in_smoke);
			AnimatedSwitch("Disable while Jumping", &sets->legit.aim.disable_in_jump);
			EndGroupbox();

			BeginGroupbox("Triggerbot");
			AnimatedSwitch("Enable Triggerbot", &sets->legit.trigger._enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderFloat("Trigger Shot Delay", &sets->legit.trigger.delay, 0.0f, 0.50f, "%.2f s");
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
			AnimatedSwitch("2D Bounding Box", &sets->visuals.esp_show[0]);
			AnimatedSwitch("Skeleton ESP (Bones)", &sets->visuals.skeleton);
			AnimatedSwitch("Health Bar", &sets->visuals.esp_show[1]);
			AnimatedSwitch("Armor Bar", &sets->visuals.armor_bar);
			AnimatedSwitch("Ammo Bar", &sets->visuals.ammo_bar);
			AnimatedSwitch("Active Weapon Text", &sets->visuals.esp_show[2]);
			AnimatedSwitch("Snaplines to Enemy", &sets->visuals.esp_show[3]);
			EndGroupbox();

			BeginGroupbox("Player Flags");
			AnimatedSwitch("Flag [HK] (Helmet/Kevlar)", &sets->visuals.flag_hk);
			AnimatedSwitch("Flag [SCOPED]", &sets->visuals.flag_scoped);
			AnimatedSwitch("Flag [RELOADING]", &sets->visuals.flag_reloading);
			AnimatedSwitch("Flag [FLASHED]", &sets->visuals.flag_flashed);
			EndGroupbox();

			BeginGroupbox("Player Chams");
			const char* chams_modes[] = { "Disabled", "Flat Colored", "Material Shaded", "Wireframe" };
			ImGui::SetNextItemWidth(160);
			ImGui::Combo("Chams Mode", &sets->visuals.chams, chams_modes, IM_ARRAYSIZE(chams_modes));
			ColorEdit3Custom("Terrorist (T) Color", sets->visuals.chams_t);
			ColorEdit3Custom("Counter-Terrorist (CT) Color", sets->visuals.chams_ct);
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

			BeginGroupbox("Camera Bypass");
			AnimatedSwitch("Thirdperson (No sv_cheats)", &sets->visuals.thirdperson);
			if (sets->visuals.thirdperson)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Camera Distance", &sets->visuals.thirdperson_dist, 30.0f, 300.0f, "%.0f u");
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

		// Column 1: Movement Exploits
		ImGui::BeginChild("MiscCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Movement Exploits");
			AnimatedSwitch("Bunnyhop (Bhop)", &sets->misc.autojump);
			AnimatedSwitch("Auto Strafer", &sets->misc.autostrafer);
			AnimatedSwitch("Fast Ladder Climb", &sets->misc.fast_ladder);
			AnimatedSwitch("Circle Strafe Helper", &sets->misc.circle_strafe);
			AnimatedSwitch("Auto Edge Jump", &sets->misc.edge_jump);
			AnimatedSwitch("Slow Walk", &sets->misc.slow_walk);
			if (sets->misc.slow_walk)
			{
				ImGui::SetNextItemWidth(170);
				ImGui::SliderFloat("Slow Walk Speed", &sets->misc.slow_walk_speed, 10.0f, 100.0f, "%.0f u/s");
			}
			AnimatedSwitch("Fake Duck", &sets->misc.fake_duck);
			EndGroupbox();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Column 2: Network & Server Bypasses
		ImGui::BeginChild("MiscCol2", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("FakeLag & Network");
			AnimatedSwitch("Enable FakeLag", &sets->misc.fakelag_enabled);
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("FakeLag Limit", &sets->misc.fakelag_limit, 1, 16, "%d ticks");
			ImGui::SetNextItemWidth(170);
			ImGui::SliderInt("FakeLag Random", &sets->misc.fakelag_random, 0, 8, "%d ticks");
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

		// Column 1: Structural UI Layout Presets (4 LAYOUTS)
		ImGui::BeginChild("SettingsCol1", ImVec2(col_w, 0), false);
		{
			BeginGroupbox("Structural UI Architecture (4 Layouts)");
			const char* layout_names[] = {
				"1. Gamesense (Skeet TopBar)",
				"2. Onetap v4 (Glassmorphic Pills)",
				"3. Fatality (3-Column Dense Grid)",
				"4. Neverlose 2.0 (3D Mesh Sidebar)"
			};
			ImGui::SetNextItemWidth(190);
			if (ImGui::Combo("UI Architecture", &current_layout, layout_names, IM_ARRAYSIZE(layout_names)))
			{
				ApplyTheme(current_theme);
			}
			ImGui::Spacing();
			ImGui::TextWrapped("Selecting a UI Architecture restructures the entire menu layout, tabs, and navigation style!");
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

		// Left Vertical Icon Strip Column
		ImGui::SetCursorPos(ImVec2(10, 12));
		ImGui::BeginChild("SkeetIconSidebar", ImVec2(60, 0), true);
		struct IconTabMap { const char* icon; int tab_id; };
		const IconTabMap skeet_icons[] = {
			{ "(R)", 0 }, // Ragebot
			{ "(A)", 1 }, // Anti-Aim
			{ "(L)", 2 }, // Legitbot
			{ "(V)", 3 }, // Visuals (All ESP & Chams)
			{ "(M)", 4 }, // Misc Exploits
			{ "(*)", 5 }  // Settings
		};
		for (int i = 0; i < IM_ARRAYSIZE(skeet_icons); i++)
		{
			bool is_sel = (current_tab == skeet_icons[i].tab_id);
			if (is_sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ui_accent_color.x * 0.35f, ui_accent_color.y * 0.35f, ui_accent_color.z * 0.35f, 0.85f));
			if (ImGui::Button(skeet_icons[i].icon, ImVec2(44, 38))) { current_tab = skeet_icons[i].tab_id; }
			if (is_sel) ImGui::PopStyleColor();
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

	// LAYOUT 2: ONETAP V4 / PRIMORDIAL (SCREENSHOT 4)
	static void RenderOnetapLayout()
	{
		ImGui::SetNextWindowPos(ImVec2(100, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(880, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin("LOVEMACHINE CS:S - Onetap v4", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		// Top Header Subtabs Bar with Skull Brand
		ImGui::BeginChild("OnetapTopBar", ImVec2(0, 42), true);
		ImGui::TextColored(ui_accent_color, " [SKULL]");
		ImGui::SameLine(0, 12);

		struct OnetapTab { const char* name; int tab_id; };
		const OnetapTab ot_top_tabs[] = {
			{ "Legit", 2 },
			{ "Rage", 0 },
			{ "Anti-Aim", 1 },
			{ "Visuals", 3 },
			{ "Misc", 4 },
			{ "Settings", 5 }
		};

		for (int i = 0; i < IM_ARRAYSIZE(ot_top_tabs); i++)
		{
			bool is_sel = (current_tab == ot_top_tabs[i].tab_id);
			if (is_sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ui_accent_color.x * 0.4f, ui_accent_color.y * 0.4f, ui_accent_color.z * 0.4f, 0.9f));
			if (ImGui::Button(ot_top_tabs[i].name, ImVec2(84, 26))) { current_tab = ot_top_tabs[i].tab_id; }
			if (is_sel) ImGui::PopStyleColor();
			if (i < IM_ARRAYSIZE(ot_top_tabs) - 1) ImGui::SameLine();
		}
		ImGui::EndChild();

		ImGui::Spacing();

		// Main Content Panel
		ImGui::BeginChild("OnetapMainBody", ImVec2(0, 0), true);
		RenderCurrentTabContent(current_tab, 1.0f);
		ImGui::EndChild();

		ImGui::End();
	}

	// LAYOUT 3: FATALITY / EV0LVE (SCREENSHOT 5)
	static void RenderFatalityLayout()
	{
		ImGui::SetNextWindowPos(ImVec2(100, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(900, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin("LOVEMACHINE CS:S - Fatality.win", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		// Top Clippy Mascot & Brand Bar
		ImGui::BeginChild("FatalityHeader", ImVec2(0, 42), true);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), " FATALITY ");
		ImGui::SameLine(0, 12);

		struct FatalityTab { const char* name; int tab_id; };
		const FatalityTab fat_tabs[] = {
			{ "RAGE", 0 },
			{ "ANTI-AIM", 1 },
			{ "LEGIT", 2 },
			{ "VISUALS", 3 },
			{ "MISC", 4 },
			{ "Settings", 5 }
		};

		for (int i = 0; i < IM_ARRAYSIZE(fat_tabs); i++)
		{
			bool is_sel = (current_tab == fat_tabs[i].tab_id);
			if (is_sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ui_accent_color.x * 0.4f, ui_accent_color.y * 0.4f, ui_accent_color.z * 0.4f, 0.9f));
			if (ImGui::Button(fat_tabs[i].name, ImVec2(74, 26))) { current_tab = fat_tabs[i].tab_id; }
			if (is_sel) ImGui::PopStyleColor();
			if (i < IM_ARRAYSIZE(fat_tabs) - 1) ImGui::SameLine();
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 170);
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "breakcore | 3 days");
		ImGui::EndChild();

		// Main Content Panel
		ImGui::BeginChild("FatalityMainBody", ImVec2(0, 0), true);
		RenderCurrentTabContent(current_tab, 1.0f);
		ImGui::EndChild();

		ImGui::End();
	}

	// LAYOUT 4: NEVERLOSE 2.0 (SCREENSHOTS 1 & 2)
	static void RenderNeverloseLayout()
	{
		ImGui::SetNextWindowPos(ImVec2(100, 70), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(940, 660), ImGuiCond_FirstUseEver);
		ImGui::Begin("LOVEMACHINE CS:S - Neverlose 2.0", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		// 1. Left Sidebar Navigation
		ImGui::BeginChild("LeftSidebarNL", ImVec2(190, 0), true);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "  NL  Neverlose");
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::TextColored(ImVec4(0.45f, 0.50f, 0.60f, 1.00f), "AIMBOT");
		if (ImGui::Button("  Rage", ImVec2(170, 30))) { current_tab = 0; }
		if (ImGui::Button("  Anti-Aim", ImVec2(170, 30))) { current_tab = 1; }
		if (ImGui::Button("  Legit", ImVec2(170, 30))) { current_tab = 2; }

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.45f, 0.50f, 0.60f, 1.00f), "VISUALS");
		if (ImGui::Button("  Visuals & ESP", ImVec2(170, 30))) { current_tab = 3; }

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.45f, 0.50f, 0.60f, 1.00f), "MISC");
		if (ImGui::Button("  Miscellaneous", ImVec2(170, 30))) { current_tab = 4; }

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.45f, 0.50f, 0.60f, 1.00f), "SYSTEM");
		if (ImGui::Button("  Settings", ImVec2(170, 30))) { current_tab = 5; }

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 54);
		ImGui::Separator();
		ImGui::TextColored(ui_accent_color, "  [Avatar] NoHyper");
		ImGui::TextColored(ImVec4(0.45f, 0.50f, 0.60f, 1.00f), "  59 days left >");
		ImGui::EndChild();

		ImGui::SameLine();

		// 2. Right Main Panel
		ImGui::BeginChild("NeverloseMainBody", ImVec2(0, 0), true);

		// Top Presets Bar
		ImGui::TextColored(ImVec4(0.70f, 0.75f, 0.85f, 1.00f), " Preset: NoHyper NL HvH  v   Scope: Global  v");
		ImGui::Separator();
		ImGui::Spacing();

		RenderCurrentTabContent(current_tab, 1.0f);
		ImGui::EndChild();

		ImGui::End();
	}

	void SetupStyle()
	{
		ApplyTheme(current_theme);
	}

	void Render()
	{
		if (!show_menu) return;

		static int prev_layout = -1;
		if (current_layout != prev_layout)
		{
			prev_layout = current_layout;
			switch (current_layout)
			{
			case LAYOUT_SKEET: ApplyTheme(THEME_SKEET); break;
			case LAYOUT_ONETAP: ApplyTheme(THEME_CYBERPUNK); break;
			case LAYOUT_FATALITY: ApplyTheme(THEME_ONYX); break;
			case LAYOUT_NEVERLOSE: default: ApplyTheme(THEME_NEVERLOSE); break;
			}
		}

		switch (current_layout)
		{
		case LAYOUT_SKEET:
			RenderSkeetLayout();
			break;
		case LAYOUT_ONETAP:
			RenderOnetapLayout();
			break;
		case LAYOUT_FATALITY:
			RenderFatalityLayout();
			break;
		case LAYOUT_NEVERLOSE:
		default:
			RenderNeverloseLayout();
			break;
		}
	}
}

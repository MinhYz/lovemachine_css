#pragma once
#include <algorithm>
#include "game shit.h"
#include "game def's.h"
#include "game classes.h"
#include "events.h"
#include "settings.h"
#include "surface.h"
#include "d3d.h"

using namespace d3d;

namespace esp
{
	struct cbox
	{
		cbox() : class_id(0), width(0), height(0), left(0), right(0), top(0), bottom(0), centerx(0) {}

		cbox(centity* entity, int class_id) : class_id(class_id), width(0), height(0), left(0), right(0), top(0), bottom(0), centerx(0)
		{
			if (!entity) return;
			const auto collideable = entity->get_collideable();
			if (!collideable) return;

			this->origin = entity->get_abs_origin();
			if (class_id == CCSPlayer)
			{
				this->max_origin = this->origin + Vector(0, 0, collideable->obb_maxs().z);
				this->origin.z -= 10.f;
				this->max_origin.z += 10.f;
			}
			else
			{
				this->max_origin = this->origin + collideable->obb_maxs();
				this->origin += collideable->obb_mins();
			}
		}

		bool construct_points()
		{
			cvector screen, max_screen;

			if (class_id == CCSPlayer)
			{
				bool w2s_origin = w2s(origin, screen);
				bool w2s_head = w2s(max_origin, max_screen);

				if (!w2s_origin && !w2s_head)
					return false;

				if (w2s_origin && !w2s_head)
				{
					max_screen.x = screen.x;
					float dist = (global::local) ? (origin - global::local->get_origin()).Length() : 400.0f;
					float estimated_height = (dist > 0.1f && global::screen.bottom > 0) ? ((float)global::screen.bottom * 50.0f / dist) : 400.0f;
					max_screen.y = screen.y - estimated_height;
				}
				else if (!w2s_origin && w2s_head)
				{
					screen.x = max_screen.x;
					float dist = (global::local) ? (origin - global::local->get_origin()).Length() : 400.0f;
					float estimated_height = (dist > 0.1f && global::screen.bottom > 0) ? ((float)global::screen.bottom * 50.0f / dist) : 400.0f;
					screen.y = max_screen.y + estimated_height;
				}

				height = (int)(screen.y - max_screen.y);
				width = height / 4;

				left = (int)(screen.x - width);
				right = (int)(screen.x + width);
				top = (int)max_screen.y;
				bottom = (int)screen.y;
				centerx = (int)screen.x;
			}
			else
			{
				if (!w2s(origin, screen) || !w2s(max_origin, max_screen))
					return false;

				left = (int)min(screen.x, max_screen.x) - 5;
				right = (int)max(screen.x, max_screen.x) + 5;
				top = (int)min(screen.y, max_screen.y) - 5;
				bottom = (int)max(screen.y, max_screen.y) + 5;

				width = right - left;
				height = bottom - top;

				centerx = left + (width / 2);
			}

			return true;
		}

		bool visible(centity* entity, matrix3x4_t matrix[128] = nullptr)
		{
			if (!entity) return false;
			if (!is_hitbox_visible(entity, hitbox_head, matrix) && 
				!is_hitbox_visible(entity, hitbox_stomach, matrix))
				return false;

			return true;
		}

		cvector origin, max_origin;
		int class_id, width, height, left, right, top, bottom, centerx;
	};

	void circle_3d(cvector pos, float radius, float resolution, color col)
	{
		if (resolution <= 0.0f) return;
		const float j = (float)M_PI * 2.f / resolution;
		for (float i = 0.f; i < (float)M_PI * 2.f; i += j)
		{
			float x1, y1, x2, y2;
			sincos(i, &y1, &x1);
			sincos(i + j, &y2, &x2);

			cvector screen1, screen2;
			if (!w2s(cvector(pos.x + x1 * radius, pos.y + y1 * radius, pos.z), screen1))
				continue;

			if (!w2s(cvector(pos.x + x2 * radius, pos.y + y2 * radius, pos.z), screen2))
				continue;

			surf::prim::line((int)screen1.x, (int)screen1.y, (int)screen2.x, (int)screen2.y, col);
		}
	}

	struct trail_node {
		cvector pos;
		float time;
	};
	inline std::deque<trail_node> local_trail;

	struct bullet_tracer_t {
		cvector start;
		cvector end;
		float time;
		color col;
	};
	inline std::deque<bullet_tracer_t> bullet_tracers;

	struct damage_indicator_t {
		cvector pos;
		int damage;
		bool headshot;
		float time;
	};
	inline std::deque<damage_indicator_t> damage_popups;

	inline void add_bullet_tracer(const cvector& start, const cvector& end, color col)
	{
		bullet_tracers.push_back({ start, end, global::realtime, col });
	}

	inline void add_damage_indicator(const cvector& pos, int damage, bool headshot)
	{
		damage_popups.push_back({ pos, damage, headshot, global::realtime });
	}

	struct kill_fx_t {
		cvector pos;
		float time;
		int type;
		color col;
	};
	inline std::deque<kill_fx_t> kill_effects;

	inline void add_kill_effect(const cvector& pos, int type, color col)
	{
		kill_effects.push_back({ pos, global::realtime, type, col });
	}

	inline float screen_hit_time = 0.0f;
	inline void trigger_screen_hit_pulse()
	{
		screen_hit_time = global::realtime;
	}

	void draw_dynamic_trail()
	{
		if (!sets->visuals.rainbow_trail || sets->visuals.trail_mode == 0 || !global::local || !global::local->valid())
		{
			local_trail.clear();
			return;
		}

		cvector cur_pos = global::local->get_origin() + cvector(0.0f, 0.0f, 15.0f);
		if (local_trail.empty() || (local_trail.back().pos - cur_pos).Length() > 5.0f)
		{
			local_trail.push_back({ cur_pos, global::realtime });
		}

		float max_time = sets->visuals.trail_length * 0.06f;
		while (!local_trail.empty() && (global::realtime - local_trail.front().time) > max_time)
		{
			local_trail.pop_front();
		}

		if (local_trail.size() < 2) return;

		for (size_t i = 0; i < local_trail.size() - 1; i++)
		{
			cvector s1, s2;
			if (w2s(local_trail[i].pos, s1) && w2s(local_trail[i + 1].pos, s2))
			{
				float progress = (float)i / (float)local_trail.size();
				int alpha = (int)(progress * 255.0f);
				color col;

				switch (sets->visuals.trail_mode)
				{
				case 1: // Rainbow Wave
				default:
				{
					float hue = fmodf(global::realtime * (sets->visuals.rainbow_trail_speed > 0.0f ? sets->visuals.rainbow_trail_speed : 1.0f) + progress * 0.8f, 1.0f);
					col = color::from_hsv(hue, 1.0f, 1.0f, alpha);
					break;
				}
				case 2: // Electric Neon Cyan
				{
					col = color(0, (int)(200 + sinf(progress * 3.14f) * 55), 255, alpha);
					break;
				}
				case 3: // Inferno Fire (Red -> Orange -> Yellow)
				{
					int r = 255;
					int g = (int)(progress * 200.0f);
					int b = (int)(progress * 30.0f);
					col = color(r, g, b, alpha);
					break;
				}
				case 4: // Cyber Violet Plasma
				{
					int r = (int)(180 + sinf(progress * 3.14f) * 75);
					int g = 20;
					int b = 255;
					col = color(r, g, b, alpha);
					break;
				}
				}

				surf::prim::line((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, col);
				surf::prim::line((int)s1.x, (int)s1.y + 1, (int)s2.x, (int)s2.y + 1, col.with_alpha(alpha / 2));
			}
		}
	}

	void draw_3d_ring(cvector pos, float radius, color col)
	{
		const int points = 16;
		cvector scr_pts[16];
		bool valid[16];

		for (int i = 0; i < points; i++)
		{
			float a = (float)i * (2.0f * (float)M_PI / (float)points);
			cvector pt = pos + cvector(cosf(a) * radius, sinf(a) * radius, 0.0f);
			valid[i] = w2s(pt, scr_pts[i]);
		}

		for (int i = 0; i < points; i++)
		{
			int next = (i + 1) % points;
			if (valid[i] && valid[next])
			{
				surf::prim::line((int)scr_pts[i].x, (int)scr_pts[i].y, (int)scr_pts[next].x, (int)scr_pts[next].y, col);
			}
		}
	}

	void draw_3d_halo(cvector head_pos, color col, float radius)
	{
		float bob = sinf(global::realtime * 3.5f) * 1.5f;
		cvector center = head_pos + cvector(0.0f, 0.0f, 13.0f + bob);
		const int points = 20;
		cvector scr_pts[20];
		bool valid[20];

		float rot = global::realtime * 1.2f;
		for (int i = 0; i < points; i++)
		{
			float a = rot + (float)i * (2.0f * (float)M_PI / (float)points);
			cvector pt = center + cvector(cosf(a) * radius, sinf(a) * radius, sinf(a * 2.0f) * 0.8f);
			valid[i] = w2s(pt, scr_pts[i]);
		}

		for (int i = 0; i < points; i++)
		{
			int next = (i + 1) % points;
			if (valid[i] && valid[next])
			{
				surf::prim::line((int)scr_pts[i].x, (int)scr_pts[i].y, (int)scr_pts[next].x, (int)scr_pts[next].y, col);
				surf::prim::line((int)scr_pts[i].x, (int)scr_pts[i].y + 1, (int)scr_pts[next].x, (int)scr_pts[next].y + 1, col.with_alpha(150));
			}
		}
	}

	void draw_3d_devil_horns(cvector head_pos, color col, float size)
	{
		const int num_rings = 7;
		const int pts_per_ring = 6;
		float sc = size * 0.05f + 0.5f;

		for (int side = -1; side <= 1; side += 2)
		{
			float s = (float)side;
			cvector rings[num_rings][pts_per_ring];
			cvector s_rings[num_rings][pts_per_ring];
			bool v_rings[num_rings][pts_per_ring];

			for (int r = 0; r < num_rings; r++)
			{
				float frac = (float)r / (float)(num_rings - 1);
				float ring_radius = (3.2f * (1.0f - frac * 0.85f)) * sc;

				float cx = s * (5.5f + sinf(frac * 1.8f) * 6.5f) * sc;
				float cy = (2.0f - frac * 12.0f - sinf(frac * (float)M_PI) * 2.0f) * sc;
				float cz = (2.0f + frac * 14.0f + powf(frac, 2.0f) * 4.0f) * sc;

				for (int p = 0; p < pts_per_ring; p++)
				{
					float angle = (float)p * (2.0f * (float)M_PI / (float)pts_per_ring);
					float px = cx + cosf(angle) * ring_radius;
					float py = cy + sinf(angle) * (ring_radius * 0.85f);
					float pz = cz + sinf(angle * 2.0f) * 0.4f * sc;

					rings[r][p] = head_pos + cvector(px, py, pz + 4.0f);
					v_rings[r][p] = w2s(rings[r][p], s_rings[r][p]);
				}
			}

			cvector tip = head_pos + cvector(s * 12.2f * sc, -10.5f * sc, (21.0f * sc) + 4.0f);
			cvector s_tip;
			bool v_tip = w2s(tip, s_tip);

			for (int r = 0; r < num_rings; r++)
			{
				float frac = (float)r / (float)(num_rings - 1);
				int cr = min(255, (int)(30.0f + frac * (float)col.r));
				int cg = min(255, (int)(15.0f + frac * (float)col.g));
				int cb = min(255, (int)(20.0f + frac * (float)col.b));
				color ring_col(cr, cg, cb, col.a);

				for (int p = 0; p < pts_per_ring; p++)
				{
					int next_p = (p + 1) % pts_per_ring;
					if (v_rings[r][p] && v_rings[r][next_p])
						surf::prim::line((int)s_rings[r][p].x, (int)s_rings[r][p].y, (int)s_rings[r][next_p].x, (int)s_rings[r][next_p].y, ring_col);

					if (r < num_rings - 1 && v_rings[r][p] && v_rings[r + 1][p])
						surf::prim::line((int)s_rings[r][p].x, (int)s_rings[r][p].y, (int)s_rings[r + 1][p].x, (int)s_rings[r + 1][p].y, ring_col);
				}
			}

			if (v_tip)
			{
				for (int p = 0; p < pts_per_ring; p++)
				{
					if (v_rings[num_rings - 1][p])
						surf::prim::line((int)s_rings[num_rings - 1][p].x, (int)s_rings[num_rings - 1][p].y, (int)s_tip.x, (int)s_tip.y, col);
				}
				surf::prim::filled_box((int)s_tip.x - 2, (int)s_tip.y - 2, (int)s_tip.x + 3, (int)s_tip.y + 3, color(255, 255, 255, col.a));
			}
		}
	}

	void draw_3d_crown(cvector head_pos, color col, float size)
	{
		cvector center = head_pos + cvector(0.0f, 0.0f, 8.5f);
		const int points = 10;
		cvector base_pts[10];
		cvector peak_pts[5];
		cvector s_base[10];
		cvector s_peak[5];
		bool v_base[10], v_peak[5];

		float rot = global::realtime * 0.8f;
		for (int i = 0; i < points; i++)
		{
			float a = rot + (float)i * (2.0f * (float)M_PI / (float)points);
			base_pts[i] = center + cvector(cosf(a) * size, sinf(a) * size, 0.0f);
			v_base[i] = w2s(base_pts[i], s_base[i]);
		}

		for (int i = 0; i < 5; i++)
		{
			float a = rot + (float)i * (2.0f * (float)M_PI / 5.0f);
			peak_pts[i] = center + cvector(cosf(a) * (size * 1.15f), sinf(a) * (size * 1.15f), 6.5f);
			v_peak[i] = w2s(peak_pts[i], s_peak[i]);
		}

		// Connect base ring
		for (int i = 0; i < points; i++)
		{
			int next = (i + 1) % points;
			if (v_base[i] && v_base[next])
			{
				surf::prim::line((int)s_base[i].x, (int)s_base[i].y, (int)s_base[next].x, (int)s_base[next].y, col);
			}
		}

		// Connect crown spikes
		for (int i = 0; i < 5; i++)
		{
			int b_left = (i * 2);
			int b_right = (i * 2 + 1) % points;
			if (v_peak[i])
			{
				if (v_base[b_left]) surf::prim::line((int)s_base[b_left].x, (int)s_base[b_left].y, (int)s_peak[i].x, (int)s_peak[i].y, col);
				if (v_base[b_right]) surf::prim::line((int)s_base[b_right].x, (int)s_base[b_right].y, (int)s_peak[i].x, (int)s_peak[i].y, col);
				surf::prim::filled_box((int)s_peak[i].x - 2, (int)s_peak[i].y - 2, (int)s_peak[i].x + 3, (int)s_peak[i].y + 3, color(255, 255, 255, col.a));
			}
		}
	}

	void draw_3d_cat_ears(cvector head_pos, color col, float size)
	{
		for (int side = -1; side <= 1; side += 2)
		{
			float s = (float)side;
			cvector b_in = head_pos + cvector(s * 2.5f, 0.0f, 5.0f);
			cvector b_out = head_pos + cvector(s * 7.5f, 0.0f, 5.0f);
			cvector tip = head_pos + cvector(s * (6.0f + size * 0.2f), 0.0f, 6.0f + size);

			cvector s_in, s_out, s_tip;
			if (w2s(b_in, s_in) && w2s(b_out, s_out) && w2s(tip, s_tip))
			{
				surf::prim::line((int)s_in.x, (int)s_in.y, (int)s_tip.x, (int)s_tip.y, col);
				surf::prim::line((int)s_out.x, (int)s_out.y, (int)s_tip.x, (int)s_tip.y, col);
				surf::prim::line((int)s_in.x, (int)s_in.y, (int)s_out.x, (int)s_out.y, col);

				// Inner ear pink glow
				cvector in_tip = head_pos + cvector(s * (5.5f + size * 0.15f), 0.5f, 5.5f + size * 0.7f);
				cvector s_intip;
				if (w2s(in_tip, s_intip))
				{
					surf::prim::line((int)s_in.x, (int)s_in.y, (int)s_intip.x, (int)s_intip.y, color(255, 180, 200, col.a));
					surf::prim::line((int)s_out.x, (int)s_out.y, (int)s_intip.x, (int)s_intip.y, color(255, 180, 200, col.a));
				}
			}
		}
	}

	void draw_3d_energy_wings(cvector origin, qangle angles, color col, float size)
	{
		cvector spine = origin + cvector(0.0f, 0.0f, 42.0f);
		float s_w = (size / 30.0f) * 0.95f;
		float t = global::realtime * 3.2f;
		float flap = sinf(t);

		Vector forward, right, up;
		AngleVectors(angles, &forward, &right, &up);

		for (int side = -1; side <= 1; side += 2)
		{
			float s = (float)side;

			float flap_z1 = flap * 5.0f * s_w;
			float flap_z2 = flap * 12.0f * s_w;
			float flap_span = cosf(t) * 6.0f * s_w;

			// 3D Joint Offsets relative to spine bone in local player coordinates
			cvector rootL_local(0.0f, -4.0f, -5.0f * s_w);
			cvector joint1L_local(s * (18.0f * s_w), -8.0f, (16.0f * s_w) + flap_z1);
			cvector joint2L_local(s * (42.0f * s_w + flap_span), -14.0f, (12.0f * s_w) + flap_z2);

			cvector tip1L_local(s * (65.0f * s_w + flap_span * 1.2f), -20.0f, (30.0f * s_w) + flap_z2 * 1.3f);
			cvector tip2L_local(s * (58.0f * s_w + flap_span * 1.1f), -18.0f, (10.0f * s_w) + flap_z2 * 1.1f);
			cvector tip3L_local(s * (45.0f * s_w + flap_span * 0.9f), -14.0f, (-12.0f * s_w) + flap_z2 * 0.8f);

			// Transform to 3D World Space using player angles
			cvector w_root = spine + (right * rootL_local.x) + (forward * rootL_local.y) + (up * rootL_local.z);
			cvector w_j1 = spine + (right * joint1L_local.x) + (forward * joint1L_local.y) + (up * joint1L_local.z);
			cvector w_j2 = spine + (right * joint2L_local.x) + (forward * joint2L_local.y) + (up * joint2L_local.z);
			cvector w_t1 = spine + (right * tip1L_local.x) + (forward * tip1L_local.y) + (up * tip1L_local.z);
			cvector w_t2 = spine + (right * tip2L_local.x) + (forward * tip2L_local.y) + (up * tip2L_local.z);
			cvector w_t3 = spine + (right * tip3L_local.x) + (forward * tip3L_local.y) + (up * tip3L_local.z);

			cvector s_root, s_j1, s_j2, s_t1, s_t2, s_t3;
			if (w2s(w_root, s_root) && w2s(w_j1, s_j1) && w2s(w_j2, s_j2) &&
				w2s(w_t1, s_t1) && w2s(w_t2, s_t2) && w2s(w_t3, s_t3))
			{
				// Khung xương chính
				surf::prim::line((int)s_root.x, (int)s_root.y, (int)s_j1.x, (int)s_j1.y, col);
				surf::prim::line((int)s_j1.x, (int)s_j1.y, (int)s_t1.x, (int)s_t1.y, col);
				surf::prim::line((int)s_t1.x, (int)s_t1.y, (int)s_t2.x, (int)s_t2.y, col.with_alpha(220));
				surf::prim::line((int)s_t2.x, (int)s_t2.y, (int)s_t3.x, (int)s_t3.y, col.with_alpha(220));
				surf::prim::line((int)s_t3.x, (int)s_t3.y, (int)s_j2.x, (int)s_j2.y, col.with_alpha(200));
				surf::prim::line((int)s_j2.x, (int)s_j2.y, (int)s_j1.x, (int)s_j1.y, col.with_alpha(200));

				// Gân nan quạt bên trong
				surf::prim::line((int)s_j1.x, (int)s_j1.y, (int)s_t2.x, (int)s_t2.y, col.with_alpha(180));
				surf::prim::line((int)s_j1.x, (int)s_j1.y, (int)s_t3.x, (int)s_t3.y, col.with_alpha(180));

				// Móng vuốt nhọn phát sáng
				surf::prim::filled_box((int)s_t1.x - 2, (int)s_t1.y - 2, (int)s_t1.x + 3, (int)s_t1.y + 3, color(255, 255, 255, col.a));
				surf::prim::filled_box((int)s_t2.x - 2, (int)s_t2.y - 2, (int)s_t2.x + 3, (int)s_t2.y + 3, color(255, 255, 255, col.a));
				surf::prim::filled_box((int)s_t3.x - 2, (int)s_t3.y - 2, (int)s_t3.x + 3, (int)s_t3.y + 3, color(255, 255, 255, col.a));
			}
		}
	}

	void draw_3d_magic_circle(cvector origin, color col, float radius)
	{
		cvector ground = origin + cvector(0.0f, 0.0f, 2.0f);
		const int points = 24;
		cvector out_pts[24], mid_pts[24], in_pts[24];
		cvector s_out[24], s_mid[24], s_in[24];
		bool v_out[24], v_mid[24], v_in[24];

		float rot1 = global::realtime * 0.9f;
		float rot2 = -global::realtime * 1.3f;

		for (int i = 0; i < points; i++)
		{
			float a1 = rot1 + (float)i * (2.0f * (float)M_PI / (float)points);
			float a2 = rot2 + (float)i * (2.0f * (float)M_PI / (float)points);
			out_pts[i] = ground + cvector(cosf(a1) * radius, sinf(a1) * radius, 0.0f);
			mid_pts[i] = ground + cvector(cosf(a1) * (radius * 0.90f), sinf(a1) * (radius * 0.90f), 0.0f);
			in_pts[i] = ground + cvector(cosf(a2) * (radius * 0.65f), sinf(a2) * (radius * 0.65f), 0.0f);

			v_out[i] = w2s(out_pts[i], s_out[i]);
			v_mid[i] = w2s(mid_pts[i], s_mid[i]);
			v_in[i] = w2s(in_pts[i], s_in[i]);
		}

		for (int i = 0; i < points; i++)
		{
			int next = (i + 1) % points;
			if (v_out[i] && v_out[next]) surf::prim::line((int)s_out[i].x, (int)s_out[i].y, (int)s_out[next].x, (int)s_out[next].y, col);
			if (v_mid[i] && v_mid[next]) surf::prim::line((int)s_mid[i].x, (int)s_mid[i].y, (int)s_mid[next].x, (int)s_mid[next].y, col.with_alpha(150));
			if (v_in[i] && v_in[next]) surf::prim::line((int)s_in[i].x, (int)s_in[i].y, (int)s_in[next].x, (int)s_in[next].y, col.with_alpha(200));

			// Radial Rune Spoke Ticks on Outer Border
			if (v_out[i] && v_mid[i] && (i % 2 == 0))
			{
				surf::prim::line((int)s_out[i].x, (int)s_out[i].y, (int)s_mid[i].x, (int)s_mid[i].y, col);
			}
		}

		// Sacred Octagram (Dual Rotating Squares)
		cvector oct_pts[8];
		cvector s_oct[8];
		bool v_oct[8];
		for (int i = 0; i < 8; i++)
		{
			float a = rot2 + (float)i * (2.0f * (float)M_PI / 8.0f);
			oct_pts[i] = ground + cvector(cosf(a) * (radius * 0.65f), sinf(a) * (radius * 0.65f), 0.0f);
			v_oct[i] = w2s(oct_pts[i], s_oct[i]);
		}

		for (int i = 0; i < 8; i++)
		{
			int next = (i + 2) % 8;
			if (v_oct[i] && v_oct[next])
			{
				surf::prim::line((int)s_oct[i].x, (int)s_oct[i].y, (int)s_oct[next].x, (int)s_oct[next].y, col.with_alpha(210));
			}
		}
	}

	void draw_bullet_tracers_and_impacts()
	{
		while (!bullet_tracers.empty() && (global::realtime - bullet_tracers.front().time) > sets->visuals.bullet_tracers_duration)
		{
			bullet_tracers.pop_front();
		}

		for (auto& bt : bullet_tracers)
		{
			float age = global::realtime - bt.time;
			float progress = 1.0f - (age / (sets->visuals.bullet_tracers_duration > 0.1f ? sets->visuals.bullet_tracers_duration : 2.5f));
			if (progress <= 0.0f) continue;

			int alpha = (int)(progress * 255.0f);
			cvector s1, s2;
			if (sets->visuals.bullet_tracers && w2s(bt.start, s1) && w2s(bt.end, s2))
			{
				color col = bt.col.with_alpha(alpha);
				surf::prim::line((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, col);
				surf::prim::line((int)s1.x, (int)s1.y + 1, (int)s2.x, (int)s2.y + 1, col.with_alpha(alpha / 2));
			}

			if (sets->visuals.impact_rings)
			{
				float ring_rad = (1.0f - progress) * 22.0f;
				draw_3d_ring(bt.end, ring_rad, sets->visuals.impact_rings_color.with_alpha(alpha));
			}
		}
	}

	void draw_damage_indicators()
	{
		if (!sets->visuals.damage_indicator)
		{
			damage_popups.clear();
			return;
		}

		while (!damage_popups.empty() && (global::realtime - damage_popups.front().time) > 2.0f)
		{
			damage_popups.pop_front();
		}

		for (auto& dp : damage_popups)
		{
			float age = global::realtime - dp.time;
			float progress = 1.0f - (age / 2.0f);
			if (progress <= 0.0f) continue;

			cvector float_pos = dp.pos + cvector(0.0f, 0.0f, age * 35.0f);
			cvector screen_pos;
			if (w2s(float_pos, screen_pos))
			{
				int alpha = (int)(progress * 255.0f);
				string txt = (dp.headshot ? "HEAD! -" : "-") + to_string(dp.damage);
				color col = dp.headshot ? color(255, 30, 30, alpha) : sets->visuals.damage_indicator_color.with_alpha(alpha);
				surf::font::draw(surf::font::esp, (int)screen_pos.x, (int)screen_pos.y, col, DT_CENTER | DT_VCENTER, txt.c_str());
			}
		}
	}

	void draw_kill_effects()
	{
		if (sets->visuals.kill_effect == 0)
		{
			kill_effects.clear();
			return;
		}

		while (!kill_effects.empty() && (global::realtime - kill_effects.front().time) > 2.5f)
		{
			kill_effects.pop_front();
		}

		for (auto& k : kill_effects)
		{
			float age = global::realtime - k.time;
			float progress = 1.0f - (age / 2.5f);
			if (progress <= 0.0f) continue;
			int alpha = (int)(progress * 255.0f);
			color c = k.col.with_alpha(alpha);

			if (k.type == 1) // 3D Lightning Strike down from sky
			{
				cvector ground = k.pos;
				cvector sky = k.pos + cvector(0.0f, 0.0f, 400.0f);
				int segments = 8;
				cvector prev = sky;
				for (int s = 1; s <= segments; s++)
				{
					float f = (float)s / (float)segments;
					float jx = (s < segments) ? sinf(age * 50.0f + s * 1.7f) * 16.0f : 0.0f;
					float jy = (s < segments) ? cosf(age * 40.0f + s * 2.3f) * 16.0f : 0.0f;
					cvector curr = sky + (ground - sky) * f + cvector(jx, jy, 0.0f);

					cvector sp, sc;
					if (w2s(prev, sp) && w2s(curr, sc))
					{
						surf::prim::line((int)sp.x, (int)sp.y, (int)sc.x, (int)sc.y, color(255, 255, 255, alpha));
						surf::prim::line((int)sp.x + 1, (int)sp.y, (int)sc.x + 1, (int)sc.y, c);
					}
					prev = curr;
				}
				draw_3d_ring(ground, (1.0f - progress) * 45.0f, c);
			}
			else if (k.type == 2) // 3D Particle Spark / Blood Fountain
			{
				for (int p = 0; p < 16; p++)
				{
					float a = (float)p * (2.0f * (float)M_PI / 16.0f);
					float spd = 25.0f + (float)(p % 5) * 8.0f;
					cvector part_pos = k.pos + cvector(cosf(a) * spd * age, sinf(a) * spd * age, age * 60.0f - 0.5f * 98.0f * age * age);
					cvector scr;
					if (w2s(part_pos, scr))
					{
						surf::prim::filled_box((int)scr.x - 1, (int)scr.y - 1, (int)scr.x + 2, (int)scr.y + 2, c);
					}
				}
			}
			else if (k.type == 3) // Ascending Soul / Skull Rising
			{
				cvector soul_pos = k.pos + cvector(0.0f, 0.0f, age * 40.0f);
				cvector scr;
				if (w2s(soul_pos, scr))
				{
					draw_3d_halo(soul_pos, c, 10.0f);
					surf::font::draw(surf::font::esp, (int)scr.x, (int)scr.y, color(255, 255, 255, alpha), DT_CENTER | DT_VCENTER, "☠ SOUL");
				}
			}
			else if (k.type == 4) // Cyber Implosion Rings
			{
				draw_3d_ring(k.pos, age * 50.0f, c);
				draw_3d_ring(k.pos + cvector(0.0f, 0.0f, 20.0f), age * 35.0f, color(255, 255, 255, alpha));
			}
		}
	}

	void draw_laser_sight()
	{
		if (!sets->visuals.laser_sight || !global::local || !global::local->valid() || !_engine) return;

		qangle va;
		_engine->get_viewangles(va);
		cvector fwd;
		angle_vectors(va, &fwd);

		cvector eye = global::local->get_eye_pos();
		float max_len = sets->visuals.laser_sight_length > 100.0f ? sets->visuals.laser_sight_length : 1500.0f;
		cvector end = eye + fwd * max_len;

		if (_engine_trace)
		{
			ray_t ray;
			ray.Init(eye, end);
			trace_t tr;
			itracefilter filter;
			filter.skip = global::local;
			_engine_trace->trace_ray(ray, MASK_SHOT_CSS, &filter, &tr);
			end = tr.endpos;
		}

		cvector s_start, s_end;
		if (w2s(eye + cvector(0.0f, 0.0f, -4.0f), s_start) && w2s(end, s_end))
		{
			surf::prim::line((int)s_start.x, (int)s_start.y, (int)s_end.x, (int)s_end.y, sets->visuals.laser_sight_color);
			surf::prim::line((int)s_start.x, (int)s_start.y + 1, (int)s_end.x, (int)s_end.y + 1, sets->visuals.laser_sight_color.with_alpha(120));
			surf::prim::filled_box((int)s_end.x - 3, (int)s_end.y - 3, (int)s_end.x + 4, (int)s_end.y + 4, color(255, 255, 255, 255));
		}
	}

	void draw_screen_hit_pulse()
	{
		if (!sets->visuals.screen_hit_pulse || screen_hit_time == 0.0f) return;
		float age = global::realtime - screen_hit_time;
		if (age > 0.4f) return;

		float progress = 1.0f - (age / 0.4f);
		int alpha = (int)(progress * 80.0f);
		color col = sets->visuals.screen_hit_pulse_color.with_alpha(alpha);

		int w = global::screen.right;
		int h = global::screen.bottom;
		int border = 8;
		surf::prim::filled_box(0, 0, w, border, col);
		surf::prim::filled_box(0, h - border, w, h, col);
		surf::prim::filled_box(0, 0, border, h, col);
		surf::prim::filled_box(w - border, 0, w, h, col);
	}

	void draw_item_sky_beam(cvector pos, color col)
	{
		cvector top = pos + cvector(0.0f, 0.0f, 180.0f);
		cvector s_bottom, s_top;
		if (w2s(pos, s_bottom) && w2s(top, s_top))
		{
			surf::prim::line((int)s_bottom.x, (int)s_bottom.y, (int)s_top.x, (int)s_top.y, col);
			surf::prim::line((int)s_bottom.x - 1, (int)s_bottom.y, (int)s_top.x - 1, (int)s_top.y, col.with_alpha(140));
			surf::prim::line((int)s_bottom.x + 1, (int)s_bottom.y, (int)s_top.x + 1, (int)s_top.y, col.with_alpha(140));
			draw_3d_ring(pos, 16.0f + sinf(global::realtime * 4.0f) * 3.0f, col);
		}
	}

	void draw_oof_arrow(centity* enemy, color col)
	{
		if (!enemy || !global::local || !global::local->valid()) return;

		cvector local_pos = global::local->get_eye_pos();
		cvector enemy_pos = enemy->get_abs_origin() + cvector(0, 0, 40.0f);

		qangle view_angles;
		_engine->get_viewangles(view_angles);

		qangle aim_ang = calc_angle(local_pos, enemy_pos);
		float yaw = DEG2RAD(view_angles.y - aim_ang.y - 90.0f);

		int center_x = global::screen.right / 2;
		int center_y = global::screen.bottom / 2;
		float radius = sets->visuals.oof_radius > 10.0f ? sets->visuals.oof_radius : 140.0f;
		float size = sets->visuals.oof_size > 5.0f ? sets->visuals.oof_size : 16.0f;

		float cx = (float)center_x + radius * cosf(yaw);
		float cy = (float)center_y + radius * sinf(yaw);

		float point_angle = yaw;
		float p1_x = cx + size * cosf(point_angle);
		float p1_y = cy + size * sinf(point_angle);

		float p2_x = cx + (size * 0.6f) * cosf(point_angle + 2.4f);
		float p2_y = cy + (size * 0.6f) * sinf(point_angle + 2.4f);

		float p3_x = cx + (size * 0.6f) * cosf(point_angle - 2.4f);
		float p3_y = cy + (size * 0.6f) * sinf(point_angle - 2.4f);

		surf::prim::line((int)p1_x, (int)p1_y, (int)p2_x, (int)p2_y, col);
		surf::prim::line((int)p2_x, (int)p2_y, (int)p3_x, (int)p3_y, col);
		surf::prim::line((int)p3_x, (int)p3_y, (int)p1_x, (int)p1_y, col);

		float dist = (enemy_pos - local_pos).Length() * 0.01905f;
		surf::font::draw(surf::font::esp, (int)cx, (int)cy + 10, color(230, 230, 230), DT_CENTER, "%.0fm", dist);
	}

	static const int s_hat_points = 24;
	static const float s_hat_cos[24] = {
		1.000000f, 0.965926f, 0.866025f, 0.707107f, 0.500000f, 0.258819f,
		0.000000f, -0.258819f, -0.500000f, -0.707107f, -0.866025f, -0.965926f,
		-1.000000f, -0.965926f, -0.866025f, -0.707107f, -0.500000f, -0.258819f,
		0.000000f, 0.258819f, 0.500000f, 0.707107f, 0.866025f, 0.965926f
	};
	static const float s_hat_sin[24] = {
		0.000000f, 0.258819f, 0.500000f, 0.707107f, 0.866025f, 0.965926f,
		1.000000f, 0.965926f, 0.866025f, 0.707107f, 0.500000f, 0.258819f,
		0.000000f, -0.258819f, -0.500000f, -0.707107f, -0.866025f, -0.965926f,
		-1.000000f, -0.965926f, -0.866025f, -0.707107f, -0.500000f, -0.258819f
	};

	void draw_sounds()
	{
		if (server::sounds.empty()) return;

		auto it = server::sounds.begin();
		while (it != server::sounds.end())
		{
			float delta = fabsf(it->time - global::curtime);
			if (delta > 2.0f)
			{
				it = server::sounds.erase(it);
				continue;
			}

			float radius = (sets->visuals.footstep_rings ? (10.0f + delta * 45.0f) : (5.0f + delta * 15.0f));
			int alpha_val = (int)(255.0f - (delta * 0.5f) * 255.0f);
			if (alpha_val < 0) alpha_val = 0;

			circle_3d(it->position, radius, 16.0f, it->col.with_alpha(alpha_val));
			
			if (sets->visuals.sound_esp)
			{
				cvector screen;
				if (w2s(it->position + cvector(0, 0, 15.0f), screen))
				{
					surf::font::draw(surf::font::esp, (int)screen.x, (int)screen.y, it->col.with_alpha(alpha_val), DT_CENTER, "[STEP]");
				}
			}
			++it;
		}
	}

	template<typename t>
	void bar(bool vertical, bool text_on_left, int left, int top, int right, int bottom, float value, float vmax, color front, color back, const char* output = "%i")
	{
		if (vmax <= 0.0f) vmax = 1.0f;
		float clamped_val = (value < 0.0f) ? 0.0f : ((value > vmax) ? vmax : value);

		surf::prim::filled_box(left - 1, top - 1, right + 1, bottom + 1, color::outline().with_alpha(front.a));
		surf::prim::filled_box(left, top, right, bottom, back);

		int v_calc = static_cast<int>(bottom - ((bottom - top) * (clamped_val / vmax)));
		int h_calc = static_cast<int>(left + ((right - left) * (clamped_val / vmax)));
		const auto amount = vertical ? (top > v_calc ? top : v_calc) : (right < h_calc ? right : h_calc);

		surf::prim::filled_box(left, vertical ? amount : top, vertical ? right : amount, bottom, front);
		if (value != vmax)
		{
			surf::font::draw(surf::font::esp, vertical ? (text_on_left ? left - 1 : right + 1) : amount, vertical ? amount : bottom + 2, value < vmax ? front + 20 : color(255, 0, 0).with_alpha(front.a), vertical ? (text_on_left ? DT_RIGHT : NULL) | DT_VCENTER : DT_CENTER, output, (t)value);
		}
	}

	template<typename t>
	t interpolate(t base, t to, t factor)
	{
		if (abs(base - to) > factor) return ((to > base) ? (base + factor) : (base - factor));
		else return to;
	}

	void draw()
	{
		if (!sets->visuals.enabled || !_engine || !_engine->in_game() || !_ent_list)
			return;

		if (sets->visuals.sound_esp || sets->visuals.footstep_rings || (sets->visuals.esp_filter[0] && sets->visuals.esp_show[4]))
			draw_sounds();

		draw_dynamic_trail();
		draw_bullet_tracers_and_impacts();
		draw_damage_indicators();
		draw_kill_effects();
		draw_laser_sight();
		draw_screen_hit_pulse();

		centity* entity = nullptr;
		iclientnetworkable* networkable = nullptr;
		clientclass* client_class = nullptr;
		int class_id = 0;
		bool dormant = false;
		const char* name = nullptr;
		matrix3x4_t matrix[128];
		cbox box;
		color p_color;
		static short alpha[65];

		bool world_filters = sets->visuals.esp_filter[1] || sets->visuals.esp_filter[2] || sets->visuals.esp_filter[3] || sets->visuals.esp_filter[4] || sets->visuals.esp_filter[5] || sets->visuals.bomb_timer;
		int max_clients = _engine->get_max_clients();
		int highest_ent = _ent_list->get_highest_entity_index();
		int max_ents = world_filters ? ((highest_ent + 1 < 512) ? (highest_ent + 1) : 512) : ((max_clients + 1 < 65) ? (max_clients + 1) : 65);
		if (max_ents <= 0) return;

		bool local_alive = (global::local && global::local->valid());
		int local_team = (global::local ? global::local->get_team() : 0);

		for (int id = 0; id < max_ents; id++)
		{
			if (id == global::local_id && !sets->visuals.thirdperson && local_alive) continue;			

			entity = _ent_list->get_centity(id);
			if (!entity || entity->get_origin().IsZero() || entity == global::local_observed) continue;

			networkable = entity->get_clientnetworkable();
			if (!networkable) continue;

			dormant = networkable->is_dormant();
			if ((!sets->visuals.fade || id >= 64) && dormant) continue;

			client_class = networkable->get_clientclass();
			if (!client_class) continue;

			class_id = client_class->class_id;
			name = client_class->name;
			if (!name) continue;

			int alpha_idx = (id >= 0 && id < 64) ? id : 64;

			bool is_teammate = (local_alive && local_team > 1 && entity->get_team() == local_team);

			if ((class_id == CPlantedC4 && ((!sets->visuals.esp_filter[3] && !sets->visuals.bomb_timer) || !events::bomb_timer::planted)) || (class_id == CCSPlayer && ((!sets->visuals.esp_filter[0] && !sets->visuals.asian_hat && !sets->visuals.offscreen_esp) || !entity->valid() || (!sets->visuals.friends && id != global::local_id && is_teammate && !sets->visuals.asian_hat))))
			{
				alpha[alpha_idx] = 0;
				continue;
			}

			switch (class_id)
			{
			case CCSPlayer:
			{
				bool has_matrix = entity->get_hitbox_matrix(matrix, global::curtime);
				bool visible = has_matrix && box.visible(entity, matrix);
				alpha[alpha_idx] = sets->visuals.fade ? interpolate<int>(alpha[alpha_idx], ((!sets->visuals.esp_check[0] && !visible) || dormant) ? 0 : 255, 7) : 255;
				if (alpha[alpha_idx] == 0 && dormant && id != global::local_id) continue;
				if (!sets->visuals.esp_check[0] && !visible && (!sets->visuals.fade || alpha[alpha_idx] == 0) && id != global::local_id) continue;
				
				bool is_enemy = (!local_alive || local_team <= 1 || entity->get_team() != local_team);
				p_color = visible ? (entity->get_team() == 2 ? sets->visuals.esp_t : sets->visuals.esp_ct) : (is_enemy ? (entity->get_team() == 2 ? sets->visuals.esp_t.with_alpha(180) : sets->visuals.esp_ct.with_alpha(180)) : color::disabled());

				// 3D Head Accessories & Attachments (Unified Combo Selection)
				int acc_mode = sets->visuals.head_accessory;
				if (acc_mode == 0)
				{
					if (sets->visuals.asian_hat) acc_mode = 1;
					else if (sets->visuals.halo_ring) acc_mode = 2;
					else if (sets->visuals.devil_horns) acc_mode = 3;
					else if (sets->visuals.crown) acc_mode = 4;
					else if (sets->visuals.cat_ears) acc_mode = 5;
				}

				if (acc_mode > 0 && entity->valid())
				{
					bool allow_hat = (id == global::local_id) ? sets->visuals.thirdperson : (!is_teammate || sets->visuals.friends);
					if (allow_hat)
					{
						cvector head_pos = has_matrix ? entity->get_hitbox(hitbox_head, matrix) : (entity->get_eye_pos() + cvector(0, 0, 4.0f));
						if (!head_pos.IsZero())
						{
							color acc_col = sets->visuals.head_accessory_color;
							float acc_sz = sets->visuals.head_accessory_size;

							if (acc_mode == 1) // Asian Rice Hat (Vietnamese Non La)
							{
								head_pos.z += 6.5f;
								cvector apex = head_pos + cvector(0.0f, 0.0f, sets->visuals.head_accessory_height);
								cvector screen_apex;
								bool apex_valid = w2s(apex, screen_apex);

								cvector rim_screens[24];
								bool rim_valid[24];

								for (int i = 0; i < s_hat_points; i++)
								{
									cvector rim_pos = head_pos + cvector(s_hat_cos[i] * acc_sz, s_hat_sin[i] * acc_sz, 0.0f);
									rim_valid[i] = w2s(rim_pos, rim_screens[i]);
								}

								int hat_alpha = (id == global::local_id) ? 255 : (alpha[alpha_idx] > 0 ? alpha[alpha_idx] : 255);
								color base_col = acc_col.with_alpha(hat_alpha);
								color ring_col = color(185, 160, 115, (int)(hat_alpha * 0.8f));

								for (int i = 0; i < s_hat_points; i++)
								{
									int next_i = (i + 1) % s_hat_points;

									if (rim_valid[i] && rim_valid[next_i])
									{
										// Bamboo Outer Rim
										surf::prim::line((int)rim_screens[i].x, (int)rim_screens[i].y, (int)rim_screens[next_i].x, (int)rim_screens[next_i].y, base_col);

										// Radial Palm Leaf Stitch Seams
										if (apex_valid)
										{
											surf::prim::line((int)rim_screens[i].x, (int)rim_screens[i].y, (int)screen_apex.x, (int)screen_apex.y, base_col.with_alpha((int)(hat_alpha * 0.7f)));
										}

										// Concentric Bamboo Rings
										if (apex_valid)
										{
											for (int r = 1; r <= 3; r++)
											{
												float frac = (float)r / 4.0f;
												int rx1 = (int)(screen_apex.x + (rim_screens[i].x - screen_apex.x) * frac);
												int ry1 = (int)(screen_apex.y + (rim_screens[i].y - screen_apex.y) * frac);
												int rx2 = (int)(screen_apex.x + (rim_screens[next_i].x - screen_apex.x) * frac);
												int ry2 = (int)(screen_apex.y + (rim_screens[next_i].y - screen_apex.y) * frac);
												surf::prim::line(rx1, ry1, rx2, ry2, ring_col);
											}
										}
									}
								}

								// Traditional Silk Chin Strap (Quai Non)
								cvector chin_pos = head_pos - cvector(0.0f, 0.0f, 6.0f);
								cvector s_chin;
								if (w2s(chin_pos, s_chin) && rim_valid[6] && rim_valid[18])
								{
									color silk_col = color(235, 55, 105, hat_alpha);
									surf::prim::line((int)rim_screens[6].x, (int)rim_screens[6].y, (int)s_chin.x, (int)s_chin.y, silk_col);
									surf::prim::line((int)rim_screens[18].x, (int)rim_screens[18].y, (int)s_chin.x, (int)s_chin.y, silk_col);
								}

								if (apex_valid)
								{
									surf::prim::filled_box((int)screen_apex.x - 2, (int)screen_apex.y - 2, (int)screen_apex.x + 3, (int)screen_apex.y + 3, color(235, 215, 175, hat_alpha));
								}
							}
							else if (acc_mode == 2) // Angel Halo Ring
							{
								draw_3d_halo(head_pos, acc_col, acc_sz * 0.6f);
							}
							else if (acc_mode == 3) // Devil Horns
							{
								draw_3d_devil_horns(head_pos, acc_col, acc_sz * 0.5f);
							}
							else if (acc_mode == 4) // Royal Crown
							{
								draw_3d_crown(head_pos, acc_col, acc_sz * 0.7f);
							}
							else if (acc_mode == 5) // Cyber Cat Ears
							{
								draw_3d_cat_ears(head_pos, acc_col, acc_sz * 0.5f);
							}
						}
					}
				}

				// 3D Energy Angel/Demon Wings
				if (sets->visuals.energy_wings && entity->valid())
				{
					bool allow_acc = (id == global::local_id) ? sets->visuals.thirdperson : (!is_teammate || sets->visuals.friends);
					if (allow_acc)
					{
						qangle ang = entity->get_angles();
						if (id == global::local_id)
						{
							_engine->get_viewangles(ang);
						}
						cvector pos = entity->get_abs_origin();
						draw_3d_energy_wings(pos, ang, sets->visuals.energy_wings_color, sets->visuals.energy_wings_size);
					}
				}

				// 3D Magic Circle Runes (Ground)
				if (sets->visuals.magic_circle && entity->valid())
				{
					bool allow_acc = (id == global::local_id) ? sets->visuals.thirdperson : (!is_teammate || sets->visuals.friends);
					if (allow_acc)
					{
						cvector pos = entity->get_abs_origin();
						draw_3d_magic_circle(pos, sets->visuals.magic_circle_color, sets->visuals.magic_circle_size);
					}
				}

				// Skeleton ESP (Xương người)
				if (sets->visuals.skeleton && entity->valid() && has_matrix)
				{
					static const int bones[][2] = {
						{ hitbox_head, hitbox_neck },
						{ hitbox_neck, hitbox_upper_chest },
						{ hitbox_upper_chest, hitbox_chest },
						{ hitbox_chest, hitbox_pelvis },
						{ hitbox_upper_chest, hitbox_l_up_arm },
						{ hitbox_l_up_arm, hitbox_l_low_arm },
						{ hitbox_upper_chest, hitbox_r_up_arm },
						{ hitbox_r_up_arm, hitbox_r_low_arm },
						{ hitbox_pelvis, hitbox_l_up_leg },
						{ hitbox_l_up_leg, hitbox_l_low_leg },
						{ hitbox_pelvis, hitbox_r_up_leg },
						{ hitbox_r_up_leg, hitbox_r_low_leg }
					};

					int skel_alpha = (id == global::local_id) ? 255 : (alpha[alpha_idx] > 0 ? alpha[alpha_idx] : 255);
					color skel_col = p_color.with_alpha(skel_alpha);
					for (int b = 0; b < 12; b++)
					{
						cvector p1_3d = entity->get_hitbox(bones[b][0], matrix);
						cvector p2_3d = entity->get_hitbox(bones[b][1], matrix);
						cvector p1_2d, p2_2d;
						if (!p1_3d.IsZero() && !p2_3d.IsZero() && w2s(p1_3d, p1_2d) && w2s(p2_3d, p2_2d))
						{
							surf::prim::line((int)p1_2d.x, (int)p1_2d.y, (int)p2_2d.x, (int)p2_2d.y, skel_col);
						}
					}
				}

				box = cbox(entity, CCSPlayer);
				if (!box.construct_points())
				{
					if (sets->visuals.offscreen_esp && id != global::local_id)
					{
						draw_oof_arrow(entity, p_color);
					}
					continue;
				}

				player_info_t info;
				if (sets->visuals.esp_show[0] && _engine->get_playerinfo(id, &info))
				{
					surf::font::draw(surf::font::esp, box.centerx, box.top - 2, color::ptext().with_alpha(alpha[alpha_idx]), DT_CENTER | DT_BOTTOM, str(info.name).substr(0, 25).c_str());
				}

				if (sets->visuals.esp_show[1])
				{
					surf::prim::bordered_box(box.left - 1, box.top - 1, box.right + 1, box.bottom + 1, color::outline().with_alpha(alpha[alpha_idx]));
					surf::prim::bordered_box(box.left, box.top, box.right, box.bottom, p_color.with_alpha(alpha[alpha_idx]));
					surf::prim::bordered_box(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, color::outline().with_alpha(alpha[alpha_idx]));
				}

				if (sets->visuals.esp_bar[0])
				{
					bar<int>(true, true, box.left - 8, box.top, box.left - 3, box.bottom, (float)entity->get_hp(), 100.f, color(0, 200, 0).with_alpha(alpha[alpha_idx]), color(0, 50, 0).with_alpha(alpha[alpha_idx]));
				}

				if (sets->visuals.esp_bar[1] && entity->get_armor() > 0)
				{
					bar<int>(true, false, box.right + 3, box.top, box.right + 8, box.bottom, (float)entity->get_armor(), 100.f, color::text().with_alpha(alpha[alpha_idx]) + 10, color::disabled().with_alpha(alpha[alpha_idx]));
				}

				if (sets->visuals.esp_show[2] || sets->visuals.esp_bar[2])
				{
					auto weapon = entity->get_weapon();
					if (weapon)
					{
						if (sets->visuals.esp_bar[2])
						{
							int ammo = weapon->get_clip1();
							int max_ammo = weapon->get_maxclip1();

							if (ammo != -1 && max_ammo > 0)
							{
								bar<int>(false, false, box.left, box.bottom + 3, box.right, box.bottom + 8, (float)ammo, (float)max_ammo, color(170, 170, 170).with_alpha(alpha[alpha_idx]), color(50, 50, 50).with_alpha(alpha[alpha_idx]), "%i");
								box.bottom += 8;
								if (ammo < max_ammo) box.bottom += 8;
							}
						}

						if (sets->visuals.esp_show[2])
						{
							surf::font::draw(surf::font::esp, box.centerx, box.bottom + 1, color::ptext().with_alpha(alpha[alpha_idx]), DT_CENTER, weapon->get_name().c_str());
						}
					}
				}

				// Player Flags Rendering
				int flag_y = box.top;
				int flag_x = box.right + (sets->visuals.esp_bar[1] ? 12 : 5);

				if (sets->visuals.flag_hk)
				{
					int armor = entity->get_armor();
					if (armor > 0)
					{
						surf::font::draw(surf::font::esp, flag_x, flag_y, color(0, 200, 255).with_alpha(alpha[alpha_idx]), NULL, "HK");
						flag_y += 11;
					}
				}

				if (entity->have_defuser())
				{
					surf::font::draw(surf::font::esp, flag_x, flag_y, color(50, 220, 50).with_alpha(alpha[alpha_idx]), NULL, "KIT");
					flag_y += 11;
				}

				if (sets->visuals.flag_flashed)
				{
					float flash_alpha = *(float*)((DWORD)entity + offsets::flash_max_alpha);
					float flash_duration = *(float*)((DWORD)entity + offsets::flash_duration);
					if (flash_duration > 0.5f || flash_alpha > 50.0f)
					{
						surf::font::draw(surf::font::esp, flag_x, flag_y, color(255, 230, 0).with_alpha(alpha[alpha_idx]), NULL, "BLIND");
						flag_y += 11;
					}
				}

				if (entity->get_flags() & FL_DUCKING)
				{
					surf::font::draw(surf::font::esp, flag_x, flag_y, color(200, 200, 200).with_alpha(alpha[alpha_idx]), NULL, "DUCK");
					flag_y += 11;
				}

				int money = *(int*)((DWORD)entity + offsets::account);
				if (money > 0)
				{
					surf::font::draw(surf::font::esp, flag_x, flag_y, color(120, 220, 80).with_alpha(alpha[alpha_idx]), NULL, "$%d", money);
					flag_y += 11;
				}

				if (legit::backtrack::draw && sets->legit.backtrack.style[1])
				{
					cvector screen(0, 0, 0);

					if (sets->legit.backtrack.style[3])
					{
						crecord record = records[id][min(13, max(0, (global::cmd->tick_count - sets->legit.backtrack.ticks + 1) % sets->legit.backtrack.ticks))];
						if (record.is_valid(entity) && record.have_matrix && record.hitbox_matrix && w2s(entity->get_hitbox(hitbox_head, record.hitbox_matrix), screen))
						{
							surf::prim::filled_box((int)screen.x - 3, (int)screen.y - 3, (int)screen.x + 3, (int)screen.y + 3, color::outline().with_alpha(alpha[alpha_idx]));
							surf::prim::filled_box((int)screen.x - 2, (int)screen.y - 2, (int)screen.x + 2, (int)screen.y + 2, color(255, 167, 25, alpha[alpha_idx]));
						}
					}

					if (sets->legit.backtrack.style[2] && best_record.id == id)
					{
						if (best_record.is_valid(entity) && best_record.have_matrix && best_record.hitbox_matrix && w2s(entity->get_hitbox(hitbox_head, best_record.hitbox_matrix), screen))
						{
							surf::prim::filled_box((int)screen.x - 3, (int)screen.y - 3, (int)screen.x + 3, (int)screen.y + 3, color::outline().with_alpha(alpha[alpha_idx]));
							surf::prim::filled_box((int)screen.x - 2, (int)screen.y - 2, (int)screen.x + 2, (int)screen.y + 2, color(30, 255, 30, alpha[alpha_idx]));
						}
					}
				}

				if (sets->visuals.esp_show[3])
				{
					surf::prim::line(global::screen.right / 2, global::screen.bottom / 2, box.centerx, box.bottom, p_color);
				}
				continue;
			}
			case CHEGrenade:
			case CSmokeGrenade:
			case CFlashbang:
			{
				if (!sets->visuals.esp_filter[2]) continue;
				
				box = cbox(entity, class_id);
				if (!box.construct_points()) continue;

				surf::font::draw(surf::font::esp, box.centerx, box.top - 2, class_id == CHEGrenade ? color(180, 0, 0) : class_id == CSmokeGrenade ? color::disabled() + 80 : color(253, 233, 16), DT_BOTTOM | DT_CENTER, class_id == CHEGrenade ? "he_grenade" : class_id == CSmokeGrenade ? "smoke_grenade" : "flashbang");

				surf::prim::bordered_box(box.left - 1, box.top - 1, box.right + 1, box.bottom + 1, color::outline());
				surf::prim::bordered_box(box.left, box.top, box.right, box.bottom, color::enabled());
				surf::prim::bordered_box(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, color::outline());
				continue;
			}
			case CBaseCSGrenadeProjectile:
			{
				if (!sets->visuals.esp_filter[3]) continue;

				auto model = entity->get_model();
				if (!model) continue;

				auto model_name = _model_info ? _model_info->get_model_name(model) : nullptr;
				if (!model_name) continue;

				bool is_smoke = strstr(model_name, "smoke") != nullptr;
				bool is_flash = strstr(model_name, "flash") != nullptr;
				bool is_he = strstr(model_name, "frag") != nullptr;

				box = cbox(entity, class_id);
				if (!box.construct_points()) continue;

				const color nade_color = is_flash ? color(253, 233, 16) : is_smoke ? color::disabled() + 80 : is_he ? color(200, 0, 0) : color::lm();

				surf::font::draw(surf::font::esp, box.centerx, box.top - 2, nade_color, DT_BOTTOM | DT_CENTER, is_flash ? "flashbang" : is_smoke ? "smoke_grenade" : is_he ? "hegrenade" : "??????");

				surf::prim::bordered_box(box.left - 1, box.top - 1, box.right + 1, box.bottom + 1, color::outline());
				surf::prim::bordered_box(box.left, box.top, box.right, box.bottom, nade_color);
				surf::prim::bordered_box(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, color::outline());
				continue;
			}
			case CC4:
			case CPlantedC4:
			{
				if (sets->visuals.item_light_beams)
				{
					draw_item_sky_beam(entity->get_origin(), sets->visuals.item_light_beams_color);
				}

				if (events::bomb_timer::explosion_time == 0.f)
				{
					events::bomb_timer::f_exp_time = entity->get_c4_timer_length();
					events::bomb_timer::explosion_time = entity->get_c4_blow_time();
				}

				if (!sets->visuals.esp_filter[4])
					continue;

				box = cbox(entity, class_id);
				if (!box.construct_points()) continue;

				surf::font::draw(surf::font::esp, box.centerx, box.top - 2, color(220, 0, 0), DT_CENTER | DT_BOTTOM, class_id == CC4 ? "bomb" : "planted_bomb");

				surf::prim::bordered_box(box.left - 1, box.top - 1, box.right + 1, box.bottom + 1, color::outline());
				surf::prim::bordered_box(box.left, box.top, box.right, box.bottom, class_id == CC4 ? color::enabled() : color(200, 0, 0));
				surf::prim::bordered_box(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, color::outline());

				if (class_id == CPlantedC4)
				{
					if (events::bomb_timer::defuse_time != 0.f && events::bomb_timer::f_def_time > 0.0f)
					{
						bar<float>(false, false, box.left, box.bottom + 21, box.right, box.bottom + 26, (events::bomb_timer::defuse_time - global::curtime), events::bomb_timer::f_def_time, color(0, 0, 200), color(0, 0, 50), (events::bomb_timer::defuse_time - global::curtime) > 0.f ? "%.1f" : "defused");
					}

					if (events::bomb_timer::f_exp_time > 0.0f)
					{
						bar<float>(false, false, box.left, box.bottom + 3, box.right, box.bottom + 8, (events::bomb_timer::explosion_time - global::curtime), events::bomb_timer::f_exp_time, color(200, 0, 0), color(50, 0, 0), (events::bomb_timer::explosion_time - global::curtime) > 0.f ? "%.1f" : "explode");
					}
				}
				continue;
			}
			case CBaseAnimating:
			{
				if (!sets->visuals.esp_filter[5] || (sets->visuals.defuser_only_if_need && (!global::local->valid() || global::local->get_team() != 3 || global::local->have_defuser())))
					continue;

				auto model = entity->get_model();
				if (!model) continue;

				auto model_name = _model_info ? _model_info->get_model_name(model) : nullptr;
				if (!model_name || !strstr(model_name, "defuse")) continue;

				box = cbox(entity, class_id);
				if (!box.construct_points()) continue;

				surf::font::draw(surf::font::esp, box.centerx, box.top - 2, color::ptext(), DT_BOTTOM | DT_CENTER, "defuser");

				surf::prim::bordered_box(box.left - 1, box.top - 1, box.right + 1, box.bottom + 1, color::outline());
				surf::prim::bordered_box(box.left, box.top, box.right, box.bottom, color::lm());
				surf::prim::bordered_box(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, color::outline());
				continue;
			}
			}

			if (strstr(name, "CWeapon") || class_id == CAK47 || class_id == CDEagle)
			{
				if (sets->visuals.item_light_beams)
				{
					draw_item_sky_beam(entity->get_origin(), sets->visuals.item_light_beams_color);
				}

				if (!sets->visuals.esp_filter[1]) continue;

				box = cbox(entity, class_id);
				if (!box.construct_points()) continue;

				string sname = name;
				sname = (class_id == CAK47 || class_id == CDEagle) ? sname.substr(1) : sname.substr(7);
				transform(sname.begin(), sname.end(), sname.begin(), change_case);

				surf::font::draw(surf::font::esp, box.centerx, box.top - 2, color::ptext(), DT_CENTER | DT_BOTTOM, sname.c_str());

				surf::prim::bordered_box(box.left - 1, box.top - 1, box.right + 1, box.bottom + 1, color::outline());
				surf::prim::bordered_box(box.left, box.top, box.right, box.bottom, color::enabled());
				surf::prim::bordered_box(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, color::outline());

				if (sets->visuals.esp_bar[3])
				{
					auto weapon = (cweapon*)entity;
					if (weapon)
					{
						int ammo = weapon->get_clip1();
						int max_ammo = weapon->get_maxclip1();

						if (ammo != -1 && max_ammo > 0)
							bar<int>(false, false, box.left, box.bottom + 3, box.right, box.bottom + 7, (float)ammo, (float)max_ammo, color(170, 170, 170), color(50, 50, 50), "%i");
					}
				}
				continue;
			}
		}
	}
}
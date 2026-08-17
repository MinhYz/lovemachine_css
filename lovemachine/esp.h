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

	void draw_rainbow_trail()
	{
		if (!sets->visuals.rainbow_trail || !global::local || !global::local->valid())
		{
			local_trail.clear();
			return;
		}

		cvector cur_pos = global::local->get_origin() + cvector(0.0f, 0.0f, 15.0f);
		if (local_trail.empty() || (local_trail.back().pos - cur_pos).Length() > 6.0f)
		{
			local_trail.push_back({ cur_pos, global::realtime });
		}

		while (!local_trail.empty() && (global::realtime - local_trail.front().time) > 1.8f)
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
				float hue = fmodf(global::realtime * (sets->visuals.rainbow_trail_speed > 0.0f ? sets->visuals.rainbow_trail_speed : 1.0f) + progress * 0.8f, 1.0f);
				int alpha = (int)(progress * 255.0f);
				color col = color::from_hsv(hue, 1.0f, 1.0f, alpha);
				surf::prim::line((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, col);
				surf::prim::line((int)s1.x, (int)s1.y + 1, (int)s2.x, (int)s2.y + 1, col);
			}
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

		if (!sets->visuals.asian_hat && !sets->visuals.bomb_timer && !sets->visuals.esp_filter[0] && !sets->visuals.esp_filter[1] && !sets->visuals.esp_filter[2] && !sets->visuals.esp_filter[3] && !sets->visuals.esp_filter[4] && !sets->visuals.esp_filter[5])
			return;

		if (sets->visuals.sound_esp || sets->visuals.footstep_rings || (sets->visuals.esp_filter[0] && sets->visuals.esp_show[4]))
			draw_sounds();

		draw_rainbow_trail();

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

				// Asian Hat 3D Conical Rice Hat (Applies to enemies, and teammates only if Draw Teammates is enabled)
				if (sets->visuals.asian_hat && entity->valid())
				{
					bool allow_hat = false;
					if (id == global::local_id)
					{
						allow_hat = sets->visuals.thirdperson;
					}
					else if (!is_teammate || sets->visuals.friends)
					{
						allow_hat = true;
					}

					if (allow_hat)
					{
						cvector head_pos(0, 0, 0);
						if (has_matrix)
						{
							head_pos = entity->get_hitbox(hitbox_head, matrix);
						}
						if (head_pos.IsZero())
						{
							head_pos = entity->get_eye_pos() + cvector(0, 0, 4.0f);
						}

						if (!head_pos.IsZero())
						{
							head_pos.z += 6.5f;
							cvector apex = head_pos + cvector(0.0f, 0.0f, sets->visuals.asian_hat_height);
							cvector screen_apex;
							bool apex_valid = w2s(apex, screen_apex);

							cvector rim_screens[24];
							bool rim_valid[24];

							for (int i = 0; i < s_hat_points; i++)
							{
								cvector rim_pos = head_pos + cvector(s_hat_cos[i] * sets->visuals.asian_hat_size, s_hat_sin[i] * sets->visuals.asian_hat_size, 0.0f);
								rim_valid[i] = w2s(rim_pos, rim_screens[i]);
							}

							int hat_alpha = (id == global::local_id) ? 255 : (alpha[alpha_idx] > 0 ? alpha[alpha_idx] : 255);
							color base_col = sets->visuals.asian_hat_color.with_alpha(hat_alpha);
							for (int i = 0; i < s_hat_points; i++)
							{
								int next_i = (i + 1) % s_hat_points;

								if (rim_valid[i] && rim_valid[next_i])
								{
									// Render 3D Conical Hat Rim & Ribs using Engine Surface
									surf::prim::line((int)rim_screens[i].x, (int)rim_screens[i].y, (int)rim_screens[next_i].x, (int)rim_screens[next_i].y, base_col);
									if (apex_valid)
									{
										surf::prim::line((int)rim_screens[i].x, (int)rim_screens[i].y, (int)screen_apex.x, (int)screen_apex.y, base_col);
									}
								}
							}

							// Draw apex cap
							if (apex_valid)
							{
								surf::prim::filled_box((int)screen_apex.x - 2, (int)screen_apex.y - 2, (int)screen_apex.x + 3, (int)screen_apex.y + 3, color(255, 255, 255, hat_alpha));
							}
						}
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

			if (sets->visuals.esp_filter[1] && (strstr(name, "CWeapon") || class_id == CAK47 || class_id == CDEagle))
			{
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
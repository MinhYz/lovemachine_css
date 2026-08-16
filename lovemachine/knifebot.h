#pragma once
#include "game shit.h"
#include "game classes.h"
#include "math.h"
#include "settings.h"
#include "global.h"

namespace legit
{
	namespace knifebot
	{
		inline int get_attack(centity* entity)
		{
			if (!entity) return IN_ATTACK2;
			int hp = entity->get_hp();
			int armor = entity->get_armor();

			if (armor == 0)
			{
				if (hp <= 20)
					return IN_ATTACK;
				else
					return IN_ATTACK2;
			}
			else
			{
				if (hp <= 17)
					return IN_ATTACK;
				else
					return IN_ATTACK2;
			}
		}

		inline void start()
		{
		}

		inline void end()
		{
		}

		inline void run()
		{
			if (!sets->legit.knifebot || !global::cmd || !global::local || !global::local->valid())
				return;

			auto weapon = global::local->get_weapon();
			if (!weapon || weapon->get_weaponid() != weapon_knife)
				return;

			if (weapon->next_primary_attack() > global::curtime && weapon->next_secondary_attack() > global::curtime)
				return;

			cvector local_eye = global::local->get_eye_pos();
			int max_clients = _engine ? _engine->get_max_clients() : 0;

			for (int id = 1; id <= max_clients; id++)
			{
				if (id == global::local_id) continue;
				centity* entity = _ent_list->get_centity(id);
				if (!entity || IsBadReadPtr(entity, sizeof(centity)) || !entity->valid() || entity->is_dormant()) continue;
				if (!sets->legit.friends && entity->get_team() == global::local->get_team()) continue;

				cvector origin = entity->get_abs_origin();
				cvector body = origin + cvector(0.0f, 0.0f, 35.0f);
				cvector head = origin + cvector(0.0f, 0.0f, 60.0f);

				float dist_body = (body - local_eye).Length();
				float dist_origin = (origin - local_eye).Length();
				float dist_head = (head - local_eye).Length();

				float min_dist = min(dist_body, min(dist_origin, dist_head));

				// CS:S knife attack range is up to 75 units for slash, 65 units for backstab/stab
				if (min_dist <= 75.0f)
				{
					qangle aim_angle = calc_angle(local_eye, body);
					normalize_angle(aim_angle);

					int attack_button = get_attack(entity);
					if (attack_button == IN_ATTACK2 && weapon->next_secondary_attack() <= global::curtime && min_dist <= 65.0f)
					{
						global::cmd->viewangles = aim_angle;
						global::cmd->buttons |= IN_ATTACK2;
						break;
					}
					else if (weapon->next_primary_attack() <= global::curtime)
					{
						global::cmd->viewangles = aim_angle;
						global::cmd->buttons |= IN_ATTACK;
						break;
					}
				}
			}
		}
	}
}
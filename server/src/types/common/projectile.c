/************************************************************************
*            Atrinik, a Multiplayer Online Role Playing Game            *
*                                                                       *
*    Copyright (C) 2009-2011 Alex Tokar and Atrinik Development Team    *
*                                                                       *
* Fork from Daimonin (Massive Multiplayer Online Role Playing Game)     *
* and Crossfire (Multiplayer game for X-windows).                       *
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program; if not, write to the Free Software           *
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.             *
*                                                                       *
* The author can be reached at admin@atrinik.org                        *
************************************************************************/

/**
 * @file
 * Common projectile object (arrow, bolt, bullet, etc) related
 * functions. */

#include <global.h>

/** @copydoc object_methods::process_func */
void common_object_projectile_process(object *op)
{
	mapstruct *m;
	int x, y;

	if (!op->map)
	{
		return;
	}

	if (op->last_sp-- <= 0 || !op->direction)
	{
		object_projectile_stop(op, OBJECT_PROJECTILE_STOP_EOL);
		return;
	}

	x = op->x + DIRX(op);
	y = op->y + DIRY(op);
	m = get_map_from_coord(op->map, &x, &y);

	if (!m)
	{
		object_remove(op, 0);
		object_destroy(op);
		return;
	}

	if (wall(m, x, y))
	{
		if (QUERY_FLAG(op, FLAG_REFLECTING))
		{
			if (op->direction & 1)
			{
				op->direction = absdir(op->direction + 4);
			}
			else
			{
				int left, right;

				left = wall(op->map, op->x + freearr_x[absdir(op->direction - 1)], op->y + freearr_y[absdir(op->direction - 1)]);
				right = wall(op->map, op->x + freearr_x[absdir(op->direction + 1)], op->y + freearr_y[absdir(op->direction + 1)]);

				if (left == right)
				{
					op->direction = absdir(op->direction + 4);
				}
				else if (left)
				{
					op->direction = absdir(op->direction + 2);
				}
				else if (right)
				{
					op->direction = absdir(op->direction - 2);
				}
			}

			SET_ANIMATION_STATE(op);
		}
		else
		{
			object_projectile_stop(op, OBJECT_PROJECTILE_STOP_WALL);
			return;
		}
	}

	op = object_projectile_move(op);

	if (!op)
	{
		return;
	}

	if (GET_MAP_FLAGS(op->map, op->x, op->y) & (P_IS_MONSTER | P_IS_PLAYER))
	{
		object *tmp;
		int ret;

		FOR_MAP_LAYER_BEGIN(op->map, op->x, op->y, LAYER_LIVING, tmp);
		{
			tmp = HEAD(tmp);

			if (((QUERY_FLAG(op, FLAG_IS_MISSILE) && QUERY_FLAG(tmp, FLAG_REFL_MISSILE)) || (QUERY_FLAG(op, FLAG_IS_SPELL) && QUERY_FLAG(tmp, FLAG_REFL_SPELL))) && rndm(0, 99) < 90 - (op->level / 10))
			{
				op->direction = absdir(op->direction + 4);
				SET_ANIMATION_STATE(op);
				FOR_MAP_LAYER_BREAK;
			}
			else
			{
				ret = object_projectile_hit(op, tmp);

				if (ret == OBJECT_METHOD_OK)
				{
					object_projectile_stop(op, OBJECT_PROJECTILE_STOP_HIT);
					return;
				}
				else if (ret == OBJECT_METHOD_ERROR)
				{
					return;
				}
			}
		}
		FOR_MAP_LAYER_END;
	}
}

/** @copydoc object_methods::projectile_move_func */
object *common_object_projectile_move(object *op)
{
	object_remove(op, 0);
	op->x = op->x + DIRX(op);
	op->y = op->y + DIRY(op);
	op = insert_ob_in_map(op, op->map, op, 0);

	return op;
}

/** @copydoc object_methods::projectile_stop_func */
object *common_object_projectile_stop_missile(object *op, int reason)
{
	(void) reason;

	/* Already stopped, nothing to do. */
	if (!op->speed)
	{
		return op;
	}

	/* Small chance of breaking */
	if (op->last_eat && rndm_chance(op->last_eat))
	{
		object_remove(op, 0);
		object_destroy(op);
		return NULL;
	}

	/* Restore arrow's properties. */
	if (op->type == ARROW)
	{
		object *owner;

		owner = get_owner(op);
		clear_owner(op);

		op->direction = 0;
		SET_ANIMATION_STATE(op);

		CLEAR_FLAG(op, FLAG_FLYING);
		CLEAR_FLAG(op, FLAG_IS_MISSILE);
		CLEAR_FLAG(op, FLAG_WALK_ON);
		CLEAR_FLAG(op, FLAG_FLY_ON);

		/* Restore WC, damage and range. */
		op->stats.wc = op->last_heal;
		op->stats.dam = op->stats.hp;
		op->last_sp = op->last_grace;

		op->last_heal = op->stats.hp = op->last_grace = 0;

		/* Reset level and speed. */
		op->level = op->arch->clone.level;
		op->speed = op->arch->clone.speed;

		if (!owner || owner->type != PLAYER)
		{
			SET_FLAG(op, FLAG_IS_USED_UP);
			SET_FLAG(op, FLAG_NO_PICK);

			op->type = MISC_OBJECT;
			op->speed = 0.1f;
			op->speed_left = 0.0f;
			op->stats.food = 20;
		}

		update_ob_speed(op);

		op = object_merge(op);
	}
	/* Not an arrow, the object has payload instead. */
	else if (op->inv)
	{
		object *payload;

		payload = op->inv;

		object_remove(payload, 0);
		object_remove(op, 0);
		object_destroy(op);
		payload->x = op->x;
		payload->y = op->y;
		payload = insert_ob_in_map(payload, op->map, op, 0);

		return payload;
	}
	/* Should not happen... */
	else
	{
		object_remove(op, 0);
		object_destroy(op);
		return NULL;
	}

	return op;
}

/** @copydoc object_methods::projectile_stop_func */
object *common_object_projectile_stop_spell(object *op, int reason)
{
	if (reason == OBJECT_PROJECTILE_STOP_HIT && op->stats.dam > 0)
	{
		return op;
	}

	if (op->other_arch)
	{
		explode_object(op);
	}
	else
	{
		object_remove(op, 0);
		object_destroy(op);
	}

	return NULL;
}

/** @copydoc object_methods::projectile_fire_func */
object *common_object_projectile_fire_missile(object *op, object *shooter, int dir)
{
	set_owner(op, shooter);
	op->direction = dir;
	SET_ANIMATION_STATE(op);

	if (!op->speed)
	{
		op->speed = 1;
	}

	/* Save the shooter's level. */
	if (!op->level)
	{
		op->level = SK_level(shooter);
	}

	op->speed_left = 0;
	update_ob_speed(op);

	SET_FLAG(op, FLAG_FLYING);
	SET_FLAG(op, FLAG_IS_MISSILE);
	SET_FLAG(op, FLAG_WALK_ON);
	SET_FLAG(op, FLAG_FLY_ON);

	/* Do not allow stacking, otherwise it is possible for rapidly-fired
	 * missiles to merge, which does not make sense. */
	CLEAR_FLAG(op, FLAG_CAN_STACK);

	op->x = shooter->x;
	op->y = shooter->y;
	op = insert_ob_in_map(op, shooter->map, op, 0);

	if (!op)
	{
		return NULL;
	}

	object_process(op);

	return op;
}

/** @copydoc object_methods::projectile_hit_func */
int common_object_projectile_hit(object *op, object *victim)
{
	object *owner;

	owner = get_owner(op);

	/* Victim is not an alive object or we're friends with the victim,
	 * pass... */
	if (!IS_LIVE(victim) || is_friend_of(owner, victim) || (QUERY_FLAG(op, FLAG_IS_SPELL) && spell_attack_missed(owner, victim)))
	{
		return OBJECT_METHOD_UNHANDLED;
	}

	if (op->stats.dam > 0)
	{
		int dam;

		OBJ_DESTROYED_BEGIN(op);
		dam = hit_player(victim, op->stats.dam, op, 0);

		if (OBJ_DESTROYED(op))
		{
			return OBJECT_METHOD_ERROR;
		}

		op->stats.dam -= dam;

		OBJ_DESTROYED_END(op);
	}

	return OBJECT_METHOD_OK;
}

/** @copydoc object_methods::move_on_func */
int common_object_projectile_move_on(object *op, object *victim, object *originator, int state)
{
	int ret;

	(void) originator;

	if (!state)
	{
		return OBJECT_METHOD_UNHANDLED;
	}

	ret = object_projectile_hit(op, victim);

	if (ret == OBJECT_METHOD_OK)
	{
		object_projectile_stop(op, OBJECT_PROJECTILE_STOP_HIT);
		return OBJECT_METHOD_OK;
	}

	return OBJECT_METHOD_OK;
}
#include <darnit/darnit.h>
#include <math.h>
#include "../main.h"
#include <string.h>
//#include "network/protocol.h"
//#include "server/server.h"
#include "../util.h"
//#include "block.h"
#include "../spritelist.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "../network/network.h"
#include "../network/protocol.h"
#include "server.h"


int movableInit() {
	int i;

	ss->movable.bbox = d_bbox_new(MAX_MOVABLE); // Should be enough for anyone
	ss->movable.movables = MAX_MOVABLE;
	d_bbox_set_indexkey(ss->movable.bbox);

	for (i = 0; i < ss->movable.movables; i++) {
		ss->movable.movable[i].used = 0;
		ss->movable.movable[i].req_cleanup = 0;
	}

	ss->movable.in_loop = 0;
	
	return 0;
}


static int _next_available() {
	return d_bbox_add(ss->movable.bbox, 3, 3, 17, 17);
}


int _test_boundaries(int x, int y, int x2, int y2) {
	int center_x, center_y, radius;
	const char *xprop, *yprop, *rprop;

	x2 += x;
	y2 += y;

	if (!strcmp(xprop = d_map_prop(ss->active_level->prop, "center_x"), "NO SUCH KEY"))
		center_x = 100;
	else
		center_x = atoi(xprop);
	if (!strcmp(yprop = d_map_prop(ss->active_level->prop, "center_y"), "NO SUCH KEY"))
		center_x = 100;
	else
		center_y = atoi(yprop);
	if (!strcmp(rprop = d_map_prop(ss->active_level->prop, "radius"), "NO SUCH KEY"))
		radius = 500;
	else
		radius = atoi(rprop);
	x -= center_x;
	y -= center_y;
	x2 -= center_x;
	y2 -= center_y;
	if ((x * x + y * y) >= radius*radius)
		return 0;
	if ((x2 * x2 + y * y) >= radius*radius)
		return 0;
	if ((x * x + y2 * y2) >= radius*radius)
		return 0;
	if ((x2 * x2 + y2 * y2) >= radius*radius)
		return 0;
	
	return 1;
}


void gcenter_calc(int x, int y, int *gx, int *gy) {
	int center_x, center_y;
	const char *xprop, *yprop;
	
	if (!strcmp(xprop = d_map_prop(ss->active_level->prop, "center_x"), "NO SUCH KEY"))
		center_x = 100;
	else
		center_x = atoi(xprop);
	if (!strcmp(yprop = d_map_prop(ss->active_level->prop, "center_y"), "NO SUCH KEY"))
		center_y = 100;
	else
		center_y = atoi(yprop);
	
	x = (x - center_x) * GRAVITY_SCALE;
	y = (y - center_y) * GRAVITY_SCALE;

	if (x * x + y * y > GRAVITY_CAP * GRAVITY_CAP) {
		*gx = (x * GRAVITY_CAP / sqrt(x * x + y * y));
		*gy = (y * GRAVITY_CAP / sqrt(x * x + y * y));
	} else
		*gx = x, *gy = y;

}


static int _lookup_movable_player_id(int id) {
	const char *playerid_str;
	int i;

	for (i = 0; i < ss->movable.movables; i++) {
		if (!(playerid_str = ss->active_level->object[ss->movable.movable[i].id].ref, "block_id"))
			return 0;
		if (atoi(playerid_str) == id)
			return i;
	}

	return 0;
}


int movableSpawnReal(DARNIT_SPRITE *spr, int x, int y, int l, int type_hint) {
	int idx;
	int h_x, h_y, h_w, h_h;

	idx = _next_available();
	ss->movable.movable[idx].used = 1;
	ss->movable.movable[idx].sprite = spr;

	d_sprite_direction_set(ss->movable.movable[idx].sprite, 0);
	ss->movable.movable[idx].x = x * 1000;
	ss->movable.movable[idx].y = y * 1000;
	printf("spawning movable %i at %i %i\n", idx, ss->movable.movable[idx].x/1000, ss->movable.movable[idx].y/1000);
	ss->movable.movable[idx].l = l;
	ss->movable.movable[idx].direction = 0;
	ss->movable.movable[idx].angle = 0;
	ss->movable.movable[idx].gravity_effect = 0;
	ss->movable.movable[idx].x_velocity = 0;
	ss->movable.movable[idx].y_velocity = 0;
	ss->movable.movable[idx].x_gravity = 0;
	ss->movable.movable[idx].y_gravity = 0;
	ss->movable.movable[idx].tile_collision = 1;
	ss->movable.movable[idx].req_cleanup = 0;
	ss->movable.movable[idx].id = idx;
	d_sprite_hitbox(ss->movable.movable[idx].sprite, &h_x, &h_y, &h_w, &h_h);
	d_bbox_move(ss->movable.bbox, idx, ss->movable.movable[idx].x / 1000 + h_x, ss->movable.movable[idx].y / 1000 + h_y);
	d_bbox_resize(ss->movable.bbox, idx, h_w, h_h);
	d_sprite_animate_start(ss->movable.movable[idx].sprite);

	/* Send packet */ {
		Packet pack;
		
		pack.type = PACKET_TYPE_MOVABLE_SPAWN;
		pack.size = sizeof(PacketMovableSpawn);

		pack.movable_spawn.x = x;
		pack.movable_spawn.y = y;
		pack.movable_spawn.l = l;
		pack.movable_spawn.sprite_type = type_hint;
		pack.movable_spawn.angle = 0;
		pack.movable_spawn.dir = 0;
		pack.movable_spawn.movable = idx;

		server_broadcast_packet(&pack);
	}

	return idx;
}

int movableSpawn(char *sprite, int x, int y, int l, int type_hint) {
	return movableSpawnReal(d_sprite_load(util_binrel_path(sprite), 0, DARNIT_PFORMAT_RGB5A1), x, y, l, type_hint);

}


int movableSpawnSprite(int x, int y, int l, int sprite) {
	return movableSpawnReal(spritelist_get(sprite), x, y, l, sprite);
}


void movableDespawn(int idx) {
	if (ss->movable.in_loop)
		ss->movable.movable[idx].req_cleanup = 1;
	else {
		d_sprite_free(ss->movable.movable[idx].sprite);
		d_bbox_delete(ss->movable.bbox, idx);
	}
	ss->movable.movable[idx].used = 0;

	/* Send packet */ {
		Packet pack;
		
		pack.type = PACKET_TYPE_MOVABLE_DESPAWN;
		pack.size = sizeof(PacketMovableDespawn);
	
		pack.movable_despawn.movable = idx;
		server_broadcast_packet(&pack);
	}
}


int movableTileCollision(MOVABLE_ENTRY *entry, int off_x, int off_y) {
	int box_x, box_y, box_w, box_h, w;

	d_sprite_hitbox(entry->sprite, &box_x, &box_y, &box_w, &box_h);
	box_x += (entry->x / 1000);
	box_y += (entry->y / 1000);
	box_x += (off_x < 0) ? -1 : (box_w + 1 * (off_x == 0 ? -1 : 1));
	box_y += (off_y < 0) ? -1 : (box_h + 1 * (off_y == 0 ? -1 : 1));
	off_x += (off_x == -2) ? 1 : 0;
	off_y += (off_y == -2) ? 1 : 0;

	box_x /= ss->active_level->layer[entry->l].tile_w;
	box_y /= ss->active_level->layer[entry->l].tile_h;
	w = ss->active_level->layer[entry->l].tilemap->w;
	return ss->active_level->layer[entry->l].tilemap->data[box_x + box_y * w];
}


/* NOTE: An award of one strawberry goes to whoever can guess who suggested this as the function suffix */
void movableKillEmAll() {
	#if 0
	int i;

	for (i = 0; i < ss->movable.movables; i++) {
		if (!ss->movable.movable[i].ai)
			continue;
		if (ss->movable.movable[i].hp <= 0)
			continue;
		ss->movable.movable[i].ai(s, &ss->movable.movable[i], MOVABLE_MSG_DESTROY);
	}

	d_bbox_clear(ss->movable.bbox);

	return;
	#endif
}


int movableMoveDo(DARNIT_MAP_LAYER *layer, int *pos, int *delta, int *vel, int space, int col, int hit_off, int vel_r, int i, int i_b, int *tile_index) {
	int u, t, tile_w, tile_h, map_w, i_2;
	unsigned int *map_d;
	tile_w = layer->tile_w;
	tile_h = layer->tile_h;
	map_w = layer->tilemap->w;
	map_d = layer->tilemap->data;

	if (tile_index)
		*tile_index = -1;

	if (!(*delta))
		return 0;
	u = (*pos) / 1000 + hit_off;
	t = u + (((*delta) > 0) ? 1 : -1);

	if (i < 0) {
		u /= tile_h;
		t /= tile_h;
	} else {
		u /= tile_w;
		t /= tile_w;
	}

	if (u == t) {
		(*pos) += space;
		(*delta) -= space;
		return 0;
	}

	i_2 = (i < 0) ? t * map_w + (abs(i) + i_b) / tile_w : ((i + i_b) / tile_h) * map_w + t;
	i = (i < 0) ? t * map_w + abs(i) / tile_w : (i / tile_h) * map_w + t;
	
	if (map_d[i] & col || map_d[i_2] & col) {
		if (map_d[i] & col) {
			if (tile_index)
				*tile_index = i;
		} else if (map_d[i_2] & col) {
			if (tile_index)
				*tile_index = i;
		}

		(*vel) = vel_r;
		(*delta) = 0;
		return 1;
	}

	(*pos) += space;
	(*delta) -= space;
	return 0;
}


int movableHackTest(MOVABLE_ENTRY *entry) {
	return !((movableTileCollision(entry, -1, 1) & movableTileCollision(entry, 1, 1)) & COLLISION_TOP);
}


int movableGravity(MOVABLE_ENTRY *entry) {
	//int gravity, hack;
	int delta_x, delta_y, r, p, gravity_x, gravity_y;
	int hit_x, hit_y, hit_w, hit_h;
	int i, tmp, collided_tile = -1;
	int collision_event[MAX_MOVABLE] = { 0 };
	unsigned int hitlist[MAX_MOVABLE], hits;

	DARNIT_MAP_LAYER *layer = &(ss->active_level->layer[entry->l]);

	d_sprite_hitbox(entry->sprite, &hit_x, &hit_y, &hit_w, &hit_h);
	hit_w--;
	hit_h--;

	entry->gravity_blocked = 0;
	entry->movement_blocked = 0;
	if (entry->gravity_effect) {
		printf("gravity effect\n");
		gcenter_calc(entry->x / 1000, entry->y / 1000, &gravity_x, &gravity_y);
		entry->x_gravity += gravity_x * d_last_frame_time();
		entry->y_gravity += gravity_y * d_last_frame_time();
		if ((entry->x_gravity * entry->x_gravity + entry->y_gravity + entry->y_gravity) > MOV_TERMINAL_VELOCITY * MOV_TERMINAL_VELOCITY) {
			gravity_x = MOV_TERMINAL_VELOCITY * entry->x_gravity / sqrt(entry->x_gravity * entry->x_gravity + entry->y_gravity + entry->y_gravity);
			gravity_y = MOV_TERMINAL_VELOCITY * entry->y_gravity / sqrt(entry->x_gravity * entry->x_gravity + entry->y_gravity + entry->y_gravity);
			entry->x_gravity = gravity_x;
			entry->y_gravity = gravity_y;
		}

		//printf("Gravity is %i, %i\n", entry->x_gravity, entry->y_gravity);


		/* Y-axis */
		delta_y = (entry->y_gravity * d_last_frame_time() / 30);

		/* X-axis */
		delta_x = entry->x_gravity * d_last_frame_time() / 30;
		//printf("ΔX = %i, ΔY = %i\n", delta_x, delta_y);
	
		/* TODO: STUB */
		if (!_test_boundaries((entry->x + delta_x)/1000 + util_sprite_xoff(entry->sprite), (entry->y + delta_y)/1000 + util_sprite_yoff(entry->sprite), util_sprite_width(entry->sprite), util_sprite_height(entry->sprite))) {
			entry->gravity_blocked = 1;
			goto nogravity;
		}

		p = delta_x * 1000 / (delta_y ? delta_y : 1);

		while (delta_x || delta_y) {
			if ((hits = d_bbox_test(ss->movable.bbox, entry->x / 1000, entry->y / 1000, hit_w, hit_h, hitlist, MAX_MOVABLE)) > 0) {
				for (i = 0; i < hits; i++) {
					if (collision_event[hitlist[i]])
						continue;
					collision_event[hitlist[i]] = 1;
					
					if (hitlist[i] == entry->id)
						continue;
					if (!entry->used)
						return -990123;
					if (!ss->movable.movable[hitlist[i]].used)
						continue;
					if (entry->callback.movable_collision)
						entry->callback.movable_collision(entry->callback.user_pointer, entry->id, hitlist[i]);
					
					if (!entry->used)
						return -123166;
					if (!ss->movable.movable[hitlist[i]].used)
						continue;
					if (ss->movable.movable[hitlist[i]].callback.movable_collision)
						ss->movable.movable[hitlist[i]].callback.movable_collision(ss->movable.movable[hitlist[i]].callback.user_pointer, hitlist[i], entry->id);
						

				}
			}
			
			if (!entry->used)
				return -1231145;

			if (delta_x && ((!delta_y || delta_x * 1000 / (delta_y ? delta_y : 1) > p))) {
				r = entry->x % 1000;
				if (r + delta_x < 1000 && r + delta_x >= 0) {
					entry->x += delta_x;
					delta_x = 0;
					continue;
				}
				
				
				if (delta_x > 0) {
					r = 1000 - r;
					if (!entry->tile_collision)
						(movableMoveDo(layer, &entry->x, &delta_x, &entry->x_gravity, r, 0, hit_x + hit_w, 0, entry->y / 1000 + hit_y, hit_h, NULL));
					else
						entry->gravity_blocked |= (movableMoveDo(layer, &entry->x, &delta_x, &entry->x_gravity, r, COLLISION_LEFT, hit_x + hit_w, 0, entry->y / 1000 + hit_y, hit_h, NULL));
						
				} else { 
					if (!r)
						r = 1000;
					r *= -1;
					if (!entry->tile_collision)
						movableMoveDo(layer, &entry->x, &delta_x, &entry->x_gravity, r, 0, hit_x, 0, entry->y / 1000 + hit_y, hit_h, NULL);
					else
						entry->gravity_blocked |= movableMoveDo(layer, &entry->x, &delta_x, &entry->x_gravity, r, COLLISION_RIGHT, hit_x, 0, entry->y / 1000 + hit_y, hit_h, NULL);
				}
			} else {
				r = entry->y % 1000;
				if (r + delta_y < 1000 && r + delta_y >= 0) {
					entry->y += delta_y;
					delta_y = 0;
					continue;
				}
			
				if (delta_y > 0) {
					r = 1000 - r;
					if (!entry->tile_collision)
						movableMoveDo(layer, &entry->y, &delta_y, &entry->y_gravity, r, 0, hit_y + hit_h, 0, entry->x / -1000 - hit_x, hit_w, NULL);
					else
						entry->gravity_blocked |= movableMoveDo(layer, &entry->y, &delta_y, &entry->y_gravity, r, COLLISION_TOP, hit_y + hit_h, 0, entry->x / -1000 - hit_x, hit_w, NULL);
				} else {
					if (!r)
						r = 1000;
					r *= -1;
					if (!entry->tile_collision)
						movableMoveDo(layer, &entry->y, &delta_y, &entry->y_gravity, r, 0, hit_y, -1, entry->x / -1000 - hit_x, hit_w, NULL);
					else
						entry->gravity_blocked |= movableMoveDo(layer, &entry->y, &delta_y, &entry->y_gravity, r, COLLISION_BOTTOM, hit_y, -1, entry->x / -1000 - hit_x, hit_w, NULL);
				}
			} 

		}
	}

nogravity:

	entry->movement_blocked = 0;
	if (entry->gravity_blocked) {
		entry->x_gravity = entry->y_gravity = 0;
		if (entry->callback.gravity_map_collision)
			entry->callback.gravity_map_collision(entry->callback.user_pointer, entry->id);
		//printf("Bonk!\n");
	}

	delta_x = (entry->x_velocity * d_last_frame_time());
	delta_y = (entry->y_velocity * d_last_frame_time());
	p = delta_x * 1000 / (delta_y ? delta_y : 1);
	
	
	while (delta_x || delta_y) {
		if ((hits = d_bbox_test(ss->movable.bbox, entry->x / 1000, entry->y / 1000, hit_w, hit_h, hitlist, MAX_MOVABLE)) > 0) {
			for (i = 0; i < hits; i++) {
				if (collision_event[hitlist[i]])
					continue;
				collision_event[hitlist[i]] = 1;
				
				if (hitlist[i] == entry->id)
					continue;
				if (!entry->used)
					return -990123;
				if (!ss->movable.movable[hitlist[i]].used)
					continue;
				if (entry->callback.movable_collision)
					entry->callback.movable_collision(entry->callback.user_pointer, entry->id, hitlist[i]);
				
				if (!entry->used)
					return -123166;
				if (!ss->movable.movable[hitlist[i]].used)
					continue;
				if (ss->movable.movable[hitlist[i]].callback.movable_collision)
					ss->movable.movable[hitlist[i]].callback.movable_collision(ss->movable.movable[hitlist[i]].callback.user_pointer, hitlist[i], entry->id);
					

			}
		}
			
		if (!entry->used)
			return -1231145;

		if (delta_x && (!delta_y || delta_x * 1000 / (delta_y ? delta_y : 1) > p)) {
			r = entry->x % 1000;
			if (r + delta_x < 1000 && r + delta_x >= 0) {
				entry->x += delta_x;
				delta_x = 0;
				continue;
			}
			
			/*
			if (!_test_boundaries((entry->x + delta_x)/1000 + util_sprite_xoff(entry->sprite), (entry->y)/1000 + util_sprite_yoff(entry->sprite), util_sprite_width(entry->sprite), util_sprite_height(entry->sprite))) {
				delta_x = 0;
				entry->movement_blocked = 1;
				continue;
			}*/

			if (delta_x > 0) {
				r = 1000 - r;

				entry->movement_blocked |= movableMoveDo(layer, &entry->x, &delta_x, &entry->x_velocity, r, COLLISION_LEFT, hit_x + hit_w, 0, entry->y / 1000 + hit_y, hit_h, &tmp);
			} else {
				if (!r)
					r = 1000;
				r *= -1;

				entry->movement_blocked |= movableMoveDo(layer, &entry->x, &delta_x, &entry->x_velocity, r, COLLISION_RIGHT, hit_x, 0, entry->y / 1000 + hit_y, hit_h, &tmp);
			}

			if (tmp > -1)
				collided_tile = tmp;
		} else {	/* delta_y måste vara != 0 */
			r = entry->y % 1000;
			if (r + delta_y < 1000 && r + delta_y >= 0) {
				entry->y += delta_y;
				delta_y = 0;
				continue;
			} 
			
			/*
			if (!_test_boundaries((entry->x)/1000 + util_sprite_xoff(entry->sprite), (entry->y + delta_y)/1000 + util_sprite_yoff(entry->sprite), util_sprite_width(entry->sprite), util_sprite_height(entry->sprite))) {
				delta_y = 0;
				entry->movement_blocked = 1;
				continue;
			}*/

			if (delta_y > 0) {
				r = 1000 - r;
				entry->movement_blocked |= movableMoveDo(layer, &entry->y, &delta_y, &entry->y_velocity, r, COLLISION_TOP, hit_y + hit_h, 0, entry->x / -1000 - hit_x, hit_w, &tmp);
			} else {
				if (!r)
					r = 1000;
				r *= -1;
				entry->movement_blocked |= movableMoveDo(layer, &entry->y, &delta_y, &entry->y_velocity, r, COLLISION_BOTTOM, hit_y, -1, entry->x / -1000 - hit_x, hit_w, &tmp);
			}
			
			if (tmp > -1)
				collided_tile = tmp;
		}
	}

	if (entry->movement_blocked)
		if (entry->callback.map_collision)
			entry->callback.map_collision(entry->callback.user_pointer, entry->id, collided_tile), printf("Blocked movement\n");

	return -1462573849;
}


void movableLoop() {
	int i, j, h_x, h_y, h_w, h_h, players_active = 0, winning_player = -1;
	bool master = ss->is_host;

	ss->movable.in_loop = 1;

	for (j = 0; j < MAX_MOVABLE; j++) {
		i = j;
		if (!ss->movable.movable[i].used)
			continue;

		if (master) {
			movableGravity(&ss->movable.movable[i]);
			if (!ss->movable.movable[i].used)
				continue;
			//printf("Player at %i %i\n", ss->movable.movable[i].x / 1000, ss->movable.movable[i].y / 1000);
		}


		if (ss->movable.movable[i].direction != d_sprite_direction_get(ss->movable.movable[i].sprite))
			d_sprite_direction_set(ss->movable.movable[i].sprite, ss->movable.movable[i].direction);
		d_sprite_move(ss->movable.movable[i].sprite, ss->movable.movable[i].x / 1000, ss->movable.movable[i].y / 1000);
		d_sprite_hitbox(ss->movable.movable[i].sprite, &h_x, &h_y, &h_w, &h_h);
		d_sprite_rotate(ss->movable.movable[i].sprite, ss->movable.movable[i].angle);
		d_bbox_move(ss->movable.bbox, i, ss->movable.movable[i].x / 1000 + h_x, ss->movable.movable[i].y / 1000 + h_y);
		d_bbox_resize(ss->movable.bbox, i, h_w, h_h);
	}

	if (players_active <= 1 && master) {
		//fprintf(stderr, "win condition\n");
		//server_announce_winner(winning_player);
	}

	for (i = 0; i < MAX_MOVABLE; i++)
		if (!ss->movable.movable[i].used && ss->movable.movable[i].req_cleanup) {
			d_sprite_free(ss->movable.movable[i].sprite);
			d_bbox_delete(ss->movable.bbox, i);
		}

	ss->movable.in_loop = 0;

}


void movableFreezeSprites(int freeze) {
	int i;

	for (i = 0; i < ss->movable.movables; i++)
		(!freeze ? d_sprite_animate_start : d_sprite_animate_pause)(ss->movable.movable[i].sprite);
	return;
}


void movableLoopRender(int layer) {
	#if 0
	int i, res;
	unsigned int *arr = s->movable.coll_buf;

	res = d_bbox_test(s->movable.bbox, s->camera.x - 128, s->camera.y - 128, d_platform_get().screen_w + 256, d_platform_get().screen_h + 256, s->movable.coll_buf, ~0);

	for (i = 0; i < res; i++) {
		if (s->movable.movable[arr[i]].l != layer)
			continue;
		d_sprite_draw(s->movable.movable[arr[i]].sprite);
	}
	#endif
}



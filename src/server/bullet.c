#include <math.h>

#include "server.h"
#include "../network/protocol.h"

#include "../main.h"
#include "../config.h"
#include "unit.h"
#include "bullet.h"

BulletProperties bullet_properties[BULLET_TYPES] = {
	[BULLET_TYPE_WIMPY] = {
		.damage = 1,
		.ttl = 1000,
		.cost = 5,
		.speed = 1000,
		//.sprite_name = "res/bullet_wimpy.spr",
		.sprite_type = SPRITELIST_ENTRY_INDEX_BULLET_WIMPY,
	},
	
	[BULLET_TYPE_BADASS] = {
		.damage = 10,
		.ttl = 1000,
		.cost = 50,
		.speed = 500,
		.sprite_type = SPRITELIST_ENTRY_INDEX_BULLET_BADASS,
		//.sprite_name = "res/bullet_badass.spr",
	},
};

void _delete_entry(Bullet *bullet) {
	movableDespawn(bullet->movable);
	free(bullet);
}


static Client *_get_movable_client(int movable) {
	Client *tmp;

	for (tmp = server_get_client_list(); tmp; tmp = tmp->next)
		if (tmp->movable == movable)
			return tmp;

	return NULL;
}


static int _get_bullet_owner(int movable_remote) {
	Bullet *next;

	for (next = ss->bullet; next; next = next->next) {
		if (next->movable == movable_remote)
			return next->owner;
	}

	return -1;
}


static int _get_bullet_type(int movable_remote) {
	Bullet *next;

	for (next = ss->bullet; next; next = next->next) {
		if (next->movable == movable_remote)
			return next->type;
	}

	return 0;;
}



void bullet_init() {
	int i;
	ss->bullet = NULL;
	#if 0
	for(i = 0; i < BULLET_TYPES; i++) {
		bullet_properties[i].sprite = d_sprite_load(bullet_properties[i].sprite_name, 0, DARNIT_PFORMAT_RGBA8);
	}
	#endif
}


static void _bullet_kill(int movable) {
	/* *ONLY* CALL FROM A CALLBACK, NEVER FROM bullet_loop */
	Bullet **next, *del;

	for (next = (&ss->bullet); *next; next = &(*next)->next) {
		if ((*next)->movable == movable) {
			(*next)->ticks = 0;
			server_particle_spawn(ss->movable.movable[(*next)->movable].x / 1000, ss->movable.movable[(*next)->movable].y / 1000, ss->movable.movable[(*next)->movable].angle + 1800, 0);
		}
	}
}


static void _bullet_movable_collision(void *ptr, int movable_self, int movable_remote) {
	int remote_owner;

	if ((remote_owner = _get_bullet_owner(movable_remote)) < 0) {
		/* Är spelare */
		if (movable_remote == _get_bullet_owner(movable_self)) {
			/* Passing through owner */
		} else {
			Client *target = _get_movable_client(movable_remote);
			target->hp -= bullet_properties[_get_bullet_type(movable_self)].damage;
			_bullet_kill(movable_self);
			printf("Hit someone else! hp=%i\n", target->hp);
		}
	} else if (remote_owner == _get_bullet_owner(movable_self)) {
		// Intersecting owners own bullets. meh.
	} else {
		// Bullet of someone else
		_bullet_kill(movable_self);
		_bullet_kill(movable_remote);
	}

	return;
}

static void _bullet_map_collision(void *usr_ptr, int movable_self, int tile_index) {
	int x, y;
	UnitEntry *u;
	BulletProperties *prop = usr_ptr;
	
	x = tile_index % ss->active_level->layer->tilemap->w;
	y = tile_index / ss->active_level->layer->tilemap->w;
	
	printf("bullet %i hit tile %i (%i %i)\n", movable_self, tile_index, x, y);
	_bullet_kill(movable_self);
	
	if((u = unit_find_tile_owner(x, y))) {
		unit_damage(u, prop->damage);
	}
		
}

int bullet_spawn_from_movable(BulletType type, int x, int y, int angle, int owner) {
	Bullet *bullet = NULL;
	
	bullet = malloc(sizeof(Bullet));
	
	bullet->type = type;
	bullet->ticks = bullet_properties[type].ttl;
	bullet->movable = movableSpawnSprite(x, y, 0, bullet_properties[type].sprite_type);
	bullet->owner = owner;
	ss->movable.movable[bullet->movable].x = x*1000;
	ss->movable.movable[bullet->movable].y = y*1000;
	ss->movable.movable[bullet->movable].angle = angle*10;
	ss->movable.movable[bullet->movable].x_velocity = bullet_properties[type].speed * cos(((double) angle)*M_PI/180.0);
	ss->movable.movable[bullet->movable].y_velocity = bullet_properties[type].speed * sin(((double) angle)*M_PI/180.0);
	ss->movable.movable[bullet->movable].callback.movable_collision = _bullet_movable_collision;
	ss->movable.movable[bullet->movable].callback.map_collision = _bullet_map_collision;
	ss->movable.movable[bullet->movable].callback.user_pointer = &bullet_properties[type];
	
	bullet->next = ss->bullet;
	ss->bullet = bullet;
	
	return bullet->movable;
}

int bullet_spawn(BulletType type, Client *owner) {
	if(ss->team[owner->team].money - bullet_properties[type].cost < 0)
		return -1;
	
	ss->team[owner->team].money -= bullet_properties[type].cost;
	
	return bullet_spawn_from_movable(type, ss->movable.movable[owner->movable].x/1000 + 20, ss->movable.movable[owner->movable].y/1000 + 20, owner->angle, owner->movable);
}



int bullet_loop() {
	Bullet **next, *del;

	for (next = &(ss->bullet); *next; next = &(*next)->next) {
		(*next)->ticks -= d_last_frame_time();
		if ((*next)->ticks <= 0) {
			del = *next;
			*next = (*next)->next;
			_delete_entry(del);
			if (!*next)
				break;
		}
	}
	
	return 0;
}

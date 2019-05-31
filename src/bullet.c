#include <math.h>

#include "server/server.h"
#include "network/protocol.h"

#include "main.h"
#include "config.h"
#include "player.h"
#include "bullet.h"

BulletProperties bullet_properties[BULLET_TYPES] = {
	[BULLET_TYPE_WIMPY] = {
		.damage = 1,
		.ttl = 2000,
		.cost = 1,
		.speed = 100,
		//.sprite_name = "res/bullet_wimpy.spr",
		.sprite_type = SPRITELIST_ENTRY_INDEX_BULLET_WIMPY,
	},
	
	[BULLET_TYPE_BADASS] = {
		.damage = 10,
		.ttl = 60,
		.cost = 10,
		.speed = 2,
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


int bullet_spawn(BulletType type, Client *owner) {
	int x, y, angle;
	Bullet *bullet = NULL;
	
	if(ss->team[owner->team].money - bullet_properties[type].cost < 0)
		return -1;
	
	bullet = malloc(sizeof(Bullet));
	
	x = ss->movable.movable[owner->movable].x;
	y =ss->movable.movable[owner->movable].y;
	angle = owner->angle;
	
	bullet->type = type;
	bullet->ticks = bullet_properties[type].ttl;
	bullet->movable = movableSpawnSprite(0, 0, 0, bullet_properties[type].sprite_type);
	bullet->owner = owner->movable;
	ss->movable.movable[bullet->movable].x = x;
	ss->movable.movable[bullet->movable].y = y;
	ss->movable.movable[bullet->movable].angle = angle;
	ss->movable.movable[bullet->movable].x_velocity = bullet_properties[type].speed * cos(((double) angle)*M_PI/180.0);
	ss->movable.movable[bullet->movable].y_velocity = bullet_properties[type].speed * sin(((double) angle)*M_PI/180.0);
	ss->movable.movable[bullet->movable].callback.movable_collision = _bullet_movable_collision;

	ss->team[owner->team].money -= bullet_properties[type].cost;
	bullet->next = ss->bullet;
	ss->bullet = bullet;

	return bullet->movable;
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

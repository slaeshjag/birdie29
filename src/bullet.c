#include <math.h>

#include "main.h"
#include "config.h"
#include "player.h"
#include "bullet.h"

BulletProperties bullet_properties[BULLET_TYPES] = {
	[BULLET_TYPE_WIMPY] = {
		.damage = 1,
		.ttl = 200,
		.cost = 1,
		.speed = 10,
		.sprite_name = "res/bullet_wimpy.spr",
	},
	
	[BULLET_TYPE_BADASS] = {
		.damage = 10,
		.ttl = 60,
		.cost = 10,
		.speed = 2,
		.sprite_name = "res/bullet_badass.spr",
	},
};

void bullet_init() {
	int i;
	for(i = 0; i < BULLET_TYPES; i++) {
		bullet_properties[i].sprite = d_sprite_load(bullet_properties[i].sprite_name, 0, DARNIT_PFORMAT_RGBA8);
	}
}


int bullet_spawn(BulletType type, Player *owner) {
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
	bullet->movable = movableSpawnSprite(bullet_properties[type].sprite, 0, 0, 0, 0 /* TODO: Replace with bullet index */);
	ss->movable.movable[bullet->movable].x = x;
	ss->movable.movable[bullet->movable].y = y;
	ss->movable.movable[bullet->movable].angle = angle;
	ss->movable.movable[bullet->movable].x_velocity = bullet_properties[type].speed * cos(((double) angle)*M_PI/180.0);
	ss->movable.movable[bullet->movable].y_velocity = bullet_properties[type].speed * sin(((double) angle)*M_PI/180.0);
	
	ss->team[owner->team].money -= bullet_properties[type].cost;
	
	return bullet->movable;
}

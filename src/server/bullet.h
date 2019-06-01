#ifndef BULLET_H_
#define BULLET_H_

#include <darnit/darnit.h>
//#include "server/server.h"
#include "../spritelist.h"

typedef enum BulletType BulletType;
enum BulletType {
	BULLET_TYPE_WIMPY,
	BULLET_TYPE_BADASS,
	BULLET_TYPES,
};

typedef struct Bullet Bullet;
struct Bullet {
	BulletType type;
	int movable;
	int ticks;
	int owner;
	Bullet *next;
};

typedef struct BulletProperties BulletProperties;
struct BulletProperties {
	int damage;
	int ttl;
	int cost;
	int speed;
	enum SpritelistEntryIndex sprite_type;
	const char *sprite_name;
	DARNIT_SPRITE *sprite;
};

extern BulletProperties bullet_properties[BULLET_TYPES];

//int bullet_spawn(BulletType type, Client *owner);
int bullet_loop();
void bullet_init();

#endif

#ifndef PLAYER_H_
#define PLAYER_H_

#include "config.h"

typedef struct Player Player;
struct Player {
	char name[NAME_LEN_MAX + 1];
	int id;
	int sprite_variant;
	int team;
	int movable;
};

void player_join(int id, const char *name, int team);

#endif

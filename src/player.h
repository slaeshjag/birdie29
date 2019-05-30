#ifndef PLAYER_H_
#define PLAYER_H_

#include "main.h"

typedef struct Player Player;
struct Player {
	char name[NAME_LEN_MAX + 1];
	bool active;
	int sprite_variant;
	int team;
	int movable;
	
};

#endif

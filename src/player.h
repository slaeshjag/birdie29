#ifndef PLAYER_H_
#define PLAYER_H_

#include "config.h"
#include "ingame.h"

typedef enum PlayerDirection PlayerDirection;
enum PlayerDirection {
	PLAYER_DIRECTION_DOWN,
	PLAYER_DIRECTION_UP,
	PLAYER_DIRECTION_LEFT,
	PLAYER_DIRECTION_RIGHT,
};

typedef struct Player Player;
struct Player {
	char name[NAME_LEN_MAX + 1];
	int id;
	int sprite_variant;
	int team;
	int movable;
	
	int angle;
	
	InGameKeyStateEntry keystate;
};

void player_set(int id, const char *name, int team);
void player_join(int id, const char *name, int team);
void player_handle_keys(Player *player);

#endif

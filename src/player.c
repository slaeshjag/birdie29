#include <string.h>

#include "player.h"
#include "main.h"

void player_set(int id, const char *name, int team) {
	strcpy(s->player[id]->name, name);
	s->player[id]->team = team;
}

void player_join(int id, const char *name, int team) {
	Player *p = NULL;
	
	p = calloc(1, sizeof(Player));
	
	strcpy(p->name, name);
	p->id = id;
	p->team = team;
	
	p->movable = movableSpawn("res/player.spr", 0, 0, 0);
	
	s->player[id] = p;
}

void player_handle_keys(Player *player) {
	MOVABLE_ENTRY *m = &s->movable.movable[player->movable];
	
	if(player->keystate.up) {
		printf("client: player %i hold up\n", player->id);
		m->y_velocity = -PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_UP;
	} else if(player->keystate.down) {
		printf("client: player %i hold down\n", player->id);
		m->y_velocity = PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_DOWN;
	} else {
		m->y_velocity = 0;
	}
	
	if(player->keystate.left) {
		printf("client: player %i hold left\n", player->id);
		m->x_velocity = -PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_LEFT;
	} else if(player->keystate.right) {
		printf("client: player %i hold right\n", player->id);
		m->x_velocity = PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_RIGHT;
	} else {
		m->x_velocity = 0;
	}
}

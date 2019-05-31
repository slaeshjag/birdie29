#include <string.h>

#include "player.h"
#include "server/server.h"
#include "main.h"

void player_set(int id, const char *name, int team) {
	strcpy(cs->player[id]->name, name);
	cs->player[id]->team = team;
}

void player_join(int id, const char *name, int team) {
	Player *p = NULL;
	
	p = calloc(1, sizeof(Player));
	
	strcpy(p->name, name);
	p->id = id;
	p->team = team;
	
	//p->movable = movableSpawn("res/player.spr", 0, 0, 0, 0 /* TODO: Replace with sprite index */);
	
	cs->player[id] = p;
}

#include <string.h>

#include "player.h"
#include "main.h"

void player_set(int id, const char *name, int team, int movable) {
	strcpy(cs->player[id]->name, name);
	cs->player[id]->team = team;
	cs->player[id]->movable = movable;
}

void player_join(int id, const char *name, int team, int movable) {
	Player *p = NULL;
	
	p = calloc(1, sizeof(Player));
	
	strcpy(p->name, name);
	p->id = id;
	p->team = team;
	p->selected_building = -1;
	p->movable = movable;
	
	//p->movable = movableSpawn("res/player.spr", 0, 0, 0, 0 /* TODO: Replace with sprite index */);
	
	cs->player[id] = p;
	
}

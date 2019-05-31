#include "main.h"
#include "unit.h"


static struct UnitTiles _building_tiles[UNIT_TYPES] = {
	[UNIT_TYPE_GENERATOR] = { 0, 0, 0, 0 },
	[UNIT_TYPE_PYLON] = { 0, 0, 0, 0 },
	[UNIT_TYPE_MINER] = { 0, 0, 0, 0 },
	[UNIT_TYPE_WALL] = { 0, 0, 0, 0 },
};



void unit_housekeeping() {
	int i;
	struct UnitEntry **e, *tmp;

	for (i = 0; i < MAX_TEAM; i++) {
		for (e = &ss->team[i].unit.unit; *e; e = &(*e)->next) {
			if ((*e)->delete_flag) {
				tmp = *e;
				*e = (*e)->next;
				free(tmp);
			}
		}
	}
}


void unit_delete(int team, int index) {
	struct UnitEntry **e;

	for (e = &ss->team[team].unit.unit; *e; e = &(*e)->next) {
		if ((*e)->map_index == index)
			continue;
		(*e)->delete_flag = 1;
		break;
	}
}


int unit_add(int team, enum UnitType type, int x, int y) {
	int index, id;
	struct UnitEntry *e;
	int success = 0;

	if (x < 0 || y < 0)
		goto fail;
	if (x >= ss->active_level->layer->tilemap->w || y >= ss->active_level->layer->tilemap->h)
		goto fail;
	
	index = x + y * ss->active_level->layer->tilemap->w;

	if ((ss->active_level->layer->tilemap->data[index] & TILESET_MASK) >= TILESET_UNIT_BASE)
		goto fail; // Unit already there
	e = malloc(sizeof(*e));
	e->create_flag = 1;
	e->modify_flag = 0;
	e->delete_flag = 0;
	e->previous_tile = ss->active_level->layer->tilemap->data[index];
	ss->active_level->layer->tilemap->data[index] = TILESET_UNIT_BASE + TILESET_TEAM_STEP * team;
	e->map_index = index;
	e->type = type;
	e->next = ss->team[team].unit.unit;
	ss->team[team].unit.unit = e;
	success = 1;

fail:
	return success;	
}


void unit_prepare() {
	int i, j;
	unsigned int tile, team;

	for (j = 0; j < ss->active_level->layer->tilemap->h; j++)
		for (i = 0; i < ss->active_level->layer->tilemap->w; i++) {
			tile = ss->active_level->layer->tilemap->data[j * ss->active_level->layer->tilemap->w + i] & TILESET_MASK;
			if (tile < TILESET_UNIT_BASE)
				continue;
			team = (tile - TILESET_UNIT_BASE) / TILESET_TEAM_STEP;
			if (((tile - TILESET_UNIT_BASE) % TILESET_TEAM_STEP) == UNIT_TYPE_SPAWN) {
				if (team >= MAX_TEAM)
					continue;
				ss->team[team].spawn.x = i * ss->active_level->layer->tile_w;
				ss->team[team].spawn.y = j * ss->active_level->layer->tile_h;
				printf("found team %i spawn point at %i, %i\n", team, i, j);
			}
		}
	return;
}


void unit_init() {
	int i;

	return;
}

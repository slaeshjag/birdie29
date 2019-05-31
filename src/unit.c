#include "main.h"
#include "unit.h"



void unit_housekeeping() {
	int i;
	struct UnitEntry **e, *tmp;

	for (i = 0; i < MAX_TEAM; i++) {
		d_util_mutex_lock(ss->team[i].unit.lock);
		
		for (e = &ss->team[i].unit.unit; *e; e = &(*e)->next) {
			if ((*e)->delete_flag) {
				tmp = *e;
				*e = (*e)->next;
				free(tmp);
			}
		}
		
		d_util_mutex_unlock(ss->team[i].unit.lock);
	}
}


void unit_delete(int team, int index) {
	struct UnitEntry **e;

	d_util_mutex_lock(ss->team[team].unit.lock);
	for (e = &ss->team[team].unit.unit; *e; e = &(*e)->next) {
		if ((*e)->map_index == index)
			continue;
		(*e)->delete_flag = 1;
		break;
	}
	
	d_util_mutex_unlock(ss->team[team].unit.lock);
}


void unit_add(int team, enum UnitType type, int x, int y) {
	int index, id;
	struct UnitEntry *e;

	d_util_mutex_lock(ss->team[team].unit.lock);

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
	
fail:
	d_util_mutex_unlock(ss->team[team].unit.lock);
	return;	
}


void unit_init() {
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
				ss->team[team].spawn.x = i;
				ss->team[team].spawn.y = j;
				printf("found team %i spawn point at %i, %i\n", team, i, j);
			}
		}

	for (i = 0; i < MAX_TEAM; i++)
		ss->team[i].unit.lock = d_util_mutex_create();
	return;
}

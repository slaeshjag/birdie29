#include "main.h"
#include "unit.h"



void unit_housekeeping() {
	int i;
	struct UnitEntry **e, *tmp;

	for (i = 0; i < MAX_TEAM; i++) {
		d_util_mutex_lock(s->team[i].unit.lock);
		
		for (e = &s->team[i].unit.unit; *e; e = &(*e)->next) {
			if ((*e)->delete_flag) {
				tmp = *e;
				*e = (*e)->next;
				free(tmp);
			}
		}
		
		d_util_mutex_unlock(s->team[i].unit.lock);
	}
}


void unit_delete(int team, int index) {
	struct UnitEntry **e;

	d_util_mutex_lock(s->team[team].unit.lock);
	for (e = &s->team[team].unit.unit; *e; e = &(*e)->next) {
		if ((*e)->map_index == index)
			continue;
		(*e)->delete_flag = 1;
		break;
	}
	
	d_util_mutex_unlock(s->team[team].unit.lock);
}


void unit_add(int team, enum UnitType type, int x, int y) {
	int index, id;
	struct UnitEntry *e;

	d_util_mutex_lock(s->team[team].unit.lock);

	if (x < 0 || y < 0)
		goto fail;
	if (x >= s->active_level->layer->tilemap->w || y >= s->active_level->layer->tilemap->h)
		goto fail;
	
	index = x + y * s->active_level->layer->tilemap->w;

	if ((s->active_level->layer->tilemap->data[index] & TILESET_MASK) >= TILESET_UNIT_BASE)
		goto fail; // Unit already there
	e = malloc(sizeof(*e));
	e->create_flag = 1;
	e->modify_flag = 0;
	e->delete_flag = 0;
	e->previous_tile = s->active_level->layer->tilemap->data[index];
	s->active_level->layer->tilemap->data[index] = TILESET_UNIT_BASE + TILESET_TEAM_STEP * team;
	e->map_index = index;
	e->type = type;
	e->next = s->team[team].unit.unit;
	s->team[team].unit.unit = e;
	
fail:
	d_util_mutex_unlock(s->team[team].unit.lock);
	return;	
}


void unit_init() {
	int i, j;
	unsigned int tile, team;

	TILESET_UNIT_BASE + TILESET_TEAM_STEP * team;

	for (j = 0; j < s->active_level->layer->tilemap->h; j++)
		for (i = 0; i < s->active_level->layer->tilemap->w; i++) {
			tile = s->active_level->layer->tilemap->data[j * s->active_level->layer->tilemap->w + i] & TILESET_MASK;
			if (tile < TILESET_UNIT_BASE)
				continue;
			team = (tile - TILESET_UNIT_BASE) / TILESET_TEAM_STEP;
			if (((tile - TILESET_UNIT_BASE) % TILESET_TEAM_STEP) == UNIT_TYPE_SPAWN) {
				if (team >= MAX_TEAM)
					continue;
				s->team[team].spawn.x = i;
				s->team[team].spawn.y = j;
				printf("found team %i spawn point at %i, %i\n", team, i, j);
			}
		}

	for (i = 0; i < MAX_TEAM; i++)
		s->team[i].unit.lock = d_util_mutex_create();
	return;
}

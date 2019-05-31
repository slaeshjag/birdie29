#include "powerpylon.h"
#include "main.h"
#include "team.h"

int static _unit_range() {
	return PYLON_RANGE;
}


int static _unit_neighbours(const int radius) {
	/* This will waste a lot of memory, but I just want to get this shit to work */
	return (radius*2+1)*(radius*2+1);
}



int _unit_pylon_list_remove(struct PylonEntry *from, struct PylonEntry *del) {
	int i;

	for (i = 0; i < from->neighbours; i++) {
		if (from->neighbour[i] == del) {
			from->neighbour[i] = NULL;
			return 0;
		}
	}

	return -1;
}


static void _unit_pylon_list_add(struct PylonEntry *to, struct PylonEntry *add) {
	int i, range, dx, dy;
	
	dx = add->x - to->x;
	dy = add->y - to->y;
	range = _unit_range();
	dx += range;
	dy += range;
	i = dx +dy * (range * 2 + 1);

	to->neighbour[i] = add;
	
	return;
}


static void _unit_pylon_pulse_climb(struct PylonEntry *pylon) {
	int i;
	
	if (pylon->pulse)
		return;
	pylon->pulse = 1;
	
	for (i = 0; i < pylon->neighbours; i++)
		if (pylon->neighbour[i])
			_unit_pylon_pulse_climb(pylon->neighbour[i]);

	return;
}


static void _unit_pylon_pulse() {
	int i;
	struct UnitEntry *next;

	for (i = 0; i < MAX_TEAM; i++) {
		_unit_pylon_pluse_climb(ss->team[i].generator);
	}

	for (i = 0; i < MAX_TEAM; i++) {
		for (next = ss->team[i].unit.unit; next; next = next->next) {
			if (!next->pylon)
				continue;
			if (next->pylon->pulse) {
				if (!next->pylon->power) {
					next->pylon->power= 1;
					//playerCalcSetPower(list->unit->owner, list->x, list->y, 1);
				}
			} else {
				if (next->pylon->power) {
					next->pylon->power = 0;
					//playerCalcSetPower(list->unit->owner, list->x, list->y, -1);
				}
			}

			next->pylon->pulse = 0;
		}
	}

	return;
}


void _unit_pylon_delete(struct UnitEntry *unit) {
	struct PylonEntry **list;
	int i;

	if (unit->pylon->power)
		playerCalcSetPower(unit->team, unit->pylon->x, unit->pylon->y, -1);
	
	for (i = 0; i < unit->pylon->neighbours; i++)
		if (unit->pylon->neighbour[i])
			_unit_pylon_list_remove(unit->pylon->neighbour[i], &unit->pylon);
	free(unit->pylon->neighbour);
	unit->pylon->neighbour = NULL;

	#if 0
	list = &server->pylons;

	if (server->pylons == NULL)
		return;

	while (*list) {
		if ((*list)->unit == unit) {
			*list = (*list)->next;
			return;
		}

		list = &(*list)->next;
	}
	#endif

	return;
}


void pylon_init(struct UnitEntry *unit, unsigned int x, unsigned int y) {
	int i, team, radius;
	struct UnitEntry *next;

	team = unit->team;
	radius = _unit_range();

	unit->pylon = malloc(sizeof(*unit->pylon));

	unit->pylon->x = x;
	unit->pylon->y = y;
	unit->pylon->pulse = 0;
	unit->pylon->power = 0;
	unit->pylon->neighbours = _unit_neighbours(_unit_range());
	unit->pylon->neighbour = malloc(sizeof(*unit->pylon->neighbour) * unit->pylon->neighbours);

	for (i = 0; i < unit->pylon->neighbours; i++)
		unit->pylon->neighbour[i] = NULL;

	for (next = ss->team[unit->team].unit.unit; next; next = next->next) {
		int dx, dy, pos_x, pos_y;
		if (!next->pylon)
			continue;
		if (next->pylon == unit->pylon)
			continue;
		pos_x = next->map_index % ss->active_level->layer->tilemap->w;
		pos_y = next->map_index / ss->active_level->layer->tilemap->w;
		dx = x - pos_x;
		dy = y - pos_y;
		if (dx*dx + dy*dy >= radius*radius)
			continue;
		if (next->pylon->power && !unit->pylon->power) {
			playerCalcSetPower(team, x, y, 1);
			unit->pylon->power = 1;
		}
		
		_unit_pylon_list_add(next->pylon, unit->pylon);
		_unit_pylon_list_add(unit->pylon, next->pylon);

	}

	
	unit->pylon->power = (unit->type == UNIT_TYPE_GENERATOR) ? 1 : unit->pylon->power;
	_unit_pylon_pulse();
	

	return;
}

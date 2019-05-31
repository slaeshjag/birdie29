#include "powerpylon.h"
#include "main.h"
#include "team.h"

#include "server/server.h"
#include "network/protocol.h"

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
		if (ss->team[i].generator)
			_unit_pylon_pulse_climb(ss->team[i].generator);
	}

	for (i = 0; i < MAX_TEAM; i++) {
		for (next = ss->team[i].unit.unit; next; next = next->next) {
			if (!next->pylon)
				continue;
			if (next->pylon->pulse) {
				if (!next->pylon->power) {
					next->pylon->power= 1;
					_pylon_recalc_diff(i, next->pylon->x, next->pylon->y, 1);
				}
			} else {
				if (next->pylon->power) {
					next->pylon->power = 0;
					_pylon_recalc_diff(i, next->pylon->x, next->pylon->y, -1);
				}
			}

			next->pylon->pulse = 0;
		}
	}

	return;
}


void unit_pylon_pulse() {
	return _unit_pylon_pulse();
}


void unit_pylon_delete(struct UnitEntry *unit) {
	int i;

	
	if (unit->pylon->power)
		_pylon_recalc_diff(unit->team, unit->pylon->x, unit->pylon->y, -1);
	
	for (i = 0; i < unit->pylon->neighbours; i++)
		if (unit->pylon->neighbour[i])
			_unit_pylon_list_remove(unit->pylon->neighbour[i], unit->pylon);
	free(unit->pylon->neighbour);
	unit->pylon->neighbour = NULL;
	free(unit->pylon);
	unit->pylon = NULL;
	_unit_pylon_pulse();

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
		if (dx*dx + dy*dy > radius*radius) {
			printf("Out of range (pos_x = %i, pos_y = %i, dx = %i, dy = %i\n", pos_x, pos_y, dx, dy);
			continue;
		}
		if (next->pylon->power && !unit->pylon->power) {
			_pylon_recalc_diff(unit->team, unit->pylon->x, unit->pylon->y, 1);
			unit->pylon->power = 1;
		}

		_unit_pylon_list_add(next->pylon, unit->pylon);
		_unit_pylon_list_add(unit->pylon, next->pylon);

	}

	
	//unit->pylon->power = (unit->type == UNIT_TYPE_GENERATOR) ? 1 : unit->pylon->power;
	printf("pylon!\n");
	_unit_pylon_pulse();
	

	return;
}



struct PylonPowerEntry *pylonpower_map_new(int w, int h) {
	struct PylonPowerMap *map;

	map = malloc(sizeof(*map));
	map->map = calloc(sizeof(*map->map) * w*h, 1);
	map->w = w;
	map->h = h;
	
	return map;
}


void _pylon_recalc_diff(int team, int x, int y, int diff) {
	/* TODO: Announce change to players */
	Packet pack;

	pack.type = PACKET_TYPE_POWER_EVENT;
	pack.size = sizeof(PacketPowerEvent);

	pack.power_event.team = team;
	pack.power_event.sign = diff;
	pack.power_event.x = x;
	pack.power_event.y = y;

	server_broadcast_packet(&pack);

	pylonpower_diff(ss->team[team].power_map, x, y, diff);
}



void pylonpower_diff(struct PylonPowerMap *map, int x, int y, int diff) {
	int i, j, k, index, range;
	
	range = _unit_range();

	for (j = -1 * range; j <= range; j++) {
		if (x + j < 0 || x + j >= map->w)
			continue;
		for (k = -1 * range; k <= range; k++) {
			if (y + k < 0 || y + k >= map->h)
				continue;
			if (j*j + k*k > range*range)
				continue;
			index = (y + k) * map->w + (x + j);
			map->map[index] += diff;
		}
	}
}

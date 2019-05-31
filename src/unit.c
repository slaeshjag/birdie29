#include "main.h"
#include "main.h"
#include "unit.h"
#include "network/protocol.h"
#include "server/server.h"

#define PLACE_TILE(X, Y, TYPE, TILE_POSITION, LAYER) do { \
	int tile = _unit_properties[TYPE].tiles.TILE_POSITION + (TILESET_TEAM_STEP) * team; \
	if(tile > 0) { \
		int index = (X) + (Y) * ss->active_level->layer[LAYER].tilemap->w; \
		ss->active_level->layer[LAYER].tilemap->data[index] = tile; \
		if(LAYER == MAP_LAYER_BUILDING_LOWER) { \
			ss->active_level->layer[0].tilemap->data[index] |= TILESET_COLLISION_MASK; \
		} \
		pack.x = (X); \
		pack.y = (Y); \
		pack.tile = tile; \
		pack.layer = LAYER; \
		server_broadcast_packet((void *) &pack); \
	} \
} while(0)

static UnitProperties _unit_properties[UNIT_TYPES] = {
	[UNIT_TYPE_GENERATOR] = {.tiles = { 104, 105, 96, 97 }, .cost = 100},
	[UNIT_TYPE_PYLON] = {.tiles = { 106, -1, 98, -1 }, .cost = 10},
	[UNIT_TYPE_MINER] = {.tiles = { 107, -1, -1, -1 }, .cost = 20},
	[UNIT_TYPE_TURRET] = {.tiles = { 108, -1, -1, -1 }, .cost = 50},
	[UNIT_TYPE_WALL] = {.tiles = { 109, -1, 101, -1 }, .cost = 5},
	[UNIT_TYPE_SPAWN] = {.tiles = { 110, -1, -1, -1 }, .cost = -1},
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


bool _collision_with_tile(int x, int y, UnitType type) {
	int index = x + y * ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->w;
	
	if(ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[index] && _unit_properties[type].tiles.bottom_left)
		return true;
	
	if(ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[index + 1] && _unit_properties[type].tiles.bottom_right)
		return true;
	
	
	return false;
}

int unit_add(int team, UnitType type, int x, int y) {
	int index, id;
	struct UnitEntry *e;
	int success = 0;

	if (x < 2 || y < 2)
		goto fail;
	if (x >= (ss->active_level->layer->tilemap->w - 2) || y >= (ss->active_level->layer->tilemap->h - 2))
		goto fail;
	
	if(ss->team[team].money < _unit_properties[type].cost)
		goto fail;
	
	if(_collision_with_tile(x, y, type))
		goto fail;
	
	ss->team[team].money -= _unit_properties[type].cost;
	
	//if ((ss->active_level->layer->tilemap->data[index] & TILESET_MASK) >= TILESET_UNIT_BASE)
	//	goto fail; // Unit already there
	e = malloc(sizeof(*e));
	e->create_flag = 1;
	e->modify_flag = 0;
	e->delete_flag = 0;
	
	e->map_index = index;
	e->type = type;
	e->next = ss->team[team].unit.unit;
	ss->team[team].unit.unit = e;
	success = 1;
	
	PacketTileUpdate pack;
	
	pack.type = PACKET_TYPE_TILE_UPDATE;
	pack.size = sizeof(PacketTileUpdate);
	
	PLACE_TILE(x, y, type, bottom_left, MAP_LAYER_BUILDING_LOWER);
	PLACE_TILE(x + 1, y, type, bottom_right, MAP_LAYER_BUILDING_LOWER);
	PLACE_TILE(x, y - 1, type, top_left, MAP_LAYER_BUILDING_UPPER);
	PLACE_TILE(x + 1, y - 1, type, top_right, MAP_LAYER_BUILDING_UPPER);

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
			if (((tile) % TILESET_TEAM_STEP) == ((_unit_properties[UNIT_TYPE_SPAWN].tiles.bottom_left) % TILESET_TEAM_STEP)) {
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

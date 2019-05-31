#include "main.h"
#include "main.h"
#include "unit.h"
#include "network/protocol.h"
#include "server/server.h"

#define PLACE_TILE(X, Y, TYPE, TILE_POSITION, LAYER) do { \
	if(_building_tiles[type].bottom_left > 0) { \
		int index = (X) + (Y) * ss->active_level->layer->tilemap->w; \
		int tile = _building_tiles[TYPE].TILE_POSITION + TILESET_TEAM_STEP * team; \
		ss->active_level->layer[LAYER].tilemap->data[index] = tile; \
		pack.x = (X); \
		pack.y = (Y); \
		pack.tile = tile; \
		pack.layer = LAYER; \
		server_broadcast_packet((void *) &pack); \
	} \
} while(0)


static struct UnitTiles _building_tiles[UNIT_TYPES] = {
	[UNIT_TYPE_GENERATOR] = { 104, 105, 96, 97 },
	[UNIT_TYPE_PYLON] = { 106, -1, 98, -1 },
	[UNIT_TYPE_MINER] = { 107, -1, -1, -1 },
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

	//if ((ss->active_level->layer->tilemap->data[index] & TILESET_MASK) >= TILESET_UNIT_BASE)
	//	goto fail; // Unit already there
	e = malloc(sizeof(*e));
	e->create_flag = 1;
	e->modify_flag = 0;
	e->delete_flag = 0;
	//e->previous_tile = ss->active_level->layer->tilemap->data[index];
	
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

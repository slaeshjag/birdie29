#include <stdbool.h>
#include "../main.h"
#include "unit.h"
#include "../network/protocol.h"
#include "server.h"
#include "unit/miner.h"
#include "unit/turret.h"

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

#define UNPLACE_TILE(X, Y, TYPE, TILE_POSITION, LAYER) do { \
	int tile = _unit_properties[TYPE].tiles.TILE_POSITION + (TILESET_TEAM_STEP) * team; \
	if(tile > 0) { \
		int index = (X) + (Y) * ss->active_level->layer[LAYER].tilemap->w; \
		ss->active_level->layer[LAYER].tilemap->data[index] = 0; \
		if(LAYER == MAP_LAYER_BUILDING_LOWER) { \
			ss->active_level->layer[0].tilemap->data[index] &= ~TILESET_COLLISION_MASK; \
		} \
		pack.x = (X); \
		pack.y = (Y); \
		pack.tile = 0; \
		pack.layer = LAYER; \
		server_broadcast_packet((void *) &pack); \
	} \
} while(0)

static int _special_func_miner(UnitEntry *unit);
static int _special_func_turret(UnitEntry *unit);

static UnitProperties _unit_properties[UNIT_TYPES] = {
	[UNIT_TYPE_GENERATOR] = {.tiles = { 104, 105, 96, 97 }, .cost = 100, .health = 100},
	[UNIT_TYPE_PYLON] = {.tiles = { 106, -1, 98, -1 }, .cost = 10, .health = 50},
	[UNIT_TYPE_MINER] = {.tiles = { 107, -1, -1, -1 }, .cost = 20, .health = 20, .special_function = unit_miner_special_function},
	[UNIT_TYPE_TURRET] = {.tiles = { 108, -1, -1, -1 }, .cost = 50, .health = 50, .special_function = unit_turret_special_function},
	[UNIT_TYPE_WALL] = {.tiles = { 109, -1, 101, -1 }, .cost = 5, .health = 50},
	[UNIT_TYPE_SPAWN] = {.tiles = { 110, -1, -1, -1 }, .cost = -1, .health = -1},
};

void unit_housekeeping() {
	int i;
	struct UnitEntry **e, *tmp;
	
	for (i = 0; i < MAX_TEAM; i++) {
		for (e = &ss->team[i].unit.unit; *e; ) {
			(*e)->powered = !!ss->team[(*e)->team].power_map->map[(*e)->map_index];
			if ((*e)->delete_flag) {
				tmp = *e;
				*e = (*e)->next;
				
				if (tmp->pylon)
					unit_pylon_delete(tmp);

				PacketTileUpdate pack;
				int x, y;
				int type;
				int team;
	
				pack.type = PACKET_TYPE_TILE_UPDATE;
				pack.size = sizeof(PacketTileUpdate);
				
				x = tmp->x;
				y = tmp->y;
				type = tmp->type;
				team = tmp->team;
				
				UNPLACE_TILE(x, y, type, bottom_left, MAP_LAYER_BUILDING_LOWER);
				UNPLACE_TILE(x + 1, y, type, bottom_right, MAP_LAYER_BUILDING_LOWER);
				UNPLACE_TILE(x, y - 1, type, top_left, MAP_LAYER_BUILDING_UPPER);
				UNPLACE_TILE(x + 1, y - 1, type, top_right, MAP_LAYER_BUILDING_UPPER);
				
				free(tmp);
			} else {
				if((*e)->special_function)
					(*e)->special_function((*e));
				
				e = &(*e)->next;
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

UnitEntry *unit_find_tile_owner(int x, int y) {
	int team;
	UnitEntry *e;
	
	for(team = 0; team < TEAMS_CAP; team++) {
		for(e = ss->team[team].unit.unit; e; e = e->next) {
			if(e->x == x && e->y == y) {
				return e;
			}
		}
	}
	
	return NULL;
}

void unit_damage(UnitEntry *unit, int damage) {
	if(unit->health < 0)
		return; //Probably invincible
	
	unit->health -= damage;
	
	if(unit->health <= 0)
		unit->delete_flag = true;
}

bool _collision_with_tile(int x, int y, UnitType type) {
	int index = x + y * ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->w;
	
	if(ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[index] && (_unit_properties[type].tiles.bottom_left >= 0))
		return true;
	
	if(ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[index + 1] && (_unit_properties[type].tiles.bottom_right >= 0)) {
		printf("arne\n");
		return true;
	}
	
	
	return false;
}

bool _miner_on_resource(int x, int y, UnitType type) {
	int index = x + y * ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->w;
	
	if(type != UNIT_TYPE_MINER)
		return false;
	
	if(ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[index] == TILE_RESOURCE)
		return true;
	
	return false;
}

bool _manual_mining(int x, int y, UnitType type) {
	int index = x + y * ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->w;
	
	if(type != UNIT_TYPE_GENERATOR)
		return false;
	
	if(ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[index] == TILE_RESOURCE)
		return true;
	
	return false;
}

bool _is_powered(int x, int y, int team) {
	int index = x + y * ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->w;
	
	if(ss->team[team].power_map->map[index])
		return true;
	
	return false;
}

int unit_add(int team, UnitType type, int x, int y, bool force) {
	int index, id;
	struct UnitEntry *e;
	int success = 0;

	if(!force) {
		if (x < 2 || y < 2)
			goto fail;
		if (x >= (ss->active_level->layer->tilemap->w - 2) || y >= (ss->active_level->layer->tilemap->h - 2))
			goto fail;
		
		if(_collision_with_tile(x, y, type)) {
			if(!_miner_on_resource(x, y, type)) {
				if(_manual_mining(x, y, type)) {
					/* Manually mine a resource */
					ss->team[team].money++;
				}
				goto fail;
			}
		} else {
			if(type == UNIT_TYPE_MINER || type == UNIT_TYPE_GENERATOR)
				goto fail;
		}
		
		if(ss->team[team].money < _unit_properties[type].cost)
			goto fail;
		
		if(!_is_powered(x, y, team))
			goto fail;
		
		ss->team[team].money -= _unit_properties[type].cost;
	}
	
	//if ((ss->active_level->layer->tilemap->data[index] & TILESET_MASK) >= TILESET_UNIT_BASE)
	//	goto fail; // Unit already there
	e = malloc(sizeof(*e));
	e->create_flag = 1;
	e->modify_flag = 0;
	e->delete_flag = 0;
	
	e->pylon = NULL;
	e->map_index = x + y*ss->active_level->layer->tilemap->w;;
	e->x = x;
	e->y = y;
	e->team = team;
	e->type = type;
	e->team = team;
	
	e->health = _unit_properties[type].health;
	
	e->special_function = _unit_properties[type].special_function;
	e->special_data = NULL;
	
	e->next = ss->team[team].unit.unit;
	ss->team[team].unit.unit = e;
	success = 1;

	if (e->type == UNIT_TYPE_PYLON)
		pylon_init(e, x, y);
	if (e->type == UNIT_TYPE_GENERATOR)
		pylon_init(e, x, y), ss->team[team].generator = e->pylon, unit_pylon_pulse();
	
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
			if((ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[j * ss->active_level->layer->tilemap->w + i] & TILESET_MASK) == TILE_RESOURCE) {
				printf("found resource %i at (%i %i)\n", ss->active_level->layer[MAP_LAYER_BUILDING_LOWER].tilemap->data[j * ss->active_level->layer->tilemap->w + i] & TILESET_MASK, i, j);
				ss->active_level->layer[MAP_LAYER_TERRAIN].tilemap->data[j * ss->active_level->layer->tilemap->w + i] |= TILESET_COLLISION_MASK;
				continue;
			}
			
			tile = ss->active_level->layer->tilemap->data[j * ss->active_level->layer->tilemap->w + i] & TILESET_MASK;
			if (tile < TILESET_UNIT_BASE)
				continue;
			team = (tile - TILESET_UNIT_BASE) / TILESET_TEAM_STEP;
			if (team >= MAX_TEAM)
				continue;
			
			/* Spawn plate */
			if (((tile) % TILESET_TEAM_STEP) == ((_unit_properties[UNIT_TYPE_SPAWN].tiles.bottom_left) % TILESET_TEAM_STEP)) {
				ss->team[team].spawn.x = i * ss->active_level->layer->tile_w;
				ss->team[team].spawn.y = j * ss->active_level->layer->tile_h;
				printf("found team %i spawn point at %i, %i\n", team, i, j);
			}
			
			/* Generator */
			if (((tile) % TILESET_TEAM_STEP) == 15) {
				printf("found generator for team at %i, %i\n", team, i, j);
				
				/* Base plate */
				ss->active_level->layer->tilemap->data[j * ss->active_level->layer->tilemap->w + i] &= ~TILESET_MASK;
				ss->active_level->layer->tilemap->data[j * ss->active_level->layer->tilemap->w + i] |= 2;
				
				ss->active_level->layer->tilemap->data[j * ss->active_level->layer->tilemap->w + i + 1] &= ~TILESET_MASK;
				ss->active_level->layer->tilemap->data[j * ss->active_level->layer->tilemap->w + i + 1] |= 2;
				
				unit_add(team, UNIT_TYPE_GENERATOR, i, j, true);
				
				PacketTileUpdate pack;
				int x, y;
				int type;
				int team;
	
				pack.type = PACKET_TYPE_TILE_UPDATE;
				pack.size = sizeof(PacketTileUpdate);
				pack.layer = 0;
				
				pack.x = i;
				pack.y = j;
				pack.tile = 2;
				server_broadcast_packet((void *) &pack);
				pack.x++;
				server_broadcast_packet((void *) &pack);
			}
		}
	return;
}


void unit_init() {
	int i;

	return;
}

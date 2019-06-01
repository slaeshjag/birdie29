#ifndef UNIT_H_
#define	UNIT_H_

#define	TILESET_UNIT_BASE		(96 + 8)
#define	TILESET_TEAM_STEP		16
#define	TILESET_MASK			0xFFF
#define	TILESET_COLLISION_MASK (0xF << 16)

#define TILE_RESOURCE (160)

#include <stdbool.h>
#include <darnit/darnit.h>
#include "powerpylon.h"

typedef enum UnitType UnitType;
typedef struct UnitTiles UnitTiles;
typedef struct UnitProperties UnitProperties;
typedef struct UnitEntry UnitEntry;
typedef struct Unit Unit;


enum UnitType {
	UNIT_TYPE_GENERATOR,
	UNIT_TYPE_PYLON,
	UNIT_TYPE_MINER,
	UNIT_TYPE_WALL,
	UNIT_TYPE_TURRET,
	UNIT_TYPE_SPAWN,
	UNIT_TYPES,
};



struct UnitTiles {
	unsigned int			bottom_left;
	unsigned int			bottom_right;
	unsigned int			top_left;
	unsigned int			top_right;
};


struct UnitProperties {
	UnitTiles tiles;
	
	int cost;
	int health;
	
	int (*special_function)(UnitEntry *unit);
};

struct UnitEntry {
	int				map_index;
	int				x;
	int				y;
	int				team;
	unsigned int			previous_tile;
	UnitType			type;
	UnitEntry		*next;
	int health;
	struct PylonEntry		*pylon;
	int				powered;
	
	int (*special_function)(UnitEntry *unit);
	void *special_data;
	
	int				create_flag;
	int				modify_flag;
	int				delete_flag;
};

struct Unit {
	UnitEntry		*unit;
};


void unit_init();
void unit_prepare();
int unit_add(int team, UnitType type, int x, int y, bool force);
void unit_delete(int team, int index);
void unit_housekeeping();
UnitEntry *unit_find_tile_owner(int x, int y);
void unit_damage(UnitEntry *unit, int damage);


#endif

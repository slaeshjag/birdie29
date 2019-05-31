#ifndef UNIT_H_
#define	UNIT_H_

#define	TILESET_UNIT_BASE		(96 + 8)
#define	TILESET_TEAM_STEP		16
#define	TILESET_MASK			0xFFF

#include <darnit/darnit.h>

typedef enum UnitType UnitType;
enum UnitType {
	UNIT_TYPE_GENERATOR,
	UNIT_TYPE_PYLON,
	UNIT_TYPE_MINER,
	UNIT_TYPE_WALL,
	UNIT_TYPE_TURRET,
	UNIT_TYPE_SPAWN,
	UNIT_TYPES,
};


typedef struct UnitTiles UnitTiles;
struct UnitTiles {
	unsigned int			bottom_left;
	unsigned int			bottom_right;
	unsigned int			top_left;
	unsigned int			top_right;
};

typedef struct UnitProperties UnitProperties;
struct UnitProperties {
	UnitTiles tiles;
	
	int cost;
};

typedef struct UnitEntry UnitEntry;
struct UnitEntry {
	int				map_index;
	unsigned int			previous_tile;
	UnitType			type;
	UnitEntry		*next;

	int				create_flag;
	int				modify_flag;
	int				delete_flag;
};

typedef struct Unit unit;
struct Unit {
	UnitEntry		*unit;
};


void unit_init();
void unit_prepare();
int unit_add(int team, UnitType type, int x, int y);
void unit_delete(int team, int index);
void unit_housekeeping();


#endif

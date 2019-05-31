#ifndef UNIT_H_
#define	UNIT_H_

#define	TILESET_UNIT_BASE		96
#define	TILESET_TEAM_STEP		8
#define	TILESET_MASK			0xFFF

#include <darnit/darnit.h>

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


struct UnitEntry {
	int				map_index;
	unsigned int			previous_tile;
	enum UnitType			type;
	struct UnitEntry		*next;

	int				create_flag;
	int				modify_flag;
	int				delete_flag;
};


struct Unit {
	struct UnitEntry		*unit;
};


void unit_init();
void unit_prepare();
int unit_add(int team, enum UnitType type, int x, int y);
void unit_delete(int team, int index);
void unit_housekeeping();


#endif

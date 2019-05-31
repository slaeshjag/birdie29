#ifndef POWERPYLON_H_
#define	POWERPYLON_H_

#include "unit.h"

#define	PYLON_RANGE			7


struct PylonEntry {
	unsigned int			power;
	unsigned int			pulse;
	unsigned int			x;
	unsigned int			y;
	struct PylonEntry		**neighbour;
	int				neighbours;
};


struct PylonPowerMap {
	int				w;
	int				h;
	unsigned int			*map;
};


//void unit_pylon_delete(struct UnitEntry *unit);
//void pylon_init(struct UnitEntry *unit, unsigned int x, unsigned int y);
void pylonpower_diff(struct PylonPowerMap *map, int x, int y, int diff);
struct PylonPowerEntry *pylonpower_map_new(int w, int h);



#endif

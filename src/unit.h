#ifndef UNIT_H_
#define	UNIT_H_


enum UnitType {
	UNIT_TYPE_GENERATOR,
	UNIT_TYPE_PYLON,
	UNIT_TYPE_WALL,
	UNIT_TYPES,
};


struct UnitEntry {
	int				map_index;
	enum UnitType			type;
};


struct Unit {
	struct UnitEntry		*unit;
	int				units;
};


#endif

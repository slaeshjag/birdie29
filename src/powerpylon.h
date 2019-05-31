#ifndef POWERPYLON_H_
#define	POWERPYLON_H_


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



#endif

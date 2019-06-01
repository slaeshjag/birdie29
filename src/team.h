#ifndef TEAM_H_
#define	TEAM_H_

#include "unit.h"
#include "config.h"
#include "powerpylon.h"

#define	MAX_TEAM TEAMS_CAP

struct Team {
	struct Unit		unit;
	struct {
		int		x;
		int		y;
	} spawn;
	int money;
	int miner_income_counter;

	struct PylonEntry	*generator;
	struct PylonPowerMap	*power_map;
};


#endif

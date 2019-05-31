#ifndef TEAM_H_
#define	TEAM_H_

#include "unit.h"
#include "config.h"

#define	MAX_TEAM TEAMS_CAP

struct Team {
	struct Unit		unit;
	int money;
};


#endif

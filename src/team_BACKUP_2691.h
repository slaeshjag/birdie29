#ifndef TEAM_H_
#define	TEAM_H_

#include "unit.h"
#include "config.h"

#define	MAX_TEAM TEAMS_CAP

struct Team {
	struct Unit		unit;
<<<<<<< HEAD
	struct {
		int		x;
		int		y;
	} spawn;
=======
	int money;
>>>>>>> 14e0666fbe68aea3a670d9d8ae7ccf31eacb08fd
};


#endif

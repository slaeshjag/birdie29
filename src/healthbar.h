#ifndef _HEALTHBAR_H__
#define	_HEALTHBAR_H__

#include <darnit/darnit.h>

struct HealthbarStruct {
	DARNIT_TILESHEET	*ts;
	DARNIT_TILE		*bar[4];
	int			hp[4];
};


void healthbar_init();
void healthbar_draw();
void healthbar_set(int id, int hp);


#endif

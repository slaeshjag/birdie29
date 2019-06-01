#include <math.h>
#include "../../config.h"
#include "../server.h"
#include "../movable.h"
#include "../../util.h"
#include "../unit.h"
#include "turret.h"

typedef struct UnitTurretData UnitTurretData;
struct UnitTurretData {
	int cooldown;
};

int unit_turret_special_function(UnitEntry *unit) {
	UnitTurretData *data;
	int team;
	int tilew, tileh;
	
	if(!unit->powered)
		return;
	
	tilew = ss->active_level->layer->tile_w;
	tileh = ss->active_level->layer->tile_h;

	if(!unit->special_data)
		unit->special_data = calloc(1, sizeof(UnitTurretData));
	
	data = unit->special_data;
	
	if((data->cooldown--) < 0) {
		data->cooldown = TURRET_COOLDOWN;
		Client *c;
	
		for(c = server_get_client_list(); c; c = c->next) {
			int x, y;
			if(c->team == unit->team)
				continue;
			
			x = ss->movable.movable[c->movable].x/1000;
			y = ss->movable.movable[c->movable].y/1000;
			
			if(util_distance(x, y, unit->x*tilew, unit->y*tileh) < TURRET_DISTANCE) {
				int angle = atan2(y - unit->y*tileh, x - unit->x*tilew)*180/M_PI/* + 180*/;
			
				bullet_spawn_from_movable(BULLET_TYPE_WIMPY, unit->x*tilew, unit->y*tileh, angle, -1);
				return 0;
			}
		}
	}
	
	
	
	return 0;
}

#include "serverplayer.h"
#include "server.h"
#include "../main.h"
#include "../particles.h"

void serverplayer_loop(Client *client) {
	Client *next;

	for (next = client; next; next = next->next) {
		if (next->hp <= 0) {
			/* Kill player */

			/* TODO: Wait with moving the playing for a second or so for the splatter to show */
			server_particle_spawn(ss->movable.movable[next->movable].x / 1000, ss->movable.movable[next->movable].x / 1000, 0, PARTICLE_TYPE_BLOOD);
			ss->movable.movable[next->movable].x = ss->team[next->team].spawn.x * 1000;
			ss->movable.movable[next->movable].y = ss->team[next->team].spawn.y * 1000;
			next->hp = PLAYER_HP;
		}
	}
}

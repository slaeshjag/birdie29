#include "serverplayer.h"
#include "server/server.h"
#include "main.h"

void serverplayer_loop(Client *client) {
	Client *next;

	for (next = client; next; next = next->next) {
		if (next->hp <= 0) {
			/* Kill player */

			/* TODO: Wait with moving the playing for a second or so for the splatter to show */
			ss->movable.movable[next->movable].x = ss->team[next->team].spawn.x * 1000;
			ss->movable.movable[next->movable].y = ss->team[next->team].spawn.y * 1000;
			next->hp = PLAYER_HP;
		}
	}
}

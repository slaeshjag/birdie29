#ifndef INGAME_H_
#define INGAME_H_

#include <stdbool.h>

//#define	TIMER_COUNTDOWN_WIN	90

void ingame_init();
void ingame_loop();


typedef struct InGameKeyStateEntry InGameKeyStateEntry;
struct InGameKeyStateEntry {
	bool			left;
	bool			right;
	bool			up;
	bool			down;
	bool			shoot;
	bool			shoot_special;
	bool			cycle_building;
	bool			build;
	bool			suicide;
};


//InGameKeyStateEntry ingame_keystate[PLAYER_CAP];

void ingame_network_handler();
void ingame_client_keyboard();

#endif

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include <darnit/darnit.h>

//#include "sfx.h"
//#include "effect.h"

#include "player.h"
#include "movable.h"
#include "drawable.h"
#include "config.h"
#include "team.h"

typedef struct Color Color;
struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

typedef enum GameState GameState;
enum GameState {
	GAME_STATE_MENU,
	GAME_STATE_SELECT_NAME,
	GAME_STATE_CHARACTERS,
	GAME_STATE_LOBBY,
	GAME_STATE_ENTER_IP,
	GAME_STATE_HOST,
	GAME_STATE_GAMEROOM,
	GAME_STATE_GAME,
	GAME_STATE_GAME_OVER,
	GAME_STATE_QUIT,
	GAME_STATES,
};

typedef struct Gfx Gfx;
struct Gfx {
	struct {
		DARNIT_FONT *small;
		DARNIT_FONT *large;
	} font;
};

extern Gfx gfx;

typedef struct GameStateStruct GameStateStruct;
struct GameStateStruct {
	bool is_host;
	Player *player[PLAYER_CAP];
	struct Team team[MAX_TEAM];
	MOVABLE movable;
	DARNIT_MAP *active_level;
	
	//SfxStruct sfx;
	
};

typedef struct ClientStateStruct ClientStateStruct;
struct ClientStateStruct {
	Player *player[PLAYER_CAP];
	struct Team team[MAX_TEAM];
	int server_sock;
	struct Drawable *drawable;
	DARNIT_MAP *active_level;
	
	struct {
		int x;
		int y;
		int follow;
	} camera;
};

extern GameStateStruct *ss;
extern ClientStateStruct *cs;
extern Player me;

extern char *team_name[TEAMS_CAP];

void game_state(GameState state);
void restart_to_menu(const char *name);
int join_game(unsigned long sip);

#endif

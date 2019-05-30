#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include <darnit/darnit.h>

//#include "sfx.h"
//#include "effect.h"

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720
#define PORT 1341

#define GAME_TITLE "The Game"

#define NAME_LEN_MAX 64
#define PLAYER_CAP 32

#define PANE_R 0xCD
#define PANE_G 0xCD
#define PANE_B 0xCD

#include "player.h"
#include "movable.h"

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
	int	player_id;
	bool is_host;
	Player player[PLAYER_CAP];
	int server_sock;
	MOVABLE movable;
	DARNIT_MAP *active_level;
	
	//SfxStruct sfx;
	struct {
		int x;
		int y;
		int follow;
	} camera;
};

extern GameStateStruct *s;
extern Player me;

void game_state(GameState state);
void restart_to_menu(const char *name);
int join_game(unsigned long sip);

#endif

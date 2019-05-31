#ifndef CONFIG_H_
#define CONFIG_H_

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720
#define PORT 1341

#define GAME_TITLE "The Game"

#define NAME_LEN_MAX 64
#define PLAYER_CAP 32
#define PLAYER_VARIANTS 4

#define TEAMS_CAP 4

#define MONEY_START 100


#define PANE_R 0xCD
#define PANE_G 0xCD
#define PANE_B 0xCD

#define PLAYER_SPEED 500
#define	PLAYER_HP 50

#define MAP_NAME_LEN_MAX 255

typedef enum MapLayer MapLayer;
enum MapLayer {
	MAP_LAYER_TERRAIN,
	MAP_LAYER_BUILDING_LOWER,
	MAP_LAYER_BUILDING_UPPER,
	MAP_LAYER_OVL_POWER,
	MAP_LAYER_OVL_MISC,
	MAP_LAYERS,
};

#endif

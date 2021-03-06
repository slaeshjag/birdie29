typedef struct Client Client;
typedef struct GameStateStruct GameStateStruct;

#ifndef SERVER_H_
#define SERVER_H_

#include "../config.h"
#include "../ingame.h"
#include "../network/protocol.h"
#include "movable.h"
#include "team.h"
#include "bullet.h"

struct Client {
	int id;
	int sock;
	char name[NAME_LEN_MAX];
	int team;
	int sprite_variant;
	int movable;
	int angle;
	int hp;

	InGameKeyStateEntry keystate;
	struct {
		int x;
		int y;
	} highlight;
	
	Client *next;
};

struct GameStateStruct {
	bool is_host;
	struct Team team[MAX_TEAM];
	MOVABLE movable;
	DARNIT_MAP *active_level;
	Bullet *bullet;
	int grace_counter;
	//SfxStruct sfx;
	
};

extern GameStateStruct *ss;

void server_start();
void server_start_game();
void server_kick();
void server_announce_winner(int winning_player);
//void server_sound(enum SoundeffectSound sound);
void server_broadcast_packet(Packet *pack);
bool server_player_is_present(int id);
void server_shutdown();
Client *server_get_client_list();
void server_particle_spawn(int x, int y, int angle, enum ParticleType type);

#endif

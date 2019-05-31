#ifndef SERVER_H_
#define SERVER_H_

#include "../config.h"
#include "../ingame.h"
#include "../network/protocol.h"

typedef struct Client Client;
struct Client {
	int id;
	int sock;
	char name[NAME_LEN_MAX];
	int team;
	int movable;
	int angle;
	
	InGameKeyStateEntry keystate;
	
	Client *next;
};

void server_start();
void server_start_game();
void server_kick();
void server_announce_winner(int winning_player);
//void server_sound(enum SoundeffectSound sound);
void server_broadcast_packet(Packet *pack);
bool server_player_is_present(int id);
void server_shutdown();

#endif

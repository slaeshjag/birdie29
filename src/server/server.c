#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <darnit/darnit.h>
#include "server.h"
#include "../main.h"
#include "../bullet.h"
#include "../network/network.h"
#include "../network/protocol.h"
#include "../serverplayer.h"

#define XSTR(s) STR(s)
#define STR(s) #s

#define HANDLE_KEY(A) do { \
		if(pack.keypress.keypress.A ) { \
			printf("server: player %i press %s\n", cli->id, STR(A)); \
			cli->keystate.A = 1; \
		} if(pack.keypress.keyrelease.A ) {\
			printf("server: player %i release %s\n", cli->id, STR(A)); \
			cli->keystate.A = 0; \
		} \
	} while(0)


int usleep(useconds_t usec);

typedef enum ServerState ServerState;
enum ServerState {
	SERVER_STATE_LOBBY,
	SERVER_STATE_STARTING,
	SERVER_STATE_GAME,
};

static int listen_sock;
static Client *client = NULL;
static int clients = 0;
static volatile ServerState server_state;

DARNIT_SEMAPHORE *sem;

static void _client_handle_keys(Client *player) {
	MOVABLE_ENTRY *m = &ss->movable.movable[player->movable];
	
	if(player->keystate.up) {
		//printf("server loop: player %i hold up\n", player->id);
		m->y_velocity = -PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_UP;
	} else if(player->keystate.down) {
		//printf("server loop: player %i hold down\n", player->id);
		m->y_velocity = PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_DOWN;
	} else {
		m->y_velocity = 0;
	}
	
	if(player->keystate.left) {
		//printf("server loop: player %i hold left\n", player->id);
		m->x_velocity = -PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_LEFT;
	} else if(player->keystate.right) {
		//printf("server loop: player %i hold right\n", player->id);
		m->x_velocity = PLAYER_SPEED;
		m->direction = PLAYER_DIRECTION_RIGHT;
	} else {
		m->x_velocity = 0;
	}
}

void _client_highlight_cursor(Client *player) {
	PacketTileUpdate pack = {.type = PACKET_TYPE_TILE_UPDATE, .size = sizeof(PacketTileUpdate)};
	int tile_size = ss->active_level->layer[0].tile_w;
	int highlight_x;
	int highlight_y;
	
	//mouse = d_mouse_get();
	highlight_x = ((ss->movable.movable[player->movable].x + (tile_size/2 - 1)*1000)/tile_size)/1000;
	highlight_y = ((ss->movable.movable[player->movable].y + (tile_size/2 - 1)*1000)/tile_size)/1000;
	//mouse.x
	
	highlight_x += 2.0*cos(player->angle*M_PI/180.0) + 0.5;
	highlight_y += 2.0*sin(player->angle*M_PI/180.0) + 0.5;
	
	pack.x = player->highlight.x;
	pack.y = player->highlight.y;
	pack.tile = 0;
	pack.layer = 2;
	
	protocol_send_packet(player->sock, (Packet *) &pack);
	
	player->highlight.x = highlight_x;
	player->highlight.y = highlight_y;
	
	pack.x = player->highlight.x;
	pack.y = player->highlight.y;
	pack.tile = 69;
	pack.layer = 2;
	
	protocol_send_packet(player->sock, (Packet *) &pack);
}

void server_handle_client(Client *cli) {
	Packet pack;
	Packet response;
	Client *tmp;
	int i;
	
	if(server_state == SERVER_STATE_GAME) {
		_client_handle_keys(cli);
		_client_highlight_cursor(cli);
	}
	
	while(network_poll_tcp(cli->sock)) {
		protocol_recv_packet(cli->sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_JOIN:
				strcpy(cli->name, pack.join.name);
				cli->team = pack.join.team;
				cli->hp = PLAYER_HP;
				printf("server: join %s team %i\n", cli->name, cli->team);
				
				response.type = PACKET_TYPE_JOIN;
				response.size = sizeof(PacketJoin);
				if (cli->movable > -1)
					movableDespawn(cli->movable);
				cli->movable = movableSpawnSprite(ss->team[cli->team].spawn.x, ss->team[cli->team].spawn.y, 0, /*TODO: Replace with sprite type */ 0);
				
				for(tmp = client; tmp; tmp = tmp->next) {
					memset(response.join.name, 0, NAME_LEN_MAX);
					strcpy(response.join.name, tmp->name);
					response.join.team = tmp->team;
					response.join.id = tmp->id;
					response.join.movable = tmp->movable;
					cli->movable = response.join.movable;
					protocol_send_packet(cli->sock, &response);
					
					if(tmp->sock != cli->sock) {
						response.join.id = cli->id;
						strcpy(response.join.name, cli->name);
						response.join.team = cli->team;
						protocol_send_packet(tmp->sock, &response);
					}
				}
				
				break;
			
			case PACKET_TYPE_MAP_CHANGE:
				response.type = PACKET_TYPE_MAP_CHANGE;
				response.size = sizeof(PacketMapChange);
				
				for(i = 0; i < TEAMS_CAP; i++) {
					ss->team[i].money = MONEY_START;
				}
				
				{
					char name[MAP_NAME_LEN_MAX + 10];
					snprintf(name, MAP_NAME_LEN_MAX + 10, "map/%s", pack.map_change.name);
					ss->active_level = d_map_load(name);
				}
				
				strcpy(response.map_change.name, pack.map_change.name);
				response.map_change.w = ss->active_level->layer[0].tilemap->w;
				response.map_change.h = ss->active_level->layer[0].tilemap->h;
					
				for(tmp = client; tmp; tmp = tmp->next) {
					protocol_send_packet(cli->sock, &response);
				}
				
				response.type = PACKET_TYPE_TILE_UPDATE;
				response.size = sizeof(PacketTileUpdate);
				response.tile_update.layer = 0;
					
				for(tmp = client; tmp; tmp = tmp->next) {
					uint32_t x, y;
					
					for(x = 0; x < response.map_change.w; x++) {
						for(y = 0; y < response.map_change.h; y++) {
							response.tile_update.x = x;
							response.tile_update.y = y;
							
							response.tile_update.tile = ss->active_level->layer[0].tilemap->data[y*response.map_change.w + x];
							protocol_send_packet(cli->sock, &response);
						}
					}
				}
				
				unit_prepare();

				break;
			
			case PACKET_TYPE_KEYPRESS:
				HANDLE_KEY(left);
				HANDLE_KEY(right);
				HANDLE_KEY(up);
				HANDLE_KEY(down);
				
				if(pack.keypress.keypress.shoot) {
					printf("server: shoot %i\n", bullet_spawn(BULLET_TYPE_WIMPY, cli));
					
				}
				
				cli->angle = pack.keypress.mouse_angle;
				
				break;
			
			case PACKET_TYPE_TILE_UPDATE:
				break;
			
			case PACKET_TYPE_PARTICLE:
				response.type = PACKET_TYPE_PARTICLE;
				response.size = sizeof(PacketParticle);
				response.particle.x = pack.particle.x;
				response.particle.y = pack.particle.y;
				response.particle.effect_type = pack.particle.effect_type;

				for(tmp = client; tmp; tmp = tmp->next) {
					protocol_send_packet(tmp->sock, &response);
				}
				break;
			case PACKET_TYPE_TIMER:
				response.type = PACKET_TYPE_TIMER;
				response.size = sizeof(PacketTimer);
				response.timer.time_left = pack.timer.time_left;
				
				for(tmp = client; tmp; tmp = tmp->next) {
					protocol_send_packet(tmp->sock, &response);
				}
				break;
			case PACKET_TYPE_BULLET_ANNOUNCE:
				break;
			case PACKET_TYPE_BULLET_UPDATE:
				break;
			case PACKET_TYPE_EXIT:
				response.type = PACKET_TYPE_EXIT;
				response.size = sizeof(PacketExit);
				
				for(tmp = client; tmp; tmp = tmp->next) {
					protocol_send_packet(tmp->sock, &response);
				}
				break;
			case PACKET_TYPE_BULLET_REMOVE:
				break;
			default:
				fprintf(stderr, "wat %i\n", pack.type);
				break;
		}
	}
}

int server_thread(void *arg) {
	Packet pack;
	Client *tmp;
	int i;
	
	for(;;) {
		switch(server_state) {
			case SERVER_STATE_LOBBY:
				if(network_poll_tcp(listen_sock)) {
					int sock;
					
					sock = network_accept_tcp(listen_sock);
					
					tmp = malloc(sizeof(Client));
					tmp->sock = sock;
					tmp->id = clients++;
					tmp->next = client;
					tmp->movable = -1;
					client = tmp;
				}
				
				for(tmp = client; tmp; tmp = tmp->next)
					server_handle_client(tmp);
				
				
				pack.type = PACKET_TYPE_LOBBY;
				pack.size = sizeof(PacketLobby);
				memset(pack.lobby.name, 0, NAME_LEN_MAX);
				strcpy(pack.lobby.name, me.name);
				
				network_broadcast_udp(&pack, pack.size);
				usleep(100000);
				break;
				
			case SERVER_STATE_STARTING:
				printf("server: starting...\n");
				for(tmp = client; tmp; tmp = tmp->next) {
					/* teleport players to their spawning point */
					ss->movable.movable[client->movable].x = ss->team[client->team].spawn.x * 1000;
					ss->movable.movable[client->movable].y = ss->team[client->team].spawn.y * 1000;

					pack.type = PACKET_TYPE_START;
					pack.size = sizeof(PacketStart);
					
					pack.start.player_id = tmp->id;
					
					protocol_send_packet(tmp->sock, (void *) &pack);
				}
				
				server_state = SERVER_STATE_GAME;
				break;
				
			case SERVER_STATE_GAME:
				d_util_semaphore_wait(sem);
				
				bullet_loop();
				movableLoop();
				serverplayer_loop(client);

				pack.type = PACKET_TYPE_MOVABLE_MOVE;
				pack.size = sizeof(PacketMovableMove);
				
				for(i = 0; i < ss->movable.movables; i++) {
					if(!ss->movable.movable[i].used)
						continue;
					
					int angle;
					pack.movable_move.x = ss->movable.movable[i].x/1000;
					pack.movable_move.y = ss->movable.movable[i].y/1000;
					pack.movable_move.dir = ss->movable.movable[i].direction;
					pack.movable_move.movable = i;

					angle = ss->movable.movable[i].angle;
					if (angle < 0)
						angle += 3600;
					pack.movable_move.angle = (angle / 10 / 2);
					
					for(tmp = client; tmp; tmp = tmp->next) {
						protocol_send_packet(tmp->sock, &pack);
					}
				}

				for (i = 0; i < MAX_TEAM; i++) {
					struct UnitEntry *ue;
					
					for (ue = ss->team[i].unit.unit; ue; ue = ue->next) {
						if (ue->create_flag); // Just created
						if (ue->modify_flag); // Has been modified since last loop
						if (ue->delete_flag); // Will be deleted at the end of the loop
					}
				}
				
				for(tmp = client; tmp; tmp = tmp->next)
					server_handle_client(tmp);
				unit_housekeeping();
				break;
		}
	}
	
	return 0;
}

void server_start() {
	sem = d_util_semaphore_new(0);
	server_state = SERVER_STATE_LOBBY;
	if((listen_sock = network_listen_tcp(PORT + 1)) < 0) {
		fprintf(stderr, "Server failed to open listening socket\n");
		exit(1);
	}
	d_util_thread_new(server_thread, NULL);
}

/*void server_sound(enum SoundeffectSound sound) {
	PacketSound pack;
	Client *tmp;

	pack.type = PACKET_TYPE_SOUND;
	pack.size = sizeof(pack);
	pack.sound = sound;
	for (tmp = client; tmp; tmp = tmp->next)
		protocol_send_packet(tmp->sock, &pack);
	return;
}*/


void server_start_game() {
	server_state = SERVER_STATE_STARTING;
}

void server_kick() {
	d_util_semaphore_add(sem, 1);
}

bool server_player_is_present(int id) {
	struct Client *next;

	for (next = client; next; next = next->next)
		if (next->id == id)
			return true;
	return false;
}

void server_shutdown() {
	Client *tmp;
	
	network_close_udp();
	
	for(tmp = client; tmp; tmp = tmp->next)
		network_close_tcp(tmp->sock);
	
	network_close_tcp(listen_sock);
	
}



void server_broadcast_packet(Packet *pack) {
	Client *tmp;

	for(tmp = client; tmp; tmp = tmp->next)
		protocol_send_packet(tmp->sock, pack);
}


Client *server_get_client_list() {
	return client;
}

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <darnit/darnit.h>
#include "../main.h"
#include "../bullet.h"
#include "../network/network.h"
#include "../network/protocol.h"

#define XSTR(s) STR(s)
#define STR(s) #s

#define HANDLE_KEY(A) do { \
		if(pack.keypress.keypress.A ) { \
			printf("server: player %i press %s\n", cli->id, STR(A)); \
			ss->player[cli->id]->keystate.A = 1; \
		} if(pack.keypress.keyrelease.A ) {\
			printf("server: player %i release %s\n", cli->id, STR(A)); \
			ss->player[cli->id]->keystate.A = 0; \
		} \
	} while(0)


int usleep(useconds_t usec);

typedef enum ServerState ServerState;
enum ServerState {
	SERVER_STATE_LOBBY,
	SERVER_STATE_STARTING,
	SERVER_STATE_GAME,
};

typedef struct ClientList ClientList;
struct ClientList {
	int id;
	int sock;
	char name[NAME_LEN_MAX];
	int team;
	
	ClientList *next;
};

static int listen_sock;
static ClientList *client = NULL;
static int clients = 0;
static volatile ServerState server_state;

DARNIT_SEMAPHORE *sem;

void server_handle_client(ClientList *cli) {
	Packet pack;
	Packet response;
	ClientList *tmp;
	int i;
	
	while(network_poll_tcp(cli->sock)) {
		protocol_recv_packet(cli->sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_JOIN:
				strcpy(cli->name, pack.join.name);
				cli->team = pack.join.team;
				printf("server: join %s team %i\n", cli->name, cli->team);
				
				response.type = PACKET_TYPE_JOIN;
				response.size = sizeof(PacketJoin);
				
				for(tmp = client; tmp; tmp = tmp->next) {
					memset(response.join.name, 0, NAME_LEN_MAX);
					strcpy(response.join.name, tmp->name);
					response.join.team = tmp->team;
					response.join.id = tmp->id;
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
					
					for(x = 0; x < pack.map_change.w; x++) {
						for(y = 0; y < pack.map_change.h; y++) {
							response.tile_update.x = x;
							response.tile_update.y = y;
							
							response.tile_update.tile = ss->active_level->layer[0].tilemap->data[y*pack.map_change.w + x];
							protocol_send_packet(cli->sock, &response);
						}
					}
				}
				
				break;
			
			case PACKET_TYPE_KEYPRESS:
				HANDLE_KEY(left);
				HANDLE_KEY(right);
				HANDLE_KEY(up);
				HANDLE_KEY(down);
				
				if(pack.keypress.keypress.shoot) {
					printf("server: shoot %i\n", bullet_spawn(BULLET_TYPE_WIMPY, ss->player[cli->id]));
					
				}
				
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
	ClientList *tmp;
	int i;
	
	for(;;) {
		switch(server_state) {
			case SERVER_STATE_LOBBY:
				if(network_poll_tcp(listen_sock)) {
					int sock;
					
					sock = network_accept_tcp(listen_sock);
					
					tmp = malloc(sizeof(ClientList));
					tmp->sock = sock;
					tmp->id = clients++;
					tmp->next = client;
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
				for(tmp = client; tmp; tmp = tmp->next) {
					pack.type = PACKET_TYPE_START;
					pack.size = sizeof(PacketStart);
					
					pack.start.player_id = tmp->id;
					
					protocol_send_packet(tmp->sock, (void *) &pack);
				}
				
				server_state = SERVER_STATE_GAME;
				break;
				
			case SERVER_STATE_GAME:
				d_util_semaphore_wait(sem);
				movableLoop();
				
				pack.type = PACKET_TYPE_MOVABLE_MOVE;
				pack.size = sizeof(PacketMovableMove);
				
				for(i = 0; i < ss->movable.movables; i++) {
					if(!ss->movable.movable[i].used)
						continue;
					
					int angle;
					pack.movable_move.x = ss->movable.movable[i].x/1000;
					pack.movable_move.y = ss->movable.movable[i].y/1000;
					pack.movable_move.dir = ss->movable.movable[i].direction;
					
					angle = ss->movable.movable[i].angle;
					if (angle < 0)
						angle += 3600;
					pack.movable_move.angle = (angle / 10 / 2);
					
					for(tmp = client; tmp; tmp = tmp->next) {
						if(tmp->id != me.id)
							protocol_send_packet(tmp->sock, &pack);
					}
				}

				for (i = 0; i < MAX_TEAM; i++) {
					struct UnitEntry *ue;
					
					d_util_mutex_lock(ss->team[i].unit.lock);
					for (ue = ss->team[i].unit.unit; ue; ue = ue->next) {
						if (ue->create_flag); // Just created
						if (ue->modify_flag); // Has been modified since last loop
						if (ue->delete_flag); // Will be deleted at the end of the loop
					}

					d_util_mutex_unlock(ss->team[i].unit.lock);
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
	ClientList *tmp;

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
	struct ClientList *next;

	for (next = client; next; next = next->next)
		if (next->id == id)
			return true;
	return false;
}

void server_shutdown() {
	ClientList *tmp;
	
	network_close_udp();
	
	for(tmp = client; tmp; tmp = tmp->next)
		network_close_tcp(tmp->sock);
	
	network_close_tcp(listen_sock);
	
}


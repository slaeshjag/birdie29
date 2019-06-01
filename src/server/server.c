#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <darnit/darnit.h>
#include "server.h"
#include "../main.h"
#include "bullet.h"
#include "../network/network.h"
#include "../network/protocol.h"
#include "serverplayer.h"
#include "../particles.h"

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

GameStateStruct *ss;

DARNIT_SEMAPHORE *sem;


static int _get_movable_index(int movable) {
	int tilew, tileh;

	tilew = ss->active_level->layer->tile_w;
	tileh = ss->active_level->layer->tile_h;

	return ss->movable.movable[movable].x/1000/tilew + ss->movable.movable[movable].y/1000/tileh * ss->active_level->layer->tilemap->w;
}

static void _client_handle_keys(Client *player) {
	MOVABLE_ENTRY *m = &ss->movable.movable[player->movable];
	
	if(player->keystate.up) {
		if (ss->team[player->team].power_map->map[_get_movable_index(player->movable)])
			m->y_velocity = -PLAYER_SPEED;
		else
			m->y_velocity = -PLAYER_SPEED_SLOW;
		//printf("server loop: player %i hold up\n", player->id);
		m->direction = PLAYER_DIRECTION_UP;
	} else if(player->keystate.down) {
		//printf("server loop: player %i hold down\n", player->id);
		if (ss->team[player->team].power_map->map[_get_movable_index(player->movable)])
			m->y_velocity = PLAYER_SPEED;
		else
			m->y_velocity = PLAYER_SPEED_SLOW;
		m->direction = PLAYER_DIRECTION_DOWN;
	} else {
		m->y_velocity = 0;
	}
	
	if(player->keystate.left) {
		//printf("server loop: player %i hold left\n", player->id);
		if (ss->team[player->team].power_map->map[_get_movable_index(player->movable)])
			m->x_velocity = -PLAYER_SPEED;
		else
			m->x_velocity = -PLAYER_SPEED_SLOW;
		m->direction = PLAYER_DIRECTION_LEFT;
	} else if(player->keystate.right) {
		//printf("server loop: player %i hold right\n", player->id);
		if (ss->team[player->team].power_map->map[_get_movable_index(player->movable)])
			m->x_velocity = PLAYER_SPEED;
		else
			m->x_velocity = PLAYER_SPEED_SLOW;
		m->direction = PLAYER_DIRECTION_RIGHT;
	} else {
		m->x_velocity = 0;
	}
}


static void _handle_timers() {
	int i, max_money = -1;

	if (ss->grace_counter > 0) {
			ss->grace_counter -= d_last_frame_time();
		if (ss->grace_counter < 0)
			ss->grace_counter = 0;
	} else {
		for (i = 0; i < MAX_TEAM; i++) {
			if (ss->team[i].money > max_money)
				max_money = ss->team[i].money;
		}

		for (i = 0; i < MAX_TEAM; i++) {
			if (ss->team[i].money == max_money)
				ss->team[i].time_to_win -= d_last_frame_time();
			if (ss->team[i].time_to_win < 0) {
				Packet pack;

				pack.type = PACKET_TYPE_EXIT;
				pack.size = sizeof(PacketExit);

				server_broadcast_packet(&pack);
				/* TODO: You're winner */
			}

		}
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
	pack.layer = MAP_LAYER_OVL_MISC;
	
	protocol_send_packet(player->sock, (Packet *) &pack);
	
	player->highlight.x = highlight_x;
	player->highlight.y = highlight_y;
	
	pack.x = player->highlight.x;
	pack.y = player->highlight.y;
	pack.tile = 1;
	pack.layer = MAP_LAYER_OVL_MISC;
	
	protocol_send_packet(player->sock, (Packet *) &pack);
}

void _client_status_update(Client *player) {
	Packet pack;
	int i;
	
	pack.type = PACKET_TYPE_STATUS_UPDATE;
	pack.size = sizeof(PacketStatusUpdate);
	
	for(i = 0; i < TEAMS_CAP; i++) {
		pack.status.money[i] = ss->team[i].money;
		pack.status.time_to_win[i] = ss->team[i].time_to_win;
	}
	
	pack.status.grace_timer = ss->grace_counter;
	
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
		_client_status_update(cli);
	}
	
	while(network_poll_tcp(cli->sock)) {
		protocol_recv_packet(cli->sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_JOIN:
				strcpy(cli->name, pack.join.name);
				cli->team = pack.join.team;
				cli->sprite_variant = pack.join.sprite_variant;
				cli->hp = PLAYER_HP;
				printf("server: join %s team %i\n", cli->name, cli->team);
				
				response.type = PACKET_TYPE_JOIN;
				response.size = sizeof(PacketJoin);
				if (cli->movable > -1)
					movableDespawn(cli->movable);
				cli->movable = -1;
				
				for(tmp = client; tmp; tmp = tmp->next) {
					memset(response.join.name, 0, NAME_LEN_MAX);
					strcpy(response.join.name, tmp->name);
					response.join.team = tmp->team;
					response.join.id = tmp->id;
					response.join.movable = cli->movable;
					protocol_send_packet(cli->sock, &response);
					
					if(tmp->sock != cli->sock) {
						response.join.id = cli->id;
						strcpy(response.join.name, cli->name);
						response.join.team = cli->team;
						response.join.movable = cli->movable;
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
					
					int map_w = pack.map_change.w = ss->active_level->layer[0].tilemap->w;
					int map_h = pack.map_change.h = ss->active_level->layer[0].tilemap->h;
					
					/* Process map collision layer */
					for(int x = 0; x < map_w; x++) {
						for(int y = 0; y < map_h; y++) {
							for(i = 0; i < MAP_LAYERS; i++) {
								uint32_t tile;
								/* Specific part of the tilesheet is collision tiles */
								tile = ss->active_level->layer[i].tilemap->data[y*map_w + x];
								if(tile >= TILESHEET_COLLISION_TILE_LOW && tile <= TILESHEET_COLLISION_TILE_HIGH) {
									//tile |= TILESET_COLLISION_MASK;
									ss->active_level->layer[0].tilemap->data[y*map_w + x] |= TILESET_COLLISION_MASK;
								}
							}
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
					bullet_spawn(BULLET_TYPE_WIMPY, cli);
				}
				
				if(pack.keypress.keypress.shoot_special) {
					bullet_spawn(BULLET_TYPE_BADASS, cli);
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
			
			case PACKET_TYPE_BUILD_UNIT:
				printf("server: building %i at (%i, %i)\n", pack.build.unit, cli->highlight.x, cli->highlight.y);
				
				unit_add(cli->team, pack.build.unit, cli->highlight.x, cli->highlight.y, false);
				
				break;
			default:
				fprintf(stderr, "wat %i\n", pack.type);
				break;
		}
	}
}

int server_thread(void *arg) {
	Packet pack = {};
	Client *tmp;
	int i;

	for(;;) {
		switch(server_state) {
			case SERVER_STATE_LOBBY:
				if(network_poll_tcp(listen_sock)) {
					int sock;
					
					sock = network_accept_tcp(listen_sock);
					
					tmp = calloc(1, sizeof(Client));
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
				for (i = 0; i < MAX_TEAM; i++) {
					ss->team[i].power_map = pylonpower_map_new(ss->active_level->layer->tilemap->w, ss->active_level->layer->tilemap->h);
					ss->team[i].time_to_win = TIMER_WIN;
				}

				ss->grace_counter = TIMER_GRACE;
				
				/* Send map change packets to clients */
				pack.type = PACKET_TYPE_MAP_CHANGE;
				pack.size = sizeof(PacketMapChange);
				
				memset(pack.map_change.name, 0, MAP_NAME_LEN_MAX);
				int map_h, map_w;
				
				map_w = pack.map_change.w = ss->active_level->layer[0].tilemap->w;
				map_h = pack.map_change.h = ss->active_level->layer[0].tilemap->h;
					
				for(tmp = client; tmp; tmp = tmp->next) {
					protocol_send_packet(tmp->sock, &pack);
				}
				
				/* Update client maps */
				pack.type = PACKET_TYPE_TILE_UPDATE;
				pack.size = sizeof(PacketTileUpdate);
				
				for(tmp = client; tmp; tmp = tmp->next) {
					uint32_t x, y;
					
					for(x = 0; x < map_w; x++) {
						for(y = 0; y < map_h; y++) {
							for(i = 0; i < MAP_LAYERS; i++) {
								pack.tile_update.x = x;
								pack.tile_update.y = y;
								pack.tile_update.layer = i;
								pack.tile_update.tile = ss->active_level->layer[i].tilemap->data[map_w*y + x] & TILESET_MASK;
								protocol_send_packet(tmp->sock, &pack);
							}
						}
					}
				}
				
				unit_prepare();
				
				for(tmp = client; tmp; tmp = tmp->next) {
					/* teleport players to their spawning point */
					tmp->movable = movableSpawnSprite(ss->team[tmp->team].spawn.x, ss->team[tmp->team].spawn.y, 0, tmp->team*PLAYER_VARIANTS + tmp->sprite_variant);
					ss->movable.movable[client->movable].x = ss->team[client->team].spawn.x * 1000;
					ss->movable.movable[client->movable].y = ss->team[client->team].spawn.y * 1000;
				}
				
				for(tmp = client; tmp; tmp = tmp->next) {
					/* Send join packets to update movable ID */
					Client *joinclient;
					
					pack.type = PACKET_TYPE_JOIN;
					pack.size = sizeof(PacketJoin);
					
					for(joinclient = client; joinclient; joinclient = joinclient->next) {
						memset(pack.join.name, 0, NAME_LEN_MAX);
						strcpy(pack.join.name, joinclient->name);
						pack.join.team = joinclient->team;
						pack.join.id = joinclient->id;
						pack.join.movable = joinclient->movable;
						protocol_send_packet(tmp->sock, &pack);
					}
				}
				
				pack.type = PACKET_TYPE_START;
				pack.size = sizeof(PacketStart);
				
				for(tmp = client; tmp; tmp = tmp->next) {	
					pack.start.player_id = tmp->id;
					pack.start.movable = tmp->movable;
					
					protocol_send_packet(tmp->sock, (void *) &pack);
				}

				for (i = 0; i < MAX_TEAM; i++) {
					for (tmp = client; tmp; tmp = tmp->next) {
						if (tmp->team == i)
							goto good;
					}

					ss->team[i].money = 0;

					good:;
				}
				
				server_state = SERVER_STATE_GAME;
				break;
				
			case SERVER_STATE_GAME:
				d_util_semaphore_wait(sem);
				
				_handle_timers();
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


void server_particle_spawn(int x, int y, int angle, enum ParticleType type) {
	static Packet pack;

	pack.type = PACKET_TYPE_PARTICLE_EVENT;
	pack.size = sizeof(PacketParticle);
	pack.particle.x = x;
	pack.particle.y = y;
	pack.particle.angle = angle;
	pack.particle.effect_type = type;

	printf("Attempting to spawn particle %i\n", pack.type);
	server_broadcast_packet(&pack);
}

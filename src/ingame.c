#include "main.h"
#include "ingame.h"
#include "camera.h"
#include "network/protocol.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "network/network.h"
#include "network/protocol.h"
#include "server/server.h"
#include "main.h"
//#include "gameover.h"
#include "util.h"
//#include "effect.h"
//#include "sfx.h"

void ingame_init() {
	int i;
	const char *playerid_str;
	/* Leak *all* the memory */
	s->active_level = d_map_load(util_binrel_path("map/map.ldmz"));
	s->camera.follow = -1;
	s->camera.x = s->camera.y = 0;

//	bulletInit();
//	movableLoad();
//	healthbar_init();
//	soundeffects_init();

	/*
	char *prop;
	int center_x, center_y, radius;
	prop = d_map_prop(s->active_level->prop, "center_x");
	center_x = atoi(prop);
	prop = d_map_prop(s->active_level->prop, "center_y");
	center_y = atoi(prop);
	prop = d_map_prop(s->active_level->prop, "radius");
	radius = atoi(prop);
	* */
}



void ingame_loop() {
	int i, team1t, team2t;
	
	d_render_clearcolor_set(0x88, 0xf2, 0xff);
	
	d_render_tint(255, 255, 255, 255);
	
	movableLoop();
	
	if(s->is_host) {
		server_kick();
		//s->time_left -= d_last_frame_time();
		//ingame_timer_package_send(s->time_left / 1000);
		
		/*
		if (s->time_left <= 0) {
			Packet pack;

			pack.type = PACKET_TYPE_EXIT;
			pack.size = sizeof(pack.exit);
			protocol_send_packet(server_sock, (void *) &pack);
		}
		* */
		//bullet_loop();
	//	turret_loop();
		
	}
	
	camera_work();
	d_map_camera_move(s->active_level, s->camera.x, s->camera.y);

	d_render_offset(s->camera.x, s->camera.y);
	
	for (i = 0; i < s->active_level->layers; i++) {
		d_render_offset(0, 0);
		d_render_tint(255, 255, 255, 255);
//		d_render_tile_blit(s->active_level->layer[i].ts, 0, 0, 1);
		d_tilemap_draw(s->active_level->layer[i].tilemap);
		d_render_offset(s->camera.x, s->camera.y);
		movableLoopRender(i);
		
	}
	
	/*for(i = 0; i < PARTICLE_EFFECTS; i++)
		d_particle_draw(s->particle_effect[i]);*/

	/* HUD */
	d_render_offset(0, 0);
	
	//ingame_timer_blit(s->time_left2 / 1000, 1, 0);
	//ingame_applegague_blit(s->player_id);
//	healthbar_draw();

	
	ingame_client_keyboard();
}


void ingame_client_keyboard() {
	static struct InGameKeyStateEntry oldstate = {};
	struct InGameKeyStateEntry newstate, pressevent, releaseevent;

	memset(&pressevent, 0, sizeof(pressevent));
	memset(&releaseevent, 0, sizeof(pressevent));
	
	newstate.left = d_keys_get().left;
	newstate.right = d_keys_get().right;
	newstate.up = d_keys_get().up;
	newstate.down = d_keys_get().down;
	newstate.suicide = d_keys_get().x;
	
	if (d_keys_get().lmb) {
		DARNIT_KEYS keys;
		keys = d_keys_zero();
		keys.lmb = 1;
		d_keys_set(keys);
		//ingame_apple_bullet_fire();
	}

	
	if(d_keys_get().select)
		restart_to_menu(me.name);
	
	
	if (newstate.left ^ oldstate.left) {
		if (newstate.left)
			pressevent.left = true, releaseevent.left = false;
		else
			releaseevent.left = true, pressevent.left = false;
	}

	if (newstate.right ^ oldstate.right) {
		if (newstate.right)
			pressevent.right = true, releaseevent.right = false;
		else
			releaseevent.right = true, pressevent.right = false;
	}
	

	//if(newstate.left || newstate.right)
	//	sfx_play(SFX_WALK);

	PacketKeypress kp;

	kp.size = sizeof(kp);
	kp.type = PACKET_TYPE_KEYPRESS;
	kp.keypress = pressevent;
	kp.keyrelease = releaseevent;

	protocol_send_packet(s->server_sock, (void *) &kp);

	oldstate = newstate;

	if (d_keys_get().rmb) {
		DARNIT_KEYS keys;
		Packet pack;

		keys = d_keys_zero();
		keys.rmb = 1;
		d_keys_set(keys);
		//pack.type = PACKET_TYPE_CHANGE_APPLE;
		//pack.size = sizeof(pack.change_apple);
		 
		//protocol_send_packet(server_sock, (void *) &pack);
	}
}

void ingame_network_handler() {
	Packet pack;
	void *p;
	int i;
	
	while(network_poll_tcp(s->server_sock)) {
		
		protocol_recv_packet(s->server_sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_MOVE_OBJECT:
				p = pack.raw;
				
				for(i = 0; i < s->movable.movables; i++) {
					s->movable.movable[i].x = ((int) (*((uint16_t *) p))) * 1000;
					p+= 2;
					s->movable.movable[i].y = ((int) (*((uint16_t *) p))) * 1000;
					p+= 2;
					s->movable.movable[i].direction = *((uint8_t *) p);
					p+= 1;
					s->movable.movable[i].angle = *((uint8_t *) p);
					s->movable.movable[i].angle *= (2 * 10);
					p += 1;
					//s->movable.movable[i].hp = *((uint16_t *) p);
					p += 2;
					//s->movable.movable[i].hp_max = *((uint16_t *) p);
					p += 2;
				}
				break;
			
			case PACKET_TYPE_BULLET_ANNOUNCE:
//				if (s->is_host)
					//bullet_add(pack.bullet_announce.bullet_type, pack.bullet_announce.id, pack.bullet_announce.x, pack.bullet_announce.y);
				break;
			case PACKET_TYPE_BULLET_UPDATE:

//				if (s->is_host)
					//bullet_update(pack.bullet_update.x, pack.bullet_update.y, pack.bullet_update.id);
				break;
			case PACKET_TYPE_BULLET_REMOVE:
//				if (s->is_host)
					//bullet_destroy(pack.bullet_remove.id);
				break;
			case PACKET_TYPE_SOUND:
	//			soundeffects_play(pack.sound.sound);
				break;
			case PACKET_TYPE_TIMER:
				//s->time_left2 = pack.timer.time_left * 1000;
				break;
			case PACKET_TYPE_EXIT:
				//game_over_set_team(pack.exit.team);
				game_state(GAME_STATE_GAME_OVER);
				break;

			case PACKET_TYPE_PARTICLE:
				//d_particle_emitter_move(s->particle_effect[pack.particle.effect_type], pack.particle.x, pack.particle.y);
				//d_particle_pulse(s->particle_effect[pack.particle.effect_type]);
				//sfx_play(SFX_APPLE);
				break;
		}
	}
}


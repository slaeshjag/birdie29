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
#include "player.h"
//#include "gameover.h"
#include "util.h"
//#include "effect.h"
//#include "sfx.h"

#define KEYEVENT(key) do { \
	if (newstate.key ^ oldstate.key) { \
		if (newstate.key) \
			pressevent.key = true, releaseevent.key = false; \
		else \
			releaseevent.key = true, pressevent.key = false; \
	} \
} while(0)

void ingame_init() {
	int i;
	//const char *playerid_str;
	/* Leak *all* the memory */
	
	
	unit_init(); // XXX: Don't even dare running init before the map is loaded
	cs->camera.follow = me.movable;
	cs->camera.x = cs->camera.y = 0;

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
	int i;
	
	d_render_clearcolor_set(0x88, 0xf2, 0xff);
	
	d_render_tint(255, 255, 255, 255);
		
	
	if(ss->is_host) {
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
	for(i = 0; i < MAP_LAYERS; i++) {
		d_tilemap_camera_move(cs->map.layer[i], cs->camera.x, cs->camera.y);
	}

	d_render_offset(cs->camera.x, cs->camera.y);
	
	for (i = 0; i < MAP_LAYERS; i++) {
		d_render_offset(0, 0);
		d_render_tint(255, 255, 255, 255);
//		d_render_tile_blit(s->active_level->layer[i].ts, 0, 0, 1);
		d_tilemap_draw(cs->map.layer[i]);
		d_render_offset(cs->camera.x, cs->camera.y);
		drawable_render(cs->drawable, i);
		
	}
	
	/*for(i = 0; i < PARTICLE_EFFECTS; i++)
		d_particle_draw(s->particle_effect[i]);*/

	/* HUD */
	d_render_offset(0, 0);
	
	//ingame_timer_blit(s->time_left2 / 1000, 1, 0);
	//ingame_applegague_blit(s->player_id);
//	healthbar_draw();

	
	ingame_client_keyboard();
	
	if(ss->is_host) {
		for(i = 0; i < PLAYER_CAP; i++) {
			if(ss->player[i])
				player_handle_keys(ss->player[i]);
		}
	}
}


void ingame_client_keyboard() {
	static struct InGameKeyStateEntry oldstate = {};
	struct InGameKeyStateEntry newstate, pressevent = {}, releaseevent = {};

	memset(&pressevent, 0, sizeof(pressevent));
	memset(&releaseevent, 0, sizeof(pressevent));
	
	newstate.left = d_keys_get().left;
	newstate.right = d_keys_get().right;
	newstate.up = d_keys_get().up;
	newstate.down = d_keys_get().down;
	newstate.suicide = d_keys_get().x;
	newstate.shoot = d_keys_get().a;
	
	if (d_keys_get().lmb) {
		DARNIT_KEYS keys;
		keys = d_keys_zero();
		keys.lmb = 1;
		d_keys_set(keys);
		//ingame_apple_bullet_fire();
	}

	
	if(d_keys_get().select)
		restart_to_menu(me.name);
	
	KEYEVENT(left);
	KEYEVENT(right);
	KEYEVENT(up);
	KEYEVENT(down);
	KEYEVENT(shoot);
	

	//if(newstate.left || newstate.right)
	//	sfx_play(SFX_WALK);

	PacketKeypress kp;

	kp.size = sizeof(kp);
	kp.type = PACKET_TYPE_KEYPRESS;
	kp.keypress = pressevent;
	kp.keyrelease = releaseevent;

	protocol_send_packet(cs->server_sock, (void *) &kp);

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
	
	while(network_poll_tcp(cs->server_sock)) {
		
		protocol_recv_packet(cs->server_sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_MOVABLE_SPAWN:
				break;
			case PACKET_TYPE_MOVABLE_MOVE:
				drawable_move(cs->drawable, pack.movable_move.movable, pack.movable_move.x, pack.movable_move.y, 20*pack.movable_move.angle, pack.movable_move.dir);
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


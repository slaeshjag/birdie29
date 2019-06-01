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
#include "hud.h"
#include "particles.h"
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
	
	hud_update();
	particle_loop();
	
	camera_work();
	for(i = 0; i < MAP_LAYERS; i++) {
		d_tilemap_camera_move(cs->map.layer[i], cs->camera.x, cs->camera.y);
	}

	d_render_offset(cs->camera.x, cs->camera.y);

	for (i = 0; i < MAP_LAYERS; i++) {
		d_render_offset(0, 0);
		d_render_tint(255, 255, 255, 255);
//		d_render_tile_blit(s->active_level->layer[i].ts, 0, 0, 1);
		d_tilemap_recalc(cs->map.layer[i]);
		d_tilemap_draw(cs->map.layer[i]);
		d_render_offset(cs->camera.x, cs->camera.y);
		drawable_render(cs->drawable, i);
		
	}
	
	/*for(i = 0; i < PARTICLE_EFFECTS; i++)
		d_particle_draw(s->particle_effect[i]);*/
	
	particle_render();

	/* HUD */
	d_render_offset(0, 0);
	
	//ingame_timer_blit(s->time_left2 / 1000, 1, 0);
	//ingame_applegague_blit(s->player_id);
//	healthbar_draw();

	
	ingame_client_keyboard();
	
	hud_render();
}


void ingame_client_keyboard() {
	DARNIT_MOUSE mouse;
	int mouse_angle;
	int tile_size;
	int angle;
	
	static struct InGameKeyStateEntry oldstate = {};
	struct InGameKeyStateEntry newstate, pressevent = {}, releaseevent = {};

	memset(&pressevent, 0, sizeof(pressevent));
	memset(&releaseevent, 0, sizeof(pressevent));
	
	newstate.left = d_keys_get().left;
	newstate.right = d_keys_get().right;
	newstate.up = d_keys_get().up;
	newstate.down = d_keys_get().down;
	newstate.suicide = d_keys_get().x;
	newstate.shoot = d_keys_get().lmb;
	newstate.build = d_keys_get().rmb;
	
	
	if(d_keys_get().select)
		restart_to_menu(me.name);
	
	KEYEVENT(left);
	KEYEVENT(right);
	KEYEVENT(up);
	KEYEVENT(down);
	KEYEVENT(shoot);
	KEYEVENT(build);
	
	if(pressevent.build) {
		PacketBuildUnit buildunit;
		
		if(cs->player[me.id]->selected_building >= 0) {		
			buildunit.type = PACKET_TYPE_BUILD_UNIT,
			buildunit.size = sizeof(PacketBuildUnit);
			
			buildunit.unit = cs->player[me.id]->selected_building;
			
			protocol_send_packet(cs->server_sock, (void *) &buildunit);
		}
	}
	
	mouse = d_mouse_get();
	
	int x, y;
	
	y = (cs->drawable->entry[cs->player[me.id]->movable].y) - (cs->camera.y + mouse.y);
	x = (cs->drawable->entry[cs->player[me.id]->movable].x) - (cs->camera.x + mouse.x);
	
	angle = atan2(y, x)*180/M_PI + 180;
	
	if(mouse.wheel != 0) {
		if(mouse.wheel > 0)
			cs->player[me.id]->selected_building += 1;
		else
			cs->player[me.id]->selected_building -= 1;
		
		if(cs->player[me.id]->selected_building >= UNIT_TYPES - 1)
			cs->player[me.id]->selected_building = -1;
		
		if(cs->player[me.id]->selected_building < -1)
			cs->player[me.id]->selected_building = UNIT_TYPES - 2;
	}
	
	//if(newstate.left || newstate.right)
	//	sfx_play(SFX_WALK);

	PacketKeypress kp;

	kp.size = sizeof(kp);
	kp.type = PACKET_TYPE_KEYPRESS;
	kp.keypress = pressevent;
	kp.keyrelease = releaseevent;
	
	kp.mouse_angle = angle;
	
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
	int i;
	
	while(network_poll_tcp(cs->server_sock)) {
		
		protocol_recv_packet(cs->server_sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_MOVABLE_SPAWN:
				drawable_spawn(cs->drawable, pack.movable_spawn.sprite_type, pack.movable_spawn.movable, pack.movable_spawn.x, pack.movable_spawn.y, pack.movable_spawn.l, pack.movable_spawn.angle);
				break;
			case PACKET_TYPE_MOVABLE_MOVE:
				drawable_move(cs->drawable, pack.movable_move.movable, pack.movable_move.x, pack.movable_move.y, 20*pack.movable_move.angle, pack.movable_move.dir);
				break;
			case PACKET_TYPE_MOVABLE_DESPAWN:
				drawable_despawn(cs->drawable, pack.movable_despawn.movable);
				break;

			case PACKET_TYPE_TILE_UPDATE:
				cs->map.layer[pack.tile_update.layer]->data[pack.tile_update.y * cs->map.layer[pack.tile_update.layer]->w + pack.tile_update.x] = pack.tile_update.tile;
				break;
			
			case PACKET_TYPE_STATUS_UPDATE:
				for(i = 0; i < TEAMS_CAP; i++) {
					cs->team[i].money = pack.status.money[i];
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
			case PACKET_TYPE_EXIT:
				//game_over_set_team(pack.exit.team);
				game_state(GAME_STATE_GAME_OVER);
				break;

			case PACKET_TYPE_PARTICLE:
				//d_particle_emitter_move(s->particle_effect[pack.particle.effect_type], pack.particle.x, pack.particle.y);
				//d_particle_pulse(s->particle_effect[pack.particle.effect_type]);
				//sfx_play(SFX_APPLE);
				break;
			case PACKET_TYPE_POWER_EVENT:
				if (pack.power_event.team == me.team) {

					pylonpower_diff(cs->power_map, pack.power_event.x, pack.power_event.y, pack.power_event.sign);
					for (i = 0; i < cs->power_map->w * cs->power_map->h; i++)
						cs->map.layer[MAP_LAYER_OVL_POWER] = cs->power_map->map[i] ? 3 : 0;
					printf("power event at %i, %i: %i\n", pack.power_event.x, pack.power_event.y, pack.power_event.sign);
				}
				break;
		}
	}
}


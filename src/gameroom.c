#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <darnit/darnit.h>
#include <muil/muil.h>
#include "network/network.h"
#include "network/protocol.h"
//#include "server/server.h"
#include "gameroom.h"
#include "main.h"

GameRoom gameroom;

static void button_callback(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	if(widget == gameroom.button.back) {
		restart_to_menu(me.name);
	} else if(widget == gameroom.button.start) {
		server_start_game();
	}
}

static void listbox_team_callback(MuilWidget *widget, unsigned int type, MuilEvent *e)  {
	MuilPropertyValue p;
	
	p = widget->get_prop(widget, MUIL_LISTBOX_PROP_SELECTED);
	
	PacketJoin join;

	join.type = PACKET_TYPE_JOIN;
	join.size = sizeof(PacketJoin);
	join.id = 0;
	memcpy(join.name, me.name, NAME_LEN_MAX);
	join.name[NAME_LEN_MAX - 1] = 0;
	join.team = p.i;

	protocol_send_packet(cs->server_sock, (void *) &join);
}

void gameroom_init() {
	int i;
	gameroom.pane.pane = muil_pane_create(10, 10, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 20, gameroom.vbox = muil_widget_create_vbox());
	gameroom.pane.next = NULL;

	gameroom.pane.pane->background_color.r = PANE_R;
	gameroom.pane.pane->background_color.g = PANE_G;
	gameroom.pane.pane->background_color.b = PANE_B;

	muil_vbox_add_child(gameroom.vbox, gameroom.label = muil_widget_create_label(gfx.font.large, "Players in game"), 0);
	muil_vbox_add_child(gameroom.vbox, gameroom.list = muil_widget_create_listbox(gfx.font.small), 1);
	
	gameroom.hbox_button = muil_widget_create_hbox();
	gameroom.hbox_team = muil_widget_create_hbox();
	
	muil_vbox_add_child(gameroom.hbox_team, gameroom.team.label = muil_widget_create_label(gfx.font.small, "Select team"), 0);
	muil_vbox_add_child(gameroom.hbox_team, gameroom.team.list = muil_widget_create_listbox(gfx.font.small), 0);
	
	for(i = 0; i < TEAMS_CAP; i++)
		muil_listbox_add(gameroom.team.list, team_name[i]);
	
	MuilPropertyValue p = {.i = 0};
	gameroom.team.list->set_prop(gameroom.team.list, MUIL_LISTBOX_PROP_SELECTED, p);
	
	muil_vbox_add_child(gameroom.hbox_button, gameroom.button.back = muil_widget_create_button_text(gfx.font.small, "Back"), 0);
	muil_vbox_add_child(gameroom.hbox_button, gameroom.button.start = muil_widget_create_button_text(gfx.font.small, "Start game"), 0);
	
	muil_vbox_add_child(gameroom.vbox, gameroom.hbox_team, 1);
	muil_vbox_add_child(gameroom.vbox, gameroom.hbox_button, 0);
	
	gameroom.button.start->enabled = false;
	
	gameroom.button.back->event_handler->add(gameroom.button.back, button_callback, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	gameroom.button.start->event_handler->add(gameroom.button.start, button_callback, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	
	gameroom.team.list->event_handler->add(gameroom.team.list, listbox_team_callback, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
}


void gameroom_network_handler() {
	Packet pack;
	void *tmp;
	int i;
	
	
	if(!network_poll_tcp(cs->server_sock))
		return;
	protocol_recv_packet(cs->server_sock, &pack);
	
	switch(pack.type) {
		case PACKET_TYPE_JOIN:
			printf("client: player %s team %i\n", pack.join.name, pack.join.team);
			if(cs->player[pack.join.id]) {
				/* Changed team */
				int i;
				MuilPropertyValue v;
				v = gameroom.list->get_prop(gameroom.list, MUIL_LISTBOX_PROP_SIZE);
				
				player_set(pack.join.id, pack.join.name, pack.join.team);
				
				for(i = 0; i < v.i; i++) {
					if(atoi(muil_listbox_get(gameroom.list, i)) == pack.join.id) {
						asprintf(&tmp, "%i: %s [Team %s]", pack.join.id, pack.join.name, team_name[pack.join.team]);
						muil_listbox_set(gameroom.list, i, tmp);
						free(tmp);
						break;
					}
				}
				
			} else {
				/* New player joined */
				asprintf(&tmp, "%i: %s [Team %s]", pack.join.id, pack.join.name, team_name[pack.join.team]);
				muil_listbox_add(gameroom.list, tmp);
				free(tmp);
				
				player_join(pack.join.id, pack.join.name, pack.join.team);
			}
			break;
		
		case PACKET_TYPE_START:
			me.id = pack.start.player_id;
			printf("Started game as player %i\n", me.id);
			game_state(GAME_STATE_GAME);
			break;
	}
}

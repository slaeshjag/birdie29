#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <muil/muil.h>
#include "hud.h"
#include "config.h"
#include "main.h"
#include "server/unit.h"

Hud hud;

void hud_init() {
	int i;
	MuilPropertyValue v;
	
	hud.pane.pane = muil_pane_create(DISPLAY_WIDTH/2 - 150, DISPLAY_HEIGHT - 96, 300, 64, hud.hbox = muil_widget_create_hbox());
	hud.pane.next = &hud.scoreboard.pane;
	
	hud.pane.pane->background_color.r = PANE_R;
	hud.pane.pane->background_color.g = PANE_G;
	hud.pane.pane->background_color.b = PANE_B;
	hud.pane.pane->background_color.a = 128;
	
	for(i = 0; i < UNIT_TYPES - 1; i++) {
		char fname[256];
		sprintf(fname, "res/hud%i.png", i);
		muil_hbox_add_child(hud.hbox, hud.picture[i] = muil_widget_create_imageview_file(fname, 48, 48, DARNIT_PFORMAT_RGBA8), 0);
	}
	
	hud.selected_frame = d_render_tilesheet_load("res/selected.png", 48, 48, DARNIT_PFORMAT_RGBA8);
	
	hud.scoreboard.pane.pane = muil_pane_create(4, 4, 128, 128, hud.scoreboard.vbox = muil_widget_create_vbox());
	hud.scoreboard.pane.next = NULL;
	
	hud.scoreboard.pane.pane->background_color.r = PANE_R;
	hud.scoreboard.pane.pane->background_color.g = PANE_G;
	hud.scoreboard.pane.pane->background_color.b = PANE_B;
	
	for(i = 0; i < TEAMS_CAP; i++) {
		muil_vbox_add_child(hud.scoreboard.vbox, hud.scoreboard.label[i] = muil_widget_create_label(gfx.font.small, strdup("lol")), 0);
	}
}

void hud_update() {
	int i;
	MuilPropertyValue v;
	
	//cs->player[me.id];
	for(i = 0; i < TEAMS_CAP; i++) {
		char buf[256];
		sprintf(buf, "%s: $%i", team_name[i], cs->team[i].money);
		
		v = hud.scoreboard.label[i]->get_prop(hud.scoreboard.label[i], MUIL_LABEL_PROP_TEXT);
		free(v.p);
		
		v.p = strdup(buf);
		hud.scoreboard.label[i]->set_prop(hud.scoreboard.label[i], MUIL_LABEL_PROP_TEXT, v);
	}
}

void hud_render() {
	if(cs->player[me.id]->selected_building >= 0) {
		int x, y;
		
		x = hud.picture[cs->player[me.id]->selected_building]->x;
		y = hud.picture[cs->player[me.id]->selected_building]->y;
		
		d_render_tile_blit(hud.selected_frame, 0, x, y);
	}
}

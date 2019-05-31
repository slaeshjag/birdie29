#include <stdint.h>
#include <stdbool.h>
#include <muil/muil.h>
#include "hud.h"
#include "config.h"
#include "main.h"

Hud hud;

void hud_init() {
	int i;
	MuilPropertyValue v;
	
	hud.pane.pane = muil_pane_create(DISPLAY_WIDTH/2 - 150, DISPLAY_HEIGHT - 96, 300, 64, hud.hbox = muil_widget_create_hbox());
	hud.pane.next = NULL;
	
	hud.pane.pane->background_color.r = PANE_R;
	hud.pane.pane->background_color.g = PANE_G;
	hud.pane.pane->background_color.b = PANE_B;
	hud.pane.pane->background_color.a = 128;
	
	for(i = 0; i < 5; i++) {
		char fname[256];
		sprintf(fname, "res/hud%i.png", i);
		muil_hbox_add_child(hud.hbox, hud.picture[i] = muil_widget_create_imageview_file(fname, 48, 48, DARNIT_PFORMAT_RGBA8), 0);
	}
	
	hud.selected_frame = d_render_tilesheet_load("res/selected.png", 48, 48, DARNIT_PFORMAT_RGBA8);
}

void hud_update() {
	//cs->player[me.id];
}

void hud_render() {
	if(cs->player[me.id]->selected_building >= 0) {
		int x, y;
		
		x = hud.picture[cs->player[me.id]->selected_building]->x;
		y = hud.picture[cs->player[me.id]->selected_building]->y;
		
		d_render_tile_blit(hud.selected_frame, 0, x, y);
	}
}

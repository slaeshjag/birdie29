#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <muil/muil.h>
#include "hud.h"
#include "config.h"
#include "main.h"
#include "server/unit.h"
#include "healthbar.h"


Hud hud;


static void _timer_blit(int time_left, int mode, int pos) {
	int minute, deka, second;

	minute = time_left / 60;
	deka = (time_left % 60) / 10;
	second = time_left % 10;
	d_render_tile_blit(hud._7seg, minute + 11*mode, pos, 0);
	d_render_tile_blit(hud._7seg, 10 + 11*mode, pos+24, 0);
	d_render_tile_blit(hud._7seg, deka + 11*mode, pos+48, 0);
	d_render_tile_blit(hud._7seg, second + 11*mode, pos+72, 0);
}



void hud_init() {
	int i;
	MuilPropertyValue v;
	
	hud.pane.pane = muil_pane_create(DISPLAY_WIDTH/2 - 150, DISPLAY_HEIGHT - 96, 300, 64, hud.hbox = muil_widget_create_hbox());
	////hud.pane.next = &hud.scoreboard.pane;
	
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
	
	//hud.scoreboard.pane.pane = muil_pane_create(4, 4, 128, 128, hud.scoreboard.vbox = muil_widget_create_vbox());
	//hud.scoreboard.pane.next = NULL;
	
	//hud.scoreboard.pane.pane->background_color.r = PANE_R;
	//hud.scoreboard.pane.pane->background_color.g = PANE_G;
	//hud.scoreboard.pane.pane->background_color.b = PANE_B;
	
	//muil_vbox_add_child(hud.scoreboard.vbox, hud.scoreboard.title = muil_widget_create_label(gfx.font.small, "Fuel Stocks"), 0);
	
	//for(i = 0; i < TEAMS_CAP; i++) {
	//	muil_vbox_add_child(hud.scoreboard.vbox, hud.scoreboard.label[i] = muil_widget_create_label(gfx.font.small, strdup("lol")), 0);
	//}

	hud._7seg = d_render_tilesheet_load("res/7seg.png", 24, 32, DARNIT_PFORMAT_RGB5A1);
	healthbar_init();
}

void hud_update() {
	int i;
	MuilPropertyValue v;
	/*
	//cs->player[me.id];
	for(i = 0; i < TEAMS_CAP; i++) {
		char buf[256];
		sprintf(buf, "%s: %i", team_name[i], cs->team[i].money);
		
		v = hud.scoreboard.label[i]->get_prop(hud.scoreboard.label[i], MUIL_LABEL_PROP_TEXT);
		free(v.p);
		
		v.p = strdup(buf);
		hud.scoreboard.label[i]->set_prop(hud.scoreboard.label[i], MUIL_LABEL_PROP_TEXT, v);
	}*/
}

void hud_render() {
	int i, max_money;


	if(cs->player[me.id]->selected_building >= 0) {
		int x, y;
		
		x = hud.picture[cs->player[me.id]->selected_building]->x;
		y = hud.picture[cs->player[me.id]->selected_building]->y;
		
		d_render_tile_blit(hud.selected_frame, 0, x, y);
	}

	d_render_offset(0, 0);
	max_money = -1;
	for (i = 0; i < MAX_TEAM; i++)
		if (max_money < cs->team[i].money)
			max_money = cs->team[i].money;
	if (max_money == 0)
		max_money = 1;
	if (cs->grace_timer > 0) {
		_timer_blit(cs->grace_timer/1000, 9, 592);
	} else {
		_timer_blit(cs->team[0].time_to_win/1000, cs->team[0].money == max_money ? 1 : 0, 0);
		_timer_blit(cs->team[1].time_to_win/1000, cs->team[1].money == max_money ? 3 : 2, 320);
		_timer_blit(cs->team[2].time_to_win/1000, cs->team[2].money == max_money ? 5 : 4, 640);
		_timer_blit(cs->team[3].time_to_win/1000, cs->team[3].money == max_money ? 7 : 6, 960);
	}
	
	healthbar_set(0, 100.f * cs->team[0].money / max_money);
	healthbar_set(1, 100.f * cs->team[1].money / max_money);
	healthbar_set(2, 100.f * cs->team[2].money / max_money);
	healthbar_set(3, 100.f * cs->team[3].money / max_money);
	healthbar_draw();
}

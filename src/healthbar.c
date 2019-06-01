#include "healthbar.h"


static struct HealthbarStruct healthbar;


static void _calculate_one(int hp, DARNIT_TILE *tile, int x_pos, int id) {
	float arne;
	int w;

	arne = ((float) hp) / 100;
	w = arne * 128;
	d_render_tile_set(tile, 0, id * 2);
	d_render_tile_set(tile, 1, id * 2 + 1);
	d_render_tile_move(tile, 0, x_pos, 32);
	d_render_tile_move(tile, 1, x_pos + w, 32);
	d_render_tile_size_set(tile, 0, w, 16);
	d_render_tile_size_set(tile, 1, 128 - w, 16);
	d_render_tile_tilesheet_coord_set(tile, 0, 0, id * 16, w, 16);
	d_render_tile_tilesheet_coord_set(tile, 1, 128 + w, 16 * id, 128 - w, 16);
}


void healthbar_calculate() {
	/* This will be retardedly slow */
	int i;
	
	for (i = 0; i < 4; i++) 
		_calculate_one(healthbar.hp[i], healthbar.bar[i], 320 * i, i);
}


void healthbar_init() {
	healthbar.ts = d_render_tilesheet_load("res/bargraph.png", 128, 16, DARNIT_PFORMAT_RGB5A1);
	healthbar.bar[0] = d_render_tile_new(2, healthbar.ts);
	healthbar.bar[1] = d_render_tile_new(2, healthbar.ts);
	healthbar.bar[2] = d_render_tile_new(2, healthbar.ts);
	healthbar.bar[3] = d_render_tile_new(2, healthbar.ts);
}


void healthbar_draw() {
	int i;

	healthbar_calculate();
	d_render_offset(0, 0);

	for (i = 0; i < 4; i++)
		if (healthbar.hp[i])
			d_render_tile_draw(healthbar.bar[i], 2);
}


void healthbar_set(int id, int hp) {
	healthbar.hp[id] = hp;
}

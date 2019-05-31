#include <stddef.h>
#include "main.h"
#include "camera.h"


void camera_work() {
	int x, y;
	
	int tile_w, tile_h;

	if (cs->camera.follow < 0)
		return;
	x = cs->drawable->entry[cs->camera.follow].x;
	y = cs->drawable->entry[cs->camera.follow].y;

	x -= d_platform_get().screen_w / 2;
	y -= d_platform_get().screen_h / 2;
	
	d_render_tilesheet_geometrics(gfx.map_tilesheet, NULL, NULL, &tile_w, &tile_h);
	
	if (x + d_platform_get().screen_w > cs->map.layer[MAP_LAYER_TERRAIN]->w * tile_w)
		x = cs->map.layer[MAP_LAYER_TERRAIN]->w * tile_w - d_platform_get().screen_w;
	if (y + d_platform_get().screen_h > cs->map.layer[MAP_LAYER_TERRAIN]->h * tile_h)
		y = cs->map.layer[MAP_LAYER_TERRAIN]->h * tile_h - d_platform_get().screen_h;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	cs->camera.x = x;
	cs->camera.y = y;
}

#include "main.h"
#include "camera.h"


void camera_work() {
	int x, y;

	if (cs->camera.follow < 0)
		return;
	x = cs->drawable->entry[cs->camera.follow].x;
	y = cs->drawable->entry[cs->camera.follow].y;

	x -= d_platform_get().screen_w / 2;
	y -= d_platform_get().screen_h / 2;

	if (x + d_platform_get().screen_w > cs->active_level->layer->tilemap->w * cs->active_level->layer->tile_w)
		x = cs->active_level->layer->tilemap->w * cs->active_level->layer->tile_w - d_platform_get().screen_w;
	if (y + d_platform_get().screen_h > cs->active_level->layer->tilemap->h * cs->active_level->layer->tile_h)
		y = cs->active_level->layer->tilemap->h * cs->active_level->layer->tile_h - d_platform_get().screen_h;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	cs->camera.x = x;
	cs->camera.y = y;
}

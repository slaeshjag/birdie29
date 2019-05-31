#include <darnit/darnit.h>
#include "drawable.h"
#include "movable.h"
#include "spritelist.h"

struct Drawable *drawable_init() {
	int i;
	struct Drawable *drw;

	drw = malloc(sizeof(*drw));
	drw->entry = malloc(sizeof(*drw->entry) * MAX_MOVABLE);
	
	for (i = 0; i < MAX_MOVABLE; i++) {
		drw->entry[i].sprite = NULL;
		drw->entry[i].x = 0;
		drw->entry[i].y = 0;
		drw->entry[i].l = 0;
		drw->entry[i].angle = 0;
	}

	return drw;

}


void drawable_spawn(struct Drawable *drw, int sprite, int id, int x, int y, int l, int angle) {
	drw->entry[id].sprite = spritelist_get(sprite);
	drw->entry[id].x = x;
	drw->entry[id].y = y;
	drw->entry[id].l = l;
	drw->entry[id].angle = angle;
	drw->entry[id].dir = 0;

	d_sprite_move(drw->entry[id].sprite, x, y);
	d_sprite_rotate(drw->entry[id].sprite, angle);
	d_sprite_direction_set(drw->entry[id].sprite, 0);
	d_sprite_animate_start(drw->entry[id].sprite);
}


void drawable_despawn(struct Drawable *drw, int id) {
	if (drw->entry[id].sprite)
		d_sprite_free(drw->entry[id].sprite);
	drw->entry[id].sprite = NULL;
}


void drawable_render(struct Drawable *drw, int layer) {
	int i;

	for (i = 0; i < MAX_MOVABLE; i++) {
		if (!drw->entry[i].sprite)
			continue;
		if (drw->entry[i].l != layer)
			continue;
		d_sprite_draw(drw->entry[i].sprite);
	}
}


void drawable_move(struct Drawable *drw, int id, int x, int y, int angle, int dir) {
	if (!drw->entry[id].sprite)
		return (void) fprintf(stderr, "Move non-existant movable %i\n", id);
	if (drw->entry[id].dir != dir) 
		d_sprite_direction_set(drw->entry[id].sprite, dir);
	drw->entry[id].dir = dir;
	drw->entry[id].x = x;
	drw->entry[id].y = y;
	drw->entry[id].angle = angle;
	d_sprite_move(drw->entry[id].sprite, x, y);
	d_sprite_rotate(drw->entry[id].sprite, angle);
}

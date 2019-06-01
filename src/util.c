//fuck da police
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libgen.h>
#include <darnit/darnit.h>


int util_sprite_width(DARNIT_SPRITE *sprite) {
	int x, y, w, h;
	d_sprite_hitbox(sprite, &x, &y, &w, &h);
	return w;
}

int util_sprite_height(DARNIT_SPRITE *sprite) {
	int x, y, w, h;
	d_sprite_hitbox(sprite, &x, &y, &w, &h);
	return h;
}

int util_sprite_xoff(DARNIT_SPRITE *sprite) {
	int x, y, w, h;
	d_sprite_hitbox(sprite, &x, &y, &w, &h);
	return x;
}

int util_sprite_yoff(DARNIT_SPRITE *sprite) {
	int x, y, w, h;
	d_sprite_hitbox(sprite, &x, &y, &w, &h);
	return y;
}

double util_distance(double x1, double y1, double x2, double y2) {
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}


char *util_binrel_path(const char *file) {
	char *path;
	char *pathpart;
	
	pathpart = strdup(d_fs_exec_path());

	asprintf(&path, "%s/%s", dirname(pathpart), file);
	free(pathpart);

	return path;
}

#ifndef _UTIL_H__
#define	_UTIL_H__

#include <darnit/darnit.h>

char *util_binrel_path(const char *file);
int util_sprite_width(DARNIT_SPRITE *sprite);
int util_sprite_height(DARNIT_SPRITE *sprite);
int util_sprite_xoff(DARNIT_SPRITE *sprite);
int util_sprite_yoff(DARNIT_SPRITE *sprite);

double util_distance(double x1, double y1, double x2, double y2);

#endif

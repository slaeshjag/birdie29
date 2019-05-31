#ifndef DRAWABLE_H_
#define	DRAWABLE_H_


struct DrawableEntry {
	DARNIT_SPRITE				*sprite;
	int					x;
	int					y;
	int					l;
	int					dir;
	int					angle;
};



struct Drawable {
	struct DrawableEntry			*entry;
};


void drawable_move(struct Drawable *drw, int id, int x, int y, int angle, int dir);
void drawable_render(struct Drawable *drw, int layer);
void drawable_despawn(struct Drawable *drw, int id);
void drawable_spawn(struct Drawable *drw, DARNIT_SPRITE *spr, int id, int x, int y, int l, int angle);
struct Drawable *drawable_init();

#endif

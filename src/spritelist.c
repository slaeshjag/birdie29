#include <darnit/darnit.h>
#include "spritelist.h"


static char *_spritelist_path[SPRITELIST_ENTRY_INDICES] = {
	"res/player.spr",
	NULL,
};


static DARNIT_SPRITE *_spritelist_sprite[SPRITELIST_ENTRY_INDICES];


void spritelist_init() {
	int i;

	for (i = 0; i < SPRITELIST_ENTRY_INDICES; i++)
		if (_spritelist_path[i])
			_spritelist_sprite[i] = d_sprite_load(_spritelist_path[i], 0, DARNIT_PFORMAT_RGBA8);
	return;
}


DARNIT_SPRITE *spritelist_get(enum SpritelistEntryIndex sprite) {
	if (_spritelist_sprite[sprite])
		return d_sprite_copy(_spritelist_sprite[sprite]);
	return NULL;
}

#ifndef SPRITELIST_H_
#define	SPRITELIST_H_

#include <darnit/darnit.h>

enum SpritelistEntryIndex {
	SPRITELIST_ENTRY_INDEX_PLAYER_0_TYPE_0,
	SPRITELIST_ENTRY_INDICES
};


struct SpritelistEntry {
	char			*path;
	DARNIT_SPRITE		*sprite;
};


void spritelist_init();
DARNIT_SPRITE *spritelist_get(enum SpritelistEntryIndex sprite);



#endif

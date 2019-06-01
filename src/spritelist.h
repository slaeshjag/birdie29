#ifndef SPRITELIST_H_
#define	SPRITELIST_H_

#include <darnit/darnit.h>

enum SpritelistEntryIndex {
	SPRITELIST_ENTRY_INDEX_TEAM_0_TYPE_0,
	SPRITELIST_ENTRY_INDEX_TEAM_0_TYPE_1,
	SPRITELIST_ENTRY_INDEX_TEAM_0_TYPE_2,
	
	SPRITELIST_ENTRY_INDEX_TEAM_1_TYPE_0,
	SPRITELIST_ENTRY_INDEX_TEAM_1_TYPE_1,
	SPRITELIST_ENTRY_INDEX_TEAM_1_TYPE_2,
	
	SPRITELIST_ENTRY_INDEX_TEAM_2_TYPE_0,
	SPRITELIST_ENTRY_INDEX_TEAM_2_TYPE_1,
	SPRITELIST_ENTRY_INDEX_TEAM_2_TYPE_2,
	
	SPRITELIST_ENTRY_INDEX_TEAM_3_TYPE_0,
	SPRITELIST_ENTRY_INDEX_TEAM_3_TYPE_1,
	SPRITELIST_ENTRY_INDEX_TEAM_3_TYPE_2,
	
	SPRITELIST_ENTRY_INDEX_BULLET_WIMPY,
	SPRITELIST_ENTRY_INDEX_BULLET_BADASS,
	SPRITELIST_ENTRY_INDICES
};


struct SpritelistEntry {
	char			*path;
	DARNIT_SPRITE		*sprite;
};


void spritelist_init();
DARNIT_SPRITE *spritelist_get(enum SpritelistEntryIndex sprite);



#endif

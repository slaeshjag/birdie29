#include <darnit/darnit.h>
#include "spritelist.h"


static char *_spritelist_path[SPRITELIST_ENTRY_INDICES] = {
	[SPRITELIST_ENTRY_INDEX_TEAM_0_TYPE_0] = "res/team0_var0.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_0_TYPE_1] = "res/team0_var1.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_0_TYPE_2] = "res/team0_var2.spr",

	[SPRITELIST_ENTRY_INDEX_TEAM_1_TYPE_0] = "res/team1_var0.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_1_TYPE_1] = "res/team1_var1.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_1_TYPE_2] = "res/team1_var2.spr",

	[SPRITELIST_ENTRY_INDEX_TEAM_2_TYPE_0] = "res/team2_var0.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_2_TYPE_1] = "res/team2_var1.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_2_TYPE_2] = "res/team2_var2.spr",

	[SPRITELIST_ENTRY_INDEX_TEAM_3_TYPE_0] = "res/team3_var0.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_3_TYPE_1] = "res/team3_var1.spr",
	[SPRITELIST_ENTRY_INDEX_TEAM_3_TYPE_2] = "res/team3_var2.spr",

	[SPRITELIST_ENTRY_INDEX_BULLET_WIMPY] = "res/bullet_wimpy.spr",
	[SPRITELIST_ENTRY_INDEX_BULLET_BADASS] = "res/bullet_badass.spr",
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

#ifndef HUD_H_
#define HUD_H_

#include <muil/muil.h>
#include <darnit/darnit.h>
#include "config.h"
#include "server/unit.h"

typedef struct Hud Hud;
struct Hud{
	struct MuilPaneList pane;
	MuilWidget *hbox;
	MuilWidget *picture[UNIT_TYPES];
	
	DARNIT_TILESHEET *unit_tilesheet[UNIT_TYPES];
	DARNIT_TILESHEET *selected_frame;
	DARNIT_TILESHEET *_7seg;
	
	struct {
		struct MuilPaneList pane;
		MuilWidget *vbox;
		MuilWidget *title;
		MuilWidget *label[TEAMS_CAP];
	} scoreboard;
};

extern Hud hud;

#endif

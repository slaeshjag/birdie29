#ifndef MENU_H_
#define MENU_H_

#include <muil/muil.h>
#include <darnit/darnit.h>

typedef struct Menu Menu;
struct Menu {
	struct MuilPaneList pane;
	MuilWidget *vbox;
	struct {
		MuilWidget *title;
		MuilWidget *author;
	} label;
	struct {
		MuilWidget *host;
		MuilWidget *join;
		MuilWidget *character;
		MuilWidget *quit;
	} button;
}; 

typedef struct MenuHelp MenuHelp;
struct MenuHelp {
	struct MuilPaneList pane;
	MuilWidget *vbox;
	struct {
		MuilWidget *title;
		MuilWidget *help;
		MuilWidget *web;
	} label;
};

typedef struct SelectName SelectName;
struct SelectName{
	struct MuilPaneList pane;
	MuilWidget *vbox;
	MuilWidget *hbox;
	MuilWidget *label;
	MuilWidget *entry;
	MuilWidget *picture;
	MuilWidget *slider;
	struct {
		MuilWidget *ok;
		MuilWidget *quit;
	} button;
	
	
	DARNIT_TILESHEET *variant_tilesheet[PLAYER_VARIANTS];
};

extern Menu menu;
extern SelectName select_name;

void menu_init();
void menu_render();

#endif

#include <darnit/darnit.h>
#include <muil/muil.h>
#include "main.h"
#include "menu.h"

Menu menu;
SelectName select_name;
MenuHelp menu_help;

static void button_callback_name(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	MuilPropertyValue v;
	if(widget == select_name.button.quit) {
		d_quit();
	}
	v = select_name.entry->get_prop(select_name.entry, MUIL_ENTRY_PROP_TEXT);
	snprintf(me.name, NAME_LEN_MAX, "%s", (char *) v.p);
	
	v = select_name.listbox->get_prop(select_name.listbox, MUIL_LISTBOX_PROP_SELECTED);
	me.sprite_variant = v.i;
	
	game_state(GAME_STATE_MENU);
}

static void button_callback_menu(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	/*MuilPropertyValue v;
	v.p = "arne";
	label->set_prop(label, MUIL_LABEL_PROP_TEXT, v);*/
	
	if(widget == menu.button.host) {
		game_state(GAME_STATE_HOST);
	} else if(widget == menu.button.join) {
		game_state(GAME_STATE_LOBBY);
	} else if(widget == menu.button.character) {
		game_state(GAME_STATE_CHARACTERS);
	} else if(widget == menu.button.quit)
		d_quit();
}

static void listbox_callback_picture(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	MuilPropertyValue v;
	
	v = widget->get_prop(widget, MUIL_LISTBOX_PROP_SELECTED);
	
	v.p = select_name.variant_tilesheet[v.i];
	select_name.picture->set_prop(select_name.picture, MUIL_IMAGEVIEW_PROP_TILESHEET, v);
	select_name.picture->set_prop(select_name.picture, MUIL_IMAGEVIEW_PROP_TILESHEET, v);
}

void menu_select_name_init() {
	MuilPropertyValue v;
	
	select_name.pane.pane = muil_pane_create(DISPLAY_WIDTH/2 - 200, DISPLAY_HEIGHT/2 - 400/2, 400, 400, select_name.vbox = muil_widget_create_vbox());
	select_name.pane.next = NULL;
	
	select_name.pane.pane->background_color.r = PANE_R;
	select_name.pane.pane->background_color.g = PANE_G;
	select_name.pane.pane->background_color.b = PANE_B;
	
	muil_vbox_add_child(select_name.vbox, select_name.label = muil_widget_create_label(gfx.font.large, "Customize your character"), 1);
	muil_vbox_add_child(select_name.vbox, select_name.entry = muil_widget_create_entry(gfx.font.small), 0);
	
	muil_vbox_add_child(select_name.vbox, select_name.picture = muil_widget_create_imageview_raw(48, 48, DARNIT_PFORMAT_RGBA8), 0);
	muil_vbox_add_child(select_name.vbox, select_name.listbox = muil_widget_create_listbox(gfx.font.small), 1);
	
	muil_listbox_add(select_name.listbox, "Com-Rad");
	muil_listbox_add(select_name.listbox, "DBS-52-74");
	muil_listbox_add(select_name.listbox, "Bruce");
	
	v.i = 0;
	select_name.listbox->set_prop(select_name.listbox, MUIL_LISTBOX_PROP_SELECTED, v);
	
	select_name.hbox = muil_widget_create_hbox();
	muil_vbox_add_child(select_name.hbox, select_name.button.ok = muil_widget_create_button_text(gfx.font.small, "OK"), 0);
	muil_vbox_add_child(select_name.hbox, select_name.button.quit = muil_widget_create_button_text(gfx.font.small, "Quit"), 0);
	muil_vbox_add_child(select_name.vbox, select_name.hbox, 0);
	
	#ifdef _WIN32
	v.p = getenv("USERNAME");
	#else
	v.p = getenv("LOGNAME");
	#endif
	select_name.entry->set_prop(select_name.entry, MUIL_ENTRY_PROP_TEXT, v);
	
	select_name.button.ok->event_handler->add(select_name.button.ok, button_callback_name, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	select_name.button.quit->event_handler->add(select_name.button.quit, button_callback_name, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	
	select_name.listbox->event_handler->add(select_name.listbox, listbox_callback_picture, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	
	select_name.variant_tilesheet[0] = d_render_tilesheet_load("res/player_var0.png", 48, 48, DARNIT_PFORMAT_RGBA8);
	select_name.variant_tilesheet[1] = d_render_tilesheet_load("res/player_var1.png", 48, 48, DARNIT_PFORMAT_RGBA8);
	select_name.variant_tilesheet[2] = d_render_tilesheet_load("res/player_var2.png", 48, 48, DARNIT_PFORMAT_RGBA8);
	//select_name.variant_tilesheet[3] = d_render_tilesheet_load("res/player_var3.png", 48, 48, DARNIT_PFORMAT_RGBA8);
	
	v.p = select_name.variant_tilesheet[0];
	select_name.picture->set_prop(select_name.picture, MUIL_IMAGEVIEW_PROP_TILESHEET, v);
	select_name.picture->set_prop(select_name.picture, MUIL_IMAGEVIEW_PROP_TILESHEET, v);
}

void menu_init() {
	//MuilPropertyValue v;
	menu.pane.pane = muil_pane_create(10, 10, DISPLAY_WIDTH - 20, 220, menu.vbox = muil_widget_create_vbox());
	menu.pane.next = &menu_help.pane;
	
	menu.pane.pane->background_color.r = PANE_R;
	menu.pane.pane->background_color.g = PANE_G;
	menu.pane.pane->background_color.b = PANE_B;
	
	menu_help.pane.pane = muil_pane_create(10, 210 + 30, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 210 - 20 - 20, menu_help.vbox = muil_widget_create_vbox());
	menu_help.pane.next = NULL;
	
	menu_help.pane.pane->background_color.r = PANE_R;
	menu_help.pane.pane->background_color.g = PANE_G;
	menu_help.pane.pane->background_color.b = PANE_B;
	
	muil_vbox_add_child(menu_help.vbox, menu_help.label.title = muil_widget_create_label(gfx.font.large, "How to play"), 0);
	muil_vbox_add_child(menu_help.vbox, muil_widget_create_spacer_size(0, 10), 0);
	muil_vbox_add_child(menu_help.vbox, menu_help.label.title = muil_widget_create_label(gfx.font.small, 
		"You are a scavenging robot left in a post-apocalyptic world.\n\n"
		"You must build and defend your base while at the same time mining for nuclear fuel resources. "
		"Use the WASD keys to move, mouse to aim and position your building cursor. "
		"TAB or mouse wheel cycles through the available buildings, SPACE places a builing. \n\n"
		"Buildings can only be built within your powergrid, supplied by your generator. "
		"Extend your power grid using pylon, but beware! If the pylons lose connection with the generator, your buildings connected to those pylons will stop working. \n\n"
		"Miners can be placed on nuclear fuel depots and will steadily generate income. "
		"Protect yourself using walls and turrets, or by shooting projectiles using left or right mouse button. "
		"After a grace period, a timer will start counting for the player who has the most fuel reserves on stock. "
		"The team that manages to stay in the lead longest will manage to get their counter down first and will win the game. \n\n"
		"Beware that building buildings and firing projectiles will use up your fuel stocks. "
		"Be wise and cunning. Aiming for the opponents power grid close to their generator will disable their entire power distribution, rendering all their miners inoperable. "
	), 0);
	muil_vbox_add_child(menu_help.vbox, muil_widget_create_spacer(), 1);
	muil_vbox_add_child(menu_help.vbox, menu_help.label.title = muil_widget_create_label(gfx.font.small, "2019 party hack for Birdie Gamedev compo\nCode by h4xxel (http://h4xxel.org), slaesjag (http://rdw.se) & Kalops of //actung fulkod\nSound effects by B2"), 0);
	
	muil_vbox_add_child(menu.vbox, menu.label.title = muil_widget_create_label(gfx.font.large, GAME_TITLE), 1);
	muil_vbox_add_child(menu.vbox, menu.label.author = muil_widget_create_label(gfx.font.small, "//actung fulkod"), 1);
	//muil_vbox_add_child(menu.vbox, muil_widget_create_entry(font), 0);
	muil_vbox_add_child(menu.vbox, menu.button.host = muil_widget_create_button_text(gfx.font.small, "Host game"), 0);
	muil_vbox_add_child(menu.vbox, menu.button.join = muil_widget_create_button_text(gfx.font.small, "Join game"), 0);
	//muil_vbox_add_child(menu.vbox, menu.button.character = muil_widget_create_button_text(gfx.font.small, "Character Showcase"), 0);
	muil_vbox_add_child(menu.vbox, menu.button.quit = muil_widget_create_button_text(gfx.font.small, "Quit game"), 0);

	menu.button.host->event_handler->add(menu.button.host, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	menu.button.join->event_handler->add(menu.button.join, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	//menu.button.character->event_handler->add(menu.button.character, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	menu.button.quit->event_handler->add(menu.button.quit, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	
	menu_select_name_init();
}

void menu_render() {
	
}

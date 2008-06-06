#ifndef LXTERMINAL_H
#define LXTERMINAL_H

#include "tab.h"

#define NEW_TAB_ACCEL "<CTRL><SHIFT>T"
#define CLOSE_TAB_ACCEL "<CTRL><SHIFT>W"
#define QUIT_ACCEL "<CTRL><SHIFT>Q"
#define COPY_ACCEL "<CTRL><SHIFT>C"
#define PASTE_ACCEL "<CTRL><SHIFT>V"
#define RENAME_TAB_ACCEL "<CTRL><SHIFT>R"
#define PREVIOUS_TAB_ACCEL "<CTRL><SHIFT>P"
#define NEXT_TAB_ACCEL "<CTRL><SHIFT>N"
#define PREVIOUS_TAB_ACCEL_GNOME "<CTRL><SHIFT>Page_Down"
#define NEXT_TAB_ACCEL_GNOME "<CTRL><SHIFT>Page_Up"
#define NEXT_TAB_ACCEL_GNOME "<CTRL><SHIFT>Page_Up"
#define SWITCH_TAB1_ACCEL "<ALT>1"
#define SWITCH_TAB2_ACCEL "<ALT>2"
#define SWITCH_TAB3_ACCEL "<ALT>3"
#define SWITCH_TAB4_ACCEL "<ALT>4"
#define SWITCH_TAB5_ACCEL "<ALT>5"
#define SWITCH_TAB6_ACCEL "<ALT>6"
#define SWITCH_TAB7_ACCEL "<ALT>7"
#define SWITCH_TAB8_ACCEL "<ALT>8"
#define SWITCH_TAB9_ACCEL "<ALT>9"

typedef struct _menu {
	GtkWidget      *menu;
	GtkItemFactory *item_factory;
	GtkAccelGroup  *accel_group;
} Menu;

typedef struct _lxterminal {
	GtkWidget *mainw;
	GtkWidget *box;
	Menu      *menubar;
	GtkWidget *notebook;
	GPtrArray *terms;
} LXTerminal;

typedef struct _term {
	gint index;
	LXTerminal *parent;
	LXTab *label;
	GtkWidget *vte;
	GtkWidget *scrollbar;
	GtkWidget *box;
} Term;

#endif

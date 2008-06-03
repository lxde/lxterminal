#ifndef LXTERMINAL_H
#define LXTERMINAL_H

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
	GtkWidget *vte;
	GtkWidget *label;
	GtkWidget *scrollbar;
	GtkWidget *box;
} Term;

#endif

#ifndef LXTERMINAL_H
#define LXTERMINAL_H

#include "setting.h"

#define NEW_WINDOW_ACCEL "<CTRL><SHIFT>N"
#define NEW_TAB_ACCEL "<CTRL><SHIFT>T"
#define CLOSE_TAB_ACCEL "<CTRL><SHIFT>W"
#define QUIT_ACCEL "<CTRL><SHIFT>Q"
#define COPY_ACCEL "<CTRL><SHIFT>C"
#define PASTE_ACCEL "<CTRL><SHIFT>V"
#define RENAME_TAB_ACCEL "<CTRL><SHIFT>R"
#define PREVIOUS_TAB_ACCEL "<CTRL>Page_Up"
#define NEXT_TAB_ACCEL "<CTRL>Page_Down"
#define MOVE_TAB_LEFT_ACCEL "<CTRL><SHIFT>Page_Up"
#define MOVE_TAB_RIGHT_ACCEL "<CTRL><SHIFT>Page_Down"

/* steal from tilda-0.09.6/src/tilda_terminal.c:36 */
#define DINGUS1 "(((news|telnet|nttp|file|http|ftp|https)://)|(www|ftp)[-A-Za-z0-9]*\\.)[-A-Za-z0-9\\.]+(:[0-9]*)?"
#define DINGUS2 "(((news|telnet|nttp|file|http|ftp|https)://)|(www|ftp)[-A-Za-z0-9]*\\.)[-A-Za-z0-9\\.]+(:[0-9]*)?/[-A-Za-z0-9_\\$\\.\\+\\!\\*\\(\\),;:@&=\\?/~\\#\\%]*[^]'\\.}>\\) ,\\\"]"

typedef struct _lxtermwindow {
	Setting   *setting;
	GPtrArray *windows;
} LXTermWindow;

typedef struct _menu {
	GtkWidget      *menu;
	GtkItemFactory *item_factory;
	GtkAccelGroup  *accel_group;
} Menu;

typedef struct _lxterminal {
	LXTermWindow *parent;
	gint       index;
	GtkWidget *mainw;
	GtkWidget *box;
	Menu      *menubar;
	GtkWidget *notebook;
	GPtrArray *terms;
	gint       resize_idle_id;
	Setting   *setting;
	GdkGeometry geometry;
	GdkWindowHints geom_mask;
	gboolean fixedsize;
	gboolean rgba;
	GdkColor background;
	GdkColor foreground;
	gint     tabpos;
	gboolean hidemenubar;
	gboolean hidescrollbar;
	gint cursormode;
	gboolean cursorblinks;
} LXTerminal;

typedef struct _tab {
	GtkWidget *main;
	GtkWidget *label;
	GtkWidget *close_btn;
} LXTab;

typedef struct _term {
	gint index;
	LXTerminal *parent;
	LXTab *label;
	GtkWidget *vte;
	GtkWidget *scrollbar;
	GtkWidget *box;
	pid_t pid;
	GClosure* closure;
} Term;

LXTerminal *lxterminal_init(LXTermWindow *lxtermwin, gint argc, gchar **argv, Setting *setting);
void terminal_setting_update(LXTerminal *terminal);

#endif

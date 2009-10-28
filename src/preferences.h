#ifndef LXTERMINAL_PREFERENCES_H
#define LXTERMINAL_PREFERENCES_H

typedef struct {
	GtkWidget *label_box;
	GtkWidget *icon;
	GtkWidget *label;
	GtkWidget *page;
	void *childs;
} TabWidget; 

typedef struct {
	LXTerminal *terminal;
	GtkWidget *dialog;
	GtkWidget *notebook;
	GPtrArray *tab;
} Prefer;

typedef struct {
	gchar *name;
	gchar *icon;
	void (*constructor)(Prefer *prefer, TabWidget *tab);
	void (*destructor)(Prefer *prefer, TabWidget *tab);
	void (*save)(Prefer *prefer, TabWidget *tab);
} TabGroup;

typedef struct {
	GtkWidget *box;
	GtkWidget *font_label;
	GtkWidget *font_button;
	GtkWidget *bgcolor_label;
	GtkWidget *bgcolor_entry;
	GtkWidget *fgcolor_label;
	GtkWidget *fgcolor_entry;
	GtkWidget *cursorblinks_label;
	GtkWidget *cursorblinks_checkbox;
} PreferStyle;

typedef struct {
	GtkWidget *box;
	GtkWidget *tabpos_label;
	GtkWidget *tabpos_combobox;
	GtkWidget *scrollback_label;
	GtkWidget *scrollback_entry;
	GtkWidget *hidescrollbar_label;
	GtkWidget *hidescrollbar_checkbox;
	GtkWidget *hidemenubar_label;
	GtkWidget *hidemenubar_checkbox;
} PreferDisplay;

typedef struct {
	GtkWidget *box;
	GtkWidget *selchars_label;
	GtkWidget *selchars_entry;
	GtkWidget *disablef10_label;
	GtkWidget *disablef10_checkbox;
} PreferMisc;

void lxterminal_preferences_dialog(GtkAction *action, gpointer data);

#endif

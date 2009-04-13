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
	GtkWidget *selchars_label;
	GtkWidget *selchars_entry;
	GtkWidget *bgcolor_label;
	GtkWidget *bgcolor_entry;
	GtkWidget *fgcolor_label;
	GtkWidget *fgcolor_entry;
	GtkWidget *bgtransparent_label;
	GtkWidget *bgtransparent_checkbox;
	GtkWidget *scrollback_label;
	GtkWidget *scrollback_entry;
	GtkWidget *tabpos_label;
	GtkWidget *tabpos_combobox;
	GtkWidget *disablef10_label;
	GtkWidget *disablef10_checkbox;
} PreferGeneral;

void lxterminal_preferences_dialog(GtkAction *action, gpointer data);

#endif

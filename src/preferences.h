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
	GtkWidget *font_box;
	GtkWidget *font_label;
	GtkWidget *font_button;
	GtkWidget *selchars_box;
	GtkWidget *selchars_label;
	GtkWidget *selchars_entry;
} PreferGeneral;

void lxterminal_preferences_dialog(LXTerminal *terminal, guint action, GtkWidget *item);

#endif

/*
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <vte/vte.h>

#include "lxterminal.h"
#include "setting.h"
#include "tab.h"
#include "preferences.h"

static GtkItemFactoryEntry menu_items[] =
{
	{ N_("/_File"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_File/_New Tab"), NEW_TAB_ACCEL, terminal_newtab, 1, "<StockItem>", GTK_STOCK_ADD },
	{ N_("/_File/_Close Tab"), CLOSE_TAB_ACCEL, terminal_closetab, 1, "<StockItem>", GTK_STOCK_CLOSE },
	{ N_("/_File/sep1"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_File/_Quit"), QUIT_ACCEL, gtk_main_quit, 0, "<StockItem>", GTK_STOCK_QUIT },
	{ N_("/_Edit"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_Edit/_Copy"), COPY_ACCEL, terminal_copy, 0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/_Edit/_Paste"), PASTE_ACCEL, terminal_paste, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/_Edit/sep1"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Edit/_Preferences..."), NULL, lxterminal_preferences_dialog, 0, "<StockItem>", GTK_STOCK_EXECUTE },
	{ N_("/_Tabs"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_Tabs/_Previous Tab"), PREVIOUS_TAB_ACCEL, terminal_prevtab, 1, "<Item>"},
	{ N_("/_Tabs/_Next Tab"), NEXT_TAB_ACCEL, terminal_nexttab, 1, "<Item>"},
};  

static GtkItemFactoryEntry vte_menu_items[] =
{
	{ N_("/_New Tab"), NEW_TAB_ACCEL, terminal_newtab, 1, "<StockItem>", GTK_STOCK_ADD },
	{ N_("/sep1"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Copy"), COPY_ACCEL, terminal_copy, 0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/_Paste"), PASTE_ACCEL, terminal_paste, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/sep2"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Preferences..."), NULL, lxterminal_preferences_dialog, 0, "<StockItem>", GTK_STOCK_EXECUTE },
	{ N_("/sep3"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Close Tab"), CLOSE_TAB_ACCEL, terminal_closetab, 1, "<StockItem>", GTK_STOCK_CLOSE }
};

void terminal_switchtab1(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 0);
}

void terminal_switchtab2(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 1);
}

void terminal_switchtab3(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 2);
}

void terminal_switchtab4(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 3);
}

void terminal_switchtab5(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 4);
}

void terminal_switchtab6(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 5);
}

void terminal_switchtab7(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 6);
}

void terminal_switchtab8(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 7);
}

void terminal_switchtab9(LXTerminal *terminal)
{
	gtk_notebook_set_current_page(terminal->notebook, 8);
}

void terminal_copy(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

	/* getting current vte */
	term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(terminal->notebook));

	/* copy from vte */
	vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

void terminal_paste(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

	/* getting current vte */
	term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(terminal->notebook));

	/* copy from vte */
	vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

void terminal_nexttab(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal = (LXTerminal *)data;

	/* cycle */
	if (gtk_notebook_get_current_page(terminal->notebook)==gtk_notebook_get_n_pages(terminal->notebook)-1)
		gtk_notebook_set_current_page(terminal->notebook, 0);
	else
		gtk_notebook_next_page(terminal->notebook);
}

void terminal_prevtab(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal = (LXTerminal *)data;

	/* cycle */
	if (gtk_notebook_get_current_page(terminal->notebook)==0)
		gtk_notebook_set_current_page(terminal->notebook, -1);
	else
		gtk_notebook_prev_page(terminal->notebook);
}

void terminal_closetab(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

	/* getting current vte */
	term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(terminal->notebook));

	/* release child */
	terminal_childexit(VTE_TERMINAL(term->vte), term);
}

void terminal_newtab(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal = (LXTerminal *)data;

	Term *term = terminal_new(terminal, "LXTerminal", g_get_current_dir(), NULL);
	
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->label->main);
    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
    g_ptr_array_add(terminal->terms, term);

	/* push VTE to queue */
	gtk_widget_queue_draw(term->vte);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(term->parent->notebook), term->index);

	if (term->index > 0)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(term->parent->notebook), TRUE);
}

void terminal_switch_tab(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
	gchar *title;
	Term *term;
	LXTerminal *terminal = (LXTerminal *)data;

	if (terminal->terms->len <= num)
		return;

	term = g_ptr_array_index(terminal->terms, num);

	/* if title of VTE is NULL */
	if ((title = vte_terminal_get_window_title(VTE_TERMINAL(term->vte))) == NULL)
		gtk_window_set_title(GTK_WINDOW(terminal->mainw), "LXTerminal");
	else
		gtk_window_set_title(GTK_WINDOW(terminal->mainw), title);

}

void terminal_title_changed(VteTerminal *vte, Term *term)
{
	/* setting label */
	lxterminal_tab_label_set_text(term->label, vte_terminal_get_window_title(VTE_TERMINAL(vte)));

	/* setting window title */
	gtk_window_set_title(GTK_WINDOW(term->parent->mainw), vte_terminal_get_window_title(VTE_TERMINAL(vte)));
}

void terminal_childexit(VteTerminal *vte, Term *term)
{
	int i;
	LXTerminal *terminal = term->parent;

	if(gtk_notebook_get_n_pages(terminal->notebook)==1) {
		/* release */
		g_ptr_array_free(terminal->terms, TRUE);

		/* quit */
		gtk_main_quit();
	} else {
		/* remove page */
		g_ptr_array_remove_index(terminal->terms, term->index);

		/* decreasing index number after the page be removed */
		for (i=term->index;i<terminal->terms->len;i++) {
			Term *t = g_ptr_array_index(terminal->terms, i);
			t->index--;
		}

		/* release */
		gtk_notebook_remove_page(terminal->notebook, term->index);
		g_free(term);

		/* if only one page, hide tab */
		if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) == 1)
			gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);

	}
}

gboolean terminal_vte_button_press(VteTerminal *vte, GdkEventButton *event, gpointer data)
{
	GtkItemFactory *item_factory;

	if (event->button == 3) {
		item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
		gtk_item_factory_create_items(item_factory, sizeof(vte_menu_items) / sizeof(vte_menu_items[0]), vte_menu_items, data);
		gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(item_factory, "<main>")), NULL, NULL, NULL, NULL, event->button, event->time);
	}

	return FALSE;
}

Term *terminal_new(LXTerminal *terminal, const gchar *label, const gchar *pwd, const gchar **env)
{
	Term *term;
	GdkColor black = {0};

	/* create terminal */
	term = g_new0(Term, 1);
	term->parent = terminal;
	term->vte = vte_terminal_new();
	term->box = gtk_hbox_new(FALSE, 0);
	term->scrollbar = gtk_vscrollbar_new(NULL);
	gtk_box_pack_start(GTK_BOX(term->box), term->vte, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(term->box), term->scrollbar, FALSE, TRUE, 0);

	/* setting terminal */
	vte_terminal_set_font_from_string(term->vte, terminal->setting->fontname);
	vte_terminal_set_color_background(term->vte, &black);

	/* create label for tab */
	term->label = lxterminal_tab_label_new(label);
	lxterminal_tab_label_close_button_clicked(G_CALLBACK(terminal_childexit), term);

	/* setting scrollbar */
	gtk_range_set_adjustment(GTK_RANGE(term->scrollbar), VTE_TERMINAL(term->vte)->adjustment);

	/* terminal fork */
	vte_terminal_fork_command(VTE_TERMINAL(term->vte), NULL, NULL, env, pwd, FALSE, TRUE, TRUE);

	/* signal handler */
	g_signal_connect(term->vte, "child-exited", G_CALLBACK(terminal_childexit), term);
	g_signal_connect(term->vte, "window-title-changed", G_CALLBACK(terminal_title_changed), term);
	g_signal_connect(term->vte, "button-press-event", G_CALLBACK(terminal_vte_button_press), terminal);
	
	gtk_widget_show_all(term->box);

	return term;
}

Term *terminal_init(LXTerminal *terminal)
{
	Term *term;

	term = terminal_new(terminal, "LXTerminal", g_get_current_dir(), NULL);

	return term;
}

Menu *menubar_init(LXTerminal *terminal)
{
	Menu *menubar;

	menubar = g_new0(Menu, 1);

	menubar->item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", NULL);
    gtk_item_factory_create_items(menubar->item_factory, sizeof(menu_items) / sizeof(menu_items[0]), menu_items, terminal);
    menubar->menu = gtk_item_factory_get_widget(menubar->item_factory, "<main>");

	return menubar;
}

void lxterminal_accelerator_init(LXTerminal *terminal)
{
	guint key;
	GdkModifierType mods;

	terminal->menubar->accel_group = gtk_accel_group_new();

	gtk_accelerator_parse(NEW_TAB_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_newtab, terminal, NULL));

	gtk_accelerator_parse(CLOSE_TAB_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_closetab, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB1_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab1, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB2_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab2, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB3_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab3, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB4_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab4, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB5_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab5, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB6_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab6, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB7_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab7, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB8_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab8, terminal, NULL));

	gtk_accelerator_parse(SWITCH_TAB9_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(terminal_switchtab9, terminal, NULL));

	gtk_accel_group_lock(terminal->menubar->accel_group);
	gtk_window_add_accel_group(GTK_WINDOW(terminal->mainw), terminal->menubar->accel_group);
}

void terminal_setting_update(LXTerminal *terminal, Setting *setting)
{
	Term *term;
	gint i;

	/* update all of terminals */
	for (i=0;i<terminal->terms->len;i++) {
		term = g_ptr_array_index(terminal->terms, i);

		vte_terminal_set_font_from_string(term->vte, terminal->setting->fontname);
//		vte_terminal_set_color_background(term->vte, &black);
	}
}

gboolean terminal_window_resize(LXTerminal *terminal)
{
	Term *term;
	GdkGeometry hints;
	gint i;
	gint xpad;
	gint ypad;
	gint grid_width;
	gint grid_height;
  
	/* The trick is rather simple here. This is called before any Gtk+ resizing operation takes
	 * place, so the columns/rows on the active terminal screen are still set to their old values.
	 * We simply query these values and force them to be set with the new style.
	 */
	term = g_ptr_array_index(terminal->terms, 0);
	vte_terminal_get_padding(VTE_TERMINAL(term->vte), &xpad, &ypad);
	hints.base_width = xpad;
	hints.base_height = ypad;
	hints.width_inc = VTE_TERMINAL(term->vte)->char_width;
	hints.height_inc = VTE_TERMINAL(term->vte)->char_height;
	hints.min_width = hints.base_width + hints.width_inc * 4;
	hints.min_height = hints.base_height + hints.height_inc * 2;

	for (i=0;i<terminal->terms->len;i++) {
		term = g_ptr_array_index(terminal->terms, i);

		gtk_window_set_geometry_hints (GTK_WINDOW(terminal->mainw),
									term->vte,
									&hints,
									GDK_HINT_RESIZE_INC
									| GDK_HINT_MIN_SIZE
									| GDK_HINT_BASE_SIZE);
	}

	return FALSE;
}

void terminal_window_resize_destroy(LXTerminal *terminal)
{
	g_source_remove(terminal->resize_idle_id);
}

LXTerminal *lxterminal_init(gint argc, gchar **argv, Setting *setting)
{
	LXTerminal *terminal;
	Term *term;

	terminal = g_new0(LXTerminal, 1);
	terminal->terms = g_ptr_array_new();

	/* Setting */
	if (setting)
		terminal->setting = setting;

	/* create window */
	terminal->mainw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(terminal->mainw), "LXTerminal");
    g_signal_connect(terminal->mainw, "delete_event", gtk_main_quit, NULL);

	/* create box for putting menubar and notebook */
	terminal->box = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(terminal->mainw), terminal->box);

	/* create menuar */
	terminal->menubar = menubar_init(terminal);
	gtk_box_pack_start(GTK_BOX(terminal->box), terminal->menubar->menu, FALSE, TRUE, 0);

	/* create notebook */
	terminal->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(terminal->notebook), TRUE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);
    g_signal_connect(terminal->notebook, "switch-page", G_CALLBACK(terminal_switch_tab), terminal);
    gtk_box_pack_start(GTK_BOX(terminal->box), terminal->notebook, TRUE, TRUE, 0);

	/* create terminal */
	term = terminal_init(terminal);
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->label->main);
    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
    g_ptr_array_add(terminal->terms, term);

	/* initializing accelerator */
	lxterminal_accelerator_init(terminal);

	gtk_widget_show_all(terminal->mainw);

	/* Gtk+ uses a priority of G_PRIORITY_HIGH_IDLE + 10 for resizing operations, so we
	 * use a slightly higher priority for the reset size operation.
	 */
	terminal->resize_idle_id = g_idle_add_full(G_PRIORITY_HIGH_IDLE + 5,
												(GSourceFunc) terminal_window_resize, terminal,
												(GDestroyNotify) terminal_window_resize_destroy);

	return terminal;
}

int main(gint argc, gchar** argv)
{
	LXTerminal *terminal;
	Setting *setting;
	gchar *dir, *path;
	gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
	bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
	bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
	textdomain ( GETTEXT_PACKAGE );
#endif

	/* load config file */
	dir = g_build_filename(g_get_user_config_dir(), "lxterminal" , NULL);
	g_mkdir_with_parents(dir, 0700);
	path = g_build_filename(dir, "lxterminal.conf", NULL);
	g_free(dir);

	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		setting = load_setting_from_file(PACKAGE_DATA_DIR "/lxterminal/lxterminal.conf");
		/* save to user's directory */
		setting_save(setting);
	} else {
		setting = load_setting_from_file(path);
	}

	g_free(path);

	/* initializing LXTerminal */
	terminal = lxterminal_init(argc, argv, setting);

	gtk_main();

    return 0;
}

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

LXTerminal *global_terminal;

Term *terminal_new(LXTerminal *terminal, const gchar *label, const gchar *pwd, const gchar **env);
void terminal_newtab(gpointer data, guint action, GtkWidget *item);


static GtkItemFactoryEntry menu_items[] =
{
	{ N_("/_File"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_File/_New Tab"), NEW_TAB_ACCEL, terminal_newtab, 1, "<StockItem>", GTK_STOCK_ADD },
	{ N_("/_File/_Close Tab"), CLOSE_TAB_ACCEL, NULL, 1, "<StockItem>", GTK_STOCK_CLOSE },
	{ N_("/_File/sep1"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_File/_Quit"), QUIT_ACCEL, gtk_main_quit, 0, "<StockItem>", GTK_STOCK_QUIT },
	{ N_("/_Edit"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_Edit/_Copy"), COPY_ACCEL, NULL, 0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/_Edit/_Paste"), PASTE_ACCEL, NULL, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/_Edit/sep1"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Edit/_Settings..."), NULL, NULL, 0, "<StockItem>", GTK_STOCK_EXECUTE },
	{ N_("/_Tabs"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_Tabs/_Rename Tab"), RENAME_TAB_ACCEL, NULL, 1, "<Item>" },
	{ N_("/_Tabs/_Previous Tab"), PREVIOUS_TAB_ACCEL, NULL, 1, "<Item>"},
	{ N_("/_Tabs/_Next Tab"), NEXT_TAB_ACCEL, NULL, 1, "<Item>"},
	{ N_("/_Help"), NULL, NULL, 0, "<Branch>" },
	{ N_("/_Help/_About"), NULL, NULL, 0, "<StockItem>", GTK_STOCK_DIALOG_INFO }
};  

static GtkItemFactoryEntry vte_menu_items[] =
{
	{ N_("/_New Tab"), NEW_TAB_ACCEL, terminal_newtab, 1, "<StockItem>", GTK_STOCK_ADD },
	{ N_("/sep1"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Copy"), COPY_ACCEL, NULL, 0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/_Paste"), PASTE_ACCEL, NULL, 0, "<StockItem>", GTK_STOCK_PASTE },
	{ N_("/sep2"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Settings..."), NULL, NULL, 0, "<StockItem>", GTK_STOCK_EXECUTE },
	{ N_("/sep3"), NULL, NULL, 0, "<Separator>" },
	{ N_("/_Close Tab"), CLOSE_TAB_ACCEL, NULL, 1, "<StockItem>", GTK_STOCK_CLOSE }
};

void terminal_newtab(gpointer data, guint action, GtkWidget *item)
{
	LXTerminal *terminal;

	if (!data)
		terminal = global_terminal;
	else
		terminal = (LXTerminal *)data;

	Term *term = terminal_new(terminal, "Default", g_get_current_dir(), NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->label);
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
	gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(term->parent->notebook),
								gtk_notebook_get_nth_page(GTK_NOTEBOOK(term->parent->notebook), term->index),
								vte_terminal_get_window_title(VTE_TERMINAL(vte)));

	/* setting window title */
	gtk_window_set_title(GTK_WINDOW(term->parent->mainw), vte_terminal_get_window_title(VTE_TERMINAL(vte)));
}

void terminal_childexit(VteTerminal *vte, Term *term)
{
	int i;

	if(gtk_notebook_get_n_pages(term->parent->notebook)==1) {
		/* destory */
		gtk_widget_destroy(term->label);
		gtk_widget_destroy(term->vte);
		gtk_widget_destroy(term->scrollbar);
		gtk_widget_destroy(term->box);

		/* release */
		g_ptr_array_free(term->parent->terms, TRUE);

		/* quit */
		gtk_main_quit();
	} else {
		/* remove page */
		gtk_notebook_remove_page(term->parent->notebook, term->index);

		/* decreasing index number after the page be removed */
		for (i=term->index;i<term->parent->terms->len;i++) {
			Term *t = g_ptr_array_index(term->parent->terms, i);
			t->index--;
		}

		/* release */
		g_free(term);

		/* if only one page, hide tab */
		if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(term->parent->notebook)) == 1)
			gtk_notebook_set_show_tabs(GTK_NOTEBOOK(term->parent->notebook), FALSE);
	}
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
	vte_terminal_set_font_from_string(term->vte, "monospace 11");
	vte_terminal_set_color_background(term->vte, &black);

	/* create label for tab */
	term->label = gtk_label_new(label);

	/* setting scrollbar */
	gtk_range_set_adjustment(GTK_RANGE(term->scrollbar), VTE_TERMINAL(term->vte)->adjustment);

	/* terminal fork */
	vte_terminal_fork_command(VTE_TERMINAL(term->vte), NULL, NULL, env, pwd, FALSE, TRUE, TRUE);

	/* signal handler */
	g_signal_connect(term->vte, "child-exited", G_CALLBACK(terminal_childexit), term);
	g_signal_connect(term->vte, "window-title-changed", G_CALLBACK(terminal_title_changed), term);
	//g_signal_connect(term->vte, "button-press-event", G_CALLBACK(terminal_vte_button_press), (gpointer)data);
	
	gtk_widget_show_all(term->box);

	return term;
}

Term *terminal_init(LXTerminal *terminal)
{
	Term *term;

	term = terminal_new(terminal, NULL, g_get_current_dir(), NULL);

	return term;
}

Menu *menubar_init()
{
	Menu *menubar;

	menubar = g_new0(Menu, 1);

	menubar->item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", NULL);
    gtk_item_factory_create_items(menubar->item_factory, sizeof(menu_items) / sizeof(menu_items[0]), menu_items, NULL);
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

	gtk_accel_group_lock(terminal->menubar->accel_group);
	gtk_window_add_accel_group(GTK_WINDOW(terminal->mainw), terminal->menubar->accel_group);
}

LXTerminal *lxterminal_init(gint argc, gchar **argv)
{
	LXTerminal *terminal;
	Term *term;

	terminal = g_new0(LXTerminal, 1);
	terminal->terms = g_ptr_array_new();

	/* create window */
	terminal->mainw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(terminal->mainw), "LXTerminal");
    g_signal_connect(terminal->mainw, "delete_event", gtk_main_quit, NULL);

	/* create box for putting menubar and notebook */
	terminal->box = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(terminal->mainw), terminal->box);

	/* create menuar */
	terminal->menubar = menubar_init();
	gtk_box_pack_start(GTK_BOX(terminal->box), terminal->menubar->menu, FALSE, TRUE, 0);

	/* create notebook */
	terminal->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(terminal->notebook), TRUE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);
    g_signal_connect(terminal->notebook, "switch-page", G_CALLBACK(terminal_switch_tab), terminal);
    gtk_box_pack_start(GTK_BOX(terminal->box), terminal->notebook, TRUE, TRUE, 0);

	/* create terminal */
	term = terminal_init(terminal);
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->label);
    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
    g_ptr_array_add(terminal->terms, term);

	/* initializing accelerator */
	lxterminal_accelerator_init(terminal);

	gtk_widget_show_all(terminal->mainw);

	return terminal;
}

int main(gint argc, gchar** argv)
{
	gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
	bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
	bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
	textdomain ( GETTEXT_PACKAGE );
#endif

	/* initializing LXTerminal */
	global_terminal = lxterminal_init(argc, argv);

	gtk_main();

    return 0;
}

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

#include "lxterminal.h"
#include "setting.h"
#include "preferences.h"

void lxterminal_preferences_general_constructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_general_destructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_general_save(Prefer *prefer, TabWidget *tab);

static TabGroup tabs[] = {
	{
		N_("General"),
		"preferences-gerneral.png",
		lxterminal_preferences_general_constructor,
		lxterminal_preferences_general_destructor,
		lxterminal_preferences_general_save
	}
};

void lxterminal_preferences_general_constructor(Prefer *prefer, TabWidget *tab)
{
	PreferGeneral *pg;

	/* create general data structure */
	pg = g_new0(PreferGeneral, 1);
	tab->childs = pg;

	pg->box = gtk_vbox_new(FALSE, 4);

	/* terminal font */
	pg->font_box = gtk_hbox_new(FALSE, 4);
	pg->font_label = gtk_label_new(_("Terminal Font:"));
	pg->font_button = gtk_font_button_new_with_font(prefer->terminal->setting->fontname);
	gtk_box_pack_start(GTK_BOX(pg->font_box), pg->font_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pg->font_box), pg->font_button, FALSE, FALSE, 0);
	/* adding to box */
	gtk_box_pack_start(GTK_BOX(pg->box), pg->font_box, FALSE, FALSE, 0);

	/* Select-by-word */
	pg->selchars_box = gtk_hbox_new(FALSE, 4);
	pg->selchars_label = gtk_label_new(_("Select-by-word characters:"));
	pg->selchars_entry = gtk_entry_new();
	gtk_entry_set_text(pg->selchars_entry, prefer->terminal->setting->selchars);
	gtk_box_pack_start(GTK_BOX(pg->selchars_box), pg->selchars_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pg->selchars_box), pg->selchars_entry, FALSE, FALSE, 0);
	/* adding to box */
	gtk_box_pack_start(GTK_BOX(pg->box), pg->selchars_box, FALSE, FALSE, 0);

	/* adding to page */
    gtk_container_add(GTK_CONTAINER(tab->page), pg->box);
}

void lxterminal_preferences_general_destructor(Prefer *prefer, TabWidget *tab)
{
	PreferGeneral *pg = tab->childs;

	g_free(pg);
}

void lxterminal_preferences_general_save(Prefer *prefer, TabWidget *tab)
{
	PreferGeneral *pg = tab->childs;

    g_free( prefer->terminal->setting->fontname );
	prefer->terminal->setting->fontname = g_strdup( gtk_font_button_get_font_name(pg->font_button) );
	prefer->terminal->setting->selchars = g_strdup( gtk_entry_get_text(pg->selchars_entry) );
}

TabWidget *lxterminal_preferences_page_new(TabGroup *tabgroup)
{
	TabWidget *tab;

	tab = g_new0(TabWidget, 1);

	/* create container without window */
	tab->page = gtk_event_box_new();
	GTK_WIDGET_SET_FLAGS(tab->page, GTK_NO_WINDOW);

	/* Sets the border width of the container. */
	gtk_container_set_border_width(GTK_CONTAINER(tab->page), 10);

	/* create container for label */
	tab->label_box = gtk_hbox_new(FALSE, 4);

	/* create icon */
	/* tab->icon = gtk_image_new_from_file(tabgroup->icon); */

	/* create label */
	tab->label = gtk_label_new(_(tabgroup->name));
	gtk_misc_set_alignment(GTK_MISC(tab->label), 0.0, 0.5);
	gtk_misc_set_padding(GTK_MISC(tab->label), 2, 2);

	/* add all of widgets to label box */
	/* gtk_box_pack_start(GTK_BOX(tab->label_box), tab->icon, FALSE, FALSE, 0); */
	gtk_box_pack_start(GTK_BOX(tab->label_box), tab->label, FALSE, FALSE, 0);

	gtk_widget_show_all(tab->label_box);

	return tab;
}

void lxterminal_preferences_page_init(Prefer *prefer)
{
	TabWidget *tab;
	int i;

	/* initializing all of tabs */
	for (i=0;i<G_N_ELEMENTS(tabs);i++) {
		tab = lxterminal_preferences_page_new(&tabs[i]);
		tabs[i].constructor(prefer, tab);

		/* add to Array */
		g_ptr_array_add(prefer->tab, tab);

		/* add to gtknotebook */
		gtk_notebook_append_page(GTK_NOTEBOOK(prefer->notebook), tab->page, tab->label_box);
	}
}

void lxterminal_preferences_free(Prefer *prefer)
{
	TabWidget *tab;
	int i;

	for (i=0;i<prefer->tab->len;i++) {
		tab = g_ptr_array_index(prefer->tab, i);
		tabs[i].destructor(prefer, tab);

		g_free(tab);
	}

	g_ptr_array_free(prefer->tab, TRUE);
	gtk_widget_destroy(prefer->dialog);
	g_free(prefer);
}

void lxterminal_preferences_on_response(GtkDialog* dlg, gint response, Prefer *prefer)
{
	gint i;
	TabWidget *tab;

    if(G_LIKELY(response == GTK_RESPONSE_OK)) {
		for (i=0;i<prefer->tab->len;i++) {
			tab = g_ptr_array_index(prefer->tab, i);
			tabs[i].save(prefer, tab);
		}

		/* saving perferences */
		setting_save(prefer->terminal->setting);

		/* update NOW! */
        /* update all terminals in all windows */
        g_ptr_array_foreach( prefer->terminal->parent->windows,
                             terminal_setting_update,
                             prefer->terminal->setting);
    }

	gtk_widget_destroy(dlg);
}

void lxterminal_preferences_dialog(LXTerminal *terminal, guint action, GtkWidget *item)
{
	Prefer *prefer;
	GtkWidget *pdialog;
	GtkWidget *tab;

	prefer = g_new0(Prefer, 1);
	prefer->terminal = terminal;
	prefer->tab = g_ptr_array_new();

	/* create window */
    prefer->dialog = gtk_dialog_new_with_buttons(_("Preferences"),
                                       NULL,
                                       GTK_DIALOG_NO_SEPARATOR,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                       NULL );
    gtk_dialog_set_alternative_button_order( prefer->dialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1 );
    gtk_dialog_set_default_response(GTK_WINDOW(prefer->dialog), GTK_RESPONSE_OK);
    gtk_window_set_position(GTK_WINDOW(prefer->dialog), GTK_WIN_POS_CENTER);

    /* g_signal */
    g_signal_connect(prefer->dialog, "response", G_CALLBACK(lxterminal_preferences_on_response), prefer);
    g_object_weak_ref(prefer->dialog, lxterminal_preferences_free, prefer);

	/* create notebook */
	prefer->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(prefer->notebook), TRUE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(prefer->notebook), TRUE);
	gtk_box_pack_start(GTK_BOX(((GtkDialog *)prefer->dialog)->vbox), prefer->notebook, FALSE, FALSE, 0);

	/* initializing pages */
	lxterminal_preferences_page_init(prefer);

	gtk_widget_show_all(prefer->dialog);
}

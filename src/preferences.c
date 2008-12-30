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

#if (GTK_MINOR_VERSION < 12)
gchar *gdk_color_to_string(const GdkColor *color)
{
    return g_strdup_printf("#%04x%04x%04x", color->red, color->green, color->blue);
}
#endif

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

	pg->box = gtk_table_new(6,4, FALSE);
	gtk_table_set_row_spacings(pg->box, 3);
	gtk_table_set_col_spacings(pg->box, 5);

	/* terminal font */
	pg->font_label = gtk_label_new(_("Terminal Font:"));
	gtk_misc_set_alignment(pg->font_label, 1, 0.5);
	pg->font_button = gtk_font_button_new_with_font(prefer->terminal->setting->fontname);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->font_label, 0,2, 0,1);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->font_button, 2,4, 0,1);

	/* Select-by-word */
	pg->selchars_label = gtk_label_new(_("Select-by-word characters:"));
	gtk_misc_set_alignment(pg->selchars_label, 1, 0.5);
	pg->selchars_entry = gtk_entry_new();
	gtk_entry_set_text((GtkEntry *)pg->selchars_entry, prefer->terminal->setting->selchars);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->selchars_label, 0,2, 1,2);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->selchars_entry, 2,4, 1,2);

	/* Background color */
	pg->bgcolor_label = gtk_label_new(_("Background:"));
	gtk_misc_set_alignment(pg->bgcolor_label, 1, 0.5);
	pg->bgcolor_entry = gtk_color_button_new_with_color(&prefer->terminal->background);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->bgcolor_label, 0,2, 3,4);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->bgcolor_entry, 2,4, 3,4);

	/* Foreground color */
	pg->fgcolor_label = gtk_label_new(_("Foreground:"));
	gtk_misc_set_alignment(pg->fgcolor_label, 1, 0.5);
	pg->fgcolor_entry = gtk_color_button_new_with_color(&prefer->terminal->foreground);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->fgcolor_label, 0,2, 4,5);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->fgcolor_entry, 2,4, 4,5);

	/* Scrollback buffer */
	pg->scrollback_label = gtk_label_new(_("Scrollback lines:"));
	gtk_misc_set_alignment(pg->scrollback_label, 1, 0.5);
	pg->scrollback_entry = gtk_spin_button_new_with_range(100, 100000, 10);
	gtk_spin_button_set_value((GtkSpinButton *)pg->scrollback_entry, (gdouble)prefer->terminal->setting->scrollback);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->scrollback_label, 0,2, 5,6);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->scrollback_entry, 2,4, 5,6);

	/* tab-panel position */
	pg->tabpos_label = gtk_label_new(_("Tab panel position:"));
	gtk_misc_set_alignment(pg->tabpos_label, 1, 0.5);
	pg->tabpos_combobox = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX (pg->tabpos_combobox), _("Top"));
	gtk_combo_box_append_text(GTK_COMBO_BOX (pg->tabpos_combobox), _("Bottom"));
	gtk_combo_box_append_text(GTK_COMBO_BOX (pg->tabpos_combobox), _("Left"));
	gtk_combo_box_append_text(GTK_COMBO_BOX (pg->tabpos_combobox), _("Right"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (pg->tabpos_combobox), prefer->terminal->tabpos);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->tabpos_label, 0,2, 6,7);
	gtk_table_attach_defaults(GTK_TABLE(pg->box), pg->tabpos_combobox, 2,4, 6,7);

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
	g_free( prefer->terminal->setting->selchars );
	prefer->terminal->setting->fontname = g_strdup( gtk_font_button_get_font_name((GtkFontButton *)pg->font_button) );
	prefer->terminal->setting->selchars = g_strdup( gtk_entry_get_text((GtkEntry *)pg->selchars_entry) );
	prefer->terminal->setting->scrollback = (glong)gtk_spin_button_get_value_as_int((GtkSpinButton *)pg->scrollback_entry);

	/* Tab position */
	g_free( prefer->terminal->setting->tabpos );
	prefer->terminal->tabpos = gtk_combo_box_get_active((GtkComboBox *)pg->tabpos_combobox);
	switch(prefer->terminal->tabpos) {
		case 0:
			prefer->terminal->setting->tabpos = g_strdup("top");
			break;
		case 1:
			prefer->terminal->setting->tabpos = g_strdup("bottom");
			break;
		case 2:
			prefer->terminal->setting->tabpos = g_strdup("left");
			break;
		case 3:
			prefer->terminal->setting->tabpos = g_strdup("right");
			break;
	}

	/* background and foreground */
	gtk_color_button_get_color( GTK_COLOR_BUTTON(pg->bgcolor_entry), &prefer->terminal->background);
	prefer->terminal->setting->bgcolor = g_strdup( gdk_color_to_string(&prefer->terminal->background) );
	gtk_color_button_get_color(GTK_COLOR_BUTTON(pg->fgcolor_entry), &prefer->terminal->foreground);
	prefer->terminal->setting->fgcolor = g_strdup( gdk_color_to_string(&prefer->terminal->foreground) );
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
		g_ptr_array_foreach(prefer->terminal->parent->windows,
					terminal_setting_update,
					prefer->terminal->setting);
	}

	gtk_widget_destroy((GtkWidget *)dlg);
}

void lxterminal_preferences_dialog(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;
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
	gtk_dialog_set_alternative_button_order((GtkDialog *)prefer->dialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_default_response((GtkDialog *)prefer->dialog, GTK_RESPONSE_OK);
	gtk_window_set_position(GTK_WINDOW(prefer->dialog), GTK_WIN_POS_CENTER);

	/* g_signal */
	g_signal_connect(prefer->dialog, "response", G_CALLBACK(lxterminal_preferences_on_response), prefer);
	g_object_weak_ref((GObject *)prefer->dialog, lxterminal_preferences_free, prefer);

	/* create notebook */
	prefer->notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(prefer->notebook), TRUE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(prefer->notebook), TRUE);
	gtk_box_pack_start(GTK_BOX(((GtkDialog *)prefer->dialog)->vbox), prefer->notebook, FALSE, FALSE, 0);

	/* initializing pages */
	lxterminal_preferences_page_init(prefer);

	gtk_widget_show_all(prefer->dialog);
}

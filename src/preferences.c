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

void lxterminal_preferences_style_constructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_style_destructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_style_save(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_display_constructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_display_destructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_display_save(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_misc_constructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_misc_destructor(Prefer *prefer, TabWidget *tab);
void lxterminal_preferences_misc_save(Prefer *prefer, TabWidget *tab);

static TabGroup tabs[] = {
	{
		N_("Style"),
		"preferences-style.png",
		lxterminal_preferences_style_constructor,
		lxterminal_preferences_style_destructor,
		lxterminal_preferences_style_save
	},
	{
		N_("Display"),
		"preferences-display.png",
		lxterminal_preferences_display_constructor,
		lxterminal_preferences_display_destructor,
		lxterminal_preferences_display_save
	},
	{
		N_("Misc"),
		"preferences-misc.png",
		lxterminal_preferences_misc_constructor,
		lxterminal_preferences_misc_destructor,
		lxterminal_preferences_misc_save
	}
};

void lxterminal_preferences_style_constructor(Prefer *prefer, TabWidget *tab)
{
	PreferStyle *ps;

	/* create general data structure */
	ps = g_new0(PreferStyle, 1);
	tab->childs = ps;

	ps->box = gtk_table_new(4, 4, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(ps->box), 3);
	gtk_table_set_col_spacings(GTK_TABLE(ps->box), 5);

	/* terminal font */
	ps->font_label = gtk_label_new(_("Terminal Font:"));
	gtk_misc_set_alignment(GTK_MISC(ps->font_label), 1, 0.5);
	ps->font_button = gtk_font_button_new_with_font(prefer->terminal->setting->fontname);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->font_label, 0,2, 0,1);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->font_button, 2,4, 0,1);

	/* Background color */
	ps->bgcolor_label = gtk_label_new(_("Background:"));
	gtk_misc_set_alignment(GTK_MISC(ps->bgcolor_label), 1, 0.5);
	ps->bgcolor_entry = gtk_color_button_new_with_color(&prefer->terminal->background);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(ps->bgcolor_entry), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(ps->bgcolor_entry), prefer->terminal->setting->bgalpha);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->bgcolor_label, 0,2, 1,2);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->bgcolor_entry, 2,4, 1,2);

	/* Foreground color */
	ps->fgcolor_label = gtk_label_new(_("Foreground:"));
	gtk_misc_set_alignment(GTK_MISC(ps->fgcolor_label), 1, 0.5);
	ps->fgcolor_entry = gtk_color_button_new_with_color(&prefer->terminal->foreground);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->fgcolor_label, 0,2, 2,3);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->fgcolor_entry, 2,4, 2,3);

	/* Cursor Blinks */
	ps->cursorblinks_label = gtk_label_new(_("Cursor Blinks:"));
	gtk_misc_set_alignment(GTK_MISC(ps->cursorblinks_label), 1, 0.5);
	ps->cursorblinks_checkbox = gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ps->cursorblinks_checkbox), prefer->terminal->setting->cursorblinks);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->cursorblinks_label, 0,2,3,4);
	gtk_table_attach_defaults(GTK_TABLE(ps->box), ps->cursorblinks_checkbox, 2,4,3,4);

	/* adding to page */
	gtk_container_add(GTK_CONTAINER(tab->page), ps->box);
}

void lxterminal_preferences_display_constructor(Prefer *prefer, TabWidget *tab)
{
	PreferDisplay *pd;

	/* create general data structure */
	pd = g_new0(PreferDisplay, 1);
	tab->childs = pd;

	pd->box = gtk_table_new(4,4, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(pd->box), 3);
	gtk_table_set_col_spacings(GTK_TABLE(pd->box), 5);

	/* tab-panel position */
	pd->tabpos_label = gtk_label_new(_("Tab panel position:"));
	gtk_misc_set_alignment(GTK_MISC(pd->tabpos_label), 1, 0.5);
	pd->tabpos_combobox = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX (pd->tabpos_combobox), _("Top"));
	gtk_combo_box_append_text(GTK_COMBO_BOX (pd->tabpos_combobox), _("Bottom"));
	gtk_combo_box_append_text(GTK_COMBO_BOX (pd->tabpos_combobox), _("Left"));
	gtk_combo_box_append_text(GTK_COMBO_BOX (pd->tabpos_combobox), _("Right"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (pd->tabpos_combobox), prefer->terminal->tabpos);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->tabpos_label, 0,2, 0,1);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->tabpos_combobox, 2,4, 0,1);

	/* Scrollback buffer */
	pd->scrollback_label = gtk_label_new(_("Scrollback lines:"));
	gtk_misc_set_alignment(GTK_MISC(pd->scrollback_label), 1, 0.5);
	pd->scrollback_entry = gtk_spin_button_new_with_range(100, 100000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(pd->scrollback_entry), (gdouble)prefer->terminal->setting->scrollback);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->scrollback_label, 0,2, 1,2);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->scrollback_entry, 2,4, 1,2);

	/* Hide scroll bar */
	pd->hidescrollbar_label = gtk_label_new(_("Hide scroll bar:"));
	gtk_misc_set_alignment(GTK_MISC(pd->hidescrollbar_label), 1, 0.5);
	pd->hidescrollbar_checkbox = gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->hidescrollbar_checkbox), prefer->terminal->setting->hidescrollbar);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->hidescrollbar_label, 0,2,2,3);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->hidescrollbar_checkbox, 2,4,2,3);

	/* Hide menu bar */
	pd->hidemenubar_label = gtk_label_new(_("Hide menu bar:"));
	gtk_misc_set_alignment(GTK_MISC(pd->hidemenubar_label), 1, 0.5);
	pd->hidemenubar_checkbox = gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->hidemenubar_checkbox), prefer->terminal->setting->hidemenubar);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->hidemenubar_label, 0,2,3,4);
	gtk_table_attach_defaults(GTK_TABLE(pd->box), pd->hidemenubar_checkbox, 2,4,3,4);

	/* adding to page */
	gtk_container_add(GTK_CONTAINER(tab->page), pd->box);
}

void lxterminal_preferences_misc_constructor(Prefer *prefer, TabWidget *tab)
{
	PreferMisc *pm;

	/* create general data structure */
	pm = g_new0(PreferMisc, 1);
	tab->childs = pm;

	pm->box = gtk_table_new(2,4, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(pm->box), 3);
	gtk_table_set_col_spacings(GTK_TABLE(pm->box), 5);

	/* Select-by-word */
	pm->selchars_label = gtk_label_new(_("Select-by-word characters:"));
	gtk_misc_set_alignment(GTK_MISC(pm->selchars_label), 1, 0.5);
	pm->selchars_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(pm->selchars_entry), prefer->terminal->setting->selchars);
	gtk_table_attach_defaults(GTK_TABLE(pm->box), pm->selchars_label, 0,2, 0,1);
	gtk_table_attach_defaults(GTK_TABLE(pm->box), pm->selchars_entry, 2,4, 0,1);

	/* Disable F10 for menu */
	pm->disablef10_label = gtk_label_new(_("Disable F10 shortcut for menu:"));
	gtk_misc_set_alignment(GTK_MISC(pm->disablef10_label), 1, 0.5);
	pm->disablef10_checkbox = gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pm->disablef10_checkbox), prefer->terminal->setting->disablef10);
	gtk_table_attach_defaults(GTK_TABLE(pm->box), pm->disablef10_label, 0,2, 1,2);
	gtk_table_attach_defaults(GTK_TABLE(pm->box), pm->disablef10_checkbox, 2,4, 1,2);

	/* adding to page */
	gtk_container_add(GTK_CONTAINER(tab->page), pm->box);
}

void lxterminal_preferences_style_destructor(Prefer *prefer, TabWidget *tab)
{
	PreferStyle *ps = tab->childs;

	g_free(ps);
}

void lxterminal_preferences_display_destructor(Prefer *prefer, TabWidget *tab)
{
	PreferDisplay *pd = tab->childs;

	g_free(pd);
}

void lxterminal_preferences_misc_destructor(Prefer *prefer, TabWidget *tab)
{
	PreferMisc *pm = tab->childs;

	g_free(pm);
}

void lxterminal_preferences_style_save(Prefer *prefer, TabWidget *tab)
{
	PreferStyle *ps = tab->childs;

	g_free( prefer->terminal->setting->fontname );
	g_free( prefer->terminal->setting->selchars );
	prefer->terminal->setting->fontname = g_strdup( gtk_font_button_get_font_name((GtkFontButton *)ps->font_button) );
	prefer->terminal->setting->bgalpha = (guint16)gtk_color_button_get_alpha(GTK_COLOR_BUTTON(ps->bgcolor_entry));
	prefer->terminal->setting->cursorblinks = (gboolean)gtk_toggle_button_get_active((GtkToggleButton *)ps->cursorblinks_checkbox);

	/* background and foreground */
	gtk_color_button_get_color( GTK_COLOR_BUTTON(ps->bgcolor_entry), &prefer->terminal->background);
	prefer->terminal->setting->bgcolor = g_strdup( gdk_color_to_string(&prefer->terminal->background) );
	gtk_color_button_get_color(GTK_COLOR_BUTTON(ps->fgcolor_entry), &prefer->terminal->foreground);
	prefer->terminal->setting->fgcolor = g_strdup( gdk_color_to_string(&prefer->terminal->foreground) );
}

void lxterminal_preferences_display_save(Prefer *prefer, TabWidget *tab)
{
	PreferDisplay *pd = tab->childs;

	prefer->terminal->setting->scrollback = (glong)gtk_spin_button_get_value_as_int((GtkSpinButton *)pd->scrollback_entry);
	prefer->terminal->setting->hidemenubar = (gboolean)gtk_toggle_button_get_active((GtkToggleButton *)pd->hidemenubar_checkbox);
	prefer->terminal->setting->hidescrollbar = (gboolean)gtk_toggle_button_get_active((GtkToggleButton *)pd->hidescrollbar_checkbox);

	/* Tab position */
	g_free( prefer->terminal->setting->tabpos );
	prefer->terminal->tabpos = gtk_combo_box_get_active((GtkComboBox *)pd->tabpos_combobox);
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
}

void lxterminal_preferences_misc_save(Prefer *prefer, TabWidget *tab)
{
	PreferMisc *pm = tab->childs;

	prefer->terminal->setting->selchars = g_strdup( gtk_entry_get_text((GtkEntry *)pm->selchars_entry) );
	prefer->terminal->setting->disablef10 = (gboolean)gtk_toggle_button_get_active((GtkToggleButton *)pm->disablef10_checkbox);
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

void lxterminal_preferences_free(gpointer prefer_p, GObject * where_the_object_was)
{
	Prefer * prefer = (Prefer *) prefer_p;
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
					(GFunc) terminal_setting_update,
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

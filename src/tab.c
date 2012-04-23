/**
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
#include "tab.h"

/* Set the "clicked" signal handler on a tab. */
void lxterminal_tab_label_close_button_clicked(GCallback func, Term * term)
{
    g_signal_connect(G_OBJECT(term->label->close_btn), "clicked", func, term);
}

/* Set the label on a tab. */
void lxterminal_tab_label_set_text(LXTab * tab, const gchar * str)
{
    gtk_label_set_text(GTK_LABEL(tab->label), str);
}

/* Set the tooltip on a tab. */
void lxterminal_tab_label_set_tooltip_text(LXTab * tab, const gchar * str)
{
    gtk_widget_set_tooltip_text(tab->label, str);
}

/* Create a new tab. */
LXTab * lxterminal_tab_label_new(const gchar * str)
{
    /* Allocate LXTab structure. */
    LXTab * tab = g_slice_new0(LXTab);

    /* Create a horizontal box as the toplevel. */
    tab->main = gtk_hbox_new(FALSE, 4);

    /* Create the Close button. */
    tab->close_btn = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(tab->close_btn), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(tab->close_btn), FALSE);
    gtk_container_add(GTK_CONTAINER(tab->close_btn), gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));

    /* Make the button as small as possible. */
    GtkRcStyle * rcstyle = gtk_rc_style_new();
    rcstyle->xthickness = rcstyle->ythickness = 0;
    gtk_widget_modify_style(tab->close_btn, rcstyle);
    gtk_rc_style_unref(rcstyle),

    /* Create the label. */
    tab->label = gtk_label_new(str);
    gtk_widget_set_size_request(GTK_WIDGET(tab->label), 100, -1);
    gtk_label_set_ellipsize(GTK_LABEL(tab->label), PANGO_ELLIPSIZE_END);
    gtk_misc_set_alignment(GTK_MISC(tab->label), 0.0, 0.5);
    gtk_misc_set_padding(GTK_MISC(tab->label), 0, 0);

    /* Pack everything, show the widget and return. */
    gtk_box_pack_start(GTK_BOX(tab->main), tab->label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab->main), tab->close_btn, FALSE, FALSE, 0);
    gtk_widget_show_all(tab->main);
    return tab;
}

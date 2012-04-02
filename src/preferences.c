/**
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
 *      Copyright (c) 2010 LxDE Developers, see the file AUTHORS for details.
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

static void preferences_dialog_response_event(GtkWidget * dialog, gint response, LXTerminal * terminal);
static void preferences_dialog_font_set_event(GtkFontButton * widget, LXTerminal * terminal);
static void preferences_dialog_background_color_set_event(GtkColorButton * widget, LXTerminal * terminal);
static void preferences_dialog_foreground_color_set_event(GtkColorButton * widget, LXTerminal * terminal);
static void preferences_dialog_allow_bold_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);
static void preferences_dialog_cursor_shape_changed_event(GtkComboBox * widget, LXTerminal * terminal);
static void preferences_dialog_audible_bell_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);
static void preferences_dialog_tab_position_changed_event(GtkComboBox * widget, LXTerminal * terminal);
static void preferences_dialog_scrollback_value_changed_event(GtkSpinButton * widget, LXTerminal * terminal);
static void preferences_dialog_hide_scroll_bar_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);
static void preferences_dialog_hide_menu_bar_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);
static void preferences_dialog_hide_close_button_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);
static gboolean preferences_dialog_selection_focus_out_event(GtkWidget * widget, GdkEventFocus * event, LXTerminal * terminal);
static void preferences_dialog_disable_f10_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);
static void preferences_dialog_disable_alt_toggled_event(GtkToggleButton * widget, LXTerminal * terminal);

/* Handler for "response" signal on preferences dialog. */
static void preferences_dialog_response_event(GtkWidget * dialog, gint response, LXTerminal * terminal)
{
    setting_save(terminal->setting);

    /* Dismiss dialog. */
    gtk_widget_destroy(dialog);
}

/* Handler for "font-set" signal on Terminal Font font button. */
static void preferences_dialog_font_set_event(GtkFontButton * widget, LXTerminal * terminal)
{
    g_free(terminal->setting->font_name);
    terminal->setting->font_name = g_strdup(gtk_font_button_get_font_name(widget));
    terminal->setting->geometry_change = TRUE;		/* Force the terminals to resize */
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "color-set" signal on Background Color color button. */
static void preferences_dialog_background_color_set_event(GtkColorButton * widget, LXTerminal * terminal)
{
    gtk_color_button_get_color(widget, &terminal->setting->background_color);
    terminal->setting->background_alpha = gtk_color_button_get_alpha(widget);

    if (terminal->setting->background_alpha == 0) {
        terminal->setting->background_alpha = 1;
    }

    terminal_settings_apply_to_all(terminal);
}

/* Handler for "color-set" signal on Foreground Color color button. */
static void preferences_dialog_foreground_color_set_event(GtkColorButton * widget, LXTerminal * terminal)
{
    gtk_color_button_get_color(widget, &terminal->setting->foreground_color);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "toggled" signal on Allow Bold toggle button.
 * Use the complement so the default is FALSE. */
static void preferences_dialog_allow_bold_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->allow_bold = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "changed" signal on Tab Position combo box. */
static void preferences_dialog_cursor_shape_changed_event(GtkComboBox * widget, LXTerminal * terminal)
{
    /* Convert the index into a string, which is what we put in the configuration file. */
    char * p = NULL;
    switch (gtk_combo_box_get_active(widget))
    {
        case 0:		p = "block";		break;
        case 1:		p = "ibeam";		break;
        case 2:		p = "underline";	break;
    }
    if (p != NULL)
    {
        g_free(terminal->setting->cursor_shape);
        terminal->setting->cursor_shape = g_strdup(p);
    }
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "toggled" signal on Audible Bell radio button. */
static void preferences_dialog_audible_bell_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->audible_bell = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "changed" signal on Tab Position combo box. */
static void preferences_dialog_tab_position_changed_event(GtkComboBox * widget, LXTerminal * terminal)
{
    /* Convert the index into a string, which is what we put in the configuration file. */
    char * p = NULL;
    switch (gtk_combo_box_get_active(widget))
    {
        case 0:		p = "top";		break;
        case 1:		p = "bottom";		break;
        case 2:		p = "left";		break;
        case 3:		p = "right";		break;
    }
    if (p != NULL)
    {
        g_free(terminal->setting->tab_position);
        terminal->setting->tab_position = g_strdup(p);
    }
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "value-changed" signal on Scrollback spin button. */
static void preferences_dialog_scrollback_value_changed_event(GtkSpinButton * widget, LXTerminal * terminal)
{
    terminal->setting->scrollback = gtk_spin_button_get_value_as_int(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "toggled" signal on Hide Scroll Bar toggle button. */
static void preferences_dialog_hide_scroll_bar_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->hide_scroll_bar = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "toggled" signal on Hide Menu Bar toggle button. */
static void preferences_dialog_hide_menu_bar_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->hide_menu_bar = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "toggled" signal on Hide Close Button toggle button. */
static void preferences_dialog_hide_close_button_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->hide_close_button = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "focus-out-event" on Selection Characters entry. */
static gboolean preferences_dialog_selection_focus_out_event(GtkWidget * widget, GdkEventFocus * event, LXTerminal * terminal)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(terminal->setting->word_selection_characters);
    terminal->setting->word_selection_characters = g_strdup(new_text);
    terminal_settings_apply_to_all(terminal);
    return FALSE;
}

/* Handler for "toggled" signal on Disable F10 toggle button. */
static void preferences_dialog_disable_f10_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->disable_f10 = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Handler for "toggled" signal on Disable Alt toggle button. */
static void preferences_dialog_disable_alt_toggled_event(GtkToggleButton * widget, LXTerminal * terminal)
{
    terminal->setting->disable_alt = gtk_toggle_button_get_active(widget);
    terminal_settings_apply_to_all(terminal);
}

/* Convert the user preference on tab position, expressed as a string, to the internal representation.
 * These have to match the order in the .glade file. */
gint terminal_tab_get_position_id(gchar * position)
{
    if (strcmp(position, "bottom") == 0)
        return 1;
    else if (strcmp(position, "left") == 0)
        return 2;
    else if (strcmp(position, "right") == 0)
        return 3;
    else
        return 0;
}

/* These have to match the order in the .glade file. */
gint terminal_cursor_get_shape_id(gchar * shape)
{
    if (strcmp(shape, "ibeam") == 0)
        return 1;
    else if (strcmp(shape, "underline") == 0)
        return 2;
    else
        return 0;
}

/* Initialize and display the preferences dialog. */
void terminal_preferences_dialog(GtkAction * action, LXTerminal * terminal)
{
    Setting * setting = terminal->setting;

    GtkBuilder * builder = gtk_builder_new();
    if ( ! gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/lxterminal/lxterminal-preferences.ui", NULL))
    {
        g_object_unref(builder);
        return;
    }

    GtkWidget * dialog = GTK_WIDGET(gtk_builder_get_object(builder, "lxterminal_preferences"));
    gtk_window_set_title(GTK_WINDOW(dialog), _("LXTerminal"));
    gtk_window_set_icon_from_file(GTK_WINDOW(dialog), PACKAGE_DATA_DIR "/pixmaps/lxterminal.png", NULL);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(preferences_dialog_response_event), terminal);

    GtkWidget * w = GTK_WIDGET(gtk_builder_get_object(builder, "terminal_font"));
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(w), setting->font_name);
    g_signal_connect(G_OBJECT(w), "font-set", G_CALLBACK(preferences_dialog_font_set_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "background_color"));
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->background_color);
    gtk_color_button_set_alpha(GTK_COLOR_BUTTON(w), setting->background_alpha);
    g_signal_connect(G_OBJECT(w), "color-set", G_CALLBACK(preferences_dialog_background_color_set_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "foreground_color"));
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->foreground_color);
    g_signal_connect(G_OBJECT(w), "color-set", G_CALLBACK(preferences_dialog_foreground_color_set_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "allow_bold"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->allow_bold);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_allow_bold_toggled_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "cursor_shape"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), terminal_cursor_get_shape_id(setting->cursor_shape));
    g_signal_connect(G_OBJECT(w), "changed", G_CALLBACK(preferences_dialog_cursor_shape_changed_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "audible_bell"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->audible_bell);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_audible_bell_toggled_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "tab_position"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), terminal_tab_get_position_id(setting->tab_position));
    g_signal_connect(G_OBJECT(w), "changed", G_CALLBACK(preferences_dialog_tab_position_changed_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "scrollback_lines"));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), setting->scrollback);
    g_signal_connect(G_OBJECT(w), "value-changed", G_CALLBACK(preferences_dialog_scrollback_value_changed_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_scroll_bar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_scroll_bar);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_hide_scroll_bar_toggled_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_menu_bar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_menu_bar);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_hide_menu_bar_toggled_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_close_button"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_close_button);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_hide_close_button_toggled_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "select_by_word"));
    gtk_entry_set_text(GTK_ENTRY(w), setting->word_selection_characters);
    g_signal_connect(G_OBJECT(w), "focus-out-event", G_CALLBACK(preferences_dialog_selection_focus_out_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_f10"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_f10);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_disable_f10_toggled_event), terminal);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_alt"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_alt);
    g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(preferences_dialog_disable_alt_toggled_event), terminal);

    gtk_widget_show_all(dialog);
    g_object_unref(builder);
}

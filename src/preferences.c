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

/* Handler for "font-set" signal on Terminal Font font button. */
static void preferences_dialog_font_set_event(GtkFontButton * widget, Setting * setting)
{
    g_free(setting->font_name);
    setting->font_name = g_strdup(gtk_font_button_get_font_name(widget));
    setting->geometry_change = TRUE;        /* Force the terminals to resize */
}

/* Handler for "color-set" signal on Background Color color button. */
static void preferences_dialog_background_color_set_event(GtkColorButton * widget, Setting * setting)
{
    gtk_color_button_get_color(widget, &setting->background_color);
    setting->background_alpha = gtk_color_button_get_alpha(widget);

    if (setting->background_alpha == 0)
    {
        setting->background_alpha = 1;
    }
}

/* Handler for "color-set" signal on Foreground Color color button. */
static void preferences_dialog_foreground_color_set_event(GtkColorButton * widget, Setting * setting)
{
    gtk_color_button_get_color(widget, &setting->foreground_color);
}

/* Handler for "toggled" signal on Allow Bold toggle button.
 * Use the complement so the default is FALSE. */
static void preferences_dialog_allow_bold_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->disallow_bold = ! gtk_toggle_button_get_active(widget);
}

/* Handler for "changed" signal on Tab Position combo box. */
static void preferences_dialog_tab_position_changed_event(GtkComboBox * widget, Setting * setting)
{
    /* Convert the index into a string, which is what we put in the configuration file. */
    const char * p = NULL;
    switch (gtk_combo_box_get_active(widget))
    {
        case 0:
            p = "top";
            break;
        case 1:
            p = "bottom";
            break;
        case 2:
            p = "left";
            break;
        case 3:
            p = "right";
            break;
    }
    if (p != NULL)
    {
        g_free(setting->tab_position);
        setting->tab_position = g_strdup(p);
    }
    //terminal_settings_apply_to_all(terminal);
}

/* Handler for "value-changed" signal on Scrollback spin button. */
static void preferences_dialog_scrollback_value_changed_event(GtkSpinButton * widget, Setting * setting)
{
    setting->scrollback = gtk_spin_button_get_value_as_int(widget);
}

/* Convert the user preference on tab position, expressed as a string, to the internal representation.
 * These have to match the order in the .glade file. */
gint terminal_tab_get_position_id(gchar * position)
{
    if (strcmp(position, "bottom") == 0)
    {
        return 1;
    }
    else if (strcmp(position, "left") == 0)
    {
        return 2;
    }
    else if (strcmp(position, "right") == 0)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}

/* Generic "toggled" handler for GtkToggleButton events */
static void preferences_dialog_generic_toggled_event(GtkToggleButton * widget, gboolean * s)
{
    *s = gtk_toggle_button_get_active(widget);
}

/* Generic "focus-out-event" handler for GtkEntry events */
static gboolean preferences_dialog_generic_focus_out_event(GtkWidget * widget, GdkEventFocus * event, gchar * s)
{
    g_free(s);
    s = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    return FALSE;
}

/* Specific "focus-out-event" handler for shortcut GtkEntry events */
static gboolean preferences_dialog_shortcut_focus_out_event(GtkWidget * widget, GdkEventFocus * event, gchar * s)
{
    guint key = 0;
    GdkModifierType mods = 0;
    const gchar * cur = gtk_entry_get_text(GTK_ENTRY(widget));
    GList * sib;

    if(g_strcmp0(s, cur) == 0)
        /* Nothing changed. */
        return FALSE;

    /* Look for dupplicate accelerator. */
    for(sib = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(widget))); sib; sib = sib->next)
	if(GTK_IS_ENTRY(sib->data) && GTK_WIDGET(sib->data) != widget && !g_strcmp0(cur, gtk_entry_get_text(GTK_ENTRY(sib->data))))
	{
	    gtk_entry_set_text(GTK_ENTRY(widget), s);
	    return FALSE;
	}

    gtk_accelerator_parse(cur, &key, &mods);

    /* Make sure accelerator is valid. */
    if( ! (key == 0 && mods == 0) && gtk_accelerator_valid(key, mods))
    {
        g_free(s);
        s = g_strdup(cur);
    } else {
        gtk_entry_set_text(GTK_ENTRY(widget), s);
    }
    return FALSE;
}

/* Initialize and display the preferences dialog. */
void terminal_preferences_dialog(GtkAction * action, LXTerminal * terminal)
{
    Setting * setting = copy_setting(get_setting());

    GtkBuilder * builder = gtk_builder_new();
    if ( ! gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/lxterminal/lxterminal-preferences.ui", NULL))
    {
        g_object_unref(builder);
        return;
    }

    GtkDialog * dialog = GTK_DIALOG(gtk_builder_get_object(builder, "lxterminal_preferences"));
    gtk_window_set_title(GTK_WINDOW(dialog), _("LXTerminal"));
    gtk_window_set_icon_from_file(GTK_WINDOW(dialog), PACKAGE_DATA_DIR "/icons/hicolor/128x128/apps/lxterminal.png", NULL);

    GtkWidget * w = GTK_WIDGET(gtk_builder_get_object(builder, "terminal_font"));
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(w), setting->font_name);
    g_signal_connect(G_OBJECT(w), "font-set", G_CALLBACK(preferences_dialog_font_set_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "background_color"));
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->background_color);
    gtk_color_button_set_alpha(GTK_COLOR_BUTTON(w), setting->background_alpha);
    g_signal_connect(G_OBJECT(w), "color-set", 
        G_CALLBACK(preferences_dialog_background_color_set_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "foreground_color"));
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->foreground_color);
    g_signal_connect(G_OBJECT(w), "color-set", 
        G_CALLBACK(preferences_dialog_foreground_color_set_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "allow_bold"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), ! setting->disallow_bold);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_allow_bold_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "cursor_blink"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->cursor_blink);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->cursor_blink);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "cursor_style_block"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), ! setting->cursor_underline);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "cursor_style_underline"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->cursor_underline);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->cursor_underline);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "audible_bell"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->audible_bell);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->audible_bell);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "tab_position"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), terminal_tab_get_position_id(setting->tab_position));
    g_signal_connect(G_OBJECT(w), "changed", 
        G_CALLBACK(preferences_dialog_tab_position_changed_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "scrollback_lines"));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), setting->scrollback);
    g_signal_connect(G_OBJECT(w), "value-changed", 
        G_CALLBACK(preferences_dialog_scrollback_value_changed_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_scroll_bar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_scroll_bar);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->hide_scroll_bar);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_menu_bar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_menu_bar);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->hide_menu_bar);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_close_button"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_close_button);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->hide_close_button);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_pointer"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_pointer);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->hide_pointer);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "select_by_word"));
    gtk_entry_set_text(GTK_ENTRY(w), setting->word_selection_characters);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_generic_focus_out_event), setting->word_selection_characters);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_f10"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_f10);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->disable_f10);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_alt"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_alt);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->disable_alt);
    
    /* Shortcuts */
    w = GTK_WIDGET(gtk_builder_get_object(builder, NEW_WINDOW_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->new_window_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->new_window_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, NEW_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->new_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->new_tab_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, CLOSE_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->close_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->close_tab_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, CLOSE_WINDOW_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->close_window_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->close_window_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, COPY_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->copy_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->copy_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, PASTE_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->paste_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->paste_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, NAME_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->name_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->name_tab_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, PREVIOUS_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->previous_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->previous_tab_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, NEXT_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->next_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->next_tab_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, MOVE_TAB_LEFT_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->move_tab_left_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->move_tab_left_accel);

    w = GTK_WIDGET(gtk_builder_get_object(builder, MOVE_TAB_RIGHT_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->move_tab_right_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_shortcut_focus_out_event), setting->move_tab_right_accel);

    g_object_unref(builder);

    gtk_window_set_modal(GTK_WINDOW(GTK_DIALOG(dialog)), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(GTK_DIALOG(dialog)), 
        GTK_WINDOW(terminal->window));

    int result = gtk_dialog_run(dialog);
    /* Dismiss dialog. */
    gtk_widget_destroy(GTK_WIDGET(dialog));
    if (result == GTK_RESPONSE_OK)
    {
        set_setting(setting);
        save_setting();
        terminal_settings_apply_to_all(terminal);
    }
    else
    {
        free_setting(setting);
    }
}

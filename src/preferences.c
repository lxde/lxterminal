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

/* Handler for "toggled" signal on Cursor Blink toggle button. */
static void preferences_dialog_cursor_blink_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->cursor_blink = gtk_toggle_button_get_active(widget);
}

/* Handler for "toggled" signal on Cursor Underline radio button. */
static void preferences_dialog_cursor_underline_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->cursor_underline = gtk_toggle_button_get_active(widget);
}

/* Handler for "toggled" signal on Audible Bell radio button. */
static void preferences_dialog_audible_bell_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->audible_bell = gtk_toggle_button_get_active(widget);
}

/* Handler for "changed" signal on Tab Position combo box. */
static void preferences_dialog_tab_position_changed_event(GtkComboBox * widget, Setting * setting)
{
    /* Convert the index into a string, which is what we put in the configuration file. */
    char * p = NULL;
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

/* Handler for "toggled" signal on Hide Scroll Bar toggle button. */
static void preferences_dialog_hide_scroll_bar_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->hide_scroll_bar = gtk_toggle_button_get_active(widget);
}

/* Handler for "toggled" signal on Hide Menu Bar toggle button. */
static void preferences_dialog_hide_menu_bar_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->hide_menu_bar = gtk_toggle_button_get_active(widget);
}

/* Handler for "toggled" signal on Hide Close Button toggle button. */
static void preferences_dialog_hide_close_button_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->hide_close_button = gtk_toggle_button_get_active(widget);
}

/* Handler for "focus-out-event" on Selection Characters entry. */
static gboolean preferences_dialog_selection_focus_out_event(GtkWidget * widget, GdkEventFocus * event, 
    Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->word_selection_characters);
    setting->word_selection_characters = g_strdup(new_text);
    return FALSE;
}

/* Handler for "toggled" signal on Disable F10 toggle button. */
static void preferences_dialog_disable_f10_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->disable_f10 = gtk_toggle_button_get_active(widget);
}

/* Handler for "toggled" signal on Disable Alt toggle button. */
static void preferences_dialog_disable_alt_toggled_event(GtkToggleButton * widget, Setting * setting)
{
    setting->disable_alt = gtk_toggle_button_get_active(widget);
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

/* Handler for "focus-out-event" on new_window_accel entry. */
static gboolean preferences_dialog_new_window_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->new_window_accel);
    setting->new_window_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on new_tab_accel entry. */
static gboolean preferences_dialog_new_tab_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->new_tab_accel);
    setting->new_tab_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on close_tab_accel entry. */
static gboolean preferences_dialog_close_tab_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->close_tab_accel);
    setting->close_tab_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on quit_accel entry. */
static gboolean preferences_dialog_quit_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->quit_accel);
    setting->quit_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on copy_accel entry. */
static gboolean preferences_dialog_copy_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->copy_accel);
    setting->copy_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on paste_accel entry. */
static gboolean preferences_dialog_paste_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->paste_accel);
    setting->paste_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on name_tab_accel entry. */
static gboolean preferences_dialog_name_tab_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->name_tab_accel);
    setting->name_tab_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on previous_tab_accel entry. */
static gboolean preferences_dialog_previous_tab_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->previous_tab_accel);
    setting->previous_tab_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on next_tab_accel entry. */
static gboolean preferences_dialog_next_tab_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->next_tab_accel);
    setting->next_tab_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on move_tab_left_accel entry. */
static gboolean preferences_dialog_move_tab_left_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->move_tab_left_accel);
    setting->move_tab_left_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

/* Handler for "focus-out-event" on move_tab_right_accel entry. */
static gboolean preferences_dialog_move_tab_right_accel_focus_out_event(GtkWidget * widget, 
    GdkEventFocus * event, Setting * setting)
{
    const gchar * new_text = gtk_entry_get_text(GTK_ENTRY(widget));
    g_free(setting->move_tab_right_accel);
    setting->move_tab_right_accel = g_strdup(new_text);
    setting->accel_changed = TRUE;
    return FALSE;
}

void show_need_restart_message_dialog(LXTerminal * terminal)
{
    GtkWidget * dialog = gtk_message_dialog_new (GTK_WINDOW(terminal->window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_WARNING,
                                  GTK_BUTTONS_CLOSE,
                                  _("Accelerator changed need restart!"));
    gtk_window_set_icon_from_file(GTK_WINDOW(dialog), PACKAGE_DATA_DIR "/pixmaps/lxterminal.xpm", NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), _("LXTerminal"));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
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
    gtk_window_set_icon_from_file(GTK_WINDOW(dialog), PACKAGE_DATA_DIR "/pixmaps/lxterminal.xpm", NULL);

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
        G_CALLBACK(preferences_dialog_cursor_blink_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "cursor_style_block"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), ! setting->cursor_underline);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "cursor_style_underline"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->cursor_underline);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_cursor_underline_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "audible_bell"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->audible_bell);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_audible_bell_toggled_event), setting);

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
        G_CALLBACK(preferences_dialog_hide_scroll_bar_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_menu_bar"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_menu_bar);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_hide_menu_bar_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "hide_close_button"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->hide_close_button);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_hide_close_button_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "select_by_word"));
    gtk_entry_set_text(GTK_ENTRY(w), setting->word_selection_characters);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_selection_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_f10"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_f10);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_disable_f10_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_alt"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_alt);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_disable_alt_toggled_event), setting);
    
    /* Shortcuts */
    w = GTK_WIDGET(gtk_builder_get_object(builder, NEW_WINDOW_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->new_window_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_new_window_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, NEW_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->new_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_new_tab_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, CLOSE_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->close_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_close_tab_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, QUIT_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->quit_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_quit_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, COPY_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->copy_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_copy_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, PASTE_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->paste_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_paste_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, NAME_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->name_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_name_tab_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, PREVIOUS_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->previous_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_previous_tab_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, NEXT_TAB_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->next_tab_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_next_tab_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, MOVE_TAB_LEFT_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->move_tab_left_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_move_tab_left_accel_focus_out_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, MOVE_TAB_RIGHT_ACCEL));
    gtk_entry_set_text(GTK_ENTRY(w), setting->move_tab_right_accel);
    g_signal_connect(G_OBJECT(w), "focus-out-event", 
        G_CALLBACK(preferences_dialog_move_tab_right_accel_focus_out_event), setting);

    g_object_unref(builder);

    gtk_window_set_modal(GTK_WINDOW(GTK_DIALOG(dialog)), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(GTK_DIALOG(dialog)), 
        GTK_WINDOW(terminal->window));
    //gtk_widget_show_all(dialog);
    setting->accel_changed = FALSE;
    int result = gtk_dialog_run(dialog);
    if (result == GTK_RESPONSE_OK)
    {
        set_setting(setting);
        save_setting();
        terminal_settings_apply_to_all(terminal);
        if (setting->accel_changed)
        {
            printf("Accelerator changed need restart\n");
            show_need_restart_message_dialog(terminal);
        }
        printf("Saved result OK\n");
    }
    else
    {
        free_setting(setting);
    }
    /* Dismiss dialog. */
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

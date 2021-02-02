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
#include <vte/vte.h>
#include <gdk/gdkkeysyms.h>

#include "lxterminal.h"
#include "setting.h"
#include "preferences.h"

GtkBuilder *builder;

gint preset_custom_id;

/* Handler for "font-set" signal on Terminal Font font button. */
static void preferences_dialog_font_set_event(GtkFontButton * widget, Setting * setting)
{
    g_free(setting->font_name);
    setting->font_name = g_strdup(gtk_font_button_get_font_name(widget));
    setting->geometry_change = TRUE;        /* Force the terminals to resize */
}

/* Handler for "color-set" signal on Background Color color button. */
static gboolean preferences_dialog_background_color_set_event(GtkWidget * widget, Setting * setting)
{
#if VTE_CHECK_VERSION (0, 38, 0)
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &setting->background_color);
#else
    gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &setting->background_color);
    setting->background_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(widget));

    if (setting->background_alpha == 0)
    {
        setting->background_alpha = 1;
    }
#endif

    GtkComboBox *w = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combobox_color_preset"));
    gtk_combo_box_set_active(w, preset_custom_id);
    setting->color_preset = color_presets[preset_custom_id].name;
    return FALSE;
}

/* Handler for "color-set" signal on Foreground Color color button. */
static gboolean preferences_dialog_foreground_color_set_event(GtkWidget * widget, Setting * setting)
{
#if VTE_CHECK_VERSION (0, 38, 0)
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &setting->foreground_color);
#else
    gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &setting->foreground_color);
#endif

    GtkComboBox *w = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combobox_color_preset"));
    gtk_combo_box_set_active(w, preset_custom_id);
    setting->color_preset = color_presets[preset_custom_id].name;
    return FALSE;
}

/* Handler for "color-set" signal on any palette color button. */
static gboolean preferences_dialog_palette_color_set_event(GtkColorButton * widget, Setting * setting)
{
    int i;

    for (i=0;i<16;i++) {
        gchar *object_key = g_strdup_printf("color_%i", i);
    	GtkColorButton *w = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, object_key));
#if VTE_CHECK_VERSION (0, 38, 0)
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(w), &setting->palette_color[i]);
#else
        gtk_color_button_get_color(GTK_COLOR_BUTTON(w), &setting->palette_color[i]);
#endif
	g_free(object_key);
    }

    GtkComboBox *w = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combobox_color_preset"));
    gtk_combo_box_set_active(w, preset_custom_id);
    setting->color_preset = color_presets[preset_custom_id].name;
    return FALSE;
}

/* Handler for "changed" signal on palette preset menu */
static gboolean preferences_dialog_palette_preset_changed_event(GtkComboBox * widget, Setting * setting)
{
    gint active;
    int i;
    GtkWidget *w;

    active = gtk_combo_box_get_active(widget);
    if (g_strcmp0(color_presets[active].name, "Custom") == 0) {
        return FALSE;
    }

    setting->color_preset = color_presets[active].name;

    w = GTK_WIDGET(gtk_builder_get_object(builder, "background_color"));
#if VTE_CHECK_VERSION (0, 38, 0)
    gdk_rgba_parse(&setting->background_color, color_presets[active].background_color);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w), &setting->background_color);
#else
    gdk_color_parse(color_presets[active].background_color, &setting->background_color);
    setting->background_alpha = 65535;
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->background_color);
    gtk_color_button_set_alpha(GTK_COLOR_BUTTON(w), 65535);
#endif

    w = GTK_WIDGET(gtk_builder_get_object(builder, "foreground_color"));
#if VTE_CHECK_VERSION (0, 38, 0)
    gdk_rgba_parse(&setting->foreground_color, color_presets[active].foreground_color);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w), &setting->foreground_color);
#else
    gdk_color_parse(color_presets[active].foreground_color, &setting->foreground_color);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->foreground_color);
#endif

    for (i=0;i<16;i++) {
        gchar *object_key = g_strdup_printf("color_%i", i);
        w = GTK_WIDGET(gtk_builder_get_object(builder, object_key));
#if VTE_CHECK_VERSION (0, 38, 0)
        gdk_rgba_parse(&setting->palette_color[i], color_presets[active].palette[i]);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w), &setting->palette_color[i]);
#else
        gdk_color_parse(color_presets[active].palette[i], &setting->palette_color[i]);
        gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->palette_color[i]);
#endif
	g_free(object_key);
    }

    return FALSE;
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

/* Handler for "value-changed" signal on spin button. */
static void preferences_dialog_int_value_changed_event(GtkSpinButton * widget, gint * value)
{
    *value = gtk_spin_button_get_value_as_int(widget);
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
static gboolean preferences_dialog_generic_focus_out_event(GtkWidget * widget, GdkEventFocus * event, gchar ** s)
{
    g_free(*s);
    *s = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    return FALSE;
}

/* Specific "key-press-event" handler for shortcut GtkEntry events */
static gboolean preferences_dialog_shortcut_key_press_event(GtkWidget * widget, GdkEventKey * ekey, gchar ** s)
{
    guint key = ekey->keyval;
    guint mod = ekey->state & gtk_accelerator_get_default_mod_mask();
    gchar * lbl;
    GList * sib;

    /* Prevent Tab being used as one of the keys, and return false if doing so. */
    if (key == GDK_KEY_Tab || key == GDK_KEY_ISO_Left_Tab) {
        return FALSE;
    }

    lbl = gtk_accelerator_get_label(key, mod);

    /* Look for duplicate accelerator. */
    for(sib = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(widget))); sib; sib = sib->next) {
	if(GTK_IS_ENTRY(sib->data) && GTK_WIDGET(sib->data) != widget && !g_strcmp0(lbl, gtk_entry_get_text(GTK_ENTRY(sib->data)))) {
            goto free_lbl;
	}
    }

    /* Make sure accelerator is valid. */
    if (!gtk_accelerator_valid(key, mod)) {
        goto free_lbl;
    }

    g_free(*s);
    *s = gtk_accelerator_name(key, mod);
    gtk_entry_set_text(GTK_ENTRY(widget), lbl);

free_lbl:
    g_free(lbl);
    return TRUE;
}

static void accel_set_label(const gchar * name, GtkWidget * w)
{
    guint key;
    GdkModifierType mods;
    gchar * label;
    gtk_accelerator_parse(name, &key, &mods);
    label = gtk_accelerator_get_label(key, mods);
    gtk_entry_set_text(GTK_ENTRY(w), label);
    g_free(label);
}

/* Initialize and display the preferences dialog. */
void terminal_preferences_dialog(GtkAction * action, LXTerminal * terminal)
{
    int i;
    Setting * setting = copy_setting(get_setting());

    builder = gtk_builder_new();
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
#if VTE_CHECK_VERSION (0, 38, 0)
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w), &setting->background_color);
#else
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->background_color);
    gtk_color_button_set_alpha(GTK_COLOR_BUTTON(w), setting->background_alpha);
#endif
    g_signal_connect(G_OBJECT(w), "color-set", 
        G_CALLBACK(preferences_dialog_background_color_set_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "foreground_color"));
#if VTE_CHECK_VERSION (0, 38, 0)
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w), &setting->foreground_color);
#else
    gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->foreground_color);
#endif
    g_signal_connect(G_OBJECT(w), "color-set", 
        G_CALLBACK(preferences_dialog_foreground_color_set_event), setting);

    for (i=0; i<16; i++) {
        gchar *object_key = g_strdup_printf("color_%i", i);
        w = GTK_WIDGET(gtk_builder_get_object(builder, object_key));
#if VTE_CHECK_VERSION (0, 38, 0)
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(w), &setting->palette_color[i]);
#else
        gtk_color_button_set_color(GTK_COLOR_BUTTON(w), &setting->palette_color[i]);
#endif
        g_signal_connect(G_OBJECT(w), "color-set",
            G_CALLBACK(preferences_dialog_palette_color_set_event), setting);
	g_free(object_key);
    }

    GtkListStore *w3 = GTK_LIST_STORE(gtk_builder_get_object(builder, "values_color_presets"));
    GtkComboBox *w2 = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combobox_color_preset"));
    gboolean preset_is_set = FALSE;
    for (i=0; ; i++) {
        gtk_list_store_insert_with_values (w3, NULL, -1, 0, color_presets[i].name, -1);

        if (g_strcmp0(color_presets[i].name, setting->color_preset) == 0) {
            gtk_combo_box_set_active(w2, i);
            preset_is_set = TRUE;
        }

        if (g_strcmp0(color_presets[i].name, "Custom") == 0) {
            if (preset_is_set == FALSE) {
                gtk_combo_box_set_active(w2, i);
            }
            preset_custom_id = i;
            break;
        }
    }

    g_signal_connect(G_OBJECT(w2), "changed",
            G_CALLBACK(preferences_dialog_palette_preset_changed_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "allow_bold"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), ! setting->disallow_bold);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_allow_bold_toggled_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "bold_bright"));
#if VTE_CHECK_VERSION (0, 52, 0)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->bold_bright);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->bold_bright);
#else
    gtk_widget_hide(w);
    w = GTK_WIDGET(gtk_builder_get_object(builder, "label_bold_bright"));
    gtk_widget_hide(w);
#endif

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

    w = GTK_WIDGET(gtk_builder_get_object(builder, "visual_bell"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->visual_bell);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->visual_bell);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "tab_position"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), terminal_tab_get_position_id(setting->tab_position));
    g_signal_connect(G_OBJECT(w), "changed", 
        G_CALLBACK(preferences_dialog_tab_position_changed_event), setting);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "scrollback_lines"));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), setting->scrollback);
    g_signal_connect(G_OBJECT(w), "value-changed", 
        G_CALLBACK(preferences_dialog_int_value_changed_event), &setting->scrollback);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "geometry_columns"));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), setting->geometry_columns);
    g_signal_connect(G_OBJECT(w), "value-changed", 
        G_CALLBACK(preferences_dialog_int_value_changed_event), &setting->geometry_columns);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "geometry_rows"));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), setting->geometry_rows);
    g_signal_connect(G_OBJECT(w), "value-changed", 
        G_CALLBACK(preferences_dialog_int_value_changed_event), &setting->geometry_rows);

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
        G_CALLBACK(preferences_dialog_generic_focus_out_event), &setting->word_selection_characters);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_f10"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_f10);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->disable_f10);

    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_alt"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_alt);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->disable_alt);
    
    w = GTK_WIDGET(gtk_builder_get_object(builder, "disable_confirm"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), setting->disable_confirm);
    g_signal_connect(G_OBJECT(w), "toggled", 
        G_CALLBACK(preferences_dialog_generic_toggled_event), &setting->disable_confirm);

    /* Shortcuts */
#define PREF_SETUP_SHORTCUT(OBJ, VAR) \
    w = GTK_WIDGET(gtk_builder_get_object(builder, OBJ)); \
    accel_set_label(VAR, w); \
    g_signal_connect(G_OBJECT(w), "key-press-event", \
        G_CALLBACK(preferences_dialog_shortcut_key_press_event), &VAR); \

    PREF_SETUP_SHORTCUT(NEW_WINDOW_ACCEL, setting->new_window_accel)
    PREF_SETUP_SHORTCUT(NEW_TAB_ACCEL, setting->new_tab_accel)
    PREF_SETUP_SHORTCUT(CLOSE_TAB_ACCEL, setting->close_tab_accel)
    PREF_SETUP_SHORTCUT(CLOSE_WINDOW_ACCEL, setting->close_window_accel)
    PREF_SETUP_SHORTCUT(COPY_ACCEL, setting->copy_accel)
    PREF_SETUP_SHORTCUT(PASTE_ACCEL, setting->paste_accel)
    PREF_SETUP_SHORTCUT(NAME_TAB_ACCEL, setting->name_tab_accel)
    PREF_SETUP_SHORTCUT(PREVIOUS_TAB_ACCEL, setting->previous_tab_accel)
    PREF_SETUP_SHORTCUT(NEXT_TAB_ACCEL, setting->next_tab_accel)
    PREF_SETUP_SHORTCUT(MOVE_TAB_LEFT_ACCEL, setting->move_tab_left_accel)
    PREF_SETUP_SHORTCUT(MOVE_TAB_RIGHT_ACCEL, setting->move_tab_right_accel)
    PREF_SETUP_SHORTCUT(ZOOM_IN_ACCEL, setting->zoom_in_accel)
    PREF_SETUP_SHORTCUT(ZOOM_OUT_ACCEL, setting->zoom_out_accel)
    PREF_SETUP_SHORTCUT(ZOOM_RESET_ACCEL, setting->zoom_reset_accel)

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
        free_setting(&setting);
    }

    g_object_unref(builder);
}

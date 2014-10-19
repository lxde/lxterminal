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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <X11/Xutil.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <vte/vte.h>
#include <langinfo.h>
#include <locale.h>
#include <sys/stat.h>

#include "lxterminal.h"
#include "setting.h"
#include "preferences.h"
#include "unixsocket.h"

/* Linux color for palette. */
static const GdkColor linux_color[16] =
{
    { 0, 0x0000, 0x0000, 0x0000 },
    { 0, 0xaaaa, 0x0000, 0x0000 },
    { 0, 0x0000, 0xaaaa, 0x0000 },
    { 0, 0xaaaa, 0x5555, 0x0000 },
    { 0, 0x0000, 0x0000, 0xaaaa },
    { 0, 0xaaaa, 0x0000, 0xaaaa },
    { 0, 0x0000, 0xaaaa, 0xaaaa },
    { 0, 0xaaaa, 0xaaaa, 0xaaaa },
    { 0, 0x5555, 0x5555, 0x5555 },
    { 0, 0xffff, 0x5555, 0x5555 },
    { 0, 0x5555, 0xffff, 0x5555 },
    { 0, 0xffff, 0xffff, 0x5555 },
    { 0, 0x5555, 0x5555, 0xffff },
    { 0, 0xffff, 0x5555, 0xffff },
    { 0, 0x5555, 0xffff, 0xffff },
    { 0, 0xffff, 0xffff, 0xffff }
};

/* X accessor. */
static void gdk_window_get_geometry_hints(GdkWindow * window, GdkGeometry * geometry, GdkWindowHints * geometry_mask);

/* Utilities. */
static GtkBorder * terminal_get_border(Term * term);
static void terminal_geometry_restore(Term * term);
static void terminal_tab_set_position(GtkWidget * notebook, gint tab_position);
static gchar * terminal_get_current_dir(LXTerminal * terminal);

/* Menu and accelerator event handlers. */
static void terminal_initialize_switch_tab_accelerator(Term * term);
static void terminal_update_alt(LXTerminal *terminal);
static gboolean terminal_switch_tab_accelerator(Term * term);
static void terminal_new_window_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_new_window_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_new_tab_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_new_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_close_tab_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_close_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_close_window_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_close_window_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_copy_url_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_copy_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_copy_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_paste_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_clear_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_paste_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_name_tab_response_event(GtkWidget * dialog, gint response, LXTerminal * terminal);
static void terminal_name_tab_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_name_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_previous_tab_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_previous_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_next_tab_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_next_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_move_tab_execute(LXTerminal * terminal, gint direction);
static void terminal_move_tab_left_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_move_tab_left_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_move_tab_right_activate_event(GtkAction * action, LXTerminal * terminal);
//static gboolean terminal_move_tab_right_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_about_activate_event(GtkAction * action, LXTerminal * terminal);

/* Window creation, destruction, and control. */
static gboolean terminal_window_size_allocate_event(GtkWidget * widget, GtkAllocation * allocation, LXTerminal * terminal);
static void terminal_window_set_fixed_size(LXTerminal * terminal);
static void terminal_switch_page_event(GtkNotebook * notebook, GtkWidget * page, guint num, LXTerminal * terminal);
static void terminal_window_title_changed_event(GtkWidget * vte, Term * term);
static void terminal_window_exit(LXTerminal * terminal, GObject * where_the_object_was);
static void terminal_child_exited_event(VteTerminal * vte, Term * term);
static gboolean terminal_tab_button_press_event(GtkWidget * widget, GdkEventButton * event, Term * term);
static gboolean terminal_vte_button_press_event(VteTerminal * vte, GdkEventButton * event, Term * term);
static void terminal_settings_apply_to_term(LXTerminal * terminal, Term * term);
static Term * terminal_new(LXTerminal * terminal, const gchar * label, const gchar * pwd, gchar * * env, const gchar * exec);
static void terminal_free(Term * term);
static void terminal_menubar_initialize(LXTerminal * terminal);
static void terminal_menu_accelerator_update(LXTerminal * terminal);
static void terminal_settings_apply(LXTerminal * terminal);
static void terminal_update_menu_shortcuts(Setting * setting);
static void terminal_initialize_menu_shortcuts(Setting * setting);

/* Menu accelerator saved when the user disables it. */
static char * saved_menu_accelerator = NULL;

/* Help when user enters an invalid command. */
static gchar usage_display[] = {
    "Usage:\n"
    "  lxterminal [Options...] - LXTerminal is a terminal emulator\n\n"
    "Options:\n"
    "  -e, --command=STRING             Execute the argument to this option inside the terminal\n"
    "  --geometry=COLUMNSxROWS          Set the terminal's size\n"
    "  -l, --loginshell                 Execute login shell\n"
    "  -t, -T, --title=,\n"
    "    --tabs=NAME[,NAME[,NAME[...]]] Set the terminal's title\n"
    "  --working-directory=DIRECTORY    Set the terminal's working directory\n"
};

/* Actions for menu bar items. */
static GtkActionEntry menu_items[] =
{
/* 0 */    { "File", NULL, N_("_File"), NULL, NULL, NULL },
/* 1 */    { "Edit", NULL, N_("_Edit"), NULL, NULL, NULL },
/* 2 */    { "Tabs", NULL, N_("_Tabs"), NULL, NULL, NULL },
/* 3 */    { "Help", NULL, N_("_Help"), NULL, NULL, NULL },
/* 4 */    { "File_NewWindow", GTK_STOCK_ADD, N_("_New Window"), NEW_WINDOW_ACCEL_DEF, "New Window", G_CALLBACK(terminal_new_window_activate_event) },
/* 5 */    { "File_NewTab", GTK_STOCK_ADD, N_("New T_ab"), NEW_TAB_ACCEL_DEF, "New Tab", G_CALLBACK(terminal_new_tab_activate_event) },
/* 6 */    { "File_Sep1", NULL, "Sep" },
/* 7 */    { "File_CloseTab", GTK_STOCK_CLOSE, N_("_Close Tab"), CLOSE_TAB_ACCEL_DEF, "Close Tab", G_CALLBACK(terminal_close_tab_activate_event) },
/* 8 */    { "File_CloseWindow", GTK_STOCK_QUIT, N_("Close _Window"), CLOSE_WINDOW_ACCEL_DEF, "Close Window", G_CALLBACK(terminal_close_window_activate_event) },
/* 9 */    { "Edit_Copy", GTK_STOCK_COPY, N_("Cop_y"), COPY_ACCEL_DEF, "Copy", G_CALLBACK(terminal_copy_activate_event) },
/* 10 */    { "Edit_Paste", GTK_STOCK_PASTE, N_("_Paste"), PASTE_ACCEL_DEF, "Paste", G_CALLBACK(terminal_paste_activate_event) },
/* 11 */    { "Edit_Clear", NULL, N_("Clear scr_ollback"), NULL, "Clear scrollback", G_CALLBACK(terminal_clear_activate_event) },
/* 12 */    { "Edit_Sep1", NULL, "Sep" },
/* 13 */    { "Edit_Preferences", GTK_STOCK_EXECUTE, N_("Preference_s"), NULL, "Preferences", G_CALLBACK(terminal_preferences_dialog) },
/* 14 */    { "Tabs_NameTab", GTK_STOCK_INFO, N_("Na_me Tab"), NAME_TAB_ACCEL_DEF, "Name Tab", G_CALLBACK(terminal_name_tab_activate_event) },
/* 15 */    { "Tabs_PreviousTab", GTK_STOCK_GO_BACK, N_("Pre_vious Tab"), PREVIOUS_TAB_ACCEL_DEF, "Previous Tab", G_CALLBACK(terminal_previous_tab_activate_event) },
/* 16 */    { "Tabs_NextTab", GTK_STOCK_GO_FORWARD, N_("Ne_xt Tab"), NEXT_TAB_ACCEL_DEF, "Next Tab", G_CALLBACK(terminal_next_tab_activate_event) },
/* 17 */    { "Tabs_MoveTabLeft", NULL, N_("Move Tab _Left"), MOVE_TAB_LEFT_ACCEL_DEF, "Move Tab Left", G_CALLBACK(terminal_move_tab_left_activate_event) },
/* 18 */    { "Tabs_MoveTabRight", NULL, N_("Move Tab _Right"), MOVE_TAB_RIGHT_ACCEL_DEF, "Move Tab Right", G_CALLBACK(terminal_move_tab_right_activate_event) },
/* 19 */    { "Help_About", GTK_STOCK_ABOUT, N_("_About"), NULL, "About", G_CALLBACK(terminal_about_activate_event) },
};
#define MENUBAR_MENUITEM_COUNT G_N_ELEMENTS(menu_items)

/* Descriptors for popup menu items, accessed via right click on the terminal. */
static GtkActionEntry vte_menu_items[] =
{
    { "VTEMenu", NULL, "VTEMenu" },
    { "NewWindow", GTK_STOCK_ADD, N_("New _Window"), NULL, "New Window", G_CALLBACK(terminal_new_window_activate_event) },
    { "NewTab", GTK_STOCK_ADD, N_("New _Tab"), NULL, "New Tab", G_CALLBACK(terminal_new_tab_activate_event) },
    { "Sep1", NULL, "Sep" },
    { "CopyURL", NULL, N_("Copy _URL"), NULL, "Copy URL", G_CALLBACK(terminal_copy_url_activate_event) },
    { "Copy", GTK_STOCK_COPY, N_("Cop_y"), NULL, "Copy", G_CALLBACK(terminal_copy_activate_event) },
    { "Paste", GTK_STOCK_PASTE, N_("_Paste"), NULL, "Paste", G_CALLBACK(terminal_paste_activate_event) },
    { "Clear", NULL, N_("Cl_ear scrollback"), NULL, "Clear scrollback", G_CALLBACK(terminal_clear_activate_event) },
    { "Sep2", NULL, "Sep" },
    { "Preferences", GTK_STOCK_EXECUTE, N_("Preference_s"), NULL, "Preferences", G_CALLBACK(terminal_preferences_dialog) },
    { "Sep3", NULL, "Sep" },
    { "NameTab", GTK_STOCK_INFO, N_("Na_me Tab"), NULL, "Name Tab", G_CALLBACK(terminal_name_tab_activate_event) },
    { "PreviousTab", GTK_STOCK_GO_BACK, N_("Pre_vious Tab"), NULL, "Previous Tab", G_CALLBACK(terminal_previous_tab_activate_event) },
    { "NextTab", GTK_STOCK_GO_FORWARD, N_("Ne_xt Tab"), NULL, "Next Tab", G_CALLBACK(terminal_next_tab_activate_event) },
    { "Tabs_MoveTabLeft", NULL, N_("Move Tab _Left"), NULL, "Move Tab Left", G_CALLBACK(terminal_move_tab_left_activate_event) },
    { "Tabs_MoveTabRight", NULL, N_("Move Tab _Right"), NULL, "Move Tab Right", G_CALLBACK(terminal_move_tab_right_activate_event) },
    { "CloseTab", GTK_STOCK_CLOSE, N_("_Close Tab"), NULL, "Close Tab", G_CALLBACK(terminal_close_tab_activate_event) }
};
#define VTE_MENUITEM_COUNT G_N_ELEMENTS(vte_menu_items)

/* Copied out of static function in gdk_window.
 * A wrapper around XGetWMNormalHints. */
static void gdk_window_get_geometry_hints(GdkWindow * window, GdkGeometry * geometry, GdkWindowHints * geometry_mask)
{
    g_return_if_fail(GDK_IS_WINDOW (window));
    g_return_if_fail(geometry != NULL);
    g_return_if_fail(geometry_mask != NULL);

    *geometry_mask = 0;

    if (gdk_window_is_destroyed(window))
        return;

    XSizeHints size_hints;
    glong junk_size_mask = 0;
    if ( ! XGetWMNormalHints(GDK_WINDOW_XDISPLAY(window), GDK_WINDOW_XID(window), &size_hints, &junk_size_mask))
        return;

    if (size_hints.flags & PMinSize)
    {
        *geometry_mask |= GDK_HINT_MIN_SIZE;
        geometry->min_width = size_hints.min_width;
        geometry->min_height = size_hints.min_height;
    }

    if (size_hints.flags & PMaxSize)
    {
        *geometry_mask |= GDK_HINT_MAX_SIZE;
        geometry->max_width = MAX (size_hints.max_width, 1);
        geometry->max_height = MAX (size_hints.max_height, 1);
    }

    if (size_hints.flags & PResizeInc)
    {
        *geometry_mask |= GDK_HINT_RESIZE_INC;
        geometry->width_inc = size_hints.width_inc;
        geometry->height_inc = size_hints.height_inc;
    }

    if (size_hints.flags & PAspect)
    {
        *geometry_mask |= GDK_HINT_ASPECT;
        geometry->min_aspect = (gdouble) size_hints.min_aspect.x / (gdouble) size_hints.min_aspect.y;
        geometry->max_aspect = (gdouble) size_hints.max_aspect.x / (gdouble) size_hints.max_aspect.y;
    }

    if (size_hints.flags & PWinGravity)
    {
        *geometry_mask |= GDK_HINT_WIN_GRAVITY;
        geometry->win_gravity = size_hints.win_gravity;
    }
}

/* Accessor for border values from VteTerminal. */
static GtkBorder * terminal_get_border(Term * term)
{
#if VTE_CHECK_VERSION(0, 24, 0)
    /* Style property, new in 0.24.0, replaces the function below. */
    GtkBorder * border;
    gtk_widget_style_get(term->vte, "inner-border", &border, NULL);
    return gtk_border_copy(border);
#else
    /* Deprecated function produces a warning. */
    GtkBorder * border = gtk_border_new();
    vte_terminal_get_padding(VTE_TERMINAL(term->vte), &border->left, &border->top);
    return border;
#endif
}

/* Restore the terminal geometry after a font size change or hiding the tab bar. */
static void terminal_geometry_restore(Term * term)
{
    /* Set fixed VTE size. */
    terminal_window_set_fixed_size(term->parent);

    /* Recover the window size. */
    GtkBorder * border = terminal_get_border(term);
    vte_terminal_set_size(VTE_TERMINAL(term->vte),
        vte_terminal_get_column_count(VTE_TERMINAL(term->vte)),
        vte_terminal_get_row_count(VTE_TERMINAL(term->vte)));
    gtk_window_resize(GTK_WINDOW(term->parent->window),
        border->left + vte_terminal_get_char_width(VTE_TERMINAL(term->vte)),
        border->top + vte_terminal_get_char_height(VTE_TERMINAL(term->vte)));
    gtk_border_free(border);
}

/* Set the position of the tabs on the main window. */
static void terminal_tab_set_position(GtkWidget * notebook, gint tab_position)
{
    switch (tab_position)
    {
        case 0:
            gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
            break;
        case 1:
            gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
            break;
        case 2:
            gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
            break;
        default:
            gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_RIGHT);
            break;
    }
}

static gchar * terminal_get_current_dir(LXTerminal * terminal)
{
    gchar * proc_cwd = NULL;

#ifdef __linux
    /* Try to get the working directory from /proc. */
    gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook));
    if (current != -1)
    {
        /* Search for the Term structure corresponding to the current tab. */
        guint i;
        for (i = 0; i < terminal->terms->len; i++)
        {
            Term * term = g_ptr_array_index(terminal->terms, i);
            if (term->index == current)
            {
                /* Get the working directory corresponding to the process ID. */
                gchar proc_cwd_link[PATH_MAX];
                g_snprintf(proc_cwd_link, PATH_MAX, "/proc/%d/cwd", term->pid);
                proc_cwd = g_file_read_link(proc_cwd_link, NULL);
                break;
            }
        }

    }
#endif

    if (proc_cwd == NULL)
    {
        proc_cwd = g_get_current_dir();
    }

    return proc_cwd;
}

/* Initialize the <ALT> n accelerators, where n is a digit.
 * These switch to the tab selected by the digit, if it exists. */
static void terminal_initialize_switch_tab_accelerator(Term * term)
{
    if ((term->index + 1) < 10)
    {
        /* Formulate the accelerator name. */
        char switch_tab_accel[1 + 3 + 1 + 1 + 1]; /* "<ALT>n" */
        sprintf(switch_tab_accel, "<ALT>%d", term->index + 1);

        /* Parse the accelerator name. */
        guint key;
        GdkModifierType mods;
        gtk_accelerator_parse(switch_tab_accel, &key, &mods);

        /* Define the accelerator. */
        term->closure = g_cclosure_new_swap(G_CALLBACK(terminal_switch_tab_accelerator), term, NULL);
        if (gtk_accel_group_from_accel_closure(term->closure) == NULL)
        {
            gtk_accel_group_connect(term->parent->accel_group, key, mods, GTK_ACCEL_LOCKED, term->closure);
        }
    }
}

/* update <ALT>n status. */
void terminal_update_alt(LXTerminal *terminal)
{
    guint i;
    Term * term;

    /* disable alt when the option is switched on or terminal has no other tabs */
    if (get_setting()->disable_alt || terminal->terms->len <= 1)
    {
        for (i = 0; i < terminal->terms->len; i++)
        {
            term = g_ptr_array_index(terminal->terms, i);

            if (term->closure != NULL)
            {
                gtk_accel_group_disconnect(term->parent->accel_group, term->closure);
            }
        }
    }
    else
    {
        for (i = 0; i < terminal->terms->len; i++)
        {
            term = g_ptr_array_index(terminal->terms, i);

            if (GTK_IS_ACCEL_GROUP(term->parent->accel_group))
            {
                if (term->closure != NULL)
                {
                    gtk_accel_group_disconnect(term->parent->accel_group, term->closure);
                }
                terminal_initialize_switch_tab_accelerator(term);
            }
        }
    }
}

/* Handler for accelerator <ALT> n, where n is a digit.
 * Switch to the tab selected by the digit, if it exists. */
static gboolean terminal_switch_tab_accelerator(Term * term)
{
    LXTerminal * terminal = term->parent;
    if (term->index < gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook))) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), term->index);
        return TRUE;
    }
    return FALSE;
}

/* Handler for "activate" signal on File/New Window menu item.
 * Open a new window. */
static void terminal_new_window_activate_event(GtkAction * action, LXTerminal * terminal)
{
    CommandArguments arguments;
    memset(&arguments, 0, sizeof(arguments));
    arguments.working_directory = terminal_get_current_dir(terminal);
    lxterminal_initialize(terminal->parent, &arguments);
    g_free(arguments.working_directory);
}

/* Handler for accelerator <SHIFT><CTRL> N.  Open a new window. */
/*static gboolean terminal_new_window_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_new_window_activate_event(NULL, terminal);
    return TRUE;
}*/

/* Handler for "activate" signal on File/New Tab menu item.
 * Open a new tab. */
static void terminal_new_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    gchar * proc_cwd = terminal_get_current_dir(terminal);

    /* Propagate the working directory of the current tab to the new tab.
     * If the working directory was determined above, use it; otherwise default to the working directory of the process.
     * Create the new terminal. */

    Term * term = terminal_new(terminal, _("LXTerminal"), proc_cwd, NULL, NULL);
    g_free(proc_cwd);

    /* Add a tab to the notebook and the "terms" array. */
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->tab);
    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
    g_ptr_array_add(terminal->terms, term);

    /* Make the new terminal's tab current and, if we went from one to more than one tab,
     * turn the tab display on. */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(term->parent->notebook), term->index);
    if (term->index > 0)
    {
        terminal_window_set_fixed_size(terminal);
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(term->parent->notebook), TRUE);
    }

    /* Disable Alt-n switch tabs or not. */
    terminal_update_alt(terminal);
}

/* Handler for accelerator <SHIFT><CTRL> T.  Open a new tab. */
/*static gboolean terminal_new_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_new_tab_activate_event(NULL, terminal);
    return TRUE;
}*/

/* Handler for "activate" signal on File/Close Tab menu item.
 * Close the current tab. */
static void terminal_close_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    terminal_child_exited_event(VTE_TERMINAL(term->vte), term);
}

/* Handler for accelerator <SHIFT><CTRL> W.  Close the current tab. */
/*static gboolean terminal_close_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_close_tab_activate_event(NULL, terminal);
    return TRUE;
}*/

/* Handler for "activate" signal on File/Close Window menu item.
 * Close the current window. */
static void terminal_close_window_activate_event(GtkAction * action, LXTerminal * terminal)
{
    /* Play it safe and delete tabs one by one. */
    while(terminal->terms->len > 0)
    {
        terminal_child_exited_event(NULL, g_ptr_array_index(terminal->terms, 0));
    }
}

/* Handler for accelerator <SHIFT><CTRL> Q.  Close the current window. */
/*static gboolean terminal_close_window_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_close_window_activate_event(NULL, terminal);
    return TRUE;
}*/

static void terminal_copy_url_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    if (term->matched_url)
    {
        GtkClipboard* clipboard = gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE));
        gtk_clipboard_set_text(clipboard, term->matched_url, -1);
    }
}

/* Handler for "activate" signal on Edit/Copy menu item.
 * Copy to the clipboard. */
static void terminal_copy_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

/* Handler for accelerator <CTRL><SHIFT> C.  Copy to the clipboard. */
/*static gboolean terminal_copy_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    // fire event only if text selected
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    if (vte_terminal_get_has_selection(VTE_TERMINAL(term->vte)))
    {
        terminal_copy_activate_event(NULL, terminal);
        return TRUE;
    }
    else
    {   
        // if not selected send keys to terminal
        return FALSE;
    }
}*/

/* Handler for "activate" signal on Edit/Paste menu item.
 * Paste from the clipboard. */
static void terminal_paste_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

/* Handler for "clear scrollback" signal on Edit/Paste menu item.
 * Clear scrollback. */
static void terminal_clear_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), 0);
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), get_setting()->scrollback);
}

/* Handler for accelerator <CTRL><SHIFT> V.  Paste from the clipboard. */
/*static gboolean terminal_paste_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_paste_activate_event(NULL, terminal);
    return TRUE;
}*/

/* Handler for "response" signal on Name Tab dialog. */
static void terminal_name_tab_response_event(GtkWidget * dialog, gint response, LXTerminal * terminal)
{
    if (response == GTK_RESPONSE_OK)
    {
        GtkWidget * dialog_item = g_object_get_data(G_OBJECT(dialog), "entry");

        gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook));
        if (current != -1)
        {
            /* Search for the Term structure corresponding to the current tab. */
            guint i;
            for (i = 0; i < terminal->terms->len; i++)
            {
                Term * term = g_ptr_array_index(terminal->terms, i);
                if (term->index == current)
                {
                /* If Term structure found, set the tab's label and mark it so we will never overwrite it. */
                    term->user_specified_label = TRUE;
                    gtk_label_set_text(GTK_LABEL(term->label), g_strdup(gtk_entry_get_text(GTK_ENTRY(dialog_item))));
                    break;
                }
            }
        }
    }

    /* Dismiss dialog. */
    gtk_widget_destroy(dialog);
}

/* Handler for "activate" signal on Tabs/Name Tab menu item.
 * Put up a dialog to get a user specified name for the tab. */
static void terminal_name_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    GtkWidget * dialog = gtk_dialog_new_with_buttons(
        _("Name Tab"),
        GTK_WINDOW(terminal->window),
        0,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    if (gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), "lxterminal"))
    {
        gtk_window_set_icon_name(GTK_WINDOW(dialog), "lxterminal");
    }
    else
    {
        gtk_window_set_icon_from_file(GTK_WINDOW(dialog), PACKAGE_DATA_DIR "/icons/hicolor/128x128/apps/lxterminal.png", NULL);
    }
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(terminal_name_tab_response_event), terminal);
    GtkWidget * dialog_item = gtk_entry_new();
    g_object_set_data(G_OBJECT(dialog), "entry", (gpointer) dialog_item);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), dialog_item, FALSE, FALSE, 2);
    gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook));
    if (current != -1)
    {
        /* Search for the Term structure corresponding to the current tab. */
        guint i;
        for (i = 0; i < terminal->terms->len; i++)
        {
            Term * term = g_ptr_array_index(terminal->terms, i);
            if (term->index == current)
            {
                gtk_entry_set_text(GTK_ENTRY(dialog_item), gtk_label_get_text(GTK_LABEL(term->label)));
                break;
            }
        }
    }
    gtk_entry_set_activates_default(GTK_ENTRY(dialog_item), TRUE);
    gtk_widget_show_all(dialog);
}

/* Handler for accelerator <CTRL><SHIFT> R.  Name the tab. */
/*static gboolean terminal_name_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_name_tab_activate_event(NULL, terminal);
    return TRUE;
}*/

/* Handler for "activate" signal on Tabs/Previous Tab menu item.
 * Cycle through tabs in the reverse direction. */
static void terminal_previous_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    /* Cycle through tabs. */
    if (gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)) == 0)
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), -1);
    }
    else
    {
        gtk_notebook_prev_page(GTK_NOTEBOOK(terminal->notebook));
    }
}

/* Handler for accelerator <CTRL><PAGE UP>.  Cycle through tabs in the reverse direction. */
/*static gboolean terminal_previous_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    GtkAction *_action = gtk_action_group_get_action(terminal->action_group, "Tabs_PreviousTab");
    gtk_action_activate(_action);
    return TRUE;
}*/

/* Handler for "activate" signal on Tabs/Next Tab menu item.
 * Cycle through tabs in the forward direction. */
static void terminal_next_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    /* Cycle through tabs. */
    if (gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)) == gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1)
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), 0);
    }
    else
    {
        gtk_notebook_next_page(GTK_NOTEBOOK(terminal->notebook));
    }
}

/* Handler for accelerator <CTRL><PAGE DOWN>.  Cycle through tabs in the forward direction. */
/*static gboolean terminal_next_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    GtkAction *_action = gtk_action_group_get_action(terminal->action_group, "Tabs_NextTab");
    gtk_action_activate(_action);
    return TRUE;
}*/

/* Helper for move tab left and right. */
static void terminal_move_tab_execute(LXTerminal * terminal, gint direction)
{
    GtkNotebook * notebook = GTK_NOTEBOOK(terminal->notebook);
    gint current_page_number = gtk_notebook_get_current_page(notebook);
    gint target_page_number = current_page_number + direction;

    /* prevent out of index, cast should be safe as we catch negatives prior */
    if (target_page_number < 0 || (guint) target_page_number >= terminal->terms->len)
    {
        return;
    }

    /* swap index in terms array and its id */
    Term * term_current = g_ptr_array_index(terminal->terms, current_page_number);
    Term * term_target = g_ptr_array_index(terminal->terms, target_page_number);
    g_ptr_array_index(terminal->terms, target_page_number) = term_current;
    g_ptr_array_index(terminal->terms, current_page_number) = term_target;
    term_current->index = target_page_number;
    term_target->index = current_page_number;

    gtk_notebook_reorder_child(notebook, gtk_notebook_get_nth_page(notebook, current_page_number), target_page_number);

    /* update alt arrangement */
    terminal_update_alt(terminal);
}

/* Handler for "activate" signal on Tabs/Move Tab Left menu item.
 * Move the tab one position in the reverse direction. */
static void terminal_move_tab_left_activate_event(GtkAction * action, LXTerminal * terminal)
{
    terminal_move_tab_execute(terminal, -1);
}

/* Handler for accelerator <SHIFT><CTRL><PAGE UP>.  Move the tab one position in the reverse direction. */
/*static gboolean terminal_move_tab_left_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_move_tab_execute(terminal, -1);
    return TRUE;
}*/

/* Handler for "activate" signal on Tabs/Move Tab Right menu item.
 * Move the tab one position in the forward direction. */
static void terminal_move_tab_right_activate_event(GtkAction * action, LXTerminal * terminal)
{
    terminal_move_tab_execute(terminal, 1);
}

/* Handler for accelerator <SHIFT><CTRL><PAGE DOWN>.  Move the tab one position in the forward direction. */
/*static gboolean terminal_move_tab_right_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_move_tab_execute(terminal, 1);
    return TRUE;
}*/

/* Handler for "activate" signal on Help/About menu item. */
static void terminal_about_activate_event(GtkAction * action, LXTerminal * terminal)
{
    const gchar * authors[] =
    {
        "Fred Chien <cfsghost@gmail.com>",
        "Marty Jack <martyj19@comcast.net>",
        "Yao Wei <mwei@lxde.org>",
        NULL
    };
    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar * translators = _("translator-credits");

    /* Create and initialize the dialog. */
    GtkWidget * about_dlg = gtk_about_dialog_new();
    gtk_container_set_border_width(GTK_CONTAINER(about_dlg), 2);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dlg), VERSION);
    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(about_dlg), _("LXTerminal"));
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dlg), gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR "/icons/hicolor/128x128/apps/lxterminal.png", NULL));
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dlg), _("Copyright (C) 2008-2011"));
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dlg), _("Terminal emulator for LXDE project"));
    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_dlg), "This program is free software; you can redistribute it and/or\nmodify it under the terms of the GNU General Public License\nas published by the Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dlg), "http://lxde.org/");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dlg), authors);
    gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about_dlg), translators);

    /* Display the dialog, wait for the user to click OK, and dismiss the dialog. */
    gtk_dialog_run(GTK_DIALOG(about_dlg));
    gtk_widget_destroy(about_dlg);
}

/* Handler for "size-request" signal on the top level window. */
static gboolean terminal_window_size_allocate_event(GtkWidget * widget, GtkAllocation * allocation, LXTerminal * terminal)
{
    /* Only do this once. */
    if (terminal->fixed_size)
    {
        /* No longer fixed size. */
        terminal->fixed_size = FALSE;
        /* Initialize geometry hints structure. */
        Term * term = g_ptr_array_index(terminal->terms, 0);
        GtkBorder * border = terminal_get_border(term);
        GdkGeometry hints;
        hints.width_inc = vte_terminal_get_char_width(VTE_TERMINAL(term->vte));
        hints.height_inc = vte_terminal_get_char_height(VTE_TERMINAL(term->vte));
        hints.base_width = border->right + 1;
        hints.base_height = border->bottom + 1;
        hints.min_width = hints.base_width + hints.width_inc * 4;
        hints.min_height = hints.base_height + hints.height_inc * 2;
        gtk_border_free(border);

        /* Set hints into all terminals.  This makes sure we resize on character cell boundaries. */
        guint i;
        for (i = 0; i < terminal->terms->len; i++)
        {
            Term * term = g_ptr_array_index(terminal->terms, i);
            gtk_window_set_geometry_hints(GTK_WINDOW(terminal->window),
            term->vte,
            &hints,
                GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);
        }

        /* Resize the window. */
	if (allocation->width > 0 && allocation->height > 0)
            gtk_window_resize(GTK_WINDOW(terminal->window), allocation->width, allocation->height);
    }
    return FALSE;
}

/* Set geometry hints for the fixed size case. */
static void terminal_window_set_fixed_size(LXTerminal * terminal)
{
    terminal->fixed_size = TRUE;
    guint i;
    for (i = 0; i < terminal->terms->len; i++)
    {
        Term * term = g_ptr_array_index(terminal->terms, i);
        gtk_window_set_geometry_hints(GTK_WINDOW(terminal->window),
            term->vte,
            &terminal->geometry,
            terminal->geometry_mask);
    }
}

/* Handler for "switch-page" event on the tab notebook. */
static void terminal_switch_page_event(GtkNotebook * notebook, GtkWidget * page, guint num, LXTerminal * terminal)
{
    if (terminal->terms->len > num)
    {
        /* Propagate the title to the toplevel window. */
        Term * term = g_ptr_array_index(terminal->terms, num);
        const gchar * title = vte_terminal_get_window_title(VTE_TERMINAL(term->vte));
        gtk_window_set_title(GTK_WINDOW(terminal->window), ((title != NULL) ? title : _("LXTerminal")));
    }
}

/* Handler for "window-title-changed" signal on a Term. */
static void terminal_window_title_changed_event(GtkWidget * vte, Term * term)
{
    /* Copy the VTE data out into the tab and the window title, unless the user edited the tab label. */
    if ( ! term->user_specified_label)
    {
        gtk_label_set_text(GTK_LABEL(term->label), vte_terminal_get_window_title(VTE_TERMINAL(vte)));
        gtk_widget_set_tooltip_text(term->label, vte_terminal_get_window_title(VTE_TERMINAL(vte)));
    }
    gtk_window_set_title(GTK_WINDOW(term->parent->window), vte_terminal_get_window_title(VTE_TERMINAL(vte)));
}

/* Weak-notify callback for LXTerminal object. */
static void terminal_window_exit(LXTerminal * terminal, GObject * where_the_object_was)
{
    /* If last window, exit main loop. */
    if (terminal->parent->windows->len == 1)
        gtk_main_quit();

    else
    {
        /* Remove the element and decrease the index number of each succeeding element. */
        g_ptr_array_remove_index(terminal->parent->windows, terminal->index);
        guint i;
        for (i = terminal->index; i < terminal->parent->windows->len; i++)
        {
            LXTerminal * t = g_ptr_array_index(terminal->parent->windows, i);
            t->index --;
        }

        /* Release */
        g_slice_free(LXTerminal, terminal);
    }
}

/* Handler for "child-exited" signal on VTE.
 * Also handler for "activate" signal on Close button of tab and File/Close Tab menu item and accelerator. */
static void terminal_child_exited_event(VteTerminal * vte, Term * term)
{
    LXTerminal * terminal = term->parent;

    /* Last tab being deleted.  Deallocate memory and close the window. */
    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) == 1)
    {
        g_ptr_array_free(terminal->terms, TRUE);
        gtk_widget_destroy(terminal->window);
    }

    /* Not last tab being deleted. */
    else
    {
        /* Remove the element and decrease the index number of each succeeding element. */
        g_ptr_array_remove_index(terminal->terms, term->index);
        guint i;
        for (i = term->index; i < terminal->terms->len; i++)
        {
            Term * t = g_ptr_array_index(terminal->terms, i);
            t->index --;
        }

        /* Delete the tab and free the Term structure. */
        gtk_notebook_remove_page(GTK_NOTEBOOK(terminal->notebook), term->index);
        terminal_free(term);

        /* If only one page is left, hide the tab. */
        if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) == 1)
            gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);

        /* update <ALT>n status */
        terminal_update_alt(terminal);
    }
}

/* Handler for "button-press-event" signal on a notebook tab. */
static gboolean terminal_tab_button_press_event(GtkWidget * widget, GdkEventButton * event, Term * term)
{
    if (event->button == 2)
    {
        /* Middle click closes the tab. */
        terminal_child_exited_event(NULL, term);
        return TRUE;
    }
    return FALSE;
}

static gchar * terminal_get_match_at(VteTerminal * vte, Term * term, int x, int y)
{
    /* steal from tilda-0.09.6/src/tilda_terminal.c:743
     * See if the terminal has matched the regular expression. */
    GtkBorder * border = terminal_get_border(term);
    gint tag;
    gchar * match = vte_terminal_match_check(vte,
        (x - border->left) / vte_terminal_get_char_width(vte),
        (y - border->top) / vte_terminal_get_char_height(vte),
        &tag);
    gtk_border_free(border);

    return match;
}

static void terminal_show_popup_menu(VteTerminal * vte, GdkEventButton * event, Term * term)
{
    /* Generate popup menu. */
    GtkUIManager * manager = gtk_ui_manager_new();
    GtkActionGroup * action_group = gtk_action_group_new("VTEMenu");
    gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions(action_group, vte_menu_items, VTE_MENUITEM_COUNT, term->parent);
    gtk_ui_manager_insert_action_group(manager, action_group, 0);

    guint merge_id = gtk_ui_manager_new_merge_id(manager);
    gtk_ui_manager_add_ui(manager, merge_id, "/", "VTEMenu", NULL, GTK_UI_MANAGER_POPUP, FALSE);

    size_t i;
    for (i = 1; i < VTE_MENUITEM_COUNT; i++)
    {
        if (strcmp(vte_menu_items[i].label, "Sep") == 0)
            gtk_ui_manager_add_ui(manager, merge_id, "/VTEMenu",
                vte_menu_items[i].name, NULL, GTK_UI_MANAGER_SEPARATOR, FALSE);
        else
            gtk_ui_manager_add_ui(manager, merge_id, "/VTEMenu",
                vte_menu_items[i].name, vte_menu_items[i].name, GTK_UI_MANAGER_MENUITEM, FALSE);
    }

    g_free(term->matched_url);
    term->matched_url = terminal_get_match_at(vte, term, event->x, event->y);

    GtkAction * action_copy_url = gtk_ui_manager_get_action(manager, "/VTEMenu/CopyURL");
    if (action_copy_url)
        gtk_action_set_visible(action_copy_url, term->matched_url != NULL);

    gtk_menu_popup(GTK_MENU(gtk_ui_manager_get_widget(manager, "/VTEMenu")),
        NULL, NULL, NULL, NULL, event->button, event->time);
}

/* Handler for "button-press-event" signal on VTE. */
static gboolean terminal_vte_button_press_event(VteTerminal * vte, GdkEventButton * event, Term * term)
{
    if (event->type != GDK_BUTTON_PRESS) /* skip GDK_2BUTTON_PRESS and GDK_3BUTTON_PRESS */
        return FALSE;

    //g_print("press\n");

    /* Right-click. */
    if (event->button == 3)
    {
        if (event->state & GDK_CONTROL_MASK)
        {
            terminal_show_popup_menu(vte, event, term);
            return TRUE;
        }
        else
            term->open_menu_on_button_release = TRUE;
    }

    /* Control left click. */
    else if ((event->button == 1) && (event->state & GDK_CONTROL_MASK))
    {
        gchar * match = terminal_get_match_at(vte, term, event->x, event->y);
        if (match != NULL)
        {
            gchar * cmd = g_strdup_printf("xdg-open %s", match);
            if ( ! g_spawn_command_line_async(cmd, NULL))
                g_warning("Failed to launch xdg-open. The command was `%s'\n", cmd);
            g_free(cmd);
            g_free(match);
        }
    }
    return FALSE;
}

static gboolean terminal_vte_button_release_event(VteTerminal * vte, GdkEventButton * event, Term * term)
{
    //g_print("release\n");

    if (event->button == 3 && term->open_menu_on_button_release)
    {
        terminal_show_popup_menu(vte, event, term);
    }

    term->open_menu_on_button_release = FALSE;

    return FALSE;
}

static void terminal_vte_commit(VteTerminal * vte, gchar * text, guint size, Term * term)
{
    //g_print("commit\n");
    term->open_menu_on_button_release = FALSE;
}

/* Apply new settings in an LXTerminal to its tab Term. */
static void terminal_settings_apply_to_term(LXTerminal * terminal, Term * term)
{
    Setting * setting = get_setting();

    /* Terminal properties. */
    vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), setting->font_name);
    vte_terminal_set_word_chars(VTE_TERMINAL(term->vte), setting->word_selection_characters);
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), setting->scrollback);
    vte_terminal_set_allow_bold(VTE_TERMINAL(term->vte), ! setting->disallow_bold);
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term->vte), ((setting->cursor_blink) ? VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF));
    vte_terminal_set_cursor_shape(VTE_TERMINAL(term->vte), ((setting->cursor_underline) ? VTE_CURSOR_SHAPE_UNDERLINE : VTE_CURSOR_SHAPE_BLOCK));
    vte_terminal_set_audible_bell(VTE_TERMINAL(term->vte), setting->audible_bell);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(term->vte), setting->hide_pointer);

    /* Background and foreground colors. */
    if (terminal->rgba)
    {
        /* vte_terminal_queue_background_update doesn't run without changing background. */
        vte_terminal_set_color_background(VTE_TERMINAL(term->vte), &setting->foreground_color);
        vte_terminal_set_background_transparent(VTE_TERMINAL(term->vte), FALSE);
        vte_terminal_set_opacity(VTE_TERMINAL(term->vte), setting->background_alpha);
    }
    else
    {
        vte_terminal_set_background_transparent(VTE_TERMINAL(term->vte), setting->background_alpha == 65535 ? FALSE : TRUE);
        vte_terminal_set_background_saturation(VTE_TERMINAL(term->vte), 1 - ((double) setting->background_alpha / 65535));
    }
    vte_terminal_set_colors(VTE_TERMINAL(term->vte), &setting->foreground_color, &setting->background_color, &linux_color[0], 16);

    /* Hide or show scrollbar. */
    if (setting->hide_scroll_bar)
    {
        gtk_widget_hide(term->scrollbar);
    }
    else
    {
        gtk_widget_show(term->scrollbar);
    }

    /* Hide or show Close button. */
    if (setting->hide_close_button)
    {
        gtk_widget_hide(term->close_button);
    }
    else
    {
        gtk_widget_show(term->close_button);
    }

    /* If terminal geometry changed, react to it. */
    if (setting->geometry_change)
    {
        terminal_geometry_restore(term);
    }
}

/* Create a new terminal. */
static Term * terminal_new(LXTerminal * terminal, const gchar * label, const gchar * pwd, gchar * * env, const gchar * exec)
{
    /* Create and initialize Term structure for new terminal. */
    Term * term = g_slice_new0(Term);
    term->parent = terminal;

    /* Create a VTE and a vertical scrollbar, and place them inside a horizontal box. */
    term->vte = vte_terminal_new();
    term->box = gtk_hbox_new(FALSE, 0);
    term->scrollbar = gtk_vscrollbar_new(NULL);
    gtk_box_pack_start(GTK_BOX(term->box), term->vte, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(term->box), term->scrollbar, FALSE, TRUE, 0);
    gtk_widget_set_no_show_all(GTK_WIDGET(term->scrollbar), TRUE);

    /* Set up the VTE. */
    setlocale(LC_ALL, "");
    vte_terminal_set_emulation(VTE_TERMINAL(term->vte), "xterm");
    vte_terminal_set_encoding(VTE_TERMINAL(term->vte), nl_langinfo(CODESET));
    vte_terminal_set_backspace_binding(VTE_TERMINAL(term->vte), VTE_ERASE_ASCII_DELETE);
    vte_terminal_set_delete_binding(VTE_TERMINAL(term->vte), VTE_ERASE_DELETE_SEQUENCE);

    /* steal from tilda-0.09.6/src/tilda_terminal.c:145 */
    /* Match URL's, etc. */
    GRegex * dingus1 = g_regex_new(DINGUS1, G_REGEX_OPTIMIZE, 0, NULL);
    GRegex * dingus2 = g_regex_new(DINGUS2, G_REGEX_OPTIMIZE, 0, NULL);
    gint ret = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), dingus1, 0);
    vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), ret, GDK_HAND2);
    ret = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), dingus2, 0);
    vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), ret, GDK_HAND2);
    g_regex_unref(dingus1);
    g_regex_unref(dingus2);

    /* Create a horizontal box inside an event box as the toplevel for the tab label. */
    term->tab = gtk_event_box_new();
    gtk_widget_set_events(term->tab, GDK_BUTTON_PRESS_MASK);
    GtkWidget * hbox = gtk_hbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(term->tab), hbox);

    /* Create the Close button. */
    term->close_button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(term->close_button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(term->close_button), FALSE);
    gtk_container_add(GTK_CONTAINER(term->close_button), gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));

    /* Make the button as small as possible. */
    GtkRcStyle * rcstyle = gtk_rc_style_new();
    rcstyle->xthickness = rcstyle->ythickness = 0;
    gtk_widget_modify_style(term->close_button, rcstyle);
    g_object_ref(rcstyle);

    /* Create the label. */
    term->label = gtk_label_new((label != NULL) ? label : pwd);
    gtk_widget_set_size_request(GTK_WIDGET(term->label), 100, -1);
    gtk_label_set_ellipsize(GTK_LABEL(term->label), PANGO_ELLIPSIZE_END);
    gtk_misc_set_alignment(GTK_MISC(term->label), 0.0, 0.5);
    gtk_misc_set_padding(GTK_MISC(term->label), 0, 0);

    /* Pack everything and show the widget. */
    gtk_box_pack_start(GTK_BOX(hbox), term->label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), term->close_button, FALSE, FALSE, 0);
    gtk_widget_show_all(term->tab);

    /* Set up scrollbar. */
    gtk_range_set_adjustment(GTK_RANGE(term->scrollbar), vte_terminal_get_adjustment(VTE_TERMINAL(term->vte)));

    /* Fork the process that will have the VTE as its controlling terminal. */
    if (exec == NULL)
    {
            exec = g_getenv("SHELL");
    }

    gchar ** command;
    g_shell_parse_argv(exec, NULL, &command, NULL);

    vte_terminal_fork_command_full(
                    VTE_TERMINAL(term->vte),
                    VTE_PTY_NO_LASTLOG | VTE_PTY_NO_UTMP | VTE_PTY_NO_WTMP,
                    pwd,
                    command,
                    env,
                    G_SPAWN_SEARCH_PATH,
                    NULL,
                    NULL,
                    &term->pid,
                    NULL);
    g_strfreev(command);

    /* Connect signals. */
    g_signal_connect(G_OBJECT(term->tab), "button-press-event", G_CALLBACK(terminal_tab_button_press_event), term);
    g_signal_connect(G_OBJECT(term->close_button), "clicked", G_CALLBACK(terminal_child_exited_event), term);
    g_signal_connect(G_OBJECT(term->vte), "button-press-event", G_CALLBACK(terminal_vte_button_press_event), term);
    g_signal_connect(G_OBJECT(term->vte), "button-release-event", G_CALLBACK(terminal_vte_button_release_event), term);
    g_signal_connect(G_OBJECT(term->vte), "commit", G_CALLBACK(terminal_vte_commit), term);
    g_signal_connect(G_OBJECT(term->vte), "child-exited", G_CALLBACK(terminal_child_exited_event), term);
    g_signal_connect(G_OBJECT(term->vte), "window-title-changed", G_CALLBACK(terminal_window_title_changed_event), term);

    /* Show the widget and return. */
    gtk_widget_show_all(term->box);

    /* Apply user preferences. */
    terminal_settings_apply_to_term(terminal, term);
    return term;
}

/* Deallocate a Term structure. */
static void terminal_free(Term * term)
{
    g_free(term->matched_url);
    if ((GTK_IS_ACCEL_GROUP(term->parent->accel_group)) && (term->closure != NULL))
    {
        gtk_accel_group_disconnect(term->parent->accel_group, term->closure);
    }
    g_slice_free(Term, term);
}

/* Initialize the menu bar. */
static void terminal_menubar_initialize(LXTerminal * terminal)
{
    /* Initialize UI manager. */
    GtkUIManager * manager = gtk_ui_manager_new();
    terminal->action_group = gtk_action_group_new("MenuBar");
    gtk_action_group_set_translation_domain(terminal->action_group, GETTEXT_PACKAGE);
    /* modify accelerators by setting */
    terminal_initialize_menu_shortcuts(get_setting());
    gtk_action_group_add_actions(terminal->action_group, menu_items, MENUBAR_MENUITEM_COUNT, terminal);
    gtk_ui_manager_insert_action_group(manager, terminal->action_group, 0);
    
    gtk_ui_manager_add_ui_from_file (manager, PACKAGE_DATA_DIR "/lxterminal/menu.ui", NULL);
    
    terminal->menu = gtk_ui_manager_get_widget(manager, "/MenuBar");
    
    /* Extract accelerators from manager and add it to top level window. */
    terminal->accel_group = gtk_ui_manager_get_accel_group(manager);
    
    gtk_window_add_accel_group(GTK_WINDOW(terminal->window), terminal->accel_group);
}

/* Update the accelerator that brings up the menu.
 * We have a user preference as to whether F10 (or a style-supplied alternate) is used for this purpose.
 * Technique taken from gnome-terminal. */
static void terminal_menu_accelerator_update(LXTerminal * terminal)
{
    /* Ensure that saved_menu_accelerator is initialized. */
    if (saved_menu_accelerator == NULL)
    {
        g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-menu-bar-accel", &saved_menu_accelerator, NULL);
    }

    /* If F10 is disabled, set the accelerator to a key combination that is not F10 and unguessable. */
    gtk_settings_set_string_property(
        gtk_settings_get_default(),
        "gtk-menu-bar-accel",
        ((get_setting()->disable_f10) ? "<Shift><Control><Mod1><Mod2><Mod3><Mod4><Mod5>F10" : saved_menu_accelerator),
        "lxterminal");
}

/* Process the argument vector into the CommandArguments structure.
 * This is called from the main entry, and also from the controlling socket flow. */
gboolean lxterminal_process_arguments(gint argc, gchar * * argv, CommandArguments * arguments)
{
    /* Loop over the argument vector to produce the CommandArguments structure. */
    memset(arguments, 0, sizeof(CommandArguments));
    arguments->executable = argv[0];

    gboolean login_shell = FALSE;
    char * * argv_cursor = argv + 1;
    argc --;
    while (argc > 0)
    {
        char * argument = *argv_cursor;

        /* --command=<command> */
        if (strncmp(argument, "--command=", 10) == 0)
        {
            g_free(arguments->command);
            arguments->command = g_strdup(&argument[10]);
        }

        /* -e <rest of arguments>, --command <rest of arguments>
         * The <rest of arguments> behavior is demanded by distros who insist on this xterm feature. */
        else if ((strcmp(argument, "--command") == 0) || (strcmp(argument, "-e") == 0))
        {
            while (argc > 1)
            {
                argc --;
                argv_cursor ++;
                if (arguments->command == NULL)
                {
                    arguments->command = g_strdup(*argv_cursor);
                }
                else
                {
                    gchar * quoted_arg = g_shell_quote(*argv_cursor);
                    gchar * new_command = g_strconcat(arguments->command, " ", quoted_arg, NULL);
                    g_free(quoted_arg);
                    g_free(arguments->command);
                    arguments->command = new_command;
                }
            }
        }

        /* --geometry=<columns>x<rows> */
        else if (strncmp(argument, "--geometry=", 11) == 0)
        {
            int result = sscanf(&argument[11], "%dx%d", &arguments->geometry_columns, &arguments->geometry_rows);
            if (result != 2)
            {
                return FALSE;
            }
        }

        /* -l, --loginshell */
        else if ((strcmp(argument, "--loginshell") == 0) || (strcmp(argument, "-l") == 0))
        {
            login_shell = TRUE;
        }

        /* --title=<title> */
        else if (strncmp(argument, "--title=", 8) == 0)
        {
            arguments->title = &argument[8];
        }

        /* --tabs=<names> */
        else if (strncmp(argument, "--tabs=", 7) == 0)
        {
            arguments->tabs = &argument[7];
        }

        /* -t <title>, -T <title>, --title <title>
         * The -T form is demanded by distros who insist on this xterm feature. */
        else if (((strcmp(argument, "--title") == 0) || (strcmp(argument, "-t") == 0) || (strcmp(argument, "-T") == 0))
        && (argc > 1))
        {
            argc --;
            argv_cursor ++;
            arguments->title = *argv_cursor;
        }

        /* --working-directory=<working directory> */
        else if (strncmp(argument, "--working-directory=", 20) == 0)
        {
            arguments->working_directory = &argument[20];
        }

        /* Undefined argument. */
        else
            return FALSE;

        argc --;
        argv_cursor ++;
    }

    /* Handle --loginshell. */
    if (login_shell)
    {
        if (arguments->command == NULL)
        {
            arguments->command = g_strdup("sh -l");
        }
        else
        {
            gchar * escaped_command = g_shell_quote(arguments->command);
            gchar * new_command = g_strdup_printf("sh -l -c %s", escaped_command);
            g_free(escaped_command);
            g_free(arguments->command);
            arguments->command = new_command;
        }
    }
    return TRUE;
}

/* Initialize a new LXTerminal.
 * This is a toplevel window that may contain tabs, each of which will contain a VTE controlled by a process. */
LXTerminal * lxterminal_initialize(LXTermWindow * lxtermwin, CommandArguments * arguments)
{
    /* Allocate and initialize the LXTerminal structure. */
    LXTerminal * terminal = g_slice_new0(LXTerminal);
    terminal->parent = lxtermwin;
    terminal->terms = g_ptr_array_new();
    terminal->fixed_size = TRUE;
    g_ptr_array_add(lxtermwin->windows, terminal);
    terminal->index = terminal->parent->windows->len - 1;
    Setting * setting = get_setting();

    /* Create toplevel window. */
    terminal->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Try to get an RGBA visual (colormap) and assign it to the new window. */
    #if GTK_CHECK_VERSION (2, 90, 8)
        GdkVisual *visual = gdk_screen_get_rgba_visual(gtk_widget_get_screen(GTK_WIDGET(terminal->window)));
        if (visual != NULL)
        {
            gtk_widget_set_visual(terminal->window, visual);
        }
    #else
        GdkColormap *colormap = gdk_screen_get_rgba_colormap(gtk_widget_get_screen(GTK_WIDGET(terminal->window)));
        if (colormap != NULL)
        {
            gtk_widget_set_colormap(terminal->window, colormap);
        }
    #endif

    /* Set window title. */
    gtk_window_set_title(GTK_WINDOW(terminal->window), 
        ((arguments->title != NULL) ? arguments->title : _("LXTerminal")));

    /* Set window icon. */
    if (gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), "lxterminal"))
    {
        gtk_window_set_icon_name(GTK_WINDOW(terminal->window), "lxterminal");
    }
    else
    {
        gtk_window_set_icon_from_file(GTK_WINDOW(terminal->window), 
            PACKAGE_DATA_DIR "/icons/hicolor/128x128/apps/lxterminal.png", NULL);
    }
    g_object_weak_ref(G_OBJECT(terminal->window), (GWeakNotify) terminal_window_exit, terminal);

    /* Create a vertical box as the child of the toplevel window. */
    terminal->box = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(terminal->window), terminal->box);

    /* Create the menu bar as the child of the vertical box. */
    terminal_menubar_initialize(terminal);
    gtk_widget_set_no_show_all(GTK_WIDGET(terminal->menu), TRUE);
    if(setting->hide_menu_bar)
        gtk_widget_hide(GTK_WIDGET(terminal->menu));
    gtk_box_pack_start(GTK_BOX(terminal->box), terminal->menu, FALSE, TRUE, 0);

    /* Create a notebook as the child of the vertical box. */
    terminal->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(terminal->notebook), TRUE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(terminal->notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(terminal->box), terminal->notebook, TRUE, TRUE, 0);

    /* Initialize tab position. */
    terminal->tab_position = terminal_tab_get_position_id(setting->tab_position);

    /* Connect signals. */
    g_signal_connect_swapped(G_OBJECT(terminal->window), "composited-changed", 
        G_CALLBACK(terminal_settings_apply), terminal);
    g_signal_connect(G_OBJECT(terminal->notebook), "switch-page", 
        G_CALLBACK(terminal_switch_page_event), terminal);

    /* Create the first terminal. */
    gchar * local_working_directory = NULL;
    if (arguments->working_directory == NULL)
    {
        local_working_directory = g_get_current_dir();
    }
    Term * term = terminal_new(
        terminal,
        _("LXTerminal"),
        ((arguments->working_directory != NULL) ? arguments->working_directory : local_working_directory),
        NULL,
        arguments->command);
    g_free(local_working_directory);

    /* Set the terminal geometry. */
    if ((arguments->geometry_columns != 0) && (arguments->geometry_rows != 0))
    {
        vte_terminal_set_size(VTE_TERMINAL(term->vte), arguments->geometry_columns, arguments->geometry_rows);
    }

    /* Add the first terminal to the notebook and the data structures. */
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->tab);
    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
    g_ptr_array_add(terminal->terms, term);

    /* Show the widget, so it is realized and has a window. */
    gtk_widget_show_all(terminal->window);

    /* Update terminal settings. */
    terminal_settings_apply(terminal);

    /* Initialize the geometry hints. */
    gdk_window_get_geometry_hints(gtk_widget_get_window(GTK_WIDGET(term->vte)), &terminal->geometry, &terminal->geometry_mask);

    if (arguments->tabs != NULL)
    {
        int tab_index = 0;
        char * strings = g_strdup(arguments->tabs);
        /* use token to slice strings to different tab names */
        char * token = strtok(strings, ",");

        while (token != NULL && token[0] != '\0')
        {
            if (tab_index > 0)
            {
                terminal_new_tab_activate_event(0, terminal);
            }

            /* set the name */
            Term * term = g_ptr_array_index(terminal->terms, tab_index);
            term->user_specified_label = TRUE;
            gtk_label_set_text(GTK_LABEL(term->label), g_strdup(token));

            token = strtok(NULL, ",");
            tab_index ++;
        }

        g_free(strings);
    }

    /* Connect signals. */
    g_signal_connect(G_OBJECT(terminal->window), "size-allocate", G_CALLBACK(terminal_window_size_allocate_event), terminal);
    return terminal;
}

/* Apply new settings to a terminal. */
static void terminal_settings_apply(LXTerminal * terminal)
{
    /* Reinitialize "composited". */
    terminal->rgba = gtk_widget_is_composited(terminal->window);

    /* Update tab position. */
    terminal->tab_position = terminal_tab_get_position_id(get_setting()->tab_position);
    terminal_tab_set_position(terminal->notebook, terminal->tab_position);

    /* Update menu accelerators. */
    terminal_menu_accelerator_update(terminal);

    /* disable mnemonics if <ALT>n is diabled */
    g_object_set(gtk_settings_get_default(), "gtk-enable-mnemonics", !get_setting()->disable_alt, NULL);

    /* Hide or show menubar. */
    if (get_setting()->hide_menu_bar)
    {
        gtk_widget_hide(terminal->menu);
    }
    else
    {
        gtk_widget_show(terminal->menu);
    }

    /* Apply settings to all tabs. */
    guint i;
    for (i = 0; i < terminal->terms->len; i++)
    {
        terminal_settings_apply_to_term(terminal, g_ptr_array_index(terminal->terms, i));
    }

    /* update <ALT>n status */
    terminal_update_alt(terminal);
}

/* Apply terminal settings to all tabs in all terminals. */
void terminal_settings_apply_to_all(LXTerminal * terminal)
{
    /* Recreate accelerators, may be changed. */
    terminal_update_menu_shortcuts(get_setting());
    /* Apply settings to all open windows. */
    g_ptr_array_foreach(terminal->parent->windows, (GFunc) terminal_settings_apply, get_setting());
    get_setting()->geometry_change = FALSE;
}

/* Update terminal menu shortcuts. */
void terminal_update_menu_shortcuts(Setting * setting)
{
    guint key;
    GdkModifierType mods;

    gtk_accelerator_parse(setting->new_window_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/File_NewWindow", key, mods, FALSE);
    gtk_accelerator_parse(setting->new_tab_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/File_NewTab", key, mods, FALSE);
    gtk_accelerator_parse(setting->close_tab_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/File_CloseTab", key, mods, FALSE);
    gtk_accelerator_parse(setting->close_window_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/File_CloseWindow", key, mods, FALSE);
    gtk_accelerator_parse(setting->copy_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Edit_Copy", key, mods, FALSE);
    gtk_accelerator_parse(setting->paste_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Edit_Paste", key, mods, FALSE);
    gtk_accelerator_parse(setting->name_tab_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Tabs_NameTab", key, mods, FALSE);
    gtk_accelerator_parse(setting->previous_tab_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Tabs_PreviousTab", key, mods, FALSE);
    gtk_accelerator_parse(setting->next_tab_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Tabs_NextTab", key, mods, FALSE);
    gtk_accelerator_parse(setting->move_tab_left_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Tabs_MoveTabLeft", key, mods, FALSE);
    gtk_accelerator_parse(setting->move_tab_right_accel, &key, &mods);
    gtk_accel_map_change_entry("<Actions>/MenuBar/Tabs_MoveTabRight", key, mods, FALSE);
}

/* Initialize terminal menu shortcuts. */
void terminal_initialize_menu_shortcuts(Setting * setting)
{
    /* Update static array by index. */
    menu_items[4].accelerator = setting->new_window_accel;
    menu_items[5].accelerator = setting->new_tab_accel;
    menu_items[7].accelerator = setting->close_tab_accel;
    menu_items[8].accelerator = setting->close_window_accel;
    menu_items[9].accelerator = setting->copy_accel;
    menu_items[10].accelerator = setting->paste_accel;
    menu_items[14].accelerator = setting->name_tab_accel;
    menu_items[15].accelerator = setting->previous_tab_accel;
    menu_items[16].accelerator = setting->next_tab_accel;
    menu_items[17].accelerator = setting->move_tab_left_accel;
    menu_items[18].accelerator = setting->move_tab_right_accel;
}

/* Main entry point. */
int main(gint argc, gchar * * argv)
{
    /* Initialize GTK. */
    gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    /* Parse the command arguments.  If there is an error, display usage help and exit. */
    CommandArguments arguments;
    if ( ! lxterminal_process_arguments(argc, argv, &arguments))
    {
        printf("%s\n", usage_display);
        return 0;
    }

    /* Initialize impure storage. */
    LXTermWindow * lxtermwin = g_slice_new0(LXTermWindow);

    /* Initialize socket.  If we were able to get another LXTerminal to manage the window, exit. */
    if ( ! lxterminal_socket_initialize(lxtermwin, &arguments))
        return 0;

    /* Load user preferences. */
    load_setting();
    
    /* Finish initializing the impure area and start the first LXTerminal. */
    lxtermwin->windows = g_ptr_array_new();
    lxterminal_initialize(lxtermwin, &arguments);

    /* Run the main loop. */
    gtk_main();

    return 0;
}


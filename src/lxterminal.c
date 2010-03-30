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

/* Menu and accelerator event handlers. */
static void terminal_initialize_switch_tab_accelerator(Term * term);
static void terminal_switch_tab_accelerator(Term * term);
static void terminal_new_window_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_new_window_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_new_tab_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_new_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_close_tab_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_close_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_copy_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_copy_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_paste_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_paste_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_name_tab_response_event(GtkWidget * dialog, gint response, LXTerminal * terminal);
static void terminal_name_tab_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_name_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_previous_tab_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_previous_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_next_tab_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_next_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_move_tab_execute(LXTerminal * terminal, gint direction);
static void terminal_move_tab_left_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_move_tab_left_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_move_tab_right_activate_event(GtkAction * action, LXTerminal * terminal);
static void terminal_move_tab_right_accelerator(LXTerminal * terminal, guint action, GtkWidget * item);
static void terminal_about_activate_event(GtkAction * action, LXTerminal * terminal);

/* Window creation, destruction, and control. */
static gboolean terminal_window_size_request_event(GtkWidget * widget, GtkRequisition * requisition, LXTerminal * terminal);
static void terminal_window_set_fixed_size(LXTerminal * terminal);
static void terminal_switch_page_event(GtkNotebook * notebook, GtkNotebookPage * page, guint num, LXTerminal * terminal);
static void terminal_window_title_changed_event(GtkWidget * vte, Term * term);
static void terminal_window_exit(LXTerminal * terminal, GObject * where_the_object_was);
static void terminal_child_exited_event(VteTerminal * vte, Term * term);
static gboolean terminal_tab_button_press_event(GtkWidget * widget, GdkEventButton * event, Term * term);
static gboolean terminal_vte_button_press_event(VteTerminal * vte, GdkEventButton * event, Term * term);
static void terminal_settings_apply_to_term(LXTerminal * terminal, Term * term);
static Term * terminal_new(LXTerminal * terminal, const gchar * label, const gchar * pwd, gchar * * env, const gchar * exec);
static void terminal_free(Term * term);
static void terminal_menubar_initialize(LXTerminal * terminal);
static void terminal_accelerator_initialize(LXTerminal * terminal);
static void terminal_menu_accelerator_update(LXTerminal * terminal);
static void terminal_settings_apply(LXTerminal * terminal);

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
	"  -t, -T, --title=STRING           Set the terminal's title\n"
	"  --working-directory=DIRECTORY    Set the terminal's working directory\n"
};

/* Descriptors for menu top level. */
static GtkActionEntry menus[] =
{
	{ "File", NULL, N_("_File") },
	{ "Edit", NULL, N_("_Edit") },
	{ "Tabs", NULL, N_("_Tabs") },
	{ "Help", NULL, N_("_Help") }
};
#define MENUBAR_MENU_COUNT G_N_ELEMENTS(menus)

/* Descriptors for menu bar items. */
static GtkActionEntry menu_items[] =
{
	{ "File_NewWindow", GTK_STOCK_ADD, N_("New _Window"), NEW_WINDOW_ACCEL, "New Window", G_CALLBACK(terminal_new_window_activate_event) },
	{ "File_NewTab", GTK_STOCK_ADD, N_("New _Tab"), NEW_TAB_ACCEL, "New Tab", G_CALLBACK(terminal_new_tab_activate_event) },
	{ "File_Sep1", NULL, "Sep" },
	{ "File_CloseTab", GTK_STOCK_CLOSE, N_("_Close Tab"), CLOSE_TAB_ACCEL, "Close Tab", G_CALLBACK(terminal_close_tab_activate_event) },
	{ "File_Quit", GTK_STOCK_QUIT, N_("_Quit"), QUIT_ACCEL, "Quit", G_CALLBACK(gtk_main_quit) },
	{ "Edit_Copy", GTK_STOCK_COPY, N_("Cop_y"), COPY_ACCEL, "Copy", G_CALLBACK(terminal_copy_activate_event) },
	{ "Edit_Paste", GTK_STOCK_PASTE, N_("_Paste"), PASTE_ACCEL, "Paste", G_CALLBACK(terminal_paste_activate_event) },
	{ "Edit_Sep1", NULL, "Sep" },
	{ "Edit_Preferences", GTK_STOCK_EXECUTE, N_("Preference_s"), NULL, "Preferences", G_CALLBACK(terminal_preferences_dialog) },
	{ "Tabs_NameTab", GTK_STOCK_INFO, N_("Na_me Tab"), NAME_TAB_ACCEL, "Name Tab", G_CALLBACK(terminal_name_tab_activate_event) },
	{ "Tabs_PreviousTab", GTK_STOCK_GO_BACK, N_("Pre_vious Tab"), PREVIOUS_TAB_ACCEL, "Previous Tab", G_CALLBACK(terminal_previous_tab_activate_event) },
	{ "Tabs_NextTab", GTK_STOCK_GO_FORWARD, N_("Ne_xt Tab"), NEXT_TAB_ACCEL, "Next Tab", G_CALLBACK(terminal_next_tab_activate_event) },
	{ "Tabs_MoveTabLeft", NULL, N_("Move Tab _Left"), MOVE_TAB_LEFT_ACCEL, "Move Tab Left", G_CALLBACK(terminal_move_tab_left_activate_event) },
	{ "Tabs_MoveTabRight", NULL, N_("Move Tab _Right"), MOVE_TAB_RIGHT_ACCEL, "Move Tab Right", G_CALLBACK(terminal_move_tab_right_activate_event) },
	{ "Help_About", GTK_STOCK_ABOUT, N_("_About"), NULL, "About", G_CALLBACK(terminal_about_activate_event) }
};
#define MENUBAR_MENUITEM_COUNT G_N_ELEMENTS(menu_items)

/* Descriptors for popup menu items, accessed via right click on the terminal. */
static GtkActionEntry vte_menu_items[] =
{
	{ "VTEMenu", NULL, "VTEMenu" },
	{ "NewWindow", GTK_STOCK_ADD, N_("New _Window"), NULL, "New Window", G_CALLBACK(terminal_new_window_activate_event) },
	{ "NewTab", GTK_STOCK_ADD, N_("New _Tab"), NULL, "New Tab", G_CALLBACK(terminal_new_tab_activate_event) },
	{ "Sep1", NULL, "Sep" },
	{ "Copy", GTK_STOCK_COPY, N_("Cop_y"), NULL, "Copy", G_CALLBACK(terminal_copy_activate_event) },
	{ "Paste", GTK_STOCK_PASTE, N_("_Paste"), NULL, "Paste", G_CALLBACK(terminal_paste_activate_event) },
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

    if (GDK_WINDOW_DESTROYED(window))
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
        border->left + VTE_TERMINAL(term->vte)->char_width,
        border->top + VTE_TERMINAL(term->vte)->char_height);
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
        gtk_accel_group_connect(term->parent->accel_group, key, mods, GTK_ACCEL_LOCKED, term->closure);
    }
}

/* Handler for accelerator <ALT> n, where n is a digit.
 * Switch to the tab selected by the digit, if it exists. */
static void terminal_switch_tab_accelerator(Term * term)
{
    LXTerminal * terminal = term->parent;
    if (term->index < gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)))
        gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), term->index);
}

/* Handler for "activate" signal on File/New Window menu item.
 * Open a new window. */
static void terminal_new_window_activate_event(GtkAction * action, LXTerminal * terminal)
{
    CommandArguments arguments;
    memset(&arguments, 0, sizeof(arguments));
    lxterminal_initialize(terminal->parent, &arguments, terminal->setting);
}

/* Handler for accelerator <SHIFT><CTRL> N.  Open a new window. */
static void terminal_new_window_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_new_window_activate_event(NULL, terminal);
}

/* Handler for "activate" signal on File/New Tab menu item.
 * Open a new tab. */
static void terminal_new_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    gchar * proc_cwd = NULL;

#ifdef __linux
    /* Try to get the working directory from /proc. */
    gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook));
    if (current != -1)
    {
        /* Search for the Term structure corresponding to the current tab. */
        int i;
        for (i = 0; i < terminal->terms->len; i += 1)
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

    /* Propagate the working directory of the current tab to the new tab.
     * If the working directory was determined above, use it; otherwise default to the working directory of the process.
     * Create the new terminal. */
    if (proc_cwd == NULL)
        proc_cwd = g_get_current_dir();
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
    terminal_initialize_switch_tab_accelerator(term);
}

/* Handler for accelerator <SHIFT><CTRL> T.  Open a new tab. */
static void terminal_new_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_new_tab_activate_event(NULL, terminal);
}

/* Handler for "activate" signal on File/Close Tab menu item.
 * Close the current tab. */
static void terminal_close_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    terminal_child_exited_event(VTE_TERMINAL(term->vte), term);
}

/* Handler for accelerator <SHIFT><CTRL> W.  Close the current tab. */
static void terminal_close_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_close_tab_activate_event(NULL, terminal);
}

/* Handler for "activate" signal on Edit/Copy menu item.
 * Copy to the clipboard. */
static void terminal_copy_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

/* Handler for accelerator <CTRL><SHIFT> C.  Copy to the clipboard. */
static void terminal_copy_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_copy_activate_event(NULL, terminal);
}

/* Handler for "activate" signal on Edit/Paste menu item.
 * Paste from the clipboard. */
static void terminal_paste_activate_event(GtkAction * action, LXTerminal * terminal)
{
    Term * term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));
    vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

/* Handler for accelerator <CTRL><SHIFT> V.  Paste from the clipboard. */
static void terminal_paste_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_paste_activate_event(NULL, terminal);
}

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
            int i;
            for (i = 0; i < terminal->terms->len; i += 1)
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
    gtk_window_set_icon_from_file(GTK_WINDOW(dialog), PACKAGE_DATA_DIR "/pixmaps/lxterminal.png", NULL);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(terminal_name_tab_response_event), terminal);
    GtkWidget * dialog_item = gtk_entry_new();
    g_object_set_data(G_OBJECT(dialog), "entry", (gpointer) dialog_item);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), dialog_item, FALSE, FALSE, 2);
    gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook));
    if (current != -1)
    {
        /* Search for the Term structure corresponding to the current tab. */
        int i;
        for (i = 0; i < terminal->terms->len; i += 1)
        {
            Term * term = g_ptr_array_index(terminal->terms, i);
            if (term->index == current)
            {
                gtk_entry_set_text(GTK_ENTRY(dialog_item), gtk_label_get_text(GTK_LABEL(term->label)));
                break;
            }
        }
    }        
    gtk_widget_show_all(dialog);
}

/* Handler for accelerator <CTRL><SHIFT> R.  Name the tab. */
static void terminal_name_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_name_tab_activate_event(NULL, terminal);
}

/* Handler for "activate" signal on Tabs/Previous Tab menu item.
 * Cycle through tabs in the reverse direction. */
static void terminal_previous_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    /* Cycle through tabs. */
    if (gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)) == 0)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), -1);
    else
        gtk_notebook_prev_page(GTK_NOTEBOOK(terminal->notebook));
}

/* Handler for accelerator <CTRL><PAGE UP>.  Cycle through tabs in the reverse direction. */
static void terminal_previous_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_previous_tab_activate_event(NULL, terminal);
}

/* Handler for "activate" signal on Tabs/Next Tab menu item.
 * Cycle through tabs in the forward direction. */
static void terminal_next_tab_activate_event(GtkAction * action, LXTerminal * terminal)
{
    /* Cycle through tabs. */
    if (gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)) == gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), 0);
    else
        gtk_notebook_next_page(GTK_NOTEBOOK(terminal->notebook));
}

/* Handler for accelerator <CTRL><PAGE DOWN>.  Cycle through tabs in the forward direction. */
static void terminal_next_tab_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_next_tab_activate_event(NULL, terminal);
}

/* Helper for move tab left and right. */
static void terminal_move_tab_execute(LXTerminal * terminal, gint direction)
{
    GtkNotebook * notebook = GTK_NOTEBOOK(terminal->notebook);
    gint current_page_number = gtk_notebook_get_current_page(notebook);
    gtk_notebook_reorder_child(notebook, gtk_notebook_get_nth_page(notebook, current_page_number), current_page_number + direction);
}

/* Handler for "activate" signal on Tabs/Move Tab Left menu item.
 * Move the tab one position in the reverse direction. */
static void terminal_move_tab_left_activate_event(GtkAction * action, LXTerminal * terminal)
{
    terminal_move_tab_execute(terminal, -1);
}

/* Handler for accelerator <SHIFT><CTRL><PAGE UP>.  Move the tab one position in the reverse direction. */
static void terminal_move_tab_left_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_move_tab_execute(terminal, -1);
}

/* Handler for "activate" signal on Tabs/Move Tab Right menu item.
 * Move the tab one position in the forward direction. */
static void terminal_move_tab_right_activate_event(GtkAction * action, LXTerminal * terminal)
{
    terminal_move_tab_execute(terminal, 1);
}

/* Handler for accelerator <SHIFT><CTRL><PAGE DOWN>.  Move the tab one position in the forward direction. */
static void terminal_move_tab_right_accelerator(LXTerminal * terminal, guint action, GtkWidget * item)
{
    terminal_move_tab_execute(terminal, 1);
}

/* Handler for "activate" signal on Help/About menu item. */ 
static void terminal_about_activate_event(GtkAction * action, LXTerminal * terminal)
{
    const gchar * authors[] =
    {
        "Fred Chien <cfsghost@gmail.com>",
        "Marty Jack <martyj19@comcast.net>",
        NULL
    };
    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar * translators = _("translator-credits");

    /* Create and initialize the dialog. */
    GtkWidget * about_dlg = gtk_about_dialog_new();
    gtk_about_dialog_set_url_hook(NULL, NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(about_dlg), 2);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dlg), VERSION);
    gtk_about_dialog_set_name (GTK_ABOUT_DIALOG(about_dlg), _("LXTerminal"));
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dlg), gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR "/pixmaps/lxterminal.png", NULL));
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dlg), _("Copyright (C) 2008-2010"));
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
static gboolean terminal_window_size_request_event(GtkWidget * widget, GtkRequisition * requisition, LXTerminal * terminal)
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
        hints.width_inc = VTE_TERMINAL(term->vte)->char_width;
        hints.height_inc = VTE_TERMINAL(term->vte)->char_height;
        hints.base_width = border->left;
        hints.base_height = border->top;
        hints.min_width = hints.base_width + hints.width_inc * 4;
        hints.min_height = hints.base_height + hints.height_inc * 2;
        gtk_border_free(border);

        /* Set hints into all terminals.  This makes sure we resize on character cell boundaries. */
        int i;
        for (i = 0; i < terminal->terms->len; i += 1)
        {
            Term * term = g_ptr_array_index(terminal->terms, i);
            gtk_window_set_geometry_hints(GTK_WINDOW(terminal->window),
	        term->vte,
	        &hints,
                GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);
        }

        /* Resize the window. */
        gtk_window_resize(GTK_WINDOW(terminal->window), requisition->width, requisition->height);
    }
    return FALSE;
}

/* Set geometry hints for the fixed size case. */
static void terminal_window_set_fixed_size(LXTerminal * terminal)
{
    terminal->fixed_size = TRUE;
    int i;
    for (i = 0; i < terminal->terms->len; i += 1)
    {
        Term * term = g_ptr_array_index(terminal->terms, i);
        gtk_window_set_geometry_hints(GTK_WINDOW(terminal->window),
            term->vte,
            &terminal->geometry,
            terminal->geometry_mask);
    }
}

/* Handler for "switch-page" event on the tab notebook. */
static void terminal_switch_page_event(GtkNotebook * notebook, GtkNotebookPage * page, guint num, LXTerminal * terminal)
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
	int i;
        for (i = terminal->index; i < terminal->parent->windows->len; i += 1)
        {
            LXTerminal * t = g_ptr_array_index(terminal->parent->windows, i);
            t->index -= 1;
        }
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
        int i;
        for (i = term->index; i < terminal->terms->len; i++)
        {
            Term * t = g_ptr_array_index(terminal->terms, i);
            t->index -= 1;
        }

        /* Delete the tab and free the Term structure. */
        gtk_notebook_remove_page(GTK_NOTEBOOK(terminal->notebook), term->index);
        terminal_free(term);

        /* If only one page is left, hide the tab and correct the geometry. */
        if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) == 1)
        {
            gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);
            terminal_geometry_restore(g_ptr_array_index(terminal->terms, 0));
        }
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

/* Handler for "button-press-event" signal on VTE. */
static gboolean terminal_vte_button_press_event(VteTerminal * vte, GdkEventButton * event, Term * term)
{
    /* Right-click. */
    if (event->button == 3)
    {
        /* Generate popup menu. */
        GtkUIManager * manager = gtk_ui_manager_new();
        GtkActionGroup * action_group = gtk_action_group_new("VTEMenu");
        gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
        gtk_action_group_add_actions(action_group, vte_menu_items, VTE_MENUITEM_COUNT, term->parent);
        gtk_ui_manager_insert_action_group(manager, action_group, 0);

        guint merge_id = gtk_ui_manager_new_merge_id(manager);
        gtk_ui_manager_add_ui(manager, merge_id, "/", "VTEMenu", NULL, GTK_UI_MANAGER_POPUP, FALSE);

        int i;
        for (i = 1; i < VTE_MENUITEM_COUNT; i += 1)
        {
            if (strcmp(vte_menu_items[i].label, "Sep") == 0)
                gtk_ui_manager_add_ui(manager, merge_id, "/VTEMenu", vte_menu_items[i].name, NULL, GTK_UI_MANAGER_SEPARATOR, FALSE);
                else gtk_ui_manager_add_ui(manager, merge_id, "/VTEMenu", vte_menu_items[i].name, vte_menu_items[i].name, GTK_UI_MANAGER_MENUITEM, FALSE);
        }
        gtk_menu_popup(GTK_MENU(gtk_ui_manager_get_widget(manager, "/VTEMenu")), NULL, NULL, NULL, NULL, event->button, event->time);
    }

    /* Control left click. */
    else if ((event->button == 1) && (event->state & GDK_CONTROL_MASK))
    {
        /* steal from tilda-0.09.6/src/tilda_terminal.c:743
         * See if the terminal has matched the regular expression. */
        GtkBorder * border = terminal_get_border(term);
        gint tag;
        gchar * match = vte_terminal_match_check(vte,
            (event->x - border->left) / vte->char_width,
            (event->y - border->top) / vte->char_height,
            &tag);
        gtk_border_free(border);

        /* Launch xdg-open with the match string. */
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

/* Apply new settings in an LXTerminal to its tab Term. */
static void terminal_settings_apply_to_term(LXTerminal * terminal, Term * term)
{
    Setting * setting = terminal->setting;

    /* Terminal properties. */
    vte_terminal_reset(VTE_TERMINAL(term->vte), FALSE, FALSE);
    vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), setting->font_name);
    vte_terminal_set_word_chars(VTE_TERMINAL(term->vte), setting->word_selection_characters);
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), setting->scrollback);
    vte_terminal_set_allow_bold(VTE_TERMINAL(term->vte), ! setting->disallow_bold);
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term->vte), ((setting->cursor_blink) ? VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF));
    vte_terminal_set_cursor_shape(VTE_TERMINAL(term->vte), ((setting->cursor_underline) ? VTE_CURSOR_SHAPE_UNDERLINE : VTE_CURSOR_SHAPE_BLOCK));
    vte_terminal_set_audible_bell(VTE_TERMINAL(term->vte), setting->audible_bell);

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
        gtk_widget_hide(term->scrollbar);
        else gtk_widget_show(term->scrollbar);

    /* Hide or show Close button. */
    if (setting->hide_close_button)
        gtk_widget_hide(term->close_button);
        else gtk_widget_show(term->close_button);

    /* If terminal geometry changed, react to it. */
    if (setting->geometry_change)
        terminal_geometry_restore(term);
}

/* Create a new terminal. */
static Term * terminal_new(LXTerminal * terminal, const gchar * label, const gchar * pwd, gchar * * env, const gchar * exec)
{
    /* Create and initialize Term structure for new terminal. */
    Term * term = g_new0(Term, 1);
    term->parent = terminal;

    /* Create a VTE and a vertical scrollbar, and place them inside a horizontal box. */
    term->vte = vte_terminal_new();
    term->box = gtk_hbox_new(FALSE, 0);
    term->scrollbar = gtk_vscrollbar_new(NULL);
    gtk_box_pack_start(GTK_BOX(term->box), term->vte, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(term->box), term->scrollbar, FALSE, TRUE, 0);

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
    gtk_rc_style_unref(rcstyle),

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
    gtk_range_set_adjustment(GTK_RANGE(term->scrollbar), VTE_TERMINAL(term->vte)->adjustment);

    /* Fork the process that will have the VTE as its controlling terminal. */
    if (exec != NULL)
    {
        gchar * * command;
        g_shell_parse_argv(exec, NULL, &command, NULL);
        term->pid = vte_terminal_fork_command(VTE_TERMINAL(term->vte), (const char *) command[0], command, env, pwd, FALSE, TRUE, TRUE);
        g_strfreev(command);
    }
    else
        term->pid = vte_terminal_fork_command(VTE_TERMINAL(term->vte), NULL, NULL, env, pwd, FALSE, TRUE, TRUE);

    /* Connect signals. */
    g_signal_connect(G_OBJECT(term->tab), "button-press-event", G_CALLBACK(terminal_tab_button_press_event), term);
    g_signal_connect(G_OBJECT(term->close_button), "clicked", G_CALLBACK(terminal_child_exited_event), term);
    g_signal_connect(G_OBJECT(term->vte), "button-press-event", G_CALLBACK(terminal_vte_button_press_event), term);
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
    gtk_accel_group_disconnect(term->parent->accel_group, term->closure);
    g_free(term);
}

/* Initialize the menu bar. */
static void terminal_menubar_initialize(LXTerminal * terminal)
{
    /* Initialize UI manager. */
    GtkUIManager * manager = gtk_ui_manager_new();
    GtkActionGroup * action_group = gtk_action_group_new("MenuBar");
    gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions(action_group, menus, MENUBAR_MENU_COUNT, terminal);
    gtk_action_group_add_actions(action_group, menu_items, MENUBAR_MENUITEM_COUNT, terminal);
    gtk_ui_manager_insert_action_group(manager, action_group, 0);

    guint merge_id = gtk_ui_manager_new_merge_id(manager);
    gtk_ui_manager_add_ui(manager, merge_id, "/", "MenuBar", NULL, GTK_UI_MANAGER_MENUBAR, FALSE);

    /* Menus. */
    int i;
    for (i = 0; i < MENUBAR_MENU_COUNT; i += 1)
    {
        gchar * path = g_strdup_printf("/MenuBar/%s", menus[i].name);
        gchar * path_ptr;
        for (path_ptr = path; *path_ptr != '\0'; path_ptr += 1)
        {
            if (*path_ptr == '_')
                *path_ptr = '/';
        }
        path_ptr = g_path_get_dirname(path);
        gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menus[i].name, menus[i].name, GTK_UI_MANAGER_MENU, FALSE);
        g_free(path);
        g_free(path_ptr);
    }

    /* Items. */
    for (i = 0; i < MENUBAR_MENUITEM_COUNT; i += 1)
    {
        gchar * path = g_strdup_printf("/MenuBar/%s", menu_items[i].name);
        gchar * path_ptr;
        for (path_ptr = path; *path_ptr != '\0'; path_ptr += 1)
        {
            if (*path_ptr == '_')
                *path_ptr = '/';
        }
        path_ptr = g_path_get_dirname(path);
        if (strcmp(menu_items[i].label, "Sep") == 0)
            gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menu_items[i].name, NULL, GTK_UI_MANAGER_SEPARATOR, FALSE);
            else gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menu_items[i].name, menu_items[i].name, GTK_UI_MANAGER_MENUITEM, FALSE);

        g_free(path);
        g_free(path_ptr);
    }

    terminal->menu = gtk_ui_manager_get_widget(manager, "/MenuBar");
}

/* Allocate, initialize, and connect up an accelerator group for the accelerators on a new terminal. */
static void terminal_accelerator_initialize(LXTerminal * terminal)
{
    guint key;
    GdkModifierType mods;

    terminal->accel_group = gtk_accel_group_new();

    gtk_accelerator_parse(NEW_WINDOW_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_new_window_accelerator), terminal, NULL));

    gtk_accelerator_parse(QUIT_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(gtk_main_quit), NULL, NULL));

    gtk_accelerator_parse(NEW_TAB_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_new_tab_accelerator), terminal, NULL));

    gtk_accelerator_parse(CLOSE_TAB_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_close_tab_accelerator), terminal, NULL));

    gtk_accelerator_parse(COPY_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_copy_accelerator), terminal, NULL));

    gtk_accelerator_parse(PASTE_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_paste_accelerator), terminal, NULL));

    gtk_accelerator_parse(NAME_TAB_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_name_tab_accelerator), terminal, NULL));

    gtk_accelerator_parse(PREVIOUS_TAB_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_previous_tab_accelerator), terminal, NULL));

    gtk_accelerator_parse(NEXT_TAB_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_next_tab_accelerator), terminal, NULL));

    gtk_accelerator_parse(MOVE_TAB_LEFT_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_move_tab_left_accelerator), terminal, NULL));

    gtk_accelerator_parse(MOVE_TAB_RIGHT_ACCEL, &key, &mods);
    gtk_accel_group_connect(terminal->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_move_tab_right_accelerator), terminal, NULL));

    gtk_window_add_accel_group(GTK_WINDOW(terminal->window), terminal->accel_group);
}

/* Update the accelerator that brings up the menu.
 * We have a user preference as to whether F10 (or a style-supplied alternate) is used for this purpose.
 * Technique taken from gnome-terminal. */
static void terminal_menu_accelerator_update(LXTerminal * terminal)
{
    /* Ensure that saved_menu_accelerator is initialized. */
    if (saved_menu_accelerator == NULL)
        g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-menu-bar-accel", &saved_menu_accelerator, NULL);

    /* If F10 is disabled, set the accelerator to a key combination that is not F10 and unguessable. */
    gtk_settings_set_string_property(
        gtk_settings_get_default(),
        "gtk-menu-bar-accel",
        ((terminal->setting->disable_f10) ? "<Shift><Control><Mod1><Mod2><Mod3><Mod4><Mod5>F10" : saved_menu_accelerator),
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
    argc -= 1;
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
                argc -= 1;
                argv_cursor += 1;
                if (arguments->command == NULL)
                    arguments->command = g_strdup(*argv_cursor);
                else
                {
                    gchar * new_command = g_strconcat(arguments->command, " ", *argv_cursor, NULL);
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
                return FALSE;
        }

        /* -l, --loginshell */
        else if ((strcmp(argument, "--loginshell") == 0) || (strcmp(argument, "-l") == 0))
            login_shell = TRUE;

        /* --title=<title> */
	else if (strncmp(argument, "--title=", 8) == 0)
	    arguments->title = &argument[8];

        /* -t <title>, -T <title>, --title <title>
         * The -T form is demanded by distros who insist on this xterm feature. */
        else if (((strcmp(argument, "--title") == 0) || (strcmp(argument, "-t") == 0) || (strcmp(argument, "-T") == 0))
        && (argc > 1))
        {
            argc -= 1;
            argv_cursor += 1;
            arguments->title = *argv_cursor;
        }

        /* --working-directory=<working directory> */
        else if (strncmp(argument, "--working-directory=", 20) == 0)
            arguments->working_directory = &argument[20];

        /* Undefined argument. */
        else
            return FALSE;

        argc -= 1;
        argv_cursor += 1;
    }

    /* Handle --loginshell. */
    if (login_shell)
    {
        if (arguments->command == NULL)
            arguments->command = g_strdup("sh -l");
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
LXTerminal * lxterminal_initialize(LXTermWindow * lxtermwin, CommandArguments * arguments, Setting * setting)
{
    /* Allocate and initialize the LXTerminal structure. */
    LXTerminal * terminal = g_new0(LXTerminal, 1);
    terminal->parent = lxtermwin;
    terminal->terms = g_ptr_array_new();
    terminal->fixed_size = TRUE;
    g_ptr_array_add(lxtermwin->windows, terminal);
    terminal->index = terminal->parent->windows->len - 1;
    terminal->setting = setting;

    /* Create toplevel window. */
    terminal->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Try to get an RGBA colormap and assign it to the new window. */
    GdkColormap * colormap = gdk_screen_get_rgba_colormap(gtk_widget_get_screen(GTK_WIDGET(terminal->window)));
    if (colormap != NULL)
        gtk_widget_set_colormap(terminal->window, colormap);

    /* Set window title. */
    gtk_window_set_title(GTK_WINDOW(terminal->window), ((arguments->title != NULL) ? arguments->title : _("LXTerminal")));

    /* Set window icon. */
    gtk_window_set_icon_from_file(GTK_WINDOW(terminal->window), PACKAGE_DATA_DIR "/pixmaps/lxterminal.png", NULL);
    g_object_weak_ref(G_OBJECT(terminal->window), (GWeakNotify) terminal_window_exit, terminal);

    /* Create a vertical box as the child of the toplevel window. */
    terminal->box = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(terminal->window), terminal->box);

    /* Create the menu bar as the child of the vertical box. */
    terminal_menubar_initialize(terminal);
    gtk_box_pack_start(GTK_BOX(terminal->box), terminal->menu, FALSE, TRUE, 0);

    /* Create a notebook as the child of the vertical box. */
    terminal->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(terminal->notebook), TRUE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(terminal->notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(terminal->box), terminal->notebook, TRUE, TRUE, 0);

    /* Initialize tab position. */
    terminal->tab_position = terminal_tab_get_position_id(terminal->setting->tab_position);

    /* Connect signals. */
    g_signal_connect_swapped(G_OBJECT(terminal->window), "composited-changed", G_CALLBACK(terminal_settings_apply), terminal);
    g_signal_connect(G_OBJECT(terminal->notebook), "switch-page", G_CALLBACK(terminal_switch_page_event), terminal);

    /* Create the first terminal. */
    gchar * local_working_directory = NULL;
    if (arguments->working_directory == NULL)
        local_working_directory = g_get_current_dir();
    Term * term = terminal_new(
        terminal,
        _("LXTerminal"),
        ((arguments->working_directory != NULL) ? arguments->working_directory : local_working_directory),
        NULL,
        arguments->command);
    g_free(local_working_directory);

    /* Set the terminal geometry. */
    if ((arguments->geometry_columns != 0) && (arguments->geometry_rows != 0))
        vte_terminal_set_size(VTE_TERMINAL(term->vte), arguments->geometry_columns, arguments->geometry_rows);

    /* Add the first terminal to the notebook and the data structures. */
    gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->tab);
    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
    g_ptr_array_add(terminal->terms, term);

    /* Initialize accelerators. */
    terminal_accelerator_initialize(terminal);
    terminal_initialize_switch_tab_accelerator(term);

    /* Update terminal settings. */
    terminal_settings_apply(terminal);

    /* Show the widget, so it is realized and has a window. */
    gtk_widget_show_all(terminal->window);

    /* Initialize the geometry hints. */
    gdk_window_get_geometry_hints(GTK_WIDGET(term->vte)->window, &terminal->geometry, &terminal->geometry_mask);

    /* Connect signals. */
    g_signal_connect(G_OBJECT(terminal->window), "size-request", G_CALLBACK(terminal_window_size_request_event), terminal);
    return terminal;
}

/* Apply new settings to a terminal. */
static void terminal_settings_apply(LXTerminal * terminal)
{
    /* Reinitialize "composited". */
    terminal->rgba = gtk_widget_is_composited(terminal->window);

    /* Apply settings to all windows. */
    int i;
    for (i = 0; i < terminal->terms->len; i += 1)
        terminal_settings_apply_to_term(terminal, g_ptr_array_index(terminal->terms, i));

    /* Update tab position. */
    terminal->tab_position = terminal_tab_get_position_id(terminal->setting->tab_position);
    terminal_tab_set_position(terminal->notebook, terminal->tab_position);

    /* Update menu accelerators. */
    terminal_menu_accelerator_update(terminal);

    /* Hide or show menubar. */
    if (terminal->setting->hide_menu_bar)
        gtk_widget_hide(terminal->menu);
        else gtk_widget_show(terminal->menu);
}

/* Apply terminal settings to all tabs in all terminals. */
void terminal_settings_apply_to_all(LXTerminal * terminal)
{
    /* Apply settings to all open windows. */
    g_ptr_array_foreach(terminal->parent->windows, (GFunc) terminal_settings_apply, terminal->setting);
    terminal->setting->geometry_change = FALSE;
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
    LXTermWindow * lxtermwin = g_new0(LXTermWindow, 1);

    /* Initialize socket.  If we were able to get another LXTerminal to manage the window, exit. */
    if ( ! lxterminal_socket_initialize(lxtermwin, &arguments))
        return 0;

    /* Load user preferences. */
    gchar * dir = g_build_filename(g_get_user_config_dir(), "lxterminal" , NULL);
    g_mkdir_with_parents(dir, S_IRUSR | S_IWUSR | S_IXUSR);
    gchar * path = g_build_filename(dir, "lxterminal.conf", NULL);
    g_free(dir);

    if ( ! g_file_test(path, G_FILE_TEST_EXISTS))
    {
        /* Copy the system-wide settings to the user's configuration. */
        lxtermwin->setting = load_setting_from_file(PACKAGE_DATA_DIR "/lxterminal/lxterminal.conf");
        setting_save(lxtermwin->setting);
    }
    else
        lxtermwin->setting = load_setting_from_file(path);

    g_free(path);

    /* Finish initializing the impure area and start the first LXTerminal. */
    lxtermwin->windows = g_ptr_array_new();
    lxterminal_initialize(lxtermwin, &arguments, lxtermwin->setting);

    /* Run the main loop. */
    gtk_main();

    return 0;
}


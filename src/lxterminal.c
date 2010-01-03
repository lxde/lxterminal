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

#include "lxterminal.h"
#include "setting.h"
#include "tab.h"
#include "preferences.h"
#include "unixsocket.h"
#include "encoding.h"

/* Linux color for palette */
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

static void terminal_childexit(VteTerminal *vte, Term *term);

static Term *terminal_new(LXTerminal *terminal, const gchar *label, const gchar *pwd, gchar **env, const gchar *exec);
static void terminal_free(Term* term);

/* menu accel saved when the user disables it */
static char *saved_menu_accel = NULL;

static gchar helpmsg[] = {
	"Usage:\n"
	"  lxterminal [Options...] - LXTerminal is a terminal emulator\n\n"
	"Options:\n"
	"  -e, --command=STRING             Execute the argument to this option inside the terminal\n"
	"  -t, -T, --title=STRING               Set the terminal's title\n"
	"  --working-directory=DIRECTORY    Set the terminal's working directory\n"
	"  --geometry=GEOMETRY              X geometry specification (see \"X\" man page), can be specified once per window to be opened.\n"
	"  -l, --loginshell                 Executes login shell.\n"
};

static GtkActionEntry menus[] =
{
	{ "File", NULL, N_("_File") },
	{ "Edit", NULL, N_("_Edit") },
//	{ "View", NULL, N_("_View") },
	{ "Tabs", NULL, N_("_Tabs") },
	{ "Help", NULL, N_("_Help") }
};
#define MENUBAR_MENU_COUNT G_N_ELEMENTS(menus)

static void
gdk_window_get_geometry_hints(GdkWindow *window,
								GdkGeometry *geometry,
								GdkWindowHints *geom_mask)
{
	XSizeHints size_hints;
	glong junk_size_mask = 0;

	g_return_if_fail (GDK_IS_WINDOW (window));
	g_return_if_fail (geometry != NULL);
	g_return_if_fail (geom_mask != NULL);

	*geom_mask = 0;

	if (GDK_WINDOW_DESTROYED (window))
		return;

	if (!XGetWMNormalHints (GDK_WINDOW_XDISPLAY (window),
							GDK_WINDOW_XID (window),
							&size_hints,
							&junk_size_mask))
	return;

	if (size_hints.flags & PMinSize) {
		*geom_mask |= GDK_HINT_MIN_SIZE;
		geometry->min_width = size_hints.min_width;
		geometry->min_height = size_hints.min_height;
	}

	if (size_hints.flags & PMaxSize) {
		*geom_mask |= GDK_HINT_MAX_SIZE;
		geometry->max_width = MAX (size_hints.max_width, 1);
		geometry->max_height = MAX (size_hints.max_height, 1);
	}

	if (size_hints.flags & PResizeInc) {
		*geom_mask |= GDK_HINT_RESIZE_INC;
		geometry->width_inc = size_hints.width_inc;
		geometry->height_inc = size_hints.height_inc;
	}

	if (size_hints.flags & PAspect) {
		*geom_mask |= GDK_HINT_ASPECT;
		geometry->min_aspect = (gdouble) size_hints.min_aspect.x / (gdouble) size_hints.min_aspect.y;
		geometry->max_aspect = (gdouble) size_hints.max_aspect.x / (gdouble) size_hints.max_aspect.y;
	}

	if (size_hints.flags & PWinGravity) {
		*geom_mask |= GDK_HINT_WIN_GRAVITY;
		geometry->win_gravity = size_hints.win_gravity;
	}
}

static gboolean terminal_window_resize(GtkWidget *widget, GtkRequisition *requisition, LXTerminal *terminal)
{
	Term *term;
	GdkGeometry hints;
	gint xpad;
	gint ypad;
	gint i;

	if (!terminal->fixedsize) {
		return FALSE;
	}

	terminal->fixedsize = FALSE;

	/* getting window size by fixed */
	term = g_ptr_array_index(terminal->terms, 0);
	vte_terminal_get_padding(VTE_TERMINAL(term->vte), &xpad, &ypad);
	hints.width_inc = VTE_TERMINAL(term->vte)->char_width;
	hints.height_inc = VTE_TERMINAL(term->vte)->char_height;
	hints.base_width = xpad;
	hints.base_height = ypad;
	hints.min_width = hints.base_width + hints.width_inc * 4;
	hints.min_height = hints.base_height + hints.height_inc * 2;

	/* allow resizing by user */
	for (i=0;i<terminal->terms->len;i++) {
		term = g_ptr_array_index(terminal->terms, i);

		gtk_window_set_geometry_hints(GTK_WINDOW(terminal->mainw),
									term->vte,
									&hints,
									GDK_HINT_RESIZE_INC
									| GDK_HINT_MIN_SIZE
									| GDK_HINT_BASE_SIZE);
	}

	/* setting fixed size */
	gtk_window_resize(GTK_WINDOW(terminal->mainw), requisition->width, requisition->height);

	return FALSE;
}

static void terminal_window_set_fixed_size(LXTerminal *terminal)
{
	Term *term;
	gint i;

	terminal->fixedsize = TRUE;

	for (i=0;i<terminal->terms->len;i++) {
		term = g_ptr_array_index(terminal->terms, i);

		gtk_window_set_geometry_hints(GTK_WINDOW(terminal->mainw),
									term->vte,
									&terminal->geometry,
									terminal->geom_mask);
	}
}

static gboolean term_switchtab(Term *term)
{
	LXTerminal *terminal = term->parent;        
	gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), term->index);
	return TRUE;
}

static gboolean terminal_copy(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

	/* getting current vte */
	term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));

	/* copy from vte */
	vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));

	return TRUE;
}

static gboolean terminal_copy_accel(gpointer data, guint action, GtkWidget *item)
{
	return terminal_copy(NULL, data);
}

static gboolean terminal_paste(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

	/* getting current vte */
	term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));

	/* copy from vte */
	vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));

	return TRUE;
}

static gboolean terminal_paste_accel(gpointer data, guint action, GtkWidget *item)
{
	return terminal_paste(NULL, data);
}

static void terminal_nexttab(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;

	/* cycle */
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook))==gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook))-1)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), 0);
	else
		gtk_notebook_next_page(GTK_NOTEBOOK(terminal->notebook));
}

static void terminal_nexttab_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_nexttab(NULL, data);
}

static void terminal_prevtab(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;

	/* cycle */
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook))==0)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(terminal->notebook), -1);
	else
		gtk_notebook_prev_page(GTK_NOTEBOOK(terminal->notebook));
}

static void terminal_prevtab_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_prevtab(NULL, data);
}

/* direction: -1=left 1=right */
static void terminal_movetab(GtkAction *action, gpointer data, gint direction)
{
	LXTerminal *terminal = (LXTerminal *)data;
	GtkNotebook *notebook = GTK_NOTEBOOK(terminal->notebook);
	gint curPageNum = gtk_notebook_get_current_page( notebook );

	gtk_notebook_reorder_child(
		notebook,
		gtk_notebook_get_nth_page(notebook, curPageNum),
		curPageNum + direction
		);
}

static void terminal_movetableft(GtkAction *action, gpointer data)
{
	terminal_movetab(action, data, -1);
}

static void terminal_movetableft_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_movetableft(NULL, data);
}

static void terminal_movetabright(GtkAction *action, gpointer data)
{
	terminal_movetab(action, data, 1);
}

static void terminal_movetabright_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_movetabright(NULL, data);
}

static void terminal_closetab(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

	/* getting current vte */
	term = g_ptr_array_index(terminal->terms, gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook)));

	/* release child */
	terminal_childexit(VTE_TERMINAL(term->vte), term);
}

static void terminal_closetab_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_closetab(NULL, data);
}

static void terminal_newwindow(GtkAction *action, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;

	lxterminal_init(terminal->parent, 0, NULL, terminal->setting);
}

static void terminal_newwindow_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_newwindow(NULL, data);
}

static void term_set_swicth_accel(Term *term)
{
	if (term->index + 1 < 10) {
    	char switch_tab_accel[1 + 3 + 1 + 1 + 1]; /* "<ALT>n" */
		guint key;
		GdkModifierType mods;
        GtkAccelGroup *accel_group = term->parent->menubar->accel_group;

		sprintf(switch_tab_accel, "<ALT>%d", term->index + 1);
		gtk_accelerator_parse(switch_tab_accel, &key, &mods);
        term->closure = g_cclosure_new_swap(G_CALLBACK(term_switchtab), term, NULL);
		gtk_accel_group_connect(accel_group, key, mods, GTK_ACCEL_LOCKED, 
                                term->closure);
	}
}

static void terminal_newtab(GtkWidget *widget, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;
	Term *term;

#ifdef __linux
	gchar cwd[PATH_MAX];

	gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(terminal->notebook));
	if (current != -1) {
		gchar proc_cwd_link[PATH_MAX];
		gchar *proc_cwd;
		gint i;
		for (i=0;i<terminal->terms->len;i++) {
			term = g_ptr_array_index(terminal->terms, i);
			if (term->index == current)
				break;
		}

		g_message("%d", term->pid);
		g_snprintf(proc_cwd_link, PATH_MAX, "/proc/%d/cwd", term->pid);
		proc_cwd = g_file_read_link(proc_cwd_link, NULL);
		if (proc_cwd) {
			g_strlcpy(cwd, proc_cwd, PATH_MAX);
			g_free(proc_cwd);
		} else {
			g_strlcpy(cwd, g_get_current_dir(), PATH_MAX);
		}
	} else {
		g_strlcpy(cwd, g_get_current_dir(), PATH_MAX);
	}

	term = terminal_new(terminal, _("LXTerminal"), cwd, NULL, NULL);
#else
	term = terminal_new(terminal, _("LXTerminal"), g_get_current_dir(), NULL, NULL);
#endif

	/* add page to notebook */
	gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->label->main);
	term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
	g_ptr_array_add(terminal->terms, term);

	/* push VTE to queue */
	gtk_widget_queue_draw(term->vte);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(term->parent->notebook), term->index);

	if (term->index > 0) {
		/* fixed VTE size */
		terminal_window_set_fixed_size(terminal);

		/* show tab */
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(term->parent->notebook), TRUE);
	}
    term_set_swicth_accel(term);
}

static void terminal_newtab_accel(gpointer data, guint action, GtkWidget *item)
{
	terminal_newtab(NULL, data);
}

static void open_url( GtkAboutDialog* dlg, const gchar* url, gpointer data )
{
    /* FIXME: */
}

static void terminal_about(GtkAction *action, gpointer data)
{
    GtkWidget * about_dlg;
    const gchar *authors[] =
    {
        "Fred Chien <cfsghost@gmail.com>",
        NULL
    };
    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar *translators = _( "translator-credits" );

    gtk_about_dialog_set_url_hook( open_url, NULL, NULL);

    about_dlg = gtk_about_dialog_new ();

    gtk_container_set_border_width ( ( GtkContainer*)about_dlg , 2 );
    gtk_about_dialog_set_version ( (GtkAboutDialog*)about_dlg, VERSION );
    gtk_about_dialog_set_name ( (GtkAboutDialog*)about_dlg, _( "LXTerminal" ) );
    gtk_about_dialog_set_logo( (GtkAboutDialog*)about_dlg, gdk_pixbuf_new_from_file(  PACKAGE_DATA_DIR"/pixmaps/lxterminal.png", NULL ) );
    gtk_about_dialog_set_copyright ( (GtkAboutDialog*)about_dlg, _( "Copyright (C) 2008" ) );
    gtk_about_dialog_set_comments ( (GtkAboutDialog*)about_dlg, _( "Terminal emulator for LXDE project" ) );
    gtk_about_dialog_set_license ( (GtkAboutDialog*)about_dlg, "This program is free software; you can redistribute it and/or\nmodify it under the terms of the GNU General Public License\nas published by the Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA." );
    gtk_about_dialog_set_website ( (GtkAboutDialog*)about_dlg, "http://lxde.org/" );
    gtk_about_dialog_set_authors ( (GtkAboutDialog*)about_dlg, authors );
    gtk_about_dialog_set_translator_credits ( (GtkAboutDialog*)about_dlg, translators );

    gtk_dialog_run( ( GtkDialog*)about_dlg );
    gtk_widget_destroy( about_dlg );
}

static GtkActionEntry menu_items[] =
{
	{ "File_NewWindow", GTK_STOCK_ADD, N_("New _Window"), NEW_WINDOW_ACCEL, "New Window", G_CALLBACK(terminal_newwindow)},
	{ "File_NewTab", GTK_STOCK_ADD, N_("New _Tab"), NEW_TAB_ACCEL, "New Tab", G_CALLBACK(terminal_newtab)},
	{ "File_Sep1", NULL, "Sep" },
	{ "File_CloseTab", GTK_STOCK_CLOSE, N_("_Close Tab"), CLOSE_TAB_ACCEL, "Close Tab", G_CALLBACK(terminal_closetab)},
	{ "File_Quit", GTK_STOCK_QUIT, N_("_Quit"), QUIT_ACCEL, "Quit", G_CALLBACK(gtk_main_quit)},
	{ "Edit_Copy", GTK_STOCK_COPY, N_("Cop_y"), COPY_ACCEL, "Copy", G_CALLBACK(terminal_copy)},
	{ "Edit_Paste", GTK_STOCK_PASTE, N_("_Paste"), PASTE_ACCEL, "Paste", G_CALLBACK(terminal_paste)},
	{ "Edit_Sep1", NULL, "Sep" },
	{ "Edit_Preferences", GTK_STOCK_EXECUTE, N_("Preference_s"), NULL, "Preferences", G_CALLBACK(lxterminal_preferences_dialog)},
//	{ "View_CharacterEncoding", NULL, N_("_Character Encoding"), NULL, "Character Encoding", NULL},
	{ "Tabs_PreviousTab", GTK_STOCK_GO_BACK, N_("Pre_vious Tab"), PREVIOUS_TAB_ACCEL, "Previous Tab", G_CALLBACK(terminal_prevtab)},
	{ "Tabs_NextTab", GTK_STOCK_GO_FORWARD, N_("Ne_xt Tab"), NEXT_TAB_ACCEL, "Next Tab", G_CALLBACK(terminal_nexttab)},
	{ "Tabs_MoveTabLeft", NULL, N_("Move Tab _Left"), MOVE_TAB_LEFT_ACCEL, "Move Tab Left", G_CALLBACK(terminal_movetableft)},
	{ "Tabs_MoveTabRight", NULL, N_("Move Tab _Right"), MOVE_TAB_RIGHT_ACCEL, "Move Tab Right", G_CALLBACK(terminal_movetabright)},
	{ "Help_About", GTK_STOCK_ABOUT, N_("_About"), NULL, "About", G_CALLBACK(terminal_about)}
};
#define MENUBAR_MENUITEM_COUNT G_N_ELEMENTS(menu_items)

static GtkActionEntry vte_menu_items[] =
{
	{ "VTEMenu", NULL, "VTEMenu" },
	{ "NewWindow", GTK_STOCK_ADD, N_("New _Window"), NULL, "New Window", G_CALLBACK(terminal_newwindow)},
	{ "NewTab", GTK_STOCK_ADD, N_("New _Tab"), NULL, "New Tab", G_CALLBACK(terminal_newtab)},
	{ "Sep1", NULL, "Sep" },
	{ "Copy", GTK_STOCK_COPY, N_("Cop_y"), NULL, "Copy", G_CALLBACK(terminal_copy)},
	{ "Paste", GTK_STOCK_PASTE, N_("_Paste"), NULL, "Paste", G_CALLBACK(terminal_paste)},
	{ "Sep2", NULL, "Sep" },
	{ "Preferences", GTK_STOCK_EXECUTE, N_("Preference_s"), NULL, "Preferences", G_CALLBACK(lxterminal_preferences_dialog)},
	{ "Sep3", NULL, "Sep" },
	{ "PreviousTab", GTK_STOCK_GO_BACK, N_("Pre_vious Tab"), NULL, "Previous Tab", G_CALLBACK(terminal_prevtab)},
	{ "NextTab", GTK_STOCK_GO_FORWARD, N_("Ne_xt Tab"), NULL, "Next Tab", G_CALLBACK(terminal_nexttab)},
	{ "Tabs_MoveTabLeft", NULL, N_("Move Tab _Left"), NULL, "Move Tab Left", G_CALLBACK(terminal_prevtab)},
	{ "Tabs_MoveTabRight", NULL, N_("Move Tab _Right"), NULL, "Move Tab Right", G_CALLBACK(terminal_nexttab)},
	{ "CloseTab", GTK_STOCK_CLOSE, N_("_Close Tab"), NULL, "Close Tab", G_CALLBACK(terminal_closetab)}
};
#define VTE_MENUITEM_COUNT G_N_ELEMENTS(vte_menu_items)

static void terminal_switch_tab(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
	const gchar *title;
	Term *term;
	LXTerminal *terminal = (LXTerminal *)data;

	if (terminal->terms->len <= num)
		return;

	term = g_ptr_array_index(terminal->terms, num);

	/* if title of VTE is NULL */
	if ((title = vte_terminal_get_window_title(VTE_TERMINAL(term->vte))) == NULL)
		gtk_window_set_title(GTK_WINDOW(terminal->mainw), _("LXTerminal"));
	else
		gtk_window_set_title(GTK_WINDOW(terminal->mainw), title);
}

static void terminal_title_changed(VteTerminal *vte, Term *term)
{
	/* setting label */
	lxterminal_tab_label_set_text(term->label, vte_terminal_get_window_title(VTE_TERMINAL(vte)));
	lxterminal_tab_label_set_tooltip_text(term->label, vte_terminal_get_window_title(VTE_TERMINAL(vte)));

	/* setting window title */
	gtk_window_set_title(GTK_WINDOW(term->parent->mainw), vte_terminal_get_window_title(VTE_TERMINAL(vte)));
}

static void terminal_windowexit(gpointer terminal_p, GObject *where_the_object_was)
{
	LXTerminal * terminal = (LXTerminal *) terminal_p;
	int i;

	if (terminal->parent->windows->len==1) {
		gtk_main_quit();
	} else {
		g_ptr_array_remove_index(terminal->parent->windows, terminal->index);

		/* decreasing index number after the window be removed */
		for (i=terminal->index;i<terminal->parent->windows->len;i++) {
			LXTerminal *t = g_ptr_array_index(terminal->parent->windows, i);
			t->index--;
		}

	}
}

static void terminal_childexit(VteTerminal *vte, Term *term)
{
	int i;
	LXTerminal *terminal = term->parent;

	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook))==1) {
		/* release */
		g_ptr_array_free(terminal->terms, TRUE);

		/* quit */
		gtk_widget_destroy(terminal->mainw);
	} else {
		/* remove page */
		g_ptr_array_remove_index(terminal->terms, term->index);

		/* decreasing index number after the page be removed */
		for (i=term->index;i<terminal->terms->len;i++) {
			Term *t = g_ptr_array_index(terminal->terms, i);
			t->index--;
		}

		/* release */
		gtk_notebook_remove_page(GTK_NOTEBOOK(terminal->notebook), term->index);
		terminal_free(term);

		/* if only one page, hide tab */
		if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) == 1) {
			gint xpad;
			gint ypad;
			gint cols;
			gint rows;

			/* get original info of VTE */
			Term *t = g_ptr_array_index(terminal->terms, 0);
			vte_terminal_get_padding(VTE_TERMINAL(t->vte), &xpad, &ypad);
			cols = vte_terminal_get_column_count(VTE_TERMINAL(t->vte));
			rows = vte_terminal_get_row_count(VTE_TERMINAL(t->vte));

			/* fixed VTE size */
			terminal_window_set_fixed_size(terminal);

			/* hide tab */
			gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);

			/* recovery window size */
			vte_terminal_set_size(VTE_TERMINAL(t->vte), cols, rows);
			gtk_window_resize(GTK_WINDOW(terminal->mainw),
				xpad + VTE_TERMINAL(t->vte)->char_width,
				ypad + VTE_TERMINAL(t->vte)->char_height);
		}
	}
}

static gboolean terminal_vte_button_press(VteTerminal *vte, GdkEventButton *event, gpointer data)
{
	LXTerminal *terminal = (LXTerminal *)data;
	GtkWidget *menu_item;
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	guint merge_id;
	gint i;

	/* right-click */
	if (event->button == 3) {
		/* initializing UI manager */
		manager = gtk_ui_manager_new();
		action_group = gtk_action_group_new("VTEMenu");
		gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
		gtk_action_group_add_actions(action_group, vte_menu_items, VTE_MENUITEM_COUNT, terminal);
		gtk_ui_manager_insert_action_group(manager, action_group, 0);

		merge_id = gtk_ui_manager_new_merge_id(manager);

		gtk_ui_manager_add_ui(manager, merge_id, "/", "VTEMenu", NULL, GTK_UI_MANAGER_POPUP, FALSE);
		for (i=1;i<VTE_MENUITEM_COUNT;i++) {
			if (strcmp(vte_menu_items[i].label, "Sep")==0)
				gtk_ui_manager_add_ui(manager, merge_id, "/VTEMenu", vte_menu_items[i].name, NULL, GTK_UI_MANAGER_SEPARATOR, FALSE);
			else
				gtk_ui_manager_add_ui(manager, merge_id, "/VTEMenu", vte_menu_items[i].name, vte_menu_items[i].name, GTK_UI_MANAGER_MENUITEM, FALSE);
		}

		gtk_menu_popup(GTK_MENU(gtk_ui_manager_get_widget(manager, "/VTEMenu")), NULL, NULL, NULL, NULL, event->button, event->time);
	} else if (event->button == 1) { /* left click */
		/* steal from tilda-0.09.6/src/tilda_terminal.c:743 */
		gint tag;
		gint xpad, ypad;
		gchar* match;
		gchar* cmd;
		gboolean ret = FALSE;

		vte_terminal_get_padding(vte, &xpad, &ypad);
		match = vte_terminal_match_check(vte,
				(event->x - xpad) / vte->char_width,
				(event->y - ypad) / vte->char_height,
				&tag);

		/* Check if we can launch a web browser, and do so if possible */
		if ((event->state & GDK_CONTROL_MASK) && match != NULL) {
#if DEBUG
			g_print("Got a Ctrl+Left Click -- Matched: `%s' (%d)\n", match, tag);
#endif
			cmd = g_strdup_printf("%s %s", "xdg-open", match);
#if DEBUG
			g_print("Launching command: `%s'\n", cmd);
#endif
			ret = g_spawn_command_line_async(cmd, NULL);

			/* Check that the command launched */
			if (!ret) {
				g_printerr(_("Failed to launch the web browser. The command was `%s'\n"), cmd);
			}

			g_free(cmd);
		}

		/* Always free match if it is non NULL */
		if (match)
			g_free(match);
	}

#if 0
	GtkItemFactory *item_factory;

	/* right-click */
	if (event->button == 3) {
		item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
		gtk_item_factory_set_translate_func(item_factory, gettext, NULL, NULL);
		gtk_item_factory_create_items(item_factory, sizeof(vte_menu_items) / sizeof(vte_menu_items[0]), vte_menu_items, data);
		gtk_menu_popup(GTK_MENU(gtk_item_factory_get_widget(item_factory, "<main>")), NULL, NULL, NULL, NULL, event->button, event->time);
	}
#endif
	return FALSE;
}

void terminal_term_setting_update(Term *term, LXTerminal *terminal){
	vte_terminal_reset((VteTerminal *)term->vte, FALSE, FALSE);

	vte_terminal_set_font_from_string((VteTerminal *)term->vte, terminal->setting->fontname);
	vte_terminal_set_word_chars((VteTerminal *)term->vte, terminal->setting->selchars);
	vte_terminal_set_scrollback_lines((VteTerminal *)term->vte, terminal->setting->scrollback);

	vte_terminal_set_cursor_blink_mode((VteTerminal *)term->vte, terminal->setting->cursorblinks?VTE_CURSOR_BLINK_ON:VTE_CURSOR_BLINK_OFF);

	/* background transparency */
	if( terminal->rgba ){
		/* dirty hack - vte_terminal_queue_background_update
		 * doesn't run without changing background */
		vte_terminal_set_color_background((VteTerminal *)term->vte, &terminal->foreground);
		vte_terminal_set_background_transparent((VteTerminal *)term->vte, FALSE);
		vte_terminal_set_opacity((VteTerminal *)term->vte, terminal->setting->bgalpha);
	} else {
		vte_terminal_set_background_transparent((VteTerminal *)term->vte, terminal->setting->bgalpha == 65535 ? FALSE : TRUE);
		vte_terminal_set_background_saturation((VteTerminal *)term->vte, 1-((double)terminal->setting->bgalpha/65535));
	}

	vte_terminal_set_colors((VteTerminal *)term->vte, &terminal->foreground, &terminal->background, &linux_color[0], 16);

	/* update scrollbar */
	if( terminal->setting->hidescrollbar )
		gtk_widget_hide( term->scrollbar );
	else
		gtk_widget_show( term->scrollbar );
}

static Term *terminal_new(LXTerminal *terminal, const gchar *label, const gchar *pwd, gchar **env, const gchar *exec)
{
	gint ret;
	GRegex *dingus1, *dingus2;
	Term *term;

	/* create terminal */
	term = g_new0(Term, 1);
	term->parent = terminal;
	term->vte = vte_terminal_new();
	term->box = gtk_hbox_new(FALSE, 0);
	term->scrollbar = gtk_vscrollbar_new(NULL);
	gtk_box_pack_start(GTK_BOX(term->box), term->vte, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(term->box), term->scrollbar, FALSE, TRUE, 0);

	/* setting terminal */
	vte_terminal_set_emulation((VteTerminal *)term->vte, "xterm");

	/* Set encoding from locale. */
	setlocale(LC_ALL, "");
	vte_terminal_set_encoding((VteTerminal *)term->vte, nl_langinfo(CODESET));

	/* fixing bugs for specific environment */
	vte_terminal_set_backspace_binding((VteTerminal *)term->vte, VTE_ERASE_ASCII_DELETE);
	vte_terminal_set_delete_binding((VteTerminal *)term->vte, VTE_ERASE_DELETE_SEQUENCE);

	/* disable beeping */
	vte_terminal_set_audible_bell((VteTerminal *)term->vte, FALSE);

	if (!gdk_color_parse(terminal->setting->bgcolor, &terminal->background)) {
		terminal->background = (GdkColor){ 0, 0, 0, 0 };
		printf("Bad bgcolor string in config: %s\n", terminal->setting->bgcolor);
	}
	
	if (!gdk_color_parse(terminal->setting->fgcolor, &terminal->foreground)) {
		terminal->foreground = (GdkColor){ 0, 0xaaaa, 0xaaaa, 0xaaaa};
		printf("Bad fgcolor string in config: %s\n", terminal->setting->fgcolor);
	}

	/* steal from tilda-0.09.6/src/tilda_terminal.c:145 */
	/* Match URL's, etc */
	dingus1 = g_regex_new(DINGUS1, G_REGEX_OPTIMIZE, 0, NULL);
	dingus2 = g_regex_new(DINGUS2, G_REGEX_OPTIMIZE, 0, NULL);
	ret = vte_terminal_match_add_gregex((VteTerminal *)term->vte, dingus1, 0);
	vte_terminal_match_set_cursor_type((VteTerminal *)term->vte, ret, GDK_HAND2);
	ret = vte_terminal_match_add_gregex((VteTerminal *)term->vte, dingus2, 0);
	vte_terminal_match_set_cursor_type((VteTerminal *)term->vte, ret, GDK_HAND2);
	g_regex_unref(dingus1);
	g_regex_unref(dingus2);

	/* create label for tab */
	if (label)
		term->label = lxterminal_tab_label_new(label);
	else
		term->label = lxterminal_tab_label_new(pwd);

	lxterminal_tab_label_close_button_clicked(G_CALLBACK(terminal_childexit), term);

	/* setting scrollbar */
	gtk_range_set_adjustment(GTK_RANGE(term->scrollbar), VTE_TERMINAL(term->vte)->adjustment);

	/* terminal fork */
	if (exec) {
		gchar **command;
		g_shell_parse_argv(exec, NULL, &command, NULL);
		term->pid = vte_terminal_fork_command(VTE_TERMINAL(term->vte), (const char *)*(command), command, env, "/tmp", FALSE, TRUE, TRUE);
		g_strfreev(command);
	} else {
		term->pid = vte_terminal_fork_command(VTE_TERMINAL(term->vte), NULL, NULL, env, pwd, FALSE, TRUE, TRUE);
	}

	/* signal handler */
	g_signal_connect(term->vte, "child-exited", G_CALLBACK(terminal_childexit), term);
	g_signal_connect(term->vte, "window-title-changed", G_CALLBACK(terminal_title_changed), term);
	g_signal_connect(term->vte, "button-press-event", G_CALLBACK(terminal_vte_button_press), terminal);

	gtk_widget_show_all(term->box);

	/* apply other settings */
	terminal_term_setting_update(term, terminal);

	return term;
}

void terminal_free(Term* term)
{
    GtkAccelGroup *accel_group = term->parent->menubar->accel_group;
    gtk_accel_group_disconnect(accel_group, term->closure);
    g_free(term);
}

static Menu *menubar_init(LXTerminal *terminal)
{
	Menu *menubar;
	GtkWidget *menu_item;
	GtkActionGroup *action_group;
	GtkAction *encoding_action;
	GtkUIManager *manager;
	guint merge_id;
	gint i;
	gchar *path, *path_ptr;

	/* initializing menu */
	menubar = g_new0(Menu, 1);

	/* initializing encoding Action */
//	encoding_action = encoding_action_new("CharacterEncoding");

	/* initializing UI manager */
	manager = gtk_ui_manager_new();
	action_group = gtk_action_group_new("MenuBar");
	gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions(action_group, menus, MENUBAR_MENU_COUNT, terminal);
	gtk_action_group_add_actions(action_group, menu_items, MENUBAR_MENUITEM_COUNT, terminal);
//	gtk_action_group_add_action(action_group, encoding_action);
	gtk_ui_manager_insert_action_group(manager, action_group, 0);

	merge_id = gtk_ui_manager_new_merge_id(manager);

	gtk_ui_manager_add_ui(manager, merge_id, "/", "MenuBar", NULL, GTK_UI_MANAGER_MENUBAR, FALSE);

	/* menus */
	for (i=0;i<MENUBAR_MENU_COUNT;i++) {
		path = g_strdup_printf("/MenuBar/%s", menus[i].name);
		for (path_ptr=path;*path_ptr!='\0';path_ptr++) {
			if (*path_ptr=='_')
				*path_ptr = '/';
		}

		path_ptr = g_path_get_dirname(path);
		gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menus[i].name, menus[i].name, GTK_UI_MANAGER_MENU, FALSE);
		g_free(path);
		g_free(path_ptr);
	}

	/* items */
	for (i=0;i<MENUBAR_MENUITEM_COUNT;i++) {
		path = g_strdup_printf("/MenuBar/%s", menu_items[i].name);
		for (path_ptr=path;*path_ptr!='\0';path_ptr++) {
			if (*path_ptr=='_')
				*path_ptr = '/';
		}

		path_ptr = g_path_get_dirname(path);

		if (strcmp(menu_items[i].label, "Sep")==0) {
			gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menu_items[i].name, NULL, GTK_UI_MANAGER_SEPARATOR, FALSE);
#if 0
		/* Encoding */
		} else if (strcmp(menu_items[i].name, "View_CharacterEncoding")==0) {
			gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menu_items[i].name, "CharacterEncoding", GTK_UI_MANAGER_MENUITEM, FALSE);
#endif
		} else {
			gtk_ui_manager_add_ui(manager, merge_id, path_ptr, menu_items[i].name, menu_items[i].name, GTK_UI_MANAGER_MENUITEM, FALSE);
		}

		g_free(path);
		g_free(path_ptr);
	}

	menubar->menu = gtk_ui_manager_get_widget(manager, "/MenuBar");

	return menubar;
}

static void lxterminal_accelerator_init(LXTerminal *terminal)
{
	guint key;
	GdkModifierType mods;

	terminal->menubar->accel_group = gtk_accel_group_new();

	gtk_accelerator_parse(NEW_WINDOW_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_newwindow_accel), terminal, NULL));

	gtk_accelerator_parse(QUIT_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(gtk_main_quit), NULL, NULL));

	gtk_accelerator_parse(NEW_TAB_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_newtab_accel), terminal, NULL));

	gtk_accelerator_parse(CLOSE_TAB_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_closetab_accel), terminal, NULL));

	gtk_accelerator_parse(COPY_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_copy_accel), terminal, NULL));

	gtk_accelerator_parse(PASTE_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_paste_accel), terminal, NULL));

	gtk_accelerator_parse(NEXT_TAB_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_nexttab_accel), terminal, NULL));

	gtk_accelerator_parse(PREVIOUS_TAB_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_prevtab_accel), terminal, NULL));

	gtk_accelerator_parse(MOVE_TAB_LEFT_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_movetableft_accel), terminal, NULL));

	gtk_accelerator_parse(MOVE_TAB_RIGHT_ACCEL, &key, &mods);
	gtk_accel_group_connect(terminal->menubar->accel_group, key, mods, GTK_ACCEL_LOCKED, g_cclosure_new_swap(G_CALLBACK(terminal_movetabright_accel), terminal, NULL));

	gtk_window_add_accel_group(GTK_WINDOW(terminal->mainw), terminal->menubar->accel_group);
}

void lxterminal_menuaccel_update(LXTerminal *terminal)
{
	/* update F10 status */
	/* hack took from gnome-terminal */
	
	if (saved_menu_accel == NULL) {
		g_object_get (G_OBJECT (gtk_settings_get_default ()),
					"gtk-menu-bar-accel",
					&saved_menu_accel,
					NULL);
		/* FIXME if gtkrc is reparsed we don't catch on,
		 * I guess.
		 */
	}

	
	if (terminal->setting->disablef10) {
		gtk_settings_set_string_property (gtk_settings_get_default(),
										"gtk-menu-bar-accel",
										/* no one will ever press this ;-) */
										"<Shift><Control><Mod1><Mod2><Mod3><Mod4><Mod5>F10",
										"lxterminal");
	} else {
		gtk_settings_set_string_property (gtk_settings_get_default(),
										"gtk-menu-bar-accel",
										saved_menu_accel,
										"lxterminal");
	}
}

void terminal_setting_update(LXTerminal *terminal)
{
	gint i;

	/* know if it is composited */
	terminal->rgba = gtk_widget_is_composited( GTK_WIDGET(terminal->mainw) );

	/* update all of terminals */
	for (i=0;i<terminal->terms->len;i++)
		terminal_term_setting_update(g_ptr_array_index(terminal->terms, i), terminal);

	/* update tab position */
	lxterminal_tab_set_position(terminal->notebook, terminal->tabpos);

	/* update menu accel */
	lxterminal_menuaccel_update(terminal);

	/* update menubar */
	if( terminal->setting->hidemenubar )
		gtk_widget_hide( terminal->menubar->menu );
	else
		gtk_widget_show( terminal->menubar->menu );
}

LXTerminal *lxterminal_init(LXTermWindow *lxtermwin, gint argc, gchar **argv, Setting *setting)
{
	LXTerminal *terminal;
	Term *term = NULL;
	gchar *cmd = NULL;
	gchar *workdir = NULL;
	gchar *title = NULL;
	gint cols = 0, rows = 0;

	/* argument */
	if (argc>1) {
		int i;

		for (i=1;i<argc;i++) {
			if (strncmp(argv[i],"--command=", 10)==0) {
				cmd = argv[i]+10;
				continue;
			} else if ((strcmp(argv[i],"--command")==0 || strcmp(argv[i],"-e") == 0) && (i+1<argc) ) {
				cmd = argv[++i];
				continue;
			} else if (strncmp(argv[i],"--title=", 8)==0) {
				title = argv[i]+8;
				continue;
			} else if ((strcmp(argv[i],"--title")==0 || strcmp(argv[i],"-t")==0 || strcmp(argv[i],"-T")==0)&&(i+1<argc)) {
				title = argv[++i];
				continue;
			} else if (strncmp(argv[i],"--working-directory=", 20)==0) {
				workdir = g_strdup(argv[i]+20);
				continue;
			} else if (strncmp(argv[i],"--geometry=", 11)==0) {
				sscanf(argv[i]+11, "%dx%d", &cols, &rows);
				continue;
			} else if ((strcmp(argv[i],"--loginshell")==0 || strcmp(argv[i],"-l")==0)&&cmd==NULL) {
				cmd = "sh -l";
			}
		}
	}

	terminal = g_new0(LXTerminal, 1);
	terminal->parent = lxtermwin;
	terminal->terms = g_ptr_array_new();
	terminal->fixedsize = TRUE;

	g_ptr_array_add(lxtermwin->windows, terminal);
	terminal->index = terminal->parent->windows->len - 1;

	/* Setting */
	if (setting)
		terminal->setting = setting;

	/* create window */
	terminal->mainw = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* colormap assign */
	GdkColormap *colormap = gdk_screen_get_rgba_colormap( gtk_widget_get_screen( GTK_WIDGET(terminal->mainw) ) );
	if (colormap) gtk_widget_set_colormap( GTK_WIDGET(terminal->mainw), colormap );

	/* when composited changed, reload the terminal settings */
	g_signal_connect_swapped(GTK_WIDGET(terminal->mainw), "composited-changed", G_CALLBACK(terminal_setting_update), terminal);
	
	if (!title)
		gtk_window_set_title(GTK_WINDOW(terminal->mainw), _("LXTerminal"));
	else
		gtk_window_set_title(GTK_WINDOW(terminal->mainw), title);

	gtk_window_set_icon_from_file(GTK_WINDOW(terminal->mainw), PACKAGE_DATA_DIR "/pixmaps/lxterminal.png", NULL);
	g_object_weak_ref((GObject *) terminal->mainw, terminal_windowexit, terminal);

	/* create box for putting menubar and notebook */
	terminal->box = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(terminal->mainw), terminal->box);

	/* create menubar */
	terminal->menubar = menubar_init(terminal);
	gtk_box_pack_start(GTK_BOX(terminal->box), terminal->menubar->menu, FALSE, TRUE, 0);

	/* create notebook */
	terminal->notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(terminal->notebook), TRUE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(terminal->notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(terminal->notebook), FALSE);

	/* Tab Position */
	terminal->tabpos = lxterminal_tab_get_position_id(terminal->setting->tabpos);

	g_signal_connect(terminal->notebook, "switch-page", G_CALLBACK(terminal_switch_tab), terminal);
	gtk_box_pack_start(GTK_BOX(terminal->box), terminal->notebook, TRUE, TRUE, 0);

	if ( ! workdir) workdir = g_get_current_dir();
	term = terminal_new(terminal, _("LXTerminal"), workdir, NULL, cmd);
	g_free(workdir);

	/* set default cols and rows */
	if (cols&&rows)
		vte_terminal_set_size(VTE_TERMINAL(term->vte), cols, rows);

	gtk_notebook_append_page(GTK_NOTEBOOK(terminal->notebook), term->box, term->label->main);
	term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
	g_ptr_array_add(terminal->terms, term);

	/* initializing accelerator */
	lxterminal_accelerator_init(terminal);
        term_set_swicth_accel(term);

	gtk_widget_show_all(terminal->mainw);

	/* update terminal settings */
	terminal_setting_update(terminal);

	/* original hints of VTE */
	gdk_window_get_geometry_hints(GTK_WIDGET(term->vte)->window,
										&terminal->geometry,
										&terminal->geom_mask);

	/* resizing terminal with window size */
	g_signal_connect(terminal->mainw, "size-request", G_CALLBACK(terminal_window_resize), terminal);

	return terminal;
}

int main(gint argc, gchar** argv)
{
	LXTermWindow *lxtermwin;
	Setting *setting;
	gchar *dir, *path;
	gint i;

	gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
	bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
	bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
	textdomain ( GETTEXT_PACKAGE );
#endif

	if (argc>1) {
		for (i=1;i<argc;i++) {
			if (strncmp(argv[i],"--command=", 10)==0) {
				continue;
			} else if ((strcmp(argv[i],"--command")==0||strcmp(argv[i],"-e")==0)&&(i+1<argc)) {
				i++;
				continue;
			} else if (strncmp(argv[i],"--title=", 8)==0) {
				continue;
			} else if ((strcmp(argv[i],"--title")==0||strcmp(argv[i],"-t")==0||strcmp(argv[i],"-T")==0)&&(i+1<argc)) {
				i++;
				continue;
			} else if (strncmp(argv[i],"--working-directory=", 20)==0) {
				continue;
			} else if (strncmp(argv[i],"--geometry=", 11)==0) {
				continue;
			} else if ((strcmp(argv[i],"--loginshell")==0 || strcmp(argv[i],"-l")==0)) {
				continue;
			}

			printf("%s\n", helpmsg);
			return 0;
		}
	}

	/* initializing Window Array */
	lxtermwin = g_new0(LXTermWindow, 1);

	/* initializing socket */
	if (!lxterminal_socket_init(lxtermwin, argc, argv))
		return 0;

	/* load config file */
	dir = g_build_filename(g_get_user_config_dir(), "lxterminal" , NULL);
	g_mkdir_with_parents(dir, 0700);
	path = g_build_filename(dir, "lxterminal.conf", NULL);
	g_free(dir);

	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		setting = load_setting_from_file(PACKAGE_DATA_DIR "/lxterminal/lxterminal.conf");
		/* save to user's directory */
		setting_save(setting);
	} else {
		setting = load_setting_from_file(path);
	}

	g_free(path);

	/* initializing something */
	lxtermwin->windows = g_ptr_array_new();
	lxtermwin->setting = setting;

	/* initializing LXTerminal */
	lxterminal_init(lxtermwin, argc, argv, setting);

	gtk_main();

    return 0;
}


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

#ifndef LXTERMINAL_H
#define LXTERMINAL_H

#include "setting.h"

/* steal from tilda-0.09.6/src/tilda_terminal.c:36 */
#define DINGUS1 "(((news|telnet|nttp|file|http|ftp|https)://)|(www|ftp)[-A-Za-z0-9]*\\.)[-A-Za-z0-9\\.]+(:[0-9]*)?"
#define DINGUS2 "(((news|telnet|nttp|file|http|ftp|https)://)|(www|ftp)[-A-Za-z0-9]*\\.)[-A-Za-z0-9\\.]+(:[0-9]*)?/[-A-Za-z0-9_\\$\\.\\+\\!\\*\\(\\),;:@&=\\?/~\\#\\%]*[^]'\\.}>\\) ,\\\"]"

/* Top level application context. */
typedef struct _lxtermwindow {
//    Setting * setting;                /* Pointer to current user preferences */
    GPtrArray * windows;            /* Array of pointers to LXTerminal structures */
} LXTermWindow;

/* Representative of a toplevel window. */
typedef struct _lxterminal {
    LXTermWindow * parent;          /* Back pointer to top level context */
    gint index;                 /* Index of this element in parent->windows */
    GtkWidget * window;             /* Toplevel window */
    GtkWidget * box;                /* Vertical box, child of toplevel window */
    GtkWidget * menu;               /* Menu bar, child of vertical box */
    GtkActionGroup *action_group;   /* Action group on this window */
    GtkAccelGroup * accel_group;        /* Accelerator group for accelerators on this window */
    GtkWidget * notebook;           /* Notebook, child of vertical box */
    GPtrArray * terms;              /* Array of pointers to Term structures */
//    Setting * setting;                /* A copy of parent->setting */
    GdkGeometry geometry;           /* Geometry hints (see XGetWMNormalHints) */
    GdkWindowHints geometry_mask;       /* Mask of valid data in geometry hints */
    gboolean rgba;              /* True if colormap is RGBA */
    GdkColor background;            /* User preference background color converted to GdkColor */
    GdkColor foreground;            /* User preference foreground color converted to GdkColor */
    gint tab_position;              /* Tab position as an integer value */
    gboolean login_shell;           /* Terminal will spawn login shells */
    gdouble scale;                  /* Terminal scale */
    glong col;                      /* Saved horizontal size */
    glong row;                      /* Saved vertical size */
} LXTerminal;

/* Representative of a tab within a toplevel window. */
typedef struct _term {
    LXTerminal * parent;            /* Back pointer to LXTerminal */
    gint index;                 /* Index of this element in parent->terms */
    GtkWidget * tab;                /* Toplevel of the tab */
    GtkWidget * label;              /* Label of the tab, child of the toplevel */
    gboolean user_specified_label;      /* User did "Name Tab", so we will never overwrite this with the window title */
    GtkWidget * close_button;           /* Close button for the tab, child of the toplevel */
    GtkWidget * box;                /* Horizontal box, child of notebook */
    GtkWidget * vte;                /* VteTerminal, child of horizontal box */
    GtkWidget * scrollbar;          /* Scroll bar, child of horizontal box */
    GPid pid;                                   /* Process ID of the process that has this as its terminal */
    GClosure * closure;             /* Accelerator structure */
    gchar * matched_url;
    gboolean open_menu_on_button_release;
    gulong exit_handler_id;
} Term;

/* Output of lxterminal_process_arguments. */
typedef struct _command_arguments {
    char * executable;              /* Value of argv[0]; points into argument vector */
    gchar * * command;              /* Value of -e, --command; memory allocated by glib */
    int geometry_bitmask;
    unsigned int geometry_columns;           /* Value of --geometry */
    unsigned int geometry_rows;
    int geometry_xoff;
    int geometry_yoff;
    char * title;               /* Value of -t, -T, --title; points into argument vector */
    char * tabs;                /* Value of --tab; points into argument vector */
    char * working_directory;           /* Value of --working-directory; points into argument vector */
    gboolean login_shell;           /* Terminal will spawn login shells */
    gboolean no_remote;
} CommandArguments;

extern gboolean lxterminal_process_arguments(gint argc, gchar * * argv, CommandArguments * arguments);
extern LXTerminal * lxterminal_initialize(LXTermWindow * lxtermwin, CommandArguments * arguments);
extern void terminal_settings_apply_to_all(LXTerminal * terminal);

#endif

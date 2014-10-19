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

#ifndef LXTERMINAL_SETTING_H
#define LXTERMINAL_SETTING_H

#include <gtk/gtk.h>

#define GENERAL_GROUP "general"
#define FONT_NAME "fontname"
#define FG_COLOR "fgcolor"
#define BG_COLOR "bgcolor"
#define BG_ALPHA "bgalpha"
#define DISALLOW_BOLD "disallowbold"
#define CURSOR_BLINKS "cursorblinks"
#define CURSOR_UNDERLINE "cursorunderline"
#define AUDIBLE_BELL "audiblebell"
#define TAB_POS "tabpos"
#define SCROLLBACK "scrollback"
#define HIDE_SCROLLBAR "hidescrollbar"
#define HIDE_MENUBAR "hidemenubar"
#define HIDE_CLOSE_BUTTON "hideclosebutton"
#define HIDE_POINTER "hidepointer"
#define SEL_CHARS "selchars"
#define DISABLE_F10 "disablef10"
#define DISABLE_ALT "disablealt"

#define SHORTCUT_GROUP "shortcut"
#define NEW_WINDOW_ACCEL "new_window_accel"
#define NEW_TAB_ACCEL "new_tab_accel"
#define CLOSE_TAB_ACCEL "close_tab_accel"
#define CLOSE_WINDOW_ACCEL "close_window_accel"
#define COPY_ACCEL "copy_accel"
#define PASTE_ACCEL "paste_accel"
#define NAME_TAB_ACCEL "name_tab_accel"
#define PREVIOUS_TAB_ACCEL "previous_tab_accel"
#define NEXT_TAB_ACCEL "next_tab_accel"
#define MOVE_TAB_LEFT_ACCEL "move_tab_left_accel"
#define MOVE_TAB_RIGHT_ACCEL "move_tab_right_accel"

#define NEW_WINDOW_ACCEL_DEF "<CTRL><SHIFT>N"
#define NEW_TAB_ACCEL_DEF "<CTRL><SHIFT>T"
#define CLOSE_TAB_ACCEL_DEF "<CTRL><SHIFT>W"
#define CLOSE_WINDOW_ACCEL_DEF "<CTRL><SHIFT>Q"
#define COPY_ACCEL_DEF "<CTRL><SHIFT>C"
#define PASTE_ACCEL_DEF "<CTRL><SHIFT>V"
#define NAME_TAB_ACCEL_DEF "<CTRL><SHIFT>I"
#define PREVIOUS_TAB_ACCEL_DEF "<CTRL>Page_Up"
#define NEXT_TAB_ACCEL_DEF "<CTRL>Page_Down"
#define MOVE_TAB_LEFT_ACCEL_DEF "<CTRL><SHIFT>Page_Up"
#define MOVE_TAB_RIGHT_ACCEL_DEF "<CTRL><SHIFT>Page_Down"

/* User preferences. */
typedef struct _setting {

    GKeyFile * keyfile;         /* Pointer to GKeyFile containing settings */
    char * font_name;           /* Font name */
    GdkColor background_color;      /* Background color */
    guint16 background_alpha;       /* Alpha value to go with background color */
    GdkColor foreground_color;      /* Foreground color */
    gboolean disallow_bold;     /* Disallow bolding by VTE */
    gboolean cursor_blink;      /* True if cursor blinks */
    gboolean cursor_underline;      /* True if underline cursor; false if block cursor */
    gboolean audible_bell;      /* True if audible bell */
    char * tab_position;        /* Position of tabs on main window (top, bottom, left, right) */
    gint scrollback;            /* Scrollback buffer size in lines */
    gboolean hide_scroll_bar;       /* True if scroll bar is NOT visible */
    gboolean hide_menu_bar;     /* True if menu bar is NOT visible */
    gboolean hide_close_button;     /* True if close buttons are NOT visible */
    gboolean hide_pointer;      /* True if mouse pointer should be auto-hidden */
    char * word_selection_characters;   /* Characters that act as word breaks during selection by word */
    gboolean disable_f10;       /* True if F10 will be passed to program; false if it brings up File menu */
    gboolean disable_alt;       /* True if Alt-n is passed to shell; false if it is used to switch between tabs */

    gboolean geometry_change;       /* True if there is a geometry change, until it has been acted on */
    
    /* Shortcut group settings. */
    char * new_window_accel;        /* NEW_WINDOW_ACCEL */
    char * new_tab_accel;       /* NEW_TAB_ACCEL */
    char * close_tab_accel;     /* CLOSE_TAB_ACCEL */
    char * close_window_accel;      /* CLOSE_WINDOW_ACCEL */
    char * copy_accel;      /* COPY_ACCEL */
    char * paste_accel;     /* PASTE_ACCEL */
    char * name_tab_accel;      /* NAME_TAB_ACCEL */
    char * previous_tab_accel;      /* PREVIOUS_TAB_ACCEL */
    char * next_tab_accel;      /* NEXT_TAB_ACCEL */
    char * move_tab_left_accel;     /* MOVE_TAB_LEFT_ACCEL */
    char * move_tab_right_accel;    /* MOVE_TAB_RIGHT_ACCEL */

} Setting;

extern Setting * get_setting();
extern void save_setting();
extern Setting * load_setting();

/* Utils for chsnge setting through preference */
extern void set_setting(Setting * setting);
extern void free_setting(Setting * setting);
extern Setting * copy_setting(Setting * setting);

extern void print_setting();

#endif

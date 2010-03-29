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

#ifndef LXTERMINAL_TAB_H
#define LXTERMINAL_TAB_H

#include "lxterminal.h"

extern void lxterminal_tab_set_position(GtkWidget * notebook, gint tabpos);
extern gint lxterminal_tab_get_position_id(gchar * position);
extern void lxterminal_tab_label_close_button_clicked(GCallback func, Term * term);
extern void lxterminal_tab_label_set_text(LXTab * tab, const gchar * str);
extern void lxterminal_tab_label_set_tooltip_text(LXTab * tab, const gchar * str);
extern LXTab * lxterminal_tab_label_new(const gchar * str);

#endif


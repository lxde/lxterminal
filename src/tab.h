#ifndef LXTERMINAL_TAB_H
#define LXTERMINAL_TAB_H

#include "lxterminal.h"

void lxterminal_tab_label_close_button_clicked(GCallback func, Term *term);
void lxterminal_tab_label_set_text(LXTab *tab, const gchar *str);
void lxterminal_tab_label_set_tooltip_text(LXTab *tab, const gchar *str);
LXTab *lxterminal_tab_label_new(const gchar *str);

#endif


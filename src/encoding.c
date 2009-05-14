/*
 *    Copyright 2008 Fred Chien <cfsghost@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *    MA 02110-1301, USA.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "lxterminal.h"
#include "encoding.h"

#define ENCODING_TYPE_ACTION	(encoding_action_get_type ())
#define ENCODING_ACTION(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ENCODING_TYPE_ACTION, EncodingAction))
#define ENCODING_IS_ACTION(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), ENCODING_TYPE_ACTION))

typedef struct _EncodingAction EncodingAction;
typedef struct _EncodingActionClass EncodingActionClass;

struct _EncodingAction
{
	GtkAction parent_instance;
	GtkWidget *menu;
};

struct _EncodingActionClass
{
	GtkActionClass parent_class;
	void (*item_activated) (EncodingAction *action, gpointer data);
};

static guint item_activated = 0;

G_DEFINE_TYPE (EncodingAction, encoding_action, GTK_TYPE_ACTION);

static void
encoding_chooser_item_activated_cb(GtkWidget *widget, GtkAction *action)
{
/*
  GtkEncodingInfo *current;

  current = gtk_encoding_chooser_get_current_item (chooser);
  if (!current)
    return;

  g_signal_emit (action, item_activated, 0, current);

  gtk_encoding_info_unref (current);
*/
}

static GtkWidget *
encoding_action_create_menu_item(GtkAction *action)
{
	GtkWidget *menuitem;
	GtkWidget *chooser_menu;

	chooser_menu = encoding_list_menu_init();
//	g_signal_connect(chooser_menu, "item-activated", G_CALLBACK (encoding_chooser_item_activated_cb), action);
  
	menuitem = g_object_new(GTK_TYPE_IMAGE_MENU_ITEM, NULL);
  
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), chooser_menu);
  
	return menuitem;
}

static void
encoding_action_class_init(EncodingActionClass *klass)
{
	GObjectClass *gobject_class;
	GtkActionClass *action_class;

	gobject_class = G_OBJECT_CLASS(klass);
	action_class = GTK_ACTION_CLASS(klass);

	action_class->menu_item_type = GTK_TYPE_IMAGE_MENU_ITEM;

	action_class->create_menu_item = encoding_action_create_menu_item;

	item_activated = g_signal_new("item-activated",
					G_OBJECT_CLASS_TYPE (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET (EncodingActionClass, item_activated),
					NULL, NULL,
					g_cclosure_marshal_VOID__BOXED,
					G_TYPE_NONE, 1,
					NULL);
}

static void
encoding_action_init(EncodingAction *action)
{

}

GtkAction *
encoding_action_new(const gchar *name)
{
	return g_object_new(ENCODING_TYPE_ACTION,
				"name", name,
				"label", "_Character Encoding",
				"tooltip", NULL,
				"stock-id", NULL,
				NULL);
}


static TerminalEncodingCategory encategories[] = {
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, N_("West European") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, N_("East European") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, N_("East Asian") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, N_("SE & SW Asian") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, N_("Middle Eastern") },
	{ TERMINAL_LANGUAGE_UNICODE, N_("Unicode") }
};

static TerminalEncoding encodings[] = {
//	{ NULL, TERMINAL_ENCODING_CURRENT_LOCALE, NULL, N_("Current Locale") },

	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_1, "ISO-8859-1", N_("Western") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_2, "ISO-8859-2", N_("Central European") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_3, "ISO-8859-3", N_("South European") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_4, "ISO-8859-4", N_("Baltic") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_5, "ISO-8859-5", N_("Cyrillic") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_ISO_8859_6, "ISO-8859-6", N_("Arabic") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_7, "ISO-8859-7", N_("Greek") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_ISO_8859_8, "ISO-8859-8", N_("Hebrew Visual") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_ISO_8859_8_I, "ISO-8859-8-I", N_("Hebrew") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_ISO_8859_9, "ISO-8859-9", N_("Turkish") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_10, "ISO-8859-10", N_("Nordic") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_13, "ISO-8859-13", N_("Baltic") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_14, "ISO-8859-14", N_("Celtic") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_15, "ISO-8859-15", N_("Western") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_ISO_8859_16, "ISO-8859-16", N_("Romanian") },

	{ TERMINAL_LANGUAGE_UNICODE, TERMINAL_ENCODING_UTF_7, "UTF-7", N_("Unicode") },
	{ TERMINAL_LANGUAGE_UNICODE, TERMINAL_ENCODING_UTF_8, "UTF-8", N_("Unicode") },
	{ TERMINAL_LANGUAGE_UNICODE, TERMINAL_ENCODING_UTF_16, "UTF-16", N_("Unicode") },
	{ TERMINAL_LANGUAGE_UNICODE, TERMINAL_ENCODING_UCS_2, "UCS-2", N_("Unicode") },
	{ TERMINAL_LANGUAGE_UNICODE, TERMINAL_ENCODING_UCS_4, "UCS-4", N_("Unicode") },

	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_ARMSCII_8, "ARMSCII-8", N_("Armenian") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_BIG5, "BIG5", N_("Chinese Traditional") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_BIG5_HKSCS, "BIG5-HKSCS", N_("Chinese Traditional") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_CP_866, "CP866", N_("Cyrillic/Russian") },

	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_EUC_JP, "EUC-JP", N_("Japanese") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_EUC_KR, "EUC-KR", N_("Korean") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_EUC_TW, "EUC-TW", N_("Chinese Traditional") },

	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_GB18030, "GB18030", N_("Chinese Simplified") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_GB2312, "GB2312", N_("Chinese Simplified") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_GBK, "GBK", N_("Chinese Simplified") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_GEOSTD8, "GEORGIAN-PS", N_("Georgian") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_HZ, "HZ", N_("Chinese Simplified") },

	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_IBM_850, "IBM850", N_("Western") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_IBM_852, "IBM852", N_("Central European") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_IBM_855, "IBM855", N_("Cyrillic") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_IBM_857, "IBM857", N_("Turkish") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_IBM_862, "IBM862", N_("Hebrew") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_IBM_864, "IBM864", N_("Arabic") },

	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_ISO_2022_JP, "ISO2022JP", N_("Japanese") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_ISO_2022_KR, "ISO2022KR", N_("Korean") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_ISO_IR_111, "ISO-IR-111", N_("Cyrillic") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_JOHAB, "JOHAB", N_("Korean") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_KOI8_R, "KOI8-R", N_("Cyrillic") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_KOI8_U, "KOI8-U", N_("Cyrillic/Ukrainian") },

	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_MAC_ARABIC, "MAC_ARABIC", N_("Arabic") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_MAC_CE, "MAC_CE", N_("Central European") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_MAC_CROATIAN, "MAC_CROATIAN", N_("Croatian") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_MAC_CYRILLIC, "MAC-CYRILLIC", N_("Cyrillic") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_MAC_DEVANAGARI, "MAC_DEVANAGARI", N_("Hindi") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_MAC_FARSI, "MAC_FARSI", N_("Farsi") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_MAC_GREEK, "MAC_GREEK", N_("Greek") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_MAC_GUJARATI, "MAC_GUJARATI", N_("Gujarati") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_MAC_GURMUKHI, "MAC_GURMUKHI", N_("Gurmukhi") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_MAC_HEBREW, "MAC_HEBREW", N_("Hebrew") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_MAC_ICELANDIC, "MAC_ICELANDIC", N_("Icelandic") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_MAC_ROMAN, "MAC_ROMAN", N_("Western") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_MAC_ROMANIAN, "MAC_ROMANIAN", N_("Romanian") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_MAC_TURKISH, "MAC_TURKISH", N_("Turkish") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_MAC_UKRAINIAN, "MAC_UKRAINIAN", N_("Cyrillic/Ukrainian") },
	
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_SHIFT_JIS, "SHIFT-JIS", N_("Japanese") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_TCVN, "TCVN", N_("Vietnamese") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_TIS_620, "TIS-620", N_("Thai") },
	{ TERMINAL_LANGUAGE_EAST_ASIAN, TERMINAL_ENCODING_UHC, "UHC", N_("Korean") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_VISCII, "VISCII", N_("Vietnamese") },

	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_WINDOWS_1250, "WINDOWS-1250", N_("Central European") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_WINDOWS_1251, "WINDOWS-1251", N_("Cyrillic") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_WINDOWS_1252, "WINDOWS-1252", N_("Western") },
	{ TERMINAL_LANGUAGE_WEST_EUROPEAN, TERMINAL_ENCODING_WINDOWS_1253, "WINDOWS-1253", N_("Greek") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_WINDOWS_1254, "WINDOWS-1254", N_("Turkish") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_WINDOWS_1255, "WINDOWS-1255", N_("Hebrew") },
	{ TERMINAL_LANGUAGE_MIDDLE_EASTERN, TERMINAL_ENCODING_WINDOWS_1256, "WINDOWS-1256", N_("Arabic") },
	{ TERMINAL_LANGUAGE_EAST_EUROPEAN, TERMINAL_ENCODING_WINDOWS_1257, "WINDOWS-1257", N_("Baltic") },
	{ TERMINAL_LANGUAGE_SE_SW_ASIAN, TERMINAL_ENCODING_WINDOWS_1258, "WINDOWS-1258", N_("Vietnamese") }
};

static GtkActionEntry area_actions[] = {
};

GtkWidget *encoding_list_menu_init()
{
	GtkWidget *mainmenu;
	GtkWidget *subitem;
	GtkWidget *submenu;
	GtkWidget *item = NULL;
	gchar en_name[64];
	int i, j;

	mainmenu = gtk_menu_new();
	for (i=0;i<G_N_ELEMENTS(encategories);i++,item=NULL) {
		//printf("%s:\n", gettext(encategories[i].name));
		subitem = gtk_menu_item_new_with_label(gettext(encategories[i].name));
		submenu = gtk_menu_new();
		for (j=0;j<G_N_ELEMENTS(encodings);j++) {
			if (encodings[j].category==encategories[i].index) {
//				printf("%s\n", gettext(encodings[j].name));
				sprintf(en_name, "%s (%s)", gettext(encodings[j].name), encodings[j].charset);
				item = gtk_radio_menu_item_new_with_label(item ? gtk_radio_menu_item_get_group((GtkRadioMenuItem *) item) : NULL, en_name);
				gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
			}
		}

		if (!item) {
			item = gtk_radio_menu_item_new_with_label(NULL, gettext(encodings[i].name));
			gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		}

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(subitem), submenu);
		gtk_menu_shell_append(GTK_MENU_SHELL(mainmenu), subitem);
	}

	gtk_widget_show_all(mainmenu);

	return mainmenu;
}


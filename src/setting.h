#ifndef LXTERMINAL_SETTING_H
#define LXTERMINAL_SETTING_H

typedef struct _setting {
	char *fontname;
	char *selchars;
	char *bgcolor;
	char *fgcolor;
	char *tabpos;
	guint16 bgalpha;
	glong scrollback;
	gboolean disablef10;
	GKeyFile *keyfile;
	gboolean hidemenubar;
	gboolean hidescrollbar;
	gboolean cursorblinks;
} Setting;

Setting *load_setting_from_file(const char *filename);

#endif

#ifndef LXTERMINAL_SETTING_H
#define LXTERMINAL_SETTING_H

typedef struct _setting {
	char *fontname;
	char *selchars;
	char *bgcolor;
	char *fgcolor;
	char *tabpos;
	glong scrollback;
	gboolean disablef10;
	gboolean bgtransparent;
	GKeyFile *keyfile;
} Setting;

Setting *load_setting_from_file(const char *filename);

#endif

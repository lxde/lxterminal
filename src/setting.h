#ifndef LXTERMINAL_SETTING_H
#define LXTERMINAL_SETTING_H

#define tabpos_top     "top"
#define tabpos_bottom  "bottom"
#define tabpos_left    "left"
#define tabpos_right   "right"

typedef struct _setting {
	char *fontname;
	char *selchars;
	char *bgcolor;
	char *fgcolor;
	char *tabpos;
	glong scrollback;
	GKeyFile *keyfile;
} Setting;

Setting *load_setting_from_file(const char *filename);

#endif

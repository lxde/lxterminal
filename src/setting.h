#ifndef LXTERMINAL_SETTING_H
#define LXTERMINAL_SETTING_H

typedef struct _setting {
	char *fontname;
	char *selchars;
	char *bgcolor;
	char *fgcolor;
	GKeyFile *keyfile;
} Setting;

Setting *load_setting_from_file(const char *filename);

#endif

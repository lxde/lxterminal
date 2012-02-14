default_config_DATA = lxterminal.conf
default_configdir = $(datadir)/lxterminal

ui_files= \
	lxterminal-preferences.ui

lxterminal.desktop_in_files = lxterminal.desktop.in
lxterminal.desktop.in_DATA = $(lxterminal.desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
lxterminal.desktop.indir = $(datadir)/applications

lxterminal_images_DATA = lxterminal.png
lxterminal_imagesdir = $(datadir)/pixmaps

EXTRA_DIST = \
	lxterminal.conf.in \
	$(lxterminal_images_DATA) \
	$(lxterminal.desktop.in_DATA) \
	$(ui_files) \
	$(default_config_DATA)

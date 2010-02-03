default_config_DATA = lxterminal.conf
default_configdir = $(datadir)/lxterminal

lxterminal.desktop_in_files = lxterminal.desktop.in
lxterminal.desktop_DATA = $(lxterminal.desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
lxterminal.desktopdir = $(datadir)/applications

lxterminal_images_DATA = lxterminal.png
lxterminal_imagesdir = $(datadir)/pixmaps

EXTRA_DIST = \
	lxterminal.conf.in \
	$(lxterminal_images_DATA) \
	$(lxterminal.desktop.in_DATA) \
	$(default_config_DATA)

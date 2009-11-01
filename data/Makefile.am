default_config_DATA = lxterminal.conf
default_configdir = $(datadir)/lxterminal

lxterminal.desktop.in_in_files = lxterminal.desktop
lxterminal_desktop_DATA = $(lxterminal_desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
lxterminal_desktopdir = $(datadir)/applications

lxterminal_images_DATA = lxterminal.png
lxterminal_imagesdir = $(datadir)/pixmaps

EXTRA_DIST = \
	lxterminal.conf.in \
	$(lxterminal_images_DATA) \
	$(lxterminal.desktop.in_DATA) \
	$(default_config_DATA)

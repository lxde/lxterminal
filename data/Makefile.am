default_config_DATA = lxterminal.conf
default_configdir = $(datadir)/lxterminal

uidir=$(datadir)/lxterminal
ui_in_files= \
	lxterminal-preferences.glade

ui_files = $(ui_in_files:.glade=.ui)

ui_DATA = $(ui_files) menu.ui

# Purge GtkBuilder UI files
.glade.ui:
	sed 's/<!--.*-->//' < $< | sed ':a;N;$$!ba;s/ *\n *</</g' > $@

CLEANFILES = $(ui_files)

lxterminal.desktop_in_files = lxterminal.desktop.in
lxterminal.desktop.in_DATA = $(lxterminal.desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
lxterminal.desktop.indir = $(datadir)/applications

lxterminal_images_DATA = lxterminal.png
lxterminal_imagesdir = $(datadir)/icons/hicolor/128x128/apps

EXTRA_DIST = \
	lxterminal.conf.in \
	$(lxterminal_images_DATA) \
	$(lxterminal.desktop_in_files) \
	$(lxterminal.desktop.in_DATA) \
	$(ui_DATA) \
	$(ui_in_files) \
	$(default_config_DATA)

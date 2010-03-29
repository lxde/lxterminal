## Process this file with automake to produce Makefile.in

bin_PROGRAMS = lxterminal

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxterminal_SOURCES = \
	unixsocket.h \
	unixsocket.c \
	setting.h \
	setting.c \
	lxterminal.h \
	lxterminal.c \
	preferences.h \
	preferences.c

lxterminal_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

noinst_PROGRAMS=xml-purge
xml_purge_SOURCES=xml-purge.c
xml_purge_CFLAGS=@CFLAGS@
xml_purge_LDADD=@LIBS@

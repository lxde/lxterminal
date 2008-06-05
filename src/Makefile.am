## Process this file with automake to produce Makefile.in

sbin_PROGRAMS = lxterminal

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxterminal_SOURCES = \
	setting.h \
	setting.c \
	lxterminal.h \
	lxterminal.c

lxterminal_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

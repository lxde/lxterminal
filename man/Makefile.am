
dist_man_MANS = lxterminal.1

EXTRA_DIST = lxterminal.sgml

dist-hook: lxterminal.1

lxterminal.1: lxterminal.sgml
	(test -f "`which docbook-to-man`" && docbook-to-man $< > $@) || \
	(test -f "`which db2man`" && db2man $< > $@) || \
	(test -f "`which docbook2man`" && docbook2man $< > $@)


dist_man_MANS = lxterminal.1

EXTRA_DIST = lxterminal.sgml

dist-hook: lxterminal.1

lxterminal.1: lxterminal.sgml
	docbook-to-man $< > $@

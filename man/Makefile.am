man_MANS = \
	lxterminal.1

man_XMANS = \
	lxterminal.xml

EXTRA_DIST = \
	$(man_MANS) \
	$(man_XMANS)


if ENABLE_REGENERATE_MAN

lxterminal.1: lxterminal.xml
	$(XSLTPROC) -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<        

CLEANFILES = $(man_MANS) 
else

$(man_MANS):
	@echo ""
	@echo "Error: you need to run configure with '--enable-man'"
	@echo "       in order to regenerate man pages."
	@echo ""
	@false
endif



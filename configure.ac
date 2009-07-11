AC_PREREQ(2.57)
AC_INIT(lxterminal, 0.1.6, http://lxde.org/)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
AC_PROG_INTLTOOL
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_HEADER_STDC
AC_CHECK_HEADERS(locale.h langinfo.h)

AC_ARG_ENABLE(man,
       [AC_HELP_STRING([--enable-man],
               [regenerate roff man pages from Docbook @<:@default=no@:>@])],
       [enable_man="${enableval}"],
       [enable_man=no]
)
# Checks for libraries.
pkg_modules="glib-2.0 >= 2.6.0 \
             gthread-2.0 \
             vte"

PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

GETTEXT_PACKAGE=lxterminal
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

ALL_LINGUAS="af ar cs da de es et eu fa fi fr gl hr hu id it ja ko lt ml ms nb nl nn pl ps pt pt_BR ru sk sl sv tr uk ur ur_PK vi zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

if test x"$enable_man" = x"yes"; then
	AC_PATH_PROG([XSLTPROC], [xsltproc])
	if test -z "$XSLTPROC"; then
		enable_man=no
	fi

	dnl check for DocBook DTD and stylesheets in the local catalog.
	JH_CHECK_XML_CATALOG([-//OASIS//DTD DocBook XML V4.1.2//EN],
		[DocBook XML DTD V4.1.2], [], enable_man=no)
	JH_CHECK_XML_CATALOG([http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl],
		[DocBook XSL Stylesheets >= 1.70.1], [], enable_man=no)
fi

AM_CONDITIONAL(ENABLE_REGENERATE_MAN, test "x$enable_man" != "xno")

AC_OUTPUT([
  Makefile
  src/Makefile
  data/Makefile
  data/lxterminal.conf
  po/Makefile.in
  man/Makefile
])

echo
echo LXTerminal................................... : Version $VERSION
echo
echo Prefix....................................... : $prefix


AC_PREREQ(2.57)
AC_INIT(lxterminal, 0.1.4, http://lxde.org/)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
AC_PROG_INTLTOOL
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_HEADER_STDC
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

ALL_LINGUAS="ar cs da de es et eu fa fi fr gl hr hu id it ja ko lt ml nb nl nn pl pt pt_BR ru sk sl sv uk vi zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

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


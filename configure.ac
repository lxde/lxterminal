AC_PREREQ(2.53)
AC_INIT(lxterminal, 0.1, http://lxde.org/)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

# Force to dynamic
AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
dnl AC_PROG_INTLTOOL(, [no-xml])

# Checks for libraries.
pkg_modules="glib-2.0 >= 2.6.0 \
             gthread-2.0 \
             vte"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

dnl GETTEXT_PACKAGE=lxnm
dnl AM_GNU_GETTEXT_VERSION(0.14.1)
dnl AC_SUBST(GETTEXT_PACKAGE)
dnl AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

dnl ALL_LINGUAS=""
dnl AM_GLIB_GNU_GETTEXT

AC_OUTPUT([
  Makefile
  src/Makefile
])

echo
echo LXTerminal................................... : Version $VERSION
echo
echo Prefix....................................... : $prefix


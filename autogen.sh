#! /bin/sh
AM_VERSION=-1.10
AC_VERSION=

set -x

if [ "x${ACLOCAL_DIR}" != "x" ]; then
  ACLOCAL_ARG=-I ${ACLOCAL_DIR}
fi

${ACLOCAL:-aclocal$AC_VERSION} ${ACLOCAL_ARG}
${AUTOHEADER:-autoheader$AC_VERSION}
AUTOMAKE=${AUTOMAKE:-automake$AM_VERSION} libtoolize -c --automake --force
AUTOMAKE=${AUTOMAKE:-automake$AM_VERSION} intltoolize -c --automake --force
${AUTOMAKE:-automake$AM_VERSION} --add-missing --copy --include-deps
${AUTOCONF:-autoconf$AC_VERSION}

# mkinstalldirs was not correctly installed in some cases.
cp -f /usr/share/${AUTOMAKE:-automake$AM_VERSION}/mkinstalldirs .

rm -rf autom4te.cache

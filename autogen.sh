#! /bin/sh
# script shamelessly taken from openbox

sh() {
  /bin/sh -c "set -x; $*"
}

export WANT_AUTOMAKE=1.9

sh autopoint --force || exit 1 # for GNU gettext
sh libtoolize --copy --force --automake || exit 1
sh aclocal -I m4 $ACLOCAL_FLAGS || exit 1
sh autoheader || exit 1
sh autoconf || exit 1
sh automake --include-deps --add-missing --copy || exit 1

echo
echo You are now ready to run ./configure
echo enjoy!

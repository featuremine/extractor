#!/bin/sh

set -e

# Display usage
installer_usage()
{
  cat <<EOF
Usage: $0 [options]
Options: [defaults in brackets after descriptions]
  --help            print this message
  --prefix=dir      directory in which to install [/usr/local]
  --user            install in $HOME/.local
EOF
  exit 1
}

installer_echo_exit()
{
  echo $1
  exit 1
}

PROJECT_NAME=
PROJECT_VERSION=
# Display version
installer_version()
{
  echo "$PROJECT_NAME Installer Version: $PROJECT_VERSION, Copyright (c) Featuremine Corporation"
}

# Helper function to fix windows paths.
installer_fix_slashes ()
{
  echo "$1" | sed 's/\\/\//g'
}

installer_user=FALSE
for a in "$@"; do
  if echo $a | grep "^--prefix=" > /dev/null 2> /dev/null; then
    installer_prefix_dir=`echo $a | sed "s/^--prefix=//"`
    installer_prefix_dir=`installer_fix_slashes "${installer_prefix_dir}"`
  fi
  if echo $a | grep "^--user" > /dev/null 2> /dev/null; then
    installer_user=TRUE
  fi
  if echo $a | grep "^--help" > /dev/null 2> /dev/null; then
    installer_usage
  fi
  if echo $a | grep "^--version" > /dev/null 2> /dev/null; then
    installer_version
    exit 2
  fi
done

installer_version
echo "This is a self-extracting archive."
toplevel="`pwd`"
if [ "x${installer_prefix_dir}x" != "xx" ]
then
  toplevel="${installer_prefix_dir}"
elif [ "x${installer_user}x" != "xTRUEx" ]
then
  toplevel="/usr/local"
else
  toplevel="$HOME/.local"
fi

echo "The archive will be extracted to: ${toplevel}"

echo
echo "Using target directory: ${toplevel}"
echo "Extracting, please wait..."
echo ""

# take the archive portion of this file and pipe it to tar
# the NUMERIC parameter in this command should be one more
# than the number of lines in this header file
# there are tails which don't understand the "-n" argument, e.g. on SunOS
# OTOH there are tails which complain when not using the "-n" argument (e.g. GNU)
# so at first try to tail some file to see if tail fails if used with "-n"
# if so, don't use "-n"
use_new_tail_syntax="-n"
tail $use_new_tail_syntax +1 "$0" > /dev/null 2> /dev/null || use_new_tail_syntax=""

extractor="pax -r"
command -v pax > /dev/null 2> /dev/null || extractor="tar xf -"

INSTALLER_LINES=
tail $use_new_tail_syntax +$INSTALLER_LINES "$0" | gunzip | (mkdir -p "${toplevel}" && cd "${toplevel}" && ${extractor}) || if [ "x${installer_user}x" != "xTRUEx" ]; then installer_echo_exit "Problem unpacking the package. Try: $0 --user"; else installer_echo_exit "Problem unpacking the package"; fi

echo "Unpacking finished successfully"

exit 0
#-----------------------------------------------------------
#      Start of TAR.GZ file
#-----------------------------------------------------------;

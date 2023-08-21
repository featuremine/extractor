#!/bin/sh

set -e

INSTALLER_LINES=$(($(cat "$1" | wc -l)+1))
DIR="$(dirname "$3")"

mkdir -p "$DIR"
(
  cat "$1" | sed \
    -e "s/INSTALLER_LINES=/INSTALLER_LINES=$INSTALLER_LINES/g" \
    -e "s/PROJECT_NAME=/PROJECT_NAME=\"$PROJECT_NAME\"/g" \
    -e "s/PROJECT_VERSION=/PROJECT_VERSION=\"$PROJECT_VERSION\"/g"
  cat "$2"
)>"$3"
chmod +x "$3"

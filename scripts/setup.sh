#!/bin/bash

head -n 1 $MESON_SOURCE_ROOT/meson.build > $MESON_INSTALL_DESTDIR_PREFIX/meson.build;
cat $MESON_SOURCE_ROOT/install/meson.build >> $MESON_INSTALL_DESTDIR_PREFIX/meson.build;
#!/bin/bash

cat $MESON_SOURCE_ROOT/Doxyfile ; echo "PROJECT_NUMBER=$1" | doxygen;
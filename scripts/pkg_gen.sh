#!/bin/bash
PROJECT=$1
MESON_EXEC_PATH="$(echo $MESONINTROSPECT| head -n1 | cut -d " " -f1)";

pushd $MESON_BUILD_ROOT;

DESTDIR=$MESON_BUILD_ROOT/.package $MESON_EXEC_PATH install;

pushd .package/usr;

mv local ../../${PROJECT};

popd;

tar czf ${PROJECT}.tar.gz ${PROJECT};

rm -rf ${PROJECT};

popd;

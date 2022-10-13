#!/bin/bash

MESON_EXEC_PATH="$(echo $MESONINTROSPECT| head -n1 | cut -d " " -f1)";

pushd $MESON_BUILD_ROOT;

DESTDIR=$MESON_BUILD_ROOT/.package $MESON_EXEC_PATH install;

pushd .package/usr;

rm -rf local

popd;

rm -f *.whl

mkdir featuremine
mv *.tar.bz2 featuremine/;

tar czf featuremine-conda-pkg.tar.gz featuremine;

rm -rf featuremine;

popd;

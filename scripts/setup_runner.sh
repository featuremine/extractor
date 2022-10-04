#!/bin/bash

DISTRO_NAME=$(lsb_release -is | tr '[:upper:]' '[:lower:]')
DISTRO_VER=$(lsb_release -rs)

pushd $2;

PROJECT_VERSION=$4 PYTHON_MAJOR=$6 PYTHON_MINOR=$7 $1 setup.py bdist_wheel;

mv dist/*.whl $MESON_BUILD_ROOT/

$1 ../../scripts/conda_gen.py $DISTRO_NAME $DISTRO_VER ${@:3} $PWD/$3;

popd;

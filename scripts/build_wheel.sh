#!/bin/bash

if [ "$#" -lt 8 ]; then
    echo "At least one target must be added to the installable targets to run this script."
    exit 1;
fi

DISTRO_NAME=$(lsb_release -is | tr '[:upper:]' '[:lower:]')
DISTRO_VER=$(lsb_release -rs)

if [ $DISTRO_NAME = "centos" ]; then
	if [ $(echo $DISTRO_VER | sed 's/\..*$//') != "7" ]; then
		echo "Centos version used not supported, Please use Centos 7 instead."
		exit 1;
	fi
	ninja -C $MESON_BUILD_ROOT || exit 1;
elif [ $DISTRO_NAME = "fedora" ]; then
	if [ $DISTRO_VER != "29" ] ; then
		echo "Fedora version used not supported, Please use Fedora 29 instead."
		exit 1;
	fi
	ninja -C $MESON_BUILD_ROOT || exit 1;
elif [ $DISTRO_NAME = "ubuntu" ]; then
	ninja -C $MESON_BUILD_ROOT || exit 1;
else
	echo "Distribution not supported"
	exit 1;
fi

$1 $MESON_SOURCE_ROOT/scripts/wheel_gen.py ${@:4} || exit 1;
$1 $MESON_SOURCE_ROOT/scripts/conda_gen.py --distro_name $DISTRO_NAME --distro_ver $DISTRO_VER ${@:4} || exit 1;

cp $9/*.whl $2/
cp $9/*.tar.bz2 $2/

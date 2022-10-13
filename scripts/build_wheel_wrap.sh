#!/bin/bash

mkdir .$1

mkdir .$1/$1

echo "from .$1 import *" > $PWD/.$1/$1/__init__.py

echo "from featuremine.$1 import *" > $PWD/.$1/$1/$1.py

$3 $4 $5 $6 $7 $8 $9/.$1 ${@:10} $PWD/.$1/$1/__init__.py $PWD/.$1/$1/$1.py  || exit 1;

rm -rf .$1/

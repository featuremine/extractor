#!/bin/bash

dataPath=$1
if [ -f $dataPath ]; then
  for ((i = 0 ; i < 10 ; i++)); do
    echo "$i,$i,Test string $i" >> $dataPath
    sleep 0.1
  done
fi

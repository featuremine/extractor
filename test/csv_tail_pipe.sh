#!/bin/bash

echo "val1,val2,message"
for ((i = 0 ; i < 10 ; i++)); do
  echo "$i,$i,Test string $i"
  sleep 0.1
done


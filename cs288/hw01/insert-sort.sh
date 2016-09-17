#!/bin/bash

arr=($@)

for i in ${!arr[@]}; do
  swp=${arr[$i]}
  for (( j = $i; j > 0; --j )); do
    (( arr[j] < arr[j-1] )) && (( arr[j] = arr[j-1] )) || break
  done
  arr[$j]="$swp"
done

echo ${arr[@]}

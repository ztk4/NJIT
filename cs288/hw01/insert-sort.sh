#!/bin/bash

function main {
  local arr=($@)

  for i in ${!arr[@]}; do
    local swp="${arr[$i]}"
    for (( j = $i; j > 0; --j )); do
      # if swp val is less than val at j-1, shift back
     if (( swp < arr[j-1] )); then
       (( arr[j] = arr[j-1] ))
     else
       break
     fi
    done
    arr[$j]="$swp" #swp over last shifted value
  done

  echo ${arr[@]}
}

main $@

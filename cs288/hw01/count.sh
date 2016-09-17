#!/bin/bash

declare -a counts
declare -a letters

for file in $(ls /bin); do
  letter="${file:0:1}"
  (( ind = $(printf "%d" "'$letter") - 97 )) #97 is decimal of 'a'
  if [ $ind -lt 26 ]; then
    (( counts[$ind] += 1))
    letters[$ind]=$letter
  fi
done

for ind in ${!counts[*]}; do
  echo ${letters[$ind]} ${counts[$ind]}
done

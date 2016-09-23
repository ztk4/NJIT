#!/bin/bash

# ord chr
function ord {
  (( i = $(printf "%d" "'$1") ))
  echo $i
}

# chr ord
function chr {
  printf "\x$(printf %x "$1")" #convert to hex, then print \x{hex}
}

function main {
  declare -a counts

  for file in $(ls /bin); do
    (( ind = $(ord ${file:0:1}) - 97 ))  #97 is decimal of 'a'
    [ $ind -lt 26 ] && (( counts[$ind] += 1 ))
  done

  for (( i = 0; i < 26; ++i )); do
    (( o = i + 97 ))  #97 is decimal of 'a'
    [ -n "${counts[$i]}" ] && local c=${counts[$i]} || local c=0
    echo $(chr $o)  $c
  done
}

[ "$#" -eq 0 ] && { main; true; } || echo "Usage: $0"

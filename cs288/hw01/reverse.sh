#!/bin/bash

function reverse {
  (( ind = $# + 1 ))
  for item in $@; do
    result[$ind]="$item"
    (( --ind ))
  done
  
  echo ${result[@]}
}

function main {
  reverse $(ls "$1")
}

[ "$#" -eq 1 ] && { main "$1"; true; } || echo "Usage: $0 dir"

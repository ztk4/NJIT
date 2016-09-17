#!/bin/bash

# average dir
function average {
  local dir_size=($(du -cb "$1"/* | tail -n1)) # du -cb dir/* gets apparent disk usage for all files in dir, with summary as last line
  echo "$dir_size / $(ls -1U "$1" | wc -l)" |bc 
}

# filter dir avg
function filter {
  for file in $(ls "$1"); do
    local size=($(du -b "$1/$file")) # Onlt need first "word" of result, so save as array so that dereference prints first elt by default
    [ $size -gt "$2" ] && echo -n "$file "
  done

  echo
}

# main dir
function main {
  filter "$1" "$(average "$1")"
}

[ "$#" -eq 1 ] && { main "$1"; true; } || echo "Usage: $0 dir"

#!/bin/bash

# average dir
function average {
  local dir_size=0
  local dir_len=0
  
  IFS=$'\n' ls_arr=($(ls -l "$1"))
  for (( i=1; i < "${#ls_arr[@]}"; ++i )); do
    IFS=' ' file_arr=(${ls_arr[$i]})
    (( dir_size += file_arr[4] ))  #4th elt is size in bytes
    (( ++dir_len ))
  done

  (( avg = dir_size / dir_len ))
  echo $avg
}

# filter dir avg
function filter {
  IFS=$'\n' ls_arr=($(ls -l "$1"))
  for (( i=1; i < "${#ls_arr[@]}"; ++i )); do
    IFS=' ' file_arr=(${ls_arr[$i]})
    [ ${file_arr[4]} -gt "$2" ] && echo -n "${file_arr[-1]} "
  done

  echo
}

# main dir
function main {
  filter "$1" "$(average "$1")"
}

[ "$#" -eq 1 ] && { main "$1"; true; } || echo "Usage: $0 dir"

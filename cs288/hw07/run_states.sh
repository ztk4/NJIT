#!/bin/bash

while [ true ]; do 

  state=$(shuf -i 0-15)

  echo $state
  echo -e "\nastar:"
  ./hw7 $state astar

  if (( $? != 0 )); then
    echo No solution..
  else
    echo -e "\nbest: "
    ./hw7 $state best
    echo -e "\nbfs: "
    ./hw7 $state bfs
    echo -e "\ndfs: "
    ./hw7 $state dfs
  fi

  wait 1

done


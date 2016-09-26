#!/bin/bash

# depth depth breadth dir num
function depth {
  if [ "$1" -le 0 ]; then
    echo $4
    return
  fi
  local num="$4"

  (( d = $1 - 1 ))
  for (( b = 0; b < $2; ++b )); do
    (( num += 1 ))
    local dir="$3/$num"
    mkdir "$dir"
    local num="$(depth "$d" "$2" "$dir" "$num")"
  done

  echo $num
}

# breadth depth breadth dir
function breadth {
  local stack=("$3")
  local curr=1

  for (( d = 0; d < $1; ++d )); do
    local new_stack=()
    for dir in "${stack[@]}"; do
      for (( b = 0; b < $2; ++b )); do
        local new_dir="$dir/$curr"
        (( curr += 1 ))
        mkdir "$new_dir"
        new_stack+=("$new_dir")
      done
    done
    stack=(${new_stack[@]})
  done
}

# main depth breadth dir {depth|breadth}
function main {
  local dir="$3-$4"

  case "$4" in
    depth)
      mkdir "$dir"
      depth "$1" "$2" "$dir" 0 > /dev/null
      ;;
    breadth)
      mkdir "$dir"
      breadth "$1" "$2" "$dir"
      ;;
    *)
      return -1
  esac
}

[ "$#" -eq 4 ] && main $@ || echo "Usage: $0 depth breadth dir {depth|breadth}"

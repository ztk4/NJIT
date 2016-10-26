#!/bin/bash

# get_user $line
function get_user {
  local user_str="$(egrep -o '"/user/[^"]+"' <<< "$1")"
  local user_str="${user_str#'"/user/'}"
  echo "${user_str%'"'}"
}

# get_views $line
function get_views {
  local views_str="$(egrep -o '[0-9,]+ views' <<< "$1")"
  local views_str="${views_str% views}"
  echo "${views_str//,/}"
}

# get_duration $line
function get_duration {
  local duration_str="$(egrep -o "Duration: [^.]+\." <<< "$1")"
  local duration_str="${duration_str#Duration: }"
  echo "${duration_str%.}"
}

# get_id $line
function get_id {
  # Cast to array bc we only need first value
  local id_str=($(egrep -o '/watch\?v=[^"]+"' <<< "$1"))
  local id_str="${id_str#/watch?v=}"
  echo "${id_str%'"'}"
}

# get_title $line
function get_title {
  local title_str="$(egrep -o 'title="([^"]+)".*\1' <<< "$1")"
  echo "${title_str#*>}"
}

# Print Multiple
# pm str n
function pm {
  local i=
  for (( i = 0; i < $2; ++i )); do echo -n "$1"; done
}

# Print header
# ph str len
function ph {
  (( buf1 = ($2 - ${#1}) / 2 ))
  (( buf2 = ($2 - ${#1} + 1) / 2 ))  # In case odd
  pm " " $buf1
  echo -n "$1"
  pm " " $buf2
  echo -n "|"
}

# Print cell left
# pcl str len
function pcl {
  (( trail = $2 - ${#1} - 1 ))
  echo -n " $1"
  pm " " $trail
  echo -n "|"
}

# Print cell right
# pcr str len
function pcr {
  (( head = $2 - ${#1} - 1 ))
  pm " " $head
  echo -n "$1 "
  echo -n "|"

}

# main [file=index.html]
function main {
  local oLang=$LANG
  LANG='en_US.utf8'  # For getting printed string length

  if [ -z "$1" ]; then
    local file=index.html
  else
    local file="$1"
  fi

  echo "User,Views,Duration,VID,Title"
  
  # All lines we care about should contain xxx,xxx views
  IFS=$'\n' lines=($(egrep "[0-9,]+ views" "$file"))
  local line=
  for line in ${lines[@]}; do
    # Add onto storage arrays
    user="$(get_user "$line")"
    views="$(get_views "$line")"
    duration="$(get_duration "$line")"
    id="$(get_id "$line")"
    title="$(get_title "$line")"

    # Set user to N/A if video not posted by a user
    [ -z "$user" ] && user="N/A"

    echo "${user//,/DELIM_COMMA},${views//,/DELIM_COMMA},${duration//,/DELIM_COMMA},${id//,/DELIM_COMMA},${title//,/DELIM_COMMA}"
  done

  LANG=$oLang
}

[ "$#" -ge 1 ] && [ "$#" -le 2 ] && { main "$2" > "$1"; true; } || echo "Usage: $0 output.csv [file=index.html]"

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
  for (( i = 0; i < $2; ++i )); do echo -n "$1"; done
}

# Print header
# ph str len
function ph {
  oLang="$LANG" 
  LANG='en_US.utf8' # For getting displayed string length
  (( buf1 = ($2 - ${#1}) / 2 ))
  (( buf2 = ($2 - ${#1} + 1) / 2 ))  # In case odd
  LANG="$oLang"
  pm " " $buf1
  echo -n "$1"
  pm " " $buf2
  echo -n "|"
}

# Print cell left
# pcl str len
function pcl {
  oLang="$LANG" 
  LANG='en_US.utf8' # For getting displayed string length
  (( trail = $2 - ${#1} - 1 ))
  LANG="$oLang"
  echo -n " $1"
  pm " " $trail
  echo -n "|"
}

# Print cell right
# pcr str len
function pcr {
  oLang="$LANG" 
  LANG='en_US.utf8' # For getting displayed string length
  (( head = $2 - ${#1} - 1 ))
  LANG="$oLang"
  pm " " $head
  echo -n "$1 "
  echo -n "|"

}

# main [file=index.html]
function main {
  if [ -z "$1" ]; then
    local file=index.html
  else
    local file="$1"
  fi
  # Column widths are hardcoded
  # Not worth the effort to make it adjustable, pretty enough as is!
  local bar_len=180

  echo -n +
  pm - $bar_len
  echo +
  
  local user_len=22
  local views_len=13
  local duration_len=10
  local id_len=13
  # The 4 accounts for the bars between cells
  (( title_len = bar_len - user_len - views_len - duration_len - id_len - 4 ))

  echo -n "|"
  ph User $user_len
  ph Views $views_len
  ph Duration $duration_len
  ph VID $id_len
  ph Title $title_len
  echo

  echo -n "|"
  pm - $bar_len
  echo "|"

  # All lines we care about should contain xxx,xxx views
  IFS=$'\n' lines=($(egrep "[0-9,]+ views" "$file"))
  for line in ${lines[@]}; do
    local user="$(get_user "$line")"
    local views="$(get_views "$line")"
    local duration="$(get_duration "$line")"
    local id="$(get_id "$line")"
    local title="$(get_title "$line")"

    [ -z "$user" ] && local user="N/A"

    echo -n "|"
    pcl "$user" $user_len
    pcr "$views" $views_len
    pcr "$duration" $duration_len
    pcl "$id" $id_len
    pcl "$title" $title_len
    echo
  done

  echo -n +
  pm - $bar_len
  echo +

  echo "Printed ${#lines[@]} rows."
}

[ "$?" -le 1 ] && { main "$1"; true; } || echo "Usage: $0 [file=index.html]"

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
  
  local users=()
  local views=()
  local durations=()
  local ids=()
  local title=()

  local maxlen_user=0
  local maxlen_views=0
  local maxlen_duration=0
  local maxlen_id=0
  local maxlen_title=0

  # All lines we care about should contain xxx,xxx views
  IFS=$'\n' lines=($(egrep "[0-9,]+ views" "$file"))
  local line=
  for line in ${lines[@]}; do
    # Add onto storage arrays
    users+=("$(get_user "$line")")
    views+=("$(get_views "$line")")
    durations+=("$(get_duration "$line")")
    ids+=("$(get_id "$line")")
    titles+=("$(get_title "$line")")

    # Set user to N/A if video not posted by a user
    [ -z "${users[-1]}" ] && users[-1]="N/A"

    # Set maxlen if last elt added is longer than current max
    [ ${#users[-1]} -gt $maxlen_user ] && local maxlen_user=${#users[-1]}
    [ ${#views[-1]} -gt $maxlen_views ] && local maxlen_views=${#views[-1]}
    [ ${#durations[-1]} -gt $maxlen_duration ] && local maxlen_duration=${#durations[-1]}
    [ ${#ids[-1]} -gt $maxlen_id ] && local maxlen_id=${#ids[-1]}
    [ ${#titles[-1]} -gt $maxlen_title ] && local maxlen_title=${#titles[-1]}
  done

  # Account for title boxes
  [ 4 -gt $maxlen_user ] && local maxlen_user=4           # len(User)     = 4
  [ 5 -gt $maxlen_views ] && local maxlen_views=5         # len(Views)    = 5
  [ 8 -gt $maxlen_duration ] && local maxlen_duration=8   # len(Duration) = 8
  [ 3 -gt $maxlen_id ] && local maxlen_id=3               # len(VID)      = 3
  [ 5 -gt $maxlen_title ] && local maxlen_title=5         # len(Title)    = 5

  # Account for 2 space padding
  local user_len=$(( maxlen_user + 2 ))
  local views_len=$(( maxlen_views + 2 ))
  local duration_len=$(( maxlen_duration + 2 ))
  local id_len=$(( maxlen_id + 2 ))
  local title_len=$(( maxlen_title + 2 ))

  # Lengths  + (n - 1 internal bars)
  local bar_len=$(( user_len + views_len + duration_len + id_len + title_len + 4 ))

  echo -n +
  pm - $bar_len
  echo +
  
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

  local i=
  for (( i = 0; i < ${#users[@]}; ++i )); do
    echo -n "|"
    pcl "${users[$i]}" $user_len
    pcr "${views[$i]}" $views_len
    pcr "${durations[$i]}" $duration_len
    pcl "${ids[$i]}" $id_len
    pcl "${titles[$i]}" $title_len
    echo
  done

  echo -n +
  pm - $bar_len
  echo +

  echo "Printed ${#users[@]} rows."

  LANG=$oLang
}

[ "$?" -le 1 ] && { main "$1"; true; } || echo "Usage: $0 [file=index.html]"

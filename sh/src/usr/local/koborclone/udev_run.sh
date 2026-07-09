#!/bin/sh

# udev kills slow scripts
if [ "$SETSID" != "1" ]; then
    SETSID=1 setsid "$0" "$@" &
    exit
fi

# load config
. "$(dirname "$0")/config.sh"

# create config dir
[ ! -e "$CONFIG_DIR" ] && mkdir -p "$CONFIG_DIR" >/dev/null 2>&1

# run the main script and redirect output to log
"$KOBORCLONE_DIR/run.sh" > "$CONFIG_DIR/koborclone.log" 2>&1 &

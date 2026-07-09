#!/bin/sh

# prevent concurrent runs
if command -v flock >/dev/null 2>&1; then
  LOCKFILE="/tmp/koborclone.lock"
  exec 9>"$LOCKFILE"
  if ! flock -n 9; then
    echo "Another instance is already running. Exiting."
    exit 0
  fi
fi

# load config
. "$(dirname "$0")/config.sh"
export USER_CONFIG

# create lib dir
[ ! -e "$LIBRARY_DIR" ] && mkdir -p "$LIBRARY_DIR" >/dev/null 2>&1

# create user config if it doesn't exist
if [ ! -e "$USER_CONFIG" ]; then
  echo "No user config, creating at $USER_CONFIG"
  if [ -e "$USER_CONFIG_TEMPLATE" ]; then
    cp "$USER_CONFIG_TEMPLATE" "$USER_CONFIG"
  else
    : > "$USER_CONFIG"
  fi
fi

# create rclone config if it doesn't exist
if [ ! -e "$RCLONE_CONFIG" ]; then
  echo "No rclone config, creating at $RCLONE_CONFIG"
  if [ -e "$RCLONE_CONFIG_TEMPLATE" ]; then
    cp "$RCLONE_CONFIG_TEMPLATE" "$RCLONE_CONFIG"
  else
    : > "$RCLONE_CONFIG"
  fi
fi

# uninstall if a file named UNINSTALL exists (ignore case and extension)
for f in "$CONFIG_DIR"/*; do
  [ -e "$f" ] || continue
  name=${f##*/}      # strip directory
  name=${name%%.*}   # strip extension
  if [ "$(printf '%s' "$name" | tr '[:upper:]' '[:lower:]')" = uninstall ]; then
    exec "$KOBORCLONE_DIR/uninstall.sh"
  fi
done

# check internet connection
retries=0
while ! wget -q -O /dev/null "http://detectportal.firefox.com/success.txt" 2>/dev/null; do
  echo "$($DT) Waiting for internet connection, retry $retries"
  retries=$((retries + 1))
  if [ "$retries" -gt 60 ]; then
    echo "$($DT) ERROR! No internet connection detected"
    exit 1
  fi
  sleep 1
done

# check for qbdb
if [ ! -f "/usr/bin/qndb" ]; then
  echo "NickelDBus not found, installing..."
  wget "https://github.com/shermp/NickelDBus/releases/latest/download/KoboRoot.tgz" -O - | tar xz -C /
fi

# check for rclone
if [ ! -x "${RCLONE}" ]; then
  echo "rclone missing: binary at ${RCLONE}"
  exit 1
fi

changes=false

# loop through each line in the user config file
while IFS= read -r url || [ -n "$url" ]; do
  url="${url#"${url%%[! ]*}"}"
  case "$url" in
    ''|'#'*) continue ;; # skip empty lines and comments
  esac
  dir="$LIBRARY_DIR/$(printf '%s' "$url" | sed 's/:/\//g')"
  mkdir -p "$dir"

  echo "$($DT) Syncing $url"
  "$RCLONE" copy \
    --ca-cert "$KOBORCLONE_DIR/cacert.pem" \
    --transfers 1 \
    --cache-dir "$RCLONE_CACHE_DIR" \
    --log-level INFO \
    --error-on-no-transfer \
    --stats 0 \
    --config "$RCLONE_CONFIG" \
    "$url" "$dir"
  rclone_exit=$?

  case "$rclone_exit" in
    0) changes=true ;;  # files were transferred
    9) ;;               # nothing transferred
    *) echo "$($DT) ERROR: rclone failed for $url (exit code $rclone_exit)" ;;
  esac
done < "$USER_CONFIG"

# refresh library if required
if [ "$changes" = false ]; then
  echo "No files changed, skipping re-scan"
else
  echo "Files have changed, re-scanning library"

  # use NickelDBus for library refresh
  /usr/bin/qndb -t 3000 -s pfmDoneProcessing -m pfmRescanBooksFull
fi

echo "$($DT) done"

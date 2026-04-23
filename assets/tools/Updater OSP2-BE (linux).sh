#!/bin/sh

HELPMSG="osp2-be updater
  --help, -h        show this menu
  --silent, -s      skip printing changelog
  --launch, -l      launch the game after updating
  --update-only     only update files, do not launch the game"

if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
  echo "$HELPMSG"
  exit 0
fi

# === SETTINGS ===
ARGS="+set sv_pure 0
      +set fs_game osp
      +set ui_cdkeychecked 1
      +set com_introplayed 1
      +set r_ingamevideo 0
      +set com_skipidlogo 1"

CURL='/usr/bin/curl'
url="https://scoqx.github.io/assets/zz-osp-pak8be.pk3"
url_version="https://scoqx.github.io/version.json?v="
url_changelog="https://scoqx.github.io/assets/changelog_be.txt"

# === PATHS ===
SCRPATH="$(cd -- "$(dirname -- "$0")" && pwd -P)"
osp_dir="$SCRPATH/osp"
osp_path="$osp_dir/zz-osp-pak8be.pk3"
version_path="$SCRPATH/.ospver"
game_bin="$SCRPATH/quake3e-vulkan.x64"

# === CHECK FOLDER ===
if [ ! -d "$osp_dir" ]; then
  echo "osp folder not found, creating one..."
  mkdir -p "$osp_dir"
fi

# === CHECK VERSION ===
echo -n "last version: "

version="$($CURL -s "$url_version" | jq -r '.version')"
echo -e "\033[1m$version\033[0m"

if [ -f "$version_path" ]; then
  current_version="$(cat "$version_path" | tr -d '\r\n')"
else
  current_version=""
fi

if [ "$version" = "$current_version" ] && [ -e "$osp_path" ]; then
  echo "you're on the latest version"
else
  echo "a new version found, downloading..."
  $CURL -# -o "$osp_path" "$url"

  echo "$version" > "$version_path"

  if [ "$1" != "--silent" ] && [ "$1" != "-s" ]; then
    version_fmt=$(echo "$version" | sed -r 's/-/ v/g')
    changelog="$($CURL -s "$url_changelog" | awk "/$version_fmt/,0")"

    echo
    echo "===== OSP2-BE UPDATE ====="
    echo "$changelog"
    echo "=========================="
    echo
  fi
fi

echo -e "\033[1m-- update complete --\033[0m"

# === LAUNCH GAME (optional) ===
if [ "$1" = "--launch" ] || [ "$1" = "-l" ]; then
  if [ -x "$game_bin" ]; then
    echo -e "\033[1m-- launching the game --\033[0m"
    "$game_bin" $ARGS
  else
    echo "Error: game binary not found or not executable: $game_bin"
  fi
else
  echo "Game not launched (use --launch or -l to start it)."
fi

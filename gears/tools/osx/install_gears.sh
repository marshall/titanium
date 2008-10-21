#!/bin/bash

# Install Gears from build directory
# Usage: install_gears.sh [bin|opt]

SCRIPT_DIR=`pwd`/`dirname $0`
GEARS_DIR="$SCRIPT_DIR/../.."
BASE_DIR="$GEARS_DIR/bin-$1/installers/Safari"
GEARS_PLUGIN_PATH="/Library/Internet Plug-Ins/Gears.plugin"

#default to bin-dbg
if [ ! -n "$1" ]; then
    BASE_DIR="$GEARS_DIR/bin-dbg/installers/Safari"
fi

if [ ! -d "$BASE_DIR" ]; then
  echo "Unable to install Gears from directory \"$BASE_DIR\" which doesn't exist."
  exit 1
fi

echo "Removing old Gears plugin from '$HOME/$GEARS_PLUGIN_PATH'"
rm -Rf "$HOME/$GEARS_PLUGIN_PATH"

echo "Removing old Gears plugin from '$GEARS_PLUGIN_PATH'"
sudo rm -Rf "$GEARS_PLUGIN_PATH"

echo "Installing new Gears plugin into '$GEARS_PLUGIN_PATH' from '$BASE_DIR/Gears.plugin'"
sudo cp -r "$BASE_DIR/Gears.plugin" "$GEARS_PLUGIN_PATH"

echo "Installing Gears InputManager from '$BASE_DIR/GearsEnabler'"
"$SCRIPT_DIR/install_inputmanager.sh" "$BASE_DIR/GearsEnabler"

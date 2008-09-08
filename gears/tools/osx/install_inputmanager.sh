#!/bin/bash

# Leopard will only accept InputManagers if they're permissions are set just so.
# This script does the dirty work of installing the Gears InputManager 
# correctly.

SCRIPT_DIR=`pwd`/`dirname $0`
SOURCE_DIR="$1"
if [ ! -n "$1" ]; then
    SOURCE_DIR=$SCRIPT_DIR/build/Release/GearsEnabler
fi

# Sanity Check.
if [ ! -d "$SOURCE_DIR" ]; then
  echo "Unable to install InputManager from directory \"$SOURCE_DIR\" which doesn't exist."
  exit 1
fi

if [ ! -f "$SOURCE_DIR/Info" ]; then
  echo "Directory \"$SOURCE_DIR\" doesn't appear to contain a valid InputManager."
  exit 1
fi

# Make sure script is run as root.
if [ $USER != 'root' ]; then
  sudo $0 $*
  exit 0
fi

INPUT_MGR_DIR="/Library/InputManagers/"
BUNDLE_DESTINATION="$INPUT_MGR_DIR/GearsEnabler"

echo "Installing InputManager from \"$SOURCE_DIR\"."

mkdir -p "$INPUT_MGR_DIR"
chmod 755 "$INPUT_MGR_DIR"
rm -rf "$BUNDLE_DESTINATION"
cp -R "$SOURCE_DIR" "$BUNDLE_DESTINATION"
chmod -R go-w "$BUNDLE_DESTINATION"
chown -R root:admin "$BUNDLE_DESTINATION"

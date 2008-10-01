#!/bin/sh

rm -rf ~/Library/Internet\ Plug-Ins/Titanium.plugin

cd ../plugin/titanium_plugin
./generate_plugin.sh

cd ../../app
/Applications/Safari.app/Contents/MacOS/Safari titanium.html

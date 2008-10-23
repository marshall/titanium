#!/bin/sh

rm -rf ~/Library/Internet\ Plug-Ins/Titanium.plugin

cd ../plugin/titanium_plugin
./generate_plugin.sh

#cd ../../app
#/Applications/Safari.app/Contents/MacOS/Safari titanium.html

cd ../../webkit_shell/osx/build/Debug/webkit_shell.app/Contents/MacOS/

./webkit_shell 'http://localhost/titanium/titanium.html'

#!/bin/sh

SRC_DIR=`pwd`
PLUGIN_NAME=Titanium
PLUGIN_DIR=build/$PLUGIN_NAME.plugin

mkdir $PLUGIN_DIR
mkdir $PLUGIN_DIR/Contents
mkdir $PLUGIN_DIR/Contents/MacOS
mkdir $PLUGIN_DIR/Contents/Resources

cd $PLUGIN_DIR/Contents
cp -r ../../../English.lproj Resources
cp ../../../Info.plist ../../../version.plist .
cp $SRC_DIR/../../scintilla/bin/libscintilla.a MacOS
cp ../../Debug/libtitanium_plugin.dylib "MacOS/$PLUGIN_NAME"

cd ../../..
cp -r $PLUGIN_DIR ~/Library/Internet\ Plug-ins/

rm -rf $PLUGIN_DIR

#!/bin/sh
sudo rm -rf /Users/marshall/Library/Internet Plug-Ins/Titanium.plugin
./generate_plugin.sh

cd ../../app
./run.sh

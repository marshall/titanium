#!/bin/sh
# must be root!

APPC_PATH=/Library/Appcelerator

mkdir $APPC_PATH/titanium
mkdir $APPC_PATH/titanium/plugins

ln -s sdk/support $APPC_PATH/titanium/support
ln -s sdk/lib/launcher.rb $APPC_PATH/lib/launcher.rb
ln -s sdk/lib/packager.rb $APPC_PATH/lib/packager.rb
ln -s sdk/commands/launch_project.rb $APPC_PATH/commands/launch_project.rb
ln -s sdk/commands/package_project.rb $APPC_PATH/commands/package_project.rb
ln -s runtime/plugins/tiplugin.rb $APPC_PATH/titanium/tiplugin.rb
ln -s runtime/src/titanium.js $APPC_PATH/titanium/titanium.js
ln -s runtime/webkit_shell $APPC_PATH/titanium/webkit_shell

cd plugins/gears
mkdir $APPC_PATH/titanium/plugins/gears
rake && unzip -o ../../stage/tiplugin_gears*.zip -d $APPC_PATH/titanium/plugins/gears

cd ../jquery
mkdir $APPC_PATH/titanium/plugins/jquery
rake && unzip -o ../../stage/tiplugin_jquery*.zip -d $APPC_PATH/titanium/plugins/jquery

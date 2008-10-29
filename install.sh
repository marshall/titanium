#!/bin/sh
# must be root!

APPC_PATH=/Library/Appcelerator
PWD=`pwd`
mkdir $APPC_PATH/titanium
mkdir $APPC_PATH/titanium/plugins

sudo ln -fs $PWD/sdk/support $APPC_PATH/titanium/support
sudo ln -fs $PWD/sdk/lib/launcher.rb $APPC_PATH/lib/launcher.rb
sudo ln -fs $PWD/sdk/lib/packager.rb $APPC_PATH/lib/packager.rb
sudo ln -fs $PWD/sdk/commands/launch_project.rb $APPC_PATH/commands/launch_project.rb
sudo ln -fs $PWD/sdk/commands/package_project.rb $APPC_PATH/commands/package_project.rb
sudo ln -fs $PWD/sdk/commands/install_tiplugin.rb $APPC_PATH/commands/install_tiplugin.rb
sudo ln -fs $PWD/runtime/plugins/tiplugin.rb $APPC_PATH/titanium/tiplugin.rb
sudo ln -fs $PWD/runtime/src/titanium.js $APPC_PATH/titanium/titanium.js
sudo ln -fs $PWD/runtime/webkit_shell $APPC_PATH/titanium/webkit_shell

cd runtime/webkit_shell/osx
xcodebuild

ln -s build/Release/webkit_shell.app webkit_shell.app
cd ../../..

cd runtime/plugins/gears
mkdir $APPC_PATH/titanium/plugins/gears
rake && unzip -o ../../stage/tiplugin_gears*.zip -d $APPC_PATH/titanium/plugins/gears


cd ../jquery
mkdir $APPC_PATH/titanium/plugins/jquery
rake && unzip -o ../../stage/tiplugin_jquery*.zip -d $APPC_PATH/titanium/plugins/jquery

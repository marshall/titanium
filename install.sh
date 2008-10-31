#!/bin/sh
# must be root!

APPC_PATH=/Library/Appcelerator
PWD=`pwd`

sudo ln -fs $PWD/sdk/lib/launcher.rb $APPC_PATH/lib/launcher.rb
sudo ln -fs $PWD/sdk/lib/packager.rb $APPC_PATH/lib/packager.rb
sudo ln -fs $PWD/sdk/lib/titanium.rb $APPC_PATH/lib/titanium.rb
sudo ln -fs $PWD/sdk/commands/launch_project.rb $APPC_PATH/commands/launch_project.rb
sudo ln -fs $PWD/sdk/commands/package_project.rb $APPC_PATH/commands/package_project.rb
sudo ln -fs $PWD/sdk/commands/install_tiplugin.rb $APPC_PATH/commands/install_tiplugin.rb

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

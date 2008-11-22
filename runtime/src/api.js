//
// Copyright 2006-2008 Appcelerator, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// These are JS wrappers for our native callbacks so we can assign extra functionality in pure JS

ti.App = {
	include: function(url) { TiApp.include(url); },
	debug: function(message) { TiApp.debug(message); },
	quit: function() { TiApp.quit(); },
	show: function() { TiApp.show(); },
	activate: function() { TiApp.activate(); },
	minimize: function() { TiApp.minimize(); },
	maximize: function() { TiApp.maximize(); },
	beep: function() { TiApp.beep(); },
	playSoundNamed: function(name) { TiApp.playSoundNamed(name); }
};

ti.Dock = {
	setIcon: function(icon) { TiDock.setIcon(icon); },
	setBadge: function(badge) { TiDock.setBadge(badge); },
	notify: function(severity) { TiDock.notify(severity); }
};

ti.Window = {
	createWindow: function() { return TiWindow.createWindow(); },
	createWindowOptions: function() { return TiWindow.createWindowOptions(); }
};

ti.Menu = {
	createSystemMenu: function() { return TiMenu.createSystemMenu(); },
	createUserMenu: function() { return TiMenu.createUserMenu(); }
};


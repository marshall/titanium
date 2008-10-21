// Copyright 2006, Google Inc.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Bootstraps an autoupdater into the component manager.

const Cc = Components.classes;
const Ci = Components.interfaces;

const OUR_COMPILED_JS = "updater.js";
const OUR_RDF_ID = "{000a9d1c-beef-4f90-9363-039d445309b8}";
// OUR_RDF_ID must match our <em:id> tag in install.rdf.m4
// This value must NOT change between versions, or autoupdate will break.

const OUR_MODULE_ID = "{b11ba0a3-e7d8-4f42-9c21-4dcb11fedbea}";
const OUR_MODULE_NAME = "@google.com/gears/updater"; // [naming]
const OUR_MODULE_OBJECT = "GearsUpdater"; // [naming]
// OUR_MODULE_OBJECT must be a unique name, but it is not programmer-visible.
// The module values CAN change between versions without breaking autoupdate,
// and they probably SHOULD change if OUR_RDF_ID changes. See the comments in
// install.rdf.m4 for more on this.

try {
  loadScript(OUR_COMPILED_JS);
  compiledJsLoaded = true;
} catch (e) {
  throw new Error("Error installing Appcelerator Titanium updater: " +  // [naming]
                  e.message); 
}

var our_module = new G_JSModule();
  
/**
 * Window observer that listens for a window to open,
 * then starts the extension updater timer.
 * If the updater has already been initialized, the listener does nothing.
 */
var initAutoUpdater = {
  updater: new G_ExtensionUpdater(OUR_RDF_ID),
  updaterInitialized: false,
  
  /**
   * Callback for the nsIObserver interface
   */
  observe: function(subject, topic, data) {
    if (!this.updaterInitialized) {
      // We cannot access the update manager until after the first
      // browser window opens.
      if (topic == "app-startup") {
        var windowWatcher = Cc["@mozilla.org/embedcomp/window-watcher;1"]
                            .getService(Ci.nsIWindowWatcher);
        windowWatcher.registerNotification(this);
      } else if (topic == "domwindowopened") {
        this.updaterInitialized = true;
        this.updater.UpdatePeriodically(true);
      }
    }
  },

  /**
   * Let XPCOM objects know that this is an observer
   *
   * @param {String} iid An interface id. 
   */
  QueryInterface: function(iid) {
    if (iid.equals(Ci.nsISupports) || iid.equals(Ci.nsIObserver)) {
      return this;
    }
    
    throw Components.results.NS_NOINTERFACE;
  },
};

our_module.registerObject(OUR_MODULE_ID,
                          OUR_MODULE_NAME, 
                          OUR_MODULE_OBJECT,
                          initAutoUpdater,
                          ["app-startup"]);

function NSGetModule() {
  return our_module;
}

function loadScript(libPath) {
   Cc["@mozilla.org/moz/jssubscript-loader;1"]
     .getService(Ci.mozIJSSubScriptLoader)
     .loadSubScript(getLibUrl(libPath));
}

function getLibUrl(path) {
  var file = __LOCATION__.clone().parent.parent;
  var parts = path.split("/");

  file.append("lib");

  for (var i = 0, part; part = parts[i]; i++) {
    file.append(part);
  }

  if (!file.exists()) {
    throw new Error("Specified library {" + file.path + "} does not exist");
  }

  return Cc["@mozilla.org/network/protocol;1?name=file"]
           .getService(Ci.nsIFileProtocolHandler)
           .getURLSpecFromFile(file);
}


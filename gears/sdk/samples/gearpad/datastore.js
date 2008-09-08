// Copyright 2007, Google Inc.
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

// Datastore maintains the last used current version and communicate with
// the server to sync and post changes.

// If localMode is enabled, DataStore mirrors all changes into the
// database so that they can be retrieved on startup.

function DataStore(syncCallback) {
  this.localMode = false;
  this.online = true;
  this.version = 0;
  this.clientId = 0;

  this.isFirstSync_ = true;
  this.dirty_ = false;
  this.syncCallback_ = syncCallback;
  this.abortLastRequest_ = function(){};

  this.initUser_();
  this.initLocalMode_();
}

DataStore.prototype.initUser_ = function() {
  var cookie = readCookie('c');

  if (cookie) {
    cookie = cookie.split('-');
    this.userId = cookie[0];
    this.email = unescape(cookie[2]);
  }

  if (!this.userId || !this.email) {
    location.href = 'login.php';
  }
};

DataStore.prototype.initLocalMode_ = function() {
  if (!gears.canGoLocal) {
    return;
  }

  // If the current user doesn't have local state, that means they aren't a 
  // local mode user. Carry on in server mode.
  var state = gears.getUser(this.userId);
  if (!state) {
    return;
  }

  this.version = state.version;
  this.clientId = state.clientid;
  this.dirty_ = state.dirty;
  this.localMode = true;
};

DataStore.prototype.sync = function(clientUpdate) {
  if (!this.clientId) {
    this.initClient_();
    return;
  }

  // Strict comparison to null so we don't confuse null with updates to empty
  // string.
  if (clientUpdate !== null) {
    this.dirty_ = true;

    if (this.localMode) {
      gears.execute('update user set dirty = 1, content = ? where id = ?', 
                    [clientUpdate, this.userId]);
    }
  }

  this.abortLastRequest_();

  if (this.dirty_) {
    this.updateServer_(clientUpdate);
  } else {
    this.syncFromServer_();
  }
};

DataStore.prototype.initClient_ = function() {
  var self = this;
  doRequest("GET", "init.php", null, 
    function(status, statusText, responseText) {
      var responseText =
        self.parseServerResponse_(status, statusText, responseText);
      if (!responseText) {
        return;
      }

      self.clientId = responseText[0];

      // we are upgrading from an old schema that didn't have clientID. Save it.
      if (self.localMode) {
        gears.execute('update user set clientid = ? where id = ?',
                [responseText[0], self.userId]);
        // ... and then do the first sync
        self.sync(null);
        return;
      }

      // otherwise we are just initializting a regular client
      self.version = responseText[1];
      responseText.splice(0, 2);
    
      // Tell the UI the result.
      self.handleSync_(responseText.join("\n"));
    }
  );
};

DataStore.prototype.updateServer_ = function(clientUpdate) {
  var self = this;
  this.abortLastRequest_ = doRequest("POST", "update.php",
    {version: this.version, client: this.clientId},
    function(status, statusText, responseText) {
      var response = self.parseSyncResponse_(status, statusText, responseText);
      if (!response) {
        self.handleSync_(null);
        return;
      }

      // check to see if the server says there is a conflict
      if (clientUpdate && response.content) {
        if (promptToOverrideConflict()) {
          self.version = response.version;
          self.sync(clientUpdate);
          return;
        }
      }

      if (self.localMode) {
        var sets = ['dirty = ?'];
        var vals = [0];

        if (response.version) {
          sets.push('version = ?');
          vals.push(response.version);
        }

        if (response.content) {
          sets.push('content = ?');
          vals.push(response.content);
        }

        vals.push(self.userId);
        gears.execute('update user set ' + sets.join(', ') + ' where id = ?',
                vals);
      }

      self.dirty_ = false;
      if (response.version) {
        self.version = response.version;
      }
      self.handleSync_(response.content);
    },
    clientUpdate
  );
};

DataStore.prototype.syncFromServer_ = function() {
  var self = this;
  this.abortLastRequest_ = doRequest("GET", "sync.php", {version: this.version},
    function(status, statusText, responseText) {
      var response = self.parseSyncResponse_(status, statusText, responseText);

      // If there was a change, update our internal state.
      if (response && response.content !== null) {
        if (self.localMode) {
          gears.execute('update user set version = ?, content = ? where id = ?',
            [response.version, response.content, self.userId]);
        }

        self.version = response.version;
      }

      self.handleSync_(response && response.content);
    }
  );
};

DataStore.prototype.handleSync_ = function(serverUpdate) {
  if (serverUpdate !== null) {
    this.syncCallback_(serverUpdate);
  } else if (this.localMode && this.isFirstSync_) {
    this.syncCallback_(
      gears.executeToObjects('select content from user where id = ?',
                             [this.userId])[0].content);
  } else {
    this.syncCallback_(null);
  }

  this.isFirstSync_ = false;
};

DataStore.prototype.parseServerResponse_ = 
    function(status, statusText, responseText) {
  if (!status) {
    this.online = false;
    this.warn('Could not connect to server');
    return null;
  }

  if (status != '200') {
    this.online = false;
    this.warn('Unexpected response: ', status, statusText, responseText);
    return null; 
  }

  this.online = true;
  this.log('Got response: ', status, statusText, responseText);
  return responseText.split("\n");
};

DataStore.prototype.parseSyncResponse_ = 
    function(status, statusText, responseText) {
  var responseText = 
    this.parseServerResponse_(status, statusText, responseText);
  if (!responseText) {
    return null;
  }

  // The first line, if any, should be the new version on the server.
  var version = responseText[0] || null;
  var content = null;

  if (version) {
    version = parseInt(version);
  }

  // The remaining lines, if any, are the new content on the server.
  if (responseText.length > 1) {
    responseText.splice(0, 1);
    content = responseText.join("\n");
  }

  return {version:version, content:content};
};

DataStore.prototype.log = function() {
  arguments[0] = 'DataStore: ' + arguments[0];
  console.log.apply(console, arguments);
};

DataStore.prototype.warn = function() {
  arguments[0] = 'DataStore: ' + arguments[0];
  console.warn.apply(console, arguments);
};

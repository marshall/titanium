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

function Gears() {
  this.initFactory_();

  this.hasGears = Boolean(this.factory_);
  if (!this.hasGears) {
    return;
  }

  this.db_ = this.factory_.create('beta.database');
  this.localServer_ = 
      this.factory_.create('beta.localserver');

  this.db_.open();
  try {
    this.db_.execute('select * from user');
    this.hasDb = true;
  } catch (e) {
    this.hasDb = false;
  }

  this.app_ = this.localServer_.openManagedStore('gearpad');
  this.isCaptured = this.app_ && this.app_.currentVersion;
  this.canGoLocal = this.hasGears && this.hasDb && this.isCaptured;

  this.upgradeDatabase_();
}

Gears.NOBODY_USER_ID = -1;

Gears.prototype.transact = function(fn) {
  // TODO: Implement nested transaction support if necessary.
  this.execute('begin');
  try {
    fn();
  } catch (e) {
    this.execute('rollback');
    throw e;
  }
  this.execute('commit');
};

Gears.prototype.capture = function() {
  console.log('Checking for updates...');
  this.app_ = this.localServer_.createManagedStore('gearpad');
  this.app_.manifestUrl = 'manifest.php';
  this.app_.checkForUpdate();

  // ugly^2, but necessary, because Pocket IE doesn't support
  // setInterval or function argument for setTimeout
  Gears.app = this.app_;
  console.log('starting check for update');
  Gears.checkForUpdate = function() {
    console.log('update status: ' + Gears.app.updateStatus);

    if (Gears.app.updateStatus == 3) { // error
      console.warn('update failed: ' + Gears.app.lastErrorMessage);
    } else if (Gears.app.updateStatus == 0) { // ok
      location.reload();
    } else {
      window.setTimeout('Gears.checkForUpdate();', 500);
    }
  }
  window.setTimeout('Gears.checkForUpdate();', 500);
};

Gears.prototype.createDatabase = function() {
  var schema = [
      // Stores information about each user who has setup local mode on this
      // browser.
      'create table user (' +
      // the id of the user on the server
      'id int not null primary key, ' +
      // the last version from the server that we got
      'version int not null, ' + 
      // whether the localstore is dirty wrt the server
      'dirty int not null, ' +
      // cookie that should be used when this user is active
      'cookie text not null, ' +
      // current content of note
      'content text not null, ' +
      // unique ID of this client
      'clientid int not null default 0)',
      // Version table. Not used for anything except indicating current version
      // through presence. We use a table to exploit a SQLite feature where we
      // are warned if the db physical schema changes.
      'create table version_2 (foo text)'
  ];

  var self = this;
  this.transact(function() {
    for (var i = 0, stmt; stmt = schema[i]; i++) {
      self.execute(stmt);
    }
  });
};

Gears.prototype.upgradeDatabase_ = function() {
  if (!this.hasDb) {
    return;
  }

  var version = 1;
  var rslt = this.executeToObjects(
      "select name from sqlite_master where tbl_name like 'version_%'")[0];
  if (rslt) {
    // version 1 did not have a version table
    version = Number(rslt.name.match(/version_(\d+)/)[1]);
  }

  // Upgrade users from version 1
  if (version < 2) {
    this.execute('alter table user add clientid int not null default 0');
    this.execute('create table version_2 (foo text)');
  }
};

Gears.prototype.addUser = function(userid, cookie, content, version, clientid) {
  var self = this;
  this.transact(function() {
    var rslt = self.executeToObjects('select * from user where id = ?', 
        [userid])[0];

    if (rslt) {
      return;
    }

    self.execute('insert into user values (?, ?, ?, ?, ?, ?)', 
                 [userid, version, 0, cookie, content, clientid]);
  });
};

Gears.prototype.getUser = function(userId) {
  return this.executeToObjects('select * from user where id = ?', [userId])[0];
};

Gears.prototype.executeToObjects = function(sql, args) {
  var rs = this.execute(sql, args);
  try {
    var rv = [];
    if (rs && rs.isValidRow()) {
      var cols = rs.fieldCount();
      var colNames = [];
      for (var i = 0; i < cols; i++) {
        colNames.push(rs.fieldName(i));
      }

      while (rs.isValidRow()) {
        var h = {};
        for (i = 0; i < cols; i++) {
          h[colNames[i]] = rs.field(i);
        }
        rv.push(h);
        rs.next();
      }
    }
  } catch (e) {
    throw e;
  } finally {
    rs.close();
    return rv;
  }
}

/**
 * Helper that executes a sql and throws any error that occurs. Errors that come
 * from Gears are not real errors in Firefox, so they don't get handled by the
 * error reporting UI correctly.
 */
Gears.prototype.execute = function(sql, args) {
  console.log('sql: %s, args: %s', sql, args);
  return this.db_.execute(sql, args);
};

Gears.prototype.initFactory_ = function() {
  // Firefox
  if (typeof GearsFactory != 'undefined') {
    this.factory_ = new GearsFactory();
    return;
  }

  try {
    var factory = new ActiveXObject("Gears.Factory");
      // privateSetGlobalObject is only required and supported on WinCE.
      if (factory.getBuildInfo().indexOf('ie_mobile') != -1) {
        factory.privateSetGlobalObject(window);
      }
    this.factory_ = factory;
  } catch (e) {
  // no Safari for now
  }
};

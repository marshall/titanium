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

// JavaScript for the main ui of Gearpad.
// Watch the textarea for changes, and when they occur send them off to
// DataStore.

var t = DOM.getElementsByTagName(document,"textarea")[0];
var lastSavedVal = t.value;
var saveTimeout = 500; // 500 milliseconds of quiet before autosave
var idleTimeout = 5000; // 5 seconds idle sync
var gears;
var timerId;
var snapshot;
var store;

function init() {
  gears = new Gears();
  store = new DataStore(onsync);

  DOM.getElementById("login-bar").style.display = "";
  DOM.getElementById("logged-in-as").innerHTML = store.email;

  if (DOM.is_pocket_ie) {
    t.style.height = (screen.availHeight - 140) + 'px';
    // can't set the width of a textarea in Pocket IE :(
  }

  if (store.localMode) {
    DOM.getElementById("setup-offline").style.display = "none";
  }

  DOM.listen(t, "keyup", keypress);
  DOM.listen(t, "keypress", keypress);

  sync();
}

function keypress() {
  scheduleSync(saveTimeout);
}

function focused(e) {
  if (!e) e = window.event;
  if (e.stopPropagation) e.stopPropagation();

  sync();

  return false;
}

function scheduleSync(timeout) {
  cancelSchedule();
  timerId = window.setTimeout(sync, timeout);
}

function cancelSchedule() {
  if (timerId) {
    timerId = window.clearTimeout(timerId);
  }
}

function sync() {
  cancelSchedule();
  snapshot = t.value;

  if (t.value != lastSavedVal) {
    setStatus("Saving...", "orange");
    store.sync(snapshot);
  } else {
    setStatus("Syncing...", "orange");
    store.sync(null);
  }
}

function onsync(newContent) {
  // If the textbox has changed since the sync was sent, then the result is
  // invalid and the sync must be redone. This can happen since syncs from 
  // typing are delayed and server requests are asynchronous.
  if (snapshot != t.value) {
    sync();
    return;
  }

  if (newContent !== null) {
    t.value = newContent;
  }

  lastSavedVal = t.value;

  if (store.online) {
    setStatus('Online.', 'green');
  } else {
    setStatus('Offline.', 'orange');
  }

  scheduleSync(idleTimeout);
}

function setStatus(msg, color) {
  var elm = DOM.getElementById("status")
  elm.className = color;
  elm.innerHTML = msg;
}

function logout() {
  eraseCookie("c");
  location.href = "login.php";
}

function setupOffline() {
  if (!gears.hasGears) {
    location.href = 'getgears.php';
    return;
  }

  if (!gears.hasDb) {
    gears.createDatabase();
  }

  gears.addUser(store.userId,
                unescape(readCookie('c')),
                t.value,
                store.version,
                store.clientId);

  if (!gears.isCaptured) {
    gears.capture();
  } else {
    location.reload();
  }
}

function promptToOverrideConflict() {
  return confirm(
     'The note has been changed on another computer. Override changes?');
}

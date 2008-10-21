// Copyright 2008, Google Inc.
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

function testNotificationApi() {
  var desktop = google.gears.factory.create('beta.desktop');
  assert(isObject(desktop));

  var notification = desktop.createNotification();
  assert(isObject(notification));

  // Make sure new ID is generated each time.
  assertEqual('string', typeof(notification.id));
  assertNotEqual('', notification.id);
  var notification2 = desktop.createNotification();
  assertEqual('string', typeof(notification2.id));
  assertNotEqual('', notification2.id);
  assertNotEqual(notification.id, notification2.id);

  // Check other properties.
  var ID = 'ID';
  notification.id = ID;
  assertEqual(ID, notification.id);

  var TITLE = 'Title';
  notification.title = TITLE;
  assertEqual(TITLE, notification.title);

  var SUBTITLE = 'Subtitle';
  notification.subtitle = SUBTITLE;
  assertEqual(SUBTITLE, notification.subtitle);

  var DESCRIPTION = 'Description';
  notification.description = DESCRIPTION;
  assertEqual(DESCRIPTION, notification.description);

  var ICON = 'http://icon';
  notification.icon = ICON;
  assertEqual(ICON, notification.icon);

  var DISPLAY_AT_TIME = new Date();
  notification.displayAtTime = DISPLAY_AT_TIME;
  assertEqual(notification.displayAtTime.toUTCString(),
              DISPLAY_AT_TIME.toUTCString());

  var DISPLAY_UNTIL_TIME = new Date();
  notification.displayUntilTime = DISPLAY_UNTIL_TIME;
  assertEqual(notification.displayUntilTime.toUTCString(),
              DISPLAY_UNTIL_TIME.toUTCString());              

  notification.addAction('View', 'http://view/...');
  notification.addAction('Snooze', 'http://snooze/...');
}

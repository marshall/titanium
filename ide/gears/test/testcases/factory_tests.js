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

function testCheckVersionProperty() {
  var versionComponents = google.gears.factory.version.split('.');
  var major = versionComponents[0];
  var minor = versionComponents[1];
  assertEqual(4, versionComponents.length);
  assertEqual('0', major);
  assertEqual('4', minor);
}

function testCreateValidModules() {
  // optional version param
  assert(isObject(google.gears.factory.create('beta.timer')));

  // explicit version param (real-world code does this!)
  assert(isObject(google.gears.factory.create('beta.timer', '1.0')));
}

function testCreateInvalidModules() {
  // bad module name
  assertError(function() {
    google.gears.factory.create('invalid');
  });

  // bad module version formats
  assertError(function() {
    google.gears.factory.create('beta.database', '1');
  });
  assertError(function() {
    google.gears.factory.create('beta.database', '1.');
  });

  // bad module version number
  assertError(function() {
    google.gears.factory.create('beta.database', '42.0');
  });
}

function testDisallowDirectObjectCreation() {
  // JS code should only be able to instantiate the GearsFactory object;
  // everything else should go through GearsFactory.
  var objects = [
    'GearsDatabase',
    'GearsHttpRequest',
    'GearsResultSet',
    'GearsWorkerPool',
    'GearsLocalServer',
    'GearsResourceStore' // testing one indirect localserver interface is enough
  ];

  for (var i = 0; i < objects.length; ++i) {
    // test 'new FOO()'
    //  and 'new ActiveXObject("Gears.FOO")'
    var prefix_suffix = [
      ['new ',                       '();'],
      ['new ActiveXObject("Gears.',  '");']
    ];

    for (var j = 0; j < prefix_suffix.length; ++j) {
      assertError(function() {
        var toEval = prefix_suffix[j][0] + objects[i] + prefix_suffix[j][1];
        var instance = eval(toEval);
      });
    }
  }
}

function testGetPermissionMultipleTimes() {
  // The running unittests must already have permission to use Gears.
  // So only test that calling getPermission() multiple times does not throw.
  var hasPermission = false;
  hasPermission = google.gears.factory.getPermission('', '', '');
  assertEqual(hasPermission, true);
  hasPermission = google.gears.factory.getPermission('', '', '');
  assertEqual(hasPermission, true);
}

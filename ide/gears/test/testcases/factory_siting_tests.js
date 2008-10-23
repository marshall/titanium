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

// Tests the use of privateSetGlobalObject.

function testSiting() {
  // Local functions.
  function createLocalFactory() {
    // This is taken from gears_init.js.
    var factory = null;
    // Firefox
    if (typeof GearsFactory != 'undefined') {
      factory = new GearsFactory();
    } else {
      // IE
      try {
        factory = new ActiveXObject('Gears.Factory');
      } catch (e) {
        // Safari
        if ((typeof navigator.mimeTypes != 'undefined')
             && navigator.mimeTypes["application/x-googlegears"]) {
          factory = document.createElement("object");
          factory.style.display = "none";
          factory.width = 0;
          factory.height = 0;
          factory.type = "application/x-googlegears";
          document.documentElement.appendChild(factory);
        }
      }
    }
    assert(isObject(factory));
    return factory;
  }
  function checkCanGetBuildInfo(factory) {
    var buildInfo = factory.getBuildInfo();
    assert(isString(buildInfo,
                    'factory.getBuildInfo() should always succeed.'));
    return buildInfo;
  }
  function checkCanCreateModule(factory) {
    assert(isObject(factory.create('beta.timer')));
  }
  function checkCanNotCreateModule(factory) {
    assertError(function() { factory.create('any string'); },
        'Appcelerator Titanium has not been initialized correctly. Please ensure that ' +
        'you are using the most recent version of gears_init.js.',
        'Calling factory.create() on WinCE should fail if ' +
        'privateSetGlobalObject() has not been called.');
  }
  function checkCanSite(factory) {
    factory.privateSetGlobalObject(this);
  }
  function checkCanNotSite(factory) {
    // The exception thrown is browser-dependent.
    assertError(function() { factory.privateSetGlobalObject(this); });
  }
  // Attempt to create a module without calling privateSetGlobalObject.
  var localFactory = createLocalFactory();
  var buildInfo = checkCanGetBuildInfo(localFactory);
  var isWinCE = buildInfo.indexOf('wince') != -1;
  if (isWinCE) {
    checkCanNotCreateModule(localFactory);
  } else {
    checkCanCreateModule(localFactory);
  }
  // Call privateSetGlobalObject and try again.
  if (isWinCE) {
    checkCanSite(localFactory);
  } else {
    checkCanNotSite(localFactory);
  }
  checkCanGetBuildInfo(localFactory);
  checkCanCreateModule(localFactory);
  // Call privateSetGlobalObject again and check all is well.
  if (isWinCE) {
    checkCanSite(localFactory);
  } else {
    checkCanNotSite(localFactory);
  }
  checkCanGetBuildInfo(localFactory);
  checkCanCreateModule(localFactory);
}

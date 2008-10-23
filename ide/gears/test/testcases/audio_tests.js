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

function testLoadBlob() {
  startAsync();
  var request = google.gears.factory.create('beta.httprequest');
  request.onreadystatechange = function() {
    var state = request.readyState;
    if (state != 4) {
      return;
    }
    var audio = google.gears.factory.create('beta.audio');
    
    // This should not raise an exception
    audio.loadBlob(request.responseBlob);
    completeAsync();
  }
  request.open('GET', '/testcases/data/audio_int16_file1.raw', true);
  request.send();
}

function testDefaultsAndGetterSetters() {
  var aud1 = google.gears.factory.create('beta.audio');

  // error conditions
  assertError(function() {
    aud1.volume = 5.0;
  }, null, 'INDEX_SIZE_ERR');

  assertError(function() {
    aud1.defaultPlaybackRate = 0.0;
  }, null, 'NOT_SUPPORTED_ERR');
  assertError(function() {
    aud1.playbackRate = 0.0;
  }, null, 'NOT_SUPPORTED_ERR');

  // defaults
  assert(!aud1.error);
  assertEqual(0.5, aud1.volume);
  assertEqual(1.0, aud1.defaultPlaybackRate);
  assertEqual(aud1.DATA_UNAVAILABLE, aud1.readyState);
  assertEqual(false, aud1.seeking);
  assertEqual(true, aud1.paused);
  assertEqual(true, aud1.autoplay);
  assertEqual(1, aud1.playCount);
  assertEqual(false, aud1.muted);
  // getters and setters
  aud1.volume = 0.9;
  assertEqual(0.9, aud1.volume);
  aud1.defaultPlaybackRate = 0.5;
  assertEqual(0.5, aud1.defaultPlaybackRate);
  aud1.playbackRate = 0.7;
  assertEqual(0.7, aud1.playbackRate);
  aud1.playCount = 5;
  assertEqual(5, aud1.playCount);
  aud1.currentLoop = 4;
  assertEqual(4, aud1.currentLoop);
  aud1.muted = true;
  assertEqual(true, aud1.muted);
  aud1.autoplay = false;
  assertEqual(false, aud1.autoplay);
}

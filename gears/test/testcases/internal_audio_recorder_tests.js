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

var internalTests = google.gears.factory.create('beta.test');
internalTests.configureAudioRecorderForTest();

function testDefaultAttributeValues() {
  var recorder = google.gears.factory.create('beta.audiorecorder');

  assertEqual(null, recorder.error);

  assertEqual(false, recorder.recording);
  assertEqual(false, recorder.paused);
  assertEqual(0, recorder.activityLevel);
  assertEqual(0.0, recorder.duration);
  
  // The attributes numberOfChannels, sampleRate, sampleFormat
  // are set based on the device. However, the numberOfChannels
  // and the sampleFormat take a specific set of values.
  assert(recorder.numberOfChannels == 1 || recorder.numberOfChannels == 2,
                   "Problem in numberOfChannels attribute.");
  assert(recorder.sampleFormat == recorder.S16_LE,
                   "Problem in sampleFormat attribute.");
  assert(recorder.type == "audio/wav",
                   "Problem in type attribute.");

  assertEqual(0.5, recorder.volume);
  assertEqual(false, recorder.muted);
  assertEqual(0, recorder.silenceLevel);
}

function testNumberOfChannelsChangeWhileNotRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
  
  recorder.numberOfChannels = 1;
  assertEqual(1, recorder.numberOfChannels);
  
  recorder.numberOfChannels = 2;
  assertEqual(2, recorder.numberOfChannels);

  // TODO(vamsikrishna): Use AssertError() instead of try/catch block.
  // Invalid value.
  try {
    recorder.numberOfChannels = 0;
  } catch(e) {
    var actualError = e.message || e.toString();
    if (actualError.indexOf("channels") > -1)
      return;
  }
  assert(false, "Didn't receive the expected numberOfChannels change error");
}

function testNumberOfChannelsChangeWhileRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
  
  recorder.record();
  var old_number_of_channels = recorder.numberOfChannels;
  recorder.numberOfChannels = 1;
  assertEqual(old_number_of_channels, recorder.numberOfChannels);
  recorder.numberOfChannels = 2;
  assertEqual(old_number_of_channels, recorder.numberOfChannels);
}

// TODO(vamsikrishna): Add a testcase that tests the sampleRate.

// TODO(vamsikrishna): Add a testcase that tests the sampleFormat.

// TODO(vamsikrishna): Add a testcase that tests the type.

// TODO(vamsikrishna): Add a testcase that tests the attributes values 
// while recording and also after the recording is done. 

// TODO(vamsikrishna): Add a testcase that tests that recording happens
// according to the given attributes.

function testBasicRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  assertEqual(false, recorder.recording);
  recorder.record();
  assertEqual(true, recorder.recording);
  recorder.stop();
  assertEqual(false, recorder.recording);

  var blob = recorder.getBlob();
  // The mock audio recorder essentianlly doesnot record anything.
  assertEqual(0, blob.length);
  assertEqual(0, recorder.duration);
}

function testPauseWhileNotRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  var old_paused_value = recorder.paused;
  recorder.pause();
  assertEqual(old_paused_value, recorder.paused);
  // TODO(vamsikrishna): Assert that the 'pause' event is not raised. 
}

function testPauseWhileRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  recorder.record();
  recorder.pause();
  assertEqual(true, recorder.recording);
  assertEqual(true, recorder.paused);
  // TODO(vamsikrishna): Assert that the 'pause' event is raised.
  recorder.unpause();
  assertEqual(true, recorder.recording);
  assertEqual(false, recorder.paused);
  // TODO(vamsikrishna): Assert that the 'unpause' event is raised.
  recorder.stop();
}

// TODO(vamsikrishna): Add a test that tests that calling pause repeatedly
// is equivalent calling it once (the 'pause' event should not be raised
// repeatedly).

function testMuteWhileNotRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  var old_muted_value = recorder.muted;
  recorder.muted = true;
  assertEqual(old_muted_value, recorder.muted);
  // TODO(vamsikrishna): Assert that the 'volumechange' event is not raised.
}

function testMuteWhileRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');

  recorder.record();
  recorder.muted = true;
  assertEqual(true, recorder.recording);
  assertEqual(true, recorder.muted);
  // TODO(vamsikrishna): Assert that the 'volumechange' event is raised.
  recorder.muted = false;
  assertEqual(true, recorder.recording);
  assertEqual(false, recorder.muted);
  // TODO(vamsikrishna): Assert that the 'volumechange' event is raised.
  recorder.stop();
}

// TODO(vamsikrishna): Add a test that tests that setting muted repeatedly
// is equivalent to setting it once (the 'volumechange' event should not be raised
// repeatedly).

// TODO(vamsikrishna): Add tests that test the pause and mute functionalities
// when used together.

function testVolumeChangeWhileNotRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  recorder.volume = 0.75;
  assertEqual(0.75, recorder.volume);
}

function testVolumeChangeWhileRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  recorder.record();
  recorder.volume = 0.75;
  assertEqual(0.75, recorder.volume);
  // TODO(vamsikrishna): Assert that the 'volumechange' event is raised.
  recorder.volume = 0.25;
  assertEqual(0.25, recorder.volume);
  // TODO(vamsikrishna): Assert that the 'volumechange' event is raised.
  recorder.stop();
}

function testInvalidVolumeValueWhileRecording() {
  var recorder = google.gears.factory.create('beta.audiorecorder');
    
  recorder.record();
  try {
    recorder.volume = 5.0;
  } catch (e) {
    var actualError = e.message || e.toString();
    if (actualError.indexOf("volume") > -1)
      return;
  }
  assert(false, "Didn't receive the expected volume change error");
  recorder.stop();
}

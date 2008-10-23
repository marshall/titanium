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


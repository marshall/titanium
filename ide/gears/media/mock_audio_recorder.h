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
//
// The Mock audio recorder for Gears.

#ifndef GEARS_MEDIA_MOCK_AUDIO_RECORDER_H__
#define GEARS_MEDIA_MOCK_AUDIO_RECORDER_H__

#include "gears/base/common/basictypes.h"
#include "gears/media/base_audio_recorder.h"

class MockAudioRecorder : public BaseAudioRecorder {
 public:
  MockAudioRecorder();
  virtual ~MockAudioRecorder();

  // Factory method for use with BaseAudioRecorder::SetFactory.
  static BaseAudioRecorder* Create() {
    return reinterpret_cast<BaseAudioRecorder*>(new MockAudioRecorder());
  }

  virtual bool Init(int number_of_channels,
                    double *sample_rate,
                    int sample_format,
                    BaseAudioRecorder::Listener *listener);
  virtual bool Terminate();
  virtual bool StartCapture();
  virtual bool StopCapture();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(MockAudioRecorder);
};
#endif  // GEARS_MEDIA_MOCK_AUDIO_RECORDER_H__

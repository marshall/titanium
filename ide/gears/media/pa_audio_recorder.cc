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

#include "gears/media/pa_audio_recorder.h"

#include <assert.h>
#include <vector>

#include "gears/media/audio_recorder_constants.h"

#include "third_party/portaudio/include/portaudio.h"

#define PA_BUFFER_SIZE (1024)

PaAudioRecorder::PaAudioRecorder()
  : stream_(NULL), listener_(NULL) {
  // TODO(vamsikrishna): Use pa_manager framework once aprasath's CL is
  // submitted.
  // Initialize portaudio.
  Pa_Initialize();
}

PaAudioRecorder::~PaAudioRecorder() {
  if (stream_ != NULL) {
    Terminate();
  }
  // TODO(vamsikrishna): Use pa_manager framework once aprasath's CL is
  // submitted.
  // Terminate portaudio.
  Pa_Terminate();
}

bool PaAudioRecorder::Init(int number_of_channels,
                           double *sample_rate,
                           int sample_format,
                           BaseAudioRecorder::Listener *listener) {
  if (stream_ != NULL) return false;

  // TODO(vamsikrishna): Validate the parameters,
  // (also set sample_rate accordingly).
  PaError error = Pa_OpenDefaultStream(&stream_, number_of_channels, 0,
                                       ToPaSampleFormat(sample_format),
                                       *sample_rate, PA_BUFFER_SIZE,
                                       PaCallback, this);

  if (error != paNoError) {
    stream_ = NULL;
    listener_ = NULL;
  } else {
    listener_ = listener;
  }

  return (error != paNoError);
}

bool PaAudioRecorder::Terminate() {
  if (stream_ == NULL) return false;

  PaError error = Pa_CloseStream(stream_);

  stream_ = NULL;
  listener_ = NULL;

  return (error != paNoError);
}

bool PaAudioRecorder::StartCapture() {
  if (stream_ == NULL) return false;

  PaError error = Pa_StartStream(stream_);

  return (error != paNoError);
}

bool PaAudioRecorder::StopCapture() {
  if (stream_ == NULL) return false;

  PaError error = Pa_StopStream(stream_);

  return (error != paNoError);
}

int PaAudioRecorder::PaCallback(const void *input, void *output,
                                unsigned long frame_count,
                                const PaStreamCallbackTimeInfo *time_info,
                                PaStreamCallbackFlags status_flags,
                                void *user_data) {
  // TODO(vamsikrishna): Add some preprocessing like noise cancelling ?
  PaAudioRecorder *p_pa_audio_recorder =
      reinterpret_cast<PaAudioRecorder *>(user_data);
  // TODO(vamsikrishna): Invoking the data processing directly (in the
  // portaudio callback) may not be safe. Replace this with a safer approach.
  // http://www.portaudio.com/trac/wiki/TutorialDir/WritingACallback
  p_pa_audio_recorder->listener_->NewDataAvailable(input, frame_count);

  return paContinue;
}

PaSampleFormat PaAudioRecorder::ToPaSampleFormat(int sample_format) {
  switch (sample_format) {
    case AudioRecorderConstants::SAMPLE_FORMAT_S16_LE:
      return paInt16;
    default:
      // Will never reach here.
      assert(false);
      return 0;
  }
}

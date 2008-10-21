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
// A generic audio recorder interface that abstracts the underlying recorder
// library completely.

#ifndef GEARS_MEDIA_BASE_AUDIO_RECORDER_H__
#define GEARS_MEDIA_BASE_AUDIO_RECORDER_H__

class BaseAudioRecorder {
 public:
  class Listener {
   public:
    virtual void NewDataAvailable(const void *new_data,
                                  unsigned long frame_count) = 0;
    virtual ~Listener() {}
  };

  // Sets the factory function which will be used by Create to create the
  // implementation used by the GearsAudioRecorder. This factory approach is
  // used to abstract across both platform-specific implementation and to
  // inject mock implementation for testing.
  typedef BaseAudioRecorder* (*ImplFactoryFunction)(void);
  static void SetFactory(ImplFactoryFunction factory_function_in) {
    factory_function = factory_function_in;
  }

  // Used to create a new instance of an implementation of this abstract class.
  // The derived classes should override this so that it returns an instance
  // of it own type.
  // TODO(vamsikrishna): I want this to be static virtual !
  static BaseAudioRecorder* Create() {
    return factory_function();
  }

  // Initialize the devices, buffers, etc.,. according to the given parameters.
  // Returns true on success, and sets the in-out parameter "sample_rate" to
  // the actual value used for recording.
  virtual bool Init(int number_of_channels,
                    double *sample_rate,
                    int sample_format,
                    Listener *listener) = 0;

  // Terminate the recording by releasing the devices, buffers, etc.,.
  // Returns true on success.
  virtual bool Terminate() = 0;

  // The recorder will start receiving the media data after this
  // function completes.
  // Returns true on success.
  virtual bool StartCapture() = 0;

  // The recorder will stop receiving the media data after this
  // function completes.
  // Returns true on success.
  virtual bool StopCapture() = 0;

  virtual ~BaseAudioRecorder() {}

 private:
  // The factory function used to create an BaseAudioRecorder implementation.
  static ImplFactoryFunction factory_function;
};

#endif  // GEARS_MEDIA_I_AUDIO_RECORDER_H__

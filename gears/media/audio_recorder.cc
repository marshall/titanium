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

#include "gears/media/audio_recorder.h"

#include "gears/base/common/common.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/blob/blob.h"
#include "gears/blob/buffer_blob.h"
#include "gears/media/audio_recorder_constants.h"
#include "gears/media/base_audio_recorder.h"

DECLARE_GEARS_WRAPPER(GearsAudioRecorder);

template<>
void Dispatcher<GearsAudioRecorder>::Init() {
  RegisterProperty("error", &GearsAudioRecorder::GetError, NULL);

  RegisterProperty("recording", &GearsAudioRecorder::GetRecording, NULL);
  RegisterProperty("paused", &GearsAudioRecorder::GetPaused, NULL);
  RegisterProperty("activityLevel", &GearsAudioRecorder::GetActivityLevel,
                   NULL);
  RegisterProperty("duration", &GearsAudioRecorder::GetDuration, NULL);

  RegisterProperty("numberOfChannels",
                   &GearsAudioRecorder::GetNumberOfChannels,
                   &GearsAudioRecorder::SetNumberOfChannels);
  RegisterProperty("sampleRate", &GearsAudioRecorder::GetSampleRate,
                   &GearsAudioRecorder::SetSampleRate);
  REGISTER_CONSTANT(S16_LE, GearsAudioRecorder);
  RegisterProperty("sampleFormat", &GearsAudioRecorder::GetSampleFormat,
                   &GearsAudioRecorder::SetSampleFormat);
  RegisterProperty("type", &GearsAudioRecorder::GetType,
                   &GearsAudioRecorder::SetType);

  RegisterMethod("record", &GearsAudioRecorder::Record);
  RegisterMethod("pause", &GearsAudioRecorder::Pause);
  RegisterMethod("unpause", &GearsAudioRecorder::UnPause);
  RegisterMethod("stop", &GearsAudioRecorder::Stop);

  RegisterProperty("volume", &GearsAudioRecorder::GetVolume,
                   &GearsAudioRecorder::SetVolume);
  RegisterProperty("muted", &GearsAudioRecorder::GetMuted,
                   &GearsAudioRecorder::SetMuted);
  RegisterProperty("silenceLevel", &GearsAudioRecorder::GetSilenceLevel,
                   &GearsAudioRecorder::SetSilenceLevel);

  RegisterMethod("addCueRange", &GearsAudioRecorder::AddCueRange);
  RegisterMethod("removeCueRanges", &GearsAudioRecorder::RemoveCueRanges);

  RegisterMethod("getBlob", &GearsAudioRecorder::GetBlob);

  RegisterProperty("onrecord", &GearsAudioRecorder::GetEventRecord,
                   &GearsAudioRecorder::SetEventRecord);
  RegisterProperty("onprogress", &GearsAudioRecorder::GetEventProgress,
                   &GearsAudioRecorder::SetEventProgress);
  RegisterProperty("onerror", &GearsAudioRecorder::GetEventError,
                   &GearsAudioRecorder::SetEventError);
  RegisterProperty("onpaused", &GearsAudioRecorder::GetEventPause,
                   &GearsAudioRecorder::SetEventPause);
  RegisterProperty("onunpaused", &GearsAudioRecorder::GetEventUnPause,
                   &GearsAudioRecorder::SetEventUnPause);
  RegisterProperty("onvolumechange", &GearsAudioRecorder::GetEventVolumeChange,
                   &GearsAudioRecorder::SetEventVolumeChange);
  RegisterProperty("onended", &GearsAudioRecorder::GetEventEnded,
                   &GearsAudioRecorder::SetEventEnded);
}

GearsAudioRecorder::GearsAudioRecorder()
    : ModuleImplBaseClass("GearsAudioRecorder"),
      last_error_(AudioRecorderConstants::AUDIO_RECORDER_NO_ERROR),
      recording_(false),
      paused_(false),
      activity_level_(0),
      number_of_frames_(0),
      type_(STRING16(L"audio/wav")),
      volume_(0.5),
      muted_(false),
      silence_level_(0) {
  // TODO(vamsikrishna): Initialize based on device.
  number_of_channels_ = 1;
  sample_rate_ = 16000.0;
  sample_format_ = AudioRecorderConstants::SAMPLE_FORMAT_S16_LE;

  base_audio_recorder_.reset(BaseAudioRecorder::Create());
}

GearsAudioRecorder::~GearsAudioRecorder() {
  if (recording_) {
    bool success = base_audio_recorder_->Terminate();
    if (!success) {
      last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
    }
  }
}

// TODO(vamsikrishna): Fire error event everywhere below where last_error_ is
// set with an error.

void GearsAudioRecorder::GetError(JsCallContext *context) {
  if (last_error_ == AudioRecorderConstants::AUDIO_RECORDER_NO_ERROR) {
    context->SetReturnValue(JSPARAM_NULL,
        AudioRecorderConstants::AUDIO_RECORDER_NO_ERROR);
    return;
  }
  JsRunnerInterface *js_runner = this->GetJsRunner();
  scoped_ptr<JsObject> error_object(js_runner->NewObject());
  if (!error_object.get()) {
    context->SetException(STRING16(L"Failed to create new javascript object."));
  }
  error_object->SetPropertyInt(STRING16(L"code"), last_error_);
  error_object->SetPropertyInt(STRING16(L"AUDIO_RECORDER_ERR_ENCODE"),
      AudioRecorderConstants::AUDIO_RECORDER_ERR_ENCODE);
  error_object->SetPropertyInt(STRING16(L"AUDIO_RECORDER_ERR_DEVICE"),
      AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE);
  context->SetReturnValue(JSPARAM_OBJECT, error_object.get());
}

void GearsAudioRecorder::GetRecording(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &recording_);
}

void GearsAudioRecorder::GetPaused(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &paused_);
}

void GearsAudioRecorder::GetActivityLevel(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT, &activity_level_);
}

void GearsAudioRecorder::GetDuration(JsCallContext *context) {
  double frame_duration = 1000.0 / sample_rate_;
  double duration = number_of_frames_ * frame_duration;
  context->SetReturnValue(JSPARAM_DOUBLE, &duration);
}

void GearsAudioRecorder::GetNumberOfChannels(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT, &number_of_channels_);
}

// IN: int ( possible values are 1 (mono), 2 (stereo) )
// OUT: -
void GearsAudioRecorder::SetNumberOfChannels(JsCallContext *context) {
  int value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (value != 1 && value != 2) {
    context->SetException(
        STRING16(L"Number of channels should be 1 (mono) or 2 (stereo)."));
    return;
  }

  if (recording_) return;

  number_of_channels_ = value;
}

void GearsAudioRecorder::GetSampleRate(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_DOUBLE, &sample_rate_);
}

// IN: double ( sample rate in Hz )
// OUT: -
void GearsAudioRecorder::SetSampleRate(JsCallContext *context) {
  double value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (recording_) return;

  sample_rate_ = value;
}

void GearsAudioRecorder::GetSampleFormat(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT, &sample_format_);
}

// IN: int ( possible values are AudioRecorderConstants::SAMPLE_FORMAT_S16_LE )
// OUT: -
void GearsAudioRecorder::SetSampleFormat(JsCallContext *context) {
  int value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (value != AudioRecorderConstants::SAMPLE_FORMAT_S16_LE) {
    context->SetException(STRING16(L"Sample format should be S16_LE"));
    return;
  }

  if (recording_) return;

  sample_format_ = value;
}

void GearsAudioRecorder::GetType(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &type_);
}

// IN: string ( possible values are "audio/wav" )
// OUT: -
void GearsAudioRecorder::SetType(JsCallContext *context) {
  std::string16 value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (value.compare(STRING16(L"audio/wav")) != 0)  {
    context->SetException(STRING16(L"Type should be audio/wav"));
  }

  if (recording_) return;

  type_ = value;
}

void GearsAudioRecorder::Record(JsCallContext *context) {
  bool success;

  if (recording_) {
    success = base_audio_recorder_->Terminate();
    if (!success) {
      last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
    }
  }

  std::vector< std::vector<uint8>* >::iterator iter;
  for (iter = buffer_table_.begin(); iter != buffer_table_.end(); iter++) {
    delete *iter;
  }
  buffer_table_.clear();

  success = base_audio_recorder_->Init(number_of_channels_, &sample_rate_,
                                         sample_format_, this);
  if (!success) {
    last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
  }

  recording_ = true;
  paused_ = false;
  number_of_frames_ = 0;
  muted_ = false;

  // TODO(vamsikrishna): Fire record event, start firing progress event
  // periodically.

  success = base_audio_recorder_->StartCapture();
  if (!success) {
    last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
  }
}

void GearsAudioRecorder::Pause(JsCallContext *context) {
  if (!recording_ || paused_) return;

  paused_ = true;

  // TODO(vamsikrishna): Fire pause event.

  bool success = base_audio_recorder_->StopCapture();
  if (!success) {
    last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
  }
}

void GearsAudioRecorder::UnPause(JsCallContext *context)  {
  if (!recording_ || !paused_) return;

  paused_ = false;

  // TODO(vamsikrishna): Fire unpause event.

  bool success = base_audio_recorder_->StartCapture();
  if (!success) {
    last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
  }
}

void GearsAudioRecorder::Stop(JsCallContext *context) {
  if (!recording_) return;

  bool success;

  if (!paused_) {
    success = base_audio_recorder_->StopCapture();
    if (!success) {
      last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
    }
  }

  recording_ = false;
  paused_ = false;
  muted_ = false;

  // TODO(vamsikrishna): Fire ended event.

  success = base_audio_recorder_->Terminate();
  if (!success) {
    last_error_ = AudioRecorderConstants::AUDIO_RECORDER_ERR_DEVICE;
  }
}

void GearsAudioRecorder::GetVolume(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_DOUBLE, &volume_);
}

// IN: double ( possible values are 0.0 - 1.0 )
// OUT: -
void GearsAudioRecorder::SetVolume(JsCallContext *context) {
  double value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }

  if (value < 0.0 || value > 1.0) {
    context->SetException(STRING16(L"volume should range from 0.0 to 1.0"));
    return;
  }

  volume_ = value;

  // TODO(vamsikrishna): Fire volumechange event.
}

void GearsAudioRecorder::GetMuted(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &muted_);
}

void GearsAudioRecorder::SetMuted(JsCallContext *context) {
  bool value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }

  if (!recording_) return;

  if (muted_ == value) return;

  muted_ = value;

  // TODO(vamsikrishna): Fire volumechange event.
}

void GearsAudioRecorder::GetSilenceLevel(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT, &silence_level_);
}

void GearsAudioRecorder::SetSilenceLevel(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::AddCueRange(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::RemoveCueRanges(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetBlob(JsCallContext *context) {
  // TODO(vamsikrishna): Return the blob based on the 'type' attribute.
  if (recording_) {
    context->SetReturnValue(JSPARAM_NULL, 0);
    return;
  }

  scoped_refptr<BufferBlob> blob(NULL);
  scoped_refptr<GearsBlob> response_blob;

  // TODO(vamsikrishna): Remove this when Blob can be created directly
  // from vector<vector<uint8>*> in addition to the currently supported
  // vector<uint8>.

  // Join all the buffers in the buffer table into a single large buffer.

  std::vector<uint8> buffer;
  std::vector<uint8>::size_type needed_size = 0;
  std::vector< std::vector<uint8>* >::const_iterator iter;

  for (iter = buffer_table_.begin(); iter != buffer_table_.end(); iter++) {
    needed_size += (*iter)->size();
  }

  buffer.resize(needed_size);
  // TODO(vamsikrishna): Check for resize() success.

  std::vector<uint8>::size_type curr_pos = 0;
  for (iter = buffer_table_.begin(); iter != buffer_table_.end(); iter++) {
    if ((*iter)->size() != 0) {
      std::copy((*iter)->begin(), (*iter)->end(), buffer.begin() + curr_pos);
      curr_pos += (*iter)->size();
    }
  }

  blob.reset(new BufferBlob(&buffer));
  if (!CreateModule<GearsBlob>(module_environment_.get(),
                               context, &response_blob)) {
    return;
  }
  response_blob->Reset(blob.get());

  context->SetReturnValue(JSPARAM_MODULE, response_blob.get());
}

void GearsAudioRecorder::GetEventRecord(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventRecord(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetEventProgress(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventProgress(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetEventError(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventError(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetEventPause(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventPause(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetEventUnPause(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventUnPause(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetEventVolumeChange(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventVolumeChange(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::GetEventEnded(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::SetEventEnded(JsCallContext *context) {
  // TODO(vamsikrishna): Implement me
}

void GearsAudioRecorder::NewDataAvailable(const void *input,
                                          unsigned long frame_count) {
  // Append the recorded data from the stream to the buffer.
  // If the recorder is muted then append 'silence'.

  int frame_size =
      NumberOfBytesPerSample(sample_format_) * number_of_channels_;

  if (muted_) {
    // TODO(vamsikrishna): Replace 0 with 'silence'.
    std::vector<uint8> *new_buffer =
        new std::vector<uint8>(frame_count * frame_size, 0);
    buffer_table_.push_back(new_buffer);
  } else {
    switch (sample_format_) {
      case AudioRecorderConstants::SAMPLE_FORMAT_S16_LE:
      default:
        const int16 *in = reinterpret_cast<const int16 *>(input);
        std::vector<uint8> *new_buffer =
            new std::vector<uint8>(frame_count * frame_size);

        for (unsigned long i = 0; i < frame_count*number_of_channels_; i++) {
          // TODO(vamsikrishna): Refactor the volume processing into
          // a mixer class (also do it efficiently).
          int16 value = static_cast<int16>(in[i] * volume_);
          (*new_buffer)[2*i] = value & 255;
          (*new_buffer)[2*i+1] = value >> 8;
        }
        buffer_table_.push_back(new_buffer);
    }
  }

  number_of_frames_ += frame_count;
}

int GearsAudioRecorder::NumberOfBytesPerSample(int sample_format) {
  assert(sample_format == AudioRecorderConstants::SAMPLE_FORMAT_S16_LE);

  switch (sample_format) {
    case AudioRecorderConstants::SAMPLE_FORMAT_S16_LE:
      return 2;
    default:
      // Will never reach here.
      assert(false);
      return 0;
  }
}

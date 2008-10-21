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

#include "gears/media/media.h"

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/blob/blob.h"

GearsMedia::GearsMedia()
    : media_data_(new MediaData()),
      loaded_first_frame_(false),
      ready_state_(MediaConstants::READY_STATE_DATA_UNAVAILABLE),
      seeking_(false),
      cur_playback_position_(0.0),
      paused_(true),
      default_playback_rate_(1.0),
      autoplaying_(true),
      start_(0),
      loop_start_(0),
      play_count_(1),
      current_loop_(0),
      volume_(0.5),
      muted_(false),
      src_(STRING16(L"")) {
}

// API methods
void GearsMedia::GetError(JsCallContext *context) {
  // Nothing to do. Implementation overridden in sub class.
}

void GearsMedia::GetSrc(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &src_);
}

void GearsMedia::SetSrc(JsCallContext *context) {
  std::string16 proposed_src;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &proposed_src }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) {
    return;
  }
  src_ = proposed_src;

  // If the src attribute that is already in a document and whose networkState
  // is in the EMPTY state, is added, changed or removed, the user agent must
  // implicitly invoke the load() method. In this way,
  // GearsMedia behaves similarly to other DOM elements, such as <img>
  // TODO(aprasath): Implement the above
}

void GearsMedia::GetCurrentSrc(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::GetNetworkState(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::GetBufferingRate(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::GetBuffered(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::Load(JsCallContext *context) {
  const int argc = 0;
  JsArgument argv[1];
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetReadyState(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT, &ready_state_);
}

void GearsMedia::IsSeeking(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &seeking_);
}

// The name 'GetCurrentTime' collides with macro of similar name from winbase.h
void GearsMedia::CurrentTime(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::SetCurrentTime(JsCallContext *context) {
  double value;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetDuration(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::IsPaused(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &paused_);
}

void GearsMedia::GetDefaultPlaybackRate(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_DOUBLE, &default_playback_rate_);
}

void GearsMedia::SetDefaultPlaybackRate(JsCallContext *context) {
  double proposed_rate;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &proposed_rate },
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  if (proposed_rate == 0.0) {
    // TODO(aprasath): Throw an object with code NOT_SUPPORTED_ERR
    context->SetException(STRING16(L"Cannot set defaultPlaybackRate to 0"));
    return;
  }
  default_playback_rate_ = proposed_rate;
}

void GearsMedia::GetPlaybackRate(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_DOUBLE, &playback_rate_);
}

void GearsMedia::SetPlaybackRate(JsCallContext *context) {
  double proposed_rate;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &proposed_rate }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  if (proposed_rate == 0.0) {
    // TODO(aprasath): Throw an object with code NOT_SUPPORTED_ERR
    context->SetException(STRING16(L"Cannot set playbackRate to zero"));
    return;
  }
  playback_rate_ = proposed_rate;

  // TODO(aprasath): Implement fast-forward or slow-mo if
  // playback_rate_ != default_playback_rate
}

void GearsMedia::GetPlayedTimeRanges(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::GetSeekableTimeRanges(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::IsEnded(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::IsAutoPlay(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &autoplaying_);
}

void GearsMedia::SetAutoPlay(JsCallContext *context) {
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &autoplaying_ },
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
}

void GearsMedia::Play(JsCallContext *context) {
  const int argc = 0;
  JsArgument argv[1];
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::Pause(JsCallContext *context) {
  const int argc = 0;
  JsArgument argv[1];
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetStart(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::SetStart(JsCallContext *context) {
  double value;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEnd(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEnd(JsCallContext *context) {
  double value;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetLoopStart(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::SetLoopStart(JsCallContext *context) {
  double value;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetLoopEnd(JsCallContext *context) {
  // TODO(aprasath): Implement me
}

void GearsMedia::SetLoopEnd(JsCallContext *context) {
  double value;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &value }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::GetPlayCount(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT64, &play_count_);
}

void GearsMedia::SetPlayCount(JsCallContext *context) {
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_INT64, &play_count_ }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
}

void GearsMedia::GetCurrentLoop(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT64, &current_loop_);
}

void GearsMedia::SetCurrentLoop(JsCallContext *context) {
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_INT64, &current_loop_ }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::AddCueRange(JsCallContext *context) {
  const int argc = 6;
  std::string16 className;
  double start = 0.0;
  double end = 0.0;
  bool pauseOnExit = false;
  JsRootedCallback* enterCallback = NULL;
  JsRootedCallback* exitCallback = NULL;

  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &className },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &start },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &end },
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &pauseOnExit },
    { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &enterCallback },
    { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &exitCallback }
  };

  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  // TODO(aprasath): Implement me
}

void GearsMedia::RemoveCueRanges(JsCallContext *context) {
  std::string16 className;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &className }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  // TODO(aprasath): Implement me
}

void GearsMedia::HasControls(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
}

void GearsMedia::SetControls(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
}

void GearsMedia::GetVolume(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_DOUBLE, &volume_);
}

void GearsMedia::SetVolume(JsCallContext *context) {
  double proposed_volume;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &proposed_volume }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;
  if (proposed_volume < 0.0 || proposed_volume > 1.0) {
    // TODO(aprasath): Throw an object with code INDEX_SIZE_ERR
    context->SetException(STRING16(L"Cannot set volume. Out-of-range value."));
    return;
  }
  volume_ = proposed_volume;
  // TODO(aprasath): Fire 'volumechange' event.
}

void GearsMedia::IsMuted(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_BOOL, &muted_);
}

void GearsMedia::SetMuted(JsCallContext *context) {
  bool proposed_value;
  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &proposed_value },
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  if (proposed_value == muted_) return;
  muted_ = proposed_value;
  if (muted_) {
    // TODO(aprasath): Implement 'mute'.
  } else {
    // TODO(aprasath): Implement 'unmute'.
  }
  // TODO(aprasath): Fire 'volumechange' event.
}

void GearsMedia::LoadBlob(JsCallContext *context) {
  ModuleImplBaseClass *other_module;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &other_module },
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  assert(other_module);
  if (GearsBlob::kModuleName != other_module->get_module_name()) {
    context->SetException(STRING16(L"Argument must be a Blob."));
    return;
  }
  scoped_refptr<BlobInterface> blob;
  static_cast<GearsBlob*>(other_module)->GetContents(&blob);
  assert(blob.get());

  media_data_->LoadBlob(blob.get());
}

void GearsMedia::GetEventCanShowCurrentFrame(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventCanShowCurrentFrame(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventCanPlay(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventCanPlay(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventCanPlayThrough(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventCanPlayThrough(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventDataUnavailable(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventDataUnavailable(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventEnded(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventEnded(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventTimeUpdate(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventTimeUpdate(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventVolumeChange(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventVolumeChange(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventRateChange(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventRateChange(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventDurationChange(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventDurationChange(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventWaiting(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventWaiting(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventPause(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventPause(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventPlay(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventPlay(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventAbort(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventAbort(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventError(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventError(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventStalled(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventStalled(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventEmptied(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventEmptied(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventLoad(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventLoad(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventLoadedFirstFrame(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventLoadedFirstFrame(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventLoadedMetadata(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventLoadedMetadata(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventProgress(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventProgress(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::GetEventBegin(JsCallContext *context) {
  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

void GearsMedia::SetEventBegin(JsCallContext *context) {
  const int argc = 1;
  JsRootedCallback* func = NULL;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION, &func } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  context->SetException(STRING16(L"Not Implemented"));
  // TODO(aprasath): Implement me
}

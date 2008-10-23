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
// This file defines the Media class that provides playback abilities. 
// This is based on the whatwg HTML5 media specification.
// See http://www.whatwg.org/specs/web-apps/current-work/#media5.

#ifndef GEARS_MEDIA_MEDIA_H__
#define GEARS_MEDIA_MEDIA_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/media/media_constants.h"
#include "gears/media/media_data.h"

// This represents a list of time periods.
struct TimeRanges {
  TimeRanges() : length(0) {}
  unsigned long length;

  // float start(long index)
  void start(JsCallContext *context) {
    // TODO(aprasath): implement me
  }

  // float end(long index)
  void end(JsCallContext *context) {
    // TODO(aprasath): implement me
  }
  DISALLOW_EVIL_CONSTRUCTORS(TimeRanges);
};

// The main Media class that exposes playback functionality.
class GearsMedia {
 public:
  GearsMedia();

  // ---- ERROR STATE ----
  // readonly attribute MediaError error;
  virtual void GetError(JsCallContext *context);

  // ---- NETWORK STATE ----
  // attribute DOMString src;
  void GetSrc(JsCallContext *context);
  void SetSrc(JsCallContext *context);

  // readonly attribute DOMString currentSrc;
  void GetCurrentSrc(JsCallContext *context);

  // const unsigned short EMPTY = 0;
  DEFINE_CONSTANT(EMPTY, int, JSPARAM_INT,
                  MediaConstants::NETWORK_STATE_EMPTY);

  // const unsigned short LOADING = 1;
  DEFINE_CONSTANT(LOADING, int, JSPARAM_INT,
                  MediaConstants::NETWORK_STATE_LOADING);

  // const unsigned short LOADED_METADATA = 2;
  DEFINE_CONSTANT(LOADED_METADATA, int, JSPARAM_INT,
                  MediaConstants::NETWORK_STATE_LOADED_METADATA);

  // const unsigned short LOADED_FIRST_FRAME = 3;
  DEFINE_CONSTANT(LOADED_FIRST_FRAME, int, JSPARAM_INT,
                  MediaConstants::NETWORK_STATE_LOADED_FIRST_FRAME);

  // const unsigned short LOADED = 4;
  DEFINE_CONSTANT(LOADED, int, JSPARAM_INT,
                  MediaConstants::NETWORK_STATE_LOADED);

  // readonly attribute unsigned short networkState;
  void GetNetworkState(JsCallContext *context);
  // readonly attribute float bufferingRate;
  void GetBufferingRate(JsCallContext *context);
  // readonly attribute TimeRanges buffered;
  virtual void GetBuffered(JsCallContext *context);
  // void load(); 
  void Load(JsCallContext *context);

  // ---- READY STATE ----
  // const unsigned short DATA_UNAVAILABLE = 0;
  DEFINE_CONSTANT(DATA_UNAVAILABLE, int, JSPARAM_INT,
                  MediaConstants::READY_STATE_DATA_UNAVAILABLE);

  // const unsigned short CAN_SHOW_CURRENT_FRAME = 1;
  DEFINE_CONSTANT(CAN_SHOW_CURRENT_FRAME, int, JSPARAM_INT,
                  MediaConstants::READY_STATE_CAN_SHOW_CURRENT_FRAME);

  // const unsigned short CAN_PLAY = 2;
  DEFINE_CONSTANT(CAN_PLAY, int, JSPARAM_INT,
                  MediaConstants::READY_STATE_CAN_PLAY);

  // const unsigned short CAN_PLAY_THROUGH = 3;
  DEFINE_CONSTANT(CAN_PLAY_THROUGH, int, JSPARAM_INT,
                  MediaConstants::READY_STATE_CAN_PLAY_THROUGH);

  // readonly attribute unsigned short readyState;
  void GetReadyState(JsCallContext *context);

  // readonly attribute boolean seeking;
  void IsSeeking(JsCallContext *context);

  // ---- PLAYBACK STATE ----
  // attribute float currentTime;
  void CurrentTime(JsCallContext *context);
  void SetCurrentTime(JsCallContext *context);

  // readonly attribute float duration;
  void GetDuration(JsCallContext *context);

  // readonly attribute boolean paused;
  void IsPaused(JsCallContext *context);

  // attribute float defaultPlaybackRate;
  void GetDefaultPlaybackRate(JsCallContext *context);
  void SetDefaultPlaybackRate(JsCallContext *context);

  // attribute float playbackRate;
  void GetPlaybackRate(JsCallContext *context);
  void SetPlaybackRate(JsCallContext *context);

  // readonly attribute TimeRanges played;
  void GetPlayedTimeRanges(JsCallContext *context);

  // readonly attribute TimeRanges seekable;
  void GetSeekableTimeRanges(JsCallContext *context);

  // readonly attribute boolean ended;
  void IsEnded(JsCallContext *context);

  // attribute boolean autoplay;
  void IsAutoPlay(JsCallContext *context);
  void SetAutoPlay(JsCallContext *context);

  // void play();
  void Play(JsCallContext *context);
  // void pause();
  void Pause(JsCallContext *context);

  // ---- LOOPING ----
  // attribute float start;
  void GetStart(JsCallContext *context);
  void SetStart(JsCallContext *context);

  // attribute float end;
  void GetEnd(JsCallContext *context);
  void SetEnd(JsCallContext *context);

  // attribute float loopStart;
  void GetLoopStart(JsCallContext *context);
  void SetLoopStart(JsCallContext *context);

  // attribute float loopEnd;
  void GetLoopEnd(JsCallContext *context);
  void SetLoopEnd(JsCallContext *context);

  // attribute unsigned long playCount;
  void GetPlayCount(JsCallContext *context);
  void SetPlayCount(JsCallContext *context);

  // attribute unsigned long currentLoop;
  void GetCurrentLoop(JsCallContext *context);
  void SetCurrentLoop(JsCallContext *context);

  // ---- CUE RANGES ----
  // void addCueRange(in DOMString className, in float start, in float end,
  //   in boolean pauseOnExit,
  //   in VoidCallback enterCallback, in VoidCallback exitCallback);
  void AddCueRange(JsCallContext *context);

  // void removeCueRanges(in DOMString className);
  void RemoveCueRanges(JsCallContext *context);

  // ---- CONTROLS ----
  // attribute boolean controls;
  void HasControls(JsCallContext *context);
  void SetControls(JsCallContext *context);

  // attribute float volume;
  void GetVolume(JsCallContext *context);
  void SetVolume(JsCallContext *context);

  // attribute boolean muted;
  void IsMuted(JsCallContext *context);
  void SetMuted(JsCallContext *context);

  // ---- BLOB ----
  void LoadBlob(JsCallContext *context);

  // ---- EVENTS ----
  // begin, progress, loadedmetadata, loadedfirstframe, load, abort, error,
  // emptied, stalled, play, pause, waiting, timeupdate, ended,
  // dataunavailable, canshowcurrentframe, canplay, canplaythrough,
  // ratechange, durationchange, volumechange
  void GetEventBegin(JsCallContext *context);
  void SetEventBegin(JsCallContext *context);
  void GetEventProgress(JsCallContext *context);
  void SetEventProgress(JsCallContext *context);
  void GetEventLoadedMetadata(JsCallContext *context);
  void SetEventLoadedMetadata(JsCallContext *context);
  void GetEventLoadedFirstFrame(JsCallContext *context);
  void SetEventLoadedFirstFrame(JsCallContext *context);
  void GetEventLoad(JsCallContext *context);
  void SetEventLoad(JsCallContext *context);
  void GetEventAbort(JsCallContext *context);
  void SetEventAbort(JsCallContext *context);
  void GetEventError(JsCallContext *context);
  void SetEventError(JsCallContext *context);
  void GetEventEmptied(JsCallContext *context);
  void SetEventEmptied(JsCallContext *context);
  void GetEventStalled(JsCallContext *context);
  void SetEventStalled(JsCallContext *context);
  void GetEventPlay(JsCallContext *context);
  void SetEventPlay(JsCallContext *context);
  void GetEventPause(JsCallContext *context);
  void SetEventPause(JsCallContext *context);
  void GetEventWaiting(JsCallContext *context);
  void SetEventWaiting(JsCallContext *context);
  void GetEventTimeUpdate(JsCallContext *context);
  void SetEventTimeUpdate(JsCallContext *context);
  void GetEventEnded(JsCallContext *context);
  void SetEventEnded(JsCallContext *context);
  void GetEventDataUnavailable(JsCallContext *context);
  void SetEventDataUnavailable(JsCallContext *context);
  void GetEventCanShowCurrentFrame(JsCallContext *context);
  void SetEventCanShowCurrentFrame(JsCallContext *context);
  void GetEventCanPlay(JsCallContext *context);
  void SetEventCanPlay(JsCallContext *context);
  void GetEventCanPlayThrough(JsCallContext *context);
  void SetEventCanPlayThrough(JsCallContext *context);
  void GetEventRateChange(JsCallContext *context);
  void SetEventRateChange(JsCallContext *context);
  void GetEventDurationChange(JsCallContext *context);
  void SetEventDurationChange(JsCallContext *context);
  void GetEventVolumeChange(JsCallContext *context);
  void SetEventVolumeChange(JsCallContext *context);

 protected:
  scoped_refptr<MediaData> media_data_;

  // network state
  bool loaded_first_frame_;

  // ready state
  int ready_state_;
  bool seeking_;

  // playback state
  double cur_playback_position_;
  bool paused_;
  double default_playback_rate_;
  double playback_rate_;
  bool autoplaying_;

  // looping
  double start_;
  double end_;
  double loop_start_;
  double loop_end_;
  uint64 play_count_;
  uint64 current_loop_;

  // controls
  double volume_;
  bool muted_;

 private:
  // network state
  std::string16 src_;
  DISALLOW_EVIL_CONSTRUCTORS(GearsMedia);
};

#define REGISTER_MEDIA_PROPERTIES_AND_METHODS(GearsMediaType)                 \
  template<>                                                                  \
  void Dispatcher<GearsMediaType>::Init() {                                   \
    RegisterProperty("error", &GearsMediaType::GetError, NULL);               \
                                                                              \
    RegisterProperty("src", &GearsMediaType::GetSrc,                          \
                     &GearsMediaType::SetSrc);                                \
    RegisterProperty("currentSrc", &GearsMediaType::GetCurrentSrc, NULL);     \
    REGISTER_CONSTANT(EMPTY, GearsMediaType);                                 \
    REGISTER_CONSTANT(LOADING, GearsMediaType);                               \
    REGISTER_CONSTANT(LOADED_METADATA, GearsMediaType);                       \
    REGISTER_CONSTANT(LOADED_FIRST_FRAME, GearsMediaType);                    \
    REGISTER_CONSTANT(LOADED, GearsMediaType);                                \
    RegisterProperty("networkState", &GearsMediaType::GetNetworkState,        \
                     NULL);                                                   \
    RegisterProperty("bufferingRate", &GearsMediaType::GetBufferingRate,      \
                     NULL);                                                   \
    RegisterProperty("buffered", &GearsMediaType::GetBuffered,                \
                     NULL);                                                   \
    RegisterMethod("load", &GearsMediaType::Load);                            \
                                                                              \
    REGISTER_CONSTANT(DATA_UNAVAILABLE, GearsMediaType);                      \
    REGISTER_CONSTANT(CAN_SHOW_CURRENT_FRAME, GearsMediaType);                \
    REGISTER_CONSTANT(CAN_PLAY, GearsMediaType);                              \
    REGISTER_CONSTANT(CAN_PLAY_THROUGH, GearsMediaType);                      \
    RegisterProperty("readyState", &GearsMediaType::GetReadyState,            \
                     NULL);                                                   \
    RegisterProperty("seeking", &GearsMediaType::IsSeeking, NULL);            \
                                                                              \
    RegisterProperty("currentTime", &GearsMediaType::CurrentTime,             \
                     &GearsMediaType::SetCurrentTime);                        \
    RegisterProperty("duration", &GearsMediaType::GetDuration, NULL);         \
    RegisterProperty("paused", &GearsMediaType::IsPaused, NULL);              \
    RegisterProperty("defaultPlaybackRate",                                   \
                     &GearsMediaType::GetDefaultPlaybackRate,                 \
                     &GearsMediaType::SetDefaultPlaybackRate);                \
    RegisterProperty("playbackRate", &GearsMediaType::GetPlaybackRate,        \
                     &GearsMediaType::SetPlaybackRate);                       \
    RegisterProperty("played", &GearsMediaType::GetPlayedTimeRanges,          \
                     NULL);                                                   \
    RegisterProperty("seekable", &GearsMediaType::GetSeekableTimeRanges,      \
                     NULL);                                                   \
    RegisterProperty("ended", &GearsMediaType::IsEnded, NULL);                \
    RegisterProperty("autoplay", &GearsMediaType::IsAutoPlay,                 \
                     &GearsMediaType::SetAutoPlay);                           \
    RegisterMethod("play", &GearsMediaType::Play);                            \
    RegisterMethod("pause", &GearsMediaType::Pause);                          \
                                                                              \
    RegisterProperty("start",  &GearsMediaType::GetStart,                     \
                     &GearsMediaType::SetStart);                              \
    RegisterProperty("end", &GearsMediaType::GetEnd,                          \
                     &GearsMediaType::SetEnd);                                \
    RegisterProperty("loopStart", &GearsMediaType::GetLoopStart,              \
                     &GearsMediaType::SetLoopStart);                          \
    RegisterProperty("loopEnd", &GearsMediaType::GetLoopEnd,                  \
                     &GearsMediaType::SetLoopEnd);                            \
    RegisterProperty("playCount", &GearsMediaType::GetPlayCount,              \
                     &GearsMediaType::SetPlayCount);                          \
    RegisterProperty("currentLoop", &GearsMediaType::GetCurrentLoop,          \
                     &GearsMediaType::SetCurrentLoop);                        \
                                                                              \
    RegisterMethod("addCueRange", &GearsMediaType::AddCueRange);              \
    RegisterMethod("removeCueRanges", &GearsMediaType::RemoveCueRanges);      \
                                                                              \
    RegisterProperty("controls", &GearsMediaType::HasControls,                \
                     &GearsMediaType::SetControls);                           \
    RegisterProperty("volume", &GearsMediaType::GetVolume,                    \
                     &GearsMediaType::SetVolume);                             \
    RegisterProperty("muted", &GearsMediaType::IsMuted,                       \
                     &GearsMediaType::SetMuted);                              \
                                                                              \
    RegisterMethod("loadBlob", &GearsMediaType::LoadBlob);                    \
                                                                              \
    RegisterProperty("onbegin", &GearsMediaType::GetEventBegin,               \
                     &GearsMediaType::SetEventBegin);                         \
    RegisterProperty("onprogress", &GearsMediaType::GetEventProgress,         \
                     &GearsMediaType::SetEventProgress);                      \
    RegisterProperty("onloadedmetadata",                                      \
                     &GearsMediaType::GetEventLoadedMetadata,                 \
                     &GearsMediaType::SetEventLoadedMetadata);                \
    RegisterProperty("onloadedfirstframe",                                    \
                     &GearsMediaType::GetEventLoadedFirstFrame,               \
                     &GearsMediaType::SetEventLoadedFirstFrame);              \
    RegisterProperty("onload", &GearsMediaType::GetEventLoad,                 \
                     &GearsMediaType::SetEventLoad);                          \
    RegisterProperty("onabort", &GearsMediaType::GetEventAbort,               \
                     &GearsMediaType::SetEventAbort);                         \
    RegisterProperty("onerror", &GearsMediaType::GetEventError,               \
                     &GearsMediaType::SetEventError);                         \
    RegisterProperty("onemptied", &GearsMediaType::GetEventEmptied,           \
                     &GearsMediaType::SetEventEmptied);                       \
    RegisterProperty("onstalled", &GearsMediaType::GetEventStalled,           \
                     &GearsMediaType::SetEventStalled);                       \
    RegisterProperty("onplay", &GearsMediaType::GetEventPlay,                 \
                     &GearsMediaType::SetEventPlay);                          \
    RegisterProperty("onpause", &GearsMediaType::GetEventPause,               \
                     &GearsMediaType::SetEventPause);                         \
    RegisterProperty("onwaiting", &GearsMediaType::GetEventWaiting,           \
                     &GearsMediaType::SetEventWaiting);                       \
    RegisterProperty("ontimeupdate", &GearsMediaType::GetEventTimeUpdate,     \
                     &GearsMediaType::SetEventTimeUpdate);                    \
    RegisterProperty("onended", &GearsMediaType::GetEventEnded,               \
                     &GearsMediaType::SetEventEnded);                         \
    RegisterProperty("ondataunavailable",                                     \
                     &GearsMediaType::GetEventDataUnavailable,                \
                     &GearsMediaType::SetEventDataUnavailable);               \
    RegisterProperty("oncanshowcurrentframe",                                 \
                     &GearsMediaType::GetEventCanShowCurrentFrame,            \
                     &GearsMediaType::SetEventCanShowCurrentFrame);           \
    RegisterProperty("oncanplay", &GearsMediaType::GetEventCanPlay,           \
                     &GearsMediaType::SetEventCanPlay);                       \
    RegisterProperty("oncanplaythrough",                                      \
                     &GearsMediaType::GetEventCanPlayThrough,                 \
                     &GearsMediaType::SetEventCanPlayThrough);                \
    RegisterProperty("onratechange", &GearsMediaType::GetEventRateChange,     \
                     &GearsMediaType::SetEventRateChange);                    \
    RegisterProperty("ondurationchange",                                      \
                     &GearsMediaType::GetEventDurationChange,                 \
                     &GearsMediaType::SetEventDurationChange);                \
    RegisterProperty("onvolumechange",                                        \
                     &GearsMediaType::GetEventVolumeChange,                   \
                     &GearsMediaType::SetEventVolumeChange);                  \
  }

#endif  // GEARS_MEDIA_MEDIA_H__

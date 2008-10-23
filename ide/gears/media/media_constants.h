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
// This file defines constants used by classes related to media.

#ifndef GEARS_MEDIA_MEDIA_CONSTANTS_H__
#define GEARS_MEDIA_MEDIA_CONSTANTS_H__

namespace MediaConstants {
  // error states
  const int MEDIA_NO_ERROR = 0;
  const int MEDIA_ERR_ABORTED = 1;
  const int MEDIA_ERR_NETWORK = 2;
  const int MEDIA_ERR_DECODE = 3;

  // network states
  const int NETWORK_STATE_EMPTY = 0;
  const int NETWORK_STATE_LOADING = 1;
  const int NETWORK_STATE_LOADED_METADATA = 2;
  const int NETWORK_STATE_LOADED_FIRST_FRAME = 3;
  const int NETWORK_STATE_LOADED = 4;

  // ready states
  const int READY_STATE_DATA_UNAVAILABLE = 0;
  const int READY_STATE_CAN_SHOW_CURRENT_FRAME = 1;
  const int READY_STATE_CAN_PLAY = 2;
  const int READY_STATE_CAN_PLAY_THROUGH = 3;
};

#endif  // GEARS_MEDIA_MEDIA_CONSTANTS_H__

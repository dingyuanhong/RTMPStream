/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "screen_capturer.h"

#include <memory>
#include <utility>

#include "desktop_capture_options.h"
#include "win/screen_capturer_win_gdi.h"

namespace webrtc {

// static
ScreenCapturer* ScreenCapturer::Create(const DesktopCaptureOptions& options) {
  std::unique_ptr<ScreenCapturer> gdi_capturer(
      new ScreenCapturerWinGdi(options));

  return gdi_capturer.release();
}

}  // namespace webrtc

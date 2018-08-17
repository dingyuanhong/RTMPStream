/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "win32.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>

namespace rtc {

bool GetOsVersion(int* major, int* minor, int* build) {
  OSVERSIONINFO info = {0};
  info.dwOSVersionInfoSize = sizeof(info);
  if (GetVersionEx(&info)) {
    if (major) *major = info.dwMajorVersion;
    if (minor) *minor = info.dwMinorVersion;
    if (build) *build = info.dwBuildNumber;
    return true;
  }
  return false;
}

}  // namespace rtc

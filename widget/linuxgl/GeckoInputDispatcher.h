/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: sw=2 ts=8 et ft=cpp : */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Code.
 *
 * The Initial Developer of the Original Code is
 *   The Mozilla Foundation
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Jones <jones.chris.g@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <queue>
#include "mozilla/Mutex.h"
#include "nsAppShell.h"
#include "nsStringGlue.h"

#ifdef MOZ_UDEV
extern "C" {
#include <libudev.h>
}
enum MUDeviceType {
    UDev_Mouse = 0x01,
    UDev_Touchpad = 0x02,
    UDev_Touchscreen = 0x04
};
#endif

namespace mozilla {

struct Touch {
    int32_t id;
    int x;
    int y;
};

struct UserInputData {
    uint64_t timeMs;
    enum {
        MOTION_DATA,
        KEY_DATA
    } type;
    int32_t action;
    int32_t flags;
    int32_t metaState;
    union {
        struct {
            int32_t keyCode;
            int32_t scanCode;
        } key;
        struct {
            int32_t touchCount;
            Touch touches[10];
        } motion;
    };
};

class GeckoInputDispatcher {
    NS_INLINE_DECL_REFCOUNTING(GeckoInputDispatcher)
public:
    GeckoInputDispatcher(nsAppShell* aAppShell);

    virtual void PushUserData(UserInputData& data);

    // Called on the main thread
    virtual void dispatchOnce();
    virtual ~GeckoInputDispatcher() {}
    static bool UdevDevicePath(int type, nsCString& aResult);
    void Painted();

private:
    void Init();
    static void MouseGenericHandlerS(int fd, FdHandler *data);
    void MouseGenericHandler(int fd);
    static void x11HandleEvent(int fd, FdHandler*);
    inline void DispatchMotionToMainThread();
    void SendMouseEvent();
    void SendMouseEventS(PRUint32 msg, uint64_t timeMs, int x, int y);

    // mQueueLock should generally be locked while using mEventQueue.
    // UserInputData is pushed on on the InputReaderThread and
    // popped and dispatched on the main thread.
    mozilla::Mutex mQueueLock;
    nsAppShell* mAppShell;
    int mMouseDev;
    nsCString mMouseDevNode;
    std::queue<UserInputData> mEventQueue;
    int m_x;
    int m_y;
    struct timeval last_evtime;
    int m_prevx, m_prevy;
    int m_buttons;
    bool m_compression;
    int m_jitterLimitSquared;
    bool mTimerStarted;
};

} // namespace mozilla

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: sw=2 ts=4 et ft=cpp : */
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

#include "GeckoInputDispatcher.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include "nsWindow.h"
#include "gfxLinuxGLPlatform.h"

using namespace std;

namespace mozilla {

GeckoInputDispatcher::GeckoInputDispatcher(nsAppShell* aAppShell)
  : mQueueLock("GeckoInputDispatcher::mQueueMutex")
  , mAppShell(aAppShell)
  , mMouseDev(-1)
{
    mTimerStarted = false;
    m_compression = true;
    m_jitterLimitSquared = 9;
    m_x = m_y = m_prevx = m_prevy = m_buttons = 0;
    Init();
}

void GeckoInputDispatcher::Init()
{
    nsresult rv;
#ifdef MOZ_X11
    if (!getenv("NO_XEVENTS") && !getenv("OMTC")) {
        int fd = -1;
        Display* dpy = gfxLinuxGLPlatform::GetXDisplay();
        if (dpy) {
            fd = ConnectionNumber(dpy);
            if (fd) {
                mMouseDev = fd;
                rv = mAppShell->AddFdHandler(mMouseDev, x11HandleEvent, "X11 handler", this);
                NS_ENSURE_SUCCESS(rv,);
                printf("Use X mouse: fd:%i\n", fd);
                return;
            }
        }
    }
#endif
    static char* customDev = getenv("MOZ_MOUSE_DEV");
    if (customDev) {
        if ((mMouseDev = open(customDev, O_RDONLY | O_NONBLOCK /*O_NDELAY*/ )) != -1) {
            rv = mAppShell->AddFdHandler(mMouseDev, MouseGenericHandlerS, customDev, this);
            NS_ENSURE_SUCCESS(rv,);
            printf("Use usedv mouse: fd:%i\n", mMouseDev);
            mMouseDevNode.Assign(customDev);
            return;
        } else {
            printf("error: opening custom mouse device\n");
        }
    } else {
#ifdef MOZ_UDEV
        if (mMouseDev < 0) {
            UdevDevicePath(UDev_Mouse | UDev_Touchpad, mMouseDevNode);
            if ((mMouseDev = open(mMouseDevNode.get(), O_RDONLY | O_NONBLOCK /*O_NDELAY*/ )) != -1) {
                rv = mAppShell->AddFdHandler(mMouseDev, MouseGenericHandlerS, mMouseDevNode.get(), this);
                NS_ENSURE_SUCCESS(rv,);
                printf("Use usedv mouse: fd:%i\n", mMouseDev);
            } else {
                printf("error: opening mouse device\n");
                mMouseDevNode.SetLength(0);
            }
        }
#endif // MOZ_UDEV
    }
}

void GeckoInputDispatcher::dispatchOnce()
{
    UserInputData data;
    {
        MutexAutoLock lock(mQueueLock);
        if (mEventQueue.empty())
            return;
        data = mEventQueue.front();
        mEventQueue.pop();
        if (!mEventQueue.empty())
            mAppShell->NotifyNativeEvent();
    }

    switch (data.type) {
    case UserInputData::MOTION_DATA: {
        PRUint32 msg = data.action;
        SendMouseEventS(msg,
                        data.timeMs,
                        data.motion.touches[0].x,
                        data.motion.touches[0].y);
        break;
    }
    case UserInputData::KEY_DATA:
        printf("Key Data event.type:%i\n", data.action);
        break;
    }
}

void
GeckoInputDispatcher::PushUserData(UserInputData& data)
{
    MutexAutoLock lock(mQueueLock);
    if (!mEventQueue.empty() &&
         mEventQueue.back().type == UserInputData::MOTION_DATA &&
        (mEventQueue.back().action == NS_MOUSE_MOVE))
        mEventQueue.back() = data;
    else
        mEventQueue.push(data);
}

#ifdef MOZ_UDEV
bool
GeckoInputDispatcher::UdevDevicePath(int type, nsCString& aResult)
{
    bool retval = false;
    udev *u = udev_new();
    udev_enumerate *ue = udev_enumerate_new(u);
    udev_enumerate_add_match_subsystem(ue, "input");
    if (type & UDev_Mouse)
        udev_enumerate_add_match_property(ue, "ID_INPUT_MOUSE", "1");
    if (type & UDev_Touchpad)
        udev_enumerate_add_match_property(ue, "ID_INPUT_TOUCHPAD", "1");
    if (type & UDev_Touchscreen)
        udev_enumerate_add_match_property(ue, "ID_INPUT_TOUCHSCREEN", "1");
    udev_enumerate_scan_devices(ue);
    udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(ue)) {
        const char *syspath = udev_list_entry_get_name(entry);
        udev_device *udevice = udev_device_new_from_syspath(u, syspath);
        const char* candidate1 = udev_device_get_devnode(udevice);
        if (candidate1 && strstr(candidate1, "/dev/input/event")) {
            aResult.Assign(candidate1);
            retval = true;
        }
        udev_device_unref(udevice);
    }
    udev_enumerate_unref(ue);
    udev_unref(u);
    return retval;
}
#endif

void
GeckoInputDispatcher::SendMouseEventS(PRUint32 msg, uint64_t timeMs, int x, int y)
{
    nsMouseEvent event(true, msg, NULL,
                       nsMouseEvent::eReal, nsMouseEvent::eNormal);

    event.refPoint.x = x;
    event.refPoint.y = y;
    event.time = timeMs;
    event.button = nsMouseEvent::eLeftButton;
    if (msg != NS_MOUSE_MOVE) {
        event.clickCount = 1;
    }

    nsWindow::DispatchInputEvent(event);
}

void
GeckoInputDispatcher::SendMouseEvent()
{
    int posx = m_x;
    int posy = m_y;
    static int prev_mbuttons = 0;
    PRUint32 msg = NS_MOUSE_MOVE;
    if (prev_mbuttons != m_buttons) {
        if (prev_mbuttons == 0) {
            msg = NS_MOUSE_BUTTON_DOWN;
        } else {
            msg = NS_MOUSE_BUTTON_UP;
        }
    }
    if (m_x < 0) {
        m_x = 0;
    }
    if (m_y < 0) {
        m_y = 0;
    }

    UserInputData data;
    data.type = UserInputData::MOTION_DATA;
    data.action = msg;
    data.timeMs = last_evtime.tv_usec;
    data.motion.touches[0].x = m_x;
    data.motion.touches[0].y = m_y;
    PushUserData(data);
    mAppShell->NotifyNativeEvent();

    prev_mbuttons = m_buttons;
    m_prevx = m_x;
    m_prevy = m_y;
    mTimerStarted = false;
}

void GeckoInputDispatcher::DispatchMotionToMainThread()
{
    if (!mTimerStarted) {
        nsCOMPtr<nsIRunnable> event =
          NS_NewRunnableMethod(this, &GeckoInputDispatcher::SendMouseEvent);
        NS_DispatchToMainThread(event);
        mTimerStarted = true;
    }
}

void
GeckoInputDispatcher::MouseGenericHandlerS(int fd, FdHandler *data)
{
    if (data->data) {
      static_cast<GeckoInputDispatcher*>(data->data)->MouseGenericHandler(fd);
    }
}

void
GeckoInputDispatcher::MouseGenericHandler(int fd)
{
    struct ::input_event buffer[32];
    int n = 0;
    bool posChanged = false;
    bool pendingMouseEvent = false;
    int eventCompressCount = 0;

    while (1 == 1) {
        n = read(fd, reinterpret_cast<char *>(buffer) + n, sizeof(buffer) - n);
        if (n == 0) {
            printf("Got EOF from the input device.\n");
            return;
        } else if (n < 0 && (errno != EINTR && errno != EAGAIN)) {
            printf("Could not read from input device: %s\n", strerror(errno));
            return;
        } else if (n % sizeof(buffer[0]) == 0) {
            break;
        }
    }

    n /= sizeof(buffer[0]);

    for (int i = 0; i < n; ++i) {
        struct ::input_event *data = &buffer[i];
        last_evtime = data->time;
        if (data->type == EV_ABS) {
            if (data->code == ABS_X && m_x != data->value) {
                m_x = data->value;
                posChanged = true;
            } else if (data->code == ABS_Y && m_y != data->value) {
                m_y = data->value;
                posChanged = true;
            }
        } else if (data->type == EV_REL) {
            if (data->code == REL_X) {
                m_x += data->value;
                posChanged = true;
            } else if (data->code == REL_Y) {
                m_y += data->value;
                posChanged = true;
            } else if (data->code == ABS_WHEEL) { // vertical scroll
                // data->value: 1 == up, -1 == down
                int delta = 120 * data->value;
                // printf(">>>>>>Func:%s::%d m(%i,%i) del:%i, vertical\n", __PRETTY_FUNCTION__, __LINE__, m_x, m_y, delta);
            } else if (data->code == ABS_THROTTLE) { // horizontal scroll
                // data->value: 1 == right, -1 == left
                int delta = 120 * -data->value;
                // printf(">>>>>>Func:%s::%d m(%i,%i) del:%i, horizontal\n", __PRETTY_FUNCTION__, __LINE__, m_x, m_y, delta);
            }
        } else if (data->type == EV_KEY && data->code == BTN_TOUCH) {
            m_buttons = data->value ? 1 : 0;
            SendMouseEvent();
            pendingMouseEvent = false;
        } else if (data->type == EV_KEY && data->code >= BTN_LEFT && data->code <= BTN_MIDDLE) {
            int button = 0;
            switch (data->code) {
            case BTN_LEFT:
                button = 1;
                break;
            case BTN_MIDDLE:
                button = 3;
                break;
            case BTN_RIGHT:
                button = 2;
                break;
            }
            if (data->value)
                m_buttons |= button;
            else
                m_buttons &= ~button;
            SendMouseEvent();
            pendingMouseEvent = false;
        } else if (data->type == EV_SYN && data->code == SYN_REPORT) {
            if (posChanged) {
                posChanged = false;
                if (m_compression) {
                    pendingMouseEvent = true;
                    eventCompressCount++;
                } else {
                    DispatchMotionToMainThread();
                }
            }
        } else if (data->type == EV_MSC && data->code == MSC_SCAN) {
            // kernel encountered an unmapped key - just ignore it
            continue;
        }
    }
    if (m_compression && pendingMouseEvent) {
        int distanceSquared = (m_x - m_prevx)*(m_x - m_prevx) + (m_y - m_prevy)*(m_y - m_prevy);
        if (distanceSquared > m_jitterLimitSquared) {
            DispatchMotionToMainThread();
        }
    }
}

#ifdef MOZ_X11
void
GeckoInputDispatcher::x11HandleEvent(int fd, FdHandler* udata)
{
    Display* mDpy = gfxLinuxGLPlatform::GetXDisplay();

    GeckoInputDispatcher* self = static_cast<GeckoInputDispatcher*>(udata->data);
    if (!self) {
        return;
    }
    XEvent event;
    while(XPending(mDpy)) {
        XNextEvent(mDpy, &event);
        UserInputData data;
        switch (event.type) {
        case 4:
        case 5: {
            XButtonEvent& lastMotion = event.xbutton;
            data.type = UserInputData::MOTION_DATA;
            data.action = event.type == 4 ? NS_MOUSE_BUTTON_DOWN : NS_MOUSE_BUTTON_UP;
            data.motion.touches[0].x = lastMotion.x;
            data.motion.touches[0].y = lastMotion.y;
            break;
        }
        case 6: {
            XMotionEvent& lastMotion = event.xmotion;
            data.type = UserInputData::MOTION_DATA;
            data.action = NS_MOUSE_MOVE;
            data.motion.touches[0].x = lastMotion.x;
            data.motion.touches[0].y = lastMotion.y;
            break;
        }
        default:
            continue;
        }
        self->PushUserData(data);
    }
    self->mAppShell->NotifyNativeEvent();
}
#endif

} // namespace mozilla

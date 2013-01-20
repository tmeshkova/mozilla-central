/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=4 sw=4 sts=4 tw=80 et: */
/* Copyright 2012 Mozilla Foundation and Mozilla contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/basictypes.h"
#include "nscore.h"
#include "mozilla/FileUtils.h"
#include "mozilla/Hal.h"
#include "mozilla/Mutex.h"
#include "mozilla/Services.h"
#include "nsAppShell.h"
#include "nsDOMTouchEvent.h"
#include "nsGkAtoms.h"
#include "nsGUIEvent.h"
#include "nsIObserverService.h"
#include "nsIScreen.h"
#include "nsScreenManagerLinuxGL.h"
#include "nsWindow.h"
#include "OrientationObserver.h"

#include "sampler.h"

#ifdef VERBOSE_LOG_ENABLED
# define VERBOSE_LOG(FMT, ARG...)
    printf("LinuxGL:%s:%s :%d: " FMT "\n", __FILE__, __FUNCTION__, __LINE__, ## ARG)
#else
# define VERBOSE_LOG(args...)                   \
    (void)0
#endif

using namespace android;
using namespace mozilla;
using namespace mozilla::dom;
using namespace mozilla::services;
using namespace mozilla::widget;

bool gDrawRequest = false;
static nsAppShell *gAppShell = NULL;
static int epollfd = 0;
static int signalfds[2] = {0};

NS_IMPL_ISUPPORTS_INHERITED1(nsAppShell, nsBaseAppShell, nsIObserver)

namespace mozilla {

bool ProcessNextEvent()
{
    return gAppShell->ProcessNextNativeEvent(true);
}

void NotifyEvent()
{
    gAppShell->NotifyNativeEvent();
}

}

static void
pipeHandler(int fd, FdHandler *data)
{
    ssize_t len;
    do {
        char tmp[32];
        len = read(fd, tmp, sizeof(tmp));
    } while (len > 0);
}

nsAppShell::nsAppShell()
    : mNativeCallbackRequest(false)
    , mHandlers()
    , mEnableDraw(false)
{
    gAppShell = this;
}

nsAppShell::~nsAppShell()
{
    gAppShell = NULL;
}

nsresult
nsAppShell::Init()
{
    nsresult rv = nsBaseAppShell::Init();
    NS_ENSURE_SUCCESS(rv, rv);

    epollfd = epoll_create(16);
    NS_ENSURE_TRUE(epollfd >= 0, NS_ERROR_UNEXPECTED);

    int ret = pipe2(signalfds, O_NONBLOCK);
    NS_ENSURE_FALSE(ret, NS_ERROR_UNEXPECTED);

    rv = AddFdHandler(signalfds[0], pipeHandler, "");
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIObserverService> obsServ = GetObserverService();
    if (obsServ) {
        obsServ->AddObserver(this, "browser-ui-startup-complete", false);
    }

    // Delay initializing input devices until the screen has been
    // initialized (and we know the resolution).
    return rv;
}

NS_IMETHODIMP
nsAppShell::Observe(nsISupports* aSubject,
                    const char* aTopic,
                    const PRUnichar* aData)
{
    if (strcmp(aTopic, "browser-ui-startup-complete")) {
        return nsBaseAppShell::Observe(aSubject, aTopic, aData);
    }

    mEnableDraw = true;
    NotifyEvent();
    return NS_OK;
}

NS_IMETHODIMP
nsAppShell::Exit()
{
    OrientationObserver::ShutDown();
    nsCOMPtr<nsIObserverService> obsServ = GetObserverService();
    if (obsServ) {
        obsServ->RemoveObserver(this, "browser-ui-startup-complete");
    }
    return nsBaseAppShell::Exit();
}

void
nsAppShell::InitInputDevices()
{
}

nsresult
nsAppShell::AddFdHandler(int fd, FdHandlerCallback handlerFunc,
                         const char* deviceName)
{
    epoll_event event = {
        EPOLLIN,
        { 0 }
    };

    FdHandler *handler = mHandlers.AppendElement();
    handler->fd = fd;
    strncpy(handler->name, deviceName, sizeof(handler->name) - 1);
    handler->func = handlerFunc;
    event.data.u32 = mHandlers.Length() - 1;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) ?
           NS_ERROR_UNEXPECTED : NS_OK;
}

void
nsAppShell::ScheduleNativeEventCallback()
{
    mNativeCallbackRequest = true;
    NotifyEvent();
}

bool
nsAppShell::ProcessNextNativeEvent(bool mayWait)
{
    SAMPLE_LABEL("nsAppShell", "ProcessNextNativeEvent");
    epoll_event events[16] = {{ 0 }};

    int event_count;
    {
        SAMPLE_LABEL("nsAppShell", "ProcessNextNativeEvent::Wait");
        if ((event_count = epoll_wait(epollfd, events, 16,  mayWait ? -1 : 0)) <= 0)
            return true;
    }

    for (int i = 0; i < event_count; i++)
        mHandlers[events[i].data.u32].run();

    // NativeEventCallback always schedules more if it needs it
    // so we can coalesce these.
    // See the implementation in nsBaseAppShell.cpp for more info
    if (mNativeCallbackRequest) {
        mNativeCallbackRequest = false;
        NativeEventCallback();
    }

    if (gDrawRequest && mEnableDraw) {
        gDrawRequest = false;
        nsWindow::DoDraw();
    }

    return true;
}

void
nsAppShell::NotifyNativeEvent()
{
    write(signalfds[1], "w", 1);
}

/* static */ void
nsAppShell::NotifyScreenInitialized()
{
    gAppShell->InitInputDevices();

    // Getting the instance of OrientationObserver to initialize it.
    OrientationObserver::GetInstance();
}

/* static */ void
nsAppShell::NotifyScreenRotation()
{
    hal::NotifyScreenConfigurationChange(nsScreenLinuxGL::GetConfiguration());
}

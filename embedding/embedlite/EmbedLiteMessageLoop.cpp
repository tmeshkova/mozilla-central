/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define LOG_COMPONENT "EmbedLiteMessageLoop"
#include "EmbedLog.h"

#include "EmbedLiteMessageLoop.h"
#include "EmbedLiteApp.h"

#include "base/message_pump_default.h"
#include "base/logging.h"
#include "base/scoped_nsautorelease_pool.h"
#include "base/message_pump.h"
#include "base/time.h"

using namespace base;

namespace mozilla {
namespace embedlite {

class MessagePumpEmbed : public MessagePump
{
public:
  MessagePumpEmbed(EmbedLiteMessageLoopListener* aListener)
    : mListener(aListener)
  {
  }
  ~MessagePumpEmbed()
  {
  }
  virtual void Run(Delegate* delegate)
  {
    mListener->Run(delegate);
  }
  virtual void Quit()
  {
    mListener->Quit();
  }
  virtual void ScheduleWork()
  {
    mListener->ScheduleWork();
  }
  virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time)
  {
    delayed_work_time_ = delayed_work_time;
    TimeDelta delay = delayed_work_time_ - TimeTicks::Now();
    int laterMsecs = delay.InMilliseconds() > std::numeric_limits<int>::max() ?
      std::numeric_limits<int>::max() : delay.InMilliseconds();
    mListener->ScheduleDelayedWork(laterMsecs);
  }
  TimeTicks& DelayedWorkTime()
  {
    return delayed_work_time_;
  }

protected:
  EmbedLiteMessageLoopListener* mListener;

private:
  TimeTicks delayed_work_time_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpEmbed);
};

EmbedLiteMessageLoop::EmbedLiteMessageLoop(EmbedLiteMessageLoopListener* aListener)
  : mListener(aListener)
  , mEmbedPump(new MessagePumpEmbed(aListener))
{
}

EmbedLiteMessageLoop::~EmbedLiteMessageLoop()
{
}

base::MessagePump*
EmbedLiteMessageLoop::GetPump()
{
  return mEmbedPump;
}

bool EmbedLiteMessageLoop::DoWork(void* aDelegate)
{
  return static_cast<base::MessagePump::Delegate*>(aDelegate)->DoWork();
}

bool EmbedLiteMessageLoop::DoDelayedWork(void* aDelegate)
{
  return static_cast<base::MessagePump::Delegate*>(aDelegate)->DoDelayedWork(&mEmbedPump->DelayedWorkTime());
}

bool EmbedLiteMessageLoop::DoIdleWork(void* aDelegate)
{
  return static_cast<base::MessagePump::Delegate*>(aDelegate)->DoIdleWork();
}

} // namespace embedlite
} // namespace mozilla

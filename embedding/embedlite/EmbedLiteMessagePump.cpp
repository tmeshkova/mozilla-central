/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define LOG_COMPONENT "EmbedLiteMessagePump"
#include "EmbedLog.h"

#include "EmbedLiteMessagePump.h"
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
  MessagePumpEmbed(EmbedLiteMessagePumpListener* aListener)
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
    ScheduleDelayedWorkIfNeeded(delayed_work_time_);
  }
  virtual void ScheduleDelayedWorkIfNeeded(const TimeTicks& delayed_work_time)
  {
    if (delayed_work_time.is_null()) {
      mListener->ScheduleDelayedWork(-1);
      return;
    }

    TimeDelta delay = delayed_work_time - TimeTicks::Now();
    int laterMsecs = delay.InMilliseconds() > std::numeric_limits<int>::max() ?
      std::numeric_limits<int>::max() : delay.InMilliseconds();
    mListener->ScheduleDelayedWork(laterMsecs);
  }
  TimeTicks& DelayedWorkTime()
  {
    return delayed_work_time_;
  }

protected:
  EmbedLiteMessagePumpListener* mListener;

private:
  TimeTicks delayed_work_time_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpEmbed);
};

EmbedLiteMessagePump::EmbedLiteMessagePump(EmbedLiteMessagePumpListener* aListener)
  : mListener(aListener)
  , mEmbedPump(new MessagePumpEmbed(aListener))
{
}

EmbedLiteMessagePump::~EmbedLiteMessagePump()
{
}

base::MessagePump*
EmbedLiteMessagePump::GetPump()
{
  return mEmbedPump;
}

bool EmbedLiteMessagePump::DoWork(void* aDelegate)
{
  return static_cast<base::MessagePump::Delegate*>(aDelegate)->DoWork();
}

bool EmbedLiteMessagePump::DoDelayedWork(void* aDelegate)
{
  bool retval = static_cast<base::MessagePump::Delegate*>(aDelegate)->DoDelayedWork(&mEmbedPump->DelayedWorkTime());
  mEmbedPump->ScheduleDelayedWorkIfNeeded(mEmbedPump->DelayedWorkTime());
  return retval;
}

bool EmbedLiteMessagePump::DoIdleWork(void* aDelegate)
{
  return static_cast<base::MessagePump::Delegate*>(aDelegate)->DoIdleWork();
}

} // namespace embedlite
} // namespace mozilla

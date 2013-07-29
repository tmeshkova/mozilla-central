/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EMBED_LITE_MESSAGE_LOOP_H
#define EMBED_LITE_MESSAGE_LOOP_H

namespace base {
class MessagePump;
}

namespace mozilla {
namespace embedlite {

class EmbedLiteApp;
class MessagePumpEmbed;
class EmbedLiteMessageLoopListener
{
public:
  virtual void Run(void* aDelegate) = 0;
  virtual void Quit() = 0;
  virtual void ScheduleWork() = 0;
  virtual void ScheduleDelayedWork(const int aDelay) = 0;
};

class EmbedLiteMessageLoop
{
public:
  EmbedLiteMessageLoop(EmbedLiteMessageLoopListener* aListener);
  virtual ~EmbedLiteMessageLoop();
  virtual base::MessagePump* GetPump();

  virtual bool DoWork(void* aDelegate);
  virtual bool DoDelayedWork(void* aDelegate);
  virtual bool DoIdleWork(void* aDelegate);

private:
  EmbedLiteMessageLoopListener* mListener;
  MessagePumpEmbed* mEmbedPump;
};

} // namespace embedlite
} // namespace mozilla

#endif // EMBED_LITE_MESSAGE_LOOP_H

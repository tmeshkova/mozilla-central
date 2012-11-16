/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// Original author: ekr@rtfm.com
#include <prlog.h>

#include "logging.h"
#include "transportflow.h"
#include "transportlayer.h"

// Logging context
namespace mozilla {

MOZ_MTLOG_MODULE("mtransport")

nsresult TransportLayer::Init() {
  if (state_ != TS_NONE)
    return state_ == TS_ERROR ? NS_ERROR_FAILURE : NS_OK;

  nsresult rv = InitInternal();

  if (!NS_SUCCEEDED(rv)) {
    state_ = TS_ERROR;
    return rv;
  }
  state_ = TS_INIT;

  return NS_OK;
}

void TransportLayer::Inserted(TransportFlow *flow, TransportLayer *downward) {
  flow_ = flow;
  downward_ = downward;

  MOZ_MTLOG(PR_LOG_DEBUG, LAYER_INFO << "Inserted: downward='" <<
    (downward ? downward->id(): "none") << "'");

  WasInserted();
}

void TransportLayer::SetState(State state) {
  if (state != state_) {
    MOZ_MTLOG(PR_LOG_DEBUG, LAYER_INFO << "state " << state_ << "->" << state);
    state_ = state;
    SignalStateChange(this, state);
  }
}

const std::string& TransportLayer::flow_id() {
    static const std::string empty;

    return flow_ ? flow_->id() : empty;
  }
}  // close namespace

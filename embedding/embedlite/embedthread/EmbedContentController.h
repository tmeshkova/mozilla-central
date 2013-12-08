/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZ_EMBED_CONTENT_CONTROLLER_H
#define MOZ_EMBED_CONTENT_CONTROLLER_H

#include "mozilla/layers/AsyncPanZoomController.h"
#include "mozilla/layers/GeckoContentController.h"
#include "FrameMetrics.h"

namespace mozilla {
namespace embedlite {
class EmbedLiteViewListener;
class EmbedLiteViewThreadParent;

/**
 * Currently EmbedAsyncPanZoomController is needed because we need to do extra steps when panning ends.
 * This is not optimal way to implement HandlePanEnd as AsyncPanZoomController
 * invokes GeckoContentController::HandlePanEnd and overridden EmbedContentController::HandlePanEnd invokes
 * a method of EmbedAsyncPanZoomController so that we can call protected methods of ASyncPanZoomController.
 * There could be a virtual method that would allow us to do the same thing.
 *
 * Regardless of above, this helps us to keep AsyncPanZoomZontroller clean from
 * embedlite specifc fixes.
 */
class EmbedAsyncPanZoomController : public mozilla::layers::AsyncPanZoomController
{
  public:
    EmbedAsyncPanZoomController(uint64_t aLayersId,
                                mozilla::layers::GeckoContentController* aGeckoContentController,
                                GestureBehavior aGestures)
      : AsyncPanZoomController(aLayersId, nullptr, aGeckoContentController, aGestures)
    {}
    void NotifyTransformEnd();
};

class EmbedContentController : public mozilla::layers::GeckoContentController
{
  typedef mozilla::layers::FrameMetrics FrameMetrics;
  typedef mozilla::layers::ScrollableLayerGuid ScrollableLayerGuid;

public:
    EmbedContentController(EmbedLiteViewThreadParent* aRenderFrame)
      : mUILoop(MessageLoop::current())
      , mRenderFrame(aRenderFrame)
      , mAsyncPanZoomController(0)
    {}

    virtual void RequestContentRepaint(const FrameMetrics& aFrameMetrics) MOZ_OVERRIDE;
    virtual void HandleDoubleTap(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
    virtual void HandleSingleTap(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
    virtual void HandleLongTap(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
    virtual void NotifyTransformEnd() MOZ_OVERRIDE;
    virtual void SendAsyncScrollDOMEvent(bool aIsRoot,
                                         const CSSRect& aContentRect,
                                         const CSSSize& aScrollableSize) MOZ_OVERRIDE;
    virtual void ScrollUpdate(const CSSPoint& aPosition, const float aResolution) MOZ_OVERRIDE;
    void ClearRenderFrame();
    virtual void PostDelayedTask(Task* aTask, int aDelayMs) MOZ_OVERRIDE;
    void SetAsyncPanZoomController(EmbedAsyncPanZoomController* aEmbedAsyncPanZoomController);

private:
    EmbedLiteViewListener* GetListener();
    void DoRequestContentRepaint(const FrameMetrics& aFrameMetrics);

    MessageLoop* mUILoop;
    EmbedLiteViewThreadParent* mRenderFrame;
    EmbedAsyncPanZoomController* mAsyncPanZoomController;
};

}}

#endif

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
namespace layers {
class APZCTreeManager;
class CompositorParent;
}
namespace embedlite {
class EmbedLiteViewListener;
class EmbedLiteViewThreadParent;

class EmbedContentController : public mozilla::layers::GeckoContentController
{
  typedef mozilla::layers::FrameMetrics FrameMetrics;
  typedef mozilla::layers::ScrollableLayerGuid ScrollableLayerGuid;

public:
  EmbedContentController(EmbedLiteViewThreadParent* aRenderFrame, mozilla::layers::CompositorParent* aCompositor, MessageLoop* aUILoop);

  // GeckoContentController interface
  virtual void RequestContentRepaint(const FrameMetrics& aFrameMetrics) MOZ_OVERRIDE;
  virtual void HandleDoubleTap(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
  virtual void HandleSingleTap(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
  virtual void HandleLongTap(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
  virtual void HandleLongTapUp(const CSSIntPoint& aPoint, int32_t aModifiers) MOZ_OVERRIDE;
  virtual void SendAsyncScrollDOMEvent(bool aIsRoot,
                                       const CSSRect& aContentRect,
                                       const CSSSize& aScrollableSize) MOZ_OVERRIDE;
  virtual void ScrollUpdate(const CSSPoint& aPosition, const float aResolution) MOZ_OVERRIDE;
  void ClearRenderFrame();
  virtual void PostDelayedTask(Task* aTask, int aDelayMs) MOZ_OVERRIDE;

  // EXTRA
  void UpdateScrollOffset(const mozilla::layers::ScrollableLayerGuid& aScrollLayerId, CSSIntPoint& aScrollOffset);

  bool HitTestAPZC(mozilla::ScreenIntPoint& aPoint);
  void TransformCoordinateToGecko(const mozilla::ScreenIntPoint& aPoint,
                                  LayoutDeviceIntPoint* aRefPointOut);
  void ContentReceivedTouch(const ScrollableLayerGuid& aGuid, bool aPreventDefault);
  nsEventStatus ReceiveInputEvent(const InputData& aEvent,
                                  ScrollableLayerGuid* aOutTargetGuid);

  mozilla::layers::APZCTreeManager* GetManager() { return mAPZC; }
private:
  EmbedLiteViewListener* const GetListener() const;
  void DoRequestContentRepaint(const FrameMetrics& aFrameMetrics);

  MessageLoop* mUILoop;
  EmbedLiteViewThreadParent* mRenderFrame;

  // Extra
  ScrollableLayerGuid mLastScrollLayerGuid;
  CSSIntPoint mLastScrollOffset;

public:
  // todo: make this a member variable as prep for multiple views
  nsRefPtr<mozilla::layers::APZCTreeManager> mAPZC;
};

}}

#endif

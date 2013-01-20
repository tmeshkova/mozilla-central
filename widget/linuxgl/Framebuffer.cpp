/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=8 et ft=cpp : */
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

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

#include "Framebuffer.h"
#include "gfxContext.h"
#include "gfxImageSurface.h"
#include "gfxUtils.h"
#include "mozilla/FileUtils.h"
#include "nsTArray.h"
#include "nsRegion.h"

#define LOG(FMT, ARG...) \
   printf("LinuxGL:%s:%s :%d: " FMT "\n", __FILE__, __FUNCTION__, __LINE__, ## ARG)

using namespace std;

namespace mozilla {

namespace Framebuffer {

static int sFd = -1;
static size_t sMappedSize;
static struct fb_var_screeninfo sVi;
static size_t sActiveBuffer;
typedef vector<nsRefPtr<gfxImageSurface> > BufferVector;
BufferVector* sBuffers;
static gfxIntSize *sScreenSize = nullptr;
nsRefPtr<gfxImageSurface> backBuffer;

BufferVector& Buffers() { return *sBuffers; }

bool
SetGraphicsMode()
{
    ScopedClose fd(open("/dev/tty0", O_RDWR | O_SYNC));
    if (0 > fd.get()) {
        // This is non-fatal; post-Cupcake kernels don't have tty0.
        LOG("No /dev/tty0?");
    } else if (ioctl(fd.get(), KDSETMODE, (void*) KD_GRAPHICS)) {
        LOG("Error setting graphics mode on /dev/tty0");
        return false;
    }
    return true;
}

bool
Open()
{
    if (0 <= sFd)
        return true;

    if (!SetGraphicsMode())
        return false;

    ScopedClose fd(open("/dev/fb0", O_RDWR));
    if (0 > fd.get()) {
        LOG("Error opening framebuffer device");
        return false;
    }

    struct fb_fix_screeninfo fi;
    if (0 > ioctl(fd.get(), FBIOGET_FSCREENINFO, &fi)) {
        LOG("Error getting fixed screeninfo");
        return false;
    }

    if (0 > ioctl(fd.get(), FBIOGET_VSCREENINFO, &sVi)) {
        LOG("Error getting variable screeninfo");
        return false;
    }

    sMappedSize = fi.smem_len;
    void* mem = mmap(0, sMappedSize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fd.rwget(), 0);
    if (MAP_FAILED == mem) {
        LOG("Error mmap'ing framebuffer");
        return false;
    }

    sFd = fd.get();
    fd.forget();

    // The android porting doc requires a /dev/fb0 device
    // that's double buffered with r5g6b5 format.  Hence the
    // hard-coded numbers here.
    gfxASurface::gfxImageFormat format = gfxASurface::ImageFormatRGB16_565;
    int bytesPerPixel = gfxASurface::BytePerPixelFromFormat(format);
    if (!sScreenSize) {
        sScreenSize = new gfxIntSize(sVi.xres, sVi.yres);
    }
    long stride = fi.line_length;
    size_t numFrameBytes = stride * sScreenSize->height;

    sBuffers = new BufferVector(2);
    unsigned char* data = static_cast<unsigned char*>(mem);
    size_t totalBytes = 0;
    for (size_t i = 0; i < 2; ++i, data += numFrameBytes) {
      totalBytes += numFrameBytes;
      if (totalBytes > sMappedSize) {
          printf("Framebuffer double buffering is not available\n");
          backBuffer = new gfxImageSurface(*sScreenSize, format);
          Buffers()[i] = Buffers()[i-1];
          continue;
      }
      memset(data, 0, numFrameBytes);
      Buffers()[i] = new gfxImageSurface(data, *sScreenSize, stride, format);
    }

    // Clear the framebuffer to a known state.
    Present(nsIntRect());

    return true;
}

int GetDepth()
{
    return sVi.bits_per_pixel;
}

bool
GetSize(nsIntSize *aScreenSize) {
    // If the framebuffer has been opened, we should always have the size.
    if (sScreenSize) {
        *aScreenSize = *sScreenSize;
        return true;
    }

    ScopedClose fd(open("/dev/fb0", O_RDWR));
    if (0 > fd.get()) {
        LOG("Error opening framebuffer device");
        return false;
    }

    if (0 > ioctl(fd.get(), FBIOGET_VSCREENINFO, &sVi)) {
        LOG("Error getting variable screeninfo");
        return false;
    }

    sScreenSize = new gfxIntSize(sVi.xres, sVi.yres);
    *aScreenSize = *sScreenSize;
    return true;
}

void
Close()
{
    if (0 > sFd)
        return;

    munmap(Buffers()[0]->Data(), sMappedSize);
    delete sBuffers;
    sBuffers = NULL;
    backBuffer = NULL;
    delete sScreenSize;
    sScreenSize = NULL;

    close(sFd);
    sFd = -1;
}

gfxASurface*
BackBuffer()
{
    if (backBuffer)
        return backBuffer.get();
    return Buffers()[!sActiveBuffer];
}

static gfxASurface*
FrontBuffer()
{
    return Buffers()[sActiveBuffer];
}

void
Present(const nsIntRegion& aUpdated)
{
    sActiveBuffer = !sActiveBuffer;

    sVi.yres_virtual = sVi.yres * 2;
    sVi.yoffset = sActiveBuffer * sVi.yres;
    sVi.bits_per_pixel = 16;
    if (ioctl(sFd, FBIOPUT_VSCREENINFO, &sVi) < 0) {
        LOG("Error presenting front buffer");
    }

    if (backBuffer) {
        nsRefPtr<gfxContext> ctx = new gfxContext(FrontBuffer());
        gfxUtils::PathFromRegion(ctx, aUpdated);
        ctx->Clip();
        ctx->SetSource(BackBuffer());
        ctx->SetOperator(gfxContext::OPERATOR_SOURCE);
        ctx->Paint(1.0);
        return;
    }

    nsRefPtr<gfxContext> ctx = new gfxContext(BackBuffer());
    gfxUtils::PathFromRegion(ctx, aUpdated);
    ctx->Clip();
    ctx->SetSource(FrontBuffer());
    ctx->SetOperator(gfxContext::OPERATOR_SOURCE);
    ctx->Paint(1.0);
}

} // namespace Framebuffer

} // namespace mozilla

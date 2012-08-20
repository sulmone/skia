/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphologyImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#endif

SkMorphologyImageFilter::SkMorphologyImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fRadius.fWidth = buffer.readInt();
    fRadius.fHeight = buffer.readInt();
}

SkMorphologyImageFilter::SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input)
    : INHERITED(input), fRadius(SkISize::Make(radiusX, radiusY)) {
}


void SkMorphologyImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fRadius.fWidth);
    buffer.writeInt(fRadius.fHeight);
}

static void erode(const SkPMColor* src, SkPMColor* dst,
                  int radius, int width, int height,
                  int srcStrideX, int srcStrideY,
                  int dstStrideX, int dstStrideY)
{
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            int minB = 255, minG = 255, minR = 255, minA = 255;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p);
                int g = SkGetPackedG32(*p);
                int r = SkGetPackedR32(*p);
                int a = SkGetPackedA32(*p);
                if (b < minB) minB = b;
                if (g < minG) minG = g;
                if (r < minR) minR = r;
                if (a < minA) minA = a;
            }
            *dptr = SkPackARGB32(minA, minR, minG, minB);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

static void erodeX(const SkBitmap& src, SkBitmap* dst, int radiusX)
{
    erode(src.getAddr32(0, 0), dst->getAddr32(0, 0),
          radiusX, src.width(), src.height(),
          1, src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels());
}

static void erodeY(const SkBitmap& src, SkBitmap* dst, int radiusY)
{
    erode(src.getAddr32(0, 0), dst->getAddr32(0, 0),
          radiusY, src.height(), src.width(),
          src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels(), 1);
}

static void dilate(const SkPMColor* src, SkPMColor* dst,
                   int radius, int width, int height,
                   int srcStrideX, int srcStrideY,
                   int dstStrideX, int dstStrideY)
{
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            int maxB = 0, maxG = 0, maxR = 0, maxA = 0;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p);
                int g = SkGetPackedG32(*p);
                int r = SkGetPackedR32(*p);
                int a = SkGetPackedA32(*p);
                if (b > maxB) maxB = b;
                if (g > maxG) maxG = g;
                if (r > maxR) maxR = r;
                if (a > maxA) maxA = a;
            }
            *dptr = SkPackARGB32(maxA, maxR, maxG, maxB);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

static void dilateX(const SkBitmap& src, SkBitmap* dst, int radiusX)
{
    dilate(src.getAddr32(0, 0), dst->getAddr32(0, 0),
           radiusX, src.width(), src.height(),
           1, src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels());
}

static void dilateY(const SkBitmap& src, SkBitmap* dst, int radiusY)
{
    dilate(src.getAddr32(0, 0), dst->getAddr32(0, 0),
           radiusY, src.height(), src.width(),
           src.rowBytesAsPixels(), 1, dst->rowBytesAsPixels(), 1);
}

bool SkErodeImageFilter::onFilterImage(Proxy* proxy,
                                       const SkBitmap& source, const SkMatrix& ctm,
                                       SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = this->getInputResult(proxy, source, ctm, offset);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();

    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.copyTo(dst, dst->config());
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (width > 0 && height > 0) {
        erodeX(src, &temp, width);
        erodeY(temp, dst, height);
    } else if (width > 0) {
        erodeX(src, dst, width);
    } else if (height > 0) {
        erodeY(src, dst, height);
    }
    return true;
}

bool SkDilateImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source, const SkMatrix& ctm,
                                        SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = this->getInputResult(proxy, source, ctm, offset);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), src.width(), src.height());
    dst->allocPixels();

    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.copyTo(dst, dst->config());
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (width > 0 && height > 0) {
        dilateX(src, &temp, width);
        dilateY(temp, dst, height);
    } else if (width > 0) {
        dilateX(src, dst, width);
    } else if (height > 0) {
        dilateY(src, dst, height);
    }
    return true;
}

GrTexture* SkDilateImageFilter::onFilterImageGPU(GrTexture* src, const SkRect& rect) {
#if SK_SUPPORT_GPU
    SkAutoTUnref<GrTexture> input(this->getInputResultAsTexture(src, rect));
    return src->getContext()->applyMorphology(input.get(), rect,
                                              GrContext::kDilate_MorphologyType,
                                              radius());
#else
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
#endif
}

GrTexture* SkErodeImageFilter::onFilterImageGPU(GrTexture* src, const SkRect& rect) {
#if SK_SUPPORT_GPU
    SkAutoTUnref<GrTexture> input(this->getInputResultAsTexture(src, rect));
    return src->getContext()->applyMorphology(input.get(), rect,
                                              GrContext::kErode_MorphologyType,
                                              radius());
#else
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
#endif
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkDilateImageFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR(SkErodeImageFilter)

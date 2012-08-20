/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureBenchmark_DEFINED
#define PictureBenchmark_DEFINED
#include "SkTypes.h"
#include "SkRefCnt.h"
#include "PictureRenderer.h"

class SkPicture;
class SkString;

namespace sk_tools {

class PictureBenchmark : public SkRefCnt {
public:
    virtual void run(SkPicture* pict) = 0;

    void setRepeats(int repeats) {
        fRepeats = repeats;
    }

    int getRepeats() const {
        return fRepeats;
    }

    void setUseBitmapDevice() {
      sk_tools::PictureRenderer* renderer = getRenderer();

        if (renderer != NULL) {
            renderer->setUseBitmapDevice();
        }
    }

#if SK_SUPPORT_GPU
    void setUseGpuDevice() {
      sk_tools::PictureRenderer* renderer = getRenderer();

        if (renderer != NULL) {
            renderer->setUseGpuDevice();
        }
    }
#endif

protected:
    int fRepeats;

private:
    typedef SkRefCnt INHERITED;

    virtual sk_tools::PictureRenderer* getRenderer() {
        return NULL;
    }
};

class PipePictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    PipePictureRenderer fRenderer;
    typedef PictureBenchmark INHERITED;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE {
        return &fRenderer;
    }
};

class RecordPictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    typedef PictureBenchmark INHERITED;
};

class SimplePictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    SimplePictureRenderer fRenderer;
    typedef PictureBenchmark INHERITED;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE {
        return &fRenderer;
    }
};

class TiledPictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;

    void setTileWidth(int width) {
        fRenderer.setTileWidth(width);
    }

    int getTileWidth() const {
        return fRenderer.getTileWidth();
    }

    void setTileHeight(int height) {
        fRenderer.setTileHeight(height);
    }

    int getTileHeight() const {
        return fRenderer.getTileHeight();
    }

    void setTileWidthPercentage(double percentage) {
        fRenderer.setTileWidthPercentage(percentage);
    }

    double getTileWidthPercentage() const {
        return fRenderer.getTileWidthPercentage();
    }

    void setTileHeightPercentage(double percentage) {
        fRenderer.setTileHeightPercentage(percentage);
    }

    double getTileHeightPercentage() const {
        return fRenderer.getTileHeightPercentage();
    }

private:
    TiledPictureRenderer fRenderer;
    typedef PictureBenchmark INHERITED;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE{
        return &fRenderer;
    }
};

class UnflattenPictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    typedef PictureBenchmark INHERITED;
};

}

#endif  // PictureBenchmark_DEFINED

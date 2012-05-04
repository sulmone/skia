
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkTDArray.h"

/*
 *  Cases to consider:
 *
 *  1. antialiasing on/off (esp. width <= 1)
 *  2. strokewidth == 0, 1, 2
 *  3. hline, vline, diagonal, rect, oval
 *  4. dots [1,1] ([N,N] where N=strokeWidth?) or arbitrary (e.g. [2,1] or [1,2,3,2])
 */
static void path_hline(SkPath* path) {
    path->moveTo(SkIntToScalar(10), SkIntToScalar(10));
    path->lineTo(SkIntToScalar(600), SkIntToScalar(10));
}

class DashBench : public SkBenchmark {
    SkTDArray<SkScalar> fIntervals;

    enum {
        N = SkBENCHLOOP(100)
    };
public:
    DashBench(void* param, const SkScalar intervals[], int count) : INHERITED(param) {
        fIntervals.append(count, intervals);
    }

    virtual void makePath(SkPath* path) {
        path_hline(path);
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "dash";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setStyle(SkPaint::kStroke_Style);
//        paint.setStrokeWidth(SK_Scalar1);
        paint.setAntiAlias(false);

        SkPath path;
        this->makePath(&path);

        paint.setPathEffect(new SkDashPathEffect(fIntervals.begin(),
                                                 fIntervals.count(), 0))->unref();
        for (int i = 0; i < N; ++i) {
            canvas->drawPath(path, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static const SkScalar gDots[] = { SK_Scalar1, SK_Scalar1 };

#define PARAM(array)    array, SK_ARRAY_COUNT(array)

static SkBenchmark* gF0(void* p) { return new DashBench(p, PARAM(gDots)); }

static BenchRegistry gR0(gF0);

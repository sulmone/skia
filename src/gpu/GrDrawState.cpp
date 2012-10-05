/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"

#include "GrPaint.h"

void GrDrawState::setFromPaint(const GrPaint& paint) {
    for (int i = 0; i < GrPaint::kMaxColorStages; ++i) {
        int s = i + GrPaint::kFirstColorStage;
        if (paint.isColorStageEnabled(i)) {
            *this->sampler(s) = paint.getColorSampler(i);
        }
    }

    this->setFirstCoverageStage(GrPaint::kFirstCoverageStage);

    for (int i = 0; i < GrPaint::kMaxCoverageStages; ++i) {
        int s = i + GrPaint::kFirstCoverageStage;
        if (paint.isCoverageStageEnabled(i)) {
            *this->sampler(s) = paint.getCoverageSampler(i);
        }
    }

    // disable all stages not accessible via the paint
    for (int s = GrPaint::kTotalStages; s < GrDrawState::kNumStages; ++s) {
        this->disableStage(s);
    }

    this->setColor(paint.fColor);

    this->setState(GrDrawState::kDither_StateBit, paint.fDither);
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.fAntiAlias);

    if (paint.fColorMatrixEnabled) {
        this->enableState(GrDrawState::kColorMatrix_StateBit);
        this->setColorMatrix(paint.fColorMatrix);
    } else {
        this->disableState(GrDrawState::kColorMatrix_StateBit);
    }
    this->setBlendFunc(paint.fSrcBlendCoeff, paint.fDstBlendCoeff);
    this->setColorFilter(paint.fColorFilterColor, paint.fColorFilterXfermode);
    this->setCoverage(paint.fCoverage);
}

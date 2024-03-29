
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugger.h"
#include "SkString.h"


SkDebugger::SkDebugger() {
    // Create this some other dynamic way?
    fDebugCanvas = new SkDebugCanvas(100, 100);
    fPicture = NULL;
    fPictureWidth = 0;
    fPictureHeight = 0;
    fIndex = 0;
}

SkDebugger::~SkDebugger() {
    // Need to inherit from SkRef object in order for following to work
    SkSafeUnref(fDebugCanvas);
    SkSafeUnref(fPicture);
}

void SkDebugger::loadPicture(SkPicture* picture) {
    fPictureWidth = picture->width();
    fPictureHeight = picture->height();
    delete fDebugCanvas;
    fDebugCanvas = new SkDebugCanvas(fPictureWidth, fPictureHeight);
    fDebugCanvas->setBounds(fPictureWidth, fPictureHeight);
    picture->draw(fDebugCanvas);
    fIndex = fDebugCanvas->getSize() - 1;
    SkRefCnt_SafeAssign(fPicture, picture);
}

SkPicture* SkDebugger::copyPicture() {
    // We can't just call clone here since we want to removed the "deleted"
    // commands. Playing back will strip those out.
    SkPicture* newPicture = new SkPicture;
    SkCanvas* canvas = newPicture->beginRecording(fPictureWidth, fPictureHeight);
    fDebugCanvas->draw(canvas);
    newPicture->endRecording();
    return newPicture;
}

void SkDebugger::getOverviewText(const SkTDArray<double>* typeTimes,
                                 double totTime,
                                 SkString* overview,
                                 int numRuns) {
    const SkTDArray<SkDrawCommand*>& commands = this->getDrawCommands();

    SkTDArray<int> counts;
    counts.setCount(LAST_DRAWTYPE_ENUM+1);
    for (int i = 0; i < LAST_DRAWTYPE_ENUM+1; ++i) {
        counts[i] = 0;
    }

    for (int i = 0; i < commands.count(); i++) {
        counts[commands[i]->getType()]++;
    }

    overview->reset();
    int total = 0;
#ifdef SK_DEBUG
    double totPercent = 0, tempSum = 0;
#endif
    for (int i = 0; i < LAST_DRAWTYPE_ENUM+1; ++i) {
        if (0 == counts[i]) {
            // if there were no commands of this type then they should've consumed no time
            SkASSERT(NULL == typeTimes || 0.0 == (*typeTimes)[i]);
            continue;
        }

        overview->append(SkDrawCommand::GetCommandString((DrawType) i));
        overview->append(": ");
        overview->appendS32(counts[i]);
        if (NULL != typeTimes) {
            overview->append(" - ");
            overview->appendf("%.2f", (*typeTimes)[i]/(float)numRuns);
            overview->append("ms");
            overview->append(" - ");
            double percent = 100.0*(*typeTimes)[i]/totTime;
            overview->appendf("%.2f", percent);
            overview->append("%");
#ifdef SK_DEBUG
            totPercent += percent;
            tempSum += (*typeTimes)[i];
#endif
        }
        overview->append("<br/>");
        total += counts[i];
    }
#ifdef SK_DEBUG
    if (NULL != typeTimes) {
        SkASSERT(SkScalarNearlyEqual(SkDoubleToScalar(totPercent),
                                     SkDoubleToScalar(100.0)));
        SkASSERT(SkScalarNearlyEqual(SkDoubleToScalar(tempSum),
                                     SkDoubleToScalar(totTime)));
    }
#endif

    if (totTime > 0.0) {
        overview->append("Total Time: ");
        overview->appendf("%.2f", totTime/(float)numRuns);
        overview->append("ms");
#ifdef SK_DEBUG
        overview->append(" ");
        overview->appendScalar(SkDoubleToScalar(totPercent));
        overview->append("% ");
#endif
        overview->append("<br/>");
    }

    SkString totalStr;
    totalStr.append("Total Draw Commands: ");
    totalStr.appendScalar(SkDoubleToScalar(total));
    totalStr.append("<br/>");
    overview->insert(0, totalStr);

    overview->append("<br/>");
    overview->append("SkPicture Width: ");
    overview->appendS32(pictureWidth());
    overview->append("px<br/>");
    overview->append("SkPicture Height: ");
    overview->appendS32(pictureHeight());
    overview->append("px");
}

#include "SkImageDecoder.h"

void forceLinking();
void forceLinking() {
    // This function leaks, but that is okay because it is not intended
    // to be called. It is only here so that the linker will include the
    // decoders.
    SkDEBUGCODE(SkImageDecoder *creator = ) CreateJPEGImageDecoder();
    SkASSERT(creator);
    SkDEBUGCODE(creator = ) CreateWEBPImageDecoder();
    SkASSERT(creator);
#if defined(SK_BUILD_FOR_UNIX) && !defined(SK_BUILD_FOR_NACL)
    SkDEBUGCODE(creator = ) CreateGIFImageDecoder();
    SkASSERT(creator);
#endif
}

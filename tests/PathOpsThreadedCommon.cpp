/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

PathOpsThreadedTestRunner::~PathOpsThreadedTestRunner() {
    for (int index = 0; index < fRunnables.count(); index++) {
        SkDELETE(fRunnables[index]);
    }
}

void PathOpsThreadedTestRunner::render() {
    SkThreadPool pool(fNumThreads);
    for (int index = 0; index < fRunnables.count(); ++ index) {
        pool.add(fRunnables[index]);
    }
#ifdef SK_DEBUG
    gDebugMaxWindSum = SK_MaxS32;
    gDebugMaxWindValue = SK_MaxS32;
#endif
}

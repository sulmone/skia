/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "SkIntersections.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct lineQuad {
    SkDQuad quad;
    SkDLine line;
    int result;
    SkDPoint expected[2];
} lineQuadTests[] = {
    //        quad                    line                  results
    {{{{1, 1}, {2, 1}, {0, 2}}}, {{{0, 0}, {1, 1}}},  1,  {{1, 1}, {0, 0}} },
    {{{{0, 0}, {1, 1}, {3, 1}}}, {{{0, 0}, {3, 1}}},  2,  {{0, 0}, {3, 1}} },
    {{{{2, 0}, {1, 1}, {2, 2}}}, {{{0, 0}, {0, 2}}},  0,  {{0, 0}, {0, 0}} },
    {{{{4, 0}, {0, 1}, {4, 2}}}, {{{3, 1}, {4, 1}}},  0,  {{0, 0}, {0, 0}} },
    {{{{0, 0}, {0, 1}, {1, 1}}}, {{{0, 1}, {1, 0}}},  1,  {{.25, .75}, {0, 0}} },
};

static size_t lineQuadTests_count = SK_ARRAY_COUNT(lineQuadTests);

static int doIntersect(SkIntersections& intersections, const SkDQuad& quad, const SkDLine& line,
                       bool& flipped) {
    int result;
    flipped = false;
    if (line[0].fX == line[1].fX) {
        double top = line[0].fY;
        double bottom = line[1].fY;
        flipped = top > bottom;
        if (flipped) {
            SkTSwap<double>(top, bottom);
        }
        result = intersections.vertical(quad, top, bottom, line[0].fX, flipped);
    } else if (line[0].fY == line[1].fY) {
        double left = line[0].fX;
        double right = line[1].fX;
        flipped = left > right;
        if (flipped) {
            SkTSwap<double>(left, right);
        }
        result = intersections.horizontal(quad, left, right, line[0].fY, flipped);
    } else {
        intersections.intersect(quad, line);
        result = intersections.used();
    }
    return result;
}

static struct oneLineQuad {
    SkDQuad quad;
    SkDLine line;
} oneOffs[] = {
    {{{{369.848602, 145.680267}, {382.360413, 121.298294}, {406.207703, 121.298294}}},
        {{{406.207703, 121.298294}, {348.781738, 123.864815}}}}
    };

static size_t oneOffs_count = SK_ARRAY_COUNT(oneOffs);

static void testOneOffs(skiatest::Reporter* reporter) {
    SkIntersections intersections;
    bool flipped = false;
    for (size_t index = 0; index < oneOffs_count; ++index) {
        const SkDQuad& quad = oneOffs[index].quad;
        const SkDLine& line = oneOffs[index].line;
        int result = doIntersect(intersections, quad, line, flipped);
        for (int inner = 0; inner < result; ++inner) {
            double quadT = intersections[0][inner];
            SkDPoint quadXY = quad.xyAtT(quadT);
            double lineT = intersections[1][inner];
            SkDPoint lineXY = line.xyAtT(lineT);
            REPORTER_ASSERT(reporter, quadXY.approximatelyEqual(lineXY));
        }
    }
}

static void PathOpsQuadLineIntersectionTest(skiatest::Reporter* reporter) {
    testOneOffs(reporter);
    for (size_t index = 0; index < lineQuadTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const SkDQuad& quad = lineQuadTests[index].quad;
        const SkDLine& line = lineQuadTests[index].line;
        SkReduceOrder reducer1, reducer2;
        int order1 = reducer1.reduce(quad, SkReduceOrder::kFill_Style);
        int order2 = reducer2.reduce(line);
        if (order1 < 3) {
            SkDebugf("%s [%d] quad order=%d\n", __FUNCTION__, iIndex, order1);
            REPORTER_ASSERT(reporter, 0);
        }
        if (order2 < 2) {
            SkDebugf("%s [%d] line order=%d\n", __FUNCTION__, iIndex, order2);
            REPORTER_ASSERT(reporter, 0);
        }
        SkIntersections intersections;
        bool flipped = false;
        int result = doIntersect(intersections, quad, line, flipped);
        REPORTER_ASSERT(reporter, result == lineQuadTests[index].result);
        if (intersections.used() <= 0) {
            continue;
        }
        for (int pt = 0; pt < result; ++pt) {
            double tt1 = intersections[0][pt];
            REPORTER_ASSERT(reporter, tt1 >= 0 && tt1 <= 1);
            SkDPoint t1 = quad.xyAtT(tt1);
            double tt2 = intersections[1][pt];
            REPORTER_ASSERT(reporter, tt2 >= 0 && tt2 <= 1);
            SkDPoint t2 = line.xyAtT(tt2);
            if (!t1.approximatelyEqual(t2)) {
                SkDebugf("%s [%d,%d] x!= t1=%1.9g (%1.9g,%1.9g) t2=%1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, iIndex, pt, tt1, t1.fX, t1.fY, tt2, t2.fX, t2.fY);
                REPORTER_ASSERT(reporter, 0);
            }
            if (!t1.approximatelyEqual(lineQuadTests[index].expected[0])
                    && (lineQuadTests[index].result == 1
                    || !t1.approximatelyEqual(lineQuadTests[index].expected[1]))) {
                SkDebugf("%s t1=(%1.9g,%1.9g)\n", __FUNCTION__, t1.fX, t1.fY);
                REPORTER_ASSERT(reporter, 0);
            }
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsQuadLineIntersectionTest)

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsCubicIntersectionTestData.h"
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsRect.h"
#include "SkReduceOrder.h"
#include "Test.h"

const int firstCubicIntersectionTest = 9;

static void standardTestCases(skiatest::Reporter* reporter) {
    for (size_t index = firstCubicIntersectionTest; index < tests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const SkDCubic& cubic1 = tests[index][0];
        const SkDCubic& cubic2 = tests[index][1];
        SkReduceOrder reduce1, reduce2;
        int order1 = reduce1.reduce(cubic1, SkReduceOrder::kNo_Quadratics,
            SkReduceOrder::kFill_Style);
        int order2 = reduce2.reduce(cubic2, SkReduceOrder::kNo_Quadratics,
            SkReduceOrder::kFill_Style);
        const bool showSkipped = false;
        if (order1 < 4) {
            if (showSkipped) {
                SkDebugf("%s [%d] cubic1 order=%d\n", __FUNCTION__, iIndex, order1);
            }
            continue;
        }
        if (order2 < 4) {
            if (showSkipped) {
                SkDebugf("%s [%d] cubic2 order=%d\n", __FUNCTION__, iIndex, order2);
            }
            continue;
        }
        SkIntersections tIntersections;
        tIntersections.intersect(cubic1, cubic2);
        if (!tIntersections.used()) {
            if (showSkipped) {
                SkDebugf("%s [%d] no intersection\n", __FUNCTION__, iIndex);
            }
            continue;
        }
        if (tIntersections.isCoincident(0)) {
            if (showSkipped) {
                SkDebugf("%s [%d] coincident\n", __FUNCTION__, iIndex);
            }
            continue;
        }
        for (int pt = 0; pt < tIntersections.used(); ++pt) {
            double tt1 = tIntersections[0][pt];
            SkDPoint xy1 = cubic1.xyAtT(tt1);
            double tt2 = tIntersections[1][pt];
            SkDPoint xy2 = cubic2.xyAtT(tt2);
            if (!xy1.approximatelyEqual(xy2)) {
                SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                    __FUNCTION__, (int)index, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
            }
            REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
        }
    }
}

static const SkDCubic testSet[] = {
// FIXME: uncommenting these two will cause this to fail
// this results in two curves very nearly but not exactly coincident
#if 0
{{{67.426548091427676, 37.993772624988935}, {23.483695892376684, 90.476863174921306},
      {35.597065061143162, 79.872482633158796}, {75.38634169631932, 18.244890038969412}}},
{{{67.4265481, 37.9937726}, {23.4836959, 90.4768632}, {35.5970651, 79.8724826},
      {75.3863417, 18.24489}}},
#endif

{{{0, 0}, {0, 1}, {1, 1}, {1, 0}}},
{{{1, 0}, {0, 0}, {0, 1}, {1, 1}}},

{{{0, 1}, {4, 5}, {1, 0}, {5, 3}}},
{{{0, 1}, {3, 5}, {1, 0}, {5, 4}}},

{{{0, 1}, {1, 6}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {6, 1}}},

{{{0, 1}, {3, 4}, {1, 0}, {5, 1}}},
{{{0, 1}, {1, 5}, {1, 0}, {4, 3}}},

{{{0, 1}, {1, 2}, {1, 0}, {6, 1}}},
{{{0, 1}, {1, 6}, {1, 0}, {2, 1}}},

{{{0, 1}, {0, 5}, {1, 0}, {4, 0}}},
{{{0, 1}, {0, 4}, {1, 0}, {5, 0}}},

{{{0, 1}, {3, 4}, {1, 0}, {3, 0}}},
{{{0, 1}, {0, 3}, {1, 0}, {4, 3}}},

{{{0, 0}, {1, 2}, {3, 4}, {4, 4}}},
{{{0, 0}, {1, 2}, {3, 4}, {4, 4}}},
{{{4, 4}, {3, 4}, {1, 2}, {0, 0}}},

{{{0, 1}, {2, 3}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {3, 2}}},

{{{0, 2}, {0, 1}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {2, 0}, {1, 0}}},

{{{0, 1}, {0, 2}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {2, 0}}},

{{{0, 1}, {1, 6}, {1, 0}, {2, 0}}},
{{{0, 1}, {0, 2}, {1, 0}, {6, 1}}},

{{{0, 1}, {5, 6}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {6, 5}}},

{{{95.837747722788592, 45.025976907939643}, {16.564570095652982, 0.72959763963222402},
        {63.209855865319199, 68.047528419665767}, {57.640240647662544, 59.524565264361243}}},
{{{51.593891741518817, 38.53849970667553}, {62.34752929878772, 74.924924725166022},
        {74.810149322641152, 34.17966562983564}, {29.368398119401373, 94.66719277886078}}},

{{{39.765160968417838, 33.060396198677083}, {5.1922921581157908, 66.854301452103215},
        {31.619281802149157, 25.269248720849514}, {81.541621071073038, 70.025341524754353}}},
{{{46.078911165743556, 48.259962651999651}, {20.24450549867214, 49.403916182650214},
        {0.26325131778756683, 24.46489805563581}, {15.915006546264051, 83.515023059917155}}},

{{{65.454505973241524, 93.881892270353575}, {45.867360264932437, 92.723972719499827},
        {2.1464054482739447, 74.636369140183717}, {33.774068594804994, 40.770872887582925}}},
{{{72.963387832494163, 95.659300729473728}, {11.809496633619768, 82.209921247423594},
        {13.456139067865974, 57.329313623406605}, {36.060621606214262, 70.867335643091849}}},

{{{32.484981432782945, 75.082940782924624}, {42.467313093350882, 48.131159948246157},
        {3.5963115764764657, 43.208665839959245}, {79.442476890721579, 89.709102357602262}}},
{{{18.98573861410177, 93.308887208490106}, {40.405250173250792, 91.039661826118675},
        {8.0467721950480584, 42.100282172719147}, {40.883324221187891, 26.030185504830527}}},

{{{7.5374809128872498, 82.441702896003477}, {22.444346930107265, 22.138854312775123},
        {66.76091829629658, 50.753805856571446}, {78.193478508942519, 97.7932997968948}}},
{{{97.700573130371311, 53.53260215070685}, {87.72443481149358, 84.575876772671876},
        {19.215031396232092, 47.032676472809484}, {11.989686410869325, 10.659507480757082}}},

{{{26.192053931854691, 9.8504326817814416}, {10.174241480498686, 98.476562741434464},
        {21.177712558385782, 33.814968789841501}, {75.329030899018534, 55.02231980442177}}},
{{{56.222082700683771, 24.54395039218662}, {95.589995289030483, 81.050822735322086},
        {28.180450866082897, 28.837706255185282}, {60.128952916771617, 87.311672180570511}}},

{{{42.449716172390481, 52.379709366885805}, {27.896043159019225, 48.797373636065686},
        {92.770268299044233, 89.899302036454571}, {12.102066544863426, 99.43241951960718}}},
{{{45.77532924980639, 45.958701495993274}, {37.458701356062065, 68.393691335056758},
        {37.569326692060258, 27.673713456687381}, {60.674866037757539, 62.47349659096146}}},

{{{67.426548091427676, 37.993772624988935}, {23.483695892376684, 90.476863174921306},
        {35.597065061143162, 79.872482633158796}, {75.38634169631932, 18.244890038969412}}},
{{{61.336508189019057, 82.693132843213675}, {44.639380902349664, 54.074825790745592},
        {16.815615499771951, 20.049704667203923}, {41.866884958868326, 56.735503699973002}}},

{{{18.1312339, 31.6473732}, {95.5711034, 63.5350219}, {92.3283165, 62.0158945},
        {18.5656052, 32.1268808}}},
{{{97.402018, 35.7169972}, {33.1127443, 25.8935163}, {1.13970027, 54.9424981},
        {56.4860195, 60.529264}}},
};

const size_t testSetCount = SK_ARRAY_COUNT(testSet);

static const SkDCubic newTestSet[] = {
{{{3, 5}, {1, 6}, {5, 0}, {3, 1}}},
{{{0, 5}, {1, 3}, {5, 3}, {6, 1}}},

{{{0, 1}, {1, 5}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {1, 0}, {5, 1}}},

{{{1, 3}, {5, 6}, {5, 3}, {5, 4}}},
{{{3, 5}, {4, 5}, {3, 1}, {6, 5}}},

{{{0, 5}, {0, 5}, {5, 4}, {6, 4}}},
{{{4, 5}, {4, 6}, {5, 0}, {5, 0}}},

{{{0, 4}, {1, 3}, {5, 4}, {4, 2}}},
{{{4, 5}, {2, 4}, {4, 0}, {3, 1}}},

{{{0, 2}, {1, 5}, {3, 2}, {4, 1}}},
{{{2, 3}, {1, 4}, {2, 0}, {5, 1}}},

{{{0, 2}, {2, 3}, {5, 1}, {3, 2}}},
{{{1, 5}, {2, 3}, {2, 0}, {3, 2}}},

{{{2, 6}, {4, 5}, {1, 0}, {6, 1}}},
{{{0, 1}, {1, 6}, {6, 2}, {5, 4}}},

{{{0, 1}, {1, 2}, {6, 5}, {5, 4}}},
{{{5, 6}, {4, 5}, {1, 0}, {2, 1}}},

{{{2.5119999999999996, 1.5710000000000002}, {2.6399999999999983, 1.6599999999999997},
        {2.8000000000000007, 1.8000000000000003}, {3, 2}}},
{{{2.4181876227114887, 1.9849772580462195}, {2.8269904869227211, 2.009330650246834},
        {3.2004679292461624, 1.9942047174679169}, {3.4986199496818058, 2.0035994597094731}}},

{{{2, 3}, {1, 4}, {1, 0}, {6, 0}}},
{{{0, 1}, {0, 6}, {3, 2}, {4, 1}}},

{{{0, 2}, {1, 5}, {1, 0}, {6, 1}}},
{{{0, 1}, {1, 6}, {2, 0}, {5, 1}}},

{{{0, 1}, {1, 5}, {2, 1}, {4, 0}}},
{{{1, 2}, {0, 4}, {1, 0}, {5, 1}}},

{{{0, 1}, {3, 5}, {2, 1}, {3, 1}}},
{{{1, 2}, {1, 3}, {1, 0}, {5, 3}}},

{{{0, 1}, {2, 5}, {6, 0}, {5, 3}}},
{{{0, 6}, {3, 5}, {1, 0}, {5, 2}}},

{{{0, 1}, {3, 6}, {1, 0}, {5, 2}}},
{{{0, 1}, {2, 5}, {1, 0}, {6, 3}}},

{{{1, 2}, {5, 6}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {2, 1}, {6, 5}}},

{{{0, 6}, {1, 2}, {1, 0}, {1, 0}}},
{{{0, 1}, {0, 1}, {6, 0}, {2, 1}}},

{{{0, 2}, {0, 1}, {3, 0}, {1, 0}}},
{{{0, 3}, {0, 1}, {2, 0}, {1, 0}}},
};

const size_t newTestSetCount = SK_ARRAY_COUNT(newTestSet);

static void oneOff(skiatest::Reporter* reporter, const SkDCubic& cubic1, const SkDCubic& cubic2) {
#if ONE_OFF_DEBUG
    SkDebugf("computed quadratics given\n");
    SkDebugf("  {{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}},\n",
        cubic1[0].fX, cubic1[0].fY, cubic1[1].fX, cubic1[1].fY,
        cubic1[2].fX, cubic1[2].fY, cubic1[3].fX, cubic1[3].fY);
    SkDebugf("  {{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}},\n",
        cubic2[0].fX, cubic2[0].fY, cubic2[1].fX, cubic2[1].fY,
        cubic2[2].fX, cubic2[2].fY, cubic2[3].fX, cubic2[3].fY);
#endif
    SkTDArray<SkDQuad> quads1;
    CubicToQuads(cubic1, cubic1.calcPrecision(), quads1);
#if ONE_OFF_DEBUG
    SkDebugf("computed quadratics set 1\n");
    for (int index = 0; index < quads1.count(); ++index) {
        const SkDQuad& q = quads1[index];
        SkDebugf("  {{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}},\n", q[0].fX, q[0].fY,
                 q[1].fX, q[1].fY,  q[2].fX, q[2].fY);
    }
#endif
    SkTDArray<SkDQuad> quads2;
    CubicToQuads(cubic2, cubic2.calcPrecision(), quads2);
#if ONE_OFF_DEBUG
    SkDebugf("computed quadratics set 2\n");
    for (int index = 0; index < quads2.count(); ++index) {
        const SkDQuad& q = quads2[index];
        SkDebugf("  {{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}},\n", q[0].fX, q[0].fY,
                 q[1].fX, q[1].fY,  q[2].fX, q[2].fY);
    }
#endif
    SkIntersections intersections;
    intersections.intersect(cubic1, cubic2);
    double tt1, tt2;
    SkDPoint xy1, xy2;
    for (int pt3 = 0; pt3 < intersections.used(); ++pt3) {
        tt1 = intersections[0][pt3];
        xy1 = cubic1.xyAtT(tt1);
        tt2 = intersections[1][pt3];
        xy2 = cubic2.xyAtT(tt2);
#if ONE_OFF_DEBUG
        SkDebugf("%s t1=%1.9g (%1.9g, %1.9g) (%1.9g, %1.9g) (%1.9g, %1.9g) t2=%1.9g\n",
                __FUNCTION__, tt1, xy1.fX, xy1.fY, intersections.pt(pt3).fX,
                intersections.pt(pt3).fY, xy2.fX, xy2.fY, tt2);
#endif
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
}

static void oneOff(skiatest::Reporter* reporter, int outer, int inner) {
    const SkDCubic& cubic1 = testSet[outer];
    const SkDCubic& cubic2 = testSet[inner];
    oneOff(reporter, cubic1, cubic2);
}

static void newOneOff(skiatest::Reporter* reporter, int outer, int inner) {
    const SkDCubic& cubic1 = newTestSet[outer];
    const SkDCubic& cubic2 = newTestSet[inner];
    oneOff(reporter, cubic1, cubic2);
}

static void oneOffTest(skiatest::Reporter* reporter) {
    oneOff(reporter, 14, 16);
    newOneOff(reporter, 0, 1);
}

static void oneOffTests(skiatest::Reporter* reporter) {
    for (size_t outer = 0; outer < testSetCount - 1; ++outer) {
        for (size_t inner = outer + 1; inner < testSetCount; ++inner) {
            oneOff(reporter, outer, inner);
        }
    }
    for (size_t outer = 0; outer < newTestSetCount - 1; ++outer) {
        for (size_t inner = outer + 1; inner < newTestSetCount; ++inner) {
            oneOff(reporter, outer, inner);
        }
    }
}

#define DEBUG_CRASH 0

static void CubicIntersection_RandTest(skiatest::Reporter* reporter) {
    srand(0);
    const int tests = 10000000;
#if !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)
    unsigned seed = 0;
#endif
    for (int test = 0; test < tests; ++test) {
        SkDCubic cubic1, cubic2;
        for (int i = 0; i < 4; ++i) {
            cubic1[i].fX = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
            cubic1[i].fY = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
            cubic2[i].fX = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
            cubic2[i].fY = static_cast<double>(SK_RAND(seed)) / RAND_MAX * 100;
        }
    #if DEBUG_CRASH
        char str[1024];
        snprintf(str, sizeof(str),
            "{{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}}},\n"
            "{{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}}},\n",
                cubic1[0].fX, cubic1[0].fY,  cubic1[1].fX, cubic1[1].fY, cubic1[2].fX, cubic1[2].fY,
                cubic1[3].fX, cubic1[3].fY,
                cubic2[0].fX, cubic2[0].fY,  cubic2[1].fX, cubic2[1].fY, cubic2[2].fX, cubic2[2].fY,
                cubic2[3].fX, cubic2[3].fY);
    #endif
        SkDRect rect1, rect2;
        rect1.setBounds(cubic1);
        rect2.setBounds(cubic2);
        bool boundsIntersect = rect1.fLeft <= rect2.fRight && rect2.fLeft <= rect2.fRight
                && rect1.fTop <= rect2.fBottom && rect2.fTop <= rect1.fBottom;
        if (test == -1) {
            SkDebugf("ready...\n");
        }
        SkIntersections intersections2;
        int newIntersects = intersections2.intersect(cubic1, cubic2);
        if (!boundsIntersect && newIntersects) {
    #if DEBUG_CRASH
            SkDebugf("%s %d unexpected intersection boundsIntersect=%d "
                    " newIntersects=%d\n%s %s\n", __FUNCTION__, test, boundsIntersect,
                    newIntersects, __FUNCTION__, str);
    #endif
            REPORTER_ASSERT(reporter, 0);
        }
        for (int pt = 0; pt < intersections2.used(); ++pt) {
            double tt1 = intersections2[0][pt];
            SkDPoint xy1 = cubic1.xyAtT(tt1);
            double tt2 = intersections2[1][pt];
            SkDPoint xy2 = cubic2.xyAtT(tt2);
            REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
        }
    }
}

static void intersectionFinder(int index0, int index1, double t1Seed, double t2Seed,
        double t1Step, double t2Step) {
    const SkDCubic& cubic1 = newTestSet[index0];
    const SkDCubic& cubic2 = newTestSet[index1];
    SkDPoint t1[3], t2[3];
    bool toggle = true;
    do {
        t1[0] = cubic1.xyAtT(t1Seed - t1Step);
        t1[1] = cubic1.xyAtT(t1Seed);
        t1[2] = cubic1.xyAtT(t1Seed + t1Step);
        t2[0] = cubic2.xyAtT(t2Seed - t2Step);
        t2[1] = cubic2.xyAtT(t2Seed);
        t2[2] = cubic2.xyAtT(t2Seed + t2Step);
        double dist[3][3];
        dist[1][1] = t1[1].distance(t2[1]);
        int best_i = 1, best_j = 1;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (i == 1 && j == 1) {
                    continue;
                }
                dist[i][j] = t1[i].distance(t2[j]);
                if (dist[best_i][best_j] > dist[i][j]) {
                    best_i = i;
                    best_j = j;
                }
            }
        }
        if (best_i == 0) {
            t1Seed -= t1Step;
        } else if (best_i == 2) {
            t1Seed += t1Step;
        }
        if (best_j == 0) {
            t2Seed -= t2Step;
        } else if (best_j == 2) {
            t2Seed += t2Step;
        }
        if (best_i == 1 && best_j == 1) {
            if ((toggle ^= true)) {
                t1Step /= 2;
            } else {
                t2Step /= 2;
            }
        }
    } while (!t1[1].approximatelyEqual(t2[1]));
    t1Step = t2Step = 0.1;
    double t10 = t1Seed - t1Step * 2;
    double t12 = t1Seed + t1Step * 2;
    double t20 = t2Seed - t2Step * 2;
    double t22 = t2Seed + t2Step * 2;
    SkDPoint test;
    while (!approximately_zero(t1Step)) {
        test = cubic1.xyAtT(t10);
        t10 += t1[1].approximatelyEqual(test) ? -t1Step : t1Step;
        t1Step /= 2;
    }
    t1Step = 0.1;
    while (!approximately_zero(t1Step)) {
        test = cubic1.xyAtT(t12);
        t12 -= t1[1].approximatelyEqual(test) ? -t1Step : t1Step;
        t1Step /= 2;
    }
    while (!approximately_zero(t2Step)) {
        test = cubic2.xyAtT(t20);
        t20 += t2[1].approximatelyEqual(test) ? -t2Step : t2Step;
        t2Step /= 2;
    }
    t2Step = 0.1;
    while (!approximately_zero(t2Step)) {
        test = cubic2.xyAtT(t22);
        t22 -= t2[1].approximatelyEqual(test) ? -t2Step : t2Step;
        t2Step /= 2;
    }
#if ONE_OFF_DEBUG
    SkDebugf("%s t1=(%1.9g<%1.9g<%1.9g) t2=(%1.9g<%1.9g<%1.9g)\n", __FUNCTION__,
        t10, t1Seed, t12, t20, t2Seed, t22);
    SkDPoint p10 = cubic1.xyAtT(t10);
    SkDPoint p1Seed = cubic1.xyAtT(t1Seed);
    SkDPoint p12 = cubic1.xyAtT(t12);
    SkDebugf("%s p1=(%1.9g,%1.9g)<(%1.9g,%1.9g)<(%1.9g,%1.9g)\n", __FUNCTION__,
        p10.fX, p10.fY, p1Seed.fX, p1Seed.fY, p12.fX, p12.fY);
    SkDPoint p20 = cubic2.xyAtT(t20);
    SkDPoint p2Seed = cubic2.xyAtT(t2Seed);
    SkDPoint p22 = cubic2.xyAtT(t22);
    SkDebugf("%s p2=(%1.9g,%1.9g)<(%1.9g,%1.9g)<(%1.9g,%1.9g)\n", __FUNCTION__,
        p20.fX, p20.fY, p2Seed.fX, p2Seed.fY, p22.fX, p22.fY);
#endif
}

static void CubicIntersection_IntersectionFinder() {
//   double t1Seed = 0.87;
//   double t2Seed = 0.87;
    double t1Step = 0.000001;
    double t2Step = 0.000001;
    intersectionFinder(0, 1, 0.855895664, 0.864850875, t1Step, t2Step);
    intersectionFinder(0, 1, 0.865207906, 0.865207887, t1Step, t2Step);
    intersectionFinder(0, 1, 0.865213351, 0.865208087, t1Step, t2Step);
}

static void cubicIntersectionSelfTest(skiatest::Reporter* reporter) {
    const SkDCubic selfSet[] = {
        {{{0, 2}, {2, 3}, {5, 1}, {3, 2}}},
        {{{0, 2}, {3, 5}, {5, 0}, {4, 2}}},
        {{{3.34, 8.98}, {1.95, 10.27}, {3.76, 7.65}, {4.96, 10.64}}},
        {{{3.13, 2.74}, {1.08, 4.62}, {3.71, 0.94}, {2.01, 3.81}}},
        {{{6.71, 3.14}, {7.99, 2.75}, {8.27, 1.96}, {6.35, 3.57}}},
        {{{12.81, 7.27}, {7.22, 6.98}, {12.49, 8.97}, {11.42, 6.18}}},
    };
    size_t selfSetCount = SK_ARRAY_COUNT(selfSet);
    size_t firstFail = 1;
    for (size_t index = firstFail; index < selfSetCount; ++index) {
        const SkDCubic& cubic = selfSet[index];
    #if ONE_OFF_DEBUG
        int idx2;
        double max[3];
        int ts = cubic.findMaxCurvature(max);
        for (idx2 = 0; idx2 < ts; ++idx2) {
            SkDebugf("%s max[%d]=%1.9g (%1.9g, %1.9g)\n", __FUNCTION__, idx2,
                    max[idx2], cubic.xyAtT(max[idx2]).fX, cubic.xyAtT(max[idx2]).fY);
        }
        SkTDArray<double> ts1;
        SkTDArray<SkDQuad> quads1;
        cubic.toQuadraticTs(cubic.calcPrecision(), &ts1);
        for (idx2 = 0; idx2 < ts1.count(); ++idx2) {
            SkDebugf("%s t[%d]=%1.9g\n", __FUNCTION__, idx2, ts1[idx2]);
        }
        CubicToQuads(cubic, cubic.calcPrecision(), quads1);
        for (idx2 = 0; idx2 < quads1.count(); ++idx2) {
            const SkDQuad& q = quads1[idx2];
            SkDebugf("  {{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}},\n",
                    q[0].fX, q[0].fY,  q[1].fX, q[1].fY,  q[2].fX, q[2].fY);
        }
        SkDebugf("\n");
    #endif
        SkIntersections i;
        int result = i.intersect(cubic);
        REPORTER_ASSERT(reporter, result == 1);
        REPORTER_ASSERT(reporter, i.used() == 1);
        REPORTER_ASSERT(reporter, !approximately_equal(i[0][0], i[1][0]));
        SkDPoint pt1 = cubic.xyAtT(i[0][0]);
        SkDPoint pt2 = cubic.xyAtT(i[1][0]);
        REPORTER_ASSERT(reporter, pt1.approximatelyEqual(pt2));
    }
}

static void PathOpsCubicIntersectionTest(skiatest::Reporter* reporter) {
    oneOffTest(reporter);
    oneOffTests(reporter);
    cubicIntersectionSelfTest(reporter);
    standardTestCases(reporter);
    if (false) CubicIntersection_IntersectionFinder();
    if (false) CubicIntersection_RandTest(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsCubicIntersectionTest)

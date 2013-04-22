/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOps_DEFINED
#define SkPathOps_DEFINED

class SkPath;

// FIXME: move everything below into the SkPath class
/**
  *  The logical operations that can be performed when combining two paths.
  */
enum SkPathOp {
    kDifference_PathOp,         //!< subtract the op path from the first path
    kIntersect_PathOp,          //!< intersect the two paths
    kUnion_PathOp,              //!< union (inclusive-or) the two paths
    kXOR_PathOp,                //!< exclusive-or the two paths
    kReverseDifference_PathOp,  //!< subtract the first path from the op path
};

/**
  *  Set this path to the result of applying the Op to this path and the
  *  specified path: this = (this op operand). The resulting path will be constructed
  *  from non-overlapping contours. The curve order is reduced where possible so that cubics may
  *  be turned into quadratics, and quadratics maybe turned into lines.
  */
void Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result);

/**
  *  Set this path to a set of non-overlapping contours that describe the same
  *  area as the original path. The curve order is reduced where possible so that cubics may
  *  be turned into quadratics, and quadratics maybe turned into lines.
  */
void Simplify(const SkPath& path, SkPath* result);

#endif


/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPictureRecord.h"
#include "SkTSearch.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkBBoxHierarchy.h"
#include "SkPictureStateTree.h"

#define MIN_WRITER_SIZE 16384
#define HEAP_BLOCK_SIZE 4096

enum {
    // just need a value that save or getSaveCount would never return
    kNoInitialSave = -1,
};

// A lot of basic types get stored as a uint32_t: bools, ints, paint indices, etc.
static int const kUInt32Size = 4;

SkPictureRecord::SkPictureRecord(uint32_t flags, SkDevice* device) :
        INHERITED(device),
        fBoundingHierarchy(NULL),
        fStateTree(NULL),
        fFlattenableHeap(HEAP_BLOCK_SIZE),
        fMatrices(&fFlattenableHeap),
        fPaints(&fFlattenableHeap),
        fRegions(&fFlattenableHeap),
        fWriter(MIN_WRITER_SIZE),
        fRecordFlags(flags) {
#ifdef SK_DEBUG_SIZE
    fPointBytes = fRectBytes = fTextBytes = 0;
    fPointWrites = fRectWrites = fTextWrites = 0;
#endif

    fRestoreOffsetStack.setReserve(32);

    fBitmapHeap = SkNEW(SkBitmapHeap);
    fFlattenableHeap.setBitmapStorage(fBitmapHeap);
    fPathHeap = NULL;   // lazy allocate
    fFirstSavedLayerIndex = kNoSavedLayerIndex;

    fInitialSaveCount = kNoInitialSave;
}

SkPictureRecord::~SkPictureRecord() {
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fPathHeap);
    SkSafeUnref(fBoundingHierarchy);
    SkSafeUnref(fStateTree);
    fFlattenableHeap.setBitmapStorage(NULL);
    fPictureRefs.unrefAll();
}

///////////////////////////////////////////////////////////////////////////////

SkDevice* SkPictureRecord::setDevice(SkDevice* device) {
    SkASSERT(!"eeek, don't try to change the device on a recording canvas");
    return this->INHERITED::setDevice(device);
}

int SkPictureRecord::save(SaveFlags flags) {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.size());

    // op + flags
    uint32_t size = 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(SAVE, &size);
    addInt(flags);

    validate(initialOffset, size);
    return this->INHERITED::save(flags);
}

int SkPictureRecord::saveLayer(const SkRect* bounds, const SkPaint* paint,
                               SaveFlags flags) {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.size());

    // op + bool for 'bounds'
    uint32_t size = 2 * kUInt32Size; 
    if (NULL != bounds) {
        size += sizeof(*bounds); // + rect
    }
    // + paint index + flags
    size += 2 * kUInt32Size;

    uint32_t initialOffset = this->addDraw(SAVE_LAYER, &size);
    addRectPtr(bounds);
    addPaintPtr(paint);
    addInt(flags);

    if (kNoSavedLayerIndex == fFirstSavedLayerIndex) {
        fFirstSavedLayerIndex = fRestoreOffsetStack.count();
    }

    validate(initialOffset, size);
    /*  Don't actually call saveLayer, because that will try to allocate an
        offscreen device (potentially very big) which we don't actually need
        at this time (and may not be able to afford since during record our
        clip starts out the size of the picture, which is often much larger
        than the size of the actual device we'll use during playback).
     */
    int count = this->INHERITED::save(flags);
    this->clipRectBounds(bounds, flags, NULL);
    return count;
}

bool SkPictureRecord::isDrawingToLayer() const {
    return fFirstSavedLayerIndex != kNoSavedLayerIndex;
}

/*
 * Read the op code from 'offset' in 'writer' and extract the size too.
 */
static DrawType peek_op_and_size(SkWriter32* writer, int32_t offset, uint32_t* size) {
    uint32_t* peek = writer->peek32(offset);

    uint32_t op;
    UNPACK_8_24(*peek, op, *size);
    if (MASK_24 == *size) {
        // size required its own slot right after the op code
        *size = *writer->peek32(offset+kUInt32Size);    
    }
    return (DrawType) op;
}

#ifdef TRACK_COLLAPSE_STATS
    static int gCollapseCount, gCollapseCalls;
#endif

/*
 *  Restore has just been called (but not recoreded), so look back at the
 *  matching save(), and see if we can eliminate the pair of them, due to no
 *  intervening matrix/clip calls.
 *
 *  If so, update the writer and return true, in which case we won't even record
 *  the restore() call. If we still need the restore(), return false.
 */
static bool collapseSaveClipRestore(SkWriter32* writer, int32_t offset) {
#ifdef TRACK_COLLAPSE_STATS
    gCollapseCalls += 1;
#endif

    int32_t restoreOffset = (int32_t)writer->size();

    // back up to the save block
    while (offset > 0) {
        offset = *writer->peek32(offset);
    }

    // now offset points to a save
    offset = -offset;
    uint32_t opSize;
    DrawType op = peek_op_and_size(writer, offset, &opSize);
    if (SAVE_LAYER == op) {
        // not ready to cull these out yet (mrr)
        return false;
    }
    SkASSERT(SAVE == op);

    // Walk forward until we get back to either a draw-verb (abort) or we hit
    // our restore (success).
    int32_t saveOffset = offset;

    offset += opSize;
    while (offset < restoreOffset) {
        op = peek_op_and_size(writer, offset, &opSize);
        if ((op > CONCAT && op < ROTATE) || (SAVE_LAYER == op)) {
            // drawing verb, abort
            return false;
        }
        offset += opSize;
    }

#ifdef TRACK_COLLAPSE_STATS
    gCollapseCount += 1;
    SkDebugf("Collapse [%d out of %d] %g%spn", gCollapseCount, gCollapseCalls,
             (double)gCollapseCount / gCollapseCalls, "%");
#endif

    writer->rewindToOffset(saveOffset);
    return true;
}

void SkPictureRecord::restore() {
    // FIXME: SkDeferredCanvas needs to be refactored to respect
    // save/restore balancing so that the following test can be
    // turned on permanently.
#if 0
    SkASSERT(fRestoreOffsetStack.count() > 1);
#endif

    // check for underflow
    if (fRestoreOffsetStack.count() == 0) {
        return;
    }

    if (fRestoreOffsetStack.count() == fFirstSavedLayerIndex) {
        fFirstSavedLayerIndex = kNoSavedLayerIndex;
    }

    uint32_t initialOffset, size;
    if (!collapseSaveClipRestore(&fWriter, fRestoreOffsetStack.top())) {
        fillRestoreOffsetPlaceholdersForCurrentStackLevel((uint32_t)fWriter.size());
        // op
        size = 1 * kUInt32Size;
        initialOffset = this->addDraw(RESTORE, &size);
    } else {
        size = 0;
        initialOffset = fWriter.size();
    }

    fRestoreOffsetStack.pop();

    validate(initialOffset, size);
    return this->INHERITED::restore();
}

bool SkPictureRecord::translate(SkScalar dx, SkScalar dy) {
    // op + dx + dy
    uint32_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    uint32_t initialOffset = this->addDraw(TRANSLATE, &size);
    addScalar(dx);
    addScalar(dy);
    validate(initialOffset, size);
    return this->INHERITED::translate(dx, dy);
}

bool SkPictureRecord::scale(SkScalar sx, SkScalar sy) {
    // op + sx + sy
    uint32_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    uint32_t initialOffset = this->addDraw(SCALE, &size);
    addScalar(sx);
    addScalar(sy);
    validate(initialOffset, size);
    return this->INHERITED::scale(sx, sy);
}

bool SkPictureRecord::rotate(SkScalar degrees) {
    // op + degrees
    uint32_t size = 1 * kUInt32Size + sizeof(SkScalar);
    uint32_t initialOffset = this->addDraw(ROTATE, &size);
    addScalar(degrees);
    validate(initialOffset, size);
    return this->INHERITED::rotate(degrees);
}

bool SkPictureRecord::skew(SkScalar sx, SkScalar sy) {
    // op + sx + sy
    uint32_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    uint32_t initialOffset = this->addDraw(SKEW, &size);
    addScalar(sx);
    addScalar(sy);
    validate(initialOffset, size);
    return this->INHERITED::skew(sx, sy);
}

bool SkPictureRecord::concat(const SkMatrix& matrix) {
    validate(fWriter.size(), 0);
    // op + matrix index
    uint32_t size = 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(CONCAT, &size);
    addMatrix(matrix);
    validate(initialOffset, size);
    return this->INHERITED::concat(matrix);
}

void SkPictureRecord::setMatrix(const SkMatrix& matrix) {
    validate(fWriter.size(), 0);
    // op + matrix index
    uint32_t size = 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(SET_MATRIX, &size);
    addMatrix(matrix);
    validate(initialOffset, size);
    this->INHERITED::setMatrix(matrix);
}

static bool regionOpExpands(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kUnion_Op:
        case SkRegion::kXOR_Op:
        case SkRegion::kReverseDifference_Op:
        case SkRegion::kReplace_Op:
            return true;
        case SkRegion::kIntersect_Op:
        case SkRegion::kDifference_Op:
            return false;
        default:
            SkDEBUGFAIL("unknown region op");
            return false;
    }
}

void SkPictureRecord::fillRestoreOffsetPlaceholdersForCurrentStackLevel(
    uint32_t restoreOffset) {
    int32_t offset = fRestoreOffsetStack.top();
    while (offset > 0) {
        uint32_t* peek = fWriter.peek32(offset);
        offset = *peek;
        *peek = restoreOffset;
    }

#ifdef SK_DEBUG
    // assert that the final offset value points to a save verb
    uint32_t opSize;
    DrawType drawOp = peek_op_and_size(&fWriter, -offset, &opSize);
    SkASSERT(SAVE == drawOp || SAVE_LAYER == drawOp);
#endif
}

void SkPictureRecord::beginRecording() {
    // we have to call this *after* our constructor, to ensure that it gets
    // recorded. This is balanced by restoreToCount() call from endRecording,
    // which in-turn calls our overridden restore(), so those get recorded too.
    fInitialSaveCount = this->save(kMatrixClip_SaveFlag);
}

void SkPictureRecord::endRecording() {
    SkASSERT(kNoInitialSave != fInitialSaveCount);
    this->restoreToCount(fInitialSaveCount);
}

void SkPictureRecord::recordRestoreOffsetPlaceholder(SkRegion::Op op) {
    if (fRestoreOffsetStack.isEmpty()) {
        return;
    }

    if (regionOpExpands(op)) {
        // Run back through any previous clip ops, and mark their offset to
        // be 0, disabling their ability to trigger a jump-to-restore, otherwise
        // they could hide this clips ability to expand the clip (i.e. go from
        // empty to non-empty).
        fillRestoreOffsetPlaceholdersForCurrentStackLevel(0);
    }

    size_t offset = fWriter.size();
    // The RestoreOffset field is initially filled with a placeholder
    // value that points to the offset of the previous RestoreOffset
    // in the current stack level, thus forming a linked list so that
    // the restore offsets can be filled in when the corresponding
    // restore command is recorded.
    addInt(fRestoreOffsetStack.top());
    fRestoreOffsetStack.top() = offset;
}

bool SkPictureRecord::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    // id + rect + clip params + restore offset
    uint32_t size = 1 * kUInt32Size + sizeof(rect) + 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(CLIP_RECT, &size);
    addRect(rect);
    addInt(ClipParams_pack(op, doAA));
    recordRestoreOffsetPlaceholder(op);

    validate(initialOffset, size);
    return this->INHERITED::clipRect(rect, op, doAA);
}

bool SkPictureRecord::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    if (rrect.isRect()) {
        return this->SkPictureRecord::clipRect(rrect.getBounds(), op, doAA);
    }

    // op + rrect + clip params + restore offset
    uint32_t size = 1 * kUInt32Size + SkRRect::kSizeInMemory + 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(CLIP_RRECT, &size);
    addRRect(rrect);
    addInt(ClipParams_pack(op, doAA));
    recordRestoreOffsetPlaceholder(op);

    validate(initialOffset, size);

    if (fRecordFlags & SkPicture::kUsePathBoundsForClip_RecordingFlag) {
        return this->INHERITED::clipRect(rrect.getBounds(), op, doAA);
    } else {
        return this->INHERITED::clipRRect(rrect, op, doAA);
    }
}

bool SkPictureRecord::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {

    SkRect r;
    if (!path.isInverseFillType() && path.isRect(&r)) {
        return this->clipRect(r, op, doAA);
    }

    // op + path index + clip params + restore offset
    uint32_t size = 4 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(CLIP_PATH, &size);
    addPath(path);
    addInt(ClipParams_pack(op, doAA));
    recordRestoreOffsetPlaceholder(op);

    validate(initialOffset, size);

    if (fRecordFlags & SkPicture::kUsePathBoundsForClip_RecordingFlag) {
        return this->INHERITED::clipRect(path.getBounds(), op, doAA);
    } else {
        return this->INHERITED::clipPath(path, op, doAA);
    }
}

bool SkPictureRecord::clipRegion(const SkRegion& region, SkRegion::Op op) {
    // op + region index + clip params + restore offset
    uint32_t size = 4 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(CLIP_REGION, &size); 
    addRegion(region);
    addInt(ClipParams_pack(op, false));
    recordRestoreOffsetPlaceholder(op);

    validate(initialOffset, size);
    return this->INHERITED::clipRegion(region, op);
}

void SkPictureRecord::clear(SkColor color) {
    // op + color
    uint32_t size = 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_CLEAR, &size);
    addInt(color);
    validate(initialOffset, size);
}

void SkPictureRecord::drawPaint(const SkPaint& paint) {
    // op + paint index
    uint32_t size = 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_PAINT, &size);
    addPaint(paint);
    validate(initialOffset, size);
}

void SkPictureRecord::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                 const SkPaint& paint) {
    // op + paint index + mode + count + point data
    uint32_t size = 4 * kUInt32Size + count * sizeof(SkPoint);
    uint32_t initialOffset = this->addDraw(DRAW_POINTS, &size);
    addPaint(paint);
    addInt(mode);
    addInt(count);
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
    validate(initialOffset, size);
}

void SkPictureRecord::drawOval(const SkRect& oval, const SkPaint& paint) {
    // op + paint index + rect
    uint32_t size = 2 * kUInt32Size + sizeof(oval);
    uint32_t initialOffset = this->addDraw(DRAW_OVAL, &size);
    addPaint(paint);
    addRect(oval);
    validate(initialOffset, size);
}

void SkPictureRecord::drawRect(const SkRect& rect, const SkPaint& paint) {
    // op + paint index + rect
    uint32_t size = 2 * kUInt32Size + sizeof(rect);
    uint32_t initialOffset = this->addDraw(DRAW_RECT, &size);
    addPaint(paint);
    addRect(rect);
    validate(initialOffset, size);
}

void SkPictureRecord::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    uint32_t initialOffset, size;
    if (rrect.isRect()) {
        // op + paint index + rect
        size = 2 * kUInt32Size + sizeof(SkRect);
        initialOffset = this->addDraw(DRAW_RECT, &size);
        addPaint(paint);
        addRect(rrect.getBounds());
    } else if (rrect.isOval()) {
        // op + paint index + rect
        size = 2 * kUInt32Size + sizeof(SkRect);
        initialOffset = this->addDraw(DRAW_OVAL, &size);
        addPaint(paint);
        addRect(rrect.getBounds());
    } else {
        // op + paint index + rrect
        size = 2 * kUInt32Size + SkRRect::kSizeInMemory;
        initialOffset = this->addDraw(DRAW_RRECT, &size);
        addPaint(paint);
        addRRect(rrect);
    }
    validate(initialOffset, size);
}

void SkPictureRecord::drawPath(const SkPath& path, const SkPaint& paint) {
    // op + paint index + path index
    uint32_t size = 3 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_PATH, &size);
    addPaint(paint);
    addPath(path);
    validate(initialOffset, size);
}

void SkPictureRecord::drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                        const SkPaint* paint = NULL) {
    // op + paint index + bitmap index + left + top
    uint32_t size = 3 * kUInt32Size + 2 * sizeof(SkScalar);
    uint32_t initialOffset = this->addDraw(DRAW_BITMAP, &size);
    addPaintPtr(paint);
    addBitmap(bitmap);
    addScalar(left);
    addScalar(top);
    validate(initialOffset, size);
}

void SkPictureRecord::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                            const SkRect& dst, const SkPaint* paint) {
    // id + paint index + bitmap index + bool for 'src'
    uint32_t size = 4 * kUInt32Size; 
    if (NULL != src) {
        size += sizeof(*src);   // + rect
    }
    size += sizeof(dst);        // + rect

    uint32_t initialOffset = this->addDraw(DRAW_BITMAP_RECT_TO_RECT, &size);
    addPaintPtr(paint);
    addBitmap(bitmap);
    addRectPtr(src);  // may be null
    addRect(dst);
    validate(initialOffset, size);
}

void SkPictureRecord::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                                       const SkPaint* paint) {
    // id + paint index + bitmap index + matrix index
    uint32_t size = 4 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_BITMAP_MATRIX, &size);
    addPaintPtr(paint);
    addBitmap(bitmap);
    addMatrix(matrix);
    validate(initialOffset, size);
}

void SkPictureRecord::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                     const SkRect& dst, const SkPaint* paint) {
    // op + paint index + bitmap id + center + dst rect
    uint32_t size = 3 * kUInt32Size + sizeof(center) + sizeof(dst); 
    uint32_t initialOffset = this->addDraw(DRAW_BITMAP_NINE, &size);
    addPaintPtr(paint);
    addBitmap(bitmap);
    addIRect(center);
    addRect(dst);
    validate(initialOffset, size);
}

void SkPictureRecord::drawSprite(const SkBitmap& bitmap, int left, int top,
                        const SkPaint* paint = NULL) {
    // op + paint index + bitmap index + left + top
    uint32_t size = 5 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_SPRITE, &size);
    addPaintPtr(paint);
    addBitmap(bitmap);
    addInt(left);
    addInt(top);
    validate(initialOffset, size);
}

// Return fontmetrics.fTop,fBottom in topbot[0,1], after they have been
// tweaked by paint.computeFastBounds().
//
static void computeFontMetricsTopBottom(const SkPaint& paint, SkScalar topbot[2]) {
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);
    SkRect bounds;
    // construct a rect so we can see any adjustments from the paint.
    // we use 0,1 for left,right, just so the rect isn't empty
    bounds.set(0, metrics.fTop, SK_Scalar1, metrics.fBottom);
    (void)paint.computeFastBounds(bounds, &bounds);
    topbot[0] = bounds.fTop;
    topbot[1] = bounds.fBottom;
}

void SkPictureRecord::addFontMetricsTopBottom(const SkPaint& paint, const SkFlatData& flat,
                                              SkScalar minY, SkScalar maxY) {
    if (!flat.isTopBotWritten()) {
        computeFontMetricsTopBottom(paint, flat.writableTopBot());
        SkASSERT(flat.isTopBotWritten());
    }
    addScalar(flat.topBot()[0] + minY);
    addScalar(flat.topBot()[1] + maxY);
}

void SkPictureRecord::drawText(const void* text, size_t byteLength, SkScalar x,
                      SkScalar y, const SkPaint& paint) {
    bool fast = !paint.isVerticalText() && paint.canComputeFastBounds();

    // op + paint index + length + 'length' worth of chars + x + y
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 2 * sizeof(SkScalar);
    if (fast) {
        size += 2 * sizeof(SkScalar); // + top & bottom
    }

    uint32_t initialOffset = this->addDraw(fast ? DRAW_TEXT_TOP_BOTTOM : DRAW_TEXT, &size);
    const SkFlatData* flatPaintData = addPaint(paint);
    SkASSERT(flatPaintData);
    addText(text, byteLength);
    addScalar(x);
    addScalar(y);
    if (fast) {
        addFontMetricsTopBottom(paint, *flatPaintData, y, y);
    }
    validate(initialOffset, size);
}

void SkPictureRecord::drawPosText(const void* text, size_t byteLength,
                         const SkPoint pos[], const SkPaint& paint) {
    size_t points = paint.countText(text, byteLength);
    if (0 == points)
        return;

    bool canUseDrawH = true;
    SkScalar minY = pos[0].fY;
    SkScalar maxY = pos[0].fY;
    // check if the caller really should have used drawPosTextH()
    {
        const SkScalar firstY = pos[0].fY;
        for (size_t index = 1; index < points; index++) {
            if (pos[index].fY != firstY) {
                canUseDrawH = false;
                if (pos[index].fY < minY) {
                    minY = pos[index].fY;
                } else if (pos[index].fY > maxY) {
                    maxY = pos[index].fY;
                }
            }
        }
    }

    bool fastBounds = !paint.isVerticalText() && paint.canComputeFastBounds();
    bool fast = canUseDrawH && fastBounds;

    // op + paint index + length + 'length' worth of data + num points
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
    if (canUseDrawH) {
        if (fast) {
            size += 2 * sizeof(SkScalar); // + top & bottom
        }
        // + y-pos + actual x-point data
        size += sizeof(SkScalar) + points * sizeof(SkScalar);
    } else {
        // + x&y point data
        size += points * sizeof(SkPoint);
        if (fastBounds) {
            size += 2 * sizeof(SkScalar); // + top & bottom
        }
    }

    DrawType op;
    if (fast) {
        op = DRAW_POS_TEXT_H_TOP_BOTTOM;
    } else if (canUseDrawH) {
        op = DRAW_POS_TEXT_H;
    } else if (fastBounds) {
        op = DRAW_POS_TEXT_TOP_BOTTOM;
    } else {
        op = DRAW_POS_TEXT;
    }
    uint32_t initialOffset = this->addDraw(op, &size);
    const SkFlatData* flatPaintData = addPaint(paint);
    SkASSERT(flatPaintData);
    addText(text, byteLength);
    addInt(points);

#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.size();
#endif
    if (canUseDrawH) {
        if (fast) {
            addFontMetricsTopBottom(paint, *flatPaintData, pos[0].fY, pos[0].fY);
        }
        addScalar(pos[0].fY);
        SkScalar* xptr = (SkScalar*)fWriter.reserve(points * sizeof(SkScalar));
        for (size_t index = 0; index < points; index++)
            *xptr++ = pos[index].fX;
    } else {
        fWriter.writeMul4(pos, points * sizeof(SkPoint));
        if (fastBounds) {
            addFontMetricsTopBottom(paint, *flatPaintData, minY, maxY);
        }
    }
#ifdef SK_DEBUG_SIZE
    fPointBytes += fWriter.size() - start;
    fPointWrites += points;
#endif
    validate(initialOffset, size);
}

void SkPictureRecord::drawPosTextH(const void* text, size_t byteLength,
                          const SkScalar xpos[], SkScalar constY,
                          const SkPaint& paint) {
    size_t points = paint.countText(text, byteLength);
    if (0 == points)
        return;

    bool fast = !paint.isVerticalText() && paint.canComputeFastBounds();

    // op + paint index + length + 'length' worth of data + num points
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
    if (fast) {
        size += 2 * sizeof(SkScalar); // + top & bottom
    }
    // + y + the actual points
    size += 1 * kUInt32Size + points * sizeof(SkScalar);

    uint32_t initialOffset = this->addDraw(fast ? DRAW_POS_TEXT_H_TOP_BOTTOM : DRAW_POS_TEXT_H, 
                                           &size);
    const SkFlatData* flatPaintData = addPaint(paint);
    SkASSERT(flatPaintData);
    addText(text, byteLength);
    addInt(points);

#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.size();
#endif
    if (fast) {
        addFontMetricsTopBottom(paint, *flatPaintData, constY, constY);
    }
    addScalar(constY);
    fWriter.writeMul4(xpos, points * sizeof(SkScalar));
#ifdef SK_DEBUG_SIZE
    fPointBytes += fWriter.size() - start;
    fPointWrites += points;
#endif
    validate(initialOffset, size);
}

void SkPictureRecord::drawTextOnPath(const void* text, size_t byteLength,
                            const SkPath& path, const SkMatrix* matrix,
                            const SkPaint& paint) {
    // op + paint index + length + 'length' worth of data + path index + matrix index
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_TEXT_ON_PATH, &size);
    addPaint(paint);
    addText(text, byteLength);
    addPath(path);
    addMatrixPtr(matrix);
    validate(initialOffset, size);
}

void SkPictureRecord::drawPicture(SkPicture& picture) {
    // op + picture index
    uint32_t size = 2 * kUInt32Size;
    uint32_t initialOffset = this->addDraw(DRAW_PICTURE, &size);
    addPicture(picture);
    validate(initialOffset, size);
}

void SkPictureRecord::drawVertices(VertexMode vmode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode*,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint) {
    uint32_t flags = 0;
    if (texs) {
        flags |= DRAW_VERTICES_HAS_TEXS;
    }
    if (colors) {
        flags |= DRAW_VERTICES_HAS_COLORS;
    }
    if (indexCount > 0) {
        flags |= DRAW_VERTICES_HAS_INDICES;
    }

    // op + paint index + flags + vmode + vCount + vertices
    uint32_t size = 5 * kUInt32Size + vertexCount * sizeof(SkPoint);
    if (flags & DRAW_VERTICES_HAS_TEXS) {
        size += vertexCount * sizeof(SkPoint);  // + uvs
    }
    if (flags & DRAW_VERTICES_HAS_COLORS) {
        size += vertexCount * sizeof(SkColor);  // + vert colors
    }
    if (flags & DRAW_VERTICES_HAS_INDICES) {
        // + num indices + indices
        size += 1 * kUInt32Size + SkAlign4(indexCount * sizeof(uint16_t));
    }

    uint32_t initialOffset = this->addDraw(DRAW_VERTICES, &size);
    addPaint(paint);
    addInt(flags);
    addInt(vmode);
    addInt(vertexCount);
    addPoints(vertices, vertexCount);
    if (flags & DRAW_VERTICES_HAS_TEXS) {
        addPoints(texs, vertexCount);
    }
    if (flags & DRAW_VERTICES_HAS_COLORS) {
        fWriter.writeMul4(colors, vertexCount * sizeof(SkColor));
    }
    if (flags & DRAW_VERTICES_HAS_INDICES) {
        addInt(indexCount);
        fWriter.writePad(indices, indexCount * sizeof(uint16_t));
    }
    validate(initialOffset, size);
}

void SkPictureRecord::drawData(const void* data, size_t length) {
    // op + length + 'length' worth of data
    uint32_t size = 2 * kUInt32Size + SkAlign4(length);
    uint32_t initialOffset = this->addDraw(DRAW_DATA, &size); 
    addInt(length);
    fWriter.writePad(data, length);
    validate(initialOffset, size);
}

///////////////////////////////////////////////////////////////////////////////

void SkPictureRecord::addBitmap(const SkBitmap& bitmap) {
    const int index = fBitmapHeap->insert(bitmap);
    // In debug builds, a bad return value from insert() will crash, allowing for debugging. In
    // release builds, the invalid value will be recorded so that the reader will know that there
    // was a problem.
    SkASSERT(index != SkBitmapHeap::INVALID_SLOT);
    addInt(index);
}

void SkPictureRecord::addMatrix(const SkMatrix& matrix) {
    addMatrixPtr(&matrix);
}

void SkPictureRecord::addMatrixPtr(const SkMatrix* matrix) {
    this->addInt(matrix ? fMatrices.find(*matrix) : 0);
}

const SkFlatData* SkPictureRecord::addPaintPtr(const SkPaint* paint) {
    const SkFlatData* data = paint ? fPaints.findAndReturnFlat(*paint) : NULL;
    int index = data ? data->index() : 0;
    this->addInt(index);
    return data;
}

void SkPictureRecord::addPath(const SkPath& path) {
    if (NULL == fPathHeap) {
        fPathHeap = SkNEW(SkPathHeap);
    }
    addInt(fPathHeap->append(path));
}

void SkPictureRecord::addPicture(SkPicture& picture) {
    int index = fPictureRefs.find(&picture);
    if (index < 0) {    // not found
        index = fPictureRefs.count();
        *fPictureRefs.append() = &picture;
        picture.ref();
    }
    // follow the convention of recording a 1-based index
    addInt(index + 1);
}

void SkPictureRecord::addPoint(const SkPoint& point) {
#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.size();
#endif
    fWriter.writePoint(point);
#ifdef SK_DEBUG_SIZE
    fPointBytes += fWriter.size() - start;
    fPointWrites++;
#endif
}

void SkPictureRecord::addPoints(const SkPoint pts[], int count) {
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
#ifdef SK_DEBUG_SIZE
    fPointBytes += count * sizeof(SkPoint);
    fPointWrites++;
#endif
}

void SkPictureRecord::addRect(const SkRect& rect) {
#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.size();
#endif
    fWriter.writeRect(rect);
#ifdef SK_DEBUG_SIZE
    fRectBytes += fWriter.size() - start;
    fRectWrites++;
#endif
}

void SkPictureRecord::addRectPtr(const SkRect* rect) {
    if (fWriter.writeBool(rect != NULL)) {
        fWriter.writeRect(*rect);
    }
}

void SkPictureRecord::addIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(rect));
}

void SkPictureRecord::addIRectPtr(const SkIRect* rect) {
    if (fWriter.writeBool(rect != NULL)) {
        *(SkIRect*)fWriter.reserve(sizeof(SkIRect)) = *rect;
    }
}

void SkPictureRecord::addRRect(const SkRRect& rrect) {
    fWriter.writeRRect(rrect);
}

void SkPictureRecord::addRegion(const SkRegion& region) {
    addInt(fRegions.find(region));
}

void SkPictureRecord::addText(const void* text, size_t byteLength) {
#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.size();
#endif
    addInt(byteLength);
    fWriter.writePad(text, byteLength);
#ifdef SK_DEBUG_SIZE
    fTextBytes += fWriter.size() - start;
    fTextWrites++;
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG_SIZE
size_t SkPictureRecord::size() const {
    size_t result = 0;
    size_t sizeData;
    bitmaps(&sizeData);
    result += sizeData;
    matrices(&sizeData);
    result += sizeData;
    paints(&sizeData);
    result += sizeData;
    paths(&sizeData);
    result += sizeData;
    pictures(&sizeData);
    result += sizeData;
    regions(&sizeData);
    result += sizeData;
    result += streamlen();
    return result;
}

int SkPictureRecord::bitmaps(size_t* size) const {
    size_t result = 0;
    int count = fBitmaps.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fBitmaps[index]) + fBitmaps[index]->size();
    *size = result;
    return count;
}

int SkPictureRecord::matrices(size_t* size) const {
    int count = fMatrices.count();
    *size = sizeof(fMatrices[0]) * count;
    return count;
}

int SkPictureRecord::paints(size_t* size) const {
    size_t result = 0;
    int count = fPaints.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fPaints[index]) + fPaints[index]->size();
    *size = result;
    return count;
}

int SkPictureRecord::paths(size_t* size) const {
    size_t result = 0;
    int count = fPaths.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fPaths[index]) + fPaths[index]->size();
    *size = result;
    return count;
}

int SkPictureRecord::regions(size_t* size) const {
    size_t result = 0;
    int count = fRegions.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fRegions[index]) + fRegions[index]->size();
    *size = result;
    return count;
}

size_t SkPictureRecord::streamlen() const {
    return fWriter.size();
}
#endif

#ifdef SK_DEBUG_VALIDATE
void SkPictureRecord::validate(uint32_t initialOffset, uint32_t size) const {
    SkASSERT(fWriter.size() == initialOffset + size);

    validateBitmaps();
    validateMatrices();
    validatePaints();
    validatePaths();
    validateRegions();
}

void SkPictureRecord::validateBitmaps() const {
    int count = fBitmapHeap->count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkBitmap* bitPtr = fBitmapHeap->getBitmap(index);
        SkASSERT(bitPtr);
        bitPtr->validate();
    }
}

void SkPictureRecord::validateMatrices() const {
    int count = fMatrices.count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkFlatData* matrix = fMatrices[index];
        SkASSERT(matrix);
//        matrix->validate();
    }
}

void SkPictureRecord::validatePaints() const {
    int count = fPaints.count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkFlatData* paint = fPaints[index];
        SkASSERT(paint);
//            paint->validate();
    }
}

void SkPictureRecord::validatePaths() const {
    if (NULL == fPathHeap) {
        return;
    }

    int count = fPathHeap->count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkPath& path = (*fPathHeap)[index];
        path.validate();
    }
}

void SkPictureRecord::validateRegions() const {
    int count = fRegions.count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkFlatData* region = fRegions[index];
        SkASSERT(region);
//        region->validate();
    }
}
#endif

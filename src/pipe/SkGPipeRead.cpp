/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkGPipePriv.h"
#include "SkReader32.h"

class SkGPipeState {
public:
    SkGPipeState();
    ~SkGPipeState();
    
    const SkPaint& getPaint(uint32_t drawOp32) const;
    
    //  Extracts index from DrawOp_unpackData().
    //  Returns the specified paint from our list, or creates a new paint if
    //  index == count. If index > count, return NULL
    SkPaint* editPaint(uint32_t drawOp32);
    
private:
    SkTDArray<SkPaint*> fPaints;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T> const T* skip(SkReader32* reader, int count = 1) {
    SkASSERT(count >= 0);
    size_t size = sizeof(T) * count;
    SkASSERT(SkAlign4(size) == size);
    return reinterpret_cast<const T*>(reader->skip(size));
}

template <typename T> const T* skipAlign(SkReader32* reader, int count = 1) {
    SkASSERT(count >= 0);
    size_t size = SkAlign4(sizeof(T) * count);
    return reinterpret_cast<const T*>(reader->skip(size));
}

static void readRegion(SkReader32* reader, SkRegion* rgn) {
    size_t size = rgn->unflatten(reader->peek());
    SkASSERT(SkAlign4(size) == size);
    (void)reader->skip(size);
}

static void readMatrix(SkReader32* reader, SkMatrix* matrix) {
    size_t size = matrix->unflatten(reader->peek());
    SkASSERT(SkAlign4(size) == size);
    (void)reader->skip(size);
}

const SkPaint& SkGPipeState::getPaint(uint32_t op32) const {
    unsigned index = DrawOp_unpackData(op32);
    if (index >= fPaints.count()) {
        SkASSERT(!"paint index out of range");
        index = 0;  // we always have at least 1 paint
    }
    return *fPaints[index];
}

SkPaint* SkGPipeState::editPaint(uint32_t op32) {
    unsigned index = DrawOp_unpackData(op32);

    if (index > fPaints.count()) {
        SkASSERT(!"paint index out of range");
        return NULL;
    }

    if (index == fPaints.count()) {
        *fPaints.append() = SkNEW(SkPaint);
    }
    return fPaints[index];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void clipPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    SkPath path;
    path.unflatten(*reader);
    canvas->clipPath(path, (SkRegion::Op)DrawOp_unpackData(op32));
}

static void clipRegion_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkRegion rgn;
    readRegion(reader, &rgn);
    canvas->clipRegion(rgn, (SkRegion::Op)DrawOp_unpackData(op32));
}

static void clipRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    canvas->clipRect(*skip<SkRect>(reader), (SkRegion::Op)DrawOp_unpackData(op32));
}

///////////////////////////////////////////////////////////////////////////////

static void setMatrix_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    SkMatrix matrix;
    readMatrix(reader, &matrix);
    canvas->setMatrix(matrix);
}

static void concat_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    SkMatrix matrix;
    readMatrix(reader, &matrix);
    canvas->concat(matrix);
}

static void scale_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->scale(param[0], param[1]);
}

static void skew_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->skew(param[0], param[1]);
}

static void rotate_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    canvas->rotate(reader->readScalar());
}

static void translate_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->translate(param[0], param[1]);
}

///////////////////////////////////////////////////////////////////////////////

static void save_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                    SkGPipeState* state) {
    canvas->save((SkCanvas::SaveFlags)DrawOp_unpackData(op32));
}

static void saveLayer_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    unsigned flags = DrawOp_unpackData(op32);

    const SkRect* bounds = NULL;
    if (flags & kSaveLayer_HasBounds_DrawOpFlag) {
        bounds = skip<SkRect>(reader);
    }
    const SkPaint* paint = NULL;
    if (flags & kSaveLayer_HasPaint_DrawOpFlag) {
        paint = &state->getPaint(reader->readU32());
    }
    canvas->saveLayer(bounds, paint,
                      (SkCanvas::SaveFlags)DrawOp_unpackFlags(op32));
}

static void restore_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                       SkGPipeState* state) {
    canvas->restore();
}

///////////////////////////////////////////////////////////////////////////////

static void drawClear_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    SkColor color = 0;
    if (DrawOp_unpackFlags(op32) & kClear_HasColor_DrawOpFlag) {
        color = reader->readU32();
    }
    canvas->clear(color);
}

static void drawPaint_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    canvas->drawPaint(state->getPaint(op32));
}

static void drawPoints_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkCanvas::PointMode mode = (SkCanvas::PointMode)DrawOp_unpackFlags(op32);
    size_t count = reader->readU32();
    const SkPoint* pts = skip<SkPoint>(reader, count);
    canvas->drawPoints(mode, count, pts, state->getPaint(op32));
}

static void drawRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    canvas->drawRect(*skip<SkRect>(reader), state->getPaint(op32));
}

static void drawPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    SkPath path;
    path.unflatten(*reader);
    canvas->drawPath(path, state->getPaint(op32));
}

static void drawVertices_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                            SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);

    SkCanvas::VertexMode mode = (SkCanvas::VertexMode)reader->readU32();
    int vertexCount = reader->readU32();
    const SkPoint* verts = skip<SkPoint>(reader, vertexCount);

    const SkPoint* texs = NULL;
    if (flags & kDrawVertices_HasTexs_DrawOpFlag) {
        texs = skip<SkPoint>(reader, vertexCount);
    }

    const SkColor* colors = NULL;
    if (flags & kDrawVertices_HasColors_DrawOpFlag) {
        colors = skip<SkColor>(reader, vertexCount);
    }

    // TODO: flatten/unflatten xfermodes
    SkXfermode* xfer = NULL;

    int indexCount = 0;
    const uint16_t* indices = NULL;
    if (flags & kDrawVertices_HasIndices_DrawOpFlag) {
        indexCount = reader->readU32();
        indices = skipAlign<uint16_t>(reader, indexCount);
    }

    canvas->drawVertices(mode, vertexCount, verts, texs, colors, xfer,
                         indices, indexCount, state->getPaint(op32));
}

///////////////////////////////////////////////////////////////////////////////

static void drawText_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    const SkScalar* xy = skip<SkScalar>(reader, 2);
    canvas->drawText(text, len, xy[0], xy[1], state->getPaint(op32));
}

static void drawPosText_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    size_t posCount = reader->readU32();    // compute by our writer
    const SkPoint* pos = skip<SkPoint>(reader, posCount);
    canvas->drawPosText(text, len, pos, state->getPaint(op32));
}

static void drawPosTextH_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    size_t posCount = reader->readU32();    // compute by our writer
    const SkScalar* xpos = skip<SkScalar>(reader, posCount);
    SkScalar constY = reader->readScalar();
    canvas->drawPosTextH(text, len, xpos, constY, state->getPaint(op32));
}

static void drawTextOnPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                              SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));

    SkPath path;
    path.unflatten(*reader);

    SkMatrix matrixStorage;
    const SkMatrix* matrix = NULL;
    if (DrawOp_unpackFlags(op32) & kDrawTextOnPath_HasMatrix_DrawOpFlag) {
        readMatrix(reader, &matrixStorage);
        matrix = &matrixStorage;
    }

    canvas->drawTextOnPath(text, len, path, matrix, state->getPaint(op32));
}

///////////////////////////////////////////////////////////////////////////////

static void drawBitmap_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawBitmapMatrix_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                                SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawBitmapRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                              SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawSprite_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    UNIMPLEMENTED
}

///////////////////////////////////////////////////////////////////////////////

static void drawData_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    // since we don't have a paint, we can use data for our (small) sizes
    size_t size = DrawOp_unpackData(op32);
    if (0 == size) {
        size = reader->readU32();
    }
    const void* data = reader->skip(SkAlign4(size));
    canvas->drawData(data, size);
}

static void drawShape_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawPicture_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                           SkGPipeState* state) {
    UNIMPLEMENTED
}

///////////////////////////////////////////////////////////////////////////////

static void inflate_patheffect(SkReader32* reader, SkPaint* paint) {
}

static void inflate_shader(SkReader32* reader, SkPaint* paint) {
}

static void inflate_xfermode(SkReader32* reader, SkPaint* paint) {
}

static void inflate_maskfilter(SkReader32* reader, SkPaint* paint) {
}

static void inflate_colorfilter(SkReader32* reader, SkPaint* paint) {
}

static void inflate_rasterizer(SkReader32* reader, SkPaint* paint) {
}

static void inflate_drawlooper(SkReader32* reader, SkPaint* paint) {
}

static void paintOp_rp(SkCanvas*, SkReader32* reader, uint32_t op32,
                       SkGPipeState* state) {
    SkPaint* p = state->editPaint(op32);
    int done;

    do {
        uint32_t p32 = reader->readU32();
        unsigned op = PaintOp_unpackOp(p32);
        unsigned data = PaintOp_unpackData(p32);
        done = PaintOp_unpackFlags(p32) & kLastOp_PaintOpFlag;

        SkDebugf(" read %08X op=%d flags=%d data=%d\n", p32, op, done, data);

        switch (op) {
            case kReset_PaintOp: p->reset(); break;
            case kFlags_PaintOp: p->setFlags(data); break;
            case kColor_PaintOp: p->setColor(reader->readU32()); break;
            case kStyle_PaintOp: p->setStyle((SkPaint::Style)data); break;
            case kJoin_PaintOp: p->setStrokeJoin((SkPaint::Join)data); break;
            case kCap_PaintOp: p->setStrokeCap((SkPaint::Cap)data); break;
            case kWidth_PaintOp: p->setStrokeWidth(reader->readScalar()); break;
            case kMiter_PaintOp: p->setStrokeMiter(reader->readScalar()); break;
            case kEncoding_PaintOp:
                p->setTextEncoding((SkPaint::TextEncoding)data);
                break;
            case kHinting_PaintOp: p->setHinting((SkPaint::Hinting)data); break;
            case kAlign_PaintOp: p->setTextAlign((SkPaint::Align)data); break;
            case kTextSize_PaintOp: p->setTextSize(reader->readScalar()); break;
            case kTextScaleX_PaintOp: p->setTextScaleX(reader->readScalar()); break;
            case kTextSkewX_PaintOp: p->setTextSkewX(reader->readScalar()); break;
                
            // flag to reference a cached index instead of inflating?
            case kPathEffect_PaintOp: inflate_patheffect(reader, p); break;
            case kShader_PaintOp: inflate_shader(reader, p); break;
            case kXfermode_PaintOp: inflate_xfermode(reader, p); break;
            case kMaskFilter_PaintOp: inflate_maskfilter(reader, p); break;
            case kColorFilter_PaintOp: inflate_colorfilter(reader, p); break;
            case kRasterizer_PaintOp: inflate_rasterizer(reader, p); break;
            case kDrawLooper_PaintOp: inflate_drawlooper(reader, p); break;
            default: SkASSERT(!"bad paintop"); return;
        }
    } while (!done);
}

///////////////////////////////////////////////////////////////////////////////

static void done_rp(SkCanvas*, SkReader32*, uint32_t, SkGPipeState*) {}

typedef void (*ReadProc)(SkCanvas*, SkReader32*, uint32_t op32, SkGPipeState*);

static const ReadProc gReadTable[] = {
    clipPath_rp,
    clipRegion_rp,
    clipRect_rp,
    concat_rp,
    drawBitmap_rp,
    drawBitmapMatrix_rp,
    drawBitmapRect_rp,
    drawClear_rp,
    drawData_rp,
    drawPaint_rp,
    drawPath_rp,
    drawPicture_rp,
    drawPoints_rp,
    drawPosText_rp,
    drawPosTextH_rp,
    drawRect_rp,
    drawShape_rp,
    drawSprite_rp,
    drawText_rp,
    drawTextOnPath_rp,
    drawVertices_rp,
    restore_rp,
    rotate_rp,
    save_rp,
    saveLayer_rp,
    scale_rp,
    setMatrix_rp,
    skew_rp,
    translate_rp,
    paintOp_rp,
    done_rp
};

///////////////////////////////////////////////////////////////////////////////

SkGPipeState::SkGPipeState() {
    // start out with one paint in default state
    *fPaints.append() = SkNEW(SkPaint);
}

SkGPipeState::~SkGPipeState() {
    fPaints.deleteAll();
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

SkGPipeReader::SkGPipeReader(SkCanvas* target) {
    SkSafeRef(target);
    fCanvas = target;
    fState = NULL;
}

SkGPipeReader::~SkGPipeReader() {
    SkSafeUnref(fCanvas);
    delete fState;
}

SkGPipeReader::Status SkGPipeReader::playback(const void* data, size_t length) {
    if (NULL == fCanvas) {
        return kError_Status;
    }

    if (NULL == fState) {
        fState = new SkGPipeState;
    }

    const ReadProc* table = gReadTable;
    SkReader32 reader(data, length);
    SkCanvas* canvas = fCanvas;
    
    while (!reader.eof()) {
        uint32_t op32 = reader.readU32();
        unsigned op = DrawOp_unpackOp(op32);
        
        if (op >= SK_ARRAY_COUNT(gReadTable)) {
            SkDebugf("---- bad op during GPipeState::playback\n");
            return kError_Status;
        }
        if (kDone_DrawOp == op) {
            return kDone_Status;
        }
        table[op](canvas, &reader, op32, fState);
    }
    return kEOF_Status;
}



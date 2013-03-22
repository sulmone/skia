/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontHost.h"
#include "SkTypefaceCache.h"

#define FONT_PATH   "/Library/Fonts/Skia.ttf"

class FTMacTypeface : public SkTypeface {
public:
    FTMacTypeface(Style style, uint32_t id, SkStream* stream) : SkTypeface(style, id) {
        // we take ownership of the stream
        fStream = stream;
    }

    virtual ~FTMacTypeface() {
        fStream->unref();
    }

    SkStream* fStream;
};

static FTMacTypeface* create_from_path(const char path[]) {
    SkStream* stream = SkStream::NewFromFile(path);
    if (!stream) {
        return NULL;
    }

    size_t size = stream->getLength();
    SkASSERT(size);
    FTMacTypeface* tf = new FTMacTypeface(SkTypeface::kNormal,
                                          SkTypefaceCache::NewFontID(),
                                          stream);
    SkTypefaceCache::Add(tf, SkTypeface::kNormal);
    return tf;
}

static SkTypeface* ref_default_typeface() {
    static SkTypeface* gDef;

    if (NULL == gDef) {
        gDef = create_from_path(FONT_PATH);
    }

    gDef->ref();
    return gDef;
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
    return ref_default_typeface();
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    SkDEBUGFAIL("SkFontHost::CreateTypefaceFromStream unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    return create_from_path(path);
}

SkStream* SkFontHost::OpenStream(uint32_t fontID) {
    FTMacTypeface* tf = (FTMacTypeface*)SkTypefaceCache::FindByID(fontID);
    if (tf) {
        tf->fStream->ref();
        return tf->fStream;
    }
    return NULL;
}

void SkFontHost::Serialize(const SkTypeface*, SkWStream*) {
    SkDEBUGFAIL("SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkDEBUGFAIL("SkFontHost::Deserialize unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::NextLogicalTypeface(SkFontID currFontID, SkFontID origFontID) {
    return NULL;
}

#include "SkTypeface_mac.h"

SkTypeface* SkCreateTypefaceFromCTFont(CTFontRef fontRef) {
    SkDEBUGFAIL("Not supported");
    return NULL;
}

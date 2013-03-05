/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEndian.h"
#include "SkFontHost.h"
#include "SkFontStream.h"
#include "SkStream.h"

int SkFontHost::CountTables(SkFontID fontID) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    int ttcIndex = 0;
    return SkFontStream::GetTableTags(stream, ttcIndex, NULL);
}

int SkFontHost::GetTableTags(SkFontID fontID, SkFontTableTag tags[]) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    int ttcIndex = 0;
    return SkFontStream::GetTableTags(stream, ttcIndex, tags);
}

size_t SkFontHost::GetTableSize(SkFontID fontID, SkFontTableTag tag) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    int ttcIndex = 0;
    return SkFontStream::GetTableData(stream, ttcIndex, tag, 0, ~0U, NULL);
}

size_t SkFontHost::GetTableData(SkFontID fontID, SkFontTableTag tag,
                                size_t offset, size_t length, void* data) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    int ttcIndex = 0;
    return SkFontStream::GetTableData(stream, ttcIndex, tag, offset, length, data);
}

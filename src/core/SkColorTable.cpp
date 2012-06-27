
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorTable.h"
#include "SkStream.h"
#include "SkTemplates.h"

SK_DEFINE_INST_COUNT(SkColorTable)

SkColorTable::SkColorTable(int count)
    : f16BitCache(NULL), fFlags(0)
{
    if (count < 0)
        count = 0;
    else if (count > 256)
        count = 256;

    fCount = SkToU16(count);
    fColors = (SkPMColor*)sk_malloc_throw(count * sizeof(SkPMColor));
    memset(fColors, 0, count * sizeof(SkPMColor));

    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)
}

// As copy constructor is hidden in the class hierarchy, we need to call
// default constructor explicitly to suppress a compiler warning.
SkColorTable::SkColorTable(const SkColorTable& src) : INHERITED() {
    f16BitCache = NULL;
    fFlags = src.fFlags;
    int count = src.count();
    fCount = SkToU16(count);
    fColors = reinterpret_cast<SkPMColor*>(
                                    sk_malloc_throw(count * sizeof(SkPMColor)));
    memcpy(fColors, src.fColors, count * sizeof(SkPMColor));

    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)
}

SkColorTable::SkColorTable(const SkPMColor colors[], int count)
    : f16BitCache(NULL), fFlags(0)
{
    if (count < 0)
        count = 0;
    else if (count > 256)
        count = 256;

    fCount = SkToU16(count);
    fColors = reinterpret_cast<SkPMColor*>(
                                    sk_malloc_throw(count * sizeof(SkPMColor)));

    if (colors)
        memcpy(fColors, colors, count * sizeof(SkPMColor));

    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)
}

SkColorTable::~SkColorTable()
{
    SkASSERT(fColorLockCount == 0);
    SkASSERT(f16BitCacheLockCount == 0);

    sk_free(fColors);
    sk_free(f16BitCache);
}

void SkColorTable::setFlags(unsigned flags)
{
    fFlags = SkToU8(flags);
}

void SkColorTable::unlockColors(bool changed)
{
    SkASSERT(fColorLockCount != 0);
    SkDEBUGCODE(fColorLockCount -= 1;)
    if (changed)
        this->inval16BitCache();
}

void SkColorTable::inval16BitCache()
{
    SkASSERT(f16BitCacheLockCount == 0);
    if (f16BitCache)
    {
        sk_free(f16BitCache);
        f16BitCache = NULL;
    }
}

#include "SkColorPriv.h"

static inline void build_16bitcache(uint16_t dst[], const SkPMColor src[], int count)
{
    while (--count >= 0)
        *dst++ = SkPixel32ToPixel16_ToU16(*src++);
}

const uint16_t* SkColorTable::lock16BitCache()
{
    if (fFlags & kColorsAreOpaque_Flag)
    {
        if (f16BitCache == NULL) // build the cache
        {
            f16BitCache = (uint16_t*)sk_malloc_throw(fCount * sizeof(uint16_t));
            build_16bitcache(f16BitCache, fColors, fCount);
        }
    }
    else    // our colors have alpha, so no cache
    {
        this->inval16BitCache();
        if (f16BitCache)
        {
            sk_free(f16BitCache);
            f16BitCache = NULL;
        }
    }

    SkDEBUGCODE(f16BitCacheLockCount += 1);
    return f16BitCache;
}

void SkColorTable::setIsOpaque(bool isOpaque) {
    if (isOpaque) {
        fFlags |= kColorsAreOpaque_Flag;
    } else {
        fFlags &= ~kColorsAreOpaque_Flag;
    }
}

///////////////////////////////////////////////////////////////////////////////

SkColorTable::SkColorTable(SkFlattenableReadBuffer& buffer) {
    f16BitCache = NULL;
    SkDEBUGCODE(fColorLockCount = 0;)
    SkDEBUGCODE(f16BitCacheLockCount = 0;)

    fCount = buffer.readU16();
    SkASSERT((unsigned)fCount <= 256);

    fFlags = buffer.readU8();

    fColors = (SkPMColor*)sk_malloc_throw(fCount * sizeof(SkPMColor));
    buffer.read(fColors, fCount * sizeof(SkPMColor));
}

void SkColorTable::flatten(SkFlattenableWriteBuffer& buffer) const {
    int count = this->count();
    buffer.write16(count);
    buffer.write8(this->getFlags());
    buffer.writeMul4(fColors, count * sizeof(SkPMColor));
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkColorTable)


/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapHeap_DEFINED
#define SkBitmapHeap_DEFINED

#include "SkBitmap.h"
#include "SkFlattenable.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"
#include "SkThread.h"
#include "SkTRefArray.h"

/**
 * SkBitmapHeapEntry provides users of SkBitmapHeap (using internal storage) with a means to...
 *  (1) get access a bitmap in the heap
 *  (2) indicate they are done with bitmap by releasing their reference (if they were an owner).
 */
class SkBitmapHeapEntry : SkNoncopyable {
public:
    ~SkBitmapHeapEntry();

    int32_t getSlot() { return fSlot; }

    SkBitmap* getBitmap() { return &fBitmap; }

    void releaseRef() {
        sk_atomic_dec(&fRefCount);
    }

private:
    SkBitmapHeapEntry();

    void addReferences(int count);

    int32_t fSlot;
    int32_t fRefCount;

    SkBitmap fBitmap;
    // Keep track of the bytes allocated for this bitmap. When replacing the
    // bitmap or removing this HeapEntry we know how much memory has been
    // reclaimed.
    size_t fBytesAllocated;
    // TODO: Generalize the LRU caching mechanism
    SkBitmapHeapEntry* fMoreRecentlyUsed;
    SkBitmapHeapEntry* fLessRecentlyUsed;

    friend class SkBitmapHeap;
};


class SkBitmapHeapReader : public SkRefCnt {
public:
    SkBitmapHeapReader() : INHERITED() {}
    virtual SkBitmap* getBitmap(int32_t slot) const = 0;
    virtual void releaseRef(int32_t slot) = 0;
private:
    typedef SkRefCnt INHERITED;
};


/**
 * TODO: stores immutable bitmaps into a heap
 */
class SkBitmapHeap : public SkBitmapHeapReader {
public:
    class ExternalStorage : public SkRefCnt {
     public:
        virtual bool insert(const SkBitmap& bitmap, int32_t slot) = 0;
    };

    static const int32_t UNLIMITED_SIZE = -1;
    static const int32_t IGNORE_OWNERS  = -1;
    static const int32_t INVALID_SLOT   = -1;

    /**
     * Constructs a heap that is responsible for allocating and managing its own storage.  In the
     * case where we choose to allow the heap to grow indefinitely (i.e. UNLIMITED_SIZE) we
     * guarantee that once allocated in the heap a bitmap's index in the heap is immutable.
     * Otherwise we guarantee the bitmaps placement in the heap until its owner count goes to zero.
     *
     * @param preferredSize  Specifies the preferred maximum number of bitmaps to store. This is
     *   not a hard limit as it can grow larger if the number of bitmaps in the heap with active
     *   owners exceeds this limit.
     * @param ownerCount  The number of owners to assign to each inserted bitmap. NOTE: while a
     *   bitmap in the heap has a least one owner it can't be removed.
     */
    SkBitmapHeap(int32_t preferredSize = UNLIMITED_SIZE, int32_t ownerCount = IGNORE_OWNERS);

    /**
     * Constructs a heap that defers the responsibility of storing the bitmaps to an external
     * function. This is especially useful if the bitmaps will be used in a separate process as the
     * external storage can ensure the data is properly shuttled to the appropriate processes.
     *
     * Our LRU implementation assumes that inserts into the external storage are consumed in the
     * order that they are inserted (i.e. SkPipe). This ensures that we don't need to query the
     * external storage to see if a slot in the heap is eligible to be overwritten.
     *
     * @param externalStorage  The class responsible for storing the bitmaps inserted into the heap
     * @param heapSize  The maximum size of the heap. Because of the sequential limitation imposed
     *   by our LRU implementation we can guarantee that the heap will never grow beyond this size.
     */
    SkBitmapHeap(ExternalStorage* externalStorage, int32_t heapSize = UNLIMITED_SIZE);

    ~SkBitmapHeap();

    /**
     * Makes a shallow copy of all bitmaps currently in the heap and returns them as an array. The
     * array indices match their position in the heap.
     *
     * @return  a ptr to an array of bitmaps or NULL if external storage is being used.
     */
    SkTRefArray<SkBitmap>* extractBitmaps() const;

    /**
     * Retrieves the bitmap from the specified slot in the heap
     *
     * @return  The bitmap located at that slot or NULL if external storage is being used.
     */
    virtual SkBitmap* getBitmap(int32_t slot) const SK_OVERRIDE {
        SkASSERT(fExternalStorage == NULL);
        SkBitmapHeapEntry* entry = getEntry(slot);
        if (entry) {
            return &entry->fBitmap;
        }
        return NULL;
    }

    /**
     * Retrieves the bitmap from the specified slot in the heap
     *
     * @return  The bitmap located at that slot or NULL if external storage is being used.
     */
    virtual void releaseRef(int32_t slot) SK_OVERRIDE {
        SkASSERT(fExternalStorage == NULL);
        if (fOwnerCount != IGNORE_OWNERS) {
            SkBitmapHeapEntry* entry = getEntry(slot);
            if (entry) {
                entry->releaseRef();
            }
        }
    }

    /**
     * Inserts a bitmap into the heap. The stored version of bitmap is guaranteed to be immutable
     * and is not dependent on the lifecycle of the provided bitmap.
     *
     * @param bitmap  the bitmap to be inserted into the heap
     * @return  the slot in the heap where the bitmap is stored or INVALID_SLOT if the bitmap could
     *          not be added to the heap. If it was added the slot will remain valid...
     *            (1) indefinitely if no owner count has been specified.
     *            (2) until all owners have called releaseRef on the appropriate SkBitmapHeapEntry*
     */
    int32_t insert(const SkBitmap& bitmap);

    /**
     * Retrieves an entry from the heap at a given slot.
     *
     * @param slot  the slot in the heap where a bitmap was stored.
     * @return  a SkBitmapHeapEntry that wraps the bitmap or NULL if external storage is used.
     */
    SkBitmapHeapEntry* getEntry(int32_t slot) const {
        SkASSERT(slot <= fStorage.count());
        if (fExternalStorage != NULL) {
            return NULL;
        }
        return fStorage[slot];
    }

    /**
     * Returns a count of the number of items currently in the heap
     */
    int count() const {
        SkASSERT(fExternalStorage != NULL || fStorage.count() == fLookupTable.count());
        return fLookupTable.count();
    }

    /**
     * Returns the total number of bytes allocated by the bitmaps in the heap
     */
    size_t bytesAllocated() const {
        return fBytesAllocated;
    }

private:
    struct LookupEntry {
        uint32_t fGenerationId; // SkPixelRef GenerationID.
        size_t fPixelOffset;
        uint32_t fWidth;
        uint32_t fHeight;

        uint32_t fStorageSlot; // slot of corresponding bitmap in fStorage.

        bool operator < (const LookupEntry& other) const {
            if (this->fGenerationId != other.fGenerationId) {
                return this->fGenerationId < other.fGenerationId;
            } else if(this->fPixelOffset != other.fPixelOffset) {
                return this->fPixelOffset < other.fPixelOffset;
            } else if(this->fWidth != other.fWidth) {
                return this->fWidth < other.fWidth;
            } else {
                return this->fHeight < other.fHeight;
            }
        }
        bool operator != (const LookupEntry& other) const {
            return this->fGenerationId != other.fGenerationId
                || this->fPixelOffset != other.fPixelOffset
                || this->fWidth != other.fWidth
                || this->fHeight != other.fHeight;
        }
    };

    /**
     * Searches for the bitmap in the lookup table and returns the bitmaps index within the table.
     * If the bitmap was not already in the table it is added.
     *
     * @param bitmap  The bitmap we using as a key to search the lookup table
     * @param entry  A pointer to a SkBitmapHeapEntry* that if non-null AND the bitmap is found
     *               in the lookup table is populated with the entry from the heap storage.
     */
    int findInLookupTable(const SkBitmap& bitmap, SkBitmapHeapEntry** entry);

    SkBitmapHeapEntry* findEntryToReplace(const SkBitmap& replacement);
    bool copyBitmap(const SkBitmap& originalBitmap, SkBitmap& copiedBitmap);
    void setMostRecentlyUsed(SkBitmapHeapEntry* entry);

    // searchable index that maps to entries in the heap
    SkTDArray<LookupEntry> fLookupTable;

    // heap storage
    SkTDArray<SkBitmapHeapEntry*> fStorage;
    ExternalStorage* fExternalStorage;

    SkBitmapHeapEntry* fMostRecentlyUsed;
    SkBitmapHeapEntry* fLeastRecentlyUsed;

    const int32_t fPreferredCount;
    const int32_t fOwnerCount;
    size_t fBytesAllocated;

    typedef SkBitmapHeapReader INHERITED;
};

#endif // SkBitmapHeap_DEFINED

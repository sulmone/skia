
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkGPipe_DEFINED
#define SkGPipe_DEFINED

#include "SkWriter32.h"
#include "SkFlattenable.h"

class SkCanvas;

// XLib.h might have defined Status already (ugh)
#ifdef Status
    #undef Status
#endif

class SkGPipeReader {
public:
    SkGPipeReader();
    SkGPipeReader(SkCanvas* target);
    ~SkGPipeReader();

    enum Status {
        kDone_Status,   //!< no more data expected from reader
        kEOF_Status,    //!< need more data from reader
        kError_Status,  //!< encountered error
        kReadAtom_Status//!< finished reading an atom
    };

    void setCanvas(SkCanvas*);
    // data must be 4-byte aligned
    // length must be a multiple of 4
    Status playback(const void* data, size_t length, size_t* bytesRead = NULL,
                    bool readAtom = false);
private:
    SkCanvas*           fCanvas;
    class SkGPipeState* fState;
};

///////////////////////////////////////////////////////////////////////////////

class SkGPipeController {
public:
    /**
     *  Called periodically by the writer, to get a working buffer of RAM to
     *  write into. The actual size of the block is also returned, and must be
     *  actual >= minRequest. If NULL is returned, then actual is ignored and
     *  writing will stop.
     *
     *  The returned block must be 4-byte aligned, and actual must be a
     *  multiple of 4.
     *  minRequest will always be a multiple of 4.
     */
    virtual void* requestBlock(size_t minRequest, size_t* actual) = 0;

    /**
     *  This is called each time some atomic portion of the data has been
     *  written to the block (most recently returned by requestBlock()).
     *  If bytes == 0, then the writer has finished.
     *
     *  bytes will always be a multiple of 4.
     */
    virtual void notifyWritten(size_t bytes) = 0;
    virtual int numberOfReaders() const { return 1; }
};

class SkGPipeWriter {
public:
    SkGPipeWriter();
    ~SkGPipeWriter();

    bool isRecording() const { return NULL != fCanvas; }

    enum Flags {
        /**
         *  Tells the writer that the reader will be in a different process, so
         *  (for example) we cannot put function pointers in the stream.
         */
        kCrossProcess_Flag              = 1 << 0,
        /**
         *  Only meaningful if kCrossProcess_Flag is set. Tells the writer that
         *  in spite of being cross process, it will have shared address space
         *  with the reader, so the two can share large objects (like SkBitmaps)
         */
        kSharedAddressSpace_SkGPipeFlag = 1 << 1
    };

    SkCanvas* startRecording(SkGPipeController*, uint32_t flags = 0);

    // called in destructor, but can be called sooner once you know there
    // should be no more drawing calls made into the recording canvas.
    void endRecording();

    /**
     *  Tells the writer to commit all recorded draw commands to the
     *  controller immediately.
     *  @param detachCurrentBlock Set to true to request that the next draw
     *      command be recorded in a new block.
     */
    void flushRecording(bool detachCurrentBlock);

private:
    class SkGPipeCanvas* fCanvas;
    SkGPipeController*   fController;
    SkFactorySet         fFactorySet;
    SkWriter32           fWriter;
};

#endif

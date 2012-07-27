/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_DEFINED
#define SkSurface_DEFINED

#include "SkRefCnt.h"
#include "SkImage.h"

class SkCanvas;
class SkPaint;

/**
 *  SkSurface represents the backend/results of drawing to a canvas. For raster
 *  drawing, the surface will be pixels, but (for example) when drawing into
 *  a PDF or Picture canvas, the surface stores the recorded commands.
 *
 *  To draw into a canvas, first create the appropriate type of Surface, and
 *  then request the canvas from the surface.
 */
class SkSurface : public SkRefCnt {
public:
    /**
     *  Create a new surface, using the specified pixels/rowbytes as its
     *  backend.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static SkSurface* NewRasterDirect(const SkImage::Info&, SkColorSpace*,
                                      void* pixels, size_t rowBytes);

    /**
     *  Return a new surface, with the memory for the pixels automatically
     *  allocated.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static SkSurface* NewRaster(const SkImage::Info&, SkColorSpace*);

    /**
     *  Return a new surface whose contents will be recorded into a picture.
     *  When this surface is drawn into another canvas, its contents will be
     *  "replayed" into that canvas.
     */
    static SkSurface* NewPicture(int width, int height);

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    /**
     *  Returns a unique non-zero, unique value identifying the content of this
     *  surface. Each time the content is changed changed, either by drawing
     *  into this surface, or explicitly calling notifyContentChanged()) this
     *  method will return a new value.
     *
     *  If this surface is empty (i.e. has a zero-dimention), this will return
     *  0.
     */
    uint32_t generationID() const;
    
    /**
     *  Call this if the contents have changed. This will (lazily) force a new
     *  value to be returned from generationID() when it is called next.
     */
    void notifyContentChanged();
    
    /**
     *  Return a canvas that will draw into this surface.
     *
     *  LIFECYCLE QUESTIONS
     *  1. Is this owned by the surface or the caller?
     *  2. Can the caller get a 2nd canvas, or reset the state of the first?
     */
    SkCanvas* newCanvas();

    /**
     *  Return a new surface that is "compatible" with this one, in that it will
     *  efficiently be able to be drawn into this surface. Typical calling
     *  pattern:
     *
     *  SkSurface* A = SkSurface::New...();
     *  SkCanvas* canvasA = surfaceA->newCanvas();
     *  ...
     *  SkSurface* surfaceB = surfaceA->newSurface(...);
     *  SkCanvas* canvasB = surfaceB->newCanvas();
     *  ... // draw using canvasB
     *  canvasA->drawSurface(surfaceB); // <--- this will always be optimal!
     */
    SkSurface* newSurface(const SkImage::Info&, SkColorSpace*);

    /**
     *  Returns an image of the current state of the surface pixels up to this
     *  point. Subsequent changes to the surface (by drawing into its canvas)
     *  will not be reflected in this image.
     */
    SkImage* newImageShapshot();

    /**
     *  Thought the caller could get a snapshot image explicitly, and draw that,
     *  it seems that directly drawing a surface into another canvas might be
     *  a common pattern, and that we could possibly be more efficient, since
     *  we'd know that the "snapshot" need only live until we've handed it off
     *  to the canvas.
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

protected:
    SkSurface(int width, int height);

private:
    const int           fWidth;
    const int           fHeight;
    mutable uint32_t    fGenerationID;
};

#endif

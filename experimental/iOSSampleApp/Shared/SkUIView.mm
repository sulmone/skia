#import "SkUIView.h"
#include <QuartzCore/QuartzCore.h>

//#define SKWIND_CONFIG       SkBitmap::kRGB_565_Config
#define SKWIND_CONFIG       SkBitmap::kARGB_8888_Config
#define SKGL_CONFIG         kEAGLColorFormatRGB565
//#define SKGL_CONFIG         kEAGLColorFormatRGBA8

#define FORCE_REDRAW

//#define USE_GL_1
//#define USE_GL_2
#if defined(USE_GL_1) || defined(USE_GL_2)
#define USE_GL
#endif

#include "SkCanvas.h"
#include "GrContext.h"
#include "GrGLInterface.h"
#include "SkGpuDevice.h"
#include "SkCGUtils.h"

SkiOSDeviceManager::SkiOSDeviceManager() {
    fGrContext = NULL;
    fGrRenderTarget = NULL;
    usingGL = false;
}
SkiOSDeviceManager::~SkiOSDeviceManager() {
    SkSafeUnref(fGrContext);
    SkSafeUnref(fGrRenderTarget);
}

void SkiOSDeviceManager::init(SampleWindow* win) {
    win->attachGL();
    if (NULL == fGrContext) {
#if defined(USE_GL_1)
        fGrContext = GrContext::Create(kOpenGL_Fixed_GrEngine, NULL);
#elsif defined(USE_GL_2)
        fGrContext = GrContext::Create(kOpenGL_Shaders_GrEngine, NULL);
#endif
    }
    fGrRenderTarget = SkGpuDevice::Current3DApiRenderTarget();
    if (NULL == fGrContext) {
        SkDebugf("Failed to setup 3D");
        win->detachGL();
    }
}    
bool SkiOSDeviceManager::supportsDeviceType(SampleWindow::DeviceType dType) {
    switch (dType) {
        case SampleWindow::kRaster_DeviceType:
        case SampleWindow::kPicture_DeviceType: // fallthru
            return true;
        case SampleWindow::kGPU_DeviceType:
            return NULL != fGrContext;
        default:
            return false;
    }
}
bool SkiOSDeviceManager::prepareCanvas(SampleWindow::DeviceType dType,
                                       SkCanvas* canvas,
                                       SampleWindow* win) {
    if (SampleWindow::kGPU_DeviceType == dType) {
        canvas->setDevice(new SkGpuDevice(fGrContext, fGrRenderTarget))->unref();
        usingGL = true;
    }
    else {
        //The clip needs to be applied with a device attached to the canvas
        //canvas->setBitmapDevice(win->getBitmap());
        usingGL = false;
    }
    return true;
}

void SkiOSDeviceManager::publishCanvas(SampleWindow::DeviceType dType,
                                       SkCanvas* canvas,
                                       SampleWindow* win) {
    if (SampleWindow::kGPU_DeviceType == dType) {
        fGrContext->flush();
    }
    else {
        CGContextRef cg = UIGraphicsGetCurrentContext();
        SkCGDrawBitmap(cg, win->getBitmap(), 0, 0);
    }
    win->presentGL();
}
////////////////////////////////////////////////////////////////////////////////
@implementation SkUIView

@synthesize fWind, fTitle, fTitleItem;

#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"

static float gScreenScale = 1;

#define kREDRAW_UIVIEW_GL "sk_redraw_uiview_gl_iOS"

static const float SCALE_FOR_ZOOM_LENS = 4.0;
#define Y_OFFSET_FOR_ZOOM_LENS           200
#define SIZE_FOR_ZOOM_LENS               250

static const float MAX_ZOOM_SCALE = 4.0;
static const float MIN_ZOOM_SCALE = 2.0 / MAX_ZOOM_SCALE;

extern bool gDoTraceDraw;
#define DO_TRACE_DRAW_MAX   100

struct FPSState {
    static const int FRAME_COUNT = 60;

    CFTimeInterval fNow0, fNow1;
    CFTimeInterval fTime0, fTime1, fTotalTime;
    int fFrameCounter;
    int fDrawCounter;
    SkString str;
    FPSState() {
        fTime0 = fTime1 = fTotalTime = 0;
        fFrameCounter = 0;
    }

    void startDraw() {
        fNow0 = CACurrentMediaTime();
        
        if (0 == fDrawCounter && false) {
            gDoTraceDraw = true;
            SkDebugf("\n");
        }
    }
    
    void endDraw() {
        fNow1 = CACurrentMediaTime();

        if (0 == fDrawCounter) {
            gDoTraceDraw = true;
        }
        if (DO_TRACE_DRAW_MAX == ++fDrawCounter) {
            fDrawCounter = 0;
        }
    }
        
    void flush(SkOSWindow* hwnd) {
        CFTimeInterval now2 = CACurrentMediaTime();

        fTime0 += fNow1 - fNow0;
        fTime1 += now2 - fNow1;
        
        if (++fFrameCounter == FRAME_COUNT) {
            CFTimeInterval totalNow = CACurrentMediaTime();
            fTotalTime = totalNow - fTotalTime;
            
            //SkMSec ms0 = (int)(1000 * fTime0 / FRAME_COUNT);
            //SkMSec msTotal = (int)(1000 * fTotalTime / FRAME_COUNT);
            //str.printf(" ms: %d [%d], fps: %3.1f", msTotal, ms0,
            //           FRAME_COUNT / fTotalTime);
            str.printf(" fps:%3.1f", FRAME_COUNT / fTotalTime);
            hwnd->setTitle(NULL);
            fTotalTime = totalNow;
            fTime0 = fTime1 = 0;
            fFrameCounter = 0;
        }
    }
};

static FPSState gFPS;

#define FPS_StartDraw() gFPS.startDraw()
#define FPS_EndDraw()   gFPS.endDraw()
#define FPS_Flush(wind) gFPS.flush(wind)


///////////////////////////////////////////////////////////////////////////////
#ifdef USE_GL
+ (Class) layerClass {
	return [CAEAGLLayer class];
}
#endif

- (id)initWithMyDefaults {
    fRedrawRequestPending = false;
    fFPSState = new FPSState;
#ifdef USE_GL
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO],
                                    kEAGLDrawablePropertyRetainedBacking,
                                    SKGL_CONFIG,
                                    kEAGLDrawablePropertyColorFormat,
                                    nil];
    
#ifdef USE_GL_1
    fGL.fContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#else
    fGL.fContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
    
    if (!fGL.fContext || ![EAGLContext setCurrentContext:fGL.fContext])
    {
        [self release];
        return nil;
    }
    
    // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
    glGenFramebuffers(1, &fGL.fFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, fGL.fFramebuffer);
    
    glGenRenderbuffers(1, &fGL.fRenderbuffer);
    glGenRenderbuffers(1, &fGL.fStencilbuffer);
    
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fGL.fRenderbuffer);
    
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fStencilbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fGL.fStencilbuffer);
#endif
    
    fDevManager = new SkiOSDeviceManager;
    fWind = new SampleWindow(self, NULL, NULL, fDevManager);
    application_init();
    fWind->resize(self.frame.size.width, self.frame.size.height, SKWIND_CONFIG);
    fMatrix.reset();
    fZoomAround = false;

    return self;
}

- (id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self = [self initWithMyDefaults];
    }
    return self;
}

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        self = [self initWithMyDefaults];
    }
    return self;
}

- (void)dealloc {
    delete fWind;
    delete fDevManager;
    delete fFPSState;
    application_term();
    [fTitleItem release];
    [super dealloc];
}
- (void)drawWithCanvas:(SkCanvas*)canvas {
    fRedrawRequestPending = false;
    fFPSState->startDraw();
    fWind->draw(canvas);
    fFPSState->endDraw();
#ifdef FORCE_REDRAW
    fWind->inval(NULL);
#endif
    fFPSState->flush(fWind);
}

///////////////////////////////////////////////////////////////////////////////

- (void)layoutSubviews {
    int W, H;
    gScreenScale = [UIScreen mainScreen].scale;
#ifdef USE_GL
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
    if ([self respondsToSelector:@selector(setContentScaleFactor:)]) {
        self.contentScaleFactor = gScreenScale;
    }
    
    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    [fGL.fContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fGL.fWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fGL.fHeight);

    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fStencilbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fGL.fWidth, fGL.fHeight);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    
    W = fGL.fWidth;
    H = fGL.fHeight;
#else
    CGRect rect = [self bounds];
    W = (int)CGRectGetWidth(rect);
    H = (int)CGRectGetHeight(rect);
#endif

    printf("---- layoutSubviews %d %d\n", W, H);
    fWind->resize(W, H);
    fWind->inval(NULL);
}

#ifdef USE_GL
#include "SkDevice.h"

- (void)drawInGL {
    // This application only creates a single context which is already set current at this point.
    // This call is redundant, but needed if dealing with multiple contexts.
    [EAGLContext setCurrentContext:fGL.fContext];
    
    // This application only creates a single default framebuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebuffer(GL_FRAMEBUFFER, fGL.fFramebuffer);
    
    GLint scissorEnable;
    glGetIntegerv(GL_SCISSOR_TEST, &scissorEnable);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (scissorEnable) {
        glEnable(GL_SCISSOR_TEST);
    }
    glViewport(0, 0, fWind->width(), fWind->height());
    
    
    GrContext* ctx = fDevManager->getGrContext();
    SkASSERT(NULL != ctx);
    
    SkCanvas canvas;
    canvas.setDevice(new SkGpuDevice(ctx, SkGpuDevice::Current3DApiRenderTarget()))->unref();
    
    // if we're not "retained", then we have to always redraw everything.
    // This call forces us to ignore the fDirtyRgn, and draw everywhere.
    // If we are "retained", we can skip this call (as the raster case does)
    fWind->forceInvalAll();
    
    [self drawWithCanvas:&canvas];

    // This application only creates a single color renderbuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    [fGL.fContext presentRenderbuffer:GL_RENDERBUFFER];
    
#if GR_COLLECT_STATS
//    static int frame = 0;
//    if (!(frame % 100)) {
//        ctx->printStats();
//    }
//    ctx->resetStats();
//    ++frame;
#endif
}

#else   // raster case

- (void)drawRect:(CGRect)rect {
    SkCanvas canvas;
    canvas.setBitmapDevice(fWind->getBitmap());
    [self drawWithCanvas:&canvas];
}
#endif

//Gesture Handlers
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kDown_State, touch);
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kMoved_State, touch);
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, touch);
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, touch);
    }
}

///////////////////////////////////////////////////////////////////////////////

- (void)setSkTitle:(const char *)title {
    NSString* text = [NSString stringWithUTF8String:title];
    if ([text length] > 0)
        self.fTitle = text;
    
    if (fTitleItem && fTitle) {
        fTitleItem.title = [NSString stringWithFormat:@"%@%@", fTitle, 
                            [NSString stringWithUTF8String:fFPSState->str.c_str()]];
    }
}

- (BOOL)onHandleEvent:(const SkEvent&)evt {
#ifdef USE_GL
    if (evt.isType(kREDRAW_UIVIEW_GL)) {
        [self drawInGL];
        return true;
    }
#endif
    return false;
}

- (void)postInvalWithRect:(const SkIRect*)r {
#ifdef USE_GL

#if 1
    if (!fRedrawRequestPending) {
        fRedrawRequestPending = true;
        /*
            performSelectorOnMainThread seems to starve updating other views
            (e.g. our FPS view in the titlebar), so we use the afterDelay
            version
         */
        if (0) {
            [self performSelectorOnMainThread:@selector(drawInGL) withObject:nil waitUntilDone:NO];
        } else {
            [self performSelector:@selector(drawInGL) withObject:nil afterDelay:0];
        }
    }
#else
    if (!fRedrawRequestPending) {
        SkEvent* evt = new SkEvent(kREDRAW_UIVIEW_GL);
        evt->post(fWind->getSinkID());
        fRedrawRequestPending = true;
    }
#endif

#else
    if (r) {
        [self setNeedsDisplayInRect:CGRectMake(r->fLeft, r->fTop,
                                               r->width(), r->height())];
    } else {
        [self setNeedsDisplay];
    }
#endif
}

@end

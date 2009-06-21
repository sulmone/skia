#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkNWayCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTime.h"

#include "SkBenchmark.h"

static void erase(SkBitmap& bm) {
    if (bm.config() == SkBitmap::kA8_Config) {
        bm.eraseColor(0);
    } else {
        bm.eraseColor(SK_ColorWHITE);
    }
}

static bool equal(const SkBitmap& bm1, const SkBitmap& bm2) {
    if (bm1.width() != bm2.width() ||
        bm1.height() != bm2.height() ||
        bm1.config() != bm2.config()) {
        return false;
    }
    
    size_t pixelBytes = bm1.width() * bm1.bytesPerPixel();
    for (int y = 0; y < bm1.height(); y++) {
        if (memcmp(bm1.getAddr(0, y), bm2.getAddr(0, y), pixelBytes)) {
            return false;
        }
    }

    return true;
}

class Iter {
public:
    Iter() {
        fBench = BenchRegistry::Head();
    }
    
    SkBenchmark* next() {
        if (fBench) {
            BenchRegistry::Factory f = fBench->factory();
            fBench = fBench->next();
            return f(0);
        }
        return NULL;
    }
    
private:
    const BenchRegistry* fBench;
};

static void make_filename(const char name[], SkString* path) {
    path->set(name);
    for (int i = 0; name[i]; i++) {
        switch (name[i]) {
            case '/':
            case '\\':
            case ' ':
            case ':':
                path->writable_str()[i] = '-';
                break;
            default:
                break;
        }
    }
}

static void saveFile(const char name[], const char config[], const char dir[],
                     const SkBitmap& bm) {
    SkBitmap copy;
    if (!bm.copyTo(&copy, SkBitmap::kARGB_8888_Config)) {
        return;
    }
    
    if (bm.config() == SkBitmap::kA8_Config) {
        // turn alpha into gray-scale
        size_t size = copy.getSize() >> 2;
        SkPMColor* p = copy.getAddr32(0, 0);
        for (size_t i = 0; i < size; i++) {
            int c = (*p >> SK_A32_SHIFT) & 0xFF;
            c = 255 - c;
            c |= (c << 24) | (c << 16) | (c << 8);
            *p++ = c | (SK_A32_MASK << SK_A32_SHIFT);
        }
    }

    SkString str;
    make_filename(name, &str);
    str.appendf("_%s.png", config);
    str.prepend(dir);
    ::remove(str.c_str());
    SkImageEncoder::EncodeFile(str.c_str(), copy, SkImageEncoder::kPNG_Type,
                               100);
}

static void performClip(SkCanvas* canvas, int w, int h) {
    SkRect r;
    
    r.set(SkIntToScalar(10), SkIntToScalar(10),
          SkIntToScalar(w*2/3), SkIntToScalar(h*2/3));
    canvas->clipRect(r, SkRegion::kIntersect_Op);

    r.set(SkIntToScalar(w/3), SkIntToScalar(h/3),
          SkIntToScalar(w-10), SkIntToScalar(h-10));
    canvas->clipRect(r, SkRegion::kXOR_Op);
}

static void performRotate(SkCanvas* canvas, int w, int h) {
    const SkScalar x = SkIntToScalar(w) / 2;
    const SkScalar y = SkIntToScalar(h) / 2;
    
    canvas->translate(x, y);
    canvas->rotate(SkIntToScalar(35));
    canvas->translate(-x, -y);
}

static void compare_pict_to_bitmap(SkPicture* pict, const SkBitmap& bm) {
    SkBitmap bm2;
    
    bm2.setConfig(bm.config(), bm.width(), bm.height());
    bm2.allocPixels();
    erase(bm2);

    SkCanvas canvas(bm2);
    canvas.drawPicture(*pict);

    if (!equal(bm, bm2)) {
        SkDebugf("----- compare_pict_to_bitmap failed\n");
    }
}

static const struct {
    SkBitmap::Config    fConfig;
    const char*         fName;
} gConfigs[] = {
    { SkBitmap::kARGB_8888_Config,  "8888" },
    { SkBitmap::kRGB_565_Config,    "565",  },
    { SkBitmap::kARGB_4444_Config,  "4444", },
    { SkBitmap::kA8_Config,         "A8",   }
};

static int findConfig(const char config[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
        if (!strcmp(config, gConfigs[i].fName)) {
            return i;
        }
    }
    return -1;
}

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;

    int repeatDraw = 1;
    int forceAlpha = 0xFF;
    bool forceAA = true;
    bool doRotate = false;
    bool doClip = false;
    bool doPict = false;

    SkString outDir;
    SkBitmap::Config outConfig = SkBitmap::kARGB_8888_Config;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-o") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outDir.set(*argv);
                if (outDir.c_str()[outDir.size() - 1] != '/') {
                    outDir.append("/");
                }
            }
        } else if (strcmp(*argv, "-pict") == 0) {
            doPict = true;
        } else if (strcmp(*argv, "-repeat") == 0) {
            argv++;
            if (argv < stop) {
                repeatDraw = atoi(*argv);
                if (repeatDraw < 1) {
                    repeatDraw = 1;
                }
            } else {
                fprintf(stderr, "missing arg for -repeat\n");
                return -1;
            }
        } else if (!strcmp(*argv, "-rotate")) {
            doRotate = true;
        } else if (!strcmp(*argv, "-clip")) {
            doClip = true;
        } else if (strcmp(*argv, "-forceAA") == 0) {
            forceAA = true;
        } else if (strcmp(*argv, "-forceBW") == 0) {
            forceAA = false;
        } else if (strcmp(*argv, "-forceBlend") == 0) {
            forceAlpha = 0x80;
        } else if (strcmp(*argv, "-forceOpaque") == 0) {
            forceAlpha = 0xFF;
        } else {
            int index = findConfig(*argv);
            if (index >= 0) {
                outConfig = gConfigs[index].fConfig;
            }
        }
    }
    
    const char* configName = "";
    int configCount = SK_ARRAY_COUNT(gConfigs);
    
    Iter iter;
    SkBenchmark* bench;
    while ((bench = iter.next()) != NULL) {
        SkIPoint dim = bench->getSize();
        if (dim.fX <= 0 || dim.fY <= 0) {
            continue;
        }
        
        bench->setForceAlpha(forceAlpha);
        bench->setForceAA(forceAA);

        printf("running bench %16s", bench->getName());

        for (int configIndex = 0; configIndex < configCount; configIndex++) {
            if (configCount > 1) {
                outConfig = gConfigs[configIndex].fConfig;
                configName = gConfigs[configIndex].fName;
            }
            
            SkBitmap bm;
            bm.setConfig(outConfig, dim.fX, dim.fY);
            bm.allocPixels();
            erase(bm);

            SkCanvas canvas(bm);

            if (doClip) {
                performClip(&canvas, dim.fX, dim.fY);
            }
            if (doRotate) {
                performRotate(&canvas, dim.fX, dim.fY);
            }

            SkMSec now = SkTime::GetMSecs();
            for (int i = 0; i < repeatDraw; i++) {
                SkCanvas* c = &canvas;

                SkNWayCanvas nway;
                SkPicture* pict = NULL;
                if (doPict) {
                    pict = new SkPicture;
                    nway.addCanvas(pict->beginRecording(bm.width(), bm.height()));
                    nway.addCanvas(&canvas);
                    c = &nway;
                }

                SkAutoCanvasRestore acr(c, true);
                bench->draw(c);
                
                if (pict) {
                    compare_pict_to_bitmap(pict, bm);
                    pict->unref();
                }
            }
            if (repeatDraw > 1) {
                printf("  %4s:%7.2f", configName,
                       (SkTime::GetMSecs() - now) / (double)repeatDraw);
            }
            if (outDir.size() > 0) {
                saveFile(bench->getName(), configName, outDir.c_str(), bm);
            }
        }
        printf("\n");
    }
    
    return 0;
}

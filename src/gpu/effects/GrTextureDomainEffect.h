/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureDomainEffect_DEFINED
#define GrTextureDomainEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrRect.h"

class GrGLTextureDomainEffect;

/**
 * Limits a texture's lookup coordinates to a domain.
 */
class GrTextureDomainEffect : public GrSingleTextureEffect {

public:
    /** Uses default texture params (no filter, clamp) */
    GrTextureDomainEffect(GrTexture*, const GrRect& domain);

    GrTextureDomainEffect(GrTexture*, const GrRect& domain, const GrTextureParams& params);

    virtual ~GrTextureDomainEffect();

    static const char* Name() { return "TextureDomain"; }

    typedef GrGLTextureDomainEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrEffect&) const SK_OVERRIDE;

    const GrRect& domain() const { return fTextureDomain; }

protected:

    GrRect fTextureDomain;

private:
    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif

# Include this gypi to include all 'gr' and 'skgr' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
  'variables': {
    'gr_sources': [
      '<(skia_include_path)/gpu/GrAARectRenderer.h',
      '<(skia_include_path)/gpu/GrBackendEffectFactory.h',
      '<(skia_include_path)/gpu/GrClipData.h',
      '<(skia_include_path)/gpu/GrColor.h',
      '<(skia_include_path)/gpu/GrConfig.h',
      '<(skia_include_path)/gpu/GrContext.h',
      '<(skia_include_path)/gpu/GrContextFactory.h',
      '<(skia_include_path)/gpu/GrEffect.h',
      '<(skia_include_path)/gpu/GrEffectStage.h',
      '<(skia_include_path)/gpu/GrEffectUnitTest.h',
      '<(skia_include_path)/gpu/GrFontScaler.h',
      '<(skia_include_path)/gpu/GrGlyph.h',
      '<(skia_include_path)/gpu/GrKey.h',
      '<(skia_include_path)/gpu/GrNoncopyable.h',
      '<(skia_include_path)/gpu/GrPaint.h',
      '<(skia_include_path)/gpu/GrPathRendererChain.h',
      '<(skia_include_path)/gpu/GrPoint.h',
      '<(skia_include_path)/gpu/GrRect.h',
      '<(skia_include_path)/gpu/GrRefCnt.h',
      '<(skia_include_path)/gpu/GrRenderTarget.h',
      '<(skia_include_path)/gpu/GrResource.h',
      '<(skia_include_path)/gpu/GrSurface.h',
      '<(skia_include_path)/gpu/GrTBackendEffectFactory.h',
      '<(skia_include_path)/gpu/GrTextContext.h',
      '<(skia_include_path)/gpu/GrTexture.h',
      '<(skia_include_path)/gpu/GrTextureAccess.h',
      '<(skia_include_path)/gpu/GrTypes.h',
      '<(skia_include_path)/gpu/GrUserConfig.h',

      '<(skia_include_path)/gpu/gl/GrGLConfig.h',
      '<(skia_include_path)/gpu/gl/GrGLExtensions.h',
      '<(skia_include_path)/gpu/gl/GrGLFunctions.h',
      '<(skia_include_path)/gpu/gl/GrGLInterface.h',

      '<(skia_src_path)/gpu/GrAAHairLinePathRenderer.cpp',
      '<(skia_src_path)/gpu/GrAAHairLinePathRenderer.h',
      '<(skia_src_path)/gpu/GrAAConvexPathRenderer.cpp',
      '<(skia_src_path)/gpu/GrAAConvexPathRenderer.h',
      '<(skia_src_path)/gpu/GrAARectRenderer.cpp',
      '<(skia_src_path)/gpu/GrAddPathRenderers_default.cpp',
      '<(skia_src_path)/gpu/GrAllocator.h',
      '<(skia_src_path)/gpu/GrAllocPool.h',
      '<(skia_src_path)/gpu/GrAllocPool.cpp',
      '<(skia_src_path)/gpu/GrAtlas.cpp',
      '<(skia_src_path)/gpu/GrAtlas.h',
      '<(skia_src_path)/gpu/GrBinHashKey.h',
      '<(skia_src_path)/gpu/GrBufferAllocPool.cpp',
      '<(skia_src_path)/gpu/GrBufferAllocPool.h',
      '<(skia_src_path)/gpu/GrCacheID.cpp',
      '<(skia_src_path)/gpu/GrClipData.cpp',
      '<(skia_src_path)/gpu/GrContext.cpp',
      '<(skia_src_path)/gpu/GrDefaultPathRenderer.cpp',
      '<(skia_src_path)/gpu/GrDefaultPathRenderer.h',
      '<(skia_src_path)/gpu/GrDrawState.cpp',
      '<(skia_src_path)/gpu/GrDrawState.h',
      '<(skia_src_path)/gpu/GrDrawTarget.cpp',
      '<(skia_src_path)/gpu/GrDrawTarget.h',
      '<(skia_src_path)/gpu/GrEffect.cpp',
      '<(skia_src_path)/gpu/GrGeometryBuffer.cpp',
      '<(skia_src_path)/gpu/GrGeometryBuffer.h',
      '<(skia_src_path)/gpu/GrClipMaskCache.h',
      '<(skia_src_path)/gpu/GrClipMaskCache.cpp',
      '<(skia_src_path)/gpu/GrClipMaskManager.h',
      '<(skia_src_path)/gpu/GrClipMaskManager.cpp',
      '<(skia_src_path)/gpu/GrGpu.cpp',
      '<(skia_src_path)/gpu/GrGpu.h',
      '<(skia_src_path)/gpu/GrGpuFactory.cpp',
      '<(skia_src_path)/gpu/GrIndexBuffer.h',
      '<(skia_src_path)/gpu/GrInOrderDrawBuffer.cpp',
      '<(skia_src_path)/gpu/GrInOrderDrawBuffer.h',
      '<(skia_src_path)/gpu/GrMemory.cpp',
      '<(skia_src_path)/gpu/GrMemoryPool.cpp',
      '<(skia_src_path)/gpu/GrMemoryPool.h',
      '<(skia_src_path)/gpu/GrPath.cpp',
      '<(skia_src_path)/gpu/GrPath.h',
      '<(skia_src_path)/gpu/GrPathRendererChain.cpp',
      '<(skia_src_path)/gpu/GrPathRenderer.cpp',
      '<(skia_src_path)/gpu/GrPathRenderer.h',
      '<(skia_src_path)/gpu/GrPathUtils.cpp',
      '<(skia_src_path)/gpu/GrPathUtils.h',
      '<(skia_src_path)/gpu/GrPlotMgr.h',
      '<(skia_src_path)/gpu/GrRectanizer.cpp',
      '<(skia_src_path)/gpu/GrRectanizer.h',
      '<(skia_src_path)/gpu/GrRedBlackTree.h',
      '<(skia_src_path)/gpu/GrRenderTarget.cpp',
      '<(skia_src_path)/gpu/GrReducedClip.cpp',
      '<(skia_src_path)/gpu/GrReducedClip.h',
      '<(skia_src_path)/gpu/GrResource.cpp',
      '<(skia_src_path)/gpu/GrResourceCache.cpp',
      '<(skia_src_path)/gpu/GrResourceCache.h',
      '<(skia_src_path)/gpu/GrStencil.cpp',
      '<(skia_src_path)/gpu/GrStencil.h',
      '<(skia_src_path)/gpu/GrStencilAndCoverPathRenderer.cpp',
      '<(skia_src_path)/gpu/GrStencilAndCoverPathRenderer.h',
      '<(skia_src_path)/gpu/GrStencilBuffer.cpp',
      '<(skia_src_path)/gpu/GrStencilBuffer.h',
      '<(skia_src_path)/gpu/GrTBSearch.h',
      '<(skia_src_path)/gpu/GrSWMaskHelper.cpp',
      '<(skia_src_path)/gpu/GrSWMaskHelper.h',
      '<(skia_src_path)/gpu/GrSoftwarePathRenderer.cpp',
      '<(skia_src_path)/gpu/GrSoftwarePathRenderer.h',
      '<(skia_src_path)/gpu/GrSurface.cpp',
      '<(skia_src_path)/gpu/GrTemplates.h',
      '<(skia_src_path)/gpu/GrTextContext.cpp',
      '<(skia_src_path)/gpu/GrTextStrike.cpp',
      '<(skia_src_path)/gpu/GrTextStrike.h',
      '<(skia_src_path)/gpu/GrTextStrike_impl.h',
      '<(skia_src_path)/gpu/GrTexture.cpp',
      '<(skia_src_path)/gpu/GrTextureAccess.cpp',
      '<(skia_src_path)/gpu/GrTHashCache.h',
      '<(skia_src_path)/gpu/GrVertexBuffer.h',
      '<(skia_src_path)/gpu/gr_unittests.cpp',

      '<(skia_src_path)/gpu/effects/Gr1DKernelEffect.h',
      '<(skia_src_path)/gpu/effects/GrTextureStripAtlas.h',
      '<(skia_src_path)/gpu/effects/GrTextureStripAtlas.cpp',
      '<(skia_src_path)/gpu/effects/GrConfigConversionEffect.cpp',
      '<(skia_src_path)/gpu/effects/GrConfigConversionEffect.h',
      '<(skia_src_path)/gpu/effects/GrConvolutionEffect.cpp',
      '<(skia_src_path)/gpu/effects/GrConvolutionEffect.h',
      '<(skia_src_path)/gpu/effects/GrSimpleTextureEffect.cpp',
      '<(skia_src_path)/gpu/effects/GrSimpleTextureEffect.h',
      '<(skia_src_path)/gpu/effects/GrSingleTextureEffect.cpp',
      '<(skia_src_path)/gpu/effects/GrSingleTextureEffect.h',
      '<(skia_src_path)/gpu/effects/GrTextureDomainEffect.cpp',
      '<(skia_src_path)/gpu/effects/GrTextureDomainEffect.h',

      '<(skia_src_path)/gpu/gl/GrGLBufferImpl.cpp',
      '<(skia_src_path)/gpu/gl/GrGLBufferImpl.h',
      '<(skia_src_path)/gpu/gl/GrGLCaps.cpp',
      '<(skia_src_path)/gpu/gl/GrGLCaps.h',
      '<(skia_src_path)/gpu/gl/GrGLContext.cpp',
      '<(skia_src_path)/gpu/gl/GrGLContext.h',
      '<(skia_src_path)/gpu/gl/GrGLCreateNativeInterface_none.cpp',
      '<(skia_src_path)/gpu/gl/GrGLDefaultInterface_none.cpp',
      '<(skia_src_path)/gpu/gl/GrGLDefines.h',
      '<(skia_src_path)/gpu/gl/GrGLEffect.cpp',
      '<(skia_src_path)/gpu/gl/GrGLEffect.h',
      '<(skia_src_path)/gpu/gl/GrGLExtensions.cpp',
      '<(skia_src_path)/gpu/gl/GrGLEffectMatrix.cpp',
      '<(skia_src_path)/gpu/gl/GrGLEffectMatrix.h',
      '<(skia_src_path)/gpu/gl/GrGLIndexBuffer.cpp',
      '<(skia_src_path)/gpu/gl/GrGLIndexBuffer.h',
      '<(skia_src_path)/gpu/gl/GrGLInterface.cpp',
      '<(skia_src_path)/gpu/gl/GrGLIRect.h',
      '<(skia_src_path)/gpu/gl/GrGLNoOpInterface.cpp',
      '<(skia_src_path)/gpu/gl/GrGLNoOpInterface.h',
      '<(skia_src_path)/gpu/gl/GrGLPath.cpp',
      '<(skia_src_path)/gpu/gl/GrGLPath.h',
      '<(skia_src_path)/gpu/gl/GrGLProgram.cpp',
      '<(skia_src_path)/gpu/gl/GrGLProgram.h',
      '<(skia_src_path)/gpu/gl/GrGLRenderTarget.cpp',
      '<(skia_src_path)/gpu/gl/GrGLRenderTarget.h',
      '<(skia_src_path)/gpu/gl/GrGLShaderBuilder.cpp',
      '<(skia_src_path)/gpu/gl/GrGLShaderBuilder.h',
      '<(skia_src_path)/gpu/gl/GrGLShaderVar.h',
      '<(skia_src_path)/gpu/gl/GrGLSL.cpp',
      '<(skia_src_path)/gpu/gl/GrGLSL.h',
      '<(skia_src_path)/gpu/gl/GrGLStencilBuffer.cpp',
      '<(skia_src_path)/gpu/gl/GrGLStencilBuffer.h',
      '<(skia_src_path)/gpu/gl/GrGLTexture.cpp',
      '<(skia_src_path)/gpu/gl/GrGLTexture.h',
      '<(skia_src_path)/gpu/gl/GrGLUtil.cpp',
      '<(skia_src_path)/gpu/gl/GrGLUtil.h',
      '<(skia_src_path)/gpu/gl/GrGLUniformManager.cpp',
      '<(skia_src_path)/gpu/gl/GrGLUniformManager.h',
      '<(skia_src_path)/gpu/gl/GrGLUniformHandle.h',
      '<(skia_src_path)/gpu/gl/GrGLVertexBuffer.cpp',
      '<(skia_src_path)/gpu/gl/GrGLVertexBuffer.h',
      '<(skia_src_path)/gpu/gl/GrGpuGL.cpp',
      '<(skia_src_path)/gpu/gl/GrGpuGL.h',
      '<(skia_src_path)/gpu/gl/GrGpuGL_program.cpp',
    ],
    'gr_native_gl_sources': [
      '<(skia_src_path)/gpu/gl/GrGLDefaultInterface_native.cpp',
      '<(skia_src_path)/gpu/gl/mac/GrGLCreateNativeInterface_mac.cpp',
      '<(skia_src_path)/gpu/gl/win/GrGLCreateNativeInterface_win.cpp',
      '<(skia_src_path)/gpu/gl/unix/GrGLCreateNativeInterface_unix.cpp',
      '<(skia_src_path)/gpu/gl/iOS/GrGLCreateNativeInterface_iOS.cpp',
      '<(skia_src_path)/gpu/gl/android/GrGLCreateNativeInterface_android.cpp',
    ],
    'gr_mesa_gl_sources': [
      '<(skia_src_path)/gpu/gl/mesa/GrGLCreateMesaInterface.cpp',
    ],
    'gr_angle_gl_sources': [
      '<(skia_src_path)/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',
    ],
    'gr_debug_gl_sources': [
      '<(skia_src_path)/gpu/gl/debug/GrGLCreateDebugInterface.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrFakeRefObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrBufferObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrBufferObj.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrFBBindableObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrRenderBufferObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrTextureObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrTextureObj.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrTextureUnitObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrTextureUnitObj.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrFrameBufferObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrFrameBufferObj.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrShaderObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrShaderObj.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrProgramObj.h',
      '<(skia_src_path)/gpu/gl/debug/GrProgramObj.cpp',
      '<(skia_src_path)/gpu/gl/debug/GrDebugGL.h',
      '<(skia_src_path)/gpu/gl/debug/GrDebugGL.cpp',
    ],
    'gr_null_gl_sources': [
      '<(skia_src_path)/gpu/gl/GrGLCreateNullInterface.cpp',
    ],

    'skgr_sources': [
      '<(skia_include_path)/gpu/SkGpuDevice.h',
      '<(skia_include_path)/gpu/SkGr.h',
      '<(skia_include_path)/gpu/SkGrPixelRef.h',
      '<(skia_include_path)/gpu/SkGrTexturePixelRef.h',

      '<(skia_include_path)/gpu/gl/SkGLContextHelper.h',

      '<(skia_src_path)/gpu/SkGpuDevice.cpp',
      '<(skia_src_path)/gpu/SkGr.cpp',
      '<(skia_src_path)/gpu/SkGrFontScaler.cpp',
      '<(skia_src_path)/gpu/SkGrPixelRef.cpp',
      '<(skia_src_path)/gpu/SkGrTexturePixelRef.cpp',

      '<(skia_src_path)/image/SkImage_Gpu.cpp',
      '<(skia_src_path)/image/SkSurface_Gpu.cpp',

      '<(skia_src_path)/gpu/gl/SkGLContextHelper.cpp'
    ],
    'skgr_native_gl_sources': [
      '<(skia_include_path)/gpu/gl/SkNativeGLContext.h',
      '<(skia_src_path)/gpu/gl/mac/SkNativeGLContext_mac.cpp',
      '<(skia_src_path)/gpu/gl/nacl/SkNativeGLContext_nacl.cpp',
      '<(skia_src_path)/gpu/gl/win/SkNativeGLContext_win.cpp',
      '<(skia_src_path)/gpu/gl/unix/SkNativeGLContext_unix.cpp',
      '<(skia_src_path)/gpu/gl/android/SkNativeGLContext_android.cpp',
      '<(skia_src_path)/gpu/gl/iOS/SkNativeGLContext_iOS.mm',
    ],
    'skgr_angle_gl_sources': [
      '<(skia_include_path)/gpu/gl/SkANGLEGLContext.h',
      '<(skia_src_path)/gpu/gl/angle/SkANGLEGLContext.cpp',
    ],
    'skgr_mesa_gl_sources': [
      '<(skia_include_path)/gpu/gl/SkMesaGLContext.h',
      '<(skia_src_path)/gpu/gl/mesa/SkMesaGLContext.cpp',
    ],
    'skgr_debug_gl_sources': [
      '<(skia_include_path)/gpu/gl/SkDebugGLContext.h',
      '<(skia_src_path)/gpu/gl/debug/SkDebugGLContext.cpp',
    ],
    'skgr_null_gl_sources': [
      '<(skia_include_path)/gpu/gl/SkNullGLContext.h',
      '<(skia_src_path)/gpu/gl/SkNullGLContext.cpp',
    ],
  },
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:

# GYP file to build unit tests.
{
  'includes': [
    'apptype_console.gypi',
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tests',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../src/gpu',
      ],
      'sources': [
        '../tests/AAClipTest.cpp',
        '../tests/BitmapCopyTest.cpp',
        '../tests/BitmapGetColorTest.cpp',
        '../tests/BitSetTest.cpp',
        '../tests/BlitRowTest.cpp',
        '../tests/BlurTest.cpp',
        '../tests/CanvasTest.cpp',
        '../tests/ClampRangeTest.cpp',
        '../tests/ClipCubicTest.cpp',
        '../tests/ClipStackTest.cpp',
        '../tests/ClipperTest.cpp',
        '../tests/ColorFilterTest.cpp',
        '../tests/ColorTest.cpp',
        '../tests/DataRefTest.cpp',
        '../tests/DequeTest.cpp',
        '../tests/DrawBitmapRectTest.cpp',
        '../tests/FillPathTest.cpp',
        '../tests/FlateTest.cpp',
        '../tests/GeometryTest.cpp',
        '../tests/GLInterfaceValidation.cpp',
        '../tests/GLProgramsTest.cpp',
        '../tests/InfRectTest.cpp',
        '../tests/MathTest.cpp',
        '../tests/MatrixTest.cpp',
        '../tests/Matrix44Test.cpp',
        '../tests/MetaDataTest.cpp',
        '../tests/PackBitsTest.cpp',
        '../tests/PaintTest.cpp',
        '../tests/ParsePathTest.cpp',
        '../tests/PathCoverageTest.cpp',
        '../tests/PathMeasureTest.cpp',
        '../tests/PathTest.cpp',
        '../tests/PDFPrimitivesTest.cpp',
        '../tests/PointTest.cpp',
        '../tests/QuickRejectTest.cpp',
        '../tests/Reader32Test.cpp',
        '../tests/ReadPixelsTest.cpp',
        '../tests/RefDictTest.cpp',
        '../tests/RegionTest.cpp',
        '../tests/ScalarTest.cpp',
        '../tests/ShaderOpacityTest.cpp',
        '../tests/Sk64Test.cpp',
        '../tests/skia_test.cpp',
        '../tests/SortTest.cpp',
        '../tests/SrcOverTest.cpp',
        '../tests/StreamTest.cpp',
        '../tests/StringTest.cpp',
        '../tests/Test.cpp',
        '../tests/Test.h',
        '../tests/TestSize.cpp',
        '../tests/ToUnicode.cpp',
        '../tests/UnicodeTest.cpp',
        '../tests/UtilsTest.cpp',
        '../tests/WArrayTest.cpp',
        '../tests/WritePixelsTest.cpp',
        '../tests/Writer32Test.cpp',
        '../tests/XfermodeTest.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'experimental.gyp:experimental',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'images.gyp:images',
        'ports.gyp:ports',
        'pdf.gyp:pdf',
        'utils.gyp:utils',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:

# ANGLE is the Windows-specific translator from OGL ES 2.0 to D3D 9

{
  'conditions': [
    [ 'skia_angle', {
      'target_defaults': {
        'defines': [
          'NOMINMAX',
        ],
      },
      'variables': {
        'component': 'static_library',
        'skia_warnings_as_errors': 0,
      },
      'includes': [
        '../../angle/src/build_angle.gypi',
      ],
    }],
  ],
  'targets': [
    {
      'target_name': 'angle',
      'type': 'none',
      'conditions': [
        [ 'skia_angle', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../../angle/include',
            ],
          },
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:

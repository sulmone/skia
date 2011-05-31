{
  'target_defaults': {
    'configurations': {
      'Debug': {
        'defines': [
          'SK_DEBUG',
          'GR_DEBUG=1',
        ],
      },
      'Release': {
        'defines': [
          'SK_RELEASE',
          'GR_RELEASE=1',
        ],
      },
    },
    'conditions': [
      [ 'OS == "linux" or OS == "freebsd" or OS == "openbsd" or OS == "solaris"', {
        'include_dirs' : [
          '/usr/include/freetype2',
        ],
      }],
      [ 'OS == "mac"', {
        'defines': [
          'SK_BUILD_FOR_MAC',
        ],
      }],
      [ 'OS == "win"', {
        'defines': [
          'SK_BUILD_FOR_WIN32',
          'SK_IGNORE_STDINT_DOT_H',
        ],
      }],
      [ 'OS == "linux"', {
        'defines': [
          'SK_SAMPLES_FOR_X',
          'SK_BUILD_FOR_UNIX',
        ],
      }],
    ],
    'direct_dependent_settings': {
      'conditions': [
        [ 'OS == "mac"', {
          'defines': [
            'SK_BUILD_FOR_MAC',
          ],
        }],
        [ 'OS == "win"', {
          'defines': [
            'SK_BUILD_FOR_WIN32',
          ],
        }],
      ],
    },
  },
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:

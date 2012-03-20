# Copyright 2012 The Android Open Source Project
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Depend on this wrapper to pick up libjpeg from third_party

{
  'targets': [
    {
      'target_name': 'libjpeg',
      'type': 'none',
      'dependencies': [
        '../third_party/externals/libjpeg/libjpeg.gyp:libjpeg',
      ],

    'conditions': [
      [ 'os_posix != 1 or OS == "mac"', {
        # need to set include path when using our own version
        'direct_dependent_settings': {
          'include_dirs': [
            '../third_party/externals/libjpeg',
          ],
         },
      },],
    ],


    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:

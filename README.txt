android-tools-unf2fs
====================

WIP



Dependencies
============

1. zlib
2. pcre2
3. lz4
4. googletest (optional with `-DANDROID_TOOLS_USE_BUNDLED_GTEST=ON`)
5. fmt (optional with `-DANDROID_TOOLS_USE_BUNDLED_FMT=ON`)



Installation
============

    $ mkdir build && cd build
    $ cmake ..
    $ make
    $ make install

pass `-DSTATIC_BUILD=ON` to cmake for static build



Tested Targets
==============

- Cygwin
- macOS
- Linux

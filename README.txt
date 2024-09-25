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

clone this project with `--recursive --shallow-submodules`
or run `git submodule update --init --depth 1`

    $ mkdir build && cd build
    $ cmake ..
    $ make
    $ make install

pass `-DSTATIC_BUILD=ON` to cmake for static build (Requires libXXX.a)
(to link shared libgcc and libc++, use `-DPARTIAL_STATIC=ON`)



Tested Targets
==============

- Cygwin
- macOS
- Linux

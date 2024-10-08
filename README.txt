android-tools-unf2fs
====================

a free drop-in replacement for f2fsUnpack for rom tools.
based on f2fs-tools; and android-tools for build system.



TODO
====

- LZ4 decompression support (coming soon)



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

(note: if u plan to build for Termux, import patches from:
 https://github.com/termux/termux-packages/tree/master/packages/android-tools
 or use Glibc (recommended): https://github.com/termux-pacman/glibc-packages

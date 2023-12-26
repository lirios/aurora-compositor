Aurora Compositor
=================

[![License](https://img.shields.io/badge/license-GPLv3.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![GitHub release](https://img.shields.io/github/release/lirios/aurora-compositor.svg)](https://github.com/lirios/aurora-compositor)
[![GitHub issues](https://img.shields.io/github/issues/lirios/aurora-compositor.svg)](https://github.com/lirios/aurora-compositor/issues)
[![CI](https://github.com/lirios/aurora-compositor/workflows/CI/badge.svg?branch=develop)](https://github.com/lirios/aurora-compositor/actions?query=workflow%3ACI)

Framework to develop QtQuick-based Wayland compositors.

## Dependencies

Qt >= 6.6.0 with at least the following modules is required:

 * [qtbase](http://code.qt.io/cgit/qt/qtbase.git)
 * [qtdeclarative](http://code.qt.io/cgit/qt/qtdeclarative.git)

The following modules and their dependencies are required:

 * [cmake](https://gitlab.kitware.com/cmake/cmake) >= 3.19.0
 * [extra-cmake-modules](https://invent.kde.org/frameworks/extra-cmake-modules) >= 5.245.0
 * [cmake-shared](https://github.com/lirios/cmake-shared.git) >= 2.0.99

## Installation

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
make
make install # use sudo if necessary
```

Replace `/path/to/prefix` to your installation prefix.
Default is `/usr/local`.

## Licensing

Licensed under the terms of the GNU General Public License version 3 or,
at your option, any later version.

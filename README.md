# Simulation of primary visual cortex
This is fork of [V1stdp written by Thomas Miconi](https://github.com/ThomasMiconi/V1stdp).
See also [original README.md](README-original.md) and original article (["Spontaneous emergence of fast attractor dynamics in a model of developing primary visual cortex"](https://doi.org/10.1038/ncomms13208)).

# Dependencies
## Build dependencies
### Tools
- [CMake](https://cmake.org/): `v3.25.1<=`
  - CMake is meta-builder, so other building system, like [GNU Makefile](https://www.gnu.org/software/make/manual/make.html) or [Ninja](https://ninja-build.org/), is needed.

### Compiler
This program uses C++20.
- [GCC](https://gcc.gnu.org/): `v11.2.0<=`
- [clang](https://clang.llvm.org/): `v17.0.6<=`

### Library
- [Boost](https://www.boost.org/): `v1.80.0<=`
- [CLI11](https://cliutils.github.io/CLI11/book/): `v2.3.2<=`
- [Eigen3](https://eigen.tuxfamily.org/): `v3.4.0<=`
- [GoogleTest](https://google.github.io/googletest/): `v1.12.1<=`
  - Only needed for running unit test
- `rm`, `diff`, `mkdir`:

## Script dependencies
- [gnuplot](http://www.gnuplot.info/)
  - To export images

## Workflow dependencies
- [ccwl](https://ccwl.systemreboot.net/)
  - `ccwl` is generator for Common Workflow Language (CWL) files, so CWL runner, like [cwltool](https://github.com/common-workflow-language/cwltool), is needed.
- `rsvg-convert` in [librsvg](https://wiki.gnome.org/Projects/LibRsvg)
  - To convert svg to png

# Build
```shell
cmake -B build -DCMAKE_BUILD_RPATH=$LIBRARY_PATH # -DCMAKE_BUILD_RPATH=$LIBRARY_PATH is only needed to find googletest on test runtime
cmake --build build
```

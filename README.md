# LIBYASVG

## Abstract
LibYASG is yet another SVG library that is able to import SVG files to a [Cairo](https://www.cairographics.org/) canvas. The main motivation for this software was to produce a lightweight alternative to the generally used [librsvg](https://github.com/GNOME/librsvg) library.

## Compilation
LIBYASVG is compiled using cmake.

Create build directory
```
mkdir build
cd build
```

Create Makefile
```
cmake ../src
```

Compile
```
make -j5
```

Execute
```
./svg2cairo
```
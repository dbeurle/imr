[![Build Status](https://travis-ci.org/dbeurle/GmshReader.svg?branch=master)](https://travis-ci.org/dbeurle/GmshReader)
[![Coverage Status](https://coveralls.io/repos/github/dbeurle/GmshReader/badge.svg?branch=master)](https://coveralls.io/github/dbeurle/GmshReader?branch=master)
# GmshReader
A reader for Gmsh files for processing serial and decomposed grids

![alt text](https://github.com/dbeurle/GmshReader/blob/master/doc/images/feti.png "FETI example")

# Building

This project uses CMake as the build system and can be built out-of-tree using:
* `mkdir build && cd build`
* `cmake ..`
* `make all`

# Gmsh file format

Gmsh specifies information tags and the nodal connectivity for each element.  When a mesh is partitioned, for example with two partitions, the information tags would have the following data:

| 10 | 2 | 5 | 1 | 6 | 2 | 2 | -1 | 5 22 4 |
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| element # | element type (Triangle3) | tags | physical # | geometrical # | processes | owner | ghost | `nodalConnectivity` |

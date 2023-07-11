#!/bin/bash

autoreconf -i
./configure CFLAGS=-I/${CONDA_PREFIX}/include LDFLAGS="-L/${CONDA_PREFIX}/lib -Wl,-rpath,$CONDA_PREFIX/lib" --prefix=$CONDA_PREFIX
make
make install

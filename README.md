  # nchash
  A C-utility for hashing NetCDF variables

  Dependencies
  * C compiler (clang for MacOS)
  * libarchive
  * libnetcdf
  * libcrypto
  

## Installation Instructions
```
autoreconf -i
./configure --prefix=<your install dir>
make
make install
```
### Note for Anaconda Users

If you are using libraries from Anaconda, the recommendation is to use the shared libraries from
the conda environment. Use the following options to `./configure`:
```
./configure CFLAGS=-I/${CONDA_PREFIX}/include LDFLAGS="-L/${CONDA_PREFIX}/lib -Wl,-rpath,$CONDA_PREFIX/lib"
```

## Usage Instructions

```
nchash <options> file

Options:
-c     Colorized output
-h     Hash algorithm 'md5', 'sha1', or 'sha256' (default)
file   Either a NetCDF file or a tar file that contains multiple NetCDF files
```

## Example
```
nchash -h md5 ocean_hgrid.nc
ocean_hgrid.nc tile: 93fa583c304560eb25b156a9f7368c70
ocean_hgrid.nc y: 88ee0026c1e6315441c63fe6e7772199
ocean_hgrid.nc x: 0d4bd34bb1afd4346fd4a9e80bea354a
ocean_hgrid.nc dy: 5a8fe9eedc30d99a6b745e02e7b5b6c3
ocean_hgrid.nc dx: 813f488487ca2308fe370588d5b38f45
ocean_hgrid.nc area: f1797c67c6af7d1334689f6e14dda51a
ocean_hgrid.nc angle_dx: 2ca36fcbdede94f70b0a8371e97fe98f
```

## To-Do
* Package for distribution via Anaconda cloud / conda-forge

### Acknowledgements
StackOverflow and ChatGPT contributed significantly to this effort.

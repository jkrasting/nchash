  # nchash
  A C-utility for hashing NetCDF variables

  Dependencies
  * C compiler
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
./configure CFLAGS=-I/${CONDA_PREFIX}/include LDFLAGS=-L/${CONDA_PREFIX}/lib
```

## Usage Instructions

```
nchash <options> file.nc

Options:
-c     Colorized output
-a     Hash algorithm.  `m`=MD5, `s`=SHA-1, `2`=SHA-2 (default)
```

## Example
```
nchash -a m ~/app/ocean_hgrid.nc
ocean_hgrid.nc: tile       93fa583c304560eb25b156a9f7368c70
ocean_hgrid.nc: y          f5871dbc20503e25f612a4aeb22da671
ocean_hgrid.nc: x          1f3658df001d9e486db46da74ac4f10b
ocean_hgrid.nc: dy         7d2e042a6ce777d705cef146ff88940f
ocean_hgrid.nc: dx         77042199cf81d8a41ffcf3728a9e553d
ocean_hgrid.nc: area       590755f3a702a31600338d700c55c91f
ocean_hgrid.nc: angle_dx   6467debe279e2d91a6e22c0eb93c715a
```

## To-Do
* Package for distribution via Anaconda cloud / conda-forge

### Acknowledgements
StackOverflow and ChatGPT contributed significantly to this effort.

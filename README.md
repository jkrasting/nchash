  nchash
  ======
  A C-utility for hashing NetCDF variables

  Dependencies
  * libnetcdf
  * libcrypto
  

Installation Instructions
-------------------------
```
autoreconf -i
./configure --prefix=<your install dir>
make
make install
```
Note
----

If you are using libraries from Anaconda, activate your environment and set the following:
```
CPATH=${CONDA_PREFIX}/include
LIBRARY_PATH=${CONDA_PREFIX}/lib
```

AC_INIT([nchash], [1.0], [your-email@example.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CHECK_LIB([netcdf], [nc_open])
AC_CHECK_LIB([archive], [archive_read_data_block])
AC_CHECK_LIB([crypto], [SHA256_Init])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

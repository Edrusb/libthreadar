# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.69)
AC_INIT([LIBTHREADAR], [1.0.0], [])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])
AC_CONFIG_SRCDIR([tampon.cpp])

AM_INIT_AUTOMAKE
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION
AM_ICONV

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_MSG_CHECKING([for C++ compiler usability])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
                                  [ class test { public: int test; }; ])],
                  [AC_MSG_RESULT(ok)],
                  [AC_MSG_ERROR([No C++ compiler found])])

# Checks for libraries.


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT


AC_CHECK_HEADERS([sys/types.h sys/stat.h fcntl.h string.h errno.h pthread.h signal.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_HEADER_MAJOR
AC_TYPE_SIGNAL

AC_MSG_CHECKING([for sed -r/-E option])
if sed -r -e 's/(c|o)+/\1/g' > /dev/null < /dev/null ; then
    local_sed="gnu"
    AC_MSG_RESULT([GNU sed, using -r option for regex])
else
   if sed -E -e 's/(c|o)+/\1/g' > /dev/null < /dev/null ; then
     local_sed="bsd"
     AC_MSG_RESULT([BSD sed, using -E option for regex])
   else
     local_sed=unknown
     AC_MSG_ERROR([unknown switch to use with sed to support regex])
   fi
fi


AM_CONDITIONAL([BSD_SED], [test "$local_sed" = "bsd"])

# defaults
AC_PREFIX_DEFAULT(/usr/local)

# hack from litool mailing-list to know from source point of view whether we are compiling for dynamic or static way

AC_CONFIG_COMMANDS([hack-libtool], [
				    sed 's,^pic_flag=,pic_flag=" -D__DYNAMIC__ ",' libtool > libtoolT \
				    && mv -f libtoolT libtool && chmod 755 libtool
])


AC_CONFIG_FILES([Makefile libthreadar.pc.tmpl])

AC_OUTPUT

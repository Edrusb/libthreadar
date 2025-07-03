#######################################################################
## libthreadar - is a library providing several C++ classes to work with threads
## Copyright (C) 2014-2025 Denis Corbin
##
## This file is part of libthreadar
##
##  libthreadar is free software: you can redistribute it and/or modify
##  it under the terms of the GNU Lesser General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  libhtreadar is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public License
##  along with libthreadar.  If not, see <http:##www.gnu.org/licenses/>
##
######
##  to contact the author: dar.linux@free.fr
#######################################################################

# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.71)
AC_INIT([LIBTHREADAR], [1.6.0], [])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])
AC_CONFIG_SRCDIR([src/tampon.hpp])

AM_INIT_AUTOMAKE([subdir-objects])
# AM_GNU_GETTEXT([external])
# AM_GNU_GETTEXT_VERSION
# AM_ICONV

# have a specific variable for pkgconfig, setting the default value:
AC_SUBST(pkgconfigdir, [${libdir}/pkgconfig])
AC_ARG_WITH([pkgconfigdir],
            AS_HELP_STRING([--with-pkgconfigdir=DIR],[defines an alternative directory to install pkconfig files, default is '${libdir}/pkgconfig']),
            [
               if [ ! -z "$withval" ] ; then
                 AC_SUBST(pkgconfigdir, $withval)
               fi
            ],
            []
           )

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
LT_INIT
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_MSG_CHECKING([for C++ compiler usability])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
                                  [ class test { public: int test; }; ])],
                  [AC_MSG_RESULT(ok)],
                  [AC_MSG_ERROR([No C++ compiler found])])

AC_MSG_CHECKING([for c++11 support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
                                   [ thread_local static int test = 0; ])
                  ],
                  [
		      AC_SUBST(cxxstdflags, [])
		      AC_MSG_RESULT(yes)
		  ],
                  [
                     AC_MSG_RESULT([no])
                     AC_MSG_CHECKING([for c++ support with -std=c++11 option set])
                     CXXSTDFLAGS="-std=c++11"
                     CXXFLAGS="$CXXFLAGS $CXXSTDFLAGS"
                     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
                                                        [ thread_local static int test = 0; ])
                                        ],
                                        [
					   AC_SUBST(cxxstdflags, $CXXSTDFLAGS)
					   AC_MSG_RESULT(yes)
					],
                                        [
                                           AC_MSG_RESULT(no)
                                           AC_MSG_ERROR([C++ compiler lack support for c++11 standard])
                                        ]
                     )
                   ])


# Checks for libraries.
AC_CHECK_LIB(pthread, [pthread_mutex_init], [], [])


# Checks for header files.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP
AC_HEADER_SYS_WAIT


AC_CHECK_HEADERS([sys/types.h sys/stat.h fcntl.h string.h errno.h pthread.h signal.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_HEADER_MAJOR

AC_CHECK_FUNCS([strerror_r])

AC_MSG_CHECKING([for strerror_r flavor])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[extern "C"
                                     {
                                     #if HAVE_STRING_H
                                     #include <string.h>
                                     #endif
                                     } // extern "C"
                                   ]],
                                   [[
                                        char *ptr = strerror_r(0, 0, 0);
                                   ]])
                  ],
                  [ AC_DEFINE(HAVE_STRERROR_R_CHAR_PTR, 1, [strerror_r() returns a char *])
                    AC_MSG_RESULT([GNU specific])
                  ],
                  [
                    AC_MSG_RESULT([XSI compliant])
                  ])

AC_MSG_CHECKING([for pthread_barrier_t availability])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[extern "C"
				   {
				   #if HAVE_PTHREAD_H
				   #include <pthread.h>
				   #endif
				   } // extern "C"
				   ]],
				   [[
					pthread_barrier_t demo;

					(void)pthread_barrier_init(&demo, NULL, 1);
					(void)pthread_barrier_destroy(&demo);
				   ]])
		 ],
		 [
		   AC_DEFINE(HAVE_PTHREAD_BARRIER_T, 1, [pthread_barrier_t availability])
		   AC_MSG_RESULT([yes])
		 ],
		 [
		   AC_DEFINE(HAVE_PTHREAD_BARRIER_T, 0, [pthread_barrier_t availability])
		   AC_MSG_RESULT([absent! will emulate barrier using pthead_cond_t])
		 ])

AC_MSG_CHECKING([for sed -r/-E option])
if sed -r -e 's/(c|o)+/\1/g' > /dev/null < /dev/null ; then
    local_sed="-r"
    AC_MSG_RESULT([GNU sed, using -r option for regex])
else
   if sed -E -e 's/(c|o)+/\1/g' > /dev/null < /dev/null ; then
     local_sed="-E"
     AC_MSG_RESULT([BSD sed, using -E option for regex])
   else
     local_sed=""
     AC_MSG_ERROR([unknown switch to use with sed to support regex])
   fi
fi
AC_SUBST(SED_REGEX, [${local_sed}])

AC_ARG_ENABLE( [build-html],
               AS_HELP_STRING([--disable-build-html],[don't build programming documentation]),
               [ doxygen="no"
                 groff="no"
               ],
               [ AC_CHECK_PROG(doxygen, doxygen, [yes], [no], [$PATH])
                   AC_MSG_CHECKING([for doxygen version])
                   if test "$doxygen" = "yes" ; then
                       n1=`doxygen --version | cut -d '.' -f 1`
                       n2=`doxygen --version | cut -d '.' -f 2`
                       if test $n1 -gt 1 -o $n2 -ge 3 ; then
                           AC_MSG_RESULT([ >= 1.3])
                       else
                           AC_MSG_RESULT([ too old (< 1.3) ignoring doxygen])
                           doxygen="no"
                       fi
                   fi
                 AC_CHECK_PROG(tmp, man, [yes], [no], [$PATH])
                 if test "$tmp" = "yes" ; then
                    AC_CHECK_PROG(groff, groff, [yes], [no], [$PATH])
                 else
                    groff = "no";
                 fi
               ]
             )

threadar_major=`echo AC_PACKAGE_VERSION | [ sed ${local_sed} -n -e 's/^([0-9]+)\..*/\1/p']`
threadar_medium=`echo AC_PACKAGE_VERSION | [ sed ${local_sed} -n -e 's/^[0-9]+\.([0-9]+)\..*/\1/p']`
threadar_minor=`echo AC_PACKAGE_VERSION | [ sed ${local_sed} -n -e 's/^[0-9]+\.[0-9]+\.([0-9]+).*/\1/p' ]`

# for makefiles.in
AC_SUBST(LIBTHREADAR_MAJOR, ${threadar_major})
AC_SUBST(LIBTHREADAR_MEDIUM, ${threadar_medium})
AC_SUBST(LIBTHREADAR_MINOR, ${threadar_minor})

# for macro in source code
AC_DEFINE_UNQUOTED(LIBTHREADAR_MAJOR, ${threadar_major}, [major version of libthreadar])
AC_DEFINE_UNQUOTED(LIBTHREADAR_MEDIUM, ${threadar_medium}, [medium version of libthreadar])
AC_DEFINE_UNQUOTED(LIBTHREADAR_MINOR, ${threadar_minor}, [minor version of libthreadar])

AM_CONDITIONAL([USE_DOXYGEN], [test "$doxygen" = "yes"])
AC_SUBST(DOXYGEN_PROG, [doxygen])
AC_SUBST(THREADAR_VERSION, AC_PACKAGE_VERSION)

# defaults
AC_PREFIX_DEFAULT(/usr/local)

# hack from libtool mailing-list to know from source point of view whether we are compiling for dynamic or static way

AC_CONFIG_COMMANDS([hack-libtool], [
				    sed 's,^pic_flag=,pic_flag=" -D__DYNAMIC__ ",' libtool > libtoolT \
				    && mv -f libtoolT libtool && chmod 755 libtool
])


AC_CONFIG_FILES([Makefile src/Makefile src/libthreadar.pc.tmpl doc/Makefile doc/examples/Makefile])
AC_OUTPUT

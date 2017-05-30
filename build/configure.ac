# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.69)
AC_INIT([LIBTHREADAR], [1.0.2], [])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])
AC_CONFIG_SRCDIR([src/tampon.hpp])

AM_INIT_AUTOMAKE
# AM_GNU_GETTEXT([external])
# AM_GNU_GETTEXT_VERSION
# AM_ICONV



# have a specific variable for pkgconfig, setting the default value:
AC_SUBST(pkgconfigdir, [${libdir}/pkgconfig])
AC_ARG_WITH([pkgconfigdir],
            AC_HELP_STRING(--with-pkgconfigdir=DIR, [defines an alternative directory to install pkconfig files, default is '${libdir}/pkgconfig']),
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
AC_PROG_LIBTOOL
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

AC_ARG_ENABLE( [build-html],
               AC_HELP_STRING(--disable-build-html, [don't build programming documentation]),
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

AM_CONDITIONAL([BSD_SED], [test "$local_sed" = "bsd"])
AM_CONDITIONAL([USE_DOXYGEN], [test "$doxygen" = "yes"])
AC_SUBST(DOXYGEN_PROG, [doxygen])



# defaults
AC_PREFIX_DEFAULT(/usr/local)

# hack from litool mailing-list to know from source point of view whether we are compiling for dynamic or static way

AC_CONFIG_COMMANDS([hack-libtool], [
				    sed 's,^pic_flag=,pic_flag=" -D__DYNAMIC__ ",' libtool > libtoolT \
				    && mv -f libtoolT libtool && chmod 755 libtool
])


AC_CONFIG_FILES([Makefile src/Makefile src/libthreadar.pc.tmpl doc/Makefile examples/Makefile])
AC_OUTPUT

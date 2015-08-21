AC_PREREQ([2.68])
AC_COPYRIGHT([2014 elemental-ircd Team])

AC_INIT([elemental-ircd],[6.6.2],[https://github.com/Elemental-IRCd/elemental-ircd/issues])
AC_CONFIG_MACRO_DIR([libratbox/m4])
AC_CONFIG_AUX_DIR([libratbox/tools])
AC_CONFIG_SRCDIR([src/ircd.c])
AC_CONFIG_HEADERS([include/setup.h])
AC_PREFIX_DEFAULT($HOME/ircd)
AC_LANG(C)

AC_ENABLE_SHARED
AC_DISABLE_STATIC

AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR

AC_PROG_CC_C99([], [AC_MSG_ERROR(elemental-ircd requires a C99 capable compiler)])

LT_INIT([dlopen])

LT_CONFIG_LTDL_DIR([extra/libltdl])
LTDL_INIT

build_ltdl=$with_included_ltdl
AM_CONDITIONAL([BUILD_LTDL], [test x"$build_ltdl" = x"yes"])

GCC_STACK_PROTECT_CC

dnl Checks for programs.
CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1 $CFLAGS"

AC_CONFIG_SUBDIRS(libratbox)

dnl We prefer gcc -MM because it's a lot less bloated
AC_PATH_PROG(MKDEP, mkdep)
AC_PATH_PROG(MAKEDEPEND, makedepend)

AC_MSG_CHECKING(how to generate dependency info)

STDOUT="> .depend"

if test "$ac_cv_c_compiler_gnu" = yes; then
	AC_MSG_RESULT(gcc -MM)
	MKDEP="$CC -MM"
elif test ! -z "$MKDEP"; then
	AC_MSG_RESULT(mkdep)
elif test ! -z "$MAKEDEPEND"; then
	AC_MSG_RESULT(makedepend)
	MKDEP="$MAKEDEPEND -f-"
else
	AC_MSG_RESULT([nothing suitable.. forget it!])
	MKDEP=":"
fi

AC_SUBST(MKDEP)

dnl check for /dev/null so we can use it to hold evil fd's
AC_MSG_CHECKING([for /dev/null])
if test -c /dev/null ; then
	AC_DEFINE(PATH_DEVNULL, "/dev/null", [Path to /dev/null])
	AC_MSG_RESULT(yes)
else
	AC_DEFINE(PATH_DEVNULL, "devnull.log", [Path to /dev/null])
	AC_MSG_RESULT(no - using devnull.log)
fi

AC_SEARCH_LIBS([strerror],[cposix])
AC_C_INLINE
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_GCC_TRADITIONAL
AC_PROG_MAKE_SET
AC_PROG_SED
AC_PROG_YACC
AC_PROG_LEX


dnl AC_PROG_YACC defaults to yacc unconditionally if nothing can be found
if test "$YACC" = "yacc" -a -z "`which $YACC 2>/dev/null`"; then
	AC_MSG_ERROR([could not locate a suitable parser generator; install bison, yacc, or byacc])
fi

if test "$LEX" = ":"; then
	AC_MSG_ERROR([could not locate a suitable lexical generator, install flex or lex.])
fi

AC_ARG_ENABLE([fhs-paths],
	[AS_HELP_STRING([--enable-fhs-paths], [Use more FHS-like pathnames (for packagers).])],
	[],
	[dnl detect if the user appears to want --enable-fhs-paths
	AS_IF([test "$libexecdir" = '${exec_prefix}/libexec' && \
		test "$localstatedir" = '${prefix}/var' && \
		test "$libdir" = '${exec_prefix}/lib'],
		[enable_fhs_paths=no],
		[enable_fhs_paths=yes])
	])
dnl use directory structure of cached as default (hack)
AS_IF([test "x$enable_fhs_paths" = "xyes"],
	[dnl Avoid name collisions.
	pkglibexecdir='${libexecdir}/${PACKAGE_TARNAME}'
	rundir=${rundir-'${prefix}/run'}
	pkgrundir='${rundir}/${PACKAGE_TARNAME}'
	pkglocalstatedir='${localstatedir}/${PACKAGE_TARNAME}'],
	[libexecdir='${bindir}'
	pkglibexecdir='${libexecdir}'
	rundir='${sysconfdir}'
	pkgrundir='${rundir}'
	localstatedir='${prefix}'
	pkglocalstatedir='${sysconfdir}'])
pkglibdir='${libdir}/${PACKAGE_TARNAME}'
AC_SUBST([pkglibdir])
AC_SUBST([rundir])
AC_SUBST([pkgrundir])
AC_SUBST([pkglocalstatedir])
AC_DEFINE_DIR([PKGLOCALSTATEDIR], [pkglocalstatedir], [[Directory in which to store state, such as ban database]])
AC_SUBST([pkglibexecdir])
AC_DEFINE_DIR([PKGLIBEXECDIR], [pkglibexecdir], [Directory where binaries the IRCd itself spawns live])

dnl Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADERS([crypt.h sys/resource.h sys/param.h errno.h sys/syslog.h stddef.h sys/wait.h wait.h sys/epoll.h sys/uio.h machine/endian.h])

dnl Stuff that the memory manager (imalloc) depends on
dnl ==================================================

AC_C_CONST
AC_C_BIGENDIAN
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

dnl Needed for bandb
PKG_CHECK_MODULES([SQLITE], [sqlite3 > 3.0.0])

dnl Networking Functions
dnl ====================

AC_ARG_ENABLE(ipv6,
AS_HELP_STRING([--enable-ipv6],[Enable IPv6 support]),[ipv6=$enableval],[ipv6=no])

AC_SEARCH_LIBS(crypt, [crypt descrypt],,)

CRYPT_LIB=$ac_cv_search_crypt

if test "$CRYPT_LIB" = "none required"; then
	unset CRYPT_LIB
elif test "$CRYPT_LIB" = no; then
	unset CRYPT_LIB
fi

AC_SUBST(CRYPT_LIB)

if test x"$ac_cv_header_sys_wait_h" = yes -o x"$ac_cv_header_wait_h" = yes; then
	VICONF=yes
	dnl We need one of the above to build viconf. Just a sanity check,
	dnl we don't want to stop people from building the rest of ircd
	dnl just because they can't build viconf.
fi

AM_CONDITIONAL([BUILD_VICONF],[test x"$VICONF" = x"yes"])

dnl See whether we can include both string.h and strings.h.
AC_CACHE_CHECK([whether string.h and strings.h may both be included],
gcc_cv_header_string,
[
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
		#include <string.h>
		#include <strings.h>
	])],
	[gcc_cv_header_string=yes],
	[gcc_cv_header_string=no])
])

if test "$gcc_cv_header_string" = "yes"; then
	AC_DEFINE(STRING_WITH_STRINGS, 1, [Define to 1 if string.h may be included along with strings.h])
fi

AC_C_BIGENDIAN

dnl Check for stdarg.h - if we can't find it, halt configure
AC_CHECK_HEADER(stdarg.h, , [AC_MSG_ERROR([** stdarg.h could not be found - charybdis will not compile without it **])])

AC_CHECK_FUNCS([strlcat strlcpy])

AC_CHECK_TYPE([u_int32_t], [],
[
	AC_CHECK_TYPE([uint32_t],
	[
		AC_DEFINE(u_int32_t, [uint32_t], [If system does not define u_int32_t, define a reasonable substitute.])
	],
	[
		AC_MSG_WARN([system has no u_int32_t or uint32_t, default to unsigned long int])
		AC_DEFINE(u_int32_t, [unsigned long int], [If system does not define u_int32_t, define to unsigned long int here.])
	])
])

AC_CHECK_TYPE([u_int16_t], [],
[
	AC_CHECK_TYPE([uint16_t],
	[
		AC_DEFINE(u_int16_t, [uint16_t], [If system does not define u_int16_t, define a usable substitute])
	],
	[
		AC_MSG_WARN([system has no u_int16_t or uint16_t, default to unsigned short int])
		AC_DEFINE(u_int16_t, [unsigned short int], [If system does not define u_int16_t, define a usable substitute.])
	])
])

AC_CHECK_TYPES([uintptr_t])

dnl check for various functions...
AC_CHECK_FUNCS([socketpair vsnprintf mmap gettimeofday strdup strndup ])

dnl Specialized functions checks
dnl ============================

dnl check for nanosleep          
AC_CHECK_FUNC(nanosleep,,[AC_CHECK_LIB(rt,nanosleep,
                 LIBS="${LIBS} -lrt",
                 [AC_CHECK_LIB(posix4,nanosleep, LIBS="${LIBS} -lposix4"
                 )])])
if test x$ac_cv_func_nanosleep = xno && test x$ac_cv_lib_posix4_nanosleep = xno  && test x$ac_cv_lib_rt_nanosleep = xno
then     
         AC_MSG_RESULT("nanosleep not found..using select for delay")
else     
	 AC_DEFINE([HAVE_NANOSLEEP], 1, [Define if nanosleep exists])
fi

dnl Specialized functions and libraries
dnl ===================================

AC_ARG_WITH(zlib-path,
AS_HELP_STRING([--with-zlib-path=DIR],[Path to libz.so for ziplinks support.]),
[LIBS="$LIBS -L$withval"],)

AC_ARG_ENABLE(zlib,
AS_HELP_STRING([--disable-zlib],[Disable ziplinks support]),
[zlib=$enableval],[zlib=yes])

if test "$zlib" = yes; then

AC_CHECK_HEADER(zlib.h, [
	AC_CHECK_LIB(z, zlibVersion,
	[
		AC_SUBST(ZLIB_LD, -lz)
		AC_DEFINE(HAVE_LIBZ, 1, [Define to 1 if zlib (-lz) is available.])
	], zlib=no)
], zlib=no)

fi

dnl **********************************************************************
dnl Check for --with-confdir [deprecated, use --sysconfdir instead]
dnl **********************************************************************

AC_ARG_WITH([confdir],
	[AS_HELP_STRING([--with-confdir=DIR],[Directory to install config files [deprecated, use --sysconfdir instead].])],
	[ sysconfdir=`echo $withval | sed 's/\/$//'` ],
	[ confdir='${sysconfdir}' ])
AC_DEFINE_DIR([ETC_DIR], [sysconfdir], [Prefix where config files are installed.])

dnl **********************************************************************
dnl Check for --with-logdir
dnl **********************************************************************

AC_MSG_CHECKING([whether to modify logdir])
AC_ARG_WITH(logdir, 
AS_HELP_STRING([--with-logdir=DIR],[Directory where to write logfiles.]),
	       [ logdir=`echo $withval | sed 's/\/$//'`
		 AC_MSG_RESULT(yes)],
	       [ AS_IF([test "x$enable_fhs_paths" = "xyes"],
			[logdir='${localstatedir}/log/${PACKAGE_TARNAME}'],
			[logdir='${prefix}/logs'])
		 AC_MSG_RESULT(no)])
AC_DEFINE_DIR([LOG_DIR], [logdir], [Prefix where to write logfiles.])
AC_SUBST_DIR([logdir])

dnl **********************************************************************
dnl Check for --with-helpdir
dnl **********************************************************************

AC_MSG_CHECKING([whether to modify helpdir])
AC_ARG_WITH(helpdir, 
AS_HELP_STRING([--with-helpdir=DIR],[Directory to install help files.]),
	       [ helpdir=`echo $withval | sed 's/\/$//'`
		 AC_MSG_RESULT(yes) ],
	       [ AS_IF([test "x$enable_fhs_paths" = "xyes"],
			[helpdir='${datadir}/${PACKAGE_TARNAME}/help'],
			[helpdir='${prefix}/help'])
		 AC_MSG_RESULT(no) ])
AC_DEFINE_DIR([HELP_DIR], [helpdir], [Prefix where help files are installed.])
AC_SUBST_DIR([helpdir])

dnl **********************************************************************
dnl Check for --with-moduledir
dnl **********************************************************************

AC_MSG_CHECKING([whether to modify moduledir])
AC_ARG_WITH(moduledir, 
	[AS_HELP_STRING([--with-moduledir=DIR],[Directory to install modules.])],
	       [ moduledir=`echo $withval | sed 's/\/$//'`
		 AC_MSG_RESULT(yes)],
	       [ AS_IF([test "x$enable_fhs_paths" = "xyes"],
			[moduledir='${pkglibdir}/modules'],
			[moduledir='${prefix}/modules'])
		 AC_MSG_RESULT(no)
		])
AC_DEFINE_DIR(MODULE_DIR, moduledir, [Prefix where modules are installed.])
AC_SUBST_DIR([moduledir])

dnl Check for --with-rundir

AC_MSG_CHECKING([whether or modify rundir])
AC_ARG_WITH([rundir],
	[AS_HELP_STRING([--with-rundir=DIR],[Directory in which to store pidfile.])],
	[AC_MSG_RESULT([yes])
	rundir=`echo $withval | sed 's/\/$//'`],
	[AC_MSG_RESULT([no])
	AS_IF([test "x$enable_fhs_paths" = "xyes"],
		[rundir='${prefix}/run'],
		[rundir='${sysconfdir}'])])
AC_SUBST([rundir])
AC_DEFINE_DIR([PKGRUNDIR], [pkgrundir], [Directory to store pidfile in.])

dnl Installed utility program prefixes (does not affect binaries
dnl installed into pkglibexecdir)
AC_MSG_CHECKING([for program prefix])
AC_ARG_WITH([program-prefix],
	[AS_HELP_STRING([--with-program-prefix=], [If set, programs installed into PATH will be installed with names prefixed by this prefix.])],
	[test "x$with_program_prefix" = "xno" && with_program_prefix=],
	[with_program_prefix=])
AC_MSG_RESULT(["$with_program_prefix"])
PROGRAM_PREFIX="$with_program_prefix"
AC_SUBST([PROGRAM_PREFIX])
AC_DEFINE_UNQUOTED([PROGRAM_PREFIX], ["$with_program_prefix"], [String with which all programs intended to be in PATH are prefixed.])


dnl Debug-related options
dnl =====================

dnl Debug-related options
dnl =====================

AC_ARG_ENABLE(assert, AS_HELP_STRING([--enable-assert],[Enable assert(). Choose between soft(warnings) and hard(aborts the daemon)]))
AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug],[Enable debug build, overrides --enable-assert]),[],[enable_debug=no])

if test x"$enable_debug" != x"no"; then
	AX_APPEND_COMPILE_FLAGS([-g])
	AX_APPEND_COMPILE_FLAGS([-Og])
	AX_APPEND_COMPILE_FLAGS([-fsanitize=address])
	AX_APPEND_COMPILE_FLAGS([-fsanitize=undefined])
	enable_assert="hard";
fi;

if test x"$enable_assert" = x"no"; then
	AC_DEFINE(NDEBUG, 1, [Define this to disable debugging support.])
elif test x"$enable_assert" = x"soft"; then
	AC_DEFINE(SOFT_ASSERT, 1, [Define this to enable soft asserts.])
	AC_DEFINE(NDEBUG, 1, [Define this to disable debugging support.])
elif test x"$enable_assert" = x"yes"; then
	enable_assert="hard";
fi

AC_MSG_CHECKING(if you want IO Debugging hooks)
AC_ARG_ENABLE(iodebug,
AS_HELP_STRING([--enable-iodebug],[Enable IO Debugging hooks]),
[iodebug=$enableval], [iodebug=no])

if test "$iodebug" = yes; then
	AC_DEFINE(USE_IODEBUG_HOOKS, 1, [Define this to enable IO Debug hooks.])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(profile,
AS_HELP_STRING([--enable-profile],[Enable profiling]),
[profile=$enableval], [profile=no])

if test "$profile" = yes; then
	AX_CHECK_COMPILE_FLAG([-pg], [AX_APPEND_FLAG([-pg])], [
		AC_MSG_ERROR([Profiling not supported on this compiler])
	])
fi

AC_ARG_ENABLE(balloc,
AS_HELP_STRING([--disable-balloc],[Disable the block allocator.]),
[balloc=$enableval], [balloc=yes])

if test "$balloc" = no; then
	AC_DEFINE([NOBALLOC], 1, [Define to 1 if you wish to disable the block allocator.])
fi

AC_ARG_ENABLE(small-net,
AS_HELP_STRING([--disable-small-net],[Disable small network support.]),
[small_net=$enableval], [small_net=yes])

if test "$small_net" = yes; then
	AC_DEFINE([NICKNAMEHISTORYLENGTH], 1500, [Size of the WHOWAS array.])
	AC_DEFINE([CHANNEL_HEAP_SIZE], 256, [Size of the channel heap.])
	AC_DEFINE([BAN_HEAP_SIZE], 128, [Size of the ban heap.])
	AC_DEFINE([CLIENT_HEAP_SIZE], 256, [Size of the client heap.])
	AC_DEFINE([LCLIENT_HEAP_SIZE], 128, [Size of the local client heap.])
	AC_DEFINE([PCLIENT_HEAP_SIZE], 32, [Size of the pre-client heap.])
	AC_DEFINE([USER_HEAP_SIZE], 128, [Size of the user heap.])
	AC_DEFINE([DNODE_HEAP_SIZE], 256, [Size of the dlink_node heap.])
	AC_DEFINE([TOPIC_HEAP_SIZE], 256, [Size of the topic heap.])
	AC_DEFINE([LINEBUF_HEAP_SIZE], 128, [Size of the linebuf heap.])
	AC_DEFINE([MEMBER_HEAP_SIZE], 256, [Sizeof member heap.])
	AC_DEFINE([ND_HEAP_SIZE], 128, [Size of the nick delay heap.])
	AC_DEFINE([CONFITEM_HEAP_SIZE], 128, [Size of the confitem heap.])
	AC_DEFINE([MONITOR_HEAP_SIZE], 128, [Size of the monitor heap.])
	AC_DEFINE([FD_HEAP_SIZE], 128, [Size of fd heap.])
	AC_DEFINE([AWAY_HEAP_SIZE], 128, [Size of away heap.])
else
dnl These settings are for a large network like efnet..they will use lots of memory
dnl so enable small net unless you really need this much support
        AC_DEFINE([NICKNAMEHISTORYLENGTH], 15000, [Size of the WHOWAS array.])
        AC_DEFINE([CHANNEL_HEAP_SIZE], 8192, [Size of the channel heap.])
        AC_DEFINE([BAN_HEAP_SIZE], 4096, [Size of the ban heap.])
        AC_DEFINE([CLIENT_HEAP_SIZE], 8192, [Size of the client heap.])
        AC_DEFINE([LCLIENT_HEAP_SIZE], 1024, [Size of the local client heap.])
	AC_DEFINE([PCLIENT_HEAP_SIZE], 256, [Size of the pre-client heap.])
        AC_DEFINE([USER_HEAP_SIZE], 8192, [Size of the user heap.])
        AC_DEFINE([DNODE_HEAP_SIZE], 8192, [Size of the dlink_node heap.])
        AC_DEFINE([TOPIC_HEAP_SIZE], 4096, [Size of the topic heap.])
        AC_DEFINE([LINEBUF_HEAP_SIZE], 2048, [Size of the linebuf heap.])
        AC_DEFINE([MEMBER_HEAP_SIZE], 32768, [Sizeof member heap.])
        AC_DEFINE([ND_HEAP_SIZE], 512, [Size of the nick delay heap.])
        AC_DEFINE([CONFITEM_HEAP_SIZE], 256, [Size of the confitem heap.])
	AC_DEFINE([MONITOR_HEAP_SIZE], 1024, [Size of the monitor heap.])
	AC_DEFINE([FD_HEAP_SIZE], 1024, [Size of fd heap.])
	AC_DEFINE([AWAY_HEAP_SIZE], 512, [Size of away heap.])
fi

AC_ARG_WITH(nicklen,
AS_HELP_STRING([--with-nicklen=LENGTH],[Set the nick length to LENGTH (default 31, max 50)]),
[
  if ! expr "$withval" + 0 >/dev/null 2>&1; then 
	AC_MSG_ERROR(NICKLEN must be a numeric value)
  fi 
  if test $withval -ge 50; then
	NICKLEN=50
	AC_MSG_WARN([NICKLEN has a hard limit of 50. Setting NICKLEN=50])
  else
	NICKLEN="$withval"
  fi
], [NICKLEN=31])

AC_ARG_WITH(topiclen,           
AS_HELP_STRING([--with-topiclen=NUMBER],[Set the max topic length to NUMBER (default 390, max 390)]),
[
 if test $withval -ge 390; then
	TOPICLEN=390
	AC_MSG_WARN([TOPICLEN has a hard limit of 390. Setting TOPICLEN=390])
 else
 	TOPICLEN=$withval
 fi
], [TOPICLEN=390])

AC_DEFINE_UNQUOTED(TOPICLEN, ${TOPICLEN}, [Maximum topic length (<=390)])
AC_DEFINE_UNQUOTED(NICKLEN, (${NICKLEN}+1), [Nickname length])

AC_DEFINE_UNQUOTED(SHARED_SUFFIX, ".so", [Suffix for shared libraries on this platform.])

# rpath, for finding libratbox.so at run time
hold_ldflags=$LDFLAGS
AC_MSG_CHECKING(for the ld -rpath flag)
LDFLAGS="${LDFLAGS} -Wl,-rpath=${libdir}"
AC_LINK_IFELSE([AC_LANG_PROGRAM([],[int i;])], found=yes, found=no)
LDFLAGS=$hold_ldflags
AC_MSG_RESULT($found)
if test "$found" = yes; then
	LDFLAGS="${LDFLAGS} -Wl,-rpath=\${libdir}"
fi

RB_CFLAGS

AC_SUBST(MODULES_LIBS)
AC_SUBST(MOD_TARGET)

AC_SUBST(LDFLAGS)
AC_SUBST(PICFLAGS)
AC_SUBST(SEDOBJ)


if test "$prefix" = "NONE"; then 
	AC_DEFINE_UNQUOTED(IRCD_PREFIX, "$ac_default_prefix", [Prefix where the ircd is installed.])

else

dnl Don't get bitten by Cygwin's stupidity if the user specified
dnl a custom prefix with a trailing slash

	prefix=`echo $prefix | sed 's/\/$//'`
	AC_DEFINE_UNQUOTED(IRCD_PREFIX, "$prefix", [Prefix where the ircd is installed.])
	
fi

AC_CONFIG_FILES(			\
	Makefile			\
	bandb/Makefile			\
	ssld/Makefile			\
	extensions/Makefile		\
	src/Makefile			\
	modules/Makefile		\
	tests/Makefile			\
	tools/Makefile			\
	tools/genssl.sh			\
	doc/Makefile			\
	help/Makefile			\
)

AC_OUTPUT

echo "
Configuration:
	Install directory  : $prefix

	Ziplinks           : $zlib
	Small network      : $small_net
	Block allocator    : $balloc

	Nickname length    : $NICKLEN
	Topic length       : $TOPICLEN

Use (g)make to compile elemental-ircd, then (g)make install to install it.
"

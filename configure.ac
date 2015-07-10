AC_PREREQ(2.68)
AC_COPYRIGHT([2014 elemental-ircd Team])

AC_INIT([elemental-ircd],[6.6.2],[https://github.com/Elemental-IRCd/elemental-ircd/issues])
AC_CONFIG_MACRO_DIR([libratbox/m4])
AC_CONFIG_SRCDIR([src/ircd.c])
AC_CONFIG_HEADER(include/setup.h)
AC_PREFIX_DEFAULT($HOME/ircd)

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
LT_INIT

dnl Checks for programs.
CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1 $CFLAGS"

AC_LANG(C)

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

AC_ISC_POSIX
AC_C_INLINE
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_GCC_TRADITIONAL
AC_PROG_MAKE_SET
AC_PROG_SED
AC_PROG_YACC
AC_PROG_LEX
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(LN, ln)
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(LD, ld)
AC_PATH_PROG(TOUCH, touch)


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
if test "$ac_cv_header_machine_endian_h" = "no" ; then
    AC_C_BIGENDIAN
fi
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

AC_SEARCH_LIBS(socket, socket, , [AC_MSG_ERROR([You have no socket()! Aborting.])])

dnl SunOS/Solaris required libnsl for inet_ntoa()
if test x"$SUN" = xyes; then
	AC_SEARCH_LIBS(inet_ntoa, nsl,, [AC_MSG_ERROR([libnsl not found! Aborting.])])
fi

AC_CHECK_TYPE(socklen_t, ,
[AC_DEFINE([socklen_t], [unsigned int],
[If we don't have a real socklen_t, unsigned int is good enough.])],
[#include <sys/types.h>
#include <sys/socket.h>])

AC_ARG_ENABLE(ipv6,
AC_HELP_STRING([--enable-ipv6],[Enable IPv6 support]),[ipv6=$enableval],[ipv6=no])

AC_SEARCH_LIBS(crypt, [crypt descrypt],,)

CRYPT_LIB=$ac_cv_search_crypt

if test "$CRYPT_LIB" = "none required"; then
	unset CRYPT_LIB
elif test "$CRYPT_LIB" = no; then
	unset CRYPT_LIB
fi

AC_SUBST(CRYPT_LIB)

if test "$ac_cv_header_sys_wait_h" = yes -o "$ac_cv_header_wait_h" = yes; then
	VICONF=viconf
	dnl We need one of the above to build viconf. Just a sanity check,
	dnl we don't want to stop people from building the rest of ircd
	dnl just because they can't build viconf.
else
	VICONF=""
fi

AC_SUBST(VICONF)

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

dnl Checks for the existence of strlcat, strlcpy, basename...
dnl This more reliable test only works with gcc though.

if test "$ac_cv_c_compiler_gnu" = yes; then

AC_MSG_CHECKING(for strlcpy)
save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -Wimplicit -Werror"

AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <string.h>
		#include <stdlib.h>]],
		[[char *a = malloc(6);
		strlcpy(a, "hello", 6);]]
	)],
	[AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_STRLCPY, 1, [Define if strlcpy is available (most BSDs.)])],
	[AC_MSG_RESULT(no)]
)

AC_MSG_CHECKING(for strlcat)
AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <string.h>
		#include <stdlib.h>]],
		[[char *a = malloc(6);
		a[0] = '\0';
		strlcat(a, "hello", 6);]]
	)],
	[AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_STRLCAT, 1, [Define if strlcat is available (most BSDs.)])],
	[AC_MSG_RESULT(no)]
)

CFLAGS=$save_CFLAGS

else

dnl Better than nothing. The more complicated test above probably fixes powerpc,
dnl so who cares.

AC_CHECK_FUNCS([strlcat strlcpy])

fi

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

AC_CHECK_TYPE([in_port_t], [],
[AC_DEFINE(in_port_t, [u_int16_t], [If system does not define in_port_t, define it to what it should be.])],
[[#include <sys/types.h>
#include <netinet/in.h>]])

AC_CHECK_TYPE([sa_family_t], [],
[AC_DEFINE(sa_family_t, [u_int16_t], [If system does not define sa_family_t, define it here.])],
[[#include <sys/types.h>
#include <sys/socket.h>]])

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
AC_HELP_STRING([--with-zlib-path=DIR],[Path to libz.so for ziplinks support.]),
[LIBS="$LIBS -L$withval"],)

AC_ARG_ENABLE(zlib,
AC_HELP_STRING([--disable-zlib],[Disable ziplinks support]),
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

dnl IO Loop Selection
dnl =================

AC_ARG_ENABLE(ports, AC_HELP_STRING([--enable-ports],[Force solaris I/O ports subsystem usage.]),
[ if test $enableval = yes; then 
	SELECT_TYPE_EXPLICIT="ports"
  else
  	use_ports=no
  fi
],)

AC_ARG_ENABLE(poll, AC_HELP_STRING([--enable-poll],[Force poll() usage.]),
[ if test $enableval = yes; then 
	SELECT_TYPE_EXPLICIT="poll"
  else
  	use_poll=no
  fi
],)

AC_ARG_ENABLE(select, AC_HELP_STRING([--enable-select],[Force select() usage.]),
[ if test $enableval = yes; then 
	SELECT_TYPE_EXPLICIT="select" 
  else
  	use_select=no
  fi
],)

AC_ARG_ENABLE(kqueue, AC_HELP_STRING([--enable-kqueue],[Force kqueue() usage.]),
[ if test $enableval = yes; then 
	SELECT_TYPE_EXPLICIT="kqueue"
  else
  	use_kqueue=no
  fi
],)

AC_ARG_ENABLE(epoll, AC_HELP_STRING([--enable-epoll],[Force sys_epoll usage (Linux only).]),
[ if test $enableval = yes; then 
	SELECT_TYPE_EXPLICIT="epoll"
	AC_CHECK_FUNCS(epoll_ctl, [haveepoll=yes], [haveepoll=no])
  else
  	use_epoll=no
  fi
],)

dnl **********************************************************************
dnl Check for --with-confdir [deprecated, use --sysconfdir instead]
dnl **********************************************************************

AC_ARG_WITH([confdir],
	[AC_HELP_STRING([--with-confdir=DIR],
	       [Directory to install config files [deprecated, use --sysconfdir instead].])],
	[ sysconfdir=`echo $withval | sed 's/\/$//'` ],
	[ confdir='${sysconfdir}' ])
AC_DEFINE_DIR([ETC_DIR], [sysconfdir], [Prefix where config files are installed.])

dnl **********************************************************************
dnl Check for --with-logdir
dnl **********************************************************************

AC_MSG_CHECKING([whether to modify logdir])
AC_ARG_WITH(logdir, 
AC_HELP_STRING([--with-logdir=DIR],
	       [Directory where to write logfiles.]),
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
AC_HELP_STRING([--with-helpdir=DIR],
	       [Directory to install help files.]),
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
	[AC_HELP_STRING([--with-moduledir=DIR],
	       [Directory to install modules.])],
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
	[AC_HELP_STRING([--with-rundir=DIR],
		[Directory in which to store pidfile.])],
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

if test ! -z "$SELECT_TYPE_EXPLICIT"; then
	SELECT_TYPE="$SELECT_TYPE_EXPLICIT";
	echo "Forcing $SELECT_TYPE to be enabled"
else

if test ! "x$use_ports" = "xno"; then
	AC_CHECK_FUNCS(port_getn, [haveports=yes], [haveports=no])
	if test "x$haveports" = "xyes" ; then
		SELECT_TYPE="ports"
	fi
fi

if test ! "x$use_select" = "xno"; then
	AC_CHECK_FUNCS(select, [haveselect=yes], [haveselect=no])
	if test "x$haveselect" = "xyes" ; then
		SELECT_TYPE="select"
	fi
fi

if test ! "x$use_poll" = "xno"; then
	AC_CHECK_FUNCS(poll, [havepoll=yes], [havepoll=no])
	if test "x$havepoll" = "xyes" ; then
		SELECT_TYPE="poll"
	fi
fi

if test ! "x$use_kqueue" = "xno"; then
	AC_CHECK_FUNCS(kevent, [havekqueue=yes], [havekqueue=no])
	if test "x$havekqueue" = "xyes" ; then
		SELECT_TYPE="kqueue"
	fi
fi

if test ! "x$use_epoll" = "xno"; then
	AC_CHECK_FUNCS(epoll_ctl, [haveepoll=yes], [haveepoll=no])
	if test "x$ac_cv_header_sys_epoll_h" = "xyes"; then
		if test "x$haveepoll" = "xyes" ; then
                	AC_MSG_CHECKING(for epoll support in kernel)
	                AC_TRY_RUN(
#include <stdint.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <unistd.h>     

int
main(int argc, char **argv)
{
        int epfd;

        epfd = epoll_create(256);
        return (epfd == -1 ? 1 : 0);
}, [AC_MSG_RESULT(yes)
	    AC_DEFINE(HAVE_EPOLL, 1,
	        [Define if your system supports the epoll system calls])
	    SELECT_TYPE="epoll"], 
	    AC_MSG_RESULT(no), AC_MSG_RESULT(no))
		fi
	fi

haveepollsyscall=no

if test "x$ac_cv_header_sys_epoll_h" = "xyes"; then
	if test "x$haveepoll" = "xno" ; then
                AC_MSG_CHECKING(for epoll system call)
                AC_TRY_RUN(
#include <stdint.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <unistd.h>     

int
epoll_create(int size)
{
        return (syscall(__NR_epoll_create, size));
}
 
int
main(int argc, char **argv)
{
        int epfd;

        epfd = epoll_create(256);
        exit (epfd == -1 ? 1 : 0);
}, [AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_EPOLL, 1,
        [Define if your system supports the epoll system calls])
    SELECT_TYPE="epoll"], 
    AC_MSG_RESULT(no), AC_MSG_RESULT(no))
	fi
fi

fi

fi

if test -z "$SELECT_TYPE"; then
	AC_MSG_ERROR([Unable to find a usable IO interface],)
fi

echo "Using $SELECT_TYPE for select loop."

AC_DEFINE_UNQUOTED(SELECT_TYPE, "$SELECT_TYPE", [This is the type of IO loop we are using])
AC_SUBST(SELECT_TYPE)


dnl Debug-related options
dnl =====================

AC_ARG_ENABLE(assert,
AC_HELP_STRING([--enable-assert],[Enable assert(). Choose between soft(warnings) and hard(aborts the daemon)]),
[assert=$enableval], [assert=no])

if test "$assert" = no; then
	AC_DEFINE(NDEBUG, 1, [Define this to disable debugging support.])
elif test "$assert" = soft; then
	AC_DEFINE(SOFT_ASSERT, 1, [Define this to enable soft asserts.])
	AC_DEFINE(NDEBUG, 1, [Define this to disable debugging support.])
elif test "$assert" = yes; then
	assert="hard";
fi

AC_MSG_CHECKING(if you want IO Debugging hooks)
AC_ARG_ENABLE(iodebug,
AC_HELP_STRING([--enable-iodebug],[Enable IO Debugging hooks]),
[iodebug=$enableval], [iodebug=no])

if test "$iodebug" = yes; then
	AC_DEFINE(USE_IODEBUG_HOOKS, 1, [Define this to enable IO Debug hooks.])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi


AC_MSG_CHECKING(if you want to do a profile build)
AC_ARG_ENABLE(profile,
AC_HELP_STRING([--enable-profile],[Enable profiling]),
[profile=$enableval], [profile=no])

if test "$profile" = yes; then
	if test "$ac_cv_c_compiler_gnu" = yes; then
		CFLAGS="$CFLAGS -pg"
		AC_MSG_RESULT([yes, adding -pg])
		AC_DEFINE(CHARYBDIS_PROFILE, 1, [Define this if you are profiling.])
	else
		AC_MSG_RESULT([no, profile builds only work with gcc])
	fi
else
	AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(balloc,
AC_HELP_STRING([--disable-balloc],[Disable the block allocator.]),
[balloc=$enableval], [balloc=yes])

if test "$balloc" = no; then
	AC_DEFINE([NOBALLOC], 1, [Define to 1 if you wish to disable the block allocator.])
fi

AC_ARG_ENABLE(small-net,
AC_HELP_STRING([--disable-small-net],[Disable small network support.]),
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
AC_HELP_STRING([--with-nicklen=LENGTH],[Set the nick length to LENGTH (default 31, max 50)]),
[
  if ! expr "$withval" + 0 >/dev/null 2>&1; then 
	AC_ERROR([NICKLEN must be a numeric value])
  fi 
  if test $withval -ge 50; then
	NICKLEN=50
	AC_MSG_WARN([NICKLEN has a hard limit of 50. Setting NICKLEN=50])
  else
	NICKLEN="$withval"
  fi
], [NICKLEN=31])

AC_ARG_WITH(topiclen,           
AC_HELP_STRING([--with-topiclen=NUMBER],[Set the max topic length to NUMBER (default 390, max 390)]),
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

shared_modules="yes"
dnl Some first-stage sanity checks.
if test "$shared_modules" = yes; then
	dnl TenDRA's cc is called tcc too.
	if test "$CC" = tcc -a "$TenDRA" = "no"; then
		AC_MSG_WARN([disabling shared modules: Tiny C Compiler can't create PIC])
		shared_modules="no"
	fi
fi

dnl Second stage: check for functions and headers.
if test "$shared_modules" = yes; then
	DYNLINK_C=dynlink.c
	AC_CHECK_HEADERS(dlfcn.h)
	AC_SEARCH_LIBS(shl_load, dld, 
	[
	  AC_DEFINE(HAVE_SHL_LOAD, 1, [Define if the shl_load function is available.])
	  SUFFIX=".sl"
	  MOD_TARGET=hpux_shared
	  SEDOBJ="s/\.o/.sl/g"  
	],
	dnl !shl_load:
	[
          dnl standard dlopen
          AC_SEARCH_LIBS(dlopen, [dl c_r],
	  [
	    AC_DEFINE(HAVE_DLOPEN, 1, [Define if the dlopen function is available.])
	    SUFFIX=".so"
	    MOD_TARGET=shared_modules
	    SEDOBJ="s/\.o/.so/g"
	    if test "$AppleGCC" = yes; then
              AC_CHECK_HEADERS([mach-o/dyld.h])
            fi
	    AC_CHECK_FUNC(dlsym, ,
	    [
	      AC_MSG_WARN([dlsym is not available, shared modules disabled])
	      shared_modules=no
	    ])
	    AC_CHECK_FUNCS(dlfunc)
	  ],
	  [
	    shared_modules=no
	  ])
	])
fi

AC_DEFINE_UNQUOTED(SHARED_SUFFIX, "$SUFFIX", [Suffix for shared libraries on this platform.])

dnl Third stage - wrangling the linker.
if test "$shared_modules" = yes; then
	# The GNU linker requires the -export-dynamic option to make
	# all symbols visible in the dynamic symbol table.
	hold_ldflags=$LDFLAGS
	AC_MSG_CHECKING(for the ld -export-dynamic flag)
	LDFLAGS="${LDFLAGS} -Wl,-export-dynamic -Werror"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([],[int i;])], found=yes, found=no)
	LDFLAGS=$hold_ldflags

	if expr "`uname -s`" : ^IRIX >/dev/null 2>&1; then
		found="no, IRIX ld uses -B,dynamic"
		LDFLAGS="${LDFLAGS} -Wl,-B,dynamic"
	fi
	
	if expr "`uname -s`" : ^AIX >/dev/null 2>&1; then
		found="no, AIX ld uses -G -brtl"
		LDFLAGS="${LDFLAGS} -Wl,-G,-brtl"
	fi

	AC_MSG_RESULT($found)

	if test "$found" = yes; then
		LDFLAGS="${LDFLAGS} -Wl,-export-dynamic"
	fi

	AC_MSG_CHECKING(for compiler option to produce PIC)
	dnl The order should be here to check for groups of compilers,
	dnl then for odd compilers, then if no PICFLAGS were set up,
	dnl check for GCC and set defaults, or else error. -jmallett
	if test "$SGS" = "yes"; then
		AC_MSG_RESULT([SVR4 SGS interfaces: -KPIC -DPIC -G])
		PICFLAGS="-KPIC -DPIC -G"
	fi
	
	if test "$AppleGCC" = "yes"; then
		AC_MSG_RESULT([Darwin Mach-O bundles: -fno-common -bundle -flat_namespace -undefined suppress])
		PICFLAGS="-fno-common -bundle -flat_namespace -undefined suppress"
	fi
	dnl Please note, that on HPUX two different stages of module compilation occurs, since
	dnl while compiling modules, the compiler does not allow you to give arguments
	dnl to the linker. (I did not design this) 
	dnl So we need -c in the first stage of module compilation.
	dnl In the second stage, we link the modules via ld -b.
	dnl Additionally, HPUX does not like -export-dynamic, it likes -E instead.
	dnl -TimeMr14C
	if test "$HPUX" = "yes" -a "$CC" != gcc; then
		AC_MSG_RESULT(HP-UX cc: +z -r -q -n)
		PICFLAGS="+z -r -q -n -c"
		AC_MSG_CHECKING([if +ESfic is required on this platform])
		
		if expr "`$CC +ESfic 2>&1`" : ".*neither supported.*" >/dev/null; then 
			AC_MSG_RESULT(no)
		else
			AC_MSG_RESULT(yes)
			PICFLAGS="$PICFLAGS +ESfic"
		fi

		LDFLAGS="${LDFLAGS} -Wl,-E"
	fi
	if test "$Tru" = yes -a "$CC" != gcc; then
		AC_MSG_RESULT([Tru64: -shared -expect_unresolved '*'])
		PICFLAGS="-shared -expect_unresolved '*' "
		LDFLAGS="-call_shared"
	fi
	if test -z "$PICFLAGS"; then
		if test "$ac_cv_c_compiler_gnu" = "yes"; then
			AC_MSG_RESULT(gcc: -fPIC -DPIC -shared)
			PICFLAGS="-fPIC -DPIC -shared"
		else
			AC_MSG_RESULT(no)
			shared_modules=no
		fi
	fi
fi

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

# This must be down here, or it will mess up checks like the ones
# for -Wl,-export-dynamic
# -- jilles
CHARYBDIS_C_GCC_TRY_FLAGS([-Wall], charybdis_cv_c_gcc_w_all)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wextra], charybdis_cv_c_gcc_w_extra)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=pointer-arith], charybdis_cv_c_gcc_w_pointer_arith)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=implicit], charybdis_cv_c_gcc_w_implicit)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=nested-externs], charybdis_cv_c_gcc_w_nested_externs)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wcast-align], charybdis_cv_c_gcc_w_cast_align)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wcast-qual], charybdis_cv_c_gcc_w_cast_qual)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=write-strings], charybdis_cv_c_gcc_w_write_strings)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=error-implicit-function-declaration], charybdis_cv_c_gcc_w_error_implicit_function_declaration)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=strict-prototypes], charybdis_cv_c_gcc_strict_prototypes)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=missing-prototypes], charybdis_cv_c_gcc_missing_prototypes)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=missing-declarations], charybdis_cv_c_gcc_missing_declarations)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wparenthesis], charybdis_cv_c_gcc_parenthesis)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=shadow], charybdis_cv_c_gcc_w_shadow)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wmissing-noreturn], charybdis_cv_c_gcc_w_missing_noreturn)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=undef], charybdis_cv_c_gcc_w_undef)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=packed], charybdis_cv_c_gcc_w_packed)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wunused-result], charybdis_cv_c_gcc_w_unused_result)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wunused-function], charybdis_cv_c_gcc_w_unused_function)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wunused-label], charybdis_cv_c_gcc_w_unused_label)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wunused-value], charybdis_cv_c_gcc_w_unused_value)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wunused-variable], charybdis_cv_c_gcc_w_unused_variable)
# Unused parameters are common
CHARYBDIS_C_GCC_TRY_FLAGS([-Wno-unused-parameter], charybdis_cv_c_gcc_w_unused_variable)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wredundant-decls], charybdis_cv_c_gcc_w_redundant_decls)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=float-equal], charybdis_cv_c_gcc_w_float_equal)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=format], charybdis_cv_c_gcc_w_format)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=format-y2k], charybdis_cv_c_gcc_w_format_y2k)
# format-security is omited as non-literal printf formats are used extensively
CHARYBDIS_C_GCC_TRY_FLAGS([-Wno-format-security], charybdis_cv_c_gcc_w_no_format_security)

dnl Stage 5 - underscores in front of symbol names.
if test "$shared_modules" = yes; then

	AC_CHECK_FUNC(nlist,,
		AC_CHECK_LIB(dl, nlist, nlist_lib="-ldl",
			AC_CHECK_LIB(elf, nlist, nlist_lib="-lelf",)
		)
	)

        dnl We need to find out whether underscores are appended to symbol
        dnl names in executable files.  First, though, we need to see
        dnl where nlist.h is hiding.
	AC_CHECK_HEADER(libelf/nlist.h, [ nlist_h="libelf/nlist.h" ], )
	AC_CHECK_HEADER(elf/nlist.h, [ nlist_h="elf/nlist.h" ], )
	AC_CHECK_HEADER(nlist.h, [ nlist_h="nlist.h" ], )
	if test x"$nlist_h" = "x"; then
		AC_DEFINE_UNQUOTED(SYMBOL_PREFIX, "", [String containing extra underscores prepended to symbols loaded from modules.])
	else
		AC_MSG_CHECKING(for extra underscores prepended to symbol names)
		AC_CACHE_VAL(_cv_symbol_underscores,
		[
cat << EOF > conftest.c
#include <$nlist_h>
#include <stdio.h>
#include <stdlib.h>
void _modinit(void);
int main(int argc, char *argv[[]]) {
	int i;
	struct nlist nl[[5]];

	/* fill the names in this way, so it'll work almost everywhere */
	nl[[0]].n_name = "_modinit";
	nl[[1]].n_name = "__modinit";
	nl[[2]].n_name = "___modinit";
	nl[[3]].n_name = "____modinit";
	nl[[0]].n_value = nl[[1]].n_value = nl[[2]].n_value = nl[[3]].n_value = nl[[4]].n_name = NULL;

	if(argc < 2 || (nlist(argv[[1]], nl)) == -1) exit(-1);
	for(i = 0; i < 4; i++) {
		if(nl[[i]].n_value != NULL)
		{
			int j;
			for(j = 0; j < i; j++)
				printf("_");
			exit(i);
		}
	}
	exit(-1);
}
void _modinit(void) { return; }
EOF
		$CC $CPPFLAGS $CFLAGS -o conftest conftest.c $nlist_lib >/dev/null 2>&1
		symbol_underscores=`./conftest conftest`
		AC_MSG_RESULT($symbol_underscores)
		$RM -f conftest conftest.c
		])
		AC_DEFINE_UNQUOTED(SYMBOL_PREFIX, "${symbol_underscores}", [String containing extra underscores prepended to symbols loaded from modules.])
	fi
fi

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
	Socket Engine      : $SELECT_TYPE
	Small network      : $small_net
	Block allocator    : $balloc

	Nickname length    : $NICKLEN
	Topic length       : $TOPICLEN

Use (g)make to compile elemental-ircd, then (g)make install to install it.
"

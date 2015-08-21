AC_PREREQ([2.68])
AC_COPYRIGHT([2014 elemental-ircd Team])

AC_INIT([libratbox],[devel])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([tools])
AC_CONFIG_SRCDIR([include/ratbox_lib.h])
AC_CONFIG_HEADERS([include/libratbox_config.h])
AC_LANG(C)

AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])

AC_PREFIX_DEFAULT(/usr/local/libratbox)
AC_USE_SYSTEM_EXTENSIONS

AC_PROG_CC_C99([], [AC_MSG_ERROR(libratbox requires a C99 capable compiler)])
AC_PROG_INSTALL
AC_PROG_EGREP
AC_PROG_SED
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(CP, cp)

AC_DISABLE_STATIC([])
AC_ENABLE_SHARED([])

LT_INIT([dlopen])

GCC_STACK_PROTECT_CC

PKG_PROG_PKG_CONFIG

LIBTOOL="$LIBTOOL --silent"

dnl use directory structure of cached as default (hack)
if test "$libexecdir" = '${exec_prefix}/libexec' &&
   test "$localstatedir" = '${prefix}/var'; then
	libexecdir='${bindir}'
	localstatedir='${prefix}'
fi

AC_C_BIGENDIAN
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T

AC_SEARCH_LIBS([strerror],[cposix])

dnl Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADERS([crypt.h unistd.h sys/socket.h sys/stat.h sys/time.h time.h netinet/in.h arpa/inet.h errno.h sys/uio.h spawn.h sys/poll.h sys/epoll.h sys/select.h sys/devpoll.h sys/event.h port.h signal.h sys/signalfd.h sys/timerfd.h linux/tcp.h windows.h])
AC_HEADER_TIME

dnl Networking Functions
dnl ====================

AC_SEARCH_LIBS(socket, [socket],,)


AC_CHECK_MEMBER([struct sockaddr.sa_len], [AC_DEFINE(SOCKADDR_IN_HAS_LEN, 1, [Define to 1 if sockaddr has a 'sa_len'
member.])],,[[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
]])

AC_CHECK_TYPE([socklen_t], [rb_socklen_t=socklen_t], [rb_socklen_t=int], [
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
])

AC_ARG_ENABLE(ipv6,AS_HELP_STRING([--disable-ipv6],[Disable IPv6 support]),[ipv6=$enableval],[ipv6=yes])

if test x$ipv6 != xyes; then
	have_v6="no"
else

have_v6=yes
RB_PROTO_INET6([], [
		AC_MSG_NOTICE([Disabling IPv6 support: PF_INET6 not found])
		have_v6=no
		])

	if test "X$have_v6" = "Xyes"; then
	RB_TYPE_STRUCT_SOCKADDR_IN6([], [
		AC_MSG_NOTICE([Disabling IPv6 support: struct sockaddr_in6 not found])
		have_v6=no
		])
	fi

fi			

AC_CHECK_TYPES([struct sockaddr_storage],[],[AC_MSG_ERROR([** Could not find sockaddr_storage])], [
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
])

dnl Check for stdarg.h - if we cant find it, halt configure
AC_CHECK_HEADER(stdarg.h, , [AC_MSG_ERROR([** stdarg.h could not be found - libratbox will not compile without it **])])

AC_CHECK_TYPE([sa_family_t], [],
[AC_DEFINE(sa_family_t, [u_int16_t], [If system does not define sa_family_t, define it here.])],
[[#include <sys/types.h>
#include <sys/socket.h>]])


dnl check for various functions...
AC_CHECK_FUNCS([socketpair gettimeofday writev sendmsg gmtime_r strtok_r usleep posix_spawn strlcpy strlcat strnlen fstat signalfd select poll kevent port_create epoll_ctl arc4random getrusage timerfd_create])	

AC_CHECK_DECLS([environ], [], [], [[#include <unistd.h>]])

AC_SEARCH_LIBS(nanosleep, rt posix4, AC_DEFINE(HAVE_NANOSLEEP, 1, [Define if you have nanosleep]))
AC_SEARCH_LIBS(timer_create, rt, AC_DEFINE(HAVE_TIMER_CREATE, 1, [Define if you have timer_create]))
AC_SEARCH_LIBS(timerfd_create, rt, AC_DEFINE(HAVE_TIMERFD_CREATE, 1, [Define if you have timerfd_create]))

AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MMAP


dnl IO Loop Selection
dnl =================

AC_ARG_WITH(ports,  AS_HELP_STRING([--with-ports],  [solaris I/O ports subsystem usage.]))
AC_ARG_WITH(poll,   AS_HELP_STRING([--with-poll],   [poll() usage.]))
AC_ARG_WITH(select, AS_HELP_STRING([--with-select], [select() usage.]))
AC_ARG_WITH(kqueue, AS_HELP_STRING([--with-kqueue], [kqueue() usage.]))
AC_ARG_WITH(epoll,  AS_HELP_STRING([--with-epoll],  [Linux epoll usage.]))
AC_ARG_WITH(devpoll,AS_HELP_STRING([--with-devpoll],[/dev/poll usage.]))
AC_ARG_WITH(win32,  AS_HELP_STRING([--with-win32],  [Windows WSAAsyncSelect usage.]))

select_types=""

RB_IF_REQUESTED([ports], [
	AC_CHECK_FUNC([port_getn], [
		AC_DEFINE([WITH_SOLARIS_PORTS],, [Support Solaris ports])
		AX_APPEND_FLAG([ports], [select_types])
	], [
		RB_IF_REQUIRED([ports], AC_MSG_ERROR([Solaris ports requested but not available]))
	])
])

RB_IF_REQUESTED([select], [
	AC_CHECK_FUNCS([select], [
		AC_DEFINE([WITH_SELECT], [], [Support select()])
		AX_APPEND_FLAG([select], [select_types])
	], [
		RB_IF_REQUIRED([select], AC_MSG_ERROR([select() support requested but not available]))
	])
])

RB_IF_REQUESTED([poll], [
	AC_CHECK_FUNCS([poll], [
		AC_DEFINE([WITH_POLL], [], [Support poll()])
		AX_APPEND_FLAG([poll], [select_types])
	], [
		RB_IF_REQUIRED([poll], AC_MSG_ERROR([poll() support requested but not available]))
	])
])

RB_IF_REQUESTED([kqueue], [
	AC_CHECK_FUNCS([kevent], [
		AC_DEFINE([WITH_KQUEUE], [], [Support FreeBSD kqueue])
		AX_APPEND_FLAG([kqueue], [select_types])
	], [
		RB_IF_REQUIRED([kqueue], AC_MSG_ERROR([kqueue support requested but not available]))
	])
])

RB_IF_REQUESTED([epoll], [
	AC_CHECK_FUNCS([epoll_ctl], [
		AC_DEFINE([WITH_EPOLL], [], [Support epoll])
		AX_APPEND_FLAG([epoll], [select_types])
	], [
		RB_IF_REQUIRED([epoll], AC_MSG_ERROR([epoll support requested but not available]))
	])
])

RB_IF_REQUESTED([devpoll], [
	AC_CHECK_HEADER([sys/devpoll.h], [
		AC_DEFINE([WITH_DEVPOLL], [], [Support /dev/poll])
		AX_APPEND_FLAG([devpoll], [select_types])
	], [
		RB_IF_REQUIRED([devpoll], AC_MSG_ERROR([/dev/poll support requested but not available]))
	])
])

dnl This check wont actually work yet
RB_IF_REQUESTED([win32], [
	AC_CHECK_LIB([ws2_32], [WSAAsyncSelect], [
		AC_DEFINE([WITH_WIN32_SELECT], [], [Support WSAAsyncSelect])
		AX_APPEND_FLAG([win32], [select_types])
		AX_APPEND_FLAG([-lws2_32], [LDFLAGS])
	], [
		RB_IF_REQUIRED([win32], AC_MSG_ERROR([win32 support requested but not available]))
	])
])

if test -z "$select_types"; then
	AC_MSG_ERROR([Unable to find a usable IO interface],)
fi


dnl OpenSSL support
AC_ARG_ENABLE(openssl, [
	AS_HELP_STRING([--enable-openssl],[Enable OpenSSL support.])
	AS_HELP_STRING([--disable-openssl],[Disable OpenSSL support.])
],
	[cf_enable_openssl=$enableval],
	[cf_enable_openssl="auto"]
)

if test x"$cf_enable_openssl" != x"no"; then
	PKG_CHECK_MODULES([SSL], [openssl > 0.9.8], [cf_have_openssl=yes], [cf_have_openssl=no])
fi

if test x"$cf_enable_openssl" == x"yes" && x"$cf_have_openssl" == x"no"; then
	AC_MSG_ERROR([** openssl requested but not found])
fi

dnl GnuTLS support
AC_ARG_ENABLE(gnutls, [
	AS_HELP_STRING([--enable-gnutls],[Enable GnuTLS support.])
	AS_HELP_STRING([--disable-gnutls],[Disable GnuTLS support.])
],
	[cf_enable_gnutls=$enableval],
	[cf_enable_gnutls="auto"]
)

if test x"$cf_enable_gnutls" != x"no"; then
	PKG_CHECK_MODULES([GNUTLS], [gnutls >= 2.11.0], [cf_have_gnutls=yes], [cf_have_gnutls=no])
fi

if test x"$cf_enable_gnutls" == x"yes" && x"$cf_have_gnutls" == x"no"; then
	AC_MSG_ERROR([** gnutls requested but not found])
fi

if test x"$cf_have_openssl" == x"yes"; then
	AC_DEFINE(HAVE_OPENSSL,1,[Has OpenSSL])
	GNUTLS_CFLAGS=""
	GNUTLS_LIBS=""
	SSL_TYPE="openssl"
elif test x"$cf_have_gnutls" == x"yes"; then
	AC_DEFINE(HAVE_GNUTLS, 1, [Has GnuTLS])
	SSL_LIBS=""
	SSL_CFLAGS=""
	SSL_TYPE="gnutls"
else
	AC_MSG_ERROR([** No ssl library found, please install openssl or gnutls])
fi

dnl Debug-related options
dnl =====================

AC_ARG_ENABLE(assert, AS_HELP_STRING([--enable-assert],[Enable assert(). Choose between soft(warnings) and hard(aborts the daemon)]),[],[enable_assert=no])
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

RB_CFLAGS

AC_SUBST(LDFLAGS)
AC_SUBST(PICFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(SEDOBJ)
AC_SUBST(SSL_CFLAGS)
AC_SUBST(SSL_LIBS)
AC_SUBST(GNUTLS_CFLAGS)
AC_SUBST(GNUTLS_LIBS)

AC_CONFIG_COMMANDS([include/librb-config.h],
[
	outfile=include/librb-config.h.tmp
	cat > $outfile  <<\_______EOF
/*    
 * librb-config.h: libratbox config file. Please modify configure.ac
 */
 
#ifndef __LIBRB_CONFIG_H
#define __LIBRB_CONFIG_H

_______EOF

if test "x$rb_have_ipv6" = "xyes"; then
	echo "#define RB_IPV6 1" >> $outfile
fi

if test "x$rb_windows_h" = "xyes"; then
	echo '#define WIN32_LEAN_AND_MEAN 1' >> $outfile
	echo '#include <windows.h>' >> $outfile
	echo '#include <winsock2.h>' >> $outfile
	echo '#include <ws2tcpip.h>' >> $outfile
	echo '#include <iphlpapi.h>' >> $outfile
fi

if test "x$rb_alloca_h" = "xyes"; then
	echo '#define RB_HAVE_ALLOCA_H 1' >> $outfile
fi

if test "x$rb_header_stdc" = "xyes"; then
	echo '#include <stdlib.h>' >> $outfile
	echo '#include <stddef.h>' >> $outfile
elif test "x$rb_header_stdlib" = "xyes"; then
	echo '#include <stdlib.h>' >> $outfile
fi

if test "x$rb_header_string_h" = "xyes"; then
	echo '#include <string.h>' >> $outfile
fi

if test "x$rb_stdint_h" = "xyes"; then
	echo '#include <stdint.h>' >> $outfile
fi

if test "x$rb_inttypes_h" = "xyes"; then
	echo '#include <inttypes.h>' >> $outfile
fi

if test "x$rb_sys_types_h" = "xyes"; then
	echo '#include <sys/types.h>' >> $outfile
fi
if test "x$rb_sys_time_h" = "xyes"; then
	echo '#include <sys/time.h>' >> $outfile
fi
if test "x$rb_sys_stat_h" = "xyes"; then
	echo '#include <sys/stat.h>' >> $outfile
fi
if test "x$rb_time_h" = "xyes"; then
	echo '#include <time.h>' >> $outfile
fi

if test "x$rb_sys_socket_h" = "xyes"; then
	echo '#include <sys/socket.h>' >> $outfile
fi

if test "x$rb_netinet_in_h" = "xyes"; then
	echo '#include <netinet/in.h>' >> $outfile
fi

if test "x$rb_arpa_inet_h" = "xyes"; then
	echo '#include <arpa/inet.h>' >> $outfile
fi

if test "x$rb_unistd_h" = "xyes"; then
	echo '#include <unistd.h>' >> $outfile
fi

if test "x$rb_crypt_h" = "xyes"; then
	echo '#include <crypt.h>' >> $outfile
fi

if test "x$rb_errno_h" = "xyes"; then
	echo '#include <errno.h>' >> $outfile
fi

echo "typedef $rb_socklen_t rb_socklen_t;" >> $outfile


if test "x$rb_sockaddr_sa_len" = "xyes"; then
	echo '#define RB_SOCKADDR_HAS_SA_LEN 1' >> $outfile
fi

cat >> $outfile <<\_______EOF
#endif /* __LIBRB_CONFIG_H */
_______EOF

if cmp -s $outfile include/librb-config.h; then
	AC_MSG_NOTICE([include/librb-config.h is unchanged])
	${rb_rm} -f $outfile
else
	${rb_mv} $outfile include/librb-config.h
fi

],[
	if test x$ac_cv_header_stdc = xyes; then
		rb_header_stdc=yes
	fi
	if test x$ac_cv_header_stdlib_h = xyes; then
		rb_header_stdlib_h=yes
	fi
	if test x$ac_cv_header_string_h = xyes; then
		rb_header_string_h=yes
	fi
	if test x$ac_cv_header_memory_h = xyes; then
		rb_header_memory_h=yes
	fi
	if test "x${ac_cv_working_alloca_h+set}" = xset ; then
		rb_alloca_h="$ac_cv_working_alloca_h"
	else
		rb_alloc_h="$ac_cv_header_alloca_h"
	fi
	if test x$ac_cv_member_struct_sockaddr_sa_len = xyes; then
		rb_sockaddr_sa_len=yes
	fi
	if test x$ac_cv_header_sys_socket_h = xyes; then
		rb_sys_socket_h=yes
	fi
	if test x$ac_cv_header_sys_types_h = xyes; then
		rb_sys_types_h=yes
	fi
	if test x$ac_cv_header_sys_stat_h = xyes; then
		rb_sys_stat_h=yes
	fi
	if test x$ac_cv_header_sys_time_h = xyes; then
		rb_sys_time_h=yes
	fi
	if test x$ac_cv_header_time = xyes; then
		rb_time_h=yes
	fi

	if test x$ac_cv_header_stdint_h = xyes; then
		rb_stdint_h=yes
	fi
	if test x$ac_cv_header_inttypes_h = xyes; then
		rb_inttypes_h=yes
	fi
	if test x$ac_cv_header_netinet_in_h = xyes; then
		rb_netinet_in_h=yes
	fi

	if test x$ac_cv_header_crypt_h = xyes; then
		rb_crypt_h=yes
	fi
	if test x$ac_cv_header_errno_h = xyes; then
		rb_errno_h=yes
	fi
	if test x$ac_cv_header_unistd_h = xyes; then
		rb_unistd_h=yes
	fi
	if test x$ac_cv_header_windows_h = xyes; then
		rb_windows_h=yes
	fi
	if test x$ac_cv_header_winsock2_h = xyes; then
		rb_winsock2_h=yes
	fi

	rb_socklen_t=$rb_socklen_t

	rb_have_ipv6="$have_v6"
		
	rb_mv="$MV"
	rb_rm="$RM"
		
]
	

)


AC_CONFIG_FILES(		\
	src/Makefile		\
	Makefile		\
	libratbox.pc
)

AC_OUTPUT

dnl Make it look sexay!

echo
echo "Compiling $PACKAGE_NAME $PACKAGE_VERSION"
echo

echo "Installing into: $prefix"

echo "IPv6 support ................... $have_v6"
echo "Assert debugging ............... $enable_assert"
echo "Block allocator ................ $balloc"
echo "Select backends ................$select_types" #there's always a leading space in $select_types
echo "SSL Type ....................... $SSL_TYPE"
if test x"$enable_debug" != x"no"; then
echo "Debug Build .................... yes"
fi
echo


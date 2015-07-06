AC_PREREQ(2.68)
AC_COPYRIGHT([2014 elemental-ircd Team])

AC_INIT([libratbox],[devel])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([include/ratbox_lib.h])

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

AM_CONFIG_HEADER(include/libratbox_config.h)

AC_PREFIX_DEFAULT(/usr/local/libratbox)
AC_GNU_SOURCE

dnl Checks for programs.
AC_PROG_CC_C99

if test x"$ac_cv_prog_cc_c99" = "xno"; then
	AC_ERROR([ircd-ratbox requires a C99 capable compiler])
fi

AC_PROG_INSTALL
AC_PROG_EGREP
AC_PROG_SED
AC_PATH_PROG(CP, cp)

F77=no
CXX=no
GCJ=no

AM_DISABLE_STATIC
AM_ENABLE_SHARED
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_C_INLINE
AC_C_CONST
AC_C_BIGENDIAN
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PATH_PROG(AUTOMAKE, automake)
AC_PATH_PROG(ACLOCAL, aclocal)
AC_PATH_PROG(AUTOHEADER, autoheader)
AC_PATH_PROG(AS, as)
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(LN, ln)
AC_PATH_PROG(TOUCH, touch)
AC_LANG(C)
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

GCC_STACK_PROTECT_CC
GCC_STACK_PROTECT_LIB

PKG_PROG_PKG_CONFIG

LIBTOOL="$LIBTOOL --silent"

dnl use directory structure of cached as default (hack)
if test "$libexecdir" = '${exec_prefix}/libexec' &&
   test "$localstatedir" = '${prefix}/var'; then
	libexecdir='${bindir}'
	localstatedir='${prefix}'
fi

AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T


dnl Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADERS([crypt.h unistd.h sys/socket.h sys/stat.h sys/time.h time.h netinet/in.h arpa/inet.h errno.h sys/uio.h spawn.h sys/poll.h sys/epoll.h sys/select.h sys/devpoll.h sys/event.h port.h signal.h sys/signalfd.h sys/timerfd.h linux/tcp.h])
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
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
]])

AC_CHECK_TYPE([socklen_t], rb_cv_socklen_t=socklen_t,[
	AC_MSG_CHECKING([for socklen_t equivalent])
	AC_CACHE_VAL([rb_cv_socklen_t],
	[
	# Systems have either "struct sockaddr *" or
	# "void *" as the second argument to getpeername
	rb_cv_socklen_t=
	for arg2 in "struct sockaddr" void; do
		for t in int size_t unsigned long "unsigned long"; do
			AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
int getpeername (int, $arg2 *, $t *);
],[
$t len;
getpeername(0,0,&len);
],[
	rb_cv_socklen_t="$t"
	break
])
		done
	done

	if test "x$rb_cv_socklen_t" = x; then
		AC_MSG_WARN([Cannot find a type to use in place of socklen_t using int])
		rb_cv_socklen_t=int
	fi
	])
	AC_MSG_RESULT($rb_cv_socklen_t)],
	[#include <sys/types.h>
#include <sys/socket.h>])

rb_socklen_t=$rb_cv_socklen_t





AC_ARG_ENABLE(ipv6,AC_HELP_STRING([--disable-ipv6],[Disable IPv6 support]),[ipv6=$enableval],[ipv6=yes])

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

AC_CHECK_TYPES([struct sockaddr_storage],[
	rb_have_sockaddr_storage=yes
        ],[], [
		#include <sys/types.h>
		#include <sys/socket.h>
	])
                

dnl Check for stdarg.h - if we cant find it, halt configure
AC_CHECK_HEADER(stdarg.h, , [AC_MSG_ERROR([** stdarg.h could not be found - libratbox will not compile without it **])])

AC_CHECK_TYPE([sa_family_t], [],
[AC_DEFINE(sa_family_t, [u_int16_t], [If system does not define sa_family_t, define it here.])],
[[#include <sys/types.h>
#include <sys/socket.h>]])


dnl check for various functions...
AC_CHECK_FUNCS([socketpair gettimeofday writev sendmsg gmtime_r strtok_r usleep posix_spawn strlcpy strlcat strnlen fstat signalfd select poll kevent port_create epoll_ctl arc4random getrusage timerfd_create])	

AC_SEARCH_LIBS(nanosleep, rt posix4, AC_DEFINE(HAVE_NANOSLEEP, 1, [Define if you have nanosleep]))
AC_SEARCH_LIBS(timer_create, rt, AC_DEFINE(HAVE_TIMER_CREATE, 1, [Define if you have timer_create]))
RB_CHECK_TIMER_CREATE
RB_CHECK_TIMERFD_CREATE

AC_FUNC_ALLOCA
AC_FUNC_VFORK
AC_FUNC_MMAP

AC_MSG_CHECKING(for /dev/poll)
if test -c "/dev/poll"; then
        AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_DEVPOLL, [1], [Define to 1 if you have devpoll])
else
	AC_MSG_RESULT(no)	
fi

dnl OpenSSL support
AC_ARG_ENABLE(openssl, [
	AC_HELP_STRING([--enable-openssl],[Enable OpenSSL support.])
	AC_HELP_STRING([--disable-openssl],[Disable OpenSSL support.])
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
	AC_HELP_STRING([--enable-gnutls],[Enable GnuTLS support.])
	AC_HELP_STRING([--disable-gnutls],[Disable GnuTLS support.])
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

AC_MSG_CHECKING(if you want to do a profile build)
AC_ARG_ENABLE(profile,
AC_HELP_STRING([--enable-profile],[Enable profiling]),
[profile=$enableval], [profile=no])

if test "$profile" = yes; then
	if test "$ac_cv_c_compiler_gnu" = yes; then
		CFLAGS="$CFLAGS -pg -static"
		AC_MSG_RESULT([yes, adding -pg -static])
		AC_DEFINE(RATBOX_PROFILE, 1, [Defined to mark profiling is enabled])
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

CHARYBDIS_C_GCC_TRY_FLAGS([-Wall], charybdis_cv_c_gcc_w_all)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wextra], charybdis_cv_c_gcc_w_extra)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=pointer-arith], charybdis_cv_c_gcc_w_pointer_arith)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=implicit], charybdis_cv_c_gcc_w_implicit)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=nested-externs], charybdis_cv_c_gcc_w_nested_externs)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wcast-align], charybdis_cv_c_gcc_w_cast_align)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wcast-qual], charybdis_cv_c_gcc_w_cast_qual)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=write-strings], charybdis_cv_c_gcc_w_write_strings)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=error-implicit-function-declaration], charybdis_cv_c_gcc_w_error_implicit_function_declaration)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wstrict-prototypes], charybdis_cv_c_gcc_strict_prototypes)
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
CHARYBDIS_C_GCC_TRY_FLAGS([-Wunused-parameter], charybdis_cv_c_gcc_w_unused_variable)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wredundant-decls], charybdis_cv_c_gcc_w_redundant_decls)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=float-equal], charybdis_cv_c_gcc_w_float_equal)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=format], charybdis_cv_c_gcc_w_format)
CHARYBDIS_C_GCC_TRY_FLAGS([-Werror=format-y2k], charybdis_cv_c_gcc_w_format_y2k)
CHARYBDIS_C_GCC_TRY_FLAGS([-Wformat-security], charybdis_cv_c_gcc_w_no_format_security)

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

if test "x$rb_sockaddr_storage" = "xyes"; then
	echo '#define rb_sockaddr_storage sockaddr_storage' >> $outfile
else
	echo 'struct rb_sockaddr_storage { uint8_t _padding[[128]]; };' >> $outfile
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

	if test "x$rb_have_sockaddr_storage" = "xyes"; then
		rb_sockaddr_storage="yes" 
	else
		rb_sockaddr_storage="no"
	fi
	
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
echo "Assert debugging ............... $assert"
echo "Block allocator ................ $balloc"
echo "SSL Type........................ $SSL_TYPE"
echo


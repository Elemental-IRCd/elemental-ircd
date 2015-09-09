# RB_IF_REQUESTED(feature, if-required)
# -------------------------------------
dnl Check if a feature is not explicitly disabled
AC_DEFUN([RB_IF_REQUESTED], [
	if test x"[$]with_$1" != x"no" && test x"[$]enable_$1" != x"no"; then
		$2
	fi
])

dnl Check if a feature has been explicitly enabled
# RB_IF_REQUIRED(feature, if-required)
# ------------------------------------
AC_DEFUN([RB_IF_REQUIRED], [
	if test x"[$]with_$1" == x"yes" || test x"[$]enable_$1" == x"yes"; then
		$2
	fi
])


# Detect if we're targeting windows
AC_DEFUN([RB_DETECT_PLATFORM], [
AC_REQUIRE([AC_CANONICAL_HOST])
case "$host_os" in
mingw* )
	rb_platform="windows";;
*)
	rb_platform="unix";;
esac
])


dnl Shared CFLAGS settings
AC_DEFUN([RB_CFLAGS], [
  AX_APPEND_COMPILE_FLAGS([-Wall])
  AX_APPEND_COMPILE_FLAGS([-Wcast-align])
  AX_APPEND_COMPILE_FLAGS([-Wcast-qual])
  AX_APPEND_COMPILE_FLAGS([-Werror=implicit-function-declaration])
  AX_APPEND_COMPILE_FLAGS([-Werror=float-equal])
  AX_APPEND_COMPILE_FLAGS([-Werror=format])
  AX_APPEND_COMPILE_FLAGS([-Werror=format-y2k])
  AX_APPEND_COMPILE_FLAGS([-Werror=implicit])
  AX_APPEND_COMPILE_FLAGS([-Werror=missing-declarations])
  AX_APPEND_COMPILE_FLAGS([-Werror=missing-prototypes])
  AX_APPEND_COMPILE_FLAGS([-Werror=nested-externs])
  AX_APPEND_COMPILE_FLAGS([-Werror=packed])
  AX_APPEND_COMPILE_FLAGS([-Werror=pointer-arith])
  AX_APPEND_COMPILE_FLAGS([-Werror=shadow])
  AX_APPEND_COMPILE_FLAGS([-Wstrict-prototypes])
  AX_APPEND_COMPILE_FLAGS([-Werror=undef])
  AX_APPEND_COMPILE_FLAGS([-Werror=write-strings])
  AX_APPEND_COMPILE_FLAGS([-Wextra])
  AX_APPEND_COMPILE_FLAGS([-Wmissing-noreturn])
  # format-security is omited as non-literal printf formats are used extensively
  AX_APPEND_COMPILE_FLAGS([-Wno-format-security])
  # Unused parameters are common
  AX_APPEND_COMPILE_FLAGS([-Wno-unused-parameter])
  AX_APPEND_COMPILE_FLAGS([-Wredundant-decls])
  AX_APPEND_COMPILE_FLAGS([-Wunused-function])
  AX_APPEND_COMPILE_FLAGS([-Wunused-label])
  AX_APPEND_COMPILE_FLAGS([-Wunused-result])
  AX_APPEND_COMPILE_FLAGS([-Wunused-value])
  AX_APPEND_COMPILE_FLAGS([-Wunused-variable])
])


dnl IPv6 support macros..pretty much swiped from wget

dnl RB_PROTO_INET6

AC_DEFUN([RB_PROTO_INET6],[
  AC_CACHE_CHECK([for INET6 protocol support], [rb_cv_proto_inet6],[
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
        #include <sys/types.h>
        #include <sys/socket.h>

        #ifndef PF_INET6
        #error Missing PF_INET6
        #endif
        #ifndef AF_INET6
        #error Mlssing AF_INET6
        #endif
      ])],
      [rb_cv_proto_inet6=yes],
      [rb_cv_proto_inet6=no])
  ])

  if test "X$rb_cv_proto_inet6" = "Xyes"; then :
    $1
  else :
    $2  
  fi    
])      


AC_DEFUN([RB_TYPE_STRUCT_SOCKADDR_IN6],[
  rb_have_sockaddr_in6=
  AC_CHECK_TYPES([struct sockaddr_in6],[
    rb_have_sockaddr_in6=yes
  ],[
    rb_have_sockaddr_in6=no
  ],[
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
  ])

  if test "X$rb_have_sockaddr_in6" = "Xyes"; then :
    $1
  else :
    $2
  fi
])

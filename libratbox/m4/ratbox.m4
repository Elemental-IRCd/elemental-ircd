
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

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

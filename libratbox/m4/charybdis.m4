# $Id: aclocal.m4 3321 2007-03-30 23:32:43Z jilles $ - aclocal.m4 - Autoconf fun...
AC_DEFUN([AC_DEFINE_DIR], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  last_ac_define_dir=`eval echo [$]$2`
  ac_define_dir=`eval echo [$]last_ac_define_dir`
  ac_define_dir_counter=0
  while test "x[$]last_ac_define_dir" != "x[$]ac_define_dir"; do
    last_ac_define_dir="[$]ac_define_dir"
    ac_define_dir=`eval echo [$]last_ac_define_dir`
    AS_VAR_ARITH([ac_define_dir_counter], [$ac_define_dir_counter + 1])
    AS_VAR_IF([ac_define_dir_counter], [128],
	[AC_MSG_ERROR([detected recusive directory expansion when expanding $1=[$]$2: [$]ac_define_dir])
	break])
  done
  $1="$ac_define_dir"
  AC_SUBST($1)
  ifelse($3, ,
    AC_DEFINE_UNQUOTED($1, "$ac_define_dir"),
    AC_DEFINE_UNQUOTED($1, "$ac_define_dir", $3))
])

AC_DEFUN([AC_SUBST_DIR], [
        ifelse($2,,,$1="[$]$2")
        $1=`(
            test "x$prefix" = xNONE && prefix="$ac_default_prefix"
            test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
            eval echo \""[$]$1"\"
        )`
        AC_SUBST($1)
])

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libratbox/include
AM_LDFLAGS = -L$(top_srcdir)/libratbox/src
LDADD = -lratbox

bin_PROGRAMS = mkpasswd

if BUILD_VICONF
bin_PROGRAMS += viconf
endif

bin_SCRIPTS = genssl.sh

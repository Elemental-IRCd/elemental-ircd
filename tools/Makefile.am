AM_CPPFLAGS = -I../include -I../libratbox/include
LDADD = ../libratbox/src/libratbox.la

bin_PROGRAMS = mkpasswd

if BUILD_VICONF
bin_PROGRAMS += viconf
endif

bin_SCRIPTS = genssl.sh

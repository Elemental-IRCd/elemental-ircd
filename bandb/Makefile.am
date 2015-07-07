AM_CFLAGS = @SQLITE_CFLAGS@
AM_LDFLAGS = @SQLITE_LIBS@
AM_CPPFLAGS = -I../include -I../libratbox/include
LDADD = ../libratbox/src/libratbox.la

bin_PROGRAMS = bandb

bandb_SOURCES = bandb.c \
                rsdb_snprintf.c \
                rsdb_sqlite3.c

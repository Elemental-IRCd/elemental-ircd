
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libratbox/include
AM_CFLAGS = @SQLITE_CFLAGS@
AM_LDFLAGS = @SQLITE_LIBS@ -L$(top_srcdir)/libratbox/src
LDADD = -lratbox

bin_PROGRAMS = bandb

bandb_SOURCES = bandb.c \
                rsdb_sqlite3.c

AM_CPPFLAGS = -I../include -I../libratbox/include
LDADD = ../libratbox/src/libratbox.la -lsqlite3

bin_PROGRAMS = bandb bantool

bandb_SOURCES = bandb.c \
                rsdb_snprintf.c \
                rsdb_sqlite3.c

bantool_SOURCES = bantool.c \
                  rsdb_snprintf.c \
                  rsdb_sqlite3.c

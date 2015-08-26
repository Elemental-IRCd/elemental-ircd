AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I. -I../include @SSL_CFLAGS@
libratbox_la_LDFLAGS = @SSL_LIBS@  -avoid-version -no-undefined -export-symbols export-syms.txt
libratbox_la_LIBADD = @SSL_LIBS@

lib_LTLIBRARIES = libratbox.la

libratbox_la_SOURCES =            \
        platform/unix.c           \
        platform/windows.c        \
        crypt.c                   \
        balloc.c                  \
        commio.c                  \
        event.c                   \
        ratbox_lib.c              \
        rb_memory.c               \
        linebuf.c                 \
        snprintf.c                \
        tools.c                   \
        helper.c                  \
        rawbuf.c                  \
        patricia.c                \
        netio/devpoll.c           \
        netio/epoll.c             \
        netio/poll.c              \
        netio/ports.c             \
        netio/select.c            \
        netio/kqueue.c            \
        netio/win32.c

if WITH_OPENSSL
libratbox_la_SOURCES += ssl/openssl.c
endif
if WITH_GNUTLS
libratbox_la_SOURCES += ssl/gnutls.c
endif


AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I. -I../include @SSL_CFLAGS@ @GNUTLS_CFLAGS@

libratbox_la_SOURCES =            \
        platform/unix.c           \
        platform/windows.c        \
        ssl/openssl.c             \
        ssl/gnutls.c              \
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
        netio/win32.c             \
        arc4random.c

libratbox_la_LDFLAGS = @SSL_LIBS@ @GNUTLS_LIBS@  -avoid-version -no-undefined -export-symbols export-syms.txt
libratbox_la_LIBADD = @SSL_LIBS@ @GNUTLS_LIBS@
lib_LTLIBRARIES = libratbox.la


AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I. -I../include @SSL_CFLAGS@ @GNUTLS_CFLAGS@

libratbox_la_SOURCES =			\
	unix.c				\
	win32.c				\
	crypt.c				\
	balloc.c			\
	commio.c			\
	openssl.c			\
	gnutls.c			\
	event.c				\
	ratbox_lib.c			\
	rb_memory.c			\
	linebuf.c			\
	snprintf.c			\
	tools.c				\
	helper.c			\
	devpoll.c			\
	epoll.c				\
	poll.c				\
	ports.c				\
	select.c			\
	kqueue.c			\
	rawbuf.c			\
	patricia.c			\
	arc4random.c

libratbox_la_LDFLAGS = @SSL_LIBS@ @GNUTLS_LIBS@  -avoid-version -no-undefined -export-symbols export-syms.txt
libratbox_la_LIBADD = @SSL_LIBS@ @GNUTLS_LIBS@
lib_LTLIBRARIES = libratbox.la


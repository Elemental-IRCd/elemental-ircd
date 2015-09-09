AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I libratbox/m4

SUBDIRS = libratbox

if BUILD_LTDL
SUBDIRS += extra/libltdl
endif

SUBDIRS += src \
           ssld \
           bandb \
           tools \
           modules \
           extensions \
           help \
           doc \
           tests

logdir = @prefix@/logs

install-data-hook:
	test -d ${logdir} || mkdir -p ${logdir}

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I libratbox/m4

SUBDIRS = libratbox \
          src \
          ssld \
          bandb \
          tools \
          modules \
          extensions \
          help \
          doc

logdir = @prefix@/logs

install-data-hook:
	test -d ${logdir} || mkdir -p ${logdir}

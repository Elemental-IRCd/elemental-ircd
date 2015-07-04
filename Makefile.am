AUTOMAKE_OPTIONS = foreign
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

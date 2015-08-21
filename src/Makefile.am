AM_CPPFLAGS = @LTDLINCL@ -I$(top_srcdir)/include -I$(top_srcdir)/libratbox/include
AM_LDFLAGS  = -L$(top_srcdir)/libratbox/src -export-dynamic -avoid-version -no-undefined
AM_YFLAGS = -d
LDADD = @LIBLTDL@ -lratbox


bin_PROGRAMS = ircd
ircd_SOURCES =                  \
  bandbi.c                      \
  blacklist.c                   \
  cache.c                       \
  channel.c                     \
  chmode.c                      \
  class.c                       \
  client.c                      \
  extban.c                      \
  getopt.c                      \
  hash.c                        \
  hook.c                        \
  hostmask.c                    \
  irc_dictionary.c              \
  ircd.c                        \
  ircd_parser.y                 \
  ircd_lexer.l                  \
  ircd_signal.c                 \
  listener.c                    \
  logger.c                      \
  match.c                       \
  modules.c                     \
  monitor.c                     \
  newconf.c                     \
  numeric.c                     \
  operhash.c                    \
  packet.c                      \
  parse.c                       \
  privilege.c                   \
  res.c                         \
  reslib.c                      \
  reject.c                      \
  restart.c                     \
  s_auth.c                      \
  s_conf.c                      \
  s_newconf.c                   \
  s_serv.c                      \
  s_user.c                      \
  scache.c                      \
  send.c                        \
  snomask.c                     \
  sslproc.c                     \
  substitution.c                \
  supported.c                   \
  tgchange.c                    \
  version.c                     \
  whowas.c

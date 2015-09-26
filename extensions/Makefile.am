AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libratbox/include
AM_LDFLAGS  = -module -export-dynamic -avoid-version -no-undefined -shared
AM_LDFLAGS += -export-symbols-regex _mheader
LIBS += $(top_srcdir)/libratbox/src/libratbox.la
LIBS += $(top_srcdir)/src/libelemental.la

extensiondir=@moduledir@/extensions

extension_LTLIBRARIES =         \
    block_invalid_utf8.la       \
    chm_adminonly.la            \
    chm_nocaps.la               \
    chm_norepeat.la             \
    chm_operonly_compat.la      \
    chm_operonly.la             \
    chm_quietunreg_compat.la    \
    chm_sslonly_compat.la       \
    chm_sslonly.la              \
    createauthonly.la           \
    createoperonly.la           \
    example_module.la           \
    force_user_invis.la         \
    hurt.la                     \
    ip_cloaking.la              \
    m_42.la                     \
    m_adminwall.la              \
    m_cycle.la                  \
    m_findforwards.la           \
    m_force.la                  \
    m_identify.la               \
    m_mkpasswd.la               \
    m_oaccept.la                \
    m_ojoin.la                  \
    m_okick.la                  \
    m_olist.la                  \
    m_omode.la                  \
    m_opme.la                   \
    m_oquit.la                  \
    m_roleplay.la               \
    m_sendbans.la               \
    m_webirc.la                 \
    no_locops.la                \
    no_oper_invis.la            \
    sno_farconnect.la           \
    sno_globalkline.la          \
    sno_globaloper.la           \
    spy_admin_notice.la         \
    spy_info_notice.la          \
    spy_links_notice.la         \
    spy_motd_notice.la          \
    spy_stats_notice.la         \
    spy_stats_p_notice.la       \
    spy_trace_notice.la


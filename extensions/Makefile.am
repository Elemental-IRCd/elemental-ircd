AM_CPPFLAGS = -I../include -I../libratbox/include
AM_LDFLAGS = -shared -export-dynamic

.c.so:
	${CC} ${PICFLAGS} ${AM_CPPFLAGS} ${CPPFLAGS} ${CFLAGS} ${AM_LDFLAGS} ${LDFLAGS} $< -o $@

extensiondir=@moduledir@/extensions

extension_DATA = chm_adminonly.so         \
                 chm_operonly_compat.so   \
                 chm_operonly.so          \
                 chm_quietunreg_compat.so \
                 chm_sslonly_compat.so    \
                 chm_sslonly.so           \
                 createauthonly.so        \
                 createoperonly.so        \
                 example_module.so        \
                 extb_account.so          \
                 extb_canjoin.so          \
                 extb_channel.so          \
                 extb_extgecos.so         \
                 extb_oper.so             \
                 extb_realname.so         \
                 extb_server.so           \
                 extb_ssl.so              \
                 extb_usermode.so         \
                 force_user_invis.so      \
                 hurt.so                  \
                 ip_cloaking.so           \
                 m_42.so                  \
                 m_adminwall.so           \
                 m_cycle.so               \
                 m_findforwards.so        \
                 m_force.so               \
                 m_identify.so            \
                 m_mkpasswd.so            \
                 m_oaccept.so             \
                 m_ojoin.so               \
                 m_okick.so               \
                 m_olist.so               \
                 m_omode.so               \
                 m_opme.so                \
                 m_oquit.so               \
                 m_roleplay.so            \
                 m_sendbans.so            \
                 m_webirc.so              \
                 no_locops.so             \
                 no_oper_invis.so         \
                 sno_farconnect.so        \
                 sno_globalkline.so       \
                 sno_globaloper.so        \
                 sno_whois.so             \
                 spy_admin_notice.so      \
                 spy_info_notice.so       \
                 spy_links_notice.so      \
                 spy_motd_notice.so       \
                 spy_stats_notice.so      \
                 spy_stats_p_notice.so    \
                 spy_trace_notice.so


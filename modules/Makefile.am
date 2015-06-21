AM_CPPFLAGS = -I../include -I../libratbox/include

.c.so:
	${CC} ${PICFLAGS} ${AM_CPPFLAGS} ${CPPFLAGS} ${CFLAGS} -Wl,-shared,-export-dynamic ${LDFLAGS} $< -o $@

automoddir=@moduledir@/autoload

module_DATA =  core/m_ban.so             \
               core/m_die.so             \
               core/m_error.so           \
               core/m_join.so            \
               core/m_kick.so            \
               core/m_kill.so            \
               core/m_message.so         \
               core/m_metadata.so        \
               core/m_mode.so            \
               core/m_nick.so            \
               core/m_part.so            \
               core/m_quit.so            \
               core/m_server.so          \
               core/m_squit.so


automod_DATA = chm_nocolour.so           \
               m_accept.so               \
               m_admin.so                \
               m_away.so                 \
               m_capab.so                \
               m_cap.so                  \
               m_certfp.so               \
               m_chghost.so              \
               m_close.so                \
               m_cmessage.so             \
               m_connect.so              \
               m_dline.so                \
               m_encap.so                \
               m_etrace.so               \
               m_help.so                 \
               m_info.so                 \
               m_invite.so               \
               m_ison.so                 \
               m_kline.so                \
               m_knock.so                \
               m_links.so                \
               m_list.so                 \
               m_locops.so               \
               m_lusers.so               \
               m_map.so                  \
               m_monitor.so              \
               m_motd.so                 \
               m_names.so                \
               m_oper.so                 \
               m_operspy.so              \
               m_pass.so                 \
               m_ping.so                 \
               m_pong.so                 \
               m_post.so                 \
               m_privs.so                \
               m_rehash.so               \
               m_restart.so              \
               m_resv.so                 \
               m_sasl.so                 \
               m_scan.so                 \
               m_services.so             \
               m_set.so                  \
               m_signon.so               \
               m_snote.so                \
               m_stats.so                \
               m_svinfo.so               \
               m_tb.so                   \
               m_testline.so             \
               m_testmask.so             \
               m_time.so                 \
               m_topic.so                \
               m_trace.so                \
               m_unreject.so             \
               m_userhost.so             \
               m_user.so                 \
               m_users.so                \
               m_version.so              \
               m_wallops.so              \
               m_whois.so                \
               m_who.so                  \
               m_whowas.so               \
               m_xline.so                \
               sno_routing.so

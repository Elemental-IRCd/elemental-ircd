#
#  elemental-ircd: Most of an ircd
#  numeric.tc: Symbolic names for irc numerics
#
#  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
#  Copyright (C) 1996-2002 Hybrid Development Team
#  Copyright (C) 2002-2004 ircd-ratbox development team
#  Copyright (C) 2015 elemental-ircd development team
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
#  USA
#

# This file was derived from include/numeric.h

set RPL_WELCOME             {001}
set RPL_YOURHOST            {002}
set RPL_CREATED             {003}
set RPL_MYINFO              {004}
set RPL_ISUPPORT            {005}

set RPL_SNOMASK             {008}

set RPL_REDIR               {010}
set RPL_MAP                 {015}
set RPL_MAPMORE             {016}
set RPL_MAPEND              {017}
set RPL_SAVENICK            {043}

set RPL_TRACELINK           {200}
set RPL_TRACECONNECTING     {201}
set RPL_TRACEHANDSHAKE      {202}
set RPL_TRACEUNKNOWN        {203}
set RPL_TRACEOPERATOR       {204}
set RPL_TRACEUSER           {205}
set RPL_TRACESERVER         {206}
set RPL_TRACENEWTYPE        {208}
set RPL_TRACECLASS          {209}

set RPL_STATSLINKINFO       {211}
set RPL_STATSCOMMANDS       {212}
set RPL_STATSCLINE          {213}
set RPL_STATSNLINE          {214}
set RPL_STATSILINE          {215}
set RPL_STATSKLINE          {216}
set RPL_STATSQLINE          {217}
set RPL_STATSYLINE          {218}
set RPL_ENDOFSTATS          {219}

set RPL_STATSPLINE          {220}
set RPL_UMODEIS             {221}

set RPL_STATSFLINE          {224}
set RPL_STATSDLINE          {225}

set RPL_SERVLIST            {234}
set RPL_SERVLISTEND         {235}

set RPL_STATSLLINE          {241}
set RPL_STATSUPTIME         {242}
set RPL_STATSOLINE          {243}
set RPL_STATSHLINE          {244}

set RPL_STATSSLINE          {245}
set RPL_STATSXLINE          {247}
set RPL_STATSULINE          {248}
set RPL_STATSDEBUG          {249}
set RPL_STATSCONN           {250}
set RPL_LUSERCLIENT         {251}
set RPL_LUSEROP             {252}
set RPL_LUSERUNKNOWN        {253}
set RPL_LUSERCHANNELS       {254}
set RPL_LUSERME             {255}
set RPL_ADMINME             {256}
set RPL_ADMINLOC1           {257}
set RPL_ADMINLOC2           {258}
set RPL_ADMINEMAIL          {259}

set RPL_TRACELOG            {261}
set RPL_ENDOFTRACE          {262}
set RPL_LOAD2HI             {263}

set RPL_LOCALUSERS          {265}
set RPL_GLOBALUSERS         {266}

set RPL_PRIVS               {270}

set RPL_WHOISCERTFP         {276}

set RPL_ACCEPTLIST          {281}
set RPL_ENDOFACCEPT         {282}

set RPL_NONE                {300}
set RPL_AWAY                {301}
set RPL_USERHOST            {302}
set RPL_ISON                {303}
set RPL_TEXT                {304}
set RPL_UNAWAY              {305}
set RPL_NOWAWAY             {306}

set RPL_WHOISUSER           {311}
set RPL_WHOISSERVER         {312}
set RPL_WHOISOPERATOR       {313}

set RPL_WHOWASUSER          {314}
set RPL_ENDOFWHOWAS         {369}

set RPL_WHOISCHANOP         {316}
set RPL_WHOISIDLE           {317}

set RPL_ENDOFWHOIS          {318}
set RPL_WHOISCHANNELS       {319}

set RPL_LISTSTART           {321}
set RPL_LIST                {322}
set RPL_LISTEND             {323}
set RPL_CHANNELMODEIS       {324}
set RPL_CHANNELMLOCK        {325}

set RPL_CHANNELURL          {328}

set RPL_CREATIONTIME        {329}
set RPL_WHOISLOGGEDIN       {330}

set RPL_NOTOPIC             {331}
set RPL_TOPIC               {332}
set RPL_TOPICWHOTIME        {333}

set RPL_WHOISBOT            {335}

set RPL_WHOISACTUALLY       {338}

set RPL_INVITING            {341}
set RPL_SUMMONING           {342}

set RPL_INVITELIST          {346}
set RPL_ENDOFINVITELIST     {347}
set RPL_EXCEPTLIST          {348}
set RPL_ENDOFEXCEPTLIST     {349}

set RPL_VERSION             {351}

set RPL_WHOREPLY            {352}
set RPL_WHOSPCRPL           {354}
set RPL_ENDOFWHO            {315}
set RPL_NAMREPLY            {353}
set RPL_WHOWASREAL          {360}
set RPL_ENDOFNAMES          {366}

set RPL_KILLDONE            {361}
set RPL_CLOSING             {362}
set RPL_CLOSEEND            {363}
set RPL_LINKS               {364}
set RPL_ENDOFLINKS          {365}
set RPL_BANLIST             {367}
set RPL_ENDOFBANLIST        {368}

set RPL_INFO                {371}
set RPL_MOTD                {372}
set RPL_INFOSTART           {373}
set RPL_ENDOFINFO           {374}
set RPL_MOTDSTART           {375}
set RPL_ENDOFMOTD           {376}
set RPL_WHOISHOST           {378}

set RPL_WHOISMODES          {379}

set RPL_YOUREOPER           {381}
set RPL_REHASHING           {382}
set RPL_MYPORTIS            {384}
set RPL_NOTOPERANYMORE      {385}
set RPL_RSACHALLENGE        {386}

set RPL_TIME                {391}
set RPL_USERSSTART          {392}
set RPL_USERS               {393}
set RPL_ENDOFUSERS          {394}
set RPL_NOUSERS             {395}
set RPL_HOSTHIDDEN          {396}

set ERR_NOSUCHNICK          {401}
set ERR_NOSUCHSERVER        {402}
set ERR_NOSUCHCHANNEL       {403}
set ERR_CANNOTSENDTOCHAN    {404}
set ERR_TOOMANYCHANNELS     {405}
set ERR_WASNOSUCHNICK       {406}
set ERR_TOOMANYTARGETS      {407}
set ERR_NOORIGIN            {409}

set ERR_INVALIDCAPCMD       {410}

set ERR_NORECIPIENT         {411}
set ERR_NOTEXTTOSEND        {412}
set ERR_NOTOPLEVEL          {413}
set ERR_WILDTOPLEVEL        {414}

set ERR_TOOMANYMATCHES      {416}

set ERR_UNKNOWNCOMMAND      {421}
set ERR_NOMOTD              {422}
set ERR_NOADMININFO         {423}
set ERR_FILEERROR           {424}

set ERR_NONICKNAMEGIVEN     {431}
set ERR_ERRONEUSNICKNAME    {432}
set ERR_NICKNAMEINUSE       {433}
set ERR_BANNICKCHANGE       {435}
set ERR_NICKCOLLISION       {436}
set ERR_UNAVAILRESOURCE     {437}
set ERR_NICKTOOFAST         {438}

set ERR_SERVICESDOWN        {440}
set ERR_USERNOTINCHANNEL    {441}
set ERR_NOTONCHANNEL        {442}
set ERR_USERONCHANNEL       {443}
set ERR_NOLOGIN             {444}
set ERR_SUMMONDISABLED      {445}
set ERR_USERSDISABLED       {446}

set ERR_NOINVITE            {447}

set ERR_NONICK              {449}

set ERR_NOTREGISTERED       {451}

set ERR_ACCEPTFULL          {456}
set ERR_ACCEPTEXIST         {457}
set ERR_ACCEPTNOT           {458}

set ERR_NEEDMOREPARAMS      {461}
set ERR_ALREADYREGISTRED    {462}
set ERR_NOPERMFORHOST       {463}
set ERR_PASSWDMISMATCH      {464}
set ERR_YOUREBANNEDCREEP    {465}
set ERR_YOUWILLBEBANNED     {466}
set ERR_KEYSET              {467}

set ERR_LINKCHANNEL         {470}
set ERR_CHANNELISFULL       {471}
set ERR_UNKNOWNMODE         {472}
set ERR_INVITEONLYCHAN      {473}
set ERR_BANNEDFROMCHAN      {474}
set ERR_BADCHANNELKEY       {475}
set ERR_BADCHANMASK         {476}
set ERR_NEEDREGGEDNICK      {477}
set ERR_BANLISTFULL         {478}
set ERR_BADCHANNAME         {479}

set ERR_THROTTLE            {480}

set ERR_NOPRIVILEGES        {481}
set ERR_CHANOPRIVSNEEDED    {482}
set ERR_CANTKILLSERVER      {483}
set ERR_ISCHANSERVICE       {484}
set ERR_BANNEDNICK          {485}
set ERR_NONONREG            {486}

set ERR_VOICENEEDED         {489}

set ERR_NOOPERHOST          {491}

set ERR_NOCTCP              {492}

set ERR_KICKNOREJOIN        {495}

set ERR_OWNMODE             {494}

set ERR_UMODEUNKNOWNFLAG    {501}
set ERR_USERSDONTMATCH      {502}

set ERR_GHOSTEDCLIENT       {503}

set ERR_USERNOTONSERV       {504}

set ERR_WRONGPONG           {513}

set ERR_DISABLED            {517}

set ERR_NOKICK              {519}

set ERR_HELPNOTFOUND        {524}

set RPL_WHOISSECURE         {671}
set RPL_WHOISWEBIRC         {672}

set RPL_MODLIST             {702}
set RPL_ENDOFMODLIST        {703}

set RPL_HELPSTART           {704}
set RPL_HELPTXT             {705}
set RPL_ENDOFHELP           {706}

set ERR_TARGCHANGE          {707}

set RPL_ETRACEFULL          {708}
set RPL_ETRACE              {709}

set RPL_KNOCK               {710}
set RPL_KNOCKDLVR           {711}

set ERR_TOOMANYKNOCK        {712}
set ERR_CHANOPEN            {713}
set ERR_KNOCKONCHAN         {714}
set ERR_KNOCKDISABLED       {715}

set ERR_TARGUMODEG          {716}
set RPL_TARGNOTIFY          {717}
set RPL_UMODEGMSG           {718}

set RPL_OMOTDSTART          {720}
set RPL_OMOTD               {721}
set RPL_ENDOFOMOTD          {722}

set ERR_NOPRIVS             {723}

set RPL_TESTMASK            {724}
set RPL_TESTLINE            {725}
set RPL_NOTESTLINE          {726}
set RPL_TESTMASKGECOS       {727}

set RPL_QUIETLIST           {728}
set RPL_ENDOFQUIETLIST      {729}

set RPL_MONONLINE           {730}
set RPL_MONOFFLINE          {731}
set RPL_MONLIST             {732}
set RPL_ENDOFMONLIST        {733}
set ERR_MONLISTFULL         {734}

set ERR_NOCOMMONCHAN        {737}

set RPL_RSACHALLENGE2       {740}
set RPL_ENDOFRSACHALLENGE2  {741}

set ERR_MLOCKRESTRICTED     {742}

set RPL_SCANMATCHED         {750}
set RPL_SCANUMODES          {751}

set RPL_LOGGEDIN            {900}
set RPL_LOGGEDOUT           {901}
set ERR_NICKLOCKED          {902}

set RPL_SASLSUCCESS         {903}
set ERR_SASLFAIL            {904}
set ERR_SASLTOOLONG         {905}
set ERR_SASLABORTED         {906}
set ERR_SASLALREADY         {907}

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

array set numerics {}

proc def_numeric {name num} {
    global $name
    set $name $num

    global numerics
    set numerics($num) $name
}

def_numeric RPL_WELCOME             {001}
def_numeric RPL_YOURHOST            {002}
def_numeric RPL_CREATED             {003}
def_numeric RPL_MYINFO              {004}
def_numeric RPL_ISUPPORT            {005}

def_numeric RPL_SNOMASK             {008}

def_numeric RPL_REDIR               {010}
def_numeric RPL_MAP                 {015}
def_numeric RPL_MAPMORE             {016}
def_numeric RPL_MAPEND              {017}
def_numeric RPL_YOURID              {042}
def_numeric RPL_SAVENICK            {043}

def_numeric RPL_TRACELINK           {200}
def_numeric RPL_TRACECONNECTING     {201}
def_numeric RPL_TRACEHANDSHAKE      {202}
def_numeric RPL_TRACEUNKNOWN        {203}
def_numeric RPL_TRACEOPERATOR       {204}
def_numeric RPL_TRACEUSER           {205}
def_numeric RPL_TRACESERVER         {206}
def_numeric RPL_TRACENEWTYPE        {208}
def_numeric RPL_TRACECLASS          {209}

def_numeric RPL_STATSLINKINFO       {211}
def_numeric RPL_STATSCOMMANDS       {212}
def_numeric RPL_STATSCLINE          {213}
def_numeric RPL_STATSNLINE          {214}
def_numeric RPL_STATSILINE          {215}
def_numeric RPL_STATSKLINE          {216}
def_numeric RPL_STATSQLINE          {217}
def_numeric RPL_STATSYLINE          {218}
def_numeric RPL_ENDOFSTATS          {219}

def_numeric RPL_STATSPLINE          {220}
def_numeric RPL_UMODEIS             {221}

def_numeric RPL_STATSFLINE          {224}
def_numeric RPL_STATSDLINE          {225}

def_numeric RPL_SERVLIST            {234}
def_numeric RPL_SERVLISTEND         {235}

def_numeric RPL_STATSLLINE          {241}
def_numeric RPL_STATSUPTIME         {242}
def_numeric RPL_STATSOLINE          {243}
def_numeric RPL_STATSHLINE          {244}

def_numeric RPL_STATSSLINE          {245}
def_numeric RPL_STATSXLINE          {247}
def_numeric RPL_STATSULINE          {248}
def_numeric RPL_STATSDEBUG          {249}
def_numeric RPL_STATSCONN           {250}
def_numeric RPL_LUSERCLIENT         {251}
def_numeric RPL_LUSEROP             {252}
def_numeric RPL_LUSERUNKNOWN        {253}
def_numeric RPL_LUSERCHANNELS       {254}
def_numeric RPL_LUSERME             {255}
def_numeric RPL_ADMINME             {256}
def_numeric RPL_ADMINLOC1           {257}
def_numeric RPL_ADMINLOC2           {258}
def_numeric RPL_ADMINEMAIL          {259}

def_numeric RPL_TRACELOG            {261}
def_numeric RPL_ENDOFTRACE          {262}
def_numeric RPL_LOAD2HI             {263}

def_numeric RPL_LOCALUSERS          {265}
def_numeric RPL_GLOBALUSERS         {266}

def_numeric RPL_PRIVS               {270}

def_numeric RPL_WHOISCERTFP         {276}

def_numeric RPL_ACCEPTLIST          {281}
def_numeric RPL_ENDOFACCEPT         {282}

def_numeric RPL_NONE                {300}
def_numeric RPL_AWAY                {301}
def_numeric RPL_USERHOST            {302}
def_numeric RPL_ISON                {303}
def_numeric RPL_TEXT                {304}
def_numeric RPL_UNAWAY              {305}
def_numeric RPL_NOWAWAY             {306}

def_numeric RPL_WHOISUSER           {311}
def_numeric RPL_WHOISSERVER         {312}
def_numeric RPL_WHOISOPERATOR       {313}

def_numeric RPL_WHOWASUSER          {314}
def_numeric RPL_ENDOFWHOWAS         {369}

def_numeric RPL_WHOISCHANOP         {316}
def_numeric RPL_WHOISIDLE           {317}

def_numeric RPL_ENDOFWHOIS          {318}
def_numeric RPL_WHOISCHANNELS       {319}

def_numeric RPL_LISTSTART           {321}
def_numeric RPL_LIST                {322}
def_numeric RPL_LISTEND             {323}
def_numeric RPL_CHANNELMODEIS       {324}
def_numeric RPL_CHANNELMLOCK        {325}

def_numeric RPL_CHANNELURL          {328}

def_numeric RPL_CREATIONTIME        {329}
def_numeric RPL_WHOISLOGGEDIN       {330}

def_numeric RPL_NOTOPIC             {331}
def_numeric RPL_TOPIC               {332}
def_numeric RPL_TOPICWHOTIME        {333}

def_numeric RPL_WHOISBOT            {335}

def_numeric RPL_WHOISACTUALLY       {338}

def_numeric RPL_INVITING            {341}
def_numeric RPL_SUMMONING           {342}

def_numeric RPL_INVITELIST          {346}
def_numeric RPL_ENDOFINVITELIST     {347}
def_numeric RPL_EXCEPTLIST          {348}
def_numeric RPL_ENDOFEXCEPTLIST     {349}

def_numeric RPL_VERSION             {351}

def_numeric RPL_WHOREPLY            {352}
def_numeric RPL_WHOSPCRPL           {354}
def_numeric RPL_ENDOFWHO            {315}
def_numeric RPL_NAMREPLY            {353}
def_numeric RPL_WHOWASREAL          {360}
def_numeric RPL_ENDOFNAMES          {366}

def_numeric RPL_KILLDONE            {361}
def_numeric RPL_CLOSING             {362}
def_numeric RPL_CLOSEEND            {363}
def_numeric RPL_LINKS               {364}
def_numeric RPL_ENDOFLINKS          {365}
def_numeric RPL_BANLIST             {367}
def_numeric RPL_ENDOFBANLIST        {368}

def_numeric RPL_INFO                {371}
def_numeric RPL_MOTD                {372}
def_numeric RPL_INFOSTART           {373}
def_numeric RPL_ENDOFINFO           {374}
def_numeric RPL_MOTDSTART           {375}
def_numeric RPL_ENDOFMOTD           {376}
def_numeric RPL_WHOISHOST           {378}

def_numeric RPL_WHOISMODES          {379}

def_numeric RPL_YOUREOPER           {381}
def_numeric RPL_REHASHING           {382}
def_numeric RPL_MYPORTIS            {384}
def_numeric RPL_NOTOPERANYMORE      {385}
def_numeric RPL_RSACHALLENGE        {386}

def_numeric RPL_TIME                {391}
def_numeric RPL_USERSSTART          {392}
def_numeric RPL_USERS               {393}
def_numeric RPL_ENDOFUSERS          {394}
def_numeric RPL_NOUSERS             {395}
def_numeric RPL_HOSTHIDDEN          {396}

def_numeric ERR_NOSUCHNICK          {401}
def_numeric ERR_NOSUCHSERVER        {402}
def_numeric ERR_NOSUCHCHANNEL       {403}
def_numeric ERR_CANNOTSENDTOCHAN    {404}
def_numeric ERR_TOOMANYCHANNELS     {405}
def_numeric ERR_WASNOSUCHNICK       {406}
def_numeric ERR_TOOMANYTARGETS      {407}
def_numeric ERR_NOORIGIN            {409}

def_numeric ERR_INVALIDCAPCMD       {410}

def_numeric ERR_NORECIPIENT         {411}
def_numeric ERR_NOTEXTTOSEND        {412}
def_numeric ERR_NOTOPLEVEL          {413}
def_numeric ERR_WILDTOPLEVEL        {414}

def_numeric ERR_TOOMANYMATCHES      {416}

def_numeric ERR_UNKNOWNCOMMAND      {421}
def_numeric ERR_NOMOTD              {422}
def_numeric ERR_NOADMININFO         {423}
def_numeric ERR_FILEERROR           {424}

def_numeric ERR_NONICKNAMEGIVEN     {431}
def_numeric ERR_ERRONEUSNICKNAME    {432}
def_numeric ERR_NICKNAMEINUSE       {433}
def_numeric ERR_BANNICKCHANGE       {435}
def_numeric ERR_NICKCOLLISION       {436}
def_numeric ERR_UNAVAILRESOURCE     {437}
def_numeric ERR_NICKTOOFAST         {438}

def_numeric ERR_SERVICESDOWN        {440}
def_numeric ERR_USERNOTINCHANNEL    {441}
def_numeric ERR_NOTONCHANNEL        {442}
def_numeric ERR_USERONCHANNEL       {443}
def_numeric ERR_NOLOGIN             {444}
def_numeric ERR_SUMMONDISABLED      {445}
def_numeric ERR_USERSDISABLED       {446}

def_numeric ERR_NOINVITE            {447}

def_numeric ERR_NONICK              {449}

def_numeric ERR_NOTREGISTERED       {451}

def_numeric ERR_ACCEPTFULL          {456}
def_numeric ERR_ACCEPTEXIST         {457}
def_numeric ERR_ACCEPTNOT           {458}

def_numeric ERR_NEEDMOREPARAMS      {461}
def_numeric ERR_ALREADYREGISTRED    {462}
def_numeric ERR_NOPERMFORHOST       {463}
def_numeric ERR_PASSWDMISMATCH      {464}
def_numeric ERR_YOUREBANNEDCREEP    {465}
def_numeric ERR_YOUWILLBEBANNED     {466}
def_numeric ERR_KEYSET              {467}

def_numeric ERR_LINKCHANNEL         {470}
def_numeric ERR_CHANNELISFULL       {471}
def_numeric ERR_UNKNOWNMODE         {472}
def_numeric ERR_INVITEONLYCHAN      {473}
def_numeric ERR_BANNEDFROMCHAN      {474}
def_numeric ERR_BADCHANNELKEY       {475}
def_numeric ERR_BADCHANMASK         {476}
def_numeric ERR_NEEDREGGEDNICK      {477}
def_numeric ERR_BANLISTFULL         {478}
def_numeric ERR_BADCHANNAME         {479}

def_numeric ERR_THROTTLE            {480}

def_numeric ERR_NOPRIVILEGES        {481}
def_numeric ERR_CHANOPRIVSNEEDED    {482}
def_numeric ERR_CANTKILLSERVER      {483}
def_numeric ERR_ISCHANSERVICE       {484}
def_numeric ERR_BANNEDNICK          {485}
def_numeric ERR_NONONREG            {486}

def_numeric ERR_VOICENEEDED         {489}

def_numeric ERR_NOOPERHOST          {491}

def_numeric ERR_NOCTCP              {492}

def_numeric ERR_KICKNOREJOIN        {495}

def_numeric ERR_OWNMODE             {494}

def_numeric ERR_UMODEUNKNOWNFLAG    {501}
def_numeric ERR_USERSDONTMATCH      {502}

def_numeric ERR_GHOSTEDCLIENT       {503}

def_numeric ERR_USERNOTONSERV       {504}

def_numeric ERR_WRONGPONG           {513}

def_numeric ERR_DISABLED            {517}

def_numeric ERR_NOKICK              {519}

def_numeric ERR_HELPNOTFOUND        {524}

def_numeric RPL_WHOISSECURE         {671}
def_numeric RPL_WHOISWEBIRC         {672}

def_numeric RPL_MODLIST             {702}
def_numeric RPL_ENDOFMODLIST        {703}

def_numeric RPL_HELPSTART           {704}
def_numeric RPL_HELPTXT             {705}
def_numeric RPL_ENDOFHELP           {706}

def_numeric ERR_TARGCHANGE          {707}

def_numeric RPL_ETRACEFULL          {708}
def_numeric RPL_ETRACE              {709}

def_numeric RPL_KNOCK               {710}
def_numeric RPL_KNOCKDLVR           {711}

def_numeric ERR_TOOMANYKNOCK        {712}
def_numeric ERR_CHANOPEN            {713}
def_numeric ERR_KNOCKONCHAN         {714}
def_numeric ERR_KNOCKDISABLED       {715}

def_numeric ERR_TARGUMODEG          {716}
def_numeric RPL_TARGNOTIFY          {717}
def_numeric RPL_UMODEGMSG           {718}

def_numeric RPL_OMOTDSTART          {720}
def_numeric RPL_OMOTD               {721}
def_numeric RPL_ENDOFOMOTD          {722}

def_numeric ERR_NOPRIVS             {723}

def_numeric RPL_TESTMASK            {724}
def_numeric RPL_TESTLINE            {725}
def_numeric RPL_NOTESTLINE          {726}
def_numeric RPL_TESTMASKGECOS       {727}

def_numeric RPL_QUIETLIST           {728}
def_numeric RPL_ENDOFQUIETLIST      {729}

def_numeric RPL_MONONLINE           {730}
def_numeric RPL_MONOFFLINE          {731}
def_numeric RPL_MONLIST             {732}
def_numeric RPL_ENDOFMONLIST        {733}
def_numeric ERR_MONLISTFULL         {734}

def_numeric ERR_NOCOMMONCHAN        {737}

def_numeric RPL_RSACHALLENGE2       {740}
def_numeric RPL_ENDOFRSACHALLENGE2  {741}

def_numeric ERR_MLOCKRESTRICTED     {742}

def_numeric RPL_SCANMATCHED         {750}
def_numeric RPL_SCANUMODES          {751}

def_numeric RPL_WHOISKEYVALUE       {760}
def_numeric RPL_KEYVALUE            {761}
def_numeric RPL_METADATAEND         {762}
def_numeric ERR_METADATALIMIT       {764}
def_numeric ERR_TARGETINVALID       {765}
def_numeric ERR_NOMATCHINGKEY       {766}
def_numeric ERR_KEYINVALID          {767}
def_numeric ERR_KEYNOTSET           {768}
def_numeric ERR_KEYNOPERMISSION     {769}

def_numeric RPL_LOGGEDIN            {900}
def_numeric RPL_LOGGEDOUT           {901}
def_numeric ERR_NICKLOCKED          {902}

def_numeric RPL_SASLSUCCESS         {903}
def_numeric ERR_SASLFAIL            {904}
def_numeric ERR_SASLTOOLONG         {905}
def_numeric ERR_SASLABORTED         {906}
def_numeric ERR_SASLALREADY         {907}

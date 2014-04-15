# TODO

## elemental-ircd 6.5.2

 - [ ] Fix `extended-join` not triggering for users on other servers
 - [x] Patch `m_sasl`, http://seclists.org/fulldisclosure/2014/Mar/320
 - [x] Fix null reference in `src/channel.c`
 - [x] Add basic autoconfiguration script
 - [x] Add modules for services packages
 - [x] Make end user oper failures more generic
 - [x] Add `OWNER=` to `005`
 - [x] Show own modes in a `WHOIS`
 - [x] Remove AHBL blacklists from default configs
 - [x] Remove `SNOTE`s in `m_post`
 - [x] Make flooding `SNOTE`s global
 - [x] Add quotes around `PART` reason
 - [x] Disable `away-notify` for now

## elemental-ircd 6.6

 - [ ] Finish websocket support
   - [x] Configuration for websocket ports
   - [x] HTTP parsing of websocket data
   - [ ] Client connections via websockets
 - [x] Send cloaked host as METADATA
 - [x] Fix a flaw in the kick permission logic
 - [x] Fix segfault on `autojoin_opers`
 - [x] Update many of the helpfiles with the proper permissions
 - [ ] extban by certfp

### Services Modules

 - [ ] Support METADATA for the cloaked host
   - [ ] Anope 2.0
   - [ ] Atheme
 - [ ] Have /hs off re-set the cloaked host instead of the user's real hostmask
   - [ ] Anope 2.0
   - [ ] Atheme


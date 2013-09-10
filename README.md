# ponychat-ircd

Ponychat-ircd is a high performance, lightweight and scalable IRC daemon. It has
its roots in ShadowIRCd and Charybdis and offers channel operators more options
as well as for server administrators to have better separation of power and
allows for easier management. It the preferred IRC daemon for PonyChat.

## Supported Platforms

 * Linux
   * Debian
   * Centos/Amazon Linux
   * Alpine Linux
 * FreeBSD

## Build Directions

1. Ensure your system has the equivalents of `openssl-dev`, `flex`, `bison`,
   and the compiler toolkit. 
2. Download the [stable tarball](https://github.com/PonyChat/ponychat-ircd/archive/ponychat-ircd-6.3.4.tar.gz)
   and extract it somewhere on your system.
3. `./configure --prefix=$HOME/ircd`
4. `make`
5. `make install`
6. Copy the example config `example.conf` to `ircd.conf` in `~/ircd/etc`
7. Edit the configuration to match your needs
8. Run `bin/ircd` and have people connect up as normal

## Support

Our support channel is `#rainbow-factory` on `irc.ponychat.net`. If you like you
can join us on our webchat [by clicking here](http://webchat.ponychat.net/?autojoin=%23#rainbow-factory).

Please note that running ponychat-ircd in weird or otherwise insane configurations
may make us unable to help you.

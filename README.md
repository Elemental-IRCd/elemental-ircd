# Elemental-IRCd

**Elemental-IRCd** is a high performance, lightweight, and scalable 
IRC daemon. It is a fork of the now-defunct ShadowIRCD and seeks to continue in 
the direction ShadowIRCD was headed.

## Supported Platforms

All modern \*NIX systems generally should work. You need the equivalent of the 
following Debian packages:

 - `libssl-dev`
 - `autoconf`
 - `autoconf-archive`
 - `automake`
 - `flex`
 - `bison`
 - `build-essential`
 - `libsqlite3-dev`
 - `libltdl-dev`
 - `libtool`
 - `pkg-config`

For an easy one-step install:

### Ubuntu/Debian

```console
$ sudo apt-get install libssl-dev autoconf autoconf-archive
                       build-essential flex bison libsqlite3-dev
                       automake libtool libltdl-dev pkg-config
```

### Fedora/CentOS

```console
$ sudo (yum/dnf) groupinstall "Development Tools"
$ sudo (yum/dnf) install openssl-devel autoconf automake flex bison
                         autoconf-archive sqlite-devel libtool-ltdl-devel
                         libtool pkgconfig tcl tcllib tcltls
```

### OSX

```console
$ brew install autoconf-archive
$ brew install sqlite
$ export PKG_CONFIG_PATH=/usr/local/opt/sqlite/lib/pkgconfig
```

### FreeBSD
Untested, may break
```console
$ pkg install autotools autoconf-archive pkgconf tcl86 tcllib tcltls
              flex bison sqlite3 libltdl openssl
```

## Others

```
Cassy | If you put something on a platform which cannot support it
      | it may tip and fall and become broken. Simple physics.
```

## Compilation

Compilation will generally follow:

```console
$ ./autogen.sh
$ ./configure [add configure options here]
$ make
$ make install
```

## Support

The official channel for Elemental-IRCd is `#elemental-ircd` on 
`irc.yolo-swag.com`. Anyone with push access to the repository will have halfop 
(`+h`, `%`) or higher.

Atheme and Anope (1.8 and 2.0) modules are included in the source tree of 
Elemental-IRCd. For most cases the default `protocol/elemental-ircd` module in 
Atheme should be fine, but this version will always be more up-to-date.

To report bugs, please use the GitHub issue tracker.

# Elemental-IRCd

**Elemental-IRCd** is a high performance, lightweight, and scalable
IRC daemon. It is a fork of the now-defunct ShadowIRCD and seeks to continue in
the direction ShadowIRCD was headed.

## Supported Platforms

All modern \*NIX systems should work. You need the equivalent of the following
Debian packages:

 - `libssl-dev`
 - `flex`
 - `bison`
 - `build-essential`
 - `libsqlite3-dev`

```
Cassy | If you put something on a platform which cannot support it
      | it may tip and fall and become broken. Simple physics.
```

Read the included documentation for detailed compilation and install
directions.

## Support

The official channel for Elemental-IRCd is `#elemental-ircd` on
`irc.yolo-swag.com`. Anyone with push access to the repository will have halfop
(`+h`, `%`) or higher.

Atheme and Anope (1.8 and 2.0) modules are included in the source tree of
Elemental-IRCd. For most cases the default `protocol/elemental-ircd` module in
Atheme should be fine, but this version will always be more up-to-date.

To report bugs, please use the GitHub issue tracker.

# Installation

Tlf now comes with automake and autoconf.
Starting with TLF-1.0.0 you need pkg-config too.

## Build Dependencies

Building TLF requires some other components to be installed first. 
It depends on

 * the _Hamlib_ library for controlling your radio,
 * _ncurses_ and _tinfo_ for text screen handling,
 * the _XMLRPC_C_ library for communication with programs like _FlDigi_ and
 * _Glib-2.0_ for supporting C functions.

If you are using a distribution of the _Debian_ family make sure to install
the needed header files also with

```
sudo apt install libglib2.0-dev libhamlib-dev libncurses5-dev libtinfo-dev
libxmlrpc-core-c3-dev
```

While not a strict build dependency installing the following packages is
helpful too:

 * _sox_ for audio signal handling during SSB contests and
 * _xplanet_ which allows you to see the latest DX spots on the globe.


## Quick Install

The easiest way to install tlf is by downloading the latest tarball, then navigating your Terminal to the directory where you unpacked it, and typing:

```
./configure
make
sudo make install
```

If you are doing a lot of contesting in digimodes using Fldigi, there is support for reading the audio frequency via xmlrpc. In that case, start the sequence above with

```
./configure --enable-fldigi-xmlrpc
```

followed by the make and make install commands.

If you are compiling tlf from a clone of the repo, please do

```
autoreconf --install
```

before the above commands.

By default, tlf will install into /usr/local/bin, together with the
shell scripts from the scripts directory. Data files will install into
/usr/local/share/tlf.

If you want to change any of these paths, you can do this with the
configure script too, e.g. './configure --prefix=/usr --datadir=/usr/share'
will install tlf and scripts into /usr/bin, datafiles into /usr/share/tlf.

You should have read and write permissions on /tmp/tlf. Everything else can be read-only.

If you want to use cwdaemon for CW, make sure it's installed, and either set it up to start at system startup, or plan to manually start it before launching TLF. 

If you want to use a K1EL Winkeyer for CW, download and install either [winkeyer_server](https://github.com/ok2cqr/winkeyer_server) from OK2CQR or N0NB's [winkeydaemon](https://github.com/N0NB/winkeydaemon)

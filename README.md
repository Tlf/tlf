# TLF ham radio contest logger

[![Build Status](https://github.com/Tlf/tlf/actions/workflows/ci-build.yml/badge.svg)](https://github.com/Tlf/tlf/actions/workflows/ci-build.yml)

## Summary

TLF is a logging program for radiosport, or ham radio contests. It supports
CQWW, CQ-WPX, ARRL-DX, ARRL-FD, PACC, and EU SPRINT contests as well as many
more, and also has general QSO and DXpedition modes. Users can add new
contests by editing a text file.

TLF interfaces with a morse code generator, your sound card, a huge number of
radios, and with a DX Cluster. It can project cluster data into the excellent
Xplanet program, written by Hari Nair.

Contest operation mimics the popular TR-Log program for DOS, and the output
file  is TR- as well as CABRILLO compatible. The user interface was designed
with over 30 years of experience in CW contesting, and a group of active
contesters continues to update the code. 

TLF runs in console mode (a terminal window), allowing it to run on smaller
machines, or remotely with limited bandwidth. Don't let the "retro" look of
the console fool you, though; this is a fully modern contest logger that can
support everything from a part time single op effort to a large multi-multi
"big gun" operation.

See [Manual.md](doc/Manual.md) for an explanation of the user interface
and detailed descriptions of setups for CW, RTTY, and SSB contesting.
For specific questions once you've started using TLF,
simply type `man tlf` at the command line.

## Basic principles

As a console program, there are no menus or pop-up windows in TLF. Instead, it
uses two configuration files: `logcfg.dat` and a contest-specific rules file.
Starting from an empty directory, it will read `<PREFIX>/logcfg.dat`, where
`<PREFIX>` is the directory where the data files were installed
(`/usr/local/share/tlf` by default). Any `logcfg.dat` or rules file stored in
the local directory where TLF launches will override the default settings,
which you probably want to do.
See the [**Quick Start**](#quick-start) guide below for an
example setup. The default `logcfg.dat` file has extensive comments
documenting the settings, and can be opened in any text editor. At a minimum,
you'll need to set your callsign.

## Installation

The easiest way to install TLF is from your distribution's repo. On
Debian-flavored distributions (including Ubuntu):

```
sudo apt install tlf
```

If you choose this route, you can skip the "Building from source" section.

If you want to use [cwdaemon](https://github.com/acerion/cwdaemon) for CW, make
sure it's installed, and either set it up to start at system startup, or plan to
start it manually before launching TLF. 

If you want to use a K1EL Winkeyer for CW, download and install either
[winkeyer_server](https://github.com/ok2cqr/winkeyer_server) or
[winkeydaemon](https://github.com/N0NB/winkeydaemon), following the instructions
on those pages. You'll need to start the Winkeyer server (either one) before
starting TLF. Both programs work by impersonating cwdaemon, which TLF will use
automatically if you've enabled the NETKEYER settings in the `logcfg.dat` file.

### Building from source

If you'd prefer to build TLF from source, or if it's not in your repo, it
requires some other components to be installed first.

Specifically, it depends on:

* the `hamlib` library for controlling your radio,
* `ncurses` and `tinfo` for text screen handling,
* the `XMLRPC_C` library for communication with programs like `Fldigi`
* `Glib-2.0` for supporting C functions.

If you are using a distribution of the Debian family (including Ubuntu) you
can get all of these dependencies by opening a Terminal window and typing:

```
sudo apt install libglib2.0-dev libhamlib-dev libncurses5-dev libtinfo-dev libxmlrpc-core-c3-dev
```

While not strict build dependencies, the following packages may be helpful
too:

* `sox` for audio signal handling during SSB contests and
* `xplanet` which allows you to see the latest DX spots on the globe.

On Debian Linuxes:

```
sudo apt install sox xplanet
```

Once the dependencies are installed, the easiest way to get TLF's source is by
downloading the latest tarball (version 1.4.1) from
[here](http://download.savannah.gnu.org/releases/tlf/tlf-1.4.1.tar.gz), then
navigating your Terminal to the directory where you unpacked it, and typing:

```
./configure
make
sudo make install
```

If you plan to do contests in digimodes using Fldigi, start the sequence above
with

```
./configure --enable-fldigi-xmlrpc
```

followed by the `make` and `make install` commands.

If you are compiling TLF from a clone of the Github repository instead of the
tarball, please do

```
autoreconf --install
```

before the above commands.

By default, TLF will install into `/usr/local/bin`, together with the
shell scripts from the scripts directory. Data files will install into
`/usr/local/share/tlf`.

If you want to change any of these paths, you can do that with the
configure script too, e.g. 

```
./configure --prefix=/usr --datadir=/usr/share
```

will install TLF and scripts into `/usr/bin`, datafiles into `/usr/share/tlf`.

## Quick start

TLF can be used in all types of contests, but regardless of the mode, much of
the initial setup will be the same. This example will assume a USA-based
station is setting up for the ARRL DX contest - simply substitute the
appropriate rules file for whatever contest you're entering.

The easiest way to keep things organized is to create a new directory for each
contest, then copy the default configuration files into it. You can do this in
your file manager, or at the command line:

```
mkdir arrldx

cd arrldx

mkdir rules

cp /usr/share/tlf/logcfg.dat .

cp /usr/share/tlf/rules/arrldx_usa rules/
```

Note that the file paths above are based on a Debian installation from the
repo. If you compiled from source instead of installing from your repo, your
`share` directory may instead be `/usr/local/share/tlf`.

Now we have a directory called `arrldx` with a `logcfg.dat` file in it, and an
`arrldx/rules` directory with `arrldx_usa` in it. These will override the
defaults when we launch TLF.

Open the local `logcfg.dat` file in your favorite text editor. The comments in
the file explain the features. Set `RULES=arrldx_usa`, uncomment any settings
you want to enable, comment (#) any you want to disable, and enter specifics
such as your callsign and preferred console-based text editor. The latter will
be used to edit this file from inside TLF.

If you plan to generate CW with TLF, you'll need either cwdaemon or a Winkeyer
server running, and for digital modes you'll need Fldigi. Rig control requires
hamlib. See [`Manual.md`](doc/Manual.md) for details.
If you want DXcluster spots to show up on a band map in TLF,
enter your preferred settings in the "PACKET INTERFACE"
section and also uncomment the CLUSTER line under "INFORMATION WINDOWS." 

When done with `logcfg.dat`, go to your local copy of `rules/arrldx_usa` and
edit it. There shouldn't be much to do besides set your exchange (unless you
happen to be in PA) and make any changes you like to the CW messages if you're
using them. The same procedure applies to any other rules file. If you want to
work a contest that doesn't currently have a rules file, copy one that's
similar and modify it - then please let us know, so we can add it to the
collection.

Once the configuration files are to your liking, make sure the terminal window
is set to 80x25 size and launch TLF from your contest directory:

```
tlf
```

(or `/usr/local/bin/tlf` if you want to be explicit).

As a console-based logger, TLF relies exclusively on keyboard commands.
Commands are either key chords (such as `Alt-H` to bring up the help screen),
or text entered directly into the empty callsign entry field (such as `:help`
to open the complete list of keyboard commands). Entering a number such as
14050 will set the current frequency in kilohertz, and if rig control is
active TLF will automatically tune the rig to that frequency. The F-keys work
as expected for sending contest macros; an abbreviated list of their current
settings is across the top line of the console.

If you've used other modern contest loggers, such as N1MM+, the keyboard
behavior will be very familiar: it's the standard "Enter Sends Message (ESM)"
operation. The spacebar will take you to the exchange field after entering a
callsign, and the Enter key triggers different macros depending on which mode
you're in (Run vs. Search and Pounce) and which part of the QSO is happening.

The default mode is "Log," equivalent to "Run" mode in other contest loggers.
You'll see the word "Log" in the upper left corner, and TLF will assume you're
calling CQ and responding to whoever calls back. In this mode, hitting "Enter"
in the blank callsign field will send your CQ. 

To toggle between "Log" and "Search and Pounce" mode, hit "+", and note that
the upper left corner now says "S&P". Now hitting "Enter" in the blank
callsign field will send your callsign (the same as the F-6 message by
default) instead of your CQ, and the rest of the Search and Pounce logging
flow will follow.

## Manual

For a complete walkthrough and more details on specific types of operations,
see [`doc/Manual.md`](doc/Manual.md), which we're in the process of revising.
Answers to specific questions can usually be found
in the built-in manual accessible from any console window:

```
man tlf
```

A copy of the old TLF manual is also available in the Github
repository [here](https://tlf.github.io/tlfdoc.old/tlfdoc.html). While many
new features have been added, much of the old information will still work.

## Contributing

If you have coding skills and want to help with the project, feel free to fork
the repo, make changes, and send a patch or pull request. 

## Bugs and feature requests

Please direct bug reports, feature requests, and questions to the [mailing
list](https://lists.nongnu.org/mailman/listinfo/tlf-devel).

## Credits

Thanks to Joop, PA4TU for the help with the make files and the cwdaemon.

Thanks to Ivo, 9A3TY for the serial port /dev/cwkeyer device.

Thanks to Eric, PA3FKN for the parallel port /dev/cwkeyer device.


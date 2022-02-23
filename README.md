# Tlf ham radio contest logger

[![Build Status](https://github.com/Tlf/tlf/actions/workflows/ci-build.yml/badge.svg)](https://github.com/Tlf/tlf/actions/workflows/ci-build.yml)

## Summary

Tlf is a console (ncurses) mode general purpose CW/VOICE keyer, logging and contest program for hamradio. 

It supports the CQWW, the WPX, the ARRL-DX , the ARRL-FD, the PACC and the EU SPRINT contests (single operator) as well as a LOT MORE basic contests, general QSO and DXpedition mode. 

It interfaces with a morse code generator, your sound card, a number of radios, and with a DX Cluster. 

Tlf can project cluster data into the excellent Xplanet program, written by Hari Nair.

Contest operation mimics the popular TR-Log program for DOS, the output file  is TR- as well as CABRILLO compatible. The user interface was designed with over 30 years of experience in CW contesting. 

The program was written for console mode on purpose, to make it run also on smaller machines, or remotely via a modem link. 

See `man tlf` for detailed documentation. See also doc/README for details and a guide to setting up, and the INSTALL file for a quick installation guide.

## Contributing

* See the homepage at https://Tlf.github.io for more information
* There is a mailing list at tlf-devel@nongnu.org (see 
  http://lists.nongnu.org/mailman/listinfo/tlf-devel)
* If you want to contribute, fork the repo, make changes and send us a patch or pull request.


## Basic setup and configuration

As a console program, there are no menus or pop-up windows in TLF. Instead, it uses two configuration files: logcfg.dat and a contest-specific rules file. Starting from an empty directory, it will read *PREFIX*/logcfg.dat, where *PREFIX* is the directory where the data files were installed (/usr/local/share/tlf by default). Any logcfg.dat or rules file stored in the local directory where TLF launches will override the default settings, which you probably want to do. See the quick start guide below for an example setup. The default logcfg.dat file has extensive comments documenting the settings, and can be opened in any text editor. At a minimum, you'll need to set your callsign.

## Installation

See INSTALL.md

## Quick start

TLF can conrol the radio, generate CW, play audio files for SSB contests, and interface with Fldigi for digital modes. Regardless of the mode, much of the initial setup will be the same. This example will assume a USA-based station is setting up for the ARRL DX contest - simply substitute the appropriate rules file for whatever contest you're entering.

The easiest way to keep things organized is to create a new directory for each contest, then copy the default configuration files into it:

```
mkdir arrldx

cd arrldx

mkdir rules

cp /usr/local/share/tlf/logcfg.dat .

cp /usr/local/share/tlf/rules/arrldx_usa rules/
```

Now we have a directory called `arrldx` with a `logcfg.dat` file in it, and a `arrldx/rules` directory with `arrldx_usa` in it. These will override the defaults when we launch TLF.

Open the local `logcfg.dat` file in your favorite text editor. The comments in the file explain the features. Set `RULES=arrldx_usa`, uncomment any settings you want to enable, comment (#) any you want to disable, and enter specifics such as your callsign and preferred console-based text editor. The latter will be used to edit this file from inside TLF.

If you plan to generate CW with TLF, you'll need either cwdaemon or a Winkeyer server running, and for digital modes you'll need Fldigi. Rig control requires hamlib. See the full documentation (`doc/Manual.md`) for details. If you want DXcluster spots to show up on a band map in TLF, enter your preferred settings in the "PACKET INTERFACE" section and also uncomment the CLUSTER line under "INFORMATION WINDOWS." 

When done with `logcfg.dat`, go to your local copy of `rules/arrldx_usa` and edit it. There shouldn't be much to do besides set your exchange (unless you happen to be in PA) and make any changes you like to the CW messages if you're using them. The same procedure applies to any other rules file. If you want to work a contest that doesn't currently have a rules file, copy one that's similar and modify it - then please let us know, so we can add it to the collection.

Once the configuration files are to your liking, make sure the terminal window is set to 80x25 size and launch TLF from your contest directory:

```
tlf
```

(or `/usr/local/bin/tlf` if you want to be explicit).

As a console-based logger, TLF relies exclusively on keyboard commands. Commands are either key chords (such as `Alt-H` to bring up the help screen), or text entered directly into the empty callsign entry field (such as `:help` to open the complete list of keyboard commands). Entering a number such as 14050 will set the current frequency in kilohertz, and if rig control is active TLF will automatically tune the rig to that frequency. The F-keys work as expected for sending contest macros; an abbreviated list of their current settings is across the top line of the console. 

The default mode is "Log," equivalent to "Run" mode in other contest loggers. You'll see the word "Log" in the upper left corner, and TLF will assume you're calling CQ and responding to whoever calls back. Hitting "Enter" will send your CQ. Entering a callsign and hitting "Enter" again will send the received callsign and your exchange, and move the cursor to the exchange entry field. Entering the incoming exchange and hitting "Enter" will send a "thank you" and restart the run cycle.

To toggle between "Log" and "Search and Pounce" mode, hit "+", and note that the upper left corner now says "S&P". Now entering a callsign and hitting "Enter" will send your callsign (your F-6 message by default). Once you've copied the exchange, hitting "Enter" will send your exchange.

## Manual

We are in the process of updating the full TLF documentation. For the latest documentation, use:

```
man tlf
```

Specific documentation for particular operations appears in appropriately named folders in the `doc` directory, e.g. README.ssb and README.rtty. A copy of the old manual is also available in the Github repository [here](https://tlf.github.io/tlfdoc.old/tlfdoc.html). While many new features have been added, most of the old information will still work.

## Contributions

Thanks to Joop, PA4TU for the help with the make files and the cwdaemon.

Thanks to Ivo, 9A3TY for the serial port /dev/cwkeyer device.

Thanks to Eric, PA3FKN for the parallel port /dev/cwkeyer device.

## Bugs and problems

Please direct bug reports, feature requests, and questions to the [mailing list](https://lists.nongnu.org/mailman/listinfo/tlf-devel).
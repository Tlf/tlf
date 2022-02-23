
# Basic setup and configuration

As a console program, there are no menus or pop-up windows in TLF. Instead, it uses two configuration files: logcfg.dat and a contest-specific rules file. Starting from an empty directory, it will read <PREFIX>logcfg.dat, where <PREFIX> is the directory where the data files were installed (/usr/local/share/tlf by default). Any logcfg.dat or rules file stored in the local directory where TLF launches will override the default settings, which you probably want to do. See the quick start guide below for an example setup. The default logcfg.dat file has extensive comments documenting the settings, and can be opened in any text editor. At a minimum, you'll need to set your callsign.

## Installation

See INSTALL.md

## Quick start

TLF can generate CW, play audio files for SSB contests, or interface with Fldigi for digital modes. 

Regardless of the mode, much of the initial setup will be the same. This example will assume a USA-based station is setting up for the ARRL DX contest - simply substitute the appropriate rules file for whatever contest you're working.

First, we create a working directory somewhere convenient, and copy the default configuration files into it:

```
mkdir contest

cd contest

mkdir rules

cp /usr/local/share/tlf/logcfg.dat .

cp /usr/local/share/tlf/rules/arrldx_usa rules
```

Now open logcfg.dat in your favorite text editor, uncomment any settings you want to enable, comment (#) any you want to disable, and enter specifics such as your callsign and preferred console-based text editor. The latter will be used to edit the config file from inside TLF.

When done with logcfg.dat, go to your local copy of rules/arrldx_usa and edit it. There shouldn't be much to do besides set your exchange (unless you happen to be in PA) and make any changes you like to the CW messages if you're using them.

Once the configuration files are to your liking, make sure the terminal window is set to 80x25 size and launch TLF from your contest directory:

```
tlf
```

(or /usr/local/bin/tlf if you want to be explicit).

## Alternative packet setup

TLF can also run packet in a separate terminal. To link this to the tlf program
start a telnet session from the working directory with:

telnet <network node> | tee -a  clfile

In case your packet program is on your own machine, use

telnet localhost | tee -a clfile

Now you have a separate packet terminal where you can e.g. start "call", telnet  or
or "minicom" and connect to your favorite dx cluster, or telnet to a cluster
on the internet.

Activate "FIFOINTERFACE".

Activate the cluster display in tlf with :cluster, :spot, or :map
You can toggle the announcements filter with :filter

## CW

See the manual... I prefer to use the cwdaemon for the parallel port.

## Manual

We are in the process of updating the full TLF documentation. In the meantime, the old manual is available in the Github repository [here](https://tlf.github.io/tlfdoc.old/tlfdoc.html).


## Contributions

Thanks to Joop, PA4TU for the help with the make files and the cwdaemon..
Thanks to Ivo, 9A3TY for the serial port /dev/cwkeyer device.
Thanks to Eric, PA3FKN for the parallel port /dev/cwkeyer device.

## Bugs and problems

Please direct bug reports, feature requests, and questions to the [mailing list](https://lists.nongnu.org/mailman/listinfo/tlf-devel).
# TLF: Full Documentation

This will be the updated version of the full documentation.

Pasted from the old README:

## Alternative packet cluster setup

TLF can also run packet spots in a separate terminal. To link this to the tlf program
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

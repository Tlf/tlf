#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
import os
import time
import random

gmfsklog = os.path.join(os.environ['HOME'], "gMFSK.log.1")

def linebegin():
    return "RX 1000 : RTTY (%sZ): " % (time.strftime("%Y-%m-%d %H:%M", time.gmtime()))

def genrandqtc(tlf):
    #nrofline = random.randint(5, 10)
    nrofline = 10
    serial = random.randint(10, 300)
    pos = random.randint(0, len(tlf)-(nrofline+1))
    qtc = ["%d/%d\n" % (serial, nrofline)]
    for i in range(pos, pos+nrofline):
	fields = filter(None, [f for f in tlf[i].strip().split(" ")])
	tfield = "".join(fields[2].split(":"))
	qtc.append(" ".join([tfield, fields[4], fields[7] + "\n"]))
    return qtc

def writelog(f, line):
    f.write(line)
    f.flush()

if len(sys.argv) < 2:
    print "Argument missing!"
    print "Use: %s /path/to/tlfcontest.log" % (sys.argv[0])
    sys.exit(1)

tlflogfile = sys.argv[1]

try:
    f = open(tlflogfile, "r")
    tlf = f.readlines()
    f.close()
except:
    print "Couldn't open tlf logfile: %s" % (tlflogfile)
    sys.exit()

f = open(gmfsklog, "a")

writelog(f, "--- Logging started at %s UTC ---\n" % (time.strftime("%a, %d %b %H:%M:%S %Y UTC", time.gmtime())))

for qtc in genrandqtc(tlf):
    writelog(f, linebegin())
    for c in qtc:
	writelog(f, c)
	time.sleep(8/45.45)

writelog(f, "--- Logging stopped at %s UTC ---\n" % (time.strftime("%a, %d %b %H:%M:%S %Y UTC", time.gmtime())))
    

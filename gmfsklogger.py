#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
import os
import time
import random

gmfsklog = os.path.join(os.environ['HOME'], "gMFSK.log")

random.seed()

def linebegin():
    return "RX 1000 : RTTY (%sZ): " % (time.strftime("%Y-%m-%d %H:%M", time.gmtime()))

def genrandqtc(tlf, has_qrm):
    nrofline = random.randint(5, 10)
    #nrofline = 10
    serial = random.randint(10, 300)
    pos = random.randint(0, len(tlf)-(nrofline+1))
    qrm = random.randint(1, 65535)%2
    qserial = str(serial)
    if qrm == 1 and has_qrm:
	qserial = qserial + chr(random.randint(65, 91))
    qtc = ["QTC %s/%d QTC %d/%d\n" % (qserial, nrofline, serial, nrofline)]
    seps = [" ", "/", "-"]
    sep = seps[random.randint(0, len(seps)-1)]
    for i in range(pos, pos+nrofline):
	fields = filter(None, [f for f in tlf[i].strip().split(" ")])
	qrm = (random.randint(1, 65535)%2)
	tfield = "".join(fields[2].split(":"))
	if qrm == 1 and has_qrm:
	    badchar = random.randint(1, 11)%2
	    if badchar == 0:
		tfield += chr(random.randint(65, 91))
	    else:
		tfield += chr(random.randint(48, 58))
	qrm = random.randint(1, 11)%2
	ts = []
	if qrm == 1 and has_qrm:
	    while len(fields[4])+len(ts) < 16:
		ts.append(chr(random.randint(65, 91)))
	fields[4] = fields[4] + "".join(ts)
	qtc.append(sep.join([tfield, fields[4], fields[7] + "\n"]))
    return qtc

def writelog(f, line):
    f.write(line)
    f.flush()

if len(sys.argv) < 2:
    print "Argument missing!"
    print "Use: %s /path/to/tlfcontest.log" % (sys.argv[0])
    sys.exit(1)

tlflogfile = sys.argv[1]
if len(sys.argv) >= 3 and sys.argv[2] == "qrm":
    has_qrm = True
else:
    has_qrm = False

try:
    f = open(tlflogfile, "r")
    tlf = f.readlines()
    f.close()
except:
    print "Couldn't open tlf logfile: %s" % (tlflogfile)
    sys.exit()

f = open(gmfsklog, "a")

#writelog(f, "--- Logging started at %s UTC ---\n" % (time.strftime("%a, %d %b %H:%M:%S %Y UTC", time.gmtime())))
#writelog(f, linebegin())
#writelog(f, "START QTC\n")

l = genrandqtc(tlf, has_qrm)
for qtc in l:
    writelog(f, linebegin())
    for c in qtc:
	writelog(f, c)
	time.sleep((8/45.45))

#writelog(f, linebegin())
#writelog(f, "END QTC\n")
writelog(f, linebegin())
writelog(f, "\n")
#writelog(f, "--- Logging stopped at %s UTC ---\n" % (time.strftime("%a, %d %b %H:%M:%S %Y UTC", time.gmtime())))
    

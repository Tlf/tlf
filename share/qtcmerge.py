#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
import time

if len(sys.argv) < 2:
    print "Argument missing! Use: %s CALLSIGN.cbr" % (sys.argv[0])
    sys.exit(1)

cbr = sys.argv[1]
sqtc = []
rqtc = []

band_hash = {
  160: "1800",
   80: "3500",
   40: "7000",
   20: "14000",
   15: "21000",
   10: "28000"
}

try:
    f = open(cbr, "r")
    lines = f.readlines()
    f.close()
except:
    print "Can't open %s!" % (cbr)
    sys.exit(2)

callsign = ""
for l in lines:
    f = l.split(" ")
    if f[0] == "CALLSIGN:":
        callsign = f[1].strip()
        break

try:
    f = open("QTC_sent.log", "r")
    qtcl = [l for l in f.readlines()]
    for l in qtcl:
	fshift = 0
	fields = filter(None, [fl for fl in l.split(" ")])

	if len(fields) == 12:
	    fshift = 1
	qtc = ['QTC:', "%s" % band_hash[(int(l[0:3]))].rjust(5, ' '), l[3:5], time.strftime("%Y-%m-%d %H%M", time.strptime(fields[3] + " " + fields[4], "%d-%b-%y %H:%M")), fields[5+fshift].ljust(13, ' ')]
	qtc.append("%s" % ("%d/%d" % (int(fields[6+fshift]), int(fields[7+fshift]))).ljust(10, " "))
	qtc.append("%s" % (callsign.ljust(13, ' ')))
	qtc.append("%s" % (fields[8+fshift]))
	qtc.append("%s" % (fields[9+fshift].ljust(13, ' ')))
	qtc.append("%s" % (fields[10+fshift].strip()))
	sqtc.append(" ".join(qtc))
    f.close()
except:
    print "Can't open or parse QTC_sent.log"

try:
    f = open("QTC_recv.log", "r")
    qtcl = [l for l in f.readlines()]
    for l in qtcl:
	fshift = 0
	fields = filter(None, [fl for fl in l.split(" ")])

	if len(fields) == 11:
	    fshift = 1
	qtc = ['QTC:', "%s" % band_hash[(int(l[0:3]))].rjust(5, ' '), l[3:5], time.strftime("%Y-%m-%d %H%M", time.strptime(fields[2] + " " + fields[3], "%d-%b-%y %H:%M")), "%s" % callsign.ljust(13, ' ')]
	qtc.append("%s" % ("%d/%d" % (int(fields[5+fshift]), int(fields[6+fshift]))).ljust(10, " "))
	qtc.append("%s" % (fields[4+fshift].ljust(13, ' ')))
	qtc.append("%s" % (fields[7+fshift]))
	qtc.append("%s" % (fields[8+fshift].ljust(13, ' ')))
	qtc.append("%s" % (fields[9+fshift].strip()))
	rqtc.append(" ".join(qtc))
    f.close()
except:
    print "Can't open or parse QTC_recv.log"

fo = open(cbr.replace(".cbr", "_QTC.cbr"), "w+")
for l in lines[:-1]:
    f = filter(None, [fl for fl in l.split(" ")])
    fo.write(l)
if len(rqtc) > 0:
    for q in rqtc:
	fo.write(q + "\n")
if len(sqtc) > 0:
    for q in sqtc:
	fo.write(q + "\n")

fo.write(lines[-1])

fo.close()

sys.exit(0)


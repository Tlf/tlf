#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
<<<<<<< HEAD
import time
=======

def timeinc(hourmin):
      h, m = hourmin[0:2], hourmin[2:4]
      if int(m) == 59:
	  m = "00"
	  if int(h) == 23:
	      h = "00"
	  else:
	      h = "%02d" % (int(h)+1)
      else:
	  m = "%02d" % (int(m)+1)

      return "".join([h, m])
>>>>>>> d21e8332bd3c8c0558887cffb346ff99b35b8f95

if len(sys.argv) < 2:
    print "Argument missing! Use: %s CALLSIGN.cbr" % (sys.argv[0])
    sys.exit(1)

cbr = sys.argv[1]
<<<<<<< HEAD
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
=======
sqtc = {}
rqtc = {}

>>>>>>> d21e8332bd3c8c0558887cffb346ff99b35b8f95
try:
    f = open(cbr, "r")
    lines = f.readlines()
    f.close()
except:
    print "Can't open %s!" % (cbr)
    sys.exit(2)

<<<<<<< HEAD
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
=======
try:
    f = open("QTC_sent.log", "r")
    qtcl = [l.strip() for l in f.readlines()]
    for l in qtcl:
	fields = filter(None, [fl for fl in l.split(" ")])
	qsonr = int(fields[2])
	if not sqtc.has_key(qsonr):
	    sqtc[qsonr] = {'serial': int(fields[0]), 'count': 0, 'qtcs': []}
	sqtc[qsonr]['qtcs'].append("%s %s %4s" % (fields[3], fields[4].ljust(13), fields[5]))
	sqtc[qsonr]['count'] += 1
    f.close()
except:
    print "Can't open QTC_sent.log"
    sys.exit(2)

try:
    f = open("QTC_recv.log", "r")
    qtcl = [l.strip() for l in f.readlines()]
    for l in qtcl:
	fields = filter(None, [fl for fl in l.split(" ")])
	qsonr = int(fields[0])
	if not rqtc.has_key(qsonr):
	    rqtc[qsonr] = {'serial': int(fields[1]), 'count': 0, 'qtcs': []}
	rqtc[qsonr]['qtcs'].append("%s %s %4s" % (fields[3], fields[4].ljust(13), fields[5]))
	rqtc[qsonr]['count'] += 1
    f.close()
except:
    print "Can't open QTC_recv.log"
    sys.exit(2)

fo = open(cbr.replace(".cbr", "_QTC.cbr"), "w+")
for l in lines:
    f = filter(None, [fl for fl in l.split(" ")])
    fo.write(l)
    if f[0] == "QSO:":
	qsonr = int(f[7])
	if sqtc.has_key(qsonr):
	    f[4] = timeinc(f[4])
	    for q in sqtc[qsonr]['qtcs']:
		s = "QTC: %5s %s %s %04d %s %3d/%d     %s %s\n" % (f[1], f[2], f[3], int(f[4]), f[8].ljust(13), int(sqtc[qsonr]['serial']), int(sqtc[qsonr]['count']), f[5].ljust(13), q)
		fo.write(s)
	if rqtc.has_key(qsonr):
	    f[4] = timeinc(f[4])
	    for q in rqtc[qsonr]['qtcs']:
		s = "QTC: %5s %s %s %04d %s %3d/%d     %s %s\n" % (f[1], f[2], f[3], int(f[4]), f[5].ljust(13), int(rqtc[qsonr]['serial']), int(rqtc[qsonr]['count']), f[8].ljust(13), q)
		fo.write(s)

>>>>>>> d21e8332bd3c8c0558887cffb346ff99b35b8f95
fo.close()

sys.exit(0)


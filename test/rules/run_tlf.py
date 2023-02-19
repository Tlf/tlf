#!/usr/bin/env python3

import sys
import pexpect

open('.paras', 'a').close()    # create .paras to skip first time greeting

rc = 0

command = "../../../src/tlf -nrv"
fout = open("run.log", "wb")

p = pexpect.spawn(command)
p.logfile = fout

i = p.expect(["continue", "save it", pexpect.TIMEOUT], timeout=2);
if i == 0:
    rc = 0
elif i == 1:
    rc = 1
    p.send('N') # do not save log
    p.expect("continue", timeout=2);
else:
    rc = 2

fout.close()

sys.exit(rc)


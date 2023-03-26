#!/usr/bin/env python3

import sys
import os
import shutil
import glob
import pexpect

TLF = "../../../src/tlf"

# first check if python plugin would be used and skip test if not compiled in
if glob.glob('rules/*.py'):
    rc = os.system(f"{TLF} -? | grep Features: | grep -q python-plugin")
    if rc != 0:
        sys.exit(3)     # skip this one


COMMAND = f"{TLF} -nrv"
fout = None
#fout = open("run.log", "wb")   # uncomment this line to get interaction log
PARAS = '.paras'

open(PARAS, 'a').close()    # create .paras to skip first time greeting

rc = 0

p = pexpect.spawn(COMMAND)
if fout:
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

if fout:
    fout.close()

os.remove(PARAS)
shutil.rmtree("rules/__pycache__", ignore_errors=True)

sys.exit(rc)


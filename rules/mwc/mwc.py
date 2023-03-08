"""
OK1WC Memorial (MWC)
https://memorial-ok1wc.cz/index.php?page=rules2l
"""

import re

# match trailing call modifier (e.g. /8, /P, /MM, /QRP, etc.)
MODIFIER_PATTERN = re.compile('/(\d|[A-Z]+)$')

def check_exchange(qso):
    call = MODIFIER_PATTERN.sub('', qso.call)   # remove modifier
    if len(call) > 1:
        mult = call[-1]     # last character of the call
    else:
        mult = ''

    return {'mult1_value': mult}


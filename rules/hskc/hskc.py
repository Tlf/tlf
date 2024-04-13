"""
Hungarian Straight Key Contest
https://hskc.ha8kux.com/en/contest-rules
"""
import re

MULT_PATTERN = re.compile(r"""
    (                       # either
        ([0-9][A-Z])[A-Z]*  # the last digit of the prefix and the next letter (first letter of the suffix)
        |                   # or
        ([0-9A-Z]{2})       # the last two characters of the callsign
    )
    (/([0-9]|[A-Z]+))?      # optional trailing call modifier - ignored
    $                       # end of string
    """, re.VERBOSE)


# QSO with a Category A station 3 points, with a Category B station 1 point
def score(qso):
    if qso.exchange.strip().endswith('A'):
        return 3
    else:
        return 1


def check_exchange(qso):
    m = MULT_PATTERN.search(qso.call)
    mult = ''
    if m:
        if m.group(2):
            mult = m.group(2)
        elif m.group(3):
            mult = m.group(3)

    return {'mult1_value': mult}


"""
Marconi Club QSO Party Day
http://www.ariloano.it/marconiclub/
"""

import re

MEMBERS = {}

def init(cfg):
    # read initial exchange file
    global MEMBERS
    comment = re.compile("(^#|^$)") # starts with a hash or empty
    with open(cfg) as file:
        for line in file:
            line = line.strip()
            if comment.match(line):
                continue
            [call, exchange] = line.split(',')
            MEMBERS[call] = exchange


def get_member_id(exchange):
    if exchange.startswith('MC'):
        return exchange

    return ''


# - 5 points for a QSO with a member of Marconi Club A.R.I. Loano
# - 1 point for a QSO with a non-member
def score(qso):

    if get_member_id(qso.exchange):
        points = 5
    else:
        points = 1

    return points


def check_exchange(qso):
    mult = get_member_id(qso.exchange)
    if not mult and qso.call in MEMBERS:
        mult = MEMBERS[qso.call]

    return {'mult1_value': mult}


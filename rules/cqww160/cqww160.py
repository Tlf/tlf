"""
CQ WW 160
https://www.cq160.com/rules.htm
"""

import re

K_STATES = ['AL','AR','AZ','CA','CO','CT','DC','DE','FL','GA','IA','ID','IL',
            'IN','KS','KY','LA','MA','MD','ME','MI','MN','MO','MS','MT','NC',
            'ND','NE','NH','NJ','NM','NV','NY','OH','OK','OR','PA','RI','SC',
            'SD','TN','TX','UT','VA','VT','WA','WI','WV','WY']

VE_PROVINCES = ['AB','BC','LAB','MB','NB','NF','NS',
                'NU','NWT','ON','PEI','QC','SK','YUK']

MY_MAIN_PREFIX = ''
MY_CONTINENT = ''

K_VE_EXCHANGES = {}


def init(cfg):
    dxcc = tlf.get_dxcc(tlf.MY_CALL)
    global MY_MAIN_PREFIX
    MY_MAIN_PREFIX = dxcc.main_prefix
    global MY_CONTINENT
    MY_CONTINENT = dxcc.continent

    # read initial exchange file
    global K_VE_EXCHANGES
    comment = re.compile("(^#|^$)") # starts with a hash or empty
    with open(cfg) as file:
        for line in file:
            line = line.strip()
            if comment.match(line):
                continue
            [call, exchange] = line.split(',')
            K_VE_EXCHANGES[call] = exchange


def score(qso):
    if qso.band != 160:
        return 0
    # Maritime mobile contacts count 5 points.
    if qso.call.endswith('/MM'):
        return 5

    dxcc = tlf.get_dxcc(qso.call)

    # Contacts with stations in own country: 2 points.
    if dxcc.main_prefix == MY_MAIN_PREFIX:
        return 2
    # Contacts with other countries on same continent: 5 points.
    if dxcc.continent == MY_CONTINENT:
        return 5
    # Contacts with other continents: 10 points
    return 10


def check_exchange(qso):
    if qso.band != 160:
        mult = ''
    # There is no multiplier value for a maritime mobile contact.
    elif qso.call.endswith('/MM'):
        mult = ''
    else:
        dxcc = tlf.get_dxcc(qso.call)
        xchg = qso.exchange.strip().upper()

        # U.S. states
        if dxcc.main_prefix == 'K':
            if xchg in K_STATES:
                mult = f'K/{xchg}'
            elif not xchg and qso.call in K_VE_EXCHANGES:
                mult = f'K/{K_VE_EXCHANGES[qso.call]}'
            else:
                mult = ''
        # Canadian provinces
        elif dxcc.main_prefix== 'VE':
            if xchg in VE_PROVINCES:
                mult = f'VE/{xchg}'
            elif not xchg and qso.call in K_VE_EXCHANGES:
                mult = f'VE/{K_VE_EXCHANGES[qso.call]}'
            else:
                mult = ''
        # DXCC plus WAE countries
        else:
            mult = dxcc.main_prefix

    return {'mult1_value': mult}


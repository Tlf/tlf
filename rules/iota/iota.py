"""
RSGB IOTA Contest
https://www.rsgbcc.org/hf/rules/2024/riota.shtml
"""

import re

MY_ISLAND_REF = None

def init(cfg):
    global MY_ISLAND_REF
    MY_ISLAND_REF = cfg


XCHG_PATTERN = re.compile(r"""
    ([0-9]+)                # serial number
    \s*                     # optional space
    (                       # optional island reference
        (AF|AN|AS|EU|NA|OC|SA)  # continent
        \s*                     # optional space
        ([0-9]+)                # and digits
    )?
    $                       # end of string
    """, re.VERBOSE)

def parse_exchange(xchg):
    m = XCHG_PATTERN.match(xchg.strip())
    if not m:
        return (0, None)

    seq_number = int(m.group(1))
    if m.group(2):
        ref_index = int(m.group(4))
        ref = f'{m.group(3)}{ref_index:03}'     # normalize island reference
    else:
        ref = None

    return (seq_number, ref)


# Island Stations contacting
#   World Stations: 5 points.
#   Island Stations having the same IOTA reference : 5 points.
#   other Island Stations: 15 points.
def score_island(ref):
    if not ref:
        return 5
    elif ref == MY_ISLAND_REF:
        return 5
    else:
        return 15

# World Stations contacting
#   World Stations: 2 points.
#   Island Stations: 15 points.
def score_world(ref):
    if not ref:
        return 2
    else:
        return 15


def score(qso):
    _, ref = parse_exchange(qso.exchange)
    if MY_ISLAND_REF:
        return score_island(ref)
    else:
        return score_world(ref)


def check_exchange(qso):
    serial, ref = parse_exchange(qso.exchange)
    if not serial:
        return {}

    mult = ''

    xchg = f'{serial:03}'
    if ref:
        xchg += ' ' + ref
        mult = ref
        if qso.mode == tlf.CWMODE:
            mult += 'c'
        else:
            mult += 's'

    return {'mult1_value': mult, 'normalized_exchange': xchg}


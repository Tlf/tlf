"""
Scandinavian Activity Contest
https://www.sactest.net/
"""

SAC_PREFIXES = ['JW', 'JX', 'LA', 'OH', 'OH0', 'OJ0', 'OX', 'OY', 'OZ', 'SM', 'TF']

MY_CONTINENT = None

def init(cfg):
    dxcc = tlf.get_dxcc(tlf.MY_CALL)
    global MY_CONTINENT
    MY_CONTINENT = dxcc.continent


def score(qso):
    dxcc = tlf.get_dxcc(qso.call)

    if dxcc.main_prefix not in SAC_PREFIXES:
        return 0

    points = 1

    # NON-EUROPEAN stations receive three (3) points for every complete QSO
    # on 3.5 and 7 MHz.
    if MY_CONTINENT != 'EU' and qso.band >= 40:
        points = 3

    return points


def check_exchange(qso):
    dxcc = tlf.get_dxcc(qso.call)

    mult = ''

    if dxcc.main_prefix in SAC_PREFIXES:
        mult = dxcc.main_prefix
        if len(mult) == 2:
            area = qso.call[2]
            if not area.isdigit():
                area = '0'
            mult += area

    return {'mult1_value': mult}


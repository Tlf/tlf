"""
Scandinavian Activity Contest
https://www.sactest.net/
"""

SCANDINAVIAN_PREFIXES = {
    'JW', 'JX', 'LA', 'OH', 'OH0', 'OJ0', 'OX', 'OY', 'OZ', 'SM', 'TF'
}

MY_CONTINENT = None
SCANDINAVIAN_STATION = False

def init(cfg):
    dxcc = tlf.get_dxcc(tlf.MY_CALL)
    global MY_CONTINENT
    MY_CONTINENT = dxcc.continent
    global SCANDINAVIAN_STATION
    SCANDINAVIAN_STATION = dxcc.main_prefix in SCANDINAVIAN_PREFIXES

def score(qso):
    dxcc = tlf.get_dxcc(qso.call)

    # 7.1 For Scandinavian stations:
    # EUROPEAN stations, outside Scandinavia, are worth two (2) points for every complete QSO.
    # NON-EUROPEAN stations are worth three (3) points for every complete QSO.
    if SCANDINAVIAN_STATION:
        if dxcc.main_prefix in SCANDINAVIAN_PREFIXES:
            return 0

        if dxcc.continent == 'EU':
            points = 2
        else:
            points = 3

    # 7.2 For non-Scandinavian stations:
    # EUROPEAN stations receive one (1) point for every complete Scandinavian QSO.
    # NON-EUROPEAN stations receive one (1) point for every complete Scandinavian QSO on 14, 21, and 28 MHz and three (3) points for every complete QSO on 3.5 and 7 MHz.
    else:
        if dxcc.main_prefix not in SCANDINAVIAN_PREFIXES:
            return 0

        points = 1

        if MY_CONTINENT != 'EU' and qso.band >= 40:
            points = 3

    return points


def check_exchange(qso):
    dxcc = tlf.get_dxcc(qso.call)

    mult = ''

    # 8.1 For Scandinavian stations
    # Each worked DXCC entity is valid as one multiplier for each band.
    if SCANDINAVIAN_STATION:
        mult = dxcc.main_prefix

    # 8.2 For non-Scandinavian stations
    # Each worked main prefix+number (0-9) in each Scandinavian DXCC entity.
    else:
        if dxcc.main_prefix in SCANDINAVIAN_PREFIXES:
            mult = dxcc.main_prefix
            if len(mult) == 2:
                area = qso.call[2]
                if not area.isdigit():
                    area = '0'
                mult += area

    return {'mult1_value': mult}


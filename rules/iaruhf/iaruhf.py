"""
IARU HF World Championship
https://www.arrl.org/iaru-hf-world-championship
"""

MY_ITU_ZONE = None
MY_CONTINENT = None

IARU_EXCHANGES = ['IARU', 'AC', 'R1', 'R2', 'R3']

def init(cfg):
    dxcc = tlf.get_dxcc(tlf.MY_CALL)
    global MY_ITU_ZONE
    MY_ITU_ZONE = dxcc.itu_zone
    global MY_CONTINENT
    MY_CONTINENT = dxcc.continent

"""
5. Scoring
 5.1 QSO points:
  5.1.1 Contacts within your own ITU zone count one (1) point.
  5.1.2 Contacts with an IARU HQ or IARU official station count one (1) point.
  5.1.3 Contacts with a station in the same ITU zone
            but on a different continent count one (1) point.
  5.1.4 Contacts within your continent but in a different ITU zone
            count three (3) points.
  5.1.5 Contacts with a different continent and ITU zone count five (5) points.
"""
def score(qso):
    dxcc = tlf.get_dxcc(qso.call)
    xchg = qso.exchange.strip()

    if dxcc.itu_zone == MY_ITU_ZONE:        # 5.1.1 and 5.1.3
        points = 1
    elif xchg in IARU_EXCHANGES:            # 5.1.2
        points = 1
    else:
        if dxcc.continent == MY_CONTINENT:  # 5.1.4 and 5.1.5
            points = 3
        else:
            points = 5

    return points


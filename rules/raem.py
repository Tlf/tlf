import re

# e.g. 001 57n85o
XCHG_PATTERN = re.compile('\d+\s+(\d+)\s*([NS])\s*(\d+)\s*([OW])')

def setup():
    return None

#
# https://raem.srr.ru/rules/
#
def score(qso):
    xchg = qso.exchange.strip().upper()

    # 1. Every valid QSO gives 50 points.
    m = XCHG_PATTERN.match(xchg)
    if not m:
        return 0
    points = 50

    # 2. Every one degree in geographic coordinates’ difference in latitude
    # (your and received coordinates in exchange) gives one point in addition;
    # the same for longitude.
    lat = int(m.group(1))
    if m.group(2) == 'S':
        lat = -lat
    lon = int(m.group(3))
    if m.group(4) == 'O':
        lon = -lon
    points = points + abs(lat - int(tlf.MY_LAT))
    points = points + abs(lon - int(tlf.MY_LONG))

    # 3. Every QSO with a polar amateur radio station (defined as a station
    # located within the Earth’s Polar Circles and sending a latitude
    # of 66 degrees or greater) adds 100 to points from p.2.
    if abs(lat) >= 66:
        points = points + 100

    # 4. Every QSO with the RAEM memorial amateur radio station adds 300
    # to points from p.2.
    if qso.call == 'RAEM':
        points = points + 300

    return points


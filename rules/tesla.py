import re

# e.g. 023 KN03
XCHG_PATTERN = re.compile('\d+\s*(\w{2}\d{2})')

def setup():
    return None

#
# http://www.radiosport.yu1srs.org.rs/HFTeslaMemorial/index.php/rules
#
def score(qso):
    xchg = qso.exchange.strip().upper()

    m = XCHG_PATTERN.match(xchg)
    if not m:
        return 0

    # Points are calculated based on the distance
    # between the centers of the middle of QTH grids:

    qrb = tlf.get_qrb_for_locator(m.group(1))
    if not qrb:
        return 0

    distance = int(qrb.distance)

    if distance <= 600: return 10   # UP to        600 km = 10 points
    if distance <= 1200: return 13  # 601 - 1200 km = 13 points
    if distance <= 1800: return 16  # 1201 - 1800 km = 16 points
    if distance <= 2400: return 20  # 1801 - 2400 km = 20 points
    if distance <= 3600: return 24  # 2401 - 3600 km = 24 points
    if distance <= 4800: return 28  # 3600 - 4800 km = 28 points
    if distance <= 6000: return 32  # 4801 - 6000 km = 32 points
    if distance <= 7200: return 36  # 6001 - 7200 km = 36 points
    if distance <= 8400: return 40  # 7201 - 8400 km = 40 points
    return 45                       # above  8401 km = 45 points


"""
EU-DX contest
https://www.eudx-contest.com/
"""
import re

MY_COUNTRY = None
MY_PREFIX = None
MY_CONTINENT = None

EU_REGION_PATTERN = re.compile(r'([A-Z]{2})\d{2}')  # two letters and two numbers

EU_COUNTRIES = ['AT', 'BE', 'BG', 'CZ', 'CY', 'HR', 'DK', 'EE',
                'FI', 'FR', 'DE', 'GR', 'HU', 'IE', 'IT', 'LV',
                'LT', 'LX', 'MT', 'NL', 'PL', 'PT', 'RO', 'SK',
                'SI', 'ES', 'SE']

#
# - EU stations shall provide their ISO_3166-1_alpha-2 country code
#           as used in the contest (NOT the DXCC country code)
# - non-EU stations shall use simply DX
#
def init(cfg):
    global MY_COUNTRY
    if cfg in EU_COUNTRIES:
        MY_COUNTRY = cfg
    else:
        MY_COUNTRY = 'DX'   # normalize to DX

    dxcc = tlf.get_dxcc(tlf.MY_CALL)
    global MY_PREFIX
    MY_PREFIX = dxcc.main_prefix
    global MY_CONTINENT
    MY_CONTINENT = dxcc.continent


# a. European Union stations:
# - your own country  2 points,
# - another European Union country  10 points,
# - with a non-European Union country in your continent  3 points,
# - another continent  5 points.
# b. Non- European Union stations:
# - European Union  10 points,
# - your own country  2 points,
# - a different country in your continent  3 points,
# - another continent  5 points.
def score(qso):
    xchg = qso.exchange.strip()

    m = EU_REGION_PATTERN.match(xchg)
    if m:
        eu_country = m.group(1)
    else:
        eu_country = None

    if MY_COUNTRY != 'DX':
        if eu_country == MY_COUNTRY:
            return 2
        if eu_country:
            return 10
        dxcc = tlf.get_dxcc(qso.call)
        if dxcc.continent == MY_CONTINENT:
            return 3
        else:
            return 5
    else:
        if eu_country:
            return 10
        dxcc = tlf.get_dxcc(qso.call)
        if dxcc.main_prefix == MY_PREFIX:
            return 2
        if dxcc.continent == MY_CONTINENT:
            return 3
        else:
            return 5



def check_exchange(qso):
    xchg = qso.exchange.strip()
    dxcc = tlf.get_dxcc(qso.call)
    mult = dxcc.main_prefix

    if EU_REGION_PATTERN.match(xchg):
        mult += ' ' + xchg

    return {'mult1_value': mult}

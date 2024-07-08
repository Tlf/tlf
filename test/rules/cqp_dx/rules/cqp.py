"""
CQP contest
https://www.cqp.org/
"""
import re

MY_STATE = None

CA_COUNTIES = ['ALAM', 'ALPI', 'AMAD', 'BUTT', 'CALA', 'COLU', 'CCOS', 'DELN',
        'ELDO', 'FRES', 'GLEN', 'HUMB', 'IMPE', 'INYO', 'KERN', 'KING', 'LAKE',
        'LASS', 'LANG', 'MADE', 'MARN', 'MARP', 'MEND', 'MERC', 'MODO', 'MONO',
        'MONT', 'NAPA', 'NEVA', 'ORAN', 'PLAC', 'PLUM', 'RIVE', 'SACR', 'SBEN',
        'SBER', 'SDIE', 'SFRA', 'SJOA', 'SLUI', 'SMAT', 'SBAR', 'SCLA', 'SCRU',
        'SHAS', 'SIER', 'SISK', 'SOLA', 'SONO', 'STAN', 'SUTT', 'TEHA', 'TRIN',
        'TULA', 'TUOL', 'VENT', 'YOLO', 'YUBA']

STATES = ['AL', 'AK', 'AZ', 'AR',
# 'CA' -- The first valid QSO logged with 4-letter county abbreviation will count as the multiplier for California.
'CO', 'CT', 'DE', 'FL', 'GA', 'HI', 'ID', 'IL', 'IN', 'IA', 'KS', 'KY',
'LA', 'ME', 'MD', 'MA', 'MI', 'MN', 'MS', 'MO', 'MT', 'NE', 'NV', 'NH',
'NJ', 'NM', 'NY', 'NC', 'ND', 'OH', 'OK', 'OR', 'PA', 'RI', 'SC', 'SD',
'TN', 'TX', 'UT', 'VT', 'VA', 'WA', 'WV', 'WI', 'WY',
# Canada
'AB', 'BC', 'MB', 'NB', 'NL', 'NT', 'NS', 'NU', 'ON', 'PE', 'QC', 'SK', 'YT'
]

MULT_PATTERN = re.compile('[A-Z\s]+$')  # trailing block of letters and spaces

def init(cfg):
    global MY_STATE
    MY_STATE = cfg

def check_exchange(qso):
    m  = MULT_PATTERN.search(qso.exchange)
    if m:
        parts = m.group(0).split()
    else:
        parts = []

    mult = ''

    if MY_STATE == 'CA':
        if len(parts) == 0: # no value
            pass
        elif len(parts) == 1: # single value
            part = parts[0]
            if part in STATES:
                mult = part
            elif part in CA_COUNTIES:
                mult = 'CA'
        else:  # multiple values, all must be valid CA counties
            ok = True
            for part in parts:
                if part not in CA_COUNTIES:
                    ok = False
            if ok:
                mult = 'CA'
    else:   # Non-California Station
        for part in parts:
            if part in CA_COUNTIES:
                mult += part + ' '

    return {'mult1_value': mult}

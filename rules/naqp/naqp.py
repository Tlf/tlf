"""
NAQP contest
https://ncjweb.com/NAQP-Rules.pdf
"""

countries = ['CA', 'AL', 'AK', 'AZ', 'AR', 'CO', 'CT', 'DE', 'FL', 'GA', 'HI', 'ID', 'IL', 'IN', 'IA',
'KS', 'KY', 'LA', 'ME', 'MD', 'MA', 'MI', 'MN', 'MS', 'MO', 'MT', 'NE', 'NV', 'NH', 'NJ',
'NM', 'NY', 'NC', 'ND', 'OH', 'OK', 'OR', 'PA', 'RI', 'SC', 'SD', 'TN', 'TX', 'UT', 'VT',
'VA', 'WA', 'WV', 'WI', 'WY', 'DC',
# Canada
'NB', 'NL', 'NS', 'PE', 'QC', 'ON', 'MB', 'SK', 'AB', 'BC', 'NT', 'NU', 'YT',
# Other countries
'4U1/u', '6Y', '8P', 'C6', 'CM', 'CY9', 'CY0', 'FG', 'FJ', 'FM', 'FO', 'FP', 'FS',
'HH', 'HI', 'HK0', 'HP', 'HR', 'J3', 'J6', 'J7', 'J8', 'KG4', 'KP1', 'KP2', 'KP4',
'KP5', 'OX', 'PJ5', 'PJ7', 'TG', 'TI', 'TI9', 'V2', 'V3', 'V4', 'VP2E', 'VP2M', 'VP2V',
'VP5', 'VP9', 'XE', 'XF4', 'YN', 'YS', 'YV0', 'ZF']

def score(qso):
    return 1

def check_exchange(qso):
    parts = qso.exchange.strip().split()

    mult = ''
    if len(parts) == 2:
        if parts[1] in countries:
            mult = parts[1]
        else:
            mult = ''
    else:
        mult = ''
    return {'mult1_value': mult}

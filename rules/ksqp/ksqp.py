"""
KSQP contest
https://ksqsoparty.org/rules/KSQPRules2022.pdf?1
"""

states = ['CA', 'AL', 'AK', 'AZ', 'AR', 'CO', 'CT', 'DE', 'FL', 'GA', 'HI', 'ID', 'IL', 'IN', 'IA',
'KS', 'KY', 'LA', 'ME', 'MD', 'MA', 'MI', 'MN', 'MS', 'MO', 'MT', 'NE', 'NV', 'NH', 'NJ',
'NM', 'NY', 'NC', 'ND', 'OH', 'OK', 'OR', 'PA', 'RI', 'SC', 'SD', 'TN', 'TX', 'UT', 'VT',
'VA', 'WA', 'WV', 'WI', 'WY', 'DC',
# Canada
'NB', 'NL', 'NS', 'PE', 'QC', 'ON', 'MB', 'SK', 'AB', 'BC', 'NT', 'NU', 'YT',
# Other countries
'DX']

counties = [
'ALL', 'AND', 'ATC', 'BAR', 'BRT', 'BOU', 'BRO', 'BUT', 'CHS', 'CHT', 'CHE', 'CHY',
'CLK', 'CLY', 'CLO', 'COF', 'COM', 'COW', 'CRA', 'DEC', 'DIC', 'DON', 'DOU', 'EDW',
'ELK', 'ELL', 'ELS', 'FIN', 'FOR', 'FRA', 'GEA', 'GOV', 'GRM', 'GRT', 'GRY', 'GLY',
'GRE', 'HAM', 'HPR', 'HVY', 'HAS', 'HOG', 'JAC', 'JEF', 'JEW', 'JOH', 'KEA', 'KIN',
'KIO', 'LAB', 'LAN', 'LEA', 'LCN', 'LIN', 'LOG', 'LYO', 'MRN', 'MSH', 'MCP', 'MEA',
'MIA', 'MIT', 'MGY', 'MOR', 'MTN', 'NEM', 'NEO', 'NES', 'NOR', 'OSA', 'OSB', 'OTT',
'PAW', 'PHI', 'POT', 'PRA', 'RAW', 'REN', 'REP', 'RIC', 'RIL', 'ROO', 'RUS', 'RSL',
'SAL', 'SCO', 'SED', 'SEW', 'SHA', 'SHE', 'SMN', 'SMI', 'STA', 'STN', 'STE', 'SUM',
'THO', 'TRE', 'WAB', 'WAL', 'WAS', 'WIC', 'WIL', 'WOO', 'WYA']

rule_mode = ""

def init(cfg):
    global rule_mode
    rule_mode = cfg


def score(qso):
    ml = 1
    if rule_mode != 'kansas' and not qso.exchange in counties:
        ml = 0

    if qso.mode == tlf.CWMODE or qso.mode == tlf.DIGIMODE:
        return 3 * ml
    return 2 * ml

def kansas_handler(qso):
    v = qso.exchange.strip()

    if v in states:
        return v
    elif v in counties:
        return 'KS'
    else:
        return ''

def non_kansas_handler(qso):
    v = qso.exchange.strip()

    if v in counties:
        return v
    else:
        return ''

def check_exchange(qso):
    if rule_mode == 'kansas':
        res = kansas_handler(qso)
        return {'mult1_value': res}
    else:
        res = non_kansas_handler(qso)
        return {'mult1_value': res}

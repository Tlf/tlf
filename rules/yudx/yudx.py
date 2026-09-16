"""
YUDX contest
https://yudx.yu1srs.org.rs/rules
"""

YU_COUNTIES = ['BGD', 'BOR', 'BRA', 'JAB', 'JBB', 'JBN', 'KMO', 'KOL',
    'KOS', 'KPO', 'MAC', 'MOR', 'NIS', 'PCI', 'PEC', 'PIR', 'POD',
    'POM', 'PRI', 'RAN', 'RAS', 'SBB', 'SBN', 'SBT', 'SRM', 'SUM',
    'TOP', 'ZAJ', 'ZBB', 'ZLA']

def check_exchange(qso):
    dxcc = tlf.get_dxcc(qso.call)

    mult = dxcc.main_prefix

    if dxcc.main_prefix == 'YU':
        exchange = qso.exchange.strip()
        if exchange in YU_COUNTIES:
            mult += ' ' + exchange

    return {'mult1_value': mult}


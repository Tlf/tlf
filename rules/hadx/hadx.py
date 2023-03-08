"""
HA-DX contest
https://ha-dx.com/en/contest-rules
"""

HA_COUNTIES = ['BN', 'BA', 'BE', 'BO', 'CS', 'FE', 'GY', 'HB', 'HE',
    'SZ', 'KO', 'NG', 'PE', 'SO', 'SA', 'TO', 'VA', 'VE', 'ZA', 'BP']

def check_exchange(qso):
    dxcc = tlf.get_dxcc(qso.call)

    if dxcc.main_prefix == 'HA':
        exchange = qso.exchange.strip()
        if exchange in HA_COUNTIES:
            mult = f'HA/{exchange}'
        else:
            mult = ''   # invalid county
    else:
        mult = dxcc.main_prefix

    return {'mult1_value': mult}


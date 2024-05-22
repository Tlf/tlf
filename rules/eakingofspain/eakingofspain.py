"""
https://concursos.ure.es/en/s-m-el-rey-de-espana-cw/bases/

Multipliers: The Multipliers, on each band, are the same for both EA and DX stations and are as follows:

– The EADX-100 entities.

– The Spanish provinces (52).

– Special station His Majesty The King of Spain (EF0F) will pass the abbreviation (SMR)

"""

EA_DXCCS = ['EA', 'EA6', 'EA8', 'EA9']
EA_PROVINCES = ['AV', 'BU', 'C', 'LE', 'LO', 'LU', 'O', 'OU', 'P', 'PO', 'S',
    'SA', 'SG', 'SO', 'VA', 'ZA', 'BI', 'HU', 'NA', 'SS', 'TE', 'VI', 'Z',
    'B', 'GI', 'L', 'T', 'BA', 'CC', 'CR', 'CU', 'GU', 'M', 'TO', 'A', 'AB',
    'CS', 'MU', 'V', 'IB', 'AL', 'CA', 'CO', 'GR', 'H', 'J', 'MA', 'SE', 'GC',
    'TF', 'CE', 'ML', 'SMR']

def check_exchange(qso):
    dxcc = tlf.get_dxcc(qso.call)

    if dxcc.main_prefix in EA_DXCCS:
        exchange = qso.exchange.strip()
        if exchange in EA_PROVINCES:
            mult = f'{dxcc.main_prefix}/{exchange}'
        else:
            mult = ''   # invalid county
    else:
        mult = dxcc.main_prefix

    return {'mult1_value': mult}


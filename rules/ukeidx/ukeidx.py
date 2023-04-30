"""
UK/EI DX Contest
https://ukeicc.com/dx-contest-rules.php
"""

import time

from enum import Flag, auto
class Location(Flag):
    UK_EI = auto()
    EU = auto()
    DX = auto()

MY_LOCATION = None

UKEI_DISTRICTS = {
    'EI': ['CE','CK','CL','CN','DO','DU','GA','KD','KE','KI','LF',
        'LH','LI','LO','LT','MA','MO','MT','OF','RO','SI','TI','WI',
        'WM','WT','WX'],
    'G': ['AL','BA','BB','BD','BH','BL','BM','BN','BR','BS','CA',
        'CB','CH','CM','CO','CR','CT','CV','CW','DA','DE','DH','DL',
        'DN','DT','DY','EC','EL','EN','EX','FY','GL','GU','HA','HD',
        'HG','HP','HR','HU','HX','IG','IP','KT','LA','LE','LN','LP',
        'LS','LU','ME','MK','MR','NE','NG','NL','NN','NK','NW','OL',
        'OX','PE','PL','PO','PR','RG','RH','RM','SD','SE','SG','SK',
        'SL','SM','SN','SO','SP','SR','SS','ST','SW','SY','TA','TF',
        'TN','TQ','TR','TS','TW','UB','WA','WC','WD','WF','WL','WN',
        'WR','WS','WV','YO',
        'TD'],  # TD is G/GM
    'GD': ['IM'],
    'GI': ['AN','AR','DR','DW','FE','TY'],
    'GJ': ['JE'],
    'GM': ['AB','DD','DG','EH','FK','GS','HS','IV','KA','KW','KY',
        'ML','PA','PH','ZE',
        'TD'],  # TD is G/GM
    'GU': ['GY'],
    'GW': ['CF','LD','LL','NP','SA']
}

def get_location(dxcc):
    if dxcc.main_prefix in UKEI_DISTRICTS:
        return Location.UK_EI
    elif dxcc.continent == 'EU':
        return Location.EU
    else:
        return Location.DX


def init(cfg):
    dxcc = tlf.get_dxcc(tlf.MY_CALL)
    global MY_LOCATION
    MY_LOCATION = get_location(dxcc)


# - basic scoring
# from   to UK/EI    EU    DX
# UK/EI       2       2     4
# EU          2       1     2
# DX          4       2     1
#
# - 80 and 40 meters count double
# - for UK/EI nightly bonus applies
def score(qso):
    dxcc = tlf.get_dxcc(qso.call)
    their_location = get_location(dxcc)

    if MY_LOCATION == Location.UK_EI:
        points = 2
        if their_location == Location.DX:
            points = 4
    elif MY_LOCATION == Location.EU:
        points = 2
        if their_location == Location.EU:
            points = 1
    else:
        points = 1
        if their_location == Location.UK_EI:
            points = 4
        elif their_location == Location.EU:
            points = 2
   
    if qso.band >= 40:
        points *= 2 

    if MY_LOCATION == Location.UK_EI:
        hour = time.gmtime(qso.utc).tm_hour
        if hour >= 1 and hour <= 4:
            points *= 2

    return points


def check_exchange(qso):
    dxcc = tlf.get_dxcc(qso.call)

    if dxcc.main_prefix in UKEI_DISTRICTS:

        district = qso.exchange.strip()[-2:]    # last 2 characters

        if district in UKEI_DISTRICTS[dxcc.main_prefix]:
            mult = f'{dxcc.main_prefix}/{district}'
            if mult == 'GM/TD': # normalize TD as G/TD
                mult = 'G/TD'
        else:
            mult = ''   # invalid district
    else:
        mult = dxcc.main_prefix

    return {'mult1_value': mult}


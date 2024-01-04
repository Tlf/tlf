"""
Marconi Club QSO Party Day
http://www.ariloano.it/marconiclub/
"""

def get_member_id(exchange):
    if exchange.startswith('MC'):
        return exchange

    return ''


# - 5 points for a QSO with a member of Marconi Club A.R.I. Loano
# - 1 point for a QSO with a non-member
def score(qso):

    if get_member_id(qso.exchange):
        points = 5
    else:
        points = 1

    return points


def check_exchange(qso):

    return {'mult1_value': get_member_id(qso.exchange)}


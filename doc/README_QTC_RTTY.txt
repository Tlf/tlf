
This is a short intro for Tlf QTC handling in RTTY mode.
Ervin Hegedus HA2OS, 2014.
airween@gmail.com


Introduction
============

The QTC base instructions described in README_QTC.txt, please
read it CAREFULLY, before you start to read this readme. There
you will find detailed information about how Tlf handles the QTC
in CW and SSB mode. In this readme, I just write the differences.

General
=======

The QTC window(s), fields, navigation is almost the same, like
in CW and SSB modes. In CW/SSB modes every QTC line int the
full QTC block exchanged by individually (every QTC line sent
or received separately), but that method would be too slow in
RTTY mode. In this mode, a complete QTC block exchanged in whole.
That's the main different, but this divergence involves a few
necessary modifications. Another important thing, that in
RTTY mode everyone can send or receive QTCs, there isn't any
limit (except the number of total QTCs between two stations).
So, it happens that you sent 5 QTCs in a block, but to complete
to 10, the second 5 QTCs you will receive from same station.
That's why there is the third possible value of "QTC" option
in logcfg.dat/rule file, which is "BOTH".


Receiving QTC
=============
Even if you choose the two-way QTC handling, or you just want to
receive QTCs, the CTRL-Q combinaton remaining to receive the
QTC block. If you open the QTC receive window with CTRL-Q,
apparently nothing has changed. The point is same than in another
modes: fill the lines. Yes, the speed of RTTY is a little bit
fast, and you have to know type as very quickly, if you want to
copy from the terminal of your modem software. To help the copy
operation, there is a new feature in Tlf: capturing the output
of modem. Currently, only the gMFSK modem could be captured.

If you want to start the capture, just press CTRL+S, that means
"Start capture". To finish the capture process, just press
CTRL+E, that means "End capture".

If you've started the capture process, the new window will open,
which is a "terminal" window - there you can see the gMFSK modem
lines. At the top of the double-window, you can see a small HELP
window, which contains all shortkeys these windows. If you press
ESC (Escape), then terminal window will be hidden, but - this is
very important! - the capture process doesn't finished! Now, if
you press CTRL+S again, the terminal window will opens again, and
you can see the characters in the meantime.

You can recognize two types of lines for QTC block: the serial/number
line, or a single QTC line. The formula of serial/number line is
NNN/NN. This works only if you didn't fill the serial and number
fields in QTC window. So, if those lines are empty, and in your
terminal window the line is like this:

"QTC 32/8 QTC 32/8"

and you scroll there, and pressing ENTER, Tlf recognizes this
as the serial and number of the QTC block, and will fill those
fields.

The QTC line formula could be:

1111/W1AW/234
1111-PX2A-345

The "separator" character could be the "/" (slash) or "-" hyphen.

If you scroll to a line like one of these, and pressing ENTER,
Tlf will fill the next empty QTC line in QTC block. Note, that the
line starts with 4 number, and at the end is also a numeric character,
and the length of leftover is less than 14, then Tlf also recognizes
the QTC line.

IMPORTANT!

Only the complete received lines can be captured from right side to
left side, and only once. You can't move that line, which increments
horizontally - that's the current line. If the line has an NL
(new-line) character, the you can capture that. If you've pressed
the ENTER on a line, then it will be marked with an "*" (asterisk)
sign. That mean, you couldn't capture that anymore. 

STILL IMPORTANT!
If you fill the serial and number fields in main QTC window, the first
pattern WILL NOT recogized (SERIAL/NR).
If you didn't fill the serial AND number fields (with or without the
capture process), you CAN'T capture the QTC lines! That mean, you press
ENTER on a QTC line at right side vainly, the line will not copied to
right side!

So, if you can copy one or more lines from right side to left with
capture process, the result could be two types: Tlf thinks the line
is a good, fail-safe QTC line, or isn't it: the line contains wrong
character, eg. the time filed contains non-numeric character. In this
case, the end of the line you can see a "?" (question mark) character.
If the line doesn't contains wrong character, you can see a "+" (plus)
character.

IMPORTANT!
You NEED to review ALL lines at the left side by press an ENTER.
If the line has "+" character, then that will changed to "*" (asterisk),
which means "line marked as confirmed". If line has "?" at the end, then
Tlf will send an "AGN N AGN N" message, where the N is the number of
the line. Then you can change the wrong character, till the status
will goes to "+".

If all lines has "*" sign, and you press the ENTER at last line, Tlf
will send the message "QSL ALL", or what you've configures for QR_F10
macro.

You can send the last message by manually, if you press F10 - then Tlf
also send that message, and it will save the received QTC block too.

If the all QTC lines are right, and Tlf sent "QTC ALL", then the QTC
block will be saved automatically.


Sending QTC
=============

As you can read the QTC=BOTH descriotion above, Tlf can send AND
receive QTCs in same contest. To avoid the shortkey collosions, in
this case the only one combination can be used: CTRL+S.

If you fill the callsign field (or that's filled when you open the
QTC send window), then the QTC serial field is fixed - you can't
change that, like CW/SSB modes. Only you can change the number of
QTCs field, which could be maximum 10, or if you have QTC (received
or sent) with the station, then Tlf calculates the possible number.

Then if you press the ENTER, Tlf sends the complete QTC block to
your modem (eg. gmfsk_autofile), and you can see the characters in
the window of your modem (eg. Fldigi).

If station asks you to resend one or more QTC line again, just move
the asked line, and press ENTER again.

If station confirms the QTC block, you have to save that with
CTRL+S.



This is a short intro for Tlf QTC handling.
Ervin Hegedus HA2OS, 2014.


Introduction
============

QTC handling was maded for WAEDC contest for Tlf. It doesn't need
any external tool, Tlf supports it out of box. To enable that,
only needs to put the "QTC" option to logcfg.dat. This parameter
needs a value, which could be one of these: "RECV", "SEND",
"BOTH". These values means you only want to receive or send
QTC's, or you want to send AND receive QTC's (for example, in
RTTY mode). Note, that currently only the RECV is implemented.


QTC window
==========

When you enabled the QTC in logcfg.dat, you can open it from from
either field: callsign or exchange, it doesn't matter. To open
that, just press CTRL+Q. If you want to close that, just simple
press ESC. The window is divided to 2 site: the left site
contains the QTC's data, the right site contains the help.

The data site contains the QTC's fields:
* QTC callsign - 14 characters, allowed: alphanums, digits, /
* QTC serial - 4 characters, allowed: digits,
* number of QSO in current QTC block - 2 characters, allowed:
  digits
* 10 QSO lines:
  * time - 4 characters, allowed: digits, ?
  * callsign - 14 characters, allowed: alphanums, digits, /, ?
  * serial - 4 characters, allowed: digits, ?

This site contains many "meta" information:
* at right of the QTC callsing field, Tlf shows the QTC info of
  current stations: how many QTC has it on the current band; when
  the station not send or received yet any QTC on the current
  band, there is this value: "0Q on 10". If you are on 40m, and
  station has 1 or more QTC, there is some like this: "5Q on 40"
* if you type the number of QSO in the QTC block, Tlf will shows
  its number at the begin of every line; if the station indicates
  that it will send to you 6 QSO, and you type it that field, Tlf
  will put a digit to the begin of first 6 lines
* every QSO line has a status, which could be: "invalid",
  "valid", "confirmed". This status is indicated by a sign at the
  end of the line. When a line is invalid, you can see a "?", if
  the line is valid, then there isn't any character, and finally,
  if the QSO line had been confirmed, you can see a "*" sign


Navigation between the fields
=============================

You can move to any fields as you want and when you want. There
isn't mandatory order, but the navigation keys doesn't works in
all cases. Normally, with TAB you can move to the next field,
which followed by current, and Shift+TAB goes back. There is an
exception: if you in a QSO line, till you don't confirm the line,
TAB doesn't takes away, it takes to the time field, if you are in
the exchange field, and Shift+TAB does it in reverse mode.
With UP and DOWN cursor keys you can go to up or down. 


Navigation in a field
=====================

If you press an allowed character in a field, that will be
showed, and the cursor will goes to right next place. If you
press a not allowed character, then nothing happens. The
BACKSPACE key deletes the next charaters to left, and move the
right part to left. DELETE key deletes the current character as
you're staying, and shifts the characters to left at the next to
right.



Shortkey summary
================

* CTRL+Q - open QTC window
* ESC - close QTC window
* TAB - move to the next field
* Shift+TAB - move to the previous field
* UP, DOWN - move to up or down
* BACKSPACE - delete the next to left
* DELETE - delete the current position, shift to left the right
  next






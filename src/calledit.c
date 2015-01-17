/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003 Rein Couperus <pa0rct@amsat.org>
 *               2011           Thomas Beierlein <tb@forth-ev.deY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

    /* ------------------------------------------------------------
     *        Edit call input field
     *
     *--------------------------------------------------------------*/

#include "calledit.h"

void calledit(void)
{

    extern char hiscall[];
    extern int block_part;

    int i = 0, l, b = 0;
    int j = 0;
    int x = 0;
    int cnt = 0, insertflg = 0;
    char dupecall[20];
    char call1[30], call2[10];

    l = strlen(hiscall);
    b = l - 1;


    while ((i != 27) && (b <= strlen(hiscall))) {

	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvprintw(12, 29, "            ");
	mvprintw(12, 29, hiscall);
	mvprintw(12, 29 + b, "");
	/* no refreshp() here as getch() calls wrefresh() for the
	 * panel with last output (whre the cursor should go */

	i = onechar();

	if ((i == 161) || (i == 160))	// Ins / Del
	    cnt++;
	else {
	    if (i != 27)
		cnt = 0;
	}

	if (i == 9)
	    block_part = 1;
	else
	    block_part = 0;

	if (i == 1)		// ctrl-A, home
	{
	    b = 0;
	    x = 0;
	}
	if (i == 5)		// ctrl-E, End
	{
	    b = strlen(hiscall) - 1;
	    x = 0;
	}

	if (i == 155) {		// left

	    if (b > 0)
		b--;

	} else if (i == 154) {	// right
	    if (b < strlen(hiscall) - 1) {
		b++;
	    } else
		break;		/* stop edit */

	} else if (i == 161) {	/* delete */

	    l = strlen(hiscall);

	    for (j = b; j <= l; j++) {
		hiscall[j] = hiscall[j + 1];	/* move to left incl. \0 */
	    }

	    strncpy(dupecall, hiscall, 16);	/* update cty info */
	    x = getctydata(dupecall);
	    showinfo(x);

	    if (cnt > 1)
		searchlog(hiscall);

	} else if (i == 127) {	/* backspace */

	    if (b > 0) {

		b--;

		l = strlen(hiscall);

		for (j = b; j <= l; j++) {
		    hiscall[j] = hiscall[j + 1];
		}

		strncpy(dupecall, hiscall, 16);	/* update cty info */
		x = getctydata(dupecall);
		showinfo(x);

		if (cnt > 1)
		    searchlog(hiscall);
	    }

	} else if (i == 160) {	/* insert */
	    if (insertflg == 0)
		insertflg = 1;
	    else
		insertflg = 0;

	} else if (i != 27) {

	    if ((i >= 97) && (i <= 122))
		i = i - 32;

	    if (((i >= 65) && (i <= 90)) || ((i >= 47) && (i <= 57))) {

		call2[0] = '\0';

		if (b <= 12) {
		    strncpy(call1, hiscall, b);
		    strncpy(call2, hiscall + b, strlen(hiscall) - (b - 1));
		}

		if (strlen(hiscall) + 1 == 12)
		    break;	// leave insert mode

		if (((i >= 65) && (i <= 90)) || ((i >= 47) && (i <= 57))) {
		    call1[b] = i;
		    call1[b + 1] = '\0';
		    if ((strlen(call1) + strlen(call2)) < 12) {
			strcat(call1, call2);
//                      if (strlen(call1) + strlen(hiscall) >= 12) break;
			if (strlen(call1) >= 12)
			    break;
			strcpy(hiscall, call1);
		    }
		}

		if ((b < strlen(hiscall) - 1) && (b <= 12))
		    b++;
		else
		    break;

		strncpy(dupecall, hiscall, 16);	/* update cty info */
		x = getctydata(dupecall);
		showinfo(x);

		searchlog(hiscall);

	    } else if (x != 0)
		i = 27;

	} else
	    i = 27;

    }

    attroff(A_STANDOUT);
    attron(COLOR_PAIR(C_HEADER));

    mvprintw(12, 29, hiscall);
    mvprintw(12, 29, "            ");
    refreshp();

    attron(A_STANDOUT);
    searchlog(hiscall);
}

int insert_char(int curposition)
{

    extern char hiscall[];

    char call1[30], call2[10];
    int ichr = 0;

    attroff(A_STANDOUT);
    attron(COLOR_PAIR(C_HEADER));

    call1[0] = '\0';
    call2[0] = '\0';

    while (ichr != 27) {

	ichr = onechar();

	if ((ichr == 9) || (ichr == '\n') || (ichr == 127))
	    break;		// leave insert mode

	if ((ichr >= 97) && (ichr <= 122))
	    ichr = ichr - 32;

	if (curposition <= 10) {
	    strncpy(call1, hiscall, curposition);
	}
	if (curposition <= 10) {
	    strncpy(call2, hiscall + curposition,
		    strlen(hiscall) - (curposition - 1));
	}

	if (strlen(hiscall) + 1 == 13)
	    break;		// leave insert mode

	if (((ichr >= 65) && (ichr <= 90))
	    || ((ichr >= 47) && (ichr <= 57))) {
	    call1[curposition] = ichr;
	    call1[curposition + 1] = '\0';
	    if ((strlen(call1) + strlen(call2)) < 12) {
		strcat(call1, call2);
		if (strlen(call1) + strlen(hiscall) >= 12)
		    break;
		strcpy(hiscall, call1);
	    }
	} else
	    break;

	attroff(A_STANDOUT);
	attron(COLOR_PAIR(C_HEADER));

	mvprintw(12, 29, hiscall);
	curposition++;
	mvprintw(12, 29 + curposition, "");
	refreshp();

    }
    ichr = 27;

    return (ichr);
}

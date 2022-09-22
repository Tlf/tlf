/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2020 Zoltan Csahok <ha5cqz@freemail.hu>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdbool.h>
#include <string.h>

#include "callinput.h"
#include "gettxinfo.h"
#include "globalvars.h"
#include "time_update.h"
#include "tlf.h"


typedef struct {
    freq_t freq;
    cqmode_t cqmode;
    char hiscall[CALL_SIZE];
} mem_t;

static mem_t trxmem = {.freq = 0, .cqmode = NONE};


void memory_store() {
    trxmem.freq = freq;
    trxmem.cqmode = cqmode;
    strcpy(trxmem.hiscall, current_qso.call);

    force_show_freq = true;
}

void memory_recall() {
    if (trxmem.freq <= 0) {
	return;
    }

    set_outfreq(trxmem.freq);
    send_bandswitch(trxmem.freq);
    cqmode = trxmem.cqmode;
    strcpy(current_qso.call, trxmem.hiscall);

    force_show_freq = true;
}

void memory_pop() {
    if (trxmem.freq <= 0) {
	return;
    }

    memory_recall();

    trxmem.freq = 0;
    trxmem.cqmode = NONE;
}

void memory_store_or_pop() {
    if (trxmem.freq <= 0) {
	memory_store();
    } else {
	memory_pop();
    }
}

void memory_swap() {
    if (trxmem.freq <= 0) {
	return;
    }

    freq_t tmp_freq = freq;
    cqmode_t tmp_cqmode = cqmode;
    char tmp_hiscall[sizeof(trxmem.hiscall)];
    strcpy(tmp_hiscall, current_qso.call);

    memory_recall();

    trxmem.freq = tmp_freq;
    trxmem.cqmode = tmp_cqmode;
    strcpy(trxmem.hiscall, tmp_hiscall);
}


freq_t memory_get_freq() {
    return trxmem.freq;
}

cqmode_t memory_get_cqmode() {
    return trxmem.cqmode;
}


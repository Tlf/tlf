/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2001-2002-2003-2004 Rein Couperus <pa0r@amsat.org>
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

/* ------------------------------------------------------------
*      audio.h include file for soundcard input routine
*
*--------------------------------------------------------------*/

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

extern char* vk_record_cmd;
extern char* vk_play_cmd;
extern char* soundlog_record_cmd;
extern char* soundlog_play_cmd;

void sound_setup_default(void);
void record(void);

void vk_play_file(char *audiofile);
void vk_stop();
bool is_vk_finished();


#endif /* AUDIO_H */

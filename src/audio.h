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

#define FS	11025
#define S_BAR 0
#define SPOT_BAR 1
#define PAN_BAR 2


void init_audio();
int close_audio();
int testaudio();
void record(void);
void play_file(char *audiofile);
void stop_vk();
int wait_vk_finish();


#endif /* AUDIO_H */

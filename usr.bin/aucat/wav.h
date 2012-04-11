/*	$OpenBSD: wav.h,v 1.13 2012/04/11 06:05:43 ratchov Exp $	*/
/*
 * Copyright (c) 2008 Alexandre Ratchov <alex@caoua.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef WAV_H
#define WAV_H

#include <sys/types.h>

#include "aparams.h"
#include "pipe.h"

struct wav {
	struct pipe pipe;
	struct wav *next;
#define HDR_AUTO	0	/* guess by looking at the file name */
#define HDR_RAW		1	/* no headers, ie openbsd native ;-) */
#define HDR_WAV		2	/* microsoft riff wave */
	unsigned int hdr;	/* HDR_RAW or HDR_WAV */
	unsigned int xrun;	/* xrun policy */
	struct aparams hpar;	/* parameters to write on the header */
	off_t rbytes;		/* bytes to read, -1 if no limit */
	off_t wbytes;		/* bytes to write, -1 if no limit */
	off_t startpos;		/* beginning of the data chunk */
	off_t endpos;		/* end of the data chunk */
	off_t mmcpos;		/* play/rec start point set by MMC */
	short *map;		/* mulaw/alaw -> s16 conversion table */
	int slot;		/* mixer ctl slot number */
	int mmc;		/* use MMC control */
	int join;		/* join/expand channels */
	unsigned int vol;	/* current volume */
	unsigned int maxweight;	/* dynamic range when vol == 127 */
#define WAV_CFG		0	/* parameters read from headers */
#define WAV_INIT	1	/* not trying to do anything */
#define WAV_START	2	/* buffer allocated */
#define WAV_READY	3	/* buffer filled enough */
#define WAV_RUN		4	/* buffer attached to device */
#define WAV_MIDI	5	/* midi "syx" file */
	unsigned int pstate;	/* one of above */
	unsigned int mode;	/* bitmap of MODE_* */
	struct dev *dev;	/* device playing or recording */
};

extern struct fileops wav_ops;
struct wav *wav_list;

struct wav *wav_new_in(struct fileops *, struct dev *,
    unsigned int, char *, unsigned int, struct aparams *,
    unsigned int, unsigned int, int, int);
struct wav *wav_new_out(struct fileops *, struct dev *,
    unsigned int, char *, unsigned int, struct aparams *,
    unsigned int, int, int);
unsigned int wav_read(struct file *, unsigned char *, unsigned int);
unsigned int wav_write(struct file *, unsigned char *, unsigned int);
void wav_close(struct file *);
int wav_readhdr(int, struct aparams *, off_t *, off_t *, short **);
int wav_writehdr(int, struct aparams *, off_t *, off_t);
void wav_conv(unsigned char *, unsigned int, short *);
int wav_init(struct wav *);

extern short wav_ulawmap[256];
extern short wav_alawmap[256];

#endif /* !defined(WAV_H) */

/*
 * (C)opyright 1993,1994,1995 by Darren Reed.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and due credit is given
 * to the original author and the contributors.
 */

/*
 * written to comply with the RFC (1761) from Sun.
 * $Id: snoop.h,v 1.3 1997/02/11 22:24:04 kstailey Exp $
 */
struct	snoophdr	{
	char	s_id[8];
	int	s_v;
	int	s_type;
};

#define	SNOOP_VERSION	2

#define	SDL_8023	0
#define	SDL_8024	1
#define	SDL_8025	2
#define	SDL_8026	3
#define	SDL_ETHER	4
#define	SDL_HDLC	5
#define	SDL_CHSYNC	6
#define	SDL_IBMCC	7
#define	SDL_FDDI	8
#define	SDL_OTHER	9

#define	SDL_MAX		9


struct	snooppkt	{
	int	sp_olen;
	int	sp_ilen;
	int	sp_plen;
	int	sp_drop;
	int	sp_sec;
	int	sp_usec;
};

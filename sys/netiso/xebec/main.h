/*	$OpenBSD: main.h,v 1.2 1996/03/04 10:37:10 mickey Exp $	*/
/*	$NetBSD: main.h,v 1.4 1994/06/29 06:41:08 cgd Exp $	*/

#define TRUE 1
#define FALSE 0
#define LINELEN 2350
	/* approx limit on token size for C compiler 
	 * which matters for the purpose of debugging (astring.c...)
	 */

#define MSIZE 4000
#define	 DEBUGFILE "astring.c"
#define  ACTFILE "driver.c"
#define  EVENTFILE_H "events.h"
#define  STATEFILE "states.h"
#define  STATEVALFILE "states.init"

#define EV_PREFIX "EV_"
#define ST_PREFIX "ST_"

#define PCBNAME "_PCB_"

extern char kerneldirname[];
extern char protocol[];
extern char *synonyms[];
#define EVENT_SYN 0
#define PCB_SYN 1

extern int transno;
extern int print_trans;
extern char *stash();


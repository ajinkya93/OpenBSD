/* $OpenBSD: xf_delspi.c,v 1.1 1997/03/01 15:08:27 provos Exp $ */
/*
 * The author of this code is John Ioannidis, ji@tla.org,
 * 	(except when noted otherwise).
 *
 * This code was written for BSD/OS in Athens, Greece, in November 1995.
 *
 * Ported to OpenBSD and NetBSD, with additional transforms, in December 1996,
 * by Angelos D. Keromytis, kermit@forthnet.gr.
 *
 * Copyright (C) 1995, 1996, 1997 by John Ioannidis and Angelos D. Keromytis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, NEITHER AUTHOR MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netns/ns.h>
#include <netiso/iso.h>
#include <netccitt/x25.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include "net/encap.h"
#include "netinet/ip_ipsp.h"

extern char buf[];

int xf_set __P((struct encap_msghdr *));

int
xf_delspi(argc, argv)
int argc;
char **argv;
{
	int chain;

	struct encap_msghdr *em;

	if (argc != 5) {
	  fprintf(stderr, "usage: %s dst spi chaindelete alg\n", argv[0]);
	  return 0;
	}

	chain = atoi(argv[3]);
	em = (struct encap_msghdr *)&buf[0];
	
	if (chain)
	  em->em_msglen = EMT_DELSPI_FLEN;
	else
	  em->em_msglen = EMT_DELSPICHAIN_FLEN;
	em->em_version = 0;
	if (chain)
	  em->em_type = EMT_DELSPICHAIN;
	else
	  em->em_type = EMT_DELSPI;
	em->em_spi = htonl(strtol(argv[2], NULL, 16));
	em->em_if = 1;
	em->em_dst.s_addr = inet_addr(argv[1]);
	em->em_alg = atoi(argv[4]);  /* sanity checking in the kernel */

	return xf_set(em);
}



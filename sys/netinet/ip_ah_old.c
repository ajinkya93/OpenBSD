
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

/*
 * Authentication Header Processing
 * Per RFCs 1828/1852 (Metzger & Simpson)
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <machine/cpu.h>

#include <net/if.h>
#include <net/route.h>
#include <net/netisr.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>

#include <sys/socketvar.h>
#include <net/raw_cb.h>
#include <net/encap.h>

#include <netinet/ip_ipsp.h>
#include <netinet/ip_ah.h>
#include <sys/syslog.h>

/*
 * ah_old_attach() is called from the transformation initialization code.
 */

int
ah_old_attach()
{
#ifdef ENCDEBUG
    if (encdebug)
      printf("ah_old_attach(): setting up\n");
#endif /* ENCDEBUG */
    return 0;
}

/*
 * ah_old_init() is called when an SPI is being set up. It interprets the
 * encap_msghdr present in m, and sets up the transformation data.
 */

int
ah_old_init(struct tdb *tdbp, struct xformsw *xsp, struct mbuf *m)
{
    struct ah_old_xencap xenc;
    struct ah_old_xdata *xd;
    struct encap_msghdr *em;

    if (m->m_len < ENCAP_MSG_FIXED_LEN)
    {
        if ((m = m_pullup(m, ENCAP_MSG_FIXED_LEN)) == NULL)
        {
#ifdef ENCDEBUG
            if (encdebug)
              printf("ah_old_init(): m_pullup failed\n");
#endif /* ENCDEBUG */
            return ENOBUFS;
        }
    }

    em = mtod(m, struct encap_msghdr *);
    if (em->em_msglen - EMT_SETSPI_FLEN <= AH_OLD_XENCAP_LEN)
    {
	log(LOG_WARNING, "ah_old_init(): initialization failed");
	return EINVAL;
    }

    /* Just copy the standard fields */
    m_copydata(m, EMT_SETSPI_FLEN, AH_OLD_XENCAP_LEN, (caddr_t) &xenc);

    /* Check whether the hash algorithm is supported */
    switch (xenc.amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	case ALG_AUTH_SHA1:
#ifdef ENCDEBUG
	    if (encdebug)
	      printf("ah_old_init(): initialized TDB with hash algorithm %d\n",
		     xenc.amx_hash_algorithm);
#endif /* ENCDEBUG */
	    break;

	default:
	    log(LOG_WARNING, "ah_old_init(): unsupported authentication algorithm %d specified", xenc.amx_hash_algorithm);
	    m_freem(m);
	    return EINVAL;
    }

    if (xenc.amx_keylen + EMT_SETSPI_FLEN + AH_OLD_XENCAP_LEN != em->em_msglen)
    {
	log(LOG_WARNING, "ah_old_init(): message length (%d) doesn't match",
	    em->em_msglen);
	return EINVAL;
    }

    MALLOC(tdbp->tdb_xdata, caddr_t, sizeof(struct ah_old_xdata) +
	   xenc.amx_keylen, M_XDATA, M_WAITOK);
    if (tdbp->tdb_xdata == NULL)
    {
#ifdef ENCDEBUG
	if (encdebug)
	  printf("ah_old_init(): MALLOC() failed\n");
#endif /* ENCDEBUG */
      	return ENOBUFS;
    }

    bzero(tdbp->tdb_xdata, sizeof(struct ah_old_xdata) + xenc.amx_keylen);
    xd = (struct ah_old_xdata *) tdbp->tdb_xdata;

    /* Pointer to the transform */
    tdbp->tdb_xform = xsp;

    xd->amx_keylen = xenc.amx_keylen;
    xd->amx_hash_algorithm = xenc.amx_hash_algorithm;

    /* Copy the key material */
    m_copydata(m, EMT_SETSPI_FLEN + AH_OLD_XENCAP_LEN, xd->amx_keylen,
	       (caddr_t) xd->amx_key);

    /* Save us some time in processing */
    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    MD5Init(&(xd->amx_md5_ctx));
	    MD5Update(&(xd->amx_md5_ctx), xd->amx_key, xd->amx_keylen);
	    MD5Final(NULL, &(xd->amx_md5_ctx));
	    break;

	case ALG_AUTH_SHA1:
	    SHA1Init(&(xd->amx_sha1_ctx));
	    SHA1Update(&(xd->amx_sha1_ctx), xd->amx_key, xd->amx_keylen);
	    SHA1Final(NULL, &(xd->amx_sha1_ctx));
	    break;
    }

    bzero(ipseczeroes, IPSEC_ZEROES_SIZE);	/* paranoid */

    return 0;
}

/*
 * Free memory
 */

int
ah_old_zeroize(struct tdb *tdbp)
{
#ifdef ENCDEBUG
    if (encdebug)
      printf("ah_old_zeroize(): freeing memory\n");
#endif /* ENCDEBUG */
    FREE(tdbp->tdb_xdata, M_XDATA);
    return 0;
}

/*
 * ah_old_input() gets called to verify that an input packet
 * passes authentication.
 */

struct mbuf *
ah_old_input(struct mbuf *m, struct tdb *tdb)
{
    struct ah_old_xdata *xd;
    struct ip *ip, ipo;
    struct ah_old *ah, *aho;
    struct ifnet *rcvif;
    int ohlen, len, count, off, alen;
    struct mbuf *m0;
    MD5_CTX md5ctx; 
    SHA1_CTX sha1ctx;
    u_int8_t optval;
    u_char buffer[40];

    aho = (struct ah_old *) buffer;

    xd = (struct ah_old_xdata *) tdb->tdb_xdata;

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    alen = AH_MD5_ALEN;
	    break;

	case ALG_AUTH_SHA1:
	    alen = AH_SHA1_ALEN;
	    break;

	default:
	    log(LOG_ALERT,
		"ah_old_input(): unsupported algorithm %d in SA %x/%08x",
		xd->amx_hash_algorithm, tdb->tdb_dst, ntohl(tdb->tdb_spi));
	    m_freem(m);
	    return NULL;
    }

    ohlen = sizeof(struct ip) + AH_OLD_FLENGTH + alen;

    rcvif = m->m_pkthdr.rcvif;
    if (rcvif == NULL)
    {
#ifdef ENCDEBUG
	if (encdebug)
	  printf("ah_old_input(): receive interface is NULL!\n");
#endif /* ENCDEBUG */
	rcvif = &enc_softc;
    }
	
    if (m->m_len < ohlen)
    {
	if ((m = m_pullup(m, ohlen)) == NULL)
	{
#ifdef ENCDEBUG
	    if (encdebug)
	      printf("ah_old_input(): m_pullup() failed\n");
#endif /* ENCDEBUG */
	    ahstat.ahs_hdrops++;
	    return NULL;
	}
    }

    ip = mtod(m, struct ip *);

    if ((ip->ip_hl << 2) > sizeof(struct ip))
    {
	if ((m = m_pullup(m, ohlen - sizeof (struct ip) +
			  (ip->ip_hl << 2))) == NULL)
	{
#ifdef ENCDEBUG
	    if (encdebug)
	      printf("ah_old_input(): m_pullup() failed\n");
#endif /* ENCDEBUG */
	    ahstat.ahs_hdrops++;
	    return NULL;
	}
	
	ip = mtod(m, struct ip *);
	ah = (struct ah_old *)((u_int8_t *) ip + (ip->ip_hl << 2));
	ohlen += ((ip->ip_hl << 2) - sizeof(struct ip));
    }
    else
      ah = (struct ah_old *) (ip + 1);

    ipo = *ip;
    ipo.ip_tos = 0;
    ipo.ip_len += (ip->ip_hl << 2);	/* adjusted in ip_intr() */
    HTONS(ipo.ip_len);
    HTONS(ipo.ip_id);
    ipo.ip_off = htons(ipo.ip_off & IP_DF);	/* XXX -- and the C bit? */
    ipo.ip_ttl = 0;
    ipo.ip_sum = 0;

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    md5ctx = xd->amx_md5_ctx;
	    MD5Update(&md5ctx, (unsigned char *) &ipo, sizeof(struct ip));
	    break;

	case ALG_AUTH_SHA1:
	    sha1ctx = xd->amx_sha1_ctx;
	    SHA1Update(&sha1ctx, (unsigned char *) &ipo, sizeof(struct ip));
	    break;
    }

    /* Options */
    if ((ip->ip_hl << 2) > sizeof(struct ip))
      for (off = sizeof(struct ip); off < (ip->ip_hl << 2);)
      {
	  optval = ((u_int8_t *) ip)[off];
	  switch (optval)
	  {
	      case IPOPT_EOL:
		  switch (xd->amx_hash_algorithm)
		  {
		      case ALG_AUTH_MD5:
			  MD5Update(&md5ctx, ipseczeroes, 1);
			  break;

		      case ALG_AUTH_SHA1:
			  SHA1Update(&sha1ctx, ipseczeroes, 1);
			  break;
		  }

		  off = ip->ip_hl << 2;
		  break;

	      case IPOPT_NOP:
		  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, ipseczeroes, 1);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, ipseczeroes, 1);
                          break;
                  }

		  off++;
		  break;

	      case IPOPT_SECURITY:
	      case 133:
	      case 134:
		  optval = ((u_int8_t *) ip)[off + 1];

		  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, (u_int8_t *) ip + off, optval);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, (u_int8_t *) ip + off, optval);
                          break;
                  }

		  off += optval;
		  break;

	      default:
		  optval = ((u_int8_t *) ip)[off + 1];

		  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, ipseczeroes, optval);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, ipseczeroes, optval);
                          break;
                  }

		  off += optval;
		  break;
	  }
      }
    
    
    switch (xd->amx_hash_algorithm)
    {
        case ALG_AUTH_MD5:
            MD5Update(&md5ctx, (unsigned char *) ah, AH_OLD_FLENGTH);
	    MD5Update(&md5ctx, ipseczeroes, AH_MD5_ALEN);
            break;

        case ALG_AUTH_SHA1:
            SHA1Update(&sha1ctx, (unsigned char *) ah, AH_OLD_FLENGTH);
	    SHA1Update(&sha1ctx, ipseczeroes, AH_SHA1_ALEN);
            break;
    }

    /*
     * Code shamelessly stolen from m_copydata
     */
    off = ohlen;
    len = m->m_pkthdr.len - off;
    m0 = m;
	
    while (off > 0)
    {
	if (m0 == 0)
	  panic("ah_old_input(): m_copydata (off)");

	if (off < m0->m_len)
	  break;

	off -= m0->m_len;
	m0 = m0->m_next;
    }

    while (len > 0)
    {
	if (m0 == 0)
	  panic("ah_old_input(): m_copydata (copy)");

	count = min(m0->m_len - off, len);

	switch (xd->amx_hash_algorithm)
	{
	    case ALG_AUTH_MD5:
		MD5Update(&md5ctx, mtod(m0, unsigned char *) + off, count);
		break;

	    case ALG_AUTH_SHA1:
		SHA1Update(&sha1ctx, mtod(m0, unsigned char *) + off, count);
	}

	len -= count;
	off = 0;
	m0 = m0->m_next;
    }

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    MD5Update(&md5ctx, (unsigned char *) xd->amx_key, xd->amx_keylen);
	    MD5Final((unsigned char *) (aho->ah_data), &md5ctx);
	    break;

	case ALG_AUTH_SHA1:
	    SHA1Update(&sha1ctx, (unsigned char *) xd->amx_key, xd->amx_keylen);
	    SHA1Final((unsigned char *) (aho->ah_data), &sha1ctx);
	    break;
    }

    if (bcmp(aho->ah_data, ah->ah_data, alen))
    {
	log(LOG_ALERT, "ah_old_input(): authentication failed for packet from %x to %x, spi %08x", ipo.ip_src, ipo.ip_dst, ntohl(tdb->tdb_spi));
	ahstat.ahs_badauth++;
	m_freem(m);
	return NULL;
    }
	
    ipo = *ip;
    ipo.ip_p = ah->ah_nh;

    /* Save options */
    m_copydata(m, sizeof(struct ip), (ip->ip_hl << 2) - sizeof(struct ip),
	       (caddr_t) buffer);

    m->m_len -= (AH_OLD_FLENGTH + alen);
    m->m_data += (AH_OLD_FLENGTH + alen);
    m->m_pkthdr.len -= (AH_OLD_FLENGTH + alen);
    m->m_pkthdr.rcvif = rcvif;	/* this should not be necessary */

    ip = mtod(m, struct ip *);
    *ip = ipo;
    ip->ip_len = htons(ip->ip_len - AH_OLD_FLENGTH - alen + (ip->ip_hl << 2));
    HTONS(ip->ip_id);
    HTONS(ip->ip_off);
    ip->ip_sum = 0;

    /* Copy the options back */
    m_copyback(m, sizeof(struct ip), (ip->ip_hl << 2) - sizeof(struct ip),
	       (caddr_t) buffer);

    ip->ip_sum = in_cksum(m, (ip->ip_hl << 2));

    /* Update the counters */
    tdb->tdb_cur_packets++;
    tdb->tdb_cur_bytes += ntohs(ip->ip_len) - (ip->ip_hl << 2);
    ahstat.ahs_ibytes += ntohs(ip->ip_len) - (ip->ip_hl << 2);

    return m;
}

int
ah_old_output(struct mbuf *m, struct sockaddr_encap *gw, struct tdb *tdb,
  	      struct mbuf **mp)
{
    struct ah_old_xdata *xd;
    struct ip *ip, ipo;
    struct ah_old *ah, aho;
    register int len, off, count;
    register struct mbuf *m0;
    MD5_CTX md5ctx;
    SHA1_CTX sha1ctx;
    int ilen, ohlen, alen;
    u_int8_t optval;
    u_char opts[40];

    ahstat.ahs_output++;
    m = m_pullup(m, sizeof(struct ip));
    if (m == NULL)
    {
#ifdef ENCDEBUG
	if (encdebug)
	  printf("ah_old_output(): m_pullup() failed, SA %x/%08x\n",
		 tdb->tdb_dst, ntohl(tdb->tdb_spi));
#endif /* ENCDEBUG */
      	return ENOBUFS;
    }

    ip = mtod(m, struct ip *);
	
    xd = (struct ah_old_xdata *) tdb->tdb_xdata;

    if ((ip->ip_hl << 2) > sizeof(struct ip))
    {
	if ((m = m_pullup(m, ip->ip_hl << 2)) == NULL)
	{
#ifdef ENCDEBUG
	    if (encdebug)
	      printf("ah_old_output(): m_pullup() failed, SA &x/%08x\n",
		     tdb->tdb_dst, ntohl(tdb->tdb_spi));
#endif /* ENCDEBUG */
	    ahstat.ahs_hdrops++;
	    return NULL;
	}
	
	ip = mtod(m, struct ip *);
    }

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    alen = AH_MD5_ALEN;
	    break;

	case ALG_AUTH_SHA1:
	    alen = AH_SHA1_ALEN;
	    break;

	default:
	    log(LOG_ALERT,
                "ah_old_output(): unsupported algorithm %d in SA %x/%08x",
                xd->amx_hash_algorithm, tdb->tdb_dst, ntohl(tdb->tdb_spi));
            m_freem(m);
            return NULL;
    }

    /* Save the options */
    m_copydata(m, sizeof(struct ip), (ip->ip_hl << 2) - sizeof(struct ip),
	       (caddr_t) opts);

    ilen = ntohs(ip->ip_len);

    ohlen = AH_OLD_FLENGTH + alen;
	
    ipo.ip_v = IPVERSION;
    ipo.ip_hl = ip->ip_hl;
    ipo.ip_tos = 0;
    ipo.ip_len = htons(ohlen + ilen);
    ipo.ip_id = ip->ip_id;
    ipo.ip_off = htons(ntohs(ip->ip_off) & IP_DF);
    ipo.ip_ttl = 0;
    ipo.ip_p = IPPROTO_AH;
    ipo.ip_sum = 0;
    ipo.ip_src = ip->ip_src;
    ipo.ip_dst = ip->ip_dst;

    aho.ah_nh = ip->ip_p;
    aho.ah_hl = alen >> 2;
    aho.ah_rv = 0;
    aho.ah_spi = tdb->tdb_spi;

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    md5ctx = xd->amx_md5_ctx;
	    MD5Update(&md5ctx, (unsigned char *) &ipo, sizeof(struct ip));
	    break;

	case ALG_AUTH_SHA1:
	    sha1ctx = xd->amx_sha1_ctx;
	    SHA1Update(&sha1ctx, (unsigned char *) &ipo, sizeof(struct ip));
	    break;
    }

    /* Options */
    if ((ip->ip_hl << 2) > sizeof(struct ip))
      for (off = sizeof(struct ip); off < (ip->ip_hl << 2);)
      {
	  optval = ((u_int8_t *) ip)[off];
	  switch (optval)
	  {
              case IPOPT_EOL:
                  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, ipseczeroes, 1);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, ipseczeroes, 1);
                          break;
                  }

                  off = ip->ip_hl << 2;
                  break;

              case IPOPT_NOP:
                  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, ipseczeroes, 1);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, ipseczeroes, 1);
                          break;
                  }

                  off++;
                  break;

              case IPOPT_SECURITY:
              case 133:
              case 134:
                  optval = ((u_int8_t *) ip)[off + 1];

                  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, (u_int8_t *) ip + off, optval);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, (u_int8_t *) ip + off, optval);
                          break;
                  }

                  off += optval;
                  break;

              default:
                  optval = ((u_int8_t *) ip)[off + 1];

                  switch (xd->amx_hash_algorithm)
                  {
                      case ALG_AUTH_MD5:
                          MD5Update(&md5ctx, ipseczeroes, optval);
                          break;

                      case ALG_AUTH_SHA1:
                          SHA1Update(&sha1ctx, ipseczeroes, optval);
                          break;
                  }

                  off += optval;
                  break;
          }
      }

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    MD5Update(&md5ctx, (unsigned char *) &aho, AH_OLD_FLENGTH);
            MD5Update(&md5ctx, ipseczeroes, alen);
            break;

	case ALG_AUTH_SHA1:
	    SHA1Update(&sha1ctx, (unsigned char *) &aho, AH_OLD_FLENGTH);
	    SHA1Update(&sha1ctx, ipseczeroes, alen);
	    break;
    }

    /* Skip the IP header and any options */
    off = ip->ip_hl << 2;

    /*
     * Code shamelessly stolen from m_copydata
     */
    len = m->m_pkthdr.len - off;
	
    m0 = m;

    while (len > 0)
    {
	if (m0 == 0)
	  panic("ah_old_output(): m_copydata()");
	count = min(m0->m_len - off, len);

	switch (xd->amx_hash_algorithm)
	{
	    case ALG_AUTH_MD5:
		MD5Update(&md5ctx, mtod(m0, unsigned char *) + off, count);
		break;

	    case ALG_AUTH_SHA1:
		SHA1Update(&sha1ctx, mtod(m0, unsigned char *) + off, count);
		break;
	}

	len -= count;
	off = 0;
	m0 = m0->m_next;
    }

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    MD5Update(&md5ctx, (unsigned char *) xd->amx_key, xd->amx_keylen);
	    break;

	case ALG_AUTH_SHA1:
	    SHA1Update(&sha1ctx, (unsigned char *) xd->amx_key, xd->amx_keylen);
	    break;
    }

    ipo.ip_tos = ip->ip_tos;
    ipo.ip_id = ip->ip_id;
    ipo.ip_off = ip->ip_off;
    ipo.ip_ttl = ip->ip_ttl;
/*  ipo.ip_len = ntohs(ipo.ip_len); */
	
    M_PREPEND(m, ohlen, M_DONTWAIT);
    if (m == NULL)
    {
#ifdef ENCDEBUG
	if (encdebug)
	  printf("ah_old_output(): M_PREPEND() failed for packet from %x to %x, spi %08x\n", ipo.ip_src, ipo.ip_dst, ntohl(tdb->tdb_spi));
#endif /* ENCDEBUG */
        return ENOBUFS;
    }

    m = m_pullup(m, ohlen + (ipo.ip_hl << 2));
    if (m == NULL)
    {
#ifdef ENCDEBUG
	if (encdebug)
	  printf("ah_old_output(): m_pullup() failed for packet from %x to %x, spi %08x\n", ipo.ip_src, ipo.ip_dst, ntohl(tdb->tdb_spi));
#endif /* ENCDEBUG */
        return ENOBUFS;
    }

    ip = mtod(m, struct ip *);
    ah = (struct ah_old *) ((u_int8_t *) ip + (ipo.ip_hl << 2));
    *ip = ipo;
    ah->ah_nh = aho.ah_nh;
    ah->ah_hl = aho.ah_hl;
    ah->ah_rv = aho.ah_rv;
    ah->ah_spi = aho.ah_spi;

    /* Restore the options */
    m_copyback(m, sizeof(struct ip), (ip->ip_hl << 2) - sizeof(struct ip),
	       (caddr_t) opts);

    switch (xd->amx_hash_algorithm)
    {
	case ALG_AUTH_MD5:
	    MD5Final(ah->ah_data, &md5ctx);
	    break;

	case ALG_AUTH_SHA1:
	    SHA1Final(ah->ah_data, &sha1ctx);
	    break;
    }

    *mp = m;

    /* Update the counters */
    tdb->tdb_cur_packets++;
    tdb->tdb_cur_bytes += ntohs(ip->ip_len) - (ip->ip_hl << 2) -
			  AH_OLD_FLENGTH - alen;
    ahstat.ahs_obytes += ntohs(ip->ip_len) - (ip->ip_hl << 2) -
			 AH_OLD_FLENGTH - alen;

    return 0;
}

/*
 * National Semiconductor  SONIC Driver
 * Copyright (c) 1991   Algorithmics Ltd (http://www.algor.co.uk)
 * You may use, copy, and modify this program so long as you retain the
 * copyright line.
 */

#include "sn.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <machine/autoconf.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#ifdef NS
#include <netns/ns.h>
#include <netns/ns_if.h>
#endif

#ifdef RMP
#include <netrmp/rmp.h>
#include <netrmp/rmp_var.h>
#endif

#include <vm/vm.h>

#include "bpfilter.h"
#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#define SONICDW 32
typedef unsigned char uchar;
#include <pica/dev/if_sn.h>
#define SWR(a, x) 	(a) = (x)
#define SRD(a)		((a) & 0xffff)

#include <machine/machConst.h>
#define IS_KSEG0(v)	(((unsigned)(v) & 0xe0000000U) == 0x80000000U)
#define IS_KSEG1(v)	(((unsigned)(v) & 0xe0000000U) == 0xa0000000U)
#define IS_KSEG2(v)	(((unsigned)(v) & 0xc0000000U) == 0xc0000000U)
#define K0_TO_PHYS(v)	MACH_CACHED_TO_PHYS(v)
#define K1_TO_PHYS(v)	MACH_UNCACHED_TO_PHYS(v)
#define K0_TO_K1(v)	(((unsigned)(v)) | 0x20000000U)
#define K1_TO_K0(v)	(((unsigned)(v)) & ~0x20000000U)
#define PHYS_TO_K0(p)	MACH_PHYS_TO_CACHED(p)
#define PHYS_TO_K1(p)	MACH_PHYS_TO_UNCACHED(p)
#define wbflush() 	MachEmptyWriteBuffer()

/*
 * Statistics collected over time
 */
struct sn_stats {
	int     ls_opacks;	/* packets transmitted */
	int     ls_ipacks;	/* packets received */
	int     ls_tdr;		/* contents of tdr after collision */
	int     ls_tdef;	/* packets where had to wait */
	int     ls_tone;	/* packets with one retry */
	int     ls_tmore;	/* packets with more than one retry */
	int     ls_tbuff;	/* transmit buff errors */
	int     ls_tuflo;	/* "      uflo  "     */
	int     ls_tlcol;
	int     ls_tlcar;
	int     ls_trtry;
	int     ls_rbuff;	/* receive buff errors */
	int     ls_rfram;	/* framing     */
	int     ls_roflo;	/* overflow    */
	int     ls_rcrc;
	int     ls_rrng;	/* rx ring sequence error */
	int     ls_babl;	/* chip babl error */
	int     ls_cerr;	/* collision error */
	int     ls_miss;	/* missed packet */
	int     ls_merr;	/* memory error */
	int     ls_copies;	/* copies due to out of range mbufs */
	int     ls_maxmbufs;	/* max mbufs on transmit */
	int     ls_maxslots;	/* max ring slots on transmit */
};

struct sn_softc {
	struct	device sc_dev;
	struct	arpcom sc_ac;
#define	sc_if		sc_ac.ac_if	/* network visible interface */
#define	sc_enaddr	sc_ac.ac_enaddr	/* hardware ethernet address */

	struct	sonic_reg *sc_csr;	/* hardware pointer */
	int	sc_rxmark;		/* position in rx ring for reading buffs */

	int	sc_rramark;		/* index into rra of wp */

	int	sc_txhead;		/* index of first TDA passed to chip  */
	int	sc_missed;		/* missed packet counter */
	struct	RXpkt *	sc_lrxp;	/* last RDA available to chip */
	struct	sn_stats sc_sum;
	short	sc_iflags;
} sn_softc;

int snmatch __P((struct device *, void *, void *));
void snattach __P((struct device *, struct device *, void *));

struct cfdriver sncd = {
	NULL, "sn", snmatch, snattach, DV_IFNET, sizeof(struct sn_softc)
};

#include <assert.h>
void
__assert(file, line, failedexpr)
	const char *file, *failedexpr;
	int line;
{
	(void)printf(
	    "assertion \"%s\" failed: file \"%s\", line %d\n",
	    failedexpr, file, line);
}

void 
m_check(m)
	struct mbuf *m;
{
	if (m->m_flags & M_EXT) {
		assert(m->m_len >= 0);
		assert(m->m_len <= m->m_ext.ext_size);
		assert(m->m_data >= &m->m_ext.ext_buf[0]);
		assert(m->m_data <= &m->m_ext.ext_buf[m->m_ext.ext_size]);
		assert(m->m_data + m->m_len <= &m->m_ext.ext_buf[m->m_ext.ext_size]);
	} else if (m->m_flags & M_PKTHDR) {
		assert(m->m_len >= 0);
		assert(m->m_len <= MHLEN);
		assert(m->m_data >= m->m_pktdat);
		assert(m->m_data <= &m->m_pktdat[MHLEN]);
		assert(m->m_data + m->m_len <= &m->m_pktdat[MHLEN]);
	} else {
		assert(m->m_len >= 0);
		assert(m->m_len <= MLEN);
		assert(m->m_data >= m->m_dat);
		assert(m->m_data <= &m->m_dat[MLEN]);
		assert(m->m_data + m->m_len <= &m->m_dat[MLEN]);
	}
}

void 
m_checkm(m)
	struct mbuf *m;
{
	while (m) {
		m_check(m);
		m = m->m_next;
	}
}

int ethdebug = 1;

int snintr __P((struct sn_softc *));
int snioctl __P((struct ifnet *ifp, u_long cmd, caddr_t data));
void snstart __P((struct ifnet *ifp));
void snwatchdog __P(( /*int unit */ ));
void snreset __P((struct sn_softc *sc));

/*
 * SONIC buffers need to be aligned 16 or 32 bit aligned.
 * These macros calculate and verify alignment.
 */
#if SONICDW == 32
#define SONICALIGN 4
#else
#define SONICALIGN 2
#endif
#define SOALIGN(array) (((int)array+SONICALIGN-1) & ~(SONICALIGN-1))
#define SOALIGNED(p) (!(((uint)p)&(SONICALIGN-1)))

#define UPPER(x) (K1_TO_PHYS(x) >> 16)
#define LOWER(x) (((unsigned)(K1_TO_PHYS(x))) & 0xffff)

/*
 * buffer sizes in 32 bit mode
 * 1 TXpkt is 4 hdr words + (3 * FRAGMAX) + 1 link word
 * FRAGMAX == 16 => 54 words == 216 bytes
 *
 * 1 RxPkt is 7 words == 28 bytes
 * 1 Rda   is 4 words == 16 bytes
 */

#define NRRA	32		/* # receive resource descriptors */
#define RRAMASK	0x1f		/* why it must be poer of two */

#define NRBA	16		/* # receive buffers < NRRA */
#define NRDA	NRBA		/* # receive descriptors */
#define NTDA	4		/* # transmit descriptors */

#define CDASIZE sizeof(struct CDA)
#define RRASIZE (NRRA*sizeof(struct RXrsrc))
#define RDASIZE (NRDA*sizeof(struct RXpkt))
#define TDASIZE (NTDA*sizeof(struct TXpkt))

#define FCSSIZE	4		/* size of FCS append te received packets */

/* maximum recieve packet size plus 2 byte pad to make each
 * one aligned. 4 byte slop (required for eobc)
 */
#define RBASIZE	(sizeof(struct ether_header) + ETHERMTU	+ FCSSIZE + 2 + 4)

/* receive buffers for sonic accessed by SONIC
 * each buffer will hold one ethernet packet
 */
char rba[NRBA][RBASIZE];	/* receive buffers */

/* space for descriptors
 */
char sonicbuffers[(RRASIZE + CDASIZE + RDASIZE + TDASIZE) * 2 + SONICALIGN - 1];

/* nicely aligned pointers into the sonicbuffers
 */
struct RXrsrc *rra;	/* receiver resource descriptors */
struct RXpkt *rda;	/* receiver desriptors */
struct TXpkt *tda;	/* transmitter descriptors */
struct CDA *cda;		/* CAM descriptors */

/* Meta transmit descriptors */
struct mtd {
	struct	mtd *mtd_link;
	struct	TXpkt *mtd_txp;
	struct	mbuf *mtd_mbuf;
} mtda[NTDA];

struct mtd *mtdfree;		/* list of free meta transmit descriptors */
struct mtd *mtdhead;		/* head of descriptors assigned to chip */
struct mtd *mtdtail;		/* tail of descriptors assigned to chip */
struct mtd *mtdnext;		/* next descriptor to give to chip */

void mtd_free __P((struct mtd *));
struct mtd *mtd_alloc __P((void));

int sngetaddr __P((struct sn_softc *sc));
int sninit __P((int unit));
int snstop __P((int unit));
int sonicput __P((struct sn_softc *sc, struct mbuf *m0));

int
snmatch(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	struct cfdata *cf = match;
	struct confargs *ca = aux;

	/* XXX CHECK BUS */
	/* make sure that we're looking for this type of device. */
	if (!BUS_MATCHNAME(ca, "sonic"))
		return (0);

	return (1);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
void
snattach(parent, self, aux)
	struct device *parent, *self;
	void   *aux;
{
	struct sn_softc *sc = (void *)self;
	struct confargs *ca = aux;
	struct ifnet *ifp = &sc->sc_if;
	struct cfdata *cf = sc->sc_dev.dv_cfdata;
	u_long iobase;

	sc->sc_csr = (struct sonic_reg *)BUS_CVTADDR(ca);

	sngetaddr(sc);
	printf(" address %s\n", ether_sprintf(sc->sc_enaddr));

	BUS_INTR_ESTABLISH(ca, snintr, (void *)sc);

	ifp->if_name = "sn";
	ifp->if_unit = sc->sc_dev.dv_unit;
	ifp->if_ioctl = snioctl;
	ifp->if_start = snstart;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_watchdog = snwatchdog;
#if NBPFILTER > 0
	bpfattach(&ifp->if_bpf, ifp, DLT_EN10MB, sizeof(struct ether_header));
#endif
	if_attach(ifp);
	ether_ifattach(ifp);
}

int
snioctl(ifp, cmd, data)
	struct ifnet *ifp;
	u_long cmd;
	caddr_t data;
{
	struct ifaddr *ifa;
	struct sn_softc *sc = sncd.cd_devs[ifp->if_unit];
	int     s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifa = (struct ifaddr *)data;
		ifp->if_flags |= IFF_UP;
		switch (ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			(void)sninit(ifp->if_unit);
			arp_ifinit(&sc->sc_ac, ifa);
			break;
#endif
#ifdef NS
		case AF_NS:
			{
				struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);

				if (ns_nullhost(*ina)) {
					ina->x_host = *(union ns_host *)(sc->sc_addr);
				} else {
					/* XXX
					 * add an extra i/f address to
					 * sonic filter
					 */
				}
			}
			(void)sninit(ifp->if_unit);
			break;
#endif	/* NS */
		default:
			(void)sninit(ifp->if_unit);
			break;
		}
		break;

	case SIOCSIFFLAGS:
		if ((ifp->if_flags & IFF_UP) == 0 &&
		    ifp->if_flags & IFF_RUNNING) {
			snstop(ifp->if_unit);
			ifp->if_flags &= ~IFF_RUNNING;
		} else if (ifp->if_flags & IFF_UP &&
		    (ifp->if_flags & IFF_RUNNING) == 0)
			(void)sninit(ifp->if_unit);
		/*
		 * If the state of the promiscuous bit changes, the interface
		 * must be reset to effect the change.
		 */
		if (((ifp->if_flags ^ sc->sc_iflags) & IFF_PROMISC) &&
		    (ifp->if_flags & IFF_RUNNING)) {
			sc->sc_iflags = ifp->if_flags;
			printf("change in flags\n");
			snreset(sc);
			snstart(ifp);
		}
		break;

	case SIOCADDMULTI:
		error = ether_addmulti((struct ifreq *)data, &sc->sc_ac);
		goto update_multicast;

	case SIOCDELMULTI:
		error = ether_delmulti((struct ifreq *)data, &sc->sc_ac);
	update_multicast:
		if (error == ENETRESET) {
			/*
			 * Multicast list has changed; set the hardware
			 * filter accordingly.
			 */
			printf("new multicast\n");
			snreset(sc);
			error = 0;
		}
		break;
	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}

/*
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
void
snstart(ifp)
	struct ifnet *ifp;
{
	struct sn_softc *sc = sncd.cd_devs[ifp->if_unit];
	struct mbuf *m;
	int	len;

	if ((sc->sc_if.if_flags & IFF_RUNNING) == 0)
		return;
	IF_DEQUEUE(&sc->sc_if.if_snd, m);
	if (m == 0)
		return;

printf("snstart\n");
	/*
	 * If there is nothing in the o/p queue, and there is room in
	 * the Tx ring, then send the packet directly.  Otherwise append
	 * it to the o/p queue.
	 */
	len = sonicput(sc, m);
printf("len=%d\n", len);
#if NBPFILTER > 0
	/*
	 * If bpf is listening on this interface, let it
	 * see the packet before we commit it to the wire.
	 */
	if (sc->sc_if.if_bpf)
		bpf_mtap(sc->sc_if.if_bpf, m);
#endif

	sc->sc_if.if_opackets++;	/* # of pkts */
	sc->sc_sum.ls_opacks++;		/* # of pkts */
}

/*
 * This is called from sonicioctl() when /etc/ifconfig is run to set
 * the address or switch the i/f on.
 */
void caminitialise __P((void));
void camentry __P((int, u_char *ea));
void camprogram __P((struct sn_softc *));
int allocatebuffers __P((void));
void initialise_tda __P((struct sn_softc *));
void initialise_rda __P((struct sn_softc *));
void initialise_rra __P((struct sn_softc *));

/*
 * reset and restart the SONIC.  Called in case of fatal
 * hardware/software errors.
 */
void
snreset(sc)
	struct sn_softc *sc;
{
	printf("snreset\n");
	snstop(sc->sc_dev.dv_unit);
	sninit(sc->sc_dev.dv_unit);
}

int 
sninit(unit)
	int unit;
{
	struct sn_softc *sc = sncd.cd_devs[unit];
	struct sonic_reg *csr = sc->sc_csr;
	int s, error;

	if (sc->sc_if.if_flags & IFF_RUNNING)
		/* already running */
		return (0);

	s = splimp();

printf("sninit csr %8x\n", csr);

	csr->s_cr = CR_RST;	/* s_dcr only accessable reset mode! */

	/* config it */
	csr->s_dcr = DCR_ASYNC | DCR_WAIT0 | DCR_DW32 | DCR_DMABLOCK |
	    DCR_RFT24 | DCR_TFT24;
	csr->s_rcr = RCR_BRD | RCR_LBNONE;
	csr->s_imr = IMR_PRXEN | IMR_PTXEN | IMR_TXEREN | IMR_HBLEN |
	    IMR_LCDEN;

	/* clear pending interrupts */
	csr->s_isr = 0x7fff;

	/* clear tally counters */
	csr->s_crct = -1;
	csr->s_faet = -1;
	csr->s_mpt = -1;

	/* initialize memory descriptors */
	if (error = allocatebuffers())
		goto bad;

	initialise_tda(sc);
	initialise_rda(sc);
	initialise_rra(sc);

	/* enable the chip */
	csr->s_cr = 0;
	wbflush();

	/* program the CAM with our address */
	caminitialise();
	camentry(0, sc->sc_enaddr);
	camprogram(sc);

	/* get it to read resource descriptors */
	csr->s_cr = CR_RRRA;
	wbflush();
	while (csr->s_cr & CR_RRRA)
		continue;

	/* enable rx */
	csr->s_cr = CR_RXEN;
	wbflush();

	/* flag interface as "running" */
	sc->sc_if.if_flags |= IFF_RUNNING;

	splx(s);
	return (0);

bad:
	snstop(sc->sc_dev.dv_unit);
	return (error);
}

/*
 * close down an interface and free its buffers
 * Called on final close of device, or if sninit() fails
 * part way through.
 */
int 
snstop(unit)
	int unit;
{
	struct sn_softc *sc = sncd.cd_devs[unit];
	struct mtd *mtd;
	int s = splimp();

	printf("snstop\n");

	/* stick chip in reset */
	sc->sc_csr->s_cr = CR_RST;
	wbflush();

	/* free all receive buffers (currently static so nothing to do) */

	/* free all pending transmit mbufs */
	while (mtd = mtdhead) {
		mtdhead = mtdhead->mtd_link;
		if (mtd->mtd_mbuf)
			m_freem(mtd->mtd_mbuf);
		mtd->mtd_mbuf = 0;
		mtd_free(mtd);
	}
	mtdnext = mtd_alloc();

	sc->sc_if.if_timer = 0;
	sc->sc_if.if_flags &= ~(IFF_RUNNING | IFF_UP);

	splx(s);
	return (0);
}

/*
 * Called if any Tx packets remain unsent after 5 seconds,
 * In all cases we just reset the chip, and any retransmission
 * will be handled by higher level protocol timeouts.
 */
void
snwatchdog(unit)
	int unit;
{
	struct sn_softc *sc = sncd.cd_devs[unit];

printf("watchdog\n");

	if (mtdhead && mtdhead->mtd_mbuf) {
		/* something still pending for transmit */
		if (mtdhead->mtd_txp->status == 0)
			log(LOG_ERR, "%s%d: Tx - timeout\n",
			    sc->sc_if.if_name, sc->sc_if.if_unit);
		else
			log(LOG_ERR, "%s%d: Tx - lost interrupt\n",
			    sc->sc_if.if_name, sc->sc_if.if_unit);
		snreset(sc);
	}
}
/*
 * stuff packet into sonic (at splimp)
*/
int 
sonicput(sc, m0)
	struct sn_softc *sc;
	struct mbuf *m0;
{
	struct sonic_reg *csr = sc->sc_csr;
	struct TXpkt *txp;
	struct mtd *mtdnew;
	struct mbuf *m;
	int len = 0, fr = 0;
	int i;

	/* grab the replacement mtd */
	if ((mtdnew = mtd_alloc()) == 0)
		return (0);
printf("got mtd %8x next %8x\n", mtdnew, mtdnext);

	/* this packet goes to mdtnext fill in the TDA */
	mtdnext->mtd_mbuf = m0;
	txp = mtdnext->mtd_txp;
	SWR(txp->config, 0);

	for (m = m0; m; m = m->m_next) {
		unsigned va = (unsigned) mtod(m, caddr_t);
		int resid = m->m_len;

printf("va=%x len=%d\n", va, resid);
		len += resid;

		while (resid) {
			unsigned pa;
			unsigned n;

			if (IS_KSEG0(va)) {
				pa = K0_TO_PHYS(va);
				n = resid;
			} else if (IS_KSEG1(va)) {
				pa = K1_TO_PHYS(va);
				n = resid;
			} else if (IS_KSEG2(va)) {
				pa = pmap_extract(pmap_kernel(), va);
				if (pa == 0)
					panic("sonicput: va not mapped");
				pa |= (va & PGOFSET);
				n = resid;
				if (n > NBPG - (va & PGOFSET)) {
					n = NBPG - (va & PGOFSET);
					va += NBPG;	/* XXX TDR added */
				}
			} else {
				panic("sonicput: bad va for mbuf");
			}

			for (i = 0; i < n; i++)
				printf("%2x ", (*(u_char *)(va + i)) & 0xff);
			printf("\n");

printf("pa=%x len=%d\n", pa, n);
			if (fr < FRAGMAX) {
				SWR(txp->u[fr].frag_ptrlo, pa & 0xffff);
				SWR(txp->u[fr].frag_ptrhi, pa >> 16);
				SWR(txp->u[fr].frag_size, n);
			}
			fr++;
			va += n;
			resid -= n;
		}
	}
	if (fr > FRAGMAX) {
		mtd_free(mtdnew);
		m_freem(m0);
		log(LOG_ERR, "%s%d: tx too many fragments %d\n",
		    sc->sc_if.if_name, sc->sc_if.if_unit, fr);
		sc->sc_if.if_oerrors++;
		return (len);
	}

	/*
	 * pad out last fragment for minimum size
	 * 
	 * XXX is this incorrectly done? does it cause random bytes of
	 * mem to be sent? if so, it should instead point to a zero'd
	 * zone of mem.
	 */
	if (len < ETHERMIN + sizeof(struct ether_header)) {
		int pad = ETHERMIN + sizeof(struct ether_header) - len;

		SWR(txp->u[fr - 1].frag_size, pad + SRD(txp->u[fr - 1].frag_size));
		len = ETHERMIN + sizeof(struct ether_header);
	}
	SWR(txp->frag_count, fr);
	SWR(txp->pkt_size, len);

	/* link onto the next mtd that will be used */
	SWR(txp->u[fr].tlink, LOWER(mtdnew->mtd_txp) | EOL);

	if (mtdhead == 0) {
		/* no current transmit list start with this one */
		mtdtail = mtdhead = mtdnext;
		csr->s_ctda = LOWER(txp);
	} else {
		/*
		 * have a transmit list append it to end note
		 * mtdnext is already physicaly linked to mtdtail in
		 * mtdtail->mtd_txp->u[mtdtail->mtd_txp->frag_count].tlink
		 */
		SWR(mtdtail->mtd_txp->u[mtdtail->mtd_txp->frag_count].tlink,
		    SRD(mtdtail->mtd_txp->u[mtdtail->mtd_txp->frag_count].tlink) & ~EOL);
		mtdtail = mtdnext;
	}
	mtdnext->mtd_link = mtdnew;
	mtdnext = mtdnew;

	/* make sure chip is running */
	wbflush();
	csr->s_cr = CR_TXP;
	wbflush();
	sc->sc_if.if_timer = 5;	/* 5 seconds to watch for failing to transmit */
	return (len);
}

/*
 * Get the ethernet addr, store it and print it
 */
unsigned 
digit(c)
	int c;
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

int 
sngetaddr(sc)
	struct sn_softc *sc;
{
	extern char *prom_getenv();
	unsigned i, x, y;
	char   *cp, *ea;

#if 0
	cp = ea = prom_getenv("ethaddr");
#else
	cp = ea = "08:00:20:a0:66:54";
#endif
	if (!cp || !*cp)
		goto bad;
	for (i = 0; i < 6; i++) {
		if (*cp == ':')
			cp++;
		x = digit(*cp++);
		if (x < 0)
			goto bad;
		y = digit(*cp++);
		if (y < 0)
			goto bad;
		sc->sc_enaddr[i] = (x << 4) | y;
	}
	if (*cp) {
bad:
		printf("$ethaddr not set or incorrect\n");
		return (-1);
	}
	return (0);
}

void sonictxint __P((struct sn_softc *));
void sonicrxint __P((struct sn_softc *));

int sonic_read __P((struct sn_softc *, struct RXpkt *));
struct mbuf *sonic_get __P((struct sn_softc *, struct ether_header *, int, int));

void 
mtd_free(mtd)
	struct mtd *mtd;
{
	mtd->mtd_link = mtdfree;
	mtdfree = mtd;
}

struct mtd *
mtd_alloc()
{
	struct mtd *mtd = mtdfree;

	if (mtd) {
		mtdfree = mtd->mtd_link;
		mtd->mtd_link = 0;
	}
	return (mtd);
}

/*
 * CAM support
 */
void 
caminitialise()
{
	int     i;

	for (i = 0; i < MAXCAM; i++)
		SWR(cda->desc[i].cam_ep, i);
	SWR(cda->enable, 0);
}

void 
camentry(entry, ea)
	int entry;
	u_char *ea;
{
	SWR(cda->desc[entry].cam_ep, entry);
	SWR(cda->desc[entry].cam_ap2, (ea[5] << 8) | ea[4]);
	SWR(cda->desc[entry].cam_ap1, (ea[3] << 8) | ea[2]);
	SWR(cda->desc[entry].cam_ap0, (ea[1] << 8) | ea[0]);
	SWR(cda->enable, SRD(cda->enable) | (1 << entry));
}

void 
camprogram(sc)
	struct sn_softc *sc;
{
	struct sonic_reg *csr;
	int     timeout;
	int     i;

	csr = sc->sc_csr;
	csr->s_cdp = LOWER(cda);
	csr->s_cdc = MAXCAM;
	csr->s_cr = CR_LCAM;
	wbflush();

	timeout = 10000;
	while (csr->s_cr & CR_LCAM && timeout--)
		continue;
	if (timeout == 0) {
		/* XXX */
		panic("sonic: CAM initialisation failed\n");
	}
	timeout = 10000;
	while ((csr->s_isr & ISR_LCD) == 0 && timeout--)
		continue;

	if (csr->s_isr & ISR_LCD)
		csr->s_isr = ISR_LCD;
	else
		printf("sonic: CAM initialisation without interrupt\n");
}

#ifdef notdef
void 
camdump()
{
	printf("CAM entries:\n");
	csr->s_cr = CR_RST;
	wbflush();

	for (i = 0; i < 16; i++) {
		ushort  ap2, ap1, ap0;
		csr->s_cep = i;
		wbflush();
		ap2 = csr->s_cap2;
		ap1 = csr->s_cap1;
		ap0 = csr->s_cap0;
		printf("%d: ap2=0x%x ap1=0x%x ap0=0x%x\n", i, ap2, ap1, ap0);
	}
	printf("CAM enable 0x%x\n", csr->s_cep);

	csr->s_cr = 0;
	wbflush();
}
#endif

/*
 * because the sonic is basicly 16bit device it 'concatenates'
 * a higher buffer address to a 16 bit offset this will cause wrap
 * around probelms near the end of 64k !!
 */
int 
allocatebuffers()
{
	int     p, n;
	p = SOALIGN(sonicbuffers);

	p = K0_TO_K1(p);

	if ((p ^ (p + RRASIZE + CDASIZE)) & 0x10000)
		p = (p + 0x10000) & ~0xffff;
	rra = (struct RXrsrc *) p;
	p += RRASIZE;
	cda = (struct CDA *) p;
	p += CDASIZE;

	if ((p ^ (p + RDASIZE)) & 0x10000)
		p = (p + 0x10000) & ~0xffff;
	rda = (struct RXpkt *) p;
	p += RDASIZE;

	if ((p ^ (p + TDASIZE)) & 0x10000)
		p = (p + 0x10000) & ~0xffff;
	tda = (struct TXpkt *) p;
	p += TDASIZE;

	printf("sonic buffers: rra=0x%x cda=0x%x rda=0x%x tda=0x%x\n",
	    rra, cda, rda, tda);

	/* check sanity of buffer addresese */
	assert(K1_TO_PHYS(rra) >= K0_TO_PHYS(&sonicbuffers[0]));
	assert(K1_TO_PHYS(rra) < K0_TO_PHYS(&sonicbuffers[sizeof(sonicbuffers)]));

	assert(K1_TO_PHYS(cda) >= K0_TO_PHYS(&sonicbuffers[0]));
	assert(K1_TO_PHYS(cda) < K0_TO_PHYS(&sonicbuffers[sizeof(sonicbuffers)]));

	assert(K1_TO_PHYS(rda) >= K0_TO_PHYS(&sonicbuffers[0]));
	assert(K1_TO_PHYS(rda) < K0_TO_PHYS(&sonicbuffers[sizeof(sonicbuffers)]));

	assert(K1_TO_PHYS(tda) >= K0_TO_PHYS(&sonicbuffers[0]));
	assert(K1_TO_PHYS(tda) < K0_TO_PHYS(&sonicbuffers[sizeof(sonicbuffers)]));

	/* return errno */
	return (0);
}

void 
initialise_tda(sc)
	struct sn_softc *sc;
{
	struct sonic_reg *csr;
	struct mtd *mtd;
	int     i;

	csr = sc->sc_csr;

	mtdfree = mtdhead = mtdtail = (struct mtd *) 0;

	for (i = 0; i < NTDA; i++) {
		mtd = &mtda[i];
		mtd->mtd_txp = &tda[i];
		mtd->mtd_mbuf = (struct mbuf *) 0;
		mtd_free(mtd);
	}
	mtdnext = mtd_alloc();

	csr->s_utda = UPPER(tda);
}

void 
initialise_rda(sc)
	struct sn_softc *sc;
{
	struct sonic_reg *csr;
	int     i;

	csr = sc->sc_csr;

	/* link the RDA's together into a circular list */
	for (i = 0; i < (NRDA - 1); i++) {
		SWR(rda[i].rlink, LOWER(&rda[i + 1]));
		SWR(rda[i].in_use, 1);
	}
	SWR(rda[NRDA - 1].rlink, LOWER(&rda[0]) | EOL);
	SWR(rda[NRDA - 1].in_use, 1);

	/* mark end of receive descriptor list */
	sc->sc_lrxp = &rda[NRDA - 1];

	sc->sc_rxmark = 0;

	csr->s_urda = UPPER(&rda[0]);
	csr->s_crda = LOWER(&rda[0]);
	wbflush();
}

void 
initialise_rra(sc)
	struct sn_softc *sc;
{
	struct sonic_reg *csr;
	int     i;

	csr = sc->sc_csr;

	csr->s_eobc = sizeof(rba[0]) / 2 - 2;	/* must be >= MAXETHERPKT */
	csr->s_urra = UPPER(rra);
	csr->s_rsa = LOWER(rra);
	csr->s_rea = LOWER(&rra[NRRA]);
	csr->s_rrp = LOWER(rra);

	/* fill up SOME of the rra with buffers */
	for (i = 0; i < NRBA; i++) {
		SWR(rra[i].buff_ptrhi, UPPER(&rba[i][0]));
		SWR(rra[i].buff_ptrlo, LOWER(&rba[i][0]));
		SWR(rra[i].buff_wchi, UPPER(sizeof(rba[i]) / 2));
		SWR(rra[i].buff_wclo, LOWER(sizeof(rba[i]) / 2));
	}
	sc->sc_rramark = NRBA;
	csr->s_rwp = LOWER(&rra[sc->sc_rramark]);
	wbflush();
}

int 
snintr(sc)
	struct sn_softc *sc;
{
	struct sonic_reg *csr = sc->sc_csr;
	int	isr;

printf("snintr\n");
	while (isr = csr->s_isr) {
		/* scrub the interrupts that we are going to service */
		printf("sonic isr %x\n", isr);
		csr->s_isr = isr;
		wbflush();

		if (isr & (ISR_BR | ISR_LCD | ISR_PINT | ISR_TC))
			printf("sonic: unexpected interrupt status 0x%x\n", isr);

		if (isr & (ISR_TXDN | ISR_TXER))
			sonictxint(sc);

		if (isr & ISR_PKTRX)
			sonicrxint(sc);

		if (isr & (ISR_HBL | ISR_RDE | ISR_RBE | ISR_RBAE | ISR_RFO)) {
			if (isr & ISR_HBL)
				printf("sonic: no heartbeat\n");
			if (isr & ISR_RDE)
				printf("sonic: receive descriptors exhausted\n");
			if (isr & ISR_RBE)
				printf("sonic: receive buffers exhausted\n");
			if (isr & ISR_RBAE)
				printf("sonic: receive buffer area exhausted\n");
			if (isr & ISR_RFO)
				printf("sonic: receive FIFO overrun\n");
		}
		if (isr & (ISR_CRC | ISR_FAE | ISR_MP)) {
#ifdef notdef
			if (isr & ISR_CRC)
				sc->sc_crctally++;
			if (isr & ISR_FAE)
				sc->sc_faetally++;
			if (isr & ISR_MP)
				sc->sc_mptally++;
#endif
		}
	}
	return (1);
}

/*
 * Transmit interrupt routine
 */
void 
sonictxint(sc)
	struct sn_softc *sc;
{
	struct TXpkt *txp;
	struct sonic_reg *csr;
	struct mtd *mtd;

	if (mtdhead == (struct mtd *) 0)
		return;

	csr = sc->sc_csr;

	while (mtd = mtdhead) {
		struct mbuf *m = mtd->mtd_mbuf;

		if (m == 0)
			break;

		txp = mtd->mtd_txp;

		if (SRD(txp->status) == 0)	/* it hasn't really gone yet */
			return;

		if (ethdebug) {
			struct ether_header *eh = mtod(m, struct ether_header *);
			printf("xmit status=0x%x len=%d type=0x%x from %s",
			    txp->status,
			    txp->pkt_size,
			    htons(eh->ether_type),
			    ether_sprintf(eh->ether_shost));
			printf(" (to %s)\n", ether_sprintf(eh->ether_dhost));
		}
		m_freem(m);
		mtd->mtd_mbuf = 0;
		mtdhead = mtd->mtd_link;

		mtd_free(mtd);

		if ((SRD(txp->status) & TCR_PTX) == 0) {
			printf("sonic: Tx packet status=0x%x\n", txp->status);

			if (mtdhead != mtdnext) {
				printf("resubmitting remaining packets\n");
				csr->s_ctda = LOWER(mtdhead->mtd_txp);
				csr->s_cr = CR_TXP;
				wbflush();
				return;
			}
		}
	}
	/* mtdhead should be at mtdnext (go) */
	assert(mtdhead == mtdnext);
	assert(mtdhead->mtd_link == 0);
	mtdhead = 0;

	/* and start feeding any queued packets to chip */
	while (1) {
		struct mbuf *m;

		IF_DEQUEUE(&sc->sc_if.if_snd, m);
		if (m == 0)	/* nothing left to send */
			break;
		if (!sonicput(sc, m)) {	/* not enough space */
			IF_PREPEND(&sc->sc_if.if_snd, m);
			break;
		}
	}
}

/*
 * Receive interrupt routine
 */
void 
sonicrxint(sc)
	struct sn_softc *sc;
{
	struct sonic_reg *csr = sc->sc_csr;
	struct RXpkt *rxp;
	u_long  addr;
	int     orra;

	rxp = &rda[sc->sc_rxmark];

	while (SRD(rxp->in_use) == 0) {
		unsigned status = SRD(rxp->status);
		printf("sonic: Rx packet status=%R\n", status);
		if ((status & RCR_LPKT) == 0)
			printf("sonic: more than one packet in RBA!\n");
		assert(PSNSEQ(SRD(rxp->seq_no)) == 0);

		if (status & RCR_PRX) {
			if (sonic_read(sc, rxp)) {
				sc->sc_if.if_ipackets++;
				sc->sc_sum.ls_ipacks++;
				sc->sc_missed = 0;
			}
		} else
			sc->sc_if.if_ierrors++;

		/*
		 * give receive buffer area back to chip XXX what buffer
		 * did the sonic use for this descriptor answer look at
		 * the rba sequence number !! 
		 */
		orra = RBASEQ(SRD(rxp->seq_no)) & RRAMASK;

		assert(SRD(rxp->pkt_ptrhi) == SRD(rra[orra].buff_ptrhi));
		assert(SRD(rxp->pkt_ptrlo) == SRD(rra[orra].buff_ptrlo));
		assert(SRD(rra[orra].buff_wclo));

		/*
		 * orra is now empty of packets and can be freed if
		 * sonic read didnt copy it out then we would have to
		 * wait !!
		 * (dont bother add it back in again straight away)
		 */
		rra[sc->sc_rramark] = rra[orra];

		/* zap old rra for fun */
		rra[orra].buff_wchi = 0;
		rra[orra].buff_wclo = 0;

		sc->sc_rramark = (sc->sc_rramark + 1) & RRAMASK;
		csr->s_rwp = LOWER(&rra[sc->sc_rramark]);
		wbflush();

		/*
		 * give recieve descriptor back to chip simple
		 * list is circular
		 */
		SWR(rxp->in_use, 1);
		SWR(rxp->rlink, SRD(rxp->rlink) | EOL);
		SWR(sc->sc_lrxp->rlink, SRD(sc->sc_lrxp->rlink) & ~EOL);
		sc->sc_lrxp = rxp;

		if (++sc->sc_rxmark >= NRDA)
			sc->sc_rxmark = 0;
		rxp = &rda[sc->sc_rxmark];
	}
}

/*
 * sonic_read -- pull packet off interface and forward to
 * appropriate protocol handler
 */
int 
sonic_read(sc, rxp)
	struct sn_softc *sc;
	struct RXpkt *rxp;
{
	struct ifnet *ifp = &sc->sc_if;
	/*extern char *ether_sprintf();*/
	struct ether_header *et;
	struct mbuf *m;
	int     len, off, i;
	caddr_t	pkt;

	/*
         * Get input data length.
         * Get pointer to ethernet header (in input buffer).
         * Deal with trailer protocol: if type is PUP trailer
         * get true type from first 16-bit word past data.
         * Remember that type was trailer by setting off.
         */

	len = SRD(rxp->byte_count) - sizeof(struct ether_header) - FCSSIZE;
	pkt = (caddr_t)PHYS_TO_K1((SRD(rxp->pkt_ptrhi) << 16) | SRD(rxp->pkt_ptrlo));
	et = (struct ether_header *)pkt;

	if (ethdebug) {
		printf("rcvd 0x%x status=0x%x, len=%d type=0x%x from %s",
		    et, rxp->status, len, htons(et->ether_type),
		    ether_sprintf(et->ether_shost));
		printf(" (to %s)\n", ether_sprintf(et->ether_dhost));
	}
	if (len < ETHERMIN || len > ETHERMTU) {
		printf("sonic: invalid packet length %d bytes\n", len);
		return (0);
	}

	off = 0;

#if NBPFILTER > 0
	/*
	 * Check if there's a bpf filter listening on this interface.
	 * If so, hand off the raw packet to enet, then discard things
	 * not destined for us (but be sure to keep broadcast/multicast).
	 */
	if (sc->sc_if.if_bpf) {
		bpf_tap(sc->sc_if.if_bpf, pkt,
		    len + sizeof(struct ether_header));
		if ((flags & (M_BCAST | M_MCAST)) == 0 &&
		    bcmp(et->ether_dhost, sc->sc_addr,
			    sizeof(et->ether_dhost)) != 0)
			return;
	}
#endif
	m = sonic_get(sc, et, len, off);
	if (m == NULL)
		return (0);
	ether_input(ifp, et, m);
}

#define sonicdataaddr(eh, off, type)       ((type)(((caddr_t)((eh)+1)+(off))))

/*
 * munge the recieved packet into an mbuf chain
 * because we are using stupif buffer management this
 * is slow.
*/
struct mbuf *
sonic_get(sc, eh, datalen, off0)
	struct sn_softc *sc;
	struct ether_header *eh;
	int datalen, off0;
{
	struct mbuf *m;
	struct mbuf *top = 0, **mp = &top;
	int off = off0, len;
	char   *spkt = sonicdataaddr(eh, 0, caddr_t);
	char   *epkt = spkt + datalen;
	char *cp = spkt;

	epkt = cp + datalen;
	if (off0) {
		cp += off + 2 * sizeof(u_short);
		datalen -= 2 * sizeof(u_short);
	}
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		return (0);
	m->m_pkthdr.rcvif = &sc->sc_if;
	m->m_pkthdr.len = datalen;
	m->m_len = MHLEN;

	while (datalen > 0) {
		if (top) {
			MGET(m, M_DONTWAIT, MT_DATA);
			if (m == 0) {
				m_freem(top);
				return (0);
			}
			m->m_len = MLEN;
		}
		len = min(datalen, epkt - cp);
		if (len >= MINCLSIZE) {
			MCLGET(m, M_DONTWAIT);
			if (m->m_flags & M_EXT)
				m->m_len = len = min(len, MCLBYTES);
			else
				len = m->m_len;
		} else {
			/*
		         * Place initial small packet/header at end of mbuf.
		         */
			if (len < m->m_len) {
				if (top == 0 && len + max_linkhdr <= m->m_len)
					m->m_data += max_linkhdr;
				m->m_len = len;
			} else
				len = m->m_len;
		}
		bcopy(cp, mtod(m, caddr_t), (unsigned) len);
		cp += len;
		*mp = m;
		mp = &m->m_next;
		datalen -= len;
		if (cp == epkt)
			cp = spkt;
	}
	return (top);
}

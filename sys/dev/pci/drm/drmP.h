/* $OpenBSD: drmP.h,v 1.208 2016/04/08 08:27:53 kettenis Exp $ */
/* drmP.h -- Private header for Direct Rendering Manager -*- linux-c -*-
 * Created: Mon Jan  4 10:05:05 1999 by faith@precisioninsight.com
 */
/*-
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Rickard E. (Rik) Faith <faith@valinux.com>
 *    Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef _DRM_P_H_
#define _DRM_P_H_

#if defined(_KERNEL) || defined(__KERNEL__)

//#define DRMDEBUG

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/malloc.h>
#include <sys/pool.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/mutex.h>
#include <sys/tree.h>
#include <sys/endian.h>
#include <sys/stdint.h>
#include <sys/memrange.h>
#include <sys/extent.h>
#include <sys/rwlock.h>

#ifdef DDB
#include <ddb/db_var.h>
#endif

#include <uvm/uvm_extern.h>
#include <uvm/uvm_object.h>

#include <dev/pci/pcidevs.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/agpvar.h>
#include <machine/bus.h>

#include "drm_linux.h"
#include "drm_linux_list.h"
#include "drm.h"
#include "drm_vma_manager.h"
#include "drm_mm.h"
#include "drm_atomic.h"
#include "agp.h"

/***********************************************************************/
/** \name DRM template customization defaults */
/*@{*/

/* driver capabilities and requirements mask */
#define DRIVER_USE_AGP     0x1
#define DRIVER_PCI_DMA     0x8
#define DRIVER_SG          0x10
#define DRIVER_HAVE_DMA    0x20
#define DRIVER_HAVE_IRQ    0x40
#define DRIVER_IRQ_SHARED  0x80
#define DRIVER_GEM         0x1000
#define DRIVER_MODESET     0x2000
#define DRIVER_PRIME       0x4000
#define DRIVER_RENDER      0x8000

#define	DRM_DEBUGBITS_DEBUG		0x1
#define	DRM_DEBUGBITS_KMS		0x2
#define	DRM_DEBUGBITS_FAILED_IOCTL	0x4

#define __OS_HAS_AGP		(NAGP > 0)

				/* Internal types and structures */
#define DRM_IF_VERSION(maj, min) (maj << 16 | min)

#define DRM_CURRENTPID		curproc->p_pid
#define DRM_MAXUNITS		8

/* DRM_SUSER returns true if the user is superuser */
#define DRM_SUSER(p)		(suser(p, 0) == 0)
#define DRM_MTRR_WC		MDF_WRITECOMBINE

#define DRM_WAKEUP(x)		wakeup(x)

extern int ticks;

#define drm_msleep(x)		mdelay(x)

extern struct cfdriver drm_cd;

/* freebsd compat */
#define	TAILQ_CONCAT(head1, head2, field) do {				\
	if (!TAILQ_EMPTY(head2)) {					\
		*(head1)->tqh_last = (head2)->tqh_first;		\
		(head2)->tqh_first->field.tqe_prev = (head1)->tqh_last;	\
		(head1)->tqh_last = (head2)->tqh_last;			\
		TAILQ_INIT((head2));					\
	}								\
} while (0)

#define DRM_ARRAY_SIZE nitems

/* DRM_READMEMORYBARRIER() prevents reordering of reads.
 * DRM_WRITEMEMORYBARRIER() prevents reordering of writes.
 * DRM_MEMORYBARRIER() prevents reordering of reads and writes.
 */
#if defined(__i386__)
#define DRM_READMEMORYBARRIER()		__asm __volatile( \
					"lock; addl $0,0(%%esp)" : : : "memory");
#define DRM_WRITEMEMORYBARRIER()	__asm __volatile("" : : : "memory");
#define DRM_MEMORYBARRIER()		__asm __volatile( \
					"lock; addl $0,0(%%esp)" : : : "memory");
#elif defined(__alpha__)
#define DRM_READMEMORYBARRIER()		alpha_mb();
#define DRM_WRITEMEMORYBARRIER()	alpha_wmb();
#define DRM_MEMORYBARRIER()		alpha_mb();
#elif defined(__amd64__)
#define DRM_READMEMORYBARRIER()		__asm __volatile( \
					"lock; addl $0,0(%%rsp)" : : : "memory");
#define DRM_WRITEMEMORYBARRIER()	__asm __volatile("" : : : "memory");
#define DRM_MEMORYBARRIER()		__asm __volatile( \
					"lock; addl $0,0(%%rsp)" : : : "memory");
#elif defined(__mips64__)
#define DRM_READMEMORYBARRIER()		DRM_MEMORYBARRIER() 
#define DRM_WRITEMEMORYBARRIER()	DRM_MEMORYBARRIER()
#define DRM_MEMORYBARRIER()		mips_sync()
#elif defined(__powerpc__)
#define DRM_READMEMORYBARRIER()		DRM_MEMORYBARRIER() 
#define DRM_WRITEMEMORYBARRIER()	DRM_MEMORYBARRIER()
#define DRM_MEMORYBARRIER()		__asm __volatile("sync" : : : "memory");
#elif defined(__sparc64__)
#define DRM_READMEMORYBARRIER()		DRM_MEMORYBARRIER() 
#define DRM_WRITEMEMORYBARRIER()	DRM_MEMORYBARRIER()
#define DRM_MEMORYBARRIER()		membar(Sync)
#endif

#define smp_mb__before_atomic_dec()	DRM_MEMORYBARRIER()
#define smp_mb__after_atomic_dec()	DRM_MEMORYBARRIER()
#define smp_mb__before_atomic_inc()	DRM_MEMORYBARRIER()
#define smp_mb__after_atomic_inc()	DRM_MEMORYBARRIER()

#define mb()				DRM_MEMORYBARRIER()
#define rmb()				DRM_READMEMORYBARRIER()
#define wmb()				DRM_WRITEMEMORYBARRIER()
#define smp_rmb()			DRM_READMEMORYBARRIER()
#define smp_wmb()			DRM_WRITEMEMORYBARRIER()
#define mmiowb()			DRM_WRITEMEMORYBARRIER()

#define	DRM_COPY_TO_USER(user, kern, size)	copyout(kern, user, size)
#define	DRM_COPY_FROM_USER(kern, user, size)	copyin(user, kern, size)

#define DRM_UDELAY(udelay)	DELAY(udelay)

static inline bool
drm_can_sleep(void)
{
#if defined(__i386__) || defined(__amd64__)
	if (in_atomic() || in_dbg_master() || irqs_disabled())
#else
	if (in_dbg_master() || irqs_disabled())
#endif
		return false;
	return true;
}

#define DRM_WAIT_ON(ret, wq, timo, condition) do {			\
	ret = wait_event_interruptible_timeout(wq, condition, timo);	\
	if (ret == 0)							\
		ret = -EBUSY;						\
	if (ret > 0)							\
		ret = 0;						\
} while (0)

#define DRM_ERROR(fmt, arg...) \
	printf("error: [" DRM_NAME ":pid%d:%s] *ERROR* " fmt,		\
	    curproc->p_pid, __func__ , ## arg)


#ifdef DRMDEBUG
#define DRM_INFO(fmt, arg...)  printf("drm: " fmt, ## arg)
#define DRM_INFO_ONCE(fmt, arg...)  printf("drm: " fmt, ## arg)
#else
#define DRM_INFO(fmt, arg...) do { } while(/* CONSTCOND */ 0)
#define DRM_INFO_ONCE(fmt, arg...) do { } while(/* CONSTCOND */ 0)
#endif

#ifdef DRMDEBUG
#undef DRM_DEBUG
#define DRM_DEBUG(fmt, arg...) do {					\
	if (drm_debug_flag)						\
		printf("[" DRM_NAME ":pid%d:%s] " fmt, curproc->p_pid,	\
			__func__ , ## arg);				\
} while (0)
#else
#define DRM_DEBUG(fmt, arg...) do { } while(/* CONSTCOND */ 0)
#endif

#ifdef DRMDEBUG
#undef DRM_DEBUG_KMS
#define DRM_DEBUG_KMS(fmt, arg...) do {					\
	if (drm_debug_flag)						\
		printf("[" DRM_NAME ":pid%d:%s] " fmt, curproc->p_pid,	\
			__func__ , ## arg);				\
} while (0)
#else
#define DRM_DEBUG_KMS(fmt, arg...) do { } while(/* CONSTCOND */ 0)
#endif

#ifdef DRMDEBUG
#undef DRM_LOG_KMS
#define DRM_LOG_KMS(fmt, arg...) do {					\
	if (drm_debug_flag)						\
		printf(fmt, ## arg);					\
} while (0)
#else
#define DRM_LOG_KMS(fmt, arg...) do { } while(/* CONSTCOND */ 0)
#endif

#ifdef DRMDEBUG
#undef DRM_DEBUG_DRIVER
#define DRM_DEBUG_DRIVER(fmt, arg...) do {					\
	if (drm_debug_flag)						\
		printf("[" DRM_NAME ":pid%d:%s] " fmt, curproc->p_pid,	\
			__func__ , ## arg);				\
} while (0)
#else
#define DRM_DEBUG_DRIVER(fmt, arg...) do { } while(/* CONSTCOND */ 0)
#endif

struct drm_pcidev {
	uint16_t vendor;
	uint16_t device;
	uint16_t subvendor;
	uint16_t subdevice;
	uint32_t class;
	uint32_t class_mask;
	unsigned long driver_data;
};

struct drm_file;
struct drm_device;

/**
 * Ioctl function type.
 *
 * \param inode device inode.
 * \param file_priv DRM file private pointer.
 * \param cmd command.
 * \param arg argument.
 */
typedef int drm_ioctl_t(struct drm_device *dev, void *data,
			struct drm_file *file_priv);

typedef int drm_ioctl_compat_t(struct file *filp, unsigned int cmd,
			       unsigned long arg);

#define DRM_AUTH	0x1
#define DRM_MASTER	0x2
#define DRM_ROOT_ONLY	0x4
#define DRM_CONTROL_ALLOW 0x8
#define DRM_UNLOCKED	0x10
#define DRM_RENDER_ALLOW 0x20

struct drm_ioctl_desc {
	unsigned int cmd;
	int flags;
	drm_ioctl_t *func;
	unsigned int cmd_drv;
};

/**
 * Creates a driver or general drm_ioctl_desc array entry for the given
 * ioctl, for use by drm_ioctl().
 */

#define DRM_IOCTL_DEF_DRV(ioctl, _func, _flags)			\
	[DRM_IOCTL_NR(DRM_##ioctl)] = {.cmd = DRM_##ioctl, .func = _func, .flags = _flags, .cmd_drv = DRM_IOCTL_##ioctl}

struct drm_dmamem {
	bus_dmamap_t		map;
	caddr_t			kva;
	bus_size_t		size;
	int			nsegs;
	bus_dma_segment_t	segs[1];
};
typedef struct drm_dmamem drm_dma_handle_t;

struct drm_pending_event {
	struct drm_event *event;
	struct list_head link;
	struct drm_file *file_priv;
	pid_t pid; /* pid of requester, no guarantee it's valid by the time
		      we deliver the event, for tracing only */
	void (*destroy)(struct drm_pending_event *event);
};

/** File private data */
struct drm_file {
	unsigned always_authenticated :1;
	unsigned authenticated :1;
	unsigned is_master :1; /* this file private is a master for a minor */
	/* true when the client has asked us to expose stereo 3D mode flags */
	unsigned stereo_allowed :1;

	drm_magic_t magic;
	int minor;

	/** Mapping of mm object handles to object pointers. */
	struct idr object_idr;
	/** Lock for synchronization of access to object_idr. */
	spinlock_t table_lock;

	struct file *filp;
	void *driver_priv;

	/**
	 * fbs - List of framebuffers associated with this file.
	 *
	 * Protected by fbs_lock. Note that the fbs list holds a reference on
	 * the fb object to prevent it from untimely disappearing.
	 */
	struct list_head fbs;
	struct rwlock fbs_lock;

	wait_queue_head_t event_wait;
	struct list_head event_list;
	int event_space;

	struct selinfo rsel;
	SPLAY_ENTRY(drm_file) link;
};

struct drm_agp_head {
	struct agp_softc			*agpdev;
	const char				*chipset;
	TAILQ_HEAD(agp_memlist, drm_agp_mem)	 memory;
	struct agp_info				 info;
	unsigned long				 base;
	unsigned long				 mode;
	unsigned long				 page_mask;
	int					 acquired;
	int					 cant_use_aperture;
	int					 enabled;
   	int					 mtrr;
};

/* location of GART table */
#define DRM_ATI_GART_MAIN 1
#define DRM_ATI_GART_FB   2

#define DRM_ATI_GART_PCI  1
#define DRM_ATI_GART_PCIE 2
#define DRM_ATI_GART_IGP  3
#define DRM_ATI_GART_R600 4

/**
 * This structure defines the drm_mm memory object, which will be used by the
 * DRM for its buffer objects.
 */
struct drm_gem_object {
	/** Reference count of this object */
	struct kref refcount;

	/**
	 * handle_count - gem file_priv handle count of this object
	 *
	 * Each handle also holds a reference. Note that when the handle_count
	 * drops to 0 any global names (e.g. the id in the flink namespace) will
	 * be cleared.
	 *
	 * Protected by dev->object_name_lock.
	 * */
	unsigned handle_count;

	/** Related drm device */
	struct drm_device *dev;

	/** File representing the shmem storage */
	struct file *filp;

	/* Mapping info for this object */
	struct drm_vma_offset_node vma_node;

	/**
	 * Size of the object, in bytes.  Immutable over the object's
	 * lifetime.
	 */
	size_t size;

	/**
	 * Global name for this object, starts at 1. 0 means unnamed.
	 * Access is covered by the object_name_lock in the related drm_device
	 */
	int name;

	/**
	 * Memory domains. These monitor which caches contain read/write data
	 * related to the object. When transitioning from one set of domains
	 * to another, the driver is called to ensure that caches are suitably
	 * flushed and invalidated
	 */
	uint32_t read_domains;
	uint32_t write_domain;

	/**
	 * While validating an exec operation, the
	 * new read/write domain values are computed here.
	 * They will be transferred to the above values
	 * at the point that any cache flushing occurs
	 */
	uint32_t pending_read_domains;
	uint32_t pending_write_domain;

	struct uvm_object uobj;
	SPLAY_ENTRY(drm_gem_object) entry;
	struct uvm_object *uao;
};

/* Size of ringbuffer for vblank timestamps. Just double-buffer
 * in initial implementation.
 */
#define DRM_VBLANKTIME_RBSIZE 2

/* Flags and return codes for get_vblank_timestamp() driver function. */
#define DRM_CALLED_FROM_VBLIRQ 1
#define DRM_VBLANKTIME_SCANOUTPOS_METHOD (1 << 0)
#define DRM_VBLANKTIME_INVBL             (1 << 1)

/* get_scanout_position() return flags */
#define DRM_SCANOUTPOS_VALID        (1 << 0)
#define DRM_SCANOUTPOS_INVBL        (1 << 1)
#define DRM_SCANOUTPOS_ACCURATE     (1 << 2)

struct drm_driver_info {
	int	(*firstopen)(struct drm_device *);
	int	(*open)(struct drm_device *, struct drm_file *);
	void	(*close)(struct drm_device *, struct drm_file *);
	void	(*preclose)(struct drm_device *, struct drm_file *);
	void	(*postclose)(struct drm_device *, struct drm_file *);
	void	(*lastclose)(struct drm_device *);
	struct uvm_object *(*mmap)(struct drm_device *, voff_t, vsize_t);
	int	(*dma_ioctl)(struct drm_device *, struct drm_dma *,
		    struct drm_file *);
	int	(*irq_handler)(int, void *);
	void	(*irq_preinstall) (struct drm_device *);
	int	(*irq_install)(struct drm_device *);
	int	(*irq_postinstall) (struct drm_device *);
	void	(*irq_uninstall)(struct drm_device *);
	int	vblank_pipes;
	u_int32_t (*get_vblank_counter)(struct drm_device *, int);
	int	(*enable_vblank)(struct drm_device *, int);
	void	(*disable_vblank)(struct drm_device *, int);
	int	(*get_scanout_position)(struct drm_device *, int,
		    unsigned int, int *, int *, ktime_t *, ktime_t *);
	int	(*get_vblank_timestamp)(struct drm_device *, int, int *,
		    struct timeval *, unsigned);;

	/**
	 * Driver-specific constructor for drm_gem_objects, to set up
	 * obj->driver_private.
	 *
	 * Returns 0 on success.
	 */
	void (*gem_free_object) (struct drm_gem_object *obj);
	int (*gem_open_object) (struct drm_gem_object *, struct drm_file *);
	void (*gem_close_object) (struct drm_gem_object *, struct drm_file *);

	int	(*gem_fault)(struct drm_gem_object *, struct uvm_faultinfo *,
		    off_t, vaddr_t, vm_page_t *, int, int, vm_prot_t, int);

	int	(*dumb_create)(struct drm_file *file_priv,
		    struct drm_device *dev, struct drm_mode_create_dumb *args);
	int	(*dumb_map_offset)(struct drm_file *file_priv,
		    struct drm_device *dev, uint32_t handle, uint64_t *offset);
	int	(*dumb_destroy)(struct drm_file *file_priv,
		    struct drm_device *dev, uint32_t handle);

	size_t	gem_size;
	size_t	buf_priv_size;
	size_t	file_priv_size;

	int	major;
	int	minor;
	int	patchlevel;
	const char *name;		/* Simple driver name		   */
	const char *desc;		/* Longer driver name		   */
	const char *date;		/* Date of last major changes.	   */

	u32 driver_features;
	const struct drm_ioctl_desc *ioctls;
	int num_ioctls;

};

#include "drm_crtc.h"

/* mode specified on the command line */
struct drm_cmdline_mode {
	bool specified;
	bool refresh_specified;
	bool bpp_specified;
	int xres, yres;
	int bpp;
	int refresh;
	bool rb;
	bool interlace;
	bool cvt;
	bool margins;
	enum drm_connector_force force;
};

struct drm_pending_vblank_event {
	struct drm_pending_event base;
	int pipe;
	struct drm_event_vblank event;
};

struct drm_vblank_crtc {
	wait_queue_head_t queue;	/**< VBLANK wait queue */
	struct timeval time[DRM_VBLANKTIME_RBSIZE];	/**< timestamp of current count */
	atomic_t count;			/**< number of VBLANK interrupts */
	atomic_t refcount;		/* number of users of vblank interruptsper crtc */
	u32 last;			/* protected by dev->vbl_lock, used */
					/* for wraparound handling */
	u32 last_wait;			/* Last vblank seqno waited per CRTC */
	unsigned int inmodeset;		/* Display driver is setting mode */
	bool enabled;			/* so we don't call enable more than
					   once per disable */
};

/** 
 * DRM device functions structure
 */
struct drm_device {
	struct device	  device; /* softc is an extension of struct device */

	struct drm_driver_info *driver;

	struct pci_dev	_pdev;
	struct pci_dev	*pdev;
	u_int16_t	 pci_device;
	u_int16_t	 pci_vendor;

	pci_chipset_tag_t		 pc;
	pcitag_t	 		*bridgetag;

	bus_dma_tag_t			dmat;
	bus_space_tag_t			bst;

	struct mutex	quiesce_mtx;
	int		quiesce;
	int		quiesce_count;

	char		  *unique;	/* Unique identifier: e.g., busid  */
	int		  unique_len;	/* Length of unique field	   */
	
	int		  if_version;	/* Highest interface version set */
				/* Locks */
	struct rwlock	  struct_mutex;	/* protects everything else */

				/* Usage Counters */
	int		  open_count;	/* Outstanding files open	   */

				/* Authentication */
	SPLAY_HEAD(drm_file_tree, drm_file)	files;
	drm_magic_t	  magicid;

				/* Context support */
	int		  irq_enabled;	/* True if the irq handler is enabled */

	/** \name VBLANK IRQ support */
	/*@{ */

	/*
	 * At load time, disabling the vblank interrupt won't be allowed since
	 * old clients may not call the modeset ioctl and therefore misbehave.
	 * Once the modeset ioctl *has* been called though, we can safely
	 * disable them when unused.
	 */
	bool vblank_disable_allowed;

	/* array of size num_crtcs */
	struct drm_vblank_crtc *vblank;

	struct mutex vblank_time_lock;    /**< Protects vblank count and time updates during vblank enable/disable */
	struct mutex vbl_lock;
	struct timeout vblank_disable_timer;

	u32 max_vblank_count;           /**< size of vblank counter register */

	/**
	 * List of events
	 */
	struct list_head vblank_event_list;
	spinlock_t event_lock;

	/*@} */

	int			*vblank_enabled;
	int			*vblank_inmodeset;
	u32			*last_vblank_wait;

	int			 num_crtcs;

	pid_t			 buf_pgid;

	struct drm_agp_head	*agp;
	void			*dev_private;
	struct address_space	*dev_mapping;
	struct drm_local_map	*agp_buffer_map;

	struct drm_mode_config	 mode_config; /* Current mode config */

	/* GEM info */
	atomic_t		 obj_count;
	u_int			 obj_name;
	atomic_t		 obj_memory;
	struct pool				objpl;

	/** \name GEM information */
	/*@{ */
	struct rwlock object_name_lock;
	struct idr object_name_idr;
	struct drm_vma_offset_manager *vma_offset_manager;
	/*@} */
};

struct drm_attach_args {
	struct drm_driver_info		*driver;
	char				*busid;
	bus_dma_tag_t			 dmat;
	bus_space_tag_t			 bst;
	size_t				 busid_len;
	int				 is_agp;
	u_int16_t			 pci_vendor;
	u_int16_t			 pci_device;
	u_int16_t			 pci_subvendor;
	u_int16_t			 pci_subdevice;
	pci_chipset_tag_t		 pc;
	pcitag_t			 tag;
	pcitag_t			*bridgetag;
	int				 console;
};

#define DRMDEVCF_CONSOLE	0
#define drmdevcf_console	cf_loc[DRMDEVCF_CONSOLE]
/* spec'd as console? */
#define DRMDEVCF_CONSOLE_UNK	-1

extern int	drm_debug_flag;

/* Device setup support (drm_drv.c) */
int	drm_pciprobe(struct pci_attach_args *, const struct drm_pcidev * );
struct device	*drm_attach_pci(struct drm_driver_info *, 
		     struct pci_attach_args *, int, int, struct device *);
dev_type_ioctl(drmioctl);
dev_type_read(drmread);
dev_type_poll(drmpoll);
dev_type_open(drmopen);
dev_type_close(drmclose);
dev_type_mmap(drmmmap);
struct drm_local_map	*drm_getsarea(struct drm_device *);
struct drm_dmamem	*drm_dmamem_alloc(bus_dma_tag_t, bus_size_t, bus_size_t,
			     int, bus_size_t, int, int);
void			 drm_dmamem_free(bus_dma_tag_t, struct drm_dmamem *);

const struct drm_pcidev	*drm_find_description(int , int ,
			     const struct drm_pcidev *);
int	 drm_order(unsigned long);

/* File operations helpers (drm_fops.c) */
struct drm_file	*drm_find_file_by_minor(struct drm_device *, int);
struct drm_device *drm_get_device_from_kdev(dev_t);

/* Memory management support (drm_memory.c) */
void	*drm_alloc(size_t);
void	*drm_calloc(size_t, size_t);
void	*drm_realloc(void *, size_t, size_t);
void	 drm_free(void *);

#include "drm_mem_util.h"

/* XXX until we get PAT support */
#define drm_core_ioremap_wc drm_core_ioremap
void	drm_core_ioremap(struct drm_local_map *, struct drm_device *);
void	drm_core_ioremapfree(struct drm_local_map *, struct drm_device *);

int	drm_mtrr_add(unsigned long, size_t, int);
int	drm_mtrr_del(int, unsigned long, size_t, int);

/* IRQ support (drm_irq.c) */
int	drm_irq_install(struct drm_device *);
int	drm_irq_uninstall(struct drm_device *);
void	drm_vblank_cleanup(struct drm_device *);
u32	drm_get_last_vbltimestamp(struct drm_device *, int ,
				  struct timeval *, unsigned);
int	drm_vblank_init(struct drm_device *, int);
u_int32_t drm_vblank_count(struct drm_device *, int);
u_int32_t drm_vblank_count_and_time(struct drm_device *, int, struct timeval *);
int	drm_vblank_get(struct drm_device *, int);
void	drm_vblank_put(struct drm_device *, int);
void	drm_vblank_off(struct drm_device *, int);
void	drm_vblank_pre_modeset(struct drm_device *, int);
void	drm_vblank_post_modeset(struct drm_device *, int);
int	drm_modeset_ctl(struct drm_device *, void *, struct drm_file *);
bool	drm_handle_vblank(struct drm_device *, int);
void	drm_calc_timestamping_constants(struct drm_crtc *,
	    const struct drm_display_mode *);
extern int drm_calc_vbltimestamp_from_scanoutpos(struct drm_device *dev,
						 int crtc, int *max_error,
						 struct timeval *vblank_time,
						 unsigned flags,
						 const struct drm_crtc *refcrtc,
						 const struct drm_display_mode *mode);
bool	drm_mode_parse_command_line_for_connector(const char *,
	    struct drm_connector *, struct drm_cmdline_mode *);
struct drm_display_mode *
	 drm_mode_create_from_cmdline_mode(struct drm_device *,
	     struct drm_cmdline_mode *);

extern unsigned int drm_timestamp_monotonic;

/* AGP/PCI Express/GART support (drm_agpsupport.c) */
struct drm_agp_head *drm_agp_init(void);
void	drm_agp_takedown(struct drm_device *);
int	drm_agp_acquire(struct drm_device *);
int	drm_agp_release(struct drm_device *);
int	drm_agp_info(struct drm_device *, struct drm_agp_info *);
int	drm_agp_enable(struct drm_device *, struct drm_agp_mode);
void	*drm_agp_allocate_memory(size_t, u32);
int	drm_agp_free_memory(void *);
int	drm_agp_bind_memory(void *, off_t);
int	drm_agp_unbind_memory(void *);
int	drm_agp_alloc(struct drm_device *, struct drm_agp_buffer *);
int	drm_agp_free(struct drm_device *, struct drm_agp_buffer *);
int	drm_agp_bind(struct drm_device *, struct drm_agp_binding *);
int	drm_agp_unbind(struct drm_device *, struct drm_agp_binding *);

/* IRQ support (drm_irq.c) */
int	drm_control(struct drm_device *, void *, struct drm_file *);
int	drm_wait_vblank(struct drm_device *, void *, struct drm_file *);
int	drm_irq_by_busid(struct drm_device *, void *, struct drm_file *);
void	drm_send_vblank_event(struct drm_device *, int,
	    struct drm_pending_vblank_event *);

/* AGP/GART support (drm_agpsupport.c) */
int	drm_agp_acquire_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_release_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_enable_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_info_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_alloc_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_free_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_unbind_ioctl(struct drm_device *, void *, struct drm_file *);
int	drm_agp_bind_ioctl(struct drm_device *, void *, struct drm_file *);

static inline int
drm_sysfs_connector_add(struct drm_connector *connector)
{
	return 0;
}

static inline void
drm_sysfs_connector_remove(struct drm_connector *connector)
{
}

static inline void
drm_sysfs_hotplug_event(struct drm_device *dev)
{
}

/* Graphics Execution Manager library functions (drm_gem.c) */
int drm_gem_init(struct drm_device *dev);
void drm_gem_destroy(struct drm_device *dev);
void drm_gem_object_release(struct drm_gem_object *obj);
void drm_gem_object_free(struct kref *kref);
int drm_gem_object_init(struct drm_device *dev,
			struct drm_gem_object *obj, size_t size);
void drm_gem_private_object_init(struct drm_device *dev,
				 struct drm_gem_object *obj, size_t size);

int drm_gem_handle_create_tail(struct drm_file *file_priv,
			       struct drm_gem_object *obj,
			       u32 *handlep);
int drm_gem_handle_create(struct drm_file *file_priv,
			  struct drm_gem_object *obj,
			  u32 *handlep);
int drm_gem_handle_delete(struct drm_file *filp, u32 handle);

void drm_gem_free_mmap_offset(struct drm_gem_object *obj);
int drm_gem_create_mmap_offset(struct drm_gem_object *obj);

struct drm_gem_object *drm_gem_object_lookup(struct drm_device *dev,
					     struct drm_file *filp,
					     u32 handle);
struct drm_gem_object *drm_gem_object_find(struct drm_file *, u32);
int drm_gem_close_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int drm_gem_flink_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int drm_gem_open_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv);
void drm_gem_open(struct drm_device *dev, struct drm_file *file_private);
void drm_gem_release(struct drm_device *dev,struct drm_file *file_private);

static __inline void
drm_gem_object_reference(struct drm_gem_object *obj)
{
	kref_get(&obj->refcount);
}

static __inline void
drm_gem_object_unreference(struct drm_gem_object *obj)
{
	if (obj != NULL)
		kref_put(&obj->refcount, drm_gem_object_free);
}

static __inline void
drm_gem_object_unreference_unlocked(struct drm_gem_object *obj)
{
	if (obj && !atomic_add_unless(&obj->refcount.refcount, -1, 1)) {
		struct drm_device *dev = obj->dev;

		mutex_lock(&dev->struct_mutex);
		if (likely(atomic_dec_and_test(&obj->refcount.refcount)))
			drm_gem_object_free(&obj->refcount);
		mutex_unlock(&dev->struct_mutex);
	}
}

int drm_gem_dumb_destroy(struct drm_file *file,
			 struct drm_device *dev,
			 uint32_t handle);

static __inline__ int drm_core_check_feature(struct drm_device *dev,
					     int feature)
{
	return ((dev->driver->driver_features & feature) ? 1 : 0);
}

static inline int drm_dev_to_irq(struct drm_device *dev)
{
	return -1;
}

#define DRM_PCIE_SPEED_25 1
#define DRM_PCIE_SPEED_50 2
#define DRM_PCIE_SPEED_80 4

int	 drm_pcie_get_speed_cap_mask(struct drm_device *, u32 *);

#endif /* __KERNEL__ */
#endif /* _DRM_P_H_ */

/*	$OpenBSD: getentropy_linux.c,v 1.5 2014/06/21 02:08:34 deraadt Exp $	*/

/*
 * Copyright (c) 2014 Theo de Raadt <deraadt@openbsd.org>
 * Copyright (c) 2014 Bob Beck <beck@obtuse.com>
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

#define	_POSIX_C_SOURCE 199309L
#define	_GNU_SOURCE     1
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <openssl/sha.h>

#include <linux/random.h>
#include <sys/vfs.h>

#define REPEAT 5
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define HASHX(a, b) \
	do { \
		if ((a)) \
			HASHD(errno); \
		else \
			HASHD(b); \
	} while (0)

#define HASHD(xxx)	(SHA512_Update(&ctx, (char *)&(xxx), sizeof (xxx)))

int	getentropy(void *buf, size_t len);

static int gotdata(char *buf, size_t len);
static int getentropy_urandom(void *buf, size_t len);
static int getentropy_sysctl(void *buf, size_t len);
static int getentropy_fallback(void *buf, size_t len);

int
getentropy(void *buf, size_t len)
{
	int ret = -1;

	if (len > 256) {
		errno = EIO;
		return -1;
	}

	/*
	 * Try to get entropy with /dev/urandom
	 *
	 * This can fail if the process is inside a chroot or if file
	 * descriptors are exhausted.
	 */
	ret = getentropy_urandom(buf, len);
	if (ret != -1)
		return (ret);

#ifdef RANDOM_UUID
	/*
	 * Try to use sysctl CTL_KERN, KERN_RANDOM, RANDOM_UUID.
	 * sysctl is a failsafe API, so it guarantees a result.  This
	 * should work inside a chroot, or when file descriptors are
	 * exhuasted.
	 *
	 * However this can fail if the Linux kernel removes support
	 * for sysctl.  Starting in 2007, there have been efforts to
	 * deprecate the sysctl API/ABI, and push callers towards use
	 * of the chroot-unavailable fd-using /proc mechanism --
	 * essentially the same problems as /dev/urandom.
	 *
	 * Numerous setbacks have been encountered in their deprecation
	 * schedule, so as of June 2014 the kernel ABI still exists. The
	 * sysctl() stub in libc is missing on some systems.  There are
	 * also reports that some kernels spew messages to the console.
	 */
	ret = getentropy_sysctl(buf, len);
	if (ret != -1)
		return (ret);
#endif /* RANDOM_UUID */

	/*
	 * Entropy collection via /dev/urandom and sysctl have failed.
	 *
	 * No other API exists for collecting entropy.  See the large
	 * comment block above.
	 *
	 * We have very few options:
	 *     - Even syslog_r is unsafe to call at this low level, so
	 *	 there is no way to alert the user or program.
	 *     - Cannot call abort() because some systems have unsafe
	 *	 corefiles.
	 *     - Could raise(SIGKILL) resulting in silent program termination.
	 *     - Return EIO, to hint that arc4random's stir function
	 *       should raise(SIGKILL)
	 *     - Do the best under the circumstances....
	 *
	 * This code path exists to bring light to the issue that Linux
	 * does not provide a failsafe API for entropy collection.
	 *
	 * We hope this demonstrates that Linux should either retain their
	 * sysctl ABI, or consider providing a new failsafe API which
	 * works in a chroot or when file descriptors are exhausted.
	 */
#undef FAIL_HARD_WHEN_LINUX_DEPRECATES_SYSCTL
#ifdef FAIL_HARD_WHEN_LINUX_DEPRECATES_SYSCTL
	raise(SIGKILL);
#endif
	ret = getentropy_fallback(buf, len);
	if (ret != -1)
		return (ret);

	errno = EIO;
	return (ret);
}

/*
 * XXX Should be replaced with a proper entropy measure.
 */
static int
gotdata(char *buf, size_t len)
{
	char	any_set = 0;
	size_t	i;

	for (i = 0; i < len; ++i)
		any_set |= buf[i];
	if (any_set == 0)
		return -1;
	return 0;
}

static int
getentropy_urandom(void *buf, size_t len)
{
	struct stat st;
	size_t i;
	int fd, cnt;
	int save_errno = errno;

start:
#ifdef O_CLOEXEC
	fd = open("/dev/urandom", O_RDONLY|O_CLOEXEC, 0);
	if (fd == -1) {
		if (errno == EINTR)
			goto start;
		goto nodevrandom;
	}
#else
	fd = open("/dev/urandom", O_RDONLY, 0);
	if (fd == -1) {
		if (errno == EINTR)
			goto start;
		goto nodevrandom;
	}
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif

	/* Lightly verify that the device node looks sane */
	if (fstat(fd, &st) == -1 || !S_ISCHR(st.st_mode)) {
		close(fd);
		goto nodevrandom;
	}
	if (ioctl(fd, RNDGETENTCNT, &cnt) == -1) {
		close(fd);
		goto nodevrandom;
	}
	for (i = 0; i < len; ) {
		size_t wanted = len - i;
		ssize_t ret = read(fd, buf + i, wanted);

		if (ret == -1) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			close(fd);
			goto nodevrandom;
		}
		i += ret;
	}
	close(fd);
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return 0;		/* satisfied */
	}
nodevrandom:
	errno = EIO;
	return -1;
}

#ifdef RANDOM_UUID
static int
getentropy_sysctl(void *buf, size_t len)
{
	static const int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_UUID };
	size_t i, chunk;
	int save_errno = errno;

	for (i = 0; i < len; ) {
		chunk = min(len - i, 16);

		/* SYS__sysctl because some systems already removed sysctl() */
		struct __sysctl_args args = {
			.name = mib,
			.nlen = 3,
			.oldval = &buf[i],
			.oldlenp = &chunk,
		};
		if (syscall(SYS__sysctl, &args) != 0)
			goto sysctlfailed;
		i += chunk;
	}
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return (0);			/* satisfied */
	}
sysctlfailed:
	errno = EIO;
	return -1;
}
#endif /* RANDOM_UUID */

static int cl[] = {
	CLOCK_REALTIME,
#ifdef CLOCK_MONOTONIC
	CLOCK_MONOTONIC,
#endif
#ifdef CLOCK_MONOTONIC_RAW
	CLOCK_MONOTONIC_RAW,
#endif
#ifdef CLOCK_TAI
	CLOCK_TAI,
#endif
#ifdef CLOCK_VIRTUAL
	CLOCK_VIRTUAL,
#endif
#ifdef CLOCK_UPTIME
	CLOCK_UPTIME,
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
	CLOCK_PROCESS_CPUTIME_ID,
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
	CLOCK_THREAD_CPUTIME_ID,
#endif
};

static int
getentropy_fallback(void *buf, size_t len)
{
	uint8_t results[SHA512_DIGEST_LENGTH];
	int save_errno = errno, e, m, pgsiz = getpagesize(), repeat;
	static int counter;
	struct timespec ts;
	struct timeval tv;
	struct rusage ru;
	sigset_t sigset;
	struct stat st;
	SHA512_CTX ctx;
	pid_t pid;
	size_t i, ii;
	void *p;

	for (i = 0; i < len; ) {
		SHA512_Init(&ctx);
		for (repeat = 0; repeat < REPEAT; repeat++) {

			HASHX((e = gettimeofday(&tv, NULL)) == -1, tv);
			if (e != -1) {
				counter += (int)tv.tv_sec;
				counter += (int)tv.tv_usec;
			}

			for (ii = 0; ii < sizeof(cl)/sizeof(cl[0]); ii++)
				HASHX(clock_gettime(cl[ii], &ts) == -1, ts);

			HASHX((pid = getpid()) == -1, pid);
			HASHX((pid = getsid(pid)) == -1, pid);
			HASHX((pid = getppid()) == -1, pid);
			HASHX((pid = getpgid(0)) == -1, pid);
			HASHX((m = getpriority(0, 0)) == -1, m);

			ts.tv_sec = 0;
			ts.tv_nsec = 1;
			(void) nanosleep(&ts, NULL);

			HASHX(sigpending(&sigset) == -1, sigset);
			HASHX(sigprocmask(SIG_BLOCK, NULL, &sigset) == -1,
			    sigset);

			HASHD(main);	   /* an address in the main program */
			HASHD(getentropy); /* man address in this library */
			HASHD(printf);	   /* an address in libc */
			p = (void *)&p;
			HASHD(p); 	  /* an address on stack */
			p = (void *)&errno;
			HASHD(p); 	 /* the address of errno */

		if (i == 0) {
			struct sockaddr_storage ss;
			struct statvfs stvfs;
			struct termios tios;
			struct statfs stfs;
			socklen_t ssl;
			off_t off;

			/*
			 * Prime-sized mappings encourage fragmentation;
			 * thus exposing some address entropy.
			 */
			struct mm {
				size_t	npg;
				void	*p;
			} mm[] =	 {
				{ 17, MAP_FAILED }, { 3, MAP_FAILED },
				{ 11, MAP_FAILED }, { 2, MAP_FAILED },
				{ 5, MAP_FAILED }, { 3, MAP_FAILED },
				{ 7, MAP_FAILED }, { 1, MAP_FAILED },
				{ 57, MAP_FAILED }, { 3, MAP_FAILED },
				{ 131, MAP_FAILED }, { 1, MAP_FAILED },
			};

			for (m = 0; m < sizeof mm/sizeof(mm[0]); m++) {
				HASHX(mm[m].p = mmap(NULL, mm[m].npg * pgsiz,
				    PROT_READ|PROT_WRITE,
				    MAP_PRIVATE|MAP_ANON, -1, (off_t)0), mm[m].p);
				if (mm[m].p != MAP_FAILED) {
					char *mp;

					/* Touch some memory... */
					mp = mm[m].p;
					mp[counter % (mm[m].npg *
					    pgsiz - 1)] = 1;
					counter += (int)((long)(mm[m].p)
					    / pgsiz);
				}

				/* Check counters and times... */
				for (ii = 0; ii < sizeof(cl)/sizeof(cl[0]);
				    ii++) {
					HASHX((e = clock_gettime(cl[ii],
					    &ts)) == -1, ts);
					if (e != -1)
						counter += (int)ts.tv_nsec;
				}

				HASHX((e = getrusage(RUSAGE_SELF, &ru)) == -1,
				    ru);
				if (e != -1) {
					counter += (int)ru.ru_utime.tv_sec;
					counter += (int)ru.ru_utime.tv_usec;
				}
			}

			for (m = 0; m < sizeof mm/sizeof(mm[0]); m++) {
				if (mm[m].p != MAP_FAILED)
					munmap(mm[m].p, mm[m].npg * pgsiz);
				mm[m].p = MAP_FAILED;
			}

			HASHX(stat(".", &st) == -1, st);
			HASHX(statvfs(".", &stvfs) == -1, stvfs);
			HASHX(statfs(".", &stfs) == -1, stfs);

			HASHX(stat("/", &st) == -1, st);
			HASHX(statvfs("/", &stvfs) == -1, stvfs);
			HASHX(statfs("/", &stfs) == -1, stfs);

			HASHX((e = fstat(0, &st)) == -1, st);
			if (e == -1) {
				if (S_ISREG(st.st_mode) ||
				    S_ISFIFO(st.st_mode) ||
				    S_ISSOCK(st.st_mode)) {
					HASHX(fstatvfs(0, &stvfs) == -1,
					    stvfs);
					HASHX(fstatfs(0, &stfs) == -1, stfs);
					HASHX((off = lseek(0, (off_t)0,
					    SEEK_CUR)) < 0, off);
				}
				if (S_ISCHR(st.st_mode)) {
					HASHX(tcgetattr(0, &tios) == -1, tios);
				} else if (S_ISSOCK(st.st_mode)) {
					memset(&ss, 0, sizeof ss);
					ssl = sizeof(ss);
					HASHX(getpeername(0, (void *)&ss,
					    &ssl) == -1, ss);
				}
			}

			HASHX((e = getrusage(RUSAGE_CHILDREN, &ru)) == -1, ru);
			if (e != -1) {
				counter += (int)ru.ru_utime.tv_sec;
				counter += (int)ru.ru_utime.tv_usec;
			}
		} else {
			/* Subsequent hashes absorb previous result */
			HASHD(results);
		}

		HASHX((e = gettimeofday(&tv, NULL)) == -1, tv);
		if (e != -1) {
			counter += (int)tv.tv_sec;
			counter += (int)tv.tv_usec;
		}

		HASHD(counter);

		} /* repeat */
		SHA512_Final(results, &ctx);
		memcpy(buf + i, results, min(sizeof(results), len - i));
		i += min(sizeof(results), len - i);
	}
	memset(results, 0, sizeof results);
	if (gotdata(buf, len) == 0) {
		errno = save_errno;
		return 0;		/* satisfied */
	}
	errno = EIO;
	return -1;
}

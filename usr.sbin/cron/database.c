/*	$OpenBSD: database.c,v 1.33 2015/11/14 13:09:14 millert Exp $	*/

/* Copyright 1988,1990,1993,1994 by Paul Vixie
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <bitstring.h>		/* for structs.h */
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>		/* for structs.h */
#include <unistd.h>

#include "pathnames.h"
#include "macros.h"
#include "structs.h"
#include "funcs.h"

#define HASH(a,b) ((a)+(b))

static	void		process_crontab(int, const char *, const char *,
					struct stat *, cron_db *, cron_db *);

void
load_database(cron_db **db)
{
	struct stat statbuf, syscron_stat;
	cron_db *new_db, *old_db = *db;
	struct dirent *dp;
	DIR *dir;
	user *u;

	/* before we start loading any data, do a stat on _PATH_CRON_SPOOL
	 * so that if anything changes as of this moment (i.e., before we've
	 * cached any of the database), we'll see the changes next time.
	 */
	if (stat(_PATH_CRON_SPOOL, &statbuf) < 0) {
		syslog(LOG_ERR, "(CRON) STAT FAILED (%s)", _PATH_CRON_SPOOL);
		return;
	}

	/* track system crontab file
	 */
	if (stat(_PATH_SYS_CRONTAB, &syscron_stat) < 0)
		syscron_stat.st_mtime = 0;

	/* if spooldir's mtime has not changed, we don't need to fiddle with
	 * the database.
	 */
	if (old_db != NULL &&
	    old_db->mtime == HASH(statbuf.st_mtime, syscron_stat.st_mtime)) {
		return;
	}

	/* something's different.  make a new database, moving unchanged
	 * elements from the old database, reloading elements that have
	 * actually changed.  Whatever is left in the old database when
	 * we're done is chaff -- crontabs that disappeared.
	 */
	if ((new_db = malloc(sizeof(*new_db))) == NULL)
		return;
	new_db->mtime = HASH(statbuf.st_mtime, syscron_stat.st_mtime);
	TAILQ_INIT(&new_db->users);

	if (syscron_stat.st_mtime) {
		process_crontab(AT_FDCWD, "*system*", _PATH_SYS_CRONTAB,
				&syscron_stat, new_db, old_db);
	}

	/* we used to keep this dir open all the time, for the sake of
	 * efficiency.  however, we need to close it in every fork, and
	 * we fork a lot more often than the mtime of the dir changes.
	 */
	if (!(dir = opendir(_PATH_CRON_SPOOL))) {
		syslog(LOG_ERR, "(CRON) OPENDIR FAILED (%s)", _PATH_CRON_SPOOL);
		/* Restore system crontab entry as needed. */
		if (!TAILQ_EMPTY(&new_db->users) &&
		    (u = TAILQ_FIRST(&old_db->users))) {
			if (strcmp(u->name, "*system*") == 0) {
				TAILQ_REMOVE(&old_db->users, u, entries);
				free_user(u);
				TAILQ_INSERT_HEAD(&old_db->users,
				    TAILQ_FIRST(&new_db->users), entries);
			}
		}
		free(new_db);
		return;
	}

	while (NULL != (dp = readdir(dir))) {
		/* avoid file names beginning with ".".  this is good
		 * because we would otherwise waste two guaranteed calls
		 * to getpwnam() for . and .., and also because user names
		 * starting with a period are just too nasty to consider.
		 */
		if (dp->d_name[0] == '.')
			continue;

		process_crontab(dirfd(dir), dp->d_name, dp->d_name,
				&statbuf, new_db, old_db);
	}
	closedir(dir);

	/* if we don't do this, then when our children eventually call
	 * getpwnam() in do_command.c's child_process to verify MAILTO=,
	 * they will screw us up (and v-v).
	 */
	endpwent();

	/* whatever's left in the old database is now junk.
	 */
	if (old_db != NULL) {
		while ((u = TAILQ_FIRST(&old_db->users))) {
			TAILQ_REMOVE(&old_db->users, u, entries);
			free_user(u);
		}
		free(old_db);
	}

	/* overwrite the database control block with the new one.
	 */
	*db = new_db;
}

user *
find_user(cron_db *db, const char *name)
{
	user *u = NULL;

	if (db != NULL) {
		TAILQ_FOREACH(u, &db->users, entries) {
			if (strcmp(u->name, name) == 0)
				break;
		}
	}
	return (u);
}

static void
process_crontab(int dfd, const char *uname, const char *fname,
		struct stat *statbuf, cron_db *new_db, cron_db *old_db)
{
	struct passwd *pw = NULL;
	int crontab_fd = -1;
	user *u;

	/* Note: pw must remain NULL for system crontab (see below). */
	if (fname[0] != '/' && (pw = getpwnam(uname)) == NULL) {
		/* file doesn't have a user in passwd file.
		 */
		syslog(LOG_WARNING, "(%s) ORPHAN (no passwd entry)", uname);
		goto next_crontab;
	}

	crontab_fd = openat(dfd, fname, O_RDONLY|O_NONBLOCK|O_NOFOLLOW);
	if (crontab_fd < 0) {
		/* crontab not accessible?
		 */
		syslog(LOG_ERR, "(%s) CAN'T OPEN (%s)", uname, fname);
		goto next_crontab;
	}

	if (fstat(crontab_fd, statbuf) < 0) {
		syslog(LOG_ERR, "(%s) FSTAT FAILED (%s)", uname, fname);
		goto next_crontab;
	}
	if (!S_ISREG(statbuf->st_mode)) {
		syslog(LOG_WARNING, "(%s) NOT REGULAR (%s)", uname, fname);
		goto next_crontab;
	}
	if (pw != NULL) {
		/* Looser permissions on system crontab. */
		if ((statbuf->st_mode & 077) != 0) {
			syslog(LOG_WARNING, "(%s) BAD FILE MODE (%s)",
			    uname, fname);
			goto next_crontab;
		}
	}
	if (statbuf->st_uid != 0 && (pw == NULL ||
	    statbuf->st_uid != pw->pw_uid || strcmp(uname, pw->pw_name) != 0)) {
		syslog(LOG_WARNING, "(%s) WRONG FILE OWNER (%s)", uname, fname);
		goto next_crontab;
	}
	if (pw != NULL && statbuf->st_nlink != 1) {
		syslog(LOG_WARNING, "(%s) BAD LINK COUNT (%s)", uname, fname);
		goto next_crontab;
	}

	u = find_user(old_db, fname);
	if (u != NULL) {
		/* if crontab has not changed since we last read it
		 * in, then we can just use our existing entry.
		 */
		if (u->mtime == statbuf->st_mtime) {
			TAILQ_REMOVE(&old_db->users, u, entries);
			TAILQ_INSERT_TAIL(&new_db->users, u, entries);
			goto next_crontab;
		}

		/* before we fall through to the code that will reload
		 * the user, let's deallocate and unlink the user in
		 * the old database.  This is more a point of memory
		 * efficiency than anything else, since all leftover
		 * users will be deleted from the old database when
		 * we finish with the crontab...
		 */
		TAILQ_REMOVE(&old_db->users, u, entries);
		free_user(u);
		syslog(LOG_INFO, "(%s) RELOAD (%s)", uname, fname);
	}
	u = load_user(crontab_fd, pw, fname);
	if (u != NULL) {
		u->mtime = statbuf->st_mtime;
		TAILQ_INSERT_TAIL(&new_db->users, u, entries);
	}

 next_crontab:
	if (crontab_fd >= 0) {
		close(crontab_fd);
	}
}

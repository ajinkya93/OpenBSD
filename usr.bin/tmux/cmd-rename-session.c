/* $OpenBSD: cmd-rename-session.c,v 1.17 2015/12/13 14:32:38 nicm Exp $ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <stdlib.h>

#include "tmux.h"

/*
 * Change session name.
 */

enum cmd_retval	 cmd_rename_session_exec(struct cmd *, struct cmd_q *);

const struct cmd_entry cmd_rename_session_entry = {
	"rename-session", "rename",
	"t:", 1, 1,
	CMD_TARGET_SESSION_USAGE " new-name",
	CMD_SESSION_T,
	cmd_rename_session_exec
};

enum cmd_retval
cmd_rename_session_exec(struct cmd *self, struct cmd_q *cmdq)
{
	struct args	*args = self->args;
	struct session	*s = cmdq->state.tflag.s;
	const char	*newname;

	newname = args->argv[0];
	if (!session_check_name(newname)) {
		cmdq_error(cmdq, "bad session name: %s", newname);
		return (CMD_RETURN_ERROR);
	}
	if (session_find(newname) != NULL) {
		cmdq_error(cmdq, "duplicate session: %s", newname);
		return (CMD_RETURN_ERROR);
	}

	RB_REMOVE(sessions, &sessions, s);
	free(s->name);
	s->name = xstrdup(newname);
	RB_INSERT(sessions, &sessions, s);

	server_status_session(s);
	notify_session_renamed(s);

	return (CMD_RETURN_NORMAL);
}

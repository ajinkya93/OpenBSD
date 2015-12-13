/* $OpenBSD: cmd-unbind-key.c,v 1.24 2015/12/13 21:53:57 nicm Exp $ */

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
 * Unbind key from command.
 */

enum cmd_retval	cmd_unbind_key_exec(struct cmd *, struct cmd_q *);
enum cmd_retval	cmd_unbind_key_mode_table(struct cmd *, struct cmd_q *,
		    key_code);

const struct cmd_entry cmd_unbind_key_entry = {
	.name = "unbind-key",
	.alias = "unbind",

	.args = { "acnt:T:", 0, 1 },
	.usage = "[-acn] [-t mode-table] [-T key-table] key",

	.flags = 0,
	.exec = cmd_unbind_key_exec
};

enum cmd_retval
cmd_unbind_key_exec(struct cmd *self, struct cmd_q *cmdq)
{
	struct args	*args = self->args;
	key_code	 key;
	const char	*tablename;

	if (!args_has(args, 'a')) {
		if (args->argc != 1) {
			cmdq_error(cmdq, "missing key");
			return (CMD_RETURN_ERROR);
		}
		key = key_string_lookup_string(args->argv[0]);
		if (key == KEYC_NONE || key == KEYC_UNKNOWN) {
			cmdq_error(cmdq, "unknown key: %s", args->argv[0]);
			return (CMD_RETURN_ERROR);
		}
	} else {
		if (args->argc != 0) {
			cmdq_error(cmdq, "key given with -a");
			return (CMD_RETURN_ERROR);
		}
		key = KEYC_UNKNOWN;
	}

	if (args_has(args, 't'))
		return (cmd_unbind_key_mode_table(self, cmdq, key));

	if (key == KEYC_UNKNOWN) {
		tablename = args_get(args, 'T');
		if (tablename == NULL) {
			key_bindings_remove_table("root");
			key_bindings_remove_table("prefix");
			return (CMD_RETURN_NORMAL);
		}
		if (key_bindings_get_table(tablename, 0) == NULL) {
			cmdq_error(cmdq, "table %s doesn't exist", tablename);
			return (CMD_RETURN_ERROR);
		}
		key_bindings_remove_table(tablename);
		return (CMD_RETURN_NORMAL);
	}

	if (args_has(args, 'T')) {
		tablename = args_get(args, 'T');
		if (key_bindings_get_table(tablename, 0) == NULL) {
			cmdq_error(cmdq, "table %s doesn't exist", tablename);
			return (CMD_RETURN_ERROR);
		}
	} else if (args_has(args, 'n'))
		tablename = "root";
	else
		tablename = "prefix";
	key_bindings_remove(tablename, key);
	return (CMD_RETURN_NORMAL);
}

enum cmd_retval
cmd_unbind_key_mode_table(struct cmd *self, struct cmd_q *cmdq, key_code key)
{
	struct args			*args = self->args;
	const char			*tablename;
	const struct mode_key_table	*mtab;
	struct mode_key_binding		*mbind, mtmp;

	tablename = args_get(args, 't');
	if ((mtab = mode_key_findtable(tablename)) == NULL) {
		cmdq_error(cmdq, "unknown key table: %s", tablename);
		return (CMD_RETURN_ERROR);
	}

	if (key == KEYC_UNKNOWN) {
		while (!RB_EMPTY(mtab->tree)) {
			mbind = RB_ROOT(mtab->tree);
			RB_REMOVE(mode_key_tree, mtab->tree, mbind);
			free(mbind);
		}
		return (CMD_RETURN_NORMAL);
	}

	mtmp.key = key;
	mtmp.mode = !!args_has(args, 'c');
	if ((mbind = RB_FIND(mode_key_tree, mtab->tree, &mtmp)) != NULL) {
		RB_REMOVE(mode_key_tree, mtab->tree, mbind);
		free(mbind);
	}
	return (CMD_RETURN_NORMAL);
}

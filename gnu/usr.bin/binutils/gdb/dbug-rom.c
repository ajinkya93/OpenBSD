/* Remote debugging interface to dBUG ROM monitor for GDB, the GNU debugger.
   Copyright 1996 Free Software Foundation, Inc.

   Written by Stan Shebs of Cygnus Support.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* dBUG is a monitor supplied on various Motorola boards, including
   m68k, ColdFire, and PowerPC-based designs.  The code here assumes
   the ColdFire, and (as of 9/25/96) has only been tested with a
   ColdFire IDP board.  */

#include "defs.h"
#include "gdbcore.h"
#include "target.h"
#include "monitor.h"
#include "serial.h"

static void dbug_open PARAMS ((char *args, int from_tty));

static void
dbug_supply_register (regname, regnamelen, val, vallen)
     char *regname;
     int regnamelen;
     char *val;
     int vallen;
{
  int regno;

  if (regnamelen != 2)
    return;

  switch (regname[0])
    {
    case 'S':
      if (regname[1] != 'R')
	return;
      regno = PS_REGNUM;
      break;
    case 'P':
      if (regname[1] != 'C')
	return;
      regno = PC_REGNUM;
      break;
    case 'D':
      if (regname[1] < '0' || regname[1] > '7')
	return;
      regno = regname[1] - '0' + D0_REGNUM;
      break;
    case 'A':
      if (regname[1] < '0' || regname[1] > '7')
	return;
      regno = regname[1] - '0' + A0_REGNUM;
      break;
    default:
      return;
    }

  monitor_supply_register (regno, val);
}

/* This array of registers needs to match the indexes used by GDB. The
   whole reason this exists is because the various ROM monitors use
   different names than GDB does, and don't support all the registers
   either. So, typing "info reg sp" becomes an "A7". */

static char *dbug_regnames[NUM_REGS] =
{
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
  "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
  "SR", "PC"
  /* no float registers */
};

static struct target_ops dbug_ops;

static char *dbug_inits[] = {"\r", NULL};

static struct monitor_ops dbug_cmds =
{
  MO_CLR_BREAK_USES_ADDR | MO_GETMEM_NEEDS_RANGE | MO_FILL_USES_ADDR,
  dbug_inits,			/* Init strings */
  "go\r",			/* continue command */
  "step\r",			/* single step */
  NULL,				/* interrupt command */
  "br %x\r",			/* set a breakpoint */
  "br -c %x\r",			/* clear a breakpoint */
  "br -c\r",			/* clear all breakpoints */
  "bf.b %x %x %x",		/* fill (start end val) */
  {
    "mm.b %x %x\r",		/* setmem.cmdb (addr, value) */
    "mm.w %x %x\r",		/* setmem.cmdw (addr, value) */
    "mm.l %x %x\r",		/* setmem.cmdl (addr, value) */
    NULL,			/* setmem.cmdll (addr, value) */
    NULL,			/* setmem.resp_delim */
    NULL,			/* setmem.term */
    NULL			/* setmem.term_cmd */
  },
  {
    "md.b %x %x\r",		/* getmem.cmdb (addr, addr2) */
    "md.w %x %x\r",		/* getmem.cmdw (addr, addr2) */
    "md.l %x %x\r",		/* getmem.cmdl (addr, addr2) */
    NULL,			/* getmem.cmdll (addr, addr2) */
    ":",			/* getmem.resp_delim */
    NULL,			/* getmem.term */
    NULL			/* getmem.term_cmd */
  },
  {
    "rm %s %x\r",		/* setreg.cmd (name, value) */
    NULL,			/* setreg.resp_delim */
    NULL,			/* setreg.term */
    NULL			/* setreg.term_cmd */
  },
  {
    "rd %s\r",			/* getreg.cmd (name) */
    ":",			/* getreg.resp_delim */
    NULL,			/* getreg.term */
    NULL			/* getreg.term_cmd */
  },
  "rd\r",			/* dump_registers */
  "\\(\\w+\\) +:\\([0-9a-fA-F]+\\b\\)", /* register_pattern */
  dbug_supply_register,		/* supply_register */
  NULL,				/* load_routine (defaults to SRECs) */
  "dl\r",			/* download command */
  "\n",				/* load response */
  "dBUG>",			/* monitor command prompt */
  "\r",				/* end-of-line terminator */
  NULL,				/* optional command terminator */
  &dbug_ops,			/* target operations */
  SERIAL_1_STOPBITS,		/* number of stop bits */
  dbug_regnames,		/* registers names */
  MONITOR_OPS_MAGIC		/* magic */
  };

static void
dbug_open(args, from_tty)
     char *args;
     int from_tty;
{
  monitor_open (args, &dbug_cmds, from_tty);
}

void
_initialize_dbug_rom ()
{
  init_monitor_ops (&dbug_ops);

  dbug_ops.to_shortname = "dbug";
  dbug_ops.to_longname = "dBUG monitor";
  dbug_ops.to_doc = "Debug via the dBUG monitor.\n\
Specify the serial device it is connected to (e.g. /dev/ttya).";
  dbug_ops.to_open = dbug_open;

  add_target (&dbug_ops);
}

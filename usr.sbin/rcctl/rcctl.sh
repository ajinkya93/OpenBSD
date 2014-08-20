#!/bin/sh
#
# Copyright (c) 2014 Antoine Jacoutot <ajacoutot@openbsd.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# get local functions from rc.subr(8)
FUNCS_ONLY=1
. /etc/rc.d/rc.subr
_rc_parse_conf

usage()
{
	_rc_err "usage: ${0##*/} enable|disable|status|action [service | daemon [flags [...]]]"
}

needs_root()
{
	if [ "$(id -u)" -ne 0 ]; then
		_rc_err "${0##*/} ${1:+$1: }need root privileges"
	fi
}

rcconf_edit_begin()
{
	_TMP_RCCONF=$(mktemp -p /etc -t rc.conf.local.XXXXXXXXXX) || exit 1
	if [ -f /etc/rc.conf.local ]; then
		cp -p /etc/rc.conf.local ${_TMP_RCCONF} || exit 1
	else
		touch /etc/rc.conf.local || exit 1
	fi
}

rcconf_edit_end()
{
	mv ${_TMP_RCCONF} /etc/rc.conf.local || exit 1
	if [ ! -s /etc/rc.conf.local ]; then
		rm /etc/rc.conf.local || exit 1
	fi
}

svc_default_enabled()
{
	local _ret=1
	local _svc=$1
	[ -n "${_svc}" ] || return

	# get _defaults_ values only
	_rc_parse_conf /etc/rc.conf

	svc_is_enabled ${_svc} && _ret=0

	# reparse _all_ values
	_rc_parse_conf

	return ${_ret}
}

svc_get_all()
{
	local _i

	(
		ls -A /etc/rc.d | grep -v rc.subr
		for _i in ${_allowed_keys[@]}; do
			echo ${_i}
		done | grep -Ev '(nfs_server|savecore_flag|amd_master|pf_rules|ipsec_rules|shlib_dirs|pkg_scripts)'
	) | sort
}

svc_get_flags()
{
	local daemon_flags
	local _svc=$1
	[ -n "${_svc}" ] || return

	if svc_is_special ${_svc}; then
		echo "$(eval echo \${${_svc}})"
	else
		daemon_flags="$(eval echo \${${_svc}_flags})"
		# reset flags for pkg daemon to match the output of base/special svc
		if ! svc_is_base ${_svc}; then
			if ! echo ${pkg_scripts} | grep -qw ${_svc}; then
				daemon_flags="NO"
			fi
		fi
		if [ -z "${daemon_flags}" ]; then
			eval $(grep '^daemon_flags=' /etc/rc.d/${_svc})
		fi
		echo ${daemon_flags}
	fi
}

svc_get_status()
{
	local _svc=$1

	if [ -n "${_svc}" ]; then
		svc_get_flags ${_svc} | sed '/^$/d'
		svc_is_enabled ${_svc}
	else
		for _i in $(svc_get_all); do
			printf "%18s" ${_i}
			svc_is_enabled ${_i} && echo -n "(enabled)" || echo -n "(disabled)"
			echo -n "\tflags="
			svc_get_flags ${_i}
		done
	fi
}

svc_is_avail()
{
	local _i

	for _i in $(svc_get_all); do
		if [ ${_i} = "$1" ]; then
			return 0
		fi
	done
	return 1
}

svc_is_base()
{
	local _svc=$1
	[ -n "${_svc}" ] || return

	grep "^start_daemon " /etc/rc | cut -d ' ' -f2- | grep -qw ${_svc}
}

svc_is_enabled()
{
	local _flags _i
	local _svc=$1
	[ -n "${_svc}" ] || return

	[[ "$(svc_get_flags ${_svc})" != "NO" ]]
}

svc_is_special()
{
	local _svc=$1
	[ -n "${_svc}" ] || return

	echo ${_allowed_keys[@]} | grep -qw ${_svc}
}

append_to_pkg_scripts()
{
	local _svc=$1
	[ -n "${_svc}" ] || return

	svc_is_enabled ${_svc} && return

	rcconf_edit_begin
	if [ -n "${pkg_scripts}" ]; then
		grep -v "^pkg_scripts.*=" /etc/rc.conf.local >${_TMP_RCCONF}
		echo pkg_scripts="${pkg_scripts} ${_svc}" >>${_TMP_RCCONF}
	else
		echo pkg_scripts="${_svc}" >>${_TMP_RCCONF}
	fi
	rcconf_edit_end
}

rm_from_pkg_scripts()
{
	local _i _pkg_scripts
	local _svc=$1
	[ -n "${_svc}" ] || return

	[ -z "${pkg_scripts}" ] && return

	for _i in ${pkg_scripts}; do
		if [ ${_i} != ${_svc} ]; then
			_pkg_scripts="${_pkg_scripts} ${_i}"
		fi
	done
	pkg_scripts=$(printf ' %s' ${_pkg_scripts})
	pkg_scripts=${_pkg_scripts## }

	rcconf_edit_begin
	grep -v "^pkg_scripts.*=" /etc/rc.conf.local >${_TMP_RCCONF}
	if [ -n "${pkg_scripts}" ]; then
		echo pkg_scripts="${pkg_scripts}" >>${_TMP_RCCONF}
	fi
	rcconf_edit_end
}

add_flags()
{
	local _flags _numargs=$#
	local _svc=$2
	[ -n "${_svc}" ] || return

	if [ -n "$3" ]; then
		# there is an early check for this; but this function is fed with $*
		[ "$3" = "flags" ] || return
		if [ -n "$4" ]; then
			while [ "${_numargs}" -ge 4 ]
			do
				eval _flags=\"\$${_numargs} ${_flags}\"
				let _numargs--
			done
			set -A _flags -- ${_flags}
		fi
	elif svc_is_enabled ${_svc}; then
		# svc is already enabled and flags are not (re)set: return unless
		# svc is enabled by default and our current flags are not empty
		# (if they are, we drop the default "svc_flags=" further down)
		if ! svc_default_enabled ${_svc}; then
			return
		elif [ -n "$(svc_get_flags ${_svc})" ]; then
			return
		fi
	fi

	# special var
	if svc_is_special ${_svc}; then
		rcconf_edit_begin
		grep -v "^${_svc}.*=" /etc/rc.conf.local >${_TMP_RCCONF}
		if ! svc_default_enabled ${_svc}; then
			echo "${_svc}=YES" >>${_TMP_RCCONF}
		fi
		rcconf_edit_end
		return
	fi

	# base-system script
	if svc_is_base ${_svc}; then
		rcconf_edit_begin
		grep -v "^${_svc}_flags.*=" /etc/rc.conf.local >${_TMP_RCCONF}
		if ! svc_default_enabled ${_svc} || test "${#_flags[*]}" -gt 0; then
			echo ${_svc}_flags=${_flags[@]} >>${_TMP_RCCONF}
		fi
		rcconf_edit_end
		return
	fi

	# pkg script
	rcconf_edit_begin
	grep -v "^${_svc}_flags.*=" /etc/rc.conf.local >${_TMP_RCCONF}
	if [ "${#_flags[*]}" -gt 0 ]; then
		echo ${_svc}_flags=${_flags[@]} >>${_TMP_RCCONF}
	fi
	rcconf_edit_end
}

rm_flags()
{
	local _svc=$1
	[ -n "${_svc}" ] || return

	rcconf_edit_begin
	if svc_is_special ${_svc}; then
		grep -v "^${_svc}.*=" /etc/rc.conf.local >${_TMP_RCCONF}
		if svc_default_enabled ${_svc}; then
			echo "${_svc}=NO" >>${_TMP_RCCONF}
		fi
	else
		grep -v "^${_svc}_flags.*=" /etc/rc.conf.local >${_TMP_RCCONF}
		if svc_default_enabled ${_svc}; then
			echo "${_svc}_flags=NO" >>${_TMP_RCCONF}
		fi
	fi
	rcconf_edit_end
}

action=$1
svc=$2
flag=$3
flags=$*

if [ $# -gt 0 ]; then
	if [ -n "$svc" ]; then
		if ! svc_is_avail $svc; then
			_rc_err "service $svc does not exist"
		fi
	elif [ "$action" != "status" ]; then
		usage
	fi
	if [ -n "$flag" ]; then
		if [ "$flag" = "flags" ]; then
			if [ "$action" != "enable" ]; then
				_rc_err "\"flags\" can only be set with \"enable\""
			fi
			if svc_is_special $svc; then
				_rc_err "\"$svc\" is a special variable, cannot set \"flags\""
			fi
		else
			usage
		fi
	fi
	case $action in
		disable)
			needs_root $svc
			if ! svc_is_base $svc && ! svc_is_special $svc; then
				rm_from_pkg_scripts $svc
			fi
			rm_flags $svc
			;;
		enable)
			needs_root $action
			add_flags $flags
			if ! svc_is_base $svc && ! svc_is_special $svc; then
				append_to_pkg_scripts $svc
			fi
			;;
		status)
			svc_get_status $svc
			;;
		start|stop|restart|reload|check)
			if svc_is_special $svc; then
				_rc_err "\"$svc\" is a special variable, no rc.d(8) script"
			fi
			/etc/rc.d/$svc $action
			;;
		*)
			usage
			;;
	esac
else
	usage
fi

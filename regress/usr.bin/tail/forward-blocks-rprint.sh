#!/bin/sh
#
# $OpenBSD: forward-blocks-rprint.sh,v 1.1 2015/11/24 04:04:19 tedu Exp $

# test if tail grep the correct number of blocks from a file.

DIR=$(mktemp -d)
echo DIR=${DIR}

NAME=${0##*/}
OUT=${DIR}/${NAME%%.sh}.out
i=0
while [ ${i} -lt 512 ]; do
	echo ${i} >> ${DIR}/bar
	i=$((i+1))
done

tail -r -b +1 ${DIR}/bar > ${OUT}

diff -u ${OUT} ${0%%.sh}.out || exit 1

# cleanup if okay
rm -Rf ${DIR}

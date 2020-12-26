#!/usr/bin/env bash

revision=0
[[ $STATIC = 1 ]] || STATIC=0

[[ -s revision.sh ]] && . revision.sh

if [[ -d .svn ]] && which svn >/dev/null 2>&1
then
    revision="$( svn info . | awk '$1=="Revision:" {print $2}' )"
    if which svnversion >/dev/null 2>&1
    then
	rev="$(svnversion|sed 's/.*://')"
	(( ${revision//[!0-9]/} < ${rev//[!0-9]/} )) && revision=$rev
    fi
    echo "revision=$revision" > revision.sh
fi

revision_num="${revision//[!0-9]/}"
revision_next=$revision_num
[[ $revision = $revision_num ]] || let revision_next++

tim=($(date '+%s %Y-%m-%d %T %Y'))

have_md5=0
[[ -r /usr/include/openssl/md5.h ]] && have_md5=1

have_sha=0
[[ -r /usr/include/openssl/sha.h ]] && have_sha=1

#---

if [[ $M32 = 1 ]]
then
    force_m32=1
    libdir=lib
    xflags="-m32"
    defines=-DFORCE_M32=1
else
    force_m32=0
    libdir=lib64
    xflags=
    defines=
fi

[[ -r /usr/include/bits/fcntl.h ]] \
	&& grep -qw fallocate /usr/include/bits/fcntl.h \
	&& defines="$defines -DHAVE_FALLOCATE=1"

[[ -r /usr/include/fcntl.h ]] \
	&& grep -qw posix_fallocate /usr/include/fcntl.h \
	&& defines="$defines -DHAVE_POSIX_FALLOCATE=1"

gcc $xflags system.c -o system.tmp && ./system.tmp >Makefile.setup
#rm -f system.tmp
SYSTEM=$(awk '$1=="SYSTEM" {print $3}' Makefile.setup)
#echo "SYSTEM=|$SYSTEM|" >&2

libpng="-lpng -lz"
case "$SYSTEM" in
    mac)
	[[ -s /usr/local/lib/libpng.a ]] && libpng="/usr/local/lib/libpng.a -lz"
	;;

    i386)
	[[ -s /usr/lib/libpng.a ]] && libpng="/usr/lib/libpng.a -lz"
	;;

    x86_64)
	[[ -s /usr/lib64/libpng.a ]] && libpng="/usr/lib64/libpng.a -lz"
	;;
esac    

#----- have_pcre

have_pcre=0
pcre_a=
[[ -r /usr/include/pcre.h || -r /usr/local/include/pcre.h ]] && have_pcre=1

if (( STATIC && have_pcre ))
then
    have_pcre=0
    for lib in /usr/local/$libdir /usr/$libdir /$libdir
    do
	if [[ -s $lib/libpcre.a ]]
	then
	    have_pcre=1
	    pcre_a=$lib/libpcre.a
	    break;
	fi
    done
fi
have_pcre=0

#--------------------------------------------------

INSTALL_PATH=/usr/local

if [[ -d $INSTALL_PATH/bin ]]
then
    HAVE_INSTBIN=1
    INSTBIN=$INSTALL_PATH/bin
else
    HAVE_INSTBIN=0
    INSTBIN=/tmp
fi

if [[ -d $INSTALL_PATH/bin32 ]]
then
    HAVE_INSTBIN_32=1
    INSTBIN_32=$INSTALL_PATH/bin32
else
    HAVE_INSTBIN_32=0
    INSTBIN_32=/tmp
fi

if [[ -d $INSTALL_PATH/bin64 ]]
then
    HAVE_INSTBIN_64=1
    INSTBIN_64=$INSTALL_PATH/bin64
elif [[ -d $INSTALL_PATH/bin-x86_64 ]]
then
    HAVE_INSTBIN_64=1
    INSTBIN_64=$INSTALL_PATH/bin-x86_64
else
    HAVE_INSTBIN_64=0
    INSTBIN_64=/tmp
fi

HAVE_WORK=0
[[ -d ./work ]] && HAVE_WORK=1

HAVE_WORK_SRC=0
[[ -d ./work/src ]] && HAVE_WORK_SRC=1

[[ $NOWIIMM = 1 || $NO_WIIMM = 1 ]] && HAVE_WORK=0 HAVE_WORK_SRC=0

HAVE_XSRC=0
[[ -h xsrc ]] && rm xsrc
if [[ -d xsrc ]]
then
    HAVE_XSRC=1
    cat <<- __EOT__ >xsrc/xversion.tmp
	#ifndef SZS_X_VERSION_H
	#define SZS_X_VERSION_H 1

	#define XSRC_BASE_VERSION "$1"
	#define XSRC_VERSION_NUM "$2"
	#define XSRC_VERSION_INT $3

	#endif // SZS_X_VERSION_H
	__EOT__
	diff -N xsrc/xversion.tmp xsrc/xversion.h >/dev/null \
		|| mv xsrc/xversion.tmp xsrc/xversion.h
elif [[ ! -a xsrc && -d ../wiimms-szs-tools/xsrc ]]
then
    ln -s ../wiimms-szs-tools/xsrc ./xsrc
    HAVE_XSRC=1
fi

#--------------------------------------------------

cat <<- ---EOT--- >>Makefile.setup
	
	REVISION	:= $revision
	REVISION_NUM	:= $revision_num
	REVISION_NEXT	:= $revision_next
	BINTIME		:= ${tim[0]}
	DATE		:= ${tim[1]}
	TIME		:= ${tim[2]}
	YEAR		:= ${tim[3]}

	FORCE_M32	:= $force_m32
	HAVE_MD5	:= $have_md5
	HAVE_SHA	:= $have_sha
	STATIC		:= $STATIC
	XFLAGS		+= $xflags
	DEFINES1	:= $defines -DDEBUG_ASSERT=1
	LIBPNG		:= $libpng

	HAVE_PCRE	:= $have_pcre
	PCRE_A		:= $pcre_a

	HAVE_INSTBIN	:= $HAVE_INSTBIN
	HAVE_INSTBIN_32	:= $HAVE_INSTBIN_32
	HAVE_INSTBIN_64	:= $HAVE_INSTBIN_64

	INSTALL_PATH	:= $INSTALL_PATH
	INSTBIN		:= $INSTBIN
	INSTBIN_32	:= $INSTBIN_32
	INSTBIN_64	:= $INSTBIN_64

	HAVE_WORK	:= $HAVE_WORK
	HAVE_WORK_SRC	:= $HAVE_WORK_SRC
	HAVE_XSRC	:= $HAVE_XSRC

	---EOT---


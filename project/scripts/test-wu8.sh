#!/usr/bin/env bash
# (c) Wiimm, 2012-10-26

export LC_ALL=POSIX
szsmode=0
logfile=

SZS=wszst
[[ -x ./wszst ]] && SZS=./wszst
SZS0="$SZS -q"
SZS="$SZS0"
echo "SZS=$SZS"

DIFF="diff -q"
echo "DIFF=$DIFF"

HAVE_DCC=0
which -q dcc &>/dev/null && HAVE_DCC=1

function error()
{
    echo "!! Test failed: $*" >&2
    exit 1
}

let count=0
for src in "$@"
do
    if [[ $src = -n || $src = --new ]]
    then
	echo ">> enabled: --NEW"
	SZS="$SZS0 --NEW"
	continue
    fi

    if [[ $src = -o || $src = --old ]]
    then
	echo ">> enabled: --OLD"
	SZS="$SZS0 --OLD"
	continue
    fi

    if [[ $src = -s || $src = --szs ]]
    then
	echo ">> enabled: SZS mode"
	szsmode=1
	continue
    fi

    if [[ $src = -l || $src = --log ]]
    then
	szsmode=1
	logfile=./wu8-size.log
	echo ">> SZS mode enabled, log to: $logfile"
	{
	    echo
	    date +'--- %F %T ---'
	    echo
	} >>"$logfile"
	continue
    fi

    let count++
    printf "%5u. %s\n" $count "$src"
    temp="$(mktemp -ut test-wu8-XXXXXXXX).tmp" || exit 1
    #echo "TEMP=$temp"

    $SZS decompress	"$src"		-D "$temp/11.u8"	|| error 11 decompress
    $SZS norm		"$temp/11.u8"				|| error 11 norm
    $SZS norm --wu8	"$temp/11.u8"	-D "$temp/12.wu8"	|| error 12 decode WU8
    $SZS norm --u8	"$temp/12.wu8"	-D "$temp/13.u8"	|| error 13 encode WU8
    $DIFF "$temp/11.u8" "$temp/13.u8"				|| error raw diff 11.u8 13.u8

    if ((szsmode))
    then
	$SZS norm	"$src"		-D "$temp/21.szs"	|| error 11 norm
	$SZS norm --wu8	"$temp/21.szs"	-D "$temp/22.wu8"	|| error 12 decode WU8
	$SZS norm --u8	"$temp/22.wu8"	-D "$temp/23.szs"	|| error 13 encode WU8
	$DIFF "$temp/21.szs" "$temp/23.szs"			|| error raw diff 21.szs 23.szs
	if [[ $logfile != "" ]]
	then
	    s1=$(stat -c%s "$temp/21.szs")
	    s2=$(stat -c%s "$temp/22.wu8")
	    if ((HAVE_DCC))
	    then
		printf "%8u %8u %6.2f%%  %s\n" $s1 $s2 $(dcc "100.0*$s2/$s1") "$src" >>"$logfile"
	    else
		printf "%8u %8u %3d%%  %s\n" $s1 $s2 $((100*s2/s1)) "$src" >>"$logfile"
	    fi
	fi
    fi

    rm -rf "$temp"
done
echo "**OK**"


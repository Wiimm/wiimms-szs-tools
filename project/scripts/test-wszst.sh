#!/usr/bin/env bash
# (c) Wiimm, 2012-11-11

export LC_ALL=POSIX

SZS=wszst
[[ -x ./wszst ]] && SZS=./wszst
SZS0="$SZS -q"
SZS="$SZS0"
echo "SZS=$SZS"

BMG=wbmgt
[[ -x ./wbmgt ]] && BMG=./wbmgt
BMG0="$BMG -q"
BMG="$BMG0"
echo "BMG=$BMG"

if [[ $1 = -i || $1 = --ignore ]]
then
    DIFF="diff -q -x wszst-setup.txt -x .string-pool.* -x .*.d"
    echo "DIFF=$DIFF"
    printf "\n!!! WARNING: '.string-pool.*' excluded !!!\n\n"
    shift
else
    DIFF="diff -q -x wszst-setup.txt"
    echo "DIFF=$DIFF"
fi

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
	BMG="$BMG0 --NEW"
	continue
    fi

    if [[ $src = -o || $src = --old ]]
    then
	echo ">> enabled: --OLD"
	SZS="$SZS0 --OLD"
	BMG="$BMG0 --OLD"
	continue
    fi

    let count++
    printf "%5u. %s\n" $count "$src"
    temp="$(mktemp -ut test-wszst-XXXXXXXX).tmp" || exit 1
    #echo "TEMP=$temp"

    $SZS decom	"$src"		-D "$temp/11.u8"		|| error 11 decompress
    $SZS compr	"$temp/11.u8"	-D "$temp/12.szs"		|| error 12 compress
    $SZS decom	"$temp/12.szs"	-D "$temp/13.u8"		|| error 13 decompress
    $DIFF "$temp/11.u8" "$temp/13.u8"				|| error raw diff 11.u8 13.u8

    $SZS extr	"$src"		-D "$temp/21.d"			|| error 21 extract
    $SZS extr	"$temp/11.u8"	-D "$temp/22.d"			|| error 22 extract
    $DIFF -r "$temp/21.d" "$temp/22.d"				|| error raw diff 21.d 22.d
    
    $SZS create	"$temp/22.d"	-D "$temp/23.u8" --noc		|| error 23 create
    $SZS diff	"$src"		   "$temp/23.u8"		|| error file diff src 23
    $SZS extr	"$temp/23.u8"	-D "$temp/24.d"			|| error 24 extract
    $DIFF -r "$temp/21.d" "$temp/24.d"				|| error raw diff 21 24

    $SZS extr	"$src"		-D "$temp/31.d"  -a --no-check	|| error 31 extract --all
    $SZS create	"$temp/31.d"	-D "$temp/32.u8" --noc		|| error 32 create
    $SZS extr	"$temp/32.u8"	-D "$temp/33.d"  -a --no-check	|| error 33 extract --all
    $DIFF -r "$temp/31.d" "$temp/33.d"				|| error raw diff 31 33

    $SZS compr	"$src"		-D "$temp/41.wbz" --wbz		|| error 41 to wbz
    $SZS compr	"$temp/41.wbz"	-D "$temp/42.szs" --szs		|| error 42 to szs
    $SZS diff	"$src"		   "$temp/42.szs"		|| error file diff 41 42

    find "$temp/24.d" -name '*.bmg' | while read f
    do
	[[ -f $f ]] || continue
	echo "	- ${f/*.d}"

	$BMG encode "$f"	-d "$f.1.bmg"			|| error encode "$f"
	$DIFF "$f" "$f.1.bmg"					|| error raw diff src 1

	$BMG decode "$f"	-d "$f.2.txt"			|| error decode "$f"
	$BMG encode "$f.2.txt"	-d "$f.3.bmg"			|| error encode "$f"
	$DIFF "$f" "$f.3.bmg"					|| error raw diff src 3

    done || exit 1
    rm -rf "$temp"

done
echo "**OK**"


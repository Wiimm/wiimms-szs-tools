#!/bin/bash
# made by Wiimm, 2012-10-06

plusmode=0
testmode=
kclmode=0

#------------------------------------------------------------------------------

function trim()
{
    echo "$1" | sed -r 's/[[:blank:]]+/ /; s/^ //; s/ $//'
}

#------------------------------------------------------------------------------

function add_destname()
{
    local dest="$1"
    local plus="$2"
    local add="$3"

    if [[ ! $dest =~ $add ]]
    then

	if ((plus))
	then
	    #--- add '+'

	    local s1="${dest%%(*}"
	    local s2="${dest#*(}"
	    if [[ $s1 != $s2 ]]
	    then
		s2="($s2"
	    else
		s1="${dest%%\[*}"
		s2="${dest#*\[}"
		if [[ $s1 != $s2 ]]
		then
		    s2="[$s2"
		else
		    s2=""
		fi
	    fi
	    s1=$( trim "$s1" )
	    s2=$( trim "$s2" )
	    dest="$s1+ $s2"
	fi


	#--- add '[+$add]'

	local s1="${dest%%]*}"
	local s2="${dest#*]}"
	if [[ $s1 != $s2 ]]
	then
	    dest="$s1,+$add]$s2"
	else
	    dest="$dest [+$add]"
	fi
    fi
    echo "$dest"
}

#------------------------------------------------------------------------------

while (($#))
do
    src="$1"
    shift

    if [[ $src = + ]]
    then
	plusmode=1
	continue
    fi

    if [[ $src = ++ ]]
    then
	plusmode=1
	kclmode=1
	continue
    fi

    if [[ $src = -t || $src == --test ]]
    then
	testmode=--test
	continue
    fi

    if [[ $src = -k || $src == --kcl ]]
    then
	kclmode=1
	continue
    fi

    if [[ ! -f $src ]]
    then
	printf "%s: Not found: %s\n" "${0##*/}" "$src" >&2
	continue
    fi

    #--- extract ext

    ext="${src##*.}"
    if [[ $ext = $src ]]
    then
	dest="$src"
	ext=""
    else
	dest="${src%.*}"
	ext=".$ext"
    fi

    d=${dest%_d}
    if [[ "$d" != "$dest" ]]
    then
	dest="$d"
	ext="_d${ext}"
    fi

    dest="$(add_destname "$dest" 1 minimap)"


    #--- trim dest

    dest="$( trim "$dest" )$ext"

    if ((plusmode))
    then
	idx="${dest:0:2}"
	idx="${idx/[^0-9]/}"
	let idx=10#0$idx
	if (( idx % 10 < 4 ))
	then
	    let idx=idx+1
	else
	    let idx=idx+7
	fi
	dest="$idx ${dest:3}"
	#echo "idx=$idx"
    fi

    temp="$dest.temp"
    echo "DEST=|$dest|"


    #--- create temp, fix minimap, rename track
    
    ln -f "$src" "$temp"
    wszst -q minimap -r --auto "$temp" $testmode
    if ! diff -q "$src" "$temp" >/dev/null
    then
	((plusmode)) || rm -f "$src"
	mv "$temp" "$dest"
	if ((kclmode))
	then
	    kcldest="$(add_destname "$dest" 0 kcl)"
	    echo "KCLDEST=|$kcldest|"
	    cp -p "$dest" "$kcldest"
	    wszst norm "$kcldest" --kcl new,drop
	    if diff -q "$dest" "$kcldest" >/dev/null
	    then
		rm "$kcldest"
	    else
		rm "$dest"
	    fi
	fi
    fi

    rm -f "$temp"

done


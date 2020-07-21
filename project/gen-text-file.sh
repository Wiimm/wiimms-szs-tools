#!/usr/bin/env bash

dir="$1"
shift

if [[ ! -d $dir ]]
then
    echo "Directory not found: $dir" >&2
    exit 1
fi

function gen_text()
{
    local varname="$1"
    local src="$2"

    printf "\nconst char %s[] =\n{\n" "$varname"
    if [[ $varname =~ _bz2$ ]]
    then
	# '-f%z' for MAC, '-c%s' for GNU
	s=$( stat -c%s "$src" 2>/dev/null || stat -f%z "$src" )
	x=$( printf "%08x" "$s" )
	let c=s/100000+1
	((c>9)) && c=9
	printf "  0x%s, 0x%s, 0x%s, 0x%s, // uncompressed size = $s, compression level = $c\n" \
		"${x:0:2}" "${x:2:2}" "${x:4:2}" "${x:6:2}"
	bzip2 -$c < "$src" | xxd -i -c16
    elif [[ $varname =~ _cr$ ]]
    then
	grep -v '^~' "$src" \
	    | sed 's/\\/\\\\/g; s/"/\\"/g; s/^/  "/; s/#FF#/#\\f/; s/$/\\r\\n"/'
    else
	grep -v '^~' "$src" \
	    | sed 's/\\/\\\\/g; s/"/\\"/g; s/^/  "/; s/#FF#/#\\f/; s/$/\\n"/'
    fi
    printf "};\n\n"
}

for src in "$@"
do
    src="${src##*/}"
    name="${src%.*}"
    name="text_${name//[-.]/_}"
    cname="$( echo "SZS_${name}_INC" | awk '{print toupper($0)}' )"
    #echo "$name : $src -> $dest"

    {
	printf "\n#ifndef %s\n#define %s 1\n" "$cname" "$cname"

	if [[ -f "$dir/$src" ]]
	then
	    gen_text "$name" "$dir/$src"
	    echo "$src: $dir/$src" >"$src.d"
	elif [[ -d "$dir/$src" ]]
	then
	    printf '%s:' "$src" >"$src.d"
	    for inc in "$dir/$src"/*.inc
	    do
		name="${inc##*/}"
		name="${name%.*}"
		name="text_${name//[-.]/_}"
		gen_text "$name" "$inc"
		printf ' \\\n\t%s' "$inc" >>"$src.d"
	    done
	    printf '\n' >>"$src.d"
	else
	    printf '\n!! Text not found: %s\n' "$dir/$src" >&2
	    rm -f "$src"
	    exit 1
	fi

	printf "#endif // %s\n\n" "$cname"

    } >"$src"

done


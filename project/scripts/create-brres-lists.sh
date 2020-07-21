#!/usr/bin/env bash

#
#------------------------------------------------------------------------------
echo "* setup"

listdir=./_lists
[[ $1 = -t || $1 == --test ]] || rm -rf  "$listdir"
mkdir -p "$listdir"

filelist="$listdir/brres.flist"
listing="$listdir/brres.list"

subfilelist="$listdir/sub-file.list"
submodelist="$listdir/sub-mode.list"
subcountlist="$listdir/sub-count.list"

#subfilesummary="$listdir/sub-summary.txt"

subfileflist="${subfilelist%.list}.flist"
submodeflist="${submodelist%.list}.flist"
subcountflist="${subcountlist%.list}.flist"

info="$listdir/INFO.txt"

#
#------------------------------------------------------------------------------
echo "* create file list : $filelist"

if [[ -s $filelist ]]
then
    echo "   -> skipped"
else
    find . -type f -name '*.brres' | sort -f >"$filelist"
fi

#
#------------------------------------------------------------------------------
echo "* create brres listing : $listing"

if [[ -s $listing ]]
then
    echo "   -> skipped"
else
    wszst lll "@$filelist" >"$listing"
fi

#
#------------------------------------------------------------------------------
echo "* create sub file listing : $subfilelist"

awk='
    BEGIN { file = "" }
    END   { print "" }

    $1 == "*" && $2 == "Files" {
	file = ""
	idx = index($0,":");
	if ( idx > 0 )
	{
	    file = substr($0,idx+1);
	    print "";
	}
	#printf("FILE: %s\n",file);
    }
    
    file != "" && $5+0 > 0 && $4 != "...." {
	printf("%s %2u %s %s\n",$4,$5,file,$6);
    }
    
'

if [[ -s $subfilelist ]]
then
    echo "   -> skipped"
else
    awk "$awk" "$listing" >"$subfilelist"
fi

#
#------------------------------------------------------------------------------
echo "* create sub file listing : $subfileflist"

awk_flist='
    $4 == "" { print; next }
	     { printf("%s %2u %s\n",$1,$2,$3) }
'

if [[ -s $subfileflist ]]
then
    echo "   -> skipped"
else
    awk "$awk_flist" "$subfilelist" | uniq >"$subfileflist"
fi

#
#------------------------------------------------------------------------------
echo "* create sub mode listing : $submodelist"

awk='
    BEGIN { mode = "" }
    END   { print "" }

    $3 != "" {
	newmode = $1 "." $2;
	if ( mode != newmode )
	{
	    if ( mode == "" )
		print "";
	    else
		print "\f";
	    mode = newmode;
	}
	printf("%s %2u %s %s\n",$1,$2,$3,$4);
    }
'

if [[ -s $submodelist ]]
then
    echo "   -> skipped"
else
    sort -f "$subfilelist" | awk "$awk" >"$submodelist"
fi

#
#------------------------------------------------------------------------------
echo "* create sub mode listing : $submodeflist"

if [[ -s $submodeflist ]]
then
    echo "   -> skipped"
else
    awk "$awk_flist" "$submodelist" | uniq >"$submodeflist"
fi

#
#------------------------------------------------------------------------------
echo "* create sub count listings : $subcountlist $subcountflist"

awk1='
    $3 != "" { printf("%s %2u\n",$1,$2); }
'

awk2='
    BEGIN { mode = "" }
    END   { print "" }

    {
	if ( mode != $2 )
	{
	    mode = $2;
	    print "";
	}
	printf("%6u*  %s %2u\n",$1,$2,$3);
    }
'

if [[ -s $subcountlist && -s $subcountflist ]]
then
    echo "   -> skipped"
else
    awk "$awk1" "$submodelist"  | uniq -c | awk "$awk2" >"$subcountlist"
    awk "$awk1" "$submodeflist" | uniq -c | awk "$awk2" >"$subcountflist"
fi

#
#------------------------------------------------------------------------------
echo "* create info file : $info"

cat <<- --EOT-- >"$info"

OVERVIEW:
---------

The list files can be divided into 2 groups:
  *.list files are the base listings with all BRRES sub files.
  *.flist are reduced listings:
       No more than one line for each BRRES and category.

All files are machine readable, just ignore empty lines (inserted for human
readability) and lines containing only 1 formfeed (ASCII 12, inserted for
fast finding of sections). Columns are separated by multiple spaces (like
awk and other tools does).



FILES:
------

${filelist##*/}
    File list of all explored BRRES files.

${listing##*/}
    A sub listing of all explored BRRES files.
    This listing was made with: wszst lll @${filelist##*/}

${subfilelist##*/}
    A list of sub files with for space separated columns:
        MAGIC  VERSION  BRRES_FILE  SUB_FILE
    The file is sorted by the BRRES_FILE column.

${subfileflist##*/}
    Same as '${subfilelist##*/}', but reduced to BRRES file level.
    A list of sub files with for space separated columns:
        MAGIC  VERSION  BRRES_FILE
    The file is sorted by the BRRES_FILE column.

${submodelist##*/}
    Same as '${subfilelist##*/}', but sorted by columns MAGIC and VERSION.

${submodeflist##*/}
    Same as '${subfileflist##*/}', but sorted by columns MAGIC and VERSION.
    Same as '${submodelist##*/}', but reduced to BRRES file level.

${subcountlist##*/}
    This is a summary of '${subfilelist##*/}' or '${submodelist##*/}'.
    Instead of filenames it contains exist counters.
        COUNT*  MAGIC  VERSION
    
${subcountflist##*/}
    This is a summary of '${subfileflist##*/}' or '${submodeflist##*/}'.
    Instead of filenames it contains exist counters.
        COUNT*  MAGIC  VERSION



STATISTICS:
-----------

--EOT--

{
    printf "SZS files:"
    sed 's+\.szs\.d/.*$++' "$filelist" | uniq | wc -l

    printf "BRRES files:"
    cat "$filelist" | wc -l

    printf "BRRES sub files:"
    grep -v '^$' "$submodelist" | wc -l

} | awk -F: '{printf("%6u  %s\n",$2,$1)}' >>"$info"


cat <<- --EOT-- >>"$info"



Wiimm, $(date '+%F %T')

--EOT--

#
#------------------------------------------------------------------------------
echo "done"


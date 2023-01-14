
/***************************************************************************
 *                         _______ _______ _______                         *
 *                        |  ___  |____   |  ___  |                        *
 *                        | |   |_|    / /| |   |_|                        *
 *                        | |_____    / / | |_____                         *
 *                        |_____  |  / /  |_____  |                        *
 *                         _    | | / /    _    | |                        *
 *                        | |___| |/ /____| |___| |                        *
 *                        |_______|_______|_______|                        *
 *                                                                         *
 *                            Wiimms SZS Tools                             *
 *                          https://szs.wiimm.de/                          *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This file is part of the SZS project.                                 *
 *   Visit https://szs.wiimm.de/ for project details and sources.          *
 *                                                                         *
 *   Copyright (c) 2011-2023 by Dirk Clemens <wiimm@wiimm.de>              *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   See file gpl-2.0.txt or http://www.gnu.org/licenses/gpl-2.0.txt       *
 *                                                                         *
 ***************************************************************************/

//
/////////////////////////////////////////////////////////////////////////////
/////////////			    wszst			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wszst", 0,
		"wszst [option]... command [option|parameter|@file]...",
		WSZST_LONG " :"
		" List, compare, (de-)compress, extract and create"
		" " CARCHIVES_AND " archives."
		" Additonally, RARC archive can be extracted."
		" It is also a wrapper to all other tools." },

  //
  //---------- list of all wszst commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION|V",
		    "wszst VERSION [ignored]...",
		"Print program name and version and exit." },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wszst HELP [--width=wd] OPT|OPTION [option]...\n"
		    "wszst HELP [--width=wd] CMD|COMMAND ...\n"
		    "wszst HELP [--width=wd] command ...\n"
		    "wszst HELP [--width=wd] ...",
		"Print help for commands and options."
		    "\f" // <-------- separator: before built-in help & below full help
		"Print help for commands and options."
		"\n "
		" The first variant (with keyword @OPT@ or @OPTION@)"
		" prints a list of all global and command specific options."
		" If at least one option is set,"
		" only info about the declared options are printed."
		" The second variant (with keyword @CMD@ or @COMMAND@)"
		" prints an overview about all commands."
		" The third variant (with a valid command name)"
		" prints details about the declared command."
		" The fourth variant prints an overview about all commands"
		" and all global options." },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wszst CONFIG [options]...",
		CONFIG_CMD
		" The command can also export the settings for scripts"
		" or can create or update configuration files." },

  { T_DEF_CMD,	"INSTALL",	"INSTALL",
		    "wszst INSTALL [file]...",
		"Install files to the shared directory."
		" Use {wszst CONFIG} to list related pathes." },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wszst ARGTEST ...",
		"This debug command accepts all kinds of parameters"
		" and prints one line for each parameter."
		" All tools know the ARGTEST command." },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wszst EXPAND ...",
		"This debug command accepts (like ARGTEST) all kinds of parameters"
		" and prints one line for each parameter."
		" After that, the parameter is treated as a filename with wildcards"
		" and all matching files are searched."
		" The special parameters '+h' and '/h' enable or disable the search"
		" for hidden directories and files (files beginning with a point)."
		" All tools know the EXPAND command." },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wszst TEST [options]... [ignored]...",
		"This debugging command accepts all options and parameters"
		" and prints a list of the internal option values."
		" The command is designed to test the option parameters."
		" The non option parameters are completely ignored." },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wszst COLORS",
		"Ignore all parameters and print colored text for testing." },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wszst ERROR [error_code]",
		"Translate an exit code to a message name."
		" If no exit code is entered, print a table with all error messages."
		" All tools know the ERROR command." },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wszst FILETYPE file...",
		"Scan the header of the entered files and print file type"
		" and path for each file as list."
		" All tools know the FILETYPE command." },

  { T_DEF_CMD,	"UI_CHECK",	"UI-CHECK|UC|UICHECK",
		    "wszst UI-CHECK file...",
		"Classify SZS files from directory .../Scene/UI/."
		" For the analysis, the filename is ignored and only the content is examined."
		"" WILDCARDS_SORT },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wszst FILEATTRIB [keyword]...",
		"Print a table with attributes for all known file types."
		" If at least one keyword is entered, only matching file types"
		" are printed."
		" A keyword is either a file type (like 'BRRES' or 'BMG')"
		" or one of the printed keywords (like 'ARCHIVE' or 'EXTRACT')."
		" All tools know the FILEATTRIB command." },

  { T_DEF_CMD,	"BRSUB",	"BRSUB",
		    "wszst BRSUB [file]...",
		" If no file is defined, print a table with known BRRES"
		" sub files consisting of"
		" file format, version, number of sections and warn level."
		" Otherwise analyse the version numbers of each BRRES sub file"
		" of each source and print warnings about invalid versions." },

  { T_DEF_CMD,	"SYMBOLS",	"SYMBOLS",
		    "wszst SYMBOLS",
		"Print predefined symbols of text parser to standard output."
		" Many tools know the SYMBOLS command." },

  { T_DEF_CMD,	"FUNCTIONS",	"FUNCTIONS",
		    "wszst FUNCTIONS [key]...",
		"Print a list with all parser functions."
		" If one or more keys entered,"
		" then only functions that match at least one key are printed."
		" Many tools know the FUNCTIONS command." },

  { T_DEF_CMD,	"CALCULATE",	"CALCULATE",
		    "wszst CALCULATE expression...",
		"Calculate each expression and print the result to standard output."
		" Many tools know the CALCULATE command." },

  { T_DEF_CMD,	"MATRIX",	"MATRIX",
		    "wszst MATRIX [patch-option]... [vector]...",
		"Use the transformation options as base and calculate and print"
		" the related vectors and the transformation matrices."
		" Then do a forward and backward transformation for each @vector@"
		" parameter using the matrices and print the results."
		    "\f" // <-------- separator: before built-in help & below full help
		"Use the transformation options as base and calculate and print"
		" the related vectors and the transformation matrices."
		" Then do a forward and backward transformation for each @vector@"
		" parameter using the matrices and print the results."
		" The transformations are done with 4 different methods"
		" to confirm the algorithms."
		" Only the main results and results, which differ, are printed."
		"\n"
		" Normally, the exit status is @OK@ (null)."
		" If at least one old transformation returns different results"
		" for the different algorithms,"
		" then status @DIFFER@ (not null) is returned." },

  { T_DEF_CMD,	"FLOAT",	"FLOAT",
		    "wszst FLOAT number...",
		"This commands helps to convert hex values into floating point numbers"
		" and vice versa."
		" For each non empty parameter, a line with 4 bytes hex and a float,"
		" 8 bytes hex and a double float, and the parameter itself is printed."
		" If a parameter begins with 'x' or '0x', it is a hex string,"
		" otherwise it is a floating point number."
		"\n "
		" The special parameters \"+\" and \"-\" mean:"
		" Increment or decrement the previous value by the smallest"
		" possible value. Use different increments for floats and doubles."
		"\n "
		" All conversions are done using big endian." },

  { T_DEF_CMD,	"VR_CALC",	"VR-CALC|VRCALC",
		    "wszst VR-CALC delta...",
		" Calculate a VR increment for each given delta value"
		" (@=VR_winner-VR_loser@). Print results as table." },

  { T_DEF_CMD,	"VR_RACE",	"VR-RACE|VRRACE",
		    "wszst VR-RACE vr1 vr2...",
		"Print a table with VR calculations for a race."
		" Each @VR@ is the current rating of a player."
		" The vr-list is well ordered: "
		" The first VR is for the winner and the last VR for the looser."
		" Up to 30 VR values will be accpeted."
		" Each VR can be preceeded by @\"NUM*\"@ to define a repeat factor." },

  { T_DEF_CMD,	"AUTOADD",	"AUTOADD",
		    "wszst AUTOADD [source]...",
		"Manage the auto-add library."
		" Primarily add files to the main auto-add archive for future"
		" use of option {--auto-add} or to convert WU8 files."
		" If @SOURCE@ is a directory, all standard track and arena"
		" file names are searched for the missed auto-add files."
		" Without source files, the status of the auto-add archive is printed."
		"\n "
		" Try {wszst AUTOADD ./path_to/Race/Course/}"
		" to add all supported files." },

  { T_DEF_CMD,	"TRACKS",	"TRACKS",
		    "wszst TRACKS [selection_mask]",
		"Print a list of all known track SZS files of Mario Kart"
		" internal directory './Race/Course'."
		" These files are 32 standard track files and 10 arena files."
		" The first column shows an unique ID of exact 3 upper case"
		" characters or digits. For '_d' files, a 'd' is appended."
		" The second column shows the internal file name."
		" Use BMG files to print real names as third column." },

  { T_DEF_CMD,	"SCANCACHE",	"SCANCACHE",
		    "wszst SCANCACHE directory\n"
		    "wszst SCANCACHE --cache directory",
		"Scan the cache directory for SZS files and re-build the"
		" checksum table. Store the result as file 'cache-content.txt'"
		" in the same directory."
		" The file is used by option {--cache} to optimize file access." },

  { H_DEF_CMD,	"EXPORT",	"_EXPORT",
		    "wszst _EXPORT key...",
		"This hidden command exports some definitions to standard output."
		" Use keyword @HELP@ to get a list of possible keywords." },

  { H_DEF_CMD,	"SIZEOF",	"SIZEOF",
		    "wszst SIZEOF key...",
		"This hidden command prints a list with the size of"
		" data structures and variables." },

  { H_DEF_CMD,	"CODE",		"_CODE",
		    "wszst _CODE [files]",
		"All bytes of all input files are xor'ed with 0xDC and copied to stdout."
		" If no source file is given, standard input is read." },

  { H_DEF_CMD,	"RECODE",	"_RECODE",
		    "wszst _RECODE mode [files]",
		"All bytes of all input files are interpreted as 1 file and re-coded."
		" @MODE@ defines the re-code mode."
		" If no source file is given, standard input is read." },

  { H_DEF_CMD,	"SUBFILE",	"_SUBFILE",
		    "wszst _SUBFILE [files]",
		"Test sub-file system." },

  { H_DEF_CMD,	"TESTNORM",	"_NORM",
		    "wszst _NORM key...",
		"This hidden command tests the iteration of norming." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"LIST",		"LIST|LS",
		    "wszst LIST [source]...",
		"List all sub files of " ARCHIVES_AND " archives."
		" If listing a directory, then a virtual SZS file"
		" is created (like CREATE does it) and listed."
		" " WILDCARDS_SORT },

  { T_DEF_CMD,	"LIST_L",	"LIST-L|LL|LISTL",
		    "wszst LIST-L [source]...",
		"List all sub files with size."
		" 'LIST-L' and 'LL' are short cuts for {LIST --long}."
		" " WILDCARDS_SORT },

  { T_DEF_CMD,	"LIST_LL",	"LIST-LL|LLL|LISTLL",
		    "wszst LIST-LL [source]...",
		"List all sub files with offset and size."
		" 'LIST-LL' and 'LLL' are short cuts for {LIST --long --long}."
		" " WILDCARDS_SORT },

  { T_DEF_CMD,	"LIST_LLL",	"LIST-LLL|LLLL|LISTLLL",
		    "wszst LIST-LLL [source]...",
		"List all sub files with offset and size in debugging mode."
		" 'LIST-LLL' and 'LLLL' are short cuts"
		" for {LIST --long --long --long}."
		" " WILDCARDS_SORT },

  { T_DEF_CMD,	"LIST_A",	"LIST-A|LA|LISTA",
		    "wszst LIST-A [source]...",
		"List recursive all sub files with size."
		" 'LIST-A' and 'LA' are short cuts for {LIST --long --all --all}."
		" " WILDCARDS_SORT },

  { T_DEF_CMD,	"LIST_LA",	"LIST-LA|LLA|LISTLA",
		    "wszst LIST-LA [source]...",
		"List recursive all sub files with offset and size."
		" 'LIST-LA' and 'LLA' are short cuts for {LIST --long --long --all --all}."
		" " WILDCARDS_SORT },

  { T_DEF_CMD,	"NAME_REF",	"NAME-REF|NAMEREF",
		    "wszst NAME-REF [source]...",
		"For each BRRES file or subfile, create a name based cross reference"
		" and print the result as list ordered by names in BRRES order."
		" For U8 archives (e.g. track SZS files),"
		" sub file @course_model.brres@ is analysed." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"ILIST",	"ILIST|IL",
		    "wszst ILIST [source]...",
		"List the images files of all sub files of " ARCHIVES_AND
		" archives; non image files are ignored."
		" If listing a directory, then a virtual SZS file"
		" is created (like CREATE does it) and listed." },

  { T_DEF_CMD,	"ILIST_L",	"ILIST-L|ILL|ILISTL",
		    "wit ILIST-L [source]...",
		"List all image files with geometry data."
		" 'ILIST-L' and 'ILL' are short cuts for {ILIST --long}." },

  { T_DEF_CMD,	"ILIST_LL",	"ILIST-LL|ILLL|ILISTLL",
		    "wit ILIST-LL [source]...",
		"List all sub files with advanced geometry data."
		" 'ILIST-LL' and 'ILLL' are short cuts for {ILIST --long --long}." },

  { T_DEF_CMD,	"ILIST_A",	"ILIST-A|ILA",
		    "wszst ILIST-A [source]...",
		"List recursive all image files."
		" 'ILIST-A' and 'ILA' are short cuts for {ILIST --all}." },

  { T_DEF_CMD,	"ILIST_LA",	"ILIST-LA|ILLA|ILISTLA",
		    "wit ILIST-LA [source]...",
		"List recursive all image files with geometry data."
		" 'ILIST-LA' and 'ILLA' are short cuts for {ILIST --long --all}." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"MEMORY",	"MEMORY|MEM",
		    "wszst MEMORY [source]...",
		"Dump a memory map of each source file."
		" Virtual images (extracted directories) are also supported." },

  { T_DEF_CMD,	"MEMORY_A",	"MEMORY-A|MEMA",
		    "wszst MEMORY-A [source]...",
		"Dump recursive a memory map of each source file."
		" 'MEMORY-A' and 'MEMA' are short cuts for {MEMORY --all --all}." },

  { T_DEF_CMD,	"DUMP",		"DUMP",
		    "wszst DUMP [source]...",
		"Dump the internal structure of a BRRES files, compressed or not."
		" Other archives are ignored."
		" The main goal is to verify string relocation."
		" Virtual images (extracted directories) are also supported." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"SHA1",		"SHA1",
		    "wszst SHA1 [source]...",
		"Print a SHA1 checksum for each source."
		" The checksum is calculated by using the uncompressed data"
		" and after patching." },

  { T_DEF_CMD,	"ANALYZE",	"ANALYZE|ANA|ANALYSE",
		    "wszst ANALYZE [source]...",
		"Analyse track files and LE-CODE binaries."
		" Print the results in machine-readable form."
		" The output format is selected by options." },

  { T_DEF_CMD,	"SPLIT",	"SPLIT",
		    "wszst SPLIT [source]...",
		"Analyse (file) names and split them into the different parts."
		" Print the results in machine readable format." },

  { T_DEF_CMD,	"IS_TEXTURE",	"IS-TEXTURE|ISTEXTURE",
		    "wszst IS-TEXTURE --reference reference [source]...",
		"Test each source to see if it's a texture hack versus a reference file."
		" For each source, one human and machine readable status line is printed."
		" It is either »-1=err« on error, »1=yes« for a texture hack,"
		" or »0=no LIST« with a comma separated list of reasons."
		"\n "
		" At the moment KCL, KMP (see notes) and LEX are checked."
		" KMP section CAME has been intentionally left out."
		" Section ENPT is only checked if KCL contains a least one face"
		" of type force-recalculation." },

  { T_DEF_CMD,	"FEATURES",	"FEATURES",
		    "wszst FEATURES [source]...",
		"Analyse track files and create a feature list for each source"
		" in machine readable format."
		" CTGP uses the feature lists to manage ghosts."
		" See option {--lex-features} for details." },

  { T_DEF_CMD,	"DISTRIBUTION",	"DISTRIBUTION|DIS|DISTRIB",
		    "wszst DISTRIBUTION result source...\n"
		    "wszst DISTRIBUTION directory [source]...",
		"Load each @SOURCE@ (SZS, WBZ, U8 or WU8) and calculate a checksum"
		" of the uncompressed data."
		" If @SOURCE@ is a directory, scan it for SZS, WBZ, U8 and WU8 files."
		" If a source directory starts with \"arena\","
		" all tracks of the directory are assumed to be battle arenas."
		" Finally print the results into file @RESULT@."
		"\n "
		" The default name for @RESULT@ is \"distribution.txt\"."
		" Extension .txt is appended to the filename if not defined."
		" If @RESULT@ is a directory, it is used as first source too."
		"\n "
		" If file @RESULT@ exists, it is automatically added as last source"
		" to the list of option --source, but only if not already added."
		" On scanning of file @result@, parameters are scanned too."
		"\n "
		"See »welect DISTRIBUTION« for a new and alternative implementation of this." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"DIFF",		"DIFF|CMP",
		    "wszst DIFF source1 source2\n"
		    "wszst DIFF source... --dest dest",
		"Compare two or more sources (each " ARCHIVES_COMMA " or directory)"
		" on file level and report mismatches for each sub file."
		" If option {--dest} is set, then each source file"
		" is compared with a file in the destination." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"CHECK",	"CHECK",
		    "wszst CHECK [source]...",
		"Make a validity check for track files:"
		" Try to read KCL and KMP (and check both) and find unknown, modified,"
		" needed and unneeded files."
		" Also analyse cannons and make slot proposals."
		" This command uses an internal object and file database."
		" The source can be a SZS, U8, WU8 or WBZ file, or an U8 like directory." },

  { T_DEF_CMD,	"SLOTS",	"SLOTS",
		    "wszst SLOTS [source]...",
		"Try to find out the slots, that will run a track."
		" Each source can be a SZS, U8, WU8 or WBZ file, or an U8 like directory."
		" For each source one status line is printed."
		" The status line contains either a positive or a negative"
		" slot list, whatever is shorter."
		" For battle arenas, the keyword @ARENA@ is printed." },

  { T_DEF_CMD,	"STGI",		"STGI",
		    "wszst STGI [source]...",
		"Find the first KMP:STGI of each file"
		" and print an analysis of it as single line statistic."
		" Additional the number of lap counter check points ('LC') is printed."
		" Track and KMP (binary and text) files are accepted as input." },

  { T_DEF_CMD,	"ISARENA",	"ISARENA",
		    "wszst ISARENA [source]...",
		"Analyse each track for its arena status and print a line with"
		" a status code and the track name."
		" The status code is: »-RACE« for a racing track,"
		" »?MAYBE« for likely an arena, »+ARENA« for an arena,"
		" »+DISPATCH« for an arena with dispatch points"
		" or any error hint beginning with an »!«."
		" So the very first character (one of »!-?+«)"
		" can be used to distinguish the status class."
		"\n "
		" The exit status is >2 on error,"
		" 2 (DIFFER), if at least one file is a racing track,"
		" or 0 (OK), if all sources are arenas." },

  { T_DEF_CMD,	"NORMALIZE",	"NORMALIZE|NORM",
		    "wszst NORMALIZE [source]...",
		"Normalize an U8 archive, compressed or not:"
		" The files are sorted in `Nintendo like' order"
		" and unused data holes are removed."
		" If directory '.' is available it becomes the root directory"
		" for all others. Compressed sources are compressed again with"
		" maximum compression (level 9)."
		" The destination is only written if any data changed."
		"\n "
		" {wszst NORMALIZE} is the same as {wszst PATCH --norm}." },

  { T_DEF_CMD,	"PATCH",	"PATCH",
		    "wszst PATCH [source]...",
		"Patch an U8 archive, compressed or not:"
		" Load the archive, apply all patching and transforming options"
		" and store the file again."
		" Compressed sources are compressed again."
		" The destination is only written if any data changed or if stored in another file."
		" " WILDCARDS },

  { T_DEF_CMD,	"COPY",		"COPY|CP",
		    "wszst COPY source dest\n"
		    "wszst COPY source... [-d|-D] dest",
		"COPY is an alternative command for copying and patching files."
		" It uses the same internal procedure as command PATCH,"
		" but has another command line syntax."
		"\n "
		"The command copies all source files to the destination."
		" If more than 1 source is defined, the destination must be"
		" either a directory or a destination filename with escapes."
		" Patching is also possible and option --overwrite is set implicitly." },

  { T_DEF_CMD,	"DUPLICATE",	"DUPLICATE|DUP",
		    "wszst DUPLICATE source [-d|-D] dest",
		"The command reads a single track file and duplicates it several"
		" times to DEST. Therefor it analyzes the presence flags combinations"
		" with and without extension support and creates one file for each"
		" relevant scenario. A LEX section TEST will be inserted to force"
		" the execution mode for offline tests."
		"\n "
		" The destination filename (not directory part) is searched"
		" for the first occurance of @'@@'@."
		" If not found, the place before the last point is used."
		" At this place an identification string is inserted."
		"\n "
		" Depending on option {--gamemodes} up to " MAX_GAMEMODES
		" files will be created. So the use of option {--fast}"
		" is recommended to speed up the process."
		" Use command {wkmpt GAMEMODES} to list all variants"
		" and to check them beforehand." },

  { T_DEF_CMD,	"MINIMAP",	"MINIMAP",
		    "wszst MINIMAP [source]...",
		"Print minimap data (more exact: the visual field of the minimap)"
		" of a SZS, U8, BRRES or MDL file."
		" If patching options are used, modify the data and write back"
		" the file using its original file format."
		" Only the translation vector is modified and"
		" new translation and inverse matrices are calculated." },

  { T_DEF_CMD,	"COMPRESS",	"COMPRESS|COM",
		    "wszst COMPRESS [source]...",
		"Compress and create a SZS archives."
		" If the source is already compressed,"
		" then decompress and compress it again."
		" The default destination is @'%P/%N.szs'@." },

  { T_DEF_CMD,	"DECOMPRESS",	"DECOMPRESS|DEC",
		    "wszst DECOMPRESS [szs]...",
		"Decompress SZS files."
		" The default destination is @'%P/%N.u8'@." },

  { T_DEF_CMD,	"ENCODE",	"ENCODE|E",
		    "wszst ENCODE [source_dir]...",
		" Create and/or encode sub files if necessary."
		" It works like command CREATE,"
		" but without creating a new first level archive file." },

  { T_DEF_CMD,	"CREATE",	"CREATE|C",
		    "wszst CREATE [source_dir]...",
		"Create " CARCHIVES_OR " archives by adding all not hidden files"
		" of the directory structure."
		" Create and/or encode sub files if necessary."
		" The default destination is @'%P/%N%T'@." },

  { T_DEF_CMD,	"UPDATE",	"UPDATE|U",
		    "wszst UPDATE [source]... [--source=] [--dest=]",
		"Update an existing " ARCHIVES_AND "archive."
		" This is done in similar way as the {EXTRACT} command,"
		" but instead of extracting, source files are searched"
		" in a prior extracted archive."
		" Only files with the same size are updated."
		" The secondary source is used to find the source sub files."
		" The default secondary source is @'%P/%N.d/'@." },

  { T_DEF_CMD,	"EXTRACT",	"EXTRACT|X",
		    "wszst EXTRACT [source]...",
		"Extract all sub files of " ARCHIVES_AND " archives."
		" The default destination is @'%P/%N.d/'@." },

  { T_DEF_CMD,	"XDECODE",	"XDECODE|XD",
		    "wszst XDECODE [source]...",
		"Command @XDECODE@ is a short cut for {EXTRACT --decode}."
		" It extract all sub files of " ARCHIVES_AND " archives"
		" and decodes all supported files."
		" The default destination is @'%P/%N.d/'@." },

  { T_DEF_CMD,	"XEXPORT",	"XEXPORT|XX",
		    "wszst XEXPORT [source]...",
		"Command @XEXPORT@ is a short cut for {EXTRACT --decode --export}."
		" It extract all sub files of " ARCHIVES_AND " archives"
		" and decodes all supported files with enabled export mode."
		" The default destination is @'%P/%N.d/'@." },

  { T_DEF_CMD,	"XALL",		"XALL|XA",
		    "wszst XALL [source]...",
		"Command @XALL@ is a short cut for {EXTRACT --all}."
		" It extract all sub files of " ARCHIVES_AND " archives"
		" recursively and decodes all supported files."
		" The default destination is @'%P/%N.d/'@." },

  { T_DEF_CMD,	"XCOMMON",	"XCOMMON",
		    "wszst XCOMMON [source]...",
		"Command @XCOMMON@ is a short cut for {EXTRACT --basedir=common}."
		" Scan archives and extract files from archives, that are below"
		" directory @'common'@."
		" The default destination is @'%P/%N.d/'@." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"BINARY",	"BINARY|BIN",
		    "wszst BINARY [source]...",
		"Read each source file (binary or text, but never archives)"
		" and write the content as binary file."
		" The file extension of the new file depends on the file type,"
		" but is usually @.bin@."
		" Currently supported file formats are @BMG@, @GEOHIT@, @ITEMSLOT@,"
		" @KCL@, @KMG@, @KMP@, @LEX@, @OBJFLOW@, @PAT@"
		" and the related text formats." },

  { T_DEF_CMD,	"TEXT",		"TEXT|TXT",
		    "wszst TEXT [source]...",
		"Read each source file (binary or text, but never archives)"
		" and write the content as text file."
		" The file extension of the new file is usually @.txt@."
		" Currently supported file formats are @BMG@, @GEOHIT@, @ITEMSLOT@,"
		" @KCL@, @KMG@, @KMP@, @LEX@, @OBJFLOW@, @PAT@"
		" and the related text formats." },

  { T_DEF_CMD,	"CAT",		"CAT",
		    "wszst CAT [source/subfile]...",
		"Extract one or more sub files of " ARCHIVES_AND " archives."
		" The output of all source files is concatenated and written to stdout." },

  { T_DEF_CMD,	"BMG",		"BMG",
		    "wszst BMG [source]...",
		"For each archive all BMG sub files are searched"
		" and, if found, decoded and printed as combined BMG text file."
		" The command works exactly like {wbmgt MIX}." },

  { T_DEF_CMD,	"KCL",		"KCL",
		    "wszst KCL [source]...",
		"For each archive the KCL sub file @course.kcl@ is searched"
		" and, if found, decoded and printed (copied to stdout)."
		"\n "
		" Automated dropping of bad KCL triangles is enabled"
		" if no other drop option is specified by option {--kcl}." },

  { T_DEF_CMD,	"KMP",		"KMP",
		    "wszst KMP [source]...",
		"For each archive the KMP sub file @course.kmp@ is searched"
		" and, if found, decoded and printed (copied to stdout)." },

  { T_DEF_CMD,	"LEX",		"LEX",
		    "wszst LEX [source]...",
		"For each archive the LEX sub file @course.lex@ is searched"
		" and, if found, decoded and printed (copied to stdout)." },

  { T_DEF_CMD,	"INFO",		"INFO",
		    "wszst INFO [source]...",
		"For each archive a file named @info.txt@ and, if not found,"
		" @credits.txt@ is searched, ignoring case."
		" The first found file of each archive is printed (copied to stdout)." },

  { H_DEF_CMD,	"GHOST",	"GHOSTS",
		    "wszst GHOST [source]...",
		"Analyse each file and print a summary about the ghost data." },

  { T_DEF_CMD,	"YAZDUMP",	"YAZDUMP|YD",
		    "wszst YAZDUMP [szs]...",
		"Analyze YAZ compression and print a dump to stdout."
		" Ignore files without YAZ compression." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { H_DEF_CMD,	"VEHICLE",	"VEHICLE",
		    "wszst VEHICLE job ...",
		"Hidden vehicle settings command." },

  //
  //---------- list of all wszst options ----------

  { T_OPT_S,	"VERSION",	"V|version",
		0,
		"Stop parsing the command line, print a version info and exit." },

  { T_OPT_S,	"HELP",		"h|help",
		0,
		"Stop parsing the command line, print a help message and exit." },

  { T_OPT_S,	"XHELP",	"xhelp",
		0,
		"Stop parsing the command line and print a help message"
		" with all commands included. Exit after printing." },

  { T_OPT_GP,	"CONFIG",	"config",
		"file",
		"Define a configuration file or a directory as source."
		" In later case, file @'" CONFIG_FILE "'@ is searched."
		"\n "
		" If option is not not set,"
		" then file @'" CONFIG_FILE "'@ is searched in different directories."
		" Use command @'wszst CONFIG'@ to get more details." },

  { T_OPT_G,	"ALLOW_ALL",	"allow-all|allowall",
		0,
		"Usually commands accept only options with impact to the command."
		" All other options fire a syntax error."
		" But if @--allow-all@ is set, all commands accept all options."
		"\n "
		" This makes changing the command of a long command line"
		" without removing useless options easier."
		" It also helps to override wrong option permissions." },

  { T_OPT_GP,	"COMPATIBLE",	"compatible",
		"vers",
		"The option expects a version number (format @'#.##'@ or @'v#.##'@)"
		" or a revision number (format @'r#'@) as parameter."
		" If set, the tools try to create BMG and KMP text files,"
		" that are compatible to the entered version of the tools."
		" This may override other legacy options." },

  { T_OPT_GP|F_IGNORE,
		"WIDTH",	"width",
		"width",
		"Define the terminal width (number of columns)"
		" for help and some other messages"
		" and disable the automatic detection of the terminal width." },

  { T_OPT_GP|F_IGNORE,
		"MAX_WIDTH",	"max-width|maxwidth",
		"maxwidth",
		"Define the maximum terminal width (number of columns)"
		" for help and some other messages."
		" The default is 120. This option is ignored if --width is set." },

  { T_OPT_G|F_IGNORE,
		"NO_PAGER",	"no-pager|nopager",
		0,
		"Forbid the internal usage of a pager." },

  { T_OPT_GM,	"QUIET",	"q|quiet",
		0,
		"Be quiet and print only error messages."
		" Multiple usage is possible."
		" The impact is command dependent."
		" In general: If set three times,"
		" different file format warnings are suppressed." },

  { T_OPT_GM,	"VERBOSE",	"v|verbose",
		0,
		"Be verbose and print more progress information."
		" Multiple usage is possible."
		" The impact is command dependent." },

  { T_OPT_GM,	"LOGGING",	"L|logging",
		0,
		"This debug option enables the logging of internal lists and maps."
		" Set it up to three times to be more verbose." },

  { T_OPT_GM,	"EXT_ERRORS",	"Y|extended-errors|extendederrors|xerr",
		0,
		"Enable extended error messages. This includes a source file and line reference."
		" Logging level 1 activates it too."
		" @--xerr@ is a short cut." },

  { T_OPT_GM,	"TIMING",	"timing",
		0,
		"Activate the timing of some operations."
		" Logging level 3 activates them too." },

  { T_OPT_GP,	"WARN",		"W|warn",
		"list",
		"Enable or disable warnings."
		" Parameter @'list'@ is a comma separated list of keywords."
		" A minus sign before a keyword disables a warning."
		" Each occurrence of the option will only change entered warning"
		" and all other warnings are untouched."
		"\n "
		" Keyword @DEFAULT@ resets the default settings, @OFF@ disables and @ALL@ enables all."
		" The other allowed keywords are:"
		" @INVALID-OFFSET@."
	//	"\1 See https://szs.wiimm.de/opt/warn for details."
		},

  { T_OPT_G,	"DE",		"de",
		0,
		"Use german track and arena names." },

  { T_OPT_GO|F_IGNORE,
		"COLORS",	"colors",
		"[=modus]",
		"Define the modus for colored text output."
		" Allowed keywords are:"
		" @OFF@ or @NO-COLORS@ to disable colors,"
		" @AUTO@ (default) for automatic detection,"
		" @ON@ for automatic detection but never @OFF@,"
		" @8-COLORS@ and @256-COLORS@ to force 8 or 256 color support."
		" Without parameter, @ON@ is used."
		"\n "
		" @AUTO@ will enable colorized output only for terminals."
		" @AUTO@ and @ON@ use environment variable @TERM@"
		" to find the correct color modus."
		"\n "
		" If a command is prefixed by @'C-'@,"
		" then @--color=ON@ is used implicitly as default." },

  { T_OPT_G|F_IGNORE,
		"NO_COLORS",	"no-colors|nocolors",
		0,
		"Deactive colorized text."
		" This is the default, if an output file is not a terminal." },

  { T_OPT_G,	"CT_CODE",	"ct-code|ctcode",
		0,
		"Enable CT-CODE support."
		"\1 See https://szs.wiimm.de/opt/ctcode for details." },

  { T_OPT_GO,	"LE_CODE",	"le-code|lecode",
		"[=param]",
		"Enable LE-CODE and CT-CODE support."
		" The optional parameter is deprecated and ignored."
		"\1 See https://szs.wiimm.de/opt/lecode for details." },

  { T_OPT_GP,	"LOAD_PREFIX",	"load-prefix|loadprefix",
		"file",
		"Read in a PREFIX file and replace the internal prefix database with the content."
		" https://ct.wiimm.de/export/prefix is the authoritative source for this." },

  { T_OPT_GP,	"LOAD_CATEGORY", "load-category|loadcategory",
		"file",
		"Read in a CATEGORY file and replace the internal category database with the content."
		" https://ct.wiimm.de/export/category is the authoritative source for this." },

  { T_OPT_CP,	"PLUS",		"plus",
		"chars",
		"If a name begins with a plus sign, then all characters up to the first space"
		" are recognized as a plus prefix."
		" The first part consists of all the plus signs followed by any other characters."
		" The first character of the second part is used to determine the sort value."
		" If the second part is empty, then the last plus sign is used instead."
		"\n "
		" The higher sorting value is now calculated from the number of plus signs"
		" in the first part, the more plus signs, the smaller the value."
		" The first character from the second part determines the lower value."
		" If it is in parameter CHARS then the position determines the value,"
		" otherwise the ASCII value to which 500 is added." },

  { T_OPT_CP,	"SPLIT",	"split",
		"num",
		"Only one line with the standardized name is output."
		" The components are divided into NUM (1-13) parts with predifined format strings."
		" The pipe sign (@|@) is used as a separator."
		" An alternative output format can be defined with the {--printf} option."
		"\1 See https://szs.wiimm.de/opt/split for details." },

  { T_OPT_CP,	"PRINTF",	"printf",
		"format",
		"Add an output line with a formatted string."
		"\1 See https://szs.wiimm.de/opt/printf for details." },

  { T_OPT_GMP,	"CHDIR",	"chdir",
		"dir",
		"Set a new working directory for all following options,"
		" for all parameters and for the command execution."
		" This option can be used multiple times,"
		" but for parameter parsing and for command execution only"
		" the last usage is relevant."
		" Nevertheless, @'@@param'@ values are scanned while reading"
		" the command line." },

  { T_OPT_GMP,	"CONST",	"c|const",
		"list",
		"Define constant values, that are used by the internal encoders"
		" and by the numeric options as predefined global variables."
		" This option allows a conditional encoding of text files."
		" It can be used multiple times for multiple definitions."
		"\n "
		" The parameter is a comma separated list of terms"
		" and a term is '@name=expression@'."
		" The expression is calculated by the text parser." },

  { T_OPT_C,	"TOUCH",	"touch",
		0,
		"Mark the minimap as modified to force a new calculation"
		" of the translation and inverse matrices." },

  { T_OPT_C,	"AUTO",		"auto",
		0,
		"Calculate the minimap translations automatically by"
		" using the minimum and maximum vertex coordinates."
		" The 'scale' and 'rotation' fields are reset."
		" This is done as first action"
		" before the SET, CENTER and the other transformations." },

  { T_OPT_CP,	"SET_FLAGS",	"set-flags|setflags",
		"flag",
		"Define the flag values (default 0x31f) of the minimap." },

  { T_OPT_CP,	"SET_SCALE",	"set-scale|setscale",
		"vector",
		"Define the scale values (default 1.0) of the minimap."
		" The parameter is either a vector expression"
		" or a comma separated list of coordinates ('x,z' or 'x,y,z')." },

  { T_OPT_CP,	"SET_ROT",	"set-rot|setrot",
		"vector",
		"Define the rotation values (default 0.0) of the minimap."
		" The parameter is either a vector expression"
		" or a comma separated list of coordinates ('x,z' or 'x,y,z')." },

  { T_OPT_CP,	"SET_X",	"set-x|setx",
		"v1[,v2]",
		"Define 2 values and sort them to define the minimum"
		" and maximum x-coordinates of the minimap."
		" If 'v2' is not entered, '-v1' is used as second value." },

  { T_OPT_CP,	"SET_Y",	"set-y|sety",
		"v1[,v2]",
		"Define 2 values and sort them to define the minimum"
		" and maximum y-coordinates of the minimap."
		" If 'v2' is not entered, '-v1' is used as second value." },

  { T_OPT_CP,	"SET_Z",	"set-z|setz",
		"v1[,v2]",
		"Define 2 values and sort them to define the minimum"
		" and maximum z-coordinates of the minimap."
		" If 'v2' is not entered, '-v1' is used as second value." },

  { T_OPT_C,	"XCENTER",	"xcenter",
		0,
		"Center the minimap in x-direction."
		" This is done after AUTO and SET, but before SCALE and SHIFT." },

  { T_OPT_C,	"YCENTER",	"ycenter",
		0,
		"Center the minimap in y-direction."
		" This is done after AUTO and SET, but before SCALE and SHIFT." },

  { T_OPT_C,	"ZCENTER",	"zcenter",
		0,
		"Center the minimap in z-direction."
		" This is done after AUTO and SET, but before SCALE and SHIFT." },

  { T_OPT_C,	"CENTER",	"center",
		0,
		"Center the minimap in all 3 directions."
		" This is a short cut for {--xcenter --ycenter --zcenter}." },

  { T_OPT_CP,	"SCALE",	"scale",
		"vector[@origin]",
		"Transform the data:"
		" Scale all coordinates and size values by 'vector'"
		" relative to the point 'origin'."
		" If the origin is not set, @0,0,0@ is used instead."
		"\n "
		" Negative values mirror the coordinates."
		" Each parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('x,z' or 'x,y,z')."
		"\n "
		" Transformations are calculated in the order"
		" SCALE, SHIFT, ROTATE, TRANSLATE." },

  { T_OPT_CP,	"SHIFT",	"shift",
		"vector",
		"Transform the data: Add 'vector' to all coordinates."
		" It's simlar to {--translate}, but the addition is done before rotation."
		" The parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('x,z' or 'x,y,z')."
		"\n "
		" Transformations are calculated in the order"
		" SCALE, SHIFT, ROTATE, TRANSLATE." },

  { T_OPT_CP,	"XSS",		"xss",
		"x1old,x1new,x2old,x2new",
		"--xss (x-scale-shift) calculates the X values of {--scale} and {--shift},"
		" so that old values are transformed to the new values."
		" The parameters are numbers or expression." },

  { T_OPT_CP,	"YSS",		"yss",
		"y1old,y1new,y2old,y2new",
		"--yss (y-scale-shift) calculates the Y values of {--scale} and {--shift},"
		" so that old values are transformed to the new values."
		" The parameters are numbers or expression." },

  { T_OPT_CP,	"ZSS",		"zss",
		"z1old,z1new,z2old,z2new",
		"--zss (z-scale-shift) calculates the Z values of {--scale} and {--shift},"
		" so that old values are transformed to the new values."
		" The parameters are numbers or expression." },

  { T_OPT_CP,	"ROT",		"rot",
		"degree[@origin]",
		"Transform the data: Rotate all coordinates and rotation values"
		" by the angle 'degree' (is a vector) around the 3 axes."
		" All 3 axes goes through the point 'origin'."
		" If the origin is not set, @0,0,0@ is used instead."
		"\n "
		" Each parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('x,y,z')."
		"\n "
		" Option --rot is an alternative for {--xrot}, {--yrot} and {--zrot}"
		" to define all 3 rotations in one step."
		" Transformations are calculated in the order"
		" SCALE, SHIFT, X-ROTATE, Y-ROTATE, Z-ROTATE, TRANSLATE." },

  { T_OPT_CP,	"XROT",		"xrot",
		"degree[@origin]",
		"Transform the data: Rotate all coordinates and rotation values"
		" by the angle 'degree' around the x-axis, that goes through the"
		" point 'origin'. If the origin is not set, @0,0,0@ is used instead."
		"\n "
		" Each parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('y,z' or 'x,y,z')."
		"\n "
		" Transformations are calculated in the order"
		" SCALE, SHIFT, X-ROTATE, Y-ROTATE, Z-ROTATE, TRANSLATE." },

  { T_OPT_CP,	"YROT",		"yrot|hrot",
		"degree[@origin]",
		"Transform the data: Rotate all coordinates and rotation values"
		" by the angle 'degree' around the x-axis, that goes through the"
		" point 'origin'. If the origin is not set, @0,0,0@ is used instead."
		" This is a horizontal counterclockwise rotation"
		" and the old option name --hrot can also used."
		"\n "
		" Each parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('x,z' or 'x,y,z')."
		"\n "
		" Transformations are calculated in the order"
		" SCALE, SHIFT, X-ROTATE, Y-ROTATE, Z-ROTATE, TRANSLATE." },

  { T_OPT_CP,	"ZROT",		"zrot",
		"degree[@origin]",
		"Transform the data: Rotate all coordinates and rotation values"
		" by the angle 'degree' around the z-axis, that goes through the"
		" point 'origin'. If the origin is not set, @0,0,0@ is used instead."
		"\n "
		" Each parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('x,y' or 'x,y,z')."
		"\n "
		" Transformations are calculated in the order"
		" SCALE, SHIFT, X-ROTATE, Y-ROTATE, Z-ROTATE, TRANSLATE." },

  { T_OPT_CP,	"YPOS",		"ypos",
		"pos",
		"This option defines an Y position for KMP:CKPT transformations."
		" It has only impact to X and Z rotations"
		" (Options {--xrot} and {--zrot})."
		" If not set, the mean y of the active rotation origins are used." },

  { T_OPT_CP,	"TRANSLATE",	"translate|trans",
		"vector",
		"Transform the data: Add 'vector' to all coordinates."
		" It's simlar to {--shift}, but the addition is done after rotation."
		" The parameter is either a vector expression or a comma separated"
		" expression list of coordinates ('x,z' or 'x,y,z')."
		" @--trans@ is a short cut."
		"\n "
		" Transformations are calculated in the order"
		" SCALE, SHIFT, ROTATE, TRANSLATE." },

  { T_OPT_C,	"NULL",		"null",
		0,
		"Create a neutral transformation without affecting the coordinates."
		" The only influence is, that the dependent values (like maximum)"
		" are re-calculated as if a transformation has taken place." },

  { T_OPT_C,	"NEXT",		"next",
		0,
		"Close the current transformation step with all scaling, shifting,"
		" rotation and translation options"
		" and open a new step with cleared options."
		" On transformation each step is logical done one by one."
		" In real, one total transformation matrix is calculated"
		" and used for fast transformations."
		"\n "
		" If option --next is used, the ability for modifying the scale and"
		" rotation vectors (not the positions) of different KMP sections"
		" is nearly always lost. A warning is printed if this occurs." },

  { T_OPT_CP,	"ASCALE",	"ascale",
		"factor@dir",
		"Transform the data and do an axis scale:"
		" Close the current transformation step like {--next} and scale the"
		" data by @'factor'@ into the direction given by the vector @'dir'@."
		" Store the resulting matrix as single transformation step"
		" and open a new one."
		"\n "
		" This kind of transformation is EXPERIMENTAL!" },

  { T_OPT_CP,	"AROT",		"arot",
		"degree[@pos1]@pos2",
		"Transform the data and do an axis rotation:"
		" Close the current transformation step like {--next} and rotate"
		" the data by @'degree'@ around the axis defined by the 2 points."
		" If @'pos1'@ is not set, use @0,0,0@ instead."
		" Store the resulting matrix as single transformation step"
		" and open a new one."
		"\n "
		" This kind of transformation is EXPERIMENTAL!" },

  { T_OPT_CP,	"TFORM_SCRIPT",	"tform-script|tformscript",
		"script",
		"Load the script and execute it by the text parser."
		"\n"
		" Then for each coordinate (2D and 3D),"
		" call the macro @TRANSFORM@ after all other transformations."
		" Call it without parameters"
		" and set variable @$$I@ with a zero based index of all vertices,"
		" variable @$$D@ to the dimension (2 or 3)"
		" and the variable @$$P@ to the vector of the current vertex."
		" If the macro returns a vector, it is the new position."
		"\n "
		" For each file, macro @BEGIN@ is called at the beginning"
		" and @END@ after processing all points,"
		" but only, if the macros are defined."
		" For @END@, variable @$$I@ is set to the"
		" number of processed vertices in the current file."
		"\n "
		" Before each macro call, the private and local variables are cleared"
		" and only global parameters are permanent."
		" The zero based index is cleared for each new source."
		" The parameters of the macro calls are defined in the local name space." },

  { T_OPT_C,	"MINIMAP",	"minimap",
		0,
		"Fix the minimap position in the same way as {wszst MINIMAP --auto}."
		" All other minimap related options are ignored for the minimap processing."
		"\1 See https://szs.wiimm.de/cmd/wszst/minimap for details." },

  { T_OPT_CP,	"RM_GOBJ",	"rm-gobj|rmgobj",
		"objlist",
		"Remove objects from the KMP section @GOBJ@."
		" Therefor a comma separated object list with single values"
		" like @'a'@ or ranges like @'a:b'@ is expected."
		" @'a'@ and @'b'@ are numbers, object names (prefixed by 'o$$')"
		" or any other parser expressions." },

  { T_OPT_GO,	"BATTLE",	"battle",
		"[=mode]",
		"@MODE@ is either @OFF@, @AUTO@ (default) or @ON@."
		" Without value, @ON@ is used."
		"\n "
		" If mode is @ON@, assume a battle arena and enable a special battle support."
		" If mode is @OFF@, disable special battle arena support."
		" If mode is @AUTO@, analyse the KMP and estimate, if it is a battle areana."
		" Modes @ON@ and @OFF@ override KMP parameter @BATTLE-MODE@." },

  { T_OPT_GO,	"EXPORT_FLAGS",	"export-flags|exportflags|exflags",
		"[=mode]",
		"@MODE@ is either @OFF@, @AUTO@ (default) or @ON@."
		" Without value, @ON@ is used."
		" --exflags is an alternative name for the option."
		"\n "
		" If mode is @ON@, this feature is forced:"
		" On creating a KMP text file, the option @EXPORT-FLAGS@ is always set to 1."
		" On creating a binary KMP file, the flags are always exported as lowest byte"
		" of the scale factors or x-rotation."
		"\n "
		" If mode is @OFF@, this feature is totally disabled."
		" If mode is not @AUTO@, KMP parameter @EXPORT-FLAGS@ is overridden." },

  // 2018-08: may be [[obsolete]] in 1 year
  { T_OPT_GO,	"ROUTE_OPTIONS", "route-options|routeoptions|ropt",
		"[=mode]",
		"@MODE@ is either @OFF@, @AUTO@ (default) or @ON@."
		" Without value, @ON@ is used."
		"\n "
		" On mode @ON@, the new commands @$$PREV@ and @$$NEXT@ are printed"
		" for each group of KMP sections ENPT and ITPT."
		" On mode @AUTO@, the command is only printed,"
		" if any option is set or if parameter @SHOW-OPTIONS@ is >0."
		" On mode @OFF@, this feature is totally disabled to create KMP text files"
		" that are compatible with versions <1.60a."
		"\n "
		" This option exists only for a transition period."
		" Its behaviour will be changed in the future."
		" @--ropt@ is a short cut."
		},

  { T_OPT_GO,	"WIM0",		"wim0",
		"[=mode]",
		"@MODE@ is either @OFF@, @AUTO@ (default) or @ON@."
		" Without value, @ON@ is used."
		"\n "
		" If mode is @ON@, a KMP section called WIM0 is added on binary KMP creation."
		" The sections contains information for Wiimms tools."
		"\n "
		" If mode is @OFF@, this feature is totally disabled."
		" If mode is not @AUTO@, KMP parameter @KMP-WIM0@ is overridden." },

  { T_OPT_CP,	"SLOT",		"slot",
		"keyword",
		"Patch a track file to run at the specified slot."
		" KMP and KCL may be modified."
		" BRRES files may be added to or removed from SZS."
		" The main keywords are @DAISY, DESERT, SHERBET, SHYGUY,"
		" STANDARD@ and @MOST@."
		" Slot numbers (except '42' and '4.2')"
		" and different other track names are also accepted."
		" For some conversions, the auto-add library is needed."
		"\1 See https://szs.wiimm.de/opt/slot for details." },

  { T_OPT_GP,	"LOAD_KCL",	"load-kcl|loadkcl",
		"file",
		"Load a KCL or OBJ file for reference issues." },

  { T_OPT_GP,	"KCL",		"kcl",
		"list",
		"Set global options for KCL processing."
		" Parameter @'list'@ is a comma separated list of keywords."
		" A minus sign before a keyword disables a setting."
		" Each occurrence of the option will only change entered settings"
		" and all other settings are untouched."
		"\n "
		" Keyword @DEFAULT@ resets the default settings"
		" and @CLEAR@ disables all."
		" Keywords @SMALL, MEDIUM@ and @CHARY@ select"
		" the default parameters for the octree creation."
		" The other allowed keywords are:"
		" @FAST, NEW, CENTER, ROUND, NORMALS, MTL,"
		" WIIMM, TRIANGLES, OUT-SWAP, G, USEMTL, CLIP, IN-SWAP, AUTO, HEX4,"
		" HEX23, HEX, DROP-UNUSED, DROP-FIXED, DROP-INVALID, DROP,"
		" RM-FACEDOWN, RM-FACEUP, FIX-ALL, TINY-0 ... TINY-7,"
		" CONV-FACEUP, WEAK-WALLS, SORT, INPLACE, SILENT@ and @LOG@."
		"\1 See https://szs.wiimm.de/opt/kcl for details." },

  { T_OPT_CP,	"KCL_FLAG",	"kcl-flag|kclflag",
		"joblist",
		"Modify the KCL flag of KCL files."
		" A comma separated job list in the format @'srclist=dest'@"
		" is expected. @'srclist'@ is a plus sign separated list"
		" with single values like @<val>@, @T<type>@ and @<val>/<mask>@"
		" or ranges like @<val>:<val>@, @T<type>:<type>@"
		" and @<val>:<val>/<mask>@."
		" @'<val>'@ is a complete KCL flag in the range of @0..0xffff@."
		" @'<type>'@ are only the base types in the range of @0..0x1f@."
		" If @<mask>@ is present, it marks the relevant bits."
		" For the @T<type>@ variant, the mask is set to @0x1f@."
		"\n "
		" All KCL flags of the source list are assigned to the new value @'dest'@,"
		" but only the set bits of @'<mask>'@ are modified."
		" A later definition overrides the previous one."
		"\1 See https://szs.wiimm.de/opt/kcl-flag for details." },

  { T_OPT_CP,	"KCL_SCRIPT",	"kcl-script|kclscript",
		"script",
		"After reading a KCL source file and executing all transformations,"
		" the text file @'script'@ is executed by the internal text parser."
		" The script should use the parser functions @tri$$*()@"
		" to read and modify the triangle positions and flags,"
		" or to add or remove complete triangles."
		"\n "
		" If this option is used multiple times,"
		" each script is executed in the entered order."
		"\1 See https://szs.wiimm.de/opt/kcl-script for details." },

  { T_OPT_GP,	"TRI_AREA",	"tri-area|triarea",
		"EXPR",
		"Define the minimal area size of KCL triangles."
		" The intention is to ignore triangles that are generally to small."
		" @EXPR@ is a floating point number or expression."
		" Triangles are invalidated if their area size is smaller than @EXPR@."
		" The test is executed after reading files, after transformations,"
		" and after calculating normals and lengths."
		" Values between 0.01 and 4.0 are recommended."
		" The careful value 1.0 is used as default."
		" Value 0 disables this filter functionality." },

  { T_OPT_GP,	"TRI_HEIGHT",	"tri-height|triheight|tri-ht|triht",
		"EXPR",
		"Define the minimal height of KCL triangles."
		" The intention is to ignore deformed triangles (very slim, but long)."
		" @EXPR@ is a floating point number or expression."
		" Triangles are invalidated if at least 1 of the 3 heights"
		" is smaller than @EXPR@."
		" The test is executed after reading files, after transformations,"
		" and after calculating normals and lengths."
		" Values between 0.01 and 2.0 are recommended."
		" The careful value 1.0 is used as default."
		" Value 0 disables this filter functionality."
		"\n "
		" --tri-ht is an alternative name for the option." },

  { T_OPT_CP,	"FLAG_FILE",	"flag-file|flagfile",
		"filename",
		"Define a flag file if scanning an OBJ file (KCL processing)."
		" The file name may contain %-escapes."
		" Set it to an empty string to disable flag files at all."
		"\n "
		" The tool search the flag file in the following order until found:"
		" File defined by --flag-file;"
		" OBJ file name with replaced extension ('a.obj' becomes 'a.flag');"
		" OBJ file name with appended extension ('a.obj' becomes 'a.obj.flag')."
		"\1 See https://szs.wiimm.de/opt/flag-file for details." },

  { H_OPT_GO,	"XTRIDATA",	"xtridata|xtri",
		"[=level]",
		"Enable extended KCL triangles tests. The default level is 1." },

  { T_OPT_GP,	"KMP",		"kmp",
		"list",
		"Set global options for KMP processing."
		" Parameter @'list'@ is a comma separated list of keywords."
		" A minus sign before a keyword disables a setting."
		" Each occurrence of the option will only change entered settings"
		" and all other settings are untouched."
		"\n "
		" Keyword @DEFAULT@ resets the default settings"
		" and @CLEAR@ disables all."
		" The other allowed keywords are:"
		" @FORCE, NEW, RM-SPCITEM, LEFT, RIGHT, WIDE, NARROW,"
		" FIX-CKPH, FIX-ENPH, FIX-ITPH, FIX-PH,"
		" FIX-CKNEXT, FIX-CKJGPT, FIX-CK, FIX-ALL,"
		" MASK-PFLAGS, RM-LECODE, PURGE-GOBJ, FULL-DEFOBJ,"
		" DUMP-CLASS, DUMP-ONEWAY, DUMP-ALL,"
		" 1LAP ... 9LAPS, MAX-LAPS, RM-EMPTY,"
		" TINY-0 ... TINY-7, INPLACE, SILENT@ and @LOG@."
		"\1 See https://szs.wiimm.de/opt/kmp for details." },

  { T_OPT_GP,	"SPEED_MOD",	"speed-mod|speedmod",
		"factor",
		"The speed modifier is a user extensions to change the basic speed"
		" of all vehicles. The speed factor itself is stored into"
		" KMP:STGI section of a track."
		"\n "
		" If --speed-mod is set to a value >0.0, then the last 2 bytes "
		" of the STGI section are patched. The value 0.0 reset"
		" these 2 bytes and deactivates the speed modifier." },

  { T_OPT_GP,	"KTPT2",	"ktpt2",
		"vector",
		"Insert or replace a second KTPT. Use VECTOR as new position."
		" VECTOR is either @X,Z@ or @X,Y,Z@ or an expression."
		" Set Y to 0 for an automatic defined height."
		"\n "
		" If the first 4 characters of VECTOR are 'AUTO' (ignoring case),"
		" then the position is calculated automatically"
		" and set to the recommendation of command @wkmpt KTPT@."
		"\n "
		" The tool looks for the nearest lap counter (CKPT with mode 0)"
		" and adjust the KTPT: The direction of the lap counter"
		" is copied and the KTPT is moved to the lap counter line"
		" using the new direction."
		"\n "
		" LE-CODE uses the second KTPT to draw the finish line"
		" at another place away from the start position." },

  { T_OPT_GP,	"TFORM_KMP",	"tform-kmp|tformkmp|tkmp",
		"list",
		"Select KMP sections and objects for a transformation."
		" @--tkmp@ is a short cut."
		" The option expects a comma separated list of keywords."
		" Non ambiguous of keywords are allowed and minus signs within"
		" keywords are optional (e.g. @GOBJS@)."
		" A minus sign before a keyword disables transformation of"
		" the related section or object."
		"\n "
		" Sections are selected by the KMP section names"
		" @AREA, CAME, CKPT, CNPT, ENPT, GOBJ, ITPT, JGPT,"
		" KTPT, MSPT@ and @POTI@."
		"\n "
		" Objects are selected by the keywords"
		" @AREA-POSITION, AREA-ROTATE, AREA-SCALE, CAME-POSITION, CKPT-POSITION,"
		" CNPT-POSITION, CNPT-ROTATE, ENPT-POSITION, ENPT-SCALE,"
		" GOBJ-POSITION, GOBJ-ROTATE, GOBJ-SCALE, ITPT-POSITION, ITPT-SCALE,"
		" JGPT-POSITION, JGPT-ROTATE, KTPT-POSITION, KTPT-ROTATE,"
		" MSPT-POSITION, MSPT-ROTATE@ and @POTI-POSITION@."
		"\n "
		" The keywords @NONE@ and @ALL@ (default) complete the field." },

  { T_OPT_CP,	"REPAIR_XPF",	"repair-xpflags|repairxpflags|repxpf",
		"file",
		"Load a KMP file and use it to repair the settings of the extended"
		" presence flags, if they are destroyed by another KMP tool."
		" @--repxpf@ is a short cut for this option." },

  { T_OPT_CP,	"GAMEMODES",	"gamemodes|gmodes",
		"list",
		"Filter the list of combinations."
		" The parameter is a comma separated list of keywords:"
		" @AUTO@: Add a automatic mode without conditions first."
		" @STANDARD@ (or @STD@): Add standard modes."
		" @1STANDARD@ (or @1STD@): Add standard modes first."
		" @BALLOON@: Add balloon battle modes."
		" @COIN@: Add coin runners modes."
		" @BATTLE@: Short cut for 'BALLOON,COIN'."
		" @VERSUS@ (or @VS@): Add versus modes."
		" @ITEMRAIN@: Add itemrain modes."
		" @RACE@: Short cut for 'VERSUS,ITEMRAIN'."
		" @ALL@ (default): Short cut for 'STANDARD,BATTLE,RACE'."
		" @NO-MODES@: Exclude 'ALL' and create auto modes only."
		" @ENGINE@: Include engine classes (default)."
		" @NO-ENGINE@: Exclude engine classes."
		" @RANDOM@: Include random modes (default)."
		" @NO-RANDOM@: Exclude random modes."
		" @OFFLINE@: Filter offline modes."
		" @ONLINE@: Filter online modes."
		" @NO-TT@: Exclude time trial."
		" @TT@: Force a time trial scenario, even if it is a duplicate."
		" @FULL@: Don't eliminate duplicates from list (up to "
				MAX_GAMEMODES " modes)."
		" @NSORT@: Sort the list by name."
		" @ISORT@: Sort the list by index."
		" @DEBUG@: Print a debug info about this option."
		},

  { T_OPT_GP,	"MDL",		"mdl",
		"list",
		"Set global options for MDL processing."
		" To enable MDL patching, use option {--patch-file}"
		" and select BRRES files for patching."
		"\n "
		" Parameter @'list'@ is a comma separated list of keywords."
		" A minus sign before a keyword disables a setting."
		" Each occurrence of the option will only change entered settings"
		" and all other settings are untouched."
		"\n "
		" A MDL transformation (controlled by the transformation options)"
		" is only done, if at least one of the keywords"
		" @VECTOR@ or @VERTEX@ is set."
		"\n "
		" Keyword @DEFAULT@ resets the default settings (@VERTEX@)"
		" and @CLEAR@ disables all."
		" The other allowed keywords are: @VECTOR, MATRIX,"
		" VERTEX, PARENT, CHILD@ and @LOG@."
		"\1 See https://szs.wiimm.de/opt/mdl for details." },

  { T_OPT_GP,	"PAT",		"pat",
		"list",
		"Set global options for PAT processing."
		" Parameter @'list'@ is a comma separated list of keywords."
		" A minus sign before a keyword disables a setting."
		" Each occurrence of the option will only change entered settings"
		" and all other settings are untouched."
		"\n "
		" Keyword @DEFAULT@ resets the default settings"
		" and @CLEAR@ disables all."
		" The other allowed keywords are:"
		" @AUTO, SIMPLE, COMPLEX, BOTH, SILENT@ and @LOG@."
		"\1 See https://szs.wiimm.de/opt/pat for details." },

  { T_OPT_GP,	"PATCH_FILE",	"patch-file|patchfile|pfile",
		"list",
		"Define, which kind of KMP, KCL and BRRES/MDL files are objects"
		" for patching. @--pfile@ is a short cut."
		"\n "
		" Parameter @'list'@ is a comma separated list of keywords."
		" A minus sign before a keyword disables a setting."
		" Each occurrence of the option will only change entered settings"
		" and all other settings are untouched."
		"\n "
		" Keyword @DEFAULT@ resets the default settings (@TRACK,LOG@)"
		" and @CLEAR@ disables all."
		" The other allowed keywords are:"
		" @KMP, KCL, OTHER-KCL, ALL-KCL, MODEL, MAP, VRCORN,"
		" OTHER-BRRES, ALL-BRRES, TRACK (=KMP,KCL,MODEL,MAP,VRCORN),"
		" ALL, LOG@ and @LOG-ALL@."
		"\1 See https://szs.wiimm.de/opt/patch for details." },

  { T_OPT_GP,	"KMG_LIMIT",	"kmg-limit|kmglimit",
		"seconds",
		"Limit the run time for Ballon Battle and Coin Runners"
		" to the given time in SECONDS. Smaller values are not changed."
		" The limitation is carried out after loading a binary file"
		" and before scanning a text file."
		" Value 0 disables this option." },

  { T_OPT_GP,	"KMG_COPY",	"kmg-copy|kmgcopy",
		"id",
		"Make all 10 slots of a KMG file equal after loading a binary file"
		" and before scanning a text file."
		" ID is either a slot index (0..9) or a slot name (A11..A25)"
		" and declares the source slot. Value -1 disables this option."
		" If ID contains brackets [...], then it is analysed as an arena name and"
		" the part between the brackets is searched for attributes A11..A25." },

//  { H_OPT_GP,	"KMG_AUTO_COPY","kmg-auto-copy|kmgautocopy",
//		0,
//		"Analyze the attributes of the source file name to find attributes"
//		" from @a21@ to @a25@. If found, use is as slot name ..." },

  { T_OPT_G,	"LT_CLEAR",	"lt-clear|ltclear",
		0,
		"Clear the LEX/TEST section and remove the section and course.lex if empty."
		" This option is executed before all other --lt-* options." },

  { T_OPT_GP,	"LT_ONLINE",	"lt-online|ltonline",
		"mode",
		"Set LEX/TEST parameter OFFLINE-ONLINE to this MODE."
		" MODE is one of @AUTO@ (default), @NEVER@ or @ALWAYS@."
		" Only track.szs files are modified."
		" If needed, file course.lex and section TEST"
		" are created or removed if empty." },

  { T_OPT_GP,	"LT_N_PLAYERS",	"lt-n-players|lt-nplayers|ltnplayers",
		"offline,online",
		"Set LEX/TEST parameters N-OFFLINE and N-ONLINE to these integer values."
		" Only track.szs files are modified."
		" If needed, file course.lex and section TEST"
		" are created or removed if empty." },

  { T_OPT_GP,	"LT_COND_BIT",	"lt-cond-bit|lt-condbit|ltcondbit",
		"bitnum",
		"Set LEX/TEST parameter COND-BIT to this integer value."
		" Use -1 to deactivate this COND-BIT."
		" Only track.szs files are modified."
		" If needed, file course.lex and section TEST"
		" are created or removed if empty." },

  { T_OPT_GP,	"LT_GAME_MODE",	"lt-game-mode|lt-gamemode|ltgamemode",
		"mode",
		"Set LEX/TEST parameter GAME-MODE to this MODE."
		" MODE is one of @AUTO@ (default), @BALLOON, COIN, VERSUS@ or @ITEMRAIN@."
		" Only track.szs files are modified."
		" If needed, file course.lex and section TEST"
		" are created or removed if empty." },

  { T_OPT_GP,	"LT_ENGINE",	"lt-engine|ltengine",
		"mode",
		"Set LEX/TEST parameter ENGINE to this MODE."
		" MODE is one of @AUTO@ (default), @BATTLE, 50CC, 100CC, 150CC, 200CC,"
		" 150M@ or @200M@. Only track.szs files are modified."
		" If needed, file course.lex and section TEST"
		" are created or removed if empty." },

  { T_OPT_GP,	"LT_RANDOM",	"lt-random|ltrandom",
		"index",
		"Force a random scenario, if between 1 and 8."
		" Value 0 enables auto selection and -1 disables this option."
		" Only track.szs files are modified."
		" If needed, file course.lex and section TEST"
		" are created or removed if empty." },

  { T_OPT_G,	"LEX_PURGE",	"lex-purge|lexpurge",
		0,
		"Delete LEX sections without any impact."
		" Delete sub-file course.lex if it no longer contains a section."
		" This option is executed after all --lt-* and --lex-* options." },

  { T_OPT_G,	"LEX_FEATURES",	"lex-features|lexfeatures",
		0,
		"Calculate LEX setion FEAT (features) by analysing the track file."
		" If no features found, delete the section and the possibly empty sub-file"
		" course.lex. If features found, then insert the section or update"
		" an existing section. If necessary, the sub-file course.lex is created."
		" This option is executed after all --lt-* options and after --lex-purge."
		"\n "
		" CTGP uses the FEAT section to manage ghosts." },

  { T_OPT_G,	"LEX_RM_FEAT",	"lex-rm-features|lexrmfeatures",
		0,
		"Remove LEX setion FEAT (features) if exists"
		" and the possibly empty sub-file course.lex."
		" This option is ignored if --lex-features is set."
		" It is executed after all --lt-* options and after --lex-purge." },

  { T_OPT_CMP,	"LOAD_BMG",	"load-bmg|loadbmg",
		"file",
		"Load immediately the file to fill an internal message pool."
		" The option can be used multiple times"
		" to fill the message pool by different sources."
		" A later message overrides the previous message."
		"\n "
		" The internal message pool is patched by all"
		" {--patch-bmg} options." },

  { T_OPT_CMP,	"PATCH_BMG",	"patch-bmg|patchbmg|pb",
		"mode[cond][=param]",
		PATCH_BMG_TEXT("") },

  { T_OPT_CMP,	"MACRO_BMG",	"macro-bmg|macrobmg|mb",
		"file",
		"Load a BMG file and add the messages to the macro library;"
		" already existing entries are replaced."
		" The macro library is accessed by escape sequence @\\m{{MID}@"
		" as fallback, if the active file has not already defined the"
		" message @MID@ by itself." },

  { T_OPT_CMP,	"FILTER_BMG",	"filter-bmg|filterbmg|fb|msg",
		"list",
		"If this option is set, the BMG messages are filtered by this list."
		" Only enabled messages are exported to the output BMG file"
		" (binary or text)."
		"\n "
		" The parameter is a list of message ids"
		" (short MID, 'Txx' or 'Uxx' or Mxx' or hex number)"
		" or message ranges (MID:MID) or one of the keywords"
		" @NONE, IDENT (=ID), PARAM, CUPS, TRACKS, ARENAS, CHAT,"
		" CTCODE (=CT), CTUPS, CTTRACKS, CTARENAS, CTREFS,"
		" LECODE (=LE), LEUPS, LETRACKS, LEARENAS, LEREFS,"
		" XCODE (=X), XUPS, XTRACKS, XARENAS, XREFS,"
		" ALLCODE, ALLUPS, ALLTRACKS, ALLARENAS, ALLREFS,"
		" GENERIC@ or @ALL@."
		" If an element is preceded by a minus sign, "
		" it is removed from the filter list (disabled)."
		"\n "
		" With v2.01a, this option was renamed from --msg to the more"
		" meaningful name --filter-bmg. The old name is still available."
		"\1\n  Read https://szs.wiimm.de/opt/filter-bmg for more details." },

  { T_OPT_C,	"LE_MENU",	"le-menu|lemenu",
		0,
		"Patch language independent SZS files from directory .../Scene/UI/"
		" to change the menu as required by LE-CODE"
		" to be able to select tracks from more than 8 cups." },

  { T_OPT_CP,	"TITLE_SCREEN",	"title-screen|titlescreen",
		"dir",
		"Specify a directory from which to search and replace title screens."
		" Title screens only appear in the file 'Title.szs'"
		" and there in the sub-directory './title/timg/'."
		" If the replacement file is not found, then search file 'title1.szs'"
		" to patch standard files or file 'title2.szs' to patch bokeboke files." },

  { T_OPT_CP,	"CUP_ICONS",	"cup-icons|cupicons",
		"image",
		"Load given image, convert it to TPL.CMPR and add the result as sub-files"
		" 'button/timg/ct_icons.tpl' and as 'control/timg/ct_icons.tpl'"
		" to files 'Channel.szs', 'MenuMulti.szs' and 'MenuSingle.szs'."
		" Both sub-files are always linked, so that storage space is saved." },

  { T_OPT_C,	"NO_COPY",	"no-copy|nocopy",
		0,
		"Don't copy a file if it was not modified." },

  { T_OPT_GP,	"CACHE",	"cache",
		"dir",
		"Define a directory for a SZS cache."
		" If creating a YAZ0 compressed file with standard compression (level 9),"
		" the cache is searched for the compressed version."
		" File cache-content.txt is used as file list with hash values."
		" If the cache file is not found or the content differ,"
		" the file is (re-)created and exit status 2=differ is returned."
		"\n "
		" Background: Loading a compressed SZS and content verification"
		" is much faster than YAZ compression."
		"\n "
		" For legay reasons:"
		" If parameter DIR is a file, it is split into directory and base name."
		" The directory is used for the cache directory and the base name"
		" for option {--cname}, but only if {--cname} is not used." },

  { T_OPT_GP,	"CNAME",	"cname",
		"name",
		"Name of the first cache file."
		" This parameter overrides the generated cache name."
		" The name is used only once for the very first cache candidate." },

  { T_OPT_G,	"PARALLEL",	"parallel",
		0,
		"Tell the tool that it will be called multiple times in parallel."
		" Currently it only has an effect if {--cache} is enabled." },

  { T_OPT_C,	"ID",		"id",
		0,
		"Print an ID with 8 characters instead of a SHA-1 sum with"
		" 40 characters. The SHA-1 sum is used to calculate the ID."
		" The ID consists of '0'-'9' and 'a'-'v' (32 different characters)." },

  { T_OPT_CO,	"BASE64",	"base64",
		"[=mode]",
		"Encode the the checksum string as BASE64 with 27 characters"
		" instead of hex."
		" If a parameter is set, it acts like {--coding=mode}." },

  { T_OPT_CO,	"DB64",		"db64",
		"[=mode]",
		"Encode the the checksum string and some length as BASE64"
		" with 32 characters instead of hex for database searches."
		" This options implies {--coding=URL} if no other coding is defined."
		" If a parameter is set, it acts like {--coding=mode}."
		" Option {--db64} has priority over {--base64}." },

  { T_OPT_CP,	"CODING",	"coding",
		"mode",
		"Define the coding of the non alphanumeric characters for BASE64:"
		" STANDARD (+/=, default), URL (-_=), STAR (-_*) or XML (.-*)." },

  { T_OPT_C,	"VERIFY",	"verify",
		0,
		"Calculate the checksum and compare the filename and return"
		" exit status (0=valid, 2=differ)."
		" The result is valid, if the checksum is found"
		" with a leading and a following point or minus sign."
		" If {--quiet} is set, error messages are suppressed." },

  { T_OPT_C,	"ROUND",	"round",
		0,
		"Print additional lines with rounded values"
		" (to 3 and 2 bytes for floats and 7 and 6 bytes for doubles)."},

  { T_OPT_CM,	"LONG",		"l|long",
		0, "Print in long format. Multiple usage possible." },

//  { T_OPT_CM,	"FULL",		"full",
//		0,
//		"Print full lists instead of partial lists." },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0,
		"Suppress printing of header and footer." },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0,
		"Suppress information lines in decoded text files."
		" This includes syntax information ({--no-header})."
		" If set twice, the output of unneeded sections is also suppressed." },

  { T_OPT_CM,	"IN_ORDER",	"in-order|inorder",
		0,
		"Process the input files in order of the command line"
		" and don't delete duplicates." },

  { T_OPT_CM,	"PIPE",		"pipe",
		0,
		"Use pipe sign instead of spaces to separate columns." },

  { T_OPT_C,	"DELTA",	"delta",
		0,
		"Print only modified records or sections on text output."
		" Supported file formats until now: @GEOHIT@, @ITEMSLOT@, @KMG@"
		" and @OBJFLOW@."
		" If set twice, limit output to modified records"
		" (only relevant for @ITEMSLOT@)."  },

  { T_OPT_C,	"DIFF",		"diff",
		0,
		"On text output, print only differences from original files."
		" It's a short cut for --no-header --brief --brief --delta --delta."
		" Additional, the output is limited to data sections." },

  { H_OPT_CP,	"EPSILON",	"epsilon",
		"float",
		"When comparing floating point numbers, 2 numbers are considered identical"
		" if their difference is less than or equal to epsilon."
		" The default value is @" CONVERT_TO_STRING(DEF_EPSILON_POS) "@." },

  { T_OPT_CP,	"BMG_ENDIAN",	"bmg-endian|bmgendian",
		"name",
		"Force an endian for new BMG files."
		" Accepted values are @BIG@ or @BE@ for big endian, @LITTLE@ or @LE@"
		" for little endian and @AUTO@ for automatic detection (default)."
		"\n "
		" Mario Kart Wii uses big endian;"
		" the default if no other endian is defined." },

  { T_OPT_CP,	"BMG_ENCODING",	"bmg-encoding|bmgencoding",
		"name",
		"Force an encoding for new BMG files."
		" Accepted encoding names are @CP-1252@, @UTF-16BE@ (or @UTF-16@),"
		" @SHIFT-JIS@, @UTF-8@ and @AUTO@ for automatic detection (default)."
		" Same keywords without minus signs are accepted too."
		"\n "
		" Mario Kart Wii uses encoding @UTF-16BE@;"
		" the default if no other encoding is defined." },

  { T_OPT_CP,	"BMG_INF_SIZE",	"bmg-inf-size|bmginfsize",
		"size",
		"Defines the 'INF1' size of new BMG files between 4 and "
		UI_TO_TEXT(BMG_INF_LIMIT) "."
		" The first 4 bytes of a INF1 record is an offset into the"
		" string pool and the remaining bytes the attributes."
		" Maximal " UI_TO_TEXT(BMG_ATTRIB_SIZE)
		" attribute bytes are supported and additional bytes are"
		" assumed to be NULL. All BMG files of MKW have a size of 8." },

  { T_OPT_CO,	"BMG_MID",	"bmg-mid|bmgmid",
		"[mode]",
		"Control the creation of BMG section @MID1@."
		" Suppress the creation, if mode is @OFF@."
		" Force the creation, if mode is @ON@."
		" On mode @AUTO@ (default), the source is analysed for the decision." },

  { T_OPT_CP,	"FORCE_ATTRIB",	"force-attrib|forceattrib",
		"attrib",
		"Forces, that all attributes are set to this vector."
		"\1 See https://szs.wiimm.de/doc/bmg/attrib for the syntax." },

  { T_OPT_CP,	"DEF_ATTRIB",	"def-attrib|defattrib",
		"attrib",
		"Define the default attributes for BMG files."
		" If not set, the default attributes are estimated."
		" On text output, strings with default attributes are printed"
		" without attribute vector."
		" The usual default attribute for MKW is @[1]@."
		"\1 See https://szs.wiimm.de/doc/bmg/attrib for the syntax." },

  { T_OPT_C,	"NO_ATTRIB",	"no-attrib|noattrib",
		0,
		"Suppress the output of any BMG attributes if creating BMG text files." },

  { T_OPT_C,	"X_ESCAPES",	"x-escapes|xescapes",
		0,
		"Use \\x{{} escapes instead of \\z{{} escapes." },

  { T_OPT_C,	"OLD_ESCAPES",	"old-escapes|oldescapes",
		0,
		"To be compatible with v1.43 and earlier,"
		" print 1A escape sequences with total size of >12 bytes"
		" as single words using \\x{{}."
		" Also don't use \\x{{} and \\u{{} with parameter lists." },

  { T_OPT_C,	"SINGLE_LINE",	"1|single-line|singleline",
		0,
		"If set, don't print continuation lines for BMG text output."
		" If set twice, print only single text lines and suppress attributes"
		" like option {--no-attrib} does it." },

  { T_OPT_C,	"NO_BMG_COLORS","no-bmg-colors|nobmgcolors",
		0,
		"If set, suppress the output of '\\c{{color}'"
		" escape sequences for colors definitions in BMG text output"
		" to be compatible with old versions of the tools." },

  { T_OPT_C,	"BMG_COLORS",	"bmg-colors|bmgcolors",
		0,
		"Force output of '\\c{{color}' escape sequences with names,"
		" even for foreign (not MKWii) BMG files." },

  { T_OPT_C,	"NO_BMG_INLINE","no-bmg-inline|nobmginline",
		0,
		"Print BMG attributes as separate line before the message definition"
		" to be compatible with versions until v1.39."
		" The output for the standard value 0x01000000 is always suppressed."
		" This was the default until v1.43." },

  { T_OPT_C,	"EXPORT",	"X|export",
		0,
		"Enable the export mode and create small and machine readable"
		" text files for easy post processing."
		" The option works similar like @-HBB@ for KMP"
		" and like @-HBl1@ for BMG text files." },

  { T_OPT_C,	"NO_PARAM",	"P|no-param|noparam",
		0,
		"Disable parameter support in decoded KMP text files." },

  { T_OPT_CP,	"SORT",		"S|sort",
		"mode",
		"Define a sort mode. Allowed keywords are:"
		" @NONE@ (default), @NAME, OFFSET, U8, PACK, BRRES, BREFT@ and @AUTO@" },

  { T_OPT_GP,	"MAX_FILE_SIZE","M|max-file-size|maxfilesize",
		"size",
		"This security option defines the maximum file size"
		" for input files; larger files are ignored with a warning."
		" The default unit is MiB and the default size is 100 MiB;"
		" that should be enough for standard usage." },

  { T_OPT_GP,	"TRACKS",	"T|tracks",
		"tracks",
		"Define a new track order."
		" A comma separated list in the format @[a=]b@ or @a:=b@ is expected."
		" @'a'@ and @'b'@ are track names or slot numbers."
		" @'a'@ is the destination slot and @'b'@ the assigned track."
		" If @'a'@ is omitted, the next slot is used;"
		" this means @[a=]b,c,d@ is possible."
		"\n "
		" Tracks will only be exchanged so that always all tracks are assigned."
		" But if using @':='@ the new slots are assigned without exchanging"
		" and the old values are lost." },

  { T_OPT_GP,	"ARENAS",	"A|arenas",
		"arenas",
		"Define a new arena order."
		" A comma separated list in the format @[a=]b@ or @a:=b@ is expected."
		" @'a'@ and @'b'@ are arena names or slot numbers."
		" @'a'@ is the destination slot and @'b'@ the assigned arena."
		" If @'a'@ is omitted, the next slot is used;"
		" this means @[a=]b,c,d@ is possible."
		"\n "
		" Arenas will only be exchanged so that always all arenas are assigned."
		" But if using @':='@ the new slots are assigned without exchanging"
		" and the old values are lost." },

  { T_OPT_GP,	"AUTOADD_PATH",	"autoadd-path|autoaddpath",
		"directory",
		"Define an alternative directory as autoadd library."
		" The default is, that the sub-directory called './auto-add/'"
		" of all search paths are used."
		" Use {wszst config -ll} for tests." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"TRANSFORM",	"x|transform",
		"list",
		"Convert image formats."
		" A comma separated list with terms is expected."
		" A term have the syntax @'[SRC=]DEST'@."
		" It means: convert @SRC@ format into @DEST@ format."
		" If @SRC=@ is not set, the destination format is used"
		" for all source formats. This makes only sense as last rule."
		" If @DEST@ is empty no transformation is done for the source."
		" Examples: '@TPL@' or '@GRAY@' or '@BTI.IA8@' or '@TPL.I4+I8=IA8,TEX=,CMPR@'"
		"\n "
		" @SRC@ and @DEST@ are tuples of file, image and palette formats"
		" and of 2 more attributes. All parts are optional and separated"
		" by points, the order is irrelevant."
		"\n "
		" Allowed file formats are: @TPL, TEX, BTI, BREFT-IMG (=BTIMG),"
		" PNG@ (import)."
		" Allowed image formats are: @I4, I8, IA4, IA8, RGB565 (=R565),"
		" RGB5A3 (=R3), RGBA32 (=R32), C4, C8, C14X2, CMPR@."
		" Allowed palette formats are: @P-IA8 (=P8), P-RGB565 (=P565),"
		" P-RGB5A3 (=P3)@."
		" Attributtes are @PALETTE@, @NO-PALETTE@, @GRAY@, @COLOR@,"
		" @ALPHA@ and @NO-ALPHA@."
		"\n "
		" All terms are managed as list."
		" For each graphic the list is iterated until the first SRC tuple"
		" of a term matches the graphic."
		" Then the destination tuple is used for the transformation."
		"\n "
		" The command {wimgt TEST} prints a clearly arranged readable table"
		" as result of this option." },

  { T_OPT_C,	"STRIP",	"strip",
		0,
		"If creating a PNG file,"
		" don't add comments about creator and source formats." },


  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { H_OPT_G,	"UTF_8",	"utf-8|utf8",
		0,
		"Enables UTF-8 support for filenames (default)." },

  { H_OPT_G,	"NO_UTF_8",	"no-utf-8|no-utf8|noutf8",
		0,
		"Disables UTF-8 support for filenames." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_GM,	"TEST",		"t|test",
		0,
		"Run in test mode, modify nothing."
		"\n"
		">>> USE THIS OPTION IF UNSURE! <<<" },

  { T_OPT_GM,	"FORCE",	"force",
		0,
		"The tools analyse input files and reject files with invalid data"
		" structures. If --force is set, little repairable issues are ignored"
		" and a warning is printed."
		"\n"
		"Option {--kmp force} does the same, but only for KMP files."
		"\n"
		">>> THE RESULT MAY BE INVALID OUTPUT FILES! <<<" },

  { T_OPT_GO,	"REPAIR_MAGICS",	"repair-magics|repairmagics",
		"[=mode]",
		"Control the reparing of missing or wrong magics of files"
		" and sub-files by analysing file path and file extension."
		" @MODE@ is one of @OFF@ (default), @ANALYZE@"
		" or @REPAIR@ (default, if set without mode)."
		" @OFF@ disables this feature, @ANALYZE@ enables it for analysis only,"
		" and @REPAIR@ repairs all magics while reading the files."
		" Keyword @FNAME@ can be appended (separated by a comma)"
		" to give filename scanning the highest priority."
		"\n"
		">>> THE RESULT MAY BE INVALID OUTPUT FILES! <<<" },

  { T_OPT_GP,	"TINY",		"tiny",
		"mode",
		"Set tiny mode to a level between 0 and 7."
		"\1 See https://szs.wiimm.de/opt/tiny for details." },

  { T_OPT_G,	"YAZ0",		"yaz0",
		0,
		"If creating a compressed file, force Yaz0 compression."
		" Yaz0 compression is the default,"
		" if no compression method is defined by option or setup file." },

  { T_OPT_G,	"YAZ1",		"yaz1",
		0,
		"If creating a compressed file, force Yaz1 compression."
		" The difference to Yaz0 is only the magic,"
		" which is set to 'Yaz1'." },

  { T_OPT_G,	"XYZ",		"xyz",
		0,
		"If creating a compressed file,"
		" force XYZ compression (an experimental Yaz0 variant)." },

  { T_OPT_G,	"BZ",		"bz",
		0,
		"If creating a compressed file,"
		" force BZ compression (a BZIP2 variant)." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_G,	"SH",		"sh",
		0,
		"Dump info using SH syntax."
		" Use {--var name} as prefix." },

  { T_OPT_G,	"BASH",		"bash",
		0,
		"Dump info using BASH syntax."
		" Use {--var name} as prefix." },

  { T_OPT_G,	"JSON",		"json",
		0,
		"Dump info using JSON syntax."
		" Options {--var} and {--array} are ignored." },

  { T_OPT_G,	"PHP",		"php",
		0,
		"Dump infos using PHP syntax."
		" Use {--var name} as variable name of the resulting object." },

  { T_OPT_G,	"MAKEDOC",	"makedoc",
		0,
		"Dump infos using MakeDoc syntax."
		" Use {--var name} as variable name of the resulting map." },

  { T_OPT_GP,	"VAR",		"var",
		"varname",
		"Use @VARNAME@ as variable name or prefix on script output."
		" The default value is 'res' for variables and 'res_' as prefix." },

  { T_OPT_G,	"ARRAY",	"array",
		0,
		"Use an array on script output if possible."
		" If arrays are not supported, then append a '_#' suffix with an zero"
		" based index to the variable name defined by {--var name}."
		" In this case, only one output file is created." },

  { T_OPT_GP,	"AVAR",		"avar",
		"varname",
		"Shortcut for: {--array --var name}" },

  { T_OPT_GP,	"CASE",		"case",
		"case",
		"@CASE@ is one of @LOWER@, @AUTO@ (default) or @UPPER@."
		"If @LOWER@ or @UPPER@ are set, then the lower/upper case"
		" of the variable names is adjusted accordingly." },

  { H_OPT_CP,	"FMODES",	"fmodes|feature-modes|featuremodes",
		"list",
		"--fmodes is an abbreviation for --feature-modes and declares an output filter."
		" @LIST@ is a keyword list. The keywords are:"
		" @SECTION, VISUAL, GAMEPLAY, BATTLE, RACING, TIMETRIAL, OFFLINE@ and @ONLINE@."
		" A feature is only printed if *all* set keywords match."
		"\n "
		" Each keyword can also be used with a minus as prefix to define"
		" a second condition. In this case the feature is excluded"
		" from printing if *any* set keywords match." },

  { T_OPT_C,	"INSTALL",	"install",
		0,
		"Manage configuration for an installation." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { H_OPT_CP,	"ANALYZE",	"analyze|analyse",
		"outfile",
		"Run in analyze mode and write results to the entered file." },

  { H_OPT_CP,	"ANALYZE_MODE",	"analyze-mode|ana-mode|anamode|amd",
		"modelist",
		"Comma separated list with objects to analyse."
		" Objects are @BRRES, MDL0, PAT0, TEX0, BREFF, BREFT, TPL,"
		" KCL, KMP, GOBJ, KTPT, MKWANA@"
		" and @ALL@ for all (=default)."
		" @--ana-mode@ is a short cut." },

 #if OPT_OLD_NEW
  { H_OPT_GM,	"OLD",		"OLD",
		0,
		"Developer option:"
		" Cancel --NEW and use old implementation if available." },

  { H_OPT_GM,	"STD",		"STD",
		0,
		"Developer option: Cancel --OLD and --NEW"
		" and use standard implementation (default)." },

  { H_OPT_GM,	"NEW",		"NEW",
		0,
		"Developer option:"
		" Cancel --OLD and use new implementation if available." },
 #endif

  { H_OPT_GP,	"EXTRACT",	"EXTRACT",
		"dir",
		"Create the directory and extract different objects"
		" in different formats at the moment when they are seen."
		" This is a development helper option." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"SOURCE",	"s|source|src",
		"path",
		"Define a secondary source path (directory/file)."
		"\n "
		" The path may contain escape sequences:"
		" @%Q@ is replaced by the fully qualified primary source name."
		" @%P@ and @%F@ are replaced by the primary source path"
		" or by the filename. @%N@ and @%E@ are replaced by primary source"
		" filename without extension or by the source extension."
		" Finally, @%T@ is replaced by the default extension"
		" of a virtual destination format."
		"\n "
		" A @'?'@ direct behind @'%'@ in @%E@ and @%T@ conversions"
		" avoid that the same extension is used twice in row."
		"\1 Read https://szs.wiimm.de/opt/source for more details." },


  { T_OPT_CP,	"REFERENCE",	"ref|reference",
		"path",
		"Define a reference SZS." },

  { T_OPT_CP,   "DEST",		"d|dest",
		"path",
		"Define a destination path (directory/file)."
		" The destination @-@ means: write to standard output."
		"\n "
		" The path may contain escape sequences:"
		" @%Q@ is replaced by the fully qualified source name."
		" @%P@ and @%F@ are replaced by the source path or by the filename."
		" @%N@ and @%E@ are replaced by source filename without extension"
		" or by the source extension."
		" Finally, @%T@ is replaced by the default extension"
		" of the destination format."
		"\n "
		" A @'?'@ direct behind @'%'@ in @%E@ and @%T@ conversions"
		" avoid that the same extension is used twice in row."
		"\1 Read https://szs.wiimm.de/opt/dest for more details." },

  { T_OPT_CP,	"DEST2",	"D|DEST",
		"path",
		"Like {--dest}, but create the directory path automatically." },

  { T_OPT_CP,	"ESC",		"E|esc",
		"char",
		"Define an alternative escape character for destination files."
		" The default is '%'."
		" For Windows batch files it is a good choice to set @'-E$$'@." },

  { T_OPT_C,	"OVERWRITE",	"o|overwrite",
		0,
		"Overwrite already existing files without warning"
		" and ignore option {--number}." },

  { T_OPT_C,	"NUMBER",	"number|num",
		0,
		"If a file already exist, append a number directly before"
		" the file extension to make the filename unique."
		" If other numbered files already exist (ignoring case),"
		" use the maximum existing index+1."
		" @--num@ is a short cut." },

  { T_OPT_C,	"REMOVE_SRC",	"remove-src|removesrc|rm-src|rmsrc",
		0,
		"Remove the source file after successful operation."
		" @--rm-src@ is a short cut." },

  { T_OPT_C,	"REMOVE_DEST",	"r|remove-dest|removedest|rm-dest|rmdest|remove",
		0,
		"Remove already existing files before creating it."
		" If set, {--overwrite} is ignored."
		" @--rm-dest@ is a short cut." },

  { T_OPT_C,	"UPDATE",	"u|update",
		0,
		"Update only existing files and don't create new files."
		" If set, {--overwrite} and {--remove-dest} are ignored." },

  { T_OPT_C,	"PRESERVE",	"p|preserve",
		0,
		"Preserve file times (atime+mtime) while converting or copying files." },

  { T_OPT_C,	"IGNORE",	"i|ignore",
		0,
		"Ignore non existing source files without warning." },

  { T_OPT_C,	"IGNORE_SETUP",	"ignore-setup|ignoresetup",
		0,
		"Ignore hidden setup file '@" SZS_SETUP_FILE "@'."
		" This file is used to determine some default settings and to find"
		" out sub archives and files to encode."
		" Use this option only if you know what you do." },

  { T_OPT_C,	"PURGE",	"purge",
		0,
		"Remove repeated files with same checksum." },

  { T_OPT_CP,	"ALIGN_U8",	"align-u8|alignu8",
		"size",
		"Define an align value for the sub files in U8 archives."
		" The value must be a power of 2 and the default value is 32 (0x20)"
		" This value is only relevant if creating or normalizing U8 archives." },

  { T_OPT_CP,	"ALIGN_PACK",	"align-pack|alignpack",
		"size",
		"Define an align value for the sub files in PACK archives."
		" The value must be a power of 2 and the default value is 32 (0x20)"
		" This value is only relevant if creating or normalizing OACK archives." },

  { T_OPT_CP,	"ALIGN_BRRES",	"align-brres|alignbrres",
		"size",
		"Define an align value for the sub files in BRRES archives."
		" The value must be a power of 2 and the default value is 4"
		" This value is only relevant if creating or normalizing BRRES archives." },

  { T_OPT_CP,	"ALIGN_BREFF",	"align-breff|alignbreff",
		"size",
		"Define an align value for the sub files in BREFF archives."
		" The value must be a power of 2 and the default value is 4"
		" This value is only relevant if creating or normalizing BREFF archives." },

  { T_OPT_CP,	"ALIGN_BREFT",	"align-breft|alignbreft",
		"size",
		"Define an align value for the sub files in BREFT archives."
		" The value must be a power of 2 and the default value is 0x20"
		" This value is only relevant if creating or normalizing BREFT archives." },

  { T_OPT_CP,	"ALIGN",	"align",
		"size",
		"Define an align value for the sub files in all archives."
		" This is a short cut for"
		" {--align-u8=size --align-pack=size"
		" --align-brres=size --align-breff=size --align-breft=size}." },

  { T_OPT_C,	"ENCODE_ALL",	"encode-all|encodeall|eall",
		0,
		"Encode all files. Normally a file is only encoded, if the source"
		" is younger than the destination or the destination is missing."
		" Only files in the @[encode]@ section of the file '@" SZS_SETUP_FILE "@'"
		" are candidates for encoding."
		" @--eall@ is a short cut."
		" Option {--no-encode} takes precedence over {--encode-all}." },

  { T_OPT_C,	"ENCODE_IMG",	"encode-img|encodeimg|eimg",
		0,
		"Encode all image files. Normally a image file is only encoded, if the source"
		" is younger than the destination or the destination is missing."
		" Only files in the @[encode]@ section of the file '@" SZS_SETUP_FILE "@'"
		" are candidates for encoding."
		" @--eimg@ is a short cut."
		" If set, option {--no-encode} is ignored for images." },

  { T_OPT_C,	"NO_ENCODE",	"no-encode|noencode|noenc",
		0,
		"Do not encode files before creating an archive."
		" @--noenc@ is a short cut."
		" Option {--no-encode} takes precedence over {--encode-all},"
		" but {--encode-img} takes precedence for images." },

  { T_OPT_C,	"NO_RECURSE",	"no-recurse|norecurse|norec",
		0,
		"Do not create archives recursively."
		" Normally files in the @[create]@ section of the file"
		" '@" SZS_SETUP_FILE "@' are created first."
		" @--norec@ is a short cut." },

  { T_OPT_C,	"AUTO_ADD",	"auto-add|autoadd|aadd",
		0,
		"Analyze the KMP (if exist) and add missing BRRES, BREFF, BREFT,"
		" BRASD and KCL files automatically, if the tool can find the files"
		" in any sub directory named 'auto-add'."
		" This sub directory is searched in all directories"
		" of @'SEARCH_PATH'@; use command @{wszst AUTOADD}@ to view them."
		" Command {wszst AUTOADD} may also create an auto-add database."
		" @--aadd@ is a short cut." },

  { T_OPT_C,	"NO_ECHO",	"no-echo|noecho",
		0,
		"Suppress output of @@ECHO commands while scanning text files." },

  { T_OPT_C,	"NO_CHECK",	"N|no-check|nocheck",
		0,
		"Do not make plausibility checks for KCL and KMP files." },

  { T_OPT_CO,	"PT_DIR",	"pt-dir|ptdir|pdir",
		"[=mode]",
		"Set one of the `point directory' modes @REMOVE@ (=@0@),"
		" @FORCE@ (=@1@) or @AUTO@ (default)."
		" If --pt-dir is used without parameter, then @FORCE@ is assumed."
		"\n "
		" This option is only relevant if creating an U8 archive."
		" It decides, if a special directory with name '.'"
		" will be added as base for all other files." },

  { T_OPT_C,	"RM_AIPARAM",	"rm-aiparam|rmaiparam|rmai",
		0,
		"Remove the directory AIParam and all files from the archive."
		" This option has only impact, if creating a new archive or"
		" normalizing an existing archive."
		" @--rmai@ is a short cut." },

  { T_OPT_C,	"U8",		"u8",
		0,
		"Create an U8 archive, if possible."
		" If the source is a WU8 archive, convert it to an U8 archive."
		" This is the default if the setup file '@" SZS_SETUP_FILE "@'"
		" has no other definition."
		" The default compression mode is set to Yaz0 (see {--yaz0})." },

  { T_OPT_C,	"SZS",		"szs",
		0,
		"Short cut for {--u8 --yaz0}:"
		" Create an U8 archive, if possible,"
		" and set the compression method to Yaz0." },

  { T_OPT_C,	"WU8",		"wu8",
		0,
		"Create a WU8 archive, if possible."
		" If the source is an U8 archive, convert it to a WU8 archive." },

  { H_OPT_C,	"XWU8",		"xwu8",
		0,
		"Short cut for {--wu8 --xyz}:"
		" Create a WU8 archive, if possible,"
		" and set the compression method to XYZ." },

  { T_OPT_C,	"WBZ",		"wbz",
		0,
		"Short cut for {--wu8 --bz}:"
		" Create a WU8 archive, if possible,"
		" and set the compression method to BZ." },

  { T_OPT_C,	"PACK",		"pack",
		0,
		"Create a PACK archive." },

  { T_OPT_C,	"BRRES",	"brres|bres",
		0,
		"Create a BRRES archive." },

  { T_OPT_C,	"BREFF",	"breff",
		0,
		"Create a BREFF archive." },

  { T_OPT_C,	"BREFT",	"breft",
		0,
		"Create a BREFT archive." },

  { T_OPT_C,	"NO_COMPRESS",	"no-compress|nocompress|noc",
		0,
		"Create an uncompressed file instead of a compressed SZS file."
		" The default destination changes to @'%P/%N%T'@."
		" @--noc@ is a short cut." },

  { T_OPT_CP,	"COMPRESS",	"C|compr|compress",
		"level",
		"Define a compression level between @0..9@:"
		" @0@ (or @NOCHUNKS@) disables finding chunks,"
		" @1@ (or @FAST@) for the fastest standard compression and"
		" @9@ (or @BEST@) for the best (=default) compression."
		" @10@ (or @ULTRA@) and @100-150@ are special time-consuming"
		" compression modes."
		" They are dedicated to competitions with size limitations."
		" The special value @UNCOMPRESSED@ acts like option {--no-compress}."
		"\n "
		" Because of many repeated data, the best bz-compression mode varies."
		" Therefor the levels @TRY2..TRY5@ (or short @T2..T5@) are defined"
		" to find the best compression mode with testing the first"
		" N modes of 9, 1, 8, 2, 5. @TRY2@ is used for normalizing."
		" For non bz-compression, @TRY*@ are the same as @BEST@."
		"\n "
		" Option {--norm} takes precedence over {--compr}"
		" and sets the compression level to @TRY2@ (@BEST@)."
		"\n "
		" For more modes and details type {wszst -C list}."
		" To force colorized output type {wszst -C clist}." },

  { T_OPT_C,	"FAST",		"fast",
		0,
		"Short cut for {--compr=fast}: Set the fastest real compression."
		" It also overrides compression rates set by {--norm} or {--compr}." },

  { T_OPT_C,	"NORM",		"n|norm",
		0,
		"The uncompressed data will be normalized."
		" See command {NORMALIZE} for more details." },

  { T_OPT_C,	"LINKS",	"links",
		0,
		"Support hardlinks while creating U8 and WU8 archives."
		" Keep hardlinks if normalizing U8 and WU8 archives."
		" On extracting, search hardlinks in every archive and try"
		" to create hardlinks at local file system." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"BASEDIR",	"basedir|bdir",
		"directory",
		"Extract only files from archives, that are below the DIRECTORY."
		" Leading points and slashes and trailing slahes are removed"
		" from the parameter. --bdir is short cut for --basedir." },

  { T_OPT_CO,	"RECURSE",	"R|recurse",
		"[=level]",
		"If a extracted file is a known and supported archive"
		", than extract it recursively until the entered level is reached."
		" Level 0 (=@NONE@) deactivates this."
		" If --recurse is used without parameter,"
		" then @UNLIMITED@ is assumed."
		"\n "
		" -R does'nt accept a parameter"
		" and is a short cut for {--recurse=unlimited}." },

  { T_OPT_C,	"EXT",	"e|ext",
		0,
		"For BRRES archives only:"
		" If a file is extracted, add a handy file extension like '.mdl'"
		" to the file name. If set twice and a textual file magic is"
		" available, use the file magic in lower case instead like '.mdl0'." },

  { T_OPT_C,	"DECODE",	"decode|dec",
		0,
		"If a extracted file is known and can be decoded, do it."
		" @--dec@ is a short cut." },

  { T_OPT_C,	"MIPMAPS",	"mipmaps|mm",
		0,
		"If reading a source, scan for mipmaps and load them too."
		" For PNG files, files named @NAME.mm#.EXT@ with #=1.. are searched."
		" This is the default. @--mm@ is a short cut." },

  { T_OPT_C,	"NO_MIPMAPS",	"no-mipmaps|nomipmaps|no-mm|nomm",
		0,
		"If reading a source, ignore mipmaps."
		" This disables not the creation of mipmaps (see {-n-mipmaps})."
		" @--no-mm@ is a short cut." },

  { T_OPT_CP,	"N_MIPMAPS",	"n-mipmaps|nmipmaps|n-mm|nmm",
		"num",
		"Force the number of mipmaps. Values between 0 and 20 are allowed."
		" Value @OFF@ (default) disables this option."
		" If enabled, options @--max-mipmaps@ and @--mipmap-size@ are ignored."
		" The creation of mipmaps stops if the width or the height"
		" of the mipmap becomes 0. @--n-mm@ is a short cut." },

  { T_OPT_CP,	"MAX_MIPMAPS",	"max-mipmaps|maxmipmaps|max-mm|maxmm",
		"num",
		"Control the automatic mipmap creation."
		" Values between 0 and 20 are allowed and the default is 4."
		" The mipmap creation for an image stops, if the set number is reached."
		" This option is ignored, if @--n-mipmaps@ is enabled."
		" @--max-mm@ is a short cut." },

  { T_OPT_CP,	"MIPMAP_SIZE",	"mipmap-size|mipmapsize|mm-size|mmsize",
		"num",
		"Define the minimal width and minimal height of a generated mipmap."
		" Any value >0 is allowed. The default is 8."
		" This option is ignored, if @--n-mipmaps@ is enabled."
		" @--mm-size@ is a short cut." },

  { T_OPT_C,	"FAST_MIPMAPS",	"fast-mipmaps|fastmipmaps|fast-mm|fastmm",
		0,
		"If resizing an image, use the old fast resize algorithm"
		" (default until v1.64a) instead of the new smart one."
		" Both algorithms differ only, when creating a mipmap for images"
		" with odd width or odd height."
		" The old algorithm is faster (factor 2-4) than the default algorithm,"
		" but the new algorithm creates better resized images."
		"\1 See https://szs.wiimm.de/doc/img/resize for details."
		"\3 @--fast-mm@ is a short cut." },

  { T_OPT_CP,	"CMPR_DEFAULT",	"cmpr-default",
		"rgb1[,rgb2]",
		"Define 2 colors for the case, that all 16 pixels of a CMPR block"
		" are transparent. The default is to calculate an average color"
		" of all transparent pixels. Before v2.04 white was used."
		"\n "
		" 2 RGB values (hex values with 6 digits each) are expected as parameter."
		" If optional @RGB2@ is missed, a copy of @RGB1@ is used."
		" As first step, both colors are converted to RGB565 colors @C1@ and @C2@"
		" and sorted, so that @C1<C2@ (mandatory for CMPR)."
		" If @C1@ and @C2@ are identical, the least significant bit of green"
		" is cleared for @C1@ and set for @C2@."
		"\n "
		" Use parameter '@0@' to force black, '@-1@' to force white,"
		" or '@-@' to reset to auto calculation." },

  { T_OPT_C,	"CUT",		"cut",
		0,
		"Cut different non archive files into smaller peaces"
		" (logical units like headers, groups, sections) or list these"
		" peaces as single sub files for a more detailed analysis."
		" Supported file types are @BMG, BREFT-IMG, KMP, PAT, TEX, TPL,"
		" BRRES@ sub files and more." },

  { T_OPT_CM,	"ALL",		"a|all",
		0,
		"Extract and decode all known and supported files recursively."
		" This is a short cut for {--recurse --decode --mipmaps}."
		" If entered twice, option {--cut} is included too." },

  { T_OPT_C,	"RAW",		"raw",
		0,
		"Extract BRRES sub file in raw mode and do not adjust"
		" string pointers and other offsets." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"SECTIONS",	"sections",
		0,
		"Print in machine readable sections and parameter lines." },
		//"\1 Read https://szs.wiimm.de/opt/sections for more details." },

  //
  //---------- wszst GROUP TRANSFORM ----------

  { T_GRP_BEG,	"TRANSFORM",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },
  { T_COPT,	"SCALE",	0,0,0 },
  { T_COPT,	"SHIFT",	0,0,0 },
  { T_COPT,	"XSS",		0,0,0 },
  { T_COPT,	"YSS",		0,0,0 },
  { T_COPT,	"ZSS",		0,0,0 },
  { T_COPT,	"ROT",		0,0,0 },
  { T_COPT,	"XROT",		0,0,0 },
  { T_COPT,	"YROT",		0,0,0 },
  { T_COPT,	"ZROT",		0,0,0 },
  { T_COPT,	"YPOS",		0,0,0 },
  { T_COPT,	"TRANSLATE",	0,0,0 },
  { T_COPT,	"NULL",		0,0,0 },
  { T_COPT,	"NEXT",		0,0,0 },
  { T_COPT,	"ASCALE",	0,0,0 },
  { T_COPT,	"AROT",		0,0,0 },
  { T_COPT,	"TFORM_SCRIPT",	0,0,0 },

  //---------- wszst GROUP TRANSFORM2 ----------

  { T_GRP_BEG,	"TRANSFORM2",	0,0,0 },

  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },
  { T_COPT,	"RM_GOBJ",	0,0,0 },
  { T_COPT,	"SLOT",		0,0,0 },
  { T_COPT,	"LOAD_KCL",	0,0,0 },
  { T_COPT,	"KCL",		0,0,0 },
  { T_COPT,	"KCL_FLAG",	0,0,0 },
  { T_COPT,	"KCL_SCRIPT",	0,0,0 },
  { T_COPT,	"TRI_AREA",	0,0,0 },
  { T_COPT,	"TRI_HEIGHT",	0,0,0 },
  { T_COPT,	"KMP",		0,0,0 },
  { T_COPT,	"SPEED_MOD",	0,0,0 },
  { T_COPT,	"KTPT2",	0,0,0 },
  { T_COPT,	"TFORM_KMP",	0,0,0 },
  { T_COPT,	"REPAIR_XPF",	0,0,0 },
  { T_COPT,	"MDL",		0,0,0 },
  { T_COPT,	"MINIMAP",	0,0,0 },
  { T_COPT,	"PAT",		0,0,0 },
  { T_COPT,	"PATCH_FILE",	0,0,0 },
  { T_COPT,	"LT_CLEAR",	0,0,0 },
  { T_COPT,	"LT_ONLINE",	0,0,0 },
  { T_COPT,	"LT_N_PLAYERS",	0,0,0 },
  { T_COPT,	"LT_COND_BIT",	0,0,0 },
  { T_COPT,	"LT_GAME_MODE",	0,0,0 },
  { T_COPT,	"LT_ENGINE",	0,0,0 },
  { T_COPT,	"LT_RANDOM",	0,0,0 },
  { T_COPT,	"LEX_PURGE",	0,0,0 },
  { T_COPT,	"LEX_FEATURES",	0,0,0 },
  { T_COPT,	"LEX_RM_FEAT",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },
  { T_COPT,	"LE_MENU",	0,0,0 },
  { T_COPT,	"TITLE_SCREEN",	0,0,0 },
  { T_COPT,	"CUP_ICONS",	0,0,0 },

  //---------- wszst GROUP DEST ----------

  { T_GRP_BEG,	"DEST",		0,0,0 },

  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"PRESERVE",	0,0,0 },
  { T_COPT,	"UPDATE",	0,0,0 },
  { T_COPT,	"OVERWRITE",	0,0,0 },
  { T_COPT,	"NUMBER",	0,0,0 },
  { T_COPT,	"REMOVE_DEST",	0,0,0 },

  //---------- wszst GROUP FFORMAT1 ----------

  { T_GRP_BEG,	"FFORMAT1",	0,0,0 },

  { T_COPT,	"U8",		0,0,0 },
  { T_COPT,	"WU8",		0,0,0 },
  { T_COPT,	"YAZ0",		0,0,0 },
  { T_COPT,	"YAZ1",		0,0,0 },
  { H_COPT,	"XYZ",		0,0,0 },
  { T_COPT,	"BZ",		0,0,0 },
  { T_COPT,	"SZS",		0,0,0 },
  { H_COPT,	"XWU8",		0,0,0 },
  { T_COPT,	"WBZ",		0,0,0 },

  //---------- wszst GROUP FFORMAT2 ----------

  { T_GRP_BEG,	"FFORMAT2",	0,0,0 },

  { T_COPT,	"U8",		0,0,0 },
  { T_COPT,	"WU8",		0,0,0 },
  { T_COPT,	"PACK",		0,0,0 },
  { T_COPT,	"BRRES",	0,0,0 },
  { T_COPT,	"BREFF",	0,0,0 },
  { T_COPT,	"BREFT",	0,0,0 },
  { T_COPT,	"YAZ0",		0,0,0 },
  { T_COPT,	"YAZ1",		0,0,0 },
  { H_COPT,	"XYZ",		0,0,0 },
  { T_COPT,	"BZ",		0,0,0 },
  { T_COPT,	"SZS",		0,0,0 },
  { H_COPT,	"XWU8",		0,0,0 },
  { T_COPT,	"WBZ",		0,0,0 },

  //
  //---------- wszst GROUP SCRIPT ----------

  { T_GRP_BEG,	"SCRIPT",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,
	"Dump info using configuration or section syntax."
	" Use {--var name} as prefix." },
  { T_COPT,	"SH",		0,0,0 },
  { T_COPT,	"BASH",		0,0,0 },
  { T_COPT,	"JSON",		0,0,0 },
  { T_COPT,	"PHP",		0,0,0 },
  { T_COPT,	"MAKEDOC",	0,0,0 },
  { T_COPT,	"VAR",		0,0,0 },
  { T_COPT,	"ARRAY",	0,0,0 },
  { T_COPT,	"AVAR",		0,0,0 },
  { T_COPT,	"CASE",		0,0,0 },

  //---------- wszst GROUP ALIGN ----------

  { T_GRP_BEG,	"ALIGN",	0,0,0 },

  { T_COPT,	"ALIGN_U8",	0,0,0 },
  { T_COPT,	"ALIGN_PACK",	0,0,0 },
  { T_COPT,	"ALIGN_BRRES",	0,0,0 },
  { T_COPT,	"ALIGN_BREFF",	0,0,0 },
  { T_COPT,	"ALIGN_BREFT",	0,0,0 },
  { T_COPT,	"ALIGN",	0,0,0 },

  //---------- wszst GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",	0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0, NO_HEAD_SYNTAX },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"DELTA",	0,0,0 },
  { T_COPT,	"DIFF",		0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },
  { T_COPT,	"NO_PARAM",	0,0,0 },

  //---------- wszst GROUP BMG ----------

  { T_GRP_BEG,	"BMG",	0,0,0 },

  { T_COPT,	"BMG_ENDIAN",	0,0,0 },
  { T_COPT,	"BMG_ENCODING",	0,0,0 },
  { T_COPT,	"BMG_INF_SIZE",	0,0,0 },
  { T_COPT,	"BMG_MID",	0,0,0 },
  { T_COPT,	"FORCE_ATTRIB",	0,0,0 },
  { T_COPT,	"DEF_ATTRIB",	0,0,0 },
  { T_COPT,	"NO_ATTRIB",	0,0,0 },
  { T_COPT,	"X_ESCAPES",	0,0,0 },
  { T_COPT,	"OLD_ESCAPES",	0,0,0 },
  { T_COPT,	"SINGLE_LINE",	0,0,0 },
  { T_COPT,	"NO_BMG_COLORS",0,0,0 },
  { T_COPT,	"BMG_COLORS",	0,0,0 },
  { T_COPT,	"NO_BMG_INLINE",0,0,0 },
  { T_COPT,	"LONG",		0,0, BMG_LONG_TEXT },
  { T_COPT,	"EXPORT",	0,0,0 },

  //---------- wszst GROUP ENCODE ----------

  { T_GRP_BEG,	"ENCODE",	0,0,0 },

  { T_COPT,	"ENCODE_ALL",	0,0,0 },
  { T_COPT,	"ENCODE_IMG",	0,0,0 },
  { T_COPT,	"ALL",		0,0,
		" Encode all files."
		" This is a short cut for {--encode-all}"
		" (and more options in the future)." },
  { T_COPT,	"NO_ENCODE",	0,0,0 },
  { T_COPT,	"NO_RECURSE",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"TRANSFORM",	0,0,0 },
  { T_COPT,	"STRIP",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },

  //---------- wszst GROUP CREATE ----------

  { T_GRP_BEG,	"CREATE",	0,0,0 },

  { T_COPT,	"IGNORE_SETUP",	0,0,0 },
  { T_COPY_GRP,	"FFORMAT2",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"PT_DIR",	0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPT,	"RM_AIPARAM",	0,0,0 },
  { T_COPY_GRP,	"ALIGN",	0,0,0 },
  { T_COPT,	"LOGGING",	0,0,0 },
  { T_COPT,	"EXT_ERRORS",	0,0,0 },
  { T_COPT,	"TIMING",	0,0,0 },
  { T_COPY_GRP,	"ENCODE",	0,0,0 },
  { T_COPT,	"AUTO_ADD",	0,0,0 },
  { T_COPT,	"MIPMAPS",	0,0,0 },
  { T_COPT,	"NO_MIPMAPS",	0,0,0 },
  { T_COPT,	"N_MIPMAPS",	0,0,0 },
  { T_COPT,	"MAX_MIPMAPS",	0,0,0 },
  { T_COPT,	"MIPMAP_SIZE",	0,0,0 },
  { T_COPT,	"FAST_MIPMAPS",	0,0,0 },
  { T_COPT,	"CMPR_DEFAULT",	0,0,0 },

  //---------- wszst GROUP NORM ----------

  { T_GRP_BEG,	"NORM",	0,0,0 },

  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPY_GRP,	"ALIGN",	0,0,0 },
  { T_COPT,	"PT_DIR",	0,0,0 },
  { T_COPT,	"RM_AIPARAM",	0,0,0 },
  { T_COPT,	"AUTO_ADD",	0,0,0 },

  //---------- wszst GROUP EXTRACT ----------

  { T_GRP_BEG,	"EXTRACT",	0,0,0 },

  { T_COPT,	"BASEDIR",	0,0,0 },
  { T_COPT,	"RECURSE",	0,0,0 },
  { T_COPT,	"EXT",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPT,	"DECODE",	0,0,0 },
  { T_COPT,	"MIPMAPS",	0,0,0 },
  { T_COPT,	"NO_MIPMAPS",	0,0,0 },
  { T_COPT,	"FAST_MIPMAPS",	0,0,0 },
  { T_COPT,	"CMPR_DEFAULT",	0,0,0 },
  { T_COPT,	"CUT",		0,0,0 },
  { T_COPT,	"ALL",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"RAW",		0,0,0 },


  //
  //---------- COMMAND wszst VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format (like --sections without section header)."
	" If set twice, append developing settings." },

  //---------- COMMAND wszst HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wszst CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"INSTALL",	0,0,0 },
  { T_COPT,	"AUTOADD_PATH",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"If printed as sections, exclude all comments."
	" Otherwise suppress configuration search list." },
  { T_COPT,	"LONG",		0,0, 
	CONFIG_LONG
	" This option is ignored, if any script output is enabled." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"SCRIPT",	0,0,0 },

  //---------- COMMAND wszst INSTALL ----------

  { T_CMD_BEG,	"INSTALL",	0,0,0 },

  //---------- COMMAND wszst ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wszst EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wszst TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },

  { T_COPT,	"VERBOSE",	0,0,
	"Print the matrices of transformations." },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wszst COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wszst ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wszst FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wszst UI_CHECK ----------

  { T_CMD_BEG,	"UI_CHECK",	0,0,0 },

  { T_COPT,	"LONG",		0,0,
	"Print the result as technical list." },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wszst FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wszst BRSUB ----------

  { T_CMD_BEG,	"BRSUB",	0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"If set, print hints about unusual versions too."
	" If set twice, print a status line for each found BRRES sub file." },

  //---------- COMMAND wszst SYMBOLS ----------

  { T_CMD_BEG,	"SYMBOLS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wszst FUNCTIONS ----------

  { T_CMD_BEG,	"FUNCTIONS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, FUNCTION_BRIEF },
  { T_COPT,	"LONG",		0,0, FUNCTION_LONG },

  //---------- COMMAND wszst CALCULATE ----------

  { T_CMD_BEG,	"CALCULATE",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },

  //---------- COMMAND wszst MATRIX ----------

  { T_CMD_BEG,	"MATRIX",	0,0,0 },

  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"If set once, suppress vector dumps for transformation steps"
		" and ignore option --long."
		" If set twice, suppress vector dump for final transformation too." },
  { T_COPT,	"LONG",		0,0,
		"If set once, print inverse matrix for the final transformation."
		" If set twice, print inverse matrix for all transformations." },
  { T_COPT,	"QUIET",	0,0,
		"Option --quiet is dedicated to automatic tests comparing"
		" the old and new transformation algorithms."
		" If set once, the tables are only printed, if the results differ."
		" If set twice, nothing is printed at all"
		" and only the exit status will inform about the results." },

  //---------- COMMAND wszst FLOAT ----------

  { T_CMD_BEG,	"FLOAT",	0,0,0 },
  { T_COPT,	"ROUND",	0,0,0 },

  //---------- COMMAND wszst VR_CALC ----------

  { T_CMD_BEG,	"VR_CALC",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress header and print only results for @VR_winner-VR_looser@." },

  //---------- COMMAND wszst VR_RACE ----------

  { T_CMD_BEG,	"VR_RACE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
		"Print full matrix of delta values." },

  //---------- COMMAND wszst AUTOADD ----------

  { T_CMD_BEG,	"AUTOADD",	0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"QUIET",	0,0,
	" If set, suppress logging." },
  { T_COPT,	"VERBOSE",	0,0,
	"If set, list all missed and invalid files of the current"
	" auto-add archive after the adding operation."
	" If set twice, then additionally list all already"
	" existing files in the current auto-add archive." },
  { T_COPT,	"DEST",		0,0,
	"Define a destination directory as current auto-add archive."
	" The default is the standard auto-add path."
	" Try {AUTOADD} without sources to view the path." },
  { T_COPT,	"DEST2",	0,0,0 },
  { T_COPT,	"OVERWRITE",	0,0,
	"Overwrite already existing files instead"
	" of only adding new files to the auto-add archive." },
  { T_COPT,	"REMOVE_DEST",	0,0,0 },
  { T_COPT,	"PRESERVE",	0,0,0 },

  //---------- COMMAND wszst TRACKS ----------

  { T_CMD_BEG,	"TRACKS",	0,0,0 },

  { T_COPT,	"VERBOSE",	0,0,
	"Print an additonally hex status value as second column before the file name."
	" These values can be used to find a selection mask." },
  { T_COPT,	"ALL",	0,0,
	"Print the '_d' files too."
	" If set twice, print all files (same as -all)." },
  { T_COPT,	"LONG",	0,0,
	"Print 5 non track files too."
	" If set twice, print 4 more files with old BRRES sub file formats too." },
  { T_COPT,	"PIPE",		0,0,0 },
  { T_COPT,	"LOAD_BMG",	0,0,0 },

  //---------- COMMAND wszst SCANCACHE ----------

  { T_CMD_BEG,	"SCANCACHE",	0,0,0 },
  { T_COPT,	"CACHE",	0,0,0 },
  { T_COPT,	"PURGE",	0,0,0 },

  //---------- COMMAND wszst _EXPORT ----------

  { T_CMD_BEG,	"EXPORT",	0,0,0 },

  //---------- COMMAND wszst _SIZEOF ----------

  { T_CMD_BEG,	"SIZEOF",	0,0,0 },
  { T_COPT,	"SORT",		0,0,
		"Define a sort mode. Keywords are:"
		" @NONE@ (default), @NAME@ end @SIZE@." },
  { T_COPT,	"BRIEF",	0,0,
		"Limit output to SZS specific data." },
  { T_COPT,	"LOGGING",	0,0,0 },
  { T_COPT,	"EXT_ERRORS",	0,0,0 },
  { T_COPT,	"TIMING",	0,0,0 },

  //---------- COMMAND wszst _CODE ----------

  { T_CMD_BEG,	"CODE",	0,0,0 },

  //---------- COMMAND wszst _RECODE ----------

  { T_CMD_BEG,	"RECODE",	0,0,0 },

  //---------- COMMAND wszst _SUBFILE ----------

  { T_CMD_BEG,	"SUBFILE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },

  //---------- COMMAND wszst LIST ----------

  { T_CMD_BEG,	"LIST",		0,0,0 },
  { T_COPT,	"RECURSE",	0,0,0 },
  { T_COPT,	"EXT",		0,0,0 },
  { T_COPT,	"CUT",		0,0,0 },
  { T_COPT,	"ALL",		0,0,0 },
  { T_COPT_M,	"LONG",		0,0,
	"If set, the size of each file is printed."
	" If set twice, offset and size are printed."
	" If set three times, debugging mode is enabled." },
  { T_COPT,	"SORT",		0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPT,	"RM_AIPARAM",	0,0,0 },
  { T_COPT,	"SLOT",		0,0,0 },

  //---------- COMMAND wszst LIST-L ----------

  { T_CMD_BEG,	"LIST_L",	0,0,0 },
  { T_COPY_CMD,	"LIST",		0,0,0 },

  //---------- COMMAND wszst LIST-LL ----------

  { T_CMD_BEG,	"LIST_LL",	0,0,0 },
  { T_COPY_CMD,	"LIST_L",	0,0,0 },

  //---------- COMMAND wszst LIST-LLL ----------

  { T_CMD_BEG,	"LIST_LLL",	0,0,0 },
  { T_COPY_CMD,	"LIST_LL",	0,0,0 },

  //---------- COMMAND wszst LIST-A ----------

  { T_CMD_BEG,	"LIST_A",	0,0,0 },
  { T_COPY_CMD,	"LIST_L",	0,0,0 },

  //---------- COMMAND wszst LIST-LLA ----------

  { T_CMD_BEG,	"LIST_LA",	0,0,0 },
  { T_COPY_CMD,	"LIST_A",	0,0,0 },

  //---------- COMMAND wszst LIST-LLA ----------

  { T_CMD_BEG,	"NAME_REF",	0,0,0 },
  { T_COPT,	"SORT",		0,0,
	"Define the sort order of the reference."
	" @NONE@ (default) uses the order of the string pool."
	" @BRRES@ orders in standard BRRES order by ASCCI value and case sensitive."
	" @NAME@ similar to @BRRES@, but case insensitive."
	" @OFFSET@ orders by the file offset of first usage." },
  { T_COPT,	"BRIEF",	0,0,
	"If set, suppress the offsets of the strings."
	" This makes comparisons after name changes easier."
	" If set twice, suppress reference offsets too."
	" If set 3 times, suppress all offsets." },
  { T_COPT,	"LONG",		0,0,
	"If set, a list ordered by @NAME@ or @BRRES@ (default)"
	" and a second list ordered by @OFFSET@ is printed."
	" If set twice, a third list with order @NONE@ is appended." },

  //---------- COMMAND wszst ILIST ----------

  { T_CMD_BEG,	"ILIST",	0,0,0 },
  { T_COPT,	"RECURSE",	0,0,0 },
  { T_COPT,	"ALL",		0,0,0 },
  { T_COPT_M,	"LONG",		0,0,
	"If set, image width, height and size are printed."
	" If set twice, a block info and attribites are printed too." },
  { T_COPT,	"SORT",		0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },

  //---------- COMMAND wszst ILIST-L ----------

  { T_CMD_BEG,	"ILIST_L",	0,0,0 },
  { T_COPY_CMD,	"ILIST",	0,0,0 },

  //---------- COMMAND wszst ILIST-LL ----------

  { T_CMD_BEG,	"ILIST_LL",	0,0,0 },
  { T_COPY_CMD,	"ILIST_L",	0,0,0 },

  //---------- COMMAND wszst ILIST-A ----------

  { T_CMD_BEG,	"ILIST_A",	0,0,0 },
  { T_COPY_CMD,	"ILIST",	0,0,0 },

  //---------- COMMAND wszst ILIST-LA ----------

  { T_CMD_BEG,	"ILIST_LA",	0,0,0 },
  { T_COPY_CMD,	"ILIST_A",	0,0,0 },

  //---------- COMMAND wszst MEMORY ----------

  { T_CMD_BEG,	"MEMORY",	0,0,0 },
  { T_COPT,	"RECURSE",	0,0,0 },
  { T_COPT,	"EXT",		0,0,0 },
  { T_COPT,	"DECODE",	0,0,
	"Decode binary files to find memory addresses of sections." },
  { T_COPT,	"MIPMAPS",	0,0,
	"Analyze images and dump mipmap adresses too." },
  { T_COPT,	"CUT",		0,0,0 },
  { T_COPT,	"ALL",		0,0,
	"A short cut for {--recurse --decode --mipmaps}."
	" If entered twice, option --cut is included too." },
  { T_COPT_M,	"LONG",		0,0,
	"If set, print absolute offsets too." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPT,	"RM_AIPARAM",	0,0,0 },
  { T_COPT,	"SLOT",		0,0,0 },

  //---------- COMMAND wszst MEMORY-A ----------

  { T_CMD_BEG,	"MEMORY_A",	0,0,0 },
  { T_COPY_CMD,	"MEMORY",	0,0,0 },

  //---------- COMMAND wszst DUMP ----------

  { T_CMD_BEG,	"DUMP",		0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },

  //---------- COMMAND wszst SHA1 ----------

  { T_CMD_BEG,	"SHA1",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"ID",		0,0,0 },
  { T_COPT,	"BASE64",	0,0,0 },
  { T_COPT,	"DB64",		0,0,0 },
  { T_COPT,	"CODING",	0,0,0 },
  { T_COPT,	"VERIFY",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Suppress output of the filename." },
  { T_COPT,	"LONG",		0,0,
	"If set and {--db64} is not active,"
	" start the line with the length of the decompressed source"
	" as decimal number (field width 9)"
	" separated with a comma from the SHA1 checksum."
	" If set twice, prefix each line with the modification time"
	" in the format @'yyyy-mm-dd HH:MM:SS '@."
	"\n "
	" If set and {--db64} is active, then CKPT lap counter, STGI lap counter,"
	" speed factor and slot info are inserted beteen checksum and filename."
	" All columns are separated by one or two spaces." },

  //---------- COMMAND wszst ANALYZE ----------

  { T_CMD_BEG,	"ANALYZE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Append a list of special files with size, SHA1 and file name." },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"QUIET",	0,0,
	"Suppress logging of input and output files." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"SCRIPT",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"REFERENCE",	0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },

  //---------- COMMAND wszst SPLIT ----------

  { T_CMD_BEG,	"SPLIT",	0,0,0 },
  { T_COPT,	"LOAD_PREFIX",	0,0,0 },
  { T_COPT,	"LOAD_CATEGORY",0,0,0 },
  { T_COPT,	"PLUS",		0,0,0 },
  { T_COPT,	"SPLIT",	0,0,0 },
  { T_COPT,	"PRINTF",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"SCRIPT",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },

  //---------- COMMAND wszst IS_TEXTURE ----------

  { T_CMD_BEG,	"IS_TEXTURE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"REFERENCE",	0,0,0 },
  { T_COPT,	"BATTLE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Append a colon and the filename after each status." },

  //---------- COMMAND wszst FEATURES ----------

  { T_CMD_BEG,	"FEATURES",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Suppres any comments at all."
	" If set twice, suppress empty lines (for grouping) too."
	" If set three times, omit inactive features (value 0) too." },
  { T_COPT,	"LONG",		0,0,
	" If set, a comma-separated list of letters is added to the end of each feature."
	" If set twice, the output is filled with minus signed to get an aligned string."
	" The letters used are:"
	" @S@ (section), @V@ (visual impact), @B@ (battle impact), @R@ (racing impact),"
	" @T@ (time trial impact), @L@ (local=offline impact) and @O@ (online impact)." },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"QUIET",	0,0,
	"Suppress logging of input and output files." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"SCRIPT",	0,0,0 },
  { T_COPT,	"FMODES",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },

  //---------- COMMAND wszst DISTRIBUTION ----------

  { T_CMD_BEG,	"DISTRIBUTION",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NORM",		0,0,0 },
  { T_COPT,	"LINKS",	0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"BASE64",	0,0,0 },
  { T_COPT,	"DB64",		0,0,0 },
  { T_COPT,	"CODING",	0,0,0 },
  { T_COPT,	"SOURCE",	0,0,
	"Define a text file for filename to slot translation."
//	" The file is either a BMG file, or a CT-CODE file,"
//	" or a plain text file with a »slot filename« list."
	" A »slot filename« list is expected."
	" Use this option multiple times to define more than one source file."
	" A definition of a later source overwrites previous definitions."
	" The file @RESULT@ of the command line is appended automatically"
	" as last entry, if exists and not already included." },

  //---------- COMMAND wszst DIFF ----------

  { T_CMD_BEG,	"DIFF",		0,0,0 },
  { T_COPT,	"QUIET",	0,0,
	"If set once, only a summary status for each section is printed."
	" If set twice, only a total summary status is printed."
	" If set three times, the status is only reported as exit status." },
  { T_COPT,	"RECURSE",	0,0,0 },
  { T_COPT,	"CUT",		0,0,0 },
  { T_COPT,	"ALL",		0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },

  //---------- COMMAND wszst CHECK ----------

  { T_CMD_BEG,	"CHECK",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,
	"Don't check the KCL and KMP files." },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Print only warnings and suppress hints." },
  { T_COPT,	"LONG",	0,0,
	"Be verbose and print more statistics." },
  { T_COPT,	"QUIET",	0,0,
	"Be quiet until the first warning or hint is found." },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst SLOTS ----------

  { T_CMD_BEG,	"SLOTS",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Print the slot list in a compressed form without spaces." },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst STGI ----------

  { T_CMD_BEG,	"STGI",		0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,
	"If set, ignore non existing source files"
	" and files without KMP without warning."
	" If set twice, ignore files with standard settings too." },

  //---------- COMMAND wszst ISARENA ----------

  { T_CMD_BEG,	"ISARENA",	0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"If set, suppress the output of header and filenames." },
  { T_COPT,	"QUIET",	0,0,
	"If set, print nothing and set only the exit status." },
  { T_COPT,	"IGNORE",	0,0,
	"If set, ignore non existing source files"
	" and files without KMP without warning." },

  //---------- COMMAND wszst NORMALIZE ----------

  { T_CMD_BEG,	"NORMALIZE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"REMOVE_SRC",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"FFORMAT1",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"NORM",		0,0,0 },
  { T_COPT,	"FAST",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst PATCH ----------

  { T_CMD_BEG,	"PATCH",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"REMOVE_SRC",	0,0,0 },
  { T_COPT,	"NO_COPY",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"FFORMAT1",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"COMPRESS",	0,0,0 },
  { T_COPY_GRP,	"NORM",		0,0,0 },
  { T_COPT,	"FAST",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst COPY ----------

  { T_CMD_BEG,	"COPY",		0,0,0 },
  { T_COPY_CMD,	"PATCH",	0,0,0 },

  //---------- COMMAND wszst DUPLICATE ----------

  { T_CMD_BEG,	"DUPLICATE",	0,0,0 },
  { T_COPT,	"GAMEMODES",	0,0,0 },
  { T_COPY_CMD,	"PATCH",	0,0,0 },

  //---------- COMMAND wszst MINIMAP ----------

  { T_CMD_BEG,	"MINIMAP",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
		"If set, print the matrices too."
		" If set twice, print minimum and maximum values too." },
  { T_COPT,	"REMOVE_DEST",	0,0,0 },
  { T_COPT,	"PRESERVE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"COMPRESS",	0,0,0 },
  { T_COPY_GRP,	"NORM",		0,0,0 },
  { T_COPT,	"FAST",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"CONST",	0,0,0 },
  { T_COPT,	"TOUCH",	0,0,0 },
  { T_COPT,	"AUTO",		0,0,0 },
  { T_COPT,	"SET_FLAGS",	0,0,0 },
  { T_COPT,	"SET_SCALE",	0,0,0 },
  { T_COPT,	"SET_ROT",	0,0,0 },
  { T_COPT,	"SET_X",	0,0,0 },
  { T_COPT,	"SET_Y",	0,0,0 },
  { T_COPT,	"SET_Z",	0,0,0 },
  { T_COPT,	"XCENTER",	0,0,0 },
  { T_COPT,	"YCENTER",	0,0,0 },
  { T_COPT,	"ZCENTER",	0,0,0 },
  { T_COPT,	"CENTER",	0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wszst COMPRESS ----------

  { T_CMD_BEG,	"COMPRESS",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"REMOVE_SRC",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"FFORMAT1",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"COMPRESS",	0,0,0 },
  { T_COPY_GRP,	"NORM",		0,0,0 },
  { T_COPT,	"FAST",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst DECOMPRESS ----------

  { T_CMD_BEG,	"DECOMPRESS",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"REMOVE_SRC",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"U8",		0,0,0 },
  { T_COPT,	"WU8",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"NORM",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },
  { T_COPT,	"IGNORE_SETUP",	0,0,0 },
  { T_COPY_GRP,	"ALIGN",	0,0,0 },
  { T_COPT,	"LOGGING",	0,0,0 },
  { T_COPT,	"EXT_ERRORS",	0,0,0 },
  { T_COPT,	"TIMING",	0,0,0 },
  { T_COPY_GRP,	"ENCODE",	0,0,0 },
  { T_COPT,	"MIPMAPS",	0,0,0 },
  { T_COPT,	"NO_MIPMAPS",	0,0,0 },
  { T_COPT,	"N_MIPMAPS",	0,0,0 },
  { T_COPT,	"MAX_MIPMAPS",	0,0,0 },
  { T_COPT,	"MIPMAP_SIZE",	0,0,0 },
  { T_COPT,	"FAST_MIPMAPS",	0,0,0 },
  { T_COPT,	"CMPR_DEFAULT",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst CREATE ----------

  { T_CMD_BEG,	"CREATE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"CREATE",	0,0,0 },
  { T_COPT,	"NO_COMPRESS",	0,0,0 },
  { T_COPT,	"COMPRESS",	0,0,0 },
  { T_COPT,	"FAST",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst UPDATE ----------

  { T_CMD_BEG,	"UPDATE",	0,0,0 },
  { T_COPT,	"SOURCE",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_COMPRESS",	0,0,0 },
  { T_COPT,	"COMPRESS",	0,0,0 },
  { T_COPT,	"FAST",		0,0,0 },
  { T_COPT,	"RECURSE",	0,0,0 },
  { T_COPT,	"ALL",		0,0,0 },
  { T_COPY_GRP,	"ENCODE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst EXTRACT ----------

  { T_CMD_BEG,	"EXTRACT",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"EXTRACT",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { H_COPT,	"ANALYZE",	0,0,0 },
  { H_COPT,	"ANALYZE_MODE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst XDECODE ----------

  { T_CMD_BEG,	"XDECODE",	0,0,0 },
  { T_COPY_CMD,	"EXTRACT",	0,0,0 },

  //---------- COMMAND wszst XEXPORT ----------

  { T_CMD_BEG,	"XEXPORT",	0,0,0 },
  { T_COPY_CMD,	"EXTRACT",	0,0,0 },

  //---------- COMMAND wszst XALL ----------

  { T_CMD_BEG,	"XALL",		0,0,0 },
  { T_COPY_CMD,	"EXTRACT",	0,0,0 },

  //---------- COMMAND wszst XCOMMON ----------

  { T_CMD_BEG,	"XCOMMON",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wszst BINARY ----------

  { T_CMD_BEG,	"BINARY",	0,0,0 },
  { T_COPT,	"KMG_LIMIT",	0,0,0 },
  { T_COPT,	"KMG_COPY",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },

  //---------- COMMAND wszst TEXT ----------

  { T_CMD_BEG,	"TEXT",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"DECODE",	0,0,0 },
  { T_COPT,	"KMG_LIMIT",	0,0,0 },
  { T_COPT,	"KMG_COPY",	0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPY_GRP,	"BMG",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },

  //---------- COMMAND wszst CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"DECODE",	0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPY_GRP,	"BMG",		0,0,0 },

  //---------- COMMAND wszst BMG ----------

  { T_CMD_BEG,	"BMG",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0, NO_HEAD_SYNTAX },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPY_GRP,	"BMG",		0,0,0 },

  //---------- COMMAND wszst KCL ----------

  { T_CMD_BEG,	"KCL",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0, NO_HEAD_SYNTAX },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"NO_PARAM",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst KMP ----------

  { T_CMD_BEG,	"KMP",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0, NO_HEAD_SYNTAX },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },
  { T_COPT,	"NO_PARAM",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM2",	0,0,0 },

  //---------- COMMAND wszst LEX ----------

  { T_CMD_BEG,	"LEX",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0, NO_HEAD_SYNTAX },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },

  //---------- COMMAND wszst INFO ----------

  { T_CMD_BEG,	"INFO",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress output of the printed filename." },

  //---------- COMMAND wszst GHOST ----------

  { T_CMD_BEG,	"GHOST",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress output of the printed filename." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"SCRIPT",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },

  //---------- COMMAND wszst YAZDUMP ----------

  { T_CMD_BEG,	"YAZDUMP",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },

  //---------- COMMAND wszst VEHICLE ----------

  { T_CMD_BEG,	"VEHICLE",	0,0,0 },
  { T_COPT,	"SOURCE",	0,0,
		"Load the file instead of using the internal base." },
  { T_COPT,	"DEST",		0,0,
		"Define a destination path (directory/file)."
		" The destination @-@ means: write to standard output." },
  { T_COPT,	"DEST2",	0,0,0 },
  { T_COPT,	"OVERWRITE",	0,0,0 },
  { T_COPT,	"QUIET",	0,0,
		"Suppress logging." },
  { T_COPT,	"VERBOSE",	0,0,
		"Log patching." },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress min/max counts on dumps." },
  { T_COPT,	"DE",		0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////


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
 *   Copyright (c) 2011-2022 by Dirk Clemens <wiimm@wiimm.de>              *
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
/////////////			    wctct			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wctct", 0,
		"wctct [option]... command [option|parameter|@file]...",
		WCTCT_LONG " :"
		" Manage the $CT-CODE$ extension."
		" " CT_CODE_SOURCE " files are accepted as input." },

  //
  //---------- list of all wctct commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION",
		    "wctct VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wctct HELP [--width=wd] OPT|OPTION ...\n"
		    "wctct HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wctct HELP [--width=wd] command [-b] [ignored]...\n"
		    "wctct HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wctct CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wctct ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wctct EXPAND ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wctct TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wctct COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wctct ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wctct FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wctct FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"SYMBOLS",	"SYMBOLS",
		    "wctct SYMBOLS",
		"Print predefined symbols of text parser to standard output."
		" The CT-CODE specific symbols are included." },

  { T_DEF_CMD,	"FUNCTIONS",	"FUNCTIONS",
		    "wctct FUNCTIONS [key]...",
		"Print a list with all parser functions including"
		" the CT-CODE specific functions."
		" If one key ore more key entered,"
		" then only functions that match at least one key are printed." },

  { T_DEF_CMD,	"CALCULATE",	"CALCULATE",
		    "wctct CALCULATE expression...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FLOAT",	"FLOAT",
		    "wctct FLOAT number...",
		0 /* copy of wszst */ },

  { H_DEF_CMD,	"EXPORT",	"_EXPORT",
		    "wszst _EXPORT key...",
		"This hidden command exports some definitions to standard output."
		" Use keyword @HELP@ to get a list of possible keywords." },

  { H_DEF_CMD,	"RAWDUMP",	"_RAWDUMP",
		    "wszst _RAWDUMP [-d|-D] destdir",
		"This hidden command creates the destination directory "
		" and create several files as copy of internal data." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"DUMP",		"D|DUMP",
		    "wctct DUMP [source]...",
		"Print an information dump for the CT-CODE of each source file."
		" " CT_CODE_SOURCE " are accepted as input." },

  { T_DEF_CMD,	"CAT",		"CAT",
		    "wctct CAT [source]...",
		"Read the source files and print the CT-CODE part"
		" them as decoded text files to standard output."
		" The output of all source files is concatenated."
		" " CT_CODE_SOURCE " are accepted as input." },

  { T_DEF_CMD,	"BMG",		"BMG",
		    "wctct BMG [source]...",
		"Read the source files and print a BMG message listing to standard output."
		" " CT_CODE_SOURCE " are accepted as input." },

  { T_DEF_CMD,	"TRACKS",	"TRACKS",
		    "wctct TRACKS [source]...",
		"Read the source files and print a track listing to standard output."
		" " CT_CODE_SOURCE " are accepted as input." },

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wctct DECODE [source]...",
		"Read the source files and decode them to CT-TEXT files."
		" The default destination is @'%P/%N.txt'@."
		" " CT_CODE_SOURCE " are accepted as input." },

  { T_DEF_CMD,	"CREATE",	"CREATE|C",
		    "wctct CREATE keyword [source]...",
		"Read the source files, scan the CT-CODE part"
		" and create an output file controlled by the keyword."
		" BRRES, TEX0, CT-CODE, CT-TEXT and LE-BIN files are accepted as input."
		" Possible keywords are: @CUP1, CRS1, r-MOD1, r-MOD2, r-OVR1,"
		" r-BAD1CODE, r-BAD1DATA (=r-CODE), r-TEX0, r-BRRES, r-SZS,"
		" BMG, TRACKS, LIST, REF@ and @FULL@."
		" 'r' defines the region and is one of"
		" @EU, RMCP, US, RMCE, JP, RMCJ, KO@ or @RMCK@."
		"\n "
		" The Korean support needs external CT-CODE files."
		" Use option {--ct-dir} to define at least one search path." },

  { T_DEF_CMD,	"PATCH",	"PATCH|P",
		    "wctct PATCH [source]...",
		"Read each source file, find and scan the CT-CODE part"
		" and patch it."
		" " CT_CODE_SOURCE " are accepted as input." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  //
  //---------- list of all wctct options ----------

  { T_OPT_S,	"VERSION",	"V|version",
		0, 0 /* copy of wszst */ },

  { T_OPT_S,	"HELP",		"h|help",
		0, 0 /* copy of wszst */ },

  { T_OPT_S,	"XHELP",	"xhelp",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"CONFIG",	"config",
		0, 0 /* copy of wszst */ },

  { T_OPT_G,	"ALLOW_ALL",	"allow-all|allowall",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"COMPATIBLE",	"compatible",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP|F_IGNORE,
		"WIDTH",	"width",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP|F_IGNORE,
		"MAX_WIDTH",	"max-width|maxwidth",
		0, 0 /* copy of wszst */ },

  { T_OPT_G|F_IGNORE,
		"NO_PAGER",	"no-pager|nopager",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"QUIET",	"q|quiet",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"VERBOSE",	"v|verbose",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"LOGGING",	"L|logging",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"TIMING",	"timing",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"WARN",		"W|warn",
		0, 0 /* copy of wszst */ },

  { T_OPT_G,	"DE",		"de",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO|F_IGNORE,
		"COLORS",	"colors",
		0, 0 /* copy of wszst */ },

  { T_OPT_G|F_IGNORE,
		"NO_COLORS",	"no-colors|nocolors",
		0, 0 /* copy of wszst */ },

  { T_OPT_G,	"CT_CODE",	"ct-code|ctcode",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"LE_CODE",	"le-code|lecode",
		0, 0 /* copy of wszst */ },

  { T_OPT_G,	"OLD_SPINY",	"old-spiny|oldspiny",
		0,
		"Disable the new feature to hold a Spiny Shell at the tail."
		"\1 https://szs.wiimm.de/opt/ctcode for details." },

  { T_OPT_GP,	"CRS1",		"crs1",
		"value",
		"Store this value into all 8 members of the property array"
		" of the CRS1 header if reading a CT-TEXT."
		" The default value is 0x800." },

  { T_OPT_GMP,	"CHDIR",	"chdir",
		0, 0 /* copy of wszst */ },

  { T_OPT_GMP,	"CONST",	"c|const",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"ROUND",	"round",
		0, 0 /* copy of wszst */ },

  { T_OPT_CM,	"LONG",		"l|long",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0,
		"Suppress cross reference comments." },

  { T_OPT_C,	"RAW",	"raw",
		0,
		"Print BMG messages as UTF-8 text without escape sequences." },

  { T_OPT_CP,	"BMG_ENDIAN",	"bmg-endian|bmgendian",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"BMG_ENCODING",	"bmg-encoding|bmgencoding",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"BMG_INF_SIZE",	"bmg-inf-size|bmginfsize",
		0, 0 /* copy of wszst */ },

  { T_OPT_CO,	"BMG_MID",	"bmg-mid|bmgmid",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"FORCE_ATTRIB",	"force-attrib|forceattrib",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"DEF_ATTRIB",	"def-attrib|defattrib",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_ATTRIB",	"no-attrib|noattrib",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"X_ESCAPES",	"x-escapes|xescapes",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"OLD_ESCAPES",	"old-escapes|oldescapes",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"SINGLE_LINE",	"1|single-line|singleline",
		0,
		"If set, don't print continuation lines for BMG text output."
		" If set twice, print only single text lines but not value lines." },

  { T_OPT_C,	"NO_BMG_COLORS","no-bmg-colors|nobmgcolors",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BMG_COLORS",	"bmg-colors|bmgcolors",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_BMG_INLINE","no-bmg-inline|nobmginline",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"EXPORT",	"X|export",
		0,
		"Enable the export mode and create small and machine readable text files"
		" for easy post processing."
		" The option works similar like @-HB@ for CT-CODE output." },

  { T_OPT_C,	"NO_PARAM",	"P|no-param|noparam",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_ECHO",	"no-echo|noecho",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_CHECK",	"N|no-check|nocheck",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"MAX_FILE_SIZE","M|max-file-size|maxfilesize",
		0, 0 /* copy of wszst */ },

  { H_OPT_GP,	"TRACKS",	"T|tracks",
		0, 0 /* copy of wszst */ },

  { H_OPT_GP,	"ARENAS",	"A|arenas",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { H_OPT_G,	"UTF_8",	"utf-8|utf8",
		0, 0 /* copy of wszst */ },

  { H_OPT_G,	"NO_UTF_8",	"no-utf-8|no-utf8|noutf8",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_GM,	"TEST",		"t|test",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"FORCE",	"force",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"REPAIR_MAGICS",	"repair-magics|repairmagics",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"CREATE_DISTRIB",	"create-distrib|createdistrib",
		"file",
		"Create a LE-CODE distribution dump with a track list and more info about"
		" the current distribution. Store the dump info a file." },

 #if OPT_OLD_NEW
  { H_OPT_GM,	"OLD",		"OLD",
		0, 0 /* copy of wszst */ },

  { H_OPT_GM,	"STD",		"STD",
		0, 0 /* copy of wszst */ },

  { H_OPT_GM,	"NEW",		"NEW",
		0, 0 /* copy of wszst */ },
 #endif

  { H_OPT_GM,	"EXTRACT",	"EXTRACT",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"LIST",		"list",
		0,
		"Cancel all previous {--rel} and {--full}"
		" and enter LIST mode (default) for text output."
		"\n "
		" In LIST mode only a track list is generated."
		" On scanning the tracks are automatically distributed to the cups."
		" This makes rearanging very simple." },

  { T_OPT_C,	"REF",		"ref",
		0,
		"Cancel all previous {--list} and {--full}"
		" and enter REFERENCE mode for text output."
		"\n "
		" In REFERENCE mode all tracks and cups are printed separately."
		" After scanning the cup and track layout is the same"
		" as the original code file." },

  { T_OPT_C,	"FULL",		"full",
		0,
		"Cancel all previous {--list} and {--rel}"
		" and enter FULL mode for text output."
		"\n "
		" FULL mode is a combination of LIST and REF modes." },

  { T_OPT_C,	"HEX",		"hex",
		0,
		"Normally and if available, names are printed for music and property"
		" slots. If this option is set, hex numbers are printed always." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_GP,	"CT_DIR",	"ct-dir|ctdir",
		"dir",
		OPT_CT_DIR("\n  wctct will search in the sub directory @'rmc[pujk]/'@" \
			" for files @'mod1.bin', 'mod2.bin', 'ovr1.bin'@" \
			" and @'bad1code.bin'@.") },

  { T_OPT_GO,	"CT_LOG",	"ct-log|ctlog",
		"[=level]",
		"Define a log level for option {--ct-dir}."
		" Without parameter, the current level is increased."
		" Allowed keywords are: 0=0FF, 1=SUCCESS, 2=FOUND, 3=ALL" },

  { T_OPT_GP,	"ALLOW_SLOTS",	"allow-slots|allowslots",
		"list",
		"Allow and disallow slots for automatic slot assigning."
		" A comma separated list of slots (A), slot ranges (A:B) or keywords"
		" is expected. If preceded by a minus sign, the slots are disabled."
		" The keywords are: @TRACKS@ (slots 0:0x1f), @ARENAS@ (0x20:0x29)"
		" and @SPECIAL@ (0x36:0x3a)."
		" @CTCODE@ is an abbreviation for @ARENAS,SPECIAL@."
		"\n "
		" The default is to enable slot ranges"
		" @0x00:0x1f@, @0x2a:0x35@, @0x3b:0x41@ and @0x44:0xfe@." },

  { T_OPT_CP,	"IMAGES",	"images",
		"dir",
		"If creating a CT-CODE BRRES or SZS file,"
		" then search the 8 strap images in the specified directory."
		" The images are searched with extensions .png, .tex, .tex0"
		" and without extension in this order until a valid image is found."
		" All images will be resized to the correct width and height "
		" and converted to format TEX.RGB565 without mipmaps."
		" If no valid image is found, a hard coded default image is used." },

  { T_OPT_CMP,	"LOAD_BMG",	"load-bmg|loadbmg",
		0, 0 /* copy of wszst */ },

  { T_OPT_CMP,	"PATCH_BMG",	"patch-bmg|patchbmg|pb",
		"mode[cond][=param]",
		PATCH_BMG_TEXT("") },

  { T_OPT_CMP,	"MACRO_BMG",	"macro-bmg|macrobmg|mb",
		0, 0 /* copy of wszst */ },

  { T_OPT_CMP,	"FILTER_BMG",	"filter-bmg|filterbmg|fb",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"PATCH_NAMES",	"patch-names|patchnames",
		0,
		"If set, the names of the CUP1 and CSR1 sections are patched "
		" by the loaded BMG files directly after loading the source files." },

  { T_OPT_CP,	"ORDER_BY",	"order-by|orderby|order",
		"bmg",
		"Load the BMG file and filter track names."
		" Then before creating output files,"
		" re-distribute the tracks of the additonal cups (9-)"
		" to the same cups by the order of the read track names."
		" Tracks without BMG reference are placed at the end of all tracks."
		" This option modifes only the track order of the cups,"
		" but will not change any name." },

  { T_OPT_C,	"ORDER_ALL",	"order-all|orderall",
		0,
		"Used together with option {--order-by}:"
		" Re-distribute the tracks of all cups beginning with Mushroom Cup." },

  { H_OPT_C,	"DYNAMIC",	"dynamic",
		0,
		"Enable dynamic length calculation for the @CTGP_CODE@ part."
		" The surrounding TEX and BRRES files are also adjusted."
		" The usage of this option is EXPERIMENTAL." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"WRITE_TRACKS",	"write-tracks|writetracks|wtracks",
		"file",
		"Create a machine readable file reflecting"
		" the racing track distribution to the cups."
		" One line of format @'IDX|MID|NAME'@ is written for each track."
		" --wtracks is a short cut." },

  { T_OPT_CP,	"DEST",		"d|dest",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"DEST2",	"D|DEST",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"ESC",		"E|esc",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"OVERWRITE",	"o|overwrite",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NUMBER",	"number|num",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"REMOVE_DEST",	"r|remove-dest|removedest|rm-dest|rmdest|remove",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"UPDATE",	"u|update",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"PRESERVE",	"p|preserve",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"IGNORE",	"i|ignore",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"SECTIONS",	"sections",
		0, 0 /* copy of wszst */ },

  //
  //---------- wctct GROUP DEST ----------

  { T_GRP_BEG,	"DEST",		0,0,0 },

  { T_COPT,	"WRITE_TRACKS",	0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"OVERWRITE",	0,0,0 },
  { T_COPT,	"NUMBER",	0,0,0 },
  { T_COPT,	"REMOVE_DEST",	0,0,0 },
  { T_COPT,	"UPDATE",	0,0,0 },
  { T_COPT,	"PRESERVE",	0,0,0 },

  //---------- wctct GROUP PATCH ----------

  { T_GRP_BEG,	"PATCH",	0,0,0 },

  { T_COPT,	"LOAD_BMG",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },
  { T_COPT,	"PATCH_NAMES",	0,0,0 },
  { T_COPT,	"ORDER_BY",	0,0,0 },
  { T_COPT,	"ORDER_ALL",	0,0,0 },
  //{ T_COPT,	"TRACKS",	0,0,0 },
  //{ T_COPT,	"ARENAS",	0,0,0 },

  //---------- wctct GROUP TEXTOPT ----------

  { T_GRP_BEG,	"TEXTOPT",	0,0,0 },

  { T_COPT,	"HEX",		0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress the syntax information section in CTC text files." },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },

  //---------- wctct GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",		0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"LIST",		0,0,0 },
  { T_COPT,	"REF",		0,0,0 },
  { T_COPT,	"FULL",		0,0,0 },
  { T_COPY_GRP,	"TEXTOPT",	0,0,0 },

  //
  //---------- COMMAND wctct VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wctct HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wctct CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wctct ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wctct EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wctct TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wctct COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wctct ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wctct FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wctct FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wctct SYMBOLS ----------

  { T_CMD_BEG,	"SYMBOLS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wctct FUNCTIONS ----------

  { T_CMD_BEG,	"FUNCTIONS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, FUNCTION_BRIEF },
  { T_COPT,	"LONG",		0,0, FUNCTION_LONG },

  //---------- COMMAND wctct CALCULATE ----------

  { T_CMD_BEG,	"CALCULATE",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },

  //---------- COMMAND wctct FLOAT ----------

  { T_CMD_BEG,	"FLOAT",	0,0,0 },
  { T_COPT,	"ROUND",	0,0,0 },

  //---------- COMMAND wctct _EXPORT ----------

  { T_CMD_BEG,	"EXPORT",	0,0,0 },

  //---------- COMMAND wctct _RAWDUMP ----------

  { T_CMD_BEG,	"RAWDUMP",	0,0,0 },

  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },

  //---------- COMMAND wctct DUMP ----------

  { T_CMD_BEG,	"DUMP",		0,0,0 },

  { T_COPT,	"LONG",		0,0,
		"Print headers as hexdump." },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wctct BMG ----------

  { T_CMD_BEG,	"BMG",		0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Filter the message by tracks, that are used a racing track."
		" If set twice, suppress also tracks created by the 'N' command"
		" of CT-TEXT files."
		" If set 3 times, suppress always the output of first 32 tracks." },
  { T_COPT,	"LONG",		0,0,
		"Normally, only the track and arena names are exported."
		" The message id (MID) range is 4000-40FF."
		" If set, cup names are exported too as MID 4200-423F."
		" If set twice, cup/track relations are also exported as MID 4300-43FF."},
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
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
  { T_COPT,	"WRITE_TRACKS",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wctct TRACKS ----------

  { T_CMD_BEG,	"TRACKS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
		"Suppress the table header." },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress the output of the music and property slots." },
  { T_COPT,	"LONG",		0,0,
		"Include the first 32 standard tracks into the output."
		" If set twice, also include the battle arenas." },
  { T_COPT,	"RAW",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"WRITE_TRACKS",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wctct CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"WRITE_TRACKS",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wctct DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wctct CREATE ----------

  { T_CMD_BEG,	"CREATE",	0,0,0 },

  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"TEXTOPT",	0,0,0 },
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
  { T_COPT,	"BRIEF",	0,0,
		"BMG: Filter the message by tracks, that are used a racing track."
		" If set twice, suppress also tracks created by the 'N' command"
		" of CT-TEXT files."
		" If set 3 times, suppress always the output of first 32 tracks." },
  { T_COPT,	"LONG",		0,0,
		"BMG: Normally, only the track and arena names are exported."
		" The message id (MID) range is 4000-40FF."
		" If set, cup names are exported too as MID 4200-423F."
		" If set twice, cup/track relations are also exported as MID 4300-43FF."},
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"CT_DIR",	0,0,0 },
  { T_COPT,	"CT_LOG",	0,0,0 },
  { T_COPT,	"ALLOW_SLOTS",	0,0,0 },
  { T_COPT,	"IMAGES",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { H_COPT,	"DYNAMIC",	0,0,0 },

  //---------- COMMAND wctct PATCH ----------

  { T_CMD_BEG,	"PATCH",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

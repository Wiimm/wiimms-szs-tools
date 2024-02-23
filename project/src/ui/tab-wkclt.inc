
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
 *   Copyright (c) 2011-2024 by Dirk Clemens <wiimm@wiimm.de>              *
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
/////////////			    wkclt			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wkclt", 0,
		"wkclt [option]... command [option|parameter|@file]...",
		WKCLT_LONG " :"
		" Analyze, modify, export and create KCL and OBJ files." },

  //
  //---------- list of all wkclt commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION|V",
		    "wkclt VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wkclt HELP [--width=wd] OPT|OPTION ...\n"
		    "wkclt HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wkclt HELP [--width=wd] command [-b] [ignored]...\n"
		    "wkclt HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wkclt CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wkclt ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wkclt EXPAND ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wkclt TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wkclt COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wkclt ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wkclt FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wkclt FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"SYMBOLS",	"SYMBOLS",
		    "wkclt SYMBOLS",
		"Print predefined symbols of text parser to standard output."
		" The KCL specific symbols are included." },

  { T_DEF_CMD,	"FUNCTIONS",	"FUNCTIONS",
		    "wkclt FUNCTIONS [key]...",
		"Print a list with all parser functions including the KCL specific functions."
		" If one key ore more key entered,"
		" then only functions that match at least one key are printed." },

  { T_DEF_CMD,	"CALCULATE",	"CALCULATE",
		    "wkclt CALCULATE expression...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"MATRIX",	"MATRIX",
		    "wkclt MATRIX [patch-option]... [vector]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FLOAT",	"FLOAT",
		    "wkclt FLOAT number...",
		0 /* copy of wszst */ },

  { H_DEF_CMD,	"EXPORT",	"_EXPORT",
		    "wkclt _EXPORT key...",
		"This hidden command exports some definitions to standard output."
		" Use keyword @HELP@ to get a list of possible keywords." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----


  { T_DEF_CMD,	"CAT",		"CAT",
		    "wkclt CAT source...",
		"Read the entered KCL and OBJ files and print"
		" them as OBJ files to standard output."
		" " WILDCARDS_PIPE
		" The output of all source files is concatenated."
		"\n "
		" Automated dropping of bad KCL triangles is enabled"
		" if no other drop option is specified by option {--kcl}." },

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wkclt DECODE source...",
		"Read the entered KCL and OBJ files"
		" and export them to OBJ files."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.txt'@."
		"\n "
		" Automated dropping of bad KCL triangles is enabled"
		" if no other drop option is specified by option {--kcl}." },

  { T_DEF_CMD,	"ENCODE",	"ENCODE|ENC",
		    "wkclt ENCODE source...",
		"Read the entered KCL and OBJ files"
		" and encode (compile) them to binary KCL files."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.kcl'@."
		"\n "
		" See https://szs.wiimm.de/doc/kcl for encoding details." },

  { T_DEF_CMD,	"COPY",		"COPY|CP",
		    "wkclt COPY source dest\n"
		    "wkclt COPY source... [-d|-D] dest",
		"Copy and convert a collision file from 'source' to 'dest'."
		" Multiple KCL and OBJ sources can be used,"
		" if 'dest' is a directory or contains at least one %-pattern."
		" " WILDCARDS_PIPE
		"\n "
		" This is an alternative for the DECODE and ENCODE commands."
		" The default destination file format is selected by scanning the"
		" file extension of the destination."
		" Is it '.kcl', a binary KCL file is created."
		" Is it '.txt' or '.obj', an OBJ file is created."
		" Otherwise the output format is the same as the source."
		"\n "
		" Automated dropping of bad KCL triangles is enabled"
		" if no other drop option is specified and the destination"
		" is an OBJ file." },

  { T_DEF_CMD,	"CFF",	"CFF",
		    "wkclt CFF source...",
		"CFF is an abbreviation of 'create flag file' and"
		" will create a flag file template for each entered source file."
		" " WILDCARDS_PIPE
		" If the flag file already exists, only missing group names"
		" are added at the end of file." },

  { H_DEF_CMD,	"TYPES",	"TYPES",
		    "wkclt TYPES arg...",
		"Print a line with KCL flag details for each argument."
		" ARG is a comma or space separated list of a optional BASEVAL"
		" and a list of TERMS. BASEVAL can be any hexa-decimal number."
		" The format of TERMS is @X[=]VALUE@."
		" So a TERM begin with letter @T@ (Type), @V@ (Value) or a group"
		" identifier (W, X,Y or Z), followed by an optional equal sign"
		" and finished by a decimal number."
		" It overrides the related part of BASEVAL." },

  { T_DEF_CMD,	"FLAGS",	"FLAGS|F",
		    "wkclt FLAGS source...",
		"Print a KCL type and flag usage statistic for each source"
		" to standard output."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"DUMP",		"DUMP|D",
		    "wkclt DUMP source...",
		"This debugging command prints an information dump about"
		" the internal triangle data base and the octree"
		" in human and machine readable text format."
		" " WILDCARDS_PIPE
		" The output of all source files is concatenated"
		" and printed to standard output." },

  { T_DEF_CMD,	"DBRIEF",	"DBRIEF|DB",
		    "wkclt DBRIEF source...",
		"Command @DBRIEF@ is a short cut for {DUMP --brief}."
		" The output of the triangle list is suppressed"
		" and only the header data and statistics are printed." },

  { T_DEF_CMD,	"LIST",		"LIST",
		    "wkclt LIST source...",
		"This debugging command prints a list of all triangles"
		" with metrics in human and machine readable text format."
		" It is like DUMP, but reduced to the list of triangles."
		" " WILDCARDS_PIPE
		" The output of all source files is concatenated"
		" and printed to standard output." },

  { T_DEF_CMD,	"TRIANGLES",	"TRIANGLES",
		    "wkclt TRIANGLES source...",
		"This debugging commands prints one data line for each triangle."
		" The goal is to compare data. Therefor and to create smaller lines"
		" the precision of the floating points numbers is reduced."
		" " WILDCARDS_PIPE
		"\n "
		" The print order is: flag used length | 3 points | 4 normals."
		" For each point and normal 3 coordinates are printed." },

  { T_DEF_CMD,	"TRAVERSE",	"TRAVERSE",
		    "wkclt TRAVERSE source point...",
		"For each point, the octree of the source file is traversed"
		" to find and print the related triangles."
		" A point is either a comma separated list with 3 parser"
		" expressions (one for each coordinate) or a vector expression." },

  { T_DEF_CMD,	"FALL",		"FALL",
		    "wkclt FALL source point...",
		"For each entered point, the lowest point is searched,"
		" that can be reached without collision."
		" The point is expanded to a cube with edge length of 100.0"
		" for the collision tests." },

  { T_DEF_CMD,	"CHECK",	"CHECK|CHK",
		    "wkclt CHECK source...",
		"Read the entered KCL files (text or binary) and"
		" and make a plausibility check."
		" " WILDCARDS_PIPE
		" Hints and warnings are written to standard output."
		" The exit status is @'DIFFER'@ if at least one KMP warning found." },

  { T_DEF_CMD,	"SHA1",		"SHA1",
		    "wszst SHA1 [source]...",
		"Print a SHA1 checksum of the KCL for each source."
		" " WILDCARDS_PIPE
		" The checksum is calculated after patching." },

  { H_DEF_CMD,	"ANALYZE",	"ANALYZE",
		    "wkclt ANALYZE source...",
		"Analyze some KCL data."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"BLOW",		"BLOW",
		    "wkclt BLOW source...",
		"Try to calculate the used blow size (KCL_BLOW) of a KCL."
		" " WILDCARDS_PIPE
		" Print one line for each source file." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  //
  //---------- list of all wkclt options ----------

  { T_OPT_S,	"VERSION",	"V|version",
		0, 0 /* copy of wszst */ },

  { T_OPT_S,	"HELP",		"h|help",
		0, 0 /* copy of wszst */ },

  { T_OPT_S,	"XHELP",	"xhelp",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"CONFIG",	"config",
		0, 0 /* copy of wszst */ },

  { H_OPT_G,	"YDEBUG",	"ydebug",
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

  { T_OPT_GM,	"EXT_ERRORS",	"Y|extended-errors|extendederrors|xerr",
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

  { T_OPT_G,	"LE_04X",	"le-04x|le04x",
		0, 0 /* copy of wszst */ },

  { T_OPT_GMP,	"CHDIR",	"chdir",
		0, 0 /* copy of wszst */ },

  { T_OPT_GMP,	"CONST",	"c|const",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"SCALE",	"scale",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"SHIFT",	"shift",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"XSS",		"xss",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"YSS",		"yss",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"ZSS",		"zss",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"ROT",		"rot",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"XROT",		"xrot",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"YROT",		"yrot|hrot",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"ZROT",		"zrot",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"TRANSLATE",	"translate",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NULL",		"null",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NEXT",		"next",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"ASCALE",	"ascale",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"AROT",		"arot",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"TFORM_SCRIPT",	"tform-script|tformscript",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"KCL",		"kcl",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"KCL_FLAG",	"kcl-flag|kclflag",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"KCL_SCRIPT",	"kcl-script|kclscript",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"TRI_AREA",	"tri-area|triarea",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"TRI_HEIGHT",	"tri-height|triheight|tri-ht|triht|kcl-min|kclmin",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"FLAG_FILE",	"flag-file|flagfile",
		0, 0 /* copy of wszst */ },

  { H_OPT_GO,	"XTRIDATA",	"xtridata|xtri",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"SLOT",		"slot",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"ID",		"id",
		0, 0 /* copy of wszst */ },

  { T_OPT_CO,	"BASE64",	"base64",
		0, 0 /* copy of wszst */ },

  { T_OPT_CO,	"DB64",		"db64",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"CODING",	"coding",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"ROUND",	"round",
		0, 0 /* copy of wszst */ },

  { T_OPT_CM,	"LONG",		"l|long",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_WILDCARDS",	"no-wildcards|nowildcards",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"IN_ORDER",	"in-order|inorder",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_PARAM",	"P|no-param|noparam",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_ECHO",	"no-echo|noecho",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_CHECK",	"N|no-check|nocheck",
		0,
		"Do not make plausibility checks for KCL files." },

  { T_OPT_GP,	"MAX_FILE_SIZE","M|max-file-size|maxfilesize",
		0,
		0, 0 /* copy of wszst */ },

  { H_OPT_GP,	"TRACKS",	"T|tracks",
		0,
		0, 0 /* copy of wszst */ },

  { H_OPT_GP,	"ARENAS",	"A|arenas",
		0,
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

  { T_OPT_GP,	"TINY",		"tiny",
		0, 0 /* copy of wszst */ },

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
  //
  //---------- wkclt GROUP TRANSFORM ----------

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
  { T_COPT,	"TRANSLATE",	0,0,0 },
  { T_COPT,	"NULL",		0,0,0 },
  { T_COPT,	"NEXT",		0,0,0 },
  { T_COPT,	"ASCALE",	0,0,0 },
  { T_COPT,	"AROT",		0,0,0 },
  { T_COPT,	"TFORM_SCRIPT",	0,0,0 },
  { T_COPT,	"KCL",		0,0,0 },
  { T_COPT,	"KCL_FLAG",	0,0,0 },
  { T_COPT,	"KCL_SCRIPT",	0,0,0 },
  { T_COPT,	"TRI_AREA",	0,0,0 },
  { T_COPT,	"TRI_HEIGHT",	0,0,0 },
  { T_COPT,	"FLAG_FILE",	0,0,0 },
  { T_COPT,	"SLOT",		0,0,0 },

  //---------- wkclt GROUP DEST ----------

  { T_GRP_BEG,	"DEST",		0,0,0 },

  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"OVERWRITE",	0,0,0 },
  { T_COPT,	"NUMBER",	0,0,0 },
  { T_COPT,	"REMOVE_DEST",	0,0,0 },
  { T_COPT,	"UPDATE",	0,0,0 },
  { T_COPT,	"PRESERVE",	0,0,0 },

  //
  //---------- wkclt GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",		0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress the syntax information section in KCL text files." },
  { T_COPT,	"BRIEF",	0,0,0 },

  //
  //---------- COMMAND wkclt VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wkclt HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkclt CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wkclt ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkclt EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkclt TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },

  { T_COPT,	"VERBOSE",	0,0,
	"Print the matrices of transformations." },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkclt COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wkclt ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wkclt FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },

  //---------- COMMAND wkclt FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wkclt SYMBOLS ----------

  { T_CMD_BEG,	"SYMBOLS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wkclt FUNCTIONS ----------

  { T_CMD_BEG,	"FUNCTIONS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, FUNCTION_BRIEF },
  { T_COPT,	"LONG",		0,0, FUNCTION_LONG },

  //---------- COMMAND wkclt CALCULATE ----------

  { T_CMD_BEG,	"CALCULATE",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },

  //---------- COMMAND wkclt MATRIX ----------

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

  //---------- COMMAND wkclt FLOAT ----------

  { T_CMD_BEG,	"FLOAT",	0,0,0 },
  { T_COPT,	"ROUND",	0,0,0 },

  //---------- COMMAND wkclt EXPORT ----------

  { T_CMD_BEG,	"EXPORT",	0,0,0 },

  //---------- COMMAND wkclt CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },
  { T_COPY_CMD,	"DECODE",	0,0,0 },

  //---------- COMMAND wkclt COPY ----------

  { T_CMD_BEG,	"COPY",		0,0,0 },
  { T_COPY_CMD,	"ENCODE",	0,0,0 },

  //---------- COMMAND wkclt CFF ----------

  { T_CMD_BEG,	"CFF",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"LONG",		0,0,
		"Force usage of @'f(typ,variant)'@."
		" If set twice, force usage of @'a(typ,w,x,y,z)'@." },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress the comments with the alternative expression formats." },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"CONST",	0,0,0 },
  { T_COPT,	"FLAG_FILE",	0,0,0 },

  //---------- COMMAND wkclt TYPES ----------

  { T_CMD_BEG,	"TYPES",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Print only the resulting flag." },

  //---------- COMMAND wkclt FLAGS ----------

  { T_CMD_BEG,	"FLAGS",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress the output of detailed flag list"
		" and print only the type summary." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt DUMP ----------

  { T_CMD_BEG,	"DUMP",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress the output of the triangle list"
		" and print only the header data and statistics." },
  { T_COPT,	"LONG",		0,0,
		"If set, append an octree dump to the output."
		" If set twice, add a triangle usage list behind the octree dump." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt DBRIEF ----------

  { T_CMD_BEG,	"DBRIEF",	0,0,0 },
  { T_COPY_CMD,	"DUMP",		0,0,0 },

  //---------- COMMAND wkclt LIST ----------

  { T_CMD_BEG,	"LIST",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt TRIANGLES ----------

  { T_CMD_BEG,	"TRIANGLES",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Suppress the output of the 'normals' data." },
  { T_COPT,	"LONG",		0,0,
		"For each occurrence of option --long increase the precision of"
		" the floating points numbers by adding 1 fractional digit,"
		" but only up to additionally 5 digits." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt TRAVERSE ----------

  { T_CMD_BEG,	"TRAVERSE",	0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Print the 0-based triangle index instead of the triangle coordinates." },
  { T_COPT,	"LONG",		0,0,
		"If set, print a calculation log. If set twice,"
		" print a calculation log for each octree traversing step." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt FALL ----------

  { T_CMD_BEG,	"FALL",	0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"Print only the height (y coordinate) of the result." },
  { T_COPT,	"LONG",		0,0,
		"If set, print a calculation log."
		" If set twice, be more verbose." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt CHECK ----------

  { T_CMD_BEG,	"CHECK",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Print only warnings and suppress hints." },
  { T_COPT,	"LONG",	0,0,
	"Be verbose and print more statistics." },
  { T_COPT,	"QUIET",	0,0,
	"Be quiet until the first warning or hint is found." },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },

  //---------- COMMAND wkclt SHA1 ----------

  { T_CMD_BEG,	"SHA1",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"ID",		0,0,0 },
  { T_COPT,	"BASE64",	0,0,0 },
  { T_COPT,	"DB64",		0,0,0 },
  { T_COPT,	"CODING",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Suppress output of the filename." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt ANALYZE ----------

  { T_CMD_BEG,	"ANALYZE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkclt BLOW ----------

  { T_CMD_BEG,	"BLOW",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

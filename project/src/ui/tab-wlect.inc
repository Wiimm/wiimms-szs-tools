
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
/////////////			    wlect			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wlect", 0,
		"wlect [option]... command [option|parameter|@file]...",
		WLECT_LONG " :"
		" Manage the $LE-CODE$ and LEX extensions." },

  //
  //---------- list of all wlect commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION|V",
		    "wlect VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wlect HELP [--width=wd] OPT|OPTION ...\n"
		    "wlect HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wlect HELP [--width=wd] command [-b] [ignored]...\n"
		    "wlect HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wlect CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wlect ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wlect EXPAND ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wlect TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wlect COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wlect ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wlect FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wlect FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"SYMBOLS",	"SYMBOLS",
		    "wlect SYMBOLS",
		"Print predefined symbols of text parser to standard output."
		" The LE-CODE specific symbols are included." },

  { T_DEF_CMD,	"FUNCTIONS",	"FUNCTIONS",
		    "wlect FUNCTIONS [key]...",
		"Print a list with all parser functions including"
		" the LE-CODE specific functions."
		" If one key ore more key entered,"
		" then only functions that match at least one key are printed." },

  { T_DEF_CMD,	"CALCULATE",	"CALCULATE",
		    "wlect CALCULATE expression...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FLOAT",	"FLOAT",
		    "wlect FLOAT number...",
		0 /* copy of wszst */ },

  { H_DEF_CMD,	"EXPORT",	"_EXPORT",
		    "wszst _EXPORT key...",
		"This hidden command exports some definitions to standard output."
		" Use keyword @HELP@ to get a list of possible keywords." },

  { H_DEF_CMD,	"DPAD",		"_DPAD",
		    "wszst DPAD number_or_code...",
		"This hidden command decodes and encodes D-Pad suquences for LE-CODE." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"DUMP",		"DUMP|D",
		    "wlect DUMP [source]...",
		"Print an information dump for each source file."
		" " WILDCARDS_PIPE
		" LE-CODE binaries and LEX binaries are accepted as input." },

  { T_DEF_CMD,	"DB",		"DB"
		    "wlect DL [source]...",
		"Short cut for {DUMP --brief}." },

  { T_DEF_CMD,	"DL",		"DL",
		    "wlect DL [source]...",
		"Short cut for {DUMP --long}." },

  { T_DEF_CMD,	"DLL",		"DLL",
		    "wlect DLL [source]...",
		"Short cut for {DUMP --long --long}." },

  { T_DEF_CMD,	"DLLL",		"DLLL",
		    "wlect DLL [source]...",
		"Short cut for {DUMP --long --long --long}." },

  { T_DEF_CMD,	"BIN_DIFF",	"BIN-DIFF|BINDIFF|BD",
		    "wlect BIN-DIFF settings binary1 binary2",
		"Compare parts of 2 different LECODE binary files."
		" @SETTINGS@ is a string with plus and minus signs to switch between"
		" plus/minus modes, and some key letters to select parts:"
		" s:size, h:header, t:timestamp, b:body, p:parameters,"
		" d:data, c:code and a:all."
		" In plus-mode, a test is enabled, in minus-mode it is disabled."
		" Scanning starts in plus-mode."
		"\n "
		" Example: @a-t@: compare all except timestamps and edit infos."
		"\n "
		" The exist status is @OK@ (0) if nothing differ,"
		" @DIFFER@ (2) if at least one test failed, or an error code." },

  { T_DEF_CMD,	"PATCH",	"PATCH|P",
		    "wlect PATCH [lecode_bin]...",
		"Read each LE-CODE binary file and patch it."
		" " WILDCARDS_PIPE
		" Use {--le-define} to setup cups and tracks." },

  { H_DEF_CMD,	"TIMESTAMP",	"TIMESTAMP",
		    "wlect TIMESTAMP [lecode_bin]...",
		"Read each LE-CODE binary file and set creation time if exists and is null"
		" with respect to others existing timestamps."
		" Update the LE-CODE signature too."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"LPAR",		"LPAR",
		    "wlect LPAR [le_bin]...",
		"Read each LE-BIN file and patch it."
		" " WILDCARDS_PIPE
		" Then extract the LE-CODE parameters and store them as LPAR"
		" text file. Print the result to standard output if no other"
		" destination is defined." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"CREATE",	"CREATE|CR",
		    "wlect CREATE [keyword] ...",
		"Create a file and print it to standard output."
		" Options {--dest} and {--DEST} can force another destination."
		" The kind of file is defined by KEYWORD."
		" The command can create LEX files, a LPAR file or a track listing."
		"\n "
		" Use the command without keyword to get an extended description." },

  { T_DEF_CMD,	"DISTRIBUTION",	"DISTRIBUTION|DIS|DISTRIB",
		    "wlect DISTRIBUTION [argument]...",
		"This command manages data for LE-CODE track distributions."
		" It reads any number of source files with different file types,"
		" collects the data and creates any number of files with different file types."
		" " WILDCARDS_STEP
		"\n "
		" Use the command without keyword to get an extended description." },

  { T_DEF_CMD,	"CAT",		"CAT",
		    "wlect CAT [source]...",
		"Read the entered LEX and LE-CODE files (text, binary or szs)"
		" and print them as decoded text files to standard output."
		" " WILDCARDS_PIPE
		" The output of all source files is concatenated." },

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wlect DECODE [source]...",
		"Read the entered LEX files (text or binary)"
		" and decode them to text files."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.txt'@." },

  { T_DEF_CMD,	"ENCODE",	"ENCODE|ENC",
		    "wlect ENCODE source...",
		"Read the entered LEX files (text or binary)"
		" and encode (compile) them to binary files."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.lex'@." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  //
  //---------- list of all wlect options ----------

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
		"[=param]",
		"This option is completely ignored by wlect."
		" It only exists due to compatibility with the other tools." },

  { T_OPT_GP,	"LOAD_PREFIX",	"load-prefix|loadprefix",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"LOAD_CATEGORY","load-category|loadcategory",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"PLUS",		"plus",
		0, 0 /* copy of wszst */ },

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

  { T_OPT_C,	"NO_WILDCARDS",	"no-wildcards|nowildcards",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"IN_ORDER",	"in-order|inorder",
		0, 0 /* copy of wszst */ },

 #if 0 // BMG disabled for wlect

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
 #endif

  { T_OPT_C,	"EXPORT",	"X|export",
		0,
		"Enable the export mode and create small and machine readable text files"
		" for easy post processing." },

  { T_OPT_C,	"NO_PARAM",	"P|no-param|noparam",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_ECHO",	"no-echo|noecho",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"MAX_FILE_SIZE","M|max-file-size|maxfilesize",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { H_OPT_G,	"UTF_8",	"utf-8|utf8",
		0, 0 /* copy of wszst */ },

  { H_OPT_G,	"NO_UTF_8",	"no-utf-8|no-utf8|noutf8",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_GP,	"CACHE",	"cache",
		"dir",
		"Define a directory for an SZS cache."
		" The cache is used to generate LTA files when new compression is forced."
		" Option {--remove-dest} is set automatically to protect cache integrity."
		"\n "
		" Background: Loading a compressed SZS and content verification"
		" is much faster than compressing a file." },

  { T_OPT_GP,	"LOG_CACHE",	"log-cache|logcache",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"LT_CLEAR",	"lt-clear|ltclear",
		0,
		"Clear LEX section TEST and remove the section if empty."
		" This option is executed before all other --lt-* options." },

  { T_OPT_CP,	"LT_ONLINE",	"lt-online|ltonline",
		"value",
		"Set LEX/TEST parameter OFFLINE-ONLINE to this MODE."
		" MODE is one of @AUTO@ (default), @NEVER@ or @ALWAYS@."
		" If needed, LEX section TEST is created or removed if empty." },

  { T_OPT_CP,	"LT_N_PLAYERS",	"lt-n-players|lt-nplayers|ltnplayers",
		"offline,online",
		"Set LEX/TEST parameters N-OFFLINE and N-ONLINE to these integer values."
		" If needed, LEX section TEST is created or removed if empty." },

  { T_OPT_GP,	"LT_COND_BIT",	"lt-cond-bit|lt-condbit|ltcondbit",
		"bitnum",
		"Set LEX/TEST parameter COND-BIT to this integer value."
		" Use -1 to deactivate this COND-BIT."
		" If needed, LEX section TEST are created or removed if empty." },

  { T_OPT_GP,	"LT_GAME_MODE",	"lt-game-mode|lt-gamemode|ltgamemode",
		"mode",
		"Set LEX/TEST parameter GAME-MODE to this MODE."
		" MODE is one of @AUTO@ (default), @Balloon, COIN, VERSUS@ or @ITEMRAIN@."
		" If needed, LEX section TEST is created or removed if empty." },

  { T_OPT_GP,	"LT_ENGINE",	"lt-engine|ltengine",
		"mode",
		"Set LEX/TEST parameter ENGINE to this MODE."
		" MODE is one of @AUTO@ (default), @BATTLE, 50CC, 100CC, 150CC, 200CC,"
		" 150M@ or @200M@. Only track.szs files are modified."
		" If needed, LEX section TEST is created or removed if empty." },

  { T_OPT_GP,	"LT_RANDOM",	"lt-random|ltrandom",
		"index",
		"Force a random scenario, if INDEX is between 1 and 8."
		" Value 0 enables auto selection and -1 disables this option."
		" If needed, LEX section TEST is created or removed if empty." },

  { T_OPT_G,	"LEX_PURGE",	"lex-purge|lexpurge",
		0,
		"Delete LEX sections without any impact."
		" This option is executed after all --lt-* options." },

  { T_OPT_G,	"LEX_RM_FEAT",	"rm-features|rmfeatures|lex-rm-features|lexrmfeatures",
		0,
		"Remove LEX setion FEAT (features) if exists."
		" It is executed after --lex-purge."
		" {--lex-rm-features} is an alternative name." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_GM,	"TEST",		"t|test",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"FORCE",	"force",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"REPAIR_MAGICS",	"repair-magics|repairmagics",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"CREATE_DISTRIB",	"create-distrib|createdistrib",
		"file",
		"Create a LE-CODE distribution dump with a track list and more info about"
		" the current distribution. Store the dump into a file." },

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

  { T_OPT_CP,	"LE_DEFINE",	"le-define|ledefine",
		"file",
		"Use this file as definition file to patch LE-CODE."
		" The file format can be LE-BIN or any supported CT-CODE file"
		" (BRRES, TEX0, CT-CODE or CT-TEXT)." },

  { H_OPT_CP,	"LE_ARENA",	"le-arena|learena",
		"file",
		"Use this file to define property and music defaults for arenas."
		" The file format can be an text file."
		" Option {--le-define} may override these defaults." },

  { T_OPT_CP,	"LPAR",		"lpar",
		"file",
		"Read a LPAR file to overwrite settings of LE-CODE section LPAR." },

  { T_OPT_CP,	"ALIAS",	"alias",
		"list",
		"The parameter is a comma separated list of @SLOT=ALIAS@ elements."
		" SLOT defines the slot, for which the ALIAS is defined."
		" ALIAS itself is a slot number to be executed, if SLOT is selected."
		" Empty and invalid elements are ignored."
		" Multiple usage is possible: the lists are concatenated." },

  { T_OPT_CP,	"ENGINE",	"engine",
		"list",
		"Define probabilities for engine classes."
		" @LIST@ is a vector for @100cc,150cc,mirror@"
		" If 200cc is enabled by {--200cc},"
		" then vector is interpreted as @150cc,200cc,mirror@."
		" Any numbers are accepted."
		" They are normalized to get a total of 100 percent."
		" Use @0,1,0@ to force 150cc only."
		" Use @0,0,0@ to reset to Nintendos VR based choice." },

  { T_OPT_CO,	"200CC",	"200cc",
		"[=mode]",
		"Enable or disable 200cc support."
		" Mode is one of @OFF@, @AUTO@ (default) or @ON@"
		" (default, if option is used without parameter)."
		" LE-CODE hasn't implemented this feature yet!" },

  { T_OPT_CO,	"PERFMON",	"perf-mon|perfmon",
		"[=mode]",
		"Enable or disable the performance monitor."
		" Mode is one of @OFF@, @AUTO@ (default, don't change setting),"
		" @ON@ (enable it for Wii and Wii U only;"
		" default, if option is used without parameter)"
		" or @FORCE@ (enable it for Wii, Wii U and for Dolphin)." },

  { T_OPT_CO,	"CUSTOM_TT",	"custom-tt|customtt",
		"[=mode]",
		"Enable or disable time trial for custom tracks."
		" Mode is one of @OFF@, @AUTO@ (default) or @ON@"
		" (default, if option is used without parameter)." },

  { T_OPT_CO,	"XPFLAGS",	"xpflags",
		"[=mode]",
		"Enable or disable support for Extended Presence Flags."
		" Mode is one of @OFF@, @AUTO@ (default) or @ON@"
		" (default, if option is used without parameter)."
		" Disable it only for tests!" },

  { T_OPT_CO,	"SPEEDOMETER",	"speedometer",
		"[=mode]",
		"Enable or disable the speedometer."
		" Mode is one of @OFF@, @AUTO@ (default, don't change setting),"
		" @ON@ (default, if option is used without parameter)"
		" or @FRACTION@ (=@1DIGIT@), @2DIGITS@ and @3DIGITS@." },

  { T_OPT_CP,	"DEBUG",	"debug",
		"mode",
		"Enable or disable the debug output while racing."
		" Mode is one of @OFF@, @AUTO@ (default, don't change setting),"
		" @USER@ or @100@ to @199@ for predefined setups." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"TRACK_DIR",	"track-dir|trackdir",
		"directory",
		"Define a destination directory,"
		" where track files are copied, moved or linked to."
		" Usually it is @.../Race/Course/@."
		"\n "
		" Files are searched in directories defined by {--copy-tracks},"
		" {--move-tracks}, {--move1-tracks} and {--link-tracks}"
		" in definition order."
		" Already existing files are removed before operation." },

  { T_OPT_CMP,	"COPY_TRACKS",	"copy-tracks|copytracks",
		"directory",
		"Define a search directory for all included tracks."
		" Files found in this directory are copied to the"
		" directory specified by option {--track-dir}." },

  { T_OPT_CMP,	"MOVE_TRACKS",	"move-tracks|movetracks",
		"directory",
		"Define a search directory for all included tracks."
		" Files found in this directory are moved to the"
		" directory specified by option {--track-dir}."
		" If moving fails, the file is copied and the source removed." },

  { T_OPT_CMP,	"MOVE1_TRACKS",	"move1-tracks|move1tracks",
		"directory",
		"Define a search directory for all included tracks."
		" Files found in this directory are moved to the"
		" directory specified by option {--track-dir},"
		" but only if a file has not more than one hard link."
		" This guarantees an unique version of the file."
		" If the file has more hard links or moving fails,"
		" the file is copied and the source removed." },

  { T_OPT_CMP,	"LINK_TRACKS",	"link-tracks|linktracks",
		"directory",
		"Define a search directory for all included tracks."
		" Files found in this directory are linked to the"
		" directory specified by option {--track-dir}."
		" If linking fails, the file is copied." },

  { T_OPT_CP,	"SZS_MODE",	"szs-mode|szsmode",
		"mode",
		"If a track is inserted via the SZS file,"
		" then the associated directory is automatically included in the list"
		" of search directories for option {--track-dir}."
		" This option now sets the transfer mode. @OFF@ turns this feature off."
		" The values @COPY@, @MOVE@, @MOVE1@ and @LINK@ define one of the 4 modes."
		" Default mode is @LINK@." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"COMPLETE",	"C|complete",
		0,
		" Append missing sections to the LEX file." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CMP,	"LOAD_BMG",	"load-bmg|loadbmg",
		0, 0 /* copy of wszst */ },

  { T_OPT_CMP,	"PATCH_BMG",	"patch-bmg|patchbmg|pb",
		"mode[cond][=param]",
		PATCH_BMG_TEXT("") },

  { T_OPT_CMP,	"MACRO_BMG",	"macro-bmg|macrobmg|mb",
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
  //---------- wlect GROUP DEST ----------

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

  //---------- wlect GROUP PARAM ----------

  { T_GRP_BEG,	"PARAM",	0,0,0 },

  { T_COPT,	"ENGINE",	0,0,0 },
  { T_COPT,	"200CC",	0,0,0 },
  { T_COPT,	"PERFMON",	0,0,0 },
  { T_COPT,	"CUSTOM_TT",	0,0,0 },
  { T_COPT,	"XPFLAGS",	0,0,0 },
  { T_COPT,	"SPEEDOMETER",	0,0,0 },
  { T_COPT,	"DEBUG",	0,0,0 },

  //---------- wlect GROUP PATCH ----------

  { T_GRP_BEG,	"PATCH",	0,0,0 },

  { T_COPT,	"LE_DEFINE",	0,0,0 },
  { H_COPT,	"LE_ARENA",	0,0,0 },
  { T_COPT,	"LPAR",		0,0,0 },
  { T_COPT,	"ALIAS",	0,0,0 },
  { T_COPY_GRP,	"PARAM",	0,0,0 },

  //---------- wlect GROUP PATCH_LEX ----------

//  { T_GRP_BEG,	"PATCH_LEX",	0,0,0 },
//  { T_COPT,	"LT_CLEAR",	0,0,0 },
//  { T_COPT,	"LT_ONLINE",	0,0,0 },
//  { T_COPT,	"LT_N_PLAYERS",	0,0,0 },
//  { T_COPT,	"LT_COND_BIT",	0,0,0 },
//  { T_COPT,	"LT_GAME_MODE",	0,0,0 },
//  { T_COPT,	"LT_ENGINE",	0,0,0 },
//  { T_COPT,	"LT_RANDOM",	0,0,0 },
//  { T_COPT,	"LEX_PURGE",	0,0,0 },
//  { T_COPT,	"LEX_RM_FEAT",	0,0,0 },

  //---------- wlect GROUP TRACK_COPY ----------

  { T_GRP_BEG,	"TRACK_COPY",	0,0,0 },

  { T_COPT,	"TRACK_DIR",	0,0,0 },
  { T_COPT,	"COPY_TRACKS",	0,0,0 },
  { T_COPT,	"MOVE_TRACKS",	0,0,0 },
  { T_COPT,	"MOVE1_TRACKS",	0,0,0 },
  { T_COPT,	"LINK_TRACKS",	0,0,0 },

  //---------- wlect GROUP TEXTOPT ----------

  { T_GRP_BEG,	"TEXTOPT",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress the syntax information section in LE text files." },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },

  //---------- wlect GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",		0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"TEXTOPT",	0,0,0 },

  //
  //---------- COMMAND wlect VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wlect HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wlect CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wlect ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wlect EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wlect TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wlect COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wlect ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wlect FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },

  //---------- COMMAND wlect FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wlect SYMBOLS ----------

  { T_CMD_BEG,	"SYMBOLS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wlect FUNCTIONS ----------

  { T_CMD_BEG,	"FUNCTIONS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, FUNCTION_BRIEF },
  { T_COPT,	"LONG",		0,0, FUNCTION_LONG },

  //---------- COMMAND wlect CALCULATE ----------

  { T_CMD_BEG,	"CALCULATE",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },

  //---------- COMMAND wlect FLOAT ----------

  { T_CMD_BEG,	"FLOAT",	0,0,0 },
  { T_COPT,	"ROUND",	0,0,0 },

  //---------- COMMAND wlect _EXPORT ----------

  { T_CMD_BEG,	"EXPORT",	0,0,0 },

  //---------- COMMAND wlect _DPAD ----------

  { T_CMD_BEG,	"DPAD",		0,0,0 },

  //---------- COMMAND wlect DUMP ----------

  { T_CMD_BEG,	"DUMP",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
		"For LE-CODE binaries:"
		" If set, print infos about file header only." },
  { T_COPT,	"LONG",		0,0,
		"For LE-CODE binaries:"
		" If set, print cups with tracks or arenas."
		" For each track, property and music slot is printed in brackets."
		" If set twice, print a track list too."
		" If set 3 times, print a track slot usage map too (same as -v)."
		"\n "
		" For LEX binaries:"
		" If set, print a hex dump of max 16 bytes (1 line) of the element data."
		" If set twice, print a hex dump of max 64 bytes (4 lines)."
		" If set 3 times ore more, print a complete hex dump." },
  { T_COPT,	"VERBOSE",		0,0,
		"For LE-CODE binaries: If set, print a track slot usage map." },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { T_COPT,	"CREATE_DISTRIB",0,0,0 },

  //---------- COMMAND wlect DB ----------

  { T_CMD_BEG,	"DB",		0,0,0 },
  { T_COPY_CMD,	"DUMP",		0,0,0 },

  //---------- COMMAND wlect DL ----------

  { T_CMD_BEG,	"DL",		0,0,0 },
  { T_COPY_CMD,	"DUMP",		0,0,0 },

  //---------- COMMAND wlect DLL ----------

  { T_CMD_BEG,	"DLL",		0,0,0 },
  { T_COPY_CMD,	"DUMP",		0,0,0 },

  //---------- COMMAND wlect DLLL ----------

  { T_CMD_BEG,	"DLLL",		0,0,0 },
  { T_COPY_CMD,	"DUMP",		0,0,0 },

  //---------- COMMAND wlect BIN-DIFF ----------

  { T_CMD_BEG,	"BIN_DIFF",	0,0,0 },

  { T_COPT,	"QUIET",	0,0,
		"Suppress all messages and tell result as exit status only." },
  { T_COPT,	"VERBOSE",	0,0,
		"Be more verbose and tell, what tests are planned." },

  //---------- COMMAND wlect PATCH ----------

  { T_CMD_BEG,	"PATCH",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { T_COPY_GRP,	"TRACK_COPY",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
//  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"CREATE_DISTRIB",0,0,0 },

  //---------- COMMAND wlect TIMESTAMP ----------

  { T_CMD_BEG,	"TIMESTAMP",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
//  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wlect LPAR ----------

  { T_CMD_BEG,	"LPAR",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },

  //---------- COMMAND wlect CREATE ----------

  { T_CMD_BEG,	"CREATE",	0,0,0 },

  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"LPAR",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"LT_CLEAR",	0,0,0 },
  { T_COPT,	"LT_ONLINE",	0,0,0 },
  { T_COPT,	"LT_N_PLAYERS",	0,0,0 },
  { T_COPT,	"LT_COND_BIT",	0,0,0 },
  { T_COPT,	"LT_GAME_MODE",	0,0,0 },
  { T_COPT,	"LT_ENGINE",	0,0,0 },
  { T_COPT,	"LT_RANDOM",	0,0,0 },
  { T_COPT,	"LEX_PURGE",	0,0,0 },
  { T_COPT,	"LEX_RM_FEAT",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"LE_DEFINE",	0,0,0 },
  { T_COPT,	"LOAD_BMG",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"PARAM",	0,0,0 },

  //---------- COMMAND wlect DISTRIBUTION ----------

  { T_CMD_BEG,	"DISTRIBUTION",	0,0,0 },

  { T_COPT,	"LOAD_PREFIX",	0,0,0 },
  { T_COPT,	"LOAD_CATEGORY",0,0,0 },
  { T_COPT,	"CACHE",	0,0,0 },
  { T_COPT,	"LOG_CACHE",	0,0,0 },
  { T_COPT,	"PLUS",		0,0,0 },
  { T_COPY_GRP,	"TRACK_COPY",	0,0,0 },
  { T_COPT,	"SZS_MODE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },

  //---------- COMMAND wlect CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COMPLETE",	0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },

  //---------- COMMAND wlect DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COMPLETE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },

  //---------- COMMAND wlect ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COMPLETE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

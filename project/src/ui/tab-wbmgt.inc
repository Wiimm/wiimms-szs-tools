
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
/////////////			    wbmgt			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wbmgt", 0,
		"wbmgt [option]... command [option|parameter|@file]...",
		WBMGT_LONG " :"
		" Convert and patch raw and text BMG (message) files." },

  //
  //---------- list of all wbmgt commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION|V",
		    "wbmgt VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wbmgt HELP [--width=wd] OPT|OPTION ...\n"
		    "wbmgt HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wbmgt HELP [--width=wd] command [-b] [ignored]...\n"
		    "wbmgt HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wbmgt CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wbmgt ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wbmgt EXPAND ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wbmgt TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wbmgt COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wbmgt ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wbmgt FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wbmgt FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"POINTS",	"POINTS|PTS",
		    "wbmgt POINTS [--points=list]",
		"Print a table with the points assigned to player after"
		" a versus or room race." },

  { T_DEF_CMD,	"REGEXP",	"REGEXP|RE",
		    "wbmgt REGEXP pattern string...",
		"Test implementation of regular expressions."
		" PATTERN is the regular expression to find matches."
		" Each STRING is object of the match and replace operation." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"EXTRACT",	"EXTRACT",
		    "wbmgt EXTRACT name source",
		"Extract BMG section NAME of file SOURCE and write"
		" the binary data to standard output."
		" If NAME consists of less than 4 characters, then the first"
		" section that starts with these characters is dumped." },

  { T_DEF_CMD,	"SECTIONS",	"SECTIONS",
		    "wbmgt SECTIONS [source]...",
		"List all sections of each binary BMG file."
		" Print offset, size, magic and a short info."
		" If known, print number and size of elements too."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"LIST",		"LIST|LS|LL",
		    "wbmgt LIST [source]...",
		"List messages to proof the content."
		" Patches are applied before listing."
		" The lines are cut to fit the terminal width."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"SLOTS",	"SLOTS",
		    "wbmgt SLOTS [source]...",
		"Print a slot ordered list with slot number, message id,"
		" delta and attributes. Patches are applied before listing."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"DIFF",		"DIFF|CMP",
		    "wbmgt DIFF source1 source2\n"
		    "wbmgt DIFF source... --dest dest\n",
		"Compare two or more BMG (raw or text) files"
		" on string level and report mismatches for each string."
		" If option {--dest} is set, then each source file"
		" is compared with a file in the destination."
		" Patches are applied to all sources before comparing." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"CAT",		"CAT",
		    "wbmgt CAT [source]...",
		"Read the entered BMG files (text, binary or szs sub file) and print"
		" them as decoded text files to standard output."
		" All BMG files of a single archive (SZS,...) are combined to 1 source."
		" The output of all source files is concatenated."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"MIX",		"MIX",
		    "wbmgt MIX [source]...",
		"Read the entered BMG files (text, binary or szs sub file) and print"
		" them as decoded text files to standard output."
		" All BMG files of all archives (SZS,...) are read."
		" The output of all source files is combined"
		" and strings of later source files override prior defined strings."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"IDENTIFIER",	"IDENTIFIER|ID|IDENTIFY",
		    "wbmgt IDENTIFIER",
		"Create a simple text BMG with the identifier message @3DEF@ only"
		" with respect to options {--le-code} and {--ct-code}."
		" Print the result to standard output." },

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wbmgt DECODE [szs]...",
		"Read the entered message files and decode them to text files."
		" All BMG files of a single archive (SZS,...) are concatenated to 1 source."
		" This command is similar to {PATCH},"
		" but the result is always decoded."
		" The default destination is @'%P/%N.txt'@."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"ENCODE",	"ENCODE|ENC",
		    "wbmgt ENCODE [source]...",
		"Read the entered message files and encode them to BMG files."
		" All BMG files of a single archive (SZS,...) are concatenated to 1 source."
		" This command is similar to {PATCH},"
		" but the result is always encoded."
		" The default destination is @'%P/%N.bmg'@."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"PATCH",	"PATCH|PAT",
		    "wbmgt PATCH [source]...",
		"Patch the entered files by using the patch list"
		" specified by option {--patch-bmg}."
		" The coding of the source files is not changed."
		" The default destination is @'%P/%F'@."
		" " WILDCARDS_PIPE
		"\n "
		" Use {wszst patch --patch-bmg ...} to patch BMG files within a SZS file." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  //
  //---------- list of all wbmgt options ----------

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
		0, 0 /* copy of wszst */ },

  { T_OPT_GMP,	"CHDIR",	"chdir",
		0, 0 /* copy of wszst */ },

  { T_OPT_GMP,	"CONST",	"c|const",
		0, 0 /* copy of wszst */ },

  { T_OPT_CM,	"LONG",		"l|long",
		0, BMG_LONG_TEXT },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, "Suppress the syntax information section in BMG text files." },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0, BMG_BRIEF_TEXT },

  { T_OPT_C,	"NO_WILDCARDS",	"no-wildcards|nowildcards",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"IN_ORDER",	"in-order|inorder",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"MAX_FILE_SIZE","M|max-file-size|maxfilesize",
		0,
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"TRACKS",	"T|tracks",
		0,
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"ARENAS",	"A|arenas",
		0,
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"POINTS",	"points|pts",
		"list",
		"This option creates the new message 3FF0 to hold"
		" a description of the versus points table."
		" It " OPT_POINTS },

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

  { T_OPT_CP,	"ALIGN",	"align",
		"size",
		"Define an align value for the sub sections in the archive."
		" The value must be a power of 2 and the default value is 32 (0x20)" },

  { T_OPT_CMP,	"PATCH_BMG",	"P|patch-bmg|patchbmg|pb|patch",
		"mode[cond][=param]",
		PATCH_BMG_TEXT(" @--patch@ is a short cut.") },

  { T_OPT_CMP,	"MACRO_BMG",	"macro-bmg|macrobmg|mb",
		0, 0 /* copy of wszst */ },

  { T_OPT_CMP,	"FILTER_BMG",	"filter-bmg|filterbmg|fb|msg",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"BMG_ENDIAN",	"endian|bmg-endian|bmgendian",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"BMG_ENCODING",	"encoding|bmg-encoding|bmgencoding",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"BMG_INF_SIZE",	"inf-size|infsize|bmg-inf-size|bmginfsize",
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
		0, 0 /* copy of wszst */ },

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
		" The option works similar like @-HBl11@ for BMG text files." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"SECTIONS",	"sections",
		0, 0 /* copy of wszst */ },

  //
  //---------- wbmgt GROUP DEST ----------

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
  //---------- wbmgt GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
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
  { T_COPT,	"LONG",		0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },

  //
  //---------- COMMAND wbmgt VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wbmgt HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wbmgt CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wbmgt ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wbmgt EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wbmgt TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wbmgt COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wbmgt ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wbmgt FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },

  //---------- COMMAND wbmgt FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wbmgt POINTS ----------

  { T_CMD_BEG,	"POINTS",	0,0,0 },
  { T_COPT,	"POINTS",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Print the table as comma separated lists in C notation." },
  { T_COPT,	"LONG",		0,0,
	"Append a string in {--points} format that defines the table."
	" If set twice, append up to 3 strings in different modes." },

  //---------- COMMAND wbmgt REGEXP ----------

  { T_CMD_BEG,	"REGEXP",	0,0,0 },

  //---------- COMMAND wbmgt EXTRACT ----------

  { T_CMD_BEG,	"EXTRACT",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
		"Exclude the section header from output." },

  //---------- COMMAND wbmgt SECTIONS ----------

  { T_CMD_BEG,	"SECTIONS",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wbmgt LIST ----------

  { T_CMD_BEG,	"LIST",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },

  //---------- COMMAND wbmgt SLOTS ----------

  { T_CMD_BEG,	"SLOTS",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },

  //---------- COMMAND wbmgt DIFF ----------

  { T_CMD_BEG,	"DIFF",		0,0,0 },
  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },

  //---------- COMMAND wbmgt CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },

  //---------- COMMAND wbmgt MIX ----------

  { T_CMD_BEG,	"MIX",		0,0,0 },

  { T_COPY_CMD,	"CAT",		0,0,0 },

  //---------- COMMAND wbmgt IDENTIFIER ----------

  { T_CMD_BEG,	"IDENTIFIER",	0,0,0 },

  { T_COPT,	"LE_CODE",	0,0,0 },
  { T_COPT,	"CT_CODE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },

  //---------- COMMAND wbmgt DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },

  //---------- COMMAND wbmgt ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"BMG_ENDIAN",	0,0,0 },
  { T_COPT,	"BMG_ENCODING",	0,0,0 },
  { T_COPT,	"BMG_INF_SIZE",	0,0,0 },
  { T_COPT,	"BMG_MID",	0,0,0 },
  { T_COPT,	"FORCE_ATTRIB",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },
  { T_COPT,	"ALIGN",	0,0,0 },

  //---------- COMMAND wbmgt PATCH ----------

  { T_CMD_BEG,	"PATCH",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"PATCH_BMG",	0,0,0 },
  { T_COPT,	"MACRO_BMG",	0,0,0 },
  { T_COPT,	"FILTER_BMG",	0,0,0 },
  { T_COPT,	"ALIGN",	0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

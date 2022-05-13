
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
/////////////			    wmdlt			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wmdlt", 0,
		"wmdlt [option]... command [option|parameter|@file]...",
		WMDLT_LONG " :"
		" Decode raw MDL and encode text MDL files." },

  //
  //---------- list of all wmdlt commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION",
		    "wmdlt VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wmdlt HELP [--width=wd] OPT|OPTION ...\n"
		    "wmdlt HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wmdlt HELP [--width=wd] command [-b] [ignored]...\n"
		    "wmdlt HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wmdlt CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wmdlt ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wmdlt TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wmdlt COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wmdlt ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wmdlt FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wmdlt FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"SYMBOLS",	"SYMBOLS",
		    "wmdlt SYMBOLS",
		"Print predefined symbols of text parser to standard output."
		" The MDL specific symbols are included." },

  { T_DEF_CMD,	"FUNCTIONS",	"FUNCTIONS",
		    "wmdlt FUNCTIONS [key]...",
		"Print a list with all parser functions including the MDL specific functions."
		" If one key ore more key entered,"
		" then only functions that match at least one key are printed." },

  { T_DEF_CMD,	"CALCULATE",	"CALCULATE",
		    "wmdlt CALCULATE expression...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"MATRIX",	"MATRIX",
		    "wmdlt MATRIX [patch-option]... [vector]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FLOAT",	"FLOAT",
		    "wmdlt FLOAT number...",
		0 /* copy of wszst */ },

  { H_DEF_CMD,	"EXPORT",	"_EXPORT",
		    "wszst _EXPORT key...",
		"This hidden command exports some definitions to standard output."
		" Use keyword @HELP@ to get a list of possible keywords." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"CAT",		"CAT",
		    "wmdlt CAT [source]...",
		"Read the entered MDL files (text or binary) and print"
		" them as decoded text files to standard output (stdout)."
		" The output of all source files is concatenated."
		"\n "
		" A decoded MDL0 file can only be used for analysis."
		" Encoding or creating a new MDL0 file is not supported." },

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wmdlt DECODE [source]...",
		"Read the entered MDL files (text or binary)"
		" and decode them to text files."
		" The default destination is @'%P/%N.txt'@."
		"\n "
		" A decoded MDL0 file can only be used for analysis."
		" Encoding or creating a new MDL0 file is not supported." },

  { H_DEF_CMD,	"ENCODE",	"ENCODE|ENC",
		    "wmdlt ENCODE [source]...",
		"Read the entered MDL files (text or binary)"
		" and encode (compile) them to binary files."
		" The default destination is @'%P/%N.mdl'@." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"STRINGS",	"STRINGS",
		    "wmdlt STRINGS [source]...",
		"Read the entered MDL files (text or binary)"
		" and print all strings of all sections."
		"\n "
		" For archives like SZS or BREES,"
		" the strings of each containing MDL are printed."
		" It is also possible to address an internal sub-file like"
		" @track.szs/course_model.brres@." },

  { H_DEF_CMD,	"GEOMETRY",	"GEOMETRY|GEO",
		    "wmdlt GEOMETRY [source]...",
		"Read the entered MDL files (text or binary) and print"
		" some geometry information to the standard output (stdout)."
		" The output of all source files is concatenated." },

  { H_DEF_CMD,	"XTEST",	"_TEST",
		    "wszst _TEST [files]",
		"Test something." },

  //
  //---------- list of all wmdlt options ----------

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

  { T_OPT_GM,	"QUIET",	"q|quiet",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"VERBOSE",	"v|verbose",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"LOGGING",	"L|logging",
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

  { T_OPT_GP,	"MDL",		"mdl",
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

  { T_OPT_C,	"ROUND",	"round",
		0, 0 /* copy of wszst */ },

  { T_OPT_CM,	"LONG",		"l|long",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_PARAM",	"P|no-param|noparam",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_ECHO",	"no-echo|noecho",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_CHECK",	"N|no-check|nocheck",
		0, 0 /* copy of wszst */ },

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
  //---------- wmdlt GROUP TRANSFORM ----------

  { T_GRP_BEG,	"TRANSFORM",	0,0,0 },

  //{ T_COPT,	"CONST",	0,0,0 },
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
  { T_COPT,	"ASCALE",	0,0,0 },
  { T_COPT,	"AROT",		0,0,0 },
  { T_COPT,	"TFORM_SCRIPT",	0,0,0 },
  { T_COPT,	"NEXT",		0,0,0 },
  { T_COPT,	"MDL",		0,0,0 },


  //---------- wmdlt GROUP DEST ----------

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
  //---------- wmdlt GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",		0,0,0 },

  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress the syntax information section in MDL text files." },
  { T_COPT,	"BRIEF",	0,0,0 },

  //
  //---------- COMMAND wmdlt VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wmdlt HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wmdlt CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wmdlt ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },

  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wmdlt TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },

  { T_COPT,	"VERBOSE",	0,0,
	"Print the matrices of transformations." },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wmdlt COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wmdlt ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wmdlt FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wmdlt FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wmdlt SYMBOLS ----------

  { T_CMD_BEG,	"SYMBOLS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wmdlt FUNCTIONS ----------

  { T_CMD_BEG,	"FUNCTIONS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, FUNCTION_BRIEF },
  { T_COPT,	"LONG",		0,0, FUNCTION_LONG },

  //---------- COMMAND wmdlt CALCULATE ----------

  { T_CMD_BEG,	"CALCULATE",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },

  //---------- COMMAND wmdlt MATRIX ----------

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

  //---------- COMMAND wmdlt FLOAT ----------

  { T_CMD_BEG,	"FLOAT",	0,0,0 },
  { T_COPT,	"ROUND",	0,0,0 },

  //---------- COMMAND wmdlt EXPORT ----------

  { T_CMD_BEG,	"EXPORT",	0,0,0 },

  //---------- COMMAND wmdlt CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wmdlt DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wmdlt ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wmdlt STRINGS ----------

  { T_CMD_BEG,	"STRINGS",	0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wmdlt GEOMETRY ----------

  { T_CMD_BEG,	"GEOMETRY",	0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wmdlt _TEST ----------

  { T_CMD_BEG,	"XTEST",	0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////


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
/////////////			    wkmpt			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wkmpt", 0,
		"wkmpt [option]... command [option|parameter|@file]...",
		WKMPT_LONG " :"
		" Analyze, modify, export and create (compile) KMP files." },

  //
  //---------- list of all wkmpt commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION|V",
		    "wkmpt VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wkmpt HELP [--width=wd] OPT|OPTION ...\n"
		    "wkmpt HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wkmpt HELP [--width=wd] command [-b] [ignored]...\n"
		    "wkmpt HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wkmpt CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wkmpt ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wkmpt EXPAND ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wkmpt TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wkmpt COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wkmpt ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wkmpt FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wkmpt FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"SYMBOLS",	"SYMBOLS",
		    "wkmpt SYMBOLS",
		"Print predefined symbols of text parser to standard output."
		" The KMP specific symbols are included." },

  { T_DEF_CMD,	"FUNCTIONS",	"FUNCTIONS",
		    "wkmpt FUNCTIONS [key]...",
		"Print a list with all parser functions including the KMP specific functions."
		" If one key ore more key entered,"
		" then only functions that match at least one key are printed." },

  { T_DEF_CMD,	"CALCULATE",	"CALCULATE",
		    "wkmpt CALCULATE expression...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"MATRIX",	"MATRIX",
		    "wkmpt MATRIX [patch-option]... [vector]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FLOAT",	"FLOAT",
		    "wkmpt FLOAT number...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"STARTPOS",	"STARTPOS|SP",
		    "wkmpt STARTPOS [patch-option]... key_or_file...",
		"Print an ASCII image or a table of the start positions."
		" The parameter list is a wild mix of filenames, keywords and numbers."
		"\n "
		" If a parameter contains a slash ('/' or '\\'),"
		" it is a filename of a KMP or SZS file and a KMP is loaded."
		" The KMP is used to print absolute positions and to find"
		" the settings for @LEFT/RIGHT@ and @WIDE/NARROW@ automatically."
		"\n"
		" A parameter may also be one of the following keywords"
		" to define a setting:"
		" @RESET, IMAGE, TABLE, RELATIVE, ABSOLUTE, LEFT, RIGHT, WIDE, NARROW,"
		" AUTO, NO-AUTO, DECIMAL, HEXADECIMAL, Z0, Z1, Z2, Z3@."
		" A table or image is only printed on numbers 1 to 12."
		"\1\n See https://szs.wiimm.de/cmd/wkmpt/startpos for details." },

  { T_DEF_CMD,	"OBJECTS",	"OBJECTS|OBJ",
		    "wkmpt OBJECTS key...",
		"Infos about all objects, that match any of the entered keys, are printed."
		" A key match, if the text is part of the object name, file list"
		" or info (ignoring case). If a key is a number (decimal or hex),"
		" the object with that id matches too."
		" This command uses an internal object and file database."
		"\1 An online query is also available:"
		" https://szs.wiimm.de/cgi/mkw/object" },

  { H_DEF_CMD,	"EXPORT",	"_EXPORT",
		    "wkmpt _EXPORT key...",
		0 /* copy of wszst */ },

  { H_DEF_CMD,	"XEXPORT",	"_XEXPORT",
		    "wkmpt _XEXPORT",
		"Export some tables for LE-CODE." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"CAT",		"CAT",
		    "wkmpt CAT [source]...",
		"Read the entered KMP files (text, binary or szs) and print"
		" them as decoded text files to standard output."
		" " WILDCARDS_PIPE
		" The output of all source files is concatenated." },

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wkmpt DECODE [source]...",
		"Read the entered KMP files (text or binary)"
		" and decode them to text files."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.txt'@." },

  { T_DEF_CMD,	"ENCODE",	"ENCODE|ENC",
		    "wkmpt ENCODE source...",
		"Read the entered KMP files (text or binary)"
		" and encode (compile) them to binary files."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.kmp'@." },

  { T_DEF_CMD,	"DIFF",		"DIFF|CMP",
		    "wkmpt DIFF source1 source2\n"
		    "wkmpt DIFF source... --dest dest",
		"Compare the KMP of two or more sources (KMP, KMP-text or SZS)."
		" Each KMP will be converted into the internal format,"
		" and then fixed and patched controlled by options."
		" Only the known sections are compared."
		" If option SORT (see {--diff}) is set,"
		" elements of section GOBJ are sorted by object-id."
		"\n "
		" This command is in development. So for the section @POTI@"
		" only a simple notice about the different data record is printed."
		"\n "
		" If option {--dest} is set, then each source file"
		" is compared with a file in the destination." },

  { T_DEF_CMD,	"DRAW",		"DRAW|D|COBJECT|COB",
		    "wkmpt DRAW source...",
		"Read the entered KMP files (text or binary),"
		" of needed load the related KCL file,"
		" draw KMP objects and create an OBJ file."
		" " WILDCARDS_PIPE
		" The default destination is @'%P/%N.kmp.obj'@."
		"\n "
		" Automated dropping of bad KCL triangles is enabled"
		" if no drop option is specified by option {--kcl}." },

  { T_DEF_CMD,	"CHECK",	"CHECK|CHK",
		    "wkmpt CHECK source...",
		"Read the entered KMP files (text or binary) and"
		" and make a plausibility check."
		" " WILDCARDS_PIPE
		" Hints and warnings are written to standard output."
		" The exit status is @'DIFFER'@ if at least one KMP warning found." },

  { T_DEF_CMD,	"STGI",		"STGI",
		    "wkmpt STGI file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"KTPT",		"KTPT",
		    "wkmpt KTPT [source]...",
		"Analyse the start positions (KMP:KTPT) and lap counters"
		" (KMP:CKPT with mode 0) and print a simple information list."
		" Use enemy points (KMP:ENPT) to find recommendations."
		"\n "
		" This command is dedicated to LE-CODE, which supports an"
		" alternative position for the finish line at the minimap." },

  { T_DEF_CMD,	"ROUTES",	"ROUTES",
		    "wkmpt ROUTES file...",
		"Print the internal cross references for routes of KMP sections"
		" CKPH, CKPT, ENPH, ENPT, ITPH and ITPT."
		" " WILDCARDS_PIPE },

  { H_DEF_CMD,	"GOBJ",		"GOBJ",
		    "wkmpt GOBJ [source]...",
		"Analyze the objects of KMP section GOBJ"
		" and print a status line for each object."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"GAMEMODES",	"GAMEMODES|GMODES",
		    "wkmpt GAMEMODES [source]...",
		"Analyze the presence flags combinations"
		" with and without extension support and print a list."
		" The lists consists of index, scenario version and scenario name."
		" Scenario versions are only equal, if the scenarios are duplicates."
		" Duplicates happens only, if {--gamemodes} option TT or FULL is set."
		" " WILDCARDS_PIPE },

  { T_DEF_CMD,	"WIM0",		"WIM0|W0",
		    "wkmpt WIM0 [source]...",
		"Analyze the WIM0 section of each KMP file (text, binary or szs)"
		" and print an info about it." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  //
  //---------- list of all wkmpt options ----------

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

  { T_OPT_CP,	"YPOS",		"ypos",
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

  { T_OPT_CP,	"RM_GOBJ",	"rm-gobj|rmgobj",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"BATTLE",	"battle",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"EXPORT_FLAGS",	"export-flags|exportflags|exflags",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"ROUTE_OPTIONS", "route-options|routeoptions|ropt",
		0, 0 /* copy of wszst */ },

  { T_OPT_GO,	"WIM0",		"wim0",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"SLOT",		"slot",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"DRAW",		"w|draw",
		"list",
		"The parameter is a comma separated list of keywords to select"
		" the elements to draw."
		" Allowed keywords are: @CKPT, CJGPT, JGPT, KTPT, ENPT, ITPT,"
		" POTI, CNPT, ITEMBOXES, COINS, ROADOBJECTS, SOLIDOBJECTS, DECORATION,"
		" BLACK, WHITE, KCL, NONE@ and @ALL@ (default);"
		" the classes @POINTS, OBJECTS@ and @RESPAWN@;"
		" the presence-flag selectors @[1-3]OFFLINE@ and @[1-3]ONLINE@;"
		" the special options @DETAILED, DISPATCH@ and @WARNINGS@."
		" If a keyword is prefixed by a minus sign, the option is disabled."
		" Option @KCL@ decides, if the reference KCL is included in the output."
		" If @DETAILED@ is disabled, an output with less faces is created."
		" If @DISPATCH@ is enabled, the settings of dispatch points are visualized."
		" If @WARNINGS@ is enabled, some objects are drawn in red."
		"\1 See https://szs.wiimm.de/opt/draw for details." },

  { T_OPT_CP,	"POS_MODE",	"pos-mode|posmode",
		"list",
		"Define a comma separated list of modes for the next {--pos-file} options."
		" Each element of the list begins with a keyword followed by"
		" a column index and/or a parameter list:"
		" @X<col>, Y<col>, Z<col>, D<col>, C<num>, T<num>,"
		" FA<col>=<str>, FB<col>=<str>, FC<col>=<str>, <COLOR>@"
		"\1\n  See https://szs.wiimm.de/opt/pos-mode for details." },

  { T_OPT_CP,	"POS_FILE",	"pos-file|posfile",
		"file",
		"Load a position file created by mkw-ana and"
		" print the positions into the map."
		" Define the modes first,"
		" because the last defined modes set by {--pos-mode} are used!" },

  { T_OPT_CP,	"PNG",		"png",
		"pix_size ['a' align] ['x' x1[:x2]] ['y' y1[:y2]] ['t' types]",
		"Draw a PNG with a top view of the track instead of an OBJ file."
		" Transformations are done after the KMP 3D drawing, but before PNG creation."
		"\n "
		" @'pix_size'@ is the number of world units (sqare) used for 1 PNG pixel."
		" @'align'@ is a factor as anti-aliasing pixel multiplicator (>1 is on)."
		" @'x'@ and @'y'@ defines the part of the world that is displayed."
		" If not set, the check points (KMP/CKPT) are used to find a good area."
		" @'t'@ is a bit field with 32 bits to enable the the KCL types."
		" Each bit corresponds to one of the KCL types and only ."
		" The default is to display all but not walls and solid fall ("
		CONVERT_TO_STRING(OPT_PNG_TYPE_CLASS) ")."
		"\n "
		" All numeric values can be any expression allowed by the parser." },

  { T_OPT_GP,	"LOAD_KCL",	"load-kcl|loadkcl",
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

  { T_OPT_GP,	"KMP",		"kmp",
		0, 0 /* copy of wszst */ },

 #if MAX_STGI_LAPS > 9
  { T_OPT_GP,	"N_LAPS",	"n-laps|nlaps",
		0, 0 /* copy of wszst */ },
 #else
  { H_OPT_GP,	"N_LAPS",	"n-laps|nlaps",
		0, 0 /* copy of wszst */ },
 #endif

  { T_OPT_GP,	"SPEED_MOD",	"speed-mod|speedmod",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"KTPT2",	"ktpt2",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"TFORM_KMP",	"tform-kmp|tformkmp|tkmp",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"REPAIR_XPF",	"repair-xpflags|repairxpflags|repxpf",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"GAMEMODES",	"gamemodes|gmodes",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"ROUND",	"round",
		0, 0 /* copy of wszst */ },

  { T_OPT_CM,	"LONG",		"l|long",
		0, 0 /* copy of wszst */ },

//  { T_OPT_CM,	"FULL",		"full",
//		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0,
		"Suppress information lines in decoded KMP text files."
		" This includes syntax information ({--no-header})."
		" If set twice, the output of unneeded sections is also suppressed." },

  { T_OPT_C,	"NO_WILDCARDS",	"no-wildcards|nowildcards",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"IN_ORDER",	"in-order|inorder",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"EXPORT",	"X|export",
		0,
		"Enable the export mode and create small and machine readable text files"
		" for easy post processing."
		" The option works similar like @-HBB@ for KMP text files." },

  { T_OPT_C,	"NO_PARAM",	"P|no-param|noparam",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"EPSILON",	"epsilon",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"DIFF",		"diff",
		"list",
		"List of KMP sections names that should be compared."
		" To select a section, use a combiantion of the following keywords:"
		" @AREA, CAME, CNPT, CKPH, CKPT, ENPH, ENPT, GOBJ, ITPH, ITPT,"
		" JGPT, KTPT, MSPT, POTI, STGI, NONE, GOOD, DEFAULT@ of @ALL@ (default)."
		" Additional keywords are @SORT@ (default) and @NOSORT@" },

  { T_OPT_C,	"GENERIC",	"G|generic",
		0,
		"Forget scanned entry names and use generic names for output." },

  { T_OPT_C,	"NO_ECHO",	"no-echo|noecho",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_CHECK",	"N|no-check|nocheck",
		0,
		"Do not make plausibility checks for KMP files." },

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
  //---------- wkmpt GROUP TRANSFORM ----------

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
  { T_COPT,	"KMP",		0,0,0 },
 #if MAX_STGI_LAPS > 9
  { T_COPT,	"N_LAPS",	0,0,0 },
 #else
  { H_COPT,	"N_LAPS",	0,0,0 },
 #endif
  { T_COPT,	"SPEED_MOD",	0,0,0 },
  { T_COPT,	"KTPT2",	0,0,0 },
  { T_COPT,	"TFORM_KMP",	0,0,0 },
  { T_COPT,	"REPAIR_XPF",	0,0,0 },
  { T_COPT,	"RM_GOBJ",	0,0,0 },
  { T_COPT,	"BATTLE",	0,0,0 },
  { T_COPT,	"EXPORT_FLAGS",	0,0,0 },
  { T_COPT,	"ROUTE_OPTIONS",0,0,0 },
  { T_COPT,	"WIM0",		0,0,0 },
  { T_COPT,	"SLOT",		0,0,0 },
  { T_COPT,	"LOAD_KCL",	0,0,0 },

  //---------- wkmpt GROUP DEST ----------

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

  //---------- wkmpt GROUP TEXTOUT ----------

  { T_GRP_BEG,	"TEXTOUT",		0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress the syntax information section in KMP text files." },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"EXPORT",	0,0,0 },
  { T_COPT,	"NO_PARAM",	0,0,0 },
  { T_COPT,	"GENERIC",	0,0,0 },
  { T_COPT,	"BATTLE",	0,0,0 },
  { T_COPT,	"EXPORT_FLAGS",	0,0,0 },
  { T_COPT,	"ROUTE_OPTIONS",0,0,0 },
  { T_COPT,	"WIM0",		0,0,0 },

  //
  //---------- COMMAND wkmpt VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wkmpt HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkmpt CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wkmpt ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkmpt EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkmpt TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },

  { T_COPT,	"VERBOSE",	0,0,
	"Print the matrices of transformations." },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wkmpt COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wkmpt ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wkmpt FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },

  //---------- COMMAND wkmpt FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wkmpt SYMBOLS ----------

  { T_CMD_BEG,	"SYMBOLS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wkmpt FUNCTIONS ----------

  { T_CMD_BEG,	"FUNCTIONS",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, FUNCTION_BRIEF },
  { T_COPT,	"LONG",		0,0, FUNCTION_LONG },

  //---------- COMMAND wkmpt CALCULATE ----------

  { T_CMD_BEG,	"CALCULATE",	0,0,0 },

  { T_COPT,	"CONST",	0,0,0 },

  //---------- COMMAND wkmpt MATRIX ----------

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

  //---------- COMMAND wkmpt FLOAT ----------

  { T_CMD_BEG,	"FLOAT",	0,0,0 },
  { T_COPT,	"ROUND",	0,0,0 },

  //---------- COMMAND wkmpt STARTPOS ----------

  { T_CMD_BEG,	"STARTPOS",	0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkmpt OBJECTS ----------

  { T_CMD_BEG,	"OBJECTS",	0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Don't print the file list and the info line(s)."
	" If set twice, only ID and object name is printed"
	" and option {--long} is ignored." },
  { T_COPT,	"LONG",		0,0,
	"Print each setting starting on a new line."
	" --long and {--brief} can be used together." },

  //---------- COMMAND wkmpt EXPORT ----------

  { T_CMD_BEG,	"EXPORT",	0,0,0 },

  //---------- COMMAND wkmpt EXPORT ----------

  { T_CMD_BEG,	"XEXPORT",	0,0,0 },

  //---------- COMMAND wkmpt CAT ----------

  { T_CMD_BEG,	"CAT",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkmpt DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPY_GRP,	"TEXTOUT",	0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkmpt ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"NO_ECHO",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wkmpt DIFF ----------

  { T_CMD_BEG,	"DIFF",		0,0,0 },

  { T_COPT,	"QUIET",	0,0,
	"If set once, only a summary status is printed."
	" If set twice, the status is only reported as exit status." },
  { T_COPT,	"EPSILON",	0,0,0 },
  { T_COPT,	"DIFF",		0,0,0 },
  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"ESC",		0,0,0 },

  //---------- COMMAND wkmpt DRAW ----------

  { T_CMD_BEG,	"DRAW",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"DRAW",		0,0,0 },
  { T_COPT,	"POS_MODE",	0,0,0 },
  { T_COPT,	"POS_FILE",	0,0,0 },
  { T_COPT,	"PNG",		0,0,0 },
  { T_COPY_CMD,	"DECODE",	0,0,0 },
  { T_COPT,	"KCL_SCRIPT",	0,0,0 },

  //---------- COMMAND wkmpt CHECK ----------

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
  { T_COPT,	"GENERIC",	0,0,0 },
  { T_COPT,	"NO_CHECK",	0,0,0 },

  //---------- COMMAND wkmpt STGI ----------

  { T_CMD_BEG,	"STGI",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,
	"If set, ignore non existing source files"
	" and files without KMP without warning."
	" If set twice, ignore files with standard settings too." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wszst KTPT ----------

  { T_CMD_BEG,	"KTPT",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,
	"If set, ignore non existing source files"
	" and files without KMP without warning." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"KTPT2",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"If set, compare each KTPT with each lap counter (some CKPT) and print"
	" positions, directions, distances and recommendations as wide table." },

  //---------- COMMAND wkmpt ROUTES ----------

  { T_CMD_BEG,	"ROUTES",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,
	"If set, ignore non existing source files"
	" and files without KMP without warning." },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wkmpt GOBJ ----------

  { T_CMD_BEG,	"GOBJ",		0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress head and foot lines." },
  { T_COPT,	"BRIEF",	0,0,
	"Print only summary for each object category."
	" If set twice, print a status vector only." },
  { T_COPT,	"LONG",		0,0,
	"?" },

  //---------- COMMAND wkmpt GAMEMODES ----------

  { T_CMD_BEG,	"GAMEMODES",	0,0,0 },

  { T_COPT,	"NO_WILDCARDS",	0,0,0 },
  { T_COPT,	"IN_ORDER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"GAMEMODES",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,
	"Suppress head and foot lines." },
  { T_COPT,	"BRIEF",	0,0,
	"Print only the scenario version and configuration name."
	" If set twice, suppress ead and foot lines too." },
  { T_COPT,	"LONG",		0,0,
	"If set, print a status vector for each GOBJ element too:"
	" '-': Object diabled;"
	" '=': Object diabled by presence flags, but visible before race;"
	" '+': Object enabled;"
	" 'e': Depends on engine;"
	" 'r': Depends on random scenario;"
	" 'b': Defintion object of type BITS;"
	" 'o': Defintion object of type OR;"
	" 'a': Defintion object of type AND." },

  //---------- COMMAND wkmpt WIM0 ----------

  { T_CMD_BEG,	"WIM0",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"If set, disable table format and append a full hexdump for each WIM0 section." },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

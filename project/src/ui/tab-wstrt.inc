
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
 *   Copyright (c) 2011-2020 by Dirk Clemens <wiimm@wiimm.de>              *
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
/////////////			    wstrt			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wstrt", 0,
		"wstrt [option]... command [option|parameter|@file]...",
		WSTRT_LONG " :"
		" Manipulate the 'main.dol' and 'StaticR.rel' files"
		" of Mario Kart Wii." },

  //
  //---------- list of all wstrt commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION",
		    "wstrt VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wstrt HELP [--width=wd] OPT|OPTION ...\n"
		    "wstrt HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wstrt HELP [--width=wd] command [-b] [ignored]...\n"
		    "wstrt HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wstrt ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wstrt TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wstrt COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wstrt ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wstrt FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wstrt FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { H_DEF_CMD,	"RAWDUMP",	"_RAWDUMP",
		    "wszst _RAWDUMP [-d|-D] destdir",
		"This hidden command creates the destination directory "
		" and create several files as copy of internal data." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"DUMP",		"DUMP",
		    "wstrt DUMP file...",
		"Dump the structure of 'StaticR.rel' (in development)"
		" and of DOL files as text." },

  { H_DEF_CMD,	"CDUMP",	"_CDUMP",
		    "wstrt _CDUMP file...",
		"Dump the structure of 'StaticR.rel' (in development)"
		" and of DOL files as C source fragments." },

  { T_DEF_CMD,	"HEXDUMP",	"HEXDUMP|XDUMP",
		    "wstrt HEXDUMP file...",
		"Dump the content of DOL files as hex dump and use virtual addresses."
		" Select parts by --vaddr, --faddr and --sname."
		" Without selectors, the whole file is dumped." },

  { H_DEF_CMD,	"HTTPS",	"_HTTPS",
		    "wstrt _HTTPS file...",
		"Collect data about https-requests and print a total summary"
		" for all accepted files." },

  { T_DEF_CMD,	"EXTRACT",	"EXTRACT|X",
		    "wstrt EXTRACT dol...",
		"Extract the sections of a DOL file into a directory."
		" StaticR.rel files are ignored." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"TRACKS",	"TRACKS",
		    "wstrt TRACKS [file]...",
		"Show track infos in the order of the 'StaticR.rel' files."
		" If no file is entered, the standard order is used." },

  { T_DEF_CMD,	"ARENAS",	"ARENAS",
		    "wstrt ARENAS [file]...",
		"Show arena infos in the order of the 'StaticR.rel' files."
		" If no file is entered, the standard order is used." },

  { T_DEF_CMD,	"FILES",	"FILES",
		    "wstrt FILES [file]...",
		"List track and arena file names in machine readable format."
		" Use the order of the entered 'StaticR.rel' files."
		" If no file is entered, the standard order is used." },

  { T_DEF_CMD,	"FIND",	"FIND",
		    "wstrt FIND [+]TRACK keyword...\n"
		    "wstrt FIND [+]ARENA keyword...",
		"Find a track or arena by keyword and print one line for each keyword"
		" in machine readable format @'type slot'@"
		" or, if not found, a simple @'-'@."
		" The name is searched in the entered context (tracks or arenas)."
		" If @TRACK@ or @ARENA@ is prefixed by a plus, then a failed search"
		" will be continued in the other context."
		" If at least one name is not found,"
		" the tool terminates with exit code @NOT_EXISTS@." },

  { T_DEF_CMD,	"POINTS",	"POINTS|PTS",
		    "wstrt POINTS [--points=list] [file]...",
		"Print a table with the points assigned to player after"
		" a versus or room race for each entered file."
		" If no file is entered, the standard table is used."
		" However, if option {--points} is set only the resulting table"
		" is printed." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"ANALYZE",	"ANALYZE|ANA|ANALYSE",
		    "wstrt ANALYZE [file]...",
		"Open 'StaticR.rel' and DOL files and analyze them."
		" The files will be classified using SHA1 checksums." },

  { T_DEF_CMD,	"PATCH",	"PATCH",
		    "wstrt PATCH [file]...",
		"Patch 'main.dol and 'StaticR.rel' files of Mario Kart Wii."
		" No backup is made." },

  //
  //---------- list of all wstrt options ----------

  { T_OPT_S,	"VERSION",	"V|version",
		0, 0 /* copy of wszst */ },

  { T_OPT_S,	"HELP",		"h|help",
		0, 0 /* copy of wszst */ },

  { T_OPT_S,	"XHELP",	"xhelp",
		0, 0 /* copy of wszst */ },

  { H_OPT_G,	"AT_DUMMY",	"@|@", 0, "" },

  { T_OPT_G,	"ALLOW_ALL",	"allow-all|allowall",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"COMPATIBLE",	"compatible",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP|F_IGNORE,
		"WIDTH",	"width",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"QUIET",	"q|quiet",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"VERBOSE",	"v|verbose",
		0, 0 /* copy of wszst */ },

  { T_OPT_GM,	"LOGGING",	"L|logging",
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
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0,
		"List only the track file names and suppress table header and footer." },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"TRACKS",	"T|tracks",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"ARENAS",	"A|arenas",
		0, 0 /* copy of wszst */ },

  { T_OPT_GP,	"REGION",	"R|region",
		"region",
		"If 'region' is an unsigned number, the region"
		" for online versus races and for battles is patched"
		" to the entered number."
		" If the number is prefixed by the letter @'t'@, a test mode"
		" is enabled and the places are patched with ascending numbers."
		" You can patch StaticR.rel and main.dol."
		"\n "
		" Otherwise a keyword is expected."
		" Official regions are: @JAPAN(0), AMERICA(1), EUROPE(2),"
		" AUSTRALIA(OCEANIA,3), KOREA(4), TAIWAN(5)@ and @CHINA(6)@."
		" @RESTORE@ restores the original Nintendo values"
		" and @NONE@ disables region patching (default)."
		"\n "
		" See https://wiimmfi.de/reg-list for other regions." },

  { T_OPT_GP,	"VS_REGION",	"vs-region|vsregion|vregion",
		"region",
		"Same as {--region}, but only for online versus races." },

  { T_OPT_GP,	"BT_REGION",	"bt-region|btregion|bregion",
		"region",
		"Same as {--region}, but only for online battles." },
	//	" If the region is prefixed by the letter @'x'@,"
	//	" than a fourth experimental place is patched too." },

  { T_OPT_GO,	"ALL_RANKS",	"all-ranks|allranks",
		"[=mode]",
		"If set to @ON@ (default without parameter),"
		" the server query to Nintendos servers is manipulated,"
		" so that the player will find all players in online races"
		" independent of their ranking points."
		" Value @RESTORE@ disables this feature and restores Nintendos original query."
		" Value @NONE@ disables this kind of patching (default)." },

  { T_OPT_GP,	"POINTS",	"points|pts",
		"list",
		"This patch option " OPT_POINTS },

  { H_OPT_CP,	"CHEAT",	"cheat",
		"region",
		"Change the output format and create a cheat code for the region."
		" Allowed keywords: JAPAN; AMERICA, NTSC; EUROPE, AUSTRALIA, PAL; KOREA." },

  { T_OPT_GP,	"HTTPS",	"https",
		"mode",
		"Patch the https strings of REL and DOL files."
		" 'mode' is one of the following keywords:"
		"\n"
		"@RESTORE@ restores the original URLs and domains."
		"\n"
		"@HTTP@ replaces HTTPS requests by HTTP and sub-domain"
		" '@naswii.@' by '@nas.@'."
		"\n"
		"@DOMAIN@ replace HTTPS requests by HTTP"
		" and the domain '@nintendowifi.net@'"
		" by a domain specified by option {--domain}."
		"\n"
		"@SAKE0@ same as DOMAIN, but don't replace URLs with SAKE domain."
		"\n"
		"@SAKE1@ same as SAKE0, but replace HTTPS of SAKE domains." },

  { T_OPT_GP,	"DOMAIN",	"domain",
		"dom",
		"Define a domain for option {--https}."
		" The default is '@wiimmfi.de@'." },

  { T_OPT_G,	"WIIMMFI",	"wiimmfi",
		0,
		"Patch the files for the new custom server."
		" It is a short cut for {--https domain --domain wiimmfi.de}." },

  { H_OPT_G,	"TWIIMMFI",	"twiimmfi",
		0,
		"Patch the files for the test server."
		" It is a short cut for {--https domain --domain test.wiimmfi.de}." },

  { T_OPT_G,	"WC24",		"wc24",
		0,
		"Exclude 2 strings of file StaticR.rel from --wiimmfi"
		" and --domain patch to enable WC24 support." },

  { H_OPT_GO,	"WCODE",	"wcode",
		"[=mode]",
		"Enable (mode=ON) or disable (mode=OFF) wiimmfi-code patching."
		" Default mode is AUTO."
		" In this case, patching is enabled if any WCODE is defined"
		" or option --domain is set either to 'wiimmfi.de'"
		" or to 'test.wiimmfi.de'." },

  { H_OPT_CP,	"ADD_WCODE",	"@add-wcode|@addwcode",
		"file",
		"Each option call adds the filename to an internal list."
		" Each file must be of type GCT and is added to the internal wcode stock."
		"\n "
		" If a single file is not found and the filename (not path)"
		" contains at least 1 '@@' character, then all '@@' characters of"
		" it are replaced by the region code (one of P,E,J,K) for a second try." },

  { T_OPT_GP,	"PB_MODE",	"pb-mode|pbmode",
		"mode",
		"Mode for the patched-by string:"
		" @OFF@ disables modifications."
		" @RESET@ resets the string to stars."
		" @WIIMMFI@ defines it only, if patched for Wiimmfi."
		" @AUTO@ (default) defines it only, if patched."
		" @ALWAYS@ defines it always." },

  { H_OPT_GP,	"PATCHED_BY",	"@patched-by|@patchedby",
		"string",
		"Define a string for @patched-by@."
		" Default is: @wstrt vVERSION rREVISION@" },

  { T_OPT_GP,	"VS",		"vs",
		"char",
		"Reset (str is empty) or replace the second character of 'vs'"
		" to create an own online world for versus races."
		" The file StaticR.rel will be patched at 4 places." },

  { H_OPT_GP,	"VS2",		"vs2",
		"str",
		"Reset (str is empty) or replace the 2 characters of 'vs'"
		" to create an own online world for versus races."
		" If the parameter contains only 1 character,"
		" use 'v' as first character."
		" The file StaticR.rel will be patched at 4 places."
		"\n "
		" Wiimmfi will only detect 'Versus' if the first letter is 'v' or 'V'." },

  { T_OPT_GP,	"BT",		"bt",
		"char",
		"Reset (str is empty) or replace the second character of 'bt'"
		" to create an own online world for battles."
		" The file StaticR.rel will be patched at 4 places." },

  { H_OPT_GP,	"BT2",		"bt2",
		"str",
		"Reset (str is empty) or replace the 2 characters of 'bt'"
		" to create an own online world for battles."
		" If the parameter contains only 1 character,"
		" use 'b' as first character."
		" The file StaticR.rel will be patched at 4 places."
		"\n "
		" Wiimmfi will only detect 'Battle' if the first letter is 'b' or 'B'." },

  { T_OPT_GP,	"CANNON",	"cannon",
		"idx,p1,p2,p3,p4",
		"Patch parameters for cannon with index IDX (0..2)."
		" Parameters P1..P4 are floating point numbers."
		" Repeat this option for each cannon, that should be modified." },

  { T_OPT_G,	"MENO",		"meno",
		0,
		"Patch some 'MenuSingle' and 'MenuMulti' strings of file StaticR.rel"
		" for online usage to 'MenO*' (MenuOnline)"
		" to distinguish between offline and online races." },

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

  { T_OPT_G,	"CLEAN_DOL",	"clean-dol|cleandol",
		0,
		"Remove additional DOL sections, reset entry point and VBI address,"
		" and try to restore other patches known by SZS tools."
		" This is done after loading a DOL and before analysing"
		" and patching to allow re-patching." },

  { T_OPT_C,	"ADD_LECODE",	"add-lecode|addlecode",
		0,
		"Add LE-CODE starter to main.dol." },

  { T_OPT_C,	"ADD_CTCODE",	"add-ctcode|addctcode",
		0,
		"If source is an original main.dol file of MKWii,"
		" then append 2 sections (usually T2 and D8) with @CT-CODE@."
		" The CTCODE is inserted after sections defined by {--add-section}"
		" and then only, if there are no address collisions."
		"\n "
		" If {--ct-dir} is used, then files named"
		" @rmc[ejp]/boot_code.bin@ and @boot_code.bin@ for T2,"
		" and @rmc[ejp]/boot_data.bin@ and @boot_data.bin@ for D8"
		" are searched." },

  { T_OPT_GP,	"CT_DIR",	"ct-dir|ctdir",
		"dir",
		OPT_CT_DIR("") },

  { T_OPT_C,	"MOVE_D8",	"move-d8|moved8",
		0,
		"Move the @CT-CODE@ section D8 to another place"
		" at the middle of binary."
		" It is done automatically, if the original place overlaps"
		" other existing sections." },

  { T_OPT_CP,	"ADD_SECTION",	"add-section|addsection",
		"file",
		"Each option call adds the filename to an internal list."
		" Each file must be of type GCT, GCH or WCH."
		" All GCT files together are combined with an internal code handler"
		" to one section (like a GCH file)."
		" GCH and WCH define sections and patches. The defined sections"
		" are added and VBI hooks are optionally initialized."
		" Use option {--force} to allow overlapped sections."
		" Use option {--verbose} up to 2 times for a log."
		"\n "
		" If a single file is not found and the filename (not path)"
		" contains at least 1 '@@' character, then all '@@' characters of"
		" it are replaced by the region code (one of P,E,J,K) for a second try."
		"\n "
		" The intention of this option is to add a Gecko Code Handler"
		" and cheat codes to a main.dol for automatic execution."
		"\1 See https://szs.wiimm.de/doc/addsect for details." },

  { H_OPT_C,	"FULL_GCH",	"full-gch|fullgch",
		0,
		"Use the full 'codehandler' instead of 'codehandleronly'." },

  { H_OPT_C,	"GCT_NO_SEP",	"gct-no-sep|gctnosep",
		0,
		"Ignore cheat code separators completly"
		" and don't filter @'F0000001 0000xxxx'@ codes."
		" Options --gct-sep and --gct-asm-sep are ignored too." },

  { H_OPT_C,	"GCT_SEP",	"gct-sep|gctsep",
		0,
		"Detect and remove cheat code separators"
		" of format @'F0000001 0000xxxx'@."
		" The separators are used to split the cheat codes"
		" into several blocks if the total size is too large."
		" ASM blocks are skipped if searching separators." },

  { H_OPT_C,	"GCT_ASM_SEP",	"gct-asm-sep|gctasmsep",
		0,
		"Detect the beginning of ASM blocks and use them"
		" as cheat code separators."
		" The separators are used to split the cheat codes"
		" into several blocks if the total size is too large." },

  { H_OPT_C,	"GCT_LIST",	"gct-list|gctlist",
		0,
		"If GCT separators active, print a summary table about"
		" the distribution of the segments."
		" Be more verbose if set twice." },

  { T_OPT_CO,	"GCT_MOVE",	"gct-move|gctmove",
		"[=mode]",
		"If a section is of type gecko code handler + cheat code, then make"
		" the heap smaller and move the cheat codes before the FST section."
		"\n "
		" Parameter MODE is optional."
		" If set, OFF disables and ON enables this feature."
		" For mode AUTO (default) this feature is enabled,"
		" if the cheat codes are too large or for internal reasons." },

  { T_OPT_CP,	"GCT_ADDR",	"gct-addr|gctaddr",
		"address",
		"If {--gct-move} is active, a text section is created"
		" to manage the code handling. This section is only needed at start"
		" and is placed by default at address 0c802c0000"
		" (good address for all 4 regions) as part of BSS."
		" This option changes the start address of the section."
		" If address is @0@ or invalid, then unused BSS space is searched." },

  { T_OPT_CP,	"GCT_SPACE",	"gct-space|gctspace",
		"size",
		"Define the minimal space for the code part of {--gct-move}."
		" This value is ignored,"
		" if it is smaller than the size of the cheat codes."
		" This option was only implemented to test the "
		" memory allocation by {--gct-move} with large blocks." },

  { T_OPT_CO,	"ALLOW_USER_GCH", "allow-user-gch|allowusergch",
		"[=mode]",
		" This option is only relevant, if main.dol is patched for wiimmfi.de."
		" It is dedicated to custom distribution creators for test purposes."
		" If enabled, a user can load an own instance of Gecko Code Handler"
		" and cheat codes and can use them online."
		" If disabled, pachcing for online modus is disabled if an"
		" alternative Gecko Code Handler is detected."
		"\n "
		" Parameter MODE is optional."
		" If set, OFF disables and ON enables this feature."
		" For mode AUTO (default) it depends on other options." },

  { H_OPT_CP,	"CREATE_SECT",	"create-sect|createsect|cs",
		"mode=file",
		"Create a new DOL section by loading the binary file."
		" MODE is a list of 'P' (parameter support),"
		" 'T' or 'D' with optional digit (define section)"
		" and an address (auto if not set)."
		" The parts are separated by spaces or commas."
		"\n "
		" If MODE is 'P', then 4 32-bit parameters are expected at offset 0x10:"
		" The section address at 0x10 (read),"
		" the relative entry point at 0x14 (read, used if >0),"
		" the original DOL entry point at 0x18 (written),"
		" the address of the last defined '.dol' section at 0x1c (written)."
		"\n "
		" If the file name is '.dol', then it points not to a file."
		" Instead a copy of the DOL header beginning with the section"
		" addresses and ending behind the entry point (156B total) is copied."
		" MODE 'P' is ignored for this section."
		" All other MODE 'P' sections are updated at offset 0x1c."
		"\n "
		" --cs is a short cut." },

  { H_OPT_CP,	"WPF",		"wpf",
		"file",
		"Each option call adds the filename to an internal list."
		" Each file must be of type WPF (Wiimms Patch File) and defines"
		" a list of conditionals and patches for staticr.rel and main.dol." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"SECTIONS",	"sections",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"VADDR",	"vaddr|va",
		"RANGE",
		"Comma separated list of virtual addresses to select the hex-dump ranges."
		" Each element is either @ADDRESS@ (1 byte)"
		" or @START:END@ or @START#SIZE@." },

  { T_OPT_CP,	"FADDR",	"faddr|fa",
		"RANGE",
		"Comma separated list of file addresses to select the hex-dump ranges."
		" Each element is either @ADDRESS@ (1 byte)"
		" or @START:END@ or @START#SIZE@." },

  { T_OPT_CP,	"SNAME",	"sname|sn",
		"LIST",
		"Comma separated list of section names to select the hex-dump ranges." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"INT1",		"i1",
		0,
		"Format: Print single bytes as integer number (default)." },

  { T_OPT_C,	"INT2",		"i2",
		0,
		"Format: Print each 2 bytes as integer number." },

  { T_OPT_C,	"INT3",		"i3",
		0,
		"Format: Print each 3 bytes as integer number." },

  { T_OPT_C,	"INT4",		"i4",
		0,
		"Format: Print each 4 bytes as integer number." },

  { T_OPT_C,	"INT5",		"i5",
		0,
		"Format: Print each 5 bytes as integer number." },

  { T_OPT_C,	"INT6",		"i6",
		0,
		"Format: Print each 6 bytes as integer number." },

  { T_OPT_C,	"INT7",		"i7",
		0,
		"Format: Print each 7 bytes as integer number." },

  { T_OPT_C,	"INT8",		"i8",
		0,
		"Format: Print each 8 bytes as integer number." },

  { T_OPT_C,	"FLOAT",	"float",
		0,
		"Format: Print each 4 bytes as floating point number." },

  { T_OPT_C,	"DOUBLE",	"double",
		0,
		"Format: Print each 8 bytes as double floating point number." },

  { T_OPT_C,	"LE",		"le",
		0,
		"Use little endian for binary data." },

  { T_OPT_C,	"BE",		"be",
		0,
		"Use big endian for binary data (default)." },

  { T_OPT_C,	"ZEROS",	"zeros",
		0,
		"Print integer numbers with leading zeros." },

  { T_OPT_C,	"HEX",		"hex",
		0,
		"Print integer numbers as hexadecimal numbers (default)." },

  { T_OPT_C,	"DEC",		"dec",
		0,
		"Print integer numbers as decimal numbers." },

  { T_OPT_C,	"C_SYNTAX",	"c-syntax|csyntax",
		0,
		"Print output in `C' syntax." },

  { T_OPT_CP,	"ADDR",		"address",
		"addr",
		"Use @ADDR@ instead of null as start address for the dump." },

  { T_OPT_C,	"ALIGN",	"align",
		0,
		"Try to align the dump, so that the addresses (printed or not)"
		" are multiple of bytes-per-line." },

  { T_OPT_CP,	"TRIGGER",	"trigger",
		"num",
		"If the number @NUM@ is found and printed,"
		" close the current line and start with a new one."
		" If {--align} is set too,"
		" continue the next line with the current column."
		" The trigger works only for integer formats, but not for floats." },

  { T_OPT_C,	"NO_ADDR",	"no-address|noaddress",
		0,
		"Suppress the address column of the dump." },

  { T_OPT_C,	"NO_NUMBERS",	"no-numbers|nonumbers",
		0,
		"Suppress the numbers column of the dump." },

  { T_OPT_C,	"NO_TEXT",	"no-text|notext",
		0,
		"Suppress the text column of the dump." },

  { T_OPT_C,	"FORMAT",	"format",
		0,
		"Add @'#FORMAT:'@ info lines." },

  { T_OPT_C,	"NO_NULL",	"no-null|nonull",
		0,
		"Ignore data lines with NULL bytes only and"
		" print a summary for each NULL block. (@'#NULL: ...'@)" },

  { T_OPT_C,	"NARROW",	"narrow",
		0,
		"Suppress the spaces that are usually inserted for readability." },

  { T_OPT_C,	"SMALL",	"small",
		0,
		"Each call will halve the number of bytes per line."
		" Usually, 16 bytes per lines are reduced to 8, then to 4, 2 and 1."
		" The effective number of bytes per line is format dependent." },

  { T_OPT_C,	"WIDE",		"wide",
		0,
		"Each call will round down the current bytes per line value"
		" to the next multiple of 16 and then increment it by 16."
		" Usually, 16 bytes per lines are enhanced to 32,"
		" then to 48, 64, 80 and so on."
		" The effective number of bytes per line is format dependent." },

  { T_OPT_CP,	"BYTES",	"bytes",
		"num",
		"Print up to @NUM@ bytes per line instead of 16."
		" The maximum allowed value is "
		CONVERT_TO_STRING(XDUMP_MAX_BYTES_PER_LINE) "."
		" The effective number of bytes per line is format dependent"
		" (multiple of format size)." },


  //
  //---------- wstrt GROUP DEST ----------

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

  //---------- wstrt GROUP PATCH ----------

  { T_GRP_BEG,	"PATCH",	0,0,0 },

  { T_COPT,	"TRACKS",	0,0,0 },
  { T_COPT,	"ARENAS",	0,0,0 },
  { T_COPT,	"REGION",	0,0,0 },
  { T_COPT,	"VS_REGION",	0,0,0 },
  { T_COPT,	"BT_REGION",	0,0,0 },
  { T_COPT,	"ALL_RANKS",	0,0,0 },
  { T_COPT,	"HTTPS",	0,0,0 },
  { T_COPT,	"DOMAIN",	0,0,0 },
  { T_COPT,	"WIIMMFI",	0,0,0 },
  { H_COPT,	"TWIIMMFI",	0,0,0 },
  { T_COPT,	"PB_MODE",	0,0,0 },
  { T_COPT,	"WC24",		0,0,0 },
  { H_COPT,	"WCODE",	0,0,0 },
  { T_COPT,	"VS",		0,0,0 },
  { H_COPT,	"VS2",		0,0,0 },
  { T_COPT,	"BT",		0,0,0 },
  { H_COPT,	"BT2",		0,0,0 },
  { T_COPT,	"CANNON",	0,0,0 },
  { T_COPT,	"MENO",		0,0,0 },

  //---------- wstrt GROUP HEXDUMP ----------

  { T_GRP_BEG,	"HEXDUMP",	0,0,0 },
  { T_COPT,	"INT1",		0,0,0 },
  { T_COPT,	"INT2",		0,0,0 },
  { T_COPT,	"INT3",		0,0,0 },
  { T_COPT,	"INT4",		0,0,0 },
  { T_COPT,	"INT5",		0,0,0 },
  { T_COPT,	"INT6",		0,0,0 },
  { T_COPT,	"INT7",		0,0,0 },
  { T_COPT,	"INT8",		0,0,0 },
  { T_COPT,	"FLOAT",	0,0,0 },
  { T_COPT,	"DOUBLE",	0,0,0 },
  { T_COPT,	"LE",		0,0,0 },
  { T_COPT,	"BE",		0,0,0 },
  { T_COPT,	"ZEROS",	0,0,0 },
  { T_COPT,	"HEX",		0,0,0 },
  { T_COPT,	"DEC",		0,0,0 },
  { T_COPT,	"C_SYNTAX",	0,0,0 },
  { T_COPT,	"ADDR",		0,0,0 },
  { T_COPT,	"ALIGN",	0,0,0 },
  { T_COPT,	"TRIGGER",	0,0,0 },
  { T_COPT,	"NO_ADDR",	0,0,0 },
  { T_COPT,	"NO_NUMBERS",	0,0,0 },
  { T_COPT,	"NO_TEXT",	0,0,0 },
  { T_COPT,	"FORMAT",	0,0,0 },
  { T_COPT,	"NO_NULL",	0,0,0 },
  { T_COPT,	"NARROW",	0,0,0 },
  { T_COPT,	"SMALL",	0,0,0 },
  { T_COPT,	"WIDE",		0,0,0 },
  { T_COPT,	"BYTES",	0,0,0 },

  //
  //---------- COMMAND wstrt VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wstrt HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wstrt ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },

  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wstrt TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },

  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wstrt COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wstrt ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wstrt FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wstrt FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wstrt _RAWDUMP ----------

  { T_CMD_BEG,	"RAWDUMP",	0,0,0 },

  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },

  //---------- COMMAND wstrt DUMP ----------

  { T_CMD_BEG,	"DUMP",		0,0,0 },
  { T_COPT,	"LONG",		0,0,
		"For DOL files:"
		" If set, a second table with the image map is printed."
		" If set twice, a third table with delta values is printed too." },

  //---------- COMMAND wstrt HEXDUMP ----------

  { T_CMD_BEG,	"HEXDUMP",	0,0,0 },
  { T_COPT,	"VADDR",	0,0,0 },
  { T_COPT,	"FADDR",	0,0,0 },
  { T_COPT,	"SNAME",	0,0,0 },
  { T_COPT,	"LOGGING",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPY_GRP,	"HEXDUMP",	0,0,0 },

  //---------- COMMAND wstrt CDUMP ----------

  { T_CMD_BEG,	"CDUMP",	0,0,0 },

  //---------- COMMAND wstrt HTTPS ----------

  { T_CMD_BEG,	"HTTPS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
		" Print output a C source" },
  { T_COPT,	"VERBOSE",	0,0,
		" Print hex dump if an entry differs." },

  //---------- COMMAND wstrt EXTRACT ----------

  { T_CMD_BEG,	"EXTRACT",	0,0,0 },

  { T_COPT,	"DEST",		0,0,0 },
  { T_COPT,	"DEST2",	0,0,0 },
  { T_COPT,	"OVERWRITE",	0,0,0 },

  //---------- COMMAND wstrt TRACKS ----------

  { T_CMD_BEG,	"TRACKS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
		"Add columns for music slot and short name." },
  { T_COPT,	"DE",		0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wstrt ARENAS ----------

  { T_CMD_BEG,	"ARENAS",	0,0,0 },
  { T_COPY_CMD,	"TRACKS",	0,0,0 },

  //---------- COMMAND wstrt FILES ----------

  { T_CMD_BEG,	"FILES",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },

  //---------- COMMAND wstrt FIND ----------

  { T_CMD_BEG,	"FIND",		0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Add a user friendly track or arena name at the end of each line."
	" If set twice, add also the file names of the track and sound files"
	" before the track name."
	" If set three times, add the slot index and the internal index"
	" (both zero based) before the file names." },
  { T_COPT,	"DE",		0,0,0 },

  //---------- COMMAND wstrt POINTS ----------

  { T_CMD_BEG,	"POINTS",	0,0,0 },
  { T_COPT,	"POINTS",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0,
	"Print the table as comma separated lists in C notation." },
  { T_COPT,	"LONG",		0,0,
	"Append a string in {--points} format that defines the table."
	" If set twice, append up to 3 strings in different modes." },
  { H_COPT,	"CHEAT",	0,0,0 },

  //---------- COMMAND wstrt ANALYZE ----------

  { T_CMD_BEG,	"ANALYZE",	0,0,0 },
  { T_COPT,	"CLEAN_DOL",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print different lists if bugs found."
	" If set twice, print the lists always." },

  //---------- COMMAND wstrt PATCH ----------

  { T_CMD_BEG,	"PATCH",	0,0,0 },

  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"CLEAN_DOL",	0,0,0 },
  { T_COPY_GRP,	"PATCH",	0,0,0 },
  { H_COPT,	"ADD_LECODE",	0,0,0 },
  { T_COPT,	"ADD_CTCODE",	0,0,0 },
  { T_COPT,	"CT_DIR",	0,0,0 },
  { T_COPT,	"MOVE_D8",	0,0,0 },
  { T_COPT,	"ADD_SECTION",	0,0,0 },
  { H_COPT,	"ADD_WCODE",	0,0,0 },
  { H_COPT,	"FULL_GCH",	0,0,0 },
  { H_COPT,	"GCT_NO_SEP",	0,0,0 },
  { H_COPT,	"GCT_SEP",	0,0,0 },
  { H_COPT,	"GCT_ASM_SEP",	0,0,0 },
  { H_COPT,	"GCT_LIST",	0,0,0 },
  { T_COPT,	"GCT_MOVE",	0,0,0 },
  { T_COPT,	"GCT_ADDR",	0,0,0 },
  { T_COPT,	"GCT_SPACE",	0,0,0 },
  { T_COPT,	"ALLOW_USER_GCH",0,0,0 },
  { T_COPT,	"FORCE",	0,0,
	"Accept overlapped sections.\n>>> THE RESULT IS MOST LIKELY INVALID! <<<" },
  { H_COPT,	"CREATE_SECT",	0,0,0 },
  { H_COPT,	"WPF",		0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

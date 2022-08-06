
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
/////////////			    wimgt			/////////////
/////////////////////////////////////////////////////////////////////////////

  { T_DEF_TOOL,	"wimgt", 0,
		"wimgt [option]... command [option|parameter|@file]...",
		WIMGT_LONG " :"
		" Extract and convert graphic images."
		" The file formats TPL, TEX, BTI, BREFT and PNG are supported." },

  //
  //---------- list of all wimgt commands ----------

  { T_DEF_CMD,	"VERSION",	"VERSION",
		    "wimgt VERSION [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"HELP",		"HELP|H|?",
		    "wimgt HELP [--width=wd] OPT|OPTION ...\n"
		    "wimgt HELP [--width=wd] CMD|COMMAND [ignored]...\n"
		    "wimgt HELP [--width=wd] command [-b] [ignored]...\n"
		    "wimgt HELP [--width=wd] [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"CONFIG",	"CONFIG",
		    "wimgt CONFIG [options]...",
		CONFIG_CMD },

  { T_DEF_CMD,	"ARGTEST",	"ARGTEST",
		    "wimgt ARGTEST ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"EXPAND",	"EXPAND",
		    "wimgt EXPAND ...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"TEST",		"TEST",
		    "wimgt TEST [options]... [ignored]...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"COLORS",	"COLORS",
		    "wimgt COLORS",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"ERROR",	"ERROR|ERR",
		    "wimgt ERROR [error_code]",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILETYPE",	"FILETYPE|FT",
		    "wimgt FILETYPE file...",
		0 /* copy of wszst */ },

  { T_DEF_CMD,	"FILEATTRIB",	"FILEATTRIB|FA",
		    "wimgt FILEATTRIB [keyword]...",
		0 /* copy of wszst */ },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"LIST",		"LIST|LS",
		    "wimgt LIST [source]...",
		"List images files and print file and image type." },

  { T_DEF_CMD,	"LIST_L",	"LIST-L|LL|LISTL",
		    "wit LIST-L [source]...",
		"List images files and print file and image type"
		" and geometry data."
		" 'LIST-L' is a short cut for {LIST --long}." },

  { T_DEF_CMD,	"LIST_LL",	"LIST-LL|LLL|LISTLL",
		    "wit LIST-LL [source]...",
		"List images files and print file and image type"
		" and advanced geometry data."
		" 'LIST-LL' is a short cut for {LIST --long --long}." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  { T_DEF_CMD,	"DECODE",	"DECODE|DEC",
		    "wimgt DECODE [source]...",
		"Decode (export) the entered image files into PNG files."
		" The default destination is @'%P/%F.png'@." },

  { T_DEF_CMD,	"ENCODE",	"ENCODE|ENC",
		    "wimgt ENCODE [source]...",
		"Encode (import) the entered PNG files and convert them to"
		" to an internal image format (TPL, TEX, BREFT)."
		" The default destination is @'%P/%N'@ (removed extension)." },

  { T_DEF_CMD,	"CONVERT",	"CONVERT|CV",
		    "wimgt CONVERT [source]...",
		"Convert the entered image files into new file and image formats."
		" The old file is replaced by the new one."
		" The difference to {ENCODE} is only the default file name handling." },

  { T_DEF_CMD,	"COPY",		"COPY|CP",
		    "wimgt COPY source dest\n"
		    "wimgt COPY source... [-d|-D] dest",
		"Copy and convert an image from 'source' to 'dest'."
		" Multiple sources can be used, if 'dest' is a directory"
		" or contains at least one %-pattern."
		"\n "
		" This is an alternative for the ENCODE and DECODE commands."
		" The default destination file format is selected by scanning the"
		" file extension of the destination."
		" Option --transform may override this."
		" Option --overwrite is set implicitly." },

  { T_SEP_CMD,	0,0,0,0 }, //----- separator -----

  //
  //---------- list of all wimgt options ----------

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
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_HEADER",	"H|no-header|noheader",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"BRIEF",	"B|brief",
		0,
		"Create smaller output." },

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

  { T_OPT_C,	"ALL",		"a|all",
		0,
		"Convert all files and not only the files with changed file"
		" or image formats." },

  { T_OPT_C,	"MIPMAPS",	"mipmaps",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"NO_MIPMAPS",	"no-mipmaps|nomipmaps|no-mm|nomm",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"N_MIPMAPS",	"n-mipmaps|nmipmaps|n-mm|nmm",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"MAX_MIPMAPS",	"max-mipmaps|maxmipmaps|max-mm|maxmm",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"MIPMAP_SIZE",	"mipmap-size|mipmapsize|mm-size|mmsize",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"FAST_MIPMAPS",	"fast-mipmaps|fastmipmaps|fast-mm|fastmm",
		0, 0 /* copy of wszst */ },

  { T_OPT_CP,	"CMPR_DEFAULT",	"cmpr-default",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"PRE_CONVERT",	"pre-convert|preconvert",
		0,
		"Convert the image before the image format for the destination file"
		" is selected. This results in a perhaps worser graphic"
		" and is only useful to control graphic conversions." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CMP,	"PATCH",	"P|patch",
		"mode1.mode2...=file",
		"This option specifies patch mode list and a file,"
		" separated by an equal sign."
		" Mode is a point separated list of the keywords:"
		" @TOP@ (@T@), @BOTTOM@ (@B@), @VCENTER@ (@V@), @INS-TOP@ (@IT@)"
		" and @INS-BOTTOM@ (@IB@) for the vertical alignment,"
		" @LEFT@ (@L@), @RIGHT@ (@R@), @HCENTER@ (@H@), @INS-LEFT@ (@IL@)"
		" and @INS-RIGHT@ (@IR@) for the horizontal alignment,"
		" @COPY@ (@CP@), @BACKGROUND@ (@BG@), @FOREGROUND@ (@FG@)"
		" and @MIX@ (@MX@) as color calculation mode and"
		" @SHRINK@ (@SH@), @GROW@ (@GR@) and @LEAVE@ (@LV@)"
		" as image size mode."
		"\n "
		" Abbreviations are allowed."
		" The default is @VCENTER.HCENTER.MIX.LEAVE@"
		" or in short form @V.H.MX.LV@."
		" If this option is used multiple times all patch files"
		" will be used in the entered order." },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_CP,	"TRANSFORM",	"x|transform",
		0, 0 /* copy of wszst */ },

  { T_OPT_C,	"STRIP",	"strip",
		0, 0 /* copy of wszst */ },

  { T_SEP_OPT,	0,0,0,0 }, //----- separator -----

  { T_OPT_C,	"SECTIONS",	"sections",
		0, 0 /* copy of wszst */ },

  //
  //---------- wimgt GROUP DEST ----------

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

  //---------- wimgt GROUP CONVERT ----------

  { T_GRP_BEG,	"TRANSFORM",	0,0,0 },

  { T_COPT,	"N_MIPMAPS",	0,0,0 },
  { T_COPT,	"MAX_MIPMAPS",	0,0,0 },
  { T_COPT,	"MIPMAP_SIZE",	0,0,0 },
  { T_COPT,	"PRE_CONVERT",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"PATCH",	0,0,0 },
  { T_SEP_OPT,	0,0,0,0 },
  { T_COPT,	"MIPMAPS",	0,0,0 },
  { T_COPT,	"NO_MIPMAPS",	0,0,0 },
  { T_COPT,	"FAST_MIPMAPS",	0,0,0 },
  { T_COPT,	"CMPR_DEFAULT",	0,0,0 },
  { T_COPT,	"TRANSFORM",	0,0,0 },
  { T_COPT,	"STRIP",	0,0,0 },

  //
  //---------- COMMAND wimgt VERSION ----------

  { T_CMD_BEG,	"VERSION",	0,0,0 },

  { T_COPT,	"BRIEF",	0,0,
	"Print only the version number, revision and system."
	" If set twice, print only the version number." },
  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print in long format. Ignored if option {--sections} is set." },

  //---------- COMMAND wimgt HELP ----------

  { T_CMD_BEG,	"HELP",		0,0,0 },

  { T_COPT,	"WIDTH",	0,0,0 },
  { T_COPT,	"MAX_WIDTH",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wimgt CONFIG ----------

  { T_CMD_BEG,	"CONFIG",	0,0,0 },

  { T_COPT,	"CONFIG",	0,0,0 },
  { T_COPT,	"BRIEF",	0,0, CONFIG_BRIEF },
  { T_COPT,	"LONG",		0,0, CONFIG_LONG },

  //---------- COMMAND wimgt ARGTEST ----------

  { T_CMD_BEG,	"ARGTEST",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wimgt EXPAND ----------

  { T_CMD_BEG,	"EXPAND",	0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wimgt TEST ----------

  { T_CMD_BEG,	"TEST",		0,0,0 },
  { T_ALL_OPT,	0,		0,0,0 },

  //---------- COMMAND wimgt COLORS ----------

  { T_CMD_BEG,	"COLORS",	0,0,0 },

  { T_COPT,	"COLORS",	0,0,0 },
  { T_COPT,	"NO_COLORS",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print also a list of all colored text types."
	" If set twice, include alternative names."
	" If set three times, include raw color names." },
  { T_COPT,	"BRIEF",	0,0,
	"Ignore option --long and print a single status line about current color settings." },

  //---------- COMMAND wimgt ERROR ----------

  { T_CMD_BEG,	"ERROR",	0,0,0 },

  { T_COPT,	"SECTIONS",	0,0,0 },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"LONG",		0,0,
	"Print message text instead of message name." },
  { T_COPT,	"BRIEF",		0,0,
	"Print message number instead of message name." },

  //---------- COMMAND wimgt FILETYPE ----------

  { T_CMD_BEG,	"FILETYPE",	0,0,0 },

  { T_COPT,	"LONG",		0,0,
	"Print an additional column with the decompressed file type and version."
	" If entered twice, a validation status column is added." },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wimgt FILEATTRIB ----------

  { T_CMD_BEG,	"FILEATTRIB",	0,0,0 },

  { T_COPT,	"NO_HEADER",	0,0,0 },

  //---------- COMMAND wimgt LIST ----------

  { T_CMD_BEG,	"LIST",		0,0,0 },
  { T_COPT_M,	"LONG",		0,0,
	"If set, geometry data of the image is printed."
	" If set twice, advanced geometry data is printed." },
  { T_COPT,	"NO_HEADER",	0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },

  //---------- COMMAND wszst LIST-L ----------

  { T_CMD_BEG,	"LIST_L",	0,0,0 },
  { T_COPY_CMD,	"LIST",		0,0,0 },

  //---------- COMMAND wszst LIST-LL ----------

  { T_CMD_BEG,	"LIST_LL",	0,0,0 },
  { T_COPY_CMD,	"LIST_L",	0,0,0 },

  //---------- COMMAND wimgt DECODE ----------

  { T_CMD_BEG,	"DECODE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wimgt ENCODE ----------

  { T_CMD_BEG,	"ENCODE",	0,0,0 },
  { T_COPY_GRP,	"DEST",		0,0,0 },
  { T_COPT,	"IGNORE",	0,0,0 },
  { T_COPT,	"ALL",		0,0,
	"If source and destination file are the same,"
	" the file is only written on modifications (transform, patch, ...)."
	" If --all is set, then the file is always written." },
  { T_COPY_GRP,	"TRANSFORM",	0,0,0 },

  //---------- COMMAND wimgt CONVERT ----------

  { T_CMD_BEG,	"CONVERT",	0,0,0 },
  { T_COPY_CMD,	"ENCODE",	0,0,0 },

  //---------- COMMAND wimgt COPY ----------

  { T_CMD_BEG,	"COPY",		0,0,0 },
  { T_COPY_CMD,	"ENCODE",	0,0,0 },

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////

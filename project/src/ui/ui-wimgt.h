
/***************************************************************************
 *                       _______ _______ _______                           *
 *                      |  ___  |____   |  ___  |                          *
 *                      | |   |_|    / /| |   |_|                          *
 *                      | |_____    / / | |_____                           *
 *                      |_____  |  / /  |_____  |                          *
 *                       _    | | / /    _    | |                          *
 *                      | |___| |/ /____| |___| |                          *
 *                      |_______|_______|_______|                          *
 *                                                                         *
 *                           Wiimms SZS Tools                              *
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
 ***************************************************************************
 *                                                                         *
 *   >>>  This file is automatically generated by './src/gen-ui.c'.  <<<   *
 *   >>>                   Do not edit this file!                    <<<   *
 *                                                                         *
 ***************************************************************************/


#ifndef SZS_UI_WIMGT_H
#define SZS_UI_WIMGT_H
#include "dclib-basics.h"
#include "dclib-ui.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////                enum enumOptions                 ///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum enumOptions
{
	OPT_NONE,

	//----- command specific options -----

	OPT_LONG,
	OPT_NO_HEADER,
	OPT_BRIEF,
	OPT_DEST,
	OPT_DEST2,
	OPT_ESC,
	OPT_OVERWRITE,
	OPT_NUMBER,
	OPT_REMOVE_DEST,
	OPT_UPDATE,
	OPT_PRESERVE,
	OPT_IGNORE,
	OPT_ALL,
	OPT_MIPMAPS,
	OPT_NO_MIPMAPS,
	OPT_N_MIPMAPS,
	OPT_MAX_MIPMAPS,
	OPT_MIPMAP_SIZE,
	OPT_FAST_MIPMAPS,
	OPT_CMPR_DEFAULT,
	OPT_PRE_CONVERT,
	OPT_PATCH,
	OPT_TRANSFORM,
	OPT_STRIP,
	OPT_SECTIONS,

	OPT__N_SPECIFIC, // == 26

	//----- global options -----

	OPT_VERSION,
	OPT_HELP,
	OPT_XHELP,
	OPT_CONFIG,
	OPT_ALLOW_ALL,
	OPT_COMPATIBLE,
	OPT_WIDTH,
	OPT_MAX_WIDTH,
	OPT_NO_PAGER,
	OPT_QUIET,
	OPT_VERBOSE,
	OPT_LOGGING,
	OPT_EXT_ERRORS,
	OPT_TIMING,
	OPT_WARN,
	OPT_DE,
	OPT_COLORS,
	OPT_NO_COLORS,
	OPT_CT_CODE,
	OPT_LE_CODE,
	OPT_CHDIR,
	OPT_CONST,
	OPT_MAX_FILE_SIZE,
	OPT_TRACKS,
	OPT_ARENAS,
	OPT_UTF_8,
	OPT_NO_UTF_8,
	OPT_TEST,
	OPT_FORCE,
	OPT_REPAIR_MAGICS,
	OPT_OLD,
	OPT_STD,
	OPT_NEW,
	OPT_EXTRACT,

	OPT__N_TOTAL // == 60

} enumOptions;

//
///////////////////////////////////////////////////////////////////////////////
///////////////               enum enumOptionsBit               ///////////////
///////////////////////////////////////////////////////////////////////////////

//	*****  only for verification  *****

//typedef enum enumOptionsBit
//{
//	//----- command specific options -----
//
//	OB_LONG			= 1llu << OPT_LONG,
//	OB_NO_HEADER		= 1llu << OPT_NO_HEADER,
//	OB_BRIEF		= 1llu << OPT_BRIEF,
//	OB_DEST			= 1llu << OPT_DEST,
//	OB_DEST2		= 1llu << OPT_DEST2,
//	OB_ESC			= 1llu << OPT_ESC,
//	OB_OVERWRITE		= 1llu << OPT_OVERWRITE,
//	OB_NUMBER		= 1llu << OPT_NUMBER,
//	OB_REMOVE_DEST		= 1llu << OPT_REMOVE_DEST,
//	OB_UPDATE		= 1llu << OPT_UPDATE,
//	OB_PRESERVE		= 1llu << OPT_PRESERVE,
//	OB_IGNORE		= 1llu << OPT_IGNORE,
//	OB_ALL			= 1llu << OPT_ALL,
//	OB_MIPMAPS		= 1llu << OPT_MIPMAPS,
//	OB_NO_MIPMAPS		= 1llu << OPT_NO_MIPMAPS,
//	OB_N_MIPMAPS		= 1llu << OPT_N_MIPMAPS,
//	OB_MAX_MIPMAPS		= 1llu << OPT_MAX_MIPMAPS,
//	OB_MIPMAP_SIZE		= 1llu << OPT_MIPMAP_SIZE,
//	OB_FAST_MIPMAPS		= 1llu << OPT_FAST_MIPMAPS,
//	OB_CMPR_DEFAULT		= 1llu << OPT_CMPR_DEFAULT,
//	OB_PRE_CONVERT		= 1llu << OPT_PRE_CONVERT,
//	OB_PATCH		= 1llu << OPT_PATCH,
//	OB_TRANSFORM		= 1llu << OPT_TRANSFORM,
//	OB_STRIP		= 1llu << OPT_STRIP,
//	OB_SECTIONS		= 1llu << OPT_SECTIONS,
//
//	//----- group & command options -----
//
//	OB_GRP_DEST		= OB_DEST
//				| OB_DEST2
//				| OB_ESC
//				| OB_OVERWRITE
//				| OB_NUMBER
//				| OB_REMOVE_DEST
//				| OB_UPDATE
//				| OB_PRESERVE,
//
//	OB_GRP_TRANSFORM	= OB_N_MIPMAPS
//				| OB_MAX_MIPMAPS
//				| OB_MIPMAP_SIZE
//				| OB_PRE_CONVERT
//				| OB_PATCH
//				| OB_MIPMAPS
//				| OB_NO_MIPMAPS
//				| OB_FAST_MIPMAPS
//				| OB_CMPR_DEFAULT
//				| OB_TRANSFORM
//				| OB_STRIP,
//
//	OB_CMD_VERSION		= OB_BRIEF
//				| OB_SECTIONS
//				| OB_LONG,
//
//	OB_CMD_HELP		= ~(u64)0,
//
//	OB_CMD_CONFIG		= OB_BRIEF
//				| OB_LONG,
//
//	OB_CMD_ARGTEST		= ~(u64)0,
//
//	OB_CMD_EXPAND		= ~(u64)0,
//
//	OB_CMD_TEST		= ~(u64)0,
//
//	OB_CMD_COLORS		= OB_LONG
//				| OB_BRIEF,
//
//	OB_CMD_ERROR		= OB_SECTIONS
//				| OB_NO_HEADER
//				| OB_LONG
//				| OB_BRIEF,
//
//	OB_CMD_FILETYPE		= OB_LONG
//				| OB_IGNORE,
//
//	OB_CMD_FILEATTRIB	= OB_NO_HEADER,
//
//	OB_CMD_LIST		= OB_LONG
//				| OB_NO_HEADER
//				| OB_IGNORE,
//
//	OB_CMD_LIST_L		= OB_CMD_LIST,
//
//	OB_CMD_LIST_LL		= OB_CMD_LIST_L,
//
//	OB_CMD_DECODE		= OB_GRP_DEST
//				| OB_IGNORE
//				| OB_GRP_TRANSFORM,
//
//	OB_CMD_ENCODE		= OB_GRP_DEST
//				| OB_IGNORE
//				| OB_ALL
//				| OB_GRP_TRANSFORM,
//
//	OB_CMD_CONVERT		= OB_CMD_ENCODE,
//
//	OB_CMD_COPY		= OB_CMD_ENCODE,
//
//} enumOptionsBit;

//
///////////////////////////////////////////////////////////////////////////////
///////////////                enum enumCommands                ///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum enumCommands
{
	CMD__NONE,

	CMD_VERSION,
	CMD_HELP,
	CMD_CONFIG,
	CMD_ARGTEST,
	CMD_EXPAND,
	CMD_TEST,
	CMD_COLORS,
	CMD_ERROR,
	CMD_FILETYPE,
	CMD_FILEATTRIB,

	CMD_LIST,
	CMD_LIST_L,
	CMD_LIST_LL,

	CMD_DECODE,
	CMD_ENCODE,
	CMD_CONVERT,
	CMD_COPY,


	CMD__N // == 18

} enumCommands;

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   enumGetOpt                    ///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum enumGetOpt
{
	GO__ERR			= '?',

	GO_ARENAS		= 'A',
	GO_BRIEF		= 'B',
	GO_DEST2		= 'D',
	GO_ESC			= 'E',
	GO_NO_HEADER		= 'H',
	GO_LOGGING		= 'L',
	GO_MAX_FILE_SIZE	= 'M',
	GO_PATCH		= 'P',
	GO_TRACKS		= 'T',
	GO_VERSION		= 'V',
	GO_WARN			= 'W',
	GO_EXT_ERRORS		= 'Y',

	GO_ALL			= 'a',
	GO_CONST		= 'c',
	GO_DEST			= 'd',
	GO_HELP			= 'h',
	GO_IGNORE		= 'i',
	GO_LONG			= 'l',
	GO_OVERWRITE		= 'o',
	GO_PRESERVE		= 'p',
	GO_QUIET		= 'q',
	GO_REMOVE_DEST		= 'r',
	GO_TEST			= 't',
	GO_UPDATE		= 'u',
	GO_VERBOSE		= 'v',
	GO_TRANSFORM		= 'x',

	GO_XHELP		= 0x80,
	GO_CONFIG,
	GO_ALLOW_ALL,
	GO_COMPATIBLE,
	GO_WIDTH,
	GO_MAX_WIDTH,
	GO_NO_PAGER,
	GO_TIMING,
	GO_DE,
	GO_COLORS,
	GO_NO_COLORS,
	GO_CT_CODE,
	GO_LE_CODE,
	GO_CHDIR,
	GO_UTF_8,
	GO_NO_UTF_8,
	GO_FORCE,
	GO_REPAIR_MAGICS,
	GO_OLD,
	GO_STD,
	GO_NEW,
	GO_EXTRACT,
	GO_NUMBER,
	GO_MIPMAPS,
	GO_NO_MIPMAPS,
	GO_N_MIPMAPS,
	GO_MAX_MIPMAPS,
	GO_MIPMAP_SIZE,
	GO_FAST_MIPMAPS,
	GO_CMPR_DEFAULT,
	GO_PRE_CONVERT,
	GO_STRIP,
	GO_SECTIONS,

} enumGetOpt;

//
///////////////////////////////////////////////////////////////////////////////
///////////////                  external vars                  ///////////////
///////////////////////////////////////////////////////////////////////////////

//extern const InfoOption_t OptionInfo[OPT__N_TOTAL+1];
//extern const KeywordTab_t CommandTab[];
//extern const char OptionShort[];
//extern const struct option OptionLong[];
//extern u8 OptionUsed[OPT__N_TOTAL+1];
//extern const OptionIndex_t OptionIndex[UIOPT_INDEX_SIZE];
//UIOPT_INDEX_SIZE := 0x140 = 320
//extern const InfoCommand_t CommandInfo[CMD__N+1];
extern const InfoUI_t InfoUI_wimgt;

//
///////////////////////////////////////////////////////////////////////////////
///////////////                       END                       ///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_UI_WIMGT_H


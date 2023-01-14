
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

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lib-std.h"
#include "lib-bmg.h"
#include "lib-ctcode.h"
#include "dclib-basics.h"
#include "dclib-xdump.h"
#include "dclib-debug.h"
#include "ui.h" // [[dclib]] wrapper
#include "dclib-ui.h"
#include "version.h"

#include "dclib-gen-ui.h"
#include "ui-head.inc"

//----------------------------------
// \1 : Text only for built in help
// \2 : Text only for ui.def
// \3 : Text for all (default)
//----------------------------------

//
///////////////////////////////////////////////////////////////////////////////
///////////////		functions not neded by gen-ui		///////////////
///////////////////////////////////////////////////////////////////////////////

bool DefineIntVar ( VarMap_t * vm, ccp varname, int value ) { return false; }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			some helper macros		///////////////
///////////////////////////////////////////////////////////////////////////////

#define UI_TO_TEXT(x) UI_TO_TEXT1(x)
#define UI_TO_TEXT1(x) #x

#define ARCHIVES_OR	"SZS, U8, PACK, BRRES, BREFF, BREFT or RARC"
#define ARCHIVES_AND	"SZS, U8, PACK, BRRES, BREFF, BREFT and RARC"
#define ARCHIVES_COMMA	"SZS, U8, PACK, BRRES, BREFF, BREFT, RARC"

// create archives
#define CARCHIVES_OR	"SZS, U8, PACK, BRRES, BREFF or BREFT"
#define CARCHIVES_AND	"SZS, U8, PACK, BRRES, BREFF and BREFT"
#define CARCHIVES_COMMA	"SZS, U8, PACK, BRRES, BREFT"

#define MAX_GAMEMODES "1573"

#define NO_HEAD_SYNTAX "Suppress the syntax information section in decoded text files."

#define PATCH_BMG_TEXT(x) \
	"This option specifies a BMG patch mode." \
	" Some of the modes need a parameter or a file name of a BMG patch file" \
	" (raw or text BMG), both separated by an equal sign." \
	" Modes with required file names are @PRINT, REPLACE, INSERT," \
	" OVERWRITE, DELETE, MASK, EQUAL@ and @NOTEQUAL@." \
	" Modes with text parameter are @FORMAT@, @REGEXP@ and @RM-REGEXP@. " \
	" Standalone modes are @ID, ID-ALL, UNICODE, RM-ESCAPES, RM-CUPS," \
	" CT-COPY, CT-FORCE-COPY, CT-FILL, LE-COPY, LE-FORCE-COPY, LE-FILL," \
	" X-COPY, X-FORCE-COPY@ and @X-FILL@." \
	" Unique abbreviations are allowed." \
	"\n " \
	" The optional condition @COND@ is either '@?MID@' or '@!MID@'." \
	" In case of '@?MID@', the patch is only applied" \
	" if the message id MID already exists." \
	" In case of '@!MID@', the patch is only applied" \
	" if the message id MID does not exists." \
	"\n " \
	" If this option is used multiple times," \
	" all patch files will be processed in the entered order." \
	x \
	"\1 Read https://szs.wiimm.de/opt/patch-bmg for more details."

#define BMG_BRIEF_TEXT \
	"If set, the information header in decoded text files" \
	" is suppressed (for historical reasons same as {--no-header})." \
	" If set at least twice, all comments are suppressed" \
	" and the output is packed without empty lines." \
	" If set 3 times, the @#BMG@ file indentification is also suppressed."

#define BMG_LONG_TEXT \
	"Print long numeric message IDs instead of" \
	" alternative message names like Txx, Uxx or Mxx."

#define OPT_POINTS \
	"defines a new table for the points assigned to players after" \
	" a versus or room race." \
	" The parameter is a blank or comma and slash (next row) separated list" \
	" with numbers (points) between 0 and 255 and the following keywords:" \
	" @NINTENDO, LINEAR, WIN, WIN15, WIN25@ to select a predefined table;" \
	" @NO-BONUS, BONUS, NULL, ONE@ as options;" \
	" @N1..N12@ to select the row for N players." \
	"\1 See https://szs.wiimm.de/opt/points for details."

#define OPT_CT_DIR(x) \
	"Define a search directory for CT-CODE parts and use the internal" \
	" copies only, if no valid file is found." \
	" Each file is searched without and then with extension '.bz2'." \
	" For both cases bzip2 files are detected and decompressed." \
	" Only files with correct file type are accepted." \
	x \
	"\n " \
	" Use this option multiple times to define more than one search directory."

#define FUNCTION_BRIEF \
	"Suppress the output of the description and print only function type and syntax."

#define FUNCTION_LONG \
	"Usually only the function syntax is compared to the keywords." \
	" But if --long is set, the descriptions are compared too."

#define CT_CODE_SOURCE \
	"BRRES, TEX0, CT-CODE, CT-TEXT and LE-BIN"

#define DEPRECATED "[DEPRECATED] "

#define CONFIG_CMD \
	"Show all information about the search" \
	" for the configuration file and its content."

#define CONFIG_BRIEF \
	" Suppress configuration search list."

#define CONFIG_LONG \
	"If set, print the global search list too." \
	" If set twice, print the auto-add search list too."

#define GENERIC_IMAGE(src) \
	"If " src " starts with colon (@:@), then it may be a generic image" \
	" instead of a real file." \
	"\1 See https://szs.wiimm.de/doc/genericimg for details."

#define WILDCARDS \
	"\1Wildcards are accepted, see https://szs.wiimm.de/doc/wildcards for details." \
	"\2$Wildcards$ are accepted.\3"

#define WILDCARDS_SORT \
	"\1Wildcards are accepted, see https://szs.wiimm.de/doc/wildcards for details." \
	"\2$Wildcards$ are accepted.\3" \
	" After evaluating the wildcards, all input files are sorted according" \
	" to their path and duplicates are deleted."

//
///////////////////////////////////////////////////////////////////////////////
///////////////			the info table			///////////////
///////////////////////////////////////////////////////////////////////////////

info_t info_tab[] =
{
    #include "tab-wszst.inc"
    #include "tab-wbmgt.inc"
    #include "tab-wctct.inc"
    #include "tab-wimgt.inc"
    #include "tab-wkclt.inc"
    #include "tab-wkmpt.inc"
    #include "tab-wlect.inc"
    #include "tab-wmdlt.inc"
    #include "tab-wpatt.inc"
    #include "tab-wstrt.inc"

    { T_END, 0,0,0,0 }
};

#include "dclib-gen-ui.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    main()			///////////////
///////////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
    return ui_main(argc,argv);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////


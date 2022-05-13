
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

// all about BMG files:
//	http://wiki.tockdom.de/index.php?title=BMG

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_BMG_H
#define SZS_LIB_BMG_H 1

#include "lib-bmg.h"
#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct bmg_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupBMG ( ccp opt_msg );

enumError LoadXBMG
(
    bmg_t		* bmg,		// pointer to valid bmg
    bool		initialize,	// true: initialize 'bmg'
    ccp			fname,		// filename of source
    bool		allow_archive,	// true: concat BMGs of archives to 1 source
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

struct raw_data_t;
enumError ScanRawDataBMG
(
    bmg_t		* bmg,		// BMG data structure
    bool		init_bmg,	// true: initialize 'bmg' first
    struct raw_data_t	* raw		// valid raw data
);

static inline file_format_t GetFF_BMG ( const bmg_t * bmg )
{
    DASSERT(bmg);
    return bmg->is_text_src ? FF_BMG_TXT : FF_BMG;
}

static inline ccp GetNameFF_BMG ( const bmg_t * bmg )
{
    DASSERT(bmg);
    return GetNameFF( bmg->is_text_src ? FF_BMG_TXT : FF_BMG, 0 );
}

//-----------------------------------------------------------------------------

enumError SaveRawXBMG
(
    bmg_t		* bmg,		// pointer to valid bmg
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

enumError SaveTextXBMG
(
    bmg_t		* bmg,		// pointer to valid bmg
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

extern int have_bmg_patch_count;

int ScanOptPatchMessage ( ccp arg );
int ScanOptMacroBMG ( ccp arg );
enumError SetupPatchingListBMG();
void ResetPatchingListBMG();
enumError UsePatchingListBMG ( bmg_t * dest );
uint ListPatchingListBMG ( FILE *f, int indent, ccp title );
ccp GetPatchNameBMG ( uint cmd );


enumError DiffBMG
(
    const bmg_t		* bmg1,		// first bmg
    const bmg_t		* bmg2,		// second bmg
    bool		quiet,		// true: be quiet
    int			width		// width out output lines
);

//-----------------------------------------------------------------------------

int ScanOptBmgEndian	( ccp arg );
int ScanOptBmgEncoding	( ccp arg );
int ScanOptBmgInfSize	( ccp arg, bool is_wbmgt );
int ScanOptBmgMid	( ccp arg );
int ScanOptForceAttrib	( ccp arg );
int ScanOptDefAttrib	( ccp arg );
int ScanOptLoadBMG	( ccp arg );

extern bmg_t opt_load_bmg;
extern bool opt_patch_names;

int ScanOptFilterBMG ( ccp arg );

#define MAX_FILTER_MSG 0x10000
extern u8 * filter_message; // NULL or field with 'MAX_FILTER_MSG' elements

enumError cmd_bmg_cat ( bool mix );
enumError cmd_bmg_identifier();

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_BMG_H


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

#ifndef SZS_LIB_ANALYZE_H
#define SZS_LIB_ANALYZE_H 1

#define _GNU_SOURCE 1

#include "lib-std.h"
#include "lib-checksum.h"
#include "lib-lex.h"
#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyze_param_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[analyze_param_t]]

typedef struct analyze_param_t
{
    ccp			fname;		// NULL or source filename
    szs_file_t		szs;		// source file
    File_t		fo;		// output file
    PrintScript_t	ps;		// script options
}
analyze_param_t;

//-----------------------------------------------------------------------------

void PrintHeaderAP
(
    analyze_param_t	*ap,		// valid structure
    ccp			analyze		// analyzed object
);

void PrintFooterAP
(
    analyze_param_t	*ap,		// valid structure
    bool		valid,		// true if source is valid
    u_usec_t		duration_usec,	// 0 or duration of analysis
    ccp			warn_format,	// NULL or format for warn message
    ...					// arguments for 'warn_format'
) __attribute__ ((__format__(__printf__,4,5)));

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyze_szs_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[analyze_szs_t]]

typedef struct analyze_szs_t
{
    //--- db+sha1 check sums

    char	db64[CHECKSUM_DB_SIZE+1];
					// DB64 checksum
    sha1_hex_t	sha1_szs;		// SHA1 of SZS file
    sha1_hex_t	sha1_szs_norm;		// SHA1 of normed SZS file
    sha1_hex_t	sha1_kmp;		// SHA1 of KMP file
    sha1_hex_t	sha1_kmp_norm;		// SHA1 of normed KMP file
    sha1_hex_t	sha1_kcl;		// SHA1 of KCL file
    sha1_hex_t	sha1_course;		// SHA1 of course-model
    sha1_hex_t	sha1_vrcorn;		// SHA1 of vrcorn
    sha1_hex_t	sha1_minimap;		// SHA1 of minimap

    //--- slots of orgiginal tracks: 0:none, 11.84:track 111..125:arena

    u8		sha1_kmp_slot;
    u8		sha1_kmp_norm_slot;
    u8		sha1_kcl_slot;
    u8		sha1_minimap_slot;

    //--- sub files

    lex_info_t		lexinfo;	// LEX info
    slot_ana_t		slotana;	// slot data
    slot_info_t		slotinfo;	// slot data
    kmp_finish_t	kmp_finish;	// finish line
    kmp_usedpos_t	used_pos;	// used positions
    szs_have_t		have;		// complete have status, copy of szs_file_t::have


    //--- more stats

    int		ckpt0_count;		// number of LC in CKPT, -1 unknown
    int		lap_count;		// STGI lap counter
    float	speed_factor;		// STGI speed factor
    u16		speed_mod;		// STGI speed mod
    u8		have_common;		// 1: found at least one common file
    u8		valid_track;		// 1: valid track or arena file

    char	gobj_info[20];		// gobj counters
    char	ct_attrib[300];		// collected ct attributes

    u_usec_t	duration_usec;		// duration of AnalyzeSZS() in usec
}
analyze_szs_t;

//-----------------------------------------------------------------------------

void InitializeAnalyzeSZS ( analyze_szs_t * as );
void ResetAnalyzeSZS ( analyze_szs_t * as );

void AnalyzeSZS
(
    analyze_szs_t	*as,		// result
    bool		init_sa,	// true: init 'as', false: reset 'as'
    szs_file_t		*szs,		// SZS file to analysze
    ccp			fname		// NULL or fname for slot analysis
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			exec analyze			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExecAnalyzeSZS	( analyze_param_t *ap );
enumError ExecAnalyzeLECODE	( analyze_param_t *ap );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_ANALYZE_H

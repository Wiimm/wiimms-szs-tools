

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

#ifndef SZS_LIB_OBJECT_H
#define SZS_LIB_OBJECT_H 1

#include "lib-std.h"
#include "db-file.h"
#include "db-object.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			status vectors			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[UsedObject_t]] [[UsedFile_t]] [[UsedFileSZS_t]]
// [[UsedFileSHA1_t]] [[UsedFileFILE_t]] [[UsedFileGROUP_t]]

typedef struct UsedObject_t	{ u8 d[N_KMP_GOBJ];	} UsedObject_t;
typedef struct UsedFile_t	{ u8 d[N_DB_FILE];	} UsedFile_t;
typedef struct UsedFileSZS_t	{ u8 d[N_DB_FILE_SZS];	} UsedFileSZS_t;
typedef struct UsedFileSHA1_t	{ u8 d[N_DB_FILE_SHA1]; } UsedFileSHA1_t;
typedef struct UsedFileFILE_t	{ u8 d[N_DB_FILE_FILE]; } UsedFileFILE_t;
typedef struct UsedFileGROUP_t	{ u8 d[N_DB_FILE_GROUP];} UsedFileGROUP_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Find DB files			///////////////
///////////////////////////////////////////////////////////////////////////////

int FindDbFile
(
    // returns the index of the file or -1 if not found

    ccp			file		// file to find, preeceding './' possible
);

///////////////////////////////////////////////////////////////////////////////

uint FindDbFileByGroup
(
    // returns the number of found files

    UsedFileFILE_t	*status,	// modify a byte for each found file
    bool		init_status,	// clear 'status' first
    uint		group,		// index of group (robust)
    u8			mask		// =0: add +1 / >0: OR mask
);

//-----------------------------------------------------------------------------

uint FindDbFileByGroups
(
    // returns the number of found files

    UsedFileFILE_t		*status,	// modify a byte for each found file
    bool			init_status,	// clear 'status' first
    const UsedFileGROUP_t	*group		// NULL or list of used groups
);

///////////////////////////////////////////////////////////////////////////////

uint FindDbFileByObject
(
    // returns the number of found files

    UsedFileFILE_t	*status,	// modify a byte for each found file
    bool		init_status,	// clear 'status' first
    uint		object,		// index of object (robust)
    u8			mask		// =0: add +1 / >0: OR mask
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Find DB groups			///////////////
///////////////////////////////////////////////////////////////////////////////

uint FindDbGroupByObject
(
    // returns the number of found groups

    UsedFileGROUP_t	*status,	// modify a byte for each found file
    bool		init_status,	// clear 'status' first
    uint		object,		// index of object (robust)
    u8			mask		// =0: add +1 / >0: OR mask
);

//-----------------------------------------------------------------------------

uint FindDbGroupByObjects
(
    // returns the number of found groups

    UsedFileGROUP_t		*status,	// modify a byte for each found file
    bool			init_status,	// clear 'status' first
    const UsedObject_t		*object		// NULL or list of used objects
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			have_kmp_obj_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[have_kmp_obj_t]]
// order is important, compare have_kmp_info[] and ct.wiimm.de

typedef enum have_kmp_obj_t
{
    HAVEKMP_WOODBOX_HT,		// 0x070:woodbox   setting #8>0  => fall height
    HAVEKMP_MUSHROOM_CAR,	// 0x0d8:penguin_m setting #8>0  => mushroom car
    HAVEKMP_PENGUIN_POS,	// 0x0d8:penguin_m setting #2>0  => start pos
    HAVEKMP_SECOND_KTPT,	// Second KTPT with index -1
    HAVEKMP_X_PFLAGS,		// Extended presence flags (XPF)
    HAVEKMP_X_COND,		// XPF: Conditional object
    HAVEKMP_X_DEFOBJ,		// XPF: Definition object
    HAVEKMP_X_RANDOM,		// XPF: Random scenarios
    HAVEKMP_EPROP_SPEED,	// 0x1a6:Epropeller   setting #8>0  => speed
    HAVEKMP_COOB_R,		// Conditional Out of Bounds by Riidefi
    HAVEKMP_COOB_K,		// Conditional Out of Bounds by kHacker35000vr
    HAVEKMP_UOOB,		// Unconditional Out of Bounds
    HAVEKMP_GOOMBA_SIZE,	// 0x191:goomba (kuribo) with scale !0 1.00
    HAVEKMP__N
}
__attribute__ ((packed)) have_kmp_obj_t;

//-----------------------------------------------------------------------------
// [[kmp_special_t]]

typedef u8 kmp_special_t[HAVEKMP__N]; // >0: number of special KMP objects found

//-----------------------------------------------------------------------------
// [[kmp_obj_info_t]]

typedef struct kmp_obj_info_t
{
    u32		skip;	// >0: if found: skip next # messages
    ccp		name;	// short name
    ccp		info;	// info for check
}
kmp_obj_info_t;

extern const kmp_obj_info_t have_kmp_info[HAVEKMP__N];

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////

void DefineObjectNameVars ( VarMap_t *vm );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_OBJECT_H


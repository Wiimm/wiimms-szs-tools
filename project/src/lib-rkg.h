
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

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_RKD_H
#define SZS_LIB_RKD_H 1

//#include "lib-std.h"
#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define RKG_MAGIC		"RKGD"
#define RKG_MAGIC_NUM		0x524b4744

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct rkg_head_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[rkg_head_t]]
// https://wiki.tockdom.com/wiki/RKG

typedef struct rkg_head_t
{
  /* 0x00 */	u32	magic;		// RKG_MAGIC_NUM
  /* 0x04 */	u8	score[3];	// 7:min 7:sec 10:milli
  /* 0x07 */	u8	track_id;
  /* 0x08 */	u32	id;		// different IDs
  /* 0x0c */	u16	type;
  /* 0x0e */	u16	data_size;	// decompressed data size
  /* 0x10 */	u8	any_data[0x88-0x10];
  /* 0xxx */
  /* 0x88 */
}
__attribute__ ((packed)) rkg_head_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct rkg_info_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[rkg_info_t]]

typedef struct rkg_info_t
{
    bool valid;			// true: data is valid
    ccp  error;			// not NULL: error message
    u32	 score;			// time in msec

    uint data_size;		// input data size
    bool is_compressed;		// true: data is compressed

    uint n_but;
    uint n_dir;
    uint n_trick;

    uint time_but;
    uint time_dir;
    uint time_trick;
}
rkg_info_t;

///////////////////////////////////////////////////////////////////////////////

static inline void InitializeRKGInfo ( rkg_info_t *ri )
	{ DASSERT(ri); memset(ri,0,sizeof(*ri)); }

static inline void ResetRKGInfo ( rkg_info_t *ri )
	{ DASSERT(ri); memset(ri,0,sizeof(*ri)); }

enumError ScanRawDataGHOST
(
    rkg_info_t		* ri,		// RKG info
    bool		init_ri,	// true: initialize 'ri' first
    struct raw_data_t	* raw		// valid raw data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_MDL_H

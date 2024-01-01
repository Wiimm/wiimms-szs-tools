
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
 *   Copyright (c) 2011-2024 by Dirk Clemens <wiimm@wiimm.de>              *
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

#ifndef SZS_LIB_RKC_H
#define SZS_LIB_RKC_H 1

#include "lib-std.h"

///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define RKCT_MAGIC		"RKCT"
#define RKCT_MAGIC_NUM		0x524b4354

#define RKCO_MAGIC		"RKCO"
#define RKCO_MAGIC_NUM		0x524b434f
#define RKCO_FILENAME		"rkco.bin"

#define RKC_VERSION		 0x640
#define RKC_U8_OFFSET		  0x50
#define RKC_U8_LIMIT		0x5000
#define RKC_U8_FILENAME		"rkc.szs"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct rkct_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[rkco_t]]

typedef struct rkco_t
{
   /*00*/   u32		magic;		// always RKCO_MAGIC_NUM
   /*04*/   u8		xdata[0x40-4];	// misc data
}
__attribute__ ((packed)) rkco_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct rkct_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[rkct_t]]

typedef struct rkct_t
{
   /*00*/   u32		magic;		// always RKCT_MAGIC_NUM
   /*04*/   u32		size;		// total RKC size
   /*08*/   u32		u8_off;		// Offset uf U8-archice (YAZ0 compressed)
					// usually RKC_U8_OFFSET
   /*0c*/   u32		version;	// always 0x640 (RKC_VERSION)
   /*ff*/   rkco_t	rkco[0];
}
__attribute__ ((packed)) rkct_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    interface			///////////////
///////////////////////////////////////////////////////////////////////////////

struct szs_iterator_t;

int IterateFilesRKC
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------

struct szs_file_t;

enumError CreateRKC
(
    struct szs_file_t	*szs,		// valid szs
    ccp			source_dir	// NULL or path to source dir
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_RKC_H

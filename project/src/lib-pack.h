
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
 *   Copyright (c) 2011-2021 by Dirk Clemens <wiimm@wiimm.de>              *
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

#ifndef SZS_LIB_PACK_H
#define SZS_LIB_PACK_H 1

#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PACK support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define PACK_MAGIC		"PACK"
#define PACK_MAGIC_NUM		0x5041434b

#define PACK_METRIC_ALIGN	0x10
#define PACK_SUBFILE_ALIGN	0x20

///////////////////////////////////////////////////////////////////////////////

typedef struct pack_header_t
{
    u32		magic;			// := PACK_MAGIC_NUM
    u32		file_size;		// total size of pack file
    u32		n_files;		// total number of subfiles
    u32		offset_metric;		// offset to (offset,size) table
    char	name_pool[0];		// pool with names
}
__attribute__ ((packed)) pack_header_t;

///////////////////////////////////////////////////////////////////////////////

typedef struct pack_metric_t
{
    u32		offset;			// offset of subfile
    u32		size;			// size of subfile
}
__attribute__ ((packed)) pack_metric_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PACK interface			///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesPACK
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

///////////////////////////////////////////////////////////////////////////////

enumError CreatePACK
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			total_size,	// total file size
    SetupParam_t	* setup_param	// NULL or setup parameters
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_PACK_H

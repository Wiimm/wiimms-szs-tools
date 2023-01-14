
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

// all about SZS, YAZ0 and U8 files:
//	https://wiki.tockdom.de/index.php?title=SZS
//	https://wiki.tockdom.de/index.php?title=YAZ0
//	https://wiki.tockdom.de/index.php?title=U8

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_RARC_H
#define SZS_LIB_RARC_H 1

#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			RARC support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define RARC_MAGIC		"RARC"
#define RARC_MAGIC_NUM		0x52415243

///////////////////////////////////////////////////////////////////////////////
// http://hitmen.c02.at/files/yagcd/yagcd/chap15.html#sec15.3

typedef struct rarc_file_header_t
{
 /* 0x00 */   be32_t	magic;		// = RARC_MAGIC_NUM
 /* 0x04 */   be32_t	file_size;	// size of file
 /* 0x08 */   be32_t	header_off;	// offset of 'rarc_header_t'
 /* 0x0c */   be32_t	header_size;	// size of header

 /* 0x10 */   be32_t	unknown_10;	// perhaps padding
 /* 0x14 */   be32_t	unknown_14;	//   "
 /* 0x18 */   be32_t	unknown_18;	//   "
 /* 0x1c */   be32_t	unknown_1c;	//   "
}
__attribute__ ((packed)) rarc_file_header_t;

//-----------------------------------------------------------------------------

typedef struct rarc_header_t
{
 /* 0x00 */   be32_t	n_node;		// number of nodes
 /* 0x04 */   be32_t	root_off;	// offset of root
 /* 0x08 */   be32_t	n_entry;	// total number of entires
 /* 0x0c */   be32_t	entry_off;	// offset of file entries
 /* 0x10 */   be32_t	str_pool_size;	// size of string pool
 /* 0x14 */   be32_t	str_pool_off;	// offset of the string pool

 /* 0x18 */   be16_t	unknown_18;	// maybe == 'n_entry'
 /* 0x1a */   be16_t	unknown_1a;
 /* 0x1c */   be32_t	unknown_1c;
}
__attribute__ ((packed)) rarc_header_t;

//-----------------------------------------------------------------------------

typedef struct rarc_node_t
{
 /* 0x00 */   char	name[4];	// uppercase of first 4 chars of filename
					// 'ROOT' for very first node
 /* 0x04 */   be32_t	name_off;	// offset into string pool
 /* 0x08 */   be16_t	unknown_08;
 /* 0x0a */   be16_t	n_entry;	// number of entires
 /* 0x0c */   be32_t	entry_index;	// index of first entry
}
__attribute__ ((packed)) rarc_node_t;

//-----------------------------------------------------------------------------

typedef struct rarc_entry_t
{
 /* 0x00 */   be16_t	id;		// file: index of file == entry_number
					// dir:  0xffff
 /* 0x02 */   be16_t	unknown_02;	// ? checksum over filename
 /* 0x04 */   be16_t	unknown_04;	// ? file mode
					//  - 0200 : directories
					//  - 1100 : plain files
 /* 0x06 */   be16_t	name_off;	// offset into string pool
 /* 0x08 */   be32_t	data_off;	// file: offset of data,
					// dir:  index of node
 /* 0x0c */   be32_t	data_size;	// file: size of data
 /* 0x10 */   be32_t	unknown_10;	// ? always 0
}
__attribute__ ((packed)) rarc_entry_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			RARC interface			///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesRARC
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_RARC_H

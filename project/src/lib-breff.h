
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

#ifndef SZS_LIB_BREFF_H
#define SZS_LIB_BREFF_H 1

#include "lib-brres.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		   BREFF + BREFT file header		///////////////
///////////////////////////////////////////////////////////////////////////////

// BREFF + BREFT uses the same header like BRRES, but other magics

#define BREFF_MAGIC		"REFF"
#define BREFF_MAGIC_NUM		0x52454646
#define BREFF_DEFAULT_ALIGN	0x04

#define BREFT_MAGIC		"REFT"
#define BREFT_MAGIC_NUM		0x52454654
#define BREFT_9_DEFAULT_ALIGN	0x20
#define BREFT_11_DEFAULT_ALIGN	0x40

//
///////////////////////////////////////////////////////////////////////////////
///////////////			breff root			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct breff_root_head_t
{
    char		magic[4];	// magic
    u32			data_size;	// data size
    u8			data[0];	// data, starting with a breff_root_t

}
__attribute__ ((packed)) breff_root_head_t;

//-----------------------------------------------------------------------------

typedef struct breff_root_t
{
    u32			first_item_off;	// offset of first breff_item_t
    u32			unknown1;	// always 0 in MKW
    u32			unknown2;	// always 0 in MKW
    u16			name_len0;	// length of name including terminating NULL
    u16			unknown3;	// always 0 in MKW
    char		name[0];	// NULL terminated name
    //u8		data[];		// padding or real data
}
__attribute__ ((packed)) breff_root_t;

///////////////////////////////////////////////////////////////////////////////

typedef struct breff_item_name_t
{
    u16			name_len0;	// length of name including terminating NULL
    //char		name[0];	// NULL terminated name
					// breff_item_data_t follows not aligned
					// -> disabled because of MVC
}
__attribute__ ((packed)) breff_item_name_t;

//-----------------------------------------------------------------------------

typedef struct breff_item_data_t
{
    u32			offset;		// data offset relative to item list
    u32			size;		// data size
}
__attribute__ ((packed)) breff_item_data_t;

//-----------------------------------------------------------------------------

typedef struct breff_item_list_t
{
    u32			size;		// size of all items
    u16			n_item;		// number of itmes
    u16			unknown1;
    breff_item_name_t	item[0];	// first item
}
__attribute__ ((packed)) breff_item_list_t;

//-----------------------------------------------------------------------------

typedef struct breft_image_t
{
    u32		unknown1;	// always 0
    u16		width;
    u16		height;
    u32		img_size;
    u8		iform;
    u8		unknown2[3+4];	// nearly always 0
    u8		n_image;	// >0: number of images (=N(mipmaps)+1)
    u8		unknown3[3+8];	// nearly always 0
    u8		data[];		// image data
}
__attribute__ ((packed)) breft_image_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Interface			///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesBREFF
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------

enumError CreateBREFF
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

#endif // SZS_LIB_BREFF_H

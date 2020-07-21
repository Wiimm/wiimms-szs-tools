
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
 *   Copyright (c) 2011-2020 by Dirk Clemens <wiimm@wiimm.de>              *
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
//	http://wiki.tockdom.de/index.php?title=SZS
//	http://wiki.tockdom.de/index.php?title=YAZ0
//	http://wiki.tockdom.de/index.php?title=U8

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_BRRES_H
#define SZS_LIB_BRRES_H 1

#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRRES support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateBRRES
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			total_size	// total file size
);

//-----------------------------------------------------------------------------

int IterateFilesBRRES
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------

int IterateFilesBRSUB
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------

int IterateFilesMDL
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------

int IterateFilesPAT
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------

int IterateFilesFuncBRSUB
(
    brsub_cut_t		*bcut,		// pointer to data structure
    int			grp,		// index of group, <0: grp_entry_t
    int			entry		// index of entry, -1: no entry
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[brres_info_t]]

typedef struct brres_info_t
{
    u16  id;		// id
    u16  left_idx;	// left index
    u16  right_idx;	// right index
    u32	 name_off;	// name offset
    u32	 data_off;	// data offset
    ccp  name;		// pointer to name
    uint nlen;		// length of name

} brres_info_t;

///////////////////////////////////////////////////////////////////////////////

void CalcEntryBRRES
(
    brres_info_t	* info,
    uint		entry_idx
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRRES subfiles			///////////////
///////////////////////////////////////////////////////////////////////////////

#define MDL_MAGIC		"MDL0"
#define TEX_MAGIC		"TEX0"
#define SRT_MAGIC		"SRT0"
#define CHR_MAGIC		"CHR0"
#define PAT_MAGIC		"PAT0"
#define CLR_MAGIC		"CLR0"
#define SHP_MAGIC		"SHP0"
#define SCN_MAGIC		"SCN0"

#define MDL_MAGIC_NUM		0x4d444c30
#define TEX_MAGIC_NUM		0x54455830
#define SRT_MAGIC_NUM		0x53525430
#define CHR_MAGIC_NUM		0x43485230
#define PAT_MAGIC_NUM		0x50415430
#define CLR_MAGIC_NUM		0x434c5230
#define SHP_MAGIC_NUM		0x53485030
#define SCN_MAGIC_NUM		0x53434e30

#define MDL_TEXT_MAGIC		"#MDL"
#define MDL_TEXT_MAGIC_NUM	0x234d444c

#define PAT_TEXT_MAGIC		"#PAT"
#define PAT_TEXT_MAGIC_NUM	0x23504154

//-----------------------------------------------------------------------------
// [[brsub_list_t]]

typedef struct brsub_list_t
{
    u8			* data;		// valid pointer to data
    size_t		size;		// size of 'data'
    struct brsub_list_t	* next;		// NULL or pointer to next element

} brsub_list_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetStringBRSUB
(
    const void		* data,		// relevant data
    const void		* end_of_src,	// end of 'data'
    u32			min_sp_off,	// minimum string pool offset
					//   maybe end of sub file, 0 if unknown
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid,	// result if invalid
    const endian_func_t * endian	// NULL (=use be) or endian functions
);

struct brsub_iterator_t;
ccp GetStringIteratorBRSUB
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    const void		* base,		// pointer to base for calculation
    const void		* ptr,		// pointer to u32 string offset
    ccp			if_null,	// result if offset == NULL
    ccp			if_invalid	// result if invalid
);

ccp GetStringBRRES
(
    const szs_file_t	* szs,		// base BRRES file
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid	// result if invalid
);

///////////////////////////////////////////////////////////////////////////////
// [[brsub_warn_t]]

typedef enum brsub_warn_t
{
	BIMD_OK,	// original record
	BIMD_HINT,	// hint about wrong version -> will work
	BIMD_FAIL,	// wrong n_sect, but will work (garbage display)
	BIMD_FATAL,	// wrong n_sect, will freeze
}
__attribute__ ((packed)) brsub_warn_t;

//-----------------------------------------------------------------------------

// [[brsub_info_t]] // BRSUB info

typedef struct brsub_info_t
{
    file_format_t	fform1;	// main file format of this record
    file_format_t	fform2;	// alternativ file format of this record

    u8			version;	// sub version of this record
    u8			good_version;	// expected version for 'fform'
    s8			n_sect;		// number of sections
    s8			good_sect;	// expected sections for 'fform'
    brsub_warn_t	warn;		// warning level
}
brsub_info_t;

//-----------------------------------------------------------------------------

// list terminated with fform1 == 0 (FF_UNKNOWN)
extern const brsub_info_t brsub_info[];

const brsub_info_t * GetInfoBRSUB
(
    file_format_t	fform,		// file format
    u32			version		// sub version
);

const brsub_info_t * GetCorrectBRSUB
(
    file_format_t	fform		// file format
);

///////////////////////////////////////////////////////////////////////////////

int GetKnownSectionNumBRSUB
(
    file_format_t	fform,		// file format
    u32			version		// sub version
);

int GetGenericSectionNumBRSUB
(
    const u8		* data,		// valid data pointer
    uint		data_size,	// size of 'data'
    const endian_func_t	* endian	// NULL or endian functions
);

int GetSectionNumBRSUB
(
    const u8		* data,		// valid data pointer
    uint		data_size,	// size of 'data'
    const endian_func_t	* endian	// NULL or endian functions
);

///////////////////////////////////////////////////////////////////////////////

ccp PrintHeaderNameBRSUB
(
    char		* buf,		// result buffer
					// If NULL, a local circulary static buffer is used
    size_t		buf_size,	// size of 'buf', ignored if buf==NULL
    const brsub_header_t * bhead,	// valid brsub header
    uint		n_grp,		// number of groups
    const endian_func_t	* endian	// NULL or endian
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRRES subfile iterator		///////////////
///////////////////////////////////////////////////////////////////////////////

struct brsub_iterator_t;

typedef int (*brsub_it_raw_func)
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* data,		// data pointer
    u32			data_size	// size of 'data'
);

typedef int (*brsub_it_grp_func)
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    brres_group_t	* grp,		// pointer to group
    uint		grp_index	// index of group
);

typedef int (*brsub_it_entry_func)
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    brres_group_t	* grp,		// pointer to group
    uint		grp_index,	// index of group
    brres_entry_t	* entry,	// pointer to entry
    u8			* data		// pointer to entry related data
);

typedef int (*brsub_it_str_func)
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* base,		// pointer to base for calculation
    u32			* ptr		// pointer to u32 string offset
);

typedef int (*brsub_it_off_func)
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* base,		// pointer to base for calculation
    u32			* ptr		// pointer to u32 offset
);

//-----------------------------------------------------------------------------
// [[brsub_iterator_t]]

typedef struct brsub_iterator_t
{
    //--- global parameters

    szs_file_t		* szs;		// valid szs pointer
    brsub_header_t	* brsub;	// brsub data
    uint		data_size;	// size of 'brsub' data
    uint		file_size;	// size of 'brsub' file
    file_format_t	fform;		// format of data, or FF_UNKNOWN
    file_format_t	force_fform;	// force this and don't check brsub
    bool		adjust;		// true: adjust offsets
    bool		static_strings;	// false: find only relocable strings
					// true:  find static strings too

    brsub_it_raw_func	raw_func;	// NULL or called for each raw data block
    brsub_it_grp_func	grp_func;	// NULL or called for each group
    brsub_it_entry_func	entry_func;	// NULL or called for each entry
    brsub_it_str_func	string_func;	// NULL or called for each string
    brsub_it_off_func	offset_func;	// NULL or called for each offset value

    void		* param;	// user defined parameter
    const endian_func_t	* endian;	// endian functions

} brsub_iterator_t;

//-----------------------------------------------------------------------------

int IterateStringsBRSUB
(
    brsub_iterator_t	* it		// valid iterator data
);

//-----------------------------------------------------------------------------

int CollectStringsBRSUB
(
    string_pool_t	* sp,		// string pool
    bool		init_sp,	// true: initialize 'sp'
    szs_file_t		* szs,		// szs of data
    bool		adjust		// true: create string pool and
					//       adjust offsets and strings
);

//-----------------------------------------------------------------------------

int AdjustStringsBRSUB
(
    szs_file_t		* szs,		// base BRRES file
    void		* brsub_data,	// brres sub data
    size_t		brsub_size	// size of 'brsub_data' including string pool
);

//-----------------------------------------------------------------------------

int DumpStructureBRSUB
(
    FILE		* f,		// output file
    szs_file_t		* base_szs,	// valid base SZS
    uint		off,		// offset of selected data within 'base'
    uint		size,		// not NULL: size of selected data
    file_format_t	fform		// not FF_UNKNOWN: use this type
);

//-----------------------------------------------------------------------------

int DumpStructureBRRES
(
    FILE		* f,		// output file
    szs_file_t		* szs		// BRRES file
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct brres_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[brres_t]]

typedef struct brres_t
{
    SubDir_t		sub;		// sub filesystem
    //string_pool;
}
brres_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			TEX0 support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[tex_info_t]]

typedef struct tex_info_t
{
    //--- part of BRSUB header
    u32			name_off;	// offset of object name

    //--- group -1 of BRSUB file
    u32			unknown1;	// always 0 im MKW
    u16			width;		// width of image in pixel
    u16			height;		// height of image in pixel
    u32			iform;		// image format
    u32			n_image;	// number of images (main+mipmaps)
    u32			unknown2;	// always 0 im MKW
    float32		image_val;	// 'n_image-1' as 32 bit float
    u32			unknown4;	// always 0 im MKW
}
__attribute__ ((packed)) tex_info_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			NAME-REF support		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[name_ref_elem_t]]

typedef struct name_ref_elem_t
{
    u32			file_off;	// offset to file start
    u32			base_off;	// offset to section base
    ccp			info;		// info text, alloced
}
name_ref_elem_t;

//-----------------------------------------------------------------------------
// [[name_ref_var_t]]

typedef struct name_ref_var_t
{
    ccp			var_name;	// NULL or variable name, alloced
    u32			var_off;	// file offset of var name
    u32			min_file_off;	// minimal file offset

    uint		eused;		// number of used elements
    uint		esize;		// number of alloced elements
    name_ref_elem_t	*elist;		// list of elements, orderd by 'file_off'
}
name_ref_var_t;

//-----------------------------------------------------------------------------
// [[name_ref_t]]

typedef struct name_ref_t
{
    u8			*data;		// data
    uint		data_size;	// size of data
    file_format_t	fform;		// file format
    ccp			fname;		// file name of source, alloced

    int			brief;		// >0: suppress string offsets
					// >1: suppress reference offsets
					// >2: suppress all offsets

    uint		vused;		// number of used variables
    uint		vsize;		// number of alloced variables
    name_ref_var_t	*vlist;		// list of variables, ordered by 'var_offset'

    char		file_info[40];	// used for name_ref_elem_t::info
}
name_ref_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeNameRef ( name_ref_t *nr );
void ResetNameRef ( name_ref_t *nr );

enumError CreateNameRef
	( name_ref_t *nr, bool init_nr, szs_file_t *szs, int brief );
void ListNameRef ( FILE *f, int indent, name_ref_t *nr, SortMode_t sort );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_BRRES_H

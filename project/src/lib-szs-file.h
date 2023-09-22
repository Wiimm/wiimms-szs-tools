
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
//
// Some notes about the name 'szs':
//
//   At project beginning the szs_* data structures were implemented to
//   manage SZS files. Meanwhile they become a general data container.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_SZS_FILE_H
#define SZS_LIB_SZS_FILE_H 1

#include "lib-std.h"
#include "lib-object.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			have_szs_file_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[have_szs_file_t]]
// order is important,
// compare have_szs_name[], have_szs_file[], have_szs_fform[] and ct.wiimm.de

typedef enum have_szs_file_t
{
    HAVESZS_COURSE_LEX,		// course.lex
    HAVESZS_OBJFLOW,		// common/ObjFlow.bin
    HAVESZS_GHT_ITEM,		// common/GeoHitTableItem.bin
    HAVESZS_GHT_ITEM_OBJ,	// common/GeoHitTableItemObj.bin
    HAVESZS_GHT_KART,		// common/GeoHitTableKart.bin
    HAVESZS_GHT_KART_OBJ,	// common/GeoHitTableKartObj.bin
    HAVESZS_ITEM_SLOT_TABLE,	// ItemSlotTable/ItemSlotTable.slt
    HAVESZS_MINIGAME,		// common/minigame.kmg
    HAVESZS_AIPARAM_BAA,	// AIParam/AIParam.baa
    HAVESZS_AIPARAM_BAS,	// AIParam/AIParam.bas
    HAVESZS__N
}
__attribute__ ((packed)) have_szs_file_t;

extern const ccp have_szs_name[HAVESZS__N];
extern const ccp have_szs_file[HAVESZS__N];
extern const file_format_t have_szs_fform[HAVESZS__N];

//-----------------------------------------------------------------------------
// [[have_file_mode_t]] [[szs_special_t]]

typedef enum have_file_mode_t
{
    HFM_NONE,		// file not found/exist
    HFM_ORIGINAL,	// file found, but original data
    HFM_MODIFIED,	// file found, modified data
}
__attribute__ ((packed)) have_file_mode_t;

typedef have_file_mode_t szs_special_t[HAVESZS__N];

///////////////////////////////////////////////////////////////////////////////
// [[have_attrib_t]]
// order is important, compare have_attrib_name[] and ct.wiimm.de

typedef enum have_attrib_t
{
    HAVEATT_CHEAT,		// attribut "cheat" found
    HAVEATT_EDIT,		// attribut "edit" found
    HAVEATT_REVERSE,		// attribut "reverse" found
    HAVEATT__N
}
__attribute__ ((packed)) have_attrib_t;

extern const ccp have_attrib_name[HAVEATT__N];

ccp CreateSpecialInfoAttrib
	( have_attrib_t attrib, bool add_value, ccp return_if_empty );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_have_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[szs_have_t]]

typedef struct szs_have_t
{
    bool		valid;		// TRUE: SZS analyzed
    szs_special_t	szs;		// u8-list: one of have_file_mode_t (H_*)
    kmp_special_t	kmp;		// u8-list: elem>0: number of special KMP objects found
    uint		lex_sect;	// bit field for found lex sections
    uint		lex_feat;	// bit field for found lex features
    uint		lex_apply_otl;	// >0: applied online time limit
    have_attrib_t	attrib;		// bit field for special attributes
}
szs_have_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			string_pool_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[string_pool_t]]

typedef struct string_pool_t
{
    //--- incomming data
    ParamField_t	pf;		// collected strings

    //--- num-to-string reference; not used elements are NULL
    ccp			*n2s_list;	// reference list
    uint		n2s_size;	// alloced elements of 'n2s_list'

    //--- created string pool
    u8			* data;		// NULL or string pool
    uint		size;		// size of 'data'
    u32			* offset;	// offset for each string in 'sf'

} string_pool_t;

//-----------------------------------------------------------------------------

void InitializeStringPool ( string_pool_t * sp );
void ResetStringPool	  ( string_pool_t * sp );
u32  InsertStringPool	  ( string_pool_t * sp, ccp string, bool move_string, ccp data );
void CalcStringPool	  ( string_pool_t * sp, u32 base_offset,
				const endian_func_t * endian );
u32  FindStringPool	  ( string_pool_t * sp, ccp string, u32 not_found_value );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_subfile_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[szs_subfile_t]]

typedef struct szs_subfile_t
{
    bool		is_dir;		// true: directory entry
    bool		has_subfiles;	// true: this file has enabled sub files
    bool		removed;	// true: file marked as removed
    bool		data_alloced;	// true: 'data' is alloced
    u16			fform;		// NULL or file format
    u16			dir_id;		// unique directory id
    u32			offset;		// offset
    u32			size;		// size of file
    u8			*data;		// not NULL: data of 'size' bytes
    ccp			path;		// alloced path name
    ccp			load_path;	// not null: load file from here (alloced)
    struct
	szs_subfile_t	*ext;		// related external data

    dev_t		device;		// link support: device number
    ino_t		inode;		// link support: inode number
    uint		link_index;	// >0: is linked to others with same index

    ccp			name;		// pointer to name of object
    int			brsub_version;	// 0 or version of parent brsub file
    s16			group;		// group index, type grp_entry_t
    s16			entry;		// entry index, type grp_entry_t

} szs_subfile_t;

///////////////////////////////////////////////////////////////////////////////
// [[szs_subfile_list_t]]

typedef struct szs_subfile_list_t
{
    szs_subfile_t	* list;		// alloced list of sub files
    uint		used;		// number of used elements in 'list'
    uint		size;		// number of allocated elements in 'list'
    SortMode_t		sort_mode;	// current sort mode
    uint		link_count;	// last used link index

} szs_subfile_list_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeSubfileList ( szs_subfile_list_t * sl );
void ResetSubfileList ( szs_subfile_list_t * sl );
void PurgeSubfileList ( szs_subfile_list_t * sl );

szs_subfile_t * InsertSubFileList
(
    szs_subfile_list_t	* sl,		// valid subfile list
    uint		insert_pos,	// insert position index, robust
    ccp			path,		// NULL of path name
    bool		move_path	// ignored if 'path' is NULL
					// false: make a copy of path
					// true:  use path and free it on reset
);

static inline szs_subfile_t * AppendSubFileList
(
    szs_subfile_list_t	* sl,		// valid subfile list
    ccp			path,		// NULL of path name
    bool		move_path	// ignored if 'path' is NULL
					// false: make a copy of path
					// true:  use path and free it on reset
)
{
    return InsertSubFileList(sl,~0,path,move_path);
}

szs_subfile_t * FindSubFileSZS
(
    szs_subfile_list_t	* sl,		// valid subfile list
    ccp			path,		// path name to search
    uint		start_index	// start seearch at this index
);

bool SortSubFiles
(
    // returns true if new sorting done

    szs_subfile_list_t	* sl,		// valid subfile list
    SortMode_t		sort_mode,	// wanted sort mode
    file_format_t	fform,		// use it if sort_mode==SORT_AUTO
    FormatField_t	* order_list	// NULL or a priority sort list
);

///////////////////////////////////////////////////////////////////////////////

typedef int (*CmpFuncSubFile) ( const szs_subfile_t * a, const szs_subfile_t * b );

CmpFuncSubFile GetCmpFunc
(
    SortMode_t		sort_mode,	// wanted sort mode
    file_format_t	fform		// use it if 'sort_mode==SORT_AUTO'
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_file_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[szs_file_t]]

struct brsub_list_t;

typedef struct szs_file_t
{
    //--- info

    ccp			fname;		// path of read file, never NULL
    ccp			dest_fname;	// not NULL: planned destination file name
    ccp			cache_fname;	// not NULL: link to the cache on SaveSZS()
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform_file;	// source file format
    file_format_t	fform_arch;	// (decompressed) archive format
    file_format_t	fform_current;	// current format of 'data'
    ff_attrib_t		ff_attrib;	// attributes of 'fform_arch'
    int			ff_version;	// -1 or version of 'fform_arch'
    const endian_func_t	* endian;	// endian functions
    bool		links;		// true: support of hardlinks enabled
    bool		readonly;	// true: file was opened for readonnly purpose
    bool		cache_used;	// true: compression by cache copy
    bool		check_only;	// true: analysing or checking the file


    //--- parent

    struct szs_file_t	* parent;	// NULL or pointer to parent SZS
    uint		parent_off;	// offset to parent data


    //--- data sections

    u8			* cdata;	// compressed data
    u8			* data;		// plain data

    size_t		csize;		// size of 'cdata'
    size_t		size;		// size of 'data'
    size_t		file_size;	// size of container file

    bool		cdata_alloced;	// true: 'cdata' must be freed
    bool		data_alloced;	// true: 'data' must be freed

    u32			min_data_off;	// minimum found data offset
    u32			max_data_off;	// maximum found data offset
					// the string pool does not start before

    ccp			obj_name;	// NULL or name of object
    u32			obj_id;		// id of object
    bool		obj_name_alloced; // true: name must be freed
    bool		obj_id_used;	// false: 'obj_id' not used or set

    DataContainer_t	*old_container;	// old data [[container]] support, init with NULL
    Container_t		container;	// container data

    //--- statistics, determined by CollectFilesSZS()( and collect_r_func()

    uint		n_dirs;		// number of directories in archive
    uint		n_files;	// number of files in archive
    uint		fw_dirs;	// max UTF8 length of all dirs
    uint		fw_files;	// max UTF8 length of all files

    //--- statistics, determined by LoadObjFileListSZS()

    UsedFileFILE_t	*used_file;	// NULL or alloced and set vector
    DbFlags_t		found_flags;	// summary (or'ed) of 'used_file'
    MissedFile_t	missed_file;	// missed files by DBT_* groups
    MissedFile_t	modified_file;	// modified files by DBT_* groups
    ParamField_t	*required_file;	// list with required files by setting
    int			n_cannon;	// number of defined cannons, -1: unknown
    IsArena_t		is_arena;	// one of ARENA_*

    bool		slot_analyzed;	// true: slot values are valid
    uint		slot_31_xx_71;	// slot required because of objects
    uint		slot_42;	// slot possible because of objects
    uint		slot_62;	// slot required because of objects


    //--- patching support

    patch_file_t	brres_type;	// type of last detected BRRES file
    bool		dont_patch_mdl;	// true: Don't MDL patch MDL files
					// of BRRES, because `disabled´
    bool		aiparam_removed;// true: AIParam removed
    bool		allow_ext_data;	// allow the following external data
    szs_subfile_list_t	ext_data;	// list of sub files with external data


    //--- composing support

    szs_subfile_list_t	subfile;	// list of sub files
    struct brsub_list_t	* brsub_list;	// list of brres sub objects
    string_pool_t	string_pool;	// string pool
    FormatField_t	* order_list;	// not NULL: Use list for primary order


    //--- specific subfiles, set by FindSpecialFilesSZS()

    bool	special_done;		// true: scanning for special files already done

    u8*		course_kcl_data;	// not NULL: pointer and size
    uint	course_kcl_size;	//           of file 'course.kcl'
    u8*		course_kmp_data;	// not NULL: pointer and size
    uint	course_kmp_size;	//           of file 'course.kmp'
    u8*		course_lex_data;	// not NULL: pointer and size
    uint	course_lex_size;	//           of file 'course.lex'
    u8*		course_model_data;	// not NULL: pointer and size
    uint	course_model_size;	//           of file 'course_model.brres'
    u8*		course_d_model_data;	// not NULL: pointer and size
    uint	course_d_model_size;	//           of file 'course_d_model.brres'
    u8*		vrcorn_model_data;	// not NULL: pointer and size
    uint	vrcorn_model_size;	//           of file 'vrcorn_model.brres'
    u8*		map_model_data;		// not NULL: pointer and size
    uint	map_model_size;		//           of file 'map_model.brres'

    bool	have_ice_brres;		// true: file 'ice.brres' found
    u8		moonview_mdl_stat;	// 0: none found
					// 1: some materials found
					// 2: all materials found
					// 3: all materials found & content ok

    warn_bits_t	warn_bits;		// warning summary
    szs_have_t	have;			// complete have status

    ParamField_t *special_file;		// list with files of directory /common/


    //--- options set by PrepareCheckTextureSZS()

    bool	check_enpt;		// true: check ENPT too

} szs_file_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_SZS_FILE_H


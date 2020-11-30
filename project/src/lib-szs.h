
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
//
// Some notes about the name 'szs':
//
//   At project beginning the szs_* data structures were implemented to
//   manage SZS files. Meanwhile they become a general data container.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_SZS_H
#define SZS_LIB_SZS_H 1

#include "lib-std.h"
#include "lib-object.h"
#include "lib-lecode.h"
#include "lib-checksum.h"
#include "lib-kmp.h"
#include "db-mkw.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			yaz0_header_t			///////////////
///////////////////////////////////////////////////////////////////////////////

#define YAZ0_MAGIC	"Yaz0"
#define YAZ1_MAGIC	"Yaz1"
#define XYZ0_MAGIC	"xYz0"

#define YAZ0_MAGIC_NUM	0x59617a30
#define YAZ1_MAGIC_NUM	0x59617a31
#define XYZ0_MAGIC_NUM	0x78597a30

typedef struct yaz0_header_t
{
    char	magic[4];		// = YAZ*_MAGIC
    be32_t	uncompressed_size;	// total size of uncompressed data
    char	padding[8];		// always 0?
}
__attribute__ ((packed)) yaz0_header_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			u8 support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define WU8_MAGIC_NUM	  0x57553861
#define U8_MAGIC_NUM	  0x55AA382D
#define U8_DEFAULT_ALIGN  0x20

typedef struct u8_header_t
{
    be32_t	magic;		// = U8_MAGIC_NUM
    be32_t	node_offset;	// offset of first node
    be32_t	fst_size;	// size of all nodes including the string table.
    be32_t	data_offset;	// file offset of data
    char	padding[0];	// filled with 0xcc until the first node
}
__attribute__ ((packed)) u8_header_t;

//-----------------------------------------------------------------------------

typedef struct u8_node_t
{
    union
    {
	u8	is_dir;		// directory flag
	be32_t	name_off;	// mask with 0x00ffffff
    };

    be32_t	offset;
    be32_t	size;
}
__attribute__ ((packed)) u8_node_t;

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
///////////////			enum mark			///////////////
///////////////////////////////////////////////////////////////////////////////

// Flags:
//  _U : object used
//  _PF : object active with full presence flag

enum szs_marker_t
{
  SLOT_31_71_SUNDS	= 0x01,  // bit for 'szs_file_t:slot_31_71'
  SLOT_31_71_PYLON01	= 0x02,  // bit for 'szs_file_t:slot_31_71'

  SLOT_42_KART_TRUCK_U	= 0x01,  // bit for 'szs_file_t:slot_42'
  SLOT_42_CAR_BODY_U	= 0x02,  // bit for 'szs_file_t:slot_42'
  SLOT_42_ALL_U		= 0x04,  // bit for 'szs_file_t:slot_42'
  SLOT_42_KART_TRUCK_PF	= 0x10,  // bit for 'szs_file_t:slot_42'
  SLOT_42_CAR_BODY_PF	= 0x20,  // bit for 'szs_file_t:slot_42'
  SLOT_42_ALL_PF	= 0x40,  // bit for 'szs_file_t:slot_42'

  SLOT_62_HEYHOSHIP	= 0x01,  // bit for 'szs_file_t:slot_62'
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			have_szs_file_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[have_szs_file_t]]
// order is important, compare have_szs_name[], have_szs_file[] and ct.wiimm.de

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
    HAVESZS__N
}
__attribute__ ((packed)) have_szs_file_t;

extern const ccp have_szs_name[HAVESZS__N];
extern const ccp have_szs_file[HAVESZS__N];

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


    //--- statistics, determined by LoadObjFileListSZS()

    UsedFileFILE_t	*used_file;	// NULL or alloced and set vector
    ParamField_t	*required_file;	// list with required files by setting
    int			n_cannon;	// number of defined cannons, -1: unknown
    IsArena_t		is_arena;	// one of ARENA_*

    bool		slot_analyzed;	// true: slot values are valid
    uint		slot_31_71;	// slot required because of objects
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
    bool	szs_special[HAVESZS__N]; // true: special file found
    kmp_special_t kmp_special;		// list: >0: number of special KMP objects found
    uint	have_lex;		// bit field for found lex eleemnts

    ParamField_t *special_file;		// list with files of directory /common/

} szs_file_t;

//-----------------------------------------------------------------------------
// [[szs_norm_t]]

typedef struct szs_norm_t
{
    u32		namepool_size;
    u32		total_size;
    bool	have_pt_dir;
    bool	rm_aiparam;
    bool	clean_lex;

} szs_norm_t;

//-----------------------------------------------------------------------------

void SetupStandardSZS();
void SetupExtendedSZS();

void InitializeSZS ( szs_file_t * szs );
void ResetSZS ( szs_file_t * szs );
void ResetFileSZS ( szs_file_t * szs, bool remove_list );

szs_subfile_t * FindLinkSZS
	( szs_file_t *szs, dev_t device, ino_t inode, const szs_subfile_t *exclude );

// [[container+]]
DataContainer_t * ContainerSZS ( szs_file_t * szs );
Container_t     * SetupContainerSZS ( szs_file_t * szs );
ContainerData_t * LinkContainerSZS  ( szs_file_t * szs );
ContainerData_t * MoveContainerSZS  ( szs_file_t * szs );
void              ClearContainerSZS ( szs_file_t * szs );

void InitializeSubSZS
(
    szs_file_t		*szs,		// SZS to initialize
    szs_file_t		*base,		// valid base SZS
    uint		off,		// offset of selected data within 'base'
    uint		size,		// not NULL: size of selected data
    file_format_t	fform,		// not FF_UNKNOWN: use this type
    ccp			fname,		// NULL or file names
    bool		decompress	// true: decompress if possible
);

void AssignSZS
(
    szs_file_t		*szs,		// SZS to initialize
    bool		init_szs,	// true: InitializeSZS(), false: ResetSZS()
    void		*data,		// data to assign
    uint		size,		// size of 'data'
    bool		move_data,	// true: free 'data' on reset
    file_format_t	fform,		// not FF_UNKNOWN: use this type
    ccp			fname		// NULL or file names
);

enumError LoadSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// valid pointer to filenname
    bool		decompress,	// decompress after loading
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    bool		mark_readonly	// true: the file will never written
);

enumError SaveSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// valid pointer to filenname
    bool		overwrite,	// true: overwrite data
    bool		compress	// false: store decompressed
					// true:  store compressed

);

void LinkCacheRAW
(
    // link/copy only files with data!

    ccp			cache_fname,	// NULL or path to cache file
    ccp			src_fname,	// source filename
    cvp			data,		// data of link() failed
    uint		size		// size of data
);

static inline void LinkCacheSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			src_fname	// source filename
)
{
    DASSERT(szs);
    LinkCacheRAW(szs->cache_fname,src_fname,szs->cdata,szs->csize);
}

//-----------------------------------------------------------------------------

void MarkReadonlySZS ( szs_file_t * szs );
enumError CompressSZS ( szs_file_t * szs, bool remove_uncompressed );
bool NormalizeSZS ( szs_file_t *szs );
bool NormalizeExSZS ( szs_file_t *szs, bool rm_aiparam, bool clean_lex, bool autoadd );
bool PatchSZS ( szs_file_t * szs );

enumError DecompressSZS
(
    szs_file_t	*szs,		// valid SZS source, use cdata
    bool	rm_compressed,	// true: remove compressed data
    FILE	*hexdump	// not NULL: write decrompression hex-dump
);

bool TryDecompressSZS
(
    szs_file_t	*szs,		// valid SZS source, use data if not NULL
    bool	rm_compressed	// true: remove compressed data
);

void ClearCompressedSZS ( szs_file_t * szs );
void ClearUncompressedSZS ( szs_file_t * szs );

enumError DecodeWU8 ( szs_file_t * szs );
enumError EncodeWU8 ( szs_file_t * szs );

u8 * DecodeXYZ
(
    u8		*dest,		// destination buffer,
				// If NULL: malloc() is used.
    const u8	*src,		// source data
    uint	size		// size of dest and source

);

u8 * EncodeXYZ
(
    u8		*dest,		// destination buffer,
				// If NULL: malloc() is used.
    const u8	*src,		// source data
    uint	size		// size of dest and source

);

size_t GetDecompressedSizeYAZ
(
    const void		* data,		// source data
    size_t		data_size	// size of 'data'
);

enumError DecompressYAZ
(
    // returns:
    //	    ERR_OK:           compression done
    //	    ERR_WARNING:      silent==true: dest buffer too small
    //	    ERR_INVALID_DATA: invalid source data

    const void		* data,		// source data
    size_t		data_size,	// size of 'data'
    void		* dest_buf,	// destination buffer (decompressed data)
    size_t		dest_buf_size,	// size of 'dest_buf'
    size_t		*write_status,	// number of written bytes
    ccp			fname,		// file name for error messages
    int			yaz_version,	// yaz version for error messages (0|1)
    bool		silent,		// true: don't print error messages
    FILE		*hexdump	// not NULL: write decrompression dump
);

bool IsFileOptional ( szs_file_t *szs, const DbFileFILE_t *file );

struct szs_norm_t;
int AddMissingFileSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// filename to add
    file_format_t	fform,		// valid file format
    struct szs_norm_t	*norm,		// NULL or norm struct
    int			log_indent	// >-1: print log with indention
);

struct scan_data_t;
struct szs_norm_t;
enumError AddMissingFiles
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to base directory
    struct scan_data_t	* sd,		// NULL or valid scan data
    struct szs_norm_t	* norm,		// NULL or valid norm data
    int			log_indent	// >-1: print log with indention
);

enumError CreateU8
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			namepool_size,	// total namepool_size
    u32			total_size,	// total file size
    bool		create_pt_dir	// create directory '.' as base
);

enumError CreateSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			dest_fname,	// NULL or planned destination file name
    ccp			source_dir,	// path to base directory
    SubDir_t		* sdir,		// not NULL: read from here and ignore source_dir
    SetupParam_t	* setup_param,	// setup parameters
					// if NULL: read SZS_SETUP_FILE
    uint		depth,		// creation depth
    uint		log_depth,	// print creation log if depth<log_depth
    bool		mark_readonly	// true: the file will never written
);

enumError LoadCreateSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// valid pointer to filenname
    bool		decompress,	// decompress after loading
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    bool		mark_readonly	// true: the file will never written
);

enumError LoadObjFileListSZS
(
    szs_file_t		* szs,		// valid szs
    const void		* kmp_data,	// not NULL: source data
    size_t		data_size,	// size of 'kmp_data'
    ccp			kmp_fname,	// if !kmp_data: load kmp file
    bool		ignore_no_file,	// param for LoadKMP()/ScanKMP()
    CheckMode_t		check_mode,	// param for LoadKMP()/ScanKMP()
    lex_info_t		*lexinfo	// NULL or valid LEX info
);

ParamFieldItem_t * IsFileRequiredSZS
(
    // check DBF_SPECIAL && szs->required_file
    szs_file_t		* szs,		// valid szs
    const DbFileFILE_t	* file		// file to proof
);

bool IsFileOptionalSZS
(
    // check DBF_OPTIONAL && DBF_SPECIAL && szs->required_file
    szs_file_t		* szs,		// valid szs
    const DbFileFILE_t	* file		// NULL or file to proof
);

enumError DiffSZS
(
    szs_file_t	* szs1,		// first szs to compare
    szs_file_t	* szs2,		// second szs tp compare
    int		recurse,	// 0:off, <0:unlimited, >0:max depth
    int		cut_files,	// <0:never, =0:auto, >0:always
    bool	quiet		// true: be quiet
);

bool SortSubFilesSZS
(
    szs_file_t		* szs,		// valid szs structure
    SortMode_t		sort_mode	// wanted sort mode
);

typedef struct check_szs_t check_szs_t;

int CheckSZS
(
    // returns number of errors

    szs_file_t		* szs,		// valid szs
    CheckMode_t		szs_mode,	// print mode for szs file
    CheckMode_t		sub_mode	// print mode for sub file (KCL,KMP,...)
);

int CheckBRRES
(
    // returns number of errors

    szs_file_t		* szs,		// valid szs
    CheckMode_t		check_mode,	// print mode
    check_szs_t		* parent_chk	// NULL or parent
);

//-----------------------------------------------------------------------------

static inline ccp GetNameFF_SZS ( const szs_file_t *szs )
{
    DASSERT(szs);
    return GetNameFFv(szs->fform_file,szs->fform_arch,szs->ff_version);
}

static inline ccp GetNameFF_SZScurrent ( const szs_file_t *szs )
{
    DASSERT(szs);
    return GetNameFF(szs->fform_file,szs->fform_current);
}

//-----------------------------------------------------------------------------
// [[slot_ana_t]]

typedef struct slot_ana_t
{
    int  stat;			// result of FindSlotsSZS() : -1, 0 or +1
    int  slot[MKW_N_TRACKS];	// set by FindSlotsSZS()
    int  required_slot;		// set by FindSlotsSZS()

    char mandatory_slot[6];	// empty | "arena" | "31+71" | "61" | "62"
    char slot_info[30];		// slot info text
}
slot_ana_t;

//-----------------------------------------------------------------------------

int FindSlotsSZS
(
    // returns -1, 0 or +1

    szs_file_t		* szs,		// valid szs
    int			slot[MKW_N_TRACKS],	// status vector:
					//	-1: slot not possible
					//	 0: unknown
					//	+1: slot possible
    int			*required_slot	// if not NULL: return 0|31|61|62
					// 31 is an alias for "31+71"
);

void AnalyzeSlot ( slot_ana_t *sa, szs_file_t *szs );

// return temporary string by GetCircBuf()
ccp CreateSlotInfo ( szs_file_t *szs );

//-----------------------------------------------------------------------------

void ClearSpecialFilesSZS
(
    szs_file_t		* szs		// valid szs
);

void FindSpecialFilesSZS
(
    szs_file_t		* szs,		// valid szs
    bool		force		// true: force new scanning
);

ccp CreateSpecialFileInfo
	( szs_file_t * szs, bool add_value, ccp return_if_empty );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs file iterator		///////////////
///////////////////////////////////////////////////////////////////////////////

struct szs_iterator_t;
typedef int (*szs_iterator_func)
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

//-----------------------------------------------------------------------------
// [[iterator_param_t]]

typedef struct iterator_param_t
{
    bool		clean_path;	// true: clean path
    bool		show_root_node;	// true: show root node
    bool		cut_files;	// true: cut files into peaces
    SortMode_t		sort_mode;	// sort files
}
iterator_param_t;

#define USE_ITERATOR_PARAM 1

//-----------------------------------------------------------------------------
// [[szs_iterator_t]]

typedef struct szs_iterator_t
{
    //--- global parameters

    szs_file_t		* szs;		// valid szs pointer
    const endian_func_t	* endian;	// endian functions
 #if USE_ITERATOR_PARAM
    iterator_param_t	itpar;		// global parameters
 #else
    bool		clean_path;	// true: clean path
    bool		show_root_node;	// true: show root node
    bool		cut_files;	// true: cut files into peaces
    SortMode_t		sort_mode;	// sort files
 #endif

    //--- client values

    void		* param;	// user defined parameter
    int			client_int;	// initial set to null

    //--- call back functions

    szs_iterator_func	func_sub;	// called by sub files functions
    szs_iterator_func	func_sort;	// called by sort functions
    szs_iterator_func	func_it;	// called by iterator functions

    //--- recursive handling

    struct
	szs_iterator_t	* parent_it;	// NULL or parent iterator
    int			recurse_level;	// current recursive level
    int			recurse_max;	// max recursive level
    int			no_recurse;	// >0: disable recurse handling

    //--- file specific parameters

    bool		is_dir;		// true: directory argument
    bool		has_subfiles;	// true: this file has enabled sub files
    u16			fform;		// NULL or file format
    int			index;		// index of 'fst_item'
    int			depth;		// directory depth
    u32			off;		// offset of object
    u32			size;		// size of object
    const u8_node_t	* fst_item;	// pointer to FST item

    //--- filename

    char		path[ARCH_FILE_MAX]; // full path
    char		* trail_path;	// trailing part of path
    ccp			name;		// raw name of the current object

    //--- brres iteration data

    const u8		* root_end;	// pointer to end of root

    //--- brsub support

    int			brsub_version;	// 0 or version of parent brsub file
    s16			group;		// group index, type grp_entry_t
    s16			entry;		// entry index, type grp_entry_t
}
szs_iterator_t;

//-----------------------------------------------------------------------------

szs_iterator_func GetIteratorFunction
(
    file_format_t	fform,		// file format format
    bool		cut_files	// true: support of cutted files
);

typedef szs_iterator_func (*get_iterator_func)(file_format_t,bool);
extern szs_iterator_func std_iter_func[FF_N];
extern szs_iterator_func cut_iter_func[FF_N];

//-----------------------------------------------------------------------------

int IterateFilesSZS
(
    szs_file_t		* szs,		// valid szs
    szs_iterator_func	func,		// call back function
    void		* param,	// user defined parameter
    const iterator_param_t
			*p_itpar,	// NULL or iteration parameters
    int			recurse		// 0:off, <0:unlimited, >0:max depth
);

int IterateFilesParSZS
(
    szs_file_t		* szs,		// valid szs
    szs_iterator_func	func,		// call back function
    void		* param,	// user defined parameter
    bool		clean_path,	// true: clean path from ../ and more
    bool		show_root_node,	// true: include root node in iteration
    int			recurse,	// 0:off, <0:unlimited, >0:max depth
    int			cut_files,	// <0:never, =0:auto(first level), >0:always
    SortMode_t		sort_mode	// sort mode
);

uint CollectFilesSZS
(
    szs_file_t		* szs,		// valid szs
    bool		clear,		// true: clear list before adding
    int			recurse,	// 0:off, <0:unlimited, >0:max depth
    int			cut_files,	// <0:never, =0:auto, >0:always
    SortMode_t		sort_mode	// sort subfiles
);

void PrintFileHeadSZS();
int PrintFileSZS
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

int IterateFilesU8
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

void PrintVersionSZS ( char *buf, uint buf_size, struct szs_iterator_t *it );

//-----------------------------------------------------------------------------

int IterateFilesData
(
    const void		*data,		// valid data pointer
    uint		data_size,	// size of data
    szs_iterator_func	func,		// call back function
    void		* param,	// user defined parameter
    file_format_t	fform,		// not FF_UNKNOWN: use this type
    ccp			fname		// NULL or filename
);

//-----------------------------------------------------------------------------

szs_subfile_t * InsertSubfileSZS
(
    szs_file_t		*szs,		// valid data structure
    uint		insert_pos,	// insert position index, robust
    szs_iterator_t	*it,		// not NULL: copy source info
    ccp			path		// not NULL: copy to member 'path'
);

static inline szs_subfile_t * AppendSubfileSZS
(
    szs_file_t		*szs,	// valid data structure
    szs_iterator_t	*it,	// not NULL: copy source info
    ccp			path	// not NULL: copy to member 'path'
)
{
    return InsertSubfileSZS(szs,~0,it,path);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRRES structures		///////////////
///////////////////////////////////////////////////////////////////////////////

#define BRRES_MAGIC		"bres"
#define BRRES_MAGIC_NUM		0x62726573
#define BRRES_DEFAULT_ALIGN	0x04
#define BRRES_STRING_POOL_FNAME	".string-pool.bin"

//-----------------------------------------------------------------------------
// [[brres_header_t]]

// the data is 'bom' dependent big or little endian

typedef struct brres_header_t
{
    char		magic[4];	// = BRRES_MAGIC
    u16			bom;		// byte order mark
    u16			version;	// usally BRESS=0 / BREFF+BREFT=9(MKW)|11(NSMB)
    u32			size;		// file size
    u16			root_off;	// offset of root section
    u16			n_sections;	// number of sections
}
__attribute__ ((packed)) brres_header_t;

//-----------------------------------------------------------------------------
// [[brres_entry_t]]

typedef struct brres_entry_t
{
    u16			id;		// entry id
    u16			unknown;	// always 0?
    u16			left_idx;	// index to left element in virtual tree
    u16			right_idx;	// index to right element in virtual tree
    u32			name_off;	// offset into string table
    u32			data_off;	// offset to data
}
__attribute__ ((packed)) brres_entry_t;

//-----------------------------------------------------------------------------
// [[brres_group_t]]

typedef struct brres_group_t
{
    u32			size;		// size of complete group
    u32			n_entries;	// number of entires
    brres_entry_t	entry[0];	// list of entries, N := 1 + n_entries
					// entry with index #0 is a special
					// root node not counting in 'n_entries'
}
__attribute__ ((packed)) brres_group_t;

//-----------------------------------------------------------------------------
// [[brres_root_t]]

#define BRRES_ROOT_MAGIC "root"

typedef struct brres_root_t
{
    char		magic[4];	// = BRRES_ROOT_MAGIC
    u32			size;		// size of complete header
    //brres_group_t	group[0];	// first group -> disabled because MVC
}
__attribute__ ((packed)) brres_root_t;

#define BRRES_ROOT_GROUP(r) ((brres_group_t*)((brres_root_t*)r+1))

//-----------------------------------------------------------------------------
// [[brsub_header_t]]

typedef struct brsub_header_t
{
 /* 0x00 */  char	magic[4];	// magic of sub file
 /* 0x04 */  u32	size;		// size of sub file
 /* 0x08 */  u32	version;	// version of sub file
 /* 0x0c */  u32	brres_offset;	// negative value to begin of brres
 /* 0x10 */  u32	grp_offset[0];	// offsets of other sections -> brres_group_t
					// this field is limited until the start
					// of the first group or until the 'n_sect'
					// non zero entry.
 // 0xNN     u32	name_off;	// offset to name of object

    // There is more data dependent of the sub file type. This data starts
    // behind 'grp_offset' and ends at the first brres_group_t data.
}
__attribute__ ((packed)) brsub_header_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		      cut files: BRSUB			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[brsub_cut_t]]

typedef struct brsub_cut_t
{
    //--- input data

    szs_iterator_t	*it;		// iterator
    szs_file_t		*szs;		// extracted from 'it'
    const u8		*data;		// extracted from 'szs'
    uint		size;		// size of 'data', extracted from 'szs'
    uint		file_size;	// size of containing file
    file_format_t	fform;		// subfile format
    int			version;	// subfile version
    ff_attrib_t		fattrib;	// calculated
    const endian_func_t	*endian;	// endian functions
    void		*param;		// user parameter


    //--- base data

    char		id[5];		// ID, created from magic
    const brsub_header_t *bh;		// brsub header
    uint		brsub_size;	// fixed size of brsub data
    uint		n_grp;		// total number of groups


    //--- group + entry iterator

    brres_group_t	*gptr;		// NULL or pointer to current group
    brres_entry_t	*eptr;		// NULL or pointer to current entry
    char		*path_end;	// pointer to end of string of 'it->path'


    //--- group iterator, private data

    int			found_grp;	// index of found group
    uint		found_off;	// offset of found group

} brsub_cut_t;

//-----------------------------------------------------------------------------
// [[grp_entry_t]]

typedef enum grp_entry_t
{
    GROUP_INVALID	= -99,
    ENTRY_INVALID	= GROUP_INVALID,

    GROUP_IDX_BRSUB_HEADER = -9,
    GROUP_IDX_SUB_HEADER,
    GROUP_IDX_STRING_POOL,

    ENTRY_IDX_GROUP,
    ENTRY_IDX_DATA,
    ENTRY_IDX_ETC,

} grp_entry_t;

//-----------------------------------------------------------------------------
// [[brsub_cut_func_t]]

typedef int (*brsub_cut_func_t)
(
    brsub_cut_t		*bcut,		// pointer to data structure
    int			grp,		// index of group, <0: grp_entry_t
    int			entry		// index of entry, -1: no entry
);

//-----------------------------------------------------------------------------

int CutFilesBRSUB
(
    szs_iterator_t	*it,		// iterator struct with all infos
    brsub_cut_func_t	func,		// call back function
    void		*param,		// user parameter
    ccp			sname_tab[],	// NULL or section name table
    uint		n_sname		// number of elements in 'sname_tab'
					// if 0: 'sname_tab' is terminated by NULL
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_extract_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[szs_extract_level_t]]

typedef enum szs_extract_level_t
{
    //--- flags

    EXLEV_F_RMPT	= 0x01,		// point "./" removed
    EXLEV_F_ICASE	= 0x02,		// case ignored
    EXLEV_F_MATCH	= 0x04,		// match a file

    //--- summary

    EXLEV_EXACT		= 0x00,
    EXLEV_MASK		= 0x07,
    EXLEV_N		= EXLEV_MASK + 1,

} szs_extract_level_t;

//-----------------------------------------------------------------------------
// [[szs_extract_t]]

typedef struct szs_extract_t
{
    //--- status

    bool		subfile_found;	// true: subfile found in path
    bool		szs_found;	// true: base file is valid SZS

    //--- extract level support

    szs_extract_level_t	exlevel;	// best level found until now
    ccp			found_path;	// found sub path
    u32			found_offset;	// found offset
    u32			found_size;	// found size
    uint		found_count;	// number of found files of 'exlevel'

    //--- helpers

    ccp			subpath;	// sub file path
    uint		subpath_len;	// length of subpath

    //--- szs info

    ccp			fname;		// path of read file, never NULL
    file_format_t	fform_file;	// source file format
    file_format_t	fform_arch;	// decompressed file format
    file_format_t	fform_current;	// current format of 'data'
    const endian_func_t	* endian;	// endian functions

    //--- extracted data

    u8			* data;		// NULL or extracted data
    u32			data_size;	// size of 'data'
    bool		data_alloced;	// true: 'data' must be freed

    DataContainer_t	*old_container;	// old data [[container]] support, init with NULL
    Container_t		container;	// container data

} szs_extract_t;

//-----------------------------------------------------------------------------

void InitializeExtractSZS ( szs_extract_t * eszs );
void ResetExtractSZS ( szs_extract_t * eszs );

// [[container+]]
DataContainer_t * GetOldContainerESZS ( szs_extract_t * eszs );
Container_t     * SetupContainerESZS  ( szs_extract_t * eszs );
ContainerData_t * LinkContainerESZS   ( szs_extract_t * eszs );
ContainerData_t * MoveContainerESZS   ( szs_extract_t * eszs );

enumError ExtractSZS
(
    szs_extract_t	* eszs,		// valid data structure
    bool		initialize,	// true: initialize 'eszs'
    ccp			fname,		// filename of source
    ccp			autoname,	// not NULL: Use this filename
					//           if subfile name is "" or "/"
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

enumError PrintErrorExtractSZS
(
    szs_extract_t	* eszs,		// valid data structure
    ccp			fname		// filename of source
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExtractFilesSZS
(
    szs_file_t	*szs,		// valid szs file
    int		recurse_level,	// recursion level
    bool	is_cutting,	// true: is cutting a file
    SubDir_t	*sdir,		// not NULL: store files here
    ccp		basedir		// not NULL: Extract only files if "/BASEDIR" or "./BASEDIR".
				// BASEDIR is like "a/b/c/" without leading slash
				// If set, don't create support files.
);

ccp GetOptBasedir();

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    raw_data_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[raw_data_t]]

typedef struct raw_data_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform;		// file format of 'data'
    file_format_t	fform_file;	// file format of file
    bool		is_0;		// an empty file detected, fname is "0"

    u8			*data;		// raw data
    bool		data_alloced;	// true: 'data' is alloced
    uint		data_size;	// size of 'data'

    DataContainer_t	*old_container;	// old data [[container]] support, init with NULL
    Container_t		container;	// container data

} raw_data_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeRawData ( raw_data_t * raw );
void ResetRawData ( raw_data_t * raw );
// [[container+]]
DataContainer_t * ContainerRawData ( raw_data_t * raw );
Container_t     * SetupContainerRawData ( raw_data_t * raw );
ContainerData_t * LinkContainerRawData  ( raw_data_t * raw );
ContainerData_t * MoveContainerRawData  ( raw_data_t * raw );

enumError LoadRawData
(
    raw_data_t		* raw,		// data structure
    bool		init_raw,	// true: initialize 'raw' first
    ccp			fname,		// valid pointer to filenname
    ccp			autoname,	// not NULL: Use this filename
					//           if subfile name is "" or "/"
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    file_format_t	allow_0		// not NULL (FF_UNKNOWN):
					//	allow "0" for an empty file and set FF
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  BZ support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define BZ_MAGIC	"WBZa"
#define BZ_MAGIC_NUM	0x57425a61

typedef struct wbz_header_t
{
    char	magic[4];		// = BZ_MAGIC
    char	first_8[8];		// first 8 bytes of uncompressed sub file
					// (fast magic detection)
    u8		bz_data[];		// compressed data
}
__attribute__ ((packed)) wbz_header_t;

//-----------------------------------------------------------------------------

enumError DecompressBZ ( szs_file_t * szs, bool rm_compressed );
enumError CompressBZ ( szs_file_t * szs, bool remove_uncompressed );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRASD support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define BRASD_MAGIC		"RASD"
#define BRASD_MAGIC_NUM		0x52415344

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyse_szs_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[analyse_szs_t]]

typedef struct analyse_szs_t
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


    //--- sub files

    lex_info_t		lexinfo;	// LEX info
    slot_ana_t		slotana;	// slot data
    slot_info_t		slotinfo;	// slot data
    kmp_finish_t	kmp_finish;	// finish line
    kmp_usedpos_t	used_pos;	// used positions


    //--- more stats

    int		ckpt0_count;		// number of LC in CKPT, -1 unknown
    int		lap_count;		// STGI lap counter
    u16		speed_mod;		// STGI speed mod
    float	speed_factor;		// STGI speed factor

    char	gobj_info[20];		// gobj counters
    char	ct_attrib[300];		// collected ct attributes

    u_usec_t	duration_usec;		// duration of AnalyseSZS() in usec
}
analyse_szs_t;

//-----------------------------------------------------------------------------

void InitializeAnalyseSZS ( analyse_szs_t * as );
void ResetAnalyseSZS ( analyse_szs_t * as );

void AnalyseSZS
(
    analyse_szs_t	*as,		// result
    bool		init_sa,	// true: init 'as', false: reset 'as'
    szs_file_t		*szs,		// SZS filre t analysze
    ccp			fname		// NULL or fname for slot analysis
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_SZS_H

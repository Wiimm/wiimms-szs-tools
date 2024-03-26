
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

// all about SZS, YAZ0 and U8 files:
//	https://wiki.tockdom.de/index.php?title=SZS
//	https://wiki.tockdom.de/index.php?title=YAZ0
//	https://wiki.tockdom.de/index.php?title=U8

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

//#include <stddef.h>

#include "lib-std.h"
#include "lib-szs-file.h"
#include "lib-object.h"
#include "lib-lecode.h"
#include "lib-checksum.h"
#include "lib-kmp.h"
#include "db-mkw.h"
#include "dclib-ui.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			yaz0_header_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[yaz0_header_t]]

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

//-----------------------------------------------------------------------------
// [[u8_header_t]]

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
// [[u8_node_t]]

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
///////////////			lta support			///////////////
///////////////////////////////////////////////////////////////////////////////
// LTA = LE-CODE Track Archive

#define LTA_MAGIC		"LTR-ARCH"
#define LTA_MAGIC_NUM		0x4c54522d41524348ull
#define LTA_VERSION		1
#define LTA_DEFAULT_ALIGN	0x20
#define LTA_MAX_SIZE		0x7fffffe0
#define LTA_MAX_NODES		0x2000

#define USELTA_MAGIC		"#USE-LTA"
#define USELTA_MAGIC_NUM	0x235553452d4c5441ull

//-----------------------------------------------------------------------------
// [[lta_header_t]]

typedef struct lta_header_t
{
    be64_t	magic;		// file magic = LTA_MAGIC_NUM
    be32_t	version;	// major version number = LTA_VERSION
    be32_t	head_size;	// size of this header = minor version
    be32_t	file_size;	// total size of this file, aligned
    be32_t	node_off;	// offset of slot node list, aligned
    be32_t	node_size;	// size of single node == sizeof(lta_node_t)
    be32_t	base_slot;	// first used slot
    be32_t	n_slots;	// number of used slots

    // extension since v2.41a, 2024-01-22
    be32_t	ext_off;	// offset of extension string list
    be32_t	ext_size;	// size of extension string list
}
__attribute__ ((packed)) lta_header_t;

//-------------------------

static inline bool HaveExtLTA ( const lta_header_t *lta )
{
    return lta
	&& lta->head_size >= offsetof(lta_header_t,ext_size) + sizeof(lta->ext_size)
	&& lta->ext_size;
}

//-----------------------------------------------------------------------------
// [[lta_node_index_t]]

typedef enum lta_node_index_t
{
    LNPI_STD_SZS,	// par index for std SZS
    LNPI_STD_LFL,	// par index for std LFL
    LNPI_D_SZS,		// par index for _d SZS
    LNPI_D_LFL,		// par index for _d LFL
    LNPI__N,		// total number of 'par' indices

    LNRI_STD = 0,	// record index for std files
    LNRI_D,		// record index for _d files
    LNRI__N,		// total number of 'record' indices
}
lta_node_index_t;

//-----------------------------------------------------------------------------
// [[lta_node_par_t]]

typedef struct lta_node_par_t
{
    be32_t	offset;		// offset of file, aligned
    be32_t	size;		// size of file
}
__attribute__ ((packed)) lta_node_par_t;

//-----------------------------------------------------------------------------
// [[lta_node_record_t]]

typedef struct lta_node_record_t
{
    be32_t	szs_off;	// offset of SZS file, aligned
    be32_t	szs_size;	// size of SZS file
    be32_t	lfl_off;	// offset of LFL file, aligned
    be32_t	lfl_size;	// size of LFL file
}
__attribute__ ((packed)) lta_node_record_t;

//-----------------------------------------------------------------------------
// [[lta_node_t]]

typedef struct lta_node_t
{
    union
    {
	lta_node_par_t    par   [LNPI__N];  // 4 par as array
	lta_node_record_t record[LNRI__N];  // 2 records as array
	struct
	{
	    lta_node_record_t std;	// std file
	    lta_node_record_t d;	// _d file
	};
    };
}
__attribute__ ((packed)) lta_node_t;

//-------------------------

static inline uint GetNodeListSizeLTA ( const lta_header_t *lta )
	{ return lta->n_slots * sizeof(lta_node_t); }

//-----------------------------------------------------------------------------
// [[lta_manager_t]]

typedef struct lta_manager_t
{
    le_distrib_t *ld;			// current le_distrib_t
    ccp		destdir;		// destination directory
    bool	rm_source;		// true: remove source SZS files
    bool	append_ext;		// true: append extension list
    file_format_t force_ff;		// force file format id !=FF_UNKNOWN
    uint	track_index;		// current track index for "tracks-%u.lta"
    u_nsec_t	start_nsec;		// start time by GetTimerNSec();

    File_t	F;			// current LTA file

    int		distrib_end_slot;	// max used slot +1 of distribution, calculated first
    int		max_slots;		// max slots of current LTA
    int		base_slot;		// base slot of current LTA, only valid if >= 0
    int		last_slot;		// last used slot of current LTA

    size_t	data_offset;		// current data offset
    size_t	current_offset;		// return value of WriteFileAt()

    u8		*buf;			// temp buffer for files, alloced
    uint	buf_size;		// size of 'buf'
    exmem_list_t stored_szs;		// list with already stored SZS files (lta_node_record_t)
    exmem_list_t stored_lfl;		// list with already stored LFL files (lta_node_par_t)

    uint	std_count;		// counter of inserted std SZS files
    uint	d_count;		// counter of inserted _d SZS files
    uint	lfl_count;		// counter of inserted LFL files
    uint	convert_count;		// counter of converted files
    uint	cache_count;		// counter of cache using
    uint	szs_dup_count;		// counter of SZS duplicates
    uint	lfl_dup_count;		// counter of LFL duplicates

    u_msec_t	start_msec;		// start time, set by GetTimerMSec() 
    u_msec_t	last_log_msec;		// last log time, set by GetTimerMSec()
    u_msec_t	show_progress;		// >0: show progress counter every # ms
    uint	progress_nodes;		// number of processed slots
    uint	progress_tracks;	// number of processed tracks

    FastBuf_t	ext;			// list with file extensions (without leading '.')
    lta_node_t	node[LTA_MAX_NODES];	// nodes, local endian
}
lta_manager_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lfl support			///////////////
///////////////////////////////////////////////////////////////////////////////
// LFL = LE-CODE File List

#define LFL_MAGIC		"L-FL"
#define LFL_MAGIC_NUM		0x4c2d464c
#define LFL_VERSION		1

//-----------------------------------------------------------------------------
// [[lfl_node_t]]

typedef struct lfl_node_t
{
    be32_t	data_offset;	// offset of data relative to this record
    be32_t	data_size;	// size of data
    be32_t	next_offset;	// offset to next file of this chain, relative to this record
    char	file_name[];	// filename and aligned(4) data
}
__attribute__ ((packed)) lfl_node_t;

//-----------------------------------------------------------------------------
// [[lfl_header_t]]

typedef struct lfl_header_t
{
    be32_t	magic;		// file magic = LFL_MAGIC_NUM
    be32_t	version;	// major version number = LFL_VERSION
    be32_t	file_size;	// total size of this file, aligned(LTA)
    lfl_node_t	first_node[];	// first node
}
__attribute__ ((packed)) lfl_header_t;

//-----------------------------------------------------------------------------
// LFL interface

int IterateFilesLFL
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
);

enumError CreateLFL
(
    struct szs_file_t	*szs,		// valid szs
    ccp			source_dir,	// path to source dir
    bool		rm_common,	// TRUE: remove leading 'common/' from path
    bool		clear_if_empty	// TRUE: clear data (size 0 bytes) if no sub file added
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum mark			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[szs_marker_t]]

// Flags:
//  _U : object used
//  _PF : object active with full presence flag

enum szs_marker_t
{
  SLOT_31_71_SUNDS	= 0x01,  // bit for 'szs_file_t:slot_31_xx_71'
  SLOT_31_71_PYLON01	= 0x02,  // bit for 'szs_file_t:slot_31_xx_71'

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
///////////////			    UI-Check			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[ui_type_t]]

typedef enum ui_type_t
{
    UIT_UNDEFINED,	// ?
    UIT_UNKNOWN,	// -

    UIT_AWARD,		// A
    UIT_CHANNEL,	// C
    UIT_EVENT,		// E
    UIT_FONT,		// F
    UIT_GLOBE,		// G
    UIT_MENU_MULTI,	// M
    UIT_MENU_OTHER,	// O
    UIT_MENU_SINGLE,	// S
    UIT_PRESENT,	// P
    UIT_RACE,		// R
    UIT_TITLE,		// T

    UIT_LANGUAGE,	// l
    UIT__N,

    UIT__FIRST		= UIT_AWARD,
    UIT__LAST		= UIT_TITLE,

    UIT_M_AWARD		= 1 << UIT_AWARD,
    UIT_M_CHANNEL	= 1 << UIT_CHANNEL,
    UIT_M_EVENT		= 1 << UIT_EVENT,
    UIT_M_FONT		= 1 << UIT_FONT,
    UIT_M_GLOBE		= 1 << UIT_GLOBE,
    UIT_M_MENU_MULTI	= 1 << UIT_MENU_MULTI,
    UIT_M_MENU_OTHER	= 1 << UIT_MENU_OTHER,
    UIT_M_MENU_SINGLE	= 1 << UIT_MENU_SINGLE,
    UIT_M_PRESENT	= 1 << UIT_PRESENT,
    UIT_M_RACE		= 1 << UIT_RACE,
    UIT_M_TITLE		= 1 << UIT_TITLE,
    UIT_M_LANGUAGE	= 1 << UIT_LANGUAGE,
    UIT_M_ALL		= ( 1 << UIT__N ) -1,

    UIT_M_CUP_ICONS	= UIT_M_CHANNEL | UIT_M_MENU_MULTI | UIT_M_MENU_SINGLE,
}
ui_type_t;

//-----------------------------------------------------------------------------

extern const char ui_type_char[UIT__N+1];
extern const ccp  ui_type_name[UIT__N+1];

ccp GetUiTypeColor ( const ColorSet_t *colset, ui_type_t type );

///////////////////////////////////////////////////////////////////////////////
// [[ui_check_t]]

typedef struct ui_check_t
{
    OffOn_t	is_ui;		// is language independent UI file
    OffOn_t	is_korean;	// is korean variant

    uint	possible;	// mask of possible types (1<<UIT_*)
    char	ui_lang;	// '-' or language character
    ui_type_t	ui_type;	// UI type except UIT_LANGUAGE
    ui_type_t	type;		// final detected type := ui_type | UIT_LANGUAGE
}
ui_check_t;

//-----------------------------------------------------------------------------

void UiCheck ( ui_check_t *uc, szs_file_t *szs );

//-----------------------------------------------------------------------------
// [[szs_norm_info_t]]
// [[norm]]

typedef enum szs_norm_info_t
{
    SZI_NORM,		// N: file is normed
    SZI_PT_DIR,		// P: have pt_dir
    SZI_AI_PARAM,	// A: have AI param
    SZI_SPEED,		// S: have speed KMP/STGI factoe
    SZI_LAPS,		// #: (1-9) KMP/STGI number of laps
    SZI_RM_LEX_FEAT,	// F: have LEX section FEAT
    SZI_RM_LEX_TEST,	// T: have LEX section TEST

    SZI__N		// number of flags
}
szs_norm_info_t;

//-----------------------------------------------------------------------------
// [[szs_u8_info_t]]

typedef struct szs_u8_info_t
{
    u32		namepool_size;	// calculated size of name-pool
    u32		total_size;	// calculated total size
    bool	have_pt_dir;	// true: have './' prefix
}
szs_u8_info_t;

//-----------------------------------------------------------------------------
// [[szs_norm_t]]
// [[norm]]

typedef struct szs_norm_t
{
    //--- status

    szs_u8_info_t u8;			// relevant data to create an U8 archive

    //--- jobs

    bool	force_pt_dir;		// force './' as prefix for each subfile
    bool	rm_aiparam;		// remove subfiles with name 'aiparam*'
    bool	clean_lex;		// [[obsolete]] when following params become active
    bool	auto_add;		// true: autoadd files
    bool	add_cup_icons;		// true: add cup icons

    //--- [[2do]] not implemented yet

    bool	norm_speed;		// normalize speed (KMP/STGI)
    u8		max_laps;		// >0: limit number of laps (KMP/STGI)
    u8		set_laps;		// >0: set number of laps (KMP/STGI)
    bool	rm_lex_test;		// remove LEX section TEST
    bool	manage_lex_feat;	// manage LEX section FEAT: 0:off, 1:remove, 2:auto
    bool	purge_lex;		// purge lex section and remove course.bin if empty

    //--- for output via PrintNorm()

    FILE *f;				// NULL or output file
    int  indent;			// indention
    char modified[SZI__N+1];		// info vector about modification -> szs_norm_info_t
}
szs_norm_t;

///////////////////////////////////////////////////////////////////////////////
// [[scan_data_t]]

typedef struct scan_data_t
{
    szs_file_t	* szs;			// valid pointer to szs data structure
    SetupParam_t * setup_param;		// valid pointer to setup parameters
    char	path[PATH_MAX];		// the current path
    char	* path_rel;		// ptr to 'path': path relative to base directory
    char	* path_dir;		// ptr to 'path': current directory
    u32		n_directories;		// total number of directories
    u32		namepool_size_u8;	// total size of name pool (for U8)
    u32		depth;			// current depth
    u32		max_depth;		// max allowed depth
    u64		total_size;		// total aligned size of all files
    u32		align;			// data alignment
}
scan_data_t;

///////////////////////////////////////////////////////////////////////////////
// [[add_missing_t]]

typedef struct add_missing_t
{
    szs_file_t		*szs;		// valid szs
    scan_data_t		*sd;		// NULL or valid scan data
    szs_norm_t		*norm;		// NULL or valid norm data

    ccp			fname;		// not NULL: add this file (no autoadd)
    szs_subfile_t	*link;		// not NULL: link to this file (no autoadd)
    void		*data;		// not NULL: add this data (no autoadd)
    uint		size;		// size of 'data'
    bool		move_data;	// data is alloced

    szs_subfile_t	*last_subfile;	// last added file, set by AddMissingFile()

    bool		print_err;	// true: print errors about missing files
    int			log_indent;	// >-1: print log with indention

}
add_missing_t;

//-----------------------------------------------------------------------------

int AddMissingFile
(
    // returns 1 if added, 0 otherwise

    ccp			path,		// calculated path
    file_format_t	fform,		// FF_BRRES | FF_BREFF | FF_BREFT | FF_U8
    add_missing_t	*am		// user defined parameter
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
    ccp			fname		// NULL or file name
);

void CopySZS
(
    szs_file_t		*szs,		// SZS to initialize
    bool		init_szs,	// true: InitializeSZS(), false: ResetSZS()
    const szs_file_t	*source		// source SZS
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

void LinkCacheData
(
    // link/copy only if opt_cache && data && size

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
    LinkCacheData(szs->cache_fname,src_fname,szs->cdata,szs->csize);
}

//-----------------------------------------------------------------------------

file_format_t GetNewCompressionSZS ( szs_file_t * szs );

#define COMPR_DEFAULT -99

enumError CompressWith
(
    // returns ERR_WARNING for invalid ff, ERR_NOTHING_TO_DO if already compressed,
    // or return value of compression function

    szs_file_t	*szs,		// NULL or valid data
    int		compr,		// compression level, default if COMPR_DEFAULT, auto if < 1
    bool	rm_uncompr,	// remove uncompressed data after compression
    file_format_t ff_compr,	// use this file format for compression
    file_format_t ff_fallback	// fall back if 'ff_compr' is not valid
);

void MarkReadonlySZS ( szs_file_t * szs );
enumError CompressSZS ( szs_file_t * szs, int compr, bool remove_uncompressed );
enumError CompressYAZ ( szs_file_t * szs, int compr, bool remove_uncompressed );
bool NormalizeSZS ( szs_file_t *szs );
bool NormalizeExSZS ( szs_file_t *szs, bool rm_aiparam, bool clean_lex, bool autoadd );
bool PatchSZS ( szs_file_t * szs );
bool CanBeATrackSZS ( szs_file_t * szs );
void CalcHaveSZS ( szs_file_t * szs );

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
struct scan_data_t;

int AddMissingFileSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// filename to add
    file_format_t	fform,		// valid file format
    struct szs_norm_t	*norm,		// NULL or norm struct
    int			log_indent	// >-1: print log with indention
);

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

enumError CreateU8ByInfo
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    szs_u8_info_t	* u8info	// valid data
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

ccp GetStatusMissedFile ( MissedFile_t miss );

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
    szs_file_t	* szs2,		// second szs to compare
    int		recurse,	// 0:off, <0:unlimited, >0:max depth
    int		cut_files,	// <0:never, =0:auto, >0:always
    bool	quiet		// true: be quiet
);

void PrepareCheckTextureSZS ( szs_file_t * szs );

enumError CheckTextureRefSZS
(
    // returns ERR_OK | ERR_DIFFER | ERR_NOTHING_TO_DO | ERR_NO_SOURCE_FOUND | >=ERR_ERROR
    szs_file_t	* szs2,		// NULL or second szs to compare
    ccp		*status		// not NULL: store status info here -> FreeString()
				// if !opt_reference: *status=NULL
);

enumError CheckTextureSZS
(
    // returns ERR_OK | ERR_DIFFER | ERR_ERROR
    szs_file_t	* szs1,		// first szs to compare
    szs_file_t	* szs2,		// second szs to compare
    ccp		*status		// not NULL: store status info here -> FreeString()
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

    char mandatory_slot[9];	// empty | "arena" | "31+71" | "31+41+71" | "61" | "62"
    char slot_info[31];		// slot info text
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
					// 31 is an alias for "31+71" or "31+41+71"
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
	( szs_file_t * szs, uint select, bool add_value, ccp return_if_empty );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs file iterator		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[szs_iterator_func]]

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
    bool		job_ui_check;	// true: call UiCheck() before main iteration
    bool		job_norm_create;// true: normalize norm_create before main iteration
    SortMode_t		sort_mode;	// sort files
}
iterator_param_t;

//-----------------------------------------------------------------------------
// [[szs_iterator_t]]

typedef struct szs_iterator_t
{
    //--- global parameters

    szs_file_t		* szs;		// valid szs pointer
    const endian_func_t	* endian;	// endian functions
    iterator_param_t	itpar;		// global parameters

    //--- client values

    void		* param;	// user defined parameter
    int			client_int;	// initial set to null
// not used yet!
//  int			condition;	// 0:not set, <0:disabled, >0:enabled

    //--- UI check

    bool		job_ui_check;	// true: call UiCheck() before main iteration
    ui_check_t		ui_check;	// result of UiCheck()

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

    bool		no_dirs;	// true: archive without directory support
    bool		is_dir;		// true: directory argument
    bool		has_subfiles;	// true: this file has enabled sub files
    bool		fix_extension;	// true: call FixIteratorExtBy*()
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

    //--- adding files

    bool		job_create;	// true: re-create archive by adding new files
    //bool		log_created;	// true: archive updated
    szs_norm_t		norm_create;	// param for CreateU8ByInfo()
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

int FindFileSZS
(
    // -1: error, 0: not found + result cleared, 1: found + result set
    szs_file_t		*szs,		// valid szs
    ccp			path,		// path to find
    const iterator_param_t
			*p_itpar,	// NULL or iteration parameters
    int			recurse,	// 0:off, <0:unlimited, >0:max depth
    szs_iterator_t	*result		// not NULL: store result here
);

//-----------------------------------------------------------------------------

int IterateFilesSZS
(
    szs_file_t		*szs,		// valid szs
    szs_iterator_func	func,		// call back function
    void		*param,		// user defined parameter
    const iterator_param_t
			*p_itpar,	// NULL or iteration parameters
    int			recurse		// 0:off, <0:unlimited, >0:max depth
);

int IterateFilesParSZS
(
    szs_file_t		*szs,		// valid szs
    szs_iterator_func	func,		// call back function
    void		*param,		// user defined parameter
    bool		clean_path,	// true: clean path from ../ and more
    bool		show_root_node,	// true: include root node in iteration
    bool		job_ui_check,	// true: call UiCheck() before main iteration
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

uint CollectCommonFilesSZS
(
    szs_file_t		* szs,		// valid szs
    bool		clear,		// true: clear list before adding
    SortMode_t		sort_mode	// sort subfiles
);

int PrintFileHeadSZS( int fw_name );

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

///////////////////////////////////////////////////////////////////////////////

void FixIteratorExtByFF
(
    szs_iterator_t	*it,		// valid iterator data
    file_format_t	fform_file,	// source file format
    file_format_t	fform_arch	// (decompressed) archive format
);

void FixIteratorExtByData
(
    szs_iterator_t	*it,		// valid iterator data
    cvp			data,		// pointer to data
    uint		size		// size of data
);

void FixIteratorExt
(
    szs_iterator_t	*it		// valid iterator data
);

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
///////////////			LTA support			///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesLTA
(
    struct
	szs_iterator_t	*it,		// iterator struct with all infos
    bool		term		// true: termination hint
);

bool DumpLTA
(
    FILE		*f,		// output file
    uint		indent,		// indention
    const ColorSet_t	*col,		// NULL or colorset
    ccp			fname,		// NULL or filename
    const u8		*data,		// data to dump
    uint		size		// size of 'data'
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
}
raw_data_t;

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
///////////////			  BZ/BZIP2 support		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[wbz_header_t]] [[wlz_header_t]]

#define BZ_MAGIC	"WBZa"
#define BZ_MAGIC_NUM	0x57425a61

typedef struct wbz_header_t
{
    char	magic[4];		// = BZ_MAGIC
    char	first_8[8];		// copy of first 8 bytes of uncompressed
					// data for fast magic detection.
    u8		cdata[];		// compressed data
}
__attribute__ ((packed)) wbz_header_t;

typedef wbz_header_t wlz_header_t;

//-----------------------------------------------------------------------------
// [[ybz_header_t]] [[ylz_header_t]]

#define YBZ_MAGIC	YAZ0_MAGIC
#define YLZ_MAGIC	YAZ0_MAGIC

typedef struct ybz_header_t
{
    char	magic[4];		// = YBZ_MAGIC
    u32		uncompressed_size;	// size of uncompressed data
    u32		compressed_size;	// size of compressed data
    char	first_4[4];		// copy of first 4 bytes of uncompressed
					// data for fast magic detection.
    u8		cdata[];		// compressed data
}
__attribute__ ((packed)) ybz_header_t;

typedef ybz_header_t ylz_header_t;

//-----------------------------------------------------------------------------

enumError DecompressBZ    ( szs_file_t * szs, bool rm_compressed );
enumError DecompressYBZ   ( szs_file_t * szs, bool rm_compressed );
enumError DecompressBZIP2 ( szs_file_t * szs, bool rm_compressed );

enumError CompressBZ    ( szs_file_t * szs, int compr, bool remove_uncompressed );
enumError CompressYBZ   ( szs_file_t * szs, int compr, bool remove_uncompressed );
enumError CompressBZIP2 ( szs_file_t * szs, int compr, bool remove_uncompressed );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  LZ/LZMA support		///////////////
///////////////////////////////////////////////////////////////////////////////

#define LZ_MAGIC	"WLZa"
#define LZ_MAGIC_NUM	0x574c5a61

//-----------------------------------------------------------------------------

enumError DecompressLZ   ( szs_file_t * szs, bool rm_compressed );
enumError DecompressYLZ  ( szs_file_t * szs, bool rm_compressed );
enumError DecompressLZMA ( szs_file_t * szs, bool rm_compressed );

enumError CompressLZ    ( szs_file_t * szs, int compr, bool remove_uncompressed );
enumError CompressYLZ    ( szs_file_t * szs, int compr, bool remove_uncompressed );
enumError CompressLZMA  ( szs_file_t * szs, int compr, bool remove_uncompressed );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRASD support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define BRASD_MAGIC		"RASD"
#define BRASD_MAGIC_NUM		0x52415344

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_SZS_H

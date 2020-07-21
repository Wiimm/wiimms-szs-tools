
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

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_KCL_H
#define SZS_LIB_KCL_H 1

#include "lib-std.h"
#include "db-kcl.h"
#include "lib-image.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef TEST // [[cube]]
  #define SUPPORT_KCL_CUBE	1
  #define SUPPORT_KCL_XTRIDATA	0   // 0=off, 1=extra
  #define SUPPORT_KCL_DELTA	0   // 0|1
#else
  #define SUPPORT_KCL_CUBE	0
  #define SUPPORT_KCL_XTRIDATA	0
  #define SUPPORT_KCL_DELTA	0
#endif

///////////////////////////////////////////////////////////////////////////////

#define KCL_TEXT_MAGIC		"#KCL"
#define KCL_TEXT_MAGIC_NUM	0x234b434c

#define KCL_SIGNATURE		"WiimmSZS"

#define KCL_MTL_EXT		".mtl"
#define KCL_FLAG_FILE1		"\1P/\1N\1T"
#define KCL_FLAG_FILE2		"\1P/\1F\1T"
#define KCL_FLAG_EXT		".flag"

#define N_KCL_SECT		4

///////////////////////////////////////////////////////////////////////////////
///////////////			kcl_analyze_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_analyze_t]]

typedef struct kcl_analyze_t
{
    valid_t	valid;			// valid status
    u16		order_value;		// digits 1, 2, 3, 4 as dec to reflect order
    bool	order_ok;		// true: regular order (order_value==1234)
    u32		head_size;		// size of head
    u32		file_size;		// not NULL: size of file
    u32		off[N_KCL_SECT];	// offsets of KCL sections
    u32		size[N_KCL_SECT];	// max size of KCL sections (U32_MAX=unknown)
    u32		n[N_KCL_SECT];		// number of KCL entries based on 'size'
}
kcl_analyze_t;

ccp GetValidInfoKCL ( kcl_analyze_t * ka );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl_head_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kcl_head_t
{
  /*0x00*/  u32		sect_off[N_KCL_SECT];	// offset of the 4 sections
  /*0x10*/  float32	unknown_0x10;
  /*0x14*/  float32	min_octree[3];		// minimum values
  /*0x20*/  u32		mask[3];		// masks for spatial index
  /*0x2c*/  u32		coord_rshift;		// right shift for all coord
  /*0x30*/  u32		y_lshift;		// left shift for y values
  /*0x34*/  u32		z_lshift;		// left shift for z values
  /*0x38*/  float32	unknown_0x38;
  /*0x3c*/
}
__attribute__ ((packed)) kcl_head_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl_triangle_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kcl_triangle_t
{
  /*0x00*/  float32	length;			//
  /*0x04*/  u16		idx_vertex;		//
  /*0x06*/  u16		idx_normal[4];		//
  /*0x0e*/  u16		flag;			// KCL flag (includes the type)
  /*0x10*/
}
__attribute__ ((packed)) kcl_triangle_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl_tridata_status_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_tridata_status_t]]

typedef enum kcl_tridata_status_t
{
    TD_INVALID_NORM	= 0x01,		// 'length' & 'normal' are invalid
    TD_INVALID		= 0x02,		// invalid data
    TD_FIXED_PT		= 0x04,		// triangle was fixed/repaired
    TD_UNUSED		= 0x08,		// triangle not used in octree
    TD_REMOVED		= 0x10,		// triangle is marked as 'removed'
    TD_ADDED		= 0x20,		// triangle was added
    TD_MARK		= 0x40,		// temporary marker for calculations

    TDX_ORIG_PT		= 0x1000,	// original 'pt' valid
    TDX_ORIG_NORM	= 0x2000,	// original 'length' & 'normal' valid

} kcl_tridata_status_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl_tridata*_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_tridata_flt_t]] [[xtridata]]

typedef struct kcl_tridata_flt_t
{
    double3		pt[3];		// 3 points
    float		length;		// length
    float3		normal[4];	// 4 normal values
}
__attribute__ ((packed)) kcl_tridata_flt_t;

//-----------------------------------------------------------------------------
// [[kcl_tridata_dbl_t]] [[xtridata]]

typedef struct kcl_tridata_dbl_t
{
    double3		pt[3];		// 3 points
    double		length;		// length
    double3		normal[4];	// 4 normal values
}
__attribute__ ((packed)) kcl_tridata_dbl_t;

//-----------------------------------------------------------------------------
// [[kcl_tridata0_t]]

typedef struct kcl_tridata0_t
{
    double3		pt[3];		// 3 points
    float		length;		// length -> TD_INVALID_NORM
    float3		normal[4];	// 4 normal values -> TD_INVALID_NORM

 #if SUPPORT_KCL_DELTA
    float		delta_min;	// minimal distance of 3 points
    float		delta_max;	// maximal distance of 3 points
 #endif

    u16			status;		// bit field: kcl_tridata_status_t
    u16			in_flag;	// original flag of input
    u32			cur_flag;	// current flag

} kcl_tridata0_t;

//-----------------------------------------------------------------------------
// [[kcl_tridata1_t]] [[xtridata]]

typedef struct kcl_tridata1_t
{
    double3		pt[3];		// 3 points
    float		length;		// length -> TD_INVALID_NORM
    float3		normal[4];	// 4 normal values -> TD_INVALID_NORM

 #if SUPPORT_KCL_DELTA
    float		delta_min;	// minimal distance of 3 points
    float		delta_max;	// maximal distance of 3 points
 #endif

    u16			status;		// bit field: kcl_tridata_status_t
    u16			in_flag;	// original flag of input
    u32			cur_flag;	// current flag

    //-- extra data: input

    kcl_tridata_dbl_t	dbl;		// parts invalid on TD_INVALID_NORM
    kcl_tridata_dbl_t	orig;		// valid on TDX_ORIG_PT, TDX_ORIG_NORM

} kcl_tridata1_t;

//-----------------------------------------------------------------------------
// [[kcl_tridata_t]] [[xtridata]]

#if SUPPORT_KCL_XTRIDATA > 0
    typedef kcl_tridata1_t kcl_tridata_t;
#else
    typedef kcl_tridata0_t kcl_tridata_t;
#endif

//-----------------------------------------------------------------------------

void CopyTriData_F2D ( kcl_tridata_dbl_t *dest, const kcl_tridata_flt_t *src );
void CopyTriData_D2F ( kcl_tridata_flt_t *dest, const kcl_tridata_dbl_t *src );

static inline void CopyTriData_T2D ( kcl_tridata_dbl_t *dest, const kcl_tridata_t *src )
	{ CopyTriData_F2D(dest,(const kcl_tridata_flt_t*)src); }
static inline void CopyTriData_D2T ( kcl_tridata_t *dest, const kcl_tridata_dbl_t *src )
	{ CopyTriData_D2F((kcl_tridata_flt_t*)dest,src); }

static inline void CopyTriData_T2F ( kcl_tridata_flt_t *dest, const kcl_tridata_t *src )
	{ memcpy(dest,src,sizeof(*dest)); }
static inline void CopyTriData_F2T ( kcl_tridata_t *dest, const kcl_tridata_flt_t *src )
	{ memcpy(dest,src,sizeof(*src)); }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 kcl_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_mode_t]]

typedef enum kcl_mode_t
{
    //--- general options

    KCLMD_FAST		= 0x00000001,	// fast mode: don't find duplicates
    KCLMD_ROUND		= 0x00000002,	// round normals toward zero
    KCLMD_NORMALS	= 0x00000004,	// print normals into OBJ
    KCLMD_MTL		= 0x00000008,	// print materials on OBJ export
    KCLMD_WIIMM		= 0x00000010,	// print materials, use "wiimm.mtl" as file
    KCLMD_TRIANGLES	= 0x00000020,	// export as triangle list
    KCLMD_OUT_SWAP	= 0x00000040,	// swap points #2 and #3 while generating OBJ

    KCLMD_USEMTL	= 0x00000080,	// use 'usemtl' for groups if reading OBJ
    KCLMD_G		= 0x00000100,	// use 'g' for groups if reading OBJ
    KCLMD_CLIP		= 0x00000200,	// enable clip function => const 'KCL_CLIP'
    KCLMD_IN_SWAP	= 0x00000400,	// swap points #2 and #3 while reading OBJ
    KCLMD_AUTO		= 0x00000800,	// enable auto swap while reading OBJ

    KCLMD_CENTER	= 0x00001000,	// center world in cube area on octree creation
    KCLMD_ADD_ROAD	= 0x00002000,	// for 'kcl_ref' only: Add DMD_ROADOBJ objects

    KCLMD_HEX4		= 0x00004000,	// enable flag _XXXX detection in group names
    KCLMD_HEX23		= 0x00008000,	// enable flag _XX_XXX detection in group names
     KCLMD_M_HEX	 = KCLMD_HEX4
			 | KCLMD_HEX23,

    KCLMD_CUBE		= 0x00010000,	// use cubes around triangles for octree calculation

    KCLMD_SMALL		= 0x01000000,	// aggressive preset to get a smaller kcl
    KCLMD_CHARY		= 0x02000000,	// chary preset, Nintendo like
    KCLMD_DRAW		= 0x04000000,	// preset for drawing
     KCLMD_M_PRESET	 = KCLMD_SMALL
			 | KCLMD_CHARY
			 | KCLMD_DRAW,

     KCLMD_M_GENERAL	 = KCLMD_FAST
			 | KCLMD_ROUND
			 | KCLMD_NORMALS
			 | KCLMD_MTL
			 | KCLMD_WIIMM
			 | KCLMD_TRIANGLES
			 | KCLMD_OUT_SWAP
			 | KCLMD_G
			 | KCLMD_USEMTL
			 | KCLMD_CLIP
			 | KCLMD_IN_SWAP
			 | KCLMD_AUTO
			 | KCLMD_CENTER
			 | KCLMD_M_HEX
			 | KCLMD_CUBE
			 | KCLMD_M_PRESET,


    //--- KCL patching options

    KCLMD_DROP_AUTO	= 0x10000000,	// enable DROP auto mode
    KCLMD_DROP_UNUSED	= 0x20000000,	// remove unreferenced triangles
    KCLMD_DROP_FIXED	= 0x40000000,	// remove fixed triangles
    KCLMD_DROP_INVALID	= 0x80000000,	// remove invalid triangles
     KCLMD_M_DROP	 = KCLMD_DROP_AUTO
			 | KCLMD_DROP_UNUSED
			 | KCLMD_DROP_FIXED
			 | KCLMD_DROP_INVALID,

    KCLMD_RM_FACEDOWN	= 1ull<<32,	// remove face down drivable triangles
    KCLMD_RM_FACEUP	= 1ull<<33,	// remove face up walls
     KCLMD_M_RM		 = KCLMD_RM_FACEDOWN
			 | KCLMD_RM_FACEUP,

    KCLMD_CONV_FACEUP	= 1ull<<34,	// convert face up walls to road
    KCLMD_WEAK_WALLS	= 1ull<<35,	// OR 0x8000 to KCL flags of walls
    KCLMD_NEW		= 1ull<<36,	// create a new KCL file even if nothing changed
    KCLMD_SORT		= 1ull<<37,	// sort triangles
     KCLMD_M_PATCH	 = KCLMD_M_DROP
			 | KCLMD_M_RM
			 | KCLMD_CONV_FACEUP
			 | KCLMD_WEAK_WALLS
			 | KCLMD_NEW
			 | KCLMD_SORT,

    //--- tiny modes

    KCLMD_S_TINY	= 40,
    KCLMD_TINY_0	= 0ull << KCLMD_S_TINY,	// TINY levels
    KCLMD_TINY_1	= 1ull << KCLMD_S_TINY,
    KCLMD_TINY_2	= 2ull << KCLMD_S_TINY,
    KCLMD_TINY_3	= 3ull << KCLMD_S_TINY,
    KCLMD_TINY_4	= 4ull << KCLMD_S_TINY,
    KCLMD_TINY_5	= 5ull << KCLMD_S_TINY,
    KCLMD_TINY_6	= 6ull << KCLMD_S_TINY,
    KCLMD_TINY_7	= 7ull << KCLMD_S_TINY,
     KCLMD_M_TINY	 = KCLMD_TINY_7,


    //--- hidden options

    KCLMD_LOG		= 1ull<<43,	// enable kcl trace log to stdout
    KCLMD_SILENT	= 1ull<<44,	// suppress kcl messages
    KCLMD_POSLEN	= 1ull<<45,	// force length>=0 in CalcPointsTriData()
    KCLMD_INPLACE	= 1ull<<46,	// force inplace patching
     KCLMD_M_HIDDEN	 = KCLMD_LOG
			 | KCLMD_SILENT
			 | KCLMD_POSLEN
			 | KCLMD_INPLACE,

    KCLMD_F_SCRIPT	= 1ull<<50,	// execute script first
    KCLMD_F_HIDE	= 1ull<<51,	// help flag to hide modes in PrintKclMode()


    //--- test mode

    KCLMD_TEST		= 1ull<<60,	// reserved for testing


    //--- summaries

    KCLMD_M_DEFAULT	= KCLMD_MTL		// default settings
			| KCLMD_DROP_AUTO,

    KCLMD_M_FIX_ALL	= KCLMD_M_DROP		// general kcl fix mode
			| KCLMD_M_RM
			| KCLMD_NEW,

    KCLMD_M_ALL		= KCLMD_M_GENERAL	// all relevant bits
			| KCLMD_M_PATCH
			| KCLMD_M_HIDDEN
			| KCLMD_ADD_ROAD
			| KCLMD_F_SCRIPT
			| KCLMD_M_TINY
			| KCLMD_TEST,

    KCLMD_M_PRINT	= KCLMD_M_ALL		// all relevant bits for obj output
			% ~KCLMD_ADD_ROAD
			& ~KCLMD_IN_SWAP
			& ~KCLMD_AUTO
			& ~KCLMD_M_HEX
			& ~KCLMD_M_HIDDEN,

} kcl_mode_t;

//-----------------------------------------------------------------------------

extern int enable_kcl_drop_auto;
extern kcl_mode_t KCL_MODE;

void SetKclMode ( kcl_mode_t new_mode );
int  ScanOptKcl ( ccp arg );
uint PrintKclMode( char *buf, uint bufsize, kcl_mode_t mode );
ccp  GetKclMode();
void SetupKCL();

extern Color_t user_color[N_KCL_USER_FLAGS];
extern uint n_user_color;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    kcl_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_t]]

#define KCL_MAX_STAT_DEPTH     32
#define KCL_MAX_TRIANGLES  0xffff
#define KCL_MAX_PT_PER_TRI     50

typedef struct kcl_t
{
    //--- base info

    ccp			fname;			// alloced filename of loaded file
    FileAttrib_t	fatt;			// file attribute
    file_format_t	fform;			// FF_KCL | FF_KCL_TXT
    file_format_t	fform_outfile;		// planned output format
    int			revision;		// tool revision
    char		signature_info[9];	// info text of signature, found in vertex #0
    float		signature;		// >0.0: signature number, found in vertex #0

    //--- base settings

    bool		silent_octree;		// true: create octree without comments
						// always init with false
    bool		fast;			// true: don't find duplicates
						// init by 'KCL_MODE & KCLMD_FAST'
    uint		min_cube_size;		// minimum cube size
						// init by 'KCL_MIN_SIZE'
    uint		max_cube_size;		// maximum cube size
						// init by 'KCL_MAX_SIZE'
    uint		cube_blow;		// blow up the cube to find tirangles
						// init by 'KCL_BLOW'
    uint		max_cube_triangles;	// maximum triangles per cube
						// init by 'KCL_MAX_TRI'
    uint		max_octree_depth;	// maximum octree depth
						// init by 'KCL_MAX_DEPTH'

    //--- file header data

    double3		min_octree;		// minimum coordinate values
    u32			mask[3];		// masks for spatial index
    u32			coord_rshift;		// right shift for all coord
    u32			y_lshift;		// left shift for y values
    u32			z_lshift;		// left shift for z values
    float		unknown_0x10;		// unknown header value
    float		unknown_0x38;		// unknown header value

    //--- triangle data

    List_t		tridata;		// triangle data base
    bool		norm_valid;		// true: all normal data is valid
    bool		used_valid;		// true: used markers are valid
    bool		no_limit;		// true: no triangle limit
    bool		limit_warning;		// true: limit warning is already printed
    double3		tri_minval;		// minimum allowed triangle value,
						//   initialized with -1e9
    double3		tri_maxval;		// maximum allowed triangle value,
						//   initialized with +1e9
    uint		tri_inv_count;		// number of invalid triangles
						//   all invalid are also fixed
						//   but not counted as 'tri_fix_count'
    uint		tri_fix_count;		// number of fixed triangles
						//   this excludes 'tri_inv_count'

    //--- octree data

    u8			*octree;		// NULL or pointer to octree
    uint		octree_size;		// size of 'octree'
    bool		octree_valid;		// octree & header data are valid
    bool		octree_alloced;		// true: FREE(octree)
    bool		recreate_octree;	// true: recreate octree before storing


    //--- statistics about triangle points

    double3		min;			// min coord of all triangle points
    double3		mean;			// mean coord of all triangle points
    double3		max;			// max coord of all triangle points
    double		tmin_dist;		// minimum triangle point distance
    double		tmax_dist;		// maximum triangle point distance
    bool		min_max_valid;		// true: all 4 vars of this part are valid

    //--- raw data, created by CreateRawKCL()

    u8			* raw_data;		// NULL or data
    uint		raw_data_size;		// size of 'raw_data'
    bool		raw_data_alloced;	// true: FREE(raw_data)
    bool		model_modified;		// the model was modified
						// raw data is invalid

    //--- flag database

    ccp			flag_fname;		// path of flag file, alloced
    bool		flag_db_loaded;		// true: flag db is loaded
    bool		accept_hex4;		// true: accept '_FFFF'
    bool		accept_hex23;		// true: accept '_TT_VVV'
    FormatField_t	flag_name;		// list with names
    FormatField_t	flag_pattern;		// list with patterns
    FormatField_t	flag_missing;		// list with unresolved group names


    //--- script flags

    bool		script_tri_removed;	// true: triangle removed
    bool		script_tri_added;	// true: triangle added
    bool		script_pt_modified;	// true: points modified


    ///////////////////////////////////////////////////////////////////////////
    ///////////////	  Statistics ==> LAST KCL DATA!		///////////////
    ///////////////////////////////////////////////////////////////////////////

    //--- statistics: base

    bool		stat_valid;		// IMPORTANT: First statsitics data!
						// true: all statistics are valid

    //--- base statistics

    uint		mask_bits[3];		// number of 0 bits in mask
    uint		bcube_bits[3];		// number of base cube relevant bits
    uint		hash_bits;		// number of bits in hash value
    uint		n_bcubes[3];		// number of base cubes per axis
    uint		total_bcubes;		// total number of base cubes
    uint		bcube_width;		// edge length of base cubes
    double3		max_octree;		// maximum coordinate values


    //--- statistics: octree

    u32			min_o_offset;		// first cube node
    u32			max_o_offset;		// end of last cube node
    u32			min_l_offset;		// min found list offset
    u32			max_l_offset;		// max found list offset
    uint		n_cube_nodes;		// number of octree cube nodes
						// (each with 8 links)
    uint		total_cubes;		// total number of cubes
    uint		n_trilist;		// number of tri-list links


    //--- statistics: octree lists

    u8			*o_depth;		// octree depth, N=n_octree
    u16			*o_count;		// octree ref counts, N=n_octree
    u16			*l_count;		// trilist ref counts, N=n_trilist

    uint		n_0_list;		// num of triangle lists
    uint		n_0_link;		// num of triangle links
    uint		n_tri_list;		// num of triangle lists
    uint		n_tri_link;		// num of triangle links
    uint		max_tri_len;		// max triangles in a list
    uint		min_depth;		// min octree depth with triangles
    uint		max_depth;		// max octree depth
    uint		max_depth_tri_len[KCL_MAX_STAT_DEPTH];
						// max triangles in a list,
						// depth dependent
    float		ave_depth;		// average depth
    uint		sum_tri_len;		// sum of all list length's


    //--- statistics: usage

    uint		n_invalid_tri_ref;	// num of invalid tri references
    uint		n_tri_used;		// num of used triangles

} kcl_t;

//-----------------------------------------------------------------------------

extern kcl_t * kcl_ref;		 // NULL or reference KCL for calculations
extern kcl_t * kcl_ref_prio;	// NULL or prioreference KCL for calculations

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KCL interface			///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsKCL();

void SetupReferenceKCL
(
    VarMap_t *vm	// not NULL: define KCL variables here
);

void InitializeKCL ( kcl_t * kcl );
void ResetKCL ( kcl_t * kcl );
void ClearKCL ( kcl_t * kcl, bool init_kcl );
void CopyKCL ( kcl_t * kcl, bool init_kcl, const kcl_t * src );

void LoadParametersKCL
(
    ccp		log_prefix		// not NULL:
					//    print log message with prefix
);

bool TransformKCL ( kcl_t * kcl );
enumError CreateOctreeKCL ( kcl_t * kcl );

void ResetStatisticsKCL	( kcl_t * kcl );
void CalcStatisticsKCL	( kcl_t * kcl );
void CalcMinMaxKCL	( kcl_t * kcl );
uint CalcBlowSizeKCL	( kcl_t * kcl );
bool DropSortTriangles	( kcl_t * kcl );

bool DropSortHelper
(
    kcl_t		*kcl,		// valid KCL
    uint		drop_mask,	// not 0: drop triangles with set bits
    bool		sort		// true: sort triangles
);

bool KCL_ACTION_LOG ( const char * format, ... )
	__attribute__ ((__format__(__printf__,1,2)));

///////////////////////////////////////////////////////////////////////////////

uint CheckMetricsTriData
(
    // check length and minimal height of triangles

    kcl_tridata_t	*td,		// valid pointer to first record
    uint		n,		// number of records to process
    kcl_t		*kcl		// not NULL: store statistics
);

//-----------------------------------------------------------------------------

uint CalcPointsTriData
(
    // calculate 'pt' using 'length' & 'normal'
    // returns the number of fixes

    kcl_tridata_t	*td,		// valid pointer to first record
    uint		n,		// number of records to process
    kcl_t		*kcl,		// not NULL: store statistics
    bool		use_minmax	// true & 'kcl': use 'tri_minval' and 'tri_maxval'
);

//-----------------------------------------------------------------------------

void CalcNormalsTriData
(
    // calculate 'length' & 'normal' using 'pt'
    // the calculation is only done, if 'TD_INVALID_NORM'

    kcl_t		*kcl,		// valid KCL
    kcl_tridata_t	*td,		// valid pointer to first record
    uint		n		// number of records to process
);

//-----------------------------------------------------------------------------

void CalcNormalsKCL
(
    // calculate 'length' & 'normal' using 'pt'
    // the calculation is only done, if '!kcl->norm_valid'

    kcl_t		*kcl,		// valid KCL
    bool		force		// true: ignore 'kcl->norm_valid'
);

//-----------------------------------------------------------------------------

void RoundNormalsKCL
(
    // round the normal values

    kcl_t		*kcl,		// valid KCL
    int			round_pow2	// round to '1/(2^round_pow2)'
);

//-----------------------------------------------------------------------------

kcl_tridata_t *GetTridataKCL
(
    kcl_t		*kcl,		// valid KCL
    uint		tri_index	// triangle index
);

///////////////////////////////////////////////////////////////////////////////
// [[kcl_tri_param_t]]

typedef struct kcl_tri_param_t
{
    kcl_t		*kcl;		// valid KCL data structure
    u32			kcl_flag;	// value of kcl flag
    bool		is_orig;	// true: is original data

    const double3	*norm;		// NULL or pointer to normal
					// => auto detection to swap pt2,pt3
    uint		swap_count;	// not NULL: increment on swap
}
kcl_tri_param_t;

//-----------------------------------------------------------------------------

static inline void InitializeTriParam
	( kcl_tri_param_t *tp, kcl_t *kcl, u32 kcl_flag )
{
    DASSERT(tp);
    DASSERT(kcl);
    memset(tp,0,sizeof(*tp));
    tp->kcl = kcl;
    tp->kcl_flag = kcl_flag;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t * PrepareAppendTrianglesKCL
(
    // Grows the triangle field and the 'used' count.
    // Returns a pointer to the first triangle.
    // If to many triangles:
    //   => a warning is printed, but only once for a kcl. NULL is returned

    kcl_t		*kcl,		// valid KCL data structure
    uint		n_append	// number of triangles to append
);

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendTriangleKCL
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_tri_param_t	*par,		// valid data structure
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3		// pointer to point #3
);

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendTriangleKCLp
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3		// pointer to point #3
);

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendQuadrilateralKCL
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_tri_param_t	*par,		// valid data structure
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3,		// pointer to point #3
    const double3	*pt4		// pointer to point #4
);

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendQuadrilateralKCLp
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3,		// pointer to point #3
    const double3	*pt4		// pointer to point #4
);

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendTrianglesKCL
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_tri_param_t	*par,		// valid data structure
    const double3	*pt1,		// pointer to first point
    int			pt_delta,	// delta in bytes to next point, <0 possible
    uint		n_pt		// number of points
);

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendTrianglesKCLp
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pt1,		// pointer to first point
    int			pt_delta,	// delta in bytes to next point, <0 possible
    uint		n_pt		// number of points
);

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendOctahedronKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    octahedron_t	*oct		// valid octahedron data
);

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendCuboidKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    cuboid_t		*c		// valid cuboid data
);

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendPyramidKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*apex,		// the apex of the pyramid
    const double3	*pt1,		// pointer to first point
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt,		// number of points
    bool		reverse		// true: reverse face orientation
);

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendPrismKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*axis1,		// first axis point
    const double3	*axis2,		// second axis point
    const double3	*dir,		// NULL or direction of first edge
    double		r,		// radius
    uint		n_side,		// number of sides, max=100
    uint		mode,		// bitfield: 0=prism, 1=antiprism
    u32			base_flag,	// color of base, -1:no base
    double		base_height,	// height of base, truncated to 40% of length
					// if 0.0: use a plane, otherwise a pyramid
    u32			arrow_flag,	// color of arror peak at axis2, -1:no arrow
    double		arrow_size,	// relative size of arrow (multiplied by *r),
					// truncated to length. no arrow if <=0.0
    bool		arrow_flip	// true: flip direction of arrow
);

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendCylinderKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pos,		// base point of cylinder
    const double3	*p_scale,	// scale. If NULL: assume v(1,1,1)
    double		scale_factor,	// Scale is multiplied by this factor
    const double3	*rotate,	// NULL or rotation vector
    uint		n_side,		// number of sides, max=100
    u32			base_flag	// color of bases, -1:no base
);

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendJoistKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    u32			kcl_flag,	// parameter for AppendTrianglesKCLp()
    double		length,		// length of corpus
    const double3	*p1,		// first point
    const double3	*p2,		// second point
    const double3	*p3,		// helper point to find direction
    int			n_mark,		// >0: add marker representing the number (modulo)
    u32			mark5_flag	// >0: replace 5 marker by one of this color
);

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    bool		use_data	// true: data is valid on 'kcl' live time
);

//-----------------------------------------------------------------------------

enumError ScanTextKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// FF_UNNOWN or file format
);

//-----------------------------------------------------------------------------

enumError ScanKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    bool		use_data,	// true: data is valid on 'kcl' live time
    CheckMode_t		mode		// not NULL: call CheckKCL(mode)
);

//-----------------------------------------------------------------------------

struct raw_data_t;

enumError ScanRawDataKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    struct raw_data_t	* raw,		// valid raw data
    bool		move_data,	// true: move 'raw.data' to 'kcl'
    CheckMode_t		mode		// not NULL: call CheckKCL(mode)
);

//-----------------------------------------------------------------------------

enumError LoadKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckKCL(mode)
);

//-----------------------------------------------------------------------------

int ScanOptLoadKcl ( ccp arg );
kcl_t * LoadReferenceKCL();
kcl_t * CreateReferenceKCL();
void UnloadAutoReferenceKCL();
void DefineAutoLoadReferenceKCL ( ccp fname, ccp ext );

static inline kcl_t * GetReferenceKCL()
	{ return kcl_ref_prio ? kcl_ref_prio : LoadReferenceKCL(); }

///////////////////////////////////////////////////////////////////////////////

enumError CreateRawKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    bool		add_sig		// true: add signature
);

//-----------------------------------------------------------------------------

enumError SaveRawKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError DumpKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		print_tridata,	// true: triangle list
    uint		print_octree	// octree print level (0..2)
);

//-----------------------------------------------------------------------------

enumError DumpTrianglesKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname		// filename of destination
);

//-----------------------------------------------------------------------------

enumError ListTrianglesKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		print_normals,	// true: print large tables
    uint		precision	// floating point pre: 0=+0, .. 3=+6
);

//-----------------------------------------------------------------------------

enumError TraverseOctreeKCL
(
    kcl_t		*kcl,		// pointer to valid KCL
    FILE		*f,		// output file, if NULL set verbosity:=0
    double3		pt,		// point to analyze
    bool		print_tri_val,	// true: print tri values instead of index
    uint		verbosity	// verbose level
					//   >=1: print base calculations
					//   >=2: print step calculations
);

//-----------------------------------------------------------------------------

double FallKCL
(
    // returns the height or -1.0 on fail

    kcl_t		*kcl,		// pointer to valid KCL
    FILE		*f,		// output file, if NULL: verbose:=0
    double3		pt,		// mid point of cube to analyze
    uint		width,		// width of cube
    int			verbosity,	// verbose level
					//   <=0: silent
					//   >=1: log triangle search
					//   >=2: print base calculations
					//   >=3: print step calculations
    u32			type_mask,	// kcl type selector (bit field)
    int			*res_kcl_flag	// not NULL: store KCL flag here
);

///////////////////////////////////////////////////////////////////////////////

int CheckKCL
(
    // returns number of errors

    kcl_t		* kcl,		// pointer to valid KCL
    CheckMode_t		mode		// print mode
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KCL flags			///////////////
///////////////////////////////////////////////////////////////////////////////

void LoadFlagFileKCL
(
    kcl_t		* kcl		// pointer to valid KCL
);

//-----------------------------------------------------------------------------

int GetFlagByNameKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			name,		// upper case name to search
    int			new_value	// return value if not found
					// >0: used as new value

    // The name is searched in 4 steps:
    //   1. Search literal in 'flag_name' (ignore case)
    //   2. Search pattern in 'flag_pattern' (ignore case)
    //   3. Search pattern in 'flag_missing' (ignore case)
    //   4. Analyze the last 5 characters for '_ffff'
    //   5. Analyze the last 7 characters for '_tt_vvv'
    //   6. Use 'new_value'
);

//-----------------------------------------------------------------------------

uint * AllocFlagCount();

uint CountFlagsKCL
(
    // returns the total number of flag groups

    const kcl_t		* kcl,		// pointer to valid KCL
    uint		* count		// pointer to field with NNN_KCL_FLAG elements
);

ccp GetGroupNameKCL ( uint kcl_flag, bool add_info );
uint GetExportFlagKCL ( uint kcl_flag );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KCL Type transformations	///////////////
///////////////////////////////////////////////////////////////////////////////

extern int have_kcl_patch_count;
extern StringField_t kcl_script_list;
extern u16 *patch_kcl_flag; // NULL or 'N_KCL_FLAG' elements

//-----------------------------------------------------------------------------

int ScanOptKclScript ( ccp arg );

void SetupPatchKclFlag();
void ResetPatchKclFlag();
void PurgePatchKclFlag();
void RemovePatchKclFlag();
int ScanOptKclFlag ( ccp arg );
int ScanOptTriArea ( ccp arg );
int ScanOptTriHeight ( ccp arg );

//-----------------------------------------------------------------------------

bool PatchRawDataKCL
(
    void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname		// file name for error messages
);

//-----------------------------------------------------------------------------

bool PatchKCL
(
    kcl_t		* kcl		// pointer to valid KCL
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			octree iterator			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_cube_t]]

typedef struct kcl_cube_t
{
    s32			min[3];		// minimum cube coordinates
    s32			max[3];		// maximum cube coordinates

} kcl_cube_t;

//-----------------------------------------------------------------------------
// [[kcl_cube_list_t]]

#if SUPPORT_KCL_CUBE
#define MAX_CUBE_TRI 10

typedef struct kcl_cube_list_t
{
    uint		n;		// number of used cubes
    kcl_cube_t		cube[MAX_CUBE_TRI]; // list of cubes

} kcl_cube_list_t;

#endif
//-----------------------------------------------------------------------------
// [[kcl_tri_t]]

typedef struct kcl_tri_t
{
    s32			pt[3][3];	// normalized positions of 3 points
    kcl_cube_t		cube;		// min and max coordinates
    u32			select;		// select level

} kcl_tri_t;

///////////////////////////////////////////////////////////////////////////////

typedef int (*oct_func)
(
    kcl_t		*kcl,		// pointer to valid KCL
    void		*param,		// user parameter
    const kcl_cube_t	*cube,		// pointer to current cube geometry
					// relative to kcl->min_octree;
    const u16		*tri_list,	// NULL or pointer to triangle index list
    uint		n_list		// number of elements in 'tri_list'
);

///////////////////////////////////////////////////////////////////////////////

int OctreeIteratorKCL
(
    kcl_t		*kcl,		// pointer to valid KCL
    oct_func		func,		// call back function
    void		*param		// user parameter
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			draw png			///////////////
///////////////////////////////////////////////////////////////////////////////

extern bool opt_png;		// if true: PNG drawing is enabled
extern uint opt_png_pix_size;	// pixel size, =0: disabled
extern uint opt_png_aalise;	// a factor for antialising
extern int  opt_png_x1;		// x1<x2: left border
extern int  opt_png_x2;		// x1<x2: right border
extern int  opt_png_y1;		// y1<y2: bop forder
extern int  opt_png_y2;		// y1<y2: bottom border
extern u32  opt_png_type_mask;	// typemask for KCL selection.
extern bool opt_png_rm;		// true: remove unused triangles

//-----------------------------------------------------------------------------

extern int ScanOptPng ( ccp arg );

struct kmp_t;
enumError SaveImageKCL
(
    struct kmp_t	* kmp,		// pointer to valid KMP
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_KCL_H

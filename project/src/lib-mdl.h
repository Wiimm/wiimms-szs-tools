
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
 *   Copyright (c) 2011-2022 by Dirk Clemens <wiimm@wiimm.de>              *
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

#ifndef SZS_LIB_MDL_H
#define SZS_LIB_MDL_H 1

#include "lib-std.h"
#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_section_id]]

typedef enum mdl_section_id
{
    MDL_SECTION_0,
    MDL_SECTION_1,
    MDL_SECTION_2,
    MDL_SECTION_3,
    MDL_SECTION_4,
    MDL_SECTION_5,
    MDL_SECTION_6,
    MDL_SECTION_7,
    MDL_SECTION_8,
    MDL_SECTION_9,
    MDL_SECTION_10,
    MDL_SECTION_11,

    MDL_SETUP,
    MDL_BONE_TABLE,
    MDL_IGNORE,
}
mdl_section_id;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct mdl_head_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_head_t]]

typedef struct mdl_head_t
{
  /* 0x00 */	u32		head_len;	// Header Length (0x40)
  /* 0x04 */	u32		offset;		// File Header Offset
  /* 0x08 */	u32		unknown_0x08;	// Unknown
  /* 0x0c */	u32		unknown_0x0c;	// Unknown
  /* 0x10 */	u32		n_vertex;	// Vertex Count
  /* 0x14 */	u32		n_face;		// Face Count
  /* 0x18 */	u32		unknown_0x18;	// Unknown
  /* 0x1c */	u32		n_bone;		// Bone Count
  /* 0x20 */	u32		unknown_0x20;	// Unknown (0x01000000)
  /* 0x24 */	u32		bone_offset;	// Bone Table Offset
  /* 0x28 */	float32		min[3];		// Minimum (float3)
  /* 0x34 */	float32		max[3];		// Maximum (float3)
  /* 0x40 */					// ---END---

}
__attribute__ ((packed)) mdl_head_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 mdl_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum mdl_mode_t
{
    //--- MDL patching objects

    MDLMD_PARENT_NODE	= 0x0001,		// patch MDL parent node
    MDLMD_CHILD_NODES	= 0x0002,		// patch all MDL child nodes
     MDLMD_M_NODES	 = MDLMD_PARENT_NODE
			 | MDLMD_CHILD_NODES,

     MDLMD_M_OBJECT	 = MDLMD_M_NODES,


    //--- MDL patching options

    MDLMD_VECTOR	= 0x0010,		// patch the base vectors
    MDLMD_MATRIX	= 0x0020,		// calc transf-matrix and it's inverse
    MDLMD_VERTEX	= 0x0040,		// patch vertex list

     MDLMD_M_PATCH	 = MDLMD_VECTOR
			 | MDLMD_MATRIX
			 | MDLMD_VERTEX,


    //--- hidden options

    MDLMD_LOG		= 0x00100000,		// enable mdl trace log to stdout
    MDLMD_SILENT	= 0x00200000,		// enable mdl trace log to stdout
     MDLMD_M_HIDDEN	 = MDLMD_LOG
			 | MDLMD_SILENT,

    //--- test modes

    MDLMD_TEST0		= 0x00000000,
    MDLMD_TEST1		= 0x10000000,
    MDLMD_TEST2		= 0x20000000,
    MDLMD_TEST3		= 0x30000000,
     MDLMD_M_TEST	 = MDLMD_TEST3,		// all bits!


    //--- summaries

    MDLMD_F_HIDE	= 0x40000000,		// help flag to hide modes in PrintMdlMode()

    MDLMD_M_DEFAULT	= MDLMD_VERTEX,		// default settings

    MDLMD_M_ALL		= MDLMD_M_OBJECT	// all relevant bits
			| MDLMD_M_PATCH
			| MDLMD_M_HIDDEN
			| MDLMD_M_TEST,

    MDLMD_M_PRINT	= MDLMD_M_ALL		// all relevant bits for obj output
			& ~MDLMD_M_HIDDEN,

} mdl_mode_t;

//-----------------------------------------------------------------------------

extern mdl_mode_t MDL_MODE;
extern int have_mdl_patch_count;

void LoadParametersMDL
(
    ccp		log_prefix		// not NULL:
					//    print log message with prefix
);

void SetMdlMode ( uint new_mode );
int  ScanOptMdl ( ccp arg );
mdl_mode_t GetRelevantMdlMode ( mdl_mode_t mode );
uint PrintMdlMode( char *buf, uint bufsize, mdl_mode_t mode );
ccp  GetMdlMode();
void SetupMDL();

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct mdl_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_t]]

struct mdl_sect1_t;

typedef struct mdl_t
{
    //--- base info

    ccp			fname;			// alloced filename of loaded file
    FileAttrib_t	fatt;			// file attribute
    file_format_t	fform;			// FF_MDL | FF_MDL_TXT

    //--- data

    uint		version;		// the mdl version of the source
    uint		n_sect;			// number of sections
    ccp			name;			// name of mdl, alloced
    mdl_head_t		head;			// copy of mdl header, host endian

    u32			*bone_tab;		// data of bone table
    uint		bone_n;			// number of elements in 'bone_tab'

 #if USE_NEW_CONTAINER_MDL
    Container_t		container;		// data container
 #else
    DataContainer_t	*old_container;		// old data container support, init with NULL
 #endif
    u32			mdl_offset;		// offset of MDL in 'old_container'

    //--- list of elements + string pool

    List_t		elem;			// list of all elements (MemItem_t)
    string_pool_t	spool;			// strings for 'elem'

    //--- parser helpers

    int			revision;		// tool revision
    bool		is_pass2;		// true: LEX compiler runs pass2

    //--- raw data, created by CreateRawMDL()

    u8			* raw_data;		// NULL or data
    uint		raw_data_size;		// data size without string pool
    uint		raw_file_size;		// size of 'raw_data'

} mdl_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeMDL ( mdl_t * mdl );
void ResetMDL ( mdl_t * mdl );
const VarMap_t * SetupVarsMDL();

//-----------------------------------------------------------------------------

ccp GetStringMDL
(
    mdl_t		* mdl,		// MDL data structure
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid	// result if invalid
);

//-----------------------------------------------------------------------------

ccp GetStringExMDL
(
    mdl_t		* mdl,		// MDL data structure
    const void		* base,		// base for offset
    u32			sect_offset,	// offset of other section
    u32			str_offset,	// offset of string offset in other section
    ccp			if_invalid	// result if invalid
);

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerMDL_t	* cdata		// NULL or container-data
);

//-----------------------------------------------------------------------------

enumError ScanTextMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerMDL_t	* cdata,	// NULL or container-data
    CheckMode_t		mode		// not NULL: call CheckMDL(mode)
);

//-----------------------------------------------------------------------------

struct raw_data_t;

enumError ScanRawDataMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckMDL(mode)
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckMDL(mode)
);

//-----------------------------------------------------------------------------

enumError SaveRawMDL
(
    mdl_t		* mdl,		// pointer to valid MDL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextMDL
(
    const mdl_t		* mdl,		// pointer to valid MDL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

///////////////////////////////////////////////////////////////////////////////

enumError PrintStringsMDL
(
    FILE		* f,		// output file
    int			indent,		// indention
    mdl_t		* mdl		// valid MDL
);

///////////////////////////////////////////////////////////////////////////////

enumError AppendMDL
(
    mdl_t		* mdl,		// working MDL (valid)
    const mdl_t		* src_mdl	// pointer to source MDL
);

///////////////////////////////////////////////////////////////////////////////

bool HavePatchMDL();

uint PatchRawDataMDL
(
    // returns
    //	0: MDL not patched
    //  1: MDL patched
    //  3: MDL patched & vector transformation recognized

    void		* data_ptr,	// data to scan
    uint		data_size,	// size of 'data'
    patch_file_t	brres_type,	// type of related BRRES file
    ccp			fname		// file name for error messages
);

float3 TransformByMatrix
(
    float3  *val,
    float   m[3][4]
);

///////////////////////////////////////////////////////////////////////////////

int CheckMDL
(
    // returns number of errors

    const mdl_t		* mdl,		// pointer to valid MDL
    CheckMode_t		mode		// print mode
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef enumError (*iterate_mdl_func)
(
    mdl_t		* mdl,		// MDL data structure
    void		* param		// a user defined parameter
);

//-----------------------------------------------------------------------------

enumError IterateRawDataMDL
(
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		check_mode,	// not NULL: call CheckMDL(mode)
    iterate_mdl_func	func,		// call this function for each found MDL
    void		* param		// a user defined parameter
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 0			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_sect0_t]]

typedef struct mdl_sect0_t
{
    const u8	*data_beg;	// begin of data
    const u8	*data_end;	// end of data
    uint	data_size;	// calculated data size
    uint	n;		// number of elements including 'END'
    bool	ok;		// data ok

    const u8	*cur;		// NULL or pointer of current entry
    uint	cur_size;	// size of current entry

    uint	sub_size;	// size of each subelement
    uint	sub_n;		// if sub_size>0: number of sub elements.
    const u8	*sub_ptr;	// if sub_size>0: pointer to first sub element

    const u8	*fail;		// NULL or pointer to failed entry

} mdl_sect0_t;

//-----------------------------------------------------------------------------

bool InitializeMDLs0 ( mdl_sect0_t *ms, const void * data, uint data_size );
bool ClearMDLs0 ( mdl_sect0_t *ms );
bool CheckMDLs0 ( mdl_sect0_t *ms );
bool FirstMDLs0 ( mdl_sect0_t *ms );
bool NextMDLs0  ( mdl_sect0_t *ms );

ccp GetTypeMDLs0 ( uint index );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 1			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_sect1_t]]

typedef struct mdl_sect1_t
{
  /* 0x00 */  u32	length;			// length of struct, usually 0xd0
  /* 0x04 */  u32	mdl0_off;
  /* 0x08 */  u32	name_off;
  /* 0x0c */  u32	index;
  /* 0x10 */  u32 	id;
  /* 0x14 */  u32	flags;			// 0x31f typical
  /* 0x18 */  u32	padding[2];

  /* 0x20 */  float3	scale;			// scaling
  /* 0x2c */  float3	rotate;			// euler rotation in degree
  /* 0x38 */  float3	translate;		// translate == shift
  /* 0x44 */  float3	minimum;
  /* 0x50 */  float3	maximum;

  /* 0x5c */  u32	parent_off;		// parent offset
  /* 0x60 */  u32 	child_off;		// first child offset
  /* 0x64 */  u32	next_off;		// next sibling offset
  /* 0x68 */  u32 	prev_off;		// previous sibling offset
  /* 0x6c */  u32 	part2_off;		// part 2 offset

  /* 0x70 */  float34	trans_matrix;		// transformation matrix
  /* 0xa0 */  float34	inv_matrix;		// inverse transformation matrix
  /* 0xd0 */
}
__attribute__ ((packed)) mdl_sect1_t;

//-----------------------------------------------------------------------------

void ResetMDLs1 ( mdl_sect1_t *ms, bool network_order );
void ClearMDLs1 ( mdl_sect1_t *ms, bool network_order );
void ntoh_MDLs1 ( mdl_sect1_t *dest, const mdl_sect1_t *src );
void hton_MDLs1 ( mdl_sect1_t *dest, const mdl_sect1_t *src );
void CalcMatrixMDLs1 ( mdl_sect1_t *ms );

void ImportMatrixMDLs1
(
    mdl_sect1_t		*ms,		// valid and initialized data
    bool		network_order,	// true: data is in network byte order
    MatrixD_t		*mat,		// valid data, import from
    uint		import_mode	// bit field:
					//  1: import scale, rotate and translate
					//  2: import transformation matrix
					//  4: import inverse matrix
);

void ExportMatrixMDLs1
(
    mdl_sect1_t		*ms,		// valid and initialized data
    bool		network_order,	// true: data is in network byte order
    MatrixD_t		*mat,		// valid data, export to here
    uint		export_mode	// bit field:
					//  1: export scale, rotate and translate
					//  2: export trans matrix and declare it valid
					//  4: export inverse matrix and declare it valid
);

bool HaveStandardVectorsMDLs1 ( mdl_sect1_t *ms, bool network_order );

void DefineMatrixMDLs1
(
    // modify the vectors and calculate the matrices

    mdl_sect1_t		* ms1,		// pointer to valid MDLs1
    bool		network_order,	// true: data is in network byte order
    uint		reset,		// 0: do nothing
					// 1: reset only scale+rot+trans vectors
					// 2: reset all
    double		* scale,	// not NULL: multiply 3D scale to record
    double		* rotate,	// not NULL: add 3D rotation to record
    double		* translate	// not NULL: add 3D translation to record
);

void PrintMatrixMDLs1
(
    FILE		* f,		// file to print to
    uint		indent,		// indention
    ccp			eol,		// not NULL: print as 'end of line'
    const mdl_sect1_t	* ms1,		// pointer to valid MDLs1
    uint		mode		// bitfield:
					//   1: print scale+rot+trans vectors
					//   2: minimum + maximum
					//   4: print matrix
					//   8: print inverse matrix
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 2			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_sect2_t]]

typedef struct mdl_sect2_t
{
  /* 0x00 */  u32	length;		// length of section (includes vertex list)
  /* 0x04 */  u32	mdl0_off;
  /* 0x08 */  u32	data_off;
  /* 0x0c */  u32	name_off;
  /* 0x10 */  u32	index;
  /* 0x14 */  u32 	has_bounds;
  /* 0x18 */  u32	format;

  /* 0x1c */  u8	r_shift;
  /* 0x1d */  u8	stride;
  /* 0x1e */  u16	n_vertex;

  /* 0x20 */  float3	minimum;
  /* 0x2c */  float3	maximum;

  /* 0x38 */
}
__attribute__ ((packed)) mdl_sect2_t;

//-----------------------------------------------------------------------------

void ResetMDLs2 ( mdl_sect2_t *ms, bool network_order );
void ntoh_MDLs2 ( mdl_sect2_t *dest, const mdl_sect2_t *src );
void hton_MDLs2 ( mdl_sect2_t *dest, const mdl_sect2_t *src );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 8			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_sect8_t]]
// see https://wiki.tockdom.com/wiki/MDL0_%28File_Format%29#Section_8_-_Materials

typedef struct mdl_sect8_t
{
  /*  0x00 */  u32	length;			// length of section (includes vertex list)
  /*  0x04 */  u32	mdl0_off;
  /*  0x08 */  u32	name_off;
  /*  0x0c */  u32	index;
  /*  0x10 */  u32	xlu_material;
  /*  0x14 */  u8	tex_gens;
  /*  0x15 */  u8	light_channels;
  /*  0x16 */  u8	shader_stages;
  /*  0x17 */  u8	indirect_textures;
  /*  0x18 */  u32	cull_mode;
  /*  0x1c */  u8	alpha_func;
  /*  0x1d */  u8	lightset;
  /*  0x1e */  u8	fogset;
  /*  0x1f */  u8	unknown_1f;		// always 0
  /*  0x20 */  u32	unknown_20;		// always 0
  /*  0x24 */  u32	unknown_24;		// always ~0
  /*  0x28 */  u32	shader_off;
  /*  0x2c */  u32	n_layer;
  /*  0x30 */  u32	layer_off;
  /*  0x34 */  u32	part2_off;
  /*  0x38 */  u32	disp_list_off_8_9;
  /*  0x3c */  u32	disp_list_off_10_11;
  /*  0x40 */  u32	unknown_40;		// always 0

  /*  0x44 */  u8	any_data_44[0x418-0x44];
  /* 0x418 */
}
__attribute__ ((packed)) mdl_sect8_t;

///////////////////////////////////////////////////////////////////////////////
// [[mdl_sect8_layer_t]]

typedef struct mdl_sect8_layer_t
{
  /* 0x00 */  u32	name_off;
  /* 0x04 */  u8	any_data_04[0x20-0x04];
  /* 0x20 */  u32	min_filter;
  /* 0x24 */  u8	any_data_24[0x34-0x24];
  /* 0x34 */
}
__attribute__ ((packed)) mdl_sect8_layer_t;

///////////////////////////////////////////////////////////////////////////////

enum Slot42Material
{
    S42M_GOAL_MERG	= 0x0001,
    S42M_IWA		= 0x0002,
    S42M_IWA_ALFA	= 0x0004,
    S42M_NUKI_RYOUMEN	= 0x0008,
    S42M_WALLMERG00	= 0x0010,
    S42M_MOON_KABE0000	= 0x0020,
    S42M_MOON_ROAD00	= 0x0040,
    S42M_ROAD		= 0x0080,
    S42M_ROAD01		= 0x0100,
    S42M_ROAD02		= 0x0200,
    S42M_ROAD03		= 0x0400,
    S42M_SIBA00		= 0x0800,

    S42M__ALL		= 0x0fff,
    S42M__N		= 12
};

//-----------------------------------------------------------------------------
// [[Slot42MaterialInfo_t]]

typedef struct Slot42MaterialInfo_t
{
    u16			val;		// Slot42Material, bit field, 1 bit set
    const char		name[14];	// name of material
    const MemItem_t	*memitem;	// pointer to related meminfo of 'slot42_mdl'
}
Slot42MaterialInfo_t;

extern mdl_t slot42_mdl;
extern Slot42MaterialInfo_t Slot42MaterialInfo[S42M__N+1]; // {0,"",0} terminated
const Slot42MaterialInfo_t * SetupSlot42Materials();

//-----------------------------------------------------------------------------
// [[Slot42MaterialStat_t]]

typedef struct Slot42MaterialStat_t
{
    enum Slot42Material  found;		// bit field: materials found
    enum Slot42Material  modified;	// for 'found' content modified

    // summary
    bool		all_found;	// true: all matrials found
    bool		content_ok;	// true: no content modified
    bool		ok;		// true: all ok >= all_found && content_ok

} Slot42MaterialStat_t;

Slot42MaterialStat_t GetSlot42SupportSZS ( struct szs_file_t	*szs );

ccp RenameForSlot42MDL
(
    void		*rawdata,	// pointer to data, modified
    uint		size,		// size of data
    ContainerMDL_t	* cdata		// NULL or container-data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 10			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mdl_sect10_t]]

typedef struct mdl_sect10_t
{
  /* 0x00 */  u32	length;		// length of struct
  /* 0x04 */  u32	mdl0_off;	// mdl0 offset
  /* 0x08 */  u32 	bone_idx;	// bone index (for single bone ref,
					//	-1 for table usage)
  /* 0x0c */  u32 	cp_vtx;		// cp_vtx (0x4400 typical)
  /* 0x10 */  u32 	cp_tex;		// cp_tex (0x2 typical)
  /* 0x14 */  u32 	xf_nor_spec;	// xf_nor_spec (0x11 typical)
  /* 0x18 */  u32 	v_decl_size;	// vertex declaration size (0xe0)
  /* 0x1c */  u32 	flags;		// unknown flags (0x80 typical)
  /* 0x20 */  u32 	v_decl_offset;	// vertex declaration offset (0x68 typical)
  /* 0x24 */  u32 	v_data_size1;	// vertex data size
  /* 0x28 */  u32 	v_data_size2;	// vertex data size
  /* 0x2c */  u32 	v_data_offset;	// vertex data offset (0x13c typical)
  /* 0x30 */  u32 	xf_flags;	// xf_flags (0x2a00 typical)
  /* 0x34 */  u32 	unknown_0x34;	// unknown (0x0 typical)
  /* 0x38 */  u32 	name_off;	// name offset
  /* 0x3c */  u32 	idx;		// index
  /* 0x40 */  u32 	n_vertex;	// vertex count
  /* 0x44 */  u32 	n_face;		// face count
  /* 0x48 */  u16 	v_group;	// vertex group index
  /* 0x4a */  u16 	n_group;	// normal group index
  /* 0x4c */  u16	col_group[2];	// 2 color group indicies
  /* 0x50 */  u16	tex_group[8]; 	// 8 texture coordinate group indicies
  /* 0x60 */  u32 	unknown_0x60;	// unknown (-1 typical) (not seen in version 9 mdl0's)
  /* 0x64 */  u32 	bone_offset;	// bone table offset
  /* 0x68 */

} mdl_sect10_t;

//-----------------------------------------------------------------------------

void ResetMDLs10 ( mdl_sect10_t *ms, bool network_order );
void ntoh_MDLs10 ( mdl_sect10_t *dest, const mdl_sect10_t *src );
void hton_MDLs10 ( mdl_sect10_t *dest, const mdl_sect10_t *src );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL minimap			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct mdl_minimap_t
{
    mdl_sect1_t	*root;			// root section
    mdl_sect1_t	*posLD;			// section with name "posLD"
    mdl_sect1_t	*posRU;			// section with name "posRU"
    mdl_sect2_t	*vertex;		// vertex section

    float3	min;			// minimum vertex of section 2
    float3	max;			// maximum vertex of section 2

    bool	rec_trans_valid;	// true: recomendation is valid
    float3	rec_trans_ld;		// recommended translation for posLD
    float3	rec_trans_ru;		// recommended translation for posRU

} mdl_minimap_t;

//-----------------------------------------------------------------------------

bool FindMinimapData
(
    mdl_minimap_t	*mmap,		// minmap data to fill
    struct szs_file_t	*szs		// valid szs data
);

//-----------------------------------------------------------------------------

uint AutoAdjustMinimap
(
    // returns:
    //	0: nothing done
    //	1: minimap changed
    //  3: minimap changed & vertex list flattened

    struct szs_file_t	*szs		// valid SZS data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL0 string iterator		///////////////
///////////////////////////////////////////////////////////////////////////////

struct StringIteratorMDL_t;

typedef void (*SectionFuncMDL)
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    MemItem_t			*mi	// memory item of section
);

//-----------------------------------------------------------------------------

typedef void (*OffsetFuncMDL)
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
);

//-----------------------------------------------------------------------------

typedef void (*StringFuncMDL)
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u32				*sptr,	// pointer to string index
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
);

//-----------------------------------------------------------------------------

typedef struct StringIteratorMDL_t
{
    const mdl_t		*mdl;		// pointer to related MDL
    string_pool_t	*sp;		// string pool to manipulate

    FILE		*f;		// NULL or output file
    int			indent;		// indention of output

    SectionFuncMDL	sect_func;	// not NULL: call it for each section
    OffsetFuncMDL	off_func;	// not NULL: call it for each begin-mdl-offset
    StringFuncMDL	str_func;	// not NULL: call it for each string
}
StringIteratorMDL_t;

///////////////////////////////////////////////////////////////////////////////

void IterateStringsMDL
(
    StringIteratorMDL_t	*sit,		// string iterator data
    MemItem_t		*mi,		// pointer to memlist
    int			mi_count	// number of elements of 'ml_ptr'
);

//-----------------------------------------------------------------------------

void IterateAllStringsMDL
(
    StringIteratorMDL_t	*sit		// string iterator data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_MDL_H

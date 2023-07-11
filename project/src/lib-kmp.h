
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

// all about BMG files:
//	https://wiki.tockdom.de/index.php?title=BMG

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_KMP_H
#define SZS_LIB_KMP_H 1

#include "lib-std.h"
#include "lib-object.h"
#include "lib-lex.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define KMP_MAGIC		"RKMD"
#define KMP_MAGIC_NUM		0x524B4D44

#define KMP_TEXT_MAGIC		"#KMP"
#define KMP_TEXT_MAGIC_NUM	0x234b4d50

#define KMP_MAX_PH_LINK		  6
#define KMP_MAX_GROUP		255
#define KMP_MAX_POINT		255
#define KMP_MAX_NAME_LEN	 20

#define KMP_DIST_HINT		10.0
#define KMP_DIST_WARN		 1.0

#define GOBJ_ELINE_CONTROL	0x014
#define GOBJ_ICE		0x017
#define GOBJ_ITEMBOX		0x065
#define GOBJ_SUN_DS		0x072
#define GOBJ_COIN		0x073
#define	GOBJ_HEYHO_SHIP		0x0ce
#define	GOBJ_HEYHO_BALL		0x0ea
#define GOBJ_OBAKE_BLOCK	0x13c
#define GOBJ_PYLON01		0x144

#define KMP_COIN_IS_INVALID	50

//-----------------------------------------------------------------------------
//
// XPF ID map:
//
//  0000-03ff : Real objects
//  1000-13ff : Disabled objects
//
//  1000-1fff : reserved for predifined conditions
//		1000-102d : hard coded conditions
//		1100-18ff : >FREE (8 maps of 8 bits)
//		1900-1903 : developer conditions
//		1a00-1dff : >FREE (4 maps of 8 bits)
//		1e00-1e7f : engine classes
//		1f00-1fff : random scenarios
//
//  2000-3fff : Definition objects type BITS
//  4000-5fff : Definition objects type OR
//  6000-7fff : Definition objects type AND
//
//  8000-dfff : >FREE (6 maps of 12 bits)
//
//  e000-e3ff : test conditions (debug version only)
//
//  f000-ffff : >FREE (1 map of 12 bits)
//
//-----------------------------------------------------------------------------

// KMP/GOBJ/obj_id
#define GOBJ_M_OBJECT		0x03ff
#define GOBJ_M_FLAGS		( 0xffff & ~GOBJ_M_OBJECT )

#define GOBJ_BIT_LECODE		12	// maybe enabled by lecode
#define GOBJ_BIT_CONDITION	12	// indicator bit for predefinied condition

#define GOBJ_M_LECODE		(1<<GOBJ_BIT_LECODE)
#define GOBJ_M_CONDITION	(1<<GOBJ_BIT_CONDITION)

#define GOBJ_MIN_CONDITION	GOBJ_M_CONDITION
#define GOBJ_MAX_CONDITION	(GOBJ_M_CONDITION|0xfff)
#define GOBJ_DEV_CONDITION	(GOBJ_MIN_CONDITION+0x900)

#define GOBJ_M_DEFINITION	0xe000
#define GOBJ_MIN_DEF_BITS	0x2000
#define GOBJ_MAX_DEF_BITS	0x3fff
#define GOBJ_MIN_DEF_OR		0x4000
#define GOBJ_MAX_DEF_OR		0x5fff
#define GOBJ_MIN_DEF_AND	0x6000
#define GOBJ_MAX_DEF_AND	0x7fff
#define GOBJ_DEF_F_NOT		0x1000
#define GOBJ_MIN_DEF		GOBJ_MIN_DEF_BITS
#define GOBJ_MAX_DEF		(GOBJ_MAX_DEF_AND|GOBJ_DEF_F_NOT)

#define GOBJ_MASK_COND_ENGINE	0x7f
#define GOBJ_MIN_COND_ENGINE	(GOBJ_MIN_CONDITION+0xe00)
#define GOBJ_MAX_COND_ENGINE	(GOBJ_MIN_COND_ENGINE+GOBJ_MASK_COND_ENGINE)

#define GOBJ_MAX_RANDOM		8
#define GOBJ_MASK_COND_RND	0xff
#define GOBJ_MIN_COND_RND	(GOBJ_MIN_CONDITION+0xf00)
#define GOBJ_MAX_COND_RND	(GOBJ_MIN_COND_RND+GOBJ_MASK_COND_RND)

#define GOBJ_MASK_COND_TEST	0x03ff
#define GOBJ_MIN_COND_TEST	0xe000
#define GOBJ_MAX_COND_TEST	(GOBJ_MIN_COND_TEST+GOBJ_MASK_COND_TEST)

// KMP/GOBJ/presence_flags
#define GOBJ_M_PF_STD		0x003f	// mask for modes
#define GOBJ_M_PF_MODE		0xf000	// mask for modes
#define GOBJ_N_PF_MODE		16	// number of modes
#define GOBJ_S_PF_MODE		12	// shift count for modes

#define GOBJ_PF_MODE1		(1<<GOBJ_S_PF_MODE)

//-----------------------------------------------------------------------------

struct kmp_t;
struct kmp_ph_t;
struct kcl_t;
struct raw_data_t;
struct kmp_linfo_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp constants			///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    //----- auto modes

    KMP_AM_OFF = 0,	// default
    KMP_AM_NORM,	// normalize modes
    KMP_AM_RENUMBER,	// renumber  modes
    KMP_AM_1_LAP,	// define modes as 1 lap race
    KMP_AM_SHORT,	// define modes to get a very short race
    KMP_AM_UNLIMIT,	// remove lap counter for unlimited race

    //----- definition-object (DOB) sort modes

    KMP_DOB_OFF = 0,	// sort if off
    KMP_DOB_BEGIN,	// sort at beginning or before
    KMP_DOB_END,	// sort at end or behind
    KMP_DOB_BEFORE	= KMP_DOB_BEGIN,
    KMP_DOB_BEHIND	= KMP_DOB_END,
};

//----- default values for ROUTE-OBJECT

#define RTOBJ_1		0x12f	// o$castletree1
#define RTOBJ_2		0x163	// o$TownTreeDS
#define RTOBJ_BASE	-2000
#define RTOBJ_SCALE_X	 0.05
#define RTOBJ_SCALE_Y	12.00
#define RTOBJ_SCALE_Z	 0.20
#define RTOBJ_SCALE_MIN	 0.01

enum // bit numbers for 'show route'
{
    SHRT_ENPT = 0,
    SHRT_ITPT,
    SHRT_JGPT,
    SHRT_CNPT,
    SHRT_MSPT,
    SHRT_POTI,
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			auto-connect constants		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum kmp_ac_mode
{
    //--- modes

    KMP_AC_OFF		= 0,		// don't auto connect, always 0
    KMP_AC_PREV,			// auto connect PREV by NEXT always
    KMP_AC_AUTO_PREV,			// auto connect PREV by NEXT if $PREV not used
    KMP_AC_DISPATCH,			// auto connect for arenas, mode DISPATCH
     KMP_AC_MASK	= 0x0f,		// mask for modes

    //--- flags/options

    KMP_ACF_FIX_PREV	= 0x10,		// fix prev links
    KMP_ACF_FIX_NEXT	= 0x20,		// fix next links
    KMP_ACF_FIX		= KMP_ACF_FIX_PREV | KMP_ACF_FIX_NEXT,
    KMP_ACF_PR_PREV	= 0x40,		// print $PREV:...
     KMP_ACF_MASK	= 0x70,		// mask for flags
}
__attribute__ ((packed)) kmp_ac_mode;

//-----------------------------------------------------------------------------
// [[kmp_class_mode]]

typedef enum kmp_class_mode
{
    KCLS_OFF = 0,	// don't connect
    KCLS_ANY,		// allow any class except KCLS_OFF
    KCLS_DEFAULT,	// default class
    KCLS_USER,		// first user defined name
}
__attribute__ ((packed)) kmp_class_mode;

//-----------------------------------------------------------------------------
// [[kmp_oneway_mode]]

typedef enum kmp_oneway_mode
{
    ONEWAY_NONE = 0,	// one-way is off
    ONEWAY_PREV = 1,	// one-way only for prev links
    ONEWAY_NEXT = 2,	// one-way only for next links
    ONEWAY_BOTH = 3,	// one-way for prev and next links
}
__attribute__ ((packed)) kmp_oneway_mode;

extern const KeywordTab_t kmp_oneway_name[];

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum kmp_entry_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_entry_t]]

typedef enum kmp_entry_t
{
    KMP_KTPT,	//	Starting positions of racers
    KMP_ENPT,	//	Enemy route points
    KMP_ENPH,	//	Enemy route points header: Divide enemy points into groups
    KMP_ITPT,	//	Item route points
    KMP_ITPH,	//	Item points header: Divide item points into groups
    KMP_CKPT,	//	Check points
    KMP_CKPH,	//	Checkpoint header: Divide checkpoints into groups
    KMP_GOBJ,	// 'O',	Global objects
    KMP_POTI,	// 'R',	Routes
    KMP_AREA,	// 'A',	Camera and other areas
    KMP_CAME,	// 'C',	Cameras
    KMP_JGPT,	// 'J',	Respawn positions
    KMP_CNPT,	// 'N',	Cannon target positions
    KMP_MSPT,	// 'M',	End battle positions
    KMP_STGI,	// 'S',	Stage information

    KMP_N_SECT,		// number of regular KMP sections

    KMP_WIM0,		// extra section to export binary data (bzip2)
    KMP_NN_SECT,	// number of all KMP sections

    KMP_NO_SECT		// value for invalid section types

} kmp_entry_t;

extern const uint kmp_entry_size[KMP_N_SECT+1];
extern const KeywordTab_t kmp_section_name[];

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp_file_*_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_file_gen_t]] generic KMP file header

typedef struct kmp_file_gen_t
{
  /*0x00*/  char	magic[4];	// always KMP_MAGIC = "RKMD"
  /*0x04*/  u32		file_size;	// size of file
  /*0x08*/  u16		n_sect;		// number of sections
  /*0x0a*/  u16		head_size;	// header size, usually 0x4c
  /*0x0c*/  u32		version;	// KMP version number

  /*0x10*/  u32		sect_off[0];	// offsets of all sections,
					// relative to 'head_size'
}
__attribute__ ((packed)) kmp_file_gen_t;

static inline uint GetFileHeadSizeKMP ( uint n_sect )
	{ return sizeof(kmp_file_gen_t) + n_sect * sizeof(u32); }

///////////////////////////////////////////////////////////////////////////////
// [[kmp_file_mkw_t]] KMP file header as MKWii template

typedef struct kmp_file_mkw_t
{
  /*0x00*/  char	magic[4];	// always KMP_MAGIC = "RKMD"
  /*0x04*/  u32		file_size;	// size of file
  /*0x08*/  u16		n_sect;		// number of sections
  /*0x0a*/  u16		head_size;	// header size, usually 0x4c
  /*0x0c*/  u32		version;	// KMP version number

  /*0x10*/  u32		sect_off[KMP_N_SECT];
					// offsets of all sections,
					// relative to 'head_size'
  /*0x4c*/
}
__attribute__ ((packed)) kmp_file_mkw_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_file_wim0_t]] KMP file header as MKWii template with WIM= section

typedef struct kmp_file_wim0_t
{
  /*0x00*/  char	magic[4];	// always KMP_MAGIC = "RKMD"
  /*0x04*/  u32		file_size;	// size of file
  /*0x08*/  u16		n_sect;		// number of sections
  /*0x0a*/  u16		head_size;	// header size, usually 0x4c
  /*0x0c*/  u32		version;	// KMP version number

  /*0x10*/  u32		sect_off[KMP_N_SECT+1];
					// offsets of all sections,
					// relative to 'head_size'
  /*0x50*/
}
__attribute__ ((packed)) kmp_file_wim0_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_head_info_t]]

typedef struct kmp_head_info_t
{
    const u8		* data;		// data to scan
    const u8		* end;		// end of data
    uint		data_size;	// size of 'data'

    uint		file_size;	// size declared in header
    uint		max_size;	// := min(data_size,file_size)
    uint		head_size;	// size of header
    uint		max_off;	// max usable offset (end-4)
    int			n_sect;		// number of section, 0 on error
    u32			*sect_off;	// list mit section offsets
}
kmp_head_info_t;

void ScanHeadInfoKMP ( kmp_head_info_t *hi, cvp data, uint data_size );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp list & entries		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_list_head_t]]

typedef struct kmp_list_head_t
{
  /*0x00*/  char	magic[4];	// related string
  /*0x04*/  u16		n_entry;	// number of entries
  /*0x06*/  u16		value;		// addititional value
					//   POTI: total number of points

  /*0x08*/  u8		entry[0];	// entries
}
__attribute__ ((packed)) kmp_list_head_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_ktpt_entry_t]]

typedef struct kmp_ktpt_entry_t
{
  // Kart Points: Starting positions of racers.

  /*0x00*/  float32	position[3];	// start position
  /*0x0c*/  float32	rotation[3];	// start rotation
  /*0x18*/  s16		player_index;	// index or -1 for all players
  /*0x1a*/  u16		unknown;	// padding?
  /*0x1c*/
}
__attribute__ ((packed)) kmp_ktpt_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_enpt_entry_t]]  [[kmp_itpt_entry_t]]

typedef struct kmp_enpt_entry_t
{
  // Enemy Points: Routes of the cpu racers

  /*0x00*/  float32	position[3];	// start position
  /*0x0c*/  float32	scale;
  /*0x10*/  u16		prop[2];	// point properties
  /*0x14*/
}
__attribute__ ((packed)) kmp_enpt_entry_t;

// [[kmp_itpt_entry_t]]  Item Points: Routes of red shells and bullet bill
typedef kmp_enpt_entry_t kmp_itpt_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_enph_entry_t]]

typedef struct kmp_enph_entry_t
{
  // Enemy Points grouping: Route links of cpu racers

  /*0x00*/  u8		pt_start;
  /*0x01*/  u8		pt_len;
  /*0x02*/  u8		prev[KMP_MAX_PH_LINK];
  /*0x08*/  u8		next[KMP_MAX_PH_LINK];
  /*0x0e*/  u8		setting[2];		// only used for arena ENPT
  /*0x10*/
}
__attribute__ ((packed)) kmp_enph_entry_t;

// [[kmp_itph_entry_t]]  Item Points grouping: Route links of cpu racers
typedef kmp_enph_entry_t kmp_itph_entry_t;

// [[kmp_ckph_entry_t]]  Check point links
typedef kmp_enph_entry_t kmp_ckph_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_ckpt_entry_t]]

typedef struct kmp_ckpt_entry_t
{
  // Check Points

  /*0x00*/  float32	left[2];	// left hand position of checkpoint line
  /*0x08*/  float32	right[2];	// right hand position of checkpoint line
  /*0x10*/  u8		respawn;	// index into JGPT section
  /*0x11*/  u8		mode;		// 0:lap counter, -1:other, *:must cross
  /*0x12*/  u8		prev;		// previous checkpoint in sequence
  /*0x13*/  u8		next;		// next checkpoint in sequence
  /*0x14*/
}
__attribute__ ((packed)) kmp_ckpt_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_gobj_entry_t]]

typedef struct kmp_gobj_entry_t
{
  // Global objects

  /*0x00*/  u16		obj_id;		// object id
  /*0x02*/  u16		ref_id;		// reference id, padding in original MKWii
  /*0x04*/  float32	position[3];
  /*0x10*/  float32	rotation[3];
  /*0x1c*/  float32	scale[3];
  /*0x28*/  u16		route_id;
  /*0x2a*/  u16		setting[8];	// object setting
  /*0x3a*/  u16		pflags;		// presence flags
  /*0x3c*/
}
__attribute__ ((packed)) kmp_gobj_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_poti_group_t]]

typedef struct kmp_poti_group_t
{
  // Routes

  /*0x00*/  u16		n_point;	// number of related points
  /*0x02*/  u8		smooth;		// 1: enable smooth moving
  /*0x03*/  u8		back;		// 1: enable forward+backward
  /*0x04*/
}
__attribute__ ((packed)) kmp_poti_group_t;

//-----------------------------------------------------------------------------
// [[kmp_poti_point_t]]

typedef struct kmp_poti_point_t
{ 
  // Routes

  /*0x00*/  float32	position[3];
  /*0x0c*/  u16		speed;		// speed in 60pt/s
  /*0x0e*/  u16		unknown;	// Nintendo: 0 (~94%), 1,2,3,4,5,6,9,12
  /*0x10*/
}
__attribute__ ((packed)) kmp_poti_point_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_area_entry_t]]

typedef struct kmp_area_entry_t
{
  // Camera and other areas

  /*0x00*/  u8		mode;		// mode of area
  /*0x01*/  u8		type;		// type of area
  /*0x02*/  u8		dest_id;	// index into CAME
  /*0x03*/  u8		prio;		// higher number => higher priority
  /*0x04*/  float32	position[3];
  /*0x10*/  float32	rotation[3];
  /*0x1c*/  float32	scale[3];
  /*0x28*/  u16		setting[2];
  /*0x2c*/  u8		route;
  /*0x2d*/  u8		enemy;
  /*0x2e*/  u16		unknown_2e;
  /*0x30*/
}
__attribute__ ((packed)) kmp_area_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_came_entry_t]]

typedef struct kmp_came_entry_t
{
  // Cameras

  /*0x00*/  u8		type;		// type of camera
  /*0x01*/  u8		next;		// next camera
  /*0x02*/  u8		unknown_0x02;
  /*0x03*/  u8		route;		// route index -> POTI
  /*0x04*/  u16		came_speed;	// unsure!
  /*0x06*/  u16		zoom_speed;	// zoom speed in units per 100/60 sec
  /*0x08*/  u16		viewpt_speed;	// view point speed in units per 100/60 sec
  /*0x0a*/  u16		unknown_0x0a;
  /*0x0c*/  float32	position[3];	// camera position
  /*0x18*/  float32	rotation[3];	// rotation? (always 0)
  /*0x24*/  float32	zoom_begin;	// zoom value begin
  /*0x28*/  float32	zoom_end;	// zoom value end
  /*0x2c*/  float32	viewpt_begin[3];// view point begin
  /*0x38*/  float32	viewpt_end[3];	// view point end
  /*0x44*/  float32	time;		// active time of the camera
  /*0x48*/
}
__attribute__ ((packed)) kmp_came_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_jgpt_entry_t]]

typedef struct kmp_jgpt_entry_t
{
  // Jugem points: Respawn positions

  /*0x00*/  float32	position[3];
  /*0x0c*/  float32	rotation[3];
  /*0x18*/  s16		id;
  /*0x1a*/  u16		effect;
  /*0x1c*/
}
__attribute__ ((packed)) kmp_jgpt_entry_t;

// [[kmp_cnpt_entry_t]]  Cannon Points: Cannon target positions
typedef kmp_jgpt_entry_t kmp_cnpt_entry_t;

// [[kmp_mspt_entry_t]]
// After battles and competitions have ended the players are placed on this point(s)
typedef kmp_jgpt_entry_t kmp_mspt_entry_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_stgi_entry_t]]

typedef struct kmp_stgi_entry_t
{
  //  Stage Information

  /*0x00*/  u8		lap_count;		// XML suggest, used by lap_modifier
  /*0x01*/  u8		pole_pos;		// 0:left, 1:right
  /*0x02*/  u8		narrow_start;		// 0:normal, 1:closer together
  /*0x03*/  u8		enable_lens_flare;	// 0:off, 1:enabled
  /*0x04*/  u32		flare_color;		// 0xe6e6e6 or 0xffffff
  /*0x08*/  u8		flare_alpha;		// alpha channel (transparency)
  /*0x09*/  u8		unknown_09;		// always 0
  /*0x0a*/  u16		speed_mod;		// nintendo: always 0
						// custom: MSB of speed modifier
  /*0x0c*/
}
__attribute__ ((packed)) kmp_stgi_entry_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP sort modes			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_sort_t]]

typedef enum kmp_sort_t
{
    KSORT_OFF,		// don't sort
    KSORT_GROUP,	// sort by groups
    KSORT_ANGLE,	// sort by groups and angles
    KSORT_XYZ,		// sort by groups and coordonates in xyz order
    KSORT_TINY,		// sort for TINY
}
kmp_sort_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Wim0 support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_wim0_sect_id]]

typedef enum kmp_wim0_sect_id
{
    W0ID_END		= 0,
    W0ID_VERSION	= 'V' << 8 | 'e',
//    W0ID_AUTO_CONNECT	= 'A' << 8 | 'C',
}
kmp_wim0_sect_id;

extern const KeywordTab_t Wim0SectionName[];

///////////////////////////////////////////////////////////////////////////////
// [[kmp_wim0_format_t]]

typedef enum kmp_wim0_format_t
{
    W0F_UNKNOWN = 0,
    W0F_BINARY,
    W0F_TEXT,
    W0F__N
}
kmp_wim0_format_t;

extern ccp Wim0FormatName[];

///////////////////////////////////////////////////////////////////////////////
// [[kmp_wim0_t]]

typedef struct kmp_wim0_t
{
    u8			*data;		// uncompressed data, alloced
    uint		used;		// number of used bytes
    uint		size;		// number of alloced bytes

    u8			*bz2data;	// NULL or bzip2 data, alloced
    uint		bz2size;	// size of bz2data
}
__attribute__ ((packed)) kmp_wim0_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_wim0_sect_t]]

typedef struct kmp_wim0_sect_t
{
    u16			id;	// one of kmp_wim0_sect_id
    u16			words;	// number of words (2=bytes) of data
    u16			data[];	// data, always ALIGN(2)
}
__attribute__ ((packed)) kmp_wim0_sect_t;

///////////////////////////////////////////////////////////////////////////////
// [[kmp_wim0_info_t]]

typedef struct kmp_wim0_info_t
{
    kmp_wim0_sect_id	sect_id;	// one of kmp_wim0_sect_id
    uint		index;		// 0..N: Index of section
    uint		offset;		// Offset relative to first WIM0 section
    char		name[8];	// either key->name1 or "0x####"
    kmp_wim0_format_t	format;		// format of data, fallback to W0F_UNKNOWN

    const
      kmp_wim0_sect_t	*sect;		// pointer to section
    const u8		*data;		// pointer to section data
    uint		size;		// size of section data

}
__attribute__ ((packed)) kmp_wim0_info_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeWim0 ( kmp_wim0_t *w0 );
void ResetWim0 ( kmp_wim0_t *w0 );
void * NeedWim0 ( kmp_wim0_t *w0, uint size, uint align );
enumError CompressWim0 ( kmp_wim0_t *w0 );
enumError DecompressWim0 ( kmp_wim0_t *w0 );

kmp_wim0_sect_t * AppendSectionWim0
(
    kmp_wim0_t		*w0,		// data structure
    kmp_wim0_sect_id	sect_id,	// section id
    cvp			data,		// pointer to data
    uint		size		// size of data in bytes
);

bool GetFirstSectionWim0
(
    kmp_wim0_info_t	*info,		// store result here (never NULL)
    const struct kmp_t	*kmp		// pointer to valid KMP
);

bool GetNextSectionWim0
(
    kmp_wim0_info_t	*info,		// update result (never NULL)
    const struct kmp_t	*kmp		// pointer to valid KMP
);

const kmp_wim0_sect_t * FindSectionWim0
(
    // return NULL or a pointer to found kmp_wim0_sect_t

    kmp_wim0_info_t	*info,		// not NULL: store extended result here
    const struct kmp_t	*kmp,		// pointer to valid KMP
    kmp_wim0_sect_id	sect_id		// section id to search
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp_section_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_section_t]]

typedef struct kmp_section_t
{
    int			sect;	// -1: invalid, -2:unknown, else: kmp_entry_t
    uint		index;	// index of analyse section
    kmp_list_head_t	*ptr;	// pointer to section
    uint		size;	// size of section
}
kmp_section_t;

//-----------------------------------------------------------------------------

bool AnalyseSectionKMP
(
    // return true if known section found

    kmp_section_t	*ks,	// store result here
    const void		*data,	// KMP raw data to scan
    uint		size,	// size of 'data'
    uint		index	// index of section to analyse
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			route types			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_rtype_mode]]

typedef enum kmp_rtype_mode
{
    KMP_RT_AUTO,	// route type detected automatically
    KMP_RT_ROUTE,	// is real route
    KMP_RT_DISPATCH,	// is dispatch point

}
__attribute__ ((packed)) kmp_rtype_mode;

extern const KeywordTab_t kmp_rtype_name[];

//-----------------------------------------------------------------------------
// [[kmp_rtype_info_t]]

typedef struct kmp_rtype_info_t
{
    // section type
    uint	sect_ph;	// PH section index
    uint	sect_pt;	// PT section index

    // basic stats
    uint	n_group;	// number of route groups
    uint	n_point;	// number of route points

    // analysis
    uint	n_route0;	// number of routes with 0 points (ERROR)
    uint	n_route1;	// number of real routes with only one points
    uint	n_route2;	// number of real routes with >1 points
    uint	n_dispatch;	// number of dispatch points (=not real route)
}
kmp_rtype_info_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  kmp_group_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_group_name_t]]

typedef char kmp_group_name_t[KMP_MAX_NAME_LEN+1];

//-----------------------------------------------------------------------------
// [[kmp_gopt_t]]

typedef struct kmp_gopt_t
{
    u8			rclass;		// route class (for incomming connections)
    u8			lclass;		// link class (for outgoing connections)
    u8			min_links;	// minimal links to auto-add
    u8			max_links;	// maxmal links to auto-add
    double		distance;	// if >0: max distance for N>min
}
kmp_gopt_t;

//-----------------------------------------------------------------------------
// [[kmp_gopt2_t]]

typedef struct kmp_gopt2_t
{
    kmp_rtype_mode	rtype_user;	// route type set by user
    kmp_rtype_mode	rtype_active;	// active route type
    kmp_oneway_mode	ac_oneway;	// oneway mode for auto-connect
    u8			setting[2];	// values of bytes at offeset 0x0e and 0x0f
//    u8			have_prev_cmd;	// >0: $PREV command used

    kmp_gopt_t		prev;		// values for first point (prev link)
    kmp_gopt_t		next;		// values for last point (next link)
}
kmp_gopt2_t;

//-----------------------------------------------------------------------------
// [[kmp_group_elem_t]]

typedef struct kmp_group_elem_t
{
    kmp_group_name_t	name;		// name of element
    kmp_oneway_mode	oneway;		// oneway mode

} kmp_group_elem_t;

//-----------------------------------------------------------------------------
// [[kmp_group_t]]

typedef struct kmp_group_t
{
    int			first_index;		// index of first point
    int			last_index;		// index of last point
    bool		no_rotate;		// true: do not rotate links
    bool		have_prev_cmd;		// >0: $PREV command scannend

    kmp_group_name_t	name;			// name of group
    kmp_group_elem_t	prev[KMP_MAX_PH_LINK];	// prev-link names and status
    kmp_group_elem_t	next[KMP_MAX_PH_LINK];	// next-link names and status

} kmp_group_t;

//-----------------------------------------------------------------------------
// [[kmp_group_list_t]]

typedef struct kmp_group_list_t
{
    struct kmp_t	*kmp;			// related kmp, never NULL
    struct kmp_ph_t	*ph;			// related ph info, never NULL

    uint		used;			// used groups
    kmp_group_t		group[KMP_MAX_GROUP];	// list with groups
    char		selector;		// '!' selector
    bool		have_options;		// >0_ at least one option detected
    Line_t		line;			// '$LINE' settings

} kmp_group_list_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		kmp_rtobj_t, kmp_rtobj_list_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_rtobj_t]]

typedef struct kmp_rtobj_t
{
    u16			obj;			// object to use
    float3		shift;			// add to position
    float3		scale;			// store as 'scale' factor
    float3		rotate;			// add to rotation

} kmp_rtobj_t;

//-----------------------------------------------------------------------------
// [[kmp_rtobj_list_t]]

typedef struct kmp_rtobj_list_t
{
    kmp_rtobj_t		rtobj;			// object to use
    u16			index;			// index to POTI group

} kmp_rtobj_list_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 kmp_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_mode_t]]

typedef enum kmp_mode_t
{
    //--- general options

    KMPMD_FORCE		= 0x00000001,	// like --force: accept little repairable issues
     KMPMD_M_GENERAL	 = KMPMD_FORCE,


    //--- KMP patching options

    KMPMD_NEW		= 0x00000010,	// create a new KMP file even if nothing changed
    KMPMD_RM_SPCITEM	= 0x00000020,	// remove special items
    KMPMD_REVERSE	= 0x00000040,	// reverse the track order
    KMPMD_MAX_LAPS	= 0x00000080,	// set LAPS only if larger

    KMPMD_1_LAP		= 0x00000100,	// set lap count to #1
    KMPMD_2_LAPS	= 0x00000200,	// set lap count to #2
    KMPMD_3_LAPS	= 0x00000300,	// set lap count to #3
    KMPMD_4_LAPS	= 0x00000400,	// set lap count to #4
    KMPMD_5_LAPS	= 0x00000500,	// set lap count to #5
    KMPMD_6_LAPS	= 0x00000600,	// set lap count to #6
    KMPMD_7_LAPS	= 0x00000700,	// set lap count to #7
    KMPMD_8_LAPS	= 0x00000800,	// set lap count to #8
    KMPMD_9_LAPS	= 0x00000900,	// set lap count to #9
     KMPMD_M_LAPS	= 0x00000f00,
     KMPMD_SHIFT_LAPS	= 8,

    KMPMD_POLE_LEFT	= 0x00001000,	// set pole position to left side
    KMPMD_POLE_RIGHT	= 0x00002000,	// set pole position to right side
    KMPMD_SP_WIDE	= 0x00004000,	// set start mode 'wide'
    KMPMD_SP_NARROW	= 0x00008000,	// set start mode 'narrow'
     KMPMD_M_POLE	 = KMPMD_POLE_LEFT
			 | KMPMD_POLE_RIGHT,
     KMPMD_M_STARTPOS	 = KMPMD_SP_WIDE
			 | KMPMD_SP_NARROW,
     KMPMD_M_STGI	 = KMPMD_M_POLE
			 | KMPMD_M_STARTPOS
			 | KMPMD_M_LAPS,

    KMPMD_FIX_CKPH	= 0x00010000,	// automatic calculations of section CKPH
    KMPMD_FIX_ENPH	= 0x00020000,	// automatic calculations of section ENPH
    KMPMD_FIX_ITPH	= 0x00040000,	// automatic calculations of section ITPH
    KMPMD_FIX_CKNEXT	= 0x00080000,	// CKPT: auto calc of 'prev' and 'next'
    KMPMD_FIX_CKJGPT	= 0x00100000,	// CKPT: fix invalid JGPT indices
     KMPMD_M_FIX_PH	 = KMPMD_FIX_CKPH
			 | KMPMD_FIX_ENPH
			 | KMPMD_FIX_ITPH,
     KMPMD_M_FIX_CK	 = KMPMD_FIX_CKPH
			 | KMPMD_FIX_CKNEXT
			 | KMPMD_FIX_CKJGPT,
     KMPMD_M_FIX_ALL	 = KMPMD_M_FIX_PH
			 | KMPMD_M_FIX_CK,

    KMPMD_MASK_PFLAGS	= 0x00200000,	// mask presence flags by 0x3f & clear padding
    KMPMD_RM_LECODE	= 0x00400000,	// remove LE-CODE objects of section GOBJ
    KMPMD_PURGE_GOBJ	= 0x00800000,	// remove invalid objects of section GOBJ except def-obj
    KMPMD_FULL_DEFOBJ	= 0x01000000,	// Display definitions objects as full object
     KMPMD_M_XPF	= KMPMD_MASK_PFLAGS
			| KMPMD_RM_LECODE
			| KMPMD_PURGE_GOBJ
			| KMPMD_FULL_DEFOBJ,

    //--- dump options

    KMPMD_DUMP_CLASS	= 0x02000000,	// dump class list at end of section if not empty
    KMPMD_DUMP_ONEWAY	= 0x04000000,	// dump oneway list at end of section if not empty
     KMPMD_M_DUMP_ALL	= KMPMD_DUMP_CLASS
			| KMPMD_DUMP_ONEWAY,


    //--- hidden options

    KMPMD_LOG		= 0x08000000,	// enable kmp trace log to stdout
    KMPMD_SILENT	= 0x10000000,	// disable logging of patching operations
    KMPMD_INPLACE	= 0x20000000,	// force inplace patching
     KMPMD_M_HIDDEN	 = KMPMD_LOG
			 | KMPMD_SILENT
			 | KMPMD_INPLACE,

    KMPMD_F_HIDE	= 0x40000000,	// help flag to hide


    //--- tiny modes

    KMPMD_S_TINY	= 32,
    KMPMD_TINY_0	= 0ull << KMPMD_S_TINY,	// TINY levels
    KMPMD_TINY_1	= 1ull << KMPMD_S_TINY,
    KMPMD_TINY_2	= 2ull << KMPMD_S_TINY,
    KMPMD_TINY_3	= 3ull << KMPMD_S_TINY,
    KMPMD_TINY_4	= 4ull << KMPMD_S_TINY,
    KMPMD_TINY_5	= 5ull << KMPMD_S_TINY,
    KMPMD_TINY_6	= 6ull << KMPMD_S_TINY,
    KMPMD_TINY_7	= 7ull << KMPMD_S_TINY,
     KMPMD_M_TINY	 = KMPMD_TINY_7,

    KMPMD_RM_EMPTY	= KMPMD_TINY_1 << 3,	// skip empty KMP sections
    KMPMD_MINIMIZE	= KMPMD_RM_EMPTY << 1,	// minimize KMP by reordering [[obsolete]] not used


    //--- misc

    KMPMD_PR_PREV	= KMPMD_MINIMIZE << 1,	// print $PREV: ... (CKPT,ENPT,ITPT)
     KMPMD_M_MISC	 = KMPMD_PR_PREV,


    //--- tests

    KMPMD_TEST1		= KMPMD_PR_PREV << 1,	// reserved for testing
    KMPMD_TEST2		= KMPMD_TEST1 << 1,	// reserved for testing
     KMPMD_M_TEST	 = KMPMD_TEST1
			 | KMPMD_TEST2,


    //--- summaries

    KMPMD_M_DEFAULT	= 0,			// default settings

    KMPMD_M_PATCH	= KMPMD_NEW		// patching flags
			| KMPMD_RM_SPCITEM
			| KMPMD_REVERSE
			| KMPMD_MAX_LAPS
			| KMPMD_M_STGI
			| KMPMD_M_FIX_ALL
			| KMPMD_M_XPF
			| KMPMD_M_TINY
			| KMPMD_RM_EMPTY
			| KMPMD_MINIMIZE
			| KMPMD_M_MISC
			| KMPMD_M_TEST,

    KMPMD_M_ALL		= KMPMD_M_GENERAL	// all relevant bits
			| KMPMD_M_PATCH
			| KMPMD_M_DUMP_ALL
			| KMPMD_M_HIDDEN,

    KMPMD_M_PRINT	= KMPMD_M_ALL		// all relevant bits for obj output
			& ~KMPMD_M_HIDDEN,
}
kmp_mode_t;

//-----------------------------------------------------------------------------
// [[kmp_diff_mode_t]]

typedef enum kmp_diff_mode_t
{
    KDMD_NONE		= 0,

    KDMD_KTPT		= 1 << KMP_KTPT,
    KDMD_ENPT		= 1 << KMP_ENPT,
    KDMD_ENPH		= 1 << KMP_ENPH,
    KDMD_ITPT		= 1 << KMP_ITPT,
    KDMD_ITPH		= 1 << KMP_ITPH,
    KDMD_CKPT		= 1 << KMP_CKPT,
    KDMD_CKPH		= 1 << KMP_CKPH,
    KDMD_GOBJ		= 1 << KMP_GOBJ,
    KDMD_POTI		= 1 << KMP_POTI,
    KDMD_AREA		= 1 << KMP_AREA,
    KDMD_CAME		= 1 << KMP_CAME,
    KDMD_JGPT		= 1 << KMP_JGPT,
    KDMD_CNPT		= 1 << KMP_CNPT,
    KDMD_MSPT		= 1 << KMP_MSPT,
    KDMD_STGI		= 1 << KMP_STGI,
    KDMD_M_SECTION	= (1<<KMP_N_SECT)-1,

    KDMD_SORT		= 1 << KMP_N_SECT,
    KDMD_NOSORT		= KDMD_SORT << 1,
    KDMD_M_SORT		= KDMD_SORT|KDMD_NOSORT,

    KDMD_M_DEFAULT	= KDMD_M_SECTION	// default setting
			| KDMD_SORT,

    KDMD_M_GOOD		= KDMD_CKPH		// good setting (fully implemented)
			| KDMD_CNPT
			| KDMD_ENPH
			| KDMD_GOBJ
			| KDMD_ITPH
			| KDMD_JGPT
			| KDMD_MSPT
			| KDMD_M_SORT,

    KDMD_M_ALL		= KDMD_M_DEFAULT
			| KDMD_M_SORT,

    KDMD_F_HIDE		= 0x40000000,		// help flag to hide
}
kmp_diff_mode_t;

//-----------------------------------------------------------------------------
// [[kmp_tform_t]]

typedef enum kmp_tform_t
{
    //--- single modes

    KMPTF_AREA_POS	= 0x000001,
    KMPTF_AREA_ROTATE	= 0x000002,
    KMPTF_AREA_SCALE	= 0x000004,

    KMPTF_CAME_POS	= 0x000008,

    KMPTF_CKPT_POS	= 0x000010,

    KMPTF_CNPT_POS	= 0x000020,
    KMPTF_CNPT_ROTATE	= 0x000040,

    KMPTF_ENPT_POS	= 0x000080,
    KMPTF_ENPT_SCALE	= 0x000100,

    KMPTF_GOBJ_POS	= 0x000200,
    KMPTF_GOBJ_ROTATE	= 0x000400,
    KMPTF_GOBJ_SCALE	= 0x000800,

    KMPTF_ITPT_POS	= 0x001000,
    KMPTF_ITPT_SCALE	= 0x002000,

    KMPTF_JGPT_POS	= 0x004000,
    KMPTF_JGPT_ROTATE	= 0x008000,

    KMPTF_KTPT_POS	= 0x010000,
    KMPTF_KTPT_ROTATE	= 0x020000,

    KMPTF_MSPT_POS	= 0x040000,
    KMPTF_MSPT_ROTATE	= 0x080000,

    KMPTF_POTI_POS	= 0x100000,


    //--- section summaries

    KMPTF_AREA		= KMPTF_AREA_POS | KMPTF_AREA_ROTATE | KMPTF_AREA_SCALE,
    KMPTF_CAME		= KMPTF_CAME_POS,
    KMPTF_CKPT		= KMPTF_CKPT_POS,
    KMPTF_CNPT		= KMPTF_CNPT_POS | KMPTF_CNPT_ROTATE,
    KMPTF_ENPT		= KMPTF_ENPT_POS | KMPTF_ENPT_SCALE,
    KMPTF_GOBJ		= KMPTF_GOBJ_POS | KMPTF_GOBJ_ROTATE | KMPTF_GOBJ_SCALE,
    KMPTF_ITPT		= KMPTF_ITPT_POS | KMPTF_ITPT_SCALE,
    KMPTF_JGPT		= KMPTF_JGPT_POS | KMPTF_JGPT_ROTATE,
    KMPTF_KTPT		= KMPTF_KTPT_POS | KMPTF_KTPT_ROTATE,
    KMPTF_MSPT		= KMPTF_MSPT_POS | KMPTF_MSPT_ROTATE,
    KMPTF_POTI		= KMPTF_POTI_POS,


    //--- masks

    KMPTF_M_ALL		= 0x1fffff,
    KMPTF_M_DEFAULT	= KMPTF_M_ALL,

    //--- flags

    KMPTF_F_HIDE	= 0x1000000

} kmp_tform_t;

//-----------------------------------------------------------------------------
// [[kmp_pt_flag_t]]

typedef enum kmp_pt_flag_t
{
	KPFL_FALL	= 0x01,  // F: fall-down enabled
	KPFL_SNAP	= 0x02,  // S: used for SNAP
	KPFL_JGPT	= 0x04,  // J: used for automatic JGPT creation

	KPFL_M_ENPT	= KPFL_FALL
			| KPFL_SNAP
			| KPFL_JGPT,

	KPFL_M_ITPT	= KPFL_FALL
			| KPFL_JGPT,

	KPFL_M_JGPT	= KPFL_FALL
			| KPFL_SNAP,

	KPFL_M_ALL	= KPFL_M_ENPT
			| KPFL_M_ITPT
			| KPFL_M_JGPT,
}
__attribute__ ((packed)) kmp_pt_flag_t;


//--- export of the flags

#define KMP_FE_MASK		0x000000ff
#define KMP_FE_ALWAYS_0		0x000000e0
#define KMP_FE_ALWAYS_1		0x00000011
#define KMP_FE_FLAGS		0x0000000e
#define KMP_FE_FLAGS_SHIFT	1

//-----------------------------------------------------------------------------

extern kmp_mode_t	KMP_MODE;
extern kmp_diff_mode_t	KMP_DIFF_MODE;
extern kmp_tform_t	KMP_TFORM;

extern bool		speed_mod_active;
extern float		speed_mod_factor;
extern u16		speed_mod_val;

int  ScanOptKmp ( ccp arg );
ccp  GetKmpMode();
void SetupKMP();

int  ScanOptKmpDiff ( ccp arg );
ccp  GetKmpDiffMode();

int  ScanOptTformKmp ( ccp arg );
uint PrintKmpTform( char *buf, uint bufsize, kmp_tform_t mode );
ccp  GetKmpTform();

int   ScanOptSpeedMod ( ccp arg );
int   ScanOptKtpt2 ( ccp arg );
u16   SpeedMod2u16 ( float speed );
float SpeedMod2float ( u16 hex );

struct kmp_t;
bool KMP_ACTION_LOG ( const struct kmp_t *kmp, bool is_patch, const char * format, ... )
	__attribute__ ((__format__(__printf__,3,4)));

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct kmp_ph_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_ph_t]] // special settings for CKPH, ENPH and ITPH

typedef struct kmp_ph_t
{
    // options
    uint		sect_ph;		// section index PH
    uint		sect_pt;		// section index PT
    u8			show_options;		// >0: enabled
    kmp_class_mode	default_class;		// default class
    kmp_ac_mode		ac_mode;		// one of KMP_AC_*
    kmp_ac_mode		ac_flags;		// bitfield of KMP_ACF_*
    u16			mask[2];		// (ENPH+ITPH only)

    // oneway support
    uint		oneway_used;		// number of used elements
    uint		oneway_size;		// number of alloced elements
    u16			*oneway_list;		// list of elements (from<<8|to)

    // lists
    ParamField_t	class_name;		// list with class names
    kmp_gopt2_t		gopt[KMP_MAX_GROUP];	// group options (ENPH+ITPH only)

    // [[obsolete]] -> replaced by 'kmp_t::index'
    kmp_group_name_t	gname[KMP_MAX_GROUP];	// group names of check points
}
kmp_ph_t;

//-----------------------------------------------------------------------------

void InitializePH ( kmp_ph_t *ph, uint sect_ph, uint sect_pt, uint ac_default );
void ResetPH ( kmp_ph_t *ph );

void InsertOnewayPH (       kmp_ph_t *ph, u8 from, u8 to );
bool FindOnewayPH   ( const kmp_ph_t *ph, u8 from, u8 to );
void SortOnewayPH   ( const kmp_ph_t *ph );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct kmp_flag_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_flag_t]] // special settings for ENPT, ITPH and JGPT

typedef struct kmp_flag_t
{
    u8			import;			// >0: flags imported
    u8			export;			// >0: flag export enabled

    u8			mask;			// any of KPFL_M_*
    u8			flags[KMP_MAX_POINT];	// kmp_pt_flag_t
}
kmp_flag_t;

//-----------------------------------------------------------------------------

void InitializeKMPFlags ( kmp_flag_t * kf, u8 mask );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    kmp_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_t]]

typedef struct kmp_t
{
    //--- base info

    ccp			fname;			// alloced filename of loaded file
    FileAttrib_t	fatt;			// file attribute
    file_format_t	fform;			// FF_KMP | FF_KMP_TXT
    struct lex_info_t	*lexinfo;		// NULL or valid LEX info

    //--- error control: if >0: suppress warnings for ...

    int			nowarn_format;		// .. invalid file formats
    bool		check_only;		// true: analysing or checking the file

    //--- data

    List_t		dlist[KMP_N_SECT];	// data lists
    List_t		poti_point;		// POTI points list

    int			revision;		// tool revision
    u32			kmp_version;		// unknown value if file header
    u16			value[KMP_N_SECT+1];	// 'value' of section header
    IsArena_t		battle_mode;		// one of ARENA_*
    IndexList_t		index[KMP_N_SECT];	// named indices
    warn_bits_t		warn_bits;		// warning summary

    char		* info;			// string behind last section
    const u8		* max_scanned;		// raw scanning: max scanned area

    //--- wim0 support

    kmp_wim0_sect_t	*wim0_data;		// NULL or (decompressed) data
    uint		wim0_size;		// size of wim0_data in bytes
    uint		wim0_import_size;	// 0 or total size of found WIM0 section
    u8			wim0_import_bz2;	// >0: found WIM0 section was bz2'ipped.
    u8			wim0_export;		// >0: create section WIM0 on export

    //--- support for PH sections

    kmp_ph_t		ckph;
    kmp_ph_t		enph;
    kmp_ph_t		itph;

    //--- flags support

    kmp_flag_t		enpt_flag;
    kmp_flag_t		itpt_flag;
    kmp_flag_t		jgpt_flag;

    //--- snap variables: if not VAR_UNSET
    //		=> move each JGPT to next ENPT and and snap value
    //--- priority: 'snap2' before 'snap' before 'hsnap'

    Var_t		jgpt_snap2;		// not VAR_UNSET: snap2 JGPT
    Var_t		cnpt_snap2;		// not VAR_UNSET: snap2 CNPT
    Var_t		mspt_snap2;		// not VAR_UNSET: snap2 MSPT
    Var_t		jgpt_snap;		// not VAR_UNSET: snap JGPT
    Var_t		cnpt_snap;		// not VAR_UNSET: snap CNPT
    Var_t		mspt_snap;		// not VAR_UNSET: snap MSPT
    Var_t		jgpt_hsnap;		// not VAR_UNSET: hsnap JGPT
    Var_t		cnpt_hsnap;		// not VAR_UNSET: hsnap CNPT
    Var_t		mspt_hsnap;		// not VAR_UNSET: hsnap MSPT

    //--- route objects (insert objects into KMP to show routes)

    kmp_rtobj_t		rtobj;			// predefined route object
    kmp_rtobj_t		rtobj_enpt;		// >0: for each ENPT 1 object
    kmp_rtobj_t		rtobj_itpt;		// >0: for each ITPT 1 object
    kmp_rtobj_t		rtobj_jgpt;		// >0: for each JGPT 1 object
    kmp_rtobj_t		rtobj_cnpt;		// >0: for each CNPT 1 object
    kmp_rtobj_t		rtobj_mspt;		// >0: for each MSPT 1 object
    kmp_rtobj_list_t	rtobj_poti[50];		// objects for POTI routes
    uint		n_rtobj_poti;		// number of valid elements
						// in 'rtobj_poti[]'
    //--- objects on check points

    int			ckpt_object_mode;	// view mode
    int			ckpt_object1;		// object 1
    int			ckpt_object2;		// object 2
    Var_t		ckpt_object1_scale;	// scale of object 1
    Var_t		ckpt_object2_scale;	// scale of object 2
    float		ckpt_object1_base;	// y base of object 1
    float		ckpt_object2_base;	// y base of object 2

    //--- special settings

    u8			ckpt_auto_respawn[KMP_MAX_POINT]; // >0: assign nearest JGPT

    int			jgpt_auto_define;	// >0 and valid section: auto define JGPT
    float		jgpt_auto_distance;	// >0.0: define only if no other pt in distance
    Var_t		jgpt_auto_adjust;	// add this vector to auto defined point
    int			jgpt_auto_remove;	// >0: remove unused: 1=auto defined, 2=all

    //--- misc

    bool		is_pass2;		// true: KMP compiler runs pass2
    bool		is_shrinked;		// true: source is a shrinked KMP
    bool		multi_ckpt_mode_0_ok;	// true: disable warnings about mode 0
    bool		enable_fall_kcl;	// true: enable 'fall_kcl' support
    struct kcl_t	*fall_kcl;		// NULL or pointer to own fall-down kcl

    struct kmp_linfo_t	*linfo;			// NULL or link infos for 'linfo_sect'
    kmp_entry_t		linfo_sect;		// section of 'linfo'

    const kmp_stgi_entry_t *stgi;		// pointer to raw STGI section.
						// only valid after ScanRawKMP()
						// as long as source is valid too

    //--- raw data, created by CreateRawKMP()

    u8			* raw_data;		// NULL or data
    uint		raw_data_size;		// size of 'raw_data'
    uint		raw_head_size;		// size of file header of 'raw_data'

} kmp_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP interface			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeKMP ( kmp_t * kmp );
void ResetKMP ( kmp_t * kmp );
void ClearSectionsKMP ( kmp_t * kmp );
const VarMap_t * SetupVarsKMP();

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanTextKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    CheckMode_t		mode		// not NULL: call CheckKMP(mode)
);

//-----------------------------------------------------------------------------

enumError ScanRawDataKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckKMP(mode)
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckKMP(mode)
);

//-----------------------------------------------------------------------------

enumError CreateRawKMP
(
    kmp_t		* kmp		// pointer to valid KMP
);

//-----------------------------------------------------------------------------

enumError SaveRawKMP
(
    kmp_t		* kmp,		// pointer to valid KMP
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextKMP
(
    const kmp_t		* kmp,		// pointer to valid KMP
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

///////////////////////////////////////////////////////////////////////////////

bool SortGOBJ
(
    kmp_t		* kmp,		// pointer to valid KMP
    kmp_sort_t		sort_mode	// one of KSORT_OFF, KSORT_GROUP,
					//   KSORT_ANGLE, KSORT_XYZ or KSORT_TINY
);

//-----------------------------------------------------------------------------

bool TransformKMP
(
    kmp_t		* kmp		// pointer to valid KMP
);

//-----------------------------------------------------------------------------

bool PatchKMP
(
    kmp_t		* kmp		// pointer to valid KMP
);

//-----------------------------------------------------------------------------

bool PatchRawDataKMP
(
    void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

///////////////////////////////////////////////////////////////////////////////

int GetNextPointKMP
(
    const kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP PT section index
    int			pt_index,	// index of point, <0: add N
    int			sub_index,	// index into 'next'
					//   -1: use 'pt_index+1' in a group
					//       find first valid at group end
					//  !-1 && KMP_CKPT: try to use 'next' link
    bool		unique,		// return 'not_found_val' on duplicates
    int			not_found_val,	// return this value, if next point not found
    int			*ret_pt_index,	// not NULL: store fixed 'pt_index' here
    int			*ret_sub_index,	// not NULL: store used sub index here
    bool		*ret_new_group,	// not NULL: store true, if new group
    const kmp_enph_entry_t **ret_ph	// not NULL: store found ph-record
);

///////////////////////////////////////////////////////////////////////////////

const kmp_enph_entry_t * FindPH
(
    const kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP PT or PH section index
    int			pt_index,	// index of point, <0: add N
    int			* res_ph_index	// not NULL: store ph_index or -1 here
);

///////////////////////////////////////////////////////////////////////////////

int CheckConvexCKPT
(
    // returns
    //  -1: illegal index
    //   0: convex
    //   1: concave

    const kmp_t		*kmp,		// KMP data structure
    uint		ckpt1,		// index of first check point (robust)
    uint		ckpt2,		// index of second check point (robust)
    double		*min_distance	// not NULL: store minimum distance here
);

///////////////////////////////////////////////////////////////////////////////

void RenameGroupKMP
(
    kmp_t		* kmp,		// KMP data structure
    uint		sect_ph,	// ph section index
    kmp_group_name_t	* gname,	// pointer to group name array
    bool		enable_dp	// enable dispatch point support
);

///////////////////////////////////////////////////////////////////////////////

void InsertLinkPH
(
    u8			* vector,	// insert into vector
    u8			link,		// link to insert
    ScanInfo_t		* si,		// not NULL: print error message
    ccp			name,		// NULL or name of line for error message
    bool		allow_dup	// true: allow multiple links into same section
);

///////////////////////////////////////////////////////////////////////////////

#define MAX_CLASS_NAMES 255

//-----------------------------------------------------------------------------

void SetupClassNamePH
(
    kmp_ph_t		* ph		// valid PH data structure
);

//-----------------------------------------------------------------------------

int InsertClassNamePH
(
    // return 1..254 if found or set, 255 if list is full, -1 on error

    kmp_ph_t		* ph,		// valid PH data structure
    ccp			cname,		// NULL or name of class
    bool		no_warn		// TRUE: suppress warning about full list
);

//-----------------------------------------------------------------------------

int FindClassNamePH
(
    // return -1, if not found, 1..254 else

    kmp_ph_t		* ph,		// valid PH data structure
    ccp			cname		// NULL or name of class
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP itembox support		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct Itembox_t
{
    ObjectFlags_t
		obj_flags;	// >0: ObjectFlags_t of analyzed itembox
    u16		obj_id;		// >0: ID of analyzed itembox
    u16		item_player;	// mode of player item
    u16		item_enemy;	// mode of enemy item
    bool	unsure_player;	// true: not sure, if 'item_player' is correct
    bool	unsure_enemy;	// true: not sure, if 'item_enemy' is correct
}
Itembox_t;

// initialze 'ibox' and returns 'ibox->obj_id'
u16 AnalyseItembox ( Itembox_t *ibox, const kmp_gobj_entry_t *op );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP draw support		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[draw_mode_t]]

typedef enum draw_mode_t
{
    DMD_NONE		= 0,

    DMD_KTPT		= 1 << KMP_KTPT,
    DMD_ENPT		= 1 << KMP_ENPT,
    DMD_ITPT		= 1 << KMP_ITPT,
    DMD_CKPT		= 1 << KMP_CKPT,
    //DMD_GOBJ		= 1 << KMP_GOBJ,
    DMD_POTI		= 1 << KMP_POTI,
    DMD_AREA		= 1 << KMP_AREA,
    //DMD_CAME		= 1 << KMP_CAME,
    DMD_JGPT		= 1 << KMP_JGPT,
    DMD_CNPT		= 1 << KMP_CNPT,
    //DMD_MSPT		= 1 << KMP_MSPT,

     DMD_SHIFT_PFLAGS	= KMP_N_SECT,		// shift right to get presence flags
    DMD_1OFFLINE	= 1 << KMP_N_SECT,
    DMD_2OFFLINE	= DMD_1OFFLINE << 1,
    DMD_3OFFLINE	= DMD_2OFFLINE << 1,
    DMD_1ONLINE		= DMD_3OFFLINE << 1,
    DMD_2ONLINE		= DMD_1ONLINE << 1,
    DMD_3ONLINE		= DMD_2ONLINE << 1,

    DMD_CJGPT		= DMD_3ONLINE << 1,
    DMD_ITEMBOX		= DMD_CJGPT << 1,
    DMD_COIN		= DMD_ITEMBOX << 1,
    DMD_ROADOBJ		= DMD_COIN << 1,
    DMD_SOLIDOBJ	= DMD_ROADOBJ << 1,
    DMD_DECORATION	= DMD_SOLIDOBJ << 1,
    DMD_DISPATCH	= DMD_DECORATION << 1,
    DMD_BLACK		= DMD_DISPATCH << 1,
    DMD_WHITE		= DMD_BLACK << 1,
    DMD_KCL		= DMD_WHITE << 1,

    DMD_DETAILED	= DMD_KCL * 2ull,
    DMD_WARNINGS	= DMD_DETAILED * 2ull,

    DMD_M_OFFLINE	= DMD_1OFFLINE	// offline presence flags
			| DMD_2OFFLINE
			| DMD_3OFFLINE,

    DMD_M_ONLINE	= DMD_1ONLINE	// online presence flags
			| DMD_2ONLINE
			| DMD_3ONLINE,

    DMD_M_PFLAGS	= DMD_M_OFFLINE	// all presence flags
			| DMD_M_ONLINE,

    DMD_M_FLAGS		= DMD_DETAILED	// flags
			| DMD_WARNINGS,

    DMD_M_RESPAWN	= DMD_CKPT	// all about respawning
			| DMD_JGPT
			| DMD_CJGPT
			| DMD_KTPT
			| DMD_KCL,

    DMD_M_PTS		= DMD_JGPT	// single points
			| DMD_KTPT,

    DMD_M_GOBJ		= DMD_ITEMBOX	// global objects
			| DMD_COIN
			| DMD_ROADOBJ
			| DMD_SOLIDOBJ
			| DMD_DECORATION,

    DMD_M_ADD		= DMD_M_PTS	// modes that add faces
			| DMD_M_GOBJ
			| DMD_ENPT
			| DMD_ITPT
			| DMD_CKPT
			| DMD_POTI
			| DMD_AREA
			| DMD_CNPT
			| DMD_CJGPT
			| DMD_DISPATCH
			| DMD_BLACK
			| DMD_WHITE,

    DMD_M_ALL		= DMD_M_ADD	// all possible modes
			| DMD_KCL
			| DMD_M_PFLAGS
			| DMD_M_FLAGS,

    DMD_M_RESET		= DMD_M_ALL	// reset these bits
			& ~DMD_M_FLAGS,

    DMD_M_NEED_REF	= DMD_CNPT,	// needs a reference KCL

    DMD_M_DEFAULT	= DMD_M_ALL	// default setting
			& ~DMD_M_PFLAGS

} draw_mode_t;

///////////////////////////////////////////////////////////////////////////////

extern draw_mode_t KMP_DRAW;

int ScanOptDraw ( ccp arg );
ccp GetDrawMode();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[DrawRouteMode_t]]

typedef enum DrawRouteMode_t
{
    // order by priority, lowest priority first

    DRTM_OFF,			// draw none
    DRTM_POTI,			// draw standard route
  //DRTM_CAME,			// draw came route
    DRTM_GOBJ,			// draw route for GOBJ objects
    DRTM_ITEMBOX,		// draw route for standard itembox
    DRTM_ITEMBOX_ENEMY,		// draw route for itembox with special item for enemies
    DRTM_ITEMBOX_PLAYER,	// draw route for itembox with special item for players
}
__attribute__ ((packed)) DrawRouteMode_t;

///////////////////////////////////////////////////////////////////////////////
// [[DrawKCL_t]]

#define MAX_ROUTE 0x100

typedef struct DrawKCL_t
{
    struct kcl_t	*draw;		// valid pointer to draw-KCL, maybe same as 'kcl'

    const struct kcl_t	*kcl;		// NULL or pointer to related source KCL
    const struct kmp_t	*kmp;		// NULL or pointer to related source KMP

    DrawRouteMode_t	route[MAX_ROUTE]; // List of routes to draw
    bool		warnings;	// true: draw warnings in red
    bool		detailed;	// true: print more detailed
    bool		as_road;	// true: print as road (KCL flag 0x0000)
}
DrawKCL_t;

//-----------------------------------------------------------------------------

static inline void InitializeDrawKCL ( DrawKCL_t *dk )
	{ DASSERT(dk); memset(dk,0,sizeof(*dk)); }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP draw functions		///////////////
///////////////////////////////////////////////////////////////////////////////

void AddAreas2KCL
(
    DrawKCL_t		* dk		// pointer to initalized object
);

//-----------------------------------------------------------------------------

void AddCheckPoints2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    bool		draw_jgpt	// true: draw connections to JGPT
);

//-----------------------------------------------------------------------------

void AddArrow2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    uint		sect,		// KMP section
    uint		arrow_color,	// color of arrow
    uint		area_color,	// color of area behind arrow
    double		area_len,	// >0.0: width of area behind arrow
    double		area_wd		// >0.0: length of area behind arrow
);

//-----------------------------------------------------------------------------

void AddRoutes2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    uint		sect_pt,	// KMP section
    uint		color_std	// standard color index
);

//-----------------------------------------------------------------------------

void AddPotiRoutes2KCL
(
    DrawKCL_t		* dk		// pointer to initalized object
);

//-----------------------------------------------------------------------------

void AddCannons2KCL
(
    DrawKCL_t		* dk		// pointer to initalized object
);

//-----------------------------------------------------------------------------

void AddObject2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    const
     kmp_gobj_entry_t	* op		// object to draw
);

//-----------------------------------------------------------------------------

void AddObjectsById2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    u16			obj_id		// object id to draw
);

//-----------------------------------------------------------------------------

void AddObjectsByFlag2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    u32			flag_mask,	// mask the object flag with this ...
    u32			flag_result	// ... and draw it, if the result is this.
);

//-----------------------------------------------------------------------------

void AddZeroGround2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    uint		color		// color of ground
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			pos file support		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[pos_param_t]] [[pos_file_t]]

#define MAX_POS_FILE_INDEX	50
#define MAX_POS_PARAM		15
#define MAX_POS_FILTER		 3

typedef struct pos_param_t
{
    //--- columns, all column indeces are 1 based

    uint		idx_x;		// >0: column index of x-pos
    uint		idx_y;		// >0: column index of y-pos
    uint		idx_z;		// >0: column index of z-pos
    uint		idx_d;		// >0: column index of directiion (degree)

    //--- color

    uint		autocolor;	// >0: column for auto color, ignore color
    uint		color;		// kcl_flag_*
    int			transp;		// 0,1,2 for transparency selection, -1:disabled

    //--- filter

    uint  idx_f[MAX_POS_FILTER];	// column index of filter A
    char  filter[MAX_POS_FILTER][20];	// text of filter A
}
pos_param_t;

typedef struct pos_file_t
{
    struct pos_file_t	*next;		// link to next entry
    ccp			fname;		// filename (alloced). NULL if invalid

    uint		n_param;	// number of used 'param' elements
    pos_param_t		param[MAX_POS_PARAM];
}
pos_file_t;

//-----------------------------------------------------------------------------

extern pos_param_t default_pos_param;
extern pos_file_t  current_pos_param;
extern pos_file_t *first_pos_file;

//-----------------------------------------------------------------------------

int ScanOptPosFile ( ccp arg );
int ScanOptPosMode ( ccp arg );

void DrawPosFiles( DrawKCL_t *draw );
enumError DrawPosFile ( DrawKCL_t *draw, pos_file_t *pf );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			link info			///////////////
///////////////////////////////////////////////////////////////////////////////

#define KMP_MAX_LINFO 0x100

//-----------------------------------------------------------------------------
// [[kmp_linfo_g_mode]]

typedef enum kmp_linfo_g_mode
{
    LINFOG_DISPATCH	= 0x01, // group is dispatch point
}
__attribute__ ((packed)) kmp_linfo_g_mode;

//-----------------------------------------------------------------------------
// [[kmp_linfo_mode]]

typedef enum kmp_linfo_mode
{
    LINFO_D_PREV	= 0x01, // $1 links directly to $2 by PREV
    LINFO_D_NEXT	= 0x02, // $1 links directly to $2 by NEXT
    LINFO_D_BY_PREV	= 0x04, // $1 is directly linked from $2 by PREV
    LINFO_D_BY_NEXT	= 0x08, // $1 is directly linked from $2 by NEXT

    LINFO_G_PREV	= 0x10, // $1 links via group to $2 by PREV
    LINFO_G_NEXT	= 0x20, // $1 links via group to $2 by NEXT
    LINFO_G_BY_PREV	= 0x40, // $1 is linked via group from $2 by PREV
    LINFO_G_BY_NEXT	= 0x80, // $1 is linked via group from $2 by NEXT

    LINFO_PREV		= LINFO_D_PREV    | LINFO_G_PREV,
    LINFO_NEXT		= LINFO_D_NEXT    | LINFO_G_NEXT,
    LINFO_BY_PREV	= LINFO_D_BY_PREV | LINFO_G_BY_PREV,
    LINFO_BY_NEXT	= LINFO_D_BY_NEXT | LINFO_G_BY_NEXT,

    LINFO_M_D		= LINFO_D_PREV | LINFO_D_NEXT | LINFO_D_BY_PREV | LINFO_D_BY_NEXT,
    LINFO_M_G		= LINFO_G_PREV | LINFO_G_NEXT | LINFO_G_BY_PREV | LINFO_G_BY_NEXT,
}
__attribute__ ((packed)) kmp_linfo_mode;

//-----------------------------------------------------------------------------
// [[kmp_linfo_t]]

typedef struct kmp_linfo_t
{
    uint	sect_pt;
    uint	sect_ph;

    uint	used;

    u8			n_point[KMP_MAX_LINFO];
    kmp_linfo_g_mode	gmode[KMP_MAX_LINFO];
    kmp_linfo_mode	summary[KMP_MAX_LINFO];
    kmp_linfo_mode	list[KMP_MAX_LINFO][KMP_MAX_LINFO];

}
kmp_linfo_t;

//-----------------------------------------------------------------------------

void ResetRouteLinksKMP ( const kmp_t *kmp );
kmp_linfo_t * SetupRouteLinksKMP ( const kmp_t *ckmp, uint sect, bool force );

uint AnalyseRouteKMP       ( kmp_linfo_t *li, const kmp_t *kmp, uint sect );
uint AnalyseGroupLinksKMP  ( kmp_linfo_t *li, const kmp_t *kmp, uint sect_ph );
uint AnalyseRouteLinksKMP  ( kmp_linfo_t *li, const kmp_t *kmp, uint sect_pt );
uint AnalyseRouteLinksCKPT ( kmp_linfo_t *li, const kmp_t *kmp );

// print_group: -1=never, 0:auto, 1:always
ccp PrintLinkInfoMode ( kmp_linfo_mode mode, int print_group, bool adjust );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			detect KMP types		///////////////
///////////////////////////////////////////////////////////////////////////////

static inline bool IsDispatchPointKMP ( const kmp_enph_entry_t *p )
{
//    return p->pt_len == 1 && p->prev[0] == 0xff && p->next[0] != 0xff;
    return p->pt_len == 1 && p->prev[0] == 0xff;
}

//-----------------------------------------------------------------------------

void GetRouteTypeInfoKMP
(
    kmp_rtype_info_t	*rti,	// store result here (never NULL)
    const struct kmp_t	*kmp,	// KMP data
    uint		sect	// section to check (PT or PH)
);

///////////////////////////////////////////////////////////////////////////////

IsArena_t IsArenaKMP ( const struct kmp_t * kmp );
IsArena_t CheckBattleModeKMP ( const struct kmp_t * kmp );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp_finish_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_finish_t]]

typedef struct kmp_finish_t
{
    bool	valid;			// true: record is valid =>
					//	finish_ktpt >= 0 && finish_ckpt >= 0
    u8		hint;			// bit field: 1=direction, 2=distance
    u8		warn;			// bit field: 1=direction, 2=distance

    u8		n_ckpt_0;		// number of check points
    u8		n_ktpt_m;		// number of KTPT with index <0

    s16		ckpt;			// -1 or index of relevant CKPT
    s16		ckpt_opt;		// -1 or index of CKPT used for optimized KTPT
    s16		ktpt;			// -1 or index of relevant KTPT
    s16		enpt_rec;		// -1 or index of ENPT used for recommended KTPT

    float3	pos_ckpt;		// neareast point to ktpt
    float3	pos_ktpt;		// position
    float3	pos_ktpt_opt;		// optimized position
    float3	pos_ktpt_rec;		// recommended position

    float	dir_ckpt;		// direction of CKPT, (0..360(
    float	dir_ktpt;		// direction of KTPT, (0..360(
    float	dir_ktpt_opt;		// optimized direction of KTPT, (0..360(
    float	dir_ktpt_rec;		// recommended direction of KTPT, (0..360(
    float	dir_delta;		// abs(dir_ckpt-dir_ckpt), (0..180)
    float	distance;		// 2D distance of KTPT and finish line
    float	distance_rec;		// 2D distance of 'pos_ktpt' and 'pos_ktpt_rec'
}
kmp_finish_t;

//-----------------------------------------------------------------------------

void InitializeFinishLine ( kmp_finish_t *kf );

double CalcCheckPointDistance
(
    const kmp_ckpt_entry_t	*ckpt,
    double			x,
    double			z
);

void CalcFinishLine
(
    const kmp_t			*kmp,	// not NULL: use it for recommendation
    kmp_finish_t		*kf,	// valid data
    const kmp_ckpt_entry_t	*ckpt,	// valid CKPT
    const kmp_ktpt_entry_t	*ktpt	// valid KTPT
);

bool CheckFinishLine ( const kmp_t *kmp, kmp_finish_t *kf );
ccp LogFinishLine ( kmp_finish_t *kf );


int InsertSecondKTPT
(
    // returns: 0=not possible, 1:inserted, 2:replaced

    kmp_t	*kmp,	    // valid KMP
    double3	pos	    // position to insert
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp_usedpos_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_usedpos_obj_t]]

typedef struct kmp_usedpos_obj_t
{
    //--- input

    float3	min;			// minimal and maximal coordinates ...
    float3	max;			// ... used by ENPT and ITPT.
    float3	allowed;		// allowed values, init by 131071 == 2^17-1

    //--- info (not used for calculations, set by ModifyUsedPosObj())

    float3	shift;			// active shift value
    float3	stretch;		// active stretch factor

    //--- results

    bool	valid;			// true: record is valid
    bool	useful;			// true: record has useful data
    bool	shift_active;		// true: shift is active
    bool	stretch_active;		// true: stretch is active

    WarnLevel_t	max_rating;		// max 'rating'
    WarnLevel_t	rating[3];		// rating for each axis of min_used & max_used
    bool	pos_factor_valid;	// TRUE: 'pos_factor' is valid
    float3	pos_factor;		// recommended factors for LEX/SET1/ITEM-POS-FACTOR
}
kmp_usedpos_obj_t;

//-----------------------------------------------------------------------------

void InitializeUsedPosObj ( kmp_usedpos_obj_t *obj );
void EvaluateUsedPosObj ( kmp_usedpos_obj_t *obj );
void ModifyUsedPosObj
	( kmp_usedpos_obj_t *obj, kmp_usedpos_obj_t *src, float3 *shift, float3 *stretch );
ccp GetUsedPosObjSuggestion
	( const kmp_usedpos_obj_t *obj, bool force, ccp return_if_empty );
void PrintUsedPosObj ( FILE *f, int indent, const kmp_usedpos_obj_t *obj );

///////////////////////////////////////////////////////////////////////////////
// [[kmp_usedpos_t]]

typedef struct kmp_usedpos_t
{
    kmp_usedpos_obj_t	orig;		// original values
    kmp_usedpos_obj_t	orig_lex;	// impact LEX/SET1/ITEM-POS-FACTOR
    kmp_usedpos_obj_t	center;		// after shifting to center
    kmp_usedpos_obj_t	center_lex;	//  + impact LEX/SET1/ITEM-POS-FACTOR

    kmp_usedpos_obj_t	*suggest;	// one of above, never NULL
}
kmp_usedpos_t;

//-----------------------------------------------------------------------------

void InitializeUsedPos ( kmp_usedpos_t *up );
void CheckUsedPos ( const kmp_t *kmp, kmp_usedpos_t *up );
void CheckWarnKMP ( kmp_t *kmp, kmp_usedpos_t *up );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp_pflags_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_pflags_t]]

typedef struct kmp_pflags_t
{
    u32			n_gobj;		// number of GOBJ elements
    u32			n_standard;	// number of standard elements
    u32			n_xpflags;	// number of GOBJ elements with mode>0
    u32			n_cat_std;	// number of used 'std_count' categories
    u32			n_cat_mode;	// number of used 'mode_count' categories

    u32			n_defobj;	// number of total definition-objects
    u32			n_defobj_bits;	// number of definition-objects type BITS
    u32			n_defobj_or;	// number of definition-objects type OR
    u32			n_defobj_and;	// number of definition-objects type AND
    u32			n_obj_ref;	// number of valid object references
    u32			n_cond_ref;	// number of valid condition references
    u32			n_cond_rnd;	// number of valid randpm condition references

    u32			n_mode[16];	// number of elements with mode N>0
    u8			n_std[64];	// counters for standard bits (pflags&0x3f)
}
kmp_pflags_t;

//-----------------------------------------------------------------------------

uint CountPresenceFlags ( const kmp_t *kmp, kmp_pflags_t *pf );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		support for extension of pflags		///////////////
///////////////////////////////////////////////////////////////////////////////
// INFO

//--------------------------------------------------------------------------
// bit mask  info
//--------------------------------------------------------------------------
//  0 0x0001 Offline race with 1 human player.
//  1 0x0002 Offline race with 2 human players.
//  2 0x0004 Offline race with 3 human players.
//  3 0x0008 Offline race with 4 human players.
//--------------------------------------------------------------------------
//  4 0x0010 Online race with 1-6 players total and 1 human at Wii.
//  5 0x0020 Online race with 1-6 players total and 2 humans at Wii.
//  6 0x0040 Online race with 7-9 players total and 1 human at Wii.
//  7 0x0080 Online race with 7-9 players total and 2 humans at Wii.
//  8 0x0100 Online race with 10-12 players total and 1 human at Wii.
//  9 0x0200 Online race with 10-12 players total and 2 humans at Wii.
// 10 0x0400 Online race with 13-18 players total and 1 human at Wii.
// 11 0x0800 Online race with 13-18 players total and 2 humans at Wii.
// 12 0x1000 Online race with 19 or more players total and 1 human at Wii.
// 13 0x2000 Online race with 19 or more players total and 2 humans at Wii.
//--------------------------------------------------------------------------
// 14 0x4000 RESERVED1. In Versus (setting #3) it is used for Time Trial.
// 15 0x8000 RESERVED2.
//--------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// enum [[condition_ref_t]]

typedef enum condition_ref_t // extended IF
{
	COR_NEVER	= GOBJ_MIN_CONDITION,
	COR_LECODE,
	COR_SINGLE,
	COR_MULTI,
	COR_OFFLINE,
	COR_ONLINE,

	COR_LE6,	// 0x1006
	COR_GE7,
	COR_LE9,
	COR_GE10,
	COR_LE12,
	COR_GE13,
	COR_LE18,
	COR_GE19,

	COR_7_9,	// 0x100e
	COR_NOT_7_9,
	COR_7_12,
	COR_NOT_7_12,
	COR_7_18,
	COR_NOT_7_18,
	COR_10_12,
	COR_NOT_10_12,
	COR_10_18,
	COR_NOT_10_18,
	COR_13_18,
	COR_NOT_13_18,

	COR_TIMETRIAL,	// 0x101a
	COR_NO_TIMETRIAL,
	COR_BATTLE,
	COR_RACE,

	COR_BALLOON,	// 0x101e
	COR_NO_BALLOON,
	COR_COIN,
	COR_NO_COIN,
	COR_VERSUS,
	COR_NO_VERSUS,
	COR_ITEMRAIN,
	COR_NO_ITEMRAIN,
	COR_SETTING5,
	COR_NO_SETTING5,
	COR_SETTING6,
	COR_NO_SETTING6,
	COR_SETTING7,
	COR_NO_SETTING7,
	COR_SETTING8,
	COR_NO_SETTING8,

	COR_P2,
	COR_P134,
	COR_P3,
	COR_P124,
	COR_P4,
	COR_P123,
	COR_P12,
	COR_P34,
	COR_P13,
	COR_P24,
	COR_P14,
	COR_P23,

	COR__N,		// 0x102e


	//--- developer values

	CORD_DEBUG	= GOBJ_DEV_CONDITION,
	CORD_RELEASE,
	CORD_TESTVAL,
	CORD_NO_TESTVAL,

	CORD__N		// 0x1904
}
condition_ref_t;

//-----------------------------------------------------------------------------
// [[gobj_cond_ref_t]]

typedef struct gobj_cond_ref_t
{
    u16		ref_id;
    char	name[14];
    char	alias[6];
    u8		option;		// Prefix: 0:no "", 2:"IF$", 4:"IFDEV$"
				// &1: new group
    char	info[20];

    u16		cond_mask;	// mask for consitions
    u8		set_mask;	// mask to select setting
    u8		op;		// operator:
				//   0: cond_mask || set_mask
				//   1: cond_mask && set_mask
				//   *: disabled
}
gobj_cond_ref_t;

//-----------------------------------------------------------------------------

extern const gobj_cond_ref_t cond_ref_tab[COR__N+1];
const gobj_cond_ref_t * FindConditionRef ( u16 ref_id );

///////////////////////////////////////////////////////////////////////////////
// [[gobj_cond_mask_t]]

typedef struct gobj_cond_mask_t
{
    u16		bit_mask;	// mask to select bit of 'setting'
    u8		game_mask;	// mask to select 'setting'
    u8		engine_mask;	// mask to select am engine
    u8		random_mask;	// mask to select a random scenario
}
gobj_cond_mask_t;

// if cref==NULL, then it is searched if needed.
bool IsConditionRefEnabled
	( const gobj_cond_mask_t *cm, u16 ref_id, const gobj_cond_ref_t *cref );

///////////////////////////////////////////////////////////////////////////////
// [[cond_ref_info_t]]

typedef struct cond_ref_info_t
{
    bool	is_condref;	// true: is valid ref_id for condition-ref
    const gobj_cond_ref_t
		*condref;	// not NULL: definition found
    ccp		cmd_suffix;	// either "DISABLE || "ENABLE", NULL if !is_condref
    u16		xor;		// 0|1, if 1: ref_id xor'd to get 'cond'
    char	cond[80];	// either "IF$.." or "0x1234" or "0" (if !is_condref)
    char	info[44];	// info for user
}
cond_ref_info_t;

// returns 'cri->s_condref'
bool GetConditionRefInfo
	( cond_ref_info_t *cri, u16 ref_id, u16 obj_id, bool create_cond  );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: support for extension of pflags	///////////////
///////////////////////////////////////////////////////////////////////////////

u16 GetActiveObjectId ( const kmp_gobj_entry_t *g );

static inline u16 GetRelevantObjectId ( u16 obj_id )
	{ return obj_id < GOBJ_MIN_DEF ? obj_id & GOBJ_M_OBJECT : obj_id; }

static inline u16 GetPFlagsMode ( u16 pflags )
	{ return ( pflags & GOBJ_M_PF_MODE ) >> GOBJ_S_PF_MODE; }

//---------------

static inline bool IsConditionRefId ( u16 ref_id )
	{ return ref_id >= GOBJ_MIN_CONDITION
	      && ref_id <= GOBJ_MAX_CONDITION; }

static inline bool IsConditionRefEngineId ( u16 ref_id )
	{ return ref_id >= GOBJ_MIN_COND_ENGINE
	      && ref_id <= GOBJ_MAX_COND_ENGINE; }

static inline bool IsConditionRefRandomId ( u16 ref_id )
	{ return ref_id >= GOBJ_MIN_COND_RND
	      && ref_id <= GOBJ_MAX_COND_RND; }

static inline bool IsConditionRefRelevantRandomId ( u16 ref_id )
	{ return ref_id > GOBJ_MIN_COND_RND
	      && ref_id < GOBJ_MAX_COND_RND; }

static inline bool IsConditionRefTestId ( u16 ref_id )
	{ return ref_id >= GOBJ_MIN_COND_TEST
	      && ref_id <= GOBJ_MAX_COND_TEST; }

//---------------

static inline bool IsDefinitionObjectIdBITS ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF_BITS
	      && obj_id <= GOBJ_MAX_DEF_BITS; }

static inline bool IsDefinitionObjectIdOR ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF_OR
	      && obj_id <= GOBJ_MAX_DEF_OR; }

static inline bool IsDefinitionObjectIdAND ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF_AND
	      && obj_id <= GOBJ_MAX_DEF_AND; }

static inline bool IsDefinitionObjectId ( u16 obj_id )
{
    return obj_id >= GOBJ_MIN_DEF_BITS && obj_id <= GOBJ_MAX_DEF_BITS
	|| obj_id >= GOBJ_MIN_DEF_OR   && obj_id <= GOBJ_MAX_DEF_OR
	|| obj_id >= GOBJ_MIN_DEF_AND  && obj_id <= GOBJ_MAX_DEF_AND;
}

#if 0 // not used yet

static inline bool IsDefinitionObjectRefBITS ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF_BITS
	      && obj_id <= (GOBJ_MAX_DEF_BITS|GOBJ_DEF_F_NOT); }

static inline bool IsDefinitionObjectRefOR ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF_OR
	      && obj_id <= (GOBJ_MAX_DEF_OR|GOBJ_DEF_F_NOT); }

static inline bool IsDefinitionObjectRefAND ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF_AND
	      && obj_id <= (GOBJ_MAX_DEF_AND|GOBJ_DEF_F_NOT); }

static inline bool IsDefinitionObjectRef ( u16 obj_id )
	{ return obj_id >= GOBJ_MIN_DEF && obj_id <= GOBJ_MAX_DEF;}

#endif


static inline bool HaveDefinitionReference ( const kmp_gobj_entry_t *g )
	{ return   g
		&& IsDefinitionObjectId(g->ref_id)
		&& ( g->pflags & GOBJ_M_PF_MODE ) == GOBJ_PF_MODE1; }

///////////////////////////////////////////////////////////////////////////////
// [[opt_gamemode_t]]

typedef enum opt_gamemode_t
{
    //--- main modes

    GMD_STD		= 0x00000001,		// standard modes
    GMD_BALLOON		= 0x00000002,		// balloon battle
    GMD_COIN		= 0x00000004,		// coin runners
    GMD_VERSUS		= 0x00000008,		// versus races
    GMD_ITEMRAIN	= 0x00000010,		// item rain
     GMD__RESERVED	= 0x000001e0,		// reserved for settings 5-8

     GMD_MD_BATTLE	= 0x00000006,
     GMD_MD_RACE	= 0x00000018,
     GMD_MD_DEFAULT	= 0x0000011f,
     GMD_MD_ALL		= 0x0000011f,


    //--- filters

    GMD_NO_MODES	= 0x00001000,		// suppress modes (only auto mode)
    GMD_OFFLINE		= 0x00002000,		// offline modes
    GMD_ONLINE		= 0x00004000,		// online modes
     GMD_FI_DEFAULT	= 0x00006000,
     GMD_FI_ALL		= 0x00007000,


    //--- options

    GMD_AUTO		= 0x00010000,		// add generic case
    GMD_STD_FIRST	= 0x00020000,		// create std settings first
    GMD_ENGINE		= 0x00040000,		// append engine selection
    GMD_RANDOM		= 0x00080000,		// append random selection
    GMD_NO_TT		= 0x00100000,		// exclude time trial
    GMD_FORCE_TT	= 0x00200000,		// force time trial
    GMD_FULL		= 0x00400000,		// don't eliminate duplicates from list
    GMD_NSORT		= 0x00800000,		// sort scenarios by name
    GMD_ISORT		= 0x01000000,		// sort scenarios by index
    GMD_DEBUG		= 0x02000000,		// enable debug mode
    GMD_DUMP		= 0x04000000,		// enable dump mode
     GMD_OP_ALL		= 0x07ff0000,

    //--- flags

    GMD_F_HIDE		= 0x40000000,		// help flag to hide

    GMD_M_SORT		= GMD_NSORT
			| GMD_ISORT,

    GMD_M_OPTIONS	= GMD_ENGINE		// start flags if scanning options
			| GMD_RANDOM,

    GMD_M_DEFAULT	= GMD_MD_DEFAULT
			| GMD_FI_DEFAULT
			| GMD_ENGINE
			| GMD_RANDOM,

    GMD_M_ALL		= GMD_MD_ALL
			| GMD_FI_ALL
			| GMD_OP_ALL
}
opt_gamemode_t;

extern opt_gamemode_t opt_gamemodes;
int ScanOptGamemodes ( ccp arg );

///////////////////////////////////////////////////////////////////////////////
// [[kmp_ana_defobj_t]]

typedef struct kmp_ana_defobj_t
{
    bool	is_defobj;		// true: is a valid id for definition-objects
    bool	is_pure_defobj;		// true: is_defobj && other parms=default
    bool	is_logical;		// true: is OR or AND condition

    ccp		cmd_name;		// suffix for "$DEFOBJ-" or "IF$"
    ccp		par_info;		// Info about parameters
    int		index_beg;		// first index of settings to print
    int		index_end;		// first index of settings to skip
}
kmp_ana_defobj_t;

void AnalyseDefObject ( kmp_ana_defobj_t *ad, const kmp_gobj_entry_t *gobj );

///////////////////////////////////////////////////////////////////////////////
// [[kmp_ana_ref_t]]

typedef struct kmp_ana_ref_t
{
    u16		mode;		// mode of presence flags (pflags)
    bool	mode_valid;	// true: valid mode

    bool	condref_valid;	// true: valid reference_id for condition-ref
    bool	condeng_valid;	// true: valid reference_id for engine condition-ref
    bool	condrnd_valid;	// true: valid reference_id for random condition-ref
    bool	condtest_valid;	// true: valid reference_id for test values
    bool	objref_valid;	// true: valid reference_id for def-object

    bool	is_condref;	// true: mode_valid && condref_valid
    bool	is_condeng;	// true: mode_valid && condeng_valid
    bool	is_condrnd;	// true: mode_valid && condrnd_valid
    bool	is_condtest;	// true: mode_valid && condtest_valid
    bool	is_objref;	// true: mode_valid && objref_valid
    bool	is_valid;	// true: is_condref || is_objref
}
kmp_ana_ref_t;

void AnalyseRefObject ( kmp_ana_ref_t *ar, const kmp_gobj_entry_t *gobj );
kmp_gobj_entry_t * FindRefObject ( const kmp_t *kmp, const kmp_gobj_entry_t *gobj );
kmp_gobj_entry_t * FindRefObjectByID ( const kmp_t *kmp, u16 ref_id );

///////////////////////////////////////////////////////////////////////////////

bool NormalizeDefObj ( kmp_gobj_entry_t *gobj );
uint CountDefObjUsage ( const kmp_t *kmp, u16 obj_id );
ccp GetDefObjConditionName ( u16 cond );

bool FixDefObj
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			fix_mode	// >0: clear unused parameters
);

bool SortDefObj
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			sort_mode	// sort if KMP_DOB_BEGIN || KMP_DOB_END
);

///////////////////////////////////////////////////////////////////////////////

bool FixReference
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			fix_mode	// >0: fix if valid mode
					// >1: fix for all modes
);

bool SortReference
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			sort_mode	// sort if KMP_DOB_BEFORE || KMP_DOB_BEHIND
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: Convert conditions to text		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[gobj_condition_t]]

typedef struct gobj_condition_t
{
    u16		mask;		// Any of 0001..ffff.
    u8		setting;	// 1 bit (bits 0-7) for each setting (#1..#8)
    s8		bitnum;		// 0..15 for a single bit, -1 for a bit combination
    s8		min_offline;	// number of min online players (-1:not defined)
    s8		max_offline;	// number of max online players (-1:not defined)
    s8		min_online;	// number of min online players (-1:not defined)
    s8		max_online;	// number of max online players (-1:not defined)
    ccp		info;		// related info. NULL: list terminator
}
gobj_condition_t;

extern const gobj_condition_t gobj_condition[];

//-----------------------------------------------------------------------------
// [[gobj_condition_set_t]]

#define GOBJ_COND_SET_MAX 10

typedef struct gobj_condition_set_t
{
    u16		mask;		// evaluated mask
    u8		setting_mask;	// setting mask
    uint	n;		// number of used elements of 'cond[]' without term
    gobj_condition_t cond[GOBJ_COND_SET_MAX+1];
				// 'n' elements + terminator
}
gobj_condition_set_t;

//-----------------------------------------------------------------------------

// terminated by info==0
extern const gobj_condition_t gobj_condition[];

// if setting==-1: evaluate for all settings
uint GetGobjConditions ( gobj_condition_set_t *gcs, int setting, u16 mask );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: Analyse GOBJ elements		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_gobj_type_t]]

typedef enum kmp_gobj_type_t
{
    KGTY_NULL,		// object-id is null
    KGTY_INVALID,	// from object range, but invalid
    KGTY_OBJECT,	// valid object

    KGTY_DEFOBJ_BITS,	// definition-objects type BITS
    KGTY_DEFOBJ_OR,	// definition-objects type OR
    KGTY_DEFOBJ_AND,	// definition-objects type AND

    KGTY__N
}
__attribute__ ((packed)) kmp_gobj_type_t;

//-----------------------------------------------------------------------------
// [[kmp_gobj_status_t]]

typedef enum kmp_gobj_status_t
{
    KGST_USED		= 0x0001,  // object used
    KGST_ENABLED	= 0x0002,  // object enabled by default
    KGST_CONDREF	= 0x0004,  // object uses a condition-reference
    KGST_DEFOBJ		= 0x0008,  // object uses a definition-object
    KGST_REF_VALID	= 0x0010,  // condition-reference/definition-object valid
    KGST_ENGINE		= 0x0020,  // uses a engine condition
    KGST_RANDOM		= 0x0040,  // uses a random condition
    KGST_DISABLED_BY	= 0x0080,  // always disabled by condition

    KGST_M_DERIVE	= KGST_ENGINE|KGST_RANDOM,
}
__attribute__ ((packed)) kmp_gobj_status_t;

//-----------------------------------------------------------------------------
// [[kmp_gobj_info_t]]

typedef struct kmp_gobj_info_t
{
    u16			obj_id;		// cleared object id
    u16			ref_id;		// cleared reference id
    s16			defobj;		// >=0: index of related definition-object
    u16			ref_count;	// number of references
    kmp_gobj_status_t	status;		// object status
    kmp_gobj_type_t	type;		// object type
}
kmp_gobj_info_t;

//-----------------------------------------------------------------------------
// [[kmp_ana_gobj_t]]

typedef struct kmp_ana_gobj_t
{
    //--- parameters

    const kmp_t		*kmp;		// valid kmp
    const kmp_gobj_entry_t
			*gobj;		// pointer to GOBJ list
    uint		n_gobj;		// number of elements in 'gobj'

    //--- analysis

    kmp_gobj_info_t	*info;		// alloced array of 'n_gobj' elements

    uint		n_object;	// number of objects := n_disabled+n_enabled
    uint		n_disabled;	// number of initial disabled elements
    uint		n_enabled;	// number of initial enabled elements
    uint		n_engine;	// number of elements using a engine condition
    uint		n_random;	// number of elements using a random condition

    uint		n_defobj;	// total number of definition-objects
    uint		n_defobj_bits;	// number of definition-objects type BITS
    uint		n_defobj_or;	// number of definition-objects type OR
    uint		n_defobj_and;	// number of definition-objects type AND
    uint		n_defobj_used;	// number of used definition-objects


    //--- misc

    uint		dur_usec;	// duration in micro seconds
}
kmp_ana_gobj_t;

//-----------------------------------------------------------------------------

void ResetAnaGobj ( kmp_ana_gobj_t *ag );
void AnalyseGobj  ( kmp_ana_gobj_t *ag, const kmp_t *kmp );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: Analyse PFlag scenarios		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kmp_ana_pflag_res_t]]

typedef struct kmp_ana_pflag_res_t
{
    char	name[16];	// name
    u16		version;	// version id
    u8		n_std;		// >0: result used by standard
    u8		n_ext;		// >0: result used by extension
    lex_test_t	lex_test;	// template for LEX section TEST
    ccp		status;		// alloced status, size=n_gobj+1
}
kmp_ana_pflag_res_t;

//-----------------------------------------------------------------------------
// [[kmp_ana_pflag_t]]

typedef struct kmp_ana_pflag_t
{
    //--- parameters

    kmp_ana_gobj_t	ag;
    opt_gamemode_t	opt_gamemode;	// game mode to analyze
    bool		auto_sep;	// true: automatic separators enabled

    //--- analysis

    uint		status_len;	// number of chars in res.status
    uint		n_res;		// number of valid results
    uint		n_version;	// number of different results
    uint		n_res_std;	// number of different results for std-code
    uint		n_res_ext;	// number of different results for extension

    kmp_ana_pflag_res_t	*res_list;	// alloced array with results
    uint		res_size;	// number of alloced elements for 'res_list'

    //--- misc

    uint		dur_usec;	// duration in micro seconds
}
kmp_ana_pflag_t;

//-----------------------------------------------------------------------------

void ResetPFlagScenarios ( kmp_ana_pflag_t *ap );
void AnalysePFlagScenarios
	( kmp_ana_pflag_t *ap, const kmp_t *kmp, opt_gamemode_t gmode );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			XPF: repair		    	///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptRepairXPF ( ccp arg );
enumError RepairXPF ( kmp_t *kmp );

///////////////////////////////////////////////////////////////////////////////
// [[xpf_mode1_t]]

typedef enum xpf_mode1_t
{
    XMODE1_NONE,	// MODE!=1
    XMODE1_NULL,	// MODE==1, but ref_id=0
    XMODE1_INVALID,	// MODE==1, but invalid ref_id
    XMODE1_CONDREF,	// MODE==1 && conditiopn reference
    XMODE1_OBJREF,	// MODE==1 && object reference
    XMODE1__N

}
xpf_mode1_t;

xpf_mode1_t CheckMode1GOBJ ( const kmp_gobj_entry_t *g );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  KMP misc			///////////////
///////////////////////////////////////////////////////////////////////////////

extern uint kmp_start_pos_mirror;   // last used setting
extern uint kmp_start_pos_narrow;   // last used setting
extern bool kmp_start_pos_auto;	    // last call used 'automode';

const double3 * GetStartPosKMP
(
    // Return NULL on failure.
    // Otherwise return a pointer to 'tab' or to an internal static buffer
    // with N elements. This buffer is overwritten on next call.

    double3	*tab,		// table with 'n' elements to store the result.
				//   if NULL: use internal static buffer
    uint	n,		// number of players: 1..12
    bool	mirror,		// true: mirror the data (pole is right)
    bool	narrow,		// true: use 'narrow' mode

    kmp_t	*kmp,		// not NULL: Use KTPT/#0 and STGI/#0
    uint	automode	// bit field: Use KMP to override parameters
				//  1: return absolute positions, if KMP available
				//  2: use pole position of KMP if available
				//  4: use narrow mode of KMP if available
);

///////////////////////////////////////////////////////////////////////////////

void DetectSpecialKMP
(
    const kmp_t		*kmp,		// pointer to valid KMP
    kmp_special_t	res		// store result here, never NULL
);

ccp CreateSpecialInfoKMP
	( kmp_special_t special, bool add_value, ccp return_if_empty );

///////////////////////////////////////////////////////////////////////////////

int CheckKMP
(
    // returns number of errors

    const kmp_t		*kmp,		// pointer to valid KMP
    CheckMode_t		mode		// print mode
);

///////////////////////////////////////////////////////////////////////////////

enumError DiffKMP
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
);

///////////////////////////////////////////////////////////////////////////////

extern int have_kmp_patch_count;

int ScanOptRmGobj ( ccp arg );
int ScanOptBattle ( ccp arg );
int ScanOptExportFlags ( ccp arg );
int ScanOptRouteOptions ( ccp arg );
int ScanOptWim0 ( ccp arg );

enumError cmd_stgi();
enumError cmd_ktpt();

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_KMP_H

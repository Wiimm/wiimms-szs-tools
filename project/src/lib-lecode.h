
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

#ifndef SZS_LIB_LECODE_H
#define SZS_LIB_LECODE_H 1

#include "lib-std.h"
#include "lib-mkw.h"
#include "lib-ctcode.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define LE_BINARY_MAGIC		"LECT"
#define LE_BINARY_MAGIC_NUM	0x4c454354

#define LE_PARAM_MAGIC		"LPAR"
#define LE_PARAM_MAGIC_NUM	0x4c504152

#define LE_LPAR_MAGIC		"#LE-LPAR"
#define LE_LPAR_MAGIC_NUM	0x234c452d4c504152ull

#define LE_CUP_MAGIC		"CUP2"
#define LE_COURSE_MAGIC		"CRS2"

#define LE_RESERVED_SLOTS	(LE_FIRST_LOWER_CT_SLOT - 32)
#define LE_MAX_BLOCK_TRACK	50

//-----------------------------------------------------------------------------
// [[le_valid_t]]

typedef enum le_valid_t // bit field
{
    LE_HEAD_FOUND	= 0x01,
    LE_HEAD_VALID	= 0x02,
    LE_HEAD_KNOWN	= 0x04,

    LE_PARAM_FOUND	= 0x10,
    LE_PARAM_VALID	= 0x20,
    LE_PARAM_KNOWN	= 0x40,
}
le_valid_t;

//-----------------------------------------------------------------------------
// [[le_usage_t]]

typedef enum le_usage_t
{
    LEU_S_UNUSED	=  0,	// - slot not used
    LEU_S_ARENA		=  1,	// A arena slot
    LEU_S_ARENA_RND	=  2,	// r arena slot
    LEU_S_ARENA_HIDE	=  3,	// a arena slot
    LEU_S_TRACK		=  4,	// T track slot
    LEU_S_TRACK_RND	=  5,	// R track slot
    LEU_S_TRACK_HIDE	=  6,	// t track slot
    LEU_S_SPECIAL	=  7,	// s special slot
    LEU_S_NETWORK	=  8,	// n network slot
    LEU_S_RANDOM	=  9,	// * network slot
    LEU_S_WIIMM		= 10,	// W random slot
    LEU_S_ALIAS		= 11,	// > is alias
     LEU_S_MASK		= 15,

    LEU_F_ONLINE	= 0x10,  // Flag: used online
}
__attribute__ ((packed)) le_usage_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    le_lpar_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_lpar_t]]

typedef struct le_lpar_t
{
    lpar_mode_t limit_mode;	// limit of other params

    u8	engine[3];		// 100cc, 150cc, mirror (sum always 100)
    u8	enable_200cc;		// TRUE: 200C enabled => 150cc, 200cc, mirror
    u8	enable_perfmon;		// TRUE: performance monitor enabled
    u8	enable_custom_tt;	// TRUE: time trial for cusotm tracks enabled
    u8	enable_xpflags;		// TRUE: extended presence flags enabled
    u8	enable_speedo;		// TRUE: speedometer is enabled
    u8	block_track;		// block used tracks for 0..20 races

    // reserved for future extensions
    u8   reserved_1b9;
    u8   reserved_1ba;
    u8   reserved_1bb;

    u16	chat_mode_1[BMG_N_CHAT]; // first set of modes for each chat message
    u16	chat_mode_2[BMG_N_CHAT]; // second set of modes for each chat message
}
le_lpar_t;

///////////////////////////////////////////////////////////////////////////////

lpar_mode_t CalcCurrentLparMode ( const le_lpar_t * lp, bool use_limit_param );
ccp GetLparModeName ( lpar_mode_t lpmode, bool export );
bool LimitToLparMode ( le_lpar_t * lp, lpar_mode_t lpmode );
void ClearLparChat ( le_lpar_t * lp );

//-----------------------------------------------------------------------------

ccp GetChatModeName ( u16 mode );
extern const u16 chat_mode_legacy[BMG_N_CHAT];
extern const u16 chat_mode_mkwfun[BMG_N_CHAT];

//-----------------------------------------------------------------------------

void InitializeLPAR ( le_lpar_t *lpar, bool load_lpar );

enumError LoadLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    bool		init_lpar,	// true: initialize 'lpar' first
    ccp			fname,		// filename of source, if NULL: use opt_lpar
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

enumError ScanTextLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    bool		init_lpar,	// true: initialize 'lpar' first
    ccp			fname,		// NULL or filename for error messages
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

enumError ScanTextLPAR_PARAM
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ScanInfo_t		*si,		// valid data
    bool		is_pass2	// false: pass1; true: pass2
);

enumError ScanTextLPAR_CHAT
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ScanInfo_t		*si,		// valid data
    bool		is_pass2	// false: pass1; true: pass2
);

//-----------------------------------------------------------------------------

enumError SaveTextLPAR
(
    const le_lpar_t	*lpar,		// LE-CODE parameters
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);


enumError WriteSectionLPAR
(
    FILE		*f,		// output files
    const le_lpar_t	*lpar		// LE-CODE parameters
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			binary headers			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_binary_head_t]]

typedef struct le_binary_head_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 4 for v4
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	size;		// size of complete file
 /*18*/  u32	off_param;	// offset of param section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	debug;		// one of: D (debug) or R (release)
 /*1e*/  u8	phase;		// >0: PHASE
 /*1f*/  char	unknown_1f;
}
__attribute__ ((packed)) le_binary_head_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v4_t]]

typedef struct le_binary_head_v4_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 4 for v4
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	size;		// size of complete file
 /*18*/  u32	off_param;	// offset of param section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	debug;		// one of: D (debug) or R (release)
 /*1e*/  u8	phase;		// >0: PHASE
 /*1f*/  char	unknown_1f;
 /*20*/  char	timestamp[];	// timestamp of build in ASCII
}
__attribute__ ((packed)) le_binary_head_v4_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_binary_param_t]]

typedef struct le_binary_param_t
{
 /*00*/  char	magic[8];	// LE_PARAM_MAGIC
 /*08*/  u32	version;	// always 1 for v1
 /*0c*/  u32	size;		// size (and minor version)
 /*10*/  u32	off_eod;	// offset of end-of-data
 /*14*/
}
__attribute__ ((packed)) le_binary_param_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_35_t]]

typedef struct le_binpar_v1_35_t
{
 /*00*/  char magic[8];	// LE_PARAM_MAGIC
 /*08*/  u32  version;	// always 1 for v1
 /*0c*/  u32  size;		// size (and minor version)
 /*10*/  u32  off_eod;	// offset of end-of-data

 /*14*/  u32  off_cup_par;	// offset to cup param
 /*18*/  u32  off_cup_track;	// offset of cup-track list
 /*1c*/  u32  off_cup_arena;	// offset of cup-arena list
 /*20*/  u32  off_course_par;	// offset of course param
 /*24*/  u32  off_property;	// offset of property list
 /*28*/  u32  off_music;	// offset of music list
 /*2c*/  u32  off_flags;	// offset of music list

 /*30*/  u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*33*/	 u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*34*/	 u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*35*/
}
__attribute__ ((packed)) le_binpar_v1_35_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_37_t]]

typedef struct le_binpar_v1_37_t
{
 /*00*/  char magic[8];	// LE_PARAM_MAGIC
 /*08*/  u32  version;	// always 1 for v1
 /*0c*/  u32  size;		// size (and minor version)
 /*10*/  u32  off_eod;	// offset of end-of-data

 /*14*/  u32  off_cup_par;	// offset to cup param
 /*18*/  u32  off_cup_track;	// offset of cup-track list
 /*1c*/  u32  off_cup_arena;	// offset of cup-arena list
 /*20*/  u32  off_course_par;	// offset of course param
 /*24*/  u32  off_property;	// offset of property list
 /*28*/  u32  off_music;	// offset of music list
 /*2c*/  u32  off_flags;	// offset of music list

 /*30*/  u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*33*/	 u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*34*/	 u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*35*/	 u8   enable_custom_tt;	// TRUE: time trial for cusotm tracks enabled
 /*36*/	 u8   enable_xpflags;	// TRUE: extended presence flags enabled
 /*37*/
}
__attribute__ ((packed)) le_binpar_v1_37_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_f8_t]]

typedef struct le_binpar_v1_f8_t
{
 /*00*/  char magic[8];	// LE_PARAM_MAGIC
 /*08*/  u32  version;	// always 1 for v1
 /*0c*/  u32  size;		// size (and minor version)
 /*10*/  u32  off_eod;	// offset of end-of-data

 /*14*/  u32  off_cup_par;	// offset to cup param
 /*18*/  u32  off_cup_track;	// offset of cup-track list
 /*1c*/  u32  off_cup_arena;	// offset of cup-arena list
 /*20*/  u32  off_course_par;	// offset of course param
 /*24*/  u32  off_property;	// offset of property list
 /*28*/  u32  off_music;	// offset of music list
 /*2c*/  u32  off_flags;	// offset of music list

 /*30*/  u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*33*/	 u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*34*/	 u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*35*/	 u8   enable_custom_tt;	// TRUE: time trial for cusotm tracks enabled
 /*36*/	 u8   enable_xpflags;	// TRUE: extended presence flags enabled
 /*37*/	 u8   block_track;	// block used track for 0.. tracks
 /*38*/  u16  chat_mode[BMG_N_CHAT]; // mode for each chat message
 /*f8*/
}
__attribute__ ((packed)) le_binpar_v1_f8_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_1b8_t]]

typedef struct le_binpar_v1_1b8_t
{
 /*000*/ char magic[8];	// LE_PARAM_MAGIC
 /*008*/ u32  version;		// always 1 for v1
 /*00c*/ u32  size;		// size (and minor version)
 /*010*/ u32  off_eod;		// offset of end-of-data

 /*014*/ u32  off_cup_par;	// offset to cup param
 /*018*/ u32  off_cup_track;	// offset of cup-track list
 /*01c*/ u32  off_cup_arena;	// offset of cup-arena list
 /*020*/ u32  off_course_par;	// offset of course param
 /*024*/ u32  off_property;	// offset of property list
 /*028*/ u32  off_music;	// offset of music list
 /*02c*/ u32  off_flags;	// offset of music list

 /*030*/ u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*033*/ u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*034*/ u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*035*/ u8   enable_custom_tt;// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;	// TRUE: extended presence flags enabled
 /*037*/ u8   block_track;	// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT]; // mode for each chat message
 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT]; // mode for each chat message
 /*1b8*/
}
__attribute__ ((packed)) le_binpar_v1_1b8_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_1bc_t]]

typedef struct le_binpar_v1_1bc_t
{
 /*000*/ char magic[8];	// LE_PARAM_MAGIC
 /*008*/ u32  version;		// always 1 for v1
 /*00c*/ u32  size;		// size (and minor version)
 /*010*/ u32  off_eod;		// offset of end-of-data

 /*014*/ u32  off_cup_par;	// offset to cup param
 /*018*/ u32  off_cup_track;	// offset of cup-track list
 /*01c*/ u32  off_cup_arena;	// offset of cup-arena list
 /*020*/ u32  off_course_par;	// offset of course param
 /*024*/ u32  off_property;	// offset of property list
 /*028*/ u32  off_music;	// offset of music list
 /*02c*/ u32  off_flags;	// offset of music list

 /*030*/ u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*033*/ u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*034*/ u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*035*/ u8   enable_custom_tt;// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;	// TRUE: extended presence flags enabled
 /*037*/ u8   block_track;	// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT]; // mode for each chat message
 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT]; // mode for each chat message
 /*1b8*/ u8   enable_speedo;	// TRUE: speedometer is enabled

 //--- reserved for future extensions
 /*1b9*/ u8   reserved_1b9;
 /*1ba*/ u8   reserved_1ba;
 /*1bb*/ u8   reserved_1bb;
 /*1bc*/
}
__attribute__ ((packed)) le_binpar_v1_1bc_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_t]]

typedef struct le_binpar_v1_1bc_t le_binpar_v1_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			binary sections			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_cup_par_t]]

typedef struct le_cup_par_t
{
 /*00*/  char	magic[4];	// LE_CUP_MAGIC
 /*04*/  u32	n_racing_cups;	// input: max cups, output: used cups
 /*08*/  u32	n_battle_cups;	// input: max cups, output: used cups
 /*0c*/
}
__attribute__ ((packed)) le_cup_par_t;

///////////////////////////////////////////////////////////////////////////////
// [[le_course_par_t]]

typedef struct le_course_par_t
{
 /*00*/  char	magic[4];	// LE_COURSE_MAGIC
 /*04*/  int	n_slot;		// input: max slots, output: used slots
 /*08*/
}
__attribute__ ((packed)) le_course_par_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			le_analyse_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_analyse_t]]

typedef struct le_analyse_t
{
    //--- header data

    le_valid_t	valid;		// see LE_VALID_* above
    uint	version;	// version = phase

    const u8	*data;		// pointer to raw data
    u32		size;		// data size

    uint	head_vers;	// not NULL: version number of header
    uint	head_size;	// not NULL: size told by header

    uint	param_offset;	// not NULL: offset of param
    uint	param_vers;	// not NULL: version number of param
    uint	param_size;	// not NULL: size told by param


    //--- working data

    uint	n_cup_track;	// number of cup tracks
    uint	max_cup_track;	// max possible cup tracks
    uint	n_cup_arena;	// number of cup arenas
    uint	max_cup_arena;	// max possible cup arenas

    uint	n_slot;		// number of slots
    uint	max_slot;	// max possible slots
    uint	max_property;	// max possible properties
    uint	max_music;	// max possible music slots
    uint	max_flags;	// max possible flags

    uint	used_rslots;	// used racing slots
    uint	max_rslots;	// max racing slots
    uint	used_bslots;	// used battle slots
    uint	max_bslots;	// max battle slots

    le_lpar_t	lpar;		// LPAR parameters

    le_usage_t	*usage;		// NULL or alloced list[n_slot], setup by GetLEUsage()
    uint	usage_size;	// number of alloed 'usage' elements


    //--- binary data

    u8		*bin_data;	// pointer to begin of binary data
    uint	bin_size;	// size of binary data


    //--- pointers

    le_cup_par_t	*cup_par;	// NULL or cup parameters
    le_course_par_t	*course_par;	// NULL or course parameters
    le_cup_track_t	*cup_track;	// NULL or list of cup tracks
    le_cup_track_t	*cup_arena;	// NULL or list of cup arenas
    le_property_t	*property;	// NULL or list of properties
    le_music_t		*music;		// NULL or list of music slots
    le_flags_t		*flags;		// NULL or list of flags

    u8			*beg_of_data;	// NULL or begin of data
    u8			*end_of_data;	// NULL or end of data

    union
    {
	le_binary_head_t	* head;
	le_binary_head_v4_t	* head_v4;
    };

    union
    {
	le_binary_param_t	* param;
	le_binpar_v1_t		* param_v1;
    };
}
le_analyse_t;

//-----------------------------------------------------------------------------

void ResetLEAnalyse ( le_analyse_t *ana );
void ResetLEAnalyseUsage ( le_analyse_t *ana );

enumError AnalyseLEBinary
(
    le_analyse_t	*ana,		// NULL or destination of analysis
    const void		*data,		// data pointer
    uint		data_size	// data size
);

void CalculateStatsLE ( le_analyse_t *ana );
const le_usage_t * GetLEUsage ( const le_analyse_t *ana0, bool force_recalc );
char GetLEUsageChar ( le_usage_t usage );
ccp GetLEUsageCharCol ( le_usage_t usage, const ColorSet_t *colset, ccp * prev_col );

ccp GetLEValid ( le_valid_t valid );
void CalculateStatsLE ( le_analyse_t *ana );
void DumpLEAnalyse ( FILE *f, uint indent, const le_analyse_t *ana );

static inline bool IsLESlotUsed ( le_analyse_t *ana, uint slot )
	{ return slot < ana->n_slot && ana->music[slot]; }

///////////////////////////////////////////////////////////////////////////////

const ctcode_t * LoadLEFile ( uint le_phase );

enumError ApplyLEFile ( le_analyse_t * ana );
enumError ApplyCTCODE ( le_analyse_t * ana, const ctcode_t * ctcode );
void PatchLECODE ( le_analyse_t * ana );
void PatchLPAR ( le_lpar_t * lp );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lecode extension LEX		///////////////
///////////////////////////////////////////////////////////////////////////////

#define LEX_MAJOR_VERSION	1
#define LEX_MINOR_VERSION	0

#define LEX_BIN_MAGIC		"LE-X"
#define LEX_BIN_MAGIC_NUM	0x4c452d58
#define LEX_TEXT_MAGIC		"#LEX"
#define LEX_TEXT_MAGIC_NUM	0x234c4558

///////////////////////////////////////////////////////////////////////////////
// [[have_lex_t]]
// order is important, compare have_lex_info[] and ct.wiimm.de

typedef enum have_lex_t
{
    HAVELEX_SET1,
    HAVELEX_CANN,
    HAVELEX_TEST,
    HAVELEX_HIPT,
    //--- add new elements here (order is important)
    HAVELEX__N
}
have_lex_t;

//-----------------------------------------------------------------------------
// [[lex_stream_id]]

typedef enum lex_stream_id
{
    LEXS_TERMINATE	= 0,		// terminate stream
    LEXS_IGNORE		= 0x2d2d2d2d,	// "----" ignore and remove

    LEXS_SET1		= 0x53455431,	// "SET1" primary settings
    LEXS_CANN		= 0x43414e4e,	// "CANN" cannon settings
    LEXS_HIPT		= 0x48495054,	// "HIPT" hide position tracker
    LEXS_TEST		= 0x54455354,	// "TEST" settings for tests
}
lex_stream_id;

//-----------------------------------------------------------------------------
// [[have_lex_info_t]]

typedef struct have_lex_info_t
{
    lex_stream_id	magic;		// section magic
    ccp			name;		// info name
    uint		min_size;	// minimal size for 'is available'
}
have_lex_info_t;

extern const have_lex_info_t have_lex_info[HAVELEX__N];

//-----------------------------------------------------------------------------
// [[lex_header_t]]

typedef struct lex_header_t
{
    char	magic[4];	// always LEX_BIN_MAGIC
    u16		major_version;	// usually LEX_MAJOR_VERSION
    u16		minor_version;	// usually LEX_MINOR_VERSION
    u32		size;		// size of this file (header+streams)
    u32		element_off;	// offset of first lex_element_t, 32-bit aligned
}
__attribute__ ((packed)) lex_header_t;

//-----------------------------------------------------------------------------
// [[lex_element_t]]

typedef struct lex_element_t
{
    u32		magic;		// identification of section
    u32		size;		// size of 'data' (this header excluded)
    u8		data[];		// section data, 32-bit aligned
}
__attribute__ ((packed)) lex_element_t;

//-----------------------------------------------------------------------------
// [[lex_set1_t]]

typedef struct lex_set1_t
{
    float3	item_factor;		// factor for item positions, always >=1.0
    u8		test[4];		// 4 test values (TEST1..TEST4)
}
__attribute__ ((packed,aligned(4))) lex_set1_t;

//-----------------------------------------------------------------------------
// [[lex_hipt_rule_t]]

typedef struct lex_hipt_rule_t
{
    u8		cond;	// condition bits: 1:offline, 2:online
    s8		lap;	// compare lap index with this. 99=all laps
    u8		from;	// compare CHKT >= from
    u8		to;	//      && CHKT <= to
    u8		mode;	// result: 0=hide, 1:show
}
__attribute__ ((packed)) lex_hipt_rule_t;

//-----------------------------------------------------------------------------
// [[lex_test_t]]

typedef struct lex_test_t
{
    u8		offline_online;	// 1: assume offline / 2: assume online
    u8		n_offline;	// >0: assume # offline humans at Wii
    u8		n_online;	// >0: assume # players total if playing online
    s8		cond_bit;	// >=0: use bit # for CONDITIONS
    u8		game_mode;	// one of LEX_GMODE_*
    u8		random;		// 1..6: force random scenario
    u8		engine;		// >=0: force engine := kmp_engine_t+1
    u8		padding;
}
__attribute__ ((packed,aligned(4))) lex_test_t;

///////////////////////////////////////////////////////////////////////////////

typedef enumError (*lex_stream_func)
	( u32 magic, const u8 *data, uint size, cvp user_ptr, int user_int );

enumError ScanLexFile
	( cvp data, uint data_size, lex_stream_func func, cvp user_ptr, int user_int );
enumError ScanLexElements
	( cvp data, uint data_size, lex_stream_func func, cvp user_ptr, int user_int );

lex_element_t * FindLexElement ( lex_header_t *head, uint data_size, u32 magic );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    lex_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[lex_item_t]]

typedef struct lex_item_t
{
    uint		sort_order;	// sort priority, lesser comes first
    uint		insert_order;	// second sort priority

    // always last element because of various size!!
    lex_element_t	elem;		// raw element data, big endian
}
lex_item_t;

///////////////////////////////////////////////////////////////////////////////
// [[lex_t]]

typedef struct lex_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform;		// FF_LEX | FF_LEX_TXT
    bool		develop;	// true: enable developer mode
    bool		modified;	// true: modified after loading


    //--- sections

    uint		item_used;	// number of used items
    uint		item_size;	// number of alloced items
    lex_item_t		**item;		// NULL or item list (alloced)
    uint		have_lex;	// status vector about found sections


    //--- parser helpers

    int			revision;	// tool revision
    bool		is_pass2;	// true: LEX compiler runs pass2


    //--- raw data, created by CreateRawLEX()

    u8			* raw_data;	// NULL or data
    uint		raw_data_size;	// size of 'raw_data'
}
lex_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX interface			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLEX ( lex_t * lex );
void ResetLEX ( lex_t * lex );
void UpdateLEX ( lex_t * lex, bool add_missing, bool add_test );

lex_item_t * AppendSet1LEX ( lex_t * lex, bool overwrite );
lex_item_t * AppendCannLEX ( lex_t * lex, bool overwrite );
lex_item_t * AppendHiptLEX ( lex_t * lex, bool overwrite );
lex_item_t * AppendTestLEX ( lex_t * lex, bool overwrite );

const VarMap_t * SetupVarsLEX();

void DumpElementsLEX ( FILE *f, lex_t *lex, bool hexdump );

ccp CreateSectionInfoLEX
	( have_lex_t special, bool add_value, ccp return_if_empty );

static inline bool IsLexFF ( file_format_t fform )
	{ return fform == FF_LEX || fform == FF_LEX_TXT; }

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    uint		dump_mode	// 0: add element to lex->sect
					// 1: dump scanned elements
					// >=0x10: add hexdump of max # bytes
);

//-----------------------------------------------------------------------------

enumError ScanTextLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanRawDataLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    struct raw_data_t	* raw		// valid raw data
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

//-----------------------------------------------------------------------------

enumError CreateRawLEX
(
    lex_t		* lex		// pointer to valid LEX
);

//-----------------------------------------------------------------------------

enumError SaveRawLEX
(
    lex_t		* lex,		// pointer to valid LEX
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextLEX
(
    const lex_t		* lex,		// pointer to valid LEX
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lex_info_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[lex_info_t]]

typedef struct lex_info_t
{
    bool	lex_found;		// true: LEX file found

    bool	set1_found;		// true: section SET1 found
    lex_set1_t	set1;			// data of section SET1

    bool	test_found;		// true: section TEST found
    lex_set1_t	test;			// data of section TEST
}
lex_info_t;

///////////////////////////////////////////////////////////////////////////////

static inline void InitializeLexInfo ( lex_info_t *info )
	{ DASSERT(info); memset(info,0,sizeof(*info)); }

static inline void ResetLexInfo ( lex_info_t *info )
	{ if (info) memset(info,0,sizeof(*info)); }

void SetupLexInfo ( lex_info_t *info, const lex_t *lex );
enumError SetupLexInfoByData ( lex_info_t *info, cvp data, uint data_size );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX patching			///////////////
///////////////////////////////////////////////////////////////////////////////

extern bool	opt_lex_purge;
extern bool	force_lex_test;
extern bool	opt_lt_clear;
extern int	opt_lt_online;
extern int	opt_lt_n_offline;
extern int	opt_lt_n_online;
extern int	opt_lt_itemrain;

int ScanOptLtOnline   ( ccp arg );
int ScanOptLtNPlayers ( ccp arg );
int ScanOptLtCondBit  ( ccp arg );
int ScanOptLtGameMode ( ccp arg );
int ScanOptLtEngine   ( ccp arg );
int ScanOptLtRandom   ( ccp arg );

// return TRUE if any option set
bool HavePatchTestLEX();
bool HaveActivePatchTestLEX();
static inline bool HavePatchLEX()
	{ return opt_lex_purge || HavePatchTestLEX(); }
static inline bool HaveActivePatchLEX()
	{ return opt_lex_purge || HaveActivePatchTestLEX(); }

// return TRUE if modified / set optional 'empty' to true, if test section is empty
bool PatchTestLEX ( lex_test_t *lt, bool *empty );
bool PatchLEX ( lex_t *lex );
bool PurgeLEX ( lex_t *lex );

struct szs_norm_t;
void AddTestLEX ( struct szs_file_t *szs, struct szs_norm_t *norm );

bool LEX_ACTION_LOG ( bool is_patch, const char * format, ... )
	__attribute__ ((__format__(__printf__,2,3)));

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsLECODE();

static inline bool IsRacingTrackLE ( uint tid )
	{ return tid < MKW_N_TRACKS
		|| tid >= LE_FIRST_LOWER_CT_SLOT && tid != 0xff; }

uint GetNextRacingTrackLE ( uint tid );

///////////////////////////////////////////////////////////////////////////////

extern ccp	opt_le_define;
extern ccp	opt_lpar;
extern ccp	opt_track_dest;
extern ParamField_t opt_track_source;

extern ccp	opt_le_alias;
extern bool	opt_engine_valid;
extern u8	opt_engine[3];
extern OffOn_t	opt_200cc;
extern OffOn_t	opt_perfmon;
extern OffOn_t	opt_custom_tt;
extern OffOn_t	opt_xpflags;
extern OffOn_t	opt_speedo;

extern int	opt_reserved_1b9;
extern int	opt_reserved_1ba;
extern int	opt_reserved_1bb;

extern bool	opt_complete;

int ScanOptTrackSource	( ccp arg, int mode );
int ScanOptAlias	( ccp arg );
int ScanOptEngine	( ccp arg );
int ScanOpt200cc	( ccp arg );
int ScanOptPerfMon	( ccp arg );
int ScanOptCustomTT	( ccp arg );
int ScanOptXPFlags	( ccp arg );
int ScanOptSpeedometer	( ccp arg );

int ScanOptReserved_1b9 ( ccp arg );
int ScanOptReserved_1ba ( ccp arg );
int ScanOptReserved_1bb ( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_LECODE_H


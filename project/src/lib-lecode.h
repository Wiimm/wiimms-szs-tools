
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
///////////////			lecode debug			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[lecode_debug_t]] [[copy-lecode]] copy changes to LE-CODE

typedef enum lecode_debug_t
{
    LEDEB_ENABLED	= 0x0001,
    LEDEB_OPPONENT	= 0x0002,
    LEDEB_SPACE		= 0x0004,

    LEDEB_POSITION	= 0x0008,
    LEDEB_CHECK_POINT	= 0x0030, // 2 bits!
    LEDEB_RESPAWN	= 0x0040,
    LEDEB_LAP_POS	= 0x0080,
    LEDEB_XPF		= 0x0300, // 2 bits!
     LEDEB_SHORT_XPF	= 0x0100,
     LEDEB_LONG_XPF	= 0x0200,
    LEDEB_ITEM_POINT	= 0x0400,
    LEDEB_KCL_TYPE	= 0x0800,

    //--- derived values

    LEDEB__STD		= 0x0ff9,   // standard output
    LEDEB__OUTPUT	= 0x0ff8,   // flags that produce output
    LEDEB__ALL		= 0x0fff,   // all flags

    LEDEB_S_CHECK_POINT	= 4,	// shift for LEDEB_CHECK_POINT
    LEDEB_M_CHECK_POINT	= 3,	// mask after shift

    LEDEB_S_XPF	= 8,		// shift for LEDEB_XPF
    LEDEB_M_XPF	= 3,		// mask after shift

    //--- special values

    LEDEB__N_CONFIG	=  4,	// number of debug configurations
    LEDEB__N_LINE	= 10,	// number of debug lines of each configuration
}
lecode_debug_t;

//-----------------------------------------------------------------------------
// [[lecode_debug_info_t]]

typedef struct lecode_debug_info_t
{
    lecode_debug_t	mode;		// mode to iterate
    bool		filter_active;	// true: iterate only values >0

    int			index;		// internal index
    lecode_debug_t	relevant;	// relevant bits for name=value
    ccp			name;		// NULL or name of parameter
    uint		value;		// related value
}
lecode_debug_info_t;

//-----------------------------------------------------------------------------

ccp GetDebugModeSummary ( const lecode_debug_info_t *ldi );

bool GetFirstDebugMode
	( lecode_debug_info_t *ldi, lecode_debug_t mode, bool filter_active );
bool GetNextDebugMode ( lecode_debug_info_t *ldi );


static inline bool isDebugModeOutput ( lecode_debug_t mode )
	{ return ( mode & LEDEB__OUTPUT ) != 0; }

static inline bool isDebugModeActive ( lecode_debug_t mode )
	{ return mode & LEDEB_ENABLED && mode & LEDEB__OUTPUT; }

//-----------------------------------------------------------------------------
// [[lecode_debug_ex_t]]

typedef struct lecode_debug_ex_t
{
    // encoded data
    lecode_debug_t mode;	

    // decoded data
    u8 enabled;
    u8 opponent;
    u8 space;
    u8 position;
    u8 check_point;
    u8 respawn;
    u8 item_point;
    u8 kcl_type;
    u8 lap_pos;
    u8 xpf;

    // status
    bool have_output;	// would produce output if ENABLED
    bool is_active;	// will produce output
}
lecode_debug_ex_t;

//-----------------------------------------------------------------------------

lecode_debug_t DecodeLecodeDebug ( lecode_debug_ex_t *lde, lecode_debug_t mode );
lecode_debug_t EncodeLecodeDebug ( lecode_debug_ex_t *lde );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_debug_mode_t]] [[copy-lecode]] copy changes to LE-CODE

typedef enum le_debug_mode_t
{
    DEBUGMD_OFF,	// debug disabled at all
    DEBUGMD_ENABLED,	// debug enabled, but hidden
    DEBUGMD_DEBUG1,	// debug enabled, start with config #0 [DEBUG-1]
    DEBUGMD_DEBUG2,	// debug enabled, start with config #1 [DEBUG-2]
    DEBUGMD_DEBUG3,	// debug enabled, start with config #2 [DEBUG-3]
    DEBUGMD_DEBUG4,	// debug enabled, start with config #3 [DEBUG-4]

    DEBUGMD__N		// number of debug modes
}
le_debug_mode_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[predef_debug_t]]

typedef enum predef_debug_t
{
    PREDEBUG_OFF,	// no predefinition
    PREDEBUG_CLEAR,	// clear all
    PREDEBUG_STANDARD,	// 1 line: standard setup
    PREDEBUG_OPPONENT,	// 2 lines: standard for player + opponent
    PREDEBUG_VERTICAL,	// vertical output
}
predef_debug_t;

//-----------------------------------------------------------------------------

ccp GetPredefDebugName ( predef_debug_t pd_mode );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    le_lpar_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_lpar_t]] [[new-lpar]]

typedef struct le_lpar_t
{
    lpar_mode_t limit_mode;	// limit of other params

    u16 thcloud_frames;		// number of frames a player is small after thundercloud hit

    u8  cheat_mode;		// 0:off, 1:debug only, 2:allow all
    u8	engine[3];		// 100cc, 150cc, mirror (sum always 100)
    u8	enable_200cc;		// TRUE: 200C enabled => 150cc, 200cc, mirror
    u8	enable_perfmon;		// >0: performance monitor enabled; >1: for dolphin too
    u8	enable_custom_tt;	// TRUE: time trial for cusotm tracks enabled
    u8	enable_xpflags;		// TRUE: extended presence flags enabled
    u8	enable_speedo;		// speedometer selection (0..3), see SPEEDO_*
    u8	block_track;		// block used tracks for 0..20 races
    u8	no_speedo_if_debug;	// 1 bit for each debug configuration
    u8  item_cheat;		// 0:disabled, 1:enabled
    u8  drag_blue_shell;	// >0: allow dragging of blue shell

    u8  debug_mode;		// debug mode
    u8  debug_predef[LEDEB__N_CONFIG];
				// information about used predefined mode
    u32 debug[LEDEB__N_CONFIG][LEDEB__N_LINE];
				// debug line settings, see LEDEB_*

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

//-----------------------------------------------------------------------------

enumError SaveTextLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

enumError WriteSectionLPAR
(
    FILE		*f,		// output files
    const le_lpar_t	*lpar		// LE-CODE parameters
);

//-----------------------------------------------------------------------------

bool SetupLecodeDebugLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    uint		config,		// configuration index 0..
    uint		setup_mode,	// setup mode, one of PREDEBUG_*
    int			*hide_speedo	// if not NULL: store new mode here
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
 /*2c*/  u32  off_flags;	// offset of flags

 /*30*/  u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*33*/	 u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*34*/	 u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*35*/
}
__attribute__ ((packed)) le_binpar_v1_35_t;

_Static_assert(sizeof(le_binpar_v1_35_t)==0x35,"le_binpar_v1_35_t");

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
 /*2c*/  u32  off_flags;	// offset of flags

 /*30*/  u8   engine[3];	// 100cc, 150cc, mirror (sum always 100)
 /*33*/	 u8   enable_200cc;	// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*34*/	 u8   enable_perfmon;	// TRUE: performance monitor enabled
 /*35*/	 u8   enable_custom_tt;	// TRUE: time trial for cusotm tracks enabled
 /*36*/	 u8   enable_xpflags;	// TRUE: extended presence flags enabled
 /*37*/
}
__attribute__ ((packed)) le_binpar_v1_37_t;

_Static_assert(sizeof(le_binpar_v1_37_t)==0x37,"le_binpar_v1_37_t");

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
 /*2c*/  u32  off_flags;	// offset of flags

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

_Static_assert(sizeof(le_binpar_v1_f8_t)==0xf8,"le_binpar_v1_f8_t");

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
 /*02c*/ u32  off_flags;	// offset of flags

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

_Static_assert(sizeof(le_binpar_v1_1b8_t)==0x1b8,"le_binpar_v1_1b8_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_1bc_t]]

typedef struct le_binpar_v1_1bc_t
{
 /*000*/ char magic[8];			// LE_PARAM_MAGIC
 /*008*/ u32  version;			// always 1 for v1
 /*00c*/ u32  size;			// size (and minor version)
 /*010*/ u32  off_eod;			// offset of end-of-data

 /*014*/ u32  off_cup_par;		// offset to cup param
 /*018*/ u32  off_cup_track;		// offset of cup-track list
 /*01c*/ u32  off_cup_arena;		// offset of cup-arena list
 /*020*/ u32  off_course_par;		// offset of course param
 /*024*/ u32  off_property;		// offset of property list
 /*028*/ u32  off_music;		// offset of music list
 /*02c*/ u32  off_flags;		// offset of flags

 /*030*/ u8   engine[3];		// 100cc, 150cc, mirror (sum always 100)
 /*033*/ u8   enable_200cc;		// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*034*/ u8   enable_perfmon;		// >0: performance monitor enabled; >1: for dolphin too
 /*035*/ u8   enable_custom_tt;		// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;		// TRUE: extended presence flags enabled
 /*037*/ u8   block_track;		// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT];	// mode for each chat message
 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT];	// mode for each chat message
 /*1b8*/ u8   enable_speedo;		// speedometer mode (0..2), see SPEEDO_*
 /*1b9*/ u8   no_speedo_if_debug;	// if bit is set: suppress speedometer
 /*1ba*/ u8   debug_mode;		// debug mode (0..), see DEBUGMD_*

 //--- reserved for future extensions
 /*1bb*/ u8   reserved_1bb;

 /*1bc*/
}
__attribute__ ((packed)) le_binpar_v1_1bc_t;

_Static_assert(sizeof(le_binpar_v1_1bc_t)==0x1bc,"le_binpar_v1_1bc_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_260_t]]

typedef struct le_binpar_v1_260_t
{
 /*000*/ char magic[8];			// LE_PARAM_MAGIC
 /*008*/ u32  version;			// always 1 for v1
 /*00c*/ u32  size;			// size (and minor version)
 /*010*/ u32  off_eod;			// offset of end-of-data

 /*014*/ u32  off_cup_par;		// offset to cup param
 /*018*/ u32  off_cup_track;		// offset of cup-track list
 /*01c*/ u32  off_cup_arena;		// offset of cup-arena list
 /*020*/ u32  off_course_par;		// offset of course param
 /*024*/ u32  off_property;		// offset of property list
 /*028*/ u32  off_music;		// offset of music list
 /*02c*/ u32  off_flags;		// offset of flags

 /*030*/ u8   engine[3];		// 100cc, 150cc, mirror (sum always 100)
 /*033*/ u8   enable_200cc;		// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*034*/ u8   enable_perfmon;		// >0: performance monitor enabled; >1: for dolphin too
 /*035*/ u8   enable_custom_tt;		// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;		// TRUE: extended presence flags enabled
 /*037*/ u8   block_track;		// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT];	// mode for each chat message
 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT];	// mode for each chat message
 /*1b8*/ u8   enable_speedo;		// speedometer selection (0..2), see SPEEDO_*
 /*1b9*/ u8   no_speedo_if_debug;	// if bit is set: suppress speedometer
 /*1ba*/ u8   debug_mode;		// debug mode (0..), see DEBUGMD_*
 /*1bb*/ u8   item_cheat;		// 0:disabled, 1:enabled

 /*1bc*/ u8   debug_predef[LEDEB__N_CONFIG];
					// information about used predefined mode
 /*1c0*/ u32  debug[LEDEB__N_CONFIG][LEDEB__N_LINE];
					// debug line settings, see LEDEB_*
 /*260*/
}
__attribute__ ((packed)) le_binpar_v1_260_t;

_Static_assert(sizeof(le_binpar_v1_260_t)==0x260,"le_binpar_v1_260_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_264_t]]

typedef struct le_binpar_v1_264_t
{
 /*000*/ char magic[8];			// LE_PARAM_MAGIC
 /*008*/ u32  version;			// always 1 for v1
 /*00c*/ u32  size;			// size (and minor version)
 /*010*/ u32  off_eod;			// offset of end-of-data

 /*014*/ u32  off_cup_par;		// offset to cup param
 /*018*/ u32  off_cup_track;		// offset of cup-track list
 /*01c*/ u32  off_cup_arena;		// offset of cup-arena list
 /*020*/ u32  off_course_par;		// offset of course param
 /*024*/ u32  off_property;		// offset of property list
 /*028*/ u32  off_music;		// offset of music list
 /*02c*/ u32  off_flags;		// offset of flags

 /*030*/ u8   engine[3];		// 100cc, 150cc, mirror (sum always 100)
 /*033*/ u8   enable_200cc;		// TRUE: 200C enabled => 150cc, 200cc, mirror
 /*034*/ u8   enable_perfmon;		// >0: performance monitor enabled; >1: for dolphin too
 /*035*/ u8   enable_custom_tt;		// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;		// TRUE: extended presence flags enabled
 /*037*/ u8   block_track;		// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT];	// mode for each chat message
 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT];	// mode for each chat message
 /*1b8*/ u8   enable_speedo;		// speedometer selection (0..2), see SPEEDO_*
 /*1b9*/ u8   no_speedo_if_debug;	// if bit is set: suppress speedometer
 /*1ba*/ u8   debug_mode;		// debug mode (0..), see DEBUGMD_*
 /*1bb*/ u8   item_cheat;		// 0:disabled, 1:enabled

 /*1bc*/ u8   debug_predef[LEDEB__N_CONFIG];
					// information about used predefined mode
 /*1c0*/ u32  debug[LEDEB__N_CONFIG][LEDEB__N_LINE];
					// debug line settings, see LEDEB_*

 /*260*/ u8   cheat_mode;		// 0:off, 1:debug only, 2:allow all
 /*261*/ u8   drag_blue_shell;		// >0: allow dragging of blue shell
 /*262*/ u16  thcloud_frames;		// number of frames a player is small after thundercloud hit

 /*264*/

}
__attribute__ ((packed)) le_binpar_v1_264_t;

_Static_assert(sizeof(le_binpar_v1_264_t)==0x264,"le_binpar_v1_264_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_t]] [[new-lpar]]

typedef struct le_binpar_v1_264_t le_binpar_v1_t;

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

    bool	arena_setup;	// set by SetupArenasLEAnalyse()
    bool	arena_applied;	// set by ApplyArena()


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
void SetupArenasLEAnalyse ( le_analyse_t *ana, bool force );

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
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsLECODE();

static inline bool IsRacingTrackLE ( uint tid )
	{ return tid < MKW_N_TRACKS
		|| tid >= LE_FIRST_LOWER_CT_SLOT && tid != 0xff; }

uint GetNextRacingTrackLE ( uint tid );

///////////////////////////////////////////////////////////////////////////////

extern ccp	opt_le_define;
extern ccp	opt_le_arena;
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
extern int	opt_speedo;
extern int	opt_lecode_debug;

extern bool	opt_complete;

int ScanOptTrackSource	( ccp arg, int mode );
int ScanOptAlias	( ccp arg );
int ScanOptEngine	( ccp arg );
int ScanOpt200cc	( ccp arg );
int ScanOptPerfMon	( ccp arg );
int ScanOptCustomTT	( ccp arg );
int ScanOptXPFlags	( ccp arg );
int ScanOptSpeedometer	( ccp arg );
int ScanOptDebug	( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_LECODE_H


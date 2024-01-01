
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

#define LE_RESERVED_SLOTS	(LE_FIRST_CT_SLOT - 32)
#define LE_MAX_BLOCK_TRACK	50

#define LE_DEF_ONLINE_SEC	340	//     5:40 : default online time limit
#define LE_MIN_ONLINE_SEC	 30	//     0:30 : min accepted online time limit
#define LE_MAX_WD_ONLINE_SEC	525	//     8:45 : max online time limit for enabled watchdog
#define LE_MAX_ONLINE_SEC	0xffff	// 18:12:15 : max possible online time limit
#define LE_VIEW_ONLINE_HMS	 61	//     1:01 : min value when time is printed as additonal HMS

//-----------------------------------------------------------------------------
// [[le_valid_t]]

typedef enum le_valid_t // bit field
{
    LE_HEAD_FOUND	= 0x01,
    LE_HEAD_VALID	= 0x02,
    LE_HEAD_KNOWN	= 0x04,
    LE_HEAD_VERSION	= 0x08,

    LE_PARAM_FOUND	= 0x10,
    LE_PARAM_VALID	= 0x20,
    LE_PARAM_KNOWN	= 0x40,

    LE_M_ALL_VALID	= 0x7f,
}
le_valid_t;

static inline bool IsLecodeSupported ( le_valid_t mode )
	{ return ( mode & LE_M_ALL_VALID ) == LE_M_ALL_VALID; }

//-----------------------------------------------------------------------------
// [[le_usage_t]] => edit usage_ch[] before GetLEUsageChar() on changes

typedef enum le_usage_t
{
    LEU_S_UNUSED =  0,	// - slot not used

    LEU_S_ARENA,	// A arena slot
    LEU_S_ARENA_HIDE,	// a arena slot
    LEU_S_ARENA_RND,	// r arena slot

    LEU_S_TRACK,	// T track slot
    LEU_S_TRACK_HIDE,	// t track slot
    LEU_S_TRACK_RND,	// R track slot

    LEU_S_LE_RANDOM,	// L LE-CODE random
    LEU_S_SPECIAL,	// s special slot
    LEU_S_NETWORK,	// n network slot
    LEU_S_ALIAS,	// > is alias	    <<< last element!

     LEU_S_MASK		= 0xf,

    LEU_F_ONLINE	= 0x10,  // Flag: used online
}
__attribute__ ((packed)) le_usage_t;

_Static_assert( LEU_S_ALIAS <= LEU_S_MASK,"le_usage_t");

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
    LEDEB_TRACK_ID	= 0x1000,
    // [[new-debug]]

    //--- derived values

    LEDEB__STD		= 0x1ff9,   // standard output
    LEDEB__OUTPUT	= 0x1ff8,   // flags that produce output
    LEDEB__ALL		= 0x1fff,   // all flags

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
    u8 track_id;
    u8 xpf;
    // [[new-debug]]

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
///////////////			enum le_region_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_region_t]]

typedef enum le_region_t
{
    LEREG_UNKNOWN = 0,

    LEREG_PAL,
    LEREG_USA,
    LEREG_JAP,
    LEREG_KOR,

    LEREG__BEG	= LEREG_PAL,
    LEREG__END	= LEREG_KOR,
    LEREG__N	= LEREG__END - LEREG__BEG + 1,
}
le_region_t;

//-----------------------------------------------------------------------------

le_region_t GetLEREG ( char ch ); // anylse first char only
ccp GetNameLEREG ( le_region_t lereg, ccp return_on_invalid );

// replace last '@' by region name.
// returns either NULL on error or 'name' or 'buf'. If !buf or to small, then use circ-buf.
ccp PatchNameLEREG ( char *buf, uint bufsize, ccp name, le_region_t lereg );

const mem_t GetLecodeLEREG ( le_region_t reg );

//-----------------------------------------------------------------------------
// some more global functions, used as helpers for *LEREG functions

// returns circ-buf for strings
ccp	GetLecodeBuildLEREG	( le_region_t reg );
ccp	GetLecodeInfoLEREG	( le_region_t reg );
u_sec_t	GetLECommitTimeLEREG	( le_region_t reg );
u_sec_t	GetLECreateTimeLEREG	( le_region_t reg );
u_sec_t	GetLERefTimeLEREG	( le_region_t reg );

// returns circ-buf for strings
ccp	GetInfoLECODE		( mem_t lecode );
ccp	GetBuildLECODE		( mem_t lecode );
u_sec_t	GetCommitTimeLECODE	( mem_t lecode );
u_sec_t	GetCreateTimeLECODE	( mem_t lecode );
u_sec_t	GetRefTimeLECODE	( mem_t lecode );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    le_lpar_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_lpar_t]] [[new-lpar]]

typedef struct le_lpar_t
{
    lpar_mode_t limit_mode;	// limit of other params

    u16 thcloud_frames;		// number of frames a player is small after thundercloud hit
    u16 default_online_sec;	// default time limit in seconds
    u16 min_online_sec;		// minimal time limit in seconds, that can be set by LEX:SET1
    u16 max_online_sec;		// maximal time limit in seconds, that can be set by LEX:SET1

    u8	developer_modes;	// >0: developer settings are recognized and accepted.
    u8	dev_mode1;		// First developer mode
    u8	dev_mode2;		// Second developer mode
    u8	dev_mode3;		// Third developer mode

    u8	cheat_mode;		// 0:off, 1:debug only, 2:allow all
    u8	engine[3];		// 100cc, 150cc, mirror (sum always 100)
    u8	enable_200cc;		// TRUE: 200C enabled => 150cc, 200cc, mirror
    u8	enable_perfmon;		// >0: performance monitor enabled; >1: for dolphin too
    u8	enable_custom_tt;	// TRUE: time trial for cusotm tracks enabled
    u8	enable_xpflags;		// TRUE: extended presence flags enabled
    u8	enable_speedo;		// speedometer selection (0..3), see SPEEDO_*
    u8	block_track;		// block used tracks for 0..20 races
    u8	no_speedo_if_debug;	// 1 bit for each debug configuration
    u8	item_cheat;		// 0:disabled, 1:enabled
    u8	drag_blue_shell;	// >0: allow dragging of blue shell
    u8	bt_worldwide;		// >0: enable worldwide battles
    u8	vs_worldwide;		// >0: enable worldwide versus races, >1: extended
    u8	bt_textures;		// &1: enable texture hacks for battles, &2:alternable
    u8	vs_textures;		// &1: enable texture hacks for versus, &2:alternable
    u8	block_textures;		// >0: enable blocking of recent texture hacks
    u8	staticr_points;		// >0: use points definied by StaticR.rel
    u8	use_avail_txt;		// unused: // >0: use file "avail.txt" to detect dir "Race/Common/###/"
    u8 cup_icon_size;		// size of single cup icon, usually 128 (128x128)

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
static inline void ResetLPAR ( le_lpar_t *lpar ) {}

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

enumError SaveTextFileLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    FILE		*f,		// open file
    bool		print_full	// true: print ehader + append section [END]
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
// [[le_binary_head_v3_t]]

typedef struct le_binary_head_v3_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 4 for v4
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	file_size;	// size of complete file
 /*18*/  u32	off_param;	// offset of LPAR section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	build_mode;	// one of: D (debug) or R (release)
 /*1e*/  u8	phase;		// >0: PHASE
 /*1f*/  char	unknown_1f;
}
__attribute__ ((packed)) le_binary_head_v3_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v4_t]]

typedef struct le_binary_head_v4_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 4 for v4
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	file_size;	// size of complete file
 /*18*/  u32	off_param;	// offset of LPAR section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	build_mode;	// one of: D (debug) or R (release)
 /*1e*/  u8	phase;		// >0: PHASE
 /*1f*/  char	unknown_1f;
 /*20*/  char	timestamp[];	// timestamp of build in ASCII
}
__attribute__ ((packed)) le_binary_head_v4_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v5_34_t]]

typedef struct le_binary_head_v5_34_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 5 for v5
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	file_size;	// size of complete file
 /*18*/  u32	off_param;	// offset of LPAR section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	build_mode;	// one of: D (debug) or R (release)
 /*1e*/  u8	phase;		// >0: PHASE (usually 2)
 /*1f*/  char	unknown_1f;

 /*20*/  u32	szs_required;	// minimal encoded version number of szs-tools required to edit
 /*24*/  u32	edit_version;	// >0: encoded version number of szs-tools, that did last edit
 /*28*/	 u32	head_size;	// size of file header
 /*2c*/	 u32	creation_time;	// unixtime of LE-CODE creation
 /*30*/	 u32	edit_time;	// >0: unixtime of last LE-CODE edit
 /*34*/
}
__attribute__ ((packed)) le_binary_head_v5_34_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v5_38_t]]

typedef struct le_binary_head_v5_38_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 5 for v5
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	file_size;	// size of complete file
 /*18*/  u32	off_param;	// offset of LPAR section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	build_mode;	// one of: R (release), T (testcode), D (debug) or X (debug+testcode)
 /*1e*/  u8	phase;		// >0: PHASE (usually 2)
 /*1f*/  char	unknown_1f;

 /*20*/  u32	szs_required;	// minimal encoded version number of szs-tools required to edit
 /*24*/  u32	edit_version;	// >0: encoded version number of szs-tools, that did last edit
 /*28*/	 u32	head_size;	// size of file header
 /*2c*/	 u32	creation_time;	// unixtime of LE-CODE creation
 /*30*/	 u32	edit_time;	// >0: unixtime of last LE-CODE edit
 /*34*/  u32	szs_recommended;// recommended version number of szs-tools to manage LE binaries
 /*38*/
}
__attribute__ ((packed)) le_binary_head_v5_38_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v5_3c_t]]

typedef struct le_binary_head_v5_3c_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 5 for v5
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	file_size;	// size of complete file
 /*18*/  u32	off_param;	// offset of LPAR section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	build_mode;	// one of: R (release), T (testcode), D (debug) or X (debug+testcode)
 /*1e*/  u8	phase;		// >0: PHASE (usually 2)
 /*1f*/  char	unknown_1f;

 /*20*/  u32	szs_required;	// minimal encoded version number of szs-tools required to edit
 /*24*/  u32	edit_version;	// >0: encoded version number of szs-tools, that did last edit
 /*28*/	 u32	head_size;	// size of file header
 /*2c*/	 u32	creation_time;	// unixtime of LE-CODE creation
 /*30*/	 u32	edit_time;	// >0: unixtime of last LE-CODE edit
 /*34*/  u32	szs_recommended;// recommended version number of szs-tools to manage LE binaries
 /*38*/  u32	commit_time;	// unixtime of "git commit"
 /*3c*/
}
__attribute__ ((packed)) le_binary_head_v5_3c_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v5_44_t]]

typedef struct le_binary_head_v5_44_t
{
 /*00*/  char	magic[4];	// LE_BINARY_MAGIC
 /*04*/  u32	version;	// always 5 for v5
 /*08*/  u32	build_number;	// current code version
 /*0c*/  u32	base_address;	// memory address of binary (destination)
 /*10*/  u32	entry_point;	// memory address (prolog function)
 /*14*/  u32	file_size;	// size of complete file
 /*18*/  u32	off_param;	// offset of LPAR section
 /*1c*/  char	region;		// one of: P, E, J, K
 /*1d*/  char	build_mode;	// one of: R (release), T (testcode), D (debug) or X (debug+testcode)
 /*1e*/  u8	phase;		// >0: PHASE (usually 2)
 /*1f*/  char	unknown_1f;

 /*20*/  u32	szs_required;	// minimal encoded version number of szs-tools required to edit
 /*24*/  u32	edit_version;	// >0: encoded version number of szs-tools, that did last edit
 /*28*/	 u32	head_size;	// size of file header
 /*2c*/	 u32	creation_time;	// unixtime of LE-CODE creation
 /*30*/	 u32	edit_time;	// >0: unixtime of last LE-CODE edit
 /*34*/  u32	szs_recommended;// recommended version number of szs-tools to manage LE binaries
 /*38*/  u32	commit_time;	// unixtime of "git commit"
 /*3c*/  u32	off_signature;	// offset of LE-CODE signature
 /*40*/  u32	size_signature;	// size of buffer for LE-CODE signature
 /*44*/
}
__attribute__ ((packed)) le_binary_head_v5_44_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_v5_t]] [[new-le-header]]

typedef le_binary_head_v5_44_t le_binary_head_v5_t;

//-----------------------------------------------------------------------------
// [[le_binary_head_t]]

typedef union
{
    le_binary_head_v3_t	v3;
    le_binary_head_v4_t	v4;
    le_binary_head_v5_t	v5;
}
le_binary_head_t;

u_sec_t GetCommitTimeLeHead ( const le_binary_head_t *head );
u_sec_t GetCreateTimeLeHead ( const le_binary_head_t *head );
u_sec_t GetRefTimeLeHead    ( const le_binary_head_t *head );

// returns pointer to signature (.ptr) and size of buffer (.len)
mem_t GetLecodeSignature ( const le_binary_head_t *head );

//-----------------------------------------------------------------------------

static inline bool IsBuildModeRelease ( char ch )
	{ return ch == 'R' || ch == 'T'; }

static inline bool IsBuildModeDebug ( char ch )
	{ return ch == 'D' || ch == 'X'; }

static inline bool IsBuildModeTest ( char ch )
	{ return ch == 'T' || ch == 'X'; }

static inline bool IsBuildModeUnknown ( char ch )
	{ return ch != 'R' || ch != 'T' || ch != 'D' || ch != 'X'; }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_binary_param_t]]

typedef struct le_binary_param_t
{
 /*00*/  char	magic[8];	// LE_PARAM_MAGIC ("LPAR")
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
 /*00*/  char magic[8];		// LE_PARAM_MAGIC ("LPAR")
 /*08*/  u32  version;		// always 1 for v1
 /*0c*/  u32  size;		// size (and minor version)
 /*10*/  u32  off_eod;		// offset of end-of-data

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
 /*00*/  char magic[8];		// LE_PARAM_MAGIC ("LPAR")
 /*08*/  u32  version;		// always 1 for v1
 /*0c*/  u32  size;		// size (and minor version)
 /*10*/  u32  off_eod;		// offset of end-of-data

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
 /*00*/  char magic[8];		// LE_PARAM_MAGIC ("LPAR")
 /*08*/  u32  version;		// always 1 for v1
 /*0c*/  u32  size;		// size (and minor version)
 /*10*/  u32  off_eod;		// offset of end-of-data

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
 /*000*/ char magic[8];		// LE_PARAM_MAGIC ("LPAR")
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
 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
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
 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
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
 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
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
// [[le_binpar_v1_269_t]]

typedef struct le_binpar_v1_269_t
{
 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
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

 /*264*/ u8   bt_worldwide;		// >0: enable worldwide battles
 /*265*/ u8   vs_worldwide;		// >0: enable worldwide versus races
 /*266*/ u8   bt_textures;		// &1: enable texture hacks for battles, &2:alternable
 /*267*/ u8   vs_textures;		// &1: enable texture hacks for versus, &2:alternable
 /*268*/ u8   block_textures;		// >0: enable blocking of recent texture hacks

 /*269*/

}
__attribute__ ((packed)) le_binpar_v1_269_t;

_Static_assert(sizeof(le_binpar_v1_269_t)==0x269,"le_binpar_v1_269_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_26a_t]]

typedef struct le_binpar_v1_26a_t
{
 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
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

 /*264*/ u8   bt_worldwide;		// >0: enable worldwide battles
 /*265*/ u8   vs_worldwide;		// >0: enable worldwide versus races
 /*266*/ u8   bt_textures;		// &1: enable texture hacks for battles, &2:alternable
 /*267*/ u8   vs_textures;		// &1: enable texture hacks for versus, &2:alternable
 /*268*/ u8   block_textures;		// >0: enable blocking of recent texture hacks
 /*269*/ u8   staticr_points;		// >0: use points definied by StaticR.rel

 /*26a*/

}
__attribute__ ((packed)) le_binpar_v1_26a_t;

//-----------------------------------------------------------------------------
// [[le_binpar_v1_270_t]]

typedef struct le_binpar_v1_270_t
{
 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
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

 /*264*/ u8   bt_worldwide;		// >0: enable worldwide battles
 /*265*/ u8   vs_worldwide;		// >0: enable worldwide versus races
 /*266*/ u8   bt_textures;		// &1: enable texture hacks for battles, &2:alternable
 /*267*/ u8   vs_textures;		// &1: enable texture hacks for versus, &2:alternable
 /*268*/ u8   block_textures;		// >0: enable blocking of recent texture hacks
 /*269*/ u8   staticr_points;		// >0: use points definied by StaticR.rel

 /*26a*/ u16  default_online_sec;	// default time limit in seconds
 /*26c*/ u16  min_online_sec;		// minimal time limit in seconds, that can be set by LEX:SET1
 /*26e*/ u16  max_online_sec;		// maximal time limit in seconds, that can be set by LEX:SET1

 /*270*/
}
__attribute__ ((packed)) le_binpar_v1_270_t;

_Static_assert(sizeof(le_binpar_v1_270_t)==0x270,"le_binpar_v1_270_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_274_t]]

typedef struct le_binpar_v1_274_t
{
 //--- le_binary_param_t

 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
 /*008*/ u32  version;			// always 1 for v1
 /*00c*/ u32  size;			// size (and minor version)
 /*010*/ u32  off_eod;			// offset of end-of-data

 //--- le_binpar_v1_35_t

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

 //--- le_binpar_v1_37_t

 /*035*/ u8   enable_custom_tt;		// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;		// TRUE: extended presence flags enabled

 //--- le_binpar_v1_f8_t

 /*037*/ u8   block_track;		// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT];	// mode for each chat message

 //--- le_binpar_v1_1b8_t

 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT];	// mode for each chat message

 //--- le_binpar_v1_1bc_t

 /*1b8*/ u8   enable_speedo;		// speedometer selection (0..2), see SPEEDO_*
 /*1b9*/ u8   no_speedo_if_debug;	// if bit is set: suppress speedometer
 /*1ba*/ u8   debug_mode;		// debug mode (0..), see DEBUGMD_*

 //--- le_binpar_v1_260_t

 /*1bb*/ u8   item_cheat;		// 0:disabled, 1:enabled

 /*1bc*/ u8   debug_predef[LEDEB__N_CONFIG];
					// information about used predefined mode
 /*1c0*/ u32  debug[LEDEB__N_CONFIG][LEDEB__N_LINE];
					// debug line settings, see LEDEB_*

 //--- le_binpar_v1_264_t

 /*260*/ u8   cheat_mode;		// 0:off, 1:debug only, 2:allow all
 /*261*/ u8   drag_blue_shell;		// >0: allow dragging of blue shell
 /*262*/ u16  thcloud_frames;		// number of frames a player is small after thundercloud hit

 //--- le_binpar_v1_269_t

 /*264*/ u8   bt_worldwide;		// >0: enable worldwide battles
 /*265*/ u8   vs_worldwide;		// >0: enable worldwide versus races
 /*266*/ u8   bt_textures;		// &1: enable texture hacks for battles, &2:alternable
 /*267*/ u8   vs_textures;		// &1: enable texture hacks for versus, &2:alternable
 /*268*/ u8   block_textures;		// >0: enable blocking of recent texture hacks

 //--- le_binpar_v1_26a_t

 /*269*/ u8   staticr_points;		// >0: use points definied by StaticR.rel

 //--- le_binpar_v1_270_t

 /*26a*/ u16  default_online_sec;	// default time limit in seconds
 /*26c*/ u16  min_online_sec;		// minimal time limit in seconds, that can be set by LEX:SET1
 /*26e*/ u16  max_online_sec;		// maximal time limit in seconds, that can be set by LEX:SET1

 //--- le_binpar_v1_274_t

 /*270*/ u8   developer_modes;		// >0: developer settings are recognized and accepted.
 /*271*/ u8   dev_mode1;		// First developer mode
 /*272*/ u8   dev_mode2;		// Second developer mode
 /*273*/ u8   dev_mode3;		// Third developer mode

 //--- END

 /*274*/
}
__attribute__ ((packed)) le_binpar_v1_274_t;

_Static_assert(sizeof(le_binpar_v1_274_t)==0x274,"le_binpar_v1_274_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_276_t]]

typedef struct le_binpar_v1_276_t
{
 //--- le_binary_param_t

 /*000*/ char magic[8];			// LE_PARAM_MAGIC ("LPAR")
 /*008*/ u32  version;			// always 1 for v1
 /*00c*/ u32  size;			// size (and minor version)
 /*010*/ u32  off_eod;			// offset of end-of-data

 //--- le_binpar_v1_35_t

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

 //--- le_binpar_v1_37_t

 /*035*/ u8   enable_custom_tt;		// TRUE: time trial for cusotm tracks enabled
 /*036*/ u8   enable_xpflags;		// TRUE: extended presence flags enabled

 //--- le_binpar_v1_f8_t

 /*037*/ u8   block_track;		// block used track for 0.. tracks
 /*038*/ u16  chat_mode_1[BMG_N_CHAT];	// mode for each chat message

 //--- le_binpar_v1_1b8_t

 /*0f8*/ u16  chat_mode_2[BMG_N_CHAT];	// mode for each chat message

 //--- le_binpar_v1_1bc_t

 /*1b8*/ u8   enable_speedo;		// speedometer selection (0..2), see SPEEDO_*
 /*1b9*/ u8   no_speedo_if_debug;	// if bit is set: suppress speedometer
 /*1ba*/ u8   debug_mode;		// debug mode (0..), see DEBUGMD_*

 //--- le_binpar_v1_260_t

 /*1bb*/ u8   item_cheat;		// 0:disabled, 1:enabled

 /*1bc*/ u8   debug_predef[LEDEB__N_CONFIG];
					// information about used predefined mode
 /*1c0*/ u32  debug[LEDEB__N_CONFIG][LEDEB__N_LINE];
					// debug line settings, see LEDEB_*

 //--- le_binpar_v1_264_t

 /*260*/ u8   cheat_mode;		// 0:off, 1:debug only, 2:allow all
 /*261*/ u8   drag_blue_shell;		// >0: allow dragging of blue shell
 /*262*/ u16  thcloud_frames;		// number of frames a player is small after thundercloud hit

 //--- le_binpar_v1_269_t

 /*264*/ u8   bt_worldwide;		// >0: enable worldwide battles
 /*265*/ u8   vs_worldwide;		// >0: enable worldwide versus races
 /*266*/ u8   bt_textures;		// &1: enable texture hacks for battles, &2:alternable
 /*267*/ u8   vs_textures;		// &1: enable texture hacks for versus, &2:alternable
 /*268*/ u8   block_textures;		// >0: enable blocking of recent texture hacks

 //--- le_binpar_v1_26a_t

 /*269*/ u8   staticr_points;		// >0: use points definied by StaticR.rel

 //--- le_binpar_v1_270_t

 /*26a*/ u16  default_online_sec;	// default time limit in seconds
 /*26c*/ u16  min_online_sec;		// minimal time limit in seconds, that can be set by LEX:SET1
 /*26e*/ u16  max_online_sec;		// maximal time limit in seconds, that can be set by LEX:SET1

 //--- le_binpar_v1_274_t

 /*270*/ u8   developer_modes;		// >0: developer settings are recognized and accepted.
 /*271*/ u8   dev_mode1;		// First developer mode
 /*272*/ u8   dev_mode2;		// Second developer mode
 /*273*/ u8   dev_mode3;		// Third developer mode

 //--- le_binpar_v1_275_t
 // not longer used
 /*274*/ u8   use_avail_txt;		// >0: use file "avail.txt" to detect dir "Race/Common/###/"

 //--- le_binpar_v1_276_t

 /*275*/ u8   cup_icon_size;		// Size of single cup icon, usually 128 (128x128)

 //--- END

 /*276*/
}
__attribute__ ((packed)) le_binpar_v1_276_t;

_Static_assert(sizeof(le_binpar_v1_276_t)==0x276,"le_binpar_v1_276_t");

//-----------------------------------------------------------------------------
// [[le_binpar_v1_t]] [[new-lpar]]

typedef struct le_binpar_v1_276_t le_binpar_v1_t;

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
///////////////			le_analyze_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_analyze_t]]

typedef struct le_analyze_t
{
    //--- header data

    le_valid_t	valid;		// see LE_VALID_* above
    le_region_t	region;		// one of LEREG_*
    u32		commit_time;	// unixtime of last 'git commit'
    u32		creation_time;	// unixtime of LE-CODE creation
    u32		edit_time;	// >0: unixtime of last LE-CODE edit
    uint	szs_required;	// minimal encoded version number of szs-tools required to edit
    uint	szs_recommended;// >0: recommended version number of szs-tools to manage LE binaries
    uint	edit_version;	// >0: encoded version number of szs-tools, that did last edit
    char	identifier[20];	// identifier of format "v#-b###-h###-<debug><region>"

    const u8	*data;		// pointer to raw data
    u32		data_size;	// data size

    uint	header_vers;	// not NULL: version number of header
    uint	header_size;	// not NULL: size told by header

    uint	param_offset;	// not NULL: offset of LPAR
    uint	param_vers;	// not NULL: version number of LPAR
    uint	param_size;	// not NULL: (max usable) param size


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
    uint	flags_bits;	// number of bits for flags: 8|16

    uint	used_rslots;	// used racing slots
    uint	max_rslots;	// max racing slots
    uint	used_bslots;	// used battle slots
    uint	max_bslots;	// max battle slots

    uint	le_random_beg;	// first LE-CODE random slot
    uint	le_random_end;	// last LE-CODE random slot + 1

    le_lpar_t	lpar;		// LPAR parameters
    le_flags_t	*flags;		// alloced list of flags, N=max_flags

    le_usage_t	*usage;		// NULL or alloced list[n_slot], setup by GetLEUsage()
    uint	usage_size;	// number of alloed 'usage' elements

    bool	arena_setup;	// set by SetupArenasLEAnalyze()
    bool	arena_applied;	// set by ApplyArena()


    //--- binary data

    u8		*bin_data;	// pointer to begin of binary data
    uint	bin_size;	// size of binary data


    //--- pointers into LE-CODE binary

    le_binary_head_t	*head;		// LECT file header
    le_cup_par_t	*cup_par;	// NULL or cup parameters
    le_course_par_t	*course_par;	// NULL or course parameters
    le_cup_track_t	*cup_track;	// NULL or list of cup tracks
    le_cup_track_t	*cup_arena;	// NULL or list of cup arenas
    le_property_t	*property;	// NULL or list of properties
    le_music_t		*music;		// NULL or list of music slots
    le_flags8_t		*flags_bin;	// NULL or list of flags

    u8			*beg_of_data;	// NULL or begin of data
    u8			*end_of_data;	// NULL or end of data

    union
    {
	le_binary_param_t	* param;
	le_binpar_v1_t		* param_v1;
    };
}
le_analyze_t;

//-----------------------------------------------------------------------------

void ResetLEAnalyze ( le_analyze_t *ana );
void ResetLEAnalyzeUsage ( le_analyze_t *ana );
void SetupArenasLEAnalyze ( le_analyze_t *ana, bool force );

enumError AnalyzeLEBinary
(
    le_analyze_t	*ana,		// NULL or destination of analysis
    const void		*data,		// data pointer
    uint		data_size	// data size
);

ccp GetLecodeSupportWarning ( const le_analyze_t *ana );
le_region_t GetLERegion ( const le_analyze_t *ana );

void CalculateStatsLE ( le_analyze_t *ana );
const le_usage_t * GetLEUsage ( const le_analyze_t *ana0, bool force_recalc );
char GetLEUsageChar ( le_usage_t usage );
ccp GetLEUsageCharCol ( le_usage_t usage, const ColorSet_t *colset, ccp * prev_col );

ccp GetLEValid ( le_valid_t valid );
ccp GetLEInValid ( le_valid_t valid );
void CalculateStatsLE ( le_analyze_t *ana );
void DumpLEAnalyse ( FILE *f, uint indent, const le_analyze_t *ana );

ccp FindTrackFile ( ccp src_name, TransferMode_t *tfer_mode ); // returns NULL or alloced path
void TransferTrackFile   ( LogFile_t *log, uint dest_slot, ccp  src_name, TransferMode_t flags );
void TransferTrackBySlot ( LogFile_t *log, uint dest_slot, uint src_slot, TransferMode_t flags );

static inline bool IsLESlotUsed ( const le_analyze_t *ana, uint slot )
	{ return slot < ana->n_slot && ( ana->music[slot] || ana->flags[slot] & G_LEFL__USED ); }

static inline bool IsLERacingSlot ( const le_analyze_t *ana, uint slot )
	{ return IsLESlotUsed(ana,slot) && ana->property[slot] < MKW_TRACK_END; }

static inline bool IsLEBattleSlot ( const le_analyze_t *ana, uint slot )
	{ return IsLESlotUsed(ana,slot) && ana->property[slot] >= MKW_ARENA_BEG; }

///////////////////////////////////////////////////////////////////////////////

const ctcode_t * LoadLEFile ( uint le_phase );

enumError ApplyLEFile ( le_analyze_t * ana );
enumError ApplyCTCODE ( le_analyze_t * ana, const ctcode_t * ctcode );
enumError PatchLECODE ( le_analyze_t * ana );
void PatchLPAR ( le_lpar_t * lp );

void UpdateLecodeFlags	( le_analyze_t * ana );
void CopyLecodeFlags	( le_analyze_t * ana );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsLECODE();

static inline bool IsRacingTrackLE ( uint tid )
	{ return tid < MKW_N_TRACKS || tid >= LE_FIRST_CT_SLOT; }

uint GetNextRacingTrackLE ( uint tid );

///////////////////////////////////////////////////////////////////////////////

extern ccp	opt_le_arena;
extern ccp	opt_lpar;
extern ccp	opt_track_dest;
extern ParamField_t opt_track_source;
extern int	opt_szs_mode;

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

int ScanOptTrackSource	( ccp arg, int arg_len, int mode );
int ScanOptSzsMode	( ccp arg );
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


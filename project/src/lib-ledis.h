
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

#ifndef SZS_LIB_LEDIS_H
#define SZS_LIB_LEDIS_H 1

#include "lib-std.h"
#include "lib-mkw.h"
#include "lib-ctcode.h"
#include "lib-lecode.h"

#include "dclib-parser.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			support options			///////////////
///////////////////////////////////////////////////////////////////////////////

// If 1, then enable the string list 'le_track_t::str_list' with some
// predifined string types.

#define LE_STRING_LIST_ENABLED 1

//-----------------------------------------------------------------------------

// If 1, then enable the string set 'le_track_t::str_set'. It supports any
// number of strings that can be accessed by '[name]'.

#define LE_STRING_SET_ENABLED 1

///////////////////////////////////////////////////////////////////////////////

// if 1, then instruction 'patch=...' is enabled (in development)

#define LE_DIS_PATCH_ENABLED 0

//-----------------------------------------------------------------------------

// if 1, then instruction 'sort-tracks=...' is enabled (in development)

#define LE_DIS_SORTTRACKS_ENABLED 0

//-----------------------------------------------------------------------------

// if 1, then instruction 'sort-cups=...' is enabled (in development)

#define LE_DIS_SORTCUPS_ENABLED 0

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define LE_DEFINE_MAGIC8	"#LE-DEF1"
#define LE_DEFINE_MAGIC8_NUM	0x234c452d44454631ull

#define LE_DISTRIB_MAGIC8	"#LE-DIST"
#define LE_DISTRIB_MAGIC8_NUM	0x234c452d44495354ull

#define LE_REFERENCE_MAGIC8	"#LE-REF1"
#define LE_REFERENCE_MAGIC8_NUM	0x234c452d52454631ull
#define LE_REFERENCE_VERSION	1

#define LE_STRINGS_MAGIC8	"#LE-STR1"
#define LE_STRINGS_MAGIC8_NUM	0x234c452d53545231ull
#define LE_STRINGS_VERSION	1

#define LE_SHA1REF_MAGIC8	"#SHA1REF"
#define LE_SHA1REF_MAGIC8_NUM	0x2353484131524546ull
#define LE_SHA1REF_VERSION	2

#define LE_PREFIX_MAGIC8	"#PREFIX1"
#define LE_PREFIX_MAGIC8_NUM	0x2350524546495831ull

#define LE_MTCAT_MAGIC8		"#MTCAT03"
#define LE_MTCAT_MAGIC8_NUM	0x234d544341543033ull

#define LE_CT_SHA1_MAGIC8	"#CT-SHA1"
#define LE_CT_SHA1_MAGIC8_NUM	0x2343542d53484131ull

#define LE_MAX_TRACKS		(MKW_MAX_TRACK_SLOT+1)
#define LE_TRACK_STRING_MAX	500

//-----------------------------------------------------------------------------

// [[le_group_t]] [[track_slot_t]] [[cup_slot_t]]
typedef u16			le_group_t;
typedef u16			track_slot_t;
typedef u16			cup_slot_t;

typedef struct le_track_t	le_track_t;
typedef struct le_distrib_t	le_distrib_t;
typedef struct raw_data_t	raw_data_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			global vars			///////////////
///////////////////////////////////////////////////////////////////////////////

extern ccp		opt_create_dump;	// NULL of filename
extern le_distrib_t	ledis_dump;		// data collector for 'opt_create_dump'
extern int		ledis_dump_enabled;	// -1:done, 0:disabled, 1:enabled

extern ccp		opt_le_define;		// NULL or last defined filename
extern StringField_t	le_define_list;		// list with all filenames

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_track_type_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_track_type_t]]

typedef enum le_track_type_t
{
    LTTY_NONE	= 0x00,
    LTTY_ARENA	= 0x01,  // true: is a battle arena
    LTTY_TRACK	= 0x02,  // true: is a versus track
    LTTY_EXTRA	= 0x04,  // true: is a MKW extra slot
    LTTY_RANDOM	= 0x08,  // true: is a LE-CODE random slot
    LTTY__NEXT	= 0x10,  // next free bit
}
__attribute__ ((packed)) le_track_type_t;

//-----------------------------------------------------------------------------

static inline bool IsArenaLTTY ( le_track_type_t track_type )
	{ return ( track_type & LTTY_ARENA ) > 0; }

static inline bool IsTrackLTTY ( le_track_type_t track_type )
	{ return ( track_type & LTTY_TRACK ) > 0; }

static inline bool IsRandomLTTY ( le_track_type_t track_type )
	{ return ( track_type & LTTY_RANDOM ) > 0; }

static inline bool IsArenaOrRandomLTTY ( le_track_type_t track_type )
	{ return ( track_type & (LTTY_ARENA|LTTY_RANDOM) ) > 0; }

static inline bool IsTrackOrRandomLTTY ( le_track_type_t track_type )
	{ return ( track_type & (LTTY_TRACK|LTTY_RANDOM) ) > 0; }


ccp  GetNameLTTY ( le_track_type_t track_type );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_track_status_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_track_status_t]]

typedef enum le_track_status_t
{
    LTS_INVALID = 0,	// record not initalized
    LTS_VALID,		// record initalized, but inactive
    LTS_FAIL,		// record initalized and inactive with any invalid settings
    LTS_ACTIVE,		// record initalized and active
    LTS_FILL,		// record initalized and active and used as fill-track
    LTS_EXPORT,		// record initalized and active and can be exported
    LTS__N
}
__attribute__ ((packed)) le_track_status_t;

ccp GetNameLTS ( le_track_status_t lts );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_track_text_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_track_text_t]]

typedef enum le_track_text_t
{
    LTT_UNDEFINED = 0,	// always NULL to support GetTextLEO()
    LTT_SHA1,		// access SHA1 checksum (standard)
    LTT_SHA1_D,		// access SHA1 checksum (_d file)
    LTT_IDENT,		// access identification (standard)
    LTT_IDENT_D,	// access identification (_d file)
    LTT_FILE,		// access file name
    LTT_PATH,		// access full path of LTT_FILE
    LTT_NAME,		// access name
    LTT_XNAME,		// access xname
    LTT_XNAME2,		// access xname with fallback to name

 #if LE_STRING_LIST_ENABLED
    // part of string-list, accesses by 'str_list[index-LTT__LIST_BEG]'
    LTT_TEMP1,		// temporary string
    LTT_TEMP2,		// temporary string
 #endif

 #if LE_STRING_SET_ENABLED
    LTT_STRSET,		// access str_set[name]
 #endif

    LTT__N,		// number of modes
    LTT__DEFAULT	= LTT_NAME,

 #if LE_STRING_LIST_ENABLED
    // pool range
    LTT__LIST_BEG	= LTT_TEMP1,
    LTT__LIST_END	= LTT_TEMP2+1,
    LTT__LIST_SIZE	= LTT__LIST_END - LTT__LIST_BEG,
 #endif
}
le_track_text_t;

//-----------------------------------------------------------------------------

ccp GetUpperNameLTT ( le_track_text_t ltt ); // 'ltt' is masked by LEO_LTT_SELECTOR
ccp GetLowerNameLTT ( le_track_text_t ltt ); // 'ltt' is masked by LEO_LTT_SELECTOR

// returns 0:unknown ltt, 1:std string, 2:_d string, 3:both
int GetDModeLTT ( le_track_text_t ltt );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_options_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_options_t]]

typedef enum le_options_t
{
    //-- text selector

    LEO_LTT_SELECTOR	= 0x0000000f,		// mask to store le_track_text_t selector

    LEO_UNDEFINED	= LTT_UNDEFINED,	// always NULL to support GetTextLEO()
    LEO_SHA1		= LTT_SHA1,		// access SHA1 checksum (standard)
    LEO_SHA1_D		= LTT_SHA1_D,		// access SHA1 checksum (_d file)
    LEO_IDENT		= LTT_IDENT,		// access identification (standard)
    LEO_IDENT_D		= LTT_IDENT_D,		// access identification (_d file)
    LEO_FILE		= LTT_FILE,		// access file name
    LEO_PATH		= LTT_PATH,		// access full path of LEO_FILE
    LEO_NAME		= LTT_NAME,		// access name
    LEO_XNAME		= LTT_XNAME,		// access xname
    LEO_XNAME2		= LTT_XNAME2,		// access xname with fallback to name


    //-- BMG input source selector. If none is set then use all

    LEO_MKW1		= 0x00000010,  // import track names from MKW set 1 (0x2454../0x24b8..)
    LEO_MKW2		= 0x00000020,  // import track names from MKW set 2 (0x2490../0x24cc..)
    LEO_CTCODE		= 0x00000040,  // import track names from CT-CODE   (0x4000..)
    LEO_LECODE		= 0x00000080,  // import track names from LE-CODE   (0x7000..)
     LEO_MKW		= LEO_MKW1 | LEO_MKW2,
     LEO_DEFAULT_BMG	= LEO_MKW | LEO_LECODE,
     LEO_M_BMG		= LEO_MKW | LEO_LECODE | LEO_CTCODE,


    //-- BMG input options

    LEO_IN_EMPTY	= 0x00000100,  // accept empty strings as valid
    LEO_IN_MINUS	= 0x00000200,  // accept minus strings as valid
     LEO_IN_ALL		= LEO_IN_EMPTY | LEO_IN_MINUS,


    //-- BMG operator

    LEO_INSERT		= 0x00000400,  // insert only new strings into the source.
    LEO_REPLACE		= 0x00000800,  // replace only strings that are already defined.
    LEO_OVERWRITE	= 0x00000c00,  // insert all valid strings
     LEO_M_HOW		= 0x00000c00,


    //-- Output filter pairs. Only valid if exact 1 bit is set

    LEO_VERSUS		= 0x00001000,  // use racing tracks only (!lt->arena)
    LEO_BATTLE		= 0x00002000,  // use battle arenas only (lt->arena)
     LEO_M_ARENA	= LEO_VERSUS | LEO_BATTLE,

    LEO_CUSTOM		= 0x00004000,  // use custom tracks only (!lt->original)
    LEO_ORIGINAL	= 0x00008000,  // use original tracks only (lt->original)
     LEO_M_ORIGINAL	= LEO_CUSTOM | LEO_ORIGINAL,

    LEO_NO_D_FILES	= 0x00010000,  // suppredd info about _d files

     LEO_M_OUTPUT	= LEO_M_ARENA | LEO_M_ORIGINAL | LEO_NO_D_FILES,


    //-- Output filter flags

    LEO_OUT_EMPTY	= 0x00020000,  // accept empty strings as valid
    LEO_OUT_MINUS	= 0x00040000,  // accept minus strings as valid
    LEO_OUT_DUMMY	= 0x00080000,  // accept dummy names ("_abc") as valid (default)
     LEO_OUT_ALL	= LEO_OUT_EMPTY | LEO_OUT_MINUS | LEO_OUT_DUMMY,


    //-- more options

    LEO_IN_LECODE	= 0x00100000,  // import lecode binary
    LEO_NO_SLOT		= 0x00200000,  // suppress 'SLOT <num>' if creating a LE-DEF file.
    LEO_HEX2SLOT	= 0x00400000,  // if reading an szs of format '%03x.*', use it to force slot
    LEO_NAME2SLOT	= 0x00800000,  // if reading an szs, use its filename to force number
    LEO_CT_SLOTS	= 0x01000000,  // LEO_HEX2SLOT,LEO_NAME2SLOT: check for ct-code slots
    LEO_BRIEF		= 0x02000000,  // suppress descriptions
    LEO_AUTO_PATH	= 0x04000000,  // enable auto paths


    //-- special signals

    LEO_CUT_ALL		= 0x08000000,  // cut all
    LEO_CUT_STD		= 0x10000000,  // cut for standard distribution.
    LEO_CUT_CTCODE	= 0x20000000,  // cut for CT-CODE distribution.
     LEO_M_CUT		= LEO_CUT_ALL | LEO_CUT_STD | LEO_CUT_CTCODE,

    LEO_HELP		= 0x40000000,  // print help and exit


    //-- misc

    LEO__ALL		= 0x1fffffff,
    LEO__DEFAULT	= LTT__DEFAULT | LEO_OVERWRITE | LEO_OUT_DUMMY | LEO_AUTO_PATH,
    LEO__SRC		= LEO_LTT_SELECTOR | LEO_IN_ALL,
    LEO__DEST		= LEO_LTT_SELECTOR | LEO_OUT_ALL | LEO_M_OUTPUT | LEO_M_HOW,
}
le_options_t;

//-----------------------------------------------------------------------------

extern le_options_t defaultLEO;
le_options_t SetupDefaultLEO(void);

le_options_t ScanLEO ( le_options_t current, ccp arg, ccp err_info );

bool IsInputNameValidLEO  ( ccp name, le_options_t opt );  // LEO_IN_EMPTY|LEO_IN_MINUS
bool IsOutputNameValidLEO ( ccp name, le_options_t opt ); // LEO_OUT_EMPTY|LEO_OUT_MINUS

// Options: LEO_M_HOW, LEO_IN_*, LEO_OUT_*
bool IsAssignmentAllowedLEO ( ccp cur_name, ccp new_name, le_options_t opt );

ccp GetTextLEO ( le_options_t opt );
ccp GetInputFilterLEO ( le_options_t opt );
ccp GetOutputFilterLEO ( le_options_t opt );

// Options: LEO_IN_*, LEO_M_BMG
bool GetBmgBySlotLEO
	( char *buf, uint bufsize, const bmg_t *bmg, int slot, le_options_t opt );

//-----------------------------------------------------------------------------
// wrapper from le_options_t to le_track_text_t

static inline ccp GetUpperNameLEO ( le_options_t opt )
	{ return GetUpperNameLTT((le_track_text_t)opt); }

static inline ccp GetLowerNameLEO ( le_options_t opt )
	{ return GetLowerNameLTT((le_track_text_t)opt); }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_strpar_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_strpar_t]]

typedef struct le_strpar_t
{
    le_options_t	opt;		// string selector and options

    bool		use_d;		// true: use _d variant if available
    bool		allow_d;	// true: allow _d pointers
    bool		use_sha1;	// true: get use_sha1 as fallback for ident
    bool		csum_only;	// true: set sha1 only, but not an ident string.

 #if LE_STRING_SET_ENABLED
    ccp			key;		// NULL or key for LTT_STRSET, alloced, lower case
 #endif
}
le_strpar_t;

//-----------------------------------------------------------------------------

extern const le_strpar_t NullLSP, DefaultLSP;

static inline void InitializeLSP ( le_strpar_t *par, le_options_t opt )
	{ DASSERT(par); memset(par,0,sizeof(*par)); par->opt = opt; }
void ResetLSP ( le_strpar_t *par );

static inline void SetOptionsLSP ( le_strpar_t *par, le_options_t opt )
	{ if (par) par->opt = opt; }

char * ScanLSP ( le_strpar_t *par, ccp arg, int arg_len, enumError *ret_err );
char * ScanOptionalLSP
	( le_strpar_t *par, ccp arg, int arg_len, le_track_text_t fallback, enumError *ret_err );

ccp GetNameLSP ( const le_strpar_t *par );
ccp GetOptionsLSP ( const le_strpar_t *par, le_options_t mask );
enumError ScanOptionsLSP ( le_strpar_t *par, ccp arg, ccp err_info );

//-----------------------------------------------------------------------------

#if LE_STRING_SET_ENABLED

  // key can be 'par->key' => don't change it
  // if key_len < 0: use strlen(key)
  void SetKeyNameLSP ( le_strpar_t *par, ccp key, int key_len );

  static inline bool IsValidSetLSP ( const le_strpar_t *par )
	{ return par && ( par->opt & LEO_LTT_SELECTOR ) == LTT_STRSET && par->key; }

#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_cup_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_cup_ref_t]] [[le_cup_t]]

typedef s16 le_cup_ref_t;

typedef struct le_cup_t
{
    le_cup_ref_t	*list;		// list with tracks => list[size][tracks]
    uint		used;		// used cups of 'list'
    uint		size;		// alloced cups of 'list'
    uint		max;		// max possible cups of 'list'
    uint		first_custom;	// index of first custom cup (2|8)
    uint		tracks;		// number of tracks of each cup

    uint		cur_cup;	// current cup
    uint		cur_index;	// current index

    int			fill_slot;	// -1 or fill slot
    int			fill_src;	// -1 or source of fill slot

    bool		dirty;		// true: call UpdateCupsLD() to recalc cup-track relations
    bool		close_used;	// true: CloseLECUP() (command NEW-CUP) used
}
le_cup_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeLECUP	( le_cup_t *lc, uint max_cups, uint tracks_per_cup );
void ResetLECUP		( le_cup_t *lc );

le_cup_ref_t * GetLECUP    ( const le_cup_t *lc, uint cup_index );
le_cup_ref_t * DefineLECUP ( le_cup_t *lc, uint cup_index );

int  GetIndexByRefLECUP ( const le_cup_t *lc, int  cup_ref );
uint GetRefByIndexLECUP ( const le_cup_t *lc, uint cup_index );

// return 0 or cup slot like 21
uint GetCupSlotLECUP	( const le_cup_t *lc, const le_cup_ref_t *ptr );

void AppendLECUP	( le_cup_t *lc, le_cup_ref_t ref );
void CloseLECUP		( le_cup_t *lc );
void PackLECUP		( le_cup_t *lc );
void EvenLECUP		( le_cup_t *lc );
void FillLECUP		( le_cup_t *lc, le_cup_ref_t fallback );
bool HaveInvalidLECUP	( le_cup_t *lc );

static inline le_track_type_t GetLttyLECUP ( const le_cup_t *lc )
	{ return lc && lc->tracks == 5 ? LTTY_ARENA : LTTY_TRACK; }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_track_id_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_track_id_t]]

typedef struct le_track_id_t
{
    // keep it small!

    bool		have_sha1;	// true: 'sha1bin' is valid
    bool		orig_sha1;	// true: 'sha1bin' is original (or normed)
    sha1_hash_t		sha1bin;	// sha1 of track, created from ident
    ccp			ident;		// NULL or identfication, fallback 'sha1bin'
}
le_track_id_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_track_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_track_t]]

typedef struct le_track_t
{
    // keep it small!

    track_slot_t	track_slot;	// index of le_distrib_t, 0..MKW_MAX_TRACK_SLOT
    cup_slot_t		cup_slot;	// 0 or 'cup * 10 + tidx'

    le_track_status_t	track_status;	// LTS_INVALID | LTS_VALID | LTS_ACTIVE | LTS_EXPORT
    le_track_type_t	track_type;	// LTTY_NONE | LTTY_ARENA | LTTY_TRACK | LTTY_RANDOM
    u8			is_original;	// 0:is custom, 1:is original
    u8			user_flags;	// free usable flags

    le_property_t	property;	// property
    le_music_t		music;		// music slot
    le_flags_t		flags;		// flags
    DistribFlags_t	distrib_flags;	// distribution flags (G_DTA_*)

    u8			lap_count;	// number of laps, valid if >0.0
    float		speed_factor;	// speed factor, valid if >0.0

    le_track_id_t	lti[2];		// index 0: normal, index 1:_d
    ccp			file;		// NULL or filename of track
    ccp			path;		// NULL or full path to track file
    ccp			name;		// NULL or name of track
    ccp			xname;		// NULL or extended name of track, fallback 'name'

 #if LE_STRING_LIST_ENABLED
    ccp			str_list[LTT__LIST_SIZE];
					// list of strings
 #endif

 #if LE_STRING_SET_ENABLED
    ParamField_t	str_set;	// set of strings, accessed by '[name]'
 #endif
}
le_track_t;

//-----------------------------------------------------------------------------

void InitializeLT   ( le_track_t *lt );
void ClearStringsLT ( le_track_t *lt );

void ClearLT ( le_track_t *lt, le_track_type_t ltty ); // auto-ltty if 0
void MoveLT  ( le_track_t *dest, le_track_t *src );
static inline void ResetLT (  le_track_t *lt ) { ClearLT(lt,0); }

void SetupStandardArenaLT  ( le_track_t *lt, uint setup_slot ); // slots 0..9 || 32..41
void SetupStandardTrackLT  ( le_track_t *lt, uint setup_slot ); // slots 0..31
void SetupByTrackInfoLT    ( le_track_t *lt, const TrackInfo_t *ti );

void SetupLecodeRandomTrackLT ( le_track_t *lt, uint setup_slot ); // slots 62..65

// -1:wrong slot or not a battle, 0:correct slots,
// 1:wrong property, 2:wrong music, 3:both wrong
int HaveUsualBattleSlots ( le_track_t *lt );

void ClearIdentLT ( le_track_t *lt );

//-----------------------------------------------------------------------------

static inline bool IsActiveLT ( const le_track_t *lt )
	{ return lt && lt->track_status >= LTS_ACTIVE; }

static inline bool IsExportLT ( const le_track_t *lt )
	{ return lt && lt->track_status >= LTS_EXPORT; }

static inline bool IsVisibleLT ( const le_track_t *lt )
	{ return lt && lt->track_status >= LTS_ACTIVE && !IsHiddenLEFL(lt->flags); }

static inline bool IsTitleLT ( const le_track_t *lt )
	{ return lt && lt->track_status >= LTS_ACTIVE && IsTitleLEFL(lt->flags); }

//-----------------------------------------------------------------------------
// SetListLT(), SetText*LT(): use 'opt & LEO_LTT_SELECTOR' to select text
// recognized options: LEO_M_HOW | LEO_IN_*

void SetIdentLT	( le_track_t *lt, const le_strpar_t *par, ccp ident ); // par: opt, use_d, csum_only
void SetFileLT	( le_track_t *lt, const le_strpar_t *par, ccp fname ); // par: opt
void SetPathLT	( le_track_t *lt, const le_strpar_t *par, ccp path  ); // par: opt
void SetNameLT	( le_track_t *lt, const le_strpar_t *par, ccp name  ); // par: opt
void SetXNameLT	( le_track_t *lt, const le_strpar_t *par, ccp xname ); // par: opt
#if LE_STRING_LIST_ENABLED
void SetListLT	( le_track_t *lt, const le_strpar_t *par, ccp text  ); // par: ltt, opt
#endif
#if LE_STRING_SET_ENABLED
void SetSetLT	( le_track_t *lt, const le_strpar_t *par, ccp text  ); // par: ltt, key, opt
void SetByKeyLT ( le_track_t *lt, ccp key,		  ccp text  );
#endif
void SetTextLT	( le_track_t *lt, const le_strpar_t *par, ccp text  ); // par: ltt, *

//-----------------------------------------------------------------------------
// set with 'opt'

void SetIdentOptLT ( le_track_t *lt, ccp ident, bool use_d, bool csum_only, le_options_t opt );
void SetTextOptLT  ( le_track_t *lt, ccp text, le_options_t opt );

//-----------------------------------------------------------------------------
// use 'opt & LEO_LTT_SELECTOR' to select text

ccp * GetPtrLT    ( const le_track_t *lt, const le_strpar_t *par );
ccp * GetPtrOptLT ( const le_track_t *lt, le_options_t opt, bool allow_d );

const u8 * GetSha1BinLT ( const le_track_t *lt, bool use_d );

ccp GetSha1LT	  ( const le_track_t *lt, bool use_d,			ccp return_on_empty );
ccp GetIdentLT	  ( const le_track_t *lt, bool use_d, bool use_sha1,	ccp return_on_empty );
ccp GetFileLT	  ( const le_track_t *lt,				ccp return_on_empty );
ccp GetPathLT	  ( const le_track_t *lt, le_options_t opt,		ccp return_on_empty );
ccp GetNameLT	  ( const le_track_t *lt,				ccp return_on_empty );
ccp GetXNameLT	  ( const le_track_t *lt,				ccp return_on_empty );
ccp GetXName2LT	  ( const le_track_t *lt,				ccp return_on_empty );

ccp GetTextLT     ( const le_track_t *lt, const le_strpar_t *par,	ccp return_on_empty );
ccp GetTextOptLT  ( const le_track_t *lt, le_options_t opt,		ccp return_on_empty );

ccp QuoteSha1LT	  ( const le_track_t *lt, bool use_d,			ccp return_on_empty );
ccp QuoteIdentLT  ( const le_track_t *lt, bool use_d, bool use_sha1,	ccp return_on_empty );
ccp QuoteFileLT	  ( const le_track_t *lt,				ccp return_on_empty );
ccp QuotePathLT	  ( const le_track_t *lt, le_options_t opt,		ccp return_on_empty );
ccp QuoteNameLT	  ( const le_track_t *lt,				ccp return_on_empty );
ccp QuoteXNameLT  ( const le_track_t *lt,				ccp return_on_empty );
ccp QuoteXName2LT ( const le_track_t *lt,				ccp return_on_empty );

ccp QuoteTextLT    ( const le_track_t *lt, const le_strpar_t *par,	ccp return_on_empty );
ccp QuoteTextOptLT ( const le_track_t *lt, le_options_t opt,		ccp return_on_empty );

ccp GetPathL2 ( le_distrib_t *ld, const le_track_t *lt, le_options_t opt, ccp return_on_empty );

//-----------------------------------------------------------------------------

#if LE_STRING_LIST_ENABLED

  ccp GetListLT   ( const le_track_t *lt, le_options_t opt, ccp return_on_empty );
  ccp QuoteListLT ( const le_track_t *lt, le_options_t opt, ccp return_on_empty );

#endif

#if LE_STRING_SET_ENABLED

  ccp GetSetLT   ( const le_track_t *lt, const le_strpar_t *par, ccp return_on_empty );
  ccp QuoteSetLT ( const le_track_t *lt, const le_strpar_t *par, ccp return_on_empty );

#endif

//-----------------------------------------------------------------------------

ccp GetSlotNameLT ( const le_track_t *lt );

ccp GetCupLT	  ( const le_track_t *lt, ccp return_on_empty );
ccp GetCupAlignLT ( const le_track_t *lt );

DistribFlags_t GetDistribFlagsLT ( const le_track_t *lt, bool use_d );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_track_arch_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_track_arch_t]]

typedef struct le_track_arch_t
{
    le_track_t		lt;		// usual track data
    le_group_t		group;		// group id
// [[mtcat]]

    // all strings are alloced

    ccp			name_order;	// relevant name for order
    ccp			version;	// version

    // order := attr_order plus_order name_order game_order

    int			force_slot;	// ≥0: force slot
    int			attr_order;	// order by attribute, default=1000
    int			plus_order;	// order by plus prefix
    int			game_order;	// order by game prefix
}
le_track_arch_t;

//-----------------------------------------------------------------------------

void ResetLTA ( le_track_arch_t *ta, bool reset_lt );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_auto_setup_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_auto_setup_t]]

typedef enum le_auto_setup_t
{
    LAS_OFF		= 0,
    LAS_ARENA		= LTTY_ARENA,	// true: setup original battle arenas automatically
    LAS_TRACK		= LTTY_TRACK,	// true: setup original versus tracks automatically
    LAS_EXTRA		= LTTY_EXTRA,	// true: setup MKW extra slots automatically
    LAS_RANDOM		= LTTY_RANDOM,	// true: setup LE-CODE random slots automatically
    LAS_DEFAULT		= LAS_ARENA | LAS_TRACK | LTTY_EXTRA | LAS_RANDOM,

    LAS_TEMPLATE	= LTTY__NEXT	// true: insert example tracks
}
__attribute__ ((packed)) le_auto_setup_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_type_settings_t +  le_settings_t	///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_type_settings_t]]

typedef struct le_type_settings_t
{
    bool	orig_tracks;	// true: tracks are original
    bool	add_unused;	// true: add unused arenas/tracks
}
__attribute__ ((packed)) le_type_settings_t;

///////////////////////////////////////////////////////////////////////////////
// [[le_settings_t]]

typedef struct le_settings_t
{
    bool		valid;		// true if scan or analysis done
    le_type_settings_t	bt;		// battle settings
    le_type_settings_t	vs;		// versus settings
}
le_settings_t;

///////////////////////////////////////////////////////////////////////////////
// [[le_group_info_t]]

typedef struct le_group_info_t
{
    uint		group;		// group number

    track_slot_t	*list;		// list of group members
    uint		used;		// number of group members
    uint		size;		// alloced elements of 'list'

    uint		n_head;		// number of head members
    uint		n_group;	// number of grp members
    uint		head1;		// first head member, valid if 'n_head>0'
}
le_group_info_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_distrib_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_distrib_t]]

typedef struct le_distrib_t
{
    //-- basic data

    bool		is_initialized;		// true: data is InitializeLD()
    le_auto_setup_t	auto_setup;		// bit files (LAS_*)
    le_strpar_t		spar;			// current string param, incl. default 'opt'
    ccp			separator;		// NULL or separator for string functions.

    le_track_t		*tlist;			// list of tracks
    uint		tlist_used;		// used elements in 'tlist'
    uint		tlist_size;		// alloced 'tlist' elements


    //-- more data

    StringField_t	remove_files;		// list of files, that ResetTD() should remove
    ParamField_t	dis_param;		// parameter list for FF_DISTRIB
    le_lpar_t		*lpar;			// NULL or LPAR settings
    le_cup_t		cup_battle;		// track list for battle cups
    le_cup_t		cup_versus;		// track list for versus cups
    mem_t		lecode[LEREG__N];	// LE-CODE data
    bool		lecode_alloced[LEREG__N];
						// true, if 'lecode' is alloced
    le_settings_t	scan;			// settings by ScanLeDefLD()
    le_settings_t	ana;			// settings by AnalyzeLD()


    //-- this is a reference list, setup by e.g. Sort*()
    //-- the list is terminated by a NULL pointer.

    le_track_t		**reflist;		// reference list, terminated by NULL
    uint		reflist_used;		// used elements in 'tlist'
    uint		reflist_size;		// alloced 'tlist' elements

    //-- track archive

    le_track_arch_t	*arch;			// list of tracks
    uint		arch_used;		// used elements in 'arch'
    uint		arch_size;		// alloced 'tlist' elements

    exmem_list_t	group;			// group manager


    //-- helper data

    ctcode_t		*ctcode;		// NULL or temporary file


    //-- scan helpers

    bool		is_pass2;		// true on pass2
    bool		second_ledef;		// true: don't clear tracks and cups
}
le_distrib_t;

//-----------------------------------------------------------------------------
// [[ld_out_param_t]]

typedef struct ld_out_param_t
{
    ccp			fname;
    FILE		*f;
    const ColorSet_t	*colset;
    le_distrib_t	*ld;
}
ld_out_param_t;

//-----------------------------------------------------------------------------

void InitializeLD ( le_distrib_t *ld );
void ResetLD ( le_distrib_t *ld );
void ResetTracksAndCupsLD ( le_distrib_t *ld );
void AutoSetupLD ( le_distrib_t *ld, le_auto_setup_t auto_setup );

void AnalyzeLD ( le_distrib_t *ld, bool force_new_analysis );

void CutLD ( le_distrib_t *ld, le_options_t opt ); // opt=LEO_M_CUT
void ClearTracksLD ( le_distrib_t *ld, int from, int to );

void SetupLparLD ( le_distrib_t *ld, bool load_lpar );

//-----------------------------------------------------------------------------
// track helpers

uint PackTracksLD		( const le_distrib_t *ld );
void CheckTracksLD		( const le_distrib_t *ld );

void DefineExtraTracksLD	( le_distrib_t *ld );
void DefineRandomTracksLD	( le_distrib_t *ld );

le_track_t * GetTrackLD		( le_distrib_t *ld, int slot );
le_track_t * DefineTrackLD	( le_distrib_t *ld, int slot, bool mark_export );
le_track_t * DefineFreeTrackLD	( le_distrib_t *ld, le_track_type_t ltty, bool mark_export );
le_track_t * DefineGroupTrackLD	( le_distrib_t *ld, le_track_type_t ltty, bool mark_export );

le_track_t * FindFillTrackLD	( le_distrib_t *ld, le_cup_t *lc, bool allow_candidate );

int FindFreeTracksLD         ( le_distrib_t *ld, le_track_type_t ltty, int n );
le_track_t * ReserveTracksLD ( le_distrib_t *ld, le_track_type_t ltty, int n, bool mark_export );

void SetupStandardArenasLD ( le_distrib_t *ld, bool if_no_arnea_defined );
void SetupStandardTracksLD ( le_distrib_t *ld, bool if_no_track_defined );

//-----------------------------------------------------------------------------
// cup helpers

void ClearUserFlagsAndCupsLD ( le_distrib_t *ld, bool purge_le_flags );
bool HaveWiimmCupLD ( const le_distrib_t *ld );

void DirtyCupLD   ( le_distrib_t *ld, le_track_type_t ltty );
void UpdateCupsLD ( le_distrib_t *ld );

bool PrintCupRefLD ( FILE *f, le_distrib_t *ld, le_cup_t *lc );

//-----------------------------------------------------------------------------
// manage archive [[track-arch]]

void ResetArchLD ( le_distrib_t *ld );
le_track_arch_t * GetNextArchLD ( le_distrib_t *ld );
enumError AddToArchLD ( le_distrib_t *ld, raw_data_t *raw );
bool CloseArchLD ( le_distrib_t *ld );  // return true on activity
bool CloseArchLogLD ( le_distrib_t *ld ); // return true on activity

static inline bool IsActiveArchLD ( le_distrib_t *ld )
	{ return ld && ld->arch; }

//-----------------------------------------------------------------------------

typedef bool (*FilterFuncLD) ( const le_track_t *lt, le_options_t opt );

int SetupReferenceList ( le_distrib_t *ld, FilterFuncLD filter, le_options_t opt );
int SortByCupLD ( le_distrib_t *ld, FilterFuncLD filter, le_options_t opt );

//-----------------------------------------------------------------------------

void ScanDumpLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

void ScanDumpSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
);

//-----------------------------------------------------------------------------

enumError ScanLeDefLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname,		// NULL or filename for error messages
    CopyMode_t		cm_fname	// copy mode of 'fname'
);

enumError ScanLeDefSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
);

//-----------------------------------------------------------------------------

void ScanRefLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

void ScanRefSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
);

//-----------------------------------------------------------------------------

void ScanStrLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

void ScanStrSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
);

//-----------------------------------------------------------------------------

void ScanSha1LD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

void ScanSha1STLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
);

bool ScanSha1LineLD
(
    le_distrib_t	*ld,		// valid structure
    ccp			src,		// NULL or start of line
    ccp			end,		// end of line
    bool		ident_only	// TRUE: SetIdent*LT() only
);

//-----------------------------------------------------------------------------

enumError ImportRawDataLD
(
    le_distrib_t	*ld,		// destination
    raw_data_t		*raw		// source
);

enumError ImportFileLD
(
    le_distrib_t	*ld,			// destination
    ccp			fname,			// file name of source
    bool		use_wildcard,		// true & file not exist: search files
    bool		support_options		// support options (leading '+' and no slash )
);

void ImportOptionsLD	( le_distrib_t *ld );

//void ImportLD		( le_distrib_t *ld, const le_distrib_t	*src );
void ImportLparLD	( le_distrib_t *ld, const le_lpar_t	*lpar );
void ImportAnaLD	( le_distrib_t *ld, const le_analyze_t	*ana );
void ImportCtcodeLD	( le_distrib_t *ld, const ctcode_t	*ctcode );
void ImportBmgLD	( le_distrib_t *ld, const bmg_t		*bmg, const le_strpar_t *par );

//-----------------------------------------------------------------------------

bool ExportAnaLD	( const le_distrib_t *ld, le_analyze_t *ana );
//bool ExportCtCodeLD	( const le_distrib_t *ld, ctcode_t     *ctcode );

//-----------------------------------------------------------------------------
// create files

enumError CreateDumpLD     ( FILE *f, le_distrib_t *ld );
enumError CreateRefLD      ( FILE *f, le_distrib_t *ld, bool add_strings );
enumError CreateStringsLD  ( FILE *f, le_distrib_t *ld );
enumError CreateNamesLD    ( FILE *f, le_distrib_t *ld, bool use_xname );
enumError CreateInfoLD     ( FILE *f, le_distrib_t *ld, bool use_xname, bool add_rating );
enumError CreateSha1LD     ( FILE *f, le_distrib_t *ld, bool use_xname, bool add_comments );
enumError CreateCtDefLD    ( FILE *f, le_distrib_t *ld );
enumError CreateLeDefLD    ( FILE *f, le_distrib_t *ld );
enumError CreateLecodeLD   ( FILE *f, le_distrib_t *ld, le_region_t region );
enumError CreateLparLD     ( FILE *f, le_distrib_t *ld, bool print_full );
enumError CreateBmgLD	   ( FILE *f, le_distrib_t *ld, bool bmg_text );

enumError CreateLeInfoLD   ( FILE *f, const le_distrib_t *ld, bool list_only );
enumError CreateDebugLD    ( FILE *f, const le_distrib_t *ld );

enumError CreateCupIconsLD ( ld_out_param_t *lop, mem_t mem_opt, bool print_info );
enumError CreateReportLD   ( ld_out_param_t *lop, mem_t mem_opt );

enumError CreateLecode4LD  ( ccp fname, le_distrib_t *ld );

//-----------------------------------------------------------------------------
// patch files

#if LE_DIS_PATCH_ENABLED
    enumError PatchLD ( le_distrib_t *ld, mem_t mem_opt, StringField_t *filelist );
#endif

//-----------------------------------------------------------------------------
// string functions

enumError SeparatorLD ( le_distrib_t *ld, ccp arg );

enumError CopyLD  ( le_distrib_t *ld, const le_strpar_t *dest,
				      const le_strpar_t *src, int n_src );
enumError SplitLD ( le_distrib_t *ld, const le_strpar_t *dest,
				      const le_strpar_t *src, ccp arg );
enumError SubstLD ( le_distrib_t *ld, const le_strpar_t *dest, ccp arg );

//-----------------------------------------------------------------------------
// special jobs

enumError TransferFilesLD ( le_distrib_t *ld, bool logit, bool testmode );

//-----------------------------------------------------------------------------
// distribution support

void UpdateDistribLD ( le_distrib_t *ld, bool overwrite );
bool AddDistribParamLD ( le_distrib_t *ld, ccp src, ccp end );

#if LE_DIS_SORTTRACKS_ENABLED
    bool SortTracksLD ( le_distrib_t *ld, const le_strpar_t *src, mem_t mem_opt );
#endif

#if LE_DIS_SORTCUPS_ENABLED
    bool SortCupsLD   ( le_distrib_t *ld, const le_strpar_t *src, mem_t mem_opt );
#endif

enumError ImportDistribLD ( le_distrib_t *ld, cvp data, uint size );
enumError ImportDistribSTLD ( le_distrib_t *ld, ScanText_t *st );

enumError CreateDistribLD ( FILE *f, le_distrib_t *ld, bool use_xname );
enumError CreateTrackArchivesLD ( le_distrib_t *ld, ccp destdir, mem_t mem_opt );

//-----------------------------------------------------------------------------
// lecode support

void FreeLecodeLD    ( le_distrib_t *ld, le_region_t reg );
mem_t AssignLecodeLD ( le_distrib_t *ld, le_region_t reg, cvp data, uint size );
mem_t GetLecodeLD    ( const le_distrib_t *ld, le_region_t reg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ledis_dump support		///////////////
///////////////////////////////////////////////////////////////////////////////

int EnableLDUMP ( ccp filename );
enumError CloseLDUMP();

void ImportAnaLDUMP	( const le_analyze_t *ana );
void ImportCtcodeLDUMP	( const ctcode_t *ctcode );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    struct DistributionInfo_t		///////////////
///////////////////////////////////////////////////////////////////////////////

#define DISTRIB_MAGIC8			"#DISTRIB"
#define DISTRIB_MAGIC8_NUM		0x2344495354524942ull

#define DISTRIBUTION_FILE_VERSION	     1
#define MIN_DISTRIBUTION_SLOT		    10	// included
#define MAX_DISTRIBUTION_TRACK		 10000	// excluded
#define MAX_DISTRIBUTION_ARENA		  1000	// excluded
#define DISTRIBUTION_ARENA_DELTA	100000

#define DEFAULT_DISTIBUTION_FNAME	"distribution.txt"

//-----------------------------------------------------------------------------

static inline bool IsValidDistribArena ( uint slot )
{
    return slot == DISTRIBUTION_ARENA_DELTA
	|| slot >= DISTRIBUTION_ARENA_DELTA+MIN_DISTRIBUTION_SLOT
		&& slot < DISTRIBUTION_ARENA_DELTA+MAX_DISTRIBUTION_ARENA;
}

static inline bool IsValidDistribTrack ( uint slot )
{
    return !slot
	|| slot >= MIN_DISTRIBUTION_SLOT && slot < MAX_DISTRIBUTION_TRACK;
}

static inline bool IsValidDistribSlot ( uint slot )
{
    return !slot
	|| slot >= MIN_DISTRIBUTION_SLOT
		&& slot < MAX_DISTRIBUTION_TRACK
	|| slot == DISTRIBUTION_ARENA_DELTA
	|| slot >= DISTRIBUTION_ARENA_DELTA+MIN_DISTRIBUTION_SLOT
		&& slot < DISTRIBUTION_ARENA_DELTA+MAX_DISTRIBUTION_ARENA;
}

//-----------------------------------------------------------------------------
// [[DistributionInfo_t]]

typedef struct DistributionInfo_t
{
    le_distrib_t	ld;
    ParamField_t	translate;
    ParamField_t	arena[MAX_DISTRIBUTION_ARENA];
    ParamField_t	track[MAX_DISTRIBUTION_TRACK];
}
DistributionInfo_t;

//-----------------------------------------------------------------------------

void InitializeDistributionInfo
	( DistributionInfo_t * dinf, bool add_default_param );
void ResetDistributionInfo ( DistributionInfo_t *dinf );
void AddParamDistributionInfo ( DistributionInfo_t * dinf, bool overwrite );

enumError ScanDistribFile
	( DistributionInfo_t *dinf, ccp fname, int ignore, bool assume_arena );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot translation		///////////////
///////////////////////////////////////////////////////////////////////////////

char * ScanSlot ( uint *res_slot, ccp source, bool need_point );

void NormalizeSlotTranslation ( char *buf, uint bufsize, ccp name,
			char ** p_first_para, char ** p_first_brack );

uint DefineSlotTranslation
	( ParamField_t *translate, bool is_arena, uint slot, ccp name );

// 'sha1' can be NULL
int FindSlotByTranslation ( const ParamField_t *translate, ccp fname, ccp sha1 );

// 'param' not NULL: Scan "@PARAM = VALUE" lines and store result.
void ScanSlotTranslation
	( ParamField_t *translate, le_distrib_t *ld, ccp fname, bool ignore );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			misc				///////////////
///////////////////////////////////////////////////////////////////////////////

void ScanOptLeDefine ( ccp arg );

void PrintDistribHead ( ccp format, ... )
	__attribute__ ((__format__(__printf__,1,2)));

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_LEDIS_H


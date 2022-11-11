
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

#ifndef SZS_LIB_CTCODE_H
#define SZS_LIB_CTCODE_H 1

#include "lib-std.h"
#include "lib-xbmg.h"
#include "lib-mkw.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define CT0_CODE_MAGIC_NUM	0xbad0c0de
#define CT0_DATA_MAGIC_NUM	0xbad0da7a
#define CT1_CODE_MAGIC_NUM	0xbad1c0de
#define CT1_DATA_MAGIC_NUM	0xbad1da7a

#define CT_CUP1_MAGIC_NUM	0x43555031
#define CT_CRS1_MAGIC_NUM	0x43525331
#define CT_MOD1_MAGIC_NUM	0x4d4f4431
#define CT_MOD2_MAGIC_NUM	0x4d4f4432
#define CT_OVR1_MAGIC_NUM	0x4f565231

#define CT_DEF_MAGIC8		"#CT-CODE"
#define CT_DEF_MAGIC8_NUM	0x2343542d434f4445ull

#define CT_CODE_TEX_OFFSET	   0x760  // CTCODE offset in TEX0 file
#define CT_CODE_TEX_IMAGE_OFF	 0x658c0  // IMAGE offset in TEX0 file
#define CT_CODE_TEX_SIZE	 0x87640  // total TEX0 size
#define CT_CODE_STRINGS_OFF	0x58a140  // total string pool offset
#define CT_CODE_BRRES_SIZE	0x58a280  // BRRES file size

#define CT_CODE_EXTEND_OFFSET	 0x142d0  // EXTEND offset in CTCODE
#define CT_CODE_MOD1_OFFSET	 0x00000  // MOD1 offset in EXTEND
#define CT_CODE_MOD2_OFFSET	 0x00f00  // MOD2 offset in EXTEND
#define CT_CODE_OVR1_OFFSET	 0x02980  // OVR1 offset in EXTEND
#define CT_CODE_EXTEND_SIZE	 0x07ce4  // total EXTEND size

#define CT_NAME_SIZE		     64  // defined by CTCODE

#define CT_CODE_DEFAULT_CUPS	     64  // default supported cups, must be <= 0x100
#define CT_CODE_MAX_CUPS	     64  // max supported cups, must be <= 0x100
#define CT_CODE_DEFAULT_TRACKS	  0x100  // default supported tracks
#define CT_CODE_MAX_TRACKS	  0x100  // max supported tracks
#define CT_CODE_MAX_CUPREF	   0x10  // track usage cross-ref, must be <= 0x100

#define LE_CODE_MAX_TRACKS	BMG_N_LE_TRACK		// max supported tracks
#define LE_CODE_MAX_CUPS	BMG_N_LE_RCUP		// max supported cups, must be <= 0x100

#define LE_FIRST_CT_SLOT	MKW_LE_SLOT_BEG		// LE: first used custom slot
#define LE_LAST_CT_SLOT		(MKW_LE_SLOT_END-1)	// LE: last used custom slot
#define LE_FIRST_RANDOM_SLOT	MKW_LE_RANDOM_BEG	// LE: first used random slot
#define LE_LAST_RANDOM_SLOT	(MKW_LE_RANDOM_END-1)	// LE: last used random slot

// 3 [[obsolete]] definitions
#define LE_FIRST_LOWER_CT_SLOT	   0x44			// LE: first used custom slot
#define LE_FIRST_UPPER_CT_SLOT	  0x120			// LE: first used upper custom slot
#define LE_LAST_UPPER_CT_SLOT	  0x13d			// LE: first used upper custom slot

#define CODE_MAX_TRACKS		LE_CODE_MAX_TRACKS	// max(CT/LE CODE_MAX_TRACKS)
#define CODE_MAX_CUPS		LE_CODE_MAX_CUPS	// max(CT/LE CODE_MAX_CUPS)
#define USED_SLOT_MAX		  0x100			// max predefined used slot

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LE-CODE support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[le_cup_track_t]] [[le_property_t]] [[le_music_t]]

#define LE_CODE_MIN_PHASE	CTM_LECODE_MIN - CTM_LECODE_BASE
#define LE_CODE_STD_PHASE	CTM_LECODE_DEF - CTM_LECODE_BASE
#define LE_CODE_MAX_PHASE	CTM_LECODE_MAX - CTM_LECODE_BASE

typedef u32 le_cup_track_t;	// for tracks and arenas

typedef u8  le_property_t;	// type of properties
typedef u8  le_music_t;		// type of music indices

static inline int TrackByAliasLE ( le_property_t prop, le_music_t music )
	{ return prop << 8 | music; }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ctcode_*_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[ctcode_sect_info_t]]

typedef struct ctcode_sect_info_t
{
  /* 00 */  be32_t	off;			// file offset of section
  /* 04 */  char	name[4];		// section name
  /* 08 */
}
__attribute__ ((packed)) ctcode_sect_info_t;

///////////////////////////////////////////////////////////////////////////////
// [[ctcode_header_t]]

typedef struct ctcode_header_t
{
  /* 00 */  char	magic[4];		// = CT1_DATA_MAGIC_NUM
  /* 04 */  be32_t	size;			// total size
  /* 08 */  be32_t	unknown_08;
  /* 0c */  be32_t	n_sect;			// number of section
  /* 10 */  be32_t	padding[4];
  /* 20 */  ctcode_sect_info_t
			sect_info[6];		// section offsets and names
  /* 50 */
}
__attribute__ ((packed)) ctcode_header_t;

///////////////////////////////////////////////////////////////////////////////
// [[ctcode_cup1_data_t]]

typedef struct ctcode_cup1_data_t
{
  /*  00 */  be16_t	name[CT_NAME_SIZE];	// name of cup (not relevant)
  /*  80 */  be32_t	track_id[5];		// index of related tracks
  /*  94 */  be32_t	unknown_84[0x1b];	// ?? padding
  /* 100 */
}
__attribute__ ((packed)) ctcode_cup1_data_t;

//-----------------------------------------------------------------------------
// [[ctcode_cup1_head_t]]

typedef struct ctcode_cup1_head_t
{
  /* 00 */  char	name[4];		// section name: "CUP1"
  /* 04 */  be32_t	size;			// size of section
  /* 08 */  be32_t	unknown_08;
  /* 0c */  be32_t	n_racing_cups;		// number of racing cups
  /* 10 */  be32_t	n_battle_cups;		// number of battle cups
  /* 14 */  be32_t	unknown_34[11];		// 5*0x800 + 6*0x000
  /* 40 */  ctcode_cup1_data_t data[];
}
__attribute__ ((packed)) ctcode_cup1_head_t;

///////////////////////////////////////////////////////////////////////////////
// [[ctcode_crs1_data_t]]

typedef struct ctcode_crs1_data_t
{
  /*  00 */  be16_t	tname[CT_NAME_SIZE];	// name of track (not relevant)
  /*  80 */  char	filename[64];		// file name of track
  /*  c0 */  be32_t	music_id;		// index of music
  /*  c4 */  be32_t	property_id;		// track index for properties
  /*  c8 */  be32_t	cup_id;			// cup index (not relevant)
  /*  cc */  be32_t	unknown_cc[0x0d];	// ?? padding
  /* 100 */
}
__attribute__ ((packed)) ctcode_crs1_data_t;

//-----------------------------------------------------------------------------
// [[ctcode_crs1_head_t]]

typedef struct ctcode_crs1_head_t
{
  /* 00 */  char	name[4];		// section name: "CRS1"
  /* 04 */  be32_t	size;			// size of section
  /* 08 */  be32_t	unknown_08;		// value 0x00
  /* 0c */  be32_t	n_tracks;		// number of tracks
  /* 10 */  be32_t	unknown_10;		// value 0x2a (first track behind arenas?)
  /* 14 */  be32_t	property[8];		// properties for first 8 cups?
						//   0x400: no sound
						//   0x800: sound
  /* 34 */  be32_t	unknown_14[3];		// always 0 (padding?)
  /* 40 */  ctcode_crs1_data_t data[];
}
__attribute__ ((packed)) ctcode_crs1_head_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    struct ctcode_cupref_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[ctcode_cupref_t]]

typedef struct ctcode_cupref_t
{
    uint  n_racing;			// used by n_racing cups
    uint  n_battle;			// used by n_battle cups

    uint  n;				// number of references
    u8	  cup[CT_CODE_MAX_CUPREF];	// cup index
    u8	  idx[CT_CODE_MAX_CUPREF];	// track index within cup
}
ctcode_cupref_t;


//
///////////////////////////////////////////////////////////////////////////////
///////////////		    struct ctcode_arena_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[ctcode_arena_t]]

enum { CT_ARENA_FLAGS_VALID = 0x1000 };

typedef struct ctcode_arena_t
{
    le_property_t	prop[MKW_N_ARENAS];	// only valid if >0
    le_music_t		music[MKW_N_ARENAS];	// only valid if >0
    s16			flags[MKW_N_ARENAS];	// only valid bit CT_ARENA_FLAGS_VALID is set
}
ctcode_arena_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct ctcode_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[ctcode_t]]

typedef struct ctcode_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform;		// file format of source
    ct_bmg_t		ctb;		// CT mode & message identifiers (MID)


    //--- cup data

    ctcode_cup1_head_t	*cup;		// NULL or data pointer to CUP1 data
    bool		cup_alloced;	// true: 'cup' is alloced
    uint		cup_size;	// size of 'cup'

    ctcode_cup1_data_t	*cup_racing;	// pointer to first racing cup
    uint		n_racing_cups;	// number of defined racing cups (CUP1 counter)
    uint		max_racing_cups;// max possible number of racing cups

    ctcode_cup1_data_t	*cup_battle;	// pointer to first battle cup
    uint		n_battle_cups;	// number of defined battle cups (CUP1 counter)
    uint		max_battle_cups;// max possible number of battle cups

    uint		n_unused_cups;	// number of unused cups


    //--- arenas setup for LE-CODE

    ctcode_arena_t	arena;		// data of section [SETUP-ARENA]


    //--- track data

    ctcode_crs1_head_t	*crs;		// NULL or data pointer to CRS1 data
    bool		crs_alloced;	// true: 'crs' is alloced
    uint		crs_size;	// size of 'crs'

    uint		n_tracks;	// number of used track slots (CRS1 counter)
    uint		max_tracks;	// total number of track slots (CRS1 data records)

    bmg_t		track_file;	// file names of any length
    bmg_t		track_string;	// track strings of any length
    bmg_t		track_xstring;	// extended track strings of any length
    bmg_t		track_ident;	// identifiers of any length (e.g. checksums)

    // property, music and le_flags
    le_property_t	property[CODE_MAX_TRACKS];	// not in use yet!!
    le_music_t		music[CODE_MAX_TRACKS];		// not in use yet!!
    le_flags8_t		le_flags[CODE_MAX_TRACKS];
    u8			hidden[CODE_MAX_TRACKS];


    //--- cup/track reference

    bool		cupref_dirty;	// true: 'cupref' must be calculated
    ctcode_cupref_t	cupref[CODE_MAX_TRACKS];
					// cross reference track to cup
    uint		used_tracks;	// number of active racing tracks (used by cup)
    uint		used_arenas;	// number of active battle arenas
    uint		assigned_arenas;// number of assigned battle arenas
    u8			arena_usage[BMG_N_CT_ARENA];
					// 1: arena assigned


    //--- LE-CODE support

    bool		add_wiimm_cup;	// true: assing 4*random to cup 8
    bool		replace_at;	// true: Replace "@..@" on track names
    bool		use_lecode;	// true: enable le-code support
    bool		use_le_flags;	// true: le_flags enabled
    u8			n_strings;	// number of used strings in track params (for syntax check)
    struct le_lpar_t	*lpar;		// NULL or LPAR parameters


    //--- misc

    ctcode_header_t	ct_head;	// copy of CTCODE header;
 #if USE_NEW_CONTAINER_CTC
    Container_t		container;	// data container
 #else
    DataContainer_t	*old_container;	// old data container support, init with NULL
 #endif
    uint		save_prepared;	// 1:PrepareSaveCTCODE() already called


    //--- text scan support

    u8		used_slot[CODE_MAX_TRACKS];	// 0:free, 1:used, 2:reserved
    u16		next_slot[CODE_MAX_TRACKS];	// predefined slots
    u8		force_slot[CODE_MAX_TRACKS];	// 0:std, 1:force slot usage
    uint	n_next_slot;			// >0: index-1 into 'next_slot' and 'force_slot'
    uint	tracks_defined;			// number of really defined tracks

    char	*next_cup_name;			// NULL or name of next cup
    uint	track_of_cup;			// number of tracks in current cup

    bool	cmd_n_used;			// true: sub command 'N' inserted 32 tracks
}
ctcode_t;

///////////////////////////////////////////////////////////////////////////////
// [[ctcode_save_t]]

typedef enum ctcode_save_t // save modes
{
    CTS_CT0CODE,	// save as FF_CT0_CODE file
    CTS_CT0DATA,	// save as FF_CT0_DATA file
    CTS_CT1CODE,	// save as FF_CT1_CODE file
    CTS_CT1DATA,	// save as FF_CT1_DATA file

    CTS_CUP1,		// save only 'CUP1' section as binary file
    CTS_CRS1,		// save only 'CRS1' section as binary file
    CTS_MOD1,		// save only 'MOD1' section as binary file
    CTS_MOD2,		// save only 'MOD2' section as binary file
    CTS_OVR1,		// save only 'OVR1' section as binary file

    CTS_TEX0,		// save as FF_TEX_CT file
    CTS_BRRES,		// save as FF_BRRES file
    CTS_SZS,		// save as compressed FF_BRRES file
    CTS_BMG,		// save as FF_BMG_TXT file
    CTS_TRACKS,		// save as track listing

    CTS_LIST,		// save as text file, mode --list
    CTS_REF,		// save as text file, mode --ref
    CTS_FULL,		// save as text file, mode --full
}
ctcode_save_t;

typedef enum ctlang_save_t // language modes
{
    CTL_ANY,
    CTL_EU,
    CTL_US,
    CTL_JP,
    CTL_KO,
}
ctlang_save_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeCTCODE ( ctcode_t * ctcode, ct_mode_t ct_mode );
void ResetCTCODE ( ctcode_t * ctcode );
void ResetDataCTCODE ( ctcode_t * ctcode, bool create_empty );
const VarMap_t * SetupVarsCTCODE();

void DumpCTCODE ( FILE *f, uint indent, ctcode_t * ctcode );

void CalcCupRefCTCODE ( ctcode_t * ctcode, bool force );
void PrepareExportCTCODE ( ctcode_t * ctcode );
void PatchNamesCTCODE ( ctcode_t * ctcode );
void PrepareSaveCTCODE ( ctcode_t * ctcode );

uint NormalizeMusicID ( uint music_id );
int  MusicID2TrackId ( uint music_id, int no_track, int not_found );
ccp  PrintMusicID ( uint mid, bool force_hex );
ccp  PrintPropertyID ( uint tid, bool force_hex );
ccp  PrintArenaSlot ( uint tid, bool force_hex );

void SetTrackBMG
(
    ctcode_t	*ctcode,	// valid CTCODE
    bmg_t	*bmg,		// destination BMG
    uint	tidx,		// index of the track
    ccp		text		// any text
);

///////////////////////////////////////////////////////////////////////////////

enumError ScanSetupArena ( ctcode_arena_t *ca, ScanInfo_t * si );

enumError ScanTextArena
(
    ctcode_arena_t	*ca,		// valid data structure
    ccp			fname,		// NULL or file name for error messages
    const void		*data,		// data to scan
    uint		data_size	// size of 'data'
);

///////////////////////////////////////////////////////////////////////////////

struct szs_iterator_t;
int IterateFilesCTCODE
(
    struct szs_iterator_t *it,		// iterator struct with all infos
    bool		multi_img	// false: mipmaps, true: multi images
);

//-----------------------------------------------------------------------------

static inline bool IsCtcodeFF ( file_format_t ff )
	{ return ff == FF_TEX_CT || ff == FF_CTDEF || ff == FF_LE_BIN; }

//-----------------------------------------------------------------------------

enumError ScanRawCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata		// NULL or container-data
);

//-----------------------------------------------------------------------------

enumError ScanTextCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata,	// NULL or container-data
    CheckMode_t		mode		// not NULL: call CheckCTCODE(mode)
);

//-----------------------------------------------------------------------------

struct raw_data_t;

enumError ScanRawDataCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckCTCODE(mode)
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckCTCODE(mode)
);

//-----------------------------------------------------------------------------

enumError SaveTextCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    const output_mode_t	* outmode	// output mode, if NULL: use defaults
);

//-----------------------------------------------------------------------------

enumError SaveMessageCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTrackListCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    uint		select,		// track selection
					//   0: only custom tracks
					//   1: standard + custom tracks
					//   2: standard + custom + battle tracks
    bool		print_slots,	// true: print music and property slots
    bool		print_head,	// true: print table header
    bool		raw_mode	// true: print raw text without escapes

);

static inline enumError SaveTrackListByOptCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    return SaveTrackListCTCODE( ctcode, fname, raw_mode,
		long_count, !brief_count,
		print_header, raw_mode );
}

//-----------------------------------------------------------------------------

enumError SaveTrackTableCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    ctcode_save_t	save_mode,	// save mode
    ctlang_save_t	lang_mode,	// language mode
    const output_mode_t	* outmode	// output mode for text output
					//   if NULL: use defaults
);

///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

extern int	opt_ct_log;	// values: see CTLOG_* above

//-----------------------------------------------------------------------------

extern bool	old_spiny;
extern u32	crs1_property;
extern u8	ctcode_used_slot[USED_SLOT_MAX]; // 0:free, 1:used, 2:reserved

int ScanOptCtLog ( ccp arg );
int ScanOptCRS1 ( ccp arg );
int ScanOptAllowSlots ( ccp arg );

extern ccp  opt_image_dir;
extern bool opt_dynamic_ctcode;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_CTCODE_H


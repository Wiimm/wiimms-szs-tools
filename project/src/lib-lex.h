
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
 *   Copyright (c) 2011-2021 by Dirk Clemens <wiimm@wiimm.de>              *
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

#ifndef SZS_LIB_LEX_H
#define SZS_LIB_LEX_H 1

#include "lib-std.h"
#include "lib-mkw.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

struct raw_data_t;
struct szs_have_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			features_szs_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[features_szs_t]]

#define FIRST_FEATURE_SZS_OFFSET 2

typedef struct features_szs_t
{
    u16	size_be;		// size of features_szs_t in big endian
				//	== version number

    //--- SZS files (initial list)

    u8 f_lex;			// HAVESZS_COURSE_LEX
    u8 f_item_slot_table;	// HAVESZS_ITEM_SLOT_TABLE
    u8 f_objflow;		// HAVESZS_OBJFLOW
    u8 f_ght_item;		// HAVESZS_GHT_ITEM
    u8 f_ght_item_obj;		// HAVESZS_GHT_ITEM_OBJ
    u8 f_ght_kart;		// HAVESZS_GHT_KART
    u8 f_ght_kart_obj;		// HAVESZS_GHT_KART_OBJ
    u8 f_minigame;		// HAVESZS_MINIGAME
    u8 f_aiparam_baa;		// HAVESZS_AIPARAM_BAA
    u8 f_aiparam_bas;		// HAVESZS_AIPARAM_BAS

    //--- KMP features (initial list)

    u8 kmp_woodbox_ht;		// HAVEKMP_WOODBOX_HT
    u8 kmp_mushroom_car;	// HAVEKMP_MUSHROOM_CAR
    u8 kmp_penguin_pos;		// HAVEKMP_PENGUIN_POS
    u8 kmp_second_ktpt;		// HAVEKMP_SECOND_KTPT
    u8 kmp_extended_pflags;	// HAVEKMP_X_PFLAGS
    u8 kmp_xpf_cond_obj;	// HAVEKMP_X_COND
    u8 kmp_xpf_def_obj;		// HAVEKMP_X_DEFOBJ
    u8 kmp_xpf_random;		// HAVEKMP_X_RANDOM
    u8 kmp_eprop_speed;		// HAVEKMP_EPROP_SPEED
    u8 kmp_coob_riidefii;	// HAVEKMP_COOB_R
    u8 kmp_coob_khacker;	// HAVEKMP_COOB_K
    u8 kmp_uncond_oob;		// HAVEKMP_UOOB

    //--- LEX sections (initial list)

    u8 lex_sect_fea0;		// HAVELEXS_FEA0
    u8 lex_sect_test;		// HAVELEXS_TEST
    u8 lex_sect_set1;		// HAVELEXS_SET1
    u8 lex_sect_cann;		// HAVELEXS_CANN
    u8 lex_sect_hipt;		// HAVELEXS_HIPT

    //--- LEX features (initial list)

    u8 lex_test_active;		// HAVELEXF_TEST_ACTIVE
    u8 lex_item_range;		// HAVELEXF_ITEM_RANGE
    u8 lex_cannon;		// HAVELEXF_CANNON
    u8 lex_hide_pos;		// HAVELEXF_HIDE_POS

    //--- new files and features will be appended here
}
__attribute__ ((packed)) features_szs_t;

//-----------------------------------------------------------------------------

void InitializeFeaturesSZS ( features_szs_t * fs );
static inline void ResetFeaturesSZS ( features_szs_t * fs ) {}

void SetupFeaturesSZS
	( features_szs_t *fs, const struct szs_have_t *have, bool is_lex );
void SetIsLexFeaturesSZS ( features_szs_t *fs );

int GetFeaturesStatusSZS ( const features_szs_t *fs );
    // returns
    //	0: all features set to null
    //	1: all features except f_lex + lex_sect_fea0 are set to null
    //	2: any features except f_lex + lex_sect_fea0 is not NULL

void PrintFeaturesSZS
(
    PrintScript_t	*ps,		// valid output definition
    const features_szs_t *fs,		// valid source
    bool		is_lex,		// TRUE: Create output for a LEX file 
    int			comments,	// >0: add extended comments
    int			print_modes,	// >0: append ",MODES"
    u8			include,	// print only if all bits match 
    u8			exclude		// exclude if any bit match
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			features_szs_mode_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[features_szs_mode_t]]

typedef enum features_szs_mode_t
{
    FZM_SECTION		= 0x01, // S: is a section
    FZM_VISUAL		= 0x02, // V: visual impact
    FZM_GAMEPLAY	= 0x04, // G: gameplay impact

    FZM_BATTLE		= 0x08, // B: impact on coin runners and balloon battle
    FZM_RACING		= 0x10, // R: impact on racings

    FZM_TIMETRIAL	= 0x20, // T: impact on time trial
    FZM_OFFLINE		= 0x40, // L: impact on offline battles/races
    FZM_ONLINE		= 0x80, // O: impact on online battles/races

    FZM_M_KIND		= FZM_VISUAL | FZM_GAMEPLAY,
    FZM_M_TYPE		= FZM_BATTLE | FZM_RACING,
    FZM_M_WHERE		= FZM_TIMETRIAL | FZM_OFFLINE | FZM_ONLINE,
    FZM__MASK		= FZM_SECTION | FZM_M_KIND | FZM_M_TYPE | FZM_M_WHERE,

}
features_szs_mode_t;

//-----------------------------------------------------------------------------

ccp GetFeaturesMode ( features_szs_mode_t mode );
ccp GetFeaturesEffects ( features_szs_mode_t mode );
ccp GetFeaturesEffectsByOffset ( uint offset );

const features_szs_t * GetFeaturesModes();

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
// [[have_lex_sect_t]]
// order is important, compare have_lex_sect_info[] and ct.wiimm.de

typedef enum have_lex_sect_t
{
    HAVELEXS_SET1,
    HAVELEXS_CANN,
    HAVELEXS_TEST,
    HAVELEXS_HIPT,
    HAVELEXS_FEA0,
    //--- add new elements here (order is important)
    HAVELEXS__N
}
have_lex_sect_t;

///////////////////////////////////////////////////////////////////////////////
// [[have_lex_feat_t]]
// order is important, compare have_lex_feat_info[] and ct.wiimm.de

typedef enum have_lex_feat_t
{
    HAVELEXF_TEST_ACTIVE,
    HAVELEXF_ITEM_RANGE,
    HAVELEXF_CANNON,
    HAVELEXF_HIDE_POS,
    //--- add new elements here (order is important)
    HAVELEXF__N
}
have_lex_feat_t;

//-----------------------------------------------------------------------------
// [[lex_stream_id]]

typedef enum lex_stream_id
{
    LEXS_TERMINATE	= 0,		// terminate stream
    LEXS_IGNORE		= 0x2d2d2d2d,	// "----" ignore and remove

    LEXS_FEA0		= 0x46454130,	// "FEA0" features
    LEXS_FEAT		= 0x46454154,	// "FEAT" features
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
    u32			magic;		// section magic
    ccp			name;		// info name
    uint		min_size;	// minimal size for 'is available'
}
have_lex_info_t;

extern const have_lex_info_t have_lex_sect_info[HAVELEXS__N];
extern const have_lex_info_t have_lex_feat_info[HAVELEXF__N];

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
    u8		random;		// 1..8: force random scenario
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
    uint		have_sect;	// bit field for found lex sections
    uint		have_feat;	// bit field for found lex features


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

lex_item_t * AppendFea0LEX ( lex_t * lex, bool overwrite, const features_szs_t *src_fs );
lex_item_t * AppendSet1LEX ( lex_t * lex, bool overwrite );
lex_item_t * AppendCannLEX ( lex_t * lex, bool overwrite );
lex_item_t * AppendHiptLEX ( lex_t * lex, bool overwrite );
lex_item_t * AppendTestLEX ( lex_t * lex, bool overwrite );

const VarMap_t * SetupVarsLEX();

void DumpElementsLEX ( FILE *f, lex_t *lex, bool hexdump );

ccp CreateSectionInfoLEX
	( have_lex_sect_t special, bool add_value, ccp return_if_empty );
ccp CreateFeatureInfoLEX
	( have_lex_feat_t special, bool add_value, ccp return_if_empty );

uint GetLexFeatures ( const lex_element_t *elem );

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
    uint	have_sect;	// bit field for found lex sections
    uint	have_feat;	// bit field for found lex features

    bool	lex_found;	// true: LEX file found

    bool	set1_found;	// true: section SET1 found
    lex_set1_t	set1;		// data of section SET1

    bool	test_found;	// true: section TEST found
    lex_set1_t	test;		// data of section TEST


    // data
    float3	item_factor;	// 1.0 or copy of set1.item_factor
}
lex_info_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeLexInfo ( lex_info_t *info );
static inline void ResetLexInfo ( lex_info_t *info )
	{ if (info) memset(info,0,sizeof(*info)); }

void SetupLexInfo ( lex_info_t *info, const lex_t *lex );
enumError SetupLexInfoByData ( lex_info_t *info, cvp data, uint data_size );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX patching			///////////////
///////////////////////////////////////////////////////////////////////////////

extern bool	opt_lex_features;
extern bool	opt_lex_rm_features;
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

static inline bool HavePatchFeaturesLEX()
	{ return opt_lex_features || opt_lex_rm_features; }

static inline bool HavePatchLEX()
	{ return opt_lex_purge || HavePatchFeaturesLEX() || HavePatchTestLEX(); }

static inline bool HaveActivePatchLEX()
	{ return opt_lex_purge || HavePatchFeaturesLEX() || HaveActivePatchTestLEX(); }

static inline bool HaveAddSectionsLEX()
	{ return HavePatchFeaturesLEX() || HaveActivePatchTestLEX(); }

// return TRUE if modified / set optional 'empty' to true, if test section is empty
bool PatchTestLEX ( lex_test_t *lt, bool *empty );
bool PatchFeaturesLEX ( lex_t * lex, const struct szs_have_t *have, bool *empty );
bool PatchLEX ( lex_t * lex, const struct szs_have_t *have );
bool PurgeLEX ( lex_t *lex );

struct szs_norm_t;
void AddSectionsLEX ( struct szs_file_t *szs, struct szs_norm_t *norm,
			const struct szs_have_t * have  );

bool LEX_ACTION_LOG ( bool is_patch, const char * format, ... )
	__attribute__ ((__format__(__printf__,2,3)));

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_LECODE_H


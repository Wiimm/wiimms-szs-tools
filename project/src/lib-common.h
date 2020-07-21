
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

#ifndef SZS_LIB_COMMON_H
#define SZS_LIB_COMMON_H 1

#include "lib-std.h"

struct BZ2Manager_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			USE_OBJECT_MANAGER		///////////////
///////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------
// USE_OBJECT_MANAGER:
//	0: only struct and initialization available
//	1: scan+compare after reading and before writing
//	2: additional use + verify
//	3: replace old code and support new features
//------------------------------------------------------

#ifdef TEST
  #define USE_OBJECT_MANAGER	2
#elif HAVE_WIIMM_EXT
  #define USE_OBJECT_MANAGER	1
#else
  #define USE_OBJECT_MANAGER	0
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			magics				///////////////
///////////////////////////////////////////////////////////////////////////////

#define ITEMSLT_TEXT_MAGIC	"#ITEMSLT"
#define ITEMSLT_TEXT_MAGIC_NUM	0x234954454d534c54ull
#define ITEMSLT_SIZE6		1457
#define ITEMSLT_SIZE12		1811

#define KMG_MAGIC		"RKMG"
#define KMG_MAGIC_NUM		0x524b4d47
#define KMG_TEXT_MAGIC		"#KMG"
#define KMG_TEXT_MAGIC_NUM	0x234b4d47

#define KRT_MAGIC		"RKGT"
#define KRT_MAGIC_NUM		0x524b4754
#define KRT_TEXT_MAGIC		"#KRT"
#define KRT_TEXT_MAGIC_NUM	0x234b5254

#define KRM_MAGIC		"RKRM"
#define KRM_MAGIC_NUM		0x524b524d
#define KRM_TEXT_MAGIC		"#KRM"
#define KRM_TEXT_MAGIC_NUM	0x234b524d

#define OBJFLOW_TEXT_MAGIC8	"#OBJFLOW"
#define OBJFLOW_TEXT_MAGIC8_NUM	0x234f424a464c4f57ull
#define OBJFLOW_SIZE		38170	// [[obsolete]]

#define GH_ITEM_TEXT_MAGIC8	"#GH-ITEM"
#define GH_ITEM_TEXT_MAGIC8_NUM	0x2347482d4954454dull

#define GH_IOBJ_TEXT_MAGIC8	"#GH-IOBJ"
#define GH_IOBJ_TEXT_MAGIC8_NUM	0x2347482d494f424aull

#define GH_KART_TEXT_MAGIC8	"#GH-KART"
#define GH_KART_TEXT_MAGIC8_NUM	0x2347482d4b415254ull

#define GH_KOBJ_TEXT_MAGIC8	"#GH-KOBJ"
#define GH_KOBJ_TEXT_MAGIC8_NUM	0x2347482d4b4f424aull

//
///////////////////////////////////////////////////////////////////////////////
///////////////			global common			///////////////
///////////////////////////////////////////////////////////////////////////////

#define OBJMGR_USUAL_SLOTS	0x13c	// usual number of object slots
#define OBJMGR_MAX_SLOTS	0x400	// max number of object slots

#define OBJMGR_USUAL_MAX_OBJID	0x2f3	// usual max allowed object id
#define OBJMGR_MAX_OBJID	0x3ff	// max allowed object id

#define OBJMGR_MAX_ROW_SIZE	 0x80
#define OBJMGR_MAX_PRIO		 0xff

//-----------------------------------------------------------------------------
// [[new_object_t]]

typedef enum new_object_t
{
    NEWOBJ_OFF,
    NEWOBJ_REPLACE,
    NEWOBJ_SHRINK,
    NEWOBJ_GROW,

    NEWOBJ__N,
    NEWOBJ__DEFAULT = NEWOBJ_OFF
}
new_object_t;

//-----------------------------------------------------------------------------
// [[object_type_t]]

typedef struct object_type_t
{
    file_format_t
		fform;		// FF_UNKNOWN or known file format
    bool	is_objflow;	// FALSE: unknown, TRUE: is ObjFlow
    bool	is_geohit;	// FALSE: unknown, TRUE: is GeoHit*
    bool	is_kart;	// GeoHit only: FALSE: is Item, TRUE: is Kart
    bool	is_obj;		// GeoHit only: TRUE: is *Obj variant

    uint	row_size;	// data size of object icluding object id
    uint	slot_offset;	// offset of slot list
    uint	ref_offset;	// offset of reference list

    new_object_t new_object;	// how to handle new objects
    uint	object_slots;	// init by OBJMGR_USUAL_SLOTS
    uint	max_object_id;	// init by OBJMGR_USUAL_MAX_OBJID
}
object_type_t;

//-----------------------------------------------------------------------------

struct BZ2Manager_t * GetCommonBZ2Manager ( file_format_t fform );
const VarMap_t * SetupVarsCOMMON();

ccp GetNewObjectName ( new_object_t no );

//-----------------------------------------------------------------------------

void InitializeObjType	( object_type_t *ot, file_format_t fform );
void SetObjTypeByFF	( object_type_t *ot, file_format_t fform );
bool ScanObjType	( object_type_t *ot, cvp data, uint data_size, uint file_size );
ccp  LogObjType		( const object_type_t *ot );
file_format_t GetObjTypeFF ( const object_type_t *ot, bool want_text );

static inline bool IsStandardObjType ( const object_type_t *ot )
{
    DASSERT(ot);
    return ot->new_object	== NEWOBJ_OFF
	&& ot->object_slots	== OBJMGR_USUAL_SLOTS
	&& ot->max_object_id	== OBJMGR_USUAL_MAX_OBJID;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			object_mgr_t			///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    OM_PRIO_UNUSED,
    OM_PRIO_DELETED,
    OM_PRIO_ORIG,
    OM_PRIO_DEFAULT,
    OM_PRIO_INSERTED,
};

//-----------------------------------------------------------------------------
// [[object_mgr_t]]

typedef struct object_mgr_t
{
    object_type_t   otype;	// object type

    //--- object table

    uint	row_data_size;	// data size of each object row := row_size-2
    u8		*list;		// data, size=OBJMGR_MAX_SLOTS * row_size
    u8		prio[OBJMGR_MAX_SLOTS];
				// >0: object in use
    u8		max_prio;	// max prio ever used
}
object_mgr_t;

//-----------------------------------------------------------------------------

void InitializeObjMgr	( object_mgr_t *om, file_format_t fform );
void ResetObjMgr	( object_mgr_t *om );
ccp  LogStateObjMgr	( const object_mgr_t *om );

//-----------------------------------------------------------------------------
#if USE_OBJECT_MANAGER
//-----------------------------------------------------------------------------

bool SetupObjMgr	( object_mgr_t *om, uint row_size );
bool SetupObjMgrByData	( object_mgr_t *om, cvp data, uint data_size, u8 prio );

u8 * InsertObjMgr	( object_mgr_t *om, uint obj_id, cvp data, u8 prio );
u8 * ClearObjMgr	( object_mgr_t *om, uint obj_id );
u8 * RemoveObjMgr	( object_mgr_t *om, uint obj_id );
u8 * GetObjMgr		( object_mgr_t *om, uint obj_id );

u8 * CreateBinObjMgr
(
    // return alloced data
    uint	*res_size,	// not NULL: store size of alloced data

    const object_mgr_t
		*om,		// valid object manager
    uint	min_prio,	// minimal prio
    uint	max_elem,	// >0: limit number of elements
    uint	max_obj_id	// >0: ignore objects with id >#
);

bool VerifyObjMgr
(
    const object_mgr_t
		*om,		// valid object manager
    cvp		data,		// data to compare
    uint	data_size,	// size of 'data'
    ccp		identify	// string to identify the caller
);

bool VerifyBinObjMgr ( cvp data, uint data_size, ccp identify );

///////////////////////////////////////////////////////////////////////////////

bool WriteObjMgrToSetupSection
(
    FILE		*f,	// output file
    const object_mgr_t	*om,	// valid object manager
    bool		always,	// false: write only if not standard
    uint		header,	// 0: off
				// 1: start with: [SECTION]\n @REVISION=
				// 2: add 'section_sep' before step 1
    bool		explain	// true: insert extented explainations
);

//-----------------------------------------------------------------------------

bool SetupObjMgrBySI
(
    object_mgr_t	* om,	// valid data
    ScanInfo_t		* si,	// valid data
    ccp			name	// name of command
);

//-----------------------------------------------------------------------------
#endif // USE_OBJECT_MANAGER
//-----------------------------------------------------------------------------
//
///////////////////////////////////////////////////////////////////////////////
///////////////			itemslot_bin_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[itemslot_bin_t]]

typedef struct itemslot_bin_t
{
    /*000*/  u8 n_table;		// number of tables (6 or 12)

    //--- probabilities for for racing

    /*001*/  u8 gp_player_rows;		// players in Grand Prix
	     u8 gp_player_cols;
	     u8 gp_player	[12][19];

    /*0e7*/  u8 gp_enemy_rows;		// enemies in Grand Prix
	     u8 gp_enemy_cols;
	     u8 gp_enemy	[12][19];

    /*1cd*/  u8 vs_player_rows;		// players in VS
	     u8 vs_player_cols;
	     u8 vs_player	[12][19];

    /*2b3*/  u8 vs_enemy_rows;		// enemies in VS
	     u8 vs_enemy_cols;
	     u8 vs_enemy	[12][19];

    /*399*/  u8 vs_online_rows;		// players in online VS
	     u8 vs_online_cols;
	     u8 vs_online	[12][19];

    /*47f*/  u8 special_rows;		// special item boxes
	     u8 special_cols;
	     u8 special		[16][19];

    //--- 0x5b1 = 1457 bytes until here
    //--- probabilities for for battles

    /*5b1*/  u8 balloon_player_rows;	// players in Balloon Battle
	     u8 balloon_player_cols;
	     u8 balloon_player	[3][19];

    /*5ec*/  u8 balloon_enemy_rows;	// enemies in Balloon Battle
	     u8 balloon_enemyv;
	     u8 balloon_enemy	[3][19];

    /*627*/  u8 coin_player_rows;	// players in Coin Runners
	     u8 coin_player_cols;
	     u8 coin_player	[3][19];

    /*662*/  u8 coin_enemy_rows;	// enemies in Coin Runners
	     u8 coin_enemy_cols;
	     u8 coin_enemy	[3][19];

    /*69d*/  u8 balloon_online_rows;	// players in online Balloon Battle
	     u8 balloon_online_cols;
	     u8 balloon_online	[3][19];

    /*6d8*/  u8 coin_online_rows;	// players in online Coin Runners
	     u8 coin_online_cols;
	     u8 coin_online	[3][19];

    //--- 0x713 = 1811 bytes total
}
__attribute__ ((packed)) itemslot_bin_t;

//-----------------------------------------------------------------------------
// [[itemslot_t]]

typedef struct itemslot_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform_source;	// file format of input file

    bool		add_battle;	// true: add battle tables (default)
    bool		use_slt;	// true: use '.slt' instead of '.bin'


    //--- parser helpers

    int			revision;	// tool revision
    bool		is_pass2;	// true: ITEMSLOT compiler runs pass2


    //--- data

    itemslot_bin_t	data;		// data
}
itemslot_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeITEMSLOT  ( itemslot_t * itemslot, file_format_t fform );
void SetupBZ2MgrITEMSLOT ( itemslot_t * itemslot, bool reload_bin );
void ResetITEMSLOT ( itemslot_t * itemslot );

bool GetOriginalITEMSLOT ( itemslot_bin_t *dest );

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanTextITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// file format
);

//-----------------------------------------------------------------------------

enumError ScanITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname		// not NULL: Analyse file extention for '.slt'
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

//-----------------------------------------------------------------------------

enumError SaveRawITEMSLOT
(
    itemslot_t		* itemslot,	// pointer to valid ITEMSLOT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextITEMSLOT
(
    const itemslot_t	* itemslot,	// pointer to valid ITEMSLOT
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >1: write only modified records
					//   >9: minimize output
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			minigame_kmg_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[minigame_kmg_t]]

typedef struct minigame_kmg_head_t
{
  //--- header

  /*000*/  u8		magic[4];	// always "RKMG"
  /*004*/  be32_t	size;		// file size
  /*008*/  be32_t	unknown_008;	// unknown
  /*00c*/  be32_t	coin_offset;	// Offset to Coin Runners Data, relative to 'data'
  /*010*/  u8		data[];		// file date
}
__attribute__ ((packed)) minigame_kmg_head_t;

//-----------------------------------------------------------------------------#
// [[minigame_kmg_t]]

typedef struct minigame_kmg_t
{
  /*000*/  minigame_kmg_head_t
			head;			// file header

  //--- balloon tables
  /*010*/  be16_t	dur_balloon[10][11];	// duration in seconds (always 0xb4=180)
  /*0ec*/  be16_t	 off_0ec[10][11];	// ?

  //--- 10 balloon parameters
  /*1c8*/  be16_t	 off_1c8;		// Number of points when hit a player
  /*1ca*/  be16_t	 off_1ca;		// Number of extra points when defeating
						//	a player (originally unused).
  /*1cc*/  be16_t	 off_1cc;		// Number of points when losing all balloons.
  /*1ce*/  be16_t	 off_1ce;		// Number of points when hit by a player
						//	(originally unused)
  /*1d0*/  be16_t	 off_1d0;		// ?
  /*1d2*/  be16_t	 off_1d2;		// ?
  /*1d4*/  be16_t	 off_1d4;		// ?
  /*1d6*/  be16_t	 off_1d6;		// ?
  /*1d8*/  be16_t	 off_1d8;		// ?
  /*1da*/  be16_t	 off_1da;		// ?

  //--- coin runners tables
  /*1dc*/  be16_t	dur_coin[10][11];	// duration in seconds (always 0xb4=180)
  /*2b8*/  be16_t	 off_2b8[10][11];	// ?
  /*394*/  be16_t	start_coins[10][11];	// number of coins at start
  /*470*/  be16_t	max_coins[10][11];	// max number of coins
  /*54c*/  be16_t	 off_5c4[10][11];	// ?
  /*628*/  be16_t	 off_628[10][11];	// ?

  //--- 10 coin runners parameters
  /*704*/  be16_t	 off_704;		// ?
  /*706*/  be16_t	 off_706;		// ?
  /*708*/  be16_t	 off_708;		// ?
  /*70a*/  be16_t	 off_70a;		// ?
  /*70c*/  be16_t	 off_70c;		// ?
  /*70e*/  be16_t	 off_70e;		// ?
  /*710*/  be16_t	 off_710;		// ?
  /*712*/  be16_t	 off_712;		// Min num of coins lost when falling down
  /*714*/  be16_t	 off_714;		// Min num of coins lost when hit by a player
  /*716*/  be16_t	 off_716;		// Percentage of coins to lose (0-100)
  /*718*/
}
__attribute__ ((packed)) minigame_kmg_t;

//-----------------------------------------------------------------------------
// [[minigame_t]]

typedef struct minigame_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform_source;	// file format of input file


    //--- parser helpers

    int			revision;	// tool revision
    bool		is_pass2;	// true: MINIGAME compiler runs pass2


    //--- data

    minigame_kmg_t	data;		// raw data, valid if magic is ok
}
minigame_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeMINIGAME  ( minigame_t * minigame, file_format_t fform );
void SetupBZ2MgrMINIGAME ( minigame_t * minigame, bool reload_bin );
void ResetMINIGAME ( minigame_t * minigame );

bool GetOriginalMINIGAME ( minigame_kmg_t *dest );

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanTextMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// file format
);

//-----------------------------------------------------------------------------

enumError ScanMINIGAME
(
    minigame_t		* minigame,		// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

//-----------------------------------------------------------------------------

enumError SaveRawMINIGAME
(
    minigame_t		* minigame,	// pointer to valid MINIGAME
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextMINIGAME
(
    const minigame_t	* minigame,	// pointer to valid MINIGAME
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >9: minimize output
);

///////////////////////////////////////////////////////////////////////////////

int ScanOptKmgLimit ( ccp arg );
int ScanOptKmgCopy ( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ObjFlow.bin			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[objflow_bin_t]]

typedef struct objflow_bin_t
{
    u16		id;
    char	name[0x20];
    char	resources[0x40];
    s16		param[9];
}
__attribute__ ((packed)) objflow_bin_t;

//-----------------------------------------------------------------------------
// [[objflow_t]]

typedef struct objflow_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform;		// FF_OBJFLOW | FF_OBJFLOW_TXT

    //--- scan helpers

    int			revision;	// tool revision
    bool		is_pass2;	// true: OBJFLOW compiler runs pass2

    //--- data

    // if USE_OBJECT_MANAGER < 2 => use obj_mgr.otype only
    object_mgr_t	obj_mgr;	// object manager

 #if USE_OBJECT_MANAGER < 3
    u8		bin[OBJFLOW_SIZE];	// raw binary data
 #endif
}
objflow_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeOBJFLOW ( objflow_t * objflow );
void ResetOBJFLOW ( objflow_t * objflow );

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanTextOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanOBJFLOW
(
    objflow_t		* objflow,		// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

//-----------------------------------------------------------------------------

enumError SaveRawOBJFLOW
(
    objflow_t		* objflow,	// pointer to valid OBJFLOW
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextOBJFLOW
(
    const objflow_t	* objflow,	// pointer to valid OBJFLOW
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >9: minimize output
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			GeoHit*.bin			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[geohit_bin_t]]

typedef struct geohit_bin_t
{
    u16		id;
    u16		param[];
}
__attribute__ ((packed)) geohit_bin_t;

//-----------------------------------------------------------------------------
// [[geohit_t]]

typedef struct geohit_t
{
    //--- base info

    ccp			fname;		// alloced filename of loaded file
    FileAttrib_t	fatt;		// file attribute
    file_format_t	fform_source;	// file format of input file

 #if USE_OBJECT_MANAGER < 3
    uint		n_rec;		// number of records
 #endif


    //--- parser helpers

    int			revision;	// tool revision
    bool		is_pass2;	// true: GEOHIT compiler runs pass2


    //--- data

    // if USE_OBJECT_MANAGER < 2 => use obj_mgr.otype only
    object_mgr_t	obj_mgr;	// object manager

 #if USE_OBJECT_MANAGER < 3
    u8			*bin;		// pointer to binary data, alloced
    uint		bin_size;	// size of 'bin'
 #endif

}
geohit_t;

///////////////////////////////////////////////////////////////////////////////

void InitializeGEOHIT  ( geohit_t * geohit, file_format_t fform );
void SetupBZ2MgrGEOHIT ( geohit_t * geohit, file_format_t fform, bool reload_bin );
void ResetGEOHIT ( geohit_t * geohit );

geohit_bin_t * GetBinGEOHIT ( cvp data, uint index );

void PrintParamGEOHIT ( FILE *f, uint fw, cvp param0, cvp cmp0, int n );

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanTextGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// file format
);

//-----------------------------------------------------------------------------

enumError ScanGEOHIT
(
    geohit_t		* geohit,		// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

//-----------------------------------------------------------------------------

enumError SaveRawGEOHIT
(
    geohit_t		* geohit,	// pointer to valid GEOHIT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextGEOHIT
(
    const geohit_t	* geohit,	// pointer to valid GEOHIT
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >9: minimize output
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_COMMON_H

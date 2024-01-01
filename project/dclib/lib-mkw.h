
/***************************************************************************
 *                                                                         *
 *                     _____     ____                                      *
 *                    |  __ \   / __ \   _     _ _____                     *
 *                    | |  \ \ / /  \_\ | |   | |  _  \                    *
 *                    | |   \ \| |      | |   | | |_| |                    *
 *                    | |   | || |      | |   | |  ___/                    *
 *                    | |   / /| |   __ | |   | |  _  \                    *
 *                    | |__/ / \ \__/ / | |___| | |_| |                    *
 *                    |_____/   \____/  |_____|_|_____/                    *
 *                                                                         *
 *                       Wiimms source code library                        *
 *                       Support for Mario Kart Wii                        *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *        Copyright (c) 2012-2024 by Dirk Clemens <wiimm@wiimm.de>         *
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

#ifndef SZS_LIB_MKWII_H
#define SZS_LIB_MKWII_H 1

#define _GNU_SOURCE 1
#include "lib-mkw-def.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CTW definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define CTW_MAX_TRACK_ID 999999

//
///////////////////////////////////////////////////////////////////////////////
///////////////			base definitions		///////////////
///////////////////////////////////////////////////////////////////////////////

extern const sizeof_info_t sizeof_info_mkw[];

///////////////////////////////////////////////////////////////////////////////

enum mkw_consts_t // some constants
{
	//--- num of players

	MKW_STD_MAX_PLAYER	= 12,
	MKW_EX_MAX_PLAYER	= 24,


	//--- tracks

	MKW_N_TRACKS		= 0x20,
	MKW_TRACK_BEG		= 0x00,
	MKW_TRACK_END		= MKW_TRACK_BEG + MKW_N_TRACKS,

	MKW_TRACK_MID_1		= 0x2454,
	MKW_TRACK_MID_2		= 0x2490,


	//--- arenas

	MKW_N_ARENAS		= 0x0a,
	MKW_ARENA_BEG		= 0x20,
	MKW_ARENA_END		= MKW_ARENA_BEG + MKW_N_ARENAS,

	MKW_ARENA_MID_1		= 0x24b8,
	MKW_ARENA_MID_2		= 0x24cc,


	//--- original track := tracks + arenas

	MKW_N_ORIGINAL		= MKW_N_TRACKS + MKW_N_ARENAS,
	MKW_ORIGINAL_BEG	= MKW_TRACK_BEG,
	MKW_ORIGINAL_END	= MKW_ARENA_END,


	//--- original extra slots

	MKW_EXTRA_BEG		= 0x36,
	MKW_EXTRA_END		= 0x3b,
	MKW_N_EXTRA		= MKW_EXTRA_END - MKW_EXTRA_BEG,


	//--- LE-CODE random tracks

	MKW_LE_RANDOM_BEG	= 0x3d,
	MKW_LE_RANDOM_TEXTURE	= MKW_LE_RANDOM_BEG,
	MKW_LE_RANDOM_BY_CUP,
	MKW_LE_RANDOM_ORIG,
	MKW_LE_RANDOM_CUSTOM,
	MKW_LE_RANDOM_NEW,
	MKW_LE_RANDOM_END,
	MKW_N_LE_RANDOM		= MKW_LE_RANDOM_END - MKW_LE_RANDOM_BEG,


	//--- LE-CODE extended slots

	MKW_LE_SLOT_BEG		=   0x44,
	MKW_LE_SLOT_END		= 0x1000,
	MKW_N_LE_SLOT		= MKW_LE_SLOT_END - MKW_LE_SLOT_BEG,


	//--- special slots (LE-CODE random included)

	MKW_SPECIAL_BEG		= MKW_ORIGINAL_END,
	MKW_SPECIAL_END		= MKW_LE_SLOT_BEG,


	//--- music ids

	MKW_MUSIC_MIN_ID	= 0x75,
	MKW_MUSIC_MAX_ID	= 0xc8,
	MKW_N_MUSIC		= 42,
};

//-----------------------------------------------------------------------------

ccp GetMkwExtraInfo ( uint slot, ccp return_on_err );

ccp GetLecodeRandomName ( uint slot, ccp return_on_err );
ccp GetLecodeRandomInfo ( uint slot, ccp return_on_err );

//-----------------------------------------------------------------------------

static inline bool IsMkwTrack ( uint tid )
	{ return tid < MKW_TRACK_END; }

static inline bool IsLecodeTrack ( uint tid )
	{ return tid < MKW_TRACK_END || tid >= MKW_LE_SLOT_BEG && tid < MKW_LE_SLOT_END; }

static inline bool IsMkwArena ( uint tid )
	{ return tid >= MKW_ARENA_BEG && tid < MKW_ARENA_END; }

static inline bool IsMkwArenaOrRandom ( uint tid )
	{ return   tid >= MKW_ARENA_BEG && tid < MKW_ARENA_END
		|| tid >= MKW_LE_RANDOM_BEG && tid < MKW_LE_RANDOM_END; }

static inline bool IsMkwOriginal ( uint tid )
	{ return tid < MKW_ARENA_END; }

static inline bool IsMkwCustom ( uint tid )
	{ return tid >= MKW_ARENA_END; }

static inline bool IsMkwExtra ( uint tid )
	{ return tid >= MKW_EXTRA_BEG && tid < MKW_EXTRA_END; }

static inline bool IsLecodeRandom ( uint tid )
	{ return tid >= MKW_LE_RANDOM_BEG && tid < MKW_LE_RANDOM_END; }

static inline bool IsValidLecodeSlot ( uint tid )
	{ return tid < MKW_ARENA_END
		|| tid >= MKW_EXTRA_BEG && tid < MKW_EXTRA_END
		|| tid >= MKW_LE_SLOT_BEG && tid < MKW_LE_SLOT_END
		|| tid >= MKW_LE_RANDOM_BEG && tid < MKW_LE_RANDOM_END; }

static inline bool IsUsableLecodeSlot ( uint tid )
	{ return tid < MKW_ARENA_END
		|| tid >= MKW_LE_SLOT_BEG && tid < MKW_LE_SLOT_END; }

static inline bool IsUsableCtcodeSlot ( uint tid )
	{ return tid < 0x42 || tid >= 0x44 && tid < 0xff; }

// if !is_ctcode, then use IsUsableLecodeSlot(), otherwise use IsUsableCtcodeSlot()
bool IsUsableXcodeSlot ( uint tid, int is_ctcode );

static inline bool IsMkwSpecial ( uint tid )
	{ return tid >= MKW_SPECIAL_BEG && tid < MKW_SPECIAL_END; }

static inline bool IsLecodeSpecial ( uint tid )
	{ return tid >= MKW_SPECIAL_BEG && tid < MKW_LE_RANDOM_BEG
	      || tid >= MKW_LE_RANDOM_END && tid < MKW_SPECIAL_END; }


static inline uint NextLecodeSlot ( uint slot )
	{ return slot == MKW_ARENA_END-1 ? MKW_LE_SLOT_BEG : slot+1; }

static inline uint NextLecodeSlotRnd ( uint slot )
	{ return slot == MKW_ARENA_END-1 ? MKW_LE_RANDOM_BEG
		: slot == MKW_LE_RANDOM_END-1 ? MKW_LE_SLOT_BEG : slot+1; }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			24p support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mkw_std_pbits1_t]] [[mkw_ex_pbits1_t]] [[mkw_pbits1_t]]
// [[mkw_std_pbits2_t]] [[mkw_ex_pbits2_t]] [[mkw_pbits2_t]]

#if !defined(SUPPORT_24P)
  #define SUPPORT_24P	1
#endif

// unsigned types, that can hold 1 or 2 bits for each player
typedef u16 mkw_std_pbits1_t;		// 12 bits needed
typedef u32 mkw_std_pbits2_t;		// 24 bits needed
typedef u32 mkw_ex_pbits1_t;		// 24 bits needed
typedef u64 mkw_ex_pbits2_t;		// 48 bits needed

#if SUPPORT_24P
  typedef u32 mkw_pbits1_t;
  typedef u64 mkw_pbits2_t;
  #define MKW_MAX_PLAYER MKW_EX_MAX_PLAYER
#else
  typedef u16 mkw_pbits1_t;
  typedef u32 mkw_pbits2_t;
  #define MKW_MAX_PLAYER MKW_STD_MAX_PLAYER
#endif

#define MKW_STD_MAX_PLAYER2 ( MKW_STD_MAX_PLAYER * MKW_STD_MAX_PLAYER )
#define MKW_EX_MAX_PLAYER2  ( MKW_EX_MAX_PLAYER  * MKW_EX_MAX_PLAYER )
#define MKW_MAX_PLAYER2	    ( MKW_MAX_PLAYER     * MKW_MAX_PLAYER )

#define MAX_ROOM_MEMBERS MKW_MAX_PLAYER

//
///////////////////////////////////////////////////////////////////////////////
///////////////		assigned points for versus and rooms	///////////////
///////////////////////////////////////////////////////////////////////////////

#define MKW_POINTS_DEF_SIZE	320

#if defined(DEBUG) || defined(TEST)
    #define MKW_POINTS_TEST_ENABLED 1
#else
    #define MKW_POINTS_TEST_ENABLED 0
#endif

//-----------------------------------------------------------------------------

typedef enum MkwPointID
{
    //--- tables
    MPI_UNKNOWN,	// no table assigned
    MPI_NINTENDO,	// original table

    MPI_LINEAR,		// 4 linear tables
     MPI_LINEAR_1,
     MPI_LINEAR_B,
     MPI_LINEAR_B1,

    MPI_WIN,		// 4 generic tables tables with winner = X points
     MPI_WIN_1,
     MPI_WIN_B,
     MPI_WIN_B1,

 #if MKW_POINTS_TEST_ENABLED
    MPI_TEST,		// 4 test tables
     MPI_TEST_1,
     MPI_TEST_B,
     MPI_TEST_B1,
 #endif
    MPI__N,

    //--- index values
    MPI_I_STD = 0,
    MPI_I_1,
    MPI_I_B,
    MPI_I_B1,
    MPI_I__N,
}
MkwPointID;

typedef struct MkwPointInfo_t
{
    MkwPointID		id;		// ID of table
    uint		mode;		// 0:permanent, 1:good, 2:temp, 3:UNKNOWN
    const u8		*table;		// related table
    ccp			info;		// info about the table
    u8			alt[MPI_I__N];	// index of related alternative tables
}
MkwPointInfo_t;

//-----------------------------------------------------------------------------

extern const MkwPointInfo_t MkwPointInfo[MPI__N+1];

// [[24P--]] all tables

extern u8 MkwPointsTab[MKW_STD_MAX_PLAYER2];
extern const u8 MkwPointsTab_NINTENDO[MKW_STD_MAX_PLAYER2];
extern const u8 MkwPointsTab_LINEAR[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_LINEAR_1[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_LINEAR_B[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_LINEAR_B1[MKW_STD_MAX_PLAYER2];
extern const u8 MkwPointsTab_WIN25[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_WIN25_1[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_WIN25_B[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_WIN25_B1[MKW_STD_MAX_PLAYER2];
extern const u8 MkwPointsTab_WIN15[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_WIN15_1[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_WIN15_B[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_WIN15_B1[MKW_STD_MAX_PLAYER2];

#if MKW_POINTS_TEST_ENABLED
 extern const u8 MkwPointsTab_TEST[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_TEST_1[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_TEST_B[MKW_STD_MAX_PLAYER2];
 extern const u8 MkwPointsTab_TEST_B1[MKW_STD_MAX_PLAYER2];
#endif

extern bool opt_points_used;

//-----------------------------------------------------------------------------

static inline u8 GetMkwPoints ( uint n_player, uint idx_player )
{
// [[24P-]]
    n_player--;
    idx_player--;
    return n_player < 12 && idx_player < 12
	? MkwPointsTab[ 12*n_player + idx_player ]
	: 0;
}

static inline u8 GetDirectMkwPoints ( uint n_player, uint idx_player )
{
// [[24P-]]
    return MkwPointsTab[ 12*n_player + idx_player - 13 ];
}

//-----------------------------------------------------------------------------

int ScanOptMkwPoints ( ccp arg, bool silent );
const MkwPointInfo_t * GetMkwPointInfo ( const u8 * data );

void PrintMkwPointsRAW ( FILE *f, uint indent, const u8 * data );
void PrintMkwPointsC ( FILE *f, uint indent, const u8 * data );

// [[24P--]]
uint PrintMkwPointsLIST
(
    // returns the used 'mode'

    char	*buf,		// valid output buffer
    uint	buf_size,	// size of 'buf', MKW_POINTS_DEF_SIZE recommended
    const u8	*data,		// 12*12 data table
    uint	mode		// 0: pure number list
				// 1: allow 'a..b'
				// 2: allow names of permanent tables
				// 3: allow names
);

// [[24P--]]
void PrintMkwPoints
(
    FILE	*f,		// destination file
    uint	indent,		// indention of the output
    const u8	*data,		// data, 12*12 table
    uint	print_table,	// print the table:
				//    0: don't print
				//    1: print table as RAW
				//    2: print table as C listing
				//  A-Z: Print as cheat code with this region
    u32		cheat_base,	// not NULL: value of first param of cheat code
    uint	print_param	// print a parameter string:
				//    0: don't print
				//    1: print single line
				//    2: print all modes
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			GetVrDiff*()			///////////////
///////////////////////////////////////////////////////////////////////////////

int GetVrDiffByTab ( int diff ); // diff = VR(winner) - VR(loser)

//
///////////////////////////////////////////////////////////////////////////////
///////////////			info tables			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp  GetMkwTrackName3	( int tid );
ccp  GetMkwMusicName3	( int mid );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			engines				///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum mkw_engine_t
{
    MKW_ENGINE_50CC,
    MKW_ENGINE_100CC,
    MKW_ENGINE_150CC,
    MKW_ENGINE_MIRROR,
    MKW_ENGINE_200CC,
    MKW_ENGINE__N
}
__attribute__ ((packed)) mkw_engine_t;

extern const char mkw_engine_info[MKW_ENGINE__N+1][7];

static inline ccp GetMkwEngineInfo ( mkw_engine_t engine )
{
    return (uint)engine < MKW_ENGINE__N ? mkw_engine_info[engine] : "?";
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    slots, drivers and vehicles		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mkw_driver_t]]

typedef struct mkw_driver_t
{
    u8   weight;	// 0:small, 1:medium, 2:large
    u8   is_mii;	// 0:no Mii, 1:A, 2:B, 3:B
    char id_en[5];
    char id_de[5];
    ccp  name_en;
    ccp  name_de;
}
mkw_driver_t;

#define MKW_N_DRIVER 42
extern const mkw_driver_t mkw_driver_attrib[MKW_N_DRIVER+1];

//-----------------------------------------------------------------------------
// [[mkw_vehicle_t]]

typedef struct mkw_vehicle_t
{
    u8   weight;	// 0:small, 1:medium, 2:large
    u8   is_bike;	// 0:kart, 1:out-bike, 2:in-bike
    u8   battle;	// 0:not used in battle, 1:used in battle
    char id_en[5];
    char id_de[5];
    ccp  name_en;
    ccp  name_de;
}
mkw_vehicle_t;

#define MKW_N_VEHICLE 36
extern const mkw_vehicle_t mkw_vehicle_attrib[MKW_N_VEHICLE+1];

//-----------------------------------------------------------------------------

static inline bool IsMkwSlotValid ( uint slot )
	{ return slot < MAX_ROOM_MEMBERS; }

static inline int GetDriverInt ( u8 driver )
	{ return driver >= 254 ? (int)driver - 256 : driver; }

static inline int GetVehicleInt ( u8 vehicle )
	{ return vehicle >= 254 ? (int)vehicle - 256 : vehicle; }

//-----------------------------------------------------------------------------

// for optimization: `colset´ is always valid and never NULL!

ccp GetSlotHex	  ( u8 slot );
ccp GetSlotNum	  ( u8 slot );
ccp GetDriverNum  ( u8 driver  );
ccp GetVehicleNum ( u8 vehicle );

ccp GetDriverName4  ( u8 driver  );
ccp GetVehicleName4 ( u8 vehicle );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LE-CODE track flags		///////////////
///////////////////////////////////////////////////////////////////////////////
// bits for [[le_flags8_t]] [[le_flags_t]]

typedef u8  le_flags8_t;	// type of LE-CODE track flags (classic 8 bits)
typedef u16 le_flags_t;		// type of LE-CODE track flags since build 35

// see 'LecodeFlags_t' in dclib/lib-mkw-def.h for G_LEFL_*

static inline bool IsTitleLEFL ( le_flags_t flags )
	{ return (flags&(G_LEFL_RND_HEAD|G_LEFL_RND_GROUP)) == G_LEFL_RND_HEAD; }

static inline bool IsHiddenLEFL ( le_flags_t flags )
	{ return (flags&G_LEFL_HIDDEN)
		||  (flags&(G_LEFL_RND_HEAD|G_LEFL_RND_GROUP)) == G_LEFL_RND_GROUP; }

static inline bool IsRandomLEFL ( le_flags_t flags )
	{ return (flags&(G_LEFL_RND_HEAD|G_LEFL_RND_GROUP)) != 0; }

//-----------------------------------------------------------------------------
// lecode flags (G_LEFL_*)

le_flags_t ScanLEFL ( ccp text );

ccp PrintLEFL8     ( le_flags_t flags, bool aligned );
ccp PrintLEFL16    ( le_flags_t flags, bool aligned );

// use PrintLEFL8() or PrintLEFL16() if col==0
ccp PrintLEFL8col  ( le_flags_t flags, bool aligned, ColorSet_t *col );
ccp PrintLEFL16col ( le_flags_t flags, bool aligned, ColorSet_t *col );

static inline ccp PrintLEFL ( le_flags_t flags_bits, le_flags_t flags, bool aligned )
	{ return ( flags_bits == 16 ? PrintLEFL16 : PrintLEFL8 )(flags,aligned); }

static inline ccp PrintLEFLcol ( le_flags_t flags_bits, le_flags_t flags, bool aligned, ColorSet_t *col )
	{ return ( flags_bits == 16 ? PrintLEFL16col : PrintLEFL8col )(flags,aligned,col); }

typedef ccp (*PrintLEFL_func) ( le_flags_t flags, bool aligned );
static inline PrintLEFL_func GetPrintLEFL ( uint flags_bits )
	{ return flags_bits == 16 ? PrintLEFL16 : PrintLEFL8; }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			distribution flags		///////////////
///////////////////////////////////////////////////////////////////////////////

DistribFlags_t ScanDTA ( ccp text );
ccp PrintDTA ( DistribFlags_t flags, bool aligned );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			track class			///////////////
///////////////////////////////////////////////////////////////////////////////

TrackClass_t	ScanTrackClass		( ccp text, int text_len );
ccp		PrintTrackClass		( TrackClass_t flags, bool aligned );
TrackClassIndex_t GetTrackClassIndex	( TrackClass_t flags );
ccp		GetTrackClassName	( TrackClassIndex_t idx, ccp return_on_none );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_CTCODE_H


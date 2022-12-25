/************************************************************************
 *                                                                      *
 *                  _____     ____                                      *
 *                 |  __ \   / __ \   _     _ _____                     *
 *                 | |  \ \ / /  \_\ | |   | |  _  \                    *
 *                 | |   \ \| |      | |   | | |_| |                    *
 *                 | |   | || |      | |   | |  ___/                    *
 *                 | |   / /| |   __ | |   | |  _  \                    *
 *                 | |__/ / \ \__/ / | |___| | |_| |                    *
 *                 |_____/   \____/  |_____|_|_____/                    *
 *                                                                      *
 *                     A library by Dirk Clemens                        *
 *                                                                      *
 *   Copyright (c) 2014-2022 by Dirk Clemens <develop@cle-mens.de>      *
 *                                                                      *
 ************************************************************************
 *                                                                      *
 *   This file is generated automatically by a script and is            *
 *   therefore always overwritten. So don't EDIT this file,             *
 *   because all modifications will be lost with the next update!       *
 *                                                                      *
 ************************************************************************/

#ifndef DCLIB_MKW_DEF_H
#define DCLIB_MKW_DEF_H 1

#include "dclib-basics.h"

//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////   MKW Track Category   ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[MkwTrackCategory]]
typedef enum MkwTrackCategory
{
	G_MTCAT_NINTENDO            = 0,
	G_MTCAT_TEXTURE             = 1,
	G_MTCAT_TEMP_ALLOW          = 2,
	G_MTCAT_TEMP_DENY           = 3,
	G_MTCAT_DENY                = 4,
	G_MTCAT_DENIED              = 5,	// [[obsolete]]
	G_MTCAT_CHEAT               = 6,
	G_MTCAT_EDIT                = 7,
	G_MTCAT_MODEL               = 8,
	G_MTCAT_CHEAT_EDIT          = 9,
	G_MTCAT_UNKNOWN             = 10,
	G_MTCAT_CUSTOM              = 11,
	G_MTCAT_HNS                 = 12,
	G_MTCAT_HNS_CT              = 13,
	G_MTCAT_MISSION             = 14,
	G_MTCAT_MISSION_CT          = 15,
	G_MTCAT__N                  = 16,
}
MkwTrackCategory;

// [[MkwTrackCatMode]]
typedef enum MkwTrackCatMode
{
	G_MTCAT_MD_UNKNOWN             = 0x01,
	G_MTCAT_MD_DEFAULT             = 0x02,
	G_MTCAT_MD_CUSTOM              = 0x04,
	G_MTCAT_MD_NINTENDO            = 0x10,
	G_MTCAT_MD_HACK                = 0x20,
	G_MTCAT_MD_CHEAT               = 0x40,
	G_MTCAT_MD__MASK               = 0x77,
}
MkwTrackCatMode;

// [[MkwTrackCategory_t]]
typedef struct MkwTrackCategory_t
{
    ccp  info;
    ccp  attribs;
    int  fg_color;
    int  bg_color;
    int  color_offset;
    u8   id;
    char ch1[3+1];
    char ch2[3+1];
    u8   mode;
    u8   ban;
    char abbrev[7+1];
    char title[12+1];
    char color[15+1];
}
MkwTrackCategory_t;

MkwTrackCategory_t * GetMkwTrackCategory ( int id );


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////   LE-CODE track flags   //////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[LecodeFlags_t]]
typedef enum LecodeFlags_t
{
	G_LEFL_NEW                 = 0x0001,	// N2: new track
	G_LEFL_RND_HEAD            = 0x0002,	// HX: first track of random group
	G_LEFL_RND_GROUP           = 0x0004,	// GX: member of random group, but not first
	G_LEFL_ALIAS               = 0x0008,	// A-: use TrackByAliasLE(prop,music)
	G_LEFL_TEXTURE             = 0x0010,	// T2: track is declared as texture hack of same property slot
	G_LEFL_HIDDEN              = 0x0020,	// i-: exclude a track from cups.
	G_LEFL__RND                = 0x0006,	// both random flags
	G_LEFL__ALL                = 0x003f,	// all bits of above
	G_LEFL_BATTLE              = 0x0100,	// B-: track is a battle arena
	G_LEFL_VERSUS              = 0x0200,	// V-: track is a versus track
	G_LEFL_CUP                 = 0x0400,	// --: track is referenced by a cup
	G_LEFL_ORIG_CUP            = 0x0800,	// ob: track is referenced by a original cup
	G_LEFL_CUSTOM_CUP          = 0x1000,	// cb: track is referenced by a custom cup
	G_LEFL_RANDOM              = 0x2000,	// r-: slot id LE-CODE random
	G_LEFL__USED               = 0x3c3f,	// if any of these bits is set, the track is used
	G_LEFL__XALL               = 0x3f3f,	// all bits of above
}
LecodeFlags_t;

#define DB_LECODE_FLAGS_TYPE      "smallint unsigned"

//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////   Distrib Track Attributes   ////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DistribFlags_t]]
typedef enum DistribFlags_t
{
	G_DTA_BOOST               = 0x0001,	// is a boost track
	G_DTA_NEW                 = 0x0002,	// new track
	G_DTA_AGAIN               = 0x0004,	// track is back again
	G_DTA_UPDATE              = 0x0008,	// updated track
	G_DTA_FILL                = 0x0010,	// updated track
	G_DTA_IS_D                = 0x0100,	// is a _d file
	G_DTA_TITLE               = 0x0200,	// track marked as title-only
	G_DTA_HIDDEN              = 0x0400,	// track marked as hidden
	G_DTA_ORIGINAL            = 0x0800,	// track marked as original
	G_DTA__STATUS             = 0x001e,	// relevant for status column
	G_DTA__ALL                = 0x0f1f,
	G_DTA_F_CALCULATED        = 0x1000,	// flags already calculated
}
DistribFlags_t;

#define DB_DTA_TYPE               "smallint unsigned"

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////   misc   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define MKW_MAX_TRACK_SLOT        4095

//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   sizeof   /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern const sizeof_info_t sizeof_info_mkw_def[];

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////   E N D   /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#endif // DCLIB_MKW_DEF_H


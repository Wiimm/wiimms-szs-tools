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


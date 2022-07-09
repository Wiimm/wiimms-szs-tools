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
	G_MTCAT_DENIED              = 2,
	G_MTCAT_ORIG_KMP_KCL        = 2,
	G_MTCAT_EDIT                = 3,
	G_MTCAT_MODEL               = 4,
	G_MTCAT_UNKNOWN             = 5,
	G_MTCAT_CHEAT               = 6,
	G_MTCAT_CUSTOM              = 7,
	G_MTCAT__N                  = 8,
}
MkwTrackCategory;


typedef struct MkwTrackCategory_t
{

    ccp  info;
    int  color_offset;
    u8   id;
    char color[9+1];
    char ch[3+1];
    char title[8+1];
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


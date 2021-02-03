
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

#include "lib-staticr.h"
#include "crypt.h"
#include "db-dol.h"
#include "db-mkw.h"
#include "lib-mkw.h"
#include "db-ctcode.h"

#include "libbz2/bzlib.h"
#include "lib-bzip2.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    data: tracks + arenas		///////////////
///////////////////////////////////////////////////////////////////////////////

static const u32 pal_track_offset[] =
{
	0x37fcd0,
    //	0x380ee0,   // original has an alternative order here
	0x384f58,
	0x397da8,
	0x398130,
	0x39d040,
	0
};

static const u32 pal_arena_offset[] =
{
	0x384fd8,
	0x3974f8,
	0x397780,
	0x39c8e0,
	0
};

//-----------------------------------------------------------------------------

static const u32 usa_track_offset[] =
{
	0x37fbe0,
    //	0x380b70,   // original has an alternative order here
	0x3839e0,
	0x397450,
	0x3977d8,
	0x39b300,
	0
};

static const u32 usa_arena_offset[] =
{
	0x383a60,
	0x39d130,
	0x39d3b8,
	0x39e988,
	0
};

//-----------------------------------------------------------------------------

static const u32 jap_track_offset[] =
{
	0x37f9a0,
    //	0x380bb0,   // original has an alternative order here
	0x384c28,
	0x397588,
	0x397910,
	0x39c820,
	0
};

static const u32 jap_arena_offset[] =
{
	0x384ca8,
	0x396cd8,
	0x396f60,
	0x39c0c0,
	0
};

//-----------------------------------------------------------------------------

static const u32 kor_track_offset[] =
{
	0x3800b0,
   // 	0x3812c8,   // original has an alternative order here
	0x385348,
	0x3981e8,
	0x398570,
	0x39d480,
	0
};

static const u32 kor_arena_offset[] =
{
	0x3853c8,
	0x397938,
	0x397bc0,
	0x39cd20,
	0
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			data: region			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum rpat_mode_t // bit field
{
    RPAT_FF	= 1,	// mask value by 0xff
    RPAT_TEST	= 2,	// test mode, only patched on 'opt_x'
}
rpat_mode_t;

typedef struct region_patch_t
{
    rpat_mode_t	patch_mode;		// bit field, see above
    u32		offset[STR_ZBI_N];	// offsets for all 4 DVD types
    u32		orig;			// original value
    u32		patch;			// patch base value
    ccp		info;			// info about impact
}
region_patch_t;

static const region_patch_t vs_region_patch[] =
{
 {0,{0x143364,0x14325c,0x143050,0x14365c}, 0x80a30084,0x38a00000, "vs: locstring status" },
 {1,{0x149d54,0x149c4c,0x149a40,0x14a04c}, 0x80840084,0x38800000, "vs: check locstring, masked by 0xff" },
 {0,{0x148f2c,0x148e24,0x148c18,0x149224}, 0x80e50084,0x38e00000, "'vs_#' at regional matches." },
 {0,{0x149444,0x14933c,0x149130,0x14973c}, 0x80e50084,0x38e00000, "'vs_#' after friend connect" },
 {0,{0,0,0,0},0,0,0}
};

static const region_patch_t bt_region_patch[] =
{
 {0,{0x1433d0,0x1432c8,0x1430bc,0x1436c8}, 0x80c30084,0x38c00000, "bt: locstring status" },
 {1,{0x149da0,0x149c98,0x149a8c,0x14a098}, 0x80840084,0x38800000, "bt: check locstring, masked by 0xff" },
 {0,{0x148f80,0x148e78,0x148c6c,0x149278}, 0x80e50084,0x38e00000, "'bt_#' at regional matches." },
 {0,{0x149498,0x149390,0x149184,0x149790}, 0x80e50084,0x38e00000, "'bt_#' after friend connect" },
 {0,{0,0,0,0},0,0,0}
};

enum
{
  N_TAB_VS_REGION_PATCH	= sizeof(vs_region_patch) / sizeof(*vs_region_patch) - 1,
  N_VS_REGION_PATCH	= N_TAB_VS_REGION_PATCH < MAX_REGION_PATCH
			? N_TAB_VS_REGION_PATCH : MAX_REGION_PATCH,

  N_TAB_BT_REGION_PATCH	= sizeof(bt_region_patch) / sizeof(*bt_region_patch) - 1,
  N_BT_REGION_PATCH	= N_TAB_BT_REGION_PATCH < MAX_REGION_PATCH
			? N_TAB_BT_REGION_PATCH : MAX_REGION_PATCH,
};

//-----------------------------------------------------------------------------

typedef struct region_info_t
{
    int			* region;	// vector with regions, -1=orig
    const
	region_patch_t	* patch_tab;	// patch table
    uint		max_patch;	// N_(VS|BT)_REGION_PATCH
    uint		flag_known;	// STR_S_(VS|BT)_REGION_KNOWN
    uint		flag_differ;	// STR_S_(VS|BT)_REGION_KNOWN_DIFF
    uint		flag_unknown;	// STR_S_(VS|BT)_REGION_UNKNOWN
    ccp			name;		// "versus" or "battle"

    int			*opt_region;	// pointer to option
    bool		*opt_x;		// pointer to x-mode
    bool		*opt_t;		// pointer to t-mode
}
region_info_t;

void SetupRegionInfo ( region_info_t *ri, staticr_t * str, bool is_battle )
{
    DASSERT(ri);

    if (is_battle)
    {
	ri->region	= str ? str->bt_region : 0;
	ri->patch_tab	= bt_region_patch;
	ri->max_patch	= N_BT_REGION_PATCH;
	ri->flag_known	= STR_S_BT_REGION_KNOWN;
	ri->flag_differ	= STR_S_BT_REGION_KNOWN_DIFF;
	ri->flag_unknown= STR_S_BT_REGION_UNKNOWN;
	ri->name	= "battle";

	ri->opt_region	= &opt_str_bt_region;
	ri->opt_x	= &opt_str_xbt_region;
	ri->opt_t	= &opt_str_tbt_region;
    }
    else
    {
	ri->region	= str ? str->vs_region : 0;
	ri->patch_tab	= vs_region_patch;
	ri->max_patch	= N_VS_REGION_PATCH;
	ri->flag_known	= STR_S_VS_REGION_KNOWN;
	ri->flag_differ	= STR_S_VS_REGION_KNOWN_DIFF;
	ri->flag_unknown= STR_S_VS_REGION_UNKNOWN;
	ri->name	= "versus";

	ri->opt_region	= &opt_str_vs_region;
	ri->opt_x	= &opt_str_xvs_region;
	ri->opt_t	= &opt_str_tvs_region;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static char all_ranks_orig[]	= "%s >= %d and %s <= %d";
static char all_ranks_patch1[]	= "(%s > %d or %s <= %d)";
static char all_ranks_patch2[]	= "(%s >= %d or %s < %d)";
static int all_ranks_size = sizeof(all_ranks_orig);

static const u32 pal_all_ranks_offset[] =
{
    0x389f36,
    0x389f52,
    0
};

static const u32 usa_all_ranks_offset[] =
{
    0x389be6,
    0x389c02,
    0
};

static const u32 jap_all_ranks_offset[] =
{
    0x389716,
    0x389732,
    0
};

static const u32 kor_all_ranks_offset[] =
{
    0x38a34e,
    0x38a36a,
    0
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			data: vs/bat			///////////////
///////////////////////////////////////////////////////////////////////////////

static const u32 pal_vs_offset[] =
{
    0x389f1b,
    0x389f24,
    0x389f9e,
    0x389fa7,
    0
};

static const u32 usa_vs_offset[] =
{
    0x389bcb,
    0x389bd4,
    0x389c4e,
    0x389c57,
    0
};

static const u32 jap_vs_offset[] =
{
    0x3896fb,
    0x389704,
    0x38977e,
    0x389787,
    0
};

static const u32 kor_vs_offset[] =
{
    0x38a333,
    0x38a33c,
    0x38a3b6,
    0x38a3bf,
    0
};

//-----------------------------------------------------------------------------

static const u32 pal_bt_offset[] =
{
    0x389f2a,
    0x389f33,
    0x389fad,
    0x389fb6,
    0
};

static const u32 usa_bt_offset[] =
{
    0x389bda,
    0x389be3,
    0x389c5d,
    0x389c66,
    0
};

static const u32 jap_bt_offset[] =
{
    0x38970a,
    0x389713,
    0x38978d,
    0x389796,
    0
};

static const u32 kor_bt_offset[] =
{
    0x38a342,
    0x38a34b,
    0x38a3c5,
    0x38a3ce,
    0
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			data: cannons			///////////////
///////////////////////////////////////////////////////////////////////////////

#define CANNON_N	 3
#define CANNON_SIZE	16
#define CANNON_N_PARAM	 4

typedef struct cannon_param_t
{
    bool  valid;			// TRUE: params are valid
    float param[CANNON_N_PARAM];	// parameters
}
cannon_param_t;

static cannon_param_t cannon_param[CANNON_N] = {{0}};

//-----------------------------------------------------------------------------

static u32 GetCannonOffset ( const staticr_t * str )
{
    ASSERT( sizeof(CannonDataLEX)-4 == sizeof(str->cannon_data) );
    ASSERT( sizeof(CannonDataLEX)-4 == CANNON_SIZE * CANNON_N );

    if ( str && str->is_staticr )
    {
	switch (str->mode)
	{
	    case STR_M_PAL:	return 0x3a5a08;
	    case STR_M_USA:	return 0x3a54c8;
	    case STR_M_JAP:	return 0x3a51e8;
	    case STR_M_KOR:	return 0x3a5e60;
	    default:		return 0;
	}
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			data: /Scene/UI/MenuSingle	///////////////
///////////////////////////////////////////////////////////////////////////////

static const u32 pal_menu_offset[] =
{
	0x387b64,  // /Scene/UI/MenuSingle
	0x387b9d,  // /Scene/UI/MenuSingle
	0x387bea,  // /Scene/UI/MenuSingle
	0x387c48,  // /Scene/UI/MenuSingle
	0x387cd9,  // /Scene/UI/MenuSingle

	0x387b79,  // /Scene/UI/MenuMulti
	0x387bb2,  // /Scene/UI/MenuMulti
	0x387bd6,  // /Scene/UI/MenuMulti
	0x387bff,  // /Scene/UI/MenuMulti

	0
};

static const u32 usa_menu_offset[] =
{
	0x38513c,  // /Scene/UI/MenuSingle
	0x385175,  // /Scene/UI/MenuSingle
	0x3851c2,  // /Scene/UI/MenuSingle
	0x385220,  // /Scene/UI/MenuSingle
	0x3852b1,  // /Scene/UI/MenuSingle

	0x385151,  // /Scene/UI/MenuMulti
	0x38518a,  // /Scene/UI/MenuMulti
	0x3851ae,  // /Scene/UI/MenuMulti
	0x3851d7,  // /Scene/UI/MenuMulti

	0
};

static const u32 jap_menu_offset[] =
{
	0x387834,  // /Scene/UI/MenuSingle
	0x38786d,  // /Scene/UI/MenuSingle
	0x3878ba,  // /Scene/UI/MenuSingle
	0x387918,  // /Scene/UI/MenuSingle
	0x3879a9,  // /Scene/UI/MenuSingle

	0x387849,  // /Scene/UI/MenuMulti
	0x387882,  // /Scene/UI/MenuMulti
	0x3878a6,  // /Scene/UI/MenuMulti
	0x3878cf,  // /Scene/UI/MenuMulti

	0
};

static const u32 kor_menu_offset[] =
{
	0x38846c,  // /Scene/UI/MenuSingle
	0x3884a5,  // /Scene/UI/MenuSingle
	0x3884f2,  // /Scene/UI/MenuSingle
	0x388550,  // /Scene/UI/MenuSingle
	0x3885e1,  // /Scene/UI/MenuSingle

	0x388481,  // /Scene/UI/MenuMulti
	0x3884ba,  // /Scene/UI/MenuMulti
	0x3884de,  // /Scene/UI/MenuMulti
	0x388507,  // /Scene/UI/MenuMulti

	0
};

static const int meno_patch_idx[] =
{
	1,	// Single: Global VS + Battle
	2,	// Single: Room
	7,	// Multi: Global VS + Battle
	8,	// Multi: Room
	-1
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			data: servers			///////////////
///////////////////////////////////////////////////////////////////////////////

const str_server_list_t ServerList[SERV__N+1] =
{
    { SERV_NONE,	"NONE",       0,  0, "" },
    { SERV_NASW,	"NASW",      90, 23, "naswii.nintendowifi.net" },
    { SERV_NASW_DEV,	"NASW_DEV",  91, 27, "naswii.dev.nintendowifi.net" },
    { SERV_NASW_TEST,	"NASW_TEST", 92, 28, "naswii.test.nintendowifi.net" },
    { SERV_MKW_RACE,	"MKW_RACE",  93, 37, "mariokartwii.race.gs.nintendowifi.net" },
    { SERV_SAKE,	"SAKE",	      0, 37, "mariokartwii.sake.gs.nintendowifi.net" },
    { SERV_S_SAKE,	"S_SAKE",     0, 27, "%s.sake.gs.nintendowifi.net" },
    { SERV_GS,		"GS",	      0, 19, "gs.nintendowifi.net" },
    {0,0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

const str_server_patch_t ServerPatch[217+1] =
{
  { STR_M_PAL,0, 2,SERV_NONE,      0x38aa32,  5, "" },
  { STR_M_PAL,0, 2,SERV_NONE,      0x38ab4c,  5, "" },
  { STR_M_PAL,2, 2,SERV_MKW_RACE,  0x38a38c, 75, "/raceservice/maindl_%s_%s.ashx" },
  { STR_M_PAL,2, 2,SERV_MKW_RACE,  0x38a458, 78, "/raceservice/messagedl_%s_%s.ashx" },
  { STR_M_PAL,0, 2,SERV_MKW_RACE,  0x38a8f0, 84, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 1,SERV_MKW_RACE,  0x38a945, 83, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 2,SERV_SAKE,      0x38a3d8, 100, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38a9bc, 117, "/SakeFileServer/ghostupload.aspx?gameid=%d&regionid=%d&courseid=%d&score=%d&pid=%d&playerinfo=%s%s" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38aaef, 92, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0&p0=%d&c0=%d&t0=%d" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38ac9c, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38ae05, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38af33, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38b014, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38b109, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38b21e, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_PAL,0, 0,SERV_GS,        0x38b330, 58, "/RaceService/NintendoRacingService.asmx" },

  { STR_M_PAL,9, 2,SERV_NONE,      0x2797f8,  8, "" }, // "https://"
  { STR_M_PAL,9, 2,SERV_NONE,      0x279808,  8, "" }, // "https://"
  { STR_M_PAL,9, 2,SERV_NONE,      0x279a78,  8, "" }, // "https://"
  { STR_M_PAL,9, 2,SERV_NONE,      0x29d568,  8, "" }, // "https://"
  { STR_M_PAL,9, 2,SERV_NONE,      0x29e388,  8, "" }, // "https://"

  { STR_M_PAL,5, 2,SERV_NASW,      0x276528, 34, "/ac" },
  { STR_M_PAL,1, 2,SERV_NASW,      0x2765a8, 34, "/pr" },
  { STR_M_PAL,5, 2,SERV_NASW_DEV,  0x27654c, 38, "/ac" },
  { STR_M_PAL,1, 2,SERV_NASW_DEV,  0x2765cc, 38, "/pr" },
  { STR_M_PAL,5, 2,SERV_NASW_TEST, 0x276500, 39, "/ac" },
  { STR_M_PAL,1, 2,SERV_NASW_TEST, 0x276580, 39, "/pr" },
  { STR_M_PAL,1, 2,SERV_S_SAKE,    0x275e58, 72, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_PAL,1, 2,SERV_S_SAKE,    0x275ea4, 62, "/SakeFileServer/upload.aspx" },
  { STR_M_PAL,1, 2,SERV_S_SAKE,    0x275ee4, 64, "/SakeFileServer/download.aspx" },
  { STR_M_PAL,1, 1,SERV_S_SAKE,    0x27a1a0, 71, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_PAL,1, 0,SERV_GS,        0x2718be, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272635, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272651, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272672, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272693, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x2726b5, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x2726d7, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x2726f7, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272717, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272736, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x272759, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x27277e, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x27279c, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x276e65, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x2775d5, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x277fed, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x27918e, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x279b82, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x279da8, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x279dc4, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x279de0, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x279f30, 19, "" },
  { STR_M_PAL,1, 0,SERV_GS,        0x279fb4, 19, "" },

  { STR_M_USA,0, 2,SERV_NONE,      0x38a3ba,  5, "" },
  { STR_M_USA,0, 2,SERV_NONE,      0x38a4d4,  5, "" },
  { STR_M_USA,2, 2,SERV_MKW_RACE,  0x389d14, 75, "/raceservice/maindl_%s_%s.ashx" },
  { STR_M_USA,2, 2,SERV_MKW_RACE,  0x389de0, 78, "/raceservice/messagedl_%s_%s.ashx" },
  { STR_M_USA,0, 2,SERV_MKW_RACE,  0x38a278, 84, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 1,SERV_MKW_RACE,  0x38a2cd, 83, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 2,SERV_SAKE,      0x389d60, 100, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0" },
  { STR_M_USA,0, 0,SERV_GS,        0x38a344, 117, "/SakeFileServer/ghostupload.aspx?gameid=%d&regionid=%d&courseid=%d&score=%d&pid=%d&playerinfo=%s%s" },
  { STR_M_USA,0, 0,SERV_GS,        0x38a477, 92, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0&p0=%d&c0=%d&t0=%d" },
  { STR_M_USA,0, 0,SERV_GS,        0x38a604, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 0,SERV_GS,        0x38a76d, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 0,SERV_GS,        0x38a89b, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 0,SERV_GS,        0x38a97c, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 0,SERV_GS,        0x38aa71, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 0,SERV_GS,        0x38ab86, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_USA,0, 0,SERV_GS,        0x38ac98, 58, "/RaceService/NintendoRacingService.asmx" },

  { STR_M_USA,9, 2,SERV_NONE,      0x275498,  8, "" }, // "https://"
  { STR_M_USA,9, 2,SERV_NONE,      0x2754a8,  8, "" }, // "https://"
  { STR_M_USA,9, 2,SERV_NONE,      0x275718,  8, "" }, // "https://"
  { STR_M_USA,9, 2,SERV_NONE,      0x299208,  8, "" }, // "https://"
  { STR_M_USA,9, 2,SERV_NONE,      0x29a028,  8, "" }, // "https://"

  { STR_M_USA,5, 2,SERV_NASW,      0x2721c8, 34, "/ac" },
  { STR_M_USA,1, 2,SERV_NASW,      0x272248, 34, "/pr" },
  { STR_M_USA,5, 2,SERV_NASW_DEV,  0x2721ec, 38, "/ac" },
  { STR_M_USA,1, 2,SERV_NASW_DEV,  0x27226c, 38, "/pr" },
  { STR_M_USA,5, 2,SERV_NASW_TEST, 0x2721a0, 39, "/ac" },
  { STR_M_USA,1, 2,SERV_NASW_TEST, 0x272220, 39, "/pr" },
  { STR_M_USA,1, 2,SERV_S_SAKE,    0x271af8, 72, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_USA,1, 2,SERV_S_SAKE,    0x271b44, 62, "/SakeFileServer/upload.aspx" },
  { STR_M_USA,1, 2,SERV_S_SAKE,    0x271b84, 64, "/SakeFileServer/download.aspx" },
  { STR_M_USA,1, 1,SERV_S_SAKE,    0x275e40, 71, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_USA,1, 0,SERV_GS,        0x26d55e, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e2d5, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e2f1, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e312, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e333, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e355, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e377, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e397, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e3b7, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e3d6, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e3f9, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e41e, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x26e43c, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x272b05, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x273275, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x273c8d, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x274e2e, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x275822, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x275a48, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x275a64, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x275a80, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x275bd0, 19, "" },
  { STR_M_USA,1, 0,SERV_GS,        0x275c54, 19, "" },

  { STR_M_JAP,0, 2,SERV_NONE,      0x38a212,  5, "" },
  { STR_M_JAP,0, 2,SERV_NONE,      0x38a32c,  5, "" },
  { STR_M_JAP,2, 2,SERV_MKW_RACE,  0x389b6c, 75, "/raceservice/maindl_%s_%s.ashx" },
  { STR_M_JAP,2, 2,SERV_MKW_RACE,  0x389c38, 78, "/raceservice/messagedl_%s_%s.ashx" },
  { STR_M_JAP,0, 2,SERV_MKW_RACE,  0x38a0d0, 84, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 1,SERV_MKW_RACE,  0x38a125, 83, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 2,SERV_SAKE,      0x389bb8, 100, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a19c, 117, "/SakeFileServer/ghostupload.aspx?gameid=%d&regionid=%d&courseid=%d&score=%d&pid=%d&playerinfo=%s%s" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a2cf, 92, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0&p0=%d&c0=%d&t0=%d" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a47c, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a5e5, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a713, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a7f4, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a8e9, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38a9fe, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_JAP,0, 0,SERV_GS,        0x38ab10, 58, "/RaceService/NintendoRacingService.asmx" },

  { STR_M_JAP,9, 2,SERV_NONE,      0x279178,  8, "" }, // "https://"
  { STR_M_JAP,9, 2,SERV_NONE,      0x279188,  8, "" }, // "https://"
  { STR_M_JAP,9, 2,SERV_NONE,      0x2793f8,  8, "" }, // "https://"
  { STR_M_JAP,9, 2,SERV_NONE,      0x29cee8,  8, "" }, // "https://"
  { STR_M_JAP,9, 2,SERV_NONE,      0x29dd08,  8, "" }, // "https://"

  { STR_M_JAP,5, 2,SERV_NASW,      0x275ea8, 34, "/ac" },
  { STR_M_JAP,1, 2,SERV_NASW,      0x275f28, 34, "/pr" },
  { STR_M_JAP,5, 2,SERV_NASW_DEV,  0x275ecc, 38, "/ac" },
  { STR_M_JAP,1, 2,SERV_NASW_DEV,  0x275f4c, 38, "/pr" },
  { STR_M_JAP,5, 2,SERV_NASW_TEST, 0x275e80, 39, "/ac" },
  { STR_M_JAP,1, 2,SERV_NASW_TEST, 0x275f00, 39, "/pr" },
  { STR_M_JAP,1, 2,SERV_S_SAKE,    0x2757d8, 72, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_JAP,1, 2,SERV_S_SAKE,    0x275824, 62, "/SakeFileServer/upload.aspx" },
  { STR_M_JAP,1, 2,SERV_S_SAKE,    0x275864, 64, "/SakeFileServer/download.aspx" },
  { STR_M_JAP,1, 1,SERV_S_SAKE,    0x279b20, 71, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_JAP,1, 0,SERV_GS,        0x27123e, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x271fb5, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x271fd1, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x271ff2, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x272013, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x272035, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x272057, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x272077, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x272097, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x2720b6, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x2720d9, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x2720fe, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x27211c, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x2767e5, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x276f55, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x27796d, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x278b0e, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x279502, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x279728, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x279744, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x279760, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x2798b0, 19, "" },
  { STR_M_JAP,1, 0,SERV_GS,        0x279934, 19, "" },

  { STR_M_KOR,2, 2,SERV_MKW_RACE,  0x38a7a4, 75, "/raceservice/maindl_%s_%s.ashx" },
  { STR_M_KOR,2, 2,SERV_MKW_RACE,  0x38a858, 78, "/raceservice/messagedl_%s_%s.ashx" },
  { STR_M_KOR,0, 2,SERV_MKW_RACE,  0x38ad10, 84, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 2,SERV_MKW_RACE,  0x38ad65, 89, "/RaceServiceStage/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 2,SERV_SAKE,      0x38a7f0, 100, "/SakeFileServer/ghostdownload.aspx?gameid=1687&region=0" },
  { STR_M_KOR,0, 2,SERV_SAKE,      0x38adcb, 131, "/%s/ghostupload.aspx?gameid=%d&regionid=%d&courseid=%d&score=%d&pid=%d&playerinfo=%s%s" },
  { STR_M_KOR,0, 2,SERV_SAKE,      0x38af0b, 106, "/%s/ghostdownload.aspx?gameid=1687&region=0&p0=%d&c0=%d&t0=%d" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b0dc, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b245, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b373, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b454, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b549, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b65e, 58, "/RaceService/NintendoRacingService.asmx" },
  { STR_M_KOR,0, 0,SERV_GS,        0x38b770, 58, "/RaceService/NintendoRacingService.asmx" },

  { STR_M_KOR,9, 2,SERV_NONE,      0x2676a8,  8, "" }, // "https://"
  { STR_M_KOR,9, 2,SERV_NONE,      0x2676b8,  8, "" }, // "https://"
  { STR_M_KOR,9, 2,SERV_NONE,      0x267928,  8, "" }, // "https://"
  { STR_M_KOR,9, 2,SERV_NONE,      0x28b570,  8, "" }, // "https://"
  { STR_M_KOR,9, 2,SERV_NONE,      0x28c390,  8, "" }, // "https://"

  { STR_M_KOR,5, 2,SERV_NASW,      0x2643d8, 34, "/ac" },
  { STR_M_KOR,1, 2,SERV_NASW,      0x264458, 34, "/pr" },
  { STR_M_KOR,5, 2,SERV_NASW_DEV,  0x2643fc, 38, "/ac" },
  { STR_M_KOR,1, 2,SERV_NASW_DEV,  0x26447c, 38, "/pr" },
  { STR_M_KOR,5, 2,SERV_NASW_TEST, 0x2643b0, 39, "/ac" },
  { STR_M_KOR,1, 2,SERV_NASW_TEST, 0x264430, 39, "/pr" },
  { STR_M_KOR,1, 2,SERV_S_SAKE,    0x263c38, 72, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_KOR,1, 2,SERV_S_SAKE,    0x263c84, 62, "/SakeFileServer/upload.aspx" },
  { STR_M_KOR,1, 2,SERV_S_SAKE,    0x263cc4, 64, "/SakeFileServer/download.aspx" },
  { STR_M_KOR,1, 2,SERV_S_SAKE,    0x263d08, 75, "/SakeStorageServerDev/StorageServer.asmx" },
  { STR_M_KOR,1, 2,SERV_S_SAKE,    0x263d54, 65, "/SakeFileServerDev/upload.aspx" },
  { STR_M_KOR,1, 2,SERV_S_SAKE,    0x263d98, 67, "/SakeFileServerDev/download.aspx" },
  { STR_M_KOR,1, 1,SERV_S_SAKE,    0x268050, 71, "/SakeStorageServer/StorageServer.asmx" },
  { STR_M_KOR,1, 0,SERV_GS,        0x25f69e, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260415, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260431, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260452, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260473, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260495, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x2604b7, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x2604d7, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x2604f7, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260516, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x260539, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x26055e, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x26057c, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x264d15, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x265485, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x265e9d, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x26703e, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x267a32, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x267c58, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x267c74, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x267c90, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x267de0, 19, "" },
  { STR_M_KOR,1, 0,SERV_GS,        0x267e64, 19, "" },

  {0,0,0,0,0,0,0}
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  data access			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetStrModeName ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_UNKNOWN: return "---";
	case STR_M_PAL:	    return "PAL";
	case STR_M_USA:	    return "USA";
	case STR_M_JAP:	    return "JAP";
	case STR_M_KOR:	    return "KOR";
	case STR_M__N:	    return "???";
    }
    return "???";
}

///////////////////////////////////////////////////////////////////////////////

char GetStrModeRegionChar ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:	    return 'P';
	case STR_M_USA:	    return 'E';
	case STR_M_JAP:	    return 'J';
	case STR_M_KOR:	    return 'K';
	default:	    return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

static u32 GetIdAddress ( const staticr_t * str )
{
    if (str)
    {
	switch (str->mode)
	{
	    case STR_M_PAL:	return 0x80276054;
	    case STR_M_USA:	return 0x80271d14;
	    case STR_M_JAP:	return 0x802759f4;
	    case STR_M_KOR:	return 0x80263e34;
	    default:		return 0;
	}
    }
    return 0;
}

//-----------------------------------------------------------------------------

static char * GetIdPointer ( const staticr_t * str )
{
    const u32 addr = GetIdAddress(str);
    if (addr)
    {
	const dol_header_t *dol_head = (dol_header_t*)str->data;
	const u32 off = GetDolOffsetByAddr(dol_head,addr,PATCHED_BY_SIZE,0);
	if (off)
	    return (char*)str->data + off;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const u32 * GetTrackOffsetTab ( const staticr_t * str )
{
    if (str)
    {
	switch (str->mode)
	{
	    case STR_M_PAL:	return pal_track_offset;
	    case STR_M_USA:	return usa_track_offset;
	    case STR_M_JAP:	return jap_track_offset;
	    case STR_M_KOR:	return kor_track_offset;
	    default:		return 0;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const u32 * GetArenaOffsetTab ( const staticr_t * str )
{
    if (str)
    {
	switch (str->mode)
	{
	    case STR_M_PAL:	return pal_arena_offset;
	    case STR_M_USA:	return usa_arena_offset;
	    case STR_M_JAP:	return jap_arena_offset;
	    case STR_M_KOR:	return kor_arena_offset;
	    default:		return 0;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const u32 * GetAllRanksOffsetTab ( const staticr_t * str )
{
    if (str)
    {
	switch (str->mode)
	{
	    case STR_M_PAL:	return pal_all_ranks_offset;
	    case STR_M_USA:	return usa_all_ranks_offset;
	    case STR_M_JAP:	return jap_all_ranks_offset;
	    case STR_M_KOR:	return kor_all_ranks_offset;
	    default:		return 0;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const u32 * GetVsBtOffsetTab ( const staticr_t * str, bool battle )
{
    if (str)
    {
	if (battle)
	{
	    switch (str->mode)
	    {
		case STR_M_PAL:	return pal_bt_offset;
		case STR_M_USA:	return usa_bt_offset;
		case STR_M_JAP:	return jap_bt_offset;
		case STR_M_KOR:	return kor_bt_offset;
		default: return 0;
	    }
	}
	else
	{
	    switch (str->mode)
	    {
		case STR_M_PAL:	return pal_vs_offset;
		case STR_M_USA:	return usa_vs_offset;
		case STR_M_JAP:	return jap_vs_offset;
		case STR_M_KOR:	return kor_vs_offset;
		default: return 0;
	    }
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const u32 * GetMenuOffsetTab ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:  return pal_menu_offset;
	case STR_M_USA:  return usa_menu_offset;
	case STR_M_JAP:  return jap_menu_offset;
	case STR_M_KOR:  return kor_menu_offset;
	default:	 return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

#define ORIG_VBI_VALUE 0x4e800020

static u32 GetVbiAddress ( const staticr_t * str )
{
    DASSERT(str);
    switch (str->mode)
    {
	case STR_M_PAL:  return 0x801bab20;
	case STR_M_USA:	 return 0x801baa80;
	case STR_M_JAP:	 return 0x801baa40;
	case STR_M_KOR:  return 0x801bae7c;
	default:	 return FindDolAddressOfVBI(str->data,str->data_size);
    }
}

///////////////////////////////////////////////////////////////////////////////

#define ORIG_SDK_VERS "001000\0" // one more '\0' is added by compiler

static u32 GetSdkverAddress ( str_mode_t mode )
{
    // return address of string '001000\0\0'
    // string 'sdkver\0\0' follows with offset +8

    switch (mode)
    {
	case STR_M_PAL:  return 0x80384fd0;
	case STR_M_USA:	 return 0x80380c50;
	case STR_M_JAP:	 return 0x80384950;
	case STR_M_KOR:  return 0x80372fd0;
	default:	 return 0;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		second host at nas login		///////////////
///////////////////////////////////////////////////////////////////////////////

static u32 GetSecondHostAddress ( str_mode_t mode, u32 *ret_orig )
{
    // PAL: Offset 800ed868, original 0x480eb86d, neu 0x60000000
    // USA: Offset 800ed7c8, original 0x480eb86d, neu 0x60000000
    // JAP: Offset 800ed788, original 0x480eb86d, neu 0x60000000
    // KOR: Offset 800ed8e0, original 0x480ebb51, neu 0x60000000

    if (ret_orig)
	*ret_orig = mode == STR_M_KOR ? 0x480ebb51 : 0x480eb86d;

    switch (mode)
    {
	case STR_M_PAL:  return 0x800ed868;
	case STR_M_USA:	 return 0x800ed7c8;
	case STR_M_JAP:	 return 0x800ed788;
	case STR_M_KOR:  return 0x800ed8e0;
	default:	 return 0;
    }
}

//-----------------------------------------------------------------------------

static u32 * GetSecondHostPointer ( const staticr_t * str, u32 *ret_orig )
{
    const u32 addr = GetSecondHostAddress(str->mode,ret_orig);
    if (addr)
    {
	const dol_header_t *dol_head = (dol_header_t*)str->data;
	const u32 off = GetDolOffsetByAddr(dol_head,addr,4,0);
	if (off)
	    return (u32*)(str->data + off);
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			wiimmfi codes			///////////////
///////////////////////////////////////////////////////////////////////////////

static const u8 wiimmfi_code_pal_bz2[] =
{
  // 4 bytes decompressed len
  0x00,0x00,0x01,0x18,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0xe0,0xa4, 0xf4,0x97,0x00,0x00,
  0x27,0x7f,0xfd,0xff, 0xd7,0xcd,0x42,0xe6, 0x56,0x00,0x5c,0x74, 0x65,0x84,0x00,0x60,
  0x42,0x00,0x24,0x7c, 0x42,0x44,0x80,0x6d, 0x6c,0x00,0x04,0x54, 0x30,0x09,0x00,0xc4,
  0x5c,0xb0,0x00,0xd9, 0x8c,0x70,0xd3,0x4c, 0x10,0xc8,0x69,0xa6, 0x46,0x4c,0x20,0x1a,
  0x68,0x03,0x09,0xa3, 0x4c,0x98,0x00,0x40, 0xd0,0x6a,0x69,0x46, 0xd4,0xd0,0xd1,0xa4,
  0xc0,0x00,0x32,0x00, 0x00,0x8c,0x0d,0x00, 0x34,0x32,0x4c,0xc9, 0xa1,0x04,0x4d,0x4d,
  0x06,0x80,0x00,0x06, 0x80,0x34,0x0d,0x01, 0xa1,0x90,0x34,0x0c, 0x80,0x03,0x12,0x46,
  0x06,0xbe,0x9a,0x92, 0xa6,0x1b,0x60,0xf3, 0x6f,0x35,0x10,0x01, 0xa1,0xd6,0xd4,0x64,
  0x10,0xcb,0x04,0x73, 0x81,0xc8,0x82,0x20, 0x03,0x02,0xca,0xcd, 0x3d,0x4c,0xe4,0xb8,
  0xac,0x91,0xdc,0xa1, 0x90,0xe1,0xa8,0x81, 0xa2,0x9c,0x05,0xc6, 0xa0,0xf2,0x82,0xaa,
  0x02,0x49,0x45,0x0c, 0x04,0x26,0x10,0x90, 0xdc,0xf7,0x57,0x43, 0x99,0x8c,0x29,0x26,
  0x3f,0x8e,0x05,0x82, 0x03,0xa9,0x1a,0xe8, 0xde,0x7b,0x29,0x22, 0xfc,0x74,0x21,0x91,
  0x50,0x43,0x29,0x9d, 0xf4,0x8c,0x69,0xff, 0xb1,0x09,0xa4,0x5f, 0xd0,0xfc,0x66,0x24,
  0xd5,0x53,0x6c,0x59, 0x0d,0xb6,0x07,0xe0, 0x1b,0x98,0xcf,0xa9, 0xf1,0xa0,0x4b,0x23,
  0x2e,0xbf,0xcd,0xce, 0x59,0x65,0xdf,0x8d, 0xcd,0xa4,0xac,0x46, 0xc0,0xac,0x91,0xab,
  0xf8,0x3b,0x2b,0x49, 0x4c,0x24,0x1a,0x22, 0x3f,0x99,0xed,0x48, 0x28,0x06,0xad,0x2d,
  0x19,0x7b,0x07,0x61, 0x05,0x5c,0xf6,0x43, 0x93,0x95,0x54,0x58, 0xc0,0xe1,0xfb,0xba,
  0xd7,0x72,0x1a,0x68, 0x3a,0x9e,0x81,0xde, 0xa2,0xee,0x07,0x9e, 0x5a,0x9b,0x3c,0x04,
  0x9f,0xf8,0xbb,0x92, 0x29,0xc2,0x84,0x87, 0x05,0x27,0xa4,0xb8,
  // [300 Bytes]
};

static BZ2Manager_t wiimmfi_code_pal_mgr
	= { wiimmfi_code_pal_bz2, sizeof(wiimmfi_code_pal_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static const u8 wiimmfi_code_usa_bz2[] =
{
  // 4 bytes decompressed len
  0x00,0x00,0x01,0x18,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x24,0x09, 0x21,0x7b,0x00,0x00,
  0x28,0x7f,0xfd,0xff, 0xd7,0xcd,0x42,0xe6, 0x56,0x00,0x5c,0x70, 0x65,0x84,0x44,0x60,
  0x42,0x00,0x24,0x7c, 0x42,0x44,0x80,0x6d, 0x6c,0x00,0x04,0x54, 0x30,0x09,0x00,0xc0,
  0x18,0xb0,0x00,0xd9, 0xb4,0x22,0x50,0x64, 0x68,0x30,0x98,0x40, 0x68,0x18,0x8c,0x02,
  0x00,0x68,0xc8,0x00, 0xc2,0x61,0x04,0x53, 0xd2,0x32,0x9a,0x32, 0x68,0x00,0x34,0x0c,
  0x80,0x1a,0x06,0x81, 0xa0,0x00,0x1a,0x68, 0xc4,0xf5,0x06,0xa6, 0xa7,0xa9,0xa8,0x00,
  0x06,0x80,0x00,0x03, 0x40,0x01,0xa1,0x90, 0x00,0x69,0xa0,0x0e, 0x65,0x49,0x81,0x7c,
  0xa8,0xd3,0x51,0x16, 0x82,0x77,0xf6,0x80, 0x90,0x34,0xb7,0xc7, 0x83,0x21,0x2c,0xb0,
  0x46,0x60,0xc8,0x84, 0x40,0x18,0x43,0x2b, 0xb4,0xf1,0xd9,0x96, 0x24,0x2e,0xa9,0xf7,
  0xa1,0x98,0xc6,0xc2, 0x0d,0x14,0xd0,0x5d, 0x2a,0xa3,0xac,0x1a, 0x89,0x0b,0x14,0x51,
  0x84,0x90,0xa0,0x0e, 0x12,0xdc,0xa7,0xf8, 0xc6,0x96,0xd8,0x51, 0x0a,0xbb,0x40,0xe8,
  0x80,0x5c,0x10,0x0f, 0x42,0x83,0x41,0xcc, 0x97,0x4a,0x6c,0x73, 0x0c,0x67,0x42,0xc0,
  0x28,0x34,0x5e,0xda, 0x0d,0xa6,0xff,0x18, 0xc9,0x15,0xa7,0x12, 0x0d,0x02,0x9a,0xca,
  0x2a,0x64,0x9f,0x40, 0x39,0x96,0x00,0xf7, 0x4d,0xdc,0x7a,0xfb, 0xa8,0x2a,0xd6,0xe8,
  0x09,0x44,0xe7,0xed, 0x3e,0x7c,0x91,0x47, 0x1d,0xe9,0x8c,0xb2, 0x58,0x06,0x31,0x84,
  0xa4,0x54,0x60,0x48, 0xea,0xa0,0x30,0x7c, 0x11,0x22,0xcb,0xbd, 0x2c,0x62,0x20,0xc4,
  0xe3,0x36,0xe8,0x45, 0x05,0xa1,0x80,0x43, 0xa4,0x35,0x99,0x55, 0x94,0x01,0x10,0x2b,
  0x8c,0x40,0xa4,0x42, 0x91,0x57,0xc0,0x87, 0x5c,0xe7,0x3a,0x38, 0x8d,0xef,0xf8,0xbb,
  0x92,0x29,0xc2,0x84, 0x81,0x20,0x49,0x0b, 0xd8,
  // [297 Bytes]
};

static BZ2Manager_t wiimmfi_code_usa_mgr
	= { wiimmfi_code_usa_bz2, sizeof(wiimmfi_code_usa_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static const u8 wiimmfi_code_jap_bz2[] =
{
  // 4 bytes decompressed len
  0x00,0x00,0x01,0x18,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x19,0x2d, 0x5f,0x73,0x00,0x00,
  0x27,0x7f,0xfd,0xff, 0xd7,0xcd,0x46,0xe6, 0x56,0x00,0x5c,0x70, 0x65,0x84,0x00,0x64,
  0x42,0x00,0x24,0x7c, 0x42,0x44,0x80,0x6d, 0x6c,0x00,0x04,0x54, 0x24,0x11,0x00,0xc0,
  0x18,0xb0,0x00,0xd9, 0x34,0x35,0x10,0x6a, 0x68,0x34,0xd0,0x34, 0x00,0x34,0x34,0x64,
  0x34,0xd0,0x00,0xd0, 0x01,0xa6,0x26,0x80, 0x22,0x9e,0x94,0x64, 0x19,0x0d,0x1a,0x32,
  0x06,0x8d,0x34,0x00, 0x34,0x1a,0x62,0x34, 0x00,0x01,0x8d,0x47, 0xa8,0x41,0x13,0x53,
  0x40,0x00,0xd0,0x01, 0xa0,0x0d,0x03,0x40, 0xf5,0x06,0x40,0xd0, 0x32,0x00,0x32,0xa8,
  0x60,0xea,0xb1,0xaa, 0x2e,0x61,0xb8,0x0f, 0xb8,0x2b,0x09,0x00, 0x1a,0x5e,0x73,0x83,
  0x40,0x86,0x92,0x47, 0x38,0x1c,0x88,0x23, 0x80,0x24,0x5a,0x61, 0xb7,0xea,0x9c,0x99,
  0x16,0x14,0x3b,0xb4, 0xb3,0x1c,0x36,0x10, 0x36,0x54,0x00,0xb9, 0xd6,0x1e,0x91,0x17,
  0x40,0x2c,0x52,0x03, 0x11,0x09,0x84,0x25, 0xb9,0x45,0xe6,0x50, 0xea,0x73,0x9c,0x53,
  0xb5,0x58,0x24,0xfc, 0x88,0x0c,0x14,0x0e, 0xe5,0x23,0x43,0x89, 0xcc,0xa2,0x66,0xd5,
  0x31,0x9d,0x16,0x84, 0x72,0xd3,0x79,0xc1, 0x99,0x2f,0x5b,0xe3, 0x42,0x66,0xf6,0xa0,
  0x34,0x95,0x25,0x24, 0x27,0x32,0x1f,0x20, 0x72,0x96,0x07,0x0d, 0x6e,0x20,0x48,0x8a,
  0x94,0x96,0x17,0x5f, 0xef,0x2f,0xac,0x40, 0x7b,0x2f,0xd0,0xf6, 0xa2,0x6c,0x00,0xb2,
  0x24,0xb8,0xf1,0xce, 0xad,0x39,0x20,0x88, 0x7c,0x44,0x90,0x55, 0xf6,0xa0,0xc8,0x39,
  0x85,0x10,0xd3,0xe4, 0x25,0x04,0xd2,0xc4, 0x21,0xcc,0xb6,0x11, 0xb4,0x70,0x10,0x23,
  0xeb,0x63,0xe9,0x51, 0x92,0x81,0x12,0x8b, 0x00,0xc6,0x35,0x73, 0x1b,0xf1,0xed,0x1a,
  0x0f,0xe2,0xee,0x48, 0xa7,0x0a,0x12,0x03, 0x25,0xab,0xee,0x60,
  // [300 Bytes]
};

static BZ2Manager_t wiimmfi_code_jap_mgr
	= { wiimmfi_code_jap_bz2, sizeof(wiimmfi_code_jap_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static const u8 wiimmfi_code_kor_bz2[] =
{
  // 4 bytes decompressed len
  0x00,0x00,0x01,0x18,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0xc2,0x0b, 0xe6,0xb5,0x00,0x00,
  0x27,0x7f,0xfd,0xff, 0xd7,0xcd,0x42,0xe4, 0x76,0x00,0x5c,0x70, 0x61,0xd4,0x40,0x60,
  0x46,0x00,0x24,0x7c, 0x42,0x44,0x80,0x6d, 0x6c,0x00,0x04,0x54, 0x28,0x05,0x00,0xc0,
  0x18,0xb0,0x00,0xd9, 0xb5,0x11,0x28,0x34, 0x68,0x06,0x9a,0x69, 0xa6,0x4d,0x34,0xd1,
  0x91,0xa6,0x9a,0x1a, 0x18,0x20,0x0d,0x00, 0x68,0xda,0x43,0x6a, 0x0d,0x53,0xd2,0x99,
  0xa2,0x19,0x00,0xd0, 0x03,0x40,0x00,0x68, 0x00,0x00,0x00,0x34, 0x62,0x68,0x35,0x34,
  0x4d,0x46,0x80,0x00, 0x03,0x40,0x00,0x00, 0xd1,0xa0,0xd0,0x1a, 0x06,0x40,0x02,0xea,
  0x1c,0x46,0xbc,0x75, 0xa8,0x9c,0xd6,0x03, 0xad,0xfd,0x5f,0xa4, 0x0d,0x0e,0x35,0xc1,
  0x90,0x96,0x58,0x24, 0x20,0x20,0x88,0x44, 0x01,0x80,0xc9,0x5a, 0xd6,0xab,0xb4,0x16,
  0x13,0x5b,0x51,0x0d, 0xac,0xba,0x06,0xc2, 0x0c,0x94,0xd0,0x5c, 0xea,0x1b,0xa4,0x0a,
  0x69,0x12,0x0a,0x28, 0xc0,0x46,0x39,0x84, 0x76,0xa7,0x39,0x55, 0x2d,0xe6,0x38,0xf0,
  0x49,0x55,0x07,0x9f, 0x01,0x60,0x80,0xf1, 0xf6,0xf0,0xec,0x88, 0x16,0xae,0xa8,0xca,
  0xe7,0x3d,0x30,0xa9, 0x4c,0x6b,0x50,0xc4, 0x9b,0x7b,0x9f,0xed, 0x3b,0x3a,0x51,0x19,
  0x49,0x48,0x4a,0xad, 0x17,0xe5,0x60,0x0d, 0xab,0x70,0xe3,0xab, 0x67,0x91,0x81,0x42,
  0x51,0x19,0x42,0xc5, 0x18,0x03,0x12,0x73, 0x93,0x26,0x68,0x86, 0x3b,0x9f,0x8d,0xab,
  0x18,0x02,0xe5,0x09, 0x88,0xa5,0xfa,0x5e, 0xc9,0x60,0x90,0xf8, 0x44,0x65,0xa8,0xb5,
  0x18,0x93,0x42,0xb2, 0xe7,0x91,0xe8,0x3f, 0x14,0xa1,0xba,0xe1, 0x0e,0xae,0xb0,0xad,
  0x54,0x01,0x82,0x2a, 0x30,0xe4,0x04,0xd2, 0x34,0x16,0xc1,0x9e, 0x9a,0x42,0x12,0x63,
  0x53,0x87,0xfc,0x5d, 0xc9,0x14,0xe1,0x42, 0x43,0x08,0x2f,0x9a, 0xd4,
  // [301 Bytes]
};

static BZ2Manager_t wiimmfi_code_kor_mgr
	= { wiimmfi_code_kor_bz2, sizeof(wiimmfi_code_kor_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static BZ2Manager_t * GetWiimmfiCode ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:  return &wiimmfi_code_pal_mgr;
	case STR_M_USA:	 return &wiimmfi_code_usa_mgr;
	case STR_M_JAP:	 return &wiimmfi_code_jap_mgr;
	case STR_M_KOR:  return &wiimmfi_code_kor_mgr;
	default:	 return 0;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lecode data			///////////////
///////////////////////////////////////////////////////////////////////////////

static const u8 lecode_loader_pal_bz2[] =
{
  // decompressed len: 0x200 = 512 bytes
  0x00,0x00,0x02,0x00,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0xa1,0xb1, 0x2c,0x5f,0x00,0x00,
  0x16,0x7f,0xff,0xfc, 0x76,0x45,0x30,0x60, 0xc7,0xc4,0x6f,0x2f, 0x6f,0xc4,0x84,0x7f,
  0xe7,0xfe,0x6e,0xdc, 0x04,0x44,0x0c,0x43, 0x60,0x00,0x15,0x54, 0x00,0x48,0x49,0x7a,
  0x40,0x00,0x01,0xb0, 0x01,0x59,0xb2,0x20, 0xd4,0xd4,0xd0,0xd1, 0x4d,0xa9,0xb5,0x34,
  0xf5,0x3d,0x40,0x7a, 0x80,0x68,0x1b,0x50, 0x64,0xf5,0x03,0x40, 0x3c,0xa1,0xa0,0x06,
  0x43,0xd4,0xc8,0xf5, 0x19,0x1e,0x88,0x7b, 0x54,0x22,0x9e,0x80, 0x98,0x4d,0x49,0xe9,
  0x3d,0x4f,0x24,0x01, 0xa0,0x00,0x0d,0x00, 0x1a,0x0d,0x00,0x00, 0xd3,0x4f,0x29,0x88,
  0x00,0x64,0x0d,0x4d, 0x48,0xd3,0x11,0x18, 0x09,0x9a,0x99,0x18, 0x9a,0x69,0x89,0x93,
  0x10,0xc9,0x84,0x32, 0x64,0x34,0x61,0x36, 0x8d,0x4c,0x68,0x8f, 0x51,0xa6,0x34,0x09,
  0x90,0x29,0xc9,0x9c, 0x3a,0x07,0xfa,0xe1, 0x92,0x40,0x83,0x36, 0x02,0x8a,0x62,0x3f,
  0xb9,0x73,0x96,0x54, 0x1d,0xea,0x43,0xbc, 0x8c,0xb1,0x75,0x00, 0xd4,0xe6,0x09,0x05,
  0xb5,0x08,0xc9,0x83, 0xc5,0x95,0xcb,0x97, 0xa4,0x10,0xfc,0x88, 0x6b,0x5c,0x06,0x08,
  0xa4,0x31,0x85,0xa7, 0x80,0x46,0xeb,0x37, 0x3a,0xa8,0x18,0x22, 0x01,0x17,0xf6,0x46,
  0x43,0x05,0xdc,0x21, 0x55,0x8e,0x6e,0xc5, 0x89,0xaa,0x1b,0x08, 0xa6,0x7c,0x55,0x37,
  0x85,0x02,0x55,0xaa, 0x2d,0x62,0xf7,0xf4, 0xde,0x0b,0x2e,0xbb, 0xf2,0xa3,0x29,0x42,
  0xe6,0x78,0x2d,0x2d, 0xeb,0x95,0xdb,0xb1, 0x49,0x30,0xb7,0x11, 0xd4,0xa1,0x0c,0x2b,
  0x2c,0x22,0x54,0xd8, 0x87,0xeb,0x40,0xe5, 0x18,0xca,0xfa,0x10, 0xe9,0x92,0x3a,0xee,
  0xc1,0x41,0x9c,0x0a, 0x45,0xe2,0x46,0xed, 0x58,0x1a,0x0a,0xf7, 0xe8,0x5c,0x42,0x4d,
  0xd6,0x77,0x31,0xf7, 0xac,0x48,0xc5,0x18, 0xe3,0x53,0x80,0x54, 0xd9,0x3d,0x52,0x40,
  0x8b,0x28,0xc4,0x73, 0x3f,0x3e,0x92,0xe1, 0x77,0xe7,0x0a,0x9a, 0xae,0x38,0xc3,0x64,
  0x75,0x14,0x14,0x14, 0x89,0x35,0x16,0x33, 0xc4,0x42,0x00,0x9d, 0xb6,0x0b,0x98,0x40,
  0xf6,0xcc,0xda,0xf4, 0x1f,0xd7,0x4c,0x76, 0x21,0x89,0x44,0x6a, 0x92,0x11,0x22,0x45,
  0x6b,0x4a,0x40,0x81, 0x27,0x8d,0xfa,0x48, 0x3e,0x21,0x02,0x47, 0x0a,0x8a,0x02,0xa2,
  0x95,0xcb,0xb2,0xe3, 0x01,0x17,0x3d,0xc5, 0x41,0xae,0x16,0xb7, 0x84,0xaf,0xa4,0x8b,
  0x62,0x06,0x18,0xd2, 0x12,0x28,0xe3,0x38, 0xa4,0x30,0x32,0x04, 0x4c,0x2a,0x4b,0x22,
  0xa4,0xe7,0x94,0x2d, 0x7c,0x63,0xf3,0x59, 0xcb,0xf7,0x6e,0xd0, 0x42,0x21,0x74,0x9b,
  0xeb,0x55,0x32,0xeb, 0xd8,0x97,0x3e,0xb7, 0xee,0x55,0xb1,0x58, 0x5d,0xb9,0x55,0xe9,
  0x47,0x11,0x18,0x7b, 0x34,0x02,0xb3,0xf9, 0xf9,0x06,0xfb,0x5b, 0x8e,0x30,0xd4,0xb2,
  0xd8,0x62,0xf8,0xae, 0x75,0x02,0x06,0x20, 0xf0,0x69,0x79,0x9e, 0x40,0x98,0x48,0xeb,
  0x63,0xa1,0x3b,0xd0, 0x1a,0xe8,0x64,0x67, 0xfe,0x51,0x09,0x4c, 0xe4,0xa7,0xbc,0xce,
  0xa1,0x94,0xad,0x87, 0xfe,0x2e,0xe4,0x8a, 0x70,0xa1,0x21,0x43, 0x62,0x58,0xbe,
  // 0x1ef = 495 Bytes
};

static BZ2Manager_t lecode_loader_pal_mgr
	= { lecode_loader_pal_bz2, sizeof(lecode_loader_pal_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static const u8 lecode_loader_usa_bz2[] =
{
  // decompressed len: 0x200 = 512 bytes
  0x00,0x00,0x02,0x00,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x9f,0x2b, 0x3c,0x9b,0x00,0x00,
  0x16,0x7f,0xff,0xfc, 0x76,0x45,0x34,0x64, 0xc7,0xc0,0x6f,0x2f, 0x6f,0x8e,0x84,0x7f,
  0xe7,0xfe,0x6e,0xdc, 0x04,0x44,0x0c,0x43, 0x60,0x00,0x15,0x50, 0x40,0x48,0x49,0x7c,
  0x80,0x00,0x01,0xb0, 0x01,0x59,0x18,0xd8, 0x6a,0x68,0x20,0x9e, 0xa9,0xa7,0xa4,0x7a,
  0x23,0x40,0x00,0x07, 0x94,0x0d,0xa8,0x32, 0x01,0x84,0x00,0x1a, 0x1e,0x93,0xd2,0x69,
  0xa0,0x00,0x7e,0xa8, 0x45,0x4f,0xd3,0x53, 0x14,0xfd,0x15,0x3d, 0xa8,0x4d,0x3c,0x90,
  0x00,0x00,0x64,0xd3, 0x4d,0x00,0x34,0x00, 0x00,0x00,0x6d,0x13, 0x20,0x1a,0x7a,0x80,
  0x4a,0x9a,0x94,0xf4, 0x64,0x35,0x34,0x34, 0x64,0x18,0x8d,0x00, 0x62,0x06,0x81,0x84,
  0x68,0xc8,0x1a,0x32, 0x06,0x99,0x33,0x51, 0xa7,0xa9,0xa6,0x8c, 0x98,0x83,0x09,0x2b,
  0xc9,0x63,0x5c,0xfe, 0x70,0xc4,0x78,0x20, 0xbd,0xe2,0xad,0x5b, 0x0e,0xa6,0x38,0xb3,
  0x75,0xc5,0xa9,0x21, 0x3d,0x14,0xda,0x6d, 0x83,0xd5,0xaf,0x23, 0x10,0x3f,0x0d,0x4c,
  0xb1,0xd1,0x49,0xc4, 0x89,0x20,0x43,0xe3, 0x41,0x54,0xda,0x50, 0xcb,0xc2,0xc8,0xdf,
  0x00,0x22,0xa5,0x2b, 0x4a,0xb3,0x0a,0x19, 0x04,0x4b,0x97,0xfd, 0x97,0x2b,0x78,0x58,
  0x12,0xbb,0xd4,0x9a, 0xeb,0x24,0xd6,0x4e, 0x32,0x12,0xe1,0x3b, 0xe1,0x62,0xf8,0xf1,
  0xc6,0x57,0x0b,0x95, 0x52,0x23,0x0f,0x2e, 0xa7,0xb0,0x2d,0x64, 0x59,0x40,0x9a,0xf8,
  0x5b,0x72,0xf9,0x6c, 0xbd,0xbd,0x3f,0xd2, 0xa9,0xce,0xf5,0x3a, 0x6a,0x50,0xa8,0x93,
  0x5e,0x23,0x99,0xb1, 0xbe,0x27,0xa6,0x32, 0x1c,0x96,0xec,0x57, 0x8f,0x5a,0xe9,0xd9,
  0x90,0x90,0x11,0x74, 0x28,0x1b,0xb2,0x46, 0x9a,0x96,0x55,0x49, 0xee,0x39,0xd8,0x90,
  0xc3,0xdd,0x6d,0xcc, 0x99,0x24,0x3e,0xe0, 0xa1,0x12,0x36,0x89, 0xb1,0x5b,0x0c,0xc4,
  0x83,0x41,0x73,0x9c, 0xd0,0x6a,0x05,0x8a, 0x10,0x3e,0xfb,0x8f, 0x12,0x0e,0x72,0x51,
  0x21,0x6f,0x61,0x35, 0xb5,0x44,0x1f,0x09, 0x55,0x01,0x91,0x08, 0x24,0x9b,0x5a,0x72,
  0xf4,0xd5,0x05,0x79, 0x73,0x4d,0x0d,0x35, 0x84,0xd5,0x42,0x7a, 0x35,0x50,0x0f,0xad,
  0x30,0x52,0x35,0x27, 0x11,0x01,0x0b,0x84, 0xc8,0x04,0xc8,0x9c, 0xcf,0x37,0x3c,0x08,
  0x65,0xbb,0x89,0x4f, 0x4b,0x63,0x24,0x09, 0x49,0x5a,0x18,0xd5, 0x0c,0xc8,0x16,0x10,
  0x9a,0x46,0x51,0x21, 0xe0,0x6b,0xea,0x88, 0x4d,0x6d,0xb8,0xa4, 0xe9,0x06,0x36,0xd2,
  0x46,0x1b,0x0c,0x49, 0x7d,0x47,0x18,0x4c, 0x10,0x81,0x77,0xb1, 0xee,0xe5,0x45,0x5b,
  0xca,0x85,0x8b,0x3f, 0x7c,0x41,0xd6,0x5d, 0xf9,0x9b,0x13,0xb4, 0x9e,0x8a,0x3d,0x5a,
  0x05,0xdf,0xf4,0xf6, 0x13,0x9a,0xe2,0xc9, 0x67,0x37,0x17,0xef, 0xf4,0xbb,0xe6,0xf5,
  0xa4,0xb9,0x4c,0x41, 0x48,0x67,0x8f,0xdd, 0xc5,0x76,0x3f,0x45, 0xaf,0xe9,0x83,0x89,
  0x94,0xb5,0x4d,0xf2, 0x1d,0x19,0x5f,0x59, 0x61,0x8c,0x5b,0x19, 0xd6,0x69,0xad,0xba,
  0xc5,0x4e,0xff,0x17, 0x72,0x45,0x38,0x50, 0x90,0x9f,0x2b,0x3c, 0x9b,
  // 0x1ed = 493 Bytes
};

static BZ2Manager_t lecode_loader_usa_mgr
	= { lecode_loader_usa_bz2, sizeof(lecode_loader_usa_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static const u8 lecode_loader_jap_bz2[] =
{
  // decompressed len: 0x200 = 512 bytes
  0x00,0x00,0x02,0x00,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x5f,0x33, 0xd7,0x7b,0x00,0x00,
  0x16,0x7f,0xff,0xfc, 0x76,0x45,0x30,0x60, 0xc7,0xc0,0x6f,0x2f, 0x7f,0xc4,0x84,0x7f,
  0xe7,0xfe,0x6e,0xdc, 0x44,0x44,0x0c,0x43, 0x60,0x00,0x15,0x50, 0x00,0x48,0x4d,0x6c,
  0x80,0x00,0x01,0xb0, 0x01,0x59,0x18,0xd8, 0x6a,0x04,0x34,0x4d, 0x4f,0x49,0xa7,0xa9,
  0xa7,0xa4,0x03,0xd4, 0x00,0x3f,0x54,0x0f, 0x28,0x00,0x6d,0x4c, 0x86,0x40,0xf4,0x8f,
  0x53,0x10,0xd0,0x00, 0x37,0xaa,0x11,0x53, 0xf4,0xd2,0x60,0x54, 0xf6,0xa1,0x3d,0x4f,
  0x6a,0x40,0x00,0x01, 0x90,0x01,0x90,0x00, 0x00,0x19,0x0c,0x98, 0x80,0x34,0x61,0x04,
  0xa9,0xa9,0x06,0x9a, 0x9b,0x48,0x1a,0x34, 0x68,0xc8,0x64,0x34, 0xc8,0xd1,0x91,0xa6,
  0x86,0x80,0x0d,0x34, 0xd0,0x64,0x7a,0x26, 0x9e,0xa1,0xb5,0x34, 0xd3,0x64,0x80,0xba,
  0x50,0x9e,0xb1,0xaa, 0x7f,0x38,0x3e,0x60, 0x41,0xfb,0xc5,0x9b, 0x37,0x20,0x53,0x71,
  0x66,0xda,0x16,0xaa, 0x87,0x0a,0x29,0xb4, 0x9e,0x18,0x8b,0x3c, 0x8b,0xb7,0x84,0xd4,
  0xcd,0x20,0x1d,0x6e, 0x24,0x99,0xc2,0x1f, 0x1a,0x4a,0xae,0xca, 0x88,0x4c,0x17,0x0f,
  0x4a,0x40,0x8a,0xa2, 0xad,0x36,0x97,0x51, 0x08,0x11,0x2f,0xb6, 0xf8,0xf2,0xef,0xb6,
  0x37,0x65,0x84,0x4a, 0xe1,0x51,0x51,0x10, 0x9d,0x09,0x2b,0x7d, 0xfd,0xd4,0x8d,0x34,
  0xc5,0x76,0x73,0x92, 0x46,0x41,0xe6,0xf6, 0x0f,0x86,0x45,0xb3, 0x87,0xef,0x83,0x8d,
  0x3b,0xb1,0x6d,0xee, 0x18,0x3d,0x64,0x31, 0xa7,0xd3,0xa6,0xc3, 0x02,0x44,0x12,0x89,
  0xeb,0x34,0x38,0x84, 0x08,0x94,0xa3,0x96, 0xe5,0x99,0x66,0x8e, 0x83,0x77,0xba,0x2d,
  0x98,0x0d,0x7e,0x76, 0x41,0x2c,0x52,0xb0, 0x96,0xae,0x82,0xd4, 0x2d,0x0a,0xd0,0x42,
  0xa5,0x76,0xf9,0x27, 0x32,0xd5,0xa3,0x70, 0x08,0xa3,0x3e,0xf1, 0x5a,0x15,0x61,0x21,
  0x4e,0x45,0x3d,0x03, 0x21,0x95,0x4e,0x11, 0x29,0xf7,0xd3,0x3c, 0xa7,0x31,0x85,0x04,
  0x64,0x9f,0x1d,0xad, 0xa6,0x21,0x08,0x49, 0xdd,0x06,0x7a,0x08, 0x40,0x4d,0xab,0x31,
  0x3b,0x48,0x8e,0xb4, 0xa8,0x96,0x66,0xad, 0x21,0x12,0x1f,0xa6, 0xcd,0x9a,0x66,0x07,
  0x29,0x0f,0x5c,0x07, 0xc8,0x30,0x3d,0xc2, 0xa2,0x01,0x51,0x13, 0x97,0x5d,0xc7,0x03,
  0x32,0xee,0x12,0x41, 0xe5,0x31,0x92,0x04, 0x94,0x93,0x31,0xa8, 0x19,0x71,0xa4,0x1e,
  0xb8,0x06,0x49,0x18, 0xe0,0xd8,0x51,0x50, 0x56,0x96,0xc5,0xaa, 0xe9,0x0b,0x1b,0xa5,
  0x0b,0xfd,0x59,0x01, 0x58,0xb5,0x64,0x25, 0x88,0x46,0x9c,0x1b, 0x3c,0x79,0xd5,0x6b,
  0x54,0x9d,0x6a,0xdd, 0x8c,0x32,0xb0,0xf7, 0xcf,0x04,0x87,0x4e, 0x44,0x51,0xec,0xc8,
  0x3f,0xc5,0xab,0xc4, 0x54,0x66,0x9c,0x5c, 0x86,0x22,0x3a,0xeb, 0x4c,0x43,0x4b,0xf5,
  0xc4,0x14,0x84,0x82, 0x24,0x1c,0xf8,0x25, 0xd1,0x72,0xf0,0x55, 0x5e,0xd4,0x44,0xaf,
  0xf4,0x1b,0x19,0x5f, 0x79,0x61,0x6b,0x75, 0x6b,0xaf,0xd6,0x4a, 0x29,0x71,0x6c,0xe0,
  0x7f,0x8b,0xb9,0x22, 0x9c,0x28,0x48,0x2f, 0x99,0xeb,0xbd,0x80,
  // 0x1ec = 492 Bytes
};

static BZ2Manager_t lecode_loader_jap_mgr
	= { lecode_loader_jap_bz2, sizeof(lecode_loader_jap_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static const u8 lecode_loader_kor_bz2[] =
{
  // decompressed len: 0x200 = 512 bytes
  0x00,0x00,0x02,0x00,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0xaa,0xbf, 0x22,0x0b,0x00,0x00,
  0x16,0x7f,0xff,0xfc, 0x76,0x45,0x30,0x60, 0xc7,0xc4,0x6f,0x2f, 0x6f,0x94,0x84,0x7f,
  0xe7,0xfe,0x6e,0xdc, 0x04,0x44,0x0c,0x43, 0x64,0x00,0x15,0x50, 0x00,0x48,0x49,0x6a,
  0x40,0x00,0x01,0xb0, 0x01,0x59,0xb2,0x36, 0x22,0x0a,0x7a,0x35, 0x43,0x1a,0x4d,0xa2,
  0x64,0x62,0x06,0x03, 0x28,0x61,0x1a,0x60, 0x06,0x90,0xd3,0x02, 0x18,0xd3,0x42,0x32,
  0x62,0x69,0xa3,0xf5, 0x08,0xa7,0xa0,0x98, 0x9a,0x68,0x29,0xe9, 0x92,0x00,0x68,0x00,
  0x00,0xc8,0x06,0x83, 0x40,0x01,0xa1,0xe8, 0x68,0x9e,0xa0,0x06, 0x80,0x4a,0x9a,0x90,
  0x9f,0xa9,0xa8,0xf4, 0x8c,0x80,0x60,0x8d, 0x0d,0x00,0x01,0x84, 0x69,0xa6,0x9a,0x06,
  0x9a,0x06,0x9a,0x34, 0xd3,0xd4,0x33,0x51, 0xbd,0x49,0xa6,0x86, 0x5a,0x6e,0xd6,0x68,
  0xd4,0x3f,0xde,0x1f, 0x36,0x20,0xfe,0xf1, 0x5e,0xbd,0x98,0x13, 0xde,0x69,0xdb,0x03,
  0x55,0x10,0xa2,0x8a, 0x75,0x36,0xc3,0x0d, 0xab,0xc8,0xb7,0x7f, 0x09,0x79,0xa4,0x03,
  0xae,0xda,0x49,0xd7, 0x10,0x7a,0x52,0xbb, 0x5c,0x51,0x05,0xb1, 0xd3,0xda,0x68,0x22,
  0xb7,0x95,0xa5,0x61, 0x6a,0x20,0x82,0x23, 0x3d,0xdf,0x1e,0x7e, 0x16,0x45,0x59,0x39,
  0xab,0xb8,0x26,0xe3, 0x36,0x29,0x4c,0x91, 0x55,0xbb,0x17,0xa4, 0x96,0x51,0x75,0x6e,
  0x57,0xad,0x4d,0xa0, 0x9d,0xfb,0xfb,0x98, 0xd3,0x14,0x28,0x44, 0x0b,0x43,0x1d,0x99,
  0xed,0xfa,0x3b,0xbe, 0xec,0x94,0x8e,0x98, 0x60,0xc6,0x0b,0x2d, 0x85,0xc8,0x5b,0xa9,
  0x85,0x80,0x7e,0xd8, 0x56,0xb4,0x35,0xf0, 0xcf,0x7b,0xa3,0xa8, 0xb7,0x01,0x00,0x2e,
  0x98,0x05,0x85,0x9f, 0x06,0xe7,0x4c,0x18, 0x1a,0x96,0x9e,0x74, 0x2e,0xbe,0x3d,0x07,
  0x88,0xd2,0x84,0xa4, 0x12,0xe6,0x16,0x80, 0xd3,0x06,0x21,0x08, 0x65,0x12,0xce,0x64,
  0x24,0x09,0x2d,0xc4, 0x12,0xd6,0x55,0x53, 0xa8,0x32,0x90,0x83, 0x48,0xa4,0x41,0xa9,
  0x2d,0x76,0x84,0x18, 0x0f,0xee,0x03,0x96, 0xc4,0x0e,0x55,0xdc, 0xa2,0x8a,0x8d,0xc7,
  0xe5,0x04,0x4a,0x22, 0xd4,0x80,0x89,0xb9, 0x15,0x4e,0xc1,0x01, 0x88,0x3c,0x69,0x39,
  0xa9,0x10,0x23,0x60, 0xa4,0xa0,0x29,0x29, 0x58,0xa9,0xd8,0x60, 0x23,0x90,0xb0,0xad,
  0x99,0xfd,0x45,0xad, 0xa1,0x32,0x92,0x45, 0xba,0xd8,0xc3,0x1a, 0x02,0x45,0x1c,0x60,
  0x29,0x0c,0x0e,0xbc, 0xdd,0x91,0x52,0x17, 0x43,0x4d,0xe6,0x94, 0x2e,0xa4,0x64,0xf3,
  0x70,0xe5,0xfa,0xa9, 0x74,0x21,0x10,0x91, 0x18,0x37,0x78,0x33, 0x6d,0x4d,0x2e,0x8c,
  0xf0,0x17,0xe4,0x2b, 0x1f,0x16,0xf8,0x63, 0x30,0xa2,0x8a,0x3d, 0x5c,0x07,0xf0,0xb9,
  0x98,0x71,0x39,0x37, 0x1f,0x45,0xb8,0x4c, 0x67,0x8b,0x78,0x89, 0xf3,0x50,0x41,0x48,
  0x48,0x3f,0x8c,0x55, 0xc8,0x59,0xfc,0x19, 0x9c,0x87,0x9a,0x27, 0xcc,0x16,0xc0,0x38,
  0x43,0x46,0xd8,0x60, 0x84,0x37,0x42,0x2f, 0x89,0xe2,0x7c,0x28, 0x33,0xdd,0xe2,0xee,
  0x48,0xa7,0x0a,0x12, 0x15,0x57,0xe4,0x41, 0x60,
  // 0x1e9 = 489 Bytes
};

static BZ2Manager_t lecode_loader_kor_mgr
	= { lecode_loader_kor_bz2, sizeof(lecode_loader_kor_bz2), 0, 0 };

//-----------------------------------------------------------------------------

static BZ2Manager_t * GetLecodeLoader ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:  return &lecode_loader_pal_mgr;
	case STR_M_USA:	 return &lecode_loader_usa_mgr;
	case STR_M_JAP:	 return &lecode_loader_jap_mgr;
	case STR_M_KOR:  return &lecode_loader_kor_mgr;
	default:	 return 0;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			more data			///////////////
///////////////////////////////////////////////////////////////////////////////

static const u8 codehandleronly_bz2[] =
{
  // 4 bytes decompressed len
  0x00,0x00,0x0a,0xb8,

  // bzipped data, includes a GCH header of sizeof(gch_header_t) == 16 bytes
  // 0x0:
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x88,0x5d, 0x08,0x9e,0x00,0x02,
  0x4f,0xff,0xff,0xff, 0xfd,0xe5,0xf5,0xff, 0xff,0xff,0xff,0xff, 0xfd,0xef,0xf6,0x7e,
  0xfd,0xfc,0xf7,0xff, 0xff,0xff,0xf5,0x7b, 0xff,0xfe,0x7d,0x4e, 0x75,0x75,0x45,0x46,
  0xe4,0xe4,0xff,0xd0, 0x05,0x3e,0xdd,0x80, 0x1d,0xd5,0xb5,0xd7, 0x1d,0x57,0x6e,0xa3,
  0x6c,0x11,0x4c,0x22, 0x64,0x9e,0xa9,0xfa, 0xa7,0xe9,0x95,0x3d, 0xa8,0x8f,0x53,0xda,
  0x93,0x41,0xb4,0x9e, 0x48,0xd1,0xfa,0xa0, 0x06,0x9e,0x46,0xa6, 0x9a,0x34,0x01,0xa3,
  0xd4,0x03,0x34,0x47, 0x84,0x86,0x8d,0x3d, 0x41,0xea,0x34,0xf4, 0x83,0xd4,0xd1,0xa3,
  0xf5,0x40,0x7a,0x80, 0x00,0x00,0x3d,0x4c, 0x86,0x80,0x00,0x68, 0x7a,0x8f,0x49,0xe5,
  0x04,0x53,0xd2,0x0d, 0x20,0x35,0x30,0x9a, 0x9e,0xa1,0xea,0x79, 0xa9,0x3d,0xaa,0x7a,
  0x9e,0x9e,0x82,0x65, 0x34,0x7a,0x81,0xe4, 0x46,0xd3,0x51,0xea, 0x6d,0x4d,0x0d,0xa9,
  0xea,0x1e,0x9a,0x4d, 0x34,0x01,0xea,0x68, 0xf5,0x1a,0x00,0x0c, 0x8d,0x03,0x41,0xa0,
  0xd1,0xea,0x06,0x80, 0x00,0x00,0x00,0x0d, 0x34,0x00,0x1a,0x15, 0x4f,0x4d,0x4c,0xa7,
  0xa9,0xa7,0x91,0xa6, 0xa1,0x1b,0x4c,0x9a, 0x00,0x00,0x00,0x08, 0xc0,0x02,0x60,0x98,
  0x00,0x03,0x40,0x46, 0x00,0x09,0x82,0x60, 0x00,0x26,0x00,0x00, 0x4c,0x46,0x00,0x02,
  0x60,0x26,0x00,0x1a, 0x08,0x00,0xd3,0x40, 0xc8,0x34,0x00,0x34, 0x0d,0x00,0xd0,0x06,
  0x43,0x43,0x4c,0x40, 0x0d,0x03,0x13,0x40, 0xd0,0x19,0x00,0x00, 0x00,0xc8,0xd1,0x88,
  // 0x100:
  0x0d,0x00,0x00,0x00, 0x06,0x86,0x46,0x8c, 0x98,0x08,0xc9,0x93, 0x4d,0x03,0x4d,0x21,
  0x04,0xd3,0x49,0x95, 0x3d,0xa6,0x46,0x40, 0x53,0x4f,0x26,0xa7, 0x8d,0x29,0xb4,0xca,
  0x1a,0x79,0xa5,0x01, 0xe9,0x34,0x7a,0x83, 0x69,0xa9,0x82,0x68, 0x06,0x81,0x90,0x00,
  0x66,0x51,0x91,0xea, 0x7a,0x46,0x8c,0x8d, 0x36,0x8c,0xa7,0xea, 0x4c,0xd4,0xc9,0x91,
  0xa0,0x0d,0x01,0xfa, 0xa0,0x00,0x19,0x1a, 0x3c,0xa6,0x77,0x37, 0x61,0x49,0xb5,0xfd,
  0x12,0xb5,0x55,0x35, 0x28,0x01,0x28,0x12, 0xd4,0xf8,0x84,0x4a, 0xca,0xdc,0x30,0x0d,
  0x2a,0x2f,0x0e,0x06, 0xc4,0xbe,0x15,0xb5, 0x0c,0xca,0x0e,0x1b, 0x4f,0x31,0x16,0xd9,
  0xf1,0xbe,0x3b,0xdb, 0xd4,0xa2,0x22,0xda, 0x45,0x51,0xf9,0x21, 0xe0,0x31,0x35,0x0b,
  0x66,0x00,0xc0,0xb4, 0x89,0x10,0xc2,0x4d, 0x66,0x0f,0x5f,0x4d, 0x7e,0xeb,0xc4,0x29,
  0xdc,0xc1,0x45,0xbb, 0x74,0x39,0xaf,0xc5, 0xe1,0xa4,0x41,0x10, 0x7c,0x26,0x16,0x41,
  0xee,0x61,0xb3,0x88, 0x06,0x60,0x4e,0x20, 0x79,0x42,0x4a,0x51, 0xc5,0xe1,0x76,0x23,
  0xab,0xd1,0x6a,0x73, 0xd6,0x2b,0x05,0x12, 0x59,0xe4,0xa2,0x17, 0x6b,0x86,0x01,0x22,
  0x61,0x1a,0xe1,0x8b, 0x53,0x78,0x18,0xfa, 0xb6,0x82,0xa9,0x00, 0x82,0x27,0x2f,0x26,
  0xa2,0x2c,0x60,0xc9, 0x7a,0xba,0xa8,0x51, 0x20,0x66,0x87,0x8b, 0x06,0x7b,0xf7,0x8e,
  0x68,0x32,0xe2,0x5e, 0x26,0x73,0xbd,0x96, 0xc4,0x21,0x56,0xf1, 0x64,0x28,0x79,0xce,
  0x7a,0x47,0xc2,0xb9, 0xaf,0xa7,0xc6,0xf4, 0x25,0x99,0x40,0xb9, 0x8a,0x60,0x55,0x1c,
  // 0x200:
  0x5b,0x75,0x74,0x40, 0xc3,0x0c,0xc7,0x35, 0x1c,0xa4,0xa9,0x07, 0x0f,0x3d,0x82,0xb0,
  0x89,0x26,0x89,0x09, 0x3e,0xeb,0xc7,0x51, 0x2f,0xec,0x75,0xee, 0x3b,0xa3,0xd3,0xbb,
  0xed,0x38,0x62,0xaa, 0xaf,0xf4,0xa3,0xac, 0x23,0xf3,0x28,0x91, 0xc2,0xa1,0x1e,0x22,
  0xd9,0x9a,0xf6,0x26, 0xac,0xce,0xae,0x73, 0x20,0x30,0x58,0x2a, 0x34,0xd2,0x20,0x50,
  0x7d,0xb6,0xf6,0xd4, 0xde,0x4f,0x28,0x43, 0x16,0x90,0x5c,0xc6, 0x3b,0x86,0xc2,0x36,
  0xd0,0xf4,0x98,0xf7, 0x4c,0x02,0x2f,0x73, 0x6f,0x32,0x60,0x24, 0x83,0xbd,0x2b,0xae,
  0x19,0x10,0x24,0xa2, 0x88,0x8e,0x20,0x5d, 0x52,0xc1,0x10,0xb5, 0x00,0x78,0x8d,0x8d,
  0x1b,0x99,0xba,0xa9, 0x59,0xd0,0xd6,0x08, 0x70,0x08,0xf9,0x32, 0xd5,0x04,0x5f,0xc4,
  0x89,0x3f,0xcc,0xa9, 0x4f,0x81,0xf3,0x5f, 0xe1,0x8c,0xc6,0xe5, 0x90,0x13,0xde,0x9b,
  0x65,0x67,0xb7,0xdb, 0x10,0x41,0x28,0x49, 0x42,0x2c,0x67,0x88, 0x00,0xff,0x22,0xe4,
  0x46,0xde,0x17,0x0d, 0xc1,0x82,0xe2,0xc2, 0xa3,0xd7,0x82,0x21, 0xbe,0x1c,0x88,0x05,
  0x88,0xd4,0x28,0x46, 0x68,0x28,0x40,0x9c, 0x1a,0xdb,0x70,0x44, 0x21,0xaa,0x01,0x07,
  0xdd,0x6b,0x6b,0x5c, 0x1f,0x23,0xae,0x20, 0x0d,0x02,0x81,0xa4, 0x38,0xee,0xb2,0x19,
  0x3a,0xfd,0xcf,0x94, 0x03,0x4c,0x42,0x12, 0x63,0x7d,0x9b,0x81, 0xae,0x3a,0xfc,0xce,
  0x39,0x73,0xca,0xae, 0xd3,0xee,0xd5,0xf7, 0x15,0x71,0xc6,0x5c, 0xf1,0xfb,0x15,0x4b,
  0xcd,0xd9,0x5f,0x36, 0xdb,0x65,0x5d,0xce, 0x4e,0xb2,0x7d,0x94, 0x39,0x8c,0x17,0x25,
  // 0x300:
  0x10,0x3d,0x68,0x74, 0x9a,0xc0,0xe6,0x39, 0x06,0x49,0xd9,0xdf, 0x2e,0x47,0x81,0xad,
  0xcb,0x6b,0x8f,0x1d, 0xfd,0xff,0x7e,0x75, 0x61,0x6c,0xac,0x75, 0x41,0x0b,0x83,0x38,
  0xe2,0x90,0x43,0x01, 0xb8,0x82,0x68,0x96, 0xc1,0x4a,0x46,0x01, 0xa0,0x58,0x67,0x85,
  0xbc,0x82,0xc5,0x84, 0x90,0xc8,0x38,0x12, 0xc5,0x4e,0x05,0xb5, 0xd0,0x59,0x3e,0x08,
  0xdd,0x52,0x00,0x2d, 0x65,0x17,0x03,0x35, 0xa2,0x58,0xf8,0x0a, 0x5a,0x75,0x86,0x2e,
  0x34,0x15,0x10,0xf2, 0x3c,0x30,0xfa,0x80, 0x1e,0x51,0xb9,0x1d, 0x3c,0x93,0xc2,0x77,
  0x99,0x49,0x15,0xc4, 0xe1,0x86,0x9b,0x9d, 0x4a,0xc9,0xdf,0x15, 0x66,0xd9,0xcd,0x64,
  0xd2,0x1a,0x14,0x62, 0x6c,0x85,0x7f,0x99, 0x87,0x77,0x9b,0xe3, 0xca,0xc5,0xa3,0xfd,
  0x67,0xa4,0x83,0xb6, 0x0e,0xe8,0xc3,0x5c, 0x75,0x63,0x5b,0xf7, 0x35,0x82,0x6e,0x13,
  0xac,0xaa,0x29,0x91, 0xbb,0x10,0xbd,0xd4, 0x43,0xa8,0xa2,0xde, 0x20,0x26,0xd3,0x38,
  0xb4,0x77,0x24,0x67, 0x45,0xab,0xba,0xde, 0x7e,0x29,0xbc,0x68, 0x1f,0xe6,0xeb,0xc5,
  0xd1,0x08,0x26,0x42, 0xbb,0x56,0xba,0xc0, 0x0f,0x3a,0x81,0xfb, 0x19,0xb8,0x2f,0x2e,
  0x54,0x33,0x68,0xdd, 0xbc,0x26,0x94,0xff, 0xaf,0xf3,0xe5,0x51, 0xac,0x79,0xf0,0x40,
  0xc0,0xe5,0x2b,0x30, 0xec,0x79,0xe1,0xc6, 0x36,0xf5,0x28,0xd3, 0xe6,0x7c,0x1e,0x7c,
  0xf7,0xc1,0x07,0x96, 0x34,0x85,0x7a,0xd2, 0x3d,0xbd,0x8b,0xdb, 0xe1,0x1e,0x82,0xce,
  0xff,0xd2,0x14,0x39, 0x29,0x23,0x80,0x02, 0x12,0xe0,0x34,0x04, 0x15,0x12,0xe8,0xe9,
  // 0x400:
  0x98,0x86,0x3f,0x68, 0xb4,0x51,0x0f,0x74, 0x09,0xbf,0xa0,0x0c, 0x49,0x32,0xcb,0x86,
  0xfa,0xfb,0x9f,0xd3, 0x1d,0xe4,0xac,0x27, 0xa6,0xde,0xe2,0xd7, 0x46,0xd9,0xb3,0x97,
  0x0b,0x1f,0x09,0xb5, 0x1f,0x4a,0xfb,0xe1, 0x31,0x82,0x16,0xac, 0x61,0xf6,0x15,0xa0,
  0xe0,0xe3,0xa0,0x1c, 0x50,0xe1,0x01,0x7f, 0x10,0x9d,0x1e,0x35, 0x13,0x67,0x81,0x48,
  0x8d,0x24,0xf0,0x02, 0x79,0x02,0xba,0xa2, 0x30,0x8e,0x7d,0xe8, 0x7a,0x34,0xe6,0x8a,
  0x0c,0xd1,0x8c,0x85, 0x03,0x13,0x4f,0x0c, 0x98,0x60,0x40,0x45, 0x48,0x9d,0x3a,0x47,
  0x2a,0x95,0x6c,0xa9, 0xf6,0x19,0x2d,0x99, 0x3e,0xc3,0x83,0xd6, 0xd6,0x51,0x63,0xe5,
  0x96,0x22,0x2b,0xa2, 0xa3,0x41,0x61,0x35, 0x4b,0xc9,0x2b,0x06, 0xbc,0x55,0xc4,0xc9,
  0x23,0x6e,0x50,0xa7, 0xba,0x3f,0xcf,0x5b, 0xf3,0x10,0x5a,0xfc, 0x38,0x81,0xdd,0x6a,
  0xca,0x2c,0x66,0x69, 0x60,0xbc,0xa4,0x04, 0xcd,0x7e,0x6a,0xab, 0x1d,0xae,0x01,0x08,
  0xd5,0x25,0x2c,0xa1, 0xaa,0x58,0x45,0x39, 0x34,0xc6,0x86,0x62, 0xa4,0x7e,0x49,0xab,
  0x65,0x25,0xf4,0x76, 0xb3,0xa7,0xaf,0x99, 0x1e,0x7c,0x93,0x42, 0x24,0xa4,0x20,0x08,
  0x39,0x5c,0x36,0x82, 0x76,0x3e,0x22,0x9e, 0xe3,0xe6,0xe1,0x7a, 0x8b,0xac,0x1d,0xca,
  0x98,0x73,0x6b,0x3b, 0xfa,0x0a,0xbf,0x80, 0xdb,0xd4,0x21,0x45, 0xed,0x7b,0xca,0xb6,
  0x28,0xaa,0x20,0xe4, 0xe7,0x0e,0xab,0x0a, 0x94,0x1c,0x23,0x84, 0x32,0x6a,0x26,0x76,
  0x5e,0xe1,0x8b,0xca, 0x4b,0x66,0x85,0x37, 0x2d,0x89,0x96,0x48, 0x50,0xa1,0xc2,0x16,
  // 0x500:
  0x84,0x1f,0xf4,0xef, 0x70,0x94,0x3b,0x6e, 0xd5,0x49,0x6e,0x97, 0x04,0xa6,0xff,0x2c,
  0x2c,0x69,0x0e,0x92, 0x65,0xdb,0x5b,0xeb, 0xf5,0x5a,0x55,0xd2, 0x88,0xf0,0x78,0x49,
  0x57,0x4b,0x1d,0x5a, 0x68,0x4a,0x0b,0x48, 0x24,0x7a,0x26,0x19, 0x6b,0x22,0xfc,0x9a,
  0xe6,0xb2,0xd0,0x03, 0x36,0x77,0x04,0xb1, 0x1a,0x57,0x24,0xb9, 0xd9,0x6f,0x2c,0x65,
  0xb4,0x64,0x50,0xbc, 0xbc,0x5f,0x0d,0x6d, 0xd1,0xc1,0x30,0x0d, 0xd6,0xbc,0xa5,0xcf,
  0x47,0xac,0xce,0xa4, 0xb6,0x6d,0xa5,0x73, 0xdd,0xd5,0x49,0x7f, 0xb3,0xe8,0x76,0x88,
  0xa3,0x9c,0x2b,0x1a, 0xf7,0x13,0xdc,0xfd, 0x3d,0x89,0x55,0x04, 0xfe,0xc1,0xaf,0x1f,
  0x0c,0x2b,0xc4,0x1b, 0x72,0x0d,0x11,0x77, 0x0e,0x7b,0x40,0xea, 0x6c,0x57,0xc2,0x9f,
  0x15,0x35,0x11,0x70, 0x0a,0x20,0xa1,0x73, 0x31,0x4d,0x43,0x02, 0x72,0xeb,0x58,0xdf,
  0xd2,0x9a,0x16,0x82, 0xc0,0xb6,0x47,0xa5, 0x07,0xf7,0xa5,0xcf, 0xa2,0xcd,0x8a,0x24,
  0xc5,0x9c,0x1c,0x71, 0xab,0x10,0x3e,0x83, 0x68,0x84,0x58,0xcf, 0x6e,0x26,0x17,0x2f,
  0x54,0x44,0x52,0x88, 0x06,0x31,0x24,0xaa, 0x03,0x4c,0x4d,0x30, 0x85,0xd4,0x95,0xc1,
  0x5c,0x46,0x9d,0xba, 0x20,0xf8,0x24,0x42, 0x57,0xe5,0x08,0xff, 0x37,0xe2,0x3e,0x0a,
  0xa4,0x26,0xfb,0x9b, 0x5a,0xca,0x94,0x5a, 0x47,0x0a,0xac,0x3d, 0xc4,0x35,0x11,0xc5,
  0x9c,0x63,0x7e,0x4b, 0xac,0x74,0xc3,0x0d, 0x13,0xc5,0x36,0x30, 0x88,0x37,0x64,0x4e,
  0x0b,0xb1,0x01,0x03, 0xde,0xc9,0x5d,0x44, 0xae,0xa6,0x5f,0xe4, 0xbc,0x9b,0x3a,0x5a,
  // 0x600:
  0x5b,0x4a,0xd9,0x64, 0x14,0x4d,0x44,0x01, 0xeb,0xfb,0x89,0xf6, 0x0f,0xc0,0x42,0x14,
  0x05,0x02,0x12,0x10, 0x91,0x4c,0x8d,0x85, 0xcd,0xd5,0xc7,0xa6, 0xb7,0x69,0x83,0xa7,
  0x30,0x2c,0xdd,0xa8, 0x39,0x71,0xa3,0xc6, 0x29,0x86,0x43,0xc6, 0x05,0x83,0x57,0xf3,
  0x86,0x77,0x7a,0x2c, 0x50,0xb9,0xc0,0xad, 0x26,0xec,0x10,0x40, 0x0c,0x0c,0x04,0xc4,
  0xcb,0xb2,0xeb,0xb6, 0x1b,0x9b,0x50,0x16, 0x62,0x42,0x57,0x56, 0x35,0xd1,0x49,0x0b,
  0x14,0x16,0xb6,0x48, 0x40,0xe6,0xd4,0xe8, 0x51,0x91,0x89,0x38, 0xa5,0x2e,0x71,0x7a,
  0x60,0xd8,0xc5,0x7f, 0x3b,0xcb,0xbd,0x88, 0x38,0x10,0x08,0xbb, 0xd6,0x31,0x83,0x05,
  0xfd,0x6e,0x8d,0x63, 0xb6,0x25,0x04,0xf0, 0x4b,0x09,0x0b,0x89, 0x67,0x5e,0x18,0x91,
  0x63,0x4e,0x14,0x20, 0x0d,0x09,0x7a,0xf2, 0x32,0xfe,0xd5,0x6b, 0x5e,0xc1,0x73,0xc8,
  0xa6,0xc5,0xed,0xd4, 0x32,0xc4,0x9d,0xce, 0xfc,0x17,0x94,0x15, 0x45,0x46,0x5d,0x8a,
  0x7b,0x20,0xf6,0xce, 0x67,0x03,0xc9,0x80, 0x45,0x3d,0x33,0x6d, 0x21,0x4b,0x37,0xf3,
  0xf3,0xce,0x43,0x3a, 0xa5,0x4e,0x59,0x66, 0xa4,0x52,0x29,0x17, 0x4a,0x31,0x9e,0x86,
  0x02,0x7a,0x65,0x36, 0x43,0x81,0x04,0x0b, 0xf7,0x92,0x32,0xff, 0x1e,0xa6,0x1c,0x01,
  0x41,0x00,0x36,0xbc, 0xfd,0x20,0xd5,0x0d, 0x64,0x69,0x67,0x6c, 0x4c,0x9f,0x3d,0xc1,
  0xec,0x1c,0xd2,0x22, 0xe1,0x49,0x86,0xaa, 0xca,0xa6,0xcd,0xe1, 0xb3,0x31,0x46,0xb9,
  0x47,0xab,0x91,0x20, 0x6a,0x21,0xc6,0x1e, 0xa9,0x53,0x6a,0xe2, 0x41,0xa2,0xc9,0x44,
  // 0x700:
  0x9c,0x9f,0x2d,0x06, 0xc1,0x73,0xc3,0xb8, 0xc4,0xb5,0x26,0x41, 0x97,0x06,0xa6,0xcb,
  0x1c,0x0b,0x48,0x29, 0xfe,0xf7,0x90,0x70, 0xa2,0x8b,0x01,0xb1, 0xe3,0x34,0x55,0x6a,
  0x6d,0x01,0x02,0x5f, 0x6a,0xa2,0x1e,0x95, 0x52,0xa9,0xce,0x2d, 0xfb,0xd5,0x86,0xc0,
  0xd2,0x05,0x03,0x8a, 0x34,0xfc,0x45,0x48, 0x20,0x80,0x4d,0x3e, 0x27,0x03,0xa9,0xf5,
  0x5d,0xd4,0x15,0x2f, 0x90,0x8f,0x8e,0x30, 0x6b,0x9a,0x39,0x3d, 0x05,0xcc,0x0d,0x48,
  0xd5,0x65,0x09,0x7e, 0x1b,0x03,0xb7,0xbd, 0x1a,0x03,0xa2,0xaa, 0x88,0x34,0x56,0xee,
  0xaa,0xb5,0x00,0xda, 0x91,0x79,0xf3,0xea, 0x8e,0xd5,0xce,0x0a, 0x50,0x7f,0xe2,0xee,
  0x48,0xa7,0x0a,0x12, 0x11,0x0b,0xa1,0x13, 0xc0,
  // 0x779 = 1913 Bytes
};

static BZ2Manager_t codehandleronly_mgr
	= { codehandleronly_bz2, sizeof(codehandleronly_bz2), 0, 0 };

///////////////////////////////////////////////////////////////////////////////

static const u8 codehandler_bz2[] =
{
  // 4 bytes decompressed len
  0x00,0x00,0x10,0xc8,

  // bzipped data, includes a GCH header of sizeof(gch_header_t) == 16 bytes
  // 0x0:
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x1e,0xd9, 0x97,0x25,0x00,0x03,
  0xe9,0xff,0xff,0xff, 0xfd,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xfd,0xff,0xff,0x7e,
  0xfd,0xff,0xff,0xff, 0xff,0xff,0xf7,0x7f, 0xff,0xfe,0x7f,0x4e, 0x77,0x75,0xe5,0x66,
  0xe6,0xf4,0xff,0xe0, 0x08,0x9f,0x5c,0xbe, 0xf8,0x7b,0x4e,0xb4, 0xe9,0xdd,0xb1,0xdf,
  0x6b,0x6f,0xb6,0xed, 0xb7,0xcd,0xe9,0xec, 0x7b,0xef,0x5a,0x1d, 0x8f,0x70,0x35,0x32,
  0x21,0x1a,0x27,0xa4, 0xd4,0xf2,0x66,0xa6, 0x46,0x54,0xf4,0x9f, 0x88,0x93,0xfd,0x52,
  0x69,0xfa,0x9a,0x6a, 0x1e,0xa7,0xb5,0x35, 0x07,0xa9,0xe9,0x1a, 0x33,0xd5,0x34,0xf6,
  0xa6,0x89,0xb1,0x21, 0xa6,0xd4,0xf4,0x8f, 0x14,0x3d,0x43,0xf5, 0x1a,0x64,0xd3,0x51,
  0xfa,0x51,0xe9,0x1e, 0xa0,0xf5,0x1e,0x48, 0x68,0x32,0x68,0x7a, 0x11,0xb4,0x83,0x47,
  0xa8,0x3d,0x46,0x81, 0xe9,0x00,0x06,0x86, 0x8f,0x48,0x7e,0xa9, 0xa6,0x41,0x13,0x50,
  0x84,0xfc,0x91,0xa9, 0xb5,0x18,0x99,0x4f, 0x49,0xa0,0xf6,0xa1, 0xe8,0x8f,0x50,0x8f,
  0x4d,0x36,0xa8,0x34, 0xd3,0xd2,0x62,0x0d, 0x1b,0x51,0xea,0x68, 0x06,0x9b,0x53,0xd4,
  0x0d,0x00,0x0d,0x34, 0x0d,0x00,0x06,0x80, 0x1a,0x34,0x00,0x34, 0xd0,0x1a,0x00,0x00,
  0x00,0x00,0x00,0x34, 0x00,0x04,0x44,0xd4, 0xc4,0xc4,0xc8,0xf4, 0x8d,0x1a,0x53,0xf4,
  0x26,0x9a,0x9b,0x50, 0xf2,0x9e,0xa1,0xb4, 0xd4,0x06,0x8f,0x50, 0x07,0xa8,0x3d,0x40,
  0x06,0x80,0x1a,0x68, 0xf2,0x8c,0x80,0x01, 0xa0,0x1e,0xa0,0x0c, 0x80,0x00,0x34,0x1a,
  // 0x100:
  0x07,0xa8,0x00,0x00, 0x00,0x06,0x80,0x00, 0x00,0x69,0xa1,0x14, 0xf2,0x35,0x32,0x20,
  0x26,0x86,0x9a,0x26, 0x13,0x4d,0x34,0x34, 0x03,0x4c,0x8d,0xa9, 0xa1,0x90,0x1a,0x7a,
  0x9a,0x03,0x43,0x40, 0x34,0x06,0x80,0xd0, 0x03,0x40,0x00,0x01, 0xa0,0x00,0x0d,0x00,
  0x64,0x34,0x69,0xa0, 0x69,0xa0,0x00,0x00, 0x00,0x0d,0x00,0x03, 0x53,0xd2,0x41,0x26,
  0x9e,0x52,0x3d,0x13, 0xd4,0x69,0xe5,0x1e, 0xa0,0x3d,0x11,0xa1, 0xea,0x3d,0x20,0x32,
  0x00,0x00,0xf4,0x86, 0x80,0x00,0x00,0x00, 0x0d,0x0c,0x83,0x20, 0x00,0x00,0x01,0xa0,
  0x1a,0x0d,0x00,0x06, 0x80,0x00,0x00,0x00, 0x00,0x00,0x25,0x35, 0x08,0x26,0xa3,0x4c,
  0x24,0x6d,0x4c,0x8f, 0x54,0xc9,0xb4,0x62, 0x9b,0x42,0x6f,0x2a, 0x60,0x4f,0x29,0x93,
  0x6a,0x7a,0x8d,0x1a, 0x7a,0x46,0x9a,0x07, 0xa8,0x34,0xd0,0x64, 0xd0,0x34,0x7a,0x8d,
  0x03,0x43,0x43,0x20, 0x06,0x99,0x00,0x34, 0x03,0x43,0x40,0x0d, 0x1a,0x69,0xa0,0x00,
  0x34,0x00,0x34,0x03, 0x46,0x87,0xa9,0xb4, 0xde,0xf4,0x75,0xe1, 0x8b,0xbc,0x72,0xbd,
  0x73,0x68,0x58,0x04, 0x99,0x8b,0x84,0x6b, 0x11,0xac,0xe9,0x36, 0x54,0xef,0xb6,0x1d,
  0xac,0x8c,0x79,0x0a, 0xbb,0x3f,0xcd,0xf1, 0xac,0xb1,0xd1,0xd3, 0xb6,0x54,0x4c,0x17,
  0x24,0xca,0xac,0x92, 0xf6,0xf8,0x49,0x67, 0x6d,0x9b,0x8c,0x8f, 0x66,0x2c,0x5a,0x6b,
  0xcd,0xf7,0x23,0x77, 0x97,0x67,0x7c,0xf2, 0x4c,0xad,0x5c,0x67, 0x42,0xe4,0x18,0xd6,
  0x12,0x38,0x2f,0x61, 0xef,0x30,0x61,0x6d, 0xdb,0x51,0x04,0x4c, 0x58,0xf9,0x8b,0x04,
  // 0x200:
  0x4a,0x75,0x24,0xbd, 0x41,0x04,0x88,0x35, 0x87,0x5e,0x18,0x80, 0x83,0x13,0x81,0x03,
  0xe0,0xe3,0xdb,0x63, 0xdd,0xe9,0xac,0xa5, 0xee,0x0e,0x36,0xda, 0xe5,0xe2,0x04,0x60,
  0xcf,0x1e,0x9c,0x51, 0xd0,0xab,0xb4,0x5d, 0xc9,0x04,0x87,0x18, 0xca,0x34,0x22,0xa4,
  0xe0,0x6e,0x9b,0xa2, 0xd5,0x0f,0x59,0x22, 0x90,0x5c,0xf6,0x61, 0xcd,0x18,0x9c,0xb2,
  0xb0,0x0c,0x02,0x74, 0x6e,0xd2,0xb2,0x05, 0x8e,0xba,0x3b,0x32, 0x83,0x14,0x7d,0x14,
  0xa9,0x87,0x19,0x66, 0xf2,0x3e,0x27,0x97, 0xa2,0xab,0x72,0x00, 0x2e,0xbe,0x5f,0x56,
  0x04,0x20,0x05,0x2b, 0x70,0x12,0x21,0x28, 0xcd,0x5c,0xa5,0x4d, 0xad,0x98,0xf9,0x67,
  0x8f,0x16,0xbd,0xdb, 0xca,0x16,0x38,0xe9, 0x4b,0x82,0xa4,0x6d, 0xc8,0x0b,0x6e,0x1a,
  0x21,0x00,0x82,0x20, 0xa5,0xaa,0x8c,0x04, 0x72,0x40,0xd9,0x96, 0xf9,0xe3,0xe8,0x49,
  0xe2,0x80,0xf4,0xa2, 0x01,0xe0,0x3c,0xa6, 0x06,0xc5,0x21,0x52, 0xe1,0x2b,0x08,0xa2,
  0x39,0x77,0xad,0x23, 0x9c,0x28,0x38,0xb7, 0x64,0x74,0xec,0xd4, 0x24,0xde,0x03,0x4e,
  0x92,0xcd,0x53,0x6e, 0x9a,0x38,0xc6,0x67, 0x84,0x12,0xb3,0x6d, 0x79,0x2b,0xaf,0x5d,
  0x26,0xf9,0xa6,0x1b, 0xec,0xa4,0x9d,0x08, 0x02,0xd0,0x64,0x08, 0x20,0x66,0x4c,0xee,
  0xe0,0x29,0x17,0x22, 0x42,0xd8,0xdf,0xc6, 0xaa,0x54,0x96,0xd2, 0x90,0x4f,0x40,0x5d,
  0xbb,0x2c,0xa4,0xb4, 0x6b,0xd5,0x98,0x03, 0x42,0xc0,0x0b,0x5a, 0x60,0x4b,0xf3,0x4a,
  0x1c,0xa0,0x88,0x58, 0x02,0x82,0x35,0x28, 0x08,0xf3,0x02,0x28, 0xb2,0x06,0xc6,0x24,
  // 0x300:
  0x53,0x28,0x49,0x10, 0x62,0x4a,0x67,0x54, 0x41,0x58,0x93,0x84, 0x53,0x73,0xec,0xa1,
  0xe4,0xe3,0x94,0x79, 0x46,0x16,0x18,0x7c, 0x48,0x31,0x45,0x69, 0x30,0x73,0x00,0x4c,
  0x1f,0x7a,0x68,0xf0, 0x1d,0x6a,0x80,0xd4, 0xde,0x60,0x62,0x0a, 0x2a,0x26,0x28,0x7d,
  0x4e,0xa0,0x88,0x58, 0x56,0xce,0xcb,0xa5, 0x8d,0xb4,0xeb,0x48, 0xb1,0xf8,0xe9,0xd2,
  0x06,0x3d,0x34,0xf6, 0x80,0x5b,0x88,0x49, 0x05,0xd4,0x43,0x2a, 0x8a,0x01,0x65,0xa2,
  0xa7,0x7f,0x29,0x28, 0x42,0xa5,0x1c,0x71, 0xeb,0xc9,0x96,0xa2, 0xbf,0x48,0xa0,0xa8,
  0x34,0x65,0x89,0x68, 0x66,0xb2,0xf1,0xc6, 0xa4,0x7b,0xd6,0x56, 0xf2,0xcd,0xb2,0xd3,
  0x53,0x9d,0x64,0xdd, 0xb4,0x26,0x17,0x68, 0x68,0xb7,0x64,0xb1, 0x5e,0xa6,0x29,0x2d,
  0xcf,0x70,0xd5,0x2a, 0x09,0x21,0x2f,0x25, 0xce,0x22,0xaa,0x4e, 0x75,0x3d,0x83,0x99,
  0xa9,0xd3,0xe1,0x9e, 0xc0,0x65,0x51,0x32, 0x66,0xc9,0x2e,0x0a, 0xfd,0xac,0x9e,0xcc,
  0xd3,0x90,0x56,0x3a, 0x2d,0x53,0xa5,0x23, 0x65,0x5e,0xf9,0xcd, 0x0f,0x03,0xb7,0xbd,
  0xe0,0x44,0x6b,0x17, 0x86,0x5a,0x01,0xc4, 0x50,0xa9,0x05,0x06, 0x6e,0xfc,0x1a,0x47,
  0x53,0xe7,0x98,0x42, 0x49,0xf0,0x87,0x6f, 0x0c,0x36,0x45,0x4e, 0x59,0xf5,0x2d,0x63,
  0x93,0xf7,0xb4,0xb6, 0xb1,0xc4,0xe3,0x6f, 0xcc,0xec,0x06,0x3e, 0x3d,0x0e,0x09,0x9a,
  0x83,0x78,0x48,0xba, 0x45,0xf2,0xcc,0xa9, 0xc9,0x86,0x3d,0xbb, 0x7a,0xcf,0xaf,0x55,
  0xcd,0xb2,0x74,0x8a, 0x02,0x09,0x51,0x25, 0x49,0x69,0x58,0x61, 0x4b,0x88,0x90,0xc2,
  // 0x400:
  0xd1,0x44,0xc7,0x24, 0xab,0xf0,0xc2,0x0f, 0xd4,0x95,0x23,0xf6, 0x55,0x57,0xe6,0x32,
  0x16,0xa9,0x87,0xf7, 0x1a,0x59,0x32,0xc5, 0xf5,0x23,0x68,0x6e, 0x26,0x0b,0xa3,0x46,
  0xe6,0x0d,0x7c,0x30, 0xde,0x00,0x79,0xd6, 0xd9,0x81,0x13,0x16, 0xfa,0x08,0xe7,0xa8,
  0x82,0xd5,0xce,0xdc, 0xf2,0x95,0x66,0x53, 0x62,0xc9,0x18,0x9e, 0x86,0xb1,0x13,0x06,
  0x1a,0x02,0x08,0x89, 0x6c,0xef,0xb6,0x30, 0x9f,0xbe,0x3b,0x43, 0x20,0xd3,0x90,0xd7,
  0xe9,0x84,0x2c,0x30, 0xe0,0x4c,0x58,0x42, 0x80,0x91,0x69,0x72, 0x90,0x6c,0xf7,0x41,
  0x45,0x73,0x07,0xe9, 0x4c,0x1a,0x63,0x14, 0x82,0x88,0x8e,0x22, 0x94,0x94,0x62,0x50,
  0x59,0x2c,0x71,0x48, 0x6d,0x8d,0xda,0x7b, 0x19,0xcf,0x11,0x4b, 0x46,0x05,0x7d,0x96,
  0x4b,0x0a,0xad,0xa5, 0xad,0xc4,0xaf,0xd2, 0xd5,0xde,0x40,0xe0, 0x10,0xc5,0xaa,0x66,
  0x25,0x45,0xcb,0xb2, 0x9a,0x72,0x3d,0xe4, 0x2e,0xf5,0xd5,0xa7, 0x33,0x65,0x7b,0xb7,
  0x1d,0x52,0xa4,0x36, 0x51,0x0c,0xce,0xe2, 0xaf,0x75,0x0f,0xd8, 0xd8,0x5f,0xec,0xe1,
  0x1a,0xdc,0x2b,0x0e, 0xf3,0xad,0x16,0x0b, 0xab,0x1a,0xda,0xb0, 0xc4,0xda,0x62,0xc2,
  0x4b,0x22,0x62,0x08, 0x7a,0x47,0x92,0x94, 0x23,0x71,0x18,0xed, 0x09,0xc2,0x40,0x1c,
  0xb0,0x46,0x55,0x54, 0xf0,0xb8,0x5c,0x51, 0x02,0xed,0x97,0x4c, 0x60,0xbd,0x19,0x1b,
  0xb5,0x61,0x5a,0x40, 0xb2,0xb6,0x32,0xb5, 0xf6,0xcd,0x9c,0x30, 0xc8,0xd1,0xe0,0xb3,
  0x73,0x84,0x02,0xa6, 0xe1,0x5d,0x83,0x74, 0x88,0x04,0x93,0xa6, 0x98,0x63,0x19,0xca,
  // 0x500:
  0x88,0x61,0xc7,0x0d, 0x8c,0x8c,0x01,0xe7, 0xcc,0x12,0xd8,0x4c, 0x56,0x9e,0xce,0xff,
  0x38,0x1d,0xe8,0x67, 0x0b,0x37,0xf8,0x10, 0x6d,0xeb,0xcd,0xff, 0x57,0x5a,0x6b,0x95,
  0x32,0x0c,0x8d,0xbe, 0xeb,0x2e,0xd9,0xe2, 0xf8,0xa7,0xc1,0xb8, 0x77,0xa7,0x15,0xa5,
  0x9d,0xc2,0xc2,0xfb, 0xec,0x69,0x15,0xdb, 0x0e,0xc4,0xf2,0x86, 0xc7,0x0a,0x0b,0xf5,
  0xf9,0x0e,0xe2,0xcf, 0xdc,0xf9,0x2f,0xd6, 0x5e,0x2e,0x53,0x0a, 0x83,0x95,0x86,0xd6,
  0x2d,0x95,0xb6,0x37, 0xcd,0xc7,0xf6,0xa2, 0x7c,0x7d,0x24,0x4d, 0x2b,0xfe,0x5d,0xf7,
  0x32,0x0e,0x9c,0x11, 0x02,0x90,0x71,0xc7, 0x74,0xfa,0x6d,0x94, 0x5d,0xe3,0xb5,0x6a,
  0xeb,0xad,0xda,0xfb, 0x52,0x73,0xc7,0x18, 0xc6,0x99,0x7b,0x86, 0x66,0x69,0x2d,0xed,
  0x6b,0x12,0xc2,0xf0, 0x81,0x0c,0x7b,0xf7, 0x49,0x15,0x31,0x8f, 0xd5,0x20,0x2a,0x33,
  0x37,0x36,0x76,0x37, 0x31,0x6d,0x03,0x1a, 0xc1,0xd3,0x1e,0x82, 0xf5,0xe9,0x6e,0x8e,
  0x9b,0xbd,0x21,0x48, 0xc4,0x0a,0xbb,0x8c, 0xc4,0x89,0x91,0x35, 0xdd,0x3f,0x36,0x92,
  0x30,0x46,0x48,0x5c, 0x04,0x9a,0xde,0xf0, 0x15,0x34,0xa7,0xcc, 0xeb,0xcc,0x13,0xfe,
  0x26,0x2e,0xd5,0x74, 0xf2,0xcd,0xd1,0x03, 0x32,0x93,0xac,0xeb, 0x15,0xb2,0x79,0x0c,
  0x2a,0x08,0x0d,0x34, 0x0d,0x59,0x59,0x9c, 0x68,0xe0,0xaa,0x36, 0x95,0xa3,0x9d,0x55,
  0x20,0x8f,0x12,0x3e, 0x10,0xe9,0x04,0xfc, 0x5a,0x0b,0x53,0xa9, 0x27,0x54,0x85,0x44,
  0x35,0x47,0x07,0xc1, 0xd9,0xe2,0x07,0x84, 0x24,0x7c,0x28,0x20, 0xc8,0x4e,0x98,0x7e,
  // 0x600:
  0x33,0x4a,0x0e,0xc6, 0x54,0xef,0xeb,0xe7, 0xf3,0x6a,0x98,0x04, 0x0b,0x82,0x7d,0xcb,
  0x47,0x60,0x3e,0xb7, 0x78,0xed,0xc6,0x9c, 0xc6,0x04,0x5d,0x50, 0x79,0x60,0xd1,0xd4,
  0x58,0x57,0x42,0x53, 0x23,0x9e,0x3e,0xd8, 0x10,0xa8,0x07,0x28, 0x53,0xa6,0x84,0xf1,
  0x0f,0x74,0x10,0xef, 0x2a,0xfd,0x3c,0x71, 0xce,0x06,0x2e,0x82, 0x39,0x64,0x13,0x3a,
  0x46,0x0b,0x30,0xa6, 0x68,0xd8,0x58,0x05, 0x92,0xf3,0x4d,0xc7, 0x20,0xd6,0xd3,0xe7,
  0x7b,0x5c,0x00,0x73, 0x17,0x3f,0x01,0x0f, 0xcd,0xfc,0x5f,0xf9, 0x10,0xa8,0x21,0x1d,
  0x3b,0x4c,0x8b,0x0d, 0x4c,0x04,0x42,0x01, 0x29,0x27,0x60,0xa4, 0x09,0x47,0x98,0x4c,
  0xdf,0x62,0x90,0x0b, 0xd1,0xad,0x18,0x19, 0x67,0xba,0x51,0xea, 0x2c,0x63,0xbe,0x94,
  0xb5,0xdd,0xac,0x3e, 0x16,0xc3,0x0a,0x43, 0x79,0xce,0xf9,0x45, 0xef,0xff,0xd9,0xed,
  0xf1,0x3f,0xd9,0x3a, 0x42,0x82,0xdd,0xfc, 0xba,0x7a,0x2d,0x48, 0x96,0xdb,0xb5,0x3c,
  0x0a,0x4b,0xc8,0x4b, 0x3d,0x94,0x4a,0x01, 0x9c,0xd9,0xe9,0xdb, 0x2a,0xd4,0xc9,0x2d,
  0xb9,0x8c,0xc0,0xb5, 0x5c,0xba,0x60,0xb8, 0x90,0x92,0x3d,0xc9, 0x69,0x86,0x66,0x48,
  0x06,0x1d,0x48,0x47, 0x30,0xa7,0xa6,0x43, 0x62,0xb0,0xf2,0xaa, 0x30,0xe3,0x9f,0xa0,
  0xad,0x14,0x30,0xe1, 0x1e,0x85,0x34,0x57, 0x23,0x91,0x51,0x93, 0x9b,0x33,0x64,0x87,
  0x03,0xfd,0x42,0x45, 0x9f,0x9b,0x55,0xf5, 0x1d,0x08,0x57,0x0e, 0xdc,0x20,0x88,0x4d,
  0xc9,0xf5,0x49,0xac, 0x36,0x24,0x0c,0x39, 0xc5,0x77,0xb3,0x5a, 0x6f,0x30,0xc7,0x50,
  // 0x700:
  0x59,0x25,0x0d,0x92, 0x91,0x6c,0x29,0x82, 0x3f,0x65,0xee,0xb7, 0xa4,0x37,0xc9,0x1a,
  0x7f,0x58,0xd6,0xfd, 0x2e,0xa8,0x90,0x64, 0x74,0xa1,0x66,0x0f, 0xbb,0xe8,0x28,0xa5,
  0x45,0x4c,0x8a,0x4f, 0xb5,0x87,0x05,0xc4, 0xfa,0x0c,0xf2,0x0e, 0xcd,0x01,0x98,0x08,
  0x45,0x87,0xb2,0x2a, 0xa5,0x0c,0xb4,0x74, 0xd9,0xe1,0x5f,0x59, 0x68,0xf6,0x6e,0xe6,
  0x9b,0x25,0xd6,0x4b, 0xd6,0x2a,0xc6,0x5e, 0x75,0x4c,0xb0,0xfc, 0x5b,0x04,0x10,0x07,
  0xde,0xc7,0x35,0x99, 0x3a,0x27,0x45,0x20, 0x54,0x11,0x5a,0x7d, 0xb3,0x33,0x51,0xa9,
  0xc8,0x52,0x5c,0x4c, 0x25,0xe6,0x17,0x96, 0x5c,0x43,0xe8,0xd3, 0x60,0x95,0x92,0x66,
  0xdd,0x29,0x62,0x92, 0xfa,0xaa,0xd8,0x58, 0xeb,0x4f,0x8f,0x74, 0xf7,0x50,0xe5,0xee,
  0xd9,0x55,0x18,0x94, 0x72,0x2b,0x7a,0x1e, 0x8a,0x17,0x24,0x85, 0x07,0x36,0x51,0xb5,
  0x2d,0x3a,0xa9,0x21, 0x54,0xaa,0xb7,0x94, 0xee,0xe1,0x95,0x35, 0x23,0x3c,0x94,0x16,
  0x45,0x88,0xf5,0xe2, 0x11,0xb7,0x5c,0x86, 0x97,0xf9,0x15,0x9b, 0xd8,0x39,0x31,0xf5,
  0x0f,0xd7,0x90,0xab, 0x66,0xc2,0x0c,0x15, 0xeb,0x86,0x30,0x43, 0xea,0x63,0x77,0xc6,
  0x93,0xb6,0x56,0x80, 0x17,0x12,0x2d,0xc6, 0xd0,0xd0,0xe2,0xc3, 0x42,0x68,0xcc,0x8b,
  0x60,0x20,0x35,0x95, 0x50,0x62,0x92,0x7e, 0x81,0xd9,0x68,0xf4, 0xad,0xbe,0xc3,0x38,
  0x63,0x1f,0x1a,0xfe, 0xff,0xc8,0x02,0x3e, 0x34,0x52,0x0c,0x99, 0x28,0xa8,0xe1,0x61,
  0x63,0x97,0xc6,0x90, 0x5c,0xcb,0x79,0x9b, 0x84,0x4e,0x9b,0xd7, 0x5f,0x40,0x4a,0xa1,
  // 0x800:
  0x0c,0xe0,0x62,0x34, 0xa6,0x6a,0xd0,0x88, 0x04,0xd2,0x84,0x10, 0xca,0xfe,0x17,0xef,
  0x3d,0x2b,0xf7,0x13, 0x9d,0xe4,0x57,0xac, 0x84,0x78,0xb6,0xae, 0xbf,0xb4,0xf9,0x4f,
  0x83,0xea,0xb2,0xd1, 0x56,0x75,0x8e,0xd1, 0x72,0xc2,0x1e,0xa2, 0xc0,0x91,0xd9,0x6f,
  0xcf,0x68,0x51,0xa5, 0x9f,0x87,0x78,0x37, 0x32,0xe7,0x29,0x58, 0xa3,0x6b,0x7c,0x56,
  0x03,0xc4,0xc2,0x85, 0x12,0xdd,0x10,0x52, 0x39,0xa0,0x98,0x12, 0xa1,0xa7,0xd5,0xf1,
  0x90,0x92,0x1d,0x24, 0x3e,0x16,0x3d,0x56, 0x28,0x90,0x9d,0xbf, 0x5f,0x55,0x07,0x18,
  0x80,0xda,0x9c,0xd2, 0x43,0xab,0xfc,0x4a, 0x8c,0x3d,0xba,0xcf, 0xdf,0xdb,0x28,0x2b,
  0x3d,0x83,0xd4,0xcf, 0xd0,0xf3,0x64,0x71, 0x71,0x98,0xd6,0x8f, 0x13,0x35,0x6a,0xfc,
  0xed,0x7a,0x41,0xed, 0x0c,0x62,0xab,0x46, 0x9c,0xc7,0x29,0x53, 0xc6,0x67,0x5e,0xf8,
  0x69,0x79,0x3c,0xe0, 0xc2,0xec,0x0a,0x52, 0x51,0x80,0x8e,0x7c, 0x9d,0x11,0x02,0x1f,
  0x91,0x6b,0x9c,0x96, 0xb4,0xa1,0x2d,0x79, 0x6c,0x5d,0x6e,0x12, 0x97,0x97,0x0e,0xb2,
  0xfb,0xba,0x4a,0x14, 0x6d,0xe0,0xe6,0x30, 0xb2,0xa0,0x19,0xc8, 0x86,0xb5,0x18,0x19,
  0xb3,0x80,0xb0,0x50, 0x40,0x44,0x4b,0x93, 0xa3,0x70,0x4a,0x9e, 0xdb,0xa9,0xd0,0x38,
  0x17,0x10,0xf4,0x66, 0xbb,0xa5,0x26,0x8a, 0xf2,0xb8,0xe7,0x8b, 0x2b,0xc2,0x41,0xf0,
  0xcb,0x80,0x30,0x0e, 0xa5,0x90,0x42,0x28, 0x65,0x13,0xb2,0xa9, 0x01,0xd1,0xd9,0x34,
  0x00,0xc2,0xc1,0x92, 0xd5,0x14,0xdb,0x94, 0xaf,0xd9,0x4b,0x28, 0x54,0x11,0xf4,0xe6,
  // 0x900:
  0xbe,0xb9,0xac,0x06, 0x67,0xb3,0xc5,0xcc, 0x54,0xf9,0xa9,0x2c, 0x13,0x46,0xde,0x12,
  0xe6,0xc9,0x8c,0xa1, 0xeb,0xb6,0x0d,0x4b, 0xe7,0xa8,0xb1,0x4f, 0x31,0x0c,0x34,0xd3,
  0x6a,0xd3,0x73,0x49, 0x3c,0xbb,0xc3,0x32, 0xb2,0x9d,0x29,0x3e, 0x70,0x96,0x56,0xd4,
  0xb9,0x2a,0xc8,0x83, 0x47,0x51,0x67,0xe0, 0x7e,0x84,0x02,0x68, 0xc9,0x7d,0xbc,0xc1,
  0x93,0x3a,0x27,0x0d, 0x8d,0x0a,0x03,0x9c, 0xb0,0xdc,0x75,0xf1, 0xbb,0x34,0x59,0x8e,
  0x2d,0x9a,0xa7,0xf2, 0xff,0xa3,0x9f,0x3d, 0xa2,0x41,0xc4,0x74, 0xc9,0x03,0x69,0x5a,
  0xad,0x3b,0x53,0x49, 0xf8,0x80,0xb2,0x3f, 0x21,0x2f,0xa5,0x69, 0xf3,0x56,0x49,0xa6,
  0xab,0xcb,0x58,0x80, 0x80,0x01,0x9c,0xcc, 0x8c,0xc2,0x40,0xce, 0x86,0x75,0x57,0xa4,
  0x31,0x11,0x43,0x21, 0x93,0x09,0x32,0x4c, 0x21,0x0c,0xc2,0x43, 0x55,0xb0,0x70,0x61,
  0xfe,0x3a,0x49,0x6c, 0x6a,0xcd,0x34,0xa0, 0x90,0x25,0x84,0xcb, 0x9a,0x3a,0x43,0x96,
  0x82,0xe7,0x66,0x20, 0x4e,0xd3,0x34,0x73, 0x78,0xfd,0xe6,0x27, 0x8e,0xb2,0x02,0xc4,
  0x28,0x4c,0x1b,0x37, 0x9a,0xdb,0x13,0x5f, 0xd4,0xce,0xa1,0x40, 0x3c,0xa5,0x21,0x0c,
  0xb7,0xbc,0x31,0x34, 0xcc,0x47,0x7a,0xe6, 0x30,0xa8,0xed,0x6c, 0xf0,0x25,0xd7,0xf7,
  0xbc,0xe9,0xae,0xa9, 0xd7,0x11,0x36,0x43, 0x30,0x19,0x12,0x83, 0x6e,0x2a,0xd9,0x90,
  0x22,0x8e,0x8d,0x21, 0xd4,0x22,0xb9,0xc8, 0x30,0x80,0x75,0x7a, 0x74,0x44,0x3c,0x6a,
  0xa7,0x9a,0x83,0xd8, 0xf8,0xb5,0x45,0x8e, 0x20,0x42,0x60,0x45, 0xc5,0x7a,0x29,0x48,
  // 0xa00:
  0x97,0xe1,0x04,0x6e, 0xee,0x01,0x54,0x80, 0xc1,0x60,0xfd,0x13, 0xda,0x4e,0x6b,0xe1,
  0x5b,0x64,0x2d,0x89, 0xe2,0x04,0x4a,0x66, 0x96,0x73,0x4c,0xa9, 0x5e,0xf0,0x94,0x2f,
  0x66,0x99,0xb1,0xb4, 0xfa,0x0c,0x3a,0xf6, 0x51,0xe1,0x5b,0xb8, 0x69,0xc8,0xb9,0x16,
  0xd7,0x3f,0x26,0x6c, 0xb3,0x8d,0xc0,0xf7, 0x22,0x49,0x23,0x7d, 0xa5,0x42,0x01,0x16,
  0xa6,0xc0,0xbb,0x90, 0x32,0x62,0x88,0x6b, 0x8b,0xe2,0x5c,0x14, 0x52,0x8d,0xb4,0x05,
  0x77,0x0e,0xc6,0x3a, 0x75,0xdd,0xac,0x52, 0xb3,0xd5,0x2c,0x9c, 0x8e,0xc5,0x66,0x74,
  0xd4,0x28,0x54,0x2c, 0x71,0x14,0x6e,0x9b, 0x62,0x39,0x27,0x30, 0x58,0xbd,0x93,0xfc,
  0xc7,0xb2,0x70,0x40, 0x82,0x20,0xdf,0x82, 0x61,0xcc,0xe8,0xf3, 0xe6,0x8b,0x56,0x49,
  0x2d,0x16,0xa3,0x21, 0x02,0x7d,0x26,0xab, 0xf0,0xa4,0x72,0x57, 0x51,0xf6,0xfc,0x2e,
  0xa1,0x3e,0x86,0xaa, 0xa5,0xc8,0x40,0xc5, 0x0d,0xbb,0x57,0x31, 0x18,0x35,0xa5,0x46,
  0x56,0x0d,0x6b,0xa8, 0x91,0x1d,0xab,0x58, 0xee,0xc4,0x55,0x6a, 0x6d,0x9a,0x2f,0xa1,
  0xb0,0xcd,0x86,0xc0, 0xc6,0x8c,0xeb,0x86, 0x0a,0xcc,0x10,0x7f, 0x59,0xb1,0x32,0x38,
  0x86,0x22,0xc6,0x4a, 0x82,0xa9,0xe9,0x98, 0xcb,0xff,0x5c,0x90, 0x12,0x10,0x0a,0x60,
  0x9e,0x0b,0x1f,0xe7, 0x72,0x29,0x9c,0x5c, 0xa5,0xae,0xb0,0xa8, 0xe4,0x34,0x92,0xcb,
  0x01,0x0b,0x28,0xc9, 0x72,0xd6,0xbe,0x0b, 0x7c,0xcb,0x52,0x38, 0x0a,0x7a,0x58,0x15,
  0xd9,0x96,0x36,0x3e, 0x64,0xcc,0x80,0x6a, 0x27,0x0e,0x50,0x53, 0xa1,0xa1,0x40,0x32,
  // 0xb00:
  0x79,0xfe,0x0e,0xc2, 0x0a,0x11,0x1d,0xcf, 0x60,0x8a,0x41,0x1a, 0x2c,0xec,0xc8,0x36,
  0xa4,0xad,0xab,0x10, 0x7c,0x04,0x6f,0x64, 0x67,0x01,0x8b,0xb3, 0xdd,0xb2,0xaf,0x16,
  0x04,0x6c,0x56,0xe5, 0x0b,0xc3,0x12,0x10, 0xec,0x0c,0xab,0x19, 0x70,0x09,0x73,0x52,
  0xba,0x81,0x2d,0x24, 0x88,0x8c,0x33,0x25, 0x3e,0xab,0x56,0x20, 0x12,0x0e,0xd9,0x08,
  0x5e,0x63,0x00,0xc4, 0x00,0xb0,0xf0,0x62, 0x72,0xa4,0xc3,0xa8, 0x52,0x90,0xc9,0x63,
  0xd0,0x35,0x1c,0x7a, 0x54,0x2d,0x0f,0x81, 0x06,0x32,0x23,0x6b, 0xe3,0x07,0xca,0xd5,
  0xec,0xac,0xad,0xad, 0x52,0xc2,0x8a,0x6b, 0x24,0xc5,0x8a,0xdd, 0x27,0x21,0x08,0x1e,
  0xe7,0x60,0x6d,0xfd, 0x2e,0x37,0x11,0x16, 0x8d,0x39,0x1b,0x6a, 0xf6,0x93,0x41,0x82,
  0x63,0x72,0x49,0x2f, 0xf5,0x13,0x6d,0xd1, 0x9e,0x59,0xb8,0x4e, 0x78,0xd4,0x50,0xe5,
  0x66,0x63,0x03,0xa1, 0x8a,0xac,0x43,0xa0, 0x91,0xd6,0x2b,0xb6, 0x1d,0x86,0xf7,0xbd,
  0x98,0xc3,0x73,0xaf, 0x21,0xf6,0xc2,0x0c, 0x1d,0x7a,0x0b,0xce, 0xeb,0x05,0x4b,0xae,
  0x51,0xc6,0x33,0xd0, 0xc7,0xfe,0x2e,0xe4, 0x8a,0x70,0xa1,0x20, 0x3d,0xb3,0x2e,0x4a,
  // 0xbc0 = 3008 Bytes
};

static BZ2Manager_t codehandler_mgr
	= { codehandler_bz2, sizeof(codehandler_bz2), 0, 0 };

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct staticr_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeSTR ( staticr_t * str )
{
    memset(str,0,sizeof(*str));
    memset(str->dol_flags,'-',sizeof(str->dol_flags)-1);
    str->fname = EmptyString;
}

///////////////////////////////////////////////////////////////////////////////

void ResetSTR ( staticr_t * str )
{
    if (str)
    {
	if (str->data_alloced)
	    FREE(str->data);
	FreeString(str->fname);

	InitializeSTR(str);
    }
}

///////////////////////////////////////////////////////////////////////////////

static bool CheckSizeSTR ( staticr_t * str )
{
    DASSERT(str);

    switch (str->data_size)
    {
	case 4903876:
	    str->mode = STR_M_PAL;
	    break;

	case 4902804:
	    str->mode = STR_M_USA;
	    break;

	case 4901564:
	    str->mode = STR_M_JAP;
	    break;

	case 4905468:
	    str->mode = STR_M_KOR;
	    break;

	default:
	    str->mode = STR_M_UNKNOWN;
	    str->str_status = STR_S_ANALYZED | STR_S_WRONG_SIZE;
	    return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadSTR
(
    staticr_t		* str,		// valid pointer
    bool		initialize,	// true: initialize 'bmg'
    ccp			fname,		// filename of source
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(str);
    DASSERT(fname);
    TRACE("LoadSTR(%d,%d) fbname=%s\n",initialize,ignore_no_file,fname);

    if (initialize)
	InitializeSTR(str);
    else
	ResetSTR(str);

    str->data_alloced = true;
    enumError err = OpenReadFILE( fname, 0, ignore_no_file,
			&str->data, &str->data_size, &str->fname, &str->fatt );
    if (err)
	return err;

// [[analyse-magic]]
    str->fform = GetByMagicFF(str->data,str->data_size,str->data_size);
    switch(str->fform)
    {
	case FF_STATICR:
	    if (!CheckSizeSTR(str))
		break;
	    str->is_staticr = true;
	    return ERR_OK;

	case FF_DOL:
	    str->is_dol = true;
	    return ERR_OK;

	default:
	    break;
    }

    if (!ignore_no_file)
	ERROR0(ERR_WARNING,"Neither StaticR nor DOL file: %s\n",fname);
    return ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

static bool ReplaceRegionChar ( staticr_t *str, char *buf, uint buf_size, ccp fname )
{
    DASSERT(str);
    DASSERT(buf);
    DASSERT(buf_size>100);

    bool stat = false;
    const char reg_char = GetStrModeRegionChar(str->mode);

    if (reg_char)
    {
	StringCopyS(buf,buf_size,fname);
	char *p = strrchr(buf,'/');
	if (!p)
	    p = buf;

	for ( ; *p; p++ )
	    if ( *p == '@' )
	    {
		*p = reg_char;
		stat = true;
	    }
    }
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			dol_sect_info_t			///////////////
///////////////////////////////////////////////////////////////////////////////

static const dol_sect_info_t dol_info_pal[DOL_NN_SECTIONS] =
{
  {  0, "T0",     0x100, 0x80004000,   0x2460, 1,0,1, 0,
		{ 0x67,0xa0,0xfd,0x48,0x8f, 0x9b,0xe2,0xb8,0xa1,0x35,
		  0xed,0x61,0x48,0xe7,0x41, 0x0b,0xb6,0x5b,0xdf,0x2f }},
  {  1, "T1",    0x2560, 0x800072c0, 0x23db20, 1,0,1, 0,
		{ 0x1c,0xbe,0x24,0xaf,0x98, 0x6a,0x6e,0x3e,0xbe,0xa1,
		  0x5c,0xc9,0x66,0x49,0xce, 0xc7,0x7d,0x73,0x75,0x7b }},
  {  2, "T2",         0,          0,        0, 0,0,0, 0, {0} },
  {  3, "T3",         0,          0,        0, 0,0,0, 0, {0} },
  {  4, "T4",         0,          0,        0, 0,0,0, 0, {0} },
  {  5, "T5",         0,          0,        0, 0,0,0, 0, {0} },
  {  6, "T6",         0,          0,        0, 0,0,0, 0, {0} },
  {  7, "D0",  0x240080, 0x80006460,    0x5c0, 1,0,1, 0,
		{ 0xc8,0xbf,0xf1,0x5d,0x1d, 0x5a,0x05,0x77,0xbc,0xec,
		  0x42,0x05,0xe7,0x11,0x76, 0x64,0xa1,0x87,0xfd,0x3d }},
  {  8, "D1",  0x240640, 0x80006a20,    0x8a0, 1,0,1, 0,
		{ 0x6c,0xe1,0x67,0x91,0x92, 0x92,0x44,0x05,0x12,0x12,
		  0x64,0xff,0x57,0x7f,0xae, 0x81,0xe5,0xa8,0xf8,0x7a }},
  {  9, "D2",  0x240ee0, 0x80244de0,     0xc0, 1,0,1, 0,
		{ 0xf6,0x7f,0x2b,0x81,0x56, 0x65,0x9d,0xac,0xd2,0x56,
		  0xb0,0xcd,0x6c,0xca,0xfd, 0x72,0x39,0xe5,0x3d,0x2e }},
  { 10, "D3",  0x240fa0, 0x80244ea0,     0x20, 1,0,1, 0,
		{ 0xeb,0x28,0xbc,0x20,0x23, 0x0a,0xce,0xa4,0xc9,0xe0,
		  0x6d,0x67,0xee,0x25,0x20, 0x1c,0xa1,0x55,0x39,0x7d }},
  { 11, "D4",  0x240fc0, 0x80244ec0,  0x136c0, 1,0,1, 0,
		{ 0x4f,0xd6,0xcf,0x0f,0x65, 0x76,0xc1,0x23,0x96,0x86,
		  0xc8,0x37,0x1b,0x95,0x93, 0x9f,0xba,0x72,0xf6,0xc5 }},
  { 12, "D5",  0x254680, 0x80258580,  0x4bac0, 1,0,1, 0,
		{ 0x4a,0x9a,0x76,0x06,0x69, 0x35,0x0b,0xd2,0x54,0xd2,
		  0xbf,0x22,0xac,0xd9,0xf9, 0xc8,0x13,0xcf,0x95,0xd8 }},
  { 13, "D6",  0x2a0140, 0x80384c00,   0x13c0, 1,0,1, 0,
		{ 0xaa,0x9a,0x25,0x46,0xb8, 0xd9,0x57,0x5e,0x22,0x6d,
		  0x72,0x2f,0x71,0xff,0x86, 0x49,0xc6,0x12,0x2d,0xcf }},
  { 14, "D7",  0x2a1500, 0x80386fa0,   0x21a0, 1,0,1, 0,
		{ 0x43,0x0d,0x6f,0xd3,0xe0, 0x12,0x72,0x2f,0x44,0x5b,
		  0x9e,0xdc,0x92,0x1b,0xe0, 0x62,0x8e,0x72,0xe8,0x07 }},
  { 15, "D8",         0,          0,        0, 0,0,0, 0, {0} },
  { 16, "D9",         0,          0,        0, 0,0,0, 0, {0} },
  { 17, "D10",        0,          0,        0, 0,0,0, 0, {0} },
  { 18, "BSS",        0, 0x802a4080,  0xe50fc, 1,0,0, 0, {0} },
  { 19, "ENT",        0, 0x800060a4,        0, 1,0,0, 0, {0} },
};

///////////////////////////////////////////////////////////////////////////////

static const dol_sect_info_t dol_info_usa[DOL_NN_SECTIONS] =
{
  {  0, "T0",     0x100, 0x80004000,   0x2460, 1,0,1, 0,
		{ 0x5e,0x16,0x94,0x9f,0x50, 0x5f,0xc6,0xc3,0x6e,0xca,
		  0x76,0x5b,0x8a,0x07,0x6e, 0x23,0x68,0x77,0x84,0x67 }},
  {  1, "T1",    0x2560, 0x800072c0, 0x23da80, 1,0,1, 0,
		{ 0x3a,0x6d,0x28,0xc3,0xdd, 0xc8,0x10,0xcc,0xf0,0xb2,
		  0x7f,0xe7,0xd8,0x7d,0x12, 0x8f,0x62,0x72,0xd0,0xed }},
  {  2, "T2",         0,          0,        0, 0,0,0, 0, {0} },
  {  3, "T3",         0,          0,        0, 0,0,0, 0, {0} },
  {  4, "T4",         0,          0,        0, 0,0,0, 0, {0} },
  {  5, "T5",         0,          0,        0, 0,0,0, 0, {0} },
  {  6, "T6",         0,          0,        0, 0,0,0, 0, {0} },
  {  7, "D0",  0x23ffe0, 0x80006460,    0x5c0, 1,0,1, 0,
		{ 0x47,0x6d,0x32,0x6c,0x21, 0xaa,0x65,0xf2,0x9e,0x69,
		  0xf9,0xde,0xd2,0xaa,0xb6, 0xc2,0x76,0x94,0x19,0x09 }},
  {  8, "D1",  0x2405a0, 0x80006a20,    0x8a0, 1,0,1, 0,
		{ 0x73,0x5b,0x90,0x85,0x58, 0x13,0x3f,0x1e,0x56,0x67,
		  0xeb,0x61,0x3a,0xa9,0xe9, 0x13,0xc4,0xd3,0xd5,0x6f }},
  {  9, "D2",  0x240e40, 0x80244d40,     0xc0, 1,0,1, 0,
		{ 0x4a,0x87,0x49,0xbd,0xa1, 0x78,0x9f,0x8c,0x9c,0x9e,
		  0x25,0x6a,0x6b,0xda,0x19, 0x7f,0x7f,0xc7,0xf5,0x7f }},
  { 10, "D3",  0x240f00, 0x80244e00,     0x20, 1,0,1, 0,
		{ 0xe1,0x05,0x64,0x2d,0x86, 0x59,0xc9,0xac,0x76,0xde,
		  0x8c,0x84,0xd0,0x67,0xc6, 0x9a,0x37,0x2f,0xd5,0xab }},
  { 11, "D4",  0x240f20, 0x80244e40,  0x13420, 1,0,1, 0,
		{ 0xbd,0x62,0x9a,0x5e,0x1b, 0xda,0xf5,0x46,0xe8,0x04,
		  0xcb,0xa1,0xfa,0xca,0xac, 0x38,0xac,0x7a,0xe6,0xaf }},
  { 12, "D5",  0x254340, 0x80258260,  0x47aa0, 1,0,1, 0,
		{ 0x9b,0x08,0x11,0xf2,0xb5, 0xed,0x17,0x69,0x7e,0x4b,
		  0xb3,0xef,0xb4,0x65,0x23, 0x78,0x21,0x24,0x27,0xc1 }},
  { 13, "D6",  0x29bde0, 0x80380880,   0x13c0, 1,0,1, 0,
		{ 0x98,0x55,0xd3,0xa2,0x15, 0x10,0xe6,0xd4,0x78,0xa8,
		  0x40,0x51,0xe5,0x51,0x77, 0x04,0x10,0x18,0x29,0x63 }},
  { 14, "D7",  0x29d1a0, 0x80382c20,   0x21a0, 1,0,1, 0,
		{ 0x9b,0x2c,0x27,0x2b,0x6b, 0x43,0xff,0xce,0x11,0x05,
		  0x92,0x29,0x42,0x26,0x6f, 0x9c,0x59,0xd6,0x55,0xe8 }},
  { 15, "D8",         0,          0,        0, 0,0,0, 0, {0} },
  { 16, "D9",         0,          0,        0, 0,0,0, 0, {0} },
  { 17, "D10",        0,          0,        0, 0,0,0, 0, {0} },
  { 18, "BSS",        0, 0x8029fd00,  0xe50fc, 1,0,0, 0, {0} },
  { 19, "ENT",        0, 0x800060a4,        0, 1,0,0, 0, {0} },
};

///////////////////////////////////////////////////////////////////////////////

static const dol_sect_info_t dol_info_jap[DOL_NN_SECTIONS] =
{
  {  0, "T0",     0x100, 0x80004000,   0x2460, 1,0,1, 0,
		{ 0xa6,0x86,0x2e,0xbc,0x13, 0xdd,0x9e,0x7e,0xfa,0xa9,
		  0x0d,0x49,0x19,0x5f,0x9b, 0xe6,0x30,0x19,0x01,0x4e }},
  {  1, "T1",    0x2560, 0x800072c0, 0x23da40, 1,0,1, 0,
		{ 0x93,0xb4,0x6b,0xbe,0xab, 0xf5,0x7d,0x98,0x93,0xf2,
		  0x16,0x07,0x4f,0xc0,0x44, 0x82,0x01,0xd1,0x42,0x40 }},
  {  2, "T2",         0,          0,        0, 0,0,0, 0, {0} },
  {  3, "T3",         0,          0,        0, 0,0,0, 0, {0} },
  {  4, "T4",         0,          0,        0, 0,0,0, 0, {0} },
  {  5, "T5",         0,          0,        0, 0,0,0, 0, {0} },
  {  6, "T6",         0,          0,        0, 0,0,0, 0, {0} },
  {  7, "D0",  0x23ffa0, 0x80006460,    0x5c0, 1,0,1, 0,
		{ 0x7f,0xcd,0xe8,0xc8,0x06, 0xfb,0x10,0xbc,0x81,0x9c,
		  0xb8,0xbb,0xa0,0x72,0x6a, 0xd8,0x75,0xb2,0xb7,0x6a }},
  {  8, "D1",  0x240560, 0x80006a20,    0x8a0, 1,0,1, 0,
		{ 0x5a,0x2c,0x70,0x39,0x11, 0x58,0xa0,0x71,0xc0,0xb9,
		  0x09,0xac,0x3e,0xde,0x55, 0x64,0xb0,0x70,0xb7,0x52 }},
  {  9, "D2",  0x240e00, 0x80244d00,     0xc0, 1,0,1, 0,
		{ 0xb4,0x63,0xae,0x32,0x03, 0x5d,0xcc,0xa7,0xb0,0x64,
		  0xee,0x4a,0x1c,0x24,0xd3, 0x10,0x84,0xd9,0xf7,0xe1 }},
  { 10, "D3",  0x240ec0, 0x80244dc0,     0x20, 1,0,1, 0,
		{ 0x40,0x65,0xb7,0x71,0x65, 0x6e,0xbc,0x7f,0x04,0x1c,
		  0xe3,0x44,0xb5,0x32,0x42, 0x0d,0x43,0x05,0x76,0xa8 }},
  { 11, "D4",  0x240ee0, 0x80244e00,  0x13120, 1,0,1, 0,
		{ 0xba,0x43,0xcc,0x17,0x8b, 0x01,0x44,0xc0,0x5b,0x38,
		  0x59,0x9d,0xfc,0x2c,0x14, 0xcd,0xa6,0x9b,0xc1,0x8a }},
  { 12, "D5",  0x254000, 0x80257f20,  0x4bac0, 1,0,1, 0,
		{ 0x16,0x24,0x5d,0xad,0xb9, 0x7f,0xe9,0x5f,0x00,0x7f,
		  0xef,0x6f,0xec,0xf6,0x24, 0xe5,0x35,0xc1,0x7c,0x38 }},
  { 13, "D6",  0x29fac0, 0x80384580,   0x13c0, 1,0,1, 0,
		{ 0xb7,0x6b,0x13,0x92,0x1c, 0xad,0x77,0x78,0x92,0xe8,
		  0xff,0xe4,0x5d,0x45,0xe6, 0x1c,0x06,0xb8,0x4d,0xdb }},
  { 14, "D7",  0x2a0e80, 0x80386920,   0x21a0, 1,0,1, 0,
		{ 0x4d,0xc4,0xf5,0x81,0x3b, 0x8b,0x9e,0x5d,0xc3,0x36,
		  0xfb,0x91,0xa9,0xc0,0x5b, 0xd7,0x59,0x11,0xee,0x25 }},
  { 15, "D8",         0,          0,        0, 0,0,0, 0, {0} },
  { 16, "D9",         0,          0,        0, 0,0,0, 0, {0} },
  { 17, "D10",        0,          0,        0, 0,0,0, 0, {0} },
  { 18, "BSS",        0, 0x802a3a00,  0xe50fc, 1,0,0, 0, {0} },
  { 19, "ENT",        0, 0x800060a4,        0, 1,0,0, 0, {0} },
};

///////////////////////////////////////////////////////////////////////////////

static const dol_sect_info_t dol_info_kor[DOL_NN_SECTIONS] =
{
  {  0, "T0",     0x100, 0x80004000,   0x2460, 1,0,1, 0,
		{ 0x8e,0x71,0x21,0xdf,0x02, 0x9f,0xff,0x15,0x45,0x5b,
		  0xc9,0x30,0xdd,0xda,0x1a, 0x84,0x35,0x41,0xe8,0xc3 }},
  {  1, "T1",    0x2560, 0x800072c0, 0x23dea0, 1,0,1, 0,
		{ 0xfc,0xf6,0x28,0x91,0x85, 0x47,0x11,0xc3,0x4e,0xac,
		  0x2e,0x61,0x47,0x53,0x5e, 0xe1,0x7c,0x3c,0xb8,0xe6 }},
  {  2, "T2",         0,          0,        0, 0,0,0, 0, {0} },
  {  3, "T3",         0,          0,        0, 0,0,0, 0, {0} },
  {  4, "T4",         0,          0,        0, 0,0,0, 0, {0} },
  {  5, "T5",         0,          0,        0, 0,0,0, 0, {0} },
  {  6, "T6",         0,          0,        0, 0,0,0, 0, {0} },
  {  7, "D0",  0x240400, 0x80006460,    0x5c0, 1,0,1, 0,
		{ 0x2d,0x62,0xc1,0xdb,0x2a, 0x82,0x64,0x0a,0xa7,0x61,
		  0xdd,0x6d,0xfc,0x4a,0x4d, 0x59,0xb4,0x51,0x6c,0x27 }},
  {  8, "D1",  0x2409c0, 0x80006a20,    0x8a0, 1,0,1, 0,
		{ 0x8b,0x99,0x7f,0x81,0xdf, 0x8b,0x0e,0x1d,0x96,0x3e,
		  0xbf,0xe7,0xa3,0xb2,0x0e, 0x92,0x7d,0xcc,0x5f,0x5b }},
  {  9, "D2",  0x241260, 0x80245160,     0xc0, 1,0,1, 0,
		{ 0x7f,0xcd,0xfb,0xdd,0x3e, 0x2b,0xf4,0xf2,0x35,0xd0,
		  0x4a,0x57,0xcb,0x9a,0xb0, 0xa2,0x2a,0x86,0xf0,0xa5 }},
  { 10, "D3",  0x241320, 0x80245220,     0x20, 1,0,1, 0,
		{ 0xcd,0x27,0xcf,0xe9,0xce, 0x3f,0x28,0xbc,0x64,0xb3,
		  0xb2,0x05,0x70,0x10,0xaf, 0x9a,0x5c,0xe6,0x04,0xf7 }},
  { 11, "D4",  0x241340, 0x80245240,  0x13100, 1,0,1, 0,
		{ 0x23,0x8c,0xc9,0x84,0xd1, 0x07,0x8a,0x1b,0xc4,0x56,
		  0x1b,0x56,0x30,0xe0,0x1a, 0x4d,0xbc,0xa1,0x48,0x0c }},
  { 12, "D5",  0x254440, 0x80258340,  0x39d00, 1,0,1, 0,
		{ 0x4a,0x10,0xfb,0x98,0x57, 0xb0,0xa5,0x96,0xb7,0xf6,
		  0x92,0x06,0xed,0xf2,0xdf, 0x04,0x5e,0x6d,0x63,0x55 }},
  { 13, "D6",  0x28e140, 0x80372c00,   0x13e0, 1,0,1, 0,
		{ 0xad,0x8f,0x5f,0x3d,0xf0, 0x9d,0xa5,0x26,0x9f,0x71,
		  0xfb,0xeb,0x7b,0x68,0x99, 0x72,0x71,0xdc,0x65,0xcc }},
  { 14, "D7",  0x28f520, 0x80374fc0,   0x21a0, 1,0,1, 0,
		{ 0xc3,0x04,0xfc,0x7f,0xf8, 0x6f,0xf0,0x39,0xa9,0xe7,
		  0xda,0x78,0xf3,0x1b,0xfa, 0x41,0x91,0x7d,0xf1,0x3c }},
  { 15, "D8",         0,          0,        0, 0,0,0, 0, {0} },
  { 16, "D9",         0,          0,        0, 0,0,0, 0, {0} },
  { 17, "D10",        0,          0,        0, 0,0,0, 0, {0} },
  { 18, "BSS",        0, 0x80292080,  0xe511c, 1,0,0, 0, {0} },
  { 19, "ENT",        0, 0x800060a4,        0, 1,0,0, 0, {0} },
};

///////////////////////////////////////////////////////////////////////////////

const dol_sect_info_t * GetInfoDOL ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:	return dol_info_pal;
	case STR_M_USA:	return dol_info_usa;
	case STR_M_JAP:	return dol_info_jap;
	case STR_M_KOR:	return dol_info_kor;
	default:	return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool GetSectionDOL
(
    staticr_t		* str,		// valid pointer
    dol_sect_info_t	* ret_info,	// valid pointer: returned data
    uint		section,	// 0 .. DOL_NN_SECTIONS
					//	(7*TEXT, 11*DATA, BSS, ENTRY )
    bool		calc_hash	// true: calculate SHA1 hash if data available

)
{
    DASSERT(str);
    DASSERT(ret_info);

    bool stat = GetDolSectionInfo( ret_info,( dol_header_t*)str->data,
					str->data_size, section );
    if ( calc_hash && stat && ret_info->data_valid )
    {
	ret_info->hash_valid = true;
	SHA1( ret_info->data, ret_info->size, ret_info->hash );
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

const DolSectionMap_t * FindSectionDOL ( uint size, u8 *hash )
{
    if ( !size || !hash )
	return 0;

    uint beg = 0;
    uint end = N_DOL_SECTION_MAP - 1;

    while ( beg <= end )
    {
	const uint idx = (beg+end)/2;
	const DolSectionMap_t *dsm = DolSectionMap + idx;
	if ( size < dsm->size )
	    end = idx - 1 ;
	else if ( size > dsm->size )
	    beg = idx + 1;
	else
	{
	    const int stat = memcmp(hash,dsm->hash,sizeof(dsm->hash));
	    if ( stat < 0 )
		end = idx - 1 ;
	    else if ( stat > 0 )
		beg = idx + 1;
	    else
		return dsm;
	}
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ct-code dol extensions		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[ct_dol_ext_t]]

typedef struct ct_dol_ext_t
{
	u32		t2_addr;
	BZ2Source_t	t2_src;

	u32		d8_addr;
	BZ2Source_t	d8_src;
}
ct_dol_ext_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ct_dol_ext_t ct_dol_ext_pal =
{
	0x802a5000,
	{ "rmcp/boot_code.bin;boot_code.bin",
		ctcode_boot_code_pal_bz2, sizeof(ctcode_boot_code_pal_bz2),
		FF_CT0_CODE },

	0x80002800,
	{ "rmcp/boot_data.bin;boot_data.bin",
		ctcode_boot_data_pal_bz2, sizeof(ctcode_boot_data_pal_bz2),
		FF_CT0_DATA },
};

//-----------------------------------------------------------------------------

ct_dol_ext_t ct_dol_ext_usa =
{
	0x802a5000,
	{ "rmce/boot_code.bin;boot_code.bin",
		ctcode_boot_code_usa_bz2, sizeof(ctcode_boot_code_usa_bz2),
		FF_CT0_CODE },

	0x80002800,
	{ "rmce/boot_data.bin;boot_data.bin",
		ctcode_boot_data_usa_bz2, sizeof(ctcode_boot_data_usa_bz2),
		FF_CT0_DATA },
};

//-----------------------------------------------------------------------------

ct_dol_ext_t ct_dol_ext_jap =
{
	0x802a5000,
	{ "rmcj/boot_code.bin;boot_code.bin",
		ctcode_boot_code_jap_bz2, sizeof(ctcode_boot_code_jap_bz2),
		FF_CT0_CODE },

	0x80002800,
	{ "rmcj/boot_data.bin;boot_data.bin",
		ctcode_boot_data_jap_bz2, sizeof(ctcode_boot_data_jap_bz2),
		FF_CT0_DATA },
};

//-----------------------------------------------------------------------------

ct_dol_ext_t ct_dol_ext_kor =
{
	0x802a5000,
	{ "rmck/boot_code.bin;boot_code.bin",
		0, 0, // [[2do]] [[ct-korea]]
		FF_CT0_CODE },

	0x80002800,
	{ "rmck/boot_data.bin;boot_data.bin",
		0, 0, // [[2do]] [[ct-korea]]
		FF_CT0_DATA },
};

//-----------------------------------------------------------------------------

ct_dol_ext_t * GetCtExtDOL ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:	return &ct_dol_ext_pal;
	case STR_M_USA:	return &ct_dol_ext_usa;
	case STR_M_JAP:	return &ct_dol_ext_jap;
	case STR_M_KOR: return &ct_dol_ext_kor;
	default:	return 0;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 PatchHTTPS()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint PatchHTTPS_HTTP
(
    staticr_t		* str,		// valid pointer
    https_mode_t	mode
)
{
    DASSERT(str);
    char buf[200], domain_buf[100];

    static const char nwn[] = "nintendowifi.net";
    static uint nwn_len = sizeof(nwn) - 1;

    const bool mode_sake   = mode == HTTPS_SAKE0 || mode == HTTPS_SAKE1;
    const bool repl_domain = mode == HTTPS_DOMAIN || mode_sake;

    if ( repl_domain && strlen(wifi_domain) > nwn_len )
    {
	ERROR0(ERR_SEMANTIC,
	    "Domain must be smaller than %u characters if using --https=DOMAIN: %s",
	    nwn_len+1, wifi_domain );
	opt_https = mode = HTTPS_HTTP; // fall back
    }

    uint count = 0;
    const str_server_patch_t *pat;
    const str_flags_t dol_flag = str->is_dol ? STR_F_IS_DOL : 0;

    for ( pat = ServerPatch; pat->mode; pat++ )
    {
	// dirty code: pat->flags is either 0(srel), 1(mdol), 2(mdol+special)
	if ( pat->mode != str->mode || (pat->flags & STR_F_IS_DOL) != dol_flag )
	    continue;
	if ( pat->flags & STR_F_IS_OPTIONAL && opt_wc24 )
	    continue;

	const str_server_list_t *sl = ServerList + pat->serv_type;
	memset(buf,0,sizeof(buf));

	if ( pat->flags & STR_F_RESTORE_HTTPS )
	{
	    strncpy(buf,"https://",sizeof(buf));
	}
	else if ( pat->flags & STR_F_IS_SPECIAL && wifi_domain_is_wiimmfi )
	{
	    switch (pat->serv_type)
	    {
		case SERV_NASW:
		    snprintf(buf,sizeof(buf),"http://naswii.%s/ac",wifi_domain);
		    break;

		case SERV_NASW_DEV:
		    snprintf(buf,sizeof(buf),"https://main.nas.%s/p%c",
				wifi_domain,
				tolower((int)VersusPointsInfo[pat->mode].region_code) );
		    break;

		case SERV_NASW_TEST:
		    snprintf(buf,sizeof(buf),"http://ca.nas.%s/ca",wifi_domain);
		    break;

		default:
		    ASSERT(0);
		    break;
	    }
	    PRINT("STR_F_IS_SPECIAL: %u %s\n",pat->serv_type,buf);
	}
	else if ( mode_sake && pat->serv_type == SERV_SAKE )
	{
	    // special case for SAKE servers

	    if (!pat->is_http)
		snprintf(buf,sizeof(buf),"gs.%s%s", nwn, pat->param );
	    else
	    {
		snprintf(buf,sizeof(buf),"%s://%s%s",
		    mode == HTTPS_SAKE0 && pat->is_http == 2 ? "https" : "http",
		    sl->domain, pat->param );
	    }
	}
	else if ( pat->serv_type == SERV_NONE )
	{
	    DASSERT(pat->is_http==2);
	    char * dest = StringCopyS(buf,sizeof(buf),
				mode == HTTPS_RESTORE ? "https" : "http" );
	    if ( pat->url_len == 8 )
		StringCopyE(dest,buf+sizeof(buf),"://");
	}
	else if (!pat->is_http)
	{
	    snprintf(buf,sizeof(buf),"gs.%s%s",
		repl_domain ? wifi_domain : nwn, pat->param);
	}
	else if ( repl_domain )
	{
	    snprintf(buf,sizeof(buf),"http://%.*s%s%s",
		sl->domain_len - nwn_len, sl->domain,
		wifi_domain, pat->param );
	}
	else
	{
	    ccp domain = sl->domain;
	    if ( mode == HTTPS_HTTP && !memcmp(domain,"naswii.",7) )
	    {
		snprintf(domain_buf,sizeof(domain_buf),"nas.%s",domain+7);
		domain = domain_buf;
	    }

	    snprintf(buf,sizeof(buf),"%s://%s%s",
		mode == HTTPS_RESTORE && pat->is_http == 2 ? "https" : "http",
		domain, pat->param );
	}

	char *data = (char*)str->data + pat->offset;
	if (memcmp(data,buf,pat->url_len+1))
	{
	    //HexDump16(stderr,0,0,data,pat->url_len+1);
	    //HexDump16(stderr,0,0,buf,pat->url_len+1);
	    memcpy(data,buf,pat->url_len+1);
	    count++;
	}
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////

static uint PatchHTTPS
(
    staticr_t		* str		// valid pointer
)
{
    switch (opt_https)
    {
	case HTTPS_RESTORE:
	case HTTPS_HTTP:
	case HTTPS_DOMAIN:
	case HTTPS_SAKE0:
	case HTTPS_SAKE1:
	    return PatchHTTPS_HTTP(str,opt_https);

	case HTTPS_NONE:
	case HTTPS__N:
	    break;
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			main.dol support		///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyzeDOL
(
    staticr_t		* str,		// valid pointer
    bool		restore		// true: restore patched data
)
{
    DASSERT(str);
    str->dol_status = DOL_S_ANALYZED;

    if (PatchHTTPS_HTTP(str,HTTPS_RESTORE))
	str->dol_status |= DOL_S_HTTPS;

    static str_mode_t tab_mode[]
	= { STR_M_PAL, STR_M_USA, STR_M_JAP, STR_M_KOR, STR_M_UNKNOWN };

    static sha1_hash_t tab_sha1_mode[] =
    {
	// STR_M_PAL
	{ 0x12, 0xb0, 0x2a, 0xf1, 0x9e,  0x07, 0xd3, 0x6d, 0x8a, 0x38,
	  0xc9, 0xa7, 0xfe, 0x89, 0x15,  0x55, 0x2f, 0x2e, 0x58, 0x5f },

	// STR_M_USA
	{ 0x0a, 0xed, 0x61, 0x96, 0x28,  0x11, 0xa1, 0x64, 0xf5, 0x8c,
	  0xd7, 0xed, 0xfc, 0x71, 0x6e,  0xeb, 0x05, 0xb8, 0x4d, 0xab },

	// STR_M_JAP
	{ 0x69, 0xed, 0xa1, 0xa4, 0x05,  0xde, 0x0e, 0xe1, 0x29, 0x09,
	  0x15, 0x4f, 0xd5, 0xdc, 0x2c,  0x33, 0x62, 0x9f, 0x6f, 0x37 },

	// STR_M_KOR
	{ 0x86, 0x2a, 0x38, 0xf2, 0xd7,  0x56, 0xe5, 0x96, 0x2c, 0xa1,
	  0x9f, 0xd0, 0x62, 0xd0, 0xbd,  0x55, 0x67, 0x10, 0xed, 0x7e },
    };

    if ( str->data_size < 0x1100 )
	return;

    sha1_hash_t hash;
    SHA1(str->data+0x100,0x1000,hash);

    uint i;
    for ( i = 0; tab_mode[i] != STR_M_UNKNOWN; i++ )
    {
	if (!memcmp(tab_sha1_mode+i,hash,sizeof(hash)))
	{
	    str->is_main	= true;
	    str->mode		= tab_mode[i];
	    str->dol_status	|= DOL_S_KNOWN;

	    SHA1(str->data,str->data_size,hash);
	    const DolSectionMap_t *dsm = FindSectionDOL(str->data_size,hash);
	    if ( dsm && dsm->mode == str->mode && dsm->dol_stat )
	    {
		str->dol_status |= dsm->dol_stat;
		str->dol_info = dsm->info;
		if ( str->dol_status & DOL_S_WIIMMFI )
		    str->dol_status &= ~DOL_S_HTTPS;
		else if ( str->dol_status & DOL_S_HTTPS )
		    str->dol_status &= ~DOL_S_ORIG;
	    }
	    break;
	}
    }
    str->dol_info_flags = ( str->dol_info_flags & ~DIF_MODE )
			| ( str->mode - STR_M_PAL & DIF_MODE );


    //-- check VBI

    const u32 vbi_addr = GetVbiAddress(str);
    if (vbi_addr)
    {
	dol_header_t *dh = (dol_header_t*)str->data;
	const u32 offset = GetDolOffsetByAddr(dh,vbi_addr,4,0);
	if (offset)
	{
	    const u32 cur_value = be32(str->data+offset);
	    if ( cur_value != ORIG_VBI_VALUE )
	    {
		if (restore)
		    write_be32(str->data+offset,ORIG_VBI_VALUE);
		str->dol_status = str->dol_status & ~DOL_S_ORIG | DOL_S_VBI;

		if ( ( cur_value & 0xff000000 ) == 0x4b000000 )
		{
		    str->vbi_address = cur_value & 0x00ffffff;
		    if ( str->vbi_address & 0x00800000 )
			str->vbi_address |= 0xff000000;
		    str->vbi_address += vbi_addr;
		}
	    }
	}
    }


    //-- check patched_by VERSION

    char *id_ptr = GetIdPointer(str);
    if (id_ptr)
    {
	char *id_end = id_ptr + PATCHED_BY_SIZE;
	char *end = id_ptr;
	while ( end < id_end && (uchar)*end >= ' ' )
	    end++;
	while ( end > id_ptr && ( end[-1] == '*' || end[-1] == ' ' ) )
	    end--;

	memset(str->patched_by,0,sizeof(str->patched_by));
	if ( id_ptr < end )
	    memcpy(str->patched_by,id_ptr,end-id_ptr);
	if (restore)
	    memset(id_ptr,'*',PATCHED_BY_SIZE);
    }


    //-- check sdkver VERSION

    const u32 vers_addr = GetSdkverAddress(str->mode);
    if (vbi_addr)
    {
	dol_header_t *dh = (dol_header_t*)str->data;
	const u32 offset = GetDolOffsetByAddr(dh,vers_addr,8,0);
	if ( offset && memcmp(str->data+offset,ORIG_SDK_VERS,8) )
	{
	    memcpy(str->sdk_version,str->data+offset,8);
	    str->dol_status = str->dol_status & ~DOL_S_ORIG | DOL_S_SDKVER;
	    if (restore)
		memcpy(str->data+offset,ORIG_SDK_VERS,8);
	}
    }


    //-- check second host

    u32 h2_orig, *h2 = GetSecondHostPointer(str,&h2_orig);
    if (h2)
    {
	if ( ntohl(*h2) == ASM_NOP )
	{
	    str->dol_status = str->dol_status & ~DOL_S_ORIG | DOL_S_2HOST;
	    if (restore)
		*h2 = htonl(h2_orig);
	}
    }


    //-- finish

    if (!str->dol_info_flags_set)
    {
	str->dol_info_flags_set = true;
	if ( str->dol_status & DOL_S_ORIG )
	    str->dol_info_flags |= DIF_ORIG;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PrintStatusDOL
(
    FILE		* f,		// destination file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    int			long_mode	// >0: print more track details if changed
)
{
    indent = NormalizeIndent(indent);
    if (!(str->dol_status&DOL_S_ANALYZED))
	AnalyzeDOL(str,true);

  #ifdef TEST
    if ( opt_new > 0 )
    {
	u32 off1 = GetVbiAddress(str);
	u32 off2 = FindDolAddressOfVBI(str->data,str->data_size);
	printf("VBI: %08x %c= %08x : %s\n",
		off1, off1 == off2 ? '=' : '!',
		off2, str->fname );
    }
  #endif

    switch (str->mode)
    {
	case STR_M_PAL:
	    fprintf(f,"%*sPAL version of main.dol found.\n", indent,"" );
	    break;

	case STR_M_USA:
	    fprintf(f,"%*sNTSC/USA version of main.dol found.\n", indent,"" );
	    break;

	case STR_M_JAP:
	    fprintf(f,"%*sNTSC/JAP version of main.dol found.\n", indent,"" );
	    break;

	case STR_M_KOR:
	    fprintf(f,"%*sNTSC/KOREA version of main.dol found.\n", indent,"" );
	    break;

	default:
	    fprintf(f,"%*sUnknown main.dol found.\n", indent,"" );
	    return;
    }

    if (str->dol_cleaned)
	fprintf(f,"%*s- DOL cleaned before analysis.\n", indent,"");

    if ( str->patched_by[0] )
	fprintf(f,"%*s- Patched by: %s\n", indent,"", str->patched_by );


    if ( str->dol_status & DOL_S_HTTPS )
    {
	fprintf(f,"%*s- Known 'https://' URLs modified.\n", indent,"");
    }
    else if (str->dol_info)
    {
	fprintf(f,"%*s- %s\n", indent,"",str->dol_info);
	return;
    }

    const dol_header_t *dh = (dol_header_t*)str->data;
    if ( str->dol_status & DOL_S_VBI )
    {
	if (str->vbi_address)
	    fprintf(f,"%*s- VBI hook to address 0x%08x%s found.\n",
			indent,"", str->vbi_address,
			GetDolSectionAddrInfo(dh,str->vbi_address,4) );
	else
	    fprintf(f,"%*s- VBI hook found.\n", indent,"");
    }

 #ifdef HAVE_WIIMM_EXT
    if ( str->dol_status & DOL_S_SDKVER )
	fprintf(f,"%*s- Value of 'sdkver' changed from '001000' to '%s'.\n",
			indent,"", str->sdk_version );
 #endif

    if ( str->dol_status & DOL_S_2HOST )
	fprintf(f,"%*s- Suppress second 'Host:' header at login.\n",
			indent,"" );

    const dol_sect_info_t *orig = GetInfoDOL(str->mode);
    if (orig)
    {
	uint sect;
	for ( sect = 0; sect < DOL_NN_SECTIONS; sect++, orig++ )
	{
	    dol_sect_info_t info;
	    GetSectionDOL(str,&info,sect,true);
	    noPRINT("%-4s: %d,%d : %8x %8x %8x\n",
		info.name, info.sect_valid, info.hash_valid,
		info.off, info.addr, info.size );

	    if (info.sect_valid)
	    {
		bool find_section = info.hash_valid;

		if (!orig->sect_valid)
		    fprintf(f,"%*s- Additional section %s.\n", indent,"", orig->name );
		else if ( orig->size != info.size )
			fprintf(f,"%*s- Size of section %s changed.\n", indent,"", orig->name );
		else if ( orig->hash_valid && memcmp(orig->hash,info.hash,sizeof(info.hash)))
		    fprintf(f,"%*s- Section %s modified.\n", indent,"", orig->name );
		else if ( orig->addr != info.addr )
		{
		    if ( sect == DOL_IDX_ENTRY )
			fprintf(f,"%*s- Entry point moved from %08x%s to %08x%s.\n",
				indent,"",
				orig->addr, GetDolSectionAddrInfo(dh,orig->addr,4),
				info.addr, GetDolSectionAddrInfo(dh,info.addr,4) );
		    else
			fprintf(f,"%*s- Section %s moved.\n", indent,"", orig->name );
		}
		else
		    find_section = false;

		if (find_section)
		{
		    const DolSectionMap_t *dsm = FindSectionDOL(info.size,info.hash);
		    if (dsm)
			printf("%*s- %s\n",indent+3,"",dsm->info);
		    else if (info.data)
		    {
			bool found = false;
			if ( info.addr == 0x80001800 && info.size > 0x800 )
			{
			    if ( be64(info.data+0x11c) == 0x3c6000d06063c0deull )
			    {
				found = true;
				printf("%*s- Gecko Code Handler (Only)\n",indent+3,"");
			    }
			    else if ( be64(info.data+0x770) == 0x3c6000d06063c0deull )
			    {
				ccp attrib = "full";
				const u64 val = be64(info.data+0x56C);
				if ( val == 0x92f8680090786810ull )
				    attrib = "Slot A";
				else if ( val == 0x92f8681490786824ull )
				    attrib = "Slot B";
				printf("%*s- Gecko Code Handler (%s).\n",indent+3,"",attrib);
				found = true;
			    }
			}

			if ( !found && info.addr == 0x802c0000 )
			    printf("%*s- Maybe code loader by wstrt.\n",indent+3,"");
		    }
		}
	    }
	    else if (orig->sect_valid)
		fprintf(f,"%*s- Section %s removed.\n", indent,"", orig->name );
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			versus points			///////////////
///////////////////////////////////////////////////////////////////////////////

uint GetVersusPointsOffset ( str_mode_t mode )
{
    switch (mode)
    {
	case STR_M_PAL:	    return 0x37fd50;
	case STR_M_USA:	    return 0x37fc60;
	case STR_M_JAP:	    return 0x37fa20;
	case STR_M_KOR:	    return 0x380130;
	default:	    return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// [[VersusPointsInfo]]

const VersusPointsInfo_t VersusPointsInfo[STR_M__N] =
{
    { STR_M_UNKNOWN, '0', 0, 0 },
    { STR_M_PAL, 'P', 0x37fd50, 0x06890030 },
    { STR_M_USA, 'E', 0x37fc60, 0x0688bbc0 },
    { STR_M_JAP, 'J', 0x37fa20, 0x0688f680 },
    { STR_M_KOR, 'K', 0x380130, 0x0687e430 },
};

///////////////////////////////////////////////////////////////////////////////

str_mode_t opt_cheat_region = STR_M_UNKNOWN;

int ScanOptCheatRegion ( ccp arg )
{
    if ( !arg || !*arg )
    {
	opt_cheat_region = STR_M_UNKNOWN;
	return 0;
    }

    static const KeywordTab_t tab[] =
    {
	{ STR_M_PAL,		"EUROPE",	"PAL",		0 },
	{ STR_M_PAL,		"AUSTRALIA",	"OCEANIA",	0 },
	{ STR_M_USA,		"AMERICA",	"NTSC",		0 },
	{ STR_M_JAP,		"JAPAN",	0,		0 },
	{ STR_M_KOR,		"KOREA",	0,		0 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	opt_cheat_region = cmd->id;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid cheat region (option --cheat): '%s'\n",arg);
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyze helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static void AnalyzeTracks
(
    staticr_t		* str,		// valid pointer
    u8			* data,		// data to analyze and modify
    const u32		* offset_tab,	// null terminated table with offsets
    const u32		* ref_tab,	// reference table
    uint		base_index,	// base index of first track
    uint		n,		// number of values in 'ref_tab'
    int			base_status	// base status bit
)
{
    DASSERT(str);
    DASSERT(data);
    DASSERT(offset_tab);
    DASSERT(ref_tab);
    DASSERT(n>0);
    DASSERT(n<=32);

    const uint cmp_size = n * sizeof(u32);

    uint i;
    u32 be_ref[32];
    for ( i = 0; i < n; i++ )
	write_be32( be_ref + i, base_index + ref_tab[i] );
    TRACE_HEXDUMP16(0,0,be_ref,cmp_size); //putchar('\n');

    uint ok_count	= 0;  // number of unmodified records
    uint mod_count	= 0;  // number of modified records

    u8  *first = 0;
    const u32 *off_ptr;
    for ( off_ptr = offset_tab ; *off_ptr; off_ptr++ )
    {
	DASSERT( *off_ptr < str->data_size );
	u8 * d = data + *off_ptr;
	if (!memcmp(d,be_ref,cmp_size))
	{
	    ok_count++;
	    continue;
	}
	mod_count++;

	if (!first)
	    first = d;
	else if (memcmp(first,d,cmp_size))
	    str->str_status |= base_status << STR_S_SHIFT_ORDER_DIFF;

	uint fail_buf[32];
	memset(fail_buf,0,sizeof(fail_buf));
	for ( i = 0; i < n; i++ )
	{
	    const u32 idx = be32(d+4*i) - base_index;
	    if ( idx >= n )
	    {
		str->str_status |= base_status << STR_S_SHIFT_ORDER_DIFF;
		break;
	    }
	    if ( fail_buf[idx] )
	    {
		str->str_status |= base_status << STR_S_SHIFT_ORDER_MULTI;
		break;
	    }
	    fail_buf[idx]++;
	}
	if ( i == n )
	    str->str_status |= base_status << STR_S_SHIFT_ORDER_ALT;

	TRACE_HEXDUMP16(0,*off_ptr,d,cmp_size); //putchar('\n');
    }

    if ( mod_count && ok_count )
	str->str_status |= base_status << STR_S_SHIFT_ORDER_DIFF;


    //--- normalize

    for ( off_ptr = offset_tab ; *off_ptr; off_ptr++ )
    {
	DASSERT( *off_ptr < str->data_size );
	u8 * d = data + *off_ptr;
	memcpy(d,be_ref,cmp_size);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void AnalyzeRegion
(
    staticr_t		* str,		// valid pointer
    u8			* data		// data to analyze and modify
)
{
    DASSERT(str);
    DASSERT(data);

    bool is_battle;
    for ( is_battle = false; is_battle <= true; is_battle++ )
    {
	region_info_t ri;
	SetupRegionInfo(&ri,str,is_battle);

	uint ok_stat = ri.flag_known;

	const uint offset_idx = str->mode - STR_ZBI_FIRST;
	memset(ri.region,0,MAX_REGION_PATCH*sizeof(*ri.region));
	uint idx, patch_count = 0, unknown_count = 0;

	for ( idx = 0; idx < ri.max_patch; idx++ )
	{
	    ri.region[idx] = -1;
	    const region_patch_t *rp = ri.patch_tab + idx;
	    if (!rp->orig)
		break;

	    u8 *focus = data + rp->offset[offset_idx];
	    u32 val = be32(focus);
	    if ( val != rp->orig )
	    {
		patch_count++;
		if ( ( val & 0xffff0000 ) == rp->patch )
		{
		    ri.region[idx] = (u16)val;
		    if ( ri.region[idx] != ri.region[0] )
			ok_stat = ri.flag_differ;
		}
		else
		    unknown_count++;
		write_be32(focus,rp->orig);
	    }
	}

	if (unknown_count)
	    str->str_status |= ri.flag_unknown;
	else if (patch_count)
	    str->str_status |= ok_stat;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeAllRanks
(
    staticr_t		* str,		// valid pointer
    u8			* data,		// data to analyze and modify
    const u32		* offset_tab	// null terminated table with offsets
)
{
    DASSERT(str);
    DASSERT(data);
    DASSERT(offset_tab);

    uint i, patch_count = 0, known_count = 0;
    for ( i = 0; offset_tab[i]; i++ )
    {
	u8 *d = data + offset_tab[i];
	if (memcmp(d,all_ranks_orig,all_ranks_size))
	{
	    patch_count++;
	    if (   memcmp(d,all_ranks_patch1,all_ranks_size)
		|| memcmp(d,all_ranks_patch2,all_ranks_size))
	    {
		known_count++;
	    }
	    memcpy(d,all_ranks_orig,all_ranks_size);
	}
    }

    if ( i == known_count )
    {
	str->str_status |= STR_S_ALL_RANKS_KNOWN;
    }
    else if ( patch_count )
	str->str_status |= STR_S_ALL_RANKS_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeVsBt
(
    staticr_t		* str,		// valid pointer
    u8			* data,		// data to analyze and modify
    const u32		* offset_tab,	// null terminated table with offsets
    ccp			name,		// 'vs' or 'bt'
    uint		flag		// flag to set
)
{
    DASSERT(str);
    DASSERT(data);
    DASSERT(offset_tab);

    uint i;
    for ( i = 0; offset_tab[i]; i++ )
    {
	u8 *d = data + offset_tab[i];
	if (strncmp((ccp)d,name,3))
	{
	    str->str_status |= flag;
	    memcpy(d,name,2);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeVersusPoints
(
    staticr_t		* str,		// valid pointer
    u8			* data		// data to analyze and modify
)
{
    DASSERT(str);
    DASSERT(data);

    const u32 offset = GetVersusPointsOffset(str->mode);
    if (offset)
    {
	data += offset;
	if (memcmp(data,MkwPointsTab_NINTENDO,12*12))
	{
	    str->str_status |= STR_S_VERSUS_POINTS;
	    memcpy(data,MkwPointsTab_NINTENDO,12*12);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeCannon
(
    staticr_t		* str,		// valid pointer
    u8			* data		// data to analyze and modify
)
{
    DASSERT(str);
    DASSERT(data);

    const u32 off = GetCannonOffset(str);
    if ( off && !(str->str_status&STR_S_CANNON) )
    {
	memcpy(str->cannon_data,data+off,sizeof(str->cannon_data));
	if (memcmp(data+off,CannonDataLEX+4,sizeof(CannonDataLEX)-4))
	{
	    str->str_status |= STR_S_CANNON;
	    memcpy(data+off,CannonDataLEX+4,sizeof(CannonDataLEX)-4);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeMenO
(
    staticr_t		* str,		// valid pointer
    u8			* data		// data to analyze and modify
)
{
    DASSERT(str);
    DASSERT(data);

    const u32 *optr = GetMenuOffsetTab(str->mode);
    if (optr)
    {
	uint count = 0, total = 0;
	const int *idx;
	for ( idx = meno_patch_idx; *idx >= 0; idx++ )
	{
	    total++;
	    u8 *d = data + optr[*idx] + 13; // 13 = 'O' offset
	    if ( *d == 'O' )
	    {
		*d = 'u';
		count++;
	    }
	}

	if ( total == count )
	    str->str_status |= STR_S_MENO;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyze staticr			///////////////
///////////////////////////////////////////////////////////////////////////////

static void AnalyzePAL
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);


    //--- first hash test

    static const u8 good_hash[20] =
    {
	0x88,0x7b,0xcc,0x07,0x67, 0x81,0xf5,0xb0,0x05,0xcc,
	0x31,0x7a,0x6e,0x3c,0xc8, 0xfd,0x5f,0x91,0x13,0x00
    };

    sha1_hash_t hash;
    SHA1(str->data,str->data_size,hash);
    if (!memcmp(hash,good_hash,sizeof(hash)))
    {
	if (!( str->str_status & STR_S_HTTPS ))
	    str->str_status |= STR_S_ORIG;
     #ifndef TEST
	return;
     #endif
    }


    //--- create a copy

    u8 * data = MALLOC(str->data_size);
    memcpy(data,str->data,str->data_size);


    //--- ctgp 4.4 tests

    static const u32 ctgp44[] =
    {
	// addr	  orig value  ctgp value
	//-------------------------------
	0x3d22b8, 0x00000db4, 0xff750030,
	0x3d22c8, 0x00000db4, 0xff750030,
	0x40df78, 0x00012660, 0xff7742e0,
	0x40df88, 0x00012660, 0xff7742e0,
	0,0,0
    };

    const u32 * ptr;
    uint orig_failed = 0, ctgp_failed = 0;
    for ( ptr = ctgp44; *ptr; ptr += 3 )
    {
	DASSERT( *ptr + 4 <= str->data_size );
	const u32 val = be32( str->data + *ptr );
	if ( val != ptr[1] ) orig_failed++;
	if ( val != ptr[2] ) ctgp_failed++;
	write_be32( data + *ptr, ptr[1] );
    }

    if (orig_failed)
    {
	str->str_status |= STR_S_CTGP44_LIKE;
	if (!ctgp_failed)
	    str->str_status |= STR_S_CTGP44;
    }


    //--- analyse tracks and arenas

    AnalyzeTracks( str, data, pal_track_offset, track_pos_default,
			MKW_TRACK_BEG, MKW_N_TRACKS, STR_S_TRACK_ORDER );
    AnalyzeTracks( str, data, pal_arena_offset, arena_pos_default,
			MKW_ARENA_BEG, MKW_N_ARENAS, STR_S_ARENA_ORDER );


    //--- analyse misc

    AnalyzeRegion(str,data);
    AnalyzeAllRanks(str,data,pal_all_ranks_offset);
    AnalyzeVsBt(str,data,pal_vs_offset,"vs",STR_S_VS);
    AnalyzeVsBt(str,data,pal_bt_offset,"bt",STR_S_BT);
    AnalyzeVersusPoints(str,data);
    AnalyzeCannon(str,data);
    AnalyzeMenO(str,data);


    //--- last hash test

    SHA1(data,str->data_size,hash);
    if (memcmp(hash,good_hash,sizeof(hash)))
	str->str_status |= STR_S_UNKNOWN_PATCH;


    //--- clean

    FREE(data);
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeUSA
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);

    //--- first hash test

    static const u8 good_hash[20] =
    {
	0x07,0x2b,0xa4,0x43,0x82, 0xfd,0xca,0x9b,0xf5,0xc0,
	0xec,0x9d,0x1e,0x9b,0x92, 0x2c,0xaf,0xd9,0x92,0x60
    };

    sha1_hash_t hash;
    SHA1(str->data,str->data_size,hash);
    if (!memcmp(hash,good_hash,sizeof(hash)))
    {
	str->str_status |= STR_S_ORIG;
     #ifndef TEST
	return;
     #endif
    }


    //--- create a copy

    u8 * data = MALLOC(str->data_size);
    memcpy(data,str->data,str->data_size);


    //--- ctgp 4.4 tests

    static const u32 ctgp44[] =
    {
	// addr	  orig value  ctgp value
	//-------------------------------
	0x3d1c38, 0x00000c14, 0xff7546e0,
	0x3d1c48, 0x00000c14, 0xff7546e0,
	0x40cda0, 0x00012098, 0xff778720,
	0x40cdb0, 0x00012098, 0xff778720,
	0,0,0
    };

    const u32 * ptr;
    uint orig_failed = 0, ctgp_failed = 0;
    for ( ptr = ctgp44; *ptr; ptr += 3 )
    {
	DASSERT( *ptr + 4 <= str->data_size );
	const u32 val = be32( str->data + *ptr );
	if ( val != ptr[1] ) orig_failed++;
	if ( val != ptr[2] ) ctgp_failed++;
	write_be32( data + *ptr, ptr[1] );
    }

    if (orig_failed)
    {
	str->str_status |= STR_S_CTGP44_LIKE;
	if (!ctgp_failed)
	    str->str_status |= STR_S_CTGP44;
    }


    //--- anlyze tracks and arenas

    AnalyzeTracks( str, data, usa_track_offset, track_pos_default,
			MKW_TRACK_BEG, MKW_N_TRACKS, STR_S_TRACK_ORDER );
    AnalyzeTracks( str, data, usa_arena_offset, arena_pos_default,
			MKW_ARENA_BEG, MKW_N_ARENAS, STR_S_ARENA_ORDER );


    //--- analyse misc

    AnalyzeRegion(str,data);
    AnalyzeAllRanks(str,data,usa_all_ranks_offset);
    AnalyzeVsBt(str,data,usa_vs_offset,"vs",STR_S_VS);
    AnalyzeVsBt(str,data,usa_bt_offset,"bt",STR_S_BT);
    AnalyzeVersusPoints(str,data);
    AnalyzeMenO(str,data);


    //--- last hash test

    SHA1(data,str->data_size,hash);
    if (memcmp(hash,good_hash,sizeof(hash)))
	str->str_status |= STR_S_UNKNOWN_PATCH;


    //--- clean

    FREE(data);
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeJAP
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);

    //--- first hash test

    static const u8 good_hash[20] =
    {
	0x20,0xb4,0x16,0x1d,0x41, 0x5e,0x40,0x92,0xa6,0x29,
	0x9b,0xb2,0x85,0xfa,0xa6, 0x22,0x83,0x5b,0x2b,0x16
    };

    sha1_hash_t hash;
    SHA1(str->data,str->data_size,hash);
    if (!memcmp(hash,good_hash,sizeof(hash)))
    {
	str->str_status |= STR_S_ORIG;
     #ifndef TEST
	return;
     #endif
    }


    //--- create a copy

    u8 * data = MALLOC(str->data_size);
    memcpy(data,str->data,str->data_size);


    //--- anlyze tracks and arenas

    AnalyzeTracks( str, data, jap_track_offset, track_pos_default,
			MKW_TRACK_BEG, MKW_N_TRACKS, STR_S_TRACK_ORDER );
    AnalyzeTracks( str, data, jap_arena_offset, arena_pos_default,
			MKW_ARENA_BEG, MKW_N_ARENAS, STR_S_ARENA_ORDER );


    //--- analyse misc

    AnalyzeRegion(str,data);
    AnalyzeAllRanks(str,data,jap_all_ranks_offset);
    AnalyzeVsBt(str,data,jap_vs_offset,"vs",STR_S_VS);
    AnalyzeVsBt(str,data,jap_bt_offset,"bt",STR_S_BT);
    AnalyzeVersusPoints(str,data);
    AnalyzeMenO(str,data);


    //--- last hash test

    SHA1(data,str->data_size,hash);
    if (memcmp(hash,good_hash,sizeof(hash)))
	str->str_status |= STR_S_UNKNOWN_PATCH;


    //--- clean

    FREE(data);
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeKOR
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);

    //--- first hash test

    static const u8 good_hash[20] =
    {
	0xe0,0x00,0x62,0x50,0xb1, 0xe5,0xc1,0xbd,0xee,0x8b,
	0xdb,0x0e,0x78,0x9e,0xa2, 0x00,0xb3,0xa8,0x46,0xe4
    };

    sha1_hash_t hash;
    SHA1(str->data,str->data_size,hash);
    if (!memcmp(hash,good_hash,sizeof(hash)))
    {
	str->str_status |= STR_S_ORIG;
     #ifndef TEST
	return;
     #endif
    }


    //--- create a copy

    u8 * data = MALLOC(str->data_size);
    memcpy(data,str->data,str->data_size);


    //--- anlyze tracks and arenas

    AnalyzeTracks( str, data, kor_track_offset, track_pos_default,
			MKW_TRACK_BEG, MKW_N_TRACKS, STR_S_TRACK_ORDER );
    AnalyzeTracks( str, data, kor_arena_offset, arena_pos_default,
			MKW_ARENA_BEG, MKW_N_ARENAS, STR_S_ARENA_ORDER );


    //--- analyse misc

    AnalyzeRegion(str,data);
    AnalyzeAllRanks(str,data,kor_all_ranks_offset);
    AnalyzeVsBt(str,data,kor_vs_offset,"vs",STR_S_VS);
    AnalyzeVsBt(str,data,kor_bt_offset,"bt",STR_S_BT);
    AnalyzeVersusPoints(str,data);
    AnalyzeMenO(str,data);


    //--- last hash test

    SHA1(data,str->data_size,hash);
    if (memcmp(hash,good_hash,sizeof(hash)))
	str->str_status |= STR_S_UNKNOWN_PATCH;


    //--- clean

    FREE(data);
}

///////////////////////////////////////////////////////////////////////////////

void AnalyzeSTR
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);

    if (str->is_dol)
    {
	if (!(str->dol_status&DOL_S_ANALYZED))
	    AnalyzeDOL(str,true);
	return;
    }

    if (!CheckSizeSTR(str))
	return;

    str->str_status = STR_S_ANALYZED;
    if (PatchHTTPS_HTTP(str,HTTPS_RESTORE))
	str->str_status |= STR_S_HTTPS;

    switch (str->mode)
    {
	case STR_M_PAL:
	    AnalyzePAL(str);
	    break;

	case STR_M_USA:
	    AnalyzeUSA(str);
	    break;

	case STR_M_JAP:
	    AnalyzeJAP(str);
	    break;

	case STR_M_KOR:
	    AnalyzeKOR(str);
	    break;

	default:
	    break;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void PrintStatusRegion
(
    FILE		* f,		// destination file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    bool		is_battle,	// true: is battle
    int			long_mode	// >0: print more track details if changed
)
{
    DASSERT(f);
    DASSERT(str);

    region_info_t ri;
    SetupRegionInfo(&ri,str,is_battle);

    if ( long_mode > 1
	|| long_mode > 0 && str->str_status & (ri.flag_unknown|ri.flag_differ) )
    {
	fprintf(f,"%*s- %c%s region settings:\n",
		indent,"", toupper((int)ri.name[0]), ri.name+1 );

	const region_patch_t *rp;
	for ( rp = ri.patch_tab; rp->orig; rp++ )
	{
	    const uint offset = rp->offset[str->mode - STR_ZBI_FIRST];
	    const u8 *d = str->data + offset;
	    fprintf(f,"%*s%9x: %02x %02x %02x %02x : ",
			indent,"", offset, d[0], d[1], d[2], d[3] );
	    const u32 val = be32(d);
	    if ( val == rp->orig )
		fprintf(f,"%-23s : %s\n","not patched",rp->info);
	    else if ( (val & 0xffff0000) == rp->patch )
		fprintf(f,"region %4x/hex = %5u : %s\n",
			val&0xffff, val&0xffff, rp->info );
	    else
		fprintf(f,"%-23s : %s\n","unknown patch",rp->info);
	}
    }
    else if ( str->str_status & ri.flag_differ )
    {
	fprintf(f,"%*s- Known %s region patch found, regions = ", indent,"", ri.name );
	ccp sep = "";
	uint i;
	for ( i = 0; i < ri.max_patch; i++ )
	{
	    if ( ri.region[i] < 0 )
		fprintf(f,"%sunpatched",sep);
	    else
		fprintf(f,"%s%#x=%u",sep,ri.region[i],ri.region[i]);
	    sep = ", ";
	}
	fputc('\n',f);
    }
    else if ( str->str_status & ri.flag_known )
	fprintf(f,"%*s- Known %s region patch found, region = 0x%02x = %u\n",
			indent,"", ri.name, ri.region[0], ri.region[0] );
    else if ( str->str_status & ri.flag_unknown )
	fprintf(f,"%*s- Unknown %s region patch found.\n", indent,"", ri.name );
}

///////////////////////////////////////////////////////////////////////////////

void PrintStatusSTR
(
    FILE		* f,		// destination file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    int			long_mode	// >0: print more track details if changed
)
{
    DASSERT(f);
    DASSERT(str);

    if (str->is_dol)
    {
	PrintStatusDOL(f,indent,str,long_mode);
	return;
    }

    indent = NormalizeIndent(indent);
    if (!(str->str_status&STR_S_ANALYZED))
	AnalyzeSTR(str);

    switch (str->mode)
    {
	case STR_M_PAL:
	    fprintf(f,"%*sPAL version of StaticR found.\n", indent,"" );
	    break;

	case STR_M_USA:
	    fprintf(f,"%*sNTSC/USA version of StaticR found.\n", indent,"" );
	    break;

	case STR_M_JAP:
	    fprintf(f,"%*sNTSC/JAP version of StaticR found.\n", indent,"" );
	    break;

	case STR_M_KOR:
	    fprintf(f,"%*sNTSC/KOREA version of StaticR found.\n", indent,"" );
	    break;

	default:
	    fprintf(f,"%*sNo StaticR found.\n", indent,"" );
	    break;
    }

    const str_status_t st = str->str_status;

    if ( st & STR_S_WRONG_SIZE )
	fprintf(f,"%*s- Wrong file size.\n", indent,"" );

    if ( st & STR_S_ORIG )
	fprintf(f,"%*s- Original file.\n", indent,"" );

    if ( st & STR_S_HTTPS )
	fprintf(f,"%*s- Known 'https://' URLs modified.\n", indent,"" );

    if ( st & STR_S_TRACK_ORDER_ALT )
	fprintf(f,"%*s- Valid alternative track order detected.\n", indent,"" );
    if ( st & STR_S_TRACK_ORDER_MULTI )
	fprintf(f,"%*s- Multiple usage of singe track detected.\n", indent,"" );
    if ( st & STR_S_TRACK_ORDER_BAD )
	fprintf(f,"%*s- Bad track index detected.\n", indent,"" );
    if ( st & STR_S_TRACK_ORDER_DIFF )
	fprintf(f,"%*s- Different track orders detected.\n", indent,"" );

    if ( long_mode > 1 || long_mode > 0 && st & STR_S_TRACK_ORDER__FAIL )
    {
	fprintf(f,"%*s- Track assigning tables:\n", indent,"" );
	const u32 *tab = GetTrackOffsetTab(str);
	DASSERT(tab);
	for(;;)
	{
	    u32 offset = *tab++;
	    if (!offset)
		break;
	    fprintf(f,"%*s%9x:", indent,"", offset );
	    const u32 * d = (u32*)( str->data + offset );
	    uint i;
	    for ( i = 0; i < MKW_N_TRACKS; i++ )
	    {
		if (!(i%4))
		    fputc(' ',f);
		fprintf(f,"%u%c", be32(d++), i == MKW_N_TRACKS-1 ? '\n' : ',' );
	    }
	}
    }

    if ( st & STR_S_ARENA_ORDER_ALT )
	fprintf(f,"%*s- Valid alternative arena order detected.\n", indent,"" );
    if ( st & STR_S_ARENA_ORDER_MULTI )
	fprintf(f,"%*s- Multiple usage of singe arena detected.\n", indent,"" );
    if ( st & STR_S_ARENA_ORDER_BAD )
	fprintf(f,"%*s- Bad arena index detected.\n", indent,"" );
    if ( st & STR_S_ARENA_ORDER_DIFF )
	fprintf(f,"%*s- Different arena orders detected.\n", indent,"" );

    if ( long_mode > 1 || long_mode > 0 && st & STR_S_ARENA_ORDER__FAIL )
    {
	fprintf(f,"%*s- Arena assigning tables:\n", indent,"" );
	const u32 *tab = GetArenaOffsetTab(str);
	DASSERT(tab);
	for(;;)
	{
	    u32 offset = *tab++;
	    if (!offset)
		break;
	    fprintf(f,"%*s%9x:", indent,"", offset );
	    const u32 * d = (u32*)( str->data + offset );
	    uint i;
	    for ( i = 0; i < MKW_N_ARENAS; i++ )
	    {
		if (!(i%5))
		    fputc(' ',f);
		fprintf(f,"%u%c", be32(d++), i == MKW_N_ARENAS-1 ? '\n' : ',' );
	    }
	}
    }

    if ( st & STR_S_CTGP44 )
	fprintf(f,"%*s- CTGP 4.4 modification found.\n", indent,"" );
    else if ( st & STR_S_CTGP44_LIKE )
	fprintf(f,"%*s- CTGP 4.4 like modification found.\n", indent,"" );

    PrintStatusRegion( f, indent, str, false, long_mode );
    PrintStatusRegion( f, indent, str, true,  long_mode );

    if ( st & STR_S_ALL_RANKS_KNOWN )
	fprintf(f,"%*s- Known 'all ranks' patch found.\n",
			indent,"" );
    else if ( st & STR_S_ALL_RANKS_UNKNOWN )
	fprintf(f,"%*s- Unknown 'all ranks' patch found.\n", indent,"" );

    if ( st & STR_S_VS )
    {
	fprintf(f,"%*s- Versus identification ('vs') patched:",indent,"");
	const u32 *ptr;
	for ( ptr = GetVsBtOffsetTab(str,false); *ptr; ptr++ )
	    fprintf(f," %.2s",str->data+*ptr);
	fputc('\n',f);
    }

    if ( st & STR_S_BT )
    {
	fprintf(f,"%*s- Battle identification ('bt') patched:",indent,"");
	const u32 *ptr;
	for ( ptr = GetVsBtOffsetTab(str,true); *ptr; ptr++ )
	    fprintf(f," %.2s",str->data+*ptr);
	fputc('\n',f);
    }

    if ( st & STR_S_VERSUS_POINTS )
    {
	const u32 offset = GetVersusPointsOffset(str->mode);
	fprintf(f,"%*s- Versus points modified to '%s'.\n",
		indent,"", GetMkwPointInfo(str->data+offset)->info );
    }

    if ( st & STR_S_CANNON )
    {
	fprintf(f,"%*s- Cannon parameters modified.\n", indent,"" );
	const u8 *orig = CannonDataLEX+4;
	const u8 *data = str->cannon_data;
	int idx;
	for ( idx = 0; orig < CannonDataLEX + sizeof(CannonDataLEX);
			idx++, orig += CANNON_SIZE, data += CANNON_SIZE )
	{
	    if (memcmp(orig,data,CANNON_SIZE))
	    {
		printf("%*s   > Cannon %u orig: %12.4f %12.4f %12.4f %12.4f\n",
			indent,"",
			idx, bef4(orig), bef4(orig+4), bef4(orig+8), bef4(orig+12) );
		printf("%*s     Cannon %u data: %12.4f %12.4f %12.4f %12.4f\n",
			indent,"",
			idx, bef4(data), bef4(data+4), bef4(data+8), bef4(data+12) );
	    }
	}
    }

    if ( st & STR_S_MENO )
	fprintf(f,"%*s- Known 'MenO' patches found.\n", indent,"" );

    if ( st & STR_S_UNKNOWN_PATCH )
	fprintf(f,"%*s- Unknown patches found.\n", indent,"" );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 PatchSTR()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint PatchTracks
(
    staticr_t		* str,		// valid pointer
    const u32		* offset_tab,	// null terminated table with offsets
    const u32		* track_tab,	// table with track indexes
    uint		base_index,	// base index of first track
    uint		n		// number of values in 'ref_tab'
)
{
    DASSERT(str);
    DASSERT(offset_tab);
    DASSERT(track_tab);
    DASSERT(n>0);
    DASSERT(n<=32);

    uint count = 0;
    const uint copy_size = n * sizeof(u32);

    uint i;
    u32 be_data[32];
    for ( i = 0; i < n; i++ )
	write_be32( be_data + i, base_index + track_tab[i] );
    TRACE_HEXDUMP16(0,0,be_data,copy_size);

    for ( ; *offset_tab; offset_tab++ )
    {
	DASSERT( *offset_tab < str->data_size );
	u8 * d = str->data + *offset_tab;
	if (memcmp(d,be_data,copy_size))
	{
	    count++;
	    memcpy(d,be_data,copy_size);
	}
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

static uint PatchVsBt
(
    staticr_t		*str,		// valid pointer
    bool		is_bt,		// false: VS, true: BT
    enum VsBtMode	mode,		// mode
    ccp			patch		// string to set
)
{
    DASSERT(str);
    DASSERT(str);

    uint count = 0;
    const u32 *tab = GetVsBtOffsetTab(str,is_bt);
    switch (mode)
    {
	case VSBT_OFF:
	    break;

	case VSBT_PATCH:
	    while (*tab)
	    {
		u8 *d = str->data + *tab++;
		if ( d[0] != patch[0] || d[1] != patch[1] )
		{
		    *d++ = patch[0];
		    *d   = patch[1];
		    count++;
		}
	    }
	    break;

	case VSBT_TEST:
	    {
		char c0 = is_bt ? 'b' : 'v';
		char c1 = '1';
		while (*tab)
		{
		    u8 *d = str->data + *tab++;
		    if ( d[0] != c0 || d[1] != c1 )
		    {
			*d++ = c0;
			*d   = c1;
			count++;
		    }
		    c1++;
		}
	    }
	    break;
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////
#if 0 // region specific patch functions not needed yet
///////////////////////////////////////////////////////////////////////////////

static uint PatchPAL
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data);
    DASSERT(str->data_size);
    DASSERT( str->mode == STR_M_PAL );

    uint count = 0;

    return count;
}

///////////////////////////////////////////////////////////////////////////////

static uint PatchUSA
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data);
    DASSERT(str->data_size);
    DASSERT( str->mode == STR_M_USA );

    uint count = 0;

    return count;
}

///////////////////////////////////////////////////////////////////////////////

static uint PatchJAP
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data);
    DASSERT(str->data_size);
    DASSERT( str->mode == STR_M_JAP );

    uint count = 0;

    return count;
}

///////////////////////////////////////////////////////////////////////////////

static uint PatchKOR
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data);
    DASSERT(str->data_size);
    DASSERT( str->mode == STR_M_KOR );

    uint count = 0;

    return count;
}

///////////////////////////////////////////////////////////////////////////////
#endif // region specific patch functions not needed yet
///////////////////////////////////////////////////////////////////////////////

static uint PatchRegion
(
    staticr_t		* str,		// valid pointer
    bool		is_battle	// true: is battle
)
{
    DASSERT(str);
    region_info_t ri;
    SetupRegionInfo(&ri,str,is_battle);

    uint count = 0;
    u8 temp[20];

    DASSERT(ri.opt_region);
    if ( *ri.opt_region == SREG_RESTORE )
    {
	const region_patch_t *rp;
	for ( rp = ri.patch_tab; rp->orig; rp++ )
	{
	    if ( rp->patch_mode & RPAT_TEST && !*ri.opt_x )
		continue;

	    write_be32(temp,rp->orig);
	    u8 *d = str->data + rp->offset[str->mode - STR_ZBI_FIRST];
	    if (memcmp(d,temp,4))
	    {
		memcpy(d,temp,4);
		count++;
	    }
	}
    }
    else if ( *ri.opt_region >= 0 )
    {
	uint val = *ri.opt_region & 0xffff;
	uint add = *ri.opt_t;

	const region_patch_t *rp;
	for ( rp = ri.patch_tab; rp->orig; rp++ )
	{
	    if ( rp->patch_mode & RPAT_TEST && !*ri.opt_x )
		continue;

	    write_be32( temp, rp->patch | ( rp->patch_mode & RPAT_FF ? val & 0xff : val ) );
	    val += add;
	    u8 *d = str->data + rp->offset[str->mode - STR_ZBI_FIRST];
	    if (memcmp(d,temp,4))
	    {
		memcpy(d,temp,4);
		count++;
	    }
	}
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

uint PatchSTR
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    if (!str->data_size)
	return 0;
    DASSERT(str->data);

    if (str->is_dol)
	return PatchDOL(str);

    if (!CheckSizeSTR(str))
	return 0;

    uint count = 0;


    //--- patch tracks and arenas

    if (opt_tracks)
    {
	const u32 *tab = GetTrackOffsetTab(str);
	if (tab)
	    count += PatchTracks( str, tab, track_pos, MKW_TRACK_BEG, MKW_N_TRACKS );
    }

    if (opt_arenas)
    {
	const u32 *tab = GetArenaOffsetTab(str);
	if (tab)
	    count += PatchTracks( str, tab, arena_pos, MKW_ARENA_BEG, MKW_N_ARENAS );
    }


    //--- patch region

    count += PatchRegion(str,false);
    count += PatchRegion(str,true);


    //--- patch all-ranks

    if ( opt_all_ranks )
    {
	ccp p1, p2;
	if ( opt_all_ranks < 0 )
	    p1 = p2 = all_ranks_orig;
	else
	{
	    p1 = all_ranks_patch1;
	    p2 = all_ranks_patch2;
	}

	const u32 *tab = GetAllRanksOffsetTab(str);
	if (tab)
	{
	    while (*tab)
	    {
		u8 *d = str->data + *tab++;
		if (memcmp(d,p1,all_ranks_size))
		{
		    memcpy(d,p1,all_ranks_size);
		    count++;
		    ccp temp = p1;
		    p1 = p2;
		    p2 = temp;
		}
	    }
	}
    }


    //--- patch vs & bt

    count += PatchVsBt(str,false,patch_vs_mode,patch_vs_str);
    count += PatchVsBt(str,true, patch_bt_mode,patch_bt_str);


    //--- patch versus points

    if (opt_points_used)
    {
	const u32 offset = GetVersusPointsOffset(str->mode);
	if (offset)
	{
	    u8 *data = str->data + offset;
	    if (memcmp(data,MkwPointsTab,12*12))
	    {
		count++;
		memcpy(data,MkwPointsTab,12*12);
	    }
	}
    }


    //--- patch cannon params

    u32 offset = GetCannonOffset(str);
    if (offset)
    {
	u8 *data = str->data + offset;
	uint i;
	for ( i = 0; i < CANNON_N; i++ )
	{
	    const cannon_param_t *cp = cannon_param+i;
	    if (cp->valid)
	    {
		uint j;
		for ( j = 0; j < CANNON_N_PARAM; j++, data += sizeof(float) )
		    write_bef4(data,cp->param[j]);
	    }
	    else
		data += CANNON_SIZE;
	}
    }


    //-- patch 'Menu*' -> 'MenO*'

    if ( opt_meno )
    {
	const u32 *optr = GetMenuOffsetTab(str->mode);
	if (optr)
	{
	    const int *idx;
	    for ( idx = meno_patch_idx; *idx >= 0; idx++ )
	    {
		u8 *d = str->data + optr[*idx] + 13; // 13 = 'O' offset
		if ( *d != 'O' )
		{
		    *d = 'O';
		    count++;
		}
	    }
	}
    }


    //--- patch https

    count += PatchHTTPS(str);


 #if 0 // region specific patch functions not needed yet

    //--- patch version specific

    str->str_status = STR_S_ANALYZED;
    switch (str->mode)
    {
	case STR_M_PAL:
	    count += PatchPAL(str);
	    break;

	case STR_M_USA:
	    count += PatchUSA(str);
	    break;

	case STR_M_JAP:
	    count += PatchJAP(str);
	    break;

	case STR_M_KOR:
	    count += PatchKOR(str);
	    break;

	default:
	    break;
    }
 #endif

    return count + PatchByListWPF(str);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CleanDOL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool CleanDol ( staticr_t * str ) // return TRUE if dol data modified.
{
    AnalyzeDOL(str,true);
    if ( str->dol_status & DOL_S_ORIG )
    {
	str->dol_status = 0;
	return 0;
    }

    uint sect_list;
    dol_header_t *dol_head = (dol_header_t*)str->data;

    uint new_size = RemoveDolSections(dol_head,0x7f83,&sect_list,logging>2?stdlog:0);
    bool stat = new_size > 0;
    noPRINT("SIZE: %x -> %x / sect=%x\n",str->data_size,new_size,sect_list);
    if (stat)
	str->data_size = new_size;


    //--- fix entry point

    const dol_sect_info_t *orig = GetInfoDOL(str->mode);
    DASSERT(orig);
    if ( orig[DOL_IDX_ENTRY].addr != ntohl(dol_head->entry_addr) )
    {
	stat = true;
	dol_head->entry_addr = htonl(orig[DOL_IDX_ENTRY].addr);
    }


    //--- restore HTTPS links

    PatchHTTPS_HTTP(str,HTTPS_RESTORE);


    //--- terminate

    str->dol_status = 0;
    str->dol_cleaned = true;
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PatchDOL()			///////////////
///////////////////////////////////////////////////////////////////////////////
// GCH support

#define GCT_ADDRESS	0x80001800u
#define	GCT_MAX		0x80003000u
#define GCT_MM_BASE	(GCT_ADDRESS-sizeof(gch_header_t))

static MemMap_t gct_data_mem	= {0};	// memory map of GCT section
static GrowBuffer_t gecko_data	= {0};
static GrowBuffer_t wcode_job	= {0};
static GrowBuffer_t wcode_data	= {0};


static bool use_gct_loader = false;

///////////////////////////////////////////////////////////////////////////////
// patched by

#define STD_PATCHED_BY  WSTRT_SHORT " v" VERSION " r" REVISION
static char opt_patched_by[PATCHED_BY_SIZE+1] = STD_PATCHED_BY;

patched_by_t patched_by_mode = PBY_AUTO;

//-----------------------------------------------------------------------------

int ScanOptPBMode ( ccp arg )
{
    if ( !arg || !*arg )
    {
	patched_by_mode = PBY_AUTO;
	return 0;
    }

    static const KeywordTab_t tab[] =
    {
	{ PBY_OFF,	"OFF",		0,		0 },
	{ PBY_RESET,	"RESET",	0,		0 },
	{ PBY_WIIMMFI,	"WIIMMFI",	0,		0 },
	{ PBY_AUTO,	"AUTO",		"DEFAULT",	0 },
	{ PBY_ALWAYS,	"ALWAYS",	0,		0 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	patched_by_mode = cmd->id;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid mode for option --pb-mode: '%s'\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(TEST) || defined(DEBUG) //|| HAVE_WIIMM_EXT
 static void TraceGCT ( ccp info )
 {
    fprintf(stdlog,"%s#GCT: gecko=%u/%u, wc-job=%u/%u, wc-data=%u/%u%s%s%s\n",
	collog->b_cyan,
	gecko_data.used,   gecko_data.size,
	wcode_job.used,    wcode_job.size,
	wcode_data.used,   wcode_data.size,
	info ? " : " : "",
	info ? info  : "",
	collog->reset );
 }
#else
 static inline void TraceGCT ( ccp info ) {}
#endif

///////////////////////////////////////////////////////////////////////////////

static void InititializeGCT()
{
    static bool done = false;
    if (!done)
    {
	done = true;

	InitializeMemMap(&gct_data_mem);
	InitializeGrowBuffer(&gecko_data,MiB);
	InitializeGrowBuffer(&wcode_job,MiB);
	InitializeGrowBuffer(&wcode_data,MiB);
	gecko_data.grow_size = 0x1000;
	wcode_job.grow_size  = 0x0100;
	wcode_data.grow_size = 0x0400;

	if (logging>2) TraceGCT("Init");
    }
}

///////////////////////////////////////////////////////////////////////////////

static void ResetGCT()
{
    InititializeGCT();

    ResetMemMap(&gct_data_mem);
    ClearGrowBuffer(&gecko_data);
    ClearGrowBuffer(&wcode_job);
    ClearGrowBuffer(&wcode_data);
}

///////////////////////////////////////////////////////////////////////////////

static enumError AddSectionHelper2
(
    // return ERR_OK | ERR_WARNING of created with warnings | ERR_ERROR on abort
    staticr_t		*str,		// valid pointer
    const wch_segment_t	*seg,		// segement info, local endian, data+size ignored
    const u8		*seg_data,	// segement data
    uint		seg_size,	// segement data
    ccp			fname		// filename for messages
)
{
    //--- patching possible?

    const u32 patch_addr = GetVbiAddress(str);
    if (!patch_addr)
	return 0;


    //--- check overlaps

    dol_header_t *dh = (dol_header_t*)str->data;
    const uint overlap = CountOverlappedDolSections(dh,seg->addr,seg_size);
    if (overlap)
    {
	if (!force_count)
	{
	    ERROR0(ERR_WARNING,
		"New section would overlap %u existing section%s -> ignore: %s\n",
		overlap, overlap == 1 ? "" : "s", fname );
	    return ERR_ERROR;
	}

	ERROR0(ERR_WARNING,
		"New section overlaps %u existing section%s: %s\n",
		overlap, overlap == 1 ? "" : "s", fname );
    }


    //--- find first unused section

    int dol_seg_type;
    switch (seg->type)
    {
	case 't': dol_seg_type = 0; break; // Any segment, TEXT first
	case 'T': dol_seg_type = 1; break; // Only TEXT segment
	case 'D': dol_seg_type = 2; break; // Only DATA segment
	default:  dol_seg_type = 3; break; // Any segment, DATA first (alias 'd')
    }

    dol_sect_info_t di;
    if (!FindFirstFreeDolSection(&di,dh,dol_seg_type))
    {
	ERROR0(ERR_WARNING,
	    "DOL: No free section available: %s\n", str->fname );
	return ERR_ERROR;
    }
    PRINT("NEW SECTION: %s\n",di.name);


    //--- append seg_data to end of dol

    uint offset = str->data_size;
    str->data_size += seg_size;
    str->data = REALLOC(str->data,str->data_size);
    dh = (dol_header_t*)str->data;
    memcpy( str->data + offset, seg_data, seg_size );


    //--- create section

    PRINT(" - create section %s [%08x..%08x], f-off 0x%06x, size 0x%x\n",
		    di.name, seg->addr, seg->addr+seg_size, offset, seg_size );

    if ( verbose > 0 )
	fprintf(stdlog,
	    "- Create section %s [%08x..%08x], file offset 0x%06x, size 0x%x\n",
	    di.name, seg->addr,  seg->addr+seg_size, offset, seg_size );

    dh->sect_off [di.section] = htonl(offset);
    dh->sect_addr[di.section] = htonl(seg->addr);
    dh->sect_size[di.section] = htonl(seg_size);


    //--- patch entry point

    u32 addr = seg->main_entry_old;
    if (addr)
    {
	const u32 store = GetDolOffsetByAddr(dh,addr,sizeof(u32),0);
	if (store)
	    memcpy(str->data+store,&dh->entry_addr,sizeof(u32));
    }

    if (seg->main_entry)
    {
	if ( verbose >= 1 )
	    fprintf(stdlog,
		"- Change entry point from %08x to %08x\n",
		ntohl(dh->entry_addr), seg->main_entry );
	write_be32(&dh->entry_addr,seg->main_entry);
    }


    //--- patch VBI entry point

    if (seg->vbi_entry)
    {
	const u32 offset = GetDolOffsetByAddr(dh,patch_addr,sizeof(u32),0);
	if (!offset)
	{
	    ERROR0(ERR_WARNING,
		"Can't patch DOL at address 0x%08x: %s\n",
		patch_addr, str->fname );
	}
	else
	{
	    u32 addr = ntohl(seg->vbi_entry_old);
	    if (addr)
	    {
		const u32 store = GetDolOffsetByAddr(dh,addr,sizeof(u32),0);
		if (!store)
		{
		    ERROR0(ERR_WARNING,
			"Can't store old VBI at DOL address 0x%08x: %s\n",
			seg->vbi_entry_old, str->fname );
		}
		else
		    memcpy(str->data+store,str->data+offset,sizeof(u32));
	    }

	    const u32 code = ( seg->vbi_entry - patch_addr ) & 0xffffff | 0x4b000000;
	    PRINT("PATCH addr %08x off %08x from %08x to %08x\n",
		patch_addr, offset, be32(str->data+offset), code );
	    if ( verbose > 0 )
		fprintf(stdlog,
			"- Patch address %08x (off %08x, VBI) from %08x to %08x\n",
			patch_addr, offset, be32(str->data+offset), code );
	    write_be32(str->data+offset,code);
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct gct_sect_t
{
    u16 off;	// source offset
    u16 size;	// size of section
    u16 idx;	// initial section index
    u16 dest;	// dest address of section (add 0x8000000 for real address)
}
gct_sect_t;

//-----------------------------------------------------------------------------

static int sort_sect_list_by_size
	( const gct_sect_t * a, const gct_sect_t * b )
{
    return (int)b->size - (int)a->size;
}

//-----------------------------------------------------------------------------

static int sort_sect_list_by_dest
	( const gct_sect_t * a, const gct_sect_t * b )
{
    return (int)a->dest - (int)b->dest;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct cheat_code_loader_t
{
    u32 entry;		// entry point => ASM: bl start
    u32 dummy_04;	// ...

    //--- data, see cheat-code-loader.sh

    u32 return_addr;	// absolute return address
    u32 var_version;	// 'vars' version of current code loader
    u32 space;		// needed space, must not < source_len

    u32 patch_addr;	// address of patch list, relative to 'vars'
    u32 data_addr;	// data address, relative to 'vars'
    u32 data_end;	// data-end address, relative to 'vars'
    u32 gecko_off;	// offset of first gecko code relative to data

    u32 verify_addr;	// address to verify own codehandler
    u32 verify_val;	// use this to verify & to decode
    u32 ch_patch_1;	// abs address for first codehandler patch
    u32 ch_patch_2;	// abs address for second codehandler patch
    u32 deny_user_gch;	// 1: skip patching on !rSZS
    u32 info_flags;	// info flags for NAS (8 bits)
}
__attribute__ ((packed)) cheat_code_loader_t;

#define CCL_VAR_OFFSET offsetof(cheat_code_loader_t,return_addr)

///////////////////////////////////////////////////////////////////////////////

typedef enum { CHT_NONE, CHT_ONLY, CHT_FULL } code_handler_t;

//-----------------------------------------------------------------------------

static const u8 loader_asm_bz2[] =
{
  // decompressed len: 0x338 = 824 bytes
  0x00,0x00,0x03,0x38,

  // bzipped data
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x9d,0xf6, 0x23,0xf1,0x00,0x01,
  0x20,0xff,0xff,0xfd, 0xde,0xc7,0xf6,0xc5, 0xef,0xf5,0xff,0x64, 0x5b,0xef,0xd0,0x6a,
  0xdc,0xc5,0xd7,0xfc, 0xde,0x79,0xe0,0x53, 0xd6,0x51,0x10,0x55, 0xc3,0x41,0x65,0x45,
  0x84,0x65,0x44,0xc0, 0x01,0xf5,0xc2,0xca, 0xd2,0x1a,0xa6,0x8c, 0xa6,0x90,0xf5,0x3d,
  0x27,0xa9,0x90,0x00, 0xd0,0x06,0x26,0x83, 0xd2,0x00,0x69,0x88, 0x1a,0x00,0x19,0x00,
  0x1a,0x1a,0x32,0x01, 0xa0,0xf4,0x41,0x90, 0x32,0x00,0xc8,0x11, 0x34,0x41,0x1a,0x4f,
  0xd5,0x3d,0x27,0x94, 0xf5,0x32,0x1e,0xa0, 0x19,0x34,0x19,0x00, 0xd0,0xf4,0x80,0x00,
  0x00,0x00,0x00,0x00, 0x00,0x00,0x0d,0x00, 0xd0,0x06,0xd4,0x41, 0x80,0x4c,0x46,0x09,
  0x93,0x08,0x30,0x98, 0x98,0x13,0x00,0x43, 0x08,0xc0,0x43,0x02, 0x60,0x4c,0x26,0x08,
  0xc0,0x00,0x04,0xd0, 0xc2,0x62,0x69,0x81, 0x03,0x4d,0x42,0x98, 0x50,0x09,0xb4,0x3d,
  0x40,0x69,0xa8,0xc9, 0x80,0x68,0x00,0x01, 0x32,0x34,0xd1,0xa6, 0x00,0x11,0x81,0xa0,
  0x32,0x23,0x00,0x00, 0x08,0x69,0x91,0xe9, 0x0c,0xa2,0x54,0xaf, 0x84,0x8f,0x0a,0x52,
  0x54,0xba,0xa7,0xd1, 0x59,0x0d,0x00,0x4b, 0x69,0xdf,0xb0,0xb0, 0xb3,0xd3,0x4a,0x1c,
  0xa2,0x85,0xc9,0x8f, 0xc7,0xb6,0x9e,0x7e, 0x12,0x2b,0x1d,0x13, 0x3f,0x9e,0x9e,0x4b,
  0xb0,0x31,0x30,0x83, 0x55,0x67,0x43,0xde, 0x80,0x53,0x40,0x4f, 0xcd,0x7b,0x06,0x1c,
  0x04,0xd7,0x3e,0x0c, 0xc5,0xd2,0x66,0x27, 0xf5,0x19,0x65,0xeb, 0x81,0x01,0x48,0xa9,
  0x51,0x42,0x9b,0xba, 0x56,0x19,0x92,0x45, 0x52,0x6e,0x22,0xe0, 0xb5,0x97,0x2c,0x23,
  0xda,0x75,0x1c,0x16, 0x2e,0xb1,0x88,0x8e, 0x91,0xa0,0xcf,0x5a, 0xc9,0x5e,0x99,0x27,
  0xe1,0x52,0x18,0x16, 0x42,0x60,0x17,0x49, 0x28,0xc2,0x8e,0x18, 0x12,0x10,0xda,0x46,
  0x01,0x76,0x36,0xd4, 0x07,0xe3,0x9d,0x42, 0x5c,0xc2,0xba,0x55, 0x7f,0xbd,0x14,0x02,
  0x70,0x29,0x37,0x17, 0xf6,0xda,0x50,0x5b, 0x05,0xa1,0xb9,0x06, 0xcd,0xb0,0xde,0x93,
  0x88,0x8b,0x0f,0x9d, 0x2a,0x4d,0xc8,0x6b, 0x7c,0xf1,0x01,0xf2, 0x14,0x6b,0x11,0x48,
  0x27,0x7d,0xb8,0xab, 0x43,0x15,0x5d,0x46, 0x21,0x1f,0x70,0x56, 0x77,0x4a,0x82,0xcc,
  0x8b,0xf1,0xc9,0xc0, 0x73,0xde,0x98,0x74, 0x4b,0x96,0x39,0xb2, 0x6b,0x99,0xee,0x23,
  0x0d,0xf0,0x69,0x16, 0x12,0x26,0xf7,0x2c, 0xe8,0x60,0x78,0xfa, 0x13,0x3b,0x56,0xc8,
  0xaa,0x69,0x82,0x3e, 0xc8,0x2b,0xee,0xad, 0xb9,0x65,0x8e,0x44, 0x98,0x27,0x77,0xcd,
  0xc5,0xcf,0x48,0x17, 0x93,0x04,0x2a,0x82, 0x96,0xa2,0x6d,0x64, 0x1c,0x03,0x7d,0x91,
  0xd1,0x4f,0x38,0x78, 0x7f,0x2a,0xfb,0x03, 0xe6,0x2f,0xe3,0x9f, 0xe5,0x0b,0x36,0x58,
  0x8c,0x70,0xba,0x9d, 0x3b,0xf3,0x4b,0xfe, 0xb9,0x53,0x6a,0x4c, 0xe8,0x6c,0xb9,0xfc,
  0x32,0xa3,0x13,0xc9, 0xed,0x29,0x10,0x4c, 0x48,0xe1,0x32,0x81, 0x0a,0x24,0x80,0x20,
  0x15,0x82,0x0d,0x89, 0x26,0xa1,0x29,0x66, 0x08,0x0a,0xd0,0x9e, 0xe0,0xce,0x50,0x7b,
  0xc5,0xfb,0x95,0x66, 0x42,0x65,0x84,0xda, 0xdf,0x37,0x52,0x53, 0x23,0x06,0x09,0x39,
  0xf6,0x1f,0x63,0xf1, 0x0b,0x13,0xd6,0x9c, 0xa2,0x63,0x2d,0xef, 0xa2,0xb2,0x8d,0xd2,
  0xa0,0xdd,0xe8,0x39, 0x9e,0xf5,0x71,0xa4, 0xd1,0xc5,0x74,0x11, 0x11,0xbb,0x42,0x20,
  0x53,0x92,0xff,0xc5, 0x36,0x67,0x4e,0xa4, 0x1b,0x55,0x95,0xef, 0x3a,0x30,0x12,0x85,
  0xea,0xe2,0x75,0xbf, 0x1a,0xb8,0xc8,0xa8, 0x12,0x2d,0x19,0x17, 0x91,0xdd,0xa7,0x23,
  0xbb,0xe7,0x18,0xb4, 0x61,0x78,0xa8,0xc4, 0x3b,0x84,0x5a,0x2d, 0x35,0x5f,0xac,0xd8,
  0x82,0xae,0x21,0x8e, 0x23,0x7d,0xf3,0x93, 0x2d,0xff,0x7b,0xbc, 0xbd,0x42,0x65,0xfc,
  0x1e,0xf9,0x2d,0xf9, 0x69,0x5a,0x12,0x08, 0x20,0x90,0x41,0x00, 0xc9,0x62,0xd2,0x88,
  0xc2,0x2a,0xa2,0x95, 0xc6,0x52,0xab,0x53, 0x31,0xb5,0xae,0x62, 0x36,0x07,0x94,0x8f,
  0x05,0x87,0x98,0x4a, 0x26,0x1a,0x0f,0x1f, 0xdb,0x2e,0x20,0xbb, 0x00,0x3c,0xb8,0x74,
  0x4c,0xb2,0x04,0x8f, 0x98,0xec,0x1d,0x43, 0x10,0x97,0x08,0x33, 0x05,0x0a,0xa0,0x48,
  0x23,0x04,0x50,0xc0, 0xfd,0x68,0x2e,0x24, 0x97,0xb1,0x4c,0xac, 0xb6,0x12,0xd4,0x00,
  0x65,0xdb,0x5d,0x74, 0x19,0x20,0x9e,0x58, 0xc2,0x10,0x12,0x00, 0x2b,0x52,0x00,0xea,
  0xa8,0xe0,0x54,0x84, 0xd0,0x7e,0x52,0x38, 0x08,0x2a,0x82,0x40, 0x59,0x3c,0xde,0xc5,
  0x08,0x1c,0x8f,0x4a, 0x79,0x27,0x98,0xbb, 0x92,0x29,0xc2,0x84, 0x84,0xef,0xb1,0x1f,
  0x88,
  // 0x2e1 = 737 Bytes
};

static BZ2Manager_t loader_asm_mgr
	= { loader_asm_bz2, sizeof(loader_asm_bz2), 0, 0 };

///////////////////////////////////////////////////////////////////////////////

static enumError SetupCodeLoader
(
    // return ERR_OK | ERR_WARNING of created with warnings,
    // ERR_ERROR on abort, ERR_NOTHING_TO_DO if nothing to do

    staticr_t		*str,		// valid pointer
    const wch_segment_t	*seg,		// NULL or segement info, seg->data is ignored
    u8			*code_data,	// NULL or code data
    uint		code_size,	// NULL or size of code data
    uint		cheat_offset,	// offset first cheat code
    code_handler_t	ch_type,	// code handler type
    ccp			fname		// filename for messages
)
{
    ASSERT( code_size >= cheat_offset );
    const uint total_data_size	= wcode_job.used + wcode_data.used
				+ code_size - cheat_offset;

 #if defined(TEST) || defined(DEBUG) // || HAVE_WIIMM_EXT
    if (logging>2)
	fprintf(stderr,
		"SetupCodeLoader() %d+%d+%d = %d, c-off=0x%x, ch-type=%d, seg=%d => %s\n",
		code_size, wcode_job.used, wcode_data.used, total_data_size,
		cheat_offset, ch_type, seg!=0,
		total_data_size ? "\e[41;37;1m CREATE WIIMMFI CODE \e[0m" : "quit" );
 #endif

    if (!total_data_size)
	return ERR_NOTHING_TO_DO;


    //--- data

    GrowBuffer_t data;
    InitializeGrowBuffer(&data,MiB);
    data.grow_size = 0x1000;


    //--- code handler

    const u32 security_off = GCT_REG_OFFSET + 16; // gecko register 4
    const u32 security_val = 0xdc020463;

    enumError err = ERR_OK;
    if ( code_size )
    {
	ASSERT(code_data);
	write_be32(code_data+security_off,security_val);
	write_be32(code_data+security_off+8,security_val);
	write_be64(code_data+cheat_offset,GCT_TERM_NUM);
	err = AddSectionHelper2(str,seg,code_data,cheat_offset+8,fname);
	if (err)
	    goto abort;
	code_data += cheat_offset;
	code_size -= cheat_offset;
	write_be64(code_data,GCT_MAGIC8_NUM);
	str->dol_flags[DPF_CHEAT] = 'l';
    }


    //--- setup data buffer

    DecodeBZIP2Manager(&loader_asm_mgr);
    InsertGrowBuffer(&data,loader_asm_mgr.data,loader_asm_mgr.size);
    cheat_code_loader_t ccl;
    memcpy(&ccl,data.ptr,sizeof(ccl));
    ccl.info_flags	= htonl(str->dol_info_flags);

    uint code_offset	= loader_asm_mgr.size - CCL_VAR_OFFSET;
    uint deny_user_gch	= opt_allow_user_gch < OFFON_AUTO;
    if (wcode_job.used)
    {
	ccl.patch_addr	= htonl(code_offset);
	InsertGrowBuffer(&data,wcode_job.ptr,wcode_job.used);
	InsertGrowBuffer(&data,EmptyString,4);
	code_offset += wcode_job.used + 4;
	deny_user_gch = opt_allow_user_gch <= OFFON_AUTO;
    }
    str->dol_flags[DPF_ALLOW_USER] = deny_user_gch ? 'd' : 'a';
    if ( deny_user_gch && code_size )
    {
	str->dol_flags[DPF_ALLOW_USER] = 's';
	deny_user_gch = 2;
    }
    ccl.deny_user_gch	= htonl(deny_user_gch);
    ccl.data_addr	= htonl(code_offset);
    ccl.gecko_off	= htonl( wcode_data.used );

    if (wcode_data.used)
	InsertGrowBuffer(&data,wcode_data.ptr,wcode_data.used);

    if (code_size)
    {
	InsertGrowBuffer(&data,code_data,code_size);
	ccl.verify_addr = htonl(seg->addr+security_off);
    }
    ccl.data_end	= htonl(data.used-CCL_VAR_OFFSET);
    const u32 space	= data.used - CCL_VAR_OFFSET - code_offset;
    ccl.space		= htonl( space > opt_gct_space ? space : opt_gct_space );
    ccl.verify_val	= htonl(security_val);

    if ( ch_type == CHT_FULL )
    {
	ccl.ch_patch_1	= htonl(seg->addr+0x04DC);
	ccl.ch_patch_2	= htonl(seg->addr+0x0758);
    }
    else if ( ch_type == CHT_ONLY )
	ccl.ch_patch_1	= htonl(seg->addr+0x0104);

    memcpy(data.ptr,&ccl,sizeof(ccl));


    //--- create segment

    wch_segment_t dseg;
    memset(&dseg,0,sizeof(dseg));
    dseg.type = 't';
    dseg.size = data.used;
    dseg.addr = opt_gct_addr < 0x80000000
		? FindFreeBssSpaceDOL((dol_header_t*)str->data,dseg.size,32,0)
		: opt_gct_addr;
    dseg.main_entry = dseg.addr;
    dseg.main_entry_old = dseg.addr + offsetof(cheat_code_loader_t,return_addr);

    u32 *ptr = (u32*)( data.ptr + loader_asm_mgr.size );
    u32 *end = (u32*)( data.ptr + data.used );
    u32 key = security_val;
    while ( ptr < end )
    {
	const u32 orig = be32(ptr);
	const u32 val = orig ^ key;
	write_be32(ptr, val << 19 | val >> 13 );
	key += orig >> 5;
	ptr++;
    }

    err = AddSectionHelper2(str,&dseg,data.ptr,dseg.size,fname);

    //--- clean
abort:
    ResetGrowBuffer(&data);
    ClearGrowBuffer(&wcode_job);
    ClearGrowBuffer(&wcode_data);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError AddSectionHelper
(
    // return ERR_OK | ERR_WARNING of created with warnings | ERR_ERROR on abort
    staticr_t		*str,		// valid pointer
    const wch_segment_t	*seg,		// segement info, seg->data is ignored
    const u8		*seg_data,	// segement data
    ccp			fname		// filename for messages
)
{
    //--- patching possible?

    const u32 patch_addr = GetVbiAddress(str);
    if (!patch_addr)
	return 0;


    //--- check presence of 'codehandleronly' or 'codehandler'
    //--- and optional replace 'codehandleronly' by 'codehandler'

    u8 *new_data = 0;
    uint seg_size = seg->size;

    code_handler_t ch_type = CHT_NONE;
    uint cheat_offset = 0;

    BZ2Manager_t *cho = &codehandleronly_mgr;
    DecodeBZIP2Manager(cho);
    if ( cho->size > sizeof(gch_header_t) )
    {
	const u8 *cho_data = cho->data + sizeof(gch_header_t);
	const uint cho_size = cho->size - sizeof(gch_header_t);
	if ( seg_size >= cho_size && !memcmp(cho_data,seg_data,cho_size) )
	{
	    ch_type = CHT_ONLY;
	    cheat_offset = cho_size;

	    if (opt_full_gch)
	    {
		PRINT("\e[35;1m Only: %u >= ( %u -> %u -> %u )\e[0m\n",
			seg_size, codehandleronly_mgr.src_size,
			codehandleronly_mgr.size, cho_size );

		BZ2Manager_t *chf = &codehandler_mgr;
		DecodeBZIP2Manager(chf);
		if ( chf->size > sizeof(gch_header_t) )
		{
		    if ( verbose > 0 )
			fprintf(stdlog,
			    "- Replace 'codehandleronly' by 'codehandler'\n");

		    const u8 *chf_data = chf->data + sizeof(gch_header_t);
		    const uint chf_size = chf->size - sizeof(gch_header_t);
		    const uint new_size = seg_size - cho_size + chf_size;
		    new_data = MALLOC(new_size);
		    memcpy( new_data, chf_data, chf_size );
		    memcpy( new_data+chf_size, seg_data+cho_size, seg_size-cho_size );
		    seg_data = new_data;
		    seg_size = new_size;

		    ch_type = CHT_FULL;
		    cheat_offset = chf_size;
		}
	    }
	}
    }

    if ( ch_type == CHT_NONE )
    {
	BZ2Manager_t *chf = &codehandler_mgr;
	DecodeBZIP2Manager(chf);
	if ( chf->size > sizeof(gch_header_t) )
	{
	    const u8 *chf_data = chf->data + sizeof(gch_header_t);
	    const uint chf_size = chf->size - sizeof(gch_header_t);
	    if ( seg_size >= chf_size && !memcmp(chf_data,seg_data,chf_size) )
	    {
		ch_type = CHT_FULL;
		cheat_offset = chf_size;
	    }
	}
    }


    //--- find and remove separators

    enum { MAX_SECT = 200 };
    gct_sect_t	sect_list[MAX_SECT+1],
		*sect = sect_list,
		*sect_end = sect_list + MAX_SECT;
    memset(sect_list,0,sizeof(sect_list));

    if ( cheat_offset > sizeof(gch_header_t) && opt_gct_scan_sep )
    {
	if (!new_data)
	{
	    // we need modifiable data!
	    new_data = MEMDUP(seg_data,seg_size);
	    seg_data = new_data;
	}

	u8 *src  = new_data + cheat_offset;
	u8 *dest = src;
	u8 *end  = new_data + seg_size;

	while ( src + 8 <= end )
	{
	    PRINT(">> LINE %3zd => %3zd : %08x %08x\n",
		( src - new_data - cheat_offset ) / 8,
		( dest - new_data - cheat_offset ) / 8,
		 be32(src), be32(src+4) );

	    uint copy_lines = 0;
	    if ( be32(src) == 0xf0000000 && !be32(src+4) )
	    {
		const u32 off = dest - new_data;
		sect->size = off - sect->off;
		sect++;
		sect->idx = sect - sect_list;
		sect->off = off;
		sect->size = 8;
		*(u64*)dest = *(u64*)src;
		src += 8;
		dest += 8;
		break;
	    }
	    else if ( be32(src) == GCT_SEP_NUM1 && !be16(src+4) )
	    {
		if ( opt_gct_sep )
		{
		    const u32 off = dest - new_data;
		    if ( sect->off != off && sect < sect_end )
		    {
			sect->size = off - sect->off;
			sect++;
			sect->idx = sect - sect_list;
			sect->off = off;
			sect->size = 0;
		    }
		}

		PRINT("\t>> SEP#%zu found: %08x %08x\n",
				sect-sect_list, be32(src),be32(src+4) );
		copy_lines = be16(src+6);
		src += 8;
	    }
	    else if ( *src == 0xc0 || *src == 0xc2 || *src == 0xd0 || *src == 0xd2 )
	    {
		if ( opt_gct_asm_sep )
		{
		    const u32 off = dest - new_data;
		    if ( sect->off != off && sect < sect_end )
		    {
			sect->size = off - sect->off;
			sect++;
			sect->idx = sect - sect_list;
			sect->off = off;
			sect->size = 0;
		    }
		}

		PRINT(">> ASM#%zu found: %08x %08x\n",
				sect-sect_list, be32(src), be32(src+4) );
		copy_lines = be32(src+4) + 1;
	    }
	    else if ( ( *src == 0x66 || *src == 0x68 )
		&& ( src[1] == 0x00 || src[1] == 0x10 || src[1] == 0x20 ) )
	    {
		PRINT(">> GOTO/GOSUB#%zu found: %08x %08x\n",
				sect-sect_list, be32(src), be32(src+4) );
		s16 go = be16(src+2);
		if ( go >= 0 )
		    copy_lines = go+1;
	    }
	    else
	    {
		*(u64*)dest = *(u64*)src;
		src += 8;
		dest += 8;
		continue;
	    }

	    if ( copy_lines > 0 )
	    {
		// use it this way to detect bad large values
		const uint max = ( end - src ) / 8;
		if ( copy_lines > max )
		     copy_lines = max;
		copy_lines *= 8; // now we have copy_bytes!

		memmove(dest,src,copy_lines);
		src  += copy_lines;
		dest += copy_lines;
	    }
	}

	//PRINT(">>> %zd %zd /%zd\n",dest-new_data,src-new_data,end-new_data);
	if ( src >= end )
	    seg_size = dest - new_data;
	else if ( src > dest )
	    memset(dest,0,src-dest);
    }
    const int n_sect = sect - sect_list;


    //--- move GCT part

    str->dol_flags[DPF_CHEAT] = 'c';

    enumError err = ERR_OK;
    if ( opt_gct_move >= OFFON_ON && cheat_offset >= 0x100 )
    {
      use_loader:;
	if (!new_data)
	{
	    // we need modifiable data!
	    new_data = MEMDUP(seg_data,seg_size);
	    seg_data = new_data;
	}

	err = SetupCodeLoader(str,seg,new_data,seg_size,cheat_offset,ch_type,fname);
	goto abort;
    }


    //--- split GCT part

    const u32 off_end = seg->addr + seg_size;
    PRINT("N=%d, of=%08x\n",n_sect,off_end);
    if ( n_sect > 2
	&& seg->addr + cheat_offset < 0x80002ff0
	&& off_end >= 0x80003000 && off_end < 0x80004000 )
    {
     #ifdef TEST0
	{
	    uint i;
	    for ( i = 0, sect = sect_list; i <= n_sect; i++, sect++ )
		PRINT(" -> SECT %2u: %4x .. %4x, size %4x\n",
		    i, sect->off, sect->off + sect->size, sect->size );
	}
     #endif

	bool sorted = false;
     restart:;
	u8 data[0x4000];
	memset(data,0,sizeof(data));

	const u32 base_addr = seg->addr - 0x80000000;
	memcpy(data+base_addr,seg_data,cheat_offset+8);

	struct data_t { u32 used; u32 size; u8 *data; } data_tab[] =
	{
	    { cheat_offset+8, 0x2ff8 - base_addr, data + base_addr },
	    {              0, 0x37f8 - 0x3200,    data + 0x3200 },
	    {              0, 0x3ff8 - 0x3880,    data + 0x3880 },
	    {0,0,0}
	};

	uint i;
	struct data_t *dp;
	bool fail = false;

	if ( opt_gct_list > 1 )
	{
	    fprintf(stdlog,"\nSummary of detected GCT sections:\n");
	    for ( i = 1, sect = sect_list+1; i < n_sect; i++, sect++ )
		fprintf(stdlog,"   S%02u: %04x .. %04x, size %04x = %4u/dec\n",
			sect->idx, sect->off, sect->off+sect->size,
			sect->size, sect->size );
	}

	for ( i = 1, sect = sect_list+1; i < n_sect; i++, sect++ )
	{
	    bool done = false;
	    for ( dp = data_tab; dp->data; dp++ )
		if ( sect->size <= dp->size - dp->used )
		{
		    memcpy(dp->data+dp->used,seg_data+sect->off,sect->size);
		    sect->dest = dp->data + dp->used - data;
		    dp->used += sect->size;
		    done = true;
		    break;
		}
	    if (!done)
	    {
		sect->dest = M1(sect->dest);
		ERROR0(ERR_WARNING,
			"No space for cheat code section %u (size %x): %s\n",
				i, sect->size, fname );
		fail = true;
	    }
	}

     #ifdef TEST0
	{
	    uint i;
	    const struct data_t *dp;
	    for ( i = 0, dp = data_tab; dp->data; i++, dp++ )
		printf(" -> %2u: %04zx .. %04zx : %4x / %4x : %4x\n",
			i, dp->data - data, dp->data - data + dp->used,
			dp->used, dp->size, dp->size - dp->used );
	}
     #endif

	if (fail)
	{
	    if (!sorted)
	    {
		qsort( sect_list+1, n_sect-1, sizeof(*sect_list),
			(qsort_func)sort_sect_list_by_size );
		sorted = true;
		goto restart;
	    }

	    FREE(new_data);
	    ERROR0(ERR_ERROR,
		"Not enough space for cheat codes: %s\n",fname);
	    return ERR_WARNING;
	}

	if ( opt_gct_list > 0 )
	{
	    qsort( sect_list+1, n_sect-1, sizeof(*sect_list),
			(qsort_func)sort_sect_list_by_dest );
	    fprintf(stdlog,"\nSummary of GCT section distribution:\n");
	    for ( i = 1, sect = sect_list+1; i < n_sect; i++, sect++ )
	    {
		fprintf(stdlog,
			"   S%02u: %04x .. %04x, size %04x = %4u/dec => ",
			sect->idx, sect->off, sect->off+sect->size,
			sect->size, sect->size );
		if (IS_M1(sect->dest))
		    fprintf(stdlog,"FAILED!\n");
		else
		    fprintf(stdlog,"%04x .. %04x\n",sect->dest,sect->dest+sect->size);
	    }
	    fputc('\n',stdlog);
	}

	struct data_t *prev = 0;
	for ( dp = data_tab; dp->data; dp++ )
	    if ( dp->used)
	    {
		if (prev)
		{
		    u8 *dest = prev->data + prev->used;
		    write_be16(dest,0x6620);
		    write_be16( dest+2, ( dp->data - dest - 8 ) / 8 );
		    write_be32(dest+4,0);
		    prev->used += 8;
		}
		prev = dp;
	    }
	if (prev)
	{
	    u8 *dest = prev->data + prev->used;
	    write_be32(dest,0xf0000000ul);
	    write_be32(dest+4,0);
	    prev->used += 8;
	}

     #ifdef TEST0
	{
	    uint i;
	    const struct data_t *dp;
	    for ( i = 0, dp = data_tab; dp->data; i++, dp++ )
		printf("  -> %2u: %04zx .. %04zx : %4x / %4x : %4x\n",
			i, dp->data - data, dp->data - data + dp->used,
			dp->used, dp->size, dp->size - dp->used );
	}
     #endif

	err = AddSectionHelper2(str,seg,data_tab->data,data_tab->used,fname);
	if ( err == ERR_OK )
	{
	    wch_segment_t dseg;
	    memset(&dseg,0,sizeof(dseg));
	    dseg.type = 'd';
	    if ( data_tab[1].used )
	    {
		dseg.addr = data_tab[1].data - data + 0x80000000;
		const uint size = data_tab[2].used
			? data_tab[2].data + data_tab[2].used - data_tab[1].data
			: data_tab[1].used;
		err = AddSectionHelper2(str,&dseg,data_tab[1].data,size,fname);
	    }
	    else if ( data_tab[2].used )
	    {
		dseg.addr = data_tab[2].data - data + 0x80000000;
		err = AddSectionHelper2(str,&dseg,data_tab[2].data,data_tab[2].used,fname);
	    }
	}
	return err;
    }


    //--- warn if overlapping 0x80003xxx

    if ( ch_type != CHT_NONE
	&& ( seg->addr < 0x80004000 && off_end > 0x80003000 ))
    {
	if ( opt_gct_move >= OFFON_AUTO && cheat_offset >= 0x100 )
	    goto use_loader;

	FREE(new_data);
	ERROR0(ERR_ERROR,
	    "Cheat codes must not overlay 0x80003000..0x80004000: %s\n",
			fname);
	return ERR_WARNING;
    }


    //--- standard: use 1 section

    err = AddSectionHelper2(str,seg,seg_data,seg_size,fname);

 abort:
    FREE(new_data);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ReadCodeFile
	( staticr_t *str, ccp fname, u8 **data, uint *size, bool *alloced )
{
    DASSERT(str);
    DASSERT(fname);
    DASSERT(data);
    DASSERT(size);
    DASSERT(alloced);

    *data = 0;
    *size = 0;
    *alloced = false;

    if ( fname && fname[0] == '@' )
    {
	BZ2Manager_t *wcm = 0;
	if      (!strcmp(fname,"@LECODE"))	wcm = GetLecodeLoader(str->mode);
//	else if (!strcmp(fname,"@FIX514XX"))	wcm = GetFix514XX(str->mode);

	if (wcm)
	{
	    DecodeBZIP2Manager(wcm);
	    *data = wcm->data;
	    *size = wcm->size;
	    return ERR_OK;
	}
    }

    enumError err = OpenReadFILE(fname,0,true,data,size,0,0);
    if (err)
    {
	char path_buf[PATH_MAX];
	if (ReplaceRegionChar(str,path_buf,sizeof(path_buf),fname))
	    err = OpenReadFILE(path_buf,0,true,data,size,0,0);

	if (err)
	{
	    ERROR0(ERR_WARNING,"Can't load file: %s\n",fname);
	    return err;
	}
    }

    *alloced = true;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError AddWCode ( cvp data, uint size )
{
    DASSERT(data);
    DASSERT(!(size&7));
    InititializeGCT();

    u8 *ptr = (u8*)data;
    u8 *end = ptr + size;
    u32 buf[5];

    if ( be64(ptr) == GCT_MAGIC8_NUM )
	ptr += 8;

    while ( ptr < end )
    {
      switch (*ptr)
      {
	case 0x04:
	    buf[0] = htonl(0x04);
	    buf[1] = htonl( be32(ptr) & 0xffffff | 0x80000000 );
	    buf[2] = ((u32*)ptr)[1];
	    InsertGrowBuffer(&wcode_job,buf,12);
	    use_gct_loader = true;
	    ptr += 8;
	    break;

	case 0xc2:
	    {
		const uint len = 8 * be32(ptr+4);
		buf[0] = htonl(0xc2);
		buf[1] = htonl( be32(ptr) & 0xffffff | 0x80000000 );
		buf[2] = htonl( wcode_data.used );
		buf[3] = htonl( wcode_data.used + len - 4 );
		InsertGrowBuffer(&wcode_job,buf,16);
		InsertGrowBuffer(&wcode_data,ptr+8,len);
		use_gct_loader = true;
		ptr += 8 + len;
	    }
	    break;

	case 0xf0:
	    if ( be32(ptr) != GCT_SEP_NUM1 )
		goto term;
	    ptr += 8; // ignore separators
	    break;

	default:
	    return ERROR0(ERR_WARNING,"WCODE %02x not supported => ABORT\n",*ptr);
      }
    }
 term:
    if (logging>2) TraceGCT("WCode");
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError AddCheatCode
(
    // returns ERR_OK, ERR_NOTHING_TO_DO, ERR_WARNING or ERR_INTERNAL

    const u8	*code_data,	// cheat code data
    uint	code_size,	// cheat code size
    bool	check_dup,	// TRUE: check for duplicates
    ccp		fname		// file name for error messages
)
{
    DASSERT(code_data);

    //--- verify code_size and skip magic and terminator

    code_size &= ~7; // multiple of 8, cut additional bytes
    if ( code_size >= 8 && be64(code_data) == GCT_MAGIC8_NUM )
    {
	// skip magic
	code_data += 8;
	code_size -= 8;
    }

    if ( code_size >= 8 && be64(code_data+code_size-8) == GCT_TERM_NUM )
    {
	// ignore terminator
	code_size -= 8;
    }

    if (!code_size)
	 return ERR_NOTHING_TO_DO;


    //--- first code -> add code handler

    InititializeGCT();

    if (!gct_data_mem.used)
    {
	BZ2Manager_t *mgr = opt_full_gch
			? &codehandler_mgr : &codehandleronly_mgr;
	enumError err = DecodeBZIP2Manager(mgr);
	if (err)
	    return ERROR0(ERR_INTERNAL,0);

	gct_data_mem.begin = GCT_ADDRESS;
	MemMapItem_t *mi;
	mi = InsertMemMap(&gct_data_mem,GCT_ADDRESS,mgr->size-sizeof(gch_header_t));
	 StringCopyS(mi->info,sizeof(mi->info),"Code Handler");
	mi = InsertMemMap(&gct_data_mem,GCT_MM_BASE+mgr->size,8);
	 StringCopyS(mi->info,sizeof(mi->info),"GCT magic");
	if (!use_gct_loader)
	{
	    mi = InsertMemMap(&gct_data_mem,GCT_MAX,0);
	    StringCopyS(mi->info,sizeof(mi->info),"-- max possible --");
	}

	InsertGrowBuffer(&gecko_data,mgr->data,mgr->size);
	u64 magic = hton64(GCT_MAGIC8_NUM);
	InsertGrowBuffer(&gecko_data,&magic,sizeof(magic));
	if (logging>2)
	    TraceGCT("Setup");
    }


    //--- check, if same code already inserted

    if ( check_dup && code_size <= gecko_data.used )
    {
	const u8 *ptr = gecko_data.ptr;
	const u8 *end = ptr + gecko_data.used - code_size;

	while ( ptr <= end )
	{
	    if (!memcmp(ptr,code_data,code_size))
		return ERR_NOTHING_TO_DO;
	    ptr += 8;
	}
    }


    //--- add cheat code

    MemMapItem_t *mi
	= InsertMemMap(&gct_data_mem,GCT_MM_BASE+gecko_data.used,code_size);
    snprintf(mi->info,sizeof(mi->info),
	    "%4u line%s of GCT file", code_size/8, code_size == 8 ? "" : "s" );

    if ( opt_gct_scan_sep && opt_gct_sep && be32(code_data) != GCT_SEP_NUM1 )
    {
	u32 buf[2];
	buf[0] = htonl(GCT_SEP_NUM1);
	buf[1] = htonl(code_size/8);
	InsertGrowBuffer(&gecko_data,buf,8);
    }

    InsertGrowBuffer(&gecko_data,code_data,code_size);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError AddWCodeFile ( staticr_t *str, ccp fname )
{
    DASSERT(str);
    if ( !fname || !*fname )
	return ERR_OK;

    u8 *data;
    uint size;
    bool alloced;
    enumError err = ReadCodeFile(str,fname,&data,&size,&alloced);
    if (err)
	return err;

// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,size,size);
    if ( fform == FF_GCT )
    {
	err = opt_wcode == WCODE_GECKO
		? AddCheatCode(data,size,true,fname)
		: AddWCode(data,size);
    }
    else
	err = ERROR0(ERR_WARNING,"Not a GCT file: %s\n",fname);

    if(alloced)
	FREE(data);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

#if HAVE_XSRC
 #include "xsrc/x-staticr.c"
#endif

///////////////////////////////////////////////////////////////////////////////

static uint AddSectionGCH
(
    staticr_t		*str,		// valid pointer
    u8			*gh_data,	// GCH data
    uint		gh_size,	// GHC size
    ccp			fname,		// filename for messages
    bool		get_min_addr	// true: do nothing and get minimal section addr
)
{
    DASSERT(str);
    DASSERT(str->data_size);
    DASSERT(str->data);
    DASSERT(str->is_dol);
    DASSERT(gh_data||!gh_size);


    //--- get param

    const u32 patch_addr = GetVbiAddress(str);
    if (!patch_addr)
	return 0;


    //--- check for GCT

// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(gh_data,gh_size,gh_size);
    if ( fform == FF_GCT )
    {
	AddCheatCode(gh_data,gh_size,false,fname);
	return 0;
    }

    const gch_header_t *gh = (gch_header_t*)gh_data;
    if (!IsValidGCH(gh,gh_size))
    {
     #if HAVE_XSRC
	return IsNotGCH(str,fname,gh_data,gh_size,get_min_addr);
     #else
	ERROR0(ERR_WARNING,"Not a valid GCH file: %s\n",fname);
	return 0;
     #endif
    }

    gh_size -= sizeof(gch_header_t); // from now we only need the raw data size

    if (get_min_addr)
	return ntohl(gh->addr);

    wch_segment_t seg;
    memset(&seg,0,sizeof(seg));
    seg.type		= 't';
    seg.addr		= ntohl(gh->addr);
    seg.size		= gh_size;
    seg.vbi_entry	= ntohl(gh->vbi_entry);

    const enumError err = AddSectionHelper(str,&seg,gh->data,fname);
    return err < ERR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

static uint AddSectionDOL
(
    staticr_t		* str,		// valid pointer
    ccp			fname,		// filename to open
    bool		get_min_addr	// true: do nothing and get minimal section addr
)
{
    DASSERT(str);
    DASSERT(str->data_size);
    DASSERT(str->data);
    DASSERT(str->is_dol);

    if ( !fname || !*fname )
	return 0;


    //--- get param

    const u32 patch_addr = GetVbiAddress(str);
    if (!patch_addr)
	return 0;


    //--- load file

    u8 *gh_data = 0;
    uint gh_size = 0;
    bool gh_alloced;
    enumError err = ReadCodeFile(str,fname,&gh_data,&gh_size,&gh_alloced);
    if (err)
	return err;

    const uint stat = AddSectionGCH(str,gh_data,gh_size,fname,get_min_addr);
    if (gh_alloced)
	FREE(gh_data);
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

static uint AddSectionsDOL
(
    staticr_t		* str,		// valid pointer
    bool		get_min_addr	// true: do nothing and get minimal section addr
)
{
    DASSERT(str);
    DASSERT(str->data_size);
    DASSERT(str->data);
    DASSERT(str->is_dol);

    uint i, count = get_min_addr ? 0xffffffff : 0;
    for ( i = 0; i < opt_sect_list.used; i++ )
    {
	ccp fname = opt_sect_list.field[i];
	const uint stat = AddSectionDOL(str,fname,get_min_addr);
	if (!get_min_addr)
	    count += stat;
	else if ( stat && count > stat )
	    count = stat;
    }

    if ( gecko_data.used )
    {
	if (logging>0)
	    fprintf(stdlog,">> GCT DATA/GROW, %u = 0x%x bytes\n",
			gecko_data.used, gecko_data.used );

	if (!get_min_addr)
	{
	    // terminate section and add it to dol
	    MemMapItem_t *mi = InsertMemMap(&gct_data_mem,GCT_MM_BASE+gecko_data.used,8);
	    StringCopyS(mi->info,sizeof(mi->info),"GCT terminator");

	    u64 term = hton64(GCT_TERM_NUM);
	    InsertGrowBuffer(&gecko_data,&term,sizeof(term));
	    count += AddSectionGCH(str,gecko_data.ptr,gecko_data.used,"GCT",false);

	    if ( verbose > 1 || logging > 1 )
	    {
		fputs("\n   Memory map of code handler and GCT files:\n",stdlog);
		PrintMemMap(&gct_data_mem,stdlog,5,"part");
		fputc('\n',stdlog);
	    }
	}
	else if ( count > 0x80001800 )
	    count = 0x80001800;
	ResetGCT();
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

static uint CreateSectionDOL
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data_size);
    DASSERT(str->data);
    DASSERT(str->is_dol);

    enum
    {
	OFF_SECT_ADDR	= 0x10,
	OFF_REL_ENTRY	= 0x14,
	OFF_ORIG_ENTRY	= 0x18,
	OFF_RDOL_ADDR	= 0x1c,
	OFF_END_PARAM	= 0x20,

	RED_DOL_SIZE	= 0xe4 - 0x48,	// reduced DOL size
    };

    u32 dol_sect_offset	= 0; // file offset of DOL section
    u32 dol_sect_addr	= 0; // address of DOL section
    dol_header_t *dh = (dol_header_t*)str->data;


    //--- find max dol address

    u32 max_addr = dh->bss_addr + dh->bss_size;
    uint sect;
    for ( sect = 0; sect < DOL_N_SECTIONS; sect++ )
    {
	const u32 size = ntohl(dh->sect_size[sect]);
	if (size)
	{
	    const u32 off = ntohl(dh->sect_addr[sect]) + size;
	    if ( max_addr < off )
		max_addr = off;
	}
    }
    max_addr = ALIGN32(max_addr,4);


    //--- iterate new section list

    uint count;
    for ( count = sect = 0; sect < n_section_list; sect++ )
    {
	dol_sect_select_t *sl = section_list + sect;
	sl->offset = 0;
	if ( sl->find_mode < 0 )
	    continue;

	dol_sect_info_t si;
	if (!FindDolSectionBySelector(&si,dh,sl))
	{
	    ERROR0(ERR_WARNING,"DOL section '%s' not available: %s\n",
		sl->name, str->fname );
	    return 0;
	}

	bool is_dol_info = !strcasecmp(sl->fname,".dol");
	if (is_dol_info)
	{
	    sl->use_param = 0;
	    sl->size = RED_DOL_SIZE;
	}
	else if (!sl->data)
	{
	    if (OpenReadFILE( sl->fname, 0, true, &sl->data, &sl->size, 0, 0 ))
	    {
		ERROR0(ERR_WARNING,"Can't load file for '%s': %s\n",
			sl->name, sl->fname );
		sl->find_mode = -1;
		continue;
	    }
	}
	count++;

	uint offset = str->data_size;
	sl->offset = offset;
	str->data_size += sl->size;
	str->data = REALLOC(str->data,str->data_size);
	dh = (dol_header_t*)str->data;
	u8 *dest = str->data + offset;

	u32 sect_addr = !sl->addr && sl->use_param
			? be32(sl->data+OFF_SECT_ADDR) : sl->addr;

	if (is_dol_info)
	{
	    sl->data = (u8*)&dh->sect_addr;
	    if (!sect_addr)
		sect_addr = 0x80004000 - RED_DOL_SIZE;
	    dol_sect_addr   = sect_addr;
	    dol_sect_offset = offset;
	}

	if (!sect_addr)
	    sect_addr = max_addr;
	const u32 end_addr = ALIGN32( sect_addr + sl->size, 4 );
	if ( max_addr < end_addr )
	    max_addr = end_addr;

	u32 new_entry = 0;
	bool param_valid = false;
	if ( sl->use_param && sl->size >= OFF_END_PARAM )
	{
	    new_entry = be32(sl->data+OFF_REL_ENTRY);
	    if (!new_entry)
	    {
		param_valid = true;
	    }
	    else if ( !(new_entry&3) && new_entry < sl->size )
	    {
		param_valid = true;
		new_entry += sect_addr;
	    }
	    else
		new_entry = 0;
	}
	if (!param_valid)
	    sl->use_param = false;


	//--- create section

	PRINT(" - create section %s[%d] [%08x..%08x], f-off 0x%06x, size 0x%x, par=0x%x\n",
		    si.name, si.section, sect_addr, sect_addr + sl->size,
		    offset, sl->size, new_entry );

	if (verbose>0)
	{
	    fprintf(stdlog,
		"- Create section %s [%08x..%08x], file offset 0x%06x, size 0x%x",
		si.name, sect_addr, sect_addr + sl->size, offset, sl->size );
	    if (new_entry)
		fprintf(stdlog,", entry 0x%x\n",new_entry);
	    else if (param_valid)
		fprintf(stdlog,", use param\n");
	    else
		fputc('\n',stdlog);
	}

	dh->sect_off [si.section] = htonl(offset);
	dh->sect_addr[si.section] = htonl(sect_addr);
	dh->sect_size[si.section] = htonl(sl->size);
	memcpy( dest, sl->data, sl->size );

	//--- store original entry point

	memcpy(dest+OFF_ORIG_ENTRY,&dh->entry_addr,sizeof(u32));

	//--- use relative entry point of new section

	if (new_entry)
	    dh->entry_addr = htonl(new_entry);
    }


    //--- final '.dol' section support

    if (dol_sect_offset)
    {
	//- refresh dol section

	PRINT("UPDATE RED-DOL\n");
	dol_header_t *dh = (dol_header_t*)str->data;
	memcpy( str->data + dol_sect_offset, &dh->sect_addr, RED_DOL_SIZE );

	//- update sections with 'use_param'

	for ( sect = 0; sect < n_section_list; sect++ )
	{
	    dol_sect_select_t *sl = section_list + sect;
	    PRINT("UPDATE sect %s [off=%x,param=%d] := %x\n",
			sl->name, sl->offset+OFF_RDOL_ADDR, sl->use_param, dol_sect_addr );
	    if ( sl->offset && sl->use_param )
		write_be32( str->data + sl->offset + OFF_RDOL_ADDR, dol_sect_addr );
	}
    }

    return count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint PatchCTCODEDOL
(
    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data_size);
    DASSERT(str->data);
    DASSERT(str->is_dol);

    if (!opt_add_ctcode)
	return 0;

    if (!(str->dol_status&DOL_S_ORIG))
    {
	ERROR0(ERR_WARNING,"Can't add CT-CODE to modified DOL: %s\n",str->fname);
	return 0;
    }

    ct_dol_ext_t *ce = GetCtExtDOL(str->mode);
    if (!ce)
    {
	ERROR0(ERR_WARNING,"Can't add CT-CODE: %s\n",str->fname);
	return 0;
    }

    BZ2Source_t *bs = &ce->t2_src;
    SearchBZ2S(bs,ct_dir_list.field,ct_dir_list.used,SEA_LOG_OFF);
    if ( !bs->data || !bs->size )
    {
     missing_ctcode:;
	ccp sem = strchr(bs->search,';');
	ERROR0(ERR_WARNING,
		"Missing %.*s => Can't add CT-CODE: %s\n\n",
		(int)( sem ? sem - bs->search : strlen(bs->search) ),
		bs->search, str->fname );
	return 0;
    }

    bs = &ce->d8_src;
    SearchBZ2S(&ce->d8_src,ct_dir_list.field,ct_dir_list.used,SEA_LOG_OFF);
    if ( !bs->data || !bs->size )
	goto missing_ctcode;

    const BZ2Source_t *t2 = &ce->t2_src;
    const BZ2Source_t *d8 = &ce->d8_src;


    //--- check data address

    bool move_d8 = opt_move_d8 || opt_sect_list.used;
    u32 d8_addr = move_d8 ? 0x802a8000 : ce->d8_addr;

    dol_header_t *dh = (dol_header_t*)str->data;
    if (CountOverlappedDolSections(dh,d8_addr,d8->size))
    {
	move_d8 = true;
	d8_addr = 0x802a8000;
	if (CountOverlappedDolSections(dh,d8_addr,d8->size))
	{
	    ERROR0(ERR_WARNING,"No space for CTCODE/D8 section: %s\n",str->fname);
	    return 0;
	}
    }

    if (CountOverlappedDolSections(dh,ce->t2_addr,t2->size))
    {
	ERROR0(ERR_WARNING,"No space for CTCODE/T2 section: %s\n",str->fname);
	return 0;
    }

    dol_sect_info_t t2info;
    if (!FindFirstFreeDolSection(&t2info,dh,0))
    {
     err2sect:
	ERROR0(ERR_WARNING,"Need 2 unused sections for CTCODE: %s\n",str->fname);
	return 0;
    }
    const int T_SECT = t2info.section;
    dh->sect_size[T_SECT] = htonl(t2->size); // mark used for next search

    dol_sect_info_t d8info;
    if (!FindFirstFreeDolSection(&d8info,dh,3))
    {
	dh->sect_size[T_SECT] = htonl(0);
	goto err2sect;
    }
    const int D_SECT = d8info.section;


    //--- logging

    uint offset = str->data_size;
    if ( verbose > 0 )
    {
	fprintf(stdlog,
	    "- Create section %s [%08x..%08x], file offset 0x%06x, size 0x%x\n",
		t2info.name, ce->t2_addr,
		ce->t2_addr + t2->size, offset, t2->size );
	fprintf(stdlog,
	    "- Create section %s [%08x..%08x], file offset 0x%06x, size 0x%x\n",
		d8info.name, ce->d8_addr,
		ce->d8_addr + d8->size, offset + t2->size, d8->size );
    }


    //--- assign data and add section

    str->data_size += t2->size + d8->size;
    str->data = REALLOC(str->data,str->data_size);
    dh = (dol_header_t*)str->data;

    PRINT("%s: off=%6x, addr=%8x, size=%6x\n",t2info.name,offset,ce->t2_addr,t2->size);
    memcpy( str->data + offset, t2->data, t2->size );
    write_be32( str->data + offset + 0x14, d8_addr );
    dh->sect_off [T_SECT] = htonl(offset);
    dh->sect_addr[T_SECT] = htonl(ce->t2_addr);
    dh->sect_size[T_SECT] = htonl(t2->size);

    offset += t2->size;
    PRINT("%s: off=%6x, addr=%8x, size=%6x\n",d8info.name,offset,ce->d8_addr,d8->size);
    memcpy( str->data + offset, d8->data, d8->size );
    dh->sect_off [D_SECT] = htonl(offset);
    dh->sect_addr[D_SECT] = htonl(d8_addr);
    dh->sect_size[D_SECT] = htonl(d8->size);

    memcpy(&dh->entry_addr,t2->data+0x0c,4);

    ClearBZ2S(&ce->t2_src,1);
    ClearBZ2S(&ce->d8_src,1);
    str->dol_flags[DPF_CTCODE] = 'C';
    str->dol_info_flags |= DIF_CTCODE;
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

uint PatchDOL
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
)
{
    DASSERT(str);
    if (!str->data_size)
	return 0;
    DASSERT(str->data);
    DASSERT(str->is_dol);
    if (!str->is_dol)
	return 0;

    uint clean_count = 0;
    if ( opt_clean_dol && CleanDol(str) )
	clean_count++;

    if (!(str->dol_status&DOL_S_ANALYZED))
	AnalyzeDOL(str,false);

    InititializeGCT();
    if ( opt_wcode >= WCODE_ENABLED )
    {
	if (wifi_domain_is_wiimmfi)
	{
	    BZ2Manager_t *wcm = GetWiimmfiCode(str->mode);
	    if (wcm)
	    {
		DecodeBZIP2Manager(wcm);
		if ( opt_wcode == WCODE_GECKO )
		{
		    str->dol_flags[DPF_WCODE] = 'w';
		    AddCheatCode(wcm->data,wcm->size,true,"WCODE");
		}
		else
		{
		    str->dol_flags[DPF_WCODE] = 'W';
		    AddWCode(wcm->data,wcm->size);
		}
	    }
	}

	uint i;
	for ( i = 0; i < opt_wcode_list.used; i++ )
	    if (AddWCodeFile(str,opt_wcode_list.field[i]))
		opt_wcode_list.field[i] = 0;
    }

    uint patch_count = 0;
    patch_count += PatchHTTPS(str);
    patch_count += PatchCTCODEDOL(str);
    patch_count += AddSectionsDOL(str,false);
    patch_count += CreateSectionDOL(str);
    patch_count += PatchByListWPF(str);

    if (use_gct_loader)
    {
	enumError err = SetupCodeLoader(str,0,0,0,0,CHT_NONE,0);
	if ( err == ERR_OK || err == ERR_WARNING )
	    patch_count++;
    }


    //-- sdkver by region

    if ( opt_str_vs_region >= 0 )
    {
	const u32 vers_addr = GetSdkverAddress(str->mode);
	if (vers_addr)
	{
	    dol_header_t *dh = (dol_header_t*)str->data;
	    const u32 offset = GetDolOffsetByAddr(dh,vers_addr,8,0);
	    if (offset)
	    {
		u8 old[8];
		memcpy(old,str->data+offset,sizeof(old));

		char buf[20];
		snprintf(buf,sizeof(buf), "%05u", opt_str_vs_region & 0xffff);
		memcpy(str->data+offset+1,buf,6);
		if ( str->data[offset] == '0' )
		    str->data[offset]++;

		if (memcmp(old,str->data+offset,sizeof(old)))
		    patch_count++;
	    }
	}
    }


    //--- patch second host

    bool wiimmfi_patched = false;
    if ( wifi_domain_is_wiimmfi || opt_wcode >= WCODE_ENABLED )
    {
	u32 *h2 = GetSecondHostPointer(str,0);
	noPRINT("h2=%p %x\n",h2,h2?ntohl(*h2):0);
	if ( h2 && ntohl(*h2) != ASM_NOP )
	{
	    write_be32(h2,ASM_NOP);
	    patch_count++;
	    wiimmfi_patched = true;
	}
    }


    //-- patched by

    patched_by_t pb_mode = patched_by_mode;
    switch((int)pb_mode)
    {
	case PBY_WIIMMFI:
	    if (wiimmfi_patched)
		pb_mode = PBY_ALWAYS;
	    break;

	case PBY_AUTO:
	    if (patch_count)
		pb_mode = PBY_ALWAYS;
	    break;
    }

    if ( pb_mode == PBY_RESET )
    {
	char *dest = GetIdPointer(str);
	if (dest)
	    memset(dest,'*',PATCHED_BY_SIZE);
    }
    else if ( pb_mode == PBY_ALWAYS )
    {
	char *dest = GetIdPointer(str);
	if (dest)
	{
	    char *end = dest + PATCHED_BY_SIZE;
	    if (*str->dol_flags)
		dest += snprintf( dest, PATCHED_BY_SIZE, "%.*s [%s]",
					PATCHED_BY_SIZE - 3 - DPF__N,
					opt_patched_by, str->dol_flags );
	    else
		dest = StringCopyE(dest,end,opt_patched_by);
	    while ( dest < end )
		*dest++ = ' ';
	}
    }

    return clean_count + patch_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			patch via WPF			///////////////
///////////////////////////////////////////////////////////////////////////////

uint PatchByListWPF
(
    staticr_t		*str		// valid pointer
)
{
    DASSERT(str);
    DASSERT(str->data_size);
    DASSERT(str->data);

    uint i, count = 0;
    for ( i = 0; i < opt_wpf_list.used; i++ )
    {
	ccp fname = opt_wpf_list.field[i];
	if ( fname && *fname )
	{
	    const int stat = PatchByFileWPF(str,fname);
	    if ( stat < 0 )
	    {
		FreeString(opt_wpf_list.field[i]);
		opt_wpf_list.field[i] = 0;
	    }
	    else if ( stat > 0 )
		count++;
	}
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

int PatchByFileWPF
(
    staticr_t		*str,		// valid pointer
    ccp			fname
)
{
    DASSERT(str);
    DASSERT(fname);


    //--- load file

    u8 *data = 0;
    uint size = 0;
    if (OpenReadFILE(fname,0,true,&data,&size,0,0))
    {
	ERROR0(ERR_WARNING,"Can't load WPF file: %s\n",fname);
	return -1;
    }


    //--- analyse file

// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,size,size);
    PRINT(">>> [%s,%u]: %s\n",GetNameFF(fform,0),size,fname);
    bool done = false;
    int stat = -1;

    if ( size > sizeof(wpf_head_t) && be32(data+4) <= size )
    {
	wpf_t *wpf = (wpf_t*)(data+sizeof(wpf_head_t));
	if ( fform == FF_WPF )
	{
	    done = true;
	    stat = PatchByWPF( str, wpf, size-sizeof(wpf_head_t), fname );
	}

     #if HAVE_XSRC
	if ( fform == FF_XPF )
	{
	    done = true;
	    stat = PatchByXPF( str, wpf, size-sizeof(wpf_head_t), fname );
	}
     #endif
    }

    if (!done)
	ERROR0(ERR_WARNING,"Not a WPF file:: %s\n",fname);
    FREE(data);
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

int PatchByWPF
(
    staticr_t		*str,		// valid pointer
    const wpf_t		*wpf,		// wpf patch list
    uint		size,		// size of data beginning at 'wpf'
    ccp			fname		// filename for error messages
)
{
    DASSERT(str);
    DASSERT(fname);
    printf("PatchByWPF(): %s\n",fname);

    HexDump16(stdout,0,0,wpf, size < 32 ? size : 32);
    return -1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

bool opt_clean_dol	= false;	// true: remove additional DOL sections
bool opt_add_ctcode	= false;	// true: ad CT-CODE to main.dol
int  opt_all_ranks	= 0;		// -1:off, 0:don't patch, 1:on
bool opt_move_d8	= false;	// true: move CT-CODE D8 section to another place
bool opt_full_gch	= false;	// true: use full- of only- 'codehandler'

bool opt_gct_scan_sep	= true;		// true: scan 'F0000001 0000xxxx' separators
bool opt_gct_sep	= false;	// true: search cheat code separators
bool opt_gct_asm_sep	= false;	// true: use beginning of ASM blocks as separator
uint opt_gct_list	= false;	// >0: print a summary / >1: be verbose
int  opt_gct_move	= OFFON_AUTO;	// move GCT to heap
u32  opt_gct_addr	= 0x802c0000;	// address for opt_gct_move
u32  opt_gct_space	= 0;		// minimal code space for opt_gct_move
int  opt_allow_user_gch	= OFFON_AUTO;	// allow user define Gecko Code Handler

StringField_t opt_sect_list = {0};	// files with section data of type GCT|GCH|WCH
StringField_t opt_wpf_list  = {0};	// files with patches of type WPF|XPF
StringField_t opt_wcode_list = {0};	// GCT files for wcode jobs

int  opt_str_vs_region	= SREG_NONE;	// versus region patch mode
bool opt_str_xvs_region	= false;	// true: patch also test offsets
bool opt_str_tvs_region	= false;	// true: enable test mode with incremented regions

int  opt_str_bt_region	= SREG_NONE;	// battle region patch mode
bool opt_str_xbt_region	= false;	// true: patch also test offsets
bool opt_str_tbt_region	= false;	// true: enable test mode with incremented regions

bool opt_meno		= false;	// true: patch 'Menu*' to 'MenO*'

///////////////////////////////////////////////////////////////////////////////

int ScanOptRegionHelper ( ccp arg, bool is_battle, ccp prefix )
{
    region_info_t ri;
    SetupRegionInfo(&ri,0,is_battle);

    DASSERT(ri.opt_region);
    if ( !arg || !*arg )
    {
	*ri.opt_region = SREG_NONE;
	return 0;
    }

    DASSERT(ri.opt_x);
    *ri.opt_x = *arg == 'x' || *arg == 'X';
    if (*ri.opt_x)
	arg++;

    DASSERT(ri.opt_t);
    *ri.opt_t = ( *arg == 't' || *arg == 'T' ) && arg[1] >= '0' && arg[1] <= '9';
    if (*ri.opt_t)
	arg++;

    if ( *arg >= '0' && *arg <= '9' )
    {
	u32 num;
	const enumError err =
	    ScanSizeOptU32(
			&num,			// u32 * num
			arg,			// ccp source
			1,			// default_factor1
			0,			// int force_base
			"region",		// ccp opt_name
			0,			// u64 min
			0xffff,			// u64 max
			1,			// u32 multiple
			0,			// u32 pow2
			false			// bool print_err
			);

	if ( err == ERR_OK )
	{
	    *ri.opt_region = num;
	    return 0;
	}
    }

    static const KeywordTab_t tab[] =
    {
	{ SREG_JAPAN,		"JAPAN",	0,		0 },
	{ SREG_AMERICA,		"AMERICA",	"USA",		0 },
	{ SREG_EUROPE,		"EUROPE",	0,		0 },

	{ SREG_AUSTRALIA,	"AUSTRALIA",	"OCEANIA",	0 },
	{ SREG_KOREA,		"KOREA",	0,		0 },
	{ SREG_TAIWAN,		"TAIWAN",	0,		0 },
	{ SREG_CHINA,		"CHINA",	0,		0 },

	{ SREG_NONE,		"NONE",		0,		0 },
	{ SREG_RESTORE,		"RESTORE",	"NINTENDO",	0 },
	{ SREG_TEST,		"TEST",		0,		9001 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	*ri.opt_region = cmd->id;
	if ( *ri.opt_region == SREG_TEST )
	{
	    *ri.opt_region = cmd->opt;
	    *ri.opt_t = true;
	}
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid `region' mode (option --%sregion): '%s'\n",prefix,arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptVersusRegion ( ccp arg )
{
    return ScanOptRegionHelper(arg,false,"vs-");
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptBattleRegion ( ccp arg )
{
    return ScanOptRegionHelper(arg,true,"bt-");
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptRegion ( ccp arg )
{
    const int stat = ScanOptRegionHelper(arg,false,"");

    opt_str_bt_region = opt_str_vs_region;
    opt_str_xbt_region = opt_str_xvs_region;
    opt_str_tbt_region = opt_str_tvs_region;
    if (opt_str_tbt_region)
	opt_str_bt_region += N_TAB_VS_REGION_PATCH;

    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptAllRanks ( ccp arg )
{
    if ( !arg || !*arg )
    {
	opt_all_ranks = 1;
	return 0;
    }

    static const KeywordTab_t tab[] =
    {
	{ -1,	"RETORE",	"NINTENDO",	0 },
	{ -1,	"RESET",	0,		0 }, // [obsolete]] in 2015
	{ 0,	"NONE",		0,		0 },
	{ 1,	"ON",		0,		0 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	opt_all_ranks = cmd->id;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid `all-ranks' mode (option --all-ranks): '%s'\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t HttpsOptions[] =
{
    { HTTPS_NONE,	"NONE",		0,		0 },
    { HTTPS_RESTORE,	"RESTORE",	"NINTENDO",	0 },
    { HTTPS_HTTP,	"HTTP",		0,		0 },
    { HTTPS_DOMAIN,	"DOMAIN",	0,		0 },
    { HTTPS_SAKE0,	"SAKE0",	"S0",		0 },
    { HTTPS_SAKE1,	"SAKE1",	"S1",		0 },
    { 0,0,0,0 }
};

https_mode_t opt_https;
bool opt_wc24 = false;
char mkw_domain[100] = "mariokartwii.gs.wiimmfi.de";
ccp  wifi_domain = mkw_domain + 16;
bool wifi_domain_is_wiimmfi = false;
wcode_t opt_wcode = WCODE_AUTO;

///////////////////////////////////////////////////////////////////////////////

int ScanOptHttps ( ccp arg )
{
    const KeywordTab_t *cmd = ScanKeyword(0,arg,HttpsOptions);
    if (cmd)
    {
	opt_https = cmd->id;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid mode for option --https: '%s'\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetHttpsName ( https_mode_t mode, ccp return_if_failed )
{
    const KeywordTab_t *cmd;
    for ( cmd = HttpsOptions; cmd->name1; cmd++ )
	if ( cmd->id == mode )
	    return cmd->name1;
    return return_if_failed;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptDomain ( ccp https, ccp arg )
{
    if (arg)
	snprintf(mkw_domain,sizeof(mkw_domain),"mariokartwii.gs.%s",arg);

    int stat = https ? ScanOptHttps(https) : 0;
    wifi_domain_is_wiimmfi = opt_https >= HTTPS_DOMAIN
			    && (  !strcmp(wifi_domain,"wiimmfi.de")
			       || !strcmp(wifi_domain,"test.wiimmfi.de") );
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t wcode_keytab[] =
{
	{ WCODE_OFF,	"OFF",	 "0",		0 },
	{ WCODE_AUTO,	"AUTO",	 "DEFAULT",	0 },
	{ WCODE_GECKO,	"GECKO", "CHEAT",	0 },
	{ WCODE_ON,	"ON",	 "1",		0 },
	{ 0,0,0,0 }
};

int ScanOptWCode ( ccp arg )
{
    if (!arg)
    {
	opt_wcode = WCODE_ON;
	return 0;
    }

    int status;
    const KeywordTab_t *key = ScanKeyword(&status,arg,wcode_keytab);
    if (key)
    {
	opt_wcode = key->id;
	return 0;
    }

    PrintKeywordError(wcode_keytab,arg,status,0,"option --wcode");
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPatchedBy ( ccp arg )
{
    if (!*arg)
	arg = "";

    while ( *arg > 0 && *arg <= ' ' )
	arg++;

    if (!*arg)
	StringCopyS(opt_patched_by,sizeof(opt_patched_by),STD_PATCHED_BY);
    else if ( *arg == '+' )
	snprintf(opt_patched_by,sizeof(opt_patched_by),
			"%s [%s]",STD_PATCHED_BY,arg+1);
    else
	StringCopyS(opt_patched_by,sizeof(opt_patched_by),arg);

    uchar *ptr;
    for ( ptr = (uchar*)opt_patched_by; *ptr; ptr++ )
	if ( *ptr < ' ' || *ptr == 0xa0 )
	    *ptr = ' ';

    noPRINT("< %s\n> %s\n",arg,opt_patched_by);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enum VsBtMode patch_vs_mode = VSBT_OFF;
enum VsBtMode patch_bt_mode = VSBT_OFF;
char patch_vs_str[2] = {0};
char patch_bt_str[2] = {0};

int ScanOptVS ( bool is_bt, bool allow_2, ccp arg )
{
    if (!arg)
	return 0;

    ccp std_str;
    char *str;
    enum VsBtMode *mode;
    if (is_bt)
    {
	std_str	= "bt";
	str	= patch_bt_str;
	mode	= &patch_bt_mode;
    }
    else
    {
	std_str	= "vs";
	str	= patch_vs_str;
	mode	= &patch_vs_mode;
    }

    switch(strlen(arg))
    {
	case 0:
	    *mode = VSBT_OFF;
	    return 0;

	case 1:
	    *mode = VSBT_PATCH;
	    str[0] = std_str[0];
	    str[1] = arg[0];
	    return 0;

	case 2:
	    if (allow_2)
	    {
		*mode = VSBT_PATCH;
		str[0] = arg[0];
		str[1] = arg[1];
		return 0;
	    }
	    break;

	case 4:
	    if (!strcasecmp(arg,"test"))
	    {
		*mode = VSBT_TEST;
		return 0;
	    }
	    break;
    }

    if (allow_2)
	ERROR0(ERR_SYNTAX,"Only 1 or 2 characters allowed for option --%s2: %s\n",
			std_str, arg );
    else
	ERROR0(ERR_SYNTAX,"Only 1 character allowed for option --%s: %s\n",
			std_str, arg );
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptCannon ( ccp arg )
{
    if ( !arg || !*arg )
    {
	memset(cannon_param,0,sizeof(cannon_param));
	return 0;
    }

    char *end;
    uint index = str2ul(arg,&end,10);
    if ( end == arg )
	return ERROR0(ERR_SYNTAX,"Option --cannon: Invalid argument: %s",arg);
    if ( index >= CANNON_N )
	return ERROR0(ERR_SYNTAX,"Option --cannon: Invalid index: %d",index);

    cannon_param_t *cp = cannon_param + index;
    memset(cp,0,sizeof(*cp));
    const u8 * orig = CannonDataLEX+4 + index * CANNON_SIZE;
    uint i;
    for ( i = 0; i < CANNON_N_PARAM; i++, orig += sizeof(float) )
	cp->param[i] = bef4(orig);

    for ( i = 0; i < CANNON_N_PARAM; i++ )
    {
	arg = end;
	while ( *arg == ' ' || *arg == '\t' || *arg == ',' )
	    arg++;
	double d = strtod(arg,&end);
	if ( end == arg )
	    break;
	cp->param[i] = d;
    }
    if ( i > 0 )
	cp->valid = true;

    #if HAVE_PRINT
    {
	uint i;
	cannon_param_t *cp = cannon_param;
	for ( i = 0; i < CANNON_N; i++, cp++ )
	    PRINT(">CANNON[%u]: valid=%d, %10f %10f %10f %10f\n",
		i, cp->valid, cp->param[0], cp->param[1], cp->param[2], cp->param[3] );
    }
    #endif

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Option --create-sect		///////////////
///////////////////////////////////////////////////////////////////////////////

dol_sect_select_t section_list[DOL_N_SECTIONS];
uint n_section_list = 0;

///////////////////////////////////////////////////////////////////////////////

int ScanOptGctMove ( ccp arg )
{
    const int stat = ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_FORCE,"Option --gct-move");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_gct_move = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptAllowUserGch ( ccp arg )
{
    const int stat
	= ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_FORCE,"Option --allow-user-gch");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_allow_user_gch = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptGctAddr ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_gct_addr,		// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"gct-addr",		// ccp opt_name
		0x00000000,		// u64 min
		0x817fffff,		// u64 max
		4,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptGctSpace ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_gct_space,		// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"gct-space",		// ccp opt_name
		0,			// u64 min
		0x1000000,		// u64 max
		4,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptCreateSect ( ccp arg )
{
    if ( !arg || !*arg )
	return 0;

    if ( n_section_list >= DOL_N_SECTIONS )
    {
	ERROR0(ERR_SEMANTIC,"Section list for --create-sect is full.\n");
	return 1;
    }


    //--- check for 'P'

    ccp arg0 = arg;
    bool use_param = *arg == 'P' || *arg == 'p';
    if (use_param)
    {
	arg++;
	while ( *arg == ' ' || *arg == '\t' || *arg == ',' )
	    arg++;
    }


    //--- check for 'T' 'D' 'T#' 'D#'

    int find_mode = 0, idx = -1;
    if ( *arg == 'T' || *arg == 't' )
    {
	find_mode = 1;
	arg++;
	if ( *arg >= '0' && *arg < '0' + DOL_N_TEXT_SECTIONS )
	    idx = *arg++ - '0';
    }
    else if ( *arg == 'D' || *arg == 'd' )
    {
	find_mode = 2;
	arg++;
	if ( *arg >= '0' && *arg < '0' + DOL_N_DATA_SECTIONS )
	    idx = *arg++ - '0' + DOL_N_TEXT_SECTIONS;
    }

    if (find_mode)
    {
	ccp start = arg;
	while ( *arg == ' ' || *arg == '\t' || *arg == ',' )
	    arg++;
	if ( arg == start && *arg != '=' )
	{
	    arg = arg0;
	    find_mode = 0;
	    idx = -1;
	}
    }

    //--- check for an addr

    char *end;
    uint addr = str2ul(arg,&end,16);
    if ( end > arg )
    {
	arg = end;
	if (addr)
	    addr |= 0x80000000;
    }
    else
	addr = 0;

    if ( *arg != '=' )
    {
	arg = arg0;
	use_param	= false;
	find_mode	= 0;
	idx		= -1;
	addr		= 0;
    }
    else
	arg++;

    dol_sect_select_t *sl = section_list + n_section_list++;
    memset(sl,0,sizeof(*sl));
    sl->find_mode	= find_mode;
    sl->sect_idx	= idx;
    sl->use_param	= use_param;
    sl->addr		= addr;
    sl->fname		= arg;

    switch (sl->find_mode)
    {
	case 1:
	    sl->name[0] = 'T';
	    sl->name[1] = sl->sect_idx < 0 ? '*'
				: '0' + sl->sect_idx;
	    break;

	case 2:
	    sl->name[0] = 'D';
	    sl->name[1] = sl->sect_idx < 0 ? '*'
				: '0' + sl->sect_idx - DOL_N_TEXT_SECTIONS;
	    break;

	default:
	    sl->name[0] = '*';
	    sl->name[1] = '*';
	    break;
    }

    PRINT("CREATE-SECTION[%s,%c]: %u,%2d %#10x : %s\n",
		sl->name, sl->use_param ? 'W' : '-',
		sl->find_mode, sl->sect_idx, sl->addr, sl->fname );

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    DumpSTR()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DumpSTR
(
    FILE		* f,		// dump to this file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    bool		use_c		// true: print in C language fragments
)
{
    DASSERT(f);
    DASSERT(str);

    if (str->is_dol)
	return DumpDOL(f,indent,str,use_c);

    DASSERT(str->data);
    DASSERT( str->data_size >= sizeof(rel_header_t) );

    indent = NormalizeIndent(indent);


    //----- dump header

    rel_header_t hd;
    be32n( (u32*)&hd, (u32*)str->data, sizeof(rel_header_t)/sizeof(u32) );

    ccp mod_name = "-";
    if ( hd.mod_name && hd.mod_name + hd.mod_name_len < str->data_size )
	mod_name = (ccp)str->data + hd.mod_name;


    fprintf(f,"%*sId:           %6u = 0x%x\n",	indent,"", hd.id, hd.id );
    fprintf(f,"%*sN(sections):  %6u\n",		indent,"", hd.n_section );
    fprintf(f,"%*sModule Name:  %6s\n",		indent,"", mod_name );
    fprintf(f,"%*sReloc Offset: %6x\n",		indent,"", hd.reloc_off );

    fprintf(f,"\n%*sSection (0..%u):\n", indent,"", hd.n_section-1 );
    rel_sect_info_t *si = (rel_sect_info_t*)( str->data + hd.section_off );
    uint s;
    for ( s = 0; s < hd.n_section; s++, si++ )
    {
	u32 size   = be32(&si->size);
	if (size)
	{
	    u32 offset = be32(&si->offset);
	    if (!offset)
		fprintf(f,"%*s%4u. BSS  %7x\n", indent,"", s, size );
	    else
	    {
		ccp type = offset & 1 ? "TEXT" : "DATA";
		offset &= ~1;
		fprintf(f,"%*s%4u. %s %7x : %6x .. %6x\n",
			indent,"", s, type, size, offset, offset + size - 1 );
	    }
	}
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    DumpDOL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError DumpDOL_c
(
    FILE		* f,		// dump to this file
    staticr_t		* str		// valid pointer
)
{
    DASSERT(f);
    DASSERT(str);
    DASSERT(str->data);
    DASSERT( str->data_size >= sizeof(dol_header_t) );

    dol_header_t dol;
    if ( str->data_size < sizeof(dol) )
	return ERR_WARNING;
    ntoh_dol_header(&dol,(dol_header_t*)str->data);

    fputs("\nstatic const dol_sect_info_t dol_info_xxx[DOL_NN_SECTIONS] =\n{\n",f);
    char name[8];

    uint sect;
    for ( sect = 0; sect < DOL_NN_SECTIONS; sect++ )
    {
	dol_sect_info_t info;
	GetSectionDOL(str,&info,sect,true);
	snprintf(name,sizeof(name),"\"%s\",",info.name);
	fprintf(f,"  { %2u, %-7s%#8x, %#10x, %#8x, %u,0,%u, 0,",
		sect, name, info.off, info.addr, info.size,
		info.sect_valid, info.hash_valid );

	if (info.hash_valid)
	{
	    u8 *ptr = info.hash;
	    uint line;
	    for ( line = 0; line < 2; line++ )
	    {
		ccp sep = line ? ",\n\t\t  " : "\n\t\t{ ";
		uint idx;
		for ( idx = 0; idx < sizeof(info.hash)/2; idx++ )
		{
		    fprintf(f,"%s%s0x%02x",sep, idx==sizeof(info.hash)/4 ? " " : "", *ptr++ );
		    sep = ",";
		}
	    }
	    fputs(" }},\n",f);
	}
	else
	    fputs(" {0} },\n",f);
    }

    fputs("};\n\n",f);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DumpDOL
(
    FILE		* f,		// dump to this file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    bool		use_c		// true: print in C language fragments
)
{
    DASSERT(f);
    DASSERT(str);

    if (use_c)
	return DumpDOL_c(f,str);

    DASSERT(str->data);
    DASSERT( str->data_size >= sizeof(dol_header_t) );

    DumpDolHeader( f, indent, (dol_header_t*)str->data, str->data_size,
		long_count > 1 ? 7 : long_count > 0 ? 3 : 1 );
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ExtractSTR()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExtractSTR
(
    staticr_t		* str,		// valid pointer
    ccp			dest_dir	// valid destination directory
)
{
    DASSERT(str);
    DASSERT(str->data);
    DASSERT(dest_dir);

    if (str->is_dol)
	return ExtractDOL(str,dest_dir);

    return ERROR0(ERR_WARNING,
	"Extraction of StaticR.rel files is not implemented yet!\n");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ExtractDOL()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExtractDOL
(
    staticr_t		* str,		// valid pointer
    ccp			dest_dir	// valid destination directory
)
{
    DASSERT(str);
    DASSERT(str->data);
    DASSERT(str->is_dol);
    DASSERT(dest_dir);

    CreatePath(dest_dir,true);

    //--- open log file

    char path_buf[PATH_MAX];
    ccp path = PathCatPP(path_buf,sizeof(path_buf),dest_dir,"dol-setup.txt");
    FILE *log = fopen(path,"wb");
    if (!log)
	return ERROR0(ERR_CANT_CREATE,
	    "Can't create setup file: %s\n",path);

    static ccp sep = "#---------------------------------------------------------------------\n";
    fprintf(log,"\n%s#sect offset  address  size  sha1 checksum\n%s",sep,sep);


    //--- extract 'dol-header.bin'

    path = PathCatPP(path_buf,sizeof(path_buf),dest_dir,"dol-header.bin");
    FILE *f = fopen(path,"wb");
    if (!f)
    {
	fclose(log);
	return ERROR0(ERR_CANT_CREATE,
	    "Can't create data file: %s\n",path);
    }
    fwrite(str->data,1,0x100,f);
    fclose(f);


    //--- extract sections

    uint sect;
    for ( sect = 0; sect < DOL_NN_SECTIONS; sect++ )
    {
	if ( sect == DOL_N_TEXT_SECTIONS || sect == DOL_N_SECTIONS )
	    fputs(sep,log);
	dol_sect_info_t info;
	GetSectionDOL(str,&info,sect,true);
	if (!info.sect_valid)
	{
	    //fprintf(log,"%-4s= -\n",info.name);
	    continue;
	}

	fprintf(log,"%-4s= %06x %08x %06x",info.name,info.off,info.addr,info.size);
	if (info.data_valid)
	{
	    if (info.hash_valid)
	    {
		fputc(' ',log);
		uint i;
		for ( i = 0; i < sizeof(info.hash); i++ )
		    fprintf(log,"%02x",info.hash[i]);
	    }

	    path = PathCatPPE(path_buf,sizeof(path_buf),dest_dir,info.name,".bin");
	    FILE *f = fopen(path,"wb");
	    if (!f)
	    {
		fclose(log);
		return ERROR0(ERR_CANT_CREATE,
		    "Can't create data file: %s\n",path);
	    }
	    fwrite(info.data,1,info.size,f);
	    fclose(f);
	}
	fputc('\n',log);
    }
    fputs(sep,log);


    //--- close log and return

    fprintf(log,"\n");
    fclose(log);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ctcode_data_tab[]		///////////////
///////////////////////////////////////////////////////////////////////////////

#undef DEF_DATA
#undef DEF_BZIP
//#undef DEF_PATCH

#define DEF_DATA(a) { #a, 0, a, sizeof(a) },
#define DEF_BZIP(a) { #a, 1, a##_bz2, sizeof(a##_bz2) },
//#define DEF_PATCH(m,a,b) { #a, m, b, sizeof(b), a },

const data_tab_t maindol_data_tab[] =
{
	DEF_BZIP(ctcode_boot_code_pal)
	DEF_BZIP(ctcode_boot_code_usa)
	DEF_BZIP(ctcode_boot_code_jap)

	DEF_BZIP(ctcode_boot_data_pal)
	DEF_BZIP(ctcode_boot_data_usa)
	DEF_BZIP(ctcode_boot_data_jap)

	DEF_BZIP(codehandler)
	DEF_BZIP(codehandleronly)

 #if HAVE_WIIMM_EXT
	DEF_BZIP(lecode_loader_pal)
	DEF_BZIP(lecode_loader_usa)
	DEF_BZIP(lecode_loader_jap)
	DEF_BZIP(lecode_loader_kor)
 #endif

	{0,0,0}
};

#undef DEF_DATA
#undef DEF_BZIP
//#undef DEF_PATCH

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


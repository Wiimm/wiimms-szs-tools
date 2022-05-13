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
 *                   A php library by Dirk Clemens                      *
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

#include "lib-mkw-def.h"

///////////////////////////////////////////////////////////////////////////////

MkwTrackCategory_t * GetMkwTrackCategory ( int id )
{
    static MkwTrackCategory_t id_MTCAT_NINTENDO =
    {
	"Original track of Nintendo",
	0,
	G_MTCAT_NINTENDO,
	"",
	"n",
	"Nintendo",
    };

    static MkwTrackCategory_t id_MTCAT_TEXTURE =
    {
	"Accepted texture hack",
	0,
	G_MTCAT_TEXTURE,
	"green",
	"t",
	"Texture",
    };

    static MkwTrackCategory_t id_MTCAT_DENIED =
    {
	"Denied texture hack",
	0,
	G_MTCAT_DENIED,
	"b_cyan",
	"d",
	"Denied",
    };

    static MkwTrackCategory_t id_MTCAT_EDIT =
    {
	"Announced as texture hack, but with more edits",
	0,
	G_MTCAT_EDIT,
	"b_yellow",
	"e",
	"Texture+",
    };

    static MkwTrackCategory_t id_MTCAT_MODEL =
    {
	"Definitely a model hack",
	0,
	G_MTCAT_MODEL,
	"b_orange",
	"m",
	"Model",
    };

    static MkwTrackCategory_t id_MTCAT_UNKNOWN =
    {
	"Unknown track",
	0,
	G_MTCAT_UNKNOWN,
	"b_red",
	"u",
	"Unknown",
    };

    static MkwTrackCategory_t id_MTCAT_CHEAT =
    {
	"Hack clearly created for cheating",
	0,
	G_MTCAT_CHEAT,
	"b_magenta",
	"▼",
	"Cheat",
    };

    static MkwTrackCategory_t id_MTCAT_CUSTOM =
    {
	"Custom track",
	0,
	G_MTCAT_CUSTOM,
	"cyan_blue",
	"c",
	"Custom",
    };

    static MkwTrackCategory_t id_MTCAT__N =
    {
	"(invalid)",
	0,
	G_MTCAT__N,
	"fail",
	"?",
	"-?-",
    };

    switch(id)
    {
	case G_MTCAT_NINTENDO: return &id_MTCAT_NINTENDO;
	case G_MTCAT_TEXTURE:  return &id_MTCAT_TEXTURE;
	case G_MTCAT_DENIED:   return &id_MTCAT_DENIED;
	case G_MTCAT_EDIT:     return &id_MTCAT_EDIT;
	case G_MTCAT_MODEL:    return &id_MTCAT_MODEL;
	case G_MTCAT_UNKNOWN:  return &id_MTCAT_UNKNOWN;
	case G_MTCAT_CHEAT:    return &id_MTCAT_CHEAT;
	case G_MTCAT_CUSTOM:   return &id_MTCAT_CUSTOM;
	case G_MTCAT__N:       return &id_MTCAT__N;
	default:               return 0;
    };
}


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   sizeof   /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const sizeof_info_t sizeof_info_mkw_def[] =
{
    SIZEOF_INFO_TITLE("MKW/def structs")
	SIZEOF_INFO_ENTRY(MkwTrackCategory_t)
    SIZEOF_INFO_TERM()
};

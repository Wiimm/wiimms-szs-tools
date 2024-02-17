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
 *   Copyright (c) 2014-2024 by Dirk Clemens <develop@cle-mens.de>      *
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
	"",
	0x000000,
	0x00eaea,
	0,
	G_MTCAT_NINTENDO,
	"n",
	"n",
	0x10,
	0,
	"Nin.",
	"Nintendo",
	"Nintendo",
	"b_cyan",
    };

    static MkwTrackCategory_t id_MTCAT_TEXTURE =
    {
	"Accepted texture hack",
	"texture,texture2do,edit.allow",
	0x000000,
	0x20ffa0,
	0,
	G_MTCAT_TEXTURE,
	"t",
	"t",
	0x20,
	0,
	"Tex.",
	"Texture",
	"Texture",
	"green",
    };

    static MkwTrackCategory_t id_MTCAT_TEMP_ALLOW =
    {
	"Temporary allowed texture hack",
	"temp-allow",
	0x000000,
	0x20ffa0,
	0,
	G_MTCAT_TEMP_ALLOW,
	"+",
	"ta",
	0x20,
	0,
	"temp+",
	"Temp-Allow",
	"Texture",
	"green",
    };

    static MkwTrackCategory_t id_MTCAT_TEMP_DENY =
    {
	"Temporary denied texture hack",
	"temp-deny",
	0x000000,
	0xffff00,
	0,
	G_MTCAT_TEMP_DENY,
	"−",
	"td",
	0x20,
	1,
	"temp−",
	"Temp-Deny",
	"Texture",
	"b_yellow",
    };

    static MkwTrackCategory_t id_MTCAT_DENY =
    {
	"Denied texture hack",
	"deny,deny2do",
	0x000000,
	0xffff00,
	0,
	G_MTCAT_DENY,
	"d",
	"d",
	0x20,
	2,
	"Deny",
	"Deny",
	"Texture",
	"b_yellow",
    };

    static MkwTrackCategory_t id_MTCAT_CHEAT =
    {
	"Denied texture hack (created to cheat)",
	"cheat,cheat2do",
	0xffffff,
	0x800000,
	0,
	G_MTCAT_CHEAT,
	"▼",
	"▼",
	0x60,
	5,
	"Cheat",
	"Cheat",
	"Texture",
	"b_red",
    };

    static MkwTrackCategory_t id_MTCAT_EDIT =
    {
	"Track edit",
	"edit,edit2do",
	0x000000,
	0xffc830,
	0,
	G_MTCAT_EDIT,
	"e",
	"e",
	0x20,
	3,
	"Edit",
	"Edit",
	"Edit",
	"b_orange_yellow",
    };

    static MkwTrackCategory_t id_MTCAT_MODEL =
    {
	"Model hack",
	"model,model2do,reverse,reverse2do,tiny,tiny2do,colossal,colossal2do,shrink,shrink2do,stretch,stretch2do",
	0x000000,
	0xff8000,
	0,
	G_MTCAT_MODEL,
	"E",
	"E",
	0x20,
	3,
	"Model",
	"Model",
	"Edit",
	"b_orange",
    };

    static MkwTrackCategory_t id_MTCAT_CHEAT_EDIT =
    {
	"Track edit (created to cheat)",
	"cheat.edit",
	0xffffff,
	0x800000,
	0,
	G_MTCAT_CHEAT_EDIT,
	"▲",
	"▲",
	0x60,
	5,
	"Cht+Ed",
	"Cheat+Edit",
	"Edit",
	"b_red",
    };

    static MkwTrackCategory_t id_MTCAT_UNKNOWN =
    {
	"Unknown track",
	"",
	0x000000,
	0xff80ff,
	0,
	G_MTCAT_UNKNOWN,
	"u",
	"u",
	1,
	4,
	"Unkn.",
	"Unknown",
	"Unknown",
	"b_magenta",
    };

    static MkwTrackCategory_t id_MTCAT_CUSTOM =
    {
	"Custom or ported track",
	"",
	0x000000,
	0xa0a0ff,
	0,
	G_MTCAT_CUSTOM,
	"C",
	"C",
	6,
	4,
	"Custom",
	"Custom",
	"Custom",
	"cyan_blue",
    };

    static MkwTrackCategory_t id_MTCAT_HNS =
    {
	"Hide'n'Seek edit",
	"hns,hns2do",
	0xffffff,
	0x404040,
	0,
	G_MTCAT_HNS,
	"h",
	"h",
	0x20,
	4,
	"H+S",
	"Hide+Seek",
	"Hide+Seek",
	"grey5",
    };

    static MkwTrackCategory_t id_MTCAT_HNS_CT =
    {
	"Custom or ported Hide'n'Seek track",
	"hns.ct",
	0xffff00,
	0x000000,
	0,
	G_MTCAT_HNS_CT,
	"H",
	"H",
	4,
	4,
	"H+S.ct",
	"Hide+Seek.ct",
	"Hide+Seek",
	"grey7",
    };

    static MkwTrackCategory_t id_MTCAT_MISSION =
    {
	"Mission edit",
	"mission,mission2do",
	0xffffff,
	0x404040,
	0,
	G_MTCAT_MISSION,
	"m",
	"m",
	0x20,
	4,
	"Mis",
	"Mission",
	"Mission",
	"grey5",
    };

    static MkwTrackCategory_t id_MTCAT_MISSION_CT =
    {
	"Custom or ported Mission track",
	"mission.ct",
	0xffff00,
	0x000000,
	0,
	G_MTCAT_MISSION_CT,
	"M",
	"M",
	4,
	4,
	"Mis.ct",
	"Mission.ct",
	"Mission",
	"grey7",
    };

    static MkwTrackCategory_t id_MTCAT__N =
    {
	"(invalid)",
	"",
	0xffffff,
	0x500050,
	0,
	G_MTCAT__N,
	"?",
	"?",
	0,
	4,
	"-?-",
	"(invalid)",
	"(invalid)",
	"fail",
    };

    switch(id)
    {
	case G_MTCAT_NINTENDO:   return &id_MTCAT_NINTENDO;
	case G_MTCAT_TEXTURE:    return &id_MTCAT_TEXTURE;
	case G_MTCAT_TEMP_ALLOW: return &id_MTCAT_TEMP_ALLOW;
	case G_MTCAT_TEMP_DENY:  return &id_MTCAT_TEMP_DENY;
	case G_MTCAT_DENY:       return &id_MTCAT_DENY;
	case G_MTCAT_CHEAT:      return &id_MTCAT_CHEAT;
	case G_MTCAT_EDIT:       return &id_MTCAT_EDIT;
	case G_MTCAT_MODEL:      return &id_MTCAT_MODEL;
	case G_MTCAT_CHEAT_EDIT: return &id_MTCAT_CHEAT_EDIT;
	case G_MTCAT_UNKNOWN:    return &id_MTCAT_UNKNOWN;
	case G_MTCAT_CUSTOM:     return &id_MTCAT_CUSTOM;
	case G_MTCAT_HNS:        return &id_MTCAT_HNS;
	case G_MTCAT_HNS_CT:     return &id_MTCAT_HNS_CT;
	case G_MTCAT_MISSION:    return &id_MTCAT_MISSION;
	case G_MTCAT_MISSION_CT: return &id_MTCAT_MISSION_CT;
	case G_MTCAT__N:         return &id_MTCAT__N;
	default:                 return 0;
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

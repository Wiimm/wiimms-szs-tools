
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

#include "dclib-utf8.h"
#include "dclib-regex.h"
#include "lib-ledis.h"
#include "lib-checksum.h"
#include "lib-szs.h"
#include "db-mkw.h"
#include "crypt.h"

#include "ledis.inc"
#include "distrib.inc"

#include <dirent.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			global vars			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp		opt_create_dump		= 0;	// NULL of filename
le_distrib_t	ledis_dump		= {0};	// data collector for 'opt_create_dump'
int		ledis_dump_enabled	= 0;	// -1:done, 0:disabled, 1:enabled

ccp		opt_le_define		= 0;	// NULL or last defined filename
StringField_t	le_define_list		= {0};	// list with all filenames

///////////////////////////////////////////////////////////////////////////////

static const le_cup_ref_t standard_bt_ref[MKW_N_ARENAS] =
	{ 33,32,35,34,36, 39,40,41,37,38 };

static const le_cup_ref_t standard_vs_ref[MKW_N_TRACKS] =
	{ 8, 1, 2, 4,  16,20,25,26,   0, 5, 6, 7,  27,31,23,18,
	  9,15,11, 3,  21,30,29,17,  14,10,12,13,  24,22,19,28 };

static const le_cup_ref_t mkwfun_random_ref[MKW_N_LE_RANDOM] =
	{ 62, 63, 64, 65 };

///////////////////////////////////////////////////////////////////////////////

#if LE_TYPE_MARKERS_ENABLED
  #define LTT_MARKER_ARENA	0x1000
  #define LTT_MARKER_TRACK	0x2000
  #define LTT_MARKER__ALL	0x3000
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////				helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

#define DUMP_CUP_SLOTS_ENABLED 0

#if DUMP_CUP_SLOTS_ENABLED
 static void DumpCupSlots ( const le_distrib_t *ld, ccp info )
 {
    fflush(stdout);
    fflush(stderr);

    const int min = 68;
    const int max = min + 15;

    fprintf(stdlog,"CUP-SLOT %d..%d:",min,max);
    for ( int slot = min; slot <= max; slot++ )
    {
	le_track_t *lt = GetTrackLD((le_distrib_t*)ld,slot);
	if (lt)
	    fprintf(stdlog," %d",lt->cup_slot);
	else
	    fputs(" -",stdlog);
    }

    if (info)
	fprintf(stdlog,"  # %s\n",info);
    else
	fputs("\n",stdlog);
    fflush(stdlog);
 }
#else
 static inline void DumpCupSlots ( le_distrib_t const *ld, ccp info ) {}
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_track_type_t		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetNameLTTY ( le_track_type_t track_type )
{
    return track_type & LTTY_TRACK
	? "vs"
	: track_type & LTTY_ARENA
	? "bt"
	: track_type & LTTY_RANDOM
	? "rd"
	: "xx";
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			le_options_t			///////////////
///////////////////////////////////////////////////////////////////////////////

le_options_t defaultLEO = LEO__DEFAULT;

//-----------------------------------------------------------------------------

static KeywordTab_t keywords_leo[] =
{
	{ LEO__DEFAULT,		"RESET",	0,		~0 },
	{ 0,			"CLEAR",	0,		~0 },

	{ LTT_SHA1,		"SHA1",		0,		LEO_LTT_SELECTOR },
	{ LTT_SHA1_D,		"D-SHA1",	"DSHA1",	LEO_LTT_SELECTOR },
	{ LTT_IDENT,		"IDENT",	0,		LEO_LTT_SELECTOR },
	{ LTT_IDENT_D,		"D-IDENT",	"DIDENT",	LEO_LTT_SELECTOR },
	{ LTT_FILE,		"FILES",	0,		LEO_LTT_SELECTOR },
	{ LTT_NAME,		"NAMES",	0,		LEO_LTT_SELECTOR },
	{ LTT_XNAME,		"XNAMES",	0,		LEO_LTT_SELECTOR },
	{ LTT_XNAME2,		"X2NAMES",	0,		LEO_LTT_SELECTOR },
 #if LE_STRING_LIST_ENABLED
	{ LTT_TEMP1,		"TEMP1",	0,		LEO_LTT_SELECTOR },
	{ LTT_TEMP2,		"TEMP2",	0,		LEO_LTT_SELECTOR },
 #endif

	{ LEO_MKW,		"MKW",		0,		0 },
	{ LEO_MKW1,		"MKW1",		0,		0 },
	{ LEO_MKW2,		"MKW2",		0,		0 },
	{ LEO_CTCODE,		"CT-CODE",	"CTCODE",	0 },
	{ LEO_LECODE,		"LE-CODE",	"LECODE",	0 },

	{ LEO_IN_EMPTY,		"IN-EMPTY",	"INEMPTY",	0 },
	 { LEO_IN_EMPTY,	"IEMPTY",	0,		0 },
	{ LEO_IN_MINUS,		"IN-MINUS",	"INMINUS",	0 },
	 { LEO_IN_MINUS,	"IMINUS",	0,		0 },
	{ LEO_IN_ALL,		"IN-ALL",	"INALL",	0 },
	 { LEO_IN_ALL,		"IALL",		0,		0 },

	{ LEO_OVERWRITE,	"OVERWRITE",	0,		LEO_M_HOW },
	{ LEO_INSERT,		"INSERT",	0,		LEO_M_HOW },
	{ LEO_REPLACE,		"REPLACE",	0,		LEO_M_HOW },

	{ LEO_VERSUS,		"VERSUS",	"VS",		0 },
	{ LEO_BATTLE,		"BATTLE",	"BT",		0 },
	{ LEO_CUSTOM,		"CUSTOM",	0,		0 },
	{ LEO_ORIGINAL,		"ORIGINAL",	0,		0 },
	{ LEO_NO_D_FILES,	"NO-D",		"NOD",		0 },

	{ LEO_OUT_EMPTY,	"OUT-EMPTY",	"OUTEMPTY",	0 },
	 { LEO_OUT_EMPTY,	"OEMPTY",	0,		0 },
	{ LEO_OUT_MINUS,	"OUT-MINUS",	"OUTMINUS",	0 },
	 { LEO_OUT_MINUS,	"OMINUS",	0,		0 },
	{ LEO_OUT_ALL,		"OUT-ALL",	"OUTALL",	0 },
	 { LEO_OUT_ALL,		"OALL",		0,		0 },

	{ LEO_IN_LECODE,	"IN-LECODE",	"INLECODE",	0 },
	 { LEO_IN_LECODE,	"ILECODE",	0,		0 },
	{ LEO_BRIEF,		"BRIEF",	0,		0 },

	{ LEO_HELP,		"HELP",	    	"H",		0 },

	{ LEO_CUT_STD,		"CUT-STD",	"CUTSTD",	0 },
	{ LEO_CUT_CTCODE,	"CUT-CTCODE",	"CUTCTCODE",	0 },

	{0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

le_options_t SetupDefaultLEO(void)
{
    defaultLEO = LEO__DEFAULT;
    if ( !print_header || brief_count )
	defaultLEO |= LEO_BRIEF;
    return defaultLEO;
}

///////////////////////////////////////////////////////////////////////////////

le_options_t ScanLEO ( le_options_t current, ccp arg, ccp err_info )
{
    if (!arg)
	return current;

    keywords_leo[0].id = SetupDefaultLEO();

    s64 stat = ScanKeywordList(arg,keywords_leo,0,true,0,current,err_info,ERR_SYNTAX);
    PRINT0("ScanLEO(%#05x,%s) => %#05x\n",current,arg,(le_options_t)stat);
    return stat != -1 ? stat : current;
}

///////////////////////////////////////////////////////////////////////////////

bool IsInputNameValidLEO ( ccp name, le_options_t opt )
{
    return !name || !*name
	? ( opt & LEO_IN_EMPTY ) != 0
	: *name == '-' && !name[1]
	? ( opt & LEO_IN_MINUS ) != 0
	: true;
}

///////////////////////////////////////////////////////////////////////////////

bool IsOutputNameValidLEO ( ccp name, le_options_t opt )
{
    return !name || !*name
	? ( opt & LEO_OUT_EMPTY ) != 0
	: *name == '-' && !name[1]
	? ( opt & LEO_OUT_MINUS ) != 0
	: true;
}

///////////////////////////////////////////////////////////////////////////////

bool IsAssignmentAllowedLEO ( ccp cur_name, ccp new_name, le_options_t opt )
{
    if (!IsInputNameValidLEO(new_name,opt))
	return false;

    switch ( opt & LEO_M_HOW )
    {
	case 0:
	case LEO_OVERWRITE: return true;
	case LEO_INSERT:    return !IsOutputNameValidLEO(cur_name,opt);
	case LEO_REPLACE:   return IsOutputNameValidLEO(cur_name,opt);
	default:	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

ccp GetUpperNameLTT ( le_track_text_t ltt )
{
    switch ( ltt & LEO_LTT_SELECTOR )
    {
	case LTT_SHA1:		return "SHA1";
	case LTT_SHA1_D:	return "D-SHA1";
	case LTT_IDENT:		return "IDENT";
	case LTT_IDENT_D:	return "D-IDENT";
	case LTT_FILE:		return "FILE";
	case LTT_NAME:		return "NAME";
	case LTT_XNAME:		return "XNAME";
	case LTT_XNAME2:	return "XNAME2";
     #if LE_STRING_LIST_ENABLED
	case LTT_TEMP1:		return "TEMP1";
	case LTT_TEMP2:		return "TEMP2";
     #endif
    }
    return 0;
}

//-----------------------------------------------------------------------------

ccp GetLowerNameLTT ( le_track_text_t ltt )
{
    switch ( ltt & LEO_LTT_SELECTOR )
    {
	case LTT_SHA1:		return "sha1";
	case LTT_SHA1_D:	return "d-sha1";
	case LTT_IDENT:		return "ident";
	case LTT_IDENT_D:	return "d-ident";
	case LTT_FILE:		return "file";
	case LTT_NAME:		return "name";
	case LTT_XNAME:		return "xname";
	case LTT_XNAME2:	return "xname2";
     #if LE_STRING_LIST_ENABLED
	case LTT_TEMP1:		return "temp1";
	case LTT_TEMP2:		return "temp2";
     #endif
    }
    return 0;
}

//-----------------------------------------------------------------------------

int GetDModeLTT ( le_track_text_t ltt )
{
    // returns 0:unknown ltt, 1:std string, 2:_d string, 3:both

    switch ( ltt & LEO_LTT_SELECTOR )
    {
	case LTT_SHA1:
	case LTT_IDENT:
	    return 1;

	case LTT_SHA1_D:
	case LTT_IDENT_D:
	    return 2;

	case LTT_FILE:
	case LTT_NAME:
	case LTT_XNAME:
	case LTT_XNAME2:
     #if LE_STRING_LIST_ENABLED
	case LTT_TEMP1:
	case LTT_TEMP2:
     #endif
	    return 3;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetTextLEO ( le_options_t opt )
{
    char buf[300], *dest = buf, *end = buf + sizeof(buf) - 1;

    static const le_options_t ctab[] =
	{ LEO_IN_ALL, LEO_M_ARENA, LEO_M_ORIGINAL, LEO_OUT_ALL, 0 };

    for ( const le_options_t *cptr = ctab; *cptr; cptr++ )
	if ( ( opt & *cptr ) == *cptr )
	    opt &= ~*cptr;

    *dest = 0;
    PrintKeywordList(dest,end-dest,0,keywords_leo,opt,0,INT64_MIN);
    if ( dest > buf && !*dest )
	*--dest = 0;
    return CopyCircBuf0(buf,strlen(buf));
}

///////////////////////////////////////////////////////////////////////////////

ccp GetInputFilterLEO ( le_options_t opt )
{
    static const le_options_t mask
		= LEO_M_BMG
		| LEO_IN_ALL
		| LEO_M_HOW
		;
    return GetTextLEO(opt&mask);
}

///////////////////////////////////////////////////////////////////////////////

ccp GetOutputFilterLEO ( le_options_t opt )
{
    static const le_options_t mask
		= LEO_M_ARENA
		| LEO_M_ORIGINAL
		| LEO_NO_D_FILES
		| LEO_OUT_ALL
		;
    return GetTextLEO(opt&mask);
}


///////////////////////////////////////////////////////////////////////////////

bool GetBmgBySlotLEO
	( char *buf, uint bufsize, const bmg_t *bmg, int slot, le_options_t opt )
{
    if ( !bmg || slot < 0 )
	return 0;

    if (!(opt&LEO_M_BMG))
	opt |= LEO_M_BMG;


    //-- LE-CODE

    if ( opt & LEO_LECODE && slot < BMG_N_LE_TRACK )
    {
	const bmg_item_t *bi = FindItemBMG(bmg,slot+MID_LE_TRACK_BEG);
	if ( bi && bi->text )
	{
	    PrintString16BMG(buf,bufsize,bi->text,bi->len,BMG_UTF8_MAX,0,1);
	    if (IsInputNameValidLEO(buf,opt))
		return true;
	}
    }


    //-- CT-CODE

    if ( opt & LEO_CTCODE && slot < BMG_N_CT_TRACK )
    {
	const bmg_item_t *bi = FindItemBMG(bmg,slot+MID_CT_TRACK_BEG);
	if ( bi && bi->text )
	{
	    PrintString16BMG(buf,bufsize,bi->text,bi->len,BMG_UTF8_MAX,0,1);
	    if (IsInputNameValidLEO(buf,opt))
		return true;
	}
    }


    //-- MKW2

    if ( opt & LEO_MKW2 )
    {
	const bmg_item_t *bi
		= IsMkwTrack(slot)
		? FindItemBMG(bmg,slot+MID_TRACK1_BEG)
		: IsMkwArena(slot)
		? FindItemBMG(bmg,slot-BMG_N_TRACK+MID_ARENA1_BEG)
		: 0;

	if ( bi && bi->text )
	{
	    PrintString16BMG(buf,bufsize,bi->text,bi->len,BMG_UTF8_MAX,0,1);
	    if (IsInputNameValidLEO(buf,opt))
		return true;
	}
    }


    //-- MKW1

    if ( opt & LEO_MKW1 )
    {
	const bmg_item_t *bi
		= IsMkwTrack(slot)
		? FindItemBMG(bmg,slot+MID_TRACK2_BEG)
		: IsMkwArena(slot)
		? FindItemBMG(bmg,slot+MID_ARENA2_BEG)
		: 0;

	if ( bi && bi->text )
	{
	    PrintString16BMG(buf,bufsize,bi->text,bi->len,BMG_UTF8_MAX,0,1);
	    if (IsInputNameValidLEO(buf,opt))
		return true;
	}
    }

    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_strpar_t		///////////////
///////////////////////////////////////////////////////////////////////////////

const le_strpar_t NullLSP	= {0};
const le_strpar_t DefaultLSP	= { .opt = LEO__DEFAULT };

///////////////////////////////////////////////////////////////////////////////

void ResetLSP ( le_strpar_t *par )
{
    if (par)
    {
     #if LE_STRING_SET_ENABLED
	FreeString(par->key);
     #endif
	InitializeLSP(par,par->opt);
    }
}

///////////////////////////////////////////////////////////////////////////////

char * ScanLSP ( le_strpar_t *par, ccp arg, enumError *ret_err )
{
    static const KeywordTab_t keytab[] =
    {
	{ LTT_SHA1,		"SHA1",		0,		0 },
	{ LTT_SHA1_D,		"D-SHA1",	"DSHA1",	0 },
	{ LTT_IDENT,		"IDENT",	0,		0 },
	{ LTT_IDENT_D,		"D-IDENT",	"DIDENT",	0 },
	{ LTT_FILE,		"FILES",	0,		0 },
	{ LTT_NAME,		"NAMES",	0,		0 },
	{ LTT_XNAME,		"XNAMES",	0,		0 },
	{ LTT_XNAME2,		"X2NAMES",	0,		0 },
     #if LE_STRING_LIST_ENABLED
	{ LTT_TEMP1,		"TEMP1",	0,		0 },
	{ LTT_TEMP2,		"TEMP2",	0,		0 },
     #endif
	{0,0,0,0}
    };

    //--------------

    if ( !par || !arg )
	return (char*)arg;

    ccp src = arg;
    while ( *src > 0 && *src <= ' ' )
	src++;

    enumError err = ERR_OK;

 #if LE_STRING_SET_ENABLED
    if ( *src == '[' )
    {
	src++;
	ccp end = strchr(src,']');
	if (!end)
	    err = ERR_SYNTAX;
	else
	{
	    SetKeyNameLSP(par,src,end-src);
	    src = end+1;
	}
    }
    else
 #endif
    {
	char name[50], *nptr = name;
	while ( isalnum((int)*src) && nptr < name + sizeof(name) -2 )
	    *nptr++ = *src++;
	*nptr = 0;

	int abbrev_count;
	const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,keytab);

	if (!cmd)
	    err = ERR_SEMANTIC;
	else
	    par->opt = par->opt & ~LEO_LTT_SELECTOR | cmd->id;
    }

    //--- terminate

    if (ret_err)
	*ret_err = err;

    if ( err == ERR_OK )
    {
	while ( *src > 0 && *src <= ' ' )
	    src++;
	arg = src;
    }

    return (char*)arg;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetNameLSP ( const le_strpar_t *par )
{
    if (par)
    {
	ccp name = GetLowerNameLTT(par->opt);
	if (name)
	    return name;

     #if LE_STRING_SET_ENABLED
	if (IsValidSetLSP(par))
	    return PrintCircBuf("[%s]",par->key);
     #endif
    }
    return "?";
}

///////////////////////////////////////////////////////////////////////////////

ccp GetOptionsLSP ( const le_strpar_t *par, le_options_t mask )
{
    if (!par)
	return 0;

 #if LE_STRING_SET_ENABLED
    if (IsValidSetLSP(par))
	return PrintCircBuf("[%s],%s",
		par->key, GetTextLEO(par->opt&~LEO_LTT_SELECTOR&mask) );
 #endif
    return GetTextLEO(par->opt&mask);
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanOptionsLSP ( le_strpar_t *par, ccp arg, ccp err_info )
{
    if ( !par || !arg )
	return ERR_MISSING_PARAM;

 #if LE_STRING_SET_ENABLED
    while ( *arg > 0 && *arg <= ' ' )
	arg++;

    if ( *arg == '[' )
    {
	enumError err;
	arg = ScanLSP(par,arg,&err);
	if (err)
	    return err;
    }
 #endif

    par->opt = ScanLEO(par->opt,arg,err_info);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_SET_ENABLED

 void SetKeyNameLSP ( le_strpar_t *par, ccp key, int key_len )
 {
    if ( par && par->key != key )
    {
	FreeString(par->key);
	if ( key && *key )
	{
	    if ( key_len < 0 )
		key_len = strlen(key);
	    par->key = MEMDUP(key,key_len);
	    par->opt = par->opt & ~LEO_LTT_SELECTOR | LTT_STRSET;
	}
	else
	    par->key = 0;
    }
 }

#endif
//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct le_cup_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLECUP ( le_cup_t *lc, uint max_cups, uint tracks_per_cup )
{
    DASSERT(lc);
    memset(lc,0,sizeof(*lc));
    lc->max		= max_cups;
    lc->tracks		= tracks_per_cup;
    lc->fill_slot	= -1;
    lc->fill_src	= -1;
    lc->dirty		= true;
}

///////////////////////////////////////////////////////////////////////////////

void ResetLECUP ( le_cup_t *lc )
{
    if (lc)
    {
	FREE(lc->list);
	InitializeLECUP(lc,lc->max,lc->tracks);
    }
}

///////////////////////////////////////////////////////////////////////////////

le_cup_ref_t * GetLECUP ( const le_cup_t *lc, uint cup_index )
{
    return lc && cup_index < lc->used
	? lc->list + cup_index * lc->tracks
	: 0;
}

///////////////////////////////////////////////////////////////////////////////

le_cup_ref_t * DefineLECUP ( le_cup_t *lc, uint cup_index )
{
    if ( !lc || cup_index >= lc->max )
	return 0;

    const uint size_per_cup = sizeof(*lc->list) * lc->tracks;

    if ( cup_index >= lc->size )
    {
	uint n_cups = cup_index + lc->size/4 + 100;
	if ( n_cups > lc->max )
	     n_cups = lc->max;

	lc->list = REALLOC( lc->list, n_cups * size_per_cup );
	lc->size = n_cups;
	ASSERT( lc->size <= lc->max );
    }
    ASSERT( cup_index < lc->size );

    if ( cup_index >= lc->used )
    {
	lc->dirty = true;
	le_cup_ref_t *beg = lc->list + lc->used * lc->tracks;
	lc->used = cup_index + 1;
	ASSERT( lc->used <= lc->size );
	le_cup_ref_t *end = lc->list + lc->used * lc->tracks;
	memset( beg, ~0, (u8*)end-(u8*)beg );
    }
    ASSERT( cup_index < lc->used );

    return lc->list + cup_index * lc->tracks;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint GetCupSlotLECUP ( const le_cup_t *lc, const le_cup_ref_t *ptr )
{
    if ( !lc || !lc->list || !ptr )
	return 0;

    const uint idx = ptr - lc->list;
    const uint cup = idx / lc->tracks;
    return cup < lc->used
	? 10 * cup + (idx % lc->tracks) + 11
	: 0;
}

///////////////////////////////////////////////////////////////////////////////

void AppendLECUP ( le_cup_t *lc, le_cup_ref_t ref )
{
    if ( lc && ref >= 0 )
    {
	if ( lc->cur_index >= lc->tracks )
	{
	    lc->cur_index = 0;
	    lc->cur_cup++;
	}

	le_cup_ref_t *cr = DefineLECUP(lc,lc->cur_cup);
	if (cr)
	{
	    lc->dirty = true;
	    cr += lc->cur_index++;
	    *cr = ref;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void CloseLECUP ( le_cup_t *lc )
{
    if (lc)
    {
	lc->close_used = true;
	if ( lc->cur_index )
	    lc->cur_index = lc->tracks;
    }
}

///////////////////////////////////////////////////////////////////////////////

void PackLECUP ( le_cup_t *lc )
{
    if ( !lc || !lc->used )
	return;

    PRINT("PackLECUP(%d) < N=%d\n",lc->tracks,lc->used);
    le_cup_ref_t *dest = lc->list;
    le_cup_ref_t *end  = lc->list + lc->used * lc->tracks;

    for ( le_cup_ref_t *src = lc->list; src < end; src++ )
	if ( *src >= 0 )
	    *dest++ = *src;

    lc->used = ( dest - lc->list + lc->tracks - 1 ) / lc->tracks;
    end = lc->list + lc->used * lc->tracks;
    while ( dest < end )
	*dest++ = -1;
    PRINT("PackLECUP(%d) > N=%d\n",lc->tracks,lc->used);
}

///////////////////////////////////////////////////////////////////////////////

void EvenLECUP ( le_cup_t *lc )
{
    if (lc)
    {
	DefineLECUP( lc, lc->tracks == 4 ? 5 : 1 );
	if ( lc->used & 1 )
	    DefineLECUP(lc,lc->used);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool HaveInvalidLECUP ( le_cup_t *lc )
{
    if (lc)
    {
	le_cup_ref_t *end = lc->list + lc->used * lc->tracks;
	for ( le_cup_ref_t *ptr = lc->list; ptr < end; ptr++ )
	    if ( *ptr < 0 )
		return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void FillLECUP ( le_cup_t *lc, le_cup_ref_t fallback )
{
    if (!lc)
	return;

    le_cup_ref_t *end = lc->list + lc->used * lc->tracks;
    for ( le_cup_ref_t *ptr = lc->list; ptr < end; ptr++ )
	if ( *ptr < 0 )
	    *ptr = fallback;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			le_track_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLT ( le_track_t *lt )
{
    DASSERT(lt);
    memset(lt,0,sizeof(*lt));
    lt->track_status = LTS_VALID;
 #if LE_STRING_SET_ENABLED
    InitializeParamField(&lt->str_set);
    lt->str_set.free_data = true;
 #endif
}

///////////////////////////////////////////////////////////////////////////////

void ClearStringsLT ( le_track_t *lt )
{
    if (lt)
    {
	FreeString(lt->file);
	FreeString(lt->name);
	FreeString(lt->xname);
	FreeString(lt->lti[0].ident);
	FreeString(lt->lti[1].ident);

	lt->file = 0;
	lt->name = 0;
	lt->xname = 0;
	memset(lt->lti,0,sizeof(*lt->lti));

     #if LE_STRING_LIST_ENABLED
	for ( int idx = 0; idx < LTT__LIST_SIZE; idx++ )
	    FreeString(lt->str_list[idx]);
	memset(lt->str_list,0,sizeof(lt->str_list));
     #endif

     #if LE_STRING_SET_ENABLED
	ResetParamField(&lt->str_set);
     #endif
    }
}

///////////////////////////////////////////////////////////////////////////////

void ClearLT ( le_track_t *lt, le_track_type_t ltty ) // auto-ltty if 0
{
    if (lt)
    {
	ClearStringsLT(lt);

	const int slot = lt->track_slot;
	memset(lt,0,sizeof(*lt));
	lt->track_slot = slot;
	lt->track_type = ltty ? ltty : IsMkwArena(slot) ? LTTY_ARENA : LTTY_TRACK;
    }
}

///////////////////////////////////////////////////////////////////////////////

void MoveLT ( le_track_t *dest, le_track_t *src )
{
    if ( dest && src && dest != src )
    {
	ClearLT(dest,0);
	memcpy(dest,src,sizeof(*dest));

	src->file		= 0;
	src->name		= 0;
	src->xname		= 0;
	src->lti[0].ident	= 0;
	src->lti[1].ident	= 0;
	ClearLT(src,0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetupStandardArenaLT ( le_track_t *lt, uint setup_slot )
{
    if (IsMkwArena(setup_slot))
	setup_slot -= MKW_ARENA_BEG;
    if ( setup_slot < MKW_N_ARENAS )
	SetupByTrackInfoLT(lt,arena_info+setup_slot);
}

///////////////////////////////////////////////////////////////////////////////

void SetupStandardTrackLT ( le_track_t *lt, uint setup_slot )
{
    if (IsMkwTrack(setup_slot))
	SetupByTrackInfoLT(lt,track_info+setup_slot);
    else if (IsMkwArena(setup_slot))
    {
	setup_slot -= MKW_ARENA_BEG;
	SetupByTrackInfoLT(lt,arena_info+setup_slot);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetupByTrackInfoLT ( le_track_t *lt, const TrackInfo_t *ti )
{
    if ( !lt || !ti )
	return;

    lt->cup_slot	= ti->def_slot;
    lt->track_type	= IsMkwArena(ti->track_id) ? LTTY_ARENA : LTTY_TRACK;
    lt->property	= ti->track_id;
    lt->music		= ti->music_id;

    le_strpar_t spar = { .opt = LEO__DEFAULT|LEO_IN_ALL };
    SetFileLT(lt,&spar,ti->track_fname);
    SetNameLT(lt,&spar,ti->name_en);

    sha1_hex_t hex;
    Sha1Bin2Hex(hex,ti->sha1[0]);
    SetIdentOptLT(lt,hex,false,false,LEO_IN_ALL);

    const u32 have_d = be32(ti->sha1[TI_SHA1_IS_D]);
    if (have_d)
    {
	Sha1Bin2Hex(hex,ti->sha1[TI_SHA1_IS_D]);
	SetIdentOptLT(lt,hex,true,false,LEO_IN_ALL);
    }

    // last action because SetIdent*LT() clears 'is_original'
    lt->is_original = lt->lti[0].orig_sha1 = lt->lti[1].orig_sha1 = lt->track_slot == ti->track_id;
}

///////////////////////////////////////////////////////////////////////////////

void SetupLecodeRandomTrackLT ( le_track_t *lt, uint setup_slot )
{
    if (IsLecodeRandom(setup_slot))
    {
	lt->track_status = LTS_ACTIVE;
	lt->track_type	= LTTY_RANDOM|LTTY_TRACK;
	lt->cup_slot	= setup_slot - MKW_LE_RANDOM_BEG + 91;
	lt->property	= 0;
	lt->music	= MKW_MUSIC_MIN_ID;

	static ccp names[] =
	{
		"Random: All Tracks",
		"Random: Original Tracks",
		"Random: Custom Tracks",
		"Random: New Tracks",
	};

	if (IsLecodeRandom(setup_slot))
	    SetNameLT(lt,0,names[setup_slot-MKW_LE_RANDOM_BEG]);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// -1:wrong slot or not a battle, 0:correct slots,
// 1:wrong property, 2:wrong music, 3:both wrong

int HaveUsualBattleSlots ( le_track_t *lt )
{
    if ( !lt || !IsArenaLTTY(lt->track_type) || !IsMkwArena(lt->track_slot) )
	return -1;

    const TrackInfo_t *ai = arena_info + lt->track_slot - MKW_ARENA_BEG;
    return ( lt->property == lt->track_slot ? 0 : 1 )
	 | ( lt->music    == ai->music_id   ? 0 : 2 );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearIdentLT ( le_track_t *lt )
{
    FreeString(lt->lti[0].ident);
    FreeString(lt->lti[1].ident);
    lt->lti[0].ident = lt->lti[1].ident = 0;

    lt->lti[0].have_sha1 = lt->lti[1].have_sha1 = 0;
    lt->lti[0].orig_sha1 = lt->lti[1].orig_sha1 = 0;
    lt->is_original = 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Set*LT() functions		///////////////
///////////////////////////////////////////////////////////////////////////////

void SetIdentOptLT
	( le_track_t *lt, ccp ident, bool use_d, bool csum_only, le_options_t opt )
{
    le_strpar_t spar = { .opt = opt, .use_d = use_d, .csum_only = csum_only };
    SetIdentLT(lt,&spar,ident);
}

//-----------------------------------------------------------------------------

void SetIdentLT ( le_track_t *lt, const le_strpar_t *par, ccp ident )
{
    // par: opt, use_d, csum_only
    if (!lt)
	return;
    if (!par)
	par = &DefaultLSP;

    le_track_id_t *lti = lt->lti + (par->use_d>0);
    if ( IsAssignmentAllowedLEO(lti->ident,ident,par->opt) )
    {
	if (!par->use_d)
	    ClearIdentLT(lt);

	if (ident)
	{
	    while ( *ident == ' ' || *ident == '\t' )
		ident++;

	    sha1_size_t sha1size;
	    const int stat = IsSSChecksum(&sha1size,ident,-1);
	    if (stat)
	    {
		lti->have_sha1 = true;
		memcpy(lti->sha1bin,sha1size.hash,sizeof(lti->sha1bin));
		FreeString(lti->ident);
		lti->ident = stat == 2 ? STRDUP(ident) : 0;
	    }
	    else
	    {
		lti->have_sha1 = false;
		memset(lti->sha1bin,0,sizeof(lti->sha1bin));
		FreeString(lti->ident);
		lti->ident = par->csum_only ? 0 : STRDUP(ident);
	    }

	    //-- calculate is_orig

	    bool is_orig = false;
	    if (lti->have_sha1)
	    {
		const TrackInfo_t *ti =
			  IsMkwTrack(lt->track_slot)
			? track_info + lt->track_slot
			: IsMkwArena(lt->track_slot)
			? arena_info + lt->track_slot - MKW_ARENA_BEG
			: 0;

		if (ti)
		{
		    const u8 *sptr = ti->sha1[ par->use_d ? TI_SHA1_IS_D : 0 ];
		    if (lti->have_sha1)
			is_orig = !memcmp(lti->sha1bin,sptr,sizeof(lti->sha1bin))
			       || !memcmp(lti->sha1bin,sptr+TI_SHA1_NORM,sizeof(lti->sha1bin));
		    else
			is_orig = !*(u32*)sptr;
		}
	    }

	    lti->orig_sha1 = is_orig;
	}
	lt->is_original = lt->lti[0].orig_sha1 && lt->lti[1].orig_sha1;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetFileLT ( le_track_t *lt, const le_strpar_t *par, ccp fname )
{
    // par: opt

    if (!par)
	par = &DefaultLSP;

    if ( lt && IsAssignmentAllowedLEO(lt->file,fname,par->opt) )
    {
	FreeString(lt->file);
	lt->file = STRDUP(fname);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetNameLT ( le_track_t *lt, const le_strpar_t *par, ccp name )
{
    // par: opt

    if (!par)
	par = &DefaultLSP;

    if ( lt && IsAssignmentAllowedLEO(lt->name,name,par->opt) )
    {
	FreeString(lt->name);
	if ( lt->xname && !strcmp(lt->xname,name) )
	{
	    lt->name  = lt->xname;
	    lt->xname = 0;
	}
	else
	    lt->name = STRDUP(name);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetXNameLT ( le_track_t *lt, const le_strpar_t *par, ccp xname )
{
    // par: opt

    if (!par)
	par = &DefaultLSP;

    if ( lt && IsAssignmentAllowedLEO(lt->xname,xname,par->opt) )
    {
	FreeString(lt->xname);
	if ( !lt->name || strcmp(lt->name,xname) )
	    lt->xname = STRDUP(xname);
    }
}

///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_LIST_ENABLED

 void SetListLT ( le_track_t *lt, const le_strpar_t *par, ccp text )
 {
    // par: ltt, opt

    if (!par)
	par = &DefaultLSP;

    const le_track_text_t ltt = par->opt & LEO_LTT_SELECTOR;

    if	( lt && ltt >= LTT__LIST_BEG && ltt < LTT__LIST_END )
    {
	ccp *ptr = lt->str_list + ltt - LTT__LIST_BEG;
	if (IsAssignmentAllowedLEO(*ptr,text,par->opt))
	{
	    FreeString(*ptr);
	    *ptr = STRDUP(text);
	}
    }
 }

#endif // LE_STRING_LIST_ENABLED
///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_SET_ENABLED

 void SetSetLT ( le_track_t *lt, const le_strpar_t *par, ccp text )
 {
    // par: ltt, key, opt
    if ( lt && par && par->key )
    {
	PRINT0(" %u [%s] = %s\n",lt->track_slot,par->key,text);
	ReplaceParamField(&lt->str_set,par->key,false,0,STRDUP(text));
    }
 }

 //----------------------------------------------------------------------------

 void SetByKeyLT ( le_track_t *lt, ccp key, ccp text )
 {
    le_strpar_t par = { .key = key };
    SetSetLT(lt,&par,text);
 }

#endif // LE_STRING_SET_ENABLED
///////////////////////////////////////////////////////////////////////////////

void SetTextOptLT ( le_track_t *lt, ccp text, le_options_t opt )
{
    le_strpar_t par = { .opt = opt };
    SetTextLT(lt,&par,text);
}

//-----------------------------------------------------------------------------

void SetTextLT ( le_track_t *lt, const le_strpar_t *par, ccp text )
{
    // par: ltt, *

    le_strpar_t mypar = {0};
    if (par)
	mypar = *par;

    const le_track_text_t ltt = mypar.opt & LEO_LTT_SELECTOR;
    switch (ltt)
    {
	case LTT_SHA1:
	    mypar.use_d     = false;
	    mypar.csum_only = true;
	    SetIdentLT(lt,&mypar,text);
	    break;

	case LTT_IDENT:
	    mypar.use_d     = false;
	    mypar.csum_only = false;
	    SetIdentLT(lt,&mypar,text);
	    break;

	case LTT_SHA1_D:
	    mypar.use_d     = true;
	    mypar.csum_only = true;
	    SetIdentLT(lt,&mypar,text);
	    break;

	case LTT_IDENT_D:
	    mypar.use_d     = true;
	    mypar.csum_only = false;
	    SetIdentLT(lt,&mypar,text);
	    break;

	case LTT_FILE:
	    SetFileLT(lt,&mypar,text);
	    break;

	case LTT_NAME:
	    SetNameLT(lt,&mypar,text);
	    break;

	case LTT_XNAME:
	case LTT_XNAME2:
	    SetXNameLT(lt,&mypar,text);
	    break;

     #if LE_STRING_SET_ENABLED
	case LTT_STRSET:
	    SetSetLT(lt,&mypar,text);
	    break;
     #endif

	default:
	 #if LE_STRING_LIST_ENABLED
	    if ( ltt >= LTT__LIST_BEG && ltt < LTT__LIST_END )
		SetListLT(lt,&mypar,text);
	 #endif
	    break;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Get*LT() functions		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetSha1LT ( const le_track_t *lt, bool use_d, ccp return_on_empty )
{
    const le_track_id_t *lti = lt->lti + (use_d>0);
    if ( !lt || lt->track_status < LTS_EXPORT || !lti->have_sha1 )
	return return_on_empty;

    char *buf = GetCircBuf(sizeof(sha1_hex_t));
    Sha1Bin2Hex(buf,lti->sha1bin);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetIdentLT ( const le_track_t *lt, bool use_d, bool use_sha1, ccp return_on_empty )
{
    const le_track_id_t *lti = lt->lti + (use_d>0);
    if ( !lt || lt->track_status < LTS_EXPORT )
	return return_on_empty;

    return lti->ident
	? ( *lti->ident ? lti->ident : return_on_empty )
	: use_sha1
	? GetSha1LT(lt,use_d,return_on_empty)
	: return_on_empty;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetFileLT ( const le_track_t *lt, ccp return_on_empty )
{
    return lt && lt->track_status >= LTS_EXPORT && lt->file && *lt->file
	? lt->file
	: return_on_empty;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetNameLT ( const le_track_t *lt, ccp return_on_empty )
{
    return lt && lt->track_status >= LTS_EXPORT && IsOutputNameValidLEO(lt->name,0)
	? lt->name
	: return_on_empty;
}

//-----------------------------------------------------------------------------

ccp GetXNameLT ( const le_track_t *lt, ccp return_on_empty )
{
    return lt && lt->track_status >= LTS_EXPORT && IsOutputNameValidLEO(lt->xname,0)
	? lt->xname
	: return_on_empty;
}

//-----------------------------------------------------------------------------

ccp GetXName2LT ( const le_track_t *lt, ccp return_on_empty )
{
    if ( !lt || lt->track_status < LTS_EXPORT )
	return return_on_empty;

    return IsOutputNameValidLEO(lt->xname,0)
	? lt->xname
	: IsOutputNameValidLEO(lt->name,0)
	? lt->name
	: return_on_empty;
}

///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_LIST_ENABLED

 ccp GetListLT ( const le_track_t *lt, le_options_t opt, ccp return_on_empty )
 {
    const le_track_text_t ltt = opt & LEO_LTT_SELECTOR;

    if	(  lt
	&& lt->track_status >= LTS_EXPORT
	&& ltt >= LTT__LIST_BEG
	&& ltt <  LTT__LIST_END
	)
    {
	ccp text = lt->str_list[ ltt - LTT__LIST_BEG ];
	if ( text && *text )
	    return text;
    }

    return return_on_empty;
 }

#endif // LE_STRING_LIST_ENABLED
///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_SET_ENABLED

 ccp GetSetLT ( const le_track_t *lt, const le_strpar_t *par, ccp return_on_empty )
 {
    if	(  lt
	&& lt->track_status >= LTS_EXPORT
	&& IsValidSetLSP(par)
	)
    {
	const ParamFieldItem_t *it = FindParamField(&lt->str_set,par->key);
	if (it)
	    return it->data;
    }
    return return_on_empty;
 }

#endif // LE_STRING_SET_ENABLED
///////////////////////////////////////////////////////////////////////////////

ccp GetTextLT ( const le_track_t *lt, const le_strpar_t *par, ccp return_on_empty )
{
    if (!par)
	return return_on_empty;

 #if LE_STRING_SET_ENABLED
    if (IsValidSetLSP(par))
	return GetSetLT(lt,par,return_on_empty);
 #endif
    return GetTextOptLT(lt,par->opt,return_on_empty);
}

//-----------------------------------------------------------------------------

ccp GetTextOptLT ( const le_track_t *lt, le_options_t opt, ccp return_on_empty )
{
    const le_track_text_t ltt = opt & LEO_LTT_SELECTOR;
    switch (ltt)
    {
	case LTT_SHA1:    return GetSha1LT(lt,false,return_on_empty);
	case LTT_SHA1_D:  return GetSha1LT(lt,true, return_on_empty);
	case LTT_IDENT:   return GetIdentLT(lt,false,false,return_on_empty);
	case LTT_IDENT_D: return GetIdentLT(lt,true, false,return_on_empty);

	case LTT_FILE:    return GetFileLT(lt,return_on_empty);
	case LTT_NAME:    return GetNameLT(lt,return_on_empty);
	case LTT_XNAME:   return GetXNameLT(lt,return_on_empty);
	case LTT_XNAME2:  return GetXName2LT(lt,return_on_empty);

	default:
	 #if LE_STRING_LIST_ENABLED
	    if ( ltt >= LTT__LIST_BEG && ltt < LTT__LIST_END )
		return GetListLT(lt,ltt,return_on_empty);
	 #endif
	    break;
    }

    return return_on_empty;
}

///////////////////////////////////////////////////////////////////////////////

ccp * GetPtrLT ( const le_track_t *lt, const le_strpar_t *par )
{
 #if LE_STRING_SET_ENABLED
    if (IsValidSetLSP(par))
    {
	ParamFieldItem_t *it = FindParamField(&lt->str_set,par->key);
	return it ? (ccp*)&it->data : 0;
    }
 #endif
    return par ? GetPtrOptLT(lt,par->opt,par->allow_d) : 0;
}

//-----------------------------------------------------------------------------

ccp * GetPtrOptLT ( const le_track_t *lt, le_options_t opt, bool allow_d )
{
    const le_track_text_t ltt = opt & LEO_LTT_SELECTOR;
    switch (ltt)
    {
	case LTT_IDENT_D:
	    if (allow_d)
		return (ccp*)&lt->lti[1].ident;
	    break;

	case LTT_IDENT:   return (ccp*)&lt->lti[0].ident;
	case LTT_FILE:    return (ccp*)&lt->file;
	case LTT_NAME:    return (ccp*)&lt->name;
	case LTT_XNAME:   return (ccp*)&lt->xname;

	default:
	 #if LE_STRING_LIST_ENABLED
	    if ( ltt >= LTT__LIST_BEG && ltt < LTT__LIST_END )
		return (ccp*)lt->str_list + ltt - LTT__LIST_BEG;
	 #endif
	    break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetSlotNameLT ( const le_track_t *lt )
{
    if (!lt)
	return 0;

    static const char rslot_name[][6] = { "rAll", "rOrig", "rCust", "rNew" };

    ccp pre = GetNameLTTY(lt->track_type);
    return lt->track_slot >= MKW_LE_RANDOM_BEG && lt->track_slot <= MKW_LE_RANDOM_END
		? rslot_name[ lt->track_slot - MKW_LE_RANDOM_BEG ]
		: PrintCircBuf( "%s%u",  pre, lt->track_slot );
}

///////////////////////////////////////////////////////////////////////////////

ccp GetCupLT ( const le_track_t *lt, ccp return_on_empty )
{
    if (!lt)
	return return_on_empty;

    const bool hidden = IsHiddenLETF(lt->flags);
    if (!lt->cup_slot)
	return hidden ? "H" : "-";

    return PrintCircBuf("%s%s%u.%u",
		hidden ? "H" : "",
		IsArenaLTTY(lt->track_type) ? "A" : IsRandomLTTY(lt->track_type) ? "R" : "",
		lt->cup_slot / 10, lt->cup_slot % 10 );
}

//-----------------------------------------------------------------------------

ccp GetCupAlignLT ( const le_track_t *lt )
{
    ccp res = GetCupLT(lt,"-");
    return IsHiddenLETF(lt->flags)
	? PrintCircBuf("%8s",res)
	: PrintCircBuf("%6s  ",res);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Quote*LT() functions		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp QuoteSha1LT ( const le_track_t *lt, bool use_d, ccp return_on_empty )
{
    ccp res = GetSha1LT(lt,use_d,return_on_empty);
    return QuoteStringCircS(res,"\"\"",CHMD__MODERN);
}

///////////////////////////////////////////////////////////////////////////////

ccp QuoteIdentLT ( const le_track_t *lt, bool use_d, bool use_sha1, ccp return_on_empty )
{
    ccp res = GetIdentLT(lt,use_d,use_sha1,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
}

///////////////////////////////////////////////////////////////////////////////

ccp QuoteFileLT ( const le_track_t *lt, ccp return_on_empty )
{
    ccp res = GetFileLT(lt,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
}

///////////////////////////////////////////////////////////////////////////////

ccp QuoteNameLT ( const le_track_t *lt, ccp return_on_empty )
{
    ccp res = GetNameLT(lt,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
}

///////////////////////////////////////////////////////////////////////////////

ccp QuoteXNameLT ( const le_track_t *lt, ccp return_on_empty )
{
    ccp res = GetXNameLT(lt,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
}

///////////////////////////////////////////////////////////////////////////////

ccp QuoteXName2LT ( const le_track_t *lt, ccp return_on_empty )
{
    ccp res = GetXName2LT(lt,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
}

///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_LIST_ENABLED

 ccp QuoteListLT ( const le_track_t *lt, le_options_t opt, ccp return_on_empty )
 {
    ccp res = GetListLT(lt,opt,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
 }

#endif // LE_STRING_LIST_ENABLED
///////////////////////////////////////////////////////////////////////////////
#if LE_STRING_SET_ENABLED

 ccp QuoteSetLT ( const le_track_t *lt, const le_strpar_t *par, ccp return_on_empty )
 {
    ccp res = GetSetLT(lt,par,return_on_empty);
    return QuoteStringCircS(res,EmptyQuote,CHMD__MODERN);
 }

#endif // LE_STRING_SET_ENABLED
///////////////////////////////////////////////////////////////////////////////

ccp QuoteTextLT ( const le_track_t *lt, const le_strpar_t *par, ccp return_on_empty )
{
    if (!par)
	return return_on_empty;

 #if LE_STRING_SET_ENABLED
    if (IsValidSetLSP(par))
	return QuoteSetLT(lt,par,return_on_empty);
 #endif
    return QuoteTextOptLT(lt,par->opt,return_on_empty);
}
//-----------------------------------------------------------------------------

ccp QuoteTextOptLT ( const le_track_t *lt, le_options_t opt, ccp return_on_empty )
{
    const le_track_text_t ltt = opt & LEO_LTT_SELECTOR;
    switch (ltt)
    {
	case LTT_SHA1:    return QuoteSha1LT(lt,false,return_on_empty); break;
	case LTT_SHA1_D:  return QuoteSha1LT(lt,true, return_on_empty); break;
	case LTT_IDENT:   return QuoteIdentLT(lt,false,false,return_on_empty); break;
	case LTT_IDENT_D: return QuoteIdentLT(lt,true, false,return_on_empty); break;

	case LTT_NAME:    return QuoteNameLT(lt,return_on_empty); break;
	case LTT_XNAME:   return QuoteXNameLT(lt,return_on_empty); break;
	case LTT_XNAME2:  return QuoteXName2LT(lt,return_on_empty); break;
	case LTT_FILE:    return QuoteFileLT(lt,return_on_empty); break;

	default:
	 #if LE_STRING_LIST_ENABLED
	    if ( ltt >= LTT__LIST_BEG && ltt < LTT__LIST_END )
		return QuoteListLT(lt,ltt,return_on_empty);
	 #endif
	    break;
    }

    return return_on_empty;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: management		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLD ( le_distrib_t *ld )
{
    DASSERT(ld);
    memset(ld,0,sizeof(*ld));

    ld->is_initialized		= true;
    ld->scan.bt.add_unused	= true;
    ld->scan.bt.add_unused	= true;
    ld->auto_setup		= LAS_DEFAULT;

    InitializeLSP(&ld->spar,SetupDefaultLEO());
    InitializeLECUP(&ld->cup_battle,BMG_MAX_BCUP,BMG_BCUP_TRACKS);
    InitializeLECUP(&ld->cup_versus,BMG_MAX_RCUP,BMG_RCUP_TRACKS);

    InitializeParamField(&ld->dis_param);
    ld->dis_param.free_data = true;

    InitializeEML(&ld->group);
}

///////////////////////////////////////////////////////////////////////////////

void ResetLD ( le_distrib_t *ld )
{
    if (ld)
    {
	ResetArchLD(ld);
	ResetTracksAndCupsLD(ld);
	ResetParamField(&ld->dis_param);
	FREE(ld->lpar);
	ResetLSP(&ld->spar);

	if (ld->ctcode)
	{
	    ResetCTCODE(ld->ctcode);
	    FREE(ld->ctcode);
	}

	for ( int reg = 0; reg < LEREG__N; reg++ )
	    if (ld->lecode_alloced[reg])
		FREE((u8*)ld->lecode[reg].ptr);

	InitializeLD(ld);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetTracksAndCupsLD ( le_distrib_t *ld )
{
    if (ld)
    {
	le_track_t *lt  = ld->tlist;
	le_track_t *end = lt + ld->tlist_used;
	for ( ; lt < end; lt++ )
	    ResetLT(lt);

	FREE(ld->tlist);
	ld->tlist = 0;
	ld->tlist_used = ld->tlist_size = 0;

	FREE(ld->reflist);
	ld->reflist = 0;
	ld->reflist_used = ld->reflist_size = 0;

	ResetLECUP(&ld->cup_battle);
	ResetLECUP(&ld->cup_versus);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AutoSetupLD ( le_distrib_t *ld, le_auto_setup_t auto_setup )
{
    if (!ld)
	return;

    ld->auto_setup = auto_setup;
    for ( int slot = MKW_TRACK_BEG; slot < MKW_ARENA_END; slot++ )
	DefineTrackLD(ld,slot,true);
    for ( int slot = MKW_LE_RANDOM_BEG; slot < MKW_LE_RANDOM_END; slot++ )
	DefineTrackLD(ld,slot,true);

    if ( auto_setup & LAS_ARENA )
    {
	DefineLECUP(&ld->cup_battle,1);
	memcpy(ld->cup_battle.list,standard_bt_ref,sizeof(standard_bt_ref));
    }

    if ( auto_setup & LAS_TRACK )
    {
	DefineLECUP(&ld->cup_versus,7);
	memcpy(ld->cup_versus.list,standard_vs_ref,sizeof(standard_vs_ref));
    }

    if ( auto_setup & LAS_TEMPLATE )
    {
	le_cup_ref_t *ref = DefineLECUP(&ld->cup_versus,8);
	if (ref)
	    memcpy(ref,mkwfun_random_ref,sizeof(mkwfun_random_ref));

	ref = DefineLECUP(&ld->cup_versus,9);
	if (ref)
	{
	    for ( int i = 1; i <= 6; i++ )
	    {
		le_track_t *lt = DefineFreeTrackLD(ld,LTTY_TRACK,true);
		if (lt)
		{
		    lt->property = 10+i;
		    lt->music = NormalizeMusicID(20+i);

		    char buf[30];
		    snprintf(buf,sizeof(buf),"file_%03u",lt->track_slot);
		    SetFileLT(lt,0,buf);

		    if ( i == 3 )
		    {
			lt->flags = LETF_RND_HEAD;
			SetNameLT(lt,0,"Head of a group");
			*ref++ = lt->track_slot;
		    }
		    else if ( i == 4 || i == 5 )
		    {
			lt->flags = LETF_RND_GROUP;
			snprintf(buf,sizeof(buf),"Group Track #%u (hidden)",i-3);
			SetNameLT(lt,0,buf);
			SetIdentOptLT(lt,"SHA1 of SZS",false,false,0);
		    }
		    else
		    {
			snprintf(buf,sizeof(buf),"Test Track #%u",i);
			SetNameLT(lt,0,buf);
			SetIdentOptLT(lt,"SHA1 of SZS",false,false,0);
			*ref++ = lt->track_slot;
		    }

		    snprintf(buf,sizeof(buf),"Extended Name #%u",i);
		    SetXNameLT(lt,0,buf);
		}
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void AnalyzeLD ( le_distrib_t *ld, bool force_new_analysis )
{
    if ( !ld || ld->ana.valid && !force_new_analysis )
	return;

    memset(&ld->ana,0,sizeof(ld->ana));
    ld->ana.valid		= true;
    ld->ana.bt.orig_tracks	= true;
    ld->ana.vs.orig_tracks	= true;

    ld->ana.bt.add_unused	= !ld->scan.valid || ld->scan.bt.add_unused;
    ld->ana.vs.add_unused	= !ld->scan.valid || ld->scan.vs.add_unused;

    for ( int slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
    {
	le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT || !lt->is_original )
	{
	    ld->ana.bt.orig_tracks = false;
	    break;
	}
    }

    for ( int slot = MKW_TRACK_BEG; slot < MKW_TRACK_END; slot++ )
    {
	le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT || !lt->is_original )
	{
	    ld->ana.vs.orig_tracks = false;
	    break;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CutLD ( le_distrib_t *ld, le_options_t opt )
{
    if (!ld)
	return;

    if ( opt & LEO_CUT_STD )
	ClearTracksLD( ld, BMG_N_TRACK + BMG_N_ARENA, INT_MAX );

    if ( opt & LEO_CUT_CTCODE )
    {
	ClearTracksLD( ld, CT_CODE_MAX_TRACKS, INT_MAX );
	ClearTracksLD( ld, BMG_N_TRACK, BMG_N_TRACK + BMG_N_ARENA );
    }
}

//-----------------------------------------------------------------------------

void ClearTracksLD ( le_distrib_t *ld, int from, int to )
{
    if (!ld)
	return;

    if ( from < 0 )
	from = 0;
    if ( to > ld->tlist_used )
	 to = ld->tlist_used;

    le_track_t *lt  = ld->tlist + from;
    le_track_t *end = ld->tlist + to;

    for ( ; lt < end; lt++ )
    {
	FreeString(lt->lti[0].ident);
	FreeString(lt->lti[1].ident);
	FreeString(lt->name);
	FreeString(lt->file);
	memset(lt,0,sizeof(*lt));
    }

    if ( ld->tlist_used == to )
	ld->tlist_used = from;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint PackTracksLD ( const le_distrib_t *ld )
{
    if (!ld)
	return 0;

    if ( ld->tlist_used < LE_FIRST_CT_SLOT )
	return LE_FIRST_CT_SLOT;

    int slot;
    for ( slot = ld->tlist_used - 1; slot >= LE_FIRST_CT_SLOT; slot-- )
    {
	le_track_t *lt = ld->tlist + slot;
	if ( lt->track_status >= LTS_ACTIVE )
	    break;
	ClearLT(lt,0);
    }
    return ((le_distrib_t*)ld)->tlist_used = slot+1;
}

///////////////////////////////////////////////////////////////////////////////

void CheckTracksLD ( const le_distrib_t *ld )
{
    if (!ld)
	return;

    le_track_t *lt = ld->tlist;
    for ( int slot = 0; slot < ld->tlist_used; slot++, lt++ )
    {
	if ( lt->track_status < LTS_ACTIVE )
	    continue;

	if ( lt->track_type & LTTY_TRACK )
	{
	    if (!IsMkwTrack(lt->property))
		lt->track_status = LTS_FAIL;
	}
	else
	{
	    if (!IsMkwArena(lt->property))
		lt->track_status = LTS_FAIL;
	}
    }

    PackTracksLD(ld);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

le_track_t * GetTrackLD ( le_distrib_t *ld, int slot )
{
    if ( !ld || !IsValidLecodeSlot(slot) || slot >= ld->tlist_used )
	return 0;

    le_track_t *lt = ld->tlist + slot;
    return lt->track_status < LTS_ACTIVE ? 0 : lt;
}

///////////////////////////////////////////////////////////////////////////////

le_track_t * DefineTrackLD ( le_distrib_t *ld, int slot, bool mark_export )
{
    if ( !ld || !IsValidLecodeSlot(slot) )
	return 0;

    if (!ld->is_initialized)
	InitializeLD(ld);

    uint need = slot + 1;
    if ( need > ld->tlist_size )
    {
	static const int tab[] = { 512, 1024, 2048, LE_MAX_TRACKS, 0 };
	for ( const int *ptr = tab; *ptr; ptr++ )
	    if ( need <= *ptr )
	    {
		need = *ptr;
		break;
	    }

	ld->tlist = REALLOC( ld->tlist, need * sizeof(*ld->tlist) );
	memset( ld->tlist + ld->tlist_size, 0, ( need - ld->tlist_size) * sizeof(*ld->tlist) );
	ld->tlist_size = need;

	FREE(ld->reflist);
	ld->reflist = 0;
	ld->reflist_used = ld->reflist_size = 0;
    }

    DASSERT( slot >= 0 && slot <= MKW_MAX_TRACK_SLOT );
    DASSERT( slot < ld->tlist_size );

    if ( ld->tlist_used <= slot )
	ld->tlist_used = slot + 1;
    if ( ld->tlist_used < LE_FIRST_CT_SLOT )
	 ld->tlist_used = LE_FIRST_CT_SLOT;
    ASSERT( ld->tlist_used <= ld->tlist_size );

    le_track_t *lt = ld->tlist + slot;
    if ( lt->track_status == LTS_INVALID )
    {
	// initialize

	InitializeLT(lt);
	lt->track_slot = slot;

	if ( IsMkwArena(slot) )
	{
	    lt->track_type	= LTTY_ARENA;
	    if ( ld->auto_setup & LAS_ARENA )
		SetupStandardArenaLT(lt,slot);
	}
	else if ( IsLecodeRandom(slot) )
	{
	    lt->track_status	= LTS_ACTIVE;
	    lt->track_type	= LTTY_RANDOM|LTTY_TRACK;
	    if ( ld->auto_setup & LAS_RANDOM )
		SetupLecodeRandomTrackLT(lt,slot);
	}
	else
	{
	    lt->track_type	= LTTY_TRACK;
	    if ( IsMkwTrack(slot) && ld->auto_setup & LAS_TRACK )
		SetupStandardTrackLT(lt,slot);
	}

	DirtyCupLD(ld,lt->track_type);
    }

    if ( mark_export && lt->track_status < LTS_ACTIVE )
	lt->track_status = LTS_EXPORT;

    return lt;
}

///////////////////////////////////////////////////////////////////////////////

le_track_t * DefineFreeTrackLD
	( le_distrib_t *ld, le_track_type_t ltty, bool mark_export )
{
    if (!ld)
	return 0;

    if (!ltty)
	ltty = LTTY_TRACK | LTTY_ARENA;
    const int slot_beg
		= ltty == LTTY_TRACK
		? MKW_TRACK_BEG
		: ltty == LTTY_ARENA
		? MKW_ARENA_BEG
		: ltty & (LTTY_TRACK|LTTY_ARENA)
		? LE_FIRST_CT_SLOT
		: -1;

    if ( slot_beg < 0 )
	return 0;

    for ( int slot = slot_beg; slot <= LE_LAST_CT_SLOT; slot++ )
    {
	if ( slot == MKW_TRACK_END && !(ltty&LTTY_ARENA) || slot == MKW_ARENA_END )
	    slot = LE_FIRST_CT_SLOT;

	le_track_t *lt = DefineTrackLD(ld,slot,false);
	if ( lt && lt->track_status < LTS_ACTIVE )
	{
	    if (mark_export)
		lt->track_status = LTS_EXPORT;
	    lt->track_type = ltty;
	    DirtyCupLD(ld,ltty);
	    return lt;
	}
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

le_track_t * DefineGroupTrackLD
	( le_distrib_t *ld, le_track_type_t ltty, bool mark_export )
{
    if (!ld)
	return 0;

    if (!ltty)
	ltty = LTTY_TRACK | LTTY_ARENA;
    DirtyCupLD(ld,ltty);

    int slot = PackTracksLD(ld);
    ASSERT ( slot >= LE_FIRST_CT_SLOT );
    return DefineTrackLD(ld,slot,mark_export);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FindFreeTracksLD ( le_distrib_t *ld, le_track_type_t ltty, int n )
{
    int beg, end;
    if ( ltty == LTTY_TRACK )
    {
	beg = MKW_TRACK_BEG;
	end = MKW_TRACK_END;
    }
    else if ( ltty == LTTY_ARENA )
    {
	beg = MKW_ARENA_BEG;
	end = MKW_ARENA_END;
    }
    else
    {
	beg = LE_FIRST_CT_SLOT;
	end = 0;
    }

    int prev = -10, start = -10;
    for ( int slot = beg; slot < LE_LAST_CT_SLOT; slot++ )
    {
	if ( slot == end )
	    slot = LE_FIRST_CT_SLOT;

	if ( slot < ld->tlist_used && IsActiveLT(ld->tlist+slot) )
	    continue;

	if ( prev+1 != slot )
	    start = slot;
	prev = slot;
	if ( start + n == slot )
	    return start;
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

le_track_t * ReserveTracksLD
	( le_distrib_t *ld, le_track_type_t ltty, int n, bool mark_export )
{
    const int first = FindFreeTracksLD(ld,ltty,n);
    if ( first < 0 )
	return 0;
    for ( int slot = first; slot < first + n; slot++ )
    {
	le_track_t *lt = DefineTrackLD(ld,slot,mark_export);
	lt->track_type = ltty;
	lt->track_status = mark_export ? LTS_EXPORT : LTS_ACTIVE;
    }
    return ld->tlist + first;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearUserFlagsAndCupsLD ( le_distrib_t *ld )
{
    if (ld)
    {	
	le_track_t *lt = ld->tlist;
	for ( int slot = 0; slot < ld->tlist_used; slot++, lt++ )
	{
	    lt->user_flags = 0;
	    lt->cup_slot = 0;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

bool HaveWiimmCupLD ( const le_distrib_t *ld )
{
    if (!ld)
	return false;

    le_cup_ref_t *ref = GetLECUP(&ld->cup_versus,8);
    return ref && ref[0] == 62 && ref[1] == 63 && ref[2] == 64 && ref[3] == 65;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: scan text files		///////////////
///////////////////////////////////////////////////////////////////////////////

static ccp xstring ( char *buf, int bufsize, mem_t src )
{
    if (!src.len)
	return 0;

    StringCopySM(buf,bufsize,src.ptr,src.len);
    ScanEscapedStringPipe(buf,bufsize,buf,0);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanDumpLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    ScanText_t st;
    SetupScanText(&st,data,data_size);
    ScanDumpSTLD(ld,init_ld,&st);
    ResetScanText(&st);
}

//-----------------------------------------------------------------------------

void ScanDumpSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
)
{
    if ( !ld || !st )
	return;

    st->detect_sections++;

    for(;;)
    {
	if (!st->is_section)
	    NextLineScanText(st);
	if (st->is_eot)
	    break;

	if (!st->is_section)
	    continue;

	ccp sect = st->line + 1;
	int slen = st->eol - 1 - sect;
	PRINT0("SECTION: %.*s\n",slen,sect);

	if (!strncasecmp(sect,"TOOL",4))
	{
	    PRINT0(">> TOOL: ignore\n");
	    st->is_section = false;
	}
	else if (!strncasecmp(sect,"SETTINGS",8))
	{
	    PRINT0(">> SETTINGS: 2do\n");
	    // [[2do]] ???
	    st->is_section = false;
	}
	else if (!strncasecmp(sect,"DISTRIBUTION-PARAM",18))
	{
	    ImportDistribSTLD(ld,st);
	}
	else if (  !strncasecmp(sect,"LECODE-PARAMETERS",17)
		|| !strncasecmp(sect,"CHAT-MESSAGE-MODES",18)
		|| !strncasecmp(sect,"DEBUG-DOCU",10)
		|| !strncasecmp(sect,"DEBUG-1",7)
		|| !strncasecmp(sect,"DEBUG-2",7)
		|| !strncasecmp(sect,"DEBUG-3",7)
		|| !strncasecmp(sect,"DEBUG-4",7)
		)
	{
	    SetupLparLD(ld,false);
	    ccp start = st->line;
	    while ( NextLineScanText(st) && !st->is_section )
		;
	    ScanTextLPAR(ld->lpar,false,0,start,st->line-start);
	}
	else if (!strncasecmp(sect,"CUP-REF",7))
	{
	    PRINT0(">> CUP-REF: 2do\n");
	    ScanRefSTLD(ld,false,st);
	}
	else if (!strncasecmp(sect,"TRACK-REF",9))
	{
	    PRINT0(">> TRACK-REF: 2do\n");
	    ScanRefSTLD(ld,false,st);
	}
	else if (!strncasecmp(sect,"TRACK-STRINGS",13))
	{
	    PRINT0(">> TRACK-STRINGS: 2do\n");
	    ScanStrSTLD(ld,false,st);
	}
	else if (  !strncasecmp(sect,"END",3)
		|| !strncasecmp(sect,"LECODE-INFO",6) )
	{
	    // simply ignore
	    st->is_section = false;
	}
	else
	{
	    ERROR0(ERR_WARNING,"Unknown section will be ignored: %.*s\n",slen,sect);
	    st->is_section = false;
	}
    }

    st->detect_sections--;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanRefLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    ScanText_t st;
    SetupScanText(&st,data,data_size);
    ScanRefSTLD(ld,init_ld,&st);
    ResetScanText(&st);
}

///////////////////////////////////////////////////////////////////////////////

void ScanRefSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
)
{
    if ( !ld || !st )
	return;

    if ( init_ld || !ld->is_initialized )
	InitializeLD(ld);

    enum
    {
	IDX_TSLOT,	// track_slot
	IDX_TYPE,	// 'vs' | 'bt'
	IDX_ORIGINAL,
	IDX_CUP_SLOT,	// cup slot
	IDX_CUP_NAME,	// cup name
	IDX_PROPERTY,
	IDX_MUSIC,
	IDX_FLAGS,
	IDX_HIDDEN,
	IDX_SHA1,
	IDX_SHA1_D,
	IDX_IDENT,
	IDX_IDENT_D,
	IDX_FILE,
	IDX_FILE_D,
	IDX_NAME,
	IDX_NAME_D,
	IDX_XNAME,
	IDX_XNAME_D,
	IDX__N
    };

    while (NextLineScanText(st))
    {
	mem_t srcline = { .ptr = st->line, .len = st->eol - st->line };
	mem_t src[IDX__N+2];
	const uint n = SplitByCharMem(src,IDX__N+1,srcline,'|');
	if ( n < IDX_FLAGS )
	    continue;

	const int tslot = str2l(src[IDX_TSLOT].ptr,0,10);
	le_track_t *lt = DefineTrackLD(ld,tslot,true);
	if (!lt)
	    continue;

	PRINT0("LINE[%d]: %.*s|\n",tslot,(int)(ptr-line),line);

	lt->cup_slot	= str2l(src[IDX_CUP_SLOT].ptr,0,10);
	lt->property	= str2l(src[IDX_PROPERTY].ptr,0,10);
	lt->music	= str2l(src[IDX_MUSIC].ptr,0,10);
	lt->flags	= str2l(src[IDX_FLAGS].ptr,0,10);

	char buf[LE_TRACK_STRING_MAX+1];
	if ( n > IDX_SHA1  )	SetIdentOptLT(lt,xstring(buf,sizeof(buf),src[IDX_SHA1]),   0,1,ld->spar.opt );
	if ( n > IDX_IDENT )	SetIdentOptLT(lt,xstring(buf,sizeof(buf),src[IDX_IDENT]),  0,0,ld->spar.opt );
	if ( n > IDX_SHA1_D  )	SetIdentOptLT(lt,xstring(buf,sizeof(buf),src[IDX_SHA1_D]), 1,1,ld->spar.opt );
	if ( n > IDX_IDENT_D )	SetIdentOptLT(lt,xstring(buf,sizeof(buf),src[IDX_IDENT_D]),1,0,ld->spar.opt );
	if ( n > IDX_FILE  )	SetFileLT (lt,&ld->spar,xstring(buf,sizeof(buf),src[IDX_FILE] ) );
	if ( n > IDX_NAME  )	SetNameLT (lt,&ld->spar,xstring(buf,sizeof(buf),src[IDX_NAME] ) );
	if ( n > IDX_XNAME )	SetXNameLT(lt,&ld->spar,xstring(buf,sizeof(buf),src[IDX_XNAME]) );

 #if 0
	// this is the last action to overwrite previous changes
	if (src[IDX_ORIGINAL].len)
	    lt->is_original = str2l(src[IDX_ORIGINAL].ptr,0,10);
 #endif
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanStrLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    ScanText_t st;
    SetupScanText(&st,data,data_size);
    ScanStrSTLD(ld,init_ld,&st);
    ResetScanText(&st);
}

///////////////////////////////////////////////////////////////////////////////

void ScanStrSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
)
{
    if ( !ld || !st )
	return;

    if ( init_ld || !ld->is_initialized )
	InitializeLD(ld);

    enum
    {
	IDX_TSLOT,	// track_slot
	IDX_D_MODE,	// bit fields: 1: set std, 2: set _d, 3: set both
	IDX_IS_SET,
	IDX_NAME,
	IDX_STRING,
	IDX__N
    };

    while (NextLineScanText(st))
    {
	mem_t srcline = { .ptr = st->line, .len = st->eol - st->line };
	mem_t src[IDX__N+2];
	const uint n = SplitByCharMem(src,IDX__N+1,srcline,'|');
	if ( n < IDX__N )
	    continue;

	const int tslot = str2l(src[IDX_TSLOT].ptr,0,10);
	le_track_t *lt = DefineTrackLD(ld,tslot,true);
	if (!lt)
	    continue;

	le_strpar_t spar = { .opt = LEO__DEFAULT|LEO_IN_ALL };
	if ( str2l(src[IDX_IS_SET].ptr,0,10) > 0 )
     #if LE_STRING_SET_ENABLED
	    SetKeyNameLSP(&spar,src[IDX_NAME].ptr,src[IDX_NAME].len);
     #else
	    continue;
     #endif
	else
	    ScanLSP(&spar,src[IDX_NAME].ptr,0);

	char buf[LE_TRACK_STRING_MAX+1];
	StringCopySM(buf,sizeof(buf),src[IDX_STRING].ptr,src[IDX_STRING].len);
	ScanEscapedStringPipe(buf,sizeof(buf),buf,0);

	PRINT1(">>>> %s = %s\n",GetNameLSP(&spar),buf);
	const int dmode = str2l(src[IDX_D_MODE].ptr,0,10);
	if ( dmode == 2 )
	    spar.use_d = true;
	SetTextLT(lt,&spar,buf);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanSha1LD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    ScanText_t st;
    SetupScanText(&st,data,data_size);
    ScanSha1STLD(ld,init_ld,&st);
    ResetScanText(&st);
}

///////////////////////////////////////////////////////////////////////////////

void ScanSha1STLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
)
{
    if ( !ld || !st )
	return;

    if ( init_ld || !ld->is_initialized )
	InitializeLD(ld);

    while (NextLineScanText(st))
	ScanSha1LineLD(ld,st->line,st->eol,true);
}

///////////////////////////////////////////////////////////////////////////////

bool ScanSha1LineLD
(
    le_distrib_t	*ld,		// valid structure
    ccp			src,		// NULL or start of line
    ccp			end,		// end of line
    bool		ident_only	// TRUE: SetIdent*LT() only
)
{
    if ( !ld || !src || !end )
	return false;

    bool is_arena	= false;
    bool is_d		= false;
    uint tslot		= -1;
    uint cup		= 0;

    int pi;
    char buf[300];
    int valid_sha1 = false;
    sha1_size_t bin;
    enum { P_TYPE, P_FLAGS, P_SHA1, P_TSLOT, P_CUP, P_NAME };

    for ( pi = P_TYPE; pi <= P_NAME; pi++ )
    {
	while ( src < end && (uchar)*src <= ' ' )
	    src++;

	ccp par = src;
	while ( src < end && (uchar)*src > ' ' )
	    src++;

	switch (pi)
	{
	 case P_TYPE:
	    PRINT0("TYPE:  %.*s\n",(int)(src-par),par);
	    if ( src - par != 2 )
		return false;
	    if (!strncasecmp(par,"bt",2))
		is_arena = true;
	    else if (strncasecmp(par,"vs",2))
		return false;
	    break;

	 case P_FLAGS:
	    PRINT0("FLAGS: %.*s\n",(int)(src-par),par);
	    StringCopySM(buf,sizeof(buf),par,src-par);
	    is_d = strchr(buf,'d') != 0;
	    break;

	 case P_SHA1:
	    PRINT0("SHA1:  %.*s\n",(int)(src-par),par);
	    valid_sha1 = IsSSChecksum(&bin,par,src-par);
	    break;

	 case P_TSLOT:
	    PRINT0("TSLOT: %.*s\n",(int)(src-par),par);
	    {
		char *scan_end;
		tslot = str2ul(par,&scan_end,10);
		if ( tslot > MKW_MAX_TRACK_SLOT || scan_end != src )
		    return false;
	    }
	    break;

	 case P_CUP:
	    PRINT0("CUP:   %.*s\n",(int)(src-par),par);
	    {
		if ( *par == 'a' || *par == 'A' )
		    par++;
		if ( par < src )
		{
		    ccp pt = strchr(par,'.');
		    if ( pt && pt < src )
			cup = str2ul(par,0,10) * 10 + str2ul(pt+1,0,10);
		}
	    }
	    break;

	 case P_NAME:
	    src = end-1;
	    while ( src >= par && (uchar)*src <= ' ' )
		src--;
	    src++;
	    PRINT0("XNAME: %.*s\n",(int)(src-par),par);
	    StringCopySM(buf,sizeof(buf),par,src-par);
	    break;
	}
    }

    if (!valid_sha1)
	return false;

    sha1_hex_t hex;
    Sha1Bin2Hex(hex,bin.hash);
    PRINT0(">>> arena=%d, d=%d, tslot=%4d, cup=%2d : %s : |%s|\n",
		is_arena, is_d, tslot, cup, hex, buf );

    le_track_t *lt = DefineTrackLD(ld,tslot,true);
    if (!ident_only)
    {
	lt->track_type = is_arena ? LTTY_ARENA : LTTY_TRACK;
	lt->cup_slot = cup;
	SetXNameLT(lt,0,buf);
    }
    SetIdentOptLT(lt,hex,is_d,true,0);
    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: scan ledef		///////////////
///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t * scan_cmd
(
    ScanInfo_t		*si,		// valid structure
    const KeywordTab_t	*commands,	// command table
    ccp			section,	// name of section
    enumError		*max_err	// NULL or max_err variable
)
{
    DASSERT(si);
    DASSERT(commands);

    char buf[100];
    if (!ScanNameSI(si,buf,sizeof(buf),true,true,0))
    {
	if (!si->no_warn)
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ScanFile_t *sf = si->cur_file;
	    DASSERT(sf);
	    ERROR0(ERR_WARNING,
		    "File %s, section [%s], line #%u:\nMissing command: %.*s\n",
		    sf->name, section, sf->line,
		    (int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	if ( max_err && *max_err < ERR_WARNING )
	    *max_err = ERR_WARNING;
	GotoEolSI(si);
	return 0;
    }

    int abbrev_count;
    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,buf,commands);
    if (!cmd)
    {
	if (!si->no_warn)
	{
	    ScanFile_t *sf = si->cur_file;
	    ccp msg = GetKeywordError(commands,buf,abbrev_count,0);
	    ERROR0(ERR_WARNING,
		    "File %s, section [%s], line #%u:\n%s\n",
		    sf->name, section, sf->line, msg );
	}
	if ( max_err && *max_err < ERR_WARNING )
	    *max_err = ERR_WARNING;
	GotoEolSI(si);
	return 0;
    }

    return cmd;
}

///////////////////////////////////////////////////////////////////////////////

static enumError scan_track ( le_track_t *lt, ScanInfo_t *si )
{
    if ( !lt || !si || !si->ld )
	return ERR_MISSING_PARAM;
    le_distrib_t *ld = si->ld;

    memset(lt,0,sizeof(*lt));
    lt->track_status = LTS_EXPORT;

    enumError err;
    ScanIndexSI(si,&err,0,0,ld->is_pass2?0:2);
    if (err)
	return err;

    uint uval;
    err = ScanUIntValueSI(si,&uval);
    if (err)
	return err;
    lt->track_type = uval;

    err = ScanUIntValueSI(si,&uval);
    if (err)
	return err;
    lt->property = uval;

    err = ScanUIntValueSI(si,&uval);
    if (err)
	return err;
    lt->music = NormalizeMusicID(uval);

    DEFINE_VAR(var);
    err = ScanValueSI(si,&var);
    if (!err)
	lt->flags = var.mode == VAR_STRING ? ScanLEFT(var.str) : GetIntV(&var);
    ResetV(&var);

    PRINT0("TRACK %s: t=%d, p=%d, m=%d, f:%x\n",
		last_index_name, lt->track_type, lt->property, lt->music, lt->flags );

    if ( lt->track_type == LTTY_TRACK )
    {
	if (!IsMkwTrack(lt->property))
	{
	    if (!si->no_warn)
	    {
		ScanFile_t *sf = si->cur_file;
		ERROR0(ERR_WARNING,
		    "File %s, section [TRACK-LIST], line #%u:\n"
		    "Versus track with invalid property %d.\n",
		    sf->name, sf->line, lt->property );
	    }
	    return ERR_WARNING;
	}
    }
    else if ( lt->track_type == LTTY_ARENA )
    {
	if (!IsMkwArena(lt->property))
	{
	    if (!si->no_warn)
	    {
		ScanFile_t *sf = si->cur_file;
		ERROR0(ERR_WARNING,
		    "File %s, section [TRACK-LIST], line #%u:\n"
		    "Battle arena with invalid property %d.\n",
		    sf->name, sf->line, lt->property );
	    }
	    return ERR_WARNING;
	}
    }
    else
    {
	if (!si->no_warn)
	{
	    ScanFile_t *sf = si->cur_file;
	    ERROR0(ERR_WARNING,
		"File %s, section [TRACK-LIST], line #%u:\n"
		"Invalid track type (not bt, not vs).\n",
		sf->name, sf->line );
	}
	return ERR_WARNING;
    }

    return err;
}

//-----------------------------------------------------------------------------

static enumError ScanLeDefTRACKS
(
    le_distrib_t	*ld,		// valid structure
    ScanInfo_t		*si,		// valid structure
    int			pass		// 1|2
)
{
    if ( !ld || !si )
	return ERR_MISSING_PARAM;

    enum { C_SLOT, C_TRACK, C_IDENT, C_FILE, C_NAME, C_XNAME,
		C_LIST, C_STD_ARENAS, C_STD_VERSUS };
    enum { O_STRING = 0x100 };

    static const KeywordTab_t commands[] =
    {
	{ C_SLOT,	"SLOT",			0,		0 },
	{ C_TRACK,	"TRACK",		0,		0 },

	{ C_IDENT,	"IDENT",		0,		O_STRING },
	{ C_FILE,	"FILE",			0,		O_STRING },
	{ C_NAME,	"NAME",			0,		O_STRING },
	{ C_XNAME,	"XNAME",		0,		O_STRING },

 #if LE_STRING_LIST_ENABLED
	{ C_LIST,	"TEMP1",		0,		O_STRING|LTT_TEMP1 },
	{ C_LIST,	"TEMP2",		0,		O_STRING|LTT_TEMP2 },
 #endif

	{ C_STD_ARENAS,	"STANDARD-BATTLE-ARENAS",
			"STANDARDBATTLEARENAS",			0 },
	{ C_STD_VERSUS,	"STANDARD-VERSUS-TRACKS",
			"STANDARDVERSUSTRACKS",			0 },
	{ C_STD_VERSUS,	"STD_VERSUS",		0,		0 },

	{ 0,0,0,0 }
    };

    s_nsec_t total_nsec = -GetTimerNSec(), get_track_nsec = 0;

    enumError max_err = ERR_OK;
    int next_slot = -1, count = 0;
    DEFINE_VAR(string_par);
    le_track_t *current_lt = 0;
    le_strpar_t spar = { .opt = ld->spar.opt };

    for (;;)
    {
	if (IsSectionOrEotSI(si,true))
	    break;

	bool define_set = false;

     #if LE_STRING_SET_ENABLED
	char ch = NextCharSI(si,true);
	if ( ch == '[' )
	{
	    ccp eol = GetEolSI(si);
	    if (eol)
	    {
		ScanFile_t *sf = si->cur_file;
		DASSERT(sf);
		ccp beg = sf->ptr+1;
		ccp ptr = memchr(beg,']',eol-beg);
		if ( ptr && ptr < eol )
		{
		    SetKeyNameLSP(&spar,beg,ptr-beg);
		    sf->ptr = ptr+1;
		    PRINT0("[%s] = %.*s\n",buf,(int)(eol-sf->ptr),sf->ptr);
		    define_set = true;
		}
	    }
	}
     #endif

	const KeywordTab_t *cmd = 0;
	if (!define_set)
	{
	    cmd = scan_cmd(si,commands,"TRACK-LIST",&max_err);
	    if (!cmd)
		continue;
	}

	const le_options_t opt = cmd ? cmd->opt : 0;
	if ( opt & O_STRING || define_set )
	{
	    if (!current_lt)
	    {
		if (!si->no_warn)
		{
		    ccp eol = FindNextLineFeedSI(si,true);
		    ScanFile_t *sf = si->cur_file;
		    DASSERT(sf);
		    ERROR0(ERR_WARNING,
			    "File %s, section [TRACK-LIST], line #%u:\nNo current track: %.*s\n",
			    sf->name, sf->line,
			    (int)(eol - sf->prev_ptr), sf->prev_ptr );
		}
		if ( max_err < ERR_WARNING )
		    max_err = ERR_WARNING;
		GotoEolSI(si);
		continue;
	    }

	    enumError err = ScanStringSI(si,&string_par);
	    if ( max_err < err )
		max_err = err;

	 #if LE_STRING_SET_ENABLED
	    if (define_set)
	    {
		SetSetLT(current_lt,&spar,string_par.str);
		continue;
	    }
	 #endif
	}

	ASSERT(cmd);
	switch(cmd->id)
	{
	 case C_SLOT:
	    ScanIntValueSI(si,&next_slot);
	    PRINT0("NEXT-SLOT: %d\n",next_slot);
	    CheckEolSI(si);
	    continue; // skip 'next_slot=-1'

	 case C_TRACK:
	    {
		current_lt = 0;
		le_track_t temp;
		if (!scan_track(&temp,si))
		{
		    count++;

		    get_track_nsec -= GetTimerNSec();
		    le_track_t *lt
			    = next_slot >= 0
			    ? DefineTrackLD(ld,next_slot,true)			// SLOT has highest priority
			    : IsRandomLETF(temp.flags)
			    ? DefineGroupTrackLD(ld,temp.track_type,true)	// append random tracks always
			    : DefineFreeTrackLD(ld,temp.track_type,true);	// fallback: search a free slot
		    get_track_nsec += GetTimerNSec();

		    if (lt)
		    {
			temp.track_slot = lt->track_slot;
			MoveLT(lt,&temp);
			if (*last_index_name)
			{
			 #if LE_TYPE_MARKERS_ENABLED
			    uint val = lt->track_slot;
			    if ( lt->track_type & LTTY_ARENA )
				val |= LTT_MARKER_ARENA;
			    if ( lt->track_type & LTTY_TRACK )
				val |= LTT_MARKER_TRACK;
			    DefineIntVar(&si->gvar,last_index_name,val);
			 #else
			    DefineIntVar(&si->gvar,last_index_name,lt->track_slot);
			 #endif
			}
			current_lt = lt;
		    }
		}
		ClearLT(&temp,0);
	    }
	    break;

	 case C_IDENT:
	    SetIdentOptLT(current_lt,string_par.str,false,true,ld->spar.opt);
	    break;

	 case C_FILE:
	    SetFileLT(current_lt,&spar,string_par.str);
	    break;

	 case C_NAME:
	    SetNameLT(current_lt,&spar,string_par.str);
	    break;

	 case C_XNAME:
	    SetXNameLT(current_lt,&spar,string_par.str);
	    break;

     #if LE_STRING_LIST_ENABLED
	 case C_LIST:
	    SetListLT(current_lt,&spar,string_par.str);
	    break;
     #endif // LE_STRING_LIST_ENABLED

	 case C_STD_ARENAS:
	    for ( int slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
		SetupStandardArenaLT(DefineFreeTrackLD(ld,LTTY_ARENA,true),slot);
	    current_lt = 0;
	    break;

	 case C_STD_VERSUS:
	    for ( int slot = MKW_TRACK_BEG; slot < MKW_TRACK_END; slot++ )
		SetupStandardTrackLT(DefineFreeTrackLD(ld,LTTY_TRACK,true),slot);
	    current_lt = 0;
	    break;
	}
	CheckEolSI(si);
	next_slot = -1;
    }

    ResetV(&string_par);

    if ( log_timing >= 1 )
    {
	total_nsec += GetTimerNSec();
	fprintf(stdlog,
		"%sLE-DEF: %d tracks scanned in %s,"
		" of which %s are searching free slots.\n",
		verbose >= 1 ? "   > " : "",
		count,
		PrintTimerNSec6(0,0,total_nsec,0),
		PrintTimerNSec6(0,0,get_track_nsec,0) );
    }

    return max_err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void add_reflist_to_cup
	( le_distrib_t *ld, const le_cup_ref_t *slot_list, int n_slots )
{
    if ( !ld || !slot_list || n_slots < 1 )
	return;

    while ( n_slots-- > 0 )
    {
     #if LE_TYPE_MARKERS_ENABLED
	le_cup_ref_t val = *slot_list++;
	le_cup_ref_t slot = val & ~LTT_MARKER__ALL;
	PRINT0("APPEND %d (val:%x)%s\n",slot,val,IsValidLecodeSlot(slot)?"":" INVALID!");
     #else
	le_cup_ref_t slot = *slot_list++;
	PRINT0("APPEND %d %s\n",slot,IsValidLecodeSlot(slot)?"":" INVALID!");
     #endif
	if (!IsValidLecodeSlot(slot))
	    continue;

	le_track_t *lt = GetTrackLD(ld,slot);
	if ( !IsVisibleLT(lt) )
	{
	 #if LE_TYPE_MARKERS_ENABLED
	    if ( val & LTT_MARKER_TRACK )
		AppendLECUP(&ld->cup_versus,slot);

	    if ( val & LTT_MARKER_ARENA )
		AppendLECUP(&ld->cup_battle,slot);
	 #endif
	    continue;
	}

	if ( lt->track_type & LTTY_TRACK )
	{
	    lt->user_flags |= LTTY_TRACK;
	    AppendLECUP(&ld->cup_versus,slot);
	}

	if ( lt->track_type & LTTY_ARENA )
	{
	    lt->user_flags |= LTTY_ARENA;
	    AppendLECUP(&ld->cup_battle,slot);
	}
    }
}

//-----------------------------------------------------------------------------

static void append_to_cup ( ScanInfo_t *si )
{
    if ( !si || !si->ld )
	return;

    for(;;)
    {
	char ch = NextCharSI(si,false);
	if (!ch)
	    break;

	int val;
	const enumError err = ScanIntValueSI(si,&val);
	if (err)
	    return;

	le_cup_ref_t ref = val;
	add_reflist_to_cup(si->ld,&ref,1);
	SkipCharSI(si,',');
    }
}

//-----------------------------------------------------------------------------

static void close_cup ( ScanInfo_t *si )
{
    if ( !si || !si->ld )
	return;

    for(;;)
    {
	char ch = NextCharSI(si,false);
	if (!ch)
	    break;

	int val;
	const enumError err = ScanIntValueSI(si,&val);
	if (err)
	    return;

	if ( val == LTTY_ARENA )
	    CloseLECUP(&si->ld->cup_battle);
	else if ( val == LTTY_TRACK )
	    CloseLECUP(&si->ld->cup_versus);
	SkipCharSI(si,',');
    }
}

//-----------------------------------------------------------------------------

static enumError ScanLeDefCUPS
(
    le_distrib_t	*ld,		// valid structure
    ScanInfo_t		*si,		// valid structure
    int			pass		// 1|2
)
{
    if ( !ld || !si )
	return ERR_MISSING_PARAM;

    enum { C_APPEND, C_NEW_CUP, C_STD_CUPS, C_MKWFUN, C_UNUSED };
    enum { O_BOOL_VAL = 1, O_BT = 2, O_VS = 4 };
    static const KeywordTab_t commands[] =
    {
	{ C_APPEND,	"APPEND",		0,			0 },
	{ C_NEW_CUP,	"NEW-CUP",		"NEWCUP",		0 },

	{ C_STD_CUPS,	"STANDARD-BATTLE-CUPS",	"STANDARDBATTLECUPS",	O_BT },
	{ C_STD_CUPS,	"STANDARD-VERSUS-CUPS",	"STANDARDVERSUSCUPS",	O_VS },
	{ C_STD_CUPS,	"STANDARD-CUPS",	"STANDARDCUPS",		O_BT|O_VS },

	{ C_MKWFUN,	"MKWFUN-RANDOM-CUP",	"MKWFUNRANDOMCUP",	0 },

	{ C_UNUSED,	"ADD-UNUSED-ARENAS",	"ADDUNUSEDARENAS",	O_BOOL_VAL|O_BT },
	{ C_UNUSED,	"ADD-UNUSED-TRACKS",	"ADDUNUSEDTRACKS",	O_BOOL_VAL|O_VS },
	{ C_UNUSED,	"ADD-UNUSED",		"ADDUNUSED",		O_BOOL_VAL|O_BT|O_VS },

	{ 0,0,0,0 }
    };

    enumError max_err = ERR_OK;
    bool bool_val = false;

    for (;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	const KeywordTab_t *cmd = scan_cmd(si,commands,"TRACK-LIST",&max_err);
	if (!cmd)
	    continue;

	if ( cmd->opt & O_BOOL_VAL )
	{
	    SkipCharSI(si,'=');
	    int val;
	    const enumError err = ScanIntValueSI(si,&val);
	    if (!err)
		bool_val = val > 0;
	    else if ( max_err < err )
		max_err = err;
	}

	switch(cmd->id)
	{
	 case C_APPEND:
	    append_to_cup(si);
	    break;

	 case C_NEW_CUP:
	    close_cup(si);
	    break;

	 case C_STD_CUPS:
	    if (cmd->opt&O_BT)
		add_reflist_to_cup(ld,standard_bt_ref,MKW_N_ARENAS);
	    if (cmd->opt&O_VS)
		add_reflist_to_cup(ld,standard_vs_ref,MKW_N_TRACKS);
	    break;

	 case C_MKWFUN:
	    for ( int slot = MKW_LE_RANDOM_BEG; slot <= MKW_LE_RANDOM_END; slot++ )
		DefineTrackLD(ld,slot,true);
	    add_reflist_to_cup(ld,mkwfun_random_ref,MKW_N_LE_RANDOM);
	    break;

	 case C_UNUSED:
	    if (cmd->opt&O_BT)
		ld->scan.bt.add_unused = bool_val;
	    if (cmd->opt&O_VS)
		ld->scan.vs.add_unused = bool_val;
	    break;
	}
	CheckEolSI(si);
    }

    return max_err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanLeDefLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname,		// NULL or filename for error messages
    CopyMode_t		cm_fname	// copy mode of 'fname'
)
{
    ScanText_t st;
    SetupScanText(&st,data,data_size);
    SetFilenameScanText(&st,fname,cm_fname);
    const enumError err = ScanLeDefSTLD(ld,init_ld,&st);
    ResetScanText(&st);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanLeDefSTLD
(
    le_distrib_t	*ld,		// valid structure
    bool		init_ld,	// TRUE: initialze 'ld'
    ScanText_t		*st		// initialized source
)
{
    enum { SECT_TOOL, SECT_TRACKS, SECT_CUPS, SECT_END };
    static const KeywordTab_t sections[] =
    {
	{ SECT_TOOL,	"TOOL",		0,		1 },
	{ SECT_TRACKS,	"TRACK-LIST",	"TRACKLIST",	1 },
	{ SECT_CUPS,	"CUP-LIST",	"CUPLIST",	2 },
	{ SECT_END,	"END",		0,		0 },
	{ 0,0,0,0 }
    };

    if ( !ld || !st )
	return ERR_MISSING_PARAM;

    if ( init_ld || !ld->is_initialized )
	InitializeLD(ld);

    if (!ld->second_ledef)
    {
	ld->second_ledef = true;
	ResetTracksAndCupsLD(ld);
	memset(&ld->scan,0,sizeof(ld->scan));
	ld->scan.valid = true;
	ld->scan.bt.add_unused = true;
	ld->scan.vs.add_unused = true;
	ClearUserFlagsAndCupsLD(ld);
    }

    ScanInfo_t si;
    InitializeSI(&si,st->data,st->data_size,st->fname,0);
    si.predef = SetupVarsLECODE();
    si.ld = ld;

    enumError max_err = ERR_OK;
    for ( int pass = 1; pass <= 2; pass++ )
    {
	if ( pass > 1 )
	    RestartSI(&si);

	PRINT("----- pass %u\n",pass);
	max_err = ERR_OK;
	ld->is_pass2 = pass == 2;
	si.no_warn = pass == 1;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", pass );

	for(;;)
	{
	    char ch = NextCharSI(&si,true);
	    if (!ch)
		break;

	    if ( ch != '[' )
	    {
		NextLineSI(&si,true,false);
		continue;
	    }
	    ResetLocalVarsSI(&si,0);

	    si.cur_file->ptr++;
	    char name[40];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    PRINT0("--> pass=%u: #%04u: %s\n",pass,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,sections);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    PRINT0("Path %d, section %s\n",pass,cmd->name1);
	    PRINT0("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    // check pass
	    if ( cmd->opt == pass )
		si.no_warn = false;
	    else if ( cmd->opt )
		continue; // wrong pass
	    else
		si.no_warn = pass == 1;

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case SECT_TRACKS: err = ScanLeDefTRACKS(ld,&si,pass); break;
		case SECT_CUPS:   err = ScanLeDefCUPS(ld,&si,pass); break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}
     #if HAVE_PRINT0
	if ( logging >= 1 )
	{
	    printf("VAR DUMP/GLOBAL PASS %u:\n",pass);
	    DumpVarMap(stdout,3,&si.gvar,false);
	}
     #endif
    }


    //--- terminate


    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextKMP) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: cup helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

void DirtyCupLD ( le_distrib_t *ld, le_track_type_t ltty )
{
    if (ld)
    {
	if ( ltty & LTTY_ARENA )
	    ld->cup_battle.dirty = true;
	if ( ltty & LTTY_TRACK )
	    ld->cup_versus.dirty = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

le_track_t * FindFillTrackLD ( le_distrib_t *ld, le_cup_t *lc, bool allow_candidate )
{
    if ( !ld || !lc )
	return 0;

    le_track_type_t ltty = GetLttyLECUP(lc);

    le_track_t *fill = GetTrackLD(ld,lc->fill_slot);
    if ( !fill || fill->track_status != LTS_FILL || !(fill->track_type&ltty) )
    {
	fill = 0;
	le_track_t *lt = ld->tlist;
	for ( int slot = 0; slot < ld->tlist_used ; slot++, lt++ )
	    if ( lt->track_status >= LTS_FILL && lt->track_type & ltty )
	    {
		if ( lt->track_status == LTS_FILL )
		{
		    // we have already a fill track
		    fill = lt;
		    lc->fill_slot = fill->track_slot;
		    break;
		}
		if ( allow_candidate && !fill )
		    fill = lt;
	    }
    }
    return fill;
}

///////////////////////////////////////////////////////////////////////////////

bool PrintCupRefLD ( FILE *f, le_distrib_t *ld, le_cup_t *lc )
{
    if ( !f || !lc || !lc->used )
	return false;

    le_track_t *fill = FindFillTrackLD(ld,lc,true);
    const int fill_slot = fill ? fill->track_slot : -999;
    ccp type = lc->tracks == BMG_BCUP_TRACKS ? "BT" : "VS";

 #if 1
    le_cup_ref_t *cup = lc->list;
    for ( int i = 0; i< lc->used; i++ )
    {
	fprintf(f,"%s|%u|",type,i);
	for ( int nt = lc->tracks; nt > 0; nt--, cup++ )
	    fprintf(f,"%d|", *cup == fill_slot ? -1 : *cup );
	fputs("\r\n",f);
    }
    return true;
 #else
    if ( lc->tracks == BMG_BCUP_TRACKS )
    {
	le_cup_ref_t *cup = lc->list;
	for ( int i = 0; i< lc->used; i++, cup += BMG_BCUP_TRACKS )
	    fprintf(f,"bt|%u|%d|%d|%d|%d|%d|\r\n",
		i, cup[0], cup[1], cup[2], cup[3], cup[4] );
	return true;
    }

    if ( lc->tracks == BMG_RCUP_TRACKS )
    {
	le_cup_ref_t *cup = lc->list;
	for ( int i = 0; i< lc->used; i++, cup += BMG_RCUP_TRACKS )
	    fprintf(f,"vs|%u|%d|%d|%d|%d|\r\n",
		i, cup[0], cup[1], cup[2], cup[3] );
	return true;
    }
    ASSERT(0);
    return false;
 #endif
}

///////////////////////////////////////////////////////////////////////////////

void update_cup_helper
(
    le_distrib_t	*ld,		// valid distrib
    le_cup_t		*lc,		// cup to update
    le_track_type_t	ltty,		// LTTY_ARENA | LTTY_TRACK
    le_cup_ref_t	p_fallback,	// fall back if no track is found
    bool		add_missed	// true: add missed files (user_flag==0)
)
{
    DASSERT(ld);
    DASSERT(lc);
    PRINT("update_cup_helper(%d,fb=%d,add=%d) #%u: used=%d, dirty=%d\n",
		lc->tracks, p_fallback, add_missed,
		__LINE__, lc->used, lc->dirty );


    //--- remove invalid cup references & mark used

    le_cup_ref_t *end = lc->list + lc->used * lc->tracks;
    for ( le_cup_ref_t *ptr = lc->list; ptr < end; ptr++ )
    {
	if ( *ptr >= 0 )
	{
	    le_track_t *lt = GetTrackLD(ld,*ptr);
	    if ( IsVisibleLT(lt) && lt->track_type & ltty )
		lt->user_flags |= ltty;
	    else
		*ptr = -1;
	}
    }


    //--- append not used tracks

    if (add_missed)
    {
	le_track_t *lt = ld->tlist;
	for ( int slot = 0; slot < ld->tlist_used; slot++, lt++ )
	    if ( IsVisibleLT(lt) && lt->track_type & ltty && !(lt->user_flags&ltty) )
		AppendLECUP(lc,slot);
    }


    //--- pack, even num of cups

    if (!lc->close_used)
	PackLECUP(lc);
    EvenLECUP(lc);


    //--- fill cups

    if (HaveInvalidLECUP(lc))
    {
	le_strpar_t spar = { .opt = LEO__DEFAULT|LEO_IN_ALL };
	le_track_t *fill = FindFillTrackLD(ld,lc,true);

	if (!fill)
	{
	    fill = DefineFreeTrackLD(ld,ltty,true);
	    SetupStandardTrackLT(fill,p_fallback);
	    fill->track_status = LTS_FILL;
	    lc->fill_slot = fill->track_slot;
	    lc->fill_src  = -1;
	}
	else if ( fill->track_status != LTS_FILL )
	{
	    le_track_t *next = DefineTrackLD(ld,ld->tlist_used,true);
	    if (next)
	    {
		next->track_status	= LTS_FILL;
		next->property		= fill->property;
		next->music		= fill->music;
		lc->fill_slot		= next->track_slot;
		lc->fill_src		= fill->track_slot;

		SetFileLT(next,&spar,GetFileLT(fill,0));
		SetIdentOptLT(next,GetIdentLT(fill,false,true,0),false,false,LEO_IN_ALL);
		SetIdentOptLT(next,GetIdentLT(fill,true, true,0),true, false,LEO_IN_ALL);
		fill = next;
	    }
	}

	if (fill)
	{
	    if ( fill->track_status == LTS_FILL )
	    {
		fill->flags = 0;
		fill->track_type = ltty;
		SetNameLT(fill,&spar,"-");
	    }

	    FillLECUP(lc,fill->track_slot);
	}
    }


    //--- assign cup_slot's

    if ( lc->tracks == 4 )
	DumpCupSlots(ld,"before assign");

    end = lc->list + lc->used * lc->tracks;
    for ( le_cup_ref_t *ptr = lc->list; ptr < end; ptr++ )
    {
	if ( *ptr >= 0 )
	{
	    le_track_t *lt = GetTrackLD(ld,*ptr);
	    if ( lt && !lt->cup_slot && lt->track_type & ltty )
	    {
		lt->cup_slot = GetCupSlotLECUP(lc,ptr);
		if ( lt->flags & LETF_RND_HEAD )
		{
		    for ( int hslot = lt->track_slot+1; hslot < ld->tlist_used; hslot++ )
		    {
			le_track_t *hlt = GetTrackLD(ld,hslot);
			if ( IsActiveLT(hlt) && IsHiddenLETF(hlt->flags) )
			    hlt->cup_slot = lt->cup_slot;
			else
			    break;
		    }
		}
	    }
	}
    }

    if ( lc->tracks == 4 )
	DumpCupSlots(ld,"behind assign");

    //--- terminate

    lc->dirty = false;
    PRINT("update_cup_helper(%d) #%u: used=%d, dirty=%d\n",lc->tracks,__LINE__,lc->used,lc->dirty);
}

//-----------------------------------------------------------------------------

void UpdateCupsLD ( le_distrib_t *ld )
{
    if (!ld)
	return;

    CloseArchLD(ld);
    if ( ld->cup_battle.used && !ld->cup_battle.dirty && ld->cup_versus.used && !ld->cup_versus.dirty )
	return;

    PRINT0("UpdateCupsLD() N=%d,%d, dirty=%d,%d\n",
		ld->cup_battle.used, ld->cup_versus.used,
		ld->cup_battle.dirty, ld->cup_versus.dirty );
    s_nsec_t dur_nsec = -GetTimerNSec();


    //--- clear user flags

    CheckTracksLD(ld);
    ClearUserFlagsAndCupsLD(ld);


    //--- battle cups

    if (!ld->cup_battle.used)
	add_reflist_to_cup(ld,standard_bt_ref,MKW_N_ARENAS);

    update_cup_helper( ld, &ld->cup_battle, LTTY_ARENA,MKW_ARENA_BEG,
			!ld->scan.valid || ld->scan.bt.add_unused );


    //--- versus cups

    if (!ld->cup_versus.used)
	add_reflist_to_cup(ld,standard_vs_ref,MKW_N_TRACKS);

    update_cup_helper( ld, &ld->cup_versus, LTTY_TRACK,MKW_TRACK_BEG,
			!ld->scan.valid || ld->scan.vs.add_unused );


    //--- terminate

    if ( log_timing >= 1 )
    {
	dur_nsec += GetTimerNSec();
	fprintf(stdlog,"%sUpdateCupsLD() N=%d,%d, done in %s\n",
		verbose >= 1 ? "   > " : "",
		ld->cup_battle.used, ld->cup_versus.used,
		PrintTimerNSec6(0,0,dur_nsec,0) );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: manage archive		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[track-arch]]

void ResetLTA ( le_track_arch_t *ta, bool reset_lt )
{
    if (ta)
    {
	if (reset_lt)
	    ResetLT(&ta->lt);
	FreeString(ta->name_order);
	FreeString(ta->version);
	memset(ta,0,sizeof(*ta));
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetArchLD ( le_distrib_t *ld )
{
    if (ld)
    {
	le_track_arch_t *ta  = ld->arch;
	le_track_arch_t *end = ta + ld->arch_used;
	for ( ; ta < end; ta++ )
	    ResetLTA(ta,true);
	ld->arch = 0;
	ld->arch_used = 0;
	ld->arch_size = 0;

	exmem_key_t *ek   = ld->group.list;
	exmem_key_t *gend = ek + ld->group.used;
	for ( ; ek < gend; ek++ )
	{
	    le_group_info_t *gi = (le_group_info_t*)ek->data.data.ptr;
	    FREE(gi->list);
	}
	ResetEML(&ld->group);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AddToGroup ( le_distrib_t *ld, le_track_arch_t *ta, mem_t grp )
{
    if ( !ld || !ta || !grp.len )
	return;

    mem_t pre = { .ptr = ta->lt.track_type == LTTY_ARENA ? "A" : "T", .len = 1 };
    mem_t key = MemCat2A(pre,grp);
    PRINT0("+GRP[%.*s] += %d\n",key.len,key.ptr,ta->lt.track_slot);

    exmem_key_t *ek = FindInsertEML(&ld->group,key.ptr,CPM_MOVE,0);
    if (!ek->data.data.ptr)
    {
	ek->data.data.ptr	= CALLOC(1,sizeof(le_group_info_t));
	ek->data.data.len	= sizeof(le_group_info_t);
	ek->data.is_alloced	= true;
	le_group_info_t *gi	= (le_group_info_t*)ek->data.data.ptr;
	gi->group		= ld->group.used;
    }

    le_group_info_t *gi		= (le_group_info_t*)ek->data.data.ptr;
    ta->group			= gi->group;

 #if 0
    char xbuf[30];
    snprintf(xbuf,sizeof(xbuf),"%d %s",ta->group,ek->key);
    SetByKeyLT(&ta->lt,"grp",xbuf);
 #endif


    //-- append index to list;

    if ( ta->lt.flags & LETF_RND_HEAD && !gi->n_head++ )
	gi->head1 = ta->lt.track_slot;
    if ( ta->lt.flags & LETF_RND_GROUP )
	gi->n_group++;

    if ( gi->used >= gi->size )
    {
	gi->size = gi->used + 10;
	gi->list = REALLOC( gi->list, gi->size * sizeof(*gi->list) );
    }
    ASSERT( gi->used < gi->size );
    gi->list[gi->used++] = ta->lt.track_slot;
}

///////////////////////////////////////////////////////////////////////////////

le_track_arch_t * GetNextArchLD ( le_distrib_t *ld )
{
    if (!ld)
	return 0;

    if ( ld->arch_used >= ld->arch_size )
    {
	static const int tab[] = { 100, 200, 500, 100, 2000, LE_MAX_TRACKS, 0 };
	uint need = ld->arch_used + 1;
	for ( const int *ptr = tab; *ptr; ptr++ )
	    if ( need <= *ptr )
	    {
		need = *ptr;
		break;
	    }

	ld->arch = REALLOC( ld->arch, need * sizeof(*ld->arch) );
	memset( ld->arch + ld->arch_size, 0, ( need - ld->arch_size ) * sizeof(*ld->arch) );
	ld->arch_size = need;
    }

    DASSERT( ld->arch_used < ld->tlist_size );

    le_track_arch_t *ta = ld->arch + ld->arch_used;
    memset(ta,0,sizeof(*ta));
    InitializeLT(&ta->lt);

    ta->lt.track_slot	= ld->arch_used++; // index of 'arch'
    ta->lt.track_status	= LTS_EXPORT;
    ta->attr_order	= 1000;
    ta->plus_order	= 10000000;
    ta->game_order	= 10000000;
    return ta;
}

///////////////////////////////////////////////////////////////////////////////

enumError AddToArchLD ( le_distrib_t *ld, raw_data_t *raw )
{
    if ( !ld || !raw )
	return ERR_MISSING_PARAM;

    char buf[LE_TRACK_STRING_MAX+1];
    exmem_dest_t exdest = { .buf = buf, .buf_size = sizeof(buf), .try_circ = true };

    szs_file_t szs;
    AssignSZS(&szs,true,raw->data,raw->data_size,false,raw->fform,raw->fname);

    analyse_szs_t as;
    AnalyseSZS(&as,true,&szs,raw->fname);
    PRINT0("SHA1: %s, slots: r=%d a=%d m=%d\n",
	as.sha1_szs,
	as.slotinfo.race_slot, as.slotinfo.arena_slot, as.slotinfo.music_index );

    split_filename_t spf;
    AnalyseSPF(&spf,true,raw->fname,0,CPM_LINK,opt_plus);

    enumError err = ERR_WARNING;
    if ( as.valid_track && !spf.f_d.len )
    {
	ccp slash = strrchr(raw->fname,'/');
	if (slash)
	    ScanOptTrackSource(raw->fname,slash-raw->fname,opt_szs_mode);
	else
	    ScanOptTrackSource(".",1,opt_szs_mode);

	err = ERR_OK;
	le_track_arch_t *ta = GetNextArchLD(ld);

	if (szs.is_arena)
	{
	    ta->lt.track_type	= LTTY_ARENA;
	    ta->lt.property	= MKW_ARENA_BEG;

	    if (as.slotinfo.arena_slot)
	    {
		int slot = as.slotinfo.arena_slot - 11;
		slot = slot/10*5 + slot%10;
		if ( (uint)slot < MKW_N_ARENAS )
		    ta->lt.property = arena_pos_default[slot] + MKW_ARENA_BEG;
	    }
	}
	else
	{
	    ta->lt.track_type	= LTTY_TRACK;
	    ta->lt.property	= MKW_TRACK_BEG;

	    if (as.slotinfo.race_slot)
	    {
		int slot = as.slotinfo.race_slot - 11;
		slot = slot/10*4 + slot%10;
		if ( (uint)slot < MKW_N_TRACKS )
		    ta->lt.property = track_pos_default[slot];
	    }
	}
	ta->lt.music = as.slotinfo.music_index
			? as.slotinfo.music_index
			: NormalizeMusicID(ta->lt.property);

	if (spf.plus.len)
	    ta->lt.flags |= LETF_NEW;


	//-- setup basic strings

	mem_t space = { .ptr = " ", .len = 1 };
	le_strpar_t spar = { .opt = LEO__DEFAULT | LEO_IN_ALL | LEO_OUT_ALL };

	SetIdentLT(&ta->lt,&spar,as.sha1_szs);

	StringCopySM(buf,sizeof(buf),spf.f_name.ptr,spf.f_name.len);
	SetFileLT(&ta->lt,&spar,buf);

     #if SUPPORT_SPLIT_SIGN
	mem_t list[]	= { spf.sign, spf.boost, spf.game, spf.name, spf.extra };
     #else
	mem_t list[]	= { spf.boost, spf.game, spf.name, spf.extra };
     #endif
	mem_src_t msrc	= { .src = list, .n_src = sizeof(list)/sizeof(*list) };
	exmem_t exsum	= ExMemCat(&exdest,space,&msrc);
	SetNameLT(&ta->lt,&spar,exsum.data.ptr);
	FreeExMem(&exsum);

	StringCopySM(buf,sizeof(buf),spf.norm.ptr,spf.norm.len);
	SetXNameLT(&ta->lt,&spar,buf);

	//-- setup arch extensions

	mem_t sum = MemCatSep3A(space,spf.name,spf.version,spf.extra);
	ta->name_order = sum.ptr;
	ta->version = MEMDUP(spf.version.ptr,spf.version.len);

	//-- setup order & group

	mem_t grp = {0};
	mem_t attr_list[100+1];
	const int n = SplitByCharMem(attr_list,100,spf.attribs,',');
	for ( int i = 0; i < n; i++ )
	{
	    mem_t attr = attr_list[i];
	    if ( !attr.ptr || !attr.len )
		continue;

	    if (!memcmp(attr.ptr,"head=",5))
	    {
		ta->lt.flags |= LETF_RND_HEAD;
		grp = MidMem(attr,5,attr.len);
	    }
	    else if (!memcmp(attr.ptr,"grp=",4))
	    {
		ta->lt.flags |= LETF_RND_GROUP;
		if (!grp.len)
		    grp = MidMem(attr,4,attr.len);
	    }
	    else if ( grp.len && !StrCmpMem(attr,"grp") )
	    {
		ta->lt.flags |= LETF_RND_GROUP;
	    }
	    else if ( !StrCmpMem(attr,"new") )
		ta->lt.flags |= LETF_NEW;
	    else if (!memcmp(attr.ptr,"order=",6))
		ta->attr_order = str2l(attr.ptr+6,0,10);
	}

	if (grp.len)
	    AddToGroup(ld,ta,grp);

	//--- plus_order, game_order

	ta->plus_order = spf.plus_order;
	ta->game_order = spf.game_order;
//	ta->game_color = spf.game_color;


	//-- clean

	PRINT0("order=%d,%d,%d, grp=%d \"%.*s\"\n",
		ta->attr_order, ta->plus_order, ta->game_order,
		ta->group, grp.len, grp.ptr );
    }

    //-- clean & return

    ResetSPF(&spf);
    ResetAnalyseSZS(&as);
    ResetSZS(&szs);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_order ( const le_track_arch_t **pa, const le_track_arch_t **pb )
{
    const le_track_arch_t *a = *pa;
    const le_track_arch_t *b = *pb;
    ASSERT(a&&b);

    int res = ( a->lt.track_type & LTTY_TRACK ) - ( b->lt.track_type & LTTY_TRACK );
    if (!res)
    {
	res = a->attr_order - b->attr_order;
	if (!res)
	{
	    res = a->plus_order - b->plus_order;
	    if (!res)
	    {
		res = strcasecmp(a->name_order,b->name_order);
		if (!res)
		{
		    res = a->game_order - b->game_order;
		    if (!res)
			res = (int)a->lt.track_slot - (int)b->lt.track_slot;
		}
	    }
	}
    }
    return res;
}

//-----------------------------------------------------------------------------

bool CloseArchLD ( le_distrib_t *ld )
{
    if ( !ld || !ld->arch )
	return false;

    //-- manage groups

    exmem_key_t *ek   = ld->group.list;
    exmem_key_t *gend = ek + ld->group.used;
    for ( ; ek < gend; ek++ )
    {
	le_group_info_t *gi = (le_group_info_t*)ek->data.data.ptr;
	const bool clear_group = !gi->n_group || gi->n_group == 1 && !gi->n_head;

	if ( !clear_group && !gi->n_head && gi->used )
	{
	    // first member becomes head
	    gi->n_head++;
	    gi->head1 = *gi->list;
	}

	for ( int i = 0; i < gi->used; i++ )
	{
	    int index = gi->list[i];
	    PRINT0("A: %zd/%d : %d/%d/%d : %d/%d/%d\n",
			ek-ld->group.list, ld->group.used,
			i, gi->used, gi->size,
			index, ld->arch_used, ld->arch_size );
	    ASSERT( index < ld->arch_used );

	    le_track_arch_t *ta = ld->arch + index;
	    if (clear_group)
	    {
		if ( ta->lt.flags & LETF_RND_GROUP )
		{
		    ta->lt.flags &= ~LETF__RND;
		    ta->group = 0;
		}
		else
		    ta->lt.track_status = LTS_VALID; // deactivate track
	    }
	    else if ( index == gi->head1 )
		ta->lt.flags |= LETF_RND_HEAD;
	    else if ( ta->lt.flags & LETF_RND_GROUP )
		ta->lt.flags &= ~LETF_RND_HEAD;
	    else
		ta->lt.track_status = LTS_VALID; // deactivate track
	}
    }


    //--- [[2do]] order

    le_track_arch_t **order = MALLOC( (ld->arch_used+1) * sizeof(*order) );
    le_track_arch_t **ptr = order;

    le_track_arch_t *ta  = ld->arch;
    le_track_arch_t *end = ta + ld->arch_used;
    for ( ; ta < end; ta++ )
	if ( ta->lt.track_status >= LTS_EXPORT && !IsHiddenLETF(ta->lt.flags) )
	    *ptr++ = ta;
    *ptr = 0;

    int n_order = ptr - order;
    if ( n_order > 1 )
	qsort( order, n_order, sizeof(*order), (qsort_func)cmp_order );


    //--- assign tracks

    for ( ptr = order; *ptr; ptr++ )
    {
	le_track_arch_t *ta = *ptr;
	le_track_t *lt	= IsRandomLETF(ta->lt.flags)
			? DefineGroupTrackLD(ld,ta->lt.track_type,true)
			: DefineFreeTrackLD(ld,ta->lt.track_type,true);
	ta->lt.track_slot = lt->track_slot;
	*lt = ta->lt;

	if (ta->group)
	{
	    for ( exmem_key_t *ek = ld->group.list; ek < gend; ek++ )
	    {
		le_group_info_t *gi = (le_group_info_t*)ek->data.data.ptr;
		if ( gi->group == ta->group )
		{
		    for ( int i = 0; i < gi->used; i++ )
		    {
			int index = gi->list[i];
			PRINT0("B: %zd/%d : %d/%d/%d : %d/%d/%d\n",
				ek-ld->group.list, ld->group.used,
				i, gi->used, gi->size,
				index, ld->arch_used, ld->arch_size );
			ASSERT( index < ld->arch_used );

			le_track_arch_t *ta2 = ld->arch + index;
			if ( ta2 != ta && IsHiddenLETF(ta2->lt.flags) )
			{
			    le_track_t *lt2 = DefineFreeTrackLD(ld,0,true);
			    ta2->lt.track_slot = lt2->track_slot;
			    *lt2 = ta2->lt;
			    ResetLTA(ta2,false);
			}
		    }
		    break;
		}
	    }
	}

	ResetLTA(ta,false);
    }

    ld->cup_battle.dirty = true;
    ld->cup_versus.dirty = true;

    FREE(order);
    ResetArchLD(ld);
    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: import			///////////////
///////////////////////////////////////////////////////////////////////////////

void ImportLD ( le_distrib_t *ld, const le_distrib_t *src )
{
    // [[2do]] ???
}

///////////////////////////////////////////////////////////////////////////////

void SetupLparLD ( le_distrib_t *ld, bool load_lpar )
{
    if ( !ld || ld->lpar )
	return;

    ld->lpar = MALLOC(sizeof(*ld->lpar));
    InitializeLPAR(ld->lpar,load_lpar);
    PatchLPAR(ld->lpar);
    ld->lpar->limit_mode = LPM_FORCE_AUTOMATIC;
}

///////////////////////////////////////////////////////////////////////////////

void ImportLparLD ( le_distrib_t *ld, const le_lpar_t *lpar )
{
    if ( !ld || !lpar )
	return;

    SetupLparLD(ld,false);
    ASSERT(ld->lpar);
    memcpy(ld->lpar,lpar,sizeof(*ld->lpar));
    PatchLPAR(ld->lpar);
    ld->lpar->limit_mode = LPM_FORCE_AUTOMATIC;
}

///////////////////////////////////////////////////////////////////////////////

void ImportAnaLD ( le_distrib_t *ld, const le_analyse_t *ana )
{
    if ( !ld || !ana || !ana->valid )
	return;


    //--- import lecode binary + lpar

    if ( ld->spar.opt & LEO_IN_LECODE )
	AssignLecodeLD(ld,GetLERegion(ana),ana->data,ana->size);
    ImportLparLD(ld,&ana->lpar);


    //--- import tracks

    DefineTrackLD(ld,ana->n_slot-1,false); // alloc reference list of good size

    for ( int slot = 0; slot < ana->n_slot; slot++ )
    {
	if ( slot == MKW_ARENA_END )
	    slot = LE_FIRST_CT_SLOT;

	if (!IsLESlotUsed(ana,slot))
	    continue;

	le_track_t *lt = DefineTrackLD(ld,slot,true);
	if (lt)
	{
	    if (ana->property)	lt->property = ana->property[slot];
	    if (ana->music)	lt->music    = ana->music[slot];
	    if (ana->flags)	lt->flags    = ana->flags[slot];
	}
    }


    //--- arena cups and slots

    if ( ana->n_cup_arena && ana->cup_arena )
    {
	const uint *ref_src = ana->cup_arena;
	for ( int cup_idx = 0; cup_idx < ana->n_cup_arena; cup_idx++ )
	{
	    le_cup_ref_t *ref_dest = DefineLECUP(&ld->cup_battle,cup_idx);
	    for ( int i = 0; i < BMG_BCUP_TRACKS; i++ )
	    {
		int slot = htonl( ana->cup_arena[ BMG_BCUP_TRACKS*cup_idx + i ] );
		le_track_t *lt = DefineTrackLD(ld,slot,false);
		if ( lt && IsArenaLTTY(lt->track_type) )
		{
		    lt->cup_slot = 10*cup_idx + i + 11;
		    if ( ref_dest && ref_src )
			*ref_dest++ = ntohl(*ref_src++);
		}
	    }
	}
    }


    //--- racing cups and slots

    if ( ana->n_cup_track && ana->cup_track )
    {
	const uint *ref_src = ana->cup_track;
	for ( int cup_idx = 0; cup_idx < ana->n_cup_track; cup_idx++ )
	{
	    le_cup_ref_t *ref_dest = DefineLECUP(&ld->cup_versus,cup_idx);
	    for ( int i = 0; i < BMG_RCUP_TRACKS; i++ )
	    {
		int slot = htonl( ana->cup_track[ BMG_RCUP_TRACKS*cup_idx + i ] );
		le_track_t *lt = DefineTrackLD(ld,slot,false);
		if ( lt && !IsArenaLTTY(lt->track_type) )
		{
		    lt->cup_slot = 10*cup_idx + i + 11;
		    if ( ref_dest && ref_src )
			*ref_dest++ = ntohl(*ref_src++);
		}
	    }
	}
    }

    PackTracksLD(ld);
}

///////////////////////////////////////////////////////////////////////////////

void ImportCtcodeLD ( le_distrib_t *ld, const ctcode_t *ctcode )
{
    if ( !ld || !ctcode )
	return;


    //--- setup lower slots with defaults

    for ( int slot = 0; slot < MKW_ARENA_END; slot++ )
    {
	le_track_t *lt	= DefineTrackLD(ld,slot,true);
	lt->property	= slot;
	lt->music	= GetMkwMusicSlot(slot);
    }


    //--- copy track properties

    le_strpar_t spar = { .opt = LEO__DEFAULT|LEO_IN_ALL };

    const ctcode_crs1_data_t *td = ctcode->crs->data;
    if (td)
    {
	const int max = ctcode->n_tracks;
	for ( int slot = 0; slot < max; slot++, td++ )
	{
	    if ( slot == MKW_ARENA_END )
	    {
		slot = LE_FIRST_CT_SLOT;
		td = ctcode->crs->data + slot;
	    }

	    le_track_t *lt = DefineTrackLD(ld,slot,true);
	    if (lt)
	    {
		if (IsMkwArena(slot))
		{
		    const u8 prop = ctcode->arena.prop[slot-MKW_ARENA_BEG];
		    if (prop) // invalid if 0
		    {
			lt->property = prop;
			lt->music    = ctcode->arena.music[slot-MKW_ARENA_BEG];
		    }
		}
		else
		{
		    lt->property = ntohl(td->property_id);
		    lt->music    = ntohl(td->music_id);
		    lt->flags    = ctcode->le_flags[slot];
		}

		char buf[200];
		const int str_idx = ctcode->ctb.track_name1.beg + slot;
		const bmg_item_t *bi;

		bi = FindItemBMG(&ctcode->track_ident,str_idx);
		if ( bi && bi->text )
		{
		    PrintString16BMG(buf,sizeof(buf),bi->text,bi->len,BMG_UTF8_MAX,0,1);
		    SetIdentLT(lt,&spar,buf);
		}

		bi = FindItemBMG(&ctcode->track_file,str_idx);
		if ( bi && bi->text )
		{
		    PrintString16BMG(buf,sizeof(buf),bi->text,bi->len,BMG_UTF8_MAX,0,1);
		    SetFileLT(lt,&spar,buf);
		}

		bi = FindItemBMG(&ctcode->track_string,str_idx);
		if ( bi && bi->text )
		{
		    PrintString16BMG(buf,sizeof(buf),bi->text,bi->len,BMG_UTF8_MAX,0,1);
		    SetNameLT(lt,&spar,buf);
		}

		bi = FindItemBMG(&ctcode->track_xstring,str_idx);
		if ( bi && bi->text )
		{
		    PrintString16BMG(buf,sizeof(buf),bi->text,bi->len,BMG_UTF8_MAX,0,1);
		    SetXNameLT(lt,&spar,buf);
		}
	    }
	}
    }


    //--- battle cups and tracks

    const ctcode_cup1_data_t *cup;
    cup = ctcode->cup_battle;
    if (cup )
    {
	uint max = ctcode->n_battle_cups;
	if ( max > ld->cup_battle.max )
	     max = ld->cup_battle.max;

	for ( int cup_idx = 0; cup_idx < max; cup_idx++, cup++ )
	{
	    le_cup_ref_t *dest = DefineLECUP(&ld->cup_battle,cup_idx);
	    for ( int i = 0; i < BMG_BCUP_TRACKS; i++ )
	    {
		const int slot = ntohl(cup->track_id[i]);
		le_track_t *lt = DefineTrackLD(ld,slot,false);
		if ( lt && IsArenaLTTY(lt->track_type) )
		{
		    lt->cup_slot = 10*cup_idx + i + 11;
		    if (dest)
			*dest++ = slot;
		}
	    }
	}
    }


    //--- racing cups and tracks

    cup = ctcode->cup_racing;
    if (cup)
    {
	uint max = ctcode->n_racing_cups;
	if ( max > ld->cup_versus.max )
	     max = ld->cup_versus.max;

	for ( int cup_idx = 0; cup_idx < max; cup_idx++, cup++ )
	{
	    le_cup_ref_t *dest = DefineLECUP(&ld->cup_versus,cup_idx);
	    for ( int i = 0; i < BMG_RCUP_TRACKS; i++ )
	    {
		const int slot = ntohl(cup->track_id[i]);
		le_track_t *lt = DefineTrackLD(ld,slot,false);
		if ( lt && !IsArenaLTTY(lt->track_type) )
		{
		    lt->cup_slot = 10*cup_idx + i + 11;
		    if (dest)
			*dest++ = slot;
		}
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void ImportBmgLD ( le_distrib_t *ld, const bmg_t *bmg, const le_strpar_t *par )
{
    if ( !ld || !bmg )
	return;

    if (!par)
	par = &ld->spar;

    char buf[LE_TRACK_STRING_MAX+1];
    for ( int slot = 0; slot < ld->tlist_used; slot++ )
    {
	le_track_t *lt = GetTrackLD(ld,slot);
	if ( lt && GetBmgBySlotLEO(buf,sizeof(buf),bmg,slot,ld->spar.opt) )
	    SetTextLT(lt,par,buf);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////	    le_distrib_t: import files and options	///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ImportRawDataLD
(
    le_distrib_t	*ld,		// destination
    raw_data_t		*raw		// source
)
{
    enumError err = ERR_MISSING_PARAM;
    if ( ld && raw )
    {
	if (!ld->is_initialized)
	    InitializeLD(ld);

	if ( logging >= 2 && raw->fname )
	    fprintf(stdlog,"Import data to LD: %s\n",raw->fname);

	err = ERR_OK;
	switch(raw->fform)
	{
	 case FF_LEDEF:
	    err = ScanLeDefLD(ld,false,raw->data,raw->data_size,raw->fname,CPM_LINK);
	    break;

	 case FF_LEDIS:
	    ScanDumpLD(ld,false,raw->data,raw->data_size);
	    break;

	 case FF_LEREF:
	    ScanRefLD(ld,false,raw->data,raw->data_size);
	    break;

	 case FF_LESTR:
	    ScanStrLD(ld,false,raw->data,raw->data_size);
	    break;

	 case FF_SHA1REF:
	    ScanSha1LD(ld,false,raw->data,raw->data_size);
	    break;

	 case FF_LE_BIN:
	    {
		le_analyse_t ana;
		err = AnalyseLEBinary(&ana,raw->data,raw->data_size);
		if (!err)
		    ImportAnaLD(ld,&ana);
		ResetLEAnalyse(&ana);
	    }
	    break;

	 case FF_PREFIX:
	    DefinePrefixTable(raw->data,raw->data_size);
	    break;

	 case FF_LPAR:
	    {
		le_lpar_t lpar;
		InitializeLPAR(&lpar,true);
		err = ScanTextLPAR(&lpar,false,raw->fname,raw->data,raw->data_size);
		if (!err)
		    ImportLparLD(ld,&lpar);
		ResetLPAR(&lpar);
	    }
	    break;

	 case FF_BMG:
	 case FF_BMG_TXT:
	    {
		bmg_t bmg;
		err = ScanBMG(&bmg,true,raw->fname,raw->data,raw->data_size);
		if (!err)
		    ImportBmgLD(ld,&bmg,&ld->spar);
		ResetBMG(&bmg);
	    }
	    break;

	 case FF_DISTRIB:
	    err = ImportDistribLD(ld,raw->data,raw->data_size);
	    break;

	 case FF_U8:
	    err = AddToArchLD(ld,raw);
	    break;

	 case FF_BRRES:
	 case FF_TEX_CT:
	 case FF_CT1_DATA:
	 case FF_CTDEF:
	    if (!ld->ctcode)
		ld->ctcode = MALLOC(sizeof(*ld->ctcode));
	    InitializeCTCODE(ld->ctcode,CTM_LECODE2);
	    err = ScanRawDataCTCODE(ld->ctcode,CTM_NO_INIT,raw,0);
	    if (!err)
		ImportCtcodeLD(ld,ld->ctcode);
	    ResetCTCODE(ld->ctcode);
	    break;

	 case FF_UNKNOWN:
	    err = ERROR0(ERR_INVALID_DATA,
			"Need LE-CODE compatible file, but have unknown file format: %s\n",
			raw->fname);
	    break;

	 default:
	    err = ERROR0(ERR_INVALID_DATA,
			"Need LE-CODE compatible file, but file format is %s: %s",
			GetNameFF(raw->fform_file,raw->fform),raw->fname);
	}
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

struct search_t
{
    le_distrib_t	*ld;
    int			count;
    enumError		max_err;
};

//-----------------------------------------------------------------------------

static enumError search_func
(
    mem_t	path,		// full path of existing file, never NULL
    uint	st_mode,	// copy of struct stat.st_mode, see "man 2 stat"
    void	*param		// user defined parameter
)
{
    if (!S_ISREG(st_mode))
	return ERR_JOB_IGNORED;

    struct search_t *s = (struct search_t*)param;
    ASSERT(s);
    ASSERT(s->ld);

    const enumError err = ImportFileLD(s->ld,path.ptr,false,false);
    if ( s->max_err < err )
	 s->max_err = err;
    s->count++;
    return err;
}

//-----------------------------------------------------------------------------

enumError ImportFileLD
(
    le_distrib_t	*ld,			// destination
    ccp			fname,			// file name of source
    bool		use_wildcard,		// true & file not exist: search files
    bool		support_options		// support options (leading '+' and no slash )
)
{
    enumError err = ERR_MISSING_PARAM;
    if ( ld && fname )
    {
	if (!ld->is_initialized)
	    InitializeLD(ld);

	if ( support_options && *fname == '+' )
	    ld->spar.opt = ScanLEO(ld->spar.opt,fname+1,"Filter option");
	else
	{
	    struct stat st;
	    if ( use_wildcard && HaveWildcards(MemByString(fname)) && stat(fname,&st) )
	    {
		struct search_t param = { .ld = ld };
		SearchPaths(fname,0,false,search_func,&param);
		return param.count
		    ? param.max_err
		    : ERROR0(ERR_CANT_OPEN,"File not found: %s\n",fname);
	    }

	    raw_data_t raw;
	    err = LoadRawData(&raw,true,fname,0,opt_ignore>0,0);
	    if (!err)
		err = ImportRawDataLD(ld,&raw);
	    ResetRawData(&raw);
	}
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

void ImportOptionsLD ( le_distrib_t *ld )
{
    if (ld)
    {
	if (!ld->is_initialized)
	    InitializeLD(ld);

	ccp *ptr = le_define_list.field, *end;
	for ( end = ptr + le_define_list.used; ptr < end; ptr++ )
	    ImportFileLD(ld,*ptr,true,false);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: export			///////////////
///////////////////////////////////////////////////////////////////////////////

bool ExportAnaLD ( const le_distrib_t *ld, le_analyse_t *ana )
{
    if ( !ld && !ana )
	return false;


    //--- assign LPAR

    if (ld->lpar)
	ana->lpar = *ld->lpar;


    //--- slots and flags

    ana->n_slot = 0;
    int max = PackTracksLD(ld);

    le_property_t *prop  = ana->property;
    le_music_t    *music = ana->music;
    le_flags_t	  *flags = ana->flags;

    if ( prop && music && flags )
    {
	if ( max > ana->max_slot )
	     max = ana->max_slot;
	ana->n_slot = max;

	const le_track_t *lt = ld->tlist;
	for ( int slot = 0; slot < max; slot++, lt++ )
	{
	    if ( lt->track_status < LTS_EXPORT )
		continue;
	    prop[slot]  = lt->property;
	    music[slot] = lt->music;
	    flags[slot] = lt->flags;
	}
    }


    //--- copy battle cups

    ana->n_cup_arena = 0;

    le_cup_track_t *cup_dest = ana->cup_arena;
    if (cup_dest)
    {
	uint max = ld->cup_battle.used;
	if ( max > ana->max_cup_arena )
	     max = ana->max_cup_arena;

	for ( int cup_idx = 0; cup_idx < max; cup_idx++ )
	{
	    const le_cup_ref_t *ref = GetLECUP(&ld->cup_battle,cup_idx);
	    if (ref)
	    {
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		ana->n_cup_arena++;
	    }
	}
    }


    //--- copy racing cups

    ana->n_cup_track = 0;

    cup_dest = ana->cup_track;
    if (cup_dest)
    {
	uint max = ld->cup_versus.used;
	if ( max > ana->max_cup_track )
	     max = ana->max_cup_track;

	for ( int cup_idx = 0; cup_idx < max; cup_idx++ )
	{
	    const le_cup_ref_t *ref = GetLECUP(&ld->cup_versus,cup_idx);
	    if (ref)
	    {
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		*cup_dest++ = htonl(*ref++);
		ana->n_cup_track++;
	    }
	}
    }
    PRINT("%s() #%u: n_cups=%d+%d\n",__FUNCTION__,__LINE__,ana->n_cup_arena,ana->n_cup_track);


    //--- setup by version

    switch (ana->param_vers)
    {
     case 1:
	ana->cup_par->n_racing_cups = htonl(ana->n_cup_track);
	ana->cup_par->n_battle_cups = htonl(ana->n_cup_arena);
	ana->course_par->n_slot = htonl(ana->n_slot);
	break;
    }


    //--- finalize

    PatchLECODE(ana);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool ExportCtCodeLD ( const le_distrib_t *ld, ctcode_t *ctc )
{
    // [[2do]] ???
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: reference list		///////////////
///////////////////////////////////////////////////////////////////////////////

static void PrintSectionTool ( FILE *f, bool be_verbose )
{
    DASSERT(f);

    if (be_verbose)
	fprintf(f,text_ledis_tool_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );
    else
	fprintf(f,
		"\r\n[TOOL]\r\n"
		"TOOL     = %s\r\n"
		"SYSTEM   = %s\r\n"
		"VERSION  = %s\r\n"
		"REVISION = %u\r\n"
		"DATE     = %s\r\n"
		"\r\n",
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );
}

///////////////////////////////////////////////////////////////////////////////

int SetupReferenceList ( le_distrib_t *ld, FilterFuncLD filter, le_options_t opt )
{
    const uint need = ld->tlist_used + 1;
    if ( ld->reflist_size < need )
    {
	FREE(ld->reflist);
	ld->reflist = CALLOC(need,sizeof(*ld->reflist));
	ld->reflist_size = need;
    }

    le_track_t *lt = ld->tlist, **dest = ld->reflist;
    for ( int slot = 0; slot < ld->tlist_used; slot++, lt++ )
	if ( filter ? filter(lt,opt) : lt->track_status >= LTS_EXPORT )
	    *dest++ = lt;
    *dest = 0;

    ld->reflist_used = dest - ld->reflist;
    ASSERT( ld->reflist_used < ld->reflist_size );
    return ld->reflist_used ;
}

///////////////////////////////////////////////////////////////////////////////

static bool check_filter ( const le_track_t *lt, le_options_t opt )
{
    const le_options_t a = opt & LEO_M_ARENA;
    if ( a == LEO_VERSUS && !IsTrackLTTY(lt->track_type) || a == LEO_BATTLE && !IsArenaLTTY(lt->track_type) )
	return false;

    const le_options_t o = opt & LEO_M_ORIGINAL;
    if ( o == LEO_CUSTOM && lt->is_original || o == LEO_ORIGINAL && !lt->is_original )
	return false;

    return true;
}

//-----------------------------------------------------------------------------

static bool filter_options_td ( const le_track_t *lt, le_options_t opt )
{
    return lt->track_status >= LTS_EXPORT
	&& check_filter(lt,opt);
}

//-----------------------------------------------------------------------------

static bool filter_name_td ( const le_track_t *lt, le_options_t opt )
{
    return lt->track_status >= LTS_EXPORT
	&& check_filter(lt,opt)
	&& IsOutputNameValidLEO(lt->name,opt);
}

//-----------------------------------------------------------------------------

static bool filter_xname2_td ( const le_track_t *lt, le_options_t opt )
{
    return lt->track_status >= LTS_EXPORT
	&& check_filter(lt,opt)
	&& ( IsOutputNameValidLEO(lt->name,opt) || IsOutputNameValidLEO(lt->xname,opt) );
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_cup_lt ( const le_track_t **pa, const le_track_t **pb )
{
    const le_track_t *a = *pa;
    const le_track_t *b = *pb;
    ASSERT(a&&b);

    int res = a->track_type - b->track_type;
    if (!res)
    {
	res = a->cup_slot - b->cup_slot;
	if (!res)
	    res = a->track_slot - b->track_slot;
    }
    return res;
}

//-----------------------------------------------------------------------------

int SortByCupLD ( le_distrib_t *ld, FilterFuncLD filter, le_options_t opt )
{
    DASSERT(ld);

    UpdateCupsLD(ld);
    int n = SetupReferenceList(ld,filter,opt);
    if ( n > 1 )
	qsort( ld->reflist, n, sizeof(*ld->reflist), (qsort_func)cmp_cup_lt );
    return n;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    le_distrib_t: listings		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateDumpLD ( FILE *f, le_distrib_t *ld )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateCupsLD(ld);

    export_count++;
    brief_count++;

    fprintf(f,
	"%s : LE-CODE distribution\r\n"
	"%s[TOOL]\r\n\r\n"
	"TOOL     = %s\r\n"
	"SYSTEM   = %s\r\n"
	"VERSION  = %s\r\n"
	"REVISION = %u\r\n"
	"DATE     = %s\r\n"
	"\r\n",
	LE_DISTRIB_MAGIC8,
	section_sep,
	tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );

 #if 0
    fprintf(f,
	"%s[SETTINGS]\r\n\r\n"
	"\r\n",
	section_sep
	);
 #endif


    //--- distribution params

    if (ld->dis_param.used)
    {
	int count = 0;
	ParamFieldItem_t *it = ld->dis_param.field, *end;
	for ( end = it + ld->dis_param.used; it < end; it++ )
	{
	    ccp data = (ccp)it->data;
	    if ( data && *data )
	    {
		if (!count++)
		    fprintf(f,"%s[DISTRIBUTION-PARAM]\r\n\r\n",section_sep);

		int len = strlen(it->key);
		ccp sep = len > 14 ? " " : len > 6 ? "\t" : "\t\t";
		fprintf(f,"@%s%s= %s\r\n", it->key, sep, data );
	    }
	}
    }


    //-- lecode info

    fprintf(f,"%s[LECODE-INFO]\r\n# Info about current lecode binaries:\r\n\r\n",
			section_sep );
    CreateLeInfoLD(f,ld,true);


    //-- LPAR

    if (ld->lpar)
	CreateLparLD(f,ld,false);


    //-- cup reference

    fprintf(f,"%s[CUP-REF]\r\n\r\n",section_sep);

    if (PrintCupRefLD(f,ld,&ld->cup_battle))
	    fputs("\r\n",f);

    if (PrintCupRefLD(f,ld,&ld->cup_versus))
	fputs("\r\n",f);


    //-- track reference

    fprintf(f,"%s[TRACK-REF]\r\n\r\n",section_sep);
    const enumError err1 = CreateRefLD(f,ld,false);


    //-- track strings

    fprintf(f,"%s[TRACK-STRINGS]\r\n\r\n",section_sep);
    const enumError err2 = CreateStringsLD(f,ld);


    //-- end

    fprintf(f,
	"%s[END]\r\n"
	"# This section is ignored.\r\n"
	"\r\n",
	section_sep );

    export_count--;
    brief_count--;
    return err1 > err2 ? err1 : err2;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateRefLD ( FILE *f, le_distrib_t *ld, bool add_strings )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateCupsLD(ld);
    SetupReferenceList(ld,filter_options_td,ld->spar.opt);

    fprintf(f,"%s : LE-CODE track reference v1\r\n",LE_REFERENCE_MAGIC8);

    ccp filter = GetOutputFilterLEO(ld->spar.opt&LEO_M_OUTPUT);
    if (filter)
	fprintf(f,"# Active output filter: %s\r\n",filter);

    fprintf(f,
	"#> track_slot|type|orig|cup_slot|cup_slot_name|property|music|flags|hidden|sha1|sha1_d|%s\r\n",
	add_strings ? "ident|ident_d|file|file_d|name|name_d|xname|xname_d|" : "" );

    const bool no_d = ( ld->spar.opt & LEO_NO_D_FILES ) != 0;
    int count = 0;

    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	count++;
	const le_track_t *lt = *plt;

	fprintf(f,"%u|%s|%u|%u|%s|%u|%u|%u|%u|%s|%s|",
		lt->track_slot, GetNameLTTY(lt->track_type),
		lt->is_original,
		lt->cup_slot, GetCupLT(lt,""),
		lt->property, lt->music,
		lt->flags, IsHiddenLETF(lt->flags),
		GetSha1LT(lt,0,""),
		no_d ? 0 : GetSha1LT(lt,1,"") );

	if (add_strings)
	{
	    exmem_t id1	= EscapeStringPipeCircS(GetIdentLT(lt,0,false,0));
	    exmem_t id2	= EscapeStringPipeCircS( no_d ? 0 : GetIdentLT(lt,1,false,0) );
	    exmem_t file	= EscapeStringPipeCircS(GetFileLT(lt,0));
	    exmem_t name	= EscapeStringPipeCircS(GetNameLT(lt,0));
	    exmem_t xname	= EscapeStringPipeCircS(GetXNameLT(lt,0));

	    fprintf(f,"%s|%s|%s||%s||%s||\r\n",
		id1.data.ptr,
		id2.data.ptr,
		file.data.ptr,
		name.data.ptr,
		xname.data.ptr );

	    FreeExMem(&id1);
	    FreeExMem(&id2);
	    FreeExMem(&file);
	    FreeExMem(&name);
	    FreeExMem(&xname);
	}
	else
	    fputs("\r\n",f);
    }

    if ( count > 1 )
	fprintf(f,"#-----\r\n# %u records total\r\n",count);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateStringsLD ( FILE *f, le_distrib_t *ld )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    SetupReferenceList(ld,filter_options_td,ld->spar.opt);
    fprintf(f,"%s : LE-CODE track strings v1\r\n",LE_STRINGS_MAGIC8);

    ccp filter = GetOutputFilterLEO(ld->spar.opt&LEO_M_OUTPUT);
    if (filter)
	fprintf(f,"# Active output filter: %s\r\n",filter);

    fputs("#> track_slot|d_mode|is_set|name|string|\r\n",f);

    const bool allow_d = !( ld->spar.opt & LEO_NO_D_FILES );
    int n_strings = 0, n_tracks = 0;

    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	n_tracks++;
	const le_track_t *lt = *plt;

	for ( int ltt = 0; ltt < LTT__N; ltt++ )
	{
	    ccp *ptr = GetPtrOptLT(lt,ltt,allow_d);
	    if ( ptr && *ptr )
	    {
		n_strings++;
		exmem_t text = EscapeStringPipeCircS(*ptr);
		fprintf(f,"%u|%u|0|%s|%s|\r\n",
			lt->track_slot,
			GetDModeLTT(ltt),
			GetLowerNameLTT(ltt),
			text.data.ptr );
		FreeExMem(&text);
	    }
	}

     #if LE_STRING_SET_ENABLED
	ParamFieldItem_t *end = lt->str_set.field + lt->str_set.used;
	for ( ParamFieldItem_t *ptr = lt->str_set.field; ptr < end; ptr++ )
	{
	    n_strings++;
	    exmem_t name = EscapeStringPipeCircS(ptr->key);
	    exmem_t text = EscapeStringPipeCircS(ptr->data);
	    fprintf(f,"%u|3|1|%s|%s|\r\n",
		    lt->track_slot,
		    name.data.ptr,
		    text.data.ptr );
	    FreeExMem(&name);
	    FreeExMem(&text);
	}
     #endif
    }

    if ( n_tracks > 1 )
	fprintf(f,"#-----\r\n# %u strings for %u tracks total\r\n",n_strings,n_tracks);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static u32 GetPrefixIdByName ( ccp name, bool is_arena )
{
    intx_t res;
    res.u32[0] = 0;

    if ( name && *name == '+' )
	for ( int i = 0; i < 4 && (u8)*name > ' ' ; i++ )
	    res.s8[i] = *name++;
    return is_arena ? ~res.u32[0] : res.u32[0];
}

//-----------------------------------------------------------------------------

uint GetNameFw ( le_distrib_t *ld, bool use_xname )
{
    uint fw = 4;
    if (ld->reflist)
    {
	for ( le_track_t **plt = ld->reflist; *plt; plt++ )
	{
	    le_track_t *lt = *plt;
	    ccp name = use_xname ? GetXName2LT(lt,"") : lt->name;
	    uint nlen = strlen8(name) + GetEscapeLen(name,-1,CHMD__MODERN,'"');
	    if ( fw < nlen )
		 fw = nlen;
	}
    }
    return fw;
}

//-----------------------------------------------------------------------------

enumError CreateNamesLD ( FILE *f, le_distrib_t *ld, bool use_xname  )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    SortByCupLD( ld, use_xname ? filter_xname2_td : filter_name_td, ld->spar.opt );

    uint fw = GetNameFw(ld,use_xname) + 11;
    fprintf(f,"\r\n%.*s\r\n   Cup    Name\r\n%.*s\r\n",
		fw, Minus300, fw, Minus300 );

    u32 prev_id = 0;
    int count = 0, prev_cup = -1;
    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	le_track_t *lt = *plt;

	ccp name = use_xname ? GetXName2LT(lt,"") : lt->name;
	u32 cur_id = GetPrefixIdByName(name,IsArenaLTTY(lt->track_type));
	if (!count++)
	    prev_id = cur_id;

	int cur_cup = lt->cup_slot / 10;

	if ( prev_id != cur_id )
	{
	    prev_id  = cur_id;
	    prev_cup = cur_cup;
	    fprintf(f,"\r\n%.*s\r\n\r\n",fw,Minus300);
	}
	else if ( prev_cup != cur_cup )
	{
	    prev_cup = cur_cup;
	    fputs("\r\n",f);
	}

	exmem_t esc = EscapeStringEx(name,-1,EmptyString,EmptyString,CHMD__MODERN,0,false);
	fprintf(f,"%s  %s\r\n", GetCupAlignLT(lt), esc.data.ptr);
	FreeExMem(&esc);
    }

    if ( count > 1 )
	fprintf(f,"\r\n%.*s\r\n\r\n",fw,Minus300);
    else
	fputs("\r\n",f);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateInfoLD ( FILE *f, le_distrib_t *ld, bool use_xname )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    DumpCupSlots(ld,"before sort");
    SortByCupLD( ld, use_xname ? filter_xname2_td : filter_name_td, ld->spar.opt );
    DumpCupSlots(ld,"behind sort");

    uint fw = GetNameFw(ld,use_xname) + 33;
    fprintf(f,"\r\n%.*s\r\n()\t   Cup   Prop\tMusic\tName\r\n%.*s\r\n",
		fw, Minus300, fw, Minus300 );

    u32 prev_id = 0;
    int count = 0, prev_cup = -1;
    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	le_track_t *lt = *plt;

	ccp name = use_xname ? GetXName2LT(lt,"") : lt->name;
	u32 cur_id = GetPrefixIdByName(name,IsArenaLTTY(lt->track_type));
	if (!count++)
	    prev_id = cur_id;

	int cur_cup = lt->cup_slot / 10;

	if ( prev_id != cur_id )
	{
	    prev_id  = cur_id;
	    prev_cup = cur_cup;
	    fprintf(f,"\r\n%.*s\r\n\r\n",fw,Minus300);
	}
	else if ( prev_cup != cur_cup )
	{
	    prev_cup = cur_cup;
	    fputs("\r\n",f);
	}

	exmem_t esc = EscapeStringEx(name,-1,EmptyString,EmptyString,CHMD__MODERN,0,false);
	fprintf(f,"()\t%s %s\t%s\t%s\r\n",
		GetCupAlignLT(lt),
		PrintPropertyID(lt->property,false),
		PrintMusicID(lt->music,false),
		esc.data.ptr );
	FreeExMem(&esc);
    }

    if ( count > 1 )
	fprintf(f,"\r\n%.*s\r\n\r\n",fw,Minus300);
    else
	fputs("\r\n",f);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateSha1LD
	( FILE *f, le_distrib_t *ld, bool use_xname, bool add_comments )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateCupsLD(ld);
    SetupReferenceList(ld,filter_options_td,ld->spar.opt);

    static char sep[] = "#----------\r\n";

    if (add_comments)
    {
	fprintf(f,
	    "%s\r\n"
	    "# A SHA1 list, that can be used for example @ Wiimmfi.de\r\n"
	    ,LE_SHA1REF_MAGIC8 );

	ccp filter = GetOutputFilterLEO(ld->spar.opt&LEO_M_OUTPUT);
	if ( filter && *filter )
	    fprintf(f,"# Active output filter: %s\r\n",filter);

	fputs(	"# Columns: TYPE FLAGS SHA1 TRACK_SLOT CUP NAME\r\n"
		"# During the import, only the first 4 columns are evaluated.\r\n"
		"# Flags: d:is _d file / t:title only / h:hidden / o:original track\r\n"
		,f );
    }

    int count = 0, prev_type = 99;
    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	const le_track_t *lt = *plt;

	if ( prev_type != lt->track_type )
	{
	    prev_type = lt->track_type;
	    fputs( add_comments ? sep : "\r\n", f );
	}

	const int max = ld->spar.opt & LEO_NO_D_FILES ? 0 : 1;
	for ( int d = 0; d <= max; d++ )
	{
	    const le_track_id_t *lti = lt->lti + d;
	    if ( lti->have_sha1 )
	    {
		count++;
		sha1_hex_t hex;
		Sha1Bin2Hex(hex,lti->sha1bin);

		ccp name = use_xname ? GetXName2LT(lt,0) : GetNameLT(lt,0);
		exmem_t esc = EscapeStringEx(name,-1,EmptyString,EmptyString,CHMD__MODERN,0,false);

		fprintf(f,"%s %c%c%c%c %s %4d %s  %s\r\n",
		    GetNameLTTY(lt->track_type),
		    d ? 'd' : '-',
		    IsTitleLETF(lt->flags) ? 't' : '-',
		    IsHiddenLETF(lt->flags) ? 'h' : '-',
		    lti->orig_sha1 ? 'o' : '-',
		    hex, lt->track_slot, GetCupAlignLT(lt),
		    esc.data.ptr );
		FreeExMem(&esc);
	    }
	}
    }

    if ( count > 3 && add_comments )
	fprintf(f,"%s# %u records total\r\n",sep,count);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateCtDefLD ( FILE *f, le_distrib_t *ld )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateCupsLD(ld);
    const bool have_wiimm_cup = HaveWiimmCupLD(ld);

    fprintf(f,text_ledis_ctdef_header_cr,CT_DEF_MAGIC8,have_wiimm_cup);

    int prev_cup = 0, tidx = (8+have_wiimm_cup)*4;
    for ( int slot = BMG_LE_FIRST_CTRACK; slot < ld->tlist_used; slot++ )
    {
	le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT )
	    continue;
	char type;
	if (IsHiddenLETF(lt->flags))
	    type = 'H';
	else
	{
	    type = 'T';
	    const int cur_cup = tidx++/4;
	    if ( prev_cup != cur_cup )
	    {
		prev_cup = cur_cup;
		fprintf(f,"\r\nC \"Cup %u\"\r\n",cur_cup);
	    }
	}

	fprintf(f,"%c %s\t; %s\t; 0x%02x\t; %s; %s; %s; %s\r\n",
		type,
		PrintMusicID(lt->music,false),
		PrintPropertyID(lt->property,false),
		lt->flags,
		QuoteFileLT(lt,""),
		QuoteNameLT(lt,""),
		QuoteIdentLT(lt,false,true,""),
		QuoteXNameLT(lt,"") );
    }

    fputs("\r\n",f);

    extern const char text_ctcode_setup_arena_cr[];
    int count = 0;
    for ( int slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
    {
	le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT || HaveUsualBattleSlots(lt) <= 0 )
	    continue;

	if (!count++)
	    fprintf(f,"\r\n%s",text_ctcode_setup_arena_cr);

	fprintf(f,"%s\t%s\t%s\t# A%u\r\n",
		PrintPropertyID(lt->track_slot,false),
		PrintPropertyID(lt->property,false),
		PrintMusicID(lt->music,false),
		lt->cup_slot );
    }
    if (count)
	fputs("\r\n",f);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static void print_ledef_text
	( FILE *f, const le_distrib_t *ld, int max_len, ccp name, ccp text )
{
    DASSERT(f);
    DASSERT(ld);
    DASSERT(name);

    if (IsOutputNameValidLEO(text,ld->spar.opt))
    {
	ccp esc = EscapeStringS(text,"",CHMD__MODERN);
	const int n_tabs = ( max_len - strlen8(name) ) / 8;
	if ( n_tabs > 0 )
	    fprintf(f,"\t%s%.*s\"%s\"\r\n",name,n_tabs,Tabs20,esc);
	else
	    fprintf(f,"\t%s \"%s\"\r\n",name,esc);
	FreeString(esc);
    }
}

//-----------------------------------------------------------------------------

static void print_ledef_track
	( FILE *f, const le_distrib_t *ld, const le_track_t *lt )
{
    DASSERT(f);
    DASSERT(ld);
    DASSERT(lt);

    char comment[30], *dest = comment;
    if ( lt->lti[0].orig_sha1 )
	dest = StringCopyE(dest,comment+sizeof(comment),", original");
    if ( IsHiddenLETF(lt->flags) )
	dest = StringCopyE(dest,comment+sizeof(comment),", hidden");
    *dest = 0;

    ccp type = GetNameLTTY(lt->track_type);
    fprintf(f,"\r\nTRACK %s%u\t%s\t%s\t%s\t\"%s\"%s%s\r\n",
		type,
		lt->track_slot,
		type,
		PrintPropertyID(lt->property,false),
		PrintMusicID(lt->music,false),
		PrintLEFT(lt->flags),
		*comment ? "\t#" : "",
		*comment ? comment+1 : "" );

 #if LE_STRING_SET_ENABLED
    int max_len = 0;
    ParamFieldItem_t *set_end = lt->str_set.field + lt->str_set.used;
    if (lt->str_set.used)
    {
	for ( ParamFieldItem_t *ptr = lt->str_set.field; ptr < set_end; ptr++ )
	    if (IsOutputNameValidLEO(ptr->data,ld->spar.opt))
	    {
		const int slen = strlen8(ptr->key);
		if ( max_len < slen )
		     max_len = slen;
	    }
    }
    max_len = ( max_len + 18 ) / 8 * 8 - 1;
 #else
    int max_len = 7;
 #endif

    print_ledef_text(f,ld,max_len,"ident",GetIdentLT(lt,false,true,0));
    print_ledef_text(f,ld,max_len,"file",GetFileLT(lt,0));
    print_ledef_text(f,ld,max_len,"name",GetNameLT(lt,0));
    print_ledef_text(f,ld,max_len,"xname",GetXNameLT(lt,0));

 #if LE_STRING_LIST_ENABLED
    for ( int ltt = LTT__LIST_BEG; ltt < LTT__LIST_END; ltt++ )
    {
	ccp *ptr = GetPtrOptLT(lt,ltt,true);
	if ( ptr && *ptr )
	    print_ledef_text(f,ld,max_len,GetLowerNameLTT(ltt),GetListLT(lt,ltt,0));
    }
 #endif

 #if LE_STRING_SET_ENABLED
    if (lt->str_set.used)
    {
	max_len -= 2;
	for ( ParamFieldItem_t *ptr = lt->str_set.field; ptr < set_end; ptr++ )
	    if (IsOutputNameValidLEO(ptr->data,ld->spar.opt))
	    {
		ccp esc = EscapeStringS(ptr->data,"",CHMD__MODERN);
		const int n_tabs = ( max_len - strlen8(ptr->key) ) / 8;
		if ( n_tabs > 0 )
		    fprintf(f,"\t[%s]%.*s\"%s\"\r\n",ptr->key,n_tabs,Tabs20,esc);
		else
		    fprintf(f,"\t[%s] \"%s\"\r\n",ptr->key,esc);
		FreeString(esc);
	    }
    }
 #endif

}

//-----------------------------------------------------------------------------

static void print_ledef_cup
	( FILE *f, le_distrib_t *ld, const le_cup_t *lc, int sep_index )
{
    DASSERT(f);
    DASSERT(cup);
    DASSERT(title);

    if (!lc->used)
	return;


    //--- get field widths

    int max = LE_FIRST_CT_SLOT;
    for ( int i = 0; i < lc->used; i++ )
    {
	le_cup_ref_t *ref = GetLECUP(lc,i);
	if (ref)
	    for ( int t = 0; t < lc->tracks; t++ )
		if ( max < ref[t] )
		     max = ref[t];
    }

    char buf[20];
    const int fw_cup	= snprintf(buf,sizeof(buf),"%d",lc->used-1);
    const int fw_track	= snprintf(buf,sizeof(buf),"%d",max) + 2;


    //--- print cups

    ccp sep = "";
    int count = 0, next_sep = -1;
    const int cup_end = lc->used;
    for ( int i = 0; i < cup_end; i++ )
    {
	le_cup_ref_t *ref = GetLECUP(lc,i);
	if ( i+2 <= cup_end && !memcmp(standard_bt_ref,ref,sizeof(standard_bt_ref)) )
	{
	    fputs(sep,f);
	    fputs("# 2 battle cups with standard layout (STANDARD-BATTLE-CUPS):\r\n",f);
	    next_sep = count+2;
	}
	else if ( i+8 <= cup_end && !memcmp(standard_vs_ref,ref,sizeof(standard_vs_ref)) )
	{
	    fputs(sep,f);
	    fputs("# 8 versus cups with standard layout (STANDARD-VERSUS-CUPS):\r\n",f);
	    next_sep = count+8;
	}
	else if ( !memcmp(mkwfun_random_ref,ref,sizeof(mkwfun_random_ref)) )
	{
	    fputs(sep,f);
	    fputs("# 1 versus cup with standard MKW-Fun random slots (MKWFUN-RANDOM-CUP):\r\n",f);
	    next_sep = count+1;
	}

	if (ref)
	{
	    if ( count == next_sep || count >= next_sep && count == sep_index )
		fputs(sep,f);
	    count++;
	    sep = "\r\n";

	    fputs("APPEND",f);
	    for ( int t = 0; t < lc->tracks; t++ )
	    {
		const int slot = ref[t];
		ccp info = "-1";
		if ( slot >= 0 )
		{
		    const le_track_t *lt = GetTrackLD(ld,slot);
		    if ( IsActiveLT(lt) )
		    {
			if ( lt->track_status == LTS_FILL )
			    info = "fill";
			else
			    info = GetSlotNameLT(lt);
		    }
		}
		fprintf(f,"\t%*s",fw_track,info);
	    }
	    fprintf(f,"\t# %*d\r\n",fw_cup,i);
	}
    }
    fputs(sep,f);
}

//-----------------------------------------------------------------------------

enumError CreateLeDefLD ( FILE *f, le_distrib_t *ld )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateCupsLD(ld);
    AnalyzeLD(ld,true);
    const bool be_verbose = !(ld->spar.opt & LEO_BRIEF);

    if (be_verbose)
	fprintf(f,text_ledis_ledef_header_cr,LE_DEFINE_MAGIC8);
    else
	fprintf(f,"%s\r\n",LE_DEFINE_MAGIC8);

    PrintSectionTool(f,be_verbose);

    //-- TRACK-LIST

    if (be_verbose)
	fprintf(f,text_ledis_ledef_track_list_cr);
    else
	fprintf(f,"\r\n[TRACK-LIST]\r\n");

    // battle arenas

    ccp info = "\r\n#--- battle arenas\r\n";
    int first = MKW_ARENA_BEG;
    if (ld->ana.bt.orig_tracks)
    {
	first = MKW_ARENA_END;
	fprintf(f,"%s\r\nSTANDARD-BATTLE-ARENAS\r\n",info);
	info = 0;
    }

    for ( int slot = first; slot < ld->tlist_used; slot++ )
    {
	const le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT || !IsArenaLTTY(lt->track_type) )
	    continue;

	if (info)
	{
	    fputs(info,f);
	    info = 0;
	}

	print_ledef_track(f,ld,lt);
    }

    // versus tracks

    info = "\r\n#--- versus tracks\r\n";
    first = MKW_TRACK_BEG;
    if (ld->ana.vs.orig_tracks)
    {
	first = MKW_TRACK_END;
	fprintf(f,"%s\r\nSTANDARD-VERSUS-TRACKS\r\n",info);
	info = 0;
    }

    for ( int slot = first; slot < ld->tlist_used; slot++ )
    {
	const le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT || IsArenaLTTY(lt->track_type) )
	    continue;

	if (info)
	{
	    fputs(info,f);
	    info = 0;
	}

	print_ledef_track(f,ld,lt);
    }

    fputs("\r\n",f);


    //-- CUP-LIST

    if (be_verbose)
	fprintf(f,text_ledis_ledef_cup_list_cr);
    else
	fprintf(f,"\n\r\n[CUP-LIST]\r\n\r\n");

    // battle cups

    fputs("#--- battle cups\r\n\r\n",f);
    print_ledef_cup(f,ld,&ld->cup_battle,2);

    fprintf(f,
	"# Append unused battle arenas automatically (0=NO/1=YES):\r\n"
	"ADD-UNUSED-ARENAS	= %s\r\n"
	"\r\n"
	,ld->ana.bt.add_unused ? "yes" : "no"
	);

    // versus cups

    fputs("#--- versus cups\r\n\r\n",f);
    print_ledef_cup(f,ld,&ld->cup_versus,8);

    fprintf(f,
	"# Append unused versus tracks automatically (0=NO/1=YES):\r\n"
	"ADD-UNUSED-TRACKS	= %s\r\n"
	"\r\n"
	,ld->ana.vs.add_unused ? "yes" : "no"
	);


    //-- END

    if (be_verbose)
	fputs("\r\n#\f\r\n###########\r\n   [END]\r\n###########\r\n",f);
    else
	fputs("\r\n[END]\r\n",f);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateLecode4LD ( ccp fname, le_distrib_t *ld )
{
    if ( !fname || !ld )
	return ERR_MISSING_PARAM;

    ccp at = strrchr(fname,'@');
    if (!at)
	return ERROR0(ERR_WARNING,"Missing '@' in filename: %s\n",fname);

    char path[PATH_MAX];
    if ( strlen(fname) > sizeof(path) - 4 )
	return ERROR0(ERR_WARNING,"Filename too long: %s\n",fname);

    char *tag = path + (at-fname);
    memcpy(path,fname,tag-path);
    strncpy(tag+3,at+1,path+sizeof(path)-tag-3);

    for ( le_region_t reg = LEREG__BEG; reg <= LEREG__END; reg++ )
    {
	memcpy(tag,GetNameLEREG(reg,"@@@"),3);
	File_t F;
	enumError err = CreateFileOpt(&F,true,path,false,0);
	if (!err)
	    err = CreateLecodeLD(F.f,ld,reg);
	if (err)
	    return err;
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateLecodeLD ( FILE *f, le_distrib_t *ld, le_region_t region )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateCupsLD(ld);

    mem_t lecode = GetLecodeLD(ld,region);
    if (!lecode.ptr)
	return ERR_MISSING_PARAM;

    le_analyse_t ana;
    u8 *data = MEMDUP(lecode.ptr,lecode.len);
    if (!AnalyseLEBinary(&ana,data,lecode.len))
    {
	ExportAnaLD(ld,&ana);
	fwrite(data,lecode.len,1,f);
    }
    FREE(data);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateLeInfoLD ( FILE *f, const le_distrib_t *ld, bool list_only )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    if (!list_only)
    {
	const ColorSet_t *colset = GetFileColorSet(f);
	fprintf(f,"\r\n%sCurrent LE-CODE binaries:%s\r\n\r\n",colset->caption,colset->reset);
    }

    for ( le_region_t reg = LEREG__BEG; reg <= LEREG__END; reg++ )
    {
	mem_t builtin = GetLecodeLEREG(reg);
	mem_t current = GetLecodeLD(ld,reg);
	if ( current.ptr )
	    fprintf(f,"  %s, %s\r\n",
		GetInfoLECODE(current),
		current.ptr == builtin.ptr ? "built-in" : "loaded" );
    }
    fputs("\r\n",f);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateLparLD ( FILE *f, le_distrib_t *ld, bool print_full )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    SetupLparLD(ld,true);
    return SaveTextFileLPAR(ld->lpar,f,print_full);
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateBmgLD ( FILE *f, le_distrib_t *ld, bool bmg_text )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    bmg_t bmg;
    InitializeBMG(&bmg);

    le_options_t source = ld->spar.opt & LEO_M_BMG;
    if (!source)
	source = LEO_M_BMG;

    for ( int slot = 0; slot < ld->tlist_used; slot++ )
    {
	le_track_t *lt = GetTrackLD(ld,slot);
	if ( !lt || lt->track_status < LTS_EXPORT )
	    continue;

	ccp text = GetTextLT(lt,&ld->spar,0);
	if (!IsOutputNameValidLEO(text,ld->spar.opt))
	    continue;

	PRINT0("SLOT %d: %s\n",slot,text);

	if ( source & LEO_MKW1 )
	{
	    const u32 mid = IsMkwTrack(slot)
			  ? slot+MID_TRACK1_BEG
			  : IsMkwArena(slot)
			  ? slot-BMG_N_TRACK+MID_ARENA1_BEG
			  : 0;

	    if (mid)
	    {
		bmg_item_t *bi = InsertItemBMG(&bmg,mid,0,0,0);
		ASSERT(bi);
		AssignItemTextBMG(bi,text,-1);
	    }
	}

	if ( source & LEO_MKW2 )
	{
	    const u32 mid = IsMkwTrack(slot)
			  ? slot+MID_TRACK2_BEG
			  : IsMkwArena(slot)
			  ? slot-BMG_N_TRACK+MID_ARENA2_BEG
			  : 0;

	    if (mid)
	    {
		bmg_item_t *bi = InsertItemBMG(&bmg,mid,0,0,0);
		ASSERT(bi);
		AssignItemTextBMG(bi,text,-1);
	    }
	}

	if ( source & LEO_CTCODE && slot < BMG_N_CT_TRACK )
	{
	    bmg_item_t *bi = InsertItemBMG(&bmg,slot+MID_CT_TRACK_BEG,0,0,0);
	    ASSERT(bi);
	    AssignItemTextBMG(bi,text,-1);
	}

	if ( source & LEO_LECODE )
	{
	    bmg_item_t *bi = InsertItemBMG(&bmg,slot+MID_LE_TRACK_BEG,0,0,0);
	    ASSERT(bi);
	    AssignItemTextBMG(bi,text,-1);
	}
    }

    enumError err = bmg_text || isatty(fileno(f))
	? SaveTextFileBMG( &bmg, f, 0, long_count > 0,
				brief_count > 0 ? brief_count : !print_header )
	: SaveRawFileBMG(&bmg,f,0);

    ResetBMG(&bmg);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static void debug_cups ( FILE *f, const le_cup_t *lc )
{
    fprintf(f,"%6u (0x%x) cups used, %u (0x%x) alloced (%s)\r\n",
	lc->used, lc->used, lc->size, lc->size,
	PrintSize1000(0,0,lc->size*sizeof(*lc->list)*lc->tracks,0) );

    if (!lc->used)
	return;

    u16 count[LE_MAX_TRACKS] = {0};
    int slots_used = 0, different = 0, max_use = 0;

    const le_cup_ref_t *end  = lc->list + lc->used * lc->tracks;
    for ( const le_cup_ref_t *ptr = lc->list; ptr < end; ptr++ )
    {
	uint idx = *ptr;
	if ( idx < LE_MAX_TRACKS )
	{
	    slots_used++;
	    if (!count[idx]++)
		different++;
	    if ( max_use < count[idx] )
		 max_use = count[idx];
	}
    }

    fprintf(f,"%6u slots total, %u slots used.\r\n",
		lc->used*lc->tracks, slots_used );
    fprintf(f,"%6u different track references, max %u for one track.\r\n",
		different, max_use );
}

//-----------------------------------------------------------------------------

enumError CreateDebugLD ( FILE *f, const le_distrib_t *ld )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    const ColorSet_t *colset = GetFileColorSet(f);

    //--- basics

    fprintf(f,"\r\n%sBasic settings:%s\r\n",colset->caption,colset->reset);
    fprintf(f,
	"  Current auto-setup: 0x%x\r\n"
	"  Current options: %s\r\n"
	,ld->auto_setup
	,GetOptionsLSP(&ld->spar,LEO__ALL)
	);


    //--- tracks

    fprintf(f,"\r\n%sTracks:%s\r\n",colset->caption,colset->reset);
    fprintf(f,"  %u (0x%x) tracks used, %u (0x%x) alloced (%s)\r\n",
	ld->tlist_used, ld->tlist_used, ld->tlist_size, ld->tlist_size,
	PrintSize1000(0,0,ld->tlist_size*sizeof(*ld->tlist),0) );

    uint n_status[LTS__N] = {0};
    uint n_arena = 0, n_track = 0, n_random = 0;
    uint n_text[LTT__N] = {0}, size_text[LTT__N] = {0};
    uint n_sha1 = 0, n_sha1_d = 0;

 #if LE_STRING_SET_ENABLED
    typedef struct count_t { uint n; uint size; } count_t;
    exmem_list_t str_count;
    InitializeEML(&str_count);
 #endif

    le_track_t *lt = ld->tlist;
    for ( int slot = 0; slot < ld->tlist_used; slot++, lt++ )
    {
	if ( lt->track_status < LTS__N )
	    n_status[lt->track_status]++;

	if ( lt->track_status < LTS_ACTIVE )
	    continue;

	if ( lt->track_type & LTTY_ARENA  ) n_arena++;
	if ( lt->track_type & LTTY_TRACK  ) n_track++;
	if ( lt->track_type & LTTY_RANDOM ) n_random++;

	if (lt->lti[0].have_sha1) n_sha1++;
	if (lt->lti[1].have_sha1) n_sha1_d++;

	for ( int ltt = 0; ltt < LTT__N; ltt++ )
	{
	    ccp *ptr = GetPtrOptLT(lt,ltt,true);
	    if ( ptr && *ptr )
	    {
		n_text[ltt]++;
		size_text[ltt] += GetGoodAllocSize(strlen(*ptr) + 1);
	    }
	}

     #if LE_STRING_SET_ENABLED
	ParamFieldItem_t *end = lt->str_set.field + lt->str_set.used;
	for ( ParamFieldItem_t *ptr = lt->str_set.field; ptr < end; ptr++ )
	{
	    exmem_key_t *ek = FindInsertEML(&str_count,ptr->key,CPM_LINK,0);
	    if (!ek->data.data.ptr)
	    {
		ek->data.data.ptr	= CALLOC(1,sizeof(count_t));
		ek->data.data.len	= sizeof(count_t);
		ek->data.is_alloced	= true;
	    }
	    count_t *cnt = (count_t*)ek->data.data.ptr;
	    cnt->n++;
	    cnt->size += GetGoodAllocSize(strlen(ptr->data)+1);
	}
     #endif
    }

    fprintf(f,"%s  Summaries by track status:%s\n",colset->heading,colset->reset);
    for ( int i = 0; i < LTS__N; i++ )
	if (n_status[i])
	    fprintf(f,"%8u of status %s\n",n_status[i],GetNameLTS(i));

    fprintf(f,"%s  Summaries by track type:%s\n",colset->heading,colset->reset);
    if (n_arena)  fprintf(f,"%8u of type BATTLE ARENA\n",n_arena);
    if (n_track)  fprintf(f,"%8u of type VERSUS TRACK\n",n_track);
    if (n_random) fprintf(f,"%8u of type RANDOM\n",n_random);

    fprintf(f,"%s  String usage:%s\n",colset->heading,colset->reset);
    if (n_sha1)
	fprintf(f,"%8u string%c of type SHA1   (not alloced)\n",
			n_sha1, n_sha1 == 1 ? ' ' : 's' );
    if (n_sha1_d)
	fprintf(f,"%8u string%c of type D-SHA1 (not alloced)\n",
			n_sha1_d, n_sha1_d == 1 ? ' ' : 's' );

    for ( int ltt = 0; ltt < LTT__N; ltt++ )
    {
	if (n_text[ltt])
	{
	    char type[20];
	    snprintf(type,sizeof(type),"%s,",GetUpperNameLTT(ltt));
	    fprintf(f,"%8u string%c of type %-8s %s\n",
		n_text[ltt], n_text[ltt] == 1 ? ' ' : 's',
		type, PrintSize1000(0,0,size_text[ltt],DC_SFORM_ALIGN));
	}
    }

 #if LE_STRING_SET_ENABLED
    if (str_count.used)
    {
	fprintf(f,"%s  String set usage:%s\n",colset->heading,colset->reset);
	exmem_key_t *end = str_count.list + str_count.used;
	for ( exmem_key_t *ek = str_count.list; ek < end; ek++ )
	{
	    count_t *cnt = (count_t*)ek->data.data.ptr;
	    fprintf(f,"%8u, %s : %s\n",
		cnt->n, PrintSize1000(0,0,cnt->size,DC_SFORM_ALIGN), ek->key );
	}
    }
 #endif // LE_STRING_SET_ENABLED

    //--- cups

    fprintf(f,"\r\n%sBattle Cups:%s\r\n",colset->caption,colset->reset);
    debug_cups(f,&ld->cup_battle);

    fprintf(f,"\r\n%sVersus Cups:%s\r\n",colset->caption,colset->reset);
    debug_cups(f,&ld->cup_versus);

    //--- misc

    fprintf(f,"\r\n%sCurrent LE-CODE binaries:%s\r\n",colset->caption,colset->reset);
    CreateLeInfoLD(f,ld,true);

    //--- terminate

 #if LE_STRING_SET_ENABLED
    ResetEML(&str_count);
 #endif

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: text jobs			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SubstLD ( le_distrib_t *ld, const le_strpar_t *dest, ccp arg )
{
    if ( !ld || !dest || !arg || !*arg )
	return ERR_MISSING_PARAM;

    //--- setup

    Regex_t re;
    enumError err = ScanRegex(&re,true,arg);
    PRINT0("err=%d, valid=%d\n",err,re.valid);
    if ( err || !re.valid )
    {
	ResetRegex(&re);
	return err ? err : ERR_INVALID_DATA;
    }

    struct { FastBuf_t b; char space[1000]; } result;
    InitializeFastBuf(&result,sizeof(result));

    SetupReferenceList(ld,filter_options_td,dest->opt);

    //--- main loop

    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	le_track_t *lt = *plt;
	if ( lt->track_status < LTS_EXPORT )
	    continue;

	ccp text = GetTextLT(lt,dest,0);
	if ( text && *text )
	{
	    ReplaceRegex(&re,&result.b,text,-1);
	    SetTextLT(lt,dest,result.b.buf);
	}
    }

    //--- terminate

    ResetRegex(&re);
    ResetFastBuf(&result.b);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CopyLD ( le_distrib_t *ld, const le_strpar_t *dest,
				     const le_strpar_t *src, int n_src )
{
    if ( !ld || !dest || !src || n_src < 1 )
	return ERR_MISSING_PARAM;

    char buf[LE_TRACK_STRING_MAX+10];

    SetupReferenceList(ld,filter_options_td,dest->opt);
    for ( le_track_t **plt = ld->reflist; *plt; plt++ )
    {
	le_track_t *lt = *plt;
	char *bufptr = buf;
	for ( int idx = 0; idx < n_src; idx++ )
	{
	    if ( bufptr > buf )
		*bufptr++ = '\1';
	    ccp text = GetTextLT(lt,src+idx,0);
	    if (IsInputNameValidLEO(text,src[idx].opt))
		bufptr = StringCopyE(bufptr,buf+sizeof(buf),text);
	    if ( bufptr >= buf + LE_TRACK_STRING_MAX )
	    {
		buf[LE_TRACK_STRING_MAX-1] = 0;
		break;
	    }
	}

	ccp *ptr = GetPtrLT(lt,dest);
	if (IsAssignmentAllowedLEO( ptr ? *ptr : 0,buf,dest->opt))
	    SetTextLT(lt,dest,buf);
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: copy files		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError TransferFilesLD ( le_distrib_t *ld, bool logit, bool testmode )
{
    LogFile_t log0 = { .log = stdlog };
    LogFile_t *log = logit ? &log0 : 0;

    TransferMode_t flags = testmode ? TFMD_F_TEST : 0;

    le_track_t *lt = ld->tlist;
    for ( int slot = 0; slot < ld->tlist_used; slot++, lt++ )
	if ( lt->track_status >= LTS_EXPORT )
	{
	    ccp file = GetFileLT(lt,0);
	    if (file)
		TransferTrackFile(log,lt->track_slot,file,flags);
	}

    // support for fill-tracks
    TransferTrackBySlot(log,ld->cup_battle.fill_slot,ld->cup_battle.fill_src,flags);
    TransferTrackBySlot(log,ld->cup_versus.fill_slot,ld->cup_versus.fill_src,flags);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		le_distrib_t: distribution		///////////////
///////////////////////////////////////////////////////////////////////////////

void UpdateDistribLD ( le_distrib_t *ld, bool overwrite )
{
    DASSERT(ld);
    char buf[100];

    UpdateCupsLD(ld);


    //-- UUID

    uuid_buf_t uuid;
    bool create_uuid = overwrite;
    if (!create_uuid)
    {
	const ParamFieldItem_t *it = FindParamField(&ld->dis_param,"UUID");
	if ( !it || !it->data || ScanUUID(uuid,(ccp)it->data) == (ccp)it->data )
	    create_uuid = true;
    }

    if (create_uuid)
    {
	CreateTextUUID(buf,sizeof(buf));
	ReplaceParamField(&ld->dis_param,"UUID",false,0,STRDUP(buf));
    }


    //-- time stamps

    ccp tim = PrintTimeByFormat("%F %T %z",GetTimeSec(false));

    if ( overwrite || !FindParamField(&ld->dis_param,"FIRST-CREATION") )
	ReplaceParamField(&ld->dis_param,"FIRST-CREATION",false,0,STRDUP(tim));
    ReplaceParamField(&ld->dis_param,"LAST-UPDATE",false,0,STRDUP(tim));
}

///////////////////////////////////////////////////////////////////////////////

bool AddDistribParamLD ( le_distrib_t *ld, ccp src, ccp end )
{
    if ( !src || *src != '@' )
	return false;

    char name_buf[100], *name = name_buf;
    src++;

    //-- scan name

    for(;;)
    {
	const char ch = *src;
	if ( !isalnum(ch) && ch != '_' && ch != '-' && ch != '.' )
	    break;
	src++;
	if ( name < name_buf + sizeof(name_buf) - 1 )
	    *name++ = toupper(ch);
    }

    //-- skip spaces

    while ( src < end && (uchar)*src <= ' ' )
	src++;

    //--- check for '='

    if ( src < end && *src == '=' )
    {
	src++;
	while ( src < end && (uchar)*src <= ' ' )
	    src++;
    }

    if ( name > name_buf )
    {
	PRINT0("@%.*s = |%.*s|\n",(int)(name-name_buf),name_buf,(int)(end-src),src);
	ReplaceParamField( &ld->dis_param, MEMDUP(name_buf,name-name_buf),
			true, 0, MEMDUP(src,end-src) );
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

enumError ImportDistribLD ( le_distrib_t *ld, cvp data, uint size )
{
    if ( !ld || !data || !size )
	return ERR_MISSING_PARAM;

    ScanText_t st;
    SetupScanText(&st,data,size);
    const enumError err = ImportDistribSTLD(ld,&st);
    ResetScanText(&st);
    return err;
}

//-----------------------------------------------------------------------------

enumError ImportDistribSTLD ( le_distrib_t *ld, ScanText_t *st )
{
    if ( !ld || !st )
	return ERR_MISSING_PARAM;

    while (NextLineScanText(st))
    {
	PRINT0("LINE: |%.*s|\n",(int)(st->eol-st->line),st->line);
	if ( *st->line == '@' )
	    AddDistribParamLD(ld,st->line,st->eol);
	else
	    ScanSha1LineLD(ld,st->line,st->eol,false);
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateDistribLD ( FILE *f, le_distrib_t *ld, bool use_xname )
{
    if ( !f || !ld )
	return ERR_MISSING_PARAM;

    UpdateDistribLD(ld,false);

    ccp uuid = GetParamFieldStr(&ld->dis_param,1,"UUID","");
    fprintf(f,text_distrib_head_cr

	,GetParamFieldStr(&ld->dis_param,1,"COMMENT1","")
	,GetParamFieldStr(&ld->dis_param,1,"COMMENT2","")
	,GetParamFieldStr(&ld->dis_param,1,"COMMENT3","")
	,GetParamFieldStr(&ld->dis_param,1,"COMMENT4","")
	,GetParamFieldStr(&ld->dis_param,1,"COMMENT5","")

	,GetParamFieldStr(&ld->dis_param,1,"USER-CT-WIIMM","")
	,GetParamFieldStr(&ld->dis_param,1,"USER-WIIMMFI","")
	,GetParamFieldStr(&ld->dis_param,1,"USER-CT-WIIKI","")
	,GetParamFieldStr(&ld->dis_param,1,"USER-MISC","")
	,GetParamFieldStr(&ld->dis_param,1,"MAIL","")
	,GetParamFieldStr(&ld->dis_param,1,"NOTE-FOR-WIMMM","")

	,GetParamFieldStr(&ld->dis_param,1,"NAME","?")
	,GetParamFieldStr(&ld->dis_param,1,"VERSION","?")
	,GetParamFieldStr(&ld->dis_param,1,"AUTHORS","?")
	,GetParamFieldStr(&ld->dis_param,1,"RELEASE-DATE","")
	,GetParamFieldStr(&ld->dis_param,1,"KEYWORDS","")
	,GetParamFieldStr(&ld->dis_param,1,"PREDECESSOR","")

	,GetParamFieldStr(&ld->dis_param,1,"WIIMMFI-REGION","")
	,GetParamFieldStr(&ld->dis_param,1,"INFO-TEXT","")
	,GetParamFieldStr(&ld->dis_param,1,"INFO-URL","")

	,uuid
	,GetParamFieldIntMM(&ld->dis_param,1,"DISPLAY-MODE",3,0,3)
	,GetParamFieldIntMM(&ld->dis_param,1,"DATABASE-NAME",1,0,1)
	,GetParamFieldIntMM(&ld->dis_param,1,"VIEW-COMMENT",0,0,1)
	,GetParamFieldIntMM(&ld->dis_param,1,"ENABLE-NEW",0,0,1)
	,GetParamFieldIntMM(&ld->dis_param,1,"ENABLE-AGAIN",0,0,1)
	,GetParamFieldIntMM(&ld->dis_param,1,"ENABLE-FILL",0,0,1)
	,GetParamFieldIntMM(&ld->dis_param,1,"ENABLE-UPDATE",0,0,1)
	,GetParamFieldIntMM(&ld->dis_param,1,"ENABLE-BOOST",0,0,1)

	,uuid
	,VERSION
	,REVISION_NUM
	,GetParamFieldStr(&ld->dis_param,1,"FIRST-CREATION","")
	,GetParamFieldStr(&ld->dis_param,1,"LAST-UPDATE","")
	);


    //--- mark some more params as 'used'

    GetParamFieldStr(&ld->dis_param,1,"WSZST-VERSION","");
    GetParamFieldStr(&ld->dis_param,1,"WSZST-REVISION","");


    //--- additional parameters

    ccp info = text_distrib_param_cr;

    const ParamFieldItem_t *it = ld->dis_param.field;
    for ( int i = 0; i < ld->dis_param.used; i++, it++ )
	if (!it->num)
	{
	    int len = strlen(it->key);
	    ccp sep = len > 14 ? " " : len > 6 ? "\t" : "\t\t";
	    fprintf(f,"%s@%s%s= %s\r\n", info, it->key, sep, (ccp)it->data );
	    info = "";
	}


    //--- sha1 list of tracks and arenas

    fputs(text_distrib_tracks_cr,f);
    CreateSha1LD(f,ld,use_xname,false);
    fputs("\r\n",f);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// lecode support

void FreeLecodeLD    ( le_distrib_t *ld, le_region_t reg )
{
    if ( !ld || reg < LEREG__BEG || reg > LEREG__END )
	return;

    const int idx = reg - LEREG__BEG;
    if (ld->lecode_alloced[idx])
	FREE((u8*)ld->lecode[idx].ptr);
    ld->lecode[idx].ptr = 0;
    ld->lecode[idx].len = 0;
    ld->lecode_alloced[idx] = false;
}

///////////////////////////////////////////////////////////////////////////////

mem_t AssignLecodeLD ( le_distrib_t *ld, le_region_t reg, cvp data, uint size )
{
    if ( !ld || reg < LEREG__BEG || reg > LEREG__END )
	return NullMem;

    FreeLecodeLD(ld,reg);
    const int idx = reg - LEREG__BEG;
    mem_t *lec = ld->lecode + idx;
    if ( data && size )
    {
	lec->ptr = MEMDUP(data,size);
	lec->len = size;
	ld->lecode_alloced[idx] = true;
    }

    return *lec;
}

///////////////////////////////////////////////////////////////////////////////

mem_t GetLecodeLD ( const le_distrib_t *ld, le_region_t reg )
{
    if ( !ld || reg < LEREG__BEG || reg > LEREG__END )
	return NullMem;

    const int idx = reg - LEREG__BEG;
    mem_t lecode = ld->lecode[idx];
    if (!lecode.ptr)
	lecode = GetLecodeLEREG(reg);
    return lecode;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ledis_dump support		///////////////
///////////////////////////////////////////////////////////////////////////////

int EnableLDUMP ( ccp filename )
{
    if ( ledis_dump_enabled >= 0 )
    {
	ledis_dump_enabled = 1;
	if (filename)
	{
	    FreeString(opt_create_dump);
	    opt_create_dump = STRDUP(filename);
	}
    }
    return ledis_dump_enabled;
}

///////////////////////////////////////////////////////////////////////////////

enumError CloseLDUMP()
{
    enumError err = ERR_NOTHING_TO_DO;
    if ( ledis_dump_enabled > 0 )
    {
	if ( opt_create_dump && *opt_create_dump && ledis_dump.tlist_used )
	{
	    File_t F;
	    err = CreateFile(&F,true,opt_create_dump,FM_OVERWRITE|FM_STDIO|FM_TOUCH);
	    if (!err)
		err = CreateRefLD(F.f,&ledis_dump,true);
	    CloseFile(&F,0);
	}
	ResetLD(&ledis_dump);
	ledis_dump_enabled = -1;
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ImportAnaLDUMP ( const le_analyse_t *ana )
{
    if ( ledis_dump_enabled > 0 )
	ImportAnaLD(&ledis_dump,ana);
}

///////////////////////////////////////////////////////////////////////////////

void ImportCtcodeLDUMP ( const ctcode_t *ctcode )
{
    if ( ledis_dump_enabled > 0 )
	ImportCtcodeLD(&ledis_dump,ctcode);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    struct DistributionInfo_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeDistributionInfo
	( DistributionInfo_t * dinf, bool add_default_param )
{
    DASSERT(dinf);
    memset(dinf,0,sizeof(*dinf));
    InitializeLD(&dinf->ld);
    InitializeParamField(&dinf->translate);
    dinf->translate.free_data = true;

    uint slot;
    for ( slot = 0; slot < MAX_DISTRIBUTION_ARENA; slot++ )
    {
	ParamField_t *pf = dinf->arena + slot;
	InitializeParamField(pf);
	pf->free_data = true;
    }

    for ( slot = 0; slot < MAX_DISTRIBUTION_TRACK; slot++ )
    {
	ParamField_t *pf = dinf->track + slot;
	InitializeParamField(pf);
	pf->free_data = true;
    }

    if (add_default_param)
	AddParamDistributionInfo(dinf,true);
}

///////////////////////////////////////////////////////////////////////////////

void ResetDistributionInfo ( DistributionInfo_t * dinf )
{
    if (dinf)
    {
	ResetParamField(&dinf->translate);
	ResetLD(&dinf->ld);

	uint slot;
	for ( slot = 0; slot < MAX_DISTRIBUTION_ARENA; slot++ )
	    ResetParamField(dinf->arena+slot);
	for ( slot = 0; slot < MAX_DISTRIBUTION_TRACK; slot++ )
	    ResetParamField(dinf->track+slot);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AddParamDistributionInfo ( DistributionInfo_t * dinf, bool overwrite )
{
    DASSERT(dinf);
    UpdateDistribLD(&dinf->ld,overwrite);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot translation		///////////////
///////////////////////////////////////////////////////////////////////////////

char * ScanSlot ( uint *res_slot, ccp source, bool need_point )
{
    DASSERT(res_slot);
    DASSERT(source);
    *res_slot = 0;

    char *end;
    uint slot = strtoul(source,&end,10);
    if ( !need_point
	&& slot >= MIN_DISTRIBUTION_SLOT
	&& slot <  MAX_DISTRIBUTION_TRACK
	&& (uchar)*end <= ' '
	&& ( end-source == 2 || end-source == 3 ))
    {
	*res_slot = slot;
	return end;
    }

    slot *= 10;
    if ( slot < MIN_DISTRIBUTION_SLOT || slot >= MAX_DISTRIBUTION_TRACK || *end != '.' )
	return (char*)source;

    uint track = strtoul(end+1,&end,10);
    slot += track;
    if ( track < 1 || track > 9 || slot >= MAX_DISTRIBUTION_TRACK )
	return (char*)source;

    *res_slot = slot;
    while ( *end == ' ' || *end == '\t' )
	end++;
    return end;
}

///////////////////////////////////////////////////////////////////////////////

void NormalizeSlotTranslation ( char *buf, uint bufsize, ccp name,
			char ** p_first_para, char ** p_first_brack )
{
    DASSERT(buf);
    DASSERT(bufsize>10);

    if (!name)
	name = "";

    char *dest = buf, *buf_end = buf + bufsize - 4;
    char *first_para = 0;
    char *first_brack = 0;

    uint have_space = 0;
    while ( *name && dest < buf_end )
    {
	int ch = (uchar)*name++;
	switch(ch)
	{
	    case ' ':
	    case '_':
	    case '-':
	    case '+':
		have_space++;
		break;

	    case '\'':
		break; // ignore

	    case '[':
		if (!first_brack)
		    first_brack = dest;
		goto bracket;

	    case '(':
		if (!first_para)
		    first_para = dest;
		// fall through

	    case '{':
	    bracket:
		ch = '(';
		// fall through

	    case '.':
		if (have_space)
		{
		    have_space = 0;
		    *dest++ = ' ';
		}
		*dest++ = ch;
		break;

	    case '}':
	    case ')':
	    case ']':
		*dest++ = ')';
		break;

	    default:
		ch = tolower(ch);
		if ( ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' )
		{
		    if (have_space)
		    {
			have_space = 0;
			*dest++ = ' ';
		    }
		    *dest++ = ch;
		}
		break;
	}
    }

    if ( dest > buf )
    {
	if ( dest > buf+4 && !memcmp(dest-4,".szs",4) || !memcmp(dest-4,".wbz",4) )
	    dest -= 4;
	if ( dest > buf+2 && !memcmp(dest-2,"_d",2) )
	    dest -= 2;
	*dest = 0;

	if ( first_brack)
	{
	    if ( first_brack >= dest )
		first_brack = 0;
	    else
		dest = first_brack;
	}

	if ( first_para >= dest )
	    first_para = 0;
    }
    else
	first_para = first_brack = 0;

    if (p_first_para)
	*p_first_para = first_para;
    if (p_first_brack)
	*p_first_brack = first_brack;
}

///////////////////////////////////////////////////////////////////////////////

uint DefineSlotTranslation
	( ParamField_t *translate, bool is_arena, uint slot, ccp name )
{
    DASSERT(translate);
    if (!name)
	return 0;

    if (is_arena)
    {
	if ( slot >= MAX_DISTRIBUTION_ARENA )
	    slot = 0;
	else
	    slot += DISTRIBUTION_ARENA_DELTA;
    }
    else
    {
	if ( slot >= MAX_DISTRIBUTION_TRACK )
	    slot = 0;
    }

    char buf[1000];
    char *first_para = 0, *first_brack = 0;
    NormalizeSlotTranslation(buf,sizeof(buf),name,&first_para,&first_brack);

    uint count = 0;
    if (*buf)
    {
	ReplaceParamField(translate,buf,false,slot,0);
	count++;

	if (first_brack)
	{
	    *first_brack = 0;
	    ReplaceParamField(translate,buf,false,slot,0);
	    count++;
	}

	if (first_para)
	{
	    *first_para = 0;
	    ReplaceParamField(translate,buf,false,slot,0);
	    count++;
	}
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

#undef EARLY_NUM_SLOT
#define EARLY_NUM_SLOT 1

int FindSlotByTranslation ( const ParamField_t *translate, ccp fname, ccp sha1 )
{
    DASSERT(translate);
    ccp slash = strrchr(fname,'/');
    if (slash)
	fname = slash+1;

 #if EARLY_NUM_SLOT
    uint slot;
    ScanSlot(&slot,fname,false);
    if (slot)
	return slot;
 #endif

    char buf[1000];

    char *first_para = 0, *first_brack = 0;
    NormalizeSlotTranslation(buf,sizeof(buf),fname,&first_para,&first_brack);
    const ParamFieldItem_t *it = FindParamField(translate,buf);
    if ( !it && first_brack )
    {
	*first_brack = 0;
	it = FindParamField(translate,buf);
    }

    if ( !it && first_para )
    {
	*first_para = 0;
	it = FindParamField(translate,buf);
    }

    if ( !it && sha1 )
	it = FindParamField(translate,sha1);


 #if EARLY_NUM_SLOT
    return it ? it->num : 0;
 #else
    if (it)
	return it->num;

    uint slot;
    ScanSlot(&slot,fname,false);
    return slot;
 #endif
}

#undef EARLY_NUM_SLOT

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanSlotTranslation
	( ParamField_t *translate, le_distrib_t *ld, ccp fname, bool ignore )
{
    DASSERT(translate);
    DASSERT(fname);
    PRINT(" - scan translation%s: %s\n", ld ? "+ld" : "", fname);

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,fname,true,ignore,true);
    if (err)
	return;


    //--- support for special file formats

    switch((int)szs.fform_arch)
    {
	case FF_BMG:
	    if (verbose>1)
		printf(" - scan translation [BMG]: %s\n",fname);
	    ResetSZS(&szs);
	    return;
    }


    //--- standard text file

    if ( verbose > 1 )
	printf("Scan translation%s[TEXT/%s]: %s\n",
		ld ? "+ld" : "",
		GetExtFF(szs.fform_file,szs.fform_arch), fname );
    else if ( verbose >= 0 )
	printf("Scan %s\n",fname);

    char *eol = (char*)szs.data;
    char *end = eol + szs.size;

    for(;;)
    {
	while ( eol < end && (uchar)*eol <= ' ' )
	    eol++;
	if ( eol == end )
	    break;

	char *src = eol;
	while ( eol < end && *eol != 0 && *eol != '\r' && *eol != '\n' )
	    eol++;
	*eol = 0;

	if ( *src == '#' || *src == '!' )
	    continue;


	//--- manage parameters

	if ( *src == '@' )
	{
	    if (ld)
		AddDistribParamLD(ld,src,eol);
	    continue;
	}

	PRINT0("LINE: |%.*s|\n",(int)(eol-src),src);


	//--- scan new sha1 list

	if (ScanSha1LineLD(ld,src,eol,false))
	    continue;


	//--- skip "()"  |  detect SHA1 checksum

	char *sha1 = 0;
	if ( *src == '(' )
	{
	    while ( src < eol && *src != ')' )
		src++;
	    if ( src++ == eol )
		continue;
	}
	else
	{
	    // detect sha1 checksum

	    char *x = src;
	    while ( x < eol && isalnum((int)*x) )
		x++;
	    if ( x < eol && x - src == 40 )
	    {
		sha1 = src;
		src = x;
	    }
	}

	while ( src < eol && (uchar)*src <= ' ' )
	    src++;

	const bool is_arena = *src == 'a' || *src == 'A';
	if (is_arena)
	    src++;

	if (!isdigit((int)*src))
	    continue;

	uint slot;
	src = ScanSlot(&slot,src,true); // ???
	if (!slot)
	    continue;

	char *tab = strrchr(src,'\t');
	if (tab)
	    src = tab;
	while ( src < eol && (uchar)*src <= ' ' )
	    src++;

	DefineSlotTranslation(translate,is_arena,slot,src);
	if (sha1)
	{
	    sha1[40] = 0;
	    DefineSlotTranslation(translate,is_arena,slot,sha1);
	}
    }

    ResetSZS(&szs);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			misc				///////////////
///////////////////////////////////////////////////////////////////////////////

void ScanOptLeDefine ( ccp arg )
{
    opt_le_define = arg;
    if ( arg && *arg )
	AppendStringField(&le_define_list,arg,false);
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintLEFT ( le_flags_t flags )
{
    char *buf = GetCircBuf(5);
    buf[0] = flags & LETF_NEW	    ? 'N' : '-';
    buf[1] = flags & LETF_RND_HEAD  ? 'H' : '-';
    buf[2] = flags & LETF_RND_GROUP ? 'G' : '-';
    buf[3] = flags & LETF_ALIAS	    ? 'A' : '-';
    buf[4] = 0;
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

le_flags_t ScanLEFT ( ccp text )
{
    le_flags_t res = 0;
    if (text)
    {
	if ( strchr(text,'N') || strchr(text,'n') ) res |= LETF_NEW;
	if ( strchr(text,'H') || strchr(text,'h') ) res |= LETF_RND_HEAD;
	if ( strchr(text,'G') || strchr(text,'g') ) res |= LETF_RND_GROUP;
	if ( strchr(text,'A') || strchr(text,'a') ) res |= LETF_ALIAS;
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetNameLTS ( le_track_status_t lts )
{
    switch(lts)
    {
	case LTS_INVALID: return "INVALID";
	case LTS_VALID:   return "VALID";
	case LTS_FAIL:    return "FAIL";
	case LTS_ACTIVE:  return "ACTIVE";
	case LTS_FILL:    return "FILL";
	case LTS_EXPORT:  return "EXPORT";
	case LTS__N:      break;
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////



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

#include "lib-kmp.h"
#include "lib-kcl.h"
#include "lib-szs.h"
#include "lib-bzip2.h"
#include "lib-mkw.h"
#include "ui.h"

#include <math.h>
#include <stddef.h>
#include <float.h>

///////////////////////////////////////////////////////////////////////////////

#define SECT_NAME(s) kmp_section_name[s].name1

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    KMP action log		///////////////
///////////////////////////////////////////////////////////////////////////////

bool KMP_ACTION_LOG ( bool is_patch, const char * format, ... )
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[KMP]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

    if ( verbose > 2 || KMP_MODE & KMPMD_LOG || is_patch && ~KMP_MODE & KMPMD_SILENT )
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[KMP]%s ",
		colset->heading,
		is_patch ? colset->highlight : colset->info );
	va_list arg;
	va_start(arg,format);
	vfprintf(stdlog,format,arg);
	va_end(arg);
	fputs(colset->reset,stdlog);
	fflush(stdlog);
	return true;
    }

    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Wim0 support			///////////////
///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t Wim0SectionName[] =
{
	{ W0ID_END,		"END",		0, W0F_BINARY },
	{ W0ID_VERSION,		"VERSION",	0, W0F_TEXT },
//	{ W0ID_AUTO_CONNECT,	"AUTO-CONNECT",	0, W0F_BINARY },

	{0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

ccp Wim0FormatName[] =
{
	"?",		// W0F_UNKNOWN,
	"binary",	// W0F_BINARY,
	"text",		// W0F_TEXT,
	0		// W0F__N
};

///////////////////////////////////////////////////////////////////////////////

void InitializeWim0 ( kmp_wim0_t *w0 )
{
    DASSERT(w0);
    memset(w0,0,sizeof(*w0));
}

///////////////////////////////////////////////////////////////////////////////

void ResetWim0 ( kmp_wim0_t *w0 )
{
    if (w0)
    {
	FREE(w0->data);
	FREE(w0->bz2data);
	InitializeWim0(w0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void * NeedWim0 ( kmp_wim0_t *w0, uint size, uint align )
{
    DASSERT(w0);
    if (!w0->data)
    {
	DASSERT(!w0->used);
	DASSERT(!w0->size);
	w0->size = ALIGN32(size,4) + 0x1000;
	w0->data = CALLOC(w0->size,1);
    }

    u32 start = w0->used;
    if ( align > 0 )
    {
	start = ALIGN32(start,align);
	size  = ALIGN32(size,align);
    }

    uint need = start + size;
    PRINT("NeedWim0(,%d,%d) need=%d/%d/%d\n",size,align,need,w0->used,w0->size);
    if ( need > w0->used )
    {
	w0->size = ALIGN32(need,4) + 0x1000;
	w0->data = REALLOC(w0->data,w0->size);
	memset(w0->data + w0->used, 0, w0->size - w0->used );
    }

    w0->used = need;
    return w0->data + start;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressWim0 ( kmp_wim0_t *w0 )
{
    DASSERT(w0);
    FREE(w0->bz2data);
    w0->bz2data = 0;

    if (!w0->used)
    {
	w0->bz2size = 0;
	return ERR_NOTHING_TO_DO;
    }

    enumError err = EncodeBZIP2( &w0->bz2data, &w0->bz2size,
				false, 0,
				w0->data, w0->used, 9 );

    DASSERT( err || w0->bz2data );
    DASSERT( err || w0->bz2size );

    if ( err || w0->bz2size > w0->used + 4 )
    {
	PRINT("CompressWim0() use uncompressed data!\n");
	w0->bz2size = ALIGN32(w0->used,4) + 4;
	FREE(w0->bz2data);
	w0->bz2data = CALLOC(w0->bz2size,1);
	memcpy(w0->bz2data+4,w0->data,w0->used);
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecompressWim0 ( kmp_wim0_t *w0 )
{
    DASSERT(w0);
    FREE(w0->data);
    w0->data = 0;

    if ( w0->bz2size <= 4 )
    {
	w0->used = w0->size = 0;
	return ERR_NOTHING_TO_DO;
    }

    DASSERT(w0->bz2data);
    if (!*(u32*)w0->bz2data)
    {
	// uncompressed data
	w0->used = w0->bz2size - 4;
	w0->size = ALIGN32(w0->used,4);
	w0->data = CALLOC(w0->size,1);
	memcpy(w0->data,w0->bz2data+4,w0->used);
	return ERR_OK;
    }

    enumError err = DecodeBZIP2( &w0->data, &w0->size, 0, w0->bz2data, w0->bz2size );
    DASSERT( err || w0->data );
    DASSERT( err || w0->size );
    w0->used = w0->size;
    PRINT_IF(err,"DecompressWim0() failed, err=%d: %p %d\n",err,w0->bz2data,w0->bz2size);
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

kmp_wim0_sect_t * AppendSectionWim0
(
    kmp_wim0_t		*w0,		// data structure
    kmp_wim0_sect_id	sect_id,	// section id
    cvp			data,		// pointer to data
    uint		size		// size of data in bytes
)
{
    DASSERT(w0);
    DASSERT( data || !size );

    if ( size > 0x1fffe )
	size = 0x1fffe;
    const uint size2 = ALIGN32(size,2);
    kmp_wim0_sect_t *sect = NeedWim0(w0,sizeof(kmp_wim0_sect_t)+size2,0);
    DASSERT(sect);

    sect->id	= htons(sect_id);
    sect->words	= htons(size2/2);
    if (size2)
    {
	sect->data[size2-1] = 0;
	memcpy(sect->data,data,size);
    }
    return sect;
}

///////////////////////////////////////////////////////////////////////////////

static bool CreateWim0
(
    const u8		**res_data,	// not NULL: store data pointer here
    uint		*res_size,	// not NULL: store data size here
    kmp_t		*kmp		// pointer to valid KMP
)
{
    DASSERT(res_data);
    DASSERT(res_size);
    DASSERT(kmp);


    //--- setup

    kmp_wim0_t w0;
    InitializeWim0(&w0);
    AppendSectionWim0(&w0,W0ID_VERSION,BASE_VERSION,sizeof(BASE_VERSION));

    if ( opt_wim0 >= 12 )
    {
	AppendSectionWim0(&w0,0xffff,LoDigits,sizeof(LoDigits));
	AppendSectionWim0(&w0,0xffff,LoDigits,sizeof(LoDigits));
    }

    // W0ID_AUTO_CONNECT

    //--- create data
    // ???


    //--- compress data and store result

    if ( opt_wim0 >= 11 )
	AppendSectionWim0(&w0,0xffff,LoDigits,sizeof(LoDigits));

    AppendSectionWim0(&w0,W0ID_END,0,0);

    bool stat = w0.used > 0;
    if (stat)
    {
	stat = CompressWim0(&w0) == ERR_OK && w0.bz2size;
	if (stat)
	{
	    *res_data = w0.bz2data;
	    *res_size = w0.bz2size;
	    w0.bz2data = 0;
	}
    }
    ResetWim0(&w0);

    if (!stat)
    {
	*res_data = 0;
	*res_size = 0;
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool GetSectionHelperWim0
(
    kmp_wim0_info_t		*info,		// store result here (never NULL)
    const struct kmp_t		*kmp,		// pointer to valid KMP
    const kmp_wim0_sect_t	*sect,		// next section candidate
    uint			index		// index of next section candidate
)
{
    DASSERT(info);
    DASSERT(kmp);

    memset(info,0,sizeof(*info));

    const u8 *min_ptr = (u8*)kmp->wim0_data;
    if ( !sect || !min_ptr || (u8*)sect < min_ptr )
	return false;

    const u8 *max_ptr = (u8*)min_ptr + kmp->wim0_size;
    if ( (u8*)sect > max_ptr )
	return 0;

    const uint words = ntohs(sect->words);
    const kmp_wim0_sect_t *next = (kmp_wim0_sect_t*)( sect->data + words );
    if ( (u8*)next > max_ptr )
	return 0;

    info->sect_id	= ntohs(sect->id);
    info->index		= index;
    info->offset	= (u8*)sect - min_ptr;
    info->name[0]	= 0;
    info->sect		= sect;
    info->data		= (u8*)sect->data;
    info->size		= 2 * words;

    const KeywordTab_t *key;
    for ( key = Wim0SectionName; key->name1; key++ )
	if ( key->id == info->sect_id )
	{
	    StringCopyS(info->name,sizeof(info->name),key->name1);
	    info->format = key->opt;
	    break;
	}
    if (!*info->name)
	snprintf(info->name,sizeof(info->name),"0x%04x",info->sect_id);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool GetFirstSectionWim0
(
    kmp_wim0_info_t	*info,		// store result here (never NULL)
    const struct kmp_t	*kmp		// pointer to valid KMP
)
{
    DASSERT(info);
    DASSERT(kmp);
    return GetSectionHelperWim0(info,kmp,kmp->wim0_data,0);
}

///////////////////////////////////////////////////////////////////////////////

bool GetNextSectionWim0
(
    kmp_wim0_info_t	*info,		// update result (never NULL)
    const struct kmp_t	*kmp		// pointer to valid KMP
)
{
    DASSERT(info);
    DASSERT(kmp);

    const kmp_wim0_sect_t *sect = info->sect;
    if (sect)
    {
	const uint words = ntohs(sect->words);
	const kmp_wim0_sect_t *next = (kmp_wim0_sect_t*)( sect->data + words );
	return GetSectionHelperWim0(info,kmp,next,info->index+1);
    }

    memset(info,0,sizeof(*info));
    return false;
}

///////////////////////////////////////////////////////////////////////////////

const kmp_wim0_sect_t * FindSectionWim0
(
    // return NULL or a pointer to found kmp_wim0_sect_t

    kmp_wim0_info_t	*info,		// not NULL: store extended result here
    const struct kmp_t	*kmp,		// pointer to valid KMP
    kmp_wim0_sect_id	sect_id		// section id to search
)
{
    DASSERT(kmp);

    if (info)
	memset(info,0,sizeof(*info));

    const kmp_wim0_sect_t *sect = kmp->wim0_data;
    if (!sect)
	return 0;

    const u8 *max_ptr = (u8*)sect + kmp->wim0_size;
    int index;
    for( index = 0; ; index++ )
    {
	if ( (u8*)sect + sizeof(*sect) > max_ptr )
	    return 0;
	const uint words = ntohs(sect->words);
	const kmp_wim0_sect_t *next = (kmp_wim0_sect_t*)( sect->data + words );
	if ( (u8*)next > max_ptr )
	    return 0;

	const u16 id = ntohs(sect->id);
	PRINT("WIM0 SECTION %04x, %u words\n",id,words);
	if ( id == sect_id )
	    break;
	if (!id)
	    return 0;
	sect = next;
    }

    // we found the section!

    if (info)
    {
	info->sect_id	= sect_id;
	info->index	= index;
	info->offset	= (u8*)sect - (u8*)kmp->wim0_data;
	info->name[0]	= 0;
	info->sect	= sect;
	info->data	= (u8*)sect->data;
	info->size	= 2 * ntohs(sect->words);

	const KeywordTab_t *key;
	for ( key = Wim0SectionName; key->name1; key++ )
	    if ( key->id == sect_id )
	    {
		StringCopyS(info->name,sizeof(info->name),key->name1);
		info->format = key->opt;
		break;
	    }
	if (!*info->name)
	    snprintf(info->name,sizeof(info->name),"0x%04x",sect_id);
    }

    return sect;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanWim0 ( kmp_t *kmp )
{
    DASSERT(kmp);

 #if HAVE_PRINT
    {
	const kmp_wim0_sect_t *sect = FindSectionWim0(0,kmp,W0ID_VERSION);
	if (sect)
	    PRINT("WIM0 version: %s\n",(ccp)sect->data);

	kmp_wim0_info_t info;
	bool avail;
	for ( avail = GetFirstSectionWim0(&info,kmp);
	      avail;
	      avail = GetNextSectionWim0(&info,kmp) )
	{
	    printf("%5d. %04x %-10s %4u bytes (%s) %s\n",
		info.index, info.offset, info.name, info.size,
		Wim0FormatName[info.format],
		info.format == W0F_TEXT ? (ccp)info.data : "" );
	}
    }
 #endif

// [[wim0]] scan data
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 kmp_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

kmp_mode_t KMP_MODE = KMPMD_M_DEFAULT;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_kmp_tab[] =
{
  { 0,			"CLEAR",	"RESET",	KMPMD_M_ALL | KMPMD_F_HIDE },
  { KMPMD_M_DEFAULT,	"DEFAULT",	0,		KMPMD_M_ALL | KMPMD_F_HIDE },

  { KMPMD_FORCE,	"FORCE",	0,		0 },
  { KMPMD_NEW,		"NEW",		0,		0 },
  { KMPMD_RM_SPCITEM,	"RM-SPCITEM",	"RMSPCITEM",	0 },
  { KMPMD_REVERSE,	"REVERSE",	0,		0 },

  { KMPMD_POLE_LEFT,	"LEFT",		"LT",		KMPMD_M_POLE },
  { KMPMD_POLE_RIGHT,	"RIGHT",	"RT",		KMPMD_M_POLE },
  { KMPMD_SP_WIDE,	"WIDE",		0,		KMPMD_M_STARTPOS },
  { KMPMD_SP_NARROW,	"NARROW",	0,		KMPMD_M_STARTPOS },

  { KMPMD_1_LAP,	"1LAPS",	"L1",		KMPMD_M_LAPS },
  { KMPMD_2_LAPS,	"2LAPS",	"L2",		KMPMD_M_LAPS },
  { KMPMD_3_LAPS,	"3LAPS",	"L3",		KMPMD_M_LAPS },
  { KMPMD_4_LAPS,	"4LAPS",	"L4",		KMPMD_M_LAPS },
  { KMPMD_5_LAPS,	"5LAPS",	"L5",		KMPMD_M_LAPS },
  { KMPMD_6_LAPS,	"6LAPS",	"L6",		KMPMD_M_LAPS },
  { KMPMD_7_LAPS,	"7LAPS",	"L7",		KMPMD_M_LAPS },
  { KMPMD_8_LAPS,	"8LAPS",	"L8",		KMPMD_M_LAPS },
  { KMPMD_9_LAPS,	"9LAPS",	"L9",		KMPMD_M_LAPS },
  { KMPMD_MAX_LAPS,	"MAX-LAPS",	"MAXLAPS",	0 },

  { KMPMD_FIX_CKPH,	"FIX-CKPH",	"FIXCKPH",	0 },
  { KMPMD_FIX_ENPH,	"FIX-ENPH",	"FIXENPH",	0 },
  { KMPMD_FIX_ITPH,	"FIX-ITPH",	"FIXITPH",	0 },
  { KMPMD_FIX_CKNEXT,	"FIX-CKNEXT",	"FIXCKNEXT",	0 },
  { KMPMD_FIX_CKJGPT,	"FIX-CKJGPT",	"FIXCKJGPT",	0 },

  { KMPMD_M_FIX_PH,	"FIX-PH",	"FIXPH",	KMPMD_M_FIX_PH  | KMPMD_F_HIDE },
  { KMPMD_M_FIX_CK,	"FIX-CK",	"FIXCK",	KMPMD_M_FIX_CK  | KMPMD_F_HIDE },
  { KMPMD_M_FIX_ALL,	"FIX-ALL",	"FIXALL",	KMPMD_M_FIX_ALL | KMPMD_F_HIDE },

  { KMPMD_MASK_PFLAGS,	"MASK-PFLAGS",	"MASKPFLAGS",	0 },
  { KMPMD_RM_LECODE,	"RM-LECODE",	"RMLECODE",	0 },
  { KMPMD_PURGE_GOBJ,	"PURGE-GOBJ",	"PURGEGOBJ",	0 },
  { KMPMD_FULL_DEFOBJ,	"FULL-DEFOBJ",	"FULLDEFOBJ",	0 },

  { KMPMD_DUMP_CLASS,	"DUMP-CLASS",	"DUMPCLASS",	0 },
  { KMPMD_DUMP_ONEWAY,	"DUMP-ONEWAY",	"DUMPONEWAY",	0 },
  { KMPMD_M_DUMP_ALL,	"DUMP-ALL",	"DUMPALL",	KMPMD_M_DUMP_ALL | KMPMD_F_HIDE },

  { KMPMD_TINY_0,	"TINY-0",	"TINY0",	KMPMD_M_TINY },
  { KMPMD_TINY_1,	"TINY-1",	"TINY1",	KMPMD_M_TINY },
  { KMPMD_TINY_2,	"TINY-2",	"TINY2",	KMPMD_M_TINY },
  { KMPMD_TINY_3,	"TINY-3",	"TINY3",	KMPMD_M_TINY },
  { KMPMD_TINY_4,	"TINY-4",	"TINY4",	KMPMD_M_TINY },
  { KMPMD_TINY_5,	"TINY-5",	"TINY5",	KMPMD_M_TINY },
  { KMPMD_TINY_6,	"TINY-6",	"TINY6",	KMPMD_M_TINY },
  { KMPMD_TINY_7,	"TINY-7",	"TINY7",	KMPMD_M_TINY },

  { KMPMD_RM_EMPTY,	"RM-EMPTY",	"RMEMPTY",	0 },
  { KMPMD_MINIMIZE,	"MINIMIZE",	0,		0 },

  { KMPMD_TEST1,	"TEST1",	0,		0 },
  { KMPMD_TEST2,	"TEST2",	0,		0 },

  { KMPMD_LOG,		"LOG",		0,		0 },
  { KMPMD_SILENT,	"SILENT",	0,		0 },
  { KMPMD_INPLACE,	"INPLACE",	0,		0 },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static void SetKmpMode ( kmp_mode_t new_mode )
{
    noPRINT("SET-KMP/IN:  PC=%d, KPC=%d\n",
		have_patch_count, have_kmp_patch_count );

    if ( KMP_MODE & KMPMD_M_PATCH )
	have_patch_count--, have_kmp_patch_count--;

    KMP_MODE = new_mode & KMPMD_M_ALL;
    if ( PATCH_FILE_MODE & PFILE_F_LOG_ALL )
	KMP_MODE |= KMPMD_LOG;
    force_kmp = ( KMP_TFORM & KMPMD_FORCE ) != 0;

    if ( KMP_MODE & KMPMD_M_TINY )
	KMP_MODE |= KMPMD_NEW | KMPMD_RM_EMPTY;

    if ( KMP_MODE & KMPMD_M_PATCH )
	have_patch_count++, have_kmp_patch_count++;

    noPRINT("SET-KMP/OUT: PC=%d, KPC=%d, force=%d\n",
		have_patch_count, have_kmp_patch_count, force_kmp );
}

//-----------------------------------------------------------------------------

int ScanOptKmp ( ccp arg )
{
    if (!arg)
	return 0;

    s64 stat = ScanKeywordList(arg,opt_kmp_tab,0,true,0,KMP_MODE,
					"Option --kmp",ERR_SYNTAX);
    if ( stat != -1 )
    {
	SetKmpMode(stat);
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetKmpMode()
{
    static char buf[200] = {0};
    if (!*buf)
	PrintKeywordList( buf, sizeof(buf), 0, opt_kmp_tab,
				KMP_MODE, KMPMD_M_DEFAULT, KMPMD_F_HIDE );
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

void SetupKMP()
{
    if ( opt_tiny > 0 )
    {
	const kmp_mode_t tiny
		= ( (kmp_mode_t)opt_tiny << KMPMD_S_TINY ) & KMPMD_M_TINY;
	if ( tiny > ( KMP_MODE & KMPMD_M_TINY ))
	    SetKmpMode( KMP_MODE & ~KMPMD_M_TINY | tiny );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 kmp_diff_mode_t		///////////////
///////////////////////////////////////////////////////////////////////////////

kmp_diff_mode_t KMP_DIFF_MODE = KDMD_M_DEFAULT;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_kmp_diff_tab[] =
{
  { KDMD_NONE,		"NONE",		0,		KDMD_M_ALL | KDMD_F_HIDE },
  { KDMD_M_GOOD,	"GOOD",		0,		KDMD_M_ALL | KDMD_F_HIDE },
  { KDMD_M_DEFAULT,	"DEFAULT",	0,		KDMD_M_ALL | KDMD_F_HIDE },
  { KDMD_M_ALL,		"ALL",		0,		KDMD_M_ALL | KDMD_F_HIDE },

  { KDMD_KTPT,		"KTPT",		0,		0 },
  { KDMD_ENPT,		"ENPT",		0,		0 },
  { KDMD_ENPH,		"ENPH",		0,		0 },
  { KDMD_ITPT,		"ITPT",		0,		0 },
  { KDMD_ITPH,		"ITPH",		0,		0 },
  { KDMD_CKPT,		"CKPT",		0,		0 },
  { KDMD_CKPH,		"CKPH",		0,		0 },
  { KDMD_GOBJ,		"GOBJ",		0,		0 },
  { KDMD_POTI,		"POTI",		0,		0 },
  { KDMD_AREA,		"AREA",		0,		0 },
  { KDMD_CAME,		"CAME",		0,		0 },
  { KDMD_JGPT,		"JGPT",		0,		0 },
  { KDMD_CNPT,		"CNPT",		0,		0 },
  { KDMD_MSPT,		"MSPT",		0,		0 },
  { KDMD_STGI,		"STGI",		0,		0 },

  { KDMD_SORT,		"SORT",		"-NOSORT",	KDMD_M_SORT },
  { KDMD_NOSORT,	"NOSORT",	"-SORT",	KDMD_M_SORT },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptKmpDiff ( ccp arg )
{
    if (!arg)
	return 0;

    s64 stat = ScanKeywordList(arg,opt_kmp_diff_tab,0,true,0,KMP_MODE,
					"Option --kmp",ERR_SYNTAX);
    if ( stat != -1 )
    {
	KMP_DIFF_MODE = stat & KDMD_M_ALL;

	if ( !(KMP_DIFF_MODE & KDMD_M_SECTION) )
	    KMP_DIFF_MODE |= KDMD_M_SECTION;

	if ( !(KMP_DIFF_MODE & KDMD_M_SORT) )
	    KMP_DIFF_MODE |= KDMD_SORT;
	else if ( KMP_DIFF_MODE & KDMD_SORT )
	    KMP_DIFF_MODE &= ~KDMD_NOSORT;

	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetKmpDiffMode()
{
    static char buf[200] = {0};
    if (!*buf)
	PrintKeywordList( buf, sizeof(buf), 0, opt_kmp_diff_tab,
				KMP_DIFF_MODE, KDMD_M_DEFAULT, KDMD_F_HIDE );
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 kmp_tform_t			///////////////
///////////////////////////////////////////////////////////////////////////////

kmp_tform_t KMP_TFORM = KMPTF_M_DEFAULT;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_tformkmp_tab[] =
{
  { 0,			"NONE",		"CLEAR",	KMPTF_M_ALL | KMPTF_F_HIDE },
  { KMPTF_M_DEFAULT,	"DEFAULT",	0,		KMPTF_M_ALL | KMPTF_F_HIDE },
  { KMPTF_M_ALL,	"ALL",		0,		KMPTF_M_ALL },

  { KMPTF_AREA,		"AREA",		0,		KMPTF_AREA },
  { 0,			"-AREA",	0,		KMPTF_AREA | KMPTF_F_HIDE },
  { KMPTF_AREA_POS,	"AREA-POS",	"AREAPOS",	0 },
  { KMPTF_AREA_ROTATE,	"AREA-ROTATE",	"AREAROTATE",	0 },
  { KMPTF_AREA_SCALE,	"AREA-SCALE",	"AREASCALE",	0 },

  { KMPTF_CAME,		"CAME",		0,		0 },
  { KMPTF_CAME_POS,	"CAME-POS",	"CAMEPOS",	0 },

  { KMPTF_CKPT,		"CKPT",		0,		0 },
  { KMPTF_CKPT_POS,	"CKPT-POS",	"CKPTPOS",	0 },

  { KMPTF_CNPT,		"CNPT",		0,		KMPTF_CNPT },
  { 0,			"-CNPT",	0,		KMPTF_CNPT | KMPTF_F_HIDE },
  { KMPTF_CNPT_POS,	"CNPT-POS",	"CNPTPOS",	0 },
  { KMPTF_CNPT_ROTATE,	"CNPT-ROTATE",	"CNPTROTATE",	0 },

  { KMPTF_ENPT,		"ENPT",		0,		KMPTF_ENPT },
  { 0,			"-ENPT",	0,		KMPTF_ENPT | KMPTF_F_HIDE },
  { KMPTF_ENPT_POS,	"ENPT-POS",	"ENPTPOS",	0 },
  { KMPTF_ENPT_SCALE,	"ENPT-SCALE",	"ENPTSCALE",	0 },

  { KMPTF_GOBJ,		"GOBJ",		0,		KMPTF_GOBJ },
  { 0,			"-GOBJ",	0,		KMPTF_GOBJ | KMPTF_F_HIDE },
  { KMPTF_GOBJ_POS,	"GOBJ-POS",	"GOBJPOS",	0 },
  { KMPTF_GOBJ_ROTATE,	"GOBJ-ROTATE",	"GOBJROTATE",	0 },
  { KMPTF_GOBJ_SCALE,	"GOBJ-SCALE",	"GOBJSCALE",	0 },

  { KMPTF_ITPT,		"ITPT",		0,		KMPTF_ITPT },
  { 0,			"-ITPT",	0,		KMPTF_ITPT | KMPTF_F_HIDE },
  { KMPTF_ITPT_POS,	"ITPT-POS",	"ITPTPOS",	0 },
  { KMPTF_ITPT_SCALE,	"ITPT-SCALE",	"ITPTSCALE",	0 },

  { KMPTF_JGPT,		"JGPT",		0,		KMPTF_JGPT },
  { 0,			"-JGPT",	0,		KMPTF_JGPT | KMPTF_F_HIDE },
  { KMPTF_JGPT_POS,	"JGPT-POS",	"JGPTPOS",	0 },
  { KMPTF_JGPT_ROTATE,	"JGPT-ROTATE",	"JGPTROTATE",	0 },

  { KMPTF_KTPT,		"KTPT",		0,		KMPTF_KTPT },
  { 0,			"-KTPT",	0,		KMPTF_KTPT | KMPTF_F_HIDE },
  { KMPTF_KTPT_POS,	"KTPT-POS",	"KTPTPOS",	0 },
  { KMPTF_KTPT_ROTATE,	"KTPT-ROTATE",	"KTPTROTATE",	0 },

  { KMPTF_MSPT,		"MSPT",		0,		KMPTF_MSPT },
  { 0,			"-MSPT",	0,		KMPTF_MSPT | KMPTF_F_HIDE },
  { KMPTF_MSPT_POS,	"MSPT-POS",	"MSPTPOS",	0 },
  { KMPTF_MSPT_ROTATE,	"MSPT-ROTATE",	"MSPTROTATE",	0 },

  { KMPTF_POTI,		"POTI",		0,		0 },
  { KMPTF_POTI_POS,	"POTI-POS",	"POTIPOS",	0 },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptTformKmp ( ccp arg )
{
    if (!arg)
	return 0;

    s64 start = 0;
    if ( *arg == '+' )
	start = KMP_TFORM;
    else if ( *arg == '-' )
	start = KMPTF_M_ALL;

    s64 stat = ScanKeywordList(arg,opt_tformkmp_tab,0,true,0,start,
					"Option --tform-kmp",ERR_SYNTAX);
    if ( stat != -1 )
    {
	noPRINT("TFORM: %x -> %llx\n",KMP_TFORM,stat & KMPTF_M_ALL);
	KMP_TFORM = stat & KMPTF_M_ALL;
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

uint PrintKmpTform ( char *buf, uint bufsize, kmp_tform_t mode )
{
    DASSERT(buf);
    DASSERT(bufsize>10);
    char *dest = buf;
    char *end = buf + bufsize - 1;

    mode = mode & KMPTF_M_ALL | KMPTF_F_HIDE;
    kmp_tform_t mode1 = mode;

    const KeywordTab_t *ct;
    for ( ct = opt_tformkmp_tab; ct->name1 && dest < end; ct++ )
    {
	if ( ct->opt & KMPMD_F_HIDE )
	    continue;

	if ( ct->opt ? (mode & ct->opt) == ct->id : mode & ct->id )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = StringCopyE(dest,end,ct->name1);
	    mode &= ~(ct->id|ct->opt);
	}
    }

    if ( mode1 == (KMPTF_M_DEFAULT|KMPTF_F_HIDE) )
	dest = StringCopyE(dest,end," (default)");
    else if (!mode1)
	dest = StringCopyE(dest,end,"(none)");

    *dest = 0;
    return dest-buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetKmpTform()
{
    static char buf[200] = {0};
    if (!*buf)
	PrintKmpTform(buf,sizeof(buf),KMP_TFORM);
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			speed modifier			///////////////
///////////////////////////////////////////////////////////////////////////////

bool  speed_mod_active = false;
float speed_mod_factor = 0.0;
u16   speed_mod_val = 0;

///////////////////////////////////////////////////////////////////////////////

u16 SpeedMod2u16 ( float speed )
{
    u8 val[4];
    write_bef4(val,speed);
    if ( val[2] >= 0x80 && !++val[1] )
	val[0]++;
    return be16(val);
}

///////////////////////////////////////////////////////////////////////////////

float SpeedMod2float ( u16 hex )
{
    u16 val[2] = { htons(hex), 0 };
    return bef4(val);
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptSpeedMod ( ccp arg )
{
    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --speed-mod",0);
    si.float_div++;

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    float factor = GetDoubleV(&temp);
    CheckEolSI(&si);
    ResetSI(&si);

    if (speed_mod_active)
	have_patch_count--, have_kmp_patch_count--;

    if ( factor < 0.0 )
	speed_mod_active = false;
    else
    {
	speed_mod_active = true;
	speed_mod_val    = SpeedMod2u16(factor);
	speed_mod_factor = SpeedMod2float(speed_mod_val);
	have_patch_count++;
	have_kmp_patch_count++;
    }

    return err != ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			second ktpt			///////////////
///////////////////////////////////////////////////////////////////////////////

bool	ktpt2_active	= false;
bool	ktpt2_auto	= false;
double	ktpt2_min	= 0.0;
double3	ktpt2_pos;

//-----------------------------------------------------------------------------

int ScanOptKtpt2 ( ccp arg )
{
    if (ktpt2_active)
    {
	ktpt2_active = false;
	have_patch_count--, have_kmp_patch_count--;
    }
    ktpt2_auto = false;

    if ( !arg || !*arg )
	return 0;

    if (!strncasecmp(arg,"auto",4))
    {
	arg += 4;
	if ( *arg == ',' || *arg == '=' )
	    arg++;
	ktpt2_min = strtod(arg,0);
	if ( !isnormal(ktpt2_min) || ktpt2_min <= 0.0 )
	    ktpt2_min = 0.0;

	ktpt2_auto = ktpt2_active = true;
	have_patch_count++, have_kmp_patch_count++;
	return 0;
    }

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --ktpt2",0);
    si.float_div++;

    DEFINE_VAR(temp);
    enumError err = ScanVectorExprSI(&si,&temp,-1.0,1);
    double3 pos = GetVectorV(&temp);
    CheckEolSI(&si);
    ResetSI(&si);

    if (!err)
    {
	ktpt2_pos = pos;
	ktpt2_active = true;
	have_patch_count++, have_kmp_patch_count++;
    }

    return err != ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			helper functions		///////////////
///////////////////////////////////////////////////////////////////////////////

int GetNextPointKMP
(
    const kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP PT section index
    int			pt_index,	// index of point, <0: add N
    int			sub_index,	// index into 'next'
					//   -1: use 'pt_index+1' in a group
					//       find first valid at group end
					//  !-1 && KMP_CKPT: try to use 'next' link
    bool		unique,		// return 'not_found_val' on duplicates
    int			not_found_val,	// return this value, if next point not found
    int			*ret_pt_index,	// not NULL: store fixed 'pt_index' here
    int			*ret_sub_index,	// not NULL: store used sub index here
    bool		*ret_new_group,	// not NULL: store true, if new group
    const kmp_enph_entry_t **ret_ph	// not NULL: store found ph-record
)
{
    DASSERT(kmp);
    DASSERT( sect == KMP_CKPT || sect == KMP_ENPT || sect == KMP_ITPT );

    if (ret_sub_index)
	*ret_sub_index = -1;
    if (ret_new_group)
	*ret_new_group = false;
    if (ret_ph)
	*ret_ph = 0;

    const List_t *ptlist, *phlist;
    if ( sect == KMP_CKPT )
    {
	ptlist = kmp->dlist + KMP_CKPT;
	phlist = kmp->dlist + KMP_CKPH;
    }
    else if ( sect == KMP_ENPT )
    {
	ptlist = kmp->dlist + KMP_ENPT;
	phlist = kmp->dlist + KMP_ENPH;
    }
    else
    {
	DASSERT( sect == KMP_ITPT );
	ptlist = kmp->dlist + KMP_ITPT;
	phlist = kmp->dlist + KMP_ITPH;
    }

    if ( pt_index < 0 )
	pt_index += ptlist->used;
    if ( pt_index < 0 || pt_index >= ptlist->used )
    {
	if (ret_pt_index)
	    *ret_pt_index = -1;
	return not_found_val;
    }

    if (ret_pt_index)
	*ret_pt_index = pt_index;

    const kmp_enph_entry_t *ph = (kmp_enph_entry_t*)phlist->list;
    uint n = phlist->used;
    while ( pt_index >= ph->pt_start + ph->pt_len && n-- > 0 )
	ph++;
    if (ret_ph)
	*ret_ph = ph;

    noPRINT(">> %u %zu[%u+%u]\n", idx,
		ph-(kmp_enph_entry_t*)phlist->list,ph->pt_start,ph->pt_len);

    if ( pt_index == ph->pt_start + ph->pt_len - 1 )
    {
	// end of group reached
	if (ret_new_group)
	    *ret_new_group = true;

	pt_index = -1; // invalidate
	uint ph_idx = ~0;

	if ( sub_index >= 0 && sub_index < KMP_MAX_PH_LINK )
	{
	    ph_idx = ph->next[sub_index];
	    if (unique)
	    {
		uint i;
		for ( i = 0; i < sub_index; i++ )
		    if ( ph_idx == ph->next[i] )
		    {
			ph_idx = ~0;
			break;
		    }
	    }
	}
	else
	{
	    uint idx2;
	    for ( idx2 = 0; idx2 < KMP_MAX_PH_LINK; idx2++ )
	    {
		if (!IS_M1(ph->next[idx2]))
		{
		    ph_idx = ph->next[idx2];
		    if (ret_sub_index)
			*ret_sub_index = idx2;
		    break;
		}
	    }
	}

	if ( ph_idx < phlist->used )
	{
	    ph = (kmp_enph_entry_t*)phlist->list + ph_idx;
	    pt_index = ph->pt_start;
	}
    }
    else if ( unique && sub_index >= 1 && sub_index < KMP_MAX_PH_LINK )
	pt_index = -1;
    else if ( sub_index != -1 && sect == KMP_CKPT )
    {
	const kmp_ckpt_entry_t *pt = (kmp_ckpt_entry_t*)ptlist->list + pt_index;
	uint next = pt->next;
	pt_index = next < ptlist->used ? next : pt_index + 1;
    }
    else
	pt_index++;

    return pt_index >= 0 && pt_index < ptlist->used ? pt_index : not_found_val;
}

///////////////////////////////////////////////////////////////////////////////

const kmp_enph_entry_t * FindPH
(
    const kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP PT or PH section index
    int			pt_index,	// index of point, <0: add N
    int			* res_ph_index	// not NULL: store ph_index or -1 here
)
{
    DASSERT(kmp);

    const List_t *ptlist, *phlist;
    if ( sect == KMP_CKPT || sect == KMP_CKPH )
    {
	ptlist = kmp->dlist + KMP_CKPT;
	phlist = kmp->dlist + KMP_CKPH;
    }
    else if ( sect == KMP_ENPT || sect == KMP_ENPH )
    {
	ptlist = kmp->dlist + KMP_ENPT;
	phlist = kmp->dlist + KMP_ENPH;
    }
    else
    {
	DASSERT( sect == KMP_ITPT || sect == KMP_ITPH );
	ptlist = kmp->dlist + KMP_ITPT;
	phlist = kmp->dlist + KMP_ITPH;
    }

    if ( pt_index < 0 )
	pt_index += ptlist->used;
    if ( pt_index < 0 || pt_index >= ptlist->used )
    {
	if (res_ph_index)
	    *res_ph_index = -1;
	return 0;
    }

    const kmp_enph_entry_t *ph = (kmp_enph_entry_t*)phlist->list;
    uint i, n = phlist->used;
    for ( i = 0; i < n; i++, ph++ )
	if ( pt_index >= ph->pt_start && pt_index < ph->pt_start + ph->pt_len )
	{
	    if (res_ph_index)
		*res_ph_index = i;
	    return ph;
	}

    if (res_ph_index)
	*res_ph_index = -1;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

u16 AnalyseItembox ( Itembox_t *ibox, const kmp_gobj_entry_t *op )
{
    // initialze 'ibox' and returns 'ibox->obj_id'

    DASSERT(ibox);
    DASSERT(op);

    memset(ibox,0,sizeof(*ibox));
    if (op)
      switch (op->obj_id)
      {
	case 0x76: // s_itembox
	case 0xd4: // w_itembox
	case 0xd5: // w_itemboxline
	case 0xee: // sin_itembox
	    ibox->unsure_player	= true;
	    ibox->unsure_enemy	= true;
	    // fall through

	case 0x65: // itembox
	case 0xc9: // f_itembox
	    ibox->obj_id	= op->obj_id;
	    ibox->obj_flags	= ObjectInfo[op->obj_id].flags;
	    ibox->item_player	= op->setting[1];
	    ibox->item_enemy	= op->setting[2];
	    break;
      }

    return ibox->obj_id;
}

///////////////////////////////////////////////////////////////////////////////

void InsertLinkPH
(
    u8			* vector,	// insert into vector
    u8			link,		// link to insert
    ScanInfo_t		* si,		// not NULL: print error message
    ccp			name,		// NULL or name of line for error message
    bool		allow_dup	// true: allow multiple links into same section
)
{
    DASSERT(vector);

    uint i;
    for ( i = 0; i < KMP_MAX_PH_LINK; i++ )
    {
	if ( vector[i] == 0xff )
	{
	    vector[i] = link;
	    return;
	}
	if ( !allow_dup && vector[i] == link )
	    return;
    }

    if ( si && !si->no_warn )
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	ERROR0(ERR_WARNING,
		"To many links [%s @%u]: %s\n",
		sf->name, sf->line, name ? name : "?" );
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int kml_class_idx = 0;

void SetupClassNamePH
(
    kmp_ph_t		* ph		// valid PH data structure
)
{
    struct inttab_t { ccp name; int val; };
    static const struct inttab_t inttab[] =
    {
	{ "$OFF",	KCLS_OFF },
	{ "$ANY",	KCLS_ANY },
	{ "$DEFAULT",	KCLS_DEFAULT },
	{0,0},
    };

    if (ph->class_name.used)
	ResetParamField(&ph->class_name);

    const struct inttab_t *ptr;
    for ( ptr = inttab; ptr->name; ptr++ )
    {
	ParamFieldItem_t *it = FindInsertParamField(&ph->class_name,ptr->name,false,0);
	DASSERT(it);
	it->num = ptr->val;
    }
    kml_class_idx = KCLS_USER;

    ph->default_class = KCLS_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////

int InsertClassNamePH
(
    // return 1..254 if found or set, 255 if list is full, -1 on error

    kmp_ph_t		* ph,		// valid PH data structure
    ccp			cname,		// NULL or name of class
    bool		no_warn		// TRUE: suppress warning about full list
)
{
    DASSERT(ph);
    if ( !cname || !*cname )
	return -1;

    if (!ph->class_name.used)
	SetupClassNamePH(ph);

    bool old_found;
    ParamFieldItem_t *it = FindInsertParamField(&ph->class_name,cname,false,&old_found);
    DASSERT(it);
    if (!old_found)
    {
	it->num = kml_class_idx++;
	if ( it->num > MAX_CLASS_NAMES )
	{
	    it->num = kml_class_idx = MAX_CLASS_NAMES;
	    if (!no_warn)
		ERROR0(ERR_WARNING,
		    "Class name list is full (%u names), can't add: %s\n",
			MAX_CLASS_NAMES, cname );
	}
    }
    return it->num;
}

///////////////////////////////////////////////////////////////////////////////

int FindClassNamePH
(
    // return -1, if not found, 1..254 else

    kmp_ph_t		* ph,		// valid PH data structure
    ccp			cname		// NULL or name of class
)
{
    DASSERT(ph);
    ParamFieldItem_t *it = cname ? FindParamField(&ph->class_name,cname) : 0;
    return it ? it->num : -1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			RenameGroupKMP()		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct group_t
{
    u8  done;
    u8	dispatch;
    u8  index;
    u8  subindex;
    u8	order;
}
group_t;

///////////////////////////////////////////////////////////////////////////////

static u64 IterateGroup
(
    const kmp_enph_entry_t	* elist,
    group_t			* grp,
    uint			idx,
    uint			n,
    uint			last_index,
    u64				steps
)
{
    DASSERT(elist);
    DASSERT(grp);
    DASSERT(idx<n);
    noPRINT("IterateGroup(%p,%p,%u,%u,%u,%u)\n",elist,grp,idx,n,last_index,steps);

    if ( steps < 1000000 ) // limit steps avoid nearly endless loops
    {
	noPRINT_IF( !(steps % 10000000), "  steps=%9llu M\n", steps/1000000);
	steps++;
	last_index++;
	const kmp_enph_entry_t * entry = elist + idx;

	uint ni;
	for ( ni=0; ni < KMP_MAX_PH_LINK; ni++ )
	{
	    const uint next = entry->next[ni];
	    if ( next < n )
	    {
		group_t * g = grp + next;
		if ( !g->done && g->index < last_index )
		{
		    g->index = last_index;
		    g->done++;
		    noPRINT(" -> ni=%u, next=%u/%u, last=%u\n",ni,next,n,last_index);
		    steps = IterateGroup(elist,grp,next,n,last_index,steps);
		    g->done--;
		}
	    }
	}
    }
    return steps;
}

///////////////////////////////////////////////////////////////////////////////

void RenameGroupKMP
(
    kmp_t		* kmp,		// KMP data structure
    uint		sect_ph,	// ph section index
    kmp_group_name_t	* gname,	// pointer to group name array
    bool		enable_dp	// enable dispatch point support
)
{
    DASSERT(kmp);
    DASSERT( sect_ph == KMP_CKPH || sect_ph == KMP_ENPH || sect_ph == KMP_ITPH );
    DASSERT(gname);

    const List_t *phlist = kmp->dlist + sect_ph;
    const uint n = phlist->used < KMP_MAX_GROUP ? phlist->used : KMP_MAX_GROUP;
    memset(gname,0,sizeof(*gname)*KMP_MAX_GROUP);

    uint last_index = 0;
    group_t grp[KMP_MAX_GROUP];
    memset(grp,0,sizeof(grp));

    const kmp_enph_entry_t *elist = (kmp_enph_entry_t*)(phlist->list);


    //--- setup dispatch points

    uint dp_fw = 0;
    if (enable_dp)
    {
	uint gi, dp = 0;
	group_t *gp;
	const kmp_enph_entry_t *p = elist;
	for ( gi = 0, gp = grp; gi < n; gi++, gp++, p++ )
	    if (IsDispatchPointKMP(p))
	    {
		gp->done++;
		gp->dispatch = ++dp;
	    }
	dp_fw = snprintf(*gname,sizeof(*gname),"%u",dp);
    }


    //--- find index number

    for(;;)
    {
	//--- find first non named group

	uint g1 = 0;
	while ( g1 < n && grp[g1].done )
	    g1++;
	if ( g1 == n )
	    break;

	grp[g1].done = 1;
	grp[g1].index = ++last_index;
 #if HAVE_PRINT0
	const uint steps = IterateGroup(elist,grp,g1,n,last_index,0);
	PRINT(" SECT=%s, steps=%8u\n",kmp_section_name[sect_ph].name1,steps);
 #else
	IterateGroup(elist,grp,g1,n,last_index,0);
 #endif

	uint gi;
	group_t * gp;
	for ( gi = 0, gp = grp; gi < n; gi++, gp++ )
	    if (gp->index)
	    {
		gp->done = 1;
		if ( last_index < gp->index )
		     last_index = gp->index;
	    }
    }


    //--- calc subindex

    u8 count[KMP_MAX_GROUP];
    memset(count,0,sizeof(count));

    uint gi;
    group_t *gp;
    for ( gi = 0, gp = grp; gi < n; gi++, gp++ )
	gp->subindex = count[gp->index]++;


    //--- print names

    const int fw = snprintf(*gname,sizeof(*gname),"%u",last_index);
    for ( gi = 0, gp = grp; gi < n; gi++, gp++, gname++ )
    {
	if (gp->dispatch)
	    snprintf( *gname,sizeof(*gname), "DP%0*u", dp_fw, gp->dispatch );
	else
	{
	    uint nc = count[gp->index];
	    if ( nc <= 1 )
		snprintf(*gname,sizeof(*gname), "G%0*u", fw,gp->index );
	    else if ( nc <= 26 )
		snprintf(*gname,sizeof(*gname),
			"G%0*u%c", fw,gp->index, 'A' + gp->subindex );
	    else
		snprintf(*gname,sizeof(*gname),
			"G%0*u%c%c", fw,gp->index,
			'A' + gp->subindex/26, 'A' + gp->subindex%26 );
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP pathes			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kmp_link_t
{
    u8	cur;		// item index of current point
    u8	next;		// item index of next point
    u8	grp_cur;	// group index of current point
    u8	grp_next;	// group index of next point
}
kmp_link_t;

//-----------------------------------------------------------------------------

typedef struct kmp_path_t
{
    uint	sect_ph;			// analysed PH section
    uint	sect_pt;			// analysed PT section

    uint	n_ph;				// number of groups
    uint	n_pt;				// number of items

    group_t	grp[KMP_MAX_GROUP];		// group_t data
    u8		grp_order[KMP_MAX_GROUP];	// order => index into 'grp'
    u8		itm_grp[KMP_MAX_POINT];		// group of each item

    kmp_link_t	*link;				// allocated list of links
    uint	n_link;				// number of element in 'link'
}
kmp_path_t;

///////////////////////////////////////////////////////////////////////////////

static inline void InitializeKmpPath ( kmp_path_t *path )
{
    DASSERT(path);
    memset(path,0,sizeof(*path));
}

///////////////////////////////////////////////////////////////////////////////

static void ResetKmpPath ( kmp_path_t *path )
{
    DASSERT(path);
    if (path->link)
	FREE(path->link);
    InitializeKmpPath(path);
}

///////////////////////////////////////////////////////////////////////////////

static void AnalyzeKmpPath
(
    const kmp_t		*kmp,		// KMP data structure
    uint		sect_ph,	// ph section index
    kmp_path_t		*path,		// store group list here
    bool		init_path,	// true: initialize 'path'
    uint		first_grp	// index of first group (robust)
)
{
 #if HAVE_PRINT
    u_usec_t start_time = GetTimerUSec();
 #endif

    DASSERT(kmp);
    DASSERT( sect_ph == KMP_CKPH || sect_ph == KMP_ENPH || sect_ph == KMP_ITPH );
    DASSERT(path);

    if (init_path)
	InitializeKmpPath(path);
    else
	ResetKmpPath(path);

    DASSERT( KMP_CKPT+1 == KMP_CKPH );
    DASSERT( KMP_ENPT+1 == KMP_ENPH );
    DASSERT( KMP_ITPT+1 == KMP_ITPH );
    path->sect_ph = sect_ph;
    path->sect_pt = sect_ph - 1;

    const List_t *phlist = kmp->dlist + sect_ph;
    const uint n_ph = phlist->used < KMP_MAX_GROUP ? phlist->used : KMP_MAX_GROUP;
    const kmp_enph_entry_t * elist = (kmp_enph_entry_t*)(phlist->list);

    const List_t *ptlist = kmp->dlist + path->sect_pt;
    const uint n_pt = ptlist->used < KMP_MAX_POINT ? ptlist->used : KMP_MAX_POINT;

    uint last_index = 0;
    path->n_ph = n_ph;
    path->n_pt = n_pt;

    for(;;)
    {
	if ( first_grp >= n_ph )
	    first_grp = 0;
	PRINT("GRP: ---- %u ---\n",first_grp);

	for(;;)
	{
	    uint g1 = first_grp;
	    while ( g1 < n_ph && path->grp[g1].done )
		g1++;
	    if ( g1 == n_ph )
		break;

	    path->grp[g1].done = 1;
	    path->grp[g1].index = ++last_index;
	 #if HAVE_PRINT0
	    const uint steps = IterateGroup(elist,path->grp,g1,n_ph,last_index,0);
	    PRINT(" SECT=%s, steps=%8u\n",kmp_section_name[sect_ph].name1,steps);
	 #else
	    IterateGroup(elist,path->grp,g1,n_ph,last_index,0);
	 #endif

	    uint gi;
	    group_t * gp;
	    for ( gi = 0, gp = path->grp; gi < n_ph; gi++, gp++ )
	    {
		if (gp->index)
		{
		    gp->done = 1;
		    if ( last_index < gp->index )
			 last_index = gp->index;
		    noPRINT("GRP %3u: %u.%u\n",gi,gp->index,gp->subindex);
		}
	    }
	}

	if (!first_grp)
	    break;
	first_grp = 0;
    }


    //--- calc subindex

    u8 count[KMP_MAX_GROUP];
    memset(count,0,sizeof(count));

    uint gi;
    group_t *gp;
    for ( gi = 0, gp = path->grp; gi < n_ph; gi++, gp++ )
    {
	gp->subindex = count[gp->index]++;
	noPRINT("GRP %3u: %u.%u\n",gi,gp->index,gp->subindex);
    }


    //--- sort groups

    u8 index[KMP_MAX_GROUP];
    memset(index,0,sizeof(index));
    for ( gi = 1; gi < n_ph; gi++ )
	index[gi] = index[gi-1] + count[gi-1];

    for ( gi = 0, gp = path->grp; gi < n_ph; gi++, gp++ )
    {
	gp->done = 0; // clear done for further processing
	gp->order = index[gp->index] + gp->subindex;
	noPRINT("GRP %3u: %3u => %u.%u\n",gi,gp->grp_order,gp->index,gp->subindex);
	path->grp_order[gp->order] = gi;
    }


    //--- setup item lists

    kmp_link_t item[KMP_MAX_PH_LINK*(KMP_MAX_GROUP+2)];
    kmp_link_t *ip = item, *iend = (kmp_link_t*)((u8*)item + sizeof(item)) - KMP_MAX_PH_LINK;
    memset(item,0,sizeof(item));
    memset(path->itm_grp,0xff,sizeof(path->itm_grp));

    uint i;
    for ( i = 0; i < n_ph; i++ )
    {
	uint gi = path->grp_order[i];

	const kmp_enph_entry_t *ep = elist + gi;
	uint idx = ep->pt_start;
	uint end = idx + ep->pt_len;
	if ( end > n_pt )
	     end = n_pt;
	end--;
	for ( ; idx <= end && ip < iend; idx++ )
	{
	    if (path->itm_grp[idx] == 0xff )
	    {
		path->itm_grp[idx] = gi;
		if ( idx == end )
		{
		    // end of group -> multi links
		    uint link;
		    for ( link = 0; link < KMP_MAX_PH_LINK; link++ )
		    {
			int next = GetNextPointKMP(kmp,path->sect_pt,idx,link,true,-1,0,0,0,0);
			if ( next >= 0 )
			{
			    ip->cur      = idx;
			    ip->next     = next;
			    ip->grp_cur	 = gi;
			    ip++;
			}
		    }
		}
		else
		{
		    ip->cur      = idx;
		    ip->next     = idx + 1;
		    ip->grp_cur	 = gi;
		    ip++;
		}
	    }
	}
    }


    //--- calculate 'next group'

    kmp_link_t *jp;
    for ( jp = item; jp < ip; jp++ )
	jp->grp_next = path->itm_grp[jp->next];


    //--- add points not handled by a group

    for ( i = 0; i < n_pt && ip < iend; i++ )
	if ( path->itm_grp[i] == 0xff )
	{
	    ip->cur      = i;
	    ip->next     = i + 1;
	    ip->grp_cur	 = 0xff;
	    ip->grp_next = 0xff;
	    ip++;
	}

    path->n_link = ip - item;
    path->link = MEMDUP(item,path->n_link*sizeof(*item));


    //--- debugging

    PRINT("AnalyzeKmpPath(%s) done in %llu µsec\n",
		kmp_section_name[sect_ph].name1, GetTimerUSec()-start_time);

    #if HAVE_PRINT0
    {
	PRINT("----- GROUP ORDER '%s' -----\n",kmp_section_name[sect_ph].name1);
	uint i;
	for ( i = 0; i < n_ph; i++ )
	{
	    uint gi = path->grp_order[i];
	    const group_t *gp = path->grp + gi;
	    const kmp_enph_entry_t *ep = elist + gi;

	    PRINT("%3u. GRP %3u: %3u.%u : %3u .. %3u [+%3u]\n",
			gp->order, gi, gp->index, gp->subindex,
			ep->pt_start, ep->pt_start + ep->pt_len-1, ep->pt_len );
	}
    }
    #endif


    #if HAVE_PRINT0
    {
	PRINT("----- ITEM ORDER '%s' -----\n",kmp_section_name[sect_ph].name1);
	uint i;
	for ( i = 0; i < path->n_link; i++ )
	{
	    const kmp_link_t *link = path->link + i;
	    PRINT("%3u. ITM %3u -> %3u : %2u -> %3u\n",
			i, link->cur, link->next, link->grp_cur, link->grp_next );
	}
    }
    #endif
}

///////////////////////////////////////////////////////////////////////////////

const kmp_link_t * FindCKPT
(
    const kmp_t		*kmp,		// KMP data structure
    kmp_path_t		*ck_path,	// ckeck point path data, already initialized
					//	data is calculated in first call
    float		x,		// x-coordiante of point
    float		z,		// y-coordiante of point
    const kmp_link_t	*start_link	// NULL or start link
)
{
    DASSERT(kmp);
    DASSERT(ck_path);

    const List_t *ptlist = kmp->dlist + KMP_CKPT;
    const uint n_pt = ptlist->used < KMP_MAX_POINT ? ptlist->used : KMP_MAX_POINT;
    if ( n_pt < 2 )
	return 0;

    if ( !ck_path->n_link || ck_path->n_pt != n_pt || ck_path->sect_ph != KMP_CKPH )
    {
	AnalyzeKmpPath(kmp,KMP_CKPH,ck_path,false,0);
	start_link = 0;
    }
    if (!start_link)
	start_link = ck_path->link;
    const kmp_link_t *link, *endlink = ck_path->link + ck_path->n_link;

    float pts[2];
    pts[0] = x;
    pts[1] = z;

    link = start_link;
    const kmp_ckpt_entry_t *pt_base = (kmp_ckpt_entry_t*)ptlist->list;

    for(;;)
    {
	DASSERT( link->cur < n_pt );
	DASSERT( link->next < n_pt );
	float quad[8];

	const kmp_ckpt_entry_t *pt1 = pt_base + link->cur;
	quad[0] = pt1->left[0];
	quad[1] = pt1->left[1];
	quad[2] = pt1->right[0];
	quad[3] = pt1->right[1];

	const kmp_ckpt_entry_t *pt2 = pt_base + link->next;
	quad[4] = pt2->right[0];
	quad[5] = pt2->right[1];
	quad[6] = pt2->left[0];
	quad[7] = pt2->left[1];

	int dir;
	if ( PointsInConvexPolygonF(pts,1,quad,4,true,&dir) && dir >= 0 )
	    return link;

	if ( ++link == endlink )
	    link = ck_path->link;
	if ( link == start_link )
	    return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

int CheckConvexCKPT
(
    // returns
    //  -1: illegal index
    //   0: convex
    //   1: concave

    const kmp_t		*kmp,		// KMP data structure
    uint		ckpt1,		// index of first check point (robust)
    uint		ckpt2,		// index of second check point (robust)
    double		*min_distance	// not NULL: store minimum distance here
)
{
    DASSERT(kmp);

    int stat = -1;
    double min_dist = INFINITY;

    const List_t *ptlist = kmp->dlist + KMP_CKPT;
    const uint n_pt = ptlist->used;

    if ( ckpt1 < n_pt && ckpt2 < n_pt )
    {
	float p[5][2];

	const kmp_ckpt_entry_t *pt1 = (kmp_ckpt_entry_t*)ptlist->list + ckpt1;
	p[0][0] = p[4][0] = pt1->left[0];
	p[0][1] = p[4][1] = pt1->left[1];
	p[1][0] = pt1->right[0];
	p[1][1] = pt1->right[1];

	const kmp_ckpt_entry_t *pt2 = (kmp_ckpt_entry_t*)ptlist->list + ckpt2;
	p[2][0] = pt2->right[0];
	p[2][1] = pt2->right[1];
	p[3][0] = pt2->left[0];
	p[3][1] = pt2->left[1];

	double dir[5];
	uint j;
	for ( j = 0; j < 4; j++ )
	{
	    noPRINT(" - p: [%8.1f,%8.1f] [%8.1f,%8.1f]\n",
			p[j][0], p[j][1], p[j+1][0], p[j+1][1] );
	    const double dx = p[j+1][0] - p[j][0];
	    const double dz = p[j+1][1] - p[j][1];
	    dir[j] = atan2(dx,dz) * (180.0/M_PI);
	    const double len = dx*dx + dz*dz;
	    if ( min_dist > len )
		 min_dist = len;
	    noPRINT(" - DIR: %6.1f %9.1f [%8.1f,%8.1f]\n",
			dir[j], len, dx, dz );
	}
	dir[4] = dir[0];

	stat = 0;
	for ( j = 0; j < 4; j++ )
	{
	    double delta = dir[j+1] - dir[j];
	    if ( delta < 0.0 )
		delta += 360.0;
	    else if ( delta > 360.0 )
		delta -= 360.0;

	    if ( delta > 180.0 )
	    {
		stat = 1;
		break;
	    }
	}
    }

    if (min_distance)
	*min_distance = sqrt(min_dist);
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    KMP data			///////////////
///////////////////////////////////////////////////////////////////////////////

const uint kmp_entry_size[KMP_N_SECT+1] =
{
	sizeof(kmp_ktpt_entry_t),
	sizeof(kmp_enpt_entry_t),
	sizeof(kmp_enph_entry_t),
	sizeof(kmp_itpt_entry_t),
	sizeof(kmp_itph_entry_t),
	sizeof(kmp_ckpt_entry_t),
	sizeof(kmp_ckph_entry_t),
	sizeof(kmp_gobj_entry_t),
	sizeof(kmp_poti_group_t),
	sizeof(kmp_area_entry_t),
	sizeof(kmp_came_entry_t),
	sizeof(kmp_jgpt_entry_t),
	sizeof(kmp_cnpt_entry_t),
	sizeof(kmp_mspt_entry_t),
	sizeof(kmp_stgi_entry_t),
	0
};

const KeywordTab_t kmp_section_name[] =
{
	{ KMP_KTPT,	"KTPT",	"Start position", 0 },
	{ KMP_ENPT,	"ENPT",	"Enemy route point", 0 },
	{ KMP_ENPH,	"ENPH",	"Enemy route group", 0 },
	{ KMP_ITPT,	"ITPT",	"Item route point", 0 },
	{ KMP_ITPH,	"ITPH",	"Item route group", 0 },
	{ KMP_CKPT,	"CKPT",	"Check point", 0 },
	{ KMP_CKPH,	"CKPH",	"Check point group", 0 },
	{ KMP_GOBJ,	"GOBJ",	"Global object", 0 },
	{ KMP_POTI,	"POTI",	"Route", 0 },
	{ KMP_AREA,	"AREA",	"Area", 0 },
	{ KMP_CAME,	"CAME",	"Camera", 0 },
	{ KMP_JGPT,	"JGPT",	"Respawn point", 0 },
	{ KMP_CNPT,	"CNPT",	"Canon point", 0 },
	{ KMP_MSPT,	"MSPT",	"Battle position", 0 },
	{ KMP_STGI,	"STGI",	"Stage info", 0 },

	{ KMP_N_SECT,	"SETUP", 0, 1  },
	{ KMP_WIM0,	"WIM0",  0, 2  },
	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t kmp_rtype_name[] =
{
	{ KMP_RT_AUTO,		"AUTO",		"A",	0 },
	{ KMP_RT_ROUTE,		"ROUTE",	"R",	0 },
	{ KMP_RT_DISPATCH,	"DISPATCH",	"D",	0 },
	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t kmp_oneway_name[] =
{
	{ ONEWAY_NONE,	"NONE",	"OFF",	0 },
	{ ONEWAY_PREV,	"PREV",	0,	0 },
	{ ONEWAY_NEXT,	"NEXT",	0,	0 },
	{ ONEWAY_BOTH,	"BOTH",	"ALL",	0 },
	{ 0,0,0,0 }
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    KMP setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeKMP ( kmp_t * kmp )
{
    static bool done = false;
    if (!done)
    {
	// do this only once
	done = true;
	SetKmpMode(KMP_MODE);
	KMP_ACTION_LOG(false,"Global KMP Modes: %s\n",GetKmpMode());
	noPRINT("Global KMP Modes: %s\n",GetKmpMode());
    }

    DASSERT(kmp);
    memset(kmp,0,sizeof(*kmp));

    kmp->fname = EmptyString;
    kmp->nowarn_format = verbose <= -3;

    uint sect;
    for ( sect = 0; sect < KMP_N_SECT; sect++ )
    {
	InitializeList( kmp->dlist+sect, kmp_entry_size[sect] );
	InitializeIL( kmp->index+sect, 0 );
    }
    InitializeList( &kmp->poti_point, sizeof(kmp_poti_point_t) );

    kmp->index[KMP_GOBJ].prefix = "o";
    kmp->index[KMP_POTI].prefix = "r";
    kmp->index[KMP_AREA].prefix = "a";
    kmp->index[KMP_CAME].prefix = "c";
    kmp->index[KMP_JGPT].prefix = "j";
    kmp->index[KMP_CNPT].prefix = "n";
    kmp->index[KMP_MSPT].prefix = "m";
    kmp->index[KMP_STGI].prefix = "s";

    kmp->revision		= REVISION_NUM;
    kmp->kmp_version		= 0x9d8;

    kmp->rtobj.obj		= GOBJ_OBAKE_BLOCK;
    kmp->rtobj.scale.x		= 0.2;
    kmp->rtobj.scale.y		= 0.4;
    kmp->rtobj.scale.z		= 1.0;

    kmp->ckpt_object1		= RTOBJ_1;
    kmp->ckpt_object2		= RTOBJ_2;
    kmp->ckpt_object1_base	= RTOBJ_BASE;
    kmp->ckpt_object2_base	= RTOBJ_BASE;

    kmp->jgpt_auto_distance	= 1000.0;
    kmp->jgpt_auto_adjust.x	=    0.0;
    kmp->jgpt_auto_adjust.y	=  200.0;
    kmp->jgpt_auto_adjust.z	=    0.0;

    InitializePH( &kmp->ckph, KMP_CKPH, KMP_CKPT, KMP_AC_PREV );
    InitializePH( &kmp->enph, KMP_ENPH, KMP_ENPT, KMP_AC_PREV );
    InitializePH( &kmp->itph, KMP_ITPH, KMP_ITPT, KMP_AC_PREV | KMP_ACF_FIX );

    InitializeKMPFlags( &kmp->enpt_flag, KPFL_M_ENPT );
    InitializeKMPFlags( &kmp->itpt_flag, KPFL_M_ITPT );
    InitializeKMPFlags( &kmp->jgpt_flag, KPFL_M_JGPT );

    if ( opt_wim0 >= OFFON_ON )
	kmp->wim0_export	= 1;

    kmp->enable_fall_kcl	= ( KCL_MODE & KCLMD_ADD_ROAD ) != 0;

    #if HAVE_PRINT0
    {
	char buf[20];
	int i;
	for ( i = 1; i < 9; i++ )
	{
	    snprintf(buf,sizeof(buf),"CNAME-%03u",i);
	    int f1 = FindClassNamePH(&kmp->ckph,buf);
	    int i1 = InsertClassNamePH(&kmp->ckph,buf,false);
	    int f2 = FindClassNamePH(&kmp->ckph,buf);
	    printf(" %s : %3d %3d %3d\n",buf,f1,i1,f2);
	}
    }
    #endif
}

///////////////////////////////////////////////////////////////////////////////

void ClearSectionsKMP ( kmp_t * kmp )
{
    DASSERT(kmp);

    uint sect;
    for ( sect = 0; sect < KMP_N_SECT; sect++ )
    {
	ResetList(kmp->dlist+sect);
	ResetIL(kmp->index+sect);
    }
    ResetList(&kmp->poti_point);
}

///////////////////////////////////////////////////////////////////////////////

void ResetKMP ( kmp_t * kmp )
{
    DASSERT(kmp);
    UnloadAutoReferenceKCL();
    FreeString(kmp->fname);
    FreeString(kmp->info);
    ClearSectionsKMP(kmp);
    FREE(kmp->raw_data);
    FREE(kmp->linfo);
    FREE(kmp->wim0_data);

    ResetPH(&kmp->ckph);
    ResetPH(&kmp->enph);
    ResetPH(&kmp->itph);

    if (kmp->fall_kcl)
    {
	if ( kcl_ref_prio == kmp->fall_kcl )
	    kcl_ref_prio = 0;
	ResetKCL(kmp->fall_kcl);
	FREE(kmp->fall_kcl);
    }

    InitializeKMP(kmp);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct kmp_ph_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializePH ( kmp_ph_t *ph, uint sect_ph, uint sect_pt, uint ac_default )
{
    DASSERT(ph);
    memset(ph,0,sizeof(*ph));

    ph->sect_ph		= sect_ph;
    ph->sect_pt		= sect_pt;
    ph->show_options	= opt_route_options >= OFFON_ON;
    ph->default_class	= KCLS_DEFAULT;
    ph->ac_mode		= ac_default & KMP_AC_MASK;
    ph->ac_flags	= ac_default & KMP_ACF_MASK;
    ph->mask[0]		= ~0;
    ph->mask[1]		= ~0;

    ph->oneway_used	= 0;
    ph->oneway_size	= 0;
    ph->oneway_list	= 0;

    InitializeParamField(&ph->class_name);
}

///////////////////////////////////////////////////////////////////////////////

void ResetPH ( kmp_ph_t *ph )
{
    DASSERT(ph);
    FREE(ph->oneway_list);
    ResetParamField(&ph->class_name);
    InitializePH(ph,ph->sect_ph,ph->sect_pt,ph->ac_mode|ph->ac_flags);
}

///////////////////////////////////////////////////////////////////////////////

void InsertOnewayPH ( kmp_ph_t *ph, u8 from, u8 to )
{
    DASSERT(ph);
    DASSERT( ph->oneway_used <= ph->oneway_size );

    if (!FindOnewayPH(ph,from,to))
    {
	if ( ph->oneway_used == ph->oneway_size )
	{
	    ph->oneway_size += 100;
	    ph->oneway_list = REALLOC(ph->oneway_list,
				      ph->oneway_size * sizeof(*ph->oneway_list));
	}
	DASSERT( ph->oneway_used < ph->oneway_size );
	DASSERT(ph->oneway_list);
	ph->oneway_list[ph->oneway_used++] = from << 8 | to;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool FindOnewayPH ( const kmp_ph_t *ph, u8 from, u8 to )
{
    DASSERT(ph);
    DASSERT( ph->oneway_used <= ph->oneway_size );

    if (ph->oneway_used)
    {
	DASSERT(ph->oneway_list);
	const u16  val = from << 8 | to;
	const u16 *ptr = ph->oneway_list;
	const u16 *end = ptr + ph->oneway_used;
	while ( ptr < end )
	    if ( *ptr++ == val )
		return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static int SortOnewayPH_helper ( const void *a, const void *b )
{
    return *(u16*)a - *(u16*)b;
}

//-----------------------------------------------------------------------------

void SortOnewayPH ( const kmp_ph_t *ph )
{
    DASSERT(ph);
    if ( ph->oneway_used > 1 )
	qsort( ph->oneway_list, ph->oneway_used, sizeof(*ph->oneway_list),
		SortOnewayPH_helper );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct kmp_flag_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeKMPFlags ( kmp_flag_t * kf, u8 mask )
{
    DASSERT(kf);
    memset(kf,0,sizeof(*kf));

    kf->export = opt_export_flags >= OFFON_ON;

    kf->mask = KPFL_M_ALL & mask;
    memset( kf->flags, kf->mask, sizeof(kf->flags) );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AnalyseSectionKMP()		///////////////
///////////////////////////////////////////////////////////////////////////////

bool AnalyseSectionKMP
(
    // return true if known section found

    kmp_section_t	*ks,	// store result here
    const void		*data,	// KMP raw data to scan
    uint		size,	// size of 'data'
    uint		index	// index of section to analyse
)
{
    DASSERT(ks);
    memset(ks,0,sizeof(*ks));
    ks->sect  = -1; // invalid
    ks->index = index;

    if ( !data || size < sizeof(kmp_file_gen_t) )
	return false;

    const kmp_file_gen_t *kmp = data;
    if ( memcmp(kmp->magic,"RKMD",sizeof(kmp->magic)))
	return false;

    const uint base_off	= ntohs(kmp->head_size);
    const uint n_sect	= ntohs(kmp->n_sect);
    if ( base_off < GetFileHeadSizeKMP(n_sect) || base_off >= size || index >= n_sect )
	return false;

    const uint sect_off = base_off + ntohl(kmp->sect_off[index]);
    if ( sect_off < base_off || sect_off > size - sizeof(kmp_list_head_t) )
	return false;


    //--- find section type

    kmp_list_head_t *sect_ptr = (kmp_list_head_t*)(data+sect_off);
    ks->ptr  = sect_ptr;

    const KeywordTab_t *kp;
    for ( kp = kmp_section_name; kp->name1; kp++ )
	if (!memcmp(sect_ptr->magic,kp->name1,4))
	    break;
    if (!kp->name1)
	return -2;
    ks->sect = kp->id;


    //--- analyse length

    uint sect_size = sizeof(kmp_list_head_t)
		   + ntohs(sect_ptr->n_entry) * kmp_entry_size[ks->sect];
    if ( ks->sect == KMP_POTI )
	 sect_size += sizeof(kmp_poti_point_t) * ntohs(sect_ptr->value);


    //--- check limit

    uint next_off = base_off
		+ ( index+1 < n_sect ? ntohl(kmp->sect_off[index+1]) : size );
    if ( next_off > size )
	next_off = size;
    if ( sect_size > next_off - sect_off )
	return false;

    ks->size = sect_size;
    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP: scan helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static const void * get_kmp_pointer
(
    // return pointer to first element, or NULL if not found

    uint		* res_n,	// not NULL: store number of entries
    uint		* res_val,	// not NULL: store 'unknown value'
    uint		* res_total,	// not NULL: store total section size incl header

    kmp_t		* kmp,		// KMP data structure
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    uint		sect		// KMP section index
)
{
    DASSERT(kmp);
    DASSERT(data);
    DASSERT( sect < KMP_N_SECT || sect == KMP_WIM0 );

    kmp_head_info_t hi;
    ScanHeadInfoKMP(&hi,data,data_size);

    ccp sect_name		= kmp_section_name[sect].name1;
    const kmp_list_head_t *list	= 0;

    if ( sect < hi.n_sect )
    {
	// optimization: try standard slot
	const uint off = ntohl(hi.sect_off[sect]);
	if ( off <= hi.max_off && !(off&3) )
	{
	    list = (kmp_list_head_t*) ( data + hi.head_size + off );
	    if (memcmp(list->magic,sect_name,sizeof(list->magic)))
		list = 0;
	}
    }

    if (!list)
    {
	// not found yet: try iteration

	noPRINT("'%s' FAILED, TRY ITERATION, ns=%d, maxoff=%x\n",
		sect_name, n_sect, hi.max_off );

	int s;
	for ( s = -1; s < hi.n_sect; s++ )
	{
	    const uint off = ntohl(hi.sect_off[s]);
	    noPRINT("%3d: %x/%x\n",s,off,hi.max_off);
	    if ( off > hi.max_off || off&3 )
		continue;

	    const kmp_list_head_t *temp
		= (kmp_list_head_t*) ( data + hi.head_size + off );
	    if (!memcmp(temp->magic,sect_name,sizeof(temp->magic)))
	    {
		list = temp;
		break;
	    }
	}

	if (!list)
	{
	    if ( sect != KMP_WIM0 && !kmp->nowarn_format )
		ERROR0(ERR_WARNING,
			"KMP: Section [%s] not found: %s\n",
			kmp_section_name[sect].name1, kmp->fname );
	    if (res_n)     *res_n = 0;
	    if (res_val)   *res_val = 0;
	    if (res_total) *res_total = 0;
	    return 0;
	}

	noPRINT("'%s' FOUND AT SLOT %u\n",sect_name,s);
    }

    if ( sect == KMP_WIM0 )
    {
	const uint size = be32(&list->n_entry);
	if (res_n)     *res_n     = size;
	if (res_val)   *res_val   = 0;
	if (res_total) *res_total = size + sizeof(*list);
	return list->entry;
    }

    kmp->value[sect] = be16(&list->value);
    const u8 * elem = list->entry;
    uint n = be16(&list->n_entry);
    uint max = ( (u8*)data + data_size - elem ) / kmp_entry_size[sect];
    if ( n > max )
    {
	ERROR0(ERR_WARNING,
		"Number of [%s] entries exceed file size (%u>%u): %s\n",
		kmp_section_name[sect].name1, n, max, kmp->fname );
	n = max;
    }
    if (res_n)
	*res_n = n;
    if (res_val)
	*res_val = be16(&list->value);

    const u8 *end = elem + n * kmp_entry_size[sect];
    if ( kmp->max_scanned < end )
	 kmp->max_scanned = end;

    if (res_total)
    {
	*res_total = sizeof(kmp_list_head_t) + n * kmp_entry_size[sect];
	if ( sect == KMP_POTI )
	    *res_total += sizeof(kmp_poti_point_t) * be16(&list->value);
    }
    return elem;
}

///////////////////////////////////////////////////////////////////////////////

static uint test_unknown_sections
(
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname,		// file name for error messages
    bool		*res_shrinked	// not NULL: store status about shrinked KMP
)
{
    kmp_head_info_t hi;
    ScanHeadInfoKMP(&hi,data,data_size);

    const bool is_shrinked = hi.n_sect < KMP_N_SECT;
    if (res_shrinked)
	*res_shrinked = is_shrinked;
    if ( is_shrinked )
	ERROR0(ERR_WARNING,
		"KMP: Is shrinked => some sections missed, some moved: %s\n",
		fname );

    uint err_count = 0;
    int s;
    for ( s = -1; s < hi.n_sect; s++ )
    {
	const uint off = ntohl(hi.sect_off[s]);
	noPRINT("%3d: %x/%x\n",s,off,hi.max_off);
	if ( off > hi.max_off || off&3 )
	    continue;

	ccp expected_sect_name = s < 0 || s >= KMP_N_SECT
		? "----" : kmp_section_name[s].name1;
	const kmp_list_head_t *temp
	    = (kmp_list_head_t*) ( data + hi.head_size + off );
	if (memcmp(temp->magic,expected_sect_name,sizeof(temp->magic)))
	{
	    uint i;
	    for ( i = 0; i < KMP_N_SECT; i++ )
	    {
		const uint off2 = ntohl(hi.sect_off[i]);
		if ( off2 > hi.max_off || off2&3 )
		    continue;
		const kmp_list_head_t * temp2
			= (kmp_list_head_t*) ( data + hi.head_size + off2 );
		if (!memcmp(temp2->magic,expected_sect_name,sizeof(temp2->magic)))
		    break;
	    }

	    if ( i < KMP_N_SECT && !is_shrinked )
		ERROR0(ERR_WARNING,
		    "KMP: Section [%s] found at index %u, but not at %d: %s\n",
		    expected_sect_name, i, s, fname );
	    err_count++;
	}
    }
    return err_count;
}

///////////////////////////////////////////////////////////////////////////////

static void scan_kmp_pt
(
    kmp_t		* kmp,		// KMP data structure
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    uint		sect		// KMP section index
)
{
    DASSERT( sect == KMP_ENPT || sect == KMP_ITPT );

//X    kmp_ph_t *ph;
    kmp_flag_t *kf;
    if ( sect == KMP_ENPT )
    {
//X	ph = &kmp->enph;
	kf = &kmp->enpt_flag;
    }
    else
    {
//X	ph = &kmp->itph;
	kf = &kmp->itpt_flag;
    }

    uint n;
    const kmp_enpt_entry_t * pt
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,sect);

    u32 flags_or  =  0;
    u32 flags_and = ~0;
    const uint max_import = sizeof(kf->flags)/sizeof(*kf->flags);

    uint i;
    for ( i = 0; i < n; i++, pt++ )
    {
	kmp_enpt_entry_t * item = AppendList(kmp->dlist+sect);
	DASSERT(item);
	bef4n(item->position,pt->position,4);
	be16n(item->prop,pt->prop,2);

	if ( i < max_import )
	{
	    flags_or  |= *(u32*)&item->scale;
	    flags_and &= *(u32*)&item->scale;
	}
    }


    //--- import flags?

    const bool import = n > 1
			&& !( flags_or  & KMP_FE_ALWAYS_0 )
			&& ( flags_and & KMP_FE_ALWAYS_1 ) == KMP_FE_ALWAYS_1;
    if (import)
    {
	kf->import = kf->export = 1;
	typeof(*kf->flags) *flags = kf->flags;

	const List_t *ptlist = kmp->dlist + sect;
	kmp_enpt_entry_t * item = (kmp_enpt_entry_t*)ptlist->list;

	const uint max = ptlist->used < max_import ? ptlist->used : max_import;
	for ( i = 0; i < max; i++, item++ )
	{
	    u32 *ptr = (u32*)&item->scale;
	    *flags++ = ( *ptr & KMP_FE_FLAGS ) >> KMP_FE_FLAGS_SHIFT;
	    *ptr &= ~KMP_FE_MASK;
	}
    }

    PRINT("(%s): or=%08x>%x, and=%08x>%x => %d (EXPORT)\n",
		kmp_section_name[sect].name1,
		flags_or,  flags_or  & KMP_FE_ALWAYS_0,
		flags_and, flags_and & KMP_FE_ALWAYS_1,
		import );
}


///////////////////////////////////////////////////////////////////////////////

static int sort_ph ( const kmp_enph_entry_t * a, const kmp_enph_entry_t * b )
{
    int stat = a->pt_start - b->pt_start;
    if (!stat)
	stat = a->pt_len - b->pt_len;
    return stat;
}

//-----------------------------------------------------------------------------

static void scan_kmp_ph
(
    kmp_t		* kmp,		// KMP data structure
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    uint		sect_ph,	// PH section index
    kmp_ph_t		* ph		// related PH info
)
{
    DASSERT(kmp);
    DASSERT(data);
    DASSERT(ph);
    DASSERT( sect_ph == KMP_CKPH || sect_ph == KMP_ENPH || sect_ph == KMP_ITPH );

    uint n;
    const kmp_enph_entry_t *phe
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,sect_ph);
    uint i;
    for ( i = 0; i < n; i++, phe++ )
    {
	kmp_enph_entry_t * item = AppendList(kmp->dlist+sect_ph);
	DASSERT(item);
	memcpy(item,phe,sizeof(*item));
	if ( i < KMP_MAX_GROUP )
	{
	    kmp_gopt2_t *go2 = ph->gopt + i;
	    go2->setting[0] = item->setting[0];
	    go2->setting[1] = item->setting[1];
	}
    }

    //--- sort groups

    kmp_enph_entry_t *list = (kmp_enph_entry_t*)(kmp->dlist[sect_ph].list);
    qsort( list, kmp->dlist[sect_ph].used, sizeof(*list), (qsort_func)sort_ph );

    //--- assign group names

    PRINT("scan_kmp_ph(%s), ac:%x,%x, battle=%d\n",
		SECT_NAME(sect_ph), ph->ac_mode, ph->ac_flags, kmp->battle_mode);
    RenameGroupKMP(kmp,sect_ph,ph->gname, ph->ac_mode == KMP_AC_DISPATCH );
}

///////////////////////////////////////////////////////////////////////////////

static void scan_kmp_pt2
(
    kmp_t		* kmp,		// KMP data structure
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    uint		sect		// KMP section index
)
{
    DASSERT(kmp);
    DASSERT( sect < KMP_N_SECT );

    uint n;
    const kmp_jgpt_entry_t *pt
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,sect);
    FillIL(kmp->index+sect,n);

    u32 flags_or  =  0;
    u32 flags_and = ~0;
    const uint max_import = sizeof(kmp->jgpt_flag.flags)/sizeof(*kmp->jgpt_flag.flags);

    uint i;
    for ( i = 0; i < n; i++, pt++ )
    {
	kmp_jgpt_entry_t * item = AppendList(kmp->dlist+sect);
	DASSERT(item);
	bef4n(item->position,pt->position,6);
	be16n((u16*)&item->id,(u16*)&pt->id,2);

	if ( i < max_import )
	{
	    flags_or  |= *(u32*)item->rotation;
	    flags_and &= *(u32*)item->rotation;
	}
    }

    const bool import = sect == KMP_JGPT
			&& !( flags_or  & KMP_FE_ALWAYS_0 )
			&& ( flags_and & KMP_FE_ALWAYS_1 ) == KMP_FE_ALWAYS_1;

    if (import)
    {
	kmp->jgpt_flag.export = kmp->jgpt_flag.import = 1;

	const List_t *ptlist = kmp->dlist + sect;
	kmp_jgpt_entry_t * item = (kmp_jgpt_entry_t*)ptlist->list;

	const uint max = ptlist->used < max_import ? ptlist->used : max_import;
	for ( i = 0; i < max; i++, item++ )
	{
	    u32 *ptr = (u32*)item->rotation;
	    kmp->jgpt_flag.flags[i] = ( *ptr & KMP_FE_FLAGS ) >> KMP_FE_FLAGS_SHIFT;
	    *ptr &= ~KMP_FE_MASK;
	}
    }

    PRINT("(%s): or=%08x>%x, and=%08x>%x => %d => %d %d (EXPORT)\n",
		kmp_section_name[sect].name1,
		flags_or,  flags_or  & KMP_FE_ALWAYS_0,
		flags_and, flags_and & KMP_FE_ALWAYS_1,
		import, kmp->jgpt_flag.import, kmp->jgpt_flag.export );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ScanRawKMP()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(kmp);
    if (init_kmp)
	InitializeKMP(kmp);

    PRINT0("KMP size:%x\n",data_size);

    const valid_t valid
	= IsValidKMP(data,data_size,data_size, kmp->nowarn_format ? 0 : kmp->fname );
    if ( valid >= VALID_ERROR )
    {
	if (!kmp->nowarn_format)
	    ERROR0(ERR_INVALID_DATA,
		"Invalid KMP file [%s]: %s\n"
		"Add option --kmp=force or --force to ignore some validity checks.",
		valid_text[valid], kmp->fname ? kmp->fname : "?" );
	return ERR_INVALID_DATA;
    }

    KMP_ACTION_LOG(false,"ScanRawKMP() %s\n",kmp->fname);
    kmp->fform = FF_KMP;
    kmp->kmp_version = be32( data + 0x0c );
    kmp->max_scanned  = 0;


    //--- test for unknown sections

    test_unknown_sections(data,data_size,kmp->fname,&kmp->is_shrinked);


    //--- scan KTPT entries

    uint n;
    const kmp_ktpt_entry_t * ktpt
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_KTPT);
    for ( ; n > 0; n--, ktpt++ )
    {
	kmp_ktpt_entry_t * entry = AppendList(kmp->dlist+KMP_KTPT);
	DASSERT(entry);
	bef4n(entry->position,ktpt->position,6);
	be16n((u16*)&entry->player_index,(u16*)&ktpt->player_index,2);
    }


    //--- scan ENPT + ENPH entries

    scan_kmp_pt(kmp,data,data_size,KMP_ENPT);
    scan_kmp_ph(kmp,data,data_size,KMP_ENPH,&kmp->enph);


    //--- scan ITPT + ITPH entries

    scan_kmp_pt(kmp,data,data_size,KMP_ITPT);
    scan_kmp_ph(kmp,data,data_size,KMP_ITPH,&kmp->itph);


    //--- scan CKPT entries

    const kmp_ckpt_entry_t * ckpt
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_CKPT);
    for ( ; n > 0; n--, ckpt++ )
    {
	kmp_ckpt_entry_t * entry = AppendList(kmp->dlist+KMP_CKPT);
	DASSERT(entry);
	memcpy(entry,ckpt,sizeof(*entry));
	bef4n(entry->left,ckpt->left,4);
    }


    //--- scan CKPH entries

    scan_kmp_ph(kmp,data,data_size,KMP_CKPH,&kmp->ckph);


    //--- scan GOBJ entries

    const kmp_gobj_entry_t * gobj
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_GOBJ);
    for ( ; n > 0; n--, gobj++ )
    {
	kmp_gobj_entry_t * entry = AppendList(kmp->dlist+KMP_GOBJ);
	DASSERT(entry);
	be16n((u16*)&entry->obj_id,(u16*)&gobj->obj_id,2);
	bef4n(entry->position,gobj->position,9);
	be16n(&entry->route_id,&gobj->route_id,10);
    }


    //--- scan POTI entries

    uint n_pt;
    const kmp_poti_group_t * pgroup
		= get_kmp_pointer(&n,&n_pt,0,kmp,data,data_size,KMP_POTI);
    while ( n_pt > 0 && n-- > 0 )
    {
	kmp_poti_group_t * group = AppendList(kmp->dlist+KMP_POTI);
	DASSERT(group);
	group->smooth	= pgroup->smooth;
	group->back	= pgroup->back;
	group->n_point	= be16((u16*)&pgroup->n_point);
	if ( group->n_point > n_pt )
	    group->n_point = n_pt;
	n_pt -= group->n_point;

	const kmp_poti_point_t * point = (kmp_poti_point_t*)(pgroup+1);
	uint count;
	for ( count = group->n_point; count > 0; count--, point++ )
	{
	    kmp_poti_point_t * entry = AppendList(&kmp->poti_point);
	    DASSERT(entry);
	    bef4n(entry->position,point->position,3);
	    be16n(&entry->speed,&point->speed,2);
	}
	pgroup = (kmp_poti_group_t*)point;
    }
    if ( kmp->max_scanned < (u8*)pgroup )
	 kmp->max_scanned = (u8*)pgroup;


    //--- scan AREA entries

    const kmp_area_entry_t * area
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_AREA);
    for ( ; n > 0; n--, area++ )
    {
	kmp_area_entry_t * entry = AppendList(kmp->dlist+KMP_AREA);
	DASSERT(entry);
	memcpy(entry,area,sizeof(*entry));
	bef4n(entry->position,area->position,9);
	be16n(entry->setting,area->setting,2);
	be16n(&entry->unknown_2e,&area->unknown_2e,1);
    }


    //--- scan CAME entries

    const kmp_came_entry_t * came
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_CAME);
    for ( ; n > 0; n--, came++ )
    {
	kmp_came_entry_t * entry = AppendList(kmp->dlist+KMP_CAME);
	DASSERT(entry);
	memcpy(entry,came,sizeof(*entry));
	be16n(&entry->came_speed,&came->came_speed,4);
	bef4n(entry->position,came->position,15);
    }


    //--- scan JGPT + CNPT + MSPT entries

    scan_kmp_pt2(kmp,data,data_size,KMP_JGPT);
    scan_kmp_pt2(kmp,data,data_size,KMP_CNPT);
    scan_kmp_pt2(kmp,data,data_size,KMP_MSPT);


    //--- scan STGI entries

    const kmp_stgi_entry_t * stgi
		= get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_STGI);
    if ( n > 0 )
	kmp->stgi = stgi;
    for ( ; n > 0; n--, stgi++ )
    {
	kmp_stgi_entry_t * entry = AppendList(kmp->dlist+KMP_STGI);
	DASSERT(entry);
	memcpy(entry,stgi,sizeof(*entry));
	entry->flare_color = be32(&stgi->flare_color);
	entry->speed_mod = be16(&stgi->speed_mod);
    }


    //--- scan WIM0 data

    const u8 *wim0_data = get_kmp_pointer(&n,0,0,kmp,data,data_size,KMP_WIM0);
    if ( wim0_data && n )
    {
	PRINT(">>> WIM0: %p %d\n",wim0_data,n);
	HEXDUMP16(9,0,wim0_data,n<0x40?n:0x40);
	const u8 *wim0_end = wim0_data + n;
	if ( kmp->max_scanned < wim0_end )
	    kmp->max_scanned = wim0_end;

	if ( opt_wim0 >= 0 )
	{
	    kmp->wim0_import_size = sizeof(kmp_list_head_t) + n;
	    kmp->wim0_import_bz2 = ntohl(*(u32*)wim0_data) != 0;
	    kmp->wim0_export = 1;
	    kmp_wim0_t w0;
	    InitializeWim0(&w0);
	    w0.bz2data = (u8*)wim0_data;
	    w0.bz2size = n;
	    if ( DecompressWim0(&w0) == ERR_OK && w0.used )
	    {
		FREE(kmp->wim0_data);
		kmp->wim0_data = (kmp_wim0_sect_t*)w0.data;
		kmp->wim0_size = w0.used;
		w0.data = 0;

		HEXDUMP16( 11, 0, kmp->wim0_data,
				kmp->wim0_size < 0x80 ? kmp->wim0_size : 0x80 );
		ScanWim0(kmp);
	    }
	    w0.bz2data = 0;
	    ResetWim0(&w0);
	}
    }


    //--- info string

    if (kmp->max_scanned)
    {
	PRINT("SCANNED: %zx/%x\n", kmp->max_scanned - (u8*)data, data_size );
	uint info_off = kmp->max_scanned - (u8*)data;
	if ( info_off < data_size )
	{
	    ccp src = (ccp)kmp->max_scanned;
	    const int info_len = strlen(src);
	    if ( info_off + info_len < data_size )
	    {
		FreeString(kmp->info);
		kmp->info = MALLOC(info_len+1);
		char *dest = kmp->info;
		for(;;)
		{
		    const uchar ch = *src++;
		    if (!ch)
			break;
		    *dest++ = ch == '\r' || ch == '\n'
				? ' '
				: ch == '"'
					? '\''
					: ch;
		}
		*dest = 0;
		DASSERT( dest == kmp->info + info_len );
		PRINT("INFO FOUND: |%s|\n",kmp->info);
	    }
	}
    }


    //--- setup battle mode and fix settings

    kmp->battle_mode = CheckBattleModeKMP(kmp);
    if ( kmp->battle_mode == ARENA_DISPATCH )
    {
	kmp->enph.ac_mode  = KMP_AC_DISPATCH;
	kmp->enph.ac_flags = 0;
	RenameGroupKMP(kmp,KMP_ENPH,kmp->enph.gname,true);
    }

    //--- term

    kmp->max_scanned = 0;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    CheckMode_t		mode		// not NULL: call CheckKMP(mode)
)
{
    DASSERT(kmp);
    PRINT("ScanKMP(mode=%x) size=%u\n",mode,data_size);
    //HEXDUMP16(0,0,data,16);

    enumError err;
// [[analyse-magic]]
    switch (GetByMagicFF(data,data_size,data_size))
    {
	case FF_KMP:
	    kmp->fform = FF_KMP;
	    err = ScanRawKMP(kmp,init_kmp,data,data_size);
	    break;

	case FF_KMP_TXT:
	    kmp->fform = FF_KMP_TXT;
	    err =  ScanTextKMP(kmp,init_kmp,data,data_size);
	    break;

	default:
	    if (init_kmp)
		InitializeKMP(kmp);
	    return ERROR0(ERR_INVALID_DATA,
		"No KMP file: %s\n", kmp->fname ? kmp->fname : "?");
    }

    PRINT("BATTLE-MODE: %d [opt=%d,isa=%d]\n",
		kmp->battle_mode, opt_battle_mode, IsArenaKMP(kmp) );

    if ( disable_patch_on_load <= 0 )
	PatchKMP(kmp);

    if ( err <= ERR_WARNING && mode )
	CheckKMP(kmp,mode);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckKMP(mode)
)
{
    DASSERT(kmp);
    DASSERT(raw);
    if (init_kmp)
	InitializeKMP(kmp);
    else
	ResetKMP(kmp);

    kmp->fatt  = raw->fatt;
    kmp->fname = raw->fname;
    raw->fname = 0;

    return raw->is_0 ? ERR_OK : ScanKMP(kmp,false,raw->data,raw->data_size,mode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckKMP(mode)
)
{
    DASSERT(kmp);
    DASSERT(fname);
    if (init_kmp)
	InitializeKMP(kmp);
    else
	ResetKMP(kmp);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	kmp->fname = raw.fname;
	raw.fname = 0;
	err = ScanKMP(kmp,false,raw.data,raw.data_size,mode);
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ShrinkKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////

bool ShrinkRawKMP
(
    // returns true (=success), if dest is created and data is shrinked

    u8		**res_dest,	// not NULL: store result here on success
    uint	*res_dest_size,	// not NULL: store result here on success
    bool	free_old_dest,	// true: FREE(*res_dest) on success

    const u8	*src_data,	// NULL or source; if NULL: use *res_dest
    uint	src_size,	// size of source
    bool	log		// true: KMP_ACTION_LOG()
)
{
    if (!src_data)
    {
	src_data = *res_dest;
	src_size = *res_dest_size;
    }

    if ( !src_data || !src_size )
	return false;


    //--- calculate new size

    kmp_section_t info[KMP_N_SECT], *iptr = info;
    const kmp_file_mkw_t *kfile	= (kmp_file_mkw_t*)src_data;
    const uint src_sect		= ntohs(kfile->n_sect);
    uint total_size		= 0;

    uint sect;
    for ( sect = 0; sect < src_sect; sect++ )
    {
	if ( AnalyseSectionKMP(iptr,src_data,src_size,sect)
	    && iptr->size > sizeof(kmp_list_head_t) )
	{
	    total_size += iptr->size;
	    TRACE(">> sect=%2d, index=%2d, off=%6zx .. %6zx, size=%5x %5x\n",
		iptr->sect, iptr->index,
		(u8*)iptr->ptr - src_data,
		(u8*)iptr->ptr - src_data + iptr->size,
		iptr->size, total_size );
	    iptr++;
	}
    }

    const uint n_sect = iptr - info;
    const uint head_size = GetFileHeadSizeKMP(n_sect);
    total_size += head_size;

    if ( !n_sect || total_size >= src_size )
	return false;

    if (log)
	KMP_ACTION_LOG(false,"ShrinkKMP() n-sect=%u/%u, size=%u+%u\n",
		n_sect, be16(&kfile->n_sect),
		head_size, total_size-head_size );

    u8 *data = MALLOC(total_size);
    kmp_file_mkw_t *khead = (kmp_file_mkw_t*)data;

    //--- setup file header

    memcpy(khead->magic,KMP_MAGIC,sizeof(khead->magic));
    khead->file_size	= htonl(total_size);
    khead->n_sect	= htons(n_sect);
    khead->head_size	= htons(head_size);
    khead->version	= kfile->version;

    u8 *dest = data + head_size;
    uint i;
    for ( i = 0, iptr = info; i < n_sect; i++, iptr++ )
    {
	khead->sect_off[i] = ntohl(dest-data-head_size);
	memcpy(dest,iptr->ptr,iptr->size);
	dest += iptr->size;
    }
    ASSERT( dest == data + total_size );


    //--- results

    if (res_dest)
    {
	if ( free_old_dest && *res_dest )
	    FREE(*res_dest);
	*res_dest = data;
    }
    else
	FREE(data);

    if (res_dest_size)
	*res_dest_size = total_size;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ShrinkKMP
(
    kmp_t		* kmp		// pointer to valid KMP
)
{
    DASSERT(kmp);
    TRACE("ShrinkKMP()\n");
    if (!kmp->raw_data)
	return ERR_OK;

    return ShrinkRawKMP( &kmp->raw_data, &kmp->raw_data_size, true, 0, 0, true )
		? ERR_DIFFER : ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateRawKMP helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static void * setup_raw_kmp
(
    const kmp_t		* kmp,		// input: KMP
    uint		sect,		// input: section number
    kmp_list_head_t	** hd,		// modify: header pointer
    const List_t	** dlist	// result: list pointer
)
{
    DASSERT(kmp);
    DASSERT( sect < KMP_N_SECT );
    DASSERT(dlist);
    DASSERT(hd);
    DASSERT(kmp->raw_data);
    DASSERT(kmp->raw_head_size);

    const List_t *dl = kmp->dlist + sect;
    *dlist = dl;
    const uint n = dl->used;

    kmp_list_head_t *h = *hd;
    *hd = (kmp_list_head_t*)( (u8*)(h+1) + kmp_entry_size[sect] * n );
    kmp_file_mkw_t *kfile = (kmp_file_mkw_t*)kmp->raw_data;
    write_be32( kfile->sect_off + sect, (u8*)h - kmp->raw_data - kmp->raw_head_size );

    memcpy(h->magic,kmp_section_name[sect].name1,sizeof(h->magic));
    write_be16(&h->n_entry,n);
    write_be16(&h->value,kmp->value[sect]);

    return h+1;
}

///////////////////////////////////////////////////////////////////////////////

static void create_kmp_pt
(
    const kmp_t		* kmp,		// input: KMP
    uint		sect,		// input: section number
    kmp_list_head_t	** hd		// modify: header pointer
)
{
    DASSERT(kmp);
    DASSERT(hd);
    DASSERT( sect == KMP_ENPT || sect == KMP_ITPT );

    //--- section setup

    const kmp_ph_t *ph;
    const kmp_flag_t *kf;
    if ( sect == KMP_ENPT )
    {
	ph = &kmp->enph;
	kf = &kmp->enpt_flag;
    }
    else
    {
	ph = &kmp->itph;
	kf = &kmp->itpt_flag;
    }

    //--- prepare flag export

    const uint max_export = sizeof(kf->flags) / sizeof(*kf->flags);
    PRINT(">> EXPORT_FLAGS[%s] = %d [%d], max=%d\n",
		kmp_section_name[sect].name1,kf->export,opt_export_flags,max_export);


    //--- create binary data

    const List_t *dlist;
    kmp_enpt_entry_t * pt = setup_raw_kmp(kmp,sect,hd,&dlist);
    uint i;
    for ( i = 0; i < dlist->used; i++, pt++ )
    {
	DASSERT( (u8*)pt < (u8*)*hd );
	const kmp_enpt_entry_t * src = (kmp_enpt_entry_t*)dlist->list + i;

	write_bef4n(pt->position,src->position,4);
	if (kf->export)
	{
	    u8 *ptr = (u8*)&pt->scale;
	    write_bef4(ptr,RoundF3bytes(bef4(ptr)));
	    ptr[3] = ( i < max_export ? kf->flags[i] : kf->mask )
			<< KMP_FE_FLAGS_SHIFT | KMP_FE_ALWAYS_1;
	}

	write_be16(pt->prop+0,src->prop[0]&ph->mask[0]);
	write_be16(pt->prop+1,src->prop[1]&ph->mask[1]);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void create_kmp_ph
(
    const kmp_t		* kmp,		// input: KMP
    uint		sect,		// input: section number
    kmp_list_head_t	** hd,		// modify: header pointer
    kmp_ph_t		* ph		// related PH info
)
{
    DASSERT(kmp);
    DASSERT(hd);
    DASSERT( sect == KMP_CKPH || sect == KMP_ENPH || sect == KMP_ITPH );

    const List_t *dlist;
    kmp_enph_entry_t * phe = setup_raw_kmp(kmp,sect,hd,&dlist);
    uint i;
    for ( i = 0; i < dlist->used; i++, phe++ )
    {
	DASSERT( (u8*)phe < (u8*)*hd );
	const kmp_enph_entry_t * src = (kmp_enph_entry_t*)dlist->list + i;
	memcpy(phe,src,sizeof(*phe));
	if ( i < KMP_MAX_GROUP && ph )
	{
	    kmp_gopt2_t *go2 = ph->gopt + i;
	    phe->setting[0] = go2->setting[0];
	    phe->setting[1] = go2->setting[1];
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void create_kmp_pt2
(
    const kmp_t		* kmp,		// input: KMP
    uint		sect,		// input: section number
    kmp_list_head_t	** hd		// modify: header pointer
)
{
    DASSERT(kmp);
    DASSERT(hd);

    const List_t *dlist;
    kmp_jgpt_entry_t * pt = setup_raw_kmp(kmp,sect,hd,&dlist);
    const bool flag_export = sect == KMP_JGPT && kmp->jgpt_flag.export;
    const uint max_export = sizeof(kmp->jgpt_flag.flags) / sizeof(*kmp->jgpt_flag.flags);
    uint i;

    for ( i = 0; i < dlist->used; i++, pt++ )
    {
	DASSERT( (u8*)pt < (u8*)*hd );
	const kmp_jgpt_entry_t * src = (kmp_jgpt_entry_t*)dlist->list + i;

	write_bef4n(pt->position,src->position,6);
	if ( flag_export )
	{
	    u8 *ptr = (u8*)&pt->rotation;
	    write_bef4(ptr,RoundF3bytes(bef4(ptr)));
	    ptr[3] = ( i < max_export ? kmp->jgpt_flag.flags[i] : kmp->jgpt_flag.mask )
			<< KMP_FE_FLAGS_SHIFT | KMP_FE_ALWAYS_1;
	}

	write_be16n((u16*)&pt->id,(u16*)&src->id,2);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateRawKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateRawKMP
(
    kmp_t		* kmp		// pointer to valid KMP
)
{
    DASSERT(kmp);
    TRACE("CreateRawKMP()\n");


    //--- calculate data size

    const uint info_size = kmp->info ? ALIGN32(strlen(kmp->info)+1,4) : 0;

    const u8 *wim0_data = 0;
    uint n_sect, head_size, wim0_data_size = 0, wim0_size;
    noPRINT(">>>> wim0_export= %d %d\n",opt_wim0,kmp->wim0_export);
    if ( kmp->wim0_export && CreateWim0(&wim0_data,&wim0_data_size,kmp) )
    {
	n_sect		= KMP_N_SECT + 1;
	head_size	= sizeof(kmp_file_wim0_t);
	wim0_size	= ALIGN32(wim0_data_size,4);
    }
    else
    {
	n_sect		= KMP_N_SECT;
	head_size	= sizeof(kmp_file_mkw_t);
	wim0_data_size	= 0;
	wim0_size	= 0;
    }

    uint data_size = head_size
		   + sizeof(kmp_list_head_t) * n_sect
		   + sizeof(kmp_poti_point_t) * kmp->poti_point.used
		   + info_size
		   + wim0_size;

    uint sect;
    for ( sect = 0; sect < KMP_N_SECT; sect++ )
	data_size += kmp_entry_size[sect] * kmp->dlist[sect].used;

    PRINT("CREATE KMP: size = %#x = %u\n",data_size,data_size);
    KMP_ACTION_LOG(false,"CreateRawKMP() size=%u\n",data_size);


    //--- alloc data

    FREE(kmp->raw_data);
    kmp_file_mkw_t * kfile	= CALLOC(1,data_size);
    kmp->raw_data	= (u8*)kfile;
    kmp->raw_data_size	= data_size;
    kmp->raw_head_size	= head_size;


    //--- setup file header

    memcpy(kfile->magic,KMP_MAGIC,sizeof(kfile->magic));
    write_be32(&kfile->file_size,data_size);
    write_be16(&kfile->n_sect,n_sect);
    write_be16(&kfile->head_size,head_size);
    write_be32(&kfile->version,kmp->kmp_version);

    kmp_list_head_t * hd = (kmp_list_head_t*)(kmp->raw_data+head_size);

    if (info_size)
	strcpy( (char*)kfile + data_size - info_size, kmp->info );

    uint i;
    const List_t *dlist;


    //--- save KTPT entries

    kmp_ktpt_entry_t * ktpt = setup_raw_kmp(kmp,KMP_KTPT,&hd,&dlist);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );
    for ( i = 0; i < dlist->used; i++, ktpt++ )
    {
	DASSERT( (u8*)ktpt < (u8*)hd );
	const kmp_ktpt_entry_t * src = (kmp_ktpt_entry_t*)dlist->list + i;
	write_bef4n(ktpt->position,src->position,6);
	write_be16n((u16*)&ktpt->player_index,(u16*)&src->player_index,2);
    }


    //--- save ENPT + ENPH entries

    create_kmp_pt(kmp,KMP_ENPT,&hd);
    create_kmp_ph(kmp,KMP_ENPH,&hd,&kmp->enph);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );


    //--- save ITPT + ITPH entries

    create_kmp_pt(kmp,KMP_ITPT,&hd);
    create_kmp_ph(kmp,KMP_ITPH,&hd,&kmp->itph);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );


    //--- save CKPT entries

    kmp_ckpt_entry_t * ckpt = setup_raw_kmp(kmp,KMP_CKPT,&hd,&dlist);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );
    for ( i = 0; i < dlist->used; i++, ckpt++ )
    {
	DASSERT( (u8*)ckpt < (u8*)hd );
	const kmp_ckpt_entry_t * src = (kmp_ckpt_entry_t*)dlist->list + i;
	memcpy(ckpt,src,sizeof(*ckpt));
	write_bef4n(ckpt->left,src->left,4);
    }


    //--- save CKPH entries

    create_kmp_ph(kmp,KMP_CKPH,&hd,&kmp->enph);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );


    //--- save GOBJ entries

    kmp_gobj_entry_t * gobj = setup_raw_kmp(kmp,KMP_GOBJ,&hd,&dlist);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );
    for ( i = 0; i < dlist->used; i++, gobj++ )
    {
	DASSERT( (u8*)gobj < (u8*)hd );
	const kmp_gobj_entry_t * src = (kmp_gobj_entry_t*)dlist->list + i;
	write_be16n((u16*)&gobj->obj_id,(u16*)&src->obj_id,2);
	write_bef4n(gobj->position,src->position,9);
	write_be16n(&gobj->route_id,&src->route_id,10);
    }


    //--- save POTI entries

    kmp_list_head_t * hd2 = hd;
    u8 * poti = setup_raw_kmp(kmp,KMP_POTI,&hd,&dlist);
    uint ng = dlist->used;
    uint np = kmp->poti_point.used;
    write_be16(&hd2->n_entry,ng);
    write_be16(&hd2->value,np);

    hd = (kmp_list_head_t*)( (u8*)hd + sizeof(kmp_poti_point_t) * np );
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );

    const kmp_poti_group_t * pg = (kmp_poti_group_t*)dlist->list;
    const kmp_poti_point_t * pp = (kmp_poti_point_t*)kmp->poti_point.list;

    uint gi;
    for ( gi = 0; gi < ng && np > 0; gi++, pg++ )
    {
	DASSERT( (u8*)poti < (u8*)hd );
	write_be16((u16*)poti,pg->n_point);
	poti[2] = pg->smooth;
	poti[3] = pg->back;

	poti += sizeof(*pg);

	uint en = pg->n_point;
	DASSERT( en <= np );
	for ( ; en > 0; en--, i++, pp++ )
	{
	    noPRINT("%p/%p %p/%p\n",
		poti, hd, pp, dlist->list + dlist->used * dlist->size );

	    DASSERT( (u8*)poti < (u8*)hd );
	    write_bef4n((float32*)poti,pp->position,3);
	    poti += sizeof(pp->position);

	    DASSERT( (u8*)poti < (u8*)hd );
	    write_be16n((u16*)poti,&pp->speed,2);
	    poti += 2*sizeof(pp->speed);
	}
    }


    //--- save AREA entries

    kmp_area_entry_t * area = setup_raw_kmp(kmp,KMP_AREA,&hd,&dlist);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );
    for ( i = 0; i < dlist->used; i++, area++ )
    {
	DASSERT( (u8*)area < (u8*)hd );
	const kmp_area_entry_t * src = (kmp_area_entry_t*)dlist->list + i;
	memcpy(area,src,sizeof(*area));
	write_bef4n(area->position,src->position,9);
	write_be16n(area->setting,src->setting,2);
	write_be16n(&area->unknown_2e,&src->unknown_2e,1);
    }


    //--- save CAME entries

    hd2 = hd;
    kmp_came_entry_t * came = setup_raw_kmp(kmp,KMP_CAME,&hd,&dlist);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );

    for ( i = 0; i < dlist->used; i++, came++ )
    {
	DASSERT( (u8*)came < (u8*)hd );
	const kmp_came_entry_t * src = (kmp_came_entry_t*)dlist->list + i;
	memcpy(came,src,sizeof(*came));
	write_be16n(&came->came_speed,&src->came_speed,4);
	write_bef4n(came->position,src->position,15);
    }


    //--- save JGPT + CNPT + MSPT entries

    create_kmp_pt2(kmp,KMP_JGPT,&hd);
    create_kmp_pt2(kmp,KMP_CNPT,&hd);
    create_kmp_pt2(kmp,KMP_MSPT,&hd);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );


    //--- save STGI entries

    kmp_stgi_entry_t * stgi = setup_raw_kmp(kmp,KMP_STGI,&hd,&dlist);
    DASSERT( (u8*)hd <= kmp->raw_data + data_size );
    for ( i = 0; i < dlist->used; i++, stgi++ )
    {
	DASSERT( (u8*)stgi < (u8*)hd );
	const kmp_stgi_entry_t * src = (kmp_stgi_entry_t*)dlist->list + i;
	memcpy(stgi,src,sizeof(*stgi));
	write_be32(&stgi->flare_color,src->flare_color);
	write_be16(&stgi->speed_mod,src->speed_mod);
    }


    //--- save WIM0 data

// [[wim0+]]
    if ( kmp->wim0_export )
    {
	kmp_list_head_t *h = hd;
	hd = (kmp_list_head_t*)( (u8*)(h+1) + wim0_size );
	write_be32( kfile->sect_off + KMP_N_SECT,
			(u8*)h - kmp->raw_data - kmp->raw_head_size );
	memcpy(h->magic,"WIM0",sizeof(h->magic));
	write_be32(&h->n_entry,wim0_size);
	memcpy(h->entry,wim0_data,wim0_data_size);
    }


    //--- terminate

    DASSERT( (u8*)hd + info_size == kmp->raw_data + data_size );

    if ( KMP_MODE & KMPMD_RM_EMPTY )
	ShrinkKMP(kmp);

    FREE((u8*)wim0_data);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    SaveRawKMP()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawKMP
(
    kmp_t		* kmp,		// pointer to valid KMP
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(kmp);
    DASSERT(fname);
    PRINT("SaveRawKMP(%s,%d)\n",fname,set_time);
    KMP_ACTION_LOG(false,"SaveRawKMP() %s\n",fname);

    //--- create raw data

    enumError err = CreateRawKMP(kmp);
    if (err)
	return err;
    DASSERT(kmp->raw_data);
    DASSERT(kmp->raw_data_size);
    DASSERT(kmp->raw_head_size);

    //--- write to file

    File_t F;
    err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&kmp->fatt,0);

    if ( fwrite(kmp->raw_data,1,kmp->raw_data_size,F.f) != kmp->raw_data_size )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Sort GOBJ			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct sort_gobj_t
{
    int			rank;	// sort 1: rank of object
    int			angle;	// sort 2: h-angle around (0,0) in degree*1000000
    int			pos;	// sort 3: old pos of object
    kmp_gobj_entry_t	data;	// raw object data

} sort_gobj_t;

//-----------------------------------------------------------------------------

static int sort_gobj ( const sort_gobj_t * a, const sort_gobj_t * b )
{
    int stat = a->rank - b->rank;
    if (!stat)
    {
	stat = a->angle - b->angle;
	if (!stat)
	    stat = a->pos - b->pos;
    }
    return stat;
}

//-----------------------------------------------------------------------------

static int sort_gobj_tiny ( const sort_gobj_t * a, const sort_gobj_t * b )
{
    int stat = memcmp(	a->data.setting,
			b->data.setting,
			sizeof(kmp_gobj_entry_t) - offsetof(kmp_gobj_entry_t,setting) );

    if (!stat)
    {
	stat = memcmp(	a->data.rotation,
			b->data.rotation,
			sizeof(kmp_gobj_entry_t) - offsetof(kmp_gobj_entry_t,rotation) );
	if (!stat)
	{
	    stat = a->rank - b->rank;
	    if (!stat)
		stat = a->pos - b->pos;
	}
    }
    return stat;
}

//-----------------------------------------------------------------------------

bool SortGOBJ
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			sort_mode	// <=0: dont sort
					//   1: group only
					//   2: secondary sort by angle
					//   3: sort for TINY
)
{
    DASSERT(kmp);
    PRINT("SortGOBJ(%d)\n",sort_mode);

    List_t *dlist = kmp->dlist + KMP_GOBJ;
    if ( sort_mode < 1 || dlist->used < 2 )
	return false;


    //--- build a group rank table

    int rank[N_KMP_GOBJ];
    memset(rank,0,sizeof(rank));

    kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)dlist->list;
    int i, rank_count = 0;
    for ( i = dlist->used - 1; i >= 0; i-- )
    {
	const typeof(gobj->obj_id) obj_id = gobj[i].obj_id;
	if ( obj_id < N_KMP_GOBJ )
	    rank[obj_id] = --rank_count;
    }
    rank[0] = 0;


    //--- create sort array and copy data

    sort_gobj_t *sg_beg = CALLOC(dlist->used,sizeof(*sg_beg));
    sort_gobj_t *sg_end = sg_beg + dlist->used;
    sort_gobj_t *sg;
    for ( sg = sg_beg; sg < sg_end; sg++, gobj++ )
    {
	const typeof(gobj->obj_id) obj_id = gobj->obj_id;
	sg->pos  = sg - sg_beg;
	sg->rank = obj_id < N_KMP_GOBJ ? rank[obj_id] : obj_id;
	if ( sort_mode == KMP_SORT_ANGLE )
	{
	    sg->angle = atan2(gobj->position[0],gobj->position[2]) * (180000000/M_PI);
	    if ( sg->angle < 0)
		sg->angle += 360000000;
	}
	memcpy(&sg->data,gobj,sizeof(sg->data));
    }

    //--- sort data

    if ( sort_mode == KMP_SORT_TINY )
	qsort( sg_beg, dlist->used, sizeof(*sg_beg), (qsort_func)sort_gobj_tiny );
    else
	qsort( sg_beg, dlist->used, sizeof(*sg_beg), (qsort_func)sort_gobj );


    //--- store sorted data

    gobj = (kmp_gobj_entry_t*)dlist->list;
    for ( sg = sg_beg; sg < sg_end; sg++, gobj++ )
    {
	PRINT("SORT: %5d %10d %5d\n",sg->rank,sg->angle,sg->pos);
	memcpy(gobj,&sg->data,sizeof(*gobj));
    }

    //--- terminate

    FREE(sg_beg);
    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		   Startpoint Calculations		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kmp_start_pos_t
{
    double x;		// relative x position, range -800.0 .. +800.0
    double z;		// relative z position, <0.0 if unknown
    double z_narrow;	// narrow z position, <0.0 if unknown
}
kmp_start_pos_t;

static const kmp_start_pos_t StartPosTab[] =
{
	{    0.0,    0.00,    0.00 },	//  1/1

	{ -400.0,    0.00,    0.00 },	//  1/2
	{  400.0,  461.88,  373.06 },	//  2/2

	{ -800.0,    0.00,    0.00 },	//  1/3
	{    0.0,  461.88,  373.06 },	//  2/3
	{  800.0,  923.75,  746.12 },	//  3/3

	{ -800.0,    0.00,    0.00 },	//  1/4
	{  400.0,  692.81,  559.56 },	//  2/4
	{ -400.0, 1530.94, 1386.56 },	//  3/4
	{  800.0, 2223.75, 1946.12 },	//  4/4

	{ -800.0,    0.00,    0.00 },	//  1/5
	{    0.0,  461.88,  373.06 },	//  2/5
	{  800.0,  923.75,  746.12 },	//  3/5
	{ -400.0, 1530.94, 1386.56 },	//  4/5	unsure
	{  400.0, 1992.81, 1759.62 },	//  5/5	unsure

	{ -800.0,    0.00,    0.00 },	//  1/6
	{ -160.0,  369.50,  298.44 },	//  2/6
	{  480.0,  739.00,  596.88 },	//  3/6
	{ -480.0, 1484.75, 1349.19 },	//  4/6
	{  160.0, 1854.25, 1647.62 },	//  5/6
	{  800.0, 2223.75, 1946.06 },	//  6/6

	{ -400.0,    0.00,    0.00 },	//  1/7		??? narrow guessed (compelte series)
	{  400.0,  461.88,  373.06 },	//  2/7
	{ -800.0, 1069.06, 1013.50 },	//  3/7	unsure
	{    0.0, 1530.94, 1386.56 },	//  4/7	unsure
	{  800.0, 1992.81, 1759.62 },	//  5/7	unsure
	{ -400.0, 2900.00, 2650.00 },	//  6/7
	{  400.0, 3361.88, 3023.06 },	//  7/7

	{ -800.0,    0.00,    0.00 },	//  1/8
	{    0.0,  461.88,  373.06 },	//  2/8
	{  800.0,  923.75,  746.12 },	//  3/8
	{ -400.0, 1530.94, 1386.56 },	//  4/8	unsure
	{  400.0, 1992.81, 1759.62 },	//  5/8	unsure
	{ -800.0, 2900.00, 2650.00 },	//  6/8	unsure
	{    0.0, 3361.88, 3023.06 },	//  7/8
	{  800.0, 3823.75, 3396.12 },	//  8/8

	{ -800.0,    0.00,    0.00 },	//  1/9
	{ -160.0,  369.50,  298.44 },	//  2/9
	{  480.0,  739.00,  596.88 },	//  3/9
	{ -480.0, 1484.75, 1349.19 },	//  4/9
	{  160.0, 1854.25, 1647.62 },	//  5/9
	{  800.0, 2223.75, 1946.07 },	//  6/9
	{ -800.0, 2900.00, 2650.00 },	//  7/9
	{ -160.0, 3269.50, 2948.44 },	//  8/9
	{  480.0, 3639.00, 3246.88 },	//  9/9

	{ -800.0,    0.00,    0.00 },	//  1/10
	{    0.0,  461.88,  373.06 },	//  2/10
	{  800.0,  923.75,  746.13 },	//  3/10
	{ -400.0, 1530.94, 1386.56 },	//  4/10
	{  400.0, 1992.82, 1759.56 },	//  5/10
	{ -800.0, 2900.00, 2650.00 },	//  6/10
	{    0.0, 3361.88, 3023.06 },	//  7/10
	{  800.0, 3823.75, 3396.12 },	//  8/10
	{ -400.0, 4430.94, 4036.56 },	//  9/10
	{  400.0, 4892.82, 4409.56 },	// 10/10

	{ -800.0,    0.00,    0.00},	//  1/11
	{ -160.0,  369.50,  298.44 },	//  2/11
	{  480.0,  739.00,  596.88 },	//  3/11
	{ -480.0, 1484.75, 1349.19 },	//  4/11
	{  160.0, 1854.25, 1647.62 },	//  5/11
	{  800.0, 2223.75, 1946.07 },	//  6/11
	{ -800.0, 2900.00, 2650.00 },	//  7/11
	{ -160.0, 3269.50, 2948.44 },	//  8/11
	{  480.0, 3639.00, 3246.88 },	//  9/11
	{ -480.0, 4384.75, 3999.21 },	// 10/11
	{  160.0, 4754.25, 4297.62 },	// 11/11

	{ -800.0,    0.00,    0.00},	//  1/12
	{ -160.0,  369.50,  298.44 },	//  2/12
	{  480.0,  739.00,  596.88 },	//  3/12
	{ -480.0, 1484.75, 1349.19 },	//  4/12
	{  160.0, 1854.25, 1647.62 },	//  5/12
	{  800.0, 2223.75, 1946.07 },	//  6/12
	{ -800.0, 2900.00, 2650.00 },	//  7/12
	{ -160.0, 3269.50, 2948.44 },	//  8/12
	{  480.0, 3639.00, 3246.88 },	//  9/12
	{ -480.0, 4384.75, 3999.21 },	// 10/12
	{  160.0, 4754.25, 4297.62 },	// 11/12
	{  800.0, 5123.75, 4596.06 },	// 12/12
};

//-----------------------------------------------------------------------------

uint kmp_start_pos_mirror = false;
uint kmp_start_pos_narrow = false;
bool kmp_start_pos_auto   = false;

const double3 * GetStartPosKMP
(
    // Return NULL on failure.
    // Otherwise return a pointer to 'tab' or to an internal static buffer
    // with N elements. This buffer is overwritten on next call.

    double3	*tab,		// table with 'n' elements to store the result.
				//   if NULL: use internal static buffer
    uint	n,		// number of players: 1..12
    bool	mirror,		// true: mirror the data (pole is right)
    bool	narrow,		// true: use 'narrow' mode

    kmp_t	*kmp,		// not NULL: Use KTPT/#0 and STGI/#0
    uint	automode	// bit field: Use KMP to override parameters
				//  1: return absolute positions, if KMP available
				//  2: use pole position of KMP if available
				//  4: use narrow mode of KMP if available
)
{
    //--- valid 'n'?

    if ( n < 1 || n > 12 )
	return 0;


    //--- auto mode

    kmp_start_pos_auto   = 0;
    kmp_start_pos_mirror = mirror;
    kmp_start_pos_narrow = narrow;

    if ( kmp && kmp->dlist[KMP_STGI].used )
    {
	const kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp->dlist[KMP_STGI].list;
	if ( automode & 2 )
	{
	    kmp_start_pos_auto |= 2;
	    kmp_start_pos_mirror = stgi->pole_pos;
	    mirror = kmp_start_pos_mirror == 1;
	}
	if ( automode & 4 )
	{
	    kmp_start_pos_auto |= 4;
	    kmp_start_pos_narrow = stgi->narrow_start;
	    narrow = kmp_start_pos_narrow == 1;
	}
    }


    //--- assign table values

    static double3 itab[12];
    if (!tab)
	tab = itab;
    double3 *d;
    const kmp_start_pos_t *sp = StartPosTab + n * ( n-1 ) / 2;

    uint i;
    for ( i = 0, d = tab; i < n; i++, d++, sp++ )
    {
	d->x = mirror ? -sp->x : sp->x;
	d->y = 0.0;
	d->z = narrow ? sp->z_narrow : sp->z;
	if (IsNormalD(d->z))
	    d->z = ldexp(trunc(ldexp(d->z,4)+0.5),-4);
    }


    //--- valid KMP/KTPT?

    if ( !(automode&1) || !kmp || !kmp->dlist[KMP_KTPT].used )
	return tab;
    kmp_start_pos_auto |= 1;


    //--- transformation step 1: y-rotate by 180 degree to get normalized vectors

    MatrixD_t matrix = {0};
    InitializeMatrixD(&matrix);
    SetRotateMatrixD(&matrix,1,180.0,0.0,0);
    TransformD3NMatrixD(&matrix,tab,n,sizeof(*tab));


    //--- transformation step 2: rotate and translate by start point settings

    const kmp_ktpt_entry_t * ktpt = (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;
    SetRotateMatrixD(&matrix,0,ktpt->rotation[0],0.0,0);
    SetRotateMatrixD(&matrix,1,ktpt->rotation[1],0.0,0);
    SetRotateMatrixD(&matrix,2,ktpt->rotation[2],0.0,0);
    double3 temp;
    temp.x = ktpt->position[0];
    temp.y = ktpt->position[1];
    temp.z = ktpt->position[2];
    SetTranslateMatrixD(&matrix,&temp);
    TransformD3NMatrixD(&matrix,tab,n,sizeof(*tab));

    return tab;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Transform KMP			///////////////
///////////////////////////////////////////////////////////////////////////////

bool TransformKMP
(
    kmp_t		* kmp		// pointer to valid KMP
)
{
    if ( have_patch_count <= 0 || !transform_active || disable_transformation > 0 )
	return false;

    KMP_ACTION_LOG(true,"Transform KMP\n");
    PRINT("** TransformKMP() **\n");
    int n;

    //--- AREA

    n = kmp->dlist[KMP_AREA].used;
    if (n)
    {
	kmp_area_entry_t * area = (kmp_area_entry_t*)kmp->dlist[KMP_AREA].list;
	if ( KMP_TFORM & KMPTF_AREA_POS )
	    TransformPosFloat3D(area->position,n,sizeof(*area));
	if ( KMP_TFORM & KMPTF_AREA_ROTATE )
	    TransformRotFloat3D(area->rotation,n,sizeof(*area));
	if ( KMP_TFORM & KMPTF_AREA_SCALE )
	    TransformScaleFloat3D(area->scale,n,sizeof(*area));
    }

    //--- CAME

    u8 poti_notrans[0x100] = {0};

    n = kmp->dlist[KMP_CAME].used;
    if ( n  )
    {
	kmp_came_entry_t * came = (kmp_came_entry_t*)kmp->dlist[KMP_CAME].list;
	while ( n-- > 0 )
	{
	    if ( came->type == 6 )
		poti_notrans[came->route] = 1;
	    else if ( KMP_TFORM & KMPTF_CAME_POS && came->type != 0 && came->type != 3 )
	    {
		TransformPosFloat3D(came->position,1,sizeof(*came));
		TransformPosFloat3D(came->viewpt_begin,1,sizeof(*came));
		TransformPosFloat3D(came->viewpt_end,1,sizeof(*came));
	    }
	    came++;
	}
    }

    //--- CKPT

    n = kmp->dlist[KMP_CKPT].used;
    if ( n && KMP_TFORM & KMPTF_CKPT_POS )
    {
	kmp_ckpt_entry_t * ckpt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;
	TransformPosFloat2D(ckpt->left,n,sizeof(*ckpt));
	TransformPosFloat2D(ckpt->right,n,sizeof(*ckpt));
    }

    //--- CNPT

    n = kmp->dlist[KMP_CNPT].used;
    if (n)
    {
	kmp_cnpt_entry_t * cnpt = (kmp_cnpt_entry_t*)kmp->dlist[KMP_CNPT].list;
	if ( KMP_TFORM & KMPTF_CNPT_POS )
	    TransformPosFloat3D(cnpt->position,n,sizeof(*cnpt));
	if ( KMP_TFORM & KMPTF_CNPT_ROTATE )
	    TransformRotFloat3D(cnpt->rotation,n,sizeof(*cnpt));
    }

    //--- ENPT

    n = kmp->dlist[KMP_ENPT].used;
    if (n)
    {
	kmp_enpt_entry_t * enpt = (kmp_enpt_entry_t*)kmp->dlist[KMP_ENPT].list;
	if ( KMP_TFORM & KMPTF_ENPT_POS )
	    TransformPosFloat3D(enpt->position,n,sizeof(*enpt));
	if ( KMP_TFORM & KMPTF_ENPT_SCALE )
	    TransformScaleFloat1D(&enpt->scale,n,sizeof(*enpt));
    }

    //--- GOBJ

    n = kmp->dlist[KMP_GOBJ].used;
    if (n)
    {
	kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	if ( KMP_TFORM & KMPTF_GOBJ_POS )
	    TransformPosFloat3D(gobj->position,n,sizeof(*gobj));
	if ( KMP_TFORM & KMPTF_GOBJ_ROTATE )
	    TransformRotFloat3D(gobj->rotation,n,sizeof(*gobj));
	if ( KMP_TFORM & KMPTF_GOBJ_SCALE )
	    TransformScaleFloat3D(gobj->scale,n,sizeof(*gobj));
    }

    //--- ITPT

    n = kmp->dlist[KMP_ITPT].used;
    if (n)
    {
	kmp_itpt_entry_t * itpt = (kmp_itpt_entry_t*)kmp->dlist[KMP_ITPT].list;
	if ( KMP_TFORM & KMPTF_ITPT_POS )
	    TransformPosFloat3D(itpt->position,n,sizeof(*itpt));
	if ( KMP_TFORM & KMPTF_ITPT_SCALE )
	    TransformScaleFloat1D(&itpt->scale,n,sizeof(*itpt));
    }

    //--- JGPT

    n = kmp->dlist[KMP_JGPT].used;
    if (n)
    {
	kmp_jgpt_entry_t * jgpt = (kmp_jgpt_entry_t*)kmp->dlist[KMP_JGPT].list;
	if ( KMP_TFORM & KMPTF_JGPT_POS )
	    TransformPosFloat3D(jgpt->position,n,sizeof(*jgpt));
	if ( KMP_TFORM & KMPTF_JGPT_ROTATE )
	    TransformRotFloat3D(jgpt->rotation,n,sizeof(*jgpt));
    }

    //--- KTPT

    n = kmp->dlist[KMP_KTPT].used;
    if (n)
    {
	kmp_ktpt_entry_t * ktpt = (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;
	if ( KMP_TFORM & KMPTF_KTPT_POS )
	    TransformPosFloat3D(ktpt->position,n,sizeof(*ktpt));
	if ( KMP_TFORM & KMPTF_KTPT_ROTATE )
	    TransformRotFloat3D(ktpt->rotation,n,sizeof(*ktpt));
    }

    //--- MSPT

    n = kmp->dlist[KMP_MSPT].used;
    if (n)
    {
	kmp_mspt_entry_t * mspt = (kmp_mspt_entry_t*)kmp->dlist[KMP_MSPT].list;
	if ( KMP_TFORM & KMPTF_MSPT_POS )
	    TransformPosFloat3D(mspt->position,n,sizeof(*mspt));
	if ( KMP_TFORM & KMPTF_MSPT_ROTATE )
	    TransformRotFloat3D(mspt->rotation,n,sizeof(*mspt));
    }

    //--- POTI

#if 1
    if ( KMP_TFORM & KMPTF_POTI_POS )
    {
	const uint ng = kmp->dlist[KMP_POTI].used;
	const kmp_poti_group_t *pg = (kmp_poti_group_t*)kmp->dlist[KMP_POTI].list;
//	const uint np = kmp->poti_point.used;
	kmp_poti_point_t *pp = (kmp_poti_point_t*)kmp->poti_point.list;

	uint gi;
	for ( gi = 0; gi < ng; gi++, pg++ )
	{
	    if ( gi >= sizeof(poti_notrans) || !poti_notrans[gi] )
		TransformPosFloat3D(pp->position,pg->n_point,sizeof(*pp));

	    pp += pg->n_point;
	}
    }
#else
    n = kmp->poti_point.used;
    if ( n && KMP_TFORM & KMPTF_POTI_POS )
    {
	kmp_poti_point_t * pp = (kmp_poti_point_t*)kmp->poti_point.list;
	TransformPosFloat3D(pp->position,n,sizeof(*pp));
    }
#endif

    //--- STGI
    // nothing to do

    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  Patch KMP helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static u8 rm_kmp_gobj[N_KMP_GOBJ] = {0};
static bool have_rm_kmp_gobj = false;

///////////////////////////////////////////////////////////////////////////////

int ScanOptRmGobj ( ccp arg )
{
    if (have_rm_kmp_gobj)
	have_patch_count--, have_kmp_patch_count--;
    have_rm_kmp_gobj = false;
    memset(rm_kmp_gobj,0,sizeof(rm_kmp_gobj));
    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --rm-gobj",0);
    si.predef = SetupVarsKMP();
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);
    sf->disable_comma++;
    for(;;)
    {
	while ( *sf->ptr > 0 && *sf->ptr <= ' ' || *sf->ptr == ',' )
	    sf->ptr++;
	if (!*sf->ptr)
	    break;

	DEFINE_VAR(val);
	if (ScanExprSI(&si,&val))
	    goto abort;
	int idx1 = GetIntV(&val);
	int idx2 = idx1;
	if ( NextCharSI(&si,false) == ':' )
	{
	    sf->ptr++;
	    if (ScanExprSI(&si,&val))
		goto abort;
	    idx2 = GetIntV(&val);
	}

	if ( idx1 < 0 )
	     idx1 = 0;
	if ( idx2 > N_KMP_GOBJ - 1 )
	     idx2 = N_KMP_GOBJ - 1;

	while ( idx1 <= idx2 )
	{
	    rm_kmp_gobj[idx1++] = 1;
	    have_rm_kmp_gobj = true;
	}
    }

    if (have_rm_kmp_gobj)
	have_patch_count++, have_kmp_patch_count++;

    #if defined(DEBUG) && defined(TEST)
    {
	PRINT("## have_rm_kmp_gobj = %d [%d,%d]\n",
		have_rm_kmp_gobj, have_patch_count, have_kmp_patch_count );
	uint i;
	for ( i = 0; i < N_KMP_GOBJ; i++ )
	    if (rm_kmp_gobj[i])
		PRINT("%6u,0x%02x := 0x%02x\n", i, i, rm_kmp_gobj[i] );
    }
    #endif

    return 0;

 abort:
    memset(rm_kmp_gobj,0,sizeof(rm_kmp_gobj));
    return 1;
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptBattle ( ccp arg )
{
    const int stat = ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_FORCE,"Option --battle");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_battle_mode = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptExportFlags ( ccp arg )
{
    const int stat = ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_FORCE,"Option --export-flags");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_export_flags = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptRouteOptions ( ccp arg )
{
    const int stat = ScanKeywordOffAutoOn(arg,OFFON_ON,12,"option --route-options");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_route_options = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptWim0 ( ccp arg )
{
    const int stat = ScanKeywordOffAutoOn(arg,OFFON_ON,12,"option --wim0");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_wim0 = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static bool FixPH
(
    kmp_t		* kmp,		// KMP data structure
    uint		sect_ph,	// KMP PH section index
    uint		sect_pt,	// related PT section index
    bool		allow_dup	// true: allow multiple links into same section
)
{
    DASSERT(kmp);
    DASSERT( sect_pt < KMP_N_SECT );
    DASSERT( sect_ph < KMP_N_SECT );

    //--- setup

    PRINT("FixPH(%s,%s,%d)\n",
		kmp_section_name[sect_ph].name1,
		kmp_section_name[sect_pt].name1, allow_dup );

    List_t *ph_list = kmp->dlist + sect_ph;
    List_t *pt_list = kmp->dlist + sect_pt;
    const uint max_pt = pt_list->used;


    //--- save in-data

    const uint saved_n = ph_list->used;
    const uint saved_size = saved_n * ph_list->elem_size;
    u8 *saved_data = MALLOC(saved_size);
    memcpy(saved_data,ph_list->list,saved_size);


    //--- first loop: find valid groups

    u8 linkref[0x100];
    memset(linkref,0xff,sizeof(linkref));

    uint pi, idx = 0;
    kmp_enph_entry_t *ph, *dest, *ph0 = (kmp_enph_entry_t*)ph_list->list;
    for ( pi = 0, ph = dest = ph0; pi < ph_list->used; pi++, ph++ )
    {
	uint end = ph->pt_start + ph->pt_len;
	if ( end > max_pt || pi == ph_list->used-1 )
	    end = max_pt;
	if ( end > idx )
	{
	    dest->pt_start = idx;
	    dest->pt_len   = end - idx;
	    idx = end;
	    memset(dest->prev,0xff,sizeof(dest->prev));
	    memcpy(dest->next,ph->next,sizeof(dest->next));
	    linkref[pi] = dest - ph0;
	    dest++;
	}
    }
    ph_list->used = dest - ph0;


    //--- second loop: analyse next links

    for ( pi = 0, ph = ph0; pi < ph_list->used; pi++, ph++ )
    {
	u8 new_next[KMP_MAX_PH_LINK];
	memset(new_next,0xff,sizeof(new_next));

	uint il;
	for ( il = 0; il < KMP_MAX_PH_LINK; il++ )
	{
	    const u8 link = linkref[ph->next[il]&0xff];
	    if ( link < ph_list->used )
	    {
		InsertLinkPH(new_next,link,0,0,allow_dup);
		InsertLinkPH(ph0[link].prev,pi,0,0,false);
	    }
	}

	if ( new_next[0] == 0xff )
	{
	    // link to self
	    InsertLinkPH(new_next,pi,0,0,allow_dup);
	}

	memcpy(ph->next,new_next,sizeof(ph->next));
    }


    //--- third loop: find empty 'prev' lists

    for ( pi = 0, ph = ph0; pi < ph_list->used; pi++, ph++ )
    {
	if ( ph->prev[0] == 0xff )
	{
	    // link to self
	    InsertLinkPH(ph->prev,pi,0,0,false);
	}
    }


    //--- calc dirty

    const bool dirty = saved_n != ph_list->used
			|| memcmp(saved_data,ph_list->list,saved_size);
    FREE(saved_data);

    if (dirty)
	KMP_ACTION_LOG(true,"Section %s fixed.\n",kmp_section_name[sect_ph].name1);
    return dirty;
}

///////////////////////////////////////////////////////////////////////////////

static kmp_gobj_entry_t * AppendGOBJ ( kmp_t * kmp, u16 obj_id )
{
    kmp_gobj_entry_t *go = AppendList(kmp->dlist + KMP_GOBJ);
    if (go)
    {
	go->obj_id	= obj_id;
	go->route_id	= M1(go->route_id);
	go->pflags	= 0x3f;
	go->scale[0]	=
	go->scale[1]	=
	go->scale[2]	= 1.0;
    }
    return go;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  Patch KMP			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool ReversePT
(
    const kmp_t		* kmp,		// KMP data structure
    uint		sect		// KMP PT section index
)
{
    DASSERT(kmp);
    DASSERT( sect == KMP_ENPT || sect == KMP_ITPT );

    const List_t *ptlist, *phlist;
    if ( sect == KMP_ENPT )
    {
	ptlist = kmp->dlist + KMP_ENPT;
	phlist = kmp->dlist + KMP_ENPH;
    }
    else
    {
	DASSERT( sect == KMP_ITPT );
	ptlist = kmp->dlist + KMP_ITPT;
	phlist = kmp->dlist + KMP_ITPH;
    }

    //--- reverse points

    const uint np = ptlist->used;
    if ( np < 2 )
	return false;

    kmp_enpt_entry_t *p1 = (kmp_enpt_entry_t*)ptlist->list;
    kmp_enpt_entry_t *p2 = p1 + np - 1;
    while ( p1 < p2 )
    {
	kmp_enpt_entry_t temp = *p1;
	*p1++ = *p2;
	*p2-- = temp;
    }

    //--- reverse groups

    const uint ng = phlist->used;
    const uint ng1 = ng - 1;
    kmp_enph_entry_t *g1 = (kmp_enph_entry_t*)phlist->list;
    kmp_enph_entry_t *g2 = g1 + ng1;
    while ( g1 < g2 )
    {
	kmp_enph_entry_t temp = *g1;
	*g1++ = *g2;
	*g2-- = temp;
    }

    //--- fix groups

    g1 = (kmp_enph_entry_t*)phlist->list;
    g2 = g1 + ng;
    for ( ; g1 < g2; g1++ )
    {
	kmp_enph_entry_t temp = *g1;

	uint i;
	for ( i = 0; i < KMP_MAX_PH_LINK; i++ )
	{
	    g1->prev[i] = temp.next[i] == 0xff ? 0xff : ng1 - temp.next[i];
	    g1->next[i] = temp.prev[i] == 0xff ? 0xff : ng1 - temp.prev[i];
	}

	g1->pt_start = np - g1->pt_start - g1->pt_len;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool ReversePOTI
(
    const kmp_t		* kmp		// KMP data structure
)
{
    DASSERT(kmp);

    const uint ng = kmp->dlist[KMP_POTI].used;
    kmp_poti_group_t *pg = (kmp_poti_group_t*)kmp->dlist[KMP_POTI].list;
    const uint np = kmp->poti_point.used;
    kmp_poti_point_t *pp = (kmp_poti_point_t*)kmp->poti_point.list;

    uint pi, gi;
    for ( gi = pi = 0; gi < ng; gi++ )
    {
	uint en = pg[gi].n_point;
	if ( en > np - pi )
	     en = np - pi;

	kmp_poti_point_t *p1 = pp + pi;
	kmp_poti_point_t *p2 = p1 + en - 1;
	PRINT("==> POTI %02u: %d..%d /%u\n",gi,(int)(p1-pp),(int)(p2-pp),np);
	pi += en;

	while ( p1 < p2 )
	{
	    kmp_poti_point_t temp = *p1;
	    *p1++ = *p2;
	    *p2-- = temp;
	}
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool ReverseCKPT
(
    const kmp_t		* kmp		// KMP data structure
)
{
    DASSERT(kmp);
    // [[2do]] ???
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool ReverseKTPT
(
    const kmp_t		* kmp		// KMP data structure
)
{
    DASSERT(kmp);
    // [[2do]] ???
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool ReverseJGPT
(
    const kmp_t		* kmp		// KMP data structure
)
{
    DASSERT(kmp);
    // [[2do]] ???
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool ReverseKMP
(
    kmp_t		* kmp		// pointer to valid KMP
)
{
    DASSERT(kmp);
    return ReversePT(kmp,KMP_ENPT)
	 | ReversePT(kmp,KMP_ITPT)
	 | ReversePOTI(kmp)
	 | ReverseCKPT(kmp)	// [[2do]]
	 | ReverseKTPT(kmp)	// [[2do]]
	 | ReverseJGPT(kmp);	// [[2do]]
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP tiny activities		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum kmp_tiny_mode_t
{
    KTM_OFF,		// do nothing
    KTM_ROUND_TO_INT,	// round to next int

    KTM_ROUND_POS,	// standard rounding for positions
    KTM_ROUND_ROT,	// standard rounding for rotatins
    KTM_ROUND_SCALE,	// standard rounding for scalings

    KTM_ROUND_ROUTE,	// for route positions
    KTM_ROUND_CANNON,	// for cannon rotation
}
kmp_tiny_mode_t;

typedef struct kmp_tiny_t
{
    u16		sect;	    // KMP section index
    u16		offset;	    // offset within KMP section
    u16		n;	    // number of related floats, usually 1 or 3
    u16		mode;	    // one of kmp_tiny_mode_t
}
kmp_tiny_t;


static const kmp_tiny_t kmp_tiny_tab[] =
{
    // sections are steadily increasing!!

    { KMP_KTPT, offsetof(kmp_ktpt_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_KTPT, offsetof(kmp_ktpt_entry_t,rotation),	3, KTM_ROUND_ROT },

    { KMP_ENPT, offsetof(kmp_enpt_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_ENPT, offsetof(kmp_enpt_entry_t,scale),	3, KTM_ROUND_SCALE },

    { KMP_ITPT, offsetof(kmp_itpt_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_ITPT, offsetof(kmp_itpt_entry_t,scale),	3, KTM_ROUND_SCALE },

    { KMP_CKPT, offsetof(kmp_ckpt_entry_t,left),	4, KTM_ROUND_POS },

    { KMP_GOBJ, offsetof(kmp_gobj_entry_t,position),	3, KTM_ROUND_POS },
    { KMP_GOBJ, offsetof(kmp_gobj_entry_t,rotation),	3, KTM_ROUND_ROT },
    { KMP_GOBJ, offsetof(kmp_gobj_entry_t,scale),	3, KTM_ROUND_SCALE },

    { KMP_POTI, offsetof(kmp_poti_point_t,position),	3, KTM_ROUND_ROUTE },

    { KMP_AREA, offsetof(kmp_area_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_AREA, offsetof(kmp_area_entry_t,rotation),	3, KTM_ROUND_ROT },
    { KMP_AREA, offsetof(kmp_area_entry_t,scale),	3, KTM_OFF },

    { KMP_CAME, offsetof(kmp_came_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_CAME, offsetof(kmp_came_entry_t,rotation),	3, KTM_ROUND_ROT },
    { KMP_CAME, offsetof(kmp_came_entry_t,zoom_begin),	2, KTM_ROUND_ROUTE },
    { KMP_CAME, offsetof(kmp_came_entry_t,viewpt_begin),6, KTM_ROUND_ROUTE },
    { KMP_CAME, offsetof(kmp_came_entry_t,time),	1, KTM_OFF },

    { KMP_JGPT, offsetof(kmp_jgpt_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_JGPT, offsetof(kmp_jgpt_entry_t,rotation),	3, KTM_ROUND_ROT },

    { KMP_CNPT, offsetof(kmp_cnpt_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_CNPT, offsetof(kmp_cnpt_entry_t,rotation),	3, KTM_ROUND_CANNON },

    { KMP_MSPT, offsetof(kmp_mspt_entry_t,position),	3, KTM_ROUND_ROUTE },
    { KMP_MSPT, offsetof(kmp_mspt_entry_t,rotation),	3, KTM_ROUND_ROT },

    { KMP_N_SECT,0,0,0} // sections are steadily increasing!!
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  Patch KMP			///////////////
///////////////////////////////////////////////////////////////////////////////

bool PatchKMP
(
    kmp_t		* kmp		// pointer to valid KMP
)
{
    DASSERT(kmp);
    bool dirty = ( KMP_MODE & KMPMD_NEW ) != 0;
    if ( have_patch_count <= 0 || have_kmp_patch_count <= 0 && !transform_active )
	return dirty;

    KMP_ACTION_LOG(false,"PatchKMP()\n");
    PRINT("** PatchKMP() **\n");

    dirty |= TransformKMP(kmp);


    //--- remove global objects (--rm-gobj, --slot)

    if ( opt_slot & SLOTMD_RM_ICE )
    {
	rm_kmp_gobj[GOBJ_ICE] = 1;
	have_rm_kmp_gobj = true;
    }
    if ( opt_slot & SLOTMD_RM_SUNDS )
    {
	rm_kmp_gobj[GOBJ_SUN_DS] = 1;
	rm_kmp_gobj[GOBJ_PYLON01] = 1;
	have_rm_kmp_gobj = true;
    }
    if ( opt_slot & SLOTMD_RM_SHYGUY )
    {
	rm_kmp_gobj[GOBJ_HEYHO_SHIP] = 1;
	rm_kmp_gobj[GOBJ_HEYHO_BALL] = 1;
	have_rm_kmp_gobj = true;
    }

    if (have_rm_kmp_gobj)
    {
	kmp_gobj_entry_t *glist = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	kmp_gobj_entry_t *src  = glist;
	kmp_gobj_entry_t *dest = glist;
	kmp_gobj_entry_t *end  = glist + kmp->dlist[KMP_GOBJ].used;
	for ( ; src < end; src++ )
	{
	    if (!rm_kmp_gobj[src->obj_id])
	    {
		memcpy(dest,src,sizeof(*dest));
		dest++;
	    }
	    else
		RemoveIL(kmp->index+KMP_GOBJ,dest-glist);
	}

	if ( dest < end )
	{
	    dirty = true;
	    kmp->dlist[KMP_GOBJ].used = dest - glist;
	}
    }


    //--- SLOTMD_ADD_SHYGUY

    if ( opt_slot & (SLOTMD_ADD_GICE|SLOTMD_ADD_SHYGUY) )
    {
	const kmp_gobj_entry_t *glist = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	const kmp_gobj_entry_t *gobj, *gend  = glist + kmp->dlist[KMP_GOBJ].used;
	bool ice_count = 0, shuyguy_count = 0;
	for ( gobj = glist; gobj < gend; gobj++ )
	{
	    if ( gobj->obj_id == GOBJ_ICE )
		ice_count++;
	    if ( gobj->obj_id == GOBJ_HEYHO_SHIP )
		shuyguy_count++;
	}

	if ( !ice_count && opt_slot & SLOTMD_ADD_GICE )
	{
	    PATCH_ACTION_LOG("Add","KMP","object ice\n");
	    AppendGOBJ(kmp,GOBJ_ICE);
	    dirty = true;
	}

	if ( !shuyguy_count && opt_slot & SLOTMD_ADD_SHYGUY )
	{
	    PATCH_ACTION_LOG("Add","KMP","object HeyhoShipGBA + route\n");

	    const uint faraway  = 150000;
	    const uint distance =   2000;
	    kmp_gobj_entry_t *go = AppendGOBJ(kmp,GOBJ_HEYHO_SHIP);
	    go->position[0] = go->position[2] = faraway;
	    go->position[1] = 1;
	    go->setting[0] = 20;
	    go->setting[1] = 100;
	    go->route_id = kmp->dlist[KMP_POTI].used;

	    kmp_poti_group_t * pg = AppendList(kmp->dlist+KMP_POTI);
	    pg->n_point = 3;

	    uint i;
	    for ( i = 0; i < 3; i++ )
	    {
		kmp_poti_point_t * pg = AppendList(&kmp->poti_point);
		pg->position[0] = faraway + ( i>0 ? distance : 0 );
		pg->position[1] = 1;
		pg->position[2] = faraway + ( i>1 ? distance : 0 );
	    }
	    dirty = true;
	}
    }


    //--- reverse track order: KMPMD_REVERSE

    if ( KMP_MODE & KMPMD_REVERSE )
	dirty |= ReverseKMP(kmp);


    //--- KMPMD_POLE_LEFT, KMPMD_POLE_RIGHT, KMPMD_SP_WIDE, KMPMD_SP_NARROW, KMPMD_*_LAPS

    if ( KMP_MODE & KMPMD_M_STGI || speed_mod_active )
    {
	int n = kmp->dlist[KMP_STGI].used;
	kmp_stgi_entry_t * stgi = (kmp_stgi_entry_t*)kmp->dlist[KMP_STGI].list;

	const uint new_lap_count = ( KMP_MODE & KMPMD_M_LAPS ) >> KMPMD_SHIFT_LAPS;
	const uint max_lap_count = KMP_MODE & KMPMD_MAX_LAPS ? new_lap_count : 0;
	bool laps_modified   = false;
	ccp  pole_modified   = 0;
	ccp  narrow_modified = 0;
	uint speed_modified  = false;
	u16  old_speed_mod = 0;

	uint i;
	for ( i = 0; i < n; i++, stgi++ )
	{
	    if ( new_lap_count
		&& stgi->lap_count != new_lap_count
		&& stgi->lap_count >= max_lap_count )
	    {
		stgi->lap_count = new_lap_count;
		laps_modified = true;
	    }

	    if ( KMP_MODE & KMPMD_POLE_LEFT && stgi->pole_pos != 0 )
	    {
		stgi->pole_pos = 0;
		pole_modified = "left";
	    }
	    if ( KMP_MODE & KMPMD_POLE_RIGHT && stgi->pole_pos != 1 )
	    {
		stgi->pole_pos = 1;
		pole_modified = "right";
	    }

	    if ( KMP_MODE & KMPMD_SP_WIDE && stgi->narrow_start != 0 )
	    {
		stgi->narrow_start = 0;
		narrow_modified = "wide";
	    }
	    if ( KMP_MODE & KMPMD_SP_NARROW && stgi->narrow_start != 1 )
	    {
		stgi->narrow_start = 1;
		narrow_modified = "narrow";
	    }

	    if ( speed_mod_active && stgi->speed_mod != speed_mod_val )
	    {
		old_speed_mod = stgi->speed_mod;
		stgi->speed_mod = speed_mod_val;
		speed_modified = true;
	    }
	}

	dirty |= laps_modified || pole_modified || narrow_modified || speed_modified;

	char lap_count_info[50] = {0};
	if (laps_modified)
	    snprintf(lap_count_info,sizeof(lap_count_info),
			" Lap counter set to '%u'.",new_lap_count);

	if ( pole_modified && narrow_modified )
	    KMP_ACTION_LOG(true,
		"STGI:%s Pole position set to '%s' and narrow mode to '%s'.\n",
		lap_count_info, pole_modified, narrow_modified );
	else if ( pole_modified )
	    KMP_ACTION_LOG(true,"STGI:%s Pole position set to '%s'.\n",
		lap_count_info, pole_modified );
	else if ( narrow_modified )
	    KMP_ACTION_LOG(true,"STGI:%s Narrow mode set to '%s'.\n",
		lap_count_info, narrow_modified);
	else if (laps_modified)
	    KMP_ACTION_LOG(true,"STGI:%s\n",lap_count_info);

	if (speed_modified)
	{
	    if ( n == 1 )
	    {
		if (speed_mod_val)
		    KMP_ACTION_LOG(true,
			"STGI: Speed modifier set from %5.3f to %5.3f (%04x->%04x/hex).\n",
			    SpeedMod2float(old_speed_mod), speed_mod_factor,
			    old_speed_mod, speed_mod_val );
		else
		    KMP_ACTION_LOG(true,
			"STGI: Speed modifier cleared (old: %5.3f = %04x/hex).\n",
			SpeedMod2float(old_speed_mod), old_speed_mod );
	    }
	    else
	    {
		if (speed_mod_val)
		    KMP_ACTION_LOG(true,"STGI: Speed modifier set to %5.3f (%04x/hex).\n",
			    speed_mod_factor, speed_mod_val );
		else
		    KMP_ACTION_LOG(true,"STGI: Speed modifier cleard.\n");
	    }
	}
    }


    //--- second KTPT

    bool k2_active = ktpt2_active;
    double3 k2_pos = ktpt2_pos;

    if ( k2_active && ktpt2_auto  )
    {
	kmp_finish_t kf;
	CheckFinishLine(kmp,&kf);
	k2_active = kf.valid && kf.enpt_rec >= 0 && kf.distance_rec >= ktpt2_min;
	k2_pos.x = kf.pos_ktpt_rec.x;
	k2_pos.y = kf.pos_ktpt_rec.y;
	k2_pos.z = kf.pos_ktpt_rec.z;
    }

    if (k2_active)
    {
	const int stat = InsertSecondKTPT(kmp,k2_pos);
	if ( stat && kmp->dlist[KMP_KTPT].used > 1 )
	{
	    const kmp_ktpt_entry_t *ktpt
			= (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list + 1;

	    KMP_ACTION_LOG(true,
		"KTPT: Second KTPT %s: position %1.0f,%1.0f,%1.0f (%4.2f°)\n",
		stat == 1 ? "inserted" : "replaced",
		ktpt->position[0], ktpt->position[1], ktpt->position[2],
		ktpt->rotation[1] );
	    dirty = true;
	}
    }


    //--- KMPMD_RM_SPCITEM

    if ( KMP_MODE & KMPMD_RM_SPCITEM )
    {
	const uint n = kmp->dlist[KMP_GOBJ].used;
	kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	uint i, count = 0;
	for ( i = 0; i < n; i++, gobj++ )
	{
	    switch (gobj->obj_id)
	    {
		case 0x65: // itembox
		case 0x76: // s_itembox
		case 0xc9: // f_itembox
		case 0xd4: // w_itembox
		case 0xd5: // w_itemboxline
		case 0xee: // sin_itembox
		    if (gobj->setting[1])
		    {
			gobj->setting[1] = 0;
			count++;
		    }
		    break;
	    }
	}
	if (count)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"%u special item%s for players removed.\n",
			count, count == 1 ? "" : "s" );
	}
    }


    //--- KMPMD_FIX_CKPH, KMPMD_FIX_ENPH, KMPMD_FIX_ITPH

    if ( KMP_MODE & KMPMD_FIX_CKPH )
	dirty |= FixPH(kmp,KMP_CKPH,KMP_CKPT,false);
    if ( KMP_MODE & KMPMD_FIX_ENPH )
	dirty |= FixPH(kmp,KMP_ENPH,KMP_ENPT,true);
    if ( KMP_MODE & KMPMD_FIX_ITPH )
	dirty |= FixPH(kmp,KMP_ITPH,KMP_ITPT,false);


    //--- KMPMD_FIX_CKNEXT & KMPMD_FIX_CKJGPT

    if ( KMP_MODE & (KMPMD_FIX_CKNEXT|KMPMD_FIX_CKJGPT) )
    {
	List_t *pt_list = kmp->dlist + KMP_CKPT;
	kmp_ckpt_entry_t *pt0 = (kmp_ckpt_entry_t*)pt_list->list;
	const uint max_pt = pt_list->used;

	if ( max_pt > 0 )
	{
	    if ( KMP_MODE & KMPMD_FIX_CKNEXT )
	    {
		// save data
		const uint saved_size = max_pt * pt_list->elem_size;
		u8 *saved_data = MALLOC(saved_size);
		memcpy(saved_data,pt_list->list,saved_size);

		uint ip;
		kmp_ckpt_entry_t *pt;
		for ( ip = 0, pt = pt0; ip < max_pt; ip++, pt++ )
		{
		    pt->prev = ip - 1;
		    pt->next = ip + 1;
		}
		pt0[max_pt-1].next = M1(pt0->next);

		uint ih;
		List_t *ph_list = kmp->dlist + KMP_CKPH;
		kmp_enph_entry_t *ph = (kmp_enph_entry_t*)ph_list->list;
		for ( ih = 0; ih < ph_list->used; ih++, ph++ )
		{
		    uint idx = ph->pt_start;
		    if ( idx < max_pt )
			pt0[idx].prev = M1(pt0->prev);

		    idx += ph->pt_len - 1;
		    if ( idx < max_pt )
			pt0[idx].next = M1(pt0->next);
		}

		const bool pt_dirty = memcmp(saved_data,pt_list->list,saved_size);
		FREE(saved_data);
		dirty |= pt_dirty;
		if (pt_dirty)
		    KMP_ACTION_LOG(true,"'prev' and 'next' of section CKPT fixed.\n");
	    }

	    if ( KMP_MODE & KMPMD_FIX_CKJGPT )
	    {
		const uint n_jgpt = kmp->dlist[KMP_JGPT].used;

		u8 last_valid = 0;
		uint ip, count = 0;
		kmp_ckpt_entry_t *pt;
		for ( ip = 0, pt = pt0; ip < max_pt; ip++, pt++ )
		{
		    if ( pt->respawn && pt->respawn >= n_jgpt )
		    {
			pt->respawn = last_valid;
			count++;
		    }
		    else
			last_valid = pt->respawn;
		}

		if (count)
		{
		    dirty = true;
		    KMP_ACTION_LOG(true,
			"Section CKPT: %u invalid respawn link%s fixed.\n",
			count, count==1 ? "" : "s" );
		}
	    }
	}
    }


    //--- KMPMD_MASK_PFLAGS

    if ( KMP_MODE & KMPMD_MASK_PFLAGS )
    {
	const uint n = kmp->dlist[KMP_GOBJ].used;
	kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	uint i, count_pflags = 0, count_padding = 0;
	for ( i = 0; i < n; i++, gobj++ )
	{
	    if ( gobj->pflags > 0x3f )
	    {
		gobj->pflags &= 0x3f;
		count_pflags++;
	    }
	    if ( gobj->ref_id )
	    {
		gobj->ref_id = 0;
		count_padding++;
	    }
	}
	if (count_pflags)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"GOBJ: %u presence flag%s masked by value 0x3f.\n",
			count_pflags, count_pflags == 1 ? "" : "s" );
	}
	if (count_padding)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"GOBJ: Padding %u element%s set to 0.\n",
			count_padding, count_padding == 1 ? "" : "s" );
	}
    }


    //--- KMPMD_RM_LECODE & KMPMD_PURGE_GOBJ

    if ( KMP_MODE & (KMPMD_RM_LECODE|KMPMD_PURGE_GOBJ) )
    {
	const uint n = kmp->dlist[KMP_GOBJ].used;
	kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	kmp_gobj_entry_t * dest = gobj;
	uint i, count_lecode = 0, count_invalid = 0;
	for ( i = 0; i < n; i++, gobj++ )
	{
	    const u16 relevant_id = gobj->obj_id & GOBJ_M_OBJECT;
	    if ( KMP_MODE & KMPMD_RM_LECODE && relevant_id != gobj->obj_id )
	    {
		count_lecode++;
		continue;
	    }

	    if ( KMP_MODE & KMPMD_PURGE_GOBJ
		&& gobj->obj_id <= GOBJ_MIN_DEF
		&& ( relevant_id >= N_KMP_GOBJ || !ObjectInfo[relevant_id].name ))
	    {
		count_invalid++;
		continue;
	    }

	    memcpy(dest,gobj,sizeof(*dest));
	    dest++;
	}
	kmp->dlist[KMP_GOBJ].used = dest - (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;

	if (count_lecode)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"GOBJ: %u LE-CODE object%s removed.\n",
			count_lecode, count_lecode == 1 ? "" : "s" );
	}
	if (count_invalid)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"GOBJ: %u invalid object%s removed.\n",
			count_invalid, count_invalid == 1 ? "" : "s" );
	}
    }


    //--- KMPMD_TEST1

    if ( KMP_MODE & KMPMD_TEST1 )
    {
	const uint n = kmp->dlist[KMP_GOBJ].used;
	kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	uint i;
	for ( i = 0; i < n; i++, gobj++ )
	    gobj->ref_id = MyRandom(0x10000);
	if (n)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"GOBJ: Padding of %u element%s set to random value.\n",
			n, n == 1 ? "" : "s" );
	}
    }

    //--- KMPMD_TEST2

    if ( KMP_MODE & KMPMD_TEST2 )
    {
	const uint n = kmp->dlist[KMP_GOBJ].used;
	kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	uint i, count = 0;
	for ( i = 0; i < n; i++, gobj++ )
	{
	    const u16 new_id = gobj->obj_id & GOBJ_M_OBJECT | GOBJ_M_LECODE;
	    if ( gobj->obj_id != new_id )
	    {
		gobj->obj_id = new_id;
		count++;
	    }
	}
	if (count)
	{
	    dirty = true;
	    KMP_ACTION_LOG(true,"GOBJ: %u element%s set to invalid id ≥0x1000.\n",
			count, count == 1 ? "" : "s" );
	}
    }


    //--- repair xpflags

    RepairXPF(kmp);


    //--- tiny

 #if defined(DEBUG) || defined(TEST)
    {
	// proof steadily increasing sections!!
	const kmp_tiny_t *kt;
	for ( kt = kmp_tiny_tab; kt->sect < KMP_N_SECT; kt++ )
	    ASSERT_MSG( kt->sect <= kt[1].sect,
		"kmp_tiny_tab[%zu] > kmp_tiny_tab[%zu] (%u>%u)\n",
		kt-kmp_tiny_tab, kt-kmp_tiny_tab+1,
		kt->sect, kt[1].sect );
    }
 #endif

    const int tiny_mode = ( KMP_MODE & KMPMD_M_TINY ) >> KMPMD_S_TINY;
    if ( tiny_mode > 0 )
    {
	DASSERT( tiny_mode < N_TINY_MODES );
	const tiny_param_t *tp = TinyParam + tiny_mode;

	uint sect;
	int round_val = 0;
	const kmp_tiny_t *kt = kmp_tiny_tab;
	for ( sect = 0; sect < KMP_N_SECT; sect++ )
	{
	    List_t *dlist = sect == KMP_POTI ? &kmp->poti_point : kmp->dlist+sect;
	    if (!dlist->used)
		continue;

	    while ( kt->sect < sect )
		kt++;
	    for ( ; kt->sect == sect; kt++ )
	    {
		PRINT("%2zu > %2u.%02u, n=%u, md=%u, elem=%3u*%2u\n",
			kt-kmp_tiny_tab, kt->sect, kt->offset, kt->n, kt->mode,
			dlist->used, dlist->elem_size );

		float *fp = (float*)( dlist->list + kt->offset );
		int n_elem = dlist->used;

		switch(kt->mode)
		{
		 case KTM_ROUND_TO_INT:
		    while ( n_elem-- > 0 )
		    {
			int i = kt->n;
			while ( i-- > 0 )
			    fp[i] = roundf(fp[i]);

			fp = (float*)( (u8*)fp + dlist->elem_size );
		    }
		    break;


		 case KTM_ROUND_POS:
		    round_val = tp->pos_round;
		    goto round_std;

		 case KTM_ROUND_ROT:
		    round_val = tp->rot_round;
		    goto round_std;

		 case KTM_ROUND_SCALE:
		    round_val = tp->scale_round;
		    goto round_std;

		 case KTM_ROUND_ROUTE:
		    round_val = tp->route_round;
		    goto round_std;

		 case KTM_ROUND_CANNON:
		    round_val = tp->cannon_round;

		 round_std:
		    if ( round_val < 99 )
		    {
			while ( n_elem-- > 0 )
			{
			    int i = kt->n;
			    while ( i-- > 0 )
				fp[i] = RoundF(fp[i],round_val);
			    fp = (float*)( (u8*)fp + dlist->elem_size );
			}
		    }
		    break;

		}
	    }
	}

	SortGOBJ(kmp,KMP_SORT_TINY);
    }


    //--- term

    return dirty;
}

///////////////////////////////////////////////////////////////////////////////

bool PatchRawDataKMP
(
    void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    if ( have_patch_count <= 0 || have_kmp_patch_count <= 0 && !transform_active )
	return false;

    KMP_ACTION_LOG(false,"PatchRawDataKMP()\n");
    PRINT("** PatchRawDataKMP() **\n");

    kmp_t kmp;
    bool stat = false;
    enumError err = ScanRawKMP(&kmp,true,data,data_size);
    if ( !err && PatchKMP(&kmp) )
    {
	stat = !CreateRawKMP(&kmp) && kmp.raw_data_size <= data_size;
	if (stat)
	    memcpy(data,kmp.raw_data,kmp.raw_data_size);
    }
    ResetKMP(&kmp);
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			detect KMP types		///////////////
///////////////////////////////////////////////////////////////////////////////

void GetRouteTypeInfoKMP
(
    kmp_rtype_info_t	*rti,	// store result here (never NULL)
    const struct kmp_t	*kmp,	// KMP data
    uint		sect	// section to check (PT or PH)
)
{
    DASSERT(rti);
    DASSERT(kmp);
    memset(rti,0,sizeof(*rti));

    switch(sect)
    {
	case KMP_ENPT:
	case KMP_ENPH:
	    rti->sect_pt = KMP_ENPT;
	    rti->sect_ph = KMP_ENPH;
	    break;

	case KMP_ITPT:
	case KMP_ITPH:
	    rti->sect_pt = KMP_ITPT;
	    rti->sect_ph = KMP_ITPH;
	    break;

	default:
	    ASSERT(0);
	    rti->sect_pt = rti->sect_ph = KMP_NO_SECT;
	    return;
    }

    const uint n = kmp->dlist[rti->sect_ph].used;
    const kmp_enph_entry_t *p = (kmp_enph_entry_t*)kmp->dlist[rti->sect_ph].list;

    rti->n_group = n;
    rti->n_point = kmp->dlist[rti->sect_pt].used;;

    uint i;
    for ( i = 0; i < n; i++, p++ )
    {
	if ( IsDispatchPointKMP(p) )	rti->n_dispatch++;
	else if ( p->pt_len > 1 )	rti->n_route2++;
	else if ( p->pt_len < 1 )	rti->n_route0++;
	else				rti->n_route1++;
    }

    PRINT("RouteType[%s/%s]: g=%u, pt=%u, routes[0,1,>1]=%u+%u+%u, dispatch=%u\n",
	kmp_section_name[rti->sect_ph].name1,
	kmp_section_name[rti->sect_pt].name1,
	rti->n_group, rti->n_point,
	rti->n_route0, rti->n_route1, rti->n_route2, rti->n_dispatch );
}

///////////////////////////////////////////////////////////////////////////////

IsArena_t IsArenaKMP ( const kmp_t * kmp )
{
    DASSERT(kmp);
    const int n_itpt = kmp->dlist[KMP_ITPT].used;
    const int n_ktpt = kmp->dlist[KMP_KTPT].used;
    const int n_mspt = kmp->dlist[KMP_MSPT].used;
    const int arena_count = 3* !kmp->dlist[KMP_CKPT].used
			  + ( !n_itpt ? 3 : n_itpt < 10 ? 2 : n_itpt < 20 ? 1 : 0 )
			  + ( n_ktpt < 8 ? 0 : n_ktpt > 12 ? 4 : n_ktpt - 8 )
			  + ( n_mspt < 4 ? n_mspt : 4 );

    noPRINT("IsArenaKMP() %d,%d,%d,%d => %d [battle=%d]\n",
		kmp->dlist[KMP_CKPT].used,
		kmp->dlist[KMP_ITPT].used,
		kmp->dlist[KMP_JGPT].used,
		kmp->dlist[KMP_MSPT].used,
		arena_count, opt_battle_mode);

    if ( arena_count >= 4 )
    {
	kmp_rtype_info_t rti;
	GetRouteTypeInfoKMP(&rti,kmp,KMP_ENPH);
	if ( rti.n_dispatch > 2 || rti.n_dispatch > 0 && arena_count >= 5 )
	    return ARENA_DISPATCH;
    }

    return arena_count >= 9 ? ARENA_FOUND
		: arena_count > 6 ? ARENA_MAYBE : ARENA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

IsArena_t CheckBattleModeKMP ( const kmp_t * kmp )
{
    const IsArena_t is_arena = IsArenaKMP(kmp);
    return opt_battle_mode <= OFFON_OFF
		? ARENA_NONE
		: opt_battle_mode == OFFON_AUTO || is_arena >= ARENA_FOUND
		? is_arena
		: ARENA_FOUND;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CheckFinishLine()		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeFinishLine ( kmp_finish_t *kf )
{
    DASSERT(kf);
    memset(kf,0,sizeof(*kf));
    kf->ckpt = kf->ckpt_opt = kf->ktpt = kf->enpt_rec = M1(kf->ckpt);
}

///////////////////////////////////////////////////////////////////////////////

double CalcCheckPointDistance
(
    const kmp_ckpt_entry_t	*ckpt,
    double			x,
    double			z
)
{
    DASSERT(ckpt);

    double3 rot;
    rot.x = rot.z = 0;
    rot.y = -CalcDirection2F(ckpt->left,ckpt->right) * ( 180 / M_PI );

    double3 base;
    base.x = x;
    base.y = 0;
    base.z = z;

    double3 pos;
    pos.x = ckpt->left[0];
    pos.y = base.y;
    pos.z = ckpt->left[1];

    Rotate(&base,&rot,&pos,0,1);
    return fabs( pos.x - base.x );
}

///////////////////////////////////////////////////////////////////////////////

void CalcFinishLine
(
    const kmp_t			*kmp,	// not NULL: use it for recommendation
    kmp_finish_t		*kf,	// valid data
    const kmp_ckpt_entry_t	*ckpt,	// valid CKPT
    const kmp_ktpt_entry_t	*ktpt	// valid KTPT
)
{
    DASSERT(kf);
    DASSERT(ckpt);
    DASSERT(ktpt);

    kf->enpt_rec = M1(kf->enpt_rec);

    kf->pos_ktpt.x = ktpt->position[0];
    kf->pos_ktpt.y = ktpt->position[1];
    kf->pos_ktpt.z = ktpt->position[2];

    double3 pos[2];
    pos[0].x = pos[0].y = 0;
    pos[0].z = 1.0;

    double3 rot;
    rot.x = ktpt->rotation[0];
    rot.y = ktpt->rotation[1];
    rot.z = ktpt->rotation[2];

    Rotate(0,&rot,pos,0,1);
    double direction = atan2(pos[0].x,pos[0].z) * ( 180 / M_PI );
    kf->dir_ktpt = fmod( 3600 + direction, 360 );
    if ( fabsf(kf->dir_ktpt) < 1e-3 )
	kf->dir_ktpt  = 0.0;

    direction = CalcDirection2F(ckpt->left,ckpt->right) * ( 180 / M_PI );
    direction = fmod( 3600 + direction, 360 );
    kf->dir_ckpt = fmod( direction + 90, 360 );
    if ( fabsf(kf->dir_ckpt) < 1e-3 )
	kf->dir_ckpt = 0.0;

    kf->dir_delta = fmod( kf->dir_ckpt - kf->dir_ktpt + 540, 360 ) - 180;
    if ( fabsf(kf->dir_delta) < 1e-3 )
	kf->dir_delta = 0.0;

    kf->hint = fabsf(kf->dir_delta) > 1.0;
    kf->warn = fabsf(kf->dir_delta) > 5.0;


    //--- calculate distance

    double3 base;
    base.x = ktpt->position[0];
    base.y = ktpt->position[1];
    base.z = ktpt->position[2];

    pos[0].x = ckpt->left[0];
    pos[0].z = ckpt->left[1];
    pos[1].x = ckpt->right[0];
    pos[1].z = ckpt->right[1];
    pos[0].y =pos[1].y = base.y;

    rot.x = rot.z = 0;
    rot.y = -direction;

    Rotate(&base,&rot,pos,sizeof(*pos),2);
    double3 *zmin, *zmax;
    if ( pos[0].z < pos[1].z )
    {
	zmin = pos+0;
	zmax = pos+1;
    }
    else
    {
	zmin = pos+1;
	zmax = pos+0;
    }

    if ( base.z < zmin->z )
    {
	kf->distance = LengthD(base.v,zmin->v);
	//pos[0].z = zmin->z;
	pos[0] = *zmin;
    }
    else if ( base.z > zmax->z )
    {
	kf->distance = LengthD(base.v,zmax->v);
	//pos[0].z = zmax->z;
	pos[0] = *zmax;
    }
    else
    {
	kf->distance = fabs( pos[0].x - base.x );
	pos[0].z = base.z;
    }
    rot.y = direction;
    Rotate(&base,&rot,pos,0,1);
    kf->pos_ckpt.x = pos[0].x;
    kf->pos_ckpt.y = pos[0].y;
    kf->pos_ckpt.z = pos[0].z;

    if ( kf->distance > 500.0 )
    {
	kf->hint |= 2;
	if ( kf->distance > 2000.0 )
	    kf->warn |= 2;
    }


    //--- recommendation

    if (!kmp)
	return;

    const uint ne = kmp->dlist[KMP_ENPT].used;
    const kmp_enpt_entry_t *enpt
			= (kmp_enpt_entry_t*)kmp->dlist[KMP_ENPT].list;

    double distance = 1e30;
    kmp_ktpt_entry_t temp;
    uint i;

    for ( i = 0; i < ne; i++, enpt++ )
    {
	const double x1 = enpt->position[0] - ckpt->left[0];
	const double z1 = enpt->position[2] - ckpt->left[1];
	const double x2 = enpt->position[0] - ckpt->right[0];
	const double z2 = enpt->position[2] - ckpt->right[1];
	const double dist = sqrt( x1*x1 + z1*z1 ) + sqrt( x2*x2 + z2*z2 );

	if ( dist < distance )
	{
	    distance = dist;
	    kf->enpt_rec = i;
	    memset(&temp,0,sizeof(temp));
	    memcpy(temp.position,enpt->position,sizeof(temp.position));
	}
    }

    PRINT0("ENPT: %d/%d |  %1.0f | %1.0f %1.0f %1.0f\n",
	kf->enpt_rec, ne, distance,
	temp.position[0], temp.position[1], temp.position[2] );

    if ( kf->enpt_rec >= 0 )
    {
	kmp_finish_t kf2;
	memset(&kf2,0,sizeof(kf2));
	CalcFinishLine(0,&kf2,ckpt,&temp);
	kf->pos_ktpt_rec = kf2.pos_ckpt;
	kf->dir_ktpt_rec = kf2.dir_ckpt;

	const double dx = kf->pos_ktpt.x - kf->pos_ktpt_rec.x;
	const double dz = kf->pos_ktpt.z - kf->pos_ktpt_rec.z;
	kf->distance_rec = sqrt ( dx*dx + dz*dz );

	PRINT("REC: %1.0f %1.0f %1.0f | %1.2f° | dist %1.0f\n",
		kf->pos_ktpt_rec.x, kf->pos_ktpt_rec.y, kf->pos_ktpt_rec.z,
		kf->dir_ktpt_rec, kf->distance_rec );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CheckFinishLine ( const kmp_t *kmp, kmp_finish_t *kf )
{
    DASSERT(kmp);
    DASSERT(kf);
    InitializeFinishLine(kf);


    //--- count CKPT_0 and find last lap counter

    const uint nc = kmp->dlist[KMP_CKPT].used;
    const kmp_ckpt_entry_t *ckpt0
			= (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;

    uint i;
    for ( i = 0; i < nc; i++ )
	if ( !ckpt0[i].mode )
	{
	    kf->n_ckpt_0++;
	    kf->ckpt = i;
	}


    //--- count KMPT<0

    const uint nk = kmp->dlist[KMP_KTPT].used;
    const kmp_ktpt_entry_t *ktpt
			= (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;

    for ( i = 0; i < nk; i++ )
	if ( ktpt[i].player_index < 0 && kf->n_ktpt_m++ < 2 )
	    kf->ktpt = i;

    if ( !kf->n_ckpt_0 || !kf->n_ktpt_m )
	return false;


    //--- calculate

    const kmp_ckpt_entry_t *ckpt = ckpt0 + kf->ckpt;
    ktpt += kf->ktpt;

    CalcFinishLine(kmp,kf,ckpt,ktpt);


    //--- find optimized value

    kmp_finish_t temp;
    memcpy(&temp,kf,sizeof(temp));
    double distance = 1e9;

    for ( i = 0; i < nc; i++ )
	if ( !ckpt0[i].mode )
	{
	    CalcFinishLine(kmp,&temp,ckpt0+i,ktpt);
	    if ( temp.distance < distance )
	    {
		distance = temp.distance;
		kf->ckpt_opt     = i;
		kf->pos_ktpt_opt = temp.pos_ckpt;
		kf->dir_ktpt_opt = temp.dir_ckpt;
	    }
	}


    //--- return true

    return kf->valid = true;
}

///////////////////////////////////////////////////////////////////////////////

ccp LogFinishLine ( kmp_finish_t *kf )
{
    if (!kf)
	return "-";

    char buf[100];
    uint len = snprintf(buf,sizeof(buf),
		"n=%d/%d, idx=%d/%d",
		kf->n_ckpt_0, kf->n_ktpt_m, kf->ckpt, kf->ktpt );

    if (kf->valid)
	len += snprintf(buf+len,sizeof(buf)-len,
		", dir=%4.2f/%4.2f >%4.2f, dist=%5.3f",
		kf->dir_ckpt, kf->dir_ktpt, kf->dir_delta, kf->distance );

    if (kf->warn)
	len += snprintf(buf+len,sizeof(buf)-len,", warn=%x",kf->warn);
    if ( kf->hint & ~kf->warn )
	len += snprintf(buf+len,sizeof(buf)-len,", hint=%x",kf->hint);

    return CopyCircBuf0(buf,len);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int InsertSecondKTPT
(
    // returns: 0=not possible, 1:inserted, 2:replaced

    kmp_t	*kmp,	    // valid KMP
    double3	pos	    // position to insert
)
{
    DASSERT(kmp);

    const uint nk = kmp->dlist[KMP_KTPT].used;
    kmp_ktpt_entry_t *ktpt = (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;

    if ( !nk || ktpt->player_index >= 0 )
	return 0;

    ktpt++;
    int stat = 2;
    if ( nk < 2 || ktpt->player_index >= 0 )
    {
	stat = 1;
	ktpt = InsertList(kmp->dlist+KMP_KTPT,1);
    }
    DASSERT(ktpt);

    memset(ktpt,0,sizeof(*ktpt));
    ktpt->position[0] = pos.x;
    ktpt->position[1] = pos.y > 0 ? pos.y : ktpt[-1].position[1];
    ktpt->position[2] = pos.z;
    ktpt->player_index = -1;


    kmp_finish_t kf;
    CheckFinishLine(kmp,&kf);
    if (kf.valid)
    {
	if ( kf.ckpt_opt >= 0 )
	{
	    ktpt->position[0] = kf.pos_ktpt_opt.x;
	    ktpt->position[1] = kf.pos_ktpt_opt.y;
	    ktpt->position[2] = kf.pos_ktpt_opt.z;
	    ktpt->rotation[1] = kf.dir_ktpt_opt;
	}
	else
	    ktpt->rotation[1] = kf.dir_ckpt;
    }
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CheckUsedPosKMP()		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeUsedPosObj ( kmp_usedpos_obj_t *obj )
{
    DASSERT(obj);
    memset(obj,0,sizeof(*obj));

    obj->min.x     = obj->min.y     = obj->min.z	= FLT_MAX;
    obj->max.x     = obj->max.y     = obj->max.z	= -FLT_MAX;
    obj->allowed.x = obj->allowed.y = obj->allowed.z	= 131071; // 2^17-1
    obj->shift.x   = obj->shift.y   = obj->shift.z	= 0.0;
    obj->stretch.x = obj->stretch.y = obj->stretch.z	= 1.0;
    obj->rating[0] = obj->rating[1] = obj->rating[2]	= WLEVEL_NA;

    obj->pos_factor_valid = false;
    obj->pos_factor = obj->stretch;
}

///////////////////////////////////////////////////////////////////////////////

void EvaluateUsedPosObj ( kmp_usedpos_obj_t *obj )
{
    DASSERT(obj);

    obj->valid = true;
    obj->useful = obj->pos_factor_valid = false;
    obj->max_rating = WLEVEL_OK;

    int i;
    for ( i = 0; i < 3; i++ )
    {
	WarnLevel_t rating = WLEVEL_OK;

	float temp  = fabsf(obj->min.v[i]);
	float value = fabsf(obj->max.v[i]);
	if ( value < temp )
	     value = temp;

	const float fail = obj->allowed.v[i];
	const float warn = fail -  5000.0;
	const float hint = fail - 10000.0;

	if      ( rating < WLEVEL_FAIL && value > fail ) rating = WLEVEL_FAIL;
	else if ( rating < WLEVEL_WARN && value > warn ) rating = WLEVEL_WARN;
	else if ( rating < WLEVEL_HINT && value > hint ) rating = WLEVEL_HINT;

	if ( i == 1 )
	{
	    // special handling of min.y
	    const float ymin = round(obj->min.y);
	    if      ( rating < WLEVEL_FAIL && ymin < 0.0 ) rating = WLEVEL_FAIL;
	    else if ( rating < WLEVEL_WARN && ymin < 1.0 ) rating = WLEVEL_WARN;
	    else if ( rating < WLEVEL_HINT && ymin < 5.0 ) rating = WLEVEL_HINT;
	}

	obj->rating[i] = rating;
	if ( obj->max_rating < rating )
	     obj->max_rating = rating;


	//-- pos factors

	const float factor = ( value + 25000 ) / fail;
	if ( factor > 1.0 )
	{
	    obj->pos_factor.v[i] = factor;
	    if ( factor > 1.1 )
		obj->pos_factor_valid = true;
	}
	else
	    obj->pos_factor.v[i] = 1.0;
    }

    obj->shift_active = obj->shift.x || obj->shift.y || obj->shift.z;
    obj->stretch_active =  obj->stretch.x > 1.0
			|| obj->stretch.y > 1.0
			|| obj->stretch.z > 1.0;
    obj->useful = obj->pos_factor_valid || obj->stretch_active;
}

///////////////////////////////////////////////////////////////////////////////

void ModifyUsedPosObj
	( kmp_usedpos_obj_t *obj, kmp_usedpos_obj_t *src, float3 *shift, float3 *stretch )
{
    DASSERT(obj);

    if (src)
	*obj = *src;

    bool useful = false;
    int i;
    for ( i = 0; i < 3; i++ )
    {
	if ( shift && shift->v[i] )
	{
	    useful = true;
	    obj->shift.v[i] += shift->v[i];
	    obj->min.v[i]   += shift->v[i];
	    obj->max.v[i]   += shift->v[i];
	}

	if ( stretch && stretch->v[i] != 1.0 )
	{
	    useful = true;
	    obj->stretch.v[i] *= stretch->v[i];
	    obj->allowed.v[i] *= stretch->v[i];
	}
    }
    EvaluateUsedPosObj(obj);  
    obj->useful = useful;

    if (src)
    {
	src->max_rating = WLEVEL_OK;
	for ( i = 0; i < 3; i++ )
	{
	    if ( src->rating[i] > WLEVEL_FIXED && src->rating[i] > obj->rating[i] )
		 src->rating[i] = obj->rating[i] > WLEVEL_FIXED
				? obj->rating[i] : WLEVEL_FIXED;
	    if ( src->max_rating < src->rating[i] )
		 src->max_rating = src->rating[i];
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

ccp GetUsedPosObjSuggestion
	( const kmp_usedpos_obj_t *obj, bool force, ccp return_if_empty )
{
    if ( obj && ( force || obj->max_rating >= WLEVEL_HINT ))
    {
	if ( obj->shift_active && obj->pos_factor_valid )
	    return PrintCircBuf(
		"--shift %1.0f,%1.0f,%1.0f & @ITEM-POS-FACTOR=v(%4.2f,%4.2f,%4.2f)",
		obj->shift.x, obj->shift.y, obj->shift.z,
		obj->pos_factor.x, obj->pos_factor.y, obj->pos_factor.z );

	if (obj->shift_active)
	    return PrintCircBuf("--shift %1.0f,%1.0f,%1.0f",
		obj->shift.x, obj->shift.y, obj->shift.z );

	if (obj->pos_factor_valid)
	    return PrintCircBuf("@ITEM-POS-FACTOR=v(%4.2f,%4.2f,%4.2f)",
		obj->pos_factor.x, obj->pos_factor.y, obj->pos_factor.z );
    }    
    return return_if_empty;
}

///////////////////////////////////////////////////////////////////////////////

void PrintUsedPosObj ( FILE *f, int indent, const kmp_usedpos_obj_t *obj )
{
    DASSERT(f);
    DASSERT(obj);
    indent = NormalizeIndent(indent);

    fprintf(f,"%*s" "[%u%u%u%u] mm=%1.0f,%1.0f,%1.0fk/%1.0f,%1.0f,%1.0fk, "
	"a=%1.0f,%1.0f,%1.0fk, ss=%1.0f,%1.0f,%1.0fk/%1.2f,%1.2f,%1.2f -> "
	"R=%d,%d,%d, P=%d/%1.2f,%1.2f,%1.2f\n",
	indent,"",
	obj->valid, obj->useful, obj->shift_active, obj->stretch_active,
	obj->min.x/1000, obj->min.y/1000, obj->min.z/1000, 
	obj->max.x/1000, obj->max.y/1000, obj->max.z/1000,
	obj->allowed.x/1000, obj->allowed.y/1000, obj->allowed.z/1000,
	obj->shift.x/1000, obj->shift.y/1000, obj->shift.z/1000,
	obj->stretch.x, obj->stretch.y, obj->stretch.z,
	obj->rating[0], obj->rating[1], obj->rating[2],
	obj->pos_factor_valid,
	obj->pos_factor.x, obj->pos_factor.y, obj->pos_factor.z );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeUsedPos ( kmp_usedpos_t *up )
{
    DASSERT(up);
    memset(up,0,sizeof(*up));

    InitializeUsedPosObj(&up->orig);
    up->orig_lex = up->center = up->center_lex = up->orig;
    up->suggest = &up->orig;
}

///////////////////////////////////////////////////////////////////////////////

static void min_max_used ( kmp_usedpos_t * up, const float *pos )
{
    DASSERT(up);
    DASSERT(pos);

    float3 *min = &up->orig.min;
    if ( min->x > pos[0] ) min->x = pos[0];
    if ( min->y > pos[1] ) min->y = pos[1];
    if ( min->z > pos[2] ) min->z = pos[2];

    float3 *max = &up->orig.max;
    if ( max->x < pos[0] ) max->x = pos[0];
    if ( max->y < pos[1] ) max->y = pos[1];
    if ( max->z < pos[2] ) max->z = pos[2];
}

//-----------------------------------------------------------------------------

void CheckUsedPos ( const kmp_t *kmp, kmp_usedpos_t *up )
{
    DASSERT(kmp);
    DASSERT(up);

    InitializeUsedPos(up);

    //--- get min and max values

    float3 *min = &up->orig.min;
    float3 *max = &up->orig.max;

    min->x = min->y = min->z = FLT_MAX;
    max->x = max->y = max->z = -FLT_MAX;

    uint i, n = kmp->dlist[KMP_ENPT].used;
    const kmp_enpt_entry_t * enpt = (kmp_enpt_entry_t*)kmp->dlist[KMP_ENPT].list;
    for ( i = 0; i < n; i++, enpt++ )
	min_max_used(up,enpt->position);

    n = kmp->dlist[KMP_ITPT].used;
    const kmp_itpt_entry_t * itpt = (kmp_itpt_entry_t*)kmp->dlist[KMP_ITPT].list;
    for ( i = 0; i < n; i++, itpt++ )
	min_max_used(up,itpt->position);

    if ( min->x > max->x )
    {
	min->x = min->y = min->z = NAN;
	*max = *min;
	up->orig_lex = up->center = up->center_lex = up->orig;
	return;
    }


    //--- evaluate standard settings

    EvaluateUsedPosObj(&up->orig);


    //--- evaluate with LEX:SET1

    up->orig_lex = up->orig;
    up->orig_lex.useful = false;

    PRINT0("lexinfo=%p [%d]\n",kmp->lexinfo, kmp->lexinfo ? kmp->lexinfo->set1_found : -1 );
    if ( kmp->lexinfo && kmp->lexinfo->set1_found )
    {
	ModifyUsedPosObj(&up->orig_lex,&up->orig,0,&kmp->lexinfo->set1.item_factor);
	if ( up->orig_lex.useful )
	    up->suggest = &up->orig_lex;
    }

    //--- evaluate centered

    kmp_usedpos_obj_t *obj = &up->center;
    InitializeUsedPosObj(obj);
    obj->min = up->orig.min;
    obj->max = up->orig.max;

    float3 shift;
    shift.x = shift.y = shift.z = 0;

    for ( i = 0; i < 3; i++ )
    {
	if ( up->orig.rating[i] > WLEVEL_OK )
	{
	    bool force_shift = false;
	    float shift_value = ( obj->min.v[i] + obj->max.v[i] ) / -2;
	    if ( i == 1 && obj->min.v[i] < 10.0 )
	    {
		// special case for Y
		const float limit = 100.0 - obj->min.v[i];
		if ( shift_value < limit )
		{
		     shift_value = limit;
		     force_shift = true;
		}
	    }

	    if ( force_shift || fabsf(shift_value) >= 1000.0 )
	    {
		obj->useful = true;
		shift.v[i] = shift_value;
	    }
	}
    }

    up->center_lex = up->center;
    if (obj->useful)
    {
	ModifyUsedPosObj(obj,0,&shift,0);
	if ( obj->useful )
	    up->suggest = obj;

	up->center_lex.useful = false;
	if ( obj->useful && kmp->lexinfo && kmp->lexinfo->set1_found )
	{
	    ModifyUsedPosObj(&up->center_lex,&up->center,0,&kmp->lexinfo->set1.item_factor);
	    if ( up->center_lex.useful )
		up->suggest = &up->center_lex;
	}
    }

 #if HAVE_WIIMM_EXT && 0
    PrintUsedPosObj(stderr,1,&up->orig);
    PrintUsedPosObj(stderr,1,&up->orig_lex);
    if (up->center.useful||1)
    {
	PrintUsedPosObj(stderr,1,&up->center);
	PrintUsedPosObj(stderr,1,&up->center_lex);
    }
    PrintUsedPosObj(stderr,1,up->suggest);
 #endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CheckWarnKMP ( kmp_t *kmp, kmp_usedpos_t *up )
{
    kmp_usedpos_t local_up;
    if (!up)
    {
	InitializeUsedPos(&local_up);
	up = &local_up;
    }

    CheckUsedPos(kmp,up);
    if ( up->suggest->max_rating >= WLEVEL_WARN )
	kmp->warn_bits |= 1 << WARNSZS_ITEMPOS;

    //-------------------------------------------------------------------------

    const uint mask = 1 << WARNSZS_SELF_ITPH;
    if (!(kmp->warn_bits & mask))
    {
	const List_t *phlist = kmp->dlist + KMP_ITPH;
	const int n_ph = phlist->used;

	if (n_ph)
	{
	    uint i;
	    for ( i = 0; i < n_ph; i++ )
	    {
		const kmp_itph_entry_t *ph = (kmp_itph_entry_t*)phlist->list + i;
		if ( ph->next[0] == i && ph->pt_len == 1 )
		{
		    kmp->warn_bits |= mask;
		    break;
		}
	    }
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			DetectSpecialKMP()		///////////////
///////////////////////////////////////////////////////////////////////////////
// order is important, compare enum have_kmp_obj_t and ct.wiimm.de

const kmp_obj_info_t have_kmp_info[HAVEKMP__N] =
{
    { 0, "woodbox-ht",	"Woodbox with fall height" },		// HAVEKMP_WOODBOX_HT
    { 0, "mushroom-car","penguin_m as mushroom car" },		// HAVEKMP_MUSHROOM_CAR
    { 0, "penguin-pos",	"Alternative start of penguin_m" },	// HAVEKMP_PENGUIN_POS
    { 0, "2ktpt",	"Second KTPT with index -1" },		// HAVEKMP_SECOND_KTPT
    { 0, "xpf",		"Extended presence flags (XPF)" },	// HAVEKMP_X_PFLAGS
    { 0, "xcond",	"XPF: Conditional object" },		// HAVEKMP_X_COND
    { 0, "xobj",	"XPF: Definition object" },		// HAVEKMP_X_DEFOBJ
    { 0, "xrnd",	"XPF: Random scenarios" },		// HAVEKMP_X_RANDOM
    { 0, "eprop-speed",	"Epropeller with speed setting" },	// HAVEKMP_EPROP_SPEED
    { 0, "coob-r",	"Conditional OOB by Riidefi" },		// HAVEKMP_COOB_R
    { 0, "coob-k",	"Conditional OOB by kHacker35000vr" },	// HAVEKMP_COOB_K
    { 0, "uoob",	"Unconditional OOB" },			// HAVEKMP_UOOB
};

///////////////////////////////////////////////////////////////////////////////

void DetectSpecialKMP
(
    const kmp_t		*kmp,		// pointer to valid KMP
    kmp_special_t	res		// store result here, never NULL
)
{
    DASSERT(kmp);
    DASSERT(res);
    memset(res,0,sizeof(kmp_special_t));

    //--- AREA

    const kmp_area_entry_t *area = (kmp_area_entry_t*)kmp->dlist[KMP_AREA].list;
    const kmp_area_entry_t *area_end = area + kmp->dlist[KMP_AREA].used;
    for ( ; area < area_end; area++ )
    {
	if ( area->type == 10 )
	{
	    if ( area->route == 0xff && area->setting[0] + area->setting[1] )
		res[HAVEKMP_COOB_R]++;
	    else if ( area->route == 1 )
		res[HAVEKMP_COOB_K]++;
	    else
		res[HAVEKMP_UOOB]++;
	}
    }


    //--- GOBJ

    const kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
    const kmp_gobj_entry_t *gobj_end = gobj + kmp->dlist[KMP_GOBJ].used;
    for ( ; gobj < gobj_end; gobj++ )
    {
	if ( gobj->pflags )
	{
	    const u16 relevant_id = GetRelevantObjectId(gobj->obj_id);
	    switch(relevant_id)
	    {
	      case 0x070: // woodbox
		if ( gobj->setting[7] && res[HAVEKMP_WOODBOX_HT] < 0xff )
		    res[HAVEKMP_WOODBOX_HT]++;
		break;

	      case 0x0d8: // penguin_m
		if ( gobj->setting[7] && res[HAVEKMP_MUSHROOM_CAR] < 0xff )
		    res[HAVEKMP_MUSHROOM_CAR]++;
		else if ( gobj->setting[1] && res[HAVEKMP_PENGUIN_POS] < 0xff )
		    res[HAVEKMP_PENGUIN_POS]++;
		break;

	      case 0x1a6: // Epropeller
		if ( gobj->setting[7] && res[HAVEKMP_EPROP_SPEED] < 0xff )
		    res[HAVEKMP_EPROP_SPEED]++;
		break;
	    }
	}
    }

    kmp_finish_t kf;
    CheckFinishLine(kmp,&kf);
    if ( kf.n_ktpt_m > 1 )
	res[HAVEKMP_SECOND_KTPT]++;

    kmp_pflags_t pf;
    CountPresenceFlags(kmp,&pf);
    res[HAVEKMP_X_PFLAGS] = pf.n_xpflags;
    res[HAVEKMP_X_COND]   = pf.n_obj_ref + pf.n_cond_ref;
    res[HAVEKMP_X_DEFOBJ] = pf.n_defobj;
    res[HAVEKMP_X_RANDOM] = pf.n_cond_rnd;
}

///////////////////////////////////////////////////////////////////////////////

ccp CreateSpecialInfoKMP
	( kmp_special_t special, bool add_value, ccp return_if_empty )
{
    static char buf[500];
    char *dest = buf;

    if (add_value)
    {
	uint i, val = 0;
	for ( i = 0; i < HAVEKMP__N; i++ )
	    if ( special[i] )
		val |= 1 << i;
	dest = snprintfE( dest, buf+sizeof(buf), "%u=" , val );
    }

    uint i;
    ccp sep = "";
    for ( i = 0; i < HAVEKMP__N; i++ )
	if ( special[i] )
	{
	    dest = StringCat2E(dest,buf+sizeof(buf),sep,have_kmp_info[i].name);
	    sep = ",";
	}

    return dest == buf ? return_if_empty : CopyCircBuf0(buf,dest-buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    CkeckKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[check_kmp_t]]

typedef struct check_kmp_t
{
    //--- param

    const kmp_t		* kmp;		// pointer to valid KMP
    CheckMode_t		mode;		// print mode

    //--- status

    int			warn_count;	// number of warnings
    int			hint_count;	// number of hints
    int			info_count;	// number of hints
    bool		head_printed;	// true: header line printed
    bool		info_printed;	// true: info line printed
    int			sect;		// last section
    int			index;		// last object index
    IsArena_t		is_arena;	// arena detected?
    ccp			t_track;	// track" or "arena"

    //--- misc data

    ColorSet_t		col;		// color setup

} check_kmp_t;

///////////////////////////////////////////////////////////////////////////////

ccp print_name ( const kmp_t * kmp, uint sect, uint index )
{
    DASSERT(kmp);
    DASSERT( sect < KMP_N_SECT );

    // cast because remove const
    IndexList_t *ill = (IndexList_t*)kmp->index + sect;
    ccp name = index < ill->used ? GetNameIL(ill,index) : 0;
    if ( !name || !name )
	return "";

    const uint bufsize = 50;
    char *buf = GetCircBuf(bufsize);
    snprintf(buf,bufsize," '%.46s'",name);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

// predefine function to avoid a warning because of __attribute__ usage
static void print_check_error
(
    check_kmp_t		* chk,		// valid data structure
    CheckMode_t		print_mode,	// NULL | CMOD_INFO|CMOD_HINT|CMOD_WARNING
    int			sect,		// section
    int			index,		// object index
    ccp			obj_type,	// NULL or object type
    ccp			format,		// format of message
    ...					// arguments

) __attribute__ ((__format__(__printf__,6,7)));

static void print_check_error
(
    check_kmp_t		* chk,		// valid data structure
    CheckMode_t		print_mode,	// NULL | CMOD_INFO|CMOD_HINT|CMOD_WARNING
    int			sect,		// section
    int			index,		// object index
    ccp			obj_type,	// NULL or object type
    ccp			format,		// format of message
    ...					// arguments

)
{
    DASSERT(chk);
    DASSERT(chk->kmp);

    ccp col; // message color
    ccp bol; // begin of line
    switch (print_mode&~CMOD_LECODE)
    {
	case CMOD_WARNING:
	    if (!( chk->mode & CMOD_WARNING ))
		return;
	    col = chk->col.warn;
	    bol = check_bowl;
	    chk->warn_count++;
	    break;

	case CMOD_HINT:
	    if (!( chk->mode & CMOD_HINT ))
		return;
	    col = chk->col.hint;
	    bol = check_bohl;
	    chk->hint_count++;
	    break;

	case CMOD_INFO:
	    if (!( chk->mode & CMOD_INFO ))
		return;
	    col = chk->col.info;
	    bol = check_boil;
	    chk->info_count++;
	    break;

	default:
	    col = 0;
	    bol = 0; // print head only
	    break;
    }

    if (!chk->head_printed)
    {
	chk->head_printed = true;
	if ( chk->mode & (CMOD_HEADER|CMOD_FORCE_HEADER) )
	    fprintf(stdlog,"%s* CHECK %s:%s%s\n",
			chk->col.heading, GetNameFF(chk->kmp->fform,0),
			chk->kmp->fname, chk->col.reset );
    }

    if (!bol)
	return;

    if (!chk->info_printed)
    {
	chk->info_printed = true;
	fprintf(stdlog,"  %s> All indices are zero based!%s\n",
			chk->col.info, chk->col.reset );
    }

    if ( sect != -99 && ( chk->sect != sect || chk->index != index ))
    {
	chk->sect  = sect;
	chk->index = index;

	if ( sect < 0 )
	    fprintf(stdlog,"  %s> Statistics%s\n",chk->col.info,chk->col.reset);
	else if ( sect == KMP_N_SECT )
	    fprintf(stdlog,"  %s> All sections%s\n",chk->col.info,chk->col.reset);
	else if ( index < 0 )
	    fprintf(stdlog,"  %s> Section %s%s%s\n",
			chk->col.info, kmp_section_name[sect].name1,
			index == -2 ? ", header" : "", chk->col.reset );
	else
	{
	    fprintf(stdlog,"  %s> Section %s, %s #%u of %u%s",
			chk->col.info,
			kmp_section_name[sect].name1,
			kmp_section_name[sect].name2,
			index, chk->kmp->dlist[sect].used,
			print_name(chk->kmp,sect,index) );

	    if (obj_type)
		fprintf(stdlog,", type '%s'%s\n",obj_type,chk->col.reset);
	    else
		fprintf(stdlog,"%s\n",chk->col.reset);
	}
    }

    if (format)
    {
	fputs(col,stdlog);
	if ( print_mode & CMOD_LECODE )
	{
	    ccp colon = strchr(bol,':');
	    fprintf(stdlog,"%.*s/LE-CODE: ",(int)(colon?colon-bol:100),bol);
	}
	else
	    fputs(bol,stdlog);
	fputs(chk->col.reset,stdlog);

	va_list arg;
	va_start(arg,format);
	vfprintf(stdlog,format,arg);
	va_end(arg);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void print_item_hint
(
    check_kmp_t		* chk,		// valid data structure
    uint		n_total,	// total number of special item boxes
    uint		n_unsure,	// unsure number of special item boxes
    ccp			dest		// "players" or "enemies"
)
{
    DASSERT(chk);
    DASSERT(dest);

    if (n_total)
    {
	char buf[25];
	if (n_unsure)
	    snprintf(buf,sizeof(buf)," (%u unsure)",n_unsure);
	else
	    *buf = 0;

	if ( n_total > 1 )
	    print_check_error(chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u item boxes give special items to %s%s.\n",
		n_total, dest, buf );
	else
	    print_check_error(chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"One item box gives a special item to %s%s.\n",
		dest, buf );
    }
}

///////////////////////////////////////////////////////////////////////////////

static void check_pt
(
    check_kmp_t		*chk,		// valid data structure
    int			sect_pt,	// PT section
    kmp_path_t		*ck_path	// path data, already initialized
					//	data is calculated in first call
)
{
    DASSERT(chk);
    DASSERT(chk->kmp);
    DASSERT( sect_pt == KMP_ENPT || sect_pt == KMP_ITPT );
    DASSERT(ck_path);

    uint n = chk->kmp->dlist[sect_pt].used;
    if ( n > KMP_MAX_POINT )
	 n = KMP_MAX_POINT;
    const kmp_enpt_entry_t *pt = (kmp_enpt_entry_t*)chk->kmp->dlist[sect_pt].list;

    if ( chk->is_arena >= ARENA_FOUND )
    {
	if ( n && sect_pt == KMP_ITPT )
	    print_check_error(chk,CMOD_HINT,sect_pt,-1,0,
		"%u element%s found, but ITPT not needed for battle arenas.\n",
		n, n==1 ? "" : "s" );
    }
    else
    {
	if ( !n && ( sect_pt != KMP_ITPT || chk->is_arena == ARENA_NONE ))
	    print_check_error(chk,CMOD_WARNING,sect_pt,-1,0,
			"No elements found, but needed for racing tracks.\n" );

	u8 fail[KMP_MAX_POINT];
	memset(fail,0,sizeof(fail));

	uint i, fail_count = 0;
	const kmp_link_t *last = 0;
	for ( i = 0; i < n; i++, pt++ )
	{
	    const kmp_link_t *link
		    = FindCKPT(chk->kmp,ck_path,pt->position[0],pt->position[2],last);
	    if (link)
		last = link;
	    else
		fail[i]++, fail_count++;
	}

	if ( fail_count )
	{
	    uint i;
	    for ( i = 0; i < n; i++ )
		if (fail[i])
		{
		    const int start = i;
		    while ( ++i < n && fail[i] )
			;
		    if ( --i == start )
			print_check_error(chk,CMOD_WARNING,sect_pt,-1,0,
			    "Point #%u is outside of all convex check quadrilaterals"
			    " (pos bug?).\n",
			    i );
		    else
		    {
			print_check_error(chk,CMOD_WARNING,sect_pt,-1,0,
			    "Points #%u..#%u are outside of all convex check quadrilaterals"
			    " (pos bug?).\n",
			    start, i );
		    }
		}
	}
    }

    if ( sect_pt == KMP_ENPT ? chk->kmp->enpt_flag.import : chk->kmp->itpt_flag.import )
	print_check_error(chk,CMOD_HINT,sect_pt,-1,0,
				"Exported flags detected (extension of tool wkmpt).\n");
}

///////////////////////////////////////////////////////////////////////////////

static void check_ph
(
    check_kmp_t		*chk,		// valid data structure
    int			sect_ph,	// PH section
    int			sect_pt,	// PT section
    kmp_path_t		*ck_path	// ckeck point path data, already initialized
					//	data is calculated in first call
)
{
    DASSERT(chk);
    const kmp_t *kmp = chk->kmp;
    DASSERT(kmp);

    const List_t *ph_list = kmp->dlist + sect_ph;
    const kmp_enph_entry_t *ph = (kmp_enph_entry_t*)ph_list->list;
    const uint n_ph = ph_list->used;

    const List_t *pt_list = kmp->dlist + sect_pt;
    const uint n_pt = pt_list->used;

    if ( !n_ph && ( sect_ph == KMP_ENPT || chk->is_arena < ARENA_FOUND ) )
    {
	print_check_error(chk,CMOD_WARNING,sect_ph,-1,0,
		"Empty section => track will freeze.\n" );
    }

    uint i, next_idx = 0;
    for ( i = 0; i < n_ph; i++, ph++ )
    {
	if ( ph->pt_start >= n_pt )
	    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
		"Route group starts at point %u, but max possible index is %u.\n",
		ph->pt_start, n_pt );
	else if ( ph->pt_start + ph->pt_len > n_pt )
	    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
		"Route group ends at point %u, but max possible index is %u.\n",
		ph->pt_start + ph->pt_len, n_pt );
	else if ( ph->pt_start != next_idx )
	    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
		"Route group starts at point %u,"
		" but %u is the expected is start point.\n",
		ph->pt_start, next_idx );
	next_idx = ph->pt_start + ph->pt_len;


	//--- test 'prev' links

	uint link, count;
	for ( link = count = 0; link < KMP_MAX_PH_LINK; link++ )
	{
	    if ( !IS_M1(ph->prev[link]) )
	    {
		if ( ph->prev[link] >= n_ph )
		    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
			"Invalid 'prev' link at index #%u:"
			" Link points to group %u, but max allowed is %u.\n",
			link, ph->prev[link], n_ph-1 );
		else
		    count++;
	    }
	}

	if ( !count && chk->is_arena < ARENA_FOUND )
	    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
		"No valid 'prev' link found. Define at least one link.\n" );


	//--- test 'next' links

	for ( link = count = 0; link < KMP_MAX_PH_LINK; link++ )
	{
	    if ( !IS_M1(ph->next[link]) )
	    {
		if ( ph->next[link] >= n_ph )
		    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
			"Invalid 'next' link at index #%u:"
			" Link points to group %u, but max allowed is %u.\n",
			link, ph->next[link], n_ph-1 );
		else
		    count++;
	    }
	    else if ( !link && sect_ph == KMP_ITPH )
	    {
		print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
			"Missing 'next' link at index #0."
			" It's needed for the standard itemroute.\n" );
	    }
	}

	if (!count)
	    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
		"No valid 'next' link found => track will freeze."
		" Define at least one link.\n" );

	if ( sect_ph == KMP_ITPH )
	{
	    if ( ph->next[0] == i )
	    {
		if ( ph->pt_len == 1 )
		    print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
			"Self linked item route with exact 1 point:"
			" This will force a heavy lag if a red shell arrives.\n" );
		else
		    print_check_error(chk,CMOD_HINT,sect_ph,i,0,
			"Self linked item route.\n" );
	    }
	    else if ( ph->pt_len == 1 )
		print_check_error(chk,CMOD_HINT,sect_ph,i,0,
			"Item route with exact 1 point.\n" );
	}
    }

    if ( sect_ph != KMP_CKPH )
    {
	// enemy and item routes
	check_pt(chk,sect_pt,ck_path);
	return;
    }


    //--- only CKPH/CKPT

    ph = (kmp_enph_entry_t*)ph_list->list;
    for ( i = 0; i < n_ph; i++, ph++ )
    {
	uint idx = ph->pt_start;
	uint end = idx + ph->pt_len;
	if ( end > n_pt )
	    end = n_pt;

	u8 done[0x100];
	memset(done,0,sizeof(done));

	uint j;
	const kmp_ckpt_entry_t *pt = ( (kmp_ckpt_entry_t*)pt_list->list ) + idx;
	for ( j = idx; j < end; j++, pt++ )
	{
	    const int good_prev = j == idx ? -1 : j - 1;
	    const int good_next = j == end-1 ? -1 : j + 1;
	    const int have_prev = IS_M1(pt->prev) ? -1 : pt->prev;
	    const int have_next = IS_M1(pt->next) ? -1 : pt->next;

	    if ( good_prev != have_prev )
		print_check_error(chk,CMOD_WARNING,sect_pt,j,0,
			"Invalid 'prev' link [have %d, should be %d]\n",
			have_prev, good_prev );

	    if ( good_next != have_next )
		print_check_error(chk,CMOD_WARNING,sect_pt,j,0,
			"Invalid 'next' link [have %d, should be %d]\n",
			have_next, good_next );

	    typeof(pt->mode) mode = pt->mode;
	    if ( !IS_M1(mode) && mode < sizeof(done)/sizeof(*done) )
		done[mode]++;
	}

	for ( j = 0; j < sizeof(done)/sizeof(*done); j++ )
	    if ( done[j] > 1 )
		print_check_error(chk,CMOD_WARNING,sect_ph,i,0,
		    "Check point mode %u used %u times in single group.\n",
		    j, done[j] );
    }
}

///////////////////////////////////////////////////////////////////////////////

void print_stat
(
    check_kmp_t		* chk,		// valid data structure
    int			sect_ph,	// -1 or PH section
    int			sect_pt,	// (PT) section
    bool		print_null,	// true: print if null pounts
    ccp			name_ph,	// name of ph
    ccp			name_pt		// name of pt
)
{
    DASSERT(chk);
    DASSERT(chk->kmp);
    DASSERT( sect_ph >= -1 && sect_ph < KMP_N_SECT );
    DASSERT( sect_pt >=  0 && sect_pt < KMP_N_SECT );
    DASSERT( sect_ph  <  0 || name_ph );
    DASSERT(name_pt);

    if ( sect_ph >= 0 )
    {
	const uint nh = chk->kmp->dlist[sect_ph].used;
	const uint nt = sect_pt == KMP_POTI
			? chk->kmp->poti_point.used
			: chk->kmp->dlist[sect_pt].used;
	print_check_error(chk,CMOD_INFO,-1,-1,0,
		"%3u %s%s for %u %s%s defined.\n",
		nt, name_pt, nt==1 ? "" : "s",
		nh, name_ph, nh==1 ? "" : "s" );
    }
    else
    {
	const uint np = chk->kmp->dlist[sect_pt].used;
	if ( np || print_null )
	    print_check_error(chk,CMOD_INFO,-1,-1,0,
			"%3u %s%s defined.\n", np, name_pt, np==1 ? "" : "s");
    }
}

//
///////////////////////////////////////////////////////////////////////////////

int CheckKMP
(
    // returns number of errors

    const kmp_t		* kmp,		// pointer to valid KMP
    CheckMode_t		mode		// print mode
)
{
    DASSERT(kmp);
    PRINT("CheckKMP(%d)\n",mode);

    //--- setup

    check_kmp_t chk;
    memset(&chk,0,sizeof(chk));
    SetupColorSet(&chk.col,stdlog);
    chk.kmp	= kmp;
    chk.mode	= mode;
    chk.sect	= -1;
    chk.t_track	= "track";

    if ( mode & CMOD_FORCE_HEADER )
	print_check_error(&chk,0,0,0,0,0);


    //--- var defs

    uint i, n;
    char buf[50];
    u8 done[0x100];

    const uint route_usage_size = 0xff;
    u8 route_usage[route_usage_size];
    memset(route_usage,0,sizeof(route_usage));
    enum { RU_CAME = 1, RU_GOBJ = 2 };

    u16 jgpt_usage[0x100] = {0};
    const int ncame = kmp->dlist[KMP_CAME].used;
    const int npoti = kmp->dlist[KMP_POTI].used;
    const int njgpt = kmp->dlist[KMP_JGPT].used;


    //--- is arena

    chk.is_arena = IsArenaKMP(kmp);
    switch(chk.is_arena)
    {
	case ARENA_NONE:
	    print_check_error(&chk,CMOD_INFO,-99,-1,0,
			"%sRacing track detected.%s\n",
			chk.col.select, chk.col.reset );
	    break;

	case ARENA_MAYBE:
	    print_check_error(&chk,CMOD_HINT,-99,-1,0,
			"%sSome hints for a battle arena found.%s\n",
			chk.col.select, chk.col.reset );
	    break;

	case ARENA_FOUND:
	    chk.t_track = "arena";
	    print_check_error(&chk,CMOD_HINT,-99,-1,0,
			"%sBattle arena detected.%s\n",
			chk.col.select, chk.col.reset );
	    break;

	case ARENA_DISPATCH:
	    chk.t_track = "arena";
	    print_check_error(&chk,CMOD_HINT,-99,-1,0,
			"%sBattle arena with dispatch points detected.%s\n",
			chk.col.select, chk.col.reset );
	    break;

	case ARENA__N:
	    break;
    }

    IsArena_t battle_mode = CheckBattleModeKMP(kmp);
    if ( battle_mode != chk.is_arena )
    {
	chk.is_arena = battle_mode;

	ccp info;
	if ( battle_mode >= ARENA_FOUND )
	{
	    chk.t_track = "arena";
	    info = battle_mode == ARENA_DISPATCH
		 ? "battle arena with dispatch mode"
		 : "battle arena";
	}
	else
	{
	    chk.t_track = "track";
	    info = "racing track";
	}

	print_check_error(&chk,CMOD_HINT,-99,-1,0,
			"%sOption --battle forces mode »%s«!%s\n",
			chk.col.select, info, chk.col.reset );
    }


    //--- statistics

    if ( mode & CMOD_INFO )
    {
	print_stat(&chk, -1,       KMP_AREA, true,  0, "area");
	print_stat(&chk, -1,       KMP_CAME, true,  0, "camera");
	print_stat(&chk, KMP_ENPH, KMP_ENPT, true,  "group", "enemy point");
	print_stat(&chk, KMP_ITPH, KMP_ITPT, true,  "group", "item point");
	print_stat(&chk, KMP_CKPH, KMP_CKPT, true,  "group", "check point");
	print_stat(&chk, KMP_POTI, KMP_POTI, true,  "route", "route point");
	print_stat(&chk, -1,       KMP_GOBJ, true,  0, "global object");
	print_stat(&chk, -1,       KMP_JGPT, true,  0, "respawn position");
	print_stat(&chk, -1,       KMP_KTPT, true,  0, "starting position");
	print_stat(&chk, -1,       KMP_CNPT, false, 0, "cannon target");
	print_stat(&chk, -1,       KMP_MSPT, false, 0, "end battle position");
	print_stat(&chk, -1,       KMP_STGI, true,  0, "stage record");
    }


    //--- check AREA entries

    n = kmp->dlist[KMP_AREA].used;
    const kmp_area_entry_t * area = (kmp_area_entry_t*)kmp->dlist[KMP_AREA].list;
    for ( i = 0; i < n; i++, area++ )
    {
	if ( area->type != 0x00 ) // only CAME checked
	    continue;

	if ( area->dest_id == 0xff )
	    print_check_error(&chk,CMOD_WARNING,KMP_AREA,i,"CAME",
			"Missing camera link.\n");
	else if ( area->dest_id >= ncame )
	    print_check_error(&chk,CMOD_WARNING,KMP_AREA,i,"CAME",
			"Invalid link to not existing camera #%u (max=#%d).\n",
			area->dest_id, ncame-1 );
    }


    //--- check CAME entries

    i = kmp->value[KMP_CAME] >> 8;
    if ( i >= ncame )
	print_check_error(&chk,CMOD_WARNING,KMP_CAME,-2,0,
			"Invalid link to not existing opening camera #%u (max=#%d).\n",
			i, ncame-1 );
    memset(done,0,sizeof(done));
    while ( i < ncame && i < sizeof(done)/sizeof(*done) && !done[i]++ )
    {
	const kmp_came_entry_t * came = (kmp_came_entry_t*)kmp->dlist[KMP_CAME].list + i;
	if ( came->type != 4 && came->type != 5 )
	    print_check_error(&chk,CMOD_WARNING,KMP_CAME,i,"opening camera",
			"Opening camera type should be 4 or 5, but not %u.\n",
			came->type );
	i = came->next;
    }

    i = kmp->value[KMP_CAME] & 0xff;
    if ( i >= ncame )
	print_check_error(&chk,CMOD_WARNING,KMP_CAME,-2,0,
			"Invalid link to not existing selection camera #%u (max=#%d).\n",
			i, ncame-1 );

    const kmp_came_entry_t * came = (kmp_came_entry_t*)kmp->dlist[KMP_CAME].list;
    uint came0count = 0;
    for ( i = 0; i < ncame; i++, came++ )
    {
	snprintf(buf,sizeof(buf),"%u",came->type);

	const typeof(came->route) route = came->route;
	if (!IS_M1(route))
	{
	    if ( route >= npoti )
		print_check_error(&chk,CMOD_WARNING,KMP_CAME,i,buf,
			"Invalid link to not existing route #%u (max=#%d).\n",
			route, npoti-1 );

	    if ( route < route_usage_size )
		route_usage[route] |= RU_CAME;
	}

	switch (came->type)
	{
	    case 0:
		came0count++;
		// fall through
	#ifdef TEST // [[2do]] is this true?
	    case 1:
	    case 3:
	#endif
		if (!IS_M1(route))
		    print_check_error(&chk,CMOD_WARNING,KMP_CAME,i,buf,
			"Link to route #%u not needed.\n",route);
		break;

	#ifdef TEST // [[2do]] is this true?
	    case 2:
	#endif
	    case 5:
	    case 6:
		if (IS_M1(route))
		    print_check_error(&chk,CMOD_WARNING,KMP_CAME,i,buf,
			"Missing route link.\n");
		break;

	#ifndef TEST // [[2do]]
	    case 1:
	    case 2:
	    case 3:
	#endif
	    case 4:
		// may have a route
		break;

	    default:
		print_check_error(&chk,CMOD_WARNING,KMP_CAME,i,buf,
			"Unknown camera type %u.\n",came->type);
	}

	if ( came->next != 0xff && came->next >= ncame )
		print_check_error(&chk,CMOD_WARNING,KMP_CAME,i,buf,
			"Invalid next link to not existing index #%u (max=#%d).\n",
			came->next, ncame-1 );
    }

    if (!came0count)
	print_check_error(&chk,CMOD_HINT,KMP_CAME,-1,"Camera type 0",
		"Missing a camera with type 0.\n" );
    else if ( came0count > 1 )
	print_check_error(&chk,CMOD_WARNING,KMP_CAME,-1,"Camera type 0",
		"Only one camera with type 0 expected, but %u exist.\n",
			came0count );

    //--- check KTPT entries

    n = kmp->dlist[KMP_KTPT].used;
    if (!n)
    {
	print_check_error(&chk,CMOD_WARNING,KMP_KTPT,-1,0,
		"No start point defined => %s will freeze.\n",
		chk.t_track );
    }
    else if ( chk.is_arena >= ARENA_FOUND )
    {
	u8 ktpt_usage[MKW_MAX_PLAYER];
	memset(ktpt_usage,0,sizeof(ktpt_usage));

	const kmp_ktpt_entry_t *ktpt = (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;
	uint i, fail = 0, m1 = 0;
	for ( i = 0; i < n; i++, ktpt++ )
	{
	    const int idx = ktpt->player_index;
	    if ( idx >= 0 && idx < MKW_MAX_PLAYER )
		ktpt_usage[idx]++;
	    else if ( idx == -1 )
		m1++;
	    else
		fail++;
	}

	if (m1)
	    print_check_error(&chk,CMOD_HINT,KMP_KTPT,-1,0,
		"Start point with player index -1 not used by battle arenas.\n");
	if (fail)
	    print_check_error(&chk,CMOD_WARNING,KMP_KTPT,-1,0,
		"%u start position%s with wrong player index found.\n",
		fail, fail==1 ? "" : "s" );

	int n_missing = 0, n_multi = 0;
	for ( i = 0; i < MKW_MAX_PLAYER; i++ )
	    if (!ktpt_usage[i])
		n_missing++;
	    else if ( ktpt_usage[i] > 1 )
		n_multi++;

	DASSERT( sizeof(buf) > 3*MKW_MAX_PLAYER );
	if ( n_missing )
	{
	    char *dest = buf, sep = ' ';
	    for ( i = 0; i < MKW_MAX_PLAYER; i++ )
		if (!ktpt_usage[i])
		{
		    dest = snprintfE(dest,buf+sizeof(buf),"%c%u",sep,i);
		    sep = ',';
		}
	    print_check_error(&chk,CMOD_WARNING,KMP_KTPT,-1,0,
		"%u start position%s missed: player %s%s.\n",
		n_missing, n_missing==1 ? "" : "s",
		n_missing==1 ? "index" : "indices", buf );
	}

	if ( n_multi )
	{
	    char *dest = buf, sep = ' ';
	    for ( i = 0; i < MKW_MAX_PLAYER; i++ )
		if (ktpt_usage[i]>1)
		{
		    dest = snprintfE(dest,buf+sizeof(buf),"%c%u",sep,i);
		    sep = ',';
		}
	    print_check_error(&chk,CMOD_HINT,KMP_KTPT,-1,0,
		"%u start position%s defined twice or more: player %s%s.\n",
		n_multi, n_multi==1 ? "" : "s",
		n_multi==1 ? "index" : "indices", buf );
	}
    }
    else if ( chk.is_arena == ARENA_NONE )
    {
	kmp_finish_t kf;
	CheckFinishLine(kmp,&kf);

	if ( kf.n_ktpt_m > 2 )
	    print_check_error(&chk,CMOD_HINT,KMP_KTPT,-1,0,
		"%u racing start points defined,"
		" but only the first 2 are used by racing tracks.\n",
		kf.n_ktpt_m );

	if ( kf.hint & 1 )
	    print_check_error(&chk,CMOD_HINT|CMOD_LECODE,KMP_KTPT,-1,0,
		"Direction of KTPT #%u and CKPT #%u differ by %3.1f°:"
		" KTPT=%3.1f, CKPT=%3.1f°\n",
		kf.ktpt, kf.ckpt, kf.dir_delta, kf.dir_ktpt, kf.dir_ckpt );
	else if ( kf.n_ktpt_m > 1 )
	    print_check_error(&chk,CMOD_HINT,KMP_KTPT,-1,0,
		"Second racing start point found: Distance to finish line:"
		" %1.0f (%4.2f°)\n",
		kf.distance, kf.dir_delta );

	if ( kf.warn & 2 )
	    print_check_error(&chk,CMOD_WARNING|CMOD_LECODE,KMP_KTPT,-1,0,
		"Distance KTPT #%u and CKPT #%u too large: %3.1f\n",
		kf.ktpt, kf.ckpt, kf.distance );
	else if ( kf.hint & 2 )
	    print_check_error(&chk,CMOD_HINT|CMOD_LECODE,KMP_KTPT,-1,0,
		"Distance KTPT #%u and CKPT #%u larger than usual: %3.1f\n",
		kf.ktpt, kf.ckpt, kf.distance );

	const uint max = kf.n_ktpt_m > 1 ? 2 : 1;
	if ( n > max )
	    print_check_error(&chk,CMOD_HINT,KMP_KTPT,-1,0,
		"%u start points defined, but only %s used by racing tracks.\n",
		n, max > 1 ? "first and second are" : "the first is" );
    }


    //--- check *PH entries

    kmp_path_t ck_path;
    InitializeKmpPath(&ck_path);

    check_ph(&chk,KMP_ENPH,KMP_ENPT,&ck_path);
    check_ph(&chk,KMP_ITPH,KMP_ITPT,&ck_path);
    check_ph(&chk,KMP_CKPH,KMP_CKPT,&ck_path);

    ResetKmpPath(&ck_path);


    //--- check CKPT entries

    uint mode0count = 0, max_mode = 0;
    memset(done,0,sizeof(done));

    n = kmp->dlist[KMP_CKPT].used;
    const kmp_ckpt_entry_t * ckpt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;

    if ( !n && chk.is_arena == ARENA_NONE )
	print_check_error(&chk,CMOD_WARNING,KMP_CKPT,-1,0,
		"No elements found, but needed for racing tracks.\n" );

    for ( i = 0; i < n; i++, ckpt++ )
    {
	if ( ckpt->respawn < sizeof(jgpt_usage)/sizeof(*jgpt_usage) )
	    jgpt_usage[ckpt->respawn]++;

	typeof(ckpt->mode) mode = ckpt->mode;
	if ( !IS_M1(mode) && mode < sizeof(done)/sizeof(*done) )
	{
	    done[mode] = 1;
	    if ( max_mode < mode )
		 max_mode = mode;
	    if (!mode)
		mode0count++;
	}

	uint link;
	for ( link = 0; link < KMP_MAX_PH_LINK; link++ )
	{
	    int next = GetNextPointKMP(kmp,KMP_CKPT,i,link,true,-1,0,0,0,0);
	    if ( next >= 0 )
	    {
		noPRINT("CKPT: %3d.%d -> %3d\n",i,link,next);
		double min_dist;
		int stat = CheckConvexCKPT(kmp,i,next,&min_dist);

		if ( min_dist < KMP_DIST_WARN )
		{
		    print_check_error(&chk,CMOD_WARNING,KMP_CKPT,-1,0,
			    "Check points #%u and #%u are to near together (%5.3f)"
			    " (pos bug?).\n",
			    i, next, min_dist );
		    continue;
		}

		if ( min_dist < KMP_DIST_HINT )
		{
		    print_check_error(&chk,CMOD_HINT,KMP_CKPT,-1,0,
			    "Check points #%u and #%u are near together (%3.1f).\n",
			    i, next, min_dist );
		}

		if ( stat > 0 )
		{
		    print_check_error(&chk,CMOD_WARNING,KMP_CKPT,-1,0,
			    "Check points #%u and #%u do not form a convex quadrilateral"
			    " (pos bug?).\n",
			    i, next );
		}
	    }
	}
    }

    if ( !mode0count && chk.is_arena < ARENA_FOUND )
	print_check_error(&chk,CMOD_WARNING,KMP_CKPT,-1,0,
		"No check point with mode 0 (lap counter).\n");
    else if ( mode0count > 1 && !kmp->multi_ckpt_mode_0_ok )
	print_check_error(&chk,CMOD_HINT,KMP_CKPT,-1,0,
		"%u check points with mode 0 (lap counter).\n",
		mode0count );

    for ( i = 1; i <= max_mode; i++ )
	if (!done[i])
	{
	    const int start = i;
	    while ( ++i <= max_mode && !done[i] )
		;
	    if ( --i == start )
		print_check_error(&chk,CMOD_HINT,KMP_CKPT,-1,0,
			"Missing mandatory check point with mode %u.\n",
		    i );
	    else
		print_check_error(&chk,CMOD_HINT,KMP_CKPT,-1,0,
			"Missing mandatory check points with modes %u..%u.\n",
		    start, i );
	}

    for ( i = njgpt; i < sizeof(jgpt_usage)/sizeof(*jgpt_usage); i++ )
	if (jgpt_usage[i])
	    print_check_error(&chk,CMOD_WARNING,KMP_CKPT,-1,0,
		"%u invalid link%s to not existing respawn point #%u (max=#%d).\n",
		jgpt_usage[i], jgpt_usage[i]>1 ? "s" : "", i, njgpt-1 );


    //--- check JGPT entries

    if ( chk.is_arena < ARENA_FOUND )
    {
	n = njgpt;
	if ( n > sizeof(jgpt_usage)/sizeof(*jgpt_usage) )
	     n = sizeof(jgpt_usage)/sizeof(*jgpt_usage);
	const kmp_jgpt_entry_t * jgpt = (kmp_jgpt_entry_t*)kmp->dlist[KMP_CKPT].list;
	for ( i = 0; i < n; i++, jgpt++ )
	    if (!jgpt_usage[i])
	    {
		const int start = i;
		while ( ++i < n && !jgpt_usage[i] )
		    ;
		if ( --i == start )
		    print_check_error(&chk,CMOD_HINT,KMP_JGPT,-1,0,
			    "Respawn point #%u of %u%s not used/linked.\n",
			    i, n, print_name(kmp,KMP_JGPT,i) );
		else
		{
		    ccp t1 = print_name(kmp,KMP_JGPT,start);
		    ccp t2 = print_name(kmp,KMP_JGPT,i);
		    if ( *t1 && *t2 )
		    {
			t1++;
			t2++;
			print_check_error(&chk,CMOD_HINT,KMP_JGPT,-1,0,
			    "Respawn points #%u..#%u of %u (%s..%s) not used/linked.\n",
			    start, i, n, t1, t2 );
		    }
		    else
			print_check_error(&chk,CMOD_HINT,KMP_JGPT,-1,0,
			    "Respawn points #%u..#%u of %u not used/linked.\n",
			    start, i, n );
		}
	    }
    }

    if (kmp->jgpt_flag.import)
	print_check_error(&chk,CMOD_HINT,KMP_JGPT,-1,0,
				"Exported flags detected (extension of tool wkmpt).\n");


    //--- check GOBJ entries & count COIN usage

    uint n_obj_0 = 0;
    uint n_pitem = 0, n_pitem_unsure = 0;
    uint n_eitem = 0, n_eitem_unsure = 0;
    uint n_coin_start = 0, n_coin_respawn = 0, n_coin_invalid = 0;
    uint n_coin_type[4] = {0};

    n = kmp->dlist[KMP_GOBJ].used;
    if ( n >= 200 )
	print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u global object%s defined.\n",
		n, n==1 ? "" : "s" );

    const kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
    for ( i = 0; i < n; i++, gobj++ )
    {
	uint id = gobj->obj_id;
	if (!IsDefinitionObjectId(id))
	{
	    id &= GOBJ_M_OBJECT;
	    if ( id >= N_KMP_GOBJ || !ObjectInfo[id].name )
	    {
		if (!id)
		    n_obj_0++;
		else
		    print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,i,0,
			    "Unknown object with ID 0x%x.\n",id);
		continue;
	    }

	    if ( id == GOBJ_COIN )
	    {
		const u16 set2 = gobj->setting[1];
		if ( set2 == M1(set2) )
		    n_coin_respawn++;
		else if ( set2 < KMP_COIN_IS_INVALID )
		    n_coin_start++;
		else
		    n_coin_invalid++;

		const u16 set3 = gobj->setting[2];
		n_coin_type[ set3 < 3 ? set3 : 3 ]++;

	    }

	    const ObjectInfo_t *oi = ObjectInfo + id;
	    const typeof(gobj->route_id) route = gobj->route_id;
	    if (IS_M1(route))
	    {
		if ( oi->flags & OBF_ALWAYS_ROUTE )
		    print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,i,oi->name,
			    "Missing route link.\n");
	    }
	    else
	    {
		if ( route >= npoti )
		    print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,i,oi->name,
			    "Invalid link to not existing route #%u (max=#%d).\n",
			    route, npoti-1 );

		if ( oi->flags & OBF_NO_ROUTE )
		    print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,i,oi->name,
			    "Link to route #%u not needed.\n",route);

		if ( route < route_usage_size )
		    route_usage[route] |= RU_GOBJ;
	    }

	    Itembox_t ibox;
	    if (AnalyseItembox(&ibox,gobj))
	    {
		if ( ibox.item_player )
		{
		    n_pitem++;
		    if ( ibox.unsure_player )
			n_pitem_unsure++;
		}

		if ( ibox.item_enemy )
		{
		    n_eitem++;
		    if ( ibox.unsure_enemy )
			n_eitem_unsure++;
		}
	    }
	} // endif !IsDefinitionObjectId()
    }

    if ( n_obj_0 > 0 )
	print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u object%s with ID 0 found.\n",
		n_obj_0, n_obj_0 > 1 ? "s" : "" );

    print_item_hint(&chk,n_pitem,n_pitem_unsure,"players");
    print_item_hint(&chk,n_eitem,n_eitem_unsure,"enemies");


    kmp_special_t special = {0};
    DetectSpecialKMP(kmp,special);
    for ( i = 0; i < HAVEKMP__N; i++ )
	if (special[i])
	{
	    print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u special object%s: %s.\n",
		special[i], special[i] == 1 ? "" : "s", have_kmp_info[i].info );
//	    i += have_kmp_info[i].skip;
	}

    //--- COIN warnings

    const uint n_coin = n_coin_start + n_coin_respawn + n_coin_invalid;
    if ( chk.is_arena == ARENA_NONE && n_coin )
	print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u coin%s found, but coins ignored by racing tracks.\n",
		n_coin, n_coin==1 ? "" : "s" );

    if ( chk.is_arena >= ARENA_FOUND )
    {
	if (!n_coin)
	    print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,-1,0,
		"No coins found, but needed for battle arenas.\n" );
	else
	{
	    if (!n_coin_start)
		print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,-1,0,
			"No start places for coins found, but needed for battle arenas.\n" );
	    else if ( n_coin_start < 20 )
		print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
			"Only %u starting place%s for coins (<20).\n",
			n_coin_start, n_coin_start==1 ? "" : "s" );

	    if (!n_coin_respawn)
		print_check_error(&chk,CMOD_WARNING,KMP_GOBJ,-1,0,
			"No respawn places for coins found, but needed for battle arenas.\n" );
	    else if ( n_coin_respawn < 20 )
		print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
			"Only %u respawn place%s for coins (<20).\n",
			n_coin_respawn, n_coin_respawn==1 ? "" : "s" );
	}
    }

    if (n_coin_invalid)
	print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u coin%s with invalid index found.\n",
		n_coin_invalid, n_coin_invalid==1 ? "" : "s" );

    if ( n_coin_start || n_coin_respawn )
	print_check_error(&chk,CMOD_INFO,KMP_GOBJ,-1,0,
			"Statistics about valid coin places:"
			" %u start place%s + %u respawn place%s.\n",
			n_coin_start, n_coin_start==1 ? "" : "s",
			n_coin_respawn, n_coin_respawn==1 ? "" : "s" );

    if (n_coin_type[3])
	print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
		"%u coin%s with invalid respawn types invalid found.\n",
		n_coin_type[3], n_coin_type[3]==1 ? "" : "s" );

    if ( n_coin_type[0] || n_coin_type[1] || n_coin_type[2] )
	print_check_error(&chk,CMOD_INFO,KMP_GOBJ,-1,0,
			"Statistics about valid coin respawn types (0,1,2): %d + %d + %d.\n",
			n_coin_type[0], n_coin_type[1], n_coin_type[2] );


    //--- check presence flags

    kmp_pflags_t pf;
    CountPresenceFlags(kmp,&pf);
    for ( i = 1; i < sizeof(pf.n_mode)/sizeof(*pf.n_mode); i++ )
	if (pf.n_mode[i])
	    print_check_error(&chk,CMOD_HINT,KMP_GOBJ,-1,0,
			"Extended presence flags: %u object%s with mode %u.\n",
			pf.n_mode[i], pf.n_mode[i] == 1 ? "" : "s", i );


    //--- check POTI entries

    uint ng = kmp->dlist[KMP_POTI].used;
    const kmp_poti_group_t * pg = (kmp_poti_group_t*)kmp->dlist[KMP_POTI].list;
    uint np = kmp->poti_point.used;
    const kmp_poti_point_t * pp = (kmp_poti_point_t*)kmp->poti_point.list;

    n = ng < route_usage_size ? ng : route_usage_size;
    uint gi;
    for ( gi = 0; gi < n; gi++ )
    {
	if (!route_usage[gi])
	{
	    const int start = gi;
	    while ( ++gi < n && !route_usage[gi] )
		;
	    if ( --gi == start )
		print_check_error(&chk,CMOD_HINT,KMP_POTI,-1,0,
		    "Route #%u of %u%s not used/linked.\n",
			gi, ng, print_name(kmp,KMP_POTI,gi) );
	    else
	    {
		ccp t1 = print_name(kmp,KMP_POTI,start);
		ccp t2 = print_name(kmp,KMP_POTI,gi);
		if ( *t1 && *t2 )
		{
		    t1++;
		    t2++;
		    print_check_error(&chk,CMOD_HINT,KMP_POTI,-1,0,
			"Routes #%u..#%u of %u (%s..%s) not used/linked.\n",
			    start, gi, ng, t1, t2 );
		}
		else
		    print_check_error(&chk,CMOD_HINT,KMP_POTI,-1,0,
			"Routes #%u..#%u of %u not used/linked.\n",
			    start, gi, ng );
	    }
	}
	else if ( route_usage[gi] == (RU_CAME|RU_GOBJ) )
	    print_check_error(&chk,CMOD_WARNING,KMP_POTI,-1,0,
		    "Route #%u of %u is linked by CAME and GOBJ (may force a freeze).\n",
		    gi,ng);
    }

    for ( gi = i = 0; gi < ng && i < np; gi++, pg++ )
    {
	uint en = pg->n_point;
	DASSERT( en <= np-i );

	const bool used_by_came = gi < route_usage_size && route_usage[gi] & RU_CAME;
	uint count = 0;
	uint ei;
	for ( ei = 0; ei < en; ei++, i++, pp++ )
	    if ( ei < en-1 && !pp->speed && used_by_came )
		count++;
	if (count)
	    print_check_error(&chk,CMOD_HINT,KMP_POTI,gi,0,
		"CAME route: %u route point%s of %u with zero speed.\n",
			count, count == 1 ? "" : "s", en );
    }


    //--- check MSPT entries

    n = kmp->dlist[KMP_MSPT].used;

    if ( chk.is_arena >= ARENA_FOUND )
    {
	if (!n)
	    print_check_error(&chk,CMOD_WARNING,KMP_MSPT,-1,0,
		"No element defined, but mandatory for battle arenas.\n");
	else if ( n != 4 )
	    print_check_error(&chk,
		n < 4 ? CMOD_WARNING : CMOD_HINT, KMP_MSPT,-1,0,
		"%u element%s defined, but 4 needed for battle arenas.\n",
		n, n==1 ? "" : "s" );
    }
    else if ( chk.is_arena == ARENA_NONE && n )
	print_check_error(&chk,CMOD_HINT,KMP_MSPT,-1,0,
		"%u element%s defined, but not used by racing tracks.\n",
		n, n==1 ? "" : "s" );


    //--- check STGI entries

    n = kmp->dlist[KMP_STGI].used;
    if (!n)
	print_check_error(&chk,CMOD_WARNING,KMP_STGI,-1,0,
		"No element defined.\n");
    else if (n>1)
	print_check_error(&chk,CMOD_WARNING,KMP_STGI,-1,0,
		"%u elements defined.\n", n );
    else
    {
	const kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp->dlist[KMP_STGI].list;

	if ( stgi->lap_count != 3 )
	    print_check_error(&chk,CMOD_HINT,KMP_STGI,0,"STGI",
		"Lap counter changed to %u.\n",stgi->lap_count);
	else if ( mode & CMOD_INFO )
	    print_check_error(&chk,CMOD_INFO,KMP_STGI,0,"STGI",
		"Lap counter is to %u.\n",stgi->lap_count);

	if ( stgi->pole_pos > 1 )
	    print_check_error(&chk,CMOD_WARNING,KMP_STGI,0,"STGI",
		"Undefined value for pole positions: %u\n",stgi->pole_pos);
	else if ( mode & CMOD_INFO )
	    print_check_error(&chk,CMOD_INFO,KMP_STGI,0,"STGI",
		"Pole position is '%s' (%u).\n",
		stgi->pole_pos ? "right" : "left", stgi->pole_pos );

	if ( stgi->narrow_start > 1 )
	    print_check_error(&chk,CMOD_WARNING,KMP_STGI,0,"STGI",
		"Undefined value for narrow mode: %u\n",stgi->narrow_start);
	else if ( mode & CMOD_INFO )
	    print_check_error(&chk,CMOD_INFO,KMP_STGI,0,"STGI",
		"Start mode is '%s' (%u).\n",
		stgi->narrow_start ? "narrow" : "wide", stgi->narrow_start );

	if ( stgi->speed_mod )
	    print_check_error(&chk,CMOD_HINT,KMP_STGI,0,"STGI",
		"Speed modifier is set to %5.3f (%04x/hex).\n",
		SpeedMod2float(stgi->speed_mod), stgi->speed_mod);
	else if ( mode & CMOD_INFO )
	    print_check_error(&chk,CMOD_INFO,KMP_STGI,0,"STGI",
		"No speed modifier set (0000/hex).\n" );
    }


    //--- check used positions

    kmp_usedpos_t up;
    CheckUsedPos(kmp,&up);

    bool lex_hint = false;
    for ( i = 0; i < 3; i++ )
    {
	if ( up.orig.rating[i] < WLEVEL_HINT )
	    continue;

	lex_hint = true;
	ccp range_info = i == 1 ? "1..131071" : "±131071";

	if ( up.orig.rating[i] >= WLEVEL_ERROR )
	    print_check_error(&chk,CMOD_WARNING, KMP_N_SECT,-1,0,
			"At least 1 %c coordinate of ENPT/ITPT"
			" is outside online limit of %s. Used range: %1.0f..%1.0f\n",
			'X'+i, range_info, up.orig.min.v[i], up.orig.max.v[i] );
	else if ( up.orig.rating[i] >= WLEVEL_WARN )
	    print_check_error(&chk,CMOD_WARNING, KMP_N_SECT,-1,0,
			"At least 1 %c coordinate of ENPT/ITPT"
			" is near online limit of %s. Used range: %1.0f..%1.0f\n",
			'X'+i, range_info, up.orig.min.v[i], up.orig.max.v[i] );
	else if ( up.orig.rating[i] >= WLEVEL_HINT )
	    print_check_error(&chk,CMOD_HINT, KMP_N_SECT,-1,0,
			"At least 1 %c coordinate of ENPT/ITPT"
			" is near online limit of %s. Used range: %1.0f..%1.0f\n",
			'X'+i, range_info, up.orig.min.v[i], up.orig.max.v[i] );
    }

    if (lex_hint)
    {
	const kmp_usedpos_obj_t *obj = up.suggest;
	if ( obj->shift_active && obj->pos_factor_valid )
	{
	    fprintf(stdlog,"%s%s%s"
		"Shift the whole track by --shift %1.0f,%1.0f,%1.0f"
		" and setup LEX/SET1 with @ITEM-POS-FACTOR=v(%4.2f,%4.2f,%4.2f)\n",
		chk.col.info, check_bosugl, chk.col.reset,
		obj->shift.x, obj->shift.y, obj->shift.z,
		obj->pos_factor.x, obj->pos_factor.y, obj->pos_factor.z );
	    if ( obj != &up.orig && up.orig.pos_factor_valid )
		fprintf(stdlog,"%s%s%s"
			"... or setup LEX/SET1 only with"
			" @ITEM-POS-FACTOR=v(%4.2f,%4.2f,%4.2f)\n",
			chk.col.info, check_bosugl, chk.col.reset,
			up.orig.pos_factor.x, up.orig.pos_factor.y, up.orig.pos_factor.z );
	}
	else if (obj->shift_active)
	    fprintf(stdlog,"%s%s%s"
		"Shift the whole track by --shift %1.0f,%1.0f,%1.0f\n",
		chk.col.info, check_bosugl, chk.col.reset,
		obj->shift.x, obj->shift.y, obj->shift.z );
	else if (obj->pos_factor_valid)
	    fprintf(stdlog,"%s%s%s"
		"Setup LEX/SET1 with @ITEM-POS-FACTOR=v(%4.2f,%4.2f,%4.2f)\n",
		chk.col.info, check_bosugl, chk.col.reset,
		obj->pos_factor.x, obj->pos_factor.y, obj->pos_factor.z );
    }


    //--- check existence of WIM0

    if (kmp->wim0_import_size)
    {
	char *dest = buf, *end = buf+sizeof(buf) - 2, *sep = " (";
	const kmp_wim0_sect_t *sect = FindSectionWim0(0,kmp,W0ID_VERSION);
	if (sect)
	{
	    dest = snprintfE(dest,end,"%sv%s",sep,(ccp)sect->data);
	    sep = ",";
	}
	if (kmp->wim0_import_bz2)
	    dest = snprintfE(dest,end,"%sbzip2",sep);

	if ( dest > buf )
	{
	    *dest++ = ')';
	    *dest = 0;
	}
	else
	    *buf = 0;

	print_check_error(&chk,CMOD_HINT,KMP_WIM0,-1,0,
			"KMP section WIM0 with %u bytes total%s detected.\n",
			kmp->wim0_import_size, buf );
    }


    //--- terminate

    if ( mode & CMOD_GLOBAL_STAT )
    {
	global_warn_count += chk.warn_count;
	global_hint_count += chk.hint_count;
	global_info_count += chk.info_count;
    }

    if ( mode & (CMOD_FOOTER|CMOD_FORCE_FOOTER) )
    {
	static ccp hint
	    = " => see https://szs.wiimm.de/cmd/wkmpt/check#desc for more info.";

	ccp bind;
	char hbuf[60], ibuf[60];
	if (chk.info_count)
	{
	    snprintf(ibuf,sizeof(ibuf)," and %s%u info%s%s",
		chk.col.info,
		chk.info_count, chk.info_count>1 ? "s" : "",
		chk.col.heading );
	    bind = ",";
	}
	else
	{
	    *ibuf = 0;
	    bind = " and";
	}

	if (chk.hint_count)
	    snprintf(hbuf,sizeof(hbuf),"%s %s%u hint%s%s",
		bind, chk.col.hint,
		chk.hint_count, chk.hint_count>1 ? "s" : "",
		chk.col.heading );
	else
	    *hbuf = 0;

	if (chk.warn_count)
	    fprintf(stdlog,
			" %s=> %s%u warning%s%s%s%s for %s:%s\n%s%s%s\n\n",
			chk.col.heading, chk.col.warn,
			chk.warn_count, chk.warn_count == 1 ? "" : "s",
			chk.col.heading,
			hbuf, ibuf, GetNameFF(kmp->fform,0), kmp->fname,
			chk.col.heading, hint, chk.col.reset );
	else if ( mode & CMOD_FORCE_FOOTER || chk.hint_count )
	    fprintf(stdlog," %s=> No warnings%s%s for %s:%s\n%s%s%s\n\n",
			chk.col.heading, hbuf, ibuf, GetNameFF(kmp->fform,0), kmp->fname,
			chk.col.heading, hint, chk.col.reset );
    }

    return chk.warn_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command stgi			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_stgi()
{
    stdlog = stderr;
    raw_data_t raw;
    InitializeRawData(&raw);

    if (print_header)
	printf("\n  start pos  speed laps LC : file\n%.*s\n",
		79, Minus300 );

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,"/course.kmp",opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	{
	    if ( max_err < err )
		max_err = err;
	    continue;
	}

	if ( opt_ignore && raw.fform != FF_KMP && raw.fform != FF_KMP_TXT )
	    continue;

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,CMOD_SILENT);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if ( max_err < err )
		max_err = err;
	    continue;
	}

	uint lap_count = 0;
	const uint n_ckpt =  kmp.dlist[KMP_CKPT].used;
	if (n_ckpt)
	{
	    const kmp_ckpt_entry_t *ckpt = (kmp_ckpt_entry_t*)kmp.dlist[KMP_CKPT].list;
	    uint i;
	    for ( i = 0; i < n_ckpt; i++, ckpt++ )
		if (!ckpt->mode)
		    lap_count++;
	}

	const kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp.dlist[KMP_STGI].list;
	const uint n_stgi = kmp.dlist[KMP_STGI].used;
	if (!n_stgi)
	    printf("%s   no STGI found     -%s", colout->warn, colout->reset );
	else if ( opt_ignore > 1
		&& lap_count == 1
		&& stgi->pole_pos < 2
		&& stgi->narrow_start < 2
		&& !stgi->speed_mod
		&& stgi->lap_count == 3
		)
	{
	    ResetKMP(&kmp);
	    continue;
	}
	else
	{
	    if ( stgi->pole_pos == 0 )
		fputs(" left ",stdout);
	    else if ( stgi->pole_pos == 1 )
		fputs(" right",stdout);
	    else
		printf("%s%5u%s ", colout->warn, stgi->pole_pos, colout->reset);

	    if ( stgi->narrow_start == 0 )
		fputs("  wide ",stdout);
	    else if ( stgi->narrow_start == 1 )
		fputs(" narrow",stdout);
	    else
		printf("%s%5u%s  ", colout->warn, stgi->narrow_start, colout->reset );

	    if (stgi->speed_mod)
	    {
		float speed = SpeedMod2float(stgi->speed_mod);
		printf("%s %4.2f%s",
		    speed == 1.0 ? "" : speed <= 1.0 ? colout->info : colout->hint,
		    speed, colout->reset );
	    }
	    else
		fputs("  -- ",stdout);

	    printf(" %s%3u%s",
		    stgi->lap_count == 3 ? ""
			    : stgi->lap_count < 3 ? colout->info : colout->hint,
		    stgi->lap_count, colout->reset );
	}
	printf(" %s%3u%s : %s\n",
		    lap_count == 1 ? "" : colout->hint,
		    lap_count, colout->reset,
		    param->arg );
	ResetKMP(&kmp);
    }
    if (print_header)
	printf( "%.*s\n\n", 79, Minus300 );

    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command ktpt			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_ktpt()
{
    stdlog = stderr;
    putchar('\n');

    char sep[30];
    StringCat3S(sep,sizeof(sep),colout->heading,"│",colout->reset);

    raw_data_t raw;
    InitializeRawData(&raw);

    kmp_t kmp;
    InitializeKMP(&kmp);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,"/course.kmp",opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	{
	    if ( max_err < err )
		max_err = err;
	    continue;
	}

	if ( opt_ignore && raw.fform != FF_KMP && raw.fform != FF_KMP_TXT )
	    continue;

	ResetKMP(&kmp);
	err = ScanRawDataKMP(&kmp,false,&raw,CMOD_SILENT);
	if ( err > ERR_WARNING )
	{
	    if ( max_err < err )
		max_err = err;
	    continue;
	}

	kmp_finish_t kf;
	CheckFinishLine(&kmp,&kf);
	if (!kf.valid)
	{
	    if (opt_ignore<1)
	    {
		if (kf.ckpt<0)
		    ERROR0(ERR_WARNING,
			"No relevant lap counter (CKPT) found: %s\n",param->arg);
		else if (kf.ktpt<0)
		    ERROR0(ERR_WARNING,
			"No relevant start points (KTPT) found: %s\n",param->arg);
	    }
	    continue;
	}

	printf(	"%s> Path and file name:             %s%s\n"
		"Number of start points (KTPT):  %3u, relevant #%u\n"
		"Number of lap counters (CKPT):  %3u, relevant #%u\n"
		"Relevant start point (pos/dir):%13.3f %11.3f %11.3f /%8.2f°\n"
		,colout->caption
		,param->arg
		,colout->reset
		,kf.n_ktpt_m, kf.ktpt
		,kf.n_ckpt_0, kf.ckpt
		,kf.pos_ktpt.x, kf.pos_ktpt.z, kf.pos_ktpt.z, kf.dir_ktpt
		);

	if ( kf.ckpt_opt >= 0 && ( kf.ckpt_opt != kf.ckpt || kf.hint ) )
	    printf(
		"%sOptimized start point (pos/dir):%12.3f %11.3f %11.3f /%8.2f°%s\n"
		,colout->hint
		,kf.pos_ktpt_opt.x, kf.pos_ktpt_opt.z, kf.pos_ktpt_opt.z
		,kf.dir_ktpt_opt
		,colout->reset
		);

	printf(	"Relevant check point (pos/dir): %12.3f %23.3f /%8.2f°\n"
		"Distance between both points: %s%14.3f%s%25s/%s%8.2f°%s\n"
		,kf.pos_ckpt.x, kf.pos_ckpt.z, kf.dir_ckpt
		,kf.hint & 2 ? colout->warn : "", kf.distance  ,colout->reset ,""
		,kf.hint & 1 ? colout->warn : "", kf.dir_delta ,colout->reset
		);

	if ( kf.enpt_rec >= 0 )
	    printf("\n"
		"%sRecommendation based on nearest ENPT #%d:%s\n"
		"  Recommended KTPT record:     "
		"1 %11.3f %11.3f %11.3f  0 %6.2f 0  -1 0\n"
		"  Recommended patch option:       --ktpt2=%1.0f,%1.0f\n"
		,colout->hint,kf.enpt_rec,colout->reset
		,kf.pos_ktpt_rec.x, kf.pos_ktpt_rec.y, kf.pos_ktpt_rec.z
		,kf.dir_ktpt_rec
		,kf.pos_ktpt_rec.x, kf.pos_ktpt_rec.z
		);

	if ( long_count > 0 )
	{
	    const int fw1 = 3 * 31;
	    const int fw2 = 3 * 24;
	    const int fw3 = 3 * 16;
	    const int fw4 = 3 * 16;

	    printf(
		"\n"
		"%sCompare all relevant KTPT with all lap counters:%s\n"
		"%s┌%.*s┬%.*s┬%.*s┬%.*s┐\n"
		"%s│     Start Position (KTPT)     │   Lap Counter (CKPT)   "
			"│    KTPT−CKPT   │ Parameter for  │\n"
		"%s│ idx  x-pos  y-pos   z-pos   ° │ idx  x-pos   z-pos   ° "
			"│ distance   °   │ option --ktpt2 │%s\n",
		colout->heading, colout->reset,
		colout->heading,
		fw1, ThinLine300_3, fw2, ThinLine300_3,
		fw3, ThinLine300_3, fw4, ThinLine300_3,
		colout->heading, colout->heading, colout->reset );

	    const uint nc = kmp.dlist[KMP_CKPT].used;
	    const uint nk = kmp.dlist[KMP_KTPT].used;
	    const kmp_ktpt_entry_t *ktpt
			= (kmp_ktpt_entry_t*)kmp.dlist[KMP_KTPT].list;

	    uint ik;
	    for ( ik = 0; ik < nk; ik++, ktpt++ )
	    {
		if ( ktpt->player_index >= 0 )
		    continue;

		const kmp_ckpt_entry_t *ckpt
			    = (kmp_ckpt_entry_t*)kmp.dlist[KMP_CKPT].list;

		if ( !ik || kf.n_ckpt_0 > 1 )
		    printf("%s├%.*s┼%.*s┼%.*s┼%.*s┤%s\n",
			colout->heading,
			fw1, ThinLine300_3, fw2, ThinLine300_3,
			fw3, ThinLine300_3, fw4, ThinLine300_3,
			colout->reset );

		uint ic;
		for ( ic = 0; ic < nc; ic++, ckpt++ )
		{
		    if (ckpt->mode)
			continue;

		    CalcFinishLine(&kmp,&kf,ckpt,ktpt);

		    char opt_buf[20], *opt = "-";
		    if ( kf.enpt_rec >= 0 )
		    {
			snprintf(opt_buf,sizeof(opt_buf),"%1.0f,%1.0f",
					kf.pos_ktpt_rec.x, kf.pos_ktpt_rec.z );
			opt = opt_buf;
		    }

		    printf(
			"%s%3u %7.0f %6.0f %7.0f %3.0f "
			"%s%3u %7.0f %7.0f %3.0f %s %s%6.0f %s%7.2f %s%s %-15s%s\n",
			sep, ik,
			kf.pos_ktpt.x, kf.pos_ktpt.y, kf.pos_ktpt.z, kf.dir_ktpt,
			sep, ic,
			kf.pos_ckpt.x, kf.pos_ckpt.z, kf.dir_ckpt,
			sep,
			kf.warn & 2 ? colout->warn
				: kf.hint & 2 ? colout->hint : "",
				kf.distance,
			kf.hint & 1 ? colout->warn
				: kf.hint & 1 ? colout->hint : colout->reset,
				kf.dir_delta,
			sep,
			ik == kf.ktpt && ic == kf.ckpt ? colout->hint : "",
			opt, sep );
		}
	    }

	    printf("%s└%.*s┴%.*s┴%.*s┴%.*s┘%s\n",
		colout->heading,
		fw1, ThinLine300_3, fw2, ThinLine300_3,
		fw3, ThinLine300_3, fw4, ThinLine300_3,
		colout->reset );
	}
	putchar('\n');
    }

    ResetKMP(&kmp);
    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


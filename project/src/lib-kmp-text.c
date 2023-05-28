
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
 *   Copyright (c) 2011-2023 by Dirk Clemens <wiimm@wiimm.de>              *
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
#include "kmp.inc"

#include <math.h>
#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cond_ref table			///////////////
///////////////////////////////////////////////////////////////////////////////

const gobj_cond_ref_t cond_ref_tab[COR__N+1] =
{
    { COR_NEVER,	"NEVER",	"",	0,"never",		0x0000,0x00,0 },
    { COR_LECODE,	"LECODE",	"P1234",2,"lecode",		0xffff,0xff,0 },
    { COR_SINGLE,	"SINGLE",	"P1",	2,"1 player @Wii",	0x5551,0x00,0 },
    { COR_MULTI,	"MULTI",	"P234",	2,"2-4 players @Wii",	0xaaae,0x00,0 },
    { COR_OFFLINE,	"OFFLINE",	"OFF",	2,"offline",		0x400f,0x00,0 },
    { COR_ONLINE,	"ONLINE",	"ON",	2,"online",		0xbff0,0x00,0 },

    { COR_LE6,		"LE6",		"1_6",	3,"≤6 players",		0xc03f,0x00,0 },
    { COR_GE7,		"GE7",		"7_99",	2,"≥7 players",		0x3fc0,0x00,0 },
    { COR_LE9,		"LE9",		"1_9",	2,"≤9 players",		0xc0ff,0x00,0 },
    { COR_GE10,		"GE10",		"10_99",2,"≥10 players",	0x3f00,0x00,0 },
    { COR_LE12,		"LE12",		"1_12",	2,"≤12 players",	0xc3ff,0x00,0 },
    { COR_GE13,		"GE13",		"13_99",2,"≥13 players",	0x3c00,0x00,0 },
    { COR_LE18,		"LE18",		"1_18",	2,"≤18 players",	0xcfff,0x00,0 },
    { COR_GE19,		"GE19",		"19_99",2,"≥19 players",	0x3000,0x00,0 },

    { COR_7_9,		"7_9",		"",	3,"7-9 players",	0x00c0,0x00,0 },
    { COR_NOT_7_9,	"NOT_7_9",	"",	2,"<7 or >9 players",	0xff3f,0x00,0 },
    { COR_7_12,		"7_12",		"",	2,"7-12 players",	0x03c0,0x00,0 },
    { COR_NOT_7_12,	"NOT_7_12",	"",	2,"<7 or >12 players",	0xfc3f,0x00,0 },
    { COR_7_18,		"7_18",		"",	2,"7-18 players",	0x0fc0,0x00,0 },
    { COR_NOT_7_18,	"NOT_7_18",	"",	2,"<7 or >18 players",	0xf03f,0x00,0 },
    { COR_10_12,	"10_12",	"",	2,"10-12 players",	0x0300,0x00,0 },
    { COR_NOT_10_12,	"NOT_10_12",	"",	2,"<10 or >12 players",	0xfcff,0x00,0 },
    { COR_10_18,	"10_18",	"",	2,"10-18 players",	0x0f00,0x00,0 },
    { COR_NOT_10_18,	"NOT_10_18",	"",	2,"<10 or >18 players",	0xf0ff,0x00,0 },
    { COR_13_18,	"13_18",	"",	2,"13-18 players",	0x0c00,0x00,0 },
    { COR_NOT_13_18,	"NOT_13_18",	"",	2,"<13 or >18 players",	0xf3ff,0x00,0 },

    { COR_TIMETRIAL,	"TIMETRIAL",	"TT",	3,"time trial",		0x4000,0x04,1 },
    { COR_NO_TIMETRIAL,	"NOT_TIMETRIAL","NOTT",	2,"not time trial",	0xbfff,0xfb,0 },
    { COR_BATTLE,	"BATTLE",	"",	2,"battle",		0x0000,0x03,0 },
    { COR_RACE,		"RACE",		"",	2,"race",		0x0000,0xfc,0 },

    { COR_BALLOON,	"BALLOON",	"",	3,"balloon",		0x0000,0x01,0 },
    { COR_NO_BALLOON,	"NOT_BALLOON",	"",	2,"not balloon",	0x0000,0xfe,0 },
    { COR_COIN,		"COIN",		"",	2,"coin",		0x0000,0x02,0 },
    { COR_NO_COIN,	"NOT_COIN",	"",	2,"not coin",		0x0000,0xfd,0 },
    { COR_VERSUS,	"VERSUS",	"",	2,"versus",		0x0000,0x04,0 },
    { COR_NO_VERSUS,	"NOT_VERSUS",	"",	2,"not versus",		0x0000,0xfb,0 },
    { COR_ITEMRAIN,	"ITEMRAIN",	"",	2,"item rain",		0x0000,0x08,0 },
    { COR_NO_ITEMRAIN,	"NOT_ITEMRAIN",	"",	2,"not item rain",	0x0000,0xf7,0 },
    { COR_SETTING5,	"SETTING5",	"",	2,"settings 5",		0x0000,0x10,0 },
    { COR_NO_SETTING5,	"NOT_SETTING5",	"",	2,"not setting 5",	0x0000,0xef,0 },
    { COR_SETTING6,	"SETTING6",	"",	2,"settings 6",		0x0000,0x20,0 },
    { COR_NO_SETTING6,	"NOT_SETTING6",	"",	2,"not setting 6",	0x0000,0xdf,0 },
    { COR_SETTING7,	"SETTING7",	"",	2,"settings 7",		0x0000,0x40,0 },
    { COR_NO_SETTING7,	"NOT_SETTING7",	"",	2,"not setting 7",	0x0000,0xbf,0 },
    { COR_SETTING8,	"SETTING8",	"",	2,"settings 8",		0x0000,0x80,0 },
    { COR_NO_SETTING8,	"NOT_SETTING8",	"",	2,"not setting 8",	0x0000,0x7f,0 },

    { COR_P2,		"P2",		"",	3,"2 players @Wii",	0xaaa2,0x00,0 },
    { COR_P134,		"P134",		"",	2,"1|3|4 players @Wii",	0x555d,0x00,0 },
    { COR_P3,		"P3",		"",	2,"3 players @Wii",	0x0004,0x00,0 },
    { COR_P124,		"P124",		"",	2,"1|2|4 players @Wii",	0xfffb,0x00,0 },
    { COR_P4,		"P4",		"",	2,"4 players @Wii",	0x0008,0x00,0 },
    { COR_P123,		"P123",		"",	2,"1-3 players @Wii",	0xfff7,0x00,0 },
    { COR_P12,		"P12",		"",	2,"1-2 players @Wii",	0xfff3,0x00,0 },
    { COR_P34,		"P34",		"",	2,"3-4 players @Wii",	0x000c,0x00,0 },
    { COR_P13,		"P13",		"",	2,"1|3 players @Wii",	0x5555,0x00,0 },
    { COR_P24,		"P24",		"",	2,"2|4 players @Wii",	0xaaaa,0x00,0 },
    { COR_P14,		"P14",		"",	2,"1|4 players @Wii",	0x5559,0x00,0 },
    { COR_P23,		"P23",		"",	2,"2-3 players @Wii",	0xaaa6,0x00,0 },

    { CORD_DEBUG,	"DEBUG",	"",	5,"if LE-CODE debug",	0x0001,0x00,2 },
    { CORD_RELEASE,	"RELEASE",	"",	4,"if LE-CODE release",	0x0002,0x00,2 },
    { CORD_TESTVAL,	"TESTVAL",	"",	4,"have test values",	0x0004,0x00,2 },
    { CORD_NO_TESTVAL,	"NOT_TESTVAL",	"",	4,"haven't test values",0x0008,0x00,2 },

    { 0,"","",0,"",0,0,0 },
};

///////////////////////////////////////////////////////////////////////////////

const gobj_cond_ref_t * FindConditionRef ( u16 ref_id )
{
    const gobj_cond_ref_t *rc = cond_ref_tab;
    if ( ref_id < COR__N )
    {
	const gobj_cond_ref_t *rc2 = cond_ref_tab + ref_id - 0x1000;
	if ( rc2->ref_id <= ref_id )
	    rc = rc2;
    }

    for ( ; rc->ref_id; rc++ )
	if ( rc->ref_id == ref_id )
	    return rc;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool IsConditionRefEnabled
	( const gobj_cond_mask_t *cm, u16 ref_id, const gobj_cond_ref_t *cref )
{
    DASSERT(cm);

    //--- check predefinied condition for ENGINE

    if (IsConditionRefEngineId(ref_id))
	return ref_id & cm->engine_mask;


    //--- check predefinied condition for RANDOM

    if (IsConditionRefRandomId(ref_id))
	return ref_id & cm->random_mask;


    //--- check hard coded condition => search list

    if (IsConditionRefId(ref_id))
    {
	if (!cref)
	    cref = FindConditionRef(ref_id);
	if (cref)
	{
	    switch (cref->op)
	    {
	     case 0:
		return cref->cond_mask & cm->bit_mask
		    || cref->set_mask  & cm->game_mask;

	     case 1:
		return cref->cond_mask & cm->bit_mask
		    && cref->set_mask  & cm->game_mask;
	    }
	}
	return false;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetConditionRefInfo
	( cond_ref_info_t *cri, u16 ref_id, u16 obj_id, bool create_cond  )
{
    DASSERT(cri);
    memset(cri,0,sizeof(*cri));

    if ( obj_id >= GOBJ_MIN_DEF || obj_id & GOBJ_M_LECODE )
	cri->cmd_suffix = "ENABLE";
    else
    {
	cri->cmd_suffix = "DISABLE";
	cri->xor	= 1;
    }

    if (IsConditionRefTestId(ref_id))
    {
	cri->is_condref = true;
	if (cri->xor)
	{
	    cri->xor = 1;
	    ref_id  ^= 1;
	}
	if (create_cond)
	{
	    snprintf( cri->cond, sizeof(cri->cond),
			"IF$TEST(%u,%u)", ref_id >> 6 & 15, ref_id >> 1 & 31 );
	    StringCopyS(cri->info,sizeof(cri->info),
			"Test condition for LE-CODE developers" );
	}
    }
    else if (IsConditionRefEngineId(ref_id))
    {
	cri->is_condref = true;
	if (cri->xor)
	{
	    cri->xor = GOBJ_MASK_COND_ENGINE;
	    ref_id  ^= GOBJ_MASK_COND_ENGINE;
	}
	if (create_cond)
	{
	    char *end = cri->cond + sizeof(cri->cond) - 3;
	    char *dest = StringCopyE(cri->cond,end,"if$engine(");
	    static const char name[][7]
		= { "battle", "50", "100", "150", "200", "150m", "200m", "" };

	    uint i, m = ref_id;
	    for ( i = 0; name[i][0]; i++, m >>= 1 )
		if ( m&1 )
		{
		    if (dest[-1] != '(' )
			*dest++ = ',';
		    dest = StringCat2E(dest,end,"en$",name[i]);
		}
	    *dest++ = ')';
	    *dest = 0;

	    StringCopyS(cri->info,sizeof(cri->info),
			"if selected engines are active" );
	}
    }
    else if (IsConditionRefRandomId(ref_id))
    {
	cri->is_condref = true;
	if (cri->xor)
	{
	    cri->xor = GOBJ_MASK_COND_RND;
	    ref_id  ^= GOBJ_MASK_COND_RND;
	}
	if (create_cond)
	{
	    char *end = cri->cond + sizeof(cri->cond) - 4;
	    char *dest = StringCopyE(cri->cond,end,"if$random(");
	    uint i, m;
	    for ( i = m = 1; i <= GOBJ_MAX_RANDOM; i++, m <<= 1 )
		if ( ref_id & m && dest < end )
		{
		    if (dest[-1] != '(' )
			*dest++ = ',';
		    *dest++ = '0'+i;
		}
	    *dest++ = ')';
	    *dest = 0;

	    StringCopyS(cri->info,sizeof(cri->info),
			"if selected random scenario is active" );
	}
    }
    else
    {
	cri->condref = FindConditionRef(ref_id);
	if (cri->condref)
	{
	    cri->is_condref = true;
	    if (create_cond)
	    {
		const gobj_cond_ref_t *iref = cri->xor
			    ? FindConditionRef(ref_id^cri->xor)
			    : cri->condref;
		if (iref)
		{
		    if ( iref->option & 4 )
		    {
			snprintf( cri->cond, sizeof(cri->cond), "IFDEV$%s",iref->name);
			snprintf( cri->info, sizeof(cri->info),
				"DEVELOPERS: %s", iref->info );
		    }
		    else
		    {
			snprintf( cri->cond, sizeof(cri->cond), "IF$%s",iref->name);
			snprintf( cri->info, sizeof(cri->info), "%s%s",
			    iref->option & 1 ? "if " : "",
			    iref->info );
		    }
		}
	    }
	}
	else
	    cri->is_condref = IsConditionRefId(ref_id);

	if ( cri->is_condref && !*cri->info )
	    snprintf( cri->cond, sizeof(cri->cond), "%#x",ref_id^cri->xor );
    }

    return cri->is_condref;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			link info			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetRouteLinksKMP ( const kmp_t *ckmp )
{
    DASSERT(ckmp);
    kmp_t *kmp = (kmp_t*)ckmp;
    if (kmp->linfo)
    {
	memset(kmp->linfo,0,sizeof(*kmp->linfo));
	kmp->linfo_sect = ~0;
    }
}

///////////////////////////////////////////////////////////////////////////////

kmp_linfo_t * SetupRouteLinksKMP ( const kmp_t *ckmp, uint sect, bool force )
{
    DASSERT(ckmp);
    kmp_t *kmp = (kmp_t*)ckmp;
    if (!kmp->linfo)
    {
	kmp->linfo = MALLOC(sizeof(*kmp->linfo));
	kmp->linfo_sect = ~0;
    }

    if ( force || kmp->linfo_sect != sect )
    {
	kmp->linfo_sect = sect;
	AnalyseRouteKMP(kmp->linfo,kmp,sect);
	noPRINT("ROUTE INFO for section #%d [%s] => %u\n",
		sect, kmp_section_name[sect].name1, kmp->linfo->used );
    }
    return (kmp_linfo_t*) kmp->linfo;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint AnalyseRouteKMP ( kmp_linfo_t *li, const kmp_t *kmp, uint sect )
{
    DASSERT(li);
    DASSERT(kmp);

    switch(sect)
    {
	case KMP_CKPH:
	case KMP_ENPH:
	case KMP_ITPH:
	    return AnalyseGroupLinksKMP(li,kmp,sect);

	case KMP_CKPT:
	case KMP_ENPT:
	case KMP_ITPT:
	    return AnalyseRouteLinksKMP(li,kmp,sect);

	default:
	    ASSERT_MSG(0,"AnalyseRouteKMP(): Unsupported KMP section: %u",sect);
	    memset(li,0,sizeof(*li));
	    li->sect_pt = li->sect_ph = KMP_NO_SECT;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint AnalyseGroupLinksKMP ( kmp_linfo_t *li, const kmp_t *kmp, uint sect_ph )
{
    DASSERT(li);
    DASSERT(kmp);
    memset(li,0,sizeof(*li));


    //--- section dependent setup

    switch(sect_ph)
    {
	case KMP_CKPH:
	case KMP_CKPT:
	    li->sect_pt = KMP_CKPT;
	    li->sect_ph = KMP_CKPH;
	    break;

	case KMP_ENPH:
	case KMP_ENPT:
	    li->sect_pt = KMP_ENPT;
	    li->sect_ph = KMP_ENPH;
	    break;

	case KMP_ITPH:
	case KMP_ITPT:
	    li->sect_pt = KMP_ITPT;
	    li->sect_ph = KMP_ITPH;
	    break;

	default:
	    ASSERT_MSG(0,"AnalyseGroupLinksKMP(): Unsupported KMP section: %u",sect_ph);
	    li->sect_pt = li->sect_ph = KMP_NO_SECT;
	    return 0;
    }


    //--- setup

    uint n = kmp->dlist[li->sect_ph].used;
    if ( n > KMP_MAX_LINFO )
	n = KMP_MAX_LINFO;


    //--- iterate groups

    uint i, max = 0;
    const kmp_enph_entry_t * p = (kmp_enph_entry_t*)kmp->dlist[li->sect_ph].list;

    for ( i = 0; i < n; i++, p++ )
    {
	li->n_point[i] = p->pt_len;
	if (IsDispatchPointKMP(p))
	    li->gmode[i] |= LINFOG_DISPATCH;

	uint l;
	for ( l = 0; l < KMP_MAX_PH_LINK; l++ )
	{
	    u8 j = p->prev[l];
	    if ( j != 0xff )
	    {
		li->summary[i] |= LINFO_PREV;
		li->list[i][j] |= LINFO_PREV;
		li->summary[j] |= LINFO_BY_PREV;
		li->list[j][i] |= LINFO_BY_PREV;
		if ( max <= i ) max = i+1;
		if ( max <= j ) max = j+1;
	    }

	    j = p->next[l];
	    if ( j != 0xff )
	    {
		li->summary[i] |= LINFO_NEXT;
		li->list[i][j] |= LINFO_NEXT;
		li->summary[j] |= LINFO_BY_NEXT;
		li->list[j][i] |= LINFO_BY_NEXT;
		if ( max <= i ) max = i+1;
		if ( max <= j ) max = j+1;
	    }
	}
    }

    noPRINT(">>> GMODE/group\n");
    //HEXDUMP16(0,0,li->gmode,max);
    return li->used = max;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint AnalyseRouteLinksPH ( kmp_linfo_t *li, const kmp_t *kmp, uint sect_ph )
{
    DASSERT(li);
    DASSERT(kmp);
    memset(li,0,sizeof(*li));


    //--- section dependent setup

    switch(sect_ph)
    {
	case KMP_CKPH:
	case KMP_CKPT:
	    li->sect_pt = KMP_CKPT;
	    li->sect_ph = KMP_CKPH;
	    break;

	case KMP_ENPH:
	case KMP_ENPT:
	    li->sect_pt = KMP_ENPT;
	    li->sect_ph = KMP_ENPH;
	    break;

	case KMP_ITPH:
	case KMP_ITPT:
	    li->sect_pt = KMP_ITPT;
	    li->sect_ph = KMP_ITPH;
	    break;

	default:
	    ASSERT_MSG(0,"AnalyseRouteLinksPH(): Unsupported KMP section: %u",sect_ph);
	    li->sect_pt = li->sect_ph = KMP_NO_SECT;
	    return 0;
    }


    //--- setup

    uint n = kmp->dlist[li->sect_ph].used;
    if ( n > KMP_MAX_LINFO )
	n = KMP_MAX_LINFO;

    const kmp_enph_entry_t *ph_list = (kmp_enph_entry_t*)kmp->dlist[li->sect_ph].list;


    //--- iterate groups

    uint i, max = 0;
    const kmp_enph_entry_t *p = ph_list;

    for ( i = 0; i < n; i++, p++ )
    {
	if (!p->pt_len)
	    continue;

	const uint iprev = p->pt_start;
	const uint inext = p->pt_start + p->pt_len - 1;

	kmp_linfo_g_mode gmode = 0;
	if (IsDispatchPointKMP(p))
	    gmode |= LINFOG_DISPATCH;
	uint pi;
	for ( pi = iprev; pi <= inext; pi++ )
	{
	    li->n_point[pi] = p->pt_len;
	    li->gmode[pi] = gmode;
	}

	uint l;
	for ( l = 0; l < KMP_MAX_PH_LINK; l++ )
	{
	    u8 j = p->prev[l];
	    if ( j != 0xff && j < n )
	    {
		const uint link = ph_list[j].pt_start + ph_list[j].pt_len - 1;
		if ( link < KMP_MAX_LINFO )
		{
		    li->summary[iprev]		|= LINFO_G_PREV;
		    li->list[iprev][link]	|= LINFO_G_PREV;
		    li->summary[link]		|= LINFO_G_BY_PREV;
		    li->list[link][iprev]	|= LINFO_G_BY_PREV;
		    if ( max <= iprev ) max = iprev+1;
		    if ( max <= link  ) max = link+1;
		}
	    }

	    if ( inext < KMP_MAX_LINFO )
	    {
		j = p->next[l];
		if ( j != 0xff && j < n )
		{
		    const uint link = ph_list[j].pt_start;
		    if ( link < KMP_MAX_LINFO )
		    {
			li->summary[inext]	|= LINFO_G_NEXT;
			li->list[inext][link]	|= LINFO_G_NEXT;
			li->summary[link]	|= LINFO_G_BY_NEXT;
			li->list[link][inext]	|= LINFO_G_BY_NEXT;
			if ( max <= inext ) max = inext+1;
			if ( max <= link  ) max = link+1;
		    }
		}
	    }
	}
    }

    noPRINT(">>> GMODE/points\n");
    //HEXDUMP16(0,0,li->gmode,max);

    return li->used = max;
}

///////////////////////////////////////////////////////////////////////////////

uint AnalyseRouteLinksKMP ( kmp_linfo_t *li, const kmp_t *kmp, uint sect_pt )
{
    DASSERT(li);
    DASSERT(kmp);


    //--- section dependent setup

    switch(sect_pt)
    {
	case KMP_CKPT:
	case KMP_CKPH:
	    return AnalyseRouteLinksCKPT(li,kmp);
	    break;

	case KMP_ENPT:
	case KMP_ENPH:
	case KMP_ITPT:
	case KMP_ITPH:
	    break;

	default:
	    ASSERT_MSG(0,"AnalyseRouteLinksKMP(): Unsupported KMP section: %u",sect_pt);
	    memset(li,0,sizeof(*li));
	    li->sect_pt = li->sect_ph = KMP_NO_SECT;
	    return 0;
    }


    //--- setup

    uint max = AnalyseRouteLinksPH(li,kmp,sect_pt);

    uint n = kmp->dlist[li->sect_pt].used;
    if ( n > KMP_MAX_LINFO )
	n = KMP_MAX_LINFO;
    if ( max < n )
	max = n;

    const kmp_enpt_entry_t *p = (kmp_enpt_entry_t*)kmp->dlist[li->sect_pt].list;


    //--- iterate points

    uint i;
    for ( i = 0; i < n; i++, p++ )
    {
	if ( i > 0 && !( li->summary[i] & LINFO_G_PREV ) )
	{
	    li->summary[i]	|= LINFO_D_PREV;
	    li->list[i][i-1]	|= LINFO_D_PREV;
	    li->summary[i-1]	|= LINFO_D_BY_PREV;
	    li->list[i-1][i]	|= LINFO_D_BY_PREV;
	}

	if ( i < KMP_MAX_LINFO-1 && !( li->summary[i] & LINFO_G_NEXT ) )
	{
	    li->summary[i]	|= LINFO_D_NEXT;
	    li->list[i][i+1]	|= LINFO_D_NEXT;
	    li->summary[i+1]	|= LINFO_D_BY_NEXT;
	    li->list[i+1][i]	|= LINFO_D_BY_NEXT;
	}
    }

    return li->used = max;
}

///////////////////////////////////////////////////////////////////////////////

uint AnalyseRouteLinksCKPT ( kmp_linfo_t *li, const kmp_t *kmp )
{
    DASSERT(li);
    DASSERT(kmp);
    memset(li,0,sizeof(*li));
    li->sect_pt = KMP_CKPT;
    li->sect_ph = KMP_CKPH;


    //--- setup

    uint max = AnalyseRouteLinksPH(li,kmp,KMP_CKPH);

    uint n = kmp->dlist[KMP_CKPT].used;
    if ( n > KMP_MAX_LINFO )
	n = KMP_MAX_LINFO;
    if ( max < n )
	max = n;

    const kmp_ckpt_entry_t *p = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;


    //--- iterate points

    uint i;
    for ( i = 0; i < n; i++, p++ )
    {
	uint j = p->prev;
	if ( j != 0xff )
	{
	    li->summary[i] |= LINFO_D_PREV;
	    li->list[i][j] |= LINFO_D_PREV;
	    li->summary[j] |= LINFO_D_BY_PREV;
	    li->list[j][i] |= LINFO_D_BY_PREV;
	}

	j = p->next;
	if ( j != 0xff )
	{
	    li->summary[i] |= LINFO_D_NEXT;
	    li->list[i][j] |= LINFO_D_NEXT;
	    li->summary[j] |= LINFO_D_BY_NEXT;
	    li->list[j][i] |= LINFO_D_BY_NEXT;
	}
    }

    return li->used = max;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintLinkInfoMode ( kmp_linfo_mode mode, int print_group, bool adjust )
{
    // print_group: -1=never, 0:auto, 1:always
    char *buf = GetCircBuf(10);
    char *dest = buf;

    if (adjust)
    {
	*dest++ = mode & LINFO_D_PREV		? 'P' : '-';
	*dest++ = mode & LINFO_D_NEXT		? 'N' : '-';
	*dest++ = mode & LINFO_D_BY_PREV	? 'p' : '-';
	*dest++ = mode & LINFO_D_BY_NEXT	? 'n' : '-';

	if ( print_group > 0 || !print_group && mode&LINFO_M_G )
	{
	    *dest++ = mode & LINFO_G_PREV	? 'P' : '-';
	    *dest++ = mode & LINFO_G_NEXT	? 'N' : '-';
	    *dest++ = mode & LINFO_G_BY_PREV	? 'p' : '-';
	    *dest++ = mode & LINFO_G_BY_NEXT	? 'n' : '-';
	}
    }
    else
    {
	if ( mode & LINFO_D_PREV )		*dest++ = 'P';
	if ( mode & LINFO_D_NEXT )		*dest++ = 'N';
	if ( mode & LINFO_D_BY_PREV )		*dest++ = 'p';
	if ( mode & LINFO_D_BY_NEXT )		*dest++ = 'n';

	if ( print_group >= 0 &&  mode&LINFO_M_G )
	{
	    *dest++ = ',';
	    if ( mode & LINFO_G_PREV )		*dest++ = 'P';
	    if ( mode & LINFO_G_NEXT )		*dest++ = 'N';
	    if ( mode & LINFO_G_BY_PREV )	*dest++ = 'p';
	    if ( mode & LINFO_G_BY_NEXT )	*dest++ = 'n';
	}
    }
    *dest = 0;
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    kmp parser function get()		///////////////
///////////////////////////////////////////////////////////////////////////////

enum // get mode
{
    GTMD_N,		// number of elements

    GTMD_HD_U16,	// head 'value' as u16
    GTMD_HD_U8HI,	// hight byte of head 'value'
    GTMD_HD_U8LO,	// low byte of head 'value'

    GTMD__DATA,		// separator of header info and data info

    GTMD_U8,		// unsigned 8 bit
    GTMD_U8M1,		// unsigned 8 bit, but 0xff is -1
    GTMD_U16,		// unsigned 16 bit
    GTMD_U16M1,		// unsigned 16 bit, but 0xffff is -1

    GTMD_DBL,		// double value
    GTMD_DBL2,		// 2d vector
    GTMD_DBL3,		// 3d double vector
};

///////////////////////////////////////////////////////////////////////////////
// [[kmp_func_t]]

#define POTI_PT_OFF 0x10

typedef struct kmp_func_t
{
    char	name[18];	// variable name
    u16		sect;		// section
    u16		off;		// data offset, for POTI: off>POTI_PT_OFF: route point
    u16		get_mode;	// see enum above
    u8		n_var;		// number of variables
    u8		n_param;	// number of function parameters
    ccp		get_info;	// NULL or function info for get function
    ccp		set_info;	// NULL or function info for set function

} kmp_func_t;

///////////////////////////////////////////////////////////////////////////////

static const kmp_func_t kmp_func_list[] =
{
    { "KTPT$N",			KMP_KTPT,    0,  GTMD_N,	0,0,
			"Returns the number of start points.",
			0 },
    { "KTPT$POS",		KMP_KTPT, 0x00,  GTMD_DBL3,	1,1,
			"Returns a start point position.",
			0 },
    { "KTPT$ROT",		KMP_KTPT, 0x0c,  GTMD_DBL3,	1,1,
			"Returns a start point rotation.",
			0 },
    { "KTPT$ID",		KMP_KTPT, 0x18,  GTMD_U16M1,	1,1,
			"Returns a start point ID.",
			0 },
    { "KTPT$UNKNOWN",		KMP_KTPT, 0x1a,  GTMD_U16,	1,1,
			"Returns the unknown value of a start point.",
			0 },

    //---------------

    { "ENPT$N",			KMP_ENPT,    0,  GTMD_N,	0,0,
			"Returns the number of enemy points.",
			0 },
    { "ENPT$POS",		KMP_ENPT, 0x00,  GTMD_DBL3,	1,1,
			"Returns the position of an enemy point.",
			0 },
    { "ENPT$SCALE",		KMP_ENPT, 0x0c,  GTMD_DBL,	1,1,
			"Returns the scale of an enemy point.",
			0 },
    { "ENPT$PROP",		KMP_ENPT, 0x10,  GTMD_U16,	2,2,
			"Returns property 0 or 1 of an enemy point.",
			0 },

    //---------------

    { "ENPH$N",			KMP_ENPH,    0,  GTMD_N,	0,0,
			"Returns the number of enemy point groups.",
			0 },
    { "ENPH$START",		KMP_ENPH, 0x00,  GTMD_U8,	1,1,
			"Returns the enemy point start index of an enemy point group.",
			0 },
    { "ENPH$LEN",		KMP_ENPH, 0x01,  GTMD_U8,	1,1,
			"Returns the number of enemy points of an enemy point group.",
			0 },
    { "ENPH$PREV",		KMP_ENPH, 0x02,  GTMD_U8M1,	KMP_MAX_PH_LINK,2,
			"Returns link 0..5 to the previous enemy point group.",
			0 },
    { "ENPH$NEXT",		KMP_ENPH, 0x08,  GTMD_U8M1,	KMP_MAX_PH_LINK,2,
			"Returns link 0..5 to the next enemy point group.",
			0 },
    { "ENPH$UNKNOWN",		KMP_ENPH, 0x0e,  GTMD_U16,	1,1,
			"Returns the unknown value of an enemy point group.",
			0 },

    //---------------

    { "ITPT$N",			KMP_ITPT,    0,  GTMD_N,	0,0,
			"Returns the number of item points.",
			0 },
    { "ITPT$POS",		KMP_ITPT, 0x00,  GTMD_DBL3,	1,1,
			"Returns the position of an item point.",
			"Set the position of an item point"
			" and returns 1 on success ans 0 , if failed." },
    { "ITPT$SCALE",		KMP_ITPT, 0x0c,  GTMD_DBL,	1,1,
			"Returns the scale of an item point.",
			0 },
    { "ITPT$PROP",		KMP_ITPT, 0x10,  GTMD_U16,	2,2,
			"Returns property 0 or 1 of an item point.",
			0 },

    //---------------

    { "ITPH$N",			KMP_ITPH,    0,  GTMD_N,	0,0,
			"Returns the number of item point groups.",
			0 },
    { "ITPH$START",		KMP_ITPH, 0x00,  GTMD_U8,	1,1,
			"Returns the item point start index of an item point group.",
			0 },
    { "ITPH$LEN",		KMP_ITPH, 0x01,  GTMD_U8,	1,1,
			"Returns the number of item points of an item point group.",
			0 },
    { "ITPH$PREV",		KMP_ITPH, 0x02,  GTMD_U8M1,	KMP_MAX_PH_LINK,2,
			"Returns link 0..5 to the previous item point group.",
			0 },
    { "ITPH$NEXT",		KMP_ITPH, 0x08,  GTMD_U8M1,	KMP_MAX_PH_LINK,2,
			"Returns link 0..5 to the next item point group.",
			0 },
    { "ITPH$UNKNOWN",		KMP_ITPH, 0x0e,  GTMD_U16,	1,1,
			"Returns the unknown value of an item point group.",
			0 },

    //---------------

    { "CKPT$N",			KMP_CKPT,    0,  GTMD_N,	0,0,
			"Returns the number of check points.",
			0 },
    { "CKPT$LEFT",		KMP_CKPT, 0x00,  GTMD_DBL2,	1,1,
			"Returns the left position of a check point.",
			0 },
    { "CKPT$RIGHT",		KMP_CKPT, 0x08,  GTMD_DBL2,	1,1,
			"Returns the right position of a check point.",
			0 },
    { "CKPT$RESPAWN",		KMP_CKPT, 0x10,  GTMD_U8,	1,1,
			"Returns the respawn index of a check point.",
			0 },
    { "CKPT$MODE",		KMP_CKPT, 0x11,  GTMD_U8M1,	1,1,
			"Returns the mode of check point.",
			0 },
    { "CKPT$PREV",		KMP_CKPT, 0x12,  GTMD_U8M1,	1,1,
			"Returns the index of the previous check point.",
			0 },
    { "CKPT$NEXT",		KMP_CKPT, 0x13,  GTMD_U8M1,	1,1,
			"Returns the index of the next check point.",
			0 },

    //---------------

    { "CKPH$N",			KMP_CKPH,    0,  GTMD_N,	0,0,
			"Returns the number of check point groups.",
			0 },
    { "CKPH$START",		KMP_CKPH, 0x00,  GTMD_U8,	1,1,
			"Returns the check point start index of a check point group.",
			0 },
    { "CKPH$LEN",		KMP_CKPH, 0x01,  GTMD_U8,	1,1,
			"Returns the number of check points of a check point group.",
			0 },
    { "CKPH$PREV",		KMP_CKPH, 0x02,  GTMD_U8M1,	KMP_MAX_PH_LINK,2,
			"Returns link 0..5 to the previous check point group.",
			0 },
    { "CKPH$NEXT",		KMP_CKPH, 0x08,  GTMD_U8M1,	KMP_MAX_PH_LINK,2,
			"Returns link 0..5 to the next check point group.",
			0 },
    { "CKPH$UNKNOWN",		KMP_CKPH, 0x0e,  GTMD_U16,	1,1,
			"Returns the unknown value of a check point group.",
			0 },

    //---------------

    { "GOBJ$N",			KMP_GOBJ,    0,  GTMD_N,	0,0,
			"Returns the number of global objects.",
			0 },
    { "GOBJ$ID",		KMP_GOBJ, 0x00,  GTMD_U16,	1,1,
			"Returns the object ID of a global object.",
			0 },
    { "GOBJ$UNKNOWN",		KMP_GOBJ, 0x02,  GTMD_U16,	1,1,
			"Returns the unknown value of a global object.",
			0 },
    { "GOBJ$POS",		KMP_GOBJ, 0x04,  GTMD_DBL3,	1,1,
			"Returns the position of a global object.",
			0 },
    { "GOBJ$ROT",		KMP_GOBJ, 0x10,  GTMD_DBL3,	1,1,
			"Returns the rotation of a global object.",
			0 },
    { "GOBJ$SCALE",		KMP_GOBJ, 0x1c,  GTMD_DBL3,	1,1,
			"Returns the scale of a global object.",
			0 },
    { "GOBJ$ROUTE",		KMP_GOBJ, 0x28,  GTMD_U16M1,	1,1,
			"Returns -1 or the index of a related route"
			" of a global object.",
			0 },
    { "GOBJ$SET",		KMP_GOBJ, 0x2a,  GTMD_U16,	8,2,
			"Returns setting 0..7 of a global object.",
			0 },
    { "GOBJ$PFLAGS",		KMP_GOBJ, 0x3a,  GTMD_U16,	1,1,
			"Returns the presence flags of a global object.",
			0 },

    //---------------

    { "POTI$N",			KMP_POTI,    0,  GTMD_N,	0,0,
			"Returns the number of routes.",
			0 },
     { "POTI$NN",		KMP_POTI,    0,  GTMD_HD_U16,	0,0,
			"Returns the total number of points of all routes.",
			0 },
    { "POTI$NP",		KMP_POTI, 0x00,  GTMD_U8,	1,1,
			"Returns the number of points of a route.",
			0 },
    { "POTI$SMOOTH",		KMP_POTI, 0x02,  GTMD_U8,	1,1,
			"Returns the smooth value of a route"
			" (route header byte at offset 0x02).",
			0 },
    { "POTI$BACK",		KMP_POTI, 0x03,  GTMD_U8,	1,1,
			"Returns the forward+backward value of a route"
			" (route header byte at offset 0x03).",
			0 },
     { "POTI$POS",		KMP_POTI, 0x10,  GTMD_DBL3,	1,2, // off+POTI_PT_OFF
			"Returns the position of a point of a route."
			" The point index is relative to the route.",
			0 },
     { "POTI$SET",		KMP_POTI, 0x1c,  GTMD_U16,	2,3, // off+POTI_PT_OFF
			"Returns setting 0 or 1 of a point of a route."
			" The point index is relative to the route.",
			0 },

    //---------------

    { "AREA$N",			KMP_AREA,    0,  GTMD_N,	0,0,
			"Returns the number of areas.",
			0 },
    { "AREA$MODE",		KMP_AREA, 0x00,  GTMD_U8M1,	1,1,
			"Returns the mode of an area.",
			0 },
    { "AREA$TYPE",		KMP_AREA, 0x01,  GTMD_U8M1,	1,1,
			"Returns the type of an area. Cameras are type 0.",
			0 },
    { "AREA$DEST",		KMP_AREA, 0x02,  GTMD_U8M1,	1,1,
			"Returns the destination index (e.g. camera index) of an area.",
			0 },
    { "AREA$UNKNOWN",		KMP_AREA, 0x03,  GTMD_U8,	1,1,
			"Returns the unknown value of an area.",
			0 },
    { "AREA$POS",		KMP_AREA, 0x04,  GTMD_DBL3,	1,1,
			"Returns the position of an area.",
			0 },
    { "AREA$ROT",		KMP_AREA, 0x10,  GTMD_DBL3,	1,1,
			"Returns the rotation of an area.",
			0 },
    { "AREA$SCALE",		KMP_AREA, 0x1c,  GTMD_DBL3,	1,1,
			"Returns the scale of an area.",
			0 },
    { "AREA$SET",		KMP_AREA, 0x28,  GTMD_U16,	4,2,
			"Returns setting 0..3 of an area.",
			0 },

    //---------------

    { "CAME$N",			KMP_CAME,    0,  GTMD_N,	0,0,
			"Returns the number of cameras.",
			0 },
     { "CAME$OCAM",		KMP_CAME,    0,  GTMD_HD_U8HI,	0,0,
			"Returns the index of the opening camera.",
			0 },
     { "CAME$SCAM",		KMP_CAME,    0,  GTMD_HD_U8LO,	0,0,
			"Returns the index of the selection camera.",
			0 },
    { "CAME$TYPE",		KMP_CAME, 0x00,  GTMD_U8,	1,1,
			"Returns the type of a camera.",
			0 },
    { "CAME$NEXT",		KMP_CAME, 0x01,  GTMD_U8M1,	1,1,
			"Returns -1 or the link to the next camera.",
			0 },
    { "CAME$UNKNOWN_02",	KMP_CAME, 0x02,  GTMD_U8,	1,1,
			"Returns the unknown value at offser 0x02 of a camera.",
			0 },
    { "CAME$ROUTE",		KMP_CAME, 0x03,  GTMD_U8M1,	1,1,
			"Returns -1 or the link to the related route.",
			0 },
    { "CAME$UNKNOWN_04",	KMP_CAME, 0x04,  GTMD_U16,	1,1,
			"Returns the unknown value at offser 0x04 of a camera.",
			0 },
    { "CAME$ZOOM_SPEED",	KMP_CAME, 0x06,  GTMD_U16,	1,1,
			"Returns the zoom speed in units per 100/60 sec.",
			0 },
    { "CAME$VIEWPT_SPEED",	KMP_CAME, 0x08,  GTMD_U16,	1,1,
			"Returns the view point speed in units per 100/60 sec.",
			0 },
    { "CAME$UNKNOWN_0A",	KMP_CAME, 0x0a,  GTMD_U16,	1,1,
			"Returns the unknown value at offser 0x0a of a camera.",
			0 },
    { "CAME$POS",		KMP_CAME, 0x0c,  GTMD_DBL3,	1,1,
			"Returns the position of a camera.",
			0 },
    { "CAME$UNKNOWN_18",	KMP_CAME, 0x18,  GTMD_DBL3,	1,1,
			"Returns the unknown value at offser 0x18 of a camera.",
			0 },
    { "CAME$ZOOM_BEGIN",	KMP_CAME, 0x24,  GTMD_DBL,	1,1,
			"Returns the zoom beginning angle of a camera.",
			0 },
    { "CAME$ZOOM_END",		KMP_CAME, 0x28,  GTMD_DBL,	1,1,
			"Returns the zoom ending angle of a camera.",
			0 },
    { "CAME$VIEWPT_BEGIN",	KMP_CAME, 0x2c,  GTMD_DBL3,	1,1,
			"Returns the start position of the view point.",
			0 },
    { "CAME$VIEWPT_END",	KMP_CAME, 0x38,  GTMD_DBL3,	1,1,
			"Returns the end position of the view point.",
			0 },
    { "CAME$TIME",		KMP_CAME, 0x44,  GTMD_DBL,	1,1,
			"Returns the run time of a camera in 1/60 sec.",
			0 },

    //---------------

    { "JGPT$N",			KMP_JGPT,    0,  GTMD_N,	0,0,
			"Returns the number of respan points.",
			0 },
    { "JGPT$POS",		KMP_JGPT, 0x00,  GTMD_DBL3,	1,1,
			"Returns the position of a respawn point.",
			0 },
    { "JGPT$ROT",		KMP_JGPT, 0x0c,  GTMD_DBL3,	1,1,
			"Returns the rotation of a respawn point.",
			0 },
    { "JGPT$ID",		KMP_JGPT, 0x18,  GTMD_U16,	1,1,
			"Returns the ID of a respawn point.",
			0 },
    { "JGPT$EFFECT",		KMP_JGPT, 0x1a,  GTMD_U16,	1,1,
			"Returns the effect value of a respawn point.",
			0 },

    //---------------

    { "CNPT$N",			KMP_CNPT,    0,  GTMD_N,	0,0,
			"Returns the number of canon point points.",
			0 },
    { "CNPT$POS",		KMP_CNPT, 0x00,  GTMD_DBL3,	1,1,
			"Returns the position of a canon point.",
			0 },
    { "CNPT$ROT",		KMP_CNPT, 0x0c,  GTMD_DBL3,	1,1,
			"Returns the rotation of a canon point.",
			0 },
    { "CNPT$ID",		KMP_CNPT, 0x18,  GTMD_U16,	1,1,
			"Returns the ID of a canon point.",
			0 },
    { "CNPT$EFFECT",		KMP_CNPT, 0x1a,  GTMD_U16,	1,1,
			"Returns the efffect value of a canon point.",
			0 },

    //---------------

    { "MSPT$N",			KMP_MSPT,    0,  GTMD_N,	0,0,
			"Returns the number of battle points.",
			0 },
    { "MSPT$POS",		KMP_MSPT, 0x00,  GTMD_DBL3,	1,1,
			"Returns the position of a battle point.",
			0 },
    { "MSPT$ROT",		KMP_MSPT, 0x0c,  GTMD_DBL3,	1,1,
			"Returns the rotation of a battle point.",
			0 },
    { "MSPT$ID",		KMP_MSPT, 0x18,  GTMD_U16,	1,1,
			"Returns the ID of a battle point.",
			0 },
    { "MSPT$EFFECT",		KMP_MSPT, 0x1a,  GTMD_U16,	1,1,
			"Returns the efffect value of a battle point.",
			0 },

    //---------------

    { "STGI$N",			KMP_STGI,    0,  GTMD_N,	0,0,
			"Returns the number of stage infos.",
			0 },
    { "STGI$BYTE",		KMP_STGI, 0x00,  GTMD_U8,	12,2,
			"Returns setting byte 0..11 of a stage info.",
			0 },
};

#define KMP_FUNC_LIST_N (sizeof(kmp_func_list)/sizeof(*kmp_func_list))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_get
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(fpar);
    DASSERT( fpar->user_id >= 0 && fpar->user_id < KMP_FUNC_LIST_N );

    if ( si && si->kmp )
    {
	const kmp_func_t *fp = kmp_func_list + fpar->user_id;
	DASSERT( n_param >= fp->n_param );
	DASSERT( fp->sect < KMP_N_SECT );
	DASSERT( fp->get_info );

	const List_t *dlist = si->kmp->dlist + fp->sect;
	uint max_index1 = dlist->used;
	int val = n_param > 0 ? GetIntV(param) : 0;
	uint index1 = val < 0 ? val + max_index1 : val;
	const u8 *data = dlist->list + dlist->elem_size * index1 + fp->off;

	if ( fp->sect == KMP_POTI && fp->off >= POTI_PT_OFF && index1 < max_index1 )
	{
	    param++;
	    n_param--;

	    const kmp_poti_group_t *pgrp = (kmp_poti_group_t*)dlist->list;
	    uint pt_start = 0;
	    while ( index1-- > 0 )
		pt_start += (pgrp++)->n_point;

	    dlist = &si->kmp->poti_point;
	    max_index1 = pgrp->n_point;
	    val = n_param > 0 ? GetIntV(param) : 0;
	    index1 = val < 0 ? val + max_index1 : val;
	    PRINT("POTI/PT: index= %u/%u  +%u\n",index1,max_index1,pt_start);
	    data = dlist->list
		 + dlist->elem_size * ( pt_start + index1 )
		 + fp->off - POTI_PT_OFF;
	}

	val = n_param > 1 ? GetIntV(param+1) : 0;
	const uint index2 = val < 0 ? val + fp->n_var : val;

	const bool valid_index12 = index1 < max_index1 && index2 < fp->n_var;
	noPRINT("index1=%d/%d, index2=%d/%d, valid=%d\n",
		index1, max_index1, index2, fp->n_var, valid_index12 );

	switch(fp->get_mode)
	{
	    case GTMD_N:
		res->i = dlist->used;
		res->mode = VAR_INT;
		return ERR_OK;

	    case GTMD_HD_U16:
		res->i = si->kmp->value[fp->sect];
		res->mode = VAR_INT;
		return ERR_OK;

	    case GTMD_HD_U8HI:
		res->i = si->kmp->value[fp->sect] >> 8;
		res->mode = VAR_INT;
		return ERR_OK;

	    case GTMD_HD_U8LO:
		res->i = si->kmp->value[fp->sect] & 0xff;
		res->mode = VAR_INT;
		return ERR_OK;

	    case GTMD_U8:
	    case GTMD_U8M1:
		if (valid_index12)
		{
		    const u8 val = data[index2];
		    res->i = val == 0xff && fp->get_mode == GTMD_U8M1 ? -1 : (int)val;
		    res->mode = VAR_INT;
		    return ERR_OK;
		}
		break;

	    case GTMD_U16:
	    case GTMD_U16M1:
		if (valid_index12)
		{
		    const u16 val = ((u16*)data)[index2];
		    res->i = val == 0xffff && fp->get_mode == GTMD_U16M1
				? -1 : (int)val;
		    res->mode = VAR_INT;
		    return ERR_OK;
		}
		break;

	    case GTMD_DBL:
		if (valid_index12)
		    return ResultFloat( res, (float*)data + index2, 1 );
		break;

	    case GTMD_DBL2:
		if (valid_index12)
		    return ResultFloat( res, (float*)data + 2 * index2, 2 );
		break;

	    case GTMD_DBL3:
		if (valid_index12)
		    return ResultFloat( res, (float*)data + 3 * index2, 3 );
		break;
	}
    }

    res->i = 0;
    res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
#ifdef TEST

static enumError F_set
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(fpar);
    DASSERT( fpar->user_id >= 0 && fpar->user_id < KMP_FUNC_LIST_N );

    if ( si && si->kmp )
    {
	const kmp_func_t *fp = kmp_func_list + fpar->user_id;
	DASSERT( n_param >= fp->n_param );
	DASSERT( fp->sect < KMP_N_SECT );
	DASSERT( fp->set_info );

	PRINT(">>> F_SET <<<\n");
	// ...
    }

    res->i = 0;
    res->mode = VAR_UNSET;
    return ERR_OK;
}

#endif
//
///////////////////////////////////////////////////////////////////////////////
///////////////			  kmp_group_t			///////////////
///////////////////////////////////////////////////////////////////////////////

static void InitializeGL
(
    kmp_group_list_t	*gl,		// list to initialize
    int			selector,
    kmp_t		*kmp,		// related KMP
    kmp_ph_t		*ph		// related PH info
)
{
    DASSERT(gl);
    DASSERT(kmp);
    DASSERT(ph);

    memset(gl,0,sizeof(*gl));
    gl->kmp = kmp;
    gl->ph  = ph;
    gl->selector = toupper(selector);
    InitializeLine(&gl->line);
}

///////////////////////////////////////////////////////////////////////////////

static uint ScanGroupNames
(
    kmp_group_list_t	* gl,		// group list
    kmp_group_elem_t	* elist,	// name list
    ScanInfo_t		* si		// valid scan info
)
{
    DASSERT(gl);
    DASSERT(elist);
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    char name[KMP_MAX_NAME_LEN+3];

    uint idx = 0;
    while ( idx < KMP_MAX_PH_LINK )
    {
	const kmp_oneway_mode oneway
		= NextCharSI(si,false) == '>' ? ONEWAY_BOTH : ONEWAY_NONE;
	if ( oneway != ONEWAY_NONE )
	    sf->ptr++;

	ScanNameSI(si,name,sizeof(name),true,true,0);
	if ( gl->selector && *sf->ptr == '!' )
	{
	    int ch = *++sf->ptr;
	    if (ch)
	    {
		sf->ptr++;
		if ( toupper(ch) != gl->selector )
		    continue;
	    }
	}

	uint n = 1;
	if ( *sf->ptr == '*' )
	{
	    sf->ptr++;
	    const uint num = *sf->ptr - '0';
	    if ( num < 10 )
	    {
		sf->ptr++;
		if ( gl->selector == 'E' )
		    n = num;
	    }
	}

	while ( n-- > 0 && idx < KMP_MAX_PH_LINK )
	{
	    memcpy(elist->name,name,sizeof(elist->name));
	    elist->oneway = oneway;
	    elist++;
	    idx++;
	}
    }

    return idx;
}

///////////////////////////////////////////////////////////////////////////////

static enumError GroupPrevGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si		// valid scan info
)
{
    DASSERT(gl);
    DASSERT(si);

    if (!gl->used)
    {
	GotoEolSI(si);
	return ERR_OK;
    }

    SkipCharSI(si,':');
    const int idx = gl->used - 1;
    kmp_group_t *grp = gl->group + idx;
    grp->have_prev_cmd = true;

 #if 0 // [[auto-prev]]
    if ( idx < KMP_MAX_GROUP )
    {
	kmp_ph_t *ph = grp->ph;
	if (ph)
	    ph->gopt[idx].have_prev_cmd = true;
    }
 #endif

    ScanGroupNames(gl,grp->prev,si);
    //HEXDUMP16(0,0,grp->prev,sizeof(grp->prev));
    return CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////

static enumError DefaultClassGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si		// valid scan info
)
{
    DASSERT(gl);
    DASSERT(gl->ph);
    DASSERT(si);

    char name[20];
    SkipCharSI(si,':');
    if (!ScanNameSI(si,name,sizeof(name),true,true,0))
    {
	GotoEolSI(si);
	return ERR_WARNING;
    }

    int stat = InsertClassNamePH(gl->ph,name,false);
    noPRINT("NEW DEFAULT CLASS: %d [%s]\n",stat,name);
    if ( stat >= 0 )
	gl->ph->default_class = stat;

    return CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////

static enumError ClassGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si,		// valid scan info
    kmp_gopt2_t		* go2		// options to set
)
{
    DASSERT(gl);
    DASSERT(gl->ph);
    DASSERT(si);

    char name[20];
    SkipCharSI(si,':');
    if (!ScanNameSI(si,name,sizeof(name),true,true,0))
    {
	GotoEolSI(si);
	return ERR_WARNING;
    }

    const int stat = InsertClassNamePH(gl->ph,name,false);
    noPRINT("NEW CLASS: %d [%s]\n",stat,name);
    if ( stat >= 0 )
    {
	go2->prev.rclass = go2->prev.lclass =
	go2->next.rclass = go2->next.lclass = stat;

	if ( SkipCharSI(si,',') && ScanNameSI(si,name,sizeof(name),true,true,0) )
	{
	    const int stat = InsertClassNamePH(gl->ph,name,false);
	    noPRINT("NEW NEXT CLASS: %d [%s]\n",stat,name);
	    if ( stat >= 0 )
		go2->next.rclass = go2->next.lclass = stat;
	}
    }

    return CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////

static enumError ACClassGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si,		// valid scan info
    kmp_gopt2_t		* go2,		// options to set
    kmp_gopt_t		* gopt		// prev or next
)
{
    DASSERT(gl);
    DASSERT(gl->ph);
    DASSERT(si);
    DASSERT(gopt);

    char name[20];
    SkipCharSI(si,':');
    if (!ScanNameSI(si,name,sizeof(name),true,true,0))
    {
	GotoEolSI(si);
	return ERR_WARNING;
    }

    int stat = InsertClassNamePH(gl->ph,name,false);
    noPRINT("NEW LINK CLASS: %d [%s]\n",stat,name);
    if ( stat >= 0 )
	gopt->lclass = stat;

    return CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////

static enumError GroupSettingsGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si,		// valid scan info
    kmp_gopt2_t		* go2		// options to set
)
{
    DASSERT(gl);
    DASSERT(si);
    DASSERT(go2);

    if (!gl->used)
	return ERR_WARNING;

    long set0 = 0, set1 = 0;
    kmp_rtype_mode rtype = 0;

    SkipCharSI(si,':');
    enumError err = ScanUValueSI(si,&set0,0);
    if ( !err && SkipCharSI(si,',') )
    {
	err = ScanUValueSI(si,&set1,0);
	if ( !err && SkipCharSI(si,',') )
	{
	    char name[20];
	    ScanNameSI(si,name,sizeof(name),true,true,0);
	    int stat;
	    const KeywordTab_t *key = ScanKeyword(&stat,name,kmp_rtype_name);
	    if (!key)
	    {
		err = ERR_WARNING;
		if (!si->no_warn)
		    PrintKeywordError(kmp_rtype_name,name,stat,0,"$SETTINGS");
	    }
	    else
		rtype = key->id;
	}
    }

    go2->rtype_user = rtype;
    go2->setting[0] = set0;
    go2->setting[1] = set1;

    noPRINT("\e[36;1m rtype %d, g-settings: %u 0x%02x [%d]\e[0m\n",
		go2->rtype_user, go2->setting[0], go2->setting[1], err );
    if (err)
    {
	GotoEolSI(si);
	return err;
    }

    return CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////

static enumError GroupOnewayGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si,		// valid scan info
    kmp_gopt2_t		* go2		// options to set
)
{
    DASSERT(gl);
    DASSERT(si);
    DASSERT(go2);

    if (!gl->used)
	return ERR_WARNING;

    SkipCharSI(si,':');

    char name[20];
    ScanNameSI(si,name,sizeof(name),true,true,0);
    int stat;
    const KeywordTab_t *key = ScanKeyword(&stat,name,kmp_oneway_name);
    if (!key)
    {
	if (!si->no_warn)
	    PrintKeywordError(kmp_oneway_name,name,stat,0,"$ONEWAY");
	GotoEolSI(si);
	return ERR_WARNING;
    }

    go2->ac_oneway = key->id;
    return CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError AddGroupGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si,		// valid scan info
    char		var_prefix,	// Prefix for var names
    uint		index,		// index of first point
    uint		n_dim		// Line_t support: number of dimensions
)
{
    DASSERT(gl);
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    char name[KMP_MAX_NAME_LEN+3];
    ScanNameSI(si,name,sizeof(name),true,true,0);

    const bool split_group = !strcmp(name,"$SPLIT");
    if (split_group)
    {
	if ( gl->ph && gl->ph->ac_mode != KMP_AC_PREV && gl->ph->ac_mode != KMP_AC_AUTO_PREV )
	{
	    si->total_err++;
	    sf->line_err++;
	    CheckEolSI(si);
	    if (!si->no_warn)
		ERROR0(ERR_WARNING,
		    "Command '$SPLIT' only possible if AUTO-CONNECT is set to AC$PREV"
		    " or AC$AUTO_PREV => ignored [%s @%u].\n", sf->name, sf->line );
	    return ERR_WARNING;
	}

	if ( !gl->used || index == gl->group[gl->used-1].first_index )
	{
	    si->total_err++;
	    sf->line_err++;
	    CheckEolSI(si);
	    if (!si->no_warn)
		ERROR0(ERR_WARNING,
		    "Command '$SPLIT' ignored [%s @%u].\n", sf->name, sf->line );
	    return ERR_WARNING;
	}
    }
    else if ( strcmp(name,"$GROUP") )
    {
	if ( !strcmp(name,"$PREV") )
	    return GroupPrevGL(gl,si);
	if (!strcmp(name,"$LINE"))
	    return ScanLine(&gl->line,si,n_dim,LM_LINE);
	if (!strcmp(name,"$BORDER"))
	    return ScanLine(&gl->line,si,n_dim,LM_BORDER);
	if (!strcmp(name,"$BEZIER"))
	    return ScanLine(&gl->line,si,n_dim,LM_BEZIER);

	if ( gl->ph )
	{
	    if ( !strcmp(name,"$DEF-CLASS") )
		return DefaultClassGL(gl,si);

	    if ( gl->used > 0 && gl->used <= KMP_MAX_GROUP )
	    {
		kmp_gopt2_t *go2 = gl->ph->gopt + gl->used - 1;
		if ( !strcmp(name,"$SETTINGS") )
		    return GroupSettingsGL(gl,si,go2);
		if ( !strcmp(name,"$ONEWAY") )
		    return GroupOnewayGL(gl,si,go2);
		if ( !strcmp(name,"$CLASS") )
		    return ClassGL(gl,si,go2);
		if ( !strcmp(name,"$AC-PREV") )
		    return ACClassGL(gl,si,go2,&go2->prev);
		if ( !strcmp(name,"$AC-NEXT") )
		    return ACClassGL(gl,si,go2,&go2->next);
	    }
	}

	si->total_err++;
	sf->line_err++;
	CheckEolSI(si);
	return ERROR0(ERR_WARNING,
		"Line ignored [%s @%u]: %s ...\n",
		sf->name, sf->line, name );
    }

    if ( gl->used >= KMP_MAX_GROUP )
    {
	si->total_err++;
	sf->line_err++;
	CheckEolSI(si);
	return ERROR0(ERR_WARNING,
		"To many groups, new group ignored: %s @%u\n",
		sf->name, sf->line );
    }


    // before incrementing the group index
    if (gl->ph)
    {
	noPRINT("NEW GROUP: %d, class %d\n",gl->used,gl->ph->default_class);
	kmp_gopt2_t *go2 = gl->ph->gopt + gl->used;
	go2->prev.rclass = go2->prev.lclass =
	go2->next.rclass = go2->next.lclass = gl->ph->default_class;
    }

    // create new group by incrementing group index
    kmp_group_t *grp = gl->group + gl->used++;
    grp->first_index = grp->last_index = index;

    if ( NextCharSI(si,false) == '-' )
    {
	sf->ptr++;
	grp->no_rotate = true;
    }

    ScanNameSI(si,name+2,sizeof(name)-2,true,true,0);
    memcpy(grp->name,name+2,sizeof(grp->name));
    name[0] = var_prefix;
    name[1] = '.';
    DefineIntVar(&si->gvar,name,gl->used-1);

    if (split_group)
    {
	kmp_group_t *prev = gl->group + gl->used - 2;
	DASSERT( prev >= gl->group );
	memcpy(grp->next,prev->next,sizeof(grp->next));
	memset(prev->next,0,sizeof(prev->next));
	memcpy(prev->next->name,name,sizeof(prev->next->name));

	CheckEolSI(si);
	return ERR_OK;
    }

    for(;;)
    {
	char ch = NextCharSI(si,false);
	noPRINT("--> %02x : %.6s\n",ch,sf->ptr);
	if (!ch)
	    break;
	sf->ptr++;
	if ( ch == ':' )
	    break;
    }

    ScanGroupNames(gl,grp->next,si);
    CheckEolSI(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static uint AddPointGL
(
    kmp_group_list_t	* gl,		// group list
    uint		index		// index of first point
)
{
    if (!gl->used)
	gl->used++;

    kmp_group_t *grp = gl->group + gl->used - 1;
    grp->last_index = index;
    return index - grp->first_index;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int SearchNearestDP
(
    kmp_group_list_t	* gl,		// group list
    kmp_t		* kmp,		// KMP data structure
    kmp_class_mode	cls,		// one of KCLS_*
    const float		* base		// base position
)
{
    DASSERT(gl);
    DASSERT(kmp);
    const kmp_ph_t *ph = gl->ph;
    DASSERT(ph);
    DASSERT( ph->sect_ph == KMP_ENPH || ph->sect_ph == KMP_ITPH );
    DASSERT( ph->sect_pt == KMP_ENPT || ph->sect_pt == KMP_ITPT );

    if ( cls == KCLS_OFF )
	return -1;

    int i, found = -1;
    kmp_group_t *grp;
    double found_distance = 0.0;

    List_t *list = kmp->dlist + ph->sect_ph;
    kmp_enph_entry_t *ph_list = (kmp_enph_entry_t*)(list->list);

    for ( i = 0, grp = gl->group; i < gl->used; i++, grp++ )
    {
	const kmp_gopt2_t *go2 = ph->gopt + i;
	if ( go2->rtype_active != KMP_RT_DISPATCH )
	    continue;

	kmp_class_mode dcls = go2->prev.rclass;
	if ( dcls == KCLS_OFF || cls != dcls && cls != KCLS_ANY && dcls != KCLS_ANY )
	    continue;

	noPRINT("--- CANDIDATE %2u: rtype=%d>%d, class=%d,%d|%d,%d\n",
			i,
			go2->rtype_user, go2->rtype_active,
			go2->prev.rclass, go2->prev.lclass,
			go2->next.rclass, go2->next.lclass );

	const kmp_enpt_entry_t *pt
		= (kmp_enpt_entry_t*)kmp->dlist[ph->sect_pt].list
		+ ph_list[i].pt_start;

	const double dx =  pt->position[0] - base[0];
	const double dy =  pt->position[1] - base[1];
	const double dz =  pt->position[2] - base[2];
	const double distance = dx*dx + dy*dy + dz*dz;

	if ( found < 0 || distance < found_distance )
	{
	    found = i;
	    found_distance = distance;
	}
    }

    noPRINT("SearchNearestDP - return %d\n",found);
    return found;
}

///////////////////////////////////////////////////////////////////////////////

static void TermGL
(
    kmp_group_list_t	* gl,		// group list
    ScanInfo_t		* si,		// not NULL: print error message
    kmp_t		* kmp,		// KMP data structure
    uint		sect_ph,	// PH section
    int			rotate_links	// >0: rotate links um x positions
)
{
    DASSERT(gl);
    DASSERT(gl->ph);
    DASSERT(kmp);
    DASSERT( sect_ph == KMP_CKPH || sect_ph == KMP_ENPH || sect_ph == KMP_ITPH );

    CheckLevelSI(si);
    ResetRouteLinksKMP(kmp);
    memset(gl->ph->gname,0,sizeof(gl->ph->gname));

    List_t *list = kmp->dlist + sect_ph;
    list->used = 0;

    uint i;
    kmp_group_t *grp = gl->group;
    for ( i = 0; i < gl->used; i++, grp++ )
    {
	kmp_enph_entry_t * entry = AppendList(list);
	entry->pt_start = grp->first_index;
	entry->pt_len = grp->last_index - grp->first_index + 1;

	memset(entry->next,-1,sizeof(entry->next));
	memset(entry->prev,-1,sizeof(entry->prev));
	if ( i < KMP_MAX_GROUP )
	    memcpy(gl->ph->gname+i,grp->name,sizeof(*gl->ph->gname));

	kmp_rtype_mode rtype = gl->ph->gopt[i].rtype_user;
	if ( rtype == KMP_RT_AUTO )
	    rtype = IsDispatchPointKMP(entry) ? KMP_RT_DISPATCH : KMP_RT_ROUTE;
	gl->ph->gopt[i].rtype_active = rtype;
    }

    rotate_links = rotate_links <= 0 ? 0 : rotate_links % KMP_MAX_PH_LINK;
    PRINT_IF(rotate_links,"ROTATE-LINKS %d\n",rotate_links);

    kmp_ph_t *ph = gl->ph;
    DASSERT(ph);

    for ( i = 0, grp = gl->group; i < gl->used; i++, grp++ )
    {
	//--- NEXT links

 // [[auto-prev]]
 #if 1
	const bool auto_prev
		=  gl->ph->ac_mode == KMP_AC_PREV
		|| gl->ph->ac_mode == KMP_AC_AUTO_PREV && !grp->have_prev_cmd;
  //if (sect_ph==KMP_ENPH) PRINT1("auto_prev=%d ; %s\n",auto_prev,grp->name);
 #else
	const bool auto_prev
		=  gl->ph->ac_mode == KMP_AC_PREV
		|| gl->ph->ac_mode == KMP_AC_AUTO_PREV;
 #endif

	uint l;
	for ( l = 0; l < KMP_MAX_PH_LINK; l++ )
	{
	    const uint l2 = grp->no_rotate
			? l
			: ( l + rotate_links ) % KMP_MAX_PH_LINK;
	    kmp_group_elem_t *elem =  grp->next+l2;
	    ccp lname = elem->name;
	    if (!*lname)
		continue;

	    noPRINT(">> '%s' links to '%s'\n",grp->name,lname);
	    kmp_group_t *grp2 = gl->group;
	    bool found = false;
	    uint j;
	    for ( j = 0; j < gl->used; j++, grp2++ )
	    {
		if (!strcmp(lname,grp2->name))
		{
		    found = true;

		    kmp_enph_entry_t * entry;
		    entry = (kmp_enph_entry_t*)(list->list) + i;
		    InsertLinkPH(entry->next,j,si,lname,true);
		    if (elem->oneway)
			InsertOnewayPH(gl->ph,i,j);

		    if ( auto_prev && !FindOnewayPH(gl->ph,i,j) )
		    {
			entry = (kmp_enph_entry_t*)(list->list) + j;
			InsertLinkPH(entry->prev,i,si,lname,false);
		    }
		    break;
		}
	    }

	    if ( !found && !si->no_warn )
		ERROR0(ERR_WARNING,
			"%s group %s: $GROUP name not found: %s",
			kmp_section_name[gl->ph->sect_pt].name1, grp->name, lname);
	}


	//--- PREV links

	if (!auto_prev)
	{
	    for ( l = 0; l < KMP_MAX_PH_LINK; l++ )
	    {
		kmp_group_elem_t *elem = grp->prev+l;
		ccp lname = elem->name;
		if (!*lname)
		    continue;

		noPRINT(">> '%s' links to '%s'\n",grp->name,lname);
		kmp_group_t *grp2 = gl->group;
		bool found = false;
		uint j;
		for ( j = 0; j < gl->used; j++, grp2++ )
		{
		    if (!strcmp(lname,grp2->name))
		    {
			found = true;

			kmp_enph_entry_t * entry;
			entry = (kmp_enph_entry_t*)(list->list) + i;
			InsertLinkPH(entry->prev,j,si,lname,true);
			if (elem->oneway)
			    InsertOnewayPH(gl->ph,i,j);
			break;
		    }
		}

		if ( !found && !si->no_warn )
		    ERROR0(ERR_WARNING,
			    "%s group %s: $PREV name not found: %s",
			    kmp_section_name[gl->ph->sect_pt].name1, grp->name, lname);
	    }
	}
    }


    //--- @AUTO-CONNECT == AC$DISPATCH

    if ( ph && ph->ac_mode == KMP_AC_DISPATCH
		&& ( sect_ph == KMP_ENPH || sect_ph == KMP_ITPH ))
    {

	//--- standard routes

	const kmp_linfo_t *li = SetupRouteLinksKMP(kmp,sect_ph,true);

	for ( i = 0, grp = gl->group; i < gl->used; i++, grp++ )
	{
	    const kmp_gopt2_t *go2 = ph->gopt + i;
	    if ( go2->rtype_active == KMP_RT_DISPATCH )
		continue;

	    PRINT(">>> STDROUTE %2u: %d %s, rtype=%d>%d, oneway=%d, class=%d,%d|%d,%d\n",
			i, li->gmode[i] & LINFOG_DISPATCH,
			PrintLinkInfoMode(li->summary[i],1,true),
			go2->rtype_user, go2->rtype_active,
			go2->ac_oneway,
			go2->prev.rclass, go2->prev.lclass,
			go2->next.rclass, go2->next.lclass );

	    kmp_enph_entry_t *entry = (kmp_enph_entry_t*)(list->list) + i;
	    const kmp_linfo_mode mode = li->summary[i];

	    if ( !(mode & LINFO_G_PREV) && go2->prev.lclass != KCLS_OFF )
	    {
		noPRINT("-> SEARCH PREV, CLASS %d\n",go2->prev.lclass);
		const kmp_enpt_entry_t *pt
			= (kmp_enpt_entry_t*)kmp->dlist[li->sect_pt].list
			+ entry->pt_start;
		const int dp = SearchNearestDP(gl,kmp,go2->prev.lclass,pt->position);
		if ( dp >= 0 )
		{
		    kmp_enph_entry_t *entry = (kmp_enph_entry_t*)(list->list) + i;
		    InsertLinkPH(entry->prev,dp,0,0,false);
		    if ( go2->ac_oneway & ONEWAY_PREV )
			InsertOnewayPH(ph,i,dp);
		}
	    }

	    if ( !(mode & LINFO_G_NEXT) && go2->next.lclass != KCLS_OFF )
	    {
		noPRINT("-> SEARCH NEXT, CLASS %d\n",go2->prev.lclass);
		const kmp_enpt_entry_t *pt
			= (kmp_enpt_entry_t*)kmp->dlist[li->sect_pt].list
			+ entry->pt_start + entry->pt_len - 1;
		const int dp = SearchNearestDP(gl,kmp,go2->prev.lclass,pt->position);
		if ( dp >= 0 )
		{
		    kmp_enph_entry_t *entry = (kmp_enph_entry_t*)(list->list) + i;
		    InsertLinkPH(entry->next,dp,0,0,false);
		    if ( go2->ac_oneway & ONEWAY_NEXT )
			InsertOnewayPH(ph,i,dp);
		}
	    }
	}


	//--- dispatch points

	li = SetupRouteLinksKMP(kmp,sect_ph,true);

	for ( i = 0, grp = gl->group; i < gl->used; i++, grp++ )
	{
	    const kmp_gopt2_t *go2 = ph->gopt + i;
	    if ( go2->rtype_active != KMP_RT_DISPATCH || go2->next.lclass == KCLS_OFF )
		continue;

	    PRINT(">>> DISPATCH %2u: %d %s, rtype=%d>%d, oneway=%d, class=%d,%d|%d,%d\n",
			i, li->gmode[i] & LINFOG_DISPATCH,
			PrintLinkInfoMode(li->summary[i],1,true),
			go2->rtype_user, go2->rtype_active,
			go2->ac_oneway,
			go2->prev.rclass, go2->prev.lclass,
			go2->next.rclass, go2->next.lclass );

	    const kmp_linfo_mode mode = li->summary[i];
	    if ( mode & (LINFO_G_PREV|LINFO_G_NEXT) )
		continue; // mode already set

	    kmp_enph_entry_t *entry = (kmp_enph_entry_t*)(list->list) + i;
	    const kmp_linfo_mode *l = li->list[i];

	    uint j;
	    for ( j = 0; j < li->used; j++, l++ )
		if ( *l & (LINFO_G_BY_PREV|LINFO_G_BY_NEXT) && !FindOnewayPH(ph,j,i) )
		    InsertLinkPH(entry->next,j,0,0,false);
	}
    }


    //--- fix empty link lists

    if ( ph->ac_flags & KMP_ACF_FIX )
    {
	const int fix_prev = ph->ac_flags & KMP_ACF_FIX_PREV;
	const int fix_next = ph->ac_flags & KMP_ACF_FIX_NEXT;
	kmp_enph_entry_t *entry = (kmp_enph_entry_t*)list->list;

	for ( i = 0; i < gl->used; i++, entry++ )
	{
	    if ( fix_prev && entry->prev[0] == M1(entry->prev[0]) )
		entry->prev[0] = i;
	    if ( fix_next && entry->next[0] == M1(entry->next[0]) )
		entry->next[0] = i;
	}
    }

    ResetRouteLinksKMP(kmp); // invalidate link-info because modified
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			more kmp functions		///////////////
///////////////////////////////////////////////////////////////////////////////

static int get_next
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    int			user_id,	// user defined id
    int			* cur_idx	// not NULL: store current index here
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);
    DASSERT( user_id == KMP_ENPT || user_id == KMP_ITPT );

    // [[2do]] use GetNextPointKMP()

    if (cur_idx)
	*cur_idx = -1;

    int idx = GetIntV(param);
    res->i = 0;
    res->mode = VAR_UNSET;

    if ( si && si->kmp )
    {
	const kmp_t *kmp = si->kmp;

	const List_t *ptlist, *phlist;
	if ( user_id == KMP_ENPT )
	{
	    ptlist = kmp->dlist + KMP_ENPT;
	    phlist = kmp->dlist + KMP_ENPH;
	}
	else
	{
	    ptlist = kmp->dlist + KMP_ITPT;
	    phlist = kmp->dlist + KMP_ITPH;
	}

	if ( idx < 0 )
	    idx += ptlist->used;

	if ( idx >= 0 && idx < ptlist->used )
	{
	    if (cur_idx)
		*cur_idx = idx;

	    const kmp_enph_entry_t *ph = (kmp_enph_entry_t*)phlist->list;
	    uint n = phlist->used;
	    while ( idx >= ph->pt_start + ph->pt_len && n-- > 0 )
		ph++;

	    noPRINT(">> %u %zu[%u+%u]\n", idx,
			ph-(kmp_enph_entry_t*)phlist->list,ph->pt_start,ph->pt_len);
	    if ( idx == ph->pt_start + ph->pt_len - 1 )
	    {
		idx = -1; // invalidate
		uint ph_idx = ~0;

		if ( n_param > 1 )
		{
		    uint idx2 = GetIntV(param+1);
		    if ( idx2 < KMP_MAX_PH_LINK )
			ph_idx = ph->next[idx2];
		}
		else
		{
		    uint idx2;
		    for ( idx2 = 0; idx2 < KMP_MAX_PH_LINK; idx2++ )
		    {
			if (!IS_M1(ph->next[idx2]))
			{
			    ph_idx = ph->next[idx2];
			    break;
			}
		    }
		}

		if ( ph_idx < phlist->used )
		{
		    ph = (kmp_enph_entry_t*)phlist->list + ph_idx;
		    idx = ph->pt_start;
		}
	    }
	    else
		idx++;

	    if ( idx >= 0 && idx < ptlist->used )
		return idx;
	}
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_get_next
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);
    DASSERT(fpar);
    DASSERT( fpar->user_id == KMP_ENPT || fpar->user_id == KMP_ITPT );

    int next = get_next(res,param,n_param,si,fpar->user_id,0);
    if ( next >= 0 )
    {
	res->i = next;
	res->mode = VAR_INT;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_get_hdir
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);
    DASSERT(fpar);
    DASSERT( fpar->user_id == KMP_ENPT || fpar->user_id == KMP_ITPT );

    int current;
    int next = get_next(res,param,n_param,si,fpar->user_id,&current);
    if ( next >= 0 )
    {
	DASSERT(si);
	const kmp_t *kmp = si->kmp;
	DASSERT(kmp);

	const kmp_enpt_entry_t *pt = (kmp_enpt_entry_t*)kmp->dlist[fpar->user_id].list;
	res->d = atan2( pt[next].position[0] - pt[current].position[0],
			pt[next].position[2] - pt[current].position[2] ) * (180/M_PI);
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isKMP
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);

    uint result = 0;
    if ( si && si->kmp )
    {
	result = 1;

	if (si->kmp->fname)
	{
	    ccp fname = strrchr(si->kmp->fname,'/');
	    if (fname)
		fname++;
	    else
		fname = si->kmp->fname;

	    if ( !strcasecmp(fname,"course.kmp")
		|| !strcasecmp(fname,"course.txt")
		|| !strcasecmp(fname,"course.txt.kmp") )
	    {
		result = 2;
	    }
	}
    }

    res->i    = result;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static const struct FuncTable_t kmp_func_tab[] =
{
    { 1, 2, "ENPT$NEXT", F_get_next, KMP_ENPT,
	"int", "enpt$next(enpt_index[,next_index])",
	"Returns the index of the next enemy point."
	" The 'next_index' is only used, if multiple links are available." },

    { 1, 2, "ITPT$NEXT", F_get_next, KMP_ITPT,
	"int", "itpt$next(itpt_index[,next_index])",
	"Returns the index of the next item point."
	" The 'next_index' is only used, if multiple links are available." },

    { 1, 2, "ENPT$HDIR", F_get_hdir, KMP_ENPT,
	"float", "enpt$hDir(enpt_index[,next_index])",
	"Calculate the horizontal direction in degree of an enemy point."
	" The 'next_index' is only used, if multiple links are available." },

    { 1, 2, "ITPT$HDIR", F_get_hdir, KMP_ITPT,
	"float", "itpt$hDir(itpt_index[,next_index])",
	"Calculate the horizontal direction in degree of an item point."
	" The 'next_index' is only used, if multiple links are available." },

    { 0, 0, "ISKMP", F_isKMP, 0,	// replace only function call
	"int", "isKMP()", 0 },		// but not info

    {0,0,0,0,0,0,0,0}
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			setup kmp parser		///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsKMP()
{
    static VarMap_t vm = { .force_case = LOUP_UPPER };
    if (!vm.used)
    {
	//--- setup parser function get()

	uint i;
	const kmp_func_t *fp = kmp_func_list;
	for ( i = 0; i < KMP_FUNC_LIST_N; i++, fp++ )
	{
	  char syntax[80];
	  if ( fp->get_info)
	  {
	    DefineParserFunc(fp->name,fp->n_param,fp->n_param,FF_KMP,F_get,i);

	    ccp retval = fp->get_mode == GTMD_DBL
				? "float"
				: fp->get_mode > GTMD_DBL
					? "vector"
					: "int";
	    char *dest = syntax;
	    ccp src = fp->name;
	    while (*src)
		*dest++ = tolower((int)*src++);
	    *dest++ = '(';
	    if ( fp->n_param > 0 )
	    {
		src = fp->sect == KMP_POTI ? "route" : kmp_section_name[fp->sect].name1;
		while (*src)
		    *dest++ = tolower((int)*src++);

		switch(fp->n_param)
		{
		    case 1:
			src = "_idx";
			break;

		    case 2:
			src = fp->sect == KMP_POTI && fp->off >= POTI_PT_OFF
				    ? "_idx,pt_idx"
				    : "_idx,val_idx";
			break;

		    case 3:
			src = "_idx,pt_idx,val_idx";
			break;

		};
		while (*src)
		    *dest++ = *src++;
	    }
	    *dest++ = ')';
	    *dest = 0;
	    DASSERT(dest < syntax + sizeof(syntax) );

	    DefineParserFuncInfo(retval,syntax,fp->get_info);
	  }

    #ifdef TEST // [[2do]]
	  if (fp->set_info)
	  {
	    char namebuf[40];
	    StringCat2S(namebuf,sizeof(namebuf),"SET_",fp->name);
	    DefineParserFunc(namebuf,fp->n_param+1,fp->n_param+1,FF_KMP,F_set,i);

	    char *dest = syntax;
	    ccp src = namebuf;
	    while (*src)
		*dest++ = tolower((int)*src++);
	    *dest++ = '(';
	    if ( fp->n_param > 0 )
	    {
		src = fp->sect == KMP_POTI ? "route" : kmp_section_name[fp->sect].name1;
		while (*src)
		    *dest++ = tolower((int)*src++);

		switch(fp->n_param)
		{
		    case 1:
			src = "_idx";
			break;

		    case 2:
			src = fp->sect == KMP_POTI && fp->off >= POTI_PT_OFF
				    ? "_idx,pt_idx"
				    : "_idx,val_idx";
			break;

		    case 3:
			src = "_idx,pt_idx,val_idx";
			break;

		};
		while (*src)
		    *dest++ = *src++;
	    }
	    DASSERT(dest < syntax + sizeof(syntax) );
	    StringCopyE(dest,syntax+sizeof(syntax),",value)");
	    DefineParserFuncInfo("int",syntax,fp->set_info);
	  }
    #endif
	}


	//--- setup additional functions

	DefineParserFuncTab(kmp_func_tab,FF_KMP);
	SetupReferenceKCL(&vm);


	//--- setup object names

	DefineObjectNameVars(&vm);


	//--- setup KMP section names

	char varname[50];
	const KeywordTab_t *key;
	for ( key = kmp_section_name; key->name2; key++ )
	{
	    snprintf(varname,sizeof(varname),"SECT$%s",key->name1);
	    DefineIntVar(&vm,varname,key->id+1);
	}


	//--- setup integer variables

	struct inttab_t { ccp name; int val; };
	static const struct inttab_t inttab[] =
	{
	    { "MODE$SLOT",		0 },
	    { "MODE$D",			0 },

	    { "MODE$TEST",		0 },
	    { "MODE$OCAM",		0 },
	    { "MODE$LAPS",		3 },
	    { "MODE$SHOWRT",		0 },
	    { "MODE$SHOWCK",		0 },
	    { "MODE$AUTOMODE",		0 },

	    { "AC$OFF",			KMP_AC_OFF },
	    { "AC$PREV",		KMP_AC_PREV },
	    { "AC$AUTO_PREV",		KMP_AC_AUTO_PREV },
	    { "AC$DISPATCH",		KMP_AC_DISPATCH },
	    { "ACF$FIX_PREV",		KMP_ACF_FIX_PREV },
	    { "ACF$FIX_NEXT",		KMP_ACF_FIX_NEXT },
	    { "ACF$FIX",		KMP_ACF_FIX },
	    { "ACF$PR_PREV",		KMP_ACF_PR_PREV },

	    { "AM$OFF",			KMP_AM_OFF },
	    { "AM$NORM",		KMP_AM_NORM },
	    { "AM$RENUMBER",		KMP_AM_RENUMBER },
	    { "AM$1LAP",		KMP_AM_1_LAP },
	    { "AM$SHORT",		KMP_AM_SHORT },
	    { "AM$UNLIMIT",		KMP_AM_UNLIMIT },

	    { "SORT$OFF",		KSORT_OFF },
	    { "SORT$GROUPING",		KSORT_GROUP },
	    { "SORT$ANGLE",		KSORT_ANGLE },
	    { "SORT$XYZ",		KSORT_XYZ },
	    { "SORT$TINY",		KSORT_TINY },

	    { "DOB$OFF",		KMP_DOB_OFF },
	    { "DOB$BEGIN",		KMP_DOB_BEGIN },
	    { "DOB$END",		KMP_DOB_END },
	    { "DOB$BEFORE",		KMP_DOB_BEFORE },
	    { "DOB$BEHIND",		KMP_DOB_BEHIND },

	    { "EN$BATTLE",		ENGM_BATTLE },
	    { "EN$50",			ENGM_50 },
	    { "EN$100",			ENGM_100 },
	    { "EN$150",			ENGM_150 },
	    { "EN$200",			ENGM_200 },
	    { "EN$150M",		ENGM_150M },
	    { "EN$200M",		ENGM_200M },
	     { "EN$150X",		ENGM_150M | ENGM_150 },
	     { "EN$200X",		ENGM_200M | ENGM_200 },
	     { "EN$MIRROR",		ENGM_150M | ENGM_200M },
	     { "EN$ALL",		ENGM__ALL },

	    { "F$FALL",			KPFL_FALL },
	    { "F$SNAP",			KPFL_SNAP },
	    { "F$JGPT",			KPFL_JGPT },

	    { "ITEM$RANDOM",		0x00 },
	    { "ITEM$BANANA",		0x01 },
	    { "ITEM$MUSHROOM",		0x02 },
	    { "ITEM$3MUSHROOMS",	0x03 },
	    { "ITEM$STAR",		0x04 },
	    { "ITEM$3GREEN",		0x05 },
	    { "ITEM$BANANA_MUSHROOM",	0x06 },
	    { "ITEM$GREEN",		0x07 },
	    { "ITEM$RED",		0x08 },
	    { "ITEM$MEGA",		0x0a },
	    { "ITEM$CLOUD",		0x0b },
	    { "ITEM$STAR_MUSHROOM",	0x0c },
	    { "ITEM$MUSHROOM_GREEN",	0x0d },
	    { "ITEM$3BANANAS_GREEN",	0x24 },

	    { "PF$OFFLINE1",		0x01 },
	    { "PF$OFFLINE2",		0x02 },
	    { "PF$OFFLINE3",		0x04 },
	    { "PF$OFFLINE",		0x07 },
	    { "PF$ONLINE1",		0x08 },
	    { "PF$ONLINE2",		0x10 },
	    { "PF$ONLINE3",		0x20 },
	    { "PF$ONLINE",		0x38 },
	    { "PF$ALL",			0x3f },

	    {0,0}
	};

	const struct inttab_t * ip;
	for ( ip = inttab; ip->name; ip++ )
	    DefineIntVar(&vm,ip->name,ip->val);


	//--- extended p-flags

	char name[20];
	const gobj_cond_ref_t *rc;
	for ( rc = cond_ref_tab; rc->ref_id; rc++ )
	{
	    ccp prefix = rc->option & 4 ? "IFDEV$" : "IF$";
	    StringCat2S(name,sizeof(name),prefix,rc->name);
	    DefineIntVar(&vm,name,rc->ref_id);

	    if (rc->alias[0])
	    {
		StringCat2S(name,sizeof(name),prefix,rc->alias);
		DefineIntVar(&vm,name,rc->ref_id);
	    }

	    if ( rc->cond_mask && !(rc->option & 4) )
	    {
		StringCat2S(name,sizeof(name),"C$",rc->name);
		DefineIntVar(&vm,name,rc->cond_mask);

		if (rc->alias[0])
		{
		    StringCat2S(name,sizeof(name),"C$",rc->alias);
		    DefineIntVar(&vm,name,rc->cond_mask);
		}
	    }
	}


	//--- setup basic parser variables

	DefineParserVars(&vm);
    }

    return &vm;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl fall helpers		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[kcl_fall_t]]

typedef struct kcl_fall_t
{
    //--- input

    kcl_t	*fall_kcl;		// KCL to use, never NULL if 'kf->valid'

    int		width;			// input for "KCL-FALL-WIDTH"
    Var_t	pre_add;		// input for "KCL-FALL-PRE-ADD"
    Var_t	post_add;		// input for "KCL-FALL-POST-ADD"
    u32		types;			// input for "KCL-FALL-TYPES"

    //---- status

    bool	valid;			// true: calculation can be done

} kcl_fall_t;

///////////////////////////////////////////////////////////////////////////////

#define KCL_FALL_VARS(k,v) \
	{ "KCL-FALL-ADD-ROAD",	SPM_BOOL, &k->enable_fall_kcl }, \
	{ "KCL-FALL-WIDTH",	SPM_INT,  &v.width }, \
	{ "KCL-FALL-PRE-ADD",	SPM_VAR,  &v.pre_add }, \
	{ "KCL-FALL-POST-ADD",	SPM_VAR,  &v.post_add }, \
	{ "KCL-FALL-TYPES",	SPM_U32,  &v.types },

///////////////////////////////////////////////////////////////////////////////

void ResetKclFall ( kcl_fall_t *kf )
{
    DASSERT(kf);
    memset(kf,0,sizeof(*kf));
    kf->types = M1(kf->types);
}

///////////////////////////////////////////////////////////////////////////////

bool CheckKclFall ( kcl_fall_t *kf )
{
    DASSERT(kf);

    kf->fall_kcl = 0;
    kf->valid = kf->width > 0 && kf->types;
    if (kf->valid)
    {
	ToVectorYV(&kf->pre_add);
	ToVectorYV(&kf->post_add);

	kf->fall_kcl = GetReferenceKCL();
	if (!kf->fall_kcl)
	    kf->valid = false;
    }
    return kf->valid;
}

///////////////////////////////////////////////////////////////////////////////

void CalcKclFall ( kcl_fall_t *kf, float *vector )
{
    DASSERT(kf);
    if (kf->valid)
    {
	DASSERT(kcl_ref);
	DASSERT(kf->fall_kcl);

	double3 pt;
	pt.x = vector[0] + kf->pre_add.x;
	pt.y = vector[1] + kf->pre_add.y;
	pt.z = vector[2] + kf->pre_add.z;
	double temp = FallKCL(kf->fall_kcl,0,pt,kf->width,0,kf->types,0);
	if ( temp >= 0.0 &&  temp < pt.y )
	{
	    vector[0] = kf->post_add.x + pt.x;
	    vector[1] = kf->post_add.y + temp;
	    vector[2] = kf->post_add.z + pt.z;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			route object helpers		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[rtobj_info_t]]

typedef struct rtobj_info_t
{
    //--- input

    int			route_object;		// input for "ROUTE-OBJECT"
    Var_t		route_shift;		// input for "ROUTE-OBJECT-SHIFT"
    Var_t		route_scale;		// input for "ROUTE-OBJECT-SCALE"
    Var_t		route_rotate;		// input for "ROUTE-OBJECT-ROTATE"
    int			show_route;		// input for "SHOW-ROUTE"
    int			route_itembox;		// input for "ROUTE-ITEMBOX"

    //--- data

    kmp_rtobj_t		*def_obj;		// default object
    kmp_rtobj_t		rtobj;			// resulting data
    bool		valid;			// true: rtobj is set

} rtobj_info_t;

///////////////////////////////////////////////////////////////////////////////

#define ROUTE_OBJECT_VARS(v) \
	{ "ROUTE-OBJECT",	SPM_INT, &v.route_object }, \
	{ "ROUTE-OBJECT-SHIFT",	SPM_VAR, &v.route_shift }, \
	{ "ROUTE-OBJECT-SCALE",	SPM_VAR, &v.route_scale }, \
	{ "ROUTE-OBJECT-ROTATE",SPM_VAR, &v.route_rotate }, \
	{ "SHOW-ROUTE",		SPM_INT, &v.show_route }, \
	{ "ROUTE-ITEMBOX",	SPM_INT, &v.route_itembox },

///////////////////////////////////////////////////////////////////////////////

void ResetRouteInfo ( rtobj_info_t *ri, kmp_rtobj_t *def_obj )
{
    DASSERT(ri);
    DASSERT(def_obj);

    memset(ri,0,sizeof(*ri));
    ri->def_obj		= def_obj;
    memcpy(&ri->rtobj,ri->def_obj,sizeof(ri->rtobj));
    ri->route_object	= N_KMP_GOBJ;
}

///////////////////////////////////////////////////////////////////////////////

bool CheckRouteInfo ( rtobj_info_t *ri )
{
    DASSERT(ri);
    DASSERT(ri->def_obj);
    noPRINT_IF( ri->route_object != N_KMP_GOBJ,
		">> CheckRouteInfo() obj=%d, shrt=%d\n",
		ri->route_object, ri->show_route );

    if (!ri->route_object)
	ri->rtobj.obj = 0;
    else if ( ri->route_object < 0 )
    {
	memcpy(&ri->rtobj,ri->def_obj,sizeof(ri->rtobj));
	ri->show_route = 1;
    }
    else if ( ri->route_object < N_KMP_GOBJ )
    {
	ri->rtobj.obj = ri->route_object;
	ri->show_route = 1;
    }
    ri->route_object = N_KMP_GOBJ;

    if ( ri->route_shift.mode != VAR_UNSET )
    {
	ri->rtobj.shift = GetVectorFV(&ri->route_shift);
	ri->route_shift.mode = VAR_UNSET;
    }

    if ( ri->route_scale.mode != VAR_UNSET )
    {
	ri->rtobj.scale = GetVectorFV(&ri->route_scale);
	ri->route_scale.mode = VAR_UNSET;
    }

    if ( ri->route_rotate.mode != VAR_UNSET )
    {
	ri->rtobj.rotate = GetVectorFV(&ri->route_rotate);
	ri->route_rotate.mode = VAR_UNSET;
    }

    if ( ri->show_route > 0 && !ri->rtobj.obj )
	ri->rtobj.obj = ri->def_obj->obj;

    if ( ri->route_itembox > 0 && !ri->valid )
    {
	ri->show_route = 1;
	ri->rtobj.obj = GOBJ_ITEMBOX;
    }

    ri->valid = ri->show_route > 0 && ri->rtobj.obj;
    return ri->valid;
}

///////////////////////////////////////////////////////////////////////////////

bool AssignRouteInfo ( rtobj_info_t *ri, kmp_rtobj_t *dest )
{
    DASSERT(ri);
    DASSERT(dest);
    if (ri->valid)
	memcpy(dest,&ri->rtobj,sizeof(*dest));
    else
	memset(dest,0,sizeof(*dest));

    return ri->valid;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			transform pos helpers		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[tform_info_t]]

typedef struct tform_info_t
{
    //--- input

    int			reset;		// input for "TFORM-RESET"
    Var_t		scale;		// input for "TFORM-SCALE"
    Var_t		shift;		// input for "TFORM-SHIFT"
    Var_t		rotate;		// input for "TFORM-ROTATE"
    Var_t		translate;	// input for "TFORM-TRANSLATE"


    //--- data

    bool		active;		// transformation is active
    MatrixD_t		matrix;		// transformation matrix;

} tform_info_t;

///////////////////////////////////////////////////////////////////////////////

#define TFORM_VARS(v) \
	{ "TFORM-RESET",	SPM_INC, &v.reset }, \
	{ "TFORM-SCALE",	SPM_VAR, &v.scale }, \
	{ "TFORM-SHIFT",	SPM_VAR, &v.shift }, \
	{ "TFORM-ROTATE",	SPM_VAR, &v.rotate }, \
	{ "TFORM-TRANSLATE",	SPM_VAR, &v.translate }, \

///////////////////////////////////////////////////////////////////////////////

void ResetTformInfo ( tform_info_t *ti )
{
    DASSERT(ti);

    memset(ti,0,sizeof(*ti));
    InitializeMatrixD(&ti->matrix);
}

///////////////////////////////////////////////////////////////////////////////

bool CheckTformInfo ( tform_info_t *ti )
{
    DASSERT(ti);

    if (ti->reset)
    {
	PRINT("#TFORM: RESET(%d)\n",ti->reset);
	ti->reset = 0;
	ti->active = false;
	InitializeMatrixD(&ti->matrix);
    }

    if ( ti->scale.mode != VAR_UNSET )
    {
	double3 d = GetVectorV(&ti->scale);
	PRINT("#TFORM: SCALE %g %g %g\n",d.x,d.y,d.z);
	SetScaleMatrixD(&ti->matrix,&d,0);
	ti->scale.mode = VAR_UNSET;
	ti->active = true;
    }

    if ( ti->shift.mode != VAR_UNSET )
    {
	double3 d = GetVectorV(&ti->shift);
	PRINT("#TFORM: SHIFT %g %g %g\n",d.x,d.y,d.z);
	SetShiftMatrixD(&ti->matrix,&d);
	ti->shift.mode = VAR_UNSET;
	ti->active = true;
    }

    if ( ti->rotate.mode != VAR_UNSET )
    {
	double3 d = GetVectorV(&ti->rotate);
	PRINT("#TFORM: ROTATE %g %g %g\n",d.x,d.y,d.z);
	SetRotateMatrixD(&ti->matrix,0,d.x,0.0,0);
	SetRotateMatrixD(&ti->matrix,1,d.y,0.0,0);
	SetRotateMatrixD(&ti->matrix,2,d.z,0.0,0);
	ti->rotate.mode = VAR_UNSET;
	ti->active = true;
    }

    if ( ti->translate.mode != VAR_UNSET )
    {
	double3 d = GetVectorV(&ti->translate);
	PRINT("#TFORM: TRANSLATE %g %g %g\n",d.x,d.y,d.z);
	SetTranslateMatrixD(&ti->matrix,&d);
	ti->translate.mode = VAR_UNSET;
	ti->active = true;
    }

    return ti->active;
}

///////////////////////////////////////////////////////////////////////////////

void TransformPos ( tform_info_t *ti, float32 *pos )
{
    DASSERT(ti);
    DASSERT(pos);

    if (ti->active)
	*(float3*)pos = TransformF3MatrixD(&ti->matrix,(float3*)pos);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		   ScanTextKMP() sub functions		///////////////
///////////////////////////////////////////////////////////////////////////////

static kmp_pt_flag_t ScanPtFlags ( ScanInfo_t *si )
{
    DASSERT(si);

    if ( NextCharSI(si,false) == '(' )
    {
	si->cur_file->ptr++;
	DEFINE_VAR(val);
	CheckWarnSI(si,')',ScanExprSI(si,&val));
	return GetIntV(&val);
    }

    char fbuf[20];
    ScanNameSI(si,fbuf,sizeof(fbuf),true,true,0);

    kmp_pt_flag_t flags = 0;
    if ( strchr(fbuf,'F') ) flags |= KPFL_FALL;
    if ( strchr(fbuf,'S') ) flags |= KPFL_SNAP;
    if ( strchr(fbuf,'J') ) flags |= KPFL_JGPT;

 #if 0
    if ( si->cur_file->revision < xxxx && !( flags & KPFL_UPDATE_Rxxxx ) )
	flags |= KPFL_UPDATE_Rxxxx;
 #endif
    return flags;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_PT_PH
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si,		// valid data
    uint		sect_pt		// KMP PT section
)
{
    DASSERT(kmp);
    DASSERT(si);
    DASSERT( sect_pt == KMP_ENPT || sect_pt == KMP_ITPT );

    uint sect_ph;
    kmp_ph_t *ph;
    kmp_flag_t *kf;

    if ( sect_pt == KMP_ENPT )
    {
	sect_ph		= KMP_ENPH;
	ph		= &kmp->enph;
	kf		= &kmp->enpt_flag;
    }
    else
    {
	sect_ph		= KMP_ITPH;
	ph		= &kmp->itph;
	kf		= &kmp->itpt_flag;
    }

    ph->default_class = KCLS_DEFAULT;

    ccp pt_name = kmp_section_name[sect_pt].name1;
    ccp ph_name = kmp_section_name[sect_ph].name1;
    TRACE("ScanText_PT_PH(%s,%s)\n",pt_name,ph_name);

    char pt_hd_name[20], ph_hd_name[20];
    snprintf( pt_hd_name,   sizeof(pt_hd_name),   "%s-HEAD-VALUE",      pt_name );
    snprintf( ph_hd_name,   sizeof(ph_hd_name),	  "%s-HEAD-VALUE",      ph_name );

    rtobj_info_t rtinfo;
    ResetRouteInfo(&rtinfo,&kmp->rtobj);

    tform_info_t tform_pos;
    ResetTformInfo(&tform_pos);

    kcl_fall_t kfall;
    ResetKclFall(&kfall);

    int auto_group_name = 0, rotate_group_links = 0;
    int show_options = 0, export_flags = 0;
    double auto_fill = 0.0;
    u8 auto_connect = ph->ac_mode | ph->ac_flags;
    u16 mask1, mask2;
    mask1 = mask2 = 0xffff;

    ScanParam_t ptab[] =
    {
	{ pt_hd_name,		SPM_U16,    kmp->value+sect_pt },
	{ ph_hd_name,		SPM_U16,    kmp->value+sect_ph },
	{ "AUTO-GROUP-NAME",	SPM_INT,    &auto_group_name },
	{ "ROTATE-GROUP-LINKS",	SPM_INT,    &rotate_group_links },
	{ "EXPORT-FLAGS",	SPM_INT,    &export_flags },
	{ "AUTO-FILL",		SPM_DOUBLE, &auto_fill },
	{ "SHOW-OPTIONS",	SPM_INT,    &show_options },
	{ "AUTO-CONNECT",	SPM_U8,	    &auto_connect },
	{ "MASK-1",		SPM_U16,    &mask1 },
	{ "MASK-2",		SPM_U16,    &mask2 },
	ROUTE_OBJECT_VARS(rtinfo)
	TFORM_VARS(tform_pos)
	KCL_FALL_VARS(kmp,kfall)
	{0}
    };

    kmp_group_list_t gl;
    InitializeGL(&gl,*pt_name,kmp,ph);
    List_t *ptlist = kmp->dlist + sect_pt;
    ptlist->used = 0;

    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    CheckRouteInfo(&rtinfo);
	    CheckTformInfo(&tform_pos);
	    CheckKclFall(&kfall);
	    continue;
	}

	if ( ch == '$' )
	{
	    AddGroupGL(&gl,si,ph_name[0],ptlist->used,3);
	    continue;
	}

	u16 idx;
	if (ScanU16SI(si,&idx,1,0))
	{
	    err = ERR_WARNING;
	    break;
	}

	kmp_enpt_entry_t temp;
	ScanFloatV3SI(si,temp.position,1);
	ScanFloatSI(si,&temp.scale,1);
	if ( si->cur_file->szs_modifier > 0 )
	    ScanBE32SI_swap(si,(u32*)temp.prop,1,1);
	else
	    ScanU16SI(si,temp.prop,2,0);
	const u8 flags = NextCharSI(si,false) ? ScanPtFlags(si) : kf->mask;
	CheckEolSI(si);

	const kmp_enpt_entry_t * prev = GetListElem(ptlist,-1,&temp);
	if (!ptlist->used) // no previous point available
	    gl.line.n = 1;
	else if ( auto_fill >= 1.0
		&& gl.line.n == 1
		&& gl.used
		&& ptlist->used != gl.group[gl.used-1].first_index )
	{
	    double3 dist3;
	    dist3.x = temp.position[0] - prev->position[0];
	    dist3.y = temp.position[1] - prev->position[1];
	    dist3.z = temp.position[2] - prev->position[2];
	    const double dist = Length3(dist3);
	    if ( dist > auto_fill )
	    {
		gl.line.n = (int)trunc(dist/auto_fill) + 1;
		PRINT("DIST=%8.2f, FILL=%8.2f => N=%2u\n",dist,auto_fill,gl.line.n);
	    }
	}

	uint p;
	for ( p = 1; p <= gl.line.n; p++ )
	{
	    AddPointGL(&gl,ptlist->used);
	    if ( ptlist->used < KMP_MAX_POINT )
		kf->flags[ptlist->used] = flags;

	    kmp_enpt_entry_t * entry = AppendList(ptlist);
	    memcpy(entry,&temp,sizeof(*entry));
	    CalcLine(&gl.line,entry->position,prev->position,temp.position,4,p);
	    TransformPos(&tform_pos,entry->position);
	    if ( flags & KPFL_FALL )
		CalcKclFall(&kfall,entry->position);
	}
	InitializeLine(&gl.line);
    }

    TermGL(&gl,si,kmp,sect_ph,rotate_group_links);

    if (gl.have_options)
	show_options = true;

    #if HAVE_PRINT0
    {
	uint i;
	for ( i = 0; i < gl.used; i++ )
	    PRINT("GROUP %3u -> %s\n",i,ph->gname[i]);
    }
    #endif

    if (opt_export_flags)
	export_flags = opt_export_flags > 0;

    kf->export		= export_flags;
    ph->ac_mode		= auto_connect & KMP_AC_MASK;
    ph->ac_flags	= auto_connect & KMP_ACF_MASK;
    ph->mask[0]		= mask1;
    ph->mask[1]		= mask2;
    if ( opt_route_options == OFFON_AUTO )
	ph->show_options = show_options;

    if (auto_group_name)
	RenameGroupKMP( kmp, sect_ph, ph->gname, ph->ac_mode == KMP_AC_DISPATCH);

    if ( sect_pt == KMP_ENPT )
	AssignRouteInfo(&rtinfo,&kmp->rtobj_enpt);
    else
	AssignRouteInfo(&rtinfo,&kmp->rtobj_itpt);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextKTPT
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextKTPT()\n");

    tform_info_t tform_pos;
    ResetTformInfo(&tform_pos);

    kcl_fall_t kfall;
    ResetKclFall(&kfall);

    ScanParam_t ptab[] =
    {
	{ "KTPT-HEAD-VALUE",	SPM_U16,    kmp->value+KMP_KTPT },
	TFORM_VARS(tform_pos)
	KCL_FALL_VARS(kmp,kfall)
	{0}
    };

    List_t *dlist = kmp->dlist + KMP_KTPT;
    dlist->used = 0;

    #if HAVE_PRINT0
	si->debug++;
    #endif

    for(;;)
    {
     #if HAVE_PRINT0
	{
	    ccp eol = FindNextLineFeedSI(si,false);
	    PRINT("##LINE %4u: %.*s\n",si->line,(int)(eol-si->ptr),si->ptr);
	}
     #endif

	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

     #if HAVE_PRINT0
	{
	    ccp eol = FindNextLineFeedSI(si,false);
	    PRINT("##LINE %4u: %.*s\n",si->line,(int)(eol-si->ptr),si->ptr);
	}
     #endif

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    CheckTformInfo(&tform_pos);
	    CheckKclFall(&kfall);
	    continue;
	}

	u16 idx;
	if (ScanU16SI(si,&idx,1,0))
	    break;

	kmp_ktpt_entry_t * entry = AppendList(dlist);
	ScanFloatV3SI(si,entry->position,2);
	ScanU16SI(si,(u16*)&entry->player_index,1,0);
	ScanU16SI(si,(u16*)&entry->unknown,1,si->cur_file->szs_modifier);

	TransformPos(&tform_pos,entry->position);
	CalcKclFall(&kfall,entry->position);
	CheckEolSI(si);
    }
    CheckLevelSI(si);

 #if HAVE_PRINT0
    si->debug--;
 #endif

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextCKPT
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextCKPT()\n");

    int auto_mode = 0, auto_next = 0, auto_respawn = 0;
    int auto_group_name = 0, rotate_group_links = 0;
    memset(kmp->ckpt_auto_respawn,0,sizeof(kmp->ckpt_auto_respawn));

    ScanParam_t ptab[] =
    {
	{ "AUTO-MODE",			SPM_INT,    &auto_mode },
	{ "AUTO-NEXT",			SPM_INT,    &auto_next },
	{ "AUTO-GROUP-NAME",		SPM_INT,    &auto_group_name },
	{ "AUTO-RESPAWN",		SPM_INT,    &auto_respawn },
	{ "ROTATE-GROUP-LINKS",		SPM_INT,    &rotate_group_links },
	{ "ROUTE-OBJECT-MODE",		SPM_INT,    &kmp->ckpt_object_mode },
	{ "ROUTE-OBJECT1",		SPM_INT,    &kmp->ckpt_object1 },
	{ "ROUTE-OBJECT2",		SPM_INT,    &kmp->ckpt_object2 },
	{ "ROUTE-OBJECT1-SCALE",	SPM_VAR,    &kmp->ckpt_object1_scale },
	{ "ROUTE-OBJECT2-SCALE",	SPM_VAR,    &kmp->ckpt_object2_scale },
	{ "ROUTE-OBJECT1-BASE",		SPM_FLOAT,  &kmp->ckpt_object1_base },
	{ "ROUTE-OBJECT2-BASE",		SPM_FLOAT,  &kmp->ckpt_object2_base },
	{ "CKPT-HEAD-VALUE",		SPM_U16,    kmp->value+KMP_CKPT },
	{ "CKPH-HEAD-VALUE",		SPM_U16,    kmp->value+KMP_CKPH },
	{0}
    };

    kmp_group_list_t gl;
    InitializeGL(&gl,0,kmp,&kmp->ckph);
    List_t *ptlist = kmp->dlist + KMP_CKPT;
    ptlist->used = 0;
    uint last_mode = 0;
    float max_x = 0.0, max_y = 0.0;

    const uint max_n = 0x100;
    float ftemp[max_n+1][4]; // x,y,unused,width/2

    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	if ( ch == '$' )
	{
	    AddGroupGL(&gl,si,'C',ptlist->used,2);
	    continue;
	}

	u16 idx;
	if (ScanU16SI(si,&idx,1,0))
	{
	    err = ERR_WARNING;
	    break;
	}

	kmp_ckpt_entry_t temp;
	ScanFloatV2SI(si,temp.left,2);

	if ( auto_respawn > 0 )
	{
	    si->no_warn++;
	    ScanU8SI(si,&temp.respawn,1,0);
	    si->no_warn--;
	}
	else
	    ScanU8SI(si,&temp.respawn,1,0);

	if ( si->cur_file->szs_modifier > 0 )
	{
	    ScanU8SI(si,&temp.mode,1,1);
	    ScanU8SI(si,&temp.prev,2,0);
	}
	else
	    ScanU8SI(si,&temp.mode,3,0);
	CheckEolSI(si);

	kmp_ckpt_entry_t * prev = GetListElem(ptlist,-1,&temp);
	if (!ptlist->used) // no previous point available
	    gl.line.n = 1;
	else if ( gl.line.n > max_n )
	    gl.line.n = max_n;

	float *p1 = 0, *p2 = 0;
	if ( gl.line.n > 1 )
	{
	    if ( gl.line.line_mode == LM_BORDER )
	    {
		p1 = ftemp[0];
		memcpy(p1,prev->left,4*sizeof(float));
		p2 = ftemp[gl.line.n];
		memcpy(p2,temp.left,4*sizeof(float));
	    }
	    else
	    {
		p1 = ftemp[0];
		p1[0] = ( prev->left[0] + prev->right[0] ) / 2;
		p1[1] = ( prev->left[1] + prev->right[1] ) / 2;
		p1[2] = 0.0;
		double dx = prev->left[0] - prev->right[0];
		double dy = prev->left[1] - prev->right[1];
		p1[3] = sqrt ( dx*dx + dy*dy ) / 2;

		p2 = ftemp[gl.line.n];
		p2[0] = ( temp.left[0] + temp.right[0] ) / 2;
		p2[1] = ( temp.left[1] + temp.right[1] ) / 2;
		p2[2] = 0.0;
		dx = temp.left[0] - temp.right[0];
		dy = temp.left[1] - temp.right[1];
		p2[3] = sqrt ( dx*dx + dy*dy ) / 2;

		if ( gl.line.line_mode == LM_BEZIER && !gl.line.n_pt )
		{
		    double angle1 = CalcDirection2F(prev->left,prev->right);
		    double angle2 = CalcDirection2F(temp.left,temp.right);
		    PRINT("BINGO 2: %7.2f %7.2f -> %7.2f %7.2f\n",
				angle1, angle2,
				fmod(angle1+M_PI,M_PI), fmod(angle2+M_PI,M_PI) );

		    double dx = p1[0] - p2[0];
		    double dy = p1[1] - p2[1];
		    double range = sqrt( dx*dx + dy*dy ) * 0.707; // sin(pi/4)

		    gl.line.pt[0][0] = p1[0] + cos(angle1) * range;
		    gl.line.pt[0][1] = p1[1] - sin(angle1) * range;
		    gl.line.pt[1][0] = p2[0] - cos(angle2) * range;
		    gl.line.pt[1][1] = p2[1] + sin(angle2) * range;
		    gl.line.n_pt = 2;
		    PRINT("\n-> %11.3f %11.3f\n"
			    "   %11.3f %11.3f\n"
			    "   %11.3f %11.3f\n"
			    "   %11.3f %11.3f\n",
			  p1[0], p1[1],
			  gl.line.pt[0][0], gl.line.pt[0][1],
			  gl.line.pt[1][0], gl.line.pt[1][1],
			  p2[0], p2[1] );
		}
	    }
	}

	uint p;
	for ( p = 1; p <= gl.line.n; p++ )
	{
	    const uint grp_idx = AddPointGL(&gl,ptlist->used);
	    kmp_ckpt_entry_t * entry = AppendList(ptlist);
	    memcpy(entry,&temp,sizeof(*entry));
	    if ( p < gl.line.n )
		CalcLine(&gl.line,ftemp[p],p1,p2,4,p);

	    switch (auto_mode)
	    {
		//case KMP_AM_NORM:	// normalize modes
		//    --> delayed
		//    break;

		case KMP_AM_RENUMBER:	// renumber  modes
		    if (!entry->mode)
			last_mode = 0;
		    else if (!IS_M1(entry->mode))
			entry->mode = ++last_mode;
		    break;

		case KMP_AM_1_LAP:	// define modes as 1 lap race
		    entry->mode = ptlist->used < 7 ? ptlist->used % 2 - 1 : -1;
		    break;

		case KMP_AM_SHORT:	// define modes to get a very short race
		    entry->mode = ptlist->used % 2 - 1;
		    break;

		case KMP_AM_UNLIMIT:	// remove lap counter for unlimited race
		    {
			static const u8 tab[] = { 0, 3,0,1,2 };
			entry->mode = ptlist->used < sizeof(tab)/sizeof(*tab)
					? tab[ptlist->used] : -1;

			if ( max_x < entry->left[0] )
			     max_x = entry->left[0];
			if ( max_x < entry->right[0] )
			     max_x = entry->right[0];

			if ( max_y < entry->left[1] )
			     max_y = entry->left[1];
			if ( max_y < entry->right[1] )
			     max_y = entry->right[1];
		    }
		    break;

		default:
		    if ( !IS_M1(entry->mode) )
			last_mode = entry->mode;
		    break;
	    }

	    if (auto_next)
	    {
		entry->next = ~0;
		entry->prev = grp_idx ? ptlist->used - 2 : ~0;
		if ( grp_idx > 0 && IS_M1(entry[-1].next) )
		    entry[-1].next = ptlist->used - 1;
	    }

	    if ( auto_respawn > 0 )
	    {
		const uint idx = ptlist->used - 1;
		if ( idx < KMP_MAX_POINT )
		    kmp->ckpt_auto_respawn[idx] = 1;
	    }
	}

	if ( gl.line.line_mode == LM_BORDER )
	{
	    // now copy the calculated values
	    for ( p = 1; p < gl.line.n; p++ )
	    {
		kmp_ckpt_entry_t * entry = prev + p;
		memcpy(entry,ftemp[p],4*sizeof(float));
	    }
	}
	else
	{
	    // now calculate angle and real positions
	    for ( p = 1; p < gl.line.n; p++ )
	    {
		kmp_ckpt_entry_t * entry = prev + p;
		float (*f)[4] = ftemp + p-1;
		double angle1 = CalcDirection2F(f[0],f[1]);
		double angle2 = CalcDirection2F(f[1],f[2]);
		noPRINT("%2u: %6.0f %6.0f %6.0f | %6.0f %6.0f %6.0f"
			" | %6.0f %6.0f %6.0f | %7.2f %7.2f\n",
		    p,
		    f[0][0],f[0][1],f[0][3], f[1][0],f[1][1],f[1][3], f[2][0],f[2][1],f[2][3],
		    angle1 * 180.0 / M_PI, angle2  * 180.0 / M_PI );

		angle1 = ( angle1 + angle2 ) / 2;
		double d = cos(angle1) * f[1][3];
		entry->left [0] = f[1][0] + d;
		entry->right[0] = f[1][0] - d;
		d = sin(angle1) * f[1][3];
		entry->left [1] = f[1][1] - d;
		entry->right[1] = f[1][1] + d;
	    }
	}
	InitializeLine(&gl.line);
    }

    if ( auto_mode == KMP_AM_NORM )
    {
	uint min_search = 1, next_val = 1;
	kmp_ckpt_entry_t *pt_end = (kmp_ckpt_entry_t*)ptlist->list + ptlist->used;
	for(;;)
	{
	    uint min_found = 0xff;
	    kmp_ckpt_entry_t *pt;
	    for ( pt = (kmp_ckpt_entry_t*)ptlist->list; pt < pt_end; pt++ )
		if ( pt->mode >= min_search && pt->mode < min_found )
		    min_found = pt->mode;

	    if ( min_found == 0xff )
		break;

	    for ( pt = (kmp_ckpt_entry_t*)ptlist->list; pt < pt_end; pt++ )
		if ( pt->mode == min_found )
		    pt->mode = next_val;

	    next_val++;
	    min_search = min_found + 1;
	}
    }
    else if ( auto_mode == KMP_AM_UNLIMIT && ptlist->used > 3 )
    {
	kmp_ckpt_entry_t *pt = ((kmp_ckpt_entry_t*)ptlist->list) + 3;
	pt->left[0]  = max_x + 20000.0;
	pt->left[1]  = max_y + 20000.0;
	pt->right[0] = max_x + 22000.0;
	pt->right[1] = max_y + 22000.0;
    }
    else if ( auto_mode == KMP_AM_SHORT || auto_mode == KMP_AM_1_LAP )
	kmp->multi_ckpt_mode_0_ok = true;

    TermGL(&gl,si,kmp,KMP_CKPH,rotate_group_links);
    if (auto_group_name)
	RenameGroupKMP(kmp,KMP_CKPH,kmp->ckph.gname,false);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static int get_defobj_id ( ScanInfo_t * si, u16 min_id, ccp cmd )
{
    u16 obj_id;
    const enumError err = ScanU16SI(si,&obj_id,1,0);
    if (err)
	return -1;

    ScanFile_t *sf = si->cur_file;
    if (min_id)
    {
	if ( obj_id >= min_id && obj_id <= min_id+0xfff )
	    return obj_id;
    }
    else if (IsDefinitionObjectId(obj_id))
	return obj_id;

    sf->ptr = sf->prev_ptr;
    WarnIgnoreSI(si,"Invalid id for %s (value %#x). ",cmd,obj_id);
    GotoEolSI(si);
    return -1;
}

//-----------------------------------------------------------------------------

static u16 negate_ref_id ( u16 ref_id )
{
    if (IsConditionRefEngineId(ref_id))
	return ref_id ^ GOBJ_MASK_COND_ENGINE;

    if (IsConditionRefRandomId(ref_id))
	return ref_id ^ GOBJ_MASK_COND_RND;

//    if (IsConditionRefTestId(ref_id))
//	return ref_id ^ GOBJ_MASK_COND_RND;

    return ref_id ^ 1;
}

//-----------------------------------------------------------------------------

static void set_ref_id ( List_t *dlist, u16 ref_id, int mode )
{
    // mode: <0: do nothing, =0:disable GOBJ_M_LECODE, >0:enable GOBJ_M_LECODE

    kmp_gobj_entry_t *entry = GetListElem(dlist,-1,0);
    if (entry)
    {
	if ( !mode || IsDefinitionObjectId(entry->obj_id) )
	    entry->obj_id &= ~GOBJ_M_LECODE;
	else if ( mode > 0 )
	    entry->obj_id |= GOBJ_M_LECODE;

	entry->ref_id = ref_id;
	entry->pflags = entry->pflags & ~GOBJ_M_PF_MODE | 1 << GOBJ_S_PF_MODE;
    }
}

//-----------------------------------------------------------------------------

static void disable_enable ( ScanInfo_t *si, List_t *dlist, bool enable )
{
    DEFINE_VAR(var);
    if (!ScanExprSI(si,&var))
    {
	int ref_id = GetIntV(&var);
	if ( ref_id < 0 )
	{
	    ref_id = -ref_id;
	    enable = !enable;
	}

	if ( IsConditionRefId(ref_id) || IsConditionRefTestId(ref_id) )
	{
	    if (!enable)
		ref_id = negate_ref_id(ref_id);
	    set_ref_id(dlist,ref_id,enable);
	}
	else if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ScanFile_t *sf = si->cur_file;
	    ERROR0(ERR_WARNING,
		"Invalid ID %#x for predefinied condition [%s @%u]:\n%.*s\n",
		ref_id, sf->name, sf->line, (int)(eol-sf->prev_ptr), sf->prev_ptr );
	}
    }
    FreeV(&var);
    CheckEolSI(si);
}

//-----------------------------------------------------------------------------

static void disable_enable_by
	( ScanInfo_t *si, List_t *dlist, bool enable, bool not )
{
    DEFINE_VAR(var);
    if (!ScanExprSI(si,&var))
    {
	int ref_id = GetIntV(&var);
	if ( ref_id < 0 )
	{
	    ref_id = -ref_id;
	    enable = !enable;
	}

	if (IsDefinitionObjectId(ref_id))
	{
	    if (not)
		ref_id |= GOBJ_DEF_F_NOT;
	    else
		ref_id &= ~GOBJ_DEF_F_NOT;
	    set_ref_id(dlist,ref_id,enable);
	}
	else if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ScanFile_t *sf = si->cur_file;
	    ERROR0(ERR_WARNING,
		"Invalid ID %#x for a reference to a definition-object [%s @%u]:\n%.*s\n",
		ref_id, sf->name, sf->line, (int)(eol-sf->prev_ptr), sf->prev_ptr );
	}
    }
    FreeV(&var);
    CheckEolSI(si);
}

//-----------------------------------------------------------------------------

static enumError ScanTextGOBJ
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextGOBJ()\n");

    tform_info_t tform_pos;
    ResetTformInfo(&tform_pos);

    kcl_fall_t kfall;
    ResetKclFall(&kfall);

    int auto_object_name = 0, sort_objects = KSORT_OFF;
    int fix_objdef = 0, sort_objdef = 0;
    int fix_reference = 0, sort_reference = 0;
    uint auto_enemy_item = 0, auto_enemy_index = 32;
    ScanParam_t ptab[] =
    {
	{ "GOBJ-HEAD-VALUE",	SPM_U16,  kmp->value+KMP_GOBJ },
	{ "AUTO-ENEMY-ITEM",	SPM_UINT, &auto_enemy_item },
	{ "AUTO-OBJECT-NAME",	SPM_INT,  &auto_object_name },
	{ "SORT-OBJECTS",	SPM_INT,  &sort_objects },

	{ "FIX-OBJDEF",		SPM_INT,  &fix_objdef },
	{ "SORT-OBJDEF",	SPM_INT,  &sort_objdef },
	{ "FIX-REFERENCE",	SPM_INT,  &fix_reference },
	{ "SORT-REFERENCE",	SPM_INT,  &sort_reference },

	TFORM_VARS(tform_pos)
	KCL_FALL_VARS(kmp,kfall)
	{0}
    };

    List_t *dlist = kmp->dlist + KMP_GOBJ;
    dlist->used = 0;

    bool ignore_itembox
	=  kmp->rtobj_enpt.obj == GOBJ_ITEMBOX
	|| kmp->rtobj_itpt.obj == GOBJ_ITEMBOX
	|| kmp->rtobj_jgpt.obj == GOBJ_ITEMBOX
	|| kmp->rtobj_cnpt.obj == GOBJ_ITEMBOX
	|| kmp->rtobj_mspt.obj == GOBJ_ITEMBOX;
    if (!ignore_itembox)
    {
	uint pi;
	for ( pi = 0; pi < kmp->n_rtobj_poti; pi++ )
	    if ( kmp->rtobj_poti[pi].rtobj.obj == GOBJ_ITEMBOX )
	    {
		ignore_itembox = true;
		break;
	    }
    }

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    CheckTformInfo(&tform_pos);
	    CheckKclFall(&kfall);
	    continue;
	}

	if ( ch == '$' )
	{
	    ScanFile_t *sf = si->cur_file;
	    DASSERT(sf);

	    char name[KMP_MAX_NAME_LEN+1];
	    ScanNameSI(si,name,sizeof(name),true,true,0);
	    int start_index = -1;
	    u16 min_id = 0x2000;

	    if (!strcmp(name,"$DEFOBJ-RACE"))
		start_index = KMP_GMODE_VERSUS;
	    else if (!strcmp(name,"$DEFOBJ-BATTLE"))
		start_index = KMP_GMODE_BALLOON;
	    else if (!strcmp(name,"$DEFOBJ-OR"))
	    {
		start_index = KMP_GMODE_BALLOON;
		min_id = 0x4000;
	    }
	    else if (!strcmp(name,"$DEFOBJ-AND"))
	    {
		start_index = KMP_GMODE_BALLOON;
		min_id = 0x6000;
	    }
	    else if (!strcmp(name,"$DISABLE"))
	    {
		disable_enable(si,dlist,false);
		continue;
	    }
	    else if (!strcmp(name,"$ENABLE"))
	    {
		disable_enable(si,dlist,true);
		continue;
	    }
	    else if (!strcmp(name,"$DISABLE-BY"))
	    {
		disable_enable_by(si,dlist,false,true);
		continue;
	    }
	    else if (!strcmp(name,"$ENABLE-BY"))
	    {
		disable_enable_by(si,dlist,true,false);
		continue;
	    }
	    else if (!strcmp(name,"$DISABLE-BY-NOT"))
	    {
		disable_enable_by(si,dlist,false,false);
		continue;
	    }
	    else if (!strcmp(name,"$ENABLE-BY-NOT"))
	    {
		disable_enable_by(si,dlist,true,true);
		continue;
	    }
	    else if (!strcmp(name,"$REFERENCE"))
	    {
		int ref_id = get_defobj_id(si,0,name);
		if ( ref_id >= 0 )
		    set_ref_id(dlist,ref_id,0);
		continue;
	    }
	    else
	    {
		sf->ptr = sf->prev_ptr;
		WarnIgnoreSI(si,"Unknown keyword »%s«.",name);
		GotoEolSI(si);
		continue;
	    }

	    //-- $DEFOBJ-RACE | $DEFOBJ-BATTLE | $DEFOBJ-OR | $DEFOBJ-AND

	    int obj_id = get_defobj_id(si,min_id,name);
	    if ( obj_id < 0 )
		continue;

	    kmp_gobj_entry_t * entry = AppendList(dlist);
	    entry->obj_id = obj_id;
	    entry->route_id = ~0;
	    entry->pflags = GOBJ_PF_MODE1;

	    u16 val = 0;
	    int index = 0;
	    DEFINE_VAR(var);
	    while ( start_index < 8 )
	    {
		if (!ScanExprSI(si,&var))
		    val = GetIntV(&var);
		while ( index <= start_index )
		    entry->setting[index++] = val;
		start_index++;

		if (!SkipCharSI(si,','))
		    break;
	    }

	    if ( min_id == 0x2000 )
		while ( index < 8 )
		    entry->setting[index++] = val;

	    CheckEolSI(si);
	    FreeV(&var);
	    continue;
	}

	enumError err;
	ScanIndexSI(si,&err,dlist->used,kmp->index+KMP_GOBJ,0);
	if (err)
	    break;

	kmp_gobj_entry_t * entry = AppendList(dlist);

	if ( si->cur_file->szs_modifier > 0 )
	{
	    ScanU16SI(si,&entry->obj_id,1,1);
	    ScanU16SI(si,&entry->ref_id,1,0);
	    ScanFloatV3SI(si,entry->position,3);
	    ScanBE32SI_swap(si,(u32*)&entry->route_id,5,1);
	}
	else
	{
	    ScanU16SI(si,&entry->obj_id,1,0);
	     ScanFloatV3SI(si,entry->position,1);
	     ScanU16SI(si,entry->setting,4,0);
	     ScanU16SI(si,&entry->route_id,1,0);
	    ScanU16SI(si,&entry->ref_id,1,0);
	     ScanFloatV3SI(si,entry->rotation,1);
	     ScanU16SI(si,entry->setting+4,4,0);
	     ScanU16SI(si,&entry->pflags,1,0);
	    ScanFloatV3SI(si,entry->scale,1);
	}

	TransformPos(&tform_pos,entry->position);
	CalcKclFall(&kfall,entry->position);
	CheckEolSI(si);

	if ( ignore_itembox
		&& entry->obj_id < N_KMP_GOBJ
		&& ObjectInfo[entry->obj_id].flags & OBF_ITEMBOX )
	{
	    dlist->used--;
	}
	else if ( auto_enemy_item
		&& entry->obj_id == GOBJ_ITEMBOX
		&& !entry->setting[2] )
	{
	    for(;;)
	    {
		if ( auto_enemy_index > 32 )
		     auto_enemy_index = 0;
		if ( auto_enemy_item & ( 1 << auto_enemy_index++ ) )
		    break;
	    }
	    entry->setting[2] = auto_enemy_index - 1;
	}
    }
    CheckLevelSI(si);

    FixDefObj(kmp,fix_objdef);
    FixReference(kmp,fix_reference);

    if (SortGOBJ(kmp,sort_objects))
	auto_object_name = 1;

    if (SortDefObj(kmp,sort_objdef))
	auto_object_name = 1;

    if (SortReference(kmp,sort_reference))
	auto_object_name = 1;

    if ( auto_object_name > 0 )
    {
	ResetIL(kmp->index+KMP_GOBJ);
	FillIL(kmp->index+KMP_GOBJ,dlist->used);
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextAREA
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextAREA()\n");

    ScanParam_t ptab[] =
    {
	{ "AREA-HEAD-VALUE", SPM_U16, kmp->value+KMP_AREA },
	{0}
    };

    List_t *dlist = kmp->dlist + KMP_AREA;
    dlist->used = 0;

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	enumError err;
	ScanIndexSI(si,&err,dlist->used,kmp->index+KMP_AREA,2);
	if (err)
	    break;

	kmp_area_entry_t * entry = AppendList(dlist);

	if ( si->cur_file->szs_modifier > 0 )
	{
	    ScanBE32SI(si,(u32*)&entry->mode,1,1);
	    ScanFloatV3SI(si,entry->position,3);
	    ScanBE32SI_swap(si,(u32*)entry->setting,2,1);
	}
	else if ( si->cur_file->revision < 2956 )
	{
	    ScanBE16SI(si,(u16*)&entry->mode,2,0);
	     ScanFloatV3SI(si,entry->position,1);
	     ScanU16SI(si,entry->setting,4,0);
	    ScanFloatV3SI(si,entry->rotation,1);
	    ScanFloatV3SI(si,entry->scale,1);
	}
	else if ( si->cur_file->revision < 8317 )
	{
	    // since  2011-08-26, v0.17a, r2956 -> new table layout

	    ScanU8SI(si,&entry->mode,2,0);
	     ScanFloatV3SI(si,entry->position,1);
	     ScanU16SI(si,entry->setting,4,0);
	    ScanU8SI(si,&entry->dest_id,2,0);
	     ScanFloatV3SI(si,entry->rotation,1);
	    ScanFloatV3SI(si,entry->scale,1);
	}
	else
	{
	    // since  2020-12-04, v2.22a, r8317 -> new table layout

	    ScanU8SI(si,&entry->mode,2,0);
	     ScanFloatV3SI(si,entry->position,1);
	     ScanU16SI(si,entry->setting,2,0);
	    ScanU8SI(si,&entry->dest_id,2,0);
	     ScanFloatV3SI(si,entry->rotation,1);
	     ScanU8SI(si,&entry->route,2,0);
	     ScanU16SI(si,&entry->unknown_2e,1,0);
	    ScanFloatV3SI(si,entry->scale,1);
	}
	CheckEolSI(si);
    }
    CheckLevelSI(si);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextCAME
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextCAME()\n");

    u8 *value = (u8*)(kmp->value + KMP_CAME);
    ScanParam_t ptab[] =
    {
	{ "CAME-HEAD-VALUE", SPM_U16, (u16*)value },
     #ifdef LITTLE_ENDIAN
	{ "OPENING-INDEX",   SPM_U8, value+1 },
	{ "SELECTION-INDEX", SPM_U8, value+0 },
     #else
	{ "OPENING-INDEX",   SPM_U8, value+0 },
	{ "SELECTION-INDEX", SPM_U8, value+1 },
     #endif
	{0}
    };

    List_t *dlist = kmp->dlist + KMP_CAME;
    dlist->used = 0;

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	enumError err;
	ScanIndexSI(si,&err,dlist->used,kmp->index+KMP_CAME,2);
	if (err)
	    break;

	kmp_came_entry_t * entry = AppendList(dlist);

	uint zoom_speed = 0, viewpt_speed = 0;

	if ( si->cur_file->szs_modifier > 0 )
	{
	    ScanBE32SI(si,(u32*)&entry->type,1,1);
	    ScanBE32SI_swap(si,(u32*)&entry->came_speed,2,1);
	    ScanFloatV3SI(si,entry->position,2);
	    ScanFloatSI(si,&entry->zoom_begin,2);
	    ScanFloatV3SI(si,entry->viewpt_begin,2);
	    ScanFloatSI(si,&entry->time,1);
	    zoom_speed = entry->zoom_speed;
	    viewpt_speed = entry->viewpt_speed;
	}
	else if ( si->cur_file->revision < 2955 )
	{
	    // 2011-08-26, v0.17a, r2956 -> new table layout

	    ScanU8SI(si,&entry->type,4,0);
	     ScanU16SI(si,&entry->came_speed,1,0);
	     ScanFloatV3SI(si,entry->position,1);
	     ScanFloatSI(si,&entry->time,1);
	    ScanU32SI(si,&zoom_speed,1,0);
	     ScanFloatV3SI(si,entry->rotation,1);
	    ScanU32SI(si,&viewpt_speed,1,0);
	     ScanFloatSI(si,&entry->zoom_begin,2);
	    ScanU16SI(si,&entry->unknown_0x0a,1,0);
	     ScanFloatV3SI(si,entry->viewpt_begin,2);
	}
	else
	{
	    ScanU8SI(si,&entry->type,1,0);
	     ScanU16SI(si,&entry->came_speed,1,0);
	     ScanFloatV3SI(si,entry->position,1);
	     ScanFloatSI(si,&entry->zoom_begin,1);
	    ScanU8SI(si,&entry->next,1,0);
	     ScanU32SI(si,&zoom_speed,1,0);
	     ScanFloatV3SI(si,entry->rotation,1);
	     ScanFloatSI(si,&entry->zoom_end,1);
	    ScanU8SI(si,&entry->unknown_0x02,1,0);
	     ScanU32SI(si,&viewpt_speed,1,0);
	     ScanFloatV3SI(si,entry->viewpt_begin,1);
	    ScanU8SI(si,&entry->route,1,0);
	     ScanU16SI(si,&entry->unknown_0x0a,1,0);
	     ScanFloatV3SI(si,entry->viewpt_end,1);
	     ScanFloatSI(si,&entry->time,1);
	}
	CheckEolSI(si);

	const double zoom_len = fabs(entry->zoom_begin-entry->zoom_end);
	entry->zoom_speed = zoom_speed;
	if ( zoom_speed == ~(u32)0 )
	    entry->zoom_speed = entry->time >= 0.001
			? (uint)floor( 100.0 * zoom_len / entry->time )
			: 1;

	const double viewpt_len = LengthF(entry->viewpt_begin,entry->viewpt_end);
	entry->viewpt_speed = viewpt_speed;
	if ( viewpt_speed == ~(u32)0 )
	    entry->viewpt_speed = entry->time >= 0.001
			? (uint)floor( 100.0 * viewpt_len / entry->time )
			: 1;

	DefineIndexDouble(si,"CAME.TIME",entry->time);
	DefineIndexDouble(si,"ZOOM.LEN",zoom_len);
	DefineIndexDouble(si,"VIEWPT.LEN",viewpt_len);
    }
    CheckLevelSI(si);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextPT2
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si,		// valid data
    uint		sect		// KMP section index
)
{
    DASSERT(kmp);
    DASSERT(si);
    DASSERT(sect<KMP_N_SECT);
    TRACE("ScanTextPT2(%s)\n",kmp_section_name[sect].name1);

    Var_t *snap2_to_enpt, *snap_to_enpt, *hsnap_to_enpt;
    kmp_rtobj_t *rtobj = 0;

    Var_t var_dummy, *auto_adjust = &var_dummy;
    int   int_dummy, *auto_define = &int_dummy, *auto_remove = &int_dummy;
    float float_dummy, *auto_distance = &float_dummy;
    kmp_flag_t *kf = 0;
    int export_flags = 0;

    switch (sect)
    {
	case KMP_JGPT:
	    snap2_to_enpt	= &kmp->jgpt_snap2;
	    snap_to_enpt	= &kmp->jgpt_snap;
	    hsnap_to_enpt	= &kmp->jgpt_hsnap;
	    rtobj		= &kmp->rtobj_jgpt;
	    auto_define		= &kmp->jgpt_auto_define;
	    auto_distance	= &kmp->jgpt_auto_distance;
	    auto_adjust		= &kmp->jgpt_auto_adjust;
	    auto_remove		= &kmp->jgpt_auto_remove;
	    kf			= &kmp->jgpt_flag;
	    break;

	case KMP_CNPT:
	    snap2_to_enpt	= &kmp->cnpt_snap2;
	    snap_to_enpt	= &kmp->cnpt_snap;
	    hsnap_to_enpt	= &kmp->cnpt_hsnap;
	    rtobj		= &kmp->rtobj_cnpt;
	    break;

	case KMP_MSPT:
	    snap2_to_enpt	= &kmp->mspt_snap2;
	    snap_to_enpt	= &kmp->mspt_snap;
	    hsnap_to_enpt	= &kmp->mspt_hsnap;
	    rtobj		= &kmp->rtobj_mspt;
	    break;

	default:
	    var_dummy.mode	= VAR_UNSET;
	    snap2_to_enpt	= &var_dummy;
	    snap_to_enpt	= &var_dummy;
	    hsnap_to_enpt	= &var_dummy;
	    break;
    }

    rtobj_info_t rtinfo;
    ResetRouteInfo(&rtinfo,&kmp->rtobj);

    tform_info_t tform_pos;
    ResetTformInfo(&tform_pos);

    kcl_fall_t kfall;
    ResetKclFall(&kfall);

    int auto_id = 0;
    char hd_name[20];
    snprintf( hd_name, sizeof(hd_name),
			"%s-HEAD-VALUE", kmp_section_name[sect].name1 );
    ScanParam_t ptab[] =
    {
	{ hd_name,		SPM_U16,    kmp->value+sect },
	{ "AUTO-ID",		SPM_INT,    &auto_id },
	{ "SNAP2-TO-ENPT",	SPM_VAR,    snap2_to_enpt },
	{ "SNAP-TO-ENPT",	SPM_VAR,    snap_to_enpt },
	{ "HSNAP-TO-ENPT",	SPM_VAR,    hsnap_to_enpt },
	{ "AUTO-DEFINE",	SPM_INT,    auto_define },
	{ "AUTO-DISTANCE",	SPM_FLOAT,  auto_distance },
	{ "AUTO-ADJUST",	SPM_VAR,    auto_adjust },
	{ "AUTO-REMOVE",	SPM_INT,    auto_remove },
	{ "EXPORT-FLAGS",	SPM_INT,    &export_flags },
	ROUTE_OBJECT_VARS(rtinfo)
	TFORM_VARS(tform_pos)
	KCL_FALL_VARS(kmp,kfall)
	{0}
    };

    List_t *dlist = kmp->dlist + sect;
    dlist->used = 0;

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    CheckRouteInfo(&rtinfo);
	    CheckTformInfo(&tform_pos);
	    CheckKclFall(&kfall);
	    continue;
	}

	enumError err;
	ScanIndexSI(si,&err,dlist->used,kmp->index+sect,2);
	if (err)
	    break;

	kmp_jgpt_entry_t * entry = AppendList(dlist);
	ScanFloatV3SI(si,entry->position,2);
	ScanU16SI(si,(u16*)&entry->id,1,0);
	ScanU16SI(si,(u16*)&entry->effect,1,si->cur_file->szs_modifier);

	const u8 flags = !kf ? KPFL_M_ALL
			: NextCharSI(si,false) ? ScanPtFlags(si) : kf->mask;
	if ( kf && (uint)(dlist->used-1) < KMP_MAX_POINT )
		kf->flags[dlist->used-1] = flags;

	TransformPos(&tform_pos,entry->position);
	if ( flags & KPFL_FALL )
	    CalcKclFall(&kfall,entry->position);
	CheckEolSI(si);

	if (auto_id)
	    entry->id = dlist->used - 1;
    }
    CheckLevelSI(si);

    PRINT_IF( snap2_to_enpt && snap2_to_enpt->mode,
		"SNAP-TO-ENPT: %d -> jgpt=%d, cnpt=%d, mspt=%d\n",
		snap2_to_enpt->mode, kmp->jgpt_snap2.mode,
		kmp->cnpt_snap2.mode, kmp->mspt_snap2.mode );
    PRINT_IF( snap_to_enpt && snap_to_enpt->mode,
		"SNAP-TO-ENPT: %d -> jgpt=%d, cnpt=%d, mspt=%d\n",
		snap_to_enpt->mode, kmp->jgpt_snap.mode,
		kmp->cnpt_snap.mode, kmp->mspt_snap.mode );

    if (kf)
	kf->export = opt_export_flags ? opt_export_flags>0 : export_flags;

    if (rtobj)
	AssignRouteInfo(&rtinfo,rtobj);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextSTGI
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextSTGI()\n");

    ScanParam_t ptab[] =
    {
	{ "STGI-HEAD-VALUE", SPM_U16, kmp->value+KMP_STGI },
	{0}
    };

    List_t *dlist = kmp->dlist + KMP_STGI;
    dlist->used = 0;

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	enumError err;
	ScanIndexSI(si,&err,dlist->used,kmp->index+KMP_STGI,2);
	if (err)
	    break;

	kmp_stgi_entry_t * entry = AppendList(dlist);
	if ( si->cur_file->revision < 3379 )
	{
	    // 2012-01-28, v0.26a, r3379
	    ScanBE16SI(si,(u16*)&entry->lap_count,6,0);
	    entry->flare_color = be32(&entry->flare_color);
	}
	else
	{
	    ScanU8SI(si,&entry->lap_count,4,0);
	    ScanU32SI(si,&entry->flare_color,1,0);
	    ScanU8SI(si,&entry->flare_alpha,2,0);

	    DEFINE_VAR(var);
	    ScanExprSI(si,&var);
	    if ( var.mode == VAR_DOUBLE || var.mode == VAR_VECTOR )
		entry->speed_mod = SpeedMod2u16(GetDoubleV(&var));
	    else
	    {
		u8 v2;
		ScanU8SI(si,&v2,1,0);
		entry->speed_mod = GetIntV(&var) << 8 | v2;
	    }
	}
	CheckEolSI(si);
    }
    CheckLevelSI(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextSETUP
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    rtobj_info_t rtinfo;
    ResetRouteInfo(&rtinfo,&kmp->rtobj);

    ScanParam_t ptab[] =
    {
	ROUTE_OBJECT_VARS(rtinfo)
	{0}
    };

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    CheckRouteInfo(&rtinfo);
	    continue;
	}

	char name[50];
	if ( !ScanNameSI(si,name,sizeof(name),true,true,0)
		|| NextCharSI(si,true) != '=' )
	{
	    CheckEolSI(si);
	    continue;
	}
	DASSERT(si->cur_file);
	si->cur_file->ptr++;
	noPRINT("### %s = %.4s...\n",name,si->cur_file->ptr);

	if (!strcmp(name,"REVISION"))
	{
	    ScanU32SI(si,(u32*)&kmp->revision,1,0);
	    si->init_revision = si->cur_file->revision = kmp->revision;
	    DefineIntVar(&si->gvar,"REVISION$SETUP",kmp->revision);
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",kmp->revision);
	}
	else if (!strcmp(name,"KCL-FALL-ADD-ROAD"))
	{
	    DEFINE_VAR(val);
	    if ( ScanExprSI(si,&val) <= ERR_WARNING )
		kmp->enable_fall_kcl = GetBoolV(&val);
	}
	else if (!strcmp(name,"KMP-WIM0"))
	{
	    DEFINE_VAR(val);
	    if ( ScanExprSI(si,&val) <= ERR_WARNING )
		kmp->wim0_export = GetBoolV(&val) || opt_wim0 > 0;
	}
	// "UNKNOWN-0X0C" replaced by "KMP-VERSION" in 2020-01
	else if ( !strcmp(name,"KMP-VERSION") || !strcmp(name,"UNKNOWN-0X0C") )
	    ScanU32SI(si,&kmp->kmp_version,1,0);
	else if (!strcmp(name,"INFO"))
	    ConcatStringsSI(si,&kmp->info,0);
	else
	    GotoEolSI(si);
	CheckEolSI(si);
    }
    CheckLevelSI(si);
    if (rtinfo.valid)
	AssignRouteInfo(&rtinfo,&kmp->rtobj);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		   ScanTextKMP() route functions	///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct route_info_t
{
    int		index;			// index of route

    int		f_xss;
    int		f_yss;
    int		f_zss;
    int		f_scale;
    int		f_shift;
    int		f_hrotate;		// [[[hrot]] [[2do]]

    rtobj_info_t rtinfo;
    kcl_fall_t	kfall;

    double	xss[4];
    double	yss[4];
    double	zss[4];
    Var_t	scale[2];
    Var_t	shift;
    Var_t	hrotate[2];		// [[[hrot]] [[2do]]
    int		reverse_order;

    double	route_time;		// >0.0: recalc setting 1

    int		show_route;		// >0: enable itemboxes for this route

} route_info_t;

///////////////////////////////////////////////////////////////////////////////

static void CloseRoute
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si,		// valid data
    const kmp_poti_group_t
			* grp,		// group to close
    route_info_t	* ri		// route info data
)
{
    DASSERT(kmp);
    DASSERT(grp);
    DASSERT(ri);

    const uint np = grp->n_point;
    DASSERT( np <= kmp->poti_point.used );

    kmp_poti_point_t *pp0 = (kmp_poti_point_t*)kmp->poti_point.list
			  + kmp->poti_point.used - np;

    //--- xss/yss/zss

    if ( ri->f_xss && fabs(ri->xss[0]-ri->xss[2]) < 1e-6 )
	ri->f_xss = 0;

    if ( ri->f_yss && fabs(ri->yss[0]-ri->yss[2]) < 1e-6 )
	ri->f_yss = 0;

    if ( ri->f_zss && fabs(ri->zss[0]-ri->zss[2]) < 1e-6 )
	ri->f_zss = 0;

    if ( ri->f_xss || ri->f_yss || ri->f_zss )
    {
	ri->f_shift = 1;
	ri->f_scale = 1;
	AssignVectorV3(&ri->shift,0.0,0.0,0.0);
	AssignVectorV3(ri->scale,1.0,1.0,1.0);

	if (ri->f_xss)
	{
	    double *d = ri->xss;
	    PRINT("XSS[%d]:     %11.3f %11.3f | %11.3f %11.3f\n",
		ri->f_xss, d[0], d[1], d[2], d[3] );
	    const double scale = ( d[3] - d[1] ) / ( d[2] - d[0] );
	    ri->scale[0].x = scale;
	    ri->shift.x = d[1] - d[0] * scale;
	}

	if (ri->f_yss)
	{
	    double *d = ri->yss;
	    PRINT("YSS[%d]:     %11.3f %11.3f | %11.3f %11.3f\n",
		ri->f_yss, d[0], d[1], d[2], d[3] );
	    const double scale = ( d[3] - d[1] ) / ( d[2] - d[0] );
	    ri->scale[0].y = scale;
	    ri->shift.y = d[1] - d[0] * scale;
	}

	if (ri->f_zss)
	{
	    double *d = ri->zss;
	    PRINT("ZSS[%d]:     %11.3f %11.3f | %11.3f %11.3f\n",
		ri->f_zss, d[0], d[1], d[2], d[3] );
	    const double scale = ( d[3] - d[1] ) / ( d[2] - d[0] );
	    ri->scale[0].z = scale;
	    ri->shift.z = d[1] - d[0] * scale;
	}
    }


    //--- scale

    if ( ri->f_scale > 1 )
    {
	Var_t *d = ri->scale;
	ToVectorV(d+0);
	ToVectorV(d+1);
	PRINT("SCALE[%d]:   %11.3f %11.3f %11.3f | %11.3f %11.3f %11.3f\n",
		ri->f_scale, d[0].x, d[0].y, d[0].z,  d[1].x, d[1].y, d[1].z );

	kmp_poti_point_t *pp = pp0;
	uint i;
	for ( i = 0; i < np; i++, pp++ )
	{
	    pp->position[0] = ( pp->position[0] - d[1].x ) * d[0].x + d[1].x;
	    pp->position[1] = ( pp->position[1] - d[1].y ) * d[0].y + d[1].y;
	    pp->position[2] = ( pp->position[2] - d[1].z ) * d[0].z + d[1].z;
	}
    }
    else if ( ri->f_scale > 0 )
    {
	Var_t *d = ri->scale;
	ToVectorV(d+0);
	PRINT("SCALE[%d]:   %11.3f %11.3f %11.3f\n",
		ri->f_scale, d->x, d->y, d->z );

	kmp_poti_point_t *pp = pp0;
	uint i;
	for ( i = 0; i < np; i++, pp++ )
	{
	    pp->position[0] *= d->x;
	    pp->position[1] *= d->y;
	    pp->position[2] *= d->z;
	}
    }


    //--- shift

    if ( ri->f_shift )
    {
	Var_t *d = &ri->shift;
	ToVectorV(d);
	PRINT("SHIFT[%d]:   %11.3f %11.3f %11.3f\n",
		ri->f_shift, d->x, d->y, d->z );

	kmp_poti_point_t *pp = pp0;
	uint i;
	for ( i = 0; i < np; i++, pp++ )
	{
	    pp->position[0] += d->x;
	    pp->position[1] += d->y;
	    pp->position[2] += d->z;
	}
     }


    //--- hrotate	// [[[hrot]] [[2do]]

    if ( ri->f_hrotate )
    {
	Var_t *d = ri->hrotate;
	ToVectorV(d+1);
	double rot = GetDoubleV(d++) * (M_PI/180.0);
	PRINT("HROTATE[%d]: %11.3f %11.3f %11.3f -> %6.3f\n",
		ri->f_hrotate, d->x, d->y, d->z,  rot );

	kmp_poti_point_t *pp = pp0;
	uint i;
	for ( i = 0; i < np; i++, pp++ )
	{
	    const double dx = pp->position[0] - d->x;
	    const double dz = pp->position[2] - d->z;
	    const double width = sqrt ( dx*dx + dz*dz );
	    if ( width >= 1e-6 )
	    {
		const double new_rot = atan2(dx,dz) + rot;
		pp->position[0] = d->x + sin(new_rot) * width;
		pp->position[2] = d->y + cos(new_rot) * width;
	    }
	}
    }


    //--- reverse order

    if (ri->reverse_order)
    {
	kmp_poti_point_t *pp1 = pp0;
	kmp_poti_point_t *pp2 = pp0 + np - 1;
	while ( pp1 < pp2 )
	{
	    noPRINT("P: %p %p / %zd %zd\n",pp1,pp2,pp1-pp0,pp2-pp0);
	    kmp_poti_point_t temp;
	    memcpy(&temp,pp1,  sizeof(temp));
	    memcpy(pp1++,pp2,  sizeof(*pp1));
	    memcpy(pp2--,&temp,sizeof(*pp2));
	}
    }


    //--- define variables

    uint i;
    double total_len = 0.0, total_tim = 0.0;
    kmp_poti_point_t *pp = pp0;
    for ( i = 0; i < np-1; i++, pp++ )
    {
	const double len = LengthF(pp->position,pp[1].position);
	total_len += len;
	if ( pp->speed > 0 )
	    total_tim += len / pp->speed;
    }

    DefineIndexDouble(si,"LEN",total_len);
    DefineIndexDouble(si,"TIME",total_tim);
    DefineIndexDouble(si,"TIME.SET",total_tim);


    //--- set route time

    if ( ri->route_time > 0.0 && total_tim > 0.0 )
    {
	PRINT("SET ROUTE TIME %s, N=%d+%u, time=%4.2f, len=%5.3f, set=%4.2f\n",
		last_index_name, kmp->poti_point.used - np, np,
		total_tim, total_len, ri->route_time );


	const double factor = ri->route_time / total_tim;
	double sum_src = 0.0, sum_dest = 0.0;
	pp = pp0;
	for ( i = 0; i < np-1; i++, pp++ )
	{
	    if ( pp->speed > 0 )
	    {
		const double len = LengthF(pp->position,pp[1].position);
		const double tim = len / pp->speed;
		sum_src += tim;
		const double wanted = sum_src * factor - sum_dest;
		DASSERT(wanted>=0.0);
		pp->speed = wanted > 0.0 ? floor( len / wanted + 0.5 ) : 0.0;
		if (!pp->speed)
		    pp->speed = 1;
		sum_dest += len / pp->speed;
		noPRINT("%6.2f/%4.2f - %6.2f/%4.2f - %6.2f -> %5u\n",
			sum_src/60.0, total_tim,
			sum_dest/60.0, total_tim*factor,
			wanted, pp->speed );
	    }
	}
	DefineIndexDouble(si,"TIME.SET",sum_dest);
    }


    //--- enable ruote itemboxes

    kmp_rtobj_t rtobj;
    if (AssignRouteInfo(&ri->rtinfo,&rtobj))
    {
	const uint max = sizeof(kmp->rtobj_poti)/sizeof(*kmp->rtobj_poti);
	uint i;
	for ( i = 0; i <= kmp->n_rtobj_poti; i++ )
	{
	    if ( i >= max )
	    {
		ERROR0(ERR_WARNING,
		    "Route list full -> @ROUTE-OBJECT ignored for route #%u",ri->index);
		break;
	    }

	    kmp_rtobj_list_t *rptr = kmp->rtobj_poti + i;
	    if ( i == kmp->n_rtobj_poti )
	    {
		rptr->index = ri->index;
		kmp->n_rtobj_poti++;

	    }

	    if ( rptr->index == ri->index )
	    {
		AssignRouteInfo(&ri->rtinfo,&rptr->rtobj);
		PRINT("@ROUTE-OBJECT[%u] = 0x%x -> %u\n",
			i, ri->index, rptr->rtobj.obj );
		break;
	    }
	}
    }

    memset(ri,0,sizeof(*ri));
    ResetRouteInfo(&ri->rtinfo,&kmp->rtobj);
    ResetKclFall(&ri->kfall);
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextPOTI
(
    kmp_t		* kmp,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(kmp);
    DASSERT(si);
    TRACE("ScanTextPOTI()\n");

    List_t *glist = kmp->dlist + KMP_POTI;
    List_t *plist = &kmp->poti_point;
    glist->used = plist->used = 0;

    Line_t line;
    InitializeLine(&line);
    kmp_poti_group_t *grp = 0;
    u8 g_smooth = 0, g_back = 0;

    int auto_route_name = 0;
    route_info_t ri = {0};
    ResetRouteInfo(&ri.rtinfo,&kmp->rtobj);
    ResetKclFall(&ri.kfall);

    ScanParam_t ptab[] =
    {
	{ "AUTO-ROUTE-NAME",    SPM_INT,    &auto_route_name },
	{ "XSS",		SPM_DOUBLE, ri.xss,     4,4, &ri.f_xss },
	{ "YSS",		SPM_DOUBLE, ri.yss,     4,4, &ri.f_yss },
	{ "ZSS",		SPM_DOUBLE, ri.zss,     4,4, &ri.f_zss },
	{ "SCALE",		SPM_VAR,    ri.scale,   1,2, &ri.f_scale },
	{ "SHIFT",		SPM_VAR,    &ri.shift,  1,1, &ri.f_shift },
// [[[hrot]] [[2do]]
	{ "HROTATE",		SPM_VAR,    ri.hrotate, 1,2, &ri.f_hrotate },
	{ "REVERSE-ORDER",	SPM_INT,    &ri.reverse_order },
	{ "SET-ROUTE-TIME",	SPM_DOUBLE, &ri.route_time },
	ROUTE_OBJECT_VARS(ri.rtinfo)
	KCL_FALL_VARS(kmp,ri.kfall)
	{0}
    };

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    CheckRouteInfo(&ri.rtinfo);
	    CheckKclFall(&ri.kfall);
	    continue;
	}

	if ( ch == '$' )
	{
	    char name[KMP_MAX_NAME_LEN+1];
	    ScanNameSI(si,name,sizeof(name),true,true,0);

	    const int is_bezier = !strcmp(name,"$BEZIER");
	    if ( is_bezier || !strcmp(name,"$LINE") )
	    {
		enumError err = ScanLine(&line,si,3,is_bezier);
		if (err)
		    return err;
		continue;
	    }

	    if ( !strcmp(name,"$COPY") )
	    {
		uint r_idx;
		if (ScanU32SI(si,&r_idx,1,0))
		    continue;

		if ( r_idx >= glist->used )
		{
		    ScanFile_t *sf = si->cur_file;
		    DASSERT(sf);
		    si->total_err++;
		    sf->line_err++;
		    ERROR0(ERR_WARNING,
				"Can't copy unknown route #%u [%s @%u]\n",
				r_idx, sf->name, sf->line );
		    GotoEolSI(si);
		}

		uint r_beg = 0, r_end = UINT_MAX;
		if (NextCharSI(si,false))
		{
		    if (ScanU32SI(si,&r_beg,1,0))
			continue;

		    if (NextCharSI(si,false))
		    {
			if (ScanU32SI(si,&r_end,1,0))
			    continue;
			r_end++;
		    }
		}

		const kmp_poti_group_t *srcgrp = GetListElem(glist,r_idx,0);
		DASSERT(srcgrp);
		if ( r_end > srcgrp->n_point )
		     r_end = srcgrp->n_point;
		PRINT("COPY ROUTE: %u,%u..%u\n",r_idx,r_beg,r_end);
		if ( r_beg >= r_end )
		    continue;

		const kmp_poti_point_t * psrc = GetListElem(plist,0,0);
		psrc += r_beg;
		while ( r_idx > 0 )
		{
		    r_idx--;
		    const kmp_poti_group_t *g = GetListElem(glist,r_idx,0);
		    DASSERT(g);
		    psrc += g->n_point;
		}
		DASSERT( psrc < (kmp_poti_point_t*)GetListElem(plist,0,0) + plist->used );

		if (!grp)
		{
		    grp = AppendList(glist);
		    grp->smooth = g_smooth;
		    grp->back   = g_back;
		}
		DASSERT(grp);
		grp->n_point += r_end - r_beg;

		for ( ; r_beg < r_end; r_beg++, psrc++ )
		{
		    kmp_poti_point_t * pdest = AppendList(plist);
		    memcpy(pdest,psrc,sizeof(*pdest));
		}
		kmp->value[KMP_POTI] = plist->used;
		continue;
	    }

	    const bool cmd_close = !strcmp(name,"$CLOSE");
	    if ( !cmd_close && strcmp(name,"$ROUTE") && strcmp(name,"$GROUP") )
	    {
		ScanFile_t *sf = si->cur_file;
		DASSERT(sf);
		si->total_err++;
		sf->line_err++;
		CheckEolSI(si);
		ERROR0(ERR_WARNING,
			"Line ignored [%s @%u]: %s ...\n",
			sf->name, sf->line, name );
		continue;
	    }


	    //--- close group

	    if (grp)
		CloseRoute(kmp,si,grp,&ri);
	    grp = 0;

	    if (cmd_close)
		continue;

	    ScanIndexSI(si,0,glist->used,kmp->index+KMP_POTI,2);

	    for(;;)
	    {
		char ch = NextCharSI(si,false);
		noPRINT("--> %02x : %.6s\n",ch,si->cur_file->ptr);
		if (!ch)
		    break;
		si->cur_file->ptr++;
		if ( ch == ':' )
		    break;
	    }
	    u16 g_temp;
	    ScanU16SI(si,&g_temp,1,0);
	    if (NextCharSI(si,false))
	    {
		g_smooth = g_temp;
		ScanU8SI(si,&g_back,1,0);
	    }
	    else
	    {
		g_smooth = g_temp >> 8;
		g_back   = (u8)g_temp;
	    }
	    noPRINT("ROUTE: smooth=%u, back=%u\n",g_smooth,g_back);
	    CheckEolSI(si);
	    continue;
	}

	if ( !grp || !grp->n_point ) // no previous point available
	    line.n = 1;
	enumError err;
	ScanIndexSI(si,&err, grp ? grp->n_point + line.n -1 : 0, 0,2);
	if (err)
	    break;

	if (!grp)
	{
	    ri.index = glist->used;
	    grp = AppendList(glist);
	    grp->smooth = g_smooth;
	    grp->back   = g_back;
	}
	DASSERT(grp);
	grp->n_point += line.n;

	kmp_poti_point_t temp;
	ScanFloatV3SI(si,temp.position,1);
	if ( si->cur_file->szs_modifier > 0 )
	    ScanBE32SI_swap(si,(u32*)&temp.speed,1,1);
	else
	    ScanU16SI(si,&temp.speed,2,0);
	CheckEolSI(si);

	const kmp_poti_point_t * prev = GetListElem(plist,-1,&temp);

	uint p;
	for ( p = 1; p <= line.n; p++ )
	{
	    kmp_poti_point_t * ppos = AppendList(plist);
	    memcpy(ppos,&temp,sizeof(*ppos));
	    CalcLine(&line,ppos->position,prev->position,temp.position,3,p);
	    CalcKclFall(&ri.kfall,ppos->position);
	}
	kmp->value[KMP_POTI] = plist->used;
	InitializeLine(&line);
    }
    if (grp)
	CloseRoute(kmp,si,grp,&ri);
    CheckLevelSI(si);

    if ( auto_route_name > 0 )
    {
	ResetIL(kmp->index+KMP_POTI);
	FillIL(kmp->index+KMP_POTI,glist->used);
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static kmp_gobj_entry_t * AddObject
(
    kmp_t		* kmp,		// KMP data structure
    const float		* pos,		// 3D vector with position
    const float		* rot,		// NULL or a rotation 3D vector
    kmp_rtobj_t		* rtobj		// object to insert
)
{
    DASSERT(kmp);
    DASSERT(pos);
    DASSERT(rtobj);
    DASSERT( rtobj->obj > 0 && rtobj->obj < N_KMP_GOBJ );

    List_t *glist  = kmp->dlist + KMP_GOBJ;
    kmp_gobj_entry_t *gobj = AppendList(glist);
    gobj->obj_id = rtobj->obj;
    gobj->position[0] = pos[0] + rtobj->shift.x;
    gobj->position[1] = pos[1] + rtobj->shift.y;
    gobj->position[2] = pos[2] + rtobj->shift.z;
    memcpy(gobj->scale,rtobj->scale.v,sizeof(gobj->scale));
    memcpy(gobj->rotation,rtobj->rotate.v,sizeof(gobj->rotation));
    gobj->route_id = (typeof(gobj->route_id))~0;
    gobj->pflags = 0x3f;
    if (rot)
    {
	gobj->rotation[0] = NormDegreeF( gobj->rotation[0] + rot[0] );
	gobj->rotation[1] = NormDegreeF( gobj->rotation[1] + rot[1] );
	gobj->rotation[2] = NormDegreeF( gobj->rotation[2] + rot[2] );
    }
    return gobj;
}

///////////////////////////////////////////////////////////////////////////////

static void AddEnemyRouteObjects
(
    kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP section index
    kmp_rtobj_t		*rtobj		// object to insert
)
{
    DASSERT(kmp);
    DASSERT(rtobj);
    DASSERT( sect == KMP_ENPT || sect == KMP_ITPT );

    if (rtobj->obj)
    {
	PRINT("AddEnemyRouteObjects(%s)\n",kmp_section_name[sect].name1);
	const uint n = kmp->dlist[sect].used;
	const kmp_enpt_entry_t * pt = (kmp_enpt_entry_t*)kmp->dlist[sect].list;

	uint i;
	for ( i = 0; i < n; i++ )
	{
	    int last = -1, link;
	    for ( link = 0; link < KMP_MAX_PH_LINK; link++ )
	    {
		int next = GetNextPointKMP(kmp,sect,i,link,false,-1,0,0,0,0);
		if ( next >= 0 && next != last )
		{
		    last = next;
		    float3 rot = CalcDirection3F(pt[i].position,pt[next].position);
		    AddObject(kmp,pt[i].position,rot.v,rtobj);
		}
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AddJugemRouteObjects
(
    kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP section index
    kmp_rtobj_t		*rtobj		// object to insert
)
{
    DASSERT(kmp);
    DASSERT(rtobj);
    DASSERT( sect == KMP_JGPT || sect == KMP_CNPT || sect == KMP_MSPT );

    if (rtobj->obj)
    {
	PRINT("AddJugemRouteObjects(%s)\n",kmp_section_name[sect].name1);
	const uint n = kmp->dlist[sect].used;
	const kmp_jgpt_entry_t * pt = (kmp_jgpt_entry_t*)kmp->dlist[sect].list;

	uint i;
	for ( i = 0; i < n; i++, pt++ )
	    AddObject(kmp,pt->position,pt->rotation,rtobj);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void AddRouteObjects
(
    kmp_t		* kmp		// KMP data structure
)
{
    DASSERT(kmp);
    PRINT("AddRouteObjects()\n");

    const uint ng = kmp->dlist[KMP_POTI].used;
    uint ilist;
    kmp_rtobj_list_t *rptr = kmp->rtobj_poti;
    for ( ilist = 0; ilist < kmp->n_rtobj_poti; ilist++, rptr++ )
    {
	int route_idx = rptr->index;
	if ( route_idx < 0 || route_idx >= ng || !rptr->rtobj.obj )
	    continue;

	PRINT("ROUTE-ITEMBOX %u/%u -> %d/%u\n",
		ilist, kmp->n_rtobj_poti, route_idx, ng );
	const kmp_poti_group_t *grp = (kmp_poti_group_t*)kmp->dlist[KMP_POTI].list;
	kmp_poti_point_t *pt = (kmp_poti_point_t*)kmp->poti_point.list;
	while ( route_idx-- > 0 )
	{
	    pt += grp->n_point;
	    grp++;
	}

	uint i;
	for ( i = 0; i < grp->n_point; i++ )
	{
	    float3 rot = CalcDirection3F( pt[i].position,
					  pt[ (i+1) % grp->n_point ].position );
	    AddObject( kmp, pt[i].position, rot.v, &rptr->rtobj );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void setup_scale ( Var_t *dest, const Var_t *src )
{
    DASSERT(dest);
    DASSERT(src);

    if ( src->mode == VAR_VECTOR )
	AssignVar(dest,src);
    else
	AssignVectorV3(dest,0.0,GetYDoubleV(src),0.0);
    DASSERT( dest->mode == VAR_VECTOR );

    if ( dest->x <= 0.0 )
	dest->x = RTOBJ_SCALE_X;
    else if ( dest->x < RTOBJ_SCALE_MIN )
	dest->y = RTOBJ_SCALE_MIN;

    if ( dest->y <= 0.0 )
	dest->y = RTOBJ_SCALE_Y;
    else if ( dest->y < RTOBJ_SCALE_MIN )
	dest->y = RTOBJ_SCALE_MIN;

    if ( dest->z <= 0.0 )
	dest->z = RTOBJ_SCALE_Z;
    else if ( dest->z < RTOBJ_SCALE_MIN )
	dest->y = RTOBJ_SCALE_MIN;
}

//-----------------------------------------------------------------------------

static void AddCheckpointObject
(
    kmp_t		* kmp		// KMP data structure
)
{
    DASSERT(kmp);
    PRINT("AddCheckpointObject()\n");

    struct obj_t
    {
	int	object;
	Var_t	scale;
	float	base;

    } obj1, obj2;

    obj1.object = kmp->ckpt_object1 > 0 && kmp->ckpt_object1 < N_KMP_GOBJ
			? kmp->ckpt_object1 : 0;
    setup_scale(&obj1.scale,&kmp->ckpt_object1_scale);
    obj1.base = kmp->ckpt_object1_base;

    obj2.object = kmp->ckpt_object2 > 0 && kmp->ckpt_object2 < N_KMP_GOBJ
			? kmp->ckpt_object2 : 0;
    setup_scale(&obj2.scale,&kmp->ckpt_object2_scale);
    obj2.base = kmp->ckpt_object2_base;

    const struct obj_t *obj_lap_lt = 0, *obj_lap_rt = 0;
    const struct obj_t *obj_man_lt = 0, *obj_man_rt = 0;
    const struct obj_t *obj_std_lt = 0, *obj_std_rt = 0;

    switch (kmp->ckpt_object_mode)
    {
	case 1:
	    obj_lap_lt = &obj2;
	    obj_lap_rt = &obj2;
	    obj_man_lt = &obj1;
	    obj_man_rt = &obj1;
	    break;

	case 2:
	    obj_lap_lt = &obj2;
	    obj_lap_rt = &obj2;
	    obj_man_lt = &obj2;
	    obj_man_rt = &obj2;
	    obj_std_lt = &obj1;
	    obj_std_rt = &obj1;
	    break;

	case 3:
	    obj_lap_lt = &obj1;
	    obj_lap_rt = &obj2;
	    obj_man_lt = &obj1;
	    obj_man_rt = &obj2;
	    obj_std_lt = &obj1;
	    obj_std_rt = &obj2;
	    break;

	default:
	    return;
    }

    opt_auto_add = true;

    List_t *glist = kmp->dlist + KMP_GOBJ;
    const kmp_ckpt_entry_t *pt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;
    uint i, n = kmp->dlist[KMP_CKPT].used;
    for ( i = 0; i < n; i++, pt++ )
    {
	double direction, factor = IS_M1(pt->mode) ? 1.0 : 3.0;
	WidthAndDirection(0,&direction,true,pt->left,pt->right);

	const struct obj_t *obj;
	obj = IS_M1(pt->mode) ? obj_std_lt : pt->mode ? obj_man_lt : obj_lap_lt;
	if (obj)
	{
	    kmp_gobj_entry_t *go = AppendList(glist);
	    go->obj_id		= obj->object;
	    go->position[0]	= pt->left[0];
	    go->position[1]	= obj->base;
	    go->position[2]	= pt->left[1];
	    go->rotation[1]	= direction;
	    go->scale[0]	= obj->scale.x * factor;
	    go->scale[1]	= obj->scale.y;
	    go->scale[2]	= obj->scale.z * factor;
	    go->route_id	= (typeof(go->route_id))~0;
	    go->pflags		= 0x3f;
	}

	obj = IS_M1(pt->mode) ? obj_std_rt : pt->mode ? obj_man_rt : obj_lap_rt;
	if (obj)
	{
	    kmp_gobj_entry_t *go = AppendList(glist);
	    go->obj_id		= obj->object;
	    go->position[0]	= pt->right[0];
	    go->position[1]	= obj->base;
	    go->position[2]	= pt->right[1];
	    go->rotation[1]	= direction;
	    go->scale[0]	= obj->scale.x * factor;
	    go->scale[1]	= obj->scale.y;
	    go->scale[2]	= obj->scale.z * factor;
	    go->route_id	= (typeof(go->route_id))~0;
	    go->pflags		= 0x3f;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void SnapToENPT
(
    kmp_t		* kmp,		// KMP data structure
    uint		sect		// KMP section index
)
{
    const Var_t * add2;  // not NULL: enter 2D-full mode && value to add
    const Var_t	* add1;	 // not NULL: enter 1D-full mode && value to add
    const Var_t	* hadd;  // not NULL: enter horiz-mode && value to add
    const kmp_flag_t *kf = 0;

    switch(sect)
    {
	case KMP_JGPT:
	    add2	= &kmp->jgpt_snap2;
	    add1	= &kmp->jgpt_snap;
	    hadd	= &kmp->jgpt_hsnap;
	    kf		= &kmp->jgpt_flag;
	    break;

	case KMP_CNPT:
	    add2	= &kmp->cnpt_snap2;
	    add1	= &kmp->cnpt_snap;
	    hadd	= &kmp->cnpt_hsnap;
	    break;

	case KMP_MSPT:
	    add2	= &kmp->mspt_snap2;
	    add1	= &kmp->mspt_snap;
	    hadd	= &kmp->mspt_hsnap;
	    break;

	default:
	    ASSERT(0);
	    return;
    }

    const Var_t *add;
    enum { MODE2, MODE1, HMODE } mode;

    if ( add2 && add2->mode != VAR_UNSET )
    {
	mode = MODE2;
	add = add2;
    }
    else if ( add1 && add1->mode != VAR_UNSET )
    {
	mode = MODE1;
	add = add1;
    }
    else if ( hadd && hadd->mode != VAR_UNSET )
    {
	mode = HMODE;
	add = hadd;
    }
    else
	return;

    PRINT("SnapToENPT(%s) mode=%d\n",kmp_section_name[sect].name1,mode);

    const uint n_enpt = kmp->dlist[KMP_ENPT].used;
    if (!n_enpt)
	return;
    const kmp_enpt_entry_t *enpt1 = (kmp_enpt_entry_t*)kmp->dlist[KMP_ENPT].list;

    kmp_jgpt_entry_t *pt = (kmp_jgpt_entry_t*)kmp->dlist[sect].list;
    kmp_jgpt_entry_t *pt_end = pt + kmp->dlist[sect].used;

    uint j;
    for ( j = 0; pt < pt_end; j++, pt++ )
    {
	if ( kf && j < KMP_MAX_POINT && !( kf->flags[j] & KPFL_SNAP ) )
	    continue;

	int i, enpt_idx = -1;
	double min_dist = 0.0;
	const kmp_enpt_entry_t *enpt;
	for ( i = 0, enpt = enpt1; i < n_enpt; i++, enpt++ )
	{
	    if ( i < KMP_MAX_POINT && !( kmp->enpt_flag.flags[i] & KPFL_SNAP ))
		continue;

	    const double dx = pt->position[0] - enpt->position[0];
	    const double dy = pt->position[1] - enpt->position[1];
	    const double dz = pt->position[2] - enpt->position[2];
	    const double dist = dx*dx + dy*dy + dz*dz;
	    if ( enpt_idx < 0 || min_dist > dist )
	    {
		min_dist = dist;
		enpt_idx = i;
	    }
	}
	if ( enpt_idx < 0 )
	    break; // no usable ENPT found -> abort

	uint next_idx = GetNextPointKMP(kmp,KMP_ENPT,enpt_idx,-1,false,0,0,0,0,0);
	DASSERT( next_idx < n_enpt );

	enpt = enpt1 + enpt_idx;

	DEFINE_VAR(var);
	var.mode = VAR_VECTOR;
	var.x = enpt->position[0];
	var.y = mode == HMODE ? pt->position[1] : enpt->position[1];
	var.z = enpt->position[2];
	DASSERT(add);
	CalcExpr(&var,add,OPI_ADD,0);
	pt->position[0] = var.x;
	pt->position[1] = var.y;
	pt->position[2] = var.z;

	const kmp_enpt_entry_t *next = enpt1 + next_idx;
	if ( mode == MODE2 )
	{
	    double3 p1, p2;
	    p1.x = enpt->position[0];
	    p1.y = enpt->position[1];
	    p1.z = enpt->position[2];
	    p2.x = next->position[0];
	    p2.y = next->position[1];
	    p2.z = next->position[2];
	    double3 dir = CalcDirection3D(&p1,&p2);
	    pt->rotation[0] = dir.x;
	    pt->rotation[1] = dir.y;
	    pt->rotation[2] = dir.z;
	}
	else
	{
	    pt->rotation[0] = pt->rotation[2] = 0.0;
	    pt->rotation[1] = atan2( next->position[0] - enpt->position[0],
				 next->position[2] - enpt->position[2] ) * (180/M_PI);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static int FindNearestJGPT2
(
    kmp_t	*kmp,		// valid KMP
    double	x,		// x-coordinate
    double	z,		// z-coordinate
    double	*res_dist2	// not NULL: store distance^2 here
)
{
    DASSERT(kmp);

    int found = -1;
    double max_dist2 = 1e30;

    const uint n_jgpt = kmp->dlist[KMP_JGPT].used;
    kmp_jgpt_entry_t *jgpt = (kmp_jgpt_entry_t*)kmp->dlist[KMP_JGPT].list;

    uint i;
    for ( i = 0; i < n_jgpt; i++, jgpt++ )
    {
	const double dx = x - jgpt->position[0];
	const double dz = z - jgpt->position[2];
	const double dist2 = dx*dx + dz*dz;
	if ( max_dist2 > dist2 )
	{
	    max_dist2 = dist2;
	    found = i;
	}
    }

    if ( res_dist2 )
	*res_dist2 = max_dist2;
    return found;
}

///////////////////////////////////////////////////////////////////////////////

static int FindNearestJGPT3
(
    kmp_t		*kmp,		// valid KMP
    const float32	*pos,		// 3d coordinates
    double		*res_dist2	// not NULL: store distance^2 here
)
{
    DASSERT(kmp);
    DASSERT(pos);

    int found = -1;
    double max_dist2 = 1e30;

    const uint n_jgpt = kmp->dlist[KMP_JGPT].used;
    kmp_jgpt_entry_t *jgpt = (kmp_jgpt_entry_t*)kmp->dlist[KMP_JGPT].list;

    uint i;
    for ( i = 0; i < n_jgpt; i++, jgpt++ )
    {
	double3 d;
	d.x = pos[0] - jgpt->position[0];
	d.y = pos[1] - jgpt->position[1];
	d.z = pos[2] - jgpt->position[2];
	const double dist2 = LengthSqare3(d);
	if ( max_dist2 > dist2 )
	{
	    max_dist2 = dist2;
	    found = i;
	}
    }

    if ( res_dist2 )
	*res_dist2 = max_dist2;
    return found;
}

///////////////////////////////////////////////////////////////////////////////

static void AutoRespawn ( kmp_t * kmp )
{
    DASSERT(kmp);


    //--- auto add respawn points

    const uint min_rm = kmp->jgpt_auto_remove > 1 ? 0 : kmp->dlist[KMP_JGPT].used;
    const int sect = kmp->jgpt_auto_define - 1;
    if ( sect == KMP_ENPT || sect == KMP_ITPT )
    {
	const uint n = kmp->dlist[sect].used;
	const kmp_enpt_entry_t *pt0 = (kmp_enpt_entry_t*)kmp->dlist[sect].list;
	const kmp_enpt_entry_t *pt = pt0;
	const kmp_flag_t *kf = sect == KMP_ENPT ? &kmp->enpt_flag : &kmp->itpt_flag;

	uint i;
	for ( i = 0; i < n && kmp->dlist[KMP_JGPT].used < KMP_MAX_POINT; i++, pt++ )
	{
	    if ( i < KMP_MAX_POINT && !( kf->flags[i] & KPFL_JGPT ) )
		continue;

	    double dist2;
	    FindNearestJGPT3(kmp,pt->position,&dist2);
	    if ( dist2 > kmp->jgpt_auto_distance )
	    {
		kmp_jgpt_entry_t * entry = AppendList(kmp->dlist+KMP_JGPT);
		entry->position[0] = pt->position[0] + kmp->jgpt_auto_adjust.x;
		entry->position[1] = pt->position[1] + kmp->jgpt_auto_adjust.y;
		entry->position[2] = pt->position[2] + kmp->jgpt_auto_adjust.z;
		entry->id = kmp->dlist[KMP_JGPT].used - 1;
		entry->effect = M1(entry->effect);

		uint next_idx = GetNextPointKMP(kmp,KMP_ENPT,i,-1,false,0,0,0,0,0);
		DASSERT( next_idx < n );
		const kmp_enpt_entry_t *next = pt0 + next_idx;

		double3 p1, p2;
		p1.x = pt->position[0];
		p1.y = pt->position[1];
		p1.z = pt->position[2];
		p2.x = next->position[0];
		p2.y = next->position[1];
		p2.z = next->position[2];
		double3 dir = CalcDirection3D(&p1,&p2);
		entry->rotation[0] = dir.x;
		entry->rotation[1] = dir.y;
		entry->rotation[2] = dir.z;
	    }
	}
    }


    //--- auto connect check points to respawn points

    u8 jgpt_used[KMP_MAX_POINT];
    memset(jgpt_used,0,sizeof(jgpt_used));

    const uint n_ckpt	= kmp->dlist[KMP_CKPT].used < KMP_MAX_POINT
			? kmp->dlist[KMP_CKPT].used : KMP_MAX_POINT;
    kmp_ckpt_entry_t *ckpt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;
    uint i;
    for ( i = 0; i < n_ckpt; i++, ckpt++ )
    {
	if ( kmp->ckpt_auto_respawn[i] )
	{
	    const double x = ( ckpt->left[0] + ckpt->right[0] ) / 2;
	    const double z = ( ckpt->left[1] + ckpt->right[1] ) / 2;
	    ckpt->respawn = FindNearestJGPT2(kmp,x,z,0);
	}
	if ( (uint)ckpt->respawn < KMP_MAX_POINT )
	    jgpt_used[ckpt->respawn] = 1;
    }


    //--- remove unused respawn points

    const uint n_jgpt = kmp->dlist[KMP_JGPT].used;
    if ( kmp->jgpt_auto_remove > 0 && n_jgpt > min_rm )
    {
	PRINT("--> JGPT: auto remove: %u..%u\n",min_rm,n_jgpt-1);

	u8 map[KMP_MAX_POINT];
	memset(map,0,sizeof(map));

	kmp_jgpt_entry_t *base = (kmp_jgpt_entry_t*)kmp->dlist[KMP_JGPT].list;
	kmp_jgpt_entry_t *src = base, *dest = base;

	uint i, new_idx = 0;
	for ( i = 0; i < n_jgpt; i++, src++ )
	{
	    if (jgpt_used[i])
	    {
		new_idx = dest - base;
		memcpy(dest,src,sizeof(*dest));
		dest++;
	    }
	    map[i] = new_idx;
	}
	kmp->dlist[KMP_JGPT].used = dest - base;
	//HEXDUMP16(0,0,map,sizeof(map));

	kmp_ckpt_entry_t *ckpt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;
	for ( i = 0; i < n_ckpt; i++, ckpt++ )
	    ckpt->respawn = map[ckpt->respawn];
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		   ScanTextKMP(): reference KCL		///////////////
///////////////////////////////////////////////////////////////////////////////

static void DefineReferenceKCLbyKMP ( kmp_t *kmp )
{
    kcl_ref_prio = 0;

    if ( kmp && kmp->enable_fall_kcl )
    {
	if (kmp->fall_kcl)
	{
	    ResetKCL(kmp->fall_kcl);
	    FREE(kmp->fall_kcl);
	    kmp->fall_kcl = 0;
	}

	DrawKCL_t dk;
	memset(&dk,0,sizeof(dk));

	const draw_mode_t pflags = ( KMP_DRAW & DMD_M_PFLAGS ) >> DMD_SHIFT_PFLAGS;

	uint n = kmp->dlist[KMP_GOBJ].used;
	PRINT("\e[36;1m### DefineReferenceKCLbyKMP(), N(GOBJ) = %u\e[0m\n",n);
	const kmp_gobj_entry_t *op = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	for ( ; n > 0; n--, op++ )
	{
	    if ( op->obj_id < N_KMP_GOBJ
		&& ObjectInfo[op->obj_id].flags & OBF_ROAD
		&& ( !pflags || pflags & op->pflags )
	       )
	    {
		if (!kmp->fall_kcl)
		{
		    kcl_t *ref = LoadReferenceKCL();
		    PRINT("\e[36;1m### CREATE KMP, kcl_ref=%d\e[0m\n",ref!=0);
		    if (!ref)
			break;

		    kmp->fall_kcl = MALLOC(sizeof(kcl_t));
		    CopyKCL(kmp->fall_kcl,true,ref);
		    kmp->fall_kcl->silent_octree = true;
		    kmp->fall_kcl->fast = true;
		    dk.kcl = dk.draw = kcl_ref_prio = kmp->fall_kcl;
		    dk.kmp = kmp;
		    dk.detailed = dk.as_road = true;
		}
		AddObject2KCL(&dk,op);
	    }
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  ScanTextKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextKMP
(
    kmp_t		* kmp,		// KMP data structure
    bool		init_kmp,	// true: initialize 'kmp' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    PRINT("ScanTextKMP(init=%d)\n",init_kmp);
    KMP_ACTION_LOG(kmp,false,"ScanTextKMP() %s\n",kmp->fname);

    DASSERT(kmp);
    if (init_kmp)
	InitializeKMP(kmp);
    else if (kmp->fname)
	DefineAutoLoadReferenceKCL(kmp->fname,".kmp.txt");

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,kmp->fname,kmp->revision);
    si.predef = SetupVarsKMP();

    enumError max_err = ERR_OK;
    kmp->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN KMP SECTIONS, PASS%u ...\n",kmp->is_pass2+1);

	max_err = ERR_OK;
	si.kmp = kmp;
	si.no_warn = !kmp->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", kmp->is_pass2+1 );

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
	    ResetLocalVarsSI(&si,kmp->revision);

	    si.cur_file->ptr++;
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    noPRINT("--> pass=%u: #%04u: %s\n",kmp->is_pass2+1,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,kmp_section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    noPRINT("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case KMP_KTPT:
		    err = ScanTextKTPT(kmp,&si);
		    break;

		case KMP_ENPT:
		    err = ScanText_PT_PH( kmp, &si, KMP_ENPT );
		    break;

		case KMP_ITPT:
		    err = ScanText_PT_PH( kmp, &si, KMP_ITPT );
		    break;

		case KMP_CKPT:
		    err = ScanTextCKPT(kmp,&si);
		    break;

		case KMP_GOBJ:
		    err = ScanTextGOBJ(kmp,&si);
		    break;

		case KMP_POTI:
		    err = ScanTextPOTI(kmp,&si);
		    break;

		case KMP_AREA:
		    err = ScanTextAREA(kmp,&si);
		    break;

		case KMP_CAME:
		    err = ScanTextCAME(kmp,&si);
		    break;

		case KMP_JGPT:
		    err = ScanTextPT2(kmp,&si,KMP_JGPT);
		    break;

		case KMP_CNPT:
		    err = ScanTextPT2(kmp,&si,KMP_CNPT);
		    break;

		case KMP_MSPT:
		    err = ScanTextPT2(kmp,&si,KMP_MSPT);
		    break;

		case KMP_STGI:
		    err = ScanTextSTGI(kmp,&si);
		    break;

		case KMP_N_SECT: // alias for [SETUP]
		    err = ScanTextSETUP(kmp,&si);
		    break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (kmp->is_pass2)
	    break;

	kmp->is_pass2 = true;

	DefineReferenceKCLbyKMP(kmp);
	RestartSI(&si);
	ClearSectionsKMP(kmp);
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextKMP) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

    SnapToENPT(kmp,KMP_JGPT);
    SnapToENPT(kmp,KMP_CNPT);
    SnapToENPT(kmp,KMP_MSPT);

    AutoRespawn(kmp);

    AddEnemyRouteObjects(kmp,KMP_ENPT,&kmp->rtobj_enpt);
    AddEnemyRouteObjects(kmp,KMP_ITPT,&kmp->rtobj_itpt);
    AddJugemRouteObjects(kmp,KMP_JGPT,&kmp->rtobj_jgpt);
    AddJugemRouteObjects(kmp,KMP_CNPT,&kmp->rtobj_cnpt);
    AddJugemRouteObjects(kmp,KMP_MSPT,&kmp->rtobj_mspt);
    AddRouteObjects(kmp);
    AddCheckpointObject(kmp);

    if (opt_generic)
    {
	uint sect;
	for ( sect = 0; sect < KMP_N_SECT; sect++ )
	{
	    ResetIL(kmp->index+sect);
	    FillIL(kmp->index+sect,kmp->dlist[sect].used);
	}
    }

    kmp->battle_mode = CheckBattleModeKMP(kmp);
    DefineReferenceKCLbyKMP(0);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTextKMP() helpers		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[print_kmp_group_t]]

typedef struct print_kmp_group_t
{
    int				index;		// current group index
    int				max;		// max group index
    const kmp_enph_entry_t	*ph;		// pointer to next ph element
    const kmp_group_name_t	*gname;		// list with names

} print_kmp_group_t;

//-----------------------------------------------------------------------------

static void init_print_kmp_group
(
    print_kmp_group_t		* grp,		// group element, will be initialized
    const kmp_t			* kmp,		// KMP data structure
    uint			sect_ph,	// related ph section index, may be invalid
    const kmp_group_name_t	* gname		// list with names
)
{
    DASSERT(grp);
    DASSERT(kmp);

    memset(grp,0,sizeof(*grp));

    if ( sect_ph < KMP_N_SECT )
    {
	grp->max   = kmp->dlist[sect_ph].used;
	grp->ph    = (kmp_enph_entry_t*)kmp->dlist[sect_ph].list;
	grp->gname = gname;
    }
}

//-----------------------------------------------------------------------------

static ccp print_kmp_group
(
    File_t		* F,		// output file
    const kmp_t		* kmp,		// KMP data structure
    const kmp_ph_t	* ph,		// related PH info
    print_kmp_group_t	* grp,		// group element, will be initialized
    uint		index,		// index of next element
    ccp			eol_text,	// text, printed to close previous line
    bool		check_eline	// true: print cross ref of GOBJ/ELINE_CONTROL
)
{
    DASSERT(F);
    DASSERT(kmp);
    DASSERT(ph);
    DASSERT(grp);

    if ( grp->index < grp->max && index == grp->ph->pt_start )
    {
	if (eol_text)
	{
	    fputs(eol_text,F->f);
	    eol_text = 0;
	}

	const uint idx = grp->index++;
	const u8 *ptr, *end;

	fprintf(F->f,"\r\n # zero based index: %u,  prev:", idx );
	for ( ptr = grp->ph->prev, end = ptr + KMP_MAX_PH_LINK; ptr < end; ptr++ )
	    if ( *ptr != 0xff )
		fprintf(F->f," %u",*ptr);

	fprintf(F->f,",  next:");
	for ( ptr = grp->ph->next, end = ptr + KMP_MAX_PH_LINK; ptr < end; ptr++ )
	    if ( *ptr != 0xff )
		fprintf(F->f," %u",*ptr);


	//--- $GROUP

	fprintf(F->f,"\r\n $GROUP %s,  next:", grp->gname[idx] );

	for ( ptr = grp->ph->next, end = ptr + KMP_MAX_PH_LINK; ptr < end; )
	{
	    const u8 *start = ptr;
	    const u8 code = *ptr++;
	    while ( ptr < end && *ptr == code )
		ptr++;
	    if ( code != 0xff )
	    {
		const uint n = ptr - start;
		if ( n > 1 )
		    fprintf(F->f," %s*%u",grp->gname[code],n);
		else
		    fprintf(F->f," %s",grp->gname[code]);
	    }
	}


	//--- $PREV

 // [[auto-prev]]
	bool print_prev = ph->ac_mode != KMP_AC_PREV && ph->ac_mode != KMP_AC_AUTO_PREV
			|| ph->ac_flags & KMP_ACF_PR_PREV
			|| KMP_MODE & KMPMD_PR_PREV;

	if ( !print_prev && idx < KMP_MAX_LINFO )
	{
	    SetupRouteLinksKMP(kmp,ph->sect_ph,false);
	    DASSERT(kmp->linfo);
	    DASSERT(kmp->linfo_sect==ph->sect_ph);

	    uint i;
	    kmp_linfo_mode *ptr;
	    for ( i = 0, ptr = kmp->linfo->list[idx]; i < kmp->linfo->used; i++ )
	    {
		const kmp_linfo_mode md = *ptr++;
		if ( !!(md & LINFO_PREV) == !(md & LINFO_BY_NEXT) )
		{
		    print_prev = true;
		    break;
		}
	    }
	}

	if (print_prev)
	{
	    fprintf(F->f,"\r\n $PREV:");

	    for ( ptr = grp->ph->prev, end = ptr + KMP_MAX_PH_LINK; ptr < end; )
	    {
		const u8 *start = ptr;
		const u8 code = *ptr++;
		while ( ptr < end && *ptr == code )
		    ptr++;
		if ( code != 0xff )
		{
		    const uint n = ptr - start;
		    if ( n > 1 )
			fprintf(F->f," %s*%u",grp->gname[code],n);
		    else
			fprintf(F->f," %s",grp->gname[code]);
		}
	    }
	}



	//--- $SETTINGS

	if ( idx < KMP_MAX_GROUP && grp->ph )
	{
	    const kmp_gopt2_t *go2 = ph->gopt + idx;
	    if ( ph->show_options || go2->setting[0] || go2->setting[1]
			|| go2->rtype_user || go2->rtype_active == KMP_RT_DISPATCH )
	    {
		const KeywordTab_t *key = GetKewordById(kmp_rtype_name,go2->rtype_user);
		fprintf(F->f, "\r\n $SETTINGS: %u 0x%02x %s",
				go2->setting[0], go2->setting[1],
				key ? key->name1 : "AUTO" );
		if ( go2->rtype_user != go2->rtype_active )
		{
		    const KeywordTab_t *key = GetKewordById(kmp_rtype_name,go2->rtype_active);
		    if (key)
			fprintf(F->f, "  # >%s",key->name1);
		}
	    }

	    if ( ph->show_options )
	    {
#if 0 // [[ac]]
		fprintf(F->f,
		    "\r\n $CONNECT-PREV: %3d %3d %3d  # class prio min"
		    "\r\n $CONNECT-NEXT: %3d %3d %3d",
		    go2->prev.cls, go2->prev.prio, go2->prev.min,
		    go2->next.cls, go2->next.prio, go2->next.min );
#endif
	    }
	}


	//--- ELINE_CONTROL

	if (check_eline)
	{
	    uint gidx;
	    for ( gidx = 0; gidx < kmp->dlist[KMP_GOBJ].used; gidx++ )
	    {
		const kmp_gobj_entry_t *op = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list + gidx;
		if ( op->obj_id == GOBJ_ELINE_CONTROL && op->setting[7] == idx )
		    fprintf(F->f,
			"\r\n  # GOBJ/ELINE_CONTROL #%u links to this group:"
			" enter=%d, sec=%d, id=%d, next=%d",
			gidx, op->setting[0], op->setting[1], op->setting[2], op->setting[3] );
	    }
	}

	fputs("\r\n\r\n",F->f);
	grp->ph++;
    }

    return eol_text;
}

///////////////////////////////////////////////////////////////////////////////

static ccp print_kmp_flags ( kmp_pt_flag_t flags, kmp_pt_flag_t pmask )
{
    if (!opt_kmp_flags)
	return EmptyString;

    static char buf[4];

    char *d = buf;
    if ( pmask & KPFL_FALL )	*d++ =  flags & KPFL_FALL	? 'F' : '-';
    if ( pmask & KPFL_SNAP )	*d++ =  flags & KPFL_SNAP	? 'S' : '-';
    if ( pmask & KPFL_JGPT )	*d++ =  flags & KPFL_JGPT	? 'J' : '-';

    DASSERT( d < buf + sizeof(buf) );
    *d = 0;
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static void print_kmp_pt
(
    File_t			* F,		// output file
    const kmp_t			* kmp,		// KMP data structure
    uint			sect_pt		// KMP PT section index
)
{
    DASSERT(F);
    DASSERT(kmp);
    DASSERT( sect_pt == KMP_ENPT || sect_pt == KMP_ITPT );

    //--- section setup

    char buf[30];
    uint sect_ph;
    const kmp_ph_t *ph;
    const kmp_flag_t *kf;
    kmp_pt_flag_t flag_mask;
    uint show_route;

    if ( sect_pt == KMP_ENPT )
    {
	sect_ph		= KMP_ENPH;
	ph		= &kmp->enph;
	kf		= &kmp->enpt_flag;
	flag_mask	= KPFL_M_ENPT;
	show_route	= 1 << SHRT_ENPT;
    }
    else
    {
	sect_ph		= KMP_ITPH;
	ph		= &kmp->itph;
	kf		= &kmp->itpt_flag;
	flag_mask	= KPFL_M_ITPT;
	show_route	= 1 << SHRT_ITPT;
    }

    ccp pt_name = kmp_section_name[sect_pt].name1;
    ccp ph_name = kmp_section_name[sect_ph].name1;


    //--- print data

    fprintf(F->f,text_kmp_pt_info_cr,
		REVISION_NUM,
		pt_name, kmp->value[sect_pt],	// @%s-HEAD-VALUE = %#x
		ph_name, kmp->value[sect_ph],	// @%s-HEAD-VALUE = %#x
		show_route,
		opt_export_flags ? opt_export_flags>0 : kf->export );

    if ( opt_route_options > OFFON_OFF )
    {
	ccp ac_mode, ac_fix, ac_pr_prev;

	switch ( ph->ac_flags & KMP_ACF_FIX )
	{
	    case KMP_ACF_FIX_PREV:	ac_fix = " | ACF$FIX_PREV"; break;
	    case KMP_ACF_FIX_NEXT:	ac_fix = " | ACF$FIX_NEXT"; break;
	    case KMP_ACF_FIX:		ac_fix = " | ACF$FIX"; break;
	    default:			ac_fix = ""; break;
	}

	ac_pr_prev = ph->ac_flags & KMP_ACF_PR_PREV ? " | ACF$PR_PREV" : "";

	switch( ph->ac_mode )
	{
	    case KMP_AC_OFF:		ac_mode = "AC$OFF"; break;
	    case KMP_AC_PREV:		ac_mode = "AC$PREV"; break;
	    case KMP_AC_AUTO_PREV:	ac_mode = "AC$AUTO_PREV"; break;
	    case KMP_AC_DISPATCH:	ac_mode = "AC$DISPATCH"; break;

	    default: // should never happen
		snprintf(buf,sizeof(buf),"0x%0x",ph->ac_mode|ph->ac_flags);
		ac_mode    = buf;
		ac_fix     = "";
		ac_pr_prev = "";
	}

	if ( sect_pt == KMP_ENPT )
	    fprintf(F->f,text_kmp_enpt_ropt_cr, ph->show_options, ac_mode,ac_fix,ac_pr_prev );
	else
	    fprintf(F->f,text_kmp_itpt_ropt_cr, ph->show_options, ac_mode,ac_fix,ac_pr_prev );
    }

    if ( ph->mask[0] != 0xffff || ph->mask[1] != 0xffff )
	fprintf(F->f,text_kmp_pt_mask_cr, ph->mask[0], ph->mask[1] );

    fprintf(F->f, text_kmp_pt_head_cr, pt_name );

    const uint n = kmp->dlist[sect_pt].used;
    const kmp_enpt_entry_t * pt = (kmp_enpt_entry_t*)kmp->dlist[sect_pt].list;

    print_kmp_group_t grp;
    init_print_kmp_group(&grp,kmp,sect_ph,ph->gname);
    ccp eol = "";

    uint i, gi = 0;
    for ( i = 0; i < n; i++, gi++, pt++ )
    {
	eol = print_kmp_group(F,kmp,ph,&grp,i,eol,sect_pt==KMP_ENPT);
	if (!eol)
	{
	    gi = 0;
	    eol = "";
	}
	else if ( gi > 1 )
	{
	    fprintf(F->f," %7.2f",
		AngleVector(pt[-2].position,pt[-1].position,pt->position,true) );
	}

	fprintf(F->f,"%s%4u  %11.3f %11.3f %11.3f  %7.3f  %#6x %#6x",
		eol, i,
		pt->position[0], pt->position[1], pt->position[2],
		pt->scale,
		pt->prop[0], pt->prop[1] );
	eol = "\r\n";

	const kmp_pt_flag_t flags = i < KMP_MAX_POINT ? kf->flags[i] : kf->mask;
	fprintf(F->f,"  %-3s",print_kmp_flags(flags,flag_mask));

	if ( gi > 0 )
	    fprintf(F->f," # %9.3f", LengthF(pt[-1].position,pt->position) );
    }
    fputs(eol,F->f);


    //----- list class names -----

    if ( KMP_MODE & KMPMD_DUMP_CLASS && ph->class_name.used > KCLS_USER )
    {
	fprintf(F->f,"\r\n#----- List of %u classes -----\r\n#\r\n",
			ph->class_name.used);

	ParamFieldItem_t *ptr = ph->class_name.field, *end;
	for ( end = ptr + ph->class_name.used; ptr < end; ptr++ )
	    fprintf(F->f,"#%4d  %s\r\n",ptr->num,ptr->key);
    }


    //----- list oneway links -----

    if ( KMP_MODE & KMPMD_DUMP_ONEWAY && ph->oneway_used )
    {
	fprintf(F->f,"\r\n#----- List of %u oneway link%s -----\r\n#",
			ph->oneway_used, ph->oneway_used==1 ? "" : "s" );

	SortOnewayPH(ph);

	uint max_fw = 1;
	u16 *ptr, *end = ph->oneway_list + ph->oneway_used;
	for ( ptr = ph->oneway_list; ptr < end; ptr++ )
	{
	    const uint fw = strlen(grp.gname[*ptr>>8]);
	    if ( max_fw < fw )
		max_fw = fw;
	}

	uint prev = ~0;
	for ( ptr = ph->oneway_list; ptr < end; ptr++ )
	{
	    uint from =  *ptr>>8;
	    if ( prev != from )
	    {
		prev = from;
		fprintf(F->f,"\r\n#  %*s :",max_fw,grp.gname[from]);
	    }
	    fprintf(F->f," %s",grp.gname[*ptr&0xff]);
	}
	fputs("\r\n",F->f);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void print_kmp_ph
(
    File_t			* F,		// output file
    const kmp_t			* kmp,		// KMP data structure
    uint			sect_ph		// KMP section index
)
{
    DASSERT(F);
    DASSERT(kmp);
    DASSERT( sect_ph == KMP_CKPH || sect_ph == KMP_ENPH || sect_ph == KMP_ITPH );

    //--- section setup

    uint sect_pt;
    const kmp_ph_t *ph;

    if ( sect_ph == KMP_CKPH )
    {
	sect_pt	= KMP_CKPT;
	ph	= &kmp->ckph;
    }
    else if ( sect_ph == KMP_ENPH )
    {
	sect_pt	= KMP_ENPT;
	ph	= &kmp->enph;
    }
    else
    {
	sect_pt	= KMP_ITPT;
	ph	= &kmp->itph;
    }

    ccp pt_name = kmp_section_name[sect_pt].name1;
    ccp ph_name = kmp_section_name[sect_ph].name1;


    //--- print data

    fprintf(F->f,text_kmp_ph_cr,
		pt_name,
		REVISION_NUM,
		ph_name, kmp->value[sect_ph],
		ph_name );

    const uint n = kmp->dlist[sect_ph].used;
    const kmp_enph_entry_t * p = (kmp_enph_entry_t*)kmp->dlist[sect_ph].list;
    uint i;
    for ( i = 0; i < n; i++, p++ )
    {
	fprintf(F->f,"%4u  %3u %3u %3u ",
			i, p->pt_start, p->pt_start+p->pt_len-1, p->pt_len );
	DASSERT( p->prev + KMP_MAX_PH_LINK == p->next ); // assume 'prev' before 'next'
	const u8 * d = p->prev;
	uint j;
	for ( j = 0; j < 12; j++, d++ )
	{
	    if ( *d == 0xff )
		fprintf(F->f,"%s   -", j==6 ? " " : "" );
	    else
		fprintf(F->f,"%s%4u", j==6 ? " " : "", *d );
	}

	if ( i < KMP_MAX_GROUP )
	{
	    const kmp_gopt2_t *go2 = ph->gopt + i;
	 #ifdef TEST
	    fprintf(F->f," %3u 0x%02x  # %-6s %s\r\n",
		go2->setting[0], go2->setting[1], ph->gname[i],
		i < kmp->index[sect_ph].used ? kmp->index[sect_ph].list[i] : "-" );
	 #else
	    fprintf(F->f," %3u 0x%02x  # %s\r\n",
		go2->setting[0], go2->setting[1], ph->gname[i] );
	 #endif
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void print_kmp_pt2
(
    File_t		* F,		// output file
    const kmp_t		* kmp,		// KMP data structure
    uint		sect,		// KMP section index
    uint		* ref_count	// NULL or reference counter array (len=0x100)
)
{
    DASSERT(F);
    DASSERT(kmp);
    DASSERT( sect < KMP_N_SECT );

    uint i;
    const uint n = kmp->dlist[sect].used;
    const kmp_jgpt_entry_t * pt = (kmp_jgpt_entry_t*)kmp->dlist[sect].list;
    IndexList_t *il = (IndexList_t*)kmp->index + sect; // cast because remove const
    FillIL(il,n);

    bool auto_id = true;
    for ( i = 0; i < n; i++, pt++ )
	if ( pt->id != i )
	{
	    auto_id = false;
	    break;
	}

    ccp pt_name = kmp_section_name[sect].name1;
    uint show_route = sect == KMP_JGPT
				? (1<<SHRT_JGPT)
				: sect == KMP_CNPT
					? (1<<SHRT_CNPT)
					: (1<<SHRT_MSPT);
    const u8 *pt_flags = sect == KMP_JGPT ? kmp->jgpt_flag.flags : 0;

    fprintf( F->f, text_kmp_pt2_cr,
		REVISION_NUM,
		pt_name, kmp->value[sect],
		auto_id,
		show_route );

    if ( sect == KMP_JGPT )
	fprintf( F->f, text_kmp_jgpt_auto_cr, kmp->jgpt_flag.export );
    else
	fprintf( F->f, text_kmp_pt2_head_cr, pt_name );

    pt = (kmp_jgpt_entry_t*)kmp->dlist[sect].list;
    for ( i = 0; i < n; i++, pt++ )
    {
	PrintNameIL(F->f,il,i,5,5,0);
	fprintf(F->f," %11.3f %11.3f %11.3f %7.2f %7.2f %7.2f %6d %6s",
		pt->position[0], pt->position[1], pt->position[2],
		pt->rotation[0], pt->rotation[1], pt->rotation[2],
		pt->id, PrintU16M1(pt->effect) );

	if (pt_flags)
	{
	    const kmp_pt_flag_t flags
		= i < KMP_MAX_POINT ? pt_flags[i] : kmp->jgpt_flag.mask;
	    fprintf(F->f,"  %s",print_kmp_flags(flags,KPFL_M_JGPT));
	}

	fprintf(F->f,"%s\r\n", ref_count && !ref_count[i] ? "  # unused" : "" );
    }
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetObjectName ( uint obj_id )
{
    const uint bufsize = OBJ_FW_NAME + 3;
    char *buf = GetCircBuf(bufsize);

    if ( obj_id < N_KMP_GOBJ && ObjectInfo[obj_id].name )
	snprintf(buf,bufsize,"o$%s",ObjectInfo[obj_id].name);
    else
	snprintf(buf,bufsize,"0x%x",obj_id);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static uint PrintDefObjInfoHelper ( FILE *f, ccp prefix, int mode, u16 mask )
{
    DASSERT(f);

    gobj_condition_set_t gcs;
    const uint n = GetGobjConditions(&gcs,mode,mask);

    uint i;
    for ( i = 0; i < n; i++ )
    {
	const gobj_condition_t *c = gcs.cond+i;
	fprintf(f,"#    %s/0x%04x: %s\n",prefix,c->mask,c->info );
    }

    return n;
}

static void PrintDefObjInfo ( FILE *f, const kmp_gobj_entry_t * gobj )
{
    DASSERT(f);
    DASSERT(gobj);

    const u16 balloon	= gobj->setting[KMP_GMODE_BALLOON];
    const u16 coin	= gobj->setting[KMP_GMODE_COIN];
    const u16 versus	= gobj->setting[KMP_GMODE_VERSUS];
    const u16 itemrain	= gobj->setting[KMP_GMODE_ITEMRAIN];

    uint count = 0;
    if ( balloon != versus || coin != versus )
    {
	if ( balloon == coin )
	    count += PrintDefObjInfoHelper(f,"Battle",KMP_GMODE_BALLOON,balloon);
	else
	{
	    count += PrintDefObjInfoHelper(f,"Balloon",KMP_GMODE_BALLOON,balloon);
	    count += PrintDefObjInfoHelper(f,"Coin",KMP_GMODE_COIN,coin);
	}
	count += PrintDefObjInfoHelper(f,"Versus",KMP_GMODE_VERSUS,versus);
    }
    else
	count += PrintDefObjInfoHelper(f,"*",-1,versus);

    if ( versus != itemrain )
	count += PrintDefObjInfoHelper(f,"Itemrain",KMP_GMODE_ITEMRAIN,itemrain);

    if (!count)
	fputs("#    Always disabled!\n",f);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  SaveTextKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextKMP
(
    const kmp_t		* kmp,		// pointer to valid KMP
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(kmp);
    DASSERT(fname);
    PRINT("SaveTextKMP(%s,%d) mtime=%lu\n",fname,set_time,kmp->fatt.mtime.tv_sec);
    KMP_ACTION_LOG(kmp,false,"SaveTextKMP() %s\n",fname);

    char temp_buf[100];


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,kmp->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&kmp->fatt,0);


    //--- print header + syntax info

    if ( print_header && !brief_count && !export_count )
	fprintf(F.f,text_kmp_head_cr);
    else
	fprintf(F.f,
		"%s\r\n"
		"# KMP syntax & semantics: https://szs.wiimm.de/info/kmp-syntax.html\r\n"
		,KMP_TEXT_MAGIC);


    //--- var defs

    uint i, n, gi, ng, np;
    char buf1[0x100], buf2[0x20];


    //--- print info section & section counter alphabetically

    if ( brief_count || export_count )
    {
	fprintf(F.f,
		"%s[SETUP]\r\n\r\n"
		"TOOL     = %s\r\n"
		"SYSTEM2  = %s\r\n"
		"VERSION  = %s\r\n"
		"REVISION = %u\r\n"
		"DATE     = %s\r\n"
		"INFO     = \"%s\"\r\n"
		"\r\n"
		"UNKNOWN-0x0c      = %#x\r\n"
		"KCL-FALL-ADD-ROAD = 0\r\n"
		"BATTLE-MODE       = %d\r\n"
		"KMP-WIM0          = %d\r\n"
		"\r\n",
		section_sep,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		kmp->info ? kmp->info : "",
		kmp->kmp_version, kmp->battle_mode, kmp->wim0_export );
    }
    else
    {
	fprintf(F.f, text_kmp_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		kmp->info ? kmp->info : "",
		kmp->kmp_version, kmp->battle_mode, kmp->wim0_export );

	ccp last_name = "";
	for(;;)
	{
	    ccp found_name = 0;
	    uint sect, found_sect = 0;
	    for ( sect = 0; sect < KMP_N_SECT; sect++ )
	    {
		ccp name = kmp_section_name[sect].name1;
		if ( strcmp(name,last_name) > 0
		    && ( !found_name || strcmp(name,found_name) < 0 ) )
		{
		    found_name = name;
		    found_sect = sect;
		}
	    }

	    if (!found_name)
		break;

	    if ( found_sect == KMP_POTI )
		fprintf(F.f, " # N-%s = %5u,%5u\r\n",
			    found_name,
			    kmp->dlist[found_sect].used, kmp->poti_point.used );
	    else
		fprintf(F.f, " # N-%s = %5u\r\n",
			    found_name,
			    kmp->dlist[found_sect].used );
	    last_name = found_name;
	}
    }


    //--- print default settings

    fprintf(F.f, text_kmp_defaults_cr,
	GetObjectName(kmp->rtobj.obj),
	kmp->rtobj.shift.x, kmp->rtobj.shift.y, kmp->rtobj.shift.z,
	kmp->rtobj.scale.x, kmp->rtobj.scale.y, kmp->rtobj.scale.z );


    //--- print parameter support

    if (print_param)
    {
	if ( !brief_count && !export_count )
	    fwrite(text_kmp_param1_cr,1,sizeof(text_kmp_param1_cr)-1,F.f);
	else
	    fprintf(F.f,"#%.78s\r\n# Parameter support\r\n",Minus300);

	fwrite(text_kmp_param2_cr,1,sizeof(text_kmp_param2_cr)-1,F.f);

	if ( !brief_count && !export_count )
	    fwrite(text_kmp_param3_cr,1,sizeof(text_kmp_param3_cr)-1,F.f);
    }


    //--- fill index lists

    IndexList_t *ill = (IndexList_t*)kmp->index; // cast because remove const
    for ( i = 0; i < KMP_N_SECT; i++ )
	FillIL(ill+i,kmp->dlist[i].used);


    //--- print macros

    fwrite(text_kmp_macros_cr,1,sizeof(text_kmp_macros_cr)-1,F.f);


    //--- print AREA entries

    fprintf(F.f,text_kmp_area_cr,REVISION_NUM,kmp->value[KMP_AREA]);

    n = kmp->dlist[KMP_AREA].used;
    const kmp_area_entry_t * area = (kmp_area_entry_t*)kmp->dlist[KMP_AREA].list;
    for ( i = 0; i < n; i++, area++ )
    {
	*buf1 = 0;
	PrintNameIL(F.f,ill+KMP_AREA,i,6,8,0);

	fprintf(F.f,
		"%#4x %#4x  %12.3f %11.3f %11.3f  %#6x %#6x\r\n >",
		area->mode, area->type,
		 area->position[0], area->position[1], area->position[2],
		 area->setting[0], area->setting[1] );

	if ( area->type == 0x00 )
	    PrintNameIL(F.f,ill+KMP_CAME,area->dest_id,10,10,2);
	else
	    fprintf(F.f,"%#10x",area->dest_id);

	fprintf(F.f,
		" %#4x  %12.3f %11.3f %11.3f  %6s %6s %#6x\r\n"
		    " > %28.3f %11.3f %11.3f\r\n"
		"#%.78s\r\n",
		area->prio,
		 area->rotation[0], area->rotation[1], area->rotation[2],
		 PrintU8M1(area->route), PrintU8M1(area->enemy), area->unknown_2e,
		area->scale[0], area->scale[1], area->scale[2],
		Minus300 );
    }


    //--- print CAME entries

    uint ocam = kmp->value[KMP_CAME] >> 8;
    uint scam = kmp->value[KMP_CAME] & 0xff;

    fprintf(F.f,text_kmp_came_cr,REVISION_NUM,
		GetNameIL(ill+KMP_CAME,ocam),
		GetNameIL(ill+KMP_CAME,scam) );

    // find opening cameras

    n = kmp->dlist[KMP_CAME].used;
    const kmp_came_entry_t * came = (kmp_came_entry_t*)kmp->dlist[KMP_CAME].list;

    uint ocount = 0, oidx = ocam;
    memset(buf1,0,sizeof(buf1));
    if (print_param)
	while ( oidx < n && ocount < 0xff && oidx < sizeof(buf1) && !buf1[oidx] )
	{
	    buf1[oidx] = ++ocount;
	    oidx = came[oidx].next;
	}

    for ( i = 0; i < n; i++, came++ )
    {
	//--- AREA reference

	const kmp_area_entry_t * area = (kmp_area_entry_t*)kmp->dlist[KMP_AREA].list;
	uint count, idx, nn = kmp->dlist[KMP_AREA].used;
	for ( count = idx = 0; idx < nn; idx++, area++ )
	    if ( area->type == 0x00 && area->dest_id == i )
		fprintf(F.f,"%s %s",
		    count++ ? "" : "\t# AREA reference:",
		    GetNameIL(ill+KMP_AREA,idx) );
	if (count)
	    fputs("\r\n",F.f);


	//--- CAME reference

	count = 0;
	if ( i == ocam )
	    fprintf(F.f,"%s @OPENING-INDEX",
		count++ ? "" : "\t# CAME reference:" );
	if ( i == scam )
	    fprintf(F.f,"%s @SELECTION-INDEX",
		count++ ? "" : "\t# CAME reference:" );

	const kmp_came_entry_t * cref = (kmp_came_entry_t*)kmp->dlist[KMP_CAME].list;
	nn = kmp->dlist[KMP_CAME].used;
	for ( idx = 0; idx < nn; idx++, cref++ )
	    if ( cref->next == i )
		fprintf(F.f,"%s %s",
			count++ ? "" : "\t# CAME reference:",
			GetNameIL(ill+KMP_CAME,idx) );
	if (count)
	    fputs("\r\n",F.f);


	//--- CAME object

	PrintNameIL(F.f,ill+KMP_CAME,i,6,8,0);
	fprintf(F.f," %3u %7u  %11.3f %11.3f %11.3f %10.3f",
		came->type, came->came_speed,
		 came->position[0], came->position[1], came->position[2],
		 came->zoom_begin );
	const double came_time = came->time/60.0;
	if ( came_time >= 0.01 )
	    fprintf(F.f,"  # came time:%8.2fs",came_time);
	fputs("\r\n >",F.f);

	if (buf1[i])
	{
	    uint idx = (uchar)buf1[i];
	    ccp term = "\r\n >            ";
	    if ( idx == ocount )
		fprintf(F.f,
		    " ( mode$ocam == %u ? %s : mode$test ? %s : %s )%s",
		    idx, GetNameIL(ill+KMP_CAME,i),
		    GetNameIL(ill+KMP_CAME,kmp->value[KMP_CAME]>>8),
		    GetNameIL(ill+KMP_CAME,came->next), term );
	    else
		fprintf(F.f,
		    " ( mode$ocam == %u ? %s : %s )%s",
		    idx, GetNameIL(ill+KMP_CAME,i),
		    GetNameIL(ill+KMP_CAME,came->next), term );
	}
	else
	    PrintNameIL(F.f,ill+KMP_CAME,came->next,10,12,2);

	fprintf(F.f," %5u  %11.3f %11.3f %11.3f %10.3f",
		came->zoom_speed,
		 came->rotation[0], came->rotation[1], came->rotation[2],
		 came->zoom_end );
	const double zoom_length = fabs(came->zoom_begin-came->zoom_end);
	if ( zoom_length >= 0.01 && came->zoom_speed )
	    fprintf(F.f,"  # zoom time:%8.2fs",
			zoom_length / came->zoom_speed * (10.0/6.0) );

	fprintf(F.f,"\r\n > %#9x %7u  %11.3f %11.3f %11.3f",
		came->unknown_0x02, came->viewpt_speed,
		 came->viewpt_begin[0], came->viewpt_begin[1], came->viewpt_begin[2] );
	const double viewpt_len = LengthF(came->viewpt_begin,came->viewpt_end);
	if ( viewpt_len >= 0.01 && came->viewpt_speed )
	    fprintf(F.f,"             # v.pt time:%8.2fs",
			viewpt_len / came->viewpt_speed * (10.0/6.0) );
	fputs("\r\n >",F.f);

	PrintNameIL(F.f,ill+KMP_POTI,came->route,10,11,2);
	fprintf(F.f,
		" %#6x  %11.3f %11.3f %11.3f %10.3f",
		came->unknown_0x0a,
		 came->viewpt_end[0], came->viewpt_end[1], came->viewpt_end[2],
		 came->time );
	if ( viewpt_len >= 0.01 )
	    fprintf(F.f,"  # v.pt len:%9.2f",viewpt_len);
	fprintf(F.f,"\r\n#%.91s\r\n",Minus300);
    }


    //--- print CKPH entries

    if ( brief_count < 2 && !export_count )
    {
	fwrite(text_kmp_ckph_cr,1,sizeof(text_kmp_ckph_cr)-1,F.f);
	print_kmp_ph(&F,kmp,KMP_CKPH);
    }


    //--- print CKPT entries

    bool auto_next = true;
    n  = kmp->dlist[KMP_CKPT].used;
    ng = kmp->dlist[KMP_CKPH].used;
    const kmp_ckph_entry_t * ckph = (kmp_ckph_entry_t*)kmp->dlist[KMP_CKPH].list;
    for ( gi = 0; gi < ng && auto_next; gi++, ckph++ )
    {
	uint start = ckph->pt_start;
	uint end = start + ckph->pt_len - 1;
	if ( !ckph->pt_len || end >= n )
	{
	    auto_next = false;
	    break;
	}

	const kmp_ckpt_entry_t * ckpt
		= (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list + start;
	uint j;
	for ( j = start; j <= end; j++, ckpt++ )
	{
	    if (   ckpt->prev != ( j == start ? -1 : j-1 )
		|| ckpt->next != ( j == end   ? -1 : j+1 ) )
	    {
		auto_next = false;
		break;
	    }
	}
    }

    fprintf(F.f,text_kmp_ckpt_cr,
		REVISION_NUM,
		kmp->value[KMP_CKPT], kmp->value[KMP_CKPH],
		auto_next,
		ObjectInfo[RTOBJ_1].name, ObjectInfo[RTOBJ_2].name,
		RTOBJ_SCALE_X, RTOBJ_SCALE_Y, RTOBJ_SCALE_Z,
		RTOBJ_SCALE_X, RTOBJ_SCALE_Y, RTOBJ_SCALE_Z,
		RTOBJ_BASE, RTOBJ_BASE );

    const kmp_ckpt_entry_t * ckpt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;

    print_kmp_group_t grp;
    init_print_kmp_group(&grp,kmp,KMP_CKPH,kmp->ckph.gname);
    uint jgpt_count[0x100];
    memset(jgpt_count,0,sizeof(jgpt_count));

    u8 prev_mode = M1(prev_mode);
    for ( i = 0; i < n; i++, ckpt++ )
    {
	if ( print_kmp_group(&F,kmp,&kmp->ckph,&grp,i,"",false)
		&& IS_M1(ckpt->mode) && !IS_M1(prev_mode) )
	{
	    fputs("\r\n",F.f);
	}
	prev_mode = ckpt->mode;
	jgpt_count[(uint)ckpt->respawn]++;

	double width, direction;
	WidthAndDirection(&width,&direction,true,ckpt->left,ckpt->right);

	const uint indent
	    = fprintf(F.f,"%4u %11.3f %11.3f %11.3f %11.3f ",
		i,
		ckpt->left[0], ckpt->left[1],
		ckpt->right[0], ckpt->right[1] );
	PrintNameIL(F.f,ill+KMP_JGPT,ckpt->respawn,5,5,indent);
	fprintf(F.f," %3d %3d %3d # %9.3f %7.2f\r\n",
		IS_M1(ckpt->mode) ? -1 : (uint)ckpt->mode,
		IS_M1(ckpt->prev) ? -1 : (uint)ckpt->prev,
		IS_M1(ckpt->next) ? -1 : (uint)ckpt->next,
		width, direction );
    }


    //--- print CNPT entries

    fwrite(text_kmp_cnpt_cr,1,sizeof(text_kmp_cnpt_cr)-1,F.f);
    print_kmp_pt2(&F,kmp,KMP_CNPT,0);


    //--- print ENPH entries

    if ( brief_count < 2 && !export_count )
    {
	fwrite(text_kmp_enph_cr,1,sizeof(text_kmp_enph_cr)-1,F.f);
	print_kmp_ph(&F,kmp,KMP_ENPH);
    }


    //--- print ENPT entries

    fwrite(text_kmp_enpt_cr,1,sizeof(text_kmp_enpt_cr)-1,F.f);
    print_kmp_pt(&F,kmp,KMP_ENPT);


    //--- print GOBJ statistics

    n = kmp->dlist[KMP_GOBJ].used;
    fprintf(F.f,text_kmp_gobj_cr,REVISION_NUM,kmp->value[KMP_GOBJ]);

    uint min_id = 0, n_types = 0, n_defobj = 0;
    for(;;)
    {
	uint current_id = UINT_MAX, count = 0;

	const kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	for ( i = 0; i < n; i++, gobj++ )
	{
	    if ( gobj->obj_id >= min_id )
	    {
		if ( gobj->obj_id == current_id )
		    count++;
		else if ( gobj->obj_id < current_id )
		{
		    count = 1;
		    current_id = gobj->obj_id;
		}
	    }
	}

	if (!count)
	    break;

	if (IsDefinitionObjectId(current_id))
	    n_defobj += count;
	else
	{
	    const u16 relevant_id = current_id & GOBJ_M_OBJECT;
	    if ( relevant_id < N_KMP_GOBJ && ObjectInfo[relevant_id].name )
	    {
		const ObjectInfo_t *oi = ObjectInfo + relevant_id;
		fprintf(F.f,
			"##%4u %#6x  %c%-*s %s\r\n",
			count, current_id, current_id==relevant_id ? ' ' : '|',
			OBJ_FW_NAME, oi->name, oi->charact );
	    }
	    else
		fprintf(F.f,"##%4u %#6x\r\n", count, current_id );
	    n_types++;
	}
	min_id = current_id + 1;
    }

    const int nstd = n - n_defobj;
    fprintf(F.f,"##%.83s\r\n## GOBJ total: %u object%s, %u type%s",
		Minus300,
		nstd, nstd == 1 ? "" : "s",
		n_types, n_types == 1 ? "" : "s" );
    if (n_defobj)
	fprintf(F.f," + %u definition object%s",
		n_defobj, n_defobj == 1 ? "" : "s" );
    fprintf(F.f,"\r\n##%.83s\r\n\r\n",Minus300);


    //--- print GOBJ entries

    fwrite(text_kmp_gobj_tabhead_cr,1,sizeof(text_kmp_gobj_tabhead_cr)-1,F.f);

    u8 done[N_KMP_GOBJ] = {0};
    const kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
    for ( i = 0; i < n; i++, gobj++ )
    {
	ccp form1, form2;
	form1 = form2 = settings_format_xxxx.std;

	const u16 relevant_id = gobj->obj_id & (GOBJ_M_OBJECT|GOBJ_M_DEFINITION);
	const ObjectInfo_t *oi = GetObjectInfo(relevant_id);
	DASSERT(oi);

	if (oi->name)
	{
	    form1 = form2 = settings_format_uuuu.std;
	    ccp char_sep = *oi->charact ? ", " : "";

	    if (!done[relevant_id] )
	    {
		done[relevant_id] = 1;

		fprintf(F.f,
		    "## o$%s = %#x = %u%s%s\r\n",
		    oi->name, relevant_id, relevant_id,
		    char_sep, oi->charact );

		if (oi->info)
		{
		    fputs("## Description: ",F.f);
		    PutLines(F.f,0,80,13,"## ",oi->info,"\r\n");
		}

		if (oi->fname)
		    fprintf(F.f,"## File names:  %s\r\n",oi->fname);

		uint p;
		for ( p = 0; p < 8; p++ )
		    if ( oi->param[p] )
		    {
			fprintf(F.f,"## Setting #%u:  ",p+1);
			PutLines(F.f,13,80,13,"## ",oi->param[p],"\r\n");
		    }
		fprintf(F.f,"#%.84s\r\n",Minus300);
	    }

	    if ( relevant_id == gobj->obj_id )
		fprintf(F.f,"# o$%s, S",oi->name);
	    else
		fprintf(F.f,"# o$%s|%#06x, S",oi->name,gobj->obj_id&GOBJ_M_FLAGS);

	    uint p;
	    for ( p = 0; p < 8; p++ )
		fputc( oi->param[p] ? '1'+p : '-', F.f );
	    fprintf(F.f,"%s%s\r\n",char_sep, oi->charact);

	    if (oi->format1)
		form1 = oi->format1->std;
	    if (oi->format2)
		form2 = oi->format2->std;
	}

	//--- print special features

	bool print_condition = false;
	bool print_reference = false;

	kmp_ana_defobj_t ad;
	AnalyseDefObject(&ad,gobj);
	if ( ad.is_defobj )
	{
	    char usage_info_buf[40], *usage_info;
	    const uint usage = CountDefObjUsage(kmp,gobj->obj_id);
	    if (usage)
	    {
		usage_info = usage_info_buf;
		snprintf(usage_info_buf,sizeof(usage_info_buf),
			"It is used by %u regular object%s.",
			usage, usage == 1 ? "" : "s" );
	    }
	    else
		usage_info = "It is not used by any regular object.";

	    if ( ad.is_pure_defobj && !(KMP_MODE & KMPMD_FULL_DEFOBJ) )
	    {
		fprintf(F.f,
			"# Definition object type %s. Param: %s\r\n",
			ad.cmd_name, ad.par_info );
		if (!ad.is_logical)
		{
		    fputs("# Meaning of the conditions:\r\n",F.f);
		    PrintDefObjInfo(F.f,gobj);
		}
		fprintf(F.f,"# %s\r\n  $DEFOBJ-%-7s %#06x",
			usage_info,ad.cmd_name,gobj->obj_id);
		uint i;
		for ( i = ad.index_beg; i < ad.index_end; i++ )
		{
		    if (ad.is_logical)
		    {
			if (!gobj->setting[i])
			    continue;
			cond_ref_info_t cri;
			GetConditionRefInfo(&cri,gobj->setting[i],~0,true);
			fprintf(F.f,", %s",cri.cond);
			continue;
		    }
		    fprintf(F.f,", %s",GetDefObjConditionName(gobj->setting[i]));
		}
		fputs("\r\n",F.f);

		if ( IsConditionRefId(gobj->ref_id) )
		{
		    cond_ref_info_t cri;
		    GetConditionRefInfo(&cri,gobj->ref_id,gobj->obj_id,true);
		    fprintf(F.f,"  $%s %s%s%s\r\n",
			cri.cmd_suffix, cri.cond,
			cri.info[0] ? " # " : "", cri.info );
		}

		fprintf(F.f,"#%.84s\r\n",Minus300);
		continue;
	    }
	    fprintf(F.f,"# Type %#06x: Definition object!\r\n# %s\r\n",
			GOBJ_M_DEFINITION, usage_info );
	}
	else
	{
	    if ( gobj->obj_id & GOBJ_M_LECODE )
		fprintf(F.f,"# |%#x => Disabled object."
			" Conditional enabled by LE-CODE.\r\n",
			GOBJ_M_LECODE);

	    const u16 mode = GetPFlagsMode(gobj->pflags);
	    if (mode)
		fprintf(F.f,"# Extended presence flags, mode %u\r\n",mode);

	    kmp_ana_ref_t ar;
	    AnalyseRefObject(&ar,gobj);
	    if ( ar.is_condref || ar.is_condrnd || ar.is_condtest )
		print_condition = true;
	    else if (ar.is_objref)
		print_reference = true;
	    else if (ar.condref_valid)
		fprintf(F.f,
			"# Warning: Valid condition, but P-Flags mode is %u\r\n",
			ar.mode );
	    else if (ar.objref_valid)
		fprintf(F.f,
			"# Warning: Valid reference id, but P-Flags mode is %u\r\n",
			ar.mode );
	    else if ( ar.mode_valid && gobj->ref_id )
		fprintf(F.f,
			"# Warning: Invalid reference id: %#06x\r\n",
			gobj->ref_id);
	}

	//--- print object information

	snprintf(buf1,sizeof(buf1),form1,
			gobj->setting[0], gobj->setting[1],
			gobj->setting[2], gobj->setting[3] );
	switch (relevant_id)
	{
	 case GOBJ_ELINE_CONTROL:
	    {
		const int idx = IS_M1(gobj->setting[7]) ? -1 : gobj->setting[7];
		if ( (uint)idx < KMP_MAX_GROUP )
		    snprintf(temp_buf,sizeof(temp_buf),"E.%s",kmp->enph.gname[idx]);
		else
		    snprintf(temp_buf,sizeof(temp_buf),"%6d",idx);
		snprintf(buf2,sizeof(buf2),"%6u %6u %6u %6s",
			gobj->setting[4], gobj->setting[5],
			gobj->setting[6], temp_buf );
	    }
	    break;

	 default:
	    snprintf(buf2,sizeof(buf2),form2,
			gobj->setting[4], gobj->setting[5],
			gobj->setting[6], gobj->setting[7] );
	}

	PrintNameIL(F.f,ill+KMP_GOBJ,i,5,5,0);
	fprintf(F.f,
		       " %#6x %11.3f %11.3f %11.3f %s  %6s\r\n"
		  "    > %#6x %11.3f %11.3f %11.3f %s  %#6x\r\n"
		"    >        %11.3f %11.3f %11.3f\r\n",
		gobj->obj_id,
		 gobj->position[0], gobj->position[1], gobj->position[2],
		 buf1, GetNameIL(ill+KMP_POTI,gobj->route_id),
		gobj->ref_id,
		 gobj->rotation[0], gobj->rotation[1], gobj->rotation[2],
		 buf2, gobj->pflags,
		gobj->scale[0], gobj->scale[1], gobj->scale[2] );


	//--- print special definitions

	if (print_condition)
	{
	    cond_ref_info_t cri;
	    GetConditionRefInfo(&cri,gobj->ref_id,gobj->obj_id,true);
	    if (cri.is_condref)
		fprintf(F.f,"  $%s %s%s%s\r\n",
			cri.cmd_suffix, cri.cond,
			cri.info[0] ? " # " : "", cri.info );
	    PRINT0("# CR[%#x,%#x]: is=%d, found=%d, cmd=%s, xor:%x->%#x, c=%s, i=%s\n",
			gobj->ref_id, gobj->obj_id,
			cri.is_condref, cri.condref!=0, cri.cmd_suffix,
			cri.xor, gobj->ref_id^cri.xor, cri.cond, cri.info );
	}
	else if (print_reference)
	{
	    fprintf(F.f,"  $%sABLE-BY%s %#06x\r\n",
		gobj->obj_id & GOBJ_M_LECODE ? "EN" : "DIS",
		(gobj->obj_id ^ gobj->ref_id) & GOBJ_DEF_F_NOT ? "" : "-NOT",
		gobj->ref_id & ~GOBJ_DEF_F_NOT );
	}
	fprintf(F.f,"#%.84s\r\n",Minus300);
    }


    //--- print ITPH entries

    if ( brief_count < 2 && !export_count )
    {
	fwrite(text_kmp_itph_cr,1,sizeof(text_kmp_itph_cr)-1,F.f);
	print_kmp_ph(&F,kmp,KMP_ITPH);
    }


    //--- print ITPT entries

    fwrite(text_kmp_itpt_cr,1,sizeof(text_kmp_itpt_cr)-1,F.f);
    print_kmp_pt(&F,kmp,KMP_ITPT);


    //--- print JGPT entries

    fwrite(text_kmp_jgpt_cr,1,sizeof(text_kmp_jgpt_cr)-1,F.f);
    print_kmp_pt2(&F,kmp,KMP_JGPT,jgpt_count);


    //--- print KTPT entries

    fprintf(F.f,text_kmp_ktpt_cr,REVISION_NUM,kmp->value[KMP_KTPT]);

    n = kmp->dlist[KMP_KTPT].used;
    const kmp_ktpt_entry_t * ktpt = (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;
    for ( i = 0; i < n; i++, ktpt++ )
    {
	fprintf(F.f,"%4u  %11.3f %11.3f %11.3f  %7.2f %7.2f %7.2f %5d %#6x\r\n",
		i,
		ktpt->position[0], ktpt->position[1], ktpt->position[2],
		ktpt->rotation[0], ktpt->rotation[1], ktpt->rotation[2],
		ktpt->player_index, ktpt->unknown );
    }


    //--- print MSPT entries

    fwrite(text_kmp_mspt_cr,1,sizeof(text_kmp_mspt_cr)-1,F.f);
    print_kmp_pt2(&F,kmp,KMP_MSPT,0);


    //--- print POTI entries

    fprintf(F.f,text_kmp_poti_cr,REVISION_NUM,1<<SHRT_POTI);

    ng = kmp->dlist[KMP_POTI].used;
    const kmp_poti_group_t * pg = (kmp_poti_group_t*)kmp->dlist[KMP_POTI].list;
    np = kmp->poti_point.used;
    const kmp_poti_point_t * pp = (kmp_poti_point_t*)kmp->poti_point.list;

    for ( gi = i = 0; gi < ng && i < np; gi++, pg++ )
    {
	uint en = pg->n_point;
	DASSERT( en <= np-i );
	fprintf(F.f,"\r\n $ROUTE %s, settings: %u %u # section %u\r\n",
			GetNameIL(ill+KMP_POTI,gi),
			pg->smooth, pg->back, gi+1 );
	const bool is_cyclic = !pg->back;

	//--- GOBJ reference

	const kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	uint count, idx, nn = kmp->dlist[KMP_GOBJ].used;
	for ( count = idx = 0; idx < nn; idx++, gobj++ )
	    if ( gobj->route_id == (uint)gi )
	    {
		const u16 relevant_id = gobj->obj_id & GOBJ_M_OBJECT;
		fprintf(F.f,"\t# GOBJ reference: %-8s type %#6x",
			GetNameIL(ill+KMP_GOBJ,idx), gobj->obj_id );
		if ( relevant_id < N_KMP_GOBJ )
		{
		    const ObjectInfo_t *oi = ObjectInfo + relevant_id;
		    if (oi->name)
		    {
			if ( relevant_id == gobj->obj_id )
			    fprintf(F.f," = %s",oi->name);
			else
			    fprintf(F.f," = %#06x | %s",gobj->obj_id&GOBJ_M_FLAGS,oi->name);
		    }
		}
		fputs("\r\n",F.f);
	    }

	//--- CAME reference

	ccp came_ref = 0;
	const kmp_came_entry_t * came = (kmp_came_entry_t*)kmp->dlist[KMP_CAME].list;
	nn = kmp->dlist[KMP_CAME].used;
	for ( count = idx = 0; idx < nn; idx++, came++ )
	    if ( came->route == gi )
	    {
		ccp came_name = GetNameIL(ill+KMP_CAME,idx);
		if (!came_ref)
		    came_ref = came_name;
		fprintf(F.f,"%s %s",
			count++ ? "" : "\t# CAME reference:",
			came_name );
	    }

	if (came_ref)
	    fprintf(F.f,"\r\n\r\n"
		"   # @set-route-time = %s.came.time  # in 1/60 sec\r\n",came_ref);


	//--- print points

	fputs("\r\n",F.f);
	double total_len = 0.0, total_tim = 0.0;
	for ( uint ei = 0; ei < en; ei++, i++, pp++ )
	{
	    fprintf(F.f, "%4u  %11.3f %11.3f %11.3f %5u %5u  #",
		ei,
		pp->position[0], pp->position[1], pp->position[2],
		pp->speed, pp->unknown );

	    if ( ei < en-1 || is_cyclic )
	    {
		const kmp_poti_point_t * pnext = pp + 1;
		if ( ei >= en-1 )
		    pnext -= en;
		double len = LengthF(pp->position,pnext->position);
		total_len += len;
		double tim = len > 0.0 && pp->speed > 0
				? len / (pp->speed*60) : 0.0;
		if ( tim >= 0.01 && tim <= 999.99 )
		{
		    total_tim += tim;
		    fprintf(F.f," %6.2f %9.3f", tim, len );
		}
		else
		    fprintf(F.f,"      - %9.3f", len );

		if ( ei || is_cyclic )
		{
		    const kmp_poti_point_t * pprev = pp - 1;
		    if (!ei)
			pprev += en;
		    fprintf(F.f," %7.2f",
			AngleVector(pprev->position,pp->position,pnext->position,true) );
		}
	    }
	    fputs("\r\n",F.f);
	}

	if ( en > 1 )
	    fprintf(F.f,"%56s %6.2f %9.3f\r\n","#TOTAL#",total_tim,total_len);
    }


    //--- print STGI entries

    fprintf(F.f,text_kmp_stgi_cr,REVISION_NUM,kmp->value[KMP_STGI]);

    n = kmp->dlist[KMP_STGI].used;
    const kmp_stgi_entry_t * stgi = (kmp_stgi_entry_t*)kmp->dlist[KMP_STGI].list;

    for ( i = 0; i < n; i++, stgi++ )
    {
	PrintNameIL(F.f,ill+KMP_STGI,i,4,5,0);
	fprintf(F.f,"%5u %4u %5u %5u   %#10x  0x%02x %#5x %#6x %#4x  # ",
		stgi->lap_count, stgi->pole_pos,
		stgi->narrow_start, stgi->enable_lens_flare,
		stgi->flare_color,
		stgi->flare_alpha, stgi->unknown_09,
		stgi->speed_mod >> 8, stgi->speed_mod & 0xff );
	if (stgi->speed_mod)
	    fprintf(F.f,"%7.3f\r\n",SpeedMod2float(stgi->speed_mod));
	else
	    fputs("   -\r\n",F.f);
    }


    //--- terminate

    fputs(section_end,F.f);
    ResetFile(&F,set_time);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


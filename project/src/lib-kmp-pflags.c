
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

#include "lib-kmp.h"
#include "lib-szs.h"
#include "dclib-xdump.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

opt_gamemode_t opt_gamemodes = GMD_M_DEFAULT;

static const KeywordTab_t opt_gamemodes_tab[] =
{
  { 0,			"CLEAR",	"RESET",	GMD_M_ALL | GMD_F_HIDE },
  { GMD_M_DEFAULT,	"DEFAULT",	0,		GMD_M_ALL | GMD_F_HIDE },

  { GMD_STD,		"STANDARD",	"STD",		0 },
  { GMD_BALLOON,	"BALLOON",	0,		0 },
  { GMD_COIN,		"COIN",		0,		0 },
  { GMD_MD_BATTLE,	"BATTLE",	0,		0 },
  { GMD_VERSUS,		"VERSUS",	"VS",		0 },
  { GMD_ITEMRAIN,	"ITEMRAIN",	0,		0 },
  { GMD_MD_RACE,	"RACE",		0,		0 },
  { GMD_MD_ALL,		"ALL",		0,		0 },

  { GMD_NO_MODES,	"NO-MODES",	"NOMODES",	0 },
  { GMD_OFFLINE,	"OFFLINE",	0,		0 },
  { GMD_ONLINE,		"ONLINE",	0,		0 },

  { GMD_AUTO,		"AUTO",		0,		0 },
  { GMD_STD_FIRST,	"1STANDARD",	"1STD",		0 },
  { GMD_ENGINE,		"ENGINE",	0,		0 },
  { 0,			"NO-ENGINE",	"NOENGINE",	GMD_ENGINE },
  { GMD_RANDOM,		"RANDOM",	0,		0 },
  { 0,			"NO-RANDOM",	"NORANDOM",	GMD_RANDOM },
  { GMD_NO_TT,		"NO-TT",	"NOTT",		0 },
  { GMD_FORCE_TT,	"TT",		0,		0 },
  { GMD_FULL,		"FULL",		0,		0 },

  { GMD_NSORT,		"SORT",		"NSORT",	GMD_M_SORT },
  { GMD_ISORT,		"ISORT",	0,		GMD_M_SORT },
  { 0,			"NO-SORT",	"NOSORT",	GMD_M_SORT },

  { GMD_DEBUG,		"DEBUG",	0,		0 },
  { GMD_DUMP,		"DUMP",		0,		0 },

  {0,0,0,0}
};

//-----------------------------------------------------------------------------

int ScanOptGamemodes ( ccp arg )
{
    if (!arg)
	return 0;

    s64 stat = ScanKeywordList(arg,opt_gamemodes_tab,0,true,0,GMD_M_OPTIONS,
					"Option --gamemodes",ERR_SYNTAX);
    if ( stat != -1 )
    {
	opt_gamemodes = stat;
	if (!(opt_gamemodes&GMD_MD_ALL))
	    opt_gamemodes |= GMD_MD_DEFAULT;
	if (!(opt_gamemodes&GMD_FI_ALL))
	    opt_gamemodes |= GMD_FI_DEFAULT;

	// do this after evaluating GMD_MD_ALL!
	if (opt_gamemodes&GMD_STD_FIRST)
	    opt_gamemodes |= GMD_STD;

	if ( opt_gamemodes & GMD_DEBUG )
	{
	    char buf[200] = {0};
	    PrintKeywordList( buf, sizeof(buf), 0, opt_gamemodes_tab,
				    opt_gamemodes, GMD_M_DEFAULT, GMD_F_HIDE );
	    fflush(stdout);
	    fprintf(stderr,"GAMEMODES: %s\n",buf);
	    fflush(stderr);
	}
	return 0;
    }
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kmp_pflags_t			///////////////
///////////////////////////////////////////////////////////////////////////////

uint CountPresenceFlags ( const kmp_t *kmp, kmp_pflags_t *pf )
{
    DASSERT(pf);
    DASSERT(kmp);

    memset(pf,0,sizeof(*pf));

    const uint n = kmp->dlist[KMP_GOBJ].used;
    pf->n_gobj = n;

    kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
    uint i;
    for ( i = 0; i < n; i++, gobj++ )
    {
	const u16 pflags = gobj->pflags;
	const u16 std    = pflags & GOBJ_M_PF_STD;
	const u16 mode   = ( pflags & GOBJ_M_PF_MODE ) >> GOBJ_S_PF_MODE;

	if ( std == pflags )
	    pf->n_standard++;

	if ( mode > 0 )
	    pf->n_xpflags++;

	if (!pf->n_mode[mode]++)
	    pf->n_cat_mode++;

	if (!pf->n_std[std]++)
	    pf->n_cat_std++;

	if ( mode == 1 )
	{
	    if (IsConditionRefRelevantRandomId(gobj->ref_id))
		pf->n_cond_rnd++;
	    else if (IsConditionRefId(gobj->ref_id))
		pf->n_cond_ref++;

	    if (IsDefinitionObjectId(gobj->ref_id))
		pf->n_obj_ref++;
	}

	if ( IsDefinitionObjectIdBITS(gobj->obj_id) )
	    pf->n_defobj_bits++;
	else if ( IsDefinitionObjectIdOR(gobj->obj_id) )
	    pf->n_defobj_or++;
	else if ( IsDefinitionObjectIdAND(gobj->obj_id) )
	    pf->n_defobj_and++;
    }

    pf->n_defobj = pf->n_defobj_bits + pf->n_defobj_or + pf->n_defobj_and;
    return n;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////	    detect, fix and sort definition-objects	///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyseDefObject ( kmp_ana_defobj_t *ad, const kmp_gobj_entry_t *gobj )
{
    DASSERT(ad);
    DASSERT(gobj);
    memset(ad,0,sizeof(*ad));

    if (IsDefinitionObjectId(gobj->obj_id))
    {
	ad->is_defobj = true;
	ad->is_pure_defobj =
		 ( gobj->ref_id == 0 || IsConditionRefId(gobj->ref_id) )
		&& gobj->position[0]	== 0.0
		&& gobj->position[1]	== 0.0
		&& gobj->position[2]	== 0.0
		&& gobj->rotation[0]	== 0.0
		&& gobj->rotation[1]	== 0.0
		&& gobj->rotation[2]	== 0.0
		&& gobj->scale[0]	== 0.0
		&& gobj->scale[1]	== 0.0
		&& gobj->scale[2]	== 0.0
		&& gobj->route_id	== 0xffff
		&& (  gobj->pflags	== 0
		   || gobj->pflags	== GOBJ_PF_MODE1 );
    }

    if ( IsDefinitionObjectIdOR(gobj->obj_id) )
    {
	ad->cmd_name = "OR";
     logical:
	ad->par_info	= "obj_id, condition_1 [,conditions_2_8]...";
	ad->is_logical	= true;
	ad->index_beg	= 0;
	ad->index_end	= 8;
	return;
    }
    else if ( IsDefinitionObjectIdAND(gobj->obj_id) )
    {
	ad->cmd_name = "AND";
	goto logical;
    }

    u16 set0 = gobj->setting[0];
    uint idx = 1;
    while ( idx < 8 && gobj->setting[idx] == set0 )
	idx++;

    if ( idx > 2 )
    {
	ad->cmd_name = "RACE";
	ad->par_info = "obj_id, versus, itemrain";
	ad->index_beg = 2;
    }
    else
    {
	ad->cmd_name = "BATTLE";
	ad->par_info = "obj_id, balloon, coin, versus, itemrain";
	ad->index_beg = 0;
    }

    while ( idx < 8 )
    {
	const u16 set1 = gobj->setting[idx++];
	if ( set0 != set1 )
	{
	    ad->index_end = idx;
	    set0 = set1;
	}
    }

    if ( ad->index_end <= ad->index_beg )
	ad->index_end = ad->index_beg + 1;
}

///////////////////////////////////////////////////////////////////////////////

bool NormalizeDefObj ( kmp_gobj_entry_t *gobj )
{
    DASSERT(gobj);
    bool stat = false;
    if (IsDefinitionObjectId(gobj->obj_id))
    {
	kmp_gobj_entry_t temp;
	memcpy(&temp,gobj,sizeof(temp));

	gobj->position[0]	= 0.0;
	gobj->position[1]	= 0.0;
	gobj->position[2]	= 0.0;
	gobj->rotation[0]	= 0.0;
	gobj->rotation[1]	= 0.0;
	gobj->rotation[2]	= 0.0;
	gobj->scale[0]		= 0.0;
	gobj->scale[1]		= 0.0;
	gobj->scale[2]		= 0.0;
	gobj->route_id		= 0xffff;
	gobj->pflags		= 0;

	if (IsDefinitionObjectIdBITS(gobj->obj_id))
	{
	    const u16 val = gobj->setting[2];
	    gobj->setting[4] = val;
	    gobj->setting[5] = val;
	    gobj->setting[6] = val;
	    gobj->setting[7] = val;
	}

	stat = memcmp(&temp,gobj,sizeof(temp)) != 0;
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

uint CountDefObjUsage ( const kmp_t *kmp, u16 obj_id )
{
    DASSERT(kmp);

    uint i, count = 0;
    const List_t *dlist = kmp->dlist + KMP_GOBJ;
    const uint n = dlist->used;
    const kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)dlist->list;
    for ( i = 0; i < n; i++, gobj++ )
    {
	if ( (gobj->ref_id & ~GOBJ_DEF_F_NOT) == obj_id )
	{
	    kmp_ana_ref_t ar;
	    AnalyseRefObject(&ar,gobj);
	    if (ar.is_valid)
		count++;
	}
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetDefObjConditionName ( u16 cond )
{
    const gobj_cond_ref_t *gcr;
    for ( gcr = cond_ref_tab; gcr->ref_id; gcr++ )
	if ( gcr->cond_mask == cond && !(gcr->option & 4) )
	    return PrintCircBuf("C$%s",gcr->name);

    return PrintCircBuf("0x%04x",cond);
}

///////////////////////////////////////////////////////////////////////////////

bool FixDefObj
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			fix_mode	// >0: clear unused parameters
)
{
    DASSERT(kmp);
    if ( fix_mode < 1 )
	return false;

    uint i, fix_count = 0;
    List_t *dlist = kmp->dlist + KMP_GOBJ;
    kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)dlist->list;
    for ( i = 0; i < dlist->used; i++, gobj++ )
	if (NormalizeDefObj(gobj))
	    fix_count++;

    return fix_count>0;
}

///////////////////////////////////////////////////////////////////////////////

static int sort_defobj_begin
	( const kmp_gobj_entry_t *a, const kmp_gobj_entry_t *b )
{
    if (IsDefinitionObjectId(a->obj_id))
    {
	if (!IsDefinitionObjectId(b->obj_id))
	    return -1;

	return a->obj_id < b->obj_id ? -1
	     : a->obj_id > b->obj_id ? 1
	     : a < b ? -1 : 1;
    }

    if (IsDefinitionObjectId(b->obj_id))
	return 1;
    return a < b ? -1 : 1;
}

//-----------------------------------------------------------------------------

static int sort_defobj_end
	( const kmp_gobj_entry_t *a, const kmp_gobj_entry_t *b )
{
    if (IsDefinitionObjectId(a->obj_id))
    {
	if (!IsDefinitionObjectId(b->obj_id))
	    return 1;

	return a->obj_id < b->obj_id ? -1
	     : a->obj_id > b->obj_id ? 1
	     : a < b ? -1 : 1;
    }

    if (IsDefinitionObjectId(b->obj_id))
	return -1;
    return a < b ? -1 : 1;
}

//-----------------------------------------------------------------------------

bool SortDefObj
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			sort_mode	// sort if KMP_DOB_BEGIN || KMP_DOB_END
)
{
    DASSERT(kmp);
    if ( sort_mode != KMP_DOB_BEGIN && sort_mode != KMP_DOB_END )
	return false;

    List_t *dlist = kmp->dlist + KMP_GOBJ;
    const uint n = dlist->used;
    if ( n < 2 )
	return false;

    kmp_gobj_entry_t *base = (kmp_gobj_entry_t*)dlist->list;
    if ( sort_mode == KMP_DOB_BEGIN )
	qsort( base, n, sizeof(*base), (qsort_func)sort_defobj_begin );
    else
	qsort( base, n, sizeof(*base), (qsort_func)sort_defobj_end );
    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		detect, fix and sort references		///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyseRefObject ( kmp_ana_ref_t *ar, const kmp_gobj_entry_t *gobj )
{
    DASSERT(ar);
    DASSERT(gobj);
    memset(ar,0,sizeof(*ar));

    ar->mode		= GetPFlagsMode(gobj->pflags);
    ar->mode_valid	= ar->mode == 1;

    ar->condref_valid	= IsConditionRefId(gobj->ref_id);
    ar->is_condref	= ar->mode_valid && ar->condref_valid;

    ar->condeng_valid	= IsConditionRefEngineId(gobj->ref_id);
    ar->is_condeng	= ar->mode_valid && ar->condeng_valid;

    ar->condrnd_valid	= IsConditionRefRandomId(gobj->ref_id);
    ar->is_condrnd	= ar->mode_valid && ar->condrnd_valid;

    ar->condtest_valid	= IsConditionRefTestId(gobj->ref_id);
    ar->is_condtest	= ar->mode_valid && ar->condtest_valid;

    ar->objref_valid	= IsDefinitionObjectId(gobj->ref_id);
    ar->is_objref	= ar->mode_valid && ar->objref_valid;

    ar->is_valid	= ar->objref_valid || ar->is_objref;
}

///////////////////////////////////////////////////////////////////////////////

kmp_gobj_entry_t * FindRefObjectByID ( const kmp_t *kmp, u16 ref_id )
{
    DASSERT(kmp);
    ref_id &= ~GOBJ_DEF_F_NOT;
    if (!IsDefinitionObjectId(ref_id))
	return 0;

    const List_t *dlist = kmp->dlist + KMP_GOBJ;
    const uint n = dlist->used;
    kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)dlist->list;
    uint i;
    for ( i = 0; i < n; i++, gobj++ )
	if ( gobj->obj_id == ref_id )
	    return gobj;
    return 0;
}

//-----------------------------------------------------------------------------

kmp_gobj_entry_t * FindRefObject ( const kmp_t *kmp, const kmp_gobj_entry_t *gobj )
{
    DASSERT(kmp);
    DASSERT(gobj);

    kmp_ana_ref_t ar;
    AnalyseRefObject(&ar,gobj);
    return ar.is_valid ? FindRefObjectByID(kmp,gobj->ref_id) : 0;
}

///////////////////////////////////////////////////////////////////////////////

bool FixReference
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			fix_mode	// >0: fix if valid mode
					// >1: fix for all modes
)
{
    DASSERT(kmp);
    if ( fix_mode < 1 )
	return false;

    uint i, fix_count = 0;
    List_t *dlist = kmp->dlist + KMP_GOBJ;
    const uint n = dlist->used;
    kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)dlist->list;
    for ( i = 0; i < n; i++, gobj++ )
    {
	kmp_ana_ref_t ar;
	AnalyseRefObject(&ar,gobj);
	if ( !ar.is_valid && ( fix_mode > 1 || ar.mode_valid ))
	{
	    gobj->ref_id = 0;
	    fix_count++;
	}
    }
    return fix_count>0;
}

///////////////////////////////////////////////////////////////////////////////

static int sort_reference ( const u32 *a, const u32 *b )
{
    return (s32)*a - (s32)*b;
}

//-----------------------------------------------------------------------------

bool SortReference
(
    kmp_t		* kmp,		// pointer to valid KMP
    int			sort_mode	// sort if KMP_DOB_BEFORE || KMP_DOB_BEHIND
)
{
    DASSERT(kmp);
    if ( sort_mode != KMP_DOB_BEFORE && sort_mode != KMP_DOB_BEHIND )
	return false;

    List_t *dlist = kmp->dlist + KMP_GOBJ;
    const uint n = dlist->used;
    if ( n < 2 )
	return false;

    kmp_gobj_entry_t *base = (kmp_gobj_entry_t*)dlist->list;
    u32 *keys = MALLOC(n*sizeof(*keys));
    uint i;
    const kmp_gobj_entry_t *gobj = base;
    if ( sort_mode == KMP_DOB_BEFORE )
    {
	for ( i = 0; i < n; i++, gobj++ )
	{
	    const kmp_gobj_entry_t *found = FindRefObject(kmp,gobj);
	    if (!found)
		keys[i] = i << 16 | 0x8000 | i;
	    else
		keys[i] = (uint)(found-base) << 16 | i;
	}
    }
    else
    {
	for ( i = 0; i < n; i++, gobj++ )
	{
	    const kmp_gobj_entry_t *found = FindRefObject(kmp,gobj);
	    if (!found)
		keys[i] = i << 16 | i;
	    else
		keys[i] = (uint)(found-base) << 16 | 0x8000 | i;
	}
    }

    qsort ( keys, n, sizeof(*keys), (qsort_func)sort_reference );

    kmp_gobj_entry_t *temp = MALLOC(n*sizeof(*temp));
    for ( i = 0; i < n; i++ )
    {
	const kmp_gobj_entry_t *src = base + ( keys[i] & 0x7fff );
	memcpy(temp+i,src,sizeof(*temp));
    }
    memcpy(base,temp,n*sizeof(*temp));
    FREE(temp);
    FREE(keys);
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			XPF: helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

u16 GetActiveObjectId ( const kmp_gobj_entry_t *g )
{
    if ( !g || g->obj_id >= GOBJ_MIN_DEF )
	return 0;

    const u16 obj_id = g->obj_id & GOBJ_M_OBJECT;
    return obj_id < N_KMP_GOBJ
		&& ( g->pflags & GOBJ_M_PF_STD
			|| (g->pflags & GOBJ_M_PF_MODE) == GOBJ_PF_MODE1 )
	? obj_id : 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: Convert conditions to text		///////////////
///////////////////////////////////////////////////////////////////////////////

const gobj_condition_t gobj_condition[] =
{
    { 0xffff,0xff,-1,  1,4, 0,99,
		"All conditions." },
    { 0x3fff,0xfb,-1,  1,4, 0,99,
		"All conditions except reserved." },
    { 0x7fff,0x04,-1,  1,4, 0,99,
		"All conditions except reserved 2." },
    { 0x3fff,0xff,-1,  1,4, 0,99,
		"All conditions except time trial and reserved." },

    //--- offline players

    { 0x000f,0xff,-1,  1,4, 0,0,
		"Offline race with 1-4 human players." },
    { 0x0007,0xff,-1,  1,3, 0,0,
		"Offline race with 1-3 human players." },
    { 0x0003,0xff,-1,  1,2, 0,0,
		"Offline race with 1-2 human players." },
    { 0x0001,0xff, 0,  1,1, 0,0,
		"Offline race with 1 human player." },

    { 0x000e,0xff,-1,  2,4, 0,0,
		"Offline race with 2-4 human players." },
    { 0x0006,0xff,-1,  2,3, 0,0,
		"Offline race with 2-3 human players." },
    { 0x0002,0xff, 1,  2,2, 0,0,
		"Offline race with 2 human players." },

    { 0x000c,0xff,-1,  3,4, 0,0,
		"Offline race with 3-4 human players." },
    { 0x0004,0xff, 2,  3,3, 0,0,
		"Offline race with 3 human players." },
    { 0x0008,0xff, 3,  4,4, 0,0,
		"Offline race with 4 human players." },


    //--- online, 1-N players total

    { 0x3ff0,0xff,-1,  1,2, 1,99,
		"Online race with any number of players." },
    { 0x1550,0xff,-1,  1,1, 1,99,
		"Online race with 1 human player at Wii." },
    { 0x2aa0,0xff,-1,  2,2, 1,99,
		"Online race with 2 human players at Wii." },

    { 0x0ff0,0xff,-1,  1,2, 1,18,
		"Online race with 1-18 players total." },
    { 0x0550,0xff,-1,  1,1, 1,18,
		"Online race with 1-18 players total and 1 human at Wii." },
    { 0x0aa0,0xff,-1,  2,2, 1,18,
		"Online race with 1-18 players total and 2 humans at Wii." },

    { 0x03f0,0xff,-1,  1,2, 1,12,
		"Online race with 1-12 players total." },
    { 0x0150,0xff,-1,  1,1, 1,12,
		"Online race with 1-12 players total and 1 human at Wii." },
    { 0x02a0,0xff,-1,  2,2, 1,12,
		"Online race with 1-12 players total and 2 humans at Wii." },

    { 0x00f0,0xff,-1,  1,2, 1,9,
		"Online race with 1-9 players total." },
    { 0x0050,0xff,-1,  1,1, 1,9,
		"Online race with 1-9 players total and 1 human at Wii." },
    { 0x00a0,0xff,-1,  2,2, 1,9,
		"Online race with 1-9 players total and 2 humans at Wii." },

    { 0x0030,0xff,-1,  1,2, 1,9,
		"Online race with 1-6 players total." },
    { 0x0010,0xff, 4,  1,1, 1,6,
		"Online race with 1-6 players total and 1 human at Wii." },
    { 0x0020,0xff, 5,  2,2, 1,6,
		"Online race with 1-6 players total and 2 humans at Wii." },


    //--- online, 7-N players total

    { 0x3fc0,0xff,-1,  1,2, 7,99,
		"Online race with 7 or more players total." },
    { 0x1540,0xff,-1,  1,1, 7,99,
		"Online race with 7 or more players total and 1 human at Wii." },
    { 0x2a80,0xff,-1,  2,2, 7,99,
		"Online race with 7 or more players total and 2 humans at Wii." },

    { 0x0fc0,0xff,-1,  1,2, 7,18,
		"Online race with 7-18 players total." },
    { 0x0540,0xff,-1,  1,1, 7,18,
		"Online race with 7-18 players total and 1 human at Wii." },
    { 0x0a80,0xff,-1,  2,2, 7,18,
		"Online race with 7-18 players total and 2 humans at Wii." },

    { 0x03c0,0xff,-1,  1,2, 7,12,
		"Online race with 7-12 players total." },
    { 0x01e0,0xff,-1,  1,1, 7,12,
		"Online race with 7-12 players total and 1 human at Wii." },
    { 0x0280,0xff,-1,  2,2, 7,12,
		"Online race with 7-12 players total and 2 humans at Wii." },

    { 0x00c0,0xff,-1,  1,2, 7,9,
		"Online race with 7-9 players total." },
    { 0x0040,0xff, 6,  1,1, 7,9,
		"Online race with 7-9 players total and 1 human at Wii." },
    { 0x0080,0xff, 7,  2,2, 7,9,
		"Online race with 7-9 players total and 2 humans at Wii." },


    //--- online, 10-N players total

    { 0x3f00,0xff,-1,  1,2, 10,99,
		"Online race with 10 or more players total." },
    { 0x1500,0xff,-1,  1,1, 10,99,
		"Online race with 10 or more players total and 1 human at Wii." },
    { 0x2a00,0xff,-1,  2,2, 10,99,
		"Online race with 10 or more players total and 2 humans at Wii." },

    { 0x0f00,0xff,-1,  1,2, 10,18,
		"Online race with 10-18 players total." },
    { 0x0500,0xff,-1,  1,1, 10,18,
		"Online race with 10-18 players total and 1 human at Wii." },
    { 0x0a00,0xff,-1,  2,2, 10,18,
		"Online race with 10-18 players total and 2 humans at Wii." },

    { 0x0300,0xff,-1,  1,2, 10,12,
		"Online race with 10-12 players total." },
    { 0x0100,0xff, 8,  1,1, 10,12,
		"Online race with 10-12 players total and 1 human at Wii." },
    { 0x0200,0xff, 9,  2,2, 10,12,
		"Online race with 10-12 players total and 2 humans at Wii." },


    //--- online, 13-N players total

    { 0x3c00,0xff,-1,  1,2, 13,99,
		"Online race with 13 or more players total." },
    { 0x1400,0xff,-1,  1,1, 13,99,
		"Online race with 13 or more players total and 1 human at Wii." },
    { 0x2800,0xff,-1,  2,2, 13,99,
		"Online race with 13 or more players total and 2 humans at Wii." },

    { 0x0c00,0xff,-1,  1,2, 13,18,
		"Online race with 13-18 players total." },
    { 0x0400,0xff,10,  1,1, 13,18,
		"Online race with 13-18 players total and 1 human at Wii." },
    { 0x0800,0xff,11,  2,2, 13,18,
		"Online race with 13-18 players total and 2 humans at Wii." },


    //--- online, 17-N players total

    { 0x3000,0xff,-1,  1,2, 19,99,
		"Online race with 19 or more players total." },
    { 0x1000,0xff,12,  1,1, 19,99,
		"Online race with 19 or more players total and 1 human at Wii." },
    { 0x2000,0xff,13,  2,2, 19,99,
		"Online race with 19 or more players total and 2 humans at Wii." },


    //--- reserved

    { 0x4000,0x04,14, 1,1, -1,-1,
		"Time Trial (Reserved 1)." },
    { 0xc000,0xfb,-1, -1,-1, -1,-1,
		"Reserved 1 and 2." },
    { 0x4000,0xfb,14, -1,-1, -1,-1,
		"Reserved 1." },
    { 0x8000,0xff,15, -1,-1, -1,-1,
		"Reserved 2." },
    {0,0,-1,-1,-1,-1,-1,0}
};

///////////////////////////////////////////////////////////////////////////////

uint GetGobjConditions ( gobj_condition_set_t *gcs, int setting, u16 mask )
{
    DASSERT(gcs);
    const u8 setting_mask = setting >= 0 && setting < 8 ? 1<<setting : 0xff;
    memset(gcs,0,sizeof(*gcs));
    gcs->mask = mask;
    gcs->setting_mask = setting_mask;

    const gobj_condition_t *src = gobj_condition;
    gobj_condition_t *dest = gcs->cond;
    while (src->info)
    {
	if (  setting_mask & src->setting
	   && ( mask & src->mask ) == src->mask
	   && dest - gcs->cond < GOBJ_COND_SET_MAX
	   )
	{
	    memcpy(dest++,src,sizeof(*dest));
	    mask &= ~src->mask;
	    DASSERT( dest - gcs->cond <= GOBJ_COND_SET_MAX );
	}
	src++;
    }
    memcpy(dest,src,sizeof(*dest));
    return gcs->n = dest - gcs->cond;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: Analyse GOBJ elements		///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetAnaGobj ( kmp_ana_gobj_t *ag )
{
    if (ag)
    {
	FREE(ag->info);
	memset(ag,0,sizeof(*ag));
    }
}

///////////////////////////////////////////////////////////////////////////////

static kmp_gobj_status_t CheckRefStatus ( u16 ref_id )
{
    if (IsConditionRefEngineId(ref_id))
	return KGST_REF_VALID | KGST_ENGINE;

    if (IsConditionRefRandomId(ref_id))
	return KGST_REF_VALID | KGST_RANDOM;

    const gobj_cond_ref_t *cr = FindConditionRef(ref_id);
    return cr ? KGST_REF_VALID : 0;
}

//-----------------------------------------------------------------------------

void AnalyseGobj ( kmp_ana_gobj_t *ag, const kmp_t *kmp )
{
    DASSERT(ag);
    DASSERT(kmp);

    const u_usec_t start_time = GetTimerUSec();

    memset(ag,0,sizeof(*ag));
    ag->kmp		= kmp;
    ag->gobj		= (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
    ag->n_gobj		= kmp->dlist[KMP_GOBJ].used;
    if ( ag->n_gobj > 0x7fff )
	ag->n_gobj = 0x7fff;

    ag->info		= CALLOC(ag->n_gobj,sizeof(*ag->info));


    //--- first loop

    uint i;
    kmp_gobj_info_t *info;
    const kmp_gobj_entry_t *gobj;
    for ( i = 0, info = ag->info, gobj = ag->gobj; i < ag->n_gobj; i++, info++, gobj++ )
    {
	info->defobj = -1;
	if (!gobj->obj_id)
	{
	    info->type = KGTY_NULL;
	    continue;
	}

	info->type = KGTY_INVALID;
	if ( gobj->obj_id < GOBJ_MIN_DEF )
	{
	    const u16 relevant_id = GetRelevantObjectId(gobj->obj_id);
	    if ( relevant_id >= N_KMP_GOBJ || !ObjectInfo[relevant_id].name )
		continue;

	    ag->n_object++;
	    info->obj_id  = relevant_id;
	    info->type    = KGTY_OBJECT;
	    info->status |= KGST_USED;

	    if ( gobj->obj_id < GOBJ_M_OBJECT )
	    {
		ag->n_enabled++;
		info->status |= KGST_ENABLED;
	    }
	    else
		ag->n_disabled++;

	    kmp_gobj_entry_t *defobj = FindRefObjectByID(kmp,gobj->ref_id);
	    if (defobj)
	    {
		info->ref_id  = gobj->ref_id;
		info->status |= KGST_REF_VALID;
		info->defobj  = defobj - ag->gobj;
		ag->info[info->defobj].ref_count++;
		ag->info[info->defobj].status |= KGST_USED;
	    }
	}
	else
	{
	    if ( IsDefinitionObjectIdBITS(gobj->obj_id) )
	    {
		ag->n_defobj_bits++;
		info->type = KGTY_DEFOBJ_BITS;
	    }
	    else if ( IsDefinitionObjectIdOR(gobj->obj_id) )
	    {
		ag->n_defobj_or++;
		info->type = KGTY_DEFOBJ_OR;
	    }
	    else if ( IsDefinitionObjectIdAND(gobj->obj_id) )
	    {
		ag->n_defobj_and++;
		info->type = KGTY_DEFOBJ_AND;
	    }
	    else
		continue;


	    // this is a valid def-object
	    if ( info->type == KGTY_DEFOBJ_OR || info->type == KGTY_DEFOBJ_AND )
	    {
		kmp_gobj_status_t rs =0;
		uint i;
		for ( i = 0; i < 8; i++ )
		     rs = CheckRefStatus(gobj->setting[i]);

		if (rs)
		{
		    info->status |= rs;
		    info->ref_id = gobj->ref_id;
		}
	    }
	}

	//--- analyse ref_id

	if ( gobj->ref_id )
	{
	    const kmp_gobj_status_t rs = CheckRefStatus(gobj->ref_id);
	    if (rs)
	    {
		info->status |= rs;
		info->ref_id = gobj->ref_id;
	    }
	    else
		info->status |= KGST_DISABLED_BY;
	}
    }


    //--- second loop

    for ( i = 0, info = ag->info; i < ag->n_gobj; i++, info++ )
    {
	if ( info->type == KGTY_OBJECT )
	{
	    if ( info->defobj >= 0 )
		info->status |= ag->info[info->defobj].status & KGST_M_DERIVE;

	    if ( info->status & KGST_ENGINE )
		ag->n_engine++;
	    if ( info->status & KGST_RANDOM )
		ag->n_random++;
	}
    }


    //--- finalize

    ag->n_object = ag->n_disabled + ag->n_enabled;
    ag->n_defobj = ag->n_defobj_bits + ag->n_defobj_or + ag->n_defobj_and;
    ag->dur_usec = GetTimerUSec() - start_time;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		XPF: Analyse PFlag Scenarios		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct xpf_bits_t
{
    u16		mask;		// mask to select bit
    u8		bitnum;		// bit number
    u8		pflag;		// mask to selct presence_flag
    u8		n_offline;	// num of human players at Wii
    u8		n_online;	// num of total online players
    u16		relevant;	// bit field: relevant for setting
    bool	no_prefix;	// true: suppress game mode prefix
    char	name[5];	// name
}
xpf_bits_t;

static const xpf_bits_t xpf_bits[16+1] =
{
    { 0x0001,  0, 1, 1,  0, 0x800f, 0, "1" },
    { 0x0002,  1, 2, 2,  0, 0x800f, 0, "2" },
    { 0x0004,  2, 4, 3,  0, 0x800f, 0, "3" },
    { 0x0008,  3, 4, 4,  0, 0x000f, 0, "4" },
    { 0x0010,  4, 1, 1,  2, 0x000f, 0, "1,2" },
    { 0x0020,  5, 2, 2,  2, 0x000f, 0, "2,2" },
    { 0x0040,  6, 1, 1,  7, 0x000f, 0, "1,7" },
    { 0x0080,  7, 2, 2,  7, 0x000f, 0, "2,7" },
    { 0x0100,  8, 1, 1, 10, 0x000f, 0, "1,10" },
    { 0x0200,  9, 2, 2, 10, 0x000f, 0, "2,10" },
    { 0x0400, 10, 1, 1, 13, 0x000f, 0, "1,13" },
    { 0x0800, 11, 2, 2, 13, 0x000f, 0, "2,13" },
    { 0x1000, 12, 1, 1, 19, 0x000f, 0, "1,19" },
    { 0x2000, 12, 2, 2, 19, 0x000f, 0, "2,19" },
    { 0x4000, 14, 1, 1,  0, 0x0004, 1, "TT" },
    { 0x8000, 15, 1, 0,  0, 0x0000, 0, "?" },
    {      0,  0, 0, 0,  0,      0, 0, "" },
};

///////////////////////////////////////////////////////////////////////////////

void ResetPFlagScenarios ( kmp_ana_pflag_t *ap )
{
    if (ap)
    {
	ResetAnaGobj(&ap->ag);

	uint i;
	for ( i = 0; i < ap->n_res; i++ )
	    FreeString(ap->res_list[i].status);
	FREE(ap->res_list);
	memset(ap,0,sizeof(*ap));
    }
}

///////////////////////////////////////////////////////////////////////////////

kmp_ana_pflag_res_t * AppendPFlagScenario ( kmp_ana_pflag_t *ap )
{
    DASSERT(ap);
    if ( ap->n_res == ap->res_size )
    {
	ap->res_size = 2 * ap->res_size + 50;
	ap->res_list = REALLOC( ap->res_list, ap->res_size*sizeof(*ap->res_list) );
    }

    kmp_ana_pflag_res_t *ptr = ap->res_list + ap->n_res++;
    memset(ptr,0,sizeof(*ptr));
    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

#undef SUPPORT_SEP

#if HAVE_WIIMM_EXT
  #define SUPPORT_SEP 1
#else
  #define SUPPORT_SEP 0
#endif

// helper consts

enum
{
    X_INVALID_MODE	= -16,
    X_INVALID_CONDREF,
    X_INVALID_REF,
    X_NOTFOUND,
    X_DISABLED,
    X_ENABLED,
    X_DEFOBJ_BITS,
    X_DEFOBJ_OR,
    X_DEFOBJ_AND,
    X_CONDENG,
    X_CONDRND,
    X_CONDREF_BEG	= -0x2000,
    X_CONDREF_END	= X_CONDREF_BEG + 0xfff,

    CH_DISABLED		= '-',
    CH_HIDDEN		= '=',
    CH_ENABLED		= '+',
    CH_ENGINE		= 'e',
    CH_RANDOM		= 'r',
    CH_DEFOBJ_BITS	= 'b',
    CH_DEFOBJ_OR	= 'o',
    CH_DEFOBJ_AND	= 'a',
    CH_SEP		= ' '
};

//-----------------------------------------------------------------------------

static int sort_apf_name
	( const kmp_ana_pflag_res_t * a, const kmp_ana_pflag_res_t * b )
{
    return strcasecmp(a->name,b->name);
}

//-----------------------------------------------------------------------------


static int sort_apf_index ( const kmp_ana_pflag_res_t * a, const kmp_ana_pflag_res_t * b )
{
    int stat = (int)a->version - (int)b->version;
    return stat ? stat : strcasecmp(a->name,b->name);;
}

//-----------------------------------------------------------------------------

static void analyze_helper
(
    kmp_ana_pflag_t	*ap,		// valid struct
    const s16		*ref_list,	// list with 'ap->ag.n_gobj' elements
    char		*res,		// vector with 'ap->status_len+1' elements

    int			gamemode,	// selected gamemode
    ccp			prefix,		// gamemode related prefix to use
    const xpf_bits_t	*xpf,		// selected condition
    int			engine,		// <0: off, 0..: select engine
    uint		random		// 0: off, 1-GOBJ_MAX_RANDOM: select random scenario
)
{
    char *res_ptr	= res;
    const bool is_std	= gamemode == KMP_GMODE_STD;
    const bool is_tt	= gamemode == KMP_GMODE_VERSUS && xpf->bitnum == 14;

    if ( is_std )
    {
	if ( random > 1 || engine != -1 && engine != ENGINE_150 )
	    return;
	engine = -1;
	random = 0;
    }
    else if ( is_tt )
    {
	if ( random > 1 || engine != -1 && engine != ENGINE_150 )
	    return;
	engine = ENGINE_150;
	random = 1;
    }
    else if ( gamemode == KMP_GMODE_BALLOON || gamemode == KMP_GMODE_COIN )
    {
	if ( engine != -1 && engine != ENGINE_BATTLE )
	    return;
    }
    else
    {
	if ( engine == ENGINE_BATTLE )
	    return;
    }

    if ( ap->opt_gamemode & GMD_DUMP )
    {
	printf("gm %2d, bit %2d, engine %2d, random %d\n",
		gamemode, xpf->bitnum, engine, random );
	return;
    }

    gobj_cond_mask_t cmask = {0};
    cmask.bit_mask	= xpf->mask;
    cmask.game_mask	= 1 << gamemode;
    cmask.engine_mask	= 1 << ( engine < 0 ? ENGINE_150 : engine );
    cmask.random_mask	= 1 << ( random ? random-1 : 0 );

    int i;
    const kmp_gobj_entry_t *gobj;
    for ( i = 0, gobj = ap->ag.gobj; i < ap->ag.n_gobj; i++, gobj++ )
    {
	u16 obj_id = gobj->obj_id;
	u16 pflags = gobj->pflags;
	char stat;

	if ( ref_list[i] == X_DEFOBJ_BITS )
	    stat = CH_DEFOBJ_BITS;
	else if ( ref_list[i] == X_DEFOBJ_OR )
	    stat = CH_DEFOBJ_OR;
	else if ( ref_list[i] == X_DEFOBJ_AND )
	    stat = CH_DEFOBJ_AND;
	else if (is_std)
	    stat = xpf->pflag & pflags ? CH_ENABLED : CH_HIDDEN;
	else
	{
	    stat = CH_DISABLED;
	    switch(ref_list[i])
	    {
	     case X_INVALID_MODE:
	     case X_INVALID_REF:
		stat = xpf->pflag & pflags ? CH_ENABLED : CH_HIDDEN;
		break;

	     case X_INVALID_CONDREF:
	     case X_NOTFOUND:
	     case X_DISABLED:
		obj_id = 0;
		break;

	     case X_ENABLED:
		stat = CH_ENABLED;
		obj_id = obj_id & ~GOBJ_M_LECODE;
		break;

	     case X_CONDENG:
		if ( engine < 0 )
		{
		    stat = CH_ENGINE;
		    obj_id = obj_id & ~GOBJ_M_LECODE;
		}
		else if ( gobj->ref_id & cmask.engine_mask )
		{
		    stat = CH_ENABLED;
		    obj_id = obj_id & ~GOBJ_M_LECODE;
		}
		break;

	     case X_CONDRND:
		if ( !random )
		{
		    stat = CH_RANDOM;
		    obj_id = obj_id & ~GOBJ_M_LECODE;
		}
		else if ( gobj->ref_id & cmask.random_mask )
		{
		    stat = CH_ENABLED;
		    obj_id = obj_id & ~GOBJ_M_LECODE;
		}
		break;

	     default:
		if ( ref_list[i] >= X_CONDREF_BEG && ref_list[i] <= X_CONDREF_END )
		{
		    const gobj_cond_ref_t *cref
			= cond_ref_tab + ( ref_list[i] - X_CONDREF_BEG );
		    if (IsConditionRefEnabled(&cmask,gobj->ref_id,cref))
			stat = CH_ENABLED;
		}
		else
		{
		    bool enabled = false;
		    const kmp_gobj_entry_t *dob = ap->ag.gobj + ref_list[i];
		    if ( !dob->ref_id || IsConditionRefEnabled(&cmask,dob->ref_id,0) )
		    {
			if (IsDefinitionObjectIdBITS(dob->obj_id))
			{
			    if ( dob->setting[gamemode] & cmask.bit_mask )
				enabled = true;
			}
			else if (IsDefinitionObjectIdOR(dob->obj_id))
			{
			    uint i;
			    for ( i = 0; i < 8; i++ )
				if ( dob->setting[i]
				    && IsConditionRefEnabled(&cmask,dob->setting[i],0) )
				{
				    enabled = true;
				    break;
				}
			}
			else if (IsDefinitionObjectIdAND(dob->obj_id))
			{
			    enabled = true;
			    uint i;
			    for ( i = 0; i < 8; i++ )
				if ( dob->setting[i]
				    && !IsConditionRefEnabled(&cmask,dob->setting[i],0) )
				{
				    enabled = false;
				    break;
				}
			}
		    }

		    if ( gobj->ref_id & GOBJ_DEF_F_NOT )
			enabled = !enabled;
		    if (enabled)
			stat = CH_ENABLED;
		}
		if ( stat == CH_ENABLED || stat == CH_RANDOM )
		    obj_id = obj_id & ~GOBJ_M_LECODE;
		break;
	    }
	}

	if ( stat != CH_DEFOBJ_BITS && stat != CH_DEFOBJ_OR && stat != CH_DEFOBJ_AND
		&& (obj_id >= N_KMP_GOBJ || !ObjectInfo[obj_id].name ))
	{
	    stat = CH_DISABLED;
	}

     #if SUPPORT_SEP
	if ( res_ptr > res
	    && ( gobj->pflags & 0x100 || ap->auto_sep && !(i%5) ))
	{
	    *res_ptr++ = CH_SEP;
	}
     #endif
	*res_ptr++ = stat;
	DASSERT( res_ptr <= res + ap->status_len );
    }
    ASSERT( res_ptr <= res + ap->status_len );

    int ri, found_ri = -1;
    for ( ri = 0; ri < ap->n_res; ri++ )
	if (!memcmp(ap->res_list[ri].status,res,ap->status_len))
	{
	    found_ri = ri;
	    kmp_ana_pflag_res_t *found = ap->res_list + ri;
	    if ( is_std && !found->n_std++ )
	    {
		ap->n_res_std++;
		StringCat2S(found->name,sizeof(found->name),found->name,"+");
	    }
	    if ( !is_std && !found->n_ext++ )
	    {
		ap->n_res_ext++;
		StringCat2S(found->name,sizeof(found->name),found->name,"+");
	    }
	    break;
	}

    if	(  found_ri < 0
	|| ap->opt_gamemode & GMD_FULL
	|| is_tt && ap->opt_gamemode & GMD_FORCE_TT
	)
    {
	kmp_ana_pflag_res_t *pr = AppendPFlagScenario(ap);
	kmp_ana_pflag_res_t *found = found_ri < 0 ? 0 : ap->res_list + found_ri;
	ccp pre = xpf->no_prefix || !prefix ? "" : prefix;
	pr->status = ALLOCDUP(res,ap->status_len+1);
	pr->version = found ? found->version : ap->n_version++;

	memcpy(&pr->lex_test,TestDataLEX,sizeof(pr->lex_test));
	char *name_end = pr->name + sizeof(pr->name);
	char *dest = snprintfE(pr->name,name_end,"%s%s",pre,xpf->name);

	if ( engine > 0 )
	{
	    static const char tab[][5] = { "bat","50c","100c","150c","200c","150m","200m" };
	    dest = StringCat2E(dest,name_end,",",tab[engine]);
	    pr->lex_test.engine = engine;
	}

	if ( random && !is_tt )
	{
	    dest = snprintfE(dest,name_end,",r%u",random);
	    pr->lex_test.random = random;
	}

	pr->lex_test.offline_online	= xpf->n_online > 0
					? LEX_OO_ONLINE : LEX_OO_OFFLINE;
	pr->lex_test.n_online		= xpf->n_online;
	pr->lex_test.n_offline		= xpf->n_offline;
	if (is_std)
	{
	    pr->lex_test.cond_bit	= -1;
	    pr->lex_test.game_mode	= LEX_GMODE_STD;
	}
	else
	{
	    pr->lex_test.cond_bit	= xpf->bitnum;
	    pr->lex_test.game_mode	= gamemode + LEX_GMODE__BASE;
	}

	if (found)
	{
	    if ( !is_std && !found->n_ext++ )
		ap->n_res_ext++;
	}
	else if ( is_std && !pr->n_std++ )
	    ap->n_res_std++;
	else if ( !is_std && !pr->n_ext++ )
	    ap->n_res_ext++;
    }
}

//-----------------------------------------------------------------------------

void AnalysePFlagScenarios
	( kmp_ana_pflag_t *ap, const kmp_t *kmp, opt_gamemode_t opt_gamemode )
{
    DASSERT(ap);
    DASSERT(kmp);

    const u_usec_t start_time = GetTimerUSec();

    memset(ap,0,sizeof(*ap));
    AnalyseGobj(&ap->ag,kmp);
    ap->opt_gamemode = opt_gamemode;

    struct job_t
    {
	char prefix[4];
	int  gamemode;
	int  opt_gmd;
    };

    static const struct job_t race_job_list[] =
    {
	{ "std",KMP_GMODE_STD,		GMD_STD		},
	{ "V",	KMP_GMODE_VERSUS,	GMD_VERSUS	},
	{ "I",	KMP_GMODE_ITEMRAIN,	GMD_ITEMRAIN	},
	{ "B",	KMP_GMODE_BALLOON,	GMD_BALLOON	},
	{ "C",	KMP_GMODE_COIN,		GMD_COIN	},
	{ "std",KMP_GMODE_STD,		GMD_STD		},
	{ "",-1,0}
    };

    static const struct job_t arena_job_list[] =
    {
	{ "std",KMP_GMODE_STD,		GMD_STD		},
	{ "B",	KMP_GMODE_BALLOON,	GMD_BALLOON	},
	{ "C",	KMP_GMODE_COIN,		GMD_COIN	},
	{ "V",	KMP_GMODE_VERSUS,	GMD_VERSUS	},
	{ "I",	KMP_GMODE_ITEMRAIN,	GMD_ITEMRAIN	},
	{ "std",KMP_GMODE_STD,		GMD_STD		},
	{ "",-1,0}
    };

    static const struct job_t auto_job_list[] =
    {
	{ "std",KMP_GMODE_STD,		GMD_STD		},
	{ "a",	KMP_GMODE_AUTO,		GMD_NO_MODES	},
	{ "std",KMP_GMODE_STD,		GMD_STD		},
	{ "",-1,0}
    };


    //--- create reference list for defobj for faster analysis

 #if SUPPORT_SEP
    uint n_sep = 0;
 #endif
    s16 *ref_list = MALLOC(ap->ag.n_gobj*sizeof(*ref_list));
    uint i;
    const kmp_gobj_entry_t *gobj;
    for ( i = 0, gobj = ap->ag.gobj; i < ap->ag.n_gobj; i++, gobj++ )
    {
     #if SUPPORT_SEP
	if (gobj->pflags&0x100)
	    n_sep++;
     #endif
	if (IsDefinitionObjectIdBITS(gobj->obj_id))
	    ref_list[i] = X_DEFOBJ_BITS;
	else if (IsDefinitionObjectIdOR(gobj->obj_id))
	    ref_list[i] = X_DEFOBJ_OR;
	else if (IsDefinitionObjectIdAND(gobj->obj_id))
	    ref_list[i] = X_DEFOBJ_AND;
	else if ( GetPFlagsMode(gobj->pflags) != 1 )
	    ref_list[i] = X_INVALID_MODE;
	else if (IsConditionRefId(gobj->ref_id))
	{
	    if (IsConditionRefEngineId(gobj->ref_id))
	    {
		const uint engine = gobj->ref_id & GOBJ_MASK_COND_ENGINE;
		if (!engine)
		    ref_list[i] = X_DISABLED;
		else if ( engine == GOBJ_MASK_COND_ENGINE )
		    ref_list[i] = X_ENABLED;
		else
		    ref_list[i] = X_CONDENG;
	    }
	    else if (IsConditionRefRandomId(gobj->ref_id))
	    {
		const uint rnd = gobj->ref_id & GOBJ_MASK_COND_RND;
		if (!rnd)
		    ref_list[i] = X_DISABLED;
		else if ( rnd == GOBJ_MASK_COND_RND )
		    ref_list[i] = X_ENABLED;
		else
		    ref_list[i] = X_CONDRND;
	    }
	    else
	    {
		const gobj_cond_ref_t *cr = FindConditionRef(gobj->ref_id);
		ref_list[i] = cr ? X_CONDREF_BEG + cr-cond_ref_tab : X_INVALID_CONDREF;
	    }
	}
	else if (!IsDefinitionObjectId(gobj->ref_id))
	    ref_list[i] = X_INVALID_REF;
	else
	{
	    const kmp_gobj_entry_t *found = FindRefObject(kmp,gobj);
	    ref_list[i] = found ? found - ap->ag.gobj : X_NOTFOUND;
	}
    }

 #if defined(TEST) || defined(HAVE_WIIMM_EXT)
    if ( logging >= 2 )
    {
	XDump_t xd;
	InitializeXDump(&xd);
	xd.print_format  = false;
	xd.print_summary = false;
	xd.print_text    = false;
	xd.format        = XDUMPF_INT_2;
	xd.min_width     = 20;
	XDump(&xd,ref_list,ap->ag.n_gobj*sizeof(*ref_list),true);
	putchar('\n');
    }
 #endif


    //--- add "auto"?

    if ( opt_gamemode & (GMD_AUTO|GMD_FULL) )
    {
	kmp_ana_pflag_res_t *pr = AppendPFlagScenario(ap);
	StringCopyS(pr->name,sizeof(pr->name),"auto");
	pr->status = EmptyString;
	memcpy(&pr->lex_test,TestDataLEX,sizeof(pr->lex_test));
    }


    //--- separator

 #if SUPPORT_SEP
    ap->auto_sep = !n_sep;
    if (ap->auto_sep)
	n_sep = ap->ag.n_gobj/5;
    ap->status_len = ap->ag.n_gobj + n_sep;
 #else
    ap->status_len = ap->ag.n_gobj;
 #endif
    char *res = CALLOC(1,ap->status_len+1);


    //--- engine & random

    int engine_beg, engine_end;
    if ( ap->ag.n_engine && ( opt_gamemode & GMD_ENGINE ))
    {
	engine_beg = ENGINE_BATTLE;
	engine_end = ENGINE__N-1;
    }
    else
	engine_beg = engine_end = -1;

    int random_beg, random_end;
    if ( ap->ag.n_random && ( opt_gamemode & GMD_RANDOM ))
    {
	random_beg = 1;
	random_end = GOBJ_MAX_RANDOM;
    }
    else
	random_beg = random_end = 0;


    //--- main loop

    const struct job_t *job
		= (opt_gamemode&GMD_NO_MODES)
		? auto_job_list
		: CheckBattleModeKMP(kmp) >= ARENA_FOUND
		? arena_job_list
		: race_job_list;
    if (!(opt_gamemode&GMD_STD_FIRST))
	job++;

    for ( ; job->gamemode >= 0; job++ )
    {
	if ( !( opt_gamemode & job->opt_gmd ) )
	    continue;

	const xpf_bits_t *xpf;
	for ( xpf = xpf_bits; xpf->mask; xpf++ )
	{
	    if ( job->gamemode == KMP_GMODE_AUTO )
		goto skip;

	    if ( !( 1 << job->gamemode & xpf->relevant ))
		continue;

	    // optimized handling for time trial
	    const bool is_tt = job->gamemode == KMP_GMODE_VERSUS && xpf->bitnum == 14;
	    if (is_tt)
	    {
		if ( opt_gamemode & GMD_NO_TT )
		    continue;
		if ( opt_gamemode & GMD_FORCE_TT )
		    analyze_helper(ap,ref_list,res, job->gamemode,job->prefix,xpf,-1,0);
	    }

	    if ( !(opt_gamemode & ( xpf->n_online ? GMD_ONLINE : GMD_OFFLINE )) )
		continue;

	    if (is_tt)
	    {
		analyze_helper(ap,ref_list,res, job->gamemode,job->prefix,xpf,-1,0);
		continue;
	    }

	skip:;
	    int e;
	    for ( e = engine_beg; e <= engine_end; e++ )
	    {
		int r;
		for ( r = random_beg; r <= random_end; r++ )
		    analyze_helper(ap,ref_list,res, job->gamemode,job->prefix,xpf,e,r);
	    }
	} // for xpf
    } // for gmode

    if ( ap->n_res > 1 )
    {
	if ( opt_gamemode & GMD_ISORT )
	    qsort( ap->res_list, ap->n_res, sizeof(*ap->res_list),
			(qsort_func)sort_apf_index );
	else if ( opt_gamemode & GMD_NSORT )
	    qsort( ap->res_list, ap->n_res, sizeof(*ap->res_list),
			(qsort_func)sort_apf_name );
    }

    FREE(res);
    FREE(ref_list);

    ap->dur_usec = GetTimerUSec() - start_time;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    XPF: repair: global helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static ccp repair_xpf_fname	= 0;
static kmp_t *repair_kmp	= 0;

///////////////////////////////////////////////////////////////////////////////

int ScanOptRepairXPF ( ccp arg )
{
    if (repair_xpf_fname)
	have_patch_count--, have_kmp_patch_count--;

    repair_xpf_fname = arg && *arg ? arg : 0;

    if (repair_xpf_fname)
	have_patch_count++, have_kmp_patch_count++;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

xpf_mode1_t CheckMode1GOBJ ( const kmp_gobj_entry_t *g )
{
    DASSERT(g);
    if (( g->pflags & GOBJ_M_PF_MODE ) != GOBJ_PF_MODE1 )
	return XMODE1_NONE;

    return IsConditionRefId(g->ref_id)     ? XMODE1_CONDREF
	 : IsDefinitionObjectId(g->ref_id) ? XMODE1_OBJREF
	 : g->ref_id ? XMODE1_INVALID : XMODE1_NULL;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			XPF: repair			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[xpf_mode1_elem_t]]

typedef struct xpf_mode1_elem_t
{
    int			index;		// index of element within list
    xpf_mode1_t		mode;		// mode
    kmp_gobj_entry_t	*obj;		// pointer to object
    kmp_gobj_entry_t	norm;		// copy of 'obj' with normalized data

    int			eval;		// evaluation
    double		diff_sum;	// sum of float differences
    struct xpf_mode1_elem_t
			*hit;		// best element found
    int			match_id;	// for logging only: identify matching pair
}
xpf_mode1_elem_t;

//-----------------------------------------------------------------------------
// [[xpf_mode1_info_t]]

typedef struct xpf_mode1_info_t
{
    uint		broken;		// number of broken elements in 'list'
    uint		used;		// number of used elements in 'list'
    uint		size;		// number of available elements in 'list'
    xpf_mode1_elem_t	list[];		// list of elements
}
xpf_mode1_info_t;

//-----------------------------------------------------------------------------

static xpf_mode1_info_t * CreateXpfModelInfo ( const List_t *list )
{
    DASSERT(list);
    uint i, n = 0;
    kmp_gobj_entry_t *obj;

    for ( i = 0, obj = (kmp_gobj_entry_t*)list->list; i < list->used; i++, obj++ )
    {
	const xpf_mode1_t mode = CheckMode1GOBJ(obj);
	if ( mode > XMODE1_NONE )
	    n++;
	PRINT0("%2d/%d: %u %04x %04x %04x\n",
		i, list->used, n, obj->obj_id, obj->ref_id, obj->pflags );
    }

    if (!n)
	return 0;

    const uint mem_size = sizeof(xpf_mode1_info_t) + n * sizeof(xpf_mode1_elem_t);
    xpf_mode1_info_t *res = CALLOC(1,mem_size);
    res->size = res->used = n;

    xpf_mode1_elem_t *elem = res->list;
    for ( i = 0, obj = (kmp_gobj_entry_t*)list->list; i < list->used; i++, obj++ )
    {
	const xpf_mode1_t mode = CheckMode1GOBJ(obj);
	if ( mode > XMODE1_NONE )
	{
	    elem->index	= i;
	    elem->obj	= obj;
	    elem->mode	= mode;
	    if ( mode <= XMODE1_INVALID )
		res->broken++;
	    memcpy(&elem->norm,obj,sizeof(elem->norm));
	    elem->norm.obj_id &= GOBJ_M_OBJECT;
	    elem->norm.ref_id  = 0;
	    elem->norm.pflags  = 0;
	    elem++;
	}
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////////

static void LogPrintElement ( xpf_mode1_elem_t *e1, xpf_mode1_elem_t *e2 )
{
    if ( e2 && e2->hit == e1 )
	fprintf(stdlog," : %s%3d. %s%04x %s%04x %s%04x%s",
		e1->index == e2->index ? collog->b_green : collog->b_red, e1->index,
		e1->obj->obj_id == e2->obj->obj_id ? collog->b_green
			: collog->b_red, e1->obj->obj_id,
		e1->obj->ref_id == e2->obj->ref_id ? collog->b_green
			: collog->b_red, e1->obj->ref_id,
		e1->obj->pflags == e2->obj->pflags ? collog->b_green
			: collog->b_red, e1->obj->pflags,
		collog->reset );
    else
	fprintf(stdlog," : %3d. %04x %04x %04x",
		e1->index, e1->obj->obj_id, e1->obj->ref_id, e1->obj->pflags );
}

//-----------------------------------------------------------------------------

static void LogListXPF
	( xpf_mode1_info_t *info, int index, ccp text, int loop, int update_eval )
{
    int i, count = 0, match_id = 0;
    xpf_mode1_elem_t *e;
    for ( i = 0, e = info->list; i < info->used; i++, e++ )
    {
	if ( e->eval >= 0 )
	{
	    if (!count++)
		fprintf(stdlog,"\nRepair X-PFlags, loop %u, %s:\n",loop,text);

	    fprintf(stdlog,"%12.3g %6d", e->diff_sum, e->eval );
	    if ( e->eval == update_eval )
	    {
		LogPrintElement(e,e->hit);
		LogPrintElement(e->hit,e);

		if ( index == 1 )
		{
		    e->match_id = ++match_id;
		    fprintf(stdlog,"  [%u]",match_id);
		}
		else
		    fprintf(stdlog,"  [%u]",e->hit->match_id);
	    }
	    else
		LogPrintElement(e,0);
	    fputc('\n',stdlog);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static uint DiffXPF ( xpf_mode1_info_t *l1, xpf_mode1_info_t *l2, int loop )
{
    DASSERT(l1);
    DASSERT(l2);

    //--- setup

    uint i1, i2;
    xpf_mode1_elem_t *e1, *e2;

    // clear second list
    for ( i2 = 0, e2 = l2->list; i2 < l2->used; i2++, e2++ )
	if ( e2->eval > 0 )
	    e2->eval = 0;


    //--- compare each with each

    int max_eval = 0;
    for ( i1 = 0, e1 = l1->list; i1 < l1->used; i1++, e1++ )
    {
	if ( e1->eval < 0 )
	    continue;

	for ( i2 = 0, e2 = l2->list; i2 < l2->used; i2++, e2++ )
	{
	    if ( e2->eval < 0 )
		continue;

	    int eval = 0;
	    double diff_sum = 0.0;

	    const kmp_gobj_entry_t *o1 = e1->obj;
	    const kmp_gobj_entry_t *o2 = e2->obj;

	    if (!memcmp(o1,o2,sizeof(*o1)))
		eval = 990000;
	    else if (!memcmp(&e1->norm,&e2->norm,sizeof(e1->norm)))
	    {
		eval = 950000;
		if ( o1->obj_id == o2->obj_id ) eval += 2000;
		if ( o1->pflags == o2->pflags ) eval += 1000;
	    }
	    else
	    {
		diff_sum = Length2F(o1->position,o2->position)
			 + Length2F(o1->rotation,o2->rotation)
			 + Length2F(o1->scale,o2->scale);

		if ( o1->obj_id == o2->obj_id )
		    eval = 400000;
		else if ( (o1->obj_id&GOBJ_M_PF_STD) == (o2->obj_id&GOBJ_M_PF_STD) )
		    eval = 200000;

		if ( o1->pflags == o2->pflags )
		    eval += 100000;

		uint i;
		for ( i = 0; i < 8; i++ )
		    if ( o1->setting[i] == o2->setting[i] )
			eval += 10000;

		diff_sum = Length2F(o1->position,o2->position)
			 + Length2F(o1->rotation,o2->rotation)
			 + Length2F(o1->scale,o2->scale);

		if ( diff_sum > 1.0 )
		{
		    int exp;
		    frexpl(diff_sum,&exp);
		    eval -= exp;
		}
	    }

	    eval += 999 - 3*abs( e1->index - e2->index );

	    if ( e1->eval < eval || e1->eval == eval && e1->diff_sum > diff_sum )
	    {
		e1->eval	= eval;
		e1->diff_sum	= diff_sum;
		e1->hit		= e2;
	    }

	    if ( e2->eval < eval || e2->eval == eval && e2->diff_sum > diff_sum )
	    {
		e2->eval	= eval;
		e2->diff_sum	= diff_sum;
		e2->hit		= e1;
	    }

	    if ( max_eval < eval )
		max_eval = eval;

	}
    }


    //--- logging

    if ( logging >= 2 )
    {
	LogListXPF(l1,1,"patching list",loop,max_eval);
	LogListXPF(l2,2,"reference list",loop,max_eval);
    }


    //--- update

    uint update_count = 0;
    if ( max_eval > 0 )
    {
	for ( i1 = 0, e1 = l1->list; i1 < l1->used; i1++, e1++ )
	{
	    if ( e1->eval == max_eval )
	    {
		e2 = e1->hit;
		if ( e2->eval == max_eval && e2->hit == e1 )
		{
		    if (!e1->obj->ref_id)
			e1->obj->ref_id = e2->obj->ref_id;
		    e1->eval = e2->eval = -1;
		    update_count++;
		}
	    }
	}
    }

    return update_count;
}

///////////////////////////////////////////////////////////////////////////////

enumError RepairXPF ( kmp_t *kmp )
{
    if (!repair_kmp)
    {
	if (!repair_xpf_fname)
	    return ERR_NOTHING_TO_DO;

	raw_data_t raw;
	enumError err = LoadRawData(&raw,true,repair_xpf_fname,"course.kmp",false,0);
	repair_xpf_fname = 0;
	if (err)
	{
	    ResetRawData(&raw);
	    return err;
	}

	disable_patch_on_load++;
	repair_kmp = MALLOC(sizeof(*repair_kmp));
	err = ScanRawDataKMP(repair_kmp,true,&raw,0);
	disable_patch_on_load--;
	if ( err > ERR_WARNING )
	{
	    ResetKMP(repair_kmp);
	    FREE(repair_kmp);
	    repair_kmp = 0;
	    return err;
	}
    }


    //--- setup lists

    xpf_mode1_info_t *rinfo = CreateXpfModelInfo( repair_kmp->dlist + KMP_GOBJ );
    xpf_mode1_info_t *kinfo = CreateXpfModelInfo( kmp->dlist + KMP_GOBJ );

    if ( !rinfo || !kinfo || !kinfo->broken )
    {
	FREE(rinfo);
	FREE(kinfo);
	return ERR_NOTHING_TO_DO;
    }

    PRINT0("REPAIR XPF! rep=%d/%d, kmp=%d/%d\n",
		rinfo->broken, rinfo->used, kinfo->broken, kinfo->used );

    int loop = 1;
    while (DiffXPF(kinfo,rinfo,loop))
	loop++;

    if ( logging >= 2 )
	fputc('\n',stdlog);

    FREE(rinfo);
    FREE(kinfo);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

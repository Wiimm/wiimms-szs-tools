
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
 *   Copyright (c) 2011-2024 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include "lib-std.h"
#include "lib-mkw.h"
#include "lib-kmp.h"
#include "db-mkw.h"
#include "ui.h"
#include <math.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    data			///////////////
///////////////////////////////////////////////////////////////////////////////

static Var_t NullVector = {0};
static double3 NullVectorD;

static VarMap_t parser_func = {0,0,0};
static VarMap_t parser_func_info = {0,0,0};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			management functions		///////////////
///////////////////////////////////////////////////////////////////////////////

void DefineParserVars ( VarMap_t * vm )
{
    SetupConst();
    NullVectorD.x = NullVectorD.y = NullVectorD.z = 0.0;


    //--- setup '$NONE'

    InsertVarMap(vm,"$NONE",false,0,0);


    //--- setup integer variables

    struct inttab_t { ccp name; int val; };
    static const struct inttab_t inttab[] =
    {
	{ "FALSE",		0 },
	{ "TRUE",		1 },
	{ "NO",			0 },
	{ "YES",		1 },
	{ "OFF",		0 },
	{ "ON",			1 },
	{ "DISABLED",		0 },
	{ "ENABLED",		1 },

	{ "INT$MIN",		INT_MIN },
	{ "INT$MAX",		INT_MAX },

	{ "TYPE$UNDEF",		-1 },
	{ "TYPE$UNSET",		VAR_UNSET },
	{ "TYPE$INT",		VAR_INT },
	{ "TYPE$FLOAT",		VAR_DOUBLE },
	{ "TYPE$VECTOR",	VAR_VECTOR },
	{ "TYPE$STR",		VAR_STRING },
	{ "TYPE$X",		VAR_X },
	{ "TYPE$Y",		VAR_Y },
	{ "TYPE$Z",		VAR_Z },

	{ "REVISION$TOOL",	REVISION_NUM },
	{ "REVISION$SETUP",	REVISION_NUM },
	{ "REVISION$ACTIVE",	REVISION_NUM },

	{0,0}
    };

    const struct inttab_t * ip;
    for ( ip = inttab; ip->name; ip++ )
	DefineIntVar(vm,ip->name,ip->val);


    //--- setup float variables

    struct dbltab_t { ccp name; double val; };
    static const struct dbltab_t dbltab[] =
    {
	{ "M$E",	2.7182818284590452354  }, // e
	{ "M$LOG2E",	1.4426950408889634074  }, // log_2(e)
	{ "M$LOG10E",	0.43429448190325182765 }, // log_10(e)
	{ "M$LN2",	0.69314718055994530942 }, // log_e(2)
	{ "M$LN10",	2.30258509299404568402 }, // log_e(10)
	{ "M$PI",	3.14159265358979323846 }, // pi
	{ "M$PI_2",	1.57079632679489661923 }, // pi/2
	{ "M$PI_4",	0.78539816339744830962 }, // pi/4
	{ "M$C1_PI",	0.31830988618379067154 }, // 1/pi
	{ "M$2_PI",	0.63661977236758134308 }, // 2/pi
	{ "M$2_SQRTPI",	1.12837916709551257390 }, // 2/sqrt(pi)
	{ "M$SQRT2",	1.41421356237309504880 }, // sqrt(2)
	{ "M$1_SQRT1",	0.70710678118654752440 }, // 1/sqrt(2)

	{ "INF",	INFINITY },
	{ "M$INF",	INFINITY },

	{0,0}
    };

    const struct dbltab_t * dp;
    for ( dp = dbltab; dp->name; dp++ )
	DefineDoubleVar(vm,dp->name,dp->val);
}

///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupParserVars()
{
    static VarMap_t vm = { .force_case = LOUP_UPPER };
    if (!vm.used)
	DefineParserVars(&vm);
    return &vm;
}

///////////////////////////////////////////////////////////////////////////////

void DefineMkwVars ( VarMap_t * vm )
{
    //--- setup integer variables

    struct inttab_t { ccp name; int val; };
    static const struct inttab_t inttab[] =
    {
	{ "N$NONE",		RTL_N_NONE },	// Mode of Nintendo-Tracks
	{ "N$HIDE",		RTL_N_HIDE },
	{ "N$SHOW",		RTL_N_SHOW },
	{ "N$SWAP",		RTL_N_SWAP },

	{ "N$F_HEX",		RTL_N_F_HEX },
	{ "N$F_WII",		RTL_N_F_WII },

	{ "N$F_RED1",		RTL_N_F_RED1 },
	 { "N$F_RED",		RTL_N_F_RED1 },
	{ "N$F_RED2",		RTL_N_F_RED2 },
	{ "N$F_RED3",		RTL_N_F_RED3 },
	{ "N$F_RED4",		RTL_N_F_RED4 },
	{ "N$F_YELLOW",		RTL_N_F_YELLOW },
	{ "N$F_GREEN",		RTL_N_F_GREEN },
	{ "N$F_BLUE1",		RTL_N_F_BLUE1 },
	 { "N$F_BLUE",		RTL_N_F_BLUE1 },
	{ "N$F_BLUE2",		RTL_N_F_BLUE2 },
	{ "N$F_WHITE",		RTL_N_F_WHITE },
	{ "N$F_CLEAR",		RTL_N_F_CLEAR },

	{ "SLOT$WAIT",		SLOT_WAIT },
	{ "SLOT$RANDOM",	SLOT_RANDOM },
	{ "SLOT$RND_ALL",	LE_SLOT_RND_ALL },
	{ "SLOT$RND_ORIG",	LE_SLOT_RND_ORIGINAL },
	{ "SLOT$RND_CUSTOM",	LE_SLOT_RND_CUSTOM },
	{ "SLOT$RND_NEW",	LE_SLOT_RND_NEW },

	{ "LE$F_NEW",		G_LEFL_NEW },
	{ "LE$F_HEAD",		G_LEFL_RND_HEAD },
	{ "LE$F_GROUP",		G_LEFL_RND_GROUP },
	{ "LE$F_ALIAS",		G_LEFL_ALIAS },
	{ "LE$F_TEXTURE",	G_LEFL_TEXTURE },
	{ "LE$F_HIDDEN",	G_LEFL_HIDDEN },

	{ "LE$DISABLE",		LE_DISABLED },
	{ "LE$DISABLED",	LE_DISABLED },
	{ "LE$ENABLE",		LE_ENABLED },
	{ "LE$ENABLED",		LE_ENABLED },
	{ "LE$ALTERABLE",	LE_ALTERABLE },
	{ "LE$EXCLUDE",		LE_EXCLUDED },
	{ "LE$EXCLUDED",	LE_EXCLUDED },
	{ "LE$INCLUDE",		LE_INCLUDED },
	{ "LE$INCLUDED",	LE_INCLUDED },
	{ "LE$EXTENT",		LE_EXTENT },
	{ "LE$EXTENDED",	LE_EXTENT },
	{ "LE$REGION",		LE_REGION },

	{ "LE$PRODUCTIVE",	LPM_PRODUCTIVE },
	{ "LE$TESTING",		LPM_TESTING },
	{ "LE$EXPERIMENTAL",	LPM_EXPERIMENTAL },
	{ "LE$AUTOMATIC",	LPM_AUTOMATIC },

	{ "LEX$CB_AUTO",	-1 },
	{ "LEX$CB_TIMETRIAL",	14 },
	{ "LEX$CB_RESERVED1",	14 },
	{ "LEX$CB_RESERVED2",	15 },

	{ "LEX$OO_AUTO",	LEX_OO_AUTO },
	{ "LEX$OO_OFFLINE",	LEX_OO_OFFLINE },
	{ "LEX$OO_ONLINE",	LEX_OO_ONLINE },

	{ "LEX$GM_AUTO",	LEX_GMODE_AUTO },
	{ "LEX$GM_STANDARD",	LEX_GMODE_STD },
	{ "LEX$GM_STD",		LEX_GMODE_STD },
	{ "LEX$GM_BALLOON",	LEX_GMODE_BALLOON },
	{ "LEX$GM_COIN",	LEX_GMODE_COIN },
	{ "LEX$GM_VERSUS",	LEX_GMODE_VERSUS },
	{ "LEX$GM_ITEMRAIN",	LEX_GMODE_ITEMRAIN },

	{ "LEX$EN_AUTO",	LEX_ENGINE_AUTO },
	{ "LEX$EN_BATTLE",	LEX_ENGINE_BATTLE },
	{ "LEX$EN_50CC",	LEX_ENGINE_50 },
	{ "LEX$EN_100CC",	LEX_ENGINE_100 },
	{ "LEX$EN_150CC",	LEX_ENGINE_150 },
	{ "LEX$EN_200CC",	LEX_ENGINE_200 },
	{ "LEX$EN_150M",	LEX_ENGINE_150M },
	{ "LEX$EN_200M",	LEX_ENGINE_200M },

	{ "RITP$OFF",		RITP_MODE_OFF },
	{ "RITP$START",		RITP_MODE_START },
	// [[new-ritp-mode]]

	{ "VEH$SMALL",		CHATVEH_SMALL },
	{ "VEH$MEDIUM",		CHATVEH_MEDIUM },
	{ "VEH$LARGE",		CHATVEH_LARGE },
	{ "VEH$ANY_SIZE",	CHATVEH_ANY_SIZE },
	 { "VEH$KART",		CHATVEH_KART },
	 { "VEH$OUT_BIKE",	CHATVEH_OUT_BIKE },
	 { "VEH$IN_BIKE",	CHATVEH_IN_BIKE },
	 { "VEH$BIKE",		CHATVEH_BIKE },
	 { "VEH$ANY_TYPE",	CHATVEH_ANY_TYPE },
	{ "VEH$ANY",		CHATVEH_ANY },

	{ "CHAT$OFF",		CHATMD_OFF },
	{ "CHAT$ANY_TRACK",	 CHATMD_ANY_TRACK },
	{ "CHAT$TRACK_BY_HOST",	 CHATMD_TRACK_BY_HOST },
	{ "CHAT$BLOCK_CLEAR",	 CHATMD_BLOCK_CLEAR },
	{ "CHAT$BLOCK_DISABLE",	 CHATMD_BLOCK_DISABLE },
	{ "CHAT$BLOCK_ENABLE",	 CHATMD_BLOCK_ENABLE },
	{ "CHAT$ANY_VEHICLE",	CHATMD_ANY_VEHICLE },
	{ "CHAT$KARTS_ONLY",	CHATMD_KARTS_ONLY },
	{ "CHAT$BIKES_ONLY",	CHATMD_BIKES_ONLY },
	{ "CHAT$RESET_ENGINE",	 CHATMD_RESET_ENGINE },
	{ "CHAT$USE_ENGINE_1",	 CHATMD_USE_ENGINE_1 },
	{ "CHAT$USE_ENGINE_2",	 CHATMD_USE_ENGINE_2 },
	{ "CHAT$USE_ENGINE_3",	 CHATMD_USE_ENGINE_3 },
	{ "CHAT$RESET",		CHATMD_RESET },

	{ "LC",			8 },		// LC has no own music slot

	{ "T0",			0 },		// unknown track or arena
	{ "T00",		0 },
	{ "A0",			0 },
	{ "A00",		0 },

	{0,0}
    };

    const struct inttab_t * ip;
    for ( ip = inttab; ip->name; ip++ )
	DefineIntVar(vm,ip->name,ip->val);

    for ( int slot = MKW_LE_RANDOM_BEG; slot < MKW_LE_RANDOM_END; slot++ )
	DefineIntVar(vm,GetLecodeRandomName(slot,0),slot);
	

    //--- setup basic parser variables

    DefineParserVars(vm);


    //--- setup music and property ids

    uint tid;
    char name[20];


    //--- tracks

    for ( tid = 0; tid < MKW_N_TRACKS; tid++ )
    {
	const TrackInfo_t *info = track_info + tid;
	snprintf(name,sizeof(name),"T%02u",info->def_slot);
	DefineIntVar(vm,name,tid);
    }


    //--- arenas

    for ( tid = 0; tid < MKW_N_ARENAS; tid++ )
    {
	const uint aid = tid + MKW_N_TRACKS;
	const TrackInfo_t *info = arena_info + tid;
	snprintf(name,sizeof(name),"A%02u",info->def_slot);
	DefineIntVar(vm,name,aid);
    }


    //--- music id, tracks after arenas because of duplicate names

    const MusicInfo_t *mi;
    for ( mi = music_info + MKW_N_MUSIC-1; mi >= music_info; mi-- )
    {
	const uint id = mi->track < 0 ? mi->id : mi->track;

	ccp src = mi->name;
	char *dest = name;
	while ( ( *dest++ = toupper((int)*src++) ) != 0 )
	    ;
	DefineIntVar(vm,name,id);
	if ( mi->flags & 2 )
	{
	    DefineIntVar(vm,name+1,id);
	    if ( mi->flags & 4 )
	    {
		name[1] = 'R';
		DefineIntVar(vm,name,id);
		DefineIntVar(vm,name+1,id);
	    }
	}
	else if ( mi->flags & 4 )
	{
	    name[0] = 'R';
	    DefineIntVar(vm,name,id);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: management		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_dollar
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

    res->i	= n_param;
    res->mode	= VAR_INT;
    return ERR_OK;
}

FuncParam_t FuncDollar;

///////////////////////////////////////////////////////////////////////////////

static enumError F_echo
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param>=0);

    uint i;
    for ( i = 0; i < n_param; i++ )
    {
	fprintf(stdlog,"  P.%-3u = ",i+1);
	PrintV(stdlog,param+i,0);
	fputc('\n',stdlog);
    }

    AssignVar(res,param);
    return ERR_OK;
}

FuncParam_t FuncDollar;

///////////////////////////////////////////////////////////////////////////////

static enumError F_line
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

    res->i	= si && si->cur_file ? si->cur_file->line : 0;
    res->mode	= VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_sourceLevel
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

    res->i	= si ? si->n_files : 0;
    res->mode	= VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_ifLevel
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

    res->i	= si && si->cur_file ? si->cur_file->if_level : 0;
    res->mode	= VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_loopLevel
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

    res->i	= si && si->cur_file ? si->cur_file->loop_level : 0;
    res->mode	= VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_loopCount
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

    int loop_level = -1;
    if (si)
    {
	DASSERT(si->cur_file);
	if ( n_param > 0 )
	{
	    loop_level = GetIntV(param);
	    if ( loop_level < 0 )
		loop_level += MAX_LOOP_DEPTH;
	}
	loop_level = si->cur_file->loop_level - 1;
    }

    res->i = loop_level >= 0 && loop_level < MAX_LOOP_DEPTH
		? si->cur_file->loop[loop_level].count_val
		: -1;
    res->mode	= VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_status
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

    if (si)
    {
	res->mode = VAR_INT;
	res->i = si->func_status;
    }
    else
	res->mode = VAR_UNSET;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////	    parser functions: source identification	///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_isKCL
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

    res->i    = si && si->kcl;
    res->mode = VAR_INT;
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

    res->i    = si && si->kmp;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isLEX
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

    res->i    = si && si->lex;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isMDL
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

    res->i    = si && si->mdl;
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: macro support		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_call_macro
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

    //--- some tests

    if (!si)
    {
	res->mode = VAR_UNSET;
	return ERR_OK;
    }

    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    static int depth = 0;
    if ( depth > MAX_FUNCTION_DEPTH )
    {
	si->total_err++;
	ERROR0(ERR_WARNING,
		"To many nested function calls (max=%u) [%s @%u]\n",
		MAX_FUNCTION_DEPTH, sf->name, sf->line );
	res->mode = VAR_UNSET;
	return ERR_WARNING;
    }
    depth++;

    //--- use macro body

    ScanMacro_t *macro = fpar->macro;
    DASSERT(macro);

    const uint saved_n_files = si->n_files;
    si->n_files = 0;
    si->cur_file = &empty_scan_file;
    ScanFile_t *newsf
	= AddSF(si,macro->data,macro->data_size,macro->src_name,sf->revision,0);
    DASSERT(newsf);
    newsf->line = macro->line0;
    VarMap_t *pvar = &newsf->pvar;

    //--- setup parameters

    uint i;
    for ( i = 1; i <= n_param; i++ )
    {
	char pname[20];
	snprintf(pname,sizeof(pname),"$%u",i);
	Var_t *pv = InsertVarMap(pvar,pname,false,0,0);
	DASSERT(pv);
	AssignVar(pv,param++);
    }
    Var_t *var = InsertVarMap(pvar,"$N",false,0,0);
    DASSERT(var);
    AssignIntV(var,n_param);

    //--- scan source

    while (NextLineSI(si,false,true))
	;

    //--- close source

    while (DropSF(si))
	;
    si->cur_file = sf;
    si->n_files = saved_n_files;

    //--- get result and terminate

    AssignVar(res,&si->last_result);
    depth--;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_result
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

    if (si)
	AssignVar(res,&si->last_result);
    else
	res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_param
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


    const int idx = GetIntV(param);
    if ( idx > 0 && si )
    {
	DASSERT(si->cur_file);
	const Var_t *var = FindVarMap(&si->cur_file->pvar,"$N",0);
	if (var)
	{
	    const int n = GetIntV(var);
	    if ( idx <= n )
	    {
		char varname[20];
		snprintf(varname,sizeof(varname),"$%u",idx);
		var = FindVarMap(&si->cur_file->pvar,varname,0);
		if (var)
		{
		    AssignVar(res,var);
		    return ERR_OK;
		}
	    }
	}
    }

    res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isMacro
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

    res->mode = VAR_INT;
    res->i = 0;
    if (si)
    {
	const Var_t *vp = FindVarMap(&si->macro,param->name,0);
	if (vp)
	{
	    ScanMacro_t *macro = vp->macro;
	    DASSERT(macro);
	    res->i = macro->is_function ? 2 : 1;
	}
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isFunction
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

    res->mode = VAR_INT;
    res->i = FindVarMap(&parser_func,param->name,0) != 0;
    if (si)
    {
	const Var_t *vp = FindVarMap(&si->macro,param->name,0);
	if (vp)
	{
	    ScanMacro_t *macro = vp->macro;
	    DASSERT(macro);
	    if (macro->is_function)
		res->i |= 2;
	}
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: variable status	///////////////
///////////////////////////////////////////////////////////////////////////////

static const Var_t * find_var ( ScanInfo_t * si, char * name, int *xyz )
{
    DASSERT(name);
    DASSERT(xyz);

    const Var_t *var = FindVarSI(si,name,false);
    if (var)
    {
	*xyz = -1;
	return var;
    }

    const uint nlen = strlen(name);
    if ( nlen > 2
	&& name[nlen-2] == '.'
	&& name[nlen-1] >= 'X'
	&& name[nlen-1] <= 'Z' )
    {
	name[nlen-2] = 0;
	var = FindVarSI(si,name,false);
	if ( var && var->mode == VAR_VECTOR )
	{
	    *xyz = name[nlen-1] - 'X';
	    return var;
	}
    }

    *xyz = -1;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isDef
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = !var ? 0 : xyz < 0 ? 2 : 1;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isInt
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = !var || var->mode != VAR_INT ? 0 : var->i > 0 ? 2 : 1;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isFloat
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = !var || var->mode != VAR_DOUBLE ? 0 : var->d > 0.0 ? 2 : 1;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isVector
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = !var || var->mode != VAR_VECTOR ? 0 : var->d > 0.0 ? 2 : 1;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isScalar
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    if (!var)
	res->i = 0;
    else if ( var->mode == VAR_INT )
	res->i = var->i > 0 ? 2 : 1;
    else if ( var->mode == VAR_DOUBLE )
	res->i = var->d > 0.0 ? 2 : 1;
    else
	res->i = 0;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isNumeric
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = var && var->mode != VAR_UNSET;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isStr
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = !var || var->mode != VAR_STRING ? 0 : var->str_len > 0 ? 2 : 1;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_type
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

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    res->i = !var ? -1 : xyz >= 0 ? VAR_X+xyz : var->mode;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_var
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
    DASSERT(n_param>=2);

    int xyz;
    const Var_t *var = find_var(si,(char*)param->name,&xyz);
    AssignVar( res, var && var->mode != VAR_UNSET ? var : param+1 );
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: type conversions	///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_int
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

    const int i = GetIntV(param);
    res->i = i;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_float
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

    const double d = GetDoubleV(param);
    res->d = d;
    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_scalar
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

    Var_t temp;
    if ( param->mode == VAR_STRING )
    {
	InitializeV(&temp);
	ScanValueV(&temp,param,"scalar()");
	param = &temp;
    }

    switch (param->mode)
    {
	case VAR_DOUBLE:
	case VAR_VECTOR:
	    {
		const double d = GetDoubleV(param);
		res->d = d;
		res->mode = VAR_DOUBLE;
	    }
	    break;

	case VAR_INT:
	    {
		const int i = GetIntV(param);
		res->i = i;
		res->mode = VAR_INT;
	    }
	    break;

	default:
	    res->i = 0;
	    res->mode = VAR_INT;
	    break;
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_str
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

    AssignStringVV(res,param);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_scanval
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

    return ScanValueV(res,param,"scanVal()");
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_scanexpr
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

    return ScanExprV(res,param,"scanExpr()");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: endian support	///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_be
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
    DASSERT(n_param==2);

    int i = GetIntV(param);
    switch (GetIntV(param+1))
    {
	case 1:  res->int_mode = IMD_BE1; i &= 0xff; break;
	case 2:  res->int_mode = IMD_BE2; i &= 0xffff; break;
	case 3:  res->int_mode = IMD_BE3; i &= 0xffffff; break;
	case 4:  res->int_mode = IMD_BE4; i &= 0xffffffff; break;
	case 5:  res->int_mode = IMD_BE5; i &= 0xffffffffff; break;
	case 6:  res->int_mode = IMD_BE6; i &= 0xffffffffffff; break;
	case 7:  res->int_mode = IMD_BE7; i &= 0xffffffffffffff; break;
	case 8:  res->int_mode = IMD_BE8; break;
	default: res->int_mode = IMD_BE0; break;
    }
    res->i = i;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_le
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
    DASSERT(n_param==2);

    int i = GetIntV(param);
    switch (GetIntV(param+1))
    {
	case 1:  res->int_mode = IMD_LE1; i &= 0xff; break;
	case 2:  res->int_mode = IMD_LE2; i &= 0xffff; break;
	case 3:  res->int_mode = IMD_LE3; i &= 0xffffff; break;
	case 4:  res->int_mode = IMD_LE4; i &= 0xffffffff; break;
	case 5:  res->int_mode = IMD_LE5; i &= 0xffffffffff; break;
	case 6:  res->int_mode = IMD_LE6; i &= 0xffffffffffff; break;
	case 7:  res->int_mode = IMD_LE7; i &= 0xffffffffffffff; break;
	case 8:  res->int_mode = IMD_LE8; break;
	default: res->int_mode = IMD_LE0; break;
    }
    res->i = i;
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: access functions	///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_select
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
    DASSERT(n_param>=2);

    int sel = GetIntV(param) + 1;
    if ( sel < 1 )
	sel = 1;
    else if ( sel >= n_param )
	sel = n_param-1;

    AssignVar(res,param+sel);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: access functions	///////////////
///////////////////////////////////////////////////////////////////////////////

static int getpos_beg ( int pos, int len )
{
    if ( pos < 0 )
    {
	pos += len;
	return pos < 0 ? 0 : pos;
    }
    return pos < len ? pos : len;
}

//-----------------------------------------------------------------------------

static enumError extract_beg
(
    struct Var_t	* res,		// store result here
    struct Var_t	* str,		// string param
    int			p1,		// start position
    int			p2		// length to extract
)
{
    ToStringV(str);
    DASSERT( str->mode == VAR_STRING ) ;
    p1 = getpos_beg(p1,str->str_len);
    p2 = getpos_beg(p2,str->str_len);
    const int len = p2 - p1;
    AssignStringVS( res, str->str + p1, len<0 ? 0 : len );
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_left
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
    DASSERT(n_param>=2);

    return extract_beg(res,param,0,GetIntV(param+1));
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_right
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
    DASSERT(n_param>=2);

    return extract_beg(res,param,-GetIntV(param+1),INT_MAX);
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_mid
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
    DASSERT(n_param>=2);

    int p1 = GetIntV(param+1);
    int p2 = n_param > 2 && param[2].mode != VAR_UNSET
		? GetIntV(param+2) : param->str_len;
    if ( p2 >= 0 )
	p2 += p1;
    return extract_beg(res,param,p1,p2);
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_extract
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
    DASSERT(n_param>=3);

    return extract_beg(res,param,GetIntV(param+1),GetIntV(param+2));
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_remove
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
    DASSERT(n_param>=3);
    DASSERT( param->mode == VAR_STRING ) ;

    const int p1 = getpos_beg( GetIntV(param+1), param->str_len );
    const int p2 = getpos_beg( GetIntV(param+2), param->str_len );
    if ( p1 < p2 )
	AssignStringVS2(res,param->str,p1,param->str+p2,param->str_len-p2);
    else
	AssignStringVV(res,param);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: print()		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct print_par_t
{
    int			index;		// index of next parameter: 1..n-1
    int			n;		// number of parameters
    Var_t		*list;		// list of parameters

    // stats
    int			max_index;	// index of max used parmeter
    int			select;		// counter: index was selected
}
print_par_t;

///////////////////////////////////////////////////////////////////////////////

static Var_t * get_print_par ( print_par_t *pp )
{
    DASSERT(pp);

    if ( pp->max_index < pp->index )
	 pp->max_index = pp->index;

    if ( pp->index >= pp->n )
    {
	static Var_t temp = {0};
	ResetV(&temp);
	return &temp;
    }

    return pp->list + pp->index++;
}

///////////////////////////////////////////////////////////////////////////////

static ccp get_h_format ( double d )
{
    const double a = fabs(d);
    return !a || a < 1e10 && a >= 0.1 ? "%5.3f" : "%9.7e";
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_print
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
    DASSERT( param->mode == VAR_STRING ) ;

    if ( n_param < 1 || param->mode != VAR_STRING )
    {
	AssignStringVS(res,0,0);
	return ERR_WARNING;
    }


    //--- prepare vars

    DEFINE_VAR(collect);
    collect.mode = VAR_STRING;

    const uint max_fw = 100;

    char fbuf[50];	// buffer for created format string
    char buf[5000];	// working buffer
    char *bufend = buf + sizeof(buf) - 3*max_fw - 20;
    char *dest = buf;

    ccp format = param->str ? param->str : "";
    ccp end = format + param->str_len;

    print_par_t pp;
    memset(&pp,0,sizeof(pp));
    pp.index	= 1;
    pp.n	= n_param;
    pp.list	= param;

    bool warnings = si && !si->no_warn;


    //--- main loop

    while ( format < end )
    {
	noPRINT("->|%.*s|\n",(int)(end-format),format);

	if ( dest >= bufend )
	{
	    // buffer allmost full
	    AppendStringVS(&collect,buf,dest-buf);
	    dest = buf;
	}

	if ( *format != '%' )
	{
	    *dest++ = *format++;
	    continue;
	}

	format++;
	if ( format < end && *format == '%' )
	{
	    *dest++ = *format++;
	    continue;
	}


	//--- now we have a print instruction
	//--- scan parameter index

	if ( *format >= '1' && *format <= '9' )
	{
	    int index = 0;
	    ccp f = format;
	    while ( *f >= '0' && *f <= '9' )
		index = index * 10 + *f++ - '0';
	    if ( *f == '$' )
	    {
		if ( index > 0 && index < n_param )
		{
		    pp.index = index;
		    pp.select++;
		}
		else if (warnings)
		{
		    ScanFile_t *sf = si->cur_file;
		    DASSERT(sf);
		    ERROR0(ERR_WARNING,
				"Parameter %d not defined, available 1..%d [%s @%u]: %.*s\n",
				index, pp.n-1, sf->name, sf->line,
				(int)(f-format+2), format-1 );
		}
		format = f+1;
	    }
	}


	//----- scan flags

	ccp  flags_begin	= format;
	int  alternative	= 0;
	char padding		= ' ';
	bool left_adjust	= false;
	char sign		= 0;
	bool right_sign		= false;
	bool grouping		= false;
	bool quote		= false;

	for(;;)
	{
	  switch(*format++)
	  {
	    case '#': // alternative format
		alternative = 1;
		break;

	    case '0': // zero padding
		padding = '0';
		break;

	    case '-': // left adjusted
		left_adjust = true;
		break;

	    case ' ': // blank before a positive number
		if (!sign)
		    sign = ' ';
		break;

	    case '+': // '+' before a positive number (ignore blank above)
		sign = '+';
		break;

	    case '>': // (extension)
		right_sign = true;
		break;

	    case '\'': // (extension)
		grouping = true;
		break;

	    case '"': // (extension)
		quote = true;
		break;

	    default:
		goto end_scan_flags;
	  }
	}
	end_scan_flags:;
	ccp flags_end = --format;


	//----- scan field width

	int fw = 0;
	if ( *format == '*' )
	{
	    format++;
	    const Var_t *v = get_print_par(&pp);
	    fw = GetIntV(v);
	    if ( fw < 0 )
	    {
		left_adjust = true;
		fw = -fw;
	    }
	}
	else if ( *format >= '0' && *format <= '9' )
	{
	    while ( *format >= '0' && *format <= '9' )
		fw = fw * 10 + *format++ - '0';
	}
	if ( fw > max_fw )
		fw = max_fw;


	//----- scan precision

	int prec = -1;
	if ( *format == '.' )
	{
	    format++;
	    prec = 0;

	    if ( *format == '*' )
	    {
		format++;
		const Var_t *v = get_print_par(&pp);
		prec = GetIntV(v);
	    }
	    else
	    {
		while ( *format >= '0' && *format <= '9' )
		    prec = prec * 10 + *format++ - '0';
	    }
	    if ( prec > 50 )
		prec = 50;
	}


	//-- scan variants

	int  mode_must_match	= 0;
	bool force_vector	= false;
	bool enclose_vector	= false;
	for(;;)
	{
	  switch(*format++)
	  {
	    case 'n':
		if (!mode_must_match)
		    mode_must_match = 1;
		break;

	    case 'N':
		mode_must_match = 2;
		break;

	    case 'v':
		enclose_vector = true;
		break;

	    case 'V':
		force_vector = true;
		break;

	    case 'l':
	    case 'z':
		// ignored
		break;

	    default:
		goto end_scan_variants;
	  }
	}
	end_scan_variants:
	format--;


	//-- scan conversion character

	Var_t *v = get_print_par(&pp);
	char convert = *format++;
	const bool human = convert == 'h';
	if ( human )
	{
	    switch(v->mode)
	    {
		case VAR_UNSET:   convert = '-'; break;
		case VAR_INT:     convert = 'd'; break;
		case VAR_DOUBLE:
		case VAR_VECTOR:  break; // 'h' is ok
		case VAR_STRING:  convert = 's'; break;
	    }
	}
	else if ( convert == 'y' )
	{
	    switch(v->mode)
	    {
		case VAR_UNSET:   convert = 'n'; break;
		case VAR_INT:     convert = 'd'; break;
		case VAR_DOUBLE:
		case VAR_VECTOR:  convert = 'a'; enclose_vector = true; break;
		case VAR_STRING:  convert = 's'; quote = true; break;
	    }
	}

	noPRINT("FORMAT |%.*s| '%c' alt=%d, grp=%d, pad=%c, left=%d, sign%s=%c,"
		" fw=%d.%d, vec=%d,%d, idx=%d/%d/%d\n",
		(int)(format-flags_begin), flags_begin, convert,
		alternative, grouping, padding, left_adjust,
		right_sign ? "/r" : "", sign ? sign : '0',
		fw, prec, force_vector, enclose_vector, pp.index, pp.max_index, pp.n );

	switch(convert)
	{
	  //--------------------------------------------------

	  case 'b':
	  case 'o':
	  case 'x':
	  case 'X':
	  case 'd':
	  case 'i':
	  case 'u':
	    if ( !mode_must_match || v->mode == VAR_INT )
	    {
		if ( prec < 0 )
		    prec = 1;

		int  shift	= 0;
		uint mask	= 0;
		ccp  hex	= LoDigits;
		int  gstep	= 4;
		char gchar	= ':';
		ccp  alt	= "0x";
		bool is_signed	= false;
		uint num	= GetIntV(v);

		switch(convert)
		{
		 case 'b': // binary (extension!)
		    shift	= 1;
		    mask	= 0x1;
		    gstep	= 4;
		    gchar	= ':';
		    if (alternative)
		    {
			alternative = 2;
			alt = "0b";
		    }
		    break;

		 case 'o': // octal
		    shift	= 3;
		    mask	= 0x7;
		    gstep	= 4;
		    gchar	= ':';
		    if ( alternative && num )
		    {
			alternative = 1;
			alt = "0";
		    }
		    else
			alternative = 0;
		    break;

		 case 'X': // hexadezimal with A-F
		    hex		= HiDigits;
		    alt		= "0X";
		    // fall through

		 case 'x': // hexadezimal with a-f
		    shift	= 4;
		    mask	= 0xf;
		    gstep	= 4;
		    gchar	= ':';
		    if (alternative)
			alternative = 2;
		    break;

		 case 'd':
		 case 'i':
		    is_signed = true;
		    // fall through
		 case 'u':
		    gstep	= 3;
		    gchar	= ' ';
		    break;
		}

		int gcount = grouping ? gstep : INT_MAX;
		char *nend = buf + sizeof(buf) - 1;
		char *nptr = nend;

		if (mask)
		{
		    while ( prec-- > 0 || num )
		    {
			if (!--gcount)
			{
			    gcount = gstep;
			    *--nptr = gchar;
			}
			*--nptr = hex[ num & mask ];
			num >>= shift;
			DASSERT( nptr >= bufend );
		    }
		}
		else // !mask
		{
		    // decimal
		    alternative = 0;
		    if (is_signed)
		    {
			if ( (int)num < 0 )
			{
			    sign = '-';
			    num = -num;
			}
			if ( right_sign )
			{
			    *--nptr = sign ? sign : ' ';
			    sign = 0;
			}
			if (sign)
			{
			    alternative = 1;
			    alt = &sign;
			}
		    }
		    else
			sign = 0;

		    while ( prec-- > 0 || num )
		    {
			if (!--gcount)
			{
			    gcount = gstep;
			    *--nptr = gchar;
			}
			lldiv_t ld = lldiv(num,10);
			*--nptr = '0'+ld.rem;
			num = ld.quot;
			DASSERT( nptr >= bufend );
		    }

		} // !mask

		int fill = fw - (nend-nptr) - alternative;
		if ( grouping && padding == '0' )
		{
		    while ( fill-- > 0 )
		    {
			if ( !--gcount )
			{
			    gcount = gstep;
			    *--nptr = gchar;
			    if ( --fill < 0 )
				break;
			}
			*--nptr = '0';
		    }
		}
		else if ( !left_adjust && padding == '0' )
		    while ( fill-- > 0 )
			*--nptr = '0';

		if (alternative)
		{
		    PRINT("ALT[%u]: %2x %2x\n",alternative,alt[0],alt[1]);
		    alt += alternative;
		    while ( alternative-- > 0 )
			*--nptr = *--alt;
		}

		if (!left_adjust)
		    while ( fill-- > 0 )
			*dest++ = padding;

		const uint len = nend - nptr;
		memcpy(dest,nptr,len);
		dest += len;

		if (left_adjust)
		    while ( fill-- > 0 )
			*dest++ = padding;
	    }
	    break;

	  //--------------------------------------------------

	  case 'e':
	  case 'E':
	  case 'f':
	  case 'F':
	  case 'g':
	  case 'G':
	  case 'a':
	  case 'A':
	    if	(  !mode_must_match
		|| mode_must_match == 1 && ( v->mode == VAR_DOUBLE || v->mode == VAR_VECTOR )
		|| ( force_vector ? v->mode == VAR_VECTOR : v->mode == VAR_DOUBLE )
		)
	    {
		// ***** 'double' wrapper to sprintf() *****

		char *fptr = fbuf;
		*fptr++ = '%';
		uint flags_len = flags_end - flags_begin;
		if ( flags_len > sizeof(fbuf)-25 )
		     flags_len = sizeof(fbuf)-25;
		memcpy(fptr,flags_begin,flags_len);
		fptr += flags_len;

		if ( fw < 0 )
		    fw = 0;

		if ( prec < 0 )
		    fptr += sprintf(fptr,"%u",fw);
		else
		    fptr += sprintf(fptr,"%u.%u",fw,prec);
		*fptr++ = convert;
		*fptr = 0;

		if ( force_vector || v->mode == VAR_VECTOR )
		{
		    ToVectorV(v);
		    if (enclose_vector)
		    {
			*dest++ = 'v';
			*dest++ = '(';
		    }

		    dest = snprintfE(dest,buf+sizeof(buf),fbuf,v->x); *dest++ = ',';
		    dest = snprintfE(dest,buf+sizeof(buf),fbuf,v->y); *dest++ = ',';
		    dest = snprintfE(dest,buf+sizeof(buf),fbuf,v->z);

		    if (enclose_vector)
			*dest++ = ')';
		}
		else
		{
		    noPRINT("DOUBLE [%s] %g\n",fbuf,GetDoubleV(v));
		    dest = snprintfE(dest,buf+sizeof(buf),fbuf,GetDoubleV(v));
		}
	    }
	    break;

	  //--------------------------------------------------

	  case 'h':
	    {
		char *start = buf + sizeof(buf) - 100;
		char *ptr = start;
		if ( force_vector || v->mode == VAR_VECTOR )
		{
		    ToVectorV(v);
		    if (enclose_vector)
		    {
			*ptr++ = 'v';
			*ptr++ = '(';
		    }

		    ptr = snprintfE(ptr,buf+sizeof(buf),get_h_format(v->x),v->x);
		    *ptr++ = ',';
		    ptr = snprintfE(ptr,buf+sizeof(buf),get_h_format(v->y),v->y);
		    *ptr++ = ',';
		    ptr = snprintfE(ptr,buf+sizeof(buf),get_h_format(v->y),v->z);
		    if (enclose_vector)
			*ptr++ = ')';
		}
		else
		{
		    noPRINT("DOUBLE [%s] %g\n",fbuf,GetDoubleV(v));
		    const double d = GetDoubleV(v);
		    ptr = snprintfE(ptr,buf+sizeof(buf),get_h_format(d),d);
		}

		const int len = ptr - start;
		int fill = fw - len;
		if (!left_adjust)
		    while ( fill-- > 0 )
			*dest++ = ' ';
		memmove(dest,start,len);
		dest += len;
		while ( fill-- > 0 )
		    *dest++ = ' ';
	    }
	    break;


	  //--------------------------------------------------

	  case 's':
	    if ( !mode_must_match || v->mode == VAR_STRING )
	    {
		// write direct to output
		AppendStringVS(&collect,buf,dest-buf);
		dest = buf;

		ToStringV(v);
		ccp  str = v->str;
		uint len = v->str_len;

		if ( quote || alternative )
		{

		    PrintEscapedString(buf+1,sizeof(buf)-5,
			    v->str, v->str_len, CHMD__ALL, '"', &len );
		    if (quote)
		    {
			str = buf;
			*buf = '"';
		    }
		    else
			str = buf+1;
		}

		if ( prec > 0 )
		{
		    if (quote)
		    {
			prec -= 2;
			if ( prec < 1 )
			    prec = 1;
		    }
		    if ( len > prec )
			len = prec;
		}

		if ( len < fw && !left_adjust )
		    AppendStringVC(&collect,' ',fw-len);
		AppendStringVS(&collect,str,len);
		if (quote)
		    AppendStringVC(&collect,'"',1);
		if ( len < fw && left_adjust )
		    AppendStringVC(&collect,' ',fw-len);
	    }
	    break;

	  //--------------------------------------------------

	  case 'n':
	    dest = StringCopyE(dest,buf+sizeof(buf),"$NONE");
	    break;

	  case 'q':
	    warnings = false;
	    break;

	  case '-':
	    // nothing to do
	    break;

	  default:
	    // [[???]] error message
	    break;
	}
    }

    if (warnings)
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);

	noPRINT("idx=%d, max=%d, n=%d, select=%d\n",
		pp.index, pp.max_index, pp.n, pp.select );

	if ( pp.max_index >= pp.n )
	    ERROR0(ERR_WARNING,
			"Access to non existing argument [%s @%u]: %s\n",
			sf->name, sf->line, param->str );
	else if ( pp.max_index < pp.n-1 && !pp.select )
	    ERROR0(ERR_WARNING,
			"Only %u of %u arguments used [%s @%u]: %s\n",
			pp.max_index, pp.n-1, sf->name, sf->line, param->str );
    }

    if ( dest > buf )
	AppendStringVS(&collect,buf,dest-buf);
    MoveDataV(res,&collect);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: timer			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_sec
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

    res->i = GetTimerMSec()/1000;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_mSec
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

    res->i = GetTimerMSec();
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_uSec
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

    res->i = GetTimerUSec();
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: basic vector		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_v
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
    noPRINT("FUNC v(%p,%u,%p)\n",param,n_param,si);

    switch(n_param)
    {
	case 0:
	    res->x = res->y = res->z = 0.0;
	    break;

	case 1:
	    if ( param->mode == VAR_VECTOR )
		AssignVar(res,param);
	    else
	    {
		res->x = GetDoubleV(param);
		res->y = res->z = 0.0;
	    }
	    break;

	case 2:
	    res->x = GetXDoubleV(param);
	    res->y = 0.0;
	    res->z = GetZDoubleV(param+1);
	    break;

	default:
	    res->x = GetXDoubleV(param);
	    res->y = GetYDoubleV(param+1);
	    res->z = GetZDoubleV(param+2);
	    break;
    }
    res->mode = VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_v3
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

    res->x = res->y = res->z = GetDoubleV(param);
    res->mode = VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_vx
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

    res->x	= GetXDoubleV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	res->y	= param[1].y;
	res->z	= param[1].z;
    }
    else
    {
	res->y	= 0.0;
	res->z	= 0.0;
    }
    res->mode	= VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_vy
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

    res->y	= GetYDoubleV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	res->x	= param[1].x;
	res->z	= param[1].z;
    }
    else
    {
	res->x	= 0.0;
	res->z	= 0.0;
    }
    res->mode	= VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_vz
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

    res->z	= GetZDoubleV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	res->x	= param[1].x;
	res->y	= param[1].y;
    }
    else
    {
	res->x	= 0.0;
	res->y	= 0.0;
    }
    res->mode	= VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_vd
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
    DASSERT(n_param>=2);

    const double len   = GetDoubleV(param);
    const double angle = GetDoubleV(param+1) * (M_PI/180.0);
    res->x	= sin(angle) * len;
    res->z	= cos(angle) * len;
    res->y	= n_param > 2 ? GetYDoubleV(param+2) : 0.0;
    res->mode	= VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_x
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
    DASSERT( fpar->user_id >= 0 && fpar->user_id <= 2 );

    const Var_t *end_param;
    for ( end_param = param + n_param; param < end_param; param++ )
	if ( param->mode == VAR_VECTOR )
	{
	    res->d = param->v[fpar->user_id];
	    goto exit;
	}
    res->d = GetDoubleV(param-1);

 exit:
    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: 2D vector		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_sideOfLine
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
    DASSERT(n_param>=3);

    ToVectorV(param);
    ToVectorV(param+1);
    ToVectorV(param+2);
    const double dstat = SideOfLineXZ(param[0],param[1],param[2]);
    res->i = dstat < 0.0 ? -1 : dstat > 0.0;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_ptInConvexPolygon
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
    DASSERT(n_param>=4);

    uint count = 0;

    int n_pol = (int)n_param - 1;
    if ( n_pol >= 3 )
    {
	const uint max_pol = 100;
	if ( n_pol > max_pol )
	    n_pol = max_pol;

	double pol[max_pol*2], pts[2];

	ToVectorV(param);
	pts[0] = param->x;
	pts[1] = param->z;

	double *p = pol;
	uint i;
	for ( i = n_pol; i > 0; i-- )
	{
	    ToVectorV(++param);
	    *p++ = param->x;
	    *p++ = param->z;
	}

	int dir;
	count = PointsInConvexPolygonD(pts,1,pol,n_pol,false,&dir);
	si->func_status = -dir;
    }

    res->i = count;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_ptsInConvexTri
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
    DASSERT(n_param>=4);

    uint count = 0;

    int n_pts = (int)n_param - 3;
    if ( n_pts > 0 )
    {
	const uint max_pts = 100;
	if ( n_pts > max_pts )
	    n_pts = max_pts;


	double tri[3*2], pts[max_pts*2], *p;
	uint i;

	for ( i = 3, p = tri; i > 0; i--, param++ )
	{
	    ToVectorV(param);
	    *p++ = param->x;
	    *p++ = param->z;
	}

	for ( i = n_pts, p = pts; i > 0; i--, param++ )
	{
	    ToVectorV(param);
	    *p++ = param->x;
	    *p++ = param->z;
	}

	int dir;
	count = PointsInConvexPolygonD(pts,n_pts,tri,3,false,&dir);
	si->func_status = -dir;
    }

    res->i = count;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_ptsInConvexQuad
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
    DASSERT(n_param>=5);

    uint count = 0;

    int n_pts = (int)n_param - 3;
    if ( n_pts > 0 )
    {
	const uint max_pts = 100;
	if ( n_pts > max_pts )
	    n_pts = max_pts;


	double quad[3*2], pts[max_pts*2], *p;
	uint i;

	for ( i = 4, p = quad; i > 0; i--, param++ )
	{
	    ToVectorV(param);
	    *p++ = param->x;
	    *p++ = param->z;
	}

	for ( i = n_pts, p = pts; i > 0; i--, param++ )
	{
	    ToVectorV(param);
	    *p++ = param->x;
	    *p++ = param->z;
	}

	int dir;
	count = PointsInConvexPolygonD(pts,n_pts,quad,4,false,&dir);
	si->func_status = -dir;
    }

    res->i = count;
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: 3D vector		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_dot
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
    DASSERT(n_param>=2);

    ToVectorV(param);
    ToVectorV(param+1);
    res->d = DotProd3(param[0],param[1]);
    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_cross
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
    DASSERT(n_param>=2);

    ToVectorV(param);
    ToVectorV(param+1);
    CrossProd3x(*res,param[0],param[1]);
    res->mode = VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_unit
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

    double dx, dy, dz;
    ToVectorV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	dx = param[1].x - param[0].x;
	dy = param[1].y - param[0].y;
	dz = param[1].z - param[0].z;
    }
    else
    {
	dx = param->x;
	dy = param->y;
	dz = param->z;
    }

    const double len = sqrt ( dx*dx + dy*dy + dz*dz );
    if ( len < 1e-9 )
	res->x = res->y = res->z = 0.0;
    else
    {
	res->x = dx / len;
	res->y = dy / len;
	res->z = dz / len;
    }
    res->mode = VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static double3 calced_normals[3];

//-----------------------------------------------------------------------------

static enumError F_calcNormals
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
    DASSERT(n_param>=2);

    ToVectorV(param);
    ToVectorV(param+1);
    double3 *helper = 0;
    if ( n_param > 2 && param[2].mode != VAR_UNSET )
    {
	ToVectorV(param+2);
	helper = &param[2].d3;
    }
    double r = n_param > 3 && param[3].mode != VAR_UNSET ? GetDoubleV(param+3) : 1.0;
    res->d = CalcNormals(calced_normals,&param[0].d3,&param[1].d3,helper,r);
    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError F_getNormal
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

    const uint idx = GetIntV(param);
    if ( idx < 3 )
    {
	res->x = calced_normals[idx].x;
	res->y = calced_normals[idx].y;
	res->z = calced_normals[idx].z;
	res->mode = VAR_VECTOR;
    }
    else
	res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_len
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

    if ( param->mode == VAR_STRING )
    {
	AssignIntV(res,param->str_len);
	return ERR_OK;
    }

    ToVectorV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	const double dx = param[1].x - param[0].x;
	const double dy = param[1].y - param[0].y;
	const double dz = param[1].z - param[0].z;
	res->d = dx*dx + dy*dy + dz*dz;
    }
    else
	res->d	= param->x * param->x
		+ param->y * param->y
		+ param->z * param->z;

    if (!fpar->user_id)
	res->d = sqrt(res->d);

    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_hLen
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

    ToVectorV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	const double dx = param[1].x - param[0].x;
	const double dz = param[1].z - param[0].z;
	res->d = dx*dx + dz*dz;
    }
    else
	res->d = param->x * param->x + param->z * param->z;

    if (!fpar->user_id)
	res->d = sqrt(res->d);

    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_pos
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
    DASSERT(n_param>=3);

    if ( param[0].mode == VAR_VECTOR )
    {
	ToVectorV(param+1);
	ToVectorV(param+2);
	res->x = param[1].x + param[0].x * ( param[2].x - param[1].x );
	res->y = param[1].y + param[0].y * ( param[2].y - param[1].y );
	res->z = param[1].z + param[0].z * ( param[2].z - param[1].z );
	res->mode = VAR_VECTOR;
    }
    else
    {
	double pos = GetDoubleV(param);

	if ( param[1].mode == VAR_VECTOR || param[2].mode == VAR_VECTOR )
	{
	    ToVectorV(param+1);
	    ToVectorV(param+2);
	    res->x = param[1].x + pos * ( param[2].x - param[1].x );
	    res->y = param[1].y + pos * ( param[2].y - param[1].y );
	    res->z = param[1].z + pos * ( param[2].z - param[1].z );
	    res->mode = VAR_VECTOR;
	}
	else
	{
	    double v1 = GetDoubleV(param+1);
	    res->d = v1 + GetDoubleV(param) * ( GetDoubleV(param+2) - v1 );
	    res->mode = VAR_DOUBLE;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_bezier
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
    DASSERT(n_param>=4);

    //--- determine type of valculation, 1=float, 3=vector

    uint i, n_dim = 1;
    for ( i = 0; i < n_param; i++ )
	if ( param[i].mode == VAR_VECTOR )
	{
	    n_dim = 3;
	    break;
	}

    //--- convert parameters

    if ( n_dim == 1 )
    {
	for ( i = 0; i < n_param; i++ )
	    ToDoubleV(param+i);
	ToDoubleV(res);
    }
    else
    {
	for ( i = 0; i < n_param; i++ )
	    ToVectorV(param+i);

	ToVectorV(res);
    }

    if ( n_param >= 5 )
    {
	// Cubic bezier
	// B(t) = (1-t)^3*P1 + 3*(1-t)^2*t*Pa + 3*(1-t)*t^2*Pb + t^3*P2

	double *r  = res->v;
	double *t  = param[0].v;
	double *p1 = param[1].v;
	double *pa = param[2].v;
	double *pb = param[3].v;
	double *p2 = param[4].v;

	int dim;
	for ( dim = 0; dim < n_dim; dim++, t++ )
	{
	    const double t2 = 1.0 - *t;
	    *r++ =    t2 * t2 * t2 * *p1++
		 + 3* *t * t2 * t2 * *pa++
		 + 3* *t * *t * t2 * *pb++
		 +    *t * *t * *t * *p2++;
	}
    }
    else
    {
	// Quadratic bezier
	// B(t) = (1-t)^2*P1 + 2*(1-t)*t*Pa + t^2*P2

	double *r  = res->v;
	double *t  = param[0].v;
	double *p1 = param[1].v;
	double *pa = param[2].v;
	double *p2 = param[3].v;

	int dim;
	for ( dim = 0; dim < n_dim; dim++, t++ )
	{
	    const double t2 = 1.0 - *t;
	    *r++ =    t2 * t2 * *p1++
		 + 2* *t * t2 * *pa++
		 +    *t * *t * *p2++;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_xRot // == xRot, yRot, zRot
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
    DASSERT(n_param>=2);
    DASSERT(fpar);
    DASSERT( fpar->user_id >= 0 && fpar->user_id <= 2 );

    uint r, a, b;
    switch (fpar->user_id)
    {
	case 0:  r = 0; a = 2; b = 1; ToVectorYV(param); break;
	case 1:  r = 1; a = 0; b = 2; ToVectorZV(param); break;
	default: r = 2; a = 1; b = 0; ToVectorXV(param); break;
    }

    const Var_t *origin = n_param > 2 ? ToVectorV(param+2) : &NullVector;

    const double da  = param->v[a] - origin->v[a];
    const double db  = param->v[b] - origin->v[b];
    const double rad = atan2(da,db) + GetDoubleV(param+1) * (M_PI/180.0);
    const double len = sqrt(da*da+db*db);

    res->v[r]	= param->v[r];
    res->v[a]	= sin(rad) * len + origin->v[a];
    res->v[b]	= cos(rad) * len + origin->v[b];
    res->mode	= VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_rot
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
    DASSERT(n_param>=2);

    ToVectorV(param+0);
    ToVectorV(param+1);
    if (n_param > 2)
    {
	ToVectorV(param+2);
	Rotate(&param[2].d3,&param[1].d3,&param[0].d3,0,1);
    }
    else
	Rotate(0,&param[1].d3,&param[0].d3,0,1);

    AssignVar(res,param);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_axisRot
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
    DASSERT(n_param>=4);

    ToVectorV(param+0);
    ToVectorV(param+2);
    ToVectorV(param+3);
    AxisRotate(&param[2].d3,&param[3].d3,GetDoubleV(param+1),&param[0].d3,0,1);
    AssignVar(res,param);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_aDir
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
    DASSERT( fpar->user_id >= 0 && fpar->user_id <= 2 );

    uint a, b;
    switch (fpar->user_id)
    {
	case 0:  a = 2; b = 1; break;
	case 1:  a = 0; b = 2; break;
	default: a = 1; b = 0; break;
    }

    ToVectorV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	res->d = atan2( param[1].v[a] - param[0].v[a],
			param[1].v[b] - param[0].v[b] ) * (180/M_PI);
    }
    else
	res->d = atan2(param->v[a],param->v[b]) * (180/M_PI);

    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_dir
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

    ToVectorV(param);
    if ( n_param > 1 )
    {
	ToVectorV(param+1);
	res->d3 = CalcDirection3D(&param->d3,&param[1].d3);
    }
    else
	res->d3 = CalcDirection3D(&NullVectorD,&param->d3);

    res->mode = VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_ptsInCuboid
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
    DASSERT(n_param>=3);

    ToVectorV(param+0);
    ToVectorV(param+1);

    int count = 0;
    struct Var_t *ptr = param + 2;
    struct Var_t *end = param + n_param;

    if (!fpar->user_id)
    {
	// strict!
	for ( ; ptr < end; ptr++ )
	{
	    ToVectorV(ptr);
	    if	(    ptr->x >= param[0].x  &&  ptr->x <= param[1].x
		  && ptr->y >= param[0].y  &&  ptr->y <= param[1].y
		  && ptr->z >= param[0].z  &&  ptr->z <= param[1].z
		)
	    {
		count++;
	    }
	}
    }
    else
    {
	// sloppy
	for ( ; ptr < end; ptr++ )
	{
	    ToVectorV(ptr);
	    if	(   ( param[0].x < param[1].x
			? ptr->x >= param[0].x  &&  ptr->x <= param[1].x
			: ptr->x >= param[1].x  &&  ptr->x <= param[0].x )
		 && ( param[0].y < param[1].y
			? ptr->y >= param[0].y  &&  ptr->y <= param[1].y
			: ptr->y >= param[1].y  &&  ptr->y <= param[0].y )
		 && ( param[0].z < param[1].z
			? ptr->z >= param[0].z  &&  ptr->z <= param[1].z
			: ptr->z >= param[1].z  &&  ptr->z <= param[0].z )
		)
	    {
		count++;
	    }
	}
    }

    res->i = count;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_OverlapCubeTri
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
    DASSERT(n_param>=5);

    res->i = OverlapCubeTri(
		&ToVectorV(param+0)->d3,
		GetDoubleV(param+1),
		&ToVectorV(param+2)->d3,
		&ToVectorV(param+3)->d3,
		&ToVectorV(param+4)->d3 );
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: simple mathematical	///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_abs
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

    switch(param->mode)
    {
	case VAR_UNSET:
	    break;

	case VAR_INT:
	    res->i = llabs(param->i);
	    break;

	case VAR_DOUBLE:
	    res->d = fabs(param->d);
	    break;

	case VAR_VECTOR:
	    res->x = fabs(param->x);
	    res->y = fabs(param->y);
	    res->z = fabs(param->z);
	    break;

	case VAR_STRING:
	    DASSERT( res->mode == VAR_UNSET );
	    break;
    }
    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_sign
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

    switch(param->mode)
    {
	case VAR_UNSET:
	    break;

	case VAR_INT:
	    res->i = param->i < 0 ? -1 : param->i > 0;
	    break;

	case VAR_DOUBLE:
	    res->d = param->d < 0.0 ? -1.0 : param->d > 0.0 ? 1.0 : 0.0;
	    break;

	case VAR_VECTOR:
	    res->x = param->x < 0.0 ? -1.0 : param->x > 0.0 ? 1.0 : 0.0;
	    res->y = param->y < 0.0 ? -1.0 : param->y > 0.0 ? 1.0 : 0.0;
	    res->z = param->z < 0.0 ? -1.0 : param->z > 0.0 ? 1.0 : 0.0;
	    break;

	case VAR_STRING:
	    DASSERT( res->mode == VAR_UNSET );
	    break;
    }
    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_min
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
    DASSERT( VAR_VECTOR > VAR_DOUBLE );
    DASSERT( VAR_DOUBLE > VAR_INT );
    DASSERT( VAR_INT    > VAR_UNSET );

    uint i;
    VarMode_t max_mode = VAR_UNSET, ref_idx = 0;
    for ( i = 0; i < n_param; i++ )
	if ( max_mode < param[i].mode && isNumericVM(param[i].mode) )
	{
	     max_mode = param[i].mode;
	     ref_idx = i;
	}
    DASSERT( param[ref_idx].mode == max_mode );
    PRINT("MODE=%d, IDX=%d\n",max_mode,ref_idx);

    switch (max_mode)
    {
	case VAR_UNSET:
	    break;

	case VAR_INT:
	{
	    int min = param[ref_idx].i;
	    for (; n_param > 0; n_param--, param++ )
		if ( param->mode == VAR_INT && min > param->i )
		     min = param->i;
	    res->i = min;
	    break;
	}

	case VAR_DOUBLE:
	{
	    double min = param[ref_idx].d;
	    for (; n_param > 0; n_param--, param++ )
		if ( param->mode != VAR_UNSET )
		{
		    const double temp = GetDoubleV(param);
		    if ( min > temp )
			 min = temp;
		}
	    res->d = min;
	    break;
	}

	case VAR_VECTOR:
	{
	    double xmin = param[ref_idx].x;
	    double ymin = param[ref_idx].y;
	    double zmin = param[ref_idx].z;
	    for (; n_param > 0; n_param--, param++ )
		switch(param->mode)
		{
		    case VAR_UNSET:
			break;

		    case VAR_INT:
		    case VAR_DOUBLE:
			{
			    const double temp = GetDoubleV(param);
			    if ( xmin > temp ) xmin = temp;
			    if ( ymin > temp ) ymin = temp;
			    if ( zmin > temp ) zmin = temp;
			}
			break;

		    case VAR_VECTOR:
			if ( xmin > param->x ) xmin = param->x;
			if ( ymin > param->y ) ymin = param->y;
			if ( zmin > param->z ) zmin = param->z;
			break;

		    case VAR_STRING: // ignore!
			break;
		}
	    res->x = xmin;
	    res->y = ymin;
	    res->z = zmin;
	    break;
	}

	case VAR_STRING: // ignore!
	    break;
    }
    res->mode = max_mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_max
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
    DASSERT( VAR_VECTOR > VAR_DOUBLE );
    DASSERT( VAR_DOUBLE > VAR_INT );
    DASSERT( VAR_INT    > VAR_UNSET );

    uint i;
    VarMode_t max_mode = VAR_UNSET, ref_idx = 0;
    for ( i = 0; i < n_param; i++ )
	if ( max_mode < param[i].mode && isNumericVM(param[i].mode) )
	{
	     max_mode = param[i].mode;
	     ref_idx = i;
	}
    DASSERT( param[ref_idx].mode == max_mode );
    PRINT("MODE=%d, IDX=%d\n",max_mode,ref_idx);

    switch (max_mode)
    {
	case VAR_UNSET:
	    break;

	case VAR_INT:
	{
	    int max = param[ref_idx].i;
	    for (; n_param > 0; n_param--, param++ )
		if ( param->mode == VAR_INT && max < param->i )
		     max = param->i;
	    res->i = max;
	    break;
	}

	case VAR_DOUBLE:
	{
	    double max = param[ref_idx].d;
	    for (; n_param > 0; n_param--, param++ )
		if ( param->mode != VAR_UNSET )
		{
		    const double temp = GetDoubleV(param);
		    if ( max < temp )
			 max = temp;
		}
	    res->d = max;
	    break;
	}

	case VAR_VECTOR:
	{
	    double xmax = param[ref_idx].x;
	    double ymax = param[ref_idx].y;
	    double zmax = param[ref_idx].z;
	    for (; n_param > 0; n_param--, param++ )
		switch(param->mode)
		{
		    case VAR_UNSET:
			break;

		    case VAR_INT:
		    case VAR_DOUBLE:
			{
			    const double temp = GetDoubleV(param);
			    if ( xmax < temp ) xmax = temp;
			    if ( ymax < temp ) ymax = temp;
			    if ( zmax < temp ) zmax = temp;
			}
			break;

		    case VAR_VECTOR:
			if ( xmax < param->x ) xmax = param->x;
			if ( ymax < param->y ) ymax = param->y;
			if ( zmax < param->z ) zmax = param->z;
			break;

		case VAR_STRING: // ignore!
		    break;
		}
	    res->x = xmax;
	    res->y = ymax;
	    res->z = zmax;
	    break;
	}

	case VAR_STRING: // ignore!
	    break;
    }
    res->mode = max_mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_minMax
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
    DASSERT(n_param>=3);

    switch(param->mode)
    {
	case VAR_UNSET:
	    break;

	case VAR_INT:
	{
	    int i = param->i;
	    if ( isNumericVM(param[1].mode) )
	    {
		const int min = GetIntV(param+1);
		if ( i < min )
		     i = min;
	    }
	    if ( isNumericVM(param[2].mode) )
	    {
		const int max = GetIntV(param+2);
		if ( i > max )
		     i = max;
	    }
	    res->i = i;
	    break;
	}

	case VAR_DOUBLE:
	{
	    double d = param->d;
	    if ( isNumericVM(param[1].mode) )
	    {
		const double min = GetDoubleV(param+1);
		if ( d < min )
		     d = min;
	    }
	    if ( isNumericVM(param[2].mode) )
	    {
		const double max = GetDoubleV(param+2);
		if ( d > max )
		     d = max;
	    }
	    res->d = d;
	    break;
	}

	case VAR_VECTOR:
	{
	    double x = param->x;
	    double y = param->y;
	    double z = param->z;
	    if ( isNumericVM(param[1].mode) )
	    {
		const Var_t *min = ToVectorV(param+1);
		if ( x < min->x ) x = min->x;
		if ( y < min->y ) y = min->y;
		if ( z < min->z ) z = min->z;
	    }
	    if ( isNumericVM(param[2].mode) )
	    {
		const Var_t *max = ToVectorV(param+2);
		if ( x > max->x ) x = max->x;
		if ( y > max->y ) y = max->y;
		if ( z > max->z ) z = max->z;
	    }
	    res->x = x;
	    res->y = y;
	    res->z = z;
	    break;
	}

	case VAR_STRING: // ignore!
	    break;
    }

    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_mean
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

    uint count = 0, vcount = 0;
    double sum  = 0.0;
    double xsum = 0.0;
    double ysum = 0.0;
    double zsum = 0.0;
    for ( ; n_param > 0; n_param--, param++ )
    {
	switch(param->mode)
	{
	    case VAR_UNSET:
		break;

	    case VAR_INT:
	    case VAR_DOUBLE:
		{
		    count++;
		    sum += GetDoubleV(param);
		}
		break;

	    case VAR_VECTOR:
		vcount++;
		xsum += param->x;
		ysum += param->y;
		zsum += param->z;
		break;

	    case VAR_STRING: // ignore!
		break;
	}
    }

    if (vcount)
    {
	count += vcount;
	res->x = ( sum + xsum ) / count;
	res->y = ( sum + ysum ) / count;
	res->z = ( sum + zsum ) / count;
	res->mode = VAR_VECTOR;
    }
    else if (count)
    {
	res->d = sum / count;
	res->mode = VAR_DOUBLE;
    }
    else
	res->mode = VAR_UNSET;

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_trunc
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

    switch(param->mode)
    {
	case VAR_UNSET:
	case VAR_INT:
	case VAR_STRING:
	    break;

	case VAR_DOUBLE:
	    res->d = trunc(param->d);
	    break;

	case VAR_VECTOR:
	    res->x = trunc(param->x);
	    res->y = trunc(param->y);
	    res->z = trunc(param->z);
	    break;
    }
    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_floor
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

    switch(param->mode)
    {
	case VAR_UNSET:
	case VAR_INT:
	case VAR_STRING:
	    break;

	case VAR_DOUBLE:
	    res->d = floor(param->d);
	    break;

	case VAR_VECTOR:
	    res->x = floor(param->x);
	    res->y = floor(param->y);
	    res->z = floor(param->z);
	    break;
    }
    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_ceil
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

    switch(param->mode)
    {
	case VAR_UNSET:
	case VAR_INT:
	case VAR_STRING:
	    break;

	case VAR_DOUBLE:
	    res->d = ceil(param->d);
	    break;

	case VAR_VECTOR:
	    res->x = ceil(param->x);
	    res->y = ceil(param->y);
	    res->z = ceil(param->z);
	    break;
    }
    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_round
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

    switch(param->mode)
    {
	case VAR_UNSET:
	case VAR_INT:
	case VAR_STRING:
	    break;

	case VAR_DOUBLE:
	    res->d = round(param->d);
	    break;

	case VAR_VECTOR:
	    res->x = round(param->x);
	    res->y = round(param->y);
	    res->z = round(param->z);
	    break;
    }
    res->mode = param->mode;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_sqrt
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

    switch(param->mode)
    {
	case VAR_UNSET:
	    res->mode = VAR_UNSET;
	    break;

	case VAR_INT:
	    res->d = sqrt(llabs(param->i));
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_DOUBLE:
	    res->d = sqrt(fabs(param->d));
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_VECTOR:
	    res->x = sqrt(fabs(param->x));
	    res->y = sqrt(fabs(param->y));
	    res->z = sqrt(fabs(param->z));
	    res->mode = VAR_VECTOR;
	    break;

	case VAR_STRING: // ignore!
	    break;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_log
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

    bool have_p2 = n_param >= 2 && param[1].mode != VAR_UNSET;
    double base = have_p2 ? log(GetDoubleV(param+1)) : 0.0;

    switch(param->mode)
    {
	case VAR_UNSET:
	    res->mode = VAR_UNSET;
	    break;

	case VAR_INT:
	    res->d = log(param->i);
	    if (have_p2)
		res->d /= base;
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_DOUBLE:
	    res->d = log(param->d);
	    if (have_p2)
		res->d /= base;
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_VECTOR:
	    res->x = log(param->x);
	    res->y = log(param->y);
	    res->z = log(param->z);
	    if (have_p2)
	    {
		res->x /= base;
		res->y /= base;
		res->z /= base;
	    }
	    res->mode = VAR_VECTOR;
	    break;

	case VAR_STRING: // ignore!
	    break;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_exp
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

    switch(param->mode)
    {
	case VAR_UNSET:
	    res->mode = VAR_UNSET;
	    break;

	case VAR_INT:
	    res->d = exp(param->i);
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_DOUBLE:
	    res->d = exp(param->d);
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_VECTOR:
	    res->x = exp(param->x);
	    res->y = exp(param->y);
	    res->z = exp(param->z);
	    res->mode = VAR_VECTOR;
	    break;

	case VAR_STRING: // ignore!
	    break;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_pow
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
    DASSERT(n_param>=2);

    double d = GetDoubleV(param+1);
    switch(param->mode)
    {
	case VAR_UNSET:
	    res->mode = VAR_UNSET;
	    break;

	case VAR_INT:
	    res->d = pow(param->i,d);
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_DOUBLE:
	    res->d = pow(param->d,d);
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_VECTOR:
	    res->x = pow(param->x,d);
	    res->y = pow(param->y,d);
	    res->z = pow(param->z,d);
	    res->mode = VAR_VECTOR;
	    break;

	case VAR_STRING: // ignore!
	    break;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_random
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

    switch( n_param ? param->mode : VAR_UNSET )
    {
	case VAR_INT:
	    res->i = MyRandom(param->i);
	    res->mode = VAR_INT;
	    break;

	case VAR_DOUBLE:
	    res->d = MyRandom(0) * param->d / (double)RAND_MAX;
	    res->mode = VAR_DOUBLE;
	    break;

	case VAR_VECTOR:
	    res->x = MyRandom(0) * param->x / (double)RAND_MAX;
	    res->y = MyRandom(0) * param->y / (double)RAND_MAX;
	    res->z = MyRandom(0) * param->z / (double)RAND_MAX;
	    res->mode = VAR_VECTOR;
	    break;

	default:
	    res->d = MyRandom(0) / (double)RAND_MAX;
	    res->mode = VAR_DOUBLE;
	    break;

    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		parser functions: sin(), cos(), tan()	///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_sin
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

    if ( param->mode == VAR_VECTOR )
    {
	res->x = sin( param->x * (M_PI/180.0) );
	res->y = sin( param->y * (M_PI/180.0) );
	res->z = sin( param->z * (M_PI/180.0) );
	res->mode = VAR_VECTOR;
    }
    else
    {
	res->d = sin( GetDoubleV(param) * (M_PI/180.0) );
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_cos
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

    if ( param->mode == VAR_VECTOR )
    {
	res->x = cos( param->x * (M_PI/180.0) );
	res->y = cos( param->y * (M_PI/180.0) );
	res->z = cos( param->z * (M_PI/180.0) );
	res->mode = VAR_VECTOR;
    }
    else
    {
	res->d = cos( GetDoubleV(param) * (M_PI/180.0) );
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tan
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

    if ( param->mode == VAR_VECTOR )
    {
	res->x = tan( param->x * (M_PI/180.0) );
	res->y = tan( param->y * (M_PI/180.0) );
	res->z = tan( param->z * (M_PI/180.0) );
	res->mode = VAR_VECTOR;
    }
    else
    {
	res->d = tan( GetDoubleV(param) * (M_PI/180.0) );
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_asin
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

    if ( param->mode == VAR_VECTOR )
    {
	res->x = asin(param->x) * (180.0/M_PI);
	res->y = asin(param->y) * (180.0/M_PI);
	res->z = asin(param->z) * (180.0/M_PI);
	res->mode = VAR_VECTOR;
    }
    else
    {
	res->d = asin(GetDoubleV(param)) * (180.0/M_PI);
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_acos
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

    if ( param->mode == VAR_VECTOR )
    {
	res->x = acos(param->x) * (180.0/M_PI);
	res->y = acos(param->y) * (180.0/M_PI);
	res->z = acos(param->z) * (180.0/M_PI);
	res->mode = VAR_VECTOR;
    }
    else
    {
	res->d = acos(GetDoubleV(param)) * (180.0/M_PI);
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_atan
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

    if ( param->mode == VAR_VECTOR )
    {
	res->x = atan(param->x) * (180.0/M_PI);
	res->y = atan(param->y) * (180.0/M_PI);
	res->z = atan(param->z) * (180.0/M_PI);
	res->mode = VAR_VECTOR;
    }
    else
    {
	res->d = atan(GetDoubleV(param)) * (180.0/M_PI);
	res->mode = VAR_DOUBLE;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_atan2
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
    DASSERT(n_param>=2);

    res->d = atan2(GetXDoubleV(param),GetZDoubleV(param+1)) * (180.0/M_PI);
    res->mode = VAR_DOUBLE;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_if_engine
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

    uint mask = 0;

    uint i;
    for ( i = 0; i < n_param; i++ )
    {
	int val = GetIntV(param+i);
	if ( val >= 0 )
	    mask |= val;
	else
	    mask &= ~-val;
    }
    res->i = GOBJ_MIN_COND_ENGINE + ( mask & ENGM__ALL );
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_if_random
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

    uint mask = 0;

    uint i;
    for ( i = 0; i < n_param; i++ )
    {
	int val = GetIntV(param+i);
	if ( val == 0x7fffffff )
	    mask = GOBJ_MASK_COND_RND;
	else if ( val >= 1 && val <= GOBJ_MAX_RANDOM )
	    mask |= 1 << val-1;
	else if ( val >= -GOBJ_MAX_RANDOM && val <= -1 )
	    mask &= ~(1 << -1-val);
    }
    res->i = GOBJ_MIN_COND_RND + mask;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_if_test
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
    DASSERT(n_param==2);
    DASSERT(fpar);

    res->i = GOBJ_MIN_COND_TEST
	   | ( GetIntV(param+0) & 0x0f ) << 6
	   | ( GetIntV(param+1) & 0x1f ) << 1;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_chat_n_races
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
    DASSERT(n_param==1);
    DASSERT(fpar);

    const int val = GetIntV(param+0);
    res->i = val >= 1 && val <= CHATMD_MAX_RACE ? val + (CHATMD_N_RACE_1-1) : 0;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_chat_vehicles
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

    uint i, vehicles = 0;
    for ( i = 0; i < n_param; i++ )
    {
	int val = GetIntV(param+i);
	if ( val < 0 )
	{
	    val = -val;
	    if ( val & CHATVEH_ANY_SIZE )
		val ^= CHATVEH_ANY_SIZE;
	    if ( val & CHATVEH_ANY_TYPE )
		val ^= CHATVEH_ANY_TYPE;
	}

	if ( val <= CHATVEH_ANY || val >= CHATMD_VEHICLE_BEG && val <= CHATMD_VEHICLE_END )
	    vehicles |= val & CHATVEH_ANY;
    }

    res->i = vehicles + CHATMD_VEHICLE_BEG;
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			function table			///////////////
///////////////////////////////////////////////////////////////////////////////

static const struct FuncTable_t func_tab[] =
{
    //----- management

    { 0, MAX_FUNC_PARAM, "$", F_dollar, 0,
	"int", "$(...)",
	"This debug function returns the number of parameters." },

    { 1, MAX_FUNC_PARAM, "ECHO", F_echo, 0,
	"*", "echo(p1,...)",
	"This debug function prints all parameters, each in a separate line,"
	" and returns the first parameter." },

    { 0, 0, "LINE", F_line, 0,
	"int", "line()",
	"This debug function returns the current line number." },

    { 0, 0, "SOURCELEVEL", F_sourceLevel, 0,
	"int", "sourceLevel()",
	"This debug function returns the current source level"
	" (number of open files and macros)." },

    { 0, 0, "IFLEVEL", F_ifLevel, 0,
	"int", "ifLevel()",
	"This debug function returns the current IF..ENDIF level"
	" (number of active '@if's)." },

    { 0, 0, "LOOPLEVEL", F_loopLevel, 0,
	"int", "loopLevel()",
	"This debug function returns the current loop level"
	" (number of active loops)." },

    { 0, 1, "LOOPCOUNT", F_loopCount, 0,
	"int", "loopCount([level])",
	"This debug function returns the loop counter of the specified loop level."
	" If 'level' is omitted, the loop count of the current loop is returned." },

    { 0, 0, "STATUS", F_status, 0,
	"int", "status()",
	"Some functions support a return value and an additionally status."
	" This function will return the last set status." },

    //----- source identfication

    { 0, 0, "ISKCL", F_isKCL, 0,
	"int", "isKCL()",
	"Returns 2, if the source is a KCL or OBJ"
	" named 'course.kcl' or 'course.txt' or 'course.txt.kcl',"
	" 1 if the source is any other KCL or OBJ, and 0 else." },

    { 0, 0, "ISKMP", F_isKMP, 0,
	"int", "isKMP()",
	"Returns 2, if the source is a KMP (binary or text)"
	" named 'course.kmp' or 'course.txt' or 'course.txt.kmp',"
	" 1 if the source is any other KMP (binary or text), and 0 else." },

    { 0, 0, "ISLEX", F_isLEX, 0,
	"int", "isLEX()",
	"Returns 2, if the source is a LEX (binary or text)"
	" named 'course.lex' or 'course.txt' or 'course.txt.lex',"
	" 1 if the source is any other LEX (binary or text), and 0 else." },

    { 0, 0, "ISMDL", F_isMDL, 0,
	"int", "isMDL()",
	"Returns 1, if the source is a MDL (binary or text)."
	" Otherwise it returns 0." },


    //----- macro support

    { 0, 0, "RESULT", F_result, 0,
	"*", "result()",
	"Returns the last include, macro or function result."
	" The value is usually '$NONE', unless the command '@RETURN' is used." },

    { 1, 1, "PARAM", F_param, 0,
	"*", "param(index)",
	"Returns the value of the macro or function parameter"
	" with the entered 1-based index."
	" Therefor the private variable '$N' and one of '$1', '$2', ... are read." },

    { -1, 1, "ISMACRO", F_isMacro, 0,
	"int", "isMacro(name)",
	"Returns 2, if 'name' is defined as user function,"
	" or 1, if 'name' is defined as simple macro."
	" Otherwise it's not a macro or user function and 0 is returned." },

    { -1, 1, "ISFUNCTION", F_isFunction, 0,
	"int", "isFunction(name)",
	"Returns 1, if 'name' is defined as system function,"
	" or 2, if 'name' is defined as user function,"
	" or 3, if 'name' is defined as system and as user function."
	" Otherwise it's not a function and 0 is returned." },


    //----- variable status

    { -1, 1, "ISDEF", F_isDef, 0,
	"int", "isDef(name)",
	"Returns 2, if variable 'name' is defined,"
	" or 1, if a variable with the base name is defined as vector,"
	" or 0, if it is not defined." },

    { -1, 1, "ISINT", F_isInt, 0,
	"int", "isInt(name)",
	"Returns 2, if variable 'name' is defined as an integer with a value >0,"
	" or 1, if it is an integer with a value <=0."
	" Otherwise it returns 0." },

    { -1, 1, "ISFLOAT", F_isFloat, 0,
	"int", "isFloat(name)",
	"Returns 2, if variable 'name' is defined as float with a value >0,"
	" or 1, if it is a float with a value <=0."
	" Otherwise it returns 0." },

    { -1, 1, "ISVECTOR", F_isVector, 0,
	"int", "isVector(name)",
	"Returns 1, if variable 'name' is defined as vector."
	" Otherwise it returns 0." },

    { -1, 1, "ISSCALAR", F_isScalar, 0,
	"int", "isScalar(name)",
	"Returns 2, if variable 'name' is an integer or float with a value >0,"
	" or 1, if it is an integer or float with a value <=0."
	" Otherwise it returns 0." },

    { -1, 1, "ISNUMERIC", F_isNumeric, 0,
	"int", "isNumeric(name)",
	"Returns 1, if variable 'name' is an integer, float or vector."
	" Otherwise it returns 0." },

    { -1, 1, "ISSTR", F_isStr, 0,
	"int", "isStr(name)",
	"Returns 2, if variable 'name' is defined as string with 1 or more characters,"
	" or 1, if it is a string without characters."
	" Otherwise it returns 0." },

    { -1, 1, "TYPE", F_type, 0,
	"int", "type(name)",
	"Returns the type of the variable 'name'"
	" (TYPE$UNSET, TYPE$INT, TYPE$FLOAT, TYPE$VECTOR, TYPE$STR, TYPE$X, TYPE$Y, TYPE$Z)"
	" or value 'TYPE$UNDEF', if it is not defined."
	" It is guaranteed, that 'TYPE$UNDEF < TYPE$UNSET < all_others'." },

    { -2, 2, "VAR", F_var, 0,
	"*", "var(name,val)",
	"If variable 'name' is defined and valid, return its value."
	" Otherwise return 'val'." },


    //----- type conversions

    { 1, 1, "INT", F_int, 0,
	"int", "int(val)",
	"Converts 'val' to an integer value. Strings are scanned for an integer." },

    { 1, 1, "FLOAT", F_float, 0,
	"float", "float(val)",
	"Converts 'val' to a float value. Strings are scanned for a float." },

    { 1, 1, "SCALAR", F_scalar, 0,
	"scalar", "scalar(val)",
	"If 'val' is a string, it is scanned for a number and converted first. "
	"If 'val' is a float or a vector, a float value is returned."
	" Otherwise an integer value is returned." },

    { 1, 1, "STR", F_str, 0,
	"str", "str(val)",
	"Converts 'val' to a string value." },

    { 1, 1, "SCANVAL", F_scanval, 0,
	"*", "scanVal(val)",
	"If 'val' is a string, scan it for a value."
	" Otherwise return 'val'."
    },

    { 1, 1, "SCANEXPR", F_scanexpr, 0,
	"*", "scanExpr(val)",
	"If 'val' is a string, scan it for an expression."
	" Otherwise return 'val'."
    },


    //---- endian support

    { 2, 2, "BE", F_be, 0,
	"int", "be(val,n)",
	"Convert 'val' to integer, limit it to 'n' bytes (1-8)"
	" and mark it for big endian usage." },

    { 2, 2, "LE", F_le, 0,
	"int", "le(val,n)",
	"Convert 'val' to integer, limit it to 'n' bytes (1-8)"
	" and mark it for little endian usage." },


    //----- access functions

    { 2, MAX_FUNC_PARAM, "SELECT", F_select, 0,
	"*", "select(sel,p0,p1,...,pN)",
	"First convert 'sel' to an integer."
	" If 'sel' is less or equal 0, 'p0' is returned."
	" If 'sel' is greater or equal N, 'pN' is returned."
	" Otherwise 'sel' is between 0 and N and 'p(sel)' is returned." },


    //----- basic string functions

    { 2, 2, "LEFT", F_left, 0,
	"str", "left(str,len)",
	"Extract the left LEN characters of string STR."
	" If LEN<0, then extract all but not the last -LEN characters." },

    { 2, 2, "RIGHT", F_right, 0,
	"str", "right(str,len)",
	"Extract the right LEN characters of string STR."
	" If LEN<0, then extract all but not the first -LEN characters." },

    { 2, 3, "MID", F_mid, 0,
	"str", "mid(str,pos[,len])",
	"Extract LEN characters of string STR beginning at postition POS."
	" If POS<0, then a position relative to the end of STR is used."
	" If LEN<0, then extract all but not the last -LEN characters."
	" If LEN is not set, then extract all characters until end of string." },

    { 3, 3, "EXTRACT", F_extract, 0,
	"str", "extract(str,pos1,pos2)",
	"Extract all characters of string STR"
	" including position POS1 and excluding position POS2."
	" Positions <0 are relative to the end of STR and adjusted."
	" If POS1≥POS2, an empty string is returned." },

    { 3, 3, "REMOVE", F_remove, 0,
	"str", "remove(str,pos1,pos2)",
	"Create a copy of string STR and remove all characters"
	" including position POS1 and excluding position POS2."
	" Positions <0 are relative to the end of STR and adjusted."
	" If POS1≥POS2, nothing is removed and the complete STR is returned." },

    { 1, MAX_FUNC_PARAM, "PRINT", F_print, 0,
	"str", "print(format,...)",
	"Create a string like sprintf() of other programming languages."
	" See https://szs.wiimm.de/doc/print for details." },


    //----- timer

    { 0, 0, "SEC", F_sec, 0,
	"int", "sec()",
	"Returns the number of seconds since an unspecific timer start."
	" Use differences between 2 calls to get the elapsed time."
	" The 3 functions sec(), mSec() and uSec() use the same time base." },

    { 0, 0, "MSEC", F_mSec, 0,
	"int", "mSec()",
	"Returns the number of milliseconds since an unspecific timer start."
	" Use differences between 2 calls to get the elapsed time."
	" The 3 functions sec(), mSec() and uSec() use the same time base." },

    { 0, 0, "USEC", F_uSec, 0,
	"int", "uSec()",
	"Returns the number of microseconds since an unspecific timer start."
	" Use differences between 2 calls to get the elapsed time."
	" This timer has an overflow at about 35 minutes."
	" The 3 functions sec(), mSec() and uSec() use the same time base." },


    //----- basic vector functions

    { 0, 3, "V", F_v, 0,
	"vector", "v([val])",
	"Defines a vector."
	" If 'val' is a vector, it is copied."
	" Otherwise 'val' is used for the x coordinate and y and z are set to 0.0."
	" If 'val' is not set, 0.0 is used."
	"\n "
	" This function is used if an internal conversion to a vector is needed." },
    { 0, 0, 0, F_v, 0,
	"vector", "v(x[,y],z)",
	"Defines a vector by using the entered coordinates."
	" If 'y' is not set, use 0.0 instead."
	" If 'x', 'y' or 'z' is a vector itself, the corespondent coordinate is used." },

    { 1, 1, "V3", F_v3, 0,
	"vector", "v3(val)",
	"Create a vector with all 3 components equal to 'val'" },

    { 1, 2, "VX", F_vx, 0,
	"vector", "vx(x[,v])",
	"Read the vector 'v' and replace the x component by 'x'."
	" If 'x' is a vector, its x component is used."
	" If 'v' is not set, use v(0,0,0) instead." },

    { 1, 2, "VY", F_vy, 0,
	"vector", "vy(y[,v])",
	"Read the vector 'v' and replace the y component by 'y'."
	" If 'y' is a vector, its y component is used."
	" If 'v' is not set, use v(0,0,0) instead." },

    { 1, 2, "VZ", F_vz, 0,
	"vector", "vz(z[,v])",
	"Read the vector 'v' and replace the z component by 'z'."
	" If 'z' is a vector, its z component is used."
	" If 'v' is not set, use v(0,0,0) instead." },

    { 2, 3, "VD", F_vd, 0,
	"vector", "vd(len,deg[,y])",
	"Creates a vector of length 'len' and horizontal angle 'deg' (degree)."
	" Value 'y' (if not set: 0.0) is used for the height." },

    { 1, MAX_FUNC_PARAM, "X", F_x, 0,
	"float", "x(val_1,...,val_n)",
	"Scan all parameters (at least 1 must exist) and find the first vector."
	" If found, return its x component."
	" If no vector exists, return the last value 'val_n'."
	"\n "
	" Confirming the rules above, 'x(val)' means:"
	" If 'val' is a vector, return its x component."
	" Otherwise convert 'val' to a float and return it." },

    { 1, MAX_FUNC_PARAM, "Y", F_x, 1,
	"float", "y(val_1,...,val_n)",
	"Scan all parameters (at least 1 must exist) and find the first vector."
	" If found, return its y component."
	" If no vector exists, return the last value 'val_n'."
	"\n "
	" Confirming the rules above, 'y(val)' means:"
	" If 'val' is a vector, return its y component."
	" Otherwise convert 'val' to a float and return it." },

    { 1, MAX_FUNC_PARAM, "Z", F_x, 2,
	"float", "z(val_1,...,val_n)",
	"Scan all parameters (at least 1 must exist) and find the first vector."
	" If found, return its z component."
	" If no vector exists, return the last value 'val_n'."
	"\n "
	" Confirming the rules above, 'z(val)' means:"
	" If 'val' is a vector, return its z component."
	" Otherwise convert 'val' to a float and return it." },


    //----- 2D vector functions

    { 3, 3, "SIDEOFLINE", F_sideOfLine, 0,
	"int", "sideOfLine(a,b,pt)",
	"All parameters are converted into vectors,"
	" but only the x and z coordinates are used."
	" Vectors 'a' and 'b' define a line (from a to b)."
	" The function returns -1, if the point 'pt' is on the left side of the line,"
	" or +1, if the point is on the right side,"
	" or 0 if the point is on the line." },

    { 4, 101, "PTINCONVEXPOLYGON", F_ptInConvexPolygon, 0,
	"int", "ptInConvexPolygon(pt,p1,..,pN)",
	"All parameters are converted into vectors,"
	" but only the x and z coordinates are used."
	" 'p1..pN' are up to 100 vertices of a convex polygon."
	" The function returns 1, if the point 'pt' is inside"
	" the polygon (including the lines), and 0 otherwise."
	"\n "
	" Function status() will return the direction of the polygon:"
	" -1 for counterclockwise, +1 for clockwise and 0 for unknown." },

    { 4, 103, "PTSINCONVEXTRI", F_ptsInConvexTri, 0,
	"int", "ptsInConvexTri(t1,t2,t3,pt1,...,ptN)",
	"All parameters are converted into vectors,"
	" but only the x and z coordinates are used."
	" 't1..t3' define a convex triangle,"
	" and 'pt1..ptN' is a list of up to 100 points."
	" The functions returns the number of points, that are inside the triangle."
	" Points on the line are counted as inside too."
	"\n "
	" Function status() will return the direction of the triangle:"
	" -1 for counterclockwise, +1 for clockwise and 0 for unknown." },

    { 5, 104, "PTSINCONVEXQUAD", F_ptsInConvexQuad, 0,
	"int", "ptsInConvexQuad(q1,q2,q3,q4,pt1,...,ptN)",
	"All parameters are converted into vectors,"
	" but only the x and z coordinates are used."
	" 'q1..q4' define a convex quadrilateral,"
	" and 'pt1..ptN' is a list of up to 100 points."
	" The functions returns the number of points, that are inside the quadrilateral."
	" Points on the line are counted as inside too."
	"\n "
	" Function status() will return the direction of the quadrilateral:"
	" -1 for counterclockwise, +1 for clockwise and 0 for unknown." },


    //----- 3D vector functions

    { 2, 2, "DOT", F_dot, 0,
	"float", "dot(v1,v2)",
	"Returns the dot product of the 2 vectors 'v1' and 'v2'." },

    { 2, 2, "CROSS", F_cross, 0,
	"vector", "cross(v1,v2)",
	"Returns the cross product of the 2 vectors 'v1' and 'v2'." },

    { 1, 2, "UNIT", F_unit, 0,
	"vector", "unit(v1[,v2])",
	"Returns the unit vector of the vector 'v1'."
	" If 'v2' is set, then the unit vector for the difference 'v2-v1' is returned." },

    { 2, 4, "CALCNORMALS", F_calcNormals, 0,
	"float", "calcNormals(p1,p2[,helper][,r]])",
	"This function calculates 3 right-angled normals (index 0..2) "
	" for the vector 'p1..p2', 'normal[0]' is the direction of this vector."
	" Is 'helper' is set, it is used to determine the direction of normal[1]."
	" All 3 normals are multiplied by the factor 'r'."
	" The function returns the distance between 'p1' and 'p2'."
	" To get the normals, use function getNormal()." },

    { 1, 1, "GETNORMAL", F_getNormal, 0,
	"vector", "getNormal(index)",
	"This function returns the normal 'index' (0..2) of the last call"
	" of function calcNormals()."
	" If 'index' is invalid, $NONE is returned." },

    { 1, 2, "LEN", F_len, 0,
	"int/flt", "len(v1[,v2])",
	"If 'v1' is a string, then 'v2' is ignored and the length of the string"
	" is returned as integer."
	" Otherwise the length of the vector 'v1' is returned as float."
	" If 'v2' is set, the distance of both points is returned." },

    { 1, 2, "LEN2", F_len, 2,
	"float", "len2(v1[,v2])",
	"Returns the square of the length of the vector 'v1'."
	" If 'v2' is set, the square distance of both points is returned." },

    { 1, 2, "HLEN", F_hLen, 0,
	"float", "hLen(v1[,v2])",
	"Returns the horizontal length of the vector 'v1' (ignoring the y component)."
	" If 'v2' is set, the horizontal distance of both points is returned." },

    { 1, 2, "HLEN2", F_hLen, 2,
	"float", "hLen2(v1[,v2])",
	"Returns the square of the horizontal length of the vector 'v1'"
	" (ignoring the y component)."
	" If 'v2' is set, the horizontal square distance of both points is returned." },

    { 3, 3, "POS", F_pos, 0,
	"flt|vec", "pos(pos,p1,p2)",
	"This function returns the relative position 'pos' on the axis 'p1' to 'p2'"
	" by calculating 'p1+pos*(p2-p1)'."
	" If one of the 3 arguments is a vector, the result is also a vector." },

    { 4, 5, "BEZIER", F_bezier, 0,
	"flt|vec", "bezier(pos,v1,va[,vb],v2)",
	"Calculate the position 'pos' on a bezier curve going from 'v1' to 'v2'."
	" 'va' and 'vb' are helper points."
	" If 'vb' is not set, a quadratic bezier curve is calculated."
	" Otherwise 'va' and 'vb' are set and a cubic bezier curve is calculated."
	" Position 0.0 returns 'v1' and position 1.0 'v2'."
	" All position values between 0.0 and 1.0 return a point of the calculated"
	" bezier curve between points 'v1' and 'v2'."
	" If 'pos' is a vector, different positions for each coordinate are used."
	"\n "
	" If any parameter is a vector, all parameters are converted to vectors and"
	" the result is a vector. Otherwise all parameters are converted to"
	" floats and the result is a float." },

    { 2, 3, "XROT", F_xRot, 0,
	"vector", "xRot(pt,deg[,origin])",
	"The point 'pt' is rotated around the x-axis of 'origin' by 'deg' degree."
	" If 'pt' is a scalar, 'vy(pt)' is used."
	" If 'origin' is not set, v(0,0,0) is used." },

    { 2, 3, "YROT", F_xRot, 1,
	"vector", "yRot(pt,deg[,origin])",
	"The point 'pt' is rotated around the y-axis of 'origin' by 'deg' degree."
	" If 'pt' is a scalar, 'vz(pt)' is used."
	" If 'origin' is not set, v(0,0,0) is used."
	" This is the classical horizontal rotation." },

    { 2, 3, "ZROT", F_xRot, 2,
	"vector", "zRot(pt,deg[,origin])",
	"The point 'pt' is rotated around the z-axis of 'origin' by 'deg' degree."
	" If 'pt' is a scalar, 'vx(pt)' is used."
	" If 'origin' is not set, v(0,0,0) is used." },

    { 2, 3, "HROT", F_xRot, 1,
	"vector", "hRot(pt,deg[,origin])",
	"'hRot()' is the old name for 'yRot()'."
	" The point 'pt' is horizontal rotated around 'origin' by 'deg' degree."
	" If 'pt' is a scalar, 'vz(pt)' is used."
	" If 'origin' is not set, v(0,0,0) is used." },

    { 2, 3, "ROT", F_rot, 0,
	"vector", "rot(pt,deg_vector[,origin])",
	"The point 'pt' is rotated around 'origin'."
	" The rotation is done for the x-, y- and z-axis in this order."
	" All parameters are converted to vectors before the operation."
	" If 'origin' is not set, v(0,0,0) is used." },

    { 4, 4, "AXISROT", F_axisRot, 0,
	"vector", "axisRot(pt,deg,axis1,axis2)",
	"The point 'pt' is rotated around the axis 'axis1->axis2'"
	" by 'deg' degree, which is a scalar."
	" The other 3 parameters are converted to vectors before operation." },

    { 1, 2, "XDIR", F_aDir, 0,
	"float", "xDir([v1,]v2)",
	"The functions returns the direction in degree of point 'v2'"
	" relative to the x-axis of point 'v1'."
	" If 'v1' is not set, v(0,0,0) is used." },

    { 1, 2, "YDIR", F_aDir, 1,
	"float", "yDir([v1,]v2)",
	"The functions returns the direction in degree of point 'v2'"
	" relative to the y-axis of point 'v1'."
	" If 'v1' is not set, v(0,0,0) is used." },

    { 1, 2, "ZDIR", F_aDir, 2,
	"float", "zDir([v1,]v2)",
	"The functions returns the direction in degree of point 'v2'"
	" relative to the z-axis of point 'v1'."
	" If 'v1' is not set, v(0,0,0) is used." },

    { 1, 2, "HDIR", F_aDir, 1,
	"float", "hDir([v1,]v2)",
	"'hDir()' is the old name for 'yDir()'."
	" The horizontal direction in degree from position 'v1' to 'v2' is calculated."
	" If 'v1' is not set, v(0,0,0) is used." },

    { 1, 2, "DIR", F_dir, 0,
	"float", "dir([v1,]v2)",
	"The 3D direction in degree from position 'v1' to 'v2' is calculated."
	" If 'v1' is not set, v(0,0,0) is used."
	" The Z coordiante of the result is always 0.0." },

    { 3, MAX_FUNC_PARAM, "PTSINCUBOID", F_ptsInCuboid, 0,
	"int", "ptsInCuboid(cube_min,cube_max,pt1,...)",
	"All parameters are converted to vectors."
	" 'cube_*' describe 2 diagonal corners of a rectangular cuboid,"
	" assuming that 'cube_min<=cube_max' is true for each coordinate."
	" The functions returns the number of points ('pt1', 'pt2', ...)"
	" that are inside of the cube including the border." },

    { 3, MAX_FUNC_PARAM, "PTINCUBOID", F_ptsInCuboid, 0,	// altenative name
	0,0,0 },

    { 3, MAX_FUNC_PARAM, "PTSINCUBOIDS", F_ptsInCuboid, 1,
	"int", "ptsInCuboidS(cube1,cube2,pt1,...)",
	"This is the sloppy version of ptInCuboid():"
	" All parameters are converted to vectors."
	" 'cube*' describe any 2 diagonal corners of a rectangular cuboid."
	" The functions returns the number of points ('pt1', 'pt2', ...)"
	" that are inside of the cube including the border." },

    { 3, MAX_FUNC_PARAM, "PTINCUBOIDS", F_ptsInCuboid, 1,	// altenative name
	0,0,0 },

 #ifdef TEST // [[2do]]
    { 5, 5, "OVERLAPCUBETRI", F_OverlapCubeTri, 1,
	"int", "OverlapCubeTri(cube_mid,cube_width,pt1,pt2,pt3)",
	"'cube_mid' is the middle point of a cube"
	" and 'cube_width' the length of its edges."
	" 'pt1'..'pt3' are the 3 points of the triangle."
	" The functions returns 1, if the cube and the triangle"
	" are overlapped. Otherwise 0 is returned." },
 #endif


    //----- simple mathematical functions

    { 1, 1, "ABS", F_abs, 0,
	"*", "abs(val)",
	"Returns the absolute value of 'val'."
	" If 'val' is a vector, the absolute value of each component is calculated."
	" The return type is identical to the type of 'val'." },

    { 1, 1, "SIGN", F_sign, 0,
	"*", "sign(val)",
	"Returns the sign of 'val': -1 if 'val<0'; 0, if 'val==0'; +1 if 'val>0'."
	" If 'val' is a vector, the sign of each component is calculated."
	" The return type is identical to the type of 'val'." },

    { 1, MAX_FUNC_PARAM, "MIN", F_min, 0,
	"*", "min(p1,...)",
	"Returns the minimum value of all parameters."
	" If at least one parameter is a vector, the result is a vector too"
	" and the minimum value of each component is calculated." },

    { 1, MAX_FUNC_PARAM, "MAX", F_max, 0,
	"*", "max(p1,...)",
	"Returns the maximum value of all parameters."
	" If at least one parameter is a vector, the result is a vector too"
	" and the maximum value of each component is calculated." },

    { 3, 3, "MINMAX", F_minMax, 0,
	"*", "minMax(val,minval,maxval)",
	"Returns 'max(min(val,minval),maxval)':"
	" Limit the value 'val' by 'minval' and 'maxval'."
	" The return type is identical to the type of 'val'." },

    { 1, MAX_FUNC_PARAM, "MEAN", F_mean, 0,
	"*", "mean(p1,...)",
	"Returns the arithmetic mean of all parameters."
	" If at least one parameter is a vector, the result is a vector too"
	" and the maximum value of each component is calculated."
	" The result type is UNSET, FLOAT or VECTOR, but never INT." },

    { 1, 1, "TRUNC", F_trunc, 0,
	"*", "trunc(val)",
	"Returns the  nearest integer not larger in absolute value than 'val'"
	" (rounding towards zero)."
	" If 'val' is a vector, the calculation is done for each component."
	" The return type is identical to the type of 'val'." },

    { 1, 1, "FLOOR", F_floor, 0,
	"*", "floor(val)",
	"Returns the largest integral value that is not greater than 'val'."
	" If 'val' is a vector, the calculation is done for each component."
	" The return type is identical to the type of 'val'." },

    { 1, 1, "CEIL", F_ceil, 0,
	"*", "ceil(val)",
	"Returns the smallest integral value that is not less than 'val'."
	" If 'val' is a vector, the calculation is done for each component."
	" The return type is identical to the type of 'val'." },

    { 1, 1, "ROUND", F_round, 0,
	"*", "round(val)",
	"Returns the rounded integer value of 'val'."
	" If 'val' is a vector, the calculation is done for each component."
	" The return type is identical to the type of 'val'." },

    { 1, 1, "SQRT", F_sqrt, 0,
	"flt|vec", "sqrt(val)",
	"Returns the nonnegative square root of 'abs(val)'."
	" If 'val' is a vector, the square root of each component is calculated." },

    { 1, 2, "LOG", F_log, 0,
	"flt|vec", "log(val[,base])",
	"If base is not set or invalid, the function returns the natural logarithm of 'val'."
	" Otherwise base is converted to a float"
	" and logarithm with the entered base is returned (=log(val)/log(base))."
	" If 'val' is a vector, the calculation is done for each component." },

    { 1, 1, "EXP", F_exp, 0,
	"flt|vec", "exp(val)",
	"Returns the value of e (the base of natural logarithms) raised to the power of 'val'."
	" If 'val' is a vector, the calculation is done for each component." },

    { 2, 2, "POW", F_pow, 0,
	"flt|vec", "pow(a,b)",
	"Returns the value of 'a' raised to the power of 'b'."
	" If 'a' is a vector, each component is raised by 'b'."
	" The operator ** does the same, but have integer support." },

    { 0, 1, "RANDOM", F_random, 0,
	"*", "random([max])",
	"Returns a random number between 0 and 'max', but never equal 'max'."
	" The return type is identical to the type of 'max'."
	" If 'max' is not set, use float 1.0 instead."
	" If 'max' is a vector, a vector with three random floats will be returned." },


    //----- sin(), cos(), tan(), ...

    { 1, 1, "SIN", F_sin, 0,
	"flt|vec", "sin(val)",
	"Returns the sine of 'val', where 'val' is given in degree."
	" If 'val' is a vector, the sine of each component is calculated." } ,

    { 1, 1, "COS", F_cos, 0,
	"flt|vec", "cos(val)",
	"Returns the cosine of 'val', where 'val' is given in degree."
	" If 'val' is a vector, the cosine of each component is calculated." } ,

    { 1, 1, "TAN", F_tan, 0,
	"flt|vec", "tan(val)",
	"Returns the tangent of 'val', where 'val' is given in degree."
	" If 'val' is a vector, the tangent of each component is calculated." } ,

    { 1, 1, "ASIN", F_asin, 0,
	"flt|vec", "asin(val)",
	"Returns the arc sine of 'val' in degree."
	" If 'val' is a vector, the arc sine of each component is calculated." } ,

    { 1, 1, "ACOS", F_acos, 0,
	"flt|vec", "acos(val)",
	"Returns the arc cosine of 'val' in degree."
	" If 'val' is a vector, the arc cosine of each component is calculated." } ,

    { 1, 1, "ATAN", F_atan, 0,
	"flt|vec", "atan(val)",
	"Returns the arc tangent of 'val' in degree."
	" If 'val' is a vector, the arc tangent of each component is calculated." } ,

    { 2, 2, "ATAN2", F_atan2, 0,
	"float", "atan2(x,z)",
	"Returns the arc tangent of 'z/x' in degree, using the signs of the two"
	" arguments to determine the quadrant of the result."
	" Both arguments are converted to a float before operation."
	" If 'x' is a vector, its x component is used."
	" If 'z' is a vector, its z component is used." },


    //----- LE-CODE/LEX support functions

    { 0, MAX_FUNC_PARAM, "IF$ENGINE", F_if_engine, 0,
	"int", "if$engine(...)",
	"Create a condition for KMP/GOBJ references."
	" The parameters are used to select engine types."
	" Available values: EN$BATTLE, EN$50, EN$100, EN$150 and EN$200."
	" Use  EN$150M, EN$200M for mirror modes."
	" Additional short cuts: EN$150X for EN$150,EN$150M,"
	" EN$200X for EN$200,EN$200M, and EN$MIRROR for EN$150M,EN$200M."
	" A negative index deselects an engine mode."
	" '*' is a short cut for all modes." },

    { 0, MAX_FUNC_PARAM, "IF$RANDOM", F_if_random, 0,
	"int", "if$random(...)",
	"Create a condition for KMP/GOBJ references."
	" The parameters are used to select random scenarios between 1 and 8."
	" A negative value deselects a scenario."
	" '*' is a short cut for the parameter list '1,2,3,4,5,6,7,8'." },

 #if HAVE_WIIMM_EXT
    { 2, 2, "IF$TEST", F_if_test, 0,
	"int", "if$test(index,bitnum)",
	"[Hidden for the public]"
	" Create a test condition for KMP/GOBJ references."
	" Index is between 0 and 15 and bitnum between 0 and 31." },
 #else
    { 2, 2, "IF$TEST", F_if_test, 0, 0,0,0 },
 #endif

    { 1, 1, "CHAT$N_RACES", F_chat_n_races, 0,
	"int", "chat$n_races(n)",
	"Create a chat modus for LE-CODE setup."
	" N is a values between 1 and 512 and defines the number of races."
	" The parameters are used to select random scenarios between 1 and 8." },

    { 0, MAX_FUNC_PARAM, "CHAT$VEHICLES", F_chat_vehicles, 0,
	"int", "chat$vehicles(...)",
	"Create a chat modus for a vehicle group."
	" 0 to N values are expected as function parameters."
	" Each parameter is either from the"
	" group VEH$SMALL, VEH$MEDIUM, VEH$LARGE and VEH$ANY_SIZE"
	" for size selections and/or from the group"
	" VEH$KART, VEH$OUT_BIKE, VEH$IN_BIKE, VEH$BIKE, VEH$ANY_TYPE"
	" for type selections."
	" Negative values are an alias for 'all of group except'."
	" VEH$ANY is a short cut for VEH$ANY_SIZE,VEH$ANY_TYPE."  },

    //----- list terminator

    {0,0,0,0,0,0,0,0}
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			function access			///////////////
///////////////////////////////////////////////////////////////////////////////

FuncParam_t * DefineParserFunc
(
    ccp			name,		// name of function
    s16			min_param,	// minimum parameter
    s16			max_param,	// maximum parameter
    u16			fform_id,	// function class, based on file_format_t
    call_func_t		func,		// function to call
    int			user_id		// user defined id
)
{
    DASSERT(name);
    DASSERT( min_param <= max_param );
    DASSERT(func);

    if (!parser_func.used)
	DefineDefaultParserFunc();

    Var_t *vp = InsertVarMap(&parser_func,name,false,0,0);
    DASSERT(vp);
    vp->func_param.need_name = min_param < 0;
    if (vp->func_param.need_name)
	min_param = -min_param;
    vp->func_param.min_param = min_param;
    vp->func_param.max_param = max_param;
    vp->func_param.func      = func;
    vp->func_param.fform_id  = fform_id;
    vp->func_param.user_id   = user_id;
    return &vp->func_param;
}

///////////////////////////////////////////////////////////////////////////////

void DefineParserFuncTab
(
    const FuncTable_t	*ftab,		// pointer to a definittion table
					// terminated with member 'func == 0'
    u16			fform_id	// function class, based on file_format_t
)
{
    DASSERT(ftab);
    for ( ; ftab->func; ftab++ )
    {
	if (ftab->name)
	    DefineParserFunc( ftab->name, ftab->min_param, ftab->max_param,
				fform_id, ftab->func, ftab->user_id );

	if (ftab->syntax)
	    DefineParserFuncInfo( ftab->result, ftab->syntax, ftab->info );
    }
}

///////////////////////////////////////////////////////////////////////////////

void DefineDefaultParserFunc()
{
    static bool done = 0;
    if (!done)
    {
	done = true;

	NullVector.name = 0;
	NullVector.mode = VAR_VECTOR;
	NullVector.x    = 0.0;
	NullVector.y    = 0.0;
	NullVector.z    = 0.0;

	DefineParserFuncTab(func_tab,0);

	memset(&FuncDollar,0,sizeof(FuncDollar));
	FuncDollar.max_param = MAX_FUNC_PARAM;
	FuncDollar.func = F_dollar;
    }
}

///////////////////////////////////////////////////////////////////////////////

const FuncParam_t * GetParserFunc
(
    ccp			name,		// name to search
    struct ScanInfo_t	*si,		// not NULL: print warning if not found
    FuncParam_t		*temp_par	// not NULL: search user defined functions,
					// and use this to create the function result
)
{
    DASSERT(name);

    if (!parser_func.used)
	DefineDefaultParserFunc();

    if ( si && temp_par )
    {
	const Var_t *vp = FindVarMap(&si->macro,name,0);
	if (vp)
	{
	    ScanMacro_t *macro = vp->macro;
	    DASSERT(macro);
	    if (macro->is_function)
	    {
		memset(temp_par,0,sizeof(*temp_par));
		temp_par->max_param	= MAX_FUNC_PARAM;
		temp_par->macro		= macro;
		temp_par->func		= F_call_macro;
		return temp_par;
	    }
	}
    }

    const Var_t *vp = FindVarMap(&parser_func,name,0);
    if (vp)
	return &vp->func_param;

    if ( si && si->no_warn <= 0 )
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	ccp eol = FindNextLineFeedSI(si,true);
	ERROR0(ERR_WARNING,
		"Function '%s' not defined [%s @%u]: %.*s\n",
		name, sf->name, sf->line,
		(int)(eol - sf->prev_ptr), sf->prev_ptr );
    }

    return &FuncDollar;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			function info			///////////////
///////////////////////////////////////////////////////////////////////////////

FuncInfo_t * DefineParserFuncInfo
(
    ccp			result,		// function result info
    ccp			syntax,		// syntax of function
    ccp			info		// description
)
{
    DASSERT(syntax);

    Var_t *vp = InsertVarMap(&parser_func_info,syntax,false,0,0);
    DASSERT(vp);
    if (result)
	vp->func_info.result = result;
    if (info)
	vp->func_info.info = info;
    return &vp->func_info;
}

///////////////////////////////////////////////////////////////////////////////

void DumpParserFuncInfo
(
    FILE		* f,		// destination file
    uint		indent,		// indention
    bool		print_header,	// true: print table header
    int			be_verbose,	// >0: print description too
    struct search_filter_t * filter	// NULL or filter data
)
{
    DASSERT(f);
    DefineDefaultParserFunc();

    if ( indent > 40 )
	 indent = 40;
    const uint twidth = GetTermWidth(80,indent+40)-1;

    uint fw_result = 4;
    uint fw_syntax = 6;

    uint i;
    for ( i = 0; i < parser_func_info.used; i++ )
	if ( !filter || filter[i].mode )
	{
	    const Var_t *vp = parser_func_info.list + i;

	    uint len = strlen(vp->name);
	    if ( fw_syntax < len )
		 fw_syntax = len;

	    len = strlen(vp->func_info.result);
	    if ( fw_result < len )
		 fw_result = len;
	}

    if ( be_verbose > 0 )
    {
	if ( fw_syntax > 15 )
	     fw_syntax = 15;

	if (print_header)
	    fprintf(f,"%*s%-*s %-*s Description\n%*s%.*s\n",
			indent, "",
			fw_result, "Type",
			fw_syntax, "Syntax",
			indent, "",
			twidth, Minus300 );

	const uint fw_head = indent + fw_result + fw_syntax + 2;

	for ( i = 0; i < parser_func_info.used; i++ )
	    if ( !filter || filter[i].mode )
	    {
		const Var_t *vp = parser_func_info.list + i;

		uint len = fprintf(f,"%*s%-*s %s",
				    indent, "",
				    fw_result, vp->func_info.result,
				    vp->name );
		if ( len >= fw_head )
		{
		    fputc('\n',f);
		    len = 0;
		}
		PutLines(stdout,fw_head,twidth,len,0,vp->func_info.info,0);
	    }
    }
    else
    {
	const uint fw_head = indent + fw_result + fw_syntax + 4;
	if (print_header)
	    fprintf(f,"%*s %-*s  Syntax\n%*s%.*s\n",
			indent, "",
			fw_result, "Type",
			indent, "",
			fw_head, Minus300 );

	for ( i = 0; i < parser_func_info.used; i++ )
	    if ( !filter || filter[i].mode )
	    {
		const Var_t *vp = parser_func_info.list + i;
		fprintf(f,"%*s %-*s  %s\n",
			indent, "",
			fw_result, vp->func_info.result,
			vp->name );
	    }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

search_filter_t * SetupParserFuncFilter()
{
    return AllocSearchFilter(parser_func_info.used);
}

///////////////////////////////////////////////////////////////////////////////

void ParserFuncFilter ( search_filter_t *filter, ccp arg, int mode )
{
    DASSERT(filter);

    if ( !arg || !*arg )
	return;

    char needle[200];
    CreateNeedleWCMP(needle,sizeof(needle),arg);

    uint i;
    for ( i = 0; i < parser_func_info.used; i++ )
    {
	if (!filter[i].mode)
	{
	    const Var_t *vp = parser_func_info.list + i;
	    filter[i].mode = strcasestr(vp->name,arg) != 0;
	    if (!filter[i].mode)
	    {
		if (!filter[i].string)
		{
		    ccp list[] = { vp->name, mode ? vp->func_info.info : 0, 0 };
		    char haystack[5000];
		    uint len = CreateHaystackListWCMP(haystack,sizeof(haystack),list);
		    filter[i].string = MEMDUP(haystack,len+1);
		}
		filter[i].mode = FindWCMP(filter[i].string,needle,3,0,0);
	    }
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DumpSymbols ( const VarMap_t * vm )
{
    if (print_header)
	putchar('\n');
    DumpVarMap(stdout,0,vm,print_header);
    if (print_header)
	putchar('\n');
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ListParserFunctions()
{
    DefineDefaultParserFunc();

    search_filter_t *filter = 0;
    if (first_param)
    {
	filter = SetupParserFuncFilter();
	ParamList_t *param;
	for ( param = first_param; param; param = param->next )
	    ParserFuncFilter(filter,param->arg,long_count);
    }

    if (print_header)
	putchar('\n');
    DumpParserFuncInfo(stdout,0,print_header,!brief_count,filter);
    if (print_header)
	putchar('\n');

    FreeSearchFilter(filter);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ParserCalc ( const VarMap_t * predef )
{
    int np = 1;
    ParamList_t *param;

    for ( param = first_param; param; param = param->next, np++ )
    {
	char name[20];
	snprintf(name,sizeof(name),"Param #%u",np);

	ScanInfo_t si;
	InitializeSI(&si,param->arg,strlen(param->arg),name,0);
	si.predef = predef;
	//si.point_is_null++;

	DEFINE_VAR(val);
	ScanExprSI(&si,&val);
	PrintV(stdout,&val,1);
	putchar('\n');
	CheckEolSI(&si);
	ResetSI(&si);
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  export			///////////////
///////////////////////////////////////////////////////////////////////////////

void ExportParserFuncMakedoc ( FILE * f, ccp index )
{
    DASSERT(f);
    DASSERT(index);

    fprintf(f,"\n#\f\n%.79s\n%.15s%20s%-29s%.15s\n%.79s\n\n",
	Hash200, Hash200, "", "Functions", Hash200, Hash200 );

    fprintf(f,"#gdef $func['%s'] = @map\n\n",index);

    const Var_t * func_end = parser_func.list + parser_func.used;
    const Var_t * info_end = parser_func_info.list + parser_func_info.used;
    const Var_t * func;

    for ( func = parser_func.list; func < func_end; func++ )
    {
	fprintf(f,
		"#pdef d = @map\n"
		"#pdef d['name'] = '%s'\n"
		"#pdef d['class'] = '%s'\n"
		"#pdef d['p-name'] = %d\n"
		"#pdef d['p-min'] = %d\n"
		"#pdef d['p-max'] = %d\n"
		"#pdef d['info'] = @map\n"
		,func->name
		,func->func_param.fform_id ? GetNameFF(0,func->func_param.fform_id) : ""
		,func->func_param.need_name
		,func->func_param.min_param
		,func->func_param.max_param
		);

	const Var_t * info;
	for ( info = parser_func_info.list; info < info_end; info++ )
	{
	    char name[100];
	    StringCopyS(name,sizeof(name),info->name);
	    char *end = strchr(name,'(');
	    if (end)
		*end = 0;
	    end = strchr(info->name,' ');
	    if (end)
		*end = 0;
	    if (!strcasecmp(func->name,name))
	    {
		fprintf(f,
			"#pdef d['info']['%s']['result'] = '%s'\n"
			"#pdef d['info']['%s']['info'] = \\\n"
			,info->name ,info->func_info.result
			,info->name
			);
		DumpText(f,0,0,info->func_info.info,-1,true,"\n");
	    }
	}

	fprintf(f,"#gdef $func['%s']['%s'] = move(d)\n\n",index,func->name);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ExportTrackInfo ( FILE * f )
{
    fputs("$MKW_TRACK_INFO = array\n(\n",f);

    uint i;
    for ( i = 0; i < MKW_N_TRACKS + MKW_N_ARENAS; i++ )
    {
	const bool is_arena = i >= MKW_N_TRACKS;
	const TrackInfo_t *ti	= is_arena
				? arena_info + i - MKW_N_TRACKS
				: track_info + i;

	fprintf(f,"    %u => (object)array(\n"
		"	'slot'	=> %3u,\n"
		"	'index'	=> %3u,\n"
		"	'music'	=> %3u,\n"
		"	'track'	=> '%c%2u',\n"
		"	'abbrev'=> '%s',\n"
		"	'de'	=> \"%s\",\n"
		"	'en'	=> \"%s\",\n"
		"	'fname'	=> \"%s\",\n"
		"	),\n\n"
		,i
		,i
		,ti->def_index
		,ti->music_id
		,is_arena ? 'A' : 'T', ti->def_slot
		,ti->abbrev
		,ti->name_de
		,ti->name_en
		,ti->track_fname
		);
    }

    fputs(");\n\n",f);
}

///////////////////////////////////////////////////////////////////////////////

enumError ExportHelper ( ccp func_mode )
{
    SetPatchFileModeReadonly();

    stdlog = stderr;
    export_mode_t exmode = ScanExportMode();
    if (!exmode)
	return ERR_SYNTAX;

    switch( exmode & EXPORT_CMD_MASK )
    {
	case EXPORT_PHP:
	    if ( exmode & EXPORT_F_TRACKS )
		ExportTrackInfo(stdout);
	    break;

	case EXPORT_MAKEDOC:
	    if ( exmode & EXPORT_F_FUNCTIONS )
		ExportParserFuncMakedoc(stdout,func_mode);
	    if ( exmode & EXPORT_F_FILEATTRIB )
		ExportFileAttribMakedoc(stdout);
	    break;
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


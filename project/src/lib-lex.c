
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

#include "dclib-xdump.h"
#include "lib-lex.h"
#include "lib-std.h"
#include "lib-mkw.h"
#include "lib-szs.h"
#include "lex.inc"

#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			DEBUG settings			///////////////
///////////////////////////////////////////////////////////////////////////////

#undef CHECK_FEATURES

#if defined(TEST) || defined(DEBUG) || defined(HAVE_WIIMM_EXT)
  #define CHECK_FEATURES 1
#else
  #define CHECK_FEATURES 0
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    LEX action log		///////////////
///////////////////////////////////////////////////////////////////////////////

bool LEX_ACTION_LOG ( bool is_patch, const char * format, ... )
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[LEX]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

//    if ( verbose > 2 || KMP_MODE & KMPMD_LOG || is_patch && ~KMP_MODE & KMPMD_SILENT )
    if ( verbose > 2 || is_patch )
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[LEX]%s ",
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
///////////////			features_szs_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[feature_var_t]]

typedef struct feature_var_t
{
    u16 offset;		// offset for features_szs_t
    u8  index;		// index or bit number of related analyze_szs_t
    u8  no_lex;		// suppress output for lex files
    ccp name;		// name of variable
    ccp comment;	// comment for variable
}
feature_var_t;

#undef DEF_VAR
#define DEF_VAR(l,i,v,c) { offsetof(features_szs_t,v),i,l,#v,c }

//-----------------------------------------------------------------------------

static const feature_var_t szs_feature_list[] =
{
    DEF_VAR( 1,HAVESZS_COURSE_LEX,	f_lex,			"File course.lex" ),
    DEF_VAR( 0,HAVESZS_ITEM_SLOT_TABLE,	f_item_slot_table,	"File ItemSlotTable/ItemSlotTable.slt" ),
    DEF_VAR( 0,HAVESZS_OBJFLOW,		f_objflow,		"File common/ObjFlow.bin" ),
    DEF_VAR( 0,HAVESZS_GHT_ITEM,	f_ght_item,		"File common/GeoHitTableItem.bin" ),
    DEF_VAR( 0,HAVESZS_GHT_ITEM_OBJ,	f_ght_item_obj,		"File common/GeoHitTableItemObj.bin" ),
    DEF_VAR( 0,HAVESZS_GHT_KART,	f_ght_kart,		"File common/GeoHitTableKart.bin" ),
    DEF_VAR( 0,HAVESZS_GHT_KART_OBJ,	f_ght_kart_obj,		"File common/GeoHitTableKartObj.bin" ),
    DEF_VAR( 0,HAVESZS_MINIGAME,	f_minigame,		"File common/minigame.kmg" ),
    DEF_VAR( 0,HAVESZS_AIPARAM_BAA,	f_aiparam_baa,		"File AIParam/AIParam.baa" ),
    DEF_VAR( 0,HAVESZS_AIPARAM_BAS,	f_aiparam_bas,		"File AIParam/AIParam.bas" ),

    {0,0,0,0,0}
};

//-----------------------------------------------------------------------------

static const feature_var_t kmp_feature_list[] =
{
    DEF_VAR( 0,HAVEKMP_GOOMBA_SIZE,	kmp_goomba_size,	"KMP: Goomba with scale != 1.00" ),
    DEF_VAR( 0,HAVEKMP_WOODBOX_HT,	kmp_woodbox_ht,		"KMP: Woodbox with fall height" ),
    DEF_VAR( 0,HAVEKMP_MUSHROOM_CAR,	kmp_mushroom_car,	"KMP: penguin_m as mushroom car" ),
    DEF_VAR( 0,HAVEKMP_PENGUIN_POS,	kmp_penguin_pos,	"KMP: Alternative start of penguin_m" ),
    DEF_VAR( 0,HAVEKMP_SECOND_KTPT,	kmp_second_ktpt,	"KMP: Second KTPT with index -1" ),
    DEF_VAR( 0,HAVEKMP_X_PFLAGS,	kmp_extended_pflags,	"KMP: Extended presence flags (XPF) used" ),
    DEF_VAR( 0,HAVEKMP_X_COND,		kmp_xpf_cond_obj,	"KMP: XPF: Conditional object used" ),
    DEF_VAR( 0,HAVEKMP_X_DEFOBJ,	kmp_xpf_def_obj,	"KMP: XPF: Definition object used" ),
    DEF_VAR( 0,HAVEKMP_X_RANDOM,	kmp_xpf_random,		"KMP: XPF: Random scenarios used" ),
    DEF_VAR( 0,HAVEKMP_EPROP_SPEED,	kmp_eprop_speed,	"KMP: Epropeller with speed setting" ),
    DEF_VAR( 0,HAVEKMP_COOB_R,		kmp_coob_riidefii,	"KMP: Conditional OOB by Riidefi" ),
    DEF_VAR( 0,HAVEKMP_COOB_K,		kmp_coob_khacker,	"KMP: Conditional OOB by kHacker35000vr" ),
    DEF_VAR( 0,HAVEKMP_UOOB,		kmp_uncond_oob,		"KMP: Unconditional OOB" ),

    {0,0,0,0,0}
};

//-----------------------------------------------------------------------------

static const feature_var_t lex_section_list[] =
{
    DEF_VAR( 1,HAVELEXS_FEAT,		lex_sect_feat,	"LEX section FEAT" ),
    DEF_VAR( 0,HAVELEXS_TEST,		lex_sect_test,	"LEX section TEST" ),
    DEF_VAR( 0,HAVELEXS_SET1,		lex_sect_set1,	"LEX section SET1" ),
    DEF_VAR( 0,HAVELEXS_CANN,		lex_sect_cann,	"LEX section CANN" ),
    DEF_VAR( 0,HAVELEXS_HIPT,		lex_sect_hipt,	"LEX section HIPT" ),
    DEF_VAR( 0,HAVELEXS_RITP,		lex_sect_ritp,	"LEX section RITP" ),

    // [[new-lex-sect]]
    {0,0,0,0,0}
};

//-----------------------------------------------------------------------------

static const feature_var_t lex_feature_list[] =
{
    DEF_VAR( 0,HAVELEXF_TEST_ACTIVE,	lex_test_active,"LEX: Active test scenario" ),
    DEF_VAR( 0,HAVELEXF_ITEM_RANGE,	lex_item_range,	"LEX: Expand the range of item positions" ),
    DEF_VAR( 0,HAVELEXF_CANNON,		lex_cannon,	"LEX: Alternative cannon settings" ),
    DEF_VAR( 0,HAVELEXF_HIDE_POS,	lex_hide_pos,	"LEX: Hide position tracker conditionally" ),
    DEF_VAR( 0,HAVELEXF_START_ITEM,	lex_start_item,	"LEX: Player can get item before start of race." ),
    DEF_VAR( 0,HAVELEXF_APPLY_OTL,	lex_apply_otl,	"LEX: Track applies alternative online time limit." ),
    DEF_VAR( 0,HAVELEXF_RND_ITPH,	lex_rnd_itph,	"LEX: Random next links @KMP:ITPH." ),

    // [[new-lex-sect]]
    {0,0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////
// [[feature_list_var_type]]

enum feature_list_var_type
{
    FVT_BOOL_ARRAY,
    FVT_BYTE_ARRAY,
    FVT_UINT_BITS,
};

//-----------------------------------------------------------------------------
// [[feature_list_t]]

typedef struct feature_list_t
{
    const feature_var_t	*var;		// var list
    uint		var_type;	// one of enum feature_list_var_type
    uint		offset;		// offset in analyze_szs_t
    uint		max_value;	// maximum possible value
    ccp			ref_prefix;	// prefix for references
    ccp			heading1;	// heading1
    ccp			heading2;	// heading2
}
feature_list_t;

//-----------------------------------------------------------------------------

static const feature_list_t feature_lists[] =
{
    { szs_feature_list,
		FVT_BYTE_ARRAY,
		offsetof(szs_have_t,szs),
		FZV_MAYBE_IMPACT,
		"SZS file",
		"Additional sub-files found in SZS file",
		"0:no file, 1:unmodified file, 2:modified file" },

    { kmp_feature_list,
		FVT_BOOL_ARRAY,
		offsetof(szs_have_t,kmp),
		FZV_IMPACT,
		"KMP feature",
		"Special features found in KMP (sub-file course.kmp)",
		"0:not found, 1:found without impact, 3:found with impact" },

    { lex_section_list,
		FVT_UINT_BITS,
		offsetof(szs_have_t,lex_sect),
		FZV_MAYBE_IMPACT,
		"LEX section",
		"Sections found in LEX (sub-file course.lex)",
		"0:section not found, 1:section found, 2:found and relevant" },

    { lex_feature_list,
		FVT_UINT_BITS,
		offsetof(szs_have_t,lex_feat),
		FZV_IMPACT,
		"LEX feature",
		"Special features found in LEX (sub-file course.lex)",
		"0:not found, 1:found without impact, 3:found with impact" },

    {0,0,0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeFeaturesSZS ( features_szs_t * fs )
{
    DASSERT(fs);
    memset(fs,0,sizeof(*fs));
    fs->size_be = htons(sizeof(*fs));
}

///////////////////////////////////////////////////////////////////////////////
#if CHECK_FEATURES

 static uint CheckUsageFeaturesSZS()
 {
    GetFeaturesModes();

    static uint err_count = 0;
    static bool done = false;
    if (done)
	return err_count;
    done = true;

    features_szs_t fs;
    InitializeFeaturesSZS(&fs);
    u8 *fs_data = (u8*)&fs;

    ccp err_msg = "Invalid feature_var_t[]\n";

    const feature_list_t *list;
    for ( list = feature_lists; list->var; list++ )
    {
	const feature_var_t *var;
	for ( var = list->var; var->offset; var++ )
	{
	    uint offset = var->offset;
	    if ( offset < FIRST_FEATURE_SZS_OFFSET || offset > sizeof(features_szs_t) )
	    {
		if (!err_count++)
		    ERROR0(ERR_INVALID_DATA,err_msg,0);
		fprintf(stderr,"!! * Offset %d out of range.\n",offset);
	    }
	    else
		fs_data[offset]++;
	}
    }

    uint offset;
    for ( offset = FIRST_FEATURE_SZS_OFFSET; offset < sizeof(features_szs_t); offset++ )
    {
	const u8 count = fs_data[offset];
	if ( count != 1 )
	{
	    if (!err_count++)
		ERROR0(ERR_INVALID_DATA,err_msg,0);
	    if (!count)
		fprintf(stderr,"!! * Offset %d not used.\n",offset);
	    else
		fprintf(stderr,"!! * Offset %d used %u times.\n",offset,count);
	}
    }

    ResetFeaturesSZS(&fs);
    return err_count;
 }

#endif // CHECK_FEATURES
///////////////////////////////////////////////////////////////////////////////

void SetupFeaturesSZS ( features_szs_t *fs, const szs_have_t *have, bool is_lex )
{
 #if CHECK_FEATURES
    CheckUsageFeaturesSZS();
 #endif

    const feature_list_t *list;
    const feature_var_t *var;

    for ( list = feature_lists; list->var; list++ )
    {
	switch (list->var_type)
	{
	 case FVT_BOOL_ARRAY:
	    {
		const u8 *src = (u8*)have + list->offset;
		for ( var = list->var; var->offset; var++ )
		    ((u8*)fs)[var->offset]
				= is_lex && var->no_lex
				? list->max_value
				: src[var->index] > 0 ? list->max_value : 0;
	    }
	    break;

	 case FVT_BYTE_ARRAY:
	    {
		const u8 *src = (u8*)have + list->offset;
		for ( var = list->var; var->offset; var++ )
		    ((u8*)fs)[var->offset]
				= is_lex && var->no_lex
				? list->max_value
				: src[var->index];
	    }
	    break;

	 case FVT_UINT_BITS:
	    {
		const uint val = *(uint*)((u8*)have + list->offset);
		for ( var = list->var; var->offset; var++ )
		    ((u8*)fs)[var->offset]
				= is_lex && var->no_lex
				? list->max_value
				: ( val & 1 << var->index ) != 0 ? list->max_value : 0;
	    }
	    break;

	 default:
	    ASSERT(0);
	}
    }

    //-------------------------------------------------------------------------

    NormalizeFeatures(fs);

    // ??? [[2do]]
}

///////////////////////////////////////////////////////////////////////////////

void SetIsLexFeaturesSZS ( features_szs_t *fs )
{
    DASSERT(fs);

 #if 1
    // short cut for code below
    fs->f_lex = fs->lex_sect_feat = 1;
 #else
    // non optimized code
    const feature_list_t *list;
    for ( list = feature_lists; list->var; list++ )
    {
	const feature_var_t *var;
	for ( var = list->var; var->offset; var++ )
	    if (var->no_lex)
		((u8*)fs)[var->offset] = 1;
    }
 #endif
}

///////////////////////////////////////////////////////////////////////////////

int GetFeaturesStatusSZS ( const features_szs_t *fs )
{
    // returns
    //	0: all features set to null
    //	1: all features except f_lex + lex_sect_feat are set to null
    //	2: any features except f_lex + lex_sect_feat is not NULL

    DASSERT(fs);

    features_szs_t temp;
    InitializeFeaturesSZS(&temp);
    if (!memcmp(&temp,fs,sizeof(temp)))
	return 0;

    temp.f_lex		= fs->f_lex;
    temp.lex_sect_feat	= fs->lex_sect_feat;
    return memcmp(&temp,fs,sizeof(temp)) ? 2 : 1;
}

///////////////////////////////////////////////////////////////////////////////

void PrintFeaturesSZS
(
    PrintScript_t	*ps,		// valid output definition
    const features_szs_t *fs,		// valid source
    bool		is_lex,		// TRUE: Create output for a LEX file
    int			comments,	// >0: add extended comments
    int			print_options,	// >0: print options as additional bits
    int			print_modes,	// >0: append ",MODES" / >1: append aligned MODES
    u8			include,	// print only if all bits match
    u8			exclude		// exclude if any bit match
)
{
    DASSERT(ps);
    DASSERT(fs);

    PrintScriptVars(ps,0,
	"\n"
	"# The base value (lower 4 bits) of each feature is one of:\n"
	"#   %u: file or feature not found.\n"
	"#   %u: file or feature found, but no impact.\n"
	"#   %u: file or feature found, maybe some settings with impact.\n"
	"#   %u: feature found with impact to game play.\n",
	FZV_UNDEF, FZV_NO_IMPACT, FZV_MAYBE_IMPACT, FZV_IMPACT );

    if ( print_options > 0 )
	PrintScriptVars(ps,0,
		"#\n"
		"# If value is %u, then some additional bits may be set:\n"
		"#   0x%02x: Impact to game play of time-trial.\n"
		"#   0x%02x: Impact to game play of offline (local) races and battles.\n"
		"#   0x%02x: Impact to game play of online races and battles.\n",
		FZV_IMPACT, FZM_TIMETRIAL, FZM_OFFLINE, FZM_ONLINE );

    if ( ps->fform == PSFF_UNKNOWN && comments >= 0 )
	ps->fform = PSFF_ASSIGN;

    int head_mode = 0;
    const u8 *modes = (u8*)GetFeaturesModes();
    const feature_list_t *list;

    for ( list = feature_lists; list->var; list++ )
    {
	if ( comments >= 0 )
	    head_mode = 2;
	else if ( comments >= -1 )
	    head_mode = 1;

	const feature_var_t *var;
	for ( var = list->var; var->offset; var++ )
	{
	    if ( is_lex && var->no_lex )
		continue;

	    const u8 mode = modes[var->offset];
	    if ( ( mode & include ) != include || mode & exclude )
		continue;

	    const u8 value = ((u8*)fs)[var->offset];
	    if ( value || comments >= -2 )
	    {
		if ( head_mode == 2 )
		{
		    const int hlen1 = strlen(list->heading1);
		    const int hlen2 = list->heading2 ? strlen(list->heading2) : 0;
		    const int sep_len = ( hlen1 > hlen2 ? hlen1 : hlen2 ) + 2;
		    PrintScriptVars(ps,0,"\n#%.*s\n# %s\n", sep_len, Minus300, list->heading1 );
		    if (list->heading2)
			PrintScriptVars(ps,0,"# %s\n", list->heading2 );
		    PrintScriptVars(ps,0,"#%.*s\n", sep_len, Minus300 );
		}
		head_mode = 0;

		if ( comments >= 0 )
		{
		    fputc('\n',ps->f);
		    if ( comments >= 2 )
			PrintScriptVars(ps,0,"# %s #%u, feature offset %u, source index %u\n",
				    list->ref_prefix, (int)(var-list->var),
				    var->offset, var->index );
		    PrintScriptVars(ps,0,"# %s\n",var->comment);
		    ccp effect = GetFeaturesEffects(mode);
		    if (effect)
			PrintScriptVars(ps,0,"# > %s\n",effect);
		}

		char valbuf[30];
		if ( value & FZV_M_OPTIONS && print_options > 0 )
		    snprintf(valbuf,sizeof(valbuf),"0x%02x",value);
		else
		    snprintf(valbuf,sizeof(valbuf),"%u", value & FZV_M_VALUE );

		if ( print_modes > 0 )
		    PrintScriptVars(ps,0,"%s=\"%s,%s\"\n",
				var->name, valbuf, GetFeaturesMode(mode,print_modes>1) );
		else
		    PrintScriptVars(ps,0,"%s=%s\n",var->name,valbuf);
	    }
	}
    }

    if ( comments >= -1 )
	fputc('\n',ps->f);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			features_szs_mode_t		///////////////
///////////////////////////////////////////////////////////////////////////////

const features_szs_t * GetFeaturesModes()
{
    static features_szs_t m = {0}; // keep name short!

    if (!m.size_be)
    {
	InitializeFeaturesSZS(&m);

	const u8 invalid = FZM_SECTION | FZM_BATTLE | FZM_TIMETRIAL;
	memset((u8*)&m+sizeof(m.size_be),invalid,sizeof(m)-sizeof(m.size_be));

	const u8 no_timetrial = FZM_OFFLINE | FZM_ONLINE;

	m.f_lex			= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_item_slot_table	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_objflow		= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_ght_item		=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_ght_item_obj	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_ght_kart		=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_ght_kart_obj	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.f_minigame		=              FZM_GAMEPLAY | FZM_BATTLE | FZM_M_WHERE;
	m.f_aiparam_baa		=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_OFFLINE;
	m.f_aiparam_bas		=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_OFFLINE;

	m.kmp_goomba_size	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_woodbox_ht	=              FZM_GAMEPLAY | FZM_M_TYPE | no_timetrial;
	m.kmp_mushroom_car	= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_penguin_pos	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_second_ktpt	= FZM_VISUAL                | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_extended_pflags	= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_xpf_cond_obj	= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_xpf_def_obj	= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_xpf_random	= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | no_timetrial;
	m.kmp_eprop_speed	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_coob_riidefii	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_coob_khacker	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.kmp_uncond_oob	=			      FZM_M_TYPE | FZM_M_WHERE;

	// existence of a section don't have impact to gameplay => see lex attributes
	m.lex_sect_feat		=							  FZM_SECTION;
	m.lex_sect_test		= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | no_timetrial | FZM_SECTION;
	m.lex_sect_set1		=	       FZM_GAMEPLAY | FZM_M_TYPE | FZM_ONLINE   | FZM_SECTION;
	m.lex_sect_cann		=	       FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE  | FZM_SECTION;
	m.lex_sect_hipt		= FZM_VISUAL		    | FZM_M_TYPE | no_timetrial | FZM_SECTION;
	m.lex_sect_ritp		=	       FZM_GAMEPLAY | FZM_M_TYPE | no_timetrial | FZM_SECTION;

	// [[new-lex-sect]]

	m.lex_test_active	= FZM_VISUAL | FZM_GAMEPLAY | FZM_M_TYPE | no_timetrial;
	m.lex_item_range	=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_ONLINE;
	m.lex_cannon		=              FZM_GAMEPLAY | FZM_M_TYPE | FZM_M_WHERE;
	m.lex_hide_pos		= FZM_VISUAL                | FZM_M_TYPE | no_timetrial;
	m.lex_start_item	=	       FZM_GAMEPLAY | FZM_M_TYPE | FZM_ONLINE;
	m.lex_apply_otl		=	       FZM_GAMEPLAY | FZM_M_TYPE | FZM_ONLINE;
	m.lex_rnd_itph		=	       FZM_GAMEPLAY | FZM_M_TYPE | no_timetrial;

     #if CHECK_FEATURES
	uint err_count = 0;
	const feature_list_t *list;
	for ( list = feature_lists; list->var; list++ )
	{
	    const feature_var_t *var;
	    for ( var = list->var; var->offset; var++ )
	    {
		u8 value = ((u8*)&m)[var->offset];
		if ( value == invalid )
		{
		    if (!err_count++)
			ERROR0(ERR_INVALID_DATA,"Invalid features modes\n");
		    fprintf(stderr,"!! * Mode %s not set.\n",var->name);
		}
	    }
	}
     #endif
    }
    return &m;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetFeaturesMode ( features_szs_mode_t mode, bool align )
{
    char *buf = GetCircBuf(CHAR_BIT+1);
    char *src = "SVGBRTLO", *dest = buf;
    u8 mask;
    for ( mask = 1; mask; mask <<= 1, src++ )
	if ( mode & mask )
	    *dest++ = *src;
	else if (align)
	    *dest++ = '-';
    *dest = 0;
    return buf;
}

//-----------------------------------------------------------------------------

ccp GetFeaturesEffects ( features_szs_mode_t mode )
{
    struct tab_t { u8 not; u8 mode; ccp info; };
    static const struct tab_t *ptr, tab[] =
    {
	{ 0,FZM_SECTION,		"section" },

	{ 1,FZM_M_KIND,			"no impact" },
	{ 0,FZM_M_KIND,			"visual and gameplay impact" },
	{ 0,FZM_VISUAL,			"visual impact only" },
	{ 0,FZM_GAMEPLAY,		"gameplay impact" },

	{ 0,FZM_M_TYPE,			0 },
	{ 0,FZM_BATTLE,			"battle only" },
	{ 0,FZM_RACING,			"racing only" },

	{ 0,FZM_M_WHERE,		0 },
	{ 0,FZM_OFFLINE|FZM_ONLINE,	"except time trial" },
	{ 0,FZM_OFFLINE|FZM_TIMETRIAL,	"offline incl. time trial" },
	{ 0,FZM_ONLINE|FZM_TIMETRIAL,	"time trial and online" },
	{ 0,FZM_TIMETRIAL,		"time trial only" },
	{ 0,FZM_OFFLINE,		"offline except time trial" },
	{ 0,FZM_ONLINE,			"online only" },

	{0,0}
    };

    char buf[200], *dest = buf, *sep = "";
    for ( ptr = tab; ptr->mode; ptr++ )
    {
	const u8 mode1 = ptr->not ? ~mode : mode;
	if ( (mode1 & ptr->mode) == ptr->mode )
	{
	    if (ptr->info)
	    {
		dest = StringCat2E(dest,buf+sizeof(buf),sep,ptr->info);
		sep = ", ";
	    }
	    if (!ptr->not)
		mode &= ~ptr->mode;
	}
    }

    return dest == buf ? 0 : CopyCircBuf0(buf,dest-buf);
}

//-----------------------------------------------------------------------------

ccp GetFeaturesEffectsByOffset ( uint offset )
{
    if ( offset < 2 && offset >= sizeof(features_szs_t) )
	return 0;

    const features_szs_t *fs = GetFeaturesModes();
    return GetFeaturesEffects(((u8*)fs)[offset]);
}

//-----------------------------------------------------------------------------

void NormalizeFeatures ( features_szs_t *fs )
{
    if (fs)
    {
	const features_szs_t *mode = GetFeaturesModes();
	for ( int idx = 2; idx < sizeof(features_szs_t); idx++ )
	{
	    const u8 md = ((u8*)mode)[idx];

	    u8 *ptr = ((u8*)fs) + idx;
	    u8 val = *ptr & FZV_M_VALUE;
	    if ( val >= FZV_IMPACT )
	    {
		if ( md & FZM_GAMEPLAY )
		    val = FZV_IMPACT | md & FZV_M_OPTIONS;
		else
		    val = FZV_NO_IMPACT;
	    }
	    *ptr = val;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lecode extension LEX		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanLexFile
	( cvp data, uint data_size, lex_stream_func func,
	  cvp user_ptr, int user_int )
{
    if ( !data || data_size < sizeof(lex_header_t) )
	return ERR_INVALID_DATA;

    const lex_header_t *hd = (lex_header_t*)data;
    if (memcmp(hd->magic,LEX_BIN_MAGIC,4))
	return ERR_INVALID_DATA;

    if ( ntohs(hd->major_version) != LEX_MAJOR_VERSION )
	return ERR_INVALID_VERSION;

    const uint size = ntohl(hd->size);
    const uint off  = ntohl(hd->element_off);
    if ( (size&3) || size > data_size || (off&3) || off + 4 > size )
	return ERR_INVALID_DATA;

    return ScanLexElements((u8*)data+off,size-off,func,user_ptr,user_int);
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanLexElements
	( cvp data, uint data_size, lex_stream_func func,
	  cvp user_ptr, int user_int )
{
    if ( !data || !data_size || (data_size&3) )
	return ERR_INVALID_DATA;

    enumError max_err = ERR_OK;

    uint off = 0;
    while ( off < data_size )
    {
	const lex_element_t *s = (lex_element_t*)( (u8*)data + off );

	const u32 magic = ntohl(s->magic);
	if (!magic)
	    break;

	const uint size = ntohl(s->size);
	off += sizeof(lex_element_t) + size;
	if ( (size&3) || off > data_size )
	    return ERR_INVALID_DATA;

	PRINT("LEX SECTION: %.4s, size:%x\n",PrintID(&s->magic,4,0),size);
	if (func)
	{
	    const int err = func(magic,s->data,size,user_ptr,user_int);
	    if (err<0)
		return -err;
	    if ( max_err < err )
		 max_err = err;
	}
    }

    return max_err;
}

///////////////////////////////////////////////////////////////////////////////

lex_element_t * FindLexElement ( lex_header_t *head, uint data_size, u32 magic )
{
    if ( !head || data_size <= sizeof(lex_header_t) || (data_size&3) )
	return 0;

    uint file_size = ntohl(head->size);
    if ( file_size > data_size )
	return 0;
    file_size -= sizeof(lex_element_t);

    uint offset = ntohl(head->element_off);
    if ( offset > file_size )
	return 0;

    magic = htonl(magic);
    for(;;)
    {
	lex_element_t *elem = (lex_element_t*)((u8*)head + offset);
	offset += 8 + ntohl(elem->size);
	if ( offset > file_size )
	    return 0;
	if ( elem->magic == magic )
	    return elem;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			fix lex items			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef void (*FixElementFunc) ( lex_t * lex, lex_item_t * item );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void FixLEXElement_SET1 ( lex_set1_t *set1 )
{
    DASSERT(set1);

    uint i;
    for ( i = 0; i < 3; i++ )
    {
	float f = bef4(set1->item_factor.v+i);
	if ( !isnormal(f) || f < 1.0 )
	    write_bef4(set1->item_factor.v+i,1.0);
    }

    set1->start_item = set1->start_item > 0;

// [[time-limit]]
    set1->apply_online_sec = htons(MINMAX0( ntohs(set1->apply_online_sec),
					LE_MIN_ONLINE_SEC, LE_MAX_ONLINE_SEC ));

    // [[new-lex-set1]]
}

//-----------------------------------------------------------------------------

static void FixLEXItem_SET1
(
    lex_t		* lex,		// LEX data structure
    lex_item_t		* item		// item to fix
)
{
    DASSERT(lex);
    DASSERT(item);
    DASSERT( ntohl(item->elem.size) >= sizeof(lex_set1_t) );

    lex_set1_t *set1 = (lex_set1_t*)item->elem.data;
    FixLEXElement_SET1(set1);
}

//-----------------------------------------------------------------------------

static void ClearLEXElement_SET1 ( lex_set1_t *set1 )
{
    DASSERT(set1);
    memset(set1,0,sizeof(*set1));
    FixLEXElement_SET1(set1);
}

//-----------------------------------------------------------------------------

static bool IsActiveLEXElement_SET1 ( const lex_set1_t *set1 )
{
    DASSERT(set1);
    lex_set1_t empty, test = *set1;
    ClearLEXElement_SET1(&empty);
    FixLEXElement_SET1(&test);
    return memcmp(&empty,&test,sizeof(empty)) != 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[new-lex-sect]] upto 4 functions, see SET1 above

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void FixLEXElement_TEST ( lex_test_t *lt )
{
    DASSERT(lt);

    if ( lt->offline_online < LEX_OO_AUTO || lt->offline_online > LEX_OO_ONLINE )
	lt->offline_online = LEX_OO_AUTO;

    if ( lt->n_offline > 4 )
	 lt->n_offline = 4;

    if ( lt->n_online > 99 )
	 lt->n_online = 99;

    if ( lt->cond_bit < -1 || lt->cond_bit > 15 )
	lt->cond_bit = -1;

    if ( lt->game_mode >= LEX_GMODE__N )
	lt->game_mode = LEX_GMODE_AUTO;

    if ( lt->random > 8 )
	lt->random = 0;

    if ( lt->engine > ENGINE__N )
	lt->engine = 0;

    lt->padding = 0;
}

//-----------------------------------------------------------------------------

static void FixLEXItem_TEST
(
    lex_t		* lex,		// LEX data structure
    lex_item_t		* item		// item to fix
)
{
    DASSERT(lex);
    DASSERT(item);
    DASSERT( ntohl(item->elem.size) >= sizeof(lex_test_t) );

    lex_test_t *lt = (lex_test_t*)item->elem.data;
    FixLEXElement_TEST(lt);
}

//-----------------------------------------------------------------------------

static void ClearLEXElement_TEST ( lex_test_t *lt )
{
    DASSERT(lt);
    memcpy(lt,TestDataLEX,sizeof(*lt));
}

//-----------------------------------------------------------------------------

static bool IsActiveLEXElement_TEST ( const lex_test_t *lt )
{
    DASSERT(lt);
    lex_test_t empty, test = *lt;
    ClearLEXElement_TEST(&empty);
    FixLEXElement_TEST(&test);
    return memcmp(&empty,&test,sizeof(empty)) != 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lex_t helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static lex_item_t * FindElementLEX
(
    const lex_t		* lex,		// LEX data structure
    u32			magic		// magic
)
{
    magic = htonl(magic);

    uint i;
    lex_item_t **ss;
    for ( i = 0, ss = lex->item; i < lex->item_used; i++, ss++ )
	if ( (*ss)->elem.magic == magic )
	    return *ss;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static uint GetLexSortOrder ( u32 magic )
{
    switch(magic)
    {
	case LEXS_FEAT:		return 1;
	case LEXS_SET1:		return 2;
	case LEXS_TEST:		return 9;
	// [[new-lex-sect]]
	default:		return 5;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void UpdateHaveLex ( lex_t *lex )
{
    DASSERT(lex);
    uint have_sect = 0;
    uint have_feat = 0;

    uint i;
    lex_item_t **ss;
    for ( i = 0, ss = lex->item; i < lex->item_used; i++, ss++ )
    {
	const u32 magic = ntohl((*ss)->elem.magic);
	const uint size = ntohl((*ss)->elem.size);
	uint j;
	for ( j = 0; j < HAVELEXS__N; j++ )
	    if ( magic == have_lex_sect_info[j].magic
		&& size >= have_lex_sect_info[j].min_size )
	    {
		have_sect |= 1 << j;
		have_feat |= GetLexFeatures(&(*ss)->elem,lex);
		break;
	    }
    }

    lex->have_sect = have_sect;
    lex->have_feat = have_feat;
}

///////////////////////////////////////////////////////////////////////////////

static lex_item_t * AppendElementLEX
(
    lex_t		* lex,		// LEX data structure
    u32			magic,		// magic
    const void		* data,		// data of section
    uint		size,		// size of 'data'
    bool		overwrite	// false: abort if exists and return 0
					// true:  overwrite
)
{
    DASSERT(lex);
    DASSERT( data || !size );
    PRINT("AppendElementLEX(%.4s) have=%02x,%02x, n=%u\n",
		(ccp)&magic, lex->have_sect, lex->have_feat, lex->item_used);


    //--- check if already exists

    lex_item_t **ss;
    const u32 be_magic = htonl(magic);
    uint i;
    for ( i = 0, ss = lex->item; i < lex->item_used; i++, ss++ )
	if ( (*ss)->elem.magic == be_magic )
	{
	    if (!overwrite)
		return 0;
	    break;
	}


    //--- grow item list if necessary

    if ( i == lex->item_size )
    {
	lex->item_size = lex->item_size + 30;
	lex->item = REALLOC( lex->item, lex->item_size * sizeof(*lex->item) );
	// 'ss' becomes invalid!
	ss = lex->item + i;
    }

    if ( i < lex->item_used )
	FREE(*ss);
    else
	lex->item_used++;


    //--- fix item size

    uint size4 = ALIGN32(size,4);
    FixElementFunc fix_func = 0;

    switch(magic)
    {
	case LEXS_FEAT:
	    if ( size4 < sizeof(features_szs_t) )
		size4 = ALIGN32(sizeof(features_szs_t),4);
	    break;

	case LEXS_SET1:
	    fix_func = FixLEXItem_SET1;
	    if ( size4 < sizeof(lex_set1_t) )
		size4 = ALIGN32(sizeof(lex_set1_t),4);
	    break;

	// [[new-lex-sect]]

	case LEXS_TEST:
	    if ( size4 < sizeof(lex_test_t) )
		size4 = ALIGN32(sizeof(lex_test_t),4);
	    break;
    }


    //--- setup item

    lex_item_t *s	= MALLOC(sizeof(lex_item_t)+size4);
    *ss			= s;
    s->sort_order	= GetLexSortOrder(magic);
    s->insert_order	= i;
    s->elem.magic	= be_magic;
    s->elem.size	= htonl(size4);
    memcpy(s->elem.data,data,size);
    if ( size < size4 )
	memset(s->elem.data+size,0,size4-size);

    if (fix_func)
	fix_func(lex,s);

    PRINT("AppendElementLEX(%.4s) have=%02x,%02x, n=%u\n",
		(ccp)&magic, lex->have_sect, lex->have_feat, lex->item_used);
    UpdateHaveLex(lex);
    return s;
}

///////////////////////////////////////////////////////////////////////////////

static bool RemoveElementLEX
(
    lex_t		*lex,		// LEX data structure
    const lex_item_t	*li		// element to remove (robust)
)
{
    if ( !li || !li->elem.magic )
	return 0;

    uint i;
    lex_item_t **ss;
    for ( i = 0, ss = lex->item; i < lex->item_used; i++, ss++ )
	if ( *ss == li )
	{
	    memmove(ss,ss+1,lex->item_used-i);
	    lex->item_used--;
	    FREE((lex_item_t*)li);
	    UpdateHaveLex(lex);
	    return true;
	}
    return false;
}

//-----------------------------------------------------------------------------

static uint RemoveElementByMagicLEX
(
    lex_t		*lex,		// LEX data structure
    u32			magic		// magic
)
{
    if (!magic)
	return 0;

    uint count = 0;
    for(;;)
    {
	lex_item_t *li = FindElementLEX(lex,magic);
	if (!li)
	    return count;
	count += RemoveElementLEX(lex,li);
    }
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_lex_item ( const lex_item_t ** pa, const lex_item_t ** pb )
{
    const lex_item_t *a = *pa;
    const lex_item_t *b = *pb;
    noPRINT(">> SORT: %u.%u : %u.%u\n",
		a->sort_order, a->insert_order,
		b->sort_order, b->insert_order );

    int res = (int)a->sort_order - (int)b->sort_order;
    return res ? res : (int)a->insert_order - (int)b->insert_order;
}

//-----------------------------------------------------------------------------

static void SortItemsLEX
(
    lex_t		* lex		// LEX data structure
)
{
    if ( lex->item_used > 1 )
	qsort( lex->item, lex->item_used, sizeof(*lex->item),
		(qsort_func)cmp_lex_item );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX features			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint GetLexFeaturesSET1 ( const lex_element_t *elem, lex_t *lex )
{
    // if 'lex' not NULL: store additional data 

    uint have_feat = 0;
    const uint size = ntohl(elem->size);
    const lex_set1_t *set1 = (lex_set1_t*)elem->data;

    if ( size >= offsetof(lex_set1_t,item_factor) + sizeof(set1->item_factor) )
    {
	if ( bef4(set1->item_factor.v+0) != 1.0
	  || bef4(set1->item_factor.v+1) != 1.0
	  || bef4(set1->item_factor.v+2) != 1.0
	   )
	{
	    have_feat |= 1 << HAVELEXF_ITEM_RANGE;
	}
    }

    if ( size > offsetof(lex_set1_t,start_item) && set1->start_item )
	have_feat |= 1 << HAVELEXF_START_ITEM;

// [[time-limit]]
    if ( size > offsetof(lex_set1_t,apply_online_sec) )
    {
	const uint apply_otl = ntohs(set1->apply_online_sec);
	if (apply_otl)
	{
	    have_feat |= 1 << HAVELEXF_APPLY_OTL;
	    if (lex)
		lex->apply_otl = apply_otl;
	}
    }

    // [[new-lex-set1]]

    return have_feat;
}

//-----------------------------------------------------------------------------

static uint GetLexFeaturesCANN ( const lex_element_t *elem )
{
    return ntohl(elem->size) != sizeof(CannonDataLEX)
	|| memcmp(elem->data,CannonDataLEX,sizeof(CannonDataLEX))
		? 1 << HAVELEXF_CANNON : 0;
}

//-----------------------------------------------------------------------------

static uint GetLexFeaturesHIPT ( const lex_element_t *elem )
{
    return ntohl(elem->size) >= sizeof(lex_hipt_rule_t)
		? 1 << HAVELEXF_HIDE_POS : 0;
}

//-----------------------------------------------------------------------------

static uint GetLexFeaturesRITP ( const lex_element_t *elem )
{
    return ntohl(elem->size) >= sizeof(lex_ritp_rule_t)
		? 1 << HAVELEXF_RND_ITPH : 0;
}

//-----------------------------------------------------------------------------

static uint GetLexFeaturesTEST ( const lex_element_t *elem )
{
    DASSERT(elem);
    return IsActiveLEXElement_TEST((lex_test_t*)elem->data)
		? 1 << HAVELEXF_TEST_ACTIVE : 0;
}

//-----------------------------------------------------------------------------

uint GetLexFeatures ( const lex_element_t *elem, lex_t *lex  )
{
    // if 'lex' not NULL: store additional data 

    switch(ntohl(elem->magic))
    {
	case LEXS_SET1: return GetLexFeaturesSET1(elem,lex);
	case LEXS_CANN: return GetLexFeaturesCANN(elem);
	case LEXS_HIPT: return GetLexFeaturesHIPT(elem);
	case LEXS_RITP: return GetLexFeaturesRITP(elem);
	case LEXS_TEST: return GetLexFeaturesTEST(elem);
	// [[new-lex-sect]]
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const have_lex_info_t have_lex_feat_info[HAVELEXF__N] =
{
    { HAVELEXF_TEST_ACTIVE,	"test",		0 },
    { HAVELEXF_ITEM_RANGE,	"itempos",	0 },
    { HAVELEXF_CANNON,		"cannon",	0 },
    { HAVELEXF_HIDE_POS,	"hipt",		0 },
    { HAVELEXF_START_ITEM,	"startitem",	0 },
    { HAVELEXF_APPLY_OTL,	"apply-otl",	0 },
    { HAVELEXF_RND_ITPH,	"rnd-itph",	0 },
    //--- add new elements here (order is important)
};

///////////////////////////////////////////////////////////////////////////////

ccp CreateFeatureInfoLEX
	( have_lex_feat_t special, bool add_value, ccp return_if_empty )
{
    static char buf[500];
    char *dest = buf;

    if (add_value)
	dest = snprintfE( dest, buf+sizeof(buf), "%u=" , special );

    uint i, mask;
    ccp sep = "";
    for ( i = 0, mask = 1; i < HAVELEXF__N; i++, mask <<= 1 )
	if ( special & mask )
	{
	    dest = StringCat2E(dest,buf+sizeof(buf),sep,have_lex_feat_info[i].name);
	    sep = ",";
	}

    return dest == buf ? return_if_empty : CopyCircBuf0(buf,dest-buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			have lex sections		///////////////
///////////////////////////////////////////////////////////////////////////////
// order is important, compare have_lex_sect_t and ct.wiimm.de

const have_lex_info_t have_lex_sect_info[HAVELEXS__N] =
{
    { LEXS_SET1, "set1",	1 },				// HAVELEXS_SET1
    { LEXS_CANN, "cannon",	0 },				// HAVELEXS_CANN
    { LEXS_TEST, "test",	1 },				// HAVELEXS_TEST
    { LEXS_HIPT, "hidepos",	sizeof(lex_hipt_rule_t) },	// HAVELEXS_HIPT
    { LEXS_FEAT, "features",	2 },				// HAVELEXS_FEAT
    { LEXS_RITP, "rnditph",	sizeof(lex_ritp_rule_t) },	// HAVELEXS_RITP
    //--- add new sections here (order is important)
    // [[new-lex-sect]]
};

///////////////////////////////////////////////////////////////////////////////

ccp CreateSectionInfoLEX
	( have_lex_sect_t special, bool add_value, ccp return_if_empty )
{
    static char buf[500];
    char *dest = buf;

    if (add_value)
	dest = snprintfE( dest, buf+sizeof(buf), "%u=" , special );

    uint i, mask;
    ccp sep = "";
    for ( i = 0, mask = 1; i < HAVELEXS__N; i++, mask <<= 1 )
	if ( special & mask )
	{
	    dest = StringCat2E(dest,buf+sizeof(buf),sep,have_lex_sect_info[i].name);
	    sep = ",";
	}

    return dest == buf ? return_if_empty : CopyCircBuf0(buf,dest-buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    LEX setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLEX ( lex_t * lex )
{
    DASSERT(lex);
    memset(lex,0,sizeof(*lex));

    lex->fname = EmptyString;
}

///////////////////////////////////////////////////////////////////////////////

void ResetLEX ( lex_t * lex )
{
    if (lex)
    {
	FreeString(lex->fname);
	if (lex->raw_data)
	    FREE(lex->raw_data);

	if (lex->item)
	{
	    uint i;
	    for ( i = 0; i < lex->item_used; i++ )
		if (lex->item[i])
		    FREE(lex->item[i]);
	    FREE(lex->item);
	}
	InitializeLEX(lex);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UpdateLEX ( lex_t * lex, bool add_missing, bool add_test )
{
    DASSERT(lex);

    if ( add_missing && lex->develop )
    {
	//--- FEAT
	AppendFeatLEX(lex,false,0);
    }

    if (add_missing)
    {
	//--- SET1
	AppendSet1LEX(lex,false);

	//--- CANN
	AppendCannLEX(lex,false);

	//--- HIPT
	AppendHiptLEX(lex,false);

	//--- RITP
	AppendRitpLEX(lex,false);
    }

    if (add_test) //--- TEST
	AppendTestLEX(lex,false);

    SortItemsLEX(lex);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendFeatLEX
	( lex_t * lex, bool overwrite, const features_szs_t *src_fs )
{
    features_szs_t fs;
    if (src_fs)
	fs = *src_fs;
    else
	InitializeFeaturesSZS(&fs);
    SetIsLexFeaturesSZS(&fs);

    return AppendElementLEX(lex,LEXS_FEAT,&fs,sizeof(fs),overwrite);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendSet1LEX ( lex_t * lex, bool overwrite )
{
    lex_set1_t set1;
    memset(&set1,0,sizeof(set1));
    // AppendElementLEX() does call FixLEXItem_SET1()
    return AppendElementLEX(lex,LEXS_SET1,&set1,sizeof(set1),overwrite);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendCannLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_CANN, CannonDataLEX,
				sizeof(CannonDataLEX), overwrite );
}

//-----------------------------------------------------------------------------

lex_item_t * AppendHiptLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_HIPT, 0, 0, overwrite );
}

//-----------------------------------------------------------------------------

lex_item_t * AppendRitpLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_RITP, 0, 0, overwrite );
}

//-----------------------------------------------------------------------------

lex_item_t * AppendTestLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_TEST, TestDataLEX,
				sizeof(TestDataLEX), overwrite );
}

// [[new-lex-sect]]

///////////////////////////////////////////////////////////////////////////////

void DumpElementsLEX ( FILE *f, lex_t *lex, bool hexdump )
{
    DASSERT(f);
    DASSERT(lex);

    fprintf(f,"LEX SECTIONS: %u/%u\n",lex->item_used,lex->item_size);

    uint i;
    for ( i = 0; i < lex->item_used; i++ )
    {
	lex_item_t *s = lex->item[i];
	const uint size = ntohl(s->elem.size);
	fprintf(f,"   %-4.4s %08x, size: %5u = 0x%04x\n",
		PrintID(&s->elem.magic,4,0), s->elem.magic, size, size );
	if ( size && hexdump )
	    HexDump16(f,8,0,s->elem.data,size);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lex parser functions		///////////////
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

    uint result = 0;
    if ( si && si->lex )
    {
	result = 1;

	if (si->lex->fname)
	{
	    ccp fname = strrchr(si->lex->fname,'/');
	    if (fname)
		fname++;
	    else
		fname = si->lex->fname;

	    if ( !strcasecmp(fname,"course.lex")
		|| !strcasecmp(fname,"course.txt")
		|| !strcasecmp(fname,"course.txt.lex") )
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

static const struct FuncTable_t lex_func_tab[] =
{
    { 0, 0, "ISLEX", F_isLEX, 0,	// replace only function call
	"int", "isLEX()", 0 },		// but not info

    {0,0,0,0,0,0,0,0}
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			setup lex parser		///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsLEX()
{
    static VarMap_t vm = { .force_case = LOUP_UPPER };
    if (!vm.used)
    {
	DefineMkwVars(&vm);
	DefineParserFuncTab(lex_func_tab,FF_LEX);
	DefineParserVars(&vm);
    }

    return &vm;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ScanRawLEX()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DumpLEXFunc
	( u32 magic, const u8 *data, uint size, cvp user_ptr, int user_int )
{
    u32 m = htonl(magic);
    printf("   %s 0x%08x, %5u = 0x%04x bytes\n",
		PrintID(&m,4,0), magic, size, size );
    if ( user_int < 0 || user_int >= 0x10 )
    {
	if ( user_int > 0 && size > user_int )
	    size = user_int;
	HexDump16(stdout,5,0,data,size);
	putchar('\n');
    }
    return ERR_OK;
}

//-----------------------------------------------------------------------------

enumError ScanLEXFunc
	( u32 magic, const u8 *data, uint size, cvp user_ptr, int user_int )
{
 #if HAVE_PRINT
    u32 m = htonl(magic);
    PRINT(">>>>> %-4.4s %08x : size: %u = 0x%x\n",
		PrintID(&m,4,0), magic, size, size );
 #endif

    lex_t *lex = (lex_t*)user_ptr;
    DASSERT(lex);

    switch (magic)
    {
	case LEXS_TERMINATE:
	case LEXS_IGNORE:
	    // ignore
	    break;

	default:
	    AppendElementLEX(lex,magic,data,size,true);
	 #if HAVE_PRINT0
	    if ( logging >= 2 )
		DumpElementsLEX(stdout, lex, logging >= 3 );
	 #endif
    }
    return ERR_OK;
}

//-----------------------------------------------------------------------------

enumError ScanRawLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    uint		dump_mode	// 0: add element to lex->item
					// 1: dump scanned elements
					// >=0x10: add hexdump of max # bytes
)
{
    DASSERT(lex);
    if (init_lex)
	InitializeLEX(lex);

    if ( IsValidLEX(data,data_size,data_size,lex->fname) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid LEX file (size %u): %s\n"
		"Add option --lex=force or --force to ignore some validity checks.",
		data_size, lex->fname ? lex->fname : "?" );
    }

    lex->fform = FF_LEX;
    enumError err = ScanLexFile( data, data_size,
			dump_mode ? DumpLEXFunc : ScanLEXFunc, lex, dump_mode );
    if (err)
	ERROR0(err,"Scanning of LEX file failed: %s\n",lex->fname);
    else
	UpdateLEX(lex,opt_complete,false);

    if ( logging >= 1 )
	DumpElementsLEX( stdout, lex, logging >= 3 );
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  ScanTextLEX()			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[lex_section_id]]

typedef enum lex_section_id
{
    LID_UNKNOWN,
    LID_IGNORE,
    LID_SETUP,
    LID_HEXDUMP,

    LID_SETTINGS1,
    LID_CANNON,
    LID_HIDE_PT,
    LID_RND_ITPH,
    LID_TEST,

    // [[new-lex-sect]]
}
lex_section_id;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t lex_section_name[] =
{
    //------------------------------------------------------------------------
    // ID		NAME		INFO				unused
    //------------------------------------------------------------------------

    { LID_IGNORE,	"END",		"--",				0 },
    { LID_SETUP,	"SETUP",	"Setup",			0 },
    { LID_HEXDUMP,	"HEXDUMP",	"Hexdump of unknown section",	0 },

    { LID_SETTINGS1,	"SET1",		"Basic settings",		0 },
    { LID_CANNON,	"CANN",		"Cannon settings",		0 },
    { LID_HIDE_PT,	"HIPT",		"Hide position tracker",	0 },
    { LID_RND_ITPH,	"RITP",		"Random next-links @KMP:ITPH",	0 },
    { LID_TEST,		"TEST",		"Test case",			0 },

    // [[new-lex-sect]]
    { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t lex_offline_online[] =
{
  { LEX_OO_AUTO,		"AUTO",		"0",	 0 },
  { LEX_OO_OFFLINE,		"OFFLINE",	"1",	 0 },
  { LEX_OO_OFFLINE,		"NEVER",	0,	 0 },
  { LEX_OO_ONLINE,		"ONLINE",	"2",	 0 },
  { LEX_OO_ONLINE,		"ALWAYS",	0,	 0 },
  { -9,				"OFF",		"-1",	-1 },
  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t lex_game_mode[] =
{
  { LEX_GMODE_AUTO,	"AUTO",		"0",	-1 },
  { LEX_GMODE_STD,	"STANDARD",	"1",	KMP_GMODE_STD },
  { LEX_GMODE_STD,	"STD",		0,	KMP_GMODE_STD },
  { LEX_GMODE_BALLOON,	"BALLOON",	"2",	KMP_GMODE_BALLOON },
  { LEX_GMODE_COIN,	"COIN",		"3",	KMP_GMODE_COIN },
  { LEX_GMODE_VERSUS,	"VERSUS",	"4",	KMP_GMODE_VERSUS },
  { LEX_GMODE_ITEMRAIN,	"ITEMRAIN",	"5",	KMP_GMODE_ITEMRAIN },
  { -1,			"OFF",		"-1",	-1 },
  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t lex_engine[] =
{
  { LEX_ENGINE_AUTO,	"AUTO",		"0",		-1 },
  { LEX_ENGINE_BATTLE,	"BATTLE",	0,		ENGINE_BATTLE	},
  { LEX_ENGINE_50,	"50CC",		0,		ENGINE_50	},
  { LEX_ENGINE_100,	"100CC",	0,		ENGINE_100	},
  { LEX_ENGINE_150,	"150CC",	0,		ENGINE_150	},
  { LEX_ENGINE_200,	"200CC",	0,		ENGINE_200	},
  { LEX_ENGINE_150M,	"150M",		"150MIRROR",	ENGINE_150M	},
  { LEX_ENGINE_200M,	"200M",		"200MIRROR",	ENGINE_200M	},
  { -1,			"OFF",		"-1",		-1 },
  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanLEXElementHexdump
(
    lex_t		* lex,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(lex);
    DASSERT(si);
    TRACE("ScanLEXElementHexdump()\n");


    //--- setup data

    u32 magic = LEXS_IGNORE;
    ScanParam_t ptab[] =
    {
	{ "MAGIC",	SPM_U32,	&magic, },
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));


    //--- main loop

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	    ScanParamSI(si,ptab);
	else
	    ScanHexlineSI(si,fb,true);
    }
    CheckLevelSI(si);

    if ( magic != LEXS_IGNORE && magic != LEXS_TERMINATE )
	AppendElementLEX(lex,magic,fb->buf,fb->ptr-fb->buf,true);

    ResetFastBuf(fb);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanLEXElement_SET1
(
    lex_t		* lex,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(lex);
    DASSERT(si);
    TRACE("ScanLEXElement_SET1()\n");


    //--- setup data

    lex_set1_t set1 = { .item_factor.v = {1,1,1} };
    //HexDump16(stdout,0,0,&set1,sizeof(set1));

    const ScanParam_t ptab[] =
    {
	{ "ITEM-POS-FACTOR",	SPM_FLOAT3_BE,	&set1.item_factor },
	{ "START-ITEM",		SPM_U8,		&set1.start_item },
	{ "PADDING-0D",		SPM_U8,		&set1.padding_0d },
// [[time-limit]]
	{ "APPLY-ONLINE-SEC",	SPM_U16_BE,	&set1.apply_online_sec },
	// [[new-lex-set1]]
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));
    AppendFastBuf(fb,&set1,sizeof(set1));


    //--- main loop

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	    ScanParamSI(si,ptab);
	else
	    ScanHexlineSI(si,fb,true);
    }
    CheckLevelSI(si);

    memcpy(fb->buf,&set1,sizeof(set1));
    HEXDUMP16(0,0,fb->buf,fb->ptr-fb->buf);
    AppendElementLEX(lex,LEXS_SET1,fb->buf,fb->ptr-fb->buf,true);

    ResetFastBuf(fb);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanLEXElement_CANN
(
    lex_t		* lex,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(lex);
    DASSERT(si);
    TRACE("ScanLEXElement_CANN()\n");


    //--- setup data

    ScanParam_t ptab[] =
    {
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));
    AppendFastBuf(fb,EmptyString,4);


    //--- main loop

    uint n_cannon = 0;
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
	ScanIndexSI(si,&err,n_cannon,0,2);
	if (err)
	    break;

	float f[4];
	ScanFloatSI(si,f,4);

	uint i;
	u32 *fp = (u32*)f;
	for ( i = 0; i < 4; i++, fp++ )
	    *fp = htonl(*fp);
	AppendFastBuf(fb,f,sizeof(f));
	n_cannon++;
	CheckEolSI(si);
    }

    if ( n_cannon > 0 )
    {
	write_be32(fb->buf,n_cannon);
	//HexDump16(stdout,0,0,fb->buf,fb->ptr-fb->buf);
	AppendElementLEX(lex,LEXS_CANN,fb->buf,fb->ptr-fb->buf,true);
    }

    ResetFastBuf(fb);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanLEXElement_HIPT
(
    lex_t		* lex,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(lex);
    DASSERT(si);
    TRACE("ScanLEXElement_HIPT()\n");


    //--- setup data

    ScanParam_t ptab[] =
    {
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));


    //--- main loop

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

	u8 num[sizeof(lex_hipt_rule_t)];
	ScanU8SI(si,num,sizeof(num),false);
	AppendFastBuf(fb,num,sizeof(num));
	CheckEolSI(si);
    }

    AppendElementLEX(lex,LEXS_HIPT,fb->buf,fb->ptr-fb->buf,true);
    ResetFastBuf(fb);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanLEXElement_RITP
(
    lex_t		* lex,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(lex);
    DASSERT(si);
    TRACE("ScanLEXElement_HIPT()\n");


    //--- setup data

    ScanParam_t ptab[] =
    {
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));


    //--- main loop

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

	u8 num[sizeof(lex_ritp_rule_t)] = {0};
	ScanU8SI(si,num,sizeof(num)-1,false);
	if (NextCharSI(si,false))
	    ScanU8SI(si,num+sizeof(num)-1,1,false);
	AppendFastBuf(fb,num,sizeof(num));
	CheckEolSI(si);
    }

    AppendElementLEX(lex,LEXS_RITP,fb->buf,fb->ptr-fb->buf,true);
    ResetFastBuf(fb);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
// [[new-lex-sect]]
///////////////////////////////////////////////////////////////////////////////

static enumError ScanLEXElement_TEST
(
    lex_t		* lex,		// KMP data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(lex);
    DASSERT(si);
    TRACE("ScanLEXElement_TEST()\n");


    //--- setup data

    lex_test_t test;
    memcpy(&test,TestDataLEX,sizeof(test));

    const ScanParam_t ptab[] =
    {
	{ "OFFLINE-ONLINE",	SPM_U8,	&test.offline_online },
	{ "N-OFFLINE",		SPM_U8,	&test.n_offline },
	{ "N-ONLINE",		SPM_U8,	&test.n_online },
	{ "COND-BIT",		SPM_S8,	&test.cond_bit },
	{ "GAME-MODE",		SPM_U8,	&test.game_mode },
	{ "ENGINE",		SPM_U8,	&test.engine },
	{ "RANDOM",		SPM_U8,	&test.random },
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));
    AppendFastBuf(fb,&test,sizeof(test));


    //--- main loop

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	    ScanParamSI(si,ptab);
	else
	    ScanHexlineSI(si,fb,true);
    }
    CheckLevelSI(si);

    memcpy(fb->buf,&test,sizeof(test));
    HEXDUMP16(0,0,fb->buf,fb->ptr-fb->buf);
    AppendElementLEX(lex,LEXS_TEST,fb->buf,fb->ptr-fb->buf,true);

    ResetFastBuf(fb);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    PRINT("ScanTextLEX(init=%d)\n",init_lex);

    DASSERT(lex);
    if (init_lex)
	InitializeLEX(lex);

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,lex->fname,lex->revision);
    si.predef = SetupVarsLEX();

    enumError max_err = ERR_OK;
    lex->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN LEX SECTIONS, PASS%u ...\n",lex->is_pass2+1);

	max_err = ERR_OK;
	si.lex = lex;
	si.no_warn = !lex->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", lex->is_pass2+1 );

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
	    ResetLocalVarsSI(&si,lex->revision);

	    si.cur_file->ptr++;
	    char sect_name[20];
	    ScanNameSI(&si,sect_name,sizeof(sect_name),true,true,0);
	    PRINT0("--> pass=%u: #%04u: %s\n",lex->is_pass2+1,si.cur_file->line,sect_name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,sect_name,lex_section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    PRINT0("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
	     case LID_IGNORE:
		// ignore it without warning
		break;

	     case LID_SETUP:
		// nothing to do at the moment
		break;

	     case LID_HEXDUMP:
		err = ScanLEXElementHexdump(lex,&si);
		break;

	     case LID_SETTINGS1:
		err = ScanLEXElement_SET1(lex,&si);
		break;

	     case LID_CANNON:
		err = ScanLEXElement_CANN(lex,&si);
		break;

	     case LID_HIDE_PT:
		err = ScanLEXElement_HIPT(lex,&si);
		break;

	     case LID_RND_ITPH:
		err = ScanLEXElement_RITP(lex,&si);
		break;

	     // [[new-lex-sect]]

	     case LID_TEST:
		err = ScanLEXElement_TEST(lex,&si);
		break;

	     default:
		err = ERROR0(ERR_WARNING,"Unknown section (ignored): %s\n",sect_name);
		break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (lex->is_pass2)
	    break;

	lex->is_pass2 = true;
	RestartSI(&si);
    }

    ResetSI(&si);

    UpdateLEX(lex,opt_complete,false);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(lex);
    PRINT("ScanLEX() size=%u\n",data_size);
    HEXDUMP16(0,0,data,16);

    enumError err;
// [[analyse-magic]]
    switch (GetByMagicFF(data,data_size,data_size))
    {
	case FF_LEX:
	    lex->fform = FF_LEX;
	    err = ScanRawLEX(lex,init_lex,data,data_size,0);
	    break;

	case FF_LEX_TXT:
	    lex->fform = FF_LEX_TXT;
	    err = ScanTextLEX(lex,init_lex,data,data_size);
	    break;

	default:
	    if (init_lex)
		InitializeLEX(lex);
	    return ERROR0(ERR_INVALID_DATA,
		"No LEX file: %s\n", lex->fname ? lex->fname : "?");
    }

    PatchLEX(lex,0);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    struct raw_data_t	* raw		// valid raw data
)
{
    DASSERT(lex);
    DASSERT(raw);
    if (init_lex)
	InitializeLEX(lex);
    else
	ResetLEX(lex);

    lex->fatt  = raw->fatt;
    lex->fname = raw->fname;
    raw->fname = 0;

    return raw->is_0 ? ERR_OK : ScanLEX(lex,false,raw->data,raw->data_size);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadLEX
(
    lex_t		* lex,		// LEX data structure
    bool		init_lex,	// true: initialize 'lex' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(lex);
    DASSERT(fname);
    if (init_lex)
	InitializeLEX(lex);
    else
	ResetLEX(lex);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	lex->fname = raw.fname;
	raw.fname = 0;
	err = ScanLEX(lex,false,raw.data,raw.data_size);
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateRawLEX()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateRawLEX
(
    lex_t		* lex		// pointer to valid LEX
)
{
    DASSERT(lex);
    if (lex->raw_data)
	FREE(lex->raw_data);

    uint total_size = sizeof(lex_header_t) + sizeof(lex_element_t); // final term!

    uint i;
    for ( i = 0; i < lex->item_used; i++ )
    {
	lex_item_t *s = lex->item[i];
	total_size += sizeof(lex_element_t) + ntohl(s->elem.size);
    }

    lex->raw_data = MALLOC(total_size);
    lex->raw_data_size = total_size;

    lex_header_t *hd = (lex_header_t*)lex->raw_data;
    memcpy(hd->magic,LEX_BIN_MAGIC,sizeof(hd->magic));
    hd->major_version	= htons(LEX_MAJOR_VERSION);
    hd->minor_version	= htons(LEX_MINOR_VERSION);
    hd->size		= htonl(total_size);
    hd->element_off	= htonl(sizeof(lex_header_t));

    u8 *dest = lex->raw_data + sizeof(lex_header_t);
    for ( i = 0; i < lex->item_used; i++ )
    {
	lex_item_t *s = lex->item[i];
	const uint size = sizeof(lex_element_t) + ntohl(s->elem.size);
	ASSERT ( dest + size < lex->raw_data + lex->raw_data_size );
	memcpy(dest,&s->elem,size);
	dest += size;
    }
    ASSERT ( dest + sizeof(lex_element_t) == lex->raw_data + lex->raw_data_size );
    memset(dest,0,sizeof(lex_element_t));

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    SaveRawLEX()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawLEX
(
    lex_t		* lex,		// pointer to valid LEX
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(lex);
    DASSERT(fname);
    PRINT("SaveRawLEX(%s,%d)\n",fname,set_time);

    //--- create raw data

    enumError err = CreateRawLEX(lex);
    if (err)
	return err;
    DASSERT(lex->raw_data);
    DASSERT(lex->raw_data_size);


    //--- write to file

    File_t F;
    err = CreateFileOpt(&F,true,fname,testmode,lex->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&lex->fatt,0);

    if ( fwrite(lex->raw_data,1,lex->raw_data_size,F.f) != lex->raw_data_size )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  SaveTextLEX()			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_FEAT
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    const uint size = ntohl(item->elem.size);
    noPRINT("SaveTextLEX_FEAT(), size=%u\n",size);
    DASSERT( size >= sizeof(lex_set1_t) );
    if ( size < sizeof(features_szs_t))
	return ERR_OK;

    fprintf(f,text_lex_elem_feat_cr,item->sort_order);
    const features_szs_t *fs = (features_szs_t*)item->elem.data;

    PrintScript_t ps;
    InitializePrintScript(&ps);
    ps.f		= f;
    ps.fform		= PSFF_ASSIGN;
    ps.force_case	= LOUP_UPPER;
    int comments	= -1;

    if ( !brief_count && !export_count )
    {
	comments = 1;
	ps.ena_empty	= true;
	ps.ena_comments	= true;
	ps.boc		= "#";
    }
    PrintFeaturesSZS(&ps,fs,true,comments,1,0,0,0);
    ResetPrintScript(&ps);

    uint data_size = ntohs(fs->size_be);
    if ( data_size > size )
	 data_size = size;

    if ( data_size > sizeof(features_szs_t) )
    {
	u8 size = data_size - sizeof(features_szs_t);
	u8 *ptr = item->elem.data + sizeof(features_szs_t);
	if ( size >= 4 || !IsMemConst(ptr,size,0) )
	{
	    fputs("\r\n# Hex dump for unknown settings:\r\n",f);
	    HexDump(f,0,sizeof(features_szs_t),5,16,ptr,size);
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_SET1
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    const uint size = ntohl(item->elem.size);
    noPRINT("SaveTextLEX_SET1(), size=%u\n",size);
    DASSERT( size >= sizeof(lex_set1_t) );
    if ( size < sizeof(lex_set1_t))
	return ERR_OK;

    const u8 *data = item->elem.data;
    const lex_set1_t *set1 = (lex_set1_t*)data;

// [[time-limit]]
    const unsigned apply_online_sec = ntohs(set1->apply_online_sec);
    if ( brief_count || export_count )
	fprintf(f,
		"%s[SET1]\r\n\r\n"
		"@ITEM-POS-FACTOR  = v( %5.3f, %5.3f, %5.3f )\r\n"
		"@START-ITEM       = %u\r\n"
		"@APPLY-ONLINE-SEC = %u%s\r\n"
		"\r\n"
		,section_sep
		,bef4(&set1->item_factor.x)
		,bef4(&set1->item_factor.y)
		,bef4(&set1->item_factor.z)
		,set1->start_item
		,apply_online_sec ,PrintHMS(0,0,apply_online_sec,LE_VIEW_ONLINE_HMS,"  # ",0)
		// [[new-lex-set1]]
		);
    else
	fprintf(f,text_lex_elem_set1_cr
		,item->sort_order
		,bef4(&set1->item_factor.x)
		,bef4(&set1->item_factor.y)
		,bef4(&set1->item_factor.z)
		,set1->start_item
		,LE_MIN_ONLINE_SEC ,PrintHMS(0,0,LE_MIN_ONLINE_SEC,LE_VIEW_ONLINE_HMS," (",")")
		,LE_MAX_ONLINE_SEC ,PrintHMS(0,0,LE_MAX_ONLINE_SEC,LE_VIEW_ONLINE_HMS," (",")")
		,apply_online_sec ,PrintHMS(0,0,apply_online_sec,LE_VIEW_ONLINE_HMS,"  # ",0)
		// [[new-lex-set1]]
		);

    if ( set1->padding_0d )
	fprintf(f,text_lex_elem_set1_develop_cr
		,set1->padding_0d
		);

    if ( size > sizeof(lex_set1_t) )
    {
	fputs("\r\n# Hex dump for unknown settings:\r\n",f);
	HexDump(f,0,sizeof(lex_set1_t),5,16,
		item->elem.data + sizeof(lex_set1_t),
		size - sizeof(lex_set1_t) );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_CANN
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    fprintf(f,text_lex_elem_cann_cr,item->sort_order);
    if ( item->elem.size < 4 + 16 )
	return ERR_OK;

    const u8 *data = item->elem.data;
    uint n = be32(data);
    uint max = ( ntohl(item->elem.size) - 4 ) / 16;
    if ( n > max )
	 n = max;

    uint i, off = 4;
    for ( i = 0; i < n; i++, off += 16 )
	fprintf(f,"%4u %10.3f %10.3f %10.3f %10.3f\r\n",
		i, bef4(data+off), bef4(data+off+4),
		bef4(data+off+8), bef4(data+off+12) );

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_HIPT
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    fprintf(f,text_lex_elem_hipt_cr,item->sort_order);
    if ( item->elem.size < 4 + 16 )
	return ERR_OK;

    const lex_hipt_rule_t *rule = (lex_hipt_rule_t*)item->elem.data;
    int n = ntohl(item->elem.size) / sizeof(*rule) ;

    int last_lap = 1000;
    for ( int i = 0; i < n; i++, rule++ )
    {
	if ( last_lap != rule->lap )
	{
	    last_lap = rule->lap;
	    fputs("\r\n",f);
	}

	fprintf(f,"%6u %5d %4u %4u %4u\r\n",
		rule->cond, rule->lap, rule->from, rule->to, rule->mode );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_RITP
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    if ( brief_count || export_count )
	fprintf(f,"%s[RITP]\r\n\r\n",section_sep);
    else
	fprintf(f,text_lex_elem_ritp_cr,item->sort_order);

    const lex_ritp_rule_t *rule = (lex_ritp_rule_t*)item->elem.data;
    int n = ntohl(item->elem.size) / sizeof(*rule) ;
    if ( n <= 0 )
	return ERR_OK;

    fputs(text_lex_elem_ritp_tabhead_cr,f);


    struct mode_t
    {
	u8   print_param;   // 0:never, 1:if not null, 2:always
	char name[7];
    };
    
    static const struct mode_t mode_tab[] =
    {
	{ 1, "OFF" },
	{ 0, "START" },
	// [[new-ritp-mode]]
    };
    _Static_assert( sizeof(mode_tab)/sizeof(*mode_tab) == RITP_MODE__N, "RITP_MODE__N" );

    for ( int i = 0; i < n; i++, rule++ )
    {
	bool print_param;
	char name[20];
	if ( rule->mode < RITP_MODE__N )
	{
	    const struct mode_t *md = mode_tab + rule->mode;
	    print_param = md->print_param;
	    snprintf(name,sizeof(name),"RITP$%s",md->name);
	}
	else
	{
	    print_param = 2;
	    snprintf(name,sizeof(name),"%4u",rule->mode);
	}

	if ( print_param >= 2 || print_param == 1 && rule->param )
	    fprintf(f,"%6u %3u  %-9s %6u\r\n",
		rule->index, rule->n_next, name, rule->param );
	else
	    fprintf(f,"%6u %3u  %s\r\n",
		rule->index, rule->n_next, name );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_TEST
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    const uint size = ntohl(item->elem.size);
    PRINT("SaveTextLEX_TEST(), size=%u\n",size);
    DASSERT( size >= sizeof(lex_test_t) );
    if ( size < sizeof(lex_test_t))
	return ERR_OK;

    const u8 *data = item->elem.data;
    const lex_test_t *test = (lex_test_t*)data;

    const KeywordTab_t *off_on;
    for ( off_on = lex_offline_online; off_on->name1; off_on++ )
	if ( off_on->id == test->offline_online && off_on->opt >= 0 )
	    break;

    const KeywordTab_t *gmode;
    for ( gmode = lex_game_mode; gmode->name1; gmode++ )
	if ( gmode->id == test->game_mode && gmode->opt >= 0 )
	    break;

    const KeywordTab_t *engine;
    for ( engine = lex_engine; engine->name1; engine++ )
	if ( engine->id == test->engine && engine->opt >= 0 )
	    break;

    fprintf(f,text_lex_elem_test_cr
		,item->sort_order
		,off_on->name1 ? off_on->name1 : "AUTO"
		,test->n_offline
		,test->n_online
		,test->cond_bit
		,gmode->name1 ? gmode->name1 : "AUTO"
		,engine->name1 ? engine->name1 : "AUTO"
		,test->random
		);

    if ( size > sizeof(lex_test_t) )
    {
	fputs("\r\n# Hex dump for unknown settings:\r\n",f);
	HexDump(f,0,sizeof(lex_test_t),5,16,
		item->elem.data + sizeof(lex_test_t),
		size - sizeof(lex_test_t) );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextLEX_HEXDUMP
(
    FILE		* f,		// file to write
    const lex_t		* lex,		// pointer to valid LEX
    lex_item_t		* item		// item to save
)
{
    DASSERT(f);
    DASSERT(lex);
    DASSERT(item);

    const bool brief = brief_count || export_count;
    const u32 magic = ntohl(item->elem.magic);
    if (brief)
	fprintf(f,"%s[HEXDUMP]\r\n@MAGIC = 0x%08x\r\n\r\n",section_sep,magic);
    else
	fprintf(f,text_lex_hexdump_cr,
		item->sort_order, magic, PrintID(&item->elem.magic,4,0) );

    HexDumpCRLF(f,0,0,5,16,item->elem.data,ntohl(item->elem.size));
    if (!brief)
	fputs(text_lex_hexdump_sep_cr,f);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveTextLEX
(
    const lex_t		* lex,		// pointer to valid LEX
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(lex);
    DASSERT(fname);
    PRINT("SaveTextLEX(%s,%d)\n",fname,set_time);


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,lex->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&lex->fatt,0);


    //--- print header + syntax info

    if ( print_header && !brief_count && !export_count )
	fprintf(F.f,text_lex_head_cr);
    else
	fprintf(F.f,
		"%s\r\n"
		"# LEX syntax & semantics: https://szs.wiimm.de/info/lex-syntax.html\r\n"
		,LEX_TEXT_MAGIC);


    //--- print info section & section counter alphabetically

    if ( brief_count || export_count )
    {
	fprintf(F.f,
		"%s[SETUP]\r\n\r\n"
		"TOOL     = %s\r\n"
		"SYSTEM   = %s\r\n"
		"VERSION  = %s\r\n"
		"REVISION = %u\r\n"
		"DATE     = %s\r\n"
		"\r\n",
		section_sep,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );
    }
    else
    {
	fprintf(F.f, text_lex_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );

	uint i;
	for ( i = 0; i < lex->item_used; i++ )
	{
	    lex_item_t *s = lex->item[i];
	    const uint size = ntohl(s->elem.size) + 8;
	    fprintf(F.f,"#   %-4.4s %08x, 0x%04x bytes\r\n",
		    PrintID(&s->elem.magic,4,0), s->elem.magic, size );
	}
    }

    enumError max_err = ERR_OK;

    uint i;
    for ( i = 0; i < lex->item_used; i++ )
    {
	lex_item_t *s = lex->item[i];
	const u32 magic = ntohl(s->elem.magic);
	enumError err = ERR_OK;
	switch(magic)
	{
	    case LEXS_FEAT: err = SaveTextLEX_FEAT(F.f,lex,s); break;
	    case LEXS_SET1: err = SaveTextLEX_SET1(F.f,lex,s); break;
	    case LEXS_CANN: err = SaveTextLEX_CANN(F.f,lex,s); break;
	    case LEXS_HIPT: err = SaveTextLEX_HIPT(F.f,lex,s); break;
	    case LEXS_RITP: err = SaveTextLEX_RITP(F.f,lex,s); break;
	    case LEXS_TEST: err = SaveTextLEX_TEST(F.f,lex,s); break;
	    // [[new-lex-sect]]

	    default:	    err = SaveTextLEX_HEXDUMP(F.f,lex,s); break;
	}

	if ( max_err < err )
	     max_err = err;
    }

    fputs(section_end,F.f);
    ResetFile(&F,set_time);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX options			///////////////
///////////////////////////////////////////////////////////////////////////////

bool	opt_lex_features	= false;
bool	opt_lex_rm_features	= false;
bool	opt_lex_purge		= false;
bool	force_lex_test		= false;
bool	opt_lt_clear		= false;
int	opt_lt_online		= -9;
int	opt_lt_n_offline	= -9;
int	opt_lt_n_online		= -9;
int	opt_lt_cond_bit		= -9;
int	opt_lt_game_mode	= -9;
int	opt_lt_engine		= -9;
int	opt_lt_random		= -9;

//-----------------------------------------------------------------------------

int ScanOptLtOnline ( ccp arg )
{
    if ( !arg || !*arg )
	opt_lt_online = -9;
    else
    {
	int abbrev;
	const KeywordTab_t *key = ScanKeyword(&abbrev,arg,lex_offline_online);
	if (!key)
	{
	    PrintKeywordError(lex_offline_online,arg,abbrev,0,
		    "keyword for option --lt-online");
	    return 1;
	}
	opt_lt_online = key->id;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptLtNPlayers ( ccp arg )
{
    if ( !arg || !*arg )
	opt_lt_n_offline = opt_lt_n_online = -9;
    else
    {
	ccp comma = strchr(arg,',');
	if (comma)
	{
	    opt_lt_n_offline = str2l(arg,0,10);
	    opt_lt_n_online  = str2l(comma+1,0,10);
	}
	else
	{
	    int n = str2l(arg,0,10);
	    if ( n > 4 )
	    {
		opt_lt_n_offline = -9;
		opt_lt_n_online  = n;
	    }
	    else
	    {
		opt_lt_n_offline = n;
		opt_lt_n_online  = -9;
	    }
	}

	if ( opt_lt_n_offline < 0 )
	    opt_lt_n_offline = -9;
	else if ( opt_lt_n_offline > 4 )
	    opt_lt_n_offline = 4;

	if ( opt_lt_n_online < 0 )
	    opt_lt_n_online = -9;
	else if ( opt_lt_n_online > 99 )
	    opt_lt_n_online = 99;
    }

    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptLtCondBit ( ccp arg )
{
    if ( !arg || !*arg )
	opt_lt_cond_bit = -9;
    else
    {
	int n = str2l(arg,0,10);
	opt_lt_cond_bit = n < -1 || n >= 16 ? -9 : n;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptLtGameMode ( ccp arg )
{
    if ( !arg || !*arg )
	opt_lt_game_mode = -1;
    else
    {
	int abbrev;
	const KeywordTab_t *key = ScanKeyword(&abbrev,arg,lex_game_mode);
	if (!key)
	{
	    PrintKeywordError(lex_game_mode,arg,abbrev,0,
		    "keyword for option --lt-game-mode");
	    return 1;
	}
	opt_lt_game_mode = key->id;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptLtEngine ( ccp arg )
{
    if ( !arg || !*arg )
	opt_lt_engine = -1;
    else
    {
	int abbrev;
	const KeywordTab_t *key = ScanKeyword(&abbrev,arg,lex_engine);
	if (!key)
	{
	    PrintKeywordError(lex_engine,arg,abbrev,0,
		    "keyword for option --lt-engine");
	    return 1;
	}
	opt_lt_engine = key->id;
    }
    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptLtRandom ( ccp arg )
{
    if ( !arg || !*arg )
	opt_lt_random = -1;
    else
    {
	int n = str2l(arg,0,10);
	opt_lt_random = n < 0 || n > 6 ? -9 : n;
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			lex_info_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLexInfo ( lex_info_t *info )
{
    DASSERT(info);
    memset(info,0,sizeof(*info));
    info->item_factor.x = 1.0;
    info->item_factor.y = 1.0;
    info->item_factor.z = 1.0;
}

///////////////////////////////////////////////////////////////////////////////

void SetupLexInfo ( lex_info_t *info, const lex_t *lex )
{
    DASSERT(info);
    InitializeLexInfo(info);

    if (lex)
    {
	info->lex_found = true;
	info->have_sect = lex->have_sect;
	info->have_feat = lex->have_feat;

	lex_item_t *item;
	item = FindElementLEX(lex,LEXS_SET1);
	if (item)
	{
	    info->set1_found = true;
	    memcpy( &info->set1, item->elem.data, sizeof(info->set1) );
	    // copy from info to info is correct, because of memcpy() above
	    bef4n( info->set1.item_factor.v, info->set1.item_factor.v, 3 );
	    memcpy( &info->item_factor, &info->set1.item_factor, sizeof(info->item_factor) );
	}

	item = FindElementLEX(lex,LEXS_TEST);
	if (item)
	{
	    info->test_found = true;
	    memcpy(&info->test,item->elem.data,sizeof(info->test));
	}

	// [[new-lex-sect]]
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError SetupLexInfoByData ( lex_info_t *info, cvp data, uint data_size )
{
    lex_t lex;
    const enumError err = ScanLEX(&lex,true,data,data_size);
    if (!err)
	SetupLexInfo(info,&lex);
    else
	InitializeLexInfo(info);
    ResetLEX(&lex);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX patching			///////////////
///////////////////////////////////////////////////////////////////////////////

// return TRUE if any option set
bool HavePatchTestLEX()
{
    return force_lex_test
	|| opt_lt_clear
	|| opt_lt_online >= 0
	|| opt_lt_n_offline >= 0
	|| opt_lt_n_online >= 0
	|| opt_lt_game_mode >= 0
	|| opt_lt_engine >= 0
	|| opt_lt_random >= 0;
}


//-----------------------------------------------------------------------------

// return TRUE if any option set and not default
bool HaveActivePatchTestLEX()
{
    return force_lex_test
	|| opt_lt_online > 0
	|| opt_lt_n_offline > 0
	|| opt_lt_n_online > 0
	|| opt_lt_game_mode > LEX_GMODE_AUTO
	|| opt_lt_engine > LEX_ENGINE_AUTO
	|| opt_lt_random >= 0;
}

//-----------------------------------------------------------------------------

// return TRUE if modified / set optional 'empty' to true, if test section is empty
bool PatchTestLEX ( lex_test_t *lt, bool *empty )
{
    DASSERT(lt);
    PRINT0("--lt-*: clear=%d, online=%d, n=%d,%d, gm=%d, en=%d, rnd=%d\n",
	opt_lt_clear, opt_lt_online, opt_lt_n_offline, opt_lt_n_online,
	opt_lt_game_mode, opt_lt_engine, opt_lt_random );

    lex_test_t temp;
    memcpy( &temp, opt_lt_clear ? (lex_test_t*)&TestDataLEX : lt, sizeof(temp) );

    if ( opt_lt_online >= LEX_OO_AUTO && opt_lt_online <= LEX_OO_ONLINE )
	temp.offline_online = opt_lt_online;

    if ( opt_lt_n_offline >= 0 )
	temp.n_offline = opt_lt_n_offline > 4 ? 4 : opt_lt_n_offline;

    if ( opt_lt_n_online >= 0 )
	temp.n_online = opt_lt_n_online > 99 ? 99 : opt_lt_n_online;

    if ( opt_lt_cond_bit >= -1 && opt_lt_cond_bit < 16 )
	temp.cond_bit = opt_lt_cond_bit;

    if ( opt_lt_game_mode >= 0 )
	temp.game_mode = opt_lt_game_mode;

    if ( opt_lt_engine >= 0 )
	temp.engine = opt_lt_engine;

    if ( opt_lt_random >= 0 )
	temp.random = opt_lt_random;

    if (empty)
	*empty = !memcmp(&temp,&TestDataLEX,sizeof(temp));
    if (memcmp(lt,&temp,sizeof(*lt)))
    {
	memcpy(lt,&temp,sizeof(*lt));
	return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// return TRUE if modified / set optional 'empty' to true, if test section is empty

bool PatchFeaturesLEX ( lex_t * lex, const szs_have_t *have, bool *empty )
{
    DASSERT(lex);
    DASSERT(have);
    DASSERT(have->valid);

    features_szs_t fs;
    SetupFeaturesSZS(&fs,have,true);
    const bool need_features = GetFeaturesStatusSZS(&fs) > 1;

    lex_item_t *li = FindElementLEX(lex,LEXS_FEAT);
    const bool have_features = li != 0;

    PRINT("--lex-features = %d, --lex-rm-features = %d : have=%d, need=%d\n",
		opt_lex_features, opt_lex_rm_features, have_features, need_features );

    bool modified = false;
    if ( opt_lex_rm_features && RemoveElementByMagicLEX(lex,LEXS_FEAT) )
	modified = true; // FEAT

    if ( have_features && !need_features )
    {
	if (RemoveElementLEX(lex,li))
	    modified = true;
    }
    else if (  opt_lex_features
	    && need_features
	    && ( !have_features || memcmp(&fs,li->elem.data,sizeof(fs)) ))
    {
	AppendFeatLEX(lex,true,&fs);
	modified = true;
    }

    ResetFeaturesSZS(&fs);

    if (empty)
	*empty = !need_features;
    if (modified)
	lex->modified = true;
    return modified;
}

//-----------------------------------------------------------------------------

bool PatchLEX ( lex_t * lex, const szs_have_t *have )
{
    DASSERT(lex);

    bool modified = false;

    //--- section TEST

    if (HavePatchTestLEX())
    {
	lex_item_t *li = FindElementLEX(lex,LEXS_TEST);
	const bool had_test = li != 0;
	if (!had_test)
	{
	    if (!HaveActivePatchTestLEX())
		return 0;
	    li = AppendTestLEX(lex,false);
	}
	DASSERT(li);
	lex_test_t orig;
	memcpy(&orig,li->elem.data,sizeof(orig));

	bool empty;
	modified = PatchTestLEX((lex_test_t*)li->elem.data,&empty);
	PRINT("PATCH+ LEX/TEST: had=%d, empty=%d, modified=%d => have_lex:%x,%x\n",
		had_test, empty, modified, lex->have_sect, lex->have_feat );
	if ( empty && !force_lex_test )
	{
	    if ( RemoveElementLEX(lex,li) && had_test )
		modified = true;
	}
	else if (!had_test)
	    modified = true;

	PRINT("PATCH- LEX/TEST: had=%d, empty=%d, modified=%d => have_lex:%x,%x\n",
		had_test, empty, modified, lex->have_sect, lex->have_feat );

    }

    // [[new-lex-sect]]

    //--- section FEAT

    if ( HavePatchFeaturesLEX() && have && have->valid )
    {
	szs_have_t temp = *have;
	temp.lex_sect = lex->have_sect;
	temp.lex_feat = lex->have_feat;

	if (PatchFeaturesLEX(lex,&temp,0))
	    modified = true;
    }


    //--- terminate

    if (opt_lex_purge)
	modified |= PurgeLEX(lex);

    if (modified)
	lex->modified = true;

    return modified;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LEX purging			///////////////
///////////////////////////////////////////////////////////////////////////////

bool PurgeLEX ( lex_t * lex )
{
    DASSERT(lex);

    bool modified = false;

    uint i = 0;
    while ( i < lex->item_used )
    {
	lex_item_t **ss = lex->item+i;
	const uint size = ntohl((*ss)->elem.size);
	const u8  *data = (*ss)->elem.data;

	bool remove = false;
	switch(ntohl((*ss)->elem.magic))
	{
	 case LEXS_FEAT:
	    remove = GetFeaturesStatusSZS((features_szs_t*)data) < 2;
	    break;

	 case LEXS_SET1:
	    remove = !IsActiveLEXElement_SET1((lex_set1_t*)data);
	    break;

	 case LEXS_CANN:
	    remove = size <= sizeof(CannonDataLEX)
		  && !memcmp(data,&CannonDataLEX,size);
	    break;

	 case LEXS_HIPT:
	    remove = size < sizeof(lex_hipt_rule_t);
	    break;

	 case LEXS_RITP:
	    remove = size < sizeof(lex_ritp_rule_t);
	    break;

        // [[new-lex-sect]]

	 case LEXS_TEST:
	    remove = !IsActiveLEXElement_TEST((lex_test_t*)data);
	    break;

	 default:
	    remove = true;
	    break;
	}

	if (remove)
	    modified |= RemoveElementLEX(lex,*ss);
	else
	    i++;
    }

    if (modified)
	UpdateHaveLex(lex);

    return modified;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


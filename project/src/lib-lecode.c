
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

#include "dclib-xdump.h"
#include "lib-ledis.h"
#include "lib-std.h"
#include "lib-mkw.h"
#include "lib-szs.h"
#include "lib-bzip2.h"
#include "lpar.inc"
#include "lecode.inc"

#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    vars			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp	opt_le_arena		= 0;
ccp	opt_lpar		= 0;
ccp	opt_track_dest		= 0;
ParamField_t opt_track_source	= {0};
int	opt_szs_mode		= TFMD_LINK;

ccp	opt_le_alias		= 0;
bool	opt_engine_valid	= false;
u8	opt_engine[3]		= {0};
OffOn_t	opt_200cc		= OFFON_AUTO;
OffOn_t	opt_perfmon		= OFFON_AUTO;
OffOn_t	opt_custom_tt		= OFFON_AUTO;
OffOn_t	opt_xpflags		= OFFON_AUTO;
int	opt_speedo		= OFFON_AUTO;
int	opt_lecode_debug	= OFFON_AUTO;

bool	opt_complete		= false;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetLecodeSupportWarning ( const le_analyze_t *ana )
{
    DASSERT(ana);
    return !( ana->valid & LE_HEAD_VALID )
	? "LE-CODE file header is invalid"
	: GetEncodedVersion() < ana->szs_required
	? PrintCircBuf("SZS Tools v%s or younger required",DecodeVersion(ana->szs_required))
	: IsLecodeSupported(ana->valid)
	? 0
	: PrintCircBuf("LE-CODE v%u is not supported",ana->header_vers);
}

///////////////////////////////////////////////////////////////////////////////

uint GetNextRacingTrackLE ( uint tid )
{
    if ( ++tid == 0xff )
	tid++;

    if ( tid > MKW_N_TRACKS && tid < LE_FIRST_CT_SLOT )
	tid = LE_FIRST_CT_SLOT;
    return tid;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptTrackSource ( ccp arg, int arg_len, int mode )
{
    static ccp last_key = ""; // for optimization if scanning path/*.szs

    if (arg)
    {
	if ( arg_len < 0 )
	    arg_len = strlen(arg);
	if ( arg_len > 0 )
	{
	    char path[PATH_MAX];
	    StringCopySM(path,sizeof(path),arg,arg_len);
	    if ( path[arg_len-1] == '/' )
		path[--arg_len] = 0;

	    if	(  arg_len > 0
		&& strcmp(last_key,path)
		&& !FindParamField(&opt_track_source,path)
		)
	    {
		PRINT("APPEND TRACK_SOURCE: 0x%03x %s\n",mode,path);
		if (!opt_track_source.field)
		    InitializeParamField(&opt_track_source);
		ParamFieldItem_t *it
		    = AppendParamField(&opt_track_source,path,false,mode,0);
		if (it)
		    last_key = it->key;
	    }
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptSzsMode ( ccp arg )
{
    const KeywordTab_t keytab[] =
    {
     { 0,		"OFF",		0,	0 },
     { TFMD_COPY,	"COPY",		"CP",	0 },
     { TFMD_MOVE,	"MOVE",		"MV",	0 },
     { TFMD_MOVE1,	"MOVE1",	"MV1",	0 },
     { TFMD_LINK,	"LINK",		"LN",	0 },
     { 0,0,0,0 }
    };

    int abbrev_count;
    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,arg,keytab);
    if (!cmd)
	return 1;

    opt_szs_mode = cmd->id;
    PRINT("SZS-MODE=%d\n",opt_szs_mode);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOpt200cc ( ccp arg )
{
    const int stat
	= ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_ON,"Option --200cc");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_200cc = stat;
    PRINT("200CC=%d\n",opt_200cc);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPerfMon ( ccp arg )
{
    const int stat
	= ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_ON,"Option --perfmon");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_perfmon = stat;
    PRINT("PERFMON=%d\n",opt_perfmon);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptCustomTT ( ccp arg )
{
    const int stat
	= ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_ON,"Option --custom-tt");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_custom_tt = stat;
    PRINT("CUSTOM-TT=%d\n",opt_custom_tt);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptXPFlags ( ccp arg )
{
    const int stat
	= ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_ON,"Option --xpflags");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_xpflags = stat;
    PRINT("XPFLAGS=%d\n",opt_xpflags);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// [[speedo_mode_t]] [[copy-lecode]] copy changes to LE-CODE

typedef enum speedo_mode_t
{
    SPEEDO_OFF		= 0,
    SPEEDO_ON,
    SPEEDO_FRACTION1,
    SPEEDO_FRACTION2,
    SPEEDO_FRACTION3,
    SPEEDO_MAX		= SPEEDO_FRACTION3,	// max allowed value
}
speedo_mode_t;

//-----------------------------------------------------------------------------

int ScanOptSpeedometer ( ccp arg )
{
    const KeywordTab_t keytab[] =
    {
     { OFFON_OFF,	"OFF",		"-1",	0 },
     { OFFON_AUTO,	"AUTO",		0,	0 },
     { SPEEDO_ON,	"ON",		0,	0 },
     { SPEEDO_ON,	"FORCE",	0,	0 }, // legacy, [[obsolete]], remove in 2022
     { SPEEDO_ON,	"0DIGITS",	"0",	0 },
     { SPEEDO_FRACTION1,"FRACTION",	0,	0 },
     { SPEEDO_FRACTION1,"1DIGITS",	"1",	0 },
     { SPEEDO_FRACTION2,"2DIGITS",	"2",	0 },
     { SPEEDO_FRACTION3,"3DIGITS",	"3",	0 },
     { 0,0,0,0 }
    };

    const int stat
	= ScanKeywordOffAutoOnEx(keytab,arg,OFFON_ON,0,"Option --speedo");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_speedo = stat;
    PRINT("SPEEDO=%d\n",opt_speedo);
    return 0;
}

//-----------------------------------------------------------------------------

ccp GetLecodeSpeedoName ( speedo_mode_t mode )
{
    switch (mode)
    {
	case SPEEDO_OFF:	break;
	case SPEEDO_ON:		return "0";
	case SPEEDO_FRACTION1:	return "1";
	case SPEEDO_FRACTION2:	return "2";
	case SPEEDO_FRACTION3:	return "3";
	// no default! => compiler warnings for missed values
    }
    return "OFF";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetLecodeDebugName ( le_debug_mode_t mode )
{
    switch (mode)
    {
	case DEBUGMD_OFF:
	case DEBUGMD__N:	break;
	case DEBUGMD_ENABLED:	return "ENABLED";
	case DEBUGMD_DEBUG1:
	case DEBUGMD_DEBUG2:
	case DEBUGMD_DEBUG3:
	case DEBUGMD_DEBUG4:	return PrintCircBuf("%u",mode-DEBUGMD_DEBUG1+1);
	// no default! => compiler warnings for missed values
    }
    return "OFF";
}

//-----------------------------------------------------------------------------

ccp GetCheatModeInfo ( uint cheat_mode )
{
    switch (cheat_mode)
    {
	case 0:  return "disabled";
	case 1:  return "debug only";
	default: return "all";
    }
}

//-----------------------------------------------------------------------------

ccp GetLecodeDebugInfo ( le_debug_mode_t mode )
{
    switch (mode)
    {
	case DEBUGMD_OFF:	return "disabled";
	case DEBUGMD_ENABLED:	return "enabled but hidden";
	case DEBUGMD_DEBUG1:
	case DEBUGMD_DEBUG2:
	case DEBUGMD_DEBUG3:
	case DEBUGMD_DEBUG4:	return PrintCircBuf("view DEBUG-%u",mode-DEBUGMD_DEBUG1+1);
	case DEBUGMD__N:	break;
	// no default! => compiler warnings for missed values
    }
    return "?";
}

//-----------------------------------------------------------------------------

int ScanOptDebug ( ccp arg )
{
    const KeywordTab_t keytab[] =
    {
      { OFFON_OFF,	"OFF",		"-1",	0 },
      { OFFON_AUTO,	"AUTO",		"0",	0 },
      { DEBUGMD_ENABLED,"USER",		"1",	0 },
      { DEBUGMD_DEBUG1,	"DEBUG1",	"D1",	0 },
      { DEBUGMD_DEBUG2,	"DEBUG2",	"D2",	0 },
      { DEBUGMD_DEBUG3,	"DEBUG3",	"D3",	0 },
      { DEBUGMD_DEBUG4,	"DEBUG4",	"D4",	0 },
      { 0,0,0,0 }
    };

    const int stat
	= ScanKeywordOffAutoOnEx(keytab,arg,OFFON_ON,DEBUGMD__N-1,"Option --debug");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_lecode_debug = stat;
    PRINT("LE-DEBUG=%d (%s)\n",opt_lecode_debug,GetLecodeDebugInfo(opt_lecode_debug));
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
#if 0

 int ScanOptReserved_1bb ( ccp arg )
 {
    opt_reserved_1bb = str2l(arg,0,10);
    PRINT("RESERVED_1BB=%d\n",opt_reserved_1bb);
    return 0;
 }

#endif
///////////////////////////////////////////////////////////////////////////////

int ScanOptAlias ( ccp arg )
{
    if ( arg && *arg )
    {
	if (opt_le_alias)
	{
	    const uint len1 = strlen(opt_le_alias);
	    const uint len2 = strlen(arg);
	    char *list = MALLOC(len1+len2+2);
	    memcpy(list,opt_le_alias,len1);
	    list[len1] = ',';
	    memcpy(list+len1+1,arg,len2+1);
	    FreeString(opt_le_alias);
	    opt_le_alias = list;
	}
	else
	    opt_le_alias = STRDUP(arg);
    }
    PRINT0("OPT ALIAS: %s|\n",opt_le_alias);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptEngine ( ccp arg )
{
    if ( !arg || !*arg )
    {
	opt_engine_valid = false;
	return 0;
    }

    const uint MAX = sizeof(opt_engine)/sizeof(*opt_engine);
    double val[MAX], sum = 0.0;
    ccp src = arg;
    uint n = 0;

    for(;;)
    {
	if ( n > MAX )
	    break;
	char *end;
	double d = strtod(src,&end);
	if ( !isnormal(d) || d <= 0.0 )
	    d = 0.0;
	val[n++] = d;
	sum += d;

	src = end;
	while (isspace((int)*src))
	    src++;
	if ( *src != ',' )
	    break;
	src++;
    }

    if ( n != 3 )
	return ERROR0(ERR_SYNTAX,
		"Invalid argument for option --engine, »a,b,c« expected: %s",arg);

    if  ( sum > 0.0 )
    {
	sum = 100/sum;
	uint total = 100;
	for ( n = 0; n < MAX-1; n++ )
	{
	    uint num = double2int( val[n] * sum );
	    if ( num > total )
		num = total;
	    opt_engine[n] = num;
	    total -= num;
	}
	opt_engine[MAX-1] += total;
    }
    else
	memset(opt_engine,0,sizeof(opt_engine));
    opt_engine_valid = true;

    PRINT("ENGINE: %g,%g,%g => %u + %u + %u = %u [200cc=%d])\n",
		val[0], val[1], val[2],
		opt_engine[0], opt_engine[1], opt_engine[2],
		opt_engine[0] + opt_engine[1] + opt_engine[2],
		opt_200cc );
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    setup			///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsLECODE()
{
    static VarMap_t vm = { .force_case = LOUP_UPPER };
    if (!vm.used)
    {
	DefineMkwVars(&vm);
	DefineParserVars(&vm);


	//--- setup integer variables

	struct inttab_t { ccp name; int val; };
	static const struct inttab_t inttab[] =
	{
	    { "DEBUG$OFF",		PREDEBUG_OFF },
	    { "DEBUG$CLEAR",		PREDEBUG_CLEAR },
	    { "DEBUG$STANDARD",		PREDEBUG_STANDARD },
	    { "DEBUG$OPPONENT",		PREDEBUG_OPPONENT },
	    { "DEBUG$VERTICAL",		PREDEBUG_VERTICAL },

	    { "DEBUG$ENABLED",		DEBUGMD_ENABLED },
	    { "DEBUG$0",		DEBUGMD_ENABLED },
	    { "DEBUG$1",		DEBUGMD_DEBUG1 },
	    { "DEBUG$2",		DEBUGMD_DEBUG2 },
	    { "DEBUG$3",		DEBUGMD_DEBUG3 },
	    { "DEBUG$4",		DEBUGMD_DEBUG4 },

	    { "SPEEDO$OFF",		SPEEDO_OFF },
	    { "SPEEDO$0",		SPEEDO_ON },
	    { "SPEEDO$1",		SPEEDO_FRACTION1 },
	    { "SPEEDO$2",		SPEEDO_FRACTION2 },
	    { "SPEEDO$3",		SPEEDO_FRACTION3 },

	    { "BT",			LTTY_ARENA },
	    { "VS",			LTTY_TRACK },
	    { "FILL",			-1 },

	    { "LE$STRING_LIST_ENABLED",	LE_STRING_LIST_ENABLED },
	    { "LE$STRING_SET_ENABLED",	LE_STRING_SET_ENABLED },
	    { "LE$TYPE_MARKERS_ENABLED",LE_TYPE_MARKERS_ENABLED },

	    {0,0}
	};

	const struct inttab_t * ip;
	for ( ip = inttab; ip->name; ip++ )
	    DefineIntVar(&vm,ip->name,ip->val);

	char name[20];
	ccp prefix = GetNameLTTY(LTTY_ARENA);
	for ( int slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
	{
	    snprintf(name,sizeof(name),"%s%u",prefix,slot);
	    StringUpperS(name,sizeof(name),name);
	    DefineIntVar(&vm,name,slot);
	}

	prefix = GetNameLTTY(LTTY_TRACK);
	for ( int slot = MKW_TRACK_BEG; slot < MKW_TRACK_END; slot++ )
	{
	    snprintf(name,sizeof(name),"%s%u",prefix,slot);
	    StringUpperS(name,sizeof(name),name);
	    DefineIntVar(&vm,name,slot);
	}

	prefix = GetNameLTTY(LTTY_RANDOM|LTTY_TRACK);
	for ( int slot = MKW_LE_RANDOM_BEG; slot < MKW_LE_RANDOM_END; slot++ )
	{
	    snprintf(name,sizeof(name),"%s%u",prefix,slot);
	    StringUpperS(name,sizeof(name),name);
	    DefineIntVar(&vm,name,slot);
	}
    }
    return &vm;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LE-CODE Debug			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetDebugModeSummary ( const lecode_debug_info_t *ldi )
{
    DASSERT(ldi);
    return PrintCircBuf("I=%d,%d, M:%02x,%02x, %s=%u",
	ldi->index, ldi->filter_active, ldi->mode, ldi->relevant,
	ldi->name ? ldi->name : "-", ldi->value );
}

///////////////////////////////////////////////////////////////////////////////

bool GetFirstDebugMode
	( lecode_debug_info_t *ldi, lecode_debug_t mode, bool filter_active )
{
    DASSERT(ldi);
    memset(ldi,0,sizeof(*ldi));
    ldi->mode		= mode & LEDEB__ALL;
    ldi->filter_active	= filter_active;

    return GetNextDebugMode(ldi);
}

///////////////////////////////////////////////////////////////////////////////

bool GetNextDebugMode ( lecode_debug_info_t *ldi )
{
    struct tab_t
    {
	lecode_debug_t	mode;
	int		shift;
	const char	name[12];
    };

    static const struct tab_t tab[] =
    {
	{ LEDEB_ENABLED,	0,			"ENABLED"	},
	{ LEDEB_OPPONENT,	0,			"OPPONENT"	},
	{ LEDEB_SPACE,		0,			"SPACE"		},
	{ LEDEB_POSITION,	0,			"POSITION"	},
	{ LEDEB_CHECK_POINT,	LEDEB_S_CHECK_POINT,	"CHECK-POINT"	},
	{ LEDEB_RESPAWN,	0,			"RESPAWN"	},
	{ LEDEB_ITEM_POINT,	0,			"ITEM-POINT"	},
	{ LEDEB_KCL_TYPE,	0,			"KCL-TYPE"	},
	{ LEDEB_LAP_POS,	0,			"LAP-POS"	},
	{ LEDEB_TRACK_ID,	0,			"TRACK-ID"	},
	{ LEDEB_XPF,		LEDEB_S_XPF,		"XPF"		},
	// [[new-debug]]
    };

    while ( ldi->index >= 0 && ldi->index < sizeof(tab)/sizeof(*tab) )
    {
	const struct tab_t *ptr = tab + ldi->index++;
	if ( ldi->filter_active && !(ptr->mode&ldi->mode) )
	    continue;

	ldi->relevant	= ptr->mode;
	ldi->name	= ptr->name;
	ldi->value	= ldi->mode & ptr->mode;
	if (ptr->shift)
	    ldi->value >>= ptr->shift;
	else
	    ldi->value = ldi->value != 0;
	return true;
    }

    ldi->index		= -1;
    ldi->relevant	= 0;
    ldi->name		= 0;
    ldi->value		= 0;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

lecode_debug_t DecodeLecodeDebug ( lecode_debug_ex_t *lde, lecode_debug_t mode )
{
    DASSERT(lde);
    memset(lde,0,sizeof(*lde));

    mode		&= LEDEB__ALL;
    lde->mode		= mode;
    lde->enabled	= ( mode & LEDEB_ENABLED ) > 0;
    lde->opponent	= ( mode & LEDEB_OPPONENT ) > 0;
    lde->space		= ( mode & LEDEB_SPACE ) > 0;
    lde->position	= ( mode & LEDEB_POSITION ) > 0;
    lde->check_point	= mode >> LEDEB_S_CHECK_POINT & LEDEB_M_CHECK_POINT;
    lde->respawn	= ( mode & LEDEB_RESPAWN ) > 0;
    lde->item_point	= ( mode & LEDEB_ITEM_POINT ) > 0;
    lde->kcl_type	= ( mode & LEDEB_KCL_TYPE ) > 0;
    lde->lap_pos	= ( mode & LEDEB_LAP_POS ) > 0;
    lde->track_id	= ( mode & LEDEB_TRACK_ID ) > 0;
    lde->xpf		= mode >> LEDEB_S_XPF & LEDEB_M_XPF;
    // [[new-debug]]

    lde->have_output	= isDebugModeOutput(mode);
    lde->is_active	= isDebugModeActive(mode);

    return mode;
}

///////////////////////////////////////////////////////////////////////////////

lecode_debug_t EncodeLecodeDebug ( lecode_debug_ex_t *lde )
{
    DASSERT(lde);

    lecode_debug_t mode = ( lde->check_point & LEDEB_M_CHECK_POINT ) << LEDEB_S_CHECK_POINT
			| ( lde->xpf & LEDEB_M_XPF ) << LEDEB_S_XPF;

    if ( lde->enabled )		mode |= LEDEB_ENABLED;
    if ( lde->opponent )	mode |= LEDEB_OPPONENT;
    if ( lde->space )		mode |= LEDEB_SPACE;
    if ( lde->position )	mode |= LEDEB_POSITION;
    if ( lde->respawn )		mode |= LEDEB_RESPAWN;
    if ( lde->item_point )	mode |= LEDEB_ITEM_POINT;
    if ( lde->kcl_type )	mode |= LEDEB_KCL_TYPE;
    if ( lde->lap_pos )		mode |= LEDEB_LAP_POS;
    if ( lde->track_id )	mode |= LEDEB_TRACK_ID;
    // [[new-debug]]

    return DecodeLecodeDebug(lde,mode); // normalize all settings
}

///////////////////////////////////////////////////////////////////////////////

ccp GetPredefDebugName ( predef_debug_t pd_mode )
{
    switch ( pd_mode )
    {
	case PREDEBUG_OFF:	break;
	case PREDEBUG_CLEAR:	return "CLEAR";
	case PREDEBUG_STANDARD:	return "STANDARD";
	case PREDEBUG_OPPONENT:	return "OPPONENT";
	case PREDEBUG_VERTICAL:	return "VERTICAL";
	// no default! => compiler warnings for missed values
    }
    return "OFF";
}

///////////////////////////////////////////////////////////////////////////////

bool SetupLecodeDebugLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    uint		config,		// configuration index 0..
    uint		setup_mode,	// setup mode, one of PREDEBUG_*
    int			*hide_speedo	// if not NULL: store new mode here
)
{
    DASSERT(lpar);
    DASSERT( config >= 0 && config < DEBUGMD__N-DEBUGMD_DEBUG1 );


    //--- setup data table

    struct setup_t
    {
	u16 predef;
	s16 line;
	u32 mode;
    };

    static const struct setup_t setup_tab[] =
    {
	{ PREDEBUG_CLEAR, 0, 0 },

	{ PREDEBUG_STANDARD, 1, LEDEB__STD|LEDEB_SHORT_XPF },

	{ PREDEBUG_OPPONENT, 1, LEDEB__STD },
	{ PREDEBUG_OPPONENT, 2, LEDEB__STD|LEDEB_OPPONENT },

	{ PREDEBUG_VERTICAL,-1, 0 }, // hide speedo
	{ PREDEBUG_VERTICAL, 1, LEDEB_CHECK_POINT },
	{ PREDEBUG_VERTICAL, 2, LEDEB_RESPAWN },
	{ PREDEBUG_VERTICAL, 3, LEDEB_ITEM_POINT },
	{ PREDEBUG_VERTICAL, 4, LEDEB_KCL_TYPE },
	{ PREDEBUG_VERTICAL, 5, LEDEB_LAP_POS },
	{ PREDEBUG_VERTICAL, 6, LEDEB_POSITION },
	{ PREDEBUG_VERTICAL, 7, LEDEB_TRACK_ID },
	{ PREDEBUG_VERTICAL, 8, LEDEB_LONG_XPF },
	// [[new-debug]]

	{0,0,0}
    };

    const struct setup_t *ptr;
    for ( ptr = setup_tab; ptr->predef; ptr++ )
    {
	if ( ptr->predef == setup_mode )
	{
	    lpar->debug_predef[config] = setup_mode;
	    int my_hide_speedo = 0;

	    u32 *debug_list = lpar->debug[config];
	    for ( int line = 0; line < LEDEB__N_LINE; line++ )
		debug_list[line] = LEDEB_ENABLED;

	    for ( ; ptr->predef == setup_mode; ptr++ )
	    {
		if ( ptr->line < 0 )
		    my_hide_speedo++;
		else
		    debug_list[ptr->line] = ptr->mode | LEDEB_ENABLED;
	    }
	    if (hide_speedo)
		*hide_speedo = my_hide_speedo;

	    const u8 hide_mask = 1 << config;
	    if ( my_hide_speedo > 0 )
		lpar->no_speedo_if_debug |= hide_mask;
	    else
		lpar->no_speedo_if_debug &= ~hide_mask;

	    return true;
	}
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool SetupLecodeDebugIfEmptyLPAR ( le_lpar_t *lpar )
{
    DASSERT(lpar);

    for ( int config = 0; config < LEDEB__N_CONFIG; config++ )
	for ( int line = 0; line < LEDEB__N_LINE; line++ )
	    if ( isDebugModeOutput(lpar->debug[config][line]) )
		return false;

    for ( int config = 0; config < LEDEB__N_CONFIG; config++ )
	for ( int line = 0; line < LEDEB__N_LINE; line++ )
	    lpar->debug[config][line] = LEDEB_ENABLED;

    SetupLecodeDebugLPAR(lpar,0,PREDEBUG_STANDARD,0);
    SetupLecodeDebugLPAR(lpar,1,PREDEBUG_OPPONENT,0);
    SetupLecodeDebugLPAR(lpar,2,PREDEBUG_VERTICAL,0);

    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			enum le_region_t		///////////////
///////////////////////////////////////////////////////////////////////////////

le_region_t GetLEREG ( char ch )
{
    switch (ch)
    {
      case 'p': case 'P': return LEREG_PAL;
      case 'u': case 'U': return LEREG_USA;
      case 'j': case 'J': return LEREG_JAP;
      case 'k': case 'K': return LEREG_KOR;
    }
    return LEREG_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetNameLEREG ( le_region_t lereg, ccp return_on_invalid )
{
    switch (lereg)
    {
	case LEREG_PAL: return "PAL";
	case LEREG_USA: return "USA";
	case LEREG_JAP: return "JAP";
	case LEREG_KOR:	return "KOR";
	default:	return return_on_invalid;
    }
}

///////////////////////////////////////////////////////////////////////////////

// replace last '@' by region name.
// returns either NULL on error or 'name' or 'buf'. If !buf or to small, then use circ-buf.

ccp PatchNameLEREG ( char *buf, uint bufsize, ccp name, le_region_t lereg )
{
    if (!name)
	return 0;

    ccp at  = strrchr(name,'@');
    ccp reg = GetNameLEREG(lereg,0);
    if ( !at || !reg )
	return name;

    const uint pos = at - name;
    const uint need = strlen(name)+ 3; // includes term-0

    if ( ( !buf || !bufsize ) && need <= CIRC_BUF_MAX_ALLOC )
    {
	buf = GetCircBuf(need);
	bufsize = need;
    }

    if ( buf && bufsize >= need )
    {
	memcpy(buf,name,pos);
	memcpy(buf+pos,reg,3);
	strcpy(buf+pos+3,at+1);
	return buf;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const mem_t GetLecodeLEREG ( le_region_t reg )
{
    BZ2Manager_t *lm;
    switch (reg)
    {
	case LEREG_PAL:  lm = &lecode_pal_bin_mgr; break;
	case LEREG_USA:	 lm = &lecode_usa_bin_mgr; break;
	case LEREG_JAP:	 lm = &lecode_jap_bin_mgr; break;
	case LEREG_KOR:  lm = &lecode_kor_bin_mgr; break;
	default:	 return NullMem;
    }

    DecodeBZIP2Manager(lm);
    mem_t res = { .ptr = (ccp)lm->data, .len = lm->size };
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetLecodeBuildLEREG ( le_region_t reg )
{
    return GetBuildLECODE(GetLecodeLEREG(reg));
}

//-----------------------------------------------------------------------------

ccp GetBuildLECODE ( mem_t lecode )
{
    if ( !lecode.ptr || lecode.len < sizeof(le_binary_head_v4_t) )
	return "?";

    const le_binary_head_v4_t *head = (le_binary_head_v4_t*)lecode.ptr;
    return ntohl(head->version) < 4
	? PrintCircBuf("%u",ntohl(head->build_number))
	: PrintCircBuf("%u (%.10s)",ntohl(head->build_number),head->timestamp);
}

///////////////////////////////////////////////////////////////////////////////

ccp GetLecodeInfoLEREG ( le_region_t reg )
{
    return GetInfoLECODE(GetLecodeLEREG(reg));
}

//-----------------------------------------------------------------------------

ccp GetInfoLECODE ( mem_t lecode )
{
    if ( !lecode.ptr || lecode.len < sizeof(le_binary_head_v4_t) )
	return "?";

    const le_binary_head_v4_t *head = (le_binary_head_v4_t*)lecode.ptr;
    ccp region	= head->region == 'P' ? "PAL"
		: head->region == 'E' ? "USA"
		: head->region == 'J' ? "JAP"
		: head->region == 'K' ? "KOR"
		: "?";
    ccp debug	= head->debug == 'D' ? "/debug" : "";

    return ntohl(head->version) < 4
	? PrintCircBuf("%s%s v%u, build %u, %u bytes",
			region, debug,
			ntohl(head->version),
			ntohl(head->build_number),
			ntohl(head->file_size) )
	: PrintCircBuf("%s%s v%u, build %u (%s UTC), %u bytes",
			region, debug,
			ntohl(head->version),
			ntohl(head->build_number),
			head->timestamp,
			ntohl(head->file_size) );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LPAR support			///////////////
///////////////////////////////////////////////////////////////////////////////

lpar_mode_t CalcCurrentLparMode ( const le_lpar_t * lp, bool use_limit_param )
{
    DASSERT(lp);

    if ( use_limit_param && (uint)lp->limit_mode < LPM_AUTOMATIC )
    {
	const lpar_mode_t temp = CalcCurrentLparMode(lp,false);
	return temp > lp->limit_mode ? temp : lp->limit_mode;
    }

    if ( lp->enable_perfmon > 1  )
	return LPM_EXPERIMENTAL;

    // [[new-lpar]]

    return lp->enable_perfmon
	|| !lp->enable_xpflags
	|| lp->enable_speedo > SPEEDO_FRACTION1
	|| lp->cheat_mode > 1
		? LPM_TESTING : LPM_PRODUCTIVE;
}

//-----------------------------------------------------------------------------

ccp GetLparModeName ( lpar_mode_t lpmode, bool export )
{
    if (export)
	return PrintCircBuf("%u # %s",lpmode,GetLparModeName(lpmode,false));

    switch (lpmode)
    {
	case LPM_PRODUCTIVE:	return "LE$PRODUCTIVE";
	case LPM_TESTING:	return "LE$TESTING";
	case LPM_EXPERIMENTAL:	return "LE$EXPERIMENTAL";
	default:		return "LE$AUTOMATIC";
    }
}

//-----------------------------------------------------------------------------

bool LimitToLparMode ( le_lpar_t * lp, lpar_mode_t lpmode )
{
    DASSERT(lp);

    bool modified = false;

    if ( lpmode == LPM_PRODUCTIVE )
    {
	if ( lp->enable_perfmon > 0 )
	    { lp->enable_perfmon = 0; modified = true; }
	if (!lp->enable_xpflags)
	    { lp->enable_xpflags = 1; modified = true; }
	if ( lp->enable_speedo > SPEEDO_MAX )
	    { lp->enable_speedo = SPEEDO_MAX; modified = true; }
	if ( lp->cheat_mode > 1 )
	    { lp->cheat_mode = 1; modified = true; }
    }
    else if ( lpmode == LPM_TESTING )
    {
	if ( lp->enable_perfmon > 1 )
	    { lp->enable_perfmon  = 1; modified = true; }
    }

    // [[new-lpar]]

    return modified;
}

//-----------------------------------------------------------------------------

void ClearLparChat ( le_lpar_t * lp )
{
    DASSERT(lp);
    memset(lp->chat_mode_1,0,sizeof(lp->chat_mode_1));
    memset(lp->chat_mode_2,0,sizeof(lp->chat_mode_2));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define RACE0 (CHATMD_N_RACE_1-1)

const u16 chat_mode_legacy[BMG_N_CHAT] =
{
    0,0,0,0, 0,0,0,0,			//  1 ..  8
    0,0,0,0, 0,0,0,0,			//  9 .. 16
    0,0,0,0, 0,0,0,0,			// 17 .. 24
    0,0,0,0, 0,0,0,0,			// 25 .. 32
    0,0,0,0, 0,0,0,0,			// 33 .. 40
    0,0,0,0, 0,0,0,0,			// 41 .. 48
    0,0,0,0, 0,0,0,0,			// 49 .. 56
    0,0,0,0, 0,0,0,0,			// 57 .. 64

    CHATMD_VEHICLE_BEG+CHATVEH_ANY,	// 65
    0,0,0,				// 66 .. 68
    CHATMD_VEHICLE_BEG+CHATVEH_KART,	// 69
    CHATMD_VEHICLE_BEG+CHATVEH_BIKE,	// 70
    CHATMD_TRACK_BY_HOST,		// 71
    CHATMD_ANY_TRACK,			// 72

    RACE0 +  1,				// 73
    RACE0 +  2,				// 74
    RACE0 +  3,				// 75
    RACE0 +  4,				// 76
    RACE0 +  5,				// 77
    RACE0 +  6,				// 78
    RACE0 +  8,				// 79
    RACE0 + 10,				// 80

    0,0,0,0, 0,0,0,0,			// 81 .. 88
    0,0,0,0, 0,0,0,0,			// 89 .. 96
};

//-----------------------------------------------------------------------------

const u16 chat_mode_mkwfun[BMG_N_CHAT] =
{
    0,0,0,0, 0,0,0,0,		//  1 ..  8
    0,0,0,0, 0,0,0,0,		//  9 .. 16
    0,0,0,0, 0,0,0,0,		// 17 .. 24
    0,0,0,0, 0,0,0,0,		// 25 .. 32
    0,0,0,0, 0,0,0,0,		// 33 .. 40
    0,0,0,0, 0,0,0,0,		// 41 .. 48
    0,0,0,0, 0,0,0,0,		// 49 .. 56
    0,0,0,0,			// 57 .. 60

    CHATMD_RESET_ENGINE,	// 61
    CHATMD_USE_ENGINE_1,	// 62
    CHATMD_USE_ENGINE_2,	// 63
    CHATMD_USE_ENGINE_3,	// 64

    CHATMD_ANY_VEHICLE,		// 65
    CHATMD_KARTS_ONLY,		// 66
    CHATMD_BIKES_ONLY,		// 67
    0,				// 68

    CHATMD_ANY_TRACK,		// 69
    CHATMD_TRACK_BY_HOST,	// 70
    0,				// 71
    0,				// 72

    RACE0 +  1,			// 73
    RACE0 +  2,			// 74
    RACE0 +  3,			// 75
    RACE0 +  4,			// 76
    RACE0 +  5,			// 77
    RACE0 +  6,			// 78
    RACE0 +  8,			// 79
    RACE0 + 10,			// 80

    0,0,0,0, 0,0,0,0,		// 81 .. 88
    0,0,0,0, 0,0,0,0,		// 89 .. 96
};

///////////////////////////////////////////////////////////////////////////////

ccp GetChatModeName ( u16 mode )
{
    char buf[100];

    switch (mode)
    {
     case CHATMD_OFF:		return "CHAT$OFF";

     case CHATMD_RESET:		return "CHAT$RESET";

     case CHATMD_ANY_TRACK:	return "CHAT$ANY_TRACK";
     case CHATMD_TRACK_BY_HOST:	return "CHAT$TRACK_BY_HOST";

     case CHATMD_BLOCK_CLEAR:	return "CHAT$BLOCK_CLEAR";
     case CHATMD_BLOCK_DISABLE:	return "CHAT$BLOCK_DISABLE";
     case CHATMD_BLOCK_ENABLE:	return "CHAT$BLOCK_ENABLE";

     case CHATMD_ANY_VEHICLE:	return "CHAT$ANY_VEHICLE";
     case CHATMD_KARTS_ONLY:	return "CHAT$KARTS_ONLY";
     case CHATMD_BIKES_ONLY:	return "CHAT$BIKES_ONLY";

     case CHATMD_RESET_ENGINE:	return "CHAT$RESET_ENGINE";
     case CHATMD_USE_ENGINE_1:	return "CHAT$USE_ENGINE_1";
     case CHATMD_USE_ENGINE_2:	return "CHAT$USE_ENGINE_2";
     case CHATMD_USE_ENGINE_3:	return "CHAT$USE_ENGINE_3";
    }


    //--- vehicles

    if ( mode >= CHATMD_VEHICLE_BEG && mode <= CHATMD_VEHICLE_END )
    {
	mode -= CHATMD_VEHICLE_BEG;

	struct mode_t { u8 mode; char name[9]; };
	static const struct mode_t tab[] =
	{
//	    { CHATVEH_ANY,	"ANY" },

	    { CHATVEH_ANY_SIZE,	"ANY_SIZE" },
	    { CHATVEH_SMALL,	"SMALL" },
	    { CHATVEH_MEDIUM,	"MEDIUM" },
	    { CHATVEH_LARGE,	"LARGE" },

	    { CHATVEH_ANY_TYPE,	"ANY_TYPE" },
	    { CHATVEH_KART,	"KART" },
	    { CHATVEH_BIKE,	"BIKE" },
	    { CHATVEH_OUT_BIKE,	"OUT_BIKE" },
	    { CHATVEH_IN_BIKE,	"IN_BIKE" },

	    { 0, "" },
	};

	char *dest = buf;
	dest[1] = 0;
	const struct mode_t *ptr;
	for ( ptr = tab; ptr->mode && mode; ptr++ )
	    if ( (ptr->mode & mode) == ptr->mode )
	    {
		mode &= ~ptr->mode;
		dest = StringCat2E(dest,buf+sizeof(buf),",VEH$",ptr->name);
	    }
	return PrintCircBuf("chat$vehicles(%s)",buf+1);
    }


    //--- n races

    if ( mode >= CHATMD_N_RACE_1 && mode <= CHATMD_N_RACE_MAX )
	return PrintCircBuf("chat$n_races(%u)",mode-(CHATMD_N_RACE_1-1));


    //--- fallback

    return PrintCircBuf("#%x",mode);
}

///////////////////////////////////////////////////////////////////////////////

void InitializeLPAR ( le_lpar_t *lpar, bool load_lpar )
{
    DASSERT(lpar);
    memset(lpar,0,sizeof(*lpar));

    lpar->limit_mode		= LPM__DEFAULT;
    lpar->engine[0]		=  10;
    lpar->engine[1]		=  60;
    lpar->engine[2]		=  30;
    lpar->enable_xpflags	=   1;
    lpar->drag_blue_shell	=   1;
    lpar->thcloud_frames	= 300;
    lpar->block_textures	=   1;

    // [[new-lpar]]

    if ( load_lpar && opt_lpar )
	LoadLPAR(lpar,false,0,false);
}

///////////////////////////////////////////////////////////////////////////////

static void NormalizeLPAR ( le_lpar_t * lp )
{
    DASSERT(lp);

    SetupLecodeDebugIfEmptyLPAR(lp);

    int sum = lp->engine[0] + lp->engine[1] + lp->engine[2];
    if ( sum != 0 && sum != 100 )
    {
	double factor = 100.0/sum;
	lp->engine[0] = double2int( lp->engine[0] * factor );
	lp->engine[1] = double2int( lp->engine[1] * factor );
	sum = lp->engine[0] + lp->engine[1];
	if ( sum > 100 )
	    lp->engine[1] -= sum-100;
	lp->engine[2] = 100 - lp->engine[0] - lp->engine[1];
	PRINT0("NORM ENGINE: %u + %u + %u = %u [200cc=%d])\n",
		lp->engine[0], lp->engine[1], lp->engine[2],
		lp->engine[0] + lp->engine[1] + lp->engine[2],
		lp->enable_200cc );
    }

    if ( lp->thcloud_frames < 1 || lp->thcloud_frames > 0x7fff )
	lp->thcloud_frames = 300;

    lp->cheat_mode		= lp->cheat_mode < 2 ? lp->cheat_mode : 2;
    lp->enable_200cc		= lp->enable_200cc > 0;
    lp->enable_perfmon		= lp->enable_perfmon < 2 ? lp->enable_perfmon : 2;
    lp->enable_custom_tt	= lp->enable_custom_tt > 0;
    lp->enable_xpflags		= lp->enable_xpflags > 0;
    lp->enable_speedo		= lp->enable_speedo < SPEEDO_MAX ? lp->enable_speedo : SPEEDO_MAX;
    lp->block_track		= lp->block_track < LE_MAX_BLOCK_TRACK
				? lp->block_track : LE_MAX_BLOCK_TRACK;
    lp->drag_blue_shell		= lp->drag_blue_shell > 0;
    lp->bt_worldwide		= lp->bt_worldwide > 0;
    lp->vs_worldwide		= lp->vs_worldwide > 0;
    lp->bt_textures		= lp->bt_textures & LE_M_TEXTURE;
    lp->vs_textures		= lp->vs_textures & LE_M_TEXTURE;
    lp->block_textures		= lp->block_textures > 0;

    // [[new-lpar]]

 #if 0
    for ( int c = 0; c < LEDEB__N_CONFIG; c++ )
	for ( int l = 0; l < LEDEB__N_LINE; l++ )
	    lp->debug[c][l] &= LEDEB__ALL;
 #endif

}


///////////////////////////////////////////////////////////////////////////////

static void CopyLPAR2Data ( le_analyze_t * ana )
{
    DASSERT(ana);

    ccp warn = GetLecodeSupportWarning(ana);
    if (warn)
    {
	ERROR0(ERR_INVALID_DATA,"Can't patch LE-CODE because %s.",warn);
	return;
    }

    NormalizeLPAR(&ana->lpar);
    const le_lpar_t *lp = &ana->lpar;
    le_binpar_v1_t *h = ana->param_v1;

    if ( offsetof(le_binpar_v1_t,engine) < ana->param_size )
	memcpy(h->engine,lp->engine,sizeof(h->engine));

    if ( offsetof(le_binpar_v1_t,enable_200cc) < ana->param_size )
	h->enable_200cc = lp->enable_200cc;

    if ( offsetof(le_binpar_v1_t,enable_perfmon) < ana->param_size )
	h->enable_perfmon = lp->enable_perfmon;

    if ( offsetof(le_binpar_v1_t,enable_custom_tt) < ana->param_size )
	h->enable_custom_tt = lp->enable_custom_tt;

    if ( offsetof(le_binpar_v1_t,enable_xpflags) < ana->param_size )
	h->enable_xpflags = lp->enable_xpflags;

    if ( offsetof(le_binpar_v1_t,enable_speedo) < ana->param_size )
	h->enable_speedo = lp->enable_speedo;

    if ( offsetof(le_binpar_v1_t,enable_speedo) < ana->param_size )
	h->no_speedo_if_debug = lp->no_speedo_if_debug;

    if ( offsetof(le_binpar_v1_t,debug_mode) < ana->param_size )
	h->debug_mode = lp->debug_mode;

    if ( offsetof(le_binpar_v1_t,item_cheat) < ana->param_size )
	h->item_cheat = lp->item_cheat;

    if ( offsetof(le_binpar_v1_t,debug_predef[LEDEB__N_CONFIG-1]) < ana->param_size )
	memcpy( h->debug_predef,lp->debug_predef,sizeof(h->debug_predef));

    if ( offsetof(le_binpar_v1_t,debug[LEDEB__N_CONFIG-1][LEDEB__N_LINE-1]) < ana->param_size )
	write_be32n( h->debug[0], lp->debug[0], LEDEB__N_CONFIG*LEDEB__N_LINE );

    if ( offsetof(le_binpar_v1_t,block_track) < ana->param_size )
	h->block_track = lp->block_track;

    if ( offsetof(le_binpar_v1_t,chat_mode_1) + sizeof(h->chat_mode_1) <= ana->param_size )
	write_be16n(h->chat_mode_1,lp->chat_mode_1,BMG_N_CHAT);

    if ( offsetof(le_binpar_v1_t,chat_mode_2) + sizeof(h->chat_mode_2) <= ana->param_size )
	write_be16n(h->chat_mode_2,lp->chat_mode_2,BMG_N_CHAT);

    if ( offsetof(le_binpar_v1_t,cheat_mode) < ana->param_size )
	h->cheat_mode = lp->cheat_mode;

    if ( offsetof(le_binpar_v1_t,drag_blue_shell) < ana->param_size )
	h->drag_blue_shell = lp->drag_blue_shell;

    if ( offsetof(le_binpar_v1_t,thcloud_frames) < ana->param_size )
	h->thcloud_frames = htons(lp->thcloud_frames);

    if ( offsetof(le_binpar_v1_t,bt_worldwide) < ana->param_size )
	h->bt_worldwide = lp->bt_worldwide;
    if ( offsetof(le_binpar_v1_t,vs_worldwide) < ana->param_size )
	h->vs_worldwide = lp->vs_worldwide;

    if ( offsetof(le_binpar_v1_t,bt_textures) < ana->param_size )
	h->bt_textures = lp->bt_textures;
    if ( offsetof(le_binpar_v1_t,vs_textures) < ana->param_size )
	h->vs_textures = lp->vs_textures;
    if ( offsetof(le_binpar_v1_t,block_textures) < ana->param_size )
	h->block_textures = lp->block_textures;

    // [[new-lpar]]
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(lpar);
    DASSERT(fname);
    PRINT("SaveTextLPAR(%s,%d)\n",fname,set_time);

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;

    err = SaveTextFileLPAR(lpar,F.f,true);

    ResetFile(&F,set_time);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveTextFileLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    FILE		*f,		// open file
    bool		print_full	// true: print ehader + append section [END]
)
{
    DASSERT(lpar);
    DASSERT(f);


    //--- print header + syntax info

    if (print_full)
    {
	if ( print_header && !brief_count && !export_count )
	    fprintf(f,text_lpar_head_cr);
	else
	    fprintf(f,"%s\r\n",LE_LPAR_MAGIC);
    }


    //--- print section

    SetupLecodeDebugIfEmptyLPAR(lpar);
    enumError err = WriteSectionLPAR(f,lpar);


    //--- print footer

    if (print_full)
	fputs(section_end,f);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static ccp PrintChatMode ( u16 msg, u16 mode1, u16 mode2, int numeric )
{
    msg++;
    if ( mode1 && mode2 && mode1 != mode2 )
    {
	if (numeric)
	    return PrintCircBuf("M%02u = %4u, %4u # %s, %s",
			msg, mode1, mode2,
			GetChatModeName(mode1), GetChatModeName(mode2) );
	return PrintCircBuf("M%02u = %s, %s",
			msg, GetChatModeName(mode1), GetChatModeName(mode2) );
    }
    else if ( mode1 || mode2 )
    {
	if (!mode1)
	    mode1 = mode2;
	if (numeric)
	    return PrintCircBuf("M%02u = %4u # %s",
			msg, mode1, GetChatModeName(mode1) );
	return PrintCircBuf("M%02u = %s", msg, GetChatModeName(mode1) );
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static ccp lpar_std_flags ( uint flags )
{
    return PrintCircBuf("LE$%sABLE%s%s%s",
		flags & LE_ENABLED	? "EN"			: "DIS",
		flags & LE_ALTERABLE	? " | LE$ALTERABLE"	: "",
		flags & LE_EXCLUDED	? " | LE$EXCLUDE"	: "",
		flags & LE_INCLUDED	? " | LE$INCLUDE"	: "" );
}

//-----------------------------------------------------------------------------

enumError WriteSectionLPAR
(
    FILE		*f,		// output files
    const le_lpar_t	*lpar		// LE-CODE parameters
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(f);
    DASSERT(lpar);

    //--- general parameters

    fprintf(f,"\r\n#\f\r\n%.79s\r\n\r\n[LECODE-PARAMETERS]\r\n",Hash200);

    const bool verbose = print_header && !brief_count && !export_count;

    if (verbose)
    {
	// [[new-lpar]]
	fprintf(f,text_lpar_sect_param_cr
		,GetLparModeName(CalcCurrentLparMode(lpar,true),export_count>0)
		,lpar->cheat_mode
		,lpar->engine[0],lpar->engine[1],lpar->engine[2]
		,lpar->enable_200cc
		,lpar->enable_perfmon
		,lpar->enable_custom_tt
		,lpar->enable_xpflags
		,LE_MAX_BLOCK_TRACK,lpar->block_track
		,GetLecodeSpeedoName(lpar->enable_speedo)
		,GetLecodeDebugName(lpar->debug_mode)
		,lpar->item_cheat
		,lpar->drag_blue_shell
		,lpar->thcloud_frames,lpar->thcloud_frames/60.0
		// [[worldwide]] should be included here, see below for a temporary solution
		,lpar_std_flags( lpar->bt_textures & LE_M_TEXTURE )
		,lpar_std_flags( lpar->vs_textures & LE_M_TEXTURE )
		,lpar->block_textures
		);
    }
    else
    {
	// [[new-lpar]]
	fprintf(f,
	       "\r\n"
	       "LIMIT-MODE	= %s\r\n\r\n"
	       "CHEAT-MODE	= %d\r\n"
	       "ENGINE		= %u,%u,%u\r\n"
	       "ENABLE-200CC	= %u\r\n"
	       "PERF-MONITOR	= %u\r\n"
	       "CUSTOM-TT	= %u\r\n"
	       "XPFLAGS		= %u\r\n"
	       "BLOCK-TRACK	= %u\r\n"
	       "SPEEDOMETER	= SPEEDO$%s\r\n"
	       "DEBUG		= DEBUG$%s\r\n"
	       "ITEM-CHEAT	= %u\r\n"
	       "DRAG-BLUE-SHELL	= %u\r\n"
	       "THCLOUD-TIME	= %u # %.2fs\r\n"
	       "BT-TEXTURES	= %s\r\n"
	       "VS-TEXTURES	= %s\r\n"
	       "BLOCK-TEXTURES	= %u\r\n"
	       // [[worldwide]] should be included here, see below for a temporary solution
		,GetLparModeName(CalcCurrentLparMode(lpar,true),export_count>0)
		,lpar->cheat_mode
		,lpar->engine[0],lpar->engine[1],lpar->engine[2]
		,lpar->enable_200cc
		,lpar->enable_perfmon
		,lpar->enable_custom_tt
		,lpar->enable_xpflags
		,lpar->block_track
		,GetLecodeSpeedoName(lpar->enable_speedo)
		,GetLecodeDebugName(lpar->debug_mode)
		,lpar->item_cheat
		,lpar->drag_blue_shell
		,lpar->thcloud_frames,lpar->thcloud_frames/60.0
		,lpar_std_flags( lpar->bt_textures & LE_M_TEXTURE )
		,lpar_std_flags( lpar->vs_textures & LE_M_TEXTURE )
		,lpar->block_textures
		);

    }

    // [[worldwide]]
    if ( lpar->bt_worldwide || lpar->vs_worldwide )
	fprintf(f,
	       "BT-WORLDWIDE	= %u\r\n"
	       "VS-WORLDWIDE	= %u\r\n"
		,lpar->bt_worldwide,
		lpar->vs_worldwide
		);


    //--- chat modes

    fprintf(f,"\r\n#\f\r\n%.79s\r\n\r\n[CHAT-MESSAGE-MODES]\r\n",Hash200);

    if (verbose)
    {
	fputs(text_lpar_sect_chat_cr,f);

	int i;
	for ( i = 0; i < BMG_N_CHAT; i++ )
	    if ( lpar->chat_mode_1[i] || lpar->chat_mode_2[i] )
		break;
	if ( i == BMG_N_CHAT )
	    fputs(text_lpar_sect_chat_example_cr,f);
    }

    int last_group = -1;
    for ( int i = 0; i < BMG_N_CHAT; i++ )
    {
	const u16 mode1 = lpar->chat_mode_1[i];
	const u16 mode2 = lpar->chat_mode_2[i];
	if ( mode1 || mode2 )
	{
	    if ( last_group != i/4 )
	    {
		last_group = i/4;
		fputs("\r\n",f);
	    }

	    fprintf(f,"%s\r\n",PrintChatMode(i,mode1,mode2,export_count));
	}
    }


    //--- debug modes

    if (verbose)
	fprintf(f,text_lpar_sect_debug_docu_cr,
				LEDEB__N_LINE, LEDEB__N_LINE-1, LEDEB__N_LINE-1 );

    for ( int conf = 0; conf < LEDEB__N_CONFIG; conf++ )
    {
	fprintf(f,"\r\n#\f\r\n%.79s\r\n\r\n[DEBUG-%u]\r\n",Hash200,conf+1);
	if (verbose)
	    fputs("# See section [DEBUG-DOCU] for details.\r\n",f);

	fprintf(f,"\r\n"
		"SETUP\t\t= DEBUG$%s\r\n"
		"HIDE-SPEEDO\t= %u\r\n",
		GetPredefDebugName(lpar->debug_predef[conf]),
		( 1<<conf & lpar->no_speedo_if_debug ) != 0 );

	int done = 0;
	for ( int line = 0; line <= LEDEB__N_LINE; line++ )
	{
	    int idx;
	    lecode_debug_t mode;

	    if ( line < LEDEB__N_LINE )
	    {
		mode = lpar->debug[conf][line];
		if (!isDebugModeOutput(mode))
		    continue;
		idx = line;
	    }
	    else
	    {
		if (done)
		    break;
		mode = 0;
		idx = 1;
	    }
	    done++;

	    bool line_printed = false;
	    lecode_debug_info_t ldi;
	    for ( GetFirstDebugMode(&ldi,mode,!verbose); ldi.name; GetNextDebugMode(&ldi) )
	    {
		if (!line_printed)
		{
		    fprintf(f,"\r\nLINE\t\t= %u\r\n",idx);
		    line_printed = true;
		}
		fprintf(f,"%s%.*s= %u\r\n",
			ldi.name,
			(23-(int)strlen(ldi.name))/8, Tabs20,
			ldi.value );
	    }
	}
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    analyse			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetLEAnalyze ( le_analyze_t *ana )
{
    if (ana)
    {
	ResetLEAnalyzeUsage(ana);
	FREE(ana->flags);
	memset(ana,0,sizeof(*ana));
	InitializeLPAR(&ana->lpar,false);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetLEAnalyzeUsage ( le_analyze_t *ana )
{
    if (ana)
    {
	if (ana->usage)
	{
	    FREE(ana->usage);
	    ana->usage = 0;
	}
	ana->usage_size = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void ApplyArena ( le_analyze_t * ana, const ctcode_arena_t * ca )
{
    DASSERT(ana);
    DASSERT(ca);

    if (!ana->arena_setup)
	SetupArenasLEAnalyze(ana,false);

    if ( ana->property && ana->music && ana->flags)
    {
	for ( int a = 0; a < MKW_N_ARENAS; a++ )
	{
	    const int slot = MKW_ARENA_BEG + a;
	    if (ca->prop[a])
	    {
		const le_property_t prop = ca->prop[a];
		if (IsLecodeRandom(prop))
		{
		    for ( int idx = 0; idx < 10; idx++ )
			if ( ntohl(ana->cup_arena[idx]) == slot )
			{
			    ana->cup_arena[idx] = htonl(prop);
			    break;
			}
		}
		else
		{
		    ana->property[slot] = prop;
		    ana->arena_applied = true;
		}
	    }

	    if (ca->music[a])
	    {
		ana->music[slot] = ca->music[a];
		ana->arena_applied = true;
	    }

	    if ( ca->flags[a] & CT_ARENA_FLAGS_VALID )
	    {
		ana->flags[slot] = ca->flags[a] & LEFL__ALL;
		ana->arena_applied = true;
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetupArenasLEAnalyze ( le_analyze_t *ana, bool force )
{
    DASSERT(ana);
    if ( ana->property && ana->music )
    {
	ana->arena_setup = true;

	if ( force || !ana->property[MKW_ARENA_BEG] )
	{
	    const TrackInfo_t *ai = arena_info;
	    for ( int a = 0; a < MKW_N_ARENAS; a++, ai++ )
	    {
		ana->property[a+MKW_ARENA_BEG] = ai->track_id;
		ana->music[a+MKW_ARENA_BEG]    = ai->music_id;
	    }
	}

	if ( opt_le_arena && *opt_le_arena )
	{
	    raw_data_t raw;
	    if ( LoadRawData(&raw,true,opt_le_arena,0,false,0) == ERR_OK )
	    {
		ctcode_arena_t ca = {{0}};
		ScanTextArena(&ca,raw.fname,raw.data,raw.data_size);
		ApplyArena(ana,&ca);
	    }
	    ResetRawData(&raw);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError AnalyzeLEBinary
(
    le_analyze_t	*ana,		// NULL or destination of analysis
    const void		*data,		// data pointer
    uint		data_size	// data size
)
{
    //--- setup analyse

    le_analyze_t ana0 = {0};
    if (!ana)
    {
	ResetLEAnalyze(&ana0);
	ana = &ana0;
    }
    memset(ana,0,sizeof(*ana));

    if (   !data
	|| data_size < sizeof(le_binary_head_t)
	|| memcmp(data,LE_BINARY_MAGIC,4) )
    {
	return ERR_INVALID_DATA;
    }

    le_binary_head_t *head = (le_binary_head_t*)data;
    const uint file_size = ntohl(head->file_size);
    if ( file_size > data_size )
	return ERR_INVALID_DATA;

    ana->valid		= LE_HEAD_FOUND;
    ana->version	= head->phase;
    ana->region		= GetLEREG(head->region);
    ana->data		= data;
    ana->data_size	= data_size;
    ana->head		= head;
    ana->header_vers	= ntohl(head->version);
    ana->end_of_data	= (u8*)data + file_size;
    ana->flags_bits	= ntohl(head->build_number) < 35 ? 8 : 16;
    ana->le_random_beg	= MKW_LE_RANDOM_BEG;
    ana->le_random_end	= MKW_LE_RANDOM_END;

    uint off_param = 0;
    switch(ana->header_vers)
    {
     case 4:
	if ( ana->data_size > sizeof(le_binary_head_v4_t) )
	{
	    off_param		=  ntohl(ana->head_v4->off_param);
	    ana->header_size	=  sizeof(le_binary_head_v4_t);
	    ana->le_random_beg	= MKW_LE_RANDOM_BY_CUP;
	    ana->valid		|= LE_HEAD_VALID | LE_HEAD_KNOWN;

	    time_t tim;
	    ScanDateTime(&tim,0,ana->head_v4->timestamp,false,0);
	    ana->creation_time	= tim;
	}
	break;

     case 5:
	if ( ana->data_size > sizeof(le_binary_head_v5_34_t) )
	{
	    off_param		=  ntohl(ana->head_v5->off_param);
	    ana->szs_required	=  ntohl(ana->head_v5->szs_required);
	    ana->edit_version	=  ntohl(ana->head_v5->edit_version);
	    ana->header_size	=  ntohl(ana->head_v5->head_size);
	    ana->creation_time	=  ntohl(ana->head_v5->creation_time);
	    ana->edit_time	=  ntohl(ana->head_v5->edit_time);

	    if ( ana->header_size > offsetof(le_binary_head_v5_t,szs_recommended) )
		ana->szs_recommended	=  ntohl(ana->head_v5->szs_recommended);

	    if ( ana->edit_version != 0x32303232 ) // first 4 bytes of "2022-10-23" => invalid
		ana->valid	|= LE_HEAD_VALID | LE_HEAD_KNOWN;
	}
	break;

     default:
	if ( ana->header_vers > 5 && ana->data_size > sizeof(le_binary_head_t) )
	{
	    off_param		=  ntohl(head->off_param);
	    ana->header_size	=  ntohl(ana->head_v5->head_size);
	    ana->valid		|= LE_HEAD_VALID;
	}
	break;
    }

    if ( GetEncodedVersion() >= ana->szs_required )
	    ana->valid |= LE_HEAD_VERSION;


    //--- analyse parameters (prepare)

    typedef struct ptr_ana_t
    {
	int off_offset;
	int off_ptr;
    }
    ptr_ana_t;

    #undef DEF_TAB
    #define DEF_TAB(s,o,p) { offsetof(s,o), offsetof(le_analyze_t,p) },

    const u8 *ptr_list[8];	// for section-size analysis
    uint n_ptr = 0;		// number of used pointers


    //--- analyse parameters

    if ( off_param
	&& !(off_param&3)
	&& ana->data_size >= off_param + sizeof(le_binary_param_t)
	&& !memcmp(data+off_param,LE_PARAM_MAGIC,4) )
    {
	le_binary_param_t *param = (le_binary_param_t*)(data+off_param);
	const uint param_size = ntohl(param->size);
	if (   off_param + param_size <= data_size
	    && off_param + param_size <= ana->data_size )
	{
	    ana->valid		|= LE_PARAM_FOUND;
	    ana->param		= param;
	    ana->param_offset	= off_param;
	    ana->param_vers	= ntohl(param->version);
	    ana->param_size	= param_size;

	    const uint max_off = ana->data_size - off_param;

	    switch(ana->param_vers)
	    {
	     case 1:
		if ( param_size >= sizeof(le_binpar_v1_35_t) )
		{
		    static const ptr_ana_t tab[] =
		    {
			DEF_TAB( le_binpar_v1_35_t, off_eod,		end_of_data )
			DEF_TAB( le_binpar_v1_35_t, off_cup_par,	cup_par )
			DEF_TAB( le_binpar_v1_35_t, off_cup_track,	cup_track )
			DEF_TAB( le_binpar_v1_35_t, off_cup_arena,	cup_arena )
			DEF_TAB( le_binpar_v1_35_t, off_course_par,	course_par )
			DEF_TAB( le_binpar_v1_35_t, off_property,	property )
			DEF_TAB( le_binpar_v1_35_t, off_music,		music )
			DEF_TAB( le_binpar_v1_35_t, off_flags,		flags_bin )
			{-1,-1}
		    };

		    ana->valid |= LE_PARAM_VALID | LE_PARAM_KNOWN;
		    le_binpar_v1_35_t *p = (le_binpar_v1_35_t*)(data+off_param);

		    const ptr_ana_t *tptr;
		    u32 min = max_off, max = 0;
		    for ( tptr = tab; tptr->off_offset >= 0; tptr++ )
		    {
			const u32 offset = be32((u8*)p+tptr->off_offset);
			if ( offset && offset < max_off )
			{
			    if ( min > offset ) min = offset;
			    if ( max < offset ) max = offset;
			    u8 *ptr = (u8*)data + off_param + offset;
			    *(u8**)((u8*)ana+tptr->off_ptr) = ptr;
			    DASSERT( n_ptr < sizeof(ptr_list)/sizeof(*ptr_list) );
			    ptr_list[n_ptr++] = ptr;
			}
		    }

		    ana->beg_of_data = (u8*)p + min;
		    ana->end_of_data = (u8*)p + max;

		    memcpy(ana->lpar.engine,p->engine,sizeof(ana->lpar.engine));
		    ana->lpar.enable_200cc	= p->enable_200cc;
		    ana->lpar.enable_perfmon	= p->enable_perfmon;
		}

		if ( param_size >= sizeof(le_binpar_v1_37_t) )
		{
		    le_binpar_v1_37_t *p	= (le_binpar_v1_37_t*)(data+off_param);
		    ana->lpar.enable_custom_tt	= p->enable_custom_tt;
		    ana->lpar.enable_xpflags	= p->enable_xpflags;
		}

		if ( param_size >= sizeof(le_binpar_v1_f8_t) )
		{
		    le_binpar_v1_f8_t *p	= (le_binpar_v1_f8_t*)(data+off_param);
		    ana->lpar.block_track	= p->block_track;
		    be16n(ana->lpar.chat_mode_1,p->chat_mode,BMG_N_CHAT);
		}
		else
		    memcpy(ana->lpar.chat_mode_1,chat_mode_legacy,sizeof(ana->lpar.chat_mode_1));

		if ( param_size >= sizeof(le_binpar_v1_1b8_t) )
		{
		    le_binpar_v1_1b8_t *p	= (le_binpar_v1_1b8_t*)(data+off_param);
		    be16n(ana->lpar.chat_mode_2,p->chat_mode_2,BMG_N_CHAT);
		}

		if ( param_size >= sizeof(le_binpar_v1_1bc_t) )
		{
		    le_binpar_v1_1bc_t *p	= (le_binpar_v1_1bc_t*)(data+off_param);
		    ana->lpar.enable_speedo	= p->enable_speedo;
		    ana->lpar.no_speedo_if_debug= p->no_speedo_if_debug;
		    ana->lpar.debug_mode	= p->debug_mode;
		}

		if ( param_size >= sizeof(le_binpar_v1_260_t) )
		{
		    le_binpar_v1_260_t *p	= (le_binpar_v1_260_t*)(data+off_param);
		    memcpy(ana->lpar.debug_predef,p->debug_predef,sizeof(ana->lpar.debug_predef));
		    be32n(ana->lpar.debug[0],p->debug[0],LEDEB__N_CONFIG*LEDEB__N_LINE);
		    ana->lpar.item_cheat	= p->item_cheat;
		}

		if ( param_size >= sizeof(le_binpar_v1_264_t) )
		{
		    le_binpar_v1_264_t *p	= (le_binpar_v1_264_t*)(data+off_param);
		    ana->lpar.cheat_mode	= p->cheat_mode;
		    ana->lpar.drag_blue_shell	= p->drag_blue_shell;
		    ana->lpar.thcloud_frames	= ntohs(p->thcloud_frames);
		}
//		else
//		{
//		    ana->lpar.drag_blue_shell	=   1;
//		    ana->lpar.thcloud_frames	= 300;
//		}

		if ( param_size >= sizeof(le_binpar_v1_26c_t) )
		{
		    le_binpar_v1_26c_t *p	= (le_binpar_v1_26c_t*)(data+off_param);
		    ana->lpar.bt_worldwide	= p->bt_worldwide;
		    ana->lpar.vs_worldwide	= p->vs_worldwide;
		    ana->lpar.bt_textures	= p->bt_textures;
		    ana->lpar.vs_textures	= p->vs_textures;
		    ana->lpar.block_textures	= p->block_textures;
		}

		// [[new-lpar]]

		break;

	     default:
		// at least v1
		if ( ana->param_vers > 1 && param_size >= sizeof(le_binary_param_t) )
		{
		    ana->valid |= LE_PARAM_VALID;
		}
		break;
	    }
	}

	ana->n_cup_track = ana->cup_par    ? ntohl(ana->cup_par->n_racing_cups) : 0;
	ana->n_cup_arena = ana->cup_par    ? ntohl(ana->cup_par->n_battle_cups) : 0;
	ana->n_slot      = ana->course_par ? ntohl(ana->course_par->n_slot)     : 0;
    }


    if (ana->end_of_data)
    {
	const int size = ana->data + ana->data_size - ana->end_of_data;
	if ( size >= 0 )
	{
	    ana->bin_data = ana->end_of_data;
	    ana->bin_size = size;
	}
    }


    //--- analyse n+max

    typedef struct ana_n_max_t
    {
	int  off_ptr;
	int  off_n;
	int  off_max;
	uint elem_size;
    }
    ana_n_max_t;

    #undef DEF_TAB
    #define DEF_TAB(p,n,m,s) {	offsetof(le_analyze_t,p), \
				offsetof(le_analyze_t,n), \
				offsetof(le_analyze_t,m), \
				sizeof(*ana->p)*s },

    static const ana_n_max_t n_max_tab[] =
    {
	DEF_TAB( cup_track,	n_cup_track,	max_cup_track,	4 )
	DEF_TAB( cup_arena,	n_cup_arena,	max_cup_arena,	5 )
	DEF_TAB( property,	n_slot,		max_property,	1 )
	DEF_TAB( music,		n_slot,		max_music,	1 )
	DEF_TAB( flags_bin,	n_slot,		max_flags,	1 )
	{-1,-1,-1}
    };

    #undef DEF_TAB

    const ana_n_max_t *pnm;

    // initialize max
    for ( pnm = n_max_tab; pnm->off_ptr >= 0; pnm++ )
	*(uint*)((u8*)ana+pnm->off_max) = ~0;

    // calculate max
    for ( pnm = n_max_tab; pnm->off_ptr >= 0; pnm++ )
    {
	const u8 *ptr = *(u8**)((u8*)ana+pnm->off_ptr);
	const u8 *end = data + ana->data_size;
	uint i;
	for ( i = 0; i < n_ptr; i++ )
	    if ( ptr_list[i] > ptr && ptr_list[i] < end )
		end = ptr_list[i];

	const uint max = (end-ptr) / pnm->elem_size;
	if ( *(uint*)((u8*)ana+pnm->off_max) > max )
	     *(uint*)((u8*)ana+pnm->off_max) = max;
    }


    //--- setup flags

    if ( ana->flags_bits == 16 )
    {
	ana->max_flags /= 2;
	ana->flags = MALLOC(ana->max_flags*sizeof(*ana->flags));
	const le_flags_t *src = (le_flags_t*)ana->flags_bin;
	if (src)
	    for ( int i = 0; i < ana->max_flags; i++ )
		ana->flags[i] = be16(src++);
    }
    else
    {
	ana->flags = MALLOC(ana->max_flags*sizeof(*ana->flags));
	const le_flags8_t *src = (le_flags8_t*)ana->flags_bin;
	if (src)
	    for ( int i = 0; i < ana->max_flags; i++ )
		ana->flags[i] = *src++;
    }

    SetupArenasLEAnalyze(ana,false);
    CalculateStatsLE(ana);


    //--- return

    return ana->valid & LE_PARAM_VALID ? ERR_OK :
	   ana->valid & LE_HEAD_VALID  ? ERR_MISSING_PARAM
				       : ERR_INVALID_DATA;
}

///////////////////////////////////////////////////////////////////////////////

le_region_t GetLERegion ( const le_analyze_t *ana )
{
    if (ana->valid)
    {
	const le_binary_head_t *h = ana->head;
	switch (h->region)
	{
	    case 'P': return LEREG_PAL;
	    case 'E': return LEREG_USA;
	    case 'J': return LEREG_JAP;
	    case 'K': return LEREG_KOR;
	}
    }
    return LEREG_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////

void CalculateStatsLE ( le_analyze_t *ana )
{
    ResetLEAnalyzeUsage(ana);

    ana->used_rslots	= 0;
    ana->max_rslots	= 0;
    ana->used_bslots	= 0;
    ana->max_bslots	= 0;

    ana->max_slot = ana->max_property < ana->max_music
		  ? ana->max_property : ana->max_music;
    if ( ana->max_slot > ana->max_flags )
	 ana->max_slot = ana->max_flags;
    if ( ana->max_slot < LE_FIRST_CT_SLOT )
	return;


    //-- count racing slots

    uint used = 0, total = 0, slot;
    for ( slot = 0; slot < MKW_N_TRACKS; slot++ )
    {
	total++;
	if (IsLESlotUsed(ana,slot))
	    used++;
    }

    for ( slot = LE_FIRST_CT_SLOT; slot < ana->n_slot; slot++ )
    {
	total++;
	if (IsLESlotUsed(ana,slot))
	    used++;
    }
    ana->used_rslots = used;
    ana->max_rslots  = total;


    //-- count battle slots

    used = total = 0;
    for ( slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
    {
	total++;
	if (IsLESlotUsed(ana,slot))
	    used++;
    }
    ana->used_bslots = used;
    ana->max_bslots  = total;
}

///////////////////////////////////////////////////////////////////////////////

const le_usage_t * GetLEUsage ( const le_analyze_t *ana0, bool force_recalc )
{
    le_analyze_t *ana = (le_analyze_t*)ana0;
    if ( force_recalc && ana->usage )
	ResetLEAnalyzeUsage(ana);

    if ( ana->usage || ana->max_slot < LE_FIRST_CT_SLOT )
	return ana->usage;

    ana->usage_size = ana->n_slot > 0x100 ? ana->n_slot : 0x100;
    le_usage_t *usage = CALLOC(ana->usage_size,sizeof(*usage));
    ana->usage = usage;
    memset(usage,LEU_S_UNUSED,ana->usage_size);


    //-- random slots, can be overwritten by special slots

    memset( usage + ana->le_random_beg,
		LEU_S_LE_RANDOM|LEU_F_ONLINE,
		ana->le_random_end - ana->le_random_beg );


    //-- special slots, may overwrite random slots

    memset(usage+0x36,LEU_S_SPECIAL,5);
    usage[0x42] = LEU_S_SPECIAL;
    usage[0x43] = LEU_S_NETWORK|LEU_F_ONLINE;


    //-- check racing slots

    uint slot;
    for ( slot = 0; slot < MKW_N_TRACKS; slot++ )
	usage[slot] = IsLESlotUsed(ana,slot)
			? LEU_S_TRACK|LEU_F_ONLINE : LEU_F_ONLINE;

    for ( slot = LE_FIRST_CT_SLOT; slot < ana->usage_size; slot++ )
    {
	if (IsLESlotUsed(ana,slot))
	{
	    const le_flags_t flags = ana->flags[slot];

	    if ( flags & LEFL_ALIAS )
		 usage[slot] = LEU_S_ALIAS;
	    else if ( IsLEBattleSlot(ana,slot) )
		usage[slot]
			= flags & LEFL_RND_HEAD			? LEU_S_ARENA_RND
			: flags & (LEFL_RND_GROUP|LEFL_HIDDEN)	? LEU_S_ARENA_HIDE
			:					  LEU_S_ARENA;
	    else
		usage[slot]
			= flags & LEFL_RND_HEAD			? LEU_S_TRACK_RND
			: flags & (LEFL_RND_GROUP|LEFL_HIDDEN)	? LEU_S_TRACK_HIDE
			:					  LEU_S_TRACK;
	}

	if ( ana->version > 1 || slot < 0xff
		|| slot >= LE_FIRST_UPPER_CT_SLOT && slot <= LE_LAST_UPPER_CT_SLOT )
	{
	    usage[slot] |= LEU_F_ONLINE;
	}
    }


    //-- check battle slots

    for ( slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
    {
	if (IsLESlotUsed(ana,slot))
	{
	    const le_flags_t flags = ana->flags[slot];
	    usage[slot] = flags & LEFL_ALIAS      ? LEU_S_ALIAS
			: flags & LEFL_RND_HEAD   ? LEU_S_ARENA_RND
			: flags & LEFL_RND_GROUP  ? LEU_S_ARENA_HIDE
			:			    LEU_S_ARENA;
	}
    }


    //--- override Network.random

    if ( ana->version == 1 )
	usage[0xff] = LEU_S_NETWORK|LEU_F_ONLINE;

    return usage;
}

///////////////////////////////////////////////////////////////////////////////

static const char usage_ch[] = "-AarTtRLsn>-----";

char GetLEUsageChar ( le_usage_t usage )
{
    return usage_ch[ usage & LEU_S_MASK ];
}

///////////////////////////////////////////////////////////////////////////////

ccp GetLEUsageCharCol ( le_usage_t usage, const ColorSet_t *colset, ccp * prev_col )
{
    const char ch = usage_ch[usage & LEU_S_MASK];

    if ( colset && colset->colorize )
    {
	ccp col;
	switch ( usage & LEU_S_MASK )
	{
	    case LEU_S_SPECIAL:
	    case LEU_S_NETWORK:
	    case LEU_S_UNUSED:		col = colset->reset; break;

	    case LEU_S_ARENA:		col = colset->b_cyan; break;
	    case LEU_S_ARENA_HIDE:	col = colset->b_blue; break;
	    case LEU_S_TRACK:		col = colset->b_yellow; break;
	    case LEU_S_TRACK_HIDE:	col = colset->b_green; break;

	    case LEU_S_ARENA_RND:	col = colset->orange; break;
	    case LEU_S_TRACK_RND:	col = colset->b_orange; break;
	    case LEU_S_LE_RANDOM:	col = colset->red; break;

	    case LEU_S_ALIAS:
	    default:			col = colset->b_magenta; break;
	}

	if ( !prev_col || *prev_col != col )
	{
	    if (prev_col)
		*prev_col = col;
	    const int clen = strlen(col);
	    char *buf = GetCircBuf(clen+2);
	    memcpy(buf,col,clen);
	    buf[clen] = ch;
	    buf[clen+1] = 0;
	    return buf;
	}
    }

    return CopyCircBuf0(&ch,1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    dump			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    CHECK_OFF,		// don't check
    CHECK_TRACK,	// check for racing tracks
    CHECK_ARENA,	// check for battle arenas
    HIGHLIGHT_ARENA,	// highlight arenas
}
slot_check_t;

//-----------------------------------------------------------------------------

ccp GetLEValid ( le_valid_t valid )
{
    char *buf = GetCircBuf(8);
    DASSERT(buf);
    char *d = buf;

    *d++ = valid & LE_HEAD_FOUND   ? 'F' : '-';
    *d++ = valid & LE_HEAD_VALID   ? 'V' : '-';
    *d++ = valid & LE_HEAD_KNOWN   ? 'K' : '-';
    *d++ = valid & LE_HEAD_VERSION ? 'L' : '-';
    *d++ = valid & LE_PARAM_FOUND  ? 'f' : '-';
    *d++ = valid & LE_PARAM_VALID  ? 'v' : '-';
    *d++ = valid & LE_PARAM_KNOWN  ? 'k' : '-';
    *d = 0;
    return buf;
}

//-----------------------------------------------------------------------------

static ccp GetSlotInfo
(
    const le_analyze_t	*ana,		// valid structure
    int			tid,		// track id
    ColorSet_t		*col,		// NULL or color set for warnings
    slot_check_t	check,		// check modus
    uint		*warnings	// not NULL: increment on warnings
)
{
    if (IsLecodeRandom(tid))
	return PrintCircBuf("[%s%-7.7s%s,%s] ",
		col->info, GetLecodeRandomName(tid,"random"), col->reset,
		PrintLEFL(ana->flags_bits,ana->flags[tid],true));

    if ( tid < ana->n_slot || IsMkwArena(tid) )
    {
	uint warn = 0;


	//-- property slot

	ccp prop1 = "", prop0 = "";
	const int prop = ana->property ? ana->property[tid] : -1;
	ccp propx  = GetMkwTrackName3(prop);
	if (  check == CHECK_TRACK && ( IsMkwArena(tid) || IsMkwArena(prop) )
	   || check == CHECK_ARENA && ( !IsMkwArena(tid) || !IsMkwArena(prop) )
	   )
	{
	    warn |= 2;
	    if (col)
	    {
		prop1 = col->bad;
		prop0 = col->reset;
	    }
	}
	else if ( IsMkwOriginal(tid) && tid != prop )
	{
	    if (col)
	    {
		prop1 = col->info;
		prop0 = col->reset;
	    }
	}
	else if ( check == HIGHLIGHT_ARENA && IsMkwArena(prop) )
	{
	    if (col)
	    {
		prop1 = col->info;
		prop0 = col->reset;
	    }
	}


	//-- music slot

	ccp music1 = "", music0 = "";
	const int music = ana->music ? ana->music[tid] : -1;
	ccp musicx = GetMkwMusicName3(music);
	if ( tid < MKW_ORIGINAL_END || tid >= LE_FIRST_CT_SLOT )
	{
	    if ( music < MKW_MUSIC_MIN_ID || music > MKW_MUSIC_MAX_ID )
	    {
		warn |= 2;
		if (col)
		{
		    music1 = col->bad;
		    music0 = col->reset;
		}
	    }
	    else if ( !(music&1) )
	    {
		warn |= 1;
		if (col)
		{
		    music1 = col->hint;
		    music0 = col->reset;
		}
	    }
	    else if ( IsMkwTrack(prop) != IsMkwTrack(MusicID2TrackId(music,0,0)) )
	    {
		if (col)
		{
		    music1 = col->info;
		    music0 = col->reset;
		}
	    }
	}


	//-- flags

	static const char warn_list[] = " <!!";
	const le_flags_t flags = ana->flags[tid];
	return flags & LEFL_ALIAS
		? PrintCircBuf("[%s->%4x %s,%s] ",
				col->differ, TrackByAliasLE(prop,music),
				col->reset, PrintLEFL(ana->flags_bits,flags,true) )
		: PrintCircBuf("[%s%s%s,%s%s%s,%s]%c",
				prop1, propx, prop0,
				music1, musicx, music0,
				PrintLEFL(ana->flags_bits,flags,true), warn_list[warn] );
    }

    if (col)
    {
	static char fail[40] = {0};
	if (!*fail)
	    snprintf(fail,sizeof(fail),"[%s---,---,------%s]!",col->bad,col->reset);
	return fail;
    }
    return "[---,---,---]!";
}

//-----------------------------------------------------------------------------

static le_cup_track_t * DumpLECup
(
    FILE		*f,		// print file
    const le_analyze_t	*ana,		// valid structure
    const le_cup_track_t *cp,		// pointer to cup
    uint		n_track,	// number of tracks in cup
    ColorSet_t		*col,		// NULL or color set for warnings
    slot_check_t	check,		// check modus
    uint		*warnings,	// not NULL: store numebr of tracks warnings
    u16			*done		// not NULL: mark slot as done
)
{
    if (warnings)
	*warnings = 0;

    uint tr;
    for ( tr = 0; tr < n_track; tr++ )
    {
	const uint tid = htonl(*cp++);
	bool highlight;
	switch (check)
	{
	    case CHECK_TRACK:	highlight = !IsLecodeTrack(tid); break;
	    case CHECK_ARENA:	highlight = !IsMkwArena(tid); break;
	    default:		highlight = IsMkwSpecial(tid); break;
	}

	fprintf(f," %s%3x%s%s",
		highlight ? col->info : "",
		tid,
		highlight ? col->reset : "",
		GetSlotInfo(ana,tid,col,check,warnings) );

	if ( done && tid < ana->n_slot )
	    done[tid]++;
    }
    return (le_cup_track_t*)cp;
}

//-----------------------------------------------------------------------------

static void DumpLETracks
	( FILE *f, uint indent, ColorSet_t *col, const le_analyze_t *ana )
{
    DASSERT(f);
    DASSERT(indent>=2);
    DASSERT(col);
    DASSERT(ana);

    ccp warn = GetLecodeSupportWarning(ana);
    if (warn)
    {
	fprintf(f,"\n%*s%s"
		"Can't print track infos because %s.%s\n\n",
		indent-2,"", col->warn, warn, col->reset );
	return;
    }

    u16 *done = CALLOC(sizeof(*done),ana->n_slot);
    DASSERT(done);
    if ( ana->version == 1 )
    {
	uint max = ana->n_slot < LE_FIRST_UPPER_CT_SLOT
		 ? ana->n_slot : LE_FIRST_UPPER_CT_SLOT;
	uint i;
	for ( i = 0xff; i < max; i++ )
	    done[i]++;
    }


    //--- intro

    fprintf(f,"\n%*s%s"
	"Description of the following track information with %u-bit flags:%s\n"
	"%*s" "The general syntax is: %shex_id [ property_slot, music_index, flags ] warning%s\n",
	indent-2,"", col->heading, ana->flags_bits, col->reset,
	indent,"", col->param, col->reset );

    if ( ana->flags_bits == 16 )
	fprintf(f,
		"%*s%sFlag   1%s:  %sB%s: battle arena, %sV%s: versus track, %sr%s: random slot.\n"
		"%*s%sFlag   2%s:  Used in: %so%s: original cup, %sc%s custom cup, %sb%s: both.\n"
		"%*s%sFlag   3%s:  %sH%s: header of group, %sG%s: group member,"
				 " %sX%s: header of group and group member.\n"
		"%*s%sFlags  4%s:  %sN%s: new track, %sT%s: texture hack, %s2%s: both.\n"
		"%*s%sFlags 5+6%s: %sA%s: alias, %si%s: invisible/hidden.\n"
		,indent+2,"", col->param, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
		,indent+2,"", col->param, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
		,indent+2,"", col->param, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
		,indent+2,"", col->param, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
		,indent+2,"", col->param, col->reset
			,col->info, col->reset
			,col->info, col->reset
		);
    else
	fprintf(f,"%*s%sFlags%s: "
		"%sN%s: new, %sH%s: group header, %sG%s: group member,"
		" %sA%s: alias, %sT%s: texture hack, %si%s: hidden.\n"
		,indent,"", col->param, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
			,col->info, col->reset
		);


    //--- battle cups

    if ( ana->n_cup_arena && ana->cup_arena )
    {
	fprintf(f,"\n%*s%s" "%u cup%s with %u battle arenas:%s\n",
		indent-2,"", col->heading,
		ana->n_cup_arena, ana->n_cup_arena == 1 ? "" : "s",
		ana->n_cup_arena*5, col->reset );

	uint cup, hidden = 0;
	const le_cup_track_t *cp = ana->cup_arena;
	for ( cup = 0; cup < ana->n_cup_arena; cup++ )
	{
	    if ( cp[0] || cp[1] || cp[2] || cp[3] || cp[4] )
	    {
		fprintf(f,"%*s" "Cup %3u:",indent,"",cup);
		uint warnings;
		DumpLECup(f,ana,cp,5,col,CHECK_ARENA,&warnings,done);
		if (warnings)
		    fputs(" <<<\n",f);
		else
		    fputc('\n',f);
	    }
	    else
		hidden++;
	    cp += 5;
	}
	if (hidden)
	    fprintf(f,"%*s  > %u unused battle cup%s not listed.\n",
			indent,"", hidden, hidden==1 ? "" : "s" );
    }


    //--- racing cups

    if ( ana->n_cup_track && ana->cup_track )
    {
	fprintf(f,"\n%*s%s" "%u cup%s with %u racing tracks:%s\n",
		indent-2,"", col->heading,
		ana->n_cup_track, ana->n_cup_track == 1 ? "" : "s",
		ana->n_cup_track*4, col->reset );

	uint cup, hidden = 0;
	const le_cup_track_t *cp = ana->cup_track;
	for ( cup = 0; cup < ana->n_cup_track; cup++ )
	{
	    if ( cp[0] || cp[1] || cp[2] || cp[3] )
	    {
		fprintf(f,"%*s" "Cup %3u:",indent,"",cup);
		uint warnings;
		DumpLECup(f,ana,cp,4,col,CHECK_TRACK,&warnings,done);
		if (warnings)
		    fputs(" <<<\n",f);
		else
		    fputc('\n',f);
	    }
	    else
		hidden++;
	    cp += 4;
	}
	if (hidden)
	    fprintf(f,"%*s  > %u unused racing cup%s not listed.\n",
			indent,"", hidden, hidden==1 ? "" : "s" );
    }


    //--- count multi/group/unused slots

 #if 0 && defined(TEST) // for tests only
    done[15] = 99;
    done[16] =  0;
    done[17] = 99;
    done[18] =  0;
 #endif

    uint i, max_viewed_slot = 0, n_multi = 0, n_group = 0, n_unused = 0;
    for ( i = 0; i < ana->n_slot; i++ )
	if (IsRacingTrackLE(i))
	{
	    if (done[i])
	    {
		max_viewed_slot = i;
		if ( done[i] > 1 )
		     n_multi++;
	    }
	    else // if ( !done[i] )
	    {
		const typeof(*ana->flags) flags = ana->flags ? ana->flags[i] : 0;
		if ( flags & LEFL_RND_GROUP )
		{
		    n_group++;
		    max_viewed_slot = i;
		}
		else
		{
		    n_unused++;
		    if ( flags & LEFL_TEXTURE )
			max_viewed_slot = i;
		}
	    }
	}
    PRINT0("m=%d, g=%d, u=%d /%d\n",n_multi,n_group,n_unused,max_viewed_slot);


    //--- multi used tracks

    if (n_multi)
    {
	fprintf(f,"\n%*s%s" "%u multiple used track slot%s:%s\n",
		indent-2,"", col->heading,
		n_multi, n_multi == 1 ? "" : "s",
		col->reset );

	for ( i = 0; i < ana->n_slot; i++ )
	    if ( done[i] > 1 && IsRacingTrackLE(i) )
		fprintf(f,"%*sSlot %4d/dec : %3x%s : %3ux\n",
			indent,"", i, i, GetSlotInfo(ana,i,col,CHECK_OFF,0), done[i]);
    }


    //--- random group tracks

    if (n_group)
    {
	fprintf(f,"\n%*s%s" "%u track slot%s reserved for random groups:%s\n",
		indent-2,"", col->heading,
		n_group, n_group == 1 ? "" : "s",
		col->reset );

	for ( i = 0; i < ana->n_slot; i++ )
	    if ( !done[i] && IsRacingTrackLE(i) && ana->flags[i] & LEFL_RND_GROUP )
	    {
		done[i]++;
		fprintf(f,"%*sSlot %4d/dec : %3x%s\n",
			indent,"", i, i, GetSlotInfo(ana,i,col,CHECK_OFF,0));
	    }
    }

    //HexDump16(stdout,0,0,done,ana->n_slot*sizeof(*done));


    //--- unused used track slots

    if (n_unused)
    {
	int count = 0;
	for ( int i = 0; i <= max_viewed_slot; i++ )
	    if ( !done[i] && IsRacingTrackLE(i) )
		count++;

	if ( count > 0 )
	{
	    fprintf(f,"\n%*s%s" "%u track%s without cup reference:%s",
		    indent-2,"", col->heading,
		    count,  count == 1 ? "" : "s",
		    col->reset );

	    int prev = -1, line_count = 99;
	    for ( int i = 0; i <= max_viewed_slot; i++ )
	    {
		if ( !done[i] && IsRacingTrackLE(i) )
		{
		    if ( ++line_count > 4 || i != prev+1 )
		    {
//DEL			if ( i == prev+1 ) fputs("...",f);
			fprintf(f,"\n%*sSlot %4d/dec :", indent,"", i );
			line_count = 1;
		    }
		    fprintf(f," %3x%s", i, GetSlotInfo(ana,i,col,HIGHLIGHT_ARENA,0) );
		    prev = i;
		}
	    }
	    putchar('\n');
	}

	uint first_unused_slot = max_viewed_slot + 1;
	if (!IsRacingTrackLE(first_unused_slot))
	    first_unused_slot = LE_FIRST_CT_SLOT;
	const int unused_slots = ana->n_slot - first_unused_slot;
	if ( unused_slots > 0 )
	    fprintf(f,"%*s  > %u slot%s beginning with slot %u (%x/hex) are not used.\n",
		indent,"",
		unused_slots, unused_slots == 1 ? "" : "s",
		first_unused_slot, first_unused_slot );
    }


    //--- term

    FREE(done);
}

//-----------------------------------------------------------------------------

static void DumpLEUsageMap
	( FILE *f, uint indent, ColorSet_t *col, const le_analyze_t *ana )
{
    DASSERT(f);
    DASSERT(indent>=2);
    DASSERT(col);
    DASSERT(ana);

    const le_usage_t *usage = GetLEUsage(ana,false);
    if (!usage)
	return;

    int max_view = ana->n_slot > MKW_LE_SLOT_BEG ? ana->n_slot : MKW_LE_SLOT_BEG;
    if ( max_view > ana->usage_size )
	 max_view = ana->usage_size;

 #if 1
    fprintf(f,"\n%*s%s"	"Slot usage map for %u slots%s\n"
		"%*s"	"%s: Arena%s, %s: Hidden Arena%s, %s: Random Arena%s, "
			"%s: LE-CODE random%s, %s: Alias%s,\n"
		"%*s"	"%s: Track%s, %s: Hidden Track%s, %s: Random Track%s, "
			"%s: Network%s, %s: Special%s, %s: unused%s.\n",

		indent-2,"", col->heading, max_view, col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA,		col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_HIDE,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_RND,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_LE_RANDOM,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ALIAS,		col,0 ), col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK,		col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_HIDE,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_RND,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_NETWORK,	col,0 ), col->reset,
		 GetLEUsageCharCol(		   LEU_S_SPECIAL,	col,0 ), col->reset,
		 GetLEUsageCharCol(		   LEU_S_UNUSED,	col,0 ), col->reset );

 #else
    fprintf(f,"\n%*s%s"	"Slot usage map for %u slots%s\n"
		"%*s"	"%s: Arena%s, %s: Hidden Arena%s, %s: Random Arena%s, "
			"%s: LE-CODE arena random%s,\n"
		"%*s"	"%s: Track%s, %s: Hidden Track%s, %s: Random Track%s, "
			"%s: LE-CODE track random%s,\n"
		"%*s"	"%s: Alias%s, %s: Network%s, %s: Special%s, %s: unused%s.\n",

		indent-2,"", col->heading, max_view, col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA,		col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_HIDE,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_RND,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_LE_RND,	col,0 ), col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK,		col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_HIDE,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_RND,	col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_LE_RND,	col,0 ), col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ALIAS,		col,0 ), col->reset,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_NETWORK,	col,0 ), col->reset,
		 GetLEUsageCharCol(		   LEU_S_SPECIAL,	col,0 ), col->reset,
		 GetLEUsageCharCol(		   LEU_S_UNUSED,	col,0 ), col->reset );
 #endif

    uint slot;
    ccp prev_col = 0;
    for ( slot = 0; slot < max_view; slot++ )
    {
	if (!(slot&63))
	{
	     fprintf(f,"%s\n%*s%3x: ",col->reset,indent,"",slot);
	     prev_col = 0;
	}
	else if (!(slot&7))
	    fputc(' ',f);
	fputs(GetLEUsageCharCol(*usage++,col,&prev_col),f);
    }
    fprintf(f,"%s\n\n",col->reset);
}

///////////////////////////////////////////////////////////////////////////////

static ccp ana_std_flags ( uint flags )
{
    return PrintCircBuf("%u (%sabled%s%s%s)",
		flags,
		flags & LE_ENABLED	? "en"		: "dis",
		flags & LE_ALTERABLE	? ",alterable"	: "",
		flags & LE_EXCLUDED	? ",excluded"	: "",
		flags & LE_INCLUDED	? ",included"	: "" );
}

//-----------------------------------------------------------------------------

void DumpLEAnalyse ( FILE *f, uint indent, const le_analyze_t *ana )
{
    DASSERT(f);
    DASSERT(ana);
    indent = NormalizeIndent(indent);

    ColorSet_t col;
    SetupColorSet(&col,f);

    fprintf(f,"%*s%sLE-CODE binary, valid=%s, file size: %x/hex = %u bytes%s\n",
		indent,"",
		col.caption, GetLEValid(ana->valid), ana->data_size, ana->data_size,
		col.reset );
    indent += 4;

 #if 0 // TEST only!
    {
	static int done = 0;
	if (!done++)
	{
	    int i;
	    for ( i = -1; i < MKW_MUSIC_MAX_ID-MKW_MUSIC_MIN_ID + 4; i++ )
	    {
		uint m = i < 0 ? i : i + MKW_MUSIC_MIN_ID - 2;
		printf(" %3d = 0x%02x : %-8s /  %3d = 0x%02x : %s\n",
		    i, i&0xff, GetMkwTrackName3(i), m, m&0xff, GetMkwMusicName3(m) );
	    }

	 #if 0
	    for ( i = 0; i <= MKW_MUSIC_MAX_ID - MKW_MUSIC_MIN_ID; i+=2 )
	    {
		ccp name = GetMkwTrackName3(music_info[i/2].track);
		printf("\"%s\", \"%c%s\", \n",name,tolower(*name),name+1);
	    }
	 #endif
	}
    }
 #endif


    //--- file header

    if ( ana->valid & LE_HEAD_VALID )
    {
	const le_binary_head_t *h = ana->head;
	DASSERT(h);
	fprintf(f,
		"\n%*s%s" "File header:%s\n"
		"%*s" "Magic:             %.4s\n"
		,indent-2,"", col.heading, col.reset
		,indent,"", h->magic
		);

	if ( h->phase != 2 )
	fprintf(f,"%*s" "Phase:             %u\n",indent,"",h->phase);

	fprintf(f,
		"%*s" "Version:           %u\n"
		"%*s" "Build number:      %u\n"
		"%*s" "Region code:       %c = %s\n"
		"%*s" "Debug flag:        %c = %s\n"
		"%*s" "Header size:       %8x/hex = %6u bytes\n"
		"%*s" "Total size:        %8x/hex = %6u bytes\n"
		"%*s" "Offset param:      %8x/hex\n"
		"%*s" "Base address:      %8x/hex\n"
		"%*s" "Entry point:       %8x/hex\n"
		,indent,"", ntohl(h->version)
		,indent,"", ntohl(h->build_number)
		,indent,"", h->region
			,h->region == 'P' ? "PAL"
			:h->region == 'E' ? "USA"
			:h->region == 'J' ? "Japan"
			:h->region == 'K' ? "Korea"
			: "?"
		,indent,"", h->debug
			,h->debug == 'R' ? "Release"
			:h->debug == 'D' ? "Debug"
			: "?"
		,indent,"", ana->header_size ,ana->header_size
		,indent,"", ntohl(h->file_size), ntohl(h->file_size)
		,indent,"", ntohl(h->off_param)
		,indent,"", ntohl(h->base_address)
		,indent,"", ntohl(h->entry_point)
		);

	if (ana->creation_time)
	    fprintf(f,
		"%*s" "Time of creation:  %s\n",
		indent,"", PrintTimeByFormat("%F %T %Z",ana->creation_time));

	if (ana->edit_time)
	    fprintf(f,
		"%*s" "Time of last edit: %s\n",
		indent,"", PrintTimeByFormat("%F %T %Z",ana->edit_time));

	if ( ana->szs_required || ana->szs_recommended )
	{
	    fprintf(f,
		"%*s" "Current SZS Tool:  v%s\n", indent,"", BASE_VERSION );

	    int fw = 0;
	    ccp required = 0, recommended = 0;

	    if (ana->szs_required)
	    {
		required = DecodeVersion(ana->szs_required);
		int len = strlen(required);
		if ( fw < len )
		     fw = len;
	    }

	    if (ana->szs_recommended)
	    {
		recommended = DecodeVersion(ana->szs_recommended);
		int len = strlen(recommended);
		if ( fw < len )
		     fw = len;
	    }

	    if (ana->szs_required)
	    {
		const bool valid = GetEncodedVersion() >= ana->szs_required;
		fprintf(f,
		    "%*s" "Required SZS:      v%-*s (%s%s%s)\n"
		    ,indent,""
		    ,fw,required
		    ,valid ? col.success : col.warn
		    ,valid ? "ok" : "outdated", col.reset
		    );
	    }

	    if (ana->szs_recommended)
	    {
		const bool valid = GetEncodedVersion() >= ana->szs_recommended;
		fprintf(f,
		    "%*s" "Recommended SZS:   v%-*s (%s%s%s)\n"
		    ,indent,""
		    ,fw,recommended
		    ,valid ? col.success : col.warn
		    ,valid ? "ok" : "outdated", col.reset
		    );
	    }
	}

	if (ana->edit_version)
	    fprintf(f,
		"%*s" "Last edit by SZS:  v%s\n",
		indent,"", DecodeVersion(ana->edit_version) );
    }


    //--- parameters

    if ( ana->valid & LE_PARAM_VALID )
    {
	const le_binary_param_t *h = ana->param;
	DASSERT(h);
	fprintf(f,
		"\n%*s%s" "Parameters (LPAR):%s\n"
		"%*s" "Magic:             %.4s\n"
		"%*s" "Version:           %u\n"
		"%*s" "Param size:        %x/hex = %u bytes\n"
		,indent-2,"", col.heading, col.reset
		,indent,"", h->magic
		,indent,"", ntohl(h->version)
		,indent,"", ntohl(h->size), ntohl(h->size)
		);
    }

    switch (ana->param_vers)
    {
     case 1:
      {
	const le_binpar_v1_t *h = ana->param_v1;
	DASSERT(h);

	if ( ana->param_size >= sizeof(le_binpar_v1_264_t)  )
	    fprintf(f,
		"%*s" "Allow cheat codes: %u = %s\n"
		,indent,"", h->cheat_mode, GetCheatModeInfo(h->cheat_mode)
		);

	fprintf(f,
		"%*s" "Engine chances:    "
			"%u + %u + %u = %u (1%s0cc+mirror=total)\n"
		"%*s" "200cc:             %u = %sabled\n"
		"%*s" "Perform. monitor:  %u = %s\n"
		,indent,""
			,h->engine[0], h->engine[1], h->engine[2]
			,h->engine[0] + h->engine[1] + h->engine[2]
			,h->enable_200cc ? "50cc+20" : "00cc+15"
		,indent,"", h->enable_200cc, h->enable_200cc ? "en" :"dis"
		,indent,"", h->enable_perfmon,
			h->enable_perfmon > 1 ? "Wii(U)+Dolphin"
			: h->enable_perfmon ? "Wii(U) only" :"disabled"
		);

	if ( ana->param_size >= sizeof(le_binpar_v1_37_t)  )
	    fprintf(f,
		"%*s" "Custom Timetrial:  %u = %sabled\n"
		"%*s" "Extended P-Flags:  %u = %sabled\n"
		"%*s" "Block tracks for:  %u race%s\n"
		,indent,"", h->enable_custom_tt, h->enable_custom_tt ? "en" :"dis"
		,indent,"", h->enable_xpflags, h->enable_xpflags ? "en" :"dis"
		,indent,"", h->block_track, h->block_track == 1 ? "" : "s"
		);

	if ( ana->param_size >= sizeof(le_binpar_v1_1bc_t)  )
	{
	    ccp comment;
	    char buf[50];
	    if ( h->enable_speedo < SPEEDO_ON )
		comment = " = disabled";
	    else if ( h->enable_speedo <= SPEEDO_MAX )
	    {
		snprintf(buf,sizeof(buf)," = format \"%.*f km/h\"",
			h->enable_speedo - SPEEDO_ON, 123.12345 );
		comment = buf;
	    }
	    else
		comment = "";

	    fprintf(f,
		"%*s" "Speedometer:       %u%s\n",
		indent,"", h->enable_speedo, comment );
	}

	if ( ana->param_size >= sizeof(le_binpar_v1_260_t)  )
	{
	    fprintf(f,
		"%*s" "Debug mode:        %u = %s\n"
		"%*s" "Item cheat:        %u = %s\n"
		,indent,"", h->debug_mode, GetLecodeDebugInfo(h->debug_mode)
		,indent,"", h->item_cheat, h->item_cheat ? "enabled" : "disabled"
		);
	}

	if ( ana->param_size >= sizeof(le_binpar_v1_264_t)  )
	{
	    const int nframes  = ntohs(h->thcloud_frames);
	    fprintf(f,
		"%*s" "Drag blue shells:  %u = %sabled\n"
		"%*s" "Thundercloud time: %u frames = %.2fs\n"
		,indent,"", h->drag_blue_shell, h->drag_blue_shell ? "en" : "dis"
		,indent,"", nframes, nframes/60.0
		);
	}

	if ( ana->param_size >= sizeof(le_binpar_v1_26c_t)  )
	{
	    // [[worldwide]]
	 #if !HAVE_WIIMM_EXT
	    if ( h->bt_worldwide || h->vs_worldwide )
	 #endif
		fprintf(f,
			"%*s" "Worldwide:         bt=%u, vs=%u  (planned)\n"
			,indent,"", h->bt_worldwide, h->vs_worldwide
			);

	    fprintf(f,
		"%*s" "Textures:          bt=%s, vs=%s\n"
		"%*s" "Block textures:    %u\n"
		,indent,""
			,ana_std_flags( h->bt_textures & LE_M_TEXTURE )
			,ana_std_flags( h->vs_textures & LE_M_TEXTURE )
		,indent,"", h->block_textures
		);
	}

	// [[new-lpar]]

 #if 0 // for later use
	if ( ana->param_size >= sizeof(le_binpar_v1_1bc_t)  )
	{
	    if (h->reserved_1bb)
		fprintf(f,"%*s" "Reserved 1BB:      %u\n",
			indent,"", h->reserved_1bb );
	}
 #endif

      #if defined(DEBUG) && defined(TEST) && 0
	const uint po = ana->param_offset;
	fprintf(f,
		"%*s" "Offset cup params: %5x/hex => %5x/hex\n"
		"%*s" "Offset cup tracks: %5x/hex => %5x/hex\n"
		"%*s" "Offset cup arenas: %5x/hex => %5x/hex\n"
		"%*s" "Offset course par: %5x/hex => %5x/hex\n"
		"%*s" "Offset properties: %5x/hex => %5x/hex\n"
		"%*s" "Offset music list: %5x/hex => %5x/hex\n"
		,indent,"", ntohl(h->off_cup_par), ntohl(h->off_cup_par) + po
		,indent,"", ntohl(h->off_cup_track), ntohl(h->off_cup_track) + po
		,indent,"", ntohl(h->off_cup_arena), ntohl(h->off_cup_arena) + po
		,indent,"", ntohl(h->off_course_par), ntohl(h->off_course_par) + po
		,indent,"", ntohl(h->off_property), ntohl(h->off_property) + po
		,indent,"", ntohl(h->off_music), ntohl(h->off_music) + po
		);
      #endif

	const bool hard_coded = ana->param_size < sizeof(le_binpar_v1_f8_t);
	const u16 *chat_mode_1 = hard_coded ? chat_mode_legacy : ana->lpar.chat_mode_1;
	const u16 *chat_mode_2 = hard_coded ? chat_mode_legacy : ana->lpar.chat_mode_2;

	uint i, n_special = 0;
	for ( i = 0; i < BMG_N_CHAT; i++ )
	    if ( chat_mode_1[i] || chat_mode_2[i] )
		n_special++;

	if (n_special)
	{
	    fprintf(f,"\n%*s%s" "%u special chat message%s%s:%s\n",
		    indent-2,"", col.heading,
		    n_special, n_special == 1 ? "" : "s",
		    hard_coded ? " (hard coded)" : "",
		    col.reset );

	    for ( i = 0; i < BMG_N_CHAT; i++ )
	    {
		ccp info = PrintChatMode(i,chat_mode_1[i],chat_mode_2[i],0);
		if (info)
		    fprintf(f,"%*s%s\n", indent,"", info );
	    }
	}
      }
      break;

     default:
	fprintf(f,"%*s" ">>> other parameters unknown!\n",indent,"");
	break;
    }


    //--- section offsets

    char fail[50];
    snprintf(fail,sizeof(fail)," %s FAIL! %s",col.fail2,col.reset);

    fprintf(f,"\n%*s%s" "Sections (offset,n/max):%s\n",
		indent-2,"", col.heading, col.reset );

    if ( ana->cup_par )
	fprintf(f,
		"%*s" "Cup parameters:    %5x/hex\n",
		indent,"", PTR_DISTANCE_INT(ana->cup_par,ana->data) );

    if ( ana->cup_track )
	fprintf(f,
		"%*s" "Racing track cups: %5x/hex, %4u / %4u%s\n",
		indent,"", PTR_DISTANCE_INT(ana->cup_track,ana->data),
			ana->n_cup_track, ana->max_cup_track,
			ana->n_cup_track > ana->max_cup_track ? fail : "" );

    if ( ana->cup_arena )
	fprintf(f,
		"%*s" "Battle arena cups: %5x/hex, %4u / %4u%s\n",
		indent,"", PTR_DISTANCE_INT(ana->cup_arena,ana->data),
			ana->n_cup_arena, ana->max_cup_arena,
			ana->n_cup_arena > ana->max_cup_arena ? fail : "" );

    if ( ana->course_par )
	fprintf(f,
		"%*s" "Course parameters: %5x/hex\n",
		indent,"", PTR_DISTANCE_INT(ana->course_par,ana->data) );

    if ( ana->property )
	fprintf(f,
		"%*s" "Property list:     %5x/hex, %4u / %4u%s\n",
		indent,"", PTR_DISTANCE_INT(ana->property,ana->data),
			ana->n_slot, ana->max_property,
			ana->n_slot > ana->max_property ? fail : "" );

    if ( ana->music )
	fprintf(f,
		"%*s" "Music list:        %5x/hex, %4u / %4u%s\n",
		indent,"", PTR_DISTANCE_INT(ana->music,ana->data),
			ana->n_slot, ana->max_music,
			ana->n_slot > ana->max_music ? fail : "" );

    if ( ana->flags )
	fprintf(f,
		"%*s" "Flags list:        %5x/hex, %4u / %4u (%u bits)%s\n",
		indent,"", PTR_DISTANCE_INT(ana->flags_bin,ana->data),
			ana->n_slot, ana->max_flags, ana->flags_bits,
			ana->n_slot > ana->max_flags ? fail : "" );

    if ( ana->end_of_data )
	fprintf(f,
		"%*s" "End of data:       %5x/hex\n",
		indent,"", PTR_DISTANCE_INT(ana->end_of_data,ana->data) );

    fprintf(f,
		"%*s" "End of file:       %5x/hex\n",
		indent,"", ana->data_size );


    //--- settings

    if ( ana->cup_par || ana->course_par )
    {
	fprintf(f,"\n%*s%s" "Cup and course parameters (n/max):%s\n",
		indent-2,"", col.heading, col.reset );

	if ( ana->cup_par )
	    fprintf(f,
		"%*s" "Cup magic:         %s\n"
		"%*s" "Racing track cups: %4u / %4u%s\n"
		"%*s" "Battle arena cups: %4u / %4u%s\n"
		,indent,"", ana->cup_par->magic
		,indent,"", ana->n_cup_track, ana->max_cup_track,
			    ana->n_cup_track > ana->max_cup_track ? fail : ""
		,indent,"", ana->n_cup_arena, ana->max_cup_arena,
			    ana->n_cup_arena > ana->max_cup_arena ? fail : ""
		);

	if ( ana->course_par )
	    fprintf(f,
		"%*s" "Course magic:      %s\n"
		"%*s" "Total track slots: %4u / %4u%s\n"
		"%*s" "Used racing slots: %4u / %4u\n"
		"%*s" "Used battle slots: %4u / %4u\n"
		,indent,"", ana->course_par->magic
		,indent,"", ana->n_slot, ana->max_slot,
			    ana->n_slot > ana->max_slot ? fail : ""
		,indent,"", ana->used_rslots, ana->max_rslots
		,indent,"", ana->used_bslots, ana->max_bslots
		);
    }

    if ( long_count > 0 )
	DumpLETracks(f,indent,&col,ana);
    if ( verbose > 0 )
	DumpLEUsageMap(f,indent,&col,ana);

    fputc('\n',f);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			copy track files		///////////////
///////////////////////////////////////////////////////////////////////////////

void TransferTrackFile
	( LogFile_t *log, uint dest_slot, ccp src_name, TransferMode_t flags )
{
    if ( opt_track_dest
	&& IsDirectory(opt_track_dest,false)
	&& opt_track_source.used )
    {
	char src[PATH_MAX];
	char dest[PATH_MAX];
	flags &= TFMD_M_FLAGS;

	ParamFieldItem_t *ptr = opt_track_source.field, *end;
	for ( end = ptr + opt_track_source.used; ptr < end; ptr++ )
	{
	    PathCatBufPPE(src,sizeof(src),ptr->key,src_name,".szs");

	    struct stat st;
	    if ( !stat(src,&st) && S_ISREG(st.st_mode) )
	    {
		snprintf(dest,sizeof(dest),"%s/%03x.szs",opt_track_dest,dest_slot);
		if (!TransferFile(log,dest,src,ptr->num|flags,0666))
		{
		    PathCatBufPPE(src,sizeof(src),ptr->key,src_name,"_d.szs");
		    if ( !stat(src,&st) && S_ISREG(st.st_mode) )
		    {
			snprintf(dest,sizeof(dest),"%s/%03x_d.szs",opt_track_dest,dest_slot);
			TransferFile(log,dest,src,ptr->num|flags,0666);
		    }
		}
		break;
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void TransferTrackBySlot
	( LogFile_t *log, uint dest_slot, uint src_slot, TransferMode_t flags )
{
    if	(  IsValidLecodeSlot(dest_slot)
	&& IsValidLecodeSlot(src_slot)
	&& opt_track_dest
	&& IsDirectory(opt_track_dest,false)
	)
    {
	flags &= TFMD_M_FLAGS;

	char dest[PATH_MAX], src[PATH_MAX];
	snprintf( dest, sizeof(dest), "%s/%03x.szs", opt_track_dest, dest_slot );
	snprintf( src,  sizeof(src),  "%s/%03x.szs", opt_track_dest, src_slot  );

	if (!TransferFile(log,dest,src,TFMD_LINK|flags,0666))
	{
	    snprintf( dest, sizeof(dest), "%s/%03x_d.szs", opt_track_dest, dest_slot );
	    snprintf( src,  sizeof(src),  "%s/%03x_d.szs", opt_track_dest, src_slot  );
	    TransferFile(log,dest,src,TFMD_LINK|flags,0666);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LECODE file			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool le_file_done = false;
static ctcode_t *le_file = 0;

///////////////////////////////////////////////////////////////////////////////

const ctcode_t * LoadLEFile ( uint le_phase )
{
    const ct_mode_t ct_mode = le_phase == 2 ? CTM_LECODE2 : CTM_LECODE1;
    if ( le_file && le_file->ctb.ct_mode != ct_mode )
    {
	// rescan nessessary
	ResetCTCODE(le_file);
	FREE(le_file);
	le_file = 0;
	le_file_done = false;
    }


    // --le-define

    if ( !le_file_done && opt_le_define && *opt_le_define )
    {
	PRINT("le=%d, ctm=%d[%s]\n",
			le_phase, ct_mode, GetCtModeNameBMG(ct_mode,true) );
	if ( logging >= 2 )
	    fprintf(stdlog,"Load LE definition (%s)\n",
			GetCtModeNameBMG(ct_mode,true) );

	le_file_done = true;
	le_file = MALLOC(sizeof(*le_file));
	InitializeCTCODE(le_file,ct_mode);

	raw_data_t raw;
	if ( LoadRawData(&raw,true,opt_le_define,0,false,0)
	    || ScanRawDataCTCODE(le_file,CTM_NO_INIT,&raw,0) )
	{
	    ResetCTCODE(le_file);
	    FREE(le_file);
	    le_file = 0;
	}
	ResetRawData(&raw);
    }
    return le_file;
}

///////////////////////////////////////////////////////////////////////////////

enumError ApplyLEFile ( le_analyze_t * ana )
{
    DASSERT(ana);
    SetupArenasLEAnalyze(ana,false);

    const ctcode_t *ctcode = LoadLEFile(ana->version);
    return ctcode ? ApplyCTCODE(ana,ctcode) : ERR_ERROR;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ApplyCTCODE()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ApplyCTCODE ( le_analyze_t * ana, const ctcode_t * ctcode )
{
    DASSERT(ana);
    DASSERT(ctcode);

    //--- setup

    if (ctcode->lpar)
	ana->lpar = *ctcode->lpar;

    ana->n_cup_track	= 0;
    ana->n_cup_arena	= 0;
    ana->n_slot		= 0;

    le_property_t *prop  = ana->property;
    le_music_t    *music = ana->music;
    le_flags_t	  *flags = ana->flags;

    if ( prop && music && flags )
    {
	const uint max	= ana->max_slot < LE_FIRST_CT_SLOT
			? ana->max_slot : LE_FIRST_CT_SLOT;
	uint tid;
	for ( tid = 0; tid < max; tid++ )
	{
	    *prop++	= tid;
	    *music++	= GetMkwMusicSlot(tid);
	    *flags++	= 0;
	}
	//HexDump16(stdout,5,0,ana->property,max);
	//HexDump16(stdout,6,0,ana->music,max);
	//HexDump16(stdout,7,0,ana->flags,max);
    }


    //--- copy racing cups

    const ctcode_cup1_data_t *cup = ctcode->cup_racing;
    le_cup_track_t *cup_dest = ana->cup_track;
    if ( cup && cup_dest )
    {
	uint cup_idx, max = ctcode->n_racing_cups;
	if ( max > ana->max_cup_track )
	     max = ana->max_cup_track;

	for ( cup_idx = 0; cup_idx < max; cup_idx++, cup++ )
	{
	    *cup_dest++ = cup->track_id[0];
	    *cup_dest++ = cup->track_id[1];
	    *cup_dest++ = cup->track_id[2];
	    *cup_dest++ = cup->track_id[3];
	    ana->n_cup_track++;
	}
    }


    //--- copy battle cups

    cup = ctcode->cup_battle;
    cup_dest = ana->cup_arena;
    if ( cup && cup_dest )
    {
	uint cup_idx, max = ctcode->n_battle_cups;
	if ( max > ana->max_cup_arena )
	     max = ana->max_cup_arena ;

	for ( cup_idx = 0; cup_idx < max; cup_idx++, cup++ )
	{
	    *cup_dest++ = cup->track_id[0];
	    *cup_dest++ = cup->track_id[1];
	    *cup_dest++ = cup->track_id[2];
	    *cup_dest++ = cup->track_id[3];
	    *cup_dest++ = cup->track_id[4];
	    ana->n_cup_arena++;
	}
    }


    //--- copy track properties

    const ctcode_crs1_data_t *td = ctcode->crs->data;
    prop  = ana->property;
    music = ana->music;
    flags = ana->flags;

    if ( td && prop && music && flags )
    {
	uint tidx, max = ctcode->n_tracks;
	if ( max > ana->max_slot )
	     max = ana->max_slot ;

	for ( tidx = 0; tidx < max; tidx++, td++ )
	{
	    *prop++  = ntohl(td->property_id);
	    *music++ = ntohl(td->music_id);
	    *flags++ = ctcode->le_flags[tidx];
	    ana->n_slot++;
	}
    }


    //--- arena setup (override)

    SetupArenasLEAnalyze(ana,false);
    ApplyArena(ana,&ctcode->arena);


    //--- setup by version

    switch (ana->param_vers)
    {
     case 1:
	ana->cup_par->n_racing_cups = htonl(ana->n_cup_track);
	ana->cup_par->n_battle_cups = htonl(ana->n_cup_arena);
	ana->course_par->n_slot = htonl(ana->n_slot);
	break;
    }


    //--- copy/move/link track files

    LogFile_t log0 = { .log = stdlog };
    LogFile_t *log = logging >= 3 ? &log0 : 0;

    char buf[500];
    const bmg_item_t * bi = ctcode->track_file.item;
    const bmg_item_t * bi_end = bi + ctcode->track_file.item_used;
    for ( ; bi < bi_end; bi++ )
    {
	PrintString16BMG( buf, sizeof(buf), bi->text, bi->len,
					BMG_UTF8_MAX, 0, true );
	TransferTrackFile(log,bi->mid - ctcode->ctb.track_name1.beg,buf,0);
    }

    CalculateStatsLE(ana);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool DefineAliasLE ( le_analyze_t * ana, uint slot, uint alias )
{
    if ( !ana->property || !ana->music || !ana->flags )
	return false;

    DASSERT(ana);
    if ( slot >= ana->n_slot )
    {
	printf("ALIAS: Invalid slot: %x = %x\n",slot,alias);
	return false;
    }

    for(;;)
    {
	if ( alias >= ana->n_slot
		|| alias >= MKW_ARENA_BEG && alias < LE_FIRST_CT_SLOT )
	{
	    printf("ALIAS: Invalid alias: %x = %x\n",slot,alias);
	    return false;
	}

	const le_flags_t fl = ana->flags[alias];
	if ( !(fl & LEFL_ALIAS) )
	{
	    ana->flags[slot]   |= LEFL_ALIAS;
	    ana->property[slot] = alias >> 8;
	    ana->music[slot]    = alias & 0xff;
	    return true;
	}

	alias = TrackByAliasLE(ana->property[alias],ana->music[alias]);
    }
}

///////////////////////////////////////////////////////////////////////////////

uint PatchAliasLE ( le_analyze_t * ana, ccp list )
{
    DASSERT(ana);

    if (!list)
	return 0;

    uint count = 0;

    ccp ptr = (char*)list;
    while (*ptr)
    {
	while ( isspace((int)*ptr) || *ptr == ',' )
	    ptr++;

	char *end;
	const uint slot = str2ul(ptr,&end,16);
	if ( end > ptr )
	{
	    ptr = end;
	    while ( isspace((int)*ptr) )
		ptr++;

	    if ( *ptr == '=' )
	    {
		const uint alias = str2ul(ptr+1,&end,16);
		if ( end > ptr )
		{
		    ptr = end;
		    count += DefineAliasLE(ana,slot,alias);
		}
	    }
	}
	while ( *ptr && *ptr != ',' )
	    ptr++;
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

void PatchLPAR ( le_lpar_t * lp )
{
    DASSERT(lp);

    if (opt_engine_valid)
	memcpy(lp->engine,opt_engine,sizeof(lp->engine));
    if ( opt_200cc != OFFON_AUTO )
	lp->enable_200cc = opt_200cc >= OFFON_ON;
    if ( opt_perfmon != OFFON_AUTO )
	lp->enable_perfmon = opt_perfmon >= OFFON_FORCE ? 2 : opt_perfmon >= OFFON_ON;
    if ( opt_custom_tt != OFFON_AUTO )
	lp->enable_custom_tt = opt_custom_tt >= OFFON_ON;
    if ( opt_xpflags != OFFON_AUTO )
	lp->enable_xpflags = opt_xpflags >= OFFON_ON;
    if ( opt_speedo != OFFON_AUTO )
	lp->enable_speedo = opt_speedo >= SPEEDO_MAX ? SPEEDO_MAX
				: opt_speedo >= SPEEDO_ON ? opt_speedo : 0;
    if ( opt_lecode_debug != OFFON_AUTO )
	lp->debug_mode = opt_lecode_debug >= DEBUGMD_ENABLED && opt_lecode_debug < DEBUGMD__N
				? opt_lecode_debug : DEBUGMD_OFF;

    // [[new-lpar]]
}

///////////////////////////////////////////////////////////////////////////////

void UpdateLecodeFlags ( le_analyze_t * ana )
{
    DASSERT(ana);
    if (!ana->flags)
	return;


    //--- calculate flags LEFL_BATTLE and LEFL_VERSUS, reset other flags

    for ( int slot = 0; slot < ana->max_flags; slot++ )
    {
	le_flags_t flags = ana->flags[slot] & LEFL__ALL;
	if ( slot < ana->max_property )
	{
	    if ( slot < MKW_ARENA_END || slot >= MKW_LE_SLOT_BEG )
		flags |= IsMkwTrack(ana->property[slot]) ? LEFL_VERSUS : LEFL_BATTLE;
	}
	ana->flags[slot] = flags;
    }


    //--- define LEFL_RANDOM

    for ( int slot = LE_SLOT_RND_BEG; slot < LE_SLOT_RND_END; slot++ )
	ana->flags[slot] |= LEFL_RANDOM;


    //--- calculate flags LEFL_CUP, LEFL_ORIG_CUP, LEFL_CUSTOM_CUP

    if ( ana->n_cup_track && ana->cup_track )
    {
	if ( ana->n_cup_track & 1 )
	    ana->n_cup_track++;

	const le_cup_track_t *src = ana->cup_track;
	const int max_tr = ana->n_cup_track * 4;
	for ( int tr = 0; tr < max_tr; tr++ )
	{
	    const uint slot = ntohl(*src++);
	    if ( slot < ana->max_flags && ana->flags[slot] & (LEFL_VERSUS|LEFL_RANDOM) )
		ana->flags[slot] |= LEFL_CUP | ( tr < MKW_N_TRACKS ? LEFL_ORIG_CUP : LEFL_CUSTOM_CUP );
	}
    }

    if ( ana->n_cup_arena && ana->cup_arena )
    {
	const le_cup_track_t *src = ana->cup_arena;
	const int max_tr = ana->n_cup_arena * 5;
	for ( int tr = 0; tr < max_tr; tr++ )
	{
	    const uint slot = ntohl(*src++);
	    if ( slot < ana->max_flags && ana->flags[slot] & (LEFL_BATTLE|LEFL_RANDOM) )
		ana->flags[slot] |= LEFL_CUP | ( tr < MKW_N_ARENAS ? LEFL_ORIG_CUP : LEFL_CUSTOM_CUP );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void CopyLecodeFlags ( le_analyze_t * ana )
{
    DASSERT(ana);
    UpdateLecodeFlags(ana);


    //--- store flags intoLE-CODE binary

    if ( !ana->flags || !ana->flags_bin )
	return;

    const le_flags_t *src = ana->flags;
    if ( ana->flags_bits == 16 )
    {
	le_flags_t *dest = (le_flags_t*)ana->flags_bin;
	for ( int i = 0; i < ana->max_flags; i++ )
		write_be16(dest++,*src++);
    }
    else
    {
	le_flags8_t *dest = (le_flags8_t*)ana->flags_bin;
	for ( int i = 0; i < ana->max_flags; i++ )
		*dest++ = *src++;
    }
}

///////////////////////////////////////////////////////////////////////////////

void PatchLECODE ( le_analyze_t * ana )
{
    DASSERT(ana);
    PRINT("PatchLECODE() opt_lpar=%s\n",opt_lpar);

    ccp warn = GetLecodeSupportWarning(ana);
    if (warn)
    {
	ERROR0(ERR_INVALID_DATA,"Can't use or patch LE-CODE because %s.",warn);
	return;
    }

    u8 *saved = MEMDUP(ana->data,ana->data_size);

    if (opt_lpar)
	LoadLPAR(&ana->lpar,false,0,false);

    switch (ana->param_vers)
    {
     case 1:
	if (opt_le_alias)
	    PatchAliasLE(ana,opt_le_alias);
	PatchLPAR(&ana->lpar);
	break;
    }

    CopyLecodeFlags(ana);
    CopyLPAR2Data(ana);

    if ( ana->header_vers >= 5 && memcmp(ana->data,saved,ana->data_size) )
    {
	le_binary_head_v5_t *h5 = (le_binary_head_v5_t*)ana->data;
	h5->edit_version = htonl(GetEncodedVersion());
	h5->edit_time = htonl(GetTimeSec(false));
    }

    FREE(saved);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    ScanTextLPAR() helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextLPAR_PARAM
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ScanInfo_t		*si,		// valid data
    bool		is_pass2	// false: pass1; true: pass2
)
{
    DASSERT(lpar);
    DASSERT(si);
    PRINT(">> ScanTextLPAR_PARAM(pass=%u)\n",is_pass2+1);


    //--- setup data

    int limit_mode = -1;
    const ScanParam_t ptab[] =
    {
	{ "LIMIT-MODE",		SPM_INT, &limit_mode },
	{ "CHEAT-MODE",		SPM_U8,  &lpar->cheat_mode },
	{ "ENGINE",		SPM_U8,	  lpar->engine, 3,3 },
	{ "ENABLE-200CC",	SPM_U8,	 &lpar->enable_200cc },
	{ "PERF-MONITOR",	SPM_U8,	 &lpar->enable_perfmon },
	{ "CUSTOM-TT",		SPM_S8,	 &lpar->enable_custom_tt },
	{ "XPFLAGS",		SPM_U8,	 &lpar->enable_xpflags },
	{ "BLOCK-TRACK",	SPM_U8,	 &lpar->block_track },
	{ "SPEEDOMETER",	SPM_U8,	 &lpar->enable_speedo },
	{ "DEBUG",		SPM_U8,	 &lpar->debug_mode },
	{ "ITEM-CHEAT",		SPM_U8,	 &lpar->item_cheat },
	{ "ITEM-CHEATS",	SPM_U8,	 &lpar->item_cheat }, // bug in 2021-04, [[obsolete]] in 2022
	{ "DRAG-BLUE-SHELL",	SPM_U8,  &lpar->drag_blue_shell },
	{ "THCLOUD-TIME",	SPM_U16, &lpar->thcloud_frames },
	{ "BT-WORLDWIDE",	SPM_U8,  &lpar->bt_worldwide },
	{ "VS-WORLDWIDE",	SPM_U8,  &lpar->vs_worldwide },
	{ "BT-TEXTURES",	SPM_U8,  &lpar->bt_textures },
	{ "VS-TEXTURES",	SPM_U8,  &lpar->vs_textures },
	{ "BLOCK-TEXTURES",	SPM_U8,  &lpar->block_textures },

	// [[new-lpar]]
	{0}
    };


    //--- main loop

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	ScanParamSI(si,ptab);
    }
    CheckLevelSI(si);

    if ( limit_mode >= 0 )
	LimitToLparMode(lpar,limit_mode);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextLPAR_CHAT
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ScanInfo_t		* si,		// valid data
    bool		is_pass2	// false: pass1; true: pass2
)
{
    PRINT(">> ScanTextLPAR_CHAT(pass=%u)\n",is_pass2+1);


    //--- setup data

    bool reset = true, legacy = false, mkwfun = false;
    const ScanParam_t ptab[] =
    {
	{ "RESET",	SPM_BOOL,	&reset },
	{ "LEGACY",	SPM_BOOL,	&legacy },
	{ "MKW-FUN",	SPM_BOOL,	&mkwfun },
	{0}
    };


    //--- main loop

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    if (reset)
	    {
		reset = false;
		ClearLparChat(lpar);
	    }

	    if ( legacy || mkwfun )
	    {
		const u16 *chat_mode = legacy ? chat_mode_legacy : chat_mode_mkwfun;
		uint i;
		for ( i = 0; i < BMG_N_CHAT; i++ )
		    if (!lpar->chat_mode_1[i])
			lpar->chat_mode_1[i] = chat_mode[i];
		legacy = mkwfun = false;
	    }
	    continue;
	}

	if (reset)
	{
	    reset = false;
	    ClearLparChat(lpar);
	}

	char name[20];
	ScanNameSI(si,name,sizeof(name),true,true,0);
	if ( name[0] != 'M' )
	{
	    wrong_name:;
	    ScanFile_t *sf = si->cur_file;
	    sf->ptr = sf->prev_ptr;
	    WarnIgnoreSI(si,"Message name (M01..M96) expected: %s\n",name);
	    continue;
	}

	char *end;
	long num = strtol(name+1,&end,10);
	if ( *end || num < 1 || num > BMG_N_CHAT )
	    goto wrong_name;

	CheckWarnSI(si,'=',ERR_OK);

	DEFINE_VAR(var);
	if (ScanExprSI(si,&var))
	    continue;
	lpar->chat_mode_1[num-1] = GetIntV(&var);

	if ( NextCharSI(si,false) == ',' )
	{
	    si->cur_file->ptr++;
	    if (!ScanExprSI(si,&var))
		lpar->chat_mode_2[num-1] = GetIntV(&var);
	}

	FreeV(&var);
	CheckEolSI(si);
    }
    CheckLevelSI(si);

    if (reset)
	ClearLparChat(lpar);

    //HEXDUMP16(0,0,fb->buf,fb->ptr-fb->buf);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextLPAR_DEBUG
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ScanInfo_t		*si,		// valid data
    bool		is_pass2,	// false: pass1; true: pass2
    uint		config		// configuration index 0..
)
{
    DASSERT(lpar);
    DASSERT(si);
    DASSERT( config < LEDEB__N_CONFIG );
    PRINT(">> ScanTextLPAR_PARAM(pass=%u)\n",is_pass2+1);
    u32 *debug_list = lpar->debug[config];


    //--- setup data

    const int hide_mask = 1 << config;
    int hide_speedo = hide_mask & lpar->no_speedo_if_debug;
    int line = -1, next_line = -1, setup = PREDEBUG_OFF;
    lecode_debug_ex_t debug;
    DecodeLecodeDebug(&debug,0);

    const ScanParam_t ptab[] =
    {
	{ "SETUP",		SPM_INT, &setup },
	{ "HIDE-SPEEDO",	SPM_INT, &hide_speedo },
	{ "LINE",		SPM_INT, &next_line },
	{ "ENABLED",		SPM_U8,  &debug.enabled },
	{ "OPPONENT",		SPM_U8,  &debug.opponent },
	{ "SPACE",		SPM_U8,  &debug.space },
	{ "POSITION",		SPM_U8,  &debug.position },
	{ "CHECK-POINT",	SPM_U8,  &debug.check_point },
	{ "ITEM-POINT",		SPM_U8,  &debug.item_point },
	{ "KCL-TYPE",		SPM_U8,  &debug.kcl_type },
	{ "RESPAWN",		SPM_U8,  &debug.respawn },
	{ "LAP-POS",		SPM_U8,  &debug.lap_pos },
	{ "TRACK-ID",		SPM_U8,  &debug.track_id },
	{ "XPF",		SPM_U8,  &debug.xpf },
	// [[new-debug]]
	{0}
    };


    //--- main loop

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	ScanParamSI(si,ptab);

	if ( setup != PREDEBUG_OFF )
	{
	    if ( SetupLecodeDebugLPAR(lpar,config,setup,&hide_speedo) && line >= 0 )
		DecodeLecodeDebug(&debug,debug_list[line]);
	    setup = PREDEBUG_OFF;
	}

	if ( line != next_line )
	{
	    if ( line >= 0 )
		debug_list[line] = EncodeLecodeDebug(&debug);

	    if ( next_line < 0 || next_line >= LEDEB__N_LINE )
		next_line = -1;
	    else
		DecodeLecodeDebug(&debug,debug_list[next_line]);
	    line = next_line;
	}
    }

    if ( line >= 0 )
	debug_list[line] = EncodeLecodeDebug(&debug);

    if ( hide_speedo > 0 )
	lpar->no_speedo_if_debug |= hide_mask;
    else
	lpar->no_speedo_if_debug &= ~hide_mask;

    CheckLevelSI(si);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ScanTextLPAR()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    bool		init_lpar,	// true: initialize 'lpar' first
    ccp			fname,		// filename of source, if NULL: use opt_lpar
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(lpar);
    if (init_lpar)
	InitializeLPAR(lpar,false);

    bool use_lpar = fname == 0;
    if (use_lpar)
    {
	if ( !opt_lpar || !*opt_lpar )
	    return ERR_OK;
	fname = opt_lpar;
    }

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	if ( raw.fform != FF_LPAR )
	    err = ERROR0(ERR_WRONG_FILE_TYPE,
			"Not a LPAR file: %s:%s\n",
			GetNameFF(raw.fform,0), raw.fname );
	else
	    err = ScanTextLPAR(lpar,false,fname,raw.data,raw.data_size);
    }

    if ( err && use_lpar )
	opt_lpar = 0;

    ResetRawData(&raw);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanTextLPAR
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    bool		init_lpar,	// true: initialize 'lpar' first
    ccp			fname,		// NULL or filename for error messages
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    PRINT("ScanTextLPAR(init=%d,size=%u)\n",init_lpar,data_size);
    DASSERT(lpar);
    if (init_lpar)
	InitializeLPAR(lpar,false);

    enum { C_PARAM, C_CHAT, C_DEBUG1, C_DEBUG2, C_DEBUG3, C_DEBUG4, C_END };
    static const KeywordTab_t section_name[] =
    {
	{ C_PARAM,	"LECODE-PARAMETERS",		0, 0 },
	{ C_CHAT,	"CHAT-MESSAGE-MODES",		0, 0 },
	{ C_END,	"DEBUG-DOCU",			0, 0 },
	{ C_DEBUG1,	"DEBUG-1",			0, 0 },
	{ C_DEBUG2,	"DEBUG-2",			0, 0 },
	{ C_DEBUG3,	"DEBUG-3",			0, 0 },
	{ C_DEBUG4,	"DEBUG-4",			0, 0 },
	{ C_END,	"END",				0, 0 },
	{0,0,0,0}
    };

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,fname,0);
    si.predef = SetupVarsLECODE();

    bool is_pass2 = false;
    enumError max_err = ERR_OK;

    for(;;)
    {
	PRINT("----- SCAN KMP SECTIONS, PASS%u ...\n",is_pass2+1);

	max_err = ERR_OK;
	si.no_warn = !is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar,"$PASS",is_pass2+1);

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
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    PRINT("--> pass=%u: #%04u: %s\n",is_pass2+1,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    noPRINT("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case C_PARAM:
		    err = ScanTextLPAR_PARAM(lpar,&si,is_pass2);
		    break;

		case C_CHAT:
		    err = ScanTextLPAR_CHAT(lpar,&si,is_pass2);
		    break;

		case C_DEBUG1:
		case C_DEBUG2:
		case C_DEBUG3:
		case C_DEBUG4:
		    err = ScanTextLPAR_DEBUG(lpar,&si,is_pass2,cmd->id-C_DEBUG1);
		    break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (is_pass2)
	    break;
	is_pass2 = true;

	RestartSI(&si);
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

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


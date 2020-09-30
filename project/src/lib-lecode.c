
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
 *   Copyright (c) 2011-2020 by Dirk Clemens <wiimm@wiimm.de>              *
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
#include "lib-lecode.h"
#include "lib-std.h"
#include "lib-mkw.h"
#include "lib-szs.h"
#include "lpar.inc"
#include "lex.inc"

#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    vars			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp	opt_le_define		= 0;
ccp	opt_lpar		= 0;
ccp	opt_track_dest		= 0;
ParamField_t opt_track_source	= {0};

ccp	opt_le_alias		= 0;
bool	opt_engine_valid	= false;
u8	opt_engine[3]		= {0};
OffOn_t	opt_200cc		= OFFON_AUTO;
OffOn_t	opt_perfmon		= OFFON_AUTO;
OffOn_t	opt_custom_tt		= OFFON_AUTO;
OffOn_t	opt_xpflags		= OFFON_AUTO;

bool	opt_complete		= false;

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
///////////////			    helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

uint GetNextRacingTrackLE ( uint tid )
{
    if ( ++tid == 0xff )
	tid++;

    if ( tid > MKW_N_TRACKS && tid < LE_FIRST_LOWER_CT_SLOT )
	tid = LE_FIRST_LOWER_CT_SLOT;
    return tid;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptTrackSource ( ccp arg, int mode )
{
    if ( arg && *arg )
    {
	if (!opt_track_source.field)
	    InitializeParamField(&opt_track_source);
	AppendParamField(&opt_track_source,arg,false,mode,0);
    }
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

    if  (sum > 0.0 )
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
    static VarMap_t vm = {0};
    if (!vm.used)
	DefineMkwVars(&vm);
    return &vm;
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

    if ( lp->enable_perfmon > 1 )
	return LPM_EXPERIMENTAL;

    return lp->enable_perfmon || !lp->enable_xpflags
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
    }
    else if ( lpmode == LPM_TESTING )
    {
	if ( lp->enable_perfmon > 1 )
	    { lp->enable_perfmon  = 1; modified = true; }
    }

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
    0,0,0,0, 0,0,0,0,		//  1 ..  8
    0,0,0,0, 0,0,0,0,		//  9 .. 16
    0,0,0,0, 0,0,0,0,		// 17 .. 24
    0,0,0,0, 0,0,0,0,		// 25 .. 32
    0,0,0,0, 0,0,0,0,		// 33 .. 40
    0,0,0,0, 0,0,0,0,		// 41 .. 48
    0,0,0,0, 0,0,0,0,		// 49 .. 56
    0,0,0,0, 0,0,0,0,		// 57 .. 64

    CHATMD_ANY_VEHICLE, 0,0,0,	// 65 .. 68
    CHATMD_KARTS_ONLY,		// 69
    CHATMD_BIKES_ONLY,		// 70
    CHATMD_TRACK_BY_HOST,	// 71
    CHATMD_ANY_TRACK,		// 72

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

    lpar->limit_mode = LPM__DEFAULT;

    lpar->engine[0]	= 10;
    lpar->engine[1]	= 60;
    lpar->engine[2]	= 30;
    lpar->enable_xpflags= 1;
    lpar->block_track	= 0;

    if ( load_lpar && opt_lpar )
	LoadLPAR(lpar,false,0,false);
}

///////////////////////////////////////////////////////////////////////////////

static void NormalizeLPAR ( le_lpar_t * lp )
{
    DASSERT(lp);

    int sum = lp->engine[0] + lp->engine[1] + lp->engine[2];
    if ( sum != 0 && sum != 100 )
    {
	double factor = 100.0/sum;
	lp->engine[0] = lp->engine[0] * factor;
	lp->engine[1] = lp->engine[1] * factor;
	sum = lp->engine[0] + lp->engine[1];
	if ( sum > 100 )
	    lp->engine[1] -= sum-100;
	lp->engine[2] = 100 - lp->engine[0] - lp->engine[1];
    }

    lp->enable_200cc		= lp->enable_200cc > 0;
    lp->enable_perfmon		= lp->enable_perfmon < 2 ? lp->enable_perfmon : 2;
    lp->enable_custom_tt	= lp->enable_custom_tt > 0;
    lp->enable_xpflags		= lp->enable_xpflags > 0;
    lp->block_track		= lp->block_track < LE_MAX_BLOCK_TRACK
				? lp->block_track : LE_MAX_BLOCK_TRACK;
}

///////////////////////////////////////////////////////////////////////////////

static void CopyLPAR2Data ( le_analyse_t * ana )
{
    DASSERT(ana);

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

    if ( offsetof(le_binpar_v1_t,block_track) < ana->param_size )
	h->block_track = lp->block_track;

    if ( offsetof(le_binpar_v1_t,chat_mode_1) + sizeof(h->chat_mode_1) <= ana->param_size )
	write_be16n(h->chat_mode_1,lp->chat_mode_1,BMG_N_CHAT);

    if ( offsetof(le_binpar_v1_t,chat_mode_2) + sizeof(h->chat_mode_2) <= ana->param_size )
	write_be16n(h->chat_mode_2,lp->chat_mode_2,BMG_N_CHAT);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextLPAR
(
    const le_lpar_t	*lpar,		// LE-CODE parameters
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(lpar);
    DASSERT(fname);
    PRINT("SaveTextLEX(%s,%d)\n",fname,set_time);


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;


    //--- print header + syntax info

    if ( print_header && !brief_count && !export_count )
	fprintf(F.f,text_lpar_head_cr);
    else
	fprintf(F.f,"%s\r\n",LE_LPAR_MAGIC);


    //--- print section

    err = WriteSectionLPAR(F.f,lpar);


    //--- print footer

    fputs(section_end,F.f);
    ResetFile(&F,set_time);
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
	fprintf(f,text_lpar_sect_param_cr
		,GetLparModeName(CalcCurrentLparMode(lpar,true),export_count>0)
		,lpar->engine[0],lpar->engine[1],lpar->engine[2]
		,lpar->enable_200cc
		,lpar->enable_perfmon
		,lpar->enable_custom_tt
		,lpar->enable_xpflags
		,LE_MAX_BLOCK_TRACK
		,lpar->block_track
		);
    else
	fprintf(f,
	       "\r\n"
	       "LIMIT-MODE	= %s\r\n\r\n"
	       "ENGINE		= %u,%u,%u\r\n"
	       "ENABLE-200CC	= %u\r\n"
	       "PERF-MONITOR	= %u\r\n"
	       "CUSTOM-TT	= %u\r\n"
	       "XPFLAGS		= %u\r\n"
	       "BLOCK-TRACK	= %u\r\n"
		,GetLparModeName(CalcCurrentLparMode(lpar,true),export_count>0)
		,lpar->engine[0],lpar->engine[1],lpar->engine[2]
		,lpar->enable_200cc
		,lpar->enable_perfmon
		,lpar->enable_custom_tt
		,lpar->enable_xpflags
		,lpar->block_track
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

    int i, last_group = -1; 
    for ( i = 0; i < BMG_N_CHAT; i++ )
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

	    fprintf(f,"%s\n",PrintChatMode(i,mode1,mode2,export_count));
	}
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    analyse			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetLEAnalyse ( le_analyse_t *ana )
{
    if (ana)
    {
	ResetLEAnalyseUsage(ana);
	memset(ana,0,sizeof(*ana));
	InitializeLPAR(&ana->lpar,false);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetLEAnalyseUsage ( le_analyse_t *ana )
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

enumError AnalyseLEBinary
(
    le_analyse_t	*ana,		// NULL or destination of analysis
    const void		*data,		// data pointer
    uint		data_size	// data size
)
{
    //--- setup analyse

    le_analyse_t ana0 = {0};
    if (!ana)
    {
	ResetLEAnalyse(&ana0);
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
    const uint head_size = ntohl(head->size);
    if ( head_size > data_size )
	return ERR_INVALID_DATA;

    ana->valid		= LE_HEAD_FOUND;
    ana->version	= head->phase;
    ana->data		= data;
    ana->size		= data_size;
    ana->head		= head;
    ana->head_vers	= ntohl(head->version);
    ana->head_size	= head_size;
    ana->end_of_data	= (u8*)data + head_size;

    uint off_param = 0;
    switch(ana->head_vers)
    {
     case 4:
	if ( ana->size > sizeof(le_binary_head_v4_t) )
	{
	    ana->valid |= LE_HEAD_VALID | LE_HEAD_KNOWN;
	    off_param = ntohl(ana->head_v4->off_param);
	}
	break;

     default:
	// at least v4
	if ( ana->head_vers > 4 && ana->size > sizeof(le_binary_head_t) )
	{
	    ana->valid |= LE_HEAD_VALID;
	    off_param = ntohl(head->off_param);
	}
	break;
    }


    //--- analyse parameters (prepare)

    typedef struct ptr_ana_t
    {
	int off_offset;
	int off_ptr;
    }
    ptr_ana_t;

    #undef DEF_TAB
    #define DEF_TAB(s,o,p) { offsetof(s,o), offsetof(le_analyse_t,p) },

    const u8 *ptr_list[8];	// for section-size analysis
    uint n_ptr = 0;		// number of used pointers


    //--- analyse parameters

    if ( off_param
	&& !(off_param&3)
	&& ana->size >= off_param + sizeof(le_binary_param_t)
	&& !memcmp(data+off_param,LE_PARAM_MAGIC,4) )
    {
	le_binary_param_t *param = (le_binary_param_t*)(data+off_param);
	const uint param_size = ntohl(param->size);
	if (   off_param + param_size <= data_size
	    && off_param + param_size <= ana->head_size )
	{
	    ana->valid		|= LE_PARAM_FOUND;
	    ana->param		= param;
	    ana->param_offset	= off_param;
	    ana->param_vers	= ntohl(param->version);
	    ana->param_size	= param_size;

	    const uint max_off = ana->head_size - off_param;

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
			DEF_TAB( le_binpar_v1_35_t, off_flags,		flags )
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
	const int size = ana->data + ana->size - ana->end_of_data;
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
    #define DEF_TAB(p,n,m,s) {	offsetof(le_analyse_t,p), \
				offsetof(le_analyse_t,n), \
				offsetof(le_analyse_t,m), \
				sizeof(*ana->p)*s },

    static const ana_n_max_t n_max_tab[] =
    {
	DEF_TAB( cup_track,	n_cup_track,	max_cup_track,	4 )
	DEF_TAB( cup_arena,	n_cup_arena,	max_cup_arena,	5 )
	DEF_TAB( property,	n_slot,		max_property,	1 )
	DEF_TAB( music,		n_slot,		max_music,	1 )
	DEF_TAB( flags,		n_slot,		max_flags,	1 )
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
	const u8 *end = data + ana->head_size;
	uint i;
	for ( i = 0; i < n_ptr; i++ )
	    if ( ptr_list[i] > ptr && ptr_list[i] < end )
		end = ptr_list[i];

	const uint max = (end-ptr) / pnm->elem_size;
	if ( *(uint*)((u8*)ana+pnm->off_max) > max )
	     *(uint*)((u8*)ana+pnm->off_max) = max;
    }

    CalculateStatsLE(ana);


    //--- return

    return ana->valid & LE_PARAM_VALID ? ERR_OK :
	   ana->valid & LE_HEAD_VALID  ? ERR_MISSING_PARAM
				       : ERR_INVALID_DATA;
}

///////////////////////////////////////////////////////////////////////////////

void CalculateStatsLE ( le_analyse_t *ana )
{
    ResetLEAnalyseUsage(ana);

    ana->used_rslots	= 0;
    ana->max_rslots	= 0;
    ana->used_bslots	= 0;
    ana->max_bslots	= 0;

    ana->max_slot = ana->max_property < ana->max_music
		  ? ana->max_property : ana->max_music;
    if ( ana->max_slot > ana->max_flags )
	 ana->max_slot = ana->max_flags;
    if ( ana->max_slot < LE_FIRST_LOWER_CT_SLOT )
	return;


    //-- count racing slots

    uint used = 0, total = 0, slot;
    for ( slot = 0; slot < MKW_N_TRACKS; slot++ )
    {
	total++;
	if (IsLESlotUsed(ana,slot))
	    used++;
    }

    for ( slot = LE_FIRST_LOWER_CT_SLOT; slot < ana->n_slot; slot++ )
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

const le_usage_t * GetLEUsage ( const le_analyse_t *ana0, bool force_recalc )
{
    le_analyse_t *ana = (le_analyse_t*)ana0;
    if ( force_recalc && ana->usage )
	ResetLEAnalyseUsage(ana);

    if ( ana->usage || ana->max_slot < LE_FIRST_LOWER_CT_SLOT )
	return ana->usage;

    ana->usage_size = ana->n_slot > 0x100 ? ana->n_slot : 0x100;
    le_usage_t *usage = CALLOC(ana->usage_size,sizeof(*usage));
    ana->usage = usage;
    memset(usage,LEU_S_UNUSED,ana->usage_size);


    //-- special slots

    memset(usage+0x36,LEU_S_SPECIAL,5);
    usage[0x42] = LEU_S_SPECIAL;
    usage[0x43] = LEU_S_NETWORK|LEU_F_ONLINE;


    //-- random slots

    memset( usage + LE_FIRST_RANDOM_SLOT,
		LEU_S_WIIMM|LEU_F_ONLINE,
		LE_LAST_RANDOM_SLOT - LE_FIRST_RANDOM_SLOT+1 );


    //-- check racing slots

    uint slot;
    for ( slot = 0; slot < MKW_N_TRACKS; slot++ )
	usage[slot] = IsLESlotUsed(ana,slot)
			? LEU_S_TRACK|LEU_F_ONLINE : LEU_F_ONLINE;

    for ( slot = LE_FIRST_LOWER_CT_SLOT; slot < ana->usage_size; slot++ )
    {
	if (IsLESlotUsed(ana,slot))
	{
	    const le_flags_t flags = ana->flags ? ana->flags[slot] : 0;
	    usage[slot] = flags & LETF_ALIAS     ? LEU_S_ALIAS
			: flags & LETF_RND_HEAD  ? LEU_S_TRACK_RND
			: flags & LETF_RND_GROUP ? LEU_S_TRACK_HIDE
			:			   LEU_S_TRACK;
	}

	if ( ana->version > 1 || slot < 0xff
		|| slot >= LE_FIRST_UPPER_CT_SLOT && slot <= LE_LAST_UPPER_CT_SLOT )
	    usage[slot] |= LEU_F_ONLINE;
    }


    //-- check battle slots

    for ( slot = MKW_ARENA_BEG; slot < MKW_ARENA_END; slot++ )
    {
	if (IsLESlotUsed(ana,slot))
	{
	    const le_flags_t flags = ana->flags ? ana->flags[slot] : 0;
	    usage[slot] = flags & LETF_ALIAS      ? LEU_S_ALIAS
			: flags & LETF_RND_HEAD   ? LEU_S_ARENA_RND
			: flags & LETF_RND_GROUP  ? LEU_S_ARENA_HIDE
			:			    LEU_S_ARENA;
	}
    }


    //--- override Network.random

    if ( ana->version == 1 )
	usage[0xff] = LEU_S_NETWORK|LEU_F_ONLINE;

    return usage;
}

///////////////////////////////////////////////////////////////////////////////

static const char usage_ch[] = "-AraTRtsn*W>----";

char GetLEUsageChar ( le_usage_t usage )
{
    return usage_ch[usage & LEU_S_MASK];
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
	    case LEU_S_UNUSED:		col = colset->reset; break;

	    case LEU_S_ARENA:		col = colset->b_cyan; break;
	    case LEU_S_ARENA_RND:	col = colset->b_yellow; break;
	    case LEU_S_ARENA_HIDE:	col = colset->b_green; break;
	    case LEU_S_TRACK:		col = colset->b_cyan_blue; break;
	    case LEU_S_TRACK_RND:	col = colset->b_yellow; break;
	    case LEU_S_TRACK_HIDE:	col = colset->green_cyan; break;

	    case LEU_S_SPECIAL:		col = colset->reset; break;
	    case LEU_S_NETWORK:		col = colset->b_orange; break;
	    case LEU_S_RANDOM:		col = colset->b_yellow; break;
	    case LEU_S_WIIMM:		col = colset->b_orange; break;
	    case LEU_S_ALIAS:		col = colset->b_red; break;
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
    CHECK_ARENA		// check for battle arenas
}
slot_check_t;

//-----------------------------------------------------------------------------

ccp GetLEValid ( le_valid_t valid )
{
    char *buf = GetCircBuf(7);
    DASSERT(buf);

    buf[0] = valid & LE_HEAD_FOUND  ? 'F' : '-';
    buf[1] = valid & LE_HEAD_VALID  ? 'V' : '-';
    buf[2] = valid & LE_HEAD_KNOWN  ? 'K' : '-';
    buf[3] = valid & LE_PARAM_FOUND ? 'f' : '-';
    buf[4] = valid & LE_PARAM_VALID ? 'v' : '-';
    buf[5] = valid & LE_PARAM_KNOWN ? 'k' : '-';
    buf[6] = 0;
    return buf;
}

//-----------------------------------------------------------------------------

static ccp GetSlotInfo
(
    const le_analyse_t	*ana,		// valid structure
    int			tid,		// track id
    ColorSet_t		*col,		// NULL or color set for warnings
    slot_check_t	check,		// check modus
    uint		*warnings	// not NULL: increment on warnings
)
{
    if ( tid < ana->n_slot )
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
	    warn |= 1;
	    if (col)
	    {
		prop1 = col->hint;
		prop0 = col->reset;
	    }
	}


	//-- music slot

	ccp music1 = "", music0 = "";
	const int music = ana->music ? ana->music[tid] : -1;
	ccp musicx = GetMkwMusicName3(music);
	if ( tid < MKW_ORIGINAL_END || tid >= LE_FIRST_LOWER_CT_SLOT )
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
	    else if (!(music&1))
	    {
		warn |= 1;
		if (col)
		{
		    music1 = col->hint;
		    music0 = col->reset;
		}
	    }
	}


	//-- flags slot

	ccp flags;
	char flags_buf[8];
	bool is_alias = false;
	if (ana->flags)
	{
	    const le_flags_t fl = ana->flags[tid];
	    if ( fl & ~LETF__ALL )
		snprintf(flags_buf,sizeof(flags_buf),"?%03x",fl);
	    else
	    {
		if ( fl & LETF_ALIAS )
		    is_alias = true;

		snprintf(flags_buf,sizeof(flags_buf),"%c%c%c%c",
			fl & LETF_NEW		? 'N' : '-',
			fl & LETF_RND_HEAD	? 'H' : '-',
			fl & LETF_RND_GROUP	? 'G' : '-',
			fl & LETF_ALIAS		? 'A' : '-' );
	    }
	    flags = flags_buf;
	}
	else
	    flags = "-?-";

	static const char warn_list[] = " <!!";
	char buf[40];
	const int len = is_alias
		? snprintf(buf,sizeof(buf),"[%s->%4x %s,%s] ",
				col->differ, TrackByAliasLE(prop,music),
				col->reset, flags )
		: snprintf(buf,sizeof(buf),"[%s%s%s,%s%s%s,%s]%c",
				prop1, propx, prop0,
				music1, musicx, music0,
				flags, warn_list[warn] );
	if ( len > 0 )
	    return CopyCircBuf0(buf,len);
    }

    if (col)
    {
	static char fail[40] = {0};
	if (!*fail)
	    snprintf(fail,sizeof(fail),"[%s---,---,----%s]!",col->bad,col->reset);
	return fail;
    }
    return "[---,---,---]!";
}

//-----------------------------------------------------------------------------

static le_cup_track_t * DumpLECup
(
    FILE		*f,		// print file
    const le_analyse_t	*ana,		// valid structure
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
	fprintf(f," %3x%s",tid,GetSlotInfo(ana,tid,col,check,warnings));

	if ( done && tid < ana->n_slot )
	    done[tid]++;
    }
    return (le_cup_track_t*)cp;
}

//-----------------------------------------------------------------------------

static void DumpLETracks
	( FILE *f, uint indent, ColorSet_t *col, const le_analyse_t *ana )
{
    DASSERT(f);
    DASSERT(indent>=2);
    DASSERT(col);
    DASSERT(ana);

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

    static const char id_info[] = "id[prop,music,new+head+group+alias]";


    //--- battle cups

    if ( ana->n_cup_track && ana->cup_track )
    {
	fprintf(f,"\n%*s%s" "%u cup%s with battle tracks (%s):%s\n",
		indent-2,"", col->heading,
		ana->n_cup_arena, ana->n_cup_arena == 1 ? "" : "s",
		id_info, col->reset );

	uint cup, hidden = 0;
	const le_cup_track_t *cp = ana->cup_arena;
	for ( cup = 0; cup < ana->n_cup_arena; cup++ )
	{
	    if ( cp[0] || cp[1] || cp[2] || cp[3] || cp[4] )
	    {
		fprintf(f,"%*s" "Cup %2u:",indent,"",cup);
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
	fprintf(f,"\n%*s%s" "%u cup%s with racing tracks (%s):%s\n",
		indent-2,"", col->heading,
		ana->n_cup_track, ana->n_cup_track == 1 ? "" : "s",
		id_info, col->reset );

	uint cup, hidden = 0;
	const le_cup_track_t *cp = ana->cup_track;
	for ( cup = 0; cup < ana->n_cup_track; cup++ )
	{
	    if ( cp[0] || cp[1] || cp[2] || cp[3] )
	    {
		fprintf(f,"%*s" "Cup %2u:",indent,"",cup);
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

    uint i, max_used_slot = 0, n_multi = 0, n_group = 0, n_unused = 0;
    for ( i = 0; i < ana->n_slot; i++ )
	if (IsRacingTrackLE(i))
	{
	    if (done[i])
	    {
		max_used_slot = i;
		if ( done[i] > 1 )
		     n_multi++;
	    }
	    else if ( !done[i] )
	    {
		if ( ana->flags && ana->flags[i] & LETF_RND_GROUP )
		    n_group++;
		else
		    n_unused++;
	    }
	}
    PRINT0("m=%d, g=%d, u=%d /%d\n",n_multi,n_group,n_unused,max_used_slot);


    //--- multi used tracks

    if (n_multi)
    {
	fprintf(f,"\n%*s%s" "%u multiple used track slot%s (%s):%s\n",
		indent-2,"", col->heading,
		n_multi, n_multi == 1 ? "" : "s",
		id_info, col->reset );

	for ( i = 0; i < ana->n_slot; i++ )
	    if ( done[i] > 1 && IsRacingTrackLE(i) )
		fprintf(f,"%*sSlot %4d/dec : %3x%s : %3ux\n",
			indent,"", i, i, GetSlotInfo(ana,i,col,CHECK_OFF,0), done[i]);
    }


    //--- random group tracks

    if (n_group)
    {
	fprintf(f,"\n%*s%s" "%u track slot%s reserved for random groups (%s):%s\n",
		indent-2,"", col->heading,
		n_group, n_group == 1 ? "" : "s",
		id_info, col->reset );

	for ( i = 0; i < ana->n_slot; i++ )
	    if ( !done[i] && IsRacingTrackLE(i) && ana->flags[i] & LETF_RND_GROUP )
	    {
		done[i]++;
		fprintf(f,"%*sSlot %4d/dec : %3x%s\n",
			indent,"", i, i, GetSlotInfo(ana,i,col,CHECK_OFF,0));
	    }
    }


    //--- unused used track slots

    if ( n_unused)
    {
	fprintf(f,"\n%*s%s" "%u unused track slot%s (%s):%s\n",
		indent-2,"", col->heading,
		n_unused, n_unused == 1 ? "" : "s",
		id_info, col->reset );
	for ( i = 0; i <= max_used_slot; i++ )
	{
	    if ( !done[i] && IsRacingTrackLE(i) )
		fprintf(f,"%*sSlot %4d/dec : %3x%s\n",
			indent,"", i, i, GetSlotInfo(ana,i,col,CHECK_OFF,0) );
	}


	uint first_unused_slot = max_used_slot + 1;
	if (!IsRacingTrackLE(first_unused_slot))
	    first_unused_slot = LE_FIRST_LOWER_CT_SLOT;
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
	( FILE *f, uint indent, ColorSet_t *col, const le_analyse_t *ana )
{
    DASSERT(f);
    DASSERT(indent>=2);
    DASSERT(col);
    DASSERT(ana);

    const le_usage_t *usage = GetLEUsage(ana,false);
    if (!usage)
	return;

    fprintf(f,"\n%*s%s"	"Slot usage map for %u slots%s\n"
		"%*s"	"%s%s=Arena, %s%s=Random Arena, %s%s=Hidden Arena,  "
			"%s%s=Wiimm Cup (random), %s%s=Alias,%s\n"
		"%*s"	"%s%s=Track, %s%s=Random Track, %s%s=Hidden Track,  "
			"%s%s=Network, %s%s=Special, %s%s=unused.%s\n",
		indent-2,"", col->heading, ana->n_slot, col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA,		col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_RND,	col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ARENA_HIDE,	col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_WIIMM,		col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_ALIAS,		col,0 ), col->info,
		col->reset,
		indent,"",
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK,		col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_RND,	col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_TRACK_HIDE,	col,0 ), col->info,
		 GetLEUsageCharCol( LEU_F_ONLINE | LEU_S_NETWORK,	col,0 ), col->info,
		 GetLEUsageCharCol(		   LEU_S_SPECIAL,	col,0 ), col->info,
		 GetLEUsageCharCol(		   LEU_S_UNUSED,	col,0 ), col->info,
		col->reset );

    uint slot;
    ccp prev_col = 0;
    for ( slot = 0; slot < ana->usage_size; slot++ )
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

void DumpLEAnalyse ( FILE *f, uint indent, const le_analyse_t *ana )
{
    DASSERT(f);
    DASSERT(ana);
    indent = NormalizeIndent(indent);

    ColorSet_t col;
    SetupColorSet(&col,f);

    fprintf(f,"%*s%sLE-CODE binary, valid=%s, file size: %x/hex = %u%s\n",
		indent,"",
		col.caption, GetLEValid(ana->valid), ana->size, ana->size,
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
		"%*s" "Version:           %u\n"
		"%*s" "Build number:      %u\n"
		"%*s" "Base address:      %8x/hex\n"
		"%*s" "Entry point:       %8x/hex\n"
		"%*s" "Total size:        %8x/hex = %u\n"
		"%*s" "Offset param:      %8x/hex\n"
		"%*s" "Region code:       %c  (%s)\n"
		"%*s" "Debug flag:        %c  (%s)\n"
		"%*s" "LE version/phase:  %u\n"
		,indent-2,"", col.heading, col.reset
		,indent,"", h->magic
		,indent,"", ntohl(h->version)
		,indent,"", ntohl(h->build_number)
		,indent,"", ntohl(h->base_address)
		,indent,"", ntohl(h->entry_point)
		,indent,"", ntohl(h->size), ntohl(h->size)
		,indent,"", ntohl(h->off_param)
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
		,indent,"", h->phase
		);
    }

    switch (ana->head_vers)
    {
     case 4:
      {
	const le_binary_head_v4_t *h = ana->head_v4;
	DASSERT(h);
	fprintf(f,
		"%*s" "Timestamp:         %s\n"
		,indent,"", h->timestamp
		);
      }
      break;

      default:
	fprintf(f,"%*s" ">>> other parameters unknown!\n",indent,"");
	break;
    }


    //--- parameters

    if ( ana->valid & LE_PARAM_VALID )
    {
	const le_binary_param_t *h = ana->param;
	DASSERT(h);
	fprintf(f,
		"\n%*s%s" "Parameters:%s\n"
		"%*s" "Magic:             %.4s\n"
		"%*s" "Version:           %u\n"
		"%*s" "Param size:        %x/hex = %u\n"
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
	fprintf(f,
		"%*s" "Engine chances:    "
			"%u + %u + %u = %u (1%s0cc,mirror)\n"
		"%*s" "200cc:             %u (%sabled)\n"
		"%*s" "Perform. monitor:  %u (%s)\n"
		,indent,""
			,h->engine[0], h->engine[1], h->engine[2]
			,h->engine[0] + h->engine[1] + h->engine[2]
			,h->enable_200cc ? "50cc,20" : "00cc,15"
		,indent,"", h->enable_200cc, h->enable_200cc ? "en" :"dis"
		,indent,"", h->enable_perfmon,
			h->enable_perfmon > 1 ? "Wii+Dolphin"
			: h->enable_perfmon ? "Wii only" :"disabled"
		);

	if ( ana->param_size >= sizeof(le_binpar_v1_37_t)  )
	    fprintf(f,
		"%*s" "Custom Timetrial:  %u (%sabled)\n"
		"%*s" "Extended P-Flags:  %u (%sabled)\n"
		"%*s" "Block tracks for:  %u races\n"
		,indent,"", h->enable_custom_tt, h->enable_custom_tt ? "en" :"dis"
		,indent,"", h->enable_xpflags, h->enable_xpflags ? "en" :"dis"
		,indent,"", h->block_track
		);

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
		"%*s" "Flags list:        %5x/hex, %4u / %4u%s\n",
		indent,"", PTR_DISTANCE_INT(ana->flags,ana->data),
			ana->n_slot, ana->max_flags,
			ana->n_slot > ana->max_flags ? fail : "" );

    if ( ana->end_of_data )
	fprintf(f,
		"%*s" "End of data:       %5x/hex\n",
		indent,"", PTR_DISTANCE_INT(ana->end_of_data,ana->data) );

    fprintf(f,
		"%*s" "End of file:       %5x/hex\n",
		indent,"", ana->size );


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

void CopyTrackFile ( uint tid, ccp name )
{
    if ( opt_track_dest
	&& IsDirectory(opt_track_dest,false)
	&& opt_track_source.used )
    {
	char src[PATH_MAX];
	char dest[PATH_MAX];

	ParamFieldItem_t *ptr = opt_track_source.field, *end;
	for ( end = ptr + opt_track_source.used; ptr < end; ptr++ )
	{
	    PathCatBufPPE(src,sizeof(src),ptr->key,name,".szs");

	    struct stat st;
	    if ( !stat(src,&st) && S_ISREG(st.st_mode) )
	    {
		snprintf(dest,sizeof(dest),"%s/%03x.szs",opt_track_dest,tid);
		if (TransferFile(src,dest,ptr->num,0666))
		{
		    PathCatBufPPE(src,sizeof(src),ptr->key,name,"_d.szs");
		    if ( !stat(src,&st) && S_ISREG(st.st_mode) )
		    {
			snprintf(dest,sizeof(dest),"%s/%03x_d.szs",opt_track_dest,tid);
			TransferFile(src,dest,ptr->num,0666);
		    }
		}
		break;
	    }
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

    if ( !le_file_done && opt_le_define && *opt_le_define )
    {
	PRINT("le=%d, ctm=%d[%s]\n",
			le_phase, ct_mode, GetCtModeNameBMG(ct_mode,true) );
	if ( logging > 1 )
	    fprintf(stdlog,"Load LE definition for phase %d (%s)\n",
			le_phase, GetCtModeNameBMG(ct_mode,true) );

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

enumError ApplyLEFile ( le_analyse_t * ana )
{
    DASSERT(ana);
    const ctcode_t *ctcode = LoadLEFile(ana->version);
    return ctcode ? ApplyCTCODE(ana,ctcode) : ERR_ERROR;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ApplyCTCODE()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ApplyCTCODE ( le_analyse_t * ana, const ctcode_t * ctcode )
{
    DASSERT(ana);
    DASSERT(ctcode);

    //--- setup

    ana->n_cup_track	= 0;
    ana->n_cup_arena	= 0;
    ana->n_slot		= 0;

    le_property_t *prop  = ana->property;
    le_music_t    *music = ana->music;
    le_flags_t	  *flags = ana->flags;

    if ( prop && music && flags )
    {
	const uint max	= ana->max_slot < LE_FIRST_LOWER_CT_SLOT
			? ana->max_slot : LE_FIRST_LOWER_CT_SLOT;
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
	     max = ana->max_cup_track ;

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

    switch (ana->param_vers)
    {
     case 1:
	ana->cup_par->n_racing_cups = htonl(ana->n_cup_track);
	ana->cup_par->n_battle_cups = htonl(ana->n_cup_arena);
	ana->course_par->n_slot = htonl(ana->n_slot);
	break;
    }


    //--- copy/move/link track files

    char buf[500];
    const bmg_item_t * bi = ctcode->track_file.item;
    const bmg_item_t * bi_end = bi + ctcode->track_file.item_used;
    for ( ; bi < bi_end; bi++ )
    {
	PrintString16BMG( buf, sizeof(buf), bi->text, bi->len,
					BMG_UTF8_MAX, 0, true );
	CopyTrackFile(bi->mid - ctcode->ctb.track_name1.beg,buf);
    }

    CalculateStatsLE(ana);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool DefineAliasLE ( le_analyse_t * ana, uint slot, uint alias )
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
		|| alias >= MKW_ARENA_BEG && alias < LE_FIRST_LOWER_CT_SLOT )
	{
	    printf("ALIAS: Invalid alias: %x = %x\n",slot,alias);
	    return false;
	}

	const le_flags_t fl = ana->flags[alias];
	if ( !(fl & LETF_ALIAS) )
	{
	    ana->flags[slot]   |= LETF_ALIAS;
	    ana->property[slot] = alias >> 8;
	    ana->music[slot]    = alias & 0xff;
	    return true;
	}

	alias = TrackByAliasLE(ana->property[alias],ana->music[alias]);
    }
}

///////////////////////////////////////////////////////////////////////////////

uint PatchAliasLE ( le_analyse_t * ana, ccp list )
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

void PatchLECODE ( le_analyse_t * ana )
{
    DASSERT(ana);
    PRINT("PatchLECODE() opt_lpar=%s)\n",opt_lpar);

    if (opt_lpar)
	LoadLPAR(&ana->lpar,false,0,false);

    switch (ana->param_vers)
    {
     case 1:
	{
	    if (opt_le_alias)
		PatchAliasLE(ana,opt_le_alias);

	    le_lpar_t *lp = &ana->lpar;
	    if (opt_engine_valid)
		memcpy(lp->engine,opt_engine,sizeof(lp->engine));
	    if ( opt_200cc != OFFON_AUTO )
		lp->enable_200cc = opt_200cc >= OFFON_ON;
	    if ( opt_perfmon != OFFON_AUTO )
		lp->enable_perfmon = opt_perfmon >= OFFON_FORCE ? 2
					: opt_perfmon >= OFFON_ON;
	    if ( opt_custom_tt != OFFON_AUTO )
		lp->enable_custom_tt = opt_custom_tt >= OFFON_ON;
	    if ( opt_xpflags != OFFON_AUTO )
		lp->enable_xpflags = opt_xpflags >= OFFON_ON;
	}
	break;
    }

    CopyLPAR2Data(ana);
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

    enum { C_PARAM, C_CHAT, C_END };
    static const KeywordTab_t section_name[] =
    {
	{ C_PARAM,	"LECODE-PARAMETERS",		0, 0 },
	{ C_CHAT,	"CHAT-MESSAGE-MODES",		0, 0 },
	{ C_END,	"END",				0, 0 },
	{0,0,0,0}
    };

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,fname,0);
    si.predef = SetupVarsLEX();

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

		default:
		    // ignore all other section without any warnings
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

///////////////////////////////////////////////////////////////////////////////

enumError ScanTextLPAR_PARAM
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
	{ "ENGINE",		SPM_U8,	lpar->engine, 3,3 },
	{ "ENABLE-200CC",	SPM_U8,	&lpar->enable_200cc },
	{ "PERF-MONITOR",	SPM_U8,	&lpar->enable_perfmon },
	{ "CUSTOM-TT",		SPM_S8,	&lpar->enable_custom_tt },
	{ "XPFLAGS",		SPM_U8,	&lpar->enable_xpflags },
	{ "BLOCK-TRACK",	SPM_U8,	&lpar->block_track },
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

enumError ScanTextLPAR_CHAT
(
    le_lpar_t		*lpar,		// LE-CODE parameters
    ScanInfo_t		* si,		// valid data
    bool		is_pass2	// false: pass1; true: pass2
)
{
    PRINT(">> ScanTextLPAR_CHAT(pass=%u)\n",is_pass2+1);
    // [[2do]] ???


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
	case LEXS_SET1:		return 1;
	case LEXS_TEST:		return 9;
	default:		return 2;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void UpdateHaveLex ( lex_t *lex )
{
    DASSERT(lex);
    uint have_lex = 0;

    uint i;
    lex_item_t **ss;
    for ( i = 0, ss = lex->item; i < lex->item_used; i++, ss++ )
    {
	const u32 magic = ntohl((*ss)->elem.magic);
	const uint size = ntohl((*ss)->elem.size);
	uint j;
	for ( j = 0; j < HAVELEX__N; j++ )
	    if ( magic == have_lex_info[j].magic && size >= have_lex_info[j].min_size )
	    {
		have_lex |= 1 << j;
		break;
	    }
    }

    lex->have_lex = have_lex;
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


    //--- add have_lex

    uint i;
    for ( i = 0; i < HAVELEX__N; i++ )
	if ( magic == have_lex_info[i].magic && size >= have_lex_info[i].min_size )
	{
	    lex->have_lex |= 1 << i;
	    break;
	}


    //--- check if already exists

    lex_item_t **ss;
    const u32 be_magic = htonl(magic);
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
	case LEXS_SET1:
	    fix_func = FixLEXItem_SET1;
	    if ( size4 < sizeof(lex_set1_t) )
		size4 = ALIGN32(sizeof(lex_set1_t),4);
	    break;

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
///////////////			have lex support		///////////////
///////////////////////////////////////////////////////////////////////////////
// order is important, compare have_lex_t and ct.wiimm.de

const have_lex_info_t have_lex_info[HAVELEX__N] =
{
    { LEXS_SET1, "set1",	1 },
    { LEXS_CANN, "cannon",	0 },
    { LEXS_TEST, "test",	1 },
    { LEXS_HIPT, "hidepos",	sizeof(lex_hipt_rule_t) },
    //--- add new sections here (order is important)
};

///////////////////////////////////////////////////////////////////////////////

ccp CreateSectionInfoLEX
	( have_lex_t special, bool add_value, ccp return_if_empty )
{
    static char buf[500];
    char *dest = buf;

    if (add_value)
	dest = snprintfE( dest, buf+sizeof(buf), "%u=" , special );

    uint i, mask;
    ccp sep = "";
    for ( i = 0, mask = 1; i < HAVELEX__N; i++, mask <<= 1 )
	if ( special & mask )
	{
	    dest = StringCat2E(dest,buf+sizeof(buf),sep,have_lex_info[i].name);
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
    }

    if (add_missing)
    {
	//--- SET1
	AppendSet1LEX(lex,false);

	//--- CANN
	AppendCannLEX(lex,false);

	//--- HIPT
	AppendHiptLEX(lex,false);
    }

    if (add_test) //--- TEST
	AppendTestLEX(lex,false);

    SortItemsLEX(lex);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendSet1LEX ( lex_t * lex, bool overwrite )
{
    lex_set1_t set1;
    memset(&set1,0,sizeof(set1));
    // AppendElementLE() do call FixLEXItem_SET1()
    return AppendElementLEX(lex,LEXS_SET1,&set1,sizeof(set1),false);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendCannLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_CANN, CannonDataLEX,
				sizeof(CannonDataLEX), overwrite);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendHiptLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_HIPT, 0, 0, overwrite);
}

//-----------------------------------------------------------------------------

lex_item_t * AppendTestLEX ( lex_t * lex, bool overwrite )
{
    return AppendElementLEX( lex, LEXS_TEST, TestDataLEX,
				sizeof(TestDataLEX), overwrite);
}

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
    static VarMap_t vm = {0};
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
	    if (logging>1)
		DumpElementsLEX(stdout,lex,logging>2);
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
		"Invalid LEX file: %s\n"
		"Add option --lex=force or --force to ignore some validity checks.",
		lex->fname ? lex->fname : "?" );
    }

    lex->fform = FF_LEX;
    enumError err = ScanLexFile( data, data_size,
			dump_mode ? DumpLEXFunc : ScanLEXFunc, lex, dump_mode );
    if (err)
	ERROR0(err,"Scanning of LEX file failed: %s\n",lex->fname);
    else
	UpdateLEX(lex,opt_complete,false);

    if (logging>0)
	DumpElementsLEX(stdout,lex,logging>2);
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
    LID_TEST,
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
    { LID_TEST,		"TEST",		"Test case",			0 },

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

    if ( magic != LEXS_IGNORE || magic != LEXS_TERMINATE )
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

    lex_set1_t set1;
    memset(&set1,0,sizeof(set1));

    const ScanParam_t ptab[] =
    {
	{ "ITEM-POS-FACTOR",	SPM_FLOAT3_BE,	&set1.item_factor },
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

    PatchLEX(lex);
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

    fprintf(f,text_lex_elem_set1_cr
		,item->sort_order
		,bef4(&set1->item_factor.x)
		,bef4(&set1->item_factor.y)
		,bef4(&set1->item_factor.z)
		);

    if ( size > sizeof(lex_set1_t) )
    {
	fputs("\n# Hex dump for unknown settings:\n",f);
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

    int i, last_lap = 1000;
    for ( i = 0; i < n; i++, rule++ )
    {
	if ( last_lap != rule->lap )
	{
	    last_lap = rule->lap;
	    fputs("\r\n",f);
	}

	fprintf(f,"%5u %4d %4u %4u %4u\r\n",
		rule->cond, rule->lap, rule->from, rule->to, rule->mode );
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
	fputs("\n# Hex dump for unknown settings:\n",f);
	HexDump(f,0,sizeof(lex_test_t),5,16,
		item->elem.data + sizeof(lex_test_t),
		size - sizeof(lex_test_t) );
    }

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
		tool_name, SYSTEM, VERSION, REVISION_NUM, DATE );
    }
    else
    {
	fprintf(F.f, text_lex_setup_cr,
		tool_name, SYSTEM, VERSION, REVISION_NUM, DATE );

	uint i;
	for ( i = 0; i < lex->item_used; i++ )
	{
	    lex_item_t *s = lex->item[i];
	    const uint size = ntohl(s->elem.size);
	    fprintf(F.f,"#   %-4.4s %08x, 0x%04x bytes\r\n",
		    PrintID(&s->elem.magic,4,0), s->elem.magic, size );
	}
    }

    enumError max_err = ERR_OK;

    uint i;
    for ( i = 0; i < lex->item_used; i++ )
    {
	lex_item_t *s = lex->item[i];
	const uint size = ntohl(s->elem.size);
	const u32 magic = ntohl(s->elem.magic);
	enumError err = ERR_OK;
	switch(magic)
	{
	    case LEXS_SET1:
		err = SaveTextLEX_SET1(F.f,lex,s);
		break;

	    case LEXS_CANN:
		err = SaveTextLEX_CANN(F.f,lex,s);
		break;

	    case LEXS_HIPT:
		err = SaveTextLEX_HIPT(F.f,lex,s);
		break;

	    case LEXS_TEST:
		err = SaveTextLEX_TEST(F.f,lex,s);
		break;

	    default:
		hexdump_eol = "\r\n";
		fprintf(F.f,text_lex_hexdump_cr,
			s->sort_order, magic, PrintID(&s->elem.magic,4,0) );
		HexDump(F.f,0,0,5,16,s->elem.data,size);
		break;
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

void SetupLexInfo ( lex_info_t *info, const lex_t *lex )
{
    DASSERT(info);
    InitializeLexInfo(info);

    if (lex)
    {
	info->lex_found = true;
	lex_item_t *item;

	item = FindElementLEX(lex,LEXS_SET1);
	if (item)
	{
	    info->set1_found = true;
	    memcpy(&info->set1,item->elem.data,sizeof(info->set1));
	    bef4n(info->set1.item_factor.v,info->set1.item_factor.v,3);
	}

	item = FindElementLEX(lex,LEXS_TEST);
	if (item)
	{
	    info->test_found = true;
	    memcpy(&info->test,item->elem.data,sizeof(info->test));
	}
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

bool PatchLEX ( lex_t * lex )
{
    DASSERT(lex);

    bool modified = false;
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
	PRINT("PATCH+ LEX/TEST: had=%d, empty=%d, modified=%d => have_lex:%x\n",
		had_test, empty, modified, lex->have_lex );
	if ( empty && !force_lex_test )
	{
	    if ( RemoveElementLEX(lex,li) && had_test )
		modified = true;
	}
	else if (!had_test)
	    modified = true;

	PRINT("PATCH- LEX/TEST: had=%d, empty=%d, modified=%d => have_lex:%x\n",
		had_test, empty, modified, lex->have_lex );

	if (modified)
	    lex->modified = true;
    }

    if (opt_lex_purge)
	modified |= PurgeLEX(lex);

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
	 case LEXS_SET1:
	    {
		lex_set1_t set1;
		memset(&set1,0,sizeof(set1));
		FixLEXElement_SET1(&set1);
		remove = !memcmp(data,&set1,sizeof(set1));
	    }
	    break;

	 case LEXS_CANN:
	    remove = size <= sizeof(CannonDataLEX)
		  && !memcmp(data,&CannonDataLEX,size);
	    break;

	 case LEXS_HIPT:
	    remove = size < sizeof(lex_hipt_rule_t);
	    break;

	 case LEXS_TEST:
	    remove = !memcmp(data,TestDataLEX,sizeof(TestDataLEX));
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


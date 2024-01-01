
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

#include <stddef.h>

#include "dclib-xdump.h"
#include "lib-szs.h"
#include "lib-common.h"
#include "lib-numeric.h"
#include "lib-bzip2.h"
#include "dclib-numeric.h"

#include "minigame.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    data			///////////////
///////////////////////////////////////////////////////////////////////////////

int opt_kmg_arena = -1;
int opt_kmg_limit = -1;

//-----------------------------------------------------------------------------

enum
{
	HEAD_PARAM	= 0,
	BALLOON_PARAM	= 1,
	COIN_PARAM	= 2,
};

//-----------------------------------------------------------------------------

static ccp minigame_param_title[] =
{
	"Parameters of file header (16 bit)",
	"Parameters for balloon battle (16 bit, offsets 0x1c8..0x1da)",
	"Parameters for coin runners (16 bit, offsets 0x704..0x716)",
};

//-----------------------------------------------------------------------------

static const KeywordTab_t minigame_param_list[] =
{
    { 0x008, "OFFSET-008", 0,				HEAD_PARAM },
    { 0x00a, "OFFSET-00A", 0,				HEAD_PARAM },

    { 0x1c8, "OFFSET-1C8", "POINTS-IF-HIT-OTHER",	BALLOON_PARAM },
    { 0x1ca, "OFFSET-1CA", "POINTS-IF-DEFEAT",		BALLOON_PARAM },
    { 0x1cc, "OFFSET-1CC", "POINTS-IF-BALLOONS-LOST",	BALLOON_PARAM },
	// [[obsolete]], typo "BALLON" fixed in 2020-01
	{ 0x1cc, "POINTS-IF-BALLONS-LOST", 0,		BALLOON_PARAM },
    { 0x1ce, "OFFSET-1CE", "POINTS-IF-HIT",		BALLOON_PARAM },
    { 0x1d0, "OFFSET-1D0", "POINTS-HITTING-TEAMMATE",	BALLOON_PARAM },
    { 0x1d2, "OFFSET-1D2", "POINTS-IF-HIT-BY-TEAMMATE",	BALLOON_PARAM },
    { 0x1d4, "OFFSET-1D4", 0,				BALLOON_PARAM },
    { 0x1d6, "OFFSET-1D6", 0,				BALLOON_PARAM },
    { 0x1d8, "OFFSET-1D8", 0,				BALLOON_PARAM },
    { 0x1da, "OFFSET-1DA", 0,				BALLOON_PARAM },

    { 0x704, "OFFSET-704", 0,				COIN_PARAM },
    { 0x706, "OFFSET-706", 0,				COIN_PARAM },
    { 0x708, "OFFSET-708", 0,				COIN_PARAM },
    { 0x70a, "OFFSET-70A", 0,				COIN_PARAM },
    { 0x70c, "OFFSET-70C", 0,				COIN_PARAM },
    { 0x70e, "OFFSET-70E", 0,				COIN_PARAM },
    { 0x710, "OFFSET-710", 0,				COIN_PARAM },
    { 0x712, "OFFSET-712", "MIN-LOST-FALLDOWN",		COIN_PARAM },
    { 0x714, "OFFSET-714", "MIN-LOST-IF-HIT",		COIN_PARAM },
    { 0x716, "OFFSET-716", "LOST-PERCENTAGE",		COIN_PARAM },

    { 0,0,0,0 }
};

//-----------------------------------------------------------------------------

static const KeywordTab_t minigame_table_list[] =
{
    { 0x010, "OFFSET-010", "BALLOON-DURATION",	BALLOON_PARAM },
	// [[obsolete]], typo "BALLON" fixed in 2020-01
	{ 0x010, "BALLON-DURATION", 0,		BALLOON_PARAM },
    { 0x0ec, "OFFSET-0EC", 0,			BALLOON_PARAM },

    { 0x1dc, "OFFSET-1DC", "COIN-DURATION",	COIN_PARAM },
    { 0x2b8, "OFFSET-2B8", 0,			COIN_PARAM },
    { 0x394, "OFFSET-394", "START-COINS",	COIN_PARAM },
    { 0x470, "OFFSET-470", "MAX-COINS",		COIN_PARAM },
    { 0x54c, "OFFSET-54C", 0,			COIN_PARAM },
    { 0x628, "OFFSET-628", 0,			COIN_PARAM },

    { 0,0,0,0 }
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MINIGAME setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeMINIGAME ( minigame_t * minigame, file_format_t fform )
{
    DASSERT(minigame);
    memset(minigame,0,sizeof(*minigame));
    minigame->fname = EmptyString;
    minigame->fform_source = fform;
    SetupBZ2MgrMINIGAME(minigame,true);
}

///////////////////////////////////////////////////////////////////////////////

void ResetMINIGAME ( minigame_t * minigame )
{
    DASSERT(minigame);
    const typeof(minigame->fform_source) fform = minigame->fform_source;
    FreeString(minigame->fname);
    InitializeMINIGAME(minigame,fform);
}

///////////////////////////////////////////////////////////////////////////////

bool GetOriginalMINIGAME ( minigame_kmg_t *dest )
{
    DASSERT(dest);
    BZ2Manager_t *bz = GetCommonBZ2Manager(FF_KMG);
    DASSERT(bz);
    DASSERT( bz->size == sizeof(*dest) );
    if ( bz && bz->size == sizeof(*dest) )
    {
	memcpy(dest,bz->data,sizeof(*dest));
	return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static void limit_duration ( be16_t *dest )
{
    DASSERT(dest);

    const u16 limith = opt_kmg_limit;
    const u16 limitn = htons(limith);

    u16 *end;
    for ( end = dest + 10*11; dest < end; dest++ )
	if ( ntohs(*dest) > limith )
	    *dest = limitn;
}

//-----------------------------------------------------------------------------

static void dup_slot ( be16_t dest[10][11] )
{
    const be16_t *src = dest[opt_kmg_arena];
    uint slot;
    for ( slot = 0; slot < 10; slot++ )
	if ( slot != opt_kmg_arena )
	    memcpy(dest[slot],src,sizeof(dest[slot]));
}

//-----------------------------------------------------------------------------

static void PatchMINIGAME ( minigame_kmg_t *dest )
{
    DASSERT(dest);

    if ( opt_kmg_limit > 0 )
    {
	limit_duration(dest->dur_balloon[0]);
	limit_duration(dest->dur_coin[0]);
    }

    if ( opt_kmg_arena >= 0 && opt_kmg_arena < 10 )
    {
	dup_slot(dest->dur_balloon);
	dup_slot(dest->off_0ec);

	dup_slot(dest->dur_coin);
	dup_slot(dest->off_2b8);
	dup_slot(dest->start_coins);
	dup_slot(dest->max_coins);
	dup_slot(dest->off_5c4);
	dup_slot(dest->off_628);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SetupBZ2MgrMINIGAME ( minigame_t * minigame, bool reload_bin )
{
    DASSERT(minigame);
    if ( reload_bin || memcmp(minigame->data.head.magic,KMG_MAGIC,4) )
    {
	GetOriginalMINIGAME(&minigame->data);
	PatchMINIGAME(&minigame->data);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawMINIGAME()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(minigame);
    if (init_minigame)
	InitializeMINIGAME(minigame,FF_KMG);

    if ( !data || data_size != sizeof(minigame->data) )
    {
	SetupBZ2MgrMINIGAME(minigame,true);
	return ERROR0(ERR_INVALID_DATA,
		"Invalid file size (%u but not %zu bytes: %s\n",
		data_size, sizeof(minigame->data), minigame->fname );
    }
    memcpy(&minigame->data,data,sizeof(minigame->data));
    PatchMINIGAME(&minigame->data);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		ScanTextMINIGAME() helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    MINIGAME_IGNORE,
    MINIGAME_SETUP,
    MINIGAME_PARAMETERS,
};

//-----------------------------------------------------------------------------

static const KeywordTab_t minigame_section_name[] =
{
	{ MINIGAME_IGNORE,	"END",		0, 0 },
	{ MINIGAME_SETUP,	"SETUP",	0, 0 },
	{ MINIGAME_PARAMETERS,	"PARAMETERS",	0, 0 },
	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextSETUP
(
    minigame_t		* minigame,	// MINIGAME data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(minigame);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

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
	    ScanU32SI(si,(u32*)&minigame->revision,1,0);
	    si->init_revision = si->cur_file->revision = minigame->revision;
	    DefineIntVar(&si->gvar,"REVISION$SETUP",minigame->revision);
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",minigame->revision);
	}
	else
	    GotoEolSI(si);
	CheckEolSI(si);
    }

    CheckLevelSI(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextPARAMETERS
(
    minigame_t		* minigame,	// MINIGAME data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(minigame);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	    si->cur_file->ptr++;

	char name[50];
	if ( !ScanNameSI(si,name,sizeof(name),true,true,0)
		|| NextCharSI(si,true) != '=' )
	{
	    CheckEolSI(si);
	    continue;
	}
	DASSERT(si->cur_file);
	si->cur_file->ptr++;
	PRINT0("### %s = %.4s...\n",name,si->cur_file->ptr);

	if (!strcmp(name,"REVISION"))
	{
	    u32 rev;
	    ScanU32SI(si,&rev,1,0);
	    si->cur_file->revision = rev;
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",rev);
	}
	else
	{
	    int abbrev_count;
	    const KeywordTab_t *par = ScanKeyword(&abbrev_count,name,minigame_param_list);
	    if ( !par || abbrev_count )
	    {
		// WARNING
		GotoEolSI(si);
		continue;
	    }

	    const uint offset = par->id;
	    u16 *dest = (u16*)( (u8*)&minigame->data + offset );
	    ScanBE16SI(si,dest,1,0);
	}
	CheckEolSI(si);
    }

    CheckLevelSI(si);
    return ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextTABLE
(
    minigame_t		* minigame,	// MINIGAME data structure
    ScanInfo_t		* si,		// valid data
    const KeywordTab_t	* cmd		// valid section pointer
)
{
    DASSERT(minigame);
    DASSERT(si);
    DASSERT(cmd);

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,0);
	    continue;
	}

	ScanFile_t *sf = si->cur_file;

	long slot;
	enumError err = ScanUValueSI(si,&slot,0);
	if (err)
	    break;
	if ( slot >= 10 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    if ( si->no_warn <= 0 )
		ERROR0(ERR_WARNING,
			"Slot must be between 0 and 9: [%s @%u]: %.*s\n",
			sf->name, sf->line, (int)(eol-sf->ptr), sf->ptr );
	    sf->ptr = eol;
	    continue;
	}

	u16 *dest = (u16*)( (u8*)&minigame->data + cmd->id ) + slot * 11;
	uint idx;
	for ( idx = 0; idx < 11; idx++ )
	{
	    ch = NextCharSI(si,false);
	    if (!ch)
		break;

	    long val;
	    err = ScanUValueSI(si,&val,0);
	    if (err)
		break;
	    *dest++ = htons(val);
	}
	CheckEolSI(si);
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextMINIGAME()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// file format
)
{
    DASSERT(minigame);
    PRINT("ScanTextMINIGAME(init=%d)\n",init_minigame);

    if (init_minigame)
	InitializeMINIGAME(minigame,FF_KMG_TXT);
    else
    {
	minigame->fform_source = FF_KMG_TXT;
	SetupBZ2MgrMINIGAME(minigame,true);
    }

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,minigame->fname,minigame->revision);
    si.predef = SetupVarsCOMMON();

    enumError max_err = ERR_OK;
    minigame->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN MINIGAME SECTIONS, PASS%u ...\n",minigame->is_pass2+1);

	max_err = ERR_OK;
	si.no_warn = !minigame->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", minigame->is_pass2+1 );

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
	    ResetLocalVarsSI(&si,minigame->revision);

	    si.cur_file->ptr++;
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    noPRINT("--> pass=%u: #%04u: %s\n",minigame->is_pass2+1,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,minigame_section_name);
	    if ( !cmd || abbrev_count )
	    {
		cmd = ScanKeyword(&abbrev_count,name,minigame_table_list);
		if ( cmd && !abbrev_count )
		{
		    while ( cmd > minigame_table_list && cmd[-1].id == cmd->id )
			cmd--;
		    NextLineSI(&si,false,false);
		    enumError err = ScanTextTABLE(minigame,&si,cmd);
		    if ( max_err < err )
			 max_err = err;
		}
		continue;
	    }

	    NextLineSI(&si,false,false);
	    noPRINT("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case MINIGAME_SETUP:
		    err = ScanTextSETUP(minigame,&si);
		    break;

		case MINIGAME_PARAMETERS:
		    err = ScanTextPARAMETERS(minigame,&si);
		    break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (minigame->is_pass2)
	    break;

	minigame->is_pass2 = true;

	RestartSI(&si);
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextMINIGAME) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MINIGAME: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(minigame);

    enumError err;
// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    switch (fform)
    {
	case FF_KMG:
	    err = ScanRawMINIGAME(minigame,init_minigame,data,data_size);
	    break;

	case FF_KMG_TXT:
	    err =  ScanTextMINIGAME(minigame,init_minigame,data,data_size,fform);
	    break;

	default:
	    if (init_minigame)
		InitializeMINIGAME(minigame,0);
	    return ERROR0(ERR_INVALID_DATA,
		"No MINIGAME file: %s\n", minigame->fname ? minigame->fname : "?");
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadMINIGAME
(
    minigame_t		* minigame,	// MINIGAME data structure
    bool		init_minigame,	// true: initialize 'minigame' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(minigame);
    DASSERT(fname);
    if (init_minigame)
	InitializeMINIGAME(minigame,0);
    else
	ResetMINIGAME(minigame);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	minigame->fname = raw.fname;
	raw.fname = 0;
	err = ScanMINIGAME(minigame,false,raw.data,raw.data_size);
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawMINIGAME()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawMINIGAME
(
    minigame_t		* minigame,	// pointer to valid MINIGAME
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(minigame);
    DASSERT(fname);

    PRINT("SaveRawMINIGAME(%s,%d)\n",fname,set_time);

    //--- write to file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&minigame->fatt,0);

    const uint size = sizeof(minigame->data);
    if ( fwrite(&minigame->data,1,size,F.f) != size )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  SaveTextMINIGAME()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextMINIGAME
(
    const minigame_t	* minigame,	// pointer to valid MINIGAME
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
// [[minimize]]
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >9: minimize output
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(minigame);
    DASSERT(fname);
    PRINT("SaveTextMINIGAME(%s,%d)\n",fname,set_time);

    BZ2Manager_t *bz = GetCommonBZ2Manager(FF_KMG);


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,minigame->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&minigame->fatt,0);


    //--- print header + syntax info

    const bool explain = print_header && !brief_count && !export_count;
    if (explain)
	fputs(text_minigame_info_cr,F.f);
    else
	fputs(text_minigame_head_cr,F.f);

    const bool tiny = brief_count > 1 || export_count;
    if (!tiny)
	fprintf(F.f,text_minigame_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );


    //--- print parameters

    bool sect_head_done	= false;
    int  last_mode	= -1;
    uint active_offset	= ~0;

    const KeywordTab_t *par;
    for ( par = minigame_param_list; par->name1; par++ )
    {
	const uint offset = par->id;
	if ( active_offset == offset ) // skip entries for same parameter
	    continue;
	active_offset = offset;

	const s16 val  = be16((u8*)&minigame->data+offset);
	const s16 orig = be16(bz->data+offset);
	if ( !minimize || val != orig )
	{
	    if (!sect_head_done)
	    {
		sect_head_done = true;
		fputs( brief_count < 2 ? section_sep : "\r\n", F.f );
		fprintf(F.f,text_minigame_param_cr,REVISION_NUM);
	    }

	    if ( last_mode != par->opt )
	    {
		last_mode = par->opt;
		fprintf(F.f,"\r\n# %s\r\n",minigame_param_title[last_mode]);
	    }
	    ccp name = par->name2 ? par->name2 : par->name1;
	    const int tablen = (39-strlen(name))/8;
	    fprintf(F.f, "%s%.*s=%6d  # orig:%6d%s\r\n",
			name, tablen > 1 ? tablen : 1, Tabs20,
			val, orig, val == orig || minimize ? "" : "  DIFFER!" );
	}
    }


    //--- print tables

    const uint slot_size  = 11*2;
    const uint table_size = 10*slot_size;

    for ( par = minigame_table_list; par->name1; par++ )
    {
	const uint offset = par->id;
	if ( active_offset == offset ) // skip entries for same section
	    continue;
	active_offset = offset;

	const u8 *val  = (u8*)&minigame->data + offset;
	const u8 *orig =  bz->data + offset;
	if ( !minimize || memcmp(val,orig,table_size) )
	{
	    fputs( brief_count < 2 ? section_sep : "\r\n", F.f );
	    fprintf(F.f, text_minigame_table_cr,
		    par->name2 ? par->name2 : par->name1,
		    offset, REVISION_NUM );
	    // [[2do]] ??? table dependent info
	    fputs(text_minigame_table_head_cr,F.f);

	    uint slot;
	    for ( slot = 0; slot < 10; slot++ )
	    {
		const int differ = memcmp(val,orig,slot_size);
		if ( !minimize || differ )
		{
		    fprintf(F.f,"%5u ",slot);
		    uint pl;
		    for ( pl = 0; pl < 11; pl++, val += 2 )
			fprintf(F.f," %4d",be16(val));
		    fprintf(F.f,"  # A%02u %s\r\n",
				arena_info[slot].def_slot, arena_info[slot].name_en );

		    if (differ)
		    {
			fputs("#ORIG:",F.f);
			val -= slot_size;
			for ( pl = 0; pl < 11; pl++, val += 2, orig += 2 )
			    if ( be16(val) == be16(orig) )
				fputs("    .",F.f);
			    else
				fprintf(F.f," %4d",be16(orig));
			fputs("\r\n",F.f);
			if (minimize)
			    fputs("\r\n",F.f);
		    }
		    else
			orig += slot_size;
		}
		else
		{
		    val  += slot_size;
		    orig += slot_size;
		}
	    }
	}
    }


    //--- terminate

    fputs(  brief_count < 2 ? section_end :"\r\n", F.f );
    ResetFile(&F,set_time);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  scan options			///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptKmgLimit ( ccp arg )
{
    char *end;
    const long num = str2l(arg,&end,10);
    if ( end == arg || *end )
    {
	ERROR0(ERR_SYNTAX,"Invalid argumant for option --kmg-limit: '%s'\n",arg);
	return 1;
    }

    opt_kmg_limit = num < 0 ? 0 : num > 0xffff ? 0xffff : num;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t arena_attrib_tab[] =
{
	{  0,	"0",	"A12",	12 },
	{  1,	"1",	"A11",	11 },
	{  2,	"2",	"A14",	14 },
	{  3,	"3",	"A13",	13 },
	{  4,	"4",	"A15",	15 },
	{  5,	"5",	"A24",	24 },
	{  6,	"6",	"A25",	25 },
	{  7,	"7",	"A21",	21 },
	{  8,	"8",	"A22",	22 },
	{  9,	"9",	"A23",	23 },
	{ -1,	"-1",	"OFF",	0 },
	{0,0,0,0}
};

//-----------------------------------------------------------------------------

int ScanOptKmgCopy ( ccp arg )
{
    if ( !arg || !*arg )
    {
	opt_kmg_arena = -1;
	return 0;
    }

    ccp p1 = strrchr(arg,'[');
    if (p1)
    {
	 ccp p2 = strchr(++p1,']');
	 if (p2)
	 {
	    while ( p1 < p2 )
	    {
		ccp comma = strchr(p1,',');
		if ( !comma || comma > p2 )
		    comma = p2;
		uint len = comma - p1;
		char name[10];
		if ( ( *p1 == 'a' || *p1 == 'A' ) && len < sizeof(name) )
		{
		    memcpy(name,p1,len);
		    name[len] = 0;
		    const KeywordTab_t *key = ScanKeyword(0,name,arena_attrib_tab);
		    if (key)
		    {
			opt_kmg_arena = key->id;
			return 0;
		    }
		}
		p1 = comma+1;
	    }
	    return 0;
	 }
    }

    int abbrev;
    const KeywordTab_t *key = ScanKeyword(&abbrev,arg,arena_attrib_tab);
    if (!key)
    {
	PrintKeywordError(arena_attrib_tab,arg,abbrev,0,
		"keyword for option --kmg-arena");
	return 1;
    }
    opt_kmg_arena = key->id;
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


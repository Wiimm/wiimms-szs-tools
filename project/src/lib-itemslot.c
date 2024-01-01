
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

#include "itemslot.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    data			///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
	RACE_PARAM	= 0,
	BATTLE_PARAM	= 1,
};

static const KeywordTab_t itemslot_table_list[] =
{
	{ 0x001, "GRAND-PRIX-PLAYER",	0,		RACE_PARAM },
	{ 0x0e7, "GRAND-PRIX-ENEMY",	0,		RACE_PARAM },
	{ 0x1cd, "VERSUS-PLAYER",	0,		RACE_PARAM },
	{ 0x2b3, "VERSUS-ENEMY",	0,		RACE_PARAM },
	{ 0x399, "VERSUS-ONLINE",	0,		RACE_PARAM },
	{ 0x47f, "SPECIAL",		0,		RACE_PARAM },

	// typo "BALLON" will be [[obsolete]], "BALLOON" since 2020-01 (3x)
	{ 0x5b1, "BALLOON-PLAYER",	"BALLON-PLAYER", BATTLE_PARAM },
	{ 0x5ec, "BALLOON-ENEMY",	"BALLON-ENEMY",  BATTLE_PARAM },
	{ 0x627, "COIN-PLAYER", 	0,		 BATTLE_PARAM },
	{ 0x662, "COIN-ENEMY",		0,		 BATTLE_PARAM },
	{ 0x69d, "BALLOON-ONLINE", 	"BALLON-ONLINE", BATTLE_PARAM },
	{ 0x6d8, "COIN-ONLINE", 	0,		 BATTLE_PARAM },

	{ 0,0,0,0 }
};

//-----------------------------------------------------------------------------

static ccp item_info[] =
{
	"Green Shell",
	"Red Shell",
	"Banana",
	"Fake Item Box",
	"Mushroom",
	"Triple Mushroom",
	"Bob-omb",
	"Blue Shell",
	"Lightning",
	"Star",
	"Golden Mushroom",
	"Mega Mushroom",
	"Blooper",
	"POW Block",
	"Thundercloud",
	"Bullet Bill",
	"Triple Green Shells",
	"Triple Red Shells",
	"Triple Bananas",
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ITEMSLOT setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeITEMSLOT ( itemslot_t * itemslot, file_format_t fform )
{
    DASSERT(itemslot);
    memset(itemslot,0,sizeof(*itemslot));
    itemslot->fname = EmptyString;
    itemslot->fform_source = fform;
    itemslot->add_battle = true;
    SetupBZ2MgrITEMSLOT(itemslot,true);
}

///////////////////////////////////////////////////////////////////////////////

void ResetITEMSLOT ( itemslot_t * itemslot )
{
    DASSERT(itemslot);
    const typeof(itemslot->fform_source) fform = itemslot->fform_source;
    FreeString(itemslot->fname);
    InitializeITEMSLOT(itemslot,fform);
}

///////////////////////////////////////////////////////////////////////////////

bool GetOriginalITEMSLOT ( itemslot_bin_t *dest )
{
    DASSERT(dest);
    BZ2Manager_t *bz = GetCommonBZ2Manager(FF_ITEMSLT);
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

void SetupBZ2MgrITEMSLOT ( itemslot_t * itemslot, bool reload_bin )
{
    DASSERT(itemslot);
    if ( reload_bin || !itemslot->data.n_table )
	GetOriginalITEMSLOT(&itemslot->data);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawITEMSLOT()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(itemslot);
    if (init_itemslot)
	InitializeITEMSLOT(itemslot,FF_ITEMSLT);

    SetupBZ2MgrITEMSLOT(itemslot,true);
    if ( !data || data_size != ITEMSLT_SIZE6 && data_size != ITEMSLT_SIZE12 )
	return ERROR0(ERR_INVALID_DATA,
		"Invalid file size (%u but not %zu bytes: %s\n",
		data_size, sizeof(itemslot->data), itemslot->fname );

    memcpy(&itemslot->data,data,data_size);
    itemslot->add_battle = data_size == ITEMSLT_SIZE12;


    //--- fix table headers

    BZ2Manager_t *bz = GetCommonBZ2Manager(FF_ITEMSLT);
    const KeywordTab_t *par;
    for ( par = itemslot_table_list; par->name1; par++ )
    {
	const uint offset = par->id;
	u8 *val		= (u8*)&itemslot->data + offset;
	const u8 *orig	= bz->data + offset;
	*val++ = *orig++;
	*val++ = *orig++;
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		ScanTextITEMSLOT() helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    ITEMSLOT_IGNORE,
    ITEMSLOT_SETUP,
    ITEMSLOT_PARAMETERS,
};

//-----------------------------------------------------------------------------

static const KeywordTab_t itemslot_section_name[] =
{
	{ ITEMSLOT_IGNORE,	"END",		0, 0 },
	{ ITEMSLOT_SETUP,	"SETUP",	0, 0 },
	{ ITEMSLOT_PARAMETERS,	"PARAMETERS",	0, 0 },
	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextSETUP
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(itemslot);
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
	    ScanU32SI(si,(u32*)&itemslot->revision,1,0);
	    si->init_revision = si->cur_file->revision = itemslot->revision;
	    DefineIntVar(&si->gvar,"REVISION$SETUP",itemslot->revision);
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",itemslot->revision);
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
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(itemslot);
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
	else if (!strcmp(name,"ADD-BATTLE"))
	{
	    int val;
	    const enumError err = ScanAssignIntSI(si,&val);
	    if (!err)
		itemslot->add_battle = val > 0 ;
	}
	else if (!strcmp(name,"USE-SLT"))
	{
	    int val;
	    const enumError err = ScanAssignIntSI(si,&val);
	    if (!err)
		itemslot->use_slt = val > 0 ;
	}
	CheckEolSI(si);
    }

    CheckLevelSI(si);
    return ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextTABLE
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    ScanInfo_t		* si,		// valid data
    const KeywordTab_t	* par		// valid section pointer
)
{
    DASSERT(itemslot);
    DASSERT(si);
    DASSERT(par);

    const uint offset	= par->id;
    u8 *val		= (u8*)&itemslot->data + offset;
    const uint n_col	= *val++;
    const uint n_row	= *val++;

    PRINT0(">> SCAN offset 0x%03x, size %2u*%u  %s\n",
		offset, n_col, n_row, par->name1 );
    si->point_is_null++;

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

	long item;
	enumError err = ScanUValueSI(si,&item,0);
	if (err)
	    break;
	if ( item >= n_row )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    if ( si->no_warn <= 0 )
		ERROR0(ERR_WARNING,
			"Item ID is %lu but must be between 0 and %u: [%s @%u]: %.*s\n",
			item, n_row-1, sf->name, sf->line, (int)(eol-sf->ptr), sf->ptr );
	    sf->ptr = eol;
	    continue;
	}

	u8 *dest = val + item * n_col;;
	uint idx;
	for ( idx = 0; idx < n_col; idx++ )
	{
	    ch = NextCharSI(si,false);
	    if (!ch)
		break;

	    long val;
	    err = ScanUValueSI(si,&val,0);
	    if (err)
		break;
	    *dest++ = val;
	}
	CheckEolSI(si);
    }

    si->point_is_null--;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextITEMSLOT()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// file format
)
{
    DASSERT(itemslot);
    PRINT("ScanTextITEMSLOT(init=%d)\n",init_itemslot);

    if (init_itemslot)
	InitializeITEMSLOT(itemslot,FF_ITEMSLT_TXT);
    else
    {
	itemslot->fform_source = FF_ITEMSLT_TXT;
	SetupBZ2MgrITEMSLOT(itemslot,true);
    }

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,itemslot->fname,itemslot->revision);
    si.predef = SetupVarsCOMMON();

    enumError max_err = ERR_OK;
    itemslot->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN ITEMSLOT SECTIONS, PASS%u ...\n",itemslot->is_pass2+1);

	max_err = ERR_OK;
	si.no_warn = !itemslot->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", itemslot->is_pass2+1 );

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
	    ResetLocalVarsSI(&si,itemslot->revision);

	    si.cur_file->ptr++;
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    noPRINT("--> pass=%u: #%04u: %s\n",itemslot->is_pass2+1,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,itemslot_section_name);
	    if ( !cmd || abbrev_count )
	    {
		cmd = ScanKeyword(&abbrev_count,name,itemslot_table_list);
		if ( cmd && !abbrev_count )
		{
		    while ( cmd > itemslot_table_list && cmd[-1].id == cmd->id )
			cmd--;
		    NextLineSI(&si,false,false);
		    enumError err = ScanTextTABLE(itemslot,&si,cmd);
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
		case ITEMSLOT_SETUP:
		    err = ScanTextSETUP(itemslot,&si);
		    break;

		case ITEMSLOT_PARAMETERS:
		    err = ScanTextPARAMETERS(itemslot,&si);
		    break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (itemslot->is_pass2)
	    break;

	itemslot->is_pass2 = true;

	RestartSI(&si);
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextITEMSLOT) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ITEMSLOT: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname		// not NULL: Analyse file extension for '.slt'
)
{
    DASSERT(itemslot);

    enumError err;
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    switch (fform)
    {
	case FF_ITEMSLT:
	    err = ScanRawITEMSLOT(itemslot,init_itemslot,data,data_size);
	    if (fname)
	    {
		const uint fnlen = strlen(fname);
		itemslot->use_slt = fnlen > 4 && !strcasecmp(fname+fnlen-4,".slt");
		PRINT("USE_SLT=%d, ADD_BATTLE=%d\n",itemslot->use_slt,itemslot->add_battle);
	    }
	    break;

	case FF_ITEMSLT_TXT:
	    err =  ScanTextITEMSLOT(itemslot,init_itemslot,data,data_size,fform);
	    break;

	default:
	    if (init_itemslot)
		InitializeITEMSLOT(itemslot,0);
	    return ERROR0(ERR_INVALID_DATA,
		"No ITEMSLOT file: %s\n", itemslot->fname ? itemslot->fname : "?");
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadITEMSLOT
(
    itemslot_t		* itemslot,	// ITEMSLOT data structure
    bool		init_itemslot,	// true: initialize 'itemslot' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(itemslot);
    DASSERT(fname);
    if (init_itemslot)
	InitializeITEMSLOT(itemslot,0);
    else
	ResetITEMSLOT(itemslot);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	itemslot->fname = raw.fname;
	raw.fname = 0;
	err = ScanITEMSLOT(itemslot,false,raw.data,raw.data_size,fname);
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawITEMSLOT()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawITEMSLOT
(
    itemslot_t		* itemslot,	// pointer to valid ITEMSLOT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(itemslot);
    DASSERT(fname);

    PRINT("SaveRawITEMSLOT(%s,%d)\n",fname,set_time);

    //--- write to file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&itemslot->fatt,0);

    const uint size = itemslot->add_battle ? ITEMSLT_SIZE12 : ITEMSLT_SIZE6;
    itemslot->data.n_table = itemslot->add_battle ? 12 : 6;
    if ( fwrite(&itemslot->data,1,size,F.f) != size )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  SaveTextITEMSLOT()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextITEMSLOT
(
    const itemslot_t	* itemslot,	// pointer to valid ITEMSLOT
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
// [[minimize]]
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >1: write only modified records
					//   >9: minimize output
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(itemslot);
    DASSERT(fname);
    PRINT("SaveTextITEMSLOT(%s,%d)\n",fname,set_time);

    BZ2Manager_t *bz = GetCommonBZ2Manager(FF_ITEMSLT);


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,itemslot->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&itemslot->fatt,0);


    //--- print header + syntax info

    const bool explain = print_header && !brief_count && !export_count;
    if (explain)
	fputs(text_itemslot_info_cr,F.f);
    else
	fputs(text_itemslot_head_cr,F.f);

    const bool tiny = brief_count > 1 || export_count;
    if (!tiny)
	fprintf(F.f,text_itemslot_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );


    //--- print parameters

    if ( minimize < 10 )
    {
	fputs( brief_count < 2 ? section_sep : "\r\n", F.f );
	fprintf(F.f,text_itemslot_param_cr,
		REVISION_NUM, itemslot->add_battle, itemslot->use_slt );
    }


    //--- print tables

    uint active_offset = 0;
    const uint N_SUM = 20;
    uint sum[N_SUM];

    const KeywordTab_t *par;
    for ( par = itemslot_table_list; par->name1; par++ )
    {
	const uint offset = par->id;
	if ( active_offset == offset ) // skip entries for same section
	    continue;
	active_offset = offset;

	const bool is_battle = par->opt == BATTLE_PARAM;
	if ( is_battle && !itemslot->add_battle )
	    continue;

	const u8 *val		= (u8*)&itemslot->data + offset + 2;
	const u8 *orig		= bz->data + offset;
	const uint n_col	= *orig++;
	const uint n_row	= *orig++;
	const uint table_size	= n_row * n_col;
	DASSERT( n_col <= N_SUM );

	if ( minimize < 1 || memcmp(val,orig,table_size) )
	{
	    uint row, col;
	    memset(sum,0,sizeof(sum));

	    fputs( brief_count < 2 ? section_sep : "\r\n", F.f );
	    fprintf(F.f, text_itemslot_table_cr,
		    par->name2 ? par->name2 : par->name1,
		    is_battle ? "battle" : "racing",
		    offset, n_row, n_col, REVISION_NUM );
	    // [[2do]] ??? table dependent info

	    const uint fw1 = 7 + 4*n_col;
	    const uint fw2 = 25;
	    fprintf(F.f,"#%.*s#%.*s\r\n# item",fw1,Minus300,fw2,Minus300);
	    for ( col = 0; col < n_col; col++ )
		fprintf(F.f,"%3u.",col+1);
	    fprintf(F.f,"  # sum item name\r\n#%.*s#%.*s\r\n",fw1,Minus300,fw2,Minus300);

	    for ( row = 0; row < n_row; row++ )
	    {
		const int differ = memcmp(val,orig,n_col);
		if ( delta_count < 2 || differ )
		{
		    fprintf(F.f,"%5u ",row);
		    uint row_sum = 0;
		    for ( col = 0; col < n_col; col++ )
		    {
			const uint num = val[col];
			if (num)
			{
			    row_sum += num;
			    sum[col] += num;
			    fprintf(F.f,"%4u",num);
			}
			else
			    fputs("   .",F.f);
		    }
		    fprintf(F.f,"  #%4u %s\r\n",row_sum,item_info[row]);

		    if (differ)
		    {
			fputs("#ORIG:",F.f);
			for ( col = 0; col < n_col; col++ )
			    if ( val[col] == orig[col] )
				fputs("   .",F.f);
			    else
				fprintf(F.f,"%4d",orig[col]);
			fputs("\r\n",F.f);
			if ( minimize < 2 )
			    fputs("\r\n",F.f);
		    }
		}
		val  += n_col;
		orig += n_col;
	    }
	    if ( minimize < 2 )
	    {
		fprintf(F.f,"#%.*s#%.*s\r\n#SUM: ",fw1,Minus300,fw2,Minus300);
		for ( col = 0; col < n_col; col++ )
		    fprintf(F.f,"%4d",sum[col]);
		fputs("\r\n",F.f);
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
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


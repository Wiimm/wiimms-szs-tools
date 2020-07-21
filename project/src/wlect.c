
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

#include "lib-lecode.h"
#include "lib-szs.h"
#include "lib-xbmg.h"
//#include "lib-bzip2.h"

#include "ui.h" // [[dclib]] wrapper
#include "ui-wlect.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WLECT_SHORT ": " WLECT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM " - " AUTHOR " - " DATE

static const char autoname[] = "/course.lex";

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    help			///////////////
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wlect,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wlect,stdout,0,0,"HELP",0,URI_HOME);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    if (print_sect_header)
	fputs("[version]\n",stdout);

    const u32 base = 0x04030201;
    const u8 * e = (u8*)&base;
    const u32 endian = be32(e);

    printf( "prog=" WLECT_SHORT "\n"
	    "name=" WLECT_LONG "\n"
	    "version=" VERSION "\n"
	    "beta=%d\n"
	    "revision=" REVISION  "\n"
	    "system=" SYSTEM "\n"
	    "endian=%u%u%u%u %s\n"
	    "author=" AUTHOR "\n"
	    "date=" DATE "\n"
	    "url=" URI_HOME WLECT_SHORT "\n"
	    "\n"
	    , BETA_VERSION
	    , e[0], e[1], e[2], e[3]
	    , endian == 0x01020304 ? "little"
		: endian == 0x04030201 ? "big" : "mixed" );
}

///////////////////////////////////////////////////////////////////////////////

static void version_exit()
{
    if ( brief_count > 1 )
	fputs( VERSION "\n", stdout );
    else if (brief_count)
	fputs( VERSION " r" REVISION " " SYSTEM "\n", stdout );
    else if (print_sections)
	print_version_section(true);
    else if (long_count)
	print_version_section(false);
    else
	fputs( TITLE "\n", stdout );

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_title ( FILE * f )
{
    static bool done = false;
    if (!done)
    {
	done = true;
	if (print_sections)
	    print_version_section(true);
	else if ( verbose >= 1 && f == stdout )
	    fprintf(f,"\n%s\n\n",TITLE);
	else
	    fprintf(f,"*****  %s  *****\n",TITLE);
    }
}

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t * current_command = 0;

static void hint_exit ( enumError stat )
{
    if ( current_command )
	fprintf(stderr,
	    "-> Type '%s help %s' (pipe it to a pager like 'less') for more help.\n\n",
	    progname, CommandInfo[current_command->id].name1 );
    else
	fprintf(stderr,
	    "-> Type '%s -h' or '%s help' (pipe it to a pager like 'less') for more help.\n\n",
	    progname, progname );
    exit(stat);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command test			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test_options()
{
    printf("\nOptions (compatibility: %s; format: hex=dec):\n",PrintOptCompatible());

    printf("  test:        %16x = %12d\n",testmode,testmode);
    printf("  verbose:     %16x = %12d\n",verbose,verbose);
    printf("  width:       %16x = %12d\n",opt_width,opt_width);
    printf("  escape-char: %16x = %12d\n",escape_char,escape_char);

    printf("  print-header:%16x = %12d\n",print_header,print_header);
    printf("  brief:       %16x = %12d\n",brief_count,brief_count);
    printf("  export:      %16x = %12d\n",export_count,export_count);

    if (opt_tracks)
	DumpTrackList(0,0,0);
    if (opt_arenas)
	DumpArenaList(0,0,0);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test()
{
 #if 0 || !defined(TEST) // test options

    return cmd_test_options();

 #elif 1

    uint tr;
    for ( tr = 0; tr < MKW_N_TRACKS + MKW_N_ARENAS; tr++ )
    {
	const TrackInfo_t *info = GetTrackInfo(tr);
	if (info)
	    printf(" %2u = %02x : 0x%02x\n",tr,tr,info->music_id);
	//printf(" %2u = %02x : %s\n",tr,tr,PrintMusicID(tr,false));
    }
    return ERR_OK;

 #elif 1

    uint mi;
    for ( mi = 0x75; mi <= 0xc9; mi += 2)
    {
	int ti = MusicID2TrackId(mi,-1,-1);
	if ( ti > 0 )
	{
	    const TrackInfo_t *info = GetTrackInfo(ti);
	    printf("0x%02x = 0x%02x  %2u %2u  %s\n",
			mi, info->music_id,
			info->def_index, info->def_slot, info->sound_n_fname );
	}
    }
    return ERR_OK;

 #elif 1

    uint ti;
    for ( ti = 0; ti < MKW_N_TRACKS; ti++ )
    {
	// find music slot
	int mi, mslot = -1;
	for ( mi = 0x75; mi <= 0xc9; mi += 2)
	    if ( MusicID2TrackId(mi,-1,-1) == ti )
	    {
		mslot = mi;
		break;
	    }

	const TrackInfo_t *info = track_info + ti;
	printf("\t{ %2u, \"%s\", \"T%02u\", 0x%02x },\n",
		ti, info->name_en, info->def_slot, mslot );
    }
    return ERR_OK;

 #elif 0

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	//NORMALIZE_FILENAME_PARAM(param);
    }
    return ERR_OK;

 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command export			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_export()
{
    SetupVarsLECODE();
    return ExportHelper("lecode");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command dump			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dump ( int long_level )
{
    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI_wlect,OPT_LONG,long_level,false);
	long_count += long_level;
    }

    raw_data_t raw;
    InitializeRawData(&raw);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 )
	{
	    fprintf(stdlog,"\nDUMP %s:%s\n", GetNameFF(raw.fform,0), raw.fname );
	    fflush(stdlog);
	}

	if ( raw.fform == FF_LE_BIN )
	{
	    le_analyse_t ana;
	    AnalyseLEBinary(&ana,raw.data,raw.data_size);
	    ApplyLEFile(&ana);
	    PatchLECODE(&ana);
	    DumpLEAnalyse(stdlog,2,&ana);
	    ResetLEAnalyse(&ana);
	}
	else if ( raw.fform == FF_LEX )
	{
	    putchar('\n');
	    lex_t lex;
	    err = ScanRawLEX(&lex,true,raw.data,raw.data_size,
			long_count > 2 ?   -1 :
			long_count > 1 ? 0x40 :
			long_count > 0 ? 0x10 : 1 );
	    ResetLEX(&lex);
	}
	else
	{
	    ctcode_t ctcode;
	    err = ScanRawDataCTCODE(&ctcode,CTM_LECODE_DEF,&raw,global_check_mode);
	    if ( err > ERR_WARNING )
		return err;

	    DumpCTCODE(stdout,2,&ctcode);
	    ResetCTCODE(&ctcode);
	}
	fflush(stdout);
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command bin-diff		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct lecode_bin_diff_t
{
    bool diff_size;
    bool diff_header;
    bool diff_timestamp;
    bool diff_body;
    bool diff_param;
    bool diff_data;
    bool diff_code;

    u8 stat_size;
    u8 stat_header;
    u8 stat_timestamp;
    u8 stat_body;
    u8 stat_param;
    u8 stat_data;
    u8 stat_code;

    enumError status;
}
lecode_bin_diff_t;

///////////////////////////////////////////////////////////////////////////////

static enumError LoadLECODE ( raw_data_t *raw, le_analyse_t *ana, ccp fname )
{
    DASSERT(raw);
    DASSERT(ana);
    DASSERT(fname);

    InitializeRawData(raw);
    memset(ana,0,sizeof(*ana));

    enumError err = LoadRawData(raw,false,fname,0,false,0);
    if (err)
	goto abort;

    if ( verbose >= 0 || raw->fform != FF_LE_BIN )
    {
	fprintf(stdlog,"File: %s:%s\n", GetNameFF(raw->fform,0), raw->fname );
	fflush(stdlog);
    }

    if ( raw->fform != FF_LE_BIN )
    {
	err = ERROR0(ERR_WRONG_FILE_TYPE,"Not a LECODE binary file: %s\n",raw->fname);
	goto abort;
    }

    err = AnalyseLEBinary(ana,raw->data,raw->data_size);
    if (err)
    {
	err = ERROR0(ERR_INVALID_DATA,"Invalid LECODE data: %s\n",raw->fname);
	goto abort;
    }

    return ERR_OK;

   abort:
    ResetLEAnalyse(ana);
    ResetRawData(raw);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static int BinDiffLECODE
	( lecode_bin_diff_t *bd, const le_analyse_t *a1, const le_analyse_t *a2 )
{
    DASSERT(bd);
    DASSERT(a1);
    DASSERT(a2);


    //--- clear "built on <DATE+TIME>"

    static const char search[] = "built on ";

    const int bodysize1 = a1->size - a1->param_offset;
    u8 *body1 = (u8*)a1->data + a1->param_offset;

    if ( bodysize1 > 0 )
    {
	u8 *found = memmem(body1,bodysize1,search,sizeof(search)-1);
	if (found)
	{
	    PRINT0("FOUND: %zx %s\n",found-body1,found);
	    u8 *end = body1 + bodysize1;
	    while ( found && found < end && *found )
		*found++ = 0;
	}
    }

    const int bodysize2 = a2->size - a2->param_offset;
    u8 *body2 = (u8*)a2->data + a2->param_offset;

    if ( bodysize2 > 0 )
    {
	u8 *found = memmem(body2,bodysize2,search,sizeof(search)-1);
	if (found)
	{
	    PRINT0("FOUND: %zx %s\n",found-body2,found);
	    u8 *end = body2 + bodysize2;
	    while ( found && found < end && *found )
		*found++ = 0;
	}
    }


    //--- compare ...

    const le_binary_head_t *h1 = a1->head;
    const le_binary_head_t *h2 = a2->head;

    bd->stat_size   = a1->size != a2->size;
    bd->stat_header = memcmp(h1,h2,sizeof(*h1)) != 0;

    bd->stat_body
		=  bodysize1 < 0
		|| bodysize2 < 0
		|| bodysize1 != bodysize2
		|| memcmp( body1, body2, bodysize1 );

    ccp ts1 = h1->version == 4 ? a1->head_v4->timestamp : 0;
    ccp ts2 = h2->version == 4 ? a2->head_v4->timestamp : 0;
    bd->stat_timestamp = !ts1 || !ts2 || strcmp(ts1,ts2);

    bd->stat_param = memcmp(&a1->lpar,&a2->lpar,sizeof(a1->lpar));

    bd->stat_data = a1->n_cup_track	!= a2->n_cup_track
		 || a1->n_cup_arena	!= a2->n_cup_arena
		 || a1->n_slot		!= a2->n_slot;
    if (!bd->stat_data)
    {
	bd->stat_data
		=  memcmp( a1->cup_track, a2->cup_track,
				a1->n_cup_track * sizeof(*a1->cup_track) )
		|| memcmp( a1->cup_arena, a2->cup_arena,
				a1->n_cup_arena * sizeof(*a1->cup_arena) )
		|| memcmp( a1->property, a2->property,
				a1->n_slot * sizeof(*a1->property) )
		|| memcmp( a1->music, a2->music,
				a1->n_slot * sizeof(*a1->music) )
		|| memcmp( a1->flags, a2->flags,
				a1->n_slot * sizeof(*a1->flags) )
		;
    }


    //-- code is separated into 2 parts

    PRINT0("CODE1: %zx..%zx + %zx..%x\n",
		body1 - a1->data,
		a1->beg_of_data - a1->data,
		a1->end_of_data - a1->data,
		a1->size );
    PRINT0("CODE2: %zx..%zx + %zx..%x\n",
		body2 - a2->data,
		a2->beg_of_data - a2->data,
		a2->end_of_data - a2->data,
		a2->size );

    bd->stat_code
		=  body1 - a1->data		!= body2 - a2->data
		|| a1->beg_of_data - a1->data	!= a2->beg_of_data - a2->data
		|| a1->end_of_data - a1->data	!= a2->end_of_data - a2->data
		|| a1->size			!= a2->size
		;

    if (!bd->stat_code)
    {
	int off1 = body1 - a1->data;
	int off2 = a1->beg_of_data - a1->data;
	if ( off1 < off2 && memcmp(a1->data+off1,a2->data+off1,off2-off1))
	    bd->stat_code = true;
	else
	{
	    off1 = a1->end_of_data - a1->data;
	    off2 = a1->size;
	    if ( off1 < off2 && memcmp(a1->data+off1,a2->data+off1,off2-off1))
		bd->stat_code = true;
	}
    }


    //--- final status

    int ec = 0;
    if ( bd->diff_size      && bd->stat_size      ) { ec++; bd->stat_size	= 2; }
    if ( bd->diff_header    && bd->stat_header    ) { ec++; bd->stat_header	= 2; }
    if ( bd->diff_timestamp && bd->stat_timestamp ) { ec++; bd->stat_timestamp	= 2; }
    if ( bd->diff_body      && bd->stat_body      ) { ec++; bd->stat_body	= 2; }
    if ( bd->diff_param     && bd->stat_param     ) { ec++; bd->stat_param	= 2; }
    if ( bd->diff_data      && bd->stat_data      ) { ec++; bd->stat_data	= 2; }
    if ( bd->diff_code      && bd->stat_code      ) { ec++; bd->stat_code	= 2; }

    return bd->status = ec ? ERR_DIFFER : ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_bin_diff()
{
    if ( n_param != 3 )
	return ERROR0(ERR_SYNTAX,"Exact 3 parameters expected for BIN-DIFF:"
			" settings binary1 binary2\n");


    //--- scan settings

    lecode_bin_diff_t bd;
    memset(&bd,0,sizeof(bd));

    ParamList_t *param = first_param;
    ccp arg = param->arg;
    bool value = true;

    while (*arg)
    {
	char ch = tolower((int)*arg++);
	switch (ch)
	{
	    case '-': value = false; break;
	    case '+': value = true; break;

	    case 's': bd.diff_size	= value; break;
	    case 'h': bd.diff_header	= value; break;
	    case 't': bd.diff_timestamp	= value; break;
	    case 'b': bd.diff_body	= value; break;
	    case 'p': bd.diff_param	= value; break;
	    case 'd': bd.diff_data	= value; break;
	    case 'c': bd.diff_code	= value; break;

	    case 'a':
		bd.diff_size		= value;
		bd.diff_header		= value;
		bd.diff_timestamp	= value;
		bd.diff_body		= value;
		bd.diff_param		= value;
		bd.diff_data		= value;
		bd.diff_code		= value;
		break;

	    default:
		ERROR0(ERR_WARNING,"BIN-DIFF: Invalid setting: %c\n",ch);
		break;
	}
    }


    //--- open file 1

    param = param->next;
    NORMALIZE_FILENAME_PARAM(param);

    raw_data_t raw1;
    le_analyse_t ana1;

    enumError err = LoadLECODE(&raw1,&ana1,param->arg);
    if (err)
	return err;


    //--- open file 2

    param = param->next;
    NORMALIZE_FILENAME_PARAM(param);

    raw_data_t raw2;
    le_analyse_t ana2;

    err = LoadLECODE(&raw2,&ana2,param->arg);
    if (err)
	return err;


    //--- logging

    if ( verbose > 0 )
    {
	fputs("\nCompare the follwong settings:\n",stdout);
	if (bd.diff_size)	fputs(" - File size (s)\n",stdout);
	if (bd.diff_header)	fputs(" - File header (h)\n",stdout);
	if (bd.diff_timestamp)	fputs(" - Timestamp (t)\n",stdout);
	if (bd.diff_body)	fputs(" - File body (b)\n",stdout);
	if (bd.diff_param)	fputs(" - Parameters (p)\n",stdout);
	if (bd.diff_data)	fputs(" - Cup and track data (d)\n",stdout);
	if (bd.diff_code)	fputs(" - Code (c)\n",stdout);
    }


    //-- diff

    err = BinDiffLECODE(&bd,&ana1,&ana2);
    if ( verbose >= 0 )
    {
	if (bd.stat_size>1)	fputs("> File size differ\n",stdout);
	if (bd.stat_header>1)	fputs("> File header differ\n",stdout);
	if (bd.stat_timestamp>1)fputs("> Timestamp differ\n",stdout);
	if (bd.stat_body>1)	fputs("> File body differ\n",stdout);
	if (bd.stat_param>1)	fputs("> Parameters differ\n",stdout);
	if (bd.stat_data>1)	fputs("> Cup and track data differ\n",stdout);
	if (bd.stat_code>1)	fputs("> Code differ\n",stdout);

	printf(">> Final status: %s (%d)\n", err == ERR_OK ? "same" : "DIFFER", err );
    }


    //-- terminate

    ResetLEAnalyse(&ana1);
    ResetLEAnalyse(&ana2);
    ResetRawData(&raw1);
    ResetRawData(&raw2);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command patch			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_patch()
{
    static ccp def_path = "\1P/\1F";
    CheckOptDest(def_path,false);

    raw_data_t raw;
    InitializeRawData(&raw);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,
			GetExtFF(raw.fform_file,raw.fform),false);
	const bool dest_is_source = IsSameFilename(dest,param->arg);

	if ( verbose >= 0 || testmode )
	{
	    ccp ff_name = GetNameFF(raw.fform_file,raw.fform);
	    if (dest_is_source)
		fprintf(stdlog,"%s%sPATCH %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			ff_name, dest );
	    else
		fprintf(stdlog,"%s%sPATCH %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			ff_name, raw.fname,
			ff_name, dest );
	    fflush(stdlog);
	}

	if ( raw.fform != FF_LE_BIN )
	    return ERROR0(ERR_INVALID_DATA,"Invalid file format: %s",raw.fname);

	le_analyse_t ana;
	AnalyseLEBinary(&ana,raw.data,raw.data_size);
	ApplyLEFile(&ana);
	PatchLECODE(&ana);
	err = SaveFILE( dest, 0, dest_is_source||opt_overwrite,
				raw.data, raw.data_size, 0 );
	ResetLEAnalyse(&ana);
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command create			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_create()
{
    stdlog = stderr;

    if ( n_param != 1 )
    {
	fputs("\n"
	 "Use command CREATE without exact one keyword.\n"
	 "\n"
	 "Possible keywords:\n"
	 "  LEX    : Create a with all sections except »TEST«.\n"
	 "  LEX+   : Create a complete LEX text file.\n"
	 "  SET1   : Create a LEX text files with section »SET1« only.\n"
	 "  CANNON : Create a LEX text files with section »CANN« only.\n"
	 "  TEST   : Create a LEX text files with section »TEST« only.\n"
	 "  LPAR   : Create a LPAR text file as template.\n"
 #if HAVE_WIIMM_EXT
	 "\n"
	 "Hidden keyword:\n"
	 "  DEVELOP : Like LEX+, but include all devoloper sections.\n"
 #endif
	 "\n",stderr);

	hint_exit(ERR_SYNTAX);
    }


    //--- data

    enum { C_LEX, C_SET1, C_CANNON, C_HIDE_PT, C_TEST, C_LPAR };
    static const KeywordTab_t tab[] =
    {
	{ C_LEX,	"LEX",		0,		0 },
	{ C_LEX,	"LEX+",		0,		1 },
	{ C_LEX,	"DEVELOP",	0,		2 },
	{ C_SET1,	"SET1",		0,		0 },
	{ C_CANNON,	"CANNON",	0,		0 },
	{ C_HIDE_PT,	"HIPT",		0,		0 },
	{ C_TEST,	"TEST",		0,		0 },
	{ C_LPAR,	"LPAR",		0,		0 },
	{ 0,0,0,0 }
    };


    //--- scan keyword

    int cmd_stat;
    const KeywordTab_t *cmd = ScanKeyword(&cmd_stat,first_param->arg,tab);
    if (!cmd)
    {
	PrintKeywordError(tab,first_param->arg,cmd_stat,0,"sub command");
	hint_exit(ERR_SYNTAX);
    }


    //--- execute sub command

    enumError err = ERR_OK;
    switch (cmd->id)
    {
     case C_LEX:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    lex.develop = cmd->opt > 1;
	    UpdateLEX(&lex,true,cmd->opt>0);
	    PatchLEX(&lex);
	    err = SaveTextLEX(&lex,"-",false);
	    ResetLEX(&lex);
	}
	break;

     case C_SET1:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    AppendSet1LEX(&lex,false);
	    err = SaveTextLEX(&lex,"-",false);
	    ResetLEX(&lex);
	}
	break;

     case C_CANNON:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    AppendCannLEX(&lex,false);
	    err = SaveTextLEX(&lex,"-",false);
	    ResetLEX(&lex);
	}
	break;

     case C_HIDE_PT:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    AppendHiptLEX(&lex,false);
	    err = SaveTextLEX(&lex,"-",false);
	    ResetLEX(&lex);
	}
	break;

     case C_TEST:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    lex_item_t * li = AppendTestLEX(&lex,false);
	    PatchTestLEX((lex_test_t*)li->elem.data,0);
	    err = SaveTextLEX(&lex,"-",false);
	    ResetLEX(&lex);
	}
	break;

     case C_LPAR:
	{
	    le_lpar_t lpar;
	    InitializeLPAR(&lpar,true);
	    lpar.limit_mode = LPM_FORCE_AUTOMATIC;
	    err = SaveTextLPAR(&lpar,"-",false);
	}
	break;
    }

    fflush(stdout);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command cat			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_cat()
{
    stdlog = stderr;
    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,autoname,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%sCAT %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname );
	    fflush(stdlog);
	}

	if (IsLexFF(raw.fform))
	{
	    lex_t lex;
	    err = ScanRawDataLEX(&lex,true,&raw);
	    if ( err > ERR_WARNING )
	    {
		ResetLEX(&lex);
		if (opt_ignore)
		    continue;
		return err;
	    }

	    if (!testmode)
	    {
		err = SaveTextLEX(&lex,"-",false);
		fflush(stdout);
		if ( err > ERR_WARNING )
		    return err;
		ResetLEX(&lex);
	    }
	}
	else
	{
	    ctcode_t ctcode;
	    err = ScanRawDataCTCODE(&ctcode,CTM_AUTO,&raw,global_check_mode);
	    if ( err > ERR_WARNING )
	    {
		ResetCTCODE(&ctcode);
		if (opt_ignore)
		    continue;
		return err;
	    }

	    if (!testmode)
	    {
		InitializeOutputMode(&output_mode);
		err = SaveTextCTCODE(&ctcode,"-",false,&output_mode);
		fflush(stdout);
		if ( err > ERR_WARNING )
		    return err;
		ResetCTCODE(&ctcode);
	    }
	}
    }

    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command lpar			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_lpar()
{
    stdlog = stderr;
    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,autoname,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%sLPAR %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname );
	    fflush(stdlog);
	}

	if ( raw.fform != FF_LE_BIN )
	    return ERROR0(ERR_INVALID_DATA,"Invalid file format: %s",raw.fname);

	le_analyse_t ana;
	AnalyseLEBinary(&ana,raw.data,raw.data_size);
	ApplyLEFile(&ana);
	PatchLECODE(&ana);
	SaveTextLPAR(&ana.lpar,"-",false);
	ResetLEAnalyse(&ana);
    }

    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		  command encode/decode			///////////////
///////////////////////////////////////////////////////////////////////////////
// cmd_encode(), cmd_decode()

static enumError cmd_convert ( int cmd_id, ccp cmd_name, ccp def_path )
{
    CheckOptDest(def_path,false);

    raw_data_t raw;
    InitializeRawData(&raw);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,autoname,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	const file_format_t dest_ff = cmd_id == CMD_ENCODE ? FF_LEX : FF_LEX_TXT;

	SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,
			GetExtFF(dest_ff,0),false);

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%s%s%s %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "", cmd_name,
			GetNameFF(raw.fform,0), raw.fname,
			GetNameFF(dest_ff,0), dest );
	    fflush(stdlog);
	}

	lex_t lex;
	err = ScanRawDataLEX(&lex,true,&raw);
	if ( err > ERR_WARNING )
	{
	    ResetLEX(&lex);
	    if (opt_ignore)
		continue;
	    return err;
	}

	if (!testmode)
	{
	    err = dest_ff == FF_LEX
			? SaveRawLEX(&lex,dest,opt_preserve)
			: SaveTextLEX(&lex,dest,opt_preserve);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetLEX(&lex);
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   check options                 ///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError CheckOptions ( int argc, char ** argv, bool is_env )
{
    TRACE("CheckOptions(%d,%p,%d) optind=%d\n",argc,argv,is_env,optind);

    optind = 0;
    int err = 0;

    for(;;)
    {
      const int opt_stat = getopt_long(argc,argv,OptionShort,OptionLong,0);
      if ( opt_stat == -1 )
	break;

      RegisterOptionByName(&InfoUI_wlect,opt_stat,1,is_env);

      switch ((enumGetOpt)opt_stat)
      {
	case GO__ERR:		err++; break;

	case GO_VERSION:	version_exit();
	case GO_HELP:		help_exit(false);
	case GO_XHELP:		help_exit(true);
	case GO_ALLOW_ALL:	allow_all = true; break;
	case GO_COMPATIBLE:	err += ScanOptCompatible(optarg); break;
	case GO_WIDTH:		err += ScanOptWidth(optarg); break;
	case GO_QUIET:		verbose = verbose > -1 ? -1 : verbose - 1; break;
	case GO_VERBOSE:	verbose = verbose <  0 ?  0 : verbose + 1; break;
	case GO_LOGGING:	logging++; break;
	case GO_DE:		use_de = true; break;
	case GO_CT_CODE:	ctcode_enabled = true; break;
	case GO_LE_CODE:	lecode_enabled = true; break;
	case GO_COLORS:		err += ScanOptColorize(0,optarg,0); break;
	case GO_NO_COLORS:	opt_colorize = COLMD_OFF; break;

	case GO_CHDIR:		err += ScanOptChdir(optarg); break;
	case GO_CONST:		err += ScanOptConst(optarg); break;

	case GO_UTF_8:		use_utf8 = true; break;
	case GO_NO_UTF_8:	use_utf8 = false; break;

	case GO_TEST:		testmode++; break;
	case GO_FORCE:		force_count++; break;
	case GO_REPAIR_MAGICS:	err += ScanOptRepairMagic(optarg); break;

 #if OPT_OLD_NEW
	case GO_OLD:		opt_new = opt_new>0 ? -1 : opt_new-1; break;
	case GO_STD:		opt_new = 0; break;
	case GO_NEW:		opt_new = opt_new<0 ? +1 : opt_new+1; break;
 #endif
	case GO_EXTRACT:	opt_extract = optarg; break;

	case GO_ESC:		err += ScanEscapeChar(optarg) < 0; break;
	case GO_DEST:		SetDest(optarg,false); break;
	case GO_DEST2:		SetDest(optarg,true); break;
	case GO_OVERWRITE:	opt_overwrite = true; break;
	case GO_NUMBER:		opt_number = true; break;
	case GO_REMOVE_DEST:	opt_remove_dest = true; break;
	case GO_UPDATE:		opt_update = true; break;
	case GO_PRESERVE:	opt_preserve = true; break;
	case GO_IGNORE:		opt_ignore++; break;

	case GO_LOAD_BMG:	err += ScanOptLoadBMG(optarg); break;
	case GO_PATCH_BMG:	err += ScanOptPatchMessage(optarg); break;
	case GO_MACRO_BMG:	err += ScanOptMacroBMG(optarg); break;
	case GO_PATCH_NAMES:	opt_patch_names = true; break;

	case GO_ORDER_BY:	opt_order_by = optarg; break;
	case GO_ORDER_ALL:	opt_order_all = true; break;

	case GO_LE_DEFINE:	opt_le_define = optarg; break;
	case GO_LPAR:		opt_lpar = optarg; break;
	case GO_ALIAS:		err += ScanOptAlias(optarg); break;
	case GO_ENGINE:		err += ScanOptEngine(optarg); break;
	case GO_200CC:		err += ScanOpt200cc(optarg); break;
	case GO_PERFMON:	err += ScanOptPerfMon(optarg); break;
	case GO_CUSTOM_TT:	err += ScanOptCustomTT(optarg); break;
	case GO_XPFLAGS:	err += ScanOptXPFlags(optarg); break;

	case GO_TRACK_DIR:	opt_track_dest = optarg; break;
	case GO_COPY_TRACKS:	err += ScanOptTrackSource(optarg,TFMD_COPY); break;
	case GO_MOVE_TRACKS:	err += ScanOptTrackSource(optarg,TFMD_MOVE); break;
	case GO_MOVE1_TRACKS:	err += ScanOptTrackSource(optarg,TFMD_MOVE1); break;
	case GO_LINK_TRACKS:	err += ScanOptTrackSource(optarg,TFMD_LINK); break;

	case GO_COMPLETE:	opt_complete = true; break;

	case GO_MAX_FILE_SIZE:	err += ScanOptMaxFileSize(optarg); break;

	case GO_ROUND:		opt_round = true; break;
	case GO_LONG:		long_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_EXPORT:		export_count++; print_header = false; break;

	case GO_LT_CLEAR:	opt_lt_clear = true; break;
	case GO_LT_ONLINE:	err += ScanOptLtOnline(optarg); break;
	case GO_LT_N_PLAYERS:	err += ScanOptLtNPlayers(optarg); break;
	case GO_LT_COND_BIT:	err += ScanOptLtCondBit(optarg); break;
	case GO_LT_GAME_MODE:	err += ScanOptLtGameMode(optarg); break;
	case GO_LT_ENGINE:	err += ScanOptLtEngine(optarg); break;
	case GO_LT_RANDOM:	err += ScanOptLtRandom(optarg); break;
	case GO_LEX_PURGE:	opt_lex_purge = true; break;

 #if 0 // BMG disabled for wlect
	case GO_RAW:		raw_mode = true; break;
	case GO_BMG_ENCODING:	err += ScanOptBmgEncoding(optarg); break;
	case GO_BMG_INF_SIZE:	err += ScanOptBmgInfSize(optarg,false); break;
	case GO_BMG_MID:	err += ScanOptBmgMid(optarg); break;
	case GO_FORCE_ATTRIB:	err += ScanOptForceAttrib(optarg); break;
	case GO_DEF_ATTRIB:	err += ScanOptDefAttrib(optarg); break;
	case GO_NO_ATTRIB:	opt_bmg_no_attrib = true; break;
	case GO_X_ESCAPES:	opt_bmg_x_escapes = true; break;
	case GO_OLD_ESCAPES:	opt_bmg_old_escapes = true; break;
	case GO_SINGLE_LINE:	opt_bmg_single_line++; break;
	case GO_NO_BMG_COLORS:	opt_bmg_colors = 0; break;
	case GO_BMG_COLORS:	opt_bmg_colors = 2; break;
	case GO_NO_BMG_INLINE:	opt_bmg_inline_attrib = false; break;
 #endif

	case GO_NO_PARAM:	print_param = false; break;
	case GO_NO_ECHO:	opt_no_echo = true; break;
	case GO_SECTIONS:	print_sections++; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wlect,TRACE_FILE,11);
 #endif
    NormalizeOptions( verbose > 3 && !is_env ? 2 : 0 );
    SetupBMG(0);
    UsePatchingListBMG(&opt_load_bmg);

    return !err ? ERR_OK : max_error ? max_error : ERR_SYNTAX;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   check command                 ///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError CheckCommand ( int argc, char ** argv )
{
    const KeywordTab_t * cmd_ct = CheckCommandHelper(argc,argv,CommandTab);
    if (!cmd_ct)
	hint_exit(ERR_SYNTAX);

    TRACE("COMMAND FOUND: #%lld = %s\n",(u64)cmd_ct->id,cmd_ct->name1);
    current_command = cmd_ct;

    if (!allow_all)
    {
	enumError err = VerifySpecificOptions(&InfoUI_wlect,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wlect);

    if ( cmd_ct->id != CMD_ARGTEST )
    {
	argc -= optind+1;
	argv += optind+1;

	if ( cmd_ct->id == CMD_TEST )
	    while ( argc-- > 0 )
		AddParam(*argv++);
	else
	    while ( argc-- > 0 )
		AtFileHelper(*argv++,AddParam);
    }

    enumError err = ERR_OK;
    switch ((enumCommands)cmd_ct->id)
    {
	case CMD_VERSION:	version_exit();
	case CMD_HELP:		PrintHelp(&InfoUI_wlect,stdout,0,"HELP",0,URI_HOME,
					first_param ? first_param->arg : 0 ); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_EXPORT:	err = cmd_export(); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupVarsLECODE()); break;
	case CMD_FUNCTIONS:	SetupVarsLECODE(); err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupVarsLECODE()); break;
	case CMD_FLOAT:		err = cmd_float(); break;

	case CMD_DUMP:		err = cmd_dump(0); break;
	case CMD_DL:		err = cmd_dump(1); break;
	case CMD_DLL:		err = cmd_dump(2); break;
	case CMD_BIN_DIFF:	err = cmd_bin_diff(); break;
	case CMD_PATCH:		err = cmd_patch(); break;

	case CMD_CREATE:	err = cmd_create(); break;
	case CMD_CAT:		err = cmd_cat(); break;
	case CMD_LPAR:		err = cmd_lpar(); break;
	case CMD_DECODE:	err = cmd_convert(cmd_ct->id,"DECODE","\1P/\1N.txt"); break;
	case CMD_ENCODE:	err = cmd_convert(cmd_ct->id,"ENCODE","\1P/\1N\1?T"); break;

	// no default case defined
	//	=> compiler checks the existence of all enum values

	case CMD__NONE:
	case CMD__N:
	    help_exit(false);
    }

    return PrintErrorStat(err,verbose,cmd_ct->name1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   main()			///////////////
///////////////////////////////////////////////////////////////////////////////

#if SZS_WRAPPER
    int main_wlect ( int argc, char ** argv )
#else
    int main ( int argc, char ** argv )
#endif
{
    print_title_func = print_title;
    SetupLib(argc,argv,WLECT_SHORT);
    ctcode_enabled = true;
    lecode_enabled = true;

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WLECT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions("WLECT_OPT",CheckOptions);
    if (err)
	hint_exit(err);

    err = CheckOptions(argc,argv,false);
    if (err)
	hint_exit(err);

    err = CheckCommand(argc,argv);
    DUMP_TRACE_ALLOC(TRACE_FILE);

    if (SIGINT_level)
	err = ERROR0(ERR_INTERRUPT,"Program interrupted by user.");
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

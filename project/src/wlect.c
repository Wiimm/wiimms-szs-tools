
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

#include "lib-ledis.h"
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
	" " SYSTEM2 " - " AUTHOR " - " DATE

static const char autoname[] = "/course.lex";

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    help			///////////////
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    SetupPager();
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wlect,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wlect,stdout,0,0,"HELP",0,URI_HOME);

    ClosePager();
    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WLECT_SHORT,WLECT_LONG,long_count-1);
}

///////////////////////////////////////////////////////////////////////////////

static void version_exit()
{
    if ( brief_count > 1 )
	fputs( VERSION "\n", stdout );
    else if (brief_count)
	fputs( VERSION " r" REVISION " " SYSTEM2 "\n", stdout );
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
	    ProgInfo.progname, CommandInfo[current_command->id].name1 );
    else
	fprintf(stderr,
	    "-> Type '%s -h' or '%s help' (pipe it to a pager like 'less') for more help.\n\n",
	    ProgInfo.progname, ProgInfo.progname );
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

    le_cup_t cup;
    //InitializeLECUP(&cup,BMG_MAX_BCUP,BMG_BCUP_TRACKS); // battle setup
    InitializeLECUP(&cup,BMG_MAX_RCUP,BMG_RCUP_TRACKS); // versus setup

    for ( int i = -1; i < 26; i++ )
    {
	int i1 = GetIndexByRefLECUP(&cup,i);
	int r1 = GetRefByIndexLECUP(&cup,i1);
	int r2 = GetRefByIndexLECUP(&cup,i);
	int i2 = GetIndexByRefLECUP(&cup,r2);
	printf("%3d : %3d %3d : %3d %3d\n",i,i1,r1,r2,i2);
    }
    return ERR_OK;

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

    for ( ParamList_t *param = first_param; param; param = param->next )
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
///////////////			command dpad			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dpad()
{
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	while ( *arg && (uint)*arg <= ' ' )
	    arg++;

	bool hex_scanned = false;
	u32 num = 0;
	if ( *arg >= '0' && *arg <= '9' || *arg >= 'a' && *arg <= 'f' || *arg >= 'A' && *arg <= 'F' )
	{
	    char *end;
	    num = str2ul(arg,&end,16);
	    hex_scanned = num && !*end;
	}

	if (!hex_scanned)
	{
	    num = 0;
	    while (*arg)
	    {
		switch (*arg++)
		{
		    case 'U': case 'u': num = num << 2 | 0; break;
		    case 'L': case 'l': num = num << 2 | 1; break;
		    case 'R': case 'r': num = num << 2 | 2; break;
		    case 'D': case 'd': num = num << 2 | 3; break;
		}
	    }
	}

	char buf[20], *dest = buf + sizeof(buf) -1;
	*dest = 0;
	for ( u32 n = num; n && dest > buf; n >>= 2 )
	{
	    switch (n&3)
	    {
		case 0:
		    if ( *dest == 'U' || *dest == '.' )
		    {
			*dest = '.';
			*--dest = '.';
		    }
		    else
			*--dest = 'U';
		    break;

		case 1: *--dest = 'L'; break;
		case 2: *--dest = 'R'; break;
		case 3: *--dest = 'D'; break;
	    }
	}
	printf(" case 0x%08x: // %s\n",num,dest);
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command dump			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dump ( int long_level )
{
    SetupPager();

    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI_wlect,OPT_LONG,long_level,false);
	long_count += long_level;
	brief_count = 0;
    }
    else if ( long_level < 0 )
    {
	RegisterOptionByIndex(&InfoUI_wlect,OPT_BRIEF,-long_level,false);
	brief_count -= long_level;
	long_count = 0;
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
	    fprintf(stdlog,"\nDUMP %s:%s\n\n", GetNameFF(raw.fform,0), raw.fname );
	    fflush(stdlog);
	}

	if ( raw.fform == FF_LE_BIN )
	{
	    le_analyze_t ana;
	    AnalyzeLEBinary(&ana,raw.data,raw.data_size);
	    ApplyLEFile(&ana);
	    PatchLECODE(&ana);
	    DumpLEAnalyse(stdlog,1,&ana);
	    ResetLEAnalyze(&ana);
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
	CloseLDUMP();
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command bin-diff		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[lecode_bin_diff_t]]

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

static enumError LoadLECODE ( raw_data_t *raw, le_analyze_t *ana, ccp fname )
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
	err = ERROR0(ERR_WRONG_FILE_TYPE,
			"Not a LECODE binary file: %s:%s\n",
			GetNameFF(raw->fform,0), raw->fname);
	goto abort;
    }

    err = AnalyzeLEBinary(ana,raw->data,raw->data_size);
    if (err)
    {
	err = ERROR0(ERR_INVALID_DATA,"Invalid LECODE data: %s\n",raw->fname);
	goto abort;
    }

    return ERR_OK;

   abort:
    ResetLEAnalyze(ana);
    ResetRawData(raw);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static int BinDiffLECODE
	( lecode_bin_diff_t *bd, const le_analyze_t *a1, const le_analyze_t *a2 )
{
    DASSERT(bd);
    DASSERT(a1);
    DASSERT(a2);


    //--- clear "built on <DATE+TIME>"

    static const char search[] = "built on ";

    const int bodysize1 = a1->data_size - a1->param_offset;
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

    const int bodysize2 = a2->data_size - a2->param_offset;
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

    bd->stat_size = a1->data_size != a2->data_size;

    if ( h1->v3.version != h2->v3.version || a1->header_size != a2->header_size )
	bd->stat_header = true;
    else if ( h1->v3.version <= 4 )
	bd->stat_header = memcmp(h1,h2,sizeof(*h1)) != 0;
    else
    {
	DASSERT( h1->v3.version >= 5 );
	DASSERT(a1->header_size == a2->header_size );

	le_binary_head_v5_t *h5	= MEMDUP(a1->head,a1->header_size);
	h5->edit_version	= a2->head->v5.edit_version;
	h5->creation_time	= a2->head->v5.creation_time;
	h5->edit_time		= a2->head->v5.edit_time;
	bd->stat_header		= memcmp(h5,&a2->head->v5,a1->header_size) != 0;
	FREE(h5);
    }

    bd->stat_body
		=  bodysize1 < 0
		|| bodysize2 < 0
		|| bodysize1 != bodysize2
		|| memcmp( body1, body2, bodysize1 );

    bd->stat_timestamp	= h1->v3.version != h2->v3.version
			? true
			: h1->v3.version >= 5
			? a1->head->v5.creation_time != a2->head->v5.creation_time
			: strcmp(a1->head->v4.timestamp,a2->head->v4.timestamp) != 0;

    bd->stat_param = memcmp(&a1->lpar,&a2->lpar,sizeof(a1->lpar));

    bd->stat_data = a1->n_cup_track	!= a2->n_cup_track
		 || a1->n_cup_arena	!= a2->n_cup_arena
		 || a1->n_slot		!= a2->n_slot
		 || a1->flags_bits	!= a2->flags_bits;
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
		a1->data_size );
    PRINT0("CODE2: %zx..%zx + %zx..%x\n",
		body2 - a2->data,
		a2->beg_of_data - a2->data,
		a2->end_of_data - a2->data,
		a2->data_size );

    bd->stat_code
		=  body1 - a1->data		!= body2 - a2->data
		|| a1->beg_of_data - a1->data	!= a2->beg_of_data - a2->data
		|| a1->end_of_data - a1->data	!= a2->end_of_data - a2->data
		|| a1->data_size		!= a2->data_size
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
	    off2 = a1->data_size;
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
    le_analyze_t ana1;

    enumError err = LoadLECODE(&raw1,&ana1,param->arg);
    if (err)
	return err;


    //--- open file 2

    param = param->next;
    NORMALIZE_FILENAME_PARAM(param);

    raw_data_t raw2;
    le_analyze_t ana2;

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

    ResetLEAnalyze(&ana1);
    ResetLEAnalyze(&ana2);
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

	le_analyze_t ana;
	AnalyzeLEBinary(&ana,raw.data,raw.data_size);
	ApplyLEFile(&ana);
	err = PatchLECODE(&ana);
	if (!err)
	    err = SaveFILE( dest, 0, dest_is_source||opt_overwrite,
				raw.data, raw.data_size, 0 );
	ImportAnaLDUMP(&ana);
	ResetLEAnalyze(&ana);
	CloseLDUMP();
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command timestamp		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_timestamp()
{
    static ccp def_path = "\1P/\1F";
    CheckOptDest(def_path,false);

    const u_sec_t reftime = GetTimeSec(false);

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
		fprintf(stdlog,"%s%sSET TIMESTRAMP %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			ff_name, dest );
	    else
		fprintf(stdlog,"%s%sSET TIMESTRAMP %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			ff_name, raw.fname,
			ff_name, dest );
	    fflush(stdlog);
	}

	if ( raw.fform != FF_LE_BIN )
	    return ERROR0(ERR_INVALID_DATA,"Invalid file format: %s",raw.fname);

	le_analyze_t ana;
	AnalyzeLEBinary(&ana,raw.data,raw.data_size);
	bool patched = false;
	if ( !ana.creation_time && ana.header_vers >= 5 )
	{
	    u32 timestamp = reftime;
	    if ( ana.edit_time && timestamp > ana.edit_time )
		timestamp = ana.edit_time;
	    if ( timestamp >= ana.commit_time )
	    {
		ana.head->v5.creation_time = htonl(timestamp);
		patched = true;
	    }
	}

	if ( !err && ( !dest_is_source || patched ) )
	    err = SaveFILE( dest, 0, dest_is_source||opt_overwrite,
				raw.data, raw.data_size, 0 );
	ResetLEAnalyze(&ana);
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command create			///////////////
///////////////////////////////////////////////////////////////////////////////

static void help_create ( enumError exit_code )
{
    //SetupTermWidth();
    if ( logging >= 3 )
	printf("# colors: %d %d %d\n",colout->col_mode,colout->colorize,colout->n_colors);

    static const char text[] =
	"\n"
	"{heading|Command »%s CREATE«}\n"
	"\n"
	"  |This command creates text files."
	" The optional argument defines the type of output. The general syntax is:\n"
	"|[4,25]\n"
	"\t|{cmd|%s CREATE}\n"
	"\t|{cmd|%s CREATE SUB_COMMAND} {opt|option}...\n"
	"\n"
	"  |This help is printed if no {cmd|SUB_COMMAND} is used or if it is »{name|HELP}«."
	" The sub-commands are case sensitive, unique abbreviations are allowed."
	" The sub-commands are logically divided into several groups:\n"
	"\n"
	"\tCreate a LEX file:\t|{name|LEX}, {name|LEX+}, {name|SET1}, {name|CANNONS}, {name|HIPT}, {name|TEST}.\n"
	"\tCreate another file:\t|{name|LPAR}, {name|LE-DEF}, {name|PREFIX}, {name|CATEGORY}.\n"
	"\tPrint an information:\t|{name|LE-INFO}.\n"
 #if HAVE_WIIMM_EXTxx
	 "\tHidden sub-commands:\t|{name|DEVELOP}, {name|FEATURES}.\n"
 #endif
	"\n"
	"  |This help and the generated text files are output via the standard output (stdout)"
	" by an internal pager."
	" Options {opt|--dest}, {opt|--DEST} and {opt|--overwrite} can change this default.\n"
	"\n"

	//---------------------------------------------------------------------

	"\n"
	"{caption|Create a LEX file:}\n"
 #if HAVE_WIIMM_EXTxx
	"\n|[4,14]"
 #else
	"\n|[4,14]"
 #endif
	"  |The following sub-commands create a LEX file with one or all sections:\n"
	"\n"
	"\t{name|LEX}:\t|"
		"Create a LEX file with all known sections except section »TEST«.\n"
	"\t{name|LEX+}:\t|"
		"Create a complete LEX text file including section »TEST«.\n"
 #if HAVE_WIIMM_EXTxx
	"\t{info|DEVELOP}:\t|"
		"This {warn|hidden sub-command} acts like LEX+,"
		" but includes all devoloper sections.\n"
 #endif
	"\n"
	"\t{name|FEATURES}:\t|"
		"Create a LEX text file with section »FEAT« only.\n"
	"\t{name|SET1}:\t|"
		"Create a LEX text file with section »SET1« only.\n"
	"\t{name|CANNONS}:\t|"
		"Create a LEX text file with section »CANN« only.\n"
	"\t{name|HIPT}:\t|"
		"Create a LEX text file with section »HIPT« only.\n"
	"\t{name|TEST}:\t|"
		"Create a LEX text file with section »TEST« only.\n"
	"\n"

	//---------------------------------------------------------------------

	"\n"
	"{caption|Create other files:}\n"
	"\n"
	"\t{name|LPAR}:\t|"
		"Create a LPAR text file (LE-CODE parameters) as template."
		" Option {opt|--lpar} is recognized to change defaults.\n"
	"\t{name|LE-DEF}:\t|"
		"Create a LE-CODE distribution definition file of type LE-DEF as template.\n"
	"\t{name|PREFIX}:\t|"
		"Create a machine readable prefix list."
		" {file|https://ct.wiimm.de/export/prefix} is the authoritative source for this.\n"
	"\t{name|CATEGORY}:\t|"
		"Create a machine readable category list."
		" {file|https://ct.wiimm.de/export/category} is the authoritative source for this.\n"
	"\n"

	//---------------------------------------------------------------------

	"\n"
	"{caption|Print an information:}\n"
	"\n"
	"\t{name|LE-INFO}:\t|"
		"Print information about built-in LE-CODE binaries.\n"
	"\n"

	;

    int good_width = GetGoodTermWidth(0,false);
    PrintColoredLines(stdout,colout,0,good_width-1,0,0,text,
		ProgInfo.progname,
		ProgInfo.progname,
		ProgInfo.progname );
    ClosePager();
    exit(exit_code);
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_create()
{
    CheckOptDest("-",false);
    if (!strcmp(opt_dest,"-"))
	SetupPager();

    if ( n_param != 1 )
	help_create(ERR_OK);


    //--- data

    enum { C_HELP,
		C_LEX, C_FEATURES, C_SET1, C_CANNON, C_HIDE_PT, C_TEST,
		C_LPAR, C_LEDEF, C_PREFIX, C_CATEGORY,
		C_LEINFO,
    };

    static const KeywordTab_t tab[] =
    {
	{ C_HELP,	"HELP",		"H",		0 },

	{ C_LEX,	"LEX",		0,		0 },
	{ C_LEX,	"LEX+",		0,		 1 },
	{ C_LEX,	"DEVELOP",	0,		 2 },
	{ C_FEATURES,	"FEATURES",	0,		0 },
	{ C_SET1,	"SET1",		0,		0 },
	{ C_CANNON,	"CANNONS",	0,		0 },
	{ C_HIDE_PT,	"HIPT",		0,		0 },
	{ C_TEST,	"TEST",		0,		0 },

	{ C_LPAR,	"LPAR",		0,		0 },
	{ C_LEDEF,	"LE-DEF",	"LEDEF",	0 },
	{ C_PREFIX,	"PREFIX",	0,		0 },
	{ C_CATEGORY,	"CATEGORY",	0,		0 },

	{ C_LEINFO,	"LE-INFO",	"LEINFO",	0 },

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
     case C_HELP:
	help_create(ERR_OK);
	break;

     //-------------------------

     case C_LEX:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    lex.develop = cmd->opt > 1;
	    UpdateLEX( &lex, true, cmd->opt > 0 );
	    PatchLEX(&lex,0);
	    err = SaveTextLEX(&lex,opt_dest,false);
	    ResetLEX(&lex);
	}
	break;

     case C_FEATURES:
     case C_SET1:
     case C_CANNON:
     case C_HIDE_PT:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    switch (cmd->id)
	    {
	      case C_FEATURES: AppendFeatLEX(&lex,false,0); break;
	      case C_SET1:     AppendSet1LEX(&lex,false); break;
	      case C_CANNON:   AppendCannLEX(&lex,false); break;
	      case C_HIDE_PT:  AppendHiptLEX(&lex,false); break;
	    }
	    err = SaveTextLEX(&lex,opt_dest,false);
	    ResetLEX(&lex);
	}
	break;

     case C_TEST:
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    lex_item_t * li = AppendTestLEX(&lex,false);
	    PatchTestLEX((lex_test_t*)li->elem.data,0);
	    err = SaveTextLEX(&lex,opt_dest,false);
	    ResetLEX(&lex);
	}
	break;

     //-------------------------

     case C_LPAR:
	{
	    le_lpar_t lpar;
	    InitializeLPAR(&lpar,true);
	    PatchLPAR(&lpar);
	    lpar.limit_mode = LPM_FORCE_AUTOMATIC;
	    err = SaveTextLPAR(&lpar,opt_dest,false);
	}
	break;

     case C_LEDEF:
	{
	    File_t F;
	    enumError err = CreateFileOpt(&F,true,opt_dest,false,0);
	    if (!err)
	    {
		le_distrib_t ld;
		InitializeLD(&ld);
		AutoSetupLD(&ld,LAS_DEFAULT|LAS_TEMPLATE);
		err = CreateLeDefLD(F.f,&ld);
		ResetLD(&ld);
	    }
	    CloseFile(&F,0);
	}
	break;

     case C_PREFIX:
	SavePrefixTableFN(opt_dest,0);
	break;

     case C_CATEGORY:
	SaveCategoryListFN(opt_dest,0);
	break;

     //-------------------------

     case C_LEINFO:
	{
	    File_t F;
	    enumError err = CreateFileOpt(&F,true,opt_dest,false,0);
	    if (!err)
	    {
		fprintf(F.f,
			"\n"
			"Built-in LE-CODE binaries:\n"
			"\n"
			"  %s\n"
			"  %s\n"
			"  %s\n"
			"  %s\n"
			"\n"
			,GetLecodeInfoLEREG(LEREG_PAL)
			,GetLecodeInfoLEREG(LEREG_USA)
			,GetLecodeInfoLEREG(LEREG_JAP)
			,GetLecodeInfoLEREG(LEREG_KOR)
			);
	    }
	    CloseFile(&F,0);
	}
    }

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command distrib			///////////////
///////////////////////////////////////////////////////////////////////////////

static void help_distrib ( enumError exit_code )
{
    if ( logging >= 3 )
	printf("# colors: %d %d %d\n",colout->col_mode,colout->colorize,colout->n_colors);

    static const char text[] =
	"\n"
	"{heading|Command »%s DISTRIBUTION«}\n"
	"\n"
	"  |This command manages data for LE-CODE track distributions."
	" It reads any number of source files with different file types,"
	" collects the data and creates any number of files with different file types.\n"
	"\n"
	"  |»{cmd|%s DISTRIBUTION}« is a very powerful tool when dealing with distributions."
	" Because of the complexity, the input line offers many possibilities."
	" First, the default options (arguments beginning with 1 or 2 minus signs"
	" like {opt|-B} or {opt|--brief}) are evaluated."
	" Then the remaining arguments of the command line are evaluated step by step from left to right."
	" There are {name|input files} that change the internal model."
	" The {name|instructions} are used to change the data or to write data to files."
	" The {name|processing options} affect both reading and writing.\n"
	"\n"
	"  {syntax|Syntax:}\n"
	"|[4]\n"
	"\t|{cmd|%s DISTRIBUTION}\n"
	"\t|{cmd|%s DISTRIBUTION} {opt|option}... {par|argument}...\n"
	"\n"
	"  |»{cmd|DIS}« and »{cmd|DISTRIB}« are well defined shortcuts for »{cmd|DISTRIBUTION}«."
	" Without arguments, this help is printed.\n"
	"\n"

	//-------------------------------------------------------------------------

	"\n"
	"{heading|Arguments:}\n"
	"\n"
	"  |Arguments are divided into 3 groups:"
	" {name|processing options}, {name|instructions} and {name|filenames}.\n"
	"\n"
	"  |If an argument beginns with a {hl|plus sign} ({hl|+}), then is is scanned as"
	" a {name|comma-separated list of processing options}."
	" To use a filename that starts with a plus sign (e.g. »{file|+file.txt}«),"
	" precede it with »{file|./}« (e.g. »{file|./+file.txt}«).\n"
	"\n"
	"  |Otherwise, if the argument is of the form {hl|COMMAND=PARAMETER},"
	" then it is a {name|instruction}."
	" COMMAND is a keyword (case-insenitive) and starts with a letter"
	" followed by any number of letters, digits and minus signs."
	" PARAMETER must consist of at least one character."
	" If PARAMETER is empty, then the next argument is used as PARAMETER."
	" This also allows the syntax {hl|COMMAND= PARAMETER} (2 arguments),"
	" which is helpful for the automatic completion of filenames.\n"
	"\n"
	"  |Otherwise it is a {name|filename of an input file}."
	" The file is read and generally overwrites existing content."
	" This means that the file last read in has the highest priority."
	" However, this can be influenced by the processing options."
	" Arguments beginning with »{file|/}« or »{file|./}«"
	" are always recognized as filenames.\n"
	"\n"
	"  |All arguments are executed in the order in which they were entered without any logging."
	" Only error messages are displayed."
	" With option {opt|--verbose} (short: {opt|-v}),"
	"  at least one log line for each argument is printed.\n"
	"\n"
	"  |It is possible to store arguments into a file (e.g. into file »{file|param.txt}«)"
	" and to include those file by option {opt|-@FILENAME}"
	" (e.g. by »{file|-@param.txt}«). This option can be used multiple times"
	" and is evaluated before the actual analysis of the command line."
	" Command »{cmd|wlect argtest ...}« is suitable for tests.\n"
	"\n"

	//-------------------------------------------------------------------------

	"\n"
	"{heading|Input Files:}\n"
	"\n"
	"  |{name|Input files} are processed in the order in which they were entered."
	" They supplement or overwrite the internal data structure with its diverse data.\n"
	"\n"
	"  |If a file does not exist but the associated filename contains wildcards"
	" (any of »{file|*?[}«), then all matching files are loaded as source."
	" So it's possible to keep the command line small if using »{file|'*.szs'}«"
	" (with apostrophs) instead of »{file|*.szs}«.\n"
	"\n"
	"  |The following file types are recognized and processed."
	" The names used here are the same as those given out"
	" by the command {cmd|FILETYPE} for identification:\n"
	"|[4,13]\n"
	"\t{name|LEDIS}:\t|"
		"A distribution dump created by {name|write instruction} »{par|DUMP=…}«."
		" Such a dump usually contains all relevant data of the internal model,"
		" which is restored by reading it in.\n"
	"\n"
	"\t{name|LEDEF}:\t|"
		"A distribution definition file."
		" This new format replaces the old {name|CTTEXT} format."
		" It supports all LE-CODE properties and will be further developed to match LE-CODE."
		" Templates can be created with instruction »{par|LEDEF=…}«"
		" or with command »{par|wlect CREATE LEDEF}«.\n"
	"\n"
	"\t{name|LEREF}:\t|"
		"A track reference list created by {name|write instruction} »{par|REF=…}«.\n"
	"\n"
	"\t{name|SHA1REF}:\t|"
		"A SHA1 reference list created by {name|write instruction} »{par|SHA1=…}«."
		" Only checksum and track slot are used.\n"
	"\n"
	"\t{name|LE-BIN}:\t|"
		"A LE-CODE binary file (e.g. »{file|lecode-PAL.bin}«)."
		" Usually settings and LPAR are imported."
		" Option {name|IN-LECODE} decides if the file is used as template"
		" for the specifig region if a binary is created.\n"
	"\n"
	"\t{name|PREFIX}:\t|"
		"Replace the internal prefix list by the content of the file."
		" {file|https://ct.wiimm.de/export/prefix} is the authoritative source for this.\n"
	"\n"
	"\t{name|MTCAT}:\t|"
		"Replace the internal track category list by the content of the file."
		" {file|https://ct.wiimm.de/export/category} is the authoritative source for this.\n"
	"\n"
	"\t{name|LPAR}:\t|"
		"A LE-CODE parameter file (e.g. »{file|lpar.txt}«).\n"
	"\n"
	"\t{name|BMG} and {name|BMGTXT}:\n\t\t|"
		"Any BMG (binary or text)."
		" See section »{heading|Processing Options}« for more details.\n"
	"\n"
	"\t{name|DISTRIB}:\t|"
		"A distribution file created by {name|write instruction} »{par|DISTRIB=…}«"
		" or by command »{cmd|wszst DISTRIBUTION}«."
		" Such files are imported by ct.wiimm.de to display information about distributions.\n"
	"\n"
	"\t{name|CTTEXT}:\t|"
		"A CT-CODE or LE-CODE definition file."
		" The file is scanned by the CT-CODE scanner and then imported.\n"
	"\n"
	"\t{name|*.szs}:\t|"
		"Track files are added to 2 different internal track tracks lists,"
		" one for racing tracks and one for battle areans."
		" I the argument is quoted then wlect will resolve the wildcards by itself." 
	"\n"
	"  |All file types that are accepted by option {opt|--le-define}"
	" ({name|BRRES}, {name|TEX0}, {name|CT-CODE}, ...) are also possible.\n"
	"\n"

	//-------------------------------------------------------------------------

	"\n"
	"{heading|Instructions:}\n"
	"\n"
	"  |{name|Instructions} are of the form {hl|COMMAND=PARAMETER} (1 argument)"
	" or {hl|COMMAND= PARAMETER} (2 arguments). Commands are case-insenitive."
	" The use of keywords without the minus sign and unique abbreviations are permitted."
	" Any number of write instructions can be used."
	" And these can be mixed with input files at will.\n"
	"\n"
	"  |Most commands, but not all, are {name|write instructions}."
	" They write the current contents of the internal model to a file."
	" In these cases, {hl|PARAMETER} is a filename."
	" If a filename is a minus sign only, then {file|stdout} is used."
	" Existing files are only overwritten if at least one of the options"
	" {opt|--overwrite} (short: {opt|-o}) or {opt|--remove-dest} (short: {opt|-r}) is set.\n"
	"\n|[4,14]"

	//-------------------------
	"  |{heading|Create human readable information files:}\n"
	"\n"

	"\t{name|NAMES}:\t|"
		"Create a human readable reference file with cup info and track names."
		" Only tracks with known name (not empty) are printed."
		" The tracks are ordered by cups.\n"
	"\t{name|XNAMES}:\t|"
		"Same as {name|NAMES}, but use extended names if available.\n"
	"\n"
	"\t{name|INFO}:\t|"
		"Create a human readable reference file with cup info slots, flags and track names."
		" Only tracks with known name (not empty) are printed."
		" The tracks are ordered by cups.\n"
	"\t{name|XINFO}:\t|"
		"Same as {name|INFO}, but use extended names if available.\n"
	"\n"
	"\t{name|RATING}:\t|"
		"Same as {name|INFO}, but with additional first column »{par|()}«"
		" to rate the tracks. The tracks are ordered by cups.\n"
	"\t{name|XRATING}:\t|"
		"Same as {name|RATING}, but use extended names if available.\n"
	"\n"
	"\t{name|PREFIX}:\t|"
		"Print a machine readable prefix list."
		" {file|https://ct.wiimm.de/export/prefix} is the authoritative source for this.\n"
	"\n"

	//-------------------------
	"  |{heading|Create human and machine readable definition files (usually input files):}\n"
	"\n"

	"\t{name|LE-DEF}:\t|"
		"Create a LE-CODE definition file of format »{name|#LE-DEF1}«,"
		" that is usually used to declare tracks for a LE-CODE distributions."
		" This new format replaces the old format created by {name|CTDEF=}."
		" It supports all LE-CODE properties and will be further developed"
		" to match future LE-CODE.\n"
	"\n"
	"\t{name|CT-DEF}:\t|"
		"Create a definition file of format »{name|#CT-CODE}«,"
		" that is usually used to declare tracks for a CT-CODE or LE-CODE distributions."
		" Only simple layouts are supported,"
		" so that the track arrangement can be distorted when importing again."
		" In addition, a file is created"
		" that can only be read by the tools since version 2.28a.\n"
	"\n"


	//-------------------------
	"  {heading|Generate machine-readable files that can also be used as input files:}\n"
	"\n"

	"\t{name|SHA1}:\t|"
		"Create a SHA1 list file that can be used on Wiimmfi"
		" to limit the tracks that can be used in a region."
		" Only tracks with known SHA1 are printed."
		" The output can be used as input file to restore the checksums.\n"
	"\t{name|XSHA1}:\t|"
		"Same as {name|SHA1}, but use extended names if available.\n"
	"\n"
	"\t{name|DISTRIB}:\t|"
		"Create a distribution file like command »{cmd|wszst DISTRIBUTION}« does."
		" However, this variant only supports tracks with a known track slot."
		" The output can be used as input file.\n"
	"\t{name|XDISTRIB}:\t|"
		"Same as {name|DISTRIB}, but use extended names for the track list if available.\n"
	"\n"
	"\t{name|LPAR}:\t|"
		"Create a LPAR file. Use {opt|--brief} ({opt|-b}) to suppress comments.\n"
	"\n"
	"\t{name|BMG}:\t|"
		"Create a BMG binary file. BMG options are recognized."
		" But if the output goes to a terminal, then use instruction {name|BMGTXT} instead."
		" If no {heading|BMG selector} is defined,"
		" then {name|MKW,LE-CODE} (all except {name|CT-CODE}) is used.\n"
	"\t{name|BMGTXT}:\t|"
		"Create a BMG text file. Global BMG options are recognized.\n"
	"\n"
	"\t{name|REF}:\t|"
		"Create a machine readable track reference (type {name|LEREF}),"
		" that can be used as input file for other commands."
		" One line is printed for each defined track.\n"
	"\n"
	"\t{name|DUMP}:\t|"
		"Create a dump of the complete internal model to a distribution file"
		" (type {name|LE-DIS})."
		" This file can be used as source to restore the internal model.\n"
	"\n"

	//-------------------------
	"  {heading|Analyze and report:}\n"
	"\n"
	"\t{name|REPORT}:\t|"
		"Analyse the current distribution and print a report."
		" Therefor count track and arena types and find duplicate names, tracks, families and clans."
		"\r\r"
		"  {heading|Syntax:} {syntax|REPORT '=' [OPTIONS] '=' FILE}"
		"\r\r"
		"{syntax|OPTIONS} is a comma separated list of keywords to select the kind of analysis."
		" If no option is set, all are enabled."
		" To find duplicate tracks, families or clans, the definition file must support SHA1"
		" checksums and a SHA1 reference file must be loaded."
		" You can get the reference file from {file|https://ct.wiimm.de/export/sha1ref/view}."
		"\n\n"
		"|[17,29]"
		"\t{par|counters}:\t|Count track and arena types in different ways.\n"
		"\t{par|names}:\t|Find track names that are used"
			" by two or more distinct tracks or arenas.\n"
		"\t{par|tracks}:\t|Find tracks that are used two or more times and list them.\n"
		"\t{par|families}:\t|Find tracks that are in the same family and list them.\n"
		"\t{par|clans}:\t|Find tracks that are in the same clan and list them.\n"
		"\t{par|duplicates}:\t|Short cut for »{par|names,tracks,families,clans}«.\n"
		"\t{par|all}:\t|Short cut for all of above and default if no option is set."
			" To exclude something, you can write e.g. »{par|all,-clans}«,"
			" or shorter »{par|-clans}«.\n"
	"\n|[4,15]"
	"\t{name|DEBUG}:\t|"
		"Print some statistics for debugging."
		" The current state is analysed without updating cups.\n"
	"\n"
	"\t{name|LE-INFO}:\t|"
		"Print information about current LE-CODE binaries.\n"
	"\n"

	//-------------------------
	"  {heading|Cup-Icon support:}\n"
	"\n|[4,15]"
	"\t{name|CUP-ICONS}:\t|"
		"Create an image file with cup icons."
		"\r\r"
		"  {heading|Syntax:} {syntax|CUP-ICON '=' [STORAGE,] [OPTIONS] '=' FILE}"
		"\r\r"
		"If the output goes to a terminal, then use instruction {name|CUP-INFO} instead."
		" To determine the file type, the file extension is analyzed."
		" Examples are »{par|*.tpl}« or »{par|*.png}«."
		"\r\r"
		"The generic icons consist of a red cup index at the top-right and the first"
		" 5 characters of the name without a prefix, shown in blue at the bottom."
		" If a name is wider than 128 pixels, than it is horizontal shrinked to 128 pixels."
		"\r\r"
		"{syntax|STORAGE} is an optional storage indicator for the source"
		" explained in section »{name|Processing Options: Storage}«."
		" Separate STORAGE and OPTIONS by a comma."
		" If not set, then {name|NAME} is used as source."
		"\r\r"
		"{syntax|OPTIONS} is a comma separated list of keywords:\n"
		"|[17,27]"
		"\t{par|original}:\t|Add 8 original cup icons.\n"
		"\t{par|swapped}:\t|Add 8 original cup icons in swapped order.\n"
		"\t{par|1wiimm}:\t|Use Wiimms avatar for the first cup, but only if original icons are not used.\n"
		"\t{par|9wiimm}:\t|Use Wiimms avatar for the ninth cup.\n"
		"\t{par|xwiimm}:\t|Use Wiimms avatar for the last cup.\n"
		"\t{par|plus}:\t|If a plus prefix exists, then insert it and an additional space.\n"
		"\t{par|xplus}:\t|Same as option {par|plus},"
			" but apppend an underline character instead of a space after the plus prefix."
			" Underline characters are printed like spaces, but they are ignored by option {par|space}.\n"
		"\t{par|game}:\t|If a game prefix exists, then insert it and an additional space.\n"
		"\t{par|xgame}:\t|Same as option {par|game},"
			" but apppend an underline character instead of a space after the game prefix.\n"
		"\t{par|space}:\t|Finish the name part at first space."
			" Ignore tabulators and undercore characters for this.\n"
		"\t{par|0} .. {par|15}:\t|"
			"Define the maximum number of charaters for the name part"
			" to any value between 0 and 15. The default is 5 characters.\n"
	"\n|[4,15]"
	"\t{name|CUP-INFO}:\t|"
		"Same as {name|CUP-ICONS}, but create a text file"
		" with a job list for tool {cmd|wimgt} and its generic file {par|:cup-file=FILE}."
		" The output is machine and human readable.\n"
	"\n|[4,14]"

	//-------------------------
	"  {heading|Create LE-CODE binary files:}\n"
	"\n"

	"\t{name|LECODE}:\t|"
		"Create 4 LE-CODE binary files, one for each region."
		" The filename must contain at least {hl|one »@« character}."
		" The last »@« character is replaced by"
		" {name|PAL}, {name|USA}, {name|JAP} and {name|KOR} in sequence."
		" The binary data embedded in the SZS tools is usually used as a template."
		" At the moment it is {info|build %s}."
		" Other templates can be used with the {name|IN-LECODE} processing option.\n"
	"\n"
	"\t{name|PAL}:\t|"
		"Like instruction {name|LECODE}, but only the PAL version is created."
		" The »@« character has no special meaning here.\n"
	"\t{name|USA}:\t|"
		"Like instruction {name|PAL}, but the USA version is created.\n"
	"\t{name|JAP}:\t|"
		"Like instruction {name|PAL}, but the JAP version is created.\n"
	"\t{name|KOR}:\t|"
		"Like instruction {name|PAL}, but the KOR version is created.\n"
	"\n"

	//-------------------------
	"  {heading|String functions:}\n"
	"\n"

	"\t{name|SEPARATOR}:| "
		"Define a separator string that is used by other commands."
		" The default is a space.\r"
		"  {heading|Syntax:} {syntax|SEPARATOR '=' STRING}  or  {syntax|SEP '=' STRING}\n"
	"\n"

	"\t{name|COPY}:\t|"
		"This instruction copies the texts of 1 or more sources to a destination.\r"
		"  {heading|Syntax of PARAMETER:} {syntax|DEST '=' SRC [ '+' SRC ]...}\r"
		"\r"
		"{syntax|DEST} and {syntax|SRC} are storage indicators"
		" explained in section »{name|Processing Options: Storage}«."
		" The sources are joined textually, with a separator"
		" (defined by instruction {name|SEPARATOR}, see above)"
		" inserted between the sources.\r"
		"\r"
		"Example: |{syntax|copy=[result]=sha1+[size]}\n"
	"\n"

	"\t{name|SPLIT}:\t|"
		"This instruction analyses the {syntax|SOURCE} like command {cmd|wszst split} does it."
		" Then the directives of {syntax|FORMAT} create a string that is copied"
		" to {syntax|DESTINATION}.\r"
		"  {heading|Syntax of PARAMETER:} {syntax|DESTINATION ',' SOURCE ',' FORMAT}\r"
		"Visit {file|https://szs.wiimm.de/opt/printf} for more details.\n"
	"\n"

	"\t{name|SUBST}:\t|"
		"This instruction searches for text passages using a regular expression"
		" and replaces the found passages with another text.\r"
		"  {heading|Syntax of PARAMETER:} |{syntax|STORAGE ',' SEP REGEXP SEP REPLACEMENT SEP OPTIONS}\n"
		"\n\t\t|"
		"{syntax|STORAGE} is a storage indicator"
		" explained in section »{name|Processing Options: Storage}«."
		" {syntax|SEP} is the very first character behind the comma."
		" It must occur exactly 3 times and separates the 3 parts from each other."
		" {syntax|REGEXP} is the extended regular expression used for searching."
		" If successful, replace that portion matched with {syntax|REPLACEMENT}."
		" The replacement may contain the special string »{mark|$0}«"
		" to refer to that portion of the pattern space which matched,"
		" and the special string1 »{mark|$1}« through »{mark|$9}«"
		" to refer to the corresponding matching sub-expressions in the regexp."
		" The two characters »{mark|g}« for global (replace all occurrences)"
		" and »{mark|i}« for ignore case are recognized as {syntax|OPTIONS}.\r"
		"\r"
		"Examples: |{syntax|subst=name,+abc+xyz+}\r"
			"{syntax|subst=[key],/search ([0-9])/replace $1/gi}\n"
	"\n"

	//-------------------------
	"  {heading|Copy, move or link track files:}\n"
	"\n|[4,12,14,22]"

	"\t{name|TRACKS}:\t|"
		"Copy, move or links files following the options {opt|--copy-tracks},"
		" {opt|--move-tracks}, {opt|--move1-tracks}, {opt|--link-tracks}"
		" and {opt|--track-dir}."
		" {hl|PARAMETER} is a keyword that specifies the options for execution:\n"
	"\n"

	"\t\t\t{name|NO-LOG}:\t|"
		"Transfer the tracks without logging.\n"
	"\t\t\t{name|LOG}:\t|"
		"Transfer the tracks with logging.\n"
	"\t\t\t{name|TEST}:\t|"
		"Log planned actions only and don't touch any file.\n"
	"\n"

	//-------------------------------------------------------------------------

	"\n"
	"{heading|Processing Options:}\n"
	"\n"
	"  |{name|Processing Options} are inserted as comma separated list."
	" A list always begins with a plus sign to distinguish it from filenames"
	" and instructions."
	" Option names are case-insenitive. Unique abbreviations are allowed."
	" Names with a minus sign in their name can also be specified without the minus sign.\n"
	"\n"
	"  |Most option names can be preeceded by a minus sign or a slash to negate its meaning."
	"\n"
	"  |{name|Processing Options} are only valid for subsequent arguments."
	" This makes it possible to use different filters for different input or output files."
	" The keywords are divided into several functional groups.\n"
	"\n|[4,16]"

	//-------------------------

	"  |{heading|Managment Options:}\n"
	"\n"
	"\t{name|HELP}:\t|"
		"Stop execution and print this help.\n"
	"\t{name|RESET}:\t|"
		"Reset the filter to its default.\n"
	"\n"
	"\t{name|CUT-ALL}:\t|"
		"Remove all tracks and their settings.\n"
	"\t{name|CUT-STD}:\t|"
		"Remove tracks and their settings, that are not needed for a standard distribution"
		" with 32 tracks and 10 battle arenas.\n"
	"\t{name|CUT-CTCODE}:\t|"
		"Remove tracks and their settings, that are not needed for a CT-CODE distribution."
		" Arenas are also removed.\n"
	"\n"

	//-------------------------

	"  |{heading|BMG selector:}\n"
	"\n"
	"\t|Define which BMG strings are used as input."
	" Multiple source can be selected. If none is selected, then all are used."
	" {name|MKW1} has the lowest priority and {name|LE-CODE} the highest.\n"
	"\n"

	"\t{name|MKW1}:\t|"
		"Use the first set of orignal names starting from"
		" message id 0x2454 (racing tracks) and 0x2490 (battle arenas).\n"
	"\t{name|MKW2}:\t|"
		"Use the second set of orignal names starting from"
		" message id 0x24b8 (racing tracks) and 0x24cc (battle arenas).\n"
	"\t{name|MKW}:\t|"
		"Short cut for »{name|+MKW1,MKW2}«.\n"
	"\t{name|CT-CODE}:\t|"
		"Use the CT-CODE names starting from message id 0x4000.\n"
	"\t{name|LE-CODE}:\t|"
		"Use the LE-CODE names starting from message id 0x7000.\n"
	"\n"

	//-------------------------

	"  |{heading|Input filters:}\n"
	"\n"
	"\t|Define the type of character identifiers accepted as source."
	" Empty character strings and character strings"
	" that only consist of a minus sign are considered invalid.\n"
	"\n"

	"\t{name|IN-EMPTY}:\t|"
		"Empty strings are also considered valid."
		" {name|IEMPTY} is a short cut for this keyword.\n"
	"\t{name|IN-MINUS}:\t|"
		"Strings consisting only of a minus sign are also considered valid."
		" {name|IMINUS} is a short cut for this keyword.\n"
	"\t{name|IN-ALL}:\t|"
		"Short cut for »{name|+IN-EMPTY,IN-MINUS}«."
		" {name|IALL} is a short cut for this keyword.\n"
	"\n"

	//-------------------------

	"  |{heading|Input operation:}\n"
	"\n"
	"\t|Define how already existing strings are overwritten.\n"
	"\n"

	"\t{name|OVERWRITE}:\t|"
		"Insert all valid BMG strings and overwrite already existing strings."
		" This is the default.\n"
	"\t{name|INSERT}:\t|"
		"Insert only valid BMG strings, that are empty in the track list.\n"
	"\t{name|REPLACE}:\t|"
		"Replace only valid BMG strings that are already defined in the track list."
		" Other strings are ignored.\n"
	"\n"

	//-------------------------

	"  |{heading|Storage:}\n"
	"\n"
	"\t|Define the string type being processed. This is used for BMG input and output files only.\n"
	"\n"
	"\t|The storge names are also used as a reference in other instructions."
	" In this case, the reference can be preceded by an at symbol ({name|@})."
	" This is mandatory if the storage can be specified optionally"
	" and does not begin with a bracket ({name|[}).\n"
	"\n"

	"\t{name|IDENT}:\t|"
		"Define a new identfication string."
		" If it is a valid SHA1 or DB64 string, then update SHA1 too."
		" If reading, then get the identfication string.\n"
	"\t{name|SHA1}:\t|"
		"Same as {name|IDENT} if writing."
		" If reading, then get the SHA1 string.\n"
	"\t{name|FILE}:\t|"
		"Define a new or get the filename.\n"
	"\t{name|NAME}:\t|"
		"Define a new or get the standard name.\n"
	"\t{name|XNAME}:\t|"
		"Define a new or get the extended name.\n"
	"\t{name|X2NAME}:\t|"
		"Define a new standard and a new extended name."
		" If reading, then get the extended name, if valid, or the standard name as fallback.\n"
	"\n"
	"\t{name|TEMP1}:\t|"
		"A temporary variable with no specific purpose."
		" Access is much faster than for dynamic strings by {name|[key]}."
		" They are therefore intended for multi-level string manipulations.\n"
	"\t{name|TEMP2}:\t|"
		"A second temporary variable.\n"
	"\n"
	"\t{name|[key]}:\t|"
		"For each track there is a set of character strings at the user's disposal."
		" The character strings are addressed via KEY."
		" The KEY itself can consist of any character and are case-sensitive."
		" This type of square bracket option can only be used"
		" directly after the leading plus sign. More processing options may follow.\n"
	"\n"

	//-------------------------

	"  |{heading|Output filters:}\n"
	"\n"
	"\t|Select tracks for the output by their features.\n"
	"\n"

	"\t{name|VERSUS}||{name|VS}:\t|"
		"Select versus tracks only.\n"
	"\t{name|BATTLE}||{name|BT}:\t|"
		"Select battle tracks (arenas) only."
		" If neither or both of {name|VERSUS} and {name|BATTLE} set, both types are selected.\n"
	"\n"
	"\t{name|CUSTOM}:\t|"
		"Select custom tracks."
		" An original track in the wrong slot will also be considered CUSTOM.\n"
	"\t{name|ORIGINAL}:\t|"
		"Select original tracks at correct track slot only."
		" Original tracks are detected by their SHA1."
		" If neither or both of {name|CUSTOM} and {name|ORIGINAL} set, both types are selected.\n"
	"\n"
	"\t{name|NO-D}:\t|"
		"This option suppresses information about so-called '_d' files."
		" This affects 2 sub-commands:"
		" With the {name|REF} sub-command, the data fields relating to '_d' files remain empty,"
		" and with {name|SHA1}, no lines are output for '_d' tracks.\n"
	"\n"

	//-------------------------

	"  |{heading|Empty output strings:}\n"
	"\n"
	"\t|For sub-commands {name|(X)NAMES} and {name|(X)INFO},"
	" only tracks that have a valid name are output."
	" Empty character strings and character strings"
	" that only consist of a minus sign are considered invalid.\n"
	"\n"

	"\t{name|OUT-EMPTY}:\t|"
		"Empty strings are also considered valid."
		" {name|OEMPTY} is a short cut for this keyword.\n"
	"\t{name|OUT-MINUS}:\t|"
		"Strings consisting only of a minus sign are also considered valid."
		" {name|OMINUS} is a short cut for this keyword.\n"
	"\t{name|OUT-DUMMY}:\t|"
		"Dummy names are considered valid (default)."
		" A dummy name consists of an underscore followed by a three-digit hex number"
		" (lower case only). This name is assigned when importing a LE-CODE binary file"
		" due to a lack of alternatives."
		" {name|ODUMMY} is a short cut for this keyword.\n"
	"\t{name|/OUT-DUMMY}:\t|"
		"Switch option {name|OUT-DUMMY} off so that dummy names are considered invalid.\n"
	"\t{name|OUT-ALL}:\t|"
		"Short cut for »{name|+OUT-EMPTY,OUT-MINUS,OUT-DUMMY}«."
		" {name|OALL} is a short cut for this keyword.\n"
	"\n"

	//-------------------------

	"\n"
	"  |{heading|More options:}\n"
	"\n"
	"\t{name|IN-LECODE}:\t|"
		"When reading a LE-CODE binaray file (e.g. »{file|lecode-PAL.bin}«),"
		" only the settings are imported, but not the binary itself."
		" So if writing a LE-CODE binary then the region dependent built-in binary is used."
		" If this option set, then binaries are imported to override the built-in versions."
		" One reason to activate this option is to use more recent LE-CODE versions."
		" {name|ILECODE} is a short cut for this keyword.\n"
	"\t{name|/IN-LECODE}:\t|"
		"Switch option {name|IN-LECODE} off.\n"
	"\n"
	"\t{name|NO-SLOT}:\t|"
		"Suppress »{name|SLOT <index>}« lines when creating a LE-CODE defintion file ({name|LE-DEF})."
		" {name|NOSLOT} is a short cut for this keyword.\n"
	"\t{name|/NO-SLOT}:\t|"
		"Switch option {name|NO-SLOT} off.\n"
	"\n"
	"\t{name|BRIEF}:\t|"
		"When creating text files, detailed descriptions are suppressed."
		" The default is based on the global options {opt|--no-header} and {opt|--brief}.\n"
	"\t{name|/BRIEF}:\t|"
		"Switch option {name|BRIEF} off.\n"
	"\n"

	//-------------------------------------------------------------------------

 #if 0 // [[2do]]
	"\n"
	"{heading|Examples:}\n"
	"\n"
	"  |....\n"
	"\n"
 #endif

	//-------------------------------------------------------------------------

	"\n"
	"{heading|Built-in LE-CODE binaries:}\n"
	"\n"
	"  |%s\r%s\r%s\r%s\n"
	"\n"

	;

 #if 0
    char wildcards[] = PATTERN_WILDCARDS;
    for ( char *ptr = wildcards; *ptr; ptr++ )
	if ( *ptr == ' ' )
	    *ptr = 0xa0;
 #endif

    int good_width = GetGoodTermWidth(0,false);
    PrintColoredLines(stdout,colout,0,good_width-1,0,0,text
		,ProgInfo.progname
		,ProgInfo.progname
		,ProgInfo.progname
		,ProgInfo.progname
	//	,wildcards
		,GetLecodeBuildLEREG(LEREG_PAL)
		,GetLecodeInfoLEREG(LEREG_PAL)
		,GetLecodeInfoLEREG(LEREG_USA)
		,GetLecodeInfoLEREG(LEREG_JAP)
		,GetLecodeInfoLEREG(LEREG_KOR)
		);
    ClosePager();
    exit(exit_code);
}

///////////////////////////////////////////////////////////////////////////////

static void PrintHead ( ccp format, ... )
	__attribute__ ((__format__(__printf__,1,2)));

static void PrintHead ( ccp format, ... )
{
    if (stdlog)
    {
	char buf[500];
	va_list arg;
	va_start(arg,format);
	vsnprintf(buf,sizeof(buf),format,arg);
	va_end(arg);

	ASSERT(collog);
	fprintf(stdlog,"%s▼ %s%s\n",collog->caption,buf,collog->reset);
	fflush(stdlog);
    }
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_distrib_instruction ( le_distrib_t *ld, ccp mode, char * arg )
{
    u_nsec_t start_nsec = GetTimerNSec();

    enum { C_NAMES, C_INFO, C_RATING, C_LEINFO, C_SHA1, C_DISTRIB,
		C_CTDEF, C_LEDEF, C_LEREF, C_STRINGS, C_DUMP, 
		C_LECODE, C_LECODE4, C_LPAR,
		C_BMG, C_CUPICON, C_SEPARATOR, C_COPY, C_SPLIT, C_SUBST,
		C_TRACKS, C_SORT, C_PREFIX, C_RESERVE, C_DEBUG, C_REPORT };

    enum
    {
	O_PARAM_OPTSRC	= 0x0100, // scan optional storage
	O_PARAM_D	= 0x0200, // scan storage DEST
	O_PARAM_S	= 0x0400, // scan storage SRC
	O_PARAM_S2	= 0x0800, // scan second storage SRC
	O_PARAM_SS	= 0x0c00, // scan storage SRC [ + SRC ]...
	O_PARAM_OPT1	= 0x1000, // 
	O_PARAM_OPT2	= 0x2000, // scan options before additional '='
	O_PARAM_A	= 0x4000, // allow additonal arguments
    };

    static const KeywordTab_t tab[] =
    {
	{ C_NAMES,	"NAMES",		0,		0  },
	{ C_NAMES,	"XNAMES",		0,		 1 },
	{ C_INFO,	"INFO",			0,		0  },
	{ C_INFO,	"XINFO",		0,		 1 },
	{ C_RATING,	"RATING",		0,		0  },
	{ C_RATING,	"XRATING",		0,		 1 },
	{ C_LEINFO,	"LE-INFO",		"LEINFO",	0 },

	{ C_SHA1,	"SHA1",			0,		0  },
	{ C_SHA1,	"XSHA1",		0,		 1 },
	{ C_DISTRIB,	"DISTRIBUTION",		0,		0  },
	{ C_DISTRIB,	"XDISTRIBUTION",	0,		 1 },

	{ C_CTDEF,	"CT-DEF",		"CTDEF",	0  },
	{ C_LEDEF,	"LE-DEF",		"LEDEF",	0  },
	 { C_LEDEF,	"DEFINITION",		"DEF",		0  },
	{ C_LEREF,	"LE-REF",		"LEREF",	0  },
	 { C_LEREF,	"REFERENCE",		"REF",		0  },
	{ C_STRINGS,	"STRINGS",		"STR",		0  },
	{ C_DUMP,	"DUMP",			0,		0  },

	{ C_LECODE,	"PAL",			0,		LEREG_PAL },
	{ C_LECODE,	"USA",			0,		LEREG_USA },
	{ C_LECODE,	"JAP",			0,		LEREG_JAP },
	{ C_LECODE,	"KOR",			0,		LEREG_KOR },
	{ C_LECODE4,	"LECODE",		0,		0  },

	{ C_LPAR,	"LPAR",			0,		0  },

	{ C_BMG,	"BMG",			"BMGBIN",	0  },
	{ C_BMG,	"BMG-TXT",		"BMGTXT",	 1 },

	{ C_CUPICON,	"CUP-ICONS",		"CUPICONS",	O_PARAM_OPTSRC|O_PARAM_OPT2  },
	{ C_CUPICON,	"CUP-INFO",		"CUPINFO",	O_PARAM_OPTSRC|O_PARAM_OPT2 | 1 },

	{ C_SEPARATOR,	"SEPARATOR",		"SEP",		0 },
	{ C_COPY,	"COPY",			0,		O_PARAM_D|O_PARAM_SS },
	{ C_SPLIT,	"SPLIT",		0,		O_PARAM_D|O_PARAM_S|O_PARAM_A },
	{ C_SUBST,	"SUBST",		0,		O_PARAM_D|O_PARAM_A },

	{ C_TRACKS,	"TRACKS",		0,		0  },
	{ C_SORT,	"SORT",			0,		O_PARAM_OPTSRC|O_PARAM_OPT1 },

	{ C_PREFIX,	"PREFIX",		0,		0  },
	{ C_RESERVE,	"RESERVE",		0,		0  },
	{ C_DEBUG,	"DEBUG",		0,		0  },
	{ C_REPORT,	"REPORT",		0,		O_PARAM_OPT2  },

	{ 0,0,0,0 }
    };

    enum { O_LOG = 1, O_TEST = 2 };
    static const KeywordTab_t tracks_tab[] =
    {
	{ 0,		"NO-LOG",		"NOLOG",	0  },
	{ 0,		"LOG",			0,		O_LOG  },
	{ 0,		"TEST",			0,		O_LOG|O_TEST  },
	{ 0,0,0,0 }
    };


    if ( !ld || !mode  || !arg || !*arg )
	return ERROR0(ERR_MISSING_PARAM,0);


    //--- scan keyword

    int cmd_stat;
    const KeywordTab_t *cmd = ScanKeyword(&cmd_stat,mode,tab);
    if (!cmd)
    {
	PrintHead("Instruction: %s = %s",mode,arg);
	PrintKeywordError(tab,mode,cmd_stat,0,"instruction");
	hint_exit(ERR_SYNTAX);
    }


    //--- start

    if ( verbose >= 1 )
	PrintHead("Instruction %s: %s",cmd->name1,arg);

    u_nsec_t temp_nsec = GetTimerNSec();
    if ( CloseArchLD(ld) && verbose >= 2 )
    {
	const u_nsec_t dur = GetTimerNSec() - temp_nsec;
	fprintf(stdlog,"%s   >> SZS archive closed in %s%s\n",
		collog->status, PrintTimerNSec6(0,0,dur,0), collog->reset );
	start_nsec += dur;
    }


    //--- text argument only

    const int MAX_PAR_SRC = 20;
    int n_par_src = 0;
    le_strpar_t par_opt, par_dest, par_src[MAX_PAR_SRC];
    mem_t mem_opt = {0};

    enumError err = ERR_OK;
    const le_options_t opt = cmd->opt;

    if ( opt & O_PARAM_OPTSRC )
	arg = ScanOptionalLSP(&par_opt,arg,LTT_NAME,&err);

    if ( opt & (O_PARAM_D|O_PARAM_S) )
    {
	if ( opt & O_PARAM_D )
	{
	    par_dest = ld->spar;
	    char *end = ScanLSP(&par_dest,arg,&err);
	    if ( opt & (O_PARAM_S|O_PARAM_A) )
	    {
		if ( *end != '=' && *end != ',' )
		    return ERROR0( ERR_SEMANTIC,"%s: Missing '=' or ',': %s\n",cmd->name1,end);
		end++;
	    }

	    if (err)
	    {
	     err_param12:;
		return ERROR0( ERR_SEMANTIC,
			    "%s: Invalid string name: %s\n", cmd->name1, arg );
	    }
	    arg = end;
	    if ( verbose >= 2 )
		fprintf(stdlog,"   > %s%s %s\n",
			    cmd->name1,
			    opt & O_PARAM_S ? " DEST:   " : ":",
			    GetOptionsLSP(&par_dest,LEO__DEST));
	}

	if ( opt & O_PARAM_S )
	{
	    const int max = opt & O_PARAM_S2 ? MAX_PAR_SRC : 1;
	    le_strpar_t *src = par_src;
	    while ( n_par_src < max )
	    {
		*src = ld->spar;
		char *end = ScanLSP(src,arg,&err);
		if (err)
		    goto err_param12;
		if ( verbose >= 2 )
		    fprintf(stdlog,"   > %s SRC[%02u]: %s\n",
			    cmd->name1, n_par_src+1, GetOptionsLSP(src,LEO__SRC));
		n_par_src++;
		src++ ;
		arg = end;
		if ( *arg != '+' && *arg != ',' )
		    break;
		arg++;
	    }
	}

	if ( *arg && !(opt&O_PARAM_A) )
	    return ERROR0( ERR_SEMANTIC,"%s: Missing end-of-line: %s\n",cmd->name1,arg);
    }

    if ( opt & O_PARAM_OPT1 )
    {
	mem_opt.ptr = arg;
	mem_opt.len = strlen(arg);
	arg += mem_opt.len;
    }
    else if ( opt & O_PARAM_OPT2 )
    {
	char *eq = strchr(arg,'=');
	if (eq)
	{
	    mem_opt.ptr = arg;
	    mem_opt.len = eq - arg;
	    arg = eq;
	    *arg++ = 0;
	}
    }


    //--- text argument only

    err = ~0;
    switch (cmd->id)
    {
	case C_LECODE4:		err = CreateLecode4LD(arg,ld); break;
	case C_SEPARATOR:	err = SeparatorLD(ld,arg); break;
	case C_COPY:		err = CopyLD(ld,&par_dest,par_src,n_par_src); break;
	case C_SPLIT:		err = SplitLD(ld,&par_dest,par_src,arg); break;
	case C_SUBST:		err = SubstLD(ld,&par_dest,arg); break;
	case C_SORT:		err = SortTracksLD(ld,&par_opt,mem_opt); break;

	case C_TRACKS:
	    cmd = ScanKeyword(&cmd_stat,arg,tracks_tab);
	    if (!cmd)
	    {
		PrintHead("Instruction: %s = %s",mode,arg);
		PrintKeywordError(tab,arg,cmd_stat,0,"TRACKS keyword");
		hint_exit(ERR_SYNTAX);
	    }
	    err = TransferFilesLD(ld,cmd->opt&O_LOG,cmd->opt&O_TEST);
	    break;

	case C_RESERVE:
	    {
		le_track_type_t ltty = LTTY_TRACK;
		char ch = tolower(*arg);
		if ( ch == 'a' || ch == 'b' )
		{
		    ltty = LTTY_ARENA;
		    arg++;
		}
		else if ( ch == 't' || ch == 'v' )
		    arg++;

		char *end;
		int n1 = str2ul(arg,&end,10);
		if ( end > arg )
		{
		    int n2 = 1;
		    if ( *end == ',' || *end == 'x' )
			n2 = str2ul(end+1,0,10);
		    ccp ltty_name = ltty==LTTY_TRACK ? "versus track" : "battle arena";
		    PRINT0("RESERVE %s %dx%d\n", ltty_name, n1, n2 );
		    if ( n2 > 0 )
		    {
			if ( verbose >= 1 )
			    fprintf(stdlog,"   > Reserve %dx%d %s%s\n",
					n1, n2, ltty_name, n1*n2 == 1 ? "" : "s" );
			while (	n1-- > 0 )
			{
			    le_track_t *lt = ReserveTracksLD(ld,ltty,n2,false);
			    if (!lt)
				break;
			    if ( verbose >= 1 )
			    {
				if ( n2 > 1 )
				    fprintf(stdlog,"     > Slots %d to %d reserved for %u %ss.\n",
					lt->track_slot, lt->track_slot + n2 -1, n2, ltty_name );
				else
				    fprintf(stdlog,"     > Slot %d reserved for a %s.\n",
					lt->track_slot, ltty_name );
			    }
			}
		    }    
		}
	    }
	    err = ERR_OK;
	    break;
    }


    //-- with opened file

    if ( err == ~0 )
    {
     #ifdef __CYGWIN__
	char norm[PATH_MAX];
	NormalizeFilenameCygwin(norm,sizeof(norm),arg);
	if ( verbose >= 2 && strcmp(arg,norm) )
	    fprintf(stdlog,"   > Normalized filename: %s\n",norm);
	arg = norm;
     #endif

	File_t F;
	err = CreateFileOpt(&F,true,arg,false,0);
	if (!err)
	{
	    ld_out_param_t lop = { .fname = arg, .f = F.f,
					.colset = GetFileColorSet(F.f), .ld = ld };
	    PRINT0("%sCOLOR-SET: %d,%d,%d  istty=%d,%d%s\n",
			lop.colset->status,
			lop.colset->col_mode, lop.colset->colorize, lop.colset->n_colors,
			isatty(fileno(F.f)), isatty(fileno(stdout)),
			lop.colset->reset );

	    const bool variant = cmd->opt & 1; // use xnames || force text mode
	    switch (cmd->id)
	    {
	      case C_NAMES:   err = CreateNamesLD(F.f,ld,variant); break;
	      case C_INFO:    err = CreateInfoLD(F.f,ld,variant,false); break;
	      case C_RATING:  err = CreateInfoLD(F.f,ld,variant,true); break;
	      case C_LEINFO:  err = CreateLeInfoLD(F.f,ld,false); break;
	      case C_SHA1:    err = CreateSha1LD(F.f,ld,variant,true); break;
	      case C_DISTRIB: err = CreateDistribLD(F.f,ld,variant); break;

	      case C_CTDEF:   err = CreateCtDefLD(F.f,ld); break;
	      case C_LEDEF:   err = CreateLeDefLD(F.f,ld); break;
	      case C_LEREF:   err = CreateRefLD(F.f,ld,true); break;
	      case C_STRINGS: err = CreateStringsLD(F.f,ld); break;
	      case C_DUMP:    err = CreateDumpLD(F.f,ld); break;

	      case C_LECODE:  err = CreateLecodeLD(F.f,ld,cmd->opt); break;
	      case C_LPAR:    err = CreateLparLD(F.f,ld,true); break;
	      case C_BMG:     err = CreateBmgLD(F.f,ld,variant||F.is_stdio); break;
	      case C_CUPICON: err = CreateCupIconsLD(F.f,ld,lop.fname,&par_opt,mem_opt,variant||F.is_stdio); break;
	      case C_PREFIX:  err = SavePrefixTable(F.f,0); break;
	      case C_DEBUG:   err = CreateDebugLD(F.f,ld); break;
	      case C_REPORT:  err = CreateReportLD(&lop,mem_opt); break;
	    }
	}
	CloseFile(&F,0);
    }

    if ( verbose >= 2 )
    {
	const u_nsec_t dur = GetTimerNSec() - start_nsec;
	fprintf(stdlog,"%s   >> %s done in %s%s\n",
		collog->status, cmd->name1,
		PrintTimerNSec6(0,0,dur,0), collog->reset );
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_distrib()
{
    stdlog = stderr;
    SetupPager();
    if (!n_param)
	help_distrib(ERR_OK);

    CheckOptDest("-",false);

    if ( verbose >= 1 )
	fputc('\n',stdlog);

    le_distrib_t ld;
    InitializeLD(&ld);

    for ( ParamList_t *param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	ccp arg = param->arg;

	if ( *arg == '+' )
	{
	    if ( verbose >= 1 )
		PrintHead("Scan options: %s",arg);
	    ScanOptionsLSP(&ld.spar,arg+1,"Filter option");
	    if ( ld.spar.opt & LEO_HELP )
		help_distrib(ERR_OK);
	    if ( ld.spar.opt & LEO_M_CUT )
	    {
		CutLD(&ld,ld.spar.opt);
		ld.spar.opt &= ~LEO_M_CUT;
	    }
	    if ( verbose >= 2 )
		fprintf(stdlog,"   > Options: %s\n",GetOptionsLSP(&ld.spar,LEO__ALL));
	    continue;
	}

	char *fname = strchr(arg,'=');
	if ( fname && isalpha((int)*arg) )
	{
	    ccp p = arg;
	    while ( isalnum((int)*p) || *p == '-' )
		p++;
	    if ( p == fname )
	    {
		*fname++ = 0;
		if (!*fname)
		{
		    if (param->next)
		    {
			param = param->next;
			NORMALIZE_FILENAME_PARAM(param);
			fname = param->arg;
		    }
		}
		cmd_distrib_instruction(&ld,arg,fname);
		continue;
	    }
	}

	if ( verbose >= 1 )
	    PrintHead("Read file(s): %s",arg);
	const u_nsec_t start_nsec = GetTimerNSec();
	ImportFileLD(&ld,arg,true,false);
	if ( verbose >= 2 )
	{
	    const u_nsec_t dur = GetTimerNSec() - start_nsec;
	    fprintf(stdlog,"%s   >> File(s) read in %s%s\n",
		collog->status, PrintTimerNSec6(0,0,dur,0), collog->reset );
	}
    }

    ClosePager();
    return ProgInfo.max_error;
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
		output_mode.lecode = true;
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

	le_analyze_t ana;
	AnalyzeLEBinary(&ana,raw.data,raw.data_size);
	ApplyLEFile(&ana);
	PatchLECODE(&ana);
	SaveTextLPAR(&ana.lpar,"-",false);
	ResetLEAnalyze(&ana);
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
	case GO_CONFIG:		opt_config = optarg;
	case GO_ALLOW_ALL:	allow_all = true; break;
	case GO_COMPATIBLE:	err += ScanOptCompatible(optarg); break;
	case GO_WIDTH:		err += ScanOptWidth(optarg); break;
	case GO_MAX_WIDTH:	err += ScanOptMaxWidth(optarg); break;
	case GO_NO_PAGER:	opt_no_pager = true; break;
	case GO_QUIET:		verbose = verbose > -1 ? -1 : verbose - 1; break;
	case GO_VERBOSE:	verbose = verbose <  0 ?  0 : verbose + 1; break;
	case GO_LOGGING:	logging++; break;
	case GO_EXT_ERRORS:	ext_errors++; break;
	case GO_TIMING:		log_timing++; break;
	case GO_WARN:		err += ScanOptWarn(optarg); break;
	case GO_DE:		use_de = true; break;
	case GO_CT_CODE:	ctcode_enabled = true; break;
	case GO_LE_CODE:	lecode_enabled = true; break; // optional argument ignored
	case GO_COLORS:		err += ScanOptColorize(0,optarg,0); break;
	case GO_NO_COLORS:	opt_colorize = COLMD_OFF; break;

	case GO_CHDIR:		err += ScanOptChdir(optarg); break;
	case GO_CONST:		err += ScanOptConst(optarg); break;

	case GO_UTF_8:		use_utf8 = true; break;
	case GO_NO_UTF_8:	use_utf8 = false; break;

	case GO_TEST:		testmode++; break;
	case GO_FORCE:		force_count++; break;
	case GO_REPAIR_MAGICS:	err += ScanOptRepairMagic(optarg); break;
	case GO_CREATE_DISTRIB:	EnableLDUMP(optarg); break;

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

	case GO_LOAD_PREFIX:	err += ScanOptLoadPrefix(optarg); break;
	case GO_LOAD_CATEGORY:	err += ScanOptLoadCategory(optarg); break;
	case GO_PLUS:		opt_plus = optarg; break;

	case GO_LOAD_BMG:	err += ScanOptLoadBMG(optarg); break;
	case GO_PATCH_BMG:	err += ScanOptPatchMessage(optarg); break;
	case GO_MACRO_BMG:	err += ScanOptMacroBMG(optarg); break;
	case GO_PATCH_NAMES:	opt_patch_names = true; break;

	case GO_ORDER_BY:	opt_order_by = optarg; break;
	case GO_ORDER_ALL:	opt_order_all = true; break;

	case GO_LE_DEFINE:	ScanOptLeDefine(optarg); break;
	case GO_LE_ARENA:	opt_le_arena = optarg; break;
	case GO_LPAR:		opt_lpar = optarg; break;
	case GO_ALIAS:		err += ScanOptAlias(optarg); break;
	case GO_ENGINE:		err += ScanOptEngine(optarg); break;
	case GO_200CC:		err += ScanOpt200cc(optarg); break;
	case GO_PERFMON:	err += ScanOptPerfMon(optarg); break;
	case GO_CUSTOM_TT:	err += ScanOptCustomTT(optarg); break;
	case GO_XPFLAGS:	err += ScanOptXPFlags(optarg); break;
	case GO_SPEEDOMETER:	err += ScanOptSpeedometer(optarg); break;
	case GO_DEBUG:		err += ScanOptDebug(optarg); break;

	case GO_TRACK_DIR:	opt_track_dest = optarg; break;
	case GO_COPY_TRACKS:	err += ScanOptTrackSource(optarg,-1,TFMD_COPY); break;
	case GO_MOVE_TRACKS:	err += ScanOptTrackSource(optarg,-1,TFMD_MOVE); break;
	case GO_MOVE1_TRACKS:	err += ScanOptTrackSource(optarg,-1,TFMD_MOVE1); break;
	case GO_LINK_TRACKS:	err += ScanOptTrackSource(optarg,-1,TFMD_LINK); break;
	case GO_SZS_MODE:	err += ScanOptSzsMode(optarg); break;

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
	case GO_LEX_RM_FEAT:	opt_lex_rm_features = true; break;
	case GO_LEX_PURGE:	opt_lex_purge = true; break;

 #if 0 // BMG disabled for wlect
	case GO_RAW:		raw_mode = true; break;
	case GO_BMG_ENDIAN:	err += ScanOptBmgEndian(optarg); break;
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
    NormalizeOptions( verbose > 3 && !is_env );
    SetupBMG(0);
    UsePatchingListBMG(&opt_load_bmg);

    return !err ? ERR_OK : ProgInfo.max_error ? ProgInfo.max_error : ERR_SYNTAX;
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
	case CMD_HELP:		PrintHelpColor(&InfoUI_wlect); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_EXPAND:	err = cmd_expand(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_EXPORT:	err = cmd_export(); break;
	case CMD_DPAD:		err = cmd_dpad(); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupVarsLECODE()); break;
	case CMD_FUNCTIONS:	SetupVarsLECODE(); err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupVarsLECODE()); break;
	case CMD_FLOAT:		err = cmd_float(); break;

	case CMD_DUMP:		err = cmd_dump(0); break;
	case CMD_DB:		err = cmd_dump(-1); break;
	case CMD_DL:		err = cmd_dump(1); break;
	case CMD_DLL:		err = cmd_dump(2); break;
	case CMD_DLLL:		err = cmd_dump(3); break;
	case CMD_BIN_DIFF:	err = cmd_bin_diff(); break;
	case CMD_PATCH:		err = cmd_patch(); break;
	case CMD_TIMESTAMP:	err = cmd_timestamp(); break;

	case CMD_CREATE:	err = cmd_create(); break;
	case CMD_DISTRIBUTION:	err = cmd_distrib(); break;
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
 #if !SZS_WRAPPER
    ArgManager_t am = {0};
    SetupArgManager(&am,LOUP_AUTO,argc,argv,false);
    ExpandAtArgManager(&am,AMXM_SHORT,10,false);
    argc = am.argc;
    argv = am.argv;
 #endif

    tool_name = "wlect";
    print_title_func = print_title;
    SetupLib(argc,argv,WLECT_SHORT,VERSION,TITLE);
    ctcode_enabled = true;
    lecode_enabled = true;


    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WLECT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions2("WLECT_OPT",CheckOptions);
    if (err)
	hint_exit(err);

    err = CheckOptions(argc,argv,false);
    if (err)
	hint_exit(err);

    err = CheckCommand(argc,argv);
    DUMP_TRACE_ALLOC(TRACE_FILE);

    if (SIGINT_level)
	err = ERROR0(ERR_INTERRUPT,"Program interrupted by user.");
    ClosePager();
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


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

#include "lib-ctcode.h"
#include "lib-ledis.h"
#include "lib-szs.h"
#include "lib-xbmg.h"
#include "lib-bzip2.h"

#include "ui.h" // [[dclib]] wrapper
#include "ui-wctct.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WCTCT_SHORT ": " WCTCT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM2 " - " AUTHOR " - " DATE

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
	    PrintHelpCmd(&InfoUI_wctct,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wctct,stdout,0,0,"HELP",0,URI_HOME);

    ClosePager();
    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WCTCT_SHORT,WCTCT_LONG,long_count-1);
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

    printf("  output-mode: %16x = %12d\n",output_mode.mode,output_mode.mode);
    printf("  hex:         %16x = %12d\n",output_mode.hex,output_mode.hex);
    printf("  print-header:%16x = %12d\n",print_header,print_header);
    printf("  brief:       %16x = %12d\n",brief_count,brief_count);
    printf("  export:      %16x = %12d\n",export_count,export_count);
    printf("  CT/LE modes:     %s\n",GetCtLeInfo());

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
    SetupVarsCTCODE();
    return ExportHelper("ctcode");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command dump			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dump()
{
    raw_data_t raw;
    InitializeRawData(&raw);

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	fprintf(stdlog,"\nDUMP %s:%s\n", GetNameFF(raw.fform,0), raw.fname );
	fflush(stdlog);

	ctcode_t ctcode;
	err = ScanRawDataCTCODE(&ctcode,CTM_AUTO,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	DumpCTCODE(stdout,2,&ctcode);
	ResetCTCODE(&ctcode);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    commands cat + bmg + tracks		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_cat ( uint cmd ) // cmd_bmg() cmd_tracks()
{
    stdlog = stderr;
    raw_data_t raw;
    InitializeRawData(&raw);

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,0,opt_ignore>0,0);
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

	ctcode_t ctcode;
	err = ScanRawDataCTCODE(&ctcode,CTM_AUTO,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    switch (cmd)
	    {
		case CMD_BMG:
		    err = SaveMessageCTCODE(&ctcode,"-",false);
		    break;

		case CMD_TRACKS:
		    err = SaveTrackListByOptCTCODE(&ctcode,"-",0);
		    break;

		default:
		    err = SaveTextCTCODE(&ctcode,"-",false,&output_mode);
		    break;
	    }
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetCTCODE(&ctcode);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command decode			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_decode()
{
    static ccp def_path = "\1P/\1F\1?T";
    CheckOptDest(def_path,false);

    raw_data_t raw;
    InitializeRawData(&raw);

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),arg,opt_dest,def_path,
			GetExtFF(FF_CTDEF,0),false);

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%s%sDECODE %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(raw.fform,0), raw.fname,
			GetNameFF(FF_CTDEF,0), dest );
	    fflush(stdlog);
	}

	ctcode_t ctcode;
	err = ScanRawDataCTCODE(&ctcode,CTM_AUTO,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = SaveTextCTCODE(&ctcode,dest,opt_preserve,&output_mode);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetCTCODE(&ctcode);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command create			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_create()
{
    if ( n_param < 1 )
    {
	ERROR0(ERR_SYNTAX,"Missing sub command for CREATE.\n");
	hint_exit(ERR_SYNTAX);
    }


    //----- find lang prefix

    static const KeywordTab_t reg_tab[] =
    {
	{ CTL_EU,	"EU",		"PAL",		0 },
	{ CTL_EU,	"P",		"RMCP",		0 },

	{ CTL_US,	"US",		"USA",		0 },
	{ CTL_US,	"E",		"RMCE",		0 },

	{ CTL_JP,	"JP",		"JAP",		0 },
	{ CTL_JP,	"J",		"RMCJ",		0 },

	{ CTL_KO,	"KO",		"KOR",		0 },
	{ CTL_KO,	"K",		"RMCK",		0 },

	{0,0,0,0},
    };

    ParamList_t *param = first_param;
    ccp cmd_name = param->arg;
    param = param->next;

    char prefix[10];
    StringCopyS(prefix,sizeof(prefix),cmd_name);
    char *found = strchr(prefix,'-');
    if (found)
	*found = 0;
    found = strchr(prefix,'.');
    if (found)
	*found = 0;

    int cmd_stat;
    const KeywordTab_t *cmd = ScanKeyword(&cmd_stat,prefix,reg_tab);
    int len;
    for ( len = 4; len >=2 && !cmd; len-- )
    {
	prefix[len] = 0;
	cmd = ScanKeyword(&cmd_stat,prefix,reg_tab);
    }

    ctlang_save_t lang = CTL_ANY;
    if (cmd)
    {
	lang = cmd->id;
	cmd_name += strlen(prefix);
	if ( *cmd_name == '-' || *cmd_name == '.' )
	    cmd_name++;
    }
    noPRINT("PREFIX: %u |%s|%s|\n",lang,prefix,cmd_name);


    //----- find sub command

    static const KeywordTab_t tab[] =
    {
	{ CTS_CT0CODE,	"BAD0CODE",	"BOOT_CODE",	1 },
	{ CTS_CT0DATA,	"BAD0DATA",	"BOOT_DATA",	1 },
	{ CTS_CT1CODE,	"BAD1CODE",	"CODE",		1 },
	{ CTS_CT1DATA,	"BAD1DATA",	"DATA",		1 },

	{ CTS_CUP1,	"CUP1",		0,		0 },
	{ CTS_CRS1,	"CRS1",		0,		0 },
	{ CTS_MOD1,	"MOD1",		0,		1 },
	{ CTS_MOD2,	"MOD2",		0,		1 },
	{ CTS_OVR1,	"OVR1",		0,		1 },

	{ CTS_TEX0,	"TEX0",		0,		1 },
	{ CTS_BRRES,	"BRRES",	0,		1 },
	{ CTS_SZS,	"SZS",		0,		1 },

	{ CTS_BMG,	"BMG",		0,		0 },
	{ CTS_TRACKS,	"TRACKS",	0,		0 },
	{ CTS_LIST,	"LIST",		0,		0 },
	{ CTS_REF,	"REF",		0,		0 },
	{ CTS_FULL,	"FULL",		0,		0 },

	{ 0,0,0,0 }
    };

    cmd = ScanKeyword(&cmd_stat,cmd_name,tab);
    if (!cmd)
    {
	PrintKeywordError(tab,cmd_name,cmd_stat,0,"sub command");
	hint_exit(ERR_SYNTAX);
    }


    //--- verify language

    if (!cmd->opt)
	lang = CTL_ANY;
    else if ( lang == CTL_ANY )
	return ERROR0(ERR_SYNTAX,
		"Missiang language code (eg. 'EU' or 'RMCP') before keyword: %s\n",
		cmd_name );


    //--- parameter setup

    ccp def_path = 0;
    file_format_t dest_ff1 = FF_UNKNOWN;
    file_format_t dest_ff2 = FF_UNKNOWN;
    switch(cmd->id)
    {
	case CTS_CT0CODE:
	    dest_ff2  = FF_CT0_CODE;
	    def_path = "\1P/\1F\1?T";
	    break;

	case CTS_CT0DATA:
	    dest_ff2  = FF_CT0_DATA;
	    def_path = "\1P/\1F\1?T";
	    break;

	case CTS_CT1CODE:
	    dest_ff2  = FF_CT1_CODE;
	    def_path = "\1P/\1F\1?T";
	    break;

	case CTS_CT1DATA:
	    dest_ff2  = FF_CT1_DATA;
	    def_path = "\1P/\1F\1?T";
	    break;

	case CTS_CUP1:
	    dest_ff2  = FF_CUP1;
	    def_path = "\1P/\1N.bin";
	    break;

	case CTS_CRS1:
	    dest_ff2  = FF_CRS1;
	    def_path = "\1P/\1N.bin";
	    break;

	case CTS_MOD1:
	    dest_ff2  = FF_MOD1;
	    def_path = "\1P/\1N.bin";
	    break;

	case CTS_MOD2:
	    dest_ff2  = FF_MOD2;
	    def_path = "\1P/\1N.bin";
	    break;

	case CTS_OVR1:
	    dest_ff2  = FF_OVR1;
	    def_path = "\1P/\1N.bin";
	    break;

	case CTS_TEX0:
	    dest_ff2  = FF_TEX_CT;
	    def_path = "\1P/\1F\1?T";
	    break;

	case CTS_BRRES:
	    dest_ff2  = FF_BRRES;
	    def_path = "\1P/\1N.brres";
	    break;

	case CTS_SZS:
	    dest_ff1  = FF_YAZ0;
	    dest_ff2  = FF_BRRES;
	    def_path = "\1P/\1N.szs";
	    break;

	case CTS_BMG:
	    dest_ff2  = FF_BMG_TXT;
	    def_path = "\1P/\1F\1?T";
	    break;

	case CTS_TRACKS:
	    dest_ff2  = FF_BMG_TXT;
	    def_path = "\1P/\1N.list";
	    break;

	case CTS_LIST:
	case CTS_REF:
	case CTS_FULL:
	    dest_ff2  = FF_CTDEF;
	    def_path = "\1P/\1F\1?T";
	    break;
    }

    CheckOptDest(def_path,false);

    raw_data_t raw;
    InitializeRawData(&raw);


    //--- file loop

    StringField_t plist = {0};
    CollectExpandParam(&plist,param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,0,opt_ignore>0,FF_CTDEF);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),arg,opt_dest,def_path,
			GetExtFF(dest_ff1,dest_ff2),false);

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%s%sCREATE %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(raw.fform,0), raw.fname,
			GetNameFF(dest_ff1,dest_ff2), dest );
	    fflush(stdlog);
	}

	ctcode_t ctcode;
	err = ScanRawDataCTCODE(&ctcode,CTM_AUTO,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = SaveCTCODE(&ctcode,dest,opt_preserve,cmd->id,lang,&output_mode);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetCTCODE(&ctcode);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return ERR_OK;
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

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;


	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),arg,opt_dest,def_path,
			GetExtFF(raw.fform_file,raw.fform),false);
	const bool dest_is_source = !strcmp(dest,arg);

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

	ctcode_t ctcode;
	err = ScanRawDataCTCODE(&ctcode,CTM_AUTO,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    PrepareSaveCTCODE(&ctcode);
	    if (IsCompressedFF(raw.fform_file))
	    {
		szs_file_t szs;
// [[fname-]]
		AssignSZS(&szs,true,raw.data,raw.data_size,false,raw.fform,0);
		err = SaveSZS(&szs,dest,dest_is_source||opt_overwrite,true);
	    }
	    else
		err = SaveFILE( dest, 0, dest_is_source||opt_overwrite,
				raw.data, raw.data_size, 0 );
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetCTCODE(&ctcode);
    }

    ResetStringField(&plist);
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
    InitializeOutputMode(&output_mode);

    optind = 0;
    int err = 0;
    ccp filter_bmg = 0;

    for(;;)
    {
      const int opt_stat = getopt_long(argc,argv,OptionShort,OptionLong,0);
      if ( opt_stat == -1 )
	break;

      RegisterOptionByName(&InfoUI_wctct,opt_stat,1,is_env);

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
	case GO_COLORS:		err += ScanOptColorize(0,optarg,0); break;
	case GO_NO_COLORS:	opt_colorize = COLMD_OFF; break;

	case GO_CT_CODE:	ctcode_enabled = true; break;
	case GO_LE_CODE:	lecode_enabled = true; break; // optional argument ignored
	case GO_OLD_SPINY:	old_spiny = true; break;
	case GO_CRS1:		err += ScanOptCRS1(optarg); break;

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

	case GO_LIST:		output_mode.mode = 1; break;
	case GO_REF:		output_mode.mode = 2; break;
	case GO_FULL:		output_mode.mode = 3; break;
	case GO_HEX:		output_mode.hex = true; break;
	case GO_LOAD_BMG:	err += ScanOptLoadBMG(optarg); break;
	case GO_PATCH_BMG:	err += ScanOptPatchMessage(optarg); break;
	case GO_MACRO_BMG:	err += ScanOptMacroBMG(optarg); break;
	case GO_FILTER_BMG:	filter_bmg = optarg; break;
	case GO_PATCH_NAMES:	opt_patch_names = true; break;

	case GO_CT_DIR:		AppendStringField(&ct_dir_list,optarg,false); break;
	case GO_CT_LOG:		err += ScanOptCtLog(optarg); break;
	case GO_ALLOW_SLOTS:	err += ScanOptAllowSlots(optarg); break;
	case GO_IMAGES:		opt_image_dir = optarg; break;
	case GO_ORDER_BY:	opt_order_by = optarg; break;
	case GO_ORDER_ALL:	opt_order_all = true; break;
	case GO_DYNAMIC:	opt_dynamic_ctcode = true; break;
	case GO_WRITE_TRACKS:	opt_write_tracks = optarg; break;

	case GO_MAX_FILE_SIZE:	err += ScanOptMaxFileSize(optarg); break;
	case GO_TRACKS:		err += ScanOptTracks(optarg); break;
	case GO_ARENAS:		err += ScanOptArenas(optarg); break;

	case GO_ROUND:		opt_round = true; break;
	case GO_LONG:		long_count++; break;
	case GO_NO_HEADER:	print_header = output_mode.syntax = false; break;
	case GO_BRIEF:		brief_count++; output_mode.cross_ref = false; break;
	case GO_NO_WILDCARDS:	no_wildcards_count++; break;
	case GO_IN_ORDER:	inorder_count++; break;
	case GO_RAW:		raw_mode = true; break;
	case GO_EXPORT:
		export_count++;
		print_header = output_mode.syntax = false;
		break;

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
	case GO_NO_PARAM:	print_param = false; break;
	case GO_NO_ECHO:	opt_no_echo = true; break;
	case GO_NO_CHECK:	opt_no_check = true; break;
	case GO_SECTIONS:	print_sections++; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wctct,TRACE_FILE,11);
 #endif
    NormalizeOptions( verbose > 3 && !is_env );
    SetupBMG(filter_bmg);
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
	enumError err = VerifySpecificOptions(&InfoUI_wctct,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wctct);

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

    extern const data_tab_t ctcode_data_tab[];

    enumError err = ERR_OK;
    switch ((enumCommands)cmd_ct->id)
    {
	case CMD_VERSION:	version_exit();
	case CMD_HELP:		PrintHelpColor(&InfoUI_wctct); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_EXPAND:	err = cmd_expand(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_EXPORT:	err = cmd_export(); break;
	case CMD_RAWDUMP:	err = cmd_rawdump(ctcode_data_tab); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupVarsCTCODE()); break;
	case CMD_FUNCTIONS:	SetupVarsCTCODE(); err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupVarsCTCODE()); break;
	case CMD_FLOAT:		err = cmd_float(); break;

	case CMD_DUMP:		err = cmd_dump(); break;
	case CMD_CAT:
	case CMD_BMG:
	case CMD_TRACKS:	err = cmd_cat(cmd_ct->id); break;

	case CMD_DECODE:	err = cmd_decode(); break;
	case CMD_CREATE:	err = cmd_create(); break;
	case CMD_PATCH:		err = cmd_patch(); break;

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
    int main_wctct ( int argc, char ** argv )
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

    tool_name = "wctct";
    print_title_func = print_title;
    SetupLib(argc,argv,WCTCT_SHORT,VERSION,TITLE);
    ctcode_enabled = true;

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WCTCT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions2("WCTCT_OPT",CheckOptions);
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

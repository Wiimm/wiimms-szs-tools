
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
 *   Copyright (c) 2011-2021 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include "lib-mdl.h"
#include "lib-szs.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wmdlt.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WMDLT_SHORT ": " WMDLT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM2 " - " AUTHOR " - " DATE

//
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wmdlt,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wmdlt,stdout,0,0,"HELP",0,URI_HOME);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WMDLT_SHORT,WMDLT_LONG,long_count-1);
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

    printf("  mdl modes:   %16x = \"%s\"\n",MDL_MODE,GetMdlMode());
    printf("  patch files: %16x = \"%s\"\n",PATCH_FILE_MODE,GetFileClassInfo());
    DumpTransformationOpt();

    if (opt_tracks)
	DumpTrackList(0,0,0);
    if (opt_arenas)
	DumpArenaList(0,0,0);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test()
{
 #if 1 || !defined(TEST) // test options

    return cmd_test_options();

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
    SetupVarsMDL();
    return ExportHelper("mdl");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command cat			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError iter_cat
(
    mdl_t		* mdl,		// MDL data structure
    void		* param		// a user defined parameter
)
{
    DASSERT(mdl);

    if ( verbose >= 0 || testmode )
    {
	fprintf(stdlog,"%sCAT %s:%s\n",
		    verbose > 0 ? "\n" : "",
		    GetNameFF(mdl->fform,0), mdl->fname );
	fflush(stdlog);
    }

    if (!testmode)
    {
	const enumError err = SaveTextMDL(mdl,"-",false);
	if ( err > ERR_WARNING )
	    return err;
    }
    fflush(stdout);

    return ERR_OK;
}

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
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;
 #if 1
	IterateRawDataMDL(&raw,global_check_mode,iter_cat,0);
 #else
	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%sCAT %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname );
	    fflush(stdlog);
	}

	mdl_t mdl;
	err = ScanRawDataMDL(&mdl,true,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = SaveTextMDL(&mdl,"-",false);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetMDL(&mdl);
 #endif
    }

    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		  command encode/decode			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_convert ( int cmd_id, ccp cmd_name, ccp def_path )
{
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
	const file_format_t dest_ff = cmd_id == CMD_ENCODE ? FF_MDL : FF_MDL_TXT;

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

	mdl_t mdl;
	err = ScanRawDataMDL(&mdl,true,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = dest_ff == FF_MDL
			? SaveRawMDL(&mdl,dest,opt_preserve)
			: SaveTextMDL(&mdl,dest,opt_preserve);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetMDL(&mdl);
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command strings			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError iter_strings
(
    mdl_t		* mdl,		// MDL data structure
    void		* param		// a user defined parameter
)
{
    DASSERT(mdl);
    putchar('\n');
    PrintStringsMDL(stdout,0,mdl);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_strings()
{
    stdlog = stderr;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	IterateRawDataMDL(&raw,global_check_mode,iter_strings,0);
    }

    putchar('\n');
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command geometry		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError iter_geometry
(
    mdl_t		* mdl,		// MDL data structure
    void		* param		// a user defined parameter
)
{
    DASSERT(mdl);

    if ( verbose >= 0 || testmode )
    {
	printf("%sGeometry of %s:%s\n",
		    verbose > 0 ? "\n" : "",
		    GetNameFF(mdl->fform,0), mdl->fname );
	fflush(stdout);
    }

    // ???

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_geometry()
{
    stdlog = stderr;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	IterateRawDataMDL(&raw,global_check_mode,iter_geometry,0);
    }

    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command _TEST			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError iter_xtest
(
    mdl_t		* mdl,		// MDL data structure
    void		* param		// a user defined parameter
)
{
    DASSERT(mdl);

 #if 1 // dump MDL section order -------------------------------------------

    // old_mario_gc_*.szs   9,0,1,6,7,8,     2,3,4,5
    // all others:         11,0,1,    8,9,10,2,3,4,5
    static char order[] = {11,0,1,6,7,8,9,10,2,3,4,5,-1};
    static char ref_v8[20], ref_v11[20] = {-1};
    if ( ref_v11[0] == -1 )
    {
	memset(ref_v11,0,sizeof(ref_v11));
	uint i;
	for ( i = 0; order[i] >= 0; i++ )
	    ref_v11[(int)order[i]] = i+2;
	HexDump16(0,0,0,ref_v11,sizeof(ref_v11));

	memcpy(ref_v8,ref_v11,sizeof(ref_v8));
	ref_v8[9] = 1;
    }
    ccp ref = mdl->version == 8 ? ref_v8 : ref_v11;

    printf("#ORDER: ");
    char sep = ' ';
    int sect = -1, last_ref = -1, fail = 0;

    SortMIL(&mdl->elem,true);

    const MemItem_t *mi = GetMemListElem(&mdl->elem,0,0);
    const MemItem_t *mi_end = mi + mdl->elem.used;
    for ( ; mi < mi_end; mi++ )
    {
	if ( sect != mi->idx1 )
	{
	    sect = mi->idx1;
	    printf("%c%d",sep,sect);
	    sep = ',';

	    if ( ref[sect] <= last_ref )
		fail++;
	    last_ref = ref[sect];
	}
    }
    printf(" %s: %s\n", fail ? "#FAIL! " : "", mdl->fname);

 #endif //------------------------------------------------------------------
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_xtest()
{
    stdlog = stderr;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	IterateRawDataMDL(&raw,global_check_mode,iter_xtest,0);
    }

    putchar('\n');
    ResetRawData(&raw);
    return cmd_err;
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

      RegisterOptionByName(&InfoUI_wmdlt,opt_stat,1,is_env);

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
	case GO_MDL:		err += ScanOptMdl(optarg); break;
	case GO_SCALE:		err += ScanOptScale(optarg); break;
	case GO_SHIFT:		err += ScanOptShift(optarg); break;
	case GO_XSS:		err += ScanOptXSS(0,optarg); break;
	case GO_YSS:		err += ScanOptXSS(1,optarg); break;
	case GO_ZSS:		err += ScanOptXSS(2,optarg); break;
	case GO_ROT:		err += ScanOptRotate(optarg); break;
	case GO_XROT:		err += ScanOptXRotate(0,optarg); break;
	case GO_YROT:		err += ScanOptXRotate(1,optarg); break;
	case GO_ZROT:		err += ScanOptXRotate(2,optarg); break;
	case GO_TRANSLATE:	err += ScanOptTranslate(optarg); break;
	case GO_NULL:		force_transform |= 1; break;
	case GO_NEXT:		err += NextTransformation(false); break;
	case GO_ASCALE:		err += ScanOptAScale(optarg); break;
	case GO_AROT:		err += ScanOptARotate(optarg); break;
	case GO_TFORM_SCRIPT:	err += ScanOptTformScript(optarg); break;

	case GO_UTF_8:		use_utf8 = true; break;
	case GO_NO_UTF_8:	use_utf8 = false; break;

	case GO_TEST:		testmode++; break;
	case GO_FORCE:		force_count++; break;
	case GO_REPAIR_MAGICS:	err += ScanOptRepairMagic(optarg); break;
	case GO_TINY:		err += ScanOptTiny(optarg); break;

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

	case GO_MAX_FILE_SIZE:	err += ScanOptMaxFileSize(optarg); break;
	case GO_TRACKS:		err += ScanOptTracks(optarg); break;
	case GO_ARENAS:		err += ScanOptArenas(optarg); break;

	case GO_ROUND:		opt_round = true; break;
	case GO_LONG:		long_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_NO_PARAM:	print_param = false; break;
	case GO_NO_ECHO:	opt_no_echo = true; break;
	case GO_NO_CHECK:	opt_no_check = true; break;
	case GO_SECTIONS:	print_sections++; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wmdlt,TRACE_FILE,11);
 #endif
    CloseTransformation();
    NormalizeOptions( verbose > 3 && !is_env ? 2 : 0 );
    SetupMDL();

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
	enumError err = VerifySpecificOptions(&InfoUI_wmdlt,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wmdlt);

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
	case CMD_HELP:		PrintHelp(&InfoUI_wmdlt,stdout,0,"HELP",0,URI_HOME,
					first_param ? first_param->arg : 0 ); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_EXPORT:	err = cmd_export(); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupVarsMDL()); break;
	case CMD_FUNCTIONS:	SetupVarsMDL(); err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupVarsMDL()); break;
	case CMD_MATRIX:	err = cmd_matrix(); break;
	case CMD_FLOAT:		err = cmd_float(); break;

	case CMD_CAT:		err = cmd_cat(); break;
	case CMD_DECODE:	err = cmd_convert(cmd_ct->id,"DECODE","\1P/\1N.txt"); break;
	case CMD_ENCODE:	err = cmd_convert(cmd_ct->id,"ENCODE","\1P/\1N\1?T"); break;
	case CMD_STRINGS:	err = cmd_strings(); break;
	case CMD_GEOMETRY:	err = cmd_geometry(); break;
	case CMD_XTEST:		err = cmd_xtest(); break;

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
    int main_wmdlt ( int argc, char ** argv )
#else
    int main ( int argc, char ** argv )
#endif
{
    print_title_func = print_title;
    SetupLib(argc,argv,WMDLT_SHORT,VERSION,TITLE);

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WMDLT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions("WMDLT_OPT",CheckOptions);
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

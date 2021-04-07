
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

#include <sys/stat.h>
#include <unistd.h>

#include "lib-image.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wimgt.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		functions not neded by wimgt		///////////////
///////////////////////////////////////////////////////////////////////////////

#if !SZS_WRAPPER
 bool DefineIntVar ( VarMap_t * vm, ccp varname, int value ) { return false; }
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WIMGT_SHORT ": " WIMGT_LONG " v" VERSION " r" REVISION \
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
	    PrintHelpCmd(&InfoUI_wimgt,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wimgt,stdout,0,0,"HELP",0,URI_HOME);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WIMGT_SHORT,WIMGT_LONG,long_count-1);
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

///////////////////////////////////////////////////////////////////////////////

#define SYNTAX_ERROR syntax_error(__FUNCTION__,__FILE__,__LINE__)

static void syntax_error ( ccp func, ccp file, uint line )
{
    if ( current_command
	&& current_command->id >= 0
	&& current_command->id < InfoUI_wimgt.n_cmd )
    {
	const InfoCommand_t *ic = InfoUI_wimgt.cmd_info + current_command->id;
	if (strchr(ic->syntax,'\n'))
	{
	    ccp src = ic->syntax;
	    char *dest = iobuf;
	    while (*src)
		if ( (*dest++ = *src++) == '\n' )
		{
		    *dest++ = ' ';
		    *dest++ = ' ';
		    *dest++ = ' ';
		}
	    *dest = 0;
	    PrintErrorFile(func,file,line,0,0,ERR_SYNTAX,"Syntax:\n   %s\n",iobuf);
	}
	else
	    PrintErrorFile(func,file,line,0,0,ERR_SYNTAX,"Syntax: %s\n",ic->syntax);
    }
    else
	PrintErrorFile(func,file,line,0,0,ERR_SYNTAX,"Syntax Error!\n");
    exit(ERR_SYNTAX);
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
    printf("  cmpr-default:         valid=%d, RGB565: %04x %04x, RGB: %06x %06x\n",
		opt_cmpr_valid,
		be16(opt_cmpr_def), be16(opt_cmpr_def+2),
		RGB565_to_RGB(be16(opt_cmpr_def)),
		RGB565_to_RGB(be16(opt_cmpr_def+2)) );
    printf("  n-mipmaps:   %16x = %12d\n",opt_n_images-1,opt_n_images-1);
    printf("  max-mipmaps: %16x = %12d\n",opt_max_images-1,opt_max_images-1);
    printf("  mipmap-size: %16x = %12d\n",opt_min_mipmap_size,opt_min_mipmap_size);
    printf("  mipmaps:     %16x = %12d\n",opt_mipmaps,opt_mipmaps);
    printf("  fast-mipmaps:%16x = %12d\n",fast_resize_enabled,fast_resize_enabled);

#if defined(TEST) || defined(DEBUG) || HAVE_WIIMM_EXT
    MipmapOptions_t mmo;
    SetupMipmapOptions(&mmo);
    PrintMipmapOptions(stdout,2,&mmo);
#endif

    if (patch_image_list.used)
    {
	printf("  image patch list:\n");
	int i;
	for ( i = 0; i < patch_image_list.used; i++ )
	{
	    const FormatFieldItem_t *ffi = patch_image_list.list + i;
	    printf("\t%04x=%s\n",ffi->patch_mode,ffi->key);
	}
    }

    DumpTransformList(stdout,1,false);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test()
{
 #if 1 || !defined(TEST) // test options

    return cmd_test_options();

 #elif 1

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	Image_t img;
	if (!LoadPNG(&img,true,true,param->arg,0))
	{
	    char path[PATH_MAX];
	    char *end = StringCopyS(path,sizeof(path),param->arg);
	    if ( end > path + 4 && !strcasecmp(end-4,".png") )
		end -= 4;
	    *end = 0;

	    StringCopyE(end,path+sizeof(path),"-std.png");
	    SavePNG(&img,true,path,0,0,false,0);

	    ConvertIMG(&img,false,0,IMG_X_RGB,PAL_INVALID);
	    StringCopyE(end,path+sizeof(path),"-rgb.png");
	    SavePNG(&img,true,path,0,0,false,0);

	    ConvertIMG(&img,false,0,IMG_X_GRAY,PAL_INVALID);
	    StringCopyE(end,path+sizeof(path),"-gray.png");
	    SavePNG(&img,true,path,0,0,false,0);

	    ConvertIMG(&img,false,0,IMG_X_PAL,PAL_INVALID);
	    StringCopyE(end,path+sizeof(path),"-pal.png");
	    SavePNG(&img,true,path,0,0,false,0);
	}
	ResetIMG(&img);
    }
    return ERR_OK;

 #else
    return ERR_OK;
 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command list			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_list ( int long_level )
{
    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI_wimgt,OPT_LONG,long_level,false);
	long_count += long_level;
    }

    if (print_header)
	PrintImageHead(0,long_count);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	char buf[CHECK_FILE_SIZE];
	enumError err = LoadFILE(param->arg,0,0,buf,sizeof(buf),1,0,false);
	if ( err > ERR_WARNING )
	{
	    if (opt_ignore)
		continue;
	    memset(buf,0,sizeof(buf));
	}
	PrintImage(buf,CHECK_FILE_SIZE,param->arg,0,0,long_count,false);
    }

    if (print_header)
	putchar('\n');

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command decode			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_decode()
{
    ccp def_path = "\1P/\1F.png";
    CheckOptDest(def_path,false);

    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	Image_t img;
	enumError err = LoadIMG(&img, true, param->arg,
				0, opt_mipmaps >= 0, true, opt_ignore>0 );
	if ( err > ERR_WARNING )
	    return err;
	if (err)
	    continue;

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,0,false);

	if ( verbose >= 0 || testmode )
	    fprintf(stdlog,"%s%sDECODE %s:%s -> PNG:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			PrintFormat3(img.info_fform,img.info_iform,img.info_pform),
			img.path, dest );

	TransformIMG(&img,2);
	if (opt_pre_convert)
	{
	    enumError err = ExecTransformIMG(&img);
	    if ( max_err < err )
	         max_err = err;
	}

	if (!testmode)
	{
	    err = SavePNG(&img,true,dest,0,0,false,0);
	    if ( max_err < err )
	         max_err = err;
	}
	ResetIMG(&img);
    }
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    command encode/convert		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_convert ( int cmd_id, ccp cmd_name, ccp def_path )
{
    CheckOptDest(def_path,false);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	Image_t img;
	enumError err = LoadIMG( &img, true, param->arg, 0,
					opt_mipmaps >= 0, false, opt_ignore );
	if ( err == ERR_NOT_EXISTS || err == ERR_WARNING )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	const file_format_t	src_f = img.info_fform;
	const image_format_t	src_i = img.info_iform;
	const palette_format_t	src_p = img.info_pform;
	TransformIMG(&img,-1);

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,0,false);
	const file_format_t fform
		= GetImageFF( img.tform_valid ? img.tform_fform : FF_INVALID,
				FF_INVALID,
				dest,
				img.info_fform,
				cmd_id == CMD_CONVERT,
				FF_TEX );

	if ( fform == FF_PNG )
	    Transform2XIMG(&img);
	else
	    Transform2InternIMG(&img);

	const bool samefile = !strcmp(param->arg,dest);
	if ( !all_count
		&& !img.conv_count
		&& !img.tform_exec
		&& !img.patch_done
		&& fform == img.info_fform
		&& samefile )
	{
	    continue;
	}

	if ( verbose >= 0 || testmode )
	{
	    image_format_t   iform = img.tform_valid && img.tform_iform != IMG_INVALID
					? img.tform_iform : img.iform;
	    palette_format_t pform = img.tform_valid && img.tform_pform != PAL_INVALID
					? img.tform_pform : img.pform;

	    fprintf(stdlog,"%s%s%s %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "", cmd_name,
			PrintFormat3(src_f,src_i,src_p),
			param->arg,
			PrintFormat3(fform,iform,pform),
			dest );
	}

	if (opt_pre_convert)
	    TransformIMG(&img,2);

	if (!testmode)
	{
	    err = SaveIMG(&img,fform,0,dest,samefile);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetIMG(&img);
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command copy			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_copy()
{
    RegisterOptionByIndex(&InfoUI_wimgt,OPT_OVERWRITE,1,false);
    opt_overwrite = true;

    if (!opt_dest)
    {
	if (!first_param)
	    return ERROR0(ERR_MISSING_PARAM, "Missing destination parameter!\n" );

	ParamList_t *param;
	for ( param = first_param; param->next; param = param->next )
	    ;
	ASSERT(param);
	ASSERT(!param->next);
	SetDest(param->arg,false);
	param->arg = 0;
    }

    file_format_t fform_dest = FF_UNKNOWN;
    ccp point = strrchr(opt_dest,'.');
    if (point)
	fform_dest = IsImageFF(GetByNameFF(point+1),true);
    PRINT("fform(dest) = %s\n",GetNameFF(0,fform_dest));

    int done_count = 0;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	if (!param->arg)
	    continue;
	NORMALIZE_FILENAME_PARAM(param);
	done_count++;

	Image_t img;
	enumError err = LoadIMG( &img, true, param->arg, 0,
					opt_mipmaps >= 0, false, opt_ignore );
	if ( err == ERR_NOT_EXISTS || err == ERR_WARNING )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	const file_format_t	src_f = img.info_fform;
	const image_format_t	src_i = img.info_iform;
	const palette_format_t	src_p = img.info_pform;
	TransformIMG(&img,-1);

	file_format_t fform = img.tform_valid && img.tform_fform != FF_INVALID
				? img.tform_fform
				: fform_dest != FF_UNKNOWN
					? fform_dest
					: src_f;
	PRINT("FFORM = %s\n",GetNameFF(0,fform));

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,0,0,false);

	if ( fform == FF_PNG )
	    Transform2XIMG(&img);
	else
	    Transform2InternIMG(&img);

	const bool samefile = !strcmp(param->arg,dest);
	if ( !all_count
		&& !img.conv_count
		&& !img.tform_exec
		&& !img.patch_done
		&& fform == img.info_fform
		&& samefile )
	{
	    continue;
	}

	if ( verbose >= 0 || testmode )
	{
	    image_format_t   iform = img.tform_valid && img.tform_iform != IMG_INVALID
					? img.tform_iform : img.iform;
	    palette_format_t pform = img.tform_valid && img.tform_pform != PAL_INVALID
					? img.tform_pform : img.pform;

	    fprintf(stdlog,"%s%sCOPY %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			PrintFormat3(src_f,src_i,src_p),
			param->arg,
			PrintFormat3(fform,iform,pform),
			dest );
	}

	if (opt_pre_convert)
	    TransformIMG(&img,2);

	if (!testmode)
	{
	    err = SaveIMG(&img,fform,0,dest,samefile);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetIMG(&img);
    }

    if (!done_count)
	SYNTAX_ERROR;

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

      RegisterOptionByName(&InfoUI_wimgt,opt_stat,1,is_env);

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
	case GO_CONST:		break; // [[2do]] err += ScanOptConst(optarg); break;

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

	case GO_MAX_FILE_SIZE:	err += ScanOptMaxFileSize(optarg); break;
	case GO_TRACKS:		err += ScanOptTracks(optarg); break;
	case GO_ARENAS:		err += ScanOptArenas(optarg); break;

	case GO_LONG:		long_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_SECTIONS:	print_sections++; break;

	case GO_ALL:		all_count++; break;
	case GO_CMPR_DEFAULT:	err += ScanOptCmprDefault(optarg); break;
	case GO_N_MIPMAPS:	err += ScanOptNMipmaps(optarg); break;
	case GO_MAX_MIPMAPS:	err += ScanOptMaxMipmaps(optarg); break;
	case GO_MIPMAP_SIZE:	err += ScanOptMipmapSize(optarg); break;
	case GO_MIPMAPS:	opt_mipmaps = +1; break;
	case GO_NO_MIPMAPS:	opt_mipmaps = -1; break;
	case GO_FAST_MIPMAPS:	fast_resize_enabled = true; break;
	case GO_PRE_CONVERT:	opt_pre_convert = true; break;
	case GO_PATCH:		err += ScanOptPatchImage(optarg); break;
	case GO_TRANSFORM:	err += ScanOptTransform(optarg); break;
	case GO_STRIP:		opt_strip = true; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wimgt,TRACE_FILE,11);
 #endif
    NormalizeOptions( verbose > 3 && !is_env ? 2 : 0 );

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
	enumError err = VerifySpecificOptions(&InfoUI_wimgt,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wimgt);

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
	case CMD_HELP:		PrintHelp(&InfoUI_wimgt,stdout,0,"HELP",0,URI_HOME,
					first_param ? first_param->arg : 0 ); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;

	case CMD_LIST:		err = cmd_list(0); break;
	case CMD_LIST_L:	err = cmd_list(1); break;
	case CMD_LIST_LL:	err = cmd_list(2); break;

	case CMD_DECODE:	err = cmd_decode(); break;
	case CMD_ENCODE:	err = cmd_convert(cmd_ct->id,"ENCODE","\1P/\1N"); break;
	case CMD_CONVERT:	err = cmd_convert(cmd_ct->id,"CONVERT",0); break;
	case CMD_COPY:		err = cmd_copy(); break;

	// no default case defined
	//	=> compiler checks the existence of all enum values

	case CMD__NONE:
	case CMD__N:
	    help_exit(false);
    }
    ResetPatchListIMG();
    return PrintErrorStat(err,verbose,cmd_ct->name1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   main()			///////////////
///////////////////////////////////////////////////////////////////////////////

#if SZS_WRAPPER
    int main_wimgt ( int argc, char ** argv )
#else
    int main ( int argc, char ** argv )
#endif
{
    print_title_func = print_title;
    SetupLib(argc,argv,WIMGT_SHORT,VERSION,TITLE);

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WIMGT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions("WIMGT_OPT",CheckOptions);
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

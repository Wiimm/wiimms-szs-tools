
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

#include <sys/stat.h>
#include <unistd.h>

#include "lib-xbmg.h"
#include "lib-szs.h"
#include "lib-mkw.h"
#include "dclib-utf8.h"
#include "dclib-regex.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wbmgt.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		functions not neded by wbmgt		///////////////
///////////////////////////////////////////////////////////////////////////////

#if !SZS_WRAPPER
 bool DefineIntVar ( VarMap_t * vm, ccp varname, int value ) { return false; }
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WBMGT_SHORT ": " WBMGT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM2 " - " AUTHOR " - " DATE

//
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    SetupPager();
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wbmgt,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wbmgt,stdout,0,0,"HELP",0,URI_HOME);

    ClosePager();
    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WBMGT_SHORT,WBMGT_LONG,long_count-1);
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

    printf("  old/new:     %16x = %12d\n",opt_new,opt_new);
    printf("  test:        %16x = %12d\n",testmode,testmode);
    printf("  verbose:     %16x = %12d\n",verbose,verbose);
    printf("  width:       %16x = %12d\n",opt_width,opt_width);
    printf("  escape-char: %16x = %12d\n",escape_char,escape_char);

    printf("  inf-size:    %16x = %12d\n",opt_bmg_inf_size,opt_bmg_inf_size);
    printf("  bmg-mid:     %16d = %12s\n",opt_bmg_mid,GetKeywordOffAutoOn(opt_bmg_mid));

    char attrib[BMG_ATTRIB_BUF_SIZE];
    if (opt_bmg_force_attrib)
    {
	PrintAttribBMG(attrib,sizeof(attrib),bmg_force_attrib,BMG_ATTRIB_SIZE,0,true);
	printf("  force-attrib: %s\n",attrib);
    }
    if (opt_bmg_def_attrib)
    {
	PrintAttribBMG(attrib,sizeof(attrib),bmg_def_attrib,BMG_ATTRIB_SIZE,0,true);
	printf("  def-attrib:   %s\n",attrib);
    }

    printf("  no-attrib:   %16x = %12d\n",opt_bmg_no_attrib,opt_bmg_no_attrib);
    printf("  x-escapes:   %16x = %12d\n",opt_bmg_x_escapes,opt_bmg_x_escapes);
    printf("  old-escapes: %16x = %12d\n",opt_bmg_old_escapes,opt_bmg_old_escapes);
    printf("  bmg-color:   %16x = %12d\n",opt_bmg_colors,opt_bmg_colors);
    printf("  bmg-inline:  %16x = %12d\n",opt_bmg_inline_attrib,opt_bmg_inline_attrib);

    ListPatchingListBMG(stdout,2,"patch-bmg list");

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
///////////////			command points			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_points()
{
    if (n_param)
    {
	ERROR0(ERR_WARNING,
		"%u parameter%s ignored.\n",
		n_param, n_param == 1 ? "" : "s" );
	n_param = 0;
    }

    const MkwPointInfo_t *mpi = GetMkwPointInfo(MkwPointsTab);
    printf("\nVersus points [%s]:\n",mpi->info);
    PrintMkwPoints( stdout, 2, MkwPointsTab, brief_count ? 2 : 1, 0, long_count );
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command regexp			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_regexp()
{
    if ( n_param < 2 )
	return ERROR0(ERR_SYNTAX,"At least 2 parameters needed: REGEX STRING...\n");

    Regex_t re;
    enumError err = ScanRegex(&re,true,first_param->arg);
    if ( err || !re.valid )
	return ERROR0(err,"Invalid regex: %s\n",first_param->arg);

    struct { FastBuf_t b; char space[500]; } res;
    InitializeFastBuf(&res,sizeof(res));

    ParamList_t *param;
    for ( param = first_param->next; param; param = param->next )
    {
	int stat = ReplaceRegex( &re, &res.b, param->arg, -1 );
	printf("\n%s\n -> [%d] %s\n",
		param->arg, stat, GetFastBufString(&res.b) );
    }

    putchar('\n');
    ResetFastBuf(&res.b);
    ResetRegex(&re);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command extract			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_extract()
{
    if ( n_param != 2 )
	return ERROR0(ERR_SYNTAX,"Excat 2 parameters expected:"
		" Section name and file\n");

    ccp sname = first_param->arg;
    const uint slen = strlen(sname);
    if ( slen < 1 || slen > 4 )
	return ERROR0(ERR_SYNTAX,
		"Section name must consist of 1 to 4 characters: %s",sname);

    ParamList_t *param = first_param->next;
    NORMALIZE_FILENAME_PARAM(param);

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,param->arg,0,opt_ignore>0,0);
    if (err)
	return err;

    if ( raw.fform != FF_BMG )
	return ERROR0(ERR_SYNTAX,"Not a binary BMG file: %s",raw.fname);


    //--- search sections

    const bmg_sect_list_t *sl = ScanSectionsBMG(raw.data,raw.data_size,0);
 #if 0
    {
	const bmg_sect_info_t *si;

	si = SearchSectionBMG(sl,sname,false,false);
	fprintf(stderr,"SEARCH              »%s« : %s\n",
		sname, si ? PrintID(si->sect->magic,4,0) : "-" );

	si = SearchSectionBMG(sl,sname,true,false);
	fprintf(stderr,"SEARCH abbrev       »%s« : %s\n",
		sname, si ? PrintID(si->sect->magic,4,0) : "-" );

	si = SearchSectionBMG(sl,sname,false,true);
	fprintf(stderr,"SEARCH        icase »%s« : %s\n",
		sname, si ? PrintID(si->sect->magic,4,0) : "-" );

	si = SearchSectionBMG(sl,sname,true,true);
	fprintf(stderr,"SEARCH abbrev icase »%s« : %s\n",
		sname, si ? PrintID(si->sect->magic,4,0) : "-" );
    }
 #endif

    err = ERR_NOT_EXISTS;
    const bmg_sect_info_t *si = SearchSectionBMG(sl,sname,true,true);

    if (si)
    {
	const u8 *data = raw.data + si->offset;
	uint size = si->real_size;

	if (!print_header)
	{
	    const uint head_size = si->head_size
					? si->head_size : sizeof(bmg_section_t);
	    if ( size >= head_size )
	    {
		data += head_size;
		size -= head_size;
	    }
	}

	PRINT("FOUND: %.4s, off: %u 0x%x, size: %u 0x%x [h=%d]\n",
		si->sect->magic, si->offset, si->offset,
		size, size, print_header );

	fwrite(data,1,size,stdout);
	err = ERR_OK;
    }

    FREE((void*)sl);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command sections		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_sections()
{
    raw_data_t raw;
    InitializeRawData(&raw);

    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if (err)
	    return err;

	if ( raw.fform != FF_BMG )
	{
	    ERROR0(ERR_SYNTAX,"Not a binary BMG file: %s",raw.fname);
	    if ( max_err < ERR_SYNTAX )
		 max_err = ERR_SYNTAX;
	    continue;
	}

	const bmg_sect_list_t *sl = ScanSectionsBMG(raw.data,raw.data_size,0);
	if (!sl)
	{
	    ERROR0(ERR_INVALID_DATA,"Can't scan binary BMG file: %s",raw.fname);
	    if ( max_err < ERR_INVALID_DATA )
		 max_err = ERR_INVALID_DATA;
	    continue;
	}

	uint max_info_len = 4;
	const bmg_sect_info_t *si;
	for ( si = sl->info; si->sect; si++ )
	{
	    if (si->info)
	    {
		const uint ilen = strlen(si->info);
		if ( max_info_len < ilen )
		     max_info_len = ilen;
	    }
	}

	const uint seplen = 3*(56+max_info_len);
	printf( "\n"
		"%sFile: %s\n"
		"%sSize: 0x%x = %u bytes%s, %u sections, encoding #%u (%s)\n"
		"%s%.*s\n"
		"%s  offset    size    sect  cut       head n(elem)  n(e)\n"
		"%s     hex     hex    size size magic size   *size  cut  info\n"
		"%s%.*s%s\n"
		,
		colout->heading, param->arg,
		colout->heading, sl->source_size, sl->source_size,
			sl->endian->is_be ? " BE" : sl->endian->is_le ? " LE" : "",
			sl->n_sections, sl->header->encoding,
			GetEncodingNameBMG(sl->header->encoding,"?"),
		colout->heading, seplen, ThinLine300_3,
		colout->heading,
		colout->heading,
		colout->heading, seplen, ThinLine300_3,
		colout->reset );

	for ( si = sl->info; si->sect; si++ )
	{
	    printf("%8x %7x %7u",
		si->offset, si->real_size, si->real_size );

	    if ( si->real_size < si->sect_size )
		printf(" %s%3u%s  ", colout->warn,
			si->sect_size - si->real_size, colout->reset );
	    else
		fputs("   -  ",stdout);

	    fputs(PrintID(si->sect->magic,4,0),stdout);

	    if ( si->head_size )
		printf(" %4u", si->head_size );
	    else
		printf("    %s?%s", colout->warn, colout->reset );

	    if (si->elem_size)
	    {
		printf(" %5u*%2u", si->n_elem_head, si->elem_size);
		if ( si->n_elem_head > si->n_elem )
		    printf(" %s%4u*%s",colout->warn,si->n_elem,colout->reset);
		else
		    fputs("    - ",stdout);
	    }
	    else
		fputs("      -      - ",stdout);

	    printf(" %s%s\n", si->info, si->is_empty ? ", empty" : "" );
	}

	printf("%s%.*s%s\n", colout->heading, seplen, ThinLine300_3, colout->reset );
	FREE((void*)sl);
    }

    ResetRawData(&raw);
    putchar('\n');
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command list			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_list()
{
    const uint max_width = GetTermWidth(80,40) - 9;
    enumError err = SetupPatchingListBMG();
    if ( err > ERR_WARNING )
	return err;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	bmg_t bmg;
	enumError err = LoadXBMG(&bmg,true,param->arg,true,opt_ignore>0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	err = UsePatchingListBMG(&bmg);
	if ( err > ERR_WARNING )
	    return err;

	printf("\nList [N=%u] %s:%s\n",
		bmg.item_used, GetNameFF_BMG(&bmg), bmg.fname );

	bmg_item_t * bi  = bmg.item;
	bmg_item_t * bi_end = bi + bmg.item_used;
	for ( ; bi < bi_end; bi++ )
	{
	    if ( memcmp(bmg.attrib,bi->attrib,bmg.attrib_used) )
	    {
		char buf[BMG_ATTRIB_BUF_SIZE];
		PrintAttribBMG(buf,sizeof(buf),bi->attrib,bmg.attrib_used,0,true);
		printf("%5x ~ %s\n",bi->mid,buf);
		if ( bi->text == bmg_null_entry )
		    continue;
	    }

	    if ( bi->text == bmg_null_entry )
		printf("%5x /\n",bi->mid);
	    else if ( bi->text )
	    {
		PrintString16BMG(iobuf,sizeof(iobuf),bi->text,bi->len,
				BMG_UTF8_MAX,0,opt_bmg_colors);
		printf("%5x = %.*s\n", bi->mid, max_width, iobuf );
	    }
	}
	ResetBMG(&bmg);
    }

    ResetPatchingListBMG();
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command slots			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_slots()
{
    enumError err = SetupPatchingListBMG();
    if ( err > ERR_WARNING )
	return err;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	bmg_t bmg;
	enumError err = LoadXBMG(&bmg,true,param->arg,true,opt_ignore>0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	err = UsePatchingListBMG(&bmg);
	if ( err > ERR_WARNING )
	    return err;

	bmg_create_t bc;
	InitializeCreateBMG(&bc,&bmg);
	bmg_item_t *bi;
	for ( bi = GetFirstBI(&bc); bi; bi = GetNextBI(&bc) )
	    ;

	const uint seplen = 3*60;
	printf( "\n"
		"%sFile: %s:%s\n"
		"%s%.*s\n"
		"%sslot/hex  mid/hex   delta  attributes\n"
		"%s%.*s%s\n"
		,
		colout->heading, GetNameFF_BMG(&bmg), param->arg,
		colout->heading, seplen, ThinLine300_3,
		colout->heading,
		colout->heading, seplen, ThinLine300_3,
		colout->reset );

	int slot;
	u32 *pmid = (u32*)bc.mid.buf;
	bmg_inf_item_t *inf = (bmg_inf_item_t*)bc.inf.buf;
	char buf[200];
	for ( slot = 0; slot < bc.n_msg; slot++ )
	{
	    const u32 mid = ntohl(*pmid++);
	    PrintAttribBMG(buf,sizeof(buf),inf->attrib,bmg.attrib_used,0,true);
	    printf("%7x %9x ", slot, mid );
	    if ( mid == 0xffff )
		fputs("      -",stdout);
	    else
		printf("%+7d",(int)mid-slot);
	    printf("  %s\n",buf);

	    inf = (bmg_inf_item_t*)( (u8*)inf + bmg.inf_size );
	}
	printf("%s%.*s%s\n", colout->heading, seplen, ThinLine300_3, colout->reset );

	ResetCreateBMG(&bc);
	ResetBMG(&bmg);
    }

    putchar('\n');
    ResetPatchingListBMG();
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command diff			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError diff_files ( ccp fname1, ccp fname2 )
{
    if ( verbose > 0 )
	printf("DIFF %s : %s\n", fname1, fname2 );

    //--- setup bmg structures

    bmg_t bmg1;
    enumError err = LoadXBMG(&bmg1,true,fname1,true,false);
    if ( err > ERR_WARNING )
	return err;
    err = UsePatchingListBMG(&bmg1);
    if ( err > ERR_WARNING )
	return err;

    bmg_t bmg2;
    err = LoadXBMG(&bmg2,true,fname2,true,false);
    if ( err > ERR_WARNING )
	return err;
    err = UsePatchingListBMG(&bmg2);
    if ( err > ERR_WARNING )
	return err;

    //--- diff

    err = DiffBMG( &bmg1, &bmg2, verbose < 0, GetTermWidth(80,40)-1 );

    if ( verbose >= -1 && err == ERR_DIFFER )
	printf("Content differ: %s : %s\n", bmg1.fname, bmg2.fname );
    else if ( verbose >= 0 && err == ERR_OK )
	printf("Content identical: %s : %s\n", bmg1.fname, bmg2.fname );

    //--- reset bmg structures

    ResetBMG(&bmg1);
    ResetBMG(&bmg2);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_diff_dest()
{
    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,"%F",0,false);
	const enumError err = diff_files(param->arg,dest);
	if ( max_err < err )
	     max_err = err;
	if ( err > ERR_WARNING || err && verbose < -1 )
	    break;
    }
    ResetPatchingListBMG();
    return max_err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_diff()
{
    if (opt_dest)
	return cmd_diff_dest();

    if ( n_param != 2 )
	return ERROR0(ERR_SYNTAX,"Exact 2 sources expected if --dest is not set.\n");

    enumError err = SetupPatchingListBMG();
    if ( err > ERR_WARNING )
	return err;

    ASSERT(first_param);
    ASSERT(first_param->next);
    err = diff_files(first_param->arg,first_param->next->arg);
    ResetPatchingListBMG();
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		  command patch/encode/decode		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_patch ( int cmd_id, ccp cmd_name, ccp def_path )
{
    CheckOptDest(def_path,false);
    enumError err = SetupPatchingListBMG();
    if ( err > ERR_WARNING )
	return err;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	bmg_t bmg;
	enumError err = LoadXBMG(&bmg,true,param->arg,cmd_id!=CMD_PATCH,opt_ignore>0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	const file_format_t dest_ff
		= cmd_id == CMD_ENCODE || cmd_id == CMD_PATCH && !bmg.is_text_src
			? FF_BMG
			: FF_BMG_TXT;

	SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,
			GetExtFF(dest_ff,0),false);

	if ( verbose >= 0 || testmode )
	    fprintf(stdlog,"%s%s%s %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "", cmd_name,
			GetNameFF_BMG(&bmg), bmg.fname,
			GetNameFF(dest_ff,0), dest );

	err = UsePatchingListBMG(&bmg);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = dest_ff == FF_BMG
			? SaveRawXBMG(&bmg,dest,opt_preserve)
			: SaveTextXBMG(&bmg,dest,opt_preserve);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetBMG(&bmg);
    }

    ResetPatchingListBMG();
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
    ccp filter_bmg = 0;

    for(;;)
    {
      const int opt_stat = getopt_long(argc,argv,OptionShort,OptionLong,0);
      if ( opt_stat == -1 )
	break;

      RegisterOptionByName(&InfoUI_wbmgt,opt_stat,1,is_env);

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
	case GO_ALIGN:		err += ScanOptAlign(optarg); break;

	case GO_MAX_FILE_SIZE:	err += ScanOptMaxFileSize(optarg); break;
	case GO_TRACKS:		err += ScanOptTracks(optarg); break;
	case GO_ARENAS:		err += ScanOptArenas(optarg); break;
	case GO_POINTS:		err += ScanOptMkwPoints(optarg,false); break;
	case GO_PATCH_BMG:	err += ScanOptPatchMessage(optarg); break;
	case GO_MACRO_BMG:	err += ScanOptMacroBMG(optarg); break;
	case GO_FILTER_BMG:	filter_bmg = optarg; break;

	case GO_LONG:		long_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;

	case GO_BMG_ENDIAN:	err += ScanOptBmgEndian(optarg); break;
	case GO_BMG_ENCODING:	err += ScanOptBmgEncoding(optarg); break;
	case GO_BMG_INF_SIZE:	err += ScanOptBmgInfSize(optarg,true); break;
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
	case GO_EXPORT:		export_count++; break;
	case GO_SECTIONS:	print_sections++; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wbmgt,TRACE_FILE,11);
 #endif
    NormalizeOptions( verbose > 3 && !is_env );
    SetupBMG(filter_bmg);

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
	enumError err = VerifySpecificOptions(&InfoUI_wbmgt,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wbmgt);

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
	case CMD_HELP:		PrintHelpColor(&InfoUI_wbmgt); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_EXPAND:	err = cmd_expand(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_POINTS:	err = cmd_points(); break;
	case CMD_REGEXP:	err = cmd_regexp(); break;

	case CMD_EXTRACT:	err = cmd_extract(); break;
	case CMD_SECTIONS:	err = cmd_sections(); break;
	case CMD_LIST:		err = cmd_list(); break;
	case CMD_SLOTS:		err = cmd_slots(); break;
	case CMD_DIFF:		err = cmd_diff(); break;

	case CMD_CAT:		err = cmd_bmg_cat(false); break;
	case CMD_MIX:		err = cmd_bmg_cat(true); break;
	case CMD_IDENTIFIER:	err = cmd_bmg_identifier(); break;
	case CMD_DECODE:	err = cmd_patch(cmd_ct->id,"DECODE","\1P/\1N.txt"); break;
	case CMD_ENCODE:	err = cmd_patch(cmd_ct->id,"ENCODE","\1P/\1N\1?T"); break;
	case CMD_PATCH:		err = cmd_patch(cmd_ct->id,"PATCH", "\1P/\1F"); break;

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
    int main_wbmgt ( int argc, char ** argv )
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

    tool_name = "wbmgt";
    print_title_func = print_title;
    SetupLib(argc,argv,WBMGT_SHORT,VERSION,TITLE);

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WBMGT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions2("WBMGT_OPT",CheckOptions);
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

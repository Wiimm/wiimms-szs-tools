
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

#include "lib-kcl.h"
#include "lib-szs.h"
#include "crypt.h"
#include "lib-checksum.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wkclt.c"
#include <math.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WKCLT_SHORT ": " WKCLT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM2 " - " AUTHOR " - " DATE

static const char autoname[] = "/course.kcl";

extern const char text_kcl_flag_template_cr[];

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
	    PrintHelpCmd(&InfoUI_wkclt,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wkclt,stdout,0,0,"HELP",0,URI_HOME);

    ClosePager();
    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WKCLT_SHORT,WKCLT_LONG,long_count-1);
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
	&& current_command->id < InfoUI_wkclt.n_cmd )
    {
	const InfoCommand_t *ic = InfoUI_wkclt.cmd_info + current_command->id;
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
    printf("  kcl modes:   %16llx = \"%s\"\n",(u64)KCL_MODE,GetKclMode());
    printf("  tri-area:    %16g\n",opt_tri_area);
    printf("  tri-height:  %16g\n",opt_tri_height);

    if (opt_xtridata)
	printf("  xtridata:    %16x = %12d [support %sabled]\n",
		opt_xtridata, opt_xtridata, SUPPORT_KCL_XTRIDATA ? "en" : "dis" );

    if ( kcl_script_list.used == 1 )
	printf("  kcl script:  %s\n",kcl_script_list.field[0]);
    else if ( kcl_script_list.used >  1 )
    {
	printf("  %u kcl scripts:\n",kcl_script_list.used);
	uint i;
	for ( i = 0; i < kcl_script_list.used; i++ )
	    printf("%6d.: %s\n",i+1,kcl_script_list.field[i]);
    }
    if (opt_slot)
	printf("  slot:        %16x = \"%s\"\n",opt_slot,PrintSlotMode(opt_slot));

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

 #else

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
    SetupVarsKCL();
    return ExportHelper("kcl");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command cat			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_cat()
{
    stdlog = stderr;
    enable_kcl_drop_auto++;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	CheckMode_t mode = GetCheckMode(false,false,true,false);
	if ( verbose >= 0 || testmode )
	{
	    mode = GetCheckMode(true,false,true,false);
	    fprintf(stdlog,"%sCAT %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname );
	    fflush(stdlog);
	}

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = SaveTextKCL(&kcl,"-",false);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		  command encode/decode			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_convert ( int cmd_id, ccp cmd_name, ccp def_path )
{
    if ( cmd_id == CMD_DECODE )
	enable_kcl_drop_auto++;
    CheckOptDest(def_path,false);

    raw_data_t raw;
    InitializeRawData(&raw);

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	const file_format_t dest_ff = cmd_id == CMD_ENCODE ? FF_KCL : FF_KCL_TXT;

	SubstDest(dest,sizeof(dest),arg,opt_dest,def_path,
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

	kcl_t kcl;
	InitializeKCL(&kcl);
	kcl.fform_outfile = dest_ff;
	err = ScanRawDataKCL(&kcl,false,&raw,true,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = dest_ff == FF_KCL
			? SaveRawKCL(&kcl,dest,opt_preserve)
			: SaveTextKCL(&kcl,dest,opt_preserve);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command copy			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_copy()
{
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

    enable_kcl_drop_auto = 0;
    file_format_t fform_dest = FF_UNKNOWN;
    ccp point = strrchr(opt_dest,'.');
    if (point)
    {
	if (!strcasecmp(point,".kcl"))
	    fform_dest = FF_KCL;
	else if ( !strcasecmp(point,".obj") || !strcasecmp(point,".txt") )
	{
	    fform_dest = FF_WAV_OBJ;
	    enable_kcl_drop_auto++;
	}
    }
    PRINT("fform(dest) = %s: %s\n",GetNameFF(0,fform_dest),opt_dest);

    raw_data_t raw;
    InitializeRawData(&raw);

    int done_count = 0;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	done_count++;

	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	const file_format_t dest_ff = fform_dest != FF_UNKNOWN
					? fform_dest
					: raw.fform;

	kcl_t kcl;
	InitializeKCL(&kcl);
	kcl.fform_outfile = dest_ff;
	err = ScanRawDataKCL(&kcl,false,&raw,true,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),arg,opt_dest,"\1P/\1N\1?T",
			GetExtFF(dest_ff,0),false);
	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%s%sCOPY %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(raw.fform,0), kcl.fname,
			GetNameFF(dest_ff,0), dest );
	    fflush(stdlog);
	}

	if (!testmode)
	{
	    err = dest_ff == FF_KCL
			? SaveRawKCL(&kcl,dest,opt_preserve)
			: SaveTextKCL(&kcl,dest,opt_preserve);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}

	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);

    if (!done_count)
	SYNTAX_ERROR;

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command cff			///////////////
///////////////////////////////////////////////////////////////////////////////

static ccp cff_func_f ( u16 flag )
{
    const uint bufsize = 20;
    char *buf = GetCircBuf(bufsize);
    DASSERT(buf);
    snprintf(buf,bufsize,"f(0x%02x,0x%03x)",
	flag % N_KCL_TYPE, flag / N_KCL_TYPE );
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static ccp cff_func_a ( u16 flag )
{
    const uint bufsize = 20;
    char *buf = GetCircBuf(bufsize);
    DASSERT(buf);
    snprintf(buf,bufsize,"a(0x%02x,%u,%u,%u,%u)",
	flag % N_KCL_TYPE,
	flag >> 13 & 7,
	flag >> 11 & 3,
	flag >>  8 & 7,
	flag >>  5 & 7 );
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static ccp cff_func_i ( u16 flag )
{
    const uint bufsize = 20;
    char *buf = GetCircBuf(bufsize);
    DASSERT(buf);
    snprintf(buf,bufsize,"i(0x%02x,%u,%u)",
	flag % N_KCL_TYPE,
	flag >> 5 & 7,
	flag >> 8 & 15 );
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_cff()
{
    CheckOptDest(KCL_FLAG_FILE1,false);

    raw_data_t raw;
    InitializeRawData(&raw);
    uint *flag_count = AllocFlagCount();
    char dest_buf[PATH_MAX];

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	LoadFlagFileKCL(&kcl);

	if ( kcl.fform == FF_KCL )
	{
	    CountFlagsKCL(&kcl,flag_count);

	    uint i;
	    for ( i = 0; i < N_KCL_FLAG; i++ )
		if (flag_count[i])
		{
		    StringCopyS(dest_buf,sizeof(dest_buf),GetGroupNameKCL(i,false));
		    char *p;
		    for ( p = dest_buf; *p; p++ )
			*p = toupper((int)*p);
		    GetFlagByNameKCL(&kcl,dest_buf,0);
		}
	}

	bool append = true;
	ccp dest = kcl.flag_fname;
	PRINT("DEST: %s\n",dest);
	if (!dest)
	{
	    SubstDest(dest_buf,sizeof(dest_buf),arg,opt_dest,
			0,KCL_FLAG_EXT,false);
	    dest = dest_buf;
	    append = false;
	}

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%s%s%s %s for %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			!kcl.flag_missing.used ? "SKIP  "
				: append ? "APPEND" : "CREATE",
			dest, GetNameFF(kcl.fform,0), kcl.fname );
	    fflush(stdlog);
	}

	if ( !testmode && kcl.flag_missing.used )
	{
	    File_t F;
	    enumError err;
	    if (append)
	    {
		err = AppendFILE(&F,true,dest,false,opt_mkdir);
		if (!err)
		    fputs("\r\n",F.f);
	    }
	    else
	    {
		err = CreateFileOpt(&F,true,dest,false,0);
		if (!err)
		    fputs(text_kcl_flag_template_cr,F.f);
	    }

	    if (!err)
	    {
		uint i, fw = 29;
		for ( i = 0; i < kcl.flag_missing.used; i++ )
		{
		    FormatFieldItem_t *it = kcl.flag_missing.list + i;
		    const uint len = strlen(it->key);
		    if ( fw < len )
			fw = len;
		}
		fw = ( fw + 10 ) / 8 * 8 + 5;

		for ( i = 0; i < kcl.flag_missing.used; i++ )
		{
		    FormatFieldItem_t *it = kcl.flag_missing.list + i;
		    int tablen = (fw-strlen(it->key))/8;
		    if ( tablen < 1 )
			tablen = 1;
		    const bool is_auto = IsAutoFlagKCL(it->num);
		    const u16 num = is_auto ? 0 : it->num; // unsigned with reduced num of bits
		    if ( long_count > 1 )
		    {
			if (brief_count)
			    fprintf(F.f,
				"  %s%.*s= %s",
				it->key, tablen, Tabs20,
				cff_func_a(num) );
			else
			    fprintf(F.f,
				"  %s%.*s= %s # %s  %s  0x%04x",
				it->key, tablen, Tabs20,
				cff_func_a(num), cff_func_f(num), cff_func_i(num), num );
		    }
		    else if (long_count)
		    {
			if (brief_count)
			    fprintf(F.f,
				"  %s%.*s= %s",
				it->key, tablen, Tabs20,
				cff_func_f(num) );
			else
			    fprintf(F.f,
				"  %s%.*s= %s # %s  %s  0x%04x",
				it->key, tablen, Tabs20,
				cff_func_f(num), cff_func_a(num), cff_func_i(num), num );
		    }
		    else if (brief_count)
			fprintf(F.f,
				"  %s%.*s= 0x%04x",
				it->key, tablen, Tabs20, num );
		    else
			fprintf(F.f,
				"  %s%.*s= 0x%04x  #  %s  %s  %s",
				it->key, tablen, Tabs20,
				num, cff_func_f(num), cff_func_a(num), cff_func_i(num) );

		    fputs(is_auto ? " # edit this line!\r\n" : "\r\n", F.f );
		}
		fputs("\r\n",F.f);
	    }

	    CloseFile(&F,false);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetKCL(&kcl);
    }

    FREE(flag_count);
    ResetStringField(&plist);
    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command types			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_types()
{
    stdlog = stderr;

    if (print_header)
    {
	if (brief_count)
	    printf("\n flag\n"
		     "-------\n");
	else
	    printf("\n  flag  = type | variant    : W X Y Z Ty\n"
		     "-----------------------------------------\n");
    }

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	char *arg = param->arg;
	u16 value = str2ul(arg,&arg,16);
	for(;;)
	{
	    while ( *arg > 0 && *arg <= ' ' || *arg == ',' )
		arg++;
	    if (!*arg)
		break;
	    char ch = tolower((int)*arg++);
	    if ( *arg == '=' )
		arg++;
	    u16 mask = 0, par = str2ul(arg,&arg,10);
	    int shift = 0;
	    switch (ch)
	    {
		case 't': mask = 0x001f; break;
		case 'v': mask = 0xffe0; shift =  5; break;
		case 'w': mask = 0xe000; shift = 13; break;
		case 'x': mask = 0x1800; shift = 11; break;
		case 'y': mask = 0x0700; shift =  8; break;
		case 'z': mask = 0x00e0; shift =  5; break;
	    }
	    if (!mask)
		break;
	    value = value & ~mask | (par<<shift) & mask;
	}
	
	if (brief_count)
	    printf("0x%04x\n",value);
	else
	{
	    const int w = value >> 13 & 7;
	    const int x = value >> 11 & 3;
	    const int y = value >>  8 & 7;
	    const int z = value >>  5 & 7;

	    printf(
		" 0x%04x = 0x%02x | 0x%03x << 5 : %c %c %c %c %2d\n",
		value, value & 0x1f, value >> 5,
		w ? '0'+w : '-',
		x ? '0'+x : '-',
		y ? '0'+y : '-',
		z ? '0'+z : '-',
		value & 0x1f );
	}
    }

    if (print_header)
	putchar('\n');

    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command flags			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_flags()
{
    stdlog = stderr;
    uint *count = AllocFlagCount();

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 || testmode )
	    fprintf(stdout,"%sKCL Flags of %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname );

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    const uint n = kcl.tridata.used;
	    CountFlagsKCL(&kcl,count);
	    uint tcount[N_KCL_TYPE];
	    memset(tcount,0,sizeof(tcount));

	    if ( print_header && !brief_count )
		printf("\n  flag  = type | variant    : W X Y Z Ty :  triangle count\n"
			 "------------------------------------------------------------\n");

	    uint i, j;
	    for ( i = 0; i < N_KCL_TYPE; i++ )
	    {
		int done = 0;
		for ( j = i; j < N_KCL_FLAG; j += N_KCL_TYPE )
		    if (count[j])
		    {
			tcount[i] += count[j];
			if (!brief_count)
			{
			    done++;
			    const int w = j >> 13 & 7;
			    const int x = j >> 11 & 3;
			    const int y = j >>  8 & 7;
			    const int z = j >>  5 & 7;

			    printf(
				" 0x%04x = 0x%02x | 0x%03x << 5"
				" : %c %c %c %c %2d"
				" :%7u = %6.2f%%\n",
				j, j & 0x1f, j >> 5,
				w ? '0'+w : '-',
				x ? '0'+x : '-',
				y ? '0'+y : '-',
				z ? '0'+z : '-',
				j & 0x1f,
				count[j], 100.0 * count[j] / n );
			}
		    }
		if ( done && print_header )
		    putchar('\n');
	    }

	    if (print_header)
		printf( "\n"
			"   type :  triangle count  : type description\n"
			"---------------------------------------------------\n");

	    for ( i = 0; i < N_KCL_TYPE; i++ )
		if (tcount[i])
		    printf(" T 0x%02x :%7u = %6.2f%% : %s\n",
			i, tcount[i], 100.0 * tcount[i] / n,
			kcl_type[i].info );
	    putchar('\n');
	}
	ResetKCL(&kcl);
	fflush(stdout);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    FREE(count);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command dump			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dump ( bool brief_level, int cmd )
{
    stdlog = stderr;

    if ( brief_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI_wkclt,OPT_BRIEF,brief_level,false);
	brief_count += brief_level;
    }

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 || testmode )
	    fprintf(stdout,"%s%s of %s:%s\r\n",
			verbose > 0 ? "\r\n" : "",
			cmd == CMD_LIST
				? "LIST"	: cmd == CMD_TRIANGLES
				? "TRIANGLES"	: "DUMP",
			GetNameFF(raw.fform,0), raw.fname );

	kcl_t kcl;
	InitializeKCL(&kcl);
	kcl.fform_outfile = FF_KCL;
	err = ScanRawDataKCL(&kcl,false,&raw,true,global_check_mode);
	if ( err > ERR_WARNING )
	    return err;

	if (!testmode)
	{
	    err = cmd == CMD_LIST
			? DumpTrianglesKCL(&kcl,"-")
			: cmd == CMD_TRIANGLES
			? ListTrianglesKCL(&kcl,"-",!brief_count,long_count)
			: DumpKCL(&kcl,"-",!brief_count,long_count);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command traverse		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_traverse ( bool fall )
{
    stdlog = stderr;

    uint width = 100;
    u32 type_mask = M1(type_mask);

    char fall_hint[50] = {0};
    if (fall)
    {
	const Var_t *var = FindConst("WIDTH");
	if ( var && var->mode != VAR_UNSET )
	{
	    int temp = GetIntV(var);
	    if ( temp > 0 )
		width = temp;
	}

	var = FindConst("MASK");
	if ( var && var->mode != VAR_UNSET )
	    type_mask = GetIntV(var);

	snprintf(fall_hint,sizeof(fall_hint),
			" [width=%u,mask=0x%x]", width, type_mask );
    }

    raw_data_t raw;
    InitializeRawData(&raw);

    ParamList_t *param = first_param;
	NORMALIZE_FILENAME_PARAM(param);
    enumError err = LoadRawData(&raw,false,first_param->arg,autoname,opt_ignore>0,FF_KCL);
    if ( err == ERR_NOT_EXISTS || err > ERR_WARNING )
	return err;

    if ( verbose >= 0 || testmode )
    {
	fprintf(stdlog,"\nANALYZE OCTREE OF %s:%s%s\n\n",
			GetNameFF(raw.fform,0), raw.fname, fall_hint );
	fflush(stdlog);
    }

    kcl_t kcl;
    err = ScanRawDataKCL(&kcl,true,&raw,true,0);
    if ( err > ERR_WARNING || !param )
	return err;

    uint np = 1;
    enumError max_err = ERR_OK;
    for ( param = param->next; param; param = param->next, np++ )
    {
	char name[50];
	snprintf(name,sizeof(name),"Point #%u",np);

	DEFINE_VAR(pt);
	err = ScanVectorExpr(param->arg,name,&pt);
	if ( max_err < err )
	     max_err = err;

	if (fall)
	{
	    int kcl_flag;
	    double height = FallKCL(&kcl,stdout,pt.d3,width,
						long_count,type_mask,&kcl_flag);
	    if (brief_count>1)
		printf("%11.3f\n",height);
	    else if (brief_count)
		printf("%11.3f %11.3f %11.3f\n",
		    pt.d3.x, height, pt.d3.z );
	    else if ( kcl_flag >= 0 )
		printf("%11.3f %11.3f %11.3f  0x%04x\n",
		    pt.d3.x, height, pt.d3.z, kcl_flag );
	    else
		printf("%11.3f %11.3f %11.3f      -1\n",
		    pt.d3.x, height, pt.d3.z );
	}
	else
	    err = TraverseOctreeKCL( &kcl, stdout, pt.d3,
					brief_count<=0, long_count );
	fflush(stdout);
	if ( max_err < err )
	     max_err = err;
    }

    ResetKCL(&kcl);
    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command check			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_check()
{
    patch_action_log_disabled++;
    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	const bool check_verbose = verbose > 0 || long_count > 0;
	CheckMode_t mode = GetCheckMode(false,brief_count>0,verbose<0,check_verbose);
	if ( verbose >= 0 || testmode )
	{
	    ColorSet_t col;
	    SetupColorSet(&col,stdlog);
	    mode = GetCheckMode(true,brief_count>0,false,check_verbose);
	    fprintf(stdlog,"%s%sCHECK %s:%s%s\n",
			verbose > 0 ? "\n" : "",
			col.heading, GetNameFF(raw.fform,0), raw.fname, col.reset );
	    fflush(stdlog);
	}

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,0);
	if ( err <= ERR_WARNING && ( !mode || CheckKCL(&kcl,mode) ))
	    cmd_err = ERR_DIFFER;
	fflush(stdout);
	ResetKCL(&kcl);
	if ( err > ERR_WARNING )
	    return err;
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command sha1			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_sha1()
{
    stdlog = stderr;
    enumError max_err = ERR_OK;
    SetPatchFileModeReadonly();
    SetupCoding64( opt_coding64, opt_db64 ? ENCODE_BASE64URL : ENCODE_BASE64 );

    raw_data_t raw;
    InitializeRawData(&raw);

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);

	if ( max_err < err )
	    max_err = err;

	if ( err > ERR_WARNING || err == ERR_NOT_EXISTS )
	{
	    if ( opt_db64 )
	    {
		putchar('-');
		if (long_count)
		    fputs("  -1 -1 0.000  -",stdout);
		if (brief_count)
		    putchar('\n');
		else
		    printf("  %s\n",arg);
	    }

	    if ( err == ERR_NOT_EXISTS || opt_ignore )
		continue;
	    return err;
	}

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,0);
	if ( err > ERR_WARNING )
	    return err;

	err = CreateRawKCL(&kcl,false);
	if ( err > ERR_WARNING )
	    return err;

	sha1_size_hash_t info;
	SHA1(kcl.raw_data,kcl.raw_data_size,info.hash);
	info.size = htonl(kcl.raw_data_size);
	char checksum[100];
	CreateSSChecksum(checksum,sizeof(checksum),&info);

	if (brief_count)
	    printf("%s\n",checksum);
	else
	    printf("%s  %s\n",checksum,kcl.fname);
	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command analyze			///////////////
///////////////////////////////////////////////////////////////////////////////

static void analyze_kcl ( kcl_t *kcl )
{
    DASSERT(kcl);

    double v_min	=  INFINITY;
    double v_max_neg	= -INFINITY;
    double v_min_pos	=  INFINITY;
    double v_max	= -INFINITY;

    float  n_min	=  INFINITY;
    float  n_max_neg	= -INFINITY;
    float  n_min_pos	=  INFINITY;
    float  n_max	= -INFINITY;

    uint n		= 0;
    const uint n_tri	= kcl->tridata.used;

    kcl_tridata_t *td_base = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t *td, *td_end = td_base + n_tri;
    for ( td = td_base; td < td_end; td++ )
    {
	if ( td->status & TD_INVALID_NORM )
	    continue;

	n++;

	double *d = td->pt->v;
	uint nd = 3;
	for ( ; nd > 0; nd--, d++ )
	{
	    if ( v_min > *d ) v_min = *d;
	    if ( v_max < *d ) v_max = *d;
	    if ( *d > 0.0 && v_min_pos > *d ) v_min_pos = *d;
	    if ( *d < 0.0 && v_max_neg < *d ) v_max_neg = *d;
	}

	float *f = td->normal->v;
	uint nf = sizeof(td->normal->v)/sizeof(*f);
	for ( ; nf > 0; nf--, f++ )
	{
	    if ( n_min > *f ) n_min = *f;
	    if ( n_max < *f ) n_max = *f;
	    if ( *f > 0.0 && n_min_pos > *f ) n_min_pos = *f;
	    if ( *f < 0.0 && n_max_neg < *f ) n_max_neg = *f;
	    //printf("XX: %08x\n",*(u32*)f);
	}
    }


    printf(
	"\n"
	"%5u / %-5u     vertex         normal\n"
	"--------------------------------------------\n"
	" min:          %13.6e  %13.6e\n"
	" max negative: %13.6e  %13.6e\n"
	" min positive: %13.6e  %13.6e\n"
	" max:          %13.6e  %13.6e\n"
	"\n",
	n, n_tri,
	v_min,		n_min,
	v_max_neg,	n_max_neg,
	v_min_pos,	n_min_pos,
	v_max,		n_max );

    // Value found in Nintendos tracks:
    // -v_max_neg == v_min_pos == 1.001358e-05 == 0x1.5p-17 == 21.0>>21
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_analyze()
{
    stdlog = stderr;
    patch_action_log_disabled++;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 || testmode )
	    fprintf(stdout,"%sAnalyze %s:%s\r\n",
			verbose > 0 ? "\r\n" : "",
			GetNameFF(raw.fform,0), raw.fname );

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,0);
	if ( err > ERR_WARNING )
	    return err;

	analyze_kcl(&kcl);
	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
    ResetRawData(&raw);
    return cmd_err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_blow()
{
    stdlog = stderr;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	enumError err = LoadRawData(&raw,false,arg,autoname,opt_ignore>0,FF_KCL);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	kcl_t kcl;
	err = ScanRawDataKCL(&kcl,true,&raw,true,0);
	if ( err > ERR_WARNING )
	    return err;

	uint blow = CalcBlowSizeKCL(&kcl);
	fprintf(stdout,"%6u = blow size of %s:%s\r\n",
			blow, GetNameFF(kcl.fform,0), kcl.fname );
	ResetKCL(&kcl);
    }

    ResetStringField(&plist);
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

      RegisterOptionByName(&InfoUI_wkclt,opt_stat,1,is_env);

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
	case GO_KCL:		err += ScanOptKcl(optarg); break;
	case GO_KCL_FLAG:	err += ScanOptKclFlag(optarg); break;
	case GO_KCL_SCRIPT:	err += ScanOptKclScript(optarg); break;
	case GO_TRI_AREA:	err += ScanOptTriArea(optarg); break;
	case GO_TRI_HEIGHT:	err += ScanOptTriHeight(optarg); break;
	case GO_FLAG_FILE:	opt_flag_file = optarg; break;
	case GO_XTRIDATA:	opt_xtridata = optarg ? str2ul(optarg,0,10) : 1; break;
	case GO_SLOT:		err += ScanOptSlot(optarg); break;

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

	case GO_ID:		opt_id = true; break;
	case GO_BASE64:		opt_base64 = true; err += ScanOptCoding64(optarg); break;
	case GO_DB64:		opt_db64 = true; err += ScanOptCoding64(optarg); break;
	case GO_CODING:		err += ScanOptCoding64(optarg); break;

	case GO_ROUND:		opt_round = true; break;
	case GO_LONG:		long_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_NO_WILDCARDS:	no_wildcards_count++; break;
	case GO_IN_ORDER:	inorder_count++; break;
	case GO_NO_PARAM:	print_param = false; break;
	case GO_NO_ECHO:	opt_no_echo = true; break;
	case GO_NO_CHECK:	opt_no_check = true; break;
	case GO_SECTIONS:	print_sections++; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wkclt,TRACE_FILE,11);
 #endif
    CloseTransformation();
    NormalizeOptions( verbose > 3 && !is_env );
    SetupKCL();

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
	enumError err = VerifySpecificOptions(&InfoUI_wkclt,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wkclt);

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
	case CMD_HELP:		PrintHelpColor(&InfoUI_wkclt); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_EXPAND:	err = cmd_expand(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_EXPORT:	err = cmd_export(); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupVarsKCL()); break;
	case CMD_FUNCTIONS:	SetupVarsKCL(); err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupVarsKCL()); break;
	case CMD_MATRIX:	err = cmd_matrix(); break;
	case CMD_FLOAT:		err = cmd_float(); break;

	case CMD_CAT:		err = cmd_cat(); break;
	case CMD_DECODE:	err = cmd_convert(cmd_ct->id,"DECODE","\1P/\1N.obj"); break;
	case CMD_ENCODE:	err = cmd_convert(cmd_ct->id,"ENCODE","\1P/\1N\1?T"); break;
	case CMD_COPY:		err = cmd_copy(); break;
	case CMD_CFF:		err = cmd_cff(); break;
	case CMD_TYPES:		err = cmd_types(); break;
	case CMD_FLAGS:		err = cmd_flags(); break;
	case CMD_DUMP:		err = cmd_dump(0,CMD_DUMP); break;
	case CMD_DBRIEF:	err = cmd_dump(1,CMD_DUMP); break;
	case CMD_LIST:		err = cmd_dump(0,CMD_LIST); break;
	case CMD_TRIANGLES:	err = cmd_dump(0,CMD_TRIANGLES); break;
	case CMD_TRAVERSE:	err = cmd_traverse(false); break;
	case CMD_FALL:		err = cmd_traverse(true); break;
	case CMD_CHECK:		err = cmd_check(); break;
	case CMD_SHA1:		err = cmd_sha1(); break;
	case CMD_ANALYZE:	err = cmd_analyze(); break;
	case CMD_BLOW:		err = cmd_blow(); break;

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
    int main_wkclt ( int argc, char ** argv )
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

    tool_name = "wkclt";
    print_title_func = print_title;
    SetupLib(argc,argv,WKCLT_SHORT,VERSION,TITLE);

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WKCLT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions2("WKCLT_OPT",CheckOptions);
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

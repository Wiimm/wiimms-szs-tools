
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

#include "dclib-xdump.h"
#include "lib-kmp.h"
#include "lib-kcl.h"
#include "lib-szs.h"
#include "db-object.h"
#include "lib-object.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wkmpt.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WKMPT_SHORT ": " WKMPT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM2 " - " AUTHOR " - " DATE

static const char autoname[] = "/course.kmp";

//
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wkmpt,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wkmpt,stdout,0,0,"HELP",0,URI_HOME);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WKMPT_SHORT,WKMPT_LONG,long_count-1);
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

    printf("  kmp draw:    %16llx = \"%s\"\n",(u64)KMP_DRAW,GetDrawMode());
    printf("  kmp modes:   %16llx = \"%s\"\n",(u64)KMP_MODE,GetKmpMode());
    printf("  kmp tform:   %16x = \"%s\"\n",KMP_TFORM,GetKmpTform());
    if (speed_mod_active)
	printf("  kmp speed modifier:      %04x ~ %5.3f\n",
		speed_mod_val, speed_mod_factor );
    if (opt_slot)
	printf("  slot:        %16x = \"%s\"\n",opt_slot,PrintSlotMode(opt_slot));

    printf("  battle mode: %16d = %s\n",
		opt_battle_mode, GetKeywordOffAutoOn(opt_battle_mode) );
    printf("  export flags:%16d = %s\n",
		opt_export_flags, GetKeywordOffAutoOn(opt_export_flags) );
    printf("  route-options: %14d = %s\n",
		opt_route_options, GetKeywordOffAutoOn(opt_route_options) );
    printf("  wim0:        %16d = %s\n",
		opt_wim0, GetKeywordOffAutoOn(opt_wim0) );

    if (opt_png)
    {
	printf("  png-pix-size:%16x = %12d\n",opt_png_pix_size,opt_png_pix_size);
	printf("  png-a-alias: %16x = %12d\n",opt_png_aalise,opt_png_aalise);
	printf("  png-x*       %16d .. %d\n",opt_png_x1,opt_png_x2);
	printf("  png-y*       %16d .. %d\n",opt_png_y1,opt_png_y2);
	printf("  png-type-mask:     %#010x\n",opt_png_type_mask);
	printf("  png-rm       %11sabled\n", opt_png_rm ? "en" : "dis" );
    }

    DumpTransformationOpt();

    if (const_map.used)
    {
	printf("  const list:\n");
	DumpVarMap(stdout,8,&const_map,false);
    }

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
///////////////		  command objects			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_objects()
{
    stdlog = stderr;
    search_filter_t *filter = AllocSearchFilter(N_KMP_GOBJ);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	ccp arg = param->arg;
	if (!*arg)
	    continue;

	char *end;
	uint num = strtoul(arg,&end,16);
	if ( num > 0 && num < N_KMP_GOBJ && !*end )
	    filter[num].mode = 1;

	num = strtoul(arg,&end,10);
	if ( num > 0 && num < N_KMP_GOBJ && !*end )
	    filter[num].mode = 1;

	char needle[200];
	CreateNeedleWCMP(needle,sizeof(needle),arg);

	uint id;
	const ObjectInfo_t *oi = ObjectInfo;
	for ( id = 0; id < N_KMP_GOBJ; id++, oi++ )
	{
	    if ( filter[id].mode || !oi->name )
		continue;

	    filter[id].mode = strcasestr(oi->name,arg)
			    || oi->fname && strcasestr(oi->fname,arg);
	    if (!filter[id].mode)
	    {
		if (!filter[id].string)
		{
		    ccp list[] = { oi->fname, oi->fname ? oi->fname : "", oi->info, 0 };
		    char haystack[5000];
		    uint len = CreateHaystackListWCMP(haystack,sizeof(haystack),list);
		    filter[id].string = MEMDUP(haystack,len+1);
		}
		filter[id].mode = FindWCMP(filter[id].string,needle,3,0,0);
	    }
	}
    }

    uint twidth = GetTermWidth(80,40)-1;
    const bool print_sep = !brief_count || brief_count < 2 && long_count;

    uint id, found_count = 0;
    const ObjectInfo_t *oi = ObjectInfo;
    for ( id = 0; id < N_KMP_GOBJ; id++, oi++ )
    {
	if ( !filter[id].mode || !oi->name )
	    continue;

	if (!found_count++)
	{
	    if (print_header)
	    {
		if ( brief_count > 1 )
		{
		    twidth = OBJ_FW_NAME + 7;
		    printf("\n  ID object name\n%.*s\n",twidth,Minus300);
		}
		else
		    printf("\n  ID %-*s Characteristics\n%.*s\n",
			OBJ_FW_NAME, "Object name", twidth, Minus300 );
	    }
	    else if (print_sep)
		printf("%.*s\n",twidth,Minus300);
	}

	if ( brief_count > 1 )
	    printf("%4x  %s\n",id,oi->name);
	else
	{
	    printf("%4x %-*s %s\n",id,OBJ_FW_NAME,oi->name,oi->charact);

	    if (!brief_count)
	    {
		if (oi->info)
		{
		    fputs("   Info:",stdout);
		    PutLines(stdout,10,twidth,8,0,oi->info,0);
		}

		if ( oi->fname && !long_count )
		    printf("   Files: %s\n",oi->fname);
	    }

	    if (long_count)
	    {
		UsedFileFILE_t used_file;
		if (FindDbFileByObject(&used_file,true,id,1))
		{
		    ccp head = "Files:";
		    int fi;
		    for ( fi = 0; fi < N_DB_FILE_FILE; fi++ )
			if (used_file.d[fi])
			{
			    printf("%9s %s\n",head,DbFileFILE[fi].file);
			    head = "";
			}
		}

		uint p;
		for ( p = 0; p < 8; p++ )
		    if ( oi->param[p] )
		    {
			printf("   Set#%u:",p+1);
			PutLines(stdout,10,twidth,9,0,oi->param[p],0);
		    }
	    }

	    if (print_sep)
		printf("%.*s\n",twidth,Minus300);
	}
    }

    if ( !print_sep && print_header )
	printf("%.*s\n\n",twidth,Minus300);
    else if (print_header)
	putchar('\n');

    FreeSearchFilter(filter);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command _export			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_export()
{
    SetupVarsKMP();
    return ExportHelper("kmp");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command _xexport		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_xexport()
{
    //--- cond_ref_tab[]

    fputs("\nstatic const gobj_cond_ref_t cond_ref_tab[] =\n{",stdout);

    u16 next = 0;
    const gobj_cond_ref_t *cr;
    for ( cr = cond_ref_tab; cr->ref_id; cr++ )
    {
	if ( next != cr->ref_id || cr->option & 1 )
	    putchar('\n');
	next = cr->ref_id + 1;

	if ( strlen(cr->name) >= sizeof(cr->name) )
		printf("ERR: STRLEN(NAME,%#x): %zd >= %zd : %s\n",
			cr->ref_id, strlen(cr->name), sizeof(cr->name), cr->name );
	if ( strlen(cr->alias) >= sizeof(cr->alias) )
		printf("ERR: STRLEN(ALIAS,%#x): %zd >= %zd : %s\n",
			cr->ref_id, strlen(cr->alias), sizeof(cr->alias), cr->alias );
	if ( strlen(cr->info) >= sizeof(cr->info) )
		printf("ERR: STRLEN(INFO,%#x): %zd >= %zd : %s\n",
			cr->ref_id, strlen(cr->info), sizeof(cr->info), cr->info );
	    
	if (!(cr->ref_id&1))
	{
	    const gobj_cond_ref_t *next = cr+1;
	    if ( next->ref_id != (cr->ref_id|1) )
		printf("ERR: REF-ID: %04x %04x\n",cr->ref_id,next->ref_id);

	    if (!(cr->option&4))
	    {
		if ( cr->cond_mask && ( cr->cond_mask ^ next->cond_mask ) != 0xffff )
		    printf("ERR: COND-MASK[%04x,%04x]: %04x %04x\n",
			    cr->ref_id, next->ref_id, cr->cond_mask, next->cond_mask );
		if ( cr->set_mask && ( cr->set_mask ^ next->set_mask ) != 0xff )
		    printf("ERR: SET-MASK[%04x,%04x]: %02x %02x\n",
			    cr->ref_id, next->ref_id, cr->set_mask, next->set_mask );
	    }
	}

	printf("    { 0x%04x, 0x%04x, 0x%02x, %u }, // %-13s : %s\n",
		cr->ref_id, cr->cond_mask, cr->set_mask, cr->op,
		cr->name, cr->info );
    }
    fputs("\n    {0,0,0,0}\n};\n\n",stdout);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command startpos		///////////////
///////////////////////////////////////////////////////////////////////////////

#define POSITION_ORIGIN 0x9f42410

static enumError cmd_startpos()
{
    stdlog = stderr;

    enum { M_RESET, M_TABLE, M_RELATIVE, M_MIRROR, M_NARROW,
		M_AUTO, M_HEX, M_Z, M_NUMBER };
    static const KeywordTab_t keytab[] =
    {
	{ M_RESET,	"CLEAR",	"RESET",	 0 },

	{ M_TABLE,	"IMAGE",	"I",		 0 },
	{ M_TABLE,	"TABLE",	"T",		 1 },

	{ M_RELATIVE,	"ABSOLUTE",	"ABS",		 0 },
	{ M_RELATIVE,	"RELATIVE",	"REL",		 1 },

	{ M_MIRROR,	"LEFT",		"L",		 0 },
	{ M_MIRROR,	"RIGHT",	"R",		 1 },

	{ M_NARROW,	"WIDE",		"W",		 0 },
	{ M_NARROW,	"NARROW",	"N",		 1 },

	{ M_AUTO,	"AUTO",		"A",		 0x06 },
	{ M_AUTO,	"NO-AUTO",	"NOAUTO",	 0x00 },

	{ M_HEX,	"DECIMAL",	"DEC",		 0 },
	{ M_HEX,	"HEXADECIMAL",	"HEX",		 1 },

	{ M_Z,		"Z0",		0,		 0 },
	{ M_Z,		"Z1",		0,		 1 },
	{ M_Z,		"Z2",		0,		 2 },
	{ M_Z,		"Z3",		0,		 3 },

	{ M_NUMBER,	"1",		0,		 1 },
	{ M_NUMBER,	"2",		0,		 2 },
	{ M_NUMBER,	"3",		0,		 3 },
	{ M_NUMBER,	"4",		0,		 4 },
	{ M_NUMBER,	"5",		0,		 5 },
	{ M_NUMBER,	"6",		0,		 6 },
	{ M_NUMBER,	"7",		0,		 7 },
	{ M_NUMBER,	"8",		0,		 8 },
	{ M_NUMBER,	"9",		0,		 9 },
	{ M_NUMBER,	"10",		0,		10 },
	{ M_NUMBER,	"11",		0,		11 },
	{ M_NUMBER,	"12",		0,		12 },

	{0,0,0,0}
    };

    enumError err = ERR_OK;
    bool table    = false;
    bool relative = false;
    bool mirror   = false;
    bool narrow   = false;
    bool hexmode  = false;
    uint z_mode   = 1;
    uint automode = 0x06;

    raw_data_t raw;
    InitializeRawData(&raw);
    kmp_t kmp;
    InitializeKMP(&kmp);

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	if (!param->arg)
	    continue;
	if ( strchr(param->arg,'/') || strchr(param->arg,'\\') )
	{
	    ResetKMP(&kmp);
	    if ( !strcmp(param->arg,"/") || !strcmp(param->arg,"\\") )
		continue;

	    enumError err = LoadRawData(&raw,false,param->arg,autoname,opt_ignore>0,0);
	    if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
		continue;
	    if ( err > ERR_WARNING )
		return err;
	    err = ScanRawDataKMP(&kmp,true,&raw,global_check_mode);
	    if ( err > ERR_WARNING )
		return err;
	    continue;
	}

	int cmd_stat;
	const KeywordTab_t * job = ScanKeyword(&cmd_stat,param->arg,keytab);
	if (!job)
	{
	    PrintKeywordError(keytab,param->arg,cmd_stat,0,0);
	    err = ERR_SYNTAX;
	    continue;
	}

	switch (job->id)
	{
	  case M_RESET:
	    table = relative = mirror = narrow = hexmode = false;
	    z_mode = 1;
	    automode = 0x06;
	    ResetKMP(&kmp);
	    break;

	  case M_TABLE:
	    table = job->opt != 0;
	    break;

	  case M_RELATIVE:
	    relative = job->opt != 0;
	    break;

	  case M_MIRROR:
	    mirror = job->opt != 0;
	    automode &= ~2;
	    break;

	  case M_NARROW:
	    narrow = job->opt != 0;
	    automode &= ~4;
	    break;

	  case M_AUTO:
	    automode = job->opt;
	    break;

	  case M_HEX:
	    hexmode = job->opt != 0;
	    break;

	  case M_Z:
	    z_mode = job->opt;
	    break;

	  case M_NUMBER:
	    {
		uint n = job->opt;
		uint amode = automode;
		if ( table && !relative )
		    amode |= 1;

		const double3 *tab
		    = GetStartPosKMP(0,n,mirror,narrow,&kmp,amode);
		if (!tab)
		    break;

		printf("\n%u player%s, %s (%s%u), %s (%s%u), %s%s:\n\n",
			n, n == 1 ? "" : "s",
			kmp_start_pos_mirror ? "pole is right" : "pole is left",
			 kmp_start_pos_auto & 2 ? "KMP=" : "",
			 kmp_start_pos_mirror,
			kmp_start_pos_narrow ? "narrow mode" : "wide mode",
			 kmp_start_pos_auto & 4 ? "KMP=" : "",
			 kmp_start_pos_narrow,
			kmp_start_pos_auto & 1 ? "absolute" : "relative",
			hexmode ? ", protocol hex mode" : "" );

		if (table)
		{
		    if (hexmode)
		    {
			const int origin
				= kmp.dlist[KMP_KTPT].used ? POSITION_ORIGIN : 0;

			uint i;
			for ( i = 0; i < n; i++ )
			    printf("%4u: %8x %8x %8x\n",
				i+1,
				double2int(tab[i].x*16.0) + origin,
				double2int(tab[i].y*16.0) + origin,
				double2int(tab[i].z*16.0) + origin );
		    }
		    else
		    {
			uint i;
			for ( i = 0; i < n; i++ )
			    printf("%4u: %11.3f %11.3f %11.3f\n",
				i+1, tab[i].x, tab[i].y, tab[i].z );
		    }
		}
		else
		{
		    const bool have_sep = n > 2
					&& IsNormalD(tab[0].z)
					&& IsNormalD(tab[1].z);
		    const double sep = have_sep ? 1.5*( tab[1].z - tab[0].z ) : 1e9;

		    uint i;
		    for ( i = 0; i < n; i++ )
		    {
			if ( have_sep
			    && i > 0
			    && IsNormalD(tab[i-1].z)
			    && tab[i].z >= tab[i-1].z + sep )
			{
			    putchar('\n');
			}

			int x = (int)tab[i].x;
			const int fw =	x < -900
					    ? 0
					    : x > 900
						? 1800/80
						: (900+x)/80;
			const char sign = x < 0 ? '-' : x > 0 ? '+' : ' ';
			if (hexmode)
			    printf("%3u:%*c%04x%*c",
				i+1, fw+3, sign, abs(x)*16, 26-fw, ':' );
			else
			    printf("%3u:%*c%03u%*c",
				i+1, fw+3, sign, abs(x), 26-fw, ':' );

			if ( z_mode > 0 && IsNormalD(tab[i].z) )
			{
			    if (hexmode)
				printf(" %5x",double2int(tab[i].z*16));
			    else
				printf(" %9.4f",tab[i].z);

			    if ( z_mode > 1 && i > 0 && IsNormalD(tab[i-1].z) )
			    {
				double delta = tab[i].z - tab[i-1].z;
				if (hexmode)
				    printf(" %5x",double2int(16*delta));
				else
				    printf(" %9.4f",delta);

				if ( z_mode > 2 && i > 1 && IsNormalD(tab[i-2].z) )
				{
				    delta -= tab[i-1].z - tab[i-2].z;
				    if (hexmode)
					printf(" %5x",double2int(16*delta)&0xfffff);
				    else
					printf(" %9.4f",delta);
				}
			    }
			}
			putchar('\n');
		    }
		}
		putchar('\n');
	    }
	    break;
	}
    }
    ResetKMP(&kmp);
    ResetRawData(&raw);

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

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if (opt_ignore)
		continue;
	    return err;
	}

	if (!testmode)
	{
	    err = SaveTextKMP(&kmp,"-",false);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetKMP(&kmp);
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
	const file_format_t dest_ff = cmd_id == CMD_ENCODE ? FF_KMP : FF_KMP_TXT;

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

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if (opt_ignore)
		continue;
	    return err;
	}

	if (!testmode)
	{
	    err = dest_ff == FF_KMP
			? SaveRawKMP(&kmp,dest,opt_preserve)
			: SaveTextKMP(&kmp,dest,opt_preserve);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetKMP(&kmp);
    }

    ResetRawData(&raw);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command diff			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError diff_kmp ( ccp fname1, ccp fname2 )
{
    if ( verbose > 0 )
	printf("DIFF/KMP %s : %s\n", fname1, fname2 );

    kmp_t kmp1, kmp2;
    InitializeKMP(&kmp1);
    InitializeKMP(&kmp2);

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError err = LoadRawData(&raw,false,fname1,"course.kmp",false,0);
    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
    {
	err = ScanRawDataKMP(&kmp1,false,&raw,0);
	if (!err)
	{
	    err = LoadRawData(&raw,false,fname2,"course.kmp",false,0);
	    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	    {
		err = ScanRawDataKMP(&kmp2,false,&raw,0);
		if (!err)
		{
		    const int diff_verbose = verbose + 2;
		    err = DiffKMP( &kmp1, &kmp2, diff_verbose < 0 ? 0 : diff_verbose );
		    if ( verbose >= -2 && err == ERR_DIFFER )
			printf("KMP differ: %s : %s\n", kmp1.fname, kmp2.fname );
		    else if ( verbose >= 0 && err == ERR_OK )
			printf("KMP identical: %s : %s\n", kmp1.fname, kmp2.fname );
		}
	    }
	}
    }

    ResetKMP(&kmp1);
    ResetKMP(&kmp2);
    ResetRawData(&raw);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError diff_kmp_dest()
{
    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,"%F",0,false);
	const enumError err = diff_kmp(param->arg,dest);
	if ( max_err < err )
	     max_err = err;
	if ( err > ERR_WARNING || err && verbose < -1 )
	    break;
    }
    return max_err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_diff()
{
    if ( verbose > 0 )
	fprintf(stdlog,"KMP diff mode: %s\n",GetKmpDiffMode());

    SetPatchFileModeReadonly();

    if (opt_dest)
	return diff_kmp_dest();

    if ( n_param != 2 )
	return ERROR0(ERR_SYNTAX,"Exact 2 sources expected if --dest is not set.\n");

    ASSERT(first_param);
    ASSERT(first_param->next);
    return diff_kmp(first_param->arg,first_param->next->arg);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command draw			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_draw()
{
    if (opt_png)
    {
	KCL_MODE = KCL_MODE & ~KCLMD_M_PRESET | KCLMD_DRAW;
	disable_transformation++;
    }

    const file_format_t ff_dest = opt_png ? FF_PNG : FF_KCL_TXT;
    ccp def_path = opt_png ? "\1P/\1N.kmp.png" : "\1P/\1N.kmp.obj";
    CheckOptDest(def_path,false);
    enable_kcl_drop_auto++;

    noPRINT(">>> cmd_draw() png=%d, ff=%u,%s, def_path=%s\n",
	opt_png, ff_dest, GetNameFF(ff_dest,0), def_path );

    const bool detailed = ( KMP_DRAW & DMD_DETAILED ) != 0;
    const bool warnings = ( KMP_DRAW & DMD_WARNINGS ) != 0;

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
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,
			GetExtFF(ff_dest,0), false );

	if ( verbose >= 0 || testmode )
	{
	    fprintf(stdlog,"%s%sCREATE %s:%s -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(raw.fform,0), raw.fname,
			GetNameFF(ff_dest,0), dest );
	    fflush(stdlog);
	}

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,global_check_mode);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if (opt_ignore)
		continue;
	    return err;
	}

	if (!testmode)
	{
	    DrawKCL_t dk;
	    InitializeDrawKCL(&dk);
	    dk.kmp = &kmp;
	    dk.warnings = warnings;
	    dk.detailed = detailed;

	    if ( KMP_DRAW & DMD_KCL )
	    {
		DefineAutoLoadReferenceKCL(kmp.fname,".kmp.txt");
		dk.kcl = dk.draw = CreateReferenceKCL();
	    }
	    else
	    {
		dk.draw = MALLOC(sizeof(*dk.draw));
		InitializeKCL(dk.draw);

		if ( KMP_DRAW & DMD_M_NEED_REF )
		{
		    DefineAutoLoadReferenceKCL(kmp.fname,".kmp.txt");
		    dk.kcl = CreateReferenceKCL();
		}
		else
		    dk.kcl = 0;
	    }
	    DASSERT(dk.draw);
	    dk.draw->no_limit = true;

	    if ( KMP_DRAW & DMD_POTI )
	    {
		uint i;
		for ( i = 0; i < MAX_ROUTE; i++ )
		    dk.route[i] = DRTM_POTI;
	    }

	    if ( KMP_DRAW & DMD_AREA )
		AddAreas2KCL(&dk);

	    if ( KMP_DRAW & DMD_CKPT )
		AddCheckPoints2KCL( &dk, (KMP_DRAW&DMD_CJGPT) != 0 );

	    if ( KMP_DRAW & DMD_JGPT )
		AddArrow2KCL( &dk, KMP_JGPT, KCL_FLAG_JGPT,
				KCL_FLAG_JGPT_AREA, 900.0, 1200.0 );

	    if ( KMP_DRAW & DMD_KTPT )
	    {
		double len = 5300;
		if ( kmp.dlist[KMP_KTPT].used > 5 )
		{
		    // seems to be a battle arena (0=race,12=battle)
		    //   => don't draw the starting area
		    len = 0.0;
		}
		else if (kmp.dlist[KMP_STGI].used)
		{
		    const kmp_stgi_entry_t *stgi
			= (kmp_stgi_entry_t*)kmp.dlist[KMP_STGI].list;
		    if ( stgi->narrow_start == 1 )
			len = 4800;
		}
		AddArrow2KCL( &dk, KMP_KTPT, KCL_FLAG_KTPT,
				KCL_FLAG_KTPT_AREA, len, 2000.0 );
	    }

	    if ( KMP_DRAW & (DMD_ENPT|DMD_DISPATCH) )
		AddRoutes2KCL( &dk, KMP_ENPT, KCL_FLAG_ENPT );

	    if ( KMP_DRAW & DMD_ITPT )
		AddRoutes2KCL( &dk, KMP_ITPT, KCL_FLAG_ITPT );

	    if ( KMP_DRAW & DMD_CNPT )
		AddCannons2KCL(&dk);

	    if ( KMP_DRAW & (DMD_ITEMBOX|DMD_SOLIDOBJ) )
	    {
		uint result = OBF_SOLID | OBF_NO_ROUTE;
		uint mask   = result | OBF_ROAD;

		switch ( KMP_DRAW & (DMD_ITEMBOX|DMD_SOLIDOBJ) )
		{
		    case DMD_ITEMBOX:
			mask   |= OBF_ITEMBOX;
			result |= OBF_ITEMBOX;
			break;

		    case DMD_SOLIDOBJ:
			mask   |= OBF_ITEMBOX;
			break;

		    default:
			break;
		}
		AddObjectsByFlag2KCL(&dk,mask,result);
	    }

	    if ( KMP_DRAW & DMD_COIN )
		AddObjectsById2KCL(&dk,GOBJ_COIN);

	    if ( KMP_DRAW & DMD_DECORATION )
	    {
		uint result = OBF_NO_ROUTE;
		uint mask   = result | OBF_SOLID | OBF_ROAD;
		AddObjectsByFlag2KCL(&dk,mask,result);
	    }

	    if ( KMP_DRAW & DMD_ROADOBJ )
		AddObjectsByFlag2KCL(&dk,OBF_ROAD,OBF_ROAD);

	    if ( KMP_DRAW & (DMD_ITEMBOX|DMD_SOLIDOBJ|DMD_POTI) )
	    {
		const kmp_gobj_entry_t *op = (kmp_gobj_entry_t*)kmp.dlist[KMP_GOBJ].list;
		const kmp_gobj_entry_t *op_end = op +  kmp.dlist[KMP_GOBJ].used;
		for ( ; op < op_end; op++ )
		{
		    if ( op->route_id < MAX_ROUTE && op->obj_id < N_KMP_GOBJ )
		    {
			const u32 flags = ObjectInfo[op->obj_id].flags;
			if ( flags & OBF_SOLID && flags & (OBF_OPT_ROUTE|OBF_ALWAYS_ROUTE) )
			{
			    int drtm = 0;
			    Itembox_t ibox;
			    if (AnalyseItembox(&ibox,op))
			    {
				if ( KMP_DRAW & (DMD_ITEMBOX|DMD_POTI) )
				    drtm = ibox.item_player
						? DRTM_ITEMBOX_PLAYER
						: ibox.item_enemy
							? DRTM_ITEMBOX_ENEMY
							: DRTM_ITEMBOX;
			    }
			    else if ( KMP_DRAW & (DMD_SOLIDOBJ|DMD_POTI) )
				drtm = DRTM_GOBJ;

			    if ( drtm > 0 && dk.route[op->route_id] < drtm )
				dk.route[op->route_id] = drtm;
			}
		    }
		}
	    }

	    // draw always late; drawing depends on 'dk.route'
	    AddPotiRoutes2KCL(&dk);

	    DrawPosFiles(&dk);

	    // zero ground is the latest element
	    if ( KMP_DRAW & DMD_BLACK )
		AddZeroGround2KCL(&dk,KCL_FLAG_ZERO_GROUND_BLACK);
	    else if ( KMP_DRAW & DMD_WHITE )
		AddZeroGround2KCL(&dk,KCL_FLAG_ZERO_GROUND_WHITE);

	    if (opt_png)
		SaveImageKCL(&kmp,dk.draw,dest,false);
	    else
		SaveTextKCL(dk.draw,dest,false);

	    ResetKCL(dk.draw);
	    FREE(dk.draw);
	    if ( dk.kcl && dk.kcl != dk.draw )
	    {
		ResetKCL((kcl_t*)dk.kcl);
		FREE((kcl_t*)dk.kcl);
	    }
	}
	ResetKMP(&kmp);
    }

    ResetRawData(&raw);
    return ERR_OK;
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
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,autoname,opt_ignore>0,0);
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

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,0);
	if ( err <= ERR_WARNING && ( !mode || CheckKMP(&kmp,mode) ))
	    cmd_err = ERR_DIFFER;
	fflush(stdout);
	ResetKMP(&kmp);
	if ( err > ERR_WARNING )
	    return err;
    }

    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command stgi			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_routes()
{
    stdlog = stderr;
    raw_data_t raw;
    InitializeRawData(&raw);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,"/course.kmp",opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	{
	    if ( max_err < err )
		max_err = err;
	    continue;
	}

	if ( opt_ignore && raw.fform != FF_KMP && raw.fform != FF_KMP_TXT )
	    continue;

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,CMOD_SILENT);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if ( max_err < err )
		max_err = err;
	    continue;
	}


	//--- print infos

	static struct { uint sect; int pmode; ccp info; } *ptr, tab[] =
	{
	    { KMP_CKPH, -1, "CKPH (check point groups)" },
	    { KMP_CKPT,  1, "CKPT (check points)" },
	    { KMP_ENPH, -1, "ENPH (enemy route groups)" },
	    { KMP_ENPT,  1, "ENPT (enemy route points)" },
	    { KMP_ITPH, -1, "ITPH (item route groups)" },
	    { KMP_ITPT,  1, "ITPT (item route points)" },
	    {0,0}
	};

	for ( ptr = tab; ptr->info; ptr++ )
	{
	    kmp_linfo_t li;
	    if (AnalyseRouteKMP(&li,&kmp,ptr->sect))
	    {
		printf("\n%s with %u records\n",ptr->info,li.used);
		uint i, j;
		for ( i = 0; i < li.used; i++ )
		    if (li.summary[i])
		    {
			printf("%5u:%3u%c", i, li.n_point[i],
				li.gmode[i] & LINFOG_DISPATCH ? 'd' : ' ' );
			kmp_linfo_mode *md = li.list[i];
			for ( j = 0; j < li.used; j++, md++ )
			    if (*md)
				printf(" %3u:%s",j,PrintLinkInfoMode(*md,ptr->pmode,true));
			putchar('\n');
		    }
	    }
	}

	ResetKMP(&kmp);
    }
    putchar('\n');

    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command gobj			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_gobj()
{
    patch_action_log_disabled++;
    raw_data_t raw;
    InitializeRawData(&raw);

    const ColorSet_t * col = GetFileColorSet(stdout);

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

	if (print_header)
	    printf("\n%sFile %s%s:%s%s\n",
		col->status, col->file,
		GetNameFF(raw.fform,0), raw.fname, col->reset );

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,CMOD_SILENT);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if (opt_ignore)
		continue;
	    return err;
	}

	kmp_ana_gobj_t ag;
	AnalyseGobj(&ag,&kmp);

	static const char type_char[] = "0-.boa";
	uint i;
	const kmp_gobj_info_t *info;
	const kmp_gobj_entry_t *gobj;
	for ( i = 0, info = ag.info, gobj = ag.gobj; i < ag.n_gobj; i++, info++, gobj++ )
	    printf("%4d. %4x %4x : %4x %4x : %u %c %2x %3u : %3d\n",
		i,
		gobj->obj_id, info->obj_id, 
		gobj->ref_id, info->ref_id,
		info->type, type_char[info->type],
		info->status, info->ref_count, info->defobj );

	if (print_header)
	{
	    printf("%s%u object%s analyzed in %s: ",
		col->status,
		ag.n_gobj, ag.n_gobj == 1 ? "" : "s",
		PrintTimerUSec6(0,0,ag.dur_usec,false) );

	    printf("o=%u=%u+%u, en=%d,%d, do=%u/%u=%u+%u+%u\n",
		ag.n_object, ag.n_disabled, ag.n_enabled,
		ag.n_engine, ag.n_random,
		ag.n_defobj_used, ag.n_defobj,
		ag.n_defobj_bits, ag.n_defobj_or, ag.n_defobj_and );
	}

	ResetAnaGobj(&ag);
	ResetKMP(&kmp);
    }

    putchar('\n');
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command gamemodes		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_gamemodes()
{
    if ( brief_count > 1 )
	print_header = false;

    patch_action_log_disabled++;
    raw_data_t raw;
    InitializeRawData(&raw);

    const ColorSet_t * col = GetFileColorSet(stdout);

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

	if (print_header)
	    printf("\n%sFile %s%s:%s%s\n",
		col->status, col->file,
		GetNameFF(raw.fform,0), raw.fname, col->reset );

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,CMOD_SILENT);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if (opt_ignore)
		continue;
	    return err;
	}

	kmp_ana_pflag_t ap;
	AnalysePFlagScenarios(&ap,&kmp,opt_gamemodes);

	uint ri;
	const kmp_ana_pflag_res_t *res;
	for ( ri = 0, res = ap.res_list; ri < ap.n_res; ri++, res++ )
	    if (brief_count)
		printf("%2u %s\n",res->version+1,res->name);
	    else if (long_count>2)
		printf("%3d. %3u %-13s  %u,%u,%02u,%02d,%u,%u,%u  %s\n",
			ri+1, res->version+1,
			res->name,
			res->lex_test.offline_online,
			res->lex_test.n_offline,
			res->lex_test.n_online,
			res->lex_test.cond_bit,
			res->lex_test.game_mode,
			res->lex_test.random,
			res->lex_test.engine,
			res->status );
	    else if (long_count)
		printf("%3d. %3u %-13s %s\n",ri+1,res->version+1,res->name,res->status);
	    else
		printf("%3d. %3u %s\n",ri+1,res->version+1,res->name);

	if (print_header)
	{
	    printf("%s%u object%s analyzed in %s: ",
		col->status,
		ap.ag.n_gobj, ap.ag.n_gobj == 1 ? "" : "s",
		PrintTimerUSec6(0,0,ap.dur_usec,false) );

	    if ( ap.n_version == ap.n_res )
		printf("%u different scenario%s total",
		    ap.n_res, ap.n_res == 1 ? "" : "s" );
	    else
		printf("%u scenario%s total, %u different",
		    ap.n_res, ap.n_res == 1 ? "" : "s", ap.n_version );

	    if ( ap.n_res_std && ap.n_res_ext )
		printf(" (%u standard code, %u extended code)",
			ap.n_res_std, ap.n_res_ext );
	    else if (ap.n_res_std)
		fputs(" (standard code only)",stdout);
	    else if (ap.n_res_ext)
		fputs(" (extended code only)",stdout);
	    printf(".%s\n",col->reset);
	}

	ResetPFlagScenarios(&ap);
	ResetKMP(&kmp);
    }

    putchar('\n');
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command wim0			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_wim0()
{
    patch_action_log_disabled++;
    raw_data_t raw;
    InitializeRawData(&raw);

    const ColorSet_t * col = GetFileColorSet(stdout);

    XDump_t xd;
    InitializeXDump(&xd);
    xd.print_format  = false;
    xd.print_summary = false;
    if ( long_count > 0 )
    {
	xd.min_addr_fw = 10;
    }
    else
    {
	xd.print_addr = false;
    }

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

	printf("\n%sFile %s%s:%s%s\n",
		col->status, col->file,
		GetNameFF(raw.fform,0), raw.fname, col->reset );

	kmp_t kmp;
	err = ScanRawDataKMP(&kmp,true,&raw,CMOD_SILENT);
	if ( err > ERR_WARNING )
	{
	    ResetKMP(&kmp);
	    if (opt_ignore)
		continue;
	    return err;
	}

	if ( !kmp.wim0_data || !kmp.wim0_size )
	    printf("%s> No WIM0 section found!%s\n",col->caption,col->reset);
	else
	{
	    const kmp_wim0_sect_t *sect = FindSectionWim0(0,&kmp,W0ID_VERSION);
	    printf("%s> WIM0 section (%u bytes total, %u payload) found, ",
			col->caption, kmp.wim0_import_size, kmp.wim0_size );
	    if (kmp.wim0_import_bz2)
		fputs("bzip2, ",stdout);

	    if (sect)
		printf("version %s%s\n", (ccp)sect->data, col->reset );
	    else
		printf("no version info found.%s\n", col->reset );

	    if ( long_count > 0 )
	    {
		kmp_wim0_info_t info;
		bool avail;
		for ( avail = GetFirstSectionWim0(&info,&kmp);
		      avail;
		      avail = GetNextSectionWim0(&info,&kmp) )
		{
		    printf("%s  index %d, offset 0x%x, %u bytes, section %s, type %s%s\n",
				col->heading,
				info.index, info.offset, info.size,
				info.name, Wim0FormatName[info.format],
				col->reset );
		    if ( info.size > 0 )
			XDump( &xd, info.data, info.size, true );
		}
	    }
	    else
	    {
		const uint fw = 103;
		printf("%s%.*s%s\n"
			"%sidx offset size section type   data%s\n"
			"%s%.*s%s\n",
			col->heading, 3*fw, ThinLine300_3, col->reset,
			col->heading, col->reset,
			col->heading, 3*fw, ThinLine300_3, col->reset );

		kmp_wim0_info_t info;
		bool avail;
		for ( avail = GetFirstSectionWim0(&info,&kmp);
		      avail;
		      avail = GetNextSectionWim0(&info,&kmp) )
		{
		    printf("%3d %05x %5u %-7s %-6s ",
			info.index, info.offset, info.size,
			info.name, Wim0FormatName[info.format] );

		    switch (info.format)
		    {
		     case W0F_TEXT:
			printf("%.70s\n",(ccp)info.data);
			break;

		     default:
			if ( info.size > 0 )
			    XDump( &xd, info.data, info.size < 0x10 ? info.size : 0x10, true );
			else
			    putchar('\n');
		    }
		}

		printf( "%s%.*s%s\n", col->heading, 3*fw, ThinLine300_3, col->reset );
	    }
	}
	ResetKMP(&kmp);
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

      RegisterOptionByName(&InfoUI_wkmpt,opt_stat,1,is_env);

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
	case GO_SCALE:		err += ScanOptScale(optarg); break;
	case GO_SHIFT:		err += ScanOptShift(optarg); break;
	case GO_XSS:		err += ScanOptXSS(0,optarg); break;
	case GO_YSS:		err += ScanOptXSS(1,optarg); break;
	case GO_ZSS:		err += ScanOptXSS(2,optarg); break;
	case GO_ROT:		err += ScanOptRotate(optarg); break;
	case GO_XROT:		err += ScanOptXRotate(0,optarg); break;
	case GO_YROT:		err += ScanOptXRotate(1,optarg); break;
	case GO_ZROT:		err += ScanOptXRotate(2,optarg); break;
	case GO_YPOS:		err += ScanOptYPos(optarg); break;
	case GO_TRANSLATE:	err += ScanOptTranslate(optarg); break;
	case GO_NULL:		force_transform |= 1; break;
	case GO_NEXT:		err += NextTransformation(false); break;
	case GO_ASCALE:		err += ScanOptAScale(optarg); break;
	case GO_AROT:		err += ScanOptARotate(optarg); break;
	case GO_TFORM_SCRIPT:	err += ScanOptTformScript(optarg); break;
	case GO_RM_GOBJ:	err += ScanOptRmGobj(optarg); break;
	case GO_BATTLE:		err += ScanOptBattle(optarg); break;
	case GO_EXPORT_FLAGS:	err += ScanOptExportFlags(optarg); break;
	case GO_ROUTE_OPTIONS:	err += ScanOptRouteOptions(optarg); break;
	case GO_WIM0:		err += ScanOptWim0(optarg); break;
	case GO_SLOT:		err += ScanOptSlot(optarg); break;
	case GO_DRAW:		err += ScanOptDraw(optarg); break;
	case GO_POS_FILE:	err += ScanOptPosFile(optarg); break;
	case GO_POS_MODE:	err += ScanOptPosMode(optarg); break;
	case GO_PNG:		err += ScanOptPng(optarg); break;
	case GO_LOAD_KCL:	err += ScanOptLoadKcl(optarg); break;
	case GO_KCL:		err += ScanOptKcl(optarg); break;
	case GO_KCL_FLAG:	err += ScanOptKclFlag(optarg); break;
	case GO_KCL_SCRIPT:	err += ScanOptKclScript(optarg); break;
	case GO_TRI_AREA:	err += ScanOptTriArea(optarg); break;
	case GO_TRI_HEIGHT:	err += ScanOptTriHeight(optarg); break;
	case GO_FLAG_FILE:	opt_flag_file = optarg; break;
	case GO_XTRIDATA:	opt_xtridata = optarg ? str2ul(optarg,0,10) : 1; break;
	case GO_KMP:		err += ScanOptKmp(optarg); break;
	case GO_SPEED_MOD:	err += ScanOptSpeedMod(optarg); break;
	case GO_KTPT2:		err += ScanOptKtpt2(optarg); break;
	case GO_TFORM_KMP:	err += ScanOptTformKmp(optarg); break;
	case GO_GAMEMODES:	err += ScanOptGamemodes(optarg); break;
	case GO_REPAIR_XPF:	err += ScanOptRepairXPF(optarg); break;

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
//	case GO_FULL:		full_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_EXPORT:		export_count++; break;
	case GO_EPSILON:	err += ScanOptEpsilon(optarg); break;
	case GO_DIFF:		err += ScanOptKmpDiff(optarg); break;
	case GO_NO_PARAM:	print_param = false; break;
	case GO_NO_ECHO:	opt_no_echo = true; break;
	case GO_GENERIC:	opt_generic = true; break;
	case GO_NO_CHECK:	opt_no_check = true; break;
	case GO_SECTIONS:	print_sections++; break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wkmpt,TRACE_FILE,11);
 #endif
    CloseTransformation();
    NormalizeOptions( verbose > 3 && !is_env ? 2 : 0 );
    SetupKCL();
    SetupKMP();

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
	enumError err = VerifySpecificOptions(&InfoUI_wkmpt,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wkmpt);

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
	case CMD_HELP:		PrintHelp(&InfoUI_wkmpt,stdout,0,"HELP",0,URI_HOME,
					first_param ? first_param->arg : 0 ); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_OBJECTS:	err = cmd_objects(); break;
	case CMD_EXPORT:	err = cmd_export(); break;
	case CMD_XEXPORT:	err = cmd_xexport(); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupVarsKMP()); break;
	case CMD_FUNCTIONS:	SetupVarsKMP(); err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupVarsKMP()); break;
	case CMD_MATRIX:	err = cmd_matrix(); break;
	case CMD_FLOAT:		err = cmd_float(); break;
	case CMD_STARTPOS:	err = cmd_startpos(); break;

	case CMD_CAT:		err = cmd_cat(); break;
	case CMD_DECODE:	err = cmd_convert(cmd_ct->id,"DECODE","\1P/\1N.txt"); break;
	case CMD_ENCODE:	err = cmd_convert(cmd_ct->id,"ENCODE","\1P/\1N\1?T"); break;
	case CMD_DIFF:		err = cmd_diff(); break;
	case CMD_DRAW:		err = cmd_draw(); break;
	case CMD_CHECK:		err = cmd_check(); break;
	case CMD_STGI:		err = cmd_stgi(); break;
	case CMD_KTPT:		err = cmd_ktpt(); break;
	case CMD_ROUTES:	err = cmd_routes(); break;
	case CMD_GOBJ:		err = cmd_gobj(); break;
	case CMD_GAMEMODES:	err = cmd_gamemodes(); break;
	case CMD_WIM0:		err = cmd_wim0(); break;

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
    int main_wkmpt ( int argc, char ** argv )
#else
    int main ( int argc, char ** argv )
#endif
{
    print_title_func = print_title;
    SetupLib(argc,argv,WKMPT_SHORT,VERSION,TITLE);

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WKMPT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions("WKMPT_OPT",CheckOptions);
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

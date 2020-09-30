
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

#include <time.h>
#include <dirent.h>

#include "dclib-utf8.h"
#include "lib-szs.h"
#include "lib-brres.h"
#include "lib-xbmg.h"
#include "lib-kcl.h"
#include "lib-kmp.h"
#include "lib-lecode.h"
#include "lib-mdl.h"
#include "lib-pat.h"
#include "lib-breff.h"
#include "lib-image.h"
#include "lib-common.h"
#include "lib-rkg.h"
#include "crypt.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wszst.c"
#include "db-mkw.h"
#include "lib-object.h"
#include "lib-checksum.h"

#if HAVE_WIIMM_EXT
  #include "lib-vehicle.h"
  #include "wcommand.h"
#endif

#include "distrib.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WSZST_SHORT ": " WSZST_LONG " v" VERSION " r" REVISION \
	" " SYSTEM " - " AUTHOR " - " DATE

#define TOOLSET_TITLE TOOLSET_SHORT ": " TOOLSET_LONG " v" VERSION " r" REVISION \
	" " SYSTEM " - " AUTHOR " - " DATE

//
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wszst,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wszst,stdout,0,0,"HELP",0,URI_HOME);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void list_compressions_exit()
{
    SetupColors();
    const int fw = GetTermWidth(80,40) - 1;
    PrintColoredLines(stdout,colout,0,fw,0,0,
	"\n{setup|%s}\n\n{caption|Compression modes:}\n"
	"\n"
	"There are several compression methods and most of them"
	" support different compression levels."
	" Each methods and level can be address by a numerical mode"
	" and some by a name.\n"
	"\n"
	"In general, 2 compression formats are supported: YAZ0/YAZ1 and BZ (bzip2).\n"
	"\n"
	"The following list shows all modes."
	" A mode can by selected by option {opt|--compr=param} or {opt|-C param}.\n"
	"|[3,9,23]\n"
	"{heading| numbers\tnames\tdescription}\n"
	"{heading|%.*s\n"
	"\t{hl| -1\tUNCOMPRESSED}\t|"
		"Don't compress.\n"
	"\t{hl|  0\tNOCHUNKS}\t|"
		"{cmd|YAZ} only: Use only byte copies.\n"
	"\n"
	"\t{hl|  1\tFAST}\t|"
		"Fastest available standard compression.\n"
	"\t{hl|1-8\t}\t|"
		"Standard levels between {par|FAST} and {par|BEST}.\n"
	"\t{hl|  9\tBEST}\t|"
		"Best and {par|default} standard compression.\n"
	"\n"
	"\t\t{hl|TRY2 - TRY5}\t|"
		"Because of many repeated data, the best {cmd|BZ} compression mode varies."
		" Therefor the levels {par|TRY2} to {par|TRY5} (or short {par|T2} to {par|T5})"
		" are defined to find the best compression mode with testing the first"
		" N levels of {par|9, 1, 8, 2, 5}. {par|TRY2} (levels 9 and 1)"
		" is used for normalizing."
		" For other than {cmd|BZ} compressions, {par|BEST} is used instead.\n"
	"\n"
	"\t{hl| 10\tULTRA}\t|"
		"A special time-consuming compression method for {cmd|YAZ} only."
		" It is dedicated to competitions with strict size limitations.\n"
	"\n"
	" {hl|100-150}\t\t|"
		"This {cmd|YAZ} compression uses a back tracking"
		" algorithm with recursion depth between {par|0 and 50} (last 2 digits)."
		" Depths 0 and 1 are similar to mode {par|9}."
		" Each new depth doubles the number of calculated pathes."
		" Because of some optimizations, a depth of +5 results"
		" in 10 to 15 times and not in 32 (2^5) times like expected."
		" This algorithm needs 4 times as much memory as the uncompressed file."
		" So it is dedicated to small files only.\n"
 #if HAVE_WIIMM_EXT && 0
	"{heading|%.*s\n"
	" {bad|500-530}\t\t|"
		"This {bad|experimental} {cmd|YAZ} compression uses a brute force"
		" algorithm with recursion depth from {par|0 to 30} (last 2 digits)."
		" It is a very time-consuming compression method"
		" and creates smaller files than {par|ULTRA}.\n"
	" {bad|600-630}\t\t|"
		"This {bad|experimental} {cmd|YAZ} compression"
		" is an optimized version of {par|100-130}.\n"
 #endif
	"{heading|%.*s\n"
	"\n"
	,TITLE
	,3*fw,ThinLine300_3
 #if HAVE_WIIMM_EXT && 0
	,3*fw,ThinLine300_3
 #endif
	,3*fw,ThinLine300_3
	);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WSZST_SHORT,WSZST_LONG);
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
	    ProgInfo.progname, CommandInfo[current_command->id].name1 );
    else
	fprintf(stderr,
	    "-> Type '%s -h' or '%s help' (pipe it to a pager like 'less') for more help.\n\n",
	    ProgInfo.progname, ProgInfo.progname );
    exit(stat);
}

///////////////////////////////////////////////////////////////////////////////

static void set_all ( bool force_cut )
{
    if ( all_count++ || force_cut )
	opt_cut = true;
    opt_decode = opt_encode_all = true;
    opt_mipmaps = 1;
    opt_recurse = INT_MAX;
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
    printf("  force:       %16x = %12d (kmp=%d)\n",force_count,force_count,force_kmp);
    printf("  sort:        %16x = %12d\n",opt_sort,opt_sort);
    printf("  width:       %16x = %12d\n",opt_width,opt_width);
    printf("  escape-char: %16x = %12d\n",escape_char,escape_char);
    printf("  align:       %16x = %12d\n",opt_align,opt_align);
    printf("  align-u8:    %16x = %12d\n",opt_align_u8,opt_align_u8);
    printf("  align-pack:  %16x = %12d\n",opt_align_pack,opt_align_pack);
    printf("  align-brres: %16x = %12d\n",opt_align_brres,opt_align_brres);
    printf("  align-breff: %16x = %12d\n",opt_align_breff,opt_align_breff);
    printf("  align-breft: %16x = %12d\n",opt_align_breft,opt_align_breft);
    printf("  tiny:        %16x = %12d\n",opt_tiny,opt_tiny);
    printf("  compression:          mode %2d, level %d\n",opt_compr_mode,opt_compr);
    printf("  recurse:     %16x = %12d\n",opt_recurse,opt_recurse);
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
    printf("  analyse-mode:%16x = %12d\n",opt_analyze_mode,opt_analyze_mode);
    printf("  set-flags:   %16x = %12d\n",set_flags,set_flags);
    printf("  kcl modes:   %16llx = \"%s\"\n",(u64)KCL_MODE,GetKclMode());

    printf("  kmp modes:   %16llx = \"%s\"\n",(u64)KMP_MODE,GetKmpMode());
    printf("  kmp tform:   %16x = \"%s\"\n",KMP_TFORM,GetKmpTform());
    if (speed_mod_active)
	printf("  kmp speed modifier:      %04x ~ %5.3f\n",
		speed_mod_val, speed_mod_factor );
    printf("  mdl modes:   %16x = \"%s\"\n",MDL_MODE,GetMdlMode());
    printf("  pat modes:   %16x = \"%s\"\n",PAT_MODE,GetPatMode());

    if (have_patch_count)
	printf("  patch count: %16x = %12d\n",have_patch_count,have_patch_count);
    printf("  patch files: %16x = \"%s\"\n",PATCH_FILE_MODE,GetFileClassInfo());
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
    SetPatchFileModeReadonly();

 #if 1 || !defined(TEST) // test options

    return cmd_test_options();

 #elif 1

    u8 * p1 = MALLOC(10);
    u8 * p2 = CALLOC(10,20);
    u8 * p3 = MALLOC(3000);
    DUMP_TRACE_ALLOC(stderr);
    p1[-1] = 'a';
    FREE(p1);
    FREE(p2);
    FREE(p3);
    DUMP_TRACE_ALLOC(stderr);
    return ERR_OK;

 #elif 1

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	SetupParam_t sp;
	InitializeSetupParam(&sp);
	enumError err = ScanSetupParam(&sp,true,param->arg,0,0,false);
	printf("err=%d, ff=%s, pt-dir=%d : %s\n",
		err, GetNameFF(sp.fform_file,sp.fform_arch),
		sp.have_pt_dir, param->arg );
	ResetSetupParam(&sp);
    }
    return ERR_OK;

 #elif 1

    const u64 base = 0x0807060504030201ull;
    u64 val;
	char buf[16];

    const endian_func_t * endian = &be_func;
    for(;;)
    {
	putchar('\n');

	memset(buf,0,sizeof(buf));
	endian->wr16(buf,base);
	HexDump16(stdout,0,0x16,buf,sizeof(buf));
	val = endian->rd16(buf);
	ASSERT_MSG( val == (u16)base, "%llx %llx\n", val, base );

	memset(buf,0,sizeof(buf));
	endian->wr24(buf,base);
	HexDump16(stdout,0,0x24,buf,sizeof(buf));
	val = endian->rd24(buf);
	ASSERT_MSG( val == ( (u32)base & 0xffffff ), "%llx %llx\n", val, base );

	memset(buf,0,sizeof(buf));
	endian->wr32(buf,base);
	HexDump16(stdout,0,0x32,buf,sizeof(buf));
	val = endian->rd32(buf);
	ASSERT_MSG( val == (u32)base, "%llx %llx\n", val, base );

	memset(buf,0,sizeof(buf));
	endian->wr48(buf,base);
	HexDump16(stdout,0,0x48,buf,sizeof(buf));
	val = endian->rd48(buf);
	ASSERT_MSG( val == ( base & 0xffffffffffff ), "%llx %llx\n", val, base );

	memset(buf,0,sizeof(buf));
	endian->wr64(buf,base);
	HexDump16(stdout,0,0x64,buf,sizeof(buf));
	val = endian->rd64(buf);
	ASSERT_MSG( val == base, "%llx %llx\n", val, base );

	if ( endian == &le_func )
	    break;
	endian = &le_func;
    }
    putchar('\n');
    return ERR_OK;

 #elif 1

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	char buf[1000];
	ccp subpath = SplitSubPath(buf,sizeof(buf),arg);
	if (!subpath)
	    printf("NOT FOUND: |%s|\n",buf);
	else if (!*subpath)
	    printf("REAL FILE: |%s|\n",buf);
	else
	    printf("SUB PATH:  |%s|%s|\n", buf, subpath );
    }
    return ERR_OK;

 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command autoadd			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError exec_autoadd
(
    UsedFileFILE_t	*used,		// vector with used files
    ccp			source		// filename of source file
)
{
    DASSERT(used);
    DASSERT(source);
    PRINT("exec_autoadd() sizeof(used)==%zu\n",sizeof(*used));

    char path_buf[PATH_MAX];

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,source,true,opt_ignore>0,true);

    if ( verbose >= 0 || testmode )
    {
	fprintf(stdlog,"%sANALYZE %s:%s\n",
		    verbose > 0 ? "\n" : "",
		    GetNameFF_SZS(&szs), szs.fname );
	fflush(stdlog);
    }

    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
    {
	CollectFilesSZS(&szs,true,0,-1,SORT_NONE);

	int idx;
	for ( idx = 0; idx < N_DB_FILE_FILE; idx++ )
	{
	    if (used->d[idx])
		continue;

	    const DbFileFILE_t *ptr = DbFileFILE + idx;
	    if (!DBF_ARCH_SUPPORT(ptr->flags))
		continue;

	    szs_subfile_t *file, *file_end = szs.subfile.list + szs.subfile.used;
	    for ( file = szs.subfile.list; file < file_end; file++ )
	    {
		ccp fpath = file->path;
		if( fpath[0] == '.' && fpath[1] == '/' )
		    fpath += 2;
		if (!strcmp(fpath,ptr->file))
		{
		    used->d[idx] = 2;
		    ccp path = PathCatPP(path_buf,sizeof(path_buf),opt_dest,ptr->file);
		    if ( verbose >= 0 || testmode )
			fprintf(stdlog,"  %sADD %7u, %s:%s\n",
			    testmode ? "WOULD " : "",
			    file->size, GetNameFF(0,ptr->fform), path );

		    File_t F;
		    CreateFileOpt(&F,true,path,testmode,0);
		    if (F.f)
		    {
			if (opt_preserve)
			    SetFileAttrib(&F.fatt,&szs.fatt,0);
			size_t wstat = fwrite( szs.data + file->offset,
						    1, file->size, F.f );
			if ( wstat != file->size )
			    err = FILEERROR1(&F,ERR_WRITE_FAILED,
				    "Writing %u bytes failed: %s\n",
				    file->size, path );
		    }
		    ResetFile(&F,opt_preserve);
		}
	    }
	}
	fflush(stdout);
    }

    ResetSZS(&szs);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError find_autoadd
(
    UsedFileFILE_t	*used,		// vector with used files
    ccp			source_dir,	// filename of source directory
    ccp			name,		// name of while without extension
    ccp			slot		// NULL or slot prefix
)
{
    DASSERT(used);
    DASSERT(source_dir);
    DASSERT(name);

    struct stat st;
    char pathbuf[PATH_MAX], fname[50];

    int try;
    for ( try = 0;; try++ )
    {
	switch (try)
	{
	    case 0:
		snprintf(fname,sizeof(fname),
			"%s", name );
		break;

	    case 1:
		snprintf(fname,sizeof(fname),
			"%s_d", name );
		break;

	    case 2:
		if (!slot)
		    *fname = 0;
		else
		    snprintf(fname,sizeof(fname),
			"%s-%s", slot, name );

		break;

	    case 3:
		if (!slot)
		    *fname = 0;
		else
		    snprintf(fname,sizeof(fname),
			"%s-%s_d", slot, name );
		break;

	    default:
		return ERR_OK;
	}

	if (*fname)
	{
	    ccp path = PathCatPPE(pathbuf,sizeof(pathbuf), source_dir, fname, ".szs" );
	    PRINT("-> %s\n",path);
	    if ( !stat(path,&st) && S_ISREG(st.st_mode) )
		return exec_autoadd(used,path);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_autoadd()
{
    SetPatchFileModeReadonly();

    char dest_path[PATH_MAX], path_buf[PATH_MAX];
    PathCatBufPP(dest_path,sizeof(dest_path),share_path,"auto-add/");
    CheckOptDest(dest_path,true);

    if ( !opt_mkdir && !ExistDirectory(opt_dest,0) )
	return ERROR0(ERR_CANT_CREATE_DIR,
	    "Directory does not exist: %s\n",opt_dest);
    opt_mkdir = true;

    if (!n_param)
	opt_overwrite = false;

    if ( !n_param || verbose >= 0 )
	fprintf(stdlog,"\nCURRENT AUTO-ADD PATH: %s\n",opt_dest);

    if ( !n_param || verbose >= 1 )
    {
	SetupAutoAdd();
	ccp *aap;
	for ( aap = auto_add_path; *aap; aap++ )
	    fprintf(stdlog,"       SEARCH PATH[%td]: %s\n",
			aap-auto_add_path,*aap);
    }


    //--- create file list

    UsedFileFILE_t used;
    memset(&used,0,sizeof(used));
    uint n_invalid = 0;

    if ( !opt_overwrite || !n_param )
    {
	if ( verbose >= 2 )
	    fprintf(stdlog,"\nLIST OF FILES IN %s\n", opt_dest );

	int count = 0;
	const DbFileFILE_t *ptr;
	for ( ptr = DbFileFILE; ptr->file; ptr++ )
	    if (DBF_ARCH_SUPPORT(ptr->flags))
	    {
		ccp path = PathCatPP(path_buf,sizeof(path_buf),opt_dest,ptr->file);
		struct stat st;
		if ( !stat(path,&st) && S_ISREG(st.st_mode) )
		{
		    used.d[ptr-DbFileFILE] = 1;
		    if ( verbose >= 2 )
		    {
			count++;
			fprintf(stdlog,"  %-5s %s\n",
			    GetNameFF(0,ptr->fform), ptr->file );
		    }

		    if ( verbose >= 1 )
		    {
			DASSERT( ptr->ref < N_DB_FILE_REF_FILE );
			const s16 ref = DbFileRefFILE[ptr->ref];
			noPRINT("%zd -> %d/%d : %d : %s\n",
				ptr-DbFileFILE, ptr->ref ,ptr->group, ref, ptr->file );
			if ( ref >= 0 )
			{
			    DASSERT( ref < N_DB_FILE );
			    const DbFile_t *db = DbFile + ref;
			    DASSERT( db->sha1 < N_DB_FILE_SHA1 );
			    const u8 *sha1 = DbFileSHA1[db->sha1].sha1;

			    sha1_size_t ss;
			    GetSSByFile(&ss,path,0);
			    if (memcmp(sha1,ss.hash,sizeof(ss.hash)))
			    {
				n_invalid++;
				used.d[ptr-DbFileFILE] = 2;
				HexDump(stdout,0,0,4,20,sha1,20);
				HexDump(stdout,0,0,4,20,ss.hash,20);
			    }
			}
		    }
		}
	    }
	if (count)
	    fprintf(stdlog,"  => %u of %zu files found.\n",count,sizeof(used));
    }


    //--- iterate parameters

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	if (!ExistDirectory(param->arg,0))
	{
	    enumError err = exec_autoadd(&used,param->arg);
	    if ( max_err < err )
		max_err = err;
	}
	else
	{
	    const DbFileSZS_t *db;
	    for ( db = DbFileSZS; db->szs_name; db++ )
		if ( (db->type & (FLT_OLD|FLT_STD)) == FLT_STD )
		    find_autoadd( &used, param->arg, db->szs_name,
					db->idx < 0 ? 0 : db->id+1 );
	}
    }


    //--- print missing list

    if ( verbose >= 1 )
    {
	fprintf(stdlog,"\nLIST OF MISSED FILES IN %s\n", opt_dest );

	int count = 0;
	const DbFileFILE_t *ptr;
	for ( ptr = DbFileFILE; ptr->file; ptr++ )
	    if ( !used.d[ptr-DbFileFILE] && DBF_ARCH_SUPPORT(ptr->flags) )
	    {
		count++;
		fprintf(stdlog,"  %-5s %s\n",
			GetNameFF(0,ptr->fform), ptr->file );
	    }
	if (count)
	    fprintf(stdlog,"  => %u of %zu files missed.\n",count,sizeof(used));
    }


    //--- print invalid list

    if (n_invalid)
    {
	fprintf(stdlog,"\nLIST OF INVALID FILES IN %s\n", opt_dest );

	const DbFileFILE_t *ptr;
	for ( ptr = DbFileFILE; ptr->file; ptr++ )
	    if ( used.d[ptr-DbFileFILE] == 2 )
		fprintf(stdlog,"  %-5s %s\n",
			GetNameFF(0,ptr->fform), ptr->file );
	fprintf(stdlog,"  => %u of %zu files invalid.\n",n_invalid,sizeof(used));
    }

    if ( verbose >= 1 )
	fputs("\n",stdlog);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command files			///////////////
///////////////////////////////////////////////////////////////////////////////

static int files_func
(
    ccp		fname,		// name of current file without '.szs' extension
    ccp		id,		// unique ID of current file
    FileList_t	mode,		// mode of current file
    const TrackInfo_t *ti,	// NULL or related track info
    void	*param		// user defined parameter
)
{
    char name[BMG_MSG_BUF_SIZE];
    name[0] = 0;
    const bmg_item_t *bi = 0;

    if ( ti && opt_load_bmg.item_used )
    {
	bi = FindItemBMG(&opt_load_bmg,ti->track_id+MID_LE_TRACK_BEG);
	if ( !bi || !bi->len )
	{
	    bi = FindItemBMG(&opt_load_bmg,ti->track_id+MID_CT_TRACK_BEG);
	    if ( !bi || !bi->len )
	    {
		bi = FindItemBMG(&opt_load_bmg,ti->bmg_mid);
		if ( bi && !bi->len )
		    bi = 0;
	    }
	}

	if (bi)
	{
	    PrintString16BMG(name,sizeof(name)-10,bi->text,bi->len,BMG_UTF8_MAX,0,0);
	    if (!strchr(name,'['))
	    {
		const uint pos = strlen(name);
		snprintf(name+pos,sizeof(name)-pos," [%c%02u]",
			mode & FLT_ARENA ? 'a' : 'r', ti->def_slot );
	    }
	}
    }

    if (pipe_count)
    {
	if (verbose>0)
	    printf("%s|0x%02x|%s|%s\n",id,mode,fname,name);
	else
	    printf("%s|%s|%s\n",id,fname,name);
    }
    else
    {
	if (verbose>0)
	    printf("%-5s 0x%02x  ",id,mode);
	else
	    printf("%-5s ",id);

	if (*name)
	    printf("%-21s %s\n",fname,name);
	else
	    printf("%s\n",fname);
    }
    return 1;
}

//-----------------------------------------------------------------------------

static enumError cmd_tracks()
{
    SetPatchFileModeReadonly();

    u32 mode = 0;
    if (first_param)
    {
	ScanNumU32(first_param->arg,0,&mode,0,FLT_M_ALL);
	mode &= FLT_M_ALL;
    }

    if (!mode)
    {
	mode = FLT_TRACK | FLT_ARENA | FLT_STD;

	if ( all_count > 1 )
	    mode = FLT_M_ALL;
	else if ( all_count )
	    mode |= FLT_D;

	if (long_count)
	{
	    mode |= FLT_OTHER;
	    if ( long_count > 1 )
		mode |= FLT_OLD;
	}
    }

    IterateTrackFiles(mode,files_func,0);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command scancache		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_scancache()
{
    if (opt_cache)
	AddParam(opt_cache);
    if ( n_param != 1 )
	return ERROR0(ERR_SYNTAX,"Exact one parameter (directory) expected.\n");

    if (!IsDirectory(first_param->arg,false))
	return ERROR0(ERR_SEMANTIC,"Directory expected: %s\n",first_param->arg);

    ScanSZSCache(first_param->arg,opt_purge);
    return SaveSZSCache();
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command export			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_export()
{
    SetPatchFileModeReadonly();
    DefineDefaultParserFunc();
    return ExportHelper("");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command _CODE			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_code()
{
    SetPatchFileModeReadonly();
    stdlog = stderr;

    if (!n_param)
	AddParam("-");

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	if (verbose)
	    fprintf(stderr,"CODE %s\n",param->arg);
	File_t F;
	if (!OpenFILE(&F,true,param->arg,false,true))
	{
	    while ( !feof(F.f) && !ferror(F.f) )
	    {
		uint size = fread(iobuf,1,sizeof(iobuf),F.f);
		u8 *src = (u8*)iobuf, *end = src + size;
		while ( src < end )
		    *src++ ^= 0xdc;
		fwrite(iobuf,1,size,stdout);
	    }
	}
	ResetFile(&F,false);
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command _RECODE			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef HAVE_WIIMM_EXT

 enumError cmd_recode()
 {
    ERROR0(ERR_NOT_IMPLEMENTED,
	"Command _RECODE not implemented in this version!\n");
    exit(ERR_NOT_IMPLEMENTED);
 }

#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command _SUBFILE		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_subfile()
{
    if (!first_param)
	return ERR_OK;

    SubDir_t dir;
    InitializeSubDir(&dir);

    NORMALIZE_FILENAME_PARAM(first_param);

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,first_param->arg,false,opt_ignore>0,true);
    if (!err)
    {
	DecompressSZS(&szs,true,0);
	#ifdef TEST
	{
	    printf("SZS: %p %zu / %p %zu\n",szs.cdata,szs.csize,szs.data,szs.size);

	    DumpInfoContainer(stdlog,collog,2,"SZS:  ",&szs.container,0x20);
	    szs_file_t szs2;
	    InitializeSubSZS(&szs2,&szs,4,12,FF_UNKNOWN,0,false);
	    printf("SZS2: %p %zu / %p %zu\n",szs2.cdata,szs2.csize,szs2.data,szs2.size);
	    DumpInfoContainer(stdlog,collog,2,"SZS1: ",&szs.container,0x20);
	    DumpInfoContainer(stdlog,collog,2,"SZS2: ",&szs2.container,0x20);
	    ResetSZS(&szs2);
	    DumpInfoContainer(stdlog,collog,2,"SZS:  ",&szs.container,0x20);
	}
	#endif
	DASSERT( !szs.file_size || szs.file_size >= szs.size );
	have_patch_count -= 1000000;
	PRINT("EXTRACT/%s[%s]: %s\n",__FUNCTION__,GetNameFF_SZS(&szs),szs.fname);
	err = ExtractFilesSZS(&szs,0,false,&dir,0);
	have_patch_count += 1000000;

	if ( opt_dest && *opt_dest )
	{
	    DumpSubDir(stdout,2,&dir);
	    SaveSubFiles(&dir,opt_dest);
	}

	SubFile_t *sf = FindSubFile(&dir,MemByString("3DModels(NW4R)/course"));
	if (sf)
	{
	    printf("** %s found [%p,0x%x] **\n",sf->fname,sf->data,sf->size);
	 #if 1
	    //ccp err = RenameForSlot42MDL(sf->data,sf->size,ContainerSZS(&szs));
	    ccp err = RenameForSlot42MDL(sf->data,sf->size,0);
	    if (err)
		printf("\e[35;1m %s \e[0m\n",err);
	    printf(">> SAVE %p,0x%x\n",sf->data,sf->size);
	    SaveFile("_mdl.tmp",0,FM_OVERWRITE|FM_TOUCH,sf->data,sf->size,0);
	 #else
	    mdl_t mdl;
	    ScanMDL(&mdl,true,sf->data,sf->size,ContainerSZS(&szs),0);
	    SetupSlot42Materials();
	    AppendMDL(&mdl,&slot42_mdl);
	    SaveRawMDL(&mdl,"_mdl.tmp",false);
	    ResetMDL(&mdl);
	 #endif
	}

	szs_file_t szs2;
	InitializeSZS(&szs2);
	enumError err = CreateSZS(&szs2,0,0,&dir,0,0,
			verbose > 0 ? UINT_MAX : 0, false );
	if (!err)
	    SaveSZS(&szs2,"_szs.tmp",true,false);
	ResetSZS(&szs2);
    }
    ResetSubDir(&dir);
    ResetSZS(&szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command BRSUB			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[check_brsub_t]]

typedef struct check_brsub_t
{
    szs_file_t		* szs;		// pointer to valid SZS
    int			mode;		// 0:warnings, 1:+hints, 2:+info

    bool		header_printed;	// true: header with filename printed
    bool		print_sep;	// true: print separator
    int			sep_fw;		// field width of separator
    uint		sep_count;	// number of printed lines after last sep

    enumError		max_err;	// error code
    ccp			brres_path;	// path of current BRRES
    uint		line_count;	// number of printed status lines
}
check_brsub_t;

///////////////////////////////////////////////////////////////////////////////

static int check_brsub
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    check_brsub_t *cb = (check_brsub_t*)it->param;
    DASSERT(cb);

    if (term)
	cb->brres_path = 0;
    else if ( it->size >= 0x0c )
    {
	u8 *data = it->szs->data + it->off;
	const file_format_t fform = GetByMagicFF(data,it->size,it->size);
	if ( fform == FF_BRRES )
	{
	    cb->brres_path = it->path;
	    cb->print_sep = print_header;
	}
	else if ( cb->brres_path && IsBRSUB(fform) )
	{
	    char stat_buf[50], vers_buf[50], ns_buf[50];
	    const int version		= it->endian->rd32(data+8);
	    const brsub_info_t *bi	= GetInfoBRSUB(fform,version);
	    const brsub_info_t *bic	= GetCorrectBRSUB(fform);
	    bool print_status		= true;
	    enumError err		= ERR_OK;

	    if (!bi)
	    {
		err = ERR_INVALID_DATA;
		snprintf(stat_buf,sizeof(stat_buf),
			"%sUnknown%s", colout->fatal, colout->reset );
	    }
	    else
	    {
		switch(bi->warn)
		{
		    case BIMD_OK:
			err = ERR_INVALID_DATA;
			print_status = cb->mode > 1;
			snprintf(stat_buf,sizeof(stat_buf),
				"%sOk     %s", colout->success, colout->reset );
			break;

		    case BIMD_INFO:
		    case BIMD_HINT:
			err = ERR_INVALID_VERSION;
			print_status = cb->mode > 0;
			snprintf(stat_buf,sizeof(stat_buf),
				"%sUnusual%s", colout->hint, colout->reset );
			break;

		    case BIMD_FAIL:
			err = ERR_INVALID_DATA;
			snprintf(stat_buf,sizeof(stat_buf),
				"%sFail   %s", colout->warn, colout->reset );
			break;

		    default:
		    //case BIMD_FATAL:
			err = ERR_INVALID_DATA;
			snprintf(stat_buf,sizeof(stat_buf),
				"%sFreeze %s", colout->bad, colout->reset );
			break;
		}
	    }

	    if ( cb->max_err < err )
		 cb->max_err = err;

	    if (print_status)
	    {
		if ( bic && version != bic->good_version )
		    snprintf(vers_buf,sizeof(vers_buf),
			    "%s%3d!%s", colout->hint, version, colout->reset );
		else
		    snprintf(vers_buf,sizeof(vers_buf), "%3d ", version );

		const int n_sect = GetGenericSectionNumBRSUB(data,it->size,it->endian);
		if ( !bic || n_sect != bic->good_sect )
		    snprintf(ns_buf,sizeof(ns_buf),
			    "%s%2d!%s", colout->fatal, n_sect, colout->reset );
		else
		    snprintf(ns_buf,sizeof(ns_buf), "%2d ", n_sect );

		if (!cb->header_printed)
		{
		    cb->header_printed = true;
		    cb->line_count = 999;
		    if (print_header)
			printf( "\n%s%s:%s%s\n"
			    "%s%.*s\n"
			    "%s Status  Type Ver N/s Filename of BRRES and sub file%s\n",
			    colout->caption, GetNameFF_SZS(cb->szs),
			    cb->szs->fname, colout->reset,
			    colout->heading, cb->sep_fw, ThinLine300_3,
			    colout->heading, colout->reset );
		    else
			printf("\n%s>%s:%s%s\n",
			    colout->caption, GetNameFF_SZS(cb->szs),
			     cb->szs->fname, colout->reset );
		}

		if ( cb->print_sep && cb->line_count > 2 )
		{
		    cb->print_sep = false;
		    cb->line_count = 0;
		    printf("%s%.*s%s\n",
			colout->heading, cb->sep_fw, ThinLine300_3, colout->reset );
		}

		cb->sep_count++;
		cb->line_count++;
		printf(" %s %-3s %s %s %s / %s\n",
			stat_buf, GetNameFF(0,bic?bic->fform1:fform),
			vers_buf, ns_buf,
			cb->brres_path, it->path );
	    }
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError cmd_brsub()
{
    if (!n_param)
    {
	const int fw = 3*34;
	if (print_header)
	    printf( "\n"
		"%s%.*s\n"
		"%s file format vers n(sect) warning%s\n",
		colout->heading, fw, ThinLine300_3,
		colout->heading, colout->reset );

	file_format_t last_ff = 0;
	const brsub_info_t *bi;
	for ( bi = brsub_info; bi->fform1; bi++ )
	{
	    if ( print_header && last_ff != bi->fform1 )
	    {
		last_ff = bi->fform1;
		if (print_header)
		    printf("%s%.*s%s\n",
			colout->heading, fw, ThinLine300_3, colout->reset );
	    }

	    printf(" %-3s %-7s %3d %6u   ",
			GetNameFF(0,bi->fform1),
			bi->fform2 ? GetNameFF(0,bi->fform2) : "-",
			bi->version, bi->n_sect );
	    switch(bi->warn)
	    {
		case BIMD_OK:
		    fputs("-\n",stdout);
		    break;

		case BIMD_INFO:
		case BIMD_HINT:
		    printf("%sUNUSUAL%s\n",colout->hint,colout->reset);
		    break;

		case BIMD_FAIL:
		    printf("%sFAIL%s\n",colout->warn,colout->reset);
		    break;

		default:
		//case BIMD_FATAL:
		    printf("%sFREEZE%s\n",colout->bad,colout->reset);
		    break;
	    }
	}

	if ( print_header )
	    printf("%s%.*s%s\n\n",
			colout->heading, fw, ThinLine300_3, colout->reset );

	return ERR_OK;
    }

    //--- have param => check BRSUB versions

    szs_file_t szs;
    InitializeSZS(&szs);
    uint line_count = 0;

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	ResetSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);

	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;
	if ( max_err < err )
	     max_err = err;

	check_brsub_t cb;
	memset(&cb,0,sizeof(cb));
	cb.szs    = &szs;
	cb.mode   = long_count;
	cb.sep_fw = 3*99;
	IterateFilesSZS(&szs,check_brsub,&cb,false,-1,-1,SORT_NONE);
	line_count += cb.line_count;

	if ( print_header && cb.header_printed )
	    printf("%s%.*s%s\n",
			colout->heading, cb.sep_fw, ThinLine300_3, colout->reset );

	if ( max_err < cb.max_err )
	     max_err = cb.max_err;
    }

    if (line_count)
	putchar('\n');
    ResetSZS(&szs);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command list			///////////////
///////////////////////////////////////////////////////////////////////////////

static int list_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if (term)
	return 0;

    ccp col_reset = "";
    if (colorize_stdout)
    {
	if (it->is_dir)
	{
	    fputs(colset->cmd,stdout);
	    col_reset = colset->reset;
	}
	else if ( it->has_subfiles )
	{
	    fputs(colset->heading,stdout);
	    col_reset = colset->reset;
	}
    }

    const int indent = 2 * it->recurse_level;

    if ( long_count > 1 )
    {
	if (it->is_dir)
	    printf("      -       -        -  -      -  %.*s%s%s\n",
			indent, indent_msg, it->path, col_reset );
	else
	{
	    char vers_buf[50];
	    PrintVersionSZS(vers_buf,sizeof(vers_buf),it);

	    const u8 * data = it->szs->data + it->off;
	    printf("%7x %7x %8u  %-4s %s %.*s%s%s\n",
			it->off, it->size, it->size,
			PrintID(data, it->size < 4 ? it->size : 4, 0 ),
			vers_buf,
			indent,indent_msg, it->path, col_reset );
	}
    }
    else if ( long_count > 0 )
    {
	if (it->is_dir)
	    printf("       -  -     %.*s%s%s\n",
			indent,indent_msg, it->path, col_reset );
	else
	    printf("%8u  %-4s  %.*s%s%s\n",
			it->size,
			PrintID( it->szs->data+it->off, it->size < 4 ? it->size : 4, 0 ),
			indent,indent_msg, it->path, col_reset );
    }
    else
	printf("%.*s%s%s\n",indent,indent_msg,it->path,col_reset);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_list ( int long_level )
{
    SetPatchFileModeReadonly();

    if ( opt_recurse < 0 )
	 opt_recurse = 0;

    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI_wszst,OPT_LONG,long_level,false);
	long_count += long_level;
    }

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( opt_norm || need_norm > 0 )
	    NormalizeSZS(&szs);

	if (print_header)
	{
	    printf("\n* Files of %s:%s\n",
			GetNameFF_SZS(&szs), param->arg );

	    static ccp f_or_d = "file or directory";
	    if ( long_count > 2 )
		PrintFileHeadSZS();
	    else if ( long_count > 1 )
		printf("\noff/hex siz/hex size/dec magic vers %s\n%.79s\n",f_or_d,Minus300);
	    else if ( long_count > 0 )
		printf("\nsize/dec  magic %s\n%.79s\n",f_or_d,Minus300);
	    else
		putchar('\n');
	}
	else if ( n_param > 1 )
	    printf("\n* Files of %s\n",param->arg);

	if ( long_count > 2 )
	    IterateFilesSZS(&szs,PrintFileSZS,0,true,opt_recurse,opt_cut,opt_sort);
	else
	    IterateFilesSZS(&szs,list_func,0,false,opt_recurse,opt_cut,opt_sort);
	ResetSZS(&szs);
    }

    if (print_header)
	putchar('\n');

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command name-ref		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_name_ref()
{
    stdlog = stderr;
    const SortMode_t std_sort = opt_sort == SORT_INAME ? SORT_INAME : SORT_BRRES;

    raw_data_t raw;
    InitializeRawData(&raw);

    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,
					"course_model.brres",opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	{
	    ResetRawData(&raw);
	    return err;
	}

	if ( raw.fform != FF_BRRES && !IsBRSUB(raw.fform) )
	{
	    printf("No BRRES family -> ignore %s:%s\n",
			GetNameFF(raw.fform,0), raw.fname );
	    continue;
	}

	szs_file_t szs;
	AssignSZS(&szs,true,raw.data,raw.data_size,false,raw.fform,raw.fname);
	raw.fname = NULL;

	name_ref_t nr;
	err = CreateNameRef(&nr,true,&szs,brief_count);
	if (err)
	    return err;

	if ( long_count > 0 )
	{
	    ListNameRef( stdout,0, &nr, std_sort );
	    ListNameRef( stdout,0, &nr, SORT_OFFSET );
	    if ( long_count > 1 )
		ListNameRef( stdout,0, &nr, SORT_NONE );
	}
	else
	    ListNameRef( stdout,0, &nr, opt_sort );

	ResetNameRef(&nr);
    }

    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command ilist			///////////////
///////////////////////////////////////////////////////////////////////////////

static int ilist_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if (term)
	return 0;

    if (!it->is_dir)
    {
	const u8 * data = it->szs->data+it->off;
// [[analyse-magic]]
	PrintImage( data, it->size, it->path,
			0, 2 * it->recurse_level, long_count,
			!it->has_subfiles
				|| GetByMagicFF(data,it->size,it->size) == FF_BREFF );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_ilist ( int long_level )
{
    SetPatchFileModeReadonly();

    if ( opt_recurse < 0 )
	 opt_recurse = 0;

    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI_wszst,OPT_LONG,long_level,false);
	long_count += long_level;
    }

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( opt_norm || need_norm > 0 )
	    NormalizeSZS(&szs);

	if (print_header)
	    PrintImageHead(0,long_count);
	else if ( n_param > 1 )
	    printf("\n* Files of %s\n",param->arg);

	IterateFilesSZS(&szs,ilist_func,0,false,-1,0,opt_sort);
	ResetSZS(&szs);
    }

    if (print_header)
	putchar('\n');

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command memory			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct dumpmem_t
{
    szs_file_t		* szs;		// the container
    u8			* data_base;	// pointer to base of complete file
    uint		last_off;	// offset of last analyzed data
    uint		last_end;	// end of last analyzed data
    bool		print_abs_off;	// true: print absolute offset

} dumpmem_t;

///////////////////////////////////////////////////////////////////////////////

static void DumpUnused
(
    dumpmem_t		* dm,		// dump info
    uint		offset,		// offset of object to analyze and dump
    file_format_t	fform,		// NULL for last entry or fileformat
    uint		recurse_level	// recurse level
)
{
    DASSERT(dm);

    recurse_level *= 2;
    if ( fform != FF_INVALID )
    {
	if (dm->print_abs_off)
	    printf("%7x ",offset);

	ccp ff = GetNameFF(fform,0);

	if ( dm->last_end < offset )
	    printf("%*s%-6.6s %6x ",
			recurse_level, "",
			ff,offset - dm->last_end );
	else if ( dm->last_end > offset )
	    printf("%*s%-6.6s %6x-",
			recurse_level, "",
			ff, dm->last_end - offset );
	else
	    printf("%*s%-6.6s      - ",
			recurse_level, "",
			ff );
    }
    else if ( dm->last_end < offset )
    {
	if (dm->print_abs_off)
	    printf("%7x ",offset);

	printf("%*s-      %6x\n",
			recurse_level, "",
			offset - dm->last_end);
    }
    else if ( dm->last_end > offset )
    {
	if (dm->print_abs_off)
	    printf("%7x ",offset);

	printf("%*s-      %6x-\n",
			recurse_level, "",
			dm->last_end - offset);
    }

    dm->last_off = dm->last_end = offset;
}

///////////////////////////////////////////////////////////////////////////////

static int memory_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    DASSERT(it->param);

    dumpmem_t *dm = it->param;

    if (term)
    {
	const uint abs_off = it->szs->data - dm->data_base + it->szs->size;
	noPRINT("TERM: %x %x %x\n",dm->last_off,dm->last_end,abs_off);
	DumpUnused(dm,abs_off,FF_INVALID,it->recurse_level);
    }
    else if (!it->is_dir)
    {
	if (!it->client_int)
	{
	    noPRINT("LAST: %x %x\n",dm->last_off, dm->last_end);
	    it->client_int++;
	    dm->last_end = dm->last_off;
	}

	const u8 *data = it->szs->data + it->off;
// [[analyse-magic]]
	file_format_t fform = GetByMagicFF(data,it->size,it->size);
	const uint abs_off = it->szs->data - dm->data_base + it->off;
	DumpUnused(dm,abs_off,fform,it->recurse_level);
	printf(" %6x .. %6x %6x  %s\n",
	    it->off, it->off + it->size, it->size, it->path );

	dm->last_off = abs_off;
	dm->last_end = abs_off + it->size;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_memory()
{
    SetPatchFileModeReadonly();

    if ( opt_recurse < 0 )
	 opt_recurse = 0;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( opt_norm || need_norm > 0 )
	    NormalizeSZS(&szs);

	dumpmem_t dm;
	memset(&dm,0,sizeof(dm));
	dm.szs		 = &szs;
	dm.data_base	 = szs.data;
	dm.print_abs_off = long_count > 0;

	if (print_header)
	{
	    printf("\n* memory dump of %s:%s\n",
			GetNameFF_SZS(&szs), param->arg );

	    fputs( dm.print_abs_off ? "\nabs off " : "\n", stdout);
	    printf( "type    unused  begin ..    end   size  file name\n"
		    "%.80s\n",
		    Minus300 );
	}

	IterateFilesSZS(&szs,memory_func,&dm,false,opt_recurse,opt_cut,SORT_OFFSET);
	ResetSZS(&szs);
    }

    if (print_header)
	putchar('\n');

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command dump			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dump()
{
    SetPatchFileModeReadonly();

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( opt_norm || need_norm > 0 )
	    NormalizeSZS(&szs);

	if ( szs.fform_arch == FF_BRRES || IsBRSUB(szs.fform_arch) )
	{
	    printf("\n* Dump structure of %s:%s\n",
			GetNameFF_SZS(&szs), param->arg );
	    DumpStructureBRRES(stdout,&szs);
	}
	else if (!opt_ignore)
	    ERROR0(ERR_WARNING,"No BRRES file -> ignored: %s\n",param->arg);
	ResetSZS(&szs);
    }
    putchar('\n');

    return ERR_OK;
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

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if ( max_err < err )
	    max_err = err;
	if ( err > ERR_WARNING || err == ERR_NOT_EXISTS )
	{
	    if ( opt_db64 )
	    {
		//fputs("--------------------------------",stdout);
		putchar('-');
		if (long_count)
		    fputs("  -1 -1 0.000  -",stdout);
		if (brief_count)
		    putchar('\n');
		else
		    printf("  %s\n",param->arg);
	    }

	    if ( err == ERR_NOT_EXISTS || opt_ignore )
		continue;
	    return err;
	}

	if ( opt_norm || need_norm > 0 )
	    NormalizeSZS(&szs);

	char checksum[100];
	CreateSSChecksumBySZS(checksum,sizeof(checksum),&szs);

	if (opt_verify)
	{
	    const uint clen = strlen(checksum);
	    ccp fname = strrchr(param->arg,'/');
	    fname = fname ? fname+1 : param->arg;
	    bool found = false;
	    for(;;)
	    {
		ccp pos = strstr(fname,checksum);
		if (!pos)
		    break;

		if ( pos == param->arg || pos[-1] == '.' || pos[-1] == '-' )
		{
		    char ch = pos[clen];
		    if ( !ch || ch == '.' || ch == '-' )
		    {
			found = true;
			break;
		    }
		}

		fname = pos+1; // search more
	    }

	    if (!found)
	    {
		if ( max_err < ERR_DIFFER )
		    max_err = ERR_DIFFER;
		if ( verbose >= 0 )
		    printf("Checksum differ: %s\n",param->arg);
	    }
	    else if ( verbose > 0 )
		printf("Checksum ok:     %s\n",param->arg);
	}
	else
	{
	    if (!opt_db64)
	    {
		if (long_count>1)
		{
	#if HAVE_FILEATTRIB_NSEC
		    struct tm * tm = localtime(&szs.fatt.mtime.tv_sec);
	#else
		    struct tm * tm = localtime(&szs.fatt.mtime);
	#endif
		    char timbuf[40];
		    strftime(timbuf,sizeof(timbuf),"%F %T ",tm);
		    fputs(timbuf,stdout);
		}
		if (long_count)
		    printf("%9zu,",szs.size);
	    }

	    fputs(checksum,stdout);

	    if ( opt_db64 && long_count )
	    {
		int	ckpt0_count	= -1;		// number of LC in CKPT
		int	lap_count	= 3;		// STGI lap counter
		float	speed_factor	= 1.0;		// STGI speed factor
		ccp slot_info = CreateSlotInfo(&szs);

		FindSpecialFilesSZS(&szs,false);
		if (szs.course_kmp_data)
		{
		    kmp_t kmp;
		    InitializeKMP(&kmp);
		    err = ScanKMP(&kmp,false,szs.course_kmp_data,szs.course_kmp_size,0);
		    if ( err <= ERR_WARNING )
		    {
			const uint n_ckpt =  kmp.dlist[KMP_CKPT].used;
			if (n_ckpt)
			{
			    const kmp_ckpt_entry_t *ckpt
				= (kmp_ckpt_entry_t*)kmp.dlist[KMP_CKPT].list;
			    uint i;
			    ckpt0_count = 0;
			    for ( i = 0; i < n_ckpt; i++, ckpt++ )
				if (!ckpt->mode)
				    ckpt0_count++;
			}

			const kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp.dlist[KMP_STGI].list;
			if ( kmp.dlist[KMP_STGI].used > 0 )
			{
			    lap_count = stgi->lap_count;

			    if (stgi->speed_mod)
				speed_factor = SpeedMod2float(stgi->speed_mod);
			}
		    }
		    ResetKMP(&kmp);
		}

		printf("  %d %d %5.3f  %s",ckpt0_count,lap_count,speed_factor,slot_info);
	    }

	    if (brief_count)
		putchar('\n');
	    else
		printf("  %s\n", param->arg );
	}
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command analyze			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_analyze()
{
    static ccp def_path = "\1P/\1F\1?T";
    CheckOptDest("-",false);
    char dest[PATH_MAX];
    enumError max_err = ERR_OK;
    if (!strcmp(opt_dest,"-"))
	fputc('\n',stdout);

    szs_file_t szs;
    InitializeSZS(&szs);

    analyse_szs_t as;
    InitializeAnalyseSZS(&as);

    File_t fo;
    InitializeFile(&fo);

    PrintScript_t ps;
    InitializePrintScript(&ps);
    ps.fo = &fo;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	ResetSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	{
	    if ( max_err < err )
		max_err = err;
	    continue;
	}

	if (!fo.f)
	    SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,
			GetExtFF(script_fform,0),false);

	if ( verbose > 0 )
	{
	    fprintf(stdlog,"%sANALYZE %s:%s => %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(szs.fform_file,szs.fform_arch), szs.fname,
			GetNameFF(script_fform,0), dest );
	    fflush(stdlog);
	}


	//--- analyse szs

	AnalyseSZS(&as,false,&szs,param->arg);


	//--- status

	if ( err >= ERR_WARNING )
	{
	    if ( max_err < err )
		 max_err = err;
	    continue;
	}

	if (!fo.f)
	{
	    enumError err = CreateFile(&fo,false,dest,FM_STDIO|FM_OVERWRITE);
	    if (err)
	    {
		max_err = err;
		break;
	    }
	    PrintScriptHeader(&ps);
	}


	//--- name attributes ('dest' can be used now)

	PrintEscapedString(dest,sizeof(dest),param->arg,-1,CHMD_UTF8,'"',0);

	uint name_attrib_len = 0;
	ccp name_attrib = strrchr(dest,'[');
	if (name_attrib)
	{
	    name_attrib++;
	    ccp end = strrchr(name_attrib,']');
	    if (end)
		name_attrib_len = end - name_attrib;
	}
	else
	    name_attrib = EmptyString;


	//--- print result

	PrintScriptVars(&ps,1,
		"file=\"%s\"\n"
		"size=%zd\n"
		 "db64=\"%s\"\n"
		 "sha1=\"%s\"\n"
		 "sha1_norm=\"%s\"\n"
		 "sha1_kcl=\"%s\"\n"
		 "sha1_kmp=\"%s\"\n"
		 "sha1_kmp_norm=\"%s\"\n"
		 "sha1_course=\"%s\"\n"
		 "sha1_vrcorn=\"%s\"\n"
		 "sha1_minimap=\"%s\"\n"
		"is_arena=\"%u %s\"\n"
		"n_ckpt0=%d\n"
		"lap_count=%d\n"
		"speed_factor=%5.3f\n"
		"slot_info=\"%s\"\n"
		"slot_attributes=\"%s\"\n"
		"race_slot=\"%u %s\"\n"
		"arena_slot=\"%u %s\"\n"
		"music_index=\"%u %s\"\n"
		 "used_x_pos=\"%u=%s %3.2f %3.2f %3.2f %3.2f\"\n"
		 "used_y_pos=\"%u=%s %3.2f %3.2f %3.2f %3.2f\"\n"
		 "used_z_pos=\"%u=%s %3.2f %3.2f %3.2f %3.2f\"\n"
		 "used_pos_suggest=\"%s\"\n"
		"ktpt2=\"%s %4.2f %4.2f\"\n"
		"gobj=\"%s\"\n"
		"special_kmp=\"%s\"\n"
		"special_files=\"%s\"\n"
		"lex_sections=\"%s\"\n"
		"warn=\"%u=%s\"\n"
		"ct_attributes=\"%s\"\n"
		"name_attributes=\"%.*s\"\n"
		"duration_usec=%llu\n"
		,dest
		,szs.size
		 ,as.db64
		 ,as.sha1_szs
		 ,as.sha1_szs_norm
		 ,as.sha1_kcl
		 ,as.sha1_kmp
		 ,as.sha1_kmp_norm
		 ,as.sha1_course
		 ,as.sha1_vrcorn
		 ,as.sha1_minimap
		,szs.is_arena
		,is_arena_name[szs.is_arena]
		,as.ckpt0_count
		,as.lap_count
		,as.speed_factor
		,as.slotana.slot_info
		,as.slotinfo.slot_attrib
		,as.slotinfo.race_slot,as.slotinfo.race_info
		,as.slotinfo.arena_slot,as.slotinfo.arena_info
		,as.slotinfo.music_index,as.slotinfo.music_info
		 ,as.used_pos.orig.rating[0]
			,WarnLevelNameLo[as.used_pos.orig.rating[0]]
		 ,as.used_pos.orig.min.x,as.used_pos.orig.max.x
			,fabsf(as.used_pos.orig.max.x-as.used_pos.orig.min.x)
			,(as.used_pos.orig.min.x+as.used_pos.orig.max.x)/2
		 ,as.used_pos.orig.rating[1],WarnLevelNameLo[as.used_pos.orig.rating[1]]
		 ,as.used_pos.orig.min.y,as.used_pos.orig.max.y
			,fabsf(as.used_pos.orig.max.y-as.used_pos.orig.min.y)
			,(as.used_pos.orig.min.y+as.used_pos.orig.max.y)/2
		 ,as.used_pos.orig.rating[2],WarnLevelNameLo[as.used_pos.orig.rating[2]]
		 ,as.used_pos.orig.min.z,as.used_pos.orig.max.z
			,fabsf(as.used_pos.orig.max.z-as.used_pos.orig.min.z)
			,(as.used_pos.orig.min.z+as.used_pos.orig.max.z)/2
		 ,GetUsedPosObjSuggestion(as.used_pos.suggest,true,"")
		,as.kmp_finish.warn ? "warn" : as.kmp_finish.hint ? "hint" : "ok"
		,as.kmp_finish.distance
		,as.kmp_finish.dir_delta
		,as.gobj_info
		,CreateSpecialInfoKMP(szs.kmp_special,true,"")
		,CreateSpecialFileInfo(&szs,true,"")
		,CreateSectionInfoLEX(szs.have_lex,true,"")
		,szs.warn_bits,GetWarnSZSNames(szs.warn_bits,' ')
		,as.ct_attrib+1
		,name_attrib_len,name_attrib
		,as.duration_usec
		);

	if ( long_count > 0 )
	{
	    const uint n = szs.special_file ? szs.special_file->used : 0;
	    PrintScriptVars(&ps,0,"subfile_n=%u\n",n);

	    uint i;
	    ParamFieldItem_t *ptr = szs.special_file->field;
	    for (i = 0; i < n; i++, ptr++ )
	    {
		sha1_hex_t hex;
		Sha1Bin2Hex(hex,(u8*)ptr->data);
		PrintScriptVars(&ps,0,"subfile_%u=\"%u %s %s\"\n",
			i, ptr->num, hex, ptr->key );
	    }
	}

	PutScriptVars(&ps,2,0);

	if (!script_array)
	{
	    PrintScriptFooter(&ps);
	    ResetFile(&fo,0);
	}
    }

    PrintScriptFooter(&ps);
    ResetPrintScript(&ps);
    ResetFile(&fo,0);
    ResetAnalyseSZS(&as);
    ResetSZS(&szs);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command distribution		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanDistribFile
	( DistributionInfo_t *dinf, ccp fname, int ignore, bool assume_arena )
{
    DASSERT(dinf);
    if ( ignore && IsDirectory(fname,true) )
	return ERR_OK;

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,fname,true,ignore>0,true);

    if ( err > ERR_WARNING || err == ERR_NOT_EXISTS )
    {
	ResetSZS(&szs);
	return err == ERR_NOT_EXISTS || ignore ? ERR_OK : err;
    }

    if ( szs.fform_arch != FF_U8 && szs.fform_arch != FF_WU8 )
    {
	ResetSZS(&szs);
	if ( ignore < 2 )
	    return ERROR0(ERR_INVALID_DATA,"Not a SZS ot WBZ file: %s\n",fname);
	return ERR_OK;
    }

    if ( opt_norm || need_norm > 0 )
	NormalizeExSZS(&szs,false,false,false);

    if ( verbose > 0 )
	printf("Read %s\n",fname);


    //--- cut filename

    char fname_buf[1000];

    ccp slash = strrchr(fname,'/');
    if (slash)
	fname = slash+1;

    StringCopyS(fname_buf,sizeof(fname_buf),fname);
    fname = fname_buf;

    char *src, *dest = strrchr(fname_buf,'.');
    if ( dest && strlen(dest) <= 4 )
	*dest = 0;

    src = dest = fname_buf;
    while (*src)
    {
	if (!memcmp(src,"ː",2))
	{
	    src += 2;
	    *dest++ = ':';
	}
	else if (!memcmp(src,"%3a",3))
	{
	    src += 3;
	    *dest++ = ':';
	}
	else
	    *dest++ = *src++;
    }
    *dest = 0;


    //--- create checksum

    char checksum[100];
    CreateSSChecksumBySZS(checksum,sizeof(checksum),&szs);
    ResetSZS(&szs);


    //--- save result

    uint slot = FindSlotByTranslation(&dinf->translate,fname,checksum);
    if (slot)
    {
	uint num;
	char *next = ScanSlot(&num,fname,false);
	if ( num == slot && (uchar)*next <= ' ' )
	    fname = next;
	else
	{
	    num = strtoul(fname,&next,10);
	    if ( num == slot && (uchar)*next <= ' ' )
		fname = next;
	}

	while ( *fname == ' ' )
	    fname++;
    }

    if (assume_arena)
	slot += DISTRIBUTION_ARENA_DELTA;

    PRINT(">>> slot=%d[%d,%d] %s\n",
		slot, IsValidDistribArena(slot), IsValidDistribTrack(slot), fname );

    if (IsValidDistribTrack(slot))
	AppendParamField(dinf->track+slot,fname,false,slot,STRDUP(checksum));
    else if (IsValidDistribArena(slot))
	AppendParamField(dinf->arena+slot-DISTRIBUTION_ARENA_DELTA,
				fname,false,slot,STRDUP(checksum));
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetDistribParamByNameS ( DistributionInfo_t *dinf, ccp name, ccp def )
{
    DASSERT(dinf);
    DASSERT(name);
    ParamFieldItem_t *it = FindParamField(&dinf->param,name);
    if ( it && it->data )
    {
	it->num = 1;
	return (ccp)it->data;
    }
    return def;
}

///////////////////////////////////////////////////////////////////////////////
// for name changes

#if 0

static ccp GetDistribParamByNameS2
	( DistributionInfo_t *dinf, ccp name1, ccp name2, ccp def )
{
    DASSERT(dinf);
    DASSERT(name);
    ParamFieldItem_t *it = FindParamField(&dinf->param,name1);
    if (!it)
	it = FindParamField(&dinf->param,name2);
    if ( it && it->data )
    {
	it->num = 1;
	return (ccp)it->data;
    }
    return def;
}

#endif

///////////////////////////////////////////////////////////////////////////////

static int GetDistribParamByNameIMM
	( DistributionInfo_t *dinf, ccp name, int def, int min, int max )
{
    DASSERT(dinf);
    DASSERT(name);
    ParamFieldItem_t *it = FindParamField(&dinf->param,name);
    if (!it)
	return def;
    it->num = 1;

    char *end;
    int val = str2l((ccp)it->data,&end,10);
    return *end || val < min || val > max ? def : val;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_distribution()
{
    SetPatchFileModeReadonly();
    SetupCoding64( opt_coding64, opt_db64 ? ENCODE_BASE64URL : ENCODE_BASE64 );

    DistributionInfo_t dinf;
    InitializeDistributionInfo(&dinf,true);

    char slotname[20];


    //--- define result file

    if ( !first_param || !first_param->arg || !*first_param->arg )
	return ERROR0(ERR_MISSING_PARAM,
		"Definition of result file missed (first parameter)!\n" );

    ParamList_t *param;
    char result[PATH_MAX], path[PATH_MAX];

    if (IsDirectory(first_param->arg,false))
    {
	PathCatPP(result,sizeof(result),first_param->arg,DEFAULT_DISTIBUTION_FNAME);
	param = first_param;
    }
    else
    {
	char *dest = StringCopyS(result,sizeof(result)-4,first_param->arg);
	if ( dest < result+4 || strcasecmp(dest-4,".txt") )
	    strcpy(dest,".txt");
	param = first_param->next;
    }


    //--- setup slot translation

    int i;
    for ( i = 0; i < MKW_N_ARENAS; i++ )
    {
	const TrackInfo_t *ti = arena_info+i;
	DefineSlotTranslation(&dinf.translate,true,ti->def_slot,ti->track_fname);
	DefineSlotTranslation(&dinf.translate,true,ti->def_slot,ti->name_en);
	DefineSlotTranslation(&dinf.translate,true,ti->def_slot,ti->name_de);
    }

    for ( i = 0; i < MKW_N_TRACKS; i++ )
    {
	const TrackInfo_t *ti = track_info+i;
	DefineSlotTranslation(&dinf.translate,false,ti->def_slot,ti->track_fname);
	DefineSlotTranslation(&dinf.translate,false,ti->def_slot,ti->name_en);
	DefineSlotTranslation(&dinf.translate,false,ti->def_slot,ti->name_de);
    }

    bool result_found = false;
    for ( i = 0; i < source_list.used; i++ )
    {
	ccp arg = source_list.field[i];
	if ( !i && !strcmp(arg,"0") )
	    ResetParamField(&dinf.translate);
	else if ( !result_found && !strcmp(result,arg) )
	{
	    result_found = true;
	    ScanSlotTranslation(&dinf.translate,&dinf.param,arg,false);
	}
	else
	    ScanSlotTranslation(&dinf.translate,0,arg,false);
    }

    if (!result_found)
	ScanSlotTranslation(&dinf.translate,&dinf.param,result,true);

    AddParamDistributionInfo(&dinf,false);


    //--- logging

    if ( logging > 0 )
    {
	fputs("---------------  translation  ---------------\n",stdlog);

	uint next_slot = 0;
	while ( next_slot < INT_MAX )
	{
	    uint slot =  next_slot;
	    next_slot = INT_MAX;

	    uint i;
	    const ParamFieldItem_t *it = dinf.translate.field;
	    for ( i = 0; i < dinf.translate.used; i++, it++ )
	    {
		if ( it->num == slot )
		{
		    if ( slot < DISTRIBUTION_ARENA_DELTA )
			fprintf(stdlog,"%4u. %3u.%u  %s\n",
			    i+1, it->num/10, it->num%10, it->key );
		    else
		    {
			snprintf( slotname, sizeof(slotname), "A%u.%u",
				(it->num-DISTRIBUTION_ARENA_DELTA)/10,
				it->num%10 );
			fprintf(stdlog,"%4u. %5s  %s\n", i+1, slotname, it->key );
		    }
		}
		else if ( it->num > slot && it->num < next_slot )
		    next_slot = it->num;
	    }
	}

	fputs("----------------  parameter  ----------------\n",stdlog);

	uint i;
	const ParamFieldItem_t *it = dinf.param.field;
	for ( i = 0; i < dinf.param.used; i++, it++ )
	    fprintf(stdlog,"%4u. %-20s = %s\n",
			i+1, it->key, (ccp)it->data );

	fputs("---------------------------------------------\n",stdlog);
    }


    //--- iterate files & directories

    if ( verbose == 0 )
	printf("Scan SZS files\n");

    for ( ; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	bool assume_arena = false;
	ccp ptr = param->arg;
	for(;;)
	{
	    ptr = strchr(ptr,'/');
	    if (!ptr)
		break;
	    ptr++;
	    if (!strncasecmp(ptr,"arena",5))
	    {
		assume_arena = true;
		PRINT("ASSUME ARENA: %s\n",param->arg);
		break;
	    }
	}

	if (IsDirectory(param->arg,false))
	{
	    DIR * dir = opendir(param->arg);
	    if (dir)
	    {
		for(;;)
		{
		    struct dirent * dent = readdir(dir);
		    if (!dent)
			break;

		    if ( *dent->d_name != '.' )
		    {
			PathCatPP(path,sizeof(path),param->arg,dent->d_name);
 #if HAVE_WIIMM_EXT // [[2do]] by option
			if ( !strstr(path,",clan]") && !strstr(path,",head=") )
 #endif
			{
			    enumError err = ScanDistribFile(&dinf,path,2,assume_arena);
			    if (err)
				return err;
			}
		    }
		}
		closedir(dir);
	    }
	}
	else
	{
	    enumError err = ScanDistribFile(&dinf,param->arg,opt_ignore,assume_arena);
	    if (err)
		return err;
	}
    }


    //--- print results

    if ( verbose >= 0 )
	printf("Create file %s\n",result);

    StringCat2S(path,sizeof(path),result,".bak");
    rename(result,path);

    FILE *f = fopen(result,"wb");
    if (!f)
	ERROR1(ERR_CANT_CREATE,"Can't create file: %s\n",result);

    ccp uuid = GetDistribParamByNameS(&dinf,"UUID","");
    fprintf(f,text_distrib_head_cr

		,GetDistribParamByNameS(&dinf,"COMMENT1","")
		,GetDistribParamByNameS(&dinf,"COMMENT2","")
		,GetDistribParamByNameS(&dinf,"COMMENT3","")
		,GetDistribParamByNameS(&dinf,"COMMENT4","")
		,GetDistribParamByNameS(&dinf,"COMMENT5","")

		,GetDistribParamByNameS(&dinf,"USER-CT-WIIMM","")
		,GetDistribParamByNameS(&dinf,"USER-WIIMMFI","")
		,GetDistribParamByNameS(&dinf,"USER-CT-WIIKI","")
		,GetDistribParamByNameS(&dinf,"USER-MISC","")
		,GetDistribParamByNameS(&dinf,"MAIL","")
		,GetDistribParamByNameS(&dinf,"NOTE-FOR-WIMMM","")

		,GetDistribParamByNameS(&dinf,"NAME","?")
		,GetDistribParamByNameS(&dinf,"VERSION","?")
		,GetDistribParamByNameS(&dinf,"AUTHORS","?")
		,GetDistribParamByNameS(&dinf,"RELEASE-DATE","")
		,GetDistribParamByNameS(&dinf,"KEYWORDS","")
		,GetDistribParamByNameS(&dinf,"PREDECESSOR","")

		,GetDistribParamByNameS(&dinf,"WIIMMFI-REGION","")
		,GetDistribParamByNameS(&dinf,"INFO-TEXT","")
		,GetDistribParamByNameS(&dinf,"INFO-URL","")

		,uuid
		,GetDistribParamByNameIMM(&dinf,"DISPLAY-MODE",3,0,3)
		,GetDistribParamByNameIMM(&dinf,"DATABASE-NAME",1,0,1)
		,GetDistribParamByNameIMM(&dinf,"VIEW-COMMENT",0,0,1)
		,GetDistribParamByNameIMM(&dinf,"ENABLE-NEW",0,0,1)
		,GetDistribParamByNameIMM(&dinf,"ENABLE-AGAIN",0,0,1)
		,GetDistribParamByNameIMM(&dinf,"ENABLE-FILL",0,0,1)
		,GetDistribParamByNameIMM(&dinf,"ENABLE-UPDATE",0,0,1)
		,GetDistribParamByNameIMM(&dinf,"ENABLE-BOOST",0,0,1)

		,uuid
		,VERSION
		,REVISION_NUM
		,GetDistribParamByNameS(&dinf,"FIRST-CREATION","")
		,GetDistribParamByNameS(&dinf,"LAST-UPDATE","")
		);



    //--- mark some more params as 'used'

    GetDistribParamByNameS(&dinf,"WSZST-VERSION","");
    GetDistribParamByNameS(&dinf,"WSZST-REVISION","");


    //--- additional parameters

    ccp info = text_distrib_param_cr;

    const ParamFieldItem_t *it = dinf.param.field;
    for ( i = 0; i < dinf.param.used; i++, it++ )
	if (!it->num)
	{
	    int len = strlen(it->key);
	    ccp sep;
	    if ( len > 14 )
		sep = " ";
	    else if ( len > 6 )
		sep = "\t";
	    else
		sep = "\t\t";

	    fprintf(f,"%s@%s%s= %s\r\n", info, it->key, sep, (ccp)it->data );
	    info = "";
	}


    //--- battle arenas

    fputs(text_distrib_tracks_cr,f);

    int slot, slot10 = -1, count = 0;
    for ( slot = 0; slot < MAX_DISTRIBUTION_ARENA; slot++ )
    {
	ParamField_t *pf = dinf.arena + slot;
	if ( pf->used )
	{
	    count++;
	    if ( slot10 != slot/10 )
	    {
		slot10 = slot/10;
		fputs("\r\n",f);
	    }

	    if (slot)
		snprintf(slotname,sizeof(slotname),"A%u.%u",slot10,slot%10);
	    else
		StringCopyS(slotname,sizeof(slotname),"arena");

	    uint i;
	    ParamFieldItem_t *it = pf->field;
	    for ( i = 0; i < pf->used; i++, it++ )
		fprintf(f,"%s %5s  %s\r\n", (ccp)it->data, slotname, it->key );
	}
    }
    if (count)
	fputs("\r\n",f);


    //--- racing tracks


    slot10 = -1;
    count = 0;

    for ( slot = 0; slot < MAX_DISTRIBUTION_TRACK; slot++ )
    {
	ParamField_t *pf = dinf.track + slot;
	if ( pf->used )
	{
	    count++;
	    if ( slot10 != slot/10 )
	    {
		slot10 = slot/10;
		fputs("\r\n",f);
	    }

	    char slotname[10];
	    if (slot)
		snprintf(slotname,sizeof(slotname),"%3u.%u",slot10,slot%10);
	    else
		StringCopyS(slotname,sizeof(slotname),"  ---");

	    uint i;
	    ParamFieldItem_t *it = pf->field;
	    for ( i = 0; i < pf->used; i++, it++ )
		fprintf(f,"%s %s  %s\r\n", (ccp)it->data, slotname, it->key );
	}
    }
    if (count)
	fputs("\r\n",f);
    fclose(f);

    ResetDistributionInfo(&dinf);
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

    szs_file_t szs1, szs2;
    InitializeSZS(&szs1);
    InitializeSZS(&szs2);

    enumError err = LoadCreateSZS(&szs1,fname1,true,false,true);
    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	err = LoadCreateSZS(&szs2,fname2,true,false,true);
    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	err = DiffSZS( &szs1, &szs2,
			opt_recurse < 0 ? 0 : opt_recurse,
			opt_cut ? 1 : -1,
			verbose < 0 );

    if ( verbose >= -1 && err == ERR_DIFFER )
	printf("Content differ: %s : %s\n", fname1, fname2 );
    else if ( verbose >= 0 && err == ERR_OK )
	printf("Content identical: %s : %s\n", fname1, fname2 );

    ResetSZS(&szs1);
    ResetSZS(&szs2);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError diff_dest()
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
    return max_err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_diff()
{
    SetPatchFileModeReadonly();

    if (opt_dest)
	return diff_dest();

    if ( n_param != 2 )
	return ERROR0(ERR_SYNTAX,"Exact 2 sources expected if --dest is not set.\n");

    ASSERT(first_param);
    ASSERT(first_param->next);
    return diff_files(first_param->arg,first_param->next->arg);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command check			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_check()
{
    SetPatchFileModeReadonly();

    patch_action_log_disabled++;
    if ( KCL_MODE & KCLMD_DROP_AUTO )
	SetKclMode(KCL_MODE|KCLMD_DROP_UNUSED);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);

	const bool check_verbose = verbose > 0 || long_count > 0;
	CheckMode_t mode = GetCheckMode(false,brief_count>0,verbose<0,check_verbose);
	if ( verbose >= 0 || testmode )
	{
	    ColorSet_t col;
	    SetupColorSet(&col,stdlog);
	    mode = GetCheckMode(true,brief_count>0,false,check_verbose);
	    fprintf(stdlog,"\n%sCHECK %s:%s%s\n",
			col.heading, GetNameFF_SZS(&szs), szs.fname, col.reset );
	    fflush(stdlog);
	}

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    switch (szs.fform_arch)
	    {
		case FF_KCL:
		case FF_KCL_TXT:
		case FF_WAV_OBJ:
		case FF_SKP_OBJ:
		{
		    kcl_t kcl;
		    err = ScanKCL(&kcl,true,szs.data,szs.size,true,mode);
		    ResetKCL(&kcl);
		}
		break;

		case FF_KMP:
		case FF_KMP_TXT:
		{
		    kmp_t kmp;
		    err = ScanKMP(&kmp,true,szs.data,szs.size,mode);
		    ResetKMP(&kmp);
		}
		break;

		case FF_U8:
		case FF_WU8:
		    if (CheckSZS(&szs,mode,global_check_mode))
			err = ERR_DIFFER;
		break;

		case FF_BRRES:
		    if (CheckBRRES(&szs,mode,0))
			err = ERR_DIFFER;
		break;

		default:
		    ERROR0(ERR_INVALID_FFORM,
			"CHECK doesn't support file format '%s': %s\n",
			GetNameFF_SZS(&szs), param->arg );
		break;
	    }
	    fflush(stdout);
	}

	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command slot			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_slots()
{
    SetPatchFileModeReadonly();

    KCL_MODE |= KCLMD_SILENT;
    if ( KCL_MODE & KCLMD_DROP_AUTO )
	SetKclMode(KCL_MODE|KCLMD_DROP_UNUSED);

    const ColorMode_t colmode = GetFileColorized(stdout);
    ccp col_minus = GetTextMode(colmode,TTM_BOLD|TTM_RED);
    ccp col_plus  = GetTextMode(colmode,TTM_BOLD|TTM_GREEN);
    ccp col_misc  = GetTextMode(colmode,TTM_BOLD|TTM_CYAN);
    ccp col_reset = GetTextMode(colmode,TTM_RESET);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);
	if (err)
	{
	    if ( max_err < err )
		 max_err = err;
	    ResetSZS(&szs);
	    continue;
	}
	if ( opt_norm || need_norm > 0 )
	    NormalizeSZS(&szs);
	//PatchSZS(&szs);

	int slot[MKW_N_TRACKS];
	int required_slot;
	const int stat = FindSlotsSZS(&szs,slot,&required_slot);

	noPRINT("IS_ARENA=%d\n",szs.is_arena);
	if ( szs.is_arena >= ARENA_FOUND )
	{
	    if (brief_count)
		printf("%sARENA%s     : %s\n",col_misc,col_reset,param->arg);
	    else
		printf("    %s ARENA %s     : %s\n",col_misc,col_reset,param->arg);
	}
	else
	{
	    if (brief_count)
	    {
		uint i, count = 0;
		for ( i = 0; i < MKW_N_TRACKS; i++ )
		    if ( slot[i] && slot[i] != stat )
		    {
			ccp col = slot[i] < 0 ? col_minus : slot[i] > 0 ? col_plus : col_misc;
			printf("%s%c%u%u",
			    col,
			    count ? ',' : stat < 0 ? '+' : '-',
			    i/4+1, i%4+1 );
			count++;
		    }

		while ( count++ < 3 )
		    fputs("   ",stdout);
	    }
	    else
	    {
		uint i, count = 0;
		for ( i = 0; i < MKW_N_TRACKS; i++ )
		    if ( slot[i] != stat )
		    {
			ccp col = slot[i] < 0 ? col_minus : slot[i] > 0 ? col_plus : col_misc;
			printf(" %s%c%u.%u",
				col,
				slot[i] < 0 ? '-' : slot[i] > 0 ? '+' : '?' ,
				i/4+1, i%4+1 );
			count++;
		    }
		    else if ( required_slot != 31 && ( i == 13 || i == 20 || i == 21 ))
		    {
			fputs("     ",stdout);
			count++;
		    }

		while ( count++ < 3 )
		    fputs("     ",stdout);
	    }
	    printf("%s : %s\n",col_reset,param->arg);
	}
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command isarena			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_isarena()
{
    SetPatchFileModeReadonly();

    if ( brief_count || verbose < 0 )
	print_header = false;

    if (print_header)
	printf("\n%sstatus   filename%s\n%s%.237s%s\n",
		colout->heading, colout->reset,
		colout->heading, ThinLine300_3, colout->reset );

    int count = 0;
    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,true);

	ccp status = 0, color  = 0;

	if ( err == ERR_NOT_EXISTS )
	{
	    if (opt_ignore)
		err = ERR_OK;
	    else
	    {
		status = "!NO-FILE";
		color  = colout->bad;
	    }
	}
	else if (err)
	{
	    status = "!ERROR";
	    color  = colout->bad;
	}
	else
	{
	    FindSpecialFilesSZS(&szs,false);
	    if (!szs.course_kmp_data)
	    {
		status = "!NO-KMP";
		color  = colout->bad;
	    }
	    else
	    {
		kmp_t kmp;
		InitializeKMP(&kmp);
		err = ScanKMP(&kmp,false,szs.course_kmp_data,szs.course_kmp_size,0);
		if (err)
		{
		    status = "!KMP-ERR";
		    color  = colout->bad;
		}
		else
		{
		    const IsArena_t is_arena = IsArenaKMP(&kmp);
		    switch (is_arena)
		    {
			case ARENA_NONE:
			    err	   = ERR_DIFFER;
			    status = "-RACE";
			    color  = colout->b_orange;
			    break;

			case ARENA_MAYBE:
			    err	   = ERR_DIFFER;
			    status = "?MAYBE";
			    color  = colout->b_yellow;
			    break;

			case ARENA_FOUND:
			    status = "+ARENA";
			    color  = colout->b_green;
			    break;

			case ARENA_DISPATCH:
			    status = "+DISPATCH";
			    color  = colout->b_green;
			    break;

			case ARENA__N:
			    break;
		    }

		    if (!status)
		    {
			status = "!UNKNOWN";
			color  = colout->bad;
		    }
		}
		ResetKMP(&kmp);
	    }
	}

	if (status)
	{
	    count++;
	    if ( verbose >= 0 )
	    {
		DASSERT(color);
		if (brief_count)
		    printf("%s%s%s\n", color, status, colout->reset );
		else
		    printf("%s%-9s%s %s\n", color, status, colout->reset, param->arg );
	    }
	}

	ResetSZS(&szs);
	if ( max_err < err )
	    max_err = err;
    }

    if (print_header)
	putchar('\n');

    return count ? max_err : ERR_NOTHING_TO_DO;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command patch			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool ScanByExtFF
	( file_format_t *ff_arch, file_format_t *ff_compr, ccp fname )
{
    DASSERT(ff_arch);
    DASSERT(ff_compr);
    *ff_arch  = FF_UNKNOWN;
    *ff_compr = FF_UNKNOWN;

    ccp point = strrchr(fname,'.');
    if (!point)
	return false;

    ++point;
    if (!strcmp(point,"szs"))
    {
	*ff_arch  = FF_U8;
	*ff_compr = FF_YAZ0;
	return true;
    }

    if (!strcmp(point,"wbz"))
    {
	*ff_arch  = FF_WU8;
	*ff_compr = FF_BZ;
	return true;
    }

    if (!strcmp(point,"u8"))
    {
	*ff_arch = FF_U8;
	return true;
    }

    if (!strcmp(point,"wu8"))
    {
	*ff_arch = FF_WU8;
	return true;
    }

    if (!strcmp(point,"xyz"))
    {
	*ff_arch = FF_XYZ;
	return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

// old [[container]] will be replaced by Container_t & ContainerData_t
#define PRINT_DATA PRINT("##### %s() #%u: cd=%p,%zu,%d / d=%p,%zu,%d / c=%p,%d #####\n", \
	__FUNCTION__, __LINE__, \
	szs.cdata, szs.csize, szs.cdata_alloced, \
	szs.data, szs.size, szs.data_alloced, \
	szs.old_container ? szs.old_container->data : 0, \
	szs.old_container ? szs.old_container->ref_count : -1 )

//-----------------------------------------------------------------------------

static enumError cmd_patch()
{
 #if HAVE_PRINT
    file_format_t dest_ff_arch;
    file_format_t dest_ff_compr;
    const bool dest_ff_valid = ScanByExtFF(&dest_ff_arch,&dest_ff_compr,opt_dest);
    PRINT("fform(dest) = %s [valid=%d]\n",
		GetNameFF(dest_ff_compr,dest_ff_arch), dest_ff_valid );
 #endif

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	if (!param->arg)
	    continue;
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,param->arg,false,opt_ignore>0,true);

	//PRINT_DATA;

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    u8 * orig_data;
	    uint orig_size;

	    const bool was_compressed = szs.cdata != 0;
	    if (was_compressed)
	    {
		DecompressSZS(&szs,false,0);
		DASSERT(szs.cdata);
		DASSERT(szs.cdata_alloced);
		orig_data = szs.cdata;
		orig_size = szs.csize;
		szs.cdata = 0;
		szs.cdata_alloced = false;
	    }
	    else
	    {
		DASSERT(szs.data);
		orig_data = MEMDUP(szs.data,szs.size);
		orig_size = szs.size;
	    }

 #ifdef TEST0
	    const ccp src_format = GetNameFF_SZS(&szs);
	    file_format_t local_ff_compr = opt_fform;
	    file_format_t local_ff_arch  = fform_compr;
 #else
	    const ccp src_format = GetNameFF_SZS(&szs);
	    const bool convert_wu8 = opt_fform == FF_WU8
			|| opt_fform == FF_UNKNOWN && szs.fform_arch == FF_WU8;
	    PRINT("CONVERT_WU8=%d, FF=%s\n", convert_wu8, src_format );
 #endif

	    DecodeWU8(&szs);

	    //szs.allow_ext_data = opt_norm;
	    szs.allow_ext_data = true;

	    bool dirty = PatchSZS(&szs);
	    if ( ( opt_norm || need_norm || opt_auto_add || dirty )
		    && NormalizeSZS(&szs) )
	    {
		dirty = true;
	    }
	    if ( OptionUsed[OPT_MINIMAP] && AutoAdjustMinimap(&szs) )
		dirty = true;

	    if (convert_wu8)
	    {
		err = EncodeWU8(&szs);
		if (err)
		    goto abort;
	    }


	    //--- dest file

	    char dest[PATH_MAX];
	    if ( opt_fform == FF_U8 || opt_fform == FF_WU8 )
	    {
		SubstDest( dest, sizeof(dest), param->arg,
		    opt_dest && *opt_dest ? opt_dest : "\1P/\1N\1?T", "\1N\1?T",
		    GetExtFF(szs.fform_file,opt_fform), false );
	    }
	    else if ( opt_dest && *opt_dest )
	    {
		SubstDest( dest, sizeof(dest), param->arg, opt_dest, 0,
		    GetExtFF(szs.fform_file,szs.fform_arch), false );
	    }
	    else
		StringCopyS(dest,sizeof(dest),param->arg);


	    //--- compression and SZS cache

	    if (was_compressed)
	    {
		CompressSZS(&szs,true);

		//PRINT_DATA;
		if ( opt_norm && !dirty )
		    dirty = orig_size != szs.csize
				|| memcmp(orig_data,szs.cdata,orig_size);
		FREE(orig_data);
		orig_data = szs.cdata;
		orig_size = szs.csize;
	    }
	    else
	    {
		//PRINT_DATA;
		if (!dirty)
		    dirty = orig_size != szs.size
				|| memcmp(orig_data,szs.data,orig_size);
		FREE(orig_data);
		orig_data = szs.data;
		orig_size = szs.size;
	    }

	    const ccp dest_format = GetNameFF_SZScurrent(&szs);
	    const bool src_dest_diff
		= strcmp(src_format,dest_format) || strcmp(dest,param->arg);

	    if ( dirty || src_dest_diff )
	    {
		if ( verbose >= 0 || testmode )
		{
		    if (src_dest_diff)
			fprintf(stdlog,"%s%s%s %s:%s -> %s:%s\n",
				testmode ? "WOULD " : "",
				opt_norm ? "NORMALIZE" : "PATCH",
				szs.aiparam_removed ? " [-AIParam]" : "",
				src_format, param->arg, dest_format, dest );
		    else
			fprintf(stdlog,"%s%s%s %s:%s\n",
				testmode ? "WOULD " : "",
				opt_norm ? "NORMALIZE" : "PATCH",
				szs.aiparam_removed ? " [-AIParam]" : "",
				dest_format, dest );
		    fflush(stdlog);
		}

		//PRINT_DATA;
		File_t F;
		err = CreateFileOpt(&F,true,dest,testmode,param->arg);
		if (F.f)
		{
		    SetFileAttrib(&F.fatt,&szs.fatt,0);
		    size_t wstat = fwrite(orig_data,1,orig_size,F.f);
		    if ( wstat != orig_size )
			err = FILEERROR1(&F,
				ERR_WRITE_FAILED,
				"Writing %u bytes failed [%zu bytes written]: %s\n",
				orig_size, wstat, dest );
		}
		ResetFile(&F,opt_preserve);
		if ( !err && opt_remove_src )
		    RemoveSource(param->arg,dest,verbose>=0,testmode);
	    }
	    else if ( verbose > 0 )
	    {
		fprintf(stdlog,"ALREADY NORMALIZED: %s:%s\n", src_format, param->arg );
		fflush(stdlog);
	    }

	    LinkCacheRAW(szs.cache_fname,dest,orig_data,orig_size);

	}
      abort:
	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command normalize		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_normalize()
{
    RegisterOptionByIndex(&InfoUI_wszst,OPT_NORM,1,false);
    opt_norm = true;
    return cmd_patch();
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command copy			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_copy()
{
    RegisterOptionByIndex(&InfoUI_wszst,OPT_OVERWRITE,1,false);
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

    return cmd_patch();
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command duplicate		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_duplicate()
{
    RegisterOptionByIndex(&InfoUI_wszst,OPT_OVERWRITE,1,false);
    opt_overwrite = true;


    //--- check parameters

    if (!first_param)
	return ERROR0(ERR_MISSING_PARAM, "Missing source file!\n" );

    if (!opt_dest)
    {
	if (!first_param->next)
	    return ERROR0(ERR_MISSING_PARAM, "Missing destination parameter!\n" );

	SetDest(first_param->next->arg,false);
	n_param--;
    }

    if ( n_param != 1 )
	return ERROR0(ERR_SEMANTIC,"Exact 1 source file expected!\n" );

    NORMALIZE_FILENAME_PARAM(first_param);
    ccp suffix, dest1 = opt_dest, dest2;
    {
	ccp slash = strrchr(dest1,'/');
	if (!slash)
	    slash = dest1;

	char *pos = strchr(slash,'@');
	if (pos)
	{
	    *pos = 0;
	    dest2 = pos+1;
	    while ( *dest2 == ' ' )
		dest2++;
	    suffix = !*dest2 || *dest2 == '.' ? "" : " ";
	    while ( pos > dest1 && pos[-1] == ' ' )
		*--pos = 0;
	}
	else
	{
	    pos = strrchr(slash,'.');
	    if (pos)
	    {
		*pos = 0;
		dest2 = pos+1;
		suffix = ".";
	    }
	    else
		dest2 = suffix = EmptyString;
	}
    }


    //--- open source

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,first_param->arg,false,opt_ignore>0,true);
    if (err)
	return err;

    const bool was_compressed = szs.cdata != 0;
    if (was_compressed)
	DecompressSZS(&szs,false,0);

    const ccp fform = GetNameFF_SZS(&szs);
    if ( verbose >= 0 )
    {
	fprintf(stdlog,"READ %s:%s\n",fform,first_param->arg);
	fflush(stdlog);
    }

    if ( szs.fform_current != FF_U8 && szs.fform_current != FF_WU8  )
	return ERROR0(ERR_INVALID_DATA,"A track file (SZS or WBZ) expected!\n");

    force_lex_test = true; // force patch to insert LEX/TEST
    PatchSZS(&szs);


    //--- find KMP

    FindSpecialFilesSZS(&szs,false);
    if (!szs.course_kmp_data)
	return ERROR0(ERR_INVALID_DATA,"KMP not found → abort: %s\n",first_param->arg);

    kmp_t kmp;
    err = ScanKMP(&kmp,true,szs.course_kmp_data,szs.course_kmp_size,0);


    //--- find LEX/TEST

    if (!szs.course_lex_data)
	return ERROR0(ERR_INTERNAL,0);

    lex_element_t *lex_test = FindLexElement( (lex_header_t*)szs.course_lex_data,
						szs.course_lex_size, LEXS_TEST );
    if (!lex_test)
	return ERROR0(ERR_INTERNAL,0);

    kmp_ana_pflag_t ap;
    AnalysePFlagScenarios(&ap,&kmp,opt_gamemodes);

    char buf[10];
    const int fw = snprintf(buf,sizeof(buf),"%u",ap.n_res);

    uint ri;
    const kmp_ana_pflag_res_t *res;
    for ( ri = 0, res = ap.res_list; ri < ap.n_res; ri++, res++ )
    {
	memcpy(lex_test->data,&res->lex_test,sizeof(res->lex_test));
	char fname[PATH_MAX];
	snprintf(fname,sizeof(fname),"%s {%02u,%s}%s%s",
		dest1, res->version+1, res->name, suffix, dest2 );
	if ( verbose >= 0 || testmode > 0 )
	{
	    fprintf(stdlog,"%sCREATE %*u/%u %s:%s\n",
		testmode > 0 ? "WOULD " : "", fw, ri+1, ap.n_res, fform, fname );
	    fflush(stdlog);
	}

	if (was_compressed)
	{
	    ClearCompressedSZS(&szs);
	    CompressSZS(&szs,false);
	}

	SaveSZS(&szs,fname,true,was_compressed);
    }

    ResetKMP(&kmp);
    ResetSZS(&szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command minimap			///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_mmap ( ccp title, float3 *a, float3 *b )
{
    DASSERT(title);
    DASSERT(a);
    DASSERT(b);

    printf("%-13s %9.1f %9.1f %9.1f  %20.1f %9.1f %9.1f\n",
	title, a->x, a->y, a->z, b->x, b->y, b->z );
}

///////////////////////////////////////////////////////////////////////////////

static void print_matrix ( ccp title, float *a, float *b )
{
    DASSERT(title);
    DASSERT(a);
    DASSERT(b);

    uint i;
    for ( i = 0; i < 3; i++ )
    {
	printf("%s-Matrix-%c: %9.1f %9.1f %9.1f %9.1f   %9.1f %9.1f %9.1f %9.1f\n",
		title, 'X'+i,
		a[0], a[1], a[2], a[3],
		b[0], b[1], b[2], b[3] );
	a += 4;
	b += 4;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void print_data
(
    mdl_minimap_t	*mmap,		// valid minimap record
    mdl_sect1_t		*root,		// NULL or valid 'root' data
    mdl_sect1_t		*ld,		// valid 'posLD' data
    mdl_sect1_t		*ru,		// valid 'posRU' data
    uint		mode,		// general print mode
    uint		modified	// 0:none, 1:translate, 2:all, 3:all+minimum
)
{
    if ( modified != 1 )
    {
	printf("Idx+Id+Flags: %9u %9u %#9x %21u %9u %#9x\n",
		ld->index, ld->id, ld->flags,
		ru->index, ru->id, ru->flags );
	print_mmap( "Scale:",	&ld->scale,  &ru->scale  );
	print_mmap( "Rotation",	&ld->rotate, &ru->rotate );
    }

    print_mmap( "Translation:",	&ld->translate, &ru->translate );
    if ( mmap->rec_trans_valid && ( !modified || mode > 0 ) )
	print_mmap(" Recommend.:",&mmap->rec_trans_ld,&mmap->rec_trans_ru);

    if ( mode > 1 && !modified )
    {
	print_mmap( "Minimum:", &ld->minimum, &ru->minimum );
	print_mmap( "Maximum:", &ld->maximum, &ru->maximum );
    }

    if ( mode > 0 )
    {
	putchar('\n');
	print_matrix("Tra",ld->trans_matrix.v,ru->trans_matrix.v);
	putchar('\n');
	print_matrix("Inv",ld->inv_matrix.v,ru->inv_matrix.v);
    }
    printf("%.95s\n",Minus300);

    if ( mode > 2 && !modified || modified > 2 )
    {
	printf("Vertex-Min: %11.1f %9.1f %9.1f\n",
			mmap->min.x, mmap->min.y, mmap->min.z );
	printf("Vertex-Max: %11.1f %9.1f %9.1f\n",
			mmap->max.x, mmap->max.y, mmap->max.z );
	if (mmap->root)
	{
	    printf("Root-Min:   %11.1f %9.1f %9.1f\n",
			root->minimum.x, root->minimum.y, root->minimum.z );
	    printf("Root-Max:   %11.1f %9.1f %9.1f\n",
			root->maximum.x, root->maximum.y, root->maximum.z );
	}
	printf("%.95s\n",Minus300);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void center_mmap ( uint idx, float3 *a, float3 *b )
{
    DASSERT(idx<3);
    DASSERT(a);
    DASSERT(b);

    const float new_val = ( a->v[idx] - b->v[idx] ) / 2;
    a->v[idx] = +new_val;
    b->v[idx] = -new_val;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_minimap()
{
    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,param->arg,false,opt_ignore>0,false);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    if ( verbose >= 0 )
		fprintf(stdlog,"\nMinimap data of %s:%s\n",
			GetNameFF_SZS(&szs), param->arg );

	    const bool was_compressed = szs.cdata != 0;
	    if (was_compressed)
		DecompressSZS(&szs,true,0);

	    bool dirty = false;
	    mdl_minimap_t mmap;
	    if (FindMinimapData(&mmap,&szs))
	    {
		DASSERT(mmap.posLD);
		DASSERT(mmap.posRU);
		PRINT("MMAP FOUND!\n");

		mdl_sect1_t root, ld, ru;
		ntoh_MDLs1(&root,mmap.root);
		ntoh_MDLs1(&ld,mmap.posLD);
		ntoh_MDLs1(&ru,mmap.posRU);

		if ( verbose >= 0 )
		{
		    printf("\n%53s%42s\n%.95s\n",
			"______________left-down_______________",
			"_______________right-up_______________",
			Minus300);
		    print_data(&mmap,&root,&ld,&ru,long_count,0);
		}

		if ( transform_active
			|| have_set_scale
			|| have_set_rot
			|| have_set_value
			|| OptionUsed[OPT_TOUCH]
			|| OptionUsed[OPT_AUTO]
			|| OptionUsed[OPT_XCENTER]
			|| OptionUsed[OPT_YCENTER]
			|| OptionUsed[OPT_ZCENTER]
			|| OptionUsed[OPT_CENTER]
			|| set_flags != M1(set_flags)
		   )
		{
		    uint modified = 1;
		    if ( OptionUsed[OPT_AUTO] && mmap.rec_trans_valid )
		    {
			modified = 2;
			ClearMDLs1(&ld,false);
			ld.translate = mmap.rec_trans_ld;
			ClearMDLs1(&ru,false);
			ru.translate = mmap.rec_trans_ru;
		    }

		    if ( set_flags != M1(set_flags) )
		    {
			modified = 2;
			ld.flags = ru.flags = set_flags;
		    }

		    if (have_set_scale)
		    {
			modified = 2;
			ld.scale.x = ru.scale.x = set_scale.x;
			ld.scale.y = ru.scale.y = set_scale.y;
			ld.scale.z = ru.scale.z = set_scale.z;
		    }

		    if (have_set_rot)
		    {
			modified = 2;
			ld.rotate.x = ru.rotate.x = set_rot.x;
			ld.rotate.y = ru.rotate.y = set_rot.y;
			ld.rotate.z = ru.rotate.z = set_rot.z;
		    }

		    if ( have_set_value & 1 )
		    {
			ld.translate.x = set_value_min.x;
			ru.translate.x = set_value_max.x;
		    }
		    if ( have_set_value & 2 )
		    {
			ld.translate.y = set_value_min.y;
			ru.translate.y = set_value_max.y;
		    }
		    if ( have_set_value & 4 )
		    {
			ld.translate.z = set_value_max.z;
			ru.translate.z = set_value_min.z;
		    }

		    if ( OptionUsed[OPT_XCENTER] || OptionUsed[OPT_CENTER] )
			center_mmap(0,&ld.translate,&ru.translate);
		    if ( OptionUsed[OPT_YCENTER] || OptionUsed[OPT_CENTER] )
			center_mmap(1,&ld.translate,&ru.translate);
		    if ( OptionUsed[OPT_ZCENTER] || OptionUsed[OPT_CENTER] )
			center_mmap(2,&ld.translate,&ru.translate);

		    TransformPosFloat3D(ld.translate.v,1,0);
		    TransformPosFloat3D(ru.translate.v,1,0);
		    CalcMatrixMDLs1(&ld);
		    CalcMatrixMDLs1(&ru);

		    hton_MDLs1(&ld,&ld);
		    hton_MDLs1(&ru,&ru);
		    dirty = memcmp(mmap.posLD,&ld,sizeof(ld))
			 || memcmp(mmap.posRU,&ru,sizeof(ru));

#if 1
		    if (mmap.root)
		    {
			root.minimum = mmap.min;
			root.maximum = mmap.max;

	    #if 0 // workaround fails
			// workaround for a bug in KMP modifier
			if ( root.minimum.x > root.minimum.z )
			     root.minimum.x = root.minimum.z;
			if ( root.maximum.x < root.maximum.z )
			     root.maximum.x = root.maximum.z;
	    #endif

			hton_MDLs1(&root,&root);
			if (memcmp(mmap.root,&root,sizeof(root)))
			{
			    memcpy(mmap.root,&root,sizeof(root));
			    modified = 3;
			    dirty = true;
			}
			ntoh_MDLs1(&root,&root);
		    }
#else
		    if ( mmap.root
			&& (   memcmp(&root.minimum,&mmap.min,sizeof(root.minimum))
			    || memcmp(&root.maximum,&mmap.max,sizeof(root.maximum)) ))
		    {
			root.minimum = mmap.min;
			root.maximum = mmap.max;

			hton_MDLs1(mmap.root,&root);
			modified = 3;
			dirty = true;
		    }
#endif

		    if (dirty)
		    {
			memcpy(mmap.posLD,&ld,sizeof(ld));
			memcpy(mmap.posRU,&ru,sizeof(ru));
			ntoh_MDLs1(&ld,&ld);
			ntoh_MDLs1(&ru,&ru);
			if ( verbose >= 0 )
			    print_data(&mmap,&root,&ld,&ru,long_count,modified);
		    }
		}
	    }

	    if (dirty)
	    {
		if ( opt_norm || need_norm > 0 )
		    NormalizeSZS(&szs);

		if ( verbose >= 0 || testmode )
		{
		    fprintf(stdlog,"%sPATCH MINIMAP %s:%s\n",
			testmode ? "WOULD " : "",
			GetNameFF_SZS(&szs), param->arg );
		    fflush(stdlog);
		}

		if (!testmode)
		    SaveSZS(&szs,szs.fname,true,was_compressed);
	    }
	}
	fflush(stdlog);

	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command compress		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_compress()
{
    static const char dest_fname[] = "\1P/\1N\1?T";
    CheckOptDest(dest_fname,false);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,param->arg,true,opt_ignore>0,false);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    file_format_t ff_dest = szs.fform_arch;
	    if (   ff_dest == FF_U8  && opt_fform == FF_WU8
		|| ff_dest == FF_WU8 && opt_fform == FF_U8 )
	    {
		 ff_dest = opt_fform;
	    }
	    PRINT("FF = %s , %s => %s\n",
		    GetNameFF_SZS(&szs),
		    GetNameFF_SZScurrent(&szs),
		    GetNameFF(0,ff_dest) );

	    char dest[PATH_MAX];
	    SubstDest(dest,sizeof(dest),param->arg,opt_dest,dest_fname,
			    GetExtFF(fform_compr,ff_dest),false);
	    if ( verbose >= 0 || testmode )
	    {
		fprintf(stdlog,"%s%sCOMPRESS %s:%s -> %s:%s\n",
			    verbose > 0 ? "\n" : "",
			    testmode ? "WOULD " : "",
			    GetNameFF_SZS(&szs), param->arg,
			    GetNameFF(fform_compr,ff_dest), dest );
		fflush(stdlog);
	    }

	    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	    {
		PatchSZS(&szs);
		if ( opt_norm || need_norm > 0 )
		    NormalizeSZS(&szs);

		if ( ff_dest == FF_WU8 )
		    err = EncodeWU8(&szs);

		if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
		{
		    szs.dest_fname = dest;
		    CompressSZS(&szs,true);
		    szs.dest_fname = 0;

		    if ( szs.cache_used && (int)max_err < ERR_CACHE_USED )
			max_err = ERR_CACHE_USED;

		    File_t F;
		    err = CreateFileOpt(&F,true,dest,testmode,param->arg);
		    if (F.f)
		    {
			SetFileAttrib(&F.fatt,&szs.fatt,0);
			size_t wstat = fwrite(szs.cdata,1,szs.csize,F.f);
			if ( wstat != szs.csize )
			    err = FILEERROR1(&F,ERR_WRITE_FAILED,
					"Writing %zu bytes failed: %s\n",
					szs.csize, dest);
		    }
		    ResetFile(&F,opt_preserve);
		    LinkCacheRAW(szs.cache_fname,dest,szs.cdata,szs.csize);

		    if ( !err && opt_remove_src )
			RemoveSource(param->arg,dest,verbose>=0,testmode);
		}
	    }
	}

	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command decompress		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_decompress()
{
    static const char dest_fname[] = "\1P/\1N\1?T";
    CheckOptDest(dest_fname,false);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,param->arg,true,opt_ignore>0,false);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    file_format_t ff_dest = szs.fform_arch;
	    if (   ff_dest == FF_U8  && opt_fform == FF_WU8
		|| ff_dest == FF_WU8 && opt_fform == FF_U8 )
	    {
		 ff_dest = opt_fform;
	    }
	    PRINT("FF = %s , %s => %s\n",
		    GetNameFF_SZS(&szs),
		    GetNameFF_SZScurrent(&szs),
		    GetNameFF(0,ff_dest) );

	    char dest[PATH_MAX];
	    ccp ext = GetExtFF(ff_dest,0);
	    SubstDest(dest,sizeof(dest),param->arg,opt_dest,ext,ext,false);

	    if ( verbose >= 0 || testmode )
	    {
		fprintf(stdlog,"%s%sDECOMPRESS %s:%s -> %s:%s\n",
			    verbose > 0 ? "\n" : "",
			    testmode ? "WOULD " : "",
			    GetNameFF_SZS(&szs), param->arg,
			    GetNameFF(0,ff_dest), dest );
		fflush(stdlog);
	    }

	    if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	    {
		PatchSZS(&szs);
		if ( opt_norm || need_norm > 0 )
		    NormalizeSZS(&szs);

		if ( ff_dest == FF_WU8 )
		    err = EncodeWU8(&szs);

		if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
		{
		    File_t F;
		    err = CreateFileOpt(&F,true,dest,testmode,param->arg);
		    if (F.f)
		    {
			SetFileAttrib(&F.fatt,&szs.fatt,0);
			size_t wstat = fwrite(szs.data,1,szs.size,F.f);
			if ( wstat != szs.size )
			    err = FILEERROR1(&F,ERR_WRITE_FAILED,
					"Writing %zu bytes failed: %s\n",
					szs.size, dest);
		    }
		    ResetFile(&F,opt_preserve);
		    if ( !err && opt_remove_src )
			RemoveSource(param->arg,dest,verbose>=0,testmode);
		}
	    }
	}

	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command create			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_create ( bool create )
{
    static const char dest_fname[] = "\1P/\1N\1?T";
    CheckOptDest(dest_fname,false);
    const bool save_auto_add = opt_auto_add;

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	opt_auto_add = save_auto_add; // restore auto-add settings

	int src_len = strlen(param->arg);
	while ( src_len > 0 && param->arg[src_len-1] == '/' )
	    src_len--;
	param->arg[src_len] = 0;

	SetupParam_t sp;
	InitializeSetupParam(&sp);
	ScanSetupParam(&sp,true,param->arg,SZS_SETUP_FILE,0,true);

	char dest[PATH_MAX];
	SubstDest(dest,sizeof(dest),param->arg,opt_dest,dest_fname,
			GetExtFF(sp.fform_file,sp.fform_arch),false);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = CreateSZS(&szs,dest,param->arg,0,&sp,0,
			verbose > 0 ? UINT_MAX : 0, false );

	//--- create file

	if ( verbose >= 0 || testmode )
	{
	    if (create)
		fprintf(stdlog,"%s%sCREATE %s/ -> %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			param->arg,
			GetNameFF_SZS(&szs),
			dest );
	    else
		fprintf(stdlog,"%s%sENCODE %s/\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			param->arg );
	    fflush(stdlog);
	}

	if ( create && err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    File_t F;
	    CreateFileOpt(&F,true,dest,testmode,0);
	    if (F.f)
	    {
		SetFileAttrib(&F.fatt,&szs.fatt,0);
		const u8 *   data = szs.cdata ? szs.cdata : szs.data;
		const size_t size = szs.cdata ? szs.csize : szs.size;
		const size_t wstat = fwrite(data,1,size,F.f);
		if ( wstat != size )
		    err = FILEERROR1(&F,ERR_WRITE_FAILED,
				"Writing %zu bytes failed: %s\n",
				size, dest);
	    }
	    ResetFile(&F,opt_preserve);
	    LinkCacheSZS(&szs,dest);
	}

	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
	ResetSetupParam(&sp);
    }
    opt_auto_add = save_auto_add; // restore auto-add settings

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command update			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct update_param_t
{
    ccp			source;		// source path
    bool		update_sub;	// true: update sub archives
    bool		encode;		// true: encode files if possible
    int			recurse_level;	// current recurse level
    int			indent;		// indention of log messages

} update_param_t;

//-----------------------------------------------------------------------------

static enumError job_update
(
    szs_file_t	*szs,		// valid szs file
    int		recurse_level	// recursion level
);

///////////////////////////////////////////////////////////////////////////////

static int update_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if (term)
	return 0;

    DASSERT(it);
    DASSERT(it->param);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    update_param_t * up = it->param;
    DASSERT(up);
    DASSERT(up->source);


    //--- ignore directories

    if (it->is_dir)
	return 0;


    //--- setup path

    ccp local_path = it->path;
    if ( *local_path == '.' && local_path[1] == '/' )
	local_path += 2;

    char path_buf[PATH_MAX];
    ccp path = PathCatPP(path_buf,sizeof(path_buf),up->source,local_path);

    u8 * data = szs->data + it->off;
// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,it->size,it->size);


    //--- [2do] encode files first


    //--- read updated file

    struct stat st;
    if (!stat(path,&st))
    {
	if (!S_ISREG(st.st_mode))
	{
	    if (!opt_ignore)
		ERROR0(ERR_WARNING,"No regular file: %s\n",path);
	}
	else if ( st.st_size != it->size )
	{
	    if (!opt_ignore)
		ERROR0(ERR_WARNING,
			"File size mismatch: have %llu, need %u: %s\n",
			(u64)st.st_size ,it->size, path );
	}
	else
	{
	    if ( verbose > 1 )
		fprintf(stdlog,"%*s- LOAD %s\n", up->indent,"", path );
	    LoadFILE(path,0,0,data,it->size,0,0,false);
	}
    }


    //--- update sub archives first

    if ( up->update_sub && IsArchiveFF(fform) )
    {
	szs_file_t subszs;	    // [[2do]] [[subfile]]
	InitializeSZS(&subszs);
	subszs.data		= data ;
	subszs.size		= it->size;
	subszs.fname		= path;
	subszs.fform_file	= fform;
	subszs.fform_arch	= fform;
	job_update(&subszs,up->recurse_level+1);
	subszs.fname		= EmptyString;
	ResetSZS(&subszs);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static enumError job_update
(
    szs_file_t	*szs,		// valid szs file
    int		recurse_level	// recursion level
)
{
    DASSERT(szs);
    MEM_CHECK;
    PRINT("JOB UPDATE: %d/%d %s\n",recurse_level,opt_recurse,szs->fname);

    char source[PATH_MAX];
    StringCat2S(source,sizeof(source),szs->fname,".d");
    if ( !recurse_level && ( opt_source || !IsDirectory(source,false) ) )
    {
	ccp my_source = opt_source;
	if (!my_source)
	{
	    switch (szs->fform_arch)
	    {
		case FF_U8:
		case FF_WU8:
		case FF_RARC:
		case FF_PACK:
		case FF_RKC:
		//case FF_BRRES:
		    my_source = "\1P/\1N.d/";
		    break;

		default:
		    my_source = "\1P/\1F.d/";
		    break;
	    }
	}
	SubstDest(source,sizeof(source),szs->fname,my_source,0,0,true);
    }

    const int indent = 2 * recurse_level;
    if ( verbose >= 0 && !recurse_level || verbose > 0 || testmode )
	fprintf(stdlog,"%s%*s- UPDATE %s:%s <- %s\n",
		verbose > 1 ? "\n" : "",
		indent,"",
		GetNameFF_SZS(szs), szs->fname, source );


    update_param_t up;
    memset(&up,0,sizeof(up));
    up.source		= source;
    up.encode		= !opt_no_encode;
    up.recurse_level	= recurse_level;
    up.update_sub	= recurse_level < opt_recurse;
    up.indent		= indent + 2;

    IterateFilesSZS(szs,update_func,&up,false,0,-1,SORT_NONE);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_update()
{
    if ( opt_source && !*opt_source )
	opt_source = 0;
    if ( opt_dest && !*opt_dest )
	opt_dest = 0;
    opt_mkdir = true;

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,false);
	if (!err)
	    err = job_update(&szs,0);
	if ( max_err < err )
	     max_err = err;

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    char dest[PATH_MAX];
	    SubstDest(dest,sizeof(dest),param->arg,opt_dest,0,0,false);

	    if ( verbose >= 0 || testmode )
	    {
		fprintf(stdlog,"%s%sCREATE %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF_SZS(&szs),
			dest );
		fflush(stdlog);
	    }

	    File_t F;
	    CreateFileOpt(&F,true,dest,testmode,param->arg);
	    if (F.f)
	    {
		if (IsCompressedFF(szs.fform_file))
		    CompressSZS(&szs,true);
		SetFileAttrib(&F.fatt,&szs.fatt,0);
		const u8 *   data = szs.cdata ? szs.cdata : szs.data;
		const size_t size = szs.cdata ? szs.csize : szs.size;
		const size_t wstat = fwrite(data,1,size,F.f);
		if ( wstat != size )
		    err = FILEERROR1(&F,ERR_WRITE_FAILED,
				"Writing %zu bytes failed: %s\n",
				size, dest);
	    }
	    ResetFile(&F,opt_preserve);
	}

	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command extract			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_extract ( enumCommands mode )
{
    ccp basedir = GetOptBasedir();
    if ( mode == CMD_XDECODE )
    {
	RegisterOptionByIndex(&InfoUI_wszst,OPT_DECODE,1,false);
	opt_decode = true;
    }
    else if ( mode == CMD_XEXPORT )
    {
	RegisterOptionByIndex(&InfoUI_wszst,OPT_DECODE,1,false);
	opt_decode = true;
	RegisterOptionByIndex(&InfoUI_wszst,OPT_EXPORT,1,false);
	export_count++;
    }
    else if ( mode == CMD_XALL )
    {
	RegisterOptionByIndex(&InfoUI_wszst,OPT_ALL,1,false);
	opt_recurse = INT_MAX;
	opt_decode  = true;
	opt_mipmaps = 1;
    }
    else if ( mode == CMD_XCOMMON )
    {
	opt_recurse = INT_MAX;
	opt_decode  = false;
	basedir	    = "common/";
    }

    if ( opt_dest && !*opt_dest )
	opt_dest = 0;
    opt_mkdir = true;

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadCreateSZS(&szs,param->arg,true,opt_ignore>0,false);

	if (!err)
	{
	    DASSERT( !szs.file_size || szs.file_size >= szs.size );

	    if ( analyze_fname && IsBRSUB(szs.fform_arch) )
		AnalyzeBRSUB(&szs,szs.data,szs.size,param->arg);

	    have_patch_count -= 1000000;
	    PRINT("EXTRACT/%s[%s]: %s\n",__FUNCTION__,GetNameFF_SZS(&szs),szs.fname);
	    err = ExtractFilesSZS(&szs,0,false,0,basedir);
	    have_patch_count += 1000000;
	}
	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			convert helper			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool ConvertHelper
(
    const u8		* data,		// valid data
    uint		data_size,	// size of 'data'
    ContainerData_t	* cdata,	// NULL or container data
    ccp			src_fname,	// filename of source
    ccp			dest_fname,	// filename of destination
    bool		binary_dest,	// output: false:text, true:binary
    enumError		* err		// not NULL: store error code here
)
{
    enumError temp_err;
    if (!err)
	err = &temp_err;
    *err = ERR_OK;

// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    switch(fform)
    {
     case FF_BMG:
     case FF_BMG_TXT:
	FreeContainerData(cdata);
	{
	    bmg_t bmg;
	    *err = ScanBMG(&bmg,true,src_fname,data,data_size);
	    if (*err)
		return true;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawXBMG(&bmg,dest_fname,true)
		: SaveTextXBMG(&bmg,dest_fname,true);
	    ResetBMG(&bmg);
	}
	return true;

     case FF_GH_ITEM:
     case FF_GH_IOBJ:
     case FF_GH_KART:
     case FF_GH_KOBJ:
     case FF_GH_ITEM_TXT:
     case FF_GH_IOBJ_TXT:
     case FF_GH_KART_TXT:
     case FF_GH_KOBJ_TXT:
	FreeContainerData(cdata);
	{
	    geohit_t geohit;
	    InitializeGEOHIT(&geohit,fform);
	    geohit.fname = src_fname;
	    *err = ScanGEOHIT(&geohit,false,data,data_size);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawGEOHIT(&geohit,dest_fname,true)
		: SaveTextGEOHIT(&geohit,dest_fname,true,minimize_level);
	    geohit.fname = 0;
	    ResetGEOHIT(&geohit);
	}
	return true;

     case FF_KCL:
     case FF_KCL_TXT:
     case FF_WAV_OBJ:
     case FF_SKP_OBJ:
	FreeContainerData(cdata);
	{
	    kcl_t kcl;
	    InitializeKCL(&kcl);
	    kcl.fname = src_fname;
	    *err = ScanKCL(&kcl,false,data,data_size,true,global_check_mode);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawKCL(&kcl,dest_fname,true)
		: SaveTextKCL(&kcl,dest_fname,true);
	    kcl.fname = 0;
	    ResetKCL(&kcl);
	}
	return true;

     case FF_ITEMSLT:
     case FF_ITEMSLT_TXT:
	FreeContainerData(cdata);
	{
	    itemslot_t itemslot;
	    InitializeITEMSLOT(&itemslot,fform);
	    itemslot.fname = src_fname;
	    *err = ScanITEMSLOT(&itemslot,false,data,data_size,src_fname);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawITEMSLOT(&itemslot,dest_fname,true)
		: SaveTextITEMSLOT(&itemslot,dest_fname,true,minimize_level);
	    itemslot.fname = 0;
	    ResetITEMSLOT(&itemslot);
	}
	return true;

     case FF_KMG:
     case FF_KMG_TXT:
	FreeContainerData(cdata);
	{
	    minigame_t minigame;
	    InitializeMINIGAME(&minigame,fform);
	    minigame.fname = src_fname;
	    *err = ScanMINIGAME(&minigame,false,data,data_size);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawMINIGAME(&minigame,dest_fname,true)
		: SaveTextMINIGAME(&minigame,dest_fname,true,minimize_level);
	    minigame.fname = 0;
	    ResetMINIGAME(&minigame);
	}
	return true;

     case FF_KMP:
     case FF_KMP_TXT:
	FreeContainerData(cdata);
	{
	    kmp_t kmp;
	    InitializeKMP(&kmp);
	    kmp.fname = src_fname;
	    *err = ScanKMP(&kmp,false,data,data_size,global_check_mode);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawKMP(&kmp,dest_fname,true)
		: SaveTextKMP(&kmp,dest_fname,true);
	    kmp.fname = 0;
	    ResetKMP(&kmp);
	}
	return true;

     case FF_LEX:
     case FF_LEX_TXT:
	FreeContainerData(cdata);
	{
	    lex_t lex;
	    InitializeLEX(&lex);
	    lex.fname = src_fname;
	    *err = ScanLEX(&lex,false,data,data_size);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawLEX(&lex,dest_fname,true)
		: SaveTextLEX(&lex,dest_fname,true);
	    lex.fname = 0;
	    ResetLEX(&lex);
	}
	return true;

     case FF_MDL:
     case FF_MDL_TXT:	// [[2do]] add format to UI commands BINARY and TEXT
	{
	    mdl_t mdl;
	    InitializeMDL(&mdl);
	    mdl.fname = src_fname;
	 #if USE_NEW_CONTAINER_MDL
	    *err = ScanMDL(&mdl,false,data,data_size,cdata,global_check_mode);
	 #else
	    *err = ScanMDL(&mdl,false,data,data_size,0,global_check_mode);
	 #endif
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawMDL(&mdl,dest_fname,true)
		: SaveTextMDL(&mdl,dest_fname,true);
	    mdl.fname = 0;
	    ResetMDL(&mdl);
	}
	return true;

     case FF_OBJFLOW:
     case FF_OBJFLOW_TXT:
	FreeContainerData(cdata);
	{
	    objflow_t objflow;
	    InitializeOBJFLOW(&objflow);
	    objflow.fname = src_fname;
	    *err = ScanOBJFLOW(&objflow,false,data,data_size);
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawOBJFLOW(&objflow,dest_fname,true)
		: SaveTextOBJFLOW(&objflow,dest_fname,true,minimize_level);
	    objflow.fname = 0;
	    ResetOBJFLOW(&objflow);
	}
	return true;

     case FF_PAT:
     case FF_PAT_TXT:
	{
	    pat_t pat;
	    InitializePAT(&pat);
	    pat.fname = src_fname;
	 #if USE_NEW_CONTAINER_PAT
	    *err = ScanPAT(&pat,false,data,data_size,cdata,global_check_mode);
	 #else
	    *err = ScanPAT(&pat,false,data,data_size,0,global_check_mode);
	 #endif
	    if (*err)
		return false;
	    *err = testmode
		? ERR_OK
		: binary_dest
		? SaveRawPAT(&pat,dest_fname,true)
		: SaveTextPAT(&pat,dest_fname,true);
	    pat.fname = 0;
	    ResetPAT(&pat);
	}
	return true;

     default:
	break;
    }

    *err = ERR_WRONG_FILE_TYPE;
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command binary/text		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_convert ( bool binary ) // cmd_binary() cmd_text()
{
    static ccp def_path = "\1P/\1N\1?T";
    CheckOptDest(def_path,false);
    char dest[PATH_MAX];
    enumError max_err = ERR_OK;

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
	{
	    ResetRawData(&raw);
	    return err;
	}

	file_format_t ff_dest = binary
		? GetBinFF(raw.fform) : GetTextFF(raw.fform);
	if (!ff_dest)
	{
	    ERROR0(ERR_WARNING,"File format %s not supported: %s\n",
			GetNameFF(raw.fform,0), param->arg );
	    if ( max_err < ERR_WARNING )
		 max_err = ERR_WARNING;
	    continue;
	}

	SubstDest( dest, sizeof(dest), param->arg, opt_dest,
			def_path, GetExtFF(0,ff_dest), false );

	if ( verbose >= 0 )
	{
	    fprintf(stdlog,"%sCREATE/%s %s:%s => %s:%s\n",
			verbose > 0 ? "\n" : "",
			binary ? "BINARY" : "TEXT",
			GetNameFF(raw.fform,0), param->arg,
			GetNameFF(ff_dest,0), dest );
	    fflush(stdlog);
	}

	SetupContainerRawData(&raw);
	ConvertHelper(	raw.data, raw.data_size,
			LinkContainerData(&raw.container),
			param->arg, dest, binary, &err );
	if (err)
	{
	    ERROR0(err,0);
	    if ( max_err < err )
		 max_err = err;
	}
    }

    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command cat			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_cat()
{
    stdlog = stderr;
    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_extract_t eszs;
	enumError err = ExtractSZS(&eszs,true,param->arg,0,opt_ignore>0);
	PRINT("stat=%u, data=%p, found=%d,%d, path=%s\n",
		err, eszs.data, !eszs.subfile_found, eszs.szs_found, eszs.subpath );

	if (eszs.data)
	{
	    if ( verbose > 0 || testmode )
	    {
		fprintf(stdlog,"%s%sCAT %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(eszs.fform_file,eszs.fform_arch),
			eszs.fname );
		fflush(stdlog);
	    }

	    if (!testmode)
	    {
		if (opt_decode)
		{
		    ConvertHelper( eszs.data, eszs.data_size,
					LinkContainerESZS(&eszs),
					eszs.fname, "-", false, &err );
		}
		else
		{
		    size_t wstat = fwrite(eszs.data,1,eszs.data_size,stdout);
		    if ( wstat != eszs.data_size )
			err = ERROR1(ERR_WRITE_FAILED,"Write to stdout failed\n");
		}
	    }
	}
	else if ( !err && !opt_ignore )
	    err = PrintErrorExtractSZS(&eszs,param->arg);
	fflush(stdout);

	if ( max_err < err )
	     max_err = err;
	ResetExtractSZS(&eszs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command <FFORM>			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_FFORM ( file_format_t fform, ccp filename )
{
    stdlog = stderr;
    char pathbuf[PATH_MAX];
    enable_kcl_drop_auto++;

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	ccp path = PathCatPP(pathbuf,sizeof(pathbuf),param->arg,filename);

	szs_extract_t eszs;
	enumError err = ExtractSZS(&eszs,true,path,0,opt_ignore>0);
	PRINT("stat=%u, data=%p, found=%d,%d, path=%s\n",
		err, eszs.data, !eszs.subfile_found, eszs.szs_found, eszs.subpath );

// [[analyse-magic]]
	if ( eszs.data && GetByMagicFF(eszs.data,eszs.data_size,eszs.data_size) == fform )
	{
	    if ( verbose > 0 || testmode )
		fprintf(stdlog,"%s%sCAT %s:%s\n",
			verbose > 0 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(eszs.fform_file,eszs.fform_arch),
			eszs.fname );

	    if (!testmode)
		ConvertHelper( eszs.data, eszs.data_size,
				LinkContainerESZS(&eszs),
				eszs.fname, "-", false, &err );
	}
	else if ( !err && !opt_ignore )
	    err = PrintErrorExtractSZS(&eszs,param->arg);

	if ( max_err < err )
	     max_err = err;
	ResetExtractSZS(&eszs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command info			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_info()
{
    stdlog = stderr;

    char pathbuf[PATH_MAX];
    ccp search_tab[] = { "info.txt", "credits.txt", 0 };

    szs_extract_t eszs;
    InitializeExtractSZS(&eszs);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	ccp * search;
	for ( search = search_tab; ; search++ )
	{
	    if (!*search)
	    {
		if (!opt_ignore)
		    printf("\n* NO INFO FOUND: %s\n\n",param->arg);
		max_err = ERR_WARNING;
		break;
	    }

	    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),param->arg,*search);
	    ResetExtractSZS(&eszs);
	    enumError err = ExtractSZS(&eszs,false,path,0,true);
	    if ( !err && eszs.data )
	    {
		if (print_header)
		    printf("\n* %s\n\n",eszs.fname);
		fwrite(eszs.data,1,eszs.data_size,stdout);
		if ( eszs.data_size && eszs.data[eszs.data_size-1] != '\n' )
		    putchar('\n');
		if (print_header)
		    putchar('\n');
		break;
	    }
	}
    }
    ResetExtractSZS(&eszs);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command ghost			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_ghost()
{
    static ccp def_path = "\1P/\1F\1?T";
    CheckOptDest(def_path,false);
    char dest[PATH_MAX];

    raw_data_t raw;
    InitializeRawData(&raw);

    rkg_info_t ri;
    InitializeRKGInfo(&ri);
    enumError max_err = ERR_OK;

    File_t fo;
    InitializeFile(&fo);

    PrintScript_t ps;
    InitializePrintScript(&ps);
    ps.fo = &fo;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,0);
	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if (!fo.f)
	    SubstDest(dest,sizeof(dest),param->arg,opt_dest,def_path,
			GetExtFF(script_fform,0),false);

	if ( verbose >= 0 )
	{
	    fprintf(stdlog,"%sANALYZE %s:%s => %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname,
			GetNameFF(script_fform,0), dest );
	    fflush(stdlog);
	}

	err = ScanRawDataGHOST(&ri,false,&raw);
	if ( err >= ERR_WARNING )
	{
	    if ( max_err < err )
		 max_err = err;
	    continue;
	}

	if (!fo.f)
	{
	    enumError err = CreateFile(&fo,false,dest,FM_STDIO|FM_OVERWRITE);
	    if (err)
	    {
		max_err = err;
		break;
	    }
	    PrintScriptHeader(&ps);
	}

	PrintScriptVars(&ps,3,
		"compressed=%d\n"
		"size=%u\n"
		"score=%u\n"
		"time_button=%u\n"
		"time_direction=%u\n"
		"time_trick=%u\n"
		,ri.is_compressed
		,ri.data_size
		,ri.score
		,ri.time_but
		,ri.time_dir
		,ri.time_trick
		);
 #if 0
	printf("score = %u, compr = %d, size = %u\n",
		ri.score, ri.is_compressed, ri.data_size );
	printf("but = %u,%u, dir = %u,%u, trick=%u,%u\n",
		ri.n_but, ri.time_but,
		ri.n_dir, ri.time_dir,
		ri.n_trick, ri.time_trick );
 #endif

	if (!script_array)
	{
	    PrintScriptFooter(&ps);
	    ResetFile(&fo,0);
	}
    }

    PrintScriptFooter(&ps);
    ResetPrintScript(&ps);
    ResetFile(&fo,0);
    ResetRKGInfo(&ri);
    ResetRawData(&raw);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command decompress		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_yazdump()
{
    CheckOptDest("-",false);

    enumError max_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,param->arg,false,opt_ignore>0,false);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS
		&& ( szs.fform_file == FF_YAZ0 || szs.fform_file == FF_YAZ1 ))
	{
	    char dest[PATH_MAX];
	    SubstDest(dest,sizeof(dest),param->arg,opt_dest,".txt",".txt",false);

	    File_t F;
	    CreateFileOpt(&F,true,dest,testmode,0);
	    if (F.f)
	    {
		if ( verbose >= 0 || testmode )
		    fprintf(F.f,"%sYAZ DUMP of %s:%s\n",
				verbose > 0 ? "\n" : "",
				GetNameFF_SZS(&szs), param->arg );
		err = DecompressSZS(&szs,true,F.f);
	    }
	}

	if ( max_err < err )
	     max_err = err;
	ResetSZS(&szs);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command vehicle			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef HAVE_WIIMM_EXT

 enumError cmd_vehicle()
 {
    ERROR0(ERR_NOT_IMPLEMENTED,
	"Command VEHICLE not implemented in this version!\n");
    exit(ERR_NOT_IMPLEMENTED);
 }

#endif

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

      RegisterOptionByName(&InfoUI_wszst,opt_stat,1,is_env);

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
	case GO_TOUCH:		break;
	case GO_AUTO:		break;
	case GO_SET_FLAGS:	err += ScanOptSetFlags(optarg); break;
	case GO_SET_SCALE:	err += ScanOptSetScale(optarg); break;
	case GO_SET_ROT:	err += ScanOptSetRot(optarg); break;
	case GO_SET_X:		err += ScanOptSet(0,optarg); break;
	case GO_SET_Y:		err += ScanOptSet(1,optarg); break;
	case GO_SET_Z:		err += ScanOptSet(2,optarg); break;
	case GO_XCENTER:	break;
	case GO_YCENTER:	break;
	case GO_ZCENTER:	break;
	case GO_CENTER:		break;
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

	case GO_MDL:		err += ScanOptMdl(optarg); break;
	case GO_MINIMAP:	break;
	case GO_PAT:		err += ScanOptPat(optarg); break;
	case GO_PATCH_FILE:	err += ScanOptPatchFile(optarg); break;

	case GO_KMG_LIMIT:	err += ScanOptKmgLimit(optarg); break;
	case GO_KMG_COPY:	err += ScanOptKmgCopy(optarg); break;

	case GO_LT_CLEAR:	opt_lt_clear = true; break;
	case GO_LT_ONLINE:	err += ScanOptLtOnline(optarg); break;
	case GO_LT_N_PLAYERS:	err += ScanOptLtNPlayers(optarg); break;
	case GO_LT_COND_BIT:	err += ScanOptLtCondBit(optarg); break;
	case GO_LT_GAME_MODE:	err += ScanOptLtGameMode(optarg); break;
	case GO_LT_ENGINE:	err += ScanOptLtEngine(optarg); break;
	case GO_LT_RANDOM:	err += ScanOptLtRandom(optarg); break;
	case GO_LEX_PURGE:	opt_lex_purge = true; break;

	case GO_UTF_8:		use_utf8 = true; break;
	case GO_NO_UTF_8:	use_utf8 = false; break;

	case GO_TEST:		testmode++; break;
	case GO_FORCE:		force_count++; break;
	case GO_REPAIR_MAGICS:	err += ScanOptRepairMagic(optarg); break;
	case GO_TINY:		err += ScanOptTiny(optarg); break;

	case GO_ANALYZE:	analyze_fname = optarg; break;
	case GO_ANALYZE_MODE:	err += ScanOptAnalyzeMode(optarg); break;

 #if OPT_OLD_NEW
	case GO_OLD:		opt_new = opt_new>0 ? -1 : opt_new-1; break;
	case GO_STD:		opt_new = 0; break;
	case GO_NEW:		opt_new = opt_new<0 ? +1 : opt_new+1; break;
 #endif
	case GO_EXTRACT:	opt_extract = optarg; break;

	case GO_ESC:		err += ScanEscapeChar(optarg) < 0; break;
	case GO_SOURCE:		SetSource(optarg); break;
	case GO_DEST:		SetDest(optarg,false); break;
	case GO_DEST2:		SetDest(optarg,true); break;
	case GO_OVERWRITE:	opt_overwrite = true; break;
	case GO_NUMBER:		opt_number = true; break;
	case GO_REMOVE_SRC:	opt_remove_src = true; break;
	case GO_REMOVE_DEST:	opt_remove_dest = true; break;
	case GO_UPDATE:		opt_update = true; break;
	case GO_PRESERVE:	opt_preserve = true; break;
	case GO_IGNORE:		opt_ignore++; break;
	case GO_IGNORE_SETUP:	opt_ignore_setup = true; break;
	case GO_PURGE:		opt_purge = true; break;

	case GO_YAZ0:		SetCompressionFF(0,FF_YAZ0); break;
	case GO_YAZ1:		SetCompressionFF(0,FF_YAZ1); break;
	case GO_XYZ:		SetCompressionFF(0,FF_XYZ); break;
	case GO_BZ:		SetCompressionFF(0,FF_BZ); break;

	case GO_U8:		SetCompressionFF(FF_U8,0); break;
	case GO_SZS:		SetCompressionFF(FF_U8,FF_YAZ0); break;
	case GO_WU8:		SetCompressionFF(FF_WU8,0); break;
	case GO_XWU8:		SetCompressionFF(FF_WU8,FF_XYZ); break;
	case GO_WBZ:		SetCompressionFF(FF_WU8,FF_BZ); break;
	//case GO_ARC:		SetCompressionFF(FF_RARC,0); break;
	case GO_BRRES:		SetCompressionFF(FF_BRRES,0); break;
	case GO_BREFF:		SetCompressionFF(FF_BREFF,0); break;
	case GO_BREFT:		SetCompressionFF(FF_BREFT,0); break;
	case GO_PACK:		SetCompressionFF(FF_PACK,0); break;

	case GO_JSON:		script_fform = FF_JSON; break;
	case GO_SH:		script_fform = FF_SH; break;
	case GO_BASH:		script_fform = FF_BASH; break;
	case GO_PHP:		script_fform = FF_PHP; break;
	case GO_MAKEDOC:	script_fform = FF_MAKEDOC; break;
	case GO_VAR:		script_varname = optarg; break;
	case GO_ARRAY:		script_array++; break;
	case GO_AVAR:		script_array++; script_varname = optarg; break;

	case GO_PT_DIR:		err += ScanOptPtDir(optarg); break;
	case GO_LINKS:		opt_links = true; break;
	case GO_RM_AIPARAM:	opt_rm_aiparam = true; break;
	case GO_ALIGN_U8:	err += ScanOptAlignU8(optarg); break;
	case GO_ALIGN_PACK:	err += ScanOptAlignPACK(optarg); break;
	case GO_ALIGN_BRRES:	err += ScanOptAlignBRRES(optarg); break;
	case GO_ALIGN_BREFF:	err += ScanOptAlignBREFF(optarg); break;
	case GO_ALIGN_BREFT:	err += ScanOptAlignBREFT(optarg); break;
	case GO_ALIGN:		err += ScanOptAlign(optarg); break;
	case GO_TRANSFORM:	err += ScanOptTransform(optarg); break;
	case GO_STRIP:		opt_strip = true; break;

	case GO_NO_COMPRESS:	opt_compr_mode = -1; break;
	case GO_COMPRESS:	err += ScanOptCompr(optarg); break;
	case GO_FAST:		opt_fast = true; err += ScanOptCompr("fast"); break;
	case GO_NORM:		opt_norm = true; break;
	case GO_MAX_FILE_SIZE:	err += ScanOptMaxFileSize(optarg); break;
	case GO_TRACKS:		err += ScanOptTracks(optarg); break;
	case GO_ARENAS:		err += ScanOptArenas(optarg); break;
	case GO_LOAD_BMG:	err += ScanOptLoadBMG(optarg); break;
	case GO_PATCH_BMG:	err += ScanOptPatchMessage(optarg); break;
	case GO_MACRO_BMG:	err += ScanOptMacroBMG(optarg); break;
	case GO_FILTER_BMG:	filter_bmg = optarg; break;
	case GO_AUTOADD_PATH:	DefineAutoAddPath(optarg); break;

	case GO_ENCODE_ALL:	opt_encode_all = true; break;
	case GO_ENCODE_IMG:	opt_encode_img = true; break;
	case GO_NO_ENCODE:	opt_no_encode = true; break;
	case GO_NO_RECURSE:	opt_no_recurse = true; break;
	case GO_AUTO_ADD:	opt_auto_add = true; break;
	case GO_NO_ECHO:	opt_no_echo = true; break;
	case GO_NO_CHECK:	opt_no_check = true; break;
	case GO_BASEDIR:	opt_basedir = optarg; break;
	case GO_RECURSE:	err += ScanOptRecurse(optarg); break;
	case GO_EXT:		opt_ext++; break;
	case GO_DECODE:		opt_decode = true; break;
	case GO_CMPR_DEFAULT:	err += ScanOptCmprDefault(optarg); break;
	case GO_N_MIPMAPS:	err += ScanOptNMipmaps(optarg); break;
	case GO_MAX_MIPMAPS:	err += ScanOptMaxMipmaps(optarg); break;
	case GO_MIPMAP_SIZE:	err += ScanOptMipmapSize(optarg); break;
	case GO_MIPMAPS:	opt_mipmaps = +1; break;
	case GO_NO_MIPMAPS:	opt_mipmaps = -1; break;
	case GO_FAST_MIPMAPS:	fast_resize_enabled = true; break;
	case GO_CUT:		opt_cut = true; break;
	case GO_RAW:		opt_raw = true; break;

	case GO_ROUND:		opt_round = true; break;
	case GO_LONG:		long_count++; break;
//	case GO_FULL:		full_count++; break;
	case GO_EXPORT:		export_count++; break;
	case GO_SORT:		err += ScanOptSort(optarg); break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_PIPE:		pipe_count++; break;
	case GO_DELTA:		delta_count++; break;
	case GO_DIFF:		diff_count++; break;
	case GO_NO_PARAM:	print_param = false; break;
	case GO_EPSILON:	err += ScanOptEpsilon(optarg); break;

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
	case GO_CACHE:		opt_cache = optarg; break;
	case GO_CNAME:		opt_cname = optarg; break;
	case GO_ID:		opt_id = true; break;
	case GO_BASE64:		opt_base64 = true; err += ScanOptCoding64(optarg); break;
	case GO_DB64:		opt_db64 = true; err += ScanOptCoding64(optarg); break;
	case GO_CODING:		err += ScanOptCoding64(optarg); break;
	case GO_VERIFY:		opt_verify = true; break;
	case GO_SECTIONS:	print_sections++; break;
	case GO_ALL:		set_all(0); break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

    if ( opt_compr_mode == -9 )
    {
// [[?]]
	if (opt_compr)
	    opt_colorize = 1;
	list_compressions_exit();
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wszst,TRACE_FILE,11);
 #endif
    CloseTransformation();
    NormalizeOptions( verbose > 3 && !is_env ? 3 : 0 );
    SetupBMG(filter_bmg);
    SetupKCL();
    SetupKMP();
    SetupMDL();

    // true until final version of 'post-patch-mkw.sh'
    SetupSZSCache(opt_cache,true);

    if (!err)
    {
	enumError err = SetupPatchingListBMG();
	if ( err > ERR_WARNING )
	    return err;
    }

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
	enumError err = VerifySpecificOptions(&InfoUI_wszst,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wszst);

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
	case CMD_HELP:		PrintHelp(&InfoUI_wszst,stdout,0,"HELP",0,URI_HOME,
					first_param ? first_param->arg : 0 ); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0);
				break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_BRSUB:		err = cmd_brsub(); break;

	case CMD_SYMBOLS:	err = DumpSymbols(SetupParserVars()); break;
	case CMD_FUNCTIONS:	SetupReferenceKCL(0);
				err = ListParserFunctions(); break;
	case CMD_CALCULATE:	err = ParserCalc(SetupParserVars()); break;
	case CMD_MATRIX:	err = cmd_matrix(); break;
	case CMD_FLOAT:		err = cmd_float(); break;
	case CMD_VR_CALC:	err = cmd_vr_calc(); break;
	case CMD_VR_RACE:	err = cmd_vr_race(); break;
	case CMD_AUTOADD:	err = cmd_autoadd(); break;
	case CMD_TRACKS:	err = cmd_tracks(); break;
	case CMD_SCANCACHE:	err = cmd_scancache(); break;
	case CMD_EXPORT:	err = cmd_export(); break;
	case CMD_CODE:		err = cmd_code(); break;
	case CMD_RECODE:	err = cmd_recode(); break;
	case CMD_SUBFILE:	err = cmd_subfile(); break;

	case CMD_LIST:		err = cmd_list(0); break;
	case CMD_LIST_L:	err = cmd_list(1); break;
	case CMD_LIST_LL:	err = cmd_list(2); break;
	case CMD_LIST_LLL:	err = cmd_list(3); break;
	case CMD_LIST_A:	set_all(1); err = cmd_list(1); break;
	case CMD_LIST_LA:	set_all(1); err = cmd_list(2); break;

	case CMD_NAME_REF:	err = cmd_name_ref(); break;

	case CMD_ILIST:		err = cmd_ilist(0); break;
	case CMD_ILIST_L:	err = cmd_ilist(1); break;
	case CMD_ILIST_LL:	err = cmd_ilist(2); break;
	case CMD_ILIST_A:	set_all(0); err = cmd_ilist(0); break;
	case CMD_ILIST_LA:	set_all(0); err = cmd_ilist(1); break;

	case CMD_DUMP:		err = cmd_dump(); break;
	case CMD_MEMORY:	err = cmd_memory(); break;
	case CMD_MEMORY_A:	set_all(1); err = cmd_memory(); break;
	case CMD_SHA1:		err = cmd_sha1(); break;
	case CMD_ANALYZE:	err = cmd_analyze(); break;
	case CMD_DISTRIBUTION:	err = cmd_distribution(); break;
	case CMD_DIFF:		err = cmd_diff(); break;

	case CMD_CHECK:		err = cmd_check(); break;
	case CMD_SLOTS:		err = cmd_slots(); break;
	case CMD_STGI:		err = cmd_stgi(); break;
	case CMD_ISARENA:	err = cmd_isarena(); break;
	case CMD_PATCH:		err = cmd_patch(); break;
	case CMD_COPY:		err = cmd_copy(); break;
	case CMD_DUPLICATE:	err = cmd_duplicate(); break;
	case CMD_NORMALIZE:	err = cmd_normalize(); break;
	case CMD_MINIMAP:	err = cmd_minimap(); break;
	case CMD_COMPRESS:	err = cmd_compress(); break;
	case CMD_DECOMPRESS:	err = cmd_decompress(); break;
	case CMD_ENCODE:	err = cmd_create(false); break;
	case CMD_CREATE:	err = cmd_create(true); break;
	case CMD_UPDATE:	err = cmd_update(); break;
	case CMD_EXTRACT:
	case CMD_XDECODE:
	case CMD_XEXPORT:
	case CMD_XALL:
	case CMD_XCOMMON:	err = cmd_extract(cmd_ct->id); break;

	case CMD_BINARY:	err = cmd_convert(true); break;
	case CMD_TEXT:		err = cmd_convert(false); break;
	case CMD_CAT:		err = cmd_cat(); break;
	case CMD_BMG:		err = cmd_bmg_cat(true); break;
	case CMD_KCL:		err = cmd_FFORM(FF_KCL,"course.kcl"); break;
	case CMD_KMP:		err = cmd_FFORM(FF_KMP,"course.kmp"); break;
	case CMD_LEX:		err = cmd_FFORM(FF_LEX,"course.lex"); break;
	case CMD_INFO:		err = cmd_info(); break;
	case CMD_GHOST:		err = cmd_ghost(); break;
	case CMD_YAZDUMP:	err = cmd_yazdump(); break;

	case CMD_VEHICLE:	err = cmd_vehicle(); break;

	// no default case defined
	//	=> compiler checks the existence of all enum values

	case CMD__NONE:
	case CMD__N:
	    help_exit(false);
    }

    SaveSZSCache();
    return PrintErrorStat(err,verbose,cmd_ct->name1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			main_wszst(), main()		///////////////
///////////////////////////////////////////////////////////////////////////////

int main_wszst ( int argc, char ** argv )
{
    print_title_func = print_title;
    SetupLib(argc,argv,WSZST_SHORT,VERSION,TITLE);
    SetupExtendedSZS();

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WSZST_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions("WSZST_OPT",CheckOptions);
    if (err)
	hint_exit(err);

    err = CheckOptions(argc,argv,false);
    if (err)
	hint_exit(err);

    err = CheckCommand(argc,argv);
    CloseAnalyzeFile();
    DUMP_TRACE_ALLOC(TRACE_FILE);

    if (SIGINT_level)
	err = ERROR0(ERR_INTERRUPT,"Program interrupted by user.");
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			tool wrapper			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef int (*main_func) ( int argc, char ** argv );

extern int main_wszst ( int argc, char ** argv );
extern int main_wbmgt ( int argc, char ** argv );
extern int main_wctct ( int argc, char ** argv );
extern int main_wimgt ( int argc, char ** argv );
extern int main_wkclt ( int argc, char ** argv );
extern int main_wkmpt ( int argc, char ** argv );
extern int main_wlect ( int argc, char ** argv );
extern int main_wmdlt ( int argc, char ** argv );
extern int main_wpatt ( int argc, char ** argv );
extern int main_wstrt ( int argc, char ** argv );
extern int main_wrapper ( int argc, char ** argv );
extern int main_getopt ( int argc, char ** argv );

typedef struct wrapper_t
{
    uint	hide;	// 0:off, 1:hide, 2:is wrapper
    main_func	func;
    ccp		name;
    ccp		info;
}
wrapper_t;

static const wrapper_t wrapper_tab[] =
{
    { 0, main_wszst, WSZST_SHORT, WSZST_LONG },
    { 0, main_wbmgt, WBMGT_SHORT, WBMGT_LONG },
    { 0, main_wctct, WCTCT_SHORT, WCTCT_LONG },
    { 0, main_wimgt, WIMGT_SHORT, WIMGT_LONG },
    { 0, main_wkclt, WKCLT_SHORT, WKCLT_LONG },
    { 0, main_wkmpt, WKMPT_SHORT, WKMPT_LONG },
    { 0, main_wlect, WLECT_SHORT, WLECT_LONG },
    { 0, main_wmdlt, WMDLT_SHORT, WMDLT_LONG },
    { 0, main_wpatt, WPATT_SHORT, WPATT_LONG },
    { 0, main_wstrt, WSTRT_SHORT, WSTRT_LONG },
    { 2, main_getopt, "getopt", 0 },
    { 2, main_wrapper, "wrapper", 0 },
    {0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

static const wrapper_t * FindWrapper ( int argc, char ** argv )
{
    if ( argc > 0 && argv[0] )
    {
	ccp name = strrchr(argv[0],'/');
	name = name ? name+1 : argv[0];
	if ( *name == 'w' || *name == 'W' )
	    name++;

	const wrapper_t *w;
	for ( w = wrapper_tab; w->func; w++ )
	    if ( !strncasecmp(w->name+1,name,3) )
		return w;
    }

    return wrapper_tab;
}

///////////////////////////////////////////////////////////////////////////////

int main_wrapper ( int argc, char ** argv )
{
    SetupLib(argc,argv,TOOLSET_SHORT,VERSION,TITLE);

    enum
    {
	C_CREATE	= 0x001,
	C_OVERWRITE	= 0x002,

	C_HARDLINKS	= 0x010,
	C_SOFTLINKS	= 0x020,
	C_CYGWIN	= 0x040,
	C_SHELL		= 0x080,
	C_BASH		= 0x100,

	C_QUIET		= 0x200,
	C_VERBOSE	= 0x400,

	C_M_DEST	= C_HARDLINKS|C_SOFTLINKS|C_CYGWIN|C_SHELL,
    };

    static const KeywordTab_t keytab[] =
    {
	{ C_CREATE,		"CREATE",	0, 3 },
	{ C_CREATE|C_OVERWRITE,	"OVERWRITE",	0, 4 },

	{ C_HARDLINKS,		"HARDLINKS",	0, 4 },
    #ifdef __CYGWIN__
	{ C_HARDLINKS,		"BESTLINKS",	0, 4 },
    #endif
	{ C_SOFTLINKS,		"SOFTLINKS",	0, 4 },
    #ifndef __CYGWIN__
	{ C_SOFTLINKS,		"BESTLINKS",	0, 4 },
    #endif
	{ C_CYGWIN,		"CYGWIN",	0, 3 },
	{ C_SHELL,		"SHELL",	0, 2 },
	{ C_SHELL|C_BASH,	"BASH",		0, 4 },

	{ C_QUIET,		"QUIET",	0, 1 },
	{ C_VERBOSE,		"VERBOSE",	0, 1 },

	{0,0,0,0}
    };

    uint i, param = 0;
    for ( i = 1; i < argc; i++ )
    {
	const KeywordTab_t *key = ScanKeyword(0,argv[i],keytab);
	if ( !key || strlen(argv[i]) < key->opt )
	    goto abort;
	param |= key->id;
    }

    const uint quiet	= param & C_QUIET;
    const uint verbose	= param & C_VERBOSE;
    const uint overwrite= param & C_OVERWRITE;
    const uint bash	= param & C_BASH;
    const uint desttype	= param & C_M_DEST;

    char mypath[PATH_MAX], dest[PATH_MAX+20];
    GetProgramPath(mypath,sizeof(mypath),true,argv[0]);
    if (verbose)
	printf("My Path: %s\n",mypath);

    if ( param & C_CREATE
	&& Count1Bits(&desttype,sizeof(desttype)) == 1
	&& *mypath )
    {
	memcpy(dest,mypath,sizeof(dest));
	char *fname = strrchr(dest,'/');
	fname = fname ? fname+1 : dest;
	if (!*fname)
	    goto abort;

	u16 cygwin_link[1000];
	uint cygwin_len = 0;
	if ( desttype == C_CYGWIN )
	{
	    ccp src = fname;
	    while ( cygwin_len < sizeof(cygwin_link)/sizeof(*cygwin_link) )
	    {
		u32 code = ScanUTF8AnsiChar(&src);
		if (!code)
		    break;
		write_le16(cygwin_link+cygwin_len++,code);
	    }
	}

	ccp srcname = mypath + (fname-dest);
	char srcname_buf[PATH_MAX];
	if ( desttype == C_SHELL )
	{
	    ccp src;
	    for ( src = srcname; *src; src++ )
	    {
		const int ch = *src;
		if ( !isalnum(ch) && ch != '_' && ( ch != '-' || src == srcname ) )
		{
		    uint len;
		    PrintEscapedString( srcname_buf+1,
				sizeof(srcname_buf)-2, srcname, -1, CHMD__ALL, '"', &len );
		    if (bash)
			srcname = srcname_buf+1;
		    else
		    {
			srcname_buf[0] = srcname_buf[len] = '"';
			srcname = srcname_buf;
		    }
		    break;
		}
	    }
	}

	FILE *f = 0;
	const wrapper_t *w;

	for ( w = wrapper_tab; w->func; w++ )
	{
	    if (w->hide)
		continue;

	    strcpy(fname,w->name);
	    if (!strcmp(mypath,dest))
		continue;

	    struct stat st;
	    if ( !overwrite && !stat(dest,&st) )
		continue;
	    unlink(dest);

	    switch(desttype)
	    {
	     case C_HARDLINKS:
		if (!quiet)
		    printf("CREATE HARDLINK %s\n",dest);
		link(mypath,dest);
		break;

	     case C_SOFTLINKS:
		if (!quiet)
		    printf("CREATE SOFTLINK %s\n",dest);
		symlink(srcname,dest);
		break;

	     case C_CYGWIN:
		if (!quiet)
		    printf("CREATE CYGWIN SOFTLINK %s\n",dest);
		f = fopen(dest,"w");
		if (f)
		{
		    fwrite("!<symlink>\xff\xfe",1,12,f);
		    fwrite(cygwin_link,2,cygwin_len,f);
		    fwrite("\0\0",1,2,f);
		    fclose(f);
		}
		break;

	     case C_SHELL:
		if (!quiet)
		    printf("CREATE %sSH SCRIPT %s\n", bash ? "BA" : "", dest);
		f = fopen(dest,"w");
		if (f)
		{
		    if (bash)
			fprintf(f,"#!/usr/bin/env bash\n"
				"\"$(dirname \"$BASH_SOURCE\")/%s\" %s \"$@\"\n",
				srcname, w->name );
		    else
			fprintf(f,"#!/bin/sh\n%s %s \"$@\"\n",srcname,w->name);
		    fclose(f);
		    chmod(dest,0755);
		}
		break;
		//"$(dirname "$BASH_SOURCE")"
	    }
	}
	return ERR_OK;
    }

 abort:;
    fprintf(stdout,
	"\n%s\n%s\n\n   Usage: %s TOOLNAME ...\n\n"
	"This is a wrapper for the following tools:\n\n",
	text_logo, TOOLSET_TITLE, ProgInfo.progname );

    const wrapper_t *w, *active = FindWrapper(argc,argv);
    DASSERT(active);
    for ( w = wrapper_tab; w->func; w++ )
	if ( w == active || !w->hide )
	    fprintf(stdout,"  %c %-5s : %s\n",
		w == active ? '*' : ' ', w->name, w->info );

    fprintf(stdout,
	"\nWrapper commands:\n\n"
	"   WRAPPER HELP\n"
	"   WRAPPER CREATE    [QUIET] HARDLINKS|SOFTLINKS|BESTLINKS|CYGWIN|SHELL|BASH\n"
	"   WRAPPER OVERWRITE [QUIET] HARDLINKS|SOFTLINKS|BESTLINKS|CYGWIN|SHELL|BASH\n"
	"\n"
	"   H[ELP]      : Print this help and exit.\n"
	"   CRE[ATE]    : Create links or scripts, but don't overwrite.\n"
	"   OVER[WRITE] : Create links or scripts and remove existing files before.\n"
	"   HARD[LINKS] : Create hard links to the main program.\n"
	"   SOFT[LINKS] : Create soft links to the main program.\n"
	"   BEST[LINKS] : For Cygwin same as HARDLINKS, for all other same as SOFTLINKS.\n"
	"   CYG[WIN]    : Create softlinks for Cygwin (plain files with special content).\n"
	"   SH[ELL]     : Create simple `sh´ scripts assuming the main program is in PATH.\n"
	"   BASH        : Create `bash´ scripts with run time path detection.\n"
	"   Q[UIET]     : Option to suppress creation messages.\n"
	"\n"
	"   See https://szs.wiimm.de/doc/wrapper for details.\n"
	" \n");

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
    //PRINT("ERR__N=%u\n",ERR__N);
    if ( argc > 1 )
    {
	const wrapper_t *w;
	for ( w = wrapper_tab; w->func; w++ )
	    if (!strcasecmp(w->name,argv[1]))
	    {
		argv[1] = argv[0];
		return w->func(argc-1,argv+1);
	    }
    }

    const wrapper_t *w = FindWrapper(argc,argv);
    DASSERT(w);
    return w->func(argc,argv);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


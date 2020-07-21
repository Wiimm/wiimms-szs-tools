
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>

#include "lib-szs.h"
#include "lib-rarc.h"
#include "lib-pack.h"
#include "lib-brres.h"
#include "lib-breff.h"
#include "lib-ctcode.h"
#include "lib-xbmg.h"
#include "lib-kcl.h"
#include "lib-kmp.h"
#include "lib-lecode.h"
#include "lib-mdl.h"
#include "lib-pat.h"
#include "lib-rkc.h"
#include "lib-image.h"
#include "lib-object.h"
#include "lib-checksum.h"
#include "crypt.h"

#include "setup.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			alter file table		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct alter_file_t
{
    SlotMode_t	rm_mode;
    SlotMode_t	add_mode;
    ccp		fname;
}
alter_file_t;

static const alter_file_t AlterFileTab[] =
{
	//---------------------------------------------------------------------
	// mode for remove	mode for add		file name
	//---------------------------------------------------------------------
	{ SLOTMD_RM_ICE,	SLOTMD_ADD_BICE,	"ice.brres" },
	{ SLOTMD_RM_SUNDS,	0,			"sunDS.brres" },
	{ SLOTMD_RM_SUNDS,	0,			"pylon01.brres" },
	{ SLOTMD_RM_SHYGUY,	SLOTMD_ADD_SHYGUY,	"HeyhoShipGBA.brres" },
	{ SLOTMD_RM_SHYGUY,	0,			"HeyhoBallGBA.brres" },
	{0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

static const alter_file_t * ShallRemoveFile ( ccp fname )
{

    if ( opt_slot & SLOTMD_JOB_RM_SZS && fname )
    {
	const alter_file_t *alter;
	for ( alter = AlterFileTab; alter->fname; alter++ )
	    if ( opt_slot & alter->rm_mode && !strcasecmp(fname,alter->fname) )
		return alter;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool AddSlotFiles ( szs_file_t *szs, struct szs_norm_t * norm )
{
    DASSERT(szs);
    bool dirty = false;
    if ( szs->fform_arch == FF_U8 || szs->fform_arch == FF_WU8 )
    {
	const alter_file_t *alter;
	for ( alter = AlterFileTab; alter->fname; alter++ )
	    if ( opt_slot & alter->add_mode
		    && AddMissingFileSZS(szs,alter->fname,FF_BRRES,norm,-1) )
	    {
		PATCH_ACTION_LOG("Add","SZS","%s\n",alter->fname);
		dirty = true;
	    }
    }
    return dirty;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    scan dir & create szs		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[scan_data_t]]

typedef struct scan_data_t
{
    szs_file_t	* szs;			// valid pointer to szs data structure
    SetupParam_t * setup_param;		// valid pointer to setup parameters
    char	path[PATH_MAX];		// the current path
    char	* path_rel;		// ptr to 'path': path relative to base directory
    char	* path_dir;		// ptr to 'path': current directory
    u32		n_directories;		// total number of directories
    u32		namepool_size_u8;	// total size of name pool (for U8)
    u32		depth;			// current depth
    u32		max_depth;		// max allowed depth
    u64		total_size;		// total aligned size of all files
    u32		align;			// data alignmet

} scan_data_t;

///////////////////////////////////////////////////////////////////////////////

static bool fname_allowed ( scan_data_t *sd, ccp name, char * path_dir )
{
    // ignore "." and ".." always
    if ( *name == '.' && ( !name[1] || name[1] == '.' && !name[2] ))
	return false;

    if ( opt_rm_aiparam && !strcasecmp(name,"aiparam") )
    {
	PATCH_ACTION_LOG("Remove","SZS","%s\n","AIParam");
	sd->szs->aiparam_removed = true;
	return false;
    }

    if (ShallRemoveFile(name))
    {
	PATCH_ACTION_LOG("Remove","SZS","%s\n",name);
	return false;
    }

    char *path_end = sd->path + sizeof(sd->path) - 1;
    sd->path_dir = StringCopyE(path_dir,path_end,name);
    const SetupParam_t * sp = sd->setup_param;
    DASSERT(sp);
    if (   !FindStringField(&sp->include_name,sd->path_rel)
	&& !MatchStringField(&sp->include_pattern,sd->path_rel) )
    {
	// file is not in include list
	if ( *name == '.'
		|| FindStringField(&sp->exclude_name,sd->path_rel)
		|| MatchStringField(&sp->exclude_pattern,sd->path_rel) )
	{
	    noPRINT("EXCLUDED: %s\n",sd->path_rel);
	    return false;
	}
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

static u32 scan_data ( scan_data_t * sd )
{
    ASSERT(sd);
    ASSERT(sd->szs);
    szs_file_t *szs = sd->szs;
    const u16 dir_id = sd->n_directories++;

    char * path_end = sd->path + sizeof(sd->path) - 1;
    DASSERT( sd->path     <= sd->path_rel );
    DASSERT( sd->path_rel <= sd->path_dir );
    DASSERT( sd->path_dir <= path_end );

    PRINT("scan_data(%.*s|%.*s|%s) depth=%d, links=%d\n",
		(int)(sd->path_rel - sd->path), sd->path,
		(int)(sd->path_dir - sd->path_rel), sd->path_rel,
		sd->path_dir, sd->depth, szs->links );

    // save current path_dir
    char * path_dir = sd->path_dir;

    u32 count = 0;
    DIR * dir = opendir(sd->path);
    if (dir)
    {
	for(;;)
	{
	    struct dirent * dent = readdir(dir);
	    if (!dent)
		break;

	    ccp name = dent->d_name;
	 #ifdef TEST // test it!
	    if (!fname_allowed(sd,name,path_dir))
		continue;
	 #else
	    // ignore "." and ".." always
	    if ( *name == '.' && ( !name[1] || name[1] == '.' && !name[2] ))
		continue;

	    if ( opt_rm_aiparam && !strcasecmp(name,"aiparam") )
	    {
		PATCH_ACTION_LOG("Remove","SZS","%s\n","AIParam");
		szs->aiparam_removed = true;
		continue;
	    }

	    if (ShallRemoveFile(name))
	    {
		PATCH_ACTION_LOG("Remove","SZS","%s\n",name);
		continue;
	    }

	    sd->path_dir = StringCopyE(path_dir,path_end,name);
	    const SetupParam_t * sp = sd->setup_param;
	    DASSERT(sp);
	    if (   !FindStringField(&sp->include_name,sd->path_rel)
		&& !MatchStringField(&sp->include_pattern,sd->path_rel) )
	    {
		// file is not in include list
		if ( *name == '.'
			|| FindStringField(&sp->exclude_name,sd->path_rel)
			|| MatchStringField(&sp->exclude_pattern,sd->path_rel) )
		{
		    noPRINT("EXCLUDED: %s\n",sd->path_rel);
		    continue;
		}
	    }
	 #endif

	    struct stat st;
	    if (stat(sd->path,&st))
		continue;

	    if ( S_ISDIR(st.st_mode) && sd->depth < sd->max_depth )
	    {
		count++;
		szs_subfile_t * file = AppendSubfileSZS(szs,0,0);
		DASSERT(file);
		file->dir_id = dir_id;
		const uint nlen = strlen(name);
		sd->namepool_size_u8 += nlen + 1;

		file->is_dir = true;
		*sd->path_dir++ = '/';
		*sd->path_dir = 0;
		file->path = STRDUP(sd->path_rel);
		noTRACE("DIR:  %s\n",path_dir);
		file->offset = sd->depth++;

		// pointer 'file' becomes invalid if realloc() called => store index
		const int idx = file - szs->subfile.list;
		const u32 sub_count = scan_data(sd);
		szs->subfile.list[idx].size = sub_count;
		count += sub_count;
		sd->depth--;
	    }
	    else if (S_ISREG(st.st_mode))
	    {
		count++;
		szs_subfile_t * file = AppendSubfileSZS(szs,0,0);
		DASSERT(file);
		file->dir_id = dir_id;
		const uint nlen = strlen(name);
		sd->namepool_size_u8 += nlen + 1;

		szs_subfile_t *lptr = szs->links
				? FindLinkSZS(szs,st.st_dev,st.st_ino,file) : 0;
		if (lptr)
		{
		    if (!lptr->link_index)
			lptr->link_index = ++szs->subfile.link_count;
		    file->link_index = lptr->link_index;
		    PRINT("LINK[%d]: %s  ->  %s\n",
				lptr->link_index, lptr->path, sd->path_rel );
		}

		file->path	= STRDUP(sd->path_rel);
		noTRACE("FILE: %s\n",path_dir);
		file->is_dir	= false;
		file->size	= st.st_size;
		file->device	= st.st_dev;
		file->inode	= st.st_ino;
		if (!lptr)
		{
		    MaxFileAttrib(&szs->fatt,0,&st);
		    sd->total_size += ALIGN32(file->size,sd->align);
		}
	    }
	    // else ignore all other files
	}
	closedir(dir);
    }

    // restore path_dir
    sd->path_dir = path_dir;
    return count;
}

///////////////////////////////////////////////////////////////////////////////

static u32 scan_sdir ( scan_data_t *sd, SubDir_t *sdir )
{
    ASSERT(sd);
    ASSERT(sd->szs);
    ASSERT(sdir);
    const u16 dir_id = sd->n_directories++;

    char * path_dir = sd->path_dir;
    u32 count = 0;

    uint i;
    for ( i = 0; i < sdir->file.used; i++ )
    {
	SubFile_t *sf = sdir->file.list[i];
	ccp name = sf->fname;
	if (!fname_allowed(sd,name,path_dir))
	    continue;

	count++;
	szs_subfile_t * file = AppendSubfileSZS(sd->szs,0,0);
	DASSERT(file);
	file->dir_id = dir_id;
	const uint nlen = strlen(name);
	sd->namepool_size_u8 += nlen + 1;

	file->path	= STRDUP(sd->path_rel);
	noTRACE("FILE: %s\n",path_dir);
	file->is_dir	= false;
	file->size	= sf->size;
	file->data	= sf->data;
	sd->total_size	+= ALIGN32(file->size,sd->align);
    }

    for ( i = 0; i < sdir->dir.used; i++ )
    {
	SubDir_t *sub = sdir->dir.list[i];
	ccp name = sub->dname;
	if (!fname_allowed(sd,name,path_dir))
	    continue;

	count++;
	szs_subfile_t * file	= AppendSubfileSZS(sd->szs,0,0);
	DASSERT(file);
	file->dir_id		= dir_id;
	const uint nlen		= strlen(name);
	sd->namepool_size_u8	+= nlen + 1;

	file->is_dir = true;
	*sd->path_dir++		= '/';
	*sd->path_dir		= 0;
	file->path		= STRDUP(sd->path_rel);
	noTRACE("DIR:  %s\n",path_dir);
	file->offset		= sd->depth++;

	// pointer 'file' becomes invalid if realloc() called => store index
	const int idx		= file - sd->szs->subfile.list;
	const u32 sub_count	= scan_sdir(sd,sub);
	sd->szs->subfile.list[idx].size = sub_count;
	count			+= sub_count;
	sd->depth--;
    }

    // restore path_dir
    sd->path_dir = path_dir;
    return count;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateSZS
(
    szs_file_t		*szs,		// valid szs
    ccp			dest_fname,	// NULL or planned destination file name
    ccp			source_dir,	// path to base directory
    SubDir_t		*sdir,		// not NULL: read from here and ignore source_dir
    SetupParam_t	*setup_param,	// setup parameters
					// if NULL: read SZS_SETUP_FILE
    uint		depth,		// creation depth
    uint		log_depth,	// print creation log if depth<log_depth
    bool		mark_readonly	// true: the file will never written
)
{
    DASSERT(szs);
    PRINT("CreateSZS(%p,%s,%s,%p,%p,%u,%u,%d)\n",
		szs, dest_fname, source_dir, sdir,
		setup_param, depth, log_depth, mark_readonly );
    ResetSZS(szs);
    if (mark_readonly)
	MarkReadonlySZS(szs);
    szs->fform_file	= szs->fform_arch = szs->fform_current = FF_DIRECTORY;
    szs->ff_attrib	= GetAttribFF(szs->fform_arch);
    szs->ff_version	= -1;
    szs->fname		= STRDUP(source_dir);
    szs->allow_ext_data	= true;

    //--- setup

    scan_data_t sd;
    memset(&sd,0,sizeof(sd));
    sd.szs = szs;
    sd.path_rel = StringCat2S(sd.path,sizeof(sd.path),source_dir,"/");
    if ( sd.path_rel > sd.path + 1 && sd.path_rel[-2] == '/' )
	sd.path_rel--;
    *sd.path_rel = 0;
    sd.depth = 1;
    sd.path_dir = sd.path_rel;

    if (!sdir)
    {
	struct stat st;
	if (stat(sd.path,&st))
	    return ERROR1(ERR_CANT_OPEN,"Can't open directory: %s\n",sd.path);
	if (!S_ISDIR(st.st_mode))
	    return ERROR1(ERR_CANT_OPEN,"Not a directory: %s\n",sd.path);
    }

    SetupParam_t local_setup_param;
    if (!setup_param)
    {
	*sd.path_rel = 0;
	InitializeSetupParam(&local_setup_param);
	setup_param = &local_setup_param;
	ScanSetupParam(setup_param,true,sd.path,SZS_SETUP_FILE,sdir,true);
    }
    sd.setup_param = setup_param;
    szs->links = opt_links && GetAttribFF(sd.setup_param->fform_arch) & FFT_LINK;


    //--- encode files

    uint encode_warnings = 0, encode_errors = 0;
    if ( !sdir && ( !opt_no_encode || opt_encode_img ))
    {
      disable_patch_on_load++;
      have_patch_count -= 1000000;
      const FormatFieldItem_t *ptr = setup_param->encode_list.list;
      const FormatFieldItem_t *end = ptr + setup_param->encode_list.used;
      for ( ; ptr < end; ptr++ )
      {
	ccp ext;
	bool encoding_needed = opt_encode_all;
	noPRINT("FF=%u %s\n",ptr->fform,GetNameFF(ptr->fform,0));
	switch(ptr->fform)
	{
	    case FF_BMG:
	    case FF_BMG_TXT:
	    case FF_KMP:
	    case FF_KMP_TXT:
	    case FF_LEX:
	    case FF_LEX_TXT:
 #if 0 // [[2do]] [[mdl]]
	    case FF_MDL:
	    case FF_MDL_TXT:
 #endif
	    case FF_PAT:
	    case FF_PAT_TXT:
		if (opt_no_encode)
		{
		    *sd.path_rel = 0;
		    continue;
		}
		ext = ".txt";
		break;

	    case FF_KCL:
	    case FF_KCL_TXT:
	    case FF_WAV_OBJ:
	    case FF_SKP_OBJ:
		if (opt_no_encode)
		{
		    *sd.path_rel = 0;
		    continue;
		}
		ext = ".obj";
		break;

	    case FF_TPL:
	    case FF_BTI:
	    case FF_TEX:
	    case FF_TEX_CT:
	    case FF_BREFT:
	    case FF_BREFT_IMG:
		ext = ".png";
		encoding_needed = opt_encode_all || opt_encode_img;
		break;

	    default:
		*sd.path_rel = 0;
		PRINT("NO-ENCODE: %s/%s\n",sd.path,ptr->key);
		continue;
	}

	struct stat st_dest;
	if (!encoding_needed)
	{
	    StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
	    encoding_needed = stat(sd.path,&st_dest) || !S_ISREG(st_dest.st_mode);
	}

	StringCat2E( sd.path_rel, sd.path+sizeof(sd.path), ptr->key, ext );
	TRACE("FIND: %s\n",sd.path);

	struct stat st;
	if ( stat(sd.path,&st) || !S_ISREG(st.st_mode) )
	{
	    NewFileExtS( sd.path, sizeof(sd.path), 0, ext );
	    if ( stat(sd.path,&st) || !S_ISREG(st.st_mode) )
		continue;
	}
	InsertStringField(&setup_param->exclude_name,sd.path_rel,false);
	if ( !encoding_needed && st.st_mtime <= st_dest.st_mtime )
	{
	    PRINT("NO-ENCODE [%d,%d]: %s\n", opt_encode_all, encoding_needed, sd.path );
	    continue;
	}
	PRINT("ENCODE: %s\n",sd.path );

	u8 * data = MALLOC(st.st_size+1);
	if (!LoadFILE(sd.path,0,0,data,st.st_size,0,0,false))
	{
	  data[st.st_size] = 0; // this EOT marker helps scanning text files

	  const file_format_t fform = GetByMagicFF(data,st.st_size,st.st_size);
	  if ( depth < log_depth )
		fprintf(stdlog,"%*s%sENCODE %s:%s\n",
			2*depth, "",
			testmode ? "WOULD " : "",
			GetNameFF(fform,0),
			sd.path );

	  switch (fform)
	  {
	    case FF_BMG:
	    case FF_BMG_TXT:
	      {
		bmg_t bmg;
		enumError err = ScanBMG(&bmg,true,sd.path,data,st.st_size);
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		if (!testmode)
		{
		    unlink(sd.path);
		    if (SaveRawXBMG(&bmg,sd.path,true))
			encode_errors++;
		}
		ResetBMG(&bmg);
	      }
	      break;

	    case FF_KCL:
	    case FF_KCL_TXT:
	    case FF_WAV_OBJ:
	    case FF_SKP_OBJ:
	      {
		kcl_t kcl;
		InitializeKCL(&kcl);
		kcl.fform_outfile = FF_KCL;
		kcl.fname = STRDUP(sd.path);
		enumError err = ScanKCL(&kcl,false,data,st.st_size,global_check_mode,true);
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		if (!testmode)
		{
		    unlink(sd.path);
		    if (SaveRawKCL(&kcl,sd.path,true))
			encode_errors++;
		}
		ResetKCL(&kcl);
	      }
	      break;

	    case FF_KMP:
	    case FF_KMP_TXT:
	      {
		kmp_t kmp;
		InitializeKMP(&kmp);
		kmp.fname = STRDUP(sd.path);
		enumError err = ScanKMP(&kmp,false,data,st.st_size,global_check_mode);
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		if (!testmode)
		{
		    unlink(sd.path);
		    if (SaveRawKMP(&kmp,sd.path,true))
			encode_errors++;
		}
		ResetKMP(&kmp);
	      }
	      break;

	    case FF_LEX:
	    case FF_LEX_TXT:
	      {
		lex_t lex;
		InitializeLEX(&lex);
		lex.fname = STRDUP(sd.path);
		enumError err = ScanLEX(&lex,false,data,st.st_size);
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		if (!testmode)
		{
		    unlink(sd.path);
		    if (SaveRawLEX(&lex,sd.path,true))
			encode_errors++;
		}
		ResetLEX(&lex);
	      }
	      break;

 #if 0 // [[mdl]]
	    case FF_MDL:
	    case FF_MDL_TXT:
	      {
		mdl_t mdl;
		InitializeMDL(&mdl);
		mdl.fname = STRDUP(sd.path);
		// [[mdl]] container [[2do]] [[container]]
		enumError err = ScanMDL(&mdl,false,data,st.st_size,0,global_check_mode);
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		if (!testmode)
		{
		    unlink(sd.path);
		    if (SaveRawMDL(&mdl,sd.path,true))
			encode_errors++;
		}
		ResetMDL(&kmp);
	      }
	      break;
 #endif

	    case FF_PAT:
	    case FF_PAT_TXT:
	      {
		pat_t pat;
		enumError err = ScanPAT(&pat,true,data,st.st_size,0,global_check_mode);
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		if (!testmode)
		{
		    unlink(sd.path);
		    if (SaveRawPAT(&pat,sd.path,true))
			encode_errors++;
		}
		ResetPAT(&pat);
	      }
	      break;

	    case FF_PNG:
	      {
		FREE(data);
		data = 0;
		Image_t img;
		enumError err = LoadIMG( &img, true, sd.path, 0,
					opt_mipmaps >= 0, false, opt_ignore );
		if (err)
		{
		    if ( err > ERR_WARNING )
		    {
			encode_errors++;
			break;
		    }
		    encode_warnings++;
		}

		Transform3IMG(&img,ptr->fform,ptr->iform,ptr->pform,true);
		TransformIMG(&img,-1);
		Transform2InternIMG(&img);

		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), ptr->key );
		const file_format_t fform
		    = GetImageFF( img.tform_valid ? img.tform_fform : FF_INVALID,
				    ptr->fform,
				    sd.path,
				    img.info_fform,
				    false,
				    FF_TEX );
		img.info_n_image = ptr->num;
		if (SaveIMG(&img,fform,0,sd.path,true))
		    encode_errors++;
	      }
	      break;

	    default:
	      ERROR0(ERR_INVALID_DATA,"Can't encode: %s\n",sd.path);
	      break;
	  }
	}
	FREE(data);
      }
      have_patch_count += 1000000;
      disable_patch_on_load--;
    }


    //--- create sub archives

    if ( !opt_no_recurse && !sdir )
    {
	disable_patch_on_load++;
	depth++;
	ccp *ptr = setup_param->create_list.field;
	ccp *end = ptr + setup_param->create_list.used;
	for ( ; ptr < end; ptr++ )
	{
	    StringCat2E( sd.path_rel, sd.path+sizeof(sd.path), *ptr, ".d" );
	    struct stat st;
	    if ( stat(sd.path,&st) || !S_ISDIR(st.st_mode) )
	    {
		NewFileExtS( sd.path, sizeof(sd.path), 0, ".d" );
		if ( stat(sd.path,&st) || !S_ISDIR(st.st_mode) )
		    continue;
	    }
	    InsertStringField(&setup_param->exclude_name,sd.path_rel,false);
	    PRINT("CREATE %s\n",sd.path);

	    SetupParam_t setup_param2;
	    InitializeSetupParam(&setup_param2);
	    ScanSetupParam(&setup_param2,true,sd.path,SZS_SETUP_FILE,0,true);
	    setup_param2.compr_mode = -1;

	    szs_file_t szs;
	    InitializeSZS(&szs);
	    if (!CreateSZS(&szs,0,sd.path,0,&setup_param2,depth,log_depth,mark_readonly))
	    {
		StringCopyE( sd.path_rel, sd.path+sizeof(sd.path), *ptr );
		struct stat st;
		const time_t mtime = stat(sd.path,&st) ? 0 : st.st_mtime;
#if 0
 BINGO;
 printf("mtime: %u - %u = %d [%s>%s>] size=%zu,%zu\n",
    (uint)mtime, (uint)szs.fatt.mtime, (int)(mtime-szs.fatt.mtime),
    GetNameFF(setup_param2.fform_file,setup_param2.fform_arch),
    GetNameFF(szs.fform_file,szs.fform_arch),
    szs.size, szs.csize );
#endif
		if ( mtime < szs.fatt.mtime )
		{
		    const bool is_compressed = IsCompressedFF(setup_param2.fform_file);
		    if (is_compressed)
			CompressSZS(&szs,true);

		    if ( depth < log_depth )
			fprintf(stdlog,"%*s%sCREATE %s:%s\n",
				2*depth, "",
				testmode ? "WOULD " : "",
				GetNameFF(szs.fform_file,szs.fform_arch),
				sd.path );
		    SaveSZS(&szs,sd.path,true,is_compressed);
		}
	    }
	    ResetSZS(&szs);
	}
	disable_patch_on_load--;
    }


    //--- scan files

    // pointer 'file' and 'part->file' becomes invalid if realloc() called
    // store the size first and then assign it (cross a sequence point)

    bool allow_add_files = false;
    switch (setup_param->fform_arch)
    {
	case FF_BRRES:
	    sd.align = opt_align_brres;
	    sd.max_depth = 20;
	    break;

	case FF_BREFF:
	case FF_BREFT:
	    sd.path_dir = StringCopyE(sd.path_rel,sd.path+sizeof(sd.path),"files/");
	    sd.align = 4; // any value because calculation is done later
	    break;

	case FF_PACK:
	    sd.align = opt_align_pack;
	    sd.max_depth = 20;
	    break;

	case FF_RKC:
	    sd.align = 0x10;
	    sd.max_depth = 0;
	    break;

	//case FF_RARC: // [[2do]] [[arc]]
	//case FF_U8:
	//case FF_WU8:
	default:
	    sd.align = opt_align_u8;
	    sd.max_depth = 20;
	    allow_add_files = true;
	    break;
    }

    *sd.path_dir = 0;
    const u32 n_sub_files = sdir ? scan_sdir(&sd,sdir) : scan_data(&sd);
    ASSERT_MSG( n_sub_files == szs->subfile.used,
		"%d+1 != %d [%s]\n", n_sub_files,
		szs->subfile.used, szs->subfile.list->path );


    //--- add missing files

    if ( !sdir && opt_auto_add && allow_add_files )
	AddMissingFiles(szs,source_dir,&sd,0, verbose>=0 ? 2*(int)depth : -1 );


    //--- debugging

    if ( logging > 1 )
    {
	int i;
	for ( i = 0; i < szs->subfile.used; i++ )
	{
	    szs_subfile_t * f = szs->subfile.list + i;
	    printf("%3d.: %u %6x %6x %s\n", i, f->is_dir, f->offset, f->size, f->path );
	}
    }


    //--- create archive

    enumError err;
    switch (setup_param->fform_arch)
    {
	case FF_BRRES:
	    szs->order_list = &setup_param->order_list;
	    szs->min_data_off = setup_param->min_data_off;
	    err = CreateBRRES( szs, source_dir, 0, sd.total_size );
	    break;

	case FF_BREFF:
	case FF_BREFT:
	    szs->order_list = &setup_param->order_list;
	    err = CreateBREFF( szs, source_dir, 0, sd.total_size, setup_param );
	    break;

	case FF_PACK:
	    szs->order_list = &setup_param->order_list;
	    err = CreatePACK( szs, source_dir, 0, sd.total_size, setup_param );
	    break;

	case FF_RKC:
	    err = CreateRKC(szs,source_dir);
	    setup_param->compr_mode = -1;
	    break;

	default:
	    err = CreateU8( szs, source_dir, 0,
			sd.namepool_size_u8, sd.total_size,
			setup_param->have_pt_dir > 0 );
	    szs->allow_ext_data = true;
	    break;
    }
    szs->order_list = 0;


    //--- transform & compress data

    szs->fform_file = szs->fform_arch;
    PatchSZS(szs);
    if ( szs->allow_ext_data )
    {
	const bool clean_lex = opt_lex_purge || HavePatchTestLEX();
	NormalizeExSZS(szs,opt_rm_aiparam,clean_lex,false);
    }

    if ( !err && setup_param->fform_arch == FF_WU8 )
	err = EncodeWU8(szs);

    if ( !err && setup_param->compr_mode >= 0 )
    {
	szs->dest_fname = dest_fname;
	CompressSZS(szs,true);
	szs->dest_fname = 0;
    }


    //--- clean and end

    if ( setup_param == &local_setup_param )
	ResetSetupParam(&local_setup_param);

    return encode_errors && err < ERR_ENCODING
		? ERR_ENCODING
		: encode_warnings && err < (enumError)ERR_ENCODING_WARN // [[dclib]]
			? ERR_ENCODING_WARN
			: err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LoadCreateSZS()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadCreateSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// valid pointer to filenname
    bool		decompress,	// decompress after loading
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    bool		mark_readonly	// true: the file will never written
)
{
    MEM_CHECK;
    DASSERT(szs);
    DASSERT(fname);
    TRACE("LoadCreateSZS(%d,%d) fname=%s\n",decompress,ignore_no_file,fname);

    struct stat st;
    if ( !stat(fname,&st) && S_ISDIR(st.st_mode) )
    {
	SetupParam_t sp;
	InitializeSetupParam(&sp);
	ScanSetupParam(&sp,true,fname,SZS_SETUP_FILE,0,true);
	sp.compr_mode = -1;
	const enumError err = CreateSZS(szs,0,fname,0,&sp,0,0,mark_readonly);
	ResetFileSZS(szs,true);
	ResetSetupParam(&sp);
	return err;
    }

    enumError err = LoadSZS(szs,fname,decompress,ignore_no_file,mark_readonly);
    PatchSZS(szs);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadObjFileListSZS
(
    szs_file_t		* szs,		// valid szs
    const void		* kmp_data,	// not NULL: source data
    size_t		data_size,	// size of 'kmp_data'
    ccp			kmp_fname,	// if !kmp_data: load kmp file
    bool		ignore_no_file,	// param for LoadKMP()/ScanKMP()
    CheckMode_t		check_mode,	// param for LoadKMP()/ScanKMP()
    lex_info_t		*lexinfo	// NULL or valid LEX info
)
{
    DASSERT(szs);
    if (szs->used_file)
	return ERR_OK;

    PRINT("sizeof(UsedFileFILE_t)=%zu\n",sizeof(UsedFileFILE_t));
    szs->used_file = CALLOC(1,sizeof(*szs->used_file));
    szs->required_file = CALLOC(1,sizeof(*szs->required_file));
    InitializeParamField(szs->required_file);


    //--- load KMP

    kmp_t kmp;
    enumError err;
    if (kmp_data)
    {
	InitializeKMP(&kmp);
	kmp.fname = STRDUP2(szs->fname,"/course.kmp");
	kmp.lexinfo = lexinfo;
	err = ScanKMP(&kmp,false,kmp_data,data_size,check_mode);
    }
    else
	err = LoadKMP(&kmp,true,kmp_fname,ignore_no_file,check_mode);
    if (err)
    {
	ResetKMP(&kmp);
	return err;
    }

    szs->n_cannon = kmp.dlist[KMP_CNPT].used;
    szs->is_arena = IsArenaKMP(&kmp);
    PRINT("LoadObjFileListSZS(): %d cannons found, is_arena=%d\n",
		szs->n_cannon, szs->is_arena );


    //--- find used objects

    const uint PFLAG = 0x3f;
    UsedObject_t pf_obj; // OR'ed presence flag
    UsedObject_t used_obj;
    memset(&used_obj,0,sizeof(used_obj));
    memset(&pf_obj,0,sizeof(pf_obj));
    PRINT("sizeof(UsedObject_t)=%zu\n",sizeof(used_obj));

    const kmp_gobj_entry_t *gobj = (kmp_gobj_entry_t*)kmp.dlist[KMP_GOBJ].list;
    const kmp_gobj_entry_t *gend = gobj + kmp.dlist[KMP_GOBJ].used;
    for ( ; gobj < gend; gobj++ )
    {
	const uint id = GetActiveObjectId(gobj);
	if (id)
	{
	    pf_obj.d[id] |= gobj->pflags & PFLAG;
	    used_obj.d[id] = 1;
	}

	const ObjectInfo_t *oi = GetObjectInfo(id);
	if ( oi && oi->flags & OBF_SPECIAL )
	{
	    const ObjSpec_t *os;
	    for ( os = ObjSpec; os->obj_id; os++ )
	     if ( os->obj_id == id )
	      switch (os->type)
	      {
		case OSP_FILE_BY_SETTING:
		  {
		    const int submode = os->mode / 10;
		    const int setting = os->mode % 10;
		    const uint value = gobj->setting[setting];
		    if ( submode == 1 && value == 1 )
			break;

		    char buf[100];
		    snprintf(buf,sizeof(buf),os->text,value);
		    InsertParamField(szs->required_file,buf,false,0,0);
		    break;
		  }

		default:
		    break;
	      }
	}
    }

    //DetectSpecialKMP(&kmp,szs->kmp_special);


    //--- slot analysis: 31+71, 62

    szs->slot_analyzed = true;

    szs->slot_31_71 = ( used_obj.d[GOBJ_SUN_DS]  ? SLOT_31_71_SUNDS : 0 )
		    | ( used_obj.d[GOBJ_PYLON01] ? SLOT_31_71_PYLON01 : 0 );

    szs->slot_62 = used_obj.d[GOBJ_HEYHO_SHIP] != 0;


    //--- slot analysis: 42

    szs->slot_42 = ( used_obj.d[0xd0] ? SLOT_42_KART_TRUCK_U : 0 )
		 | ( used_obj.d[0xd1] ? SLOT_42_CAR_BODY_U : 0 )
		 | ( pf_obj.d[0xd0] == PFLAG ? SLOT_42_KART_TRUCK_PF : 0 )
		 | ( pf_obj.d[0xd1] == PFLAG ? SLOT_42_CAR_BODY_PF : 0 );

    const uint USED42 = SLOT_42_KART_TRUCK_U | SLOT_42_CAR_BODY_U;
    if ( (szs->slot_42 & USED42) == USED42 )
	szs->slot_42 |= SLOT_42_ALL_U;

    const uint PF42 = SLOT_42_KART_TRUCK_PF | SLOT_42_CAR_BODY_PF;
    if ( (szs->slot_42 & PF42) == PF42 )
	szs->slot_42 |= SLOT_42_ALL_PF;
    //printf("--> SLOT_42 = %02x\n",szs->slot_42);


    //--- find used files

    UsedFileGROUP_t used_group;
    FindDbGroupByObjects(&used_group,true,&used_obj);
    FindDbFileByGroups(szs->used_file,false,&used_group);

    ResetKMP(&kmp);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

ParamFieldItem_t * IsFileRequiredSZS
(
    // check DBF_SPECIAL && szs->required_file
    szs_file_t		* szs,		// valid szs
    const DbFileFILE_t	* file		// file to proof
)
{
    DASSERT(szs);
    DASSERT(file);

    if (!szs->required_file)
	return 0;

    ParamFieldItem_t *it = FindParamField(szs->required_file,file->file);
    if (it)
	it->num |= 1;
    return it;
}

///////////////////////////////////////////////////////////////////////////////

bool IsFileOptionalSZS
(
    // check DBF_OPTIONAL && DBF_SPECIAL && szs->required_file
    szs_file_t		* szs,		// valid szs
    const DbFileFILE_t	* file		// NULL or file to proof
)
{
    DASSERT(szs);
    if ( !file || !(file->flags & DBF_OPTIONAL) )
	return false;

    return !IsFileRequiredSZS(szs,file);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void AddSpecialFile
	( szs_file_t *szs, ccp fname, const u8 *data, uint size )
{
    if (!szs->special_file)
    {
	szs->special_file = CALLOC(1,sizeof(*szs->special_file));
	InitializeParamField(szs->special_file);
	szs->special_file->free_data = true;
    }

    char buf[200];
    const file_format_t fform = GetByMagicFF(data,size,size);
    snprintf(buf,sizeof(buf),"%s %s",GetNameFF(fform,0),fname);

    ParamFieldItem_t *it
	= InsertParamFieldEx(szs->special_file,buf,false,size,0);
    if (!it->data)
    {
	sha1_hash_t *hash = MALLOC(sizeof(sha1_hash_t));
	SHA1(data,size,*hash);
	it->data = hash;
    }
}

//-----------------------------------------------------------------------------

static void CheckSpecialFile
	( szs_file_t *szs, const szs_subfile_t *file, ccp fname, ccp dir )
{
    DASSERT(szs);
    DASSERT(file);
    DASSERT(dir);

    const int dlen = strlen(dir);
    if ( strlen(fname) > dlen
	&& !strncasecmp(fname,dir,dlen)
	&& !strchr(fname+dlen,'/') )
    {
	AddSpecialFile(szs,fname,szs->data+file->offset,file->size);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FindSpecialFilesSZS
(
    szs_file_t		* szs,		// valid szs
    bool		force		// true: force new scanning
)
{
    DASSERT(szs);

    if ( !force && szs->special_done )
	return;

    ClearSpecialFilesSZS(szs);
    CollectFilesSZS(szs,true,0,-1,SORT_NONE);

    szs_subfile_t *file, *file_end = szs->subfile.list + szs->subfile.used;
    for ( file = szs->subfile.list; file < file_end; file++ )
    {
	ccp fname = file->path;
	if ( fname[0] == '.' && fname[1] == '/' )
	    fname += 2;

	if ( !szs->course_kcl_data && !strcasecmp(fname,"course.kcl") )
	{
	    szs->course_kcl_data = szs->data + file->offset;
	    szs->course_kcl_size = file->size;
	}
	else if ( !szs->course_kmp_data && !strcasecmp(fname,"course.kmp") )
	{
	    szs->course_kmp_data = szs->data + file->offset;
	    szs->course_kmp_size = file->size;
	}
	else if ( !szs->course_lex_data && !strcasecmp(fname,"course.lex") )
	{
	    szs->course_lex_data = szs->data + file->offset;
	    szs->course_lex_size = file->size;
	    szs->szs_special[HAVESZS_COURSE_LEX] = true;
	}
	else if ( !szs->course_model_data && !strcasecmp(fname,"course_model.brres") )
	{
	    szs->course_model_data = szs->data + file->offset;
	    szs->course_model_size = file->size;
	}
	else if ( !szs->course_d_model_data && !strcasecmp(fname,"course_d_model.brres") )
	{
	    szs->course_d_model_data = szs->data + file->offset;
	    szs->course_d_model_size = file->size;
	}
	else if ( !szs->vrcorn_model_data && !strcasecmp(fname,"vrcorn_model.brres") )
	{
	    szs->vrcorn_model_data = szs->data + file->offset;
	    szs->vrcorn_model_size = file->size;
	}
	else if ( !szs->map_model_data && !strcasecmp(fname,"map_model.brres") )
	{
	    szs->map_model_data = szs->data + file->offset;
	    szs->map_model_size = file->size;
	}
	else if ( !szs->have_ice_brres && !strcasecmp(fname,"ice.brres") )
	{
	    szs->have_ice_brres = true;
	}

	if ( !szs->moonview_mdl_stat
		&& (  !strcasecmp(fname,"course_model.brres")
		   || !strcasecmp(fname,"course_d_model.brres") ))
	{
	    szs_file_t subszs;
// [[fname+]]
	    InitializeSubSZS(&subszs,szs,file->offset,file->size,FF_UNKNOWN,fname,false);
	    Slot42MaterialStat_t slot42 = GetSlot42SupportSZS(&subszs);
	    noPRINT("SLOT42: found=%03x, mod=%03x, all=%d, cok=%d, ok=%d\n",
			slot42.found, slot42.modified,
			slot42.all_found, slot42.content_ok, slot42.ok );
	    szs->moonview_mdl_stat = slot42.ok		? 3
				   : slot42.all_found	? 2
				   : slot42.found	? 1
				   :			  0;
	    ResetSZS(&subszs);
	}
	else
	{
	    uint i;
	    for ( i = 0; i < HAVESZS__N; i++ )
		if ( !szs->szs_special[i] && !strcasecmp(fname,have_szs_file[i]) )
		{
		    szs->szs_special[i] = true;
		    break;
		}
	}

	CheckSpecialFile(szs,file,fname,"common/");
	CheckSpecialFile(szs,file,fname,"itemslottable/");
    }
}

///////////////////////////////////////////////////////////////////////////////

ccp CreateSpecialFileInfo
	( szs_file_t * szs, bool add_value, ccp return_if_empty )
{
    DASSERT(szs);

    static char buf[500];
    char *dest = buf;

    if (add_value)
    {
	uint i, val = 0;
	for ( i = 0; i < HAVESZS__N; i++ )
	    if ( szs->szs_special[i] )
		val |= 1 << i;
	dest = snprintfE( dest, buf+sizeof(buf), "%u=" , val );
    }

    uint i;
    ccp sep = "";
    for ( i = 0; i < HAVESZS__N; i++ )
	if ( szs->szs_special[i] )
	{
	    dest = StringCat2E(dest,buf+sizeof(buf),sep,have_szs_name[i]);
	    sep = ",";
	}

    return dest == buf ? return_if_empty : CopyCircBuf0(buf,dest-buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			NormalizeSZS()			///////////////
///////////////////////////////////////////////////////////////////////////////

static int norm_collect_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if (term)
	return 0;

    szs_norm_t * norm = it->param;
    DASSERT(norm);

    uint depth = it->depth;
    ccp path = it->path;
    if ( path[0] == '.' && path[1] == '/' )
    {
	path += 2;
	depth--;
	if (!*path)
	{
	    norm->have_pt_dir = true;
	    return 0;
	}
    }

    if (!*path)
	return 0;

    if ( norm->rm_aiparam && !strncasecmp(path,"aiparam",7) )
    {
	PATCH_ACTION_LOG("Remove","SZS","%s\n","AIPARAM");
	it->szs->aiparam_removed = true;
	return 0;
    }

    if ( norm->clean_lex && !strncasecmp(path,"course.lex",10) )
    {
	lex_t lex;
	u8 *data = it->szs->data + it->off;
	enumError err = ScanRawLEX(&lex,true,data,it->size,0);
	if ( !err && PatchLEX(&lex) && CreateRawLEX(&lex) == ERR_OK )
	{
	    if (!lex.have_lex)
	    {
		PATCH_ACTION_LOG("Remove","SZS","%s\n","course.lex");
		return 0;
	    }

	    if ( lex.raw_data_size <= it->size
		&& memcmp(data,lex.raw_data,lex.raw_data_size) )
	    {
		PATCH_ACTION_LOG("Purge","SZS","%s\n","course.lex");
		memcpy(data,lex.raw_data,lex.raw_data_size);
		it->size = lex.raw_data_size;
	    }
	}
    }

    if (ShallRemoveFile(path))
    {
	PATCH_ACTION_LOG("Remove","SZS","%s\n",path);
	return 0;
    }

    noPRINT_IF(it->is_dir,"ADD[%u-%u=%u]: %s\n",
		it->size, it->index, it->size-it->index, path );

    szs_subfile_t * file = AppendSubfileSZS(it->szs,it,0);
    DASSERT(file);
    ccp ptr = file->path + strlen(file->path) - 1;
    if (it->is_dir)
	ptr--;
    while ( ptr > file->path && *ptr != '/' )
	ptr--;
    if ( *ptr == '/' )
	ptr++;

    norm->namepool_size += strlen(ptr);
    noPRINT("ADD: %s[%zu+%u]\n",ptr,strlen(ptr),!it->is_dir);
    if (it->is_dir)
    {
	file->offset = depth;
	file->size   = it->size - it->index - 1;
    }
    else
    {
	file->device = it->size;
	file->inode  = it->off;

	uint relevant_size = file->size;
	if (it->szs->ext_data.used)
	{
	    int idx;
	    for ( idx = 0; idx < it->szs->ext_data.used; idx++ )
	    {
		szs_subfile_t * f = it->szs->ext_data.list + idx;
		if (!StrPathCmp(it->path,f->path))
		{
		    PRINT("########## %s %s / %u -> %u%s\n",
				it->path, f->path, it->size, f->size,
				f->removed ? " REMOVE!" : "" );
		    file->ext = f;
		    relevant_size = f->size;
		    file->device = 0;
		    file->inode  = 0;

		    if ( f->removed)
		    {
			norm->namepool_size -= strlen(ptr)+1;
			relevant_size = 0;
		    }
		}
	    }
	}

	szs_subfile_t *lptr = opt_links
			? FindLinkSZS(it->szs,file->device,file->inode,file) : 0;
	if (lptr)
	{
	    if (!lptr->link_index)
		lptr->link_index = ++it->szs->subfile.link_count;
	    file->link_index = lptr->link_index;
	    PRINT("NORM/LINK[%d]: %s  ->  %s\n",
			lptr->link_index, lptr->path, file->path );
	}
	else
	    norm->total_size += ALIGN32(relevant_size,opt_align_u8);

	norm->namepool_size++;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool NormalizeSZS ( szs_file_t *szs )
{
    const bool clean_lex = opt_lex_purge || HavePatchTestLEX();
    return NormalizeExSZS(szs,opt_rm_aiparam,clean_lex,opt_auto_add);
}

//-----------------------------------------------------------------------------

bool NormalizeExSZS
	( szs_file_t *szs, bool rm_aiparam, bool clean_lex, bool autoadd )
{
    DASSERT(szs);
    PRINT("*** NormalizeExSZS(,%d,%d,%d) ***\n",rm_aiparam,clean_lex,autoadd);

    // [[2do]] [[arc]]
    if ( szs->fform_arch != FF_U8 && szs->fform_arch != FF_WU8 || !szs->size || !szs->data )
	return false;

    ResetFileSZS(szs,false);

    szs_norm_t norm;
    memset(&norm,0,sizeof(norm));
    norm.rm_aiparam = rm_aiparam;
    norm.clean_lex = clean_lex;
    IterateFilesSZS(szs,norm_collect_func,&norm,false,0,-1,SORT_NONE);

    if (autoadd)
	AddMissingFiles(szs,0,0,&norm,2);
    AddSlotFiles(szs,&norm);
    AddTestLEX(szs,&norm);
    SortSubFilesSZS(szs,SORT_AUTO);

    uint old_size	= szs->size;
    u8 * old_data	= szs->data;
    bool old_alloced	= szs->data_alloced;
    szs->data		= 0;
    szs->data_alloced	= false;

    CreateU8(szs,0,old_data,norm.namepool_size,norm.total_size,norm.have_pt_dir);

    bool dirty = old_size != szs->size || memcmp(old_data,szs->data,old_size);
    if (old_alloced)
	FREE(old_data);
    return dirty;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddMissingFiles()		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct add_missing_t
{
    szs_file_t		* szs;		// valid szs
    scan_data_t		* sd;		// NULL or valid scan data
    szs_norm_t		* norm;		// NULL or valid norm data
    int			log_indent;	// >-1: print log with indention
    bool		print_err;	// true: print errors about missing files

} add_missing_t;

//-----------------------------------------------------------------------------

static int add_missing_file
(
    ccp			path,		// calculated path
    file_format_t	fform,		// FF_BRRES | FF_BREFF | FF_BREFT
    add_missing_t	*am		// user defined parameter
)
{
    DASSERT(path);
    DASSERT(fform);
    DASSERT(am);
    DASSERT(am->szs);
    DASSERT( am->sd || am->norm );

    noPRINT("--> %s:%s\n", GetNameFF(0,fform), path );

    szs_subfile_list_t *subfile = &am->szs->subfile;
    szs_subfile_t *sptr, *send = subfile->list + subfile->used;
    for ( sptr = subfile->list; sptr < send; sptr++ )
    {
	ccp spath = sptr->path;
	if ( spath[0] == '.' && spath[1] == '/' )
	    spath += 2;
	if (!strcmp(path,spath))
	{
	    PRINT("AUTO-ADD/FOUND:   %s\n",path);
	    return 0;
	}
    }

    const bool is_course_lex = !strcmp(path,"course.lex");

    s64 size = 0;
    char load_path[PATH_MAX];

    if (is_course_lex)
    {
	StringCopyS(load_path,sizeof(load_path),path);
    }
    else
    {
	size = FindAutoAdd(path,0,load_path,sizeof(load_path));
	if ( size <= 0 )
	{
	    noPRINT("AUTO-ADD/MISSING: %s\n",path);
	    if (am->print_err)
		ERROR0(ERR_WARNING,
		    "Missing file in auto-add archive: %s\n",path);
	 #ifdef TEST
	    if (verbose>=0)
		ERROR0(ERR_WARNING,"Missing sub file '%s': %s\n",
		    path, am->szs->fname );
	 #endif
	    return 0;
	}
    }

    if ( am->log_indent >= 0 )
	fprintf(stdlog,"%*s%sAUTO-ADD %s\n",
			am->log_indent, "",
			testmode ? "WOULD " : "",
			load_path );

    int stack[10], *stack_end = stack;
    int dir_id = 0;

    char path_buf[ARCH_FILE_MAX];
    StringCopyS(path_buf,sizeof(path_buf),path);
    char *ptr = path_buf;
    bool pt_prefix = false;

    for(;;)
    {
	ccp start = ptr;
	while ( *ptr && *ptr != '/' )
	    ptr++;
	if ( !*ptr || !ptr[1] )
	    break;

	const char save_ch = *++ptr;
	*ptr = 0;
	bool found = false;
	szs_subfile_t *sptr, *send = subfile->list + subfile->used;
	for ( sptr = subfile->list; sptr < send; sptr++ )
	{
	    noPRINT(" -> %s | %s\n",path_buf,sptr->path);
	    ccp spath = sptr->path;
	    if ( spath[0] == '.' && spath[1] == '/' )
		spath += 2;
	    if (!strcmp(path_buf,spath))
	    {
		found = true;
		if ( spath > sptr->path )
		    pt_prefix = true;
		break;
	    }
	}

	if (found)
	{
	    dir_id = am->sd ? sptr->dir_id : sptr - subfile->list;
	    PRINT("DIR-FOUND[%u]: %s\n",dir_id,sptr->path);
	}
	else
	{

	    if (am->sd)
	    {
		int *stack_ptr;
		for ( stack_ptr = stack; stack_ptr < stack_end; stack_ptr++ )
		    subfile->list[*stack_ptr].size++;

		sptr = AppendSubfileSZS(am->szs,0,0);
	    }
	    else
	    {
		int *stack_ptr;
		for ( stack_ptr = stack; stack_ptr < stack_end; stack_ptr++ )
		{
		    subfile->list[*stack_ptr].size++;
		    if ( *stack_ptr > dir_id )
			(*stack_ptr)++;
		}

		sptr = InsertSubfileSZS(am->szs,++dir_id,0,0);
	    }
	    DASSERT(sptr);

	    sptr->dir_id	= dir_id;
	    sptr->is_dir	= true;
	    sptr->path		= pt_prefix ? STRDUP2("./",path_buf) : STRDUP(path_buf);
	    PRINT("ADD-DIR[%d]:  %s\n",dir_id,sptr->path);

	    if (am->sd)
	    {
		am->sd->namepool_size_u8 += ptr - start;
		dir_id = am->sd->n_directories++;
	    }
	    else if (am->norm)
	    {
		am->norm->namepool_size += ptr - start;
		sptr->offset = stack_end - stack + 1;
	    }
	}

	sptr->size++;
	*stack_end++ = sptr - subfile->list;
	*ptr = save_ch;
    }

    lex_t lex; // only valid if 'is_course_lex'
    if ( is_course_lex )
    {
	InitializeLEX(&lex);
	if ( PatchLEX(&lex) && CreateRawLEX(&lex) == ERR_OK )
	    size = lex.raw_data_size;
    }

    szs_subfile_t *file = 0;
    if (am->sd)
    {
	file = AppendSubfileSZS(am->szs,0,0);
	DASSERT(file);
	ccp fname = strrchr(path,'/');
	am->sd->namepool_size_u8 += strlen(fname ? fname+1 : path) + 1;

	file->dir_id        = dir_id;
	file->is_dir        = false;
	file->path          = STRDUP(path);
	file->size          = size;
	am->sd->total_size  += ALIGN32(file->size,am->sd->align);

	if (!is_course_lex)
	    file->load_path = STRDUP(load_path);
	PRINT("ADD-FILE[%d]: %s\n",dir_id,path);
    }
    else if (am->norm)
    {
	file = InsertSubfileSZS(am->szs,dir_id+1,0,0);

	file->dir_id	= dir_id;
	file->is_dir	= false;
	file->path	= pt_prefix ? STRDUP2("./",path) : STRDUP(path);
	file->size	= size;

	if (!is_course_lex)
	    file->load_path = STRDUP(load_path);

	ccp fname = strrchr(path,'/');
	am->norm->namepool_size += strlen(fname ? fname+1 : path) + 1;
	am->norm->total_size  += ALIGN32(file->size,opt_align_u8);

	PRINT("ADD-FILE[%d,%zd]: %s, %u bytes\n",
		dir_id+1, file-am->szs->subfile.list, path, file->size );
    }

    if (is_course_lex)
    {
	if (file)
	{
	    file->data_alloced = true;
	    file->data = lex.raw_data;
	    lex.raw_data = 0;
	}
	ResetLEX(&lex);
    }

    return 1;
}

//-----------------------------------------------------------------------------

int AddMissingFileSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// filename to add
    file_format_t	fform,		// valid file format
    struct szs_norm_t	*norm,		// NULL or norm struct
    int			log_indent	// >-1: print log with indention
)
{
    DASSERT(szs);
    DASSERT(fname);
    DASSERT(fform);

    add_missing_t am	= {0};
    am.szs		= szs;
    am.norm		= norm;
    am.log_indent	= log_indent;
    am.print_err	= true;
    return add_missing_file(fname,fform,&am);
}

//-----------------------------------------------------------------------------

enumError AddMissingFiles
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to base directory
    struct scan_data_t	* sd,		// NULL or valid scan data
    struct szs_norm_t	* norm,		// NULL or valid norm data
    int			log_indent	// >-1: print log with indention
)
{
    DASSERT(szs);
    DASSERT( sd || norm );

    if (!SetupAutoAdd())
	return ERR_OK;

    char path_buf[PATH_MAX];
    if (source_dir)
    {
	ccp path = PathCatPP(path_buf,sizeof(path_buf),source_dir,"course.kmp");
	LoadObjFileListSZS(szs,0,0,path,true,0,0);
    }
    else
    {
	szs_subfile_t *sptr, *send = szs->subfile.list + szs->subfile.used;
	for ( sptr = szs->subfile.list; sptr < send; sptr++ )
	    if ( !strcmp(sptr->path,"./course.kmp") || !strcmp(sptr->path,"course.kmp") )
		break;
	if ( sptr >= send )
	    return ERR_WARNING;
	LoadObjFileListSZS(szs,szs->data+sptr->offset,sptr->size,0,true,0,0);
    }
    if (!szs->used_file)
	return ERR_WARNING;

    add_missing_t am	= {0};
    am.szs		= szs;
    am.sd		= sd;
    am.norm		= norm;
    am.log_indent	= log_indent;

    int i, insert_count = 0;
    for ( i = 0; i < N_DB_FILE_FILE; i++ )
	if (szs->used_file->d[i])
	{
	    const DbFileFILE_t *ptr = DbFileFILE + i;
	    if (!IsFileOptionalSZS(szs,ptr))
		insert_count += add_missing_file(ptr->file,ptr->fform,&am);
	}

    #if HAVE_PRINT0
    {
	szs_subfile_t *sptr, *send = szs->subfile.list + szs->subfile.used;
	for ( sptr = szs->subfile.list; sptr < send; sptr++ )
	    PRINT("-- %2d.%d %6u %s\n",
			sptr->dir_id, sptr->is_dir, sptr->size, sptr->path );
    }
    #endif

    return insert_count ? ERR_DIFFER : ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

void AddTestLEX ( szs_file_t *szs, szs_norm_t *norm )
{
    DASSERT(szs);
    if (HaveActivePatchTestLEX())
    {
	lex_test_t temp;
	memcpy(&temp,&TestDataLEX,sizeof(temp));
	bool empty;
	PatchTestLEX(&temp,&empty);
	if ( !empty || force_lex_test )
	    AddMissingFileSZS(szs,"course.lex",FF_LEX,norm,verbose<1?-1:0);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cut files			///////////////
///////////////////////////////////////////////////////////////////////////////
// The 'cut files' iterators are placed in this file, because they cover
// all other file formats like CreateSZS().
///////////////////////////////////////////////////////////////////////////////

static int IterateFilesIMG
(
    szs_iterator_t	*it,		// iterator struct with all infos
    bool		multi_img	// false: mipmaps, true: multi images
)
{
    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 * data = szs->data;
    DASSERT(data);

    int stat = 0;

    Image_t img;
    if (!AssignIMG(&img,true,data,szs->size,0,true,it->endian,EmptyString))
    {
	if (!img.mipmap)
	    multi_img = false;

	//--- first evaluate palette data

	int i;
	Image_t *iptr;
	u8 *last_pal = 0;
	for ( i = 0, iptr = &img; iptr && !stat; i++, iptr = iptr->mipmap )
	{
	    if ( iptr->pal && iptr->pal != last_pal )
	    {
		it->index++;
		it->off		= iptr->pal - data;
		it->size	= iptr->pal_size;
		if (multi_img)
		    snprintf(it->path,sizeof(it->path),"palette-%u.%u.%s",
			i, iptr->n_pal, GetPaletteFormatName(iptr->pform,"unknown") );
		else
		    snprintf(it->path,sizeof(it->path),"palette.%u.%s",
			iptr->n_pal, GetPaletteFormatName(iptr->pform,"unknown") );
		it->func_it(it,false);
	    }
	}


	//--- second: evaluate image data

	for ( i = 0, iptr = &img; iptr && !stat; i++, iptr = iptr->mipmap )
	{
	    it->index++;
	    it->off	= iptr->data - data;
	    it->size	= iptr->data_size;
	    ccp formname = GetImageFormatName(iptr->iform,"unknown");
	    if (multi_img)
		snprintf(it->path,sizeof(it->path),"image-%u.%ux%u.%s",
			i, iptr->width, iptr->height, formname );
	    else if (i)
		snprintf(it->path,sizeof(it->path),"mipmap-%u.%ux%u.%s",
			i, iptr->width, iptr->height, formname );
	    else
		snprintf(it->path,sizeof(it->path),"image.%ux%u.%s",
			iptr->width, iptr->height, formname );
	    stat = it->func_it(it,false);
	}
    }
    ResetIMG(&img);
    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int IterateFilesBREFT_IMG
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if (term)
	return 0;

    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);

    if ( !szs->data || szs->size < sizeof(breft_image_t) )
	return -1;

    it->no_recurse++;

    //const u8 * data		= szs->data;
    //const breft_image_t *bi	= (breft_image_t*)data;

    it->index	= 0;
    it->off	= 0;
    it->size	= sizeof(breft_image_t);
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),".BREFT-IMG.header");
    it->func_it(it,false);

    return IterateFilesIMG(it,false);
}

///////////////////////////////////////////////////////////////////////////////

static int IterateFilesTEX
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if (term)
	return 0;

    // [[2do]] ??? BRSUB support

    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);

    if ( !szs->data || szs->size <= sizeof(brsub_header_t) )
	return -1;

    const u8 * data		= szs->data;
    const u8 * data_end		= data + szs->size;
    const brsub_header_t *bh	= (brsub_header_t*)data;
    const uint n_grp		= GetSectionNumBRSUB(data,szs->size,it->endian);
    const tex_info_t *ti	= (tex_info_t*)( bh->grp_offset + n_grp );

    if ( (u8*)ti >= data_end || memcmp(bh->magic,TEX_MAGIC,sizeof(bh->magic)) )
	return -1;

    it->no_recurse++;

    it->index	= 0;
    it->off	= 0;
    it->size	= (u8*)ti - data;
    it->is_dir	= 0;
    PrintHeaderNameBRSUB(it->path,sizeof(it->path),bh,n_grp,it->endian);
    it->func_it(it,false);

    it->index++;
    it->off	+= it->size;
    it->size	= sizeof(*ti);
    StringCopyS(it->path,sizeof(it->path),".TEX.header");
    it->func_it(it,false);

    if ( be32(&bh->version) == 3 )
    {
	const u32 raw_off = 0x40;
	const u32 img_off = be32(bh->grp_offset);
	if ( img_off > raw_off && img_off <= szs->size )
	{
	    const bool have_ct_code = szs->fform_arch == FF_TEX_CT
					&& img_off > CT_CODE_TEX_OFFSET;

	    const u32 raw_end = have_ct_code ? CT_CODE_TEX_OFFSET : img_off;
	    it->index++;
	    it->off  = raw_off;
	    it->size = raw_end - raw_off;
	    StringCopyS(it->path,sizeof(it->path),"raw.bin");
	    it->func_it(it,false);

	    if (have_ct_code)
	    {
		it->index++;
		it->off  = CT_CODE_TEX_OFFSET;
		it->size = img_off - CT_CODE_TEX_OFFSET;
		StringCopyS(it->path,sizeof(it->path),"raw.ctcode");
		it->func_it(it,false);
	    }
	}
    }

    return IterateFilesIMG(it,false);
}

///////////////////////////////////////////////////////////////////////////////

static int IterateFilesTPL
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if (term)
	return 0;

    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);

    if ( !szs->data || szs->size < sizeof(tpl_header_t) + sizeof(tpl_imgtab_t) )
	return -1;

    const tpl_header_t *head;
    const tpl_imgtab_t *tab;
    const u8 * data = szs->data;
    if (!SetupPointerTPL(data,szs->size,0,&head,&tab,0,0,0,0,it->endian))
	return -1;

    it->no_recurse++;

    const uint n_img = it->endian->rd32(&head->n_image);

    it->index	= 0;
    it->off	= (u8*)head - data;
    it->size	= sizeof(tpl_header_t);
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),".TPL.header");
    it->func_it(it,false);

    it->index++;
    it->off	= (u8*)tab - data;
    it->size	= sizeof(tpl_imgtab_t) * n_img;
    StringCopyS(it->path,sizeof(it->path),".TPL.image-table");
    it->func_it(it,false);

    uint i;
    for ( i = 0; i < n_img; i++ )
    {
	const tpl_pal_header_t	*tpl_pal;
	const tpl_img_header_t	*tpl_img;
	if (SetupPointerTPL(data,szs->size,i,0,0,&tpl_pal,&tpl_img,0,0,it->endian))
	{
	    if (tpl_pal)
	    {
		it->index++;
		it->off  = (u8*)tpl_pal - data;
		it->size = sizeof(tpl_pal_header_t);
		if ( n_img > 0 )
		    snprintf(it->path,sizeof(it->path),".palette-%u.header",i);
		else
		    StringCopyS(it->path,sizeof(it->path),".palette.header");
		it->func_it(it,false);
	    }

	    if (tpl_img)
	    {
		it->index++;
		it->off  = (u8*)tpl_img - data;
		it->size = sizeof(tpl_img_header_t);
		if ( n_img > 0 )
		    snprintf(it->path,sizeof(it->path),".image-%u.header",i);
		else
		    StringCopyS(it->path,sizeof(it->path),".image.header");
		it->func_it(it,false);
	    }
	}
    }

    return IterateFilesIMG(it,true);
}

///////////////////////////////////////////////////////////////////////////////

static int IterateFilesBTI
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if (term)
	return 0;

    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);

    if ( !szs->data || szs->size < sizeof(bti_header_t) )
	return -1;

    it->index++;
    it->off	= 0;
    it->size	= sizeof(bti_header_t);
    StringCopyS(it->path,sizeof(it->path),".BTI.header");
    it->func_it(it,false);

    return IterateFilesIMG(it,false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int IterateFilesBMG
(
    szs_iterator_t	*it,		// iterator struct with all infos
    bool		multi_img	// false: mipmaps, true: multi images
)
{
    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 * data = szs->data;
    const u8 * data_end	= data + szs->size;
    DASSERT(data);

    if ( !data || szs->size < sizeof(bmg_header_t) + sizeof(bmg_section_t) )
	return -1;

    const bmg_header_t * bh = (bmg_header_t*)data;
    if (memcmp(bh->magic,BMG_MAGIC,sizeof(bh->magic)))
	return -1;

    it->no_recurse++;

    it->index	= 0;
    it->off	= 0;
    it->size	= sizeof(bmg_header_t);
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),".BMG.header");
    int stat = it->func_it(it,false);

    const bmg_section_t *sect = (bmg_section_t*)(bh+1);
    char id_buf[sizeof(sect->magic)+1];
    int idx;
    for ( idx = 0; (u8*)sect < data_end && !stat; idx++ )
    {
	const uint sect_size = it->endian->rd32(&sect->size);
	if (!sect_size)
	    break;

	it->index++;
	it->off		= (u8*)sect - data;
	it->size	= sect_size;
	snprintf(it->path,sizeof(it->path), "BMG.section-%u.%s",
		idx, PrintID(sect->magic,sizeof(sect->magic),id_buf) );
	stat = it->func_it(it,false);

	sect = (bmg_section_t*)( (u8*)sect + sect_size );
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

static int IterateFilesKCL
(
    szs_iterator_t	*it,		// iterator struct with all infos
    bool		multi_img	// false: mipmaps, true: multi images
)
{
    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 *data = szs->data;
    DASSERT(data);

    kcl_analyze_t ka;
    if ( IsValidKCL(&ka,data,szs->size,szs->size,"") >= VALID_ERROR )
	return -1;

    it->no_recurse++;

    const kcl_head_t *kclhead = (kcl_head_t*)data;
    u32 sect_off[N_KCL_SECT+1];
    DASSERT( sizeof(sect_off) > sizeof(kclhead->sect_off) );

    it->index	= 0;
    it->off	= 0;
    it->size	= sizeof(kcl_head_t);
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),".KCL.header.bin");
    int stat = it->func_it(it,false);

    uint i;
    for ( i = 0; i < N_KCL_SECT; i++ )
	sect_off[i] = be32(kclhead->sect_off+i);
    sect_off[2] += 0x10;
    sect_off[N_KCL_SECT] = szs->size;

    static ccp  name[N_KCL_SECT] = { "verticies", "normals", "triangles", "spartial-index" };
    static uint size[N_KCL_SECT] = { sizeof(double3), sizeof(double3),
				     sizeof(kcl_triangle_t), 0 };

    for ( i = 0; i < N_KCL_SECT && !stat; i++ )
    {
	it->off		= sect_off[i];
	it->size	= sect_off[i+1] - sect_off[i];
	if (size[i])
	    snprintf(it->path,sizeof(it->path),"KCL.section-%u.%u_%s.bin",
		i+1,it->size/size[i],name[i]);
	else
	    snprintf(it->path,sizeof(it->path),"KCL.section-%u.%s.bin",i+1,name[i]);
	stat = it->func_it(it,false);
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

static int IterateFilesKMP
(
    szs_iterator_t	*it,		// iterator struct with all infos
    bool		multi_img	// false: mipmaps, true: multi images
)
{
    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 * data = szs->data;
    DASSERT(data);

    if ( IsValidKMP(data,szs->size,szs->size,"") >= VALID_ERROR )
	return -1;

    it->no_recurse++;

    kmp_head_info_t hi;
    ScanHeadInfoKMP(&hi,data,szs->size);

    it->index	= 0;
    it->off	= 0;
    it->size	= sizeof(hi.head_size);
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),".KMP.header");
    it->func_it(it,false);

    int stat = 0;
    uint sect, min_off = 0;
    char id_buf[4+1];

    for ( sect = 0; sect < hi.n_sect && !stat; sect++ )
    {
	uint off = ntohl(hi.sect_off[sect]);
	if ( off < min_off || off > hi.max_off || off&3 )
	    continue;

	it->index++;
	off		+= hi.head_size;
	it->off		= off;
	it->size	= 0;

	PrintID(data+off,4,id_buf);
	int abbrev_count;
	const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,id_buf,kmp_section_name);
	if ( cmd && !abbrev_count && cmd->id >= 0 )
	{
	    const kmp_list_head_t *list = (kmp_list_head_t*) ( data + off );
	    if ( cmd->id < KMP_N_SECT )
	    {
		const uint n = be16(&list->n_entry);
		it->size = kmp_entry_size[cmd->id] * n + sizeof(kmp_list_head_t);
		if ( cmd->id == KMP_POTI )
		    it->size += be16(&list->value) * sizeof(kmp_poti_point_t);
	    }
	    else if ( cmd->id == KMP_WIM0 )
		it->size = be32(&list->n_entry) + sizeof(kmp_list_head_t);
	}

	snprintf(it->path,sizeof(it->path), "KMP.section-%02u.%s",
		sect, id_buf );
	stat = it->func_it(it,false);
    }

    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PatchSZS()			///////////////
///////////////////////////////////////////////////////////////////////////////

static int transform_collect_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if ( !term && !it->is_dir )
    {
	int *dirty = it->param;
	DASSERT(dirty);
	szs_file_t *szs = it->szs;
	DASSERT(szs);
	u8 * data = szs->data + it->off;
// [[analyse-magic]]
	switch (GetByMagicFF(data,it->size,it->size))
	{
	    case FF_BMG:
		if (!have_bmg_patch_count)
		{
		    PRINT("### Patch/NOT BMG: %s\n",it->name);
		    break;
		}

		PRINT("### Patch BMG: %s/%s\n",szs->fname,it->name);
		{
		    bmg_t bmg;
		    enumError err = ScanBMG(&bmg,true,it->name,data,it->size);
		    if (!err)
		    {
			bmg.szs = szs;
			err = UsePatchingListBMG(&bmg);
			if ( err == ERR_DIFFER && !CreateRawBMG(&bmg) )
			{
			    PRINT("### BMG: %u -> %u\n",it->size,bmg.raw_data_size);
			    if ( bmg.raw_data_size == it->size
				|| !szs->allow_ext_data && bmg.raw_data_size < it->size )
			    {
				PATCH_ACTION_LOG("Patch","BMG/RAW","%s\n",it->name);
				memcpy(data,bmg.raw_data,it->size);
				it->size = bmg.raw_data_size;
				*dirty = 1;
			    }
			    else if ( szs->allow_ext_data )
			    {
				PATCH_ACTION_LOG("Patch","BMG","%s\n",it->name);
				szs_subfile_t * file
					= AppendSubFileList(&szs->ext_data,it->path,false);
				DASSERT(file);
				file->load_path = (ccp)bmg.raw_data;
				file->size      = bmg.raw_data_size;
				it->off += bmg.raw_data - data;
				it->size = bmg.raw_data_size;
				bmg.raw_data = 0;
				*dirty = 1;
			    }
			    else
				ERROR0(ERR_WARNING,
					"Can't patch BMG because of size [old=%u.new=%u]: %s\n",
					it->size, bmg.raw_data_size, it->name );
			}
		    }
		    ResetBMG(&bmg);
		}
		break;

	    case FF_KCL:
		if (!PatchFileClass(FF_KCL,it->name))
		{
		    noPRINT("### Patch/NOT KCL: %s\n",it->name);
		    break;
		}

		PRINT("### Patch KCL: %s/%s\n",szs->fname,it->name);
		if ( szs->allow_ext_data && !(KCL_MODE&KCLMD_INPLACE) )
		{
		    kcl_t kcl;
		    InitializeKCL(&kcl);
		    kcl.fform_outfile = FF_KCL;
		    enumError err = ScanRawKCL(&kcl,false,data,it->size,true);
		    kcl.fast = false;

		    if ( !err
			&& PatchKCL(&kcl)
			&& CreateRawKCL(&kcl,false) == ERR_OK )
		    {
			PATCH_ACTION_LOG("Patch","KCL","%s\n",it->name);
			if ( it->size == kcl.raw_data_size )
			{
			    KCL_ACTION_LOG("Overwrite KCL, size %u.\n",it->size);
			    memcpy(data,kcl.raw_data,it->size);
			}
			else
			{
			    KCL_ACTION_LOG("Replace KCL, size %u -> %u.\n",
						it->size, kcl.raw_data_size );
			    szs_subfile_t * file
				    = AppendSubFileList(&szs->ext_data,it->path,false);
			    DASSERT(file);
			    if (kcl.raw_data_alloced)
			    {
				file->load_path = (ccp)kcl.raw_data;
				kcl.raw_data = 0;
				kcl.raw_data_alloced = 0;
			    }
			    else
				file->load_path = MEMDUP(kcl.raw_data,kcl.raw_data_size);
			    it->off += (u8*)file->load_path - data;
			    it->size = file->size = kcl.raw_data_size;
			}
			*dirty = 1;
		    }
		    ResetKCL(&kcl);
		}
		else if (PatchRawDataKCL(data,it->size,it->name))
		{
		    PATCH_ACTION_LOG("Patch","KCL/RAW","%s\n",it->name);
		    *dirty = 1;
		}
		break;


	    case FF_KMP:
		if (!PatchFileClass(FF_KMP,it->name))
		{
		    noPRINT("### Patch/NOT KMP: %s\n",it->name);
		    break;
		}

		PRINT("### Patch KMP: %s\n",it->name);
		if ( szs->allow_ext_data && !(KMP_MODE&KMPMD_INPLACE) )
		{
		    kmp_t kmp;
		    enumError err = ScanRawKMP(&kmp,true,data,it->size);
		    if ( !err
			&& PatchKMP(&kmp)
			&& CreateRawKMP(&kmp) == ERR_OK )
		    {
			PATCH_ACTION_LOG("Patch","KMP","%s\n",it->name);
			if ( it->size == kmp.raw_data_size )
			{
			    KCL_ACTION_LOG("Overwrite KMP, size %u.\n",it->size);
			    memcpy(data,kmp.raw_data,it->size);
			}
			else
			{
			    KMP_ACTION_LOG(false,"Replace KMP, size %u -> %u.\n",
						it->size, kmp.raw_data_size );

			    szs_subfile_t * file
				    = AppendSubFileList(&szs->ext_data,it->path,false);
			    DASSERT(file);
			    file->load_path = (ccp)kmp.raw_data;
			    file->size      = kmp.raw_data_size;
			    it->off += kmp.raw_data - data;
			    it->size = kmp.raw_data_size;
			    kmp.raw_data = 0;
			}
			*dirty = 1;
		    }
		    ResetKMP(&kmp);
		}
		else if (PatchRawDataKMP(data,it->size))
		{
		    PATCH_ACTION_LOG("Patch","KMP/RAW","%s\n",it->name);
		    *dirty = 1;
		}
		break;

	    case FF_LEX:
		if ( !szs->allow_ext_data || !HavePatchTestLEX())
		{
		    PRINT("### Patch/NOT LEX: %s\n",it->name);
		    break;
		}

		PRINT("### Patch LEX: %s/%s\n",szs->fname,it->name);
		{
		    lex_t lex;
		    enumError err = ScanRawLEX(&lex,true,data,it->size,0);
		    if ( !err && PatchLEX(&lex) && CreateRawLEX(&lex) == ERR_OK )
		    {
			if (!lex.have_lex)
			{
			    LEX_ACTION_LOG(false,"Remove LEX, old_size %u.\n",it->size);

			    szs_subfile_t * file
				= AppendSubFileList(&szs->ext_data,it->path,false);
			    DASSERT(file);
			    file->removed = true;
			    it->off  = 0;
			    it->size = 0;
			}
			else if ( it->size == lex.raw_data_size )
			{
			    LEX_ACTION_LOG(false,"Overwrite LEX, size %u.\n",it->size);
			    memcpy(data,lex.raw_data,it->size);
			}
			else
			{
			    LEX_ACTION_LOG(false,"Replace LEX, size %u -> %u.\n",
						it->size, lex.raw_data_size );

			    szs_subfile_t * file
				= AppendSubFileList(&szs->ext_data,it->path,false);
			    DASSERT(file);
			    file->load_path = (ccp)lex.raw_data;
			    file->size      = lex.raw_data_size;
			    it->off += lex.raw_data - data;
			    it->size = lex.raw_data_size;
			    lex.raw_data = 0;
			}
			*dirty = 1;
		    }
		    PRINT("PATCH/LEX: modified=%d, have_lex:%x\n",
				lex.modified, lex.have_lex );
		    ResetLEX(&lex);
		}
		break;

	    case FF_BRRES:
		szs->brres_type = GetFileClass(FF_BRRES,it->name);
		szs->dont_patch_mdl = !( szs->brres_type & PATCH_FILE_MODE )
				   || disable_patch_on_load > 0
				   || !HavePatchMDL();

		noPRINT("### Patch%s BRRES: %s [%x]\n",
			szs->dont_patch_mdl ? "/NOT" : "", it->name,
			szs->brres_type );
		if (!szs->dont_patch_mdl)
		    PATCH_ACTION_LOG("Enable","BRRES","%s\n",it->name);
		break;


	    case FF_MDL:
		if ( szs->fform_arch == FF_BRRES && szs->dont_patch_mdl
		    || disable_patch_on_load > 0
		    || !HavePatchMDL() )
		{
		    noPRINT("### Patch/NOT MDL: %s\n",it->name);
		    break;
		}

		{
		    noPRINT("### Patch MDL: %s [%x]\n",it->name,szs->brres_type);
		    const uint stat
			= PatchRawDataMDL(data,it->size,szs->brres_type,
				szs && szs->fname && *szs->fname ? szs->fname : it->name );
		    if (stat)
		    {
			PATCH_ACTION_LOG(" Patch","MDL/RAW","%s%s\n",
				it->name,
				stat&2 ? " (vector transformation recognized)" : "" );
			*dirty = 1;
		    }
		}
		break;

	    default:
		break;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool PatchSZS ( szs_file_t * szs )
{
    DASSERT(szs);
    const bool patch_lex = HaveActivePatchLEX();
 #ifdef TEST
    static int done = 0;
    if (!done++)
    {
	printf("PATCH-COUNT: *=%d, bmg=%d, kcl=%d, kmp=%d, mdl=%d, lex=%d\n",
		have_patch_count,
		have_bmg_patch_count,
		have_kcl_patch_count,
		have_kmp_patch_count,
		have_mdl_patch_count,
		patch_lex );
    }
 #endif

    int dirty = 0;
    if ( szs->data && szs->size )
    {
	if (patch_lex)
	    dirty |= NormalizeExSZS(szs,0,0,0);

	if ( have_patch_count > 0 || opt_lex_purge )
	{
	    PRINT("** PatchSZS() **\n");
	    ScanTformBegin();
	    IterateFilesSZS(szs,transform_collect_func,&dirty,false,-1,-1,SORT_NONE);
	    TformScriptEnd();
	    ClearSpecialFilesSZS(szs);
	}
    }

    return dirty != 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SetupExtendedSZS		///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupExtendedSZS()
{
    static bool done = false;
    if (done)
	return;
    done = true;

    SetupStandardSZS();

    cut_iter_func[FF_CT1_DATA]	= IterateFilesCTCODE;
    cut_iter_func[FF_BMG]	= IterateFilesBMG;
    cut_iter_func[FF_BREFT_IMG]	= IterateFilesBREFT_IMG;
    cut_iter_func[FF_KCL]	= IterateFilesKCL;
    cut_iter_func[FF_KMP]	= IterateFilesKMP;
//    cut_iter_func[FF_LEX]	= IterateFilesLEX; // [[lex]]
    cut_iter_func[FF_MDL]	= IterateFilesMDL;
    cut_iter_func[FF_PAT]	= IterateFilesPAT;
    cut_iter_func[FF_TEX]	= IterateFilesTEX;
    cut_iter_func[FF_TEX_CT]	= IterateFilesTEX;
    cut_iter_func[FF_TPL]	= IterateFilesTPL;
    cut_iter_func[FF_BTI]	= IterateFilesBTI;

  //std_iter_func[FF_TEX_CT]	= IterateFilesTEX;
  //std_iter_func[FF_CT1_DATA]	= IterateFilesCTCODE;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  CheckSZS			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[check_szs_t]]

typedef struct check_szs_t
{
    //--- param

    szs_file_t		* szs;		// pointer to valid SZS
    CheckMode_t		mode;		// print mode

    //--- status

    int			warn_count;	// number of warnings
    int			hint_count;	// number of hints
    int			info_count;	// number of hints
    bool		head_printed;	// true: header line printed

    //--- misc data

    ColorSet_t		col;		// color setup
    uint		img_count;	// number of checked images

    //--- vertex counter

    uint		*num_vertex;	// NULL or pointer to vertex counter
    uint		*num_mdl;	// NULL or pointer to MDL counter

    uint		nv_model;	// number of vert. in 'course_model.brres'
    uint		nm_model;	// number of MDL's in 'course_model.brres'
    uint		nv_map;		// number of vert. in 'map_model.brres'
    uint		nm_map;		// number of MDL's in 'map_model.brres'
    uint		nv_vrcorn;	// number of vert. in 'vrcorn_model.brres'
    uint		nm_vrcorn;	// number of MDL's in 'vrcorn_model.brres'

    uint		nv_total;	// total vertex count
    uint		nm_total;	// total number of scanned MDL files

} check_szs_t;

///////////////////////////////////////////////////////////////////////////////

// predefine function to avoid a warning because of __attribute__ usage
static void print_check_error
(
    check_szs_t		* chk,		// valid data structure
    CheckMode_t		print_mode,	// NULL | CMOD_WARN|CMOD_HINT|CMOD_SLOT|CMOD_INFO
    ccp			format,		// format of message
    ...					// arguments

) __attribute__ ((__format__(__printf__,3,4)));

static void print_check_error
(
    check_szs_t		* chk,		// valid data structure
    CheckMode_t		print_mode,	// NULL | CMOD_WARN|CMOD_HINT|CMOD_SLOT|CMOD_INFO
    ccp			format,		// format of message
    ...					// arguments

)
{
    DASSERT(chk);
    DASSERT(chk->szs);

    ccp col; // message color
    ccp bol; // begin of line
    switch (print_mode)
    {
	case CMOD_WARNING:
	    if (!( chk->mode & CMOD_WARNING ))
		return;
	    col = chk->col.warn;
	    bol = check_bowl;
	    chk->warn_count++;
	    break;

	case CMOD_HINT:
	    if (!( chk->mode & CMOD_HINT ))
		return;
	    col = chk->col.hint;
	    bol = check_bohl;
	    chk->hint_count++;
	    break;

	case CMOD_SLOT:
	    if (!( chk->mode & CMOD_SLOT ))
		return;
	    col = chk->col.info;
	    bol = check_bosl;
	    chk->info_count++;
	    break;

	case CMOD_INFO:
	    if (!( chk->mode & CMOD_INFO ))
		return;
	    col = chk->col.info;
	    bol = check_boil;
	    chk->info_count++;
	    break;

	default:
	    col = 0;
	    bol = 0; // print head only
	    break;
    }

    if (!chk->head_printed)
    {
	chk->head_printed = true;
	if (chk->mode & (CMOD_HEADER|CMOD_FORCE_HEADER) )
	    fprintf(stdlog,"%s* CHECK %s:%s%s\n",
			chk->col.heading,
			GetNameFF(chk->szs->fform_file,chk->szs->fform_arch),
			chk->szs->fname, chk->col.reset );
    }

    if ( bol && format )
    {
	fputs(col,stdlog);
	fputs(bol,stdlog);
	fputs(chk->col.reset,stdlog);

	va_list arg;
	va_start(arg,format);
	vfprintf(stdlog,format,arg);
	va_end(arg);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void print_vertex_count
(
    check_szs_t	*chk,
    uint	vert_counter,
    uint	mdl_counter,
    uint	hint_level,
    uint	warn_level,
    ccp		model_name
)
{
    DASSERT(chk);

    if (vert_counter)
    {
	CheckMode_t mode = vert_counter >= warn_level
		? CMOD_WARNING : vert_counter >= hint_level
		? CMOD_HINT : CMOD_INFO;

	if (model_name)
	    print_check_error( chk, mode,
		"File '%s_model.brres' has %u vertices in %u MDL file%s.\n",
		model_name, vert_counter, mdl_counter,
		mdl_counter == 1 ? "" : "s" );
	else
	    print_check_error( chk, mode,
		"Total of %u vertices in %u MDL file%s found.\n",
		vert_counter, mdl_counter,
		mdl_counter == 1 ? "" : "s" );
    }
}

///////////////////////////////////////////////////////////////////////////////

static int check_mdl_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if ( term || it->is_dir )
	return 0;

    if ( it->group == 2 && it->entry >= 0 )
    {
	check_szs_t *chk = it->param;
	DASSERT(chk);

	mdl_sect2_t *ms = (mdl_sect2_t*)( it->szs->data + it->off );
	const u32 nv = be16(&ms->n_vertex);
	chk->nv_total += nv;
	if (chk->num_vertex)
	    *chk->num_vertex += nv;
	noPRINT("MDL group %d, entry %d, N=%u: %s\n",
		it->group, it->entry, nv, it->path );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int check_brres_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if ( term || it->is_dir )
	return 0;

    const u8 *data = it->szs->data + it->off;
    const file_format_t fform = GetByMagicFF(data,it->size,it->size);
    if ( fform == FF_BRRES )
    {
	check_szs_t *chk = it->param;
	DASSERT(chk);

	szs_file_t subszs;
	InitializeSubSZS(&subszs,it->szs,it->off,it->size,fform,it->path,false);
	subszs.fname = it->path;
	CheckBRRES( &subszs, chk->mode | CMOD_FORCE_HEADER, chk );
	subszs.fname = 0;
	ResetSZS(&subszs);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int check_file_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if ( term || it->is_dir )
	return 0;

    const u8 *data = it->szs->data+it->off;
// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,it->size,it->size);
    check_szs_t *chk = it->param;
    DASSERT(chk);

    switch(fform)
    {
     case FF_BRRES:
	switch (GetFileClass(FF_BRRES,it->path))
	{
	    case PFILE_MODEL:
		chk->num_vertex	= &chk->nv_model;
		chk->num_mdl	= &chk->nm_model;
		break;

	    case PFILE_MINIMAP:
		chk->num_vertex	= &chk->nv_map;
		chk->num_mdl	= &chk->nm_map;
		break;

	    case PFILE_VRCORN:
		chk->num_vertex	= &chk->nv_vrcorn;
		chk->num_mdl	= &chk->nm_vrcorn;
		break;

	    default:
		chk->num_vertex	= 0;
		chk->num_mdl	= 0;
		break;

	}
	noPRINT("### %d %s %s\n",it->parent_it!=0,GetNameFF(fform,0),it->path);
	break;

     case FF_MDL:
	{
	    noPRINT("### %d %s %s\n",it->parent_it!=0,GetNameFF(fform,0),it->path);
	    chk->nm_total++;
	    if (chk->num_mdl)
		chk->num_mdl[0]++;
	    szs_file_t subszs;
// [[fname+]]
	    InitializeSubSZS(&subszs,it->szs,it->off,it->size,fform,it->path,false);
	    subszs.fname = it->path;
	    IterateFilesSZS(&subszs,check_mdl_func,chk,true,0,1,SORT_NONE);
	    subszs.fname = 0;
	    ResetSZS(&subszs);
	}
	break;

     default:
	break;
    }

    Image_t img;
    ScanDataIMG(&img,true,data,it->size);
    const ImageGeometry_t *geo = GetImageGeometry(img.iform);

    if (geo)
    {
	chk->img_count++;

	if ( !img.width || !img.height )
	{
	    print_check_error( chk, CMOD_WARNING,
		"%s %ux%u: Illegal image size: %s/%s\n",
		PrintFormat3(img.info_fform,img.iform,img.pform),
		img.width, img.height,
		it->parent_it ? it->parent_it->path : ".",
		it->path );
	    goto exit;
	}

	uint wd, ht;
	for ( wd = 1; !(img.width&wd); wd <<= 1 )
	    ;
	for ( ht = 1; !(img.height&ht); ht <<= 1 )
	    ;
	if ( wd != img.width || ht != img.height )
	{
	    print_check_error( chk, CMOD_WARNING,
		"%s %ux%u: Image size not power of 2: %s/%s\n",
		PrintFormat3(img.info_fform,img.iform,img.pform),
		img.width, img.height,
		it->parent_it ? it->parent_it->path : ".",
		it->path );
	    goto exit;
	}

	if ( img.width > 1024 || img.height > 1024 )
	{
	    print_check_error( chk, CMOD_HINT,
		"%s %ux%u: Large image found: %s/%s\n",
		PrintFormat3(img.info_fform,img.iform,img.pform),
		img.width, img.height,
		it->parent_it ? it->parent_it->path : ".",
		it->path );
	    goto exit;
	}
    }

 exit:
    ResetIMG(&img);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static void PrintCheckSummarySZS ( const check_szs_t *chk )
{
    DASSERT(chk);
    DASSERT(chk->szs);

    if ( chk->mode & (CMOD_FOOTER|CMOD_FORCE_FOOTER) )
    {
	ccp bind;
	char hbuf[60], ibuf[60];
	if (chk->info_count)
	{
	    snprintf(ibuf,sizeof(ibuf)," and %s%u info%s%s",
		chk->col.info,
		chk->info_count, chk->info_count>1 ? "s" : "",
		chk->col.heading );
	    bind = ",";
	}
	else
	{
	    *ibuf = 0;
	    bind = " and";
	}

	if (chk->hint_count)
	    snprintf(hbuf,sizeof(hbuf),"%s %s%u hint%s%s",
		bind, chk->col.hint,
		chk->hint_count, chk->hint_count>1 ? "s" : "",
		chk->col.heading );
	else
	    *hbuf = 0;

	if (chk->warn_count)
	    fprintf(stdlog,
			" %s=> %s%u warning%s%s%s%s for %s:%s%s\n\n",
			chk->col.heading, chk->col.warn,
			chk->warn_count, chk->warn_count == 1 ? "" : "s",
			chk->col.heading,
			hbuf, ibuf, GetNameFF_SZS(chk->szs),
			chk->szs->fname, chk->col.reset );
	else if ( chk->mode & CMOD_FORCE_FOOTER || chk->hint_count )
	    fprintf(stdlog," %s=> no warnings%s%s for %s:%s%s\n\n",
			chk->col.heading,
			hbuf, ibuf, GetNameFF_SZS(chk->szs),
			chk->szs->fname, chk->col.reset );
    }
}

///////////////////////////////////////////////////////////////////////////////

int CheckSZS
(
    // returns number of errors

    szs_file_t		* szs,		// valid szs
    CheckMode_t		szs_mode,	// print mode for szs file
    CheckMode_t		sub_mode	// print mode for sub file (KCL,KMP,...)
)
{
    DASSERT(szs);
    PRINT("CheckSZS(%d,%d)\n",szs_mode,sub_mode);
    szs_mode = GetMainCheckMode(szs_mode,true);
    sub_mode = GetSubCheckMode(sub_mode,true);


    //--- setup

    check_szs_t chk;
    memset(&chk,0,sizeof(chk));
    SetupColorSet(&chk.col,stdlog);
    chk.szs  = szs;
    chk.mode = szs_mode;

    enum
    {
	F_REQUIRED	=    1,
	F_REQUIRED2	=    2,
	F_OPTIONAL	=    4,
	F_IS_OBJECT	=    8,
	F_FOUND		= 0x10,
	F_MODIFIED	= 0x20,

	F_M_ADDITIONAL	= F_REQUIRED | F_REQUIRED2 | F_OPTIONAL | F_FOUND,
	F_M_OPTIONAL	= F_OPTIONAL | F_FOUND,
    };


    //--- check BRRES + BRSUB

    IterateFilesSZS(szs,check_brres_func,&chk,false,-1,0,SORT_AUTO);


    //--- find KCL

    FindSpecialFilesSZS(szs,false);

    int max_cannon = -1;
    bool need_ice_brres = false;
    bool kcl_valid = false;
    kcl_t kcl;

    InitializeKCL(&kcl);
    if ( szs->course_kcl_data )
    {
	kcl.fname = STRDUP2(szs->fname,"/course.kcl");
	kcl_valid = !ScanKCL( &kcl, false, szs->course_kcl_data,
			szs->course_kcl_size, true, sub_mode);

	if (kcl_valid)
	{
	    //---- calc statistics

	    uint n = kcl.tridata.used;
	    kcl_tridata_t *td = (kcl_tridata_t*)kcl.tridata.list;
	    for ( ; n-- > 0; td++ )
	    {
		if ( td->cur_flag == 0x70 )
		{
		    need_ice_brres = true;
		}
		else if (IS_KCL_TYPE(td->cur_flag,0x11))
		{
		    const int idx = td->cur_flag >> 5;
		    if ( max_cannon < idx )
			 max_cannon = idx;
		}
	    }
	    max_cannon++;
	}
    }


    //--- find LEX

    lex_info_t lexinfo;
    if (szs->course_lex_data)
	SetupLexInfoByData(&lexinfo,szs->course_lex_data,szs->course_lex_size);
    else
	InitializeLexInfo(&lexinfo);


    //--- find KMP

    enumError err = ERR_WARNING;
    if (szs->course_kmp_data)
	err = LoadObjFileListSZS( szs, szs->course_kmp_data,
				szs->course_kmp_size, 0, false, sub_mode, &lexinfo );

    //--- find MINIMAP

    uint minimap_stat = 0;
    if (szs->map_model_data)
    {
	minimap_stat = 4;

	szs_file_t mapszs;
// [[fname-]]
	InitializeSubSZS( &mapszs, szs, szs->map_model_data - szs->data,
				szs->map_model_size, FF_UNKNOWN, 0, false );
	mdl_minimap_t mmap;
	FindMinimapData(&mmap,&mapszs);
	if (mmap.posLD)
	    minimap_stat |= 1;
	if (mmap.posRU)
	    minimap_stat |= 2;
	ResetSZS(&mapszs);
    }


    //--- term on error

    if ( err || !szs->used_file )
	return -1;


    //--- add global statistics

    if ( global_warn_count || global_hint_count || szs_mode & CMOD_FORCE_HEADER )
    {
	chk.mode |= CMOD_FORCE_HEADER;
	print_check_error(&chk,0,0);
    }

    chk.warn_count += global_warn_count;
    chk.hint_count += global_hint_count;
    chk.info_count += global_info_count;


    //--- test unknown files

    DbFlags_t found_flags = 0;
    szs_subfile_t *file, *file_end = szs->subfile.list + szs->subfile.used;
    for ( file = szs->subfile.list; file < file_end; file++ )
    {
	if (file->is_dir)
	    continue;

	ccp path = file->path;
	if ( path[0] == '.' && path[1] == '/' )
	    path += 2;
	int fidx = FindDbFile(path);
	if ( fidx < 0 )
	{
	    bool found = false;
	    int i;
	    for ( i = 0; i < HAVESZS__N; i++ )
		if (strcasecmp(path,have_szs_file[i]))
		{
		    found = true;
		    break;
		}

	    if (!found)
		print_check_error( &chk, CMOD_HINT,
			"Unknown file:    ./%s\n", path );
	}
	else
	{
	    u8 *flags = szs->used_file->d + fidx;
	    *flags |= F_FOUND;

	    const DbFileFILE_t *ptr = DbFileFILE + fidx;
	    found_flags |= ptr->flags;

	    if (IsFileOptionalSZS(szs,ptr))
		*flags |= F_OPTIONAL;
	    if ( ptr->flags & DBF_REQUIRED )
		*flags |= F_REQUIRED;
	    if ( ptr->flags & DBF_REQUIRED2 )
		*flags |= F_REQUIRED2;

	    if (DBF_ARCH_SUPPORT(ptr->flags))
	    {
		*flags |= F_IS_OBJECT;

		sha1_hash_t hash;
		SHA1( szs->data+file->offset, file->size, hash );

		bool found = false;
		const s16 *ref = DbFileRefFILE + ptr->ref;
		while ( !found && *ref >= 0 )
		{
		    DASSERT( *ref < N_DB_FILE );
		    const DbFile_t *db = DbFile + *ref++;
		    DASSERT( db->sha1 < N_DB_FILE_SHA1 );
		    found = !memcmp(hash,DbFileSHA1[db->sha1].sha1,sizeof(hash));
		}
		if (!found)
		    *flags |= F_MODIFIED;
	    }
	}
    }


    if ( chk.mode & CMOD_HINT )
    {
	//--- test special files

	int i;
	for ( i = 0; i < HAVESZS__N; i++ )
	    if (szs->szs_special[i])
		print_check_error( &chk, CMOD_HINT,
				"Special file:    ./%s\n",have_szs_file[i]);

	//--- test optional files

	if (need_ice_brres)
	{
	    int i = FindDbFile("ice.brres");
	    if ( i > 0 )
		szs->used_file->d[i] = szs->used_file->d[i] & ~F_OPTIONAL | F_REQUIRED;
	}

	for ( i = 0; i < N_DB_FILE_FILE; i++ )
	{
	    if ( ( szs->used_file->d[i] & F_M_OPTIONAL ) == F_M_OPTIONAL )
	    {
		const DbFileFILE_t *ptr = DbFileFILE + i;
	     #ifdef TEST0
		print_check_error( &chk, CMOD_HINT,
				"Optional file [%02x]:   ./%s\n",
				szs->used_file->d[i], ptr->file );
	     #else
		print_check_error( &chk, CMOD_HINT,
				"Optional file:   ./%s\n", ptr->file );
	     #endif
	    }
	}

	//--- test additional files

	for ( i = 0; i < N_DB_FILE_FILE; i++ )
	    if ( ( szs->used_file->d[i] & F_M_ADDITIONAL ) == F_FOUND )
	    {
		const DbFileFILE_t *ptr = DbFileFILE + i;
	     #ifdef TEST0
		print_check_error( &chk, CMOD_HINT,
				"Additional file [%02x]: ./%s\n",
				szs->used_file->d[i], ptr->file );
	     #else
		print_check_error( &chk, CMOD_HINT,
				"Additional file: ./%s\n", ptr->file );
	     #endif
	    }

	//--- test modified files

	const DbFileFILE_t *ptr = DbFileFILE;
	for ( i = 0; i < N_DB_FILE_FILE; i++, ptr++ )
	    if ( szs->used_file->d[i] & F_MODIFIED )
		print_check_error( &chk, CMOD_HINT,
				"Modified file:   ./%s\n", ptr->file );
    }


    //--- test missing files

    if ( !(found_flags & DBF_REQUIRED2) )
	print_check_error( &chk, CMOD_WARNING,
		"Missing file: ./course_model.brres (or '_d' variant)\n" );

    int i;
    const DbFileFILE_t *ptr = DbFileFILE;
    for ( i = 0; i < N_DB_FILE_FILE; i++, ptr++ )
    {
	const u8 flag = szs->used_file->d[i];
	if ( flag & F_FOUND )
	    continue;
	if ( flag & F_REQUIRED || ptr->flags & DBF_REQUIRED )
	{
	    if (!IsFileOptionalSZS(szs,ptr))
		print_check_error( &chk, CMOD_WARNING,
				"Missing file: ./%s\n", ptr->file );
	}
    }

    if (szs->required_file)
    {
	ParamFieldItem_t *ptr = szs->required_file->field, *end;
	for ( end = ptr + szs->required_file->used; ptr < end; ptr++ )
	    if (!ptr->num)
		print_check_error( &chk, CMOD_WARNING,
				"Missing unusual file: ./%s\n", ptr->key );
    }



    //--- check images

    IterateFilesSZS(szs,check_file_func,&chk,false,-1,0,SORT_AUTO);
    if (chk.img_count)
	print_check_error( &chk, CMOD_INFO,
		"Geometry of %u images checked.\n",chk.img_count);


    //--- check number of vertices

    print_vertex_count( &chk, chk.nv_model,	chk.nm_model,	20000, 50000, "course");
    print_vertex_count( &chk, chk.nv_map,	chk.nm_map,	 2500,  5000, "map");
    print_vertex_count( &chk, chk.nv_vrcorn,	chk.nm_vrcorn,	  300,  1000, "vrcorn");
    print_vertex_count( &chk, chk.nv_total,	chk.nm_total,	25000, 60000, 0);


    //--- compare KMP and KCL data

    if (kcl_valid)
    {
	//--- check cannons

	if ( szs->n_cannon >= 0 )
	{
	    PRINT("N-CANNON: kmp=%d, kcl=%d\n",szs->n_cannon,max_cannon);
	    if ( max_cannon > szs->n_cannon )
		print_check_error( &chk, CMOD_WARNING,
			    "KMP defines only %u cannon%s, but KCL needs %u.\n",
			    szs->n_cannon, szs->n_cannon == 1 ? "" : "s",
			    max_cannon );
	    else if ( max_cannon < szs->n_cannon )
		print_check_error( &chk, CMOD_HINT,
			    "KMP defines %u cannon%s, but KCL needs only %u.\n",
			    szs->n_cannon, szs->n_cannon == 1 ? "" : "s",
			    max_cannon );
	}


	//--- check slot 6.1 and ice.brres

	if (need_ice_brres)
	{
	    if (szs->have_ice_brres)
		print_check_error( &chk, CMOD_SLOT,
			"Track will only run at slot 6.1 (sherbet),"
			" because KCL uses 'ice.brres'.\n" );
	    else
		print_check_error( &chk, CMOD_WARNING,
			"KCL need missing file 'ice.brres'"
			" ==> Wii may freeze on fall down.\n" );
	}
	else if (szs->have_ice_brres)
	    print_check_error( &chk, CMOD_SLOT,
			"Track will also run at slot 6.1 (sherbet).\n" );
	else if ( szs->slot_analyzed && szs_mode & CMOD_VERBOSE )
	    print_check_error( &chk, CMOD_SLOT,
			"Track will not run at slot 6.1 (sherbet).\n" );
    }
    ResetKCL(&kcl);


    //--- minimap status

    noPRINT("MINIMAP STATUS: %x\n",minimap_stat);
    switch (minimap_stat)
    {
	case 4:
	    print_check_error( &chk, CMOD_WARNING,
		"Missing bones 'posLD' and 'posRU' in minimap MDL.\n" );
	    break;

	case 5:
	    print_check_error( &chk, CMOD_WARNING,
		"Missing bone 'posRU' in minimap MDL.\n" );
	    break;

	case 6:
	    print_check_error( &chk, CMOD_WARNING,
		"Missing bone 'posLD' in minimap MDL.\n" );
	    break;
    }


    //--- slot 3.1 and 7.1

    if (szs->slot_31_71)
    {
	if ( szs->slot_31_71 == (SLOT_31_71_SUNDS|SLOT_31_71_PYLON01) )
	    print_check_error( &chk, CMOD_SLOT,
		"Track will only run at slots 3.1 and 7.1,"
		" because objects 'sunDS' and 'pylon01' are used.\n" );
	else
	    print_check_error( &chk, CMOD_SLOT,
		"Track will only run at slots 3.1 and 7.1,"
		" because object '%s' is used.\n",
		szs->slot_31_71 &  SLOT_31_71_SUNDS ? "sunDS" : "pylon01" );
    }


    //--- slot 4.2

    if ( szs->moonview_mdl_stat < 3 )
    {
	if ( szs->slot_analyzed && szs_mode & CMOD_VERBOSE )
	{
	    if ( szs->moonview_mdl_stat == 2 )
		print_check_error( &chk, CMOD_SLOT,
			"Track most likely run at slot 4.2."
			" All MDL materials found, but some are modified.\n" );
	    else if ( szs->moonview_mdl_stat == 1 )
		print_check_error( &chk, CMOD_SLOT,
			"Track will not run at slot 4.2,"
			" because of some missed MDL materials.\n" );
	    else
		print_check_error( &chk, CMOD_SLOT,
			"Track will not run at slot 4.2,"
			" because no needed MDL material was found.\n" );

	    if ( !(szs->slot_42 & SLOT_42_ALL_PF) )
		print_check_error( &chk, CMOD_SLOT,
			"... Also objects 'car_body' and/or 'kart_truck' aren't used.\n" );
	}
	else if ( szs->moonview_mdl_stat == 2 && szs->slot_42 & SLOT_42_ALL_PF )
	    print_check_error( &chk, CMOD_SLOT,
		"Track most likely run at slot 4.2."
		" All MDL materials found, but some are modified.\n" );
    }
    else if ( szs->slot_42 & SLOT_42_ALL_PF )
    {
	print_check_error( &chk, CMOD_SLOT,
		"Track will run at slot 4.2,"
		" because car+truck and all needed MDL materials are defined.\n" );
    }
    else if ( szs->slot_42 & SLOT_42_ALL_U )
    {
	print_check_error( &chk, CMOD_SLOT,
		"Track may run at slot 4.2,"
		" because car+truck and all needed MDL materials are defined.\n" );
	print_check_error( &chk, CMOD_SLOT,
		"... However, the presence flags are not set to 0x3f"
		" to support all variants.\n" );
    }
    else if ( szs->slot_analyzed && szs_mode & CMOD_VERBOSE )
    {
	if ( szs->slot_42 & SLOT_42_KART_TRUCK_U )
	    print_check_error( &chk, CMOD_SLOT,
		"Track will not run at slot 4.2,"
		" because object 'car_body' isn't used.\n" );
	else if ( szs->slot_42 & SLOT_42_CAR_BODY_U )
	    print_check_error( &chk, CMOD_SLOT,
		"Track will not run at slot 4.2,"
		" because object 'kart_truck' isn't used.\n" );
	else
	    print_check_error( &chk, CMOD_SLOT,
		"Track will not run at slot 4.2,"
		" because objects 'car_body' and 'kart_truck' aren't used.\n" );
    }


    //--- slot 6.2

    if (szs->slot_62)
	print_check_error( &chk, CMOD_SLOT,
		"Track will only run at slot 6.2,"
		" because object 'HeyhoShipGBA' is used.\n" );
    else if ( szs->slot_analyzed && szs_mode & CMOD_VERBOSE )
	print_check_error( &chk, CMOD_SLOT,
		"Track will not run at slot 6.2,"
		" because object 'HeyhoShipGBA' isn't used.\n" );


    //--- terminate

#if 1
    PrintCheckSummarySZS(&chk);
#else

    if ( szs_mode & (CMOD_FOOTER|CMOD_FORCE_FOOTER) )
    {
	ccp bind;
	char hbuf[60], ibuf[60];
	if (chk.info_count)
	{
	    snprintf(ibuf,sizeof(ibuf)," and %s%u info%s%s",
		chk.col.info,
		chk.info_count, chk.info_count>1 ? "s" : "",
		chk.col.heading );
	    bind = ",";
	}
	else
	{
	    *ibuf = 0;
	    bind = " and";
	}

	if (chk.hint_count)
	    snprintf(hbuf,sizeof(hbuf),"%s %s%u hint%s%s",
		bind, chk.col.hint,
		chk.hint_count, chk.hint_count>1 ? "s" : "",
		chk.col.heading );
	else
	    *hbuf = 0;

	if (chk.warn_count)
	    fprintf(stdlog,
			" %s=> %s%u warning%s%s%s%s for %s:%s%s\n\n",
			chk.col.heading, chk.col.warn,
			chk.warn_count, chk.warn_count == 1 ? "" : "s",
			chk.col.heading,
			hbuf, ibuf, GetNameFF(szs->fform_file,szs->fform_arch),
			szs->fname, chk.col.reset );
	else if ( szs_mode & CMOD_FORCE_FOOTER || chk.hint_count )
	    fprintf(stdlog," %s=> no warnings%s%s for %s:%s%s\n\n",
			chk.col.heading,
			hbuf, ibuf, GetNameFF(szs->fform_file,szs->fform_arch),
			szs->fname, chk.col.reset );
    }
#endif

    return chk.warn_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  CheckBRRES			///////////////
///////////////////////////////////////////////////////////////////////////////

static int check_brres
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    if ( term || it->size < 0x0c )
	return 0;

    u8 *data = it->szs->data + it->off;
    const file_format_t fform = GetByMagicFF(data,it->size,it->size);
    if (IsBRSUB(fform))
    {
	check_szs_t *chk = (check_szs_t*)it->param;
	DASSERT(chk);

	const int version	= it->endian->rd32(data+8);
	const brsub_info_t *bi	= GetInfoBRSUB(fform,version);
	const int ns		= GetSectionNumBRSUB(data,it->size,it->endian);

	if ( !bi || bi->warn >= BIMD_FATAL || ns != bi->good_sect )
	     print_check_error( chk, CMOD_WARNING,
				"%s v%d (%d section%s) will freeze track: %s\n",
				GetNameFF(0,fform), version,
				ns, ns == 1 ? "" : "s", it->path );
	else if ( bi->warn >= BIMD_FAIL )
	     print_check_error( chk, CMOD_WARNING,
				"%s v%d (%d section%s) is not displayed correctly: %s\n",
				GetNameFF(0,fform), version,
				ns, ns == 1 ? "" : "s", it->path );
	else if ( bi->warn >= BIMD_HINT )
	     print_check_error( chk, CMOD_HINT,
				"Unusual %s version %d: %s\n",
				GetNameFF(0,fform), version, it->path );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int CheckBRRES
(
    // returns number of errors

    szs_file_t		* szs,		// valid szs
    CheckMode_t		check_mode,	// print mode
    check_szs_t		* parent_chk	// NULL or parent
)
{
    DASSERT(szs);
    PRINT("CheckBRRES(0x%x)\n",check_mode);

    check_szs_t chk;
    memset(&chk,0,sizeof(chk));
    SetupColorSet(&chk.col,stdlog);
    chk.szs  = szs;
    chk.mode = check_mode;
    IterateFilesSZS(szs,check_brres,&chk,false,0,-1,SORT_NONE);

    if (parent_chk)
    {
	parent_chk->warn_count += chk.warn_count;
	parent_chk->hint_count += chk.hint_count;
	parent_chk->info_count += chk.info_count;
    }
    else
	PrintCheckSummarySZS(&chk);

    return chk.warn_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  FindSlotsSZS			///////////////
///////////////////////////////////////////////////////////////////////////////

#undef SLOT
#undef ISLOT

#define SLOT(a,b) slot[a*4+b-5]
#define ISLOT(a,b) (a*4+b-5)

///////////////////////////////////////////////////////////////////////////////

static void set_required_slot
(
    int			slot[MKW_N_TRACKS],	// status vector
    uint		slot1,		// first required slot index
    uint		slot2		// second required slot index
)
{
    uint i;
    for ( i = 0; i < MKW_N_TRACKS; i++ )
	if ( i != slot1 && i != slot2 )
	    slot[i] = -1;
	else if (!slot[i])
	    slot[i] = 1;
}

///////////////////////////////////////////////////////////////////////////////

int FindSlotsSZS
(
    // returns -1, 0 or +1

    szs_file_t		* szs,		// valid szs
    int			slot[MKW_N_TRACKS],	// status vector:
					//	-1: slot not possible
					//	 0: unknown
					//	+1: slot possible
    bool		*res_special	// not NULL: return TRUE, if an other slot
					//           than 4.2|6.1|6.2 is needed
)
{
    DASSERT(szs);
    DASSERT(slot);

    //--- load data

    FindSpecialFilesSZS(szs,false);

    if ( !szs->course_kcl_data || !szs->course_kmp_data )
    {
	uint i;
	for ( i = 0; i < MKW_N_TRACKS; i++ )
	    slot[i] = 0;
	return 0;
    }

    LoadObjFileListSZS( szs, szs->course_kmp_data,
			szs->course_kmp_size, 0, false, 0, 0 );

    bool need_ice_brres = false;

    kcl_t kcl;
    InitializeKCL(&kcl);
    kcl.fform_outfile = FF_KCL;
    enable_kcl_drop_auto++;
    if (!ScanKCL( &kcl, false, szs->course_kcl_data,
		szs->course_kcl_size, true, 0 ))
    {
	uint n = kcl.tridata.used;
	kcl_tridata_t *td = (kcl_tridata_t*)kcl.tridata.list;

	for ( ; n-- > 0; td++ )
	    if ( td->cur_flag == 0x70 )
	    {
		need_ice_brres = true;
		break;
	    }
	ResetKCL(&kcl);
    }
    enable_kcl_drop_auto--;


    //--- predefine

    uint i;
    for ( i = 0; i < MKW_N_TRACKS; i++ )
	slot[i] = 1;
    SLOT(4,2) = 0;
    SLOT(6,1) = 0;
    SLOT(6,2) = 0;

    if (res_special)
	*res_special = false;

    //uint pos_count = MKW_N_TRACKS - 3;
    //uint neg_count = 0;

    int stat = 1;


    //--- analyze slots 3.1 & 7.1

    if (szs->slot_31_71)
    {
	set_required_slot(slot,ISLOT(3,1),ISLOT(7,1));
	stat = -1;
	if (res_special)
	    *res_special = true;
    }


    //--- analyze slot 4.2

    if ( !szs->slot_42 || szs->moonview_mdl_stat < 2 )
	SLOT(4,2) = -1;
    else // if ( szs->moonview_mdl_stat == 3 )
	SLOT(4,2) = +1;


    //--- analyze slot 6.1

    if (need_ice_brres)
    {
	set_required_slot(slot,ISLOT(6,1),ISLOT(6,1));
	if (!szs->have_ice_brres)
	    SLOT(6,1) = -1;
	stat = -1;
    }
    else if ( !SLOT(6,1) && szs->have_ice_brres )
	SLOT(6,1) = 1;


    //--- analyze slot 6.2

    if (szs->slot_62)
    {
	set_required_slot(slot,ISLOT(6,2),ISLOT(6,2));
	stat = -1;
    }


    //--- adjust special slots & terminate

    //if (!SLOT(4,2)) SLOT(4,2) = -1;
    if (!SLOT(6,1)) SLOT(6,1) = -1;
    if (!SLOT(6,2)) SLOT(6,2) = -1;

    return stat;
}

#undef SLOT
#undef ISLOT

///////////////////////////////////////////////////////////////////////////////

ccp CreateSlotInfo ( szs_file_t * szs )
{
    DASSERT(szs);
    int slot[MKW_N_TRACKS];
    bool need_special;
    const int stat = FindSlotsSZS(szs,slot,&need_special);

    if ( szs->is_arena >= ARENA_FOUND )
	return "arena";

    static char buf[500];
    char *dest = buf;

    uint i;
    for ( i = 0; i < MKW_N_TRACKS; i++ )
	if ( slot[i] != stat )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = snprintfE(dest,buf+sizeof(buf),"%c%u.%u",
		    slot[i] < 0 ? '-' : slot[i] > 0 ? '+' : '?' ,
		    i/4+1, i%4+1 );
	}

    if ( dest == buf )
	*dest++ = '%';
    *dest++ = 0;
    const uint size = dest - buf;
    char *res = GetCircBuf(size);
    memcpy(res,buf,size);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp CreateBriefSlotInfo ( szs_file_t * szs )
{
    DASSERT(szs);
    int slot[MKW_N_TRACKS];
    bool need_special;
    const int stat = FindSlotsSZS(szs,slot,&need_special);

    static char buf[500], *dest = buf;

    uint i, count = 0;
    const char sign = stat < 0 ? '+' : '-';

    for ( i = 0; i < MKW_N_TRACKS; i++ )
	if ( slot[i] && slot[i] != stat )
	{
	    dest = snprintfE(dest,buf+sizeof(buf),"%c%u%u",
			count ? ',' : sign, i/4+1, i%4+1 );
	    count++;
	}

    *dest++ = 0;
    const uint size = dest - buf;
    char *res = GetCircBuf(size);
    memcpy(res,buf,size);
    return res;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ExtractFilesSZS()		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct extract_param_t
{
    ccp			dest;		// destination path
    bool		have_pt_dir;	// true: '.' directory found
    bool		is_cutting;	// true: is cutting a file
    bool		extract;	// true: extract subfiles
    bool		decode;		// true: store subfiles for decoding
    bool		mipmap;		// true: extract mipmaps too
    int			recurse_level;	// current recurse level
    int			indent;		// indention of log messages
    u32			align;		// found alignment

    ccp			basedir;	// not NULL: extract only basedir/*
    uint		basedir_off;	// >0: ...

    SubDir_t		*sdir;		// not NULL: store files here

    file_format_t	parent_fform;	// file format of parent

    StringField_t	include_list;	// include usually hidden files
    StringField_t	exclude_list;	// exclude files
    FormatField_t	decode_list;	// list with decode jobs
    StringField_t	extract_list;	// list with extract jobs
    FormatField_t	order_list;	// list of extracted files in sort order

} extract_param_t;

///////////////////////////////////////////////////////////////////////////////

static int extract_func
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
    DASSERT( !szs->file_size || szs->file_size >= szs->size );
    extract_param_t * ep = it->param;
    DASSERT(ep);
    DASSERT(ep->dest);

    if ( it->size > 0x40000000 )
	it->size = 0;


    //--- report files with trailing '.'

    const bool is_string_pool = !strcmp(it->path,BRRES_STRING_POOL_FNAME);

    noPRINT("trail=%s\n",it->trail_path);
    if ( !is_string_pool && it->trail_path && *it->trail_path == '.' )
    {
	if ( it->trail_path[1] == '/' && !it->trail_path[2] )
	    ep->have_pt_dir = true;
	else
	{
	    char * end = it->trail_path + strlen(it->trail_path) - 1;
	    const bool have_slash = *end == '/';
	    if ( have_slash )
		*end = 0;
	    noPRINT("TRAILING POINT: %s\n",it->path);
	    InsertStringField(&ep->include_list,it->path,false);
	    if ( have_slash )
		*end = '/';
	}
    }


    //--- check basedir

    ccp local_path = it->path;
    if ( *local_path == '.' && local_path[1] == '/' )
	local_path += 2;

    if (ep->basedir)
    {
	if (ep->basedir_off)
	{
	    if (strncasecmp(local_path,ep->basedir,ep->basedir_off))
		ep->basedir_off = 0;
	}
	else if (it->is_dir)
	    ep->basedir_off = strcasecmp(local_path,ep->basedir)
			? 0 : strlen(local_path);

	if (!ep->basedir_off)
	    return 0;

	local_path += ep->basedir_off;
    }


    //--- create dir or file

    FormatFieldItem_t * order_item
	= AppendFormatField(&ep->order_list,local_path,false,false);

    char pathbuf[PATH_MAX];
// [[sdir]]
//    ccp pathptr = sdir ? local_path
//		: PathCatPP(pathbuf,sizeof(pathbuf),ep->dest,local_path);
    ccp pathptr = PathCatPP(pathbuf,sizeof(pathbuf),ep->dest,local_path);

    if (it->is_dir)
    {
	if ( verbose > 1 || testmode && !analyze_fname )
	{
	    fprintf(stdlog,"%*s- %sCREATE  %s\n",
				ep->indent,"", testmode ? "WOULD " : "", pathptr );
	    fflush(stdlog);
	}
	if (ep->sdir)
	    InsertSubDir(ep->sdir,MemByString(local_path),0);
	else if (!testmode)
	    CreatePath(pathptr,true);
	return 0;
    }


    //--- file handling

    DASSERT(!it->is_dir); // now we have only files
    DASSERT(order_item);
    order_item->num = it->off;

    szs_file_t subszs;
// [[fname+]]
    InitializeSubSZS(&subszs,szs,it->off,it->size,FF_UNKNOWN,local_path,false);

    if ( verbose > 1 || testmode && !analyze_fname )
    {
	fprintf(stdlog,"%*s- %sEXTRACT%s %s:%s\n",
			    ep->indent,"", testmode ? "WOULD " : "",
			    ep->is_cutting ? "/CUT" : "",
			    GetNameFF(0,subszs.fform_arch), pathptr );
	fflush(stdlog);
    }

    ep->align |= it->off;

    if ( analyze_fname && IsBRSUB(subszs.fform_arch) )
	AnalyzeBRSUB(szs,subszs.data,subszs.size,it->name);


    //--- write extracted file, but don't close file here

    MEM_CHECK;

    string_pool_t sp;
    enum { SPM_OFF = 0, SPM_BRRES } sp_mode = SPM_OFF;
    if (!opt_raw)
    {
	switch ((int)szs->fform_arch)
	{
	 case FF_BRRES:
	    sp_mode = SPM_BRRES;
	    CollectStringsBRSUB(&sp,true,&subszs,true);
	    break;
	}
    }

    File_t F;
    if (ep->sdir)
    {
	memset(&F,0,sizeof(F));

	bool new_file;
	SubFile_t *sf = InsertSubFile(ep->sdir,MemByString(local_path),&new_file);
	if ( sf && new_file && subszs.size )
	{
	    sf->data_alloced = true;
	    sf->type = szs->fform_arch;
	    if (sp_mode)
	    {
		sf->size = subszs.size + sp.size;
		sf->data = MALLOC(sf->size);
		memcpy(sf->data,subszs.data,subszs.size);
		memcpy(sf->data+subszs.size,sp.data,sp.size);
	    }
	    else
	    {
		sf->data = MEMDUP(subszs.data,subszs.size);
		sf->size = subszs.size;
	    }
	}
    }
    else
    {
	pathptr = PathCatPP(pathbuf,sizeof(pathbuf),ep->dest,local_path);

	if (opt_links)
	{
	    szs_subfile_t *lptr = FindLinkSZS(szs,it->size,it->off,0);
	    if (lptr)
	    {
		PRINT("LINK: %s  ->  %s\n",pathptr,lptr->path);
		if (!link(lptr->path,pathptr))
		    goto no_create;
	    }

	    szs_subfile_t *file = AppendSubfileSZS(it->szs,it,0);
	    file->path   = STRDUP(pathptr);
	    file->device = it->size;
	    file->inode  = it->off;
	}

	CreateFileOpt(&F,true,pathptr,testmode,0);
	if (F.f)
	{
	    SetFileAttrib(&F.fatt,&szs->fatt,0);
	    size_t wstat = fwrite(subszs.data,1,subszs.size,F.f);
	    enumError err = wstat != subszs.size ? ERR_WRITE_FAILED : ERR_OK;

	    if ( !err && sp_mode && sp.data )
	    {
		wstat = fwrite(sp.data,1,sp.size,F.f);
		if ( wstat != sp.size )
		    err = ERR_WRITE_FAILED;
	    }

	    if (err)
		FILEERROR1(&F,err,
			    "Writing %zu bytes failed: %s\n",
			    subszs.size, pathptr);
	}
    }
  no_create:;
    if (sp_mode)
	ResetStringPool(&sp);

    if (is_string_pool)
    {
	char *dest = StringCopyS(pathbuf,sizeof(pathbuf),pathptr) - 4;
	if ( dest < pathbuf )
	    dest = pathbuf;
	StringCopyE(dest,pathbuf+sizeof(pathbuf),".txt");

	ccp fpath = strrchr(pathbuf,'/');
	InsertStringField(&ep->exclude_list,fpath?fpath+1:pathbuf,false);
	InsertStringField(&ep->exclude_list,local_path,false);

	if (!ep->sdir)
	{
	    File_t F;
	    CreateFileOpt(&F,true,pathbuf,testmode,0);
	    if (F.f)
	    {
		u32 off = 0;
		while ( off < subszs.size && !it->endian->rd32(subszs.data+off) )
		    off += 4;
		off += 4;
		while ( off < subszs.size )
		{
		    ccp sptr = (ccp)subszs.data + off;
		    if (!*sptr)
			break;
		    fprintf(F.f,"%s\r\n",sptr);
		    off += ALIGN32(strlen(sptr)+5,4);
		}
	    }
	    ResetFile(&F,opt_preserve);
	}
    }


    //--- prepare ...

    TryDecompressSZS(&subszs,true);

    bool extract = false, cut_file = false;
    if (!ep->is_cutting)
    {
	//--- decode

	MEM_CHECK;
	noPRINT("FF=%d,%s %s\n",subszs.fform_arch,
		GetNameFF(0,subszs.fform_arch),PrintID(subszs.data,4,0));
	switch (ep->parent_fform)
	{
	  case FF_BREFT:
	    if (!memcmp(local_path,"files/",6))
	    {
		const ccp dest_path = STRDUP2(local_path,".png");
		FormatFieldItem_t *item
			= InsertFormatFieldFF(&ep->decode_list,
					local_path,FF_BREFT,false,false,0);

		uint n_image;
		ExportPNG(
			    ep->dest,
			    dest_path,
			    opt_preserve ? &szs->fatt : 0,
			    subszs.data,
			    subszs.size,
			    0,
			    ep->mipmap,
			    &n_image,
			    it->endian,
			    item,
			    ep->decode,
			    &ep->exclude_list );
		item->num = n_image ? n_image : 1;
		FreeString(dest_path);
	    }
	    break;

	  default:
	    break;
	}

	ccp ext = ".txt";
	switch (subszs.fform_arch)
	{
	  case FF_BMG:
	    {
		InsertStringField(&ep->exclude_list,STRDUP2(local_path,".txt"),true);
		FormatFieldItem_t * item
		    = InsertFormatField(&ep->decode_list,local_path,false,false,0);
		DASSERT(item);
		item->fform = subszs.fform_arch;

		if (ep->decode)
		{
		    bmg_t bmg;
		    ScanBMG(&bmg,true,pathptr,subszs.data,subszs.size);
		    pathptr = PathCatPPE(pathbuf,sizeof(pathbuf),ep->dest,local_path,".txt");
		    if ( verbose > 1 || testmode )
		    {
			fprintf(stdlog,"%*s- %sDECODE  %s:%s\n",
				ep->indent,"", testmode ? "WOULD " : "",
				GetNameFF(0,subszs.fform_arch), pathptr );
			fflush(stdlog);
		    }
		    if (!testmode)
			SaveTextXBMG(&bmg,pathptr,true);
		    ResetBMG(&bmg);
		}
	    }
	    break;


	  case FF_KCL:
	    InsertStringField(&ep->exclude_list,STRDUP2(local_path,KCL_MTL_EXT),true);
	    InsertStringField(&ep->exclude_list,STRDUP2(local_path,KCL_FLAG_EXT),true);
	    ext = ".obj";
	    // fall through

	  case FF_KMP:
	  case FF_LEX:
	  case FF_MDL:
	  case FF_PAT:
	    {
		InsertStringField(&ep->exclude_list,STRDUP2(local_path,ext),true);
		FormatFieldItem_t * item
		    = InsertFormatField(&ep->decode_list,local_path,false,false,0);
		DASSERT(item);
		item->fform = subszs.fform_arch;

		if (ep->decode)
		{
		    pathptr = PathCatPPE(pathbuf,sizeof(pathbuf),ep->dest,local_path,ext);
		    if ( verbose > 1 || testmode )
		    {
			fprintf(stdlog,"%*s- %sDECODE  %s:%s\n",
				ep->indent,"", testmode ? "WOULD " : "",
				GetNameFF(0,subszs.fform_arch), pathptr );
			fflush(stdlog);
		    }

		    switch ((int)subszs.fform_arch)
		    {
			case FF_KCL:
			{
			    kcl_t kcl;
			    InitializeKCL(&kcl);
			    kcl.fname = pathptr;
			    ScanKCL(&kcl,false,subszs.data,subszs.size,true,global_check_mode);
			    if (!testmode)
				SaveTextKCL(&kcl,pathptr,true);
			    if (analyze_fname)
				AnalyzeKCL(&kcl);
			    kcl.fname = 0;
			    ResetKCL(&kcl);
			}
			break;

			case FF_KMP:
			{
			    kmp_t kmp;
			    InitializeKMP(&kmp);
			    kmp.fname = pathptr;
			    ScanKMP(&kmp,false,subszs.data,subszs.size,global_check_mode);
			    if (!testmode)
				SaveTextKMP(&kmp,pathptr,true);
			    if (analyze_fname)
				AnalyzeKMP(&kmp,szs->fname);
			    kmp.fname = 0;
			    ResetKMP(&kmp);
			}
			break;

			case FF_LEX:
			{
			    lex_t lex;
			    InitializeLEX(&lex);
			    lex.fname = pathptr;
			    ScanLEX(&lex,false,subszs.data,subszs.size);
			    if (!testmode)
				SaveTextLEX(&lex,pathptr,true);
			    lex.fname = 0;
			    ResetLEX(&lex);
			}
			break;

			case FF_MDL:
			{
			    mdl_t mdl;
			    InitializeMDL(&mdl);
			    mdl.fname = pathptr;
			#if USE_NEW_CONTAINER_MDL
			    ScanMDL(&mdl,false,subszs.data,subszs.size,
					LinkContainerSZS(szs),global_check_mode);
			#else
			    ScanMDL(&mdl,false,subszs.data,subszs.size,
					ContainerSZS(szs),global_check_mode);
			#endif
			    if (!testmode)
				SaveTextMDL(&mdl,pathptr,true);
			    mdl.fname = 0;
			    ResetMDL(&mdl);
			}
			break;

			case FF_PAT:
			{
			    pat_t pat;
			    InitializePAT(&pat);
			    pat.fname = pathptr;
			#if USE_NEW_CONTAINER_PAT
			    ScanPAT(&pat,false,subszs.data,subszs.size,
					LinkContainerSZS(szs),global_check_mode);
			#else
			    ScanPAT(&pat,false,subszs.data,subszs.size,
					ContainerSZS(szs),global_check_mode);
			#endif
			    if (!testmode)
				SaveTextPAT(&pat,pathptr,true);
			    pat.fname = 0;
			    ResetPAT(&pat);
			}
			break;
		    }
		}
	    }
	    break;

	  case FF_TPL:
	    if (analyze_fname)
		AnalyzeTPL(szs,subszs.data,subszs.size,it->name);

	    // fall through

	  case FF_BTI:
	  case FF_TEX:
	  case FF_TEX_CT:
	    {
		const ccp dest_path = STRDUP2(local_path,".png");
		FormatFieldItem_t * item
		    = InsertFormatField(&ep->decode_list,local_path,false,false,0);
		DASSERT(item);
		item->fform = subszs.fform_arch;

		uint n_image;
		ExportPNG(ep->dest,
			    dest_path,
			    opt_preserve ? &szs->fatt : 0,
			    subszs.data,
			    subszs.size,
			    0,
			    ep->mipmap,
			    &n_image,
			    it->endian,
			    item,
			    ep->decode,
			    &ep->exclude_list );
		item->num = n_image ? n_image : 1;
		FreeString(dest_path);
	    }
	    break;

	  default:
	    break;
	}

	if ( subszs.ff_attrib & FFT_EXTRACT )
	{
	    InsertStringField(&ep->extract_list,local_path,false);
	    InsertStringField(&ep->exclude_list,STRDUP2(local_path,".d"),true);
	    extract = ep->extract;
	}
	else if ( subszs.ff_attrib & FFT_CUT )
	{
	    InsertStringField(&ep->exclude_list,STRDUP2(local_path,".d"),true);
	    cut_file = opt_cut && IsValidSZS(&subszs,false) < VALID_ERROR;
	}
    }


    //--- extract sub archives or cut file

    if ( extract || cut_file )
    {
	pathptr = PathCatPP(pathbuf,sizeof(pathbuf),ep->dest,local_path);
	subszs.fname = pathptr;
	PRINT("EXTRACT/%s[%s]: %s\n",__FUNCTION__,GetNameFF_SZS(&subszs),subszs.fname);
	if (extract)
	    ExtractFilesSZS(&subszs,ep->recurse_level+1,false,ep->sdir,0);
	if (cut_file)
	    ExtractFilesSZS(&subszs,ep->recurse_level+1,true,ep->sdir,0);

	subszs.fname = EmptyString;
    }
    ResetSZS(&subszs);


    //--- now close file (because of time stamps)

    ResetFile(&F,opt_preserve);
    return 0;
}

// local_path local_path it->path local_path it->path local_path it->path local_path it->path local_path

///////////////////////////////////////////////////////////////////////////////

enumError ExtractFilesSZS
(
    szs_file_t	*szs,		// valid szs file
    int		recurse_level,	// recursion level
    bool	is_cutting,	// true: is cutting a file
    SubDir_t	*sdir,		// not NULL: store files here
    ccp		basedir		// not NULL: Extract only files if "/BASEDIR" or "./BASEDIR".
				// BASEDIR is like "a/b/c/" without leading slash
				// If set, don't create support files.
)
{
    DASSERT(szs);
    MEM_CHECK;
    PRINT("JOB EXTRACT: %d/%d %d ff=%d,%d[%s], %zu/%zu %s\n",
		recurse_level, opt_recurse, is_cutting,
		szs->fform_file, szs->fform_arch,
		GetNameFF(szs->fform_file,szs->fform_arch),
		szs->size, szs->file_size, szs->fname );
    DASSERT( !szs->file_size || szs->file_size >= szs->size );

    if (analyze_fname)
	switch ((int)szs->fform_arch)
	{
	    case FF_BREFF:
	    case FF_BREFT:
		AnalyzeBREFF(szs,szs->data,szs->size,szs->fname);
		break;
	}

    char dest[PATH_MAX];
    StringCat2S(dest,sizeof(dest),szs->fname,".d");
    PRINT("rec=%d isdir=%d dest=%d,%s\n",
	recurse_level, IsDirectory(dest,false), opt_dest!=0, opt_dest );
    if ( !recurse_level && ( opt_dest || !IsDirectory(dest,false) ) )
    {
	ccp my_dest = opt_dest;
	if (!my_dest)
	{
	    switch (szs->fform_arch)
	    {
		case FF_U8:
		case FF_WU8:
		case FF_RARC:
		case FF_PACK:
		case FF_RKC:
		    my_dest = "\1P/\1N.d/";
		    break;

		default:
		    my_dest = "\1P/\1F.d/";
		    break;
	    }
	}
	SubstDest(dest,sizeof(dest),szs->fname,my_dest,0,0,true);
	noPRINT("new-dest=%s\n",dest);
    }
    //else if ( recurse_level && !IsArchiveFF(szs->fform_arch) )
    else if (is_cutting)
    {
	ccp file = strrchr(szs->fname,'/');
	file = file ? file+1 : szs->fname;
	snprintf(dest,sizeof(dest),"%.*s.%s.d",
		(int)(file - szs->fname), szs->fname, file );
	PRINT("CUT TO: %s\n",dest);
    }

    const int indent = 2 * recurse_level;
    if ( verbose >= 0 && !recurse_level || verbose > 0 || testmode && !analyze_fname )
    {
	fprintf(stdlog,"%s%*s%sEXTRACT %s:%s -> %s\n",
		verbose > 1 ? "\n" : "",
		indent,"",
		testmode ? "WOULD " : "",
		GetNameFF_SZS(szs), szs->fname, dest );
	fflush(stdlog);
    }

    if ( !opt_overwrite && !opt_update && !sdir )
    {
	struct stat st;
	if (!stat(dest,&st))
	    return ERROR0(ERR_ALREADY_EXISTS,"Destination already exists: %s",dest);
    }

    extract_param_t ep;
    memset(&ep,0,sizeof(ep));
    InitializeStringField(&ep.include_list);
    InitializeStringField(&ep.exclude_list);
    InitializeFormatField(&ep.decode_list);
    InitializeStringField(&ep.extract_list);
    InitializeFormatField(&ep.order_list);
    InsertStringField(&ep.exclude_list,SZS_SETUP_FILE,false);

    ep.parent_fform	= szs->fform_arch;
    ep.sdir		= sdir;
    ep.dest		= dest;
    ep.decode		= opt_decode;
    ep.mipmap		= opt_mipmaps >= 0;
    ep.recurse_level	= recurse_level;
    ep.extract		= recurse_level < opt_recurse;
    ep.is_cutting	= is_cutting;
    ep.indent		= indent + 1;
    ep.align		= 0x80000000;
    ep.basedir		= basedir;

    switch (szs->fform_arch)
    {
     case FF_U8:
     case FF_WU8:
	InsertStringField(&ep.exclude_list,"course.kcl.d",false);
	InsertStringField(&ep.exclude_list,"course.kcl.obj",false);
	InsertStringField(&ep.exclude_list,"course.kcl.mtl",false);
	InsertStringField(&ep.exclude_list,"course.kcl.flag",false);
	InsertStringField(&ep.exclude_list,"course.kmp.d",false);
	InsertStringField(&ep.exclude_list,"course.kmp.txt",false);
	InsertStringField(&ep.exclude_list,"course.lex.txt",false);
	InsertFormatFieldFF(&ep.decode_list,"course.kcl",FF_KCL,false,false,0);
	InsertFormatFieldFF(&ep.decode_list,"course.kmp",FF_KMP,false,false,0);
	InsertFormatFieldFF(&ep.decode_list,"course.lex",FF_LEX,false,false,0);
	break;

     default:
	break;
    }

    const int cut_files = is_cutting
	? 1
	: recurse_level || IsArchiveFF(szs->fform_arch) ? -1 : 0;
    PRINT("cut_files=%d\n",cut_files);
    IterateFilesSZS(szs,extract_func,&ep,false,0,cut_files,SORT_NONE);


    //----- write setup file

 #ifndef __APPLE__
    char  *f_data = 0;
    size_t f_size = 0;
 #endif

    ccp destptr;
    FILE *f = 0;
    if (!basedir)
    {
	if (!sdir)
	{
	    destptr = PathCatPP(dest,sizeof(dest),dest,SZS_SETUP_FILE);
	    f = fopen(destptr,"w");
	}
     #ifndef __APPLE__
	else
	{
	    destptr = SZS_SETUP_FILE;
	    f = open_memstream(&f_data,&f_size);
	}
     #endif
    }

    if (f)
    {
	u32 align = 1;
	if ( ep.align )
	    while ( ! (ep.align & align) )
		align <<= 1;

	fputs(text_setup_header_cr,f);
	if (szs->obj_name)
	    fprintf(f,
		"# Name of the object:\r\n"
		"object-name = %s\r\n"
		"\r\n"
		,
		szs->obj_name );

	if (szs->obj_id_used)
	    fprintf(f,
		"# ID of the object:\r\n"
		"object-id = 0x%x\r\n"
		"\r\n"
		,
		szs->obj_id );

	fprintf(f,
	    "# The internal archive format (U8, WU8, BRRES, BREFF or BREFT):\r\n"
	    "archive-format = %s\r\n"
	    "\r\n"
	    "# The file format (YAZ0, YAZ1 or BZ for a compressed archive, other ignored):\r\n"
	    "file-format = %s\r\n"
	    "\r\n"
	    "# An archive format related version number (-1: unknown or not relevant)\r\n"
	    "version = %d\r\n"
	    "\r\n"
	    "# For U8+WU8 archives: Is there a special '.' base directory:\r\n"
	    "have-pt-dir = %d\r\n"
	    "\r\n"
	    "# The minimum and maximum file data positions:\r\n"
	    "min-data-offset = 0x%x\r\n"
	    "max-data-offset = 0x%x\r\n"
	    "\r\n"
	    "# The calculated alignment of all files:\r\n"
	    "data-align = 0x%x\r\n"
	    ,
	    GetNameFF(szs->fform_arch,szs->fform_arch),
	    GetNameFF(szs->fform_file,szs->fform_file),
// [[version-suffix]]
	    GetVersionFF(szs->fform_arch,szs->data,szs->size,0),
	    ep.have_pt_dir,
	    szs->min_data_off, szs->max_data_off, align );

	fputs(text_setup_include_cr,f);
	WriteStringField(f,destptr,&ep.include_list,"./","\r\n");
	fputs(text_setup_exclude_cr,f);
	WriteStringField(f,destptr,&ep.exclude_list,"./","\r\n");
	fputs(text_setup_encode_cr,f);
	WriteFormatField(f,destptr,&ep.decode_list,0,"./","\r\n",0);
	fputs(text_setup_create_cr,f);
	WriteStringField(f,destptr,&ep.extract_list,"./","\r\n");
	fputs(text_setup_order_cr,f);
	WriteFormatField(f,destptr,&ep.order_list,0,"./","\r\n",10);
	fputs( "\r\n#---------------------------------------"
		    "---------------------------------------\r\n\r\n",f);
	fclose(f);

 #ifndef __APPLE__
	if (sdir)
	{
	    bool new_file;
	    SubFile_t *sf = InsertSubFile(sdir,MemByString(SZS_SETUP_FILE),&new_file);
	    if ( sf && new_file )
	    {
	     #if TRACE_ALLOC_MODE > 2
		RegisterAlloc(__FUNCTION__,__FILE__,__LINE__,f_data,f_size,false);
	     #endif
		sf->data_alloced = true;
		sf->data = (u8*)f_data;
		sf->size = f_size;
	    }
	    else
		FREE(f_data);
	}
 #endif
    }

    ResetStringField(&ep.include_list);
    ResetStringField(&ep.exclude_list);
    ResetFormatField(&ep.decode_list);
    ResetStringField(&ep.extract_list);
    ResetFormatField(&ep.order_list);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetOptBasedir()
{
    if (!opt_basedir)
	return 0;

    while ( *opt_basedir == '/' || *opt_basedir == '\\' || *opt_basedir == '.' )
	opt_basedir++;

    char *src, *dest;
    int have_slash = 0;
    for ( src = dest = opt_basedir; *src; src++ )
    {
	if ( *src == '/' || *src == '\\' )
	    have_slash++;
	else
	{
	    if (have_slash)
	    {
		have_slash = 0;
		*dest++ = '/';
	    }
	    *dest++ = *src;
	}
    }

    if ( dest < src )
    {
	*dest++ = '/';
	*dest = 0;
    }
    else
    {
	const uint pos = dest - opt_basedir;
	opt_basedir = MEMDUP(opt_basedir,pos+1);
	opt_basedir[pos] = '/';
    }

    return opt_basedir;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

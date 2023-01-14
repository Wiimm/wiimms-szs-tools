
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "lib-xbmg.h"
#include "lib-szs.h"
#include "dclib-utf8.h"
#include "lib-mkw.h"
#include "db-mkw.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    MKW hooks			///////////////
///////////////////////////////////////////////////////////////////////////////

static int GetTrackIndexXBMG ( uint idx, int result_on_invalid )
{
    return idx < BMG_N_TRACK
		? track_pos[idx]
		: idx < BMG_N_CT_TRACK
		? idx
		: result_on_invalid;
}

///////////////////////////////////////////////////////////////////////////////

static int GetArenaIndexXBMG ( uint idx, int result_on_invalid )
{
    return idx < BMG_N_ARENA
		? arena_pos[idx]
		: result_on_invalid;
}


///////////////////////////////////////////////////////////////////////////////

static ccp GetContainerNameXBMG ( const bmg_t *bmg )
{
    DASSERT(bmg);

    if ( bmg->szs && bmg->szs->fname && *bmg->szs->fname )
    {
	ccp name = strrchr(bmg->szs->fname,'/');
	return name ? name + 1 : bmg->szs->fname;
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   options			///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupBMG ( ccp opt_filter_bmg )
{
    DASSERT( MID_CT_END		<= MAX_FILTER_MSG );
    DASSERT( MID_CT_CUP_END	<= MAX_FILTER_MSG );
    DASSERT( MID_CT_TRACK_END	<= MAX_FILTER_MSG );
    DASSERT( MID_CT_CUP_REF_END	<= MAX_FILTER_MSG );

    DASSERT( MID_LE_END		<= MAX_FILTER_MSG );
    DASSERT( MID_LE_CUP_END	<= MAX_FILTER_MSG );
    DASSERT( MID_LE_TRACK_END	<= MAX_FILTER_MSG );
    DASSERT( MID_LE_CUP_REF_END	<= MAX_FILTER_MSG );

    //--- general setting

    opt_bmg_force_count		= force_count;
    opt_bmg_support_mkw		= true;
    opt_bmg_support_ctcode	= ctcode_enabled;
    opt_bmg_support_lecode	= lecode_enabled;
    opt_bmg_rcup_fill_limit	=  0x80;
    opt_bmg_track_fill_limit	= 0x200;
    opt_bmg_export		= export_count > 0 ;
 #ifdef TEST
    opt_bmg_allow_print		= true;
 #else
    opt_bmg_allow_print		= verbose > 1;
 #endif
    opt_bmg_use_slots		= true;
    opt_bmg_use_raw_sections	= true;

    if (opt_align)
	opt_bmg_align = ALIGN32(opt_align,4);

    GetTrackIndexBMG		= GetTrackIndexXBMG;
    GetArenaIndexBMG		= GetArenaIndexXBMG;
    GetContainerNameBMG		= GetContainerNameXBMG;


    //--- compatibility settings

    if ( compatible < COMPAT_2_08 )
    {
	opt_bmg_use_slots	 = false;
	opt_bmg_use_raw_sections = false;
    }

    if ( compatible < COMPAT_1_44 )
    {
	opt_bmg_old_escapes = true;
    }

    if ( compatible < COMPAT_1_39 )
    {
	opt_bmg_inline_attrib = false;
	opt_bmg_color_name = BMG_CNL_BASICS;
    }

    if ( compatible < COMPAT_1_23 )
    {
	opt_bmg_colors = 0;
	opt_bmg_color_name = BMG_CNL_NONE;
    }

    if (opt_filter_bmg)
	ScanOptFilterBMG(opt_filter_bmg);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextBMG()			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[obsolete]] ?

static char * scan_attrib ( u8 *dest, uint size, ccp src )
{
    DASSERT(dest);
    DASSERT( size <= BMG_ATTRIB_SIZE );
    memset(dest,0,BMG_ATTRIB_SIZE);

    while ( *src ==  ' ' || *src == '\t' )
	src++;
    const bool have_brackets = *src == '[';
    if ( have_brackets )
	src++;

    int idx = 0, add_idx = 0;
    for(;;)
    {
	while ( *src ==  ' ' || *src == '\t' )
	    src++;

	if ( *src == ',' )
	{
	    src++;
	    idx++;
	    add_idx = 0;
	}
	else if ( *src == '/' )
	{
	    src++;
	    idx = ALIGN32(idx+1,4);
	    add_idx = 0;
	}
	else if ( *src == '%' )
	{
	    src++;
	    char *end;
	    u32 num = str2ul(src,&end,16);
	    if ( src == end )
		break;

	    idx = ALIGN32(idx+add_idx,4);
	    noPRINT("U32[0x%02x/%02x]: %8x\n",idx,size,num);

	    char buf[4];
	    write_be32(buf,num);
	    const int max_copy = (int)size - (int)idx;
	    if ( max_copy > 0 )
		memcpy(dest+idx,buf, max_copy < 4 ? max_copy : 4 );
	    idx += 3;
	    add_idx = 1;
	    src = end;
	}
	else
	{
	    char *end;
	    u8 num = str2ul(src,&end,16);
	    if ( src == end )
		break;

	    idx += add_idx;
	    noPRINT("U8[0x%02x/%02x]: %8x\n",idx,size,num);
	    if ( idx < size )
		dest[idx] = num;
	    add_idx = 1;
	    src = end;
	}
    }
    //HexDump16(stderr,0,0,dest,size);

    if ( have_brackets && *src == ']' )
	src++;
    return (char*)src;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LoadXBMG()			///////////////
///////////////////////////////////////////////////////////////////////////////

static int read_bmg
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if (term)
	return 0;

    const u8 * data = it->szs->data + it->off;
// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,it->size,it->size);
    if ( fform != FF_BMG && fform != FF_BMG_TXT )
	return 0;

    bmg_t *bmg = it->param;
    DASSERT(bmg);
    bmg->src_count++;
    PRINT("BMG #%u found: %s\n",bmg->src_count,it->name);

    bmg_t temp;
    enumError err = ScanBMG(&temp,true,it->name,data,it->size);
    if (!err)
	PatchBMG(bmg,&temp,BMG_PM_OVERWRITE,0,true);
    else if ( bmg->max_err < err )
	bmg->max_err = err;
    ResetBMG(&temp);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadXBMG
(
    bmg_t		* bmg,		// pointer to valid bmg
    bool		initialize,	// true: initialize 'bmg'
    ccp			fname,		// filename of source
    bool		allow_archive,	// true: concat BMGs of archives to 1 source
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(bmg);
    DASSERT(fname);
    PRINT("LoadXBMG(%p,init=%d,,aa=%d,ign=%d) fname=%s\n",
		bmg, initialize, allow_archive, ignore_no_file, fname );

    if (initialize)
	InitializeBMG(bmg);
    else
	ResetBMG(bmg);

    if (!strcmp(fname,"0"))
    {
	bmg->is_text_src = false;
	bmg->fname = STRDUP(fname);
	return ERR_OK;
    }

    szs_extract_t eszs;
    enumError err = ExtractSZS(&eszs,true,fname,0,ignore_no_file);
    if (err)
	return err;

    if (eszs.data)
    {
	bmg->data_alloced = eszs.data_alloced;
	bmg->data_size	= eszs.data_size;
	bmg->data	= eszs.data;
	eszs.data	= 0;
	bmg->fname	= eszs.fname;
	eszs.fname	= 0;
	ResetExtractSZS(&eszs);
    }
    else
    {
	szs_file_t szs; // use 'szs_file_t' for automatic decompression
	InitializeSZS(&szs);
	err = LoadSZS(&szs,fname,true,ignore_no_file,true);
	if ( allow_archive && IsArchiveFF(szs.fform_arch) )
	{
	    bmg->src_is_arch = true;
	    IterateFilesParSZS(&szs,read_bmg,bmg,false,false,0,-1,SORT_NONE);
	    bmg->fname = szs.fname;
	    szs.fname = 0;
	    ResetSZS(&szs);
	    if ( !bmg->max_err && !bmg->src_count )
		bmg->max_err = ERR_NO_SOURCE_FOUND;
	    noPRINT("BMG/FNAME=%s\n",bmg->fname);
	    return bmg->max_err;
	}

	bmg->data	= szs.data;
	bmg->data_size	= szs.size;
	bmg->data_alloced = szs.data_alloced;
	bmg->fatt	= szs.fatt;
	bmg->fname	= szs.fname;
	szs.data	= 0;
	szs.data_alloced= 0;
	szs.fname	= 0;
	ResetSZS(&szs);

	if (err)
	    return err;
    }

    return ScanBMG(bmg,false,0,0,0);
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataBMG
(
    bmg_t		* bmg,		// BMG data structure
    bool		init_bmg,	// true: initialize 'bmg' first
    struct raw_data_t	* raw		// valid raw data
)
{
    DASSERT(bmg);
    DASSERT(raw);

    PRINT("BMG/FF: %u = %s\n",raw->fform,GetNameFF(raw->fform,0));

    if (init_bmg)
	InitializeBMG(bmg);
    else
	ResetBMG(bmg);
    if (!raw->data)
	return ERR_OK;
    bmg->fatt  = raw->fatt;

    if (IsArchiveFF(raw->fform))
    {
	bmg->src_is_arch = true;
	szs_file_t  szs;
// [[fname+]]
	AssignSZS(&szs,true,raw->data,raw->data_size,false,raw->fform,bmg->fname);
	szs.fname = raw->fname;
	raw->fname = 0;
	IterateFilesParSZS(&szs,read_bmg,bmg,false,false,0,-1,SORT_NONE);
	bmg->fname = szs.fname;
	szs.fname = 0;
	noPRINT("BMG/FNAME=%s\n",bmg->fname);
	ResetSZS(&szs);
	if ( !bmg->max_err && !bmg->src_count )
	    bmg->max_err = ERR_NO_SOURCE_FOUND;
	return bmg->max_err;
    }

    return ScanBMG(bmg,init_bmg,raw->fname,raw->data,raw->data_size);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawXBMG()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawXBMG
(
    bmg_t		* bmg,		// pointer to valid bmg
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(bmg);
    DASSERT(fname);

    return SaveRawBMG( bmg, fname,
		GetFileModeByOpt(testmode,fname,bmg->fname), set_time );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTextXBMG()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextXBMG
(
    bmg_t		* bmg,		// pointer to valid bmg
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(bmg);
    DASSERT(fname);

    return SaveTextBMG( bmg, fname,
		GetFileModeByOpt(testmode,fname,bmg->fname),
		set_time, long_count > 0,
		brief_count > 0 ? brief_count : !print_header );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    Patching			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[patch_bmg_cond_t]]

typedef struct patch_bmg_cond_t
{
    bool not;		// true: enable patching, if not any mid is found
    u32  mid[3];	// up to 3 MIDs
}
patch_bmg_cond_t;

//-----------------------------------------------------------------------------

static bmg_t *patch_bmg	 = 0;	// list with loaded BMG files for --patch-bmg
static int n_patch_bmg	 = 0;	// number of elements of 'patch_bmg'
int have_bmg_patch_count = 0;	// >0: have bmg patch

///////////////////////////////////////////////////////////////////////////////

int ScanOptPatchMessage ( ccp arg )
{
    ccp eq = strchr(arg,'=');
    if (!eq)
	eq = strchr(arg,','); // only for compatibility, 2011-07 => leave it

    char keyword[100];
    StringCopyS(keyword,sizeof(keyword),arg);
    if ( eq && eq-arg < sizeof(keyword) )
	keyword[eq-arg] = 0;


    //--- analyse condition

    patch_bmg_cond_t cond;
    cond.not = false;

    char *sep = strchr(keyword,'?');
    if (!sep)
    {
	 cond.not = true;
	 sep = strchr(keyword,'!');
    }

    if (sep)
	*sep++ = 0;


    //--- analyse keyword

    const KeywordTab_t *cmd = ScanKeyword(0,keyword,PatchKeysBMG);
    if (!cmd)
    {
	ERROR0(ERR_SYNTAX,"Option --patch-bmg: Invalid patch mode: '%s'\n",keyword);
	return 1;
    }

    if ( cmd->opt && !eq )
    {
	ERROR0(ERR_SYNTAX,
		"Option --patch-bmg: Missing '=%s' for keyword '%s': %s\n",
		cmd->opt == 1 ? "filename" : "parameter", cmd->name1, arg );
	return 1;
    }
    else if ( !cmd->opt && eq )
    {
	ERROR0(ERR_SYNTAX,
		"Option --patch-bmg: Parameter not allowed for '%s': %s\n",
		cmd->name1, arg );
	return 1;
    }


    //--- complete condition scanning

    bool cond_valid = false;

    if ( sep && *sep )
    {
	char *end;
	int stat = ScanMidBMG(0,cond.mid,cond.mid+1,cond.mid+2,sep,0,&end);
	if ( stat < 0 || *end )
	    ERROR0(ERR_WARNING,
		"Option --patch-bmg: Invalid message condition: %s %c %s\n",
		cmd->name1, cond.not ? '!' : '?', sep );
	else
		cond_valid = stat > 0;
	PRINT("STAT=%d, NOT=%d, MID=%x,%x,%x => valid=%d\n",
		stat, cond.not, cond.mid[0], cond.mid[1], cond.mid[2], cond_valid );
    }


    //--- store record

    patch_bmg_cond_t *pcond = cond_valid ? ALLOCDUP(&cond,sizeof(cond)) : 0;

    const uint num = cmd->id | cmd->opt << BMG_PM__SHIFT_OPT;
    if ( cmd->opt == 2 )
    {
	char buf[10000];
	ScanEscapedString(buf,sizeof(buf),eq+1,-1,true,0,0);
	AppendParamField(&patch_bmg_list,buf,false,num,pcond);
    }
    else if (cmd->opt)
	AppendParamField(&patch_bmg_list,eq+1,false,num,pcond);
    else
	AppendParamField(&patch_bmg_list,EmptyString,true,num,pcond);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptMacroBMG ( ccp arg )
{

    bmg_t temp;
    enumError stat = LoadXBMG(&temp,true,arg,true,false);
    if (!stat)
    {
	if (!bmg_macros)
	{
	    bmg_macros = MALLOC(sizeof(*bmg_macros));
	    InitializeBMG(bmg_macros);
	}
	PatchOverwriteBMG(bmg_macros,&temp,true);
    }

    ResetBMG(&temp);
    return stat != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError SetupPatchingListBMG()
{
    uint max_err = ERR_OK;

    if ( n_patch_bmg != patch_bmg_list.used )
    {
	if (n_patch_bmg)
	    have_patch_count--, have_bmg_patch_count--;

	ResetPatchingListBMG();
	n_patch_bmg = patch_bmg_list.used;
	patch_bmg = CALLOC(n_patch_bmg,sizeof(*patch_bmg));

	int i;
	for ( i = 0; i < n_patch_bmg; i++ )
	{
	    ParamFieldItem_t *pi = patch_bmg_list.field + i;
	    if ( pi->num >> BMG_PM__SHIFT_OPT == 1 )
	    {
		enumError err = LoadXBMG(patch_bmg+i,true,pi->key,true,false);
		if ( err > ERR_WARNING && err > max_err )
		    max_err = err;
	    }
	}

	if (n_patch_bmg)
	    have_patch_count++, have_bmg_patch_count++;
    }

    return max_err;
}

///////////////////////////////////////////////////////////////////////////////

void ResetPatchingListBMG()
{
    int i;
    for ( i = 0; i < n_patch_bmg; i++ )
	ResetBMG(patch_bmg+i);
    FREE(patch_bmg);
    patch_bmg = 0;
    n_patch_bmg = 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError UsePatchingListBMG ( bmg_t * dest )
{
    if (!n_patch_bmg)
	SetupPatchingListBMG();
    DASSERT( n_patch_bmg == patch_bmg_list.used );

    int i;
    enumError stat = ERR_OK;
    for ( i = 0; i < n_patch_bmg; i++ )
    {
	ParamFieldItem_t *pi = patch_bmg_list.field + i;
	if (pi->data)
	{
	    patch_bmg_cond_t *cond = pi->data;
	    bool found = 0;
	    uint i;
	    for ( i = 0; !found && i < sizeof(cond->mid)/sizeof(*cond->mid); i++ )
		found = cond->mid[i] && FindItemBMG(dest,cond->mid[i]);

	    PRINT("COND FOUND: mid=%x,%x,%x, not=%d => found=%d => %s\n",
			cond->mid[0], cond->mid[1], cond->mid[2], cond->not,
			found, found^cond->not ? "EXEC" : "SKIP" );

	    if (!(found^cond->not))
		continue;
	}

	enumError err = PatchBMG( dest, patch_bmg+i,
					pi->num & BMG_PM__MASK_CMD, pi->key, false );
	if ( err > ERR_WARNING )
	    return err;
	if ( err == ERR_DIFFER )
	    stat = ERR_DIFFER;
    }

    if (opt_points_used)
    {
	char buf[500];
	strcpy(buf,"points=");
	PrintMkwPointsLIST(buf+7,sizeof(buf)-7,MkwPointsTab,2);

	bmg_item_t * dptr = InsertItemBMG(dest,MID_VERSUS_POINTS,0,0,0);
	if (dptr)
	    AssignItemTextBMG(dptr,buf,-1);
    }

    if (filter_message)
    {
	bmg_item_t * dptr = dest->item;
	bmg_item_t * dend = dptr + dest->item_used;

	bool dirty = false;
	for ( ; dptr < dend; dptr++ )
	{
	    if ( dptr->mid >= MAX_FILTER_MSG || !filter_message[dptr->mid] )
	    {
		FreeItemBMG(dptr);
		dptr->text  = 0;
		ResetAttribBMG(dest,dptr);
		dirty = true;
	    }
	}
	if ( dirty && stat < ERR_DIFFER )
	    stat = ERR_DIFFER;
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

uint ListPatchingListBMG ( FILE *f, int indent, ccp title )
{
    DASSERT(f);
    if (!patch_bmg_list.used)
	return 0;

    indent = NormalizeIndent(indent);

    if (title && *title)
	fprintf(f,"%*s%s [N=%u]\n",indent,"",title,patch_bmg_list.used);
    indent += 3;

    uint i;
    for ( i = 0; i < patch_bmg_list.used; i++ )
    {
	ParamFieldItem_t *pi = patch_bmg_list.field + i;
	fprintf(f,"%*s%-9s",indent,"",GetPatchNameBMG(pi->num));

	char buf[1000];
	PrintEscapedString(buf,sizeof(buf),pi->key,-1,CHMD__ALL,0,0);
	switch( pi->num >> BMG_PM__SHIFT_OPT )
	{
	    case 1: fprintf(f," file: %s\n",buf); break;
	    case 2: fprintf(f," parameter: %s\n",buf); break;
	    default: fputc('\n',f); break;
	}
    }

    return patch_bmg_list.used;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetPatchNameBMG ( uint cmd )
{
    cmd &= BMG_PM__MASK_CMD;
    const KeywordTab_t *kt;
    for ( kt = PatchKeysBMG; kt->name1; kt++ )
	if ( kt->id == cmd )
	    return kt->name1;

    const uint buf_size = 6;
    char *buf = GetCircBuf(buf_size);
    snprintf(buf,buf_size,"[%u]",cmd);
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   DiffBMG()			///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_only_in
(
    uint		index,		// index of relevant bmg (1|2)
    const bmg_item_t	* bi,		// relevant bmg item
    int			width		// width out output lines
)
{
    DASSERT(bi);
    char msg[300], attrib[BMG_ATTRIB_BUF_SIZE], text[1000];
    PrintAttribBMG(attrib,sizeof(attrib),bi->attrib,bi->attrib_used,0,true);
    const int msg_len = snprintfS(msg,sizeof(msg),
			"* Only in source #%u:%6x %s",
			index, bi->mid, attrib );

    width -= msg_len + 7;
    if ( width <= 0 )
	printf("%s\n",msg);
    else
    {
	PrintString16BMG(text,sizeof(text),bi->text,bi->len,
			BMG_UTF8_MAX_DIFF,0,opt_bmg_colors);
	printf("%s %.*s\n",msg,width,text);
    }
};

///////////////////////////////////////////////////////////////////////////////

static void print_differ
(
    const bmg_item_t	* bi1,		// first bmg item
    const bmg_item_t	* bi2,		// second bmg item
    ccp			message,	// differ message
    bool		text_differ,	// true: print text of both
    int			width		// width out output lines
)
{
    DASSERT(bi1);
    DASSERT(bi2);
    DASSERT( bi1->mid == bi2->mid );

    char attrib1[BMG_ATTRIB_BUF_SIZE], attrib2[BMG_ATTRIB_BUF_SIZE];
    PrintAttribBMG(attrib1,sizeof(attrib1),bi1->attrib,bi1->attrib_used,0,true);
    PrintAttribBMG(attrib2,sizeof(attrib2),bi2->attrib,bi2->attrib_used,0,true);

    char msg[50+2*BMG_ATTRIB_BUF_SIZE], text[1000];
    const int msg_len = !strcmp(attrib1,attrib2)
	? snprintf(msg,sizeof(msg),
			"* %-18s%6x %s", message, bi1->mid, attrib1 )
	: snprintf(msg,sizeof(msg),
			"* %-18s%6x %s %s", message, bi1->mid, attrib1, attrib2 );

    if (text_differ)
    {
	width -= 8;
	if ( width < 20 )
	     width = 20;

	printf("%s\n",msg);
	int min_len, max_len;
	if ( bi1->len < bi2->len )
	{
	    min_len = bi1->len;
	    max_len = bi2->len;
	}
	else
	{
	    min_len = bi2->len;
	    max_len = bi1->len;
	}

	u16 *t1 = bi1->text;
	u16 *t2 = bi2->text;
	u16 *end = t1 + min_len;
	while ( t1 < end && *t1 == *t2 )
	    t1++, t2++;
	int diff_idx = t1 - bi1->text;

	if ( diff_idx > max_len - width/2 )
	     diff_idx = max_len - width/2;
	if ( diff_idx < 0 )
	     diff_idx = 0;

	PrintString16BMG(text,sizeof(text), bi1->text+diff_idx,
			bi1->len-diff_idx, BMG_UTF8_MAX_DIFF, 0, opt_bmg_colors );
	printf("    1: %.*s\n",width,text);
	PrintString16BMG(text,sizeof(text), bi2->text+diff_idx,
			bi2->len-diff_idx, BMG_UTF8_MAX_DIFF, 0, opt_bmg_colors );
	printf("    2: %.*s\n",width,text);
    }
    else
    {
	width -= msg_len + 7;
	if ( width <= 0 )
	    printf("%s\n",msg);
	else
	{
	    PrintString16BMG(text,sizeof(text),bi1->text,bi1->len,
				BMG_UTF8_MAX,0,opt_bmg_colors);
	    printf("%s %.*s\n",msg,width,text);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError DiffBMG
(
    const bmg_t		* bmg1,		// first bmg
    const bmg_t		* bmg2,		// second bmg
    bool		quiet,		// true: be quiet
    int			width		// width out output lines
)
{
    PRINT("DiffSZS(%p,%p,%d,%d)\n",bmg1,bmg2,quiet,width);
    DASSERT(bmg1);
    DASSERT(bmg2);


    const bmg_item_t * p1 = bmg1->item;
    const bmg_item_t * e1 = p1 + bmg1->item_used;
    const bmg_item_t * p2 = bmg2->item;
    const bmg_item_t * e2 = p2 + bmg2->item_used;

    PRINT("PTR: %p..%p/%u, %p..%p/%u\n",
		p1, e1, bmg1->item_used,
		p2, e2, bmg2->item_used );

    int differ = 0;
    while ( p1 && p1 < e1 && p2 && p2 < e2 )
    {
	noPRINT("\t%s : %s\n",p1->path,p2->path);

	if ( p1->mid < p2->mid )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    print_only_in(1,p1,width);
	    p1++;
	    continue;
	}

	if ( p1->mid > p2->mid )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    print_only_in(2,p2,width);
	    p2++;
	    continue;
	}

	const bool attrib_differ
		=  p1->attrib_used != p2->attrib_used
		|| memcmp(p1->attrib,p2->attrib,p1->attrib_used);
	if ( p1->len != p2->len
		|| memcmp(p1->text,p2->text,p1->len*sizeof(*p1->text)) )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    ccp msg = attrib_differ ? "Text+attr differ:" : "Text differ:";
	    print_differ(p1,p2,msg,true,width);
	}
	else if ( attrib_differ )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    print_differ(p1,p2,"Attributes differ:",false,width);
	}

	p1++;
	p2++;
    }

    while ( p1 && p1 < e1 )
    {
	if (quiet)
	    return ERR_DIFFER;
	differ++;
	print_only_in(1,p1,width);
	p1++;
    }

    while ( p2 && p2 < e2 )
    {
	if (quiet)
	    return ERR_DIFFER;
	differ++;
	print_only_in(2,p2,width);
	p2++;
    }

    return differ ? ERR_DIFFER : ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   ScanOptLoadBMG()		///////////////
///////////////////////////////////////////////////////////////////////////////

bmg_t opt_load_bmg = {0};
bool opt_patch_names = false;

//-----------------------------------------------------------------------------

int ScanOptLoadBMG ( ccp arg )
{
    if ( !arg || !*arg )
	return 0;

    bmg_t temp;
    enumError err = LoadXBMG(&temp,true,arg,true,false);
    if (!err)
	PatchBMG(&opt_load_bmg,&temp,BMG_PM_OVERWRITE,0,true);
    ResetBMG(&temp);
    return err > ERR_WARNING;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanOptFilterBMG()		///////////////
///////////////////////////////////////////////////////////////////////////////

u8 * filter_message = 0; // NULL or field with 'MAX_FILTER_MSG' elements

//-----------------------------------------------------------------------------

enum
{
    MD_NONE, MD_ID, MD_PARAM, MD_GENERIC, MD_ALL, MD_CHAT,

                MD_CUPS,    MD_TRACKS,    MD_ARENAS,
    MD_CTCODE,  MD_CTCUPS,  MD_CTTRACKS,  MD_CTARENAS,  MD_CTREFS,
    MD_LECODE,  MD_LECUPS,  MD_LETRACKS,  MD_LEARENAS,  MD_LEREFS,
    MD_XCODE,   MD_XCUPS,   MD_XTRACKS,   MD_XARENAS,   MD_XREFS,
    MD_ALLCODE, MD_ALLCUPS, MD_ALLTRACKS, MD_ALLARENAS, MD_ALLREFS,
};

//-----------------------------------------------------------------------------

static void ScanOptMsgJob ( int mode, u8 value )
{
    if ( mode >= MD_XCODE && mode <= MD_XREFS )
    {
	if (opt_bmg_support_lecode)
	    mode = mode - MD_XCODE + MD_LECODE;
	else if (opt_bmg_support_ctcode)
	    mode = mode - MD_XCODE + MD_CTCODE;
    }

    switch(mode)
    {
	case MD_NONE:
	    PRINT("SET ALL = %d\n",!value);
	    memset(filter_message,!value,MAX_FILTER_MSG);
	    break;

	case MD_ID:
	    filter_message[MID_PARAM_IDENTIFY] = value;
	    break;

	case MD_PARAM:
	    filter_message[MID_PARAM_IDENTIFY] = value;
	    memset( filter_message + MID_PARAM_BEG, value,
		    MID_PARAM_END - MID_PARAM_BEG );
	    break;

	case MD_CUPS:
	    PRINT("SET CUPS = %d\n",value);
	    memset( filter_message + MID_RCUP_BEG, value,
		    MID_RCUP_END - MID_RCUP_BEG );
	    memset( filter_message + MID_BCUP_BEG, value,
		    MID_BCUP_END - MID_BCUP_BEG );
	    ScanOptMsgJob(MD_XCUPS,value);
	    ScanOptMsgJob(MD_XREFS,value);
	    break;

	case MD_TRACKS:
	    PRINT("SET TRACKS = %d\n",value);
	    memset(filter_message+MKW_TRACK_MID_1,value,MKW_N_TRACKS);
	    memset(filter_message+MKW_TRACK_MID_2,value,MKW_N_TRACKS);
	    ScanOptMsgJob(MD_XTRACKS,value);
	    ScanOptMsgJob(MD_XREFS,value);
	    break;

	case MD_ARENAS:
	    PRINT("SET ARENAS = %d\n",value);
	    memset(filter_message+MKW_ARENA_MID_1,value,MKW_N_ARENAS);
	    memset(filter_message+MKW_ARENA_MID_2,value,MKW_N_ARENAS);
	    ScanOptMsgJob(MD_XARENAS,value);
	    break;

	case MD_CHAT:
	    PRINT("SET CHAT = %d\n",value);
	    memset(filter_message+MID_CHAT_BEG,value,BMG_N_CHAT);
	    break;

    //-- ct-code

	case MD_CTCODE:
	    PRINT("SET CT-CODE = %d\n",value);
	    memset( filter_message + MID_CT_BEG, value,
		    MID_CT_END - MID_CT_BEG );
	    break;

	case MD_CTCUPS:
	    PRINT("SET CT-CUPS = %d\n",value);
	    memset( filter_message + MID_CT_CUP_BEG, value,
		    MID_CT_CUP_END - MID_CT_CUP_BEG );
	    break;

	case MD_CTTRACKS:
	    PRINT("SET CT-TRACKS = %d\n",value);
	    memset( filter_message + MID_CT_TRACK_BEG, value,
		    MID_CT_TRACK_END - MID_CT_TRACK_BEG );
	    break;

	case MD_CTARENAS:
	    PRINT("SET CT-ARENAS = %d\n",value);
	    memset( filter_message + MID_CT_ARENA_BEG, value,
		    MID_CT_ARENA_END - MID_CT_ARENA_BEG );
	    break;

	case MD_CTREFS:
	    PRINT("SET CT-REFS = %d\n",value);
	    memset( filter_message + MID_CT_CUP_REF_BEG, value,
		    MID_CT_CUP_REF_END - MID_CT_CUP_REF_BEG );
	    break;

    //-- le-code

	case MD_LECODE:
	    PRINT("SET LE-CODE = %d\n",value);
	    memset( filter_message + MID_LE_BEG,
		    value, MID_LE_END - MID_LE_BEG );
	    break;

	case MD_LECUPS:
	    PRINT("SET LE-CUPS = %d\n",value);
	    memset( filter_message + MID_LE_CUP_BEG, value,
		    MID_LE_CUP_END - MID_LE_CUP_BEG );
	    break;

	case MD_LETRACKS:
	    PRINT("SET LE-TRACKS = %d\n",value);
	    memset( filter_message + MID_LE_TRACK_BEG, value,
		    MID_LE_TRACK_END - MID_LE_TRACK_BEG );
	    break;

	case MD_LEARENAS:
	    PRINT("SET LE-ARENAS = %d\n",value);
	    memset( filter_message + MID_LE_ARENA_BEG, value,
		    MID_LE_ARENA_END - MID_LE_ARENA_BEG );
	    break;

	case MD_LEREFS:
	    PRINT("SET LE-REFS = %d\n",value);
	    memset( filter_message + MID_LE_CUP_REF_BEG, value,
		    MID_LE_CUP_REF_END - MID_LE_CUP_REF_BEG );
	    break;

    //-- all*

	case MD_ALLCODE:
	    ScanOptMsgJob(MD_CTCODE,value);
	    ScanOptMsgJob(MD_LECODE,value);
	    break;

	case MD_ALLCUPS:
	    ScanOptMsgJob(MD_CUPS,value);
	    ScanOptMsgJob(MD_CTCUPS,value);
	    ScanOptMsgJob(MD_LECUPS,value);
	    break;

	case MD_ALLTRACKS:
	    ScanOptMsgJob(MD_TRACKS,value);
	    ScanOptMsgJob(MD_CTTRACKS,value);
	    ScanOptMsgJob(MD_LETRACKS,value);
	    break;

	case MD_ALLARENAS:
	    ScanOptMsgJob(MD_ARENAS,value);
	    ScanOptMsgJob(MD_CTARENAS,value);
	    ScanOptMsgJob(MD_LEARENAS,value);
	    break;

	case MD_ALLREFS:
	    ScanOptMsgJob(MD_CTREFS,value);
	    ScanOptMsgJob(MD_LEREFS,value);
	    break;


    //-- misc

	case MD_GENERIC:
	    PRINT("SET GENRIC = %d\n",value);
	    memset(filter_message+MID_GENERIC_BEG,value,BMG_N_GENERIC);
	    break;

	case MD_ALL:
	    PRINT("SET ALL = %d\n",value);
	    memset(filter_message,value,MAX_FILTER_MSG);
	    break;
    }
}

//-----------------------------------------------------------------------------

int ScanOptFilterBMG ( ccp arg )
{
    // >>> see SetupBMG() for MID assertions!

    if ( !arg || !*arg )
	return 0;

    if (!filter_message)
    {
	while ( *arg > 0 && *arg <= ' ' )
	    arg++;

	PRINT("FILTER-MESSAGE: ALLOC SET %d\n",*arg == '-');
	filter_message = MALLOC(MAX_FILTER_MSG);
	memset( filter_message, *arg == '-', MAX_FILTER_MSG );
    }

    //------------------------------------------------------------------
    // MD_CTCODE..MD_CTREFS & MD_LECODE..MD_LEREFS & MD_XCODE..MD_XREFS
    // must have the same order to support X-calcualtions below!
    //------------------------------------------------------------------

    static const KeywordTab_t opt_msg[] =
    {
	{ MD_NONE,	"NONE",			0,	0 },
	{ MD_ID,	"IDENTIFICATION",	"ID",	0 },
	{ MD_PARAM,	"PARAM",		0,	0 },
	 { MD_GENERIC,	"GENERIC",		0,	0 },
	 { MD_ALL,	"ALL",			0,	0 },
	{ MD_CUPS,	"CUPS",			0,	0 },
	{ MD_TRACKS,	"TRACKS",		0,	0 },
	{ MD_ARENAS,	"ARENAS",		0,	0 },
	{ MD_CHAT,	"CHAT",			0,	0 },
	 { MD_CTCODE,	"CTCODE",		"CT",	0 },
	 { MD_CTCUPS,	"CTCUPS",		0,	0 },
	 { MD_CTTRACKS,	"CTTRACKS",		0,	0 },
	 { MD_CTARENAS,	"CTARENAS",		0,	0 },
	 { MD_CTREFS,	"CTREFS",		0,	0 },
	{ MD_LECODE,	"LECODE",		"LE",	0 },
	{ MD_LECUPS,	"LECUPS",		0,	0 },
	{ MD_LETRACKS,	"LETRACKS",		0,	0 },
	{ MD_LEARENAS,	"LEARENAS",		0,	0 },
	{ MD_LEREFS,	"LEREFS",		0,	0 },
	 { MD_XCODE,	"XCODE",		"X",	0 },
	 { MD_XCUPS,	"XCUPS",		0,	0 },
	 { MD_XTRACKS,	"XTRACKS",		0,	0 },
	 { MD_XARENAS,	"XARENAS",		0,	0 },
	 { MD_XREFS,	"XREFS",		0,	0 },
	{ MD_ALLCODE,	"ALLCODE",		0,	0 },
	{ MD_ALLCUPS,	"ALLCUPS",		0,	0 },
	{ MD_ALLTRACKS,	"ALLTRACKS",		0,	0 },
	{ MD_ALLARENAS,	"ALLARENAS",		0,	0 },
	{ MD_ALLREFS,	"ALLREFS",		0,	0 },
	{0,0,0,0}
    };

    for(;;)
    {
	while ( *arg > 0 && *arg <= ' ' || *arg == ',' || *arg == '+' )
	    arg++;
	if (!*arg)
	    break;

	const u8 value = *arg != '-';
	if (!value)
	    arg++;

	char keyname[20], *dest = keyname;
	while (isalnum((int)*arg))
	{
	    if ( dest < keyname + sizeof(keyname)-1 )
		*dest++ = *arg;
	    arg++;
	}
	*dest = 0;

	if ( dest > keyname+1 )
	{
	    const KeywordTab_t * key = ScanKeyword(0,keyname,opt_msg);
	    if (key)
	    {
		ScanOptMsgJob(key->id,value);
		continue;
	    }
	}

	u32 a1, a2, a3;
	if ( ScanMidBMG(0,&a1,&a2,&a3,keyname,dest,0) < 1 )
	{
	    ERROR0(ERR_SYNTAX,"Option --msg: Invalid parameter: %s\n",keyname);
	    return 1;
	}
	if (!a2)
	    a2 = a1;
	if (!a3)
	    a3 = a1;

	while ( *arg > 0 && *arg <= ' ' )
	    arg++;
	if ( *arg != ':' )
	{
	    PRINT("MID FOUND: %x,%x,%x := %u\n",a1,a2,a3,value);
	    if ( a1 < MAX_FILTER_MSG )
		filter_message[a1] = value;
	    if ( a2 < MAX_FILTER_MSG )
		filter_message[a2] = value;
	    if ( a3 < MAX_FILTER_MSG )
		filter_message[a3] = value;
	    continue;
	}
	arg++;

	u32 b1, b2, b3;
	if ( ScanMidBMG(0,&b1,&b2,&b3,arg,0,(char**)&arg) < 1 )
	{
	    ERROR0(ERR_SYNTAX,"Option --msg: Invalid parameter: %s\n",keyname);
	    return 1;
	}
	if (!b2)
	    b2 = b1;
	if (!b3)
	    b3 = b1;

	PRINT("MID FOUND: %x..%x, %x..%x, %x..%x := %u\n",a1,b1,a2,b2,a3,b3,value);

	if ( ++b1 > MAX_FILTER_MSG )
	    b1 = MAX_FILTER_MSG;
	if ( a1 < b1 )
	    memset( filter_message+a1, value, b1-a1 );

	if ( ++b2 > MAX_FILTER_MSG )
	    b2 = MAX_FILTER_MSG;
	if ( a2 < b2 )
	    memset( filter_message+a2, value, b2-a2 );

	if ( ++b3 > MAX_FILTER_MSG )
	    b3 = MAX_FILTER_MSG;
	if ( a3 < b3 )
	    memset( filter_message+a3, value, b3-a3 );
    }

    if ( logging >= 2 )
	HexDump016(stderr,0,0,filter_message,MAX_FILTER_MSG);

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 cmd_bmg_cat()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_bmg_cat ( bool mix )
{
    stdlog = stderr;

    enumError err = SetupPatchingListBMG();
    if ( err > ERR_WARNING )
	return err;

    bmg_t mix_bmg;
    InitializeBMG(&mix_bmg);

    raw_data_t raw;
    InitializeRawData(&raw);

    uint mix_count = 0;
    enumError cmd_err = ERR_OK;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	enumError err = LoadRawData(&raw,false,param->arg,0,opt_ignore>0,FF_BMG);

	if ( err == ERR_NOT_EXISTS || err > ERR_WARNING && opt_ignore )
	    continue;
	if ( err > ERR_WARNING )
	    return err;

	if ( verbose >= 0 || testmode )
	    fprintf(stdlog,"%sCAT %s:%s\n",
			verbose > 0 ? "\n" : "",
			GetNameFF(raw.fform,0), raw.fname );

	bmg_t bmg;
	err = ScanRawDataBMG(&bmg,true,&raw);
	if ( err > ERR_WARNING )
	    return err;

	err = UsePatchingListBMG(&bmg);
	if ( err > ERR_WARNING )
	    return err;

	if (mix)
	{
	    PatchBMG(&mix_bmg,&bmg,BMG_PM_OVERWRITE,0,true);
	    mix_count++;
	}
	else if (!testmode)
	{
	    err = SaveTextXBMG(&bmg,"-",false);
	    fflush(stdout);
	    if ( err > ERR_WARNING )
		return err;
	}
	ResetBMG(&bmg);
    }

    if ( !testmode && mix )
    {
	if (!mix_count)
	{
	    err = UsePatchingListBMG(&mix_bmg);
	    if ( err > ERR_WARNING )
		return err;
	}

	err = SaveTextXBMG(&mix_bmg,"-",false);
	fflush(stdout);
	if ( err > ERR_WARNING )
	    return err;
    }

    ResetBMG(&mix_bmg);
    ResetRawData(&raw);
    return cmd_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 cmd_bmg_identifier()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_bmg_identifier()
{
    stdlog = stderr;

    ct_bmg_t ctb;
    SetupCtBMG(&ctb,CTM_OPTIONS,CTM_OPTIONS);

    bmg_t bmg;
    InitializeBMG(&bmg);
    PatchIdentificationBMG(&bmg,&ctb);
    SaveTextXBMG(&bmg,"-",false);
    fflush(stdout);

    ResetBMG(&bmg);
    ResetCtBMG(&ctb);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    scan options		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptBmgEndian ( ccp arg )
{
    const KeywordTab_t *cmd = ScanKeyword(0,arg,TabEndianBMG);
    if (!cmd)
    {
	ERROR0(ERR_SYNTAX,
		"Option --bmg-endian: Invalid mode: '%s'\n",arg);
	return 1;
    }

    opt_bmg_endian = cmd->id;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptBmgEncoding ( ccp arg )
{
    const KeywordTab_t *cmd = ScanKeyword(0,arg,TabEncodingBMG);
    if (!cmd)
    {
	ERROR0(ERR_SYNTAX,
		"Option --bmg-encoding: Invalid mode: '%s'\n",arg);
	return 1;
    }

    if (cmd->opt)
	ERROR0(ERR_WARNING,
		"Option --bmg-encoding: Mode »%s« not supported yet!\n",
		GetEncodingNameBMG(cmd->id,"?") );

    opt_bmg_encoding = cmd->id;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptBmgInfSize ( ccp arg, bool is_wbmgt )
{
    if ( !arg || !*arg )
    {
	opt_bmg_inf_size = 0;
	return 0;
    }

    char *end;
    uint num = str2ul(arg,&end,10);
    if ( end == arg || *end )
    {
	ERROR0(ERR_SYNTAX,
		"Option --%sinf-size: Not a number: %s\n",
		is_wbmgt ? "" : "bmg-", arg );
	return 1;
    }

    if ( num < 4 || num > BMG_INF_LIMIT )
    {
	ERROR0(ERR_SEMANTIC,
		"Option --%sinf-size: Number must be in range 4..%u: %s\n",
		is_wbmgt ? "" : "bmg-", BMG_INF_LIMIT, arg );
	return 1;
    }
    opt_bmg_inf_size = num;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptBmgMid ( ccp arg )
{
    const int stat = ScanKeywordOffAutoOn(arg,OFFON_ON,OFFON_FORCE,"Option --bmg-mid");
    if ( stat == OFFON_ERROR )
	return 1;

    opt_bmg_mid = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int scan_opt_attrib ( bool *flag, u8* attrib, ccp opt, ccp arg )
{
    DASSERT(flag);
    DASSERT(attrib);
    DASSERT(opt);

    *flag = false;
    memset(attrib,0,BMG_ATTRIB_SIZE);
    if ( !arg || !*arg )
	return 0;

    ccp end = scan_attrib(attrib,BMG_ATTRIB_SIZE,arg);
    if (end)
	while ( *end == ' ' || *end == '\t' )
	    end++;
    if ( !end || *end )
    {
	ERROR0(ERR_SYNTAX,
		"Option --%s: Invalid attribute vector: %s\n", opt, arg );
	return 1;
    }

    *flag = true;
    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptForceAttrib ( ccp arg )
{
    return scan_opt_attrib(&opt_bmg_force_attrib,bmg_force_attrib,"force-attrib",arg);
}

//-----------------------------------------------------------------------------

int ScanOptDefAttrib ( ccp arg )
{
    return scan_opt_attrib(&opt_bmg_def_attrib,bmg_def_attrib,"def-attrib",arg);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

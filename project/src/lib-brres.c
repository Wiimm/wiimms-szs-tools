
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "lib-brres.h"
#include "lib-mdl.h"
#include "lib-pat.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			brres helper			///////////////
///////////////////////////////////////////////////////////////////////////////

static u16 get_highest_bit ( u8 val )
{
    // return bit number of highest set bit, or 0 if no bit is set.

    return  val & 0xf0
	? ( val & 0x80 ? 7
	  : val & 0x40 ? 6
	  : val & 0x20 ? 5
	  :              4 )
	: ( val & 0x08 ? 3
	  : val & 0x04 ? 2
	  : val & 0x02 ? 1
	  :              0 );
}

///////////////////////////////////////////////////////////////////////////////

static u16 calc_brres_id
(
    ccp			prev_name,
    uint		prev_len,
    ccp			cur_name,
    uint		cur_len
)
{
    if ( prev_len < cur_len )
	return cur_len - 1 << 3 | get_highest_bit(cur_name[cur_len-1]);

    while ( cur_len-- > 0 )
    {
	const u8 ch = prev_name[cur_len] ^ cur_name[cur_len];
	if (ch)
	    return cur_len << 3 | get_highest_bit(ch);
    }

    return ~(u16)0;
}

///////////////////////////////////////////////////////////////////////////////

static void dump_brres_group
(
    const brres_root_t	*  root,
    const endian_func_t	* endian
)
{
    const uint root_size = endian->rd32(&root->size);
    const u8 * root_end = (u8*)root + root_size;

    int grp_idx = 0;
    const brres_group_t * grp = BRRES_ROOT_GROUP(root);
    while ( (u8*)grp < root_end )
    {
	const uint grp_size	= endian->rd32(&grp->size);
	const uint n_entries	= endian->rd32(&grp->n_entries);
	const u8 * next_grp	= (u8*)grp + grp_size;
	noPRINT("GRP %2u: n=%02u, size=%04x\n",
		grp_idx, n_entries, grp_size );
	if ( next_grp > root_end )
	    break;

	printf(	"\n"
		" idx    ID    ? left right    name     data  magic name\n"
		"%.79s\n",Minus300);

	const brres_entry_t * entry = grp->entry;
	const brres_entry_t * entry_end = entry + 1 + n_entries;
	int entry_idx;
	for ( entry_idx = 0; entry < entry_end; entry++, entry_idx++ )
	{
	    ccp name = "";
	    uint name_len = 0;
	    const uint name_off = endian->rd32(&entry->name_off);
	    if (name_off)
	    {
		name = (ccp)grp + name_off;
		name_len = endian->rd32(name-4);
	    }

	    const uint data_off = endian->rd32(&entry->data_off);
	    const u8 * data = (u8*)grp + data_off;

	    printf("%4x: %4x %4x %4x %4x %8x %8x  %-4s  %.*s\n",
			entry_idx,
			endian->rd16(&entry->id),
			endian->rd16(&entry->unknown),
			endian->rd16(&entry->left_idx),
			endian->rd16(&entry->right_idx),
			endian->rd32(&entry->name_off),
			endian->rd32(&entry->data_off),
			data_off ? PrintID(data,4,0) : "-",
			name_len, name );
	}

	grp = (brres_group_t*)next_grp;
	grp_idx++;
    }
    putchar('\n');
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			get BRSUB info			///////////////
///////////////////////////////////////////////////////////////////////////////
// http://wiki.tockdom.com/wiki/BRRES_Sub_Files_(File_Format)#Common_Format

const brsub_info_t brsub_info[] =
{

 //-----------------------------------------------------------------------
 // fform1+2,	vers	nsect	warn		// comment
 //-----------------------------------------------------------------------

  { FF_CHR, 0,	 3,5,	 1,2,	BIMD_FATAL },
  { FF_CHR, 0,	 5,5,	 2,2,	BIMD_OK },	// [[2do]] verify hexdump

  { FF_CLR, 0,	 4,4,	 2,2,	BIMD_OK },	// [[2do]] verify hexdump

  { FF_MDL, 0,	 8,11,	11,14,	BIMD_FATAL },
  { FF_MDL, 0,	 9,11,	11,14,	BIMD_FATAL },	// found in SSBB
  { FF_MDL, 0,	11,11,	14,14,	BIMD_OK },

  { FF_PAT, 0,	 4,4,	 6,6,	BIMD_OK },

  { FF_SCN, 0,	 4,5,	 6,7,	BIMD_FATAL },
  { FF_SCN, 0,	 5,5,	 7,7,	BIMD_OK },	// [[2do]] verify hexdump

  { FF_SHP, 0,	 4,4,	 3,3,	BIMD_OK },	// [[2do]] verify hexdump

  { FF_SRT, 0,	 4,5,	 1,2,	BIMD_FATAL },
  { FF_SRT, 0,	 5,5,	 2,2,	BIMD_OK },	// [[2do]] verify hexdump

  { FF_TEX,
    FF_TEX_CT,	 1,3,	 1,1,	BIMD_INFO },	// not BIMD_HINT
  { FF_TEX,
    FF_TEX_CT,	 2,3,	 2,1,	BIMD_FAIL },
  { FF_TEX,
    FF_TEX_CT,	 3,3,	 1,1,	BIMD_OK },

  //--- list terminated with fform1 == 0 (FF_UNKNOWN)
  {0,0,0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

const brsub_info_t * GetInfoBRSUB
(
    file_format_t	fform,		// file format
    u32			version		// sub version
)
{
    if (fform)
    {
	const brsub_info_t *bi;
	for ( bi = brsub_info; bi->fform1; bi++ )
	    if ( ( bi->fform1 == fform || bi->fform2 == fform )
		&& bi->version == version )
	    {
		return bi;
	    }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const brsub_info_t * GetCorrectBRSUB
(
    file_format_t	fform		// file format
)
{
    if (fform)
    {
	const brsub_info_t *bi;
	for ( bi = brsub_info; bi->fform1; bi++ )
	    if ( ( bi->fform1 == fform || bi->fform2 == fform ) && !bi->warn )
		return bi;
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			get num of sections		///////////////
///////////////////////////////////////////////////////////////////////////////

int GetKnownSectionNumBRSUB
(
    file_format_t	fform,		// file format
    u32			version		// sub version
)
{
    noPRINT("GetKnownSectionNumBRSUB(%d,%d) ff=%s\n",
		fform, version, GetNameFF(0,fform) );

    const brsub_info_t *bi = GetInfoBRSUB(fform,version);
    return bi ? bi->n_sect : -1;
}

///////////////////////////////////////////////////////////////////////////////

int GetGenericSectionNumBRSUB
(
    const u8		* data,		// valid data pointer
    uint		data_size,	// size of 'data'
    const endian_func_t	* endian	// NULL or endian functions
)
{
    DASSERT(data);
    if ( data_size < 0x20 )
	return -1;

    if (!endian)
	endian = &be_func;
    u32 max_offset = endian->rd32(data+4);
    if ( max_offset > data_size )
	 max_offset = data_size;
    const u32 body_size = max_offset;

    int n_sect;
    for ( n_sect = 0; ; n_sect++ )
    {
	u32 offset = 0x10 + n_sect * 4;
	if ( offset >= max_offset )
	    break;

	u32 grp_offset = endian->rd32(data+offset);
	noPRINT("n=%2u, off=%5x, goff=%5x, maxoff=%5x\n",
		n_sect, offset, grp_offset, max_offset );
	if (!grp_offset)
	    continue;
	if ( grp_offset <= offset || grp_offset > body_size || grp_offset&3 )
	    break;

	if ( data[grp_offset]
		&& endian->rd32(data+grp_offset-4) == strlen((ccp)data+grp_offset) )
	    break;

	if ( max_offset > grp_offset )
	     max_offset = grp_offset;
    }

    return n_sect;
}

///////////////////////////////////////////////////////////////////////////////

int GetSectionNumBRSUB
(
    const u8		* data,		// valid data pointer
    uint		data_size,	// size of 'data'
    const endian_func_t	* endian	// NULL or endian functions
)
{
    DASSERT(data);

// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    if ( fform && data_size >= 0x0c )
    {
	if (!endian)
	    endian = &be_func;
	const u32 version = endian->rd32(data+8);
	const int n = GetKnownSectionNumBRSUB(fform,version);
	if ( n >= 0 )
	    return n;
    }

    return GetGenericSectionNumBRSUB(data,data_size,endian);
};

///////////////////////////////////////////////////////////////////////////////

ccp PrintHeaderNameBRSUB
(
    char		* buf,		// result buffer
					// If NULL, a local circulary static buffer is used
    size_t		buf_size,	// size of 'buf', ignored if buf==NULL
    const brsub_header_t * bhead,	// valid brsub header
    uint		n_grp,		// number of groups
    const endian_func_t	* endian	// NULL or endian
)
{
    DASSERT(bhead);

    if (!buf)
	buf = GetCircBuf( buf_size = 40 );

    if (endian)
	endian = &be_func;

    snprintf( buf, buf_size,
		".BRSUB.header.%s-v%u.n%u.bin",
		PrintID(bhead->magic,4,0),
		endian->rd32(&bhead->version),
		n_grp );

    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			create brres			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool get_brres_id_bit
(
    brres_info_t * info,
    u16 id
)
{
    DASSERT(info);
    const uint char_idx = id >> 3;
    noPRINT("get_brres_id_bit(%x,%.*s) -> %d+%d/%d -> %d\n",
	    id, info->nlen, info->name,
	    char_idx, id&7, info->nlen,
	    char_idx < info->nlen
		&& info->name[char_idx] >> ( id & 7 ) & 1 );
    return char_idx < info->nlen
	&& info->name[char_idx] >> ( id & 7 ) & 1;
}

///////////////////////////////////////////////////////////////////////////////

void CalcEntryBRRES
(
    brres_info_t	* info,
    uint		entry_idx
)
{
    DASSERT(info);

    // setup 'entry' : item to insert
    brres_info_t * entry = info + entry_idx;
    entry->id = calc_brres_id(0,0,entry->name,entry->nlen);
    entry->left_idx = entry->right_idx = entry_idx;

    // setup 'prev' : the previuos 'current' item
    brres_info_t * prev = info;

    // setup 'current' : the current item while walking through the tree
    uint current_idx = prev->left_idx;
    brres_info_t * current = info + current_idx;

    // last direction
    bool is_right = false;

    noPRINT("++ID: e=%zx,%x, p=%zx,%x, c=%zx,%x\n",
		entry-info, entry->id,
		prev-info, prev->id,
		current-info, current->id ) ;

    while ( entry->id <= current->id && current->id < prev->id )
    {
	if ( entry->id == current->id )
	{
	    // calculate a new entry id
	    entry->id = calc_brres_id( current->name, current->nlen,
				      entry->name, entry->nlen );
	    if (get_brres_id_bit(current,entry->id))
	    {
		entry->left_idx  = entry_idx;
		entry->right_idx = current_idx;
	    }
	    else
	    {
		entry->left_idx  = current_idx;
		entry->right_idx = entry_idx;
	    }
	}

	prev = current;

	is_right = get_brres_id_bit(entry,current->id);

	current_idx = is_right ? current->right_idx : current->left_idx;
	current = info + current_idx;
	noPRINT("ID: e=%zx,%x, p=%zx,%x, c=%zx,%x\n",
		entry-info, entry->id,
		prev-info, prev->id,
		current-info, current->id ) ;
    }

    if ( current->nlen == entry->nlen && get_brres_id_bit(current,entry->id) )
	entry->right_idx = current_idx;
    else
	entry->left_idx = current_idx;

    if (is_right)
	prev->right_idx = entry_idx;
    else
	prev->left_idx = entry_idx;
}

//-----------------------------------------------------------------------------

enumError CreateBRRES
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			total_size	// total file size
)
{
    DASSERT(szs);
    PRINT("CreateBRRES() ndir=%d, totsize=%x, align=%x\n",
		szs->subfile.used, total_size, opt_align_brres );

    enumError max_err = ERR_OK;
    char path_buf[PATH_MAX];
    const endian_func_t	* endian = &be_func;
    szs->endian = endian;


    //--- sort

    SortSubFilesSZS(szs,SORT_BRRES);


    //--- load and analyze data

    total_size = ALIGN32(total_size,opt_align_brres);
    u8 * file_data = CALLOC(1,total_size);

    uint n_directories = 1;
    uint total_data_size = 0;
    uint min_string_pool_size = 0;

    {
	// find all strings

	u8 * data_ptr = file_data;
	szs_subfile_t *f_ptr, *f_end = szs->subfile.list + szs->subfile.used;
	for ( f_ptr = szs->subfile.list; f_ptr < f_end; f_ptr++ )
	{
	    noPRINT_IF(f_ptr->is_dir, "DIR-ID=%d, |%s|\n",f_ptr->dir_id,f_ptr->path);

	    if (f_ptr->is_dir)
	    {
		PRINT("DIR #%u FOUND: %s\n",n_directories,f_ptr->path);
		IsolateFilename(path_buf,sizeof(path_buf),f_ptr->path,0);
		InsertStringPool(&szs->string_pool,path_buf,false,0);
		n_directories++;
	    }
	    else
	    {
		f_ptr->offset = data_ptr - file_data;
		u32 data_size = f_ptr->size;
		if (f_ptr->data)
		    memcpy(data_ptr,f_ptr->data,data_size);
		else
		{
		    enumError err = LoadFILE(source_dir,f_ptr->path,0,
					data_ptr,data_size,0,&szs->fatt,true);
		    if ( max_err < err )
			max_err = err;
		}

// [[analyse-magic]]
		f_ptr->fform = GetByMagicFF(data_ptr,data_size,data_size);
		IsolateFilename(path_buf,sizeof(path_buf),f_ptr->path,f_ptr->fform);
		InsertStringPool(&szs->string_pool,path_buf,false,0);

		if (IsValidBRSUB(data_ptr,data_size,data_size,
				0,f_ptr->fform,true,f_ptr->path,endian) < VALID_ERROR )
		{
		    noPRINT("----- %s -> %s -----\n",f_ptr->path,path_buf);
		    szs_file_t sub;
		    AssignSZS(&sub,true,data_ptr,data_size,false,FF_UNKNOWN,f_ptr->path);
		    if ( CollectStringsBRSUB(&szs->string_pool,false,&sub,false) > 0 )
		    {
			// fix file size for later copy operation
			const u32 new_data_size = endian->rd32(data_ptr+4);
			if ( data_size > new_data_size )
			     data_size = new_data_size;

			brsub_list_t * list = CALLOC(1,sizeof(*list));
			list->data = data_ptr;
			list->size = data_size;
			list->next = szs->brsub_list;
			szs->brsub_list = list;
		    }
		    ResetSZS(&sub);

		    const int string_pool_size = f_ptr->size - data_size;
		    if ( (int)min_string_pool_size < string_pool_size )
			 min_string_pool_size = string_pool_size;
		}

		total_data_size += ALIGN32(data_size,opt_align_brres);
		data_ptr += ALIGN32(f_ptr->size,opt_align_brres);
		DASSERT( data_ptr <= file_data + total_size );
	    }
	}
	DASSERT( data_ptr == file_data + total_size );

     #if defined(TEST) && 0
	ccp * ptr = szs->string_pool.sf.field;
	ccp * end = ptr + szs->string_pool.sf.used;
	while ( ptr < end )
	    PRINT(">%s<\n",*ptr++);
     #endif
    }


    //--- size calculations

    u32 grp_offset  = sizeof(brres_header_t) + sizeof(brres_root_t);
    u32 grp_end	    = grp_offset
		    + sizeof(brres_group_t) * n_directories
		    + sizeof(brres_entry_t) * ( n_directories + szs->subfile.used );

    u32 data_offset = grp_end <= szs->min_data_off
		    ? ALIGN32(szs->min_data_off,opt_align_brres)
		    : ALIGN32(grp_end,
				opt_align_brres > 0x20 ? opt_align_brres : 0x20 );
    u32 str_offset  = ALIGN32(data_offset + total_data_size, opt_align_brres );
    CalcStringPool(&szs->string_pool,str_offset,endian);
    u32 data_size   = ALIGN32( str_offset + szs->string_pool.size,
				opt_align_brres > 0x80 ? opt_align_brres : 0x80 );

    PRINT("* GRP=%x..%x, DATA=%x, STR=%x, END=%x\n",
		grp_offset, grp_end, data_offset, str_offset, data_size );


    szs->fform_arch		= FF_BRRES;
    szs->ff_attrib		= GetAttribFF(szs->fform_arch);
    szs->ff_version		= -1;
    szs->data_alloced		= true;
    szs->size			= data_size;
    szs->file_size		= data_size;

    // alloc a little bit more temporary space if needed
    if ( min_string_pool_size < szs->string_pool.size )
	 min_string_pool_size = szs->string_pool.size;
    szs->data = CALLOC(1,szs->size - szs->string_pool.size + min_string_pool_size );


    //--- setup BRRES header

    brres_header_t * bhead = (brres_header_t*)szs->data;
    memcpy(bhead->magic,BRRES_MAGIC,sizeof(bhead->magic));
    endian->wr16(&bhead->bom,0xfeff);
    endian->wr32(&bhead->size,data_size);
    endian->wr16(&bhead->root_off,sizeof(brres_header_t));
    endian->wr16(&bhead->n_sections, 2 + szs->subfile.used - n_directories );


    //--- setup BRRES root

    brres_root_t * broot = (brres_root_t*)(szs->data+sizeof(brres_header_t));
    memcpy(broot->magic,BRRES_ROOT_MAGIC,sizeof(broot->magic));
    endian->wr32(&broot->size, grp_end - grp_offset + sizeof(brres_root_t) );


    //--- setup stack for directory offsets

    typedef struct entry_pt_t
    {
	u8 * dest;	// destination
	u32 delta;	// delta, that must be substracted

    } entry_pt_t;

    entry_pt_t * ep_list = CALLOC(n_directories+1,sizeof(entry_pt_t));
    entry_pt_t * ep_read  = ep_list;
    entry_pt_t * ep_write = ep_list + 1; // skip first


    //--- setup subfile int dir_id order

    const uint dir_order_size = ( n_directories + 1 ) * sizeof(u16);
    u16 * dir_order = MALLOC(dir_order_size);
    memset(dir_order,~0,dir_order_size); // special value for empty directories

    bool * dir_done = CALLOC(n_directories+1,sizeof(*dir_done));

    u16 * dir_ptr = dir_order;
    szs_subfile_t *f_ptr, *f_end = szs->subfile.list + szs->subfile.used;
    for ( f_ptr = szs->subfile.list; f_ptr < f_end; f_ptr++ )
    {
	const u16 dir_id = f_ptr->dir_id;
	DASSERT( dir_id < n_directories );
	if (!dir_done[dir_id])
	{
	    dir_done[dir_id] = true;
	    *dir_ptr++ = dir_id;
	}
    }
    PRINT("DIR-PTR = %zu/%u\n", dir_ptr - dir_order, n_directories );
    DASSERT( dir_ptr - dir_order <= n_directories );
    FREE(dir_done);


    //--- setup info

    // that's enough!
    const size_t info_size = ( szs->subfile.used + 1 ) * sizeof(brres_info_t);
    brres_info_t * info_list = MALLOC(info_size);


    //--- setup groups

    u8 * data_ptr = szs->data + data_offset;
    brres_group_t * grp = BRRES_ROOT_GROUP(broot);

    int dir_index;
    for ( dir_index = 0; dir_index < n_directories; dir_index++ )
    {
	u16 dir_id = dir_order[dir_index];
	szs_subfile_t *f_ptr = szs->subfile.list;
	while ( f_ptr < f_end && f_ptr->dir_id != dir_id )
	    f_ptr++;

	int n_entries = 0;
	szs_subfile_t *f_temp;
	for ( f_temp = f_ptr; f_temp < f_end; f_temp++ )
	    if ( f_temp->dir_id == dir_id )
		n_entries++;

	noPRINT("----- NEW GRP #%u: N=%u, %zx -----\n",
		dir_id, n_entries, (u8*)grp - szs->data );

	//--- set entry point

	noPRINT("EP-READ  #%zu: %zx-%x := %zx\n",
			ep_read - ep_list,
			ep_read->dest ? ep_read->dest - szs->data : 0,
			ep_read->delta,
			(u8*)grp - szs->data - ep_read->delta );
	if (ep_read->dest)
	    endian->wr32( ep_read->dest, (u8*)grp - szs->data - ep_read->delta );
	ep_read++;


	//--- setup info list

	memset(info_list,0,info_size);
	info_list->id = 0xffff;
	brres_info_t *info_ptr = info_list + 1;


	//--- setup group

	endian->wr32(&grp->size, sizeof(brres_group_t)
				+ (n_entries+1) * sizeof(brres_entry_t) );
	endian->wr32(&grp->n_entries,n_entries);

	brres_entry_t * entry = grp->entry;
	endian->wr16(&entry->id,0xffff);
	entry++;

	//--- iterate files

	for ( ; f_ptr < f_end; f_ptr++ )
	{
	    if ( f_ptr->dir_id != dir_id )
		continue;

	    //--- write name data

	    IsolateFilename(path_buf,sizeof(path_buf),f_ptr->path,f_ptr->fform);
	    const u32 str_off = FindStringPool(&szs->string_pool,path_buf,0);
	    endian->wr32( &entry->name_off, (ccp)szs->data + str_off - (ccp)grp );

	    info_ptr->name = (ccp)szs->string_pool.data + str_off - str_offset;
	    info_ptr->nlen = strlen(info_ptr->name);
	    info_ptr++;
	    DASSERT( info_ptr - info_list <= szs->subfile.used );

	    //---

	    CalcEntryBRRES( info_list, entry - grp->entry );

	    //---

	    if (f_ptr->is_dir)
	    {
		ep_write->dest  = (u8*)&entry->data_off;
		ep_write->delta = (u8*)grp - szs->data;
		noPRINT("EP-WRITE #%zu: %zx-%x\n",
			ep_write - ep_list,
			ep_write->dest - szs->data, ep_write->delta );
		ep_write++;
		DASSERT( ep_write - ep_list <= n_directories );
	    }
	    else
	    {
		endian->wr32(&entry->data_off,(ccp)data_ptr-(ccp)grp);
		u32 data_size = f_ptr->size;
		memcpy(data_ptr,file_data+f_ptr->offset,data_size);

		szs_file_t sub;
// [[fname+]]
		InitializeSubSZS( &sub,szs, data_ptr - szs->data, data_size,
					FF_UNKNOWN, f_ptr->path, false );
		AdjustStringsBRSUB(&sub,data_ptr,data_size);
		ResetSZS(&sub);

		brsub_list_t * list;
		for ( list = szs->brsub_list; list; list = list->next )
		    if ( list->data == file_data + f_ptr->offset )
		    {
			noPRINT("ADJUST\n");
			DASSERT( list->size <= f_ptr->size );
			data_size = list->size;
			memset( data_ptr + data_size, 0, f_ptr->size - data_size );
			break;
		    }

		data_ptr += ALIGN32(data_size,opt_align_brres);
	    }

	    entry++;
	}

	//--- transfer data

	info_ptr = info_list;
	brres_entry_t * entry_ptr = grp->entry;
	for ( ; entry_ptr < entry; entry_ptr++, info_ptr++ )
	{
	    endian->wr16( &entry_ptr->id,        info_ptr->id );
	    endian->wr16( &entry_ptr->left_idx,  info_ptr->left_idx );
	    endian->wr16( &entry_ptr->right_idx, info_ptr->right_idx );
	}

	grp = (brres_group_t*)entry;
    }


    //--- copy new string pool

    memcpy( szs->data + str_offset, szs->string_pool.data, szs->string_pool.size );


    //--- clean

    if (logging)
	dump_brres_group(broot,endian);

    FREE(ep_list);
    FREE(info_list);
    FREE(dir_order);
    FREE(file_data);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			brres file iterator		///////////////
///////////////////////////////////////////////////////////////////////////////

static int iterate_brres_group
(
    szs_iterator_t	* it,		// valid iterator
    const brres_group_t	* grp,		// valid grp
    uint		path_len	// current path length
)
{
    DASSERT(it);
    DASSERT(it->szs);
    DASSERT(it->func_it);
    DASSERT(it->endian);
    DASSERT(it->root_end);
    DASSERT(grp);

    szs_file_t * szs = it->szs;

    const uint base_dir_index		= it->index;
    const uint n_entries		= it->endian->rd32(&grp->n_entries);
    const brres_entry_t * entry		= grp->entry + 1;
    const brres_entry_t * entry_end	= entry + n_entries;
    int stat				= 0;

 #if USE_ITERATOR_PARAM
    if (it->itpar.cut_files)
 #else
    if (it->cut_files)
 #endif
    {
	it->index	= 0;
	it->fst_item	= 0;
	it->is_dir	= 0;

	it->off		= (u8*)grp - szs->data;
	it->size	= (u8*)entry_end - (u8*)grp;
	StringCopyE(it->path+path_len,it->path+sizeof(it->path),".BRRES.group");
	it->func_it(it,false);
    }

    for ( ; entry < entry_end && !stat; entry++ )
    {
	it->name = 0;
	ccp name = "";
	int name_len = 0;
	uint new_path_len = path_len;
	const uint name_off = it->endian->rd32(&entry->name_off);

	if ( name_off && name_off < szs->size )
	{
	    it->name = name = (ccp)grp + name_off;
	    name_len = it->endian->rd32(name-4);
	    if ( name_len < 0 )
		name_len = strlen(name);
	    if ( name_len < 1 || name_off + name_len > szs->size )
		name_len = 0;
	}

	char name_buf[20];
	if ( name_len < 1 )
	{
	    name_len = snprintf(name_buf,sizeof(name_buf),"_file_%zu",entry-grp->entry);
	    name = name_buf;
	    it->name = 0;
	}

	if ( path_len + name_len < sizeof(it->path)-2 )
	{
	    it->trail_path = it->path+path_len;
	    memcpy(it->trail_path,name,name_len);
	    new_path_len += name_len;
	    it->path[new_path_len] = 0;
	}
	
	const u8 * data = (u8*)grp + it->endian->rd32(&entry->data_off);
	it->index++;
	if ( data < it->root_end )
	{
	    it->is_dir	= true;
	    it->off	= base_dir_index;
	    it->size	= 0;
	    it->path[new_path_len++] = '/';
	    it->path[new_path_len] = 0;
	    stat = it->func_it(it,false);
	    if (!stat)
		stat = iterate_brres_group(it,(brres_group_t*)data,new_path_len);
	}
	else
	{
	    it->is_dir	= false;
	    it->off	= (u8*)grp - szs->data + it->endian->rd32(&entry->data_off);
	    it->size	= it->endian->rd32(szs->data+it->off+4);

	    if ( szs->min_data_off > it->off )
		 szs->min_data_off = it->off;

	    const u32 end_off = it->off + it->size;
	    if ( szs->max_data_off < end_off )
		 szs->max_data_off = end_off;

	    if ( opt_ext && it->szs->fform_arch == FF_BRRES )
	    {
// [[analyse-magic]]
		it->fform = GetByMagicFF(szs->data+it->off,it->size,it->size);
		ccp ext = opt_ext > 1
				? GetMagicExtFF(0,it->fform)
				: GetExtFF(0,it->fform);
		StringCopyE( it->path+new_path_len, it->path+sizeof(it->path), ext );
	    }

	    noPRINT("DATA-OFF: %x .. %x\n",szs->min_data_off,szs->max_data_off);
	    stat = it->func_it(it,false);
	    it->fform = 0;
	}
    }
    it->name = 0;

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

int IterateFilesBRRES
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

    //--- brres header

    if ( !szs->data || szs->size < sizeof(brres_header_t) )
	return -1;

    const brres_header_t * bh = (brres_header_t*)szs->data;
    if ( !IsRepairMagic() && memcmp(bh->magic,BRRES_MAGIC,sizeof(bh->magic)) )
	return -1;

    const endian_func_t * endian = GetEndianFunc(&bh->bom);
    if (!endian)
	return -1;
    szs->endian = endian;

    const uint data_size = endian->rd32(&bh->size);
    const uint root_off  = endian->rd16(&bh->root_off);
    PRINT("data_size=%u, root_off=%u, n_sections=%u\n",
		data_size,root_off, endian->rd16(&bh->n_sections));
    if ( data_size > szs->size || root_off >= szs->size )
	return -1;


    //--- root

    const brres_root_t * root = (brres_root_t*)(szs->data+root_off);
    if ( !IsRepairMagic() && memcmp(root->magic,BRRES_ROOT_MAGIC,sizeof(root->magic)) )
	return -1;

    const uint root_size = endian->rd32(&root->size);
    const u8 * root_end  = (u8*)root + root_size;


    //----- cut files?

 #if USE_ITERATOR_PARAM
    if (it->itpar.cut_files)
 #else
    if (it->cut_files)
 #endif
    {
	it->index	= 0;
	it->fst_item	= 0;
	it->is_dir	= 0;

	it->off		= 0;
	it->size	= sizeof(brres_header_t);
	StringCopyS(it->path,sizeof(it->path),".BRRES.header");
	it->func_it(it,false);

	it->off		= (u8*)root - szs->data;
	it->size	= sizeof(brres_root_t);
	StringCopyS(it->path,sizeof(it->path),".BRRES.root");
	it->func_it(it,false);
    }


    //--- iterate groups

 #if defined(TEST) || 1
    if (logging)
	dump_brres_group(root,endian);
 #endif

    szs->max_data_off	= 0;
    szs->min_data_off	= ~(u32)0;
    it->endian		= endian;
    it->root_end	= root_end;
    int stat = iterate_brres_group(it,BRRES_ROOT_GROUP(root),0);

    if ( !stat && szs->max_data_off )
    {
	it->index++;
	it->is_dir	= false;
	it->off		= szs->max_data_off;
	it->size	= data_size - szs->max_data_off;
	StringCopyS(it->path,sizeof(it->path),BRRES_STRING_POOL_FNAME);
	stat = it->func_it(it,false);
    }
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			brres sub file			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetStringBRSUB
(
    const void		* data,		// relevant data
    const void		* end_of_src,	// end of 'data', 0 if unknown
    u32			min_sp_off,	// minimum string pool offset
					//   maybe end of sub file, 0 if unknown
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid,	// result if invalid
    const endian_func_t * endian	// NULL (=use be) or endian functions
)
{
    DASSERT(data);
    DASSERT(base);

    u32 data_size = end_of_src ? end_of_src - data : ~(u32)0;
    if ( offset && !(offset&3) && offset < data_size && data )
    {
	offset += (u8*)base - (u8*)data;
	if ( offset >= min_sp_off && offset < data_size )
	{
	    ccp ptr = (ccp)data + offset;
	    if (!endian)
		endian = &be_func;
	    if ( endian->rd32(ptr-4) == strlen(ptr) )
		return ptr;
	}
    }
    return if_invalid;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetStringIteratorBRSUB
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    const void		* base,		// pointer to base for calculation
    const void		* ptr,		// pointer to u32 string offset
    ccp			if_null,	// result if offset == NULL
    ccp			if_invalid	// result if invalid
)
{
    u32 offset = it->endian->rd32(ptr);
    return !offset
	? if_null
	: GetStringBRSUB(
		it->brsub,
		(u8*)it->brsub + it->file_size,
		it->static_strings ? 0 : it->data_size,
		base,
		offset,
		if_invalid,
		it->endian );
}

///////////////////////////////////////////////////////////////////////////////

ccp GetStringBRRES
(
    const szs_file_t	* szs,		// base BRRES file
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid	// result if invalid
)
{
    // this functions find all strings, not only relocation candidates

    return szs
	? GetStringBRSUB(
		szs->data,
		szs->data + szs->size,
		0, //szs->max_data_off,
		base,
		offset,
		if_invalid,
		szs->endian )
	: if_invalid;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRSUB iterator			///////////////
///////////////////////////////////////////////////////////////////////////////

static int brsub_string_func
(
    brsub_cut_t		*bcut,		// pointer to data structure
    int			grp,		// index of group, <0: grp_entry_t
    int			entry		// index of entry, -1: no entry
)
{
    noPRINT("##A### XXX( %2d, %2d, %s )\n",grp,entry);

    DASSERT(bcut);
    szs_iterator_t *it = bcut->it;
    DASSERT(it);
    brsub_iterator_t *brit = bcut->param;
    DASSERT(brit);
    const endian_func_t * endian = it->endian;
    DASSERT(endian);

    if ( entry == ENTRY_IDX_GROUP )
    {
	PRINT_IF( bcut->fform == FF_PAT,
		"#PAT# g+e=%d,%d | %s #%u\n",grp,entry,__FUNCTION__,__LINE__);
	if (brit->grp_func)
	    it->index += brit->grp_func(brit,bcut->gptr,grp);
    }
    else if ( grp >= 0 && entry == ENTRY_IDX_DATA && !brit->raw_func )
    {
	PRINT_IF( bcut->fform == FF_PAT,
		"#PAT# g+e=%d,%d v=%d | %s #%u\n",
		grp, entry, bcut->version, __FUNCTION__, __LINE__ );

	u8 *data = (u8*)bcut->data + it->off;
	switch(BRSUB_MODE3(bcut->fform,bcut->version,grp))
	{
	 case BRSUB_MODE3(FF_PAT,4,0):
	  {
	    pat_analyse_t ana;
	    IsValidPAT(bcut->data,bcut->size,bcut->size,0,0,0,&ana);
	    PRINT("\e[32m#PAT# grp=%d val=%d,%d | %s #%u\e[0m\n",
			grp, ana.valid, ana.data_complete,
			__FUNCTION__, __LINE__ );
	    if (!ana.data_complete)
		break;

	    HEXDUMP16(10,0,data,it->size);
	    uint i;
	    for ( i = 0; i < ana.n_sect0; i++ )
	    {
		pat_s0_belem_t *elem = ana.s0_base->elem + i;
		it->index += brit->string_func(brit,data,&elem->offset_name);
	    }

	    for ( i = 0; i < PAT_MAX_ELEM; i++ )
	    {
		pat_s0_sref_t *sref = ana.s0_sref[i];
		if (sref)
		    it->index += brit->string_func(brit,(u8*)sref,&sref->offset_name);
	    }
	    HEXDUMP16(10,0,data,it->size);
	  }
	  break;

	 case BRSUB_MODE3(FF_PAT,4,1):
	  {
	    pat_analyse_t ana;
	    IsValidPAT(bcut->data,bcut->size,bcut->size,0,0,0,&ana);
	    PRINT("\e[32m#PAT# grp=%d val=%d,%d | %s #%u\e[0m\n",
			grp, ana.valid, ana.data_complete,
			__FUNCTION__, __LINE__ );
	    if (!ana.data_complete)
		break;

	    HEXDUMP16(10,0,data,it->size);
	    uint i;
	    for ( i = 0; i < ana.n_sect1; i++ )
		it->index += brit->string_func(brit,data,(u32*)data+i);
	    HEXDUMP16(10,0,data,it->size);
	  }
	  break;
	}
    }
    else if ( grp < 0 || entry < 0 )
    {
	PRINT_IF( bcut->fform == FF_PAT,
		"#PAT# g+e=%d,%d raw=%p | %s #%u\n",
		grp,entry, brit->raw_func, __FUNCTION__, __LINE__ );
	if ( entry == ENTRY_IDX_DATA && brit->raw_func )
	    it->index += brit->raw_func( brit, (u8*)bcut->data + it->off, it->size );
    }
    else
    {
	PRINT_IF( bcut->fform == FF_PAT,
		"#PAT# g+e=%d,%d | %s #%u\n",grp,entry,__FUNCTION__,__LINE__);
	entry++;
	u8 *edata = (u8*)bcut->gptr + endian->rd32(&bcut->eptr->data_off);
	if (brit->entry_func)
	    it->index += brit->entry_func(brit,bcut->gptr,grp,bcut->eptr,edata);

	if (brit->string_func)
	{
	  it->index += brit->string_func(brit,(u8*)bcut->gptr,&bcut->eptr->name_off);

	  u32 data_off = endian->rd32(&bcut->eptr->data_off);
	  u8 *data = (u8*)bcut->gptr + data_off;
	  const u8 *data_end = bcut->data + bcut->file_size;

	  if ( data_off && data < data_end )
	  {
	    if ( bcut->fform == FF_MDL && bcut->version == 11 )
	    {
	      switch(grp)
	      {
		case 1:
		    it->index += brit->string_func(brit,data,(u32*)(data+0x08));
		    break;

		case 2:
		case 3:
		case 4:
		case 5:
		    it->index += brit->string_func(brit,data,(u32*)(data+0x0c));
		    break;

		case 8:
		    {
			it->index += brit->string_func(brit,data,(u32*)(data+0x08));

			const uint n_layer = endian->rd32(data+0x2c);
			u32 layer_off = endian->rd32(data+0x30);
			uint i;
			for ( i = 0; i < n_layer; i++, layer_off += 0x34 )
			{
			    u8 * d = data + layer_off;
			    it->index += brit->string_func(brit,d,(u32*)d);
			}
		    }
		    break;

		case 10:
		    it->index += brit->string_func(brit,data,(u32*)(data+0x38));
		    break;
	      }
	    }
	    else if ( grp == 0 )
	    {
	      switch(BRSUB_MODE2(bcut->fform,bcut->version))
	      {
		case BRSUB_MODE2(FF_CHR,3):
		case BRSUB_MODE2(FF_CHR,5):
		case BRSUB_MODE2(FF_CLR,4):
		case BRSUB_MODE2(FF_SRT,4):
		case BRSUB_MODE2(FF_SRT,5):
		    it->index += brit->string_func(brit,data,(u32*)(data+0x00));
		    break;

		case BRSUB_MODE2(FF_SHP,4):
		    it->index += brit->string_func(brit,data,(u32*)(data+0x04));
		    break;

		case BRSUB_MODE2(FF_SCN,4):
		case BRSUB_MODE2(FF_SCN,5):
		    it->index += brit->string_func(brit,data,(u32*)(data+0x20));
		    break;

		case BRSUB_MODE2(FF_PAT,4):
		    DASSERT(0);
		    break;

		default:
		    break;
	      } // switch
	    } // else
	  } // if
	} // if string_func
    } // else entry >= 0

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// [[IterateStringsBRSUB]]

int IterateStringsBRSUB
(
    brsub_iterator_t	* it		// valid iterator data
)
{
    DASSERT(it);
    DASSERT(it->szs);
    DASSERT( it->szs->file_size > 0 );
    DASSERT( it->szs->file_size < 0x1000000 );
    DASSERT( it->file_size > 0 );
    DASSERT( it->file_size < 0x1000000 );

    const endian_func_t * endian = it->endian;
    if (!endian)
	it->endian = endian = it->szs && it->szs->endian ? it->szs->endian : &be_func;

    file_format_t fform;
    if (it->force_fform)
	fform = it->force_fform;
    else
    {
// [[analyse-magic]]
	fform = GetByMagicFF(it->brsub,it->file_size,it->file_size);
	if ( IsValidBRSUB( it->brsub, it->file_size, it->file_size,
			it->adjust ? 0 : it->szs,
			fform, false, 0, endian) >= VALID_ERROR )
	{
	    PRINT_IF(it->adjust,"INVALID, adjust=%d\n",it->adjust);
	    return -1;
	}
    }

    brsub_header_t *bh	= it->brsub;
    u8 * brsub_data	= (u8*)bh;
    it->data_size	= endian->rd32(&bh->size);
    const int n_grp	= GetSectionNumBRSUB(brsub_data,it->data_size,endian);
    u32 name_off	= 0x10 + n_grp * 4;
    uint count		= 0;

    PRINT_IF( fform == FF_PAT, "#PAT# %s @ %s #%u | n_grp=%d\n",
		__FUNCTION__,__FILE__,__LINE__,n_grp);


    //--- static data

    it->fform = fform;

    if (it->offset_func)
	count += it->offset_func(it,brsub_data,&bh->brres_offset);

    if ( it->string_func && n_grp >= 0 )
	count += it->string_func(it,brsub_data,(u32*)(brsub_data+name_off));

    it->fform = FF_UNKNOWN;

    szs_iterator_t it2 = {0};
    it2.szs		= it->szs;
    it2.size		= it->file_size;
    it2.endian		= it->endian;
    //it2.clean_path	= ;
 #if USE_ITERATOR_PARAM // [[itpar]] copy itpar
    it2.itpar.cut_files	= true;
 #else
    it2.cut_files	= true;
 #endif
    CutFilesBRSUB( &it2, brsub_string_func, it, 0, 0 );
    return count + it2.index;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		BRSUB iterator callback functions	///////////////
///////////////////////////////////////////////////////////////////////////////

static int clear_brsub_offset
(
    struct brsub_iterator_t	* it,	// iterator struct with all infos
    u8				* base,	// pointer to base for calculation
    u32				* ptr	// pointer to u32 string offset
)
{
    DASSERT(it);
    DASSERT(it->endian);
    it->endian->wr32(ptr,0);
    return 0;
}

//-----------------------------------------------------------------------------

static int adjust_brsub_offset
(
    struct brsub_iterator_t	* it,	// iterator struct with all infos
    u8				* base,	// pointer to base for calculation
    u32				* ptr	// pointer to u32 offset
)
{
    DASSERT(it);
    DASSERT(it->endian);
    DASSERT(it->szs);
    DASSERT(it->szs->parent);
    DASSERT(it->szs->parent->data);

    noPRINT("##A## ADJUST OFFSET: %x -> %x\n",
		it->endian->rd32(ptr), (u32)(it->szs->parent->data - base) );
    it->endian->wr32( ptr, it->szs->parent->data - base );
    return 0;
}

//-----------------------------------------------------------------------------

static int insert_brsub_string
(
    struct brsub_iterator_t	* it,	// iterator struct with all infos
    u8				* base,	// pointer to base for calculation
    u32				* ptr	// pointer to u32 string offset
)
{
    DASSERT(it);
    DASSERT(it->endian);
    DASSERT(it->param);

    ccp string = GetStringIteratorBRSUB(it,base,ptr,0,0);
    if (!string)
	return 0;
    PRINT_IF( it->fform == FF_PAT, "#PAT# INSERT %s @%s\n",string,__FUNCTION__);
    PRINT_IF( it->fform == FF_MDL, "#MDL# INSERT %s @%s\n",string,__FUNCTION__);

    InsertStringPool(it->param,string,false,0);
    return 1;
}

//-----------------------------------------------------------------------------

static int adjust_brsub_string_szs
(
    struct brsub_iterator_t	* it,	// iterator struct with all infos
    u8				* base,	// pointer to base for calculation
    u32				* ptr	// pointer to u32 string offset
)
{
    noPRINT("##A## size=%x,%x, base=%zx, ptr=%zx\n",
	it->data_size, it->file_size,
	base - (u8*)it->brsub, (u8*)ptr - (u8*)it->brsub );

    DASSERT(it);
    DASSERT(it->endian);
    DASSERT(it->szs);
    DASSERT(it->szs->parent);
    DASSERT(it->szs->parent->data);
    DASSERT(it->param);

    ccp string = GetStringIteratorBRSUB(it,base,ptr,0,0);
    if (!string)
	return 0;

    const u32 off = FindStringPool(it->param,string,0) - (base - it->szs->parent->data);
    noPRINT("##A## ADJUST STRING: %x -> %x : %s\n", it->endian->rd32(ptr), off, string );
    PRINT_IF( it->fform == FF_PAT,
		"#PAT# ADJUST %x -> %x : %s @%s\n",
		it->endian->rd32(ptr), off, string, __FUNCTION__ );
    it->endian->wr32(ptr,off);
    return 1;
}

//-----------------------------------------------------------------------------

static int adjust_brsub_string
(
    struct brsub_iterator_t	* it,	// iterator struct with all infos
    u8				* base,	// pointer to base for calculation
    u32				* ptr	// pointer to u32 string offset
)
{
    DASSERT(it);
    DASSERT(it->endian);
    DASSERT(it->szs);
    DASSERT(it->szs->data);
    DASSERT(it->param);

    ccp string = GetStringIteratorBRSUB(it,base,ptr,0,0);
    if (!string)
	return 0;

    const u32 off = FindStringPool(it->param,string,0) - (base - (u8*)it->brsub);
    noPRINT("ADJUST STRING: %x -> %x : %s\n", it->endian->rd32(ptr), off, string );
    it->endian->wr32(ptr,off);
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    BRSUB iterator interface		///////////////
///////////////////////////////////////////////////////////////////////////////

int CollectStringsBRSUB
(
    string_pool_t	* sp,		// string pool
    bool		init_sp,	// true: initialize 'sp'
    szs_file_t		* szs,		// szs of data
    bool		adjust		// true: create string pool and
					//       adjust offsets and strings
)
{
    DASSERT(sp);
    DASSERT(szs);

    if (init_sp)
	InitializeStringPool(sp);

    brsub_iterator_t it;
    memset(&it,0,sizeof(it));
    it.szs		= szs;
    it.brsub		= (void*)szs->data;
    it.file_size	= szs->file_size;
    it.string_func	= insert_brsub_string;
    it.offset_func	= adjust ? clear_brsub_offset : 0;
    it.param		= sp;
    it.endian		= szs->endian;

    const int stat = IterateStringsBRSUB(&it);

    if ( adjust && stat > 0 )
    {
	CalcStringPool( sp, it.data_size, szs->endian );
	it.adjust	= true;
	it.string_func	= adjust_brsub_string;
	it.offset_func	= 0;
	IterateStringsBRSUB(&it);
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int AdjustStringsBRSUB
(
    szs_file_t		* szs,		// base BRRES file
    void		* brsub_data,	// brres sub data
    size_t		brsub_size	// size of 'brsub_data' including string pool
)
{
    DASSERT(szs);
    DASSERT(szs->parent);
    DASSERT(brsub_data);

    brsub_iterator_t it;
    memset(&it,0,sizeof(it));
    it.szs		= szs;
    it.brsub		= brsub_data;
    it.file_size	= brsub_size;
    it.string_func	= adjust_brsub_string_szs;
    it.offset_func	= adjust_brsub_offset;
    it.param		= &szs->parent->string_pool;
    it.adjust		= true;
    it.endian		= szs->endian;

    return IterateStringsBRSUB(&it);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BRSUB Dump Structure		///////////////
///////////////////////////////////////////////////////////////////////////////

static int dump_brsub_raw
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* data,		// data pointer
    u32			data_size	// size of 'data'
)
{
    DASSERT(it);
    FILE * f = it->param;
    DASSERT(f);

    u32 data_offset = data - (u8*)it->brsub;
    fprintf(f,"    - RAW DATA %x .. %x, size %x\n",
		data_offset, data_offset + data_size, data_size );
    return 0;
}

//-----------------------------------------------------------------------------

static int dump_brsub_grp
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    brres_group_t	* grp,		// pointer to group
    uint		grp_index	// index of group
)
{
    DASSERT(it);
    FILE * f = it->param;
    DASSERT(f);

    fprintf(f,"    - GRP #%-2u %4x\n", grp_index, (int)( (u8*)grp - (u8*)it->brsub ));
    return 0;
}

//-----------------------------------------------------------------------------

static int dump_brsub_entry
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    brres_group_t	* grp,		// pointer to group
    uint		grp_index,	// index of group
    brres_entry_t	* entry,	// pointer to entry
    u8			* data		// pointer to entry related data
)
{
    DASSERT(it);
    FILE * f = it->param;
    DASSERT(f);

    ccp string = GetStringIteratorBRSUB(it,grp,&entry->name_off,
				"<ignored>", "**invalid**");

    fprintf(f,"      - ENTRY #%-2u %4x [+%3x] : %8x %s-> %s\n",
		(int)( entry - grp->entry ),
		(int)( (u8*)entry - (u8*)it->brsub ),
		(int)( (u8*)entry - (u8*)grp ),
		it->endian->rd32(&entry->name_off),
		string <= (ccp)it->brsub + it->data_size ? "[intern] " : "",
		string );
    return 0;
}

//-----------------------------------------------------------------------------

static int dump_brsub_string
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* base,		// pointer to base for calculation
    u32			* ptr		// pointer to u32 string offset
)
{
    DASSERT(it);
    FILE * f = it->param;
    DASSERT(f);

    ccp string = GetStringIteratorBRSUB(it,base,ptr,"<ignored>","**invalid**");

    fprintf(f,"        > STRING %5x [+%3x] : %8x %s-> %s\n",
		(int)( (u8*)ptr - (u8*)it->brsub ),
		(int)( (u8*)ptr - base ),
		it->endian->rd32(ptr),
		string <= (ccp)it->brsub + it->data_size ? "[intern] " : "",
		string );
    return 0;
}

//-----------------------------------------------------------------------------

static int dump_brsub_offset
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* base,		// pointer to base for calculation
    u32			* ptr		// pointer to u32 string offset
)
{
    DASSERT(it);
    FILE * f = it->param;
    DASSERT(f);

    fprintf(f,"        > OFFSET %5x [+%3x] : %8x\n",
		(int)( (u8*)ptr - (u8*)it->brsub ),
		(int)( (u8*)ptr - base ),
		it->endian->rd32(ptr) );
    return 0;
}

//-----------------------------------------------------------------------------

int DumpStructureBRSUB
(
    FILE		* f,		// output file
    szs_file_t		* base_szs,	// valid base SZS
    uint		off,		// offset of selected data within 'base'
    uint		size,		// not NULL: size of selected data
    file_format_t	fform		// not FF_UNKNOWN: use this type
)
{
    DASSERT(f);
    DASSERT(base_szs);
    PRINT("DumpStructureBRSUB(off=%x,size=%x,ff=%d)\n",off,size,fform);

    szs_file_t szs;
// [[fname-]]
    InitializeSubSZS(&szs,base_szs,off,size,fform,0,false);

    brsub_iterator_t it;
    memset(&it,0,sizeof(it));
    it.szs		= &szs;
    it.brsub		= (void*)szs.data;
    it.file_size	= szs.file_size;
    it.raw_func		= dump_brsub_raw;
    it.grp_func		= dump_brsub_grp;
    it.entry_func	= dump_brsub_entry;
    it.string_func	= dump_brsub_string;
    it.offset_func	= dump_brsub_offset;
    it.param		= f;
    it.static_strings	= true;

    return IterateStringsBRSUB(&it);
}

///////////////////////////////////////////////////////////////////////////////

static int dump_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if ( term || it->is_dir )
	return 0;

    szs_file_t * szs = it->szs;
    DASSERT(szs);

    u8 * data = szs->data+it->off;
// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,it->size,it->size);
    if (IsBRSUB(fform))
	fprintf(it->param,"\n  * Sub file: %s.%u:%s\n",
		    GetNameFF(fform,fform),
		    it->endian->rd32(data+8),
		    it->path );
    else
	fprintf(it->param,"\n  * Data: %s\n",it->path);

    return DumpStructureBRSUB(it->param,szs,it->off,it->size,fform);
}

///////////////////////////////////////////////////////////////////////////////

int DumpStructureBRRES
(
    FILE		* f,		// output file
    szs_file_t		* szs		// BRRES file
)
{
    DASSERT(f);
    DASSERT(szs);

    if ( szs->fform_arch != FF_BRRES )
	return -1;

    return IterateFilesParSZS(szs,dump_func,f,false,false,0,-1,SORT_NONE);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			NAME-REF support		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeNameRef ( name_ref_t *nr )
{
    DASSERT(nr);
    memset(nr,0,sizeof(*nr));
}

///////////////////////////////////////////////////////////////////////////////

void ResetNameRef ( name_ref_t *nr )
{
    DASSERT(nr);

    if (nr->vlist)
    {
	name_ref_var_t *vptr, *vend = nr->vlist + nr->vused;
	for ( vptr = nr->vlist; vptr < vend; vptr++ )
	{
	    FreeString(vptr->var_name);

	    if (vptr->elist)
	    {
		name_ref_elem_t *eptr, *eend = vptr->elist + vptr->eused;
		for ( eptr = vptr->elist; eptr < eend; eptr++ )
		    FreeString(eptr->info);

		FREE(vptr->elist);
		vptr->elist = 0;
	    }
	    vptr->eused = vptr->esize = 0;

	}
	FREE(nr->vlist);
	nr->vlist = 0;
    }
    nr->vused = nr->vsize = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void add_ref_string
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* base,		// pointer to base for calculation
    u32			* ptr,		// pointer to u32 string offset

    ccp			format,		// format string for info
    ...					// arguments for 'format'
)
__attribute__ ((__format__(__printf__,4,5)));

//-----------------------------------------------------------------------------

static void add_ref_string
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* base,		// pointer to base for calculation
    u32			* ptr,		// pointer to u32 string offset

    ccp			format,		// format string for info
    ...					// arguments for 'format'
)
{
    ccp info = 0;
    if (format)
    {
	char buf[200];
	va_list arg;
	va_start(arg,format);
	int len = vsnprintf(buf,sizeof(buf),format,arg);
	va_end(arg);
	if ( len > 0 )
	    info = MEMDUP(buf,len);
    }

    name_ref_t *nr = it->param;
    DASSERT(nr);

    ccp var_name	= GetStringIteratorBRSUB(it,base,ptr,0,0);
    const u32 var_off	= be32(ptr) + base - nr->data;
    const u32 file_off	= (u8*)ptr - nr->data;
    const u32 base_off	= (u8*)ptr - it->szs->data;

    PRINT0(">>> %06x %05x %06x %s [%s]\n",
		base_off, file_off, var_off, var_name, info );


    //--- insert into vlist

    uint vi;
    name_ref_var_t *vptr = nr->vlist;
    for ( vi = 0; vi < nr->vused; vi++, vptr++ )
	if ( var_off <= vptr->var_off )
	    break;

    if ( vi == nr->vused || var_off != vptr->var_off )
    {
	// insert element
	if ( nr->vused == nr->vsize )
	{
	    nr->vsize += 100;
	    nr->vlist = REALLOC( nr->vlist, nr->vsize * sizeof(*nr->vlist) );
	    vptr = nr->vlist + vi;
	}

	if ( vi < nr->vused )
	    memmove(vptr+1,vptr,(nr->vused-vi)*sizeof(*vptr));

	memset(vptr,0,sizeof(*vptr));
	vptr->var_name     = var_name ? STRDUP(var_name) : EmptyString;
	vptr->var_off      = var_off;
	vptr->min_file_off = file_off;
	nr->vused++;
    }
    else if ( file_off < vptr->min_file_off )
	vptr->min_file_off = file_off;


    //--- insert into elist

    uint ei;
    name_ref_elem_t *eptr = vptr->elist;
    for ( ei = 0; ei < vptr->eused; ei++, eptr++ )
	if ( file_off < eptr->file_off )
	    break;

    if ( vptr->eused == vptr->esize )
    {
	vptr->esize += 20;
	vptr->elist = REALLOC( vptr->elist, vptr->esize * sizeof(*vptr->elist) );
	eptr = vptr->elist + ei;
    }

    if ( ei < vptr->eused )
	memmove(eptr+1,eptr,(vptr->eused-ei)*sizeof(*eptr));

    memset(eptr,0,sizeof(*eptr));
    eptr->file_off  = file_off;
    eptr->base_off  = base_off;
    eptr->info	    = info;
    vptr->eused++;
}

///////////////////////////////////////////////////////////////////////////////

static int insert_name_ref
(
    struct brsub_iterator_t	* it,	// iterator struct with all infos
    u8				* base,	// pointer to base for calculation
    u32				* ptr	// pointer to u32 string offset
)
{
    DASSERT(it);
    DASSERT(it->szs);
    name_ref_t *nr = it->param;
    DASSERT(nr);

    if ( nr->brief > 2 )
	add_ref_string(it,base,ptr,"%s",nr->file_info);
    else
	add_ref_string(it,base,ptr,"%s" "offset %x",
		nr->file_info, (uint)( (u8*)ptr - it->szs->data ));
    return 1;
}

//-----------------------------------------------------------------------------

static void iterate_nr_brsub
(
    name_ref_t		*nr,		// valid struct
    szs_file_t		*szs,		// valid struct
    cvp			data,		// pointer to data
    uint		data_size,	// size of data
    file_format_t	fform		// FF_UNKNOWN or file format of 'data'
)
{
    DASSERT(nr);
    DASSERT(szs);
    DASSERT(data||!data_size);

    if ( !data || !data_size )
	return;

    if ( fform == FF_UNKNOWN )
	fform = AnalyseMagicByOpt(0,data,data_size,data_size,FF_UNKNOWN,0);

    if (IsBRSUB(fform))
    {
	if ( nr->brief > 2 )
	    snprintf(nr->file_info,sizeof(nr->file_info),"%s",
		GetNameFF(0,fform) );
	else
	    snprintf(nr->file_info,sizeof(nr->file_info),"%s @%x, ",
		GetNameFF(0,fform), (uint)( (u8*)data - szs->data ));

	szs_file_t sub;
	InitializeSubSZS( &sub, szs, (u8*)data - szs->data, 0, fform, 0, false );

	brsub_iterator_t it;
	memset(&it,0,sizeof(it));
	it.szs		= &sub;
	it.brsub	= (void*)data;
	it.file_size	= szs->data + szs->size - (u8*)data;
	it.fform	= fform;
	it.param	= nr;
	it.endian	= szs->endian;
	it.string_func	= insert_name_ref;

	IterateStringsBRSUB(&it);
	ResetSZS(&sub);
	*nr->file_info = 0;
    }
}

//-----------------------------------------------------------------------------

static int iterate_nr_brres
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if (term)
	return 0;

    const u8 * data = it->szs->data+it->off;
    iterate_nr_brsub(it->param,it->szs,data,it->size,FF_UNKNOWN);
    return 0;
}

//-----------------------------------------------------------------------------

enumError CreateNameRef
	( name_ref_t *nr, bool init_nr, szs_file_t *szs, int brief )
{
    DASSERT(nr);
    DASSERT(szs);

    if (init_nr)
	InitializeNameRef(nr);
    else
	ResetNameRef(nr);

    nr->data		= szs->data;
    nr->data_size	= szs->file_size;
    nr->fform		= szs->fform_arch;
    nr->fname		= STRDUP(szs->fname);
    nr->brief		= brief;

    if ( szs->fform_arch == FF_BRRES )
	IterateFilesParSZS( szs, iterate_nr_brres,nr, false,true,0,-1, SORT_NONE );
    else
	iterate_nr_brsub(nr,szs,szs->data,szs->file_size,FF_UNKNOWN);
#if 0
    {
	brsub_iterator_t it;
	memset(&it,0,sizeof(it));
	it.szs		= szs;
	it.brsub	= (void*)szs->data;
	it.file_size	= szs->file_size;
	it.string_func	= insert_name_ref;
	it.param	= nr;
	it.endian	= szs->endian;

	IterateStringsBRSUB(&it);
    }
#endif
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int sort_nr_by_name ( const void *va, const void *vb )
{
    name_ref_var_t **a = (name_ref_var_t**)va;
    name_ref_var_t **b = (name_ref_var_t**)vb;
    return strcasecmp( (*a)->var_name, (*b)->var_name );
}

//-----------------------------------------------------------------------------

int sort_nr_by_brres ( const void *va, const void *vb )
{
    name_ref_var_t **a = (name_ref_var_t**)va;
    name_ref_var_t **b = (name_ref_var_t**)vb;
    return strcmp( (*a)->var_name, (*b)->var_name );
}

//-----------------------------------------------------------------------------

int sort_nr_by_offset ( const void *va, const void *vb )
{
    name_ref_var_t **a = (name_ref_var_t**)va;
    name_ref_var_t **b = (name_ref_var_t**)vb;
    return (int)(*a)->min_file_off - (int)(*b)->min_file_off;
}

//-----------------------------------------------------------------------------

static void list_var ( FILE *f, int indent, name_ref_var_t *vptr, int brief )
{
    DASSERT(f);
    DASSERT(vptr);

    if ( brief > 0 )
	fprintf(f,"\n%*s»%s«\n",indent,"",
		vptr->var_name ? vptr->var_name : "" );
    else
	fprintf(f,"\n%*s%6x  »%s«\n",indent,"",
		vptr->var_off, vptr->var_name ? vptr->var_name : "" );

    name_ref_elem_t *eptr, *eend = vptr->elist + vptr->eused;
    if ( brief > 1 )
	for ( eptr = vptr->elist; eptr < eend; eptr++ )
	    fprintf(f,"%*s  %s\n",indent,"",
		eptr->info ? eptr->info : "" );
    else
	for ( eptr = vptr->elist; eptr < eend; eptr++ )
	    fprintf(f,"%*s  %6x  %s\n",indent,"",
		eptr->file_off, eptr->info ? eptr->info : "" );
}

//-----------------------------------------------------------------------------

void ListNameRef ( FILE *f, int indent, name_ref_t *nr, SortMode_t sort )
{
    qsort_func sort_func;
    ccp sort_info;
    switch (sort)
    {
	case SORT_INAME:
	    sort_func = sort_nr_by_name;
	    sort_info = "sorted by name";
	    break;

	case SORT_BRRES:
	    sort_func = sort_nr_by_brres;
	    sort_info = "sorted like BRRES";
	    break;

	case SORT_OFFSET:
	    sort_func = sort_nr_by_offset;
	    sort_info = "sorted by offset";
	    break;

	default:
	    sort_func = 0;
	    sort_info = "unsorted";
	    break;
    }

    indent = NormalizeIndent(indent);
    printf("%*s" "Name reference, %s, file %s:%s\n",
		indent,"", sort_info, GetNameFF(nr->fform,0), nr->fname );
    indent += 3;

    if ( !sort_func || nr->vused < 2 )
    {
	//-- unsorted

	name_ref_var_t *vptr, *vend = nr->vlist + nr->vused;
	for ( vptr = nr->vlist; vptr < vend; vptr++ )
	    list_var(f,indent,vptr,nr->brief);
    }
    else
    {
	//--- sorted

	name_ref_var_t **ref_list = MALLOC(nr->vused*sizeof(*ref_list));
	DASSERT(ref_list);
	uint i;
	for ( i = 0; i < nr->vused; i++ )
	    ref_list[i] = nr->vlist+i;

	qsort( ref_list, nr->vused, sizeof(*ref_list), sort_func );

	for ( i = 0; i < nr->vused; i++ )
	    list_var(f,indent,ref_list[i],nr->brief);

	FREE(ref_list);
    }

    fputc('\n',f);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


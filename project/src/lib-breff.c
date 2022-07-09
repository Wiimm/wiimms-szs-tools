
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "lib-breff.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    breff+breft file iterator		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesBREFF
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

    //--- bref* header

    if ( !szs->data || szs->size < sizeof(brres_header_t) )
	return -1;

    const brres_header_t * bh = (brres_header_t*)szs->data;
    ccp root_magic = 0;
    u32 add_file_size = 0;
    switch (szs->fform_file)
    {
	case FF_BREFF:
	    root_magic = BREFF_MAGIC;
	    break;

	case FF_BREFT:
	    root_magic = BREFT_MAGIC;
	    add_file_size = 0x20;
	    break;

	default:
	    return -1;
    }

    if (memcmp(bh->magic,root_magic,sizeof(bh->magic)))
	return -1;

    const endian_func_t * endian = GetEndianFunc(&bh->bom);
    if (!endian)
	return -1;
    szs->endian = endian;

    const uint data_size = endian->rd32(&bh->size);
    const uint root_off  = endian->rd16(&bh->root_off);
    TRACE("data_size=%u, root_off=%u, n_sections=%u\n",
		data_size,root_off, endian->rd16(&bh->n_sections));
    if ( data_size > szs->size || root_off >= szs->size )
	return -1;

    // [extend] may be a list of roots (or itemlists)


    //--- analyze root

    const breff_root_head_t * rh = (breff_root_head_t*)( szs->data + sizeof(*bh) );
    if (memcmp(rh->magic,root_magic,sizeof(rh->magic)))
	return -1;

    const uint root_size = endian->rd32(&rh->data_size);
    if ( root_size + sizeof(*bh) > szs->size )
	return -1;

    const breff_root_t * root = (breff_root_t*)rh->data;
    szs->obj_name	= root->name;
    //szs->obj_id	= endian->rd16(&root->unknown3);
    //szs->obj_id_used	= true;
    TRACE("ROOT: val=%x,%x,%x, name[%u]=%s\n",
		endian->rd32(&root->unknown1),
		endian->rd32(&root->unknown2),
		endian->rd16(&root->unknown3),
		endian->rd16(&root->name_len0), szs->obj_name );


    //--- analyze item list

    const uint first_item_off = endian->rd32(&root->first_item_off);
    breff_item_list_t * ilist = (breff_item_list_t*)( rh->data + first_item_off );
    if ( (u8*)ilist >= szs->data + szs->size )
	return -1;
    const uint ilist_size = endian->rd32(&ilist->size);
    const uint n_item     = endian->rd16(&ilist->n_item);
    TRACE("ITEM-LIST: n=%u, size=%x\n",n_item,ilist_size);


    //--- call iterator function for file header

    StringCopyS(it->path,sizeof(it->path)-2,"header.bin");
    it->is_dir	= false;
    it->off	= 0;
    it->size	= (u8*)rh - szs->data;
    int stat	= it->func_it(it,false);
    if (stat)
	return stat;
    it->index++;


    //--- call iterator function for root

    StringCopyS(it->path,sizeof(it->path)-2,"root.bin");
    it->off	= (u8*)rh - szs->data;
    it->size	= (u8*)ilist - (u8*)rh;
    stat	= it->func_it(it,false);
    if (stat)
	return stat;
    it->index++;


    //--- call iterator function for file list

    StringCopyS(it->path,sizeof(it->path)-2,"file-list.bin");
    it->off	= (u8*)ilist - szs->data;
    it->size	= ilist_size;
    stat	= it->func_it(it,false);
    if (stat)
	return stat;
    it->index++;


    //--- call iterator function for 'files' directory

    char * path_ptr = StringCopyS(it->path,sizeof(it->path)-2,"files");
    *path_ptr++ = '/';
    *path_ptr	= 0;
    it->is_dir	= true;
    it->off	= 0;
    it->size	= 0;
    stat	= it->func_it(it,false);
    if (stat)
	return stat;
    it->index++;


    //--- iterate item list

    //**************************************************************
    //***  This pointer is not aligned because of used names!    ***
    //***  It's not problematic if reading via endian functions  ***
    //**************************************************************
    const breff_item_name_t * item = ilist->item;
    const u32 base_off = (u8*)ilist - szs->data;

    uint ii;
    for ( ii = 0; ii < n_item; ii++ )
    {
	const uint namelen0 = endian->rd16(&item->name_len0);
	char *name = (char*)(item+1);
	const breff_item_data_t * idata = (breff_item_data_t*)( name + namelen0 );
	noPRINT("%3u: %6x %6x  %s\n",
		ii,
		endian->rd32(&idata->offset),
		endian->rd32(&idata->size),
		name );

	StringCopyE(path_ptr,it->path + sizeof(it->path), name );
	it->is_dir	= false;
	it->off		= endian->rd32(&idata->offset) + base_off;
	it->size	= endian->rd32(&idata->size) + add_file_size;

	if ( szs->min_data_off > it->off )
	     szs->min_data_off = it->off;

	const u32 max = it->off + it->size;
	if ( szs->max_data_off < max )
	     szs->max_data_off = max;

	const int stat	= it->func_it(it,false);
	if (stat)
	    break;
	it->index++;

	item = (breff_item_name_t*)( (u8*)idata + sizeof(*idata) );
    }
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			create breff			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateBREFF
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			total_size,	// total file size
    SetupParam_t	* setup_param	// NULL or setup parameters
)
{
    DASSERT(szs);
    PRINT("CreateBREFF() ndir=%d, totsize=%x, align=%x,%x\n",
		szs->subfile.used, total_size, opt_align_breff, opt_align_breft );

    enumError max_err = ERR_OK;
    const endian_func_t	* endian = &be_func;
    szs->endian = endian;

    const bool is_breft	= setup_param && setup_param->fform_arch == FF_BREFT;
    szs->fform_arch	= is_breft ? FF_BREFT : FF_BREFF;
    szs->ff_attrib	= GetAttribFF(szs->fform_arch);
    szs->ff_version	= setup_param && setup_param->version > 0
				? setup_param->version : 9;

    uint align;
    if (is_breft)
    {
	align = opt_align_breft
		? opt_align_breft
		: szs->ff_version <= 9
			? BREFT_9_DEFAULT_ALIGN
			: BREFT_11_DEFAULT_ALIGN;
    }
    else
	align = opt_align_breff;


    //--- sort files using 'file-list.bin'

    SortSubFilesSZS(szs,SORT_BREFF);

    if ( logging >= 2 )
    {
	printf("\nsorted file list:\n");
	int i;
	for ( i = 0; i < szs->subfile.used; i++ )
	{
	    szs_subfile_t * f = szs->subfile.list + i;
	    printf("%3d.: %u %6x %6x %s\n", i, f->is_dir, f->offset, f->size, f->path );
	}
    }

    //--- find root name

    ccp root_name = 0;
    bool root_name_alloced = false;

    if ( setup_param && setup_param->object_name && *setup_param->object_name )
    {
	root_name = setup_param->object_name;
	PRINT("ROOT-NAME: %s [setup-param]\n",root_name);
    }
    else
    {
	ccp end = source_dir + strlen(source_dir) - 1;
	while ( end >= source_dir && *end == '/' )
	    end--;
	ccp beg = end++;
	while ( beg >= source_dir && *beg != '/' )
	{
	    if ( *beg == '.' )
	    {
		const uint len = end - beg;
		if (   len == 2 & !memcmp(beg,".d",2)
		    || len == 6 & !memcmp(beg,".breff",6)
		    || len == 6 & !memcmp(beg,".breft",6) )
		{
		    end = beg;
		}
	    }
	    beg--;
	}
	beg++;
	const uint len = end - beg;
	char *name = MALLOC(len+1);
	memcpy(name,beg,len);
	name[len] = 0;
	root_name = name;
	root_name_alloced = true;
	PRINT("ROOT-NAME: %s [path]\n",root_name);
    }


    //--- calculate data size

    uint root_name_size = strlen(root_name) + 1;
    uint root_size = sizeof(breff_root_head_t) + sizeof(breff_root_t) + root_name_size;
    if ( is_breft && root_size < 0x50 )
	root_size = 0x50;
    root_size = ALIGN32(root_size,4);

    uint ilist_size = sizeof(breff_item_list_t);	// size of item list
    uint fdata_size = 0;				// size of file data

    szs_subfile_t *fptr, *fend = szs->subfile.list + szs->subfile.used;
    for ( fptr = szs->subfile.list; fptr < fend; fptr++ )
    {
	//--- name

	ccp fname = strrchr(fptr->path,'/');
	fname = fname ? fname + 1 : fptr->path;
	const uint namelen = strlen(fname) + 1;
	ilist_size += sizeof(breff_item_data_t) + sizeof(breff_item_name_t) + namelen;

	//--- data

	fptr->offset = fdata_size;
	fdata_size = ALIGN32( fdata_size + fptr->size, align );
    }
    ilist_size = ALIGN32( ilist_size, align );
    szs->size = sizeof(brres_header_t) + root_size + ilist_size + fdata_size;

    PRINT("SIZE: root=%x, flist=%x, fdata=%x => %zx = %zu\n",
		root_size, ilist_size, fdata_size, szs->size, szs->size );


    //--- setup data pointer

    szs->data = CALLOC(1,szs->size);

    brres_header_t	*bh	= (brres_header_t*)szs->data;
    breff_root_head_t	*rh	= (breff_root_head_t*)(bh+1);
    breff_root_t	*root	= (breff_root_t*)(&rh->data);
    breff_item_list_t	*ilist	= (breff_item_list_t*)( (u8*)rh + root_size );
    u8			*data	= (u8*)ilist + ilist_size;
    DASSERT( data + fdata_size == szs->data + szs->size );

    PRINT("%8zx file header\n", (u8*)bh - szs->data );
    PRINT("%8zx root header\n", (u8*)rh - szs->data );
    PRINT("%8zx root\n", (u8*)root - szs->data );
    PRINT("%8zx file list\n", (u8*)ilist - szs->data );
    PRINT("%8zx data\n", (u8*)data - szs->data );
    PRINT("%8zx END\n", szs->size );


    //--- setup file header

    ccp magic = is_breft ? BREFT_MAGIC : BREFF_MAGIC;
    memcpy(bh->magic,magic,sizeof(bh->magic));
    endian->wr16(&bh->bom,0xfeff);
    endian->wr16(&bh->version,szs->ff_version);
    endian->wr32(&bh->size,szs->size);
    endian->wr16(&bh->root_off,sizeof(brres_header_t));
    endian->wr16(&bh->n_sections,1);


    //--- setup root

    memcpy(rh->magic,magic,sizeof(bh->magic));
    endian->wr32(&rh->data_size,szs->size-sizeof(*bh)-sizeof(*rh));
    endian->wr32(&root->first_item_off,(u8*)ilist-(u8*)root);
    endian->wr16(&root->name_len0,(root_name_size));
    memcpy(root->name,root_name,root_name_size);


    //--- setup item list

    endian->wr32(&ilist->size,ilist_size);
    endian->wr16(&ilist->n_item,szs->subfile.used);


    //--- load files

    u8 *idata = (u8*)ilist->item;
    for ( fptr = szs->subfile.list; fptr < fend; fptr++ )
    {
	//--- name

	ccp fname = strrchr(fptr->path,'/');
	fname = fname ? fname + 1 : fptr->path;
	const uint namelen = strlen(fname) + 1;
	endian->wr16(idata,namelen);
	idata += 2;
	memcpy(idata,fname,namelen);
	idata += namelen;

	endian->wr32(idata,data-(u8*)ilist);
	idata += 4;
	endian->wr32( idata, fptr->size - ( is_breft ? 0x20 : 0 ) );
	idata += 4;

	//--- data

	PRINT("LOAD: %zx+%x %s\n",data-szs->data,fptr->size,fptr->path);
	if (fptr->data)
	    memcpy(data,fptr->data,fptr->size);
	else
	{
	    enumError err = LoadFILE(source_dir,fptr->path,0,
			data, fptr->size, 0, &szs->fatt,true );
	    if ( max_err < err )
		max_err = err;
	}
	data += ALIGN32( fptr->size, align );
    }


    //--- terminate

    if (root_name_alloced)
	FreeString(root_name);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


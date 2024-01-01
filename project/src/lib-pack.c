
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
 *   Copyright (c) 2011-2024 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include "lib-pack.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IterateFilesPACK()		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesPACK
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

    if ( !szs->data || szs->size <= sizeof(pack_header_t) )
	return -1;

    //--- analyze file header

    const pack_header_t * pack	= (pack_header_t*)szs->data;
    const uint size		= ntohl(pack->file_size);
    const uint n_files		= ntohl(pack->n_files);
    const uint offset_metric	= ntohl(pack->offset_metric);

    if
    (	ntohl(pack->magic) != PACK_MAGIC_NUM
	|| size < szs->size
	|| offset_metric < 0x20
	|| offset_metric + n_files * sizeof(pack_metric_t) >= size
    )
    return -1;


    //--- cut file header

    if (it->itpar.cut_files)
    {
	it->index	= 0;
	it->fst_item	= 0;
	it->is_dir	= 0;

	it->off		= 0;
	it->size	= sizeof(pack_header_t);
	StringCopyS(it->path,sizeof(it->path),".PACK.header");
	it->func_it(it,false);

	it->off		= sizeof(pack_header_t);
	it->size	= offset_metric;
	StringCopyS(it->path,sizeof(it->path),".PACK.filenames");
	it->func_it(it,false);

	it->off		= offset_metric;
	it->size	= offset_metric + n_files * sizeof(pack_metric_t);
	StringCopyS(it->path,sizeof(it->path),".PACK.metric");
	it->func_it(it,false);
    }


    //--- sub files

    const pack_metric_t *metric = (pack_metric_t*)(szs->data+offset_metric);
    ccp name = pack->name_pool;

    uint idx;
    for ( idx = 0; idx < n_files; idx++, metric++ )
    {
	while ( *name == '/' ) // skip leading slashes (security)
	    name++;
	StringCopyS(it->path,sizeof(it->path),name);
	name += strlen(name) + 1;

	it->index	= idx+1;
	it->fst_item	= 0;
	it->is_dir	= 0;

	it->off	= ntohl(metric->offset);
	if ( it->off >= size )
	{
	    it->off = size;
	    it->size = 0;
	}
	else
	{
	    it->size = ntohl(metric->size);
	    if ( it->size > size - it->off )
		 it->size = size - it->off;
	}

	it->func_it(it,false);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreatePACK
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			total_fsize,	// total file size
    SetupParam_t	* setup_param	// NULL or setup parameters
)
{
    DASSERT(szs);
    PRINT("CreatePACK() ndir=%d, totfsize=%x, align=%x,%x\n",
		szs->subfile.used, total_fsize, opt_align_breff, opt_align_pack );

    //--- setup

    enumError max_err = ERR_OK;
    const endian_func_t	* endian = &be_func;
    szs->endian = endian;

    szs->fform_arch	= FF_PACK;
    szs->ff_attrib	= GetAttribFF(szs->fform_arch);

    SortSubFilesSZS(szs,SORT_AUTO);

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

    //--- calc segment sizes and alloc data

    u32 spool_size = 0, n_files = 0;

    int i;
    szs_subfile_t * fptr;
    for ( i = 0, fptr = szs->subfile.list; i < szs->subfile.used; i++, fptr++ )
    {
	if (!fptr->is_dir)
	{
	    n_files++;
	    spool_size += strlen(fptr->path) + 1;
	}
    }

    spool_size = ALIGN32(spool_size,PACK_METRIC_ALIGN);
    const u32 metric_offset = sizeof(pack_header_t) + spool_size;
    const u32 metric_size
	= ALIGN32( n_files * sizeof(pack_metric_t),
		opt_align_pack > PACK_METRIC_ALIGN ? opt_align_pack : PACK_METRIC_ALIGN );

    total_fsize = ALIGN32(total_fsize,opt_align_pack);
    u32 file_offset = ALIGN32(metric_offset+metric_size,opt_align_pack);

    szs->size = file_offset + total_fsize;
    szs->data = CALLOC(1,szs->size);

    noPRINT(">>> SIZE=%zu=%#zx, N=%u,%u\n",
		szs->size, szs->size, n_files, szs->subfile.used );


    //--- setup header

    pack_header_t *ph	= (pack_header_t*)szs->data;
    ph->magic		= htonl(PACK_MAGIC_NUM);
    ph->file_size	= htonl(szs->size);
    ph->n_files		= htonl(n_files);
    ph->offset_metric	= htonl(metric_offset);


    //--- copy files

    pack_metric_t *metric = (pack_metric_t*)(szs->data+metric_offset);
    char *name = ph->name_pool;
    char *name_end = (char*)metric;

    for ( i = 0, fptr = szs->subfile.list; i < szs->subfile.used; i++, fptr++ )
    {
	MARK_USED(name,name_end);
	if (!fptr->is_dir)
	{
	    name = StringCopyE(name,name_end,fptr->path);
	    name++;

	    metric->offset = htonl(file_offset);
	    metric->size   = htonl(fptr->size);
	    metric++;

	    if (fptr->data)
		memcpy(szs->data + file_offset, fptr->data, fptr->size );
	    else
	    {
		PRINT("LOAD: %x+%x %s\n",file_offset,fptr->size,fptr->path);
		enumError err = LoadFILE( source_dir, fptr->path, 0,
				szs->data + file_offset, fptr->size, 0, &szs->fatt, true );
		if ( max_err < err )
		     max_err = err;
	    }
	    file_offset += ALIGN32(fptr->size,opt_align_pack);
	}
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


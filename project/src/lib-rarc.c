
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

#include "lib-rarc.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IterateFilesRARC()		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct rarc_info_t
{
    const u8		*data;		// begin of file data
    uint		data_off;	// offset of 'data'

    const rarc_header_t	*rh;		// RARC header
    const rarc_node_t	*node;		// first RARC node
    const rarc_entry_t	*entry;		// first RARC entry
    ccp			str_pool;	// pointer to string pool
    ccp			str_pool_end;	// pointer to string pool end
    uint		n_node;		// total number of nodes
    uint		n_entry;	// total number of entires

} rarc_info_t;

///////////////////////////////////////////////////////////////////////////////

static int iterate_rarc_dir
(
    szs_iterator_t	*it,		// iterator struct with all infos
    rarc_info_t		*ri,		// valid rarc info
    uint		node_idx	// node to print
)
{
    DASSERT(it);
    DASSERT(it->szs);
    DASSERT(ri);
    DASSERT(ri->rh);
    DASSERT(ri->node);
    DASSERT(ri->entry);

    if ( node_idx >= ri->n_node )
	return 0;

    const rarc_node_t *node = ri->node + node_idx;
    uint idx = be32(&node->entry_index);
    if ( idx >= ri->n_entry )
	return 0;
    uint n = be16(&node->n_entry);
    if ( n > ri->n_entry - idx )
	 n = ri->n_entry - idx;

    char *path = it->trail_path;
    char *path_end = it->path + sizeof(it->path) - 4;
    const rarc_entry_t *entry = ri->entry + idx;

    int stat;
    for ( stat = 0; !stat && n > 0; idx++, n--, entry++ )
    {
	it->index = idx;

	ccp fname = ri->str_pool + be16(&entry->name_off);
	char *dest = StringCopyE( path, path_end, fname );
	if ( be16(&entry->id) != 0xffff )
	{
	    // is file

	    it->is_dir	= 0;
	    it->off	= ri->data_off + be32(&entry->data_off);
	    it->size	= be32(&entry->data_size);
	    stat = it->func_it(it,false);
	}
	else if ( !( *fname == '.' && ( !fname[1] || fname[1] == '.' && !fname[2] )))
	{
	    // is directory

	    *dest++ = '/';
	    *dest = 0;
	    it->is_dir	= 1;
	    it->off	= 0;
	    it->size	= 0;
	    stat = it->func_it(it,false);

	    char *save_trail_path = it->trail_path;
	    it->trail_path = dest;
	    iterate_rarc_dir(it,ri,be32(&entry->data_off));
	    it->trail_path = save_trail_path;
	}
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

int IterateFilesRARC
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

    if ( !szs->data || szs->size <= sizeof(rarc_file_header_t) )
	return -1;

    //--- analyze file header

    const rarc_file_header_t *fh = (rarc_file_header_t*)szs->data;
    const uint file_size = be32(&fh->file_size);
    const uint hd_off    = be32(&fh->header_off);
    const uint hd_size   = be32(&fh->header_size);
    TRACE("RARC/FH: fsize=%x, header=%x+%x\n",file_size,hd_off,hd_size);

    const u8 *data_beg = szs->data + hd_off + hd_size;
    const u8 *data_end = szs->data + szs->size;

    if ( be32(&fh->magic) != RARC_MAGIC_NUM
		|| file_size >  szs->size
		|| hd_off    >= szs->size
		|| hd_size   >= szs->size
		|| hd_off + hd_size >= szs->size )
	return -1;


    //--- analyze rarc header

    rarc_info_t ri;
    memset(&ri,0,sizeof(ri));
    ri.data	= data_beg;
    ri.data_off	= ri.data - szs->data;

    ri.rh		= (rarc_header_t*)(szs->data+hd_off);
    ri.node		= (rarc_node_t*)( (u8*)ri.rh + be32(&ri.rh->root_off) );
    ri.entry		= (rarc_entry_t*)( (u8*)ri.rh + be32(&ri.rh->entry_off) );
    ri.str_pool		= (ccp)ri.rh + be32(&ri.rh->str_pool_off);
    ri.str_pool_end	= ri.str_pool + be32(&ri.rh->str_pool_size);
    ri.n_node		= be32(&ri.rh->n_node);
    ri.n_entry		= be32(&ri.rh->n_entry);

    if ( (u8*)ri.node < (u8*)ri.rh+sizeof(*ri.rh)
		|| (u8*)ri.node + sizeof(*ri.node) > data_end
		|| (u8*)ri.entry <= (u8*)ri.node
		|| (u8*)ri.entry + sizeof(*ri.node) * ri.n_entry > data_end )
	return -1;

    #if HAVE_PRINT0
    {
	PRINT(">> %6zx header\n", (u8*)ri.rh - szs->data );
	PRINT(">> %6zx %u nodes\n", (u8*)ri.node - szs->data, ri.n_node );
	PRINT(">> %6zx %u entries\n", (u8*)ri.entry - szs->data, ri.n_entry );
	PRINT(">> %6zx string pool\n", (u8*)ri.str_pool - szs->data );
	PRINT(">> %6zx END string pool\n", (u8*)ri.str_pool_end - szs->data );
	PRINT(">> %6zx data\n", (u8*)data_beg - szs->data );
	PRINT(">> %6zx END data\n", (u8*)data_end - szs->data );
	PRINT(">> %6zx END file\n", szs->size );

	int i;

	putchar('\n');
	const rarc_node_t *node = ri.node;
	for ( i = 0; i < ri.n_node; i++, node++ )
	{
	    printf("N %3d: %.4s %04x : %4u %4u : %s\n",
		i,
		node->name, be16(&node->unknown_08),
		be16(&node->n_entry), be32(&node->entry_index),
		ri.str_pool + be16(&node->name_off) );
	}

	putchar('\n');
	const rarc_entry_t *entry = ri.entry;
	for ( i = 0; i < ri.n_entry; i++, entry++ )
	{
	    printf("E %3d: %04x %04x %04x : %8x %6x : %08x : %s\n",
		i,
		be16(&entry->id),
		be16(&entry->unknown_02), be16(&entry->unknown_04),
		be32(&entry->data_off), be32(&entry->data_size),
		be32(&entry->unknown_10),
		ri.str_pool + be16(&entry->name_off) );
	}
    }
    #endif

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
	it->size	= sizeof(rarc_file_header_t);
	StringCopyS(it->path,sizeof(it->path),".RARC.file-header");
	it->func_it(it,false);

	it->off		= (u8*)ri.rh - szs->data;
	it->size	= sizeof(rarc_header_t);
	StringCopyS(it->path,sizeof(it->path),".RARC.header");
	it->func_it(it,false);

	it->off		= (u8*)ri.node - szs->data;
	it->size	= sizeof(rarc_node_t) * ri.n_node;
	snprintf(it->path,sizeof(it->path),".RARC.%u_node%s",
		ri.n_node, ri.n_node == 1 ? "" : "s" );
	it->func_it(it,false);

	it->off		= (u8*)ri.entry - szs->data;
	it->size	= sizeof(rarc_entry_t) * ri.n_entry;
	snprintf(it->path,sizeof(it->path),".RARC.%u_entr%s",
		ri.n_entry, ri.n_entry == 1 ? "y" : "ies" );
	it->func_it(it,false);

	it->off		= (u8*)ri.str_pool - szs->data;
	it->size	= ri.str_pool_end - ri.str_pool;
	StringCopyS(it->path,sizeof(it->path),".RARC.string-pool");
	it->func_it(it,false);
    }

    it->trail_path = it->path;
    iterate_rarc_dir(it,&ri,0);
    it->trail_path = 0;

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


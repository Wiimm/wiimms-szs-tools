
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

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>

#include "lib-szs.h"
#include "lib-kcl.h"
#include "lib-rarc.h"
#include "lib-pack.h"
#include "lib-rkc.h"
#include "lib-brres.h"
#include "lib-breff.h"
#include "lib-image.h"
#include "lib-bzip2.h"
#include "lib-lzma.h"
#include "lib-checksum.h"
#include "dclib-utf8.h"
#include "crypt.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			have_szs_file			///////////////
///////////////////////////////////////////////////////////////////////////////
// order is important,
// compare have_szs_name[], have_szs_file[], have_szs_fform[] and ct.wiimm.de

const ccp have_szs_name[HAVESZS__N] =
{
    "lex",		// HAVESZS_COURSE_LEX
    "objflow",		// HAVESZS_OBJFLOW
    "ght_item",		// HAVESZS_GHT_ITEM
    "ght_itemobj",	// HAVESZS_GHT_ITEM_OBJ
    "ght_kart",		// HAVESZS_GHT_KART
    "ght_kartobj",	// HAVESZS_GHT_KART_OBJ
    "itemslot",		// HAVESZS_ITEM_SLOT_TABLE
    "minigame",		// HAVESZS_MINIGAME
    "aiparam_baa",	// HAVESZS_AIPARAM_BAA
    "aiparam_bas",	// HAVESZS_AIPARAM_BAS
    "license",		// HAVESZS_LICENSE_TXT
    "ver_bin",		// HAVESZS_VERSION_BIN
};

const ccp have_szs_file[HAVESZS__N] =
{
    "course.lex",			// HAVESZS_COURSE_LEX
    "common/ObjFlow.bin",		// HAVESZS_OBJFLOW
    "common/GeoHitTableItem.bin",	// HAVESZS_GHT_ITEM
    "common/GeoHitTableItemObj.bin",	// HAVESZS_GHT_ITEM_OBJ
    "common/GeoHitTableKart.bin",	// HAVESZS_GHT_KART
    "common/GeoHitTableKartObj.bin",	// HAVESZS_GHT_KART_OBJ
    "ItemSlotTable/ItemSlotTable.slt",	// HAVESZS_ITEM_SLOT_TABLE
    "common/minigame.kmg",		// HAVESZS_MINIGAME
    "AIParam/AIParam.baa",		// HAVESZS_AIPARAM_BAA
    "AIParam/AIParam.bas",		// HAVESZS_AIPARAM_BAS
    "license.txt",			// HAVESZS_LICENSE_TXT
    "version.bin",			// HAVESZS_VERSION_BIN
};

const file_format_t have_szs_fform[HAVESZS__N] =
{
    FF_LEX,		// HAVESZS_COURSE_LEX
    FF_OBJFLOW,		// HAVESZS_OBJFLOW
    FF_GH_ITEM,		// HAVESZS_GHT_ITEM
    FF_GH_IOBJ,		// HAVESZS_GHT_ITEM_OBJ
    FF_GH_KART,		// HAVESZS_GHT_KART
    FF_GH_KOBJ,		// HAVESZS_GHT_KART_OBJ
    FF_ITEMSLT,		// HAVESZS_ITEM_SLOT_TABLE
    FF_KMG,		// HAVESZS_MINIGAME
    FF_UNKNOWN,		// HAVESZS_AIPARAM_BAA
    FF_UNKNOWN,		// HAVESZS_AIPARAM_BAS
    FF_UNKNOWN,		// HAVESZS_LICENSE_TXT
    FF_UNKNOWN,		// HAVESZS_VERSION_BIN
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			have_attrib			///////////////
///////////////////////////////////////////////////////////////////////////////

const ccp have_attrib_name[HAVEATT__N] =
{
    "cheat",		// HAVEATT_CHEAT
    "edit",		// HAVEATT_EDIT
    "reverse",		// HAVEATT_REVERSE
};

///////////////////////////////////////////////////////////////////////////////

ccp CreateSpecialInfoAttrib
	( have_attrib_t attrib, bool add_value, ccp return_if_empty )
{
    static char buf[500];
    char *dest = buf;

    if (add_value)
	dest = snprintfE( dest, buf+sizeof(buf), "%u=" , attrib );

    uint i, mask;
    ccp sep = "";
    for ( i = 0, mask = 1; i < HAVEATT__N; i++, mask <<= 1 )
	if ( attrib & mask )
	{
	    dest = StringCat2E(dest,buf+sizeof(buf),sep,have_attrib_name[i]);
	    sep = ",";
	}

    return dest == buf ? return_if_empty : CopyCircBuf0(buf,dest-buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_file_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeSZS ( szs_file_t * szs )
{
    DASSERT(szs);
    memset(szs,0,sizeof(*szs));
    szs->fname		= EmptyString;
    szs->endian		= &be_func;
    szs->n_cannon	= -1;
    szs->ff_version	= -1;

    InitializeStringPool(&szs->string_pool);
    InitializeSubfileList(&szs->subfile);
}

///////////////////////////////////////////////////////////////////////////////

void InitializeSubSZS
(
    szs_file_t		*szs,		// SZS to initialize
    szs_file_t		*base,		// valid base SZS
    uint		off,		// offset of selected data within 'base'
    uint		size,		// not NULL: size of selected data
    file_format_t	fform,		// not FF_UNKNOWN: use this type
    ccp			fname,		// NULL or filename
    bool		decompress	// true: decompress if possible
)
{
    DASSERT(szs);
    DASSERT(base);
    DASSERT( off <= base->size );
    DASSERT( off + size <= base->size );

    InitializeSZS(szs);
    szs->parent = base;

    if ( off <= base->size )
    {
	szs->data	= base->data + off;
	szs->size	= base->size - off;
	szs->file_size	= base->file_size ? base->file_size - off : 0;
	szs->parent_off	= off;

	if ( size && szs->size > size )
	    szs->size = size;
    }

    if ( fform == FF_UNKNOWN )
    {
	fform = RepairMagicByOpt(0,szs->data,szs->file_size,szs->file_size,
						FF_UNKNOWN, fname );
	PRINT("InitializeSubSZS() new-ff=%s by %s\n",
		GetNameFF(0,fform), fname );
    }

    szs->fform_file	= szs->fform_arch = szs->fform_current = fform;
    szs->ff_attrib	= GetAttribFF(szs->fform_arch);
// [[version-suffix]]
    szs->ff_version	= GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    PRINT("InitializeSubSZS() ff=%s, vers=%d\n",
		GetNameFF(szs->fform_file,szs->fform_arch), szs->ff_version );

    if (decompress)
	TryDecompressSZS(szs,true);

    UseContainerData(&szs->container,false,SetupContainerSZS(base));

    szs->brres_type	= base->brres_type;
    szs->dont_patch_mdl	= base->dont_patch_mdl;
}

///////////////////////////////////////////////////////////////////////////////

void AssignSZS
(
    szs_file_t		*szs,		// SZS to initialize
    bool		init_szs,	// true: InitializeSZS(), false: ResetSZS()
    void		*data,		// data to assign
    uint		size,		// size of 'data'
    bool		move_data,	// true: free 'data' on reset
    file_format_t	fform,		// not FF_UNKNOWN: use this type
    ccp			fname		// NULL or file name
)
{
    DASSERT(szs);

    if (!init_szs)
	ResetSZS(szs);
    InitializeSZS(szs);
    szs->file_size = data ? size : 0;
    if (fname)
	szs->fname = STRDUP(fname);

// [[analyse-magic+]]
// [[ff_fallback]]
    if ( fform == FF_UNKNOWN )
	fform = RepairMagicByOpt(0,data,size,size,FF_UNKNOWN,szs->fname);

    if (IsCompressedFF(fform))
    {
	szs->cdata		= data;
	szs->csize		= szs->file_size;
	szs->cdata_alloced	= data && move_data;
	szs->fform_file		= fform;
	DecompressSZS(szs,true,0);
    }
    else
    {
	szs->data		= data;
	szs->size		= szs->file_size;
	szs->data_alloced	= data && move_data;
	szs->fform_file		= fform;
	szs->fform_arch		= fform;
	szs->fform_current	= fform;
	szs->ff_attrib		= GetAttribFF(fform);
// [[version-suffix]]
	szs->ff_version		= GetVersionFF(szs->fform_arch,szs->data,szs->size,0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void CopySZS
(
    szs_file_t		*szs,		// SZS to initialize
    bool		init_szs,	// true: InitializeSZS(), false: ResetSZS()
    const szs_file_t	*source		// source SZS
)
{
    DASSERT(szs);


    if ( source && source->data )
	AssignSZS( szs, init_szs,
		MEMDUP(source->data,source->size), source->size, true,
		source->fform_current, source->fname );
    else
    {
	if (!init_szs)
	    ResetSZS(szs);
	InitializeSZS(szs);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetSZS ( szs_file_t * szs )
{
    if (szs)
    {
	if (szs->cdata_alloced)
	    FREE(szs->cdata);
	if (szs->data_alloced)
	    FREE(szs->data);
	if (szs->obj_name_alloced)
	    FreeString(szs->obj_name);
	FreeContainer(szs->old_container);
	ResetContainer(&szs->container);
	ResetFileSZS(szs,true);
	FreeString(szs->fname);
	FreeString(szs->dest_fname);
	FreeString(szs->cache_fname);
	FREE(szs->used_file);
	if (szs->required_file)
	{
	    ResetParamField(szs->required_file);
	    FREE(szs->required_file);
	}
	if (szs->special_file)
	{
	    ResetParamField(szs->special_file);
	    FREE(szs->special_file);
	}
	ResetSubfileList(&szs->ext_data);

	InitializeSZS(szs);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetFileSZS ( szs_file_t * szs, bool remove_list )
{
    if (szs)
    {
	while (szs->brsub_list)
	{
	    brsub_list_t * ptr = szs->brsub_list;
	    szs->brsub_list = ptr->next;
	    FREE(ptr);
	}

	ResetStringPool(&szs->string_pool);

	if (remove_list)
	    ResetSubfileList(&szs->subfile);
	else
	    PurgeSubfileList(&szs->subfile);
    }
}

///////////////////////////////////////////////////////////////////////////////

szs_subfile_t * FindLinkSZS
	( szs_file_t *szs, dev_t device, ino_t inode, const szs_subfile_t *exclude )
{
    DASSERT(szs);
    if ( !inode && !device )
	return 0;

    szs_subfile_t *ptr = szs->subfile.list;
    szs_subfile_t *end = ptr + szs->subfile.used;
    for ( ; ptr < end; ptr++ )
	if ( ptr != exclude && ptr->inode == inode && ptr->device == device )
	    return ptr;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearContainerSZS ( szs_file_t * szs )
{
    DASSERT(szs);
    FreeContainer(szs->old_container);
    ResetContainer(&szs->container);
}

///////////////////////////////////////////////////////////////////////////////

Container_t * SetupContainerSZS ( szs_file_t * szs )
{
    DASSERT(szs);

    u8 *data;
    uint size;
    CopyMode_t mode;

    if (szs->data)
    {
	data = szs->data;
	size = szs->size;
	mode = szs->data_alloced ? CPM_MOVE : CPM_LINK;
	szs->data_alloced = false;
    }
    else
    {
	data = szs->cdata;
	size = szs->csize;
	mode = szs->cdata_alloced ? CPM_MOVE : CPM_LINK;
	szs->cdata_alloced = false;
    }

 #ifdef TEST
    if (AssignContainer(&szs->container,szs->container.protect_level,data,size,mode))
	DumpInfoContainer(stdlog,collog,2,"SZS: ",&szs->container,0);
 #else
    AssignContainer(&szs->container,szs->container.protect_level,data,size,mode);
 #endif
    return &szs->container;
}

///////////////////////////////////////////////////////////////////////////////

ContainerData_t * LinkContainerSZS ( szs_file_t * szs )
{
    DASSERT(szs);
    if (!IsValidContainer(&szs->container))
	SetupContainerSZS(szs);
    return LinkContainerData(&szs->container);
}

///////////////////////////////////////////////////////////////////////////////

ContainerData_t * MoveContainerSZS ( szs_file_t * szs )
{
    DASSERT(szs);
    if (!IsValidContainer(&szs->container))
	SetupContainerSZS(szs);
    return MoveContainerData(&szs->container);
}

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * ContainerSZS ( szs_file_t * szs )
{
    // old [[container]] will be [[obsolete+]] 2016-06 -> or in 2018

    DASSERT(szs);
    if (!szs->old_container)
    {
	if (!IsValidContainer(&szs->container))
	    SetupContainerSZS(szs);
	szs->old_container = UseSubContainer(0,szs->container.cdata);
    }

    return AllocContainer(szs->old_container);
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadSZS
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
    PRINT("LoadSZS(%d,%d,%d) fname=%s\n",decompress,ignore_no_file,mark_readonly,fname);

    ResetSZS(szs);
    if (mark_readonly)
	MarkReadonlySZS(szs);

    File_t F;
    enumError err = OpenFILE(&F,true,fname,ignore_no_file,false);
    if (err)
	return err;

    if (F.is_stdio)
    {
	uint size = 0, used = 0;
	u8 *data = 0;

	while (!feof(F.f))
	{
	    uint space = size - used;
	    if ( space < 0x100 )
	    {
		PRINT("READ STDIN, REALLOC, used: %u/%u\n",used,size);
		size = 12*size/10 + 1024*1024;
		data = REALLOC(data,size);
		space = size - used;
	    }
	    const size_t rd_size = fread(data+used,1,space,F.f);
	    if ( rd_size > 0 )
		used += rd_size;
	    else
		if (ferror(F.f))
		    break;
	}

	szs->cdata_alloced = true;
	szs->csize = used;
	szs->cdata = REALLOC(data,used);
    }
    else
    {
	szs->cdata_alloced = true;
	szs->csize = F.st.st_size;
	szs->cdata = MALLOC(szs->csize);

	const size_t rd_size = fread(szs->cdata,1,szs->csize,F.f);
	if ( rd_size != szs->csize )
	{
	    ERROR1(ERR_READ_FAILED,"Can't read file: %s\n",fname);
	    ResetFile(&F,false);
	    return ERR_READ_FAILED;
	}
	szs->fname = F.fname;
	F.fname = 0;
	SetFileAttrib(&szs->fatt,&F.fatt,0);
    }
    ResetFile(&F,false);

// [[analyse-magic+]]
    szs->fform_file
	= RepairMagicByOpt(0,szs->cdata,szs->csize,szs->csize,FF_UNKNOWN,fname);

    if (!IsCompressedFF(szs->fform_file))
    {
	szs->fform_arch		= szs->fform_file;
	szs->fform_current	= szs->fform_file;
	szs->data		= szs->cdata;
	szs->size		= szs->csize;
	szs->file_size		= szs->csize;
	szs->data_alloced	= szs->cdata_alloced;

	szs->cdata		= 0;
	szs->csize		= 0;
	szs->cdata_alloced	= false;
    }
    szs->ff_attrib  = GetAttribFF(szs->fform_file);
// [[version-suffix]]
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    if (decompress)
    {
	err = DecompressSZS(szs,true,0);
	if (!err)
	    err = DecodeWU8(szs);
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveSZS
(
    szs_file_t		* szs,		// valid szs
    ccp			fname,		// valid pointer to filenname
    bool		overwrite,	// true: overwrite data
    bool		compress	// false: store decompressed
					// true:  store compressed
)
{
    MEM_CHECK;
    DASSERT(szs);
    DASSERT(fname);

    u8 * data;
    size_t size;

    if (compress)
    {
	CompressSZS(szs,0,true);
	data = szs->cdata;
	size = szs->csize;
    }
    else
    {
	DecompressSZS(szs,true,0);
	data = szs->data;
	size = szs->size;
    }

    if (!data)
	return ERROR0(ERR_INVALID_DATA,"No SZS data");

    File_t F;
    enumError err = CreateFILE(&F,true,fname,testmode,false,overwrite,opt_remove_dest,false);
    if (F.f)
    {
	SetFileAttrib(&F.fatt,&szs->fatt,0);
	size_t wstat = fwrite(data,1,size,F.f);
	if ( wstat != size )
	    err = FILEERROR1(&F,ERR_WRITE_FAILED,
		"Saving file (%zu bytes) failed: %s\n",size,fname);
    }
    enumError err2 = ResetFile(&F,opt_preserve);

    if ( compress && !err )
	LinkCacheSZS(szs,fname);

    return err ? err : err2;
}

///////////////////////////////////////////////////////////////////////////////

void LinkCacheData
(
    // link/copy only if opt_cache && data && size

    ccp			cache_fname,	// NULL or path to cache file
    ccp			src_fname,	// source filename
    cvp			data,		// data of link() failed
    uint		size		// size of data
)
{
    if ( opt_cache && cache_fname && src_fname && data && size )
    {
	PRINT("LinkCacheData(%d,%u) %s -> %s\n",data!=0,size,src_fname,cache_fname);
	unlink(cache_fname);
	if (link(src_fname,cache_fname))
	{
	    File_t F;
	    CreateFile(&F,true,cache_fname,FM_SILENT|FM_REMOVE);
	    if (F.f)
		fwrite(data,1,size,F.f);
	    ResetFile(&F,false);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void MarkReadonlySZS
(
    szs_file_t		* szs		// valid szs
)
{
    noPRINT("MarkReadonlySZS()\n");
    DASSERT(szs);

    szs->readonly = true;
}

///////////////////////////////////////////////////////////////////////////////

size_t GetDecompressedSizeYAZ
(
    const void		* data,		// source data
    size_t		data_size	// size of 'data'
)
{
    DASSERT(data);

    const u8 * src	= data;
    const u8 * src_end	= src + data_size;
    u8  code		= 0;
    int code_len	= 0;
    size_t total_size	= 0;

    while ( src < src_end )
    {
	if (!code_len--)
	{
	    code = *src++;
	    code_len = 7;
	}

	if ( code & 0x80 )
	{
	    // 1 byte direct
	    total_size++;
	}
	else
	{
	    // rle part

	    int n = *src++ >> 4;
	    if (!n)
		n = *src++ + 0x12;
	    else
		n += 2;
	    DASSERT( n >= 3 && n <= 0x111 );

	    if ( src > src_end )
		break;
	    total_size += n;
	}
	code <<= 1;
    }
    ASSERT( src <= src_end+2 );
    return total_size;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecompressYAZ
(
    // returns:
    //	    ERR_OK:           compression done
    //	    ERR_WARNING:      silent==true: dest buffer too small
    //	    ERR_INVALID_DATA: invalid source data

    const void		* data,		// source data
    size_t		data_size,	// size of 'data'
    void		* dest_buf,	// destination buffer (decompressed data)
    size_t		dest_buf_size,	// size of 'dest_buf'
    size_t		*write_status,	// number of written bytes
    ccp			fname,		// file name for error messages
    int			yaz_version,	// yaz version for error messages (0|1)
    bool		silent,		// true: don't print error messages
    FILE		*hexdump	// not NULL: write decrompression hex-dump
)
{
    DASSERT(data);
    DASSERT(dest_buf);
    DASSERT(dest_buf_size);
    TRACE("DecompressYAZ(vers=%d) src=%p+%zu, dest=%p+%zu\n",
		yaz_version, data, data_size, dest_buf, dest_buf_size );

    const u8 * src	= data;
    const u8 * src_end	= src + data_size;
    u8 * dest		= dest_buf;
    u8 * dest_end	= dest + dest_buf_size;
    u8  code		= 0;
    int code_len	= 0;

    uint count0		= 0;
    uint count1		= 0;
    uint count2		= 0;
    uint count3		= 0;

    int addr_fw = 0;
    char buf[12];
    if (hexdump)
    {
	fprintf(hexdump,"\n"
		"# size of compressed data:    %#8zx = %9zu\n"
		"# size of de-compressed data: %#8zx = %9zu\n"
		"\n",
		data_size, data_size, dest_buf_size, dest_buf_size );
	addr_fw = snprintf(buf,sizeof(buf),"%zx",dest_buf_size-1)+1;
    }

    while ( src < src_end && dest < dest_end )
    {
	if (!code_len--)
	{
	    if (hexdump)
	    {
		count0++;
		fprintf(hexdump,
			"%*x: %02x -- -- : type byte\n",
			addr_fw, (uint)(src-(u8*)data), *src );
	    }

	    code = *src++;
	    code_len = 7;
	}

	if ( code & 0x80 )
	{
	    if (hexdump)
	    {
		count1++;
		fprintf(hexdump,
			"%*x: %02x -- -- : copy direct\n",
			addr_fw, (uint)(src-(u8*)data), *src );
	    }

	    // copy 1 byte direct
	    *dest++ = *src++;
	}
	else
	{
	    // rle part

	    const u8 b1 = *src++;
	    const u8 b2 = *src++;
	    const u8 * copy_src = dest - ( ( b1 & 0x0f ) << 8 | b2 ) - 1;

	    int n = b1 >> 4;
	    if (!n)
		n = *src++ + 0x12;
	    else
		n += 2;
	    DASSERT( n >= 3 && n <= 0x111 );

	    noPRINT_IF ( copy_src + n > dest,
			"RLE OVERLAP: copy %zu..%zu -> %zu\n",
				copy_src - szs->data,
				copy_src + n - szs->data,
				dest  - szs->data );

	    if ( copy_src < (u8*)dest_buf )
	    {
		if (write_status)
		    *write_status = dest - (u8*)dest_buf;
		if (!silent)
		    ERROR0(ERR_INVALID_DATA,
			"YAZ%u data corrupted:"
			" Back reference points before beginning of data: %s\n",
			yaz_version, fname ? fname : "?" );
		return ERR_INVALID_DATA;
	    }

	    if ( dest + n > dest_end )
	    {
		// first copy as much as possible
		while ( dest < dest_end )
		    *dest++ = *copy_src++;

		if (write_status)
		    *write_status = dest - (u8*)dest_buf;
		return silent
		    ? ERR_WARNING
		    : ERROR0(ERR_INVALID_DATA,
			"YAZ%u data corrupted:"
			" Decompressed data larger than specified (%zu>%zu): %s\n",
			yaz_version,
			GetDecompressedSizeYAZ(data,data_size),
			dest_buf_size,
			fname ? fname : "?" );
	    }

	    if (hexdump)
	    {
		if ( n < 0x12 )
		{
		    count2++;
		    fprintf(hexdump,"%*x: %02x %02x --",
			addr_fw, (uint)(src-2-(u8*)data), src[-2], src[-1] );
		}
		else
		{
		    count3++;
		    fprintf(hexdump,"%*x: %02x %02x %02x",
			addr_fw, (uint)(src-3-(u8*)data),
			src[-3], src[-2], src[-1] );
		}
		fprintf(hexdump," : copy %03x off %04d:",
			n, (uint)(copy_src-dest) );
		int max = n < 10 ? n : 10;
		const u8 *hex_src = copy_src;

		// copy data before hexdump
		while ( n-- > 0 )
		    *dest++ = *copy_src++;

		while ( max-- > 0 )
		    fprintf(hexdump," %02x",*hex_src++);
		if ( hex_src != copy_src )
		    fputs(" ...\n",hexdump);
		else
		    fputc('\n',hexdump);
	    }
	    else
	    {
		// don't use memcpy() or memmove() here because
		// they don't work with self referencing chunks.
		while ( n-- > 0 )
		    *dest++ = *copy_src++;
	    }
	}

	code <<= 1;
    }
    ASSERT( src <= src_end );
    ASSERT( dest <= dest_end );

    if (hexdump)
	fprintf(hexdump,"\n"
		"# %u type bytes, %u single bytes, %u+%u back references\n"
		"\n",
		count0, count1, count2, count3 );

    if (write_status)
	*write_status = dest - (u8*)dest_buf;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

void ClearCompressedSZS ( szs_file_t * szs )
{
    if (szs->cdata_alloced)
    {
	szs->cdata_alloced = false;
	FREE(szs->cdata);
    }
    szs->cdata = 0;
    szs->csize = 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecompressSZS
(
    szs_file_t	*szs,		// valid SZS source, use cdata
    bool	rm_compressed,	// true: remove compressed data
    FILE	*hexdump	// not NULL: write decrompression hex-dump
)
{
    TRACE("DecompressSZS(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    switch(szs->fform_file)
    {
	case FF_BZ:	return DecompressBZ(szs,rm_compressed);
	case FF_YBZ:	return DecompressYBZ(szs,rm_compressed);
	case FF_BZIP2:	return DecompressBZIP2(szs,rm_compressed);
	case FF_LZ:	return DecompressLZ(szs,rm_compressed);
	case FF_YLZ:	return DecompressYLZ(szs,rm_compressed);
	case FF_LZMA:	return DecompressLZMA(szs,rm_compressed);
	default: break;
    }

    u8 *cdata = szs->cdata;
    const yaz0_header_t *yaz0 = (yaz0_header_t*)cdata;

    if ( !memcmp(yaz0->magic,XYZ0_MAGIC,sizeof(yaz0->magic)) )
    {
	cdata = DecodeXYZ(0,cdata,szs->csize);
	yaz0 = (yaz0_header_t*)cdata;
    }

    if (   memcmp(yaz0->magic,YAZ0_MAGIC,sizeof(yaz0->magic))
	&& memcmp(yaz0->magic,YAZ1_MAGIC,sizeof(yaz0->magic)) )
    {
	return ERROR0(ERR_INVALID_DATA,"Invalid Yaz* magic!\n");
    }

    szs->data_alloced = true;
    szs->file_size = szs->size = ntohl(yaz0->uncompressed_size);
    TRACE("U-SIZE: %zu\n",szs->size);
    szs->data = CALLOC(1,szs->size);

    enumError err
	= DecompressYAZ( cdata + sizeof(yaz0_header_t),
			  szs->csize - sizeof(yaz0_header_t),
			  szs->data,
			  szs->size,
			  &szs->size,
			  szs->fname,
			  yaz0->magic[3] - '0',
			  false,
			  hexdump );
    if (err)
	return err;

// [[analyse-magic]]
    szs->fform_arch = szs->fform_current = GetByMagicFF(szs->data,szs->size,szs->size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
// [[version-suffix]]
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    if ( cdata != szs->cdata )
	FREE(cdata);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool TryDecompressSZS
(
    szs_file_t	*szs,		// valid SZS source, use data if not NULL
    bool	rm_compressed	// true: remove compressed data
)
{
    DASSERT(szs);
    if (!IsCompressedFF(szs->fform_arch))
	return false;

    if (szs->data)
    {
	if ( szs->cdata && szs->cdata_alloced )
	    FREE(szs->cdata);

	szs->cdata		= szs->data;
	szs->data		= 0;
	szs->csize		= szs->size;
	szs->size		= 0;
	szs->cdata_alloced	= szs->data_alloced;
	szs->data_alloced	= false;
    }

    return DecompressSZS(szs,rm_compressed,0) == ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearUncompressedSZS ( szs_file_t * szs )
{
    DASSERT(szs);

    ClearSpecialFilesSZS(szs);
    if (szs->data_alloced)
    {
	szs->data_alloced = false;
	FREE(szs->data);
    }
    szs->data = 0;
    szs->file_size = szs->size = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearSpecialFilesSZS
(
    szs_file_t		* szs		// valid szs
)
{
    DASSERT(szs);

    szs->special_done		= false;
    szs->course_kcl_data	= 0;
    szs->course_kcl_size	= 0;
    szs->course_kmp_data	= 0;
    szs->course_kmp_size	= 0;
    szs->course_lex_data	= 0;
    szs->course_lex_size	= 0;
    szs->course_model_data	= 0;
    szs->course_model_size	= 0;
    szs->course_d_model_data	= 0;
    szs->course_d_model_size	= 0;
    szs->vrcorn_model_data	= 0;
    szs->vrcorn_model_size	= 0;
    szs->map_model_data		= 0;
    szs->map_model_size		= 0;
    szs->have_ice_brres		= 0;

    if (szs->special_file)
	ResetParamField(szs->special_file);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    YAZ helpers			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[yaz_res_t]]

typedef struct yaz_res_t
{
    u16  ref;
    u16  len;
}
yaz_res_t;

//-----------------------------------------------------------------------------
// [[yaz_compr_t]]

typedef struct yaz_compr_t
{
    uint	secure_size;

    uint	dest_buf_size;
    u8		* dest_buf;
    u8		* dest_ptr;
    u8		* dest_end;

    uint	src_len;
    const u8	* src;
    const u8	* src_end;

    szs_file_t	* szs;		// reference file for cache operations

}
yaz_compr_t;

///////////////////////////////////////////////////////////////////////////////

static void SetupDestYAZ ( yaz_compr_t *yaz, u32 secure_size )
{
    DASSERT(yaz);
    DASSERT(secure_size<sizeof(iobuf)/2);

    yaz->secure_size	= secure_size;
    yaz->dest_buf_size	= sizeof(iobuf) - secure_size;
    yaz->dest_buf	= (u8*)iobuf;
    yaz->dest_ptr	= yaz->dest_buf;
    yaz->dest_end	= yaz->dest_buf + yaz->dest_buf_size;
}

///////////////////////////////////////////////////////////////////////////////

static void GrowDestYAZ ( yaz_compr_t *yaz )
{
    DASSERT(yaz);

    const uint dest_pos = yaz->dest_ptr - yaz->dest_buf;

    yaz->dest_buf_size *= 2;
    const uint new_size = yaz->dest_buf_size + yaz->secure_size;

    TRACE("GrowDestYAZ(), new size; 0x%x = %u\n",new_size,new_size);

    if ( yaz->dest_buf == (u8*)iobuf )
    {
	yaz->dest_buf = MALLOC(new_size);
	memcpy(yaz->dest_buf,iobuf,dest_pos);
    }
    else
	yaz->dest_buf = REALLOC(yaz->dest_buf,new_size);

    yaz->dest_ptr  = yaz->dest_buf + dest_pos;
    yaz->dest_end  = yaz->dest_buf + yaz->dest_buf_size;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		YAZ compression, Back Tracking		///////////////
///////////////////////////////////////////////////////////////////////////////
// back tracking

const uint BACK_TRACK_MAX_DEPTH = 50;

///////////////////////////////////////////////////////////////////////////////

static uint BackTrackVal
(
    // returns a evaluation value

    yaz_res_t	*yaz_buf,	// yaz buffer
    uint	yaz_size,	// number of elements in 'yaz_buf'
    yaz_res_t	*yptr,		// pointer the next point to analyze
    int		max_depth,	// max depth of analysis
    uint	first_step,	// true: force byte copy
    uint	abort_val	// abort if best_val > abort_val
)
{
    //--- setup

    DASSERT( max_depth <= BACK_TRACK_MAX_DEPTH);

    yaz_res_t *yaz_end = yaz_buf + yaz_size;
    if (!abort_val)
	abort_val = M1(abort_val);

    struct stack_t
    {
	yaz_res_t	*yptr;	// pointer to yaz_res_t
	int		clen;	// code len
	int		depth;	// count down counter
    };
    struct stack_t stack[BACK_TRACK_MAX_DEPTH+1];


    //--- put the first job to the stack

    struct stack_t *sp = stack;
    if ( yptr->len >= 3 && !first_step )
    {
	sp->yptr  = yptr + yptr->len;
	sp->clen  = yptr->len < 0x12 ? 17 : 25;
    }
    else
    {
	sp->yptr  = yptr + 1;
	sp->clen  = 9;
    }
    sp->depth = max_depth - 1;
    sp++;


    //--- main loop

    uint best_val = 0;

    do
    {
	//--- get last element from stack

	sp--;
	yaz_res_t *yp	= sp->yptr;
	int clen	= sp->clen;
	int depth	= sp->depth;

	while ( yp < yaz_end && depth > 0 )
	{
	    depth--;
	    if ( yp->len >= 3 )
	    {
		// put new alternative to stack

		sp->yptr  = yp + yp->len;
		sp->clen  = clen + ( yp->len < 0x12 ? 17 : 25 );
		sp->depth = depth;
		sp++;
	    }

	    //--- process single byte variant directly

	    yp++;
	    clen += 9;
	}


	//-- evaluate

	const uint val = ((yp-yptr+1)<<11) - clen;
 #if 0
	if (verbose>0)
	    fprintf(stdlog,">> %6zx: %6zx cl=%3x, depth=%2u/%u -> %5x/%5x, stack=%zd\n",
			yptr-yaz_buf, yp-yptr, clen, depth, max_depth,
			val, best_val, sp-stack );
 #endif
	if ( best_val < val )
	{
	    best_val  = val;
	    if ( best_val > abort_val )
		return best_val;
	}
    }
    while ( sp > stack );

 #if 0
    if (verbose>0)
	fprintf(stdlog,">> return %x\n",best_val);
 #endif
    return best_val;
}

///////////////////////////////////////////////////////////////////////////////

static enumError BackTrackYAZ ( yaz_compr_t *yaz, uint max_depth )
{
    if ( max_depth > BACK_TRACK_MAX_DEPTH )
	max_depth = BACK_TRACK_MAX_DEPTH;

    if ( logging >= 1 )
    {
	fprintf(stdlog,">>YAZ back tracking: depth=%u, max-depth=%u\n",
		max_depth, BACK_TRACK_MAX_DEPTH );
	fflush(stdlog);
    }

    SetupDestYAZ(yaz,0x40);


    //--- setup yaz_buf

    const int range	= 0x1000;
    const int max_len	=  0x111;

    yaz_res_t *yaz_buf = CALLOC(sizeof(*yaz_buf),yaz->src_len);
    yaz_res_t *yref = yaz_buf;

    const u8 *src	= yaz->src;
    const u8 *src_end	= yaz->src_end;

    while ( src + 2 < src_end )
    {
	const u8 * search = src - range;
	if ( search < yaz->src )
	     search = yaz->src;

	const u8 * cmp_end = src + max_len;
	if ( cmp_end > src_end )
	     cmp_end = src_end;

	uint found_len = 1;
	const u8 * found = src;
	const u8 c1 = *src;

	for ( ; search < src; search++ )
	{
	    search = memchr(search,c1,src-search);
	    if (!search)
		break;
	    DASSERT( search < src );
	    const u8 * cmp1 = search + 1;
	    const u8 * cmp2 = src + 1;
	    while ( cmp2 < cmp_end && *cmp1 == *cmp2 )
		cmp1++, cmp2++;
	    const int len = cmp2 - src;
	    DASSERT( cmp1 - search == len );

	    if ( len > 2 && found_len < len )
	    {
		found_len = len;
		found = search;
		if ( found_len == max_len )
		    break;
	    }
	}
	yref->ref = src - found - 1;
	yref->len = found_len;
	yref++;
	src++;
    }

    while ( src < src_end )
    {
	yref->ref = 0;
	yref->len = 1;
	yref++;
	src++;
    }


    //--- create yaz0 stream

    u8 * dest		= yaz->dest_ptr;
    u8 * dest_end	= yaz->dest_end;
    src			= yaz->src;
    yref		= yaz_buf;
    u8 mask		= 0;
    u8 *code_byte	= dest;

    while ( src < src_end )
    {
	if (!yref->len)
	{
	    fprintf(stdlog,"src=%x, y=%zx,%d,%d\n",
		(uint)(src-yaz->src), sizeof(*yref)*(yref-yaz_buf),
		yref->ref, yref->len );
	    HexDump16(stdlog,0,0,(u8*)yref-0x20,0x40);
	    ASSERT(yref->len);
	}

	if ( dest > dest_end )
	{
	    const uint code_pos = code_byte - yaz->dest_buf;
	    yaz->dest_ptr  = dest;

	    GrowDestYAZ(yaz);

	    dest	= yaz->dest_ptr;
	    dest_end	= yaz->dest_end;
	    code_byte	= yaz->dest_buf + code_pos;
	}

	if (!mask)
	{
	    code_byte = dest;
	    *dest++ = 0;
	    mask = 0x80;
	}

	if ( yref->len < 3 )
	{
	    *code_byte |= mask;
	    *dest++ = *src++;
	    yref++;
	}
	else
	{
	    uint val0 = BackTrackVal(yaz_buf,yaz->src_len,yref,max_depth,true,0);
	    uint val1 = BackTrackVal(yaz_buf,yaz->src_len,yref,max_depth,false,val0);

	    if ( val0 >= val1 )
	    {
		*code_byte |= mask;
		*dest++ = *src++;
		yref++;
	    }
	    else
	    {
		DASSERT( yref->ref < 0x1000 );
		if ( yref->len < 0x12 )
		{
		    *dest++ = yref->ref >> 8 | ( yref->len - 2 ) << 4;
		    *dest++ = yref->ref;
		    noPRINT(" %5zx -> %02x %02x\n",
			    src-szs->data, dest[-2], dest[-1] );
		}
		else
		{
		    *dest++ = yref->ref >> 8;
		    *dest++ = yref->ref;
		    *dest++ = yref->len - 0x12;
		    noPRINT(" %5zx -> %02x %02x %02x\n",
			    src-szs->data, dest[-3], dest[-2], dest[-1] );
		}
		src  += yref->len;
		yref += yref->len;
	    }
	}
	mask >>= 1;
    }

    yaz->dest_ptr  = dest;
    FREE(yaz_buf);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ClassicCompressYAZ()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ClassicCompressYAZ ( yaz_compr_t *yaz, int compr )
{
    DASSERT(yaz);
    SetupDestYAZ(yaz,0x40);

    //--- calc range

    int fast = opt_fast;
    if ( compr < 1 )
	compr = opt_norm ? 9 : opt_compr;
    else
	fast = false;

    const u32 range = fast
			? 0x100
			: !compr
			? 0
			: opt_compr < 9
			? 0x10e0 * opt_compr / 9 - 0x0e0
			: 0x1000;


    //--- load from cache?

    if ( range == 0x1000 && IsSZSCacheEnabled() && yaz->szs )
    {
	check_cache_t cc;
	if (CheckSZSCache(&cc,yaz->szs,yaz->src,yaz->src_len,FF_YAZ0,".szs"))
	{
	    cc.cache.csize -= sizeof(yaz0_header_t);
	    memmove(cc.cache.cdata,cc.cache.cdata+sizeof(yaz0_header_t),cc.cache.csize);

	    yaz->dest_buf_size	= cc.cache.csize;
	    yaz->dest_buf	= cc.cache.cdata;
	    yaz->dest_ptr	=
	    yaz->dest_end	= cc.cache.cdata + cc.cache.csize;
	    yaz->szs->cache_used = true;

	    cc.cache.cdata = 0;
	    cc.cache.csize = 0;
	    ResetCheckCache(&cc);
	    return ERR_OK;
	}
	ResetCheckCache(&cc);
    }


    //--- setup, use local vars for optimization

    const bool optimize = opt_compr >= 10;

    u8 * dest		= yaz->dest_ptr;
    u8 * dest_end	= yaz->dest_end;
    const u8 *src	= yaz->src;
    const u8 *src_end	= yaz->src_end;

    u8 mask		= 0;
    u8 *code_byte	= dest;


    //--- main loop

    uint saved_len = 0;
    const u8 * saved = 0;

    while ( src < src_end )
    {
	if ( dest > dest_end )
	{
	    const uint code_pos = code_byte - yaz->dest_buf;
	    yaz->dest_ptr  = dest;

	    GrowDestYAZ(yaz);

	    dest	= yaz->dest_ptr;
	    dest_end	= yaz->dest_end;
	    code_byte	= yaz->dest_buf + code_pos;
	}

	if (!mask)
	{
	    code_byte = dest;
	    *dest++ = 0;
	    mask = 0x80;
	}

	const int max_len = 0x111;
	uint found_len = 1;
	const u8 * found = 0;

	if ( range && src + 2 < src_end )
	{
	    if (saved_len)
	    {
		found_len = saved_len;
		saved_len = 0;
		found = saved;
	    }
	    else
	    {
		const u8 * search = src - range;
		if ( search < yaz->src )
		     search = yaz->src;

		const u8 * cmp_end = src + max_len;
		if ( cmp_end > src_end )
		     cmp_end = src_end;

		const u8 c1 = *src;
		for ( ; search < src; search++ )
		{
		    search = memchr(search,c1,src-search);
		    if (!search)
			break;
		    DASSERT( search < src );
		    const u8 * cmp1 = search + 1;
		    const u8 * cmp2 = src + 1;
		    while ( cmp2 < cmp_end && *cmp1 == *cmp2 )
			cmp1++, cmp2++;
		    const int len = cmp2 - src;
		    DASSERT( cmp1 - search == len );
		    noPRINT_IF ( search + len > src,
					"FOUND l=%u,%u pos=%zu,%zu\n",
					len, found_len, search-szs->data, src-szs->data );

		    if ( found_len < len )
		    {
			found_len = len;
			found = search;
			if ( found_len == max_len )
			    break;
		    }
		}
	    }

	    if ( optimize && found_len >= 3 )
	    {
		uint found_len2 = 0;
		//const u8 * found2 = 0;
		const u8 * src2 = src + 1;

		const u8 * search = src2 - range;
		if ( search < yaz->src )
		     search = yaz->src;

		const u8 * cmp_end = src2 + max_len;
		if ( cmp_end > src_end )
		     cmp_end = src_end;

		const u8 c1 = *src2;
		for ( ; search < src2; search++ )
		{
		    search = memchr(search,c1,src2-search);
		    if (!search)
			break;
		    DASSERT( search < src2 );
		    const u8 * cmp1 = search + 1;
		    const u8 * cmp2 = src2 + 1;
		    while ( cmp2 < cmp_end && *cmp1 == *cmp2 )
			cmp1++, cmp2++;
		    const int len = cmp2 - src2;
		    DASSERT( cmp1 - search == len );
		    noPRINT_IF ( search + len > src2,
					"FOUND l=%u,%u pos=%zu,%zu\n",
					len, found_len2, search-szs->data, src2-szs->data );
		    if ( found_len2 < len )
		    {
			found_len2 = len;
			saved = search;
			if ( found_len2 == max_len )
			    break;
		    }
		}

		if ( found_len2 > found_len )
		{
		    found_len = 1;
		    saved_len = found_len2;
		}
	    }
	}
	else
	    saved_len = 0;

	if ( found_len >= 3 )
	{
	    DASSERT(found);
	    const u32 delta = src - found - 1;
	    DASSERT( delta < 0x1000 );
	    if ( found_len < 0x12 )
	    {
		*dest++ = delta >> 8 | ( found_len - 2 ) << 4;
		*dest++ = delta;
		noPRINT(" %5zx -> %02x %02x\n",
			src-szs->data, dest[-2], dest[-1] );
	    }
	    else
	    {
		*dest++ = delta >> 8;
		*dest++ = delta;
		*dest++ = found_len - 0x12;
		noPRINT(" %5zx -> %02x %02x %02x\n",
			src-szs->data, dest[-3], dest[-2], dest[-1] );
	    }
	    src += found_len;
	}
	else
	{
	    *code_byte |= mask;
	    *dest++ = *src++;
	}

	mask >>= 1;
    }

    yaz->dest_ptr  = dest;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CompressWith()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CompressWith
(
    // returns ERR_WARNING for invalid ff, ERR_NOTHING_TO_DO if already compressed,
    // or return value of compression function

    szs_file_t	*szs,		// NULL or valid data
    int		compr,		// compression level, default if COMPR_DEFAULT, auto if < 1
    bool	rm_uncompr,	// remove uncompressed data after compression
    file_format_t ff_compr,	// use this file format for compression
    file_format_t ff_fallback	// fall back if 'ff_compr' is not valid
)
{
    if (!szs)
	return ERR_OK; // no data

    if (!IsTrackCompressFF(ff_compr))
    {
	if (!IsTrackCompressFF(ff_fallback))
	    return ERR_WARNING;
	ff_compr = ff_fallback;
    }

    if (szs->cdata)
    {
	if ( szs->fform_file == ff_compr )
	    return ERR_NOTHING_TO_DO;
	DecompressSZS(szs,true,0);
    }

    if ( !szs->size || !szs->data )
	return ERR_NO_SOURCE_FOUND; // no data to compress

    switch(ff_compr)
    {
	case FF_BZ:	return CompressBZ   (szs,compr,rm_uncompr);
	case FF_YBZ:	return CompressYBZ  (szs,compr,rm_uncompr);
	case FF_BZIP2:	return CompressBZIP2(szs,compr,rm_uncompr);
	case FF_LZ:	return CompressLZ   (szs,compr,rm_uncompr);
	case FF_YLZ:	return CompressYLZ  (szs,compr,rm_uncompr);
	case FF_LZMA:	return CompressLZMA (szs,compr,rm_uncompr);
	default:	break;
    }

    return CompressYAZ(szs,compr,rm_uncompr);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SZS/YAZ compression		///////////////
///////////////////////////////////////////////////////////////////////////////

file_format_t GetNewCompressionSZS ( szs_file_t * szs )
{
    return fform_compr_force ? fform_compr_force : szs->fform_file;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressSZS ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressSZS(), compr=%d.%02d\n",
		opt_compr_mode, opt_compr );

    TRACE("CompressSZS(%p,%d)\n",szs,remove_uncompressed);
    DASSERT(szs);

    if ( !szs || !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    const file_format_t ff = GetNewCompressionSZS(szs);
    return CompressWith(szs,compr,remove_uncompressed,ff,FF_YAZ0);
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressYAZ ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressYAZ(), compr=%d.%02d\n",
		opt_compr_mode, opt_compr );
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;


    //--- setup yaz-control record

    yaz_compr_t yaz;
    memset(&yaz,0,sizeof(yaz));
    yaz.src_len  = szs->size;
    yaz.src      = szs->data;
    yaz.src_end  = szs->data + szs->size;
    yaz.szs      = szs;


    //--- compress data

    enumError err;
    if ( compr == COMPR_DEFAULT )
	compr = 9;
    if ( compr > 0 )
	err = ClassicCompressYAZ(&yaz,compr);
    else
    {
	switch(opt_compr_mode)
	{
	  case 11: err = BackTrackYAZ(&yaz,opt_compr); break; // back tracking
	  default: err = ClassicCompressYAZ(&yaz,0);
	}
    }


    //--- store compressed data

    if ( err == ERR_OK )
    {
	const uint used_len = yaz.dest_ptr - yaz.dest_buf;
	szs->csize = sizeof(yaz0_header_t) + used_len;
	szs->cdata = MALLOC(szs->csize);
	memcpy( szs->cdata + sizeof(yaz0_header_t), yaz.dest_buf, used_len );
	szs->cdata_alloced = true;

	yaz0_header_t * yaz0 = (yaz0_header_t*)szs->cdata;
	memset(yaz0,0,sizeof(*yaz0));
	yaz0->uncompressed_size = htonl(szs->size);

	szs->fform_file = GetYazFF(szs->fform_file);
	if ( szs->fform_file == FF_XYZ )
	    EncodeXYZ(szs->cdata,szs->cdata,szs->csize);
	else
	    memcpy( yaz0->magic,
		    szs->fform_file == FF_YAZ1 ? YAZ1_MAGIC : YAZ0_MAGIC,
		    sizeof(yaz0->magic) );

	if (remove_uncompressed)
	{
	    ClearContainerSZS(szs);
	    ClearUncompressedSZS(szs);
	}
    }

    if ( yaz.dest_buf != (u8*)iobuf )
	FREE(yaz.dest_buf);

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			wu8 coding			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[wu8_info_t]]

typedef struct wu8_info_t
{
    szs_file_t		*szs;		// the szs
    u8			*orig;		// a copy of the original data
    u32			size;		// data size
    u8			*temp;		// temp buffer
    uint		temp_size;	// size of 'temp'
    u8			base;		// base xor value
    u8			conv;		// xor value for additionally files
    bool		encode;		// false: decode, true: encode
    uint		pass1_count;	// number of files in pass1
    uint		err_count;	// number of errors

} wu8_info_t;

///////////////////////////////////////////////////////////////////////////////

static enumError SetupWU8Info ( wu8_info_t *wu8, szs_file_t *szs, bool encode )
{
    DASSERT(wu8);
    DASSERT(szs);

    memset(wu8,0,sizeof(*wu8));

    if (!IsAutoAddAvailable())
	return ERROR0(ERR_CANT_OPEN,
		"WU8-%sCODE: Can't find autoadd library.\n",
		encode ? "EN" : "DE" );

    wu8->szs	= szs;
    wu8->orig	= MEMDUP(szs->data,szs->size);
    wu8->size	= szs->size;
    wu8->encode	= encode;

    const u8 *p = (u8*)&wu8->size;
    wu8->conv = wu8->base = p[0] ^ p[1] ^ p[2] ^ p[3];
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ResetWU8Info ( wu8_info_t *wu8, file_format_t fform )
{
    DASSERT(wu8);
    DASSERT(wu8->szs);
    DASSERT(wu8->orig);
    DASSERT(wu8->size == wu8->szs->size);

    enumError err = ERR_OK;
    if (wu8->err_count)
    {
	memcpy(wu8->szs->data,wu8->orig,wu8->szs->size);
	err = ERR_ERROR;
    }
    else if ( fform != FF_UNKNOWN )
	wu8->szs->fform_current = fform;
    FREE(wu8->orig);
    FREE(wu8->temp);
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError load_wu8
(
    wu8_info_t		*wu8,		// valud wu8
    ccp			path,		// relative sub file path
    s64			*ret_size	// not NULL: return file size
)
{
    char load_path[PATH_MAX];
    s64 size = FindAutoAdd(path,0,load_path,sizeof(load_path));
    if (ret_size)
	*ret_size = size;

    if ( size <= 0 )
    {
	wu8->err_count++;
	return ERROR0(ERR_CANT_OPEN,
			"WU8-%sCODE: Missing file in autoadd library: %s\n",
			wu8->encode ? "EN" : "DE", path );
    }

    if ( wu8->temp_size < size )
    {
	FREE(wu8->temp);
	wu8->temp_size = ALIGN32(size,0x200000);
	wu8->temp = MALLOC(wu8->temp_size);
    }

    enumError err = LoadFILE( load_path, 0, 0, wu8->temp, size, 0, 0, false );
    if (err)
    {
	wu8->err_count++;
	return err;
    }

    wu8->conv ^= wu8->temp[size/2] ^ wu8->temp[size/3] ^ wu8->temp[size/4];
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static int code_wu8_pass_1
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if ( term || it->is_dir )
	return 0;

    wu8_info_t *wu8 = it->param;
    DASSERT(wu8);
    DASSERT(it->szs);
    DASSERT( wu8->szs == it->szs );

    ccp path = it->path;
    if ( path[0] == '.' && path[1] == '/' )
	path += 2;
    int fidx = FindDbFile(path);
    if ( fidx < 0 )
	return 0;

    const DbFileFILE_t *df = DbFileFILE+fidx;
    if (!DBF_ARCH_REQUIRED(df->flags))
	return 0;

    noPRINT("%sCODE-1 %s\n", wu8->encode ? "EN" : "DE", path );

    s64 size;
    enumError err = load_wu8(wu8,path,&size);
    if (err)
	return 0;

    u8 *xptr = wu8->temp;
    u8 *xend = xptr + size;
    wu8->pass1_count++;

    u8 *ptr = it->szs->data + it->off;
    u8 *end = ptr + it->size;

    while ( ptr < end )
    {
	if ( xptr == xend )
	    xptr = wu8->temp;
	*ptr++ ^= wu8->base ^ *xptr++;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int code_wu8_pass_2
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if ( term || it->is_dir )
	return 0;

    wu8_info_t *wu8 = it->param;
    DASSERT(wu8);
    DASSERT(it->szs);
    DASSERT( wu8->szs == it->szs );

    ccp path = it->path;
    if ( path[0] == '.' && path[1] == '/' )
	path += 2;
    int fidx = FindDbFile(path);
    if ( fidx >= 0 )
    {
	const DbFileFILE_t *df = DbFileFILE+fidx;
	if (DBF_ARCH_REQUIRED(df->flags))
	    return 0;
    }

    noPRINT("%sCODE-2 %s\n", wu8->encode ? "EN" : "DE", path );

    u8 *ptr = it->szs->data + it->off;
    u8 *end = ptr + it->size;
    while ( ptr < end )
	*ptr++ ^= wu8->conv;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static void iterate_wu8
(
    szs_file_t		*szs,	// valid szs
    wu8_info_t		*wu8	// valud wu8
)
{
    IterateFilesParSZS(szs,code_wu8_pass_1,wu8,false,false,false,0,-1,SORT_NONE);
    PRINT("PASS1-COUNT = %d, ERR-COUNT = %d\n",wu8->pass1_count,wu8->err_count);
    if (!wu8->err_count)
    {
	if (!wu8->pass1_count)
	    load_wu8(wu8,"itembox.brres",0);
	IterateFilesParSZS(szs,code_wu8_pass_2,wu8,false,false,false,0,-1,SORT_NONE);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecodeWU8 ( szs_file_t * szs )
{
    DASSERT(szs);
    if ( szs->fform_current != FF_WU8 || !szs->data || szs->size < sizeof(u8_header_t) )
	return ERR_OK;

    u8_header_t * u8p = (u8_header_t*)szs->data;
    if ( ntohl(u8p->magic) != WU8_MAGIC_NUM )
	return ERR_OK;

    PRINT("DecodeWU8()\n");

    wu8_info_t wu8;
    enumError err = SetupWU8Info(&wu8,szs,false);
    if (err)
	return err;

    u8p->magic = htonl(U8_MAGIC_NUM);
    u8 *ptr = szs->data + ntohl(u8p->node_offset);
    u8 *end = ptr + ntohl(u8p->fst_size);
    while ( ptr < end )
	*ptr++ ^= wu8.base;

    iterate_wu8(szs,&wu8);
    return ResetWU8Info(&wu8,FF_U8);
}

///////////////////////////////////////////////////////////////////////////////

enumError EncodeWU8 ( szs_file_t * szs )
{
    DASSERT(szs);
    if ( szs->fform_current != FF_U8 || !szs->data || szs->size < sizeof(u8_header_t) )
	return ERR_OK;

    u8_header_t * u8p = (u8_header_t*)szs->data;
    if ( ntohl(u8p->magic) != U8_MAGIC_NUM )
	return ERR_OK;

    PRINT("EncodeWU8()\n");

    wu8_info_t wu8;
    enumError err = SetupWU8Info(&wu8,szs,true);
    if (err)
	return err;

    iterate_wu8(szs,&wu8);
    u8p->magic = htonl(WU8_MAGIC_NUM);
    u8 *ptr = szs->data + ntohl(u8p->node_offset);
    u8 *end = ptr + ntohl(u8p->fst_size);
    while ( ptr < end )
	*ptr++ ^= wu8.base;
    return ResetWU8Info(&wu8,FF_WU8);
}

///////////////////////////////////////////////////////////////////////////////

u8 * DecodeXYZ
(
    u8		*dest,		// destination buffer,
				// If NULL: malloc() is used.
    const u8	*src,		// source data
    uint	size		// size of dest and source

)
{
    DASSERT(src);
    DASSERT( size >= sizeof(yaz0_header_t) );

    if (!dest)
	dest = MALLOC(size);

    u8 *dptr = dest;
    u8 *dend = dest + size;
    while ( dptr < dend )
	*dptr++ = *src++ ^ 0xdc;

    memcpy(dest,YAZ0_MAGIC,4);
    return dest;
}

///////////////////////////////////////////////////////////////////////////////

u8 * EncodeXYZ
(
    u8		*dest,		// destination buffer,
				// If NULL: malloc() is used.
    const u8	*src,		// source data
    uint	size		// size of dest and source

)
{
    DASSERT(src);
    DASSERT( size >= sizeof(yaz0_header_t) );

    dest = DecodeXYZ(dest,src,size);
    memcpy(dest,XYZ0_MAGIC,4);
    return dest;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  BZ/YBZ/BZIP2 helper		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[c_bzip2_t]]

typedef struct c_bzip2_t
{
    enumError	err;
    u8		*cdata;
    uint	csize;
    int		compr_level;
}
c_bzip2_t;

///////////////////////////////////////////////////////////////////////////////

static c_bzip2_t get_compressed_bzip2
(
    szs_file_t	*szs,		// valid SZS data
    uint	header_size,	// insert 'header_size' bytes before dest data
    int		compr,		// compression level or COMPR_DEFAULT
    bool	add_dec_size	// true: add decompressed size before BZIP2 header
)
{
    c_bzip2_t res = {0};
    const int max_compr = szs->size / 100000 + 1;
    if ( compr == COMPR_DEFAULT )
	compr = BZIP2_DEFAULT_COMPR;

    if ( compr < 1 && ( opt_norm || opt_compr_mode > 1 ))
    {
	res.csize = UINT_MAX;

	static int ctab[] = { 1, 9, 8, 2, 5, 0 }, *cptr;
	int n = opt_norm ? 2 : opt_compr_mode;
	for ( cptr = ctab; *cptr > 0 && n > 0; cptr++, n-- )
	{
	    int compr = *cptr;
	    if ( compr >= max_compr )
	    {
		if ( compr < 9 )
		    continue;
		compr = max_compr;
	    }
	    PRINT("TRY COMPRESSION %u => %u/%u\n",*cptr,compr,max_compr);

	    u8  *temp_data;
	    uint temp_size;
	    enumError err = EncodeBZIP2( &temp_data, &temp_size, true,
		    header_size, add_dec_size, szs->data, szs->size, compr );
	    if (err)
		continue;
	    if ( temp_size < res.csize )
	    {
		PRINT("  => USE COMPRESSION %u\n",compr);
		FREE(res.cdata);
		res.cdata = temp_data;
		res.csize = temp_size;
	    }
	    else
		FREE(temp_data);
	}
	if (!res.cdata)
	    res.err = ERR_BZIP2;
    }
    else
    {
	if (!compr)
	    compr = opt_compr_mode > 0 ? opt_compr : BZIP2_DEFAULT_COMPR;
	if ( compr > max_compr )
	     compr = max_compr;

	PRINT0("  => USE COMPRESSION %u/%u\n",compr,max_compr);
	enumError err = EncodeBZIP2( &res.cdata, &res.csize, true,
		    header_size, add_dec_size, szs->data, szs->size, compr );
	if (err)
	    res.err = err;
    }
    res.compr_level = compr;
    return res;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  BZ support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecompressBZ ( szs_file_t * szs, bool rm_compressed )
{
    PRINT("DecompressBZ(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    const wbz_header_t *wh = (wbz_header_t*)szs->cdata;
    if (memcmp(wh->magic,BZ_MAGIC,sizeof(wh->magic)))
	return ERROR0(ERR_INVALID_DATA,"Invalid BZ magic!\n");

    u8 *data;
    uint size;
    enumError err = DecodeBZIP2( &data, &size, 0,
		wh->cdata, szs->csize - sizeof(*wh) );
    if (err)
	return err;

    szs->data = data;
    szs->size = size;
// [[analyse-magic]]
    szs->fform_arch = szs->fform_current = GetByMagicFF(data,size,size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
// [[version-suffix]]
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressBZ ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressBZ(%p,%d,%d)\n",szs,compr,remove_uncompressed);
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    if ( compr < 1 || compr == BZIP2_DEFAULT_COMPR )
    {
	if (CheckSZSCacheSZS(szs,FF_BZ,".bz",remove_uncompressed))
	    return ERR_OK;
    }

    c_bzip2_t c = get_compressed_bzip2(szs,sizeof(wbz_header_t),compr,true);
    if (c.err)
	return c.err;
    szs->cdata = c.cdata;
    szs->csize = c.csize;
    //HexDump16(stdout,0,0,c.cdata,32);

    wbz_header_t *wh = (wbz_header_t*)c.cdata;
    memcpy(wh->magic,BZ_MAGIC,sizeof(wh->magic));
    memcpy(wh->first_8,szs->data,sizeof(wh->first_8));
    szs->fform_file = FF_BZ;

    ClearContainerSZS(szs);
    if (remove_uncompressed)
	ClearUncompressedSZS(szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  YBZ support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecompressYBZ ( szs_file_t * szs, bool rm_compressed )
{
    PRINT("DecompressYBZ(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    const ybz_header_t *yh = (ybz_header_t*)szs->cdata;
    if (memcmp(yh->magic,YBZ_MAGIC,sizeof(yh->magic)))
	return ERROR0(ERR_INVALID_DATA,"Invalid YBZ magic!\n");

    const uint usize = ntohl(yh->uncompressed_size);
    u8 *data = MALLOC(usize);
    uint size;
    enumError err = DecodeBZIP2buf( data, usize, &size,
		yh->cdata, szs->csize - sizeof(ybz_header_t) );
    if (err)
    {
	FREE(data);
	return err;
    }

    szs->data = data;
    szs->size = size;
// [[analyse-magic]]
    szs->fform_arch = szs->fform_current = GetByMagicFF(data,size,size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
// [[version-suffix]]
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressYBZ ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressYBZ(%p,%d,%d)\n",szs,compr,remove_uncompressed);
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    if ( compr < 1 || compr == BZIP2_DEFAULT_COMPR )
    {
	if (CheckSZSCacheSZS(szs,FF_BZ,".ybz",remove_uncompressed))
	    return ERR_OK;
    }

    c_bzip2_t c = get_compressed_bzip2(szs,sizeof(ybz_header_t),compr,false);
    if (c.err)
	return c.err;
    szs->cdata = c.cdata;
    szs->csize = c.csize;
    //HexDump16(stdout,0,0,c.cdata,32);

    ybz_header_t *yh = (ybz_header_t*)c.cdata;
    memcpy(yh->magic,YBZ_MAGIC,sizeof(yh->magic));
    memcpy(yh->first_4,szs->data,sizeof(yh->first_4));
    yh->uncompressed_size = htonl(szs->size);
    yh->compressed_size = htonl(c.csize-sizeof(ybz_header_t));
    szs->fform_file = FF_YBZ;

    ClearContainerSZS(szs);
    if (remove_uncompressed)
	ClearUncompressedSZS(szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  BZIP2 support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecompressBZIP2 ( szs_file_t * szs, bool rm_compressed )
{
    PRINT("DecompressBZIP2(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    u8 *data;
    uint size;
    enumError err = DecodeBZIP2bin(&data,&size,0,szs->cdata,szs->csize);
    if (err)
	return err;

    szs->data = data;
    szs->size = size;
    szs->fform_arch = szs->fform_current = GetByMagicFF(data,size,size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressBZIP2 ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressBZIP2(%p,%d,%d)\n",szs,compr,remove_uncompressed);
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    if ( compr < 1 || compr == BZIP2_DEFAULT_COMPR )
    {
	if (CheckSZSCacheSZS(szs,FF_BZIP2,".bz2",remove_uncompressed))
	    return ERR_OK;
    }

    c_bzip2_t c = get_compressed_bzip2(szs,0,compr,false);
    if (c.err)
	return c.err;
    szs->cdata = c.cdata;
    szs->csize = c.csize;
    szs->fform_file = FF_BZIP2;
    //HexDump16(stdout,0,0,c.cdata,32);

    ClearContainerSZS(szs);
    if (remove_uncompressed)
	ClearUncompressedSZS(szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  LZ support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecompressLZ ( szs_file_t * szs, bool rm_compressed )
{
    PRINT("DecompressLZ(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    const wlz_header_t *wh = (wlz_header_t*)szs->cdata;
    if (memcmp(wh->magic,LZ_MAGIC,sizeof(wh->magic)))
	return ERROR0(ERR_INVALID_DATA,"Invalid LZ magic!\n");

    u8 *data;
    uint size;
    const uint delta = sizeof(wlz_header_t) + sizeof(u32);
    enumError err = DecodeLZMAbin( &data, &size, 0,
		szs->cdata + delta, szs->csize - delta );
    if (err)
	return err;

    szs->data = data;
    szs->size = size;
// [[analyse-magic]]
    szs->fform_arch = szs->fform_current = GetByMagicFF(data,size,size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
// [[version-suffix]]
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressLZ ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressLZ(%p,%d)\n",szs,remove_uncompressed);
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    if ( compr == COMPR_DEFAULT )
	compr = LZMA_DEFAULT_COMPR;
    else if (!compr)
	compr = opt_compr_mode > 0 ? opt_compr : LZMA_DEFAULT_COMPR;
    compr = CalcCompressionLevelLZMA(compr);
    PRINT0("  => USE LZ COMPRESSION %u\n",compr);

    if ( compr == LZMA_DEFAULT_COMPR
	&& CheckSZSCacheSZS(szs,FF_LZ,".lz",remove_uncompressed))
    {
	return ERR_OK;
    }

    u8  *cdata;
    uint csize;
    enumError err = EncodeLZMA( &cdata, &csize, true,
		    sizeof(wlz_header_t), true, szs->data, szs->size, compr );
    if (err)
	return err;
    szs->cdata = cdata;
    szs->csize = csize;

    wlz_header_t *wh = (wlz_header_t*)cdata;
    memcpy(wh->magic,LZ_MAGIC,sizeof(wh->magic));
    memcpy(wh->first_8,szs->data,sizeof(wh->first_8));
    szs->fform_file = FF_LZ;
    //HexDump16(stdout,0,0,cdata,32);

    ClearContainerSZS(szs);
    if (remove_uncompressed)
	ClearUncompressedSZS(szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  YLZ support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[xcompr+]]

enumError DecompressYLZ ( szs_file_t * szs, bool rm_compressed )
{
    PRINT("DecompressYLZ(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    const ylz_header_t *yh = (ylz_header_t*)szs->cdata;
    if (memcmp(yh->magic,YLZ_MAGIC,sizeof(yh->magic)))
	return ERROR0(ERR_INVALID_DATA,"Invalid YLZ magic!\n");

    u8 *data;
    uint size;
    const uint delta = sizeof(ylz_header_t);
    enumError err = DecodeLZMAbin( &data, &size, 0,
		szs->cdata + delta, szs->csize - delta );
    if (err)
	return err;

    szs->data = data;
    szs->size = size;
// [[analyse-magic]]
    szs->fform_arch = szs->fform_current = GetByMagicFF(data,size,size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
// [[version-suffix]]
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressYLZ ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT0("CompressYLZ(%p,%d)\n",szs,remove_uncompressed);
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    if ( compr == COMPR_DEFAULT )
	compr = LZMA_DEFAULT_COMPR;
    else if (!compr)
	compr = opt_compr_mode > 0 ? opt_compr : LZMA_DEFAULT_COMPR;
    compr = CalcCompressionLevelLZMA(compr);
    PRINT0("  => USE YLZ COMPRESSION %u\n",compr);

    if ( compr == LZMA_DEFAULT_COMPR
	&& CheckSZSCacheSZS(szs,FF_YLZ,".ylz",remove_uncompressed))
    {
	return ERR_OK;
    }

    u8  *cdata;
    uint csize;
    enumError err = EncodeLZMA( &cdata, &csize, true,
		    sizeof(ylz_header_t), false, szs->data, szs->size, compr );
    if (err)
	return err;
    szs->cdata = cdata;
    szs->csize = csize;

    ylz_header_t *yh = (ylz_header_t*)cdata;
    memcpy(yh->magic,YLZ_MAGIC,sizeof(yh->magic));
    memcpy(yh->first_4,szs->data,sizeof(yh->first_4));
    yh->uncompressed_size = htonl(szs->size);
    yh->compressed_size = htonl(csize-sizeof(ylz_header_t));
    szs->fform_file = FF_YLZ;
    //HexDump16(stdout,0,0,cdata,32);

    ClearContainerSZS(szs);
    if (remove_uncompressed)
	ClearUncompressedSZS(szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  LZMA support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecompressLZMA ( szs_file_t * szs, bool rm_compressed )
{
    PRINT("DecompressLZMA(%p,%d)\n",szs,rm_compressed);
    DASSERT(szs);

    if ( !szs->csize || !szs->cdata || szs->data )
	return ERR_OK;

    u8 *data;
    uint size;
    enumError err = DecodeLZMAbin(&data,&size,0,szs->cdata,szs->csize);
    if (err)
	return err;

    szs->data = data;
    szs->size = size;
    szs->fform_arch = szs->fform_current = GetByMagicFF(data,size,size);
    szs->ff_attrib  = GetAttribFF(szs->fform_arch);
    szs->ff_version = GetVersionFF(szs->fform_arch,szs->data,szs->size,0);

    ClearContainerSZS(szs);
    if (rm_compressed)
	ClearCompressedSZS(szs);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CompressLZMA ( szs_file_t * szs, int compr, bool remove_uncompressed )
{
    PRINT("CompressLZMA(%p,%d)\n",szs,remove_uncompressed);
    DASSERT(szs);

    if ( !szs->size || !szs->data || szs->cdata )
	return ERR_OK;

    if ( compr == COMPR_DEFAULT )
	compr = LZMA_DEFAULT_COMPR;
    else if (!compr)
	compr = opt_compr_mode > 0 ? opt_compr : LZMA_DEFAULT_COMPR;
    compr = CalcCompressionLevelLZMA(compr);
    PRINT0("  => USE LZMA COMPRESSION %u\n",compr);

    if ( compr == LZMA_DEFAULT_COMPR
	&& CheckSZSCacheSZS(szs,FF_LZMA,".lzma",remove_uncompressed))
    {
	return ERR_OK;
    }

    u8  *cdata;
    uint csize;
    enumError err = EncodeLZMA( &cdata, &csize, true,
		    0, false, szs->data, szs->size, compr );

    if (err)
	return err;
    szs->cdata = cdata;
    szs->csize = csize;
    szs->fform_file = FF_LZMA;
    //HexDump16(stdout,0,0,cdata,32);

    ClearContainerSZS(szs);
    if (remove_uncompressed)
	ClearUncompressedSZS(szs);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			create u8			///////////////
///////////////////////////////////////////////////////////////////////////////

int PrintFileHeadSZS ( int fw_name )
{
    if ( fw_name <= 0 )
	fw_name = 60;
    if ( fw_name <= 17 )
	fw_name = 17;
    fw_name += 60;

    printf("\n%s idx      offset/hex   size/hex size/dec magic f.form vers file or directory\n"
	   "%s%.*s%s\n",
	   colout->heading, colout->heading, 3*fw_name, ThinLine300_3, colout->reset );
    return fw_name;
}

///////////////////////////////////////////////////////////////////////////////

void PrintVersionSZS ( char *buf, uint buf_size, struct szs_iterator_t *it )
{
    DASSERT(buf);
    DASSERT(it);
    *buf = 0;

// [[version-suffix]]

    const u8 * data = it->szs->data + it->off;
    file_format_t fform = GetByMagicFF(data,it->size,0);
    if ( IsBRSUB(fform) && it->size >= 12 )
    {
	const int vers = be32(data+8);
	const brsub_info_t *bi = GetInfoBRSUB(fform,vers);
	if ( !bi || bi->warn >= BIMD_FAIL )
	    snprintf(buf,buf_size,"%s%3d!%s",
			colout->fatal, vers, colout->reset );
	else if ( bi->warn >= BIMD_INFO )
	    snprintf(buf,buf_size,"%s%3d %s",
			colout->hint, vers, colout->reset );
	else
	    snprintf(buf,buf_size,"%3d ",vers);
	return;
    }
    else
    {
	const int vers = GetVersionFF(fform,data,it->size,0);
	if ( vers >= 0 )
	{
	    snprintf(buf,buf_size,"%3u ",vers);
	    return;
	}
    }

    strncpy(buf,"  - ",buf_size);
}

///////////////////////////////////////////////////////////////////////////////

int PrintFileSZS
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if (term)
	return 0;

    if (it->is_dir)
    {
	printf("%s%3x: %7x<       -%8x>       -  -    DIR      -  %.*s%s%s\n",
		colset->cmd,
		it->index, it->off, it->size,
		2 * it->recurse_level, indent_msg, it->path,
		colset->reset );
    }
    else
    {
	const ColorSet_t * col = GetFileColorSet( it->has_subfiles ? stdout : 0 );

	const u8 * data = it->szs->data+it->off;
// [[analyse-magic+]]
// [[ff_fallback]]
	const file_format_t fform
		= AnalyseMagicByOpt(0,data,it->size,it->szs->size-it->off,
					FF_UNKNOWN, it->path);
	FixIteratorExtByFF(it,fform,fform);

	char vers_buf[50];
	PrintVersionSZS(vers_buf,sizeof(vers_buf),it);

	printf("%s%3x: %8x..%8x %7x %8u  %-4s %-6s %s %s%.*s%s%s\n",
		col->heading,
		it->index, it->off, it->off+it->size, it->size, it->size,
		PrintID(data, it->size < 4 ? it->size : 4, 0 ),
		GetNameFF(0,fform),
		vers_buf,
		it->no_dirs ? "": " ",
		2 * it->recurse_level, indent_msg, it->path,
		col->reset );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateU8
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    u32			namepool_size,	// total namepool_size
    u32			total_size,	// total file size
    bool		create_pt_dir	// create directory '.' as base
)
{
    DASSERT(szs);
    PRINT("CreateU8() namesize=%x, totsize=%x, align=%x, .dir=%d\n",
		namepool_size, total_size, opt_align_u8, create_pt_dir );

    //--- sort

    SortSubFilesSZS(szs,SORT_U8);


    //--- alloc data and setup

    if ( opt_pt_dir < 0 )
	create_pt_dir = false;
    else if ( opt_pt_dir > 0 )
	create_pt_dir = true;

    namepool_size += create_pt_dir ? 3 : 1; // extra byte for root node and "."

    const size_t n_files	= szs->subfile.used + 1 + create_pt_dir;
    const size_t dir_tab_size	= n_files * sizeof(u8_node_t);
    const size_t fst_size	= ALIGN32( dir_tab_size + namepool_size, opt_align_u8 );
    const size_t u8_head_size	= ALIGN32( sizeof(u8_header_t), opt_align_u8 );
    const size_t head_size	= ALIGN32( u8_head_size + fst_size, opt_align_u8 );

    szs->fform_arch		= FF_U8;
    szs->fform_current		= FF_U8;
    szs->ff_attrib		= GetAttribFF(szs->fform_arch);
    szs->ff_version		= -1;
    szs->data_alloced		= true;
    szs->file_size = szs->size	= head_size + total_size;
    szs->data			= CALLOC(1,szs->size);

    u8_header_t	* u8head	= (u8_header_t*)szs->data;
    u8_node_t	* u8node0	= (u8_node_t*)(szs->data+u8_head_size);
    u8_node_t	* u8node	= u8node0;
    char	* name_pool	= (char*)u8node + dir_tab_size;
    u8		* data_ptr	= szs->data + head_size;

    PRINT("ADDR: %zx..%zx..%zx..%zx..%zx\n",
		(u8*)u8head - szs->data,
		(u8*)u8node - szs->data,
		(u8*)name_pool - szs->data,
		(u8*)data_ptr - szs->data,
		szs->size );

    u8head->magic	= htonl( U8_MAGIC_NUM );
    u8head->node_offset	= htonl( u8_head_size );
    u8head->fst_size	= htonl( dir_tab_size + namepool_size );
    u8head->data_offset	= htonl( data_ptr - szs->data );

    if ( u8_head_size > sizeof(u8_header_t) )
	memset(u8head->padding,0xcc,u8_head_size-sizeof(u8_header_t));

    const int MAX_DIR_DEPTH = 50;
    u32 basedir[MAX_DIR_DEPTH+1];
    memset(basedir,0,sizeof(basedir));


    //--- root file

    char * name_ptr = name_pool;
    u8node->name_off = htonl(name_ptr-name_pool);
    *name_ptr++ = 0;
    u8node->is_dir = 1;
    u8node->size = htonl(n_files);
    u8node++;


    //--- "." file

    if (create_pt_dir)
    {
	u8node->name_off = htonl(name_ptr-name_pool);
	*name_ptr++ = '.';
	*name_ptr++ = 0;
	u8node->is_dir = 1;
	u8node->size = htonl(n_files);
	u8node++;
	basedir[1] = 1;
    }


    //--- prepare link support

    u8_node_t **u8link = 0;
    if (szs->subfile.link_count)
	u8link = CALLOC(szs->subfile.link_count+1,sizeof(*u8link));


    //--- iterate files

    enumError max_err = ERR_OK;
    int idx;
    for ( idx = 0; idx < szs->subfile.used; idx++ )
    {
	szs_subfile_t * f = szs->subfile.list + idx;
	u8node->name_off = htonl(name_ptr-name_pool);

	u8node->is_dir = f->is_dir;
	bool append_name = true;
	if (f->is_dir)
	{
	    u8node->size = htonl( idx + f->size + 2 + create_pt_dir );
	    noPRINT("DIR: %d..%d\n",idx,ntohl(u8node->size));

	    u32 depth = f->offset;
	    if ( depth < MAX_DIR_DEPTH )
	    {
		noPRINT("depth=%d -> %d\n",depth,basedir[depth]);
		u8node->offset = htonl(basedir[depth]);
		basedir[depth+1] = u8node - u8node0;
	    }
	}
	else if ( u8link && u8link[f->link_index] )
	{
	    u8_node_t *u8src = u8link[f->link_index];
	    PRINT0("LINK/USE[%u]: %s  ->  #%zu\n",
			f->link_index, f->path, u8src-u8node0 );
	    u8node->offset = u8src->offset;
	    u8node->size   = u8src->size;
	}
	else
	{
	    PRINT0("** %#x %#x %s\n",f->offset,f->size,f->path);
	    //DASSERT ( data_ptr + f->size <= szs->data + szs->size );
	    uint relevant_size;
	    if (f->ext)
	    {
		if (f->ext->removed)
		{
		    append_name = false;
		    relevant_size = 0;
		}
		else
		{
		    relevant_size = f->ext->size;
		    PRINT("USE EXT: %s, %u B, %p\n",f->path,relevant_size,f->ext->load_path);
		    DASSERT(data_ptr + relevant_size <= szs->data + szs->size );
		    memcpy(data_ptr,f->ext->load_path,relevant_size);
		}
	    }
	    else if ( f->data && !f->load_path )
	    {
		relevant_size = f->size;
		memcpy(data_ptr,f->data,relevant_size);
	    }
	    else if ( source_data && !f->load_path )
	    {
		relevant_size = f->size;
		memcpy(data_ptr,source_data+f->offset,relevant_size);
	    }
	    else
	    {
		relevant_size = f->size;
		enumError err = f->load_path
			? LoadFILE( f->load_path, 0, 0,
					data_ptr, relevant_size, 0, &szs->fatt, true )
			: LoadFILE( source_dir, f->path, 0,
					data_ptr, relevant_size, 0, &szs->fatt, true );

		if ( max_err < err )
		    max_err = err;
	    }
	    f->offset      = data_ptr - szs->data;
	    u8node->offset = htonl(f->offset);
	    u8node->size   = htonl( relevant_size );
	    data_ptr += ALIGN32(relevant_size,opt_align_u8);

	    if (f->link_index)
	    {
		PRINT("LINK/SET[%u]: %s  ->  #%zu\n",
			f->link_index, f->path, u8node-u8node0 );
		u8link[f->link_index] = u8node;
	    }

	    DASSERT_MSG ( data_ptr <= szs->data + szs->size,
		"data overlow: %zu=0x%zx bytes\n",
			data_ptr - (szs->data + szs->size),
			data_ptr - (szs->data + szs->size) );
	}

	if (append_name)
	{
	    ccp pend = f->path + strlen(f->path);
	    if ( pend > f->path && pend[-1] == '/' )
		 pend--;
	    ccp pstart = pend;
	    while ( pstart > f->path && pstart[-1] != '/' )
		pstart--;
	    noPRINT("NAME: |%.*s| %zu+%zu/%zu\n",
		    (int)(pend-pstart), pstart,
		    name_ptr-name_pool, pend-pstart, namepool_size );
	    while ( pstart < pend )
		*name_ptr++ = *pstart++;
	    *name_ptr++ = 0;
	    DASSERT_MSG( name_ptr <= name_pool + namepool_size,
		"name pool overflow: %zx/%x\n", name_ptr - name_pool, namepool_size );
	}

	u8node++;
	DASSERT( (ccp)u8node <= name_pool );
    }
    DASSERT( (ccp)u8node == name_pool );
    DASSERT ( data_ptr <= szs->data + szs->size );
    DASSERT_MSG( name_ptr == name_pool + namepool_size,
		"name pool underflow: %zx/%x\n", name_ptr - name_pool, namepool_size );

    FREE(u8link);
    ClearSpecialFilesSZS(szs);


    //--- debugging

    if ( logging >= 1 )
    {
	printf("----- internal file list -----\n");
	IterateFilesParSZS(szs,PrintFileSZS,0,false,true,false,0,0,SORT_NONE);
	printf("------------------------------\n");
    }

    return max_err;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateU8ByInfo
(
    szs_file_t		* szs,		// valid szs
    ccp			source_dir,	// NULL or path to source dir
    u8			* source_data,	// NULL or source data
    szs_u8_info_t	* u8info	// valid data
)
{
    DASSERT(szs);
    DASSERT(u8info);
    return CreateU8( szs, source_dir, source_data,
		u8info->namepool_size, u8info->total_size, u8info->have_pt_dir );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_extract_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeExtractSZS ( szs_extract_t * eszs )
{
    DASSERT(eszs);
    memset(eszs,0,sizeof(*eszs));
    eszs->exlevel = EXLEV_N; // == not found
    eszs->endian  = &be_func;
}

///////////////////////////////////////////////////////////////////////////////

void ResetExtractSZS ( szs_extract_t * eszs )
{
    DASSERT(eszs);
    if (eszs->data_alloced)
	FREE(eszs->data);
    FreeString(eszs->fname);
    FreeString(eszs->found_path);
    FreeContainer(eszs->old_container);
    ResetContainer(&eszs->container);
    InitializeExtractSZS(eszs);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Container_t * SetupContainerESZS ( szs_extract_t * eszs )
{
    DASSERT(eszs);
    AssignContainer( &eszs->container, eszs->container.protect_level,
			eszs->data, eszs->data_size,
			eszs->data_alloced ? CPM_MOVE : CPM_LINK );
    eszs->data_alloced = false;
 #ifdef TEST
    DumpInfoContainer(stdlog,collog,2,"ESZS: ",&eszs->container,0);
 #endif
    return &eszs->container;
}

///////////////////////////////////////////////////////////////////////////////

ContainerData_t * LinkContainerESZS ( szs_extract_t * eszs )
{
    DASSERT(eszs);
    if (!IsValidContainer(&eszs->container))
	SetupContainerESZS(eszs);
    return LinkContainerData(&eszs->container);
}

///////////////////////////////////////////////////////////////////////////////

ContainerData_t * MoveContainerESZS ( szs_extract_t * eszs )
{
    DASSERT(eszs);
    if (!IsValidContainer(&eszs->container))
	SetupContainerESZS(eszs);
    return MoveContainerData(&eszs->container);
}

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * GetOldContainerESZS ( szs_extract_t * eszs )
{
    DASSERT(eszs);
// old [[container]] will be replaced by Container_t & ContainerData_t
    if (!eszs->old_container)
    {
	if (!IsValidContainer(&eszs->container))
	    SetupContainerESZS(eszs);
	eszs->old_container = UseSubContainer(0,eszs->container.cdata);
    }

    return AllocContainer(eszs->old_container);
}

///////////////////////////////////////////////////////////////////////////////

static int node_found
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    szs_extract_level_t		level	// found level
)
{
    DASSERT(it);
    szs_extract_t * eszs = it->param;
    DASSERT(eszs);

    if ( eszs->exlevel > level )
    {
	eszs->exlevel	   = level;
	eszs->found_offset = it->off;
	eszs->found_size   = it->size;
	eszs->found_count  = 1;
	FreeString(eszs->found_path);
	eszs->found_path   = STRDUP(it->path);
	eszs->endian	   = it->endian;
    }
    else if ( eszs->exlevel == level )
    {
	DASSERT(eszs->found_count);
	DASSERT(eszs->found_path);
	eszs->found_count++;
    }

    PRINT("*** FOUND: lev=%d n=%d off=%x siz=%x %s\n",
	eszs->exlevel, eszs->found_count,
	eszs->found_offset, eszs->found_size, eszs->found_path );

    return eszs->exlevel == 0;
}

///////////////////////////////////////////////////////////////////////////////

static int extract_data_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    if ( !term && !it->is_dir )
    {
	szs_extract_t * eszs = it->param;
	DASSERT(eszs);
	DASSERT(eszs->subpath);
	DASSERT( eszs->exlevel > 0 && eszs->exlevel <= EXLEV_N );

	if (!strcmp(it->path,eszs->subpath))
	    return node_found(it,0);

	ccp nopt_path = it->path[0] == '.' && it->path[1] == '/' ? it->path + 2 : 0;
	if ( nopt_path && !strcmp(nopt_path,eszs->subpath) )
	    return node_found(it,EXLEV_F_RMPT);

	if ( eszs->exlevel >= EXLEV_F_ICASE
		&& !strcasecmp(it->path,eszs->subpath))
	    return node_found(it,EXLEV_F_ICASE);

	if (  eszs->exlevel >= (EXLEV_F_ICASE|EXLEV_F_RMPT)
		&& nopt_path
		&& !strcasecmp(nopt_path,eszs->subpath) )
	    return node_found(it,EXLEV_F_ICASE|EXLEV_F_RMPT);

	if ( eszs->exlevel >= EXLEV_F_MATCH )
	{
	    uint path_len = strlen(it->path);
	    if ( path_len > eszs->subpath_len )
	    {
		ccp cmp_path = it->path + path_len - eszs->subpath_len;

		if (!strcmp(cmp_path,eszs->subpath))
		    return node_found(it,EXLEV_F_MATCH);

		if ( eszs->exlevel >= (EXLEV_F_MATCH|EXLEV_F_ICASE)
			&& !strcasecmp(cmp_path,eszs->subpath) )
		    return node_found(it,EXLEV_F_MATCH|EXLEV_F_ICASE);
	    }
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError ExtractSZS
(
    szs_extract_t	* eszs,		// valid data structure
    bool		initialize,	// true: initialize 'eszs'
    ccp			fname,		// filename of source
    ccp			autoname,	// not NULL: Use this filename
					//           if subfile name is "" or "/"
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(eszs);
    DASSERT(fname);

    if (initialize)
	InitializeExtractSZS(eszs);
    else
	ResetExtractSZS(eszs);

    char path[PATH_MAX];
    ccp subpath = SplitSubPath(path,sizeof(path),fname);

    PRINT0("ExtractSZS() FF: %s -> %s\n",
		GetNameFF(eszs->fform_file,eszs->fform_arch),
		GetNameFF(0,eszs->fform_current) );

    if ( !subpath || !*subpath && !autoname )
	return ERR_OK;

    eszs->subfile_found = true;

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,path,true,false,true);
    if (err)
    {
	ResetSZS(&szs);
	return err;
    }

    const bool is_arch = IsArchiveFF(szs.fform_current);
    if ( !is_arch || szs.fform_current == FF_BRRES )
	autoname = 0;
    else if ( autoname && ( !*subpath || !strcmp(subpath,"/") || !strcmp(subpath,"|") ))
	subpath = autoname;

    if ( !is_arch || !subpath || !*subpath )
    {
	eszs->subfile_found = false;
	ResetSZS(&szs);
	return ERR_OK;
    }

    eszs->szs_found	= true;
    eszs->subpath	= subpath + 1;
    eszs->subpath_len	= strlen(eszs->subpath);
    IterateFilesParSZS(&szs,extract_data_func,eszs,false,false,false,0,-1,SORT_NONE);

    PRINT("### FOUND: lev=%d n=%d off=%x siz=%x %s\n",
	eszs->exlevel, eszs->found_count,
	eszs->found_offset, eszs->found_size, eszs->found_path );

    if ( eszs->found_count == 1 )
    {
	DASSERT(eszs->exlevel < EXLEV_N );
	DASSERT(eszs->found_path);

	uint p1len = strlen(path);
	uint p2len = strlen(eszs->found_path);
	eszs->fname = MALLOC(p1len+p2len+2);
	sprintf((char*)eszs->fname,"%s/%s",path,eszs->found_path);

	eszs->data_alloced = true;
	eszs->data_size = eszs->found_size;
	eszs->data = MALLOC(eszs->found_size);
	memcpy( eszs->data, szs.data + eszs->found_offset, eszs->found_size );
    }
    else if ( ignore_no_file )
	err = ERR_NOT_EXISTS;
    else if (!eszs->found_count)
	err = ERROR0(ERR_CANT_OPEN,
		"Sub file not exist: %s -> %s\n", path, subpath+1 );
    else
	err = ERROR0(ERR_CANT_OPEN,
		"Sub file ambiguous (N=%d): %s -> %s\n", eszs->found_count, path, subpath+1 );

    eszs->subpath	= 0;
    eszs->subpath_len	= 0;
    eszs->fform_file	= szs.fform_file;
    eszs->fform_arch	= szs.fform_arch;
    eszs->fform_current	= szs.fform_arch;
    ResetSZS(&szs);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError PrintErrorExtractSZS
(
    szs_extract_t	* eszs,		// valid data structure
    ccp			fname		// filename of source
)
{
    DASSERT(eszs);
    DASSERT(fname);

    if (eszs->data)
	return ERR_OK;

    if (eszs->szs_found)
	return ERROR0(ERR_CANT_OPEN,"Sub file not found: %s\n",eszs->subpath);

    if (eszs->subfile_found)
	return ERROR0(ERR_CANT_OPEN,"No SZS/U8 file found: %s\n",fname);

    return ERROR0(ERR_CANT_OPEN,"No sub file specified: %s\n",fname);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    raw_data_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeRawData ( raw_data_t * raw )
{
    DASSERT(raw);
    memset(raw,0,sizeof(*raw));
    raw->fname = EmptyString;
}

///////////////////////////////////////////////////////////////////////////////

void ResetRawData ( raw_data_t * raw )
{
    DASSERT(raw);
    FreeString(raw->fname);
    if (raw->data_alloced)
	FREE(raw->data);
    FreeContainer(raw->old_container);
    ResetContainer(&raw->container);
    InitializeRawData(raw);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Container_t * SetupContainerRawData ( raw_data_t * raw )
{
    DASSERT(raw);
    AssignContainer( &raw->container, raw->container.protect_level,
			raw->data, raw->data_size,
			raw->data_alloced ? CPM_MOVE : CPM_LINK );
    raw->data_alloced = false;
 #ifdef TEST
    DumpInfoContainer(stdlog,collog,2,"ESZS: ",&raw->container,0);
 #endif
    return &raw->container;
}

///////////////////////////////////////////////////////////////////////////////

ContainerData_t * LinkContainerRawData ( raw_data_t * raw )
{
    DASSERT(raw);
    if (!IsValidContainer(&raw->container))
	SetupContainerRawData(raw);
    return LinkContainerData(&raw->container);
}

///////////////////////////////////////////////////////////////////////////////

ContainerData_t * MoveContainerRawData ( raw_data_t * raw )
{
    DASSERT(raw);
    if (!IsValidContainer(&raw->container))
	SetupContainerRawData(raw);
    return MoveContainerData(&raw->container);
}

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * ContainerRawData ( raw_data_t * raw )
{
    DASSERT(raw);
// old [[container]] will be replaced by Container_t & ContainerData_t
    if (!raw->old_container)
    {
	if (!IsValidContainer(&raw->container))
	    SetupContainerRawData(raw);
	raw->old_container = UseSubContainer(0,raw->container.cdata);
    }

    return AllocContainer(raw->old_container);
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadRawData
(
    raw_data_t		* raw,		// data structure
    bool		init_raw,	// true: initialize 'raw' first
    ccp			fname,		// valid pointer to filenname
    ccp			autoname,	// not NULL: Use this filename
					//           if subfile name is "" or "/"
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    file_format_t	allow_0		// not NULL (FF_UNKNOWN):
					//	allow "0" for an empty file and set FF
)
{
    DASSERT(raw);
    DASSERT(fname);
    if (init_raw)
	InitializeRawData(raw);
    else
	ResetRawData(raw);


    //--- special case "0"

    if ( allow_0 && !strcmp(fname,"0") )
    {
	raw->fform = allow_0;
	raw->is_0  = true;
	raw->fname = STRDUP("<empty>");
	return ERR_OK;
    }


    //--- load data

    szs_extract_t eszs;
    enumError err = ExtractSZS(&eszs,true,fname,autoname,ignore_no_file);
    if (err)
	return err;

    if (eszs.data)
    {
	raw->data_alloced	= eszs.data_alloced;
	raw->data_size		= eszs.data_size;
	raw->data		= eszs.data;
	eszs.data		= 0;
	raw->fname		= eszs.fname;
	eszs.fname		= 0;
	ResetExtractSZS(&eszs);
    }
    else
    {
	szs_file_t szs; // use 'szs_file_t' for automatic decompression
	InitializeSZS(&szs);
	err = LoadSZS(&szs,fname,true,ignore_no_file,true);
	raw->fform_file	= szs.fform_file;
	raw->data	= szs.data;
	raw->data_size	= szs.size;
	raw->data_alloced = szs.data_alloced;
	raw->fatt	= szs.fatt;
	raw->fname	= szs.fname;
	szs.data	= 0;
	szs.data_alloced= 0;
	szs.fname	= 0;
	ResetSZS(&szs);

	if (err)
	    return err;
    }
// [[analyse-magic]]
    raw->fform = GetByMagicFF(raw->data,raw->data_size,raw->data_size);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    UI-Check			///////////////
///////////////////////////////////////////////////////////////////////////////

const char ui_type_char[UIT__N+1] = "?-ACEFGMOSPRTl";

const ccp ui_type_name[UIT__N+1] =
{
    "?",
    "-",
    "Award",
    "Channel",
    "Event",
    "Font",
    "Globe",
    "MenuMulti",
    "MenuOther",
    "MenuSingle",
    "Present",
    "Race",
    "Title",
    "language",
    0
};

///////////////////////////////////////////////////////////////////////////////

ccp GetUiTypeColor ( const ColorSet_t *colset, ui_type_t type )
{
    if (!colset)
	return "";

    switch(type)
    {
	case UIT_UNDEFINED: return colset->bad;
	case UIT_UNKNOWN:   return colset->warn;
	case UIT_LANGUAGE:  return colset->hint;
	case UIT_FONT:      return colset->mark;
	default:	    return colset->info;
    }
}

///////////////////////////////////////////////////////////////////////////////

static int ui_check_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if (term)
	return 0;

    ui_check_t *uc = it->param;
    DASSERT(uc);

    ccp path = it->path;
    if( path[0] == '.' && path[1] == '/' )
	path += 2;

    if (!strcmp(path,"message/Common.bmg"))
    {
	uc->is_ui = OFFON_OFF;
	uc->is_korean = OFFON_AUTO;
	uc->type = UIT_LANGUAGE;
	return 1;
    }

    if ( uc->is_korean == OFFON_AUTO && strstr(path,"/blyt/national_flag.brlyt") )
    {
	if ( it->size == 232 )
	    uc->is_korean = OFFON_ON;
	else if ( it->size == 16380 )
	    uc->is_korean = OFFON_OFF;
    }

    if ( uc->is_korean == OFFON_AUTO && strstr(path,"/blyt/chara_flag_machine_picture_common.brlyt") )
    {
	if ( it->size == 232 )
	    uc->is_korean = OFFON_ON;
	else if ( it->size == 33120 )
	    uc->is_korean = OFFON_OFF;
    }

    if (!memcmp(path,"title/timg/tt_title_screen_peachi",33))
    {
	uc->type = UIT_TITLE;
	if ( uc->is_korean != OFFON_AUTO )
	    return 1;
    }

    if (!strcmp(path,"indicator_font.brfnt"))
    {
	uc->is_korean = OFFON_OFF;
	uc->type = UIT_FONT;
	return 1;
    }

    if (!strcmp(path,"parameter/mission_ui_single.bin"))
    {
	uc->type = UIT_MENU_SINGLE;
	if ( uc->is_korean != OFFON_AUTO )
	    return 1;
    }

    if (!strcmp(path,"button/ctrl/AfterMenuBT.brctr"))
    {
	uc->type = UIT_RACE;
	return 1;
    }

    if (!strcmp(path,"control/blyt/present_message_window_course.brlyt"))
    {
	uc->type = UIT_PRESENT;
	return 1;
    }

    if (!strcmp(path,"globe/timg/tt_star_01.tpl"))
    {
	uc->type = UIT_GLOBE;
	return 1;
    }

    if (it->is_dir)
    {
	if (!strcmp(path,"award/"))
	    uc->type = UIT_AWARD;
	else if (!strcmp(path,"ranking/ctrl/"))
	{
	    uc->type = UIT_CHANNEL;
	    return 1;
	}
    }

    if (!strcmp(path,"bg/anim/option_bg_Loop.brlan"))
	uc->possible &= UIT_M_CHANNEL | UIT_M_MENU_OTHER;

    if (!strcmp(path,"bg/ctrl/EventObiTop.brctr"))
	uc->possible &= UIT_M_CHANNEL | UIT_M_EVENT;

    if (!strcmp(path,"control/timg/tt_baby_daisy_64x64.tpl"))
	uc->possible &= UIT_M_CUP_ICONS;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void UiCheck ( ui_check_t *uc, szs_file_t *szs )
{
    DASSERT(uc);
    DASSERT(szs);
    memset(uc,0,sizeof(*uc));
    uc->ui_lang		= '-';
    uc->ui_type		= UIT_UNKNOWN;
    uc->type		= UIT_UNKNOWN;
    uc->possible	= UIT_M_ALL;

    IterateFilesParSZS(szs,ui_check_func,uc,false,false,false,-1,-1,SORT_NONE);

    if ( uc->is_korean == OFFON_ON && uc->is_ui == OFFON_AUTO )
	uc->is_ui = OFFON_ON;

    if ( uc->type <= UIT_UNKNOWN )
    {
	if ( uc->possible == (UIT_M_CHANNEL|UIT_M_EVENT) )
	    uc->type = UIT_EVENT;
	if ( uc->possible == (UIT_M_CHANNEL|UIT_M_MENU_OTHER) )
	    uc->type = UIT_MENU_OTHER;
	if ( uc->possible == (UIT_M_CHANNEL|UIT_M_MENU_MULTI|UIT_M_MENU_SINGLE) )
	    uc->type = UIT_MENU_MULTI;

	for ( ui_type_t type = 1; type < UIT__N; type++ )
	{
	    const uint mask = 1 << type;
	    if ( uc->possible == mask )
	    {
		uc->type = type;
		break;
	    }
	}
    }

    if (uc->type)
	uc->possible = 1 << uc->type;

    //-- ui_type & ui_lang

    uc->ui_type = uc->type;
    ccp fname = strrchr(szs->fname,'/');
    if (fname)
    {
	fname++;
	if ( uc->ui_type == UIT_LANGUAGE )
	{
	    uc->ui_type = UIT_UNKNOWN;
	    PRINT0("NAME; %s\n",fname);
	    for ( int idx = UIT__FIRST; idx <= UIT__LAST; idx++ )
	    {
		ccp tab = ui_type_name[idx];
		if (!strncasecmp(fname,tab,strlen(tab)))
		{
		    uc->ui_type = idx;
		    uc->possible = 1 << idx;
		    break;
		}
	    }
	}

	ccp point = strrchr(fname,'.');
	if ( point && point - fname > 3 && point[-2] == '_' )
	{
	    const char ch = point[-1];
	    if ( isalpha(ch))
	    {
		if ( ch == 'R' )
		    uc->is_korean = true;
		else
		    uc->ui_lang = ch;
	    }
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IterateFilesU8()		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesU8
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

    if ( !szs->data || szs->size < sizeof(u8_header_t) )
	return -1;

    const u8_header_t * u8p = (u8_header_t*)szs->data;
    if ( ntohl(u8p->magic) != U8_MAGIC_NUM )
	return -1;

    //PRINT("U8:"); HEXDUMP16(6,0,u8p,sizeof(*u8p));


    //----- setup stack

    const int MAX_DEPTH = 25; // maximum supported directory depth
    typedef struct stack_t
    {
	const u8_node_t * dir_end;
	char * path;
    } stack_t;
    stack_t stack_buf[MAX_DEPTH];
    stack_t *stack = stack_buf;
    stack_t *stack_max = stack_buf + MAX_DEPTH;


    //----- setup fst + path

    const u8_node_t *fst_base	= (u8_node_t*)(szs->data+ntohl(u8p->node_offset));
    const int n_fst		= ntohl(fst_base->size);
    const u8_node_t *fst_end	= fst_base + n_fst;
    const u8_node_t *dir_end	= fst_end;
    char * path_ptr		= it->path;
    char * path_end		= it->path + sizeof(it->path) - MAX_DEPTH;

    TRACE("N=%d\n",n_fst);
    if ( (u8*)fst_end > szs->data + szs->size )
	return -1;
    //HEXDUMP(0,(u8*)fst_base-szs->data,6,sizeof(u8_node_t),fst_base,n_fst*sizeof(u8_node_t));
    //HEXDUMP16(0,(u8*)fst_end-szs->data,fst_end,32);


    //----- cut files?

    if (it->itpar.cut_files)
    {
	it->index	= 0;
	it->fst_item	= 0;
	it->is_dir	= 0;

	it->off		= 0;
	it->size	= sizeof(u8_header_t);
	StringCopyS(it->path,sizeof(it->path),".U8.header");
	it->func_it(it,false);

	it->off		= ntohl(u8p->node_offset);
	it->size	= (u8*)fst_end - (u8*)fst_base;
	StringCopyS(it->path,sizeof(it->path),".U8.file-table");
	it->func_it(it,false);

	uint maxoff = 0;
	const u8_node_t *fst;
	for ( fst = fst_base; fst < fst_end; fst++ )
	{
	    uint nameoff = ntohl(fst->name_off) & 0xffffff;
	    if ( maxoff <= nameoff )
	    {
		ccp fname = (ccp)fst_end + nameoff;
		maxoff = nameoff + strlen(fname) + 1;
	    }
	}

	it->off		= (u8*)fst_end - szs->data;
	it->size	= maxoff;
	StringCopyS(it->path,sizeof(it->path),".U8.string-pool");
	it->func_it(it,false);
    }


    //----- main loop

    int stat = 0;
    const u8_node_t *fst;
    for ( fst = fst_base + !it->itpar.show_root_node; fst < fst_end && !stat; fst++ )
    {
	while ( fst >= dir_end && stack > stack_buf )
	{
	    // leave a directory
	    stack--;
	    it->depth = stack - stack_buf;
	    dir_end = stack->dir_end;
	    path_ptr = stack->path;
	}

	ccp fname = (ccp)fst_end + (ntohl(fst->name_off)&0xffffff);
	it->name = fname;
	char *path_dest = it->trail_path = path_ptr;
	while ( path_dest < path_end && *fname )
	{
	    const char ch = *fname++;
	    if ( ch != '\\' && ch != '/' || !it->itpar.clean_path )
		*path_dest++ = ch;
	}
	if ( it->itpar.clean_path && path_dest == path_ptr+2
			&& path_ptr[0] == '.' && path_ptr[1] == '.' )
	    path_dest--;

	it->index	= fst - fst_base;
	it->fst_item	= (u8_node_t*)fst;
	it->off		= ntohl(fst->offset);
	it->size	= ntohl(fst->size);
	it->is_dir	= fst->is_dir;
	if (fst->is_dir)
	{
	    if ( fst > fst_base )
		*path_dest++ = '/';
	    *path_dest = 0;

	    ASSERT(stack<stack_max);
	    if ( stack < stack_max )
	    {
		stack->dir_end	= dir_end;
		stack->path	= path_ptr;
		stack++;
		it->depth	= stack - stack_buf;
		dir_end		= fst_base + ntohl(fst->size);
		path_ptr	= path_dest;
	    }
	}
	else
	{
	    *path_dest = 0;
	    if ( szs->min_data_off > it->off )
		 szs->min_data_off = it->off;

	    const u32 max = it->off + it->size;
	    if ( szs->max_data_off < max )
		 szs->max_data_off = max;
	}

	stat = it->func_it(it,false);
    }

    it->fst_item = 0;
    it->name = 0;
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IterateFilesLTA()		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesLTA
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

    if ( !szs->data || szs->size < sizeof(lta_header_t) )
	return -1;

    const lta_header_t lta = *(lta_header_t*)szs->data;
    if ( ntoh64(lta.magic) != LTA_MAGIC_NUM )
	return -1;

    // convert to local endian
    be32n( (u32*)&lta, (u32*)&lta, sizeof(lta_header_t)/4 );


    //----- load node list

    const uint node_size = GetNodeListSizeLTA(&lta);
    lta_node_t *node_buf = MALLOC(node_size);
    enumError err = LoadFILE(szs->fname,0,lta.node_off,node_buf,node_size,0,0,false);
    if (err)
	return 1;
    //be32n( (u32*)&node_buf, (u32*)&node_buf, node_size/4 );


    //----- load ext list

    char *ext_buf = 0, *ext_ptr = 0, *ext_end = 0;
    if ( HaveExtLTA(&lta) && lta.ext_size )
    {
	ext_buf = MALLOC(lta.ext_size);
	enumError err = LoadFILE(szs->fname,0,lta.ext_off,ext_buf,lta.ext_size,2,0,false);
	PRINT0("LoadFILE(%s), err=%d\n",szs->fname,err);
	if (!err)
	{
	    ext_ptr = ext_buf;
	    ext_end = ext_buf + lta.ext_size;
	}
    }

    ccp format_szs, format_lfl, format_d_szs, format_d_lfl;
    if ( lecode_04x || lta.base_slot + lta.n_slots >= 0x1000 )
    {
	format_szs   = "%04x.%s";
	format_lfl   = "%04x.lfl";
	format_d_szs = "%04x_d.%s";
	format_d_lfl = "%04x_d.lfl";
    }
    else
    {
	format_szs   = "%03x.%s";
	format_lfl   = "%03x.lfl";
	format_d_szs = "%03x_d.%s";
	format_d_lfl = "%03x_d.lfl";
    }

    //----- main loop

    it->index	= 0;
    it->is_dir	= false;
    it->no_dirs	= true;

    int stat = 0;
    for ( uint rel_slot = 0; rel_slot < lta.n_slots && !stat; rel_slot++ )
    {
	lta_node_t node;
	// convert to local endian
	be32n( (u32*)&node, (u32*)(node_buf+rel_slot), sizeof(node)/4 );
	const uint slot = lta.base_slot + rel_slot;

	if ( node.std.szs_size )
	{
	    it->fix_extension = !( ext_ptr < ext_end && *ext_ptr );
	    ccp ext	= it->fix_extension ? "x" : ext_ptr;
	    snprintf(it->path,sizeof(it->path),format_szs,slot,ext);
	    it->name	= it->path;
	    it->off	= node.std.szs_off;
	    it->size	= node.std.szs_size;
	    stat	= it->func_it(it,false);
	    it->index++;
	}

	it->fix_extension = false;
	if ( ext_ptr < ext_end )
	    ext_ptr += strlen(ext_ptr)+1;

	if ( !stat && node.std.lfl_size )
	{
	    snprintf(it->path,sizeof(it->path),format_lfl,slot);
	    it->name	= it->path;
	    it->off	= node.std.lfl_off;
	    it->size	= node.std.lfl_size;
	    stat	= it->func_it(it,false);
	    it->index++;
	}

	if ( !stat && node.d.szs_size && node.d.szs_off != node.std.szs_off )
	{
	    it->fix_extension = !( ext_ptr < ext_end && *ext_ptr );
	    ccp ext	= it->fix_extension ? "x" : ext_ptr;
	    snprintf(it->path,sizeof(it->path),format_d_szs,slot,ext);
	    it->name	= it->path;
	    it->off	= node.d.szs_off;
	    it->size	= node.d.szs_size;
	    stat	= it->func_it(it,false);
	    it->index++;
	}

	it->fix_extension = false;
	if ( ext_ptr < ext_end )
	    ext_ptr += strlen(ext_ptr)+1;

	if ( !stat && node.d.lfl_size && node.d.lfl_off != node.std.lfl_off )
	{
	    snprintf(it->path,sizeof(it->path),format_d_lfl,slot);
	    it->name	= it->path;
	    it->off	= node.d.lfl_off;
	    it->size	= node.d.lfl_size;
	    stat	= it->func_it(it,false);
	    it->index++;
	}
    }

    if (!stat)
    {
	StringCopyS(it->path,sizeof(it->path),NODE_LIST_FILE);
	it->name = it->path;
	it->off  = lta.node_off;
	it->size = lta.n_slots * sizeof(lta_node_t);
	stat     = it->func_it(it,false);

	if ( !stat && HaveExtLTA(&lta) )
	{
	    StringCopyS(it->path,sizeof(it->path),EXT_LIST_FILE);
	    it->name = it->path;
	    it->off  = lta.ext_off;
	    it->size = lta.ext_size;
	    stat     = it->func_it(it,false);
	}
    }

    // reset param
    FREE(ext_buf);
    FREE(node_buf);
    it->name = 0;
    it->no_dirs	= false;
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

bool DumpLTA
(
    FILE		*f,		// output file
    uint		indent,		// indention
    const ColorSet_t	*col,		// NULL or colorset
    ccp			fname,		// NULL or filename
    const u8		*data,		// data to dump
    uint		size		// size of 'data'
)
{
    DASSERT(f);
    DASSERT(col);
    DASSERT(data||!size);

    if ( !data || size < sizeof(lta_header_t) )
	return false;

    indent = NormalizeIndent(indent);
    if (!col)
	col = GetColorSet0();

    if ( fname && *fname )
	fprintf(f,"%*.s%sInfo dump of LTA:%s%s\n",
		indent,"", col->caption, fname, col->reset );
    else
	fprintf(f,"%*.s%sInfo dump of a LTA file:%s\n",
		indent,"", col->caption, col->reset );

    const lta_header_t *xhead = (lta_header_t*)data;
    lta_header_t head;
    be32n((u32*)&head,(u32*)xhead,sizeof(head)/4);

    indent += 2;
    const unsigned last_slot = head.base_slot + head.n_slots - 1;
    printf(
	"%*s" "Magic:        %.8s\n"
	"%*s" "Version:      %10u\n"
	"%*s" "Header size:  %10u = %#10x\n"
	"%*s" "File size:    %10u = %#10x\n"
	"%*s" "Node offset:  %10u = %#10x\n"
	"%*s" "Node size:    %10u = %#10x\n"
	,indent,"", (char*)&xhead->magic
	,indent,"", head.version
	,indent,"", head.head_size, head.head_size
	,indent,"", head.file_size, head.file_size
	,indent,"", head.node_off,  head.node_off
	,indent,"", head.node_size, head.node_size
	);

    if (HaveExtLTA(&head))
	printf(	"%*s" "Ext offset:   %10u = %#10x\n"
		"%*s" "Ext size:     %10u = %#10x\n"
		,indent,"", head.ext_off,  head.ext_off
		,indent,"", head.ext_size, head.ext_size
		);

    printf(
	"%*s" "Base slot:    %10u = %#10x\n"
	"%*s" "Num of slots: %10u = %#10x\n"
	"%*s" "Last slot:    %10u = %#10x\n"
	,indent,"", head.base_slot, head.base_slot
	,indent,"", head.n_slots,   head.n_slots
	,indent,"", last_slot,      last_slot
	);

    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    LFL interface		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesLFL
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

    if ( !szs->data || szs->size < sizeof(lfl_header_t) )
	return -1;

    const lfl_header_t lfl = *(lfl_header_t*)szs->data;
    be32n( (u32*)&lfl, (u32*)&lfl, sizeof(lfl_header_t)/4 );
    if ( lfl.magic != LFL_MAGIC_NUM )
	return -1;

    uint offset = sizeof(lfl_header_t);


    //----- main loop

    it->index	= 0;
    it->is_dir	= false;
    it->no_dirs	= true;

    int stat = 0;
    while (!stat)
    {
	const lfl_node_t *node = (lfl_node_t*)(szs->data + offset);
	StringCopyS(it->path,sizeof(it->path),node->file_name);
	it->name	= it->path;
	it->off		= offset + ntohl(node->data_offset);
	it->size	= ntohl(node->data_size);
	if (!it->size)
	    break;
	stat		= it->func_it(it,false);
	it->index++;

	offset += ntohl(node->next_offset);
    }

    // reset param
    it->name = 0;
    it->no_dirs	= false;
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

static ccp fix_path_lfl ( ccp path, bool rm_common )
{
    if ( path[0] == '.' || path[1] == '/' )
	path += 2;
    if ( rm_common && !strncasecmp(path,"common/",7) )
	path += 7;
    return path;
}

//-----------------------------------------------------------------------------

enumError CreateLFL
(
    struct szs_file_t	*szs,		// valid szs
    ccp			source_dir,	// path to source dir
    bool		rm_common,	// TRUE: remove leading 'common/' from path
    bool		clear_if_empty	// TRUE: clear data (size 0 bytes) if no sub file added
)
{
    //--- sort

    SortSubFilesSZS(szs,SORT_INAME);


    //--- calculate size

    uint total_size = sizeof(lfl_header_t) + sizeof(lfl_node_t); // header + terminal node
    uint max_size   = 0;

    szs_subfile_t *ptr = szs->subfile.list;
    szs_subfile_t *end = ptr + szs->subfile.used;
    for ( ; ptr < end; ptr++ )
    {
	if (ptr->is_dir)
	    continue;

	ccp path	= fix_path_lfl(ptr->path,rm_common);
	const uint size	= sizeof(lfl_node_t)
			+ ALIGN32(strlen(path)+1,4)
			+ ALIGN32(ptr->size,4);
	if ( max_size < size )
	    max_size = size;
	total_size += size;
    }

    if ( !max_size && clear_if_empty )
    {
	ResetSZS(szs);
	return ERR_NO_SOURCE_FOUND;
    }

    u8 *buf = MALLOC(max_size);


    //--- setup and write header

    u8 *new_data = MALLOC(total_size);
    memset(new_data,0,total_size);

    lfl_header_t *head	= (lfl_header_t*)new_data;
    head->magic		= htonl(LFL_MAGIC_NUM);
    head->version	= htonl(LFL_VERSION);
    head->file_size	= htonl(total_size);


    //--- add files

    uint offset = sizeof(lfl_header_t);
    for ( szs_subfile_t *ptr = szs->subfile.list; ptr < end; ptr++ )
    {
	if (ptr->is_dir)
	    continue;

	ASSERT( offset + sizeof(lfl_node_t) + ptr->size <= total_size );
	PRINT0("offset = %3x\n",offset);

	ccp path		= fix_path_lfl(ptr->path,rm_common);
	lfl_node_t *node	= (lfl_node_t*)(new_data+offset);
	uint node_size		= sizeof(lfl_node_t) + ALIGN32(strlen(path)+1,4);
	node->data_offset	= htonl(node_size);
	node->data_size		= htonl(ptr->size);

	strcpy(node->file_name,path);
	StringLower(node->file_name);
	if (ptr->data)
	    memcpy(new_data+offset+node_size,ptr->data,ptr->size);
	else
	    LoadFILE(source_dir,path,0,new_data+offset+node_size,ptr->size,0,0,false);

	node_size		+= ALIGN32(ptr->size,4);
	node->next_offset	= htonl(node_size);
	ASSERT( offset + node_size <= total_size );
	offset			+= node_size;
    }


    //--- finalize

    FREE(buf);
    AssignSZS(szs,false,new_data,total_size,true,FF_LFL,0);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IterateFilesDOL()		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesDOL
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

    if (!IsDolHeader(szs->data,szs->size,0))
	return FF_DOL;

    it->index	= 0;
    it->off	= 0;
    it->size	= sizeof(dol_header_t);
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),"DOL.header");
    it->func_it(it,false);

    dol_header_t *dh = (dol_header_t*)szs->data;
    int sect, stat = 0;
    for ( sect = 0; sect < DOL_N_SECTIONS && !stat; sect++ )
    {
	const u32 size = ntohl(dh->sect_size[sect]);
	if (size)
	{
	    const u32 off = ntohl(dh->sect_off[sect]);
	    if ( off < szs->size )
	    {
		uint real_size = szs->size - off;
		if ( real_size > size )
		    real_size = size;

		it->index++;
		it->off		= off;
		it->size	= real_size;
		StringCopyS(it->path,sizeof(it->path),GetDolSectionName(sect));
		stat = it->func_it(it,false);
	    }
	}
    }

    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Iterate Files			///////////////
///////////////////////////////////////////////////////////////////////////////

szs_iterator_func GetIteratorFunction
(
    file_format_t	fform,		// file format format
    bool		cut_files	// true: support of cut files
)
{
    PRINT0("GetIteratorFunction(%x=%s)\n",fform,GetNameFF(fform,fform));

    if (!std_iter_func[FF_U8])
	SetupStandardSZS();

    return (uint)fform >= FF_N
		? 0
		: cut_files
			? cut_iter_func[fform]
			: std_iter_func[fform];
}

///////////////////////////////////////////////////////////////////////////////

void FixIteratorExtByFF
(
    szs_iterator_t	*it,		// valid iterator data
    file_format_t	fform_file,	// source file format
    file_format_t	fform_arch	// (decompressed) archive format
)
{
    if ( it && it->fix_extension )
    {
	it->fix_extension = false;
	ccp ext = GetExtFF(fform_file,fform_arch);
	if (!ext)
	    ext = ".bin";

	int plen = strlen(it->path);
	ccp slash = strrchr(it->path,'/');
	ccp point = strrchr(it->path,'.');
	if ( point && ( !slash || point > slash ))
	    plen = point - it->path;
	if ( plen + strlen(ext) < sizeof(it->path) )
	    StringCopyE(it->path+plen,it->path+sizeof(it->path),ext);
    }
}

//-----------------------------------------------------------------------------

void FixIteratorExtByData
(
    szs_iterator_t	*it,		// valid iterator data
    cvp			data,		// pointer to data
    uint		size		// size of data
)
{
    if ( it && it->fix_extension )
    {
	file_format_t ff = GetByMagicFF(data,size,size);
	FixIteratorExtByFF(it,ff,ff);
    }
}

//-----------------------------------------------------------------------------

void FixIteratorExt
(
    szs_iterator_t	*it		// valid iterator data
)
{
    if ( it && it->fix_extension )
    {
	const u8 * data = it->szs->data + it->off;
	const file_format_t fform
		= AnalyseMagicByOpt(0,data,it->size,it->szs->size-it->off,
		    FF_UNKNOWN, it->path );
	FixIteratorExtByFF(it,fform,fform);
    }
}

///////////////////////////////////////////////////////////////////////////////

void * UpdateIteratorFF ( struct szs_iterator_t *it )
{
    if (!it)
	return 0;

// [[analyse-magic+]]
// [[ff_fallback]]
    u8 * data = it->szs->data + it->off;
    it->fform = RepairMagicByOpt( 0, data, it->size, it->szs->size - it->off,
					FF_UNKNOWN, it->path );
    return data;
}

///////////////////////////////////////////////////////////////////////////////

static int iterate_sort_files
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    DASSERT(it->func_sort);
    DASSERT( it->func_sort != iterate_sort_files );

    if (!term)
    {
	if ( it->off > it->szs->size || it->off + it->size > it->szs->size )
	{
	    if ( it->size != M1(it->size) && WARN_MODE & WARN_INVALID_OFFSET && ErrorLogEnabled() )
		ERROR0(ERR_WARNING,
		    "Invalid offset [%x..%x, size=%zx] for subfile.\n"
		    "=> File ignored: %s%s%s\n",
		    it->off, it->off+it->size, it->szs->size,
		    it->szs->fname, *it->szs->fname ? "/" : "",
		    it->path );
	    return 0;
	}

	AppendSubfileSZS(it->szs,it,0);
	return 0;
    }

    SortSubFilesSZS(it->szs,it->itpar.sort_mode);

    const szs_subfile_t * ptr = it->szs->subfile.list;
    const szs_subfile_t * end = ptr + it->szs->subfile.used;
    int stat = 0;
    uint i;
    for ( i = 0; ptr < end && !stat; i++, ptr++ )
    {
	it->index		= i;
	it->is_dir		= ptr->is_dir;
	it->has_subfiles	= ptr->has_subfiles;
	it->fform		= ptr->fform;
	it->name		= ptr->name;
	it->brsub_version	= ptr->brsub_version;
	it->group		= ptr->group;
	it->entry		= ptr->entry;
	it->off			= ptr->offset;
	it->size		= ptr->size;
//	it->device		= ptr->device;
//	it->inode		= ptr->inode;

	if (IS_M1(it->size))
	{
	    it->size = ( ptr+1 < end ? ptr[1].offset : it->szs->size ) - it->off;
	    noPRINT("SIZE==-1 => ptr=%zd/%d => %x+%x=%x | %zx\n",
		ptr-(szs_subfile_t*)it->szs->subfile.list, it->szs->subfile.used,
		it->off, it->size, it->off+it->size, it->szs->size );
	    if ( it->off + it->size > it->szs->size )
		it->size = 0;
	}
	StringCopyS(it->path,sizeof(it->path),ptr->path);
	stat = it->func_sort(it,false);
    }
    ResetFileSZS(it->szs,false);

    *it->path		= 0;
    it->brsub_version	= 0;
    it->group		= GROUP_INVALID;
    it->entry		= ENTRY_INVALID;
    it->name		= 0;
    const int term_stat = it->func_sort(it,true);
    return stat ? stat : term_stat;
}

///////////////////////////////////////////////////////////////////////////////

static int iterate_sub_files
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT( it->func_sub );
    DASSERT( it->func_sub != iterate_sub_files );

    if ( !term && it->no_recurse <= 0 && !it->is_dir )
    {
	if ( it->off > it->szs->size || it->off + it->size > it->szs->size )
	{
	    if ( WARN_MODE & WARN_INVALID_OFFSET && ErrorLogEnabled() )
		ERROR0(ERR_WARNING,
			"Invalid offset [%x..%x, size=%zx] for subfile.\n"
			"=> File ignored: %s%s%s\n",
			it->off, it->off+it->size, it->szs->size,
			it->szs->fname, *it->szs->fname ? "/" : "",
			it->path );
	    return 0;
	}

	UpdateIteratorFF(it);
	const file_format_t fform = it->fform;

	szs_iterator_func it_func = 0;
	if ( it->recurse_level < it->recurse_max )
	{
	    it_func = GetIteratorFunction(fform,it->itpar.cut_files);
	}
	else if ( it->itpar.cut_files )
	{
	    it_func = GetIteratorFunction(fform,true);
	    if (GetIteratorFunction(fform,false))
		it_func = 0;
	}
	PRINT0("it_func=%p\n",it_func);

	if (it_func)
	{
	    it->has_subfiles	= true;
	    const int stat	= it->func_sub(it,false);
	    it->has_subfiles	= false;

	    szs_file_t szs2;
// [[fname+]]
	    InitializeSubSZS(&szs2,it->szs,it->off,it->size,fform,it->path,false);

	    szs_iterator_t it2;
	    memcpy(&it2,it,sizeof(it2));
	    it2.szs		= &szs2;
	    it2.index		= 0;
	    it2.client_int	= 0;
	    it2.parent_it	= it;
	    it2.fform		= fform;
	    it2.recurse_level++;

	    const int stat2 = it_func(&it2,false);
	    it2.name = 0;
	    *it2.path = 0;
	    if ( stat2 != -1 )
		it2.func_it(&it2,true);	// ignore status
	    ResetSZS(&szs2);

	    return stat;
	}
    }
    return it->func_sub(it,term);
}


///////////////////////////////////////////////////////////////////////////////

static int simple_norm_collect_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if (term)
	return 0;

// [[norm]]
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
	    norm->u8.have_pt_dir = true;
	    return 0;
	}
    }

    if (!*path)
	return 0;

    szs_subfile_t * file = AppendSubfileSZS(it->szs,it,0);
    DASSERT(file);
    ccp ptr = file->path + strlen(file->path) - 1;
    if (it->is_dir)
	ptr--;
    while ( ptr > file->path && *ptr != '/' )
	ptr--;
    if ( *ptr == '/' )
	ptr++;

    norm->u8.namepool_size += strlen(ptr);
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
			norm->u8.namepool_size -= strlen(ptr)+1;
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
	    norm->u8.total_size += ALIGN32(relevant_size,opt_align_u8);

	norm->u8.namepool_size++;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// [[IterateFilesSZS]]

int IterateFilesSZS
(
    szs_file_t		*szs,		// valid szs
    szs_iterator_func	func,		// call back function
    void		*param,		// user defined parameter
    const iterator_param_t
			*p_itpar,	// NULL or iteration parameters
    int			recurse		// 0:off, <0:unlimited, >0:max depth
)
{
    DASSERT(szs);
    DASSERT(func);

    iterator_param_t itpar;
    if (p_itpar)
	memcpy(&itpar,p_itpar,sizeof(itpar));
    else
	memset(&itpar,0,sizeof(itpar));

    PRINT("IterateFilesParSZS(ipar=%d,clean=%d,rec=%d,cut=%d,sort=%d) ff=%s\n",
	p_itpar!=0, itpar.clean_path, recurse, itpar.cut_files, itpar.sort_mode,
	GetNameFF(szs->fform_file,szs->fform_arch));

    szs_iterator_func ifunc
	= GetIteratorFunction( szs->fform_arch, itpar.cut_files>=0 );
    if (!ifunc)
	return -1;

    if ( itpar.sort_mode == SORT_NONE && IsBRSUB(szs->fform_arch) )
	itpar.sort_mode = SORT_OFFSET;
    const bool sort_files = itpar.sort_mode != SORT_NONE;
    if ( sort_files && szs->subfile.used )
	ResetFileSZS(szs,false);

    szs_iterator_t it = {0};
    it.szs		= szs;
    it.itpar		= itpar;
    it.func_sub		= func;
    it.func_sort	= recurse || itpar.cut_files ? iterate_sub_files : func;
    it.func_it		= sort_files ? iterate_sort_files : it.func_sort;
    it.param		= param;
    it.endian		= &be_func;
    it.recurse_max	= recurse >= 0 ? recurse : INT_MAX;
    it.group		= GROUP_INVALID;
    it.entry		= ENTRY_INVALID;

    it.job_ui_check	= itpar.job_ui_check;
    if (itpar.job_ui_check)
	UiCheck (&it.ui_check,szs);

// [[lta]] [[2do]]
// [[lfl]] [[2do]]
    if ( itpar.job_norm_create && ( szs->fform_arch == FF_U8 || szs->fform_arch == FF_WU8 ))
    {
	ResetFileSZS(szs,false);
	// [[norm]]
	IterateFilesParSZS(szs,simple_norm_collect_func,&it.norm_create,false,false,false,0,-1,SORT_NONE);
    }

    const int stat = ifunc(&it,false);
    it.name = 0;
    *it.path = 0;
    const int stat_term = it.func_it(&it,true);

    if (it.job_create)
    {
	SortSubFilesSZS(szs,SORT_AUTO);

	//uint old_size		= szs->size;
	u8 * old_data		= szs->data;
	bool old_alloced	= szs->data_alloced;
	szs->data		= 0;
	szs->data_alloced	= false;

	CreateU8ByInfo(szs,0,old_data,&it.norm_create.u8);

	//it.log_created = old_size != szs->size || memcmp(old_data,szs->data,old_size);
	if (old_alloced)
	    FREE(old_data);
    }

    return stat ? stat : stat_term;
}

///////////////////////////////////////////////////////////////////////////////
// [[IterateFilesParSZS]]

int IterateFilesParSZS
(
    szs_file_t		*szs,		// valid szs
    szs_iterator_func	func,		// call back function
    void		*param,		// user defined parameter
    bool		clean_path,	// true: clean path from ../ and more
    bool		show_root_node,	// true: include root node in iteration
    bool		job_ui_check,	// true: call UiCheck() before main iteration
    int			recurse,	// 0:off, <0:unlimited, >0:max depth
    int			cut_files,	// <0:never, =0:auto(first level), >0:always
    SortMode_t		sort_mode	// sort mode
)
{
    DASSERT(szs);
    DASSERT(func);

    iterator_param_t itpar =
    {
	.cut_files	= cut_files > 0,
	.sort_mode	= sort_mode,
	.clean_path	= clean_path,
	.show_root_node	= show_root_node,
	.job_ui_check	= job_ui_check,
    };
    return IterateFilesSZS(szs,func,param,&itpar,recurse);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define MAX_COLLECT_RECURSE 100

typedef struct collect_recurse_t
{
    szs_file_t	* szs;				// base szs: collect ehre

    uint	level;				// file depth level

    char	path[PATH_MAX];			// path
    char	* path_ptr[MAX_COLLECT_RECURSE+1]; // save path pointers
    char	* path_end;			// current end of path
    ccp		last_path;			// pointer to last source path

    u32		off[MAX_COLLECT_RECURSE+1];	// save 'add_off' values
    u32		add_off;			// add this to get abpsoulte offset
    u32		last_off;			// pointer to last source offset

} collect_recurse_t;

///////////////////////////////////////////////////////////////////////////////

static int collect_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    if (!term)
	AppendSubfileSZS(it->szs,it,0);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int collect_common_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    if (!term)
    {
	ccp path = it->path;
	if ( path[0] == '.' && path[1] == '/' )
	    path += 2;

	if (!strncasecmp(path,"common/",7))
	    AppendSubfileSZS(it->szs,it,0);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int collect_r_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    szs_file_t *szs = it->szs;

    collect_recurse_t *cr = it->param;
    DASSERT(cr);

    if (term)
    {
	DASSERT( cr->level >= 0 );
	if ( cr->level > 0 && --cr->level < MAX_COLLECT_RECURSE )
	{
	    cr->path_end = cr->path_ptr[cr->level];
	    cr->add_off  = cr->off[cr->level];
	}
    }
    else
    {
	if (!it->client_int++)
	{
	    if ( cr->level < MAX_COLLECT_RECURSE )
	    {
		cr->path_ptr[cr->level] = cr->path_end;
		char *dest = cr->path_end + strlen(cr->path_end);
		if ( dest > cr->path && dest < cr->path + sizeof(cr->path) - 3 )
		{
		    *dest++ = '/';
		    *dest++ = '/';
		}
		*dest = 0;
		cr->path_end = dest;

		cr->off[cr->level] = cr->add_off;
		cr->add_off = cr->last_off;
	    }
	    cr->level++;
	}

	StringCopyE(cr->path_end,cr->path+sizeof(cr->path),it->path);
	cr->last_off = it->off + cr->add_off;

	const int path_len = strlen8(it->path);
	if (it->is_dir)
	{
	    szs->n_dirs++;
	    if ( szs->fw_dirs < path_len )
		 szs->fw_dirs = path_len;
	}
	else
	{
	    szs->n_files++;
	    if ( szs->fw_files < path_len )
		 szs->fw_files = path_len;
	}

	szs_subfile_t * file = AppendSubfileSZS(cr->szs,it,STRDUP(cr->path));
	DASSERT(file);
	file->offset = cr->last_off;

	noPRINT("%2u %c%c %6x %6x %s\n",
		cr->level, file->is_dir ? 'd' : '-', file->has_subfiles ? 's' : '-',
		file->offset, file->size, file->path );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint CollectFilesSZS
(
    szs_file_t		* szs,		// valid szs
    bool		clear,		// true: clear list before adding
    int			recurse,	// 0:off, <0:unlimited, >0:max depth
    int			cut_files,	// <0:never, =0:auto, >0:always
    SortMode_t		sort_mode	// sort subfiles
)
{
    DASSERT(szs);
    if ( clear && szs->subfile.used )
	ResetFileSZS(szs,false);
    const uint base = szs->subfile.used;
    DecompressSZS(szs,true,0);

    szs->n_dirs		= 0;
    szs->n_files	= 0;
    szs->fw_dirs	= 0;
    szs->fw_files	= 0;

    if ( recurse < 0 || recurse > MAX_COLLECT_RECURSE )
	recurse = MAX_COLLECT_RECURSE;
    if ( recurse > 0 || cut_files >= 0 )
    {
	collect_recurse_t cr;
	memset(&cr,0,sizeof(cr));
	cr.szs = szs;
	cr.path_end = cr.path;
	cr.last_path = EmptyString;
	IterateFilesParSZS(szs,collect_r_func,&cr,false,false,false,recurse,cut_files,SORT_NONE);
    }
    else
	IterateFilesParSZS(szs,collect_func,0,false,false,false,0,-1,SORT_NONE);

    SortSubFilesSZS(szs,sort_mode);
    noPRINT("%d/%d files collected.\n", szs->subfile.used - base, szs->subfile.used );
    return szs->subfile.used - base;
}

///////////////////////////////////////////////////////////////////////////////

uint CollectCommonFilesSZS
(
    szs_file_t		* szs,		// valid szs
    bool		clear,		// true: clear list before adding
    SortMode_t		sort_mode	// sort subfiles
)
{
    DASSERT(szs);
    if ( clear && szs->subfile.used )
	ResetFileSZS(szs,false);
    const uint base = szs->subfile.used;
    DecompressSZS(szs,true,0);

    szs->n_dirs		= 0;
    szs->n_files	= 0;
    szs->fw_dirs	= 0;
    szs->fw_files	= 0;

    IterateFilesParSZS(szs,collect_common_func,0,false,false,false,0,-1,SORT_NONE);

    SortSubFilesSZS(szs,sort_mode);
    return szs->subfile.used - base;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesData
(
    const void		*data,		// valid data pointer
    uint		data_size,	// size of data
    szs_iterator_func	func,		// call back function
    void		* param,	// user defined parameter
    file_format_t	fform,		// not FF_UNKNOWN: use this type
    ccp			fname		// NULL or filename
)
{
    DASSERT(data);
    DASSERT(func);

    szs_file_t szs;
    AssignSZS(&szs,true,(u8*)data,data_size,false,fform,fname);
    const int stat
	= IterateFilesParSZS ( &szs, func, param, false, true, false, -1, 1, SORT_OFFSET );
    ResetSZS(&szs);
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		      FindFileSZS()			///////////////
///////////////////////////////////////////////////////////////////////////////

struct find_subfile_t
{
    ccp path;
    int status;
    szs_iterator_t result;
};

///////////////////////////////////////////////////////////////////////////////

int find_subfile_iter
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if (!term)
    {
	struct find_subfile_t *fsub = it->param;
	DASSERT(fsub);

	ccp path = it->path;
	if ( path[0] == '.' && path[1] == '/' )
	    path += 2;
	if (!strcmp(path,fsub->path))
	{
	    fsub->status = 1;
	    fsub->result = *it;
	    return 1;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int FindFileSZS
(
    // -1: error, 0: not found + result cleared, 1: found + result set
    szs_file_t		*szs,		// valid szs
    ccp			path,		// path to find
    const iterator_param_t
			*p_itpar,	// NULL or iteration parameters
    int			recurse,	// 0:off, <0:unlimited, >0:max depth
    szs_iterator_t	*result		// not NULL: store result here
)
{
    if ( !szs || !path )
	goto err;

    while ( path[0] == '.' && path[1] == '/' )
	path += 2;
    if (!*path)
	goto err;

    struct find_subfile_t fsub = {0};
    fsub.path = path;

    int stat = IterateFilesSZS(szs,find_subfile_iter,&fsub,p_itpar,recurse);
    DASSERT(szs);
    if ( stat >= 0 )
    {
	if (result)
	{
	    if ( fsub.status > 0 )
		memcpy(result,&fsub.result,sizeof(*result));
	    else
		memset(result,0,sizeof(*result));
	}
	if ( fsub.status >= 0 )
	    return fsub.status;
    }

 err:
    if (result)
	memset(result,0,sizeof(*result));
    return -1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		      cut files: BRSUB			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool find_next_brsub_group
(
    brsub_cut_t	* bcut		// valid data
)
{
    DASSERT(bcut);
    DASSERT(bcut->bh);
    DASSERT(bcut->endian);

    const uint last_off  = bcut->found_off;
    bcut->found_off = bcut->brsub_size;
    const int last_grp = bcut->found_grp;
    bcut->found_grp = -1;

    int grp;
    for ( grp = 0; grp < bcut->n_grp; grp++ )
    {
	const uint off = bcut->endian->rd32(bcut->bh->grp_offset+grp);
	//if ( off > last_off && off < bcut->found_off )
	if ( ( off > last_off || off > 0 && off == last_off && grp > last_grp )
		&& off < bcut->found_off )
	{
	    bcut->found_grp = grp;
	    bcut->found_off = off;
	}
    }
    return bcut->found_grp >= 0;
}

//-----------------------------------------------------------------------------
// [[CutFilesBRSUB]]

#undef GETSTR
#define GETSTR(n,d) ( n >= bcut.brsub_size && n < bcut.file_size ? (ccp)bcut.data+n : d )

int CutFilesBRSUB
(
    szs_iterator_t	*it,		// iterator struct with all infos
    brsub_cut_func_t	func,		// call back function
    void		*param,		// user parameter
    ccp			sname_tab[],	// NULL or section name table
    uint		n_sname		// number of elements in 'sname_tab'
					// if 0: 'sname_tab' is terminated by NULL
)
{
    DASSERT(it);
    DASSERT(it->endian);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 * data = szs->data;
    DASSERT(data);
    DASSERT(func);

    if ( !szs->data || szs->size <= sizeof(brsub_header_t) )
	return -1;

    const int n_grp = GetSectionNumBRSUB(data,szs->size,it->endian);
    if ( n_grp <= 0 )
	return -1;
    const uint brsub_header_size = sizeof(brsub_header_t) + 4*n_grp + 4;

    brsub_cut_t bcut;
    memset(&bcut,0,sizeof(bcut));
    bcut.bh		= (brsub_header_t*)data;
    bcut.it		= it;
    bcut.szs		= szs;
    bcut.data		= data;
    bcut.size		= szs->size;
    bcut.file_size	= szs->file_size;
// [[analyse-magic]]
    bcut.fform		= GetByMagicFF(data,szs->size,szs->size);
    bcut.version	= it->endian->rd32(&bcut.bh->version);
    bcut.fattrib	= GetAttribFF(bcut.fform);
    bcut.n_grp		= n_grp;
    bcut.endian		= it->endian;
    bcut.param		= param;

    int stat		= 0;
    it->brsub_version	= bcut.version;
    it->group		= GROUP_INVALID;
    it->entry		= ENTRY_INVALID;

    PrintID(bcut.bh->magic,sizeof(bcut.id)-1,bcut.id);
    bcut.brsub_size = szs->size;
    uint off = be32(&bcut.bh->size);
    if ( off > sizeof(*bcut.bh) && off < bcut.brsub_size )
	bcut.brsub_size = off;

    it->no_recurse++;
    it->index	= 0;
    it->off	= 0;
    it->size	= brsub_header_size;
    it->is_dir	= 0;
    PrintHeaderNameBRSUB(it->path,sizeof(it->path),
				(brsub_header_t*)data,n_grp,it->endian);
    u32 name_off = bcut.endian->rd32(data+brsub_header_size-4);
    ccp name_str = GETSTR(name_off,0);
    if (it->itpar.cut_files)
    {
	it->name = name_str;
	stat = func(&bcut,GROUP_IDX_BRSUB_HEADER,ENTRY_IDX_ETC);
	if (stat)
	    goto abort;
    }

    if ( sname_tab && !n_sname )
	while ( sname_tab[n_sname] )
	    n_sname++;

    find_next_brsub_group(&bcut);
    if ( bcut.found_grp >= 0 && bcut.found_off > it->size )
    {
	it->is_dir	= 0;
	it->off		= it->size;
	it->size	= bcut.found_off - it->off;
	bcut.path_end = it->path
			+ snprintf(it->path,sizeof(it->path),"%s.header.bin",bcut.id);
	stat = func(&bcut,GROUP_IDX_SUB_HEADER,ENTRY_IDX_DATA);
	if (stat)
	    goto abort;
    }

    while ( bcut.found_grp >= 0 )
    {
	int  cur_grp = bcut.found_grp;
	uint cur_off = bcut.found_off;
	find_next_brsub_group(&bcut);

	it->is_dir	= 0;
	it->off		= cur_off;

	ccp sname = cur_grp < n_sname ? sname_tab[cur_grp] : 0;
	if (!sname)
	    sname = "";

	if ( bcut.fattrib & FFT_BRSUB2 )
	{
	    bcut.gptr			= (brres_group_t*)( bcut.data + it->off );
	    const u32 bg_size		= bcut.endian->rd32(&bcut.gptr->size);
	    const u32 bg_n_entry	= bcut.endian->rd32(&bcut.gptr->n_entries);

	    const u32 x_size = sizeof(brres_group_t) + (bg_n_entry+1)*sizeof(bg_n_entry);
	    if ( x_size > bg_size )
		goto plain;

	    if (it->itpar.cut_files)
	    {
		bcut.path_end = it->path
		    + snprintf(it->path,sizeof(it->path),".%s.s%0*u%s.header.n%u.bin",
			bcut.id, n_grp > 10 ? 2 : 1, cur_grp, sname, bg_n_entry );
		it->size = bg_size;
		it->name = 0;
		stat = func(&bcut,cur_grp,ENTRY_IDX_GROUP);
		if (stat)
		    goto abort;
	    }

	    bcut.eptr = bcut.gptr->entry + 1;
	    int cur_entry;
	    const u8* max_eptr = bcut.data + bcut.size - sizeof(*bcut.eptr);
	    for ( cur_entry = 0; cur_entry < bg_n_entry; cur_entry++, bcut.eptr++ )
	    {
		if ( (u8*)bcut.eptr > max_eptr )
		{
		    if ( WARN_MODE & WARN_INVALID_OFFSET && ErrorLogEnabled() )
			ERROR0(ERR_WARNING,
				"Invalid offset for BRSUB file => %u of %u entries ignored: %s%s%s\n",
				bg_n_entry - cur_entry, bg_n_entry,
				it->szs->fname, *it->szs->fname ? "/" : "", it->path );
		    break;
		}
		u32 data_off = bcut.endian->rd32(&bcut.eptr->data_off);
		u32 name_off = bcut.endian->rd32(&bcut.eptr->name_off);
		ccp name_ptr = GETSTR(name_off+((u8*)bcut.gptr-data),"");
		{
		    ccp ptr;
		    for ( ptr = name_ptr; *ptr; ptr++ )
			if ( ptr-name_ptr > 100 || *(uchar*)ptr >= 0x7f )
			{
			    PRINT("!!!!!!!!!! CutFilesBRSUB() => |%.30s|\n",name_ptr);
			    name_ptr = ""; // [[2do]] should never happen
			    break;
			}
		}
		const u8 *dptr = (u8*)bcut.gptr + data_off;

		it->off  = dptr - bcut.data;
	     #ifdef TEST0 // [[2do]]
		it->size = 1;
	     #else
		it->size = M1(it->size);
	     #endif
		if ( it->off <= bcut.brsub_size - 4 )
		{
		    const u32 len = bcut.endian->rd32(data+it->off);
		    if ( !(len&3) && it->off + len <= bcut.brsub_size )
			it->size = len;
		}

		bcut.path_end = it->path
			+ snprintf(it->path,sizeof(it->path),"%s.s%0*u_%0*u%s.%s.bin",
				bcut.id,
				n_grp > 10 ? 2 : 1, cur_grp,
				bg_n_entry > 10 ? 2 : 1, cur_entry,
				sname, name_ptr );
		it->name = name_ptr;
		stat = func(&bcut,cur_grp,cur_entry);
		if (stat)
		    goto abort;
	    }
	    bcut.gptr = 0;
	    bcut.eptr = 0;
	}
	else
	{
	 plain:
 #if 1 // [[2do]]
	    it->size = bcut.found_off - cur_off;
 #else
	    it->size = bcut.found_off > cur_off
			? bcut.found_off - cur_off
			: bcut.size - cur_off;
 #endif
	    noPRINT("--> %x + %x = %x <= %x\n",
		it->off,it->size,it->off+it->size,bcut.brsub_size);
	    if (it->size)
	    {
		bcut.path_end = it->path
			     + snprintf(it->path,sizeof(it->path),"%s.s%0*u%s.data.bin",
					    bcut.id, n_grp > 10 ? 2 : 1, cur_grp, sname );
		it->name = 0;
		stat = func(&bcut,cur_grp,ENTRY_IDX_DATA);
		if (stat)
		    goto abort;
	    }
	}
    }

    if ( it->itpar.cut_files && bcut.brsub_size < bcut.size )
    {
	it->off	 = bcut.brsub_size;
	it->size = bcut.size - bcut.brsub_size;
	it->name = 0;
	snprintf(it->path,sizeof(it->path),".%s.string-pool.bin",bcut.id);
	int stat = func(&bcut,GROUP_IDX_STRING_POOL,ENTRY_IDX_ETC);
	if (stat)
	    return stat;
    }

 abort:
    it->brsub_version	= 0;
    it->group		= GROUP_INVALID;
    it->entry		= ENTRY_INVALID;
    it->name		= 0;
    return stat;
}

#undef GETSTR

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesFuncBRSUB
(
    brsub_cut_t		*bcut,		// pointer to data structure
    int			grp,		// index of group, <0: grp_entry_t
    int			entry		// index of entry, -1: no entry
)
{
    DASSERT(bcut);
    szs_iterator_t *it = bcut->it;
    DASSERT(it);
    DASSERT(it->func_it);

    it->group		= grp;
    it->entry		= entry;
    const int stat = it->func_it(it,false);
    it->index++;
    return stat;
}

//-----------------------------------------------------------------------------

int IterateFilesBRSUB
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    return CutFilesBRSUB(it,IterateFilesFuncBRSUB,0,0,0);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs_subfile_list_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeSubfileList ( szs_subfile_list_t * sl )
{
    DASSERT(sl);
    memset(sl,0,sizeof(*sl));
}

///////////////////////////////////////////////////////////////////////////////

void ResetSubfileList ( szs_subfile_list_t * sl )
{
    DASSERT(sl);
    DASSERT( sl->used <= sl->size );
    DASSERT( !sl->size == !sl->list );

    PurgeSubfileList(sl);
    FREE(sl->list);
    InitializeSubfileList(sl);
}

///////////////////////////////////////////////////////////////////////////////

void PurgeSubfileList ( szs_subfile_list_t * sl )
{
    DASSERT(sl);
    DASSERT( sl->used <= sl->size );
    DASSERT( !sl->size == !sl->list );

    uint i;
    for ( i = 0; i < sl->used; i++ )
    {
	szs_subfile_t *sf = sl->list + i;
	if ( sf->data_alloced && sf->data )
	    FREE(sf->data);
	FreeString(sf->path);
	FreeString(sf->load_path);
    }
    sl->used = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

szs_subfile_t * InsertSubFileList
(
    szs_subfile_list_t	* sl,		// valid subfile list
    uint		insert_pos,	// insert position index, robust
    ccp			path,		// NULL of path name
    bool		move_path	// ignored if 'path' is NULL
					// false: make a copy of path
					// true:  use path and free it on reset
)
{
    DASSERT(sl);
    DASSERT( sl->used <= sl->size );
    DASSERT( !sl->size == !sl->list );

    if ( sl->used == sl->size )
    {
	sl->size += sl->size/4 + 1000;
	sl->list = REALLOC(sl->list,sl->size*sizeof(*sl->list));
	TRACE("REALLOC(sl->list): %u (size=%zu) -> %p\n",
		sl->size,sl->size*sizeof(*sl->list),sl->list);
    }

    sl->sort_mode = SORT_NONE;

    szs_subfile_t * file;
    if ( insert_pos < sl->used )
    {
	// insert
	memmove( sl->list + insert_pos + 1,
		 sl->list + insert_pos,
		 ( sl->used - insert_pos ) * sizeof(*sl->list) );
	sl->used++;
	file = sl->list + insert_pos;
    }
    else // append
	file = sl->list + sl->used++;

    memset(file,0,sizeof(*file));
    if (path)
	file->path = move_path ? path : STRDUP(path);
    return file;
}

///////////////////////////////////////////////////////////////////////////////

szs_subfile_t * InsertSubfileSZS
(
    szs_file_t		*szs,		// valid data structure
    uint		insert_pos,	// insert position index, robust
    szs_iterator_t	*it,		// not NULL: copy source info
    ccp			path		// not NULL: copy to member 'path'
)
{
    DASSERT(szs);
    szs_subfile_t *file = InsertSubFileList(&szs->subfile,insert_pos,0,0);
    DASSERT(file);
    if (it)
    {
	file->is_dir		= it->is_dir;
	file->has_subfiles	= it->has_subfiles;
	file->fform		= it->fform;
	file->offset		= it->off;
	file->size		= it->size;
	file->path		= path ? path : STRDUP(it->path);
	file->name		= it->name;
	file->brsub_version	= it->brsub_version;
	file->group		= it->group;
	file->entry		= it->entry;

	if ( it->off && szs->data )
	    file->data = szs->data + it->off;
    }
    else
	file->path = path;
    return file;
}

///////////////////////////////////////////////////////////////////////////////

szs_subfile_t * FindSubFileSZS
(
    szs_subfile_list_t	* sl,		// valid subfile list
    ccp			path,		// path name to search
    uint		start_index	// start seearch at this index
)
{
    DASSERT(sl);
    DASSERT( sl->used <= sl->size );
    DASSERT( !sl->size == !sl->list );

    if ( sl->used > start_index && path )
    {
	szs_subfile_t *ptr, *end = sl->list + sl->used;
	for ( ptr = sl->list + start_index; ptr < end; ptr++ )
	    if (!strcmp(path,ptr->path))
		return ptr;
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sort sub files			///////////////
///////////////////////////////////////////////////////////////////////////////

enum			// sort order in BRRES files
{
    SORTVAL_3DMODELS,
    SORTVAL_TEXTURES,
    SORTVAL_ANMCHR,
    SORTVAL_ANMCLR,
    SORTVAL_ANMTEXPAT,
    SORTVAL_ANMTEXSRT,
    SORTVAL_ANMSCN,

    SORTVAL_ANMSHP,	// only known: SORTVAL_ANMSHP > SORTVAL_ANMCHR

    SORTVAL_OTHER
};

//-----------------------------------------------------------------------------

static int get_brres_sort_value ( ccp name )
{
    switch (name[0])
    {
	case '3':
	    return !memcmp(name,"3DModels(NW4R)/",15)
			? SORTVAL_3DMODELS
			: SORTVAL_OTHER;

	case 'T':
	    return !memcmp(name,"Textures(NW4R)/",15)
			? SORTVAL_TEXTURES
			: SORTVAL_OTHER;

	case 'A':
	    switch(name[3])
	    {
		case 'C':
		    return !memcmp(name,"AnmChr(NW4R)/",13)
				? SORTVAL_ANMCHR
				: !memcmp(name,"AnmClr(NW4R)/",13)
					? SORTVAL_ANMCLR
					: SORTVAL_OTHER;

		case 'T':
		    return !memcmp(name,"AnmTexPat(NW4R)/",16)
				? SORTVAL_ANMTEXPAT
				: !memcmp(name,"AnmTexSrt(NW4R)/",16)
					? SORTVAL_ANMTEXSRT
					: SORTVAL_OTHER;

		case 'S':
		    return !memcmp(name,"AnmScn(NW4R)/",13)
				? SORTVAL_ANMSCN
				: !memcmp(name,"AnmShp(NW4R)/",13)
					? SORTVAL_ANMSCN
					: SORTVAL_OTHER;
	    }
    }

    return SORTVAL_OTHER;
}

///////////////////////////////////////////////////////////////////////////////
#ifdef TEST
///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_iname ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    return PathCmp(a->path,b->path,2);
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_u8 ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    return NintendoCmp(a->path,b->path);
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_brres ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    const int aval = get_brres_sort_value(a->path);
    const int bval = get_brres_sort_value(b->path);
    if ( aval != bval )
	return aval < bval ? -1 : +1;

    if ( !a->is_dir && !b->is_dir && aval == SORTVAL_3DMODELS )
    {
	const uint alen = strlen(a->path);
	const uint blen = strlen(b->path);
	if ( alen < blen )
	{
	    if (!memcmp(a->path,b->path,alen))
		return +1;
	}
	else if ( alen > blen )
	{
	    if (!memcmp(a->path,b->path,blen))
		return -1;
	}
    }

    return NintendoCmp(a->path,b->path);
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_offset ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    if ( a->offset < b->offset ) return -1;
    if ( a->offset > b->offset ) return +1;

    if ( a->size < b->size ) return -1;
    if ( a->size > b->size ) return +1;

    if ( a->is_dir != b->is_dir ) return a->is_dir ? -1 : 1;

    if ( a->group < b->group ) return -1;
    if ( a->group > b->group ) return +1;

    if ( a->entry < b->entry ) return -1;
    if ( a->entry > b->entry ) return +1;

    return PathCmp(a->path,b->path,2);
}

///////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_iname ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    const int stat = strcasecmp(a->path,b->path);
    return stat ? stat : strcmp(a->path,b->path);
}

static int cmp_subfile_u8 ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    // try to sort in a nintendo like way
    //  1.) ignoring case but sort carefully directories
    //  2.) files before sub directories

    static char transform[0x100] = {0};
    if (!transform[1]) // ==> setup needed once!
    {
	// define some special characters

	uint index = 1;
	transform[(u8)'/'] = index++;
	transform[(u8)'.'] = index++;

	// define digits

	uint i;
	for ( i = '0'; i <= '9'; i++ )
	    transform[i] = index++;

	// define letters

	for ( i = 'A'; i <= 'Z'; i++ )
	    transform[i] = transform[i-'A'+'a'] = index++;

	// define all other

	for ( i = 1; i < sizeof(transform); i++ )
	    if (!transform[i])
		transform[i] = index++;

	DASSERT( index <= sizeof(transform) );
	//HexDump16(stderr,0,0,transform,sizeof(transform));
    }

    //--- setup

    const u8 * ap		= (u8*)a->path;
    const u8 * bp		= (u8*)b->path;
    const u8 * a_last_slash	= (u8*)strrchr(a->path,'/');
    const u8 * b_last_slash	= (u8*)strrchr(b->path,'/');
    const uint a_dir_len	= a_last_slash ? a_last_slash-ap+1 : 0;
    const uint b_dir_len	= b_last_slash ? b_last_slash-bp+1 : 0;
    const uint min_dir_len	= a_dir_len < b_dir_len ? a_dir_len : b_dir_len;
    const u8 * a_end_dir_cmp	= ap + min_dir_len;

    //--- first compare directory part

    while ( ap < a_end_dir_cmp )
    {
	if ( transform[*ap] != transform[*bp] )
	{
	    const int stat = (int)(transform[*ap]) - (int)(transform[*bp]);
	    return stat;
	}
	ap++, bp++;
    }

    if ( a_dir_len != b_dir_len )
	return a_dir_len - b_dir_len;

    int stat = strncmp(a->path,b->path,min_dir_len);
    if (stat)
	return stat;


    //--- and now compare the file part

    while ( *ap || *bp )
    {
	if ( transform[*ap] != transform[*bp] )
	{
	    const int stat = (int)(transform[*ap]) - (int)(transform[*bp]);
	    return stat;
	}
	ap++, bp++;
    }

    return strcmp(a->path,b->path);
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_brres ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    const int aval = get_brres_sort_value(a->path);
    const int bval = get_brres_sort_value(b->path);
    if ( aval != bval )
	return aval < bval ? -1 : +1;

    if ( !a->is_dir && !b->is_dir && aval == SORTVAL_3DMODELS )
    {
	const uint alen = strlen(a->path);
	const uint blen = strlen(b->path);
	if ( alen < blen )
	{
	    if (!memcmp(a->path,b->path,alen))
		return +1;
	}
	else if ( alen > blen )
	{
	    if (!memcmp(a->path,b->path,blen))
		return -1;
	}
    }

    return cmp_subfile_u8(a,b);
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_offset ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    if ( a->offset < b->offset ) return -1;
    if ( a->offset > b->offset ) return +1;

    if ( a->size < b->size ) return -1;
    if ( a->size > b->size ) return +1;

    if ( a->is_dir != b->is_dir ) return a->is_dir ? -1 : 1;

    if ( a->group < b->group ) return -1;
    if ( a->group > b->group ) return +1;

    if ( a->entry < b->entry ) return -1;
    if ( a->entry > b->entry ) return +1;

    return strcmp(a->path,b->path);
}

///////////////////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int cmp_subfile_pack ( const szs_subfile_t * a, const szs_subfile_t * b )
{
    ccp sa = strrchr(a->path,'/');
    if (!sa)
	sa = a->path;
    ccp pa = strrchr(sa,'.');

    ccp sb = strrchr(b->path,'/');
    if (!sb)
	sb = b->path;
    ccp pb = strrchr(sb,'.');

    int stat = strcmp( pa ? pa : EmptyString, pb ? pb : EmptyString );
    return stat ? stat : PathCmp(a->path,b->path,2);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CmpFuncSubFile GetCmpFunc
(
    SortMode_t		sort_mode,	// wanted sort mode
    file_format_t	fform		// use it if 'sort_mode==SORT_AUTO'
)
{
    switch (GetSortMode(sort_mode,fform,SORT_INAME))
    {
	case SORT_U8:
	    return cmp_subfile_u8;

	case SORT_BRRES:
	    return cmp_subfile_brres;

	case SORT_PACK:
	    return cmp_subfile_pack;

	case SORT_OFFSET:
	case SORT_SIZE:
	    return cmp_subfile_offset;

	//case SORT_BREFF:
	default:
	    return cmp_subfile_iname;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool SortSubFiles
(
    // returns true if new sorting done

    szs_subfile_list_t	* sl,		// valid subfile list
    SortMode_t		sort_mode,	// wanted sort mode
    file_format_t	fform,		// use it if sort_mode==SORT_AUTO
    FormatField_t	* order_list	// NULL or a priority sort list
)
{
    DASSERT(sl);
    DASSERT( sl->used <= sl->size );
    DASSERT( !sl->size == !sl->list );

    sort_mode = GetSortMode(sort_mode,fform,SORT_INAME);
    if ( sl->sort_mode != sort_mode && sort_mode != SORT_NONE )
    {
	sl->sort_mode = sort_mode;
	uint n = sl->used;
	if ( n > 1 )
	{
	    szs_subfile_t * list = sl->list;
	    if ( order_list && order_list->used > 0 )
	    {
		uint count = 0;
		FormatFieldItem_t *ptr = order_list->list, *end;
		for ( end = ptr + order_list->used; ptr < end; ptr++ )
		{
		    ccp fname = ptr->key;
		    if ( *fname == '.' && fname[1] == '/' )
			fname += 2;
		    szs_subfile_t *found = FindSubFileSZS(sl,fname,count);
		    if (found)
		    {
			noPRINT(" %s <- %s\n",list->path, found->path);
			szs_subfile_t temp;
			memcpy(&temp,list,sizeof(temp));
			memcpy(list,found,sizeof(*list));
			memcpy(found,&temp,sizeof(*found));
			count++;
			list++;
			n--;
		    }
		}
	    }

	    if ( n > 1 )
		qsort( list, n, sizeof(*sl->list),
			    (qsort_func)GetCmpFunc(sort_mode,0) );
	    return true;
	}
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool SortSubFilesSZS
(
    szs_file_t		* szs,		// valid szs structure
    SortMode_t		sort_mode	// wanted sort mode
)
{
    DASSERT(szs);
    return SortSubFiles( &szs->subfile, sort_mode,
				szs->fform_arch, szs->order_list );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   DiffSZS()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DiffSZS
(
    szs_file_t	* szs1,		// first szs to compare
    szs_file_t	* szs2,		// second szs to compare
    int		recurse,	// 0:off, <0:unlimited, >0:max depth
    int		cut_files,	// <0:never, =0:auto, >0:always
    bool	quiet		// true: be quiet
)
{
    PRINT("DiffSZS(%p,%p,%d,%d,%d)\n",szs1,szs2,recurse,cut_files,quiet);
    DASSERT(szs1);
    DASSERT(szs2);

    if (!szs1->subfile.used)
	CollectFilesSZS(szs1,false,recurse,cut_files,SORT_INAME);
    if (!szs2->subfile.used)
	CollectFilesSZS(szs2,false,recurse,cut_files,SORT_INAME);

    const szs_subfile_t * p1 = szs1->subfile.list;
    const szs_subfile_t * e1 = szs1->subfile.list + szs1->subfile.used;
    const szs_subfile_t * p2 = szs2->subfile.list;
    const szs_subfile_t * e2 = szs2->subfile.list + szs2->subfile.used;

    noPRINT("PTR: %p..%p/%u, %p..%p/%u\n",
		p1, e1, szs1->subfile.used,
		p2, e2, szs2->subfile.used );

    static char only_message[] = "* Only in source #%u: %s\n";

    int differ = 0;
    while ( p1 && p1 < e1 && p2 && p2 < e2 )
    {
	noPRINT("\t%s : %s\n",p1->path,p2->path);
	const int stat = cmp_subfile_iname(p1,p2);

	if ( stat < 0 )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    printf(only_message,1,p1->path);
	    p1++;
	    continue;
	}

	if ( stat > 0 )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    printf(only_message,2,p2->path);
	    p2++;
	    continue;
	}

	if ( p1->is_dir != p2->is_dir )
	{
	    if (quiet)
		return ERR_DIFFER;
	    differ++;
	    printf("* File types differ: %s\n",p2->path);
	    p1++;
	    p2++;
	    continue;
	}

	if (!p1->is_dir)
	{
	    if ( p1->size != p2->size )
	    {
		if (quiet)
		    return ERR_DIFFER;
		differ++;
		printf("* File size differ:  %s [%u%+d=%u]\n",
			p1->path, p1->size, p2->size - p1->size, p2->size );
	    }
	    else if ( p1->size
			&& p1->size <= szs1->size
			&& p1->offset + p1->size <= szs1->size
			&& memcmp( szs1->data + p1->offset,
				   szs2->data + p2->offset, p1->size ))
	    {
		if (quiet)
		    return ERR_DIFFER;
		differ++;
		printf("* File data differ:  %s\n", p1->path );
	    }
	    else if ( !quiet && verbose > 1 )
	    {
		printf("  + Files identical: %s\n", p1->path );
	    }
	}

	p1++;
	p2++;
    }

    while ( p1 && p1 < e1 )
    {
	if (quiet)
	    return ERR_DIFFER;
	differ++;
	printf(only_message,1,p1->path);
	p1++;
    }

    while ( p2 && p2 < e2 )
    {
	if (quiet)
	    return ERR_DIFFER;
	differ++;
	printf(only_message,2,p2->path);
	p2++;
    }

    return differ ? ERR_DIFFER : ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			string_pool_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeStringPool ( string_pool_t * sp )
{
    DASSERT(sp);
    memset(sp,0,sizeof(*sp));
    InitializeParamField(&sp->pf);
}

///////////////////////////////////////////////////////////////////////////////

void ResetStringPool ( string_pool_t * sp )
{
    DASSERT(sp);
    ResetParamField(&sp->pf);
    FREE(sp->n2s_list);
    FREE(sp->data);
    FREE(sp->offset);
    InitializeStringPool(sp);
}

///////////////////////////////////////////////////////////////////////////////

u32 InsertStringPool
	( string_pool_t * sp, ccp string, bool move_string, ccp data )
{
    DASSERT(sp);
    if (string)
    {
	PRINT_IF( !strcmp(string,"-"),"!!! MINUS ADDED !!!\n");
	if (!sp->data)
	{
	    ParamFieldItem_t *pi
		= FindInsertParamField(&sp->pf,string,move_string,sp->pf.used+1,0);
	    if (pi)
	    {
		if (data)
		    pi->data = (void*)data;
		if ( pi->num >= sp->n2s_size )
		{
		    const uint new_size = 3*pi->num/2 + 0x40;
		    sp->n2s_list
			= REALLOC( sp->n2s_list, sizeof(*sp->n2s_list)*new_size );
		    memset( sp->n2s_list + sp->n2s_size, 0,
			    ( new_size - sp->n2s_size ) * sizeof(*sp->n2s_list) );
		    sp->n2s_size = new_size;
		}
		DASSERT( pi->num < sp->n2s_size );
		sp->n2s_list[pi->num] = pi->key;
		noPRINT("++ %d->%s [%s]\n",pi->num,pi->key,string);
		return pi->num;
	    }
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CalcStringPool
	( string_pool_t * sp, u32 base_offset, const endian_func_t * endian )
{
    DASSERT(sp);
    DASSERT(!sp->data);
    DASSERT(!sp->offset);

    //HEXDUMP16(0,0,sp->n2s_list,sizeof(*sp->n2s_list)*sp->n2s_size);

    u32 * offset = CALLOC(sp->pf.used,sizeof(*offset));
    sp->offset = offset;

    // align string pool
    u32 off = ALIGN32(base_offset,4) - base_offset;
    uint size = off;

    ParamFieldItem_t *ptr, *end = sp->pf.field + sp->pf.used;
    for ( ptr = sp->pf.field; ptr < end; ptr++ )
	size += ALIGN32(strlen(ptr->key)+5,4);
    sp->size = size;
    sp->data = CALLOC(1,size);

    for ( ptr = sp->pf.field; ptr < end; ptr++ )
    {
	*offset++ = base_offset + off + 4;
	u8 * str_ptr = sp->data + off;
	const uint slen = strlen(ptr->key);
	endian->wr32(str_ptr,slen);
	memcpy(str_ptr+4,ptr->key,slen);
	off += ALIGN32(slen+5,4);
    }
    DASSERT( offset == sp->offset + sp->pf.used );
}

///////////////////////////////////////////////////////////////////////////////

u32 FindStringPool ( string_pool_t * sp, ccp string, u32 not_found_value )
{
    DASSERT(sp);
    if (sp->data)
    {
	DASSERT(sp->offset);
	const int str_idx = FindParamFieldIndex(&sp->pf,string,-1);
	noPRINT("STRING-IDX=%d %s\n",str_idx,string);
	if ( str_idx >= 0 )
	    return sp->offset[str_idx];
    }
    return not_found_value;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SetupStandardSZS		///////////////
///////////////////////////////////////////////////////////////////////////////

szs_iterator_func std_iter_func[FF_N] = {0};
szs_iterator_func cut_iter_func[FF_N] = {0};

///////////////////////////////////////////////////////////////////////////////

void SetupStandardSZS()
{
    static bool done = false;
    if (done)
	return;
    done = true;

    std_iter_func[FF_U8]	= IterateFilesU8;
    std_iter_func[FF_WU8]	= IterateFilesU8;
    std_iter_func[FF_LTA]	= IterateFilesLTA;
    std_iter_func[FF_LFL]	= IterateFilesLFL;
    std_iter_func[FF_RARC]	= IterateFilesRARC;
    std_iter_func[FF_RKC]	= IterateFilesRKC;
    std_iter_func[FF_PACK]	= IterateFilesPACK;
    std_iter_func[FF_BRRES]	= IterateFilesBRRES;
    std_iter_func[FF_BREFF]	= IterateFilesBREFF;
    std_iter_func[FF_BREFT]	= IterateFilesBREFF;
 #if 0
    std_iter_func[FF_TEX]	= IterateFilesBRRES;
    std_iter_func[FF_TEX_CT]	= IterateFilesBRRES;
 #endif

    memcpy(cut_iter_func,std_iter_func,sizeof(cut_iter_func));
    cut_iter_func[FF_DOL]	= IterateFilesDOL;

    int i;
    for ( i = FF__FIRST_BRSUB; i <= FF__LAST_BRSUB; i++ )
	cut_iter_func[i] = IterateFilesBRSUB;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


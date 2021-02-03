
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

#include "lib-rkc.h"
#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  RKC action log		///////////////
///////////////////////////////////////////////////////////////////////////////

static bool RKC_ACTION_LOG ( const char * format, ...)
	__attribute__ ((__format__(__printf__,1,2)));

static bool RKC_ACTION_LOG ( const char * format, ...)
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[RKC]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

    if ( verbose > 2 ) // || RKC_MODE & RKCMD_LOG
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[RKC]%s ",colset->heading,colset->info);
	va_list arg;
	va_start(arg,format);
	vfprintf(stdlog,format,arg);
	va_end(arg);
	fputs(colset->reset,stdlog);
	fflush(stdlog);
	return true;
    }

    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IterateFilesRKC()		///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateFilesRKC
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

    if ( !szs->data || szs->size < sizeof(rkct_t) )
	return -1;

    const rkct_t *rkct = (rkct_t*)szs->data;
    if ( ntohl(rkct->magic) != RKCT_MAGIC_NUM )
	return -1;

    it->index    = 0;
    it->fst_item = 0;
    it->is_dir   = 0;

 #if USE_ITERATOR_PARAM
    if (it->itpar.cut_files)
 #else
    if (it->cut_files)
 #endif
    {
	it->no_recurse++;
	it->off		= 0;
	it->size	= sizeof(rkct_t);
	StringCopyS(it->path,sizeof(it->path),".rkc.header");
	it->func_it(it,false);
	it->no_recurse--;
    }

    it->off = sizeof(rkct_t);
    if ( szs->size < sizeof(rkct_t)+sizeof(rkco_t) )
    {
	it->size = szs->size - it->off;
	StringCopyS(it->path,sizeof(it->path),".rkc.trash");
	return it->func_it(it,false);
    }

    it->size = sizeof(rkco_t);
    StringCopyS(it->path,sizeof(it->path),RKCO_FILENAME);
    it->func_it(it,false);

    const uint u8_off = ntohl(rkct->u8_off);
    const uint u8_end = ntohl(rkct->size);
    if ( szs->size < u8_off || szs->size < u8_end )
    {
	it->size = szs->size - it->off;
	StringCopyS(it->path,sizeof(it->path),".rkc.trash.bin");
	return it->func_it(it,false);
    }

 #if USE_ITERATOR_PARAM
    if ( it->itpar.cut_files && sizeof(rkct_t)+sizeof(rkco_t) < u8_off )
 #else
    if ( it->cut_files && sizeof(rkct_t)+sizeof(rkco_t) < u8_off )
 #endif
    {
	it->off  = sizeof(rkct_t)+sizeof(rkco_t);
	it->size = u8_off - it->off;
	StringCopyS(it->path,sizeof(it->path),".rkc.pad-head.bin");
	it->func_it(it,false);
    }

    it->off  = u8_off;
    it->size = u8_end - u8_off;
    StringCopyS(it->path,sizeof(it->path),RKC_U8_FILENAME);
    int stat = it->func_it(it,false);
    if (stat)
	return stat;

    if ( !it->no_recurse && it->recurse_level < it->recurse_max )
    {
	szs_file_t szs2;
// [[fname+]]
	InitializeSubSZS(&szs2,szs,u8_off,u8_end-u8_off,FF_UNKNOWN,it->path,true);

     #if USE_ITERATOR_PARAM
	szs_iterator_func it_func
		= GetIteratorFunction(szs2.fform_arch,it->itpar.cut_files);
     #else
	szs_iterator_func it_func
		= GetIteratorFunction(szs2.fform_arch,it->cut_files);
     #endif

	if (it_func)
	{
	    szs_iterator_t it2;
	    memcpy(&it2,it,sizeof(it2));
	    it2.szs		= &szs2;
	    it2.index		= 0;
	    it2.client_int	= 0;
	    it2.parent_it	= it;
	    it2.recurse_level++;

	    const int stat2 = it_func(&it2,false);
	    it2.name = 0;
	    *it2.path = 0;
	    if ( stat2 != -1 )
		it2.func_it(&it2,true);	// ignore status
	}
	ResetSZS(&szs2);
    }

    if ( szs->size > u8_end )
    {
	it->off	 = u8_end;
	it->size = szs->size - u8_end;
	StringCopyS(it->path,sizeof(it->path),".rkc.pad-szs.bin");
	it->func_it(it,false);
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    CreateRKC()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint get_file_size ( ccp source_dir, ccp fname, uint min, uint max )
{
    s64 szs_size = GetFileSize(source_dir,fname,-1,0,false);
    if ( szs_size < 0 )
    {
	ERROR1(ERR_CANT_OPEN,
		"Can't open file: %s/%s",source_dir,fname);
	return 0;
    }

    if ( szs_size < min || szs_size > max )
    {
	ERROR0(ERR_INVALID_DATA,
		"Wrong filesize (not %u..%u, but %llu): %s/%s",
		min, max, szs_size, source_dir, fname );
	return 0;
    }

    return szs_size;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateRKC
(
    struct szs_file_t	*szs,		// valid szs
    ccp			source_dir	// NULL or path to source dir
)
{
    PRINT("CreateRKC(%s) size=%zu, data=%p\n",
		source_dir, szs->size, szs->data );


    //--- check files

    const uint rkco_size
	= get_file_size( source_dir, RKCO_FILENAME,
			sizeof(rkco_t), RKC_U8_OFFSET-sizeof(rkct_t) );
    const uint szs_size
	= get_file_size( source_dir, RKC_U8_FILENAME,
			sizeof(yaz0_header_t), RKC_U8_LIMIT );
    if ( !rkco_size || !szs_size )
	return ERR_INVALID_DATA;


    //--- create szs

    szs->size = RKC_U8_OFFSET + szs_size;
    szs->data = CALLOC(szs->size,1);

    ClearFileAttrib(&szs->fatt);

    enumError err;
    err = LoadFile( source_dir, RKCO_FILENAME, 0,
			szs->data + sizeof(rkct_t), rkco_size, 0,
			&szs->fatt, true );
    if (err)
	return err;

    err = LoadFile( source_dir, RKC_U8_FILENAME, 0,
			szs->data + RKC_U8_OFFSET, szs_size, 0,
			&szs->fatt, true );
    if (err)
	return err;

    rkct_t *rkct = (rkct_t*)szs->data;
    rkct->magic   = htonl(RKCT_MAGIC_NUM);
    rkct->size    = htonl(szs->size);
    rkct->u8_off  = htonl(RKC_U8_OFFSET);
    rkct->version = htonl(RKC_VERSION);

    szs->fform_arch = szs->fform_file = szs->fform_current = FF_RKC;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

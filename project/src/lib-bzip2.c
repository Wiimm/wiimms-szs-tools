
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

// BZIP2 support: http://www.bzip.org/1.0.5/bzip2-manual-1.0.5.html

#ifndef NO_BZIP2
  #include "libbz2/bzlib.h"
  #include "lib-bzip2.h"
  #include "lib-szs.h"
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    BZIP2 helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

int IsBZ
(
    // returns
    // -1:    not BZIP2 data
    //  1..9: seems to be BZIP2 data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
)
{
    const uint delta = sizeof(wbz_header_t)+sizeof(u32);
    const u8 *d = data;
    if ( !d
	|| size < 0x20
	|| memcmp(d,BZ_MAGIC,4)
	|| memcmp(d+delta,"BZh",3)
	|| memcmp(d+delta+4,"1AY&SY'",6) )
    {
	return -1;
    }

    const uint level = d[delta+3] - '0';
    return level < 10 ? level : 0;
}

///////////////////////////////////////////////////////////////////////////////

int IsBZIP2
(
    // returns
    // -1:    not BZIP2 data
    //  1..9: seems to be BZIP2 data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
)
{
    const u8 *d = data;
    if ( !d || size < 10 || memcmp(d,"BZh",3) || memcmp(d+4,"1AY&SY'",6) )
	return -1;

    const uint level = d[3] - '0';
    return level < 10 ? level : 0;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetMessageBZIP2
(
    int			err,		// error code
    ccp			unkown_error	// result for unkown error codes
)
{
 #ifndef NO_BZIP2
    switch (err)
    {
	case BZ_OK:			return "OK";

	case BZ_CONFIG_ERROR:		return "CONFIG ERROR";
	case BZ_DATA_ERROR:		return "DATA ERROR";
	case BZ_DATA_ERROR_MAGIC:	return "DATA ERROR MAGIC";
	case BZ_IO_ERROR:		return "IO ERROR";
	case BZ_MEM_ERROR:		return "MEM ERROR";
	case BZ_PARAM_ERROR:		return "PARAM ERROR";
	case BZ_SEQUENCE_ERROR:		return "SEQUENCE ERROR";
	case BZ_STREAM_END:		return "STREAM END";
	case BZ_UNEXPECTED_EOF:		return "UNEXPECTED EOF";
    }
 #endif

    return unkown_error;
};

///////////////////////////////////////////////////////////////////////////////

int CalcCompressionLevelBZIP2
(
    int			compr_level	// valid are 1..9 / 0: use default value
)
{
    return compr_level < 1
		? BZIP2_DEFAULT_COMPR
		: compr_level < 9
			? compr_level
			: 9;
}

///////////////////////////////////////////////////////////////////////////////

u32 CalcMemoryUsageBZIP2
(
    int			compr_level,	// valid are 1..9 / 0: use default value
    bool		is_writing	// false: reading mode, true: writing mode
)
{
    compr_level = CalcCompressionLevelBZIP2(compr_level);
    return is_writing
		? ( 4 + 8 * compr_level ) * 102400
		: ( 1 + 4 * compr_level ) * 102400;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 writing			///////////////
///////////////////////////////////////////////////////////////////////////////
#if 0 // not needed yet
///////////////////////////////////////////////////////////////////////////////

enumError EncodeBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file,		// destination file
    int			compr_level	// valid are 1..9 / 0: use default value
)
{
    DASSERT(bz);
    DASSERT(file);
    DASSERT(file->f);

    bz->file = file;
    bz->compr_level = CalcCompressionLevelBZIP2(compr_level);

    int bzerror;
    bz->handle = BZ2_bzWriteOpen(&bzerror,file->f,bz->compr_level,0,0);
    if ( !bz || bzerror != BZ_OK )
    {
	if (bz->handle)
	{
	    BZ2_bzWriteClose(0,bz->handle,0,0,0);
	    bz->handle = 0;
	}

	return ERROR0(ERR_BZIP2,
		"Error while opening bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
	return 0;
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError EncodeBZIP2_Write
(
    BZIP2_t		* bz,		// created by EncodeBZIP2_Open()
    const void		* data,		// data to write
    size_t		data_size	// size of data to write
)
{
    DASSERT(bz);
    DASSERT(bz->handle);

    if (data_size)
    {
	int bzerror;
	BZ2_bzWrite(&bzerror,bz->handle,(u8*)data,data_size);
	if ( bzerror != BZ_OK )
	{
	    BZ2_bzWriteClose(0,bz->handle,0,0,0);
	    bz->handle = 0;

	    return ERROR0(ERR_BZIP2,
		    "Error while writing bzip2 stream: %s\n-> bzip2 error: %s\n",
		    bz->file->fname, GetMessageBZIP2(bzerror,"?") );
	}
	//bz->file->bytes_written += data_size;
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError EncodeBZIP2_Close
(
    BZIP2_t		* bz,		// NULL or created by EncodeBZIP2_Open()
    u32			* bytes_written	// not NULL: store written bytes
)
{
    u32 written = 0;
    if ( bz && bz->handle )
    {
	int bzerror;
	BZ2_bzWriteClose(&bzerror,bz->handle,0,0,&written);
	bz->handle = 0;

	if ( bzerror != BZ_OK )
	    return ERROR0(ERR_BZIP2,
		"Error while closing bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }

    if (bytes_written)
	*bytes_written = written;

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
#endif // 0 // not needed yet
///////////////////////////////////////////////////////////////////////////////

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 reading			///////////////
///////////////////////////////////////////////////////////////////////////////
#if 0 // not needed yet
///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file		// source file
)
{
    DASSERT(file);
    DASSERT(file->f);

    bz->file = file;

    int bzerror;
    bz->handle = BZ2_bzReadOpen(&bzerror,file->f,0,0,0,0);
    if ( !bz->handle || bzerror != BZ_OK )
    {
	if (bz->handle)
	{
	    BZ2_bzReadClose(0,bz->handle);
	    bz->handle = 0;
	}

	return ERROR0(ERR_BZIP2,
		"Error while opening bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2_Read
(
    BZIP2_t		* bz,		// created by DecodeBZIP2_Open()
    void		* buf,		// destination buffer
    size_t		buf_size,	// size of destination buffer
    u32			* buf_written	// not NULL: store bytes written to buf
)
{
    DASSERT(bz);
    DASSERT(bz->handle);
    DASSERT(buf);

    int bzerror;
    const u32 written = BZ2_bzRead(&bzerror,bz->handle,buf,buf_size);
    noPRINT("BZREAD, num=%x, buf_size=%zx, err=%d\n",written,buf_size,bzerror);
    if ( bzerror != BZ_STREAM_END )
    {
	BZ2_bzReadClose(0,bz->handle);
	bz->handle = 0;

	return ERROR0(ERR_BZIP2,
		"Error while reading bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }

    //bz->file->bytes_read += written;
    if (buf_written)
	*buf_written = written;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2_Close
(
    BZIP2_t		* bz		// NULL or created by DecodeBZIP2_Open()
)
{
    if ( bz && bz->handle )
    {
	int bzerror;
	BZ2_bzReadClose(&bzerror,bz->handle);
	bz->handle = 0;

	if ( bzerror != BZ_OK )
	    return ERROR0(ERR_BZIP2,
		    "Error while closing bzip2 stream: %s\n-> bzip2 error: %s\n",
		    bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
#endif // 0 // not needed yet
///////////////////////////////////////////////////////////////////////////////

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    BZIP2 memory conversions		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError EncodeBZIP2buf
(
    void		*dest,		// valid destination buffer
    uint		dest_size,	// size of 'dest'
    uint		*dest_written,	// store num bytes written to 'dest', never NULL

    const void		*src,		// source buffer
    uint		src_size,	// size of source buffer

    int			compr_level,	// valid are 1..9 / 0: use default value
    bool		add_dec_size	// true: add decompressed size
)
{
 #ifdef NO_BZIP2
    DASSERT(dest_written);
    *dest_written = 0;
    return ERROR0(ERR_BZIP2,"BZIP2 compression not supported\n");
 #else // !NO_BZIP2

    // http://www.bzip.org/1.0.3/html/util-fns.html#bzbufftobuffcompress

    DASSERT(dest);
    DASSERT( dest_size > sizeof(u32) );
    DASSERT(dest_written);
    DASSERT(src);

    compr_level = CalcCompressionLevelBZIP2(compr_level);
    PRINT("EncodeBZIP2buf() %u bytes, level = %u\n",src_size,compr_level);

    *dest_written = dest_size;
    if (add_dec_size)
    {
	*(u32*)dest = htonl(src_size);
	dest += sizeof(u32);
	*dest_written -= sizeof(u32);
    }

    int bzerror = BZ2_bzBuffToBuffCompress ( dest, dest_written,
				(char*)src, src_size, compr_level, 0, 0 );

    if ( bzerror != BZ_OK )
	return ERROR0(ERR_BZIP2,
		"Error while compressing data.\n-> bzip2 error: %s\n",
		GetMessageBZIP2(bzerror,"?") );

    if (add_dec_size)
	*dest_written += 4;
    return ERR_OK;
 #endif // !NO_BZIP2
}

///////////////////////////////////////////////////////////////////////////////

enumError EncodeBZIP2
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest', never NULL
    bool		use_iobuf,	// true: allow the usage of 'iobuf'
    uint		header_size,	// insert 'header_size' bytes before dest data
    bool		add_dec_size,	// true: add decompressed size

    const void		*src,		// source buffer
    uint		src_size,	// size of source buffer

    int			compr_level	// valid are 1..9 / 0: use default value
)
{
    DASSERT(dest_ptr);
    DASSERT(dest_written);
    DASSERT(src);

    char *dest_alloced = 0, *dest;
    uint dest_size = src_size + src_size/100 + 600 + 20;
    if ( dest_size <= sizeof(iobuf) && use_iobuf )
    {
	dest = iobuf;
	dest_size = sizeof(iobuf);
    }
    else
    {
	dest_alloced = MALLOC(dest_size+header_size);
	memset(dest_alloced,0,header_size);
	dest = dest_alloced + header_size;
    }

    enumError err = EncodeBZIP2buf( dest, dest_size, dest_written,
					src, src_size, compr_level, add_dec_size );
    if (err)
    {
	FREE(dest_alloced);
	*dest_ptr = 0;
	*dest_written = 0;
    }
    else if (dest_alloced)
    {
	*dest_written += header_size;
	*dest_ptr = REALLOC(dest_alloced,*dest_written);
    }
    else
    {
	DASSERT(dest==iobuf);
	dest = MALLOC(*dest_written+header_size);
	memset(dest,0,header_size);
	memcpy(dest+header_size,iobuf,*dest_written);
	*dest_ptr = (u8*)dest;
	*dest_written += header_size;
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2bin
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest', never NULL
    uint		header_size,	// insert 'header_size' bytes before dest data

    const void		*src,		// source buffer
    uint		src_size	// size of source buffer
)
{
    DASSERT(dest_ptr);
    DASSERT(dest_written);
    DASSERT( src || !src_size );

    *dest_ptr = 0;
    *dest_written = 0;

    if ( !src || !src_size )
	return ERR_NOTHING_TO_DO;

 #ifdef NO_BZIP2
    return ERROR0(ERR_BZIP2,"BZIP2 compression not supported\n");
 #else // !NO_BZIP2

    uint data_size = 2*MiB + header_size;
    u8 *data = MALLOC(data_size);
    memset(data,0,header_size);
    uint data_used = header_size;

    bz_stream bzs = {0};
    int bz2err = BZ2_bzDecompressInit(&bzs,0,0);
    if ( bz2err != BZ_OK )
	goto error;

    bzs.next_in = (char*)src;
    bzs.avail_in = src_size;

    for(;;)
    {
	if ( data_size - data_used < MiB )
	{
	    data_size += 2*MiB;
	    data = REALLOC(data,data_size);
	    if (!data)
		goto error;
	}

	bzs.next_out = (char*)data + data_used;
	bzs.avail_out = data_size - data_used;
	bz2err = BZ2_bzDecompress(&bzs);
	data_used = bzs.next_out - (char*)data;
	if ( bz2err == BZ_STREAM_END )
	    break;
	if ( bz2err != BZ_OK )
	    goto error;
    }
    BZ2_bzDecompressEnd(&bzs);
    u8 *newdata = REALLOC(data,data_used);
    *dest_ptr = newdata ? newdata : data;
    *dest_written = data_used;
    return ERR_OK;

 error:
    BZ2_bzDecompressEnd(&bzs);
    FREE(data);
    return ERR_BZIP2;

 #endif // !NO_BZIP2
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2part
(
    // decompress until dest buffer full or end of source reached
    // return ERR_WARNING for an incomplete decompression

    void		*dest_buf,	// result: store read data here
    uint		dest_size,	// size of 'dest_buf'
    uint		*dest_written,	// store num bytes written to 'dest_ptr', never NULL

    const void		*src,		// source buffer
    uint		src_size	// size of source buffer
)
{
    DASSERT(dest_buf);
    DASSERT(dest_size);
    DASSERT(dest_written);
    DASSERT( src || !src_size );

    *dest_written = 0;
    if ( !src || !src_size )
	return ERR_NOTHING_TO_DO;

 #ifdef NO_BZIP2
    return ERROR0(ERR_BZIP2,"BZIP2 compression not supported\n");
 #else // !NO_BZIP2

    bz_stream bzs = {0};
    int bz2err = BZ2_bzDecompressInit(&bzs,0,0);
    if ( bz2err != BZ_OK )
	goto error;

    bzs.next_in   = (char*)src;
    bzs.avail_in  = src_size;
    bzs.next_out  = (char*)dest_buf;
    bzs.avail_out = dest_size;

    bz2err = BZ2_bzDecompress(&bzs);
    *dest_written = bzs.next_out - (char*)dest_buf;
    if ( bz2err != BZ_STREAM_END && bz2err != BZ_OK )
	goto error;

    BZ2_bzDecompressEnd(&bzs);
    return bz2err == BZ_OK ? ERR_OK : ERR_WARNING;

 error:
    BZ2_bzDecompressEnd(&bzs);
    return ERR_BZIP2;

 #endif // !NO_BZIP2
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2buf
(
    void		*dest,		// valid destination buffer
    uint		dest_size,	// size of 'dest'
    uint		*dest_written,	// store num bytes written to 'dest', never NULL

    const void		*src,		// source buffer, first 4 bytes = dest size
    uint		src_size	// size of source buffer
)
{
 #ifdef NO_BZIP2
    DASSERT(dest_written);
    *dest_written = 0;
    return ERROR0(ERR_BZIP2,"BZIP2 compression not supported\n");
 #else // !NO_BZIP2

    // http://www.bzip.org/1.0.3/html/util-fns.html#bzbufftobuffdecompress

    DASSERT(dest);
    DASSERT( dest_size >= sizeof(u32) );
    DASSERT(dest_written);
    DASSERT(src);

    uint dest_need = ntohl(*(u32*)src);
    PRINT("DecodeBZIP2buf() %u -> %u,%u bytes\n",src_size,dest_size,dest_need);

    if ( be32(src+4) == 0x52415730 ) // "RAW0" == not compressed
    {
	const uint copy_len = dest_size < dest_need ? dest_size : dest_need;
	*dest_written = copy_len;
	memcpy(dest,(u8*)src+8,copy_len);
	return ERR_OK;
    }

    *dest_written = dest_need;
    int bzerror = BZ2_bzBuffToBuffDecompress ( (char*)dest, dest_written,
				(char*)src+4, src_size-4, 0, 0 );

    if ( bzerror != BZ_OK )
	return ERROR0(ERR_BZIP2,
		"Error while decompressing data.\n-> bzip2 error: %s\n",
		GetMessageBZIP2(bzerror,"?") );

    return ERR_OK;
 #endif // !NO_BZIP2
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest', never NULL
    uint		header_size,	// insert 'header_size' bytes before dest data

    const void		*src,		// source buffer, first 4 bytes = dest size
    uint		src_size	// size of source buffer
)
{
    DASSERT(dest_ptr);
    DASSERT(dest_written);
    DASSERT(src);
    DASSERT( src_size >= sizeof(u32) );

    uint dest_size = ntohl(*(u32*)src);
    u8 *dest = MALLOC(dest_size+header_size);
    memset(dest,0,header_size);

    enumError err = DecodeBZIP2buf(dest+header_size,dest_size,dest_written,src,src_size);
    if (err)
    {
	FREE(dest);
	*dest_ptr = 0;
	*dest_written = 0;
    }
    else
    {
	*dest_ptr = dest;
	*dest_written += header_size;
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2Manager
(
    BZ2Manager_t	*mgr		// manager data
)
{
    DASSERT(mgr);

    if (!mgr->data)
    {
	mgr->size = 0;
	if (mgr->src_data)
	    return DecodeBZIP2(&mgr->data,&mgr->size,0,mgr->src_data,mgr->src_size);
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZ2S = BZ2-Source		///////////////
///////////////////////////////////////////////////////////////////////////////

const char InternalBZ2S[] = "<intern>";

///////////////////////////////////////////////////////////////////////////////

uint ClearBZ2S
(
    // Returns the number of cleard elements
    // Call this function to free results of SearchBZ2S() and SearchListBZ2S()
    // Both functions call ClearBZ2S() at the beginning.

    BZ2Source_t		*src,		// valid BZ2Source_t list
    int			n		// >=0: clear 'n' elements max,
					//      but always stop at list end!
)
{
    DASSERT(src);

    BZ2Source_t *s = src;
    for ( ; s->search && n-- != 0 ; s++ )
    {
	if (s->fname)
	{
	    if ( s->fname != InternalBZ2S )
		FreeString(s->fname);
	    s->fname = NULL;
	}

	if (s->data)
	{
	    if (IsDataAllocedBZ2S(s))
		FREE(s->data);
	    s->data = 0;
	}

	s->size = 0;
	s->status = BZ2S_T_NONE;
    }

    return s - src;
}

///////////////////////////////////////////////////////////////////////////////

u8 * NonConstDataBZ2S
(
    // Converts src->data to malloced data (modifications possible)
    // return src->data after conversion.

    BZ2Source_t		*src		// valid BZ2Source_t
)
{
    if ( !src || !src->data || !src->size )
	return 0;

    if ( !IsDataAllocedBZ2S(src) )
	src->data = MEMDUP(src->data,src->size);
    return src->data;
}

///////////////////////////////////////////////////////////////////////////////

BZ2Status_t SearchBZ2S
(
    BZ2Source_t		*src,		// valid single BZ2Source_t
    ccp			*dir_list,	// list with search directories
					// NULL and empty strings are ignored.
    int			n_dir_list,	// number of elements in 'dir_list'
					// <0: NULL termianted list
    SearchLog_t		log_level	// one of SEA_LOG_*
)
{
    DASSERT(src);

    ClearBZ2S(src,1);

    if ( dir_list && src->search && *src->search )
    {
	for ( ; n_dir_list-- != 0 ; dir_list++ )
	{
	    if (!*dir_list)
	    {
		if ( n_dir_list > 0 )
		    continue;
		break;
	    }
	    if (!**dir_list)
		continue;
	    if ( log_level >= SEA_LOG_ALL )
		fprintf(stdlog,"> SEARCH %s @DIR %s\n",src->search,*dir_list);

	    char path[PATH_MAX], *pend = path + sizeof(path) - 1;
	    char *pbase = StringCopyS(path,sizeof(path)-2,*dir_list);
	    if ( pbase == path )
	    {
		*pbase++ = '.';
		*pbase++ = '/';
	    }
	    else if ( pbase[-1] != '/' )
		*pbase++ = '/';

	    ccp search = src->search;
	    DASSERT(search);
	    while( *search )
	    {
		while ( *search == ';' )
		    search++;
		char *pptr = pbase;
		while ( *search && *search != ';' )
		{
		    if ( pptr < pend )
			*pptr++ = *search;
		    search++;
		}
		DASSERT( pptr < path + sizeof(path) );
		if ( pptr == pbase )
		    continue;
		*pptr = 0;

		noPRINT("  SEARCH: %s\n",path);

		u8 *data = 0;
		uint size = 0;
		enumError err = OpenReadFILE(path,0,true,&data,&size,0,0);
		if ( err != ERR_OK )
		{
		    FREE(data);
		    StringCopyE(pptr,pend,".bz2");
		    noPRINT("  SEARCH: %s\n",path);
		    err = OpenReadFILE(path,0,true,&data,&size,0,0);
		    if ( err != ERR_OK )
		    {
			if ( log_level >= SEA_LOG_ALL )
			    fprintf(stdlog,">> NOT FOUND: %.*s\n",(int)(pptr-path),path);
			FREE(data);
			continue;
		    }
		}

		src->status = BZ2S_T_FILE;
		if ( IsBZIP2(data,size) >= 0 )
		{
		    // seems to be bzip2 encoded
		    if ( log_level >= SEA_LOG_FOUND )
			fprintf(stdlog,"> BZIP2 READ: %s\n",path);

		    PRINT("     \e[36;1m==> BZIP2: %s\e[0m\n",path);

		    u8 *bz2data = 0;
		    uint bz2size;
		    if (DecodeBZIP2bin(&bz2data,&bz2size,0,data,size))
			FREE(bz2data);
		    else
		    {
			FREE(data);
			data = bz2data;
			size = bz2size;
			src->status |= BZ2S_F_BZIP2;
		    }
		}
// [[lzma]] [[lz]] [[xz]]
		else if ( log_level >= SEA_LOG_FOUND )
		    fprintf(stdlog,"> READ: %s\n",path);

		if ( src->fform != FF_UNKNOWN )
		{
// [[analyse-magic]]
		    file_format_t ff = GetByMagicFF(data,size,size);
		    if ( ff != src->fform )
		    {
			ERROR0(ERR_WARNING,
			    "Need file format %s, but file is %s -> ignore %s\n",
			    GetNameFF(src->fform,0), GetNameFF(ff,0), path );
			src->status = BZ2S_T_NONE;
			continue;
		    }
		}

		src->fname = STRDUP(path);
		src->data = data;
		src->size = size;
		PRINT("     \e[33;1m==> %u bytes FOUND: %s\e[0m\n",size,path);
		return src->status;
	    }
	}
    }


    //--- fall back to internal source

    if ( src->src_data && src->src_size )
    {
	src->fname = InternalBZ2S;

	u8 *data  = (u8*)src->src_data;
	uint size = src->src_size;

	src->status = BZ2S_T_INTERN;
	if ( size > 4 && IsBZIP2(data+4,size-4) >= 0 )
	{
	    PRINT("     \e[36;1m==> BZIP2: %s\e[0m\n",src->fname);

	    u8 *bz2data = 0;
	    uint bz2size;
	    if (DecodeBZIP2(&bz2data,&bz2size,0,data,size))
		FREE(bz2data);
	    else
	    {
		data = bz2data;
		size = bz2size;
		src->status |= BZ2S_F_BZIP2;
	    }
	}
// [[lz]] [[lzma]] [[xz]]

	if ( src->part_offset > 0 )
	{
	    const uint off = src->part_offset < size ? src->part_offset : size;
	    size -= off;
	    if ( src->part_size && src->part_size < size )
		size = src->part_size;
	    PRINT("     \e[36;1m==> PART %u..%u: %s\e[0m\n",off,off+size,src->fname);
	    if ( src->status == BZ2S_T_INTERN ) // without BZ2S_F_BZIP2
		data += off;
	    else
	    {
		u8 *newdata = MEMDUP(data+off,size);
		FREE(data);
		data = newdata;
	    }
	}

	src->data = data;
	src->size = src->part_size && src->part_size < size ? src->part_size : size;
	PRINT("     \e[33;1m==> %u/%u bytes FOUND: %s\e[0m\n",src->size,size,src->fname);
	return src->status;
    }


    //--- no file found

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint SearchListBZ2S
(
    // returns the number of found (file or internal) sources

    BZ2Source_t		*src,		// valid BZ2Source_t list
    int			n,		// >=0: search 'n' elements max
					//      but always stop at list end!
    ccp			*dir_list,	// NULL or list with search directories
					// NULL and empty strings are ignored.
    int			n_dir_list,	// number of elements in 'dir_list'
					// <0: NULL terminated list
    SearchLog_t		log_level	// one of SEA_LOG_*
)
{
    DASSERT(src);
    uint count = 0;
    for ( ; src->search && n-- != 0 ; src++ )
	if (SearchBZ2S(src,dir_list,n_dir_list,log_level))
	    count++;
    return count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


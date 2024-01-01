
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

#ifndef NO_LZMA
  #include "lib-szs.h"
  #include "lib-lzma.h"
  #include "liblzma/LzmaEnc.h"
  #include "liblzma/LzmaDec.h"
#endif

// https://7-zip.org/sdk.html

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    LZMA helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static void * AllocLZMA ( ISzAllocPtr p, size_t size )
{
    return MALLOC(size);
}

static void FreeLZMA ( ISzAllocPtr p, void *ptr )
{
    FREE(ptr);
}

static ISzAlloc lzma_alloc = { AllocLZMA, FreeLZMA };

///////////////////////////////////////////////////////////////////////////////

int IsLZ
(
    // returns
    // -1:    not LZ data
    //	0:    seems to be LZ; compression level is unknown
    //  1..9: seems to be LZ; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
)
{
    const uint delta = sizeof(wlz_header_t)+sizeof(u32);
    const u8 *d = data;
    if ( !d || size < 0x20 || memcmp(d,LZ_MAGIC,4) ) 
    {
	return -1;
    }

    const u32 magic = be24(d+delta);
    if ( magic != LZMA_MAGIC_NUM3 )
	return -1;

    return GetComprLevelLZMA(le32(d+delta+1));
}

///////////////////////////////////////////////////////////////////////////////

int IsLZMA
(
    // returns
    // -1:    not LZMA data
    //	0:    seems to be LZMA data; compression level is unknown
    //  1..9: seems to be LZMA data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
)
{
    const u8 *d = data;
    if ( !d || size < 20 )
	return -1;

    const u32 magic = be24(d);
    if ( magic != LZMA_MAGIC_NUM3 )
	return -1;

    return GetComprLevelLZMA(le32(d+1));
}

///////////////////////////////////////////////////////////////////////////////

int CalcCompressionLevelLZMA
(
    int			compr_level	// valid are 1..9 / 0: use default value
)
{
    return compr_level < 1
		? LZMA_DEFAULT_COMPR
		: compr_level < 9
		? compr_level
		: 9;
}

///////////////////////////////////////////////////////////////////////////////

static int lzma_dict_size[] =
{
		0, // level 0
	0x0100000, // level 1
	0x0200000, // level 2
	0x0400000, // level 3
	0x0400000, // level 4
	0x0800000, // level 5
	0x0800000, // level 6
	0x1000000, // level 7
	0x2000000, // level 8
	0x4000000, // level 9
};

///////////////////////////////////////////////////////////////////////////////

int GetDicSizeByLevel ( int level )
{
    if ( level < 1 )
	level = 1;
    else if ( level > 9 )
	level = 9;
    return lzma_dict_size[level];
}

///////////////////////////////////////////////////////////////////////////////

int GetComprLevelLZMA ( u32 dict_size )
{
    for ( int level = 9; level >= 1; level -- )
	if ( dict_size >= lzma_dict_size[level] )
	    return level;
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetMessageLZMA
(
    int			err,		// error code
    ccp			unkown_error	// result for unkown error codes
)
{
    switch (err)
    {
	case SZ_OK:			return "OK";

	case SZ_ERROR_ARCHIVE:		return "Archive error";
	case SZ_ERROR_CRC:		return "CRC error";
	case SZ_ERROR_DATA:		return "Data error";
	case SZ_ERROR_FAIL:		return "Operation failed";
	case SZ_ERROR_INPUT_EOF:	return "Need more input";
	case SZ_ERROR_MEM:		return "Memory allocation error";
	case SZ_ERROR_NO_ARCHIVE:	return "No archive";
	case SZ_ERROR_OUTPUT_EOF:	return "Putput buffer overflow";
	case SZ_ERROR_PARAM:		return "Incorrect parameter";
	case SZ_ERROR_PROGRESS:		return "Some break from progress callback";
	case SZ_ERROR_READ:		return "Reading error";
	case SZ_ERROR_THREAD:		return "Errors in multithreading functions";
	case SZ_ERROR_UNSUPPORTED:	return "Unsupported properties";
	case SZ_ERROR_WRITE:		return "Writing error";
    }

    return unkown_error;
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    Encode LZMA			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError EncodeLZMAbuf
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
 #ifdef NO_LZMA
    DASSERT(dest_written);
    *dest_written = 0;
    return ERROR0(ERR_LZMA,"LZMA compression not supported\n");
 #else // !NO_LZMA

    DASSERT(dest);
    DASSERT( dest_size > 20 );
    DASSERT(dest_written);
    DASSERT(src);

    const int header_size = LZMA_PROPS_SIZE + 8;
    compr_level = CalcCompressionLevelLZMA(compr_level);
    size_t out_len = 0;
    PRINT0("EncodeLZMAbuf() %u bytes, level = %u\n",src_size,compr_level);

    if (add_dec_size)
    {
	*(u32*)dest = htonl(src_size);
	dest += sizeof(u32);
	dest_size -= sizeof(u32);
    }

    CLzmaEncHandle lzma = LzmaEnc_Create(&lzma_alloc);
    if (!lzma)
        return SZ_ERROR_MEM;

    CLzmaEncProps props;
    LzmaEncProps_Init(&props);
    props.level = compr_level;
    props.dictSize = GetDicSizeByLevel(compr_level);
    LzmaEncProps_Normalize(&props);

    SRes res = LzmaEnc_SetProps(lzma,&props);
    if ( res == SZ_OK )
    {
	size_t prop_size = LZMA_PROPS_SIZE;
        res = LzmaEnc_WriteProperties(lzma,dest,&prop_size);
//        write_le64(dest+LZMA_PROPS_SIZE,src_size);
        write_le64(dest+LZMA_PROPS_SIZE,~0ull); // stream mode
        dest += header_size;
        dest_size -= header_size;

        if ( res == SZ_OK )
        {
	    out_len = dest_size;
	    size_t in_len  = src_size;
            res = LzmaEnc_MemEncode( lzma, dest, &out_len, src, in_len,
					true, 0, &lzma_alloc, &lzma_alloc );
	}
    }
    LzmaEnc_Destroy( lzma,&lzma_alloc,&lzma_alloc);
    *dest_written = out_len + header_size;
    if (add_dec_size)
	*dest_written += sizeof(u32);
 #endif
    return res == SZ_OK ? ERR_OK : ERR_LZMA;
}

///////////////////////////////////////////////////////////////////////////////

enumError EncodeLZMA
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

    enumError err = EncodeLZMAbuf( dest, dest_size, dest_written,
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

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    Decode LZMA			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecodeLZMAbin
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

    if ( !src || src_size <= LZMA_PROPS_SIZE + 8 )
	return ERR_NOTHING_TO_DO;

 #ifdef NO_LZMA
    return ERROR0(ERR_LZMA,"LZMA compression not supported\n");
 #else // !NO_LZMA

    CLzmaDec lzma;
    LzmaDec_Construct(&lzma);
    SRes res = LzmaDec_Allocate(&lzma,src,LZMA_PROPS_SIZE,&lzma_alloc);
    if ( res != SZ_OK )
	return ERROR0(ERR_LZMA,
		"Error while setup LZMA properties: %s\n", GetMessageLZMA(res,"?") );

    src += LZMA_PROPS_SIZE;
    src_size -= LZMA_PROPS_SIZE;

    size_t in_len = le64(src);
    src += 8;
    src_size -= 8;

    FastBuf_t destbuf;
    InitializeFastBuf(&destbuf,sizeof(destbuf));

    const int outbuf_size = 0x200000;
    u8 *outbuf = MALLOC(outbuf_size);

    LzmaDec_Init(&lzma);

    while ( src_size > 0 )
    {
	size_t out_len = outbuf_size;
	ELzmaFinishMode finish = LZMA_FINISH_ANY;
	ELzmaStatus status;

	res = LzmaDec_DecodeToBuf(&lzma,outbuf,&out_len,src,&in_len,finish,&status);
	PRINT0("DECODED, res=%s, stat=%d, in=%zu/%u out=%zu/%u\n",
		GetMessageLZMA(res,"?"), status,
		in_len, src_size, out_len, outbuf_size );

	if ( res == SZ_OK && !in_len && !out_len && status != LZMA_STATUS_FINISHED_WITH_MARK )
	    res = SZ_ERROR_DATA;

	if ( res != SZ_OK )
	{
	    ERROR0(ERR_LZMA,
		"Error while reading LZMA stream: %s\n", GetMessageLZMA(res,"?") );
	    goto error;
	}

	AppendFastBuf(&destbuf,outbuf,out_len);

	if ( in_len < src_size )
	{
	    src_size -= in_len;
	    src += in_len;
	}
	else
	    break;

	if ( status == LZMA_STATUS_FINISHED_WITH_MARK )
	    break;
    }

    LzmaDec_Free(&lzma,&lzma_alloc);
    FREE(outbuf);
    *dest_ptr = MEMDUP(destbuf.buf,GetFastBufLen(&destbuf));
    *dest_written = GetFastBufLen(&destbuf);
    ResetFastBuf(&destbuf);
    return ERR_OK;

 error:;
    LzmaDec_Free(&lzma,&lzma_alloc);
    FREE(outbuf);
    ResetFastBuf(&destbuf);
    return ERR_LZMA;

 #endif // !NO_LZMA
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeLZMApart
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
    DASSERT(dest_ptr);
    DASSERT(dest_written);
    DASSERT( src || !src_size );

    *dest_written = 0;

    if ( !src || src_size <= LZMA_PROPS_SIZE + 8 )
	return ERR_NOTHING_TO_DO;

 #ifdef NO_LZMA
    return ERROR0(ERR_LZMA,"LZMA compression not supported\n");
 #else // !NO_LZMA

    CLzmaDec lzma;
    LzmaDec_Construct(&lzma);
    SRes res = LzmaDec_Allocate(&lzma,src,LZMA_PROPS_SIZE,&lzma_alloc);
    if ( res != SZ_OK )
	return ERROR0(ERR_LZMA,
		"Error while setup LZMA properties: %s\n", GetMessageLZMA(res,"?") );

    src += LZMA_PROPS_SIZE;
    src_size -= LZMA_PROPS_SIZE;

    size_t in_len = le64(src);
    src += 8;
    src_size -= 8;

    LzmaDec_Init(&lzma);

    size_t out_len = dest_size;
    ELzmaFinishMode finish = LZMA_FINISH_ANY;
    ELzmaStatus status;

    res = LzmaDec_DecodeToBuf(&lzma,dest_buf,&out_len,src,&in_len,finish,&status);
    PRINT0("DECODED, res=%s, stat=%d, in=%zu/%u out=%zu/%u\n",
	    GetMessageLZMA(res,"?"), status,
	    in_len, src_size, out_len, dest_size );

    if ( res == SZ_OK && !in_len && !out_len && status != LZMA_STATUS_FINISHED_WITH_MARK )
	res = SZ_ERROR_DATA;

    if ( res != SZ_OK )
    {
	ERROR0(ERR_LZMA,
	    "Error while reading LZMA stream: %s\n", GetMessageLZMA(res,"?") );
	goto error;
    }

    LzmaDec_Free(&lzma,&lzma_alloc);
    *dest_written = out_len;
    return in_len < src_size ? ERR_WARNING : ERR_OK;

 error:;
    LzmaDec_Free(&lzma,&lzma_alloc);
    return ERR_LZMA;

 #endif // !NO_LZMA
}

///////////////////////////////////////////////////////////////////////////////

enumError DecodeLZMAsize
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

    uint dest_size = ntohl(*(u32*)src)+4; // +4: to suppress abort because dest buff is full
    u8 *dest = MALLOC(dest_size+header_size);
    memset(dest,0,header_size);
    *dest_written = header_size;

    enumError err = DecodeLZMApart(dest+header_size,dest_size,dest_written,src+4,src_size-4);
    if (err)
    {
	FREE(dest);
	*dest_ptr = 0;
	*dest_written = 0;
    }
    else
	*dest_ptr = dest;

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    XZ helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

int IsXZ
(
    // returns
    // -1:    not XZ data
    //	0:    seems to be LZMA data; compression level is unknown
    //  1..9: seems to be LZMA data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
)
{
    const u8 *d = data;
    if ( !d || size < 20 )
	return -1;

    u64 magic = be48(d);
    if ( magic != XZ_MAGIC_NUM6 )
	return -1;

    switch(d[16])
    {
	case 0x10: return 1;
	case 0x12: return 2;
	case 0x14: return 4;
	case 0x16: return 6;
	case 0x18: return 7;
	case 0x1a: return 8;
	case 0x1c: return 9;
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////



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

#ifndef SZS_LIB_LZMA_H
#define SZS_LIB_LZMA_H 1

#define _GNU_SOURCE 1

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define LZMA_MAGIC_NUM3		0x5d0000		// 3 relevant bytes
#define XZ_MAGIC_NUM6		0xfd377a585a00ull	// 6 relevant bytes

#define LZMA_DEFAULT_COMPR	6

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

int IsLZ
(
    // returns
    // -1:    not LZ data
    //	0:    seems to be LZ; compression level is unknown
    //  1..9: seems to be LZ; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
);

//-----------------------------------------------------------------------------

int IsLZMA
(
    // returns
    // -1:    not LZMA data
    //	0:    seems to be LZMA data; compression level is unknown
    //  1..9: seems to be LZMA data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
);

//-----------------------------------------------------------------------------

int CalcCompressionLevelLZMA
(
    int			compr_level	// valid are 1..9 / 0: use default value
);

//-----------------------------------------------------------------------------

int GetDicSizeByLevel ( int level );
int GetComprLevelLZMA ( u32 dict_size );

//-----------------------------------------------------------------------------

int IsXZ
(
    // returns
    // -1:    not XZ data
    //	0:    seems to be LZMA data; compression level is unknown
    //  1..9: seems to be LZMA data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
);

//-----------------------------------------------------------------------------

ccp GetMessageLZMA
(
    int			err,		// error code
    ccp			unkown_error	// result for unkown error codes
);

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
);

//-----------------------------------------------------------------------------

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
);

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
);

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
);

///////////////////////////////////////////////////////////////////////////////

enumError DecodeLZMAsize
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest', never NULL
    uint		header_size,	// insert 'header_size' bytes before dest data

    const void		*src,		// source buffer, first 4 bytes = dest size
    uint		src_size	// size of source buffer
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_LZMA_H 1


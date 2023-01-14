
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

#ifndef WIT_LIB_BZIP2_H
#define WIT_LIB_BZIP2_H 1
#ifndef NO_BZIP2

#define _GNU_SOURCE 1

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef _BZLIB_H
    typedef void BZFILE;
#endif

#define BZIP2_DEFAULT_COMPR 9

//-----------------------------------------------------------------------------

typedef struct BZIP2_t
{
    File_t		* file;		// IO file
    BZFILE		* handle;	// bzip2 handle
    int			compr_level;	// active compression level

} BZIP2_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

uint IsBZIP2
(
    // returns
    //	0:    not BZIP2 data
    //  1..9: seems to be BZIP2 data; compression level is returned

    cvp			data,		// NULL or data to investigate
    uint		size		// size of 'data'
);

//-----------------------------------------------------------------------------

ccp GetMessageBZIP2
(
    int			err,		// error code
    ccp			unkown_error	// result for unkown error codes
);

//-----------------------------------------------------------------------------

int CalcCompressionLevelBZIP2
(
    int			compr_level	// valid are 1..9 / 0: use default value
);

//-----------------------------------------------------------------------------

u32 CalcMemoryUsageBZIP2
(
    int			compr_level,	// valid are 1..9 / 0: use default value
    bool		is_writing	// false: reading mode, true: writing mode
);

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
);

//-----------------------------------------------------------------------------

enumError EncodeBZIP2_Write
(
    BZIP2_t		* bz,		// created by EncodeBZIP2_Open()
    const void		* data,		// data to write
    size_t		data_size	// size of data to write
);

//-----------------------------------------------------------------------------

enumError EncodeBZIP2_Close
(
    BZIP2_t		* bz,		// NULL or created by EncodeBZIP2_Open()
    u32			* bytes_written	// not NULL: store written bytes
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 reading			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecodeBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file		// source file
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2_Read
(
    BZIP2_t		* bz,		// created by DecodeBZIP2_Open()
    void		* buf,		// destination buffer
    size_t		buf_size,	// size of destination buffer
    u32			* buf_written	// not NULL: store bytes written to buf
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2_Close
(
    BZIP2_t		* bz		// NULL or created by DecodeBZIP2_Open()
);

///////////////////////////////////////////////////////////////////////////////
#endif // 0 // not needed yet
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
///////////////		    BZIP2 memory conversions		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[BZ2Manager_t]]

typedef struct BZ2Manager_t
{
    //--- source

    cvp		src_data;	// BZ2 is automatically detected and
				// decoded by DecodeBZIP2(). Never NULL
    uint	src_size;	// size of 'src_data'

    //--- decoded

    u8		*data;		// NULL or data, alloced if not part of 'src_data'
    uint	size;		// size of 'data'
}
BZ2Manager_t;

///////////////////////////////////////////////////////////////////////////////

enumError EncodeBZIP2buf
(
    void		*dest,		// valid destination buffer
    uint		dest_size,	// size of 'dest'
    uint		*dest_written,	// store num bytes written to 'dest', never NULL

    const void		*src,		// source buffer
    uint		src_size,	// size of source buffer

    int			compr_level	// valid are 1..9 / 0: use default value
);

//-----------------------------------------------------------------------------

enumError EncodeBZIP2
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest_ptr', never NULL
    bool		use_iobuf,	// true: allow thhe usage of 'iobuf'
    uint		header_size,	// insert 'header_size' bytes before dest data

    const void		*src,		// source buffer
    uint		src_size,	// size of source buffer

    int			compr_level	// valid are 1..9 / 0: use default value
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2stream
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest_ptr', never NULL
    uint		header_size,	// insert 'header_size' bytes before dest data

    const void		*src,		// source buffer
    uint		src_size	// size of source buffer
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2part
(
    // decompress until dest buffer full or end of source reached
    // return ERR_WARNING for an incomplete decompression

    void		*dest_buf,	// result: store rad data hers
    uint		dest_size,	// size of 'dest_buf'
    uint		*dest_written,	// store num bytes written to 'dest_ptr', never NULL

    const void		*src,		// source buffer
    uint		src_size	// size of source buffer
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2buf
(
    void		*dest,		// valid destination buffer
    uint		dest_size,	// size of 'dest'
    uint		*dest_written,	// store num bytes written to 'dest_ptr', never NULL

    const void		*src,		// source buffer
    uint		src_size	// size of source buffer
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2
(
    u8			**dest_ptr,	// result: store destination buffer addr
    uint		*dest_written,	// store num bytes written to 'dest', never NULL
    uint		header_size,	// insert 'header_size' bytes before dest data

    const void		*src,		// source buffer
    uint		src_size	// size of source buffer
);

//-----------------------------------------------------------------------------

enumError DecodeBZIP2Manager
(
    BZ2Manager_t	*mgr		// manager data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZ2S = BZ2-Source		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[BZ2SMode_t]]

typedef enum BZ2Status_t
{
    BZ2S_F_BZIP2	= 1,	// flag: source was bzip2 encoded
    BZ2S_M_TYPE		= ~1,	// types without flags

    BZ2S_T_NONE		= 0,	// no data found
    BZ2S_T_INTERN	= 2,	// internal data used
    BZ2S_T_FILE		= 4,	// external file used
}
BZ2Status_t;

//-----------------------------------------------------------------------------
// [[BZ2Source_t]]

typedef struct BZ2Source_t
{
    //--- search files

    ccp		search;		// Semicolon separated list of filenames
				// or sub-paths to search.
				// NULL terminate source lists.

    //--- internal fall back data

    cvp		src_data;	// NULL or internal fall back data.
				// BZ2 is automatically detected and
				// decoded by DecodeBZIP2().
    uint	src_size;	// size of 'src_data'

    file_format_t fform;	// not FF_UNKNOWN: Allow only this type

    uint	part_offset;	// >0: use only a part of (decoced) 'src_data'
    uint	part_size;	// >0: limit result to this size if
				// 'src_data' is used

    //--- result of search

    BZ2Status_t	status;		// return value of SearchBZ2S()
    ccp		fname;		// NULL or filename, alloced if not 'InternalBZ2S'
    u8		*data;		// NULL or data, alloced if not part of 'src_data'
    uint	size;		// size of 'data'
}
BZ2Source_t;

///////////////////////////////////////////////////////////////////////////////

typedef enum SearchLog_t
{
	SEA_LOG_OFF,
	SEA_LOG_SUCCESS,
	SEA_LOG_FOUND,
	SEA_LOG_ALL,
}
SearchLog_t;

extern const char InternalBZ2S[]; // = "<intern>"

//-----------------------------------------------------------------------------

uint ClearBZ2S
(
    // Returns the number of cleard elements
    // Call this function to free results of SearchBZ2S() and SearchListBZ2S()
    // Both functions call ClearBZ2S() at the beginning.

    BZ2Source_t		*src,		// valid BZ2Source_t list
    int			n		// >=0: clear 'n' elements max,
					//      but always stop at list end!
);

//-----------------------------------------------------------------------------

static inline bool IsDataAllocedBZ2S ( BZ2Source_t *src	)
{
    DASSERT(src);
    return !src->src_data
	|| src->data < (u8*)src->src_data
	|| src->data > (u8*)src->src_data + src->src_size;
}

//-----------------------------------------------------------------------------

u8 * NonConstDataBZ2S
(
    // Converts src->data to malloced data (modifications possible)
    // return src->data after conversion.

    BZ2Source_t		*src		// valid BZ2Source_t
);

//-----------------------------------------------------------------------------

BZ2Status_t SearchBZ2S
(
    BZ2Source_t		*src,		// valid single BZ2Source_t
    ccp			*dir_list,	// list with search directories
					// NULL and empty strings are ignored.
    int			n_dir_list,	// number of elements in 'dir_list'
					// <0: NULL termianted list
    SearchLog_t		log_level	// one of SEA_LOG_*
);

//-----------------------------------------------------------------------------

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
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // !NO_BZIP2
#endif // WIT_LIB_BZIP2_H 1


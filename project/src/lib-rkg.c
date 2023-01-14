
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

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <dirent.h>
//#include <math.h>

#include "lib-rkg.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawDataGHOST()		///////////////
///////////////////////////////////////////////////////////////////////////////

static u32 ScanTimeRKG ( const u8 * data )
{
    DASSERT(data);
    const u32 min = data[0] >> 1;
    const u32 sec = be16(data) >> 2 & 0x7f;
    return ( be16(data+1) & 0x3ff ) + 1000 * sec + 60000 * min;
}

//-----------------------------------------------------------------------------

static uint CalcFrameTime ( const u8 ** pptr, uint n )
{
    const u8 *ptr = *pptr;
    uint frames = 0;

    while ( n-- > 0 )
    {
	ptr++;			// skip data field
	frames += *ptr++;	// add frame count
    }
    *pptr = ptr;

    // subtract 4 sec (0xfd frames) for countdown, then convert to msec
    frames = frames < 0xf0 ? 0 : frames - 0xf0;
    return frames * 100000 / 5994;
}

//-----------------------------------------------------------------------------

enumError ScanRawDataGHOST
(
    rkg_info_t		* ri,		// RKG info
    bool		init_ri,	// true: initialize 'ri' first
    struct raw_data_t	* raw		// valid raw data
)
{
    DASSERT(ri);
    DASSERT(raw);

    if (init_ri)
	InitializeRKGInfo(ri);
    else
	ResetRKGInfo(ri);

    if ( raw->data_size < sizeof(rkg_head_t) )
    {
	ri->error = "too small";
	return ERR_WARNING;
    }

    rkg_head_t *rh = (rkg_head_t*)raw->data;
    if ( memcmp(&rh->magic,RKG_MAGIC,4) )
    {
	ri->error = "wrong magic";
	return ERR_WARNING;
    }

    ri->score = ScanTimeRKG(rh->score);
    ri->is_compressed = ( ntohs(rh->type) & 0x0800 ) != 0;
    ri->data_size = ntohs(rh->data_size);

    const u8 * data = raw->data + sizeof(*rh);
    uint alloced_size = 0;
    if (ri->is_compressed)
    {
	uint alloced_size = ri->data_size;
	u8 *dest = CALLOC(alloced_size,1);
	size_t written, csize = be32(data);

	enumError err = DecompressYAZ( data+0x14, csize,
		dest, alloced_size, &written, raw->fname, 1, false, 0 );
	if (err)
	{
	    FREE(dest);
	    ri->error = "decompression failed";
	    return ERR_WARNING;
	}

	data = dest;
	ri->data_size = written;
	if ( verbose > 1 )
	    HexDump16(stdout,0,0,data,ri->data_size);
    }

    ri->n_but   = be16(data);
    ri->n_dir   = be16(data+2);
    ri->n_trick = be16(data+4);

    const u8 *ptr = data+8;
    ri->time_but   = CalcFrameTime(&ptr,ri->n_but);
    ri->time_dir   = CalcFrameTime(&ptr,ri->n_dir);
    ri->time_trick = CalcFrameTime(&ptr,ri->n_trick);

    if (alloced_size)
	FREE((char*)data);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


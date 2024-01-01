
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

#ifndef SZS_LIB_CHECKSUM_H
#define SZS_LIB_CHECKSUM_H 1

#define _GNU_SOURCE 1

#include "dclib-types.h"
#include "lib-szs-file.h"
#include "lib-ledis.h"
#include "crypt.h"

typedef struct szs_file_t szs_file_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct sha1_size_hash_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sha1_size_hash_t]]

#define CHECKSUM_DB_SIZE 32

///////////////////////////////////////////////////////////////////////////////

void CreateSS ( cvp data, uint size, sha1_size_hash_t *dest );
void CreateSS64 ( cvp data, uint size, sha1_size_b64_t dest );

// bufsize should be >40
void CreateSSChecksum		( char *buf, uint bufsize, const sha1_size_hash_t *ss );
void CreateSSChecksumBySZS	( char *buf, uint bufsize, const szs_file_t *szs );

// normed DB coding (independent of options)
void CreateSSChecksumDB		( char *buf, uint bufsize, const sha1_size_hash_t *ss );
void CreateSSChecksumDBByData	( char *buf, uint bufsize, cvp data, uint size );
void CreateSSChecksumDBBySZS	( char *buf, uint bufsize, const szs_file_t *szs );
void CreateSSXChecksumDBByData	( char *buf, uint bufsize, cvp data, uint size, file_format_t ff );
void CreateSSXChecksumDBBySZS	( char *buf, uint bufsize, const szs_file_t *szs );

enumError GetSSByFile ( sha1_size_hash_t *ss, ccp path1, ccp path2 );

// slen < 0 => strlen(source)
// returns: 0:fail, 1:SHA1, 2:DB64
// if res != NULL: scanned SHA1
int IsSSChecksum ( sha1_size_hash_t *res, ccp source, int slen );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs cache support		///////////////
///////////////////////////////////////////////////////////////////////////////

#define SZS_CACHE_FNAME "cache-content.txt"

extern ccp		szs_cache_dir;
extern ParamField_t	szs_cache;

///////////////////////////////////////////////////////////////////////////////

void LogCacheActivity ( ccp keyword, ccp format, ... )
	__attribute__ ((__format__(__printf__,2,3)));

static inline bool IsSZSCacheEnabled()
	{ return szs_cache_dir != 0; }

void SetupSZSCache ( ccp dir_name, bool use_dirname );

void ScanSZSCache ( ccp dir_name, bool purge );
enumError LoadSZSCache(void);
enumError SaveSZSCache ( bool force );
enumError AppendSZSCache(void);

ParamFieldItem_t * StoreSZSCache
(
    // return NULL or pointer to cache item

    ccp		fname,		// NULL or filename, only basename(fname) is used
    ccp		checksum,	// DB checksum; if NULL: load file and calc checksum
    bool	rename_file,	// true: rename existing file
    bool	*r_exists	// not NULL: store a status here
				//	=> true: file result->data exists
);

//-----------------------------------------------------------------------------
// [[check_cache_t]]

typedef struct check_cache_t
{
    file_format_t	fform;
    bool		found;		// true, if file found in cache
    char		csum[CHECKSUM_DB_SIZE+4];
					// checksum with suffix
    szs_file_t		cache;		// file loaded from cache, valid if found==true
}
check_cache_t;

void ResetCheckCache ( check_cache_t *cc );

bool CheckSZSCache
(
    // returns true if file found in cache (same as cc->found)

    check_cache_t	*cc,		// initialized by CheckSZSCache()
    szs_file_t		*szs,		// related SZS file
    cvp			data,		// data
    uint		size,		// size of 'data'
    file_format_t	fform,		// compression format
    ccp			ext		// wanted file extension including leading '.'
);

bool CheckSZSCacheSZS
(
    // returns true if file found in cache (same as cc->found)

    szs_file_t		*szs,		// related SZS file
    file_format_t	fform,		// compression format
    ccp			ext,		// wanted file extension including leading '.'
    bool		rm_uncompressed	// true: remove uncompressed data if cache is used
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_CHECKSUM_H

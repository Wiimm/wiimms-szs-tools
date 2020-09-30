
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
 *   Copyright (c) 2011-2020 by Dirk Clemens <wiimm@wiimm.de>              *
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

typedef struct szs_file_t szs_file_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct sha1_size_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sha1_size_t]]

#define CHECKSUM_DB_SIZE 32

///////////////////////////////////////////////////////////////////////////////

typedef struct sha1_size_t
{
    sha1_hash_t	hash;	    // SHA1 checksum (binary)
    u32		size;	    // size of date (big endian)
}
__attribute__ ((packed)) sha1_size_t;

///////////////////////////////////////////////////////////////////////////////

// bufsize should be >40
void CreateSSChecksum		( char *buf, uint bufsize, const sha1_size_t *ss );
void CreateSSChecksumBySZS	( char *buf, uint bufsize, const szs_file_t *szs );

// normed DB coding (independent of options)
void CreateSSChecksumDB		( char *buf, uint bufsize, const sha1_size_t *ss );
void CreateSSChecksumDBByData	( char *buf, uint bufsize, cvp data, uint size );
void CreateSSChecksumDBBySZS	( char *buf, uint bufsize, const szs_file_t *szs );

enumError GetSSByFile ( sha1_size_t *ss, ccp path1, ccp path2 );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    struct DistributionInfo_t		///////////////
///////////////////////////////////////////////////////////////////////////////

#define DISTRIB_MAGIC8			"#DISTRIB"
#define DISTRIB_MAGIC8_NUM		0x2344495354524942ull

#define DISTRIBUTION_FILE_VERSION	     1
#define MIN_DISTRIBUTION_SLOT		    10	// included
#define MAX_DISTRIBUTION_TRACK		 10000	// excluded
#define MAX_DISTRIBUTION_ARENA		  1000	// excluded
#define DISTRIBUTION_ARENA_DELTA	100000

#define DEFAULT_DISTIBUTION_FNAME	"distribution.txt"

//-----------------------------------------------------------------------------

static inline bool IsValidDistribArena ( uint slot )
{
    return slot == DISTRIBUTION_ARENA_DELTA
	|| slot >= DISTRIBUTION_ARENA_DELTA+MIN_DISTRIBUTION_SLOT
		&& slot < DISTRIBUTION_ARENA_DELTA+MAX_DISTRIBUTION_ARENA;
}

static inline bool IsValidDistribTrack ( uint slot )
{
    return !slot
	|| slot >= MIN_DISTRIBUTION_SLOT && slot < MAX_DISTRIBUTION_TRACK;
}

static inline bool IsValidDistribSlot ( uint slot )
{
    return !slot
	|| slot >= MIN_DISTRIBUTION_SLOT
		&& slot < MAX_DISTRIBUTION_TRACK
	|| slot == DISTRIBUTION_ARENA_DELTA
	|| slot >= DISTRIBUTION_ARENA_DELTA+MIN_DISTRIBUTION_SLOT
		&& slot < DISTRIBUTION_ARENA_DELTA+MAX_DISTRIBUTION_ARENA;
}

//-----------------------------------------------------------------------------
// [[DistributionInfo_t]]

typedef struct DistributionInfo_t
{
    ParamField_t	translate;
    ParamField_t	param;
    ParamField_t	arena[MAX_DISTRIBUTION_ARENA];
    ParamField_t	track[MAX_DISTRIBUTION_TRACK];
}
DistributionInfo_t;

//-----------------------------------------------------------------------------

void InitializeDistributionInfo
	( DistributionInfo_t * dinf, bool add_default_param );
void ResetDistributionInfo ( DistributionInfo_t *dinf );
void AddParamDistributionInfo ( DistributionInfo_t * dinf, bool overwrite );

enumError ScanDistribFile
	( DistributionInfo_t *dinf, ccp fname, int ignore, bool assume_arena );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot translation		///////////////
///////////////////////////////////////////////////////////////////////////////

char * ScanSlot ( uint *res_slot, ccp source, bool need_point );

void NormalizeSlotTranslation ( char *buf, uint bufsize, ccp name,
			char ** p_first_para, char ** p_first_brack );

uint DefineSlotTranslation
	( ParamField_t *translate, bool is_arena, uint slot, ccp name );

// 'sha1' can be NULL
int FindSlotByTranslation ( const ParamField_t *translate, ccp fname, ccp sha1 );

// 'param' not NULL: Scan "@PARAM = VALUE" lines and store result.
void ScanSlotTranslation
	( ParamField_t *translate, ParamField_t *param, ccp fname, bool ignore );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			szs cache support		///////////////
///////////////////////////////////////////////////////////////////////////////

#define SZS_CACHE_FNAME "cache-content.txt"

extern ccp		szs_cache_dir;
extern ParamField_t	szs_cache;

///////////////////////////////////////////////////////////////////////////////

static inline bool IsSZSCacheEnabled()
	{ return szs_cache_dir != 0; }

void SetupSZSCache ( ccp dir_name, bool use_dirname );

void ScanSZSCache ( ccp dir_name, bool purge );
enumError LoadSZSCache();
enumError SaveSZSCache();

ParamFieldItem_t * StoreSZSCache
(
    // return NULL or pointer to cache item

    ccp		fname,		// NULL or filename, only basename(fname) is used
    ccp		checksum,	// DB checksum; if NULL: load file and calc checksum
    bool	rename_file,	// true: rename existing file
    bool	*r_exists	// not NULL: store a status here
				//	=> true: file result->data exists
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_CHECKSUM_H

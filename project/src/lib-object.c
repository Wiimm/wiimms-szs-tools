
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

#include "lib-object.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			find helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint Mark
(
    const s16	*start,		// start of index field
    u8		*status,	// status vector
    uint	size,		// size of status vector (for assertions)
    u8		mask		// =0: add +1 / >0: OR mask
)
{
    DASSERT(start);
    DASSERT(status);

    const s16 *ptr = start;
    if (mask)
    	while ( *ptr >= 0 )
	{
	    DASSERT( *ptr < size );
	    noPRINT("MARK[%u] |= %02x\n",*ptr,mask);
	    status[*ptr++] |= mask;
	}
    else
    	while ( *ptr >= 0 )
	{
	    DASSERT( *ptr < size );
	    if (!++status[*ptr++])
		status[ptr[-1]]--;
	}

    return ptr - start;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Find DB files			///////////////
///////////////////////////////////////////////////////////////////////////////

int FindDbFile
(
    // returns the index of the file or -1 if not found

    ccp			file		// file to find, preeceding './' possible
)
{
    DASSERT(file);
    if ( file[0] == '.' && file[1] == '/' )
	file += 2;

    int beg = 0;
    int end = N_DB_FILE_FILE - 1;
    while ( beg <= end )
    {
	uint idx = (beg+end)/2;
	int stat = strcmp(file,DbFileFILE[idx].file);
	if ( stat < 0 )
	    end = idx - 1 ;
	else if ( stat > 0 )
	    beg = idx + 1;
	else
	    return idx;
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint FindDbFileByGroup
(
    // returns the number of found files

    UsedFileFILE_t	*status,	// modify a byte for each found file
    bool		init_status,	// clear 'status' first
    uint		group,		// index of group (robust)
    u8			mask		// =0: add +1 / >0: OR mask
)
{
    noPRINT("FindDbFileByGroup(,%d,%u) size=%zu\n",
		init_status, group, sizeof(*status) );

    DASSERT(status);
    if (init_status)
	memset(status,0,sizeof(*status));

    if ( group < N_DB_FILE_GROUP )
    {
	DASSERT( DbFileGROUP[group].ref >= 0 );
	return Mark( DbFileRefGROUP + DbFileGROUP[group].ref,
			status->d, N_DB_FILE_FILE, mask );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint FindDbFileByGroups
(
    // returns the number of found files

    UsedFileFILE_t		*status,	// modify a byte for each found file
    bool			init_status,	// clear 'status' first
    const UsedFileGROUP_t	*group		// NULL or list of used groups
)
{
    noPRINT("FindDbFileByGroups(,%d,)\n",init_status);

    DASSERT(status);
    if (init_status)
	memset(status,0,sizeof(*status));

    int count = 0;
    if (group)
    {
	int gi;
	for ( gi = 0; gi < N_DB_FILE_GROUP; gi++ )
	    if (group->d[gi])
	    {
		noPRINT("group[%u] = %u -> %d -> %d, %d, %d\n",gi,
			group->d[gi],DbFileGROUP[gi].ref,
			DbFileRefGROUP[DbFileGROUP[gi].ref+0],
			DbFileRefGROUP[DbFileGROUP[gi].ref+1],
			DbFileRefGROUP[DbFileGROUP[gi].ref+2]);

		count += Mark( DbFileRefGROUP + DbFileGROUP[gi].ref,
			    status->d, N_DB_FILE_FILE, group->d[gi] );
	    }
    }

    #if HAVE_PRINT0
    {
	int fi;
	for ( fi = 0; fi < N_DB_FILE_FILE; fi++ )
	    if (status->d[fi])
		printf(" -> FILE: %4u %02x %p\n",
			fi, status->d[fi], DbFileFILE[fi].file );
    }
    #endif

    return count;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint FindDbFileByObject
(
    // returns the number of found files

    UsedFileFILE_t	*status,	// modify a byte for each found file
    bool		init_status,	// clear 'status' first
    uint		object,		// index of object (robust)
    u8			mask		// =0: add +1 / >0: OR mask
)
{
    noPRINT("FindDbFileByObject(,%d,%u) size=%zu\n",
		init_status, object, sizeof(*status) );

    DASSERT(status);
    if (init_status)
	memset(status,0,sizeof(*status));

    UsedFileGROUP_t used_group;
    FindDbGroupByObject(&used_group,true,object,mask);
    return FindDbFileByGroups(status,init_status,&used_group);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Find DB groups			///////////////
///////////////////////////////////////////////////////////////////////////////

uint FindDbGroupByObject
(
    // returns the number of found groups

    UsedFileGROUP_t	*status,	// modify a byte for each found file
    bool		init_status,	// clear 'status' first
    uint		object,		// index of object (robust)
    u8			mask		// =0: add +1 / >0: OR mask
)
{
    noPRINT("FindDbGroupByObjects(,%d,)\n",init_status);

    DASSERT(status);
    if (init_status)
	memset(status,0,sizeof(*status));

    int count = 0;
    if ( object < N_KMP_GOBJ )
    {
	const ObjectInfo_t *oi = ObjectInfo + object;
	if ( oi->fname )
	{
	    int i;
	    for ( i = 0; i < sizeof(oi->group_idx)/sizeof(*oi->group_idx); i++ )
	    {
		uint fid = oi->group_idx[i];
		if ( fid < N_DB_FILE_GROUP )
		{
		    if (mask)
			status->d[fid] |= mask;
		    else if (!++status->d[fid])
			status->d[fid]--;
		    count++;
		}
	    }
	}
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////

uint FindDbGroupByObjects
(
    // returns the number of found groups

    UsedFileGROUP_t		*status,	// modify a byte for each found file
    bool			init_status,	// clear 'status' first
    const UsedObject_t		*object		// NULL or list of used objects
)
{
    noPRINT("FindDbGroupByObjects(,%d,)\n",init_status);

    DASSERT(status);
    if (init_status)
	memset(status,0,sizeof(*status));

    int count = 0;
    if (object)
    {
	int obj;
	const ObjectInfo_t *oi = ObjectInfo;
	for ( obj = 0; obj < N_KMP_GOBJ; obj++, oi++ )
	    if ( object->d[obj] && oi->fname )
	    {
		int i;
		for ( i = 0; i < sizeof(oi->group_idx)/sizeof(*oi->group_idx); i++ )
		{
		    uint fid = oi->group_idx[i];
		    if ( fid < N_DB_FILE_GROUP )
		    {
			status->d[fid] |= object->d[obj];
			count++;
		    }
		}
	    }
    }

    #if HAVE_PRINT0
    {
	int gi;
	for ( gi = 0; gi < N_DB_FILE_GROUP; gi++ )
	    if (status->d[gi])
		printf(" -> GROUP: %4u %02x %s\n",
			gi, status->d[gi], DbFileGROUP[gi].name );
    }
    #endif

    return count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			for external modules		///////////////
///////////////////////////////////////////////////////////////////////////////

void DefineObjectNameVars ( VarMap_t *vm )
{
    DASSERT(vm);

    char varname[50], *varname_end = varname + sizeof(varname)-1;
    const ObjectInfo_t *oi, *oi_end = ObjectInfo + N_KMP_GOBJ;
    int index = 0;
    for ( oi = ObjectInfo; oi < oi_end; oi++, index++ )
	if (oi->name)
	{
	    char * vn = varname;
	    *vn++ = 'O';
	    *vn++ = '$';
	    ccp src = oi->name;
	    while ( *src && vn < varname_end )
		*vn++ = toupper((int)*src++);
	    *vn = 0;

	    DefineIntVar(vm,varname,index);
	}
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


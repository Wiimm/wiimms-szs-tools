
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <stddef.h>

#include "lib-brres.h"
#include "lib-mdl.h"
#include "lib-bzip2.h"

#include "mdl.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL action log			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool MDL_ACTION_LOG ( bool is_patch, const char * format, ... )
	__attribute__ ((__format__(__printf__,2,3)));

static bool MDL_ACTION_LOG ( bool is_patch, const char * format, ... )
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[MDL]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

    if ( verbose > 2 || MDL_MODE & MDLMD_LOG || is_patch && ~MDL_MODE & MDLMD_SILENT )
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[MDL]%s ",colset->heading,colset->info);
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
///////////////			 mdl_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

mdl_mode_t MDL_MODE = MDLMD_M_DEFAULT;
int have_mdl_patch_count = 0;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_mdl_tab[] =
{
  { 0,			"CLEAR",	"RESET",	MDLMD_M_ALL | MDLMD_F_HIDE },
  { MDLMD_M_DEFAULT,	"DEFAULT",	0,		MDLMD_M_ALL | MDLMD_F_HIDE },

  { MDLMD_LOG,		"LOG",		0,		0 },
  { MDLMD_SILENT,	"SILENT",	0,		0 },

  { MDLMD_M_NODES,	"ALL-NODES",	"ALLNODES",	MDLMD_M_NODES },
  { MDLMD_PARENT_NODE,	"PARENT-NODE",	"PARENTNODES",	MDLMD_M_NODES },
  { MDLMD_CHILD_NODES,	"CHILD-NODES",	"CHILDNODES",	MDLMD_M_NODES },

  { MDLMD_VECTOR,	"VECTOR",	0,		0 },
  { MDLMD_MATRIX,	"MATRIX",	0,		0 },
  { MDLMD_VERTEX,	"VERTEX",	"VERTICES",	0 },

  { MDLMD_TEST0,	"TEST0",	"T0",		MDLMD_M_TEST | MDLMD_F_HIDE },
  { MDLMD_TEST1,	"TEST1",	"T1",		MDLMD_M_TEST },
  { MDLMD_TEST2,	"TEST2",	"T2",		MDLMD_M_TEST },
  { MDLMD_TEST3,	"TEST3",	"T3",		MDLMD_M_TEST },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

void LoadParametersMDL
(
    ccp		log_prefix		// not NULL:
					//    print log message with prefix
)
{
    static bool done = false;
    if (done)
	return;
    done = true;

    SetMdlMode(MDL_MODE);


    //--- logging

    if ( !MDL_ACTION_LOG(false,"Global MDL Modes: %s\n",GetMdlMode())
	&& log_prefix
	&& MDL_MODE != MDLMD_M_DEFAULT
	&& !(MDL_MODE & MDLMD_SILENT) )
    {
	fprintf(stdlog,"%sglobal mdl modes: %s [%x]\n",log_prefix,GetMdlMode(),MDL_MODE);
	fflush(stdlog);
    }
}

///////////////////////////////////////////////////////////////////////////////

mdl_mode_t GetRelevantMdlMode ( mdl_mode_t mode )
{
    mode = mode & MDLMD_M_ALL;

    if ( !( mode & (MDLMD_VECTOR|MDLMD_MATRIX) ))
	mode &= ~MDLMD_M_NODES;
    else if ( !( mode & MDLMD_M_NODES ))
	mode |= MDLMD_M_NODES;

    if ( PATCH_FILE_MODE & PFILE_F_LOG_ALL )
	mode |= MDLMD_LOG;

    return mode;
}

///////////////////////////////////////////////////////////////////////////////

void SetMdlMode ( uint new_mode )
{
    //--------------------------------------
    DASSERT(  0x40 == sizeof(mdl_head_t) );
    DASSERT(  0xd0 == sizeof(mdl_sect1_t) );
    DASSERT(  0x38 == sizeof(mdl_sect2_t) );
    DASSERT( 0x418 == sizeof(mdl_sect8_t) );
    DASSERT(  0x34 == sizeof(mdl_sect8_layer_t) );
    DASSERT(  0x68 == sizeof(mdl_sect10_t) );
    //--------------------------------------

    noPRINT("SET-MDL/IN:  PC=%d, MPC=%d\n",
		have_patch_count, have_mdl_patch_count );

    if (have_mdl_patch_count)
	have_patch_count--, have_mdl_patch_count--;

    if ( mdl_switched_to_vertex
	&& ( new_mode & (MDLMD_VECTOR|MDLMD_VERTEX) ) == MDLMD_VECTOR )
    {
	new_mode = new_mode & ~MDLMD_VECTOR | MDLMD_VERTEX;
	PRINT("MDL: VECTOR diabled, VERTEX enabled\n");
	if ( mdl_switched_to_vertex == 1 )
	    mdl_switched_to_vertex++;
    }

    MDL_MODE = GetRelevantMdlMode(new_mode);

    if ( MDL_MODE & MDLMD_M_PATCH )
	have_patch_count++, have_mdl_patch_count++;

    noPRINT("SET-MDL/OUT: PC=%d, MPC=%d\n",
		have_patch_count, have_mdl_patch_count );
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptMdl ( ccp arg )
{
    if (!arg)
	return 0;

    static bool done = false;
    if (!done)
    {
	done = true;
	if ( *arg != '+' && *arg != '-' )
	    MDL_MODE = 0;
    }

    s64 stat = ScanKeywordList(arg,opt_mdl_tab,0,true,0,MDL_MODE,
				"Option --mdl",ERR_SYNTAX);
    if ( stat != -1 )
    {
	SetMdlMode(stat);
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

uint PrintMdlMode ( char *buf, uint bufsize, mdl_mode_t mode )
{
    DASSERT(buf);
    DASSERT(bufsize>10);
    char *dest = buf;
    char *end = buf + bufsize - 1;

    mode = GetRelevantMdlMode(mode) | MDLMD_F_HIDE;
    mdl_mode_t mode1 = mode;

    const KeywordTab_t *ct;
    for ( ct = opt_mdl_tab; ct->name1 && dest < end; ct++ )
    {
	if ( ct->opt & MDLMD_F_HIDE )
	    continue;

	if ( ct->opt ? (mode & ct->opt) == ct->id : mode & ct->id )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = StringCopyE(dest,end,ct->name1);
	    mode &= ~(ct->id|ct->opt);
	}
    }

    if ( mode1 == (MDLMD_M_DEFAULT|MDLMD_F_HIDE) )
	dest = StringCopyE(dest,end," (default)");
    else if (!mode1)
	dest = StringCopyE(dest,end,"(none)");

    *dest = 0;
    return dest-buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetMdlMode()
{
    static char buf[100] = {0};
    if (!*buf)
	PrintMdlMode(buf,sizeof(buf),MDL_MODE);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

void SetupMDL()
{
 #if 0
    if ( opt_tiny > 0 )
    {
	const mdl_mode_t tiny
		= ( (mdl_mode_t)opt_tiny << MDLMD_S_TINY ) & MDLMD_M_TINY;
	if ( tiny > ( MDL_MODE & MDLMD_M_TINY ))
	    MDL_MODE = MDL_MODE & ~MDLMD_M_TINY | tiny;
    }
 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			general helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static void * GetVectorMDL
(
    float3		*dest,
    const void		*src,
    uint		format,
    uint		r_shift
)
{
    DASSERT(dest);
    DASSERT(src);

    switch (format)
    {
	case 0: // u8
	    dest->x = ldexpf(((u8*)src)[0],-r_shift);
	    dest->y = ldexpf(((u8*)src)[1],-r_shift);
	    dest->z = ldexpf(((u8*)src)[2],-r_shift);
	    return (u8*)src + 3;

	case 1: // s8
	    dest->x = ldexpf(((s8*)src)[0],-r_shift);
	    dest->y = ldexpf(((s8*)src)[1],-r_shift);
	    dest->z = ldexpf(((s8*)src)[2],-r_shift);
	    return (s8*)src + 3;

	case 2: // u16
	    dest->x = ldexpf(be16((u16*)src+0),-r_shift);
	    dest->y = ldexpf(be16((u16*)src+1),-r_shift);
	    dest->z = ldexpf(be16((u16*)src+2),-r_shift);
	    return (u16*)src + 3;

	case 3: // s16
	    dest->x = ldexpf((s16)be16((u16*)src+0),-r_shift);
	    dest->y = ldexpf((s16)be16((u16*)src+1),-r_shift);
	    dest->z = ldexpf((s16)be16((u16*)src+2),-r_shift);
	    return (s16*)src + 3;

	case 4: // float
	    bef4n(dest->v,src,3);
	    return (float*)src + 3;

	default:
	    dest->x = dest->y = dest->z = 0.0;
	    return (void*)src;
     }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline int float_to_mdl_int
(
    float	num,
    uint	r_shift,
    int		min,
    int		max
)
{
    //const int res = truncf( ldexpf(num,r_shift) + 0.5 );
    const int res = ldexpf(num,r_shift);
    return res < min ? min : res > max ? max : res;
}

//-----------------------------------------------------------------------------

static void * SetVectorMDL
(
    const float3	*src,
    void		*dest,
    uint		format,
    uint		r_shift
)
{
    DASSERT(dest);
    DASSERT(src);

    switch (format)
    {
	case 0: // u8
	    ((u8*)dest)[0] = float_to_mdl_int(src->x,r_shift,0,0xff);
	    ((u8*)dest)[1] = float_to_mdl_int(src->y,r_shift,0,0xff);
	    ((u8*)dest)[2] = float_to_mdl_int(src->z,r_shift,0,0xff);
	    return (u8*)dest + 3;

	case 1: // s8
	    ((s8*)dest)[0] = float_to_mdl_int(src->x,r_shift,-0x80,0x7f);
	    ((s8*)dest)[1] = float_to_mdl_int(src->y,r_shift,-0x80,0x7f);
	    ((s8*)dest)[2] = float_to_mdl_int(src->z,r_shift,-0x80,0x7f);
	    return (s8*)dest + 3;

	case 2: // u16
	    write_be16((u16*)dest+0,float_to_mdl_int(src->x,r_shift,0,0xffff));
	    write_be16((u16*)dest+1,float_to_mdl_int(src->y,r_shift,0,0xffff));
	    write_be16((u16*)dest+2,float_to_mdl_int(src->z,r_shift,0,0xffff));
	    return (u16*)dest + 3;

	case 3: // s16
	    write_be16((s16*)dest+0,float_to_mdl_int(src->x,r_shift,-0x8000,0x7fff));
	    write_be16((s16*)dest+1,float_to_mdl_int(src->y,r_shift,-0x8000,0x7fff));
	    write_be16((s16*)dest+2,float_to_mdl_int(src->z,r_shift,-0x8000,0x7fff));
	    return (s16*)dest + 3;

	case 4: // float
	    write_bef4n(dest,src->v,3);
	    return (float*)dest + 3;

	default:
	    return (void*)dest;
     }
}

///////////////////////////////////////////////////////////////////////////////

static const MemItem_t * GetMemItemMDLs1
(
    const mdl_t		*mdl,		// pointer to valid MDL
    const MemItem_t	*eptr,		// pointer to section data
    s32			offset		// offset to neigbour section 1
)
{
    DASSERT(mdl);
    DASSERT(eptr);

    if (!offset)
	return 0;

    const MemItem_t *ebase = GetMemListElem(&mdl->elem,0,0);
    const MemItem_t *ptr, *eend  = ebase + mdl->elem.used;

 #if USE_NEW_CONTAINER_MDL
    if (InContainerP(&mdl->container,eptr->data))
 #else
    if (InDataContainer(mdl->old_container,eptr->data))
 #endif
    {
	const u8 * search = eptr->data + offset;
	for ( ptr = ebase; ptr < eend; ptr++ )
	    if ( ptr->idx1 == 1 && ptr->data == search )
	    {
		noPRINT("MDLs1 found by address: %u + %x = %u\n",
			eptr->idx2, offset, ptr->idx2 );
		return ptr;
	    }
    }

    int idx = offset / (int)sizeof(mdl_sect1_t);
    if ( idx * (int)sizeof(mdl_sect1_t) == offset )
    {
	idx += eptr->idx2;
	for ( ptr = ebase; ptr < eend; ptr++ )
	    if ( ptr->idx1 == 1 && ptr->idx2 == idx )
	    {
		noPRINT("MDLs1 found by index: %u + %x = %u\n",
			    eptr->idx2, offset, ptr->idx2 );
		return ptr;
	    }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetNameMDLs1
(
    const mdl_t		*mdl,		// pointer to valid MDL
    const MemItem_t	*eptr,		// pointer to section data
    s32			offset,		// offset to neigbour section 1
    ccp			if_not_found	// return this, if not found
)
{
    DASSERT(mdl);
    DASSERT(eptr);

    eptr = GetMemItemMDLs1(mdl,eptr,offset);
    return eptr ? eptr->name : if_not_found;
}

///////////////////////////////////////////////////////////////////////////////

static MemItem_t * FindSectionbyTypeMDL
(
    const mdl_t		*mdl,		// pointer to valid MDL
    uint		idx		// index of section to find
)
{
    DASSERT(mdl);

    MemItem_t *ptr = GetMemListElem(&mdl->elem,0,0);
    MemItem_t *end = ptr + mdl->elem.used;

    for ( ; ptr < end; ptr++ )
	if ( ptr->idx1 == idx )
	    return ptr;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static MemItem_t * FindSectionbyPtrMDL
(
    const mdl_t		*mdl,		// valid MDL
    const u8		*sptr,		// pointer to search
    int			sect		// >=0: search section type only
)
{
    MemItem_t *ptr = GetMemListElem(&mdl->elem,0,0);
    MemItem_t *end = ptr + mdl->elem.used;

    for ( ; ptr < end; ptr++ )
	if ( ptr->data == sptr )
	    return sect < 0 || sect == ptr->idx1 ? ptr : 0;
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL0 string iterator		///////////////
///////////////////////////////////////////////////////////////////////////////

static void scan_sections
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    MemItem_t			*mi	// memory item of section
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(mi);

    switch(mi->idx1)
    {
     //case 1:
	// offsets: 0x5c..: parent, child1, next, prev, part2

     //case 8:
	// offsets: 0x28=shader, 0x30=layer, 0x34=part2,
	//	    0x38=display_8_9, 0x3c=display_10_11,

    //case 10:
	// offsets: 0x20=vertex_decl, 0x2c=vertex_data, 0x64=bone

     case 11:
	{
	    MemItem_t *m8_base = GetMemListElem(&sit->mdl->elem,0,0);
	    MemItem_t *m8_ptr, *m8_end = m8_base + sit->mdl->elem.used;

	    uint n = ( mi->size - 4 ) / 8;
	    if ( n > 0 )
	    {
		uint max = be32(mi->data);
		if ( n > max )
		    n = max;
		u32 *sptr = GetStrIdxMIL(mi,2*n);
		u32 *src = (u32*)mi->data + 1;
		for ( ; n-- > 0; sptr += 2, src += 2 )
		{
		    sptr[0] = sptr[1] = 1; // default to "-"
		    const u8* ptr1 = mi->data + be32(src);
		    for ( m8_ptr = m8_base; m8_ptr < m8_end; m8_ptr++ )
			if ( m8_ptr->idx1 == 8 && m8_ptr->data == ptr1 )
			{
			    const u32 *m8_sptr = GetStrIdxListMIL(m8_ptr);
			    sptr[0] = m8_sptr[0];

			    const u8* ptr2 = mi->data + be32(src+1);
			    if ( ptr2 < m8_ptr->data + m8_ptr->size )
			    {
				const u8 *layer0 = m8_ptr->data + be32(m8_ptr->data+0x30);
				const uint layer_index = ( ptr2 - layer0 ) / 0x34;
				//printf("%p %p => 0x%06zx => %d\n",ptr2,layer0,ptr2-layer0,layer_index);
				if ( layer0 + 0x34 * layer_index == ptr2 )
				    sptr[1] = m8_sptr[layer_index+1];
			    }
			    break;
			}
		}
	    }
	}
	break;
    }
}

//-----------------------------------------------------------------------------

void scan_strings
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u32				*sptr,	// pointer to string index
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(sit->sp);
    DASSERT(sptr);
    DASSERT(base);
    DASSERT(data);

 #if USE_NEW_CONTAINER_MDL
    if (InContainerP(&sit->mdl->container,data))
 #else
    if (InDataContainer(sit->mdl->old_container,data))
 #endif
    {
	ccp str = (ccp)base + ntohl(*data);
     #if USE_NEW_CONTAINER_MDL
	if (InContainerP(&sit->mdl->container,str))
     #else
	if (InDataContainer(sit->mdl->old_container,str))
     #endif
	{
	    noPRINT("%p %p+%06x = %p |%s|\n",base,data,ntohl(*data),str,str);
	    *sptr = InsertStringPool(sit->sp,str,false,str);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void string_collect
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u32				*sptr,	// pointer to string index
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(sit->sp);
    DASSERT(base);
    DASSERT(data);

    const uint idx = be32(data);
    if ( idx < sit->mdl->spool.n2s_size )
    {
	ccp str = sit->mdl->spool.n2s_list[idx];
	if (str)
	    InsertStringPool(sit->sp,str,false,0);
    }
}

//-----------------------------------------------------------------------------

void string_export
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u32				*sptr,	// pointer to string index
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(sit->sp);
    DASSERT(sptr);
    DASSERT(base);
    DASSERT(data);

    u32 res = 0;
    const uint idx = *sptr;
    if ( idx < sit->mdl->spool.n2s_size )
    {
	ccp str = sit->mdl->spool.n2s_list[idx];
	if (str)
	    res = FindStringPool(sit->sp,str,0) + sit->mdl->raw_data - base;
	    noPRINT("-> %d/%d : %zx %x %s\n",
			idx, sit->sp->n2s_size, res + base - sit->mdl->raw_data, res, str );
    }
    write_be32(data,res);
}

//-----------------------------------------------------------------------------

void offset_export
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(sit->sp);
    DASSERT(base);
    DASSERT(data);

    write_be32(data,sit->mdl->raw_data-base);
}

//-----------------------------------------------------------------------------

void string_copy
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    u32				*sptr,	// pointer to string index
    u8				*base,	// base of item
    u32				*data	// pointer to string reference
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(sit->sp);
    DASSERT(sptr);
    DASSERT(base);
    DASSERT(data);

    u32 res = 0;
    const uint idx = *sptr;
    if ( idx < sit->mdl->spool.n2s_size )
    {
	ccp str = sit->mdl->spool.n2s_list[idx];
	if (str)
	    res = InsertStringPool(sit->sp,str,false,0);
    }
    *sptr = res;
}

///////////////////////////////////////////////////////////////////////////////

void IterateStringsMDL
(
    StringIteratorMDL_t	*sit,		// string iterator data
    MemItem_t		*mi,		// pointer to memlist
    int			mi_count	// number of elements of 'ml_ptr'
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);
    DASSERT(sit->sp);
    DASSERT(mi);

    // => GetStrIdxMIL() must be called for every access, because data may be moved!

    for ( ; mi_count > 0; mi_count--, mi++ )
    {
	u8 *data = (u8*)mi->data;
	if (!data)
	    continue;

	noPRINT("--- grp=%d, idx=%d, size=%u\n",mi->idx1,mi->idx2,mi->size);

	if (sit->sect_func)
	    sit->sect_func(sit,mi);

	if ( sit->off_func && mi->idx1 >= 1 && mi->idx1 <= 10 )
	    sit->off_func(sit,data,(u32*)(data+0x04));

	switch(mi->idx1) // group
	{
	 case 1:
	    if (sit->str_func)
		sit->str_func(sit,GetStrIdxMIL(mi,1),data,(u32*)(data+0x08));
	    break;

	 case 2:
	 case 3:
	 case 4:
	 case 5:
	    if (sit->str_func)
		sit->str_func(sit,GetStrIdxMIL(mi,1),data,(u32*)(data+0x0c));
	    break;

	 case 8:
	    if (sit->str_func)
	    {
		const uint n_layer = be32(data+0x2c);
		sit->str_func(sit,GetStrIdxMIL(mi,n_layer+1),data,(u32*)(data+0x08));
		noPRINT("[%d] %p %x\n",count,data,be32(data+0x08));
		u32 layer_off = be32(data+0x30);
		uint i;
		for ( i = 0; i < n_layer; i++, layer_off += 0x34 )
		{
		    u8 * d = data + layer_off;
		    sit->str_func(sit,GetStrIdxMIL(mi,0)+i+1,d,(u32*)d);
		}
	    }
	    break;

	 case 10:
	    if (sit->str_func)
		sit->str_func(sit,GetStrIdxMIL(mi,1),data,(u32*)(data+0x38));
	    break;
	}
    }
}

//-----------------------------------------------------------------------------

void IterateAllStringsMDL
(
    StringIteratorMDL_t	*sit		// string iterator data
)
{
    DASSERT(sit);
    DASSERT(sit->mdl);

    MemItem_t *mi = GetMemListElem(&sit->mdl->elem,0,0);
    IterateStringsMDL(sit,mi,sit->mdl->elem.used);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL0 iterator			///////////////
///////////////////////////////////////////////////////////////////////////////

static int mdl_iter_func
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

    it->brsub_version	= bcut->version;
    it->group		= grp;
    it->entry		= entry;

    noPRINT("MDL( %2d, %2d, %s )\n",grp,entry,it->path);

    switch (grp)
    {
      case GROUP_IDX_SUB_HEADER:
	{
	    const mdl_head_t *head = (mdl_head_t*)( bcut->data + it->off );
	    it->size = sizeof(*head);
	    it->entry = 0;
	    //int stat = IterateFilesFuncBRSUB(bcut,grp,entry,name);
	    int stat = it->func_it(it,false);
	    it->index++;

	    //--- MDL bone table

	    u32 off = be32(&head->bone_offset);
	    if ( off && !stat )
	    {
		u8 * d = (u8*)head + off;
		it->off	  = d - bcut->data;
		it->size  = be32(d) * 4 + 4;
		it->entry = 1;
		snprintf(it->path,sizeof(it->path),"%s.bone-table.bin",bcut->id);
		stat = it->func_it(it,false);
		it->index++;
	    }
	    return stat;
	}

      case 0:
	{
	    mdl_sect0_t ms0;
	    InitializeMDLs0(&ms0,bcut->data+it->off,bcut->size-it->off);
	    it->size = ms0.data_size;
	}
	break;

      case 11:
	if (entry>=0)
	    it->size = it->endian->rd32(bcut->data+it->off) * 8 + 4;
	break;
    }

    const int stat = it->func_it(it,false);
    it->index++;
    return stat;
}

//-----------------------------------------------------------------------------

static ccp mdl_names[] =
{
	".draw-list",	//  0
	".bones",	//  1
	".verticies",	//  2
	".normals",	//  3
	".colors",	//  4
	".t-coord",	//  5
	"",		//  6
	"",		//  7
	".materials",	//  8
	".mat-nodes",	//  9
	".objects",	// 10
	".t-links",	// 11
	0
};

static ccp GetSectNameMDL ( uint index )
{
    ccp res = index < sizeof(mdl_names)/sizeof(*mdl_names)
		? mdl_names[index]
		: 0;
    if ( !res || !*res )
    {
	const int bufsize = 12;
	char *buf = GetCircBuf(bufsize);
	snprintf(buf,bufsize,".%u",index);
	res = buf;
    }
    return res;
}

//-----------------------------------------------------------------------------

int IterateFilesMDL
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    return CutFilesBRSUB( it, mdl_iter_func, 0,
		mdl_names, sizeof(mdl_names) / sizeof(*mdl_names) -1 );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL0 base interface		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeMDL ( mdl_t * mdl )
{
    DASSERT(mdl);
    memset(mdl,0,sizeof(*mdl));

    mdl->fname = EmptyString;
    InitializeMIL(&mdl->elem);
    InitializeStringPool(&mdl->spool);

    LoadParametersMDL( verbose > 0 ? "  - " : 0 );

    // [[2do]] [[mdl]] ???
}

///////////////////////////////////////////////////////////////////////////////

void ResetMDL ( mdl_t * mdl )
{
    DASSERT(mdl);
    FREE(mdl->bone_tab);
    FreeString(mdl->fname);
    FreeString(mdl->name);
    ResetMIL(&mdl->elem);
    ResetStringPool(&mdl->spool);
 #if USE_NEW_CONTAINER_MDL
    ResetContainer(&mdl->container);
 #else
    FreeContainer(mdl->old_container);
 #endif
    InitializeMDL(mdl);
}

///////////////////////////////////////////////////////////////////////////////

ccp GetStringMDL
(
    mdl_t		* mdl,		// MDL data structure
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid	// result if invalid
)
{
    DASSERT(mdl);
    DASSERT(base);

    if (offset)
    {
	ccp string = (ccp)base + offset;
 #if USE_NEW_CONTAINER_MDL
	if (InContainerP(&mdl->container,string))
 #else
	if (InDataContainer(mdl->old_container,string))
 #endif
	{
	    //HEXDUMP16(0,0,string,32);
	    return string;
	}
    }

    return if_invalid;
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetStringDataMDL
(
    mdl_t		*mdl,		// MDL data structure
    const void		*data,		// pointer to data
    int			group,		// group index
    ccp			if_invalid	// result if invalid
)
{
    DASSERT(mdl);
    DASSERT(data);

 #if USE_NEW_CONTAINER_MDL
    if (IsValidContainer(&mdl->container))
 #else
    if (mdl->old_container)
 #endif
    {
	switch (group)
	{
	    case 1:
	    {
		const mdl_sect1_t *ms = (mdl_sect1_t*)data;
		const u32 name_off = ntohl(ms->name_off);
		return GetStringMDL(mdl,data,name_off,if_invalid);
	    }

	    case 2:
	    {
		const mdl_sect2_t *ms = (mdl_sect2_t*)data;
		const u32 name_off = ntohl(ms->name_off);
		return GetStringMDL(mdl,data,name_off,if_invalid);
	    }

	    case 8:
	    {
		const mdl_sect8_t *ms = (mdl_sect8_t*)data;
		const u32 name_off = ntohl(ms->name_off);
		return GetStringMDL(mdl,data,name_off,if_invalid);
	    }

	    case 10:
	    {
		const mdl_sect10_t *ms = (mdl_sect10_t*)data;
		const u32 name_off = ntohl(ms->name_off);
		return GetStringMDL(mdl,data,name_off,if_invalid);
	    }

	    // [[newsect]]
	}
    }

    return if_invalid;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetStringItMDL
(
    struct
	szs_iterator_t	*it,		// iterator struct with all infos
    bool		*res_alloced	// result => true: name is alloced
)
{
    DASSERT(it);
    DASSERT(res_alloced);
    DASSERT(it->szs);
    mdl_t *mdl = it->param;
    DASSERT(mdl);

    *res_alloced = false;


    //--- first try 'it->name'

 #if USE_NEW_CONTAINER_MDL
    if (InContainerP(&mdl->container,it->name))
 #else
    if (InDataContainer(mdl->old_container,it->name))
 #endif
    {
	noPRINT("GetStringItMDL() LINK NAME => %s\n",it->name);
	return it->name;
    }


    //--- analyse well known name offsets

    const u8 *data = it->szs->data + it->off;
    ccp res = GetStringDataMDL(mdl,data,it->group,0);


    //--- should never happen: make a copy of it->name

    if ( !res && it->name )
    {
	noPRINT("!!!!!!!!!! GetStringItMDL() => |%.30s|\n",it->name);
	res = STRDUP(it->name);
	*res_alloced = true;
    }

    return res;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			setup mdl parser		///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsMDL()
{
    static VarMap_t vm = {0};
    if (!vm.used)
    {
 #if 0
	//--- setup integer variables

	struct inttab_t { ccp name; int val; };
	static const struct inttab_t inttab[] =
	{
	    {0,0}
	};

	const struct inttab_t * ip;
	for ( ip = inttab; ip->name; ip++ )
	    DefineIntVar(&vm,ip->name,ip->val);
 #endif

	//--- setup basic parser variables

	DefineParserVars(&vm);
    }

    return &vm;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Scan MDL sections		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanSetupMDL
(
    mdl_t		* mdl,		// MDL data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(mdl);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    ScanParam_t ptab[] =
    {
	{ "MDL-NAME",		SPM_STRING,	&mdl->name },
	{ "MDL-VERSION",	SPM_NONE,	0 },
	{ "N-SECTIONS",		SPM_NONE,	0 },
	{0}
    };

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	// ignore all others
	GotoEolSI(si);
    }
    CheckLevelSI(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanHexMDL
(
    mdl_t		* mdl,		// MDL data structure
    mdl_section_id	sect,		// MDL section
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(mdl);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    char *name = 0;
    uint sort_index = 0;
    ScanParam_t ptab[] =
    {
	{ "NAME",		SPM_STRING,	&name },
	{ "SORT-INDEX",		SPM_UINT,	&sort_index },
	{0}
    };

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	if ( ch == '$' )
	{
	    GotoEolSI(si);
	    continue;
	}

	ScanHexlineSI(si,fb,true);
    }
    CheckLevelSI(si);

    printf("NAME: %s\nSORT: %u\n",name,sort_index);
    HexDump16(stdout,0,0,fb->buf,fb->ptr-fb->buf);

    FreeString(name);
    ResetFastBuf(fb);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t mdl_section_name[] =
{
    { MDL_IGNORE,	"END",		"--",		0 },
    { MDL_SETUP,	"SETUP",	"Setup",	0 },
    { MDL_SETUP,	"BONE-TABLE",	"Bone Table",	0 },

    { MDL_SECTION_0,	"SECTION-0",	"Section 0",	0 },
    { MDL_SECTION_1,	"SECTION-1",	"Section 1",	0 },
    { MDL_SECTION_2,	"SECTION-2",	"Section 2",	0 },
    { MDL_SECTION_3,	"SECTION-3",	"Section 3",	0 },
    { MDL_SECTION_4,	"SECTION-4",	"Section 4",	0 },
    { MDL_SECTION_5,	"SECTION-5",	"Section 5",	0 },
    { MDL_SECTION_6,	"SECTION-6",	"Section 6",	0 },
    { MDL_SECTION_7,	"SECTION-7",	"Section 7",	0 },
    { MDL_SECTION_8,	"SECTION-8",	"Section 8",	0 },
    { MDL_SECTION_9,	"SECTION-9",	"Section 9",	0 },
    { MDL_SECTION_10,	"SECTION-10",	"Section 10",	0 },
    { MDL_SECTION_11,	"SECTION-11",	"Section 11",	0 },


    { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    PRINT("ScanTextMDL(init=%d)\n",init_mdl);

    DASSERT(mdl);
    if (init_mdl)
	InitializeMDL(mdl);

    mdl->version = 11;
    mdl->n_sect  = 14;

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,mdl->fname,mdl->revision);
    si.predef = SetupVarsMDL();

    enumError max_err = ERR_OK;
    mdl->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN MDL SECTIONS, PASS%u ...\n",mdl->is_pass2+1);

	max_err = ERR_OK;
	si.mdl = mdl;
	si.no_warn = !mdl->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", mdl->is_pass2+1 );

	for(;;)
	{
	    char ch = NextCharSI(&si,true);
	    if (!ch)
		break;

	    if ( ch != '[' )
	    {
		NextLineSI(&si,true,false);
		continue;
	    }
	    ResetLocalVarsSI(&si,mdl->revision);

	    si.cur_file->ptr++;
	    char sect_name[20];
	    ScanNameSI(&si,sect_name,sizeof(sect_name),true,true,0);
	    PRINT0("--> pass=%u: #%04u: %s\n",mdl->is_pass2+1,si.cur_file->line,sect_name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,sect_name,mdl_section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    PRINT0("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
	     case MDL_IGNORE:
		// ignore it without warning
		break;

	     case MDL_SETUP:
		ScanSetupMDL(mdl,&si);
		break;

	     case MDL_BONE_TABLE:
	     case MDL_SECTION_0:
	     case MDL_SECTION_1:
	     case MDL_SECTION_2:
	     case MDL_SECTION_3:
	     case MDL_SECTION_4:
	     case MDL_SECTION_5:
	     case MDL_SECTION_6:
	     case MDL_SECTION_7:
	     case MDL_SECTION_8:
	     case MDL_SECTION_9:
	     case MDL_SECTION_10:
	     case MDL_SECTION_11:
		ScanHexMDL(mdl,cmd->id,&si);
		break;

	      default:
		err = ERROR0(ERR_WARNING,"Unknown section (ignored): %s\n",sect_name);
		break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (mdl->is_pass2)
	    break;

	mdl->is_pass2 = true;
	RestartSI(&si);
    }

    ResetSI(&si);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static int scan_raw_mdl_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    mdl_t *mdl = it->param;
    DASSERT(mdl);

    noPRINT("--> %3d,%3d term=%u size=%4u : %s\n",
	it->group, it->entry, term, it->size, it->path );
    if ( term || it->size <= 0 )
	return 0;

    const u8 *data = it->szs->data + it->off;
    if ( it->group == GROUP_IDX_SUB_HEADER )
    {
	if ( it->entry == 0 && it->size >= sizeof(mdl_head_t) )
	{
	    const mdl_head_t *hsrc = (mdl_head_t*)data;
	    mdl_head_t *hdest = &mdl->head;
	    be32n(&hdest->head_len,&hsrc->head_len,10);
	    bef4n(hdest->min,hsrc->min,6);
	}
	else if ( it->entry == 1 && it->size >= 4 )
	{
	    const int max = (it->size-4) / sizeof(*mdl->bone_tab);
	    uint n = be32(data);
	    PRINT("BONE TABLE: N=%u,%u\n",n,max);
	    if ( n > max )
		 n = max;
	    if ( n > 0 )
	    {
		mdl->bone_n = n;
		FREE(mdl->bone_tab);
		const u32 *src = (u32*)(data+4);
		u32 *dest = mdl->bone_tab = MALLOC( n * sizeof(*mdl->bone_tab) );
		while ( n-- > 0 )
		    *dest++ = be32(src++);
		DASSERT( (u8*)src <= data + it->size );
	    }
	}
    }
    else if ( it->entry >= 0 )
    {
	MemItem_t * item = AppendMIL(&mdl->elem);
	DASSERT(item);
	item->idx1 = it->group;
	item->idx2 = it->entry;
	item->size = it->size;
 #if USE_NEW_CONTAINER_MDL
	if (InContainerP(&mdl->container,data))
 #else
	if (InDataContainer(mdl->old_container,data))
 #endif
	    item->data = (u8*)data;
	else
	{
	    item->data = MEMDUP(data,item->size);
	    item->data_alloced = true;
	}

	item->name = GetStringItMDL(it,&item->name_alloced);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerMDL_t	* cdata		// NULL or container-data
)
{
    DASSERT(mdl);

 #ifdef TEST
    SaveFile("_mdl-1.tmp",0,FM_OVERWRITE,data,data_size,0);
 #endif

 #if USE_NEW_CONTAINER_MDL // ---------------------------------------

    if (init_mdl)
	InitializeMDL(mdl);

    CatchContainerData(&mdl->container,0,cdata);
    AssignContainer(&mdl->container,0,data,data_size,CPM_COPY);
 #ifdef TEST
    DumpInfoContainer(stdlog,collog,2,"MDL: ",&mdl->container,0);
 #endif

    if (InContainerP(&mdl->container,data))
	mdl->mdl_offset = (u8*)data - mdl->container.cdata->data;

 #else // !USE_NEW_CONTAINER_MDL // ---------------------------------

    if (init_mdl)
	InitializeMDL(mdl);
    else
	FreeContainer(mdl->old_container);

    mdl->old_container = DupContainer(cdata,data,data_size,true,&data);
    PRINT("CONTAINER: %p,0x%x -> %p,0x%x [%d]\n",
	    data, data_size,
	    mdl->old_container->data, mdl->old_container->size,
	    mdl->old_container->ref_count );

    if (InDataContainer(cdata,data))
	mdl->mdl_offset = (u8*)data - cdata->data;

 #endif // !USE_NEW_CONTAINER_MDL // --------------------------------

    const endian_func_t	* endian = &be_func;
    if ( IsValidMDL(data,data_size,data_size,0,mdl->fname,endian) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid MDL file: %s\n", mdl->fname ? mdl->fname : "?");
    }


    //--- scan MDL base infos

    mdl->version = endian->rd32(data+8);
    mdl->n_sect  = GetSectionNumBRSUB(data,data_size,endian);


    //--- get MDL name

    const uint off = offsetof(brsub_header_t,grp_offset) + mdl->n_sect * 4;
    const uint name_off = be32(data+off);
    ccp name = GetStringBRSUB(data,data+data_size,off,data,name_off,0,0);
    if (name)
	mdl->name = STRDUP(name);


    //--- scan MDL header & sections

    cut_iter_func[FF_MDL] = IterateFilesMDL; // enable MDL support
// [[fname+]]
    IterateFilesData(data,data_size,scan_raw_mdl_func,mdl,FF_MDL,mdl->fname);
    SortMIL(&mdl->elem,false);


    //--- string handling

    ResetStringPool(&mdl->spool);
    InsertStringPool(&mdl->spool,"-",false,0);

    StringIteratorMDL_t sim;
    memset(&sim,0,sizeof(sim));
    sim.mdl		= mdl;
    sim.sp		= &mdl->spool;
    sim.sect_func	= scan_sections;
    sim.str_func	= scan_strings;
    IterateAllStringsMDL(&sim);

    #ifdef TEST0
    {
	uint i;
	for ( i = 0; i < mdl->spool.n2s_size; i++ )
	{
	    ccp str = mdl->spool.n2s_list[i];
	    if (str)
		PRINT("SP %2u: %s\n",i,str);
	}
    }
    #endif

 #ifdef TEST
    SaveFile("_mdl-2.tmp",0,FM_OVERWRITE,data,data_size,0);
 #endif
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			iterate MDL sub files		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct iter_helper_mdl_t
{
    struct raw_data_t	* raw;		// valid raw data
    CheckMode_t		check_mode;	// not NULL: call CheckMDL(mode)
    iterate_mdl_func	func;		// function to call
    void		*param;		// user defined parameter
    enumError		max_err;	// max error code

} iter_helper_mdl_t;

//-----------------------------------------------------------------------------

static int iter_raw_data_mdl
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if ( term || it->is_dir )
	return 0;

    DASSERT(it->szs);
    const u8 *data = it->szs->data + it->off;
// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,it->size,it->size);
    if ( fform != FF_MDL && fform != FF_MDL_TXT )
	return 0;

    DASSERT(it->param);
    iter_helper_mdl_t *iparam = it->param;
    DASSERT(iparam->raw);
    DASSERT(iparam->func);

    mdl_t mdl;
    InitializeMDL(&mdl);
    mdl.fname = STRDUP3(iparam->raw->fname,"/",it->path);

 #if USE_NEW_CONTAINER_MDL
    enumError err = ScanMDL(&mdl,false,data,it->size,
				LinkContainerRawData(iparam->raw),iparam->check_mode);
 #else
    enumError err = ScanMDL(&mdl,false,data,it->size,
				ContainerRawData(iparam->raw),iparam->check_mode);
 #endif
    if ( err < ERR_WARNING )
	err = iparam->func(&mdl,iparam->param);
    if ( iparam->max_err < err )
	 iparam->max_err = err;
    ResetMDL(&mdl);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError IterateRawDataMDL
(
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		check_mode,	// not NULL: call CheckMDL(mode)
    iterate_mdl_func	func,		// call this function for each found MDL
    void		* param		// a user defined parameter
)
{
    DASSERT(raw);
    DASSERT(func);

    if (   raw->fform == FF_MDL
	|| raw->fform == FF_MDL_TXT
	|| !IsArchiveFF(raw->fform) )
    {
	mdl_t mdl;
	enumError err = ScanRawDataMDL(&mdl,true,raw,check_mode);
	if ( err < ERR_WARNING )
	    err = func(&mdl,param);
	ResetMDL(&mdl);
	return err;
    }

    iter_helper_mdl_t iparam;
    iparam.raw		= raw;
    iparam.check_mode	= check_mode;
    iparam.func		= func;
    iparam.param	= param;
    iparam.max_err	= ERR_OK;

    szs_file_t szs;
// [[fname+]]
    AssignSZS(&szs,true,raw->data,raw->data_size,false,raw->fform,raw->fname);
    IterateFilesParSZS(&szs,iter_raw_data_mdl,&iparam,false,false,-1,-1,SORT_NONE);
    ResetSZS(&szs);
    return iparam.max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL0 scan + load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerMDL_t	* cdata,	// NULL or container-data
    CheckMode_t		mode		// not NULL: call CheckMDL(mode)
)
{
    DASSERT(mdl);
    PRINT("ScanMDL(mode=%x) size=%u\n",mode,data_size);

    enumError err;
// [[analyse-magic]]
    switch (GetByMagicFF(data,data_size,data_size))
    {
	case FF_MDL:
	    mdl->fform = FF_MDL;
	    if ( disable_patch_on_load > 0 || !HavePatchMDL() )
		err = ScanRawMDL(mdl,init_mdl,data,data_size,cdata);
	    else
	    {
		//dc = 0;
		u8 *temp = MEMDUP(data,data_size);
		PatchRawDataMDL(temp,data_size,0,mdl->fname);
		err = ScanRawMDL(mdl,init_mdl,temp,data_size,0);
		FREE(temp);
	    }
	    break;

	case FF_MDL_TXT:
	 #if USE_NEW_CONTAINER_MDL
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    mdl->fform = FF_MDL_TXT;
	    err =  ScanTextMDL(mdl,init_mdl,data,data_size);
	    break;

	default:
	 #if USE_NEW_CONTAINER_MDL
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    if (init_mdl)
		InitializeMDL(mdl);
	    return ERROR0(ERR_INVALID_DATA,
		"No MDL file: %s\n", mdl->fname ? mdl->fname : "?");
    }

    if ( err <= ERR_WARNING && mode )
	CheckMDL(mdl,mode);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckMDL(mode)
)
{
    DASSERT(mdl);
    DASSERT(raw);
    if (init_mdl)
	InitializeMDL(mdl);
    else
	ResetMDL(mdl);

    mdl->fatt  = raw->fatt;
    mdl->fname = raw->fname;
    raw->fname = 0;
 #if USE_NEW_CONTAINER_MDL
    return ScanMDL(mdl,false,raw->data,raw->data_size,LinkContainerRawData(raw),mode);
 #else
    return ScanMDL(mdl,false,raw->data,raw->data_size,ContainerRawData(raw),mode);
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadMDL
(
    mdl_t		* mdl,		// MDL data structure
    bool		init_mdl,	// true: initialize 'mdl' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckMDL(mode)
)
{
    DASSERT(mdl);
    DASSERT(fname);
    if (init_mdl)
	InitializeMDL(mdl);
    else
	ResetMDL(mdl);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	mdl->fname = raw.fname;
	raw.fname = 0;
     #if USE_NEW_CONTAINER_MDL
	err = ScanMDL(mdl,false,raw.data,raw.data_size,LinkContainerRawData(&raw),mode);
     #else
	err = ScanMDL(mdl,false,raw.data,raw.data_size,ContainerRawData(&raw),mode);
     #endif
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateRawMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateRawMDL
(
    mdl_t		* mdl		// pointer to valid MDL
)
{
    DASSERT(mdl);
    TRACE("CreateRawMDL()\n");

    const uint MAX_SECT		= 20;	// max supported sections


    //--- setup string pool

    string_pool_t sp;
    InitializeStringPool(&sp);

    char fname_buf[200];
    ccp mdl_name = 0;
    if (mdl->name)
    {
	mdl_name = mdl->name;
	InsertStringPool(&sp,mdl_name,false,0);
    }
    else if (mdl->fname)
    {
	mdl_name = strrchr(mdl->fname,'/');
	mdl_name = mdl_name ? mdl_name+1 : mdl->fname;
	uint cut = 0;
	const int len = strlen(mdl_name);
	if ( len > 4 && (  !strcasecmp(mdl_name+len-4,".mdl")
			|| !strcasecmp(mdl_name+len-4,".bin") ))
	    cut = len-4;
	else if ( len > 5 && !strcasecmp(mdl_name+len-5,".mdl0") )
	    cut = len-5;

	if (cut)
	{
	    StringCopySM(fname_buf,sizeof(fname_buf),mdl_name,cut);
	    mdl_name = fname_buf;
	}
	InsertStringPool(&sp,mdl_name,false,0);
    }


    //--- version and sections

    uint sect_count[MAX_SECT];
    memset(sect_count,0,sizeof(sect_count));

    int version = mdl->version;
    int n_sect = GetKnownSectionNumBRSUB(FF_MDL,version);
    if ( n_sect <= 0 )
    {
	version = 11;
	n_sect = GetKnownSectionNumBRSUB(FF_MDL,version);
	ASSERT(n_sect>0);
    }
    const int max_sect = n_sect < MAX_SECT ? n_sect : MAX_SECT;


    //--- calculate data size (section data)

    SortMIL(&mdl->elem,true);
    MemItem_t *mi_base = GetMemListElem(&mdl->elem,0,0);
    MemItem_t *mi_ptr, *mi_prev, *mi_end = mi_base + mdl->elem.used;

    uint data_size = 0;
    for ( mi_ptr = mi_base, mi_prev = 0; mi_ptr < mi_end; mi_ptr++ )
    {
	u8 *data = (u8*)mi_ptr->data;
	if ( !data || mi_ptr->idx1 >= max_sect )
	    continue;

	if ( !mi_prev || mi_ptr->size != mi_prev->size
		|| memcmp(mi_ptr->data,mi_prev->data,mi_ptr->size) )
	{
	    data_size += mi_ptr->size;
	}
	sect_count[mi_ptr->idx1]++;

	InsertStringPool(&sp,mi_ptr->name,false,0);
	mi_prev = mi_ptr;
    }


    //--- calculate header size

    const uint head_offset = 0x14 + 4*n_sect;
    const uint bone_offset = head_offset + sizeof(mdl_head_t);
    const uint header_size = bone_offset
			   + 4 + mdl->bone_n * sizeof(*mdl->bone_tab);

    uint sect, brsub_n = 0, brsub_size = 0;
    for ( sect = 0; sect < max_sect; sect++ )
    {
	const uint n = sect_count[sect];
	if (n)
	{
	    brsub_n += 1 + n; // root + N
	    brsub_size += 0x18 + n*0x10;
	}
    }

    PRINT("NS=%d, header_size=0x%x, brsub_n/size=%u,0x%x\n",
		n_sect, header_size, brsub_n, brsub_size );


    //--- create string pool

    const uint string_offset = header_size + brsub_size + data_size;

    StringIteratorMDL_t sim;
    memset(&sim,0,sizeof(sim));
    sim.mdl	 = mdl;
    sim.sp	 = &sp;
    sim.str_func = string_collect;
    IterateAllStringsMDL(&sim);
    CalcStringPool(&sp,string_offset,&be_func);
    const uint file_size = string_offset + sp.size;

    PRINT("CREATE MDL: size = %#x = %u\n",data_size,data_size);
    MDL_ACTION_LOG(false,"CreateRawMDL() size=%u\n",data_size);


    //--- alloc data

    FREE(mdl->raw_data);
    u8 * mdata		= CALLOC(1,file_size);
    mdl->raw_data	= mdata;
    mdl->raw_data_size	= string_offset;
    mdl->raw_file_size	= file_size;


    //--- setup header + string pool

    memcpy(mdata,MDL_MAGIC,4);
    write_be32(mdata+0x04,string_offset);
    write_be32(mdata+0x08,version);
    if (mdl_name)
	write_be32(mdata+0x10+n_sect*4,FindStringPool(&sp,mdl_name,0));
    memcpy(mdata+string_offset,sp.data,sp.size);
    write_be32n((u32*)(mdata+head_offset),(u32*)&mdl->head,16);


    //--- copy bones

    u32 *ptr = (u32*)(mdata + bone_offset);
    write_be32(ptr++,mdl->bone_n);
    uint i;
    for ( i = 0; i < mdl->bone_n; i++ )
	write_be32(ptr++,mdl->bone_tab[i]);


    //--- prepare entries

    uint sect_info_idx[MAX_SECT];
    memset(sect_info_idx,0,sizeof(sect_info_idx));

    brres_info_t *info = CALLOC(brsub_n,sizeof(*info));
    brres_info_t *info_ptr = info;

    u32 offset = header_size;
    for ( sect = 0; sect < max_sect; sect++ )
    {
	const uint n = sect_count[sect];
	if (n)
	{
	    info_ptr->id = 0xffff;
	    info_ptr++;
	    sect_info_idx[sect] = info_ptr - info;
	    info_ptr += n;
	}
    }
    //HEXDUMP16(0,0,sect_info_idx,sizeof(sect_info_idx));


    //--- copy section data

    memset(&sim,0,sizeof(sim));
    sim.mdl	 = mdl;
    sim.sp	 = &sp;
    sim.off_func = offset_export;
    sim.str_func = string_export;

    offset = header_size + brsub_size;
    for ( mi_ptr = mi_base, mi_prev = 0; mi_ptr < mi_end; mi_ptr++ )
    {
	noPRINT("SECT-%d.%d, off=0x%x, size=0x%x\n",
		mi_ptr->idx1, mi_ptr->idx2, offset, mi_ptr->size );
	u8 *data = (u8*)mi_ptr->data;
	if ( !data || mi_ptr->idx1 >= max_sect )
	    continue;

	if ( mi_prev && mi_ptr->size == mi_prev->size
		&& !memcmp(mi_ptr->data,mi_prev->data,mi_ptr->size) )
	{
	    offset -= mi_prev->size;
	}

	brres_info_t *info_ptr = info + sect_info_idx[mi_ptr->idx1]++;
	info_ptr->data_off = offset;
	info_ptr->name = mi_ptr->name ? mi_ptr->name : "";
	info_ptr->nlen = strlen(info_ptr->name);
	info_ptr->name_off = FindStringPool(&sp,info_ptr->name,0);

	mi_ptr->data = mdata+offset; // temporary
	memcpy(mi_ptr->data,data,mi_ptr->size);
	IterateStringsMDL(&sim,mi_ptr,1);
	mi_ptr->data = data;

	offset += mi_ptr->size;
	DASSERT( offset <= string_offset );
	mi_prev = mi_ptr;
    }
    DASSERT( offset == string_offset );
    //HEXDUMP16(0,0,sect_info_idx,sizeof(sect_info_idx));

    #ifdef TEST0
    {
	uint i;
	for ( i = 0; i < brsub_n; i++ )
	{
	    brres_info_t *bi = info + i;
	    printf("\t%04x %04x %04x : %06x %06x : [%2d] |%s|\n",
		bi->id, bi->left_idx, bi->right_idx,
		bi->name_off, bi->data_off,
		bi->nlen, bi->name );
	}
    }
    #endif


    //--- finish brsub header

    offset = header_size;
    for ( info_ptr = info, sect = 0; sect < max_sect; sect++ )
    {
	uint n = sect_count[sect];
	if (n)
	{
	    u32 base_offset = offset;
	    write_be32(mdata+0x10+4*sect,offset);
	    write_be32(mdata+offset,(n+1)*0x10+8);
	    offset += 4;
	    write_be32(mdata+offset,n);
	    offset += 4;

	    uint i;
	    for ( i = 1; i <= n; i++ ) // without root
		CalcEntryBRRES(info_ptr,i);

	    for ( i = 0; i <= n; i++ ) // with root
	    {
		brres_entry_t *entry_ptr = (brres_entry_t*)(mdata+offset);
		write_be16( &entry_ptr->id,        info_ptr->id );
		write_be16( &entry_ptr->left_idx,  info_ptr->left_idx );
		write_be16( &entry_ptr->right_idx, info_ptr->right_idx );
		if (info_ptr->data_off)
		    write_be32( &entry_ptr->data_off,  info_ptr->data_off - base_offset );
		if (info_ptr->name_off)
		    write_be32( &entry_ptr->name_off,  info_ptr->name_off - base_offset );
		offset += 0x10;
		info_ptr++;
	    }
	}
    }


    //--- terminate

    ResetStringPool(&sp);
    FREE(info);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawMDL
(
    mdl_t		* mdl,		// pointer to valid MDL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(mdl);
    DASSERT(fname);
    PRINT("SaveRawMDL(%s,%d)\n",fname,set_time);
    MDL_ACTION_LOG(false,"SaveRawMDL() %s\n",fname);


    //--- create raw data

    enumError err = CreateRawMDL(mdl);
    if (err)
	return err;
    DASSERT(mdl->raw_data);
    DASSERT(mdl->raw_file_size);


    //--- write to file

    File_t F;
    err = CreateFileOpt(&F,true,fname,testmode,mdl->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&mdl->fatt,0);

    if ( fwrite(mdl->raw_data,1,mdl->raw_file_size,F.f) != mdl->raw_file_size )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTextMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextMDLhex
(
    FILE		* f,		// file to print to
    const mdl_t		* mdl,		// pointer to valid MDL
    const MemItem_t	* eptr		// pointer to section data
)
{
    DASSERT(f);
    DASSERT(mdl);
    DASSERT(eptr);

    fputs(text_mdl_hexdump_cr,f);
    hexdump_eol = "\r\n";
    HexDump(f,0,0,5,16,eptr->data,eptr->size);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextMDLs0
(
    FILE		* f,		// file to print to
    const mdl_t		* mdl,		// pointer to valid MDL
    const MemItem_t	* eptr		// pointer to section data
)
{
    DASSERT(f);
    DASSERT(mdl);
    DASSERT(eptr);

    if (!brief_count)
	fprintf(f,text_mdl_s0_cr);
    fprintf(f,text_mdl_s0_tabhead_cr);

    mdl_sect0_t ms0;
    InitializeMDLs0(&ms0,eptr->data,eptr->size);
    while (NextMDLs0(&ms0))
    {
	ccp typestr = GetTypeMDLs0(*ms0.cur);
	if (ms0.sub_size)
	{
	    DASSERT(ms0.sub_n>0);
	    DASSERT(ms0.sub_ptr);
	    DASSERT(ms0.sub_ptr > ms0.cur);
	    PrintNumF(f,ms0.cur,ms0.sub_ptr-ms0.cur,typestr);

	    typestr = strchr(typestr,'|');
	    if (typestr)
		typestr++;

	    int i;
	    for ( i = 0; i < ms0.sub_n; i++ )
	    {
		fputs("\r\n\t> ",f);
		PrintNumF(f,ms0.sub_ptr + i*ms0.sub_size,ms0.sub_size,typestr);
	    }
	}
	else
	    PrintNumF(f,ms0.cur,ms0.cur_size,typestr);
	fputs("\r\n",f);
    }

    if (ms0.fail)
    {
	fputs("$RAW",f);
	PrintNumF(f,ms0.fail,ms0.cur_size,0);
	fputs("\r\n",f);
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextMDLs1
(
    FILE		* f,		// file to print to
    const mdl_t		* mdl,		// pointer to valid MDL
    const MemItem_t	* eptr		// pointer to section data
)
{
    DASSERT(f);
    DASSERT(mdl);
    DASSERT(eptr);

    mdl_sect1_t ms;
    ntoh_MDLs1(&ms,(mdl_sect1_t*)eptr->data);
    if ( eptr->size != sizeof(ms) || ms.length != sizeof(ms) )
	return SaveTextMDLhex(f,mdl,eptr);

    fprintf(f,
       "length		= %#11x\r\n"
       "mdl0-offset	= %#11x\r\n"
//     "name-offset	= %#11x\r\n"
       "index		= %#11x\r\n"
       "id		= %#11x\r\n"
       "flags		= %#11x\r\n"
       "padding		= %#11x %#11x\r\n"
       "\r\n"
       "parent		= %#11x  \"%s\"\r\n"
       "child		= %#11x  \"%s\"\r\n"
       "next		= %#11x  \"%s\"\r\n"
       "prev		= %#11x  \"%s\"\r\n"
       "part2		= %#11x  \"%s\"\r\n"
       "\r\n"

	,ms.length
	,ms.mdl0_off
//	,ms.name_off
	,ms.index
	,ms.id
	,ms.flags
	,ms.padding[0] ,ms.padding[1]

	,ms.parent_off	,GetNameMDLs1(mdl,eptr,ms.parent_off,"")
	,ms.child_off	,GetNameMDLs1(mdl,eptr,ms.child_off,"")
	,ms.next_off	,GetNameMDLs1(mdl,eptr,ms.next_off,"")
	,ms.prev_off	,GetNameMDLs1(mdl,eptr,ms.prev_off,"")
	,ms.part2_off	,GetNameMDLs1(mdl,eptr,ms.part2_off,"")
	);

    PrintMatrixMDLs1(f,0,"\r\n",&ms,~0);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextMDLs2
(
    FILE		* f,		// file to print to
    const mdl_t		* mdl,		// pointer to valid MDL
    const MemItem_t	* eptr		// pointer to section data
)
{
    DASSERT(f);
    DASSERT(mdl);
    DASSERT(eptr);

    mdl_sect2_t ms;
    ntoh_MDLs2(&ms,(mdl_sect2_t*)eptr->data);
    // if ( eptr->size != sizeof(ms) || ms.length != sizeof(ms) )
    if ( ms.length < sizeof(ms) )
	return SaveTextMDLhex(f,mdl,eptr);

    //--- print base data

    fprintf(f,
       "length		= %#11x\r\n"
       "mdl0-offset	= %#11x\r\n"
       "data-offset	= %#11x\r\n"
//     "name-offset	= %#11x\r\n"
       "index		= %#11x\r\n"
       "has-bounds	= %11u\r\n"
       "format		= %11u\r\n"
       "r_shift		= %11u\r\n"
       "stride		= %11u\r\n"
       "n_vertex	= %#11x = %6u\r\n"
       "minimum		= %11.3f %11.3f %11.3f\r\n"
       "maximum		= %11.3f %11.3f %11.3f\r\n"
       "\r\n"
	,ms.length
	,ms.mdl0_off
	,ms.data_off
//	,ms.name_off
	,ms.index
	,ms.has_bounds
	,ms.format
	,ms.r_shift
	,ms.stride
	,ms.n_vertex ,ms.n_vertex
	,ms.minimum.x ,ms.minimum.y ,ms.minimum.z
	,ms.maximum.x ,ms.maximum.y ,ms.maximum.z
	);


    //--- print vertex table

    const void *data    = eptr->data + ms.data_off;
    uint n_vertex	= ms.n_vertex;
    noPRINT("N-VERTEX=%u, FORMAT=%u, R-SHIFT=%u\n",n_vertex,ms.format,ms.r_shift);

    while ( n_vertex-- > 0 )
    {
	float3 temp;
	data = GetVectorMDL(&temp,data,ms.format,ms.r_shift);
	fprintf(f,"v %11.3f %11.3f %11.3f\r\n", temp.x, temp.y, temp.z );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SaveTextMDLs10
(
    FILE		* f,		// file to print to
    const mdl_t		* mdl,		// pointer to valid MDL
    const MemItem_t	* eptr		// pointer to section data
)
{
    DASSERT(f);
    DASSERT(mdl);
    DASSERT(eptr);

    mdl_sect10_t ms;
    ntoh_MDLs10(&ms,(mdl_sect10_t*)eptr->data);
    if ( eptr->size < ms.length || ms.length < sizeof(ms) )
	return SaveTextMDLhex(f,mdl,eptr);

    //--- print base data

    fprintf(f,
       "length		= %#10x\r\n"
       "mdl0_off	= %#10x\r\n"
       "bone_idx	= %#10x\r\n"
       "cp_vtx		= %#10x\r\n"
       "cp_tex		= %#10x\r\n"
       "xf_nor_spec	= %#10x\r\n"
       "v_decl_size	= %#10x\r\n"
       "flags		= %#10x\r\n"
       "v_decl_offset	= %#10x\r\n"
       "v_data_size1	= %#10x\r\n"
       "v_data_size2	= %#10x\r\n"
       "v_data_offset	= %#10x\r\n"
       "xf_flags	= %#10x\r\n"
       "unknown_0x34	= %#10x\r\n"
       "name_off	= %#10x\r\n"
       "idx		= %#10x = %6u\r\n"
       "n_vertex	= %#10x = %6u\r\n"
       "n_face		= %#10x = %6u\r\n"
       "v_group		= %#10x = %6u\r\n"
       "n_group		= %#10x = %6u\r\n"
       "col_groups	= %#10x %#06x\r\n"
       "tex_groups	= %#10x %#06x %#06x %#06x  %#06x %#06x %#06x %#06x\r\n"
       "unknown_0x60	= %#10x\r\n"
       "bone_offset	= %#10x\r\n"
	,ms.length
	,ms.mdl0_off
	,ms.bone_idx
	,ms.cp_vtx
	,ms.cp_tex
	,ms.xf_nor_spec
	,ms.v_decl_size
	,ms.flags
	,ms.v_decl_offset
	,ms.v_data_size1
	,ms.v_data_size2
	,ms.v_data_offset
	,ms.xf_flags
	,ms.unknown_0x34
	,ms.name_off
	,ms.idx ,ms.idx
	,ms.n_vertex ,ms.n_vertex
	,ms.n_face ,ms.n_face
	,ms.v_group ,ms.v_group
	,ms.n_group ,ms.n_group
	,ms.col_group[0] ,ms.col_group[1]
	,ms.tex_group[0] ,ms.tex_group[1] ,ms.tex_group[2] ,ms.tex_group[3]
	,ms.tex_group[4] ,ms.tex_group[5] ,ms.tex_group[6] ,ms.tex_group[7]
	,ms.unknown_0x60
	,ms.bone_offset
	);


    //--- additonally data

    u32 bone_off = ms.bone_offset;
    u32 v_decl_offset = ms.v_decl_offset + 0x20;
    u32 v_data_offset = ms.v_data_offset + 0x24;


    //--- bones

    u32 end = eptr->size;
    if ( v_decl_offset > bone_off && end > v_decl_offset )
	end = v_decl_offset;
    if ( v_data_offset > bone_off && end > v_data_offset )
	end = v_data_offset;

    if ( bone_off < end )
    {
	fprintf(f,"\r\n# bones\r\n\r\n");
	hexdump_prefix = " $HEX ";
	hexdump_eol = "\r\n";
	HexDump(f,0,bone_off,5,16,eptr->data+bone_off,end-bone_off);
    }


    //--- vertex declaration

    end = eptr->size;
    if ( bone_off > v_decl_offset && end > bone_off )
	end = bone_off;
    if ( v_data_offset > v_decl_offset && end > v_data_offset )
	end = v_data_offset;

    if ( v_decl_offset < end )
    {
	fprintf(f,"\r\n# vertex declaration\r\n\r\n");
	hexdump_prefix = " $HEX ";
	hexdump_eol = "\r\n";
	HexDump(f,0,v_decl_offset,5,16,eptr->data+v_decl_offset,end-v_decl_offset);
    }


    //--- vertex data

    end = eptr->size;
    if ( bone_off > v_data_offset && end > bone_off )
	end = bone_off;
    if ( v_decl_offset > v_data_offset && end > v_decl_offset )
	end = v_decl_offset;

    if ( v_data_offset < end )
    {
     #ifdef TEST0
	fprintf(f,"\r\n# vertex data\r\n");
	mdl_sect2_t ms2;
	const u8 *s2data = 0;
	const MemItem_t *s2 = FindSectionbyTypeMDL(mdl,2);
	if (s2)
	{
	    ntoh_MDLs2(&ms2,(mdl_sect2_t*)s2->data);
	    s2data = s2->data + ms2.data_off;
	}

	u8 *ptr = eptr->data + v_data_offset;
	u8 *endptr = eptr->data + end;

	uint i_face = 0;
	while ( ptr+3 < endptr && *ptr )
	{
	    uint max = (endptr-ptr-3)/4;
	    uint n = be16(ptr+1);
	    if ( n > max )
		n = max;
	    fprintf(f,"\r\n  # face %u, mode = %#04x, n = %u\r\n",i_face++,*ptr,n);
	    ptr += 3;

	    if (s2data)
	    {
		for ( ; n > 0; n--, ptr += 4 )
		{
		    float3 temp;
		    uint vidx = be16(ptr);
		    if ( vidx < ms2.n_vertex )
		    {
			GetVectorMDL(&temp, s2data + vidx * ms2.stride,
					ms2.format, ms2.r_shift );
		    }
		    else
			temp.x = temp.y = temp.z = NAN;

		    fprintf(f,"\t%11.3f %11.3f %11.3f  %#06x\r\n",
				temp.x, temp.y, temp.z, be16(ptr+2) );
		}
	    }
	    else
	    {
		for ( ; n > 0; n--, ptr += 4 )
		    fprintf(f,"\t%5u %#06x\r\n",be16(ptr),be16(ptr+2));
	    }
	}
	fprintf(f,"\r\n# vertex data as hexdump\r\n\r\n");
     #else
	fprintf(f,"\r\n# vertex data\r\n\r\n");
     #endif

	hexdump_prefix = " $HEX ";
	hexdump_eol = "\r\n";
	HexDump(f,0,v_data_offset,5,16,eptr->data+v_data_offset,end-v_data_offset);
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextMDL
(
    const mdl_t		* mdl,		// pointer to valid MDL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(mdl);
    DASSERT(fname);
    PRINT("SaveTextMDL(%s,%d)\n",fname,set_time);


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,mdl->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&mdl->fatt,0);


    //--- print file header + syntax info

    if ( print_header && !brief_count )
	fprintf(F.f,text_mdl_file_head_cr);
    else
	fprintf(F.f,"%s\r\n",MDL_TEXT_MAGIC);


    //--- print info section

    if (brief_count)
    {
	fprintf(F.f,
		"%s[SETUP]\r\n\r\n"
		"TOOL     = %s\r\n"
		"SYSTEM2  = %s\r\n"
		"VERSION  = %s\r\n"
		"REVISION = %u\r\n"
		"DATE     = %s\r\n"
		"\r\n"
		"MDL-NAME    = \"%s\"\r\n"
		"MDL-VERSION = %2u\r\n"
		"N-SECTIONS  = %2u\r\n"
		"\r\n",
		section_sep,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		mdl->name ? mdl->name : "",
		mdl->version, mdl->n_sect );
    }
    else
    {
	fprintf(F.f, text_mdl_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		mdl->name ? mdl->name : "",
		mdl->version, mdl->n_sect );
    }

 #if 0 // [[2do]] [[mdl]] ???
    if (print_param)
    {
	if (!brief_count)
	    fwrite(text_mdl_param1_cr,1,sizeof(text_mdl_param1_cr)-1,F.f);
	else
	    fprintf(F.f,"#%.78s\r\n# Parameter support\r\n",Minus300);

	fwrite(text_mdl_param2_cr,1,sizeof(text_mdl_param2_cr)-1,F.f);
	if (!brief_count)
	    fwrite(text_mdl_param3_cr,1,sizeof(text_mdl_param3_cr)-1,F.f);
    }
 #endif


    //--- write MDL header

    if (brief_count)
	fprintf(F.f,
		"%s[HEADER]\r\n"
		"@REVISION = %u\r\n"
		"\r\n",
		section_sep,
		REVISION_NUM );
    else
	fprintf(F.f,text_mdl_header_cr,REVISION_NUM);

    const mdl_head_t *h = &mdl->head;
    fprintf(F.f,
		"HEAD-LEN    = %#10x\r\n"
		"OFFSET      = %#10x\r\n"
		"UNKNOWN-08  = %#10x\r\n"
		"UNKNOWN-0c  = %#10x\r\n"
		"N-VERTEX    = %10u\r\n"
		"N-FACE      = %10u\r\n"
		"UNKNOWN-18  = %#10x\r\n"
		"N-BONE      = %10u\r\n"
		"UNKNOWN-20  = %#10x\r\n"
		"UNKNOWN-08  = %#10x\r\n"
		"BONE-OFFSET = %#10x\r\n"
		"MIN         = %11.3f %11.3f %11.3f\r\n"
		"MAX         = %11.3f %11.3f %11.3f\r\n"
		"\r\n",
		h->head_len,
		h->offset,
		h->unknown_0x08,
		h->unknown_0x0c,
		h->n_vertex,
		h->n_face,
		h->unknown_0x18,
		h->unknown_0x0c,
		h->n_bone,
		h->unknown_0x0c,
		h->bone_offset,
		h->min[0], h->min[1], h->min[2],
		h->max[0], h->max[1], h->max[2] );


    //--- write bone table

// [[2do]]
//    if (brief_count)
	fprintf(F.f,
		"%s[BONE-TABLE]\r\n"
		"@REVISION = %u\r\n"
		"\r\n",
		section_sep,
		REVISION_NUM );
//    else
//	fprintf(F.f,text_mdl_header_cr,REVISION_NUM);

    fprintf(F.f,"# N = %2u\r\n",mdl->bone_n);
    int i;
    for ( i = 0; i < mdl->bone_n; i++ )
	fprintf(F.f,"%8d\r\n",mdl->bone_tab[i]);


    //--- write MDL sections

    const MemItem_t *eptr = GetMemListElem(&mdl->elem,0,0);
    const MemItem_t *eend = eptr + mdl->elem.used;
    for ( ; err < ERR_WARNING && eptr < eend; eptr++ )
    {
	fprintf(F.f,
		"%s[SECTION-%u]\r\n"
		"# MDL%s.%02u",
		section_sep, eptr->idx1,
		GetSectNameMDL(eptr->idx1), eptr->idx2
		);

     #if USE_NEW_CONTAINER_MDL
	if (InContainerP(&mdl->container,eptr->data))
	{
	    u8* base = mdl->container.cdata->data;
	    // dont use "%zx" because of __APPLE__
	    if ( mdl->mdl_offset )
		fprintf(F.f,", MDL file offset 0x%x, BRRES file offset 0x%x",
			(int)( eptr->data - base - mdl->mdl_offset ),
			(int)( eptr->data - base ));
	    else
		fprintf(F.f,", MDL file offset 0x%x",
			(int)( eptr->data - base ));
	}
     #else
	if (InDataContainer(mdl->old_container,eptr->data))
	{
	    // dont use "%zx" because of __APPLE__
	    if ( mdl->mdl_offset )
		fprintf(F.f,", MDL file offset 0x%x, BRRES file offset 0x%x",
			(int)( eptr->data - mdl->old_container->data - mdl->mdl_offset ),
			(int)( eptr->data - mdl->old_container->data ));
	    else
		fprintf(F.f,", MDL file offset 0x%x",
			(int)( eptr->data - mdl->old_container->data ));
	}
     #endif

	fprintf(F.f,
		"\r\n"
		"@REVISION    = %5u\r\n"
		"@ENTRY-INDEX = %5u\r\n"
		"@SORT-INDEX  = %5u\r\n"
		"@NAME        = \"%s\"\r\n"
		"\r\n",
		REVISION_NUM,
		eptr->idx2,
		eptr->sort_idx,
		eptr->name );

	switch(eptr->idx1) // [[newsect]]
	{
//	    case  0:  err = SaveTextMDLs0 (F.f,mdl,eptr); break;
	    case  1:  err = SaveTextMDLs1 (F.f,mdl,eptr); break;
	    case  2:  err = SaveTextMDLs2 (F.f,mdl,eptr); break;
//	    case 10:  err = SaveTextMDLs10(F.f,mdl,eptr); break;
	    default:  err = SaveTextMDLhex(F.f,mdl,eptr); break;
	}
    }


    //--- terminate

    fputs(section_end,F.f);
    ResetFile(&F,set_time);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    CkeckMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

int CheckMDL
(
    // returns number of errors

    const mdl_t		* mdl,		// pointer to valid MDL
    CheckMode_t		mode		// print mode
)
{
    // [[2do]] [[mdl]] ???
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 0			///////////////
///////////////////////////////////////////////////////////////////////////////

bool InitializeMDLs0 ( mdl_sect0_t *ms, const void * data, uint data_size )
{
    DASSERT(ms);
    DASSERT(data);

    memset(ms,0,sizeof(*ms));
    ms->data_beg  = data;
    ms->data_end  = ms->data_beg + data_size;
    ms->ok	  = true;

    while (NextMDLs0(ms))
    {
	ms->n++;
	ms->data_size += ms->cur_size;
    }
    ClearMDLs0(ms);

    return ms->ok;
}

///////////////////////////////////////////////////////////////////////////////

bool ClearMDLs0 ( mdl_sect0_t *ms )
{
    DASSERT(ms);
    ms->cur = ms->fail = 0;
    ms->cur_size = 0;
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool CheckMDLs0 ( mdl_sect0_t *ms )
{
    DASSERT(ms);
    ms->fail = 0;
    ms->sub_size = 0;
    if (!ms->cur)
	return 0;

    if ( ms->cur < ms->data_beg || ms->cur >= ms->data_end )
    {
	ms->fail = ms->cur;
	ms->cur = 0;
	ms->cur_size = 0;
	return ms->ok = false;
    }

    switch (*ms->cur)
    {
	case 1: // END (code)
	    ms->cur_size = 1;
	    break;

	case 2: // bone mapping (code,u16,u16)
	case 5: // indexing (code,u16,u16)
	    ms->cur_size = 5;
	    break;

	case 3: // weighting (code,u16,N=u8,6*N)
	    ms->sub_size = 6;
	    ms->sub_ptr  = ms->cur + 4;
	    ms->sub_n    = ms->cur[3];
	    ms->cur_size = 4 + 6 * ms->sub_n;
	    break;

	case 4: // draw polygon (code,u16,u16,u16,u8)
	    ms->cur_size = 8;
	    break;

	default:
	    ms->cur_size = 0; // == FAIL
	    break;
    }

    if ( !ms->cur_size || ms->cur + ms->cur_size > ms->data_end )
    {
	ms->fail = ms->cur;
	ms->cur_size =  ms->data_end - ms->cur;
	ms->cur = 0;
	return ms->ok = false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool FirstMDLs0 ( mdl_sect0_t *ms )
{
    DASSERT(ms);
    ms->cur = ms->data_beg;
    return CheckMDLs0(ms);
}

///////////////////////////////////////////////////////////////////////////////

bool NextMDLs0 ( mdl_sect0_t *ms )
{
    if (!ms->cur)
	return FirstMDLs0(ms);

    if ( *ms->cur == 1 )
	return ClearMDLs0(ms);

    ms->cur += ms->cur_size;
    return CheckMDLs0(ms);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ccp MDLs0_types[] =
{
    0,
    "u1",		// 1
    "u1u2u2",		// 2
    "u1u2u1|u2f4",	// 3
    "u1u2u2u2x1",	// 4
    "u1u2u2"		// 5
};

//-----------------------------------------------------------------------------

ccp GetTypeMDLs0 ( uint index )
{
    return index < sizeof(MDLs0_types)/sizeof(*MDLs0_types)
	? MDLs0_types[index]
	: 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 1			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetMDLs1 ( mdl_sect1_t *ms, bool network_order )
{
    DASSERT(ms);
    memset(ms,0,sizeof(*ms));

    ms->length	= sizeof(*ms);
    ms->flags	= MINIMAP_DEFAULT_FLAGS;
    ms->scale.x = ms->scale.y = ms->scale.z = 1.0;

    if (network_order)
	hton_MDLs1(ms,ms);
}

///////////////////////////////////////////////////////////////////////////////

void ClearMDLs1 ( mdl_sect1_t *ms, bool network_order )
{
    DASSERT(ms);

    if (network_order)
	ntoh_MDLs1(ms,ms);

    memset(&ms->scale,0,sizeof(float3)*5);
    memset(ms->trans_matrix.v,0,sizeof(ms->trans_matrix));
    memset(ms->inv_matrix.v,0,sizeof(ms->inv_matrix));
    ms->scale.x = ms->scale.y = ms->scale.z = 1.0;

    ms->length	= sizeof(*ms);
    ms->flags	= MINIMAP_DEFAULT_FLAGS;

    if (network_order)
	hton_MDLs1(ms,ms);
}

///////////////////////////////////////////////////////////////////////////////

void ntoh_MDLs1 ( mdl_sect1_t *dest, const mdl_sect1_t *src )
{
    DASSERT(dest);

    if (src)
    {
	// float and u32 are equal for endian swapping!
	be32n( (u32*)dest, (u32*)src, sizeof(*dest)/sizeof(u32) );
    }
    else
	memset(dest,0,sizeof(*dest));
}

///////////////////////////////////////////////////////////////////////////////

void hton_MDLs1 ( mdl_sect1_t *dest, const mdl_sect1_t *src )
{
    DASSERT(dest);
    DASSERT(src);

 #if 1
    // float and u32 are equal for endian swapping!
    write_be32n( (u32*)dest, (u32*)src, sizeof(*dest)/sizeof(u32) );
 #else
    write_be32n( &dest->length,		&src->length,		  6   );
    write_bef4n( &dest->scale,		&src->scal,		 5*3  );
    write_be32n( &dest->parent_off,	&src->parent_off,	  5   );
    write_bef4n(  dest->trans_matrix,	 src->trans_matrix,	2*4*3 );
 #endif
}

///////////////////////////////////////////////////////////////////////////////

void CalcMatrixMDLs1 ( mdl_sect1_t *ms )
{
    DASSERT(ms);

    float3 rotate_rad;
    Mult3f(rotate_rad,ms->rotate,M_PI/180.0);

    SetupMatrixF34( &ms->trans_matrix,
		    &ms->scale,
		    &rotate_rad,
		    &ms->translate );

    CalcInverseMatrixF34( &ms->inv_matrix,
			  &ms->trans_matrix );
}

///////////////////////////////////////////////////////////////////////////////

void ImportMatrixMDLs1 // [[matrix]] [[test]] new function
(
    mdl_sect1_t		*ms,		// valid and initialized data
    bool		network_order,	// true: data is in network byte order
    MatrixD_t		*mat,		// valid data, import from
    uint		import_mode	// bit field:
					//  1: import scale, rotate and translate
					//  2: import transformation matrix
					//  4: import inverse matrix
)
{
    DASSERT(ms);
    DASSERT(mat);

    if (network_order)
	ntoh_MDLs1(ms,ms);


    //--- copy vectors

    if ( import_mode & 1 )
    {
	CalcNormMatrixD(mat);

	ms->scale.x = mat->norm_scale.x;
	ms->scale.y = mat->norm_scale.y;
	ms->scale.z = mat->norm_scale.z;

	ms->rotate.x = mat->norm_rotate_deg.x;
	ms->rotate.y = mat->norm_rotate_deg.y;
	ms->rotate.z = mat->norm_rotate_deg.z;

	ms->translate.x = mat->norm_translate.x;
	ms->translate.y = mat->norm_translate.y;
	ms->translate.z = mat->norm_translate.z;
    }


    //--- copy matrices

    if ( import_mode & 2 )
    {
	CalcTransMatrixD(mat,true);
	CopyD34toF34(&ms->trans_matrix,&mat->trans_matrix);
    }

    if ( import_mode & 4 )
    {
	CalcMatrixD(mat,true);
	CopyD34toF34(&ms->inv_matrix,&mat->inv_matrix);
    }


    //--- terminate

    if (network_order)
	hton_MDLs1(ms,ms);
}

///////////////////////////////////////////////////////////////////////////////

void ExportMatrixMDLs1 // [[matrix]] [[test]] new function
(
    mdl_sect1_t		*ms,		// valid and initialized data
    bool		network_order,	// true: data is in network byte order
    MatrixD_t		*mat,		// valid data, export to here
    uint		export_mode	// bit field:
					//  1: export scale, rotate and translate
					//  2: export trans matrix and declare it valid
					//  4: export inverse matrix and declare it valid
)
{
    DASSERT(ms);
    DASSERT(mat);

    if (network_order)
	ntoh_MDLs1(ms,ms);


    //--- copy vectors

    InitializeMatrixD(mat);

    if ( export_mode & 1 )
    {
	mat->scale.x = ms->scale.x;
	mat->scale.y = ms->scale.y;
	mat->scale.z = ms->scale.z;

	mat->rotate_deg.x = ms->rotate.x;
	mat->rotate_deg.y = ms->rotate.y;
	mat->rotate_deg.z = ms->rotate.z;

	mat->translate.x = ms->translate.x;
	mat->translate.y = ms->translate.y;
	mat->translate.z = ms->translate.z;
    }

    CalcNormMatrixD(mat);


    //--- copy matrices

    if ( export_mode & 2 )
    {
	CopyF34toD34(&mat->trans_matrix,&ms->trans_matrix);
	mat->tmatrix_valid = true;
    }

    if ( export_mode & 4 )
    {
	CopyF34toD34(&mat->inv_matrix,&ms->inv_matrix);
	mat->imatrix_valid = true;
    }


    //--- terminate

    if (network_order)
	hton_MDLs1(ms,ms);
}

///////////////////////////////////////////////////////////////////////////////

bool HaveStandardVectorsMDLs1 ( mdl_sect1_t *ms, bool network_order )
{
    DASSERT(ms);

    mdl_sect1_t ms_local;
    if (network_order)
    {
	ntoh_MDLs1(&ms_local,ms);
	ms = &ms_local;
    }

    return fabs(ms->scale.x-1.0) < 1e-6
	&& fabs(ms->scale.y-1.0) < 1e-6
	&& fabs(ms->scale.z-1.0) < 1e-6
	&& fabs(ms->rotate.y) < 1e-6
	&& fabs(ms->rotate.y) < 1e-6
	&& fabs(ms->rotate.z) < 1e-6
	&& fabs(ms->translate.x) < 1e-6
	&& fabs(ms->translate.y) < 1e-6
	&& fabs(ms->translate.z) < 1e-6;
}

///////////////////////////////////////////////////////////////////////////////

void DefineMatrixMDLs1
(
    // modify the vectors and calculate the matrices

    mdl_sect1_t		* ms1,		// pointer to valid MDLs1
    bool		network_order,	// true: data is in network byte order
    uint		reset,		// 0: do nothing
					// 1: reset only scale+rot+trans vectors
					// 2: reset all
    double		* scale,	// not NULL: multiply 3D scale to record
    double		* rotate,	// not NULL: add 3D rotation to record
    double		* translate	// not NULL: add 3D translation to record
)
{
    DASSERT(ms1);
    if ( reset > 1 )
	ResetMDLs1(ms1,false);
    else
    {
	if (network_order)
	    ntoh_MDLs1(ms1,ms1);
	if (reset)
	    ClearMDLs1(ms1,false);
    }

    if (scale)
    {
	ms1->scale.x *= scale[0];
	ms1->scale.y *= scale[1];
	ms1->scale.z *= scale[2];
    }

    if (rotate)
    {
	ms1->rotate.x += rotate[0];
	ms1->rotate.y += rotate[1];
	ms1->rotate.z += rotate[2];
    }

    if (translate)
    {
	ms1->translate.x += translate[0];
	ms1->translate.y += translate[1];
	ms1->translate.z += translate[2];
    }

    CalcMatrixMDLs1(ms1);
    if (network_order)
	hton_MDLs1(ms1,ms1);
}

///////////////////////////////////////////////////////////////////////////////

void PrintMatrixMDLs1
(
    FILE		* f,		// file to print to
    uint		indent,		// indention
    ccp			eol,		// not NULL: print as 'end of line'
    const mdl_sect1_t	* ms1,		// pointer to valid MDLs1
    uint		mode		// bitfield:
					//   1: print scale+rot+trans vectors
					//   2: minimum + maximum
					//   4: print matrix
					//   8: print inverse matrix
)
{
    indent = NormalizeIndent(indent);
    if (!eol)
	eol = "\n";

    if ( mode & 1 )
    {
	fprintf(f,
	    "%*sscale		= %11.3f %11.3f %11.3f%s"
	    "%*srotation	= %11.3f %11.3f %11.3f%s"
	    "%*stranslation	= %11.3f %11.3f %11.3f%s",

	    indent,"",
		ms1->scale.x,
		ms1->scale.y,
		ms1->scale.z,
	    eol,

	    indent,"",
		ms1->rotate.x,
		ms1->rotate.y,
		ms1->rotate.z,
	    eol,

	    indent,"",
		ms1->translate.x,
		ms1->translate.y,
		ms1->translate.z,
	    eol );
    }

    if ( mode & 2 )
    {
	fprintf(f,
	    "%*sminimum		= %11.3f %11.3f %11.3f%s"
	    "%*smaximum		= %11.3f %11.3f %11.3f%s",

	    indent,"",
		ms1->minimum.x,
		ms1->minimum.y,
		ms1->minimum.z,
	    eol,

	    indent,"",
		ms1->maximum.x,
		ms1->maximum.y,
		ms1->maximum.z,
	    eol );
    }

    if ( mode & 3 )
	fputs(eol,f);

    if ( mode & 4 )
    {
	fprintf(f,
	    "%*strans-matrix-x  = %11.3f %11.3f %11.3f %11.3f%s"
	    "%*strans-matrix-y  = %11.3f %11.3f %11.3f %11.3f%s"
	    "%*strans-matrix-z  = %11.3f %11.3f %11.3f %11.3f%s"
	    "%s",

	    indent,"",
		ms1->trans_matrix.m[0][0],
		ms1->trans_matrix.m[0][1],
		ms1->trans_matrix.m[0][2],
		ms1->trans_matrix.m[0][3],
	    eol,

	    indent,"",
		ms1->trans_matrix.m[1][0],
		ms1->trans_matrix.m[1][1],
		ms1->trans_matrix.m[1][2],
		ms1->trans_matrix.m[1][3],
	    eol,

	    indent,"",
		ms1->trans_matrix.m[2][0],
		ms1->trans_matrix.m[2][1],
		ms1->trans_matrix.m[2][2],
		ms1->trans_matrix.m[2][3],
	    eol,
	    eol );
    }

    if ( mode & 8 )
    {
	fprintf(f,
	    "%*sinv-matrix-x    = %11.3f %11.3f %11.3f %11.3f%s"
	    "%*sinv-matrix-y    = %11.3f %11.3f %11.3f %11.3f%s"
	    "%*sinv-matrix-z    = %11.3f %11.3f %11.3f %11.3f%s"
	    "%s",

	    indent,"",
		ms1->inv_matrix.m[0][0],
		ms1->inv_matrix.m[0][1],
		ms1->inv_matrix.m[0][2],
		ms1->inv_matrix.m[0][3],
	    eol,

	    indent,"",
		ms1->inv_matrix.m[1][0],
		ms1->inv_matrix.m[1][1],
		ms1->inv_matrix.m[1][2],
		ms1->inv_matrix.m[1][3],
	    eol,

	    indent,"",
		ms1->inv_matrix.m[2][0],
		ms1->inv_matrix.m[2][1],
		ms1->inv_matrix.m[2][2],
		ms1->inv_matrix.m[2][3],
	    eol,
	    eol );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 2			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetMDLs2 ( mdl_sect2_t *ms, bool network_order )
{
    DASSERT(ms);
    memset(ms,0,sizeof(*ms));

    ms->length	= sizeof(*ms);
    if (network_order)
	hton_MDLs2(ms,ms);
}

///////////////////////////////////////////////////////////////////////////////

void ntoh_MDLs2 ( mdl_sect2_t *dest, const mdl_sect2_t *src )
{
    DASSERT(dest);
    DASSERT(src);

    be32n ( &dest->length,	&src->length,		7 );
    memcpy( &dest->r_shift,	&src->r_shift,		2 );
    be16n ( &dest->n_vertex,	&src->n_vertex,		1 );
    bef4n (  dest->minimum.v,	 src->minimum.v,	6 );
}

///////////////////////////////////////////////////////////////////////////////

void hton_MDLs2 ( mdl_sect2_t *dest, const mdl_sect2_t *src )
{
    DASSERT(dest);
    DASSERT(src);

    write_be32n( &dest->length,		&src->length,		7 );
    memcpy     ( &dest->r_shift,	&src->r_shift,		2 );
    write_be16n( &dest->n_vertex,	&src->n_vertex,		1 );
    write_bef4n(  dest->minimum.v,	 src->minimum.v,	6 );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL section 10			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetMDLs10 ( mdl_sect10_t *ms, bool network_order )
{
    DASSERT(ms);
    memset(ms,0,sizeof(*ms));

    ms->length		= sizeof(*ms);
    ms->cp_vtx		= 0x4400;
    ms->cp_tex		=      2;
    ms->flags		=   0x80;
    ms->xf_flags	= 0x2a00;
    ms->unknown_0x60	= M1(ms->unknown_0x60);

    if (network_order)
	hton_MDLs10(ms,ms);
}

///////////////////////////////////////////////////////////////////////////////

void ntoh_MDLs10 ( mdl_sect10_t *dest, const mdl_sect10_t *src )
{
    DASSERT(dest);
    DASSERT(src);

    be32n( &dest->length,	&src->length,		18 );
    be16n( &dest->v_group,	&src->v_group,		12 );
    be32n( &dest->unknown_0x60,	&src->unknown_0x60,	 2 );
}

///////////////////////////////////////////////////////////////////////////////

void hton_MDLs10 ( mdl_sect10_t *dest, const mdl_sect10_t *src )
{
    DASSERT(dest);
    DASSERT(src);

    write_be32n( &dest->length,		&src->length,		18 );
    write_be16n( &dest->v_group,	&src->v_group,		12 );
    write_be32n( &dest->unknown_0x60,	&src->unknown_0x60,	 2 );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL minimap			///////////////
///////////////////////////////////////////////////////////////////////////////

#define MINIMAP_MAX_VISIBLE 49e3

///////////////////////////////////////////////////////////////////////////////

static int find_minimap_mdl
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    if ( term || it->entry < 0 )
	return 0;

    if ( it->group == 1 )
    {
	noPRINT("MDL-s1: %d.%d '%s'\n",it->group,it->entry,it->name);
	mdl_sect1_t *ms = (mdl_sect1_t*)( it->szs->data + it->off );
	mdl_minimap_t *mmap = it->param;

	if (!mmap->root)
	{
	    noPRINT("root FOUND! %p %u %u\n",it->param,it->group,it->entry);
	    mmap->root = ms;
	}

	if ( it->name )
	{
	    if ( be32(&ms->length) == sizeof(*ms) )
	    {
		DASSERT(mmap);
		if (!strcmp(it->name,"posLD"))
		{
		    noPRINT("posLD FOUND! %p %u %u\n",it->param,it->group,it->entry);
		    mmap->posLD = ms;
		}
		else if (!strcmp(it->name,"posRU"))
		{
		    noPRINT("posRU FOUND! %p %u %u\n",it->param,it->group,it->entry);
		    mmap->posRU = ms;
		}
	    }
	}
    }
    else if ( it->group == 2 )
    {
	mdl_sect2_t *ms = (mdl_sect2_t*)( it->szs->data + it->off );
	//HEXDUMP16(0,0,ms,sizeof(*ms));
	const void *data    = (u8*)ms + be32(&ms->data_off);
	const uint format   = be32(&ms->format);
	const uint r_shift  = ms->r_shift;
	uint n_vertex	    = be16(&ms->n_vertex);
	noPRINT("N-VERTEX=%u, FORMAT=%u, R-SHIFT=%u\n",n_vertex,format,r_shift);

	mdl_minimap_t *mmap = it->param;
	DASSERT(mmap);
	if ( n_vertex && !mmap->vertex )
	    mmap->vertex = ms;

	float3 min = mmap->min;
	float3 max = mmap->max;

	while ( n_vertex-- > 0 )
	{
	    float3 temp;
	    data = GetVectorMDL(&temp,data,format,r_shift);
	    noPRINT("GET= %11.3f %11.3f %11.3f [%d,%d,%d->%d]\n",
			temp.x, temp.y, temp.z,
			IsNormalF(temp.x), IsNormalF(temp.y), IsNormalF(temp.z),
			IsNormalF3(temp.v) );

	    if (   IsNormalF3(temp.v)
		&& fabsf(temp.x) < 1e7
		&& fabsf(temp.y) < 1e7
		&& fabsf(temp.z) < 1e7
	       )
	    {
		if ( min.x > temp.x ) min.x = temp.x;
		if ( min.y > temp.y ) min.y = temp.y;
		if ( min.z > temp.z ) min.z = temp.z;
		if ( max.x < temp.x ) max.x = temp.x;
		if ( max.y < temp.y ) max.y = temp.y;
		if ( max.z < temp.z ) max.z = temp.z;
	    }
	}

	PRINT("MIN= %11.3f %11.3f %11.3f\n",min.x,min.y,min.z);
	PRINT("MAX= %11.3f %11.3f %11.3f\n",max.x,max.y,max.z);
	mmap->min = min;
	mmap->max = max;

	const float x_add = ( max.x - min.x ) * 0.02 + 100;
	const float z_add = ( max.z - min.z ) * 0.02 + 100;

	mmap->rec_trans_ld.x = min.x - x_add;
	mmap->rec_trans_ld.y = 0.0;
	mmap->rec_trans_ld.z = max.z + z_add;

	mmap->rec_trans_ru.x = max.x + x_add;
	mmap->rec_trans_ru.y = 0.0;
	mmap->rec_trans_ru.z = min.z - z_add;

	if ( max.y > MINIMAP_MAX_VISIBLE )
	    mmap->rec_trans_ld.y = mmap->rec_trans_ru.y
		= 1.34 * ( max.y - MINIMAP_MAX_VISIBLE );

	mmap->rec_trans_valid
	    =  IsNormalF(mmap->rec_trans_ld.x) && fabsf(mmap->rec_trans_ld.x) < 1e6
	    && IsNormalF(mmap->rec_trans_ld.y) && fabsf(mmap->rec_trans_ld.y) < 1e6
	    && IsNormalF(mmap->rec_trans_ld.z) && fabsf(mmap->rec_trans_ld.z) < 1e6
	    && IsNormalF(mmap->rec_trans_ru.x) && fabsf(mmap->rec_trans_ru.x) < 1e6
	    && IsNormalF(mmap->rec_trans_ru.z) && fabsf(mmap->rec_trans_ru.z) < 1e6;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int find_minimap_brres
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    u8 *data = it->szs->data + it->off;

    if ( !term
	&& ( it->fform == FF_MDL
		|| it->fform == FF_UNKNOWN && !memcmp(data,MDL_MAGIC,4) )
       )
    {
	PRINT("MDL FOUND: %x |%s|%s|  fn=%s\n",it->fform,it->path,it->name,it->szs->fname);
	szs_file_t szs;
	InitializeSubSZS(&szs,it->szs,it->off,it->size,FF_MDL,it->path,false);
	int stat = IterateFilesParSZS ( &szs, find_minimap_mdl, it->param,
					false, true, 0, 1, SORT_NONE );
	ResetSZS(&szs);
	return stat;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int find_minimap_szs
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    u8 *data = it->szs->data + it->off;

    if ( !term
	&& ( it->fform == FF_BRRES
		|| it->fform == FF_UNKNOWN && !memcmp(data,BRRES_MAGIC,4) )
	&& !strcmp(it->name,"map_model.brres")
       )
    {
	PRINT("MAP MODEL FOUND: %x |%s|%s|\n",it->fform,it->path,it->name);
	szs_file_t szs;
	InitializeSubSZS(&szs,it->szs,it->off,it->size,FF_BRRES,it->path,false);
	int stat = IterateFilesParSZS ( &szs, find_minimap_brres, it->param,
					false, true, 0, -1, SORT_NONE );
	ResetSZS(&szs);
	return stat;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool FindMinimapData
(
    mdl_minimap_t	*mmap,		// minmap data to fill
    struct szs_file_t	*szs		// valid szs data
)
{
    DASSERT(mmap);
    DASSERT(szs);
    noPRINT("FindMinimapData(mmap=%p)\n",mmap);

    memset(mmap,0,sizeof(*mmap));
    mmap->min.x = mmap->min.y = mmap->min.z = +INFINITY;
    mmap->max.x = mmap->max.y = mmap->max.z = -INFINITY;

    if ( szs->fform_arch == FF_MDL )
	IterateFilesParSZS ( szs, find_minimap_mdl, mmap, false, true, 0, 1, SORT_NONE );
    else if ( szs->fform_arch == FF_BRRES )
	IterateFilesParSZS ( szs, find_minimap_brres, mmap, false, true, 0, -1, SORT_NONE );
    else if (IsArchiveFF(szs->fform_arch))
	IterateFilesParSZS ( szs, find_minimap_szs, mmap, false, true, 0, -1, SORT_NONE );

    PRINT_IF(mmap->posLD," posLD found, in=%d\n",
	    (u8*)mmap->posLD >= szs->data && (u8*)mmap->posLD < szs->data + szs->size );
    PRINT_IF(mmap->posRU," posLD found, in=%d\n",
	    (u8*)mmap->posRU >= szs->data && (u8*)mmap->posRU < szs->data + szs->size );

    return mmap->posLD && mmap->posRU;
}

///////////////////////////////////////////////////////////////////////////////

uint AutoAdjustMinimap
(
    // returns:
    //	0: nothing done
    //	1: minimap changed
    //  3: minimap changed & vertex list flattened

    struct szs_file_t	*szs		// valid SZS data
)
{
    DASSERT(szs);
    uint ret_status = 0;

    mdl_minimap_t mmap;
    if ( FindMinimapData(&mmap,szs) && mmap.rec_trans_valid )
    {
	PRINT("AutoAdjustMinimap()\n");
	DASSERT(mmap.posLD);
	DASSERT(mmap.posRU);
	ret_status = 1;

	if ( mmap.vertex && ( mmap.min.y < 0.0 || mmap.max.y > MINIMAP_MAX_VISIBLE ) )
	{
	    PATCH_ACTION_LOG("Patch","MINIMAP","(vertex list flattened)\n");
	    ret_status |= 2;
	    mmap.rec_trans_ld.y = mmap.rec_trans_ru.y = 0;
	    // vertices are flattened only, if vectors or matrices changed.
	}
	else
	    PATCH_ACTION_LOG("Patch","MINIMAP",0);

	mdl_sect1_t ld;
	ntoh_MDLs1(&ld,mmap.posLD);
	ClearMDLs1(&ld,false);
	ld.translate = mmap.rec_trans_ld;
	CalcMatrixMDLs1(&ld);

	mdl_sect1_t ru;
	ntoh_MDLs1(&ru,mmap.posRU);
	ClearMDLs1(&ru,false);
	ru.translate = mmap.rec_trans_ru;
	CalcMatrixMDLs1(&ru);

	hton_MDLs1(&ld,&ld);
	hton_MDLs1(&ru,&ru);
	if (   memcmp(mmap.posLD,&ld,sizeof(ld))
	    || memcmp(mmap.posRU,&ru,sizeof(ru)) )
	{
	    memcpy(mmap.posLD,&ld,sizeof(ld));
	    memcpy(mmap.posRU,&ru,sizeof(ru));

	    if (ret_status&2)
	    {
		void *data		= (u8*)mmap.vertex + be32(&mmap.vertex->data_off);
		const uint format   = be32(&mmap.vertex->format);
		const uint r_shift  = mmap.vertex->r_shift;
		uint n_vertex	= be16(&mmap.vertex->n_vertex);
		noPRINT("N-VERTEX=%u, FORMAT=%u, R-SHIFT=%u\n",n_vertex,r_shift);

		while ( n_vertex-- > 0 )
		{
		    float3 temp;
		    GetVectorMDL(&temp,data,format,r_shift);
		    temp.y = 20000.0;
		    data = SetVectorMDL(&temp,data,format,r_shift);
		}
	    }

	    return ret_status;
	}
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL moonview data		///////////////
///////////////////////////////////////////////////////////////////////////////
// CMD: ./work/scripts/etc/bin-to-bz_c.sh ext.tmp/*.bin
///////////////////////////////////////////////////////////////////////////////

const u8 S42_MDL_BZ2[] =
{
  0x00,0x00,0x4b,0x00, // 19200 bytes decompressed
  // 0x0:
  0x42,0x5a,0x68,0x39, 0x31,0x41,0x59,0x26, 0x53,0x59,0x0d,0x14, 0x24,0x06,0x00,0x0c,
  0x3b,0xff,0xff,0xff, 0xff,0xff,0xf7,0xcf, 0x7e,0x7f,0xcd,0xf5, 0xff,0x76,0xc7,0xbf,
  0xef,0xde,0xf4,0x4c, 0x54,0x66,0x48,0x44, 0x40,0x46,0x44,0xd2, 0xc1,0x05,0x50,0xff,
  0xce,0x58,0x49,0xd0, 0x04,0x97,0x80,0x34, 0x00,0x1a,0x29,0xaa, 0x0c,0xa2,0x50,0x06,
  0x98,0x80,0x3d,0x40, 0x00,0x00,0x01,0xa0, 0x01,0xa0,0x00,0x00, 0xc8,0x00,0x01,0xa0,
  0x0f,0x50,0x34,0x00, 0x00,0xd0,0x68,0x03, 0xd4,0x1a,0x50,0x53, 0xd4,0xcf,0x4a,0x66,
  0xa7,0xa4,0x1e,0x14, 0x00,0x00,0xd0,0x00, 0x68,0x68,0x00,0xd0, 0x00,0x00,0x00,0x00,
  0x03,0x40,0x00,0x00, 0x0d,0x00,0x00,0x81, 0x93,0x04,0x64,0xc0, 0x8d,0x31,0x0d,0x30,
  0x8c,0x43,0x4c,0x00, 0x46,0x04,0x60,0x20, 0xc9,0xa6,0x43,0x46, 0x43,0x40,0x62,0x60,
  0x26,0x21,0x80,0x98, 0x8d,0x34,0xc4,0xc0, 0x98,0x10,0x32,0x60, 0x8c,0x98,0x11,0xa6,
  0x21,0xa6,0x11,0x88, 0x69,0x80,0x08,0xc0, 0x8c,0x04,0x19,0x34, 0xc8,0x68,0xc8,0x68,
  0x0c,0x4c,0x04,0xc4, 0x30,0x13,0x11,0xa6, 0x98,0x98,0x13,0x00, 0x89,0x48,0x49,0xa6,
  0x26,0x86,0x8a,0x66, 0xa6,0x98,0xd4,0x69, 0x80,0x46,0x83,0x4f, 0x50,0x18,0x8c,0x8d,
  0x07,0xa8,0xc4,0xc8, 0x68,0x69,0xe8,0x86, 0x8f,0x50,0x64,0xc8, 0xd3,0x26,0x99,0x19,
  0x1a,0x1a,0x64,0x66, 0xa1,0xea,0x64,0xc1, 0x32,0xb3,0x91,0x32, 0xeb,0xae,0xba,0xcb,
  0x26,0xa3,0x06,0x6e, 0x70,0x23,0x8e,0x0e, 0x45,0xc6,0x33,0xc0, 0x0a,0xf1,0xaf,0xf0,
  // 0x100:
  0x04,0xe1,0x59,0xc9, 0xd9,0xe3,0x19,0x3c, 0x25,0xc0,0x05,0xc5, 0xd0,0x73,0xac,0xb9,
  0x72,0xd5,0xcc,0x70, 0xc5,0x85,0xf0,0x44, 0xc5,0x06,0x76,0x33, 0xb1,0x70,0x71,0x3e,
  0x2c,0x8d,0x10,0x76, 0x97,0xfa,0x77,0x41, 0xd9,0x83,0x9a,0x0f, 0x43,0x1c,0x1d,0x4e,
  0x15,0x14,0xca,0x70, 0x50,0x89,0x22,0x5b, 0x04,0x49,0x01,0xfc, 0x54,0xfc,0xed,0x54,
  0x00,0x0c,0x47,0x99, 0x83,0x99,0xd2,0xe4, 0x6a,0x37,0xfe,0x69, 0x6a,0xe7,0xe5,0xcf,
  0xbc,0x0e,0x2e,0x65, 0xff,0x44,0x18,0x0f, 0x70,0x1d,0x88,0x36, 0x94,0xe7,0xe4,0x83,
  0x89,0x86,0xa9,0x98, 0x45,0x78,0x78,0x68, 0x54,0x86,0xeb,0xac, 0x9d,0xe9,0x59,0xd1,
  0x34,0xfb,0xaa,0x95, 0x00,0xe2,0x03,0xf0, 0x50,0x13,0x61,0x1b, 0xca,0x67,0xd0,0x1d,
  0xb6,0x35,0x10,0x78, 0xb1,0x1d,0x66,0xfe, 0x80,0xf0,0x63,0xbd, 0x8a,0xa3,0xce,0x80,
  0xd3,0x7d,0x4f,0xe6, 0x00,0x03,0x78,0x1a, 0xe8,0x8e,0x74,0xcf, 0x80,0xde,0xe2,0xeb,
  0x33,0x2b,0x06,0xed, 0x94,0x92,0x11,0x18, 0x40,0x72,0xfd,0xc0, 0x7f,0xc0,0x74,0x41,
  0x0c,0xc0,0x1f,0xf4, 0x1b,0x40,0xe1,0x45, 0x24,0x8f,0x72,0x80, 0xfb,0x40,0xc0,0x00,
  0x0d,0xb0,0x30,0x04, 0xec,0xc4,0x77,0x71, 0x1c,0x90,0x70,0x81, 0xa7,0xe9,0x04,0x10,
  0xcd,0x94,0x88,0x28, 0xd0,0x06,0x40,0x18, 0x76,0xa2,0x35,0x44, 0x0f,0x9c,0x19,0x97,
  0x55,0x0d,0xed,0x28, 0x69,0xa9,0x4c,0xfa, 0x76,0x01,0xa6,0x4b, 0xc0,0x26,0xed,0x29,
  0x42,0xca,0x80,0x00, 0x2a,0x10,0x54,0x73, 0x4f,0x03,0x08,0xd2, 0xa9,0x9d,0x1b,0x5c,
  // 0x200:
  0x38,0x86,0xf5,0x61, 0xbb,0xe6,0xa8,0x07, 0xba,0x24,0x03,0x04, 0x07,0xea,0x13,0x24,
  0x10,0xa0,0x21,0x01, 0x80,0xc0,0x60,0x21, 0xfb,0x83,0x14,0x47, 0x1a,0x09,0x96,0x08,
  0x90,0x44,0x4b,0x91, 0x12,0xfc,0x51,0xc8, 0x8a,0xe0,0x8a,0x38, 0x20,0x09,0x91,0x11,
  0xe1,0x40,0xc8,0x88, 0x0b,0x4c,0x8a,0x20, 0x28,0x78,0x90,0x57, 0x55,0x06,0x31,0x91,
  0x5c,0x80,0x60,0x85, 0x22,0x2a,0xc8,0x23, 0x08,0x20,0xa4,0x82, 0x26,0x14,0x01,0xee,
  0x83,0xa5,0x52,0x98, 0xd8,0x94,0x51,0xd2, 0x83,0x8a,0xa5,0x24, 0xce,0x06,0x29,0x40,
  0x18,0x59,0x6d,0x0a, 0x49,0x08,0xb0,0x18, 0x8a,0x90,0x4b,0x54, 0x88,0x50,0x25,0x01,
  0x90,0x4c,0x68,0xe0, 0x06,0x08,0xdf,0x3b, 0x20,0x85,0x10,0x06, 0xc2,0x0a,0x5a,0x80,
  0xfb,0x20,0xf2,0x39, 0x55,0x00,0x5f,0x06, 0x22,0xf4,0xd4,0x80, 0xea,0x81,0x98,0x84,
  0x06,0x61,0xb4,0x51, 0x39,0x70,0x5b,0xc4, 0x4e,0xa5,0x77,0x9a, 0x91,0x06,0xd4,0x51,
  0xaa,0xaa,0x32,0xaa, 0x15,0x44,0x00,0x66, 0x9a,0xd5,0x40,0xd7, 0x30,0xb9,0x20,0xd0,
  0x1d,0x40,0x37,0x81, 0xa0,0x20,0x62,0x5b, 0xb7,0x5d,0x68,0xb8, 0x01,0xe1,0x50,0xae,
  0xcd,0x86,0xd7,0x81, 0x60,0x77,0x49,0xcc, 0xbc,0x0d,0xb2,0xa3, 0x6f,0x38,0x20,0xc8,
  0x0d,0x59,0x15,0x1c, 0x9c,0x6d,0x00,0x42, 0xa0,0xb2,0xca,0x6c, 0x83,0xdb,0xdc,0xd0,
  0xdc,0x71,0x3f,0x2b, 0xd5,0xe6,0x95,0xd9, 0x64,0x92,0x56,0x5b, 0x46,0x12,0x17,0x29,
  0x42,0x28,0xde,0xaa, 0xed,0x5b,0x98,0xd5, 0xa3,0x5e,0x40,0x9c, 0x70,0x01,0xb0,0x00,
  // 0x300:
  0x60,0x00,0xe5,0x80, 0x0d,0x01,0xb4,0x08, 0x9e,0x28,0x38,0xa0, 0xed,0xc1,0xb7,0x65,
  0x01,0xa8,0x06,0x86, 0x7d,0xee,0x05,0xbc, 0x2d,0x97,0x07,0x2f, 0xa1,0xdb,0xeb,0xdd,
  0x0d,0x7e,0x93,0xca, 0xab,0x93,0x10,0xd2, 0x57,0x6e,0xad,0xfd, 0xf8,0x3c,0xa5,0x97,
  0x66,0x89,0x84,0x1c, 0x01,0x92,0xd7,0xac, 0x34,0x3c,0x1c,0x90, 0x04,0xca,0x64,0x08,
  0xf0,0x09,0x48,0xf4, 0x91,0x00,0xba,0x0f, 0x07,0xc3,0x53,0xee, 0xb7,0xa3,0x66,0xb4,
  0x32,0xee,0x62,0x50, 0xa4,0x74,0x9c,0x7a, 0x6b,0xba,0x72,0xaa, 0x83,0x43,0xad,0x67,
  0x2c,0x1b,0xa0,0xf5, 0x20,0xed,0x08,0x1b, 0x80,0x73,0xc1,0xb6, 0x74,0xb9,0x53,0xaa,
  0xa6,0x93,0xf8,0x2f, 0xeb,0x81,0xb8,0x0f, 0x7e,0x0d,0x01,0xc0, 0x0f,0x00,0x06,0xb0,
  0x60,0x30,0x10,0x0d, 0x78,0x9c,0x79,0xce, 0x07,0x2b,0x29,0xd0, 0xbf,0x28,0x66,0xec,
  0x7d,0x8a,0x80,0xca, 0xbb,0x1e,0xb6,0x3b, 0x87,0xa9,0xd0,0x6f, 0x6a,0xb4,0x36,0xd7,
  0x00,0x4c,0xa5,0x80, 0x64,0x23,0x69,0xdb, 0x11,0xf0,0xf0,0x60, 0xe6,0xe1,0x50,0x10,
  0x40,0xb9,0xe6,0x9c, 0xc1,0x07,0xca,0x10, 0x1e,0x2f,0x91,0x24, 0x5b,0x05,0xbc,0xac,
  0x98,0x28,0x04,0xc9, 0xa6,0xf2,0xfe,0xad, 0x81,0x8d,0x4c,0x7e, 0xae,0x56,0xdc,0xc7,
  0x30,0xf2,0xb7,0xcf, 0x7d,0x93,0x14,0xa6, 0xe2,0x9f,0xb7,0x08, 0x11,0x24,0x05,0x10,
  0xfa,0x01,0xc7,0x52, 0xd5,0x40,0x03,0xcd, 0x07,0x3c,0x1f,0x1c, 0x1e,0x30,0x3c,0x60,
  0x6f,0x03,0xdb,0x07, 0x86,0x62,0x75,0xfb, 0x1d,0x1f,0xfb,0xb3, 0xda,0x63,0xb9,0xa4,
  // 0x400:
  0xba,0xca,0x00,0x63, 0xf0,0x9d,0xfd,0xb2, 0x55,0x0e,0xf5,0x93, 0x3d,0xb7,0xf9,0xf4,
  0x01,0x97,0xc8,0xaf, 0x6d,0xc0,0xfe,0xd7, 0x3d,0xc3,0x48,0x37, 0xa5,0xd2,0x01,0x02,
  0x43,0xfb,0xae,0xd1, 0x85,0x79,0xe8,0x7d, 0x7d,0xca,0xb5,0x9c, 0xdd,0x9e,0xb7,0x26,
  0xce,0xec,0x3e,0x6d, 0x3e,0x2e,0x5e,0xa6, 0xca,0xf7,0x7b,0xce, 0xde,0xaf,0xf5,0xf3,
  0xc1,0xc2,0x80,0x86, 0xd6,0x02,0x14,0x02, 0x80,0x00,0x05,0x08, 0x00,0x92,0xc1,0x92,
  0xce,0x97,0xdd,0x6c, 0xbc,0xee,0xbb,0xa7, 0x1e,0x0c,0x78,0xec, 0x12,0xcc,0x02,0xe5,
  0x2a,0x80,0x54,0x00, 0xa6,0xe0,0x07,0x56, 0xb5,0xeb,0xe8,0xd0, 0x1a,0xd0,0x60,0x30,
  0x00,0x6e,0x90,0x0a, 0x03,0x10,0x2e,0xc0, 0x6c,0x05,0x78,0xc0, 0xc0,0x1a,0x56,0xa6,
  0xca,0x80,0x3a,0xbf, 0x4c,0x1b,0x01,0xff, 0xc5,0xdc,0x91,0x4e, 0x14,0x24,0x03,0x45,
  0x09,0x01,0x80,
  // 0x493 = 1171 Bytes
};

///////////////////////////////////////////////////////////////////////////////

mdl_t slot42_mdl = {0};

#undef def42
#define def42(n,i) { S42M_##n, i, 0 },

Slot42MaterialInfo_t Slot42MaterialInfo[S42M__N+1] =
{
	def42( GOAL_MERG,	"Goal_Merg" )
	def42( IWA,		"Iwa" )
	def42( IWA_ALFA,	"Iwa_alfa" )
	def42( NUKI_RYOUMEN,	"Nuki_Ryoumen" )
	def42( WALLMERG00,	"WallMerg00" )
	def42( MOON_KABE0000,	"moon_kabe0000" )
	def42( MOON_ROAD00,	"moon_road00" )
	def42( ROAD,		"road" )
	def42( ROAD01,		"road01" )
	def42( ROAD02,		"road02" )
	def42( ROAD03,		"road03" )
	def42( SIBA00,		"siba00" )
	{0,"",0}
};

#undef def42

///////////////////////////////////////////////////////////////////////////////

const Slot42MaterialInfo_t * SetupSlot42Materials()
{
    if ( slot42_mdl.fform != FF_MDL )
    {
	InitializeMDL(&slot42_mdl);
	slot42_mdl.fname = STRDUP("course.mdl");
	slot42_mdl.fform = FF_MDL;

	u8 *data;
	uint size;
	if (!DecodeBZIP2(&data,&size,0,S42_MDL_BZ2,sizeof(S42_MDL_BZ2)))
	{
	 #if USE_NEW_CONTAINER_MDL
	    Container_t *c = CreateContainer(0,0,data,size,CPM_MOVE);
	    ScanRawMDL(&slot42_mdl,false,data,size,LinkContainerData(c));
	 #else
	    DataContainer_t *dc = NewContainer(0,data,size,CPM_MOVE);
	    ScanRawMDL(&slot42_mdl,false,data,size,dc);
	 #endif

	    MemItem_t *ptr = GetMemListElem(&slot42_mdl.elem,0,0);
	    MemItem_t *end = ptr + slot42_mdl.elem.used;
	    for ( ; ptr < end; ptr++ )
	    {
		Slot42MaterialInfo_t *sptr;
		for ( sptr = Slot42MaterialInfo; sptr->val; sptr++ )
		    if (!strcmp(sptr->name,ptr->name))
		    {
			sptr->memitem = ptr;
			break;
		    }
	    }
	}
	//HEXDUMP16(0,0,Slot42MaterialInfo,sizeof(Slot42MaterialInfo));
    }
    return Slot42MaterialInfo;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MDL moonview detection		///////////////
///////////////////////////////////////////////////////////////////////////////

static int ana_slot42_mdl
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    if ( term || it->entry < 0 )
	return 0;

    Slot42MaterialStat_t *stat42 = it->param;
    DASSERT(stat42);

    char buf[2000];

    if ( it->group == 8 && it->name )
    {
	const Slot42MaterialInfo_t *ptr;
	for ( ptr = SetupSlot42Materials(); ptr->val; ptr++ )
	    if (!strcasecmp(ptr->name,it->name))
	    {
		stat42->found |= ptr->val;
		const MemItem_t *mi = ptr->memitem;
		if (mi)
		{
		    DASSERT( mi->size <= sizeof(buf) );
		    if ( mi->size != it->size || mi->size > sizeof(buf) )
			stat42->modified |= ptr->val;
		    else
		    {
			// fix name offsets before comparing

			memcpy(buf,mi->data,mi->size);

			u8 *sdata = it->szs->data + it->off;
			memcpy(buf+0x04,sdata+0x04,8); // offset + name
			memcpy(buf+0x18,sdata+0x18,4); // culling

			const endian_func_t * endian = it->endian;
			DASSERT(endian);

			const uint n_layer = endian->rd32(buf+0x2c);
			u32 layer_off = endian->rd32(buf+0x30);
			uint i;
			for ( i = 0; i < n_layer; i++, layer_off += 0x34 )
			    memcpy(buf+layer_off,sdata+layer_off,4); // name
			HEXDIFF16(0,0,sdata,mi->size,buf,mi->size);

			if ( memcmp(buf,sdata,mi->size) )
			    stat42->modified |= ptr->val;
		    }
		}

		stat42->all_found = stat42->found == S42M__ALL;
		stat42->content_ok = !stat42->modified;
		stat42->ok = stat42->all_found && stat42->content_ok;

		//-- extract

		File_t F;
		if (PrepareExtract(&F,-1,"%s.bin",ptr->name))
		{
		    printf("EXTRACT: %s\n",F.fname);
		    fwrite(it->szs->data+it->off,it->size,1,F.f);
		}
		CloseFile(&F,false);
		break;
	    }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int ana_slot42_brres
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);

    u8 *data = it->szs->data + it->off;

    if ( !term
	&& it->off +4 <= it->size
	&& ( it->fform == FF_MDL
		|| it->fform == FF_UNKNOWN && !memcmp(data,MDL_MAGIC,4) )
	&& !strcasecmp(it->name,"course")
       )
    {
	PRINT("MOONVIEW MDL FOUND: %x |%s|%s|\n",it->fform,it->path,it->name);
	szs_file_t szs;
// [[fname+]]
	InitializeSubSZS(&szs,it->szs,it->off,it->size,FF_MDL,it->path,false);
	int stat = IterateFilesParSZS ( &szs, ana_slot42_mdl, it->param,
					false, true, 0, 1, SORT_NONE );

	Slot42MaterialStat_t *stat42 = it->param;
	DASSERT(stat42);
	if (stat42->ok)
	{
	    File_t F;
	    if (PrepareExtract(&F,-1,"course-42.mdl"))
	    {
		printf("EXTRACT: %s\n",F.fname);

		mdl_t mdl;
	     #if USE_NEW_CONTAINER_MDL
		if (!ScanRawMDL(&mdl,true,szs.data,szs.size,LinkContainerSZS(&szs)))
	     #else
		if (!ScanRawMDL(&mdl,true,szs.data,szs.size,ContainerSZS(&szs)))
	     #endif
		{
		    mdl.bone_n = 0;
		    MemItem_t *ptr = GetMemListElem(&mdl.elem,0,0);
		    MemItem_t *end = ptr + mdl.elem.used;
		    for ( ; ptr < end; ptr++ )
		    {
			bool delete = true;
			if ( ptr->idx1 == 8 )
			{
			    const Slot42MaterialInfo_t *sptr;
			    for ( sptr = Slot42MaterialInfo; sptr->val; sptr++ )
				if (!strcmp(sptr->name,ptr->name))
				{
				    delete = false;
				    break;
				}
			}
			if (delete)
			{
			    if (ptr->data_alloced)
				FREE(ptr->data);
			    ptr->data = 0;
			    ptr->size = 0;
			}
		    }

		    CreateRawMDL(&mdl);
		    fwrite(mdl.raw_data,mdl.raw_file_size,1,F.f);
		}
	    }
	    CloseFile(&F,false);
	}

	ResetSZS(&szs);
	return stat;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int ana_slot42_szs
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    u8 *data = it->szs->data + it->off;

    if ( !term
	&& ( it->fform == FF_BRRES
		|| it->fform == FF_UNKNOWN && !memcmp(data,BRRES_MAGIC,4) )
	&& ( !strcmp(it->name,"course_model.brres")
		||  !strcmp(it->name,"course_d_model.brres") )
       )
    {
	PRINT("COURSE MODEL FOUND: %x |%s|%s|\n",it->fform,it->path,it->name);
	szs_file_t szs;
// [[fname+]]
	InitializeSubSZS(&szs,it->szs,it->off,it->size,FF_BRRES,it->path,false);
	int stat = IterateFilesParSZS ( &szs, ana_slot42_brres, it->param,
					false, true, 0, -1, SORT_NONE );
	ResetSZS(&szs);
	return stat;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Slot42MaterialStat_t GetSlot42SupportSZS ( struct szs_file_t *szs )
{
    DASSERT(szs);

    Slot42MaterialStat_t stat42;
    memset(&stat42,0,sizeof(stat42));

    if ( !szs->data )
	return stat42;

    PRINT("CHECK MOONVIEW [%zu] %s\n",szs->size,GetNameFF(szs->fform_arch,0));

    if ( szs->fform_arch == FF_MDL )
	IterateFilesParSZS ( szs, ana_slot42_mdl, &stat42, false, true, 0, 1, SORT_NONE );
    else if ( szs->fform_arch == FF_BRRES )
	IterateFilesParSZS ( szs, ana_slot42_brres, &stat42, false, true, 0, -1, SORT_NONE );
    else if (IsArchiveFF(szs->fform_arch))
	IterateFilesParSZS ( szs, ana_slot42_szs, &stat42, false, true, 0, -1, SORT_NONE );

    PRINT("MOONVIEW [%zu] %s: found=%x, mod=%x -> all=%d, content=%d -> ok=%d\n",
		szs->size, GetNameFF(szs->fform_arch,0),
		stat42.found, stat42.modified,
		stat42.all_found, stat42.content_ok, stat42.ok );

    return stat42;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[obsolete]] 2016-06
// RenameForSlot42MDL() Don't work! A modified track runs at slot 1.1,
// but not at 4.2. I assume, that the layers must be correct.

ccp RenameForSlot42MDL
(
    void		*rawdata,	// pointer to data, modified
    uint		size,		// size of 'rawdata'
    ContainerMDL_t	* cdata		// NULL or container-data
)
{
    mdl_t mdl;
    enumError err = ScanMDL(&mdl,true,rawdata,size,cdata,0);
    ccp errmsg = 0;
    if (err)
    {
	errmsg = "Invalid MDL";
	goto abort;
    }

    {
	u8 slot42ok[S42M__N];
	memset(slot42ok,0,sizeof(slot42ok));
	uint n_ok = 0;

	const uint MAX_SECT8 = 1000;
	const MemItem_t *sect8[MAX_SECT8];
	uint sect8_count = 0;

	const MemItem_t *mi = GetMemListElem(&mdl.elem,0,0);
	const MemItem_t *mi_end = mi + mdl.elem.used;
	for ( ; mi < mi_end && sect8_count < MAX_SECT8; mi++ )
	{
	    if ( mi->idx1 != 8 || !mi->data )
		continue;

	    sect8[sect8_count] = mi;

	    const Slot42MaterialInfo_t *sptr;
	    for ( sptr = Slot42MaterialInfo; sptr->val; sptr++ )
		if (!strcmp(sptr->name,mi->name))
		{
		    slot42ok[sptr-Slot42MaterialInfo] = 1;
		    sect8[sect8_count] = 0;
		    n_ok++;
		    break;
		}

	    sect8_count++;
	}
	PRINT("N(slot42)=%d, N(mat)=%d\n",S42M__N,sect8_count);
	HEXDUMP16(5,0,slot42ok,sizeof(slot42ok));

	if ( sect8_count < S42M__N )
	{
	    errmsg = "Too less materials";
	    goto abort;
	}
	if ( n_ok == S42M__N )
	    goto abort; // all done

	ccp rename[S42M__N];
	memset(rename,0,sizeof(rename));
	uint sect, n_rename = 0;

	for ( sect = 0; sect < S42M__N; sect++ )
	{
	    if (slot42ok[sect])
		continue;
	    const Slot42MaterialInfo_t *sptr = Slot42MaterialInfo + sect;
	    const uint mlen = strlen(sptr->name);

	    int found = -1;
	    uint flen = 0x40000000, s8idx;
	    for ( s8idx = 0; s8idx < sect8_count; s8idx++ )
	    {
		const MemItem_t *mi = sect8[s8idx];
		if (!mi)
		    continue;

		const uint slen = strlen(mi->name);
		if ( mlen <= slen && slen < flen )
		{
		    found = s8idx;
		    flen  = slen;
		}
	    }
	    if ( found < 0 )
	    {
		errmsg = "Material names to short";
		goto abort;
	    }


	    rename[sect] = sect8[found]->name;
	    sect8[found] = 0;
	    n_rename++;
	}

     #ifdef TEST
	//if ( logging >= 2 && stdlog )
	{
	    fprintf(stdlog,
		    "\tRename %u MDL material%s:\n",
		    n_rename, n_rename == 1 ? "" : "s" );
	    for ( sect = 0; sect < S42M__N; sect++ )
		if (rename[sect])
		{
		    ParamFieldItem_t *par = FindParamField(&mdl.spool.pf,rename[sect]);
		    ccp pdata = par ? par->data : 0;
		    fprintf(stdlog,"\t  %-13s <- %-30s %p %s\n",
				Slot42MaterialInfo[sect].name, rename[sect],
				pdata, pdata );
		}
	}
     #endif

     #if USE_NEW_CONTAINER_MDL
	DASSERT(mdl.container.cdata);
	DASSERT(mdl.container.cdata->data);
	const u8 *copy_start = mdl.container.cdata->data + size;
     #else
	DASSERT(mdl.old_container);
	DASSERT(mdl.old_container->data);
	const u8 *copy_start = mdl.old_container->data + size;
     #endif
	for ( sect = 0; sect < S42M__N; sect++ )
	    if (rename[sect])
	    {
		ParamFieldItem_t *par = FindParamField(&mdl.spool.pf,rename[sect]);
		if ( !par || !par->data )
		{
		    errmsg = "Internal Error";
		    goto abort;
		}
		DASSERT(!strcmp(rename[sect],par->data));
		u8 *dest = par->data - 4;
		if ( copy_start > dest )
		    copy_start = dest;
		const uint rlen = strlen(rename[sect]);
		const uint mlen = strlen(Slot42MaterialInfo[sect].name);
		DASSERT( mlen <= rlen );
		write_be32(par->data-4,mlen);
		memset(par->data,0,rlen);
		memcpy(par->data,Slot42MaterialInfo[sect].name,mlen);
	    }

     #if USE_NEW_CONTAINER_MDL
	const int offset = copy_start - mdl.container.cdata->data;
	if ( offset > 0 )
	    memcpy( rawdata+offset, mdl.container.cdata->data+offset, size-offset );
     #else
	const int offset = copy_start - mdl.old_container->data;
	if ( offset > 0 )
	    memcpy( rawdata+offset, mdl.old_container->data+offset, size-offset );
     #endif
    }

 abort:
    ResetMDL(&mdl);
    return errmsg;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PrintStringsMDL()		///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_sections_mdl
(
    struct StringIteratorMDL_t	*sit,	// string iterator data
    MemItem_t			*mi	// memory item of section
)
{
    DASSERT(sit);
    DASSERT(sit->f);
    DASSERT(mi);

    fprintf( sit->f, "%*sSection %2d.%02d: %s\n",
		sit->indent,"", mi->idx1, mi->idx2, mi->name );

    const u32 *sptr = GetStrIdxListMIL(mi);
    uint i, n = GetStrIdxCountMIL(mi);
    for ( i = 0; i < n; i++ )
    {
	const uint idx = sptr[i];
	if ( idx && idx < sit->sp->n2s_size )
	{
	    ccp str = sit->sp->n2s_list[idx];
	    if (!str)
		str = "-invalid-";
	    fprintf( sit->f, "%*s  %s\n", sit->indent, "", str );
	}
    }
}

//-----------------------------------------------------------------------------

enumError PrintStringsMDL
(
    FILE		* f,		// output file
    int			indent,		// indention
    mdl_t		* mdl		// valid MDL
)
{
    DASSERT(f);
    DASSERT(mdl);

    StringIteratorMDL_t sim;
    memset(&sim,0,sizeof(sim));
    sim.mdl		= mdl;
    sim.sp		= &mdl->spool;
    sim.sect_func	= print_sections_mdl;
    //sim.str_func	= print_strings_mdl;
    sim.f		= f;
    sim.indent		= NormalizeIndent(indent)+2;

    fprintf(f,"%*sStrings of %s\n",sim.indent-2,"",mdl->fname);
    IterateAllStringsMDL(&sim);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AppendMDL()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError AppendMDL
(
    mdl_t		* mdl,		// working MDL (valid)
    const mdl_t		* src_mdl	// pointer to source MDL
)
{
    DASSERT(mdl);
    DASSERT(src_mdl);

    //--- find max sort_index

    int total_max_sort = 0;
    MemItem_t *base = GetMemListElem(&mdl->elem,0,0);
    MemItem_t *ptr, *end = base + mdl->elem.used;
    for ( ptr = base; ptr < end; ptr++ )
	if ( total_max_sort < ptr->sort_idx )
	    total_max_sort = ptr->sort_idx;

    PRINT("TOTAL-MAX-SORT: %d\n",total_max_sort);

    StringIteratorMDL_t sim;
    memset(&sim,0,sizeof(sim));
    sim.mdl	 = mdl;
    sim.sp	 = &mdl->spool;
    sim.str_func = string_copy;

    const MemItem_t *mi = GetMemListElem(&src_mdl->elem,0,0);
    const MemItem_t *mi_end = mi + src_mdl->elem.used;
    for ( ; mi < mi_end; mi++ )
    {
	//--- start with index corrections

	int sort_idx = -1, found = 0;
	for ( ptr = base; ptr < end; ptr++ )
	{
	    if ( ptr->idx1 == mi->idx1 && sort_idx < ptr->sort_idx )
		sort_idx = ptr->sort_idx;
	    if (!strcmp(ptr->name,mi->name))
	    {
		found++;
		break;
	    }
	}
	if (found)
	    continue;

	//if ( sort_idx < 0 )
	    sort_idx = total_max_sort;
	for ( ptr = base; ptr < end; ptr++ )
	    if ( ptr->sort_idx > sort_idx )
		ptr->sort_idx += 10;
	total_max_sort += 10;


	//--- insert element

	MemItem_t * item = AppendMIL(&mdl->elem);
	item->idx1 = mi->idx1;
	item->idx2 = mi->idx2;
	item->sort_idx = sort_idx + 10;
	item->name = STRDUP(mi->name);
	item->data = MEMDUP(mi->data,mi->size);
	item->size = mi->size;
	item->name_alloced = item->data_alloced = true;

	IterateStringsMDL(&sim,item,1);
    }

    return ERR_WARNING;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Patch MDL			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct mdl_patch_t
{
    patch_file_t	brres_type;	// type of related BRRES file
    u32			patch_status;	// mdl patch status

    bool		root_s1_valid;	// true: 'root_s1' is valid copy
    bool		is_transformed;	// true: is transformed by vectors
    mdl_sect1_t		root_s1;	// root section-1 element
    MatrixD_t		matrix;		// calculated matrix, if 'is_transformed'
    MatrixD_t		*mat;		// Matrix for transforming. Is a pointer to
					// either 'matrix' or 'opt_transform' (=init)
} mdl_patch_t;


///////////////////////////////////////////////////////////////////////////////

float3 TransformByMatrix
(
    float3  *val,
    float   m[3][4]
)
{
    DASSERT(val);
    DASSERT(m);

    float3 res;
    res.x = m[0][0] * val->x + m[0][1] * val->y + m[0][2] * val->z + m[0][3];
    res.y = m[1][0] * val->x + m[1][1] * val->y + m[1][2] * val->z + m[1][3];
    res.z = m[2][0] * val->x + m[2][1] * val->y + m[2][2] * val->z + m[2][3];
    return res;
}

///////////////////////////////////////////////////////////////////////////////

bool HavePatchMDL()
{
    LoadParametersMDL( verbose > 0 ? "  - " : 0 );
    if ( have_patch_count <= 0 || have_mdl_patch_count <= 0 )
	return false;

    return MDL_MODE & MDLMD_MATRIX || transform_active;
}

///////////////////////////////////////////////////////////////////////////////

static int patch_mdl
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    mdl_patch_t *mdlpat = it->param;
    DASSERT(mdlpat);

    if ( term || it->entry < 0 )
	return 0;

    if ( it->group == 1 )
    {
	mdl_sect1_t *ms = (mdl_sect1_t*)( it->szs->data + it->off );
	if ( be32(&ms->length) == sizeof(*ms) )
	{
	    if ( !mdlpat->root_s1_valid && !ms->parent_off )
	    {
		mdlpat->root_s1_valid = true;
		ntoh_MDLs1(&mdlpat->root_s1,ms);
		mdlpat->is_transformed
		    =  !( mdlpat->brres_type & PFILE_MINIMAP )
		    && !HaveStandardVectorsMDLs1(&mdlpat->root_s1,false);

		if (mdlpat->is_transformed)
		{
		    MatrixD_t temp;
		    ExportMatrixMDLs1( &mdlpat->root_s1, false, &temp, 1 );
		    MultiplyMatrixD( &mdlpat->matrix, &temp, &opt_transform, 0 );
		    MultiplyMatrixD( &mdlpat->matrix, 0, &temp, 2 );
		    mdlpat->mat = &mdlpat->matrix;
		    mdlpat->patch_status |= 2;
		}
	    }

	    if ( mdl_switched_to_vertex == 2 )
	    {
		mdl_switched_to_vertex++;
		ERROR0(ERR_WARNING,
		    "Because of the complex transformation matrix, it is"
		    " impossible to modify the scale and rotate vectors"
		    " of the MDL bones. So the VECTOR transformation is"
		    " disabled and the VERTEX transformation enabled"
		    " to support full transformation." );
	    }

	    if ( MDL_MODE & ( ms->parent_off ? MDLMD_CHILD_NODES : MDLMD_PARENT_NODE ))
	    {
		PRINT("  - PATCH MDLs1  '%s'\n", it->name );
		mdl_sect1_t ms1;
		ntoh_MDLs1(&ms1,ms);

		if ( MDL_MODE & MDLMD_VECTOR && opt_transform.norm_valid )
		{
		    if (opt_transform.scale_enabled)
		    {
			ms1.scale.x *= opt_transform.norm_scale.x;
			ms1.scale.y *= opt_transform.norm_scale.y;
			ms1.scale.z *= opt_transform.norm_scale.z;
			mdlpat->patch_status |= 1;
		    }

		    if (opt_transform.rotate_enabled)
		    {
			ms1.rotate.x += opt_transform.norm_rotate_deg.x;
			ms1.rotate.y += opt_transform.norm_rotate_deg.y;
			ms1.rotate.z += opt_transform.norm_rotate_deg.z;
			mdlpat->patch_status |= 1;
		    }

		    if (opt_transform.translate_enabled)
		    {
			ms1.translate.x += opt_transform.norm_translate.x;
			ms1.translate.y += opt_transform.norm_translate.y;
			ms1.translate.z += opt_transform.norm_translate.z;
			mdlpat->patch_status |= 1;
		    }
		}

		if ( MDL_MODE & MDLMD_MATRIX )
		{
		    CalcMatrixMDLs1(&ms1);
		    mdlpat->patch_status |= 1;
		}

		hton_MDLs1(ms,&ms1);

	     #ifdef TEST0
		PrintMatrixMDLs1(stdout,0,0,&ms1,13);
	     #endif
	    }
	}
    }
    else if ( it->group == 2 && MDL_MODE & MDLMD_VERTEX )
    {
	mdl_sect2_t *ms	    = (mdl_sect2_t*)( it->szs->data + it->off );
	void *data	    = (u8*)ms + be32(&ms->data_off);
	const uint format   = be32(&ms->format);
	const uint r_shift  = ms->r_shift;
	uint n_vertex	    = be16(&ms->n_vertex);
	noPRINT("N-VERTEX=%u, FORMAT=%u, R-SHIFT=%u\n",n_vertex,r_shift);

	if ( n_vertex > 0 )
	{
	    // [[2do]] optimization?: load N, transform N, store N

	    float3 min, max;
	    min.x = min.y = min.z = +INFINITY;
	    max.x = max.y = max.z = -INFINITY;

	    DASSERT(mdlpat->mat);
	    while ( n_vertex-- > 0 )
	    {
		float3 temp;
		GetVectorMDL(&temp,data,format,r_shift);
		temp = TransformF3MatrixD(mdlpat->mat,&temp);
		if (tform_script_enabled)
		    TformScriptCallF(3,&temp,1,0);
		data = SetVectorMDL(&temp,data,format,r_shift);

		if (IsNormalF3(temp.v))
		{
		    if ( min.x > temp.x ) min.x = temp.x;
		    if ( min.y > temp.y ) min.y = temp.y;
		    if ( min.z > temp.z ) min.z = temp.z;
		    if ( max.x < temp.x ) max.x = temp.x;
		    if ( max.y < temp.y ) max.y = temp.y;
		    if ( max.z < temp.z ) max.z = temp.z;
		}
	    }
	    write_bef4n(ms->minimum.v,min.v,3);
	    write_bef4n(ms->maximum.v,max.v,3);
	    mdlpat->patch_status |= 1;
	}
    }
 #ifdef TEST0
    else if ( it->group == 8 )
    {
	mdl_sect8_t *ms = (mdl_sect8_t*)( it->szs->data + it->off );
	const uint n_layer = ntohl(ms->n_layer);
	PRINT("### MDL SECTION 8, brres-type=%x, size=0x%x, n-lay=%u\n",
		it->szs->brres_type, be32(&ms->length), n_layer );

	uint layer;
	mdl_sect8_layer_t *msl = (mdl_sect8_layer_t*)( (u8*)ms + ntohl(ms->layer_off) );
	for ( layer = 0; layer < n_layer; layer++, msl++ )
	{
	    PRINT("\tLAYER #%u, off=%zx, min-filter=%x\n",
		layer, (u8*)msl - (u8*)ms, ntohl(msl->min_filter) );
	}
    }
 #endif
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint PatchRawDataMDL
(
    // returns
    //	0: MDL not patched
    //  1: MDL patched
    //  3: MDL patched & vector transformation recognized

    void		* data_ptr,	// data to scan
    uint		data_size,	// size of 'data'
    patch_file_t	brres_type,	// type of related BRRES file
    ccp			fname		// file name for error messages
)
{
    if (!HavePatchMDL())
	return false;

    MDL_ACTION_LOG(false,"PatchRawDataMDL(): %s\n",GetMdlMode());
    PRINT("PatchRawDataMDL(,,%x,): %s\n",brres_type,GetMdlMode());

    mdl_patch_t mdlpat;
    memset(&mdlpat,0,sizeof(mdlpat));
    mdlpat.brres_type = brres_type;
    mdlpat.mat = &opt_transform;

    szs_file_t szs;
// [[fname+]]
    AssignSZS(&szs,true,data_ptr,data_size,false,FF_MDL,fname);
    szs.fname = fname;
    IterateFilesParSZS(&szs,patch_mdl,&mdlpat,false,true,0,1,SORT_NONE);
    szs.fname = 0;
    ResetSZS(&szs);

    return mdlpat.patch_status & 1 ? mdlpat.patch_status : 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


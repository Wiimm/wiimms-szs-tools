
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

#include "lib-kmp.h"
//#include "lib-szs.h"
//#include "dclib-xdump.h"

#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define COL_OK   reset
#define COL_HINT yellow
#define COL_WARN red
#define COL_DIFF magenta
#define COL_SRC1 cyan
#define COL_SRC2 b_blue

#define OK_WARN0(a,b,n) a->n == b->n ? ok : warn
#define OK_WARN1(a,b,n) a->n == b->n ? ok : warn, a->n

//
///////////////////////////////////////////////////////////////////////////////
///////////////			section_info_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[section_info_t]]

typedef struct section_info_t
{
    kmp_entry_t	sect;		// section to compare

    kmp_t	*kmp1;		// first kmp to compare
    kmp_t	*kmp2;		// first kmp to compare
    uint	verbose;	// 0:quiet, 1:summary only, 2:full report

    char	sect_name[5];	// section name
    char	subject[16];	// subject for status messages
}
section_info_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			helper functions		///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_float
	( uint n, const float *f1, const float *f2, double epsilon )
{
    while ( n-- > 0 )
    {
	printf(" %s%11.3f",
		*f1 == *f2 ? colout->COL_OK
		: fabsf(*f1-*f2) > epsilon ? colout->COL_WARN : colout->COL_HINT,
		*f1 );
	f1++;
	f2++;
    }
}

//-----------------------------------------------------------------------------

static void print_rot ( uint n, const float *f1, const float *f2 )
{
    while ( n-- > 0 )
    {
	printf(" %s%7.2f",
		*f1 == *f2 ? colout->COL_OK
		: fabsf(*f1-*f2) > epsilon_rot ? colout->COL_WARN : colout->COL_HINT,
		*f1 );
	f1++;
	f2++;
    }
}

//-----------------------------------------------------------------------------

static void print_dec8_6 ( uint n, const u8 *u1, const u8 *u2 )
{
    while ( n-- > 0 )
    {
	printf(" %s%6s",
		*u1 == *u2 ? colout->COL_OK : colout->COL_WARN, PrintU8M1(*u1) );
	u1++;
	u2++;
    }
}

//-----------------------------------------------------------------------------

static void print_hex16 ( uint n, const u16 *u1, const u16 *u2 )
{
    while ( n-- > 0 )
    {
	printf(" %s%#6x",
		*u1 == *u2 ? colout->COL_OK : colout->COL_WARN, *u1 );
	u1++;
	u2++;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		ph: helper for CKPH, ENPH, ITPH		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sort_ph_t]]

typedef struct sort_ph_t
{
    int			rank;	// previous rank order
    kmp_enph_entry_t	data;	// original object data

} sort_ph_t;

//-----------------------------------------------------------------------------

static sort_ph_t * sort_ph ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_ph_t *res = CALLOC(list->used,sizeof(*res));
    sort_ph_t *ptr = res;
    kmp_enph_entry_t *e = (kmp_enph_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_ph );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static void print_ph
	( char mode, const kmp_enph_entry_t *p1, const kmp_enph_entry_t *p2 )
{
    if ( p2 && colout->colorize )
    {
	ccp ok   = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	const uint end1 = p1->pt_start + p1->pt_len - 1;
	const uint end2 = p2->pt_start + p2->pt_len - 1;
	printf("%c %s%3u %s%3u %s%3u %s:",
		mode,
		p1->pt_start == p2->pt_start ? ok : warn, p1->pt_start,
		end1 == end2 ? ok : colout->COL_HINT, end1,
		p1->pt_len == p2->pt_len ? ok : warn, p1->pt_len,
		colout->reset );

	uint j;
	const u8 *d1 = p1->prev;
	const u8 *d2 = p2->prev;
	for ( j = 0; j < 12; j++, d1++, d2++ )
	{
	    if ( j == 6 )
		printf(" %s:",colout->reset);
	    fputs( *d1 == *d2 ? ok : warn, stdout );
	    if ( *d1 == 0xff )
		fputs("   -",stdout);
	    else
		printf("%4u",*d1);
	}

	printf(" : %s0x%02x %s0x%02x%s\n",
		p1->setting[0] == p2->setting[0] ? ok : warn, p1->setting[0],
		p1->setting[1] == p2->setting[1] ? ok : warn, p1->setting[1],
		colout->reset );
    }
    else
    {
	printf("%c %3u %3u %3u :",
		mode, p1->pt_start, p1->pt_start + p1->pt_len - 1, p1->pt_len );

	uint j;
	const u8 *d1 = p1->prev;
	for ( j = 0; j < 12; j++, d1++ )
	{
	    if ( j == 6 )
		fputs(" :",stdout);
	    if ( *d1 == 0xff )
		fputs("   -",stdout);
	    else
		printf("%4u",*d1);
	}

	printf(" : 0x%02x 0x%02x\n", p1->setting[0], p1->setting[1] );
    }
}

//-----------------------------------------------------------------------------

static sort_ph_t * print_ph_single ( const section_info_t *si,
			char mode, const sort_ph_t *e, sort_ph_t *end )
{
    DASSERT(si);
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%s%s: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		si->sect_name, mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_ph(mode,&e->data,0);
	e++;
    }
    return (sort_ph_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_ph ( const sort_ph_t *e1, const sort_ph_t *e2 )
{
    uint size = sizeof(e1->data);
    if ( opt_battle_mode <= OFFON_OFF )
	size -= sizeof(e1->data.setting);
    if (!memcmp(&e1->data,&e2->data,size))
	return 0;

    uint count	= ( e1->data.pt_start != e2->data.pt_start )
		+ ( e1->data.pt_len   != e2->data.pt_len );

    if (memcmp(e1->data.prev,e2->data.prev,sizeof(e1->data.prev)))
	count++;
    if (memcmp(e1->data.next,e2->data.next,sizeof(e1->data.next)))
	count++;
    if (memcmp(e1->data.setting,e2->data.setting,sizeof(e1->data.setting)))
	count++;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_ph_si ( const section_info_t *si )
{
    DASSERT(si);
    DASSERT(si->kmp1);
    DASSERT(si->kmp2);

    uint diff_count = 0;
    List_t *l1 = si->kmp1->dlist + si->sect;
    List_t *l2 = si->kmp2->dlist + si->sect;

    if ( l1->used != l2->used )
    {
	if (!si->verbose)
	    return 1;
	printf("%s%s: Number of %s groups differ: Have %u and %u.%s\n",
		colout->COL_DIFF,
		si->sect_name, si->subject, l1->used, l2->used,
		colout->reset );
	if (si->verbose==1)
	    return 1;
	diff_count++;
    }

    sort_ph_t *s1 = sort_ph(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_ph_t *s2 = sort_ph(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_ph(e1,e2);
	if (stat)
	{
	    if ( si->verbose < 2 )
	    {
		if (si->verbose)
		    printf("%s%s: Data differ%s\n",
			colout->COL_DIFF, si->sect_name, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_ph_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_ph(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_ph_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_ph(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_ph_single(si,'<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_ph_single(si,'>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%s%s: %c%s group @%u:%u differ%s\n",
			colout->COL_DIFF,
			si->sect_name,
			toupper((int)si->subject[0]), si->subject+1,
			e1->rank, e2->rank, colout->reset );
	    print_ph('<',&e1->data,&e2->data);
	    print_ph('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_ph_single(si,'<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_ph_single(si,'>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (si->verbose)
	    printf("%s%s: Order of groups differ%s\n",
			colout->COL_DIFF, si->sect_name, colout->reset );
	if ( si->verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		pt: helper for ENPT, ITPT		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sort_pt_t]]

typedef struct sort_pt_t
{
    int			rank;	// previous rank order
    kmp_enpt_entry_t	data;	// original object data

} sort_pt_t;

//-----------------------------------------------------------------------------

static sort_pt_t * sort_pt ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_pt_t *res = CALLOC(list->used,sizeof(*res));
    sort_pt_t *ptr = res;
    kmp_enpt_entry_t *e = (kmp_enpt_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_pt );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static void print_pt
	( char mode, const kmp_enpt_entry_t *p1, const kmp_enpt_entry_t *p2 )
{
    if ( p2 && colout->colorize )
    {
	ccp ok   = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	putchar(mode);
	putchar(' ');
	print_float(3,p1->position,p2->position,epsilon_pos);

	printf("  %s%7.3f  %s%#6x %s%#6x%s\n",
		p1->scale == p2->scale ? ok
			: fabsf(p1->scale-p2->scale) > epsilon_scale ? colout->COL_WARN
			: colout->COL_HINT,
		p1->scale,
		p1->prop[0] == p2->prop[0] ? ok : warn, p1->prop[0],
		p1->prop[1] == p2->prop[1] ? ok : warn, p1->prop[1],
		colout->reset );
    }
    else
    {
	printf("%c  %11.3f %11.3f %11.3f  %7.3f  %#6x %#6x\n",
		mode,
		p1->position[0], p1->position[1], p1->position[2],
		p1->scale,
		p1->prop[0], p1->prop[1] );
    }
}

//-----------------------------------------------------------------------------

static sort_pt_t * print_pt_single ( const section_info_t *si,
			char mode, const sort_pt_t *e, sort_pt_t *end )
{
    DASSERT(si);
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%s%s: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		si->sect_name, mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_pt(mode,&e->data,0);
	e++;
    }
    return (sort_pt_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_pt ( const sort_pt_t *e1, const sort_pt_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    uint count	= ( e1->data.prop[0]	!= e2->data.prop[0] )
		+ ( e1->data.prop[1]	!= e2->data.prop[1] );

    count += fabsf( e1->data.position[0] - e2->data.position[0] ) > epsilon_pos
	  || fabsf( e1->data.position[1] - e2->data.position[1] ) > epsilon_pos
	  || fabsf( e1->data.position[2] - e2->data.position[2] ) > epsilon_pos;

    count += fabsf( e1->data.scale - e2->data.scale ) > epsilon_scale;
    return count;
}

//-----------------------------------------------------------------------------

static uint diff_pt_si ( const section_info_t *si )
{
    DASSERT(si);
    DASSERT(si->kmp1);
    DASSERT(si->kmp2);

    uint diff_count = 0;
    List_t *l1 = si->kmp1->dlist + si->sect;
    List_t *l2 = si->kmp2->dlist + si->sect;

    if ( l1->used != l2->used )
    {
	if (!si->verbose)
	    return 1;
	printf("%s%s: Number of %ss differ: Have %u and %u.%s\n",
		colout->COL_DIFF,
		si->sect_name, si->subject, l1->used, l2->used,
		colout->reset );
	if (si->verbose==1)
	    return 1;
	diff_count++;
    }

    sort_pt_t *s1 = sort_pt(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_pt_t *s2 = sort_pt(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_pt(e1,e2);
	if (stat)
	{
	    if ( si->verbose < 2 )
	    {
		if (si->verbose)
		    printf("%s%s: Data differ%s\n",
			colout->COL_DIFF, si->sect_name, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_pt_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_pt(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_pt_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_pt(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_pt_single(si,'<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_pt_single(si,'>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%s%s: %s @%u:%u differ%s\n",
			colout->COL_DIFF,
			si->sect_name, si->subject,
			e1->rank, e2->rank, colout->reset );
	    print_pt('<',&e1->data,&e2->data);
	    print_pt('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_pt_single(si,'<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_pt_single(si,'>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (si->verbose)
	    printf("%s%s: Order of %ss differ%s\n",
			colout->COL_DIFF, si->sect_name,
			si->subject, colout->reset );
	if ( si->verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		pt2: helper for CNPT, JGPT, MSPT	///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sort_pt2_t]]

typedef struct sort_pt2_t
{
    int			rank;	// previous rank order
    kmp_jgpt_entry_t	data;	// original object data

} sort_pt2_t;

//-----------------------------------------------------------------------------

static sort_pt2_t * sort_pt2 ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_pt2_t *res = CALLOC(list->used,sizeof(*res));
    sort_pt2_t *ptr = res;
    kmp_jgpt_entry_t *e = (kmp_jgpt_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_pt2 );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static void print_pt2
	( char mode, const kmp_jgpt_entry_t *p1, const kmp_jgpt_entry_t *p2 )
{
    if ( p2 && colout->colorize )
    {
	ccp ok  = colout->COL_OK;

	putchar(mode);
	print_float(3,p1->position,p2->position,epsilon_pos);
	putchar(' ');
	print_rot(3,p1->rotation,p2->rotation);

	printf("  %s%6d %s%#6x%s\n",
		p1->id     == p2->id     ? ok : colout->COL_HINT, p1->id,
		p1->effect == p2->effect ? ok : colout->COL_WARN, p1->effect,
		colout->reset );
    }
    else
    {
	printf("%c %11.3f %11.3f %11.3f  %7.2f %7.2f %7.2f  %6d %#6x\n",
		mode,
		p1->position[0], p1->position[1], p1->position[2],
		p1->rotation[0], p1->rotation[1], p1->rotation[2],
		p1->id, p1->effect );
    }
}

//-----------------------------------------------------------------------------

static sort_pt2_t * print_pt2_single ( const section_info_t *si,
			char mode, const sort_pt2_t *e, sort_pt2_t *end )
{
    DASSERT(si);
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%s%s: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		si->sect_name, mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_pt2(mode,&e->data,0);
	e++;
    }
    return (sort_pt2_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_pt2 ( const sort_pt2_t *e1, const sort_pt2_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    // 'data.id' ignored!
    uint count	= e1->data.effect != e2->data.effect;

    count += fabsf( e1->data.position[0] - e2->data.position[0] ) > epsilon_pos
	  || fabsf( e1->data.position[1] - e2->data.position[1] ) > epsilon_pos
	  || fabsf( e1->data.position[2] - e2->data.position[2] ) > epsilon_pos;

    count += fabsf( e1->data.rotation[0] - e2->data.rotation[0] ) > epsilon_rot
	  || fabsf( e1->data.rotation[1] - e2->data.rotation[1] ) > epsilon_rot
	  || fabsf( e1->data.rotation[2] - e2->data.rotation[2] ) > epsilon_rot;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_pt2_si ( const section_info_t *si )
{
    DASSERT(si);
    DASSERT(si->kmp1);
    DASSERT(si->kmp2);

    uint diff_count = 0;
    List_t *l1 = si->kmp1->dlist + si->sect;
    List_t *l2 = si->kmp2->dlist + si->sect;

    if ( l1->used != l2->used )
    {
	if (!si->verbose)
	    return 1;
	printf("%s%s: Number of %s points differ: Have %u and %u.%s\n",
		colout->COL_DIFF,
		si->sect_name, si->subject, l1->used, l2->used,
		colout->reset );
	if (si->verbose==1)
	    return 1;
	diff_count++;
    }

    sort_pt2_t *s1 = sort_pt2(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_pt2_t *s2 = sort_pt2(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_pt2(e1,e2);
	if (stat)
	{
	    if ( si->verbose < 2 )
	    {
		if (si->verbose)
		    printf("%s%s: Data differ%s\n",
			colout->COL_DIFF, si->sect_name, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_pt2_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_pt2(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_pt2_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_pt2(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_pt2_single(si,'<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_pt2_single(si,'>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%s%s: %s group @%u:%u differ%s\n",
			colout->COL_DIFF,
			si->sect_name, si->subject,
			e1->rank, e2->rank, colout->reset );
	    print_pt2('<',&e1->data,&e2->data);
	    print_pt2('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_pt2_single(si,'<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_pt2_single(si,'>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (si->verbose)
	    printf("%s%s: Order of %s points differ%s\n",
			colout->COL_DIFF, si->sect_name,
			si->subject, colout->reset );
	if ( si->verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_AREA()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct sort_area_t
{
    int			rank;	// previous rank order
    kmp_area_entry_t	data;	// original object data

} sort_area_t;

//-----------------------------------------------------------------------------

static sort_area_t * sort_area ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_area_t *res = CALLOC(list->used,sizeof(*res));
    sort_area_t *ptr = res;
    kmp_area_entry_t *e = (kmp_area_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_area );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static void print_area
	( char mode, const kmp_area_entry_t *e1, const kmp_area_entry_t *e2 )
{
    if ( e2 && colout->colorize )
    {
	ccp ok   = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	printf("%c %s%5u  %s%5u",
		mode, OK_WARN1(e1,e2,mode), OK_WARN1(e1,e2,type) );
	print_float(3,e1->position,e2->position,epsilon_pos);
	putchar(' ');
	print_hex16(2,e1->setting,e2->setting);

	printf("%s\n%c %s%5u  %s%5u",
		colout->reset, mode,
		OK_WARN1(e1,e2,dest_id), OK_WARN1(e1,e2,prio) );
	print_float(3,e1->rotation,e2->rotation,epsilon_rot);
	putchar(' ');
	print_dec8_6(2,&e1->route,&e2->route);
	print_hex16(1,&e1->unknown_2e,&e2->unknown_2e);

	printf("%s\n%-14c", colout->reset, mode );
	print_float(3,e1->scale,e2->scale,epsilon_scale);

	printf("%s\n",colout->reset);
    }
    else
    {
	printf(	"%c %5u %5u  %11.3f %11.3f %11.3f  %#6x %#6x\n"
		"%c %5u %5u  %11.3f %11.3f %11.3f  %6s %6s %#6x\n"
		"%c %24.3f %11.3f %11.3f\n",
		mode,
		 e1->mode, e1->type,
		 e1->position[0], e1->position[1], e1->position[2],
		 e1->setting[0], e1->setting[1],
		mode,
		 e1->dest_id, e1->prio,
		 e1->rotation[0], e1->rotation[1], e1->rotation[2],
		 PrintU8M1(e1->route), PrintU8M1(e1->enemy), e1->unknown_2e,
		mode,
		e1->scale[0], e1->scale[1], e1->scale[2] );
    }
}

//-----------------------------------------------------------------------------

static sort_area_t * print_area_single
	( char mode, const sort_area_t *e, sort_area_t *end )
{
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%sAREA: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_area(mode,&e->data,0);
	e++;
    }
    return (sort_area_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_area ( const sort_area_t *e1, const sort_area_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    uint count	= ( e1->data.mode	!= e2->data.mode )
		+ ( e1->data.type	!= e2->data.type )
		+ ( e1->data.dest_id	!= e2->data.dest_id )
		+ ( e1->data.prio	!= e2->data.prio )
		+ ( e1->data.setting[0]	!= e2->data.setting[0] )
		+ ( e1->data.setting[1]	!= e2->data.setting[1] )
		+ ( e1->data.route	!= e2->data.route )
		+ ( e1->data.enemy	!= e2->data.enemy )
		+ ( e1->data.unknown_2e	!= e2->data.unknown_2e );

    count += fabsf( e1->data.position[0] - e2->data.position[0] ) > epsilon_pos
	  || fabsf( e1->data.position[1] - e2->data.position[1] ) > epsilon_pos
	  || fabsf( e1->data.position[2] - e2->data.position[2] ) > epsilon_pos;

    count += fabsf( e1->data.rotation[0] - e2->data.rotation[0] ) > epsilon_rot
	  || fabsf( e1->data.rotation[1] - e2->data.rotation[1] ) > epsilon_rot
	  || fabsf( e1->data.rotation[2] - e2->data.rotation[2] ) > epsilon_rot;

    count += fabsf( e1->data.scale[0] - e2->data.scale[0] ) > epsilon_scale
	  || fabsf( e1->data.scale[1] - e2->data.scale[1] ) > epsilon_scale
	  || fabsf( e1->data.scale[2] - e2->data.scale[2] ) > epsilon_scale;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_KMP_AREA
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *l1 = kmp1->dlist + KMP_AREA;
    List_t *l2 = kmp2->dlist + KMP_AREA;

    if ( l1->used != l2->used )
    {
	if (!verbose)
	    return 1;
	printf("%sAREA: Number of areas differ: Have %u and %u.%s\n",
		colout->COL_WARN, l1->used, l2->used, colout->reset );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    sort_area_t *s1 = sort_area(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_area_t *s2 = sort_area(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_area(e1,e2);
	if (stat)
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    printf("%sAREA: Data differ%s\n",
			colout->COL_DIFF, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_area_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_area(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_area_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_area(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_area_single('<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_area_single('>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%sAREA: Area @%u:%u differ%s\n",
			colout->COL_DIFF, e1->rank,e2->rank,
			colout->reset );
	    print_area('<',&e1->data,&e2->data);
	    print_area('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_area_single('<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_area_single('>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (verbose)
	    printf("%sAREA: Order of areas differ%s\n",
			colout->COL_DIFF, colout->reset );
	if ( verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_CAME()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct sort_came_t
{
    int			rank;	// previous rank order
    kmp_came_entry_t	data;	// original object data

} sort_came_t;

//-----------------------------------------------------------------------------

static sort_came_t * sort_came ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_came_t *res = CALLOC(list->used,sizeof(*res));
    sort_came_t *ptr = res;
    kmp_came_entry_t *e = (kmp_came_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_came );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static void print_came
	( char mode, const kmp_came_entry_t *e1, const kmp_came_entry_t *e2 )
{
    if ( e2 && colout->colorize )
    {
	ccp ok   = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	printf("%c %s%5u %s%5u ",
		mode,
		OK_WARN1(e1,e2,type),
		OK_WARN1(e1,e2,came_speed) );
	print_float(3,e1->position,e2->position,epsilon_pos);
	putchar(' ');
	print_float(1,&e1->zoom_begin,&e2->zoom_begin,epsilon_pos);

	printf("%s\n%c %s%5s %s%5u ",
		colout->reset,
		mode,
		OK_WARN0(e1,e2,next),PrintU8M1(e1->next),
		OK_WARN1(e1,e2,zoom_speed) );
	print_float(3,e1->rotation,e2->rotation,epsilon_pos);
	putchar(' ');
	print_float(1,&e1->zoom_end,&e2->zoom_end,epsilon_pos);

	printf("%s\n%c %s%5u %s%5u ",
		colout->reset,
		mode,
		OK_WARN1(e1,e2,unknown_0x02),
		OK_WARN1(e1,e2,viewpt_speed) );
	print_float(3,e1->viewpt_begin,e2->viewpt_begin,epsilon_pos);

	printf("%s\n%c %s%5s %s%5u ",
		colout->reset,
		mode,
		OK_WARN0(e1,e2,route),PrintU8M1(e1->route),
		OK_WARN1(e1,e2,unknown_0x0a) );
	print_float(3,e1->viewpt_end,e2->viewpt_end,epsilon_pos);
	putchar(' ');
	print_float(1,&e1->time,&e2->time,epsilon_time);

	printf("%s\n",colout->reset);
    }
    else
    {
	printf(	"%c %5u %5u  %11.3f %11.3f %11.3f  %11.3f\n"
		"%c %5s %5u  %11.3f %11.3f %11.3f  %11.3f\n"
		"%c %5u %5u  %11.3f %11.3f %11.3f\n"
		"%c %5s %5u  %11.3f %11.3f %11.3f  %11.3f\n",
		mode,
		 e1->type, e1->came_speed,
		 e1->position[0], e1->position[1], e1->position[2],
		 e1->zoom_begin,
		mode,
		 PrintU8M1(e1->next), e1->zoom_speed,
		 e1->rotation[0], e1->rotation[1], e1->rotation[2],
		 e1->zoom_end,
		mode,
		 e1->unknown_0x02, e1->viewpt_speed,
		 e1->viewpt_begin[0], e1->viewpt_begin[1], e1->viewpt_begin[2],
		mode,
		 PrintU8M1(e1->route), e1->unknown_0x0a,
		 e1->viewpt_end[0], e1->viewpt_end[1], e1->viewpt_end[2],
		 e1->time );
    }
}

//-----------------------------------------------------------------------------

static sort_came_t * print_came_single
	( char mode, const sort_came_t *e, sort_came_t *end )
{
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%sCAME: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_came(mode,&e->data,0);
	e++;
    }
    return (sort_came_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_came ( const sort_came_t *e1, const sort_came_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    uint count	= ( e1->data.type		!= e2->data.type )
		+ ( e1->data.next		!= e2->data.next )
		+ ( e1->data.unknown_0x02	!= e2->data.unknown_0x02 )
		+ ( e1->data.route		!= e2->data.route )
		+ ( e1->data.came_speed	!= e2->data.came_speed )
		+ ( e1->data.zoom_speed	!= e2->data.zoom_speed )
		+ ( e1->data.viewpt_speed	!= e2->data.viewpt_speed )
		+ ( e1->data.unknown_0x0a	!= e2->data.unknown_0x0a );

    count += fabsf( e1->data.position[0] - e2->data.position[0] ) > epsilon_pos
	  || fabsf( e1->data.position[1] - e2->data.position[1] ) > epsilon_pos
	  || fabsf( e1->data.position[2] - e2->data.position[2] ) > epsilon_pos;

    count += fabsf( e1->data.rotation[0] - e2->data.rotation[0] ) > epsilon_rot
	  || fabsf( e1->data.rotation[1] - e2->data.rotation[1] ) > epsilon_rot
	  || fabsf( e1->data.rotation[2] - e2->data.rotation[2] ) > epsilon_rot;

    count += fabsf( e1->data.zoom_begin - e2->data.zoom_begin ) > epsilon_scale
	  || fabsf( e1->data.zoom_end   - e2->data.zoom_end   ) > epsilon_scale;

    count += fabsf( e1->data.viewpt_begin[0] - e2->data.viewpt_begin[0] ) > epsilon_pos
	  || fabsf( e1->data.viewpt_begin[1] - e2->data.viewpt_begin[1] ) > epsilon_pos
	  || fabsf( e1->data.viewpt_begin[2] - e2->data.viewpt_begin[2] ) > epsilon_pos;


    count += fabsf( e1->data.viewpt_end[0] - e2->data.viewpt_end[0] ) > epsilon_pos
	  || fabsf( e1->data.viewpt_end[1] - e2->data.viewpt_end[1] ) > epsilon_pos
	  || fabsf( e1->data.viewpt_end[2] - e2->data.viewpt_end[2] ) > epsilon_pos;

    count += fabsf( e1->data.time - e2->data.time ) > epsilon_time;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_KMP_CAME
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *l1 = kmp1->dlist + KMP_CAME;
    List_t *l2 = kmp2->dlist + KMP_CAME;

    if ( l1->used != l2->used )
    {
	if (!verbose)
	    return 1;
	printf("%sCAME: Number of cameras differ: Have %u and %u.%s\n",
		colout->COL_WARN, l1->used, l2->used, colout->reset );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    sort_came_t *s1 = sort_came(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_came_t *s2 = sort_came(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_came(e1,e2);
	if (stat)
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    printf("%sCAME: Data differ%s\n",
			colout->COL_DIFF, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_came_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_came(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_came_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_came(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_came_single('<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_came_single('>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%sCAME: Camera @%u:%u differ%s\n",
			colout->COL_DIFF, e1->rank,e2->rank,
			colout->reset );
	    print_came('<',&e1->data,&e2->data);
	    print_came('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_came_single('<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_came_single('>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (verbose)
	    printf("%sCAME: Order of cameras differ%s\n",
			colout->COL_DIFF, colout->reset );
	if ( verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_CKPH()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_CKPH
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_CKPH,kmp1,kmp2,verbose,"CKPH","check point"};
    return diff_ph_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_CKPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct sort_ckpt_t
{
    int			rank;	// previous rank order
    kmp_ckpt_entry_t	data;	// original object data

} sort_ckpt_t;

//-----------------------------------------------------------------------------

static sort_ckpt_t * sort_ckpt ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_ckpt_t *res = CALLOC(list->used,sizeof(*res));
    sort_ckpt_t *ptr = res;
    kmp_ckpt_entry_t *e = (kmp_ckpt_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_ckpt );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static void print_ckpt
	( char mode, const kmp_ckpt_entry_t *e1, const kmp_ckpt_entry_t *e2 )
{
    if ( e2 && colout->colorize )
    {
	ccp ok   = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	putchar(mode);
	print_float(2,e1->left,e2->left,epsilon_pos);
	putchar(' ');
	print_float(2,e1->right,e2->right,epsilon_pos);

	printf("  %s%3d %s%3s %s%3s %s%3s%s\n",
		OK_WARN1(e1,e2,respawn),
		OK_WARN0(e1,e2,mode), PrintU8M1(e1->mode),
		OK_WARN0(e1,e2,prev), PrintU8M1(e1->prev),
		OK_WARN0(e1,e2,next), PrintU8M1(e1->next),
		colout->reset );
    }
    else
    {
	printf("%c %11.3f %11.3f  %11.3f %11.3f  %3d %3s %3s %3s\n",
		mode,
		e1->left[0], e1->left[1],
		e1->right[0], e1->right[1],
		e1->respawn,
		PrintU8M1(e1->mode),
		PrintU8M1(e1->prev),
		PrintU8M1(e1->next) );
    }
}

//-----------------------------------------------------------------------------

static sort_ckpt_t * print_ckpt_single
	( char mode, const sort_ckpt_t *e, sort_ckpt_t *end )
{
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%sCKPT: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_ckpt(mode,&e->data,0);
	e++;
    }
    return (sort_ckpt_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_ckpt ( const sort_ckpt_t *e1, const sort_ckpt_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    uint count	= ( e1->data.respawn	!= e2->data.respawn )
		+ ( e1->data.mode	!= e2->data.mode )
		+ ( e1->data.prev	!= e2->data.prev )
		+ ( e1->data.next	!= e2->data.next );

    count += fabsf( e1->data.left[0] - e2->data.left[0] ) > epsilon_pos
	  || fabsf( e1->data.left[1] - e2->data.left[1] ) > epsilon_pos;

    count += fabsf( e1->data.right[0] - e2->data.right[0] ) > epsilon_pos
	  || fabsf( e1->data.right[1] - e2->data.right[1] ) > epsilon_pos;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_KMP_CKPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *l1 = kmp1->dlist + KMP_CKPT;
    List_t *l2 = kmp2->dlist + KMP_CKPT;

    if ( l1->used != l2->used )
    {
	if (!verbose)
	    return 1;
	printf("%sCKPT: Number of check points differ: Have %u and %u.%s\n",
		colout->COL_WARN, l1->used, l2->used, colout->reset );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    sort_ckpt_t *s1 = sort_ckpt(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_ckpt_t *s2 = sort_ckpt(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_ckpt(e1,e2);
	if (stat)
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    printf("%sCKPT: Data differ%s\n",
			colout->COL_DIFF, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_ckpt_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_ckpt(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_ckpt_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_ckpt(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_ckpt_single('<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_ckpt_single('>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%sCKPT: CKPT @%u:%u differ%s\n",
			colout->COL_DIFF, e1->rank,e2->rank,
			colout->reset );
	    print_ckpt('<',&e1->data,&e2->data);
	    print_ckpt('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_ckpt_single('<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_ckpt_single('>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (verbose)
	    printf("%sCKPT: Order of check points differ%s\n",
			colout->COL_DIFF, colout->reset );
	if ( verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_CNPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_CNPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_CNPT,kmp1,kmp2,verbose,"CNPT","cannon"};
    return diff_pt2_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_ENPH()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_ENPH
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_ENPH,kmp1,kmp2,verbose,"ENPH","enemy point"};
    return diff_ph_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_ENPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_ENPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_ENPT,kmp1,kmp2,verbose,"ENPT","enemy point"};
    return diff_pt_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_GOBJ()			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sort_gobj_t]]

typedef struct sort_gobj_t
{
    int			obj_id;	// relevant object id
    int			rank;	// previous rank order
    kmp_gobj_entry_t	data;	// original object data

} sort_gobj_t;

//-----------------------------------------------------------------------------

static int compare_sort_gobj ( const sort_gobj_t * a, const sort_gobj_t * b )
{
    int stat = a->obj_id - b->obj_id;
    if (!stat)
	stat = a->rank - b->rank;
    return stat;
}

//-----------------------------------------------------------------------------

static sort_gobj_t * sort_gobj ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_gobj_t *res = CALLOC(list->used,sizeof(*res));
    sort_gobj_t *sg = res;
    kmp_gobj_entry_t *e = (kmp_gobj_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, sg++ )
    {
	sg->obj_id = e->obj_id & ~GOBJ_M_LECODE;
	sg->rank = i;
	memcpy(&sg->data,e,sizeof(sg->data));
    }

    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_gobj );
    return res;
}

//-----------------------------------------------------------------------------

static void print_gobj ( char mode, const kmp_gobj_entry_t *g1,
		const kmp_gobj_entry_t *g2, const ObjectInfo_t *oi )
{
    char buf1[32], buf2[32];
    if (!oi)
	oi = GetObjectInfo(g1->obj_id);
    DASSERT(oi);

    if ( g2 && colout->colorize )
    {
	ccp ok  = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	printf("%c %s%#6x",
		mode, g1->obj_id == g2->obj_id ? ok : warn, g1->obj_id );
	print_float(3,g1->position,g2->position,epsilon_pos);

	fputs("  ",stdout);
	printf( oi->format1 ? oi->format1->col : settings_format_uuuu.col,
		g1->setting[0] == g2->setting[0] ? ok : warn , g1->setting[0],
		g1->setting[1] == g2->setting[1] ? ok : warn , g1->setting[1],
		g1->setting[2] == g2->setting[2] ? ok : warn , g1->setting[2],
		g1->setting[3] == g2->setting[3] ? ok : warn , g1->setting[3] );

	printf("  %s%6s%s\n%c %s%#6x",
		g1->route_id == g2->route_id ? ok : warn, PrintU16M1(g1->route_id),
		colout->reset,
		mode,
		g1->ref_id == g2->ref_id ? ok : warn, g1->ref_id );
	print_float(3,g1->rotation,g2->rotation,epsilon_rot);

	fputs("  ",stdout);
	printf(	oi->format2 ? oi->format2->col : settings_format_uuuu.col,
		g1->setting[4] == g2->setting[4] ? ok : warn , g1->setting[4],
		g1->setting[5] == g2->setting[5] ? ok : warn , g1->setting[5],
		g1->setting[6] == g2->setting[6] ? ok : warn , g1->setting[6],
		g1->setting[7] == g2->setting[7] ? ok : warn , g1->setting[7] );

	printf("  %s%#6x%s\n%c       ",
		g1->pflags == g2->pflags ? ok : warn, g1->pflags,
		colout->reset, mode );
	print_float(3,g1->scale,g2->scale,epsilon_scale);

	printf(	"%s\n",colout->reset);
    }
    else
    {
	snprintf( buf1, sizeof(buf1),
		oi->format1 ? oi->format1->std : settings_format_uuuu.std,
		g1->setting[0], g1->setting[1], g1->setting[2], g1->setting[3] );

	snprintf( buf2, sizeof(buf2),
		oi->format2 ? oi->format2->std : settings_format_uuuu.std,
		g1->setting[4], g1->setting[5], g1->setting[6], g1->setting[7] );

	printf(	"%c %#6x %11.3f %11.3f %11.3f  %s  %6s\n"
		"%c %#6x %11.3f %11.3f %11.3f  %s  %#6x\n"
		"%c        %11.3f %11.3f %11.3f\n",
		mode, g1->obj_id,
		 g1->position[0], g1->position[1], g1->position[2],
		 buf1, PrintU16M1(g1->route_id),
		mode, g1->ref_id,
		 g1->rotation[0], g1->rotation[1], g1->rotation[2],
		 buf2, g1->pflags,
		mode, g1->scale[0], g1->scale[1], g1->scale[2]
		);
    }
}

//-----------------------------------------------------------------------------

static sort_gobj_t * print_gobj_single
	( char mode, const sort_gobj_t *e, sort_gobj_t *end )
{
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	const ObjectInfo_t *oi = GetObjectInfo(e->data.obj_id);
	printf("%sGOBJ: Only found in source %c (@%u,%s)%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		mode == '<' ? '1' : '2', e->rank,
		oi->name ? oi->name : "?",
		colout->reset );
	print_gobj(mode,&e->data,0,0);
	e++;
    }
    return (sort_gobj_t*)e;
}

//-----------------------------------------------------------------------------
// return 0:equal, >0:number of different elements, 99:different relevant obj_id

static int diff_gobj ( const sort_gobj_t *e1, const sort_gobj_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    if ( e1->obj_id != e2->obj_id )
	return 99;

    uint count	= ( e1->data.obj_id	!= e2->data.obj_id )
		+ ( e1->data.ref_id	!= e2->data.ref_id )
		+ ( e1->data.route_id	!= e2->data.route_id )
		+ ( e1->data.pflags	!= e2->data.pflags );

    uint i;
    for ( i = 0; i < 8; i++ )
	count += e1->data.setting[i] != e2->data.setting[i];

    count += fabsf( e1->data.position[0] - e2->data.position[0] ) > epsilon_pos
	  || fabsf( e1->data.position[1] - e2->data.position[1] ) > epsilon_pos
	  || fabsf( e1->data.position[2] - e2->data.position[2] ) > epsilon_pos;

    count += fabsf( e1->data.rotation[0] - e2->data.rotation[0] ) > epsilon_rot
	  || fabsf( e1->data.rotation[1] - e2->data.rotation[1] ) > epsilon_rot
	  || fabsf( e1->data.rotation[2] - e2->data.rotation[2] ) > epsilon_rot;

    count += fabsf( e1->data.scale[0] - e2->data.scale[0] ) > epsilon_scale
	  || fabsf( e1->data.scale[1] - e2->data.scale[1] ) > epsilon_scale
	  || fabsf( e1->data.scale[2] - e2->data.scale[2] ) > epsilon_scale;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_KMP_GOBJ
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *l1 = kmp1->dlist + KMP_GOBJ;
    List_t *l2 = kmp2->dlist + KMP_GOBJ;

    if ( l1->used != l2->used )
    {
	if (!verbose)
	    return 1;
	printf("%sGOBJ: Number of objects differ: Have %u and %u.%s\n",
		colout->COL_DIFF, l1->used, l2->used, colout->reset );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    sort_gobj_t *sg1 = sort_gobj(l1), *e1 = sg1, *end1 = e1 + l1->used;
    sort_gobj_t *sg2 = sort_gobj(l2), *e2 = sg2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	if ( e1->obj_id < e2->obj_id )
	{
	    e1 = print_gobj_single('<',e1,e1+1);
	    diff_count++;
	    continue;
	}

	if ( e2->obj_id < e1->obj_id )
	{
	    e2 = print_gobj_single('>',e2,e2+1);
	    diff_count++;
	    continue;
	}

	const int stat = diff_gobj(e1,e2);
	if (stat)
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    printf("%sGOBJ: Data differ%s\n",
			colout->COL_DIFF, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_gobj_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_gobj(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_gobj_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_gobj(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    //e1 = print_gobj_single('<',e1,p1);
		    e1 = print_gobj_single('<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    //e2 = print_gobj_single('>',e2,p2);
		    e2 = print_gobj_single('>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    const ObjectInfo_t *oi = GetObjectInfo(e1->data.obj_id);
	    printf("%sGOBJ: Object @%u:%u (%s) differ%s\n",
			colout->COL_DIFF, e1->rank,e2->rank,
			oi->name ? oi->name : "?", colout->reset );
	    print_gobj('<',&e1->data,&e2->data,oi);
	    print_gobj('>',&e2->data,&e1->data,oi);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_gobj_single('<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_gobj_single('>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (verbose)
	    printf("%sGOBJ: Order of objects differ%s\n",
			colout->COL_DIFF, colout->reset );
	if ( verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(sg1);
    FREE(sg2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_ITPH()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_ITPH
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_ITPH,kmp1,kmp2,verbose,"ITPH","item point"};
    return diff_ph_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_ITPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_ITPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_ITPT,kmp1,kmp2,verbose,"ITPT","item point"};
    return diff_pt_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_JGPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_JGPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_JGPT,kmp1,kmp2,verbose,"JGPT","respawn"};
    return diff_pt2_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_KTPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct sort_ktpt_t
{
    int			rank;	// previous rank order
    kmp_ktpt_entry_t	data;	// original object data

} sort_ktpt_t;

//-----------------------------------------------------------------------------

static sort_ktpt_t * sort_ktpt ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_ktpt_t *res = CALLOC(list->used,sizeof(*res));
    sort_ktpt_t *ptr = res;
    kmp_ktpt_entry_t *e = (kmp_ktpt_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_ktpt );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static inline void print_ktpt
	( char mode, const kmp_ktpt_entry_t *e1, const kmp_ktpt_entry_t *e2 )
{
    print_pt2(mode,(kmp_jgpt_entry_t*)e1,(kmp_jgpt_entry_t*)e2);
}

//-----------------------------------------------------------------------------

static sort_ktpt_t * print_ktpt_single
	( char mode, const sort_ktpt_t *e, sort_ktpt_t *end )
{
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%sKTPT: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_ktpt(mode,&e->data,0);
	e++;
    }
    return (sort_ktpt_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_ktpt ( const sort_ktpt_t *e1, const sort_ktpt_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    uint count	= ( e1->data.player_index	!= e2->data.player_index )
		+ ( e1->data.unknown		!= e2->data.unknown );

    count += fabsf( e1->data.position[0] - e2->data.position[0] ) > epsilon_pos
	  || fabsf( e1->data.position[1] - e2->data.position[1] ) > epsilon_pos
	  || fabsf( e1->data.position[2] - e2->data.position[2] ) > epsilon_pos;

    count += fabsf( e1->data.rotation[0] - e2->data.rotation[0] ) > epsilon_rot
	  || fabsf( e1->data.rotation[1] - e2->data.rotation[1] ) > epsilon_rot
	  || fabsf( e1->data.rotation[2] - e2->data.rotation[2] ) > epsilon_rot;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_KMP_KTPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *l1 = kmp1->dlist + KMP_KTPT;
    List_t *l2 = kmp2->dlist + KMP_KTPT;

    if ( l1->used != l2->used )
    {
	if (!verbose)
	    return 1;
	printf("%sKTPT: Number of start positions differ: Have %u and %u.%s\n",
		colout->COL_WARN, l1->used, l2->used, colout->reset );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    sort_ktpt_t *s1 = sort_ktpt(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_ktpt_t *s2 = sort_ktpt(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_ktpt(e1,e2);
	if (stat)
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    printf("%sKTPT: Data differ%s\n",
			colout->COL_DIFF, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_ktpt_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_ktpt(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_ktpt_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_ktpt(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_ktpt_single('<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_ktpt_single('>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%sKTPT: KTPT @%u:%u differ%s\n",
			colout->COL_DIFF, e1->rank,e2->rank,
			colout->reset );
	    print_ktpt('<',&e1->data,&e2->data);
	    print_ktpt('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_ktpt_single('<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_ktpt_single('>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (verbose)
	    printf("%sKTPT: Order of start positions differ%s\n",
			colout->COL_DIFF, colout->reset );
	if ( verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_MSPT()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint diff_KMP_MSPT
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    section_info_t si = {KMP_MSPT,kmp1,kmp2,verbose,"MSPT","mission success"};
    return diff_pt2_si(&si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_POTI()			///////////////
///////////////////////////////////////////////////////////////////////////////

static int diff_poti_group ( const kmp_poti_group_t *e1, const kmp_poti_group_t *e2 )
{
    if (!memcmp(e1,e2,sizeof(*e1)))
	return 0;

    return ( e1->n_point != e2->n_point )
	 + ( e1->smooth  != e2->smooth )
	 + ( e1->back    != e2->back );
}

//-----------------------------------------------------------------------------

static int diff_poti_point ( const kmp_poti_point_t *e1, const kmp_poti_point_t *e2 )
{
    if (!memcmp(e1,e2,sizeof(*e1)))
	return 0;

    uint count	= ( e1->speed	!= e2->speed )
		+ ( e1->unknown	!= e2->unknown );

    count += fabsf( e1->position[0] - e2->position[0] ) > epsilon_pos
	  || fabsf( e1->position[1] - e2->position[1] ) > epsilon_pos
	  || fabsf( e1->position[2] - e2->position[2] ) > epsilon_pos;

    return count;
}

//-----------------------------------------------------------------------------

static uint diff_KMP_POTI
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *gl1 = kmp1->dlist + KMP_POTI;
    List_t *pl1 = &kmp1->poti_point;
    List_t *gl2 = kmp2->dlist + KMP_POTI;
    List_t *pl2 = &kmp2->poti_point;


    if ( gl1->used != gl2->used )
    {
	if (!verbose)
	    return 1;
	printf("POTI: Number of routes differ: Have %u and %u.\n",
		gl1->used, gl2->used );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    if ( pl1->used != pl2->used )
    {
	if (!verbose)
	    return 1;
	printf("POTI: Number of total route points differ: Have %u and %u.\n",
		pl1->used, pl2->used );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

// [[2do]] ??? qqq

    uint i;
    const uint gn = gl1->used < gl2->used ? gl1->used : gl2->used;
    kmp_poti_group_t *ge1 = (kmp_poti_group_t*)gl1->list;
    kmp_poti_group_t *ge2 = (kmp_poti_group_t*)gl2->list;

    for ( i = 0; i < gn; i++, ge1++, ge2++ )
    {
	if (diff_poti_group(ge1,ge2))
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    fputs("POTI: Groups differ\n",stdout);
		return 1;
	    }

	    printf("POTI: Group #%u differ\n",i);
	    diff_count++;
	}
    }


// [[2do]] ??? qqq

    const uint pn = pl1->used < pl2->used ? pl1->used : pl2->used;
    kmp_poti_point_t *pe1 = (kmp_poti_point_t*)pl1->list;
    kmp_poti_point_t *pe2 = (kmp_poti_point_t*)pl2->list;

    for ( i = 0; i < pn; i++, pe1++, pe2++ )
    {
	if (diff_poti_point(pe1,pe2))
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    fputs("POTI: Route points differ\n",stdout);
		return 1;
	    }

	    printf("POTI: Route point #%u differ\n",i);
	    diff_count++;
	}
    }

    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			diff_KMP_STGI()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct sort_stgi_t
{
    int			rank;	// previous rank order
    kmp_stgi_entry_t	data;	// original object data

} sort_stgi_t;

//-----------------------------------------------------------------------------

static sort_stgi_t * sort_stgi ( const List_t *list )
{
    DASSERT(list);
    if (!list->used)
	return 0;

    sort_stgi_t *res = CALLOC(list->used,sizeof(*res));
    sort_stgi_t *ptr = res;
    kmp_stgi_entry_t *e = (kmp_stgi_entry_t*)list->list;
    const uint n = list->used;

    uint i;
    for ( i = 0; i < n; i++, e++, ptr++ )
    {
	ptr->rank = i;
	memcpy(&ptr->data,e,sizeof(ptr->data));
    }

 #if 0
    if ( n > 1 && KMP_DIFF_MODE & KDMD_SORT )
	qsort( res, n, sizeof(*res), (qsort_func)compare_sort_stgi );
 #endif
    return res;
}

//-----------------------------------------------------------------------------

static inline void print_stgi
	( char mode, const kmp_stgi_entry_t *e1, const kmp_stgi_entry_t *e2 )
{
    if ( e2 && colout->colorize )
    {
	ccp ok   = colout->COL_OK;
	ccp warn = colout->COL_WARN;

	printf("%c %s%5u %s%4u %s%5u %s%5u   %s%#10x  %s0x%02x %s%#5x %s%#6x",
		mode,
		OK_WARN1(e1,e2,lap_count),
		OK_WARN1(e1,e2,pole_pos),
		OK_WARN1(e1,e2,narrow_start),
		OK_WARN1(e1,e2,enable_lens_flare),
		OK_WARN1(e1,e2,flare_color),
		OK_WARN1(e1,e2,flare_alpha),
		OK_WARN1(e1,e2,unknown_09),
		OK_WARN1(e1,e2,speed_mod) );

	if (e1->speed_mod)
	    printf(" (%5.3f)",SpeedMod2float(e1->speed_mod));
	printf("%s\n",colout->reset);
    }
    else
    {
	printf("%c %5u %4u %5u %5u   %#10x  0x%02x %#5x %#6x",
		mode,
		e1->lap_count, e1->pole_pos,
		e1->narrow_start, e1->enable_lens_flare,
		e1->flare_color,
		e1->flare_alpha, e1->unknown_09,
		e1->speed_mod );
	if (e1->speed_mod)
	    printf(" (%5.3f)\n",SpeedMod2float(e1->speed_mod));
	else
	    putchar('\n');
    }
}

//-----------------------------------------------------------------------------

static sort_stgi_t * print_stgi_single
	( char mode, const sort_stgi_t *e, sort_stgi_t *end )
{
    DASSERT(e);
    DASSERT(end);

    while ( e < end )
    {
	printf("%sSTGI: Only found in source %c @%u%s\n",
		mode == '<' ? colout->COL_SRC1 : colout->COL_SRC2,
		mode == '<' ? '1' : '2', e->rank,
		colout->reset );
	print_stgi(mode,&e->data,0);
	e++;
    }
    return (sort_stgi_t*)e;
}

//-----------------------------------------------------------------------------

static int diff_stgi ( const sort_stgi_t *e1, const sort_stgi_t *e2 )
{
    if (!memcmp(&e1->data,&e2->data,sizeof(e1->data)))
	return 0;

    return ( e1->data.lap_count		!= e2->data.lap_count )
	 + ( e1->data.pole_pos		!= e2->data.pole_pos )
	 + ( e1->data.narrow_start	!= e2->data.narrow_start )
	 + ( e1->data.enable_lens_flare	!= e2->data.enable_lens_flare )
	 + ( e1->data.flare_color	!= e2->data.flare_color )
	 + ( e1->data.flare_alpha	!= e2->data.flare_alpha )
	 + ( e1->data.unknown_09	!= e2->data.unknown_09 )
	 + ( e1->data.speed_mod		!= e2->data.speed_mod );
}

//-----------------------------------------------------------------------------

static uint diff_KMP_STGI
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);

    uint diff_count = 0;
    List_t *l1 = kmp1->dlist + KMP_STGI;
    List_t *l2 = kmp2->dlist + KMP_STGI;

    if ( l1->used != l2->used )
    {
	if (!verbose)
	    return 1;
	printf("%sSTGI: Number of stage info records differ: Have %u and %u.%s\n",
		colout->COL_WARN, l1->used, l2->used, colout->reset );
	if (verbose==1)
	    return 1;
	diff_count++;
    }

    sort_stgi_t *s1 = sort_stgi(l1), *e1 = s1, *end1 = e1 + l1->used;
    sort_stgi_t *s2 = sort_stgi(l2), *e2 = s2, *end2 = e2 + l2->used;
    uint order_differ = 0;

    while ( e1 < end1 && e2 < end2 )
    {
	const int stat = diff_stgi(e1,e2);
	if (stat)
	{
	    if ( verbose < 2 )
	    {
		if (verbose)
		    printf("%sSTGI: Data differ%s\n",
			colout->COL_DIFF, colout->reset );
		return 1;
	    }

	    const int trigger = 2;
	    if ( stat >= trigger )
	    {
		//--- try to sync

		int dist1 = 99999;
		sort_stgi_t *p1;
		for ( p1 = e1+1; p1 < end1; p1++ )
		    if ( diff_stgi(p1,e2) < trigger )
		    {
			dist1 = p1 - e1;
			break;
		    }

		int dist2 = 99999;
		sort_stgi_t *p2;
		for ( p2 = e2+1; p2 < end2; p2++ )
		    if ( diff_stgi(p2,e1) < trigger )
		    {
			dist2 = p2 - e2;
			break;
		    }

		PRINT0("DIST: %d %d\n",dist1,dist2);

		if ( dist1 < 99999 && dist1 < dist2 )
		{
		    e1 = print_stgi_single('<',e1,e1+1);
		    diff_count++;
		    continue;
		}
		else if ( dist2 < 99999 && dist2 < dist1 )
		{
		    e2 = print_stgi_single('>',e2,e2+1);
		    diff_count++;
		    continue;
		}
	    }

	    //--- print data

	    printf("%sSTGI: Stage info records @%u:%u differ%s\n",
			colout->COL_DIFF, e1->rank,e2->rank,
			colout->reset );
	    print_stgi('<',&e1->data,&e2->data);
	    print_stgi('>',&e2->data,&e1->data);
	    diff_count++;
	}

	if ( e1->rank != e2->rank )
	    order_differ++;

	e1++;
	e2++;
    }

    if ( e1 < end1 )
    {
	print_stgi_single('<',e1,end1);
	diff_count++;
    }

    if ( e2 < end2 )
    {
	print_stgi_single('>',e2,end2);
	diff_count++;
    }

    if (order_differ)
    {
	if (verbose)
	    printf("%sSTGI: Order of stage info records differ%s\n",
			colout->COL_DIFF, colout->reset );
	if ( verbose < 2 )
	    return 1;
	diff_count++;
    }

    FREE(s1);
    FREE(s2);
    return diff_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			DiffKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DiffKMP
(
    kmp_t	* kmp1,		// first kmp to compare
    kmp_t	* kmp2,		// second kmp tp compare
    uint	verbose		// 0:quiet, 1:summary only, 2:full report
)
{
    DASSERT(kmp1);
    DASSERT(kmp2);
    PRINT("DiffKMP( %s, %s, verbose=%d )\n",kmp1->fname,kmp2->fname,verbose);

    //--- compare sections

    enumError diff_count = 0;

    if ( KMP_DIFF_MODE & KDMD_AREA && ( verbose || !diff_count ))
		diff_count += diff_KMP_AREA(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_CAME && ( verbose || !diff_count ))
		diff_count += diff_KMP_CAME(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_CKPH && ( verbose || !diff_count ))
		diff_count += diff_KMP_CKPH(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_CKPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_CKPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_CNPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_CNPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_ENPH && ( verbose || !diff_count ))
		diff_count += diff_KMP_ENPH(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_ENPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_ENPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_GOBJ && ( verbose || !diff_count ))
		diff_count += diff_KMP_GOBJ(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_ITPH && ( verbose || !diff_count ))
		diff_count += diff_KMP_ITPH(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_ITPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_ITPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_JGPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_JGPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_KTPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_KTPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_MSPT && ( verbose || !diff_count ))
		diff_count += diff_KMP_MSPT(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_POTI && ( verbose || !diff_count ))
		diff_count += diff_KMP_POTI(kmp1,kmp2,verbose);

    if ( KMP_DIFF_MODE & KDMD_STGI && ( verbose || !diff_count ))
		diff_count += diff_KMP_STGI(kmp1,kmp2,verbose);

    return diff_count ? ERR_DIFFER : ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

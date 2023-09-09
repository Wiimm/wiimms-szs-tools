
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

#include "lib-std.h"
#include "lib-szs.h"
#include "lib-brres.h"
#include "lib-breff.h"
#include "lib-xbmg.h"
#include "lib-image.h"
#include "dclib-utf8.h"
#include "ui.h"
#include "db-mkw.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan size options		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptAlignU8 ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_align_u8,		// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align-u8",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptAlignLTA ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_align_lta,		// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align-lta",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptAlignPACK ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_align_pack,	// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align-pack",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptAlignBRRES ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_align_brres,	// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align-brres",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptAlignBREFF ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_align_breff,	// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align-breff",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptAlignBREFT ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_align_breft,	// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align-breft",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptAlign ( ccp arg )
{
    const int stat = ScanSizeOptU32(
		&opt_align,		// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"align",		// ccp opt_name
		0x00001,		// u64 min
		0x10000,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;

    if (!stat)
	opt_align_u8 = opt_align_pack =
		opt_align_brres = opt_align_breff = opt_align_breft = opt_align;
    return stat;
}

//-----------------------------------------------------------------------------

int ScanOptMaxFileSize ( ccp arg )
{
    return ScanSizeOptU32(
		&opt_max_file_size,	// u32 * num
		arg,			// ccp source
		MiB,			// default_factor1
		0,			// int force_base
		"max-file-size",	// ccp opt_name
		MiB,			// u64 min
		2ull*GiB,		// u64 max
		0,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

//-----------------------------------------------------------------------------

int ScanOptEpsilon ( ccp arg )
{
    // [[2do]] ???

    if ( !arg || !*arg )
    {
	epsilon_pos	= DEF_EPSILON_POS;
	epsilon_rot	= DEF_EPSILON_ROT;
	epsilon_scale	= DEF_EPSILON_SCALE;
	epsilon_time	= DEF_EPSILON_TIME;
    }
    else
    {
	double epsilon = strtod(arg,0);
	if ( epsilon < 0.0 )
	    epsilon = 0.0;

	epsilon_pos	=
	epsilon_rot	=
	epsilon_scale	=
	epsilon_time	= epsilon;
    }

    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptNMipmaps ( ccp arg )
{
    static const KeywordTab_t tab[] =
    {
	{  0,	"OFF",	    "AUTO",	0 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	opt_n_images = cmd->id;
	return 0;
    }

    uint num = 0;
    const int stat = ScanSizeOptU32(
		&num,			// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"n-mipmaps",		// ccp opt_name
		0,			// u64 min
		MAX_MIPMAPS,		// u64 max
		0,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;

    if (!stat)
	opt_n_images = num + 1;
    return stat;
}

//-----------------------------------------------------------------------------

int ScanOptMaxMipmaps ( ccp arg )
{
    uint num = 0;
    const int stat = ScanSizeOptU32(
		&num,			// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"max-mipmaps",		// ccp opt_name
		0,			// u64 min
		MAX_MIPMAPS,		// u64 max
		0,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;

    if (!stat)
	opt_max_images = num + 1;
    return stat;
}

//-----------------------------------------------------------------------------

int ScanOptMipmapSize ( ccp arg )
{
    uint num = 0;
    const int stat = ScanSizeOptU32(
		&num,			// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"mipmap-size",		// ccp opt_name
		1,			// u64 min
		1024,			// u64 max
		0,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;

    if (!stat)
	opt_min_mipmap_size = num > 1 ? num : 1;
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  scan num/range		///////////////
///////////////////////////////////////////////////////////////////////////////

double strtod_comma ( const char *nptr, char **endptr )
{
    DASSERT(nptr);

    char *end;
    double d = strtod(nptr,&end);
    if ( end && *end == ',' )
    {
	*(char*)end = '.';
	char *end2;
	d = strtod(nptr,&end2);
	*(char*)end = ',';
	end = end2;
    }
    if (endptr)
	*endptr = end;
    return d;
}

///////////////////////////////////////////////////////////////////////////////

char * ScanNumU32
(
    ccp			arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..1)
    u32			* p_num,	// return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
)
{
    ASSERT(arg);
    ASSERT(p_num);
    TRACE("ScanNumU32(%s)\n",arg);

    while ( *arg > 0 && *arg <= ' ' )
	arg++;

    char * end;
    u32 num = strtoul(arg,&end, arg[1] >= '0' && arg[1] <= '9' ? 10 : 0 );
    u32 stat = end > arg;
    if (stat)
    {
	if ( num < min )
	    num = min;
	else if ( num > max )
	    num = max;

	while ( *end > 0 && *end <= ' ' )
	    end++;
    }
    else
	num = 0;

    if (p_stat)
	*p_stat = stat;
    *p_num = num;

    TRACE("END ScanNumU32() stat=%u, n=%u ->%s\n",stat,num,arg);
    return end;
}

///////////////////////////////////////////////////////////////////////////////

char * ScanRangeU32
(
    ccp			arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..2)
    u32			* p_n1,		// first return value (valid pointer)
    u32			* p_n2,		// second return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
)
{
    ASSERT(arg);
    ASSERT(p_n1);
    ASSERT(p_n2);
    TRACE("ScanRangeU32(%s)\n",arg);

    int stat = 0;
    u32 n1 = ~(u32)0, n2 = 0;

    while ( *arg > 0 && *arg <= ' ' )
	arg++;

    if ( *arg == '-' )
	n1 = min;
    else
    {
	char * end;
	u32 num = strtoul(arg,&end,0);
	if ( arg == end )
	    goto abort;

	stat = 1;
	arg = end;
	n1 = num;

	while ( *arg > 0 && *arg <= ' ' )
	    arg++;
    }

    if ( *arg != '-' && *arg != ':' )
    {
	stat = 1;
	n2 = n1;
	goto abort;
    }
    arg++;

    while ( *arg > 0 && *arg <= ' ' )
	arg++;

    char * end;
    n2 = strtoul(arg,&end,0);
    if ( end == arg )
	n2 = max;
    stat = 2;
    arg = end;

 abort:

    if ( stat > 0 )
    {
	if ( n1 < min )
	    n1 = min;
	if ( n2 > max )
	    n2 = max;
    }

    if ( !stat || n1 > n2 )
    {
	stat = 0;
	n1 = ~(u32)0;
	n2 = 0;
    }

    if (p_stat)
	*p_stat = stat;
    *p_n1 = n1;
    *p_n2 = n2;

    while ( *arg > 0 && *arg <= ' ' )
	arg++;

    TRACE("END ScanRangeU32() stat=%u, n=%u..%u ->%s\n",stat,n1,n2,arg);
    return (char*)arg;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned long int strtoul16
(
    const u16		*  ptr16,
    u16			** end16,
    int			base
)
{
    DASSERT(ptr16);

    char buf[100];
    char * dest = buf;
    const u16 * p16 = ptr16;
    while ( dest < buf + sizeof(buf)-1 )
    {
	int ch = be16(p16++);
	if ( ch >= '0' && ch <= '9'
	    || ch >= 'a' && ch <= 'f'
	    || ch >= 'A' && ch <= 'F'
	    || ch == 'x'
	    || ch == 'X'
	    || isspace(ch)
	    )
	{
	    *dest++ = ch;
	}
	else
	    break;
    }
    DASSERT( dest < buf + sizeof(buf) );
    *dest = 0;

    char *end;
    unsigned long num = strtoul(buf,&end,base);
    if (end16)
	*end16 = (u16*)ptr16 + (end-buf);
    return num;
}

///////////////////////////////////////////////////////////////////////////////

u16 * skip_ctrl16 ( const u16 * ptr )
{
    DASSERT(ptr);
    for(;;)
    {
	const u16 ch = be16(ptr);
	if ( !ch || ch > ' ' )
	    return (u16*)ptr;
	ptr++;
    }
}

///////////////////////////////////////////////////////////////////////////////

u16 * ScanNum16U32
(
    const u16		* arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..1)
    u32			* p_num,	// return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
)
{
    ASSERT(arg);
    ASSERT(p_num);
    TRACE("ScanNum16U32()\n");

    arg = skip_ctrl16(arg);

    u16 * end;
    u32 num = strtoul16(arg,&end, arg[1] >= '0' && arg[1] <= '9' ? 10 : 0 );
    u32 stat = end > arg;
    if (stat)
    {
	if ( num < min )
	    num = min;
	else if ( num > max )
	    num = max;

	end = skip_ctrl16(end);
    }
    else
	num = 0;

    if (p_stat)
	*p_stat = stat;
    *p_num = num;

    TRACE("END ScanNum16U32() stat=%u, n=%u\n",stat,num);
    return end;
}

///////////////////////////////////////////////////////////////////////////////

u16 * ScanRange16U32
(
    const u16		* arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..2)
    u32			* p_n1,		// first return value (valid pointer)
    u32			* p_n2,		// second return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
)
{
    ASSERT(arg);
    ASSERT(p_n1);
    ASSERT(p_n2);
    TRACE("ScanRangeU32()\n");

    int stat = 0;

    u32 n1 = ~(u32)0, n2 = 0;
    arg = skip_ctrl16(arg);

    if ( be16(arg) == '-' )
	n1 = min;
    else
    {
	u16 * end;
	u32 num = strtoul16(arg,&end,0);
	if ( arg == end )
	    goto abort;

	stat = 1;
	arg = skip_ctrl16(end);
	n1 = num;
    }

    if ( be16(arg) != '-' )
    {
	stat = 1;
	n2 = n1;
	goto abort;
    }

    arg = skip_ctrl16(arg+1);
    u16 * end;
    n2 = strtoul16(arg,&end,0);
    if ( end == arg )
	n2 = max;
    stat = 2;
    arg = end;

 abort:

    if ( stat > 0 )
    {
	if ( n1 < min )
	    n1 = min;
	if ( n2 > max )
	    n2 = max;
    }

    if ( !stat || n1 > n2 )
    {
	stat = 0;
	n1 = ~(u32)0;
	n2 = 0;
    }

    if (p_stat)
	*p_stat = stat;
    *p_n1 = n1;
    *p_n2 = n2;

    arg = skip_ctrl16(arg);

    TRACE("END ScanRange16U32() stat=%u, n=%u..%u\n",stat,n1,n2);
    return (u16*)arg;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			vector calculations		///////////////
///////////////////////////////////////////////////////////////////////////////

double LengthF
(
    const float *a,		// A vector (x,y,z)
    const float *b		// B vector (x,y,z)
)
{
    DASSERT(a);
    DASSERT(b);

    const double d0 = a[0] - b[0];
    const double d1 = a[1] - b[1];
    const double d2 = a[2] - b[2];

    return sqrt ( d0*d0 + d1*d1 + d2*d2 );
}

///////////////////////////////////////////////////////////////////////////////

double Length2F
(
    const float * a,		// A vector (x,y,z)
    const float * b		// B vector (x,y,z)
)
{
    DASSERT(a);
    DASSERT(b);

    const double d0 = a[0] - b[0];
    const double d1 = a[1] - b[1];
    const double d2 = a[2] - b[2];

    return d0*d0 + d1*d1 + d2*d2;
}

///////////////////////////////////////////////////////////////////////////////

double LengthD
(
    const double * a,		// A vector (x,y,z)
    const double * b		// B vector (x,y,z)
)
{
    DASSERT(a);
    DASSERT(b);

    const double d0 = a[0] - b[0];
    const double d1 = a[1] - b[1];
    const double d2 = a[2] - b[2];

    return sqrt ( d0*d0 + d1*d1 + d2*d2 );
}

///////////////////////////////////////////////////////////////////////////////

double Length2D
(
    const double * a,		// A vector (x,y,z)
    const double * b		// B vector (x,y,z)
)
{
    DASSERT(a);
    DASSERT(b);

    const double d0 = a[0] - b[0];
    const double d1 = a[1] - b[1];
    const double d2 = a[2] - b[2];

    return d0*d0 + d1*d1 + d2*d2;
}

///////////////////////////////////////////////////////////////////////////////

double AngleVector
(
    const float	* prev,		// vector (x,y,z) of previous point
    const float	* point,	// vector (x,y,z) of current point
    const float	* next,		// vector (x,y,z) of next point
    bool	return_degree	// true: return in normed degree; false: radiants
)
{
    DASSERT(prev);
    DASSERT(point);
    DASSERT(next);

    noTRACE("ANGLE: %11.3f %11.3f | %11.3f %11.3f | %11.3f %11.3f\n",
	    prev[0], prev[2], point[0], point[2], next[0], next[2] );

    const double ax = point[0] - prev[0];
    const double az = point[2] - prev[2];
    if ( fabs(ax) + fabs(az) < 1e-6 )
	return 0.0;

    const double bx = next[0] - point[0];
    const double bz = next[2] - point[2];
    if ( fabs(bx) + fabs(bz) < 1e-6 )
	return 0.0;

    if (!return_degree)
	return atan2(bx,bz) - atan2(ax,az);

    const double deg = ( atan2(bx,bz) - atan2(ax,az) ) * ( 180 / M_PI );
    return deg > 180.0 ? deg - 360.0 : deg <= -180.0 ? deg + 360.0 : deg;
}

///////////////////////////////////////////////////////////////////////////////

void WidthAndDirection
(
    double	* r_width,	// if not NULL: return width here
    double	* r_direction,	// if not NULL: return direction here
    bool	return_degree,	// true: return in normed degree; false: radiants
    const float	* a,		// A point (x,y)
    const float	* b		// B point (x,y)
)
{
    const double dx = b[0] - a[0];
    const double dy = b[1] - a[1];
    const double width = sqrt ( dx*dx + dy*dy );
    if (r_width)
	*r_width = width;

    if (r_direction)
    {
	if ( width < 1e-6 )
	    *r_direction = 0.0;
	else if (return_degree)
	{
	    double deg = atan2(dx,dy) * ( 180 / M_PI ) + 90;
	    *r_direction = deg > 180.0 ? deg - 360.0 : deg;
	}
	else
	    *r_direction = atan2(dx,dy);
    }
}

///////////////////////////////////////////////////////////////////////////////

double CalcDirection2F
(
    const float	* a,		// A point (x,y)
    const float	* b		// B point (x,y)
)
{
    const double dx = b[0] - a[0];
    const double dy = b[1] - a[1];
    return fabs(dx) + fabs(dy) < 1e-6
	? 0.0
	: atan2(dx,dy);
}

///////////////////////////////////////////////////////////////////////////////

float3 CalcDirection3F
(
    const float * a,		// A point (x,y,z)
    const float * b		// B point (x,y,z)
)
{
    DASSERT(a);
    DASSERT(b);

    const double dx = b[0] - a[0];
    const double dy = b[1] - a[1];
    const double dz = b[2] - a[2];
    const double hl = sqrt ( dx*dx + dz*dz );

    float3 res;
    res.x =  fabs(dy) + fabs(hl) < 1e-6 ? 0.0 : atan2(dy,hl) * ( -180.0 / M_PI );
    res.y =  fabs(dx) + fabs(dz) < 1e-6 ? 0.0 : atan2(dx,dz) * (  180.0 / M_PI );
    res.z = 0.0;
    return res;
}

///////////////////////////////////////////////////////////////////////////////

double3 CalcDirection3D
(
    const double3 * a,		// A point (x,y,z)
    const double3 * b		// B point (x,y,z)
)
{
    DASSERT(a);
    DASSERT(b);

    const double dx = b->x - a->x;
    const double dy = b->y - a->y;
    const double dz = b->z - a->z;
    const double hl = sqrt ( dx*dx + dz*dz );

    double3 res;
    res.x =  fabs(dy) + fabs(hl) < 1e-6 ? 0.0 : atan2(dy,hl) * ( -180.0 / M_PI );
    res.y =  fabs(dx) + fabs(dz) < 1e-6 ? 0.0 : atan2(dx,dz) * (  180.0 / M_PI );
    res.z = 0.0;
    return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Scale
(
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*scale,		// NULL or scaling vector
    double3		*pt,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
)
{
    DASSERT( pt || !n_pt );

    if (scale)
    {
	if (origin)
	{
	    for ( ; n_pt-- > 0; pt = (double3*)((u8*)pt + pt_delta) )
	    {
		pt->x = ( pt->x - origin->x ) * scale->x + origin->x;
		pt->y = ( pt->y - origin->y ) * scale->y + origin->y;
		pt->z = ( pt->z - origin->z ) * scale->z + origin->z;
	    }
	}
	else
	{
	    for ( ; n_pt-- > 0; pt = (double3*)((u8*)pt + pt_delta) )
	    {
		pt->x *= scale->x;
		pt->y *= scale->y;
		pt->z *= scale->z;
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void Rotate
(
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*rot,		// rotation vector
    double3		*pt1,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
)
{
    DASSERT(pt1);

    if (!rot)
	return;

    bool have_rot[3];
    have_rot[0] = fabs(rot->x) > 1e-6;
    have_rot[1] = fabs(rot->y) > 1e-6;
    have_rot[2] = fabs(rot->z) > 1e-6;
    if ( !have_rot[0] && !have_rot[1] && !have_rot[2] )
	return;

    uint p;
    for ( p = 0; p < 3; p++ )
    {
	if (!have_rot[p])
	    continue;

	const double rad_rot = rot->v[p] * (M_PI/180.0);
	const uint a = (p+2) % 3;
	const uint b = (p+1) % 3;

	if (origin)
	{
	    uint i;
	    double3 *pt = pt1;
	    for ( i = n_pt; i-- > 0; pt = (double3*)((u8*)pt+pt_delta) )
	    {
		const double da  = pt->v[a] - origin->v[a];
		const double db  = pt->v[b] - origin->v[b];
		const double rad = atan2(da,db) + rad_rot;
		const double len = sqrt(da*da+db*db);

		pt->v[a] = sin(rad) * len + origin->v[a];
		pt->v[b] = cos(rad) * len + origin->v[b];
	    }
	}
	else
	{
	    uint i;
	    double3 *pt = pt1;
	    for ( i = n_pt; i-- > 0; pt = (double3*)((u8*)pt+pt_delta) )
	    {
		const double da  = pt->v[a];
		const double db  = pt->v[b];
		const double rad = atan2(da,db) + rad_rot;
		const double len = sqrt(da*da+db*db);

		pt->v[a] = sin(rad) * len;
		pt->v[b] = cos(rad) * len;
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void Translate
(
    const double3	*trans,		// NULL or translation vector
    double3		*pt,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
)
{
    DASSERT( pt || !n_pt );

    if (trans)
	for ( ; n_pt-- > 0; pt = (double3*)((u8*)pt + pt_delta) )
	{
	    pt->x += trans->x;
	    pt->y += trans->y;
	    pt->z += trans->z;
	}
}

///////////////////////////////////////////////////////////////////////////////

void AxisRotate
(
    const double3	*a1,		// first axis point
    const double3	*a2,		// second axis point
    double		degree,		// degree of rotation
    double3		*pt1,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
)
{
    DASSERT(a1);
    DASSERT(a2);
    DASSERT( pt1 || !n_pt );


    //--- some precalculations

    double3 n12;
    Sub3(n12,*a2,*a1);
    Unit3(n12);

    const double rad = degree * (M_PI/180.0);
    const double fsin = sin(rad);
    const double fcos = cos(rad);

    uint i;
    double3 *pt = pt1;
    for ( i = n_pt; i-- > 0; pt = (double3*)((u8*)pt+pt_delta) )
    {
	double3 d13, n13, nx, base;
	Sub3(d13,*pt,*a1);
	CrossProd3(nx,n12,d13);
	CrossProd3(n13,nx,n12);
	Sub3(base,*pt,n13);

	pt->x = fcos * n13.x + fsin * nx.x + base.x;
	pt->y = fcos * n13.y + fsin * nx.y + base.y;
	pt->z = fcos * n13.z + fsin * nx.z + base.z;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

double Degree // output: -180.00 .. +180.00
(
    double	rad		// radians
)
{
    double deg = fmod( rad * (180.0/M_PI) + 180.0, 360.0 );
    return deg < 0.0 ? deg + 180.0 : deg - 180.0;
}

///////////////////////////////////////////////////////////////////////////////

float NormDegreeF // output: -180.00 .. +180.00
(
    float	deg
)
{
    deg = fmod( deg + 180.0, 360.0 );
    return deg < 0.0 ? deg + 180.0 : deg - 180.0;
}

///////////////////////////////////////////////////////////////////////////////

void MinMax3
(
    double3		*min,		// valid pointer to initialized min value
    double3		*max,		// valid pointer to initialized max value
    const double3	*val,		// valid pointer to source list
    int			n_val		// if >0: number of elements in 'val'
)
{
    DASSERT(min);
    DASSERT(max);
    DASSERT(val);

    while ( n_val-- > 0 )
    {
	if ( min->x > val->x ) min->x = val->x;
	if ( min->y > val->y ) min->y = val->y;
	if ( min->z > val->z ) min->z = val->z;

	if ( max->x < val->x ) max->x = val->x;
	if ( max->y < val->y ) max->y = val->y;
	if ( max->z < val->z ) max->z = val->z;

	val++;
    }
}

///////////////////////////////////////////////////////////////////////////////

void UnitZero3 ( double3 *dest, const double3 *src )
{
    DASSERT(dest);
    DASSERT(src);

    double len = Length3(*src);
    double min = len * 1e-5;
    int dirty = 0;

    if ( fabs(src->x) < min )
    {
	dest->x = 0.0;
	dirty = true;
    }
    else
	dest->x = src->x;

    if ( fabs(src->y) < min )
    {
	dest->y = 0.0;
	dirty = true;
    }
    else
	dest->y = src->y;

    if ( fabs(src->z) < min )
    {
	dest->z = 0.0;
	dirty = true;
    }
    else
	dest->z = src->z;

    if (dirty)
	len = Length3(*dest);

    if (len)
    {
	dest->x /= len;
	dest->y /= len;
	dest->z /= len;
    }
}

///////////////////////////////////////////////////////////////////////////////

double CalcNormals
(
    // return length of p1..p2

    double3		*res_norm,	// return 3 normals here
    const double3	*p1,		// first point
    const double3	*p2,		// second point
    const double3	*dir,		// NULL or helper point for normal[1]
    double		r		// radius == length of norm[0]..norm[2]
)
{
    DASSERT(res_norm);
    DASSERT(p1);
    DASSERT(p2);

    bool vertical = false;

    Sub3(res_norm[0],*p2,*p1);
    double len0 = Length3(res_norm[0]);
    if ( len0 < 1e-9 )
    {
	len0 = 0.0;
	res_norm[0].x = 0.0;
	res_norm[0].y = 0.0;
	res_norm[0].z = 1.0;
    }
    else
    {
	res_norm[0].x /= len0;
	res_norm[0].y /= len0;
	res_norm[0].z /= len0;
	vertical = fabs(res_norm[0].y) > 0.9;
    }

    double3 temp;
    if (!dir)
    {
	dir = &temp;
	memcpy(&temp,p1,sizeof(temp));
	if (vertical)
	    temp.z += 1000;
	else
	    temp.y += 1000;
    }

    double3 dist;
    Sub3(dist,*dir,*p1);
    CrossProd3(res_norm[2],res_norm[0],dist);
    double len1 = Length3(res_norm[2]);
    if ( len1 < 1e-9 )
    {
	if (vertical)
	{
	    temp.x = temp.y = 0.0;
	    temp.z = 1000.0;
	}
	else
	{
	    temp.x = temp.z = 0.0;
	    temp.y = 1000.0;
	}
	CrossProd3(res_norm[2],res_norm[0],temp);
	len1 = Length3(res_norm[2]);
    }
    len1 = r / len1;
    res_norm[2].x *= len1;
    res_norm[2].y *= len1;
    res_norm[2].z *= len1;

    CrossProd3(res_norm[1],res_norm[2],res_norm[0]);
    res_norm[0].x *= r;
    res_norm[0].y *= r;
    res_norm[0].z *= r;

    return len0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    OverlapCubeTri()			///////////////
///////////////////////////////////////////////////////////////////////////////

bool OverlapCubeTri
(
    double3		*cube_mid,	// middle of the cube
    double		cube_width,	// width of the cube
    double3		*pt1,		// first point of triangle
    double3		*pt2,		// second point of triangle
    double3		*pt3		// third point of triangle
)
{
    ERROR0(ERR_NOT_IMPLEMENTED,0);
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  triangles			///////////////
///////////////////////////////////////////////////////////////////////////////

void CalcTriMetricsByPoints
(
    tri_metrics_t	*met,	// result
    const double3	*p1,	// NULL or first point
    const double3	*p2,	// NULL or second point
    const double3	*p3	// NULL or third point
)
{
    DASSERT(met);
    memset(met,0,sizeof(*met));
    if (p1) Assign3(met->p1,*p1);
    if (p2) Assign3(met->p2,*p2);
    if (p3) Assign3(met->p3,*p3);
    CalcTriMetrics(met);
}

///////////////////////////////////////////////////////////////////////////////

void CalcTriMetrics ( tri_metrics_t *met )
{
    DASSERT(met);

    double3 x, y, z;
    Sub3(x,met->p2,met->p1);
    Sub3(y,met->p3,met->p2);
    Sub3(z,met->p1,met->p3);

    // calc side lengths
    const double a2 = LengthSqare3(x); met->l1 = sqrt(a2);
    const double b2 = LengthSqare3(y); met->l2 = sqrt(b2);
    const double c2 = LengthSqare3(z); met->l3 = sqrt(c2);

    // calc area ( 0.0625 = 1/16 )
    met->area = sqrt(( 2*(a2*b2+a2*c2+b2*c2) - a2*a2 - b2*b2 - c2*c2 ) * 0.0625 );

    // calc heights
    const double area2 = 2*met->area;
    met->h1 = area2/met->l1;
    met->h2 = area2/met->l2;
    met->h3 = area2/met->l3;
    MinMax3p(met->min_ht,met->max_ht,met->h1,met->h2,met->h3);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  octahedron_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void CreateRegularOctahedron
(
    octahedron_t	*oct,		// valid pointer, initialize
    double		radius,		// radius of octahedron
    const double3	*rot,		// not NULL: rotate octahedron
    const double3	*shift		// not NULL: shift octahedron
)
{
    DASSERT(oct);

    double3 radius3;
    radius3.x = radius3.y = radius3.z = radius;
    CreateOctahedron(oct,&radius3,rot,shift);
}

//-----------------------------------------------------------------------------

void CreateRegularOctahedronF
(
    octahedron_t	*oct,		// valid pointer, initialize
    double		radius,		// radius of octahedron
    const float		*rot,		// not NULL: 3D vector, rotate octahedron
    const float		*shift		// not NULL: 3D vector, shift octahedron
)
{
    DASSERT(oct);

    double3 radius3;
    radius3.x = radius3.y = radius3.z = radius;

    double3 *rot3 = 0, rot3_data;
    if (rot)
    {
	rot3_data.x = *rot++;
	rot3_data.y = *rot++;
	rot3_data.z = *rot;
	rot3 = &rot3_data;
    }

    double3 *shift3 = 0, shift3_data;
    if (shift)
    {
	shift3_data.x = *shift++;
	shift3_data.y = *shift++;
	shift3_data.z = *shift;
	shift3 = &shift3_data;
    }

    CreateOctahedron(oct,&radius3,rot3,shift3);
}

//-----------------------------------------------------------------------------

void CreateOctahedron
(
    octahedron_t	*oct,		// valid pointer, initialize
    const double3	*radius,	// radius of octahedron
    const double3	*rot,		// not NULL: rotate octahedron
    const double3	*shift		// not NULL: shift octahedron
)
{
    DASSERT(oct);
    DASSERT(radius);

    oct->x1.x = -radius->x;
    oct->x1.y = 0.0;
    oct->x1.z = 0.0;

    oct->x2.x = +radius->x;
    oct->x2.y = 0.0;
    oct->x2.z = 0.0;

    oct->y1.x = 0.0;
    oct->y1.y = -radius->y;
    oct->y1.z = 0.0;

    oct->y2.x = 0.0;
    oct->y2.y = +radius->y;
    oct->y2.z = 0.0;

    oct->z1.x = 0.0;
    oct->z1.y = 0.0;
    oct->z1.z = -radius->z;

    oct->z2.x = 0.0;
    oct->z2.y = 0.0;
    oct->z2.z = +radius->z;

    if (rot)
	RotateOctahedron(oct,0,rot);
    if (shift)
	ShiftOctahedron(oct,shift);
}

//-----------------------------------------------------------------------------

void ScaleOctahedron
(
    octahedron_t	*oct,		// valid octahedron data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*scale		// not NULL: scale octahedron
)
{
    DASSERT(oct);
    if (scale)
    {
	if (origin)
	{
	    uint p;
	    for ( p = 0; p < 6; p++ )
	    {
		oct->p[p].x = ( oct->p[p].x - origin->x ) * scale->x + origin->x;
		oct->p[p].y = ( oct->p[p].y - origin->y ) * scale->y + origin->y;
		oct->p[p].z = ( oct->p[p].z - origin->z ) * scale->z + origin->z;
	    }
	}
	else
	{
	    uint p;
	    for ( p = 0; p < 6; p++ )
	    {
		oct->p[p].x *= scale->x;
		oct->p[p].y *= scale->y;
		oct->p[p].z *= scale->z;
	    }
	}
    }
}

//-----------------------------------------------------------------------------

void RotateOctahedron
(
    octahedron_t	*oct,		// valid octahedron data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*rot		// not NULL: rotate octahedron
)
{
    if (rot)
	Rotate(origin,rot,oct->p,sizeof(*oct->p),6);
}

//-----------------------------------------------------------------------------

void ShiftOctahedron
(
    octahedron_t	*oct,		// valid octahedron data
    const double3	*shift		// not NULL: shift octahedron
)
{
    DASSERT(oct);
    if (shift)
    {
	uint p;
	for ( p = 0; p < 6; p++ )
	{
	    oct->p[p].x += shift->x;
	    oct->p[p].y += shift->y;
	    oct->p[p].z += shift->z;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  cuboid_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void CreateCube
(
    cuboid_t		*cub,		// valid pointer, initialize
    double		width,		// width of cube
    const double3	*rot,		// not NULL: rotate cube
    const double3	*shift		// not NULL: shift cube
)
{
    DASSERT(cub);

    double3 size;
    size.x = size.y = size.z = width;
    CreateCuboid(cub,&size,rot,shift);
}

//-----------------------------------------------------------------------------

void CreateCubeF
(
    cuboid_t		*cub,		// valid pointer, initialize
    double		width,		// width of cube
    const float		*rot,		// not NULL: 3D vector, rotate cube
    const float		*shift		// not NULL: 3D vector, shift cube
)
{
    DASSERT(cub);

    double3 size;
    size.x = size.y = size.z = width;

    double3 *rot3 = 0, rot3_data;
    if (rot)
    {
	rot3_data.x = *rot++;
	rot3_data.y = *rot++;
	rot3_data.z = *rot;
	rot3 = &rot3_data;
    }

    double3 *shift3 = 0, shift3_data;
    if (shift)
    {
	shift3_data.x = *shift++;
	shift3_data.y = *shift++;
	shift3_data.z = *shift;
	shift3 = &shift3_data;
    }

    CreateCuboid(cub,&size,rot3,shift3);
}

//-----------------------------------------------------------------------------

void CreateCuboid
(
    cuboid_t		*cub,		// valid pointer, initialize
    const double3	*size,		// base size of cuboid
    const double3	*rot,		// not NULL: rotate cuboid
    const double3	*shift		// not NULL: shift cuboid
)
{
    DASSERT(cub);
    DASSERT(size);

    double3 s;
    s.x = size->x / 2;
    s.y = size->y / 2;
    s.z = size->z / 2;

    uint p;
    for ( p = 0; p < 8; p++ )
    {
	cub->p[p].x = p & 1 ? +s.x : -s.x;
	cub->p[p].y = p & 2 ? +s.y : -s.y;
	cub->p[p].z = p & 4 ? +s.z : -s.z;
    }

    if (rot)
	RotateCuboid(cub,0,rot);
    if (shift)
	ShiftCuboid(cub,shift);
}

//-----------------------------------------------------------------------------

void CreateCuboidF
(
    cuboid_t		*cub,		// valid pointer, initialize
    const float		*size,		// base size of cuboid
    const float		*rot,		// not NULL: rotate cuboid
    const float		*shift		// not NULL: shift cuboid
)
{
    DASSERT(cub);
    DASSERT(size);

    double3 size3;
    size3.x = *size++;
    size3.y = *size++;
    size3.z = *size;

    double3 *rot3 = 0, rot3_data;
    if (rot)
    {
	rot3_data.x = *rot++;
	rot3_data.y = *rot++;
	rot3_data.z = *rot;
	rot3 = &rot3_data;
    }

    double3 *shift3 = 0, shift3_data;
    if (shift)
    {
	shift3_data.x = *shift++;
	shift3_data.y = *shift++;
	shift3_data.z = *shift;
	shift3 = &shift3_data;
    }

    CreateCuboid(cub,&size3,rot3,shift3);
}

//-----------------------------------------------------------------------------

void ScaleCuboid
(
    cuboid_t		*cub,		// valid cuboid data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*scale		// not NULL: scale cuboid
)
{
    DASSERT(cub);
    if (scale)
    {
	if (origin)
	{
	    uint p;
	    for ( p = 0; p < 8; p++ )
	    {
		cub->p[p].x = ( cub->p[p].x - origin->x ) * scale->x + origin->x;
		cub->p[p].y = ( cub->p[p].y - origin->y ) * scale->y + origin->y;
		cub->p[p].z = ( cub->p[p].z - origin->z ) * scale->z + origin->z;
	    }
	}
	else
	{
	    uint p;
	    for ( p = 0; p < 8; p++ )
	    {
		cub->p[p].x *= scale->x;
		cub->p[p].y *= scale->y;
		cub->p[p].z *= scale->z;
	    }
	}
    }
}

//-----------------------------------------------------------------------------

void RotateCuboid
(
    cuboid_t		*cub,		// valid cuboid data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*rot		// not NULL: rotate cuboid
)
{
    if (rot)
	Rotate(origin,rot,cub->p,sizeof(*cub->p),8);
}

//-----------------------------------------------------------------------------

void ShiftCuboid
(
    cuboid_t		*cub,		// valid cuboid data
    const double3	*shift		// not NULL: shift cuboid
)
{
    DASSERT(cub);
    if (shift)
    {
	uint p;
	for ( p = 0; p < 8; p++ )
	{
	    cub->p[p].x += shift->x;
	    cub->p[p].y += shift->y;
	    cub->p[p].z += shift->z;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			index lists			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeIL ( IndexList_t * il, ccp prefix )
{
    DASSERT(il);

    memset(il,0,sizeof(*il));
    il->prefix = prefix;
}

///////////////////////////////////////////////////////////////////////////////

void ResetIL ( IndexList_t * il )
{
    DASSERT(il);
    DASSERT( il->used <= il->size );
    DASSERT( !il->list == !il->size );

    uint i;
    for ( i = 0; i < il->used; i++ )
	FreeString(il->list[i]);
    FREE(il->list);

    InitializeIL(il,il->prefix);
}

///////////////////////////////////////////////////////////////////////////////

void FillIL
(
    IndexList_t		* il,		// valid index list
    uint		min_fill	// fill minimum # elements
)
{
    DASSERT(il);
    DASSERT( il->used <= il->size );
    DASSERT( !il->list == !il->size );

    if (il->prefix)
    {
	if ( min_fill > il->used )
	    DefineIL(il,min_fill-1,0,0);

	uint i;
	for ( i = 0; i < il->used; i++ )
	    if (!il->list[i])
	    {
		if ( il->next_index < i )
		     il->next_index = i;
		char buf[VARNAME_SIZE+1];
		snprintf(buf,sizeof(buf),"%s%u",il->prefix,il->next_index++);
		il->list[i] = STRDUP(buf);
	    }
    }
}

///////////////////////////////////////////////////////////////////////////////

void DefineIL
(
    IndexList_t		* il,		// valid index list
    uint		index,		// index of object
    ccp			name,		// name of object
    bool		move_name	// true: 'name' is alloced -> move it
)
{
    DASSERT(il);
    DASSERT( il->used <= il->size );
    DASSERT( !il->list == !il->size );

    if ( index >= il->size )
    {
	il->size = 3*il->size/2 + 10;
	if ( il->size < index + 10 )
	     il->size = index + 10;
	il->list = REALLOC(il->list,il->size*sizeof(*il->list));
    }
    DASSERT( il->list );
    DASSERT( index < il->size );

    uint used = il->used;
    while ( used <= index )
	il->list[used++] = 0;
    il->used = used;
    DASSERT( index < il->used );

    FreeString(il->list[index]);
    il->list[index] = move_name || !name ? name : STRDUP(name);
    noPRINT("DEFINE-IL() %s = %u\n", name, index );

    if ( name && il->prefix )
    {
	ccp prefix = il->prefix;
	while ( *prefix && toupper((int)*prefix) == toupper((int)*name) )
	    prefix++, name++;

	if ( !*prefix && *name )
	{
	    char *end;
	    uint next = strtoul(name,&end,10) + 1;
	    if ( !*end && il->next_index < next )
		il->next_index = next;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

bool RemoveIL
(
    IndexList_t		* il,		// valid index list
    uint		index		// index of object
)
{
    DASSERT(il);
    DASSERT( il->used <= il->size );
    DASSERT( !il->list == !il->size );
    if ( index >= il->used )
	return false;

    ccp *ptr = il->list + index;
    FreeString(*ptr);
    il->used--;
    if ( index < il->used )
	memmove(ptr,ptr+1,sizeof(*ptr)*(il->used-index));
    il->list[il->used] = 0;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetNameIL
(
    IndexList_t		* il,		// valid index list
    uint		index		// index of object
)
{
    DASSERT(il);
    DASSERT( il->used <= il->size );
    DASSERT( !il->list == !il->size );

    if ( index >= il->used )
	return "-1";
    DASSERT( index < il->used );

    ccp name = il->list[index];
    if (!name)
    {
	FillIL(il,0);
	name = il->list[index];
    }
    DASSERT(name);
    return name;
}

///////////////////////////////////////////////////////////////////////////////

void PrintNameIL
(
    FILE		* f,		// output files
    IndexList_t		* il,		// valid index list
    uint		index,		// index of object
    uint		align,		// align end of name to character pos
    uint		fw,		// field width of name
    uint		indent		// indention of continuation line
)
{
    DASSERT(il);
    DASSERT( il->used <= il->size );
    DASSERT( !il->list == !il->size );

    ccp name = GetNameIL(il,index);
    DASSERT(name);
    const uint nlen = strlen(name);

    if ( nlen > fw )
    {
	// print with continuation line
	if ( align < fw )
	    fprintf(f,"%s\r\n%*s%*s%*s", name, indent,"", align,">", fw-align,">" );
	else
	    fprintf(f,"%s\r\n%*s%*s", name, indent,"", fw,">" );
    }
    else if ( nlen > align )
    {
	// use extra space of fw
	fprintf(f,"%-*s", fw, name );
    }
    else if ( align < fw )
    {
	// aligned output
	fprintf(f,"%*s%*s", align, name, fw-align, "" );
    }
    else
    {
	// aligned output
	fprintf(f,"%*s", fw, name );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			color conversions		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptCmprDefault ( ccp arg )
{
    DASSERT( opt_cmpr_def[4] == 0xff );
    DASSERT( opt_cmpr_def[5] == 0xff );
    DASSERT( opt_cmpr_def[6] == 0xff );
    DASSERT( opt_cmpr_def[7] == 0xff );

    static u8 reset[8] = { 0x00,0x00, 0x00,0x20, 0xff,0xff,0xff,0xff };
    memcpy(opt_cmpr_def,reset,sizeof(opt_cmpr_def));
    opt_cmpr_valid = false;

    if ( !arg || !strcmp(arg,"-") )
	return 0;

    char *end;
    u32 col1 = str2ul(arg,&end,16);
    if ( end == arg )
	return 0;

    u32 col2 = col1;
    arg = end;
    while (isspace((int)*arg))
	arg++;
    if ( *arg == ',' )
    {
	arg++;
	col2 = str2ul(arg,&end,16);
	if ( end == arg )
	    col2 = col1;
    }

    u16 c1 = RGB_to_RGB565(col1);
    u16 c2 = RGB_to_RGB565(col2);
    if ( c1 > c2 )
    {
	const u16 temp = c1;
	c1 = c2;
	c2 = temp;
    }
    else if ( c1 == c2 )
    {
	c1 &= ~0x0020; // modify least significant bit of green
	c2 |=  0x0020;
    }

    write_be16(opt_cmpr_def,c1);
    write_be16(opt_cmpr_def+2,c2);
    opt_cmpr_valid = true;

 #if 0
    PRINT("%08x,%08x -> %04x,%04x\n",col1,col2,c1,c2);
    HEXDUMP16(0,0,opt_cmpr_def,8);
 #endif
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			color conversion tables		///////////////
///////////////////////////////////////////////////////////////////////////////

const u8 cc38[8] = // convert 3-bit color to 8-bit color
{
    0x00,0x24,0x49,0x6d, 0x92,0xb6,0xdb,0xff
};

//-----------------------------------------------------------------------------

const u8 cc48[16] = // convert 4-bit color to 8-bit color
{
    0x00,0x11,0x22,0x33, 0x44,0x55,0x66,0x77, 0x88,0x99,0xaa,0xbb, 0xcc,0xdd,0xee,0xff
};

//-----------------------------------------------------------------------------

const u8 cc58[32] = // convert 5-bit color to 8-bit color
{
    0x00,0x08,0x10,0x19, 0x21,0x29,0x31,0x3a, 0x42,0x4a,0x52,0x5a, 0x63,0x6b,0x73,0x7b,
    0x84,0x8c,0x94,0x9c, 0xa5,0xad,0xb5,0xbd, 0xc5,0xce,0xd6,0xde, 0xe6,0xef,0xf7,0xff
};

//-----------------------------------------------------------------------------

const u8 cc68[64] = // convert 6-bit color to 8-bit color
{
    0x00,0x04,0x08,0x0c, 0x10,0x14,0x18,0x1c, 0x20,0x24,0x28,0x2d, 0x31,0x35,0x39,0x3d,
    0x41,0x45,0x49,0x4d, 0x51,0x55,0x59,0x5d, 0x61,0x65,0x69,0x6d, 0x71,0x75,0x79,0x7d,
    0x82,0x86,0x8a,0x8e, 0x92,0x96,0x9a,0x9e, 0xa2,0xa6,0xaa,0xae, 0xb2,0xb6,0xba,0xbe,
    0xc2,0xc6,0xca,0xce, 0xd2,0xd7,0xdb,0xdf, 0xe3,0xe7,0xeb,0xef, 0xf3,0xf7,0xfb,0xff
};

///////////////////////////////////////////////////////////////////////////////

const u8 cc83[256] = // convert 8-bit color to 3-bit color
{
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x06,0x06,0x06, 0x06,0x06,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x07,0x07,0x07,
    0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07
};

//-----------------------------------------------------------------------------

const u8 cc84[256] = // convert 8-bit color to 4-bit color
{
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x03, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x07,
    0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08,
    0x08,0x09,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x09,
    0x09,0x09,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a,
    0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0b,
    0x0b,0x0b,0x0b,0x0b, 0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d,
    0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e,
    0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0f, 0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f
};

//-----------------------------------------------------------------------------

const u8 cc85[256] = // convert 8-bit color to 5-bit color
{
    0x00,0x00,0x00,0x00, 0x00,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x08,0x08,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x0a,
    0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0d, 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d,
    0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f,
    0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x11,0x11,0x11,0x11, 0x11,0x11,0x11,0x11,
    0x12,0x12,0x12,0x12, 0x12,0x12,0x12,0x12, 0x12,0x13,0x13,0x13, 0x13,0x13,0x13,0x13,
    0x13,0x14,0x14,0x14, 0x14,0x14,0x14,0x14, 0x14,0x15,0x15,0x15, 0x15,0x15,0x15,0x15,
    0x15,0x16,0x16,0x16, 0x16,0x16,0x16,0x16, 0x16,0x16,0x17,0x17, 0x17,0x17,0x17,0x17,
    0x17,0x17,0x18,0x18, 0x18,0x18,0x18,0x18, 0x18,0x18,0x19,0x19, 0x19,0x19,0x19,0x19,
    0x19,0x19,0x1a,0x1a, 0x1a,0x1a,0x1a,0x1a, 0x1a,0x1a,0x1b,0x1b, 0x1b,0x1b,0x1b,0x1b,
    0x1b,0x1b,0x1b,0x1c, 0x1c,0x1c,0x1c,0x1c, 0x1c,0x1c,0x1c,0x1d, 0x1d,0x1d,0x1d,0x1d,
    0x1d,0x1d,0x1d,0x1e, 0x1e,0x1e,0x1e,0x1e, 0x1e,0x1e,0x1e,0x1f, 0x1f,0x1f,0x1f,0x1f
};

//-----------------------------------------------------------------------------

const u8 cc85s1[256] = // convert 8-bit color to 5-bit color, shifted by 1 bit
{
    0x00,0x00,0x00,0x00, 0x00,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x08,0x08,0x08,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0c,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x10,0x10,
    0x10,0x10,0x10,0x10, 0x10,0x10,0x12,0x12, 0x12,0x12,0x12,0x12, 0x12,0x12,0x12,0x14,
    0x14,0x14,0x14,0x14, 0x14,0x14,0x14,0x16, 0x16,0x16,0x16,0x16, 0x16,0x16,0x16,0x18,
    0x18,0x18,0x18,0x18, 0x18,0x18,0x18,0x1a, 0x1a,0x1a,0x1a,0x1a, 0x1a,0x1a,0x1a,0x1a,
    0x1c,0x1c,0x1c,0x1c, 0x1c,0x1c,0x1c,0x1c, 0x1e,0x1e,0x1e,0x1e, 0x1e,0x1e,0x1e,0x1e,
    0x20,0x20,0x20,0x20, 0x20,0x20,0x20,0x20, 0x22,0x22,0x22,0x22, 0x22,0x22,0x22,0x22,
    0x24,0x24,0x24,0x24, 0x24,0x24,0x24,0x24, 0x24,0x26,0x26,0x26, 0x26,0x26,0x26,0x26,
    0x26,0x28,0x28,0x28, 0x28,0x28,0x28,0x28, 0x28,0x2a,0x2a,0x2a, 0x2a,0x2a,0x2a,0x2a,
    0x2a,0x2c,0x2c,0x2c, 0x2c,0x2c,0x2c,0x2c, 0x2c,0x2c,0x2e,0x2e, 0x2e,0x2e,0x2e,0x2e,
    0x2e,0x2e,0x30,0x30, 0x30,0x30,0x30,0x30, 0x30,0x30,0x32,0x32, 0x32,0x32,0x32,0x32,
    0x32,0x32,0x34,0x34, 0x34,0x34,0x34,0x34, 0x34,0x34,0x36,0x36, 0x36,0x36,0x36,0x36,
    0x36,0x36,0x36,0x38, 0x38,0x38,0x38,0x38, 0x38,0x38,0x38,0x3a, 0x3a,0x3a,0x3a,0x3a,
    0x3a,0x3a,0x3a,0x3c, 0x3c,0x3c,0x3c,0x3c, 0x3c,0x3c,0x3c,0x3e, 0x3e,0x3e,0x3e,0x3e
};

//-----------------------------------------------------------------------------

const u8 cc86[256] = // convert 8-bit color to 6-bit color
{
    0x00,0x00,0x00,0x01, 0x01,0x01,0x01,0x02, 0x02,0x02,0x02,0x03, 0x03,0x03,0x03,0x04,
    0x04,0x04,0x04,0x05, 0x05,0x05,0x05,0x06, 0x06,0x06,0x06,0x07, 0x07,0x07,0x07,0x08,
    0x08,0x08,0x08,0x09, 0x09,0x09,0x09,0x0a, 0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0c,
    0x0c,0x0c,0x0c,0x0d, 0x0d,0x0d,0x0d,0x0e, 0x0e,0x0e,0x0e,0x0f, 0x0f,0x0f,0x0f,0x10,
    0x10,0x10,0x10,0x11, 0x11,0x11,0x11,0x12, 0x12,0x12,0x12,0x13, 0x13,0x13,0x13,0x14,
    0x14,0x14,0x14,0x15, 0x15,0x15,0x15,0x15, 0x16,0x16,0x16,0x16, 0x17,0x17,0x17,0x17,
    0x18,0x18,0x18,0x18, 0x19,0x19,0x19,0x19, 0x1a,0x1a,0x1a,0x1a, 0x1b,0x1b,0x1b,0x1b,
    0x1c,0x1c,0x1c,0x1c, 0x1d,0x1d,0x1d,0x1d, 0x1e,0x1e,0x1e,0x1e, 0x1f,0x1f,0x1f,0x1f,
    0x20,0x20,0x20,0x20, 0x21,0x21,0x21,0x21, 0x22,0x22,0x22,0x22, 0x23,0x23,0x23,0x23,
    0x24,0x24,0x24,0x24, 0x25,0x25,0x25,0x25, 0x26,0x26,0x26,0x26, 0x27,0x27,0x27,0x27,
    0x28,0x28,0x28,0x28, 0x29,0x29,0x29,0x29, 0x2a,0x2a,0x2a,0x2a, 0x2a,0x2b,0x2b,0x2b,
    0x2b,0x2c,0x2c,0x2c, 0x2c,0x2d,0x2d,0x2d, 0x2d,0x2e,0x2e,0x2e, 0x2e,0x2f,0x2f,0x2f,
    0x2f,0x30,0x30,0x30, 0x30,0x31,0x31,0x31, 0x31,0x32,0x32,0x32, 0x32,0x33,0x33,0x33,
    0x33,0x34,0x34,0x34, 0x34,0x35,0x35,0x35, 0x35,0x36,0x36,0x36, 0x36,0x37,0x37,0x37,
    0x37,0x38,0x38,0x38, 0x38,0x39,0x39,0x39, 0x39,0x3a,0x3a,0x3a, 0x3a,0x3b,0x3b,0x3b,
    0x3b,0x3c,0x3c,0x3c, 0x3c,0x3d,0x3d,0x3d, 0x3d,0x3e,0x3e,0x3e, 0x3e,0x3f,0x3f,0x3f
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


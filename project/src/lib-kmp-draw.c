
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
 *   Copyright (c) 2011-2021 by Dirk Clemens <wiimm@wiimm.de>              *
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
#include "lib-kcl.h"
#include "db-object.h"
#include "db-file.h"

#include <math.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KMP draw support		///////////////
///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_draw_tab[] =
{
    { 0,		"NONE",		0,		DMD_M_RESET },
    { 0,		"-ALL",		0,		DMD_M_RESET },
    { DMD_M_ALL,	"ALL",		0,		DMD_M_RESET },
    { 0,		"-POINTS",	"-PTS",		DMD_M_PTS },
    { DMD_M_PTS,	"POINTS",	"PTS",		DMD_M_PTS },
    { 0,		"-OBJECTS",	"-GOBJ",	DMD_M_GOBJ },
    { DMD_M_GOBJ,	"OBJECTS",	"GOBJ",		DMD_M_GOBJ },
    { DMD_M_RESPAWN,	"RESPAWN",	0,		DMD_M_RESPAWN },
    { DMD_M_DEFAULT,	"DEFAULT",	0,		DMD_M_ALL },

    { 0,		"-OFFLINE",	0,		DMD_M_OFFLINE },
    { DMD_M_OFFLINE,	"OFFLINE",	0,		DMD_M_OFFLINE },
    { 0,		"-ONLINE",	0,		DMD_M_ONLINE },
    { DMD_M_ONLINE,	"ONLINE",	0,		DMD_M_ONLINE },

    { DMD_1OFFLINE,	"1OFFLINE",	0,		0 },
    { DMD_2OFFLINE,	"2OFFLINE",	0,		0 },
    { DMD_3OFFLINE,	"3OFFLINE",	"4OFFLINE",	0 },
    { DMD_1ONLINE,	"1ONLINE",	0,		0 },
    { DMD_2ONLINE,	"2ONLINE",	0,		0 },
    { DMD_3ONLINE,	"3ONLINE",	"4ONLINE",	0 },

    { DMD_ENPT,		"ENPT",		0,		0 },
    { DMD_ITPT,		"ITPT",		0,		0 },
    { DMD_CKPT,		"CKPT",		0,		0 },
    { DMD_CJGPT,	"CJGPT",	0,		0 },
    { DMD_JGPT,		"JGPT",		0,		0 },
    { DMD_KTPT,		"KTPT",		0,		0 },
    { DMD_CNPT,		"CNPT",		0,		0 },
    { DMD_POTI,		"POTI",		0,		0 },
    { DMD_AREA,		"AREA",		0,		0 },
    { DMD_ITEMBOX,	"ITEMBOXES",	0,		0 },
    { DMD_COIN,		"COINS",	0,		0 },
    { DMD_ROADOBJ,	"ROADOBJECTS",	0,		0 },
    { DMD_SOLIDOBJ,	"SOLIDOBJECTS",	0,		0 },
    { DMD_DECORATION,	"DECORATION",	0,		0 },
    { DMD_BLACK,	"BLACK",	0,		0 },
    { DMD_WHITE,	"WHITE",	0,		0 },
    { DMD_KCL,		"KCL",		0,		0 },

    { DMD_DETAILED,	"DETAILED",	0,		0 },
    { DMD_DISPATCH,	"DISPATCH",	0,		0 },
    { DMD_WARNINGS,	"WARNINGS",	0,		0 },

    { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

draw_mode_t KMP_DRAW = DMD_M_DEFAULT;

int ScanOptDraw ( ccp arg )
{
    if (!arg)
	return 0;

    s64 stat = ScanKeywordList( arg, opt_draw_tab, 0, true, 0,
			*arg == '-' ? DMD_M_ALL : (DMD_M_ALL&~DMD_M_RESET),
			"Option --draw", ERR_SYNTAX );
    if ( stat != M1(stat) )
    {
	KMP_DRAW = stat & DMD_M_ALL;
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetDrawMode()
{
    static char buf[200] = {0};
    if (!*buf)
	PrintKeywordList( buf, sizeof(buf), 0, opt_draw_tab,
				KMP_DRAW, DMD_M_DEFAULT, 0 );
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

#define OBJ_ROAD_COL 0x0000

///////////////////////////////////////////////////////////////////////////////

static void TransformByGOBJ
(
    const
      kmp_gobj_entry_t	*obj,		// object to draw
    DbFlags_t		flags,		// flags DBF_SCALE and DBF_ROTATE

    double3		*pt,		// pointer to first point to transform
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
)
{
    DASSERT(obj);
    DASSERT(pt||!n_pt);

    double3 temp;

    if ( flags & DBF_SCALE )
    {
	temp.x = obj->scale[0];
	temp.y = obj->scale[1];
	temp.z = obj->scale[2];
	Scale(0,&temp,pt,pt_delta,n_pt);
    }

    if ( flags & DBF_ROTATE)
    {
	temp.x = obj->rotation[0];
	temp.y = obj->rotation[1];
	temp.z = obj->rotation[2];
	Rotate(0,&temp,pt,pt_delta,n_pt);
    }

    temp.x = obj->position[0];
    temp.y = obj->position[1];
    temp.z = obj->position[2];
    Translate(&temp,pt,pt_delta,n_pt);
}

///////////////////////////////////////////////////////////////////////////////

typedef struct rot_shape_t { int x; int y; } rot_shape_t;

//-----------------------------------------------------------------------------

static double3 * CreateRotatedShape
(
    // returns an alloced list or NULL on error -> call FREE()

    const rot_shape_t	*shape,		// pointer to the shape
    int			n_shape,	// number of elements of 'shape'
    int			n_segments,	// number of segments

    const
      kmp_gobj_entry_t	*obj,		// NULL or object for transformation
    DbFlags_t		flags		// flags DBF_SCALE and DBF_ROTATE
)
{
    if ( n_shape < 1 || n_segments < 3 )
	return 0;
    DASSERT(shape);

    const uint n_total = n_shape * n_segments;
    double3 *res = MALLOC(n_total*sizeof(*res));

    //--- create first segment

    uint i;
    for ( i = 0; i < n_shape; i++ )
    {
	res[i].x = shape[i].x;
	res[i].y = shape[i].y;
	res[i].z = 0.0;
    }

    double3 rot;
    rot.x = rot.y = rot.z = 0.0;

    //--- create additonal segments

    uint s;
    for ( s = 1; s < n_segments; s++ )
    {
	double3 *dest = res + s * n_shape;
	memcpy( dest, res, sizeof(*dest) * n_shape );
	rot.y = 360.0 * s / n_segments;
	Rotate(0,&rot,dest,sizeof(double3),n_shape);
    }

    if (obj)
	TransformByGOBJ(obj,flags,res,sizeof(double3),n_total);

    return res;
}

///////////////////////////////////////////////////////////////////////////////

static int DrawRotatedShape
(
    struct kcl_t	*dk,		// pointer to valid KCL

    const rot_shape_t	*shape,		// pointer to the shape
    int			n_shape,	// number of elements of 'shape'
    int			n_segments,	// number of segments

    const
      kmp_gobj_entry_t	*obj,		// NULL or object for transformation
    DbFlags_t		flags,		// flags DBF_SCALE and DBF_ROTATE
    u32			col		// print color
)
{
    DASSERT(dk);

    double3 *pt = CreateRotatedShape(shape,n_shape,n_segments,obj,flags);
    if (!pt)
	return 0;

// [[append-p]]

    double3 *s2 = pt + n_shape * (n_segments-1);
    uint s;
    for ( s = 0; s < n_segments; s++ )
    {
	double3 *s1 = s2;
	s2 = pt + n_shape * s;

	double3 *p1 = s1, *p2 = s2;
	uint i;
	for ( i = 0; i < n_shape-1; i++, p1++, p2++ )
	{
	    if (!shape[i+1].x)
	    {
		if ( shape[i+1].y == -1 )
		    i++, p1++, p2++;
		else if (shape[i].x)
		    AppendTriangleKCLp ( dk,col, p1, p1+1,       p2 );
	    }
	    else if (!shape[i].x)
		AppendTriangleKCLp     ( dk,col, p1, p1+1, p2+1     );
	    else
		AppendQuadrilateralKCLp( dk,col, p1, p1+1, p2+1, p2 );
	}
    }

    FREE(pt);
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   AddAreas2KCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

void AddAreas2KCL
(
    DrawKCL_t		* dk		// pointer to initalized object
)
{
    const double width	=  150;
    const double length	= 1500;

    const uint n = dk->kmp->dlist[KMP_AREA].used;
    const kmp_area_entry_t *area = (kmp_area_entry_t*)dk->kmp->dlist[KMP_AREA].list;

    uint i;
    for ( i = 0; i < n; i++, area++ )
    {
	double3 pos, rot, scale, base;
	pos.x	= area->position[0];
	pos.y	= area->position[1];
	pos.z	= area->position[2];
	rot.x	= area->rotation[0];
	rot.y	= area->rotation[1];
	rot.z	= area->rotation[2];
	scale.x	= area->scale[0];
	scale.y	= area->scale[1];
	scale.z	= area->scale[2];
	base.x	= pos.x;
	base.y	= pos.y;
	base.z	= pos.z - length;

	Rotate(&pos,&rot,&base,sizeof(base),1);

	if (dk->detailed)
	    AppendPrismKCL( dk->draw, KCL_FLAG_AREA_ARROW,
			&base, &pos, 0, width, 6, 0,
			KCL_FLAG_AREA_ARROW, width,
			KCL_FLAG_AREA_ARROW, 1.8, false );
	else
	    AppendPrismKCL( dk->draw, KCL_FLAG_AREA_ARROW,
			&base, &pos, 0, width, 4, 1,
			~0, 0.0, ~0, 0.0, false );

	if ( area->mode == 1 )
	{
	    AppendCylinderKCL( dk->draw, KCL_FLAG_AREA_BORDER,
				&pos, &scale, 5000, &rot, 10, KCL_FLAG_AREA_BORDER );
	}
	else
	{
	    double3 size, shift;
	    size.x = size.y = size.z = 10000;
	    shift.x = shift.z = 0;
	    shift.y = 5000;

	    cuboid_t cub;
	    CreateCuboid(&cub,&size,0,&shift);
	    ScaleCuboid(&cub,0,&scale);
	    RotateCuboid(&cub,0,&rot);
	    ShiftCuboid(&cub,&pos);
	    AppendCuboidKCL(dk->draw,KCL_FLAG_AREA_BORDER,&cub);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddCheckPoints2KCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

static void add_ckpt_joist
(
    kcl_t			*kcl,
    const kmp_ckpt_entry_t	*ckpt,
    const kmp_ckpt_entry_t	*c1,
    uint			next,
    double			min,
    double			max,
    double			width,
    u32				kcl_flag_lt,
    u32				kcl_flag_rt
)
{
    const kmp_ckpt_entry_t *c2 = ckpt + next;

    double3 d[3];
    d[0].y = d[1].y	= max;
    d[2].y		= min;

    d[0].x = d[2].x	= c1->left[0];
    d[0].z = d[2].z	= c1->left[1];
    d[1].x		= c2->left[0];
    d[1].z		= c2->left[1];
    AppendJoistKCL(kcl,kcl_flag_lt,width,d,d+1,d+2,0,0);

    d[0].x = d[2].x	= c1->right[0];
    d[0].z = d[2].z	= c1->right[1];
    d[1].x		= c2->right[0];
    d[1].z		= c2->right[1];
    AppendJoistKCL(kcl,kcl_flag_rt,width,d,d+1,d+2,0,0);
}

///////////////////////////////////////////////////////////////////////////////

void AddCheckPoints2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    bool		draw_jgpt	// true: draw connections to JGPT
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp )
	return;

    const double lc_width   = 600.0;
    const double std_width  = 300.0;
    const double tie_width  = 200.0;
    const double next_width = 150.0;
    const double jgpt_width = 150.0;


    //--- find min and max heigth

    float min = 1000000;
    float max = 0;

    uint n = dk->kmp->dlist[KMP_ENPT].used;
    const kmp_enpt_entry_t * pt = (kmp_enpt_entry_t*)dk->kmp->dlist[KMP_ENPT].list;
    for ( ; n > 0; n--, pt++ )
    {
	if ( min > pt->position[1] )
	     min = pt->position[1];
	if ( max < pt->position[1] )
	     max = pt->position[1];
    }
    if ( min > max )
	 min = max;
    min = round( min - 1500.0 );
    max = round( max + 4000.0 );


    //--- create rectangles for the check points

    n = dk->kmp->dlist[KMP_CKPT].used;
    const kmp_ckpt_entry_t *ckpt = (kmp_ckpt_entry_t*)dk->kmp->dlist[KMP_CKPT].list;
    const kmp_ckpt_entry_t *c1   = ckpt;
    uint i;
    for ( i = 0; i < n; i++, c1++ )
    {
	//--- predefine params for check point gate

	uint kcl_flag, n_mark = 0;
	double width = std_width;

	if ( !c1->mode )
	{
	    kcl_flag = KCL_FLAG_CKPT_LC; // KCL_FLAG_CKPT_LC_TT is used by 'kcl_flag+2'
	    width = lc_width;
	}
	else if ( c1->mode != M1(c1->mode) )
	{
	    kcl_flag = KCL_FLAG_CKPT_MD; // KCL_FLAG_CKPT_MD_TT is used by 'kcl_flag+2'
	    n_mark = (c1->mode-1) % 50 + 1;
	}
	else
	    kcl_flag = KCL_FLAG_CKPT_ETC; // KCL_FLAG_CKPT_ETC_TT is used by 'kcl_flag+2'


	//--- iterate all links and draw them

	int std_next = GetNextPointKMP(dk->kmp,KMP_CKPT,c1-ckpt,-1,false,-1,0,0,0,0);
	int link;
	for ( link = 0; link < KMP_MAX_PH_LINK; link++ )
	{
	    bool new_group;
	    int next = GetNextPointKMP(dk->kmp,KMP_CKPT,c1-ckpt,link,true,-1,0,0,&new_group,0);
	    if ( next >= 0 )
	    {
		if ( std_next == next )
		    std_next = -1;

		u32 flag_lt = new_group ? KCL_FLAG_CKPT_TIE_GROUP_LT : KCL_FLAG_CKPT_TIE_LT;
		u32 flag_rt = new_group ? KCL_FLAG_CKPT_TIE_GROUP_RT : KCL_FLAG_CKPT_TIE_RT;

		if (dk->warnings)
		{
		    double min_dist;
		    const int stat = CheckConvexCKPT(dk->kmp,i,next,&min_dist);
		    if ( stat || min_dist < KMP_DIST_WARN )
		    {
			flag_lt = flag_rt = kcl_flag = KCL_FLAG_CKPT_WARNING;
			// KCL_FLAG_CKPT_WARNING_TT is used by 'kcl_flag+2'
		    }
		}

		add_ckpt_joist(dk->draw,ckpt,c1,next,min,max,tie_width,flag_lt,flag_rt);
	    }
	}

	if ( std_next >= 0 )
	    add_ckpt_joist( dk->draw, ckpt, c1, std_next, min, max, next_width,
				KCL_FLAG_CKPT_TIE_NEXT_LT, KCL_FLAG_CKPT_TIE_NEXT_RT );


	//--- draw check point gate

	double3 d[4];
	d[0].x = d[1].x = c1->left[0];
	d[0].z = d[1].z = c1->left[1];
	d[2].x = d[3].x = c1->right[0];
	d[2].z = d[3].z = c1->right[1];
	d[0].y = d[3].y = max;
	d[1].y = d[2].y = min;

	AppendJoistKCL(dk->draw,kcl_flag,width,d+0,d+1,d+3,0,0);
	AppendJoistKCL(dk->draw,kcl_flag,width,d+1,d+2,d+0,0,0);
	AppendJoistKCL(dk->draw,kcl_flag,width,d+2,d+3,d+1,0,0);
	AppendJoistKCL(dk->draw,kcl_flag,width,d+3,d+0,d+2,n_mark,KCL_FLAG_CKPT_MD5);
	AppendTrianglesKCLp(dk->draw,kcl_flag+2,d,sizeof(double3),4);


	//--- draw respawn point links

	if ( draw_jgpt && c1->respawn < dk->kmp->dlist[KMP_JGPT].used )
	{
	    const kmp_jgpt_entry_t *jgpt
		= (kmp_jgpt_entry_t*)dk->kmp->dlist[KMP_JGPT].list + c1->respawn;

	    d[0].y		= max;
	    d[2].y		= min;
	    d[0].x = d[2].x	= c1->left[0];
	    d[0].z = d[2].z	= c1->left[1];
	    d[1].x		= jgpt->position[0];
	    d[1].y		= jgpt->position[1];
	    d[1].z		= jgpt->position[2];
	    AppendJoistKCL(dk->draw,KCL_FLAG_CKPT_JGPT,jgpt_width,d,d+1,d+2,0,0);

	    d[0].x = d[2].x	= c1->right[0];
	    d[0].z = d[2].z	= c1->right[1];
	    AppendJoistKCL(dk->draw,KCL_FLAG_CKPT_JGPT,jgpt_width,d,d+1,d+2,0,0);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddArrow2KCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

void AddArrow2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    uint		sect,		// KMP section
    uint		arrow_color,	// color of arrow
    uint		area_color,	// color of area behind arrow
    double		area_len,	// >0.0: width of area behind arrow
    double		area_wd		// >0.0: length of area behind arrow
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp )
	return;

    const bool have_area = area_len > 0.0 && area_wd > 0.0;
    const double length  = have_area ? area_len : 1500;
    //const double width   = length/2 < 200 ? length/2 : 200;
    const double width   = 150;
    area_wd /= 2;

    const uint n = dk->kmp->dlist[sect].used;
    const kmp_jgpt_entry_t * pt = (kmp_jgpt_entry_t*)dk->kmp->dlist[sect].list;

// [[append-p]]

    uint i;
    for ( i = 0; i < n; i++, pt++ )
    {
	double3 pos, rot, base;
	pos.x	= pt->position[0];
	pos.y	= pt->position[1];
	pos.z	= pt->position[2];
	rot.x	= pt->rotation[0];
	rot.y	= pt->rotation[1];
	rot.z	= pt->rotation[2];
	base.x	= pos.x;
	base.y	= pos.y;
	base.z	= pos.z - length;

	Rotate(&pos,&rot,&base,sizeof(base),1);

	if (dk->detailed)
	    AppendPrismKCL( dk->draw, arrow_color,
			&base, &pos, 0, width, 6, 0,
			arrow_color, width,
			arrow_color, 1.8, false );
	else
	    AppendPrismKCL( dk->draw, arrow_color,
			&base, &pos, 0, width, 4, 1,
			~0, 0.0, ~0, 0.0, false );

	if ( area_len > 0.0 && area_wd > 0.0 )
	{
	    double3 p[4];
	    p[0].x = p[1].x =			pos.x + area_wd;
	    p[2].x = p[3].x =			pos.x - area_wd;
	    p[0].y = p[1].y = p[2].y = p[3].y = pos.y + 20;
	    p[0].z = p[3].z =			pos.z;
	    p[1].z = p[2].z =			pos.z - area_len;

	    Rotate(&pos,&rot,p,sizeof(*p),4);
	    AppendQuadrilateralKCLp( dk->draw, area_color, p, p+1, p+2, p+3 );
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddPotiRoute2KCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

static void AddPotiRoute2KCL
(
    struct kcl_t	*kcl,		// pointer to valid KCL
    const u32		color,		// color
    double		width,		// width of pyramid
    bool		detailed,	// print end points detailed
    const kmp_poti_group_t *pg,		// pointer to group
    const kmp_poti_point_t *pp,		// pointer to first point
    int			limit		// max num of points to draw

)
{
    uint en = pg->n_point;
    if ( en > (uint)limit )
	en = limit;
    if ( !en || limit <= 0 )
	return;

    const kmp_poti_point_t *pp0 = pp;

    double3 pt2;
    pt2.x = pp->position[0];
    pt2.y = pp->position[1];
    pt2.z = pp->position[2];
    pp++;

    if ( en == 1 )
    {
	double3 pt[4];
	pt[0] = pt2; pt[0].z += width;
	pt[1] = pt2; pt[1].x += width;
	pt[2] = pt2; pt[2].z -= width;
	pt[3] = pt2; pt[3].x -= width;

	double3 apex;
	apex = pt2; apex.y += width;
	AppendPyramidKCL(kcl,color,&apex,pt,sizeof(*pt),4,false);
	apex.y = pt2.y - width;
	AppendPyramidKCL(kcl,color,&apex,pt,sizeof(*pt),4,false);

	return;
    }

    uint ei;
    for ( ei = 1; ei < en; ei++ )
    {
	double3 pt1 = pt2;
	pt2.x = pp->position[0];
	pt2.y = pp->position[1];
	pt2.z = pp->position[2];
	pp++;

	double3 helper = pt1;
	helper.y += 1000;

	if (detailed)
	    AppendPrismKCL( kcl, color, &pt1, &pt2, &helper,
				width, 6, 0, color, width, ~0, 0.0, false );
	else
	    AppendPrismKCL( kcl, color, &pt1, &pt2, &helper,
				width, 4, 1, ~0, 0.0, ~0, 0.0, false );
    }

    if ( !pg->back && en > 2 ) // is cyclic
    {

	double3 pt0;
	pt0.x = pp0->position[0];
	pt0.y = pp0->position[1];
	pt0.z = pp0->position[2];

	double3 helper = pt0;
	helper.y += 1000;

	if (detailed)
	    AppendPrismKCL( kcl, color, &pt2, &pt0, &helper,
				width, 6, 0, color, width, ~0, 0.0, false );
	else
	    AppendPrismKCL( kcl, color, &pt2, &pt0, &helper,
				width, 4, 1, ~0, 0.0, ~0, 0.0, false );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddRoutes2KCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

void AddRoutes2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    uint		sect_pt,	// KMP section
    uint		color_std	// standart color index
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp )
	return;
    DASSERT( sect_pt == KMP_ENPT || sect_pt == KMP_ITPT );

    //--- setup

    const uint color_single   = color_std + ( KCL_FLAG_ENPT_SINGLE   - KCL_FLAG_ENPT );
    const uint color_tie_bi   = color_std + ( KCL_FLAG_ENPT_TIE_BI   - KCL_FLAG_ENPT );
    const uint color_tie_prev = color_std + ( KCL_FLAG_ENPT_TIE_PREV - KCL_FLAG_ENPT );
    const uint color_tie_next = color_std + ( KCL_FLAG_ENPT_TIE_NEXT - KCL_FLAG_ENPT );
    const uint color_dispatch = color_std + ( KCL_FLAG_ENPT_DISPATCH - KCL_FLAG_ENPT );

    double3 max_pt;
    max_pt.x = max_pt.y = max_pt.z = 0;
    uint col1_used = 0, col2_used = 0;

    double3 pos_beg[KMP_MAX_LINFO], pos_end[KMP_MAX_LINFO];
    memset(pos_beg,0,sizeof(pos_beg));
    memset(pos_end,0,sizeof(pos_end));

    const double width_std  = 200.0;
    const double width_tie2 = 120.0;
    const double width_next =  70.0;
    const double width_prev =  40.0;
    const double width_oct  =   1.8 * width_std;
    const double dist_oct   =   1.9 * width_std;

    kmp_linfo_t *li = SetupRouteLinksKMP(dk->kmp,sect_pt,false);

    const uint n = dk->kmp->dlist[li->sect_pt].used;
    const kmp_enpt_entry_t *pt = (kmp_enpt_entry_t*)dk->kmp->dlist[li->sect_pt].list;


    //--- draw dispatch points and sub routes, but no links

    uint i;
    double3 pt0;
    bool pt0_valid = false;

    for ( i = 0; i < n; i++ )
    {
	//--- calculate position of point

	double3 pt1, helper;
	pt1.x = helper.x = pt[i].position[0];
	pt1.y            = pt[i].position[1];
	pt1.z = helper.y = pt[i].position[2];
	helper.y = pt1.y + 10000;

	if (i)
	{
	    if ( max_pt.x < pt1.x ) max_pt.x = pt1.x;
	    if ( max_pt.y < pt1.y ) max_pt.y = pt1.y;
	    if ( max_pt.z < pt1.z ) max_pt.z = pt1.z;
	}
	else
	    max_pt = pt1;


	//--- store postions of first and last point of a route

	int ph_index;
	const kmp_enph_entry_t *ph = FindPH(dk->kmp,li->sect_ph,i,&ph_index);
	if (!ph)
	{
	    pt0_valid = false;
	    continue;
	}

	DASSERT( ph_index >= 0 );
	if ( i == ph->pt_start )
	    pos_beg[ph_index] = pt1;

	const bool is_last_pt = i == ph->pt_start + ph->pt_len - 1;
	if ( is_last_pt )
	    pos_end[ph_index] = pt1;


	//-- draw dispatch point

	if ( li->gmode[i] & LINFOG_DISPATCH || li->n_point[i] == 1 )
	{
	    octahedron_t oct;
	    CreateRegularOctahedron(&oct,width_oct,0,&pt1);

	    if ( !( li->gmode[i] & LINFOG_DISPATCH ))
		AppendOctahedronKCL(dk->draw,color_single,&oct);
	    else if ( KMP_DRAW & DMD_DISPATCH )
	    {
		double3 shift;
		shift.x = -dist_oct/2;
		shift.y = width_tie2;
		shift.z = 0.0;
		ShiftOctahedron(&oct,&shift);
		uint color = ph->setting[0] & 7;
		col1_used |= 1 << color;
		AppendOctahedronKCL(dk->draw,KCL_FLAG_TEST0+3*color,&oct);

		shift.x = dist_oct;
		shift.y = 0.0;
		ShiftOctahedron(&oct,&shift);
		color = ph->setting[1] >> 5 & 7;
		col2_used |= 1 << color;
		AppendOctahedronKCL(dk->draw,KCL_FLAG_TEST0+3*color,&oct);
	    }
	    else
		AppendOctahedronKCL(dk->draw,color_dispatch,&oct);
	    pt0_valid = false;
	}
	else
	{
	    if ( pt0_valid )
	    {
		if (dk->detailed)
		    AppendPrismKCL( dk->draw, color_std, &pt0, &pt1, &helper,
					width_std, 6, 0, color_std,
					width_std, ~0, 0.0, false );
		else
		    AppendPrismKCL( dk->draw, color_std, &pt0, &pt1, &helper,
					width_std, 4, 1, ~0,
					0.0, ~0, 0.0, false );
	    }
	    pt0_valid = !is_last_pt;
	    pt0 = pt1;
	}
    }


    //--- draw dispatch points summary

    if ( KMP_DRAW & DMD_DISPATCH && max_pt.y > 0.0 )
    {
	max_pt.x += 1500;
	max_pt.z -= 7*2.2*width_oct;

	double3 shift;
	shift.x = dist_oct;
	shift.y = shift.z = 0.0;

	uint i;
	for ( i = 0; i < 8; i++ )
	{
	    octahedron_t oct1, oct2;
	    CreateRegularOctahedron(&oct1,width_oct,0,&max_pt);
	    CreateRegularOctahedron(&oct2,0.4*width_oct,0,&max_pt);

	    const uint color = KCL_FLAG_TEST0 + 3*i;
	    if ( col1_used & 1<<i )
		AppendOctahedronKCL(dk->draw,color,&oct1);
	    else
		AppendOctahedronKCL(dk->draw,color,&oct2);

	    if (!(i&1))
	    {
		if ( col2_used & 1<<i )
		{
		    ShiftOctahedron(&oct1,&shift);
		    AppendOctahedronKCL(dk->draw,color,&oct1);
		}
		else
		{
		    ShiftOctahedron(&oct2,&shift);
		    AppendOctahedronKCL(dk->draw,color,&oct2);
		}
	    }
	    max_pt.z += 2.2*width_oct;
	}
    }


    //--- draw group links in 2 passes (0,1): bidirect first, others second

    li = SetupRouteLinksKMP(dk->kmp,li->sect_ph,false);

    int pass;
    for ( pass = 0; pass < 2; pass++ )
    {
	for ( i = 0; i < li->used; i++ )
	{
	    const kmp_linfo_mode *lm = li->list[i];
	    uint j0;
	    for ( j0 = 0; j0 < li->used; j0++ )
	    {
		kmp_linfo_mode mode = *lm++;
		if (!mode)
		    continue; // fast skip

		uint	j	= j0;
		uint	color	= 0;
		double	width	= 0.0;
		double3	*pt1	= pos_end+i;
		double3	*pt2	= pos_beg+j;
		u32  arrow_col	= ~0;
		bool arrow_flip	= false;

		if ( mode & LINFO_G_NEXT && li->gmode[i] & LINFOG_DISPATCH )
		{
		    double3 *pt3 = pos_end+j;
		    if ( Length2D(pt1->v,pt3->v) < Length2D(pt1->v,pt2->v) )
		    {
			pt2 = pt3;
			if ( mode & LINFO_G_BY_NEXT )
			    mode |= LINFO_G_BY_PREV;
		    }
		}

		if ( mode & LINFO_G_NEXT )
		{
		    if ( mode & LINFO_G_BY_PREV ||
			 mode & LINFO_G_BY_NEXT && li->gmode[i] & LINFOG_DISPATCH )
		    {
			li->list[i][j] = li->list[j][i] = 0; // reset connection info
			color = color_tie_bi;
			width = width_tie2;
		    }
		    else if (pass)
		    {
			color = arrow_col = color_tie_next;
			width = width_next;
		    }
		}
		else if ( pass && mode & LINFO_G_BY_PREV )
		{
		    color = arrow_col = color_tie_prev;
		    width = width_prev;
		    double3 *temp = pt1;
		    pt1 = pt2;
		    pt2 = temp;
		}

		if (color)
		{
		    double3 helper;
		    helper.x = pt1->x;
		    helper.y = pt1->y + 10000;
		    helper.z = pt1->z;

		    double arrow_size = 0.0;
		    if ( arrow_col != M1(arrow_col) )
		    {
			arrow_size = 1.5;
		    }

		    if (dk->detailed)
			AppendPrismKCL( dk->draw, color, pt1, pt2, &helper,
					    width, 6, 0, color, width,
					    arrow_col, arrow_size, arrow_flip );
		    else
			AppendPrismKCL( dk->draw, color, pt1, pt2, &helper,
					    width, 4, 1, ~0, 0.0,
					    arrow_col, arrow_size, arrow_flip );
		}
	    }
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddPotiRoutes2KCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

void AddPotiRoutes2KCL
(
    DrawKCL_t		* dk		// pointer to initalized object
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp )
	return;

    const double width = 200.0;

    const uint ng = dk->kmp->dlist[KMP_POTI].used;
    const kmp_poti_group_t * pg = (kmp_poti_group_t*)dk->kmp->dlist[KMP_POTI].list;
    const uint np = dk->kmp->poti_point.used;
    const kmp_poti_point_t * pp = (kmp_poti_point_t*)dk->kmp->poti_point.list;

    uint gi, i;
    for ( gi = i = 0; gi < ng && i < np; gi++, pg++ )
    {
	u32 color = KCL_FLAG_POTI0 + gi%4;
	if ( gi < MAX_ROUTE )
	  switch (dk->route[gi])
	  {
	    case DRTM_OFF:
		color = 0;
		break;

	    case DRTM_POTI:
	    //case DRTM_CAME:
		// use default
		break;

	    case DRTM_GOBJ:
		color = KCL_FLAG_SOLID_ROUTE;
		break;

	    case DRTM_ITEMBOX:
		color = KCL_FLAG_ITEMROUTE;
		break;

	    case DRTM_ITEMBOX_ENEMY:
		color = KCL_FLAG_ITEMROUTE_ENEMY;
		break;

	    case DRTM_ITEMBOX_PLAYER:
		color = KCL_FLAG_ITEMROUTE_PLAYER;
		break;
	  }

	if (color)
	    AddPotiRoute2KCL( dk->draw, color, width, dk->detailed, pg, pp, np-i );
	pp += pg->n_point;
	i  += pg->n_point;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddCannons2KCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

const double cannon_aura = 50.0;

//-----------------------------------------------------------------------------

static uint td_minmax
(
    const kcl_tridata_t	*td,		// valid tridata
    double		rot_rad,	// rotation in radiant
    double		*res_min,	// store min data here
    double		*res_max,	// store max data here
    double3		*res_sum	// add points here
)
{
    DASSERT(td);
    DASSERT(res_min);
    DASSERT(res_max);
    DASSERT(res_sum);

    const double3 *pt = td->pt;
    double rad = atan2(pt->x,pt->z) + rot_rad;
    double min, max = sin(rad) * sqrt( pt->x * pt->x + pt->z * pt->z );

    pt++;
    rad = atan2(pt->x,pt->z) + rot_rad;
    double temp = sin(rad) * sqrt( pt->x * pt->x + pt->z * pt->z );
    if ( temp <= max )
	min = temp;
    else
    {
	min = max;
	max = temp;
    }

    pt++;
    rad = atan2(pt->x,pt->z) + rot_rad;
    temp = sin(rad) * sqrt( pt->x * pt->x + pt->z * pt->z );
    *res_min = ( temp < min ? temp : min ) - cannon_aura;
    *res_max = ( temp > min ? temp : max ) + cannon_aura;

    res_sum->x += td->pt[0].x + td->pt[1].x + td->pt[2].x;
    res_sum->y += td->pt[0].y + td->pt[1].y + td->pt[2].y;
    res_sum->z += td->pt[0].z + td->pt[1].z + td->pt[2].z;
    return 3;
}

///////////////////////////////////////////////////////////////////////////////

void AddCannons2KCL
(
    DrawKCL_t		* dk		// pointer to initalized object
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp  || !dk->kcl )
	return;

    const uint ctype = 0x11;
    const double cube_size =  500.0;
    //const double way_size  = 1500.0;

    //--- set td markers for unresolved cannon triangles
    //--- and find first and last cannon (for optimization)

    const uint n_tri = dk->kcl->tridata.used;
    uint i, first_can = n_tri+1, last_can = 0;
    kcl_tridata_t *td = (kcl_tridata_t*)dk->kcl->tridata.list;
    for ( i = 0; i < n_tri; i++, td++ )
    {
	td->status &= ~TD_MARK;
	if (IS_KCL_TYPE(td->cur_flag,ctype))
	{
	    td->status |= TD_MARK;
	    if ( first_can > i )
		 first_can = i;
	    last_can = i;
	}
    }
    PRINT("CANNONS: %u .. %u / %u\n",first_can,last_can,n_tri);
    if ( first_can > last_can )
	return;
    const uint n_can = last_can + 1 - first_can;


    //--- main loop

    const uint nc = dk->kmp->dlist[KMP_CNPT].used;
    const kmp_cnpt_entry_t * cnpt = (kmp_cnpt_entry_t*)dk->kmp->dlist[KMP_CNPT].list;
    uint ic = 0;
    while ( ic < nc )
    {
	//--- find first cannon

	const uint ftype = ctype | ic << 5;
	const double rot_rad = -cnpt->rotation[1] * (M_PI/180.0);

	bool retry = false;
	kcl_tridata_t *td = (kcl_tridata_t*)dk->kcl->tridata.list + first_can;
	kcl_tridata_t *td_end = td + n_can;
	for ( ; td < td_end; td++ )
	    if ( td->status & TD_MARK && td->cur_flag == ftype )
	    {
		double min, max;
		double3 sum;
		sum.x = sum.y = sum.z = 0.0;
		uint n = td_minmax(td,rot_rad,&min,&max,&sum);
		td->status &= ~TD_MARK;
		for ( ; td < td_end; td++ )
		if ( td->status & TD_MARK && td->cur_flag == ftype )
		{
		    double min2, max2;
		    n += td_minmax(td,rot_rad,&min2,&max2,&sum);
		    if ( min2 <= max && max2 >= min )
		    {
			if ( min > min2 )
			     min = min2;
			if ( max < max2 )
			     max = max2;
			td->status &= ~TD_MARK;
		    }
		}
		min += cannon_aura;
		max -= cannon_aura;

		PRINT("CANNON #%u found: %3.1f..%3.1f\n", ic, min, max );

		double3 size;
		size.x = max - min;
		size.y = size.z = cube_size;

		double3 shift;
		shift.x = ( min + max ) /2;
		shift.y = cnpt->position[1];
		shift.z = cos( atan2(cnpt->position[0],cnpt->position[2]) + rot_rad )
			* sqrt(   cnpt->position[0]*cnpt->position[0]
				+ cnpt->position[2]*cnpt->position[2] );

		cuboid_t cub;
		CreateCuboid(&cub,&size,0,&shift);

		double3 rotate;
		rotate.x = rotate.z = 0.0;
		rotate.y = cnpt->rotation[1];
		RotateCuboid(&cub,0,&rotate);

		double f = cnpt->rotation[0];
		if ( f > 0.0 && f <= 100.0 )
		{
		    shift.x = ( cub.p000.x + cub.p111.x ) / 2;
		    shift.y = ( cub.p000.y + cub.p111.y ) / 2;
		    shift.z = ( cub.p000.z + cub.p111.z ) / 2;
		    f /= 100.0;
		    shift.x = ( cnpt->position[0] - shift.x ) * f;
		    shift.y = ( cnpt->position[1] - shift.y ) * f;
		    shift.z = ( cnpt->position[2] - shift.z ) * f;
		    ShiftCuboid(&cub,&shift);
		}

		AppendCuboidKCL(dk->draw,KCL_FLAG_CNPT_LANDING,&cub);

		double3 dest, helper;
		dest.x = helper.x = ( cub.p000.x + cub.p011.x ) / 2;
		dest.y =            ( cub.p000.y + cub.p011.y ) / 2;
		dest.z = helper.z = ( cub.p000.z + cub.p011.z ) / 2;
		helper.y = dest.y + 10000;
		sum.x /= n;
		sum.y /= n;
		sum.z /= n;

		if (dk->detailed)
		    AppendPrismKCL( dk->draw, KCL_FLAG_CNPT_WAY,
				&sum, &dest, &helper, cube_size/4, 6, 0,
				KCL_FLAG_CNPT_WAY, cube_size,
				KCL_FLAG_CNPT_WAY, 2.0, false );
		else
		    AppendPrismKCL( dk->draw, KCL_FLAG_CNPT_WAY,
				&sum, &dest, &helper, cube_size/4, 4, 1,
				~0, 0.0, ~0, 0.0, false );

	    #ifdef TEST0
		size.y   = cube_size/3;
		size.z   = way_size;
		shift.z -= (cube_size+way_size)/2;
		CreateCuboid(&cub,&size,0,&shift);
		RotateCuboid(&cub,0,&rotate);
		AppendCuboidKCL(dk->draw,KCL_FLAG_CNPT_WAY,&cub);
	    #endif

	    #ifdef TEST0
		CreateCube(&cub,10000,0,0);
		AppendCuboidKCL(dk->draw,KCL_FLAG_BLUE,&cub);
	    #endif

		retry = true;
		break;
	    }

	if (!retry)
	{
	    ic++;
	    cnpt++;
	    continue;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddObject helper		///////////////
///////////////////////////////////////////////////////////////////////////////

static void SetupMatrixD
(
    MatrixD_t		*mat,		// will be initialized
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*raise		// NULL or value to raise
)
{
    DASSERT(mat);
    DASSERT(op);

    InitializeMatrixD(mat);

    if ( raise && *raise )
    {
	double3 shift;
	shift.x = shift.z = 0.0;
	shift.y = *raise;
	SetShiftMatrixD(mat,&shift);
    }

    if ( flags & DBF_SCALE )
    {
	double3 scale;
	scale.x = op->scale[0];
	scale.y = op->scale[1];
	scale.z = op->scale[2];
	SetScaleMatrixD(mat,&scale,0);
    }

    if ( flags & DBF_ROTATE )
    {
	SetRotateMatrixD(mat,0,op->rotation[0],0.0,0);
	SetRotateMatrixD(mat,1,op->rotation[1],0.0,0);
	SetRotateMatrixD(mat,2,op->rotation[2],0.0,0);
    }

    double3 trans;
    trans.x = op->position[0];
    trans.y = op->position[1];
    trans.z = op->position[2];
    SetTranslateMatrixD(mat,&trans);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddObject: Add_*()		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef uint (*add_func)
(
    struct kcl_t	*dk,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
);

///////////////////////////////////////////////////////////////////////////////

static uint Add_RECTANGLE
(
    struct kcl_t	*dk,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(dk);
    DASSERT(op);
    DASSERT(data);

    // ==> color x-len z-len x-rot y-rot z-rot

    if ( data_size < 6 )
	return 0;

    u32 col = *data++ + N_KCL_FLAG;
    if (as_road)
	col = OBJ_ROAD_COL;

    double xlen = *data++;
    double zlen = *data++;
    double3 rot;
    rot.x = *data++;
    rot.y = *data++;
    rot.z = *data++;

    double3 pt[4];
    pt[0].x = pt[1].x = -xlen;
    pt[2].x = pt[3].x = +xlen;
    pt[0].y = pt[1].y = pt[2].y = pt[3].y = 25.0; // make it a little bit higher
    pt[0].z = pt[3].z = -zlen;
    pt[1].z = pt[2].z = +zlen;

    Rotate(0,&rot,pt,sizeof(double3),4);
    TransformByGOBJ(op,flags,pt,sizeof(double3),4);
    AppendQuadrilateralKCLp(dk,col,pt,pt+1,pt+2,pt+3);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_CUBE
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);
    DASSERT(data);

     // ==> color width [raise]

    if ( data_size < 2 )
	return 0;

    u32	   col	 = *data++ + N_KCL_FLAG;
    double wd	 = *data++;
    double raise = data_size >= 3 ? *data : 0.0;

    float3 pos;
    pos.x = op->position[0];
    pos.y = op->position[1] + raise;
    pos.z = op->position[2];

    float3 size;
    if ( flags & DBF_SCALE )
    {
	size.x = wd * op->scale[0];
	size.y = wd * op->scale[1];
	size.z = wd * op->scale[2];
    }
    else
	size.x = size.y = size.z = wd;

    cuboid_t cub;
    CreateCuboidF( &cub, size.v, flags & DBF_ROTATE ? op->rotation : 0, pos.v );
    AppendCuboidKCL(draw,col,&cub);

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_CUBOID
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);
    DASSERT(data);

     // ==> color width [raise]

    if ( data_size < 4 )
	return 0;

    u32	   col	 = *data++ + N_KCL_FLAG;
    float3 size;
    size.x = *data++;
    size.y = *data++;
    size.z = *data++;

    float3 shift;
    if ( data_size >= 7 )
    {
	shift.x = *data++;
	shift.y = *data++;
	shift.z = *data++;
    }
    else
	shift.x = shift.y = shift.z = 0.0;

    if ( flags & DBF_SCALE )
    {
	size.x *= op->scale[0];
	size.y *= op->scale[1];
	size.z *= op->scale[2];

	shift.x *= op->scale[0];
	shift.y *= op->scale[1];
	shift.z *= op->scale[2];
    }

    float3 pos;
    pos.x = op->position[0] + shift.x;
    pos.y = op->position[1] + shift.y;
    pos.z = op->position[2] + shift.z;

    cuboid_t cub;
    CreateCuboidF( &cub, size.v, flags & DBF_ROTATE ? op->rotation : 0, pos.v );
    AppendCuboidKCL(draw,col,&cub);

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_CYLINDER_Y
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);
    DASSERT(data);

     // ==> color length radius [raise]

    if ( data_size < 3 )
	return 0;

    u32    col	= *data++ + N_KCL_FLAG;
    double len	= *data++;
    double xr	= *data++;
    double zr	= xr;
    double raise = data_size >= 4 ? *data : 0.0;
    len += raise;

    if ( flags & DBF_SCALE )
    {
	xr	*= op->scale[0];
	len	*= op->scale[1];
	raise	*= op->scale[1];
	zr	*= op->scale[2];
    }

    const uint max_sides = 8;
    double3 pt[2*max_sides+4];

    const uint n_sides = detailed ? max_sides : 6;
    const uint n_used  = 2*n_sides+4;
    double3 *tab0 = pt+2, *tab1 = tab0 + n_sides + 1;

    pt[0].x = pt[0].z = pt[1].x = pt[1].z = 0.0;
    pt[0].y = raise;
    pt[1].y = len;

    double3 *p0 = tab0, *p1 = tab1;
    const double add_rad = M_PI / n_sides;
    double rad = 0.0;

    uint i;
    for ( i = 0; i < n_sides; i++ )
    {
	p0->x = cos(rad) * xr;
	p0->y = raise;
	p0->z = sin(rad) * zr;
	p0++;
	rad += add_rad;

	p1->x = cos(rad) * xr;
	p1->y = len;
	p1->z = sin(rad) * zr;
	p1++;
	rad += add_rad;
    }
    *p0 = *tab0;
    *p1 = *tab1;

    if ( flags & DBF_ROTATE )
    {
	double3 rot;
	rot.x = op->rotation[0];
	rot.y = op->rotation[1];
	rot.z = op->rotation[2];
	Rotate(0,&rot,pt,sizeof(*pt),n_used);
    }

    double3 pos;
    pos.x = op->position[0];
    pos.y = op->position[1];
    pos.z = op->position[2];
    Translate(&pos,pt,sizeof(*pt),n_used);

    for ( i = 0, p0 = tab0, p1 = tab1; i < n_sides; i++, p0++, p1++ )
    {
	AppendTriangleKCLp( draw, col, p0+1, p0, p1   );
	AppendTriangleKCLp( draw, col, p0+1, p1, p1+1 );
    }

    AppendTrianglesKCLp(draw,col,tab0,sizeof(*tab0),n_sides);
    AppendTrianglesKCLp(draw,col,tab1+n_sides-1,-(int)sizeof(*tab1),n_sides);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_CYLINDER_Z
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);
    DASSERT(data);

     // ==> color length radius [raise]

    if ( data_size < 3 )
	return 0;

    u32    col	= *data++ + N_KCL_FLAG;
    double len	= *data++;
    double xr	= *data++;
    double yr	= xr;
    double raise = data_size >= 4 ? *data : 0.0;
    len += raise;

    if ( flags & DBF_SCALE )
    {
	xr	*= op->scale[0];
	yr	*= op->scale[1];
	len	*= op->scale[2];
	raise	*= op->scale[2];
    }

    const uint max_sides = 8;
    double3 pt[2*max_sides+4];

    const uint n_sides = detailed ? max_sides : 6;
    const uint n_used  = 2*n_sides+4;
    double3 *tab0 = pt+2, *tab1 = tab0 + n_sides + 1;

    pt[0].x = pt[0].y = pt[1].x = pt[1].y = 0.0;
    pt[0].z = raise;
    pt[1].z = len;

    double3 *p0 = tab0, *p1 = tab1;
    const double add_rad = M_PI / n_sides;
    double rad = 0.0;

    uint i;
    for ( i = 0; i < n_sides; i++ )
    {
	p0->x = sin(rad) * xr;
	p0->y = cos(rad) * yr + yr;
	p0->z = raise;
	p0++;
	rad += add_rad;

	p1->x = sin(rad) * xr;
	p1->y = cos(rad) * yr + yr;
	p1->z = len;
	p1++;
	rad += add_rad;
    }
    *p0 = *tab0;
    *p1 = *tab1;

    if ( flags & DBF_ROTATE )
    {
	double3 rot;
	rot.x = op->rotation[0];
	rot.y = op->rotation[1];
	rot.z = op->rotation[2];
	Rotate(0,&rot,pt,sizeof(*pt),n_used);
    }

    double3 pos;
    pos.x = op->position[0];
    pos.y = op->position[1];
    pos.z = op->position[2];
    Translate(&pos,pt,sizeof(*pt),n_used);

    for ( i = 0, p0 = tab0, p1 = tab1; i < n_sides; i++, p0++, p1++ )
    {
	AppendTriangleKCLp( draw, col, p0+1, p0, p1   );
	AppendTriangleKCLp( draw, col, p0+1, p1, p1+1 );
    }

    AppendTrianglesKCLp(draw,col,tab0,sizeof(*tab0),n_sides);
    AppendTrianglesKCLp(draw,col,tab1+n_sides-1,-(int)sizeof(*tab1),n_sides);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_ITEMBOX
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);

    float3 pos;
    pos.x = op->position[0];
    pos.y = op->position[1] + 140.0;
    pos.z = op->position[2];

    float3 rot;
    rot.x = 45.000;
    rot.y =  0.000;
    rot.z = 35.264;

    cuboid_t cub;
    CreateCubeF(&cub,240.0,rot.v,pos.v);

    const u32 col = op->setting[1]
			? KCL_FLAG_ITEMBOX_PLAYER
			: op->setting[2]
				? KCL_FLAG_ITEMBOX_ENEMY
				: KCL_FLAG_ITEMBOX;
    AppendCuboidKCL(draw,col,&cub);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_COIN
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);

    u32 col;
    double size;
    if ( op->setting[1] == M1(op->setting[1]) )
    {
	col  = KCL_FLAG_COIN_RESPAWN;
	size = 150;
    }
    else if ( op->setting[1] < KMP_COIN_IS_INVALID )
    {
	col  = KCL_FLAG_COIN_START;
	size = 220;
    }
    else
    {
	col  = KCL_FLAG_COIN_INVALID;
	size = 220;
    }

    float3 pos;
    pos.x = op->position[0];
    pos.y = op->position[1] + 150.0;
    pos.z = op->position[2];

    cuboid_t cub;
    CreateCubeF(&cub,size,0,pos.v);
    AppendCuboidKCL(draw,col,&cub);

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_KINOKO
(
    struct kcl_t	*dk,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(dk);
    DASSERT(op);

    static const rot_shape_t shape[] =
    {
	{    0,  545 },
	{  760,  530 },
	{ 1400,  420 },
	{ 2050,  420 },
	{ 2050,  -50 },
 #ifdef TEST0
	{  100,  -50 },
	{  100,-1000 },
	{    0,-1000 },
	{    0,   -1 },
	{ 2500,  500 },
	{ 3000,  500 },
	{ 3000,    0 },
	{ 2500,    0 },
 #endif
    };

    const uint n_shp = sizeof(shape)/sizeof(*shape);
    const uint n_seg = detailed ? 10 : 6;
    const u32  col   = as_road
			? OBJ_ROAD_COL
			: op->obj_id == 0x1fa
				? KCL_FLAG_KINOKO_GREEN
				: KCL_FLAG_KINOKO_RED;

    return DrawRotatedShape(dk,shape,n_shp,n_seg,op,flags,col);
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_PYRAMID
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);
    DASSERT(data);

    // => color N height radius [raise]

    if ( data_size < 4 )
	return 0;

    u32  col	= *data++ + N_KCL_FLAG;
    uint n_side	= *data++;
    double ht	= *data++;
    double r	= *data++;
    const s16 *raise = data_size >= 4 ? data : 0;

    const uint max_side = 10;
    if ( n_side > max_side )
	 n_side = max_side;

    double3 pt[max_side+1]; // pt[0] is apex
    pt[0].x = pt[0].z = 0.0;
    pt[0].y = ht;

    const double rad_f = 2*M_PI / n_side;
    double3 *p = pt + 1;
    uint i;
    for ( i = 0; i < n_side; i++, p++ )
    {
	const double rad = i * rad_f;
	p->x = cos(rad) * r;
	p->y = 0.0;
	p->z = sin(rad) * r;
    }

    MatrixD_t mat;
    SetupMatrixD(&mat,op,flags,raise);
    TransformD3NMatrixD(&mat,pt,n_side+1,sizeof(*pt));

    AppendPyramidKCL(draw,col,pt,pt+1,sizeof(*pt),n_side,true);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static uint Add_TREE
(
    struct kcl_t	*draw,		// pointer to valid KCL
    const
      kmp_gobj_entry_t	*op,		// object to draw
    DbFlags_t		flags,		// file flags
    const s16		*data,		// valid pointer to data
    uint		data_size,	// size of data
    bool		detailed,	// draw object detailed
    bool		as_road		// draw as road
)
{
    DASSERT(draw);
    DASSERT(op);

    // => trunk_wd trunk_dp trunk_ht  crown_wd crown_dp crown_ht
    // =>  data[0]  data[1]  data[2]   data[3]  data[4]  data[5]

    if ( data_size < 6 )
	return 0;

    s16 d1[] = { KCL_FLAG_TREE_TRUNK - N_KCL_FLAG,
			data[1] + data[2], data[0]/2, -data[1] };
    Add_CYLINDER_Y(draw,op,flags,d1,4,false,false);

    s16 d2[] = { KCL_FLAG_TREE_CROWN - N_KCL_FLAG,
			4, data[4] + data[5], data[3]/2, data[2] - data[4] };
    Add_PYRAMID(draw,op,flags,d2,5,false,false);

    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AddObject*2KCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

void AddObject2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    const
     kmp_gobj_entry_t	* op		// object to draw
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    DASSERT(op);
    if ( !dk || !dk->draw || !op )
	return;

    uint draw_count = 0;
    if ( op->obj_id < N_KMP_GOBJ )
    {
	const ObjectInfo_t *oi = ObjectInfo + op->obj_id;
	if ( oi->fname )
	{
	  int i;
	  for ( i = 0; i < sizeof(oi->group_idx)/sizeof(*oi->group_idx); i++ )
	  {
	    uint gid = oi->group_idx[i];
	    if ( gid >= N_DB_FILE_GROUP )
		break;

	    const s16 *gp = DbFileRefGROUP + DbFileGROUP[gid].ref;
	    while ( *gp >= 0 )
	    {
	      const DbFileFILE_t *f = DbFileFILE + *gp++;
	      if (f->draw_index)
	      {
		const s16 *ptr = DbFileDraw + f->draw_index;
		for(;;)
		{
		    if ( *ptr < 2 )
			break;

		    add_func func = 0;
		    switch(ptr[1])
		    {
			case DFD_RECTANGLE:	func = Add_RECTANGLE; break;
			case DFD_CUBE:		func = Add_CUBE; break;
			case DFD_CUBOID:	func = Add_CUBOID; break;
			case DFD_CYLINDER_Y:	func = Add_CYLINDER_Y; break;
			case DFD_CYLINDER_Z:	func = Add_CYLINDER_Z; break;
			case DFD_ITEMBOX:	func = Add_ITEMBOX; break;
			case DFD_KINOKO:	func = Add_KINOKO; break;
			case DFD_COIN:		func = Add_COIN; break;
			case DFD_PYRAMID:	func = Add_PYRAMID; break;
			case DFD_TREE:		func = Add_TREE; break;
		    }
		    if (func)
			draw_count += func( dk->draw, op, f->flags, ptr+2, *ptr-2,
						dk->detailed, dk->as_road );
		    ptr += *ptr;
		}
	      }
	    }
	  }
	}
    }

    // fall back

    if (!draw_count)
    {
	const float radius = 150;
	float3 pos;
	pos.x = op->position[0];
	pos.y = op->position[1] + radius;
	pos.z = op->position[2];

	octahedron_t oct;
	CreateRegularOctahedronF(&oct,radius,0,pos.v);
	AppendOctahedronKCL(dk->draw,KCL_FLAG_SOLIDOBJ,&oct);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AddObjectsById2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    u16			obj_id		// object id to draw
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp )
	return;

    uint n = dk->kmp->dlist[KMP_GOBJ].used;
    const kmp_gobj_entry_t *op = (kmp_gobj_entry_t*)dk->kmp->dlist[KMP_GOBJ].list;
    for ( ; n > 0; n--, op++ )
	if ( op->obj_id == obj_id )
	    AddObject2KCL(dk,op);
}

///////////////////////////////////////////////////////////////////////////////

void AddObjectsByFlag2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    u32			flag_mask,	// mask the object flag with this ...
    u32			flag_result	// ... and draw it, if the result is this.
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw || !dk->kmp )
	return;

    const draw_mode_t pflags = ( KMP_DRAW & DMD_M_PFLAGS ) >> DMD_SHIFT_PFLAGS;

    uint n = dk->kmp->dlist[KMP_GOBJ].used;
    const kmp_gobj_entry_t *op = (kmp_gobj_entry_t*)dk->kmp->dlist[KMP_GOBJ].list;
    for ( ; n > 0; n--, op++ )
    {
	if ( op->obj_id < N_KMP_GOBJ && ( !pflags || pflags & op->pflags ) )
	{
	    const u32 flags = ObjectInfo[op->obj_id].flags;
	    if ( (flags & flag_mask) == flag_result )
		AddObject2KCL(dk,op);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void AddZeroGround2KCL
(
    DrawKCL_t		* dk,		// pointer to initalized object
    uint		color		// color of ground
)
{
    DASSERT(dk);
    DASSERT(dk->draw);
    if ( !dk || !dk->draw )
	return;

    const uint n_tri = dk->draw->tridata.used;
    if (!n_tri)
	return;

    uint i, n = 0;
    double xmin = 0, xmax = 0, zmin = 0, zmax = 0;
    const kcl_tridata_t *td = (kcl_tridata_t*)dk->draw->tridata.list;

    for ( i = 0; i < n_tri; i++, td++ )
    {
     #if HAVE_PRINT0
	if ( td->cur_flag >= KCL_FLAG_TEST0 && td->cur_flag <= KCL_FLAG_TEST7 )
	    printf("%6x: %04x %04x %7.1f\n", td->cur_flag, td->status,
			td->status & (TD_REMOVED|TD_INVALID), td->pt[0].x );
     #endif

	if ( td->status & (TD_REMOVED|TD_INVALID) )
	    continue;

	const double3 *pt = td->pt;
	if (!n++)
	{
	    xmin = xmax = pt[0].x;
	    zmin = zmax = pt[0].z;
	}

	if ( xmin > pt[0].x ) xmin = pt[0].x;
	if ( xmin > pt[1].x ) xmin = pt[1].x;
	if ( xmin > pt[2].x ) xmin = pt[2].x;

	if ( xmax < pt[0].x ) xmax = pt[0].x;
	if ( xmax < pt[1].x ) xmax = pt[1].x;
	if ( xmax < pt[2].x ) xmax = pt[2].x;

	if ( zmin > pt[0].z ) zmin = pt[0].z;
	if ( zmin > pt[1].z ) zmin = pt[1].z;
	if ( zmin > pt[2].z ) zmin = pt[2].z;

	if ( zmax < pt[0].z ) zmax = pt[0].z;
	if ( zmax < pt[1].z ) zmax = pt[1].z;
	if ( zmax < pt[2].z ) zmax = pt[2].z;
    }

    if (n)
    {
	PRINTF("xmax= %6.1f\n",xmax);
	double3 pt1, pt2, pt3, pt4;
	pt2.x = pt3.x = xmin - 1000;
	pt1.x = pt4.x = xmax + 1000;
	pt3.z = pt4.z = zmin - 1000;
	pt1.z = pt2.z = zmax + 1000;
	pt1.y = pt2.y = pt3.y = pt4.y = 0.0;

	AppendQuadrilateralKCLp(dk->draw,color,&pt1,&pt2,&pt3,&pt4);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			pos file support		///////////////
///////////////////////////////////////////////////////////////////////////////

pos_param_t default_pos_param = {0};
pos_file_t  current_pos_param = {0};
pos_file_t *first_pos_file = 0;

static ParamField_t pos_autocolor = {0};
static u32 pos_colors[] =
{
    KCL_FLAG_ORANGE,
    KCL_FLAG_BLUE,
    KCL_FLAG_GREEN,
    KCL_FLAG_MAGENTA,
    KCL_FLAG_YELLOW,
    KCL_FLAG_RED,
    KCL_FLAG_CYAN,

    KCL_FLAG_GOLD,
    KCL_FLAG_SILVER,
    KCL_FLAG_BRONZE,

    KCL_FLAG_WHITE,
    KCL_FLAG_GREY,
    KCL_FLAG_BROWN,
    KCL_FLAG_BLACK,

    KCL_FLAG_ORANGE_YELLOW,
    KCL_FLAG_BLUE_MAGENTA,
    KCL_FLAG_GREEN_CYAN,
    KCL_FLAG_MAGENTA_RED,
    KCL_FLAG_YELLOW_GREEN,
    KCL_FLAG_RED_ORANGE,
    KCL_FLAG_CYAN_BLUE,
};

const uint n_pos_colors = sizeof(pos_colors)/sizeof(*pos_colors);

///////////////////////////////////////////////////////////////////////////////

static void SetupPosMode()
{
    if (!default_pos_param.color)
    {
	memset(&default_pos_param,0,sizeof(default_pos_param));
	default_pos_param.color = KCL_FLAG_SILVER;
	default_pos_param.idx_x =  8;
	default_pos_param.idx_d = 16;

	memset(&current_pos_param,0,sizeof(current_pos_param));
	current_pos_param.n_param = 1;
	current_pos_param.param[0] = default_pos_param;
    }
}

///////////////////////////////////////////////////////////////////////////////

static void NormalizePosMode ( pos_file_t *pf )
{
    DASSERT(pf);

    uint i;
    pos_param_t *pp;
    for ( i = 0, pp = pf->param; i < pf->n_param; i++, pp++ )
    {
	if (!pp->idx_y) pp->idx_y = pp->idx_x + 1;
	if (!pp->idx_z) pp->idx_z = pp->idx_y + 1;
	if ( pp->transp > 0 )
	{
	    pp->color += pp->transp;
	    pp->transp = 0;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void CopyPosMode()
{
    SetupPosMode();

    pos_file_t *pf;
    for ( pf = first_pos_file; pf; pf = pf->next )
	if (!pf->n_param)
	{
	    ccp save_fname = pf->fname;
	    memcpy(pf,&current_pos_param,sizeof(*pf));
	    pf->fname = save_fname;
	    NormalizePosMode(pf);
	}
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPosMode ( ccp arg )
{
    if (!arg)
	return 0;

    enum { SC_XPOS, SC_YPOS, SC_ZPOS, SC_DIR, SC_COLOR, SC_TRANSP, SC_FILTER };

    static const KeywordTab_t keytab[] =
    {
	{ SC_XPOS,	"X",	"XPOS",		0 },
	{ SC_YPOS,	"Y",	"YPOS",		0 },
	{ SC_ZPOS,	"Z",	"ZPOS",		0 },
	{ SC_DIR,	"D",	"DIR",		0 },

	{ SC_COLOR,	"C",	"COLOR",	0 },
	{ SC_TRANSP,	"T",	"TRANSPARENT",	0 },

	{ SC_FILTER,	"FA",	"FILTER-A",	0 },
	{ SC_FILTER,	"FB",	"FILTER-B",	1 },
	{ SC_FILTER,	"FC",	"FILTER-C",	2 },

	{0,0,0,0}
    };

    SetupPosMode();
    pos_param_t *pp = current_pos_param.param;

    char skip_plus = '+';
    for(;;)
    {
	while ( *arg > 0 && *arg <= ' ' || *arg == ',' || *arg == skip_plus )
	    arg++;
	if (!*arg)
	    break;

	if ( *arg == '+' )
	{
	    if ( current_pos_param.n_param == MAX_POS_PARAM )
		return ERROR0(ERR_SYNTAX,
			"Too many '+' terms for option --pos-mode: %s\n",arg);
	    memcpy(pp+1,pp,sizeof(*pp));
	    pp++;
	    current_pos_param.n_param++;
	    skip_plus = '+';
	    continue;
	}
	skip_plus = ' ';

	char name[50];
	uint i;
	for ( i = 0; i < sizeof(name) - 1; i++ )
	{
	    char ch = toupper((int)(arg[i]));
	    if ( ch < 'A' || ch > 'Z' )
		break;
	    name[i] = ch;
	}
	if (!i)
	    return ERROR0(ERR_SYNTAX,
			"Option --pos-mode: Missing sub command: %s\n",arg);
	name[i] = 0;
	//ccp subcmd = arg;
	arg += i;

	int stat;
	const KeywordTab_t *key = ScanKeyword(&stat,name,keytab);
	if (!key)
	{
	    int cstat;
	    const KeywordTab_t *ckey = ScanKeyword(&cstat,name,kcl_color_key_tab);
	    if (ckey)
	    {
		pp->color = ckey->id;
		pp->transp = ckey->opt & 1 ? 0 : -1;
		goto next_parm;
	    }
	    if ( stat < 1 && cstat > 1 )
		return PrintKeywordError(kcl_color_key_tab,name,cstat,0,
			"Color of --pos-mode:");
	    return PrintKeywordError(keytab,name,stat,0,
			"Sub command of --pos-mode:");
	}

	char *end = 0;
	switch(key->id)
	{
	 case SC_XPOS: pp->idx_x = str2ul(arg,&end,10); break;
	 case SC_YPOS: pp->idx_y = str2ul(arg,&end,10); break;
	 case SC_ZPOS: pp->idx_z = str2ul(arg,&end,10); break;
	 case SC_DIR:  pp->idx_d = str2ul(arg,&end,10); break;

	 case SC_COLOR:
	    {
		const uint num = str2ul(arg,&end,10);
		if  ( num <= MAX_POS_FILE_INDEX )
		    pp->autocolor = num;
	    }
	    break;

	 case SC_TRANSP:
	    {
		const uint num = str2ul(arg,&end,10);
		if ( pp->transp >= 0 && num <= 2 )
		   pp->transp = num;
	    }
	    break;

	 case SC_FILTER:
	    {
		uint idx = str2ul(arg,&end,10);
		if ( (ccp)end > arg )
		{
		    if ( *end == '=' )
			end++;
		    arg = end;
		    pp->idx_f[key->opt] = idx;
		    while ( *arg && *arg != ',' && *arg != '+' )
			arg++;
		    StringCopySM( pp->filter[key->opt], sizeof(*pp->filter),
					end, arg - end );
		    end = 0;
		    PRINT("FILTER[%u.%llu]: %u = %s\n",
				current_pos_param.n_param, key->opt,
				pp->idx_f[key->opt], pp->filter[key->opt]);
		}
	    }
	    break;
	}

	if (end)
	{
	    if ( arg == (ccp)end )
		return ERROR0(ERR_SYNTAX,
			"Option --pos-mode: Missing parameter for '%s': %s\n",
				key->name1,arg);
	    arg = end;
	}

      next_parm:
	while ( *arg > 0 && *arg <= ' ' )
	    arg++;
	if (!*arg)
	    break;
	if ( *arg != ',' && *arg != '+' )
	    return ERROR0(ERR_SYNTAX,
		"Option --pos-mode: Comma or plus expected: %s\n",arg);
    }

 #if HAVE_PRINT0
    {
	pos_param_t *pp = current_pos_param.param;
	PRINT(" - FA %u = %s\n",pp->idx_f[0],pp->filter[0]);
	PRINT(" - FB %u = %s\n",pp->idx_f[1],pp->filter[1]);
	PRINT(" - FC %u = %s\n",pp->idx_f[2],pp->filter[2]);
    }
 #endif
    CopyPosMode();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPosFile ( ccp arg )
{
    if ( !arg || !*arg )
	return 0;

    //SetupPosMode();

    pos_file_t *pf = MALLOC(sizeof(*pf));
    DASSERT(pf);
    memcpy(pf,&current_pos_param,sizeof(*pf));
    pf->fname = STRDUP(arg);
    NormalizePosMode(pf);

    pf->next = first_pos_file;
    first_pos_file = pf;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void DrawPosFiles ( DrawKCL_t *draw )
{
    CopyPosMode();

    pos_file_t *pf;
    for ( pf = first_pos_file; pf; pf = pf->next )
	DrawPosFile(draw,pf);
}

///////////////////////////////////////////////////////////////////////////////

enumError DrawPosFile ( DrawKCL_t *draw, pos_file_t *pf )
{
    DASSERT(draw);

    if ( !pf || !pf->fname || !*pf->fname )
	return ERR_NOTHING_TO_DO;

    SetupPosMode();

    File_t F;
    enumError err = OpenFile(&F,true,pf->fname,FM_STDIO,0,0);
    if (err)
    {
	FreeString(pf->fname);
	pf->fname = 0;
	return err;
    }

    PRINT("scan [N=%u] %s\n",n_param,pf->fname);
    PRINT(" - FA %u = %s\n",pf->param->idx_f[0],pf->param->filter[0]);
    PRINT(" - FB %u = %s\n",pf->param->idx_f[1],pf->param->filter[1]);
    PRINT(" - FC %u = %s\n",pf->param->idx_f[2],pf->param->filter[2]);

    double3 pos[3];
    pos[0].x = pos[0].y = pos[0].z = 0.0;
    double3 rot;
    rot.x = rot.y = rot.z = 0.0;

    char buf[1000];
    ccp cols[MAX_POS_FILE_INDEX+1];
    cols[0] = buf;

    while (fgets(buf,sizeof(buf)-1,F.f))
    {
	char *ptr = buf;

	int idx;
	for ( idx = 1; idx <= MAX_POS_FILE_INDEX; )
	{
	    while ( *ptr == ' ' || *ptr == '\t' )
		ptr++;
	    if ( (uchar)*ptr < ' ' || *ptr == '#' )
		break;

	    cols[idx++] = ptr;
	    while ( (uchar)*ptr > ' ' )
		ptr++;
	    if (!*ptr)
		break;
	    *ptr++ = 0;
	}

	uint i;
	pos_param_t *pp;
	for ( i = 0, pp = pf->param; i < pf->n_param; i++, pp++ )
	{
	    uint fi, ok = 1;
	    for ( fi = 0; fi < MAX_POS_FILTER; fi++ )
	    {
		const uint fidx = pp->idx_f[fi];
		if (!fidx)
		    continue;
		if ( fidx >= idx || strcmp(pp->filter[fi],cols[fidx]) )
		{
		    ok = 0;
		    break;
		}
	    }
	    if (!ok)
		continue;

	    if ( pp->idx_x < idx && pp->idx_y < idx && pp->idx_z < idx )
	    {
		pos[0].x = strtod(cols[pp->idx_x],0);
		pos[0].y = strtod(cols[pp->idx_y],0);
		pos[0].z = strtod(cols[pp->idx_z],0);
	    }
	    else
		continue;

	    if ( pp->idx_d && pp->idx_d < idx )
		rot.y = strtod(cols[pp->idx_d],0);

	    //printf("%9.1f %8.1f %9.1f | %4.0f\n",pos[0].x,pos[0].y,pos[0].z,rot.y);

	    pos[1].x = pos[0].x + 40;
	    pos[2].x = pos[0].x - 40;
	    pos[1].y = pos[2].y = pos[0].y;
	    pos[1].z = pos[2].z = pos[0].z - 120;

	    u32 col = pp->color;
	    if ( pp->autocolor && pp->autocolor < idx && cols[pp->autocolor][0] )
	    {
		if (!pos_autocolor.used)
		    InitializeParamField(&pos_autocolor);

		bool old;
		ParamFieldItem_t *it
		    = FindInsertParamField(&pos_autocolor,cols[pp->autocolor],false,&old);
		if (it)
		{
		    if (!old)
			it->num = ( pos_autocolor.used -1 ) % n_pos_colors;
		    col = pos_colors[it->num];
		    if ( pp->transp >= 0 )
			col += pp->transp;
		}
	    }

	    Rotate(pos,&rot,pos+1,sizeof(double3),2);
	    AppendTriangleKCLp(draw->draw,col,pos+0,pos+1,pos+2);
	}
    }

    CloseFile(&F,0);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


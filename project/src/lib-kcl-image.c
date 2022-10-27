
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

#include "lib-kcl.h"
#include "lib-kmp.h"
#include "lib-image.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanOptPng()			///////////////
///////////////////////////////////////////////////////////////////////////////

bool opt_png		= false; // if true: PNG drawing is enabled
uint opt_png_pix_size	= 0;	// pixel size
uint opt_png_aalise	= 0;	// a factor for antialising
int  opt_png_x1		= 0;	// x1<x2: left border
int  opt_png_x2		= 0;	// x1<x2: right border
int  opt_png_y1		= 0;	// y1<y2: bop forder
int  opt_png_y2		= 0;	// y1<y2: bottom border
u32  opt_png_type_mask	= 0;	// typemask for KCL selection.
bool opt_png_rm		= 0;	// true: remove unused triangles

Var_t opt_png_param = {0};	// a parameter for tests

///////////////////////////////////////////////////////////////////////////////

static int ScanIntSI ( ScanInfo_t *si, int *err )
{
    DASSERT(si);
    DASSERT(err);

    DEFINE_VAR(var);
    if (ScanExprSI(si,&var))
    {
	*err = 1;
	return 0;
    }
    *err = 0;
    const int res = GetIntV(&var);
    FreeV(&var);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

static uint ScanUIntSI ( ScanInfo_t *si, int *err )
{
    DASSERT(si);
    DASSERT(err);

    int res = ScanIntSI(si,err);
    if ( *err || res < 0 )
    {
	*err = 1;
	return 0;
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPng ( ccp arg )
{
    opt_png		= false;
    opt_png_pix_size	= 0;
    opt_png_aalise	= 1;
    opt_png_x1		= 0;
    opt_png_x2		= 0;
    opt_png_y1		= 0;
    opt_png_y2		= 0;
    opt_png_type_mask	= OPT_PNG_TYPE_CLASS;
    opt_png_rm		= 0;
    opt_png_param.mode	= VAR_UNSET;

    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --png",0);
    si.predef = SetupVarsKCL();

    while (NextCharSI(&si,true))
    {
	int err;
	int *opt1 = &opt_png_x1;
	int *opt2 = &opt_png_x2;

	ScanFile_t *sf = si.cur_file;
	DASSERT(sf);

	switch(*sf->ptr)
	{
	    case 'u': case 'U':
		sf->ptr++;
		//fall through
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		opt_png_pix_size = ScanUIntSI(&si,&err);
		if ( err || !opt_png_pix_size )
		    goto syntax_err;
		break;

	    case 'a': case 'A': case '/':
		sf->ptr++;
		opt_png_aalise = ScanUIntSI(&si,&err);
		if (err)
		    goto syntax_err;
		if (!opt_png_aalise)
		    opt_png_aalise = 1;
		break;
	    case 'r': case 'R':
		sf->ptr++;
		opt_png_rm = true;
		break;

	    case 't': case 'T':
		sf->ptr++;
		opt_png_type_mask = ScanIntSI(&si,&err);
		if (err)
		    goto syntax_err;
		break;

	    case 'y': case 'Y':
		opt1 = &opt_png_y1;
		opt2 = &opt_png_y2;
		//fall through
	    case 'x': case 'X':
		sf->ptr++;
		*opt1 = ScanIntSI(&si,&err);
		if (err)
		    goto syntax_err;
		if ( NextCharSI(&si,false) == ':' )
		{
		    sf->ptr++;
		    *opt2 = ScanIntSI(&si,&err);
		    if (err)
			goto syntax_err;
		}
		else
		{
		    if ( *opt1 > 0 )
			*opt1 = -*opt1;
		    *opt2 = -*opt1;
		}
		break;

	    case 'p': case 'P':
		if (ScanExprSI(&si,&opt_png_param))
		    goto syntax_err;
		break;

	    default:
		sf->prev_ptr = sf->ptr;
		goto syntax_err;
	}
    }

    opt_png = opt_png_pix_size > 0;
    ResetSI(&si);
    return 0;

 syntax_err:
    ERROR0(ERR_SYNTAX,
	"Invalid parameter for option --png: %s\n",
	si.cur_file->prev_ptr );
    ResetSI(&si);
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveImageKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct png_param_t
{
    kcl_t		* kcl;		// pointer to valid KCL

    Image_t		img;		// image
    Color_t		col_bg;		// background color
    u32			type_mask;	// kcl type selector (bit field)

    int			x1;		// left side of world
    int			x2;		// right side of world
    int			y;		// start height
    int			z1;		// top side of world
    int			z2;		// bottom side of world

    int			pix_size;	// size of a quadratic pixel

    Color_t	kclcol1[N_KCL_FLAG];	// array with kcl colors
    Color_t	*kclcol2;		// pointer to extended colors
}
png_param_t;

///////////////////////////////////////////////////////////////////////////////

typedef struct png_object_t
{
    int		xpos;		// current x pos
    int		ypos;		// current y pos
    int		zpos;		// current z pos
    int		pix_size;	// current pix_size

    uint	ximg;		// image x pos
    uint	yimg;		// image y pos
}
png_object_t;

///////////////////////////////////////////////////////////////////////////////

static void ImageFall ( png_param_t *p, const png_object_t *o, uint multi )
{
    noTRACE("FALL: *%u: %4u %4u / %6d %6d %6d / %u\n",
	multi, o->ximg, o->yimg, o->xpos, o->ypos, o->zpos, o->pix_size );


    //--- fall

    double3 pos;
    pos.x = o->xpos;
    pos.y = o->ypos;
    pos.z = o->zpos;

    int res_kcl_flag;
    double res = FallKCL(p->kcl,stdout,pos,o->pix_size,0,p->type_mask,&res_kcl_flag);
    if ( res_kcl_flag < 0 )
	return; // nothing found


    //--- draw

    if ( multi == 1 )
    {
	Color_t col;
	if ( res_kcl_flag < N_KCL_FLAG )
	    col = p->kclcol1[res_kcl_flag];
	else if ( res_kcl_flag < NN_KCL_FLAG )
	    col.val = p->kclcol2[res_kcl_flag].val;
	else
	{
	    uint idx = res_kcl_flag - KCL_FLAG_USER_0;
	    col.val = idx < n_user_color
		? user_color[idx].val
		: 0;
	}
	col.a = 0xff;
	TRACE("DRAW: %05x -> %08x\n",res_kcl_flag,col.val);
	noPRINT_IF( res_kcl_flag >= N_KCL_FLAG,
			"DRAW: %05x -> %08x\n", res_kcl_flag, col.val );
	DrawPointIMG(&p->img,o->ximg,o->yimg,col,false);
	return;
    }


    //--- 2x2 split

    multi /= 2;
    png_object_t obj;
    obj.pix_size = o->pix_size / 2;
    const int half = obj.pix_size / 2;
    obj.xpos = o->xpos - half;
    obj.ypos = double2int(res) + half;
    obj.zpos = o->zpos - half;

    obj.ximg = o->ximg;
    obj.yimg = o->yimg;

    ImageFall(p,&obj,multi);

    obj.xpos += obj.pix_size;
    obj.ximg += multi;
    ImageFall(p,&obj,multi);

    obj.zpos += obj.pix_size;
    obj.yimg += multi;
    ImageFall(p,&obj,multi);

    obj.xpos -= obj.pix_size;
    obj.ximg -= multi;
    ImageFall(p,&obj,multi);
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveImageKCL
(
    kmp_t		* kmp,		// pointer to valid KMP
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(kmp);
    DASSERT(kcl);
    DASSERT(fname);


    //--- calc pix_size & pix_aalise

    uint pix_aalise = opt_png_aalise ? opt_png_aalise : 1;
    uint pix_size = opt_png_pix_size / opt_png_aalise;
    if (!pix_size)
    {
	pix_size = 1;
	pix_aalise = opt_png_pix_size;
    }

    const uint multi = 8;


    //--- remove unneeded faces and recalc octree

    CalcMinMaxKCL(kcl);
    const double orig_max_ht = kcl->max.y;

    disable_transformation--;
    TransformKCL(kcl);
    disable_transformation++;

    if (opt_png_rm)
    {
	uint n_tri = kcl->tridata.used;
	kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;

	for ( ; n_tri-- > 0; td++ )
	{
	    if ( td->cur_flag < N_KCL_FLAG
		    && !( 1u << ( td->cur_flag & 0x1f ) & opt_png_type_mask ) )
	    {
		td->status |= TD_REMOVED;
	    }
	}
	DropSortHelper(kcl,TD_REMOVED,0);
    }

 #ifdef TEST

    CalcMinMaxKCL(kcl);
    float raise = kcl->min.y < 10.0 ? 10.0 - kcl->min.y : 0.0;
    float perspective = 50000.0;

    if ( raise > 0.0 || perspective != 0.0 )
    {
	uint n_tri = kcl->tridata.used;
	kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	for ( ; n_tri-- > 0; td++ )
	{
	    td->pt[0].y += raise;
	    td->pt[1].y += raise;
	    td->pt[2].y += raise;
	    td->status |= TD_INVALID_NORM;

	    uint i;
	    for ( i =0; i < 3; i++ )
	    {
		//td->pt[i].x = td->pt[i].x / ( 50000.0 - td->pt[i].y ) * 1000;
		//td->pt[i].z = td->pt[i].z / ( 50000.0 - td->pt[i].y ) * 1000;
		float f = td->pt[i].y / 50000;
		td->pt[i].x = td->pt[i].x * f;
		td->pt[i].z = td->pt[i].z * f;
	    }
	}
    }
 #else
    double raise = 0.0;
    CalcMinMaxKCL(kcl);
    if ( kcl->min.y < 10.0 )
    {
	raise = 10.0 - kcl->min.y;

	uint n_tri = kcl->tridata.used;
	kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	for ( ; n_tri-- > 0; td++ )
	{
	    td->pt[0].y += raise;
	    td->pt[1].y += raise;
	    td->pt[2].y += raise;
	    td->status |= TD_INVALID_NORM;
	}
    }
 #endif

#if 1
    const uint min_cube_size = multi * pix_size / 2;
    if ( kcl->min_cube_size < min_cube_size )
	 kcl->min_cube_size = min_cube_size;
#endif

    kcl->min_max_valid = kcl->stat_valid = kcl->octree_valid = false;
    CreateOctreeKCL(kcl);
    KCL_ACTION_LOG(kcl,"SaveImageKCL(%s) N=%u, model_modified=%d\n",
		fname, kcl->tridata.used, kcl->model_modified );


    //--- find view port

    float xmin, xmax, zmin, zmax;

    const uint n_ckpt =  kmp->dlist[KMP_CKPT].used;
    if (n_ckpt)
    {
	const kmp_ckpt_entry_t *ckpt = (kmp_ckpt_entry_t*)kmp->dlist[KMP_CKPT].list;
	xmin = xmax = ckpt->left[0];
	zmin = zmax = ckpt->left[1];

	uint i;
	for ( i = 0; i < n_ckpt; i++, ckpt++ )
	{
	    if ( xmin > ckpt->left [0] ) xmin = ckpt->left [0];
	    if ( xmax < ckpt->left [0] ) xmax = ckpt->left [0];
	    if ( xmin > ckpt->right[0] ) xmin = ckpt->right[0];
	    if ( xmax < ckpt->right[0] ) xmax = ckpt->right[0];

	    if ( zmin > ckpt->left [1] ) zmin = ckpt->left [1];
	    if ( zmax < ckpt->left [1] ) zmax = ckpt->left [1];
	    if ( zmin > ckpt->right[1] ) zmin = ckpt->right[1];
	    if ( zmax < ckpt->right[1] ) zmax = ckpt->right[1];
	}
	PRINT("SETUP: %2.0f .. %2.0f / %2.0f .. %2.0f\n",xmin,xmax,zmin,zmax);
    }
    else
    {
	CalcMinMaxKCL(kcl);
	xmin = kcl->min.x - 10.0;
	xmax = kcl->max.x + 10.0;
	zmin = kcl->min.z - 10.0;
	zmax = kcl->max.z + 10.0;
    }

    if (transform_active)
    {
	// create a surrounding cuboid, ...
	float3 tab[8];
	tab[0].x = tab[2].x = tab[4].x = tab[5].x = xmin;
	tab[1].x = tab[3].x = tab[5].x = tab[7].x = xmax;
	 tab[0].y = tab[1].y = tab[4].y = tab[5].y = 0.0;
	 tab[2].y = tab[3].y = tab[6].y = tab[7].y = orig_max_ht;
	tab[0].z = tab[1].z = tab[2].z = tab[3].z = zmin;
	tab[4].z = tab[5].z = tab[6].z = tab[7].z = zmax;

	// ... transform ...
	TransformPosFloat3D(tab->v,8,sizeof(*tab));

	// ... and get min/max again
	xmin = xmax = tab[0].x;
	zmin = zmax = tab[0].z;
	uint i;
	for ( i = 1; i < 8; i++ )
	{
	    if ( xmin > tab[i].x ) xmin = tab[i].x;
	    if ( xmax < tab[i].x ) xmax = tab[i].x;
	    if ( zmin > tab[i].z ) zmin = tab[i].z;
	    if ( zmax < tab[i].z ) zmax = tab[i].z;
	}
	PRINT("SETUP: %2.0f .. %2.0f / %2.0f .. %2.0f [TRANSFORM]\n",
			xmin, xmax, zmin, zmax );
    }

    //--- user settings override calcualted settings

    if ( opt_png_x1 < opt_png_x2 )
    {
	xmin = opt_png_x1;
	xmax = opt_png_x2;
    }

    if ( opt_png_y1 < opt_png_y2 )
    {
	zmin = opt_png_y1;
	zmax = opt_png_y2;
    }
    PRINT("SETUP: %2.0f .. %2.0f / %2.0f .. %2.0f\n",xmin,xmax,zmin,zmax);


    //--- limit the output to kcl range

    CalcMinMaxKCL(kcl);
    if ( xmin < kcl->min.x ) xmin = kcl->min.x;
    if ( xmax > kcl->max.x ) xmax = kcl->max.x;
    if ( zmin < kcl->min.z ) zmin = kcl->min.z;
    if ( zmax > kcl->max.z ) zmax = kcl->max.z;

    uint x_total = (int)ceil(xmax-xmin);
    uint z_total = (int)ceil(zmax-zmin);
    PRINT("SETUP: %2.0f .. %2.0f / %2.0f .. %2.0f => %u * %u\n",
		xmin, xmax, zmin, zmax, x_total, z_total );


    //--- setup parameters

    png_param_t param;
    memset(&param,0,sizeof(param));
    param.kcl = kcl;
    param.type_mask = opt_png_type_mask;
    InitializeIMG(&param.img);
    SETCOLOR(param.col_bg,0,0,0x80,0xff);


    //--- pixel coordinates

    param.pix_size = pix_size;

    uint xn  = ALIGN32( (x_total+multi+1) / pix_size, multi );
    uint zn  = ALIGN32( (z_total+multi+1) / pix_size, multi );
    param.x1 = double2int((xmax+xmin)/2) - xn * pix_size / 2;
    param.x2 = param.x1 + xn * pix_size;
    param.z1 = double2int((zmax+zmin)/2) - zn * pix_size / 2;
    param.z2 = param.z1 + zn * pix_size;
    param.y  = double2int(kcl->max.y) + 100;

    TRACE("SETUP: %d .. %d / %d / %d .. %d => %u*%u *%u /%u\n",
		param.x1, param.x2, param.y, param.z1, param.z2,
		xn, zn, pix_aalise, pix_size );
    if ( verbose >= 0 && !(KCL_MODE & KCLMD_SILENT) )
	fprintf(stdlog,
		"  - png draw: x=%d..%d, y=%d, z=%d..%d"
		" => size=%u*%u [a.alising %u], units/pix=%u\n",
		param.x1, param.x2, param.y, param.z1, param.z2,
		xn, zn, pix_aalise > 1 ? pix_aalise : 0, pix_size );
    fflush(stdlog);

    CreateIMG( &param.img, true, xn, zn, param.col_bg );


    //--- calc colors

    Color_t *kclcol2 = (Color_t*)kcl_color;
    param.kclcol2 = kclcol2 - N_KCL_FLAG;

    memset(param.kclcol1,0x00,sizeof(param.kclcol1));
    uint *flag_count = AllocFlagCount();
    CountFlagsKCL(kcl,flag_count);

    uint i, class_idx[N_KCL_CLASS] = {0};
    for ( i = 0; i < N_KCL_TYPE; i++ )
    {
	const kcl_class_t *cls = kcl_class + kcl_type[i].cls;
	uint col_idx = class_idx[cls->cls];
	uint col_max = cls->ncol;

	uint j;
	for ( j = i ; j < N_KCL_FLAG; j += N_KCL_TYPE )
	{
	    if (!flag_count[j])
		continue;

	    param.kclcol1[j].val = kclcol2[cls->color_index+col_idx].val;
	    if ( ++col_idx >= col_max )
		col_idx = 0;
	}
	class_idx[cls->cls] = col_idx ;
    }
    FREE(flag_count);


    //--- setup object and iterate

    png_object_t object;
    memset(&object,0,sizeof(object));

    object.pix_size = multi * param.pix_size;
    object.xpos = param.x1 + object.pix_size / 2;
    object.ypos = param.y;
    object.ximg  = 0;

    while ( object.xpos < param.x2 )
    {
	object.zpos = param.z1 + object.pix_size / 2;
	object.yimg = 0;
	while ( object.zpos < param.z2 )
	{
	    ImageFall(&param,&object,multi);
	    object.zpos += object.pix_size;
	    object.yimg += multi;
	}
	object.xpos += object.pix_size;
	object.ximg += multi;
    }


    //--- terminate

    if ( pix_aalise > 1 )
    {
	PRINT("RESIZE: %u x %u -> %u x %u\n",
	    param.img.width, param.img.height, xn/pix_aalise, zn/pix_aalise );
	SmartResizeIMG(&param.img,false,0,xn/pix_aalise,zn/pix_aalise);
    }

    enumError err = SavePNG(&param.img,true,fname,0,0,true,0);
    ResetIMG(&param.img);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


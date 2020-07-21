
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

#include "lib-std.h"
#include <math.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

u32   set_flags		= M1(u32);
bool  have_set_scale	= false;
bool  have_set_rot	= false;
DEFINE_VAR(set_scale);
DEFINE_VAR(set_rot);

uint	have_set_value = 0; // x=bit-0, y=bit-1, z=bit-2
double3	set_value_min, set_value_max;

///////////////////////////////////////////////////////////////////////////////

int ScanOptSetFlags ( ccp arg )
{
    return ScanSizeOptU32(
		&set_flags,		// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"set-flag",		// ccp opt_name
		0,			// u64 min
		0xffff,			// u64 max
		0,			// u32 multiple
		0,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptSetScale ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --set-scale",0);
    enumError err = ScanVectorExprSI(&si,&set_scale,1.0,1);
    DASSERT( set_scale.mode == VAR_VECTOR );
    have_set_scale = !err && IsNormalD3(set_scale.v);
    if ( err != ERR_OK )
	err = CheckEolSI(&si);
    ResetSI(&si);

    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptSetRot ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --set-rot",0);
    enumError err = ScanVectorExprSI(&si,&set_rot,0.0,1);
    DASSERT( set_rot.mode == VAR_VECTOR );
    have_set_rot = !err && IsNormalD3(set_rot.v);
    if ( err != ERR_OK )
	err = CheckEolSI(&si);
    ResetSI(&si);

    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptSet ( uint xyz, ccp arg )
{
    DASSERT( xyz < 3 );
    if ( !arg || xyz >= 3 )
	return 0;

    char optname[10];
    snprintf(optname,sizeof(optname),"%cset",'x'+xyz);

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),optname,0);
    DEFINE_VAR(min);
    DEFINE_VAR(max);

    const enumError err = ScanMinMaxExprSI(&si,&min,&max);
    CheckEolSI(&si);
    ResetSI(&si);
    if (err)
	return 1;

    set_value_min.v[xyz] = min.v[xyz];
    set_value_max.v[xyz] = max.v[xyz];
    have_set_value     |= 1 << xyz;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptShift ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --shift",0);

    DEFINE_VAR(temp);
    const enumError err = ScanVectorExprSI(&si,&temp,0.0,1);
    DASSERT( temp.mode == VAR_VECTOR );
    CheckEolSI(&si);
    ResetSI(&si);

    if (!err)
	SetShiftMatrixD(&opt_transform,&temp.d3);
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptTranslate ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --translate",0);

    DEFINE_VAR(temp);
    const enumError err = ScanVectorExprSI(&si,&temp,0.0,1);
    DASSERT( temp.mode == VAR_VECTOR );
    CheckEolSI(&si);
    ResetSI(&si);

    if (!err)
	SetTranslateMatrixD(&opt_transform,&temp.d3);
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptScale ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --scale",0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);

    DEFINE_VAR(scale);
    DEFINE_VAR(origin);
    enumError err = ScanVectorExprSI(&si,&scale,1.0,1);
    DASSERT( scale.mode == VAR_VECTOR );
    if ( !err && NextCharSI(&si,false) == '@' )
    {
	sf->ptr++;
	err = ScanVectorExprSI(&si,&origin,0.0,1);
	DASSERT( origin.mode == VAR_VECTOR );
    }
    else
    {
	origin.x = origin.y = origin.z = 0.0;
	origin.mode = VAR_VECTOR;
    }

    CheckEolSI(&si);
    ResetSI(&si);

    if (!err)
	SetScaleMatrixD(&opt_transform,&scale.d3,&origin.d3);

    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptRotate ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --rot",0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);

    DEFINE_VAR(rotate);
    DEFINE_VAR(origin);
    enumError err = ScanVectorExprSI(&si,&rotate,1.0,1);
    DASSERT( rotate.mode == VAR_VECTOR );
    if ( !err && NextCharSI(&si,false) == '@' )
    {
	sf->ptr++;
	err = ScanVectorExprSI(&si,&origin,0.0,1);
	DASSERT( origin.mode == VAR_VECTOR );
    }
    else
    {
	origin.x = origin.y = origin.z = 0.0;
	origin.mode = VAR_VECTOR;
    }

    CheckEolSI(&si);
    ResetSI(&si);

    if (!err)
    {
	SetRotateMatrixD(&opt_transform,0,rotate.x,0.0,&origin.d3);
	SetRotateMatrixD(&opt_transform,1,rotate.y,0.0,&origin.d3);
	SetRotateMatrixD(&opt_transform,2,rotate.z,0.0,&origin.d3);
    }

    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptXRotate ( uint xyz, ccp arg )
{
    DASSERT(xyz<3);
    if ( !arg || xyz >= 3 )
	return 0;

    static ccp tab_info[] = { "Option --xrot", "Option --yrot", "Option --zrot" };

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),tab_info[xyz],0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    double degree = GetDoubleV(&temp);

    if ( !err && NextCharSI(&si,false) == '@' )
    {
	sf->ptr++;
	err = ScanVectorExprSI(&si,&temp,0.0,xyz);
	DASSERT( temp.mode == VAR_VECTOR );
    }
    else
    {
	temp.x = temp.y = temp.z = 0.0;
	temp.mode = VAR_VECTOR;
    }
    CheckEolSI(&si);
    ResetSI(&si);

    if (!err)
	SetRotateMatrixD(&opt_transform,xyz,degree,0.0,&temp.d3);

    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptAScale ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --ascale",0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    if (err)
	goto abort;
    const double scale = GetDoubleV(&temp);
    if ( fabs(scale) < NULL_EPSILON )
    {
	err = ERROR0(ERR_SEMANTIC,
			"Option --ascale: Scale factor too small: %s\n",arg);
	goto abort;
    }

    err = CheckWarnSI(&si,'@',err);
    if (err)
	goto abort;
    ccp arg_vector =sf->ptr;

    DEFINE_VAR(dir);
    err = ScanVectorExprSI(&si,&dir,0.0,1);
    if (err)
	goto abort;
    DASSERT( dir.mode == VAR_VECTOR );

    const double dir_len = Length3(dir);
    if ( dir_len < NULL_EPSILON )
    {
	err = ERROR0(ERR_SEMANTIC,
			"Option --ascale: Vector is too small: %s\n",arg_vector);
	goto abort;
    }
    err = CheckEolSI(&si);
    if (err)
	goto abort;

    //--- valid axis scale entered

    ascale_valid = fabs(scale) >= NULL_EPSILON && fabs(scale-10.0) >= NULL_EPSILON;
    ascale = scale;
    Div3f(ascale_dir,dir,dir_len);
    NextTransformation(false);

 abort:
    ResetSI(&si);
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptARotate ( ccp arg )
{
     if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --arot",0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    if (err)
	goto abort;
    const double degree = fmod(GetDoubleV(&temp)+180.0,360.0)-180.0;

    err = CheckWarnSI(&si,'@',err);
    if (err)
	goto abort;
    ccp arg_vector =sf->ptr;

    DEFINE_VAR(p1);
    DEFINE_VAR(p2);
    err = ScanVectorExprSI(&si,&p2,0.0,1);
    if (err)
	goto abort;
    DASSERT( p2.mode == VAR_VECTOR );

    if ( NextCharSI(&si,false) == '@' )
    {
	p1 = p2;
	sf->ptr++;
	err = ScanVectorExprSI(&si,&p2,0.0,1);
	if (err)
	    goto abort;
	DASSERT( p2.mode == VAR_VECTOR );
    }
    else
	p1.x = p1.y = p1.z = 0.0;

    double3 delta;
    Sub3(delta,p1,p2);
    const double len = Length3(delta);
    if ( len < NULL_EPSILON )
    {
	err = ERROR0(ERR_SEMANTIC,
			"Option --arot: Points to near: %s\n",arg_vector);
	goto abort;
    }
    err = CheckEolSI(&si);
    if (err)
	goto abort;

    //--- valid axis rotation entered

    arot_valid = true; //fabs(degree) >= MIN_DEGREE;
    arot_deg = degree;
    arot1 = p1.d3;
    arot2 = p2.d3;
    NextTransformation(false);

 abort:
    ResetSI(&si);
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptYPos ( ccp arg )
{
    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Options --ypos",0);

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    yposition_valid = temp.mode != VAR_UNSET;
    yposition = GetYDoubleV(&temp);
    CheckEolSI(&si);
    ResetSI(&si);
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptXSS ( uint xyz, ccp arg )
{
    DASSERT( xyz < 3 );
    if ( !arg || xyz >= 3 )
	return 0;

    static ccp tab_info[] = { "Option --xss", "Option --yss", "Option --zss" };
    ccp opt_info = tab_info[xyz];

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),opt_info,0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);
    sf->disable_comma++;

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    err = CheckWarnSI(&si,',',err);
    if (err)
	goto abort;
    double old1 = GetDoubleV(&temp);

    err = ScanExprSI(&si,&temp);
    err = CheckWarnSI(&si,',',err);
    if (err)
	goto abort;
    double new1 = GetDoubleV(&temp);

    err = ScanExprSI(&si,&temp);
    err = CheckWarnSI(&si,',',err);
    if (err)
	goto abort;
    double old2 = GetDoubleV(&temp);

    err = ScanExprSI(&si,&temp);
    if (err)
	goto abort;
    double new2 = GetDoubleV(&temp);

    CheckEolSI(&si);

    if ( fabs(old1-old2) < 1e-6 )
    {
	err = ERROR0(ERR_SEMANTIC,
		"%s: 'old' values are to close together, delta=%6.3e",
		opt_info, fabs(old1-old2) );
	goto abort;
    }

    SetScaleShiftMatrixD(&opt_transform,xyz,old1,new1,old2,new2);

 abort:
    ResetSI(&si);
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////		    Transformation Script		///////////////
///////////////////////////////////////////////////////////////////////////////

static ccp		 tfs_filename = 0;
static u8		*tfs_data = 0;
static uint		 tfs_size = 0;
static ScanInfo_t	 tfs_si = {0};
static ScanMacro_t	*tfs_macro = 0;
static uint		 tfs_index = 0;

#if HAVE_PRINT
    static uint		 tfs_count_f = 0;
    static uint		 tfs_count_d = 0;
#endif

bool tform_script_enabled = false;

///////////////////////////////////////////////////////////////////////////////

int ScanOptTformScript ( ccp arg )
{
    tfs_filename = arg;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTformScript()
{
    tfs_macro = 0;
    tform_script_enabled = false;

    if ( !tfs_filename || !*tfs_filename )
	return ERR_NOTHING_TO_DO;

    enumError err = OpenReadFILE(tfs_filename,0,false,&tfs_data,&tfs_size,0,0);
    if ( err || !tfs_data || !tfs_size )
    {
     abort:
	FREE(tfs_data);
	tfs_data = 0;
	tfs_size = 0;
	tfs_filename = 0;
	return err;
    }

    InitializeSI(&tfs_si,(ccp)tfs_data,tfs_size,tfs_filename,REVISION_NUM);
    while (NextLineSI(&tfs_si,false,true))
	;  //--- scan source

    const Var_t *vp = FindVarMap(&tfs_si.macro,"TRANSFORM",0);
    if ( !vp || !vp->macro )
    {
	ResetSI(&tfs_si);
	err = ERR_NOTHING_TO_DO;
	goto abort;
    }

    tfs_macro = vp->macro;
    PRINT("MACRO TRANSFORM found\n");
    tform_script_enabled = true;
    transform_active = true;
    have_patch_count++;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool tfs_call_macro
(
    const ScanMacro_t	*macro,		// macro to call
    uint		dimension	// '2' for a 2D and '3' for a 3D vector
)
{
    DASSERT(tform_script_enabled);
    DASSERT(macro);


    //--- define local variables, '$P' is already defined

    Var_t *var = InsertVarMap(&tfs_si.lvar,"$D",false,0,0);
    DASSERT(var);
    var->mode = VAR_INT;
    var->i = dimension;

    var = InsertVarMap(&tfs_si.lvar,"$I",false,0,0);
    DASSERT(var);
    var->mode = VAR_INT;
    var->i = tfs_index++;


    //--- setup macro

    ScanFile_t *sf = tfs_si.cur_file;
    DASSERT(sf);

    const uint saved_n_files = tfs_si.n_files;
    tfs_si.n_files = 0;
    tfs_si.cur_file = &empty_scan_file;
    tfs_si.last_result.mode = VAR_UNSET;

    ScanFile_t *newsf = AddSF( &tfs_si, macro->data, macro->data_size,
				macro->src_name, REVISION_NUM, 0 );
    DASSERT(newsf);

    newsf->active_macro = macro;
    newsf->line = macro->line0;

    //--- scan source

    while (NextLineSI(&tfs_si,false,true))
	;

    //--- close source

    while (DropSF(&tfs_si))
	;
    tfs_si.cur_file = sf;
    tfs_si.n_files = saved_n_files;

    //--- terminate

    return tfs_si.last_result.mode == VAR_VECTOR;
}

///////////////////////////////////////////////////////////////////////////////

bool ScanTformBegin()
{
    if (!tform_script_enabled)
	return false;
    DASSERT(tfs_macro);

    ClearVarMap(&tfs_si.lvar);
    //ClearVarMap(&tfs_si.gvar);

    const Var_t *vp = FindVarMap(&tfs_si.macro,"BEGIN",0);
    if ( vp && vp->macro )
    {
	tfs_index = -1;
	tfs_call_macro(vp->macro,0);
    }

    tfs_index = 0;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TformScriptEnd()
{
    if (!tform_script_enabled)
	return false;

    PRINT("### TFROM-STAT:  %llu + %llu\n",
		N_MatrixD_forward, N_MatrixD_inverse );
    PRINT("### SCRIPT-STAT: %u | %u + %u = %u\n",
		tfs_index,
		tfs_count_f, tfs_count_d, tfs_count_f + tfs_count_d );

    const Var_t *vp = FindVarMap(&tfs_si.macro,"END",0);
    if ( vp && vp->macro )
    {
	tfs_call_macro(vp->macro,0);
	tfs_index--;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError TformScriptCallF
(
    uint	dimension,	// '2' for a 2D and '3' for a 3D vector
    float3	*v,		// pointer to list of 3D vectors
    int		n,		// number of 3D vectors
    uint	off		// if n>1: offset from one to next vector
)
{
    DASSERT(tform_script_enabled);
    DASSERT(tfs_macro);

    if ( n <= 0 || !tfs_macro )
	return ERR_NOTHING_TO_DO;
    DASSERT(v);

   #if HAVE_PRINT
    tfs_count_f += n;
   #endif

    for(;;)
    {
	ClearVarMap(&tfs_si.lvar);
	Var_t *var = InsertVarMap(&tfs_si.lvar,"$P",false,0,0);
	DASSERT(var);
	var->mode = VAR_VECTOR;
	var->x = v->x;
	var->y = v->y;
	var->z = v->z;

	if (tfs_call_macro(tfs_macro,dimension))
	{
	    v->x = tfs_si.last_result.x;
	    v->y = tfs_si.last_result.y;
	    v->z = tfs_si.last_result.z;
	}

	if (!--n)
	    return ERR_OK;
	v = (typeof(v))( (u8*)v + off );
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError TformScriptCallD
(
    uint	dimension,	// '2' for a 2D and '3' for a 3D vector
    double3	*v,		// pointer to list of 3D vectors
    int		n,		// number of 3D vectors
    uint	off		// if n>1: offset from one to next vector
)
{
    DASSERT(tform_script_enabled);
    DASSERT(tfs_macro);

    if ( n <= 0 || !tfs_macro )
	return ERR_NOTHING_TO_DO;
    DASSERT(v);

   #if HAVE_PRINT
    tfs_count_d += n;
   #endif

    for(;;)
    {
	ClearVarMap(&tfs_si.lvar);
	Var_t *var = InsertVarMap(&tfs_si.lvar,"$P",false,0,0);
	DASSERT(var);
	var->mode = VAR_VECTOR;
	var->x = v->x;
	var->y = v->y;
	var->z = v->z;

	if (tfs_call_macro(tfs_macro,dimension))
	{
	    v->x = tfs_si.last_result.x;
	    v->y = tfs_si.last_result.y;
	    v->z = tfs_si.last_result.z;
	}

	if (!--n)
	    return ERR_OK;
	v = (typeof(v))( (u8*)v + off );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    Transformation Setup		///////////////
///////////////////////////////////////////////////////////////////////////////

uint	force_transform		= 0;		// bit 0: force tform even if not needed
						// bit 1: ignore it

MatrixD_t opt_transform		= {0};		// current transform values
MatrixD_t transform_list[N_TRANSFORM] = {{0}};	// list with stored tforms

uint	n_transform_list	= 0;		// number of elements in list
bool    transform_active	= false;	// true: transformation active
int	disable_transformation	= 0;		// >0: transformation is temporary disabled

double  yposition		= 0.0;		// defined y position
bool    yposition_valid		= false;	// 'yposition' is valid

bool	ascale_valid		= false;	// true: --ascale defined
double	ascale			= 0.0;		// scaling factor
double3	ascale_dir;				// scaling direction (vector)

bool	arot_valid		= false;	// true: --arot defined
double	arot_deg		= 0.0;		// angle of arot in degree
double3	arot1, arot2;				// points to define rotation axis

uint	mdl_switched_to_vertex	= 0;		// 0: default
						// 1: switch to 'VERTEX' needed
						// 2: MDL_MODE switched to 'VERTEX'
						// 3: '2' and message printed

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ResetTransformation() // reset all transformation steps
{
    if (transform_active)
    {
	transform_active = false;
	have_patch_count--;
    }

    n_transform_list = 0;
    InitializeMatrixD(&opt_transform);
}

///////////////////////////////////////////////////////////////////////////////

int NextTransformation ( bool for_close ) // enter next transformation step
{
    CalcMatrixD(&opt_transform,true);
    if ( opt_transform.transform_enabled || force_transform == 1 && for_close )
    {
	force_transform |= 2;
	if ( n_transform_list >= N_TRANSFORM-!for_close )
	{
	    static bool err_done = false;
	    if (!err_done)
	    {
		err_done = true;
		ERROR0(ERR_SEMANTIC,
			"Maximum number of --next (%u) already reached.\n",
			N_TRANSFORM-1 );
	    }
	    return 1;
	}

	memcpy( transform_list + n_transform_list,
		&opt_transform,
		sizeof(*transform_list) );

	n_transform_list++;
	PRINT("--next: level %u/%u\n",n_transform_list,N_TRANSFORM);
    }

    InitializeMatrixD(&opt_transform);


    //--- special: axis scaling

    if (ascale_valid)
    {
	ascale_valid = false;
	SetAScaleMatrixD(&opt_transform,ascale,&ascale_dir);
	NextTransformation(for_close);
    }


    //--- special: axis rotation

    if (arot_valid)
    {
	arot_valid = false;
	SetARotateMatrixD(&opt_transform,arot_deg,0.0,&arot1,&arot2);
	NextTransformation(for_close);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void CloseTransformation() // close transformation input
{
    NextTransformation(true);
    transform_active = n_transform_list > 0;

    if (!transform_active)
	InitializeMatrixD(&opt_transform);
    else
    {
	have_patch_count++;
	memcpy( &opt_transform, transform_list, sizeof(opt_transform) );

	uint i;
	for  ( i = 1; i < n_transform_list; i++ )
	    MultiplyMatrixD(&opt_transform,0,transform_list+i,0);
    }
    CalcMatrixD(&opt_transform,true);
    mdl_switched_to_vertex = !opt_transform.norm_valid;

    ScanTformScript();

    if (transform_active)
	PATCH_ACTION_LOG("Setup","-","%u transformation step%s defined.\n",
		n_transform_list, n_transform_list == 1 ? "" : "s" );

}

///////////////////////////////////////////////////////////////////////////////

void DumpTransformationOpt()
{
    if (have_set_scale)
	printf("  set-scale:        %11.3f %11.3f %11.3f\n",
		set_scale.x, set_scale.y, set_scale.z );

    if (have_set_rot)
	printf("  set-rot:          %11.3f %11.3f %11.3f\n",
		set_rot.x, set_rot.y, set_rot.z );

    if (have_set_value)
	printf(	"  set-xyz-min:      %11.3f %11.3f %11.3f\n"
		"  set-xyz-max:      %11.3f %11.3f %11.3f\n",
		set_value_min.x, set_value_min.y, set_value_min.z,
		set_value_max.x, set_value_max.y, set_value_max.z );

    if ( ascale != 0.0 )
    {
	printf(	"  last axis-scale:  %11.3f @ %11.3f %11.3f %11.3f\n",
		ascale,
		ascale_dir.x, ascale_dir.y, ascale_dir.z );
    }

    if ( arot_deg != 0.0 )
    {
	printf(	"  last axis-rot:    %11.3f @ %11.3f %11.3f %11.3f\n"
		"%33s %11.3f %11.3f %11.3f\n",
		arot_deg, arot1.x, arot1.y, arot1.z,
		"->",     arot2.x, arot2.y, arot2.z );
    }

    if (!transform_active)
	return;

    if ( n_transform_list > 1 )
    {
	uint i;
	for ( i = 0; i < n_transform_list; i++ )
	{
	    printf("\nTransformation step %u/%u:\n",i+1,n_transform_list);
	    if ( verbose > 0 )
		PrintMatrixD(stdout,2,0,transform_list+i,~0,0x1f);
	    else
		PrintMatrixD(stdout,2,0,transform_list,1,3);
	}
	printf("\nFinal Transformation:\n");
    }
    else
	printf("\nTransformation:\n");

    if ( verbose > 0 )
	PrintMatrixD(stdout,2,0,&opt_transform,~0,0x1f);
    else
	PrintMatrixD(stdout,2,0,&opt_transform,1,3);
}

///////////////////////////////////////////////////////////////////////////////

double3 TransformD3( uint mode, double3 *val )	// transformate a single vector
{
    DASSERT(val);

    uint i;
    double3 res;
    switch (mode)
    {
	case 1:
	    res = *val;
	    for ( i = 0; i < n_transform_list; i++ )
		res = BaseTransformD3MatrixD(transform_list+i,&res);
	    break;

	case 2:
	    res = *val;
	    for ( i = 0; i < n_transform_list; i++ )
		res = NormTransformD3MatrixD(transform_list+i,&res);
	    break;

	case 3:
	    res = *val;
	    for ( i = 0; i < n_transform_list; i++ )
		res = TransformD3MatrixD(transform_list+i,&res);
	    break;

	default:
	    res = TransformD3MatrixD(&opt_transform,val);
	    break;
    }
    if (tform_script_enabled)
	TformScriptCallD(3,&res,1,0);
    return res;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Transformations			///////////////
///////////////////////////////////////////////////////////////////////////////

void TransformPosFloat2D
(
    float	*v,	// pointer to list of 2D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= 2 * sizeof(*v) );

    CalcNormMatrixD(&opt_transform); // calc 'norm_pos2d'
    const double ypos = yposition_valid ? yposition : opt_transform.norm_pos2d.y;

    off -= 2 * sizeof(*v);
    while ( n-- > 0 )
    {
	float3 f;
	f.x = v[0];
	f.y = ypos;
	f.z = v[1];
	f = TransformF3MatrixD(&opt_transform,&f);
	if (tform_script_enabled)
	    TformScriptCallF(2,&f,1,0);
	*v++ = f.x;
	*v++ = f.z;
	v = (float*)( (u8*)v + off );
    }
}

///////////////////////////////////////////////////////////////////////////////

void TransformPosDouble2D
(
    double	*v,	// pointer to list of 2D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= 2 * sizeof(*v) );

    CalcNormMatrixD(&opt_transform); // calc 'norm_pos2d'
    const double ypos = yposition_valid ? yposition : opt_transform.norm_pos2d.y;

    off -= 2 * sizeof(*v);
    while ( n-- > 0 )
    {
	double3 d;
	d.x = v[0];
	d.y = ypos;
	d.z = v[1];
	d = TransformD3MatrixD(&opt_transform,&d);
	if (tform_script_enabled)
	    TformScriptCallD(2,&d,1,0);
	*v++ = d.x;
	*v++ = d.z;
	v = (double*)( (u8*)v + off );
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool IsNormValid()
{
    CalcNormMatrixD(&opt_transform);
    if (opt_transform.norm_valid)
	return true;

    static bool done = false;
    if ( !done && opt_transform.tmatrix_valid )
    {
	done = true;
	ERROR0(ERR_WARNING,
		"Because of the complex transformation matrix, it is impossible"
		" to modify the scale and rotate vectors of different KMP sections;"
		" the vectors keep unchanged. A manual correction is needed."
		" The position vectors can always be transformed." );
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void TransformScaleFloat1D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= sizeof(*v) );

    if ( IsNormValid() && opt_transform.scale_enabled )
    {
	// only for KMP ENPT+ITPT scales:
	const float scale_1d = ( fabs(opt_transform.norm_scale.x)
			       + fabs(opt_transform.norm_scale.z) ) / 2;

	while ( n-- > 0 )
	{
	    *v *= scale_1d;
	    v = (float*)( (u8*)v + off );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void TransformScaleFloat3D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= 3 * sizeof(*v) );

    if ( IsNormValid() && opt_transform.scale_enabled )
    {
	const double3 *scale = &opt_transform.norm_scale;

	while ( n-- > 0 )
	{
	    v[0] *= scale->x;
	    v[1] *= scale->y;
	    v[2] *= scale->z;
	    v = (float*)( (u8*)v + off );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void TransformScaleDouble3D
(
    double	*v,	// pointer to list of 3D vectors
    int		n,	// number of 3D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= 3 * sizeof(*v) );

    if ( IsNormValid() && opt_transform.scale_enabled )
    {
	const double3 *scale = &opt_transform.norm_scale;
	while ( n-- > 0 )
	{
	    v[0] *= scale->x;
	    v[1] *= scale->y;
	    v[2] *= scale->z;
	    v = (double*)( (u8*)v + off );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TransformRotFloat3D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= 3 * sizeof(*v) );

    if ( IsNormValid() && opt_transform.rotate_enabled )
    {
	const double3 *rotate = &opt_transform.norm_rotate_deg;

	while ( n-- > 0 )
	{
	    v[0] = fmod( v[0] + rotate->x, 360.0 );
	    v[1] = fmod( v[1] + rotate->y, 360.0 );
	    v[2] = fmod( v[2] + rotate->z, 360.0 );
	    v = (float*)( (u8*)v + off );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void TransformRotDouble3D
(
    double	*v,	// pointer to list of 3D vectors
    int		n,	// number of 3D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    DASSERT(v);
    DASSERT(n>0);
    DASSERT( n <= 1 || off >= 3 * sizeof(*v) );

    if ( IsNormValid() && opt_transform.rotate_enabled )
    {
	const double3 *rotate = &opt_transform.norm_rotate_deg;

	while ( n-- > 0 )
	{
	    v[0] = fmod( v[0] + rotate->x, 360.0 );
	    v[1] = fmod( v[1] + rotate->y, 360.0 );
	    v[2] = fmod( v[2] + rotate->z, 360.0 );
	    v = (double*)( (u8*)v + off );
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command matrix			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_matrix()
{
    enumError stat = ERR_OK;
    const uint print_mode = verbose < -1 ? 0 : verbose < 0 ? 1 : 2;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	ScanInfo_t si;
	InitializeSI(&si,param->arg,strlen(param->arg),"TEST",0);
	DEFINE_VAR(val);
	const enumError err = ScanVectorExprSI(&si,&val,0.0,1);
	if (!err)
	    TestMatrix(&val.d3,print_mode,true);
    }

    if ( verbose >= 0 )
	PrintTestMatrix(true);

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

bool TestMatrix
(
    // returns false on error

    double3	*val,		// NULL or value to test
    uint	print_mode,	// 0: silent
				// 1: print vectors on error
				// 2: print vectors always
    bool	print_matrix	// true && if vector shall print:
				//   print matrix before, but only the first time
)
{
    if (!val)
	return true;

    enum { max_mode = 4 };

    uint mode;
    double3 inv, tr[max_mode];
    for ( mode = 0; mode < max_mode; mode++ )
	tr[mode] = TransformD3(mode,val);
    inv = InvTransformD3MatrixD(&opt_transform,tr);

    const double null_epsilon	  = 1e-7;
    const double diff_epsilon     = 1e-6;
    bool inv_ok = IsEqualD3( &inv,  val, null_epsilon, diff_epsilon );

    bool all_ok = inv_ok, tr_ok[max_mode];
    for ( mode = 1; mode < max_mode; mode++ )
    {
	tr_ok[mode] = IsEqualD3(tr,tr+mode,null_epsilon,diff_epsilon);
	if (!tr_ok[mode])
	    all_ok = false;
    }

    if ( print_mode > all_ok )
    {
	if (print_matrix)
	    PrintTestMatrix(true);

	printf(" Base vector:    %11.3f %11.3f %11.3f\n",
		val->x, val->y, val->z );
	if ( verbose > 0 || !inv_ok )
	    printf(" T.form+Inverse: %11.3f %11.3f %11.3f [%d]\n",
		inv.x, inv.y, inv.z, inv_ok );
	printf(" Transformation: %11.3f %11.3f %11.3f\n",
		tr->x, tr->y, tr->z );
	for ( mode = 1; mode < max_mode; mode++ )
	    if ( verbose > 0 || !tr_ok[mode] )
		printf(" Transform[%u]:   %11.3f %11.3f %11.3f [%d]\n",
		    mode, tr[mode].x, tr[mode].y, tr[mode].z, tr_ok[mode] );
	printf("%.57s\n",Minus300);
    }
    return all_ok;
}

///////////////////////////////////////////////////////////////////////////////

bool PrintTestMatrix ( bool print_sep )
{
    static uint seq_num = ~0;
    if ( seq_num == opt_transform.sequence_number )
	return false;
    seq_num = opt_transform.sequence_number;
    putchar('\n');

    static char xyz[8][4] = { "---", "x--", "-y-", "xy-", "--z", "x-z", "-yz", "xyz" };
    static char use[4] = "-us?";

    uint pmode_step, pmode_final;
    if ( brief_count > 1 )
    {
	pmode_step  = 0x08;
	pmode_final = 0x08;
    }
    else if ( brief_count > 0 )
    {
	pmode_step  = 0x08;
	pmode_final = 0x16;
    }
    else if ( long_count > 1 )
    {
	pmode_step  = 0x1f;
	pmode_final = 0x1e;
    }
    else if ( long_count > 0 )
    {
	pmode_step  = 0x17;
	pmode_final = 0x1e;
    }
    else
    {
	pmode_step  = 0x17;
	pmode_final = n_transform_list > 1 ? 0x16 : 0x1f;
    }

    ccp sep;
    static char sep_eq[] = "========================================"
			   "========================================";

    if ( n_transform_list <= 1 )
    {
	pmode_final |= pmode_step;
	sep = Minus300;
    }
    else
    {
	sep = sep_eq;
	uint i;
	for ( i = 0; i < n_transform_list; i++ )
	{
	    MatrixD_t *t = transform_list+i;
	    printf("--------------- transformation matrix %u/%u [%s,%s,%s,%c] ---------------\n",
		i+1, n_transform_list,
		xyz[t->scale_enabled&7],
		xyz[t->rotate_enabled&7],
		xyz[t->translate_enabled&7],
		use[t->use_matrix&3] );
	    PrintMatrixD(stdout,0,0,t,0,pmode_step);
	}
    }

    printf("%.14s Final transformation matrix [%s,%s,%s,%c] %.14s\n",
	sep,
	xyz[opt_transform.scale_enabled&7],
	xyz[opt_transform.rotate_enabled&7],
	xyz[opt_transform.translate_enabled&7],
	use[opt_transform.use_matrix&3],
	sep );
    PrintMatrixD(stdout,0,0,&opt_transform,0,pmode_final);

 #ifdef TEST
    fputs("Vectors calculated by Matrix:\n",stdout);
    MatrixD_t temp = opt_transform;
    CalcVectorsMatrixD(&temp,true);
    PrintMatrixD(stdout,0,0,&temp,0,2);
 #endif

    if (print_sep)
	printf("%.73s\n",sep);

    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


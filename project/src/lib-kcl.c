
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
#include "lib-szs.h"
#include "lib-image.h"
#include "lib-bzip2.h"
#include "kcl.inc"
#include "obj-mtl-bz2.inc"
#include <math.h>
#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    KCL action log		///////////////
///////////////////////////////////////////////////////////////////////////////

bool KCL_ACTION_LOG ( const char * format, ... )
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[KCL]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

    if ( verbose > 2 || KCL_MODE & KCLMD_LOG )
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[KCL]%s ",colset->heading,colset->info);
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
///////////////			 kcl_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool drop_sort_triangles = false;

int enable_kcl_drop_auto = 0;
kcl_mode_t KCL_MODE = KCLMD_M_DEFAULT;

kcl_t * kcl_ref		 = 0;	// NULL or reference KCL for calculations
kcl_t * kcl_ref_prio	 = 0;	// NULL or prioreference KCL for calculations

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_kcl_tab[] =
{
  { 0,			"CLEAR",	"RESET",	KCLMD_M_ALL | KCLMD_F_HIDE },
  { KCLMD_M_DEFAULT,	"DEFAULT",	0,		KCLMD_M_ALL | KCLMD_F_HIDE },

  { KCLMD_FAST,		"FAST",		0,		0 },
  { KCLMD_ROUND,	"ROUND",	0,		0 },
  { KCLMD_NORMALS,	"NORMALS",	0,		0 },
  { KCLMD_MTL,		"MTL",		"MATERIALS",	0 },
  { KCLMD_WIIMM,	"WIIMM",	0,		0 },
  { KCLMD_TRIANGLES,	"TRIANGLES",	0,		0 },
  { KCLMD_OUT_SWAP,	"OUT-SWAP",	"OUTSWAP",	0 },
   { KCLMD_OUT_SWAP,	"OSWAP",	0,		0 },

  { KCLMD_G,		"G",		0,		0 },
  { KCLMD_USEMTL,	"USEMTL",	0,		0 },
  { KCLMD_CLIP,		"CLIP",		0,		0 },
  { KCLMD_IN_SWAP,	"IN-SWAP",	"INSWAP",	0 },
   { KCLMD_IN_SWAP,	"ISWAP",	0,		0 },
  { KCLMD_AUTO,		"AUTO",		0,		0 },

  { KCLMD_CENTER,	"CENTER",	0,		0 },
  { KCLMD_ADD_ROAD,	"ADD-ROAD",	"ADDROAD",	0 },

  { KCLMD_HEX4,		"HEX4",		"H4",		0 },
  { KCLMD_HEX23,	"HEX23",	"H23",		0 },
  { KCLMD_M_HEX,	"HEX",		0,		KCLMD_M_HEX | KCLMD_F_HIDE },
   { 0,			"-HEX",		0,		KCLMD_M_HEX | KCLMD_F_HIDE },

  { KCLMD_CUBE,		"CUBE",		0,		0 },

  { KCLMD_SMALL,	"SMALL",	0,		KCLMD_M_PRESET },
  { KCLMD_CHARY,	"CHARY",	"NINTENDO",	KCLMD_M_PRESET },
  { KCLMD_DRAW,		"DRAW",		0,		KCLMD_M_PRESET },
   { 0,			"-SMALL",	0,		KCLMD_SMALL | KCLMD_F_HIDE },
   { 0,			"-CHARY",	"-NINTENDO",	KCLMD_CHARY | KCLMD_F_HIDE },
   { 0,			"MEDIUM",	0,		KCLMD_M_PRESET | KCLMD_F_HIDE },

  { KCLMD_DROP_AUTO,	"DROP-AUTO",	"DROPAUTO",	KCLMD_M_DROP },
  {  0,			"-DROP-AUTO",	"-DROPAUTO",	KCLMD_DROP_AUTO|KCLMD_F_HIDE },
  { KCLMD_DROP_UNUSED
    |KCLMD_DROP_AUTO,	"DROP-UNUSED",	"DROPUNUSED",	0 },
  { KCLMD_DROP_FIXED
    |KCLMD_DROP_AUTO,	"DROP-FIXED",	"DROPFIXED",	0 },
  { KCLMD_DROP_INVALID
    |KCLMD_DROP_AUTO,	"DROP-INVALID",	"DROPINVALID",	0 },
  { KCLMD_M_DROP,	"DROP",		"DROP-ALL",	KCLMD_M_DROP | KCLMD_F_HIDE },
   { 0,			"-DROP",	"-DROP-ALL",	KCLMD_M_DROP | KCLMD_F_HIDE },

  { KCLMD_RM_FACEDOWN,	"RM-FACEDOWN",	"RMFACEDOWN",	0 },
  { KCLMD_RM_FACEUP,	"RM-FACEUP",	"RMFACEUP",	0 },
  { KCLMD_M_FIX_ALL,	"FIX-ALL",	"FIXALL",	KCLMD_M_FIX_ALL | KCLMD_F_HIDE },

  { KCLMD_CONV_FACEUP,	"CONV-FACEUP",	"CONVFACEUP",	0 },
  { KCLMD_WEAK_WALLS,	"WEAK-WALLS",	"WEAKWALLS",	0 },
  { KCLMD_CLR_VISUAL,	"CLR-VISUAL",	"CLRVISUAL",	0 },

  { KCLMD_NEW,		"NEW",		0,		0 },
  { KCLMD_SORT,		"SORT",		0,		0 },

  { KCLMD_TINY_0,	"TINY-0",	"TINY0",	KCLMD_M_TINY },
  { KCLMD_TINY_1,	"TINY-1",	"TINY1",	KCLMD_M_TINY },
  { KCLMD_TINY_2,	"TINY-2",	"TINY2",	KCLMD_M_TINY },
  { KCLMD_TINY_3,	"TINY-3",	"TINY3",	KCLMD_M_TINY },
  { KCLMD_TINY_4,	"TINY-4",	"TINY4",	KCLMD_M_TINY },
  { KCLMD_TINY_5,	"TINY-5",	"TINY5",	KCLMD_M_TINY },
  { KCLMD_TINY_6,	"TINY-6",	"TINY6",	KCLMD_M_TINY },
  { KCLMD_TINY_7,	"TINY-7",	"TINY7",	KCLMD_M_TINY },

  { KCLMD_LOG,		"LOG",		0,		0 },
  { KCLMD_SILENT,	"SILENT",	0,		0 },
  { KCLMD_POSLEN,	"POSLEN",	0,		0 },
  { KCLMD_INPLACE,	"INPLACE",	0,		0 },

  { KCLMD_F_SCRIPT,	"SCRIPT",	0,		0 },
  { KCLMD_TEST,		"TEST",		"T",		0 },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

void SetKclMode ( kcl_mode_t new_mode )
{
    PRINT("KCL-MODE-IN:  PC=%d, KPC=%d, DS=%d\n",
		have_patch_count, have_kcl_patch_count, drop_sort_triangles );

    // disable DROP_AUTO mode if any other DROP is set
    if ( (new_mode & KCLMD_M_DROP ) != KCLMD_DROP_AUTO )
	new_mode &= ~KCLMD_DROP_AUTO;

    if (drop_sort_triangles)
	have_patch_count--, have_kcl_patch_count--;

    KCL_MODE = new_mode & KCLMD_M_ALL;

    const kcl_mode_t tiny = KCL_MODE & KCLMD_M_TINY;
    if ( tiny >= KCLMD_TINY_1 )
    {
	KCL_MODE = KCL_MODE & ~KCLMD_M_PRESET | (KCLMD_SMALL|KCLMD_ROUND|KCLMD_SORT);
	if ( tiny >= KCLMD_TINY_2 )
	    KCL_MODE |= KCLMD_NEW;
    }

    if ( PATCH_FILE_MODE & PFILE_F_LOG_ALL )
	KCL_MODE |= KCLMD_LOG;

    drop_sort_triangles = ( KCL_MODE & KCLMD_M_PATCH ) != 0;
    if (drop_sort_triangles)
	have_patch_count++, have_kcl_patch_count++;

    PRINT("KCL-MODE-OUT: PC=%d, KPC=%d, DS=%d\n",
		have_patch_count, have_kcl_patch_count, drop_sort_triangles );
}

//-----------------------------------------------------------------------------

int ScanOptKcl ( ccp arg )
{
    if (!arg)
	return 0;

    s64 stat = ScanKeywordList(arg,opt_kcl_tab,0,true,0,KCL_MODE,
				"Option --kcl",ERR_SYNTAX);
    if ( stat != -1 )
    {
	SetKclMode(stat);
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

uint PrintKclMode ( char *buf, uint bufsize, kcl_mode_t mode )
{
    DASSERT(buf);
    DASSERT(bufsize>10);
    char *dest = buf;
    char *end = buf + bufsize - 1;

    mode = mode & KCLMD_M_ALL | KCLMD_F_HIDE;
    kcl_mode_t mode1 = mode;

    const KeywordTab_t *ct;
    for ( ct = opt_kcl_tab; ct->name1 && dest < end; ct++ )
    {
	if ( ct->opt & KCLMD_F_HIDE )
	    continue;

	if ( ct->opt ? (mode & ct->opt) == ct->id : mode & ct->id )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = StringCopyE(dest,end,ct->name1);
	    mode &= ~(ct->id|ct->opt);
	}
    }

    if ( mode1 == (KCLMD_M_DEFAULT|KCLMD_F_HIDE) )
	dest = StringCopyE(dest,end," (default)");
    else if (!mode1)
	dest = StringCopyE(dest,end,"(none)");

    *dest = 0;
    return dest-buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetKclMode()
{
    static char buf[200] = {0};
    if (!*buf)
	PrintKclMode(buf,sizeof(buf),KCL_MODE);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

void SetupKCL()
{
    if ( opt_tiny > 0 )
    {
	const kcl_mode_t tiny
		= ( (kcl_mode_t)opt_tiny << KCLMD_S_TINY ) & KCLMD_M_TINY;
	if ( tiny > ( KCL_MODE & KCLMD_M_TINY ))
	    SetKclMode( KCL_MODE & ~KCLMD_M_TINY | tiny );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum CommandKCL_t
{
    KCMD_NONE		= 0,	// definitiv NULL

    KCMD_V,
    KCMD_VN,
    KCMD_F,
    KCMD_G,
    KCMD_USEMTL,
    KCMD_TRI,

} CommandKCL_t;

///////////////////////////////////////////////////////////////////////////////

CommandKCL_t GetCommandKCL
(
    ccp			name		// name to scan, upper case is assumed
)
{
    static FormatField_t cmdtab = {0};
    if (!cmdtab.used)
    {
	struct kcmdtab_t { ccp name; int id; };
	static const struct kcmdtab_t def_tab[] =
	{
	    { "V",		KCMD_V },
	    { "VN",		KCMD_VN },
	    { "F",		KCMD_F },
	    { "G",		KCMD_G },
	    { "USEMTL",		KCMD_USEMTL },
	    { "TRI",		KCMD_TRI },

	    {0,0}
	};

	const struct kcmdtab_t * cp;
	for ( cp = def_tab; cp->name; cp++ )
	    InsertFormatField(&cmdtab,cp->name,0,0,0)->num = cp->id;
    }

    FormatFieldItem_t * item = FindFormatField(&cmdtab,name);
    return item ? item->num : KCMD_NONE;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			const parameters		///////////////
///////////////////////////////////////////////////////////////////////////////

// create octree params
uint	KCL_BITS	=   0;		//   0 = generic
uint	KCL_BLOW	= 400;
uint	KCL_MAX_DEPTH	=  10;
uint	KCL_MAX_TRI	=  30;
uint	KCL_MIN_SIZE	= 512;
uint	KCL_MAX_SIZE	= 0x100000;
double3	KCL_MIN;			// -inf
double3	KCL_MAX;			// +inf
uint	KCL_TRI_SPLIT	= 1024;		// >0: split triangles (recures) if side length >#

// export obj param
static double3	KCL_CLIP;	// = 1.0

///////////////////////////////////////////////////////////////////////////////

static bool DefineVector ( double3 *val, ccp name )
{
    bool stat = false;

    const Var_t *var = FindConst(name);
    if ( var && var->mode != VAR_UNSET )
    {
	stat = true;
	val->x = GetXDoubleV(var);
	val->y = GetYDoubleV(var);
	val->z = GetZDoubleV(var);
    }

    char varname[100];
    snprintf(varname,sizeof(varname),"%s.X",name);
    var = FindConst(varname);
    if ( var && var->mode != VAR_UNSET )
    {
	stat = true;
	val->x = GetXDoubleV(var);
    }

    snprintf(varname,sizeof(varname),"%s.Y",name);
    var = FindConst(varname);
    if ( var && var->mode != VAR_UNSET )
    {
	stat = true;
	val->y = GetYDoubleV(var);
    }

    snprintf(varname,sizeof(varname),"%s.Z",name);
    var = FindConst(varname);
    if ( var && var->mode != VAR_UNSET )
    {
	stat = true;
	val->z = GetZDoubleV(var);
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

void LoadParametersKCL
(
    ccp		log_prefix		// not NULL:
					//    print log message with prefix
)
{
    static bool done = false;
    if (done)
	return;
    done = true;


    //--- --kcl small,chary,draw ==> change defaults

    SetKclMode(KCL_MODE);
    kcl_mode_t preset = KCL_MODE & KCLMD_M_PRESET;

    if ( preset == KCLMD_DRAW )
    {
	KCL_MODE	= KCL_MODE | KCLMD_FAST | KCLMD_ROUND;
	KCL_BLOW	= 200;
	KCL_MAX_TRI	=  50;
    }
    else if ( preset == KCLMD_SMALL )
    {
	KCL_MODE	= KCL_MODE & ~KCLMD_FAST | KCLMD_ROUND;
	KCL_BLOW	= 200;
	KCL_MAX_TRI	=  40;
    }
    else if ( preset == KCLMD_CHARY )
    {
	KCL_BLOW	= 600;
	KCL_MAX_TRI	=  20;
    }


    //--- KCL_BITS

    const Var_t *var = FindConst("KCL_BITS");
    if (var)
    {
	const uint num = GetIntV(var);
	if ( num >= 0 )
	    KCL_BITS = num < 3 ? 3 : num > 20 ? 20 : num;
    }


    //--- KCL_BLOW

    var = FindConst("KCL_BLOW");
    if (var)
    {
	const uint num = GetIntV(var);
	if ( num >= 0 )
	    KCL_BLOW = num < 10000 ? num : 10000;
    }


    //--- KCL_CLIP

    KCL_CLIP.x = KCL_CLIP.y = KCL_CLIP.z = 1.0;
    DefineVector(&KCL_CLIP,"KCL_CLIP");
    if ( KCL_CLIP.x <= 0.0 ) KCL_CLIP.x = 1.0;
    if ( KCL_CLIP.y <= 0.0 ) KCL_CLIP.y = 1.0;
    if ( KCL_CLIP.z <= 0.0 ) KCL_CLIP.z = 1.0;


    //--- KCL_MAX_DEPTH

    var = FindConst("KCL_MAX_DEPTH");
    if (var)
    {
	const uint num = GetIntV(var);
	if ( num > 0 )
	    KCL_MAX_DEPTH = num;
    }


    //--- KCL_MAX_TRI

    var = FindConst("KCL_MAX_TRI");
    if (var)
    {
	const uint num = GetIntV(var);
	if ( num > 0 )
	    KCL_MAX_TRI = num < 5 ? 5 : num > 1000 ? 1000 : num;
    }


    //--- KCL_MAX_SIZE, KCL_MIN_SIZE

    var = FindConst("KCL_MAX_SIZE");
    if (var)
    {
	const uint num = GetIntV(var);
	if ( num > 0 )
	    KCL_MAX_SIZE = num < 0x100 ? 0x100 : num > 0x100000 ? 0x100000 : num;
    }

    var = FindConst("KCL_MIN_SIZE");
    if (var)
    {
	const uint num = GetIntV(var);
	if ( num > 0 )
	    KCL_MIN_SIZE = num < KCL_MAX_SIZE ? num : KCL_MAX_SIZE;
    }


    //--- KCL_MIN, KCL_MAX

    KCL_MIN.x = KCL_MIN.y = KCL_MIN.z = -INFINITY;
    DefineVector(&KCL_MIN,"KCL_MIN");

    KCL_MAX.x = KCL_MAX.y = KCL_MAX.z = INFINITY;
    DefineVector(&KCL_MAX,"KCL_MAX");

    if ( KCL_MAX.x < KCL_MIN.x ) KCL_MAX.x = KCL_MIN.x;
    if ( KCL_MAX.y < KCL_MIN.y ) KCL_MAX.y = KCL_MIN.y;
    if ( KCL_MAX.z < KCL_MIN.z ) KCL_MAX.z = KCL_MIN.z;


    //--- KCL_TRI_SPLIT,

    var = FindConst("KCL_TRI_SPLIT");
    if (var)
    {
	KCL_TRI_SPLIT = GetIntV(var);
	if ( KCL_TRI_SPLIT && KCL_TRI_SPLIT < 512 )
	    KCL_TRI_SPLIT = 512;
    }


    //--- logging

    if ( !KCL_ACTION_LOG("Global KCL Modes: %s\n",GetKclMode())
	&& log_prefix
	&& KCL_MODE != KCLMD_M_DEFAULT
	&& !(KCL_MODE & KCLMD_SILENT) )
    {
	fprintf(stdlog,"%sglobal kcl modes: %s\n",log_prefix,GetKclMode());
	fflush(stdlog);
    }

  #if HAVE_PRINT
    PRINT("KCL-PARAM: MODE= [%llx] %s\n",(u64)KCL_MODE,GetKclMode());

    PRINT("  > BITS=%u, MAX_TRI=%u, MAX_DEPTH=%u, CUBE_SIZE=%u..%u\n",
		KCL_BITS, KCL_MAX_TRI, KCL_MAX_DEPTH, KCL_MIN_SIZE, KCL_MAX_SIZE );
    PRINT("  > CLIP: %11.3f %11.3f %11.3f, enabled=%d \n",
		KCL_CLIP.x, KCL_CLIP.y, KCL_CLIP.z, (KCL_MODE&KCLMD_CLIP)!=0 );
    PRINT("  > MIN:  %11.3f %11.3f %11.3f\n",
		KCL_MIN.x, KCL_MIN.y, KCL_MIN.z );
    PRINT("  > MAX:  %11.3f %11.3f %11.3f\n",
		KCL_MAX.x, KCL_MAX.y, KCL_MAX.z );

    if ( KCL_MODE & KCLMD_CUBE )
	PRINT("  > Use CUBE mode (experimental), split triangles @ %u\n",
		KCL_TRI_SPLIT );
  #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    KCL setup			///////////////
///////////////////////////////////////////////////////////////////////////////

Color_t user_color[N_KCL_USER_FLAGS];
uint n_user_color = 0;

///////////////////////////////////////////////////////////////////////////////

kcl_flag_t * CreatePatchFlagKCL
(
    // returns NULL or N_KCL_FLAG elements

    kcl_mode_t kcl_mode,    // relevant: KCLMD_CLR_VISUAL | KCLMD_WEAK_WALLS
    SlotMode_t slot_mode,   // relevant: SLOTMD_ICE_TO_WATER | SLOTMD_WATER_TO_ICE
    bool null_on_orig	    // TRUE: return NULL on no flag modifications
)
{
    kcl_flag_t *flag = MALLOC( N_KCL_FLAG * sizeof(*flag) );
    for ( int i = 0; i < N_KCL_FLAG; i++ )
	flag[i] = i;

    if ( opt_slot & (SLOTMD_ICE_TO_WATER|SLOTMD_WATER_TO_ICE) )
    {
	const u16 search = opt_slot & SLOTMD_ICE_TO_WATER ? 0x0070 : 0x0030;
	for ( int i = 0; i < N_KCL_FLAG; i++ )
	    if ( ( flag[i] & 0x00ff ) == search )
		flag[i] = flag[i] ^ 0x0040;
    }

    if ( kcl_mode & KCLMD_WEAK_WALLS )
    {
	for ( int i = 0; i < N_KCL_FLAG; i++ )
	{
	    const int type = GET_KCL_TYPE(i);
	    if ( type >= 0 && kcl_type[type].attrib & KCLT_C_WALL )
		flag[i] |= 0x8000;
	}
    }

    if ( kcl_mode & KCLMD_CLR_VISUAL )
    {
	static u16 tab[32] =
	{
	    0xe01f,0xe01f,0xe01f,0xe01f, 0xe01f,0xe01f,0xe0ff,0xe0ff,
	    0xe0ff,0xe0ff,0xe01f,0xe0ff, 0x601f,0x60ff,0x60ff,0x601f,
	    0xe01f,0xe0ff,0xffff,0xe0ff, 0xe01f,0xe0ff,0xe01f,0xe01f,
	    0x001f,0xffff,0xe01f,0xffff, 0xe0ff,0xe0ff,0x601f,0x60ff,
	};

	for ( int i = 0; i < N_KCL_FLAG; i++ )
	    flag[i] &= tab[i&31];
    }

    if (null_on_orig)
    {
	bool have_patch = false;
	for ( int i = 0; i < N_KCL_FLAG; i++ )
	    if ( flag[i] != i )
	    {
		have_patch = true;
		break;
	    }
	if (!have_patch)
	{
	    FREE(flag);
	    flag = 0;
	}
    }

    return flag;
}

///////////////////////////////////////////////////////////////////////////////

void InitializeKCL ( kcl_t * kcl )
{
    //--- one time setup

    static int done = 0;
    if (!done++)
    {
	SetupPatchKclFlag();
	kcl_flag_t *flag = CreatePatchFlagKCL(KCL_MODE,opt_slot,false);
	DASSERT(flag);
	memcpy(patch_kcl_flag,flag,sizeof(*patch_kcl_flag)*N_KCL_FLAG);
	FREE(flag);
	PurgePatchKclFlag();
    }


    //--- object setup

    DASSERT(kcl);
    memset(kcl,0,sizeof(*kcl));

    kcl->fname = EmptyString;
    kcl->revision = REVISION_NUM;

    InitializeList(&kcl->tridata,sizeof(kcl_tridata_t));
    InitializeFormatField(&kcl->flag_name);
    InitializeFormatField(&kcl->flag_pattern);
    InitializeFormatField(&kcl->flag_missing);

    // default header values
    kcl->unknown_0x10 = 300.0;
    kcl->unknown_0x38 = 250.0;

    kcl->tri_minval.x = -1e9;
    kcl->tri_minval.y = -1e9;
    kcl->tri_minval.z = -1e9;

    kcl->tri_maxval.x =  1e9;
    kcl->tri_maxval.y =  1e9;
    kcl->tri_maxval.z =  1e9;

    LoadParametersKCL( verbose > 0 ? "  - " : 0 );

    kcl->fast			= ( KCL_MODE & KCLMD_FAST  ) != 0;
    kcl->accept_hex4		= ( KCL_MODE & KCLMD_HEX4  ) != 0;
    kcl->accept_hex23		= ( KCL_MODE & KCLMD_HEX23 ) != 0;

    kcl->min_cube_size		= KCL_MIN_SIZE;
    kcl->max_cube_size		= KCL_MAX_SIZE;
    kcl->cube_blow		= KCL_BLOW;
    kcl->max_cube_triangles	= KCL_MAX_TRI;
    kcl->max_octree_depth	= KCL_MAX_DEPTH;
}

///////////////////////////////////////////////////////////////////////////////

void ResetKCL ( kcl_t * kcl )
{
    DASSERT(kcl);

    const file_format_t fform_outfile = kcl->fform_outfile;

    ResetStatisticsKCL(kcl);
    FreeString(kcl->fname);
    FreeString(kcl->flag_fname);
    if (kcl->octree_alloced)
	FREE(kcl->octree);
    if (kcl->raw_data_alloced)
	FREE(kcl->raw_data);
    ResetList(&kcl->tridata);
    ResetFormatField(&kcl->flag_name);
    ResetFormatField(&kcl->flag_pattern);
    ResetFormatField(&kcl->flag_missing);

    InitializeKCL(kcl);
    kcl->fform_outfile = fform_outfile;
}

///////////////////////////////////////////////////////////////////////////////

void ClearKCL ( kcl_t * kcl, bool init_kcl )
{
    DASSERT(kcl);
    if (init_kcl)
	InitializeKCL(kcl);
    else
    {
	ccp fname = kcl->fname;
	kcl->fname = 0;
	ResetKCL(kcl);
	kcl->fname = fname;
    }
}

///////////////////////////////////////////////////////////////////////////////

void CopyKCL ( kcl_t * kcl, bool init_kcl, const kcl_t * src )
{
    DASSERT(kcl);
    DASSERT(src);

    if ( !kcl || kcl == src )
	return;

    ClearKCL(kcl,init_kcl);
    if (!src)
	return;

    const uint n_tri = src->tridata.used;
    kcl_tridata_t *td = (kcl_tridata_t*)src->tridata.list;
    uint i;
    for ( i = 0; i < n_tri; i++, td++ )
	AppendTriangleKCLp( kcl, td->cur_flag, td->pt, td->pt+1, td->pt+2 );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl_tridata_t helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

void CopyTriData_F2D ( kcl_tridata_dbl_t *dest, const kcl_tridata_flt_t *src )
{
    DASSERT(src);
    DASSERT(dest);
    memcpy(dest->pt,src->pt,sizeof(dest->pt));

    const int n = SIZEOFRANGE(kcl_tridata_dbl_t,length,normal)/sizeof(double);
    const float *s = &src->length;
    double *d = &dest->length;
    int i;
    for ( i = 0; i < n; i++ )
	*d++ = *s++;
}

//-----------------------------------------------------------------------------

void CopyTriData_D2F ( kcl_tridata_flt_t *dest, const kcl_tridata_dbl_t *src )
{
    DASSERT(src);
    DASSERT(dest);
    memcpy(dest->pt,src->pt,sizeof(dest->pt));

    const int n = SIZEOFRANGE(kcl_tridata_dbl_t,length,normal)/sizeof(double);
    const double *s = &src->length;
    float *d = &dest->length;
    int i;
    for ( i = 0; i < n; i++ )
	*d++ = *s++;
}

///////////////////////////////////////////////////////////////////////////////
// [[xtridata]]

uint CheckMetricsTriData
(
    // check length and minimal height of triangles

    kcl_tridata_t	*td,		// valid pointer to first record
    uint		n,		// number of records to process
    kcl_t		*kcl		// not NULL: store statistics
)
{
    DASSERT( td || !n );
 #if HAVE_PRINT
    const u_usec_t start_time = GetTimerUSec();
 #endif

    uint invalid_count = 0;
    kcl_tridata_t *end = td + n;
    for ( ; td < end; td++ )
    {
	if ( td->status & TD_INVALID )
	{
	    invalid_count++;
	    continue;
	}

	if ( !( td->status & TD_INVALID_NORM ) &&
		( !isnormal(td->length) || fabs(td->length) < opt_tri_height ))
	{
	    PRINT0("CHECK-TRI/INVALID LENGTH %g\n",td->length);
	    td->status |= TD_INVALID|TD_INVALID_NORM;
	    invalid_count++;
	    continue;
	}

	if ( opt_tri_area > 0 || opt_tri_height > 0 )
	{
	    tri_metrics_t met;
	    CalcTriMetricsByPoints(&met,td->pt,td->pt+1,td->pt+2);
	    if ( met.area < opt_tri_area || met.min_ht < opt_tri_height )
	    {
		#if HAVE_PRINT
		{
		    ccp sep = " ";
		    char hbuf[30], abuf[30];
		    if ( met.min_ht < opt_tri_height )
		    {
			snprintf(hbuf,sizeof(hbuf)," HEIGHT %g",met.min_ht);
			sep = ", ";
		    }
		    else
			*hbuf = 0;

		    if ( met.area < opt_tri_area )
			snprintf(abuf,sizeof(abuf),"%sAREA %g",sep,met.area);
		    else
			*abuf = 0;

		    PRINT0("CHECK-TRI/INVALID%s%s\n",hbuf,abuf);
		}
		#endif

		td->status |= TD_INVALID;
		invalid_count++;
		continue;
	    }
	}
    }

 #if HAVE_PRINT
    const u_usec_t duration = GetTimerUSec() - start_time;
 #endif

    if (kcl)
    {
	PRINT("CheckMetricsTriData(%u), invalid: %u -> %u, done in %s\n",
		n, kcl->tri_inv_count, invalid_count,
		PrintTimerUSec(0,0,duration,6) );
	kcl->tri_inv_count = invalid_count;
    }
    else
	PRINT("CheckMetricsTriData(%u), invalid: %u, done in %s\n",
		n, invalid_count, PrintTimerUSec(0,0,duration,6) );
    return invalid_count;
}

///////////////////////////////////////////////////////////////////////////////
// [[xtridata]]

uint CalcPointsTriData
(
    // calculate 'pt' using 'length' & 'normal'
    // returns the number of fixes

    kcl_tridata_t	*td,		// valid pointer to first record
    uint		n,		// number of records to process
    kcl_t		*kcl,		// not NULL: store statistics
    bool		use_minmax	// true & 'kcl': use 'tri_minval' and 'tri_maxval'
)
{
    DASSERT( td || !n );

    if (!kcl)
	use_minmax = false;

    const double min_divisor = 1e-9;
    uint fix_count = 0, invalid_count = 0;

    for ( ; n-- > 0; td++ )
    {
	if (  td->status & TD_INVALID
	   || !isnormal(td->length)
	   || fabs(td->length) < opt_tri_height
	   )
	{
	  invalid:
	    PRINT0("TRI INVALID %g\n",td->length);
	    td->length = 0.0;
//	    memcpy(td->pt+1,td->pt,sizeof(*td->pt));
//	    memcpy(td->pt+2,td->pt,sizeof(*td->pt));
	    td->status |= TD_INVALID|TD_INVALID_NORM;
	    invalid_count++;
	    continue;
	}

	double3 cross_1, cross_2;
	double dot_1, dot_2;

	CrossProd3(cross_1,td->normal[1],td->normal[0]);
	dot_1 = DotProd3(cross_1,td->normal[3]);
	if (!isfinite(dot_1))
	    goto invalid;

	if ( fabs(dot_1) >= min_divisor )
	{
	    const double factor_1 = td->length / dot_1;
	    double3 *d3 = td->pt + 2;
	    d3->x = td->pt[0].x + cross_1.x * factor_1;
	    d3->y = td->pt[0].y + cross_1.y * factor_1;
	    d3->z = td->pt[0].z + cross_1.z * factor_1;

	    if ( use_minmax &&
		(  d3->x < kcl->tri_minval.x || d3->x > kcl->tri_maxval.x
		|| d3->y < kcl->tri_minval.y || d3->y > kcl->tri_maxval.y
		|| d3->z < kcl->tri_minval.z || d3->z > kcl->tri_maxval.z ))
	    {
		TRACE("VERTEX-1:   %12.6g %12.6g %12.6g\n",
			d3->x, d3->y, d3->z );
		goto abort_1;
	    }
	}
	else
	{
	  abort_1:
	    noPRINT("INVALID DOT_1\n");
	    memcpy(td->pt+2,td->pt,sizeof(*td->pt));
	    td->status |= TD_FIXED_PT|TD_INVALID_NORM;
	}

	CrossProd3(cross_2,td->normal[2],td->normal[0]);
	dot_2 = DotProd3(cross_2,td->normal[3]);
	if (!isfinite(dot_2))
	    goto invalid;

	if ( fabs(dot_2) >= min_divisor )
	{
	    const double factor_2 = td->length / dot_2;
	    double3 *d3 = td->pt + 1;
	    d3->x = td->pt[0].x + cross_2.x * factor_2;
	    d3->y = td->pt[0].y + cross_2.y * factor_2;
	    d3->z = td->pt[0].z + cross_2.z * factor_2;

	    if ( use_minmax &&
		(  d3->x < kcl->tri_minval.x || d3->x > kcl->tri_maxval.x
		|| d3->y < kcl->tri_minval.y || d3->y > kcl->tri_maxval.y
		|| d3->z < kcl->tri_minval.z || d3->z > kcl->tri_maxval.z ))
	    {
		TRACE("VERTEX-2:   %12.6g %12.6g %12.6g\n",
			d3->x, d3->y, d3->z );
		goto abort_2;
	    }
	}
	else
	{
	  abort_2:
	    noPRINT("INVALID DOT_2\n");
	    memcpy(td->pt+1,td->pt,sizeof(*td->pt));
	    td->status |= TD_FIXED_PT|TD_INVALID_NORM;
	}

	if ( td->status & TD_FIXED_PT )
	    fix_count++;
    }

    if (kcl)
    {
	kcl->tri_inv_count = invalid_count;
	kcl->tri_fix_count = fix_count;
    }

    return invalid_count + fix_count;
}

///////////////////////////////////////////////////////////////////////////////
// [[xtridata]]

void CalcNormalsTriData
(
    // calculate 'length' & 'normal' using 'pt'
    // the calculation is only done, if 'TD_INVALID_NORM'

    kcl_t		*kcl,		// valid KCL
    kcl_tridata_t	*td,		// valid pointer to first record
    uint		n		// number of records to process
)
{
    DASSERT( td || !n );

    #undef XPRINT
    #define XPRINT noPRINT

    const bool pos_length = ( KCL_MODE & KCLMD_POSLEN ) != 0;

    for ( ; n-- > 0; td++ )
    {
	if ( !( td->status & TD_INVALID_NORM ) || td->status & TD_REMOVED )
	    continue;

	XPRINT("-----\n");
	XPRINT("PT0:  %6.2f %6.2f %6.2f\n", td->pt[0].x, td->pt[0].y, td->pt[0].z );
	XPRINT("PT1:  %6.2f %6.2f %6.2f\n", td->pt[1].x, td->pt[1].y, td->pt[1].z );
	XPRINT("PT2:  %6.2f %6.2f %6.2f\n", td->pt[2].x, td->pt[2].y, td->pt[2].z );

	double3 diff_2_0, diff_1_0, diff_1_2, normal[4];

	Sub3(diff_2_0,td->pt[2],td->pt[0]);
	Sub3(diff_1_0,td->pt[1],td->pt[0]);
	Sub3(diff_1_2,td->pt[1],td->pt[2]);
	XPRINT("D-10: %6.2f %6.2f %6.2f\n", diff_1_0.x, diff_1_0.y, diff_1_0.z );
	XPRINT("D-20: %6.2f %6.2f %6.2f\n", diff_2_0.x, diff_2_0.y, diff_2_0.z );
	XPRINT("D-12: %6.2f %6.2f %6.2f\n", diff_1_2.x, diff_1_2.y, diff_1_2.z );

	CrossProd3( normal[0], diff_1_0,  diff_2_0  ); Unit3(normal[0]);
	CrossProd3( normal[1], normal[0], diff_2_0  ); Unit3(normal[1]);
	CrossProd3( normal[2], diff_1_0,  normal[0] ); Unit3(normal[2]);
	CrossProd3( normal[3], normal[0], diff_1_2  ); Unit3(normal[3]);

	double length = DotProd3(diff_1_0,normal[3]);
	if ( pos_length && length < 0.0 )
	{
	    length = -length;
	    Neg3(normal[3]);
	}

	XPRINT("A:    %6.2f %6.2f %6.2f\n",normal[1].x,normal[1].y,normal[1].z);
	XPRINT("B:    %6.2f %6.2f %6.2f\n",normal[2].x,normal[2].y,normal[2].z);
	XPRINT("C:    %6.2f %6.2f %6.2f\n",normal[3].x,normal[3].y,normal[3].z);
	XPRINT("LEN:  %6.2f\n",length);

	//--- now we assign the double values to the float members

	// need cast to avoid gcc warning 'above array bounds'
	double *d = (double*)&normal->x;
	float  *f = td->normal->v;
	float  *f_end = (float*)( (u8*)f + sizeof(td->normal) );

	if ( KCL_MODE & KCLMD_M_TINY )
	{
	    while ( f < f_end )
		*f++ = ldexp(round(ldexp(*d++,15)),-15);
	}
	else if ( KCL_MODE & KCLMD_ROUND )
	{
	    while ( f < f_end )
		*f++ = ldexp(round(ldexp(*d++,17)),-17);
	}
	else
	    while ( f < f_end )
		*f++ = *d++;

	td->length  = length;
	td->status &= ~TD_INVALID_NORM;

	if (   !isnormal(td->length)
	    || fabs(td->length) < opt_tri_height
	    || !isfinite(normal[0].x)
	    || !isfinite(normal[0].y)
	    || !isfinite(normal[0].z)
	    || !isfinite(normal[1].x)
	    || !isfinite(normal[1].y)
	    || !isfinite(normal[1].z)
	    || !isfinite(normal[2].x)
	    || !isfinite(normal[2].y)
	    || !isfinite(normal[2].z)
	    || !isfinite(normal[3].x)
	    || !isfinite(normal[3].y)
	    || !isfinite(normal[3].z)
	    )
	{
	    if (kcl)
	    {
		PRINT("INVALID TRIANGLE #%u! [len=%5.3g]\n",
		    (int)( td - (kcl_tridata_t*)kcl->tridata.list ), td->length );
		if (!(td->status & TD_INVALID))
		    kcl->tri_inv_count++;
	    }
	    td->status |= TD_INVALID;
	    td->length  = 0.0;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void CalcNormalsKCL
(
    // calculate 'length' & 'normal' using 'pt'
    // the calculation is only done, if '!kcl->norm_valid'

    kcl_t		*kcl,		// valid KCL
    bool		force		// true: ignore 'kcl->norm_valid'
)
{
    DASSERT(kcl);

    if ( !kcl->norm_valid || force )
    {
	CalcNormalsTriData( kcl, (kcl_tridata_t*)kcl->tridata.list,
				kcl->tridata.used );
	kcl->norm_valid = true;

 #ifdef TEST0
	// verify valid triangels
	PRINT(" verify valid triangels\n");
	CalcPointsTriData( (kcl_tridata_t*)kcl->tridata.list,
				kcl->tridata.used, 0,false );
 #endif
    }
}

///////////////////////////////////////////////////////////////////////////////
// [[xtridata]]

void RoundPointsKCL
(
    // round the normal values

    kcl_t		*kcl,		// valid KCL
    int			round_pow2	// round to '1/(2^round_pow2)'
)
{
    DASSERT(kcl);

    uint n = kcl->tridata.used;
    kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;

    for ( ; n-- > 0; td++ )
    {
	double *d = td->pt->v;
	uint i;
	for ( i = 0; i < sizeof(td->pt->v)/sizeof(*d); i++, d++ )
	    *d = ldexp(round(ldexp(*d,round_pow2)),-round_pow2);
	td->status |= TD_INVALID_NORM;
    }

    CalcNormalsKCL(kcl,true);
}

///////////////////////////////////////////////////////////////////////////////

void RoundNormalsKCL
(
    // round the normal values

    kcl_t		*kcl,		// valid KCL
    int			round_pow2	// round to '1/(2^round_pow2)'
)
{
    DASSERT(kcl);

    uint n = kcl->tridata.used;
    kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;

    if (!kcl->norm_valid)
    {
	CalcNormalsTriData(kcl,td,n);
	kcl->norm_valid = true;
    }

    for ( ; n-- > 0; td++ )
    {
	float *f = td->normal->v;
	uint i;
	for ( i = 0; i < sizeof(td->normal->v)/sizeof(*f); i++, f++ )
	    *f = ldexpf(roundf(ldexpf(*f,round_pow2)),-round_pow2);
    }
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *GetTridataKCL
(
    kcl_t		*kcl,		// NULL or valid kcl
    uint		tri_index	// triangle index
)
{
    return kcl && tri_index < kcl->tridata.used
		? (kcl_tridata_t*)kcl->tridata.list + tri_index
		: 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			append triangles		///////////////
///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t * PrepareAppendTrianglesKCL
(
    // Grows the triangle field and the 'used' count.
    // Returns a pointer to the first triangle.
    // If to many triangles:
    //   => a warning is printed, but only once for a kcl. NULL is returned

    kcl_t		*kcl,		// valid KCL data structure
    uint		n_append	// number of triangles to append
)
{
    DASSERT(kcl);
    if ( kcl->no_limit || n_append + kcl->tridata.used <= KCL_MAX_TRIANGLES )
    {
	kcl_tridata_t *td = GrowListSize(&kcl->tridata,n_append,1000);
	DASSERT(td);
	memset(td,0,n_append*sizeof(*td));
	return td;
    }

    if (!kcl->limit_warning)
    {
	kcl->limit_warning = true;
	ERROR0(ERR_SEMANTIC,
		"Can't add new triangles, because limit (%u) reached!",
		KCL_MAX_TRIANGLES );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendTriangleKCLp
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3		// pointer to point #3
)
{
    DASSERT(kcl);
    DASSERT(pt1);
    DASSERT(pt2);
    DASSERT(pt3);

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);
    return AppendTriangleKCL(&par,pt1,pt2,pt3);
}

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendTriangleKCL
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_tri_param_t	*par,		// valid data structure
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3		// pointer to point #3
)
{
    DASSERT(par);
    DASSERT(par->kcl);
    DASSERT(pt1);
    DASSERT(pt2);
    DASSERT(pt3);

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(par->kcl,1);
    if (td)
    {
	memcpy(td->pt+0,pt1,sizeof(*td->pt));
	memcpy(td->pt+1,pt2,sizeof(*td->pt));
	memcpy(td->pt+2,pt3,sizeof(*td->pt));
	td->in_flag = td->cur_flag = par->kcl_flag;
	td->status  = TD_ADDED|TD_INVALID_NORM;
	par->kcl->model_modified = true;

	#if SUPPORT_KCL_DELTA
	{
	    float d1 = LengthD(pt1->v,pt2->v);
	    float d2 = LengthD(pt2->v,pt3->v);
	    float d3 = LengthD(pt3->v,pt1->v);
	    float dmin, dmax;
	    if ( d1 < d2 )
	    {
		dmin = d1;
		dmax = d2;
	    }
	    else
	    {
		dmin = d2;
		dmax = d1;
	    }
	    td->delta_min = dmin < d3 ? dmin : d3;
	    td->delta_max = dmax > d3 ? dmax : d3;
	}
	#endif

	if (par->norm)
	{
	    const double norm_len = Length3(*par->norm);
	    if ( norm_len > 1e-3 )
	    {
		// first calc the normal of the direction

		double3 norm_unit;
		Div3f(norm_unit,*par->norm,norm_len);
		PRINT("NORM/DIR: %8.5f %8.5f %8.5f | %11.3f\n",
			norm_unit.x, norm_unit.y, norm_unit.z, norm_len );

		// now calc a normal by the points

		double3 diff_2_0, diff_1_0, pt_norm;
		Sub3(diff_2_0,td->pt[2],td->pt[0]);
		Sub3(diff_1_0,td->pt[1],td->pt[0]);
		CrossProd3(pt_norm,diff_1_0,diff_2_0);
		Unit3(pt_norm);
		PRINT("NORM/PTS: %8.5f %8.5f %8.5f\n",
			pt_norm.x, pt_norm.y, pt_norm.z );

		// comparing
		double dp = DotProd3(norm_unit,pt_norm);
		if ( dp < -0.2 )
		{
		    PRINT(" -> SWAP POINTS, dp=%7.5f\n",dp);
		    memcpy(td->pt+1,pt3,sizeof(*td->pt));
		    memcpy(td->pt+2,pt2,sizeof(*td->pt));
		    par->swap_count++;
		}
	    }
	}

	#if SUPPORT_KCL_XTRIDATA
	if (opt_xtridata)
	{
	    CopyTriData_T2D(&td->dbl,td);
	    if (par->is_orig)
	    {
		td->status |= TDX_ORIG_PT;
		memcpy(&td->orig,&td->dbl,sizeof(td->orig));
	    }
	}
	#endif

// [[tridata]] [[2do]] ??? drop invalid triangles
    }
    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendQuadrilateralKCLp
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3,		// pointer to point #3
    const double3	*pt4		// pointer to point #4
)
{
    DASSERT(kcl);
    DASSERT(pt1);
    DASSERT(pt2);
    DASSERT(pt3);
    DASSERT(pt4);

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);
    return AppendQuadrilateralKCL(&par,pt1,pt2,pt3,pt4);
}

//-----------------------------------------------------------------------------

kcl_tridata_t *AppendQuadrilateralKCL
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_tri_param_t	*par,		// valid data structure
    const double3	*pt1,		// pointer to point #1
    const double3	*pt2,		// pointer to point #2
    const double3	*pt3,		// pointer to point #3
    const double3	*pt4		// pointer to point #4
)
{
    DASSERT(par);
    DASSERT(par->kcl);
    DASSERT(pt1);
    DASSERT(pt2);
    DASSERT(pt3);
    DASSERT(pt4);

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(par->kcl,2);
    if (!td)
	return 0;
    par->kcl->tridata.used -= 2;

    if ( Length2D(pt1->v,pt3->v) <= 1.1 * Length2D(pt2->v,pt4->v) )
    {
	// this variant is prioritized to force
	// 'pt1' as vertex at nearly same length

	AppendTriangleKCL( par, pt1, pt2, pt3 );
	AppendTriangleKCL( par, pt1, pt3, pt4 );
    }
    else
    {
	AppendTriangleKCL( par, pt2, pt4, pt1 );
	AppendTriangleKCL( par, pt2, pt3, pt4 );
    }
    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendTrianglesKCLp
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pt1,		// pointer to first point
    int			pt_delta,	// delta in bytes to next point, <0 possible
    uint		n_pt		// number of points
)
{
    DASSERT(kcl);
    DASSERT(pt1);
    DASSERT( pt_delta >= sizeof(double3) || pt_delta <= -(int)sizeof(double3) );

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);
    return AppendTrianglesKCL(&par,pt1,pt_delta,n_pt);
}

//-----------------------------------------------------------------------------

kcl_tridata_t * AppendTrianglesKCL
(
    // returns the new triangle, or NULL if failed (to many triangles)

    kcl_tri_param_t	*par,		// valid data structure
    const double3	*pt1,		// pointer to first point
    int			pt_delta,	// delta in bytes to next point, <0 possible
    uint		n_pt		// number of points
)
{
    DASSERT(par);
    DASSERT(par->kcl);
    DASSERT(pt1);
    DASSERT( pt_delta >= sizeof(double3) || pt_delta <= -(int)sizeof(double3) );

    if ( n_pt <= 4 )
	return n_pt == 3
		? AppendTriangleKCL(	par,
					pt1,
					(double3*)((u8*)pt1 +   pt_delta),
					(double3*)((u8*)pt1 + 2*pt_delta) )
		: n_pt == 4
		    ? AppendQuadrilateralKCL(
					par,
					pt1,
					(double3*)((u8*)pt1 +   pt_delta),
					(double3*)((u8*)pt1 + 2*pt_delta),
					(double3*)((u8*)pt1 + 3*pt_delta) )
		    : 0;

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(par->kcl,n_pt-2);
    if (!td)
	return 0;
    par->kcl->tridata.used -= n_pt-2;


    //--- setup internal data structure

    struct tab_t
    {
	double3 *pt;	// pointer to coordinates
	double  len;	// distance to point+2 (square!)
    };

    struct tab_t *tab, tab_buf[KCL_MAX_PT_PER_TRI];
    if ( n_pt <= KCL_MAX_PT_PER_TRI )
	tab = tab_buf;
    else
	tab = MALLOC(n_pt*sizeof(*tab)); // this function allows more points

    int i;
    for ( i = 0; i < n_pt; i++ )
	tab[i].pt = (double3*)( (u8*)pt1 + i * pt_delta );


    //--- polygons with >= 5 verteces

    DASSERT( n_pt >= 5 );
    // init differences
    for ( i = 0; i < n_pt; i++ )
	tab[i].len = Length2D(tab[i].pt->v,tab[(i+2)%n_pt].pt->v);


    //--- main loop

    while ( n_pt > 3 )
    {
	// find min distance
	uint found = 0;
	double min = tab[0].len;
	for ( i = 1; i < n_pt; i++ )
	    if ( min > tab[i].len )
	    {
		min = tab[i].len;
		found = i;
	    }
	uint found1 = ( found + 1 ) % n_pt;

	AppendTriangleKCL(par,
			tab[found].pt,
			tab[found1].pt,
			tab[(found+2)%n_pt].pt
			);

	//--- remove the mid point and fix the table

	n_pt--;
	memmove( tab+found1, tab+found1+1, (n_pt-found1)*sizeof(*tab) );
	tab[found].len = Length2D(tab[found].pt->v,tab[(found+2)%n_pt].pt->v);
	found = ( found + n_pt - 1 ) % n_pt;
	tab[found].len = Length2D(tab[found].pt->v,tab[(found+2)%n_pt].pt->v);
    }


    //--- add the last triangle

    DASSERT( n_pt == 3 );
    AppendTriangleKCL(par, tab[0].pt, tab[1].pt, tab[2].pt );


    //--- terminate

    if ( tab != tab_buf )
	FREE(tab);
    return td;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendOctahedronKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    octahedron_t	*oct		// valid octahedron data
)
{
    DASSERT(kcl);
    DASSERT(oct);

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(kcl,8);
    if (!td)
	return 0;
    kcl->tridata.used -= 8;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);

    AppendTriangleKCL( &par, &oct->x1, &oct->z1, &oct->y1 );
    AppendTriangleKCL( &par, &oct->z1, &oct->x2, &oct->y1 );
    AppendTriangleKCL( &par, &oct->x2, &oct->z2, &oct->y1 );
    AppendTriangleKCL( &par, &oct->z2, &oct->x1, &oct->y1 );

    AppendTriangleKCL( &par, &oct->x1, &oct->z2, &oct->y2 );
    AppendTriangleKCL( &par, &oct->z2, &oct->x2, &oct->y2 );
    AppendTriangleKCL( &par, &oct->x2, &oct->z1, &oct->y2 );
    AppendTriangleKCL( &par, &oct->z1, &oct->x1, &oct->y2 );

    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendCuboidKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    cuboid_t		*c		// valid cuboid data
)
{
    DASSERT(kcl);
    DASSERT(c);

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(kcl,12);
    if (!td)
	return 0;
    kcl->tridata.used -= 12;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);

    AppendQuadrilateralKCL( &par, &c->p000, &c->p010, &c->p011, &c->p001 );
    AppendQuadrilateralKCL( &par, &c->p000, &c->p001, &c->p101, &c->p100 );
    AppendQuadrilateralKCL( &par, &c->p000, &c->p100, &c->p110, &c->p010 );
    AppendQuadrilateralKCL( &par, &c->p111, &c->p101, &c->p001, &c->p011 );
    AppendQuadrilateralKCL( &par, &c->p111, &c->p110, &c->p100, &c->p101 );
    AppendQuadrilateralKCL( &par, &c->p111, &c->p011, &c->p010, &c->p110 );
    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendPyramidKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*apex,		// the apex of the pyramid
    const double3	*pt1,		// pointer to first point
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt,		// number of points
    bool		reverse		// true: reverse face orientation
)
{
    DASSERT(kcl);
    DASSERT(apex);
    DASSERT(pt1);
    DASSERT(n_pt>=2);

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(kcl,n_pt);
    if (!td)
	return 0;
    kcl->tridata.used -= n_pt;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);

    if (reverse)
    {
	int i;
	const double3 *pt = pt1;
	for ( i = n_pt-1; i >= 0; i-- )
	{
	    const double3 *next = (double3*)((u8*)pt1 + i*pt_delta);
	    AppendTriangleKCL( &par, apex, pt, next );
	    pt = next;
	}
    }
    else
    {
	int i;
	const double3 *pt = (double3*)((u8*)pt1 + (n_pt-1)*pt_delta);
	for ( i = 0; i < n_pt; i++ )
	{
	    const double3 *next = (double3*)((u8*)pt1 + i*pt_delta);
	    noPRINT("%6.1f %6.1f %6.1f / %6.1f %6.1f %6.1f / %6.1f %6.1f %6.1f\n",
			apex->x, apex->y, apex->z,
			pt->x, pt->y, pt->z,
			next->x, next->y, next->z );
	    AppendTriangleKCL( &par, apex, pt, next );
	    pt = next;
	}
    }
    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t *AppendPrismKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*axis1,		// first axis point
    const double3	*axis2,		// second axis point
    const double3	*dir,		// NULL or direction of first edge
    double		r,		// radius
    uint		n_side,		// number of sides, max=100
    uint		mode,		// bitfield: 0=prism, 1=antiprism
    u32			base_flag,	// color of base, -1:no base
    double		base_height,	// height of base, truncated to 40% of length
					// if 0.0: use a plane, otherwise a pyramid
    u32			arrow_flag,	// color of arror peak at axis2, -1:no arrow
    double		arrow_size,	// relative size of arrow (multiplied by *r),
					// truncated to length. no arrow if <=0.0
    bool		arrow_flip	// true: flip direction of arrow
)
{
    DASSERT(kcl);
    DASSERT(axis1);
    DASSERT(axis2);

    const uint max_side = 100;
    if ( n_side < 2 )
	n_side = 2;
    else if ( n_side > max_side )
	n_side = max_side;

    const uint n_tri = n_side * ( base_flag == M1(base_flag) ? 2 : 4 );
    kcl_tridata_t *td = PrepareAppendTrianglesKCL(kcl,n_tri);
    if (!td)
	return 0;
    kcl->tridata.used -= n_tri;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);

    //--- calculations

    double3 n[3];
    double prism_len = CalcNormals(n,axis1,axis2,dir,r);
    if (!prism_len)
	return 0;

    if ( base_height > 0.4 * prism_len )
	 base_height = 0.4 * prism_len;
    base_height /= r;

    const bool draw_arrow = arrow_flag != M1(arrow_flag) && arrow_size > 0.0;
    double arrow_height;
    if (draw_arrow)
    {
	arrow_height = 1.5;
	const double max = prism_len / (arrow_height*r) - 1.0;
	if ( arrow_size > max )
	     arrow_size = max;
    }
    else
	arrow_height = base_height;

    double3 base1, base2;
    base1.x = axis1->x + n[0].x * base_height;
    base1.y = axis1->y + n[0].y * base_height;
    base1.z = axis1->z + n[0].z * base_height;
    base2.x = axis2->x - n[0].x * arrow_height;
    base2.y = axis2->y - n[0].y * arrow_height;
    base2.z = axis2->z - n[0].z * arrow_height;

    double3 tab1[max_side+1], tab2[max_side+1];
    double3 *p1 = tab1, *p2 = tab2;


    //--- calculate the points and print the faces

    if ( mode & 1 )
    {
	// anti prism

	const double add_rad = M_PI / n_side;
	double rad = 0.0;

	uint i;
	for ( i = 0; i < n_side; i++ )
	{
	    double fcos, fsin;

	    fcos = cos(rad);
	    fsin = sin(rad);
	    rad += add_rad;
	    p1->x = base1.x + fcos * n[1].x + fsin * n[2].x;
	    p1->y = base1.y + fcos * n[1].y + fsin * n[2].y;
	    p1->z = base1.z + fcos * n[1].z + fsin * n[2].z;
	    p1++;

	    fcos = cos(rad);
	    fsin = sin(rad);
	    rad += add_rad;
	    p2->x = base2.x + fcos * n[1].x + fsin * n[2].x;
	    p2->y = base2.y + fcos * n[1].y + fsin * n[2].y;
	    p2->z = base2.z + fcos * n[1].z + fsin * n[2].z;
	    p2++;
	}
	*p1 = *tab1;
	*p2 = *tab2;

	for ( i = 0, p1 = tab1, p2 = tab2; i < n_side; i++, p1++, p2++ )
	{
	    AppendTriangleKCL( &par, p1, p1+1, p2   );
	    AppendTriangleKCL( &par, p2, p1+1, p2+1 );
	}
    }
    else
    {
	const double rad_f = 2*M_PI / n_side;

	uint i;
	for ( i = 0; i < n_side; i++, p1++, p2++ )
	{
	    const double rad = i * rad_f;
	    const double fcos = cos(rad);
	    const double fsin = sin(rad);
	    noPRINT("ROT=%6.2f, COS=%6.3f, SIN=%6.3f\n", rad * (180/M_PI), fcos, fsin );
	    double a;
	    a = fcos * n[1].x + fsin * n[2].x;  p1->x = base1.x + a;  p2->x = base2.x + a;
	    a = fcos * n[1].y + fsin * n[2].y;  p1->y = base1.y + a;  p2->y = base2.y + a;
	    a = fcos * n[1].z + fsin * n[2].z;  p1->z = base1.z + a;  p2->z = base2.z + a;
	    if ( i > 0 )
		AppendQuadrilateralKCL( &par, p1-1, p1, p2, p2-1 );
	}
	AppendQuadrilateralKCL( &par, p1-1, tab1, tab2, p2-1 );
    }

    //--- print base pyramids

    if ( base_flag != M1(base_flag) )
    {
	if ( fabs(base_height) < 1e-9 )
	{
	    AppendTrianglesKCL( &par, tab1 + n_side - 1,
					-(int)sizeof(*tab1), n_side );
	    if (!draw_arrow)
		AppendTrianglesKCL( &par, tab2, sizeof(*tab2), n_side );
	}
	else
	{
	    AppendPyramidKCL(kcl,base_flag,axis1,tab1,sizeof(*tab1),n_side,true);
	    if (!draw_arrow)
		AppendPyramidKCL(kcl,base_flag,axis2,tab2,sizeof(*tab2),n_side,false);
	}
    }

    if (draw_arrow)
    {
	if (arrow_flip)
	{
	    // ???
	}
	else
	{
	    uint i;
	    for ( i = 0, p2 = tab2; i < n_side; i++, p2++ )
	    {
		p2->x += ( p2->x - axis2->x ) * arrow_size;
		p2->y += ( p2->y - axis2->y ) * arrow_size;
		p2->z += ( p2->z - axis2->z ) * arrow_size;
	    }
	    AppendPyramidKCL(kcl,arrow_flag,axis2,tab2,sizeof(*tab2),n_side,true);
	    AppendPyramidKCL(kcl,arrow_flag,axis2,tab2,sizeof(*tab2),n_side,false);
	}
    }

    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t * AppendCylinderKCL
(
    kcl_t		*kcl,		// valid KCL data structure
    u32			kcl_flag,	// value of kcl flag
    const double3	*pos,		// base point of cylinder
    const double3	*p_scale,	// scale. If NULL: assume v(1,1,1)
    double		scale_factor,	// Scale is multiplied by this factor
    const double3	*rotate,	// NULL or rotation vector
    uint		n_side,		// number of sides, max=100
    u32			base_flag	// color of bases, -1:no base
)
{
    DASSERT(kcl);
    DASSERT(pos);

    const uint max_side = 100;
    if ( n_side < 2 )
	n_side = 2;
    else if ( n_side > max_side )
	n_side = max_side;

    const uint n_tri = n_side * ( base_flag == M1(base_flag) ? 2 : 4 );

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(kcl,n_tri);
    if (!td)
	return 0;
    kcl->tridata.used -= n_tri;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);


    //--- setup pt list

    const uint n_used = 2*n_side+2;
    double3 pt[2*max_side+2], scale;
    double3 *tab0 = pt, *tab1 = tab0 + n_side+1;

    scale.x = scale.y = scale.z = scale_factor;
    if (p_scale)
    {
	scale.x *= p_scale->x;
	scale.y *= p_scale->y;
	scale.z *= p_scale->z;
    }
    const double ht = 2*scale.y;

    double3 *p0 = tab0, *p1 = tab1;
    const double add_rad = M_PI / n_side;
    double rad = 0.0;

    uint i;
    for ( i = 0; i < n_side; i++ )
    {
	p0->x = scale.x * cos(rad);
	p0->y = 0.0;
	p0->z = scale.z * sin(rad);
	p0++;
	rad += add_rad;

	p1->x = scale.x * cos(rad);
	p1->y = ht;
	p1->z = scale.z * sin(rad);
	p1++;
	rad += add_rad;
    }
    *p0 = *tab0;
    *p1 = *tab1;


    //--- calculations

    if (rotate)
	Rotate(0,rotate,pt,sizeof(*pt),n_used);

    Translate(pos,pt,sizeof(*pt),n_used);


    //--- draw

    for ( i = 0, p0 = tab0, p1 = tab1; i < n_side; i++, p0++, p1++ )
    {
	AppendTriangleKCL( &par, p0+1, p0, p1   );
	AppendTriangleKCL( &par, p0+1, p1, p1+1 );
    }

    if ( base_flag != M1(base_flag) )
    {
	InitializeTriParam(&par,kcl,base_flag);
	AppendTrianglesKCL(&par,tab0,sizeof(*tab0),n_side);
	AppendTrianglesKCL(&par,tab1+n_side-1,-(int)sizeof(*tab1),n_side);
    }

    return td;
}

///////////////////////////////////////////////////////////////////////////////

kcl_tridata_t * AppendJoistKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    u32			kcl_flag,	// parameter for AppendTrianglesKCLp()
    double		length,		// length of corpus
    const double3	*p1,		// first point
    const double3	*p2,		// second point
    const double3	*p3,		// helper point to find direction
    int			n_mark,		// >0: add marker representing the number (modulo)
    u32			mark5_flag	// >0: replace 5 marker by one of this color
)
{
    DASSERT(kcl);
    DASSERT(p1);
    DASSERT(p2);
    DASSERT(p3);

    kcl_tridata_t *td = PrepareAppendTrianglesKCL(kcl,6);
    if (!td)
	return 0;
    kcl->tridata.used -= 6;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,kcl_flag);


    //--- calculate joist

    double3 d13, n12, n13, nx;
    Sub3(n12,*p2,*p1);
    Unit3(n12);
    Sub3(d13,*p3,*p1);
    CrossProd3(nx,n12,d13);
    Unit3(nx);
    CrossProd3(n13,nx,n12);

    double3 p1a, p1b, p2a, p2b;
    uint p;
    for ( p = 0; p < 3; p++ )
    {
	const double d12 = length * n12.v[p];
	const double d13 = length * n13.v[p];
	const double dx  = length * nx.v[p] * 0.3;
	p1a.v[p] = p1->v[p] + d12 + d13 - dx;
	p1b.v[p] = p1->v[p] + d12 + d13 + dx;
	p2a.v[p] = p2->v[p] - d12 + d13 - dx;
	p2b.v[p] = p2->v[p] - d12 + d13 + dx;
    }


    //--- draw joist

    AppendQuadrilateralKCL( &par,  p1,  &p1a, &p2a,  p2  );
    AppendQuadrilateralKCL( &par, &p1a, &p1b, &p2b, &p2a );
    AppendQuadrilateralKCL( &par, &p1b,  p1,   p2,  &p2b );

    if ( n_mark <= 0 )
	return td;


    //--- calc marker counter

    const uint n_mark5 = mark5_flag > 0 ? n_mark/5 : 0;
    n_mark -= n_mark5 * 4;


    //--- calc marker basics

    length *= 2;

    double3 pt1, pt2, delta[4];
    for ( p = 0; p < 3; p++ )
    {
	const double d12 = length * n12.v[p];
	const double d13 = length * n13.v[p];
	const double dx  = length * nx.v[p];

	pt1.v[p] = p1->v[p] + d12 + 0.25 * d13;
	pt2.v[p] = p2->v[p] - d12 + 0.25 * d13;

	delta[0].v[p] = -d13 - dx;
	delta[1].v[p] = -d13 + dx;
	delta[2].v[p] =  d13 + dx;
	delta[3].v[p] =  d13 - dx;
	noPRINT("DELTA[%u]: %6.1f %6.1f %6.1f %6.1f\n",
		p, delta[0].v[p], delta[1].v[p], delta[2].v[p], delta[3].v[p] );
    }


    //--- calc axis positions

    const double factor = 2 * n_mark + 0.51;

    double dist = LengthD(pt1.v,pt2.v);
    double need = length * factor;
    if ( need <= dist )
    {
	need = (1.0-need/dist)/2;
	for ( p = 0; p < 3; p++ )
	{
	    double d = ( pt2.v[p] - pt1.v[p] ) * need;
	    pt1.v[p] += d;
	    pt2.v[p] -= d;
	}
    }

    double3 dx;
    dx.x = ( pt2.x - pt1.x ) / factor;
    dx.y = ( pt2.y - pt1.y ) / factor;
    dx.z = ( pt2.z - pt1.z ) / factor;


    //--- append markers

    uint i;
    for ( i = 0; i < n_mark; i++ )
    {
	if ( i < n_mark5 )
	{
	    double3 base1[4], base2[4];
	    for ( p = 0; p < 3; p++ )
	    {
		pt2.v[p] = pt1.v[p] + 0.5 * dx.v[p];
		pt1.v[p] = pt2.v[p] + 1.5 * dx.v[p];

		uint j;
		for ( j = 0; j < 4; j++ )
		{
		    base1[j].v[p] = pt1.v[p] + delta[j].v[p];
		    base2[j].v[p] = pt2.v[p] + delta[j].v[p];
		}
	    }

	    AppendPyramidKCL(kcl,mark5_flag,&pt2,base1,sizeof(double3),4,true);
	    AppendPyramidKCL(kcl,mark5_flag,&pt1,base2,sizeof(double3),4,true);
	}
	else
	{
	    double3 base[4];
	    for ( p = 0; p < 3; p++ )
	    {
		pt2.v[p] = pt1.v[p] + 0.5 * dx.v[p];
		pt1.v[p] = pt2.v[p] + 1.5 * dx.v[p];

		const double base1 = ( pt1.v[p] + pt2.v[p] ) / 2;
		uint j;
		for ( j = 0; j < 4; j++ )
		    base[j].v[p] = base1 + delta[j].v[p];
	    }

	    AppendPyramidKCL(kcl,kcl_flag,&pt2,base,sizeof(double3),4,false);
	    AppendPyramidKCL(kcl,kcl_flag,&pt1,base,sizeof(double3),4,true);
	}
    }

    return td;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			statistics			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetStatisticsKCL ( kcl_t * kcl )
{
    DASSERT(kcl);

    FREE(kcl->o_depth);
    FREE(kcl->o_count);
    FREE(kcl->l_count);

    u8 *stat_beg = (u8*)&kcl->stat_valid;
    u8 *stat_end = (u8*)kcl + sizeof(*kcl);
    memset(stat_beg,0,stat_end-stat_beg);

    DASSERT(!kcl->stat_valid);
    DASSERT(!kcl->o_depth);
    DASSERT(!kcl->o_count);
    DASSERT(!kcl->l_count);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static u32 calc_stat_oct
(
    // returns next offset

    kcl_t		*kcl,		// valid KCL
    u32			offset,		// offset check
    uint		n		// number of links to check
)
{
    DASSERT(kcl);
    noPRINT("calc_stat_oct: off=%x, n=%x\n",offset,n);

    if ( offset >= kcl->octree_size )
	return kcl->octree_size;
    const u32 baseoff = offset;

    const uint max_n = ( kcl->octree_size - offset ) / 4;
    if ( n > max_n )
	 n = max_n;

    while ( n-- > 0 )
    {
	u32 val = be32(kcl->octree+offset);
	offset += 4;

	if ( val & 0x80000000 )
	{
	    val = ( val & 0x7ffffff ) + baseoff + 2;
	    if ( kcl->min_l_offset > val )
		 kcl->min_l_offset = val;
	    if ( kcl->max_l_offset < val )
		 kcl->max_l_offset = val;
	}
	else
	{
	    val += baseoff + 0x20;
	    if ( kcl->max_o_offset < val )
		 kcl->max_o_offset = val;
	}
    }
    return offset;
}

///////////////////////////////////////////////////////////////////////////////

static u32 calc_stat_octlist
(
    // returns next offset

    kcl_t		*kcl,		// valid KCL
    u32			offset,		// offset check
    uint		n,		// number of links to check
    uint		depth		// current depth
)
{
    DASSERT(kcl);
    noPRINT("calc_stat_oct: off=%x, n=%x\n",offset,n);

    if ( offset >= kcl->octree_size )
	return kcl->octree_size;
    const u32 baseoff = offset;

    const uint max_n = ( kcl->octree_size - offset ) / 4;
    if ( n > max_n )
	 n = max_n;

    while ( n-- > 0 )
    {
	u32 val = be32(kcl->octree+offset);
	offset += 4;

	if ( val & 0x80000000 )
	{
	    u32 off = ( val & 0x7ffffff ) + baseoff + 2;
	    val = ( off - kcl->min_l_offset ) / 2;
	    if ( val < kcl->n_trilist )
	    {
		u32 len = 0;
		while (be16(kcl->octree+off))
		    off+=2, len++;
		if (len)
		{
		    kcl->sum_tri_len += len;
		    if ( kcl->max_tri_len < len )
			 kcl->max_tri_len = len;
		    if ( depth < KCL_MAX_STAT_DEPTH
			 && kcl->max_depth_tri_len[depth] < len )
		    {
			 kcl->max_depth_tri_len[depth] = len;
		    }
		    kcl->n_tri_link++;
		    if (!kcl->l_count[val]++)
			kcl->n_tri_list++;
		}
		else
		{
		    kcl->n_0_link++;
		    if (!kcl->l_count[val]++)
			kcl->n_0_list++;
		}
	    }
	}
	else
	{
	    val = ( val + baseoff - kcl->min_o_offset ) / 0x20;
	    if ( val < kcl->n_cube_nodes )
	    {
		kcl->o_count[val]++;
		if ( kcl->o_depth[val] < depth )
		     kcl->o_depth[val] = depth;
	    }
	}
    }
    return offset;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CalcStatisticsKCL
(
    kcl_t		*kcl		// valid KCL
)
{
    DASSERT(kcl);

    if (kcl->stat_valid)
	return;

    DASSERT(kcl);
    ResetStatisticsKCL(kcl);
    KCL_ACTION_LOG("CalcStatisticsKCL()\n");
    CreateOctreeKCL(kcl);


    //--- setup

    kcl->hash_bits = 0;
    kcl->total_bcubes = 1;
    kcl->bcube_width = 1 << kcl->coord_rshift;


    //--- axis dependent calculations

    uint p;
    for ( p = 0; p < 3; p++ )
    {
	u32 val = kcl->mask[p];
	uint count;
	if (!val)
	    count = 32;
	else
	{
	    count = 0;
	    for ( ; !(val&1); val >>= 1 )
		count++;
	}
	kcl->mask_bits[p] = count;
	count = count > kcl->coord_rshift ? count - kcl->coord_rshift : 0;
	kcl->bcube_bits[p]	 = count;
	kcl->hash_bits		+= count;
	kcl->n_bcubes[p]	 = 1 << count;
	kcl->total_bcubes	*= 1 <<  count;
	kcl->max_octree.v[p]	 = kcl->min_octree.v[p]
				 + kcl->bcube_width * kcl->n_bcubes[p];
    }

    //--- logging

    noPRINT("KCL BASE STATS:\n"
	"\t\tH.mask:         0x%08x  0x%08x  0x%08x\n"
	"\t\tH.shift:       %11u %11u %11u\n"
	"\t\tS.mask_bits:   %11u %11u %11u\n"
	"\t\tS.bcube_bits:  %11u %11u %11u =>%5u\n"
	"\t\tS.n_bcubes:    %11u %11u %11u =>%5u\n"
	"\t\tS.bcube_width: %19u\n"
	"\t\tH.min_octree:  %11.3f %11.3f %11.3f\n"
	"\t\tS.max_octree:  %11.3f %11.3f %11.3f\n"
	"\t\t---\n",
	kcl->mask[0], kcl->mask[1], kcl->mask[2],
	kcl->coord_rshift, kcl->y_lshift, kcl->z_lshift,
	kcl->mask_bits[0], kcl->mask_bits[1], kcl->mask_bits[2],
	kcl->bcube_bits[0], kcl->bcube_bits[1], kcl->bcube_bits[2], kcl->hash_bits,
	kcl->n_bcubes[0], kcl->n_bcubes[1], kcl->n_bcubes[2], kcl->total_bcubes,
	kcl->bcube_width,
	kcl->min_octree.x, kcl->min_octree.y, kcl->min_octree.z,
	kcl->max_octree.x, kcl->max_octree.y, kcl->max_octree.z
	);


    //--- octree setup

    kcl->max_o_offset = kcl->total_bcubes * 4;
    kcl->max_l_offset = 0;
    kcl->min_l_offset = kcl->octree_size;

    u32 offset = calc_stat_oct(kcl,0,kcl->total_bcubes);
    kcl->min_o_offset = offset;
    while ( offset < kcl->max_o_offset && offset < kcl->octree_size )
	offset = calc_stat_oct(kcl,offset,8);

    kcl->n_cube_nodes  = ( kcl->max_o_offset - kcl->min_o_offset ) / 0x20;
    kcl->n_trilist = ( kcl->octree_size - kcl->min_l_offset ) / 2 + 1;


    //--- fix max_l_offset to end of list

    offset = kcl->max_l_offset;
    while ( offset < kcl->octree_size )
    {
	const u16 val = be16(kcl->octree+offset);
	offset += 2;
	if (!val)
	    break;
    }
    kcl->max_l_offset = offset;


    //--- logging

    noPRINT("KCL OCTREE STATS:\n"
	"\t\tS.o_offset: %#9x .. %#9x, N:%8u\n"
	"\t\tS.l_offset: %#9x .. %#9x, N:%8u\n"
	"\t\t---\n",
	kcl->min_o_offset, kcl->max_o_offset, kcl->n_cube_nodes,
	kcl->min_l_offset, kcl->max_l_offset, kcl->n_trilist );


    //--- setup octree lists

    FREE(kcl->o_depth);
    FREE(kcl->o_count);
    FREE(kcl->l_count);
    kcl->o_depth = CALLOC(kcl->n_cube_nodes,sizeof(*kcl->o_depth));
    kcl->o_count = CALLOC(kcl->n_cube_nodes,sizeof(*kcl->o_count));
    kcl->l_count = CALLOC(kcl->n_trilist,sizeof(*kcl->l_count));

    kcl->n_0_list	= 0;
    kcl->n_0_link	= 0;
    kcl->n_tri_list	= 0;
    kcl->n_tri_link	= 0;
    kcl->max_tri_len	= 0;
    kcl->sum_tri_len	= 0;
    memset(kcl->max_depth_tri_len,0,sizeof(kcl->max_depth_tri_len));


    //--- fill octree lists

    uint max_depth = 0, sum_depth = 0;
    offset = calc_stat_octlist(kcl,0,kcl->total_bcubes,1);

    while ( offset < kcl->max_o_offset && offset < kcl->octree_size )
    {
	const uint depth = kcl->o_depth[(offset-kcl->min_o_offset)/0x20];
	sum_depth += 7*depth + 1;
	if ( max_depth < depth )
	     max_depth = depth;
	offset = calc_stat_octlist(kcl,offset,8,depth+1);
    }
    kcl->max_depth = max_depth;
    kcl->total_cubes = kcl->n_cube_nodes * 8 + kcl->total_bcubes;
    kcl->ave_depth = (double)sum_depth/kcl->total_cubes;

    kcl->min_depth = 0;
    uint d;
    for ( d = 0; d < KCL_MAX_STAT_DEPTH; d++ )
	if ( kcl->max_depth_tri_len[d] )
	{
	    kcl->min_depth = d - 1;
	    break;
	}


    //--- logging

    PRINT("KCL OCTREE STATS: 0=%u,%u,  tri=%u,%u,  len=%u,%u,  depth=%u..%u\n",
		kcl->n_0_list, kcl->n_0_link,
		kcl->n_tri_list, kcl->n_tri_link,
		kcl->max_tri_len, kcl->sum_tri_len,
		kcl->min_depth, kcl->max_depth );


    //--- setup triangle usage

    const int n_tri = kcl->tridata.used;
    u8 *tri_used = CALLOC(n_tri,sizeof(*tri_used));
    kcl->n_invalid_tri_ref = 0;
    kcl->n_tri_used = 0;


    //--- calc triangle usage

    noPRINT("OFF: %x .. %x\n",kcl->min_l_offset,kcl->max_l_offset);
    for ( offset = kcl->min_l_offset; offset < kcl->max_l_offset; offset += 2 )
    {
	uint val = be16( kcl->octree + offset );
	if (val)
	{
	    if ( --val >= n_tri )
		kcl->n_invalid_tri_ref++;
	    else if (!tri_used[val])
	    {
		tri_used[val] = 1;
		kcl->n_tri_used++;
	    }
	}
    }


    //--- set TD_UNUSED

    kcl->used_valid = true;
    kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    uint i;
    for ( i = 0; i < n_tri; i++, td++ )
	if (tri_used[i])
	    td->status &= ~TD_UNUSED;
	else
	    td->status |= TD_UNUSED;

    FREE(tri_used);


    //--- terminate & logging

    kcl->stat_valid = true;
    noPRINT("KCL TRIANGLE STATS: used=%u/%u, invalid link=%u\n",
		kcl->n_tri_used, n_tri, kcl->n_invalid_tri_ref );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CalcMinMaxKCL
(
    kcl_t		* kcl		// KCL data structure
)
{
    DASSERT(kcl);

    if (!kcl->min_max_valid)
    {
	KCL_ACTION_LOG("CalcMinMaxKCL()\n");
	kcl->min_max_valid = true;

	const uint n_tri = kcl->tridata.used;
	if (n_tri)
	{
	    const kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	    const kcl_tridata_t *td_end = td + n_tri;

	    memcpy(&kcl->min,td->pt,sizeof(kcl->min));
	    memcpy(&kcl->max,td->pt,sizeof(kcl->max));
	    double tmin = 1e99;
	    double tmax = 0.0;
	    double3 sum;
	    sum.x = sum.y = sum.z = 0.0;

	    for ( ; td < td_end; td++ )
	    {
		const double3 *d3 = td->pt;
		MinMax3(&kcl->min,&kcl->max,d3,3);

		sum.x += d3[0].x + d3[1].x + d3[2].x;
		sum.y += d3[0].y + d3[1].y + d3[2].y;
		sum.z += d3[0].z + d3[1].z + d3[2].z;

		double dist;
		double3 diff;

		Sub3(diff,d3[0],d3[1]);
		dist = LengthSqare3(diff);
		if ( tmin > dist ) tmin = dist;
		if ( tmax < dist ) tmax = dist;

		Sub3(diff,d3[0],d3[2]);
		dist = LengthSqare3(diff);
		if ( tmin > dist ) tmin = dist;
		if ( tmax < dist ) tmax = dist;

		Sub3(diff,d3[1],d3[2]);
		dist = LengthSqare3(diff);
		if ( tmin > dist ) tmin = dist;
		if ( tmax < dist ) tmax = dist;
	    }
	    kcl->tmin_dist = sqrt(tmin);
	    kcl->tmax_dist = sqrt(tmax);

	    const double factor = 1 / ( n_tri * 3.0 );
	    kcl->mean.x = sum.x * factor;
	    kcl->mean.y = sum.y * factor;
	    kcl->mean.z = sum.z * factor;
	}
	else
	{
	    memset(&kcl->min,0,sizeof(kcl->min));
	    memset(&kcl->max,0,sizeof(kcl->max));
	    memset(&kcl->mean,0,sizeof(kcl->max));
	    kcl->tmin_dist = 0.0;
	    kcl->tmax_dist = 0.0;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateOctreeKCL()		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[oct_info_t]]

typedef struct oct_info_t
{
    kcl_t		*kcl;		// current KCL
    uint		n_tri;		// number of triangles
    kcl_tri_t		*tri;		// triangle values
 #if SUPPORT_KCL_CUBE
    kcl_cube_list_t	*cubelist;	// NULL or a cube list for each 'tri'
 #endif

    u32			*olist;		// octree list
    uint		oused;		// number of used elements in 'olist'
    uint		osize;		// number of alloced elements in 'olist'

    u16			*tlist;		// triangle list
    uint		tused;		// number of used elements in 'tlist'
    uint		tsize;		// number of alloced elements in 'tlist'

} oct_info_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void setup_oct_tri
(
    kcl_t		*kcl,		// valid kcl
    kcl_tri_t		*tri,		// valid pointer -> dest
 #if SUPPORT_KCL_CUBE
    kcl_cube_list_t	*cl,		// NULL or pointer to a cube list
 #endif
    const kcl_tridata_t *td		// valid data -> source
)
{
    DASSERT(kcl);
    DASSERT(tri);
    DASSERT(td);

    uint p;
    const double3 *d3 = td->pt;
    for ( p = 0; p < 3; p++ )
    {
	s32 min = (u32)round( d3[0].v[p] - kcl->min_octree.v[p] );
	s32 max = min;
	tri->pt[0][p] = min;

	s32 temp = (u32)round( d3[1].v[p] - kcl->min_octree.v[p] );
	tri->pt[1][p] = temp;
	if ( min > temp )
	     min = temp;
	if ( max < temp )
	     max = temp;

	temp = (u32)round( d3[2].v[p] - kcl->min_octree.v[p] );
	tri->pt[2][p] = temp;

	tri->cube.min[p] = min < temp ? min : temp;
	tri->cube.max[p] = max > temp ? max : temp;
	noPRINT("  %u: min=%6d, max=%6d, t=%6d %6d %6d\n",
	    p, tri->cube.min[p], tri->cube.max[p],
	    tri->pt[0][p], tri->pt[1][p], tri->pt[0][p] );

     #if SUPPORT_KCL_CUBE
	if (cl)
	{
	    // [[2do]] [[cube]] KCL_TRI_SPLIT

	    cl->n = 1;
	    cl->cube[0] = tri->cube;
	}
     #endif
    }
    tri->select = td->status & TD_INVALID ? M1(tri->select) : 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//======================== X-tests ========================

#define AXISTEST_X01(a, b, fa, fb) \
	p0 = a*v0.y - b*v0.z; \
	p2 = a*v2.y - b*v2.z; \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = (fa+fb) * cubehalf; \
	if ( min > rad || max <- rad ) return 0;

#define AXISTEST_X2(a, b, fa, fb) \
	p0 = a*v0.y - b*v0.z; \
	p1 = a*v1.y - b*v1.z; \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = (fa+fb) * cubehalf; \
	if(min>rad || max<-rad) return 0;

//======================== Y-tests ========================

#define AXISTEST_Y02(a, b, fa, fb) \
	p0 = -a*v0.x + b*v0.z; \
	p2 = -a*v2.x + b*v2.z; \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = (fa+fb) * cubehalf; \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb) \
	p0 = -a*v0.x + b*v0.z; \
	p1 = -a*v1.x + b*v1.z; \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = (fa+fb) * cubehalf; \
	if(min>rad || max<-rad) return 0;

//======================== Z-tests ========================

#define AXISTEST_Z12(a, b, fa, fb) \
	p1 = a*v1.x - b*v1.y; \
	p2 = a*v2.x - b*v2.y; \
	if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = (fa+fb) * cubehalf; \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb) \
	p0 = a*v0.x - b*v0.y; \
	p1 = a*v1.x - b*v1.y; \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = (fa+fb) * cubehalf; \
	if(min>rad || max<-rad) return 0;

//==========================================================

static bool OctCubeTriangleOverlaped
(
    kcl_cube_t		*cube,		// cube info
    kcl_tri_t		*tri		// triangle info
 #if SUPPORT_KCL_CUBE
    ,
    kcl_cube_list_t	*cl		// NULL or cube list
 #endif
)
{
    DASSERT( cube && tri );

    //------------------------------------------------------------
    // we use this algorithm:
    // http://jgt.akpeters.com/papers/AkenineMoller01/tribox.html
    //------------------------------------------------------------

    //--- Bullet 1:  test the cube, that surrounds the triangle

    if ( tri->select == M1(tri->select)
	|| tri->cube.min[0] > cube->max[0] || tri->cube.max[0] < cube->min[0]
	|| tri->cube.min[1] > cube->max[1] || tri->cube.max[1] < cube->min[1]
	|| tri->cube.min[2] > cube->max[2] || tri->cube.max[2] < cube->min[2] )
    {
	return false;
    }


 #if SUPPORT_KCL_CUBE // [[cube]]
    if (cl)
    {
	if ( cl->n == 1 )
	    return true;

	// use [[cube]] param?
    }
 #endif


    //--- center cube and triangle

    const int cubehalf = ( cube->max[0] - cube->min[0] ) / 2;
    DASSERT( cubehalf == ( cube->max[1] - cube->min[1] ) / 2 );
    DASSERT( cubehalf == ( cube->max[2] - cube->min[2] ) / 2 );

    uint p;
    double3 v0, v1, v2;
    for ( p = 0; p < 3; p++ )
    {
	const int cubemid = cube->min[p] + cubehalf;
	v0.v[p] = tri->pt[0][p] - cubemid;
	v1.v[p] = tri->pt[1][p] - cubemid;
	v2.v[p] = tri->pt[2][p] - cubemid;
    }

    double3 e0,e1,e2;
    Sub3(e0,v1,v0); // tri edge 0
    Sub3(e1,v2,v1); // tri edge 1
    Sub3(e2,v0,v2); // tri edge 2

    //--- Bullet 3:

    double min, max, p0,p1,p2, rad, fex,fey,fez;

    fex = fabs(e0.x);
    fey = fabs(e0.y);
    fez = fabs(e0.z);
    AXISTEST_X01(e0.z, e0.y, fez, fey);
    AXISTEST_Y02(e0.z, e0.x, fez, fex);
    AXISTEST_Z12(e0.y, e0.x, fey, fex);

    fex = fabs(e1.x);
    fey = fabs(e1.y);
    fez = fabs(e1.z);
    AXISTEST_X01(e1.z, e1.y, fez, fey);
    AXISTEST_Y02(e1.z, e1.x, fez, fex);
    AXISTEST_Z0 (e1.y, e1.x, fey, fex);

    fex = fabs(e2.x);
    fey = fabs(e2.y);
    fez = fabs(e2.z);
    AXISTEST_X2 (e2.z, e2.y, fez, fey);
    AXISTEST_Y1 (e2.z, e2.x, fez, fex);
    AXISTEST_Z12(e2.y, e2.x, fey, fex);

    //--- Bullet 2:
    //  test if the box intersects the plane of the triangle
    //  compute plane equation of triangle: normal*x+d=0

    double3 normal;
    CrossProd3(normal,e0,e1);

    double3 vmin, vmax;
    for ( p = 0; p < 3; p++ )
    {
	double v = v0.v[p];
	if ( normal.v[p] > 0.0 )
	{
	    vmin.v[p] = -cubehalf - v;
	    vmax.v[p] =  cubehalf - v;
	}
	else
	{
	    vmin.v[p] =  cubehalf - v;
	    vmax.v[p] = -cubehalf - v;
	}
    }

    return DotProd3(normal,vmin) <= 0.0
	&& DotProd3(normal,vmax) >= 0.0;
}

///////////////////////////////////////////////////////////////////////////////

static u32 CalcCube
(
    oct_info_t		*oi,		// base data
    kcl_cube_t		*cube,		// cube info
    uint		depth		// current depth, also used for select level
)
{
    DASSERT(oi);
    DASSERT(oi->kcl);
    DASSERT(cube);

    noPRINT("CUBE*: min=%6u,%6u,%6u, max=%6u,%6u,%6u\n",
	    cube->min[0], cube->min[1], cube->min[2],
	    cube->max[0], cube->max[1], cube->max[2] );

    uint tri_count = 0;

    const uint cube_blow = oi->kcl->cube_blow;
    cube->min[0] -= cube_blow;
    cube->min[1] -= cube_blow;
    cube->min[2] -= cube_blow;
    cube->max[0] += cube_blow;
    cube->max[1] += cube_blow;
    cube->max[2] += cube_blow;

 #if SUPPORT_KCL_CUBE
    kcl_tri_t *tri, *end = oi->tri + oi->n_tri;
    kcl_cube_list_t *cl = oi->cubelist;
    for ( tri = oi->tri ; tri < end; tri++ )
    {
	if ( tri->select == depth && OctCubeTriangleOverlaped(cube,tri,cl) )
	{
	    tri->select++;
	    tri_count++;
	}
	if (cl)
	    cl++;
    }
 #else
    kcl_tri_t *tri, *end = oi->tri + oi->n_tri;
    for ( tri = oi->tri ; tri < end; tri++ )
	if ( tri->select == depth && OctCubeTriangleOverlaped(cube,tri) )
	{
	    tri->select++;
	    tri_count++;
	}
 #endif

    cube->min[0] += cube_blow;
    cube->min[1] += cube_blow;
    cube->min[2] += cube_blow;
    cube->max[0] -= cube_blow;
    cube->max[1] -= cube_blow;
    cube->max[2] -= cube_blow;

    depth++;
    uint cube_size = cube->max[0] - cube->min[0];

    if (   cube_size <= oi->kcl->max_cube_size && tri_count <= oi->kcl->max_cube_triangles
	|| cube_size <= oi->kcl->min_cube_size
	|| depth     >  oi->kcl->max_octree_depth )
    {
	noPRINT("--> TLIST:   n-tri =%5u, width =%6u, base = %6u,%6u,%6u, depth=%u/%u\n",
		tri_count, cube_size,
		cube->min[0], cube->min[1], cube->min[2],
		depth, kcl->max_octree_depth );

	if (!tri_count)
	    return 0;

	if ( oi->tused + tri_count + 1 > oi->tsize )
	{
	    oi->tsize *= 2;
	    TRACE("REALLOC(tlist), new size = %u\n",oi->tsize);
	    oi->tlist = REALLOC( oi->tlist, oi->tsize * sizeof(*oi->tlist) );
	}
	u32 result = 0x80000000 | oi->tused * sizeof(*oi->tlist);

	// create the list
	u16 *tdata = oi->tlist + oi->tused;
	u16 *tptr = tdata;
	for ( tri = oi->tri ; tri < end; tri++ )
	    if ( tri->select == depth )
	    {
		tri->select--;
		write_be16( tptr++, tri - oi->tri + 1 );
	    }
	write_be16( tptr++, 0);
	DASSERT( tptr - oi->tlist <= oi->tsize );
	DASSERT( tptr - oi->tlist == oi->tused + tri_count + 1 );

	if (!oi->kcl->fast)
	{
	    // try to find a duplicate
	    u16 *tmax = tdata - tri_count - 1;
	    const uint tsize = ( tri_count + 1 ) * sizeof(*oi->tlist);
	    for ( tptr = oi->tlist; tptr <= tmax; tptr++ )
		if (!memcmp(tptr,tdata,tsize))
		    return ( tptr - oi->tlist ) * sizeof(*oi->tlist) | 0x80000000;
	}

	// no duplicate found
	oi->tused += tri_count + 1;
	return result;
    }
    else
    {
	noPRINT("--> SUBCUBE: n-tri =%5u, width =%6u, base = %6u,%6u,%6u, depth=%u/%u\n",
		tri_count, cube_size,
		cube->min[0], cube->min[1], cube->min[2],
		depth, kcl->max_octree_depth );

	if ( oi->oused + 8 > oi->osize )
	{
	    oi->osize *= 2;
	    TRACE("REALLOC(olist), new size = %u\n",oi->osize);
	    oi->olist = REALLOC( oi->olist, oi->osize * sizeof(*oi->olist) );
	}
	u32 result = oi->oused * sizeof(*oi->olist);
	uint ores = oi->oused;
	oi->oused += 8;
	DASSERT( oi->oused <= oi->osize );

	cube_size /= 2;
	uint zi;
	for ( zi = 0; zi < 2; zi++ )
	{
	    uint yi;
	    for ( yi = 0; yi < 2; yi++ )
	    {
		uint xi;
		for ( xi = 0; xi < 2; xi++ )
		{
		    kcl_cube_t subcube;
		    subcube.min[0] = cube->min[0] + xi * cube_size;
		    subcube.max[0] = subcube.min[0] + cube_size;
		    subcube.min[1] = cube->min[1] + yi * cube_size;
		    subcube.max[1] = subcube.min[1] + cube_size;
		    subcube.min[2] = cube->min[2] + zi * cube_size;
		    subcube.max[2] = subcube.min[2] + cube_size;

		    // 2 steps, because oi->olist may be realloced
		    const u32 result = CalcCube(oi,&subcube,depth);
		    oi->olist[ores++] = result;
		}
	    }
	}

	for ( tri = oi->tri ; tri < end; tri++ )
	    if ( tri->select == depth )
		tri->select--;
	return result;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateOctreeKCL
(
    kcl_t		* kcl		// KCL data structure
)
{
    DASSERT(kcl);
    if ( kcl->octree_valid && !kcl->recreate_octree )
	return ERR_NOTHING_TO_DO;

    TRACE_SIZEOF(kcl_mode_t);
    TRACE_SIZEOF(kcl_cube_t);
 #if SUPPORT_KCL_CUBE
    TRACE_SIZEOF(kcl_cube_list_t);
 #endif
    TRACE_SIZEOF(kcl_tri_t);
    TRACE_SIZEOF(oct_info_t);

    if (!kcl->silent_octree)
	KCL_ACTION_LOG("Create KCL Octree [valid=%d,recreate=%d]\n",
			kcl->octree_valid, kcl->recreate_octree );

    CalcMinMaxKCL(kcl);

    if (kcl->octree_alloced)
	FREE(kcl->octree);
    kcl->octree = 0;
    kcl->octree_size = 0;
    kcl->octree_alloced = false;
    kcl->recreate_octree = false;

    if (kcl->raw_data_alloced)
	FREE(kcl->raw_data);
    kcl->raw_data		= 0;
    kcl->raw_data_size		= 0;
    kcl->raw_data_alloced	= false;
    kcl->model_modified		= true;


    //--- calculate relevant bits

    uint p, relevant_bits[3], max_relevant = 0;
    double max[3];
    for ( p = 0; p < 3; p++ )
    {
	kcl->min_octree.v[p]
	    = ( IsNormalD(KCL_MIN.v[p]) ? KCL_MIN.v[p] : floor(kcl->min.v[p]) )
	    - kcl->cube_blow;

	int exp;
	max[p] = ( IsNormalD(KCL_MAX.v[p]) ? KCL_MAX.v[p] : kcl->max.v[p] )
	       + kcl->cube_blow;

	frexp( max[p] - kcl->min_octree.v[p], &exp );
	relevant_bits[p] = exp < 0 ? 0 : exp;
	kcl->mask[p] = ~(u32)0 << relevant_bits[p];
	if ( max_relevant < relevant_bits[p] )
	     max_relevant = relevant_bits[p];
    }
    noPRINT("  *MIN: %11.3f %11.3f %11.3f\n", kcl->min.x, kcl->min.y, kcl->min.z );
    noPRINT("  *MAX: %11.3f %11.3f %11.3f\n", kcl->max.x, kcl->max.y, kcl->max.z );
    noPRINT("  BASE: %11.3f %11.3f %11.3f\n",
		kcl->min_octree.x, kcl->min_octree.y, kcl->min_octree.z );


    //--- maximum allowed bits in base cube selection

    int max_bits;
    if (KCL_BITS)
	max_bits = KCL_BITS;
    else
    {
	max_bits = log(kcl->tridata.used)/log(2);
	if ( max_bits < 5 )
	     max_bits = 5;
     #if 0
	else if ( max_bits >= 10 )
	    max_bits = max_relevant > 10
				? 10 + ( max_relevant - 10 ) / 2
				: 10;
     #else
	else if ( max_bits > 10 )
	    max_bits = 10;
     #endif
    }
    PRINT("KCL-PARAM: BITS=%u, MAX_TRI=%u, MAX_DEPTH=%u, CUBE_SIZE=%u..%u, BLOW=%u\n",
		max_bits, kcl->max_cube_triangles, kcl->max_octree_depth,
		kcl->min_cube_size, kcl->max_cube_size, kcl->cube_blow );


    //--- calculate shifts

    uint rshift = 1, nbits[3];
    while ( 1 << rshift < kcl->min_cube_size )
	rshift++;
    for(;;)
    {
	uint sumbits = 0;
	for ( p = 0; p < 3; p++ )
	{
	    nbits[p] = relevant_bits[p] > rshift ? relevant_bits[p] - rshift : 0;
	    sumbits += nbits[p];
	}
	noPRINT("RSHIFT=%2u, NBITS= %2d + %2d + %2d = %2d / %2d\n",
		rshift,
		nbits[0], nbits[1], nbits[2],
		sumbits, max_bits );
	if ( sumbits <= max_bits )
	    break;
	rshift++;
    }

    const uint xn = 1 << nbits[0];
    const uint yn = 1 << nbits[1];
    const uint zn = 1 << nbits[2];
    const uint n_cubes = xn * yn * zn;

    PRINT("RSHIFT=%u, NBITS=%u,%u,%u, RELBITS=%u,%u,%u, N_CUBES=%u\n",
		rshift,
		nbits[0], nbits[1], nbits[2],
		relevant_bits[0], relevant_bits[1], relevant_bits[2],
		n_cubes );

    kcl->coord_rshift = rshift;
    kcl->y_lshift = nbits[0];
    kcl->z_lshift = nbits[0] + nbits[1];


    //--- adjust min_coord

    if ( KCL_MODE & KCLMD_CENTER )
    {
	for ( p = 0; p < 3; p++ )
	{
	    const double space	= ( 1 << rshift + nbits[p] )
				+ kcl->min_octree.v[p] - max[p];
	    noPRINT("ADJUST[%u] MIN=%11.3f, SPC=%11.3f\n",p,kcl->min_octree.v[p],space);
	    if ( space > 0.0 )
	    {
		PRINT("ADJUST[%u] %11.3f -> %11.3f\n",
			p, kcl->min_octree.v[p],
			kcl->min_octree.v[p] - floor(space/2) );
		kcl->min_octree.v[p] -= floor(space/2);
	    }
	}
    }


    //--- setup data for octree calculations

    oct_info_t oi;
    memset(&oi,0,sizeof(oi));
    oi.kcl	= kcl;
    oi.n_tri	= kcl->tridata.used;
    oi.tri	= CALLOC(oi.n_tri,sizeof(*oi.tri));
 #ifdef TEST // [[cube]]
    if ( KCL_MODE & KCLMD_CUBE )
	oi.cubelist = CALLOC(oi.n_tri,sizeof(*oi.cubelist));
 #endif

    oi.osize	= oi.n_tri + n_cubes;
    oi.oused	= n_cubes;
    oi.olist	= CALLOC(oi.osize,sizeof(*oi.olist));
    uint ores	= 0;

    oi.tsize	= 2 + oi.n_tri;
    oi.tused	= 0;
    oi.tlist	= CALLOC(oi.tsize,sizeof(*oi.tlist));

    if ( verbose >= 0 && !(KCL_MODE & KCLMD_SILENT) && !kcl->silent_octree )
	fprintf(stdlog,
		"  - create octree: rshift=%u, n_bcube=%u, cube_size=%u..%u, blow=%u,"
			" max_tri=%u, max_depth=%u, fast=%u\n",
		rshift, n_cubes,
		kcl->min_cube_size, kcl->max_cube_size, kcl->cube_blow,
		kcl->max_cube_triangles, kcl->max_octree_depth, kcl->fast);

    uint t;
    kcl_tri_t *tri = oi.tri;
    const kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    for ( t = 0; t < oi.n_tri; t++, tri++, td++ )
    {
     #if SUPPORT_KCL_CUBE
	setup_oct_tri( kcl, tri, oi.cubelist ? oi.cubelist + t : 0 , td );
     #else
	setup_oct_tri(kcl,tri,td);
     #endif
	noPRINT("%5u: min=%5d,%5d,%5d, max=%5d,%5d,%5d, select=%d\n",
	    t,
	    tri->min[0], tri->min[1], tri->min[2],
	    tri->max[0], tri->max[1], tri->max[2],
	    tri->select );
    }


    //--- now create the octree (start recursion)

    const uint cube_size = 1 << rshift;

    uint zi;
    for ( zi = 0; zi < zn; zi++ )
    {
	uint yi;
	for ( yi = 0; yi < yn; yi++ )
	{
	    uint xi;
	    for ( xi = 0; xi < xn; xi++ )
	    {
		kcl_cube_t cube;
		cube.min[0] = xi * cube_size;
		cube.max[0] = cube.min[0] + cube_size;
		cube.min[1] = yi * cube_size;
		cube.max[1] = cube.min[1] + cube_size;
		cube.min[2] = zi * cube_size;
		cube.max[2] = cube.min[2] + cube_size;

		// 2 steps, because oi.olist may be realloced
		const u32 result = CalcCube(&oi,&cube,0);
		oi.olist[ores++] = result;
	    }
	}
    }

    PRINT("*** O-USED = %u, T-USED = %u, n_CUBES = %u\n",oi.oused,oi.tused,n_cubes);
    //HEXDUMP16(0,0,oi.olist,oi.oused*sizeof(*oi.olist)); putchar('\n');
    //HEXDUMP16(0,0,oi.tlist,oi.tused*sizeof(*oi.tlist)); putchar('\n');


    //--- concatenate both data areas

    if (!oi.tused)
    {
	// be sure that at least one 0 element is in the list
	oi.tused++;
	*oi.tlist = 0;
    }

    u32 toffset = oi.oused * sizeof(*oi.olist);
    u32 tsize   = oi.tused * sizeof(*oi.tlist);
    kcl->octree_size = toffset + tsize;
    kcl->octree = REALLOC(oi.olist,kcl->octree_size);
    kcl->octree_alloced = true;
    oi.olist = (u32*)kcl->octree;
    memcpy( kcl->octree + toffset, oi.tlist, tsize );


    //--- we have an absolute offset, but need big endian relative -> fix it

    u32 null_offset = tsize + toffset - 4 | 0x80000000;
    toffset -= 2; // all tri offsets are 2 bytes to short!

    uint idx = 0, end = n_cubes;
    while ( idx < oi.oused )
    {
	u32 baseoff = idx * 4;
	noPRINT("ADJUST %x..%x, baseoff=%x\n",idx,end,baseoff);
	for ( ; idx < end; idx++ )
	{
	    u32 val = oi.olist[idx];
	    if (!val)
		val = null_offset - baseoff;
	    else if ( val & 0x80000000 )
		val += toffset - baseoff;
	    else
		val -= baseoff;
	    write_be32( oi.olist + idx, val );
	}
	end = idx + 8;
    }

    //HEXDUMP16(0,0,kcl->octree,kcl->octree_size); putchar('\n');


    //--- clean up and terminate

    //FREE(oi.olist); don't free this. it's already realloced!
    FREE(oi.tlist);
 #if SUPPORT_KCL_CUBE
    FREE(oi.cubelist);
 #endif
    kcl->octree_valid = true;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			flag support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanFlagFile
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname		// filename to scan
)
{
    DASSERT(kcl);
    DASSERT(fname);

    static u8  *data = 0;
    static uint data_size = 0;
    enumError err = OpenReadFILE(fname,0,true,&data,&data_size,0,0);
    if (!err)
    {
	KCL_ACTION_LOG("Scan flag file: %s\n",fname);
	FreeString(kcl->flag_fname);
	kcl->flag_fname = STRDUP(fname);

	ScanInfo_t si;
	InitializeSI(&si,(ccp)data,data_size,kcl->fname,kcl->revision);
	si.predef = SetupVarsKCL();

	while (NextCharSI(&si,true))
	{
	    //-- scan the group name

	    ScanFile_t *sf = si.cur_file;
	    DASSERT(sf);
	    char *ptr  = (char*)sf->ptr;
	    char *name = ptr;
	    bool have_wildcard = false;
	    while ( *ptr > ' ' && *ptr != '=' && *ptr != '#' )
	    {
		if ( !have_wildcard && strchr(PATTERN_WILDCARDS,*ptr) )
		    have_wildcard = true;
		*ptr = toupper((int)*ptr);
		ptr++;
	    }
	    const uint namelen = ptr - name;
	    sf->ptr = ptr;

	    if (!namelen)
	    {
		CheckEolSI(&si);
		continue;
	    }

	    DEFINE_VAR(flag);
	    if ( CheckWarnSI(&si,'=',0) || ScanExprSI(&si,&flag) )
	    {
		GotoEolSI(&si);
		continue;
	    }
	    CheckEolSI(&si);
	    int num = GetIntV(&flag);
	    if ( num <  0 )
		 num = -1; // -1 is the only allowed negative value


	    //--- here we have all data!

	    name[namelen] = 0;
	    noPRINT("FLAG-FOUND[%d]: |%s| = 0x%04x\n",have_wildcard,name,num);

	    FormatFieldItem_t *it
		= InsertFormatField(&kcl->flag_name,name,false,false,0);
	    DASSERT(it);
	    it->num = num;

	    if (have_wildcard)
	    {
		it = AppendFormatField(&kcl->flag_pattern,name,false,false);
		DASSERT(it);
		it->num = num;
	    }
	}

	const Var_t *var = FindVarSI(&si,"HEX4",false);
	if ( var && var->mode != VAR_UNSET )
	    kcl->accept_hex4 = GetBoolV(var);

	var = FindVarSI(&si,"HEX23",false);
	if ( var && var->mode != VAR_UNSET )
	    kcl->accept_hex23 = GetBoolV(var);

	ResetSI(&si);
    }

    FREE(data);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

void LoadFlagFileKCL
(
    kcl_t		* kcl		// pointer to valid KCL
)
{
    DASSERT(kcl);

    if ( !kcl->flag_db_loaded && kcl->fname )
    {
	kcl->flag_db_loaded = true;
	if ( !opt_flag_file || *opt_flag_file )
	{
	    enumError err = ERR_WARNING;
	    if (opt_flag_file)
	    {
		SubstDest(iobuf,sizeof(iobuf),kcl->fname,opt_flag_file,
			0,KCL_FLAG_EXT,false);
		err = ScanFlagFile(kcl,iobuf);
	    }

	    if (err)
	    {
		SubstDest(iobuf,sizeof(iobuf),kcl->fname,KCL_FLAG_FILE1,
			0,KCL_FLAG_EXT,false);
		err = ScanFlagFile(kcl,iobuf);
	    }

	    if (err)
	    {
		SubstDest(iobuf,sizeof(iobuf),kcl->fname,KCL_FLAG_FILE2,
			0,KCL_FLAG_EXT,false);
		err = ScanFlagFile(kcl,iobuf);
	    }

	    if (err)
	    {
		FreeString(kcl->flag_fname);
		kcl->flag_fname = 0;
		KCL_ACTION_LOG("No flag file found for: %s\n",kcl->fname);
	    }

	    PRINT("KCL.FLAGS loaded, N=%u+%u, hex4=%d, hex23=%d\n",
		kcl->flag_name.used, kcl->flag_pattern.used,
		kcl->accept_hex4, kcl->accept_hex23 );
	}
	else
	    KCL_ACTION_LOG("No flag file searched for: %s\n",kcl->fname);
    }
}

///////////////////////////////////////////////////////////////////////////////

int GetFlagByNameKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			name,		// upper case name to search
    int			new_value	// return value if not found
					// >0: used as new value

    // The name is searched in 4 steps:
    //   1. Search literal in 'flag_name' (ignore case)
    //   2. Search pattern in 'flag_pattern' (ignore case)
    //   3. Search pattern in 'flag_missing' (ignore case)
    //   4. Analyze the last 5 characters for '_ffff'
    //   5. Analyze the last 7 characters for '_tt_vvv'
    //   6. Use 'new_value'
)
{
    DASSERT(kcl);
    DASSERT(name);

    if (!kcl->flag_db_loaded)
	LoadFlagFileKCL(kcl);


    //--- 1. Search literal in 'flag_name' (ignore case)

    if ( kcl->flag_name.used )
    {
	FormatFieldItem_t *ptr = FindFormatField(&kcl->flag_name,name);
	if (ptr)
	{
	    noPRINT("GetFlagByNameKCL(%s) FOUND NAME -> %04x\n",name,ptr->num);
	    return (int)ptr->num;
	}
    }


    //--- 2. Search pattern in 'flag_pattern' (ignore case)

    if ( kcl->flag_pattern.used )
    {
	FormatFieldItem_t *ptr = MatchFormatField(&kcl->flag_pattern,name);
	if (ptr)
	{
	    noPRINT("GetFlagByNameKCL(%s) FOUND PATTERN-> %04x\n",name,ptr->num);
	    return (int)ptr->num;
	}
    }


    //--- 3. Search literal in 'flag_missing' (ignore case)

    if ( kcl->flag_missing.used )
    {
	FormatFieldItem_t *ptr = FindFormatField(&kcl->flag_missing,name);
	if (ptr)
	{
	    noPRINT("GetFlagByNameKCL(%s) FOUND MISSING -> %04x %s\n",name,ptr->num,ptr->key);
	    return (int)ptr->num;
	}
    }


    //--- flag is missed in flag file

    FormatFieldItem_t *it = InsertFormatField(&kcl->flag_missing,name,false,false,0);
    DASSERT(it);
    it->num = new_value > 0 ? new_value : 0x10000;
    PRINT("GetFlagByNameKCL(%s) INSERT INTO MISSING := %04x\n",name,it->num);


    //--- 4. Analyze the last 5-6 characters for '_ffff' or  '_Fffff

    const uint len = strlen(name);
    if ( kcl->accept_hex4 && len >= 5 && name[len-5] == '_'
	|| len >= 6 && name[len-6] == '_' && name[len-5] == 'F' )
    {
	char *end;
	const uint num = strtoul(name+len-4,&end,16);
	if (!*end)
	{
	    noPRINT("GetFlagByNameKCL(%s) FOUND '_FFFF' -> %04x\n",name,num);
	    it->num = num;
	    return num;
	}
    }


    //--- 5. Analyze the last 7 characters for '_tt_vvv'

    if ( kcl->accept_hex23
		&& len >= 7 && name[len-7] == '_'  && name[len-4] == '_' )
    {
	char *end1, *end2;
	const uint num1 = strtoul(name+len-6,&end1,16);
	const uint num2 = strtoul(name+len-3,&end2,16);
	if ( end1 == name+len-4 && !*end2 && num1 < 0x20 && num2 < 0x800 )
	{
	    noPRINT("GetFlagByNameKCL(%s) FOUND '_TT_VVV' -> %02x,%03x=%04x\n",
			name, num1, num2, num1 | num2 << 5 );
	    it->num = num1 | num2 << 5;
	    return it->num;
	}
    }


    //--- 6. Use 'new_value'

    return new_value;
}

///////////////////////////////////////////////////////////////////////////////

uint * AllocFlagCount()
{
    return CALLOC(NNN_KCL_FLAG,sizeof(uint));
}

///////////////////////////////////////////////////////////////////////////////

uint CountFlagsKCL
(
    // returns the total number of flag groups

    const kcl_t		* kcl,		// pointer to valid KCL
    uint		* count		// pointer to field with NNN_KCL_FLAG elements
)
{
    DASSERT(kcl);
    DASSERT(count);

    uint total = 0;
    memset(count,0,N_KCL_FLAG*sizeof(*count));

    const uint n = kcl->tridata.used;
    const kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    const kcl_tridata_t *end = td + n;
    for ( ; td < end; td++ )
    {
	uint idx = td->cur_flag < NNN_KCL_FLAG ? td->cur_flag : KCL_FLAG_UNKNOWN;
	if (!count[idx]++)
	    total++;
    }
    return total;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetGroupNameKCL ( uint kcl_flag, bool add_info )
{
    if ( kcl_flag < N_KCL_FLAG )
    {
	const uint bufsize = add_info ? 60 : 40;
	char *buf = GetCircBuf(bufsize);

	const uint type = kcl_flag & 0x1f;
	const kcl_class_t *cls = kcl_class + kcl_type[type].cls;
	if (add_info)
	    snprintf(buf,bufsize,"%s_%02x_F%04x (%s)",
			cls->name, type, kcl_flag, kcl_type[type].info );
	else
	    snprintf(buf,bufsize,"%s_%02x_F%04x",
			cls->name, type, kcl_flag );
	return buf;
    }

    if ( kcl_flag >= KCL_FLAG_AUTO_0 && kcl_flag <= KCL_FLAG_AUTO_MAX )
    {
	const uint bufsize = 9;
	char *buf = GetCircBuf(bufsize);
	snprintf( buf, bufsize, "auto_%03u",kcl_flag-KCL_FLAG_AUTO_0);
	return buf;
    }

    if ( kcl_flag >= KCL_FLAG_USER_0 && kcl_flag <= KCL_FLAG_USER_MAX )
    {
	Color_t col = user_color[kcl_flag-KCL_FLAG_USER_0];
	const uint bufsize = 14;
	char *buf = GetCircBuf(bufsize);
	snprintf( buf, bufsize, "user_%02x%02x%02x%02x",
			col.a, col.r, col.g, col.b );
	return buf;
    }

    return GetFlagGroupName(kcl_flag);
}

///////////////////////////////////////////////////////////////////////////////

uint GetExportFlagKCL ( uint kcl_flag )
{
    if ( kcl_flag < NN_KCL_FLAG )
	return kcl_flag;

    if ( kcl_flag >= KCL_FLAG_USER_0 && kcl_flag <= KCL_FLAG_USER_MAX )
	return kcl_flag - KCL_FLAG_USER_0 + 0x1000000;

    if ( kcl_flag >= KCL_FLAG_AUTO_0 && kcl_flag <= KCL_FLAG_AUTO_MAX )
	return kcl_flag - KCL_FLAG_AUTO_0 + 0x2000000;

    return kcl_flag | 0x8000000;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  ScanTextKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// FF_UNKNOWN or file format
)
{
    TRACE("ScanTextKCL()\n");

    DASSERT(kcl);
    ClearKCL(kcl,init_kcl);
// [[analyse-magic]]
    kcl->fform = fform != FF_UNKNOWN ? fform : GetByMagicFF(data,data_size,data_size);

    bool use_g      = 0 != ( KCL_MODE & KCLMD_G );
    bool use_usemtl = 0 != ( KCL_MODE & KCLMD_USEMTL );
    if ( !use_g && !use_usemtl )
    {
	if ( kcl->fform == FF_SKP_OBJ )
	     use_usemtl = true;
	else
	    use_g = true;
    }
    DASSERT( use_g || use_usemtl );
    KCL_ACTION_LOG("ScanTextKCL() %s [%s,%s]\n",
		kcl->fname,GetNameFF(kcl->fform,0),
		use_g && use_usemtl ? "'g'+'usemtl'" : use_g ? "'g'" : "'usemtl'" );


    //--- setup data

    double3List_t  vertex, normal;
    InitializeD3L(&vertex);
    InitializeD3L(&normal);


    //--- scan text file

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,kcl->fname,kcl->revision);
    si.predef = SetupVarsKCL();
    enumError max_err = ERR_OK;

    int auto_flag = KCL_FLAG_AUTO_0;
    char buf[200]; // for commands and group names

    struct tab_t
    {
	uint	idx;
	double3 pt;
    }
    tab_buf[KCL_MAX_PT_PER_TRI], *tab = tab_buf;
    uint tab_size = KCL_MAX_PT_PER_TRI;

    kcl_tri_param_t par;
    InitializeTriParam(&par,kcl,0);
    par.is_orig = true;
    uint bad_count = 0;

    static const uint norm_assign_23[] = { 0, 1, 2 };
    static const uint norm_assign_32[] = { 0, 2, 1 };
    const uint *norm_assign = KCL_MODE & KCLMD_IN_SWAP
				? norm_assign_32 : norm_assign_23;

    while (NextCharSI(&si,true))
    {
	if (!ScanNameSI(&si,buf,sizeof(buf),true,true,0))
	{
	    ccp eol = FindNextLineFeedSI(&si,true);
	    ScanFile_t *sf = si.cur_file;
	    DASSERT(sf);
	    ERROR0(ERR_WARNING,
			"Missing name [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	    if ( max_err < ERR_WARNING )
		max_err = ERR_WARNING;
	    GotoEolSI(&si);
	    continue;
	}

	enumError err = ERR_OK;
	CommandKCL_t cmd = GetCommandKCL(buf);
	switch (cmd)
	{
	 case KCMD_V:
	    noPRINT("V\n");
	    {
		double3 temp;
		err = ScanDoubleV3SI(&si,temp.v,1);
		if (!err)
		{
		    double3 *dest = AppendD3L(&vertex);
		    DASSERT(dest);
		    memcpy(dest,&temp,sizeof(*dest));
		}
	    }
	    break;

	 case KCMD_VN:
	    noPRINT("Vn\n");
	    {
		double3 temp;
		err = ScanDoubleV3SI(&si,temp.v,1);
		if (!err)
		{
		    double3 *dest = AppendD3L(&normal);
		    DASSERT(dest);
		    memcpy(dest,&temp,sizeof(*dest));
		}
	    }
	    break;

	 case KCMD_F:
	    {
		noPRINT("F\n");
		ScanFile_t *sf = si.cur_file;
		DASSERT(sf);

		//--- scan points

		uint k, norm = ~0;
		for ( k = 0;; k++ )
		{
		    if ( k >= 3 && !NextCharSI(&si,false) )
			break;

		    if ( k >= tab_size )
		    {
			const uint new_size = 2*tab_size;
			struct tab_t *new_tab = MALLOC(new_size*sizeof(*new_tab));
			memcpy(new_tab,tab,tab_size*sizeof(*tab));
			PRINT("KCMD_F/TAB: grow %d -> %d\n",tab_size,new_size);
			if ( tab != tab_buf )
			    FREE(tab);
			tab = new_tab;
			tab_size = new_size;
		    }

		    u32 idx;
		    err = ScanU32SI(&si,&idx,1,0);
		    if (err)
			break;
		    // transform to 0-based
		    tab[ k < 3 ? norm_assign[k] : k ].idx = idx - 1;

		    if ( *sf->ptr == '/' )
		    {
			// texture index found => ignore
			sf->ptr++;
			while ( *sf->ptr >= '0' && *sf->ptr <= '9' )
			    sf->ptr++;

			if ( *sf->ptr == '/' )
			{
			    // normal index found
			    sf->ptr++;
			    if ( *sf->ptr >= '0' && *sf->ptr <= '9'
				    && !ScanU32SI(&si,&idx,1,0)
				    && !k
				    && (KCL_MODE & KCLMD_AUTO) )
			    {
				PRINT("NORMAL FOUND: %u\n",idx-1);
				norm = idx - 1;
			    }
			}
		    }
		}

		//--- add point

		if ( !err && par.kcl_flag >= 0 )
		{
		    bool valid = true;
		    uint i;
		    for ( i = 0; i < k; i++ )
			if ( tab[i].idx > vertex.used )
			{
			    PRINT("INVALID VERTEX INDEX\n");
			    bad_count++;
			    valid = false;
			    break;
			}
			else
			    tab[i].pt = vertex.list[tab[i].idx];

		    if (valid)
		    {
			par.norm = norm < normal.used ? normal.list + norm : 0;
			AppendTrianglesKCL( &par, &tab[0].pt, sizeof(*tab), k );
		    }
		}
	    }
	    break;

	 case KCMD_G:
	    PRINT("G %.10s\n",si.cur_file->ptr);
	    if (use_g)
	    {
	     scan_group:
		ScanNameSI(&si,buf,sizeof(buf),true,true,0);
		par.kcl_flag = GetFlagByNameKCL(kcl,buf,auto_flag);
		if ( par.kcl_flag == auto_flag )
		{
		    if ( ++auto_flag > KCL_FLAG_AUTO_MAX )
			auto_flag = KCL_FLAG_AUTO_0;
		    PRINT("KCL-FLAG set to %04x: %s\n",par.kcl_flag,buf);
		}
	    }
	    GotoEolSI(&si);
	    break;

	 case KCMD_USEMTL:
	    noPRINT("USEMTL\n");
	    if (use_usemtl)
		goto scan_group;
	    GotoEolSI(&si);
	    break;

	 case KCMD_TRI:
	    {
		noPRINT("TRI\n");
		ScanFile_t *sf = si.cur_file;
		DASSERT(sf);

		//--- scan flag

		u32 flag;
		err = ScanU32SI(&si,&flag,1,0);


		//--- scan points

		uint k;
		for ( k = 0; k < KCL_MAX_PT_PER_TRI && !err; k++ )
		{
		    noPRINT(">> %u: %.8s\n",k,sf->ptr);
		    char ch = NextCharSI(&si,false);
		    if ( ch == '|' )
		    {
			sf->ptr++;
			ch = NextCharSI(&si,false);
		    }
		    if (!ch)
			break;
		    err = ScanDoubleSI(&si, tab[ k < 3 ? norm_assign[k] : k ].pt.v, 3 );
		}


		//--- add point

		if ( !err && k >= 3 )
		{
		    par.norm = 0;
		    AppendTrianglesKCL( &par, &tab[0].pt, sizeof(*tab), k );
		}
	    }
	    break;

	  default:
	    TRACE("IGNORE KCL/OBJ COMMAND %s\n",buf);
	    GotoEolSI(&si);
	    break;
	}

	if (err)
	{
	    if ( max_err < err )
		max_err = err;
	    GotoEolSI(&si);
	}
	else
	    CheckEolSI(&si);
    }

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextKCL) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);


    //--- logging

   if ( !(KCL_MODE & KCLMD_SILENT)
	&& ( verbose > 0 || verbose >=0 && par.swap_count ) )
    {
	fprintf(stdlog,
		"  - %u triangles read,"
		" points #2 and #3 are swapped for %u triangles.\n",
		kcl->tridata.used, par.swap_count );
    }

    if (bad_count)
    {
	const uint total = bad_count + kcl->tridata.used;
	ERROR0(ERR_WARNING,
	    "%u of %u (%4.2f%%) triangles ignored because of wrong indices: %s\n",
	    bad_count, total,
	    100.0 * bad_count / total,
	    kcl->fname );
	if ( max_err < ERR_WARNING )
	     max_err = ERR_WARNING;
    }


    //--- terminate

    if ( tab != tab_buf )
	FREE(tab);
    ResetD3L(&vertex);
    ResetD3L(&normal);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    ScanRawKCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    bool		use_data	// true: data is valid on 'kcl' live time
)
{
    DASSERT(kcl);
    ClearKCL(kcl,init_kcl);

    kcl_analyze_t ka;
    if ( IsValidKCL(&ka,data,data_size,data_size,kcl->fname) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid KCL file: %s\n", kcl->fname ? kcl->fname : "?");
    }

    KCL_ACTION_LOG("ScanRawKCL() %s\n",kcl->fname);
    kcl->fform = FF_KCL;

    if (!ka.order_ok)
	ERROR0(ERR_WARNING,
		"KCL: Unusual section order: %d\n",ka.order_value);


    //--- save raw data for optimized handling

    const bool store_raw_data
	= ( KCL_MODE & (KCLMD_NEW|KCLMD_DROP_UNUSED) ) != KCLMD_NEW;
    PRINT("STORE_RAW_DATA=%d\n",store_raw_data);
    if (store_raw_data)
    {
	if (use_data)
	{
	    kcl->raw_data = (u8*)data;
	    kcl->raw_data_alloced = false;
	}
	else
	{
	    kcl->raw_data = MEMDUP(data,data_size);
	    kcl->raw_data_alloced = true;
	}
	kcl->raw_data_size = data_size;
    }


    //--- transfer header

    const kcl_head_t *kclhead = data;
    kcl->min_octree.x	= bef4(kclhead->min_octree+0);
    kcl->min_octree.y	= bef4(kclhead->min_octree+1);
    kcl->min_octree.z	= bef4(kclhead->min_octree+2);
    kcl->mask[0]	= be32(kclhead->mask+0);
    kcl->mask[1]	= be32(kclhead->mask+1);
    kcl->mask[2]	= be32(kclhead->mask+2);
    kcl->coord_rshift	= be32(&kclhead->coord_rshift);
    kcl->y_lshift	= be32(&kclhead->y_lshift);
    kcl->z_lshift	= be32(&kclhead->z_lshift);
    kcl->unknown_0x10	= bef4(&kclhead->unknown_0x10);
    kcl->unknown_0x38	= bef4(&kclhead->unknown_0x38);


    //--- store triangles

    const uint n_vert = ka.n[0];
    const uint n_norm = ka.n[1];
    const uint n_tri  = ka.n[2];
    PRINT("N(vert)=%u, N(norm)=%u, N(tri)=%u\n",n_vert,n_norm,n_tri);

    const float3 *vert = (float3*)( data + ka.off[0] );
    const float3 *norm = (float3*)( data + ka.off[1] );
    const kcl_triangle_t *tri = (kcl_triangle_t*)( data + ka.off[2] );

    kcl_tridata_t *td = GrowListSize(&kcl->tridata,n_tri,100);
    DASSERT( kcl->tridata.used == n_tri );
    memset(td,0,sizeof(*td)*n_tri);

    double3 min, max; // min and max values of all incomming vertex points
    min.x = min.y = min.z = -1000.0;
    max.x = min.y = max.z =  1000.0;

    uint ti;
    for ( ti = 0; ti < n_tri; ti++, td++, tri++ )
    {
	u16 in[6];
	be16n(in,&tri->idx_vertex,6);

	if ( in[0] < n_vert )
	{
	    const float *flt = vert[in[0]].v;
	    td->pt[0].x = bef4(flt++);
	    td->pt[0].y = bef4(flt++);
	    td->pt[0].z = bef4(flt);
	    MinMax3(&min,&max,td->pt,1);
	    // [[xtridata]]
	}

	uint p;
	for ( p = 0; p < 4; p++ )
	    if ( in[p+1] < n_norm )
		bef4n(td->normal[p].v,norm[in[p+1]].v,3);

	td->length = bef4(&tri->length);

	td->in_flag  = in[5];
	td->cur_flag = patch_kcl_flag ? patch_kcl_flag[in[5]] : in[5];
	PRINT_IF ( td->in_flag!=td->cur_flag,
		"IN/FLAGS: %04x -> %04x\n",td->in_flag,td->cur_flag);
    }


    //--- calc 'tri_minval' and 'tri_maxval'

    const bool clip = ( KCL_MODE & KCLMD_CLIP ) != 0;
    if (clip)
    {
	double temp;
	temp = ( max.x - min.x ) * KCL_CLIP.x;
	kcl->tri_minval.x = min.x - temp;
	kcl->tri_maxval.x = max.x + temp;

	temp = ( max.y - min.y ) * KCL_CLIP.y;
	kcl->tri_minval.y = min.y - temp;
	kcl->tri_maxval.y = max.y + temp;

	temp = ( max.z - min.z ) * KCL_CLIP.z;
	kcl->tri_minval.z = min.z - temp;
	kcl->tri_maxval.z = max.z + temp;

	TRACE("VERTEX-MIN: %12.6g %12.6g %12.6g\n",
	    kcl->tri_minval.x, kcl->tri_minval.y, kcl->tri_minval.z );
	TRACE("VERTEX-MAX: %12.6g %12.6g %12.6g\n",
	    kcl->tri_maxval.x, kcl->tri_maxval.y, kcl->tri_maxval.z );
    }


    //--- calc triangle points

    CalcPointsTriData( (kcl_tridata_t*)kcl->tridata.list, n_tri, kcl, clip );
    kcl->norm_valid = true;


    //--- test signature

    {
	ccp ptr = (ccp)&vert->x;
	ccp end = ptr + 8;
	for(;;)
	{
	    char ch = *ptr++;
	    noPRINT("|%02x|\n",ch);
	    if ( ch <= 0x20 || ch >= 0x7f )
		break;

	    if ( ptr == end )
	    {
		float sig = bef4(&vert->z);
		if ( sig >= 0.01 && sig <= 1000.0 )
		{
		    kcl->signature = sig;
		    memcpy(kcl->signature_info,&vert->x,8);
		}
		break;
	    }
	}
    }


    //--- store octree

    if ( store_raw_data && ka.off[3] < data_size )
    {
	kcl->octree_size = data_size - ka.off[3];
	if (kcl->raw_data)
	{
	    kcl->octree = kcl->raw_data + ka.off[3];
	    kcl->octree_alloced = false;
	}
	else
	{
	    kcl->octree = MEMDUP( data+ka.off[3], kcl->octree_size );
	    kcl->octree_alloced = true;
	}
	kcl->octree_valid = true;
    }


    //--- terminate

    //PRINT("KCL-N: %u %u %u\n",n_vert,n_norm,kcl->triangle.used);
    kcl->min_max_valid	= false;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KCL: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    bool		use_data,	// true: data is valid on 'kcl' live time
    CheckMode_t		mode		// not NULL: call CheckKCL(mode)
)
{
    DASSERT(kcl);
    PRINT("ScanKCL(mode=%x) size=%u\n",mode,data_size);
    //HEXDUMP16(0,0,data,16);

    enumError err;
// [[analyse-magic]]
//    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    switch (fform)
    {
	case FF_KCL:
	    err = ScanRawKCL(kcl,init_kcl,data,data_size,use_data);
	    break;

	case FF_KCL_TXT:
	case FF_WAV_OBJ:
	case FF_SKP_OBJ:
	    err = ScanTextKCL(kcl,init_kcl,data,data_size,fform);
	    break;

	default:
	    ClearKCL(kcl,init_kcl);
	    return ERROR0(ERR_INVALID_DATA,
		"No KCL or OBJ file: %s\n", kcl->fname ? kcl->fname : "?");
    }
    kcl->fform = fform;
    const bool rm_octree = kcl->octree_valid && KCL_MODE & KCLMD_NEW;
    PRINT("NEW->RM(OCTREE): %u && %u => %u\n",
		kcl->octree_valid, (KCL_MODE & KCLMD_NEW)!=0, rm_octree );


    CheckMetricsTriData( (kcl_tridata_t*)kcl->tridata.list, kcl->tridata.used, kcl );

    if ( disable_patch_on_load <= 0 )
	PatchKCL(kcl);

    if ( err <= ERR_WARNING && mode )
	CheckKCL(kcl,mode);

    if (rm_octree)
    {
	KCL_ACTION_LOG("Remove Octree\n");

	if (kcl->octree_alloced)
	    FREE(kcl->octree);
	kcl->octree = 0;
	kcl->octree_size = 0;
	kcl->octree_alloced = false;
	kcl->octree_valid = false;
    }
    kcl->recreate_octree = ( KCL_MODE & KCLMD_NEW ) != 0;

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    struct raw_data_t	* raw,		// valid raw data
    bool		move_data,	// true: move 'raw.data' to 'kcl'
    CheckMode_t		check_mode	// not NULL: call CheckKCL(check_mode)
)
{
    DASSERT(kcl);
    DASSERT(raw);
    if (init_kcl)
	InitializeKCL(kcl);
    else
	ResetKCL(kcl);

    kcl->fatt  = raw->fatt;
    kcl->fname = raw->fname;
    raw->fname = 0;

    if ( raw->is_0 )
    {
	if ( disable_patch_on_load <= 0 )
	    PatchKCL(kcl);
	return ERR_OK;
    }

    enumError err = ScanKCL(kcl,false,raw->data,raw->data_size,move_data,check_mode);

    if ( kcl->raw_data == raw->data )
    {
	// move the data
	kcl->raw_data_alloced = raw->data_alloced;
	raw->data_alloced = false;
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadKCL
(
    kcl_t		* kcl,		// KCL data structure
    bool		init_kcl,	// true: initialize 'kcl' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckKCL(mode)
)
{
    DASSERT(kcl);
    DASSERT(fname);
    if (init_kcl)
	InitializeKCL(kcl);
    else
	ResetKCL(kcl);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	kcl->fname = raw.fname;
	raw.fname = 0;
	err = ScanKCL(kcl,false,raw.data,raw.data_size,true,mode);
	if ( kcl->raw_data == raw.data )
	{
	    // move the data
	    kcl->raw_data_alloced = raw.data_alloced;
	    raw.data_alloced = false;
	}
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Reference KCL			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp kcl_ref_fname = 0;
ccp kcl_auto_ref_fname = 0;

int ScanOptLoadKcl ( ccp arg )
{
    kcl_ref_fname = arg;
    SetupReferenceKCL(0);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void UnloadAutoReferenceKCL()
{
    if ( !kcl_ref_fname && kcl_ref )
    {
	// unload only auto loaded kcl
	PRINT("UNLOAD AUTO KCL\n");
	ResetKCL(kcl_ref);
	FREE(kcl_ref);
	kcl_ref = 0;
    }

    FreeString(kcl_auto_ref_fname);
    kcl_auto_ref_fname = 0;
}

///////////////////////////////////////////////////////////////////////////////

void DefineAutoLoadReferenceKCL ( ccp fname, ccp ext )
{
    DASSERT(fname);
    if ( kcl_ref_fname || !fname )
	return;

    UnloadAutoReferenceKCL();

    char buf[PATH_MAX];
    char *dest = StringCopyS(buf,sizeof(buf),fname);
    bool done = false;
    if (ext)
    {
	uint elen = strlen(ext);
	if ( elen < dest - buf && !strcasecmp(dest-elen,ext) )
	{
	    dest -= elen;
	    done = true;
	}
    }

    if (!done)
    {
	char *pt = strrchr(buf,'.');
	if (pt)
	    dest = pt;
    }

    StringCopyE(dest,buf+sizeof(buf),".kcl");
    kcl_auto_ref_fname = STRDUP(buf);
    PRINT("NEW AUTO KCL: %s\n",kcl_auto_ref_fname);
}

///////////////////////////////////////////////////////////////////////////////

kcl_t * LoadReferenceKCL()
{
    bool silent;
    ccp path;
    if (kcl_ref_fname)
    {
	path = kcl_ref_fname;
	silent = false;
    }
    else
    {
	path = kcl_auto_ref_fname;
	silent = true;
    }

    if ( kcl_ref || !path || !*path )
	return kcl_ref;

    PRINT("GetReferenceKCL() %s\n",path);

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,path,"course.kcl",silent,0);
    if (err)
    {
	ResetRawData(&raw);
	FreeString(kcl_auto_ref_fname);
	kcl_ref_fname = kcl_auto_ref_fname = 0;
	return 0;
    }

    kcl_t kcl;
    InitializeKCL(&kcl);
    //kcl.fform_outfile = FF_KCL;
    err = ScanRawDataKCL(&kcl,false,&raw,false,0);
    ResetRawData(&raw);
    if (err)
    {
	ResetKCL(&kcl);
	FreeString(kcl_auto_ref_fname);
	kcl_ref_fname = kcl_auto_ref_fname = 0;
	return 0;
    }

    PRINT("REF-KCL loaded!\n");
    kcl_ref = MALLOC(sizeof(*kcl_ref));
    memcpy(kcl_ref,&kcl,sizeof(*kcl_ref));

    return kcl_ref;
}

///////////////////////////////////////////////////////////////////////////////

kcl_t * CreateReferenceKCL()
{
    kcl_t *kcl = LoadReferenceKCL();
    if (kcl)
    {
	kcl_ref = 0;
	return kcl;
    }

    kcl = MALLOC(sizeof(*kcl));
    InitializeKCL(kcl);
    return kcl;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int kcl_fall_flag = -1;

static enumError F_kcl_fall
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    ToVectorV(param);
    double y = param->y;
    kcl_t *kcl = GetReferenceKCL();
    if ( !kcl && si )
	kcl = si->kcl;
    if (kcl)
    {
	u32 typemask = n_param > 2 && param[2].mode != VAR_UNSET
			? GetIntV(param+2)
			: M1(typemask);
	double temp = FallKCL(kcl,0,param->d3,
				GetDoubleV(param+1),0,typemask,&kcl_fall_flag);
	if ( temp >= 0.0 )
	    y = temp;
    }

    res->x = param->x;
    res->y = y;
    res->z = param->z;
    res->mode = VAR_VECTOR;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_kcl_fall_flag
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    res->i    = kcl_fall_flag;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_isKCL
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);

    uint result = 0;
    if ( si && si->kcl )
    {
	result = 1;

	if (si->kcl->fname)
	{
	    ccp fname = strrchr(si->kcl->fname,'/');
	    if (fname)
		fname++;
	    else
		fname = si->kcl->fname;

	    if ( !strcasecmp(fname,"course.kcl")
		|| !strcasecmp(fname,"course.txt")
		|| !strcasecmp(fname,"course.txt.kcl") )
	    {
		result = 2;
	    }
	}
    }

    res->i    = result;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static const struct FuncTable_t kcl_ref_func_tab[] =
{
    { 2, 3, "KCL$FALL", F_kcl_fall, 0,
	"vector", "kcl$fall(pt,width[,typemask])",
	" If a reference KCL is defined, search the lowest point"
	" below 'pt', that can be reached without collision."
	" The point is expanded to a cube with edge length 'width'"
	" for the collision tests."
	" If 'typemask' is set, only KCL types with related bit number set"
	" are recognized."
	" Use kcl$fallFlag() to get the corespondent KCL flag." },

    { 0, 0, "KCL$FALLFLAG", F_kcl_fall_flag, 0,
	"int", "kcl$fallFlag()",
	" This functions returns the corespondent KCL flag of the last"
	" call of kcl$fall()."
	" The return value is the KCL flag of the colliding triangle,"
	" or -1 if no collision was found." },

    { 0, 0, "ISKCL", F_isKCL, 0,	// replace only function call
	"int", "isKCL()", 0 },		// but not info

    {0,0,0,0,0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

void SetupReferenceKCL
(
    VarMap_t *vm	// not NULL: define KCL variables here
)
{
    static bool done = false;
    if (!done)
    {
	done = true;
	DefineParserFuncTab(kcl_ref_func_tab,0);
    }

    if (vm)
    {
	char name[50];

	const kcl_attrib_name_t *an;
	for ( an = kcl_attrib_name; an->name; an++ )
	{
	    u32 typemask = 0;
	    const kcl_type_t *kt;
	    for ( kt = kcl_type; kt->info; kt++ )
		if ( kt->attrib & an->attrib )
		    typemask |= 1ul << kt->type;

	    StringCat2S(name,sizeof(name),"KCL$",an->name);
	    DefineIntVar(vm,name,typemask);
	}

	uint i;
	for ( i = N_KCL_FLAG; i < NN_KCL_FLAG; i++ )
	{
	    StringCat2S(name,sizeof(name),"FLAG$",GetFlagName(i));
	    DefineIntVar(vm,name,i);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateRawKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError FastCreateRawKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    bool		add_sig		// true: add signature
)
{
    DASSERT(kcl);
    DASSERT(!kcl->model_modified);
    DASSERT(kcl->raw_data);
    DASSERT(kcl->raw_data_size);


    //--- setup section offsets

    const kcl_head_t * kclhead = (kcl_head_t*)kcl->raw_data;
    u32 sect_off[N_KCL_SECT];
    DASSERT( sizeof(sect_off) == sizeof(kclhead->sect_off) );

    uint i;
    for ( i = 0; i < N_KCL_SECT; i++ )
	sect_off[i] = be32(kclhead->sect_off+i);
    sect_off[2] += 0x10;


    //--- compare number triangles

    const uint n_tri  = ( sect_off[3] - sect_off[2] ) / sizeof(kcl_triangle_t);
    KCL_ACTION_LOG("FastCreateRawKCL() N=%u\n",n_tri);
    if ( n_tri != kcl->tridata.used )
    {
	PRINT("N-TRI: %u != %u\n",n_tri,kcl->tridata.used );
	kcl->model_modified = true;
	return CreateRawKCL(kcl,add_sig);
    }


    //--- store modified flags

    kcl_triangle_t *tri = (kcl_triangle_t*)( kcl->raw_data + sect_off[2] );
    kcl_tridata_t  *td  = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t  *end = td + n_tri;

    for ( ; td < end; td++, tri++ )
	write_be16(&tri->flag,td->cur_flag);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static void AddSignature ( float3List_t *vertex, bool fast )
{
    // the first normal is always a signature
    float3 sig;
    memcpy(&sig,KCL_SIGNATURE,8);
    sig.x = bef4(&sig.x);
    sig.y = bef4(&sig.y);
    sig.z = strtod(VERSION_NUM,0);
    FindInsertFloatF3L(vertex,&sig,fast);
}

///////////////////////////////////////////////////////////////////////////////

uint FindInsertFloatWithMask ( float3List_t *f3l, float3 *val, u32 mask )
{
    float3 temp;
    temp.u[0] = val->u[0] & mask;
    temp.u[1] = val->u[1] & mask;
    temp.u[2] = val->u[2] & mask;
    return FindInsertFloatF3L(f3l,&temp,false);
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateRawKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    bool		add_sig		// true: add signature
)
{
    DASSERT(kcl);
    TRACE("CreateRawKCL()\n");

    if (   !kcl->model_modified
	&& kcl->raw_data
	&& kcl->raw_data_size
	&& kcl->octree_valid )
    {
	return FastCreateRawKCL(kcl,add_sig);
    }

    if ( CreateOctreeKCL(kcl) == ERR_OK )
	add_sig = true;

    KCL_ACTION_LOG("CreateRawKCL()\n");

    //--- calculate vetex and normal lists

    CalcNormalsKCL(kcl,false);

    static const int round_tab[] = { 17, 14, 11, 8, 6, 4, 3, 2, 1, 0 };
    const int *round_ptr = KCL_MODE & KCLMD_ROUND ? round_tab+1 : round_tab;
    uint start_normals = 0, round_steps = 0;

    const int tiny_level = ( KCL_MODE & KCLMD_M_TINY ) >> KCLMD_S_TINY;
    const tiny_param_t *tp = TinyParam + tiny_level;
    const uint n_tri = kcl->tridata.used;
    const bool fast  = kcl->fast && n_tri < 0x4000 && !tp->kcl_round;
    float3List_t vertex, normal;

    // vertex + 4*normal + flag
    typedef u16 vertex_index_t;
    vertex_index_t *index_list = CALLOC(6*n_tri,sizeof(*index_list));

    kcl_tridata_t *td_base = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t *td, *td_end = td_base + n_tri;

    for(;;)
    {
	InitializeF3L(&vertex,n_tri);
	InitializeF3L(&normal,n_tri*4);

	vertex_index_t *idx = index_list;
	if (tp->kcl_mask)
	{
	    for ( td = td_base; td < td_end; td++ )
	    {
		float3 temp;
		temp.x = RoundF(td->pt->x,tp->kcl_round);
		temp.y = RoundF(td->pt->y,tp->kcl_round);
		temp.z = RoundF(td->pt->z,tp->kcl_round);
		*idx++ = FindInsertFloatF3L(&vertex,&temp,false);

		*idx++ = FindInsertFloatWithMask(&normal,td->normal+0,tp->kcl_mask);
		*idx++ = FindInsertFloatWithMask(&normal,td->normal+1,tp->kcl_mask);
		*idx++ = FindInsertFloatWithMask(&normal,td->normal+2,tp->kcl_mask);
		*idx++ = FindInsertFloatWithMask(&normal,td->normal+3,tp->kcl_mask);

		*idx++ = td->cur_flag;
	    }
	}
	else
	{
	    if ( add_sig && !tiny_level )
		AddSignature(&vertex,fast);

	    for ( td = td_base; td < td_end; td++ )
	    {
		float3 temp;
		temp.x = td->pt->x;
		temp.y = td->pt->y;
		temp.z = td->pt->z;
		*idx++ = FindInsertFloatF3L(&vertex,&temp,fast);

		*idx++ = FindInsertFloatF3L(&normal,td->normal+0,fast);
		*idx++ = FindInsertFloatF3L(&normal,td->normal+1,fast);
		*idx++ = FindInsertFloatF3L(&normal,td->normal+2,fast);
		*idx++ = FindInsertFloatF3L(&normal,td->normal+3,fast);

		*idx++ = td->cur_flag;
	    }
	}

	if ( normal.used < 0x10000 )
	{
	    if (start_normals)
	    {
		KCL_ACTION_LOG("Number of normals reduced from %u to %u [steps=%u]\n",
			start_normals, normal.used, round_steps );
	    }
	    break;
	}

	if (!start_normals)
	    start_normals = normal.used;
	round_steps++;

	const int round = *round_ptr++;
	if (!round)
	{
	    return ERROR0(ERR_INVALID_DATA,
		"Unable to limit normals list to <65536 elements (have %u): %s\n",
		normal.used, kcl->fname );
	}

 #if defined(TEST) || 0
	KCL_ACTION_LOG("Have %u normals: try to reduce to 65535 (level %u)\n",
		normal.used, round );
 #endif

	for ( td = td_base; td < td_end; td++ )
	{
	    // need cast to avoid gcc warning 'above array bounds'
	    float *f = (float*)&td->normal->x;
	    float *f_end = (float*)( (u8*)f + sizeof(td->normal) );
	    while ( f < f_end )
	    {
		*f = ldexpf(truncf(ldexpf(*f,round)),-round);
		f++;
	    }
	}
    }


    //--- calculate data size

    const uint vertex_size	= sizeof(float3) * vertex.used;
    const uint normal_size	= sizeof(float3) * normal.used;
    const uint triangle_size	= sizeof(kcl_triangle_t) * n_tri;

    uint data_size = sizeof(kcl_head_t)
		   + vertex_size
		   + normal_size
		   + triangle_size
		   + kcl->octree_size;


    //--- alloc data

    u8 *old_data = 0;
    if (kcl->raw_data_alloced)
	old_data = kcl->raw_data; // octree may use this data

    kcl_head_t * kclhead = CALLOC(1,data_size);
    kcl->raw_data = (u8*)kclhead;
    kcl->raw_data_size = data_size;
    kcl->raw_data_alloced = true;

    PRINT("CREATE KCL: size = %#x = %u\n",data_size,data_size);


    //--- write header

    const uint vertex_offset	= sizeof(kcl_head_t);
    const uint normal_offset	= vertex_offset + vertex_size;
    const uint triangle_offset	= normal_offset + normal_size;
    const uint octree_offset	= triangle_offset + triangle_size;

    write_be32(kclhead->sect_off+0,	vertex_offset);
    write_be32(kclhead->sect_off+1,	normal_offset);
    write_be32(kclhead->sect_off+2,	triangle_offset - 0x10 );
    write_be32(kclhead->sect_off+3,	octree_offset);

    write_bef4(kclhead->min_octree+0,	kcl->min_octree.x);
    write_bef4(kclhead->min_octree+1,	kcl->min_octree.y);
    write_bef4(kclhead->min_octree+2,	kcl->min_octree.z);
    write_be32(kclhead->mask+0,		kcl->mask[0]);
    write_be32(kclhead->mask+1,		kcl->mask[1]);
    write_be32(kclhead->mask+2,		kcl->mask[2]);
    write_be32(&kclhead->coord_rshift,	kcl->coord_rshift);
    write_be32(&kclhead->y_lshift,	kcl->y_lshift);
    write_be32(&kclhead->z_lshift,	kcl->z_lshift);
    write_bef4(&kclhead->unknown_0x10,	kcl->unknown_0x10);
    write_bef4(&kclhead->unknown_0x38,	kcl->unknown_0x38);


    //--- transfer verticies

    float *src  = vertex.list->v;
    float *dest = (float*)(kcl->raw_data + vertex_offset);
    write_bef4n(dest,src,3*vertex.used);


    //--- transfer normals

    src  = normal.list->v;
    dest = (float*)(kcl->raw_data + normal_offset);
    write_bef4n(dest,src,3*normal.used);


    //--- transfer triangles

    const vertex_index_t *idx = index_list;
    kcl_triangle_t *tdest = (kcl_triangle_t*)( kcl->raw_data + triangle_offset );
    for ( td = td_base; td < td_end; td++, tdest++, idx+=6 )
    {
	write_bef4(&tdest->length,td->length);
	write_be16n(&tdest->idx_vertex,idx,6);
    }


    //--- transfer octree

    DASSERT( octree_offset + kcl->octree_size == kcl->raw_data_size );
    memcpy( kcl->raw_data+octree_offset, kcl->octree, kcl->octree_size );


    //--- terminate

    ResetF3L(&vertex);
    ResetF3L(&normal);
    FREE(index_list);

    kcl->model_modified = false;

    if (!kcl->octree_alloced)
    {
	// is part of 'old_data'
	kcl->octree = 0;
	kcl->octree_size = 0;
	kcl->octree_alloced = false;
    }
    FREE(old_data);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    SaveRawKCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(kcl);
    DASSERT(fname);
    PRINT("SaveRawKCL(%s,%d)\n",fname,set_time);

    //--- create raw data

    enumError err = CreateRawKCL(kcl,true);
    if (err)
	return err;
    DASSERT(kcl->raw_data);
    DASSERT(kcl->raw_data_size);
    KCL_ACTION_LOG("SaveRawKCL(%s) N=%u, model_modified=%d\n",
		fname, kcl->tridata.used, kcl->model_modified );

    //--- write to file

    File_t F;
    err = CreateFileOpt(&F,true,fname,testmode,kcl->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&kcl->fatt,0);

    if ( fwrite(kcl->raw_data,1,kcl->raw_data_size,F.f) != kcl->raw_data_size )
	err = FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTextKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    // use DOS/Windows line format -> unix can handle it ;)
    // obj file format:
    //     http://en.wikipedia.org/wiki/Wavefront_.obj_file
    //     http://www.martinreddy.net/gfx/3d/OBJ.spec

    DASSERT(kcl);
    DASSERT(fname);

    KCL_ACTION_LOG("SaveTextKCL(%s) N=%u, model_modified=%d\n",
		fname, kcl->tridata.used, kcl->model_modified );

    static ccp sep  = "#\f\r\n#-----------------------------------------------\r\n";
    static ccp head = "\r\n%s# %u %s\r\n\r\n";

    if ( KCL_MODE & KCLMD_NORMALS )
	CalcNormalsKCL(kcl,false);

    const uint n_tri = kcl->tridata.used;
    uint p, n_vert, n_norm, n_valid_tri = 0;

    uint *flag_count = AllocFlagCount();
    typedef u32 vertex_index_t;
    vertex_index_t *index_list = CALLOC(4*n_tri,sizeof(*index_list));
    kcl_tridata_t *td_base = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t *td, *td_end = td_base + n_tri;

    float3List_t vertex, normal;
    const bool print_tri = ( KCL_MODE & KCLMD_TRIANGLES ) != 0;
    if ( print_tri || kcl->fast )
    {
	InitializeF3L(&vertex,0);
	InitializeF3L(&normal,0);

	// setup of 'index_list' is delayed;

	for ( td = td_base; td < td_end; td++ )
	    if ( !(td->status & TD_INVALID) )
		n_valid_tri++;

	n_vert = print_tri ? 0 : 3 * n_valid_tri;
	n_norm = !print_tri && KCL_MODE & KCLMD_NORMALS ? n_valid_tri : 0;
    }
    else
    {
	InitializeF3L(&vertex,n_tri*3);
	InitializeF3L(&normal,n_tri);

	vertex_index_t *idx = index_list;
	for ( td = td_base; td < td_end; td++ )
	{
	    if ( !(td->status & TD_INVALID) )
	    {
		n_valid_tri++;
		for ( p = 0; p < 3; p++ )
		{
		    float3 temp;
		    temp.x = td->pt[p].x;
		    temp.y = td->pt[p].y;
		    temp.z = td->pt[p].z;
		    *idx++ = FindInsertFloatF3L(&vertex,&temp,false) + 1;
		}
		*idx++ = ( KCL_MODE & KCLMD_NORMALS )
			    ? FindInsertFloatF3L(&normal,td->normal+0,false) + 1
			    : 0;
	    }
	}
	n_vert = vertex.used;
	n_norm = normal.used;
    }


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,kcl->fname);
    if ( err > ERR_WARNING || !F.f )
	goto abort;
    SetFileAttrib(&F.fatt,&kcl->fatt,0);

    const uint n_grp = CountFlagsKCL(kcl,flag_count);
    PRINT("n_vert=%u, n_norm=%u, n_tri=%u/%u, n_grp=%u\n",
		n_vert, n_norm, n_valid_tri, n_tri, n_grp );


    //--- print header

    fprintf(F.f, text_kcl_head_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE, GetKclMode(),
		n_vert, n_norm, n_valid_tri, n_grp );

    uint i;
    for ( i = 0; i <= N_KCL_TYPE; i++ )
    {
	int j, j_max, j_inc;
	if ( i < N_KCL_TYPE )
	{
	    j     = i;
	    j_max = N_KCL_FLAG;
	    j_inc = N_KCL_TYPE;
	}
	else
	{
	    j     = N_KCL_FLAG;
	    j_max = KCL_FLAG_USER_0 + n_user_color;
	    j_inc = 1;
	}
	for ( ; j < j_max; j += j_inc )
	    if (flag_count[j])
		fprintf(F.f,"# %8u triangles in group %s\r\n",
			flag_count[j], GetGroupNameKCL(j,true) );
    }

    char mtl_fname[PATH_MAX];
    const bool have_mtl = !print_tri && !F.is_stdio
		&& KCL_MODE & (KCLMD_MTL|KCLMD_WIIMM);
    if (have_mtl)
    {
	char *dest = StringCopyS(mtl_fname,sizeof(mtl_fname),fname);
	if ( KCL_MODE & KCLMD_WIIMM )
	{
	    dest = strrchr(mtl_fname,'/');
	    dest = dest ? dest+1 : mtl_fname;
	    StringCopyE(dest,mtl_fname+sizeof(mtl_fname),"wiimm" KCL_MTL_EXT);
	    fprintf(F.f,"\r\n\r\nmtllib %s\r\n",dest);
	}
	else
	{
	    char *found = strrchr(mtl_fname,'.');
	    if ( found && !strcasecmp(found,".obj") )
		dest = found;
	    StringCopyE(dest,mtl_fname+sizeof(mtl_fname),KCL_MTL_EXT);

	    found = strrchr(mtl_fname,'/');
	    found = found ? found+1 : mtl_fname;
	    fprintf(F.f,"\r\n\r\nmtllib %s\r\n",found);
	}
    }


    //--- print vertices

    if (n_vert)
    {
	fprintf(F.f,head,sep,n_vert,"vertices");

	if (kcl->fast)
	{
	    vertex_index_t *idx = index_list;
	    uint vidx = 1, nidx = 1;
	    for ( td = td_base; td < td_end; td++ )
	    {
		if ( !(td->status & TD_INVALID) )
		{
		    fprintf(F.f,"\r\n"
			"v %11.3f %11.3f %11.3f\r\n"
			"v %11.3f %11.3f %11.3f\r\n"
			"v %11.3f %11.3f %11.3f\r\n",
			td->pt[0].x, td->pt[0].y, td->pt[0].z,
			td->pt[1].x, td->pt[1].y, td->pt[1].z,
			td->pt[2].x, td->pt[2].y, td->pt[2].z );
		    *idx++ = vidx++;
		    *idx++ = vidx++;
		    *idx++ = vidx++;
		    *idx++ = nidx++;
		}
	    }
	}
	else
	{
	    float3 *f3  = vertex.list;
	    float3 *end = f3 + vertex.used;
	    for ( ; f3 < end; f3++  )
		fprintf(F.f, "v %11.3f %11.3f %11.3f\r\n", f3->x, f3->y, f3->z );
	}
    }

    //--- print normals

    if (n_norm)
    {
	fprintf(F.f,head,sep,n_norm,"normals");

	if (kcl->fast)
	{
	    for ( td = td_base; td < td_end; td++ )
		if ( !(td->status & TD_INVALID) )
		    fprintf(F.f,
			"vn %11.3f %11.3f %11.3f\r\n",
			td->normal->x, td->normal->y, td->normal->z );
	}
	else
	{
	    float3 *f3  = normal.list;
	    float3 *end = f3 + normal.used;
	    for ( ; f3 < end; f3++  )
		fprintf(F.f,
		    "vn %11.3f %11.3f %11.3f\r\n",
		    f3->x, f3->y, f3->z );
	}
    }


    //--- print groups and faces

    uint idx2, idx3;
    if ( KCL_MODE & KCLMD_OUT_SWAP )
	idx2 = 2, idx3 = 1;
    else
	idx2 = 1, idx3 = 2;

    uint class_idx[N_KCL_CLASS] = {0};
    for ( i = 0; i <= N_KCL_TYPE; i++ )
    {
	const kcl_class_t *cls = kcl_class + kcl_type[i].cls;
	uint col_idx = class_idx[cls->cls];
	uint col_max = cls->ncol;

	int j, j_max, j_inc;
	if ( i < N_KCL_TYPE )
	{
	    j     = i;
	    j_max = N_KCL_FLAG;
	    j_inc = N_KCL_TYPE;
	}
	else
	{
	    j     = N_KCL_FLAG;
	    j_max = KCL_FLAG_USER_0 + n_user_color;
	    j_inc = 1;
	}

	for ( ; j < j_max; j += j_inc )
	{
	    if (!flag_count[j])
		continue;

	    ccp grp_name = GetGroupNameKCL(j,false);
	    fprintf(F.f,
		"\r\n%s"
		"# %u triangles in group '%s'\r\n"
		"#   -> flag 0x%04x => type 0x%02x, variant 0x%03x\r\n"
		"\r\n"
		"g %s\r\n"
		"s off\r\n"
		, sep
		, flag_count[j], grp_name
		, j, i < N_KCL_TYPE ? i : 0x100, i < N_KCL_TYPE ? j >> 5 : j & 0xffff
		, grp_name
		);

	    if (have_mtl)
	    {
		if ( i < N_KCL_TYPE )
		{
		    fprintf(F.f,"usemtl %s_%u\r\n\r\n",cls->name,++col_idx);
		    if ( col_idx >= col_max )
			col_idx = 0;
		}
		else
		    fprintf(F.f,"usemtl %s\r\n\r\n",grp_name);
	    }
	    else
		fputs("\r\n",F.f);

	    if (print_tri)
	    {
	      for ( td = td_base; td < td_end; td++ )
		if ( td->cur_flag == j && !(td->status & TD_INVALID) )
		{
		    fprintf(F.f,
			"TRI 0x%04x | %11.3f %11.3f %11.3f"
				  " | %11.3f %11.3f %11.3f"
				  " | %11.3f %11.3f %11.3f\r\n",
			GetExportFlagKCL(td->cur_flag),
			td->pt[0].x, td->pt[0].y, td->pt[0].z,
			td->pt[1].x, td->pt[1].y, td->pt[1].z,
			td->pt[2].x, td->pt[2].y, td->pt[2].z );
		}
	     #ifdef TEST0
		else
		    fprintf(F.f,"# flag=%x,%x, j=%x\n",td->in_flag,td->cur_flag,j);
	     #endif
	    }
	    else if (n_norm)
	    {
	      for ( td = td_base; td < td_end; td++ )
		if ( td->cur_flag == j && !(td->status & TD_INVALID) )
		{
		    vertex_index_t *idx = index_list + 4*(td-td_base);
		    fprintf(F.f,
			"f %u//%u %u//%u %u//%u\r\n",
			idx[0], idx[3],
			idx[idx2], idx[3],
			idx[idx3], idx[3] );
		}
	    }
	    else
	    {
	      for ( td = td_base; td < td_end; td++ )
		if ( td->cur_flag == j && !(td->status & TD_INVALID) )
		{
		    vertex_index_t *idx = index_list + 4*(td-td_base);
		    fprintf(F.f,
			"f %u %u %u\r\n",
			idx[0], idx[idx2], idx[idx3] );
		}
	    }
	}
	class_idx[cls->cls] = col_idx ;
    }


    //--- terminate

    fprintf(F.f,"\r\n%s# END\r\n"
		"# define an empty last group as workaround for a SketchUp importer bug\r\n"
		"g empty_group\r\n"
		"\r\n"
		,sep);
    ResetFile(&F,set_time);

    //--- create mtl file

    if (have_mtl)
    {
	enumError err = CreateFileOpt(&F,true,mtl_fname,testmode,0);
	if ( err <= ERR_WARNING && F.f )
	{
	    SetFileAttrib(&F.fatt,&kcl->fatt,0);
 #if 1
	    u8 *data;
	    uint size;
	    if (!DecodeBZIP2(&data,&size,0,text_obj_mtl_bz2,sizeof(text_obj_mtl_bz2)))
		fwrite(data,size,1,F.f);
	    FREE(data);
 #else
	    fputs(kcl_mtl,F.f);
 #endif

	    uint ci;
	    for ( ci = 0; ci < n_user_color; ci++ )
	    {
		fprintf(F.f,"newmtl %s\r\n",GetGroupNameKCL(ci+KCL_FLAG_USER_0,0));
		Color_t col = user_color[ci];
		if ( col.a != 0xff )
		{
		    const float a = col.a / 255.0;
		    fprintf(F.f,"\td  %5.3f\r\n\tTr %5.3f\r\n",a,1.0-a);
		}

		const float r = col.r / 255.0;
		const float g = col.g / 255.0;
		const float b = col.b / 255.0;
		fprintf(F.f,
			"\tKa %5.3f %5.3f %5.3f\r\n"
			"\tKd %5.3f %5.3f %5.3f\r\n"
			"\tKs %5.3f %5.3f %5.3f\r\n"
			"\r\n",
			r, g, b,
			r, g, b,
			r, g, b );
	    }
	}
	ResetFile(&F,set_time);
    }

 abort:
    FREE(flag_count);
    FREE(index_list);
    ResetF3L(&vertex);
    ResetF3L(&normal);

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PrintTriangles()		///////////////
///////////////////////////////////////////////////////////////////////////////

static bool PrintTriangles ( File_t *F, kcl_t *kcl, bool have_used )
{
    DASSERT(F);
    DASSERT(kcl);

    const uint n_tri = kcl->tridata.used;
    if (!n_tri)
	return false;

    FILE *f = F->f;
    fprintf(f,text_kcl_dump_tridata_cr,n_tri);

    kcl_tridata_t *td_base = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t *td, *td_end = td_base + n_tri;

    for ( td = td_base; td < td_end; td++ )
    {
	fprintf(f,"#%u\n",(int)(td-td_base));

	const float3 *f3 = td->normal;
	fprintf(f,"T.N0  %11.6f %11.6f %11.6f | 0x%04x -> 0x%04x\r\n",
		f3->x, f3->y, f3->z,
		td->in_flag, td->cur_flag );

	f3++;
	fprintf(f,"T.N1  %11.6f %11.6f %11.6f | %s\r\n",
		f3->x, f3->y, f3->z,
		td->status & TD_FIXED_PT ? "fixed" : "-" );

	f3++;
	fprintf(f,"T.N2  %11.6f %11.6f %11.6f | %svalid",
		f3->x, f3->y, f3->z,
		td->status & TD_INVALID ? "in" : "" );

	if (have_used)
	    fprintf(f,", %sused", td->status & TD_UNUSED ? "un" : "" );

	f3++;
	fprintf(f,"\r\nT.N3  %11.6f %11.6f %11.6f |\r\n",
		    f3->x, f3->y, f3->z );

    #if SUPPORT_KCL_DELTA
	fprintf(f,"T.P1  %11.3f %11.3f %11.3f | len %g%s, delta %g/%g = %g\r\n",
		td->pt[0].x, td->pt[0].y, td->pt[0].z,
		td->length,
		td->length, < opt_tri_height ? " (too small)" : "",
		td->delta_max, td->delta_min,
		td->delta_min > 0 ? td->delta_max / td->delta_min : 0.0 );
    #else
	fprintf(f,"T.P1  %11.3f %11.3f %11.3f | len %g%s\r\n",
		td->pt[0].x, td->pt[0].y, td->pt[0].z,
		td->length,
		td->length < opt_tri_height ? " (too small)" : "" );
    #endif

	int p;
	for ( p = 1; p < 3; p++ )
	    fprintf(f,"T.P%u  %11.3f %11.3f %11.3f | %10.3f %10.3f %10.3f \r\n",
		p+1, td->pt[p].x, td->pt[p].y, td->pt[p].z,
		td->pt[p].x - td->pt[0].x,
		td->pt[p].y - td->pt[0].y,
		td->pt[p].z - td->pt[0].z );

	tri_metrics_t met;
	CalcTriMetricsByPoints(&met,td->pt,td->pt+1,td->pt+2);
	fprintf(f,"T.LEN %11.3f %11.3f %11.3f | area %g%s\r\n",
	    met.l1, met.l2, met.l3, met.area,
	    met.area < opt_tri_area ? " (too small)" : "" );
	fprintf(f,"T.HT  %11.3f %11.3f %11.3f | min %g%s, max %g\r\n",
	    met.h1, met.h2, met.h3, met.min_ht,
	    met.min_ht < opt_tri_height ? " (too small)" : "",
	    met.max_ht );

	fputs("\r\n",f);
    }

    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  DumpKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

static u32 dump_o_table
(
    // returns next offset

    FILE		*f,		// output file
    kcl_t		*kcl,		// valid kcl
    u32			offset,		// offset to print
    u32			empty_offset,	// offset of empty list
    uint		n,		// number of elements to print
    int			count		// >=0: print usage counter
)
{
    DASSERT(f);
    DASSERT(kcl);
    DASSERT(kcl->octree);
    DASSERT(kcl->o_depth);
    DASSERT(kcl->o_count);
    DASSERT(kcl->l_count);

    noPRINT("dump_o_table: off=%x, n=%x\n",offset,n);

    if ( offset >= kcl->octree_size )
	return kcl->octree_size;

    const uint max_n = ( kcl->octree_size - offset ) / 4;
    if ( n > max_n )
	 n = max_n;

    const u32 baseoff = offset;

    while ( n > 0 )
    {
	fprintf(f,"O.%06x:",offset);

	uint i, max = n < 8 ? n : 8;
	n -= max;
	for ( i = 0; i < max; i++ )
	{
	    if ( i == 4 )
		fputc(' ',f);
	    u32 val = be32(kcl->octree+offset);
	    offset += 4;
	    if ( val & 0x80000000 )
	    {
		val = ( val & 0x7ffffff ) + baseoff + 2;
		if ( val == empty_offset )
		    fprintf(f,"        -");
		else
		    fprintf(f," L.%06x",val);
	    }
	    else
	    {
		val += baseoff;
		fprintf(f," %8x",val & 0x7ffffff);
	    }
	}
	if ( count >= 0 )
	{
	    fprintf(f,"  # %2u*\r\n",count);
	    count = -1;
	}
	else
	    fputs("\r\n",f);
    }
    return offset;
}

///////////////////////////////////////////////////////////////////////////////

enumError DumpKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		print_tridata,	// true: triangle list
    uint		print_octree	// octree print level (0..2)
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(kcl);
    DASSERT(fname);

    CalcNormalsKCL(kcl,false);
    CalcMinMaxKCL(kcl);
    CreateOctreeKCL(kcl);
    CalcStatisticsKCL(kcl);
    CreateRawKCL(kcl,false);
    //const uint blow_size = CalcBlowSizeKCL(kcl);
    KCL_ACTION_LOG("DumpKCL(%s,%d,%d)\n",fname,print_tridata,print_octree);

    PRINT("DumpKCL(%s,PT=%d,PO=%d) N(td)=%u\n",
		fname, print_tridata, print_octree, kcl->tridata.used );


    //--- calculate number of verteces and normals

    const uint n_tri = kcl->tridata.used;
    uint n_vert, n_norm;

    kcl_tridata_t *td_base = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t *td, *td_end = td_base + n_tri;

    if (kcl->fast)
    {
	n_vert = 3 * n_tri;
	n_norm = KCL_MODE & KCLMD_NORMALS ? n_tri : 0;
    }
    else
    {
     #if HAVE_PRINT
	u_usec_t start_time = GetTimerUSec();
     #endif
	float3List_t vertex, normal;
	InitializeF3L(&vertex,n_tri);
	InitializeF3L(&normal,n_tri*4);
	//AddSignature(&vertex,false);

	for ( td = td_base; td < td_end; td++ )
	{
	    float3 temp;
	    temp.x = td->pt->x;
	    temp.y = td->pt->y;
	    temp.z = td->pt->z;
	    FindInsertFloatF3L(&vertex,&temp,false);

	    FindInsertFloatF3L(&normal,td->normal+0,false);
	    FindInsertFloatF3L(&normal,td->normal+1,false);
	    FindInsertFloatF3L(&normal,td->normal+2,false);
	    FindInsertFloatF3L(&normal,td->normal+3,false);
	}
	n_vert = vertex.used;
	n_norm = normal.used;
     #if HAVE_PRINT
	uint end_time = GetTimerUSec();
	PRINT(">> %u/%u/%u verteces and %u/%u/%u normals inserted in %6.4f sec.\n",
		vertex.used, vertex.size, n_tri,
		normal.used, normal.size, 4*n_tri,
		1e-6 * (end_time-start_time) );
     #endif
	ResetF3L(&vertex);
	ResetF3L(&normal);
    }


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,kcl->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&kcl->fatt,0);


    //--- print header

    if (kcl->signature > 0.0 )
	fprintf(F.f,"\r\nH.signature      = %s v%4.2f\r\n",
		kcl->signature_info, kcl->signature );

    fprintf(F.f,text_kcl_dump_head_cr,
	n_vert, n_norm, n_tri,
	    n_tri - kcl->n_tri_used, kcl->tri_fix_count, kcl->tri_inv_count,
	kcl->octree_size, kcl->raw_data_size,

	kcl->unknown_0x10, kcl->unknown_0x38,

	kcl->min_octree.x, kcl->min_octree.y, kcl->min_octree.z,
	kcl->min.x, kcl->min.y, kcl->min.z,
	kcl->mean.x, kcl->mean.y, kcl->mean.z,
	kcl->max.x, kcl->max.y, kcl->max.z,
	kcl->max_octree.x, kcl->max_octree.y, kcl->max_octree.z,

	kcl->mask[0], kcl->mask[1], kcl->mask[2],
	kcl->coord_rshift, kcl->y_lshift, kcl->z_lshift,
	kcl->mask_bits[0], kcl->mask_bits[1], kcl->mask_bits[2],
	kcl->bcube_bits[0], kcl->bcube_bits[1], kcl->bcube_bits[2],
	kcl->n_bcubes[0], kcl->n_bcubes[1], kcl->n_bcubes[2],

	kcl->tmin_dist,
	kcl->tmax_dist,

	kcl->bcube_width,
	kcl->bcube_width >> kcl->min_depth,
	kcl->bcube_width >> kcl->max_depth,
	//blow_size,
	kcl->total_bcubes,
	kcl->n_cube_nodes,
	kcl->total_cubes,
	kcl->max_depth,
	kcl->ave_depth,

	kcl->n_0_list,
	kcl->n_tri_list,
	kcl->n_0_link,
	kcl->n_tri_link
	);

    uint di;
    //for ( di = 0; di < KCL_MAX_STAT_DEPTH && di <= kcl->max_depth; di++ )
    for ( di = 0; di < KCL_MAX_STAT_DEPTH; di++ )
	if (kcl->max_depth_tri_len[di])
	    fprintf(F.f,"S.max_tri_size.%-2u= %8u"
		"       # max number of triangles in all lists of depth %u\r\n",
		di-1, kcl->max_depth_tri_len[di], di-1 );

    fprintf(F.f,
	"S.max_tri_size   = %8u       # maximum number of triangles in all lists\r\n"
	"S.ave_tri_size   = %11.2f    # average number of triangles in all lists\r\n\r\n",
	kcl->max_tri_len,					// max_tri_size
	(double)kcl->sum_tri_len/kcl->n_tri_list		// ave_tri_size
	);


    //--- print triangles

    if (print_tridata)
    {
	fputs("#\f\r\n"
		"########################################"
		"#######################################\r\n",F.f);

	PrintTriangles(&F,kcl,true);
    }


    //--- print octree

    if ( print_octree && kcl->octree_valid && kcl->octree )
    {
	fprintf(F.f,text_kcl_dump_octree_cr,
		kcl->total_bcubes, kcl->total_cubes - kcl->total_bcubes,
		kcl->n_tri_list );


	u32 empty_offset = kcl->octree_size >= 2
			    && !be16(kcl->octree+kcl->octree_size-2)
			    ? kcl->octree_size - 2 : 0;

	u32 offset = dump_o_table(F.f,kcl,0,empty_offset,kcl->total_bcubes,-1);
	fputs("\r\n",F.f);

	while ( offset < kcl->max_o_offset && offset < kcl->octree_size )
	    offset = dump_o_table(F.f,kcl,offset,empty_offset,8,
				    kcl->o_count[(offset-kcl->min_o_offset)/0x20]);

	if ( print_octree > 1 )
	{
	    fprintf(F.f,text_kcl_dump_octlist_cr);

	    uint li;
	    for ( li = 0; li < kcl->n_trilist; li++ )
	    {
		if ( kcl->l_count[li] )
		{
		    offset = li * 2 + kcl->min_l_offset;
		    fprintf(F.f,"L.%06x [%2u*]:",offset,kcl->l_count[li]);
		    for(;;)
		    {
			const u16 tidx = be16(kcl->octree+offset);
			if (!tidx)
			    break;
			fprintf(F.f," %u",tidx-1);
			offset += 2;
		    }
		    fputs("\r\n",F.f);
		}
	    }
	    fputs("\r\n",F.f);
	}
    }


    //--- terminate

    ResetFile(&F,false);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			DumpTrianglesKCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DumpTrianglesKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname		// filename of destination
)
{
    DASSERT(kcl);
    DASSERT(fname);

    KCL_ACTION_LOG("DumpTrianglesKCL(%s)\n",fname);
    PRINT("DumpTrianglesKCL(%s)\n",fname);
    CalcNormalsKCL(kcl,false);

    if (kcl->octree_valid)
	CalcStatisticsKCL(kcl);

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,kcl->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&kcl->fatt,0);
    PrintTriangles(&F,kcl,kcl->octree_valid);
    ResetFile(&F,false);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ListTrianglesKCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ListTrianglesKCL
(
    kcl_t		* kcl,		// pointer to valid KCL
    ccp			fname,		// filename of destination
    bool		print_normals,	// true: print large tables
    uint		precision	// floating point pre: 0=+0, .. 3=+6
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(kcl);
    DASSERT(fname);

    CalcNormalsKCL(kcl,false);
    CalcStatisticsKCL(kcl);

    KCL_ACTION_LOG("ListTrianglesKCL(%s,%d,%u)\n",fname,print_normals,precision);

    PRINT("ListTrianglesKCL(%s,PN=%d,PREC=%u) N=%u\n",
		fname, print_normals, precision, kcl->tridata.used );

    if ( precision > 5 )
	 precision = 5;
    const uint prec_pt   = precision;
    const uint prec_len  = precision + 1;
    const uint prec_norm = precision + 2;
    const uint fw_pt     = 7 + prec_pt + (prec_pt>0);
    const uint fw_len    = 6 + prec_len;
    const uint fw_norm   = 3 + prec_norm;


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,kcl->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&kcl->fatt,0);


    //--- print data

    const kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    uint i;
    for ( i = 0; i < kcl->tridata.used; i++, td++ )
    {
	fprintf(F.f,
	    "%04x %c %*.*f",
	    td->cur_flag,
	    !kcl->used_valid ? '?' : td->status & TD_UNUSED ? '-' : '+',
	    fw_len, prec_len, td->length
	    );

	uint ti;
	const double3 *d3 = td->pt;
	for ( ti = 0; ti < 3; ti++, d3++ )
	    fprintf(F.f,"%s %*.*f %*.*f %*.*f",
		ti ? "," : " |",
		fw_pt, prec_pt, d3->x,
		fw_pt, prec_pt, d3->y,
		fw_pt, prec_pt, d3->z );

	if (print_normals)
	{
	    uint ni;
	    const float3 *f3 = td->normal;
	    for ( ni = 0; ni < 4; ni++, f3++ )
	    {
		fprintf(F.f,"%s %*.*f %*.*f %*.*f",
		    ni ? "," : " |",
		    fw_norm, prec_norm, f3->x,
		    fw_norm, prec_norm, f3->y,
		    fw_norm, prec_norm, f3->z );
	    }
	}
	fputs("\r\n",F.f);
    }


    //--- terminate

    ResetFile(&F,false);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			TraverseOctreeKCL()		///////////////
///////////////////////////////////////////////////////////////////////////////

static int TraverseOctree
(
    // returns	 -2: octree invalid
    //		 -1: octree data invalid
    //		>=0: number of triangles

    kcl_t		*kcl,		// pointer to valid KCL
    FILE		*f,		// log file
    double3		pt,		// point to analyze
    int			verbosity,	// log verbose level
					//   <=0: silent
					//   >=1: print warnings
					//   >=2: print base calculations
					//   >=3: print step calculations
    double3		*cube_min,	// not NULL: store cube data here
    uint		*cube_width,	// not NULL: store cube width here
    u16			**tri_list	// not NULL: store pointer to
					//           triangle list here
)
{
    DASSERT(kcl);
    if ( !kcl->octree || !kcl->octree_valid )
	return -1;

    if (cube_min)
	memset(cube_min,0,sizeof(*cube_min));
    if (cube_width)
	*cube_width = 0.0;
    if (tri_list)
	*tri_list = 0;


    //--- test 'min_octree'

    if (   pt.x < kcl->min_octree.x
	|| pt.y < kcl->min_octree.y
	|| pt.z < kcl->min_octree.z )
    {
	if ( verbosity > 0 )
	    fprintf(f,
		"# No triangles: At least one coordinate is too small.\n"
		"#    * point:      %11.3f %11.3f %11.3f\n"
		"#    * min border: %11.3f %11.3f %11.3f\n"
		"#\n",
		pt.x, pt.y, pt.z,
		kcl->min_octree.x, kcl->min_octree.y, kcl->min_octree.z );
	return 0;
    }

    double3 basept;
    Sub3(basept,pt,kcl->min_octree);

    if ( basept.x > 2e9 || basept.y > 2e9 || basept.z > 2e9 )
    {
	if ( verbosity > 0 )
	    fprintf(f,
		"# No triangles: At least one coordinate is too large (diff>10^9).\n"
		"#    * point:       %11.3f %11.3f %11.3f\n"
		"#    * point - min: %11.3f %11.3f %11.3f\n"
		"#\n",
		pt.x, pt.y, pt.z,
		basept.x, basept.y, basept.z );
	return 0;
    }


    //--- create and test integer coordinates

    u32 idx_mask = 1 << kcl->coord_rshift; // same as current cube width!

    double3 cmin;
    uint p, failed = 0;
    u32 base[3], relevant[3], nmask[3], nbits[3];
    for ( p = 0; p < 3; p++ )
    {
	cmin.v[p] = trunc( basept.v[p] / idx_mask ) * idx_mask + kcl->min_octree.v[p];
	base[p] = round(basept.v[p]);
	u32 mask = kcl->mask[p];
	relevant[p] = ( base[p] & ~mask ) >> kcl->coord_rshift << kcl->coord_rshift;
	if ( base[p] & mask )
	    failed++;

	if (!mask)
	    nmask[p] = 32;
	else
	{
	    nmask[p] = 0;
	    for ( ; !(mask&1); mask >>= 1 )
		nmask[p]++;
	}
	nbits[p] = nmask[p] > kcl->coord_rshift
			? nmask[p] - kcl->coord_rshift
			: 0;
    }

    if (failed)
    {
	if ( verbosity > 0 )
	    fprintf(f,
		"# No triangles: Mask test failed.\n"
		"#    * point:       %11.3f %11.3f %11.3f\n"
		"#    * min border:  %11.3f %11.3f %11.3f\n"
		"#    * relevant pt: %11u %11u %11u\n"
		"#    * relevant pt: %#11x %#11x %#11x\n"
		"#    * mask:        %#11x %#11x %#11x\n"
		"#\n",
		pt.x, pt.y, pt.z,
		kcl->min_octree.x, kcl->min_octree.y, kcl->min_octree.z,
		base[0], base[1], base[2],
		base[0], base[1], base[2],
		kcl->mask[0], kcl->mask[1], kcl->mask[2] );
	return ERR_OK;
    }

    u32 oct_index = base[0] >> kcl->coord_rshift
		  | base[1] >> kcl->coord_rshift << kcl->y_lshift
		  | base[2] >> kcl->coord_rshift << kcl->z_lshift;

    u32 baseoff = 0;
    u32 offset  = oct_index * 4;

    if ( verbosity > 1 )
    {
	fprintf(f,"# Base values:\n"
		"#    * point:         %11.3f %11.3f %11.3f\n"
		"#    * min border:    %11.3f %11.3f %11.3f\n"
		"#    * relevant pt:   %11u %11u %11u\n"
		"#    * relevant pt:   %#11x %#11x %#11x\n"
		"#    * mask:          %#11x %#11x %#11x\n"
		"#    * relevant bits: %11u %11u %11u\n"
		"#    * index bits:    %11u %11u %11u  (rshift=%u)\n"
		"#    * index pt:      %#11x %#11x %#11x\n"
		"#    * start index:   %#x | %#x | %#x == %#x  =>  offset=%#x\n"
		"#    * cube min:      %11.3f %11.3f %11.3f  (width=%u)\n"
		"#    * cube max:      %11.3f %11.3f %11.3f\n"
		"#\n",
		pt.x, pt.y, pt.z,
		kcl->min_octree.x, kcl->min_octree.y, kcl->min_octree.z,
		base[0], base[1], base[2],
		base[0], base[1], base[2],
		kcl->mask[0], kcl->mask[1], kcl->mask[2],
		nmask[0], nmask[1], nmask[2],
		nbits[0], nbits[1], nbits[2], kcl->coord_rshift,
		relevant[0], relevant[1], relevant[2],
		base[0] >> kcl->coord_rshift,
		 base[1] >> kcl->coord_rshift << kcl->y_lshift,
		 base[2] >> kcl->coord_rshift << kcl->z_lshift,
		 oct_index, offset,
		cmin.x, cmin.y, cmin.z, idx_mask,
		cmin.x+idx_mask, cmin.y+idx_mask, cmin.z+idx_mask );
    }


    //--- traverse the octree

    uint step;
    for ( step=1;; step++ )
    {
	if ( baseoff + 0x20 > kcl->octree_size )
	{
	    if ( verbosity > 0 )
		fprintf(f,"# Abort, because octree index out of bounds!\n"
		    "#    * point:  %11.3f %11.3f %11.3f\n"
		    "#    * Offset: %#10x\n"
		    "#    * Size:   %#10x\n"
		    "#\n",
		    pt.x, pt.y, pt.z,
		    offset, kcl->octree_size );
	    return -1;
	}

	if ( offset & 3 )
	{
	    if ( verbosity > 0 )
		fprintf(f,"# Abort, because invalid octree index!\n"
		    "#    * point:      %11.3f %11.3f %11.3f\n"
		    "#    * Offset:     %#10x\n"
		    "#    * Low 2 bits: %#10x\n"
		    "#\n",
		    pt.x, pt.y, pt.z,
		    offset, offset&3 );
	    return -1;
	}

	u32 val = be32( kcl->octree + offset );
	if ( val & 0x80000000 )
	{
	    offset = baseoff + ( val & 0x7ffffff ) + 2;
	    if ( offset+2 > kcl->octree_size )
	    {
		if ( verbosity > 0 )
		    fprintf(f,"# Abort, because triangle list index out of bounds!\n"
			"#    * point:  %11.3f %11.3f %11.3f\n"
			"#    * Offset: %#10x\n"
			"#    * Size:   %#10x\n"
			"#\n",
			pt.x, pt.y, pt.z,
			offset, kcl->octree_size );
		return -1;
	    }

	    if ( offset & 1 )
	    {
		if ( verbosity > 0 )
		    fprintf(f,"# Abort, because invalid triangle list index!\n"
			"#    * point:   %11.3f %11.3f %11.3f\n"
			"#    * Offset:  %#10x\n"
			"#    * Low bit: %#10x\n"
			"#\n",
			pt.x, pt.y, pt.z,
			offset, offset&1 );
		return -1;
	    }

	    if (cube_min)
		*cube_min = cmin;
	    if (cube_width)
		*cube_width = idx_mask;
	    if (tri_list)
		*tri_list = (u16*)( kcl->octree + offset );

	    u32 off;
	    for ( off = offset; off < kcl->octree_size; off += 2 )
		if (!be16( kcl->octree + off ))
		    break;
	    return ( off - offset ) / 2;
	}

	uint next_index = 0;
	idx_mask >>= 1;
	if ( idx_mask & base[0] )
	{
	    next_index |= 1;
	    cmin.x += idx_mask;
	}
	if ( idx_mask & base[1] )
	{
	    next_index |= 2;
	    cmin.y += idx_mask;
	}
	if ( idx_mask & base[2] )
	{
	    next_index |= 4;
	    cmin.z += idx_mask;
	}
	baseoff += val;
	offset = baseoff + 4*next_index;

	if ( verbosity > 2 )
	{
	    fprintf(f,"# Step #%u, next octree entry found, value %#x:\n"
		    "#    * mask %#x,  index %u+%u+%u = %u,  offset %#x,%#x\n"
		    "#    * cube min:      %11.3f %11.3f %11.3f  (width=%u)\n"
		    "#    * cube max:      %11.3f %11.3f %11.3f\n"
		    "#\n",
		    step, val,
		    idx_mask,
		    next_index & 1, next_index & 2, next_index & 4,
		    next_index, baseoff, offset,
		    cmin.x, cmin.y, cmin.z, idx_mask,
		    cmin.x+idx_mask, cmin.y+idx_mask, cmin.z+idx_mask );
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError TraverseOctreeKCL
(
    kcl_t		*kcl,		// pointer to valid KCL
    FILE		*f,		// output file, if NULL set verbosity:=0
    double3		pt,		// point to analyze
    bool		print_tri_val,	// true: print tri values instead of index
    uint		verbosity	// verbose level
					//   >=1: print base calculations
					//   >=2: print step calculations
)
{
    DASSERT(kcl);
    CreateOctreeKCL(kcl);
    u16 *tri_ptr;
    int stat = TraverseOctree(kcl,f,pt,verbosity+1,0,0,&tri_ptr);
    if ( stat < 0 )
	return ERR_WARNING;

    const int fw_tlist = GetTermWidth(80,40) - 7;

    if (!f)
	verbosity = 0;
    else if (verbosity>0)
	fputc('\n',f);
    fprintf(f,"List with %u triangles found at offset %#x\n",
		stat, (int)((u8*)tri_ptr - kcl->octree) );

    if (!print_tri_val)
    {
	uint pcount = fprintf(f,"    - ");;
	for ( ; stat > 0; stat--, tri_ptr++ )
	{
	    uint tidx = be16(tri_ptr);
	    if (!tidx--)
		break;
	    if ( pcount > fw_tlist )
		pcount = fprintf(f,"\n      ");
	    pcount += fprintf(f," %u%s",
		tidx,
		tidx <= kcl->tridata.used ? "" : "!" );
	}
    }
    else
    {
	fprintf(f,"\n   * searched pt:   %11.3f %11.3f %11.3f\n\n",
		    pt.x, pt.y, pt.z );

	for ( ; stat > 0; stat--, tri_ptr++ )
	{
	    uint tidx = be16(tri_ptr);
	    if (!tidx--)
		break;
	    if ( tidx < kcl->tridata.used )
	    {
		kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list + tidx;
		fprintf(f,
		    "   * triangle pt 1: %11.3f %11.3f %11.3f  (index %u)\n"
		    "            * pt 2: %11.3f %11.3f %11.3f  (flag 0x%04x)\n"
		    "            * pt 3: %11.3f %11.3f %11.3f\n"
		    "\n",
		    td->pt[0].x, td->pt[0].y, td->pt[0].z, tidx,
		    td->pt[1].x, td->pt[1].y, td->pt[1].z, td->cur_flag,
		    td->pt[2].x, td->pt[2].y, td->pt[2].z );
	    }
	    else
		fprintf(f,"   * Invalid triangle, index = %u = %#x\n",
		    tidx, tidx );

	}
    }
    fputs("\n\n",f);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			FallKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

#define N_FALL_LIST 10

typedef struct kcl_fall_t
{
    kcl_t	* kcl;			// related kcl
    FILE	*f;			// log file
    int		verbose;		// verbose level
    u32		type_mask;		// kcl type selector (bit field)

    kcl_cube_t	cube;			// cube to test
    uint	width;			// width of cube

    u16		*tri_list[N_FALL_LIST];	// list to tri lists
    uint	n_tri_list;		// number of valid 'tri_list'

    bool	last_valid;		// last values are valid
    double3	last_min;		// last traversed cube min
    uint	last_width;		// last traversed cube width

} kcl_fall_t;

///////////////////////////////////////////////////////////////////////////////

static int Collide2
(
    kcl_fall_t		*kf,		// valid data
    double3		pt		// point for octree search
)
{
    DASSERT(kf);
    noPRINT("Collide2() n=%u, v=%d, pt=%.0f %.0f %.0f\n",
	    kf->n_tri_list, kf->last_valid, pt.x, pt.y, pt.z );


    //--- test if point is in previous cube

    if ( kf->last_valid
	&& pt.x >= kf->last_min.x && pt.x <= kf->last_min.x + kf->last_width
	&& pt.y >= kf->last_min.y && pt.y <= kf->last_min.y + kf->last_width
	&& pt.z >= kf->last_min.z && pt.z <= kf->last_min.z + kf->last_width
       )
    {
	return -1;
    }

    //--- find tri list

    u16 *tri_list;
    int stat = TraverseOctree(kf->kcl,kf->f,pt,
		kf->verbose-1, &kf->last_min,&kf->last_width,&tri_list);
    if ( stat < 0 )
	return -1;

    kf->last_valid = true;
    if ( !tri_list || !*tri_list )
	return -1;

    uint i;
    for ( i = 0; i < kf->n_tri_list; i++ )
	if (kf->tri_list[i] == tri_list )
	    return -1;

    if ( kf->n_tri_list < N_FALL_LIST )
	kf->tri_list[kf->n_tri_list++] = tri_list;

    uint ti;
    for ( ti = 0; ti < stat; ti++ )
    {
	DASSERT(tri_list);
	uint tidx = be16(tri_list+ti);
	if (!tidx--)
	    break;
	if ( tidx >= kf->kcl->tridata.used )
	    continue;

	const kcl_tridata_t *td = (kcl_tridata_t*)kf->kcl->tridata.list + tidx;
	if ( td->cur_flag >= N_KCL_TYPE || ( 1u << (td->cur_flag&0x1f) & kf->type_mask ) )
	{
	    kcl_tri_t ot;
	 #if SUPPORT_KCL_CUBE
	    setup_oct_tri(kf->kcl,&ot,0,td); // use [[cube]] list param?
	    if (OctCubeTriangleOverlaped(&kf->cube,&ot,0)) // [use [[cube]] list param?
		return td->cur_flag;
	 #else
	    setup_oct_tri(kf->kcl,&ot,td);
	    if (OctCubeTriangleOverlaped(&kf->cube,&ot))
		return td->cur_flag;
	 #endif
	}
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

static int Collide1
(
    kcl_fall_t		*kf,		// valid data
    double		y		// y point to test
)
{
    DASSERT(kf);
    kf->n_tri_list = 0;
    kf->last_valid = false;

    kf->cube.min[1] = round ( y - kf->kcl->min_octree.y );
    kf->cube.max[1] = kf->cube.min[1] + kf->width;

    noPRINT("OCT=%d,%d,%d .. %d,%d,%d\n",
		kf->cube.min[0], kf->cube.min[1], kf->cube.min[2],
		kf->cube.max[0], kf->cube.max[1], kf->cube.max[2] );

    double3 pt;
    double3 *min = &kf->kcl->min_octree;
    pt.x = min->x + ( kf->cube.min[0] + kf->cube.max[0] ) / 2;
    pt.y = min->y +   kf->cube.min[1];
    pt.z = min->z + ( kf->cube.min[2] + kf->cube.max[2] ) / 2;
    int stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.x = min->x + kf->cube.min[0];
    pt.z = min->z + kf->cube.min[2];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.z = min->z + kf->cube.max[2];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.x = min->x + kf->cube.max[0];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.z = min->z + kf->cube.min[2];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.y = min->y + kf->cube.max[1];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.z = min->z + kf->cube.max[2];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.x = min->y + kf->cube.min[0];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.z = min->z + kf->cube.min[2];
    stat = Collide2(kf,pt);
    if ( stat >= 0 )
	return stat;

    pt.x = min->x + ( kf->cube.min[0] + kf->cube.max[0] ) / 2;
    pt.z = min->z + ( kf->cube.min[2] + kf->cube.max[2] ) / 2;
    return Collide2(kf,pt);
}

///////////////////////////////////////////////////////////////////////////////

double FallKCL
(
    // returns the height or -1.0 on fail

    kcl_t		*kcl,		// pointer to valid KCL
    FILE		*f,		// output file, if NULL: verbose:=0
    double3		pt,		// mid point of cube to analyze
    uint		width,		// width of cube
    int			verbosity,	// verbose level
					//   <=0: silent
					//   >=1: log triangle search
					//   >=2: print base calculations
					//   >=3: print step calculations
    u32			type_mask,	// kcl type selector (bit field)
    int			*res_kcl_flag	// not NULL: store KCL flag here
)
{
    DASSERT(kcl);
    CreateOctreeKCL(kcl);
    if ( !kcl->octree || !kcl->octree_valid || pt.y < 0.0  )
	return -1.0;

    if ( width < 1 )
	 width = 1;

    kcl_fall_t kf;
    memset(&kf,0,sizeof(kf));
    kf.kcl	 = kcl;
    kf.f	 = f;
    kf.verbose	 = f ? verbosity : 0;
    kf.width	 = width;
    kf.type_mask = type_mask;

    uint p;
    for ( p = 0; p < 3; p++ )
    {
	kf.cube.min[p] = round ( pt.v[p] - kcl->min_octree.v[p] - width/2);
	kf.cube.max[p] = kf.cube.min[p] + width;
    }

    if ( width > 1 )
	 width--;

    // very first test
    int stat = Collide1(&kf,pt.y);
    if ( stat >= 0 )
    {
	if (res_kcl_flag)
	    *res_kcl_flag = stat;
	return pt.y;
    }

    pt.y = ceil( pt.y - width ); // round only if really falling!
    for(;;)
    {
	stat = Collide1(&kf,pt.y);
	if ( stat >= 0 )
	    break;
	if ( pt.y <= 0.0 )
	{
	    if (res_kcl_flag)
		*res_kcl_flag = -1;
	    return -1.0;
	}
	pt.y -= width;
	if ( pt.y < 0.0 )
	    pt.y = 0.0;
    }

    pt.y += width;
    while ( width > 1 )
    {
	width = (width+1)/2;
	pt.y -= width;
	int stat2 = Collide1(&kf,pt.y);
	if ( stat2 >= 0 )
	{
	    stat = stat2;
	    pt.y += width;
	}
    }

    if ( pt.y < 0.0 )
    {
	pt.y = -1.0;
	stat = -1;
    }

    if (res_kcl_flag)
	*res_kcl_flag = stat;
    return pt.y;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    drop+sort unused triangles		///////////////
///////////////////////////////////////////////////////////////////////////////

static kcl_t *kcl_sort = 0;

static int sort_triangle ( const u16 *ia, const u16 *ib )
{
    DASSERT(kcl_sort);

    kcl_tridata_t *ta = (kcl_tridata_t*)kcl_sort->tridata.list + *ia;
    kcl_tridata_t *tb = (kcl_tridata_t*)kcl_sort->tridata.list + *ib;
    if ( ta->length != tb->length )
    {
	double absa = fabs(ta->length);
	double absb = fabs(tb->length);
	if ( absa != absb )
	    return absa < absb ? -1 : +1;
	return ta->length < tb->length ? -1 : +1;
    }

    double *pa = ta->pt->v;
    double *pb = tb->pt->v;
    uint count;
    for ( count = 0; count < 9; count++, pa++, pb++ )
	if ( *pa != *pb )
	    return *pa < *pb ? -1 : +1;

    if ( ta->cur_flag != tb->cur_flag )
	return ta->cur_flag < tb->cur_flag ? -1 : +1;

    return ia < ib ? -1 : ia > ib;
}

///////////////////////////////////////////////////////////////////////////////

bool DropSortHelper
(
    kcl_t		*kcl,		// valid KCL
    uint		drop_mask,	// not 0: drop triangles with set bits
    bool		sort		// true: sort triangles
)
{
    DASSERT(kcl);
    if (sort)
	CalcNormalsKCL(kcl,false);
    KCL_ACTION_LOG("DropSortTriangles(,drop=%x,sort=%d)\n",drop_mask,sort);


    //--- setup list

    const uint n_tri = kcl->tridata.used;
    if (!n_tri)
	return false;
    u16 *list = CALLOC(n_tri,sizeof(*list));
    bool dirty = false;


    //--- drop unreferenced triangles

    kcl_tridata_t *td_base = (kcl_tridata_t*)kcl->tridata.list;
    kcl_tridata_t *td, *td_end = td_base + n_tri;

    uint n_list = 0;
    if (drop_mask)
    {
	uint n_invalid = 0, n_fixed = 0;
	for ( td = td_base; td < td_end; td++ )
	{
	    if ( !(td->status & drop_mask) )
	    {
		list[n_list++] = td - td_base;
		if ( td->status & TD_INVALID )
		    n_invalid++;
		else if ( td->status & TD_FIXED_PT )
		    n_fixed++;
	    }
	    else if ( !(td->status & TD_UNUSED) )
		kcl->octree_valid = false; // a used triangles was removed
	}
	kcl->tri_inv_count = n_invalid;
	kcl->tri_fix_count = n_fixed;
	if ( n_list < n_tri )
	{
	    dirty = true;
	    kcl->min_max_valid = false;
	}
    }
    else
    {
	for ( ; n_list < n_tri; n_list++ )
	    list[n_list] = n_list;
    }

    #if HAVE_PRINT
    {
	uint i, max = n_list < 10 ? n_list : 10;
	PRINT("INDEX-LIST [%u/%u/%u,dirty=%u]:",max,n_list,n_tri,dirty);
	for ( i = 0; i < max; i++ )
	    fprintf(stderr," %u",list[i]);
	fputc('\n',stderr);
    }
    #endif


    //--- sort triangles

    if ( sort && n_list > 1 )
    {
	dirty = true; // assume that sort will change anything
	kcl_sort = kcl;
	qsort( list, n_list, sizeof(*list), (qsort_func)sort_triangle );
	kcl_sort = 0;
    }


    //--- fix kcl->tridata

    if (dirty)
    {
	kcl->model_modified = true;

	kcl_tridata_t *tsrc = (kcl_tridata_t*)kcl->tridata.list;
	kcl->tridata.list = 0;
	ResetList(&kcl->tridata);
	kcl_tridata_t *tdest = GrowList(&kcl->tridata,n_list);

	uint idx;
	for ( idx = 0; idx < n_list; idx++, tdest++ )
	    memcpy(tdest,tsrc+list[idx],sizeof(*tdest));
	FREE(tsrc);
    }


    //--- fix kcl->octree

    if ( dirty && kcl->octree && kcl->octree_valid )
    {
	// we need a reverse list!
	u16 *rlist = CALLOC(n_tri+1,sizeof(*rlist));
	uint idx;
	for ( idx = 0; idx < n_list; idx++ )
	    rlist[list[idx]+1] = idx+1;

	// now we fix the octree
	u16 *ptr = (u16*)(kcl->octree + kcl->min_l_offset);
	u16 *end = (u16*)(kcl->octree + kcl->max_l_offset);
	for ( ; ptr < end; ptr++ )
	    write_be16(ptr,rlist[be16(ptr)]);

	FREE(rlist);
    }


    //--- terminate

    FREE(list);
    if (dirty)
	ResetStatisticsKCL(kcl);
    return dirty;
}

///////////////////////////////////////////////////////////////////////////////

bool DropSortTriangles ( kcl_t * kcl )
{
    DASSERT(kcl);
    if (!drop_sort_triangles)
	return false;

    kcl_mode_t local_mode = KCL_MODE;
    if ( enable_kcl_drop_auto > 0 && local_mode & KCLMD_DROP_AUTO )
    {
	local_mode |= kcl->octree_valid && kcl->fform_outfile == FF_KCL
			? KCLMD_DROP_UNUSED
			: KCLMD_M_DROP;
	PRINT("DROP-AUTO[%d,%d] => %llx -> %llx\n",
		kcl->octree_valid, kcl->fform_outfile == FF_KCL,
		(u64)KCL_MODE, (u64)local_mode );
    }

    kcl_tridata_status_t mode = 0;
    if ( local_mode & KCLMD_DROP_UNUSED  ) mode |= TD_UNUSED;
    if ( local_mode & KCLMD_DROP_FIXED   ) mode |= TD_FIXED_PT;
    if ( local_mode & KCLMD_DROP_INVALID ) mode |= TD_INVALID;
    PRINT("DROP-MODE: %x\n",mode);

    bool sort_triangles	= 0 != ( local_mode & KCLMD_SORT );
    PRINT("*** DropSortTriangles() mode=%x|%u=%u ***\n",
		mode, sort_triangles, drop_sort_triangles );

    if ( local_mode & KCLMD_DROP_UNUSED && kcl->octree_valid )
	CalcStatisticsKCL(kcl);

    if ( local_mode & (KCLMD_DROP_INVALID|KCLMD_RM_FACEDOWN
			|KCLMD_RM_FACEUP|KCLMD_CONV_FACEUP) )
    {
	CalcNormalsKCL(kcl,false);
	if ( local_mode & (KCLMD_RM_FACEDOWN|KCLMD_RM_FACEUP|KCLMD_CONV_FACEUP) )
	{
	    mode |= TD_MARK;

	    kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	    kcl_tridata_t *td_end = td + kcl->tridata.used;
	    for ( ; td < td_end; td++ )
		td->status &= ~TD_MARK;
	}
    }

    if ( local_mode & KCLMD_RM_FACEDOWN )
    {
	kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	kcl_tridata_t *td_end = td + kcl->tridata.used;
	for ( ; td < td_end; td++ )
	{
	    if ( td->normal[0].y <= -0.5 )
	    {
		DASSERT(!(td->status&TD_INVALID_NORM));
		const int type = GET_KCL_TYPE(td->cur_flag);
		if ( type >= 0 && kcl_type[type].attrib & KCLT_DRIVE )
		    td->status |= TD_MARK;
	    }
	}
    }

    if ( local_mode & (KCLMD_RM_FACEUP|KCLMD_CONV_FACEUP) )
    {
	kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	kcl_tridata_t *td_end = td + kcl->tridata.used;
	for ( ; td < td_end; td++ )
	{
	    if ( td->normal[0].y > 0.985 )
	    {
		DASSERT(!(td->status&TD_INVALID_NORM));
		const int type = GET_KCL_TYPE(td->cur_flag);
		if ( type >= 0 && kcl_type[type].attrib & KCLT_C_WALL )
		{
		    if ( local_mode & KCLMD_CONV_FACEUP )
			td->cur_flag = 0;
		    else
			td->status |= TD_MARK;
		}
	    }
	}
    }

    if ( local_mode & KCLMD_WEAK_WALLS )
    {
	kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
	kcl_tridata_t *td_end = td + kcl->tridata.used;
	for ( ; td < td_end; td++ )
	{
	    const int type = GET_KCL_TYPE(td->cur_flag);
	    if ( type >= 0 && kcl_type[type].attrib & KCLT_C_WALL )
		td->cur_flag |= 0x8000;
	}
    }

    return DropSortHelper( kcl, mode, sort_triangles );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			KCL Script			///////////////
///////////////////////////////////////////////////////////////////////////////

StringField_t kcl_script_list = {0};

///////////////////////////////////////////////////////////////////////////////

int ScanOptKclScript ( ccp arg )
{
    if ( arg && *arg )
    {
	if (!kcl_script_list.used)
	    have_patch_count++, have_kcl_patch_count++;
	InsertStringField(&kcl_script_list,arg,false);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool ScriptKCL
(
    kcl_t		*kcl,	    // valid KCL data
    bool		is_first    // true: called at first place
)
{
    DASSERT(kcl);

    if ( (KCL_MODE & KCLMD_F_SCRIPT) ? !is_first : is_first )
	return false;

    PRINT("ScriptKCL(,%d)\n",is_first);

    //--- load source file once

    static uint  used = 0;
    static u8  **data = 0;
    static uint *data_size = 0;

    if ( !used && kcl_script_list.used )
    {
	used = kcl_script_list.used;

	// script files are only loaded once
	data      = CALLOC( kcl_script_list.used, sizeof(*data) );
	data_size = CALLOC( kcl_script_list.used, sizeof(*data_size) );

	uint i, active = 0;
	for ( i = 0; i < used; i++ )
	{
	    OpenReadFILE(kcl_script_list.field[i],0,false,data+i,data_size+i,0,0);
	    if ( data[i] && data_size[i] )
		active++;
	}

	if (!active)
	{
	    ResetStringField(&kcl_script_list);
	    have_patch_count--, have_kcl_patch_count--;
	    used = 0;
	}
    }

    if (!used)
	return false;

    uint i;
    for ( i = 0; i < used && i < kcl_script_list.used; i++ )
    {
	if ( data[i] && data_size[i] )
	{
	    ccp fname = kcl_script_list.field[i];
	    KCL_ACTION_LOG("ScriptKCL() %s\n",fname);

	    ScanInfo_t si;
	    InitializeSI(&si,(ccp)data[i],data_size[i],fname,REVISION_NUM);
	    si.kcl = kcl;
	    si.predef = SetupVarsKCL();

	    kcl->script_tri_removed	= false;
	    kcl->script_tri_added	= false;
	    kcl->script_pt_modified	= false;

	    while (NextLineSI(&si,false,true))
		;  //--- scan source

	    ResetSI(&si);
	}
    }

    //--- transfer results

    bool sort_triangles = false;

    if ( kcl->script_tri_added || kcl->script_pt_modified )
    {
	kcl->model_modified = true;
	CalcNormalsKCL(kcl,true);
	kcl->octree_valid = kcl->min_max_valid = kcl->stat_valid = false;
	if ( KCL_MODE & KCLMD_SORT )
	    sort_triangles = true;
    }

    if (kcl->script_tri_removed)
    {
	if ( KCL_MODE & KCLMD_SORT )
	    sort_triangles = true;
	kcl->octree_valid = false;
	DropSortHelper(kcl,TD_REMOVED,sort_triangles);
	sort_triangles = false;
    }
    else if (sort_triangles)
	DropSortHelper(kcl,0,true);

    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Transform KCL			///////////////
///////////////////////////////////////////////////////////////////////////////

bool TransformKCL
(
    kcl_t		* kcl		// pointer to valid KMP
)
{
    DASSERT(kcl);
    if ( have_patch_count <= 0 || !transform_active || disable_transformation > 0 )
	return false;

    KCL_ACTION_LOG("TransformKCL()\n");

    const uint n_tri = kcl->tridata.used;
    if (!n_tri)
	return false;

    kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    TransformPosDouble3D(td->pt[0].v,n_tri,sizeof(*td));
    TransformPosDouble3D(td->pt[1].v,n_tri,sizeof(*td));
    TransformPosDouble3D(td->pt[2].v,n_tri,sizeof(*td));
    // [[xtridata]]

    kcl_tridata_t *td_end = td + n_tri;
    for ( ; td < td_end; td++ )
	td->status |= TD_INVALID_NORM;

    kcl->octree_valid = kcl->min_max_valid = kcl->stat_valid
		= kcl->norm_valid = false;

    kcl->model_modified = true;
    return true;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Patch KCL			///////////////
///////////////////////////////////////////////////////////////////////////////

kcl_flag_t *patch_kcl_flag = 0; // NULL or 'N_KCL_FLAG' elements

///////////////////////////////////////////////////////////////////////////////

void RemovePatchKclFlag()
{
    if (patch_kcl_flag)
    {
	have_patch_count--, have_kcl_patch_count--;
	FREE(patch_kcl_flag);
	patch_kcl_flag = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

void PurgePatchKclFlag()
{
    if (patch_kcl_flag)
    {
	uint i;
	bool have_patch = false;
	for ( i = 0; i < N_KCL_FLAG; i++ )
	    if ( patch_kcl_flag[i] != i )
	    {
		have_patch = true;
		break;
	    }
	if (!have_patch)
	    RemovePatchKclFlag();
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetPatchKclFlag()
{
    if (!patch_kcl_flag)
    {
	have_patch_count++, have_kcl_patch_count++;
	patch_kcl_flag = MALLOC( N_KCL_FLAG * sizeof(*patch_kcl_flag) );
    }

    for ( int i = 0; i < N_KCL_FLAG; i++ )
	patch_kcl_flag[i] = i;
}

///////////////////////////////////////////////////////////////////////////////

void SetupPatchKclFlag()
{
    if (!patch_kcl_flag)
	ResetPatchKclFlag();
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptKclFlag ( ccp arg )
{
    if (!arg)
    {
	RemovePatchKclFlag();
	return 0;
    }

    ResetPatchKclFlag();
    for(;;)
    {
	//--- skip spaces & ','

	while ( *arg && (uchar)*arg <= ' ' || *arg == ',' )
	    arg++;
	if (!*arg)
	    break;

	//--- find '='

	ccp old = arg;
	for ( ; *arg != '='; arg++ )
	{
	    if ( !*arg || *arg == ',' )
	    {
		ERROR0(ERR_SYNTAX,
			"Missing '=' in parameter of --kcl-flag: %s\n",old);
		goto abort;
	    }
	}
	DASSERT( *arg == '=' );
	arg++;

	//--- scan new value

	u32 new_val, valid;
	arg = ScanNumU32(arg,&valid,&new_val,0,N_KCL_FLAG-1);
	if (!valid)
	{
	    ERROR0(ERR_SYNTAX,
		"Missing number in parameter of --kcl-flag: %s\n",arg);
	    goto abort;
	}
	PRINT("NEW-VAL: - %04x\n",new_val);


	//--- now scan old values (=index) and insert new value into table

	for(;;)
	{
	    //--- skip spaces & '+'

	    while ( *old && (uchar)*old <= ' ' || *old == '+' )
		old++;
	    if ( *old == '=' )
		break;


	    //--- test for 'T' prefix

	    const bool have_t = *old == 't' || *old == 'T';
	    if (have_t)
	    {
		old++;
		while ( *old && (uchar)*old <= ' ' )
		    old++;
	    }


	    //--- scan range

	    u32 old1, old2;
	    old = ScanRangeU32(old,&valid,&old1,&old2,0,
				have_t ? N_KCL_TYPE-1 : N_KCL_FLAG-1 );
	    if (!valid)
	    {
		ERROR0(ERR_SYNTAX,
		    "Missing number in parameter of --kcl-flag: %s\n",old);
		goto abort;
	    }
	    DASSERT( old1 >= 0 );
	    DASSERT( old2 <  N_KCL_FLAG );

	    while ( *old && (uchar)*old <= ' ' )
		old++;


	    //--- scan mask

	    u32 mask = 0xffff;
	    if (have_t)
		mask = N_KCL_TYPE-1;
	    else if ( *old == '/' )
	    {
		old++;
		old = ScanNumU32(old,&valid,&mask,0,N_KCL_FLAG-1);

		while ( *old && (uchar)*old <= ' ' )
		    old++;
	    }

	    if ( !valid || *old != '+' && *old != '=' )
	    {
		ERROR0(ERR_SYNTAX,
			"Missing '=' in parameter of --kcl-flag: %s\n",old);
		goto abort;
	    }


	    //--- normalize params

	    if (mask)
	    {
		const u32 xmask	= (u16)~mask;
		      u32 val	= old1 & mask;
		const u32 end	= old2 | xmask;
		const u32 newv	= new_val & mask;

		PRINT("OLD-VAL: %s %04x..%04x / %04x => %04x..%04x := %04x\n",
			    have_t ? "T" : " ", old1, old2, mask, val, end, newv );

		for ( ; val <= end; val++ )
		{
		    const u16 mval = val & mask;
		    if ( mval >= old1 && mval <= old2 )
			patch_kcl_flag[val] = val & xmask | newv;
		}
	    }
	}
    }
    PurgePatchKclFlag();

 #if defined(DEBUG) && defined(TEST)
    if (patch_kcl_flag)
    {
	PRINT("## HAVE KCL PATCH [%d,%d]\n",have_patch_count,have_kcl_patch_count);
     #if 0
	for ( i = 0; i < N_KCL_FLAG; i++ )
	    if ( patch_kcl_flag[i] != i )
		PRINT("   %04x := %04x\n", i, patch_kcl_flag[i] );
     #endif
    }
 #endif

    return 0;

 abort:
    PRINT("ABORT KCL-FLAG\n");
    RemovePatchKclFlag();
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptTriArea ( ccp arg )
{
    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --tri-area",0);
    si.float_div++;

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    if (!err)
    {
	const double d = GetDoubleV(&temp);
	if ( d >= 0 )
	    opt_tri_area = d;
	CheckEolSI(&si);
	ResetSI(&si);
    }
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptTriHeight ( ccp arg )
{
    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --tri-height",0);
    si.float_div++;

    DEFINE_VAR(temp);
    enumError err = ScanExprSI(&si,&temp);
    if (!err)
    {
	const double d = GetDoubleV(&temp);
	if ( d >= 0 )
	    opt_tri_height = d;
	CheckEolSI(&si);
	ResetSI(&si);
    }
    return err != ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool PatchRawDataKCL
(
    void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ccp			fname		// file name for error messages
)
{
    if ( have_patch_count <= 0 || have_kcl_patch_count <= 0 && !transform_active )
	return false;
    kcl_analyze_t ka;
    if ( IsValidKCL(&ka,data,data_size,data_size,0) >= VALID_ERROR )
	return false;

    KCL_ACTION_LOG("PatchRawDataKCL: %s\n",fname);
    PRINT("** PatchRawDataKCL() **\n");

    kcl_t kcl;
    InitializeKCL(&kcl);
    kcl.fform_outfile = FF_KCL;
    enumError err = ScanRawKCL(&kcl,false,data,data_size,true);
    kcl.fast = false;

    bool stat = false;
    if ( !err && PatchKCL(&kcl) )
    {
	enumError err = CreateRawKCL(&kcl,false);

	if ( !err && kcl.raw_data_size > data_size )
	{
	    KCL_ACTION_LOG(
		"New KCL is %u bytes too large => round normals.\n",
		kcl.raw_data_size - data_size );

	    RoundNormalsKCL(&kcl,16);
	    kcl.model_modified = true;
	    err = CreateRawKCL(&kcl,false);
	}

 #if 0 // this has sometimes negative effect (enlarge the data)
	if ( !err && kcl.raw_data_size > data_size )
	{
	    KCL_ACTION_LOG(
		"New KCL is %u bytes too large => round points.\n",
		kcl.raw_data_size - data_size );

	    RoundPointsKCL(&kcl,1);
	    RoundNormalsKCL(&kcl,15);
	    kcl.model_modified = true;
	    err = CreateRawKCL(&kcl,false);
	}
 #endif

	if ( !err && kcl.raw_data_size > data_size )
	{
	    KCL_ACTION_LOG(
		"New KCL is %u bytes too large => enlarge triangle lists.\n",
		kcl.raw_data_size - data_size );

	    RoundNormalsKCL(&kcl,15);
	    if ( kcl.cube_blow > 400 )
		 kcl.cube_blow = 400;
	    kcl.max_cube_triangles *= 2;
	    kcl.octree_valid        = false;
	    kcl.model_modified	    = true;
	    err = CreateRawKCL(&kcl,false);
	}

	if ( !err && kcl.raw_data_size > data_size )
	{
	    KCL_ACTION_LOG(
		"New KCL is %u bytes too large => new octree param.\n",
		kcl.raw_data_size - data_size );

	    if ( kcl.cube_blow > 250 )
		 kcl.cube_blow = 250;
	    kcl.min_cube_size      *= 2;
	    kcl.max_cube_triangles *= 2;
	    kcl.max_octree_depth   -= 1;
	    kcl.octree_valid        = false;
	    kcl.model_modified	    = true;
	    err = CreateRawKCL(&kcl,false);
	}

	if ( !err && kcl.raw_data_size > data_size )
	{
	    KCL_ACTION_LOG(
		"New KCL is %u bytes too large => new octree param again.\n",
		kcl.raw_data_size - data_size );

	    RoundNormalsKCL(&kcl,14);
	    if ( kcl.cube_blow > 150 )
		 kcl.cube_blow = 150;
	    kcl.octree_valid        = false;
	    kcl.model_modified	    = true;
	    err = CreateRawKCL(&kcl,false);
	}

	if (err)
	    ERROR0(err,"Creation of new KCL failed: %s\n",fname);
	else if ( kcl.raw_data_size > data_size )
	    ERROR0(ERR_WARNING,
		"Creation of new KCL failed,"
		" because new KCL data is too large (%u>%u): %s\n",
		kcl.raw_data_size, data_size, fname );
	else
	{
	    KCL_ACTION_LOG("Replace KCL [inplace], size %u -> %u.\n",
				data_size, kcl.raw_data_size );
	    memcpy(data,kcl.raw_data,kcl.raw_data_size);
	    stat = true;
	}
    }
    ResetKCL(&kcl);
    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool PatchKCL
(
    kcl_t		* kcl		// pointer to valid KCL
)
{
    DASSERT(kcl);
    if ( have_patch_count <= 0 || have_kcl_patch_count <= 0 && !transform_active )
	return false;

    KCL_ACTION_LOG("PatchKCL() KCL=%d\n",kcl->fform_outfile == FF_KCL);

    const bool stat
	= ScriptKCL(kcl,true)
	| DropSortTriangles(kcl)
	| TransformKCL(kcl)
	| ScriptKCL(kcl,false)
	| patch_kcl_flag != 0
	| ( !kcl->octree_valid && kcl->fform_outfile == FF_KCL );

    CheckMetricsTriData( (kcl_tridata_t*)kcl->tridata.list, kcl->tridata.used, kcl );
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			octree iterator			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct oct_iter_t
{
    kcl_t		*kcl;		// pointer to valid KCL
    void		*param;		// user parameter
    oct_func		func;		// call back function

} oct_iter_t;

///////////////////////////////////////////////////////////////////////////////

static int iterate_oct8
(
    const oct_iter_t	*iter,		// iteration parameter
    u32			baseoff,	// offset of current table
    u32			offset,		// offset to handle
    const kcl_cube_t	*cube		// current octree
)
{
    DASSERT(iter);
    DASSERT(iter->kcl);
    DASSERT(iter->func);
    DASSERT(cube);


    if ( offset + 4 > iter->kcl->octree_size )
	return 0;

    u32 val = be32( iter->kcl->octree + offset );
    noPRINT("\t\t\t>>> %8x %8x [%08x]\n",baseoff,offset,val);

    if ( val & 0x80000000 )
    {
	offset = baseoff + ( val & 0x7ffffff ) + 2;
	if ( offset < iter->kcl->octree_size )
	{
	    const u16 *ptr, *list = (u16*)(iter->kcl->octree + offset);
	    for ( ptr = list; be16(ptr); ptr++ )
		;
	    return iter->func(iter->kcl,iter->param,cube,list,ptr-list);
	}
	return 0;
    }

    baseoff += val;
    if ( val && baseoff + 0x20 <= iter->kcl->octree_size )
    {
	const uint cube_size = ( cube->max[0] - cube->min[0] ) / 2;

	uint i;
	for ( i = 0; i < 8; i++ )
	{
	    kcl_cube_t subcube;
	    subcube.min[0] = cube->min[0] + ( i&1 ? cube_size : 0 );
	    subcube.max[0] = subcube.min[0] + cube_size;
	    subcube.min[1] = cube->min[1] + ( i&2 ? cube_size : 0 );
	    subcube.max[1] = subcube.min[1] + cube_size;
	    subcube.min[2] = cube->min[2] + ( i&4 ? cube_size : 0 );
	    subcube.max[2] = subcube.min[2] + cube_size;
	    const int stat = iterate_oct8(iter,baseoff,baseoff+i*4,&subcube);
	    if (stat)
		return stat;
	}
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int OctreeIteratorKCL
(
    kcl_t		*kcl,		// pointer to valid KCL
    oct_func		func,		// call back function
    void		*param		// user parameter
)
{
    DASSERT(kcl);
    DASSERT(func);

    CalcStatisticsKCL(kcl);
    if (!kcl->octree_valid)
	return -1;

    const uint cube_size = 1 << kcl->coord_rshift;

    const uint xn = 1 << kcl->bcube_bits[0];
    const uint yn = 1 << kcl->bcube_bits[1];
    const uint zn = 1 << kcl->bcube_bits[2];

    oct_iter_t iter;
    iter.kcl	= kcl;
    iter.func	= func;
    iter.param	= param;

    u32 offset = 0;

    uint zi;
    for ( zi = 0; zi < zn; zi++ )
    {
	uint yi;
	for ( yi = 0; yi < yn; yi++ )
	{
	    uint xi;
	    for ( xi = 0; xi < xn; xi++ )
	    {
		kcl_cube_t cube;
		cube.min[0] = xi * cube_size;
		cube.max[0] = cube.min[0] + cube_size;
		cube.min[1] = yi * cube_size;
		cube.max[1] = cube.min[1] + cube_size;
		cube.min[2] = zi * cube_size;
		cube.max[2] = cube.min[2] + cube_size;

		iterate_oct8(&iter,0,offset,&cube);
		offset += 4;
	    }
	}
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			calculate blow level		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kcl_blow_t
{
    uint		min_blow;	// minimum blow value

    uint		n_tri;		// number of triangles
    kcl_tri_t		*tlist;		// list of triangles

} kcl_blow_t;

///////////////////////////////////////////////////////////////////////////////

static int calc_blow
(
    kcl_t		*kcl,		// pointer to valid KCL
    void		*param,		// user parameter
    const kcl_cube_t	*cube,		// pointer to current cube geometry
					// relative to kcl->min_octree;
    const u16		*tri_list,	// NULL or pointer to triangle index list
    uint		n_list		// number of elements in 'tri_list'
)
{
    DASSERT(kcl);
    DASSERT(cube);
    DASSERT( tri_list || !n_list );

    kcl_blow_t *blow = param;
    DASSERT(blow);

    noPRINT("** %7u,%7u,%7u .. %7u,%7u,%7u   WD=%7u, N=%5u, D=%6zx, BLOW=%u\n",
		cube->min[0], cube->min[1], cube->min[2],
		cube->max[0], cube->max[1], cube->max[2],
		cube->max[0] - cube->min[0], n_list,
		tri_list ? (u8*)tri_list - kcl->octree : 0, blow->min_blow );

    kcl_cube_t xcube;
    xcube.min[0] = cube->min[0] - blow->min_blow;
    xcube.min[1] = cube->min[1] - blow->min_blow;
    xcube.min[2] = cube->min[2] - blow->min_blow;
    xcube.max[0] = cube->max[0] + blow->min_blow;
    xcube.max[1] = cube->max[1] + blow->min_blow;
    xcube.max[2] = cube->max[2] + blow->min_blow;

    while ( n_list-- > 0 )
    {
	const uint tidx = be16(tri_list++) - 1;

     #if SUPPORT_KCL_CUBE
	// use [[cube]] list param?
	noPRINT("%d/%u -> %d\n",tidx,blow->n_tri,
		OctCubeTriangleOverlaped(&xcube,blow->tlist+tidx,0));

	// use [[cube]] list param?
	if ( tidx >= blow->n_tri
	    || OctCubeTriangleOverlaped(&xcube,blow->tlist+tidx,0) )
	{
	    continue;
	}
     #else
	noPRINT("%d/%u -> %d\n",tidx,blow->n_tri,
		OctCubeTriangleOverlaped(&xcube,blow->tlist+tidx));
	if ( tidx >= blow->n_tri
	    || OctCubeTriangleOverlaped(&xcube,blow->tlist+tidx) )
	{
	    continue;
	}

     #endif


	//--- enlarge the box until all triangles are inside

	const uint grow_size =   512;
	const int max_grow   = 99999;

	uint grow, good_grow = blow->min_blow;
	for ( grow = grow_size; ; grow *= 2 )
	{
	    uint grow_val = blow->min_blow + grow;
	    if ( grow_val > max_grow )
		 grow_val = max_grow;
	    noPRINT("%5u : %6u + %6u = %6u\n", n_list, blow->min_blow, grow, grow_val );

	    kcl_cube_t ycube;
	    ycube.min[0] = cube->min[0] - grow_val;
	    ycube.min[1] = cube->min[1] - grow_val;
	    ycube.min[2] = cube->min[2] - grow_val;
	    ycube.max[0] = cube->max[0] + grow_val;
	    ycube.max[1] = cube->max[1] + grow_val;
	    ycube.max[2] = cube->max[2] + grow_val;

	 #if SUPPORT_KCL_CUBE
	    // use [[cube]] list param?
	    if (OctCubeTriangleOverlaped(&ycube,blow->tlist+tidx,0))
		break;
	 #else
	    if (OctCubeTriangleOverlaped(&ycube,blow->tlist+tidx))
		break;
	 #endif

	    if ( grow_val >= 99999 )
	    {
		blow->min_blow = 99999;
		return 1;
	    }
	    good_grow = grow_val;
	}
	noPRINT("GROW (A) TO %u\n",blow->min_blow);
	blow->min_blow = good_grow;


	//--- now use a binary method to find the border

	for ( grow = grow_size / 2; grow; grow /= 2 )
	{
	    const uint grow_val = blow->min_blow + grow;

	    kcl_cube_t ycube;
	    ycube.min[0] = cube->min[0] - grow_val;
	    ycube.min[1] = cube->min[1] - grow_val;
	    ycube.min[2] = cube->min[2] - grow_val;
	    ycube.max[0] = cube->max[0] + grow_val;
	    ycube.max[1] = cube->max[1] + grow_val;
	    ycube.max[2] = cube->max[2] + grow_val;

	 #if SUPPORT_KCL_CUBE
	    // use [[cube]] list param?
	    if (!OctCubeTriangleOverlaped(&ycube,blow->tlist+tidx,0))
		blow->min_blow = grow_val;
	 #else
	    if (!OctCubeTriangleOverlaped(&ycube,blow->tlist+tidx))
		blow->min_blow = grow_val;
	 #endif
	}
	noPRINT("GROW (B) TO %u\n",blow->min_blow);


	//--- fix the xcube

	xcube.min[0] = cube->min[0] - blow->min_blow;
	xcube.min[1] = cube->min[1] - blow->min_blow;
	xcube.min[2] = cube->min[2] - blow->min_blow;
	xcube.max[0] = cube->max[0] + blow->min_blow;
	xcube.max[1] = cube->max[1] + blow->min_blow;
	xcube.max[2] = cube->max[2] + blow->min_blow;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint CalcBlowSizeKCL ( kcl_t *kcl )
{
    DASSERT(kcl);

    KCL_ACTION_LOG("CalcBlowSizeKCL()\n");

    kcl_blow_t blow;
    memset(&blow,0,sizeof(blow));
    blow.n_tri = kcl->tridata.used;
    blow.tlist = CALLOC(blow.n_tri,sizeof(*blow.tlist));

    uint t;
    kcl_tri_t *tri = blow.tlist;
    const kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    for ( t = 0; t < blow.n_tri; t++, tri++, td++ )
     #if SUPPORT_KCL_CUBE
	setup_oct_tri(kcl,tri,0,td); // use [[cube]] list param?
     #else
	setup_oct_tri(kcl,tri,td);
     #endif

    OctreeIteratorKCL(kcl,calc_blow,&blow);

    FREE(blow.tlist);
    return blow.min_blow;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    CkeckKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct check_kcl_t
{
    //--- param

    const kcl_t		* kcl;		// pointer to valid KCL
    CheckMode_t		mode;		// print mode

    //--- status

    int			warn_count;	// number of warnings
    int			hint_count;	// number of hints
    int			info_count;	// number of infos
    bool		head_printed;	// true: header line printed
    bool		sig_printed;	// true: signature line printed

    //--- misc data

    ColorSet_t		col;		// color setup

} check_kcl_t;

///////////////////////////////////////////////////////////////////////////////
// [[xtridata]]

// predefine function to avoid a warning because of __attribute__ usage
static void print_check_error
(
    check_kcl_t		* chk,		// valid data structure
    CheckMode_t		print_mode,	// NULL | CMOD_WARNING | CMOD_HINT | CMOD_INFO
    ccp			format,		// format of message
    ...					// arguments

) __attribute__ ((__format__(__printf__,3,4)));

static void print_check_error
(
    check_kcl_t		* chk,		// valid data structure
    CheckMode_t		print_mode,	// NULL | CMOD_WARNING | CMOD_HINT | CMOD_INFO
    ccp			format,		// format of message
    ...					// arguments

)
{
    DASSERT(chk);
    DASSERT(chk->kcl);

    ccp col; // message color
    ccp bol; // begin of line
    switch (print_mode)
    {
	case CMOD_WARNING:
	    if (!( chk->mode & CMOD_WARNING ))
		return;
	    col = chk->col.warn;
	    bol = check_bowl;
	    chk->warn_count++;
	    break;

	case CMOD_HINT:
	    if (!( chk->mode & CMOD_HINT ))
		return;
	    col = chk->col.hint;
	    bol = check_bohl;
	    chk->hint_count++;
	    break;

	case CMOD_INFO:
	    if (!( chk->mode & CMOD_INFO ))
		return;
	    col = chk->col.info;
	    bol = check_boil;
	    chk->info_count++;
	    break;

	default:
	    col = 0;
	    bol = 0; // print head only
	    break;
    }

    if (!chk->head_printed)
    {
	chk->head_printed = true;
	if (chk->mode & (CMOD_HEADER|CMOD_FORCE_HEADER) )
	    fprintf(stdlog,"%s* CHECK %s:%s%s\n",
			chk->col.heading, GetNameFF(chk->kcl->fform,0),
			chk->kcl->fname, chk->col.reset );
    }

    if ( !chk->sig_printed && chk->kcl->signature > 0.0 )
    {
	chk->sig_printed = true;
	chk->info_count++;
	fprintf(stdlog,"%s%s%sSignature: %s v%4.2f\n",
		chk->col.info, check_boil, chk->col.reset,
		chk->kcl->signature_info, chk->kcl->signature );
    }

    if (!bol)
	return;

    if (format)
    {
	fputs(col,stdlog);
	fputs(bol,stdlog);
	fputs(chk->col.reset,stdlog);

	va_list arg;
	va_start(arg,format);
	vfprintf(stdlog,format,arg);
	va_end(arg);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[xtridata]]

int CheckKCL
(
    // returns number of errors

    kcl_t		* kcl,		// pointer to valid KCL
    CheckMode_t		mode		// print mode
)
{
    DASSERT(kcl);
    PRINT("CheckKCL(%d)\n",mode);

    if ( disable_checks > 0 )
	return 0;


    //--- setup

    check_kcl_t chk;
    memset(&chk,0,sizeof(chk));
    SetupColorSet(&chk.col,stdlog);
    chk.kcl  = kcl;
    chk.mode = mode;

    if ( mode & CMOD_FORCE_HEADER || kcl->signature > 0.0 )
	print_check_error(&chk,0,0);


    //--- special statistics

    uint count_0 = 0, count_30 = 0, total = 0, n = kcl->tridata.used;
    kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;

    for ( ; n-- > 0; td++ )
	if ( td->cur_flag < N_KCL_FLAG
		&& kcl_type[ td->cur_flag & 0x1f ].attrib & KCLT_DRIVE )
	{
	    total++;
	    if ( td->status & TD_INVALID_NORM )
		CalcNormalsTriData(kcl,td,1);
	    if ( td->normal[0].y < 0.0 )
	    {
		count_0++;
		if ( td->normal[0].y < -0.5 )
		    count_30++;
	    }
	}
    if (count_0)
	print_check_error(&chk,CMOD_HINT,
		"%u of %u drivable triangle%s %s face down"
		" => --kcl=RM-FACEDOWN\n",
		count_0, total,
		total == 1 ? "" : "s",
		count_0 == 1 ? "is" : "are" );
    if (count_30)
	print_check_error(&chk,CMOD_HINT,
		"%u of %u drivable triangle%s %s face down (>30°).\n",
		count_30, total,
		total == 1 ? "" : "s",
		count_30 == 1 ? "is" : "are" );


    //--- analyze statistics

    if (kcl->octree_valid)
    {
	CalcStatisticsKCL(kcl);
	if ( kcl->n_tri_used < kcl->tridata.used )
	{
	    uint unused = kcl->tridata.used - kcl->n_tri_used;
	    print_check_error(&chk,CMOD_HINT,
		    "%u of %u triangle%s obsolete"
		    " (not referenced by the octree)"
		    " => --kcl=DROP-UNUSED\n",
		    unused, kcl->tridata.used,
		    unused == 1 ? " is" : "s are" );
	}
    }

    if (kcl->tri_fix_count)
	print_check_error(&chk,CMOD_HINT,
		"%u of %u triangle%s had extreme values"
		" => --kcl=DROP-FIXED\n",
		kcl->tri_fix_count, kcl->tridata.used,
		kcl->tridata.used == 1 ? "" : "s" );

    if (kcl->tri_inv_count)
	print_check_error(&chk,CMOD_WARNING,
		"%u of %u triangle%s invalid values"
		" => --kcl=DROP-INVALID\n",
		kcl->tri_inv_count, kcl->tridata.used,
		kcl->tridata.used == 1 ? " has" : "s have" );

    if (kcl->n_invalid_tri_ref)
	print_check_error(&chk,CMOD_WARNING,
		"The octree contains %u invalid triangle reference%s!\n",
		kcl->n_invalid_tri_ref,
		kcl->n_invalid_tri_ref == 1 ? "" : "s" );


    //--- terminate

    if ( mode & CMOD_GLOBAL_STAT )
    {
	global_warn_count += chk.warn_count;
	global_hint_count += chk.hint_count;
	global_info_count += chk.info_count;
    }

    if ( mode & (CMOD_FOOTER|CMOD_FORCE_FOOTER) )
    {
	static ccp hint
	    = " => see https://szs.wiimm.de/cmd/wkclt/check#desc for more info.";

	ccp bind;
	char hbuf[60], ibuf[60];
	if (chk.info_count)
	{
	    snprintf(ibuf,sizeof(ibuf)," and %s%u info%s%s",
		chk.col.info,
		chk.info_count, chk.info_count>1 ? "s" : "",
		chk.col.heading );
	    bind = ",";
	}
	else
	{
	    *ibuf = 0;
	    bind = " and";
	}

	if (chk.hint_count)
	    snprintf(hbuf,sizeof(hbuf),"%s %s%u hint%s%s",
		bind, chk.col.hint,
		chk.hint_count, chk.hint_count>1 ? "s" : "",
		chk.col.heading );
	else
	    *hbuf = 0;

	if (chk.warn_count)
	    fprintf(stdlog,
			" %s=> %s%u warning%s%s%s%s for %s:%s\n%s%s%s\n\n",
			chk.col.heading, chk.col.warn,
			chk.warn_count, chk.warn_count == 1 ? "" : "s",
			chk.col.heading,
			hbuf, ibuf, GetNameFF(kcl->fform,0), kcl->fname,
			chk.col.heading, hint, chk.col.reset );
	else if ( mode & CMOD_FORCE_FOOTER || chk.hint_count )
	    fprintf(stdlog," %s=> No warnings%s%s for %s:%s\n%s%s%s\n\n",
			chk.col.heading, hbuf, ibuf, GetNameFF(kcl->fform,0), kcl->fname,
			chk.col.heading, hint, chk.col.reset );
    }

    return chk.warn_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			kcl parser functions		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_n
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);

    res->i = si && si->kcl ? si->kcl->tridata.used : 0;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_pt
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    if (si)
    {
	const int pt_index  = GetIntV(param+1);
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if ( td && pt_index >= 0 && pt_index <= 2 )
	{
	    AssignVectorV(res,td->pt+pt_index);
	    return ERR_OK;
	}
    }

    res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_length
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);

    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    if ( td->status & TD_INVALID_NORM )
		CalcNormalsTriData(si->kcl,td,1);
	    res->d = td->length;
	    res->mode = VAR_DOUBLE;
	    return ERR_OK;
	}
    }

    res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_normal
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    if (si)
    {
	const int pt_index  = GetIntV(param+1);
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if ( td && pt_index >= 0 && pt_index <= 3 )
	{
	    if ( td->status & TD_INVALID_NORM )
		CalcNormalsTriData(si->kcl,td,1);
	    float *f = td->normal[pt_index].v;
	    res->x = *f++;
	    res->y = *f++;
	    res->z = *f;
	    res->mode = VAR_VECTOR;
	    return ERR_OK;
	}
    }

    res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_setPt
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=3);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    if ( n_param > 3 )
	    {
		td->pt[0] = GetVectorV(param+1);
		td->pt[1] = GetVectorV(param+2);
		td->pt[2] = GetVectorV(param+3);
		status = 1;
	    }
	    else
	    {
		const int pt_index = GetIntV(param+1);
		if ( pt_index >= 0 && pt_index <= 2 )
		{
		    td->pt[pt_index] = GetVectorV(param+2);
		    status = 1;
		}
	    }
	    if (status>0)
	    {
		td->status |= TD_INVALID_NORM;
		si->kcl->script_pt_modified = true;
	    }
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_scale
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    Var_t *v = ToVectorV(param+1);
	    if ( n_param > 2 )
	    {
		Var_t *origin = ToVectorV(param+2);
		uint p;
		for ( p = 0 ; p < 3; p++ )
		{
		    td->pt[p].x = ( td->pt[p].x - origin->x ) * v->x + origin->x;
		    td->pt[p].y = ( td->pt[p].y - origin->y ) * v->y + origin->y;
		    td->pt[p].z = ( td->pt[p].z - origin->z ) * v->z + origin->z;
		}
	    }
	    else
	    {
		uint p;
		for ( p = 0 ; p < 3; p++ )
		{
		    td->pt[p].x *= v->x;
		    td->pt[p].y *= v->y;
		    td->pt[p].z *= v->z;
		}
	    }
	    td->status |= TD_INVALID_NORM;
	    si->kcl->script_pt_modified = true;
	    status = 1;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_shift
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    Var_t *v = ToVectorV(param+1);
	    uint p;
	    for ( p = 0 ; p < 3; p++ )
	    {
		td->pt[p].x += v->x;
		td->pt[p].y += v->y;
		td->pt[p].z += v->z;
	    }
	    td->status |= TD_INVALID_NORM;
	    si->kcl->script_pt_modified = true;
	    status = 1;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_hRot
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    const double hrot_rad = GetDoubleV(param+1) * (M_PI/180.0);

	    if ( n_param > 2 )
	    {
		Var_t *origin = ToVectorV(param+2);

		uint p;
		double3 *d3 = td->pt;
		for ( p = 0 ; p < 3; p++, d3++ )
		{
		    const double dx  = d3->x - origin->x;
		    const double dz  = d3->z - origin->z;
		    const double rad = atan2(dx,dz) + hrot_rad;
		    const double len = sqrt(dx*dx+dz*dz);

		    d3->x = sin(rad) * len;
		    d3->z = cos(rad) * len;
		}
	    }
	    else
	    {

		uint p;
		double3 *d3 = td->pt;
		for ( p = 0 ; p < 3; p++, d3++ )
		{
		    const double rad = atan2(d3->x,d3->z) + hrot_rad;
		    const double len = sqrt(d3->x*d3->x+d3->z*d3->z);

		    d3->x = sin(rad) * len;
		    d3->z = cos(rad) * len;
		}
	    }
	    td->status |= TD_INVALID_NORM;
	    si->kcl->script_pt_modified = true;
	    status = 1;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_remove
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    status = ( td->status & TD_REMOVED ) != 0;
	    td->status |= TD_REMOVED;
	    si->kcl->script_tri_removed = true;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_unremove
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    status = ( td->status & TD_REMOVED ) != 0;
	    td->status &= ~TD_REMOVED;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_isRemoved
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	    status = ( td->status & TD_REMOVED ) != 0;
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_create
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=4);

    int status = -1;
    if ( si && si->kcl )
    {
	uint i;
	for ( i = 1; i < n_param; i++ )
	    ToVectorV(param+i);

	kcl_tridata_t *td = AppendTrianglesKCLp(
					si->kcl,
					GetIntV(param),
					&param[1].d3,
					sizeof(*param),
					n_param-1 );
	if (td)
	{
	    si->kcl->script_tri_added = true;
	    status = td - (kcl_tridata_t*)si->kcl->tridata.list;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_createOctahedron
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=3);

    int status = -1;
    if ( si && si->kcl )
    {
	ToVectorV(param+1);
	ToVectorV(param+2);
	octahedron_t oct;
	if ( n_param > 3 )
	{
	    ToVectorV(param+3);
	    CreateOctahedron( &oct, &param[2].d3, &param[3].d3, &param[1].d3 );
	}
	else
	    CreateOctahedron( &oct, &param[2].d3, 0, &param[1].d3 );

	kcl_tridata_t *td = AppendOctahedronKCL( si->kcl, GetIntV(param), &oct );
	if (td)
	{
	    si->kcl->script_tri_added = true;
	    status = td - (kcl_tridata_t*)si->kcl->tridata.list;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_createCuboid
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=3);

    int status = -1;
    if ( si && si->kcl )
    {
	ToVectorV(param+1);
	ToVectorV(param+2);
	cuboid_t cub;
	if ( n_param > 3 )
	{
	    ToVectorV(param+3);
	    CreateCuboid( &cub, &param[2].d3, &param[3].d3, &param[1].d3 );
	}
	else
	    CreateCuboid( &cub, &param[2].d3, 0, &param[1].d3 );

	kcl_tridata_t *td = AppendCuboidKCL( si->kcl, GetIntV(param), &cub );
	if (td)
	{
	    si->kcl->script_tri_added = true;
	    status = td - (kcl_tridata_t*)si->kcl->tridata.list;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_createPyramid
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=4);

    int status = -1;
    if ( si && si->kcl )
    {
	uint i;
	for ( i = 1; i < n_param; i++ )
	    ToVectorV(param+i);

	kcl_tridata_t *td = AppendPyramidKCL(
					si->kcl,
					GetIntV(param),
					&param[1].d3,
					&param[2].d3,
					sizeof(*param),
					n_param-2,
					false );
	if (td)
	{
	    si->kcl->script_tri_added = true;
	    status = td - (kcl_tridata_t*)si->kcl->tridata.list;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_createPrism
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
					//  * fpar->user_id (bit field)
					//	bit 0: anti prism
					//	bit 1: arrow
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=6);
    DASSERT(fpar);

    int status = -1;
    if ( si && si->kcl )
    {

	// tri$createPrism( kcl_flag, pt1,pt2, ptX, r,n [,base_flag] [,base_height] )
	// tri$createArrow( kcl_flag, pt1,pt2, ptX, r,n,
	//			arrow_flag [,arrow_size] [,base_flag] [,base_height] )

	ToVectorV(param+1);
	ToVectorV(param+2);
	ToVectorV(param+3);

	int base_flag, arrow_flag;
	double base_height, arrow_size;
	if ( fpar->user_id & 2 )
	{
	    DASSERT( n_param >= 7 );
	    arrow_flag	= GetIntV(param+6);
	    arrow_size	= n_param > 7 ? GetDoubleV(param+7) : 1.0;
	    base_flag	= n_param > 8 ? GetIntV(param+8)    : -1;
	    base_height	= n_param > 9 ? GetDoubleV(param+9) : 0.0;
	}
	else
	{
	    arrow_flag	= -1;
	    arrow_size	= 0.0;
	    base_flag	= n_param > 6 ? GetIntV(param+6)    : -1;
	    base_height	= n_param > 7 ? GetDoubleV(param+7) : 0.0;
	}

	kcl_tridata_t *td
	    = AppendPrismKCL(	si->kcl,
				GetIntV(param),
				&param[1].d3,
				&param[2].d3,
				&param[3].d3,
				GetDoubleV(param+4),
				GetIntV(param+5),
				fpar->user_id & 1,
				base_flag,
				base_height,
				arrow_flag,
				arrow_size,
				false );
	if (td)
	{
	    si->kcl->script_tri_added = true;
	    status = td - (kcl_tridata_t*)si->kcl->tridata.list;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_createJoist
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=5);

    int status = -1;
    if ( si && si->kcl )
    {
	ToVectorV(param+2);
	ToVectorV(param+3);
	ToVectorV(param+4);

	kcl_tridata_t *td = AppendJoistKCL(
					si->kcl,
					GetIntV(param),
					GetDoubleV(param+1),
					&param[2].d3,
					&param[3].d3,
					&param[4].d3,
					n_param > 5 ? GetIntV(param+5) : 0,
					n_param > 6 ? GetIntV(param+6) : 0 );
	if (td)
	{
	    si->kcl->script_tri_added = true;
	    status = td - (kcl_tridata_t*)si->kcl->tridata.list;
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_ptsInCuboid
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=3);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    status = 0;
	    ToVectorV(param+1);
	    ToVectorV(param+2);

	    double3 *d3 = td->pt;
	    uint p;
	    for ( p = 0; p < 3; p++, d3++ )
	    {
		if (    d3->x >= param[1].x  &&  d3->x <= param[2].x
		     && d3->y >= param[1].y  &&  d3->y <= param[2].y
		     && d3->z >= param[1].z  &&  d3->z <= param[2].z
		   )
		{
		    status++;
		}
	    }
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_OverlapCube
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=3);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    status = OverlapCubeTri(
				&ToVectorV(param+1)->d3,
				GetDoubleV(param+2),
				td->pt+0,
				td->pt+1,
				td->pt+2 );
	}
    }

    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_flag
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);
    DASSERT(fpar);

    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    res->i = td->cur_flag;
	    if ( n_param > 1 )
	    {
		const int mode = GetIntV(param+1);
		if ( mode == 2 && patch_kcl_flag )
		    res->i = patch_kcl_flag[td->in_flag];
		else if ( mode == 1 || mode == 2 )
		    res->i = td->in_flag;
	    }
	    if (fpar->user_id)
		res->i = GET_KCL_TYPE(res->i);
	    res->mode = VAR_INT;
	    return ERR_OK;
	}
    }

    res->mode = VAR_UNSET;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_setFlag
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=2);

    int status = -1;
    if (si)
    {
	kcl_tridata_t *td = GetTridataKCL(si->kcl,GetIntV(param));
	if (td)
	{
	    status = td->cur_flag;
	    td->cur_flag = GetIntV(param+1);
	    // no 'flag changed' status needed
	}
    }
    res->i = status;
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_defColor
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);

    const u32 col0 = GetIntV(param);
    Color_t col;
    col.a = col0 >> 24;
    col.r = col0 >> 16;
    col.g = col0 >>  8;
    col.b = col0;

    switch (n_param)
    {
	case 1:
	    if (!col.a)
		col.a = 0xff;
	    break;

	case 2:
	    col.a   = GetIntV(param+1);
	    break;

	case 4:
	    col.a = GetIntV(param+3);
	case 3:
	    col.r = GetIntV(param+0);
	    col.g = GetIntV(param+1);
	    col.b = GetIntV(param+2);
	    break;
    }

    int ci;
    for ( ci = 0; ci < n_user_color; ci++ )
	if ( user_color[ci].val == col.val )
	    break;

    if ( ci >= N_KCL_USER_FLAGS )
	res->i = -1;
    else
    {
	if ( ci == n_user_color )
	    user_color[n_user_color++] = col;
	res->i = ci + KCL_FLAG_USER_0;
    }
    res->mode = VAR_INT;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError F_tri_getColor
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const FuncParam_t	* fpar		// pointer to user parameter
)
{
    DASSERT(res);
    DASSERT(param);
    DASSERT(n_param>=1);

    const uint idx = GetIntV(param);
    if ( idx >= KCL_FLAG_USER_0 && idx <= KCL_FLAG_USER_MAX )
    {
	Color_t col = user_color[ idx - KCL_FLAG_USER_0];
	res->i = col.a << 24 | col.r << 16 | col.g << 8 | col.b;
	res->mode = VAR_INT;
    }
    else
	res->mode = VAR_UNSET;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const struct FuncTable_t func_tab[] =
{
    { 0, 0, "TRI$N", F_tri_n, 0,
	"vector", "tri$n()",
	"This function returns the number of triangles." },

    { 2, 2, "TRI$PT", F_tri_pt, 0,
	"vector", "tri$pt(tri_index,pt_index)",
	"This function returns the point with index 'pt_index' (0..2)"
	" of triangle 'tri_index' (0..)." },

    { 1, 1, "TRI$LENGTH", F_tri_length, 0,
	"float", "tri$length(tri_index)",
	"This function returns the length of triangle 'tri_index' (0..)." },

    { 2, 2, "TRI$NORMAL", F_tri_normal, 0,
	"vector", "tri$normal(tri_index,norm_index)",
	"This function returns the normal with index 'norm_index' (0..3)"
	" of triangle 'tri_index' (0..)." },

    { 3, 4, "TRI$SETPT", F_tri_setPt, 0,
	"int", "tri$setPt(tri_index,pt0,pt1,pt2)",
	"This function sets the coordinates of all 3 points"
	" of the triangle 'tri_index' (0..) to the vectors 'pt*'." },
    { 0, 0, 0, F_tri_setPt, 0,
	"int", "tri$setPt(tri_index,pt_index,pt)",
	"This function sets the coordinates of the single point"
	" with index 'pt_index' (0..2) of triangle 'tri_index' (0..)."
	" to the vector 'pt'." },

    { 2, 3, "TRI$SCALE", F_tri_scale, 0,
	"int", "tri$scale(tri_index,scale[,origin])",
	"This function scales the triangle 'tri_index' (0..)"
	" by the vector 'scale' relative to point 'origin'."
	" If the origin is not set, v(0,0,0) is used instead." },

    { 2, 2, "TRI$SHIFT", F_tri_shift, 0,
	"int", "tri$shift(tri_index,shift)",
	"This function adds the vector 'shift' to all 3 points"
	" of the triangle 'tri_index' (0..)." },

    { 2, 3, "TRI$HROT", F_tri_hRot, 0,
	"int", "tri$hRot(tri_index,degree[,origin])",
	"This function rotates the triangle 'tri_index' (0..)"
	" horizontal counterclockwise by the angle 'degree'"
	" around the point 'origin'."
	" If the origin is not set, v(0,0,0) is used instead." },

    { 1, 1, "TRI$REMOVE", F_tri_remove, 0,
	"int", "tri$remove(tri_index)",
	"This function marks the triangle 'tri_index' (0..) as 'REMOVED'."
	" The return value is -1 for an invalid index,"
	" 1 if the triangle was already removed or 0 if not." },

    { 1, 1, "TRI$UNREMOVE", F_tri_unremove, 0,
	"int", "tri$unremove(tri_index)",
	"This function clears the 'REMOVED' marker of the triangle 'tri_index' (0..)."
	" The return value is -1 for an invalid index,"
	" 1 if the triangle was removed or 0 if not." },

    { 1, 1, "TRI$ISREMOVED", F_tri_isRemoved, 0,
	"int", "tri$isRemoved(tri_index)",
	"This returns the status of the 'REMOVED' marker"
	" of the triangle 'tri_index' (0..)."
	" The return value is -1 for an invalid index,"
	" 1 if the triangle is marked as removed or 0 if not." },

    { 4, MAX_FUNC_PARAM, "TRI$CREATE", F_tri_create, 0,
	"int", "tri$create(kcl_flag,pt0,pt1,pt2,...)",
	"This function appends new triangles at the end of the triangle list"
	" with the entered parameters."
	" If more than 3 points are entered, all points must be in the same plane"
	" and the polygon is split into multiple (N-2) triangles."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 3, 4, "TRI$CREATEOCTAHEDRON", F_tri_createOctahedron, 0,
	"int", "tri$createOctahedron(kcl_flag,pos,radius[,rot])",
	"This function creates a octahedron by adding 8 triangles."
	" If 'radius' is a scalar, a regular octahedron created."
	" Otherwise it is a vector and define different radiuses for each axis."
	" If 'rot' is set, the octahedron is rotated."
	" As last operation the center (origin) of the cube is shifted to 'pos'."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 3, 4, "TRI$CREATECUBOID", F_tri_createCuboid, 0,
	"int", "tri$createCuboid(kcl_flag,pos,size[,rot])",
	"This function creates a cuboid by adding 12 triangles."
	" If 'size' is a scalar, a cube with the entered edge length is created."
	" If 'rot' is set, the cube is rotated."
	" As last operation the center (origin) of the cube is shifted to 'pos'."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 4, MAX_FUNC_PARAM, "TRI$CREATEPYRAMID", F_tri_createPyramid, 0,
	"int", "tri$createPyramid(kcl_flag,apex,pt1,pt2,...)",
	"This function creates a pyramid by adding N triangles."
	" 'pt1'..'ptN' descibribes the base polygon, which is not created."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 6, 8, "TRI$CREATEPRISM", F_tri_createPrism, 0,
	"int", "tri$createPrism(kcl_flag,pt1,pt2,ptX,r,n[,base_flag][,base_height])",
	"This function creates a prism with radius 'r' and N faces"
	" from 'pt1' to 'pt2' by adding 2*N triangles."
	" The point 'ptX' is used for the orientation (direction of first edge)."
	" If 'base_flag' is set and not @-1@, the base is printed with ths kcl flag value."
	" If 'base_height' is set, a pyramid is drawn as base."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 6, 8, "TRI$CREATEANTIPRISM", F_tri_createPrism, 1,
	"int", "tri$createAntiPrism(kcl_flag,pt1,pt2,ptX,r,n[,base_flag][,base_height])",
	"This function creates an antiprism with radius 'r' and N faces"
	" from 'pt1' to 'pt2' by adding 2*N triangles."
	" The point 'ptX' is used for the orientation (direction of first edge)."
	" If 'base_flag' is set and not @-1@, the base is printed with ths kcl flag value."
	" If 'base_height' is set, a pyramid is drawn as base."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 7, 10, "TRI$CREATEARROW", F_tri_createPrism, 2,
	"int", "tri$createArrow(flag,pt1,pt2,ptX,r,n,a_flag[,a_size][,b_flag][,b_ht])",
	"This function creates a prism based arrow with radius 'r' and N faces"
	" from 'pt1' to 'pt2', where 'pt2' is the place of the arrowhead."
	" The point 'ptX' is used for the orientation (direction of first edge)."
	" 'flag', 'a_flag' and 'b_flag' are KCL flags to define the color."
	" 'a_size' is the addition size factor of the arrowhead, the default is 1.0."
	" If 'b_flag' is not set or @-1@, no base is drawn."
	" If 'base_height' is set, a pyramid is drawn as base."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 7, 10, "TRI$CREATEANTIARROW", F_tri_createPrism, 3,
	"int", "tri$createAntiArrow(flag,pt1,pt2,ptX,r,n,a_flag[,a_size][,b_flag][,b_ht])",
	"This function creates an antiprism based arrow with radius 'r' and N faces"
	" from 'pt1' to 'pt2', where 'pt2' is the place of the arrowhead."
	" The point 'ptX' is used for the orientation (direction of first edge)."
	" 'flag', 'a_flag' and 'b_flag' are KCL flags to define the color."
	" 'a_size' is the addition size factor of the arrowhead, the default is 1.0."
	" If 'b_flag' is not set or @-1@, no base is drawn."
	" If 'base_height' is set, a pyramid is drawn as base."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 5, 6, "TRI$CREATEJOIST", F_tri_createJoist, 0,
	"int", "tri$createJoist(kcl_flag,length,pt1,pt2,pt_dir[,n_marker][,mark5_flag])",
	"This function appends a triangular joist from 'p1' to 'p2'"
	" to at the end of the triangle list."
	" 'length' the length of the body"
	" and 'pt_dir' is a helper point for its direction."
	" If 'n_marker' is >0, markers representing the number are added."
	" If 'mark5_flag' is >0, every 5 markers are replaced by one of the entered flag."
	" The function returns the index of the first new triangle or -1 if failed." },

    { 3, 3, "TRI$PTSINCUBOID", F_tri_ptsInCuboid, 0,
	"int", "tri$ptsInCuboid(tri_index,cube_min,cube_max)",
	" Parameters 'cube_*' are converted to vectors."
	" They describe 2 diagonal corners of a rectangular cuboid,"
	" assuming that 'cube_min<=cube_max' is true for each coordinate."
	" The functions returns the number of points"
	" of the triangle 'tri_index' (0..),"
	" that are inside of the cube including the border."
	" If 'tri_index' is invalid, -1 is returned." },

 #ifdef TEST // [[2do]]
    { 3, 3, "TRI$OVERLAPCUBE", F_tri_OverlapCube, 1,
	"int", "tri$OverlapCube(tri_index,cube_mid,cube_width)",
	" 'tri_index' (0..) is a triangle index."
	" 'cube_mid' is the middle point of a cube"
	" and 'cube_width' the length of its edges."
	" The function returns -1 for an invalid triangle index,"
	" or 1, if the cube and the triangle are overlapped."
	" Otherwise 0 is returned." },
 #endif

    { 1, 2, "TRI$FLAG", F_tri_flag, 0,
	"int", "tri$flag(tri_index[,mode])",
	"This function returns the KCL flag of the triangle 'tri_index' (0..)."
	" If mode is '1', the original source value is returned."
	" If mode is '2', the original source value "
	" transformed by --kcl-flag is returned;"
	" if --kcl-flag is not set, it is the same as mode '1'."
	" Otherwise the current value is returned."
	" The current value is initialized by the value of mode '2'."
	" Values >0xffff are possible and represent internal drawings." },

    { 1, 2, "TRI$TYPE", F_tri_flag, 1,
	"int", "tri$type(tri_index[,mode])",
	"This function returns the KCL type of the triangle 'tri_index' (0..)."
	" It works like tri$flag(), but the flag is transformed to a type."
	" The return value is in the range of 0x00..0x1f (5 lowest bits of the flag),"
	" if it is a real KCL flag (<0x10000), or -1 otherwise (special type)." },

    { 2, 2, "TRI$SETFLAG", F_tri_setFlag, 0,
	"int", "tri$setFlag(tri_index,new_flag)",
	"This function sets the new KCL flag of triangle 'tri_index' (0..)."
	" The function returns -1 for an invalid index"
	" or the previous value of the KCL flag." },

    { 1, 4, "TRI$DEFCOLOR", F_tri_defColor, 0,
	"int", "tri$defColor(argb[,alpha])",
	"Define a new color."
	" 'ARGB' is a bytes coded color: alpha,red,green,blue."
	" If 'ALPHA' is not set, the highest byte of 'ARGB' is used."
	" If this highest byte is null, 0xff (no transparency) is used."
	" The function returns -1, if already 200 colors defined."
	" Otherwise it returns the virtual KCL flag for the user defined color." },
    { 0, 0, 0, F_tri_defColor, 0,
	"int", "tri$defColor(red,green,blue[,alpha])",
	"Define a new color."
	" If ALPHA is not set, 0xff (no transparency) is used."
	" For the 3 colors, values between 0x00 and 0xff are expected."
	" The function returns -1, if already 200 colors defined."
	" Otherwise it returns the virtual KCL flag for the user defined color." },

    { 1, 1, "TRI$GETCOLOR", F_tri_getColor, 0,
	"int", "tri$getColor(index)",
	"This function returns the color defined by a previous tri$defColor() call."
	" INDEX is the virtual KCL flag for the user defined color."
	" On error, the value NONE (type 'undefined') is returned." },

    {0,0,0,0,0,0,0,0}
};

// [[2do]] ideas:
//  tri$isAdded(tindex) : true, if triangle is marked as 'added'
//  tri$isFixed(tindex) : true, if triangle is marked as 'fixed'
//  tri$isUsed(tindex) : true, if triangle is marked as 'used'

//
///////////////////////////////////////////////////////////////////////////////
///////////////			setup kcl parser		///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsKCL()
{
    static VarMap_t vm = {0};
    if (!vm.used)
    {
	//---- define functions

	DefineParserFuncTab(func_tab,FF_KCL);
	SetupReferenceKCL(&vm);

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
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


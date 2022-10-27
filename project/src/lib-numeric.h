
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

#ifndef SZS_LIB_NUMERIC_H
#define SZS_LIB_NUMERIC_H 1

#define _GNU_SOURCE 1

#include "dclib-numeric.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  defines			///////////////
///////////////////////////////////////////////////////////////////////////////

#define EXPAND8(a) ( (a) + 7 & ~(u32)7 )

///////////////////////////////////////////////////////////////////////////////

#define MAX_LOOP_DEPTH		10 // max loop depth
#define MAX_FUNCTION_DEPTH	10 // max macro depth
#define MAX_SOURCE_DEPTH	50 // max open source files/macros

///////////////////////////////////////////////////////////////////////////////

struct Var_t;
struct FuncParam_t;
struct ScanMacro_t;
struct ScanFile_t;
struct ScanInfo_t;

///////////////////////////////////////////////////////////////////////////////

enum
{
	RTL_N_NONE	= 0,
	RTL_N_HIDE,
	RTL_N_SHOW,
	RTL_N_SWAP,

	RTL_N_M_MODE	= 0x0ff,	// mask for modes above

	//--- flags

	RTL_N_F_HEX	= 0x0100,	// use hex codes for standard track files
	RTL_N_F_WII	= 0x0200,	// Prefix 'Wii' on Wii tracks

	RTL_N_F_RED1	= 0x001000,	// color console with 'red1' (alias 'red')
	RTL_N_F_RED2	= 0x002000,	// color console with 'red2'
	RTL_N_F_RED3	= 0x004000,	// color console with 'red3'
	RTL_N_F_RED4	= 0x008000,	// color console with 'red4'
	RTL_N_F_YELLOW	= 0x010000,	// color console with 'yellow'
	RTL_N_F_GREEN	= 0x020000,	// color console with 'green'
	RTL_N_F_BLUE1	= 0x040000,	// color console with 'blue1' (alias 'blue')
	RTL_N_F_BLUE2	= 0x080000,	// color console with 'blue2'
	RTL_N_F_WHITE	= 0x100000,	// color console with 'white'
	RTL_N_F_CLEAR	= 0x200000,	// color console with 'clear'
};

///////////////////////////////////////////////////////////////////////////////
// slots

#define SLOT_WAIT		   0x43	 // online: no track selected yet
#define SLOT_RANDOM		   0xff	 // online: random track selected

#define LE_SLOT_RND_ALL		0x3e
#define LE_SLOT_RND_ORIGINAL	0x3f
#define LE_SLOT_RND_CUSTOM	0x40
#define LE_SLOT_RND_NEW		0x41

#define LE_SLOT_RND_BEG		0x3e
#define LE_SLOT_RND_END		0x42

//-----------------------------------------------------------------------------
// bits for [[le_flags8_t]] [[le_flags_t]]

typedef u8  le_flags8_t;	// type of LE-CODE track flags (classic 8 bits)
typedef u16 le_flags_t;		// type of LE-CODE track flags since build 35

#define LEFL_NEW	0x01	// new track
#define LEFL_RND_HEAD	0x02	// first track of random group
#define LEFL_RND_GROUP	0x04	// member of random group, but not first
				// members are not added to cups
#define LEFL_ALIAS	0x08	// use TrackByAliasLE(prop,music)
				// as alias for another track
#define LEFL_TEXTURE	0x10	// track is declared as texture hack of same property slot
#define LEFL_HIDDEN	0x20	// exclude a track from cups.

#define LEFL__RND	0x06	// both random flags
#define LEFL__ALL	0x3f	// all bits of above

// flags calculated by CopyLecodeFlags()
#define LEFL_BATTLE	0x0100	// track is a battle arena
#define LEFL_VERSUS	0x0200	// track is a versus track
#define LEFL_CUP	0x0400	// track is refefrenced by a cup
#define LEFL_ORIG_CUP	0x0800	// track is refefrenced by a original cup
#define LEFL_CUSTOM_CUP	0x1000	// track is refefrenced by a custom cup
#define LEFL_RANDOM	0x2000	// slot id LE-CODE random
#define LEFL__XALL	0x3f3f	// all bits of above

static inline bool IsTitleLEFL ( uint flags )
	{ return (flags&(LEFL_RND_HEAD|LEFL_RND_GROUP)) == LEFL_RND_HEAD; }

static inline bool IsHiddenLEFL ( uint flags )
	{ return (flags&LEFL_HIDDEN)
		||  (flags&(LEFL_RND_HEAD|LEFL_RND_GROUP)) == LEFL_RND_GROUP; }

static inline bool IsRandomLEFL ( uint flags )
	{ return (flags&(LEFL_RND_HEAD|LEFL_RND_GROUP)) != 0; }

ccp PrintLEFL8  ( le_flags_t flags, bool aligned );
ccp PrintLEFL16 ( le_flags_t flags, bool aligned );

static inline ccp PrintLEFL ( uint flags_bits, le_flags_t flags, bool aligned )
	{ return ( flags_bits == 16 ? PrintLEFL16 : PrintLEFL8 )(flags,aligned); }

typedef ccp (*PrintLEFL_func) ( le_flags_t flags, bool aligned );
static inline PrintLEFL_func GetPrintLEFL ( uint flags_bits )
	{ return flags_bits == 16 ? PrintLEFL16 : PrintLEFL8; }

//-----------------------------------------------------------------------------
// some more constants

enum
{
    LE_DISABLED		=    0,	// setting is dsiabled
    LE_ENABLED		= 0x01,	// setting is enabled
    LE_ALTERABLE	= 0x02,	// setting can be altered by player
    LE_EXCLUDED		= 0x04,	// exclude some features or objects
    LE_INCLUDED		= 0x08,	// include some features or objects

    LE_M_TEXTURE	= LE_ENABLED | LE_ALTERABLE | LE_EXCLUDED,
};

//-----------------------------------------------------------------------------
// [[lpar_mode_t]]

typedef enum lpar_mode_t
{
    LPM_PRODUCTIVE,
    LPM_TESTING,
    LPM_EXPERIMENTAL,
    LPM_FORCE_AUTOMATIC,
    LPM_AUTOMATIC,

    LPM__DEFAULT = LPM_AUTOMATIC
}
lpar_mode_t;

//-----------------------------------------------------------------------------
// [[kmp_gmode_t]] settings for KMP GAME_MODE

typedef enum kmp_gmode_t
{
    KMP_GMODE_BALLOON,		// balloon battle
    KMP_GMODE_COIN,		// coin runners
    KMP_GMODE_VERSUS,		// versus
    KMP_GMODE_ITEMRAIN,		// itemrain
    KMP_GMODE__N,

    KMP_GMODE_AUTO = 14,	// special value: don't simulate a mode behaviour
    KMP_GMODE_STD  = 15,	// special value: simulate standard behaviour
}
kmp_gmode_t;

//-----------------------------------------------------------------------------
// [[kmp_engine_t]]

typedef enum kmp_engine_t
{
    ENGINE_BATTLE,
    ENGINE_50,
    ENGINE_100,
    ENGINE_150,
    ENGINE_200,
    ENGINE_150M,
    ENGINE_200M,
    ENGINE__N,

    ENGM_BATTLE	= 1 << ENGINE_BATTLE,
    ENGM_50	= 1 << ENGINE_50,
    ENGM_100	= 1 << ENGINE_100,
    ENGM_150	= 1 << ENGINE_150,
    ENGM_200	= 1 << ENGINE_200,
    ENGM_150M	= 1 << ENGINE_150M,
    ENGM_200M	= 1 << ENGINE_200M,

    ENGM__ALL	= ( 1 << ENGINE__N ) - 1,
}
kmp_engine_t;

//-----------------------------------------------------------------------------
// [[lex_oomode_t]] settings for LEX/TEST @OFFLINE_ONLINE

typedef enum lex_oomode_t
{
    LEX_OO_AUTO,	// automatic select (default)
    LEX_OO_OFFLINE,	// force offline
    LEX_OO_ONLINE,	// force online
}
lex_oomode_t;

//-----------------------------------------------------------------------------
// [[lex_gmode_t]] settings for LEX/TEST @GAME_MODE

typedef enum lex_gmode_t
{
    LEX_GMODE_AUTO,	// automatic select (default)
    LEX_GMODE_STD,	// standard node (no LE-CODE executor)
    LEX_GMODE__BASE,	// base index for KMP_GMODE_* values

    // force settings ...
    LEX_GMODE_BALLOON	= LEX_GMODE__BASE + KMP_GMODE_BALLOON,
    LEX_GMODE_COIN	= LEX_GMODE__BASE + KMP_GMODE_COIN,
    LEX_GMODE_VERSUS	= LEX_GMODE__BASE + KMP_GMODE_VERSUS,
    LEX_GMODE_ITEMRAIN	= LEX_GMODE__BASE + KMP_GMODE_ITEMRAIN,

    LEX_GMODE__N
}
lex_gmode_t;

//-----------------------------------------------------------------------------
// [[lex_engine_t]] settings for LEX/TEST @ENGINE

typedef enum lex_engine_t
{
    LEX_ENGINE_AUTO,	// automatic select (default)
    LEX_ENGINE__BASE,	// base index for KMP_ENGINE_* values

    // force settings ...
    LEX_ENGINE_BATTLE	= LEX_ENGINE__BASE + ENGINE_BATTLE,
    LEX_ENGINE_50	= LEX_ENGINE__BASE + ENGINE_50,
    LEX_ENGINE_100	= LEX_ENGINE__BASE + ENGINE_100,
    LEX_ENGINE_150	= LEX_ENGINE__BASE + ENGINE_150,
    LEX_ENGINE_200	= LEX_ENGINE__BASE + ENGINE_200,
    LEX_ENGINE_150M	= LEX_ENGINE__BASE + ENGINE_150M,
    LEX_ENGINE_200M	= LEX_ENGINE__BASE + ENGINE_200M,
}
lex_engine_t;

//-----------------------------------------------------------------------------
// [[chat_vehicle_t]] bits for vehicle settings for CHAT-MODE

typedef enum chat_vehicle_t
{
    CHATVEH_SMALL	= 0x01,
    CHATVEH_MEDIUM	= 0x02,
    CHATVEH_LARGE	= 0x04,
     CHATVEH_ANY_SIZE	= CHATVEH_SMALL | CHATVEH_MEDIUM | CHATVEH_LARGE,

    CHATVEH_KART	= 0x08,
    CHATVEH_OUT_BIKE	= 0x10,
    CHATVEH_IN_BIKE	= 0x20,
     CHATVEH_BIKE	= CHATVEH_OUT_BIKE | CHATVEH_IN_BIKE,
     CHATVEH_ANY_TYPE	= CHATVEH_KART | CHATVEH_BIKE,

    CHATVEH_ANY		= CHATVEH_ANY_SIZE | CHATVEH_ANY_TYPE,
}
chat_vehicle_t;

//-----------------------------------------------------------------------------
// [[chat_mode_t]] settings for CHAT-MODE

typedef enum chat_mode_t
{
    //-- 0x0000 - 0x01ff : simple values

    CHATMD_OFF,

    CHATMD_ANY_TRACK,
    CHATMD_TRACK_BY_HOST,

    CHATMD_ANY_VEHICLE,
    CHATMD_KARTS_ONLY,
    CHATMD_BIKES_ONLY,

    CHATMD_RESET_ENGINE,
    CHATMD_USE_ENGINE_1,
    CHATMD_USE_ENGINE_2,
    CHATMD_USE_ENGINE_3,

    CHATMD_RESET,

    CHATMD_BLOCK_CLEAR,
    CHATMD_BLOCK_DISABLE,
    CHATMD_BLOCK_ENABLE,

    CHATMD__N,


    //-- 0x0200 - 0x03ff (-0x05ff): special range for n_races

    CHATMD_MAX_RACE	= 512,
    CHATMD_N_RACE_1	= 0x0200,
    CHATMD_N_RACE_MAX	= CHATMD_N_RACE_1 + CHATMD_MAX_RACE-1,


    //-- 0x0600 - 0x063f (-0x6ff): special range for vehicles

    CHATMD_VEHICLE_BEG	= 0x0600,
    CHATMD_VEHICLE_END	= CHATMD_VEHICLE_BEG + CHATVEH_ANY,
}
__attribute__ ((packed)) chat_mode_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan options			///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptAlignU8	( ccp arg );
int ScanOptAlignPACK	( ccp arg );
int ScanOptAlignBRRES	( ccp arg );
int ScanOptAlignBREFF	( ccp arg );
int ScanOptAlignBREFT	( ccp arg );
int ScanOptAlign	( ccp arg );
int ScanOptMaxFileSize	( ccp arg );
int ScanOptEpsilon	( ccp arg );
int ScanOptNMipmaps	( ccp arg );
int ScanOptMaxMipmaps	( ccp arg );
int ScanOptMipmapSize	( ccp arg );
int ScanOptPtDir	( ccp arg );
int ScanOptRecurse	( ccp arg );
int ScanOptCase		( ccp arg );
int ScanOptFModes	( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  scan num/range		///////////////
///////////////////////////////////////////////////////////////////////////////

double strtod_comma ( const char *nptr, char **endptr );

///////////////////////////////////////////////////////////////////////////////

char * ScanNumU32
(
    ccp			arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..1)
    u32			* p_num,	// return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
);

char * ScanRangeU32
(
    ccp			arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..2)
    u32			* p_n1,		// first return value (valid pointer)
    u32			* p_n2,		// second return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
);

unsigned long int strtoul16
(
    const u16		*  ptr16,
    u16			** end16,
    int			base
);

u16 * skip_ctrl16 ( const u16 * ptr );

u16 * ScanNum16U32
(
    const u16		* arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..1)
    u32			* p_num,	// return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
);

u16 * ScanRange16U32
(
    const u16		* arg,		// text to scan (valid pointer)
    u32			* p_stat,	// NULL or return status:
					//   number of scanned numbers (0..2)
    u32			* p_n1,		// first return value (valid pointer)
    u32			* p_n2,		// second return value (valid pointer)
    u32			min,		// min allowed value (fix num)
    u32			max		// max allowed value (fix num)
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			vector calculations		///////////////
///////////////////////////////////////////////////////////////////////////////

double LengthF
(
    const float *a,		// A vector (x,y,z)
    const float *b		// B vector (x,y,z)
);

double Length2F
(
    const float *a,		// A vector (x,y,z)
    const float *b		// B vector (x,y,z)
);

double LengthD
(
    const double *a,		// A vector (x,y,z)
    const double *b		// B vector (x,y,z)
);

double Length2D
(
    const double *a,		// A vector (x,y,z)
    const double *b		// B vector (x,y,z)
);

double AngleVector
(
    const float	* prev,		// vector (x,y,z) of previous point
    const float	* point,	// vector (x,y,z) of current point
    const float	* next,		// vector (x,y,z) of next point
    bool	return_degree	// true: return in normed degree; false: radiants
);

void WidthAndDirection
(
    double	* r_width,	// if not NULL: return width here
    double	* r_direction,	// if not NULL: return direction here
    bool	return_degree,	// true: return in normed degree; false: radiants
    const float	* a,		// A point (x,y)
    const float	* b		// B point (x,y)
);

double CalcDirection2F
(
    const float	* a,		// A point (x,y)
    const float	* b		// B point (x,y)
);

float3 CalcDirection3F
(
    const float * a,		// A point (x,y,z)
    const float * b		// B point (x,y,z)
);

double3 CalcDirection3D
(
    const double3 * a,		// A point (x,y,z)
    const double3 * b		// B point (x,y,z)
);

void Scale
(
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*scale,		// NULL or scaling vector
    double3		*pt,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
);

void Rotate
(
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*rot,		// rotation vector
    double3		*pt,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
);

void Translate
(
    const double3	*trans,		// NULL or translation vector
    double3		*pt,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
);

void AxisRotate
(
    const double3	*a1,		// first axis point
    const double3	*a2,		// second axis point
    double		degree,		// degree of rotation
    double3		*pt,		// pointer to first point to rotate
    uint		pt_delta,	// delta in bytes to next point
    uint		n_pt		// number of points to rotate
);

double Degree // output: -180.00 .. +180.00
(
    double	rad		// radians
);

float NormDegreeF // output: -180.00 .. +180.00
(
    float	deg
);

void MinMax3
(
    double3		*min,		// valid pointer to initialized min value
    double3		*max,		// valid pointer to initialized max value
    const double3	*val,		// valid pointer to source list
    int			n_val		// if >0: number of elements in 'val'
);

void UnitZero3 ( double3 *dest, const double3 *src );

double CalcNormals
(
    // return length of p1..p2

    double3		*res_norm,	// return 3 normals here
    const double3	*p1,		// first point
    const double3	*p2,		// second point
    const double3	*dir,		// NULL or helper point for normal[1]
    double		r		// radius == length of norm[0]..norm[2]
);

bool OverlapCubeTri
(
    double3		*cube_mid,	// middle of the cube
    double		cube_width,	// width of the cube
    double3		*pt1,		// first point of triangle
    double3		*pt2,		// second point of triangle
    double3		*pt3		// third point of triangle
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  triangles			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[tri_metrics_t]]

typedef struct tri_metrics_t
{
    double3 p1, p2, p3;		// base points
    double  l1, l2, l3;		// side lengths
    double  area;		// areas of triangle
    double  h1, h2, h3;		// heights
    double  min_ht;		// minimal height
    double  max_ht;		// maximal height
}
tri_metrics_t;

//-----------------------------------------------------------------------------

void CalcTriMetricsByPoints
(
    tri_metrics_t	*met,	// result
    const double3	*p1,	// first point
    const double3	*p2,	// second point
    const double3	*p3	// third point
);

void CalcTriMetrics ( tri_metrics_t *met );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  octahedron_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[octahedron_t]]

typedef struct octahedron_t
{
    union
    {
	double3 p[6];
	struct
	{
	    double3 x1;	 // left point
	    double3 x2;	 // right point
	    double3 y1;  // bottom point
	    double3 y2;  // top point
	    double3 z1;  // far point
	    double3 z2;  // near point
	};
    };

} octahedron_t;

//-----------------------------------------------------------------------------

void CreateRegularOctahedron
(
    octahedron_t	*oct,		// valid pointer, initialize
    double		radius,		// radius of octahedron
    const double3	*rot,		// not NULL: rotate octahedron
    const double3	*shift		// not NULL: shift octahedron
);

//-----------------------------------------------------------------------------

void CreateRegularOctahedronF
(
    octahedron_t	*oct,		// valid pointer, initialize
    double		radius,		// radius of octahedron
    const float		*rot,		// not NULL: 3D vector, rotate octahedron
    const float		*shift		// not NULL: 3D vector, shift octahedron
);

//-----------------------------------------------------------------------------

void CreateOctahedron
(
    octahedron_t	*oct,		// valid pointer, initialize
    const double3	*radius,	// radius of octahedron
    const double3	*rot,		// not NULL: rotate octahedron
    const double3	*shift		// not NULL: shift octahedron
);

//-----------------------------------------------------------------------------

void ScaleOctahedron
(
    octahedron_t	*oct,		// valid octahedron data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*scale		// not NULL: scale octahedron
);

//-----------------------------------------------------------------------------

void RotateOctahedron
(
    octahedron_t	*oct,		// valid octahedron data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*rot		// not NULL: rotate octahedron
);

//-----------------------------------------------------------------------------

void ShiftOctahedron
(
    octahedron_t	*oct,		// valid octahedron data
    const double3	*shift		// not NULL: shift octahedron
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  cuboid_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[cuboid_t]]

typedef struct cuboid_t
{
    union
    {
	double3 p[8];
	struct
	{
	    double3 p000;
	    double3 p001;
	    double3 p010;
	    double3 p011;
	    double3 p100;
	    double3 p101;
	    double3 p110;
	    double3 p111;
	};
    };

} cuboid_t;

//-----------------------------------------------------------------------------

void CreateCube
(
    cuboid_t		*cub,		// valid pointer, initialize
    double		width,		// width of cube
    const double3	*rot,		// not NULL: rotate cube
    const double3	*shift		// not NULL: shift cube
);

//-----------------------------------------------------------------------------

void CreateCubeF
(
    cuboid_t		*cub,		// valid pointer, initialize
    double		width,		// width of cube
    const float		*rot,		// not NULL: 3D vector, rotate cube
    const float		*shift		// not NULL: 3D vector, shift cube
);

//-----------------------------------------------------------------------------

void CreateCuboid
(
    cuboid_t		*cub,		// valid pointer, initialize
    const double3	*size,		// base size of cuboid
    const double3	*rot,		// not NULL: rotate cuboid
    const double3	*shift		// not NULL: shift cuboid
);

//-----------------------------------------------------------------------------

void CreateCuboidF
(
    cuboid_t		*cub,		// valid pointer, initialize
    const float		*size,		// base size of cuboid
    const float		*rot,		// not NULL: rotate cuboid
    const float		*shift		// not NULL: shift cuboid
);

//-----------------------------------------------------------------------------

void ScaleCuboid
(
    cuboid_t		*cub,		// valid cuboid data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*scale		// not NULL: scale cuboid
);

//-----------------------------------------------------------------------------

void RotateCuboid
(
    cuboid_t		*cub,		// valid cuboid data
    const double3	*origin,	// origin of rotation, NULL = v(0,0,0)
    const double3	*rot		// not NULL: rotate cuboid
);

//-----------------------------------------------------------------------------

void ShiftCuboid
(
    cuboid_t		*cub,		// valid cuboid data
    const double3	*shift		// not NULL: shift cuboid
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			index lists			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[IndexList_t]]

typedef struct IndexList_t
{
    ccp		* list;		// list of names
    uint	used;		// used elements of 'list'
    uint	size;		// alloced elements of 'list'

    ccp		prefix;		// prefix for automatic named variables
    uint	next_index;	// next index for autmatic named variables

} IndexList_t;

//-----------------------------------------------------------------------------

void InitializeIL ( IndexList_t * il, ccp prefix );
void ResetIL ( IndexList_t * il );

void FillIL
(
    IndexList_t		* il,		// valid index list
    uint		min_fill	// fill minimum # elements
);

void DefineIL
(
    IndexList_t		* il,		// valid index list
    uint		index,		// index of object
    ccp			name,		// name of object
    bool		move_name	// true: 'name' is alloced -> move it
);

bool RemoveIL
(
    IndexList_t		* il,		// valid index list
    uint		index		// index of object
);

ccp GetNameIL
(
    IndexList_t		* il,		// valid index list
    uint		index		// index of object
);

void PrintNameIL
(
    FILE		* f,		// output files
    IndexList_t		* il,		// valid index list
    uint		index,		// index of object
    uint		align,		// align end of name to character pos
    uint		fw,		// field width of name
    uint		indent		// indention of continuation line
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			parser functions		///////////////
///////////////////////////////////////////////////////////////////////////////

#define MAX_FUNC_PARAM 1000

typedef enumError (*call_func_t)
(
    struct Var_t	* res,		// store result here
    struct Var_t	* param,	// parameters, modified
    uint		n_param,	// number of parameters
    struct ScanInfo_t	* si,		// not NULL: use for error messages
    const struct FuncParam_t
			* fpar		// pointer to user parameter
);

//-----------------------------------------------------------------------------
// [[FuncParam_t]]

typedef struct FuncParam_t
{
    bool		need_name;	// true: needs a name as first param
    s16			min_param;	// minimum parameter
    s16			max_param;	// maximum parameter
    u16			fform_id;	// function class, based on file_format_t
    union
    {
	s32		user_id;	// user defined id
	struct ScanMacro_t
			* macro;	// NULL or related macro to call
    };
    call_func_t		func;		// function to call

} FuncParam_t;

extern FuncParam_t FuncDollar;

//-----------------------------------------------------------------------------
// [[FuncTable_t]]

typedef struct FuncTable_t
{
    //--- function def
    s32			min_param;	// Minimum number of parameters,
					//	<0: first param is a var name
    s32			max_param;	// Maximum number of parameters
    ccp			name;		// Name of function / If NULL: info only
    call_func_t		func;		// Function to call / If NULL: end of list
    int			user_id;	// User defined id

    //--- function info

    ccp			result;		// Function result info
    ccp			syntax;		// Syntax of function // If NULL: no info
    ccp			info;		// Description

} FuncTable_t;

//-----------------------------------------------------------------------------

void DefineDefaultParserFunc();

void DefineParserFuncTab
(
    const FuncTable_t	*ftab,		// pointer to a definittion table
					// terminated with member 'func == 0'
    u16			fform_id	// function class, based on file_format_t
);

FuncParam_t * DefineParserFunc
(
    ccp			name,		// name of function
    s16			min_param,	// minimum parameter
    s16			max_param,	// maximum parameter
    u16			fform_id,	// function class, based on file_format_t
    call_func_t		func,		// function to call
    int			user_id		// user defined id
);

const FuncParam_t * GetParserFunc
(
    ccp			name,		// name to search
    struct ScanInfo_t	*si,		// not NULL: print warning if not found
    FuncParam_t		*temp_par	// not NULL: search user defined functions,
					// and use this to create the function result
);

///////////////////////////////////////////////////////////////////////////////
// [[FuncInfo_t]]

typedef struct FuncInfo_t
{
    ccp			result;		// function result info
    ccp			info;		// description

} FuncInfo_t;

struct search_filter_t;

//-----------------------------------------------------------------------------

FuncInfo_t * DefineParserFuncInfo
(
    ccp			result,		// function result info
    ccp			syntax,		// syntax of function
    ccp			info		// description
);

void DumpParserFuncInfo
(
    FILE		* f,		// destination file
    uint		indent,		// indention
    bool		print_header,	// true: print table header
    int			long_count,	// >0: print description too
    struct search_filter_t * filter	// NULL or filter data
);

//-----------------------------------------------------------------------------

struct search_filter_t * SetupParserFuncFilter();
void ParserFuncFilter ( struct search_filter_t *filter_data, ccp arg, int mode );

void ExportTrackInfo ( FILE * f );
void ExportParserFuncMakedoc ( FILE * f, ccp index );
enumError ExportHelper ( ccp func_mode );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			variables & maps		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[VarMode_t]] [[VarModeHelpers_t]]

#define VARNAME_SIZE	100
#define STACK_SIZE	100

typedef enum VarMode_t
{
    VAR_UNSET,
    VAR_INT,
    VAR_DOUBLE,
    VAR_VECTOR,
    VAR_STRING,
}
__attribute__ ((packed)) VarMode_t;

typedef enum VarModeHelpers_t
{
    VAR_X = VAR_VECTOR+1,
    VAR_Y,
    VAR_Z
}
__attribute__ ((packed)) VarModeHelpers_t;

//-----------------------------------------------------------------------------

static inline bool isScalarVM ( VarMode_t mode )
	{ return mode <= VAR_DOUBLE; }

static inline bool isNumericVM ( VarMode_t mode )
	{ return mode >= VAR_INT && mode <= VAR_VECTOR; }

///////////////////////////////////////////////////////////////////////////////
// [[Var_t]]

typedef struct Var_t
{
    ccp		name;		// name of variable, alloced
    VarMode_t	mode;		// mode of variable
    IntMode_t	int_mode;	// integer mode for special purpose

    union
    {
	s64	i;		// int64 value
	double	d;		// double value
	double  v[3];		// double vector
	struct
	{
	    double x;		// double components
	    double y;
	    double z;
	};
	double3 d3;		// double3 value for references
	struct
	{
	    uint str_len;	// used length of 'str' (support of NULL bytes)
	    uint str_size;	// alloced size of 'str', NULL term is not counting!
	    char *str;		// NULL or string (alloced)
	};

	// for internal usage
	FuncParam_t func_param;
	FuncInfo_t  func_info;
	struct ScanMacro_t * macro;
    };

} Var_t;

extern Var_t var_unset;

//-----------------------------------------------------------------------------

static void inline InitializeV ( Var_t * var )
{
    DASSERT(var);
    memset(var,0,sizeof(*var));
}

#define DEFINE_VAR(v) Var_t v = {0};

//-----------------------------------------------------------------------------

static void inline ResetV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode == VAR_STRING )
	FREE(var->str);
    var->mode = VAR_UNSET;
    var->int_mode = IMD_UNSET;
}

//-----------------------------------------------------------------------------

static void inline FreeV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode == VAR_STRING )
    {
	FREE(var->str);
	var->mode = VAR_UNSET;
	var->int_mode = IMD_UNSET;
    }
}

//-----------------------------------------------------------------------------

static void inline MarkFreeV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode == VAR_STRING )
	var->mode = VAR_UNSET;
}

//-----------------------------------------------------------------------------

// move data only (exclude name); src can be NULL or ==dest
void MoveDataV ( Var_t *dest, Var_t * src );
void MoveDataInitV ( Var_t *dest, Var_t * src );

// move complete including name); src can be NULL or ==dest
void MoveAllV ( Var_t *dest, Var_t * src );
void MoveAllInitV ( Var_t *dest, Var_t * src );

///////////////////////////////////////////////////////////////////////////////
// [[VarMap_t]]

typedef struct VarMap_t
{
    Var_t		*list;		// pointer to the item list
    uint		used;		// number of used elements of 'list'
    uint		size;		// number of allocated  elements of 'list'
    LowerUpper_t	force_case;	// change case if LOUP_UPPER | LOUP_LOWER

} VarMap_t;

//-----------------------------------------------------------------------------

void DefineParserVars ( VarMap_t * vm );
void DefineMkwVars ( VarMap_t * vm );
const VarMap_t * SetupParserVars();

enumError DumpSymbols ( const VarMap_t * vm );
enumError ListParserFunctions();
enumError ParserCalc ( const VarMap_t * predef );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan info			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[TextCommand_t]]

typedef enum TextCommand_t
{
    TCMD_NONE		= 0,	// definitiv NULL

    TCMD_ECHO,
    TCMD_WARN,

    TCMD_RESET_PRIVATE,
    TCMD_RESET_LOCAL,
    TCMD_RESET_GLOBAL,

    TCMD_DUMP_VAR,
    TCMD_DUMP_PRIVATE,
    TCMD_DUMP_LOCAL,
    TCMD_DUMP_GLOBAL,
    TCMD_DUMP_CONST,
    TCMD_DUMP_PREDEF,

    TCMD_REVISION,
    TCMD_SZS_MODIFIER,
    TCMD_SETUP_RANDOM,

    TCMD_PDEF,		//+++ 'TCMD_LDEF* - TCMD_PDEF*' is constant for all '*'
    TCMD_PDEF_I,	//
    TCMD_PDEF_F,	//
    TCMD_PDEF_S,	//
    TCMD_PDEF_V,	//
    TCMD_PDEF_X,	//	//+++ _X _Y and _Z are well sorted
    TCMD_PDEF_Y,	//	//
    TCMD_PDEF_Z,	//	//---
    TCMD_PDEF_ENUM,	//
    TCMD_PDEF_SHIFT,	//---

    TCMD_LDEF,		//+++ 'TCMD_PDEF* - TCMD_LDEF*' is constant for all '*'
    TCMD_LDEF_I,	//
    TCMD_LDEF_F,	//
    TCMD_LDEF_S,	//
    TCMD_LDEF_V,	//
    TCMD_LDEF_X,	//	//+++ _X _Y and _Z are well sorted
    TCMD_LDEF_Y,	//	//
    TCMD_LDEF_Z,	//	//---
    TCMD_LDEF_ENUM,	//
    TCMD_LDEF_SHIFT,	//---

    TCMD_GDEF,		//+++ 'TCMD_GDEF* - TCMD_LDEF*' is constant for all '*'
    TCMD_GDEF_I,	//
    TCMD_GDEF_F,	//
    TCMD_GDEF_S,	//
    TCMD_GDEF_V,	//
    TCMD_GDEF_X,	//	//+++ _X _Y and _Z are well sorted
    TCMD_GDEF_Y,	//	//
    TCMD_GDEF_Z,	//	//---
    TCMD_GDEF_ENUM,	//
    TCMD_GDEF_SHIFT,	//---

    TCMD_IF,		//+++ first IF related command
    TCMD_ELIF,		//
    TCMD_ELSE,		//
    TCMD_ENDIF,		//--- last IF related command

    TCMD_DOIF,

    TCMD_LOOP,		//+++ LOOP commands, same order as TCMD_END* & loop_name[]
    TCMD_REPEAT,	//
    TCMD_FOR,		//
    TCMD_FOREACH,	//
    TCMD_WHILE,		//---

    TCMD_ENDLOOP,	//+++ END-LOOP commands, same order as TCMD_LOOP...
    TCMD_ENDREPEAT,	//    & loop_name[]
    TCMD_ENDFOR,	//
    TCMD_ENDFOREACH,	//
    TCMD_ENDWHILE,	//---

    TCMD_BREAK,		// terminate a loop
    TCMD_CONTINUE,	// continue a loop

    TCMD_EXEC,
    TCMD_INCLUDE,
    TCMD_RETURN,
    TCMD_EXIT,
    TCMD_ASSERT,

    TCMD_MACRO,		//+++ same order as TCMD_END*
    TCMD_FUNCTION,	//---
    TCMD_ENDMACRO,	//+++ same order as TCMD_* without END
    TCMD_ENDFUNCTION,	//---

    TCMD_CALL,
    TCMD_PARAMETERS,

    TCMD__N

} TextCommand_t;

//-----------------------------------------------------------------------------
// [[ScanLoop_t]]

typedef struct ScanLoop_t
{
    int			tcmd_end;	// closing TCMD_*
    ccp			ptr;		// file position of loop start
    uint		line;		// line number on loop begin
    int			if_level;	// number of active IF..ENDIF on loop begin

    int			count_val;	// internal loop counter 0..
    int			end_val;	// end value for @REPEAT + @FOR
    int			step_val;	// step_val for @FOR
    ccp			varname;	// variable name for @FOR + @FOREACH, alloced
    ccp			expr_ptr;	// expression pointer
    uint		expr_line;	// expression line

} ScanLoop_t;

//-----------------------------------------------------------------------------
// [[ScanMacro_t]]

typedef struct ScanMacro_t
{
    ccp			data;		// begin of text
    uint		data_size;	// size of data
    bool		data_alloced;	// true if 'data' alloced
    uint		line0;		// line number of first line
    ccp			src_name;	// name of source file, alloced
    bool		is_function;	// true: defined as function
    Var_t		* map;		// pointer to map entry

} ScanMacro_t;

//-----------------------------------------------------------------------------
// [[ScanFile_t]]

typedef struct ScanFile_t // includes and macros
{
    //--- scanning infos

    struct ScanInfo_t	* si;		// base scan info
    ScanMacro_t		* sm;		// not NULL: pointer to macro

    ccp			data;		// begin of text
    bool		data_alloced;	// true if 'data' alloced
    ccp			ptr;		// current text position
    ccp			end;		// end of text
    ccp			prev_ptr;	// 'ptr' before scanning an object
    uint		line;		// current line number
    uint		line_err;	// number of line errors
    ccp			name;		// name or file for warnings
    bool		name_alloced;	// true if 'name' alloced
    int			if_level;	// number of active IF..ENDIF
    struct ScanFile_t	* next;		// next file for RETURN

    //--- loop control

    int			loop_level;	// number of active loops
    ScanLoop_t		loop[MAX_LOOP_DEPTH];	// loop data

    //--- some options

    int			revision;	// current revision number
    int			szs_modifier;	// >0: szs_modifier mode enabled
    int			disable_comma;	// >0: disable comma in floats
    int			expr_depth;	// level of expression depth

    //--- variables

    VarMap_t		pvar;		// private variables
    const ScanMacro_t	* active_macro;	// active macro or function

    //--- parameter stack

    Var_t		param[STACK_SIZE]; // parameter stack
    int			param_used;	// number of used parameters

} ScanFile_t;

extern ScanFile_t empty_scan_file;

//-----------------------------------------------------------------------------
// [[ScanInfo_t]]

typedef struct ScanInfo_t
{
    //--- init data

    ccp			init_data;	// begin of text
    uint		init_data_size;	// total size of text
    ccp			init_name;	// name or file for warnings
    int			init_revision;	// revision number at start

    //--- current file

    uint		n_files;	// number of open files and macros
    ScanFile_t		* cur_file;	// pointer to current file, never NULL
    VarMap_t		macro;		// macro map

    //--- scanning infos

    uint		total_err;	// total number of errors
    int			no_warn;	// >0: suppress warnings
    int			no_calc;	// >0: suppress calculations and function calls
    int			fast_scan;	// >0: ignore @commands
    int			force_hex;	// >0: force hex for non prefixed integers
    int			point_is_null;	// >0: a single point is read as Null
    int			float_div;	// >0: floating point division
 #if HAVE_PRINT
    int			debug;		// >0: print debugging infos
 #endif

    //--- variables

    VarMap_t		* param;	// NULL or parameters, not cleared
    VarMap_t		lvar;		// local variables
    VarMap_t		gvar;		// global variables
    const VarMap_t	* predef;	// NULL or predefined variables
    Var_t		last_result;	// last macro/function result
    int			func_status;	// second status of some parser functions

    //--- user info

    struct kcl_t	* kcl;		// for KCL scanning
    struct kmp_t	* kmp;		// for KMP scanning
    struct lex_t	* lex;		// for LEX scanning
    struct mdl_t	* mdl;		// for MDL scanning
    struct le_distrib_t	* ld;		// for LEDEF scanning

} ScanInfo_t;

//-----------------------------------------------------------------------------

typedef enum ScanParamMode
{
    SPM_NONE,		// ignore result
    SPM_INC,		// result has type int: increment value

    SPM_BOOL,		// result has type bool

    SPM_U8,		// result has type u8
    SPM_S8,		// result has type s8
    SPM_U16,		// result has type u16
    SPM_S16,		// result has type s16
    SPM_U32,		// result has type u32
    SPM_S32,		// result has type s32

    SPM_U16_BE,		// result has type u16, stored as big endian
    SPM_S16_BE,		// result has type s16, stored as big endian
    SPM_U32_BE,		// result has type u32, stored as big endian
    SPM_S32_BE,		// result has type s32, stored as big endian

    SPM_INT,		// result has type int
    SPM_UINT,		// result has type uint
    SPM_FLOAT,		// result has type float
    SPM_FLOAT_BE,	// result has type float and is stored as big endian
    SPM_DOUBLE,		// result has type double

    SPM_DOUBLE_X,	// result has type double, use X on vector
    SPM_DOUBLE_Y,	// result has type double, use Y on vector
    SPM_DOUBLE_Z,	// result has type double, use Z on vector

    SPM_FLOAT3,		// result has type float3
    SPM_FLOAT3_BE,	// result has type float3 and is stored as big endian

    SPM_STRING,		// result has type char*
    SPM_VAR,		// result has type Var_t

}  ScanParamMode;

//-----------------------------------------------------------------------------
// [[ScanParam_t]]

typedef struct ScanParam_t
{
    ccp			name;		// name of the parameter
    ScanParamMode	mode;		// type of result
    void		*result;	// pointer to result
    uint		min;		// >1: scan at least 'min' values
    uint		max;		// >min: scan maxixmal 'max' values
    int			*n_result;	// not NULL: store number of read values

} ScanParam_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			conversion helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

bool	GetBoolV    ( const Var_t * var );
int	GetIntV	    ( const Var_t * var );
double	GetDoubleV  ( const Var_t * var );
double	GetXDoubleV ( const Var_t * var );
double	GetYDoubleV ( const Var_t * var );
double	GetZDoubleV ( const Var_t * var );
double3	GetVectorV  ( const Var_t * var );
float3	GetVectorFV ( const Var_t * var );
Var_t * GetStringV  ( const Var_t * var, Var_t * temp ); // 'var' can be NULL

Var_t * ToNoneV    ( Var_t * var );
Var_t * ToBoolV    ( Var_t * var );
Var_t * ToIntV     ( Var_t * var );
Var_t * ToDoubleV  ( Var_t * var );
Var_t * ToXDoubleV ( Var_t * var );
Var_t * ToYDoubleV ( Var_t * var );
Var_t * ToZDoubleV ( Var_t * var );
Var_t * ToVectorV  ( Var_t * var );
Var_t * ToVectorXV ( Var_t * var );
Var_t * ToVectorYV ( Var_t * var );
Var_t * ToVectorZV ( Var_t * var );
Var_t * ToStringV  ( Var_t * var );
Var_t * ToVarMode  ( Var_t * var, TextCommand_t mode );

void AssignIntV     ( Var_t * var, int val );
void AssignDoubleV  ( Var_t * var, double val );
void AssignXDoubleV ( Var_t * var, double val );
void AssignYDoubleV ( Var_t * var, double val );
void AssignZDoubleV ( Var_t * var, double val );
void AssignVectorV  ( Var_t * var, const double3 * val );
void AssignVectorV3 ( Var_t * var, double x, double y, double z );
void AssignVar      ( Var_t * dest, const Var_t * src );
void AssignVarMode  ( Var_t * dest, const Var_t * src, TextCommand_t mode );

// return the length of the string after assigning
uint AssignStringVV  ( Var_t * var, const Var_t * src ); // 'src' can be NULL
uint AppendStringVV  ( Var_t * var, const Var_t * src ); // 'src' can be NULL
uint AssignStringVS  ( Var_t * var, ccp str, int len );
uint AssignStringVS2 ( Var_t * var, ccp str1, int len1, ccp str2, int len2 );
uint AppendStringVS  ( Var_t * var, ccp str, int len );
uint AppendStringVC  ( Var_t * var, char ch, int repeat );

// scan STRINGS, but copy 'src' on other modes
enumError ScanValueV ( Var_t * dest, const Var_t * src, ccp name );
enumError ScanExprV  ( Var_t * dest, const Var_t * src, ccp name );

//-----------------------------------------------------------------------------

static void inline SetModeStringV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_STRING )
    {
	var->str_len = var->str_size = 0;
	var->str  = 0;
	var->mode = VAR_STRING;
	var->int_mode = IMD_UNSET;
    }
}

//-----------------------------------------------------------------------------

enumError ResultFloat
(
    Var_t		* dest,		// valid destination var
    const float		* val,		// pointer to N floats
    uint		n_val		// number of floats in 'n'
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////		variables & maps: interface		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeVarMap
(
    VarMap_t		* vm		// data structure to initialize
);

//-----------------------------------------------------------------------------

void ResetVarMap
(
    VarMap_t		* vm		// data structure to reset
);

//-----------------------------------------------------------------------------

void ClearVarMap
(
    VarMap_t		* vm		// data structure to resewt
);

//-----------------------------------------------------------------------------

void CopyVarMap
(
    VarMap_t		* dest,		// destination var map
    bool		init_dest,	// true: initialialize 'dest' first
    const VarMap_t	* src		// source var map
);

//-----------------------------------------------------------------------------

void MoveVarMap
(
    VarMap_t		* dest,		// destination var map
    bool		init_dest,	// true: initialialize 'dest' first
    VarMap_t		* src		// source var map -> will be resetted
);

//-----------------------------------------------------------------------------

int FindVarMapIndex
(
    const VarMap_t	* vm,		// valid variable map
    ccp			varname,	// variable name
    int			not_found_value	// return value if not found
);

//-----------------------------------------------------------------------------

bool RemoveVarMap
(
    // returns 'true' if var deleted

    VarMap_t		* vm,		// valid variable map
    ccp			varname	// variable name
);

//-----------------------------------------------------------------------------

Var_t * InsertVarMap
(
    VarMap_t		* vm,		// valid variable map
    ccp			varname,	// varname to insert
    bool		move_varname,	// true: move 'varname', false: strdup()
    ScanInfo_t		* si,		// not null: print "already defined" message
    bool		* found		// not null: store found status
);

//-----------------------------------------------------------------------------

bool DefineIntVar
(
    VarMap_t		* vm,		// valid variable map
    ccp			varname,	// name of variable
    int			value		// value to assign
);

//-----------------------------------------------------------------------------

bool DefineDoubleVar
(
    VarMap_t		* vm,		// valid variable map
    ccp			varname,	// name of variable
    double		value		// value to assign
);

//-----------------------------------------------------------------------------

const Var_t * FindConst
(
    ccp			varname	// variable name
);

//-----------------------------------------------------------------------------

const Var_t * FindVarMap
(
    const VarMap_t	* vm,		// NULL or variable map
    ccp			varname,	// variable name
    ScanInfo_t		* si		// not null: print "not found" message
);

//-----------------------------------------------------------------------------

void PrintV
(
    FILE		* f,		// destination file
    const Var_t		* v,		// variable to print
    int			str_mode	// 0:-, 1:limit+escape, 2:limit+escape+quote
);

//-----------------------------------------------------------------------------

void DumpVarMap
(
    FILE		* f,		// destination file
    uint		indent,		// indention
    const VarMap_t	* vm,		// valid variable map
    bool		print_header	// print table header
);

//-----------------------------------------------------------------------------

extern VarMap_t const_map;

#define MINIMAP_DEFAULT_FLAGS 0x31f

extern u32   set_flags;
extern bool  have_set_scale;
extern bool  have_set_rot;
extern Var_t set_scale, set_rot;

extern uint	have_set_value; // x=bit-0, y=bit-1, z=bit-2
extern double3	set_value_min, set_value_max;

//-----------------------------------------------------------------------------
// transformation options, values and functions

#define N_TRANSFORM 10

extern uint	force_transform;		// bit 0: force tform even if not needed
						// bit 1: ignore it

extern MatrixD_t opt_transform;			// current transform values
extern MatrixD_t transform_list[N_TRANSFORM];	// list with stored tforms
extern uint	n_transform_list;		// number of elements in list
extern bool	transform_active;		// true: transformation active
extern int	disable_transformation;		// >0: transformation is temporary disabled

extern double	yposition;			// defined y position
extern bool	yposition_valid;		// 'yposition' is valid

extern bool	ascale_valid;			// true: --ascale defined
extern double	ascale;				// scaling factor
extern double3	ascale_dir;			// scaling direction (vector)

extern bool	arot_valid;			// true: --arot defined
extern double	arot_deg;			// angle of arot in degree
extern double3	arot1, arot2;			// points to define rotation axis

extern uint	mdl_switched_to_vertex;		// 0: default
						// 1: switch to 'VERTEX' needed
						// 2: MDL_MODE switched to 'VERTEX'
						// 3: '2' and message printed

//------------------------------

void ResetTransformation();			// reset all transformation steps
int  NextTransformation( bool for_close );	// enter next transformation step
void CloseTransformation();			// close transformation input
void DumpTransformationOpt();			// dump transformation options

double3 TransformD3( uint mode, double3 *val );	// transformate a singe vector
// mode:
//   0: use opt_transform and default
//   1: single steps through 'list', use base vectors
//   2: single steps through 'list', use norm vectors
//   3: single steps through 'list', use matrix

int ScanOptShift ( ccp arg );			// --shift
int ScanOptTranslate ( ccp arg );		// --translate
int ScanOptScale ( ccp arg );			// --scale
int ScanOptRotate ( ccp arg );			// --rot
int ScanOptXRotate ( uint xyz, ccp arg );	// --xrot --yrot --zrot
int ScanOptYPos ( ccp arg );			// --ypos
int ScanOptXSS	( uint xyz, ccp arg );		// --xss --yss --zss
int ScanOptAScale ( ccp arg );			// --ascale
int ScanOptARotate ( ccp arg );			// --arot

//-----------------------------------------------------------------------------

extern bool tform_script_enabled;

int ScanOptTformScript ( ccp arg );		// --tfor-script
bool ScanTformBegin();
bool TformScriptEnd();

enumError TformScriptCallF
(
    uint	dimension,	// '2' for a 2D and '3' for a 3D vector
    float3	*v,		// pointer to list of 3D vectors
    int		n,		// number of 3D vectors
    uint	off		// if n>1: offset from one to next vector
);

enumError TformScriptCallD
(
    uint	dimension,	// '2' for a 2D and '3' for a 3D vector
    double3	*v,		// pointer to list of 3D vectors
    int		n,		// number of 3D vectors
    uint	off		// if n>1: offset from one to next vector
);

//-----------------------------------------------------------------------------

void SetupConst();				// inititalize constants
int ScanOptConst ( ccp arg );			// --const
int ScanOptSetFlags ( ccp arg );		// --set-flags
int ScanOptSetScale ( ccp arg );		// --set-scale
int ScanOptSetRot ( ccp arg );			// --set-rot
int ScanOptSet ( uint xyz, ccp arg );		// --set

//-----------------------------------------------------------------------------

void TransformPosFloat2D
(
    float	*v,	// pointer to list of 2D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
);

static inline void TransformPosFloat3D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 3D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    TransformF3NMatrixD(&opt_transform,(float3*)v,n,off);
    if (tform_script_enabled)
	TformScriptCallF(3,(float3*)v,n,off);
}

void TransformPosDouble2D
(
    double	*v,	// pointer to list of 2D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
);

static inline void TransformPosDouble3D
(
    double	*v,	// pointer to list of 3D vectors
    int		n,	// number of 3D vectors
    uint	off	// if n>1: offset from one to next vector
)
{
    TransformD3NMatrixD(&opt_transform,(double3*)v,n,off);
    if (tform_script_enabled)
	TformScriptCallD(3,(double3*)v,n,off);
}

//-----------------------------------------------------------------------------

void TransformScaleFloat1D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
);

void TransformScaleFloat3D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
);

void TransformScaleDouble3D
(
    double	*v,	// pointer to list of 3D vectors
    int		n,	// number of 3D vectors
    uint	off	// if n>1: offset from one to next vector
);

//-----------------------------------------------------------------------------

void TransformRotFloat3D
(
    float	*v,	// pointer to list of 3D vectors
    int		n,	// number of 2D vectors
    uint	off	// if n>1: offset from one to next vector
);

void TransformRotDouble3D
(
    double	*v,	// pointer to list of 3D vectors
    int		n,	// number of 3D vectors
    uint	off	// if n>1: offset from one to next vector
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    scan text: Interface		///////////////
///////////////////////////////////////////////////////////////////////////////

ScanFile_t * AddSF
(
    ScanInfo_t		* si,		// base scan info
    ccp			data,		// data to scan
    uint		data_size,	// size of data
    ccp			name,		// NULL or name for warnings
    int			revision,	// revision number
    VarMap_t		* varmap	// NULL or new varmap (content moved)
);

//-----------------------------------------------------------------------------

bool DropSF
(
    ScanInfo_t		* si		// data structure
);

///////////////////////////////////////////////////////////////////////////////

void InitializeSI
(
    ScanInfo_t		* si,		// data structure to setup
    ccp			data,		// data to scan
    uint		data_size,	// size of data
    ccp			name,		// NULL or name for warnings
    int			revision	// revision number
);

//-----------------------------------------------------------------------------

void ResetSI
(
    ScanInfo_t		* si		// data structure to setup
);

//-----------------------------------------------------------------------------

void ResetLocalVarsSI
(
    ScanInfo_t		* si,		// data structure to setup
    int			revision	// revision number
);

//-----------------------------------------------------------------------------

void RestartSI
(
    ScanInfo_t		* si		// data structure to setup
);

//-----------------------------------------------------------------------------

char NextCharSI
(
    ScanInfo_t		* si,		// valid data
    bool		skip_lines	// true: skip empty lines
);

//-----------------------------------------------------------------------------

bool IsSectionOrEotSI
(
    // Call NextCharSI() and check for new section or EOT (return true if found)
    // Section name is stored in 'last_index_name' (upper case)

    ScanInfo_t		* si,		// valid data
    bool		skip_lines	// true: skip empty lines
);

//-----------------------------------------------------------------------------

char SkipCharSI
(
    // return next non skipped character

    ScanInfo_t		* si,		// valid data
    char		ch		// character to skip
);

//-----------------------------------------------------------------------------

char NextLineSI
(
    ScanInfo_t		* si,		// valid data
    bool		skip_cont,	// true: skip continuation lines
    bool		allow_tcmd	// true: scan and execute text commands
);

//-----------------------------------------------------------------------------

enumError CheckLevelSI
(
    ScanInfo_t		* si		// valid data
);

//-----------------------------------------------------------------------------

ccp GetEolSI
(
    ScanInfo_t		* si		// valid data
);

//-----------------------------------------------------------------------------

void GotoEolSI
(
    ScanInfo_t		* si		// valid data
);

//-----------------------------------------------------------------------------

enumError CheckEolSI
(
    ScanInfo_t		* si		// valid data
);

//-----------------------------------------------------------------------------

enumError CheckWarnSI
(
    ScanInfo_t		* si,		// valid data
    char		ch,		// char to test
    enumError		err		// previous error level
);

//-----------------------------------------------------------------------------

enumError WarnIgnoreSI
(
    ScanInfo_t		* si,		// valid data
    ccp			format,		// NULL or format of text before 'line ignored'
    ...					// arguments for 'format'
)
__attribute__ ((__format__(__printf__,2,3)));

//-----------------------------------------------------------------------------
// find first EOT|LF|CR|NUL

ccp FindEndOfLineSI
(
    ScanInfo_t		* si,		// valid data
    ccp			ptr		// start search here
);

//-----------------------------------------------------------------------------

ccp FindLineFeedSI
(
    const ScanInfo_t	* si,		// valid data
    ccp			ptr,		// start search here
    bool		mark_error	// true: increment error counter (ignore const)
);

//-----------------------------------------------------------------------------

ccp FindNextLineFeedSI
(
    const ScanInfo_t	* si,		// valid data
    bool		mark_error	// true: increment error counter (ignore const)
);

//-----------------------------------------------------------------------------

mem_t GetLineSI
(
    const ScanInfo_t	* si		// valid data
);

///////////////////////////////////////////////////////////////////////////////

uint ScanNameSI
(
    // return length of name

    ScanInfo_t		* si,		// valid data
    char		* name_buf,	// destination buffer
    uint		buf_size,	// size of 'name_buf'
    bool		allow_minus,	// true: allow '-' in names
    bool		to_upper,	// true: convert letters to upper case
    int			* xyz		// not null: store vector index
					// -1: no index, 0=x, 1=y, 2=z
);

//-----------------------------------------------------------------------------

enumError ConcatStringsSI
(
    ScanInfo_t		* si,		// valid data
    char		** dest_str,	// not NULL: store string here, use FREE + MALLOC
    uint		* ret_len	// not NULL: store length of return string here
);

//-----------------------------------------------------------------------------

enumError ScanStringSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// store result here (already initialized)
);

//-----------------------------------------------------------------------------

int ScanIndexSI
(
    // varname is stored in 'last_index_name' (upper case)

    ScanInfo_t		* si,		// valid data
    enumError		* err,		// return value: error code
    int			index,		// index to define if name
    IndexList_t		* il,		// NULL or index list
    int			insert_mode	// 0: don't insert
					// 1: insert into global namespace
					// 2: '1' + print "already defined" error
);

//-----------------------------------------------------------------------------

extern char last_index_name[VARNAME_SIZE+1];

static inline void SetIndexName ( ccp name )
	{ StringUpperS(last_index_name,sizeof(last_index_name),name); }

static inline void SetIndexNameM ( mem_t name )
	{ MemUpperS(last_index_name,sizeof(last_index_name),name); }

//-----------------------------------------------------------------------------

const Var_t * FindVarSI
(
    ScanInfo_t		* si,		// not NULL: use var maps
    ccp			varname,	// variable name
    bool		print_warning	// true && 'si': print 'not found' warning
);

//-----------------------------------------------------------------------------

Var_t * OverwriteVarSI
(
    // returns always not NULL

    ScanInfo_t		* si,		// not NULL: use var maps for search
    VarMap_t		* vm_insert,	// valid variable map, insert search failed
    ccp			varname,	// varname to insert
    int			* xyz,		// not NULL: enable .X/Y/Z support
					//   input: <0: not a X/Y/Z
					//        0..2: is a X/Y/Z
					//	    >2: unknown
					//   output: store corrected value
    bool		* found		// not null: store found status (false=NEW)
);

//-----------------------------------------------------------------------------

bool DefineIndexInt
(
    // returns true, if inserted

    ScanInfo_t		* si,		// valid data
    ccp			suffix,		// name suffix
    int			value		// value to assign
);

//-----------------------------------------------------------------------------

bool DefineIndexDouble
(
    // returns true, if inserted

    ScanInfo_t		* si,		// valid data
    ccp			suffix,		// name suffix
    double		value		// value to assign
);

//-----------------------------------------------------------------------------

enumError ScanExprSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// result: store value here
);

//-----------------------------------------------------------------------------

enumError ScanVectorExprSI
(
    // scans: v | a,b | x,y,z

    ScanInfo_t		* si,		// valid data
    Var_t		* var,		// result: store value here
    double		ab_default,	// used for 'a,b' format
    uint		xyz		// for ab default: 0:yz, 1:xz, 2:xy
);

enumError ScanVectorExpr
(
    // scans: v | x,z | x,y,z

    ccp			arg,		// string to scan
    ccp			name,		// name for error messages
    Var_t		* var		// result: store value here
);

//-----------------------------------------------------------------------------

enumError ScanMinMaxExprSI
(
    // scans: val (min=-val,max=+val) | val1,val2

    ScanInfo_t		*si,		// valid data
    Var_t		*min,		// result: store min vector here
    Var_t		*max		// result: store max vector here
);

//-----------------------------------------------------------------------------

enumError ScanValueSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// result: store value here
);

//-----------------------------------------------------------------------------

enumError ScanUValueSI
(
    ScanInfo_t		* si,		// valid data
    long		* num,		// return value
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanIntValueSI
(
    ScanInfo_t		* si,		// valid data
    int			* num		// return value
);

//-----------------------------------------------------------------------------

static inline enumError ScanUIntValueSI
(
    ScanInfo_t		* si,		// valid data
    uint		* num		// return value
)
{
    return ScanIntValueSI(si,(int*)num);
}

//-----------------------------------------------------------------------------

enumError ScanDValueSI
(
    ScanInfo_t		* si,		// valid data
    double		* num		// return value
);

//-----------------------------------------------------------------------------

enumError ScanBitfieldSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// result: store value here
);

//-----------------------------------------------------------------------------

enumError ScanU8SI
(
    ScanInfo_t		* si,		// valid data
    u8			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanU16SI
(
    ScanInfo_t		* si,		// valid data
    u16			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanU32SI
(
    ScanInfo_t		* si,		// valid data
    u32			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanBE16SI
(
    ScanInfo_t		* si,		// valid data
    u16			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanBE32SI
(
    ScanInfo_t		* si,		// valid data
    u32			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanBE32SI_swap
(
    ScanInfo_t		* si,		// valid data
    u32			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
);

//-----------------------------------------------------------------------------

enumError ScanFloatSI
(
    ScanInfo_t		* si,		// valid data
    float		* dest,		// destination list
    int			n		// number of elements in 'dest'
);

//-----------------------------------------------------------------------------

enumError ScanFloatV2SI
(
    ScanInfo_t		* si,		// valid data
    float		* dest,		// destination list
    int			n		// number of 2d vectors in 'dest'
);

//-----------------------------------------------------------------------------

enumError ScanFloatV3SI
(
    ScanInfo_t		* si,		// valid data
    float		* dest,		// destination list
    int			n		// number of 3d vectors in 'dest'
);

//-----------------------------------------------------------------------------

enumError ScanDoubleSI
(
    ScanInfo_t		* si,		// valid data
    double		* dest,		// destination list
    int			n		// number of elements in 'dest'
);

//-----------------------------------------------------------------------------

enumError ScanDoubleV3SI
(
    ScanInfo_t		* si,		// valid data
    double		* dest,		// destination list
    int			n		// number of elements in 'dest'
);

//-----------------------------------------------------------------------------

enumError ScanAssignIntSI
(
    ScanInfo_t		* si,		// valid data
    int			* value		// result: scanned value
);

//-----------------------------------------------------------------------------

enumError ScanAssignDoubleSI
(
    ScanInfo_t		* si,		// valid data
    double		* value		// result: scanned value
);

//-----------------------------------------------------------------------------

enumError ScanParamSI
(
    ScanInfo_t		* si,		// valid data
    const ScanParam_t	* sp		// NULL or scan parameter
					// list ends with sp->name==NULL
);

//-----------------------------------------------------------------------------

enumError ScanHexlineSI
(
    ScanInfo_t		* si,		// valid data
    FastBuf_t		* dest,		// valid, append scanned data here
    bool		skip_addr	// true: line may start with address field
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TextCommand_t GetTextCommand
(
    ccp			name		// name to scan, upper case is assumed
);

//-----------------------------------------------------------------------------

TextCommand_t ScanTextCommand
(
    ScanInfo_t		* si		// valid data
);

//-----------------------------------------------------------------------------

enumError ExecTextCommand
(
    ScanInfo_t		* si,		// valid data
    TextCommand_t	tcmd		// command to execute
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct Line_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[Line_t]]

#define MAX_LINE_HELPER 2
#define MAX_LINE_DIMEN  3

enum LineMode { LM_LINE, LM_BORDER, LM_BEZIER };

typedef struct Line_t
{
    uint	n;		// number of line points
    uint	n_pt;		// number of helper points: 0..2
    double	pt[MAX_LINE_HELPER][MAX_LINE_DIMEN]; // helper points
    int		line_mode;	// user defined mode

} Line_t;

//-----------------------------------------------------------------------------

void InitializeLine
(
    Line_t	* lin		// valid line data
);

enumError ScanLine
(
    Line_t	* lin,		// valid line data
    ScanInfo_t	* si,		// scanning source
    uint	n_dim,		// number of elements in each vector (<=3)
    int		mode		// user defined mode, stored in 'lin->mode'
);

void CalcLine
(
    Line_t	* lin,		// valid line data
    float	* dest,		// destination vector
    const float	* p1,		// source vector P1
    const float	* p2,		// source vector P2
    uint	n_dim,		// number of elements in each vector, n>3 possible
    uint	step		// value between 0 and lin->n inclcusive
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			matrix support			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_matrix();

bool PrintTestMatrix( bool print_sep );

bool TestMatrix
(
    // returns false on error

    double3	*val,		// NULL or value to test
    uint	print_mode,	// 0: silent
				// 1: print vectors on error
				// 2: print vectors always
    bool	print_matrix	// true && if vector shall print:
				//   print matrix before, but only the first time
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			operators			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[OpId_t]] created by work/scripts/create-db/optab.gen

typedef enum OpId_t
{
	OPI_NONE = 0,

	OPI_ADD,
	OPI_AND,
	OPI_DIV,
	OPI_EOR,
	OPI_EQ,
	OPI_EQEQ,
	OPI_GE,
	OPI_GT,
	OPI_JOIN,
	OPI_LAND,
	OPI_LE,
	OPI_LEOR,
	OPI_LOR,
	OPI_LSHIFT,
	OPI_LT,
	OPI_MOD,
	OPI_MULT,
	OPI_NEQ,
	OPI_NEQNEQ,
	OPI_OR,
	OPI_POW,
	OPI_RSHIFT,
	OPI_SUB,

	OPI__N // = 24

} OpId_t;

//-----------------------------------------------------------------------------

void CalcExpr
(
    Var_t		* dst,		// first source and destination
    const Var_t		* src,		// second source
    OpId_t		op_id,		// operator id
    ScanInfo_t		* si		// not NULL: Use for warnings
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			color conversions		///////////////
///////////////////////////////////////////////////////////////////////////////

extern const u8 cc38  [  8];	// convert 3-bit color to 8-bit color
extern const u8 cc48  [ 16];	// convert 4-bit color to 8-bit color
extern const u8 cc58  [ 32];	// convert 5-bit color to 8-bit color
extern const u8 cc68  [ 64];	// convert 6-bit color to 8-bit color

extern const u8 cc83  [256];	// convert 8-bit color to 3-bit color
extern const u8 cc84  [256];	// convert 8-bit color to 4-bit color
extern const u8 cc85  [256];	// convert 8-bit color to 5-bit color
extern const u8 cc85s1[256];	// convert 8-bit color to 5-bit color, shifted by 1 bit
extern const u8 cc86  [256];	// convert 8-bit color to 6-bit color

//-----------------------------------------------------------------------------

static inline u32 RGB565_to_RGB ( u16 rgb565 )
{
    return cc58[ rgb565 >> 11        ] << 16	// red
	 | cc68[ rgb565 >>  5 & 0x3f ] <<  8	// green
	 | cc58[ rgb565       & 0x1f ];		// blue
}

static inline u16 RGB_to_RGB565 ( u32 rgb )
{
    return cc85[ rgb >> 16 & 0xff ] << 11
	 | cc86[ rgb >>  8 & 0xff ] <<  5
	 | cc85[ rgb       & 0xff ];
}

int ScanOptCmprDefault	( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_NUMERIC_H


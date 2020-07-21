
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_DB_OBJECT_H
#define SZS_DB_OBJECT_H 1

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   enum ObjectFlags_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[ObjectFlags_t]]
typedef enum ObjectFlags_t
{
    OBF_REQUIRED     = 0x000000001,  // for track files required
    OBF_REQUIRED2    = 0x000000002,  // alternativley required
    OBF_OPTIONAL     = 0x000000004,  // optional file
    OBF_OBJECT       = 0x000000008,  // file is object relevant
    OBF_EXTERNAL     = 0x000000010,  // external file
    OBF_UNIQUE       = 0x000000020,  // all instances are identical
    OBF_BRRES        = 0x000000040,  // B: at least one BRRES file exists
    OBF_EFFECT       = 0x000000080,  // E: at least one effect pair (BREFF+BREFT) exists
    OBF_BRASD        = 0x000000100,  // D: at least one BRASD file exists
    OBF_KCL          = 0x000000200,  // K: at least one KCL file exists
    OBF_KMP          = 0x000000400,  // P: at least one KMP file exists
    OBF_OTHER        = 0x000000800,  // x: at least one unknown file exists
    OBF_NO_FILE      = 0x000001000,  // ?: no file found for group
    OBF_SCALE        = 0x000002000,  // is known as scalable
    OBF_ROTATE       = 0x000004000,  // is known as rotatable
    OBF_FALLDOWN     = 0x000008000,  // relevant for fall-down
    OBF_COMMONOBJ    = 0x000010000,  // found in CommonObj*.szs
    OBF_ARCH_REQ     = 0x000020000,  // required for archive => used for WU8 encoding
    OBF_ARCH_OPT     = 0x000040000,  // optional in archive
    OBF_SPECIAL      = 0x000080000,  // special settings -> ObjectSpecial*
    OBF_OBJFLOW      = 0x000100000,  // defined in ObjFlow.bin
    OBF_SOLID        = 0x000200000,  // object is solid
    OBF_ROAD         = 0x000400000,  // object acts like a road
    OBF_NO_ROUTE     = 0x000800000,  // object needs no route
    OBF_OPT_ROUTE    = 0x001000000,  // object has an optional route
    OBF_ALWAYS_ROUTE = 0x002000000,  // object has always a route
    OBF_FNAME        = 0x004000000,  // object need at least 1 file
    OBF_MULTI_FNAME  = 0x008000000,  // object needs 2 or more files
    OBF_F_TRACK_S    = 0x010000000,  // object found in standard track file(s)
    OBF_F_TRACK_D    = 0x020000000,  // object found in _d track file(s)
    OBF_F_ARENA_S    = 0x040000000,  // object found in standard arena file(s)
    OBF_F_ARENA_D    = 0x080000000,  // object found in _d arena file(s)
    OBF_F_OTHER      = 0x100000000,  // object found in other file(s)
    OBF_ITEMBOX      = 0x200000000,  // object is an itembox

    OBF_M_ROUTE      = OBF_NO_ROUTE | OBF_OPT_ROUTE | OBF_ALWAYS_ROUTE,
    OBF_M_TRACK      = OBF_F_TRACK_S | OBF_F_TRACK_D,
    OBF_M_ARENA      = OBF_F_ARENA_S | OBF_F_ARENA_D,
    OBF_M_FOUND      = OBF_M_TRACK | OBF_M_ARENA | OBF_F_OTHER,

    OBF_NONE         = 0x000000000,  // none set
    OBF_ALL          = 0x3ffffffff   // all 34 flags of above

} ObjectFlags_t;

//
///////////////////////////////////////////////////////////////////////////////
////////////////////////   struct ObjSettingFormat_t   ////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[ObjSettingFormat_t]]
typedef struct ObjSettingFormat_t
{
    char	std[4*5+2];		// standard like "%6u %#6x %#6x %6u"
    char	col[4*7+2];		// with color support like "%s%6u %s%#6x %s%#6x %s%6u"
}
ObjSettingFormat_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct ObjectInfo_t   ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[ObjectInfo_t]]
typedef struct ObjectInfo_t
{
    ccp			name;		// NULL or name of object
    ccp			info;		// NULL or info for object
    ObjectFlags_t	flags;		// flags of object
    ccp			charact;	// characteristics based on 'flags'
    const ObjSettingFormat_t
			*format1;	// formats for first 4 settings
    const ObjSettingFormat_t
			*format2;	// formats for second 4 settings
    int			group_idx[3];	// >=0: index into 'DbFileGROUP'
    ccp			fname;		// NULL or filename list
    ccp			param[8];	// NULL or parameter info
}
ObjectInfo_t;

#define N_KMP_GOBJ 756
extern const ObjectInfo_t ObjectInfo[N_KMP_GOBJ];

static inline u16 GetValidObjectId ( u16 obj_id )
{
    if ( obj_id < 0x2000 )
    {
	obj_id &= 0x03ff;
	if ( obj_id < N_KMP_GOBJ )
	    return obj_id;
    }
    return 0;
}

static inline const ObjectInfo_t * GetObjectInfo ( u16 obj_id )
{
    if ( obj_id < 0x2000 )
    {
	obj_id &= 0x03ff;
	if ( obj_id < N_KMP_GOBJ )
	    return ObjectInfo + obj_id;
    }
    return ObjectInfo;
}


//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////   format strings   //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern const ObjSettingFormat_t settings_format_uuuu;
extern const ObjSettingFormat_t settings_format_uxuu;
extern const ObjSettingFormat_t settings_format_uxxu;
extern const ObjSettingFormat_t settings_format_xxxx;

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   enum ObjSpecType_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[ObjSpecType_t]]
typedef enum ObjSpecType_t
{
    OSP_NONE,            // not used
    OSP_FILE_BY_SETTING, // select file by setting
}
__attribute__ ((packed)) ObjSpecType_t;


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////   struct ObjSpec_t   /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[ObjSpec_t]]
typedef struct ObjSpec_t
{
    u16		obj_id;		// id of related object, 0=term
    ObjSpecType_t type;		// type of special
    u8		mode;		// type dependent mode
    ccp		text;		// NULL or type dependent text
}
ObjSpec_t;

#define N_OBJ_SPEC 8
extern const ObjSpec_t ObjSpec[N_OBJ_SPEC+1];


//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////   statistics   ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define OBJ_FW_NAME     18
#define OBJ_FW_INFO    897
#define OBJ_FW_FNAME    46
#define OBJ_FW_CHAR     55
#define OBJ_FW_SETTING 330


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_DB_OBJECT_H


//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_DB_FILE_H
#define SZS_DB_FILE_H 1

#include "lib-std.h"


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   const   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define N_DB_FILE		2445
#define N_DB_FILE_SZS		 111
#define N_DB_FILE_SHA1		 920
#define N_DB_FILE_FILE		 496
#define N_DB_FILE_GROUP		 261

#define N_DB_FILE_REF_SZS	2552
#define N_DB_FILE_REF_SHA1	3363
#define N_DB_FILE_REF_FILE	2941
#define N_DB_FILE_REF_GROUP	 757

#define MAX_DB_FILE_REF_SZS	  77
#define MAX_DB_FILE_REF_SHA1	  88
#define MAX_DB_FILE_REF_FILE	  99
#define MAX_DB_FILE_REF_GROUP	  43

#define MAX_DB_FILE_SIZE	3402114


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////   enum DbType_t   //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbType_t]]
typedef enum DbType_t
{
    DBT_NONE   =  0,  // -: none of below
    DBT_BRRES  =  1,  // b: *.brres
    DBT_KCL    =  2,  // k: *.kcl (not course.kcl)
    DBT_BRASD  =  3,  // d: brasd/*/*.brasd
    DBT_BREFF  =  4,  // f: effect/*/*.breff
    DBT_BREFT  =  5,  // t: effect/*/*.breft
    DBT_POSTEF =  6,  // p: posteffect/*
    DBT__N     =  7,  // -: number types

} DbType_t;

#define DbTypeChars "-bkdftp-"


//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////   enum DbFlags_t   //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFlags_t]]
typedef enum DbFlags_t
{
    DBF_REQUIRED  = 0x00001,  // for track files required
    DBF_REQUIRED2 = 0x00002,  // alternativley required
    DBF_OPTIONAL  = 0x00004,  // optional file
    DBF_OBJECT    = 0x00008,  // file is object relevant
    DBF_EXTERNAL  = 0x00010,  // external file
    DBF_UNIQUE    = 0x00020,  // all instances are identical
    DBF_BRRES     = 0x00040,  // B: at least one BRRES file exists
    DBF_EFFECT    = 0x00080,  // E: at least one effect pair (BREFF+BREFT) exists
    DBF_BRASD     = 0x00100,  // D: at least one BRASD file exists
    DBF_KCL       = 0x00200,  // K: at least one KCL file exists
    DBF_KMP       = 0x00400,  // P: at least one KMP file exists
    DBF_OTHER     = 0x00800,  // x: at least one unknown file exists
    DBF_NO_FILE   = 0x01000,  // ?: no file found for group
    DBF_SCALE     = 0x02000,  // is known as scalable
    DBF_ROTATE    = 0x04000,  // is known as rotatable
    DBF_FALLDOWN  = 0x08000,  // relevant for fall-down
    DBF_COMMONOBJ = 0x10000,  // found in CommonObj*.szs
    DBF_ARCH_REQ  = 0x20000,  // required for archive => used for WU8 encoding
    DBF_ARCH_OPT  = 0x40000,  // optional in archive

    DBF_NONE      = 0x00000,  // none set
    DBF_ALL       = 0x7ffff   // all 19 flags of above

} DbFlags_t;

#define DBF_ARCH_REQUIRED(f) ( (f&DBF_ARCH_REQ) != 0 )
#define DBF_ARCH_OPTIONAL(f) ( (f&DBF_ARCH_OPT) != 0 )
#define DBF_ARCH_SUPPORT(f)  ( (f&(DBF_ARCH_REQ|DBF_ARCH_OPT)) != 0 )

#define OLD_DBF_IN_ARCHIVE(f) ( !(f&DBF_COMMONOBJ) && ( (f&DBF_OBJECT) \
		|| (f&(DBF_EXTERNAL|DBF_UNIQUE)) == DBF_UNIQUE ))


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////   draw data   ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFileDraw_t]]
typedef enum DbFileDraw_t
{
	DFD_TERM,
	DFD_COIN,
	DFD_CUBE,
	DFD_CUBOID,
	DFD_CYLINDER_Y,
	DFD_CYLINDER_Z,
	DFD_ITEMBOX,
	DFD_KINOKO,
	DFD_PYRAMID,
	DFD_RECTANGLE,
	DFD_TREE,
	DFD__N
}
DbFileDraw_t;

extern const s16 DbFileDraw[175];


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////   data base   ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFile_t]]
typedef struct DbFile_t
{
    s16		szs;		// index into table 'DbFileSZS'
    s16		sha1;		// index into table 'DbFileSHA1'
    s16		file;		// index into table 'DbFileFILE'

}
__attribute__ ((packed)) DbFile_t;

extern const DbFile_t DbFile[N_DB_FILE+1];


//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////   szs   ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFileSZS_t]]
typedef struct DbFileSZS_t
{
    s16		ref;		// index into table 'DbFileRefSZS'
    char	id[6];		// alpha numeric ID of track
    ccp		szs_name;	// szs filename
    s16		idx;		// -1 or index
    u16		type;		// type of file
}
__attribute__ ((packed)) DbFileSZS_t;

extern const DbFileSZS_t DbFileSZS[N_DB_FILE_SZS+1];
extern const s16 DbFileRefSZS[N_DB_FILE_REF_SZS+1];


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   sha1   ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFileSHA1_t]]
typedef struct DbFileSHA1_t
{
    s16		ref;		// index into table 'DbFileRefSHA1'
    sha1_hash_t	sha1;		// SHA1 check sum
}
__attribute__ ((packed)) DbFileSHA1_t;

extern const DbFileSHA1_t DbFileSHA1[N_DB_FILE_SHA1+1];
extern const s16 DbFileRefSHA1[N_DB_FILE_REF_SHA1+1];


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   file   ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFileFILE_t]]
typedef struct DbFileFILE_t
{
    s16		ref;		// index into table 'DbFileRefFILE'
    u16		group;		// -1:none, >=0:index into 'DbFileGROUP'
    u32		flags;		// enum DbFlags_t
    u16		draw_index;	// index into draw list
    u8		fform;		// file format (FF_*)
    u8		subtype;	// subfile type (DBT_*)
    ccp		file;		// internal filename without './' prefix
}
__attribute__ ((packed)) DbFileFILE_t;

extern const DbFileFILE_t DbFileFILE[N_DB_FILE_FILE+1];
extern const s16 DbFileRefFILE[N_DB_FILE_REF_FILE+1];


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   group   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DbFileGROUP_t]]
typedef struct DbFileGROUP_t
{
    u32		flags;		// enum DbFlags_t
    s16		ref;		// index into table 'DbFileRefGROUP'
    char	status[6+1];	// flags as text
    char	name[18+1];	// group name
}
__attribute__ ((packed)) DbFileGROUP_t;

extern const DbFileGROUP_t DbFileGROUP[N_DB_FILE_GROUP+1];
extern const s16 DbFileRefGROUP[N_DB_FILE_REF_GROUP+1];

enum // group index of specialgroups
{
    DB_GROUP_COURSE		= 257,
    DB_GROUP_POSTEFFECT		= 258,
    DB_GROUP_USELESS		= 259,
    DB_GROUP__UNKNOWN		= 260,
};


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_DB_FILE_H

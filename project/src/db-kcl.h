
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_DB_KCL_H
#define SZS_DB_KCL_H 1

#include "types.h"


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////   enum kcl_attrib_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum kcl_attrib_t
{

    KCLT_C_ROAD       = 0x000001,  // normal road
    KCLT_C_OFFROAD    = 0x000002,  // offroad
    KCLT_C_FAST       = 0x000004,  // boost and trick
    KCLT_C_HALFPIPE   = 0x000008,  // halfpipe
    KCLT_C_MOVING     = 0x000010,  // moving terrain
    KCLT_C_WATER      = 0x000020,  // shallow water
    KCLT_C_CANNON     = 0x000040,  // cannon activator
    KCLT_C_WALL       = 0x000080,  // wall
    KCLT_C_FALL       = 0x000100,  // fall or out of bounds
    KCLT_C_EFFECT     = 0x000200,  // sound trigger and other effects
    KCLT_C_ITEM       = 0x000400,  // for items only
    KCLT_C_UNKNOWN    = 0x000800,  // unknown type
     KCLT_M_CLASS     = 0x000fff,  // mask of classes

    KCLT_BORDER       = 0x001000,  // road or wall
    KCLT_DRIVE        = 0x002000,  // is driveable
    KCLT_BOOST        = 0x004000,  // boost
    KCLT_TRICK        = 0x008000,  // trickable
    KCLT_SOUND        = 0x010000,  // sound effect
    KCLT_ITEM         = 0x020000,  // special for items
    KCLT_UNKNOWN      = 0x040000,  // unknown
    KCLT_VARIANT_X    = 0x080000,  // unknown variant
    KCLT_VARIANT_A    = 0x100000,  // variant A
    KCLT_VARIANT_B    = 0x200000,  // variant B
     KCLT_M_FEATURES  = 0x3ff000,  // mask of base features

    KCLT_NONE         = 0x000000,  // none set
    KCLT_M_ALL        = 0x3fffff   // all flags of above

} kcl_attrib_t;

///////////////////////////////////////////////////////////////////////////////

#define N_KCL_CLASS     12
#define N_KCL_TYPE    0x20
#define N_KCL_ATTRIB    22
#define N_KCL_FLAG 0x10000
#define N_KCL_AUTO_FLAGS 200
#define N_KCL_USER_FLAGS 200
#define N_KCL_COLORS 272

#define IS_KCL_TYPE(f,t) ( (f) < N_KCL_FLAG && ((f)&0x1f) == (t) )
#define GET_KCL_TYPE(f) ( (f) < N_KCL_FLAG ? (f)&0x1f : -1 )

enum
{
    KCL_FLAG_UNKNOWN = N_KCL_FLAG,

    KCL_FLAG_CKPT_WARNING,
    KCL_FLAG_CKPT_WARNING_T,
    KCL_FLAG_CKPT_WARNING_TT,
    KCL_FLAG_CKPT_LC,
    KCL_FLAG_CKPT_LC_T,
    KCL_FLAG_CKPT_LC_TT,
    KCL_FLAG_CKPT_MD,
    KCL_FLAG_CKPT_MD_T,
    KCL_FLAG_CKPT_MD_TT,
    KCL_FLAG_CKPT_MD5,
    KCL_FLAG_CKPT_MD5_T,
    KCL_FLAG_CKPT_MD5_TT,
    KCL_FLAG_CKPT_ETC,
    KCL_FLAG_CKPT_ETC_T,
    KCL_FLAG_CKPT_ETC_TT,
    KCL_FLAG_CKPT_TIE_LT,
    KCL_FLAG_CKPT_TIE_RT,
    KCL_FLAG_CKPT_TIE_GROUP_LT,
    KCL_FLAG_CKPT_TIE_GROUP_RT,
    KCL_FLAG_CKPT_TIE_NEXT_LT,
    KCL_FLAG_CKPT_TIE_NEXT_RT,
    KCL_FLAG_CKPT_JGPT,
    KCL_FLAG_JGPT,
    KCL_FLAG_JGPT_AREA,
    KCL_FLAG_KTPT,
    KCL_FLAG_KTPT_AREA,
    KCL_FLAG_ENPT,
    KCL_FLAG_ENPT_SINGLE,
    KCL_FLAG_ENPT_TIE_BI,
    KCL_FLAG_ENPT_TIE_NEXT,
    KCL_FLAG_ENPT_TIE_PREV,
    KCL_FLAG_ENPT_DISPATCH,
    KCL_FLAG_ITPT,
    KCL_FLAG_ITPT_SINGLE,
    KCL_FLAG_ITPT_TIE_BI,
    KCL_FLAG_ITPT_TIE_NEXT,
    KCL_FLAG_ITPT_TIE_PREV,
    KCL_FLAG_ITPT_DISPATCH,
    KCL_FLAG_CNPT_LANDING,
    KCL_FLAG_CNPT_WAY,
    KCL_FLAG_AREA_ARROW,
    KCL_FLAG_AREA_BORDER,
    KCL_FLAG_POTI0,
    KCL_FLAG_POTI1,
    KCL_FLAG_POTI2,
    KCL_FLAG_POTI3,
    KCL_FLAG_ITEMBOX,
    KCL_FLAG_ITEMBOX_ENEMY,
    KCL_FLAG_ITEMBOX_PLAYER,
    KCL_FLAG_ITEMROUTE,
    KCL_FLAG_ITEMROUTE_ENEMY,
    KCL_FLAG_ITEMROUTE_PLAYER,
    KCL_FLAG_KINOKO_GREEN,
    KCL_FLAG_KINOKO_RED,
    KCL_FLAG_BRIDGE,
    KCL_FLAG_DOKAN,
    KCL_FLAG_KAREHAYAMA,
    KCL_FLAG_OBAKEBLOCK,
    KCL_FLAG_TREE_CROWN,
    KCL_FLAG_TREE_TRUNK,
    KCL_FLAG_WOODBOX,
    KCL_FLAG_COIN_START,
    KCL_FLAG_COIN_RESPAWN,
    KCL_FLAG_COIN_INVALID,
    KCL_FLAG_SOLIDOBJ,
    KCL_FLAG_SOLID_ROUTE,
    KCL_FLAG_DECORATION,
    KCL_FLAG_ZERO_GROUND_BLACK,
    KCL_FLAG_ZERO_GROUND_WHITE,
    KCL_FLAG_RED,
    KCL_FLAG_RED_T,
    KCL_FLAG_RED_TT,
    KCL_FLAG_RED_ORANGE,
    KCL_FLAG_RED_ORANGE_T,
    KCL_FLAG_RED_ORANGE_TT,
    KCL_FLAG_ORANGE,
    KCL_FLAG_ORANGE_T,
    KCL_FLAG_ORANGE_TT,
    KCL_FLAG_ORANGE_YELLOW,
    KCL_FLAG_ORANGE_YELLOW_T,
    KCL_FLAG_ORANGE_YELLOW_TT,
    KCL_FLAG_YELLOW,
    KCL_FLAG_YELLOW_T,
    KCL_FLAG_YELLOW_TT,
    KCL_FLAG_YELLOW_GREEN,
    KCL_FLAG_YELLOW_GREEN_T,
    KCL_FLAG_YELLOW_GREEN_TT,
    KCL_FLAG_GREEN,
    KCL_FLAG_GREEN_T,
    KCL_FLAG_GREEN_TT,
    KCL_FLAG_GREEN_CYAN,
    KCL_FLAG_GREEN_CYAN_T,
    KCL_FLAG_GREEN_CYAN_TT,
    KCL_FLAG_CYAN,
    KCL_FLAG_CYAN_T,
    KCL_FLAG_CYAN_TT,
    KCL_FLAG_CYAN_BLUE,
    KCL_FLAG_CYAN_BLUE_T,
    KCL_FLAG_CYAN_BLUE_TT,
    KCL_FLAG_BLUE,
    KCL_FLAG_BLUE_T,
    KCL_FLAG_BLUE_TT,
    KCL_FLAG_BLUE_MAGENTA,
    KCL_FLAG_BLUE_MAGENTA_T,
    KCL_FLAG_BLUE_MAGENTA_TT,
    KCL_FLAG_MAGENTA,
    KCL_FLAG_MAGENTA_T,
    KCL_FLAG_MAGENTA_TT,
    KCL_FLAG_MAGENTA_RED,
    KCL_FLAG_MAGENTA_RED_T,
    KCL_FLAG_MAGENTA_RED_TT,
    KCL_FLAG_BROWN,
    KCL_FLAG_BROWN_T,
    KCL_FLAG_BROWN_TT,
    KCL_FLAG_WHITE,
    KCL_FLAG_WHITE_T,
    KCL_FLAG_WHITE_TT,
    KCL_FLAG_GREY,
    KCL_FLAG_GREY_T,
    KCL_FLAG_GREY_TT,
    KCL_FLAG_BLACK,
    KCL_FLAG_BLACK_T,
    KCL_FLAG_BLACK_TT,
    KCL_FLAG_GOLD,
    KCL_FLAG_GOLD_T,
    KCL_FLAG_GOLD_TT,
    KCL_FLAG_SILVER,
    KCL_FLAG_SILVER_T,
    KCL_FLAG_SILVER_TT,
    KCL_FLAG_BRONZE,
    KCL_FLAG_BRONZE_T,
    KCL_FLAG_BRONZE_TT,
    KCL_FLAG_TEST0,
    KCL_FLAG_TEST0_T,
    KCL_FLAG_TEST0_TT,
    KCL_FLAG_TEST1,
    KCL_FLAG_TEST1_T,
    KCL_FLAG_TEST1_TT,
    KCL_FLAG_TEST2,
    KCL_FLAG_TEST2_T,
    KCL_FLAG_TEST2_TT,
    KCL_FLAG_TEST3,
    KCL_FLAG_TEST3_T,
    KCL_FLAG_TEST3_TT,
    KCL_FLAG_TEST4,
    KCL_FLAG_TEST4_T,
    KCL_FLAG_TEST4_TT,
    KCL_FLAG_TEST5,
    KCL_FLAG_TEST5_T,
    KCL_FLAG_TEST5_TT,
    KCL_FLAG_TEST6,
    KCL_FLAG_TEST6_T,
    KCL_FLAG_TEST6_TT,
    KCL_FLAG_TEST7,
    KCL_FLAG_TEST7_T,
    KCL_FLAG_TEST7_TT,
    KCL_FLAG_TEST8,
    KCL_FLAG_TEST8_T,
    KCL_FLAG_TEST8_TT,

    NN_KCL_FLAG,

    KCL_FLAG_AUTO_0   = NN_KCL_FLAG,
    KCL_FLAG_AUTO_MAX = KCL_FLAG_AUTO_0 + N_KCL_AUTO_FLAGS - 1,
    KCL_FLAG_USER_0   = KCL_FLAG_AUTO_MAX + 1,
    KCL_FLAG_USER_MAX = KCL_FLAG_USER_0 + N_KCL_USER_FLAGS - 1,
    NNN_KCL_FLAG
};

ccp GetFlagName ( uint kcl_flag );
ccp GetFlagGroupName ( uint kcl_flag );

static inline bool IsAutoFlagKCL ( uint flag )
	    { return flag >= KCL_FLAG_AUTO_0 && flag <= KCL_FLAG_AUTO_MAX; }

static inline bool IsUserFlagKCL ( uint flag )
	    { return flag >= KCL_FLAG_USER_0 && flag <= KCL_FLAG_USER_MAX; }


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////   struct kcl_attrib_name_t   /////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kcl_attrib_name_t
{
	kcl_attrib_t	attrib;		// attribute
	ccp		name;		// name of attribute

} kcl_attrib_name_t;

extern const kcl_attrib_name_t kcl_attrib_name[N_KCL_ATTRIB+1];


//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct kcl_class_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kcl_class_t
{
	u16		cls;		// class id, index to self
	u16		ncol;		// number of colors in mtl
	u16		color_index;	// first index into kcl_colors[]
	ccp		name;		// short name
	ccp		info;		// description
	kcl_attrib_t	attrib;		// attributes

} kcl_class_t;

extern const kcl_class_t kcl_class[N_KCL_CLASS+1];
extern const u8 kcl_color[N_KCL_COLORS][4];


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////   struct kcl_type_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct kcl_type_t
{
	u16		type;		// type id, index to self
	u16		cls;		// class index
	ccp		info;		// description
	kcl_attrib_t	attrib;		// attributes

} kcl_type_t;

extern const kcl_type_t kcl_type[N_KCL_TYPE+1];


//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   kcl_color_key_tab[]   ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern const KeywordTab_t kcl_color_key_tab[];


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_DB_KCL_H

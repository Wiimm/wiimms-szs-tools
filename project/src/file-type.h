
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_FILE_TYPE_H
#define SZS_FILE_TYPE_H 1

#include "types.h"
#include "dclib-basics.h"


//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////   enum ff_attrib_t   /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[ff_attrib_t]]
typedef enum ff_attrib_t
{
    FFT_VALID       = 0x00001,  // a valid file format

    FFT_COMPRESS    = 0x00002,  // is a compression format
    FFT_ARCHIVE     = 0x00004,  // is a supported archive
    FFT_GRAPHIC     = 0x00008,  // graphic image format
    FFT_TEXT        = 0x00010,  // (decoded) text file
     FFT_M_FTYPE    = 0x0001e,  // mask of file types

    FFT_BRSUB       = 0x00020,  // is a BRRES sub file
    FFT_BRSUB2      = 0x00040,  // have a scondary file structure like a BRSUB
    FFT_EXTERNAL    = 0x00080,  // external file format
    FFT_TRACK       = 0x00100,  // can be a track file
     FFT_M_CLASS    = 0x001e0,  // mask of file classes

    FFT_CREATE      = 0x00200,  // creation supported
    FFT_EXTRACT     = 0x00400,  // extracting supported
    FFT_CUT         = 0x00800,  // 'cut-files' supported
    FFT_DECODE      = 0x01000,  // decoding supported
    FFT_ENCODE      = 0x02000,  // encoding supported
    FFT_PARSER      = 0x04000,  // parser support on encoding
    FFT_PATCH       = 0x08000,  // patching of binaries
    FFT_LINK        = 0x10000,  // hardlinks supported
     FFT_M_SUPPORT  = 0x1fe00,  // mask of supported operations

    FFT_NONE        = 0x00000,  // none set
    FFT_M_ALL       = 0x1ffff   // all flags of above

} ff_attrib_t;


//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   enum file_format_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[file_format_t]]
typedef enum file_format_t
{
	FF_UNKNOWN = 0, // definitley 0

	//--- file modes

	FF_YAZ0,	//  1
	FF_YAZ1,	//  2
	FF_XYZ,		//  3
	FF_BZ,		//  4
	FF_BZIP2,	//  5
	FF_LZ,		//  6
	FF_LZMA,	//  7
	FF_XZ,		//  8

	FF_U8,		//  9
	FF_WU8,		// 10
	FF_RARC,	// 11
	FF_BRRES,	// 12
	FF_BREFF,	// 13
	FF_BREFT,	// 14
	FF_RKC,		// 15
	FF_PACK,	// 16
	FF_USE_LTA,	// 17
	FF_LTA,		// 18
	FF_LFL,		// 19

	FF_CHR,		// 20
	FF_CLR,		// 21
	FF_MDL,		// 22
	FF_PAT,		// 23
	FF_SCN,		// 24
	FF_SHP,		// 25
	FF_SRT,		// 26
	FF_TEX,		// 27

	FF_TEX_CT,	// 28
	FF_CTDEF,	// 29

	FF_CT0_CODE,	// 30
	FF_CT0_DATA,	// 31
	FF_CT1_CODE,	// 32
	FF_CT1_DATA,	// 33

	FF_CUP1,	// 34
	FF_CRS1,	// 35
	FF_MOD1,	// 36
	FF_MOD2,	// 37
	FF_OVR1,	// 38

	FF_LE_BIN,	// 39
	FF_LEX,		// 40
	FF_LEX_TXT,	// 41

	FF_LPAR,	// 42
	FF_LEDEF,	// 43
	FF_LEDIS,	// 44
	FF_LEREF,	// 45
	FF_LESTR,	// 46
	FF_SHA1REF,	// 47
	FF_SHA1ID,	// 48
	FF_PREFIX,	// 49
	FF_MTCAT,	// 50
	FF_CT_SHA1,	// 51

	FF_MDL_TXT,	// 52

	FF_PAT_TXT,	// 53

	FF_TPL,		// 54
	FF_TPLX,	// 55
	FF_CUPICON,	// 56
	FF_BTI,		// 57
	FF_BREFT_IMG,	// 58

	FF_BMG,		// 59
	FF_BMG_TXT,	// 60

	FF_KCL,		// 61
	FF_KCL_TXT,	// 62
	FF_WAV_OBJ,	// 63
	FF_SKP_OBJ,	// 64

	FF_KMP,		// 65
	FF_KMP_TXT,	// 66

	FF_ITEMSLT,	// 67
	FF_ITEMSLT_TXT,	// 68

	FF_KMG,		// 69
	FF_KMG_TXT,	// 70

	FF_KRM,		// 71
	FF_KRM_TXT,	// 72

	FF_KRT,		// 73
	FF_KRT_TXT,	// 74

	FF_OBJFLOW,	// 75
	FF_OBJFLOW_TXT,	// 76

	FF_GH_ITEM,	// 77
	FF_GH_ITEM_TXT,	// 78

	FF_GH_IOBJ,	// 79
	FF_GH_IOBJ_TXT,	// 80

	FF_GH_KART,	// 81
	FF_GH_KART_TXT,	// 82

	FF_GH_KOBJ,	// 83
	FF_GH_KOBJ_TXT,	// 84

	FF_DRIVER,	// 85
	FF_VEHICLE,	// 86

	FF_BRASD,	// 87
	FF_RKG,		// 88
	FF_RKCO,	// 89

	FF_STATICR,	// 90
	FF_DOL,		// 91

	FF_GCT,		// 92
	FF_GCT_TXT,	// 93
	FF_GCH,		// 94
	FF_WCH,		// 95
	FF_WPF,		// 96
	FF_XPF,		// 97
	FF_DISTRIB,	// 98

	FF_PNG,		// 99
	FF_PORTDB,	// 100

	FF_TXT,		// 101
	FF_SCRIPT,	// 102
	FF_JSON,	// 103
	FF_SH,		// 104
	FF_BASH,	// 105
	FF_PHP,		// 106
	FF_MAKEDOC,	// 107

	FF_DIRECTORY,	// 108

	//--- number of elements

	FF_N,
	FF_AUTO		= FF_N,

	//--- some more values

	FF_INVALID	= -1,
	FF__FIRST_BRSUB	= FF_CHR,
	FF__LAST_BRSUB	= FF_TEX,

} file_format_t;


//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct file_type_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[file_type_t]]
typedef struct file_type_t
{
	file_format_t fform;	// file format
	file_format_t fform_bin;// file format of binary partner (or 0=FF_UNKOWN)
	file_format_t fform_txt;// file format of text partner (or 0=FF_UNKOWN)

	ccp name;		// name
	ccp ext;		// standard file extension
	ccp ext_compr;		// file extension if compressed
	ccp ext_magic;		// magic based file extension

	ff_attrib_t attrib;	// file atributes

	u8  magic_len;		// length of magic, usually 0, 4 or 8 bytes
	u8  magic[8+1];		// usual magic, NULL terminated
	ccp subdir;		// NULL or BRRES directory name

	ccp read_info;		// info about reading support, never NULL
	ccp write_info;		// info about writing support, never NULL
	ccp remark;		// remark about file type, never NULL

} file_type_t;

extern const file_type_t FileTypeTab[FF_N+1];
extern const struct KeywordTab_t cmdtab_FileType[];
extern const char filetype_info_unknown[];
extern const char filetype_info_not_supported[];


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_FILE_TYPE_H

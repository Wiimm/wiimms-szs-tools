
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
    FFT_VALID       = 0x0001,  // a valid file format

    FFT_COMPRESS    = 0x0002,  // is a compression format
    FFT_ARCHIVE     = 0x0004,  // is a supported archive
    FFT_GRAPHIC     = 0x0008,  // graphic image format
    FFT_TEXT        = 0x0010,  // (decoded) text file
     FFT_M_FTYPE    = 0x001e,  // mask of file types

    FFT_BRSUB       = 0x0020,  // is a BRRES sub file
    FFT_BRSUB2      = 0x0040,  // have a scondary file structure like a BRSUB
    FFT_EXTERNAL    = 0x0080,  // external file format
     FFT_M_CLASS    = 0x00e0,  // mask of file classes

    FFT_CREATE      = 0x0100,  // creation supported
    FFT_EXTRACT     = 0x0200,  // extracting supported
    FFT_CUT         = 0x0400,  // 'cut-files' supported
    FFT_DECODE      = 0x0800,  // decoding supported
    FFT_ENCODE      = 0x1000,  // encoding supported
    FFT_PARSER      = 0x2000,  // parser support on encoding
    FFT_PATCH       = 0x4000,  // patching of binaries
    FFT_LINK        = 0x8000,  // hardlinks supported
     FFT_M_SUPPORT  = 0xff00,  // mask of supported operations

    FFT_NONE        = 0x0000,  // none set
    FFT_M_ALL       = 0xffff   // all flags of above

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
	FF_BZ2,		//  5

	FF_U8,		//  6
	FF_WU8,		//  7
	FF_RARC,	//  8
	FF_BRRES,	//  9
	FF_BREFF,	// 10
	FF_BREFT,	// 11
	FF_RKC,		// 12
	FF_PACK,	// 13

	FF_CHR,		// 14
	FF_CLR,		// 15
	FF_MDL,		// 16
	FF_PAT,		// 17
	FF_SCN,		// 18
	FF_SHP,		// 19
	FF_SRT,		// 20
	FF_TEX,		// 21

	FF_TEX_CT,	// 22
	FF_CT_TEXT,	// 23

	FF_CT0_CODE,	// 24
	FF_CT0_DATA,	// 25
	FF_CT1_CODE,	// 26
	FF_CT1_DATA,	// 27

	FF_CUP1,	// 28
	FF_CRS1,	// 29
	FF_MOD1,	// 30
	FF_MOD2,	// 31
	FF_OVR1,	// 32

	FF_LE_BIN,	// 33
	FF_LEX,		// 34
	FF_LEX_TXT,	// 35

	FF_LPAR,	// 36

	FF_MDL_TXT,	// 37

	FF_PAT_TXT,	// 38

	FF_TPL,		// 39
	FF_BTI,		// 40
	FF_BREFT_IMG,	// 41

	FF_BMG,		// 42
	FF_BMG_TXT,	// 43

	FF_KCL,		// 44
	FF_KCL_TXT,	// 45
	FF_WAV_OBJ,	// 46
	FF_SKP_OBJ,	// 47

	FF_KMP,		// 48
	FF_KMP_TXT,	// 49

	FF_ITEMSLT,	// 50
	FF_ITEMSLT_TXT,	// 51

	FF_KMG,		// 52
	FF_KMG_TXT,	// 53

	FF_KRM,		// 54
	FF_KRM_TXT,	// 55

	FF_KRT,		// 56
	FF_KRT_TXT,	// 57

	FF_OBJFLOW,	// 58
	FF_OBJFLOW_TXT,	// 59

	FF_GH_ITEM,	// 60
	FF_GH_ITEM_TXT,	// 61

	FF_GH_IOBJ,	// 62
	FF_GH_IOBJ_TXT,	// 63

	FF_GH_KART,	// 64
	FF_GH_KART_TXT,	// 65

	FF_GH_KOBJ,	// 66
	FF_GH_KOBJ_TXT,	// 67

	FF_DRIVER,	// 68
	FF_VEHICLE,	// 69

	FF_BRASD,	// 70
	FF_RKG,		// 71
	FF_RKCO,	// 72

	FF_STATICR,	// 73
	FF_DOL,		// 74

	FF_GCT,		// 75
	FF_GCH,		// 76
	FF_WCH,		// 77
	FF_WPF,		// 78
	FF_XPF,		// 79
	FF_DISTRIB,	// 80

	FF_PNG,		// 81

	FF_TXT,		// 82
	FF_SCRIPT,	// 83
	FF_JSON,	// 84
	FF_SH,		// 85
	FF_BASH,	// 86
	FF_PHP,		// 87
	FF_MAKEDOC,	// 88

	FF_DIRECTORY,	// 89

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

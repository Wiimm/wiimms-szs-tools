
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#include "file-type.h"


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////   FileTypeTab[]   //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const char filetype_info_unknown[] = "?";
const char filetype_info_not_supported[] = "not supported";

const file_type_t FileTypeTab[FF_N+1] =
{

 // FF_UNKNOWN = 0
    {
	FF_UNKNOWN, 0, 0, "?",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	filetype_info_not_supported,
	filetype_info_not_supported,
	"Unknown file"
    },

 // FF_YAZ0 = 1
    {
	FF_YAZ0, 0, 0, "YAZ0",
	".szs", ".szs", ".szs",
	FFT_VALID | FFT_COMPRESS,
	4, {0x59,0x61,0x7a,0x30}, // "Yaz0"
	0,
	"0",
	"0,1",
	"YAZ compression"
    },

 // FF_YAZ1 = 2
    {
	FF_YAZ1, 0, 0, "YAZ1",
	".szs", ".szs", ".szs",
	FFT_VALID | FFT_COMPRESS,
	4, {0x59,0x61,0x7a,0x31}, // "Yaz1"
	0,
	"1",
	"0,1",
	"YAZ compression"
    },

 // FF_XYZ = 3
    {
	FF_XYZ, 0, 0, "XYZ",
	".xyz", ".szs", ".xyz",
	FFT_VALID | FFT_COMPRESS,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"disguise YAZ compression"
    },

 // FF_BZ = 4
    {
	FF_BZ, 0, 0, "BZ",
	".bz", ".szs", ".bz",
	FFT_VALID | FFT_COMPRESS,
	4, {0x57,0x42,0x5a,0x61}, // "WBZa"
	0,
	MinusString,
	MinusString,
	"bzip2 compression used"
    },

 // FF_BZ2 = 5
    {
	FF_BZ2, 0, 0, "BZ2",
	".bz2", ".szs", ".bz2",
	FFT_VALID | FFT_COMPRESS,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"bzip2 compressed file"
    },

 // FF_U8 = 6
    {
	FF_U8, 0, 0, "U8",
	".u8", ".szs", ".u8",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT | FFT_CUT | FFT_LINK,
	4, {0x55,0xaa,0x38,0x2d}, // "Uª8-"
	0,
	MinusString,
	MinusString,
	"Nintendos archive format"
    },

 // FF_WU8 = 7
    {
	FF_WU8, 0, 0, "WU8",
	".wu8", ".wu8", ".wu8",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT | FFT_CUT | FFT_LINK,
	4, {0x57,0x55,0x38,0x61}, // "WU8a"
	0,
	MinusString,
	MinusString,
	"Wiimms differential U8"
    },

 // FF_RARC = 8
    {
	FF_RARC, 0, 0, "RARC",
	".rarc", ".arc", ".rarc",
	FFT_VALID | FFT_ARCHIVE | FFT_EXTRACT | FFT_CUT,
	4, {0x52,0x41,0x52,0x43}, // "RARC"
	0,
	MinusString,
	filetype_info_not_supported,
	"Archive format for objects"
    },

 // FF_BRRES = 9
    {
	FF_BRRES, 0, 0, "BRRES",
	".brres", ".szs", ".bres",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT | FFT_CUT,
	4, {0x62,0x72,0x65,0x73}, // "bres"
	0,
	MinusString,
	MinusString,
	"Archive format for objects"
    },

 // FF_BREFF = 10
    {
	FF_BREFF, 0, 0, "BREFF",
	".breff", ".szs", ".reff",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT,
	4, {0x52,0x45,0x46,0x46}, // "REFF"
	0,
	"9,11",
	"9,11",
	"Main file of an effect pair"
    },

 // FF_BREFT = 11
    {
	FF_BREFT, 0, 0, "BREFT",
	".breft", ".szs", ".reft",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT | FFT_CUT,
	4, {0x52,0x45,0x46,0x54}, // "REFT"
	0,
	"9,11",
	"9,11",
	"Image file of an effect pair"
    },

 // FF_RKC = 12
    {
	FF_RKC, 0, 0, "RKC",
	".rkc", ".szs", ".rkc",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT | FFT_CUT,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_PACK = 13
    {
	FF_PACK, 0, 0, "PACK",
	".pack", ".szs", ".pack",
	FFT_VALID | FFT_ARCHIVE | FFT_CREATE | FFT_EXTRACT | FFT_CUT,
	4, {0x50,0x41,0x43,0x4b}, // "PACK"
	0,
	MinusString,
	MinusString,
	"Simple archive format"
    },

 // FF_CHR = 14
    {
	FF_CHR, 0, 0, "CHR",
	".chr", ".szs", ".chr0",
	FFT_VALID | FFT_BRSUB | FFT_BRSUB2 | FFT_CUT,
	4, {0x43,0x48,0x52,0x30}, // "CHR0"
	"AnmChr(NW4R)",
	"5,(*)",
	filetype_info_not_supported,
	"Model movement animations"
    },

 // FF_CLR = 15
    {
	FF_CLR, 0, 0, "CLR",
	".clr", ".szs", ".clr0",
	FFT_VALID | FFT_BRSUB | FFT_BRSUB2 | FFT_CUT,
	4, {0x43,0x4c,0x52,0x30}, // "CLR0"
	"AnmClr(NW4R)",
	"4,(*)",
	filetype_info_not_supported,
	"Colour changing animations"
    },

 // FF_MDL = 16
    {
	FF_MDL, FF_MDL, FF_MDL_TXT, "MDL",
	".mdl", ".szs", ".mdl0",
	FFT_VALID | FFT_BRSUB | FFT_BRSUB2 | FFT_CUT | FFT_DECODE | FFT_PATCH,
	4, {0x4d,0x44,0x4c,0x30}, // "MDL0"
	"3DModels(NW4R)",
	"11,(*)",
	filetype_info_not_supported,
	"Model files"
    },

 // FF_PAT = 17
    {
	FF_PAT, FF_PAT, FF_PAT_TXT, "PAT",
	".pat", ".szs", ".pat0",
	FFT_VALID | FFT_BRSUB | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	4, {0x50,0x41,0x54,0x30}, // "PAT0"
	"AnmTexPat(NW4R)",
	"4,(*)",
	"4",
	"Texture swapping animations"
    },

 // FF_SCN = 18
    {
	FF_SCN, 0, 0, "SCN",
	".scn", ".szs", ".scn0",
	FFT_VALID | FFT_BRSUB | FFT_BRSUB2 | FFT_CUT,
	4, {0x53,0x43,0x4e,0x30}, // "SCN0"
	"AnmScn(NW4R)",
	"5,(*)",
	filetype_info_not_supported,
	"Polygon morphing animations"
    },

 // FF_SHP = 19
    {
	FF_SHP, 0, 0, "SHP",
	".shp", ".szs", ".shp0",
	FFT_VALID | FFT_BRSUB | FFT_BRSUB2 | FFT_CUT,
	4, {0x53,0x48,0x50,0x30}, // "SHP0"
	"AnmShp(NW4R)",
	"4,(*)",
	filetype_info_not_supported,
	filetype_info_unknown
    },

 // FF_SRT = 20
    {
	FF_SRT, 0, 0, "SRT",
	".srt", ".szs", ".srt0",
	FFT_VALID | FFT_BRSUB | FFT_BRSUB2 | FFT_CUT,
	4, {0x53,0x52,0x54,0x30}, // "SRT0"
	"AnmTexSrt(NW4R)",
	"5,(*)",
	filetype_info_not_supported,
	"Texture movement animations"
    },

 // FF_TEX = 21
    {
	FF_TEX, 0, 0, "TEX",
	".tex", ".szs", ".tex0",
	FFT_VALID | FFT_GRAPHIC | FFT_BRSUB | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	4, {0x54,0x45,0x58,0x30}, // "TEX0"
	"Textures(NW4R)",
	"3,(*)",
	"3",
	"Texture file"
    },

 // FF_TEX_CT = 22
    {
	FF_TEX_CT, 0, 0, "TEX+CT",
	".tex", ".szs", ".tex0",
	FFT_VALID | FFT_GRAPHIC | FFT_BRSUB | FFT_CUT | FFT_DECODE | FFT_ENCODE | FFT_PATCH,
	0, {0}, // no magic
	0,
	"3,(*)",
	"3",
	"Texture file with CTGP-CODE"
    },

 // FF_CTDEF = 23
    {
	FF_CTDEF, 0, 0, "CT-DEF",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x43,0x54,0x2d,0x43,0x4f,0x44,0x45}, // "#CT-CODE"
	0,
	MinusString,
	MinusString,
	"CT-CODE definition file"
    },

 // FF_CT0_CODE = 24
    {
	FF_CT0_CODE, 0, 0, "C0CODE",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-CODE for main.dol section T2"
    },

 // FF_CT0_DATA = 25
    {
	FF_CT0_DATA, 0, 0, "C0DATA",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA for main.dol section D8"
    },

 // FF_CT1_CODE = 26
    {
	FF_CT1_CODE, 0, 0, "C1CODE",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-CODE for strap TEX0 file"
    },

 // FF_CT1_DATA = 27
    {
	FF_CT1_DATA, 0, 0, "C1DATA",
	".ctcode", ".szs", ".ctcode",
	FFT_VALID | FFT_CUT | FFT_DECODE | FFT_PATCH,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA for strap TEX0 file"
    },

 // FF_CUP1 = 28
    {
	FF_CUP1, 0, 0, "CUP1",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA section: Cups"
    },

 // FF_CRS1 = 29
    {
	FF_CRS1, 0, 0, "CRS1",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA section: Tracks"
    },

 // FF_MOD1 = 30
    {
	FF_MOD1, 0, 0, "MOD1",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA section: main.dol patches"
    },

 // FF_MOD2 = 31
    {
	FF_MOD2, 0, 0, "MOD2",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA section: StaticR.rel patches"
    },

 // FF_OVR1 = 32
    {
	FF_OVR1, 0, 0, "OVR1",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"CT-DATA section: Speedometer"
    },

 // FF_LE_BIN = 33
    {
	FF_LE_BIN, 0, 0, "LE-BIN",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_CUT | FFT_DECODE | FFT_PATCH,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"LE-CODE binary"
    },

 // FF_LEX = 34
    {
	FF_LEX, FF_LEX, FF_LEX_TXT, "LEX",
	".lex", ".szs", ".lex",
	FFT_VALID | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	4, {0x4c,0x45,0x2d,0x58}, // "LE-X"
	0,
	MinusString,
	MinusString,
	"LECODE extension file"
    },

 // FF_LEX_TXT = 35
    {
	FF_LEX_TXT, FF_LEX, FF_LEX_TXT, "LEX-TXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	4, {0x23,0x4c,0x45,0x58}, // "#LEX"
	0,
	MinusString,
	MinusString,
	"Text version of LEX"
    },

 // FF_LPAR = 36
    {
	FF_LPAR, 0, 0, "LPAR",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x4c,0x45,0x2d,0x4c,0x50,0x41,0x52}, // "#LE-LPAR"
	0,
	MinusString,
	MinusString,
	"Text file with LE-CODE parameters"
    },

 // FF_LEDEF = 37
    {
	FF_LEDEF, 0, 0, "LE-DEF",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x4c,0x45,0x2d,0x44,0x45,0x46,0x31}, // "#LE-DEF1"
	0,
	MinusString,
	MinusString,
	"LE-CODE definition file"
    },

 // FF_LEDIS = 38
    {
	FF_LEDIS, 0, 0, "LE-DIS",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE,
	8, {0x23,0x4c,0x45,0x2d,0x44,0x49,0x53,0x54}, // "#LE-DIST"
	0,
	MinusString,
	MinusString,
	"Text file with CT+LE-CODE distribution settings"
    },

 // FF_LEREF = 39
    {
	FF_LEREF, 0, 0, "LE-REF",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE,
	8, {0x23,0x4c,0x45,0x2d,0x52,0x45,0x46,0x31}, // "#LE-REF1"
	0,
	MinusString,
	MinusString,
	"Text file as track reference"
    },

 // FF_LESTR = 40
    {
	FF_LESTR, 0, 0, "LE-STR",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE,
	8, {0x23,0x4c,0x45,0x2d,0x53,0x54,0x52,0x31}, // "#LE-STR1"
	0,
	MinusString,
	MinusString,
	"Text file strings for tracks"
    },

 // FF_SHA1REF = 41
    {
	FF_SHA1REF, 0, 0, "SHA1REF",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_ENCODE,
	8, {0x23,0x53,0x48,0x41,0x31,0x52,0x45,0x46}, // "#SHA1REF"
	0,
	MinusString,
	MinusString,
	"Text file with SHA1 of tracks"
    },

 // FF_PREFIX = 42
    {
	FF_PREFIX, 0, 0, "PREFIX",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE,
	8, {0x23,0x50,0x52,0x45,0x46,0x49,0x58,0x31}, // "#PREFIX1"
	0,
	MinusString,
	MinusString,
	"Text file with console/game prefixes and info"
    },

 // FF_MTCAT = 43
    {
	FF_MTCAT, 0, 0, "MTCAT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE,
	8, {0x23,0x4d,0x54,0x43,0x41,0x54,0x30,0x33}, // "#MTCAT03"
	0,
	MinusString,
	MinusString,
	"Text file with MKW Track Categories"
    },

 // FF_MDL_TXT = 44
    {
	FF_MDL_TXT, FF_MDL, FF_MDL_TXT, "MDLTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT,
	4, {0x23,0x4d,0x44,0x4c}, // "#MDL"
	0,
	filetype_info_not_supported,
	MinusString,
	"Text version of MDL"
    },

 // FF_PAT_TXT = 45
    {
	FF_PAT_TXT, FF_PAT, FF_PAT_TXT, "PATTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	4, {0x23,0x50,0x41,0x54}, // "#PAT"
	0,
	MinusString,
	MinusString,
	"Text version of PAT"
    },

 // FF_TPL = 46
    {
	FF_TPL, 0, 0, "TPL",
	".tpl", ".szs", ".tpl",
	FFT_VALID | FFT_GRAPHIC | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	4, {0x00,0x20,0xaf,0x30}, // "\000 ¯0"
	0,
	MinusString,
	MinusString,
	"Image container"
    },

 // FF_BTI = 47
    {
	FF_BTI, 0, 0, "BTI",
	".bti", ".szs", ".bti",
	FFT_VALID | FFT_GRAPHIC | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Image container"
    },

 // FF_BREFT_IMG = 48
    {
	FF_BREFT_IMG, 0, 0, "BT-IMG",
	".bt-img", ".szs", ".bt-img",
	FFT_VALID | FFT_GRAPHIC | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Raw image of BREFT file"
    },

 // FF_BMG = 49
    {
	FF_BMG, FF_BMG, FF_BMG_TXT, "BMG",
	".bmg", ".szs", ".bmg",
	FFT_VALID | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Message file"
    },

 // FF_BMG_TXT = 50
    {
	FF_BMG_TXT, FF_BMG, FF_BMG_TXT, "BMGTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Text version of BMG"
    },

 // FF_KCL = 51
    {
	FF_KCL, FF_KCL, FF_KCL_TXT, "KCL",
	".kcl", ".szs", ".kcl",
	FFT_VALID | FFT_CUT | FFT_DECODE | FFT_ENCODE | FFT_PATCH,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Collision file"
    },

 // FF_KCL_TXT = 52
    {
	FF_KCL_TXT, FF_KCL, FF_KCL_TXT, "KCLTXT",
	".obj", ".szs", ".obj",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	4, {0x23,0x4b,0x43,0x4c}, // "#KCL"
	0,
	MinusString,
	MinusString,
	"Wavefront OBJ by WSZST"
    },

 // FF_WAV_OBJ = 53
    {
	FF_WAV_OBJ, FF_KCL, FF_WAV_OBJ, "WAVOBJ",
	".obj", ".szs", ".obj",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Wavefront OBJ"
    },

 // FF_SKP_OBJ = 54
    {
	FF_SKP_OBJ, FF_KCL, FF_SKP_OBJ, "SKPOBJ",
	".obj", ".szs", ".obj",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Wavefront OBJ by Sketchup"
    },

 // FF_KMP = 55
    {
	FF_KMP, FF_KMP, FF_KMP_TXT, "KMP",
	".kmp", ".szs", ".kmp",
	FFT_VALID | FFT_CUT | FFT_DECODE | FFT_ENCODE,
	4, {0x52,0x4b,0x4d,0x44}, // "RKMD"
	0,
	MinusString,
	MinusString,
	"Track information file"
    },

 // FF_KMP_TXT = 56
    {
	FF_KMP_TXT, FF_KMP, FF_KMP_TXT, "KMPTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	4, {0x23,0x4b,0x4d,0x50}, // "#KMP"
	0,
	MinusString,
	MinusString,
	"Text version of KMP"
    },

 // FF_ITEMSLT = 57
    {
	FF_ITEMSLT, FF_ITEMSLT, FF_ITEMSLT_TXT, "ITEMSLT",
	".bin", ".szs", ".slt",
	FFT_VALID,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_ITEMSLT_TXT = 58
    {
	FF_ITEMSLT_TXT, FF_ITEMSLT, FF_ITEMSLT_TXT, "ITEMSLTTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_KMG = 59
    {
	FF_KMG, FF_KMG, FF_KMG_TXT, "KMG",
	".kmg", ".szs", ".kmg",
	FFT_VALID | FFT_DECODE | FFT_ENCODE,
	4, {0x52,0x4b,0x4d,0x47}, // "RKMG"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_KMG_TXT = 60
    {
	FF_KMG_TXT, FF_KMG, FF_KMG_TXT, "KMGTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	4, {0x23,0x4b,0x4d,0x47}, // "#KMG"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_KRM = 61
    {
	FF_KRM, FF_KRM, FF_KRM_TXT, "KRM",
	".krm", ".szs", ".krm",
	FFT_VALID,
	4, {0x52,0x4b,0x52,0x4d}, // "RKRM"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_KRM_TXT = 62
    {
	FF_KRM_TXT, FF_KRM, FF_KRM_TXT, "KRMTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT,
	4, {0x23,0x4b,0x52,0x4d}, // "#KRM"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_KRT = 63
    {
	FF_KRT, FF_KRT, FF_KRT_TXT, "KRT",
	".krt", ".szs", ".krt",
	FFT_VALID,
	4, {0x52,0x4b,0x47,0x54}, // "RKGT"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_KRT_TXT = 64
    {
	FF_KRT_TXT, FF_KRT, FF_KRT_TXT, "KRTTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT,
	4, {0x23,0x4b,0x52,0x54}, // "#KRT"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_OBJFLOW = 65
    {
	FF_OBJFLOW, FF_OBJFLOW, FF_OBJFLOW_TXT, "OBFLOW",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_OBJFLOW_TXT = 66
    {
	FF_OBJFLOW_TXT, FF_OBJFLOW, FF_OBJFLOW_TXT, "OF-TXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x4f,0x42,0x4a,0x46,0x4c,0x4f,0x57}, // "#OBJFLOW"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_ITEM = 67
    {
	FF_GH_ITEM, FF_GH_ITEM, FF_GH_ITEM_TXT, "GHITEM",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_ITEM_TXT = 68
    {
	FF_GH_ITEM_TXT, FF_GH_ITEM, FF_GH_ITEM_TXT, "GI-TXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x47,0x48,0x2d,0x49,0x54,0x45,0x4d}, // "#GH-ITEM"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_IOBJ = 69
    {
	FF_GH_IOBJ, FF_GH_IOBJ, FF_GH_IOBJ_TXT, "GHIOBJ",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_IOBJ_TXT = 70
    {
	FF_GH_IOBJ_TXT, FF_GH_IOBJ, FF_GH_IOBJ_TXT, "GIOTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x47,0x48,0x2d,0x49,0x4f,0x42,0x4a}, // "#GH-IOBJ"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_KART = 71
    {
	FF_GH_KART, FF_GH_KART, FF_GH_KART_TXT, "GHKART",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_KART_TXT = 72
    {
	FF_GH_KART_TXT, FF_GH_KART, FF_GH_KART_TXT, "GK-TXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x47,0x48,0x2d,0x4b,0x41,0x52,0x54}, // "#GH-KART"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_KOBJ = 73
    {
	FF_GH_KOBJ, FF_GH_KOBJ, FF_GH_KOBJ_TXT, "GHKOBJ",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_DECODE | FFT_ENCODE,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_GH_KOBJ_TXT = 74
    {
	FF_GH_KOBJ_TXT, FF_GH_KOBJ, FF_GH_KOBJ_TXT, "GKOTXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_DECODE | FFT_ENCODE | FFT_PARSER,
	8, {0x23,0x47,0x48,0x2d,0x4b,0x4f,0x42,0x4a}, // "#GH-KOBJ"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_DRIVER = 75
    {
	FF_DRIVER, 0, 0, "DRV",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_VEHICLE = 76
    {
	FF_VEHICLE, 0, 0, "VEH",
	".bin", ".szs", ".bin",
	FFT_VALID,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_BRASD = 77
    {
	FF_BRASD, 0, 0, "BRASD",
	".brasd", ".szs", ".brasd",
	FFT_VALID,
	4, {0x52,0x41,0x53,0x44}, // "RASD"
	0,
	MinusString,
	MinusString,
	filetype_info_unknown
    },

 // FF_RKG = 78
    {
	FF_RKG, 0, 0, "RKG",
	".rkg", ".szs", ".rkg",
	FFT_VALID,
	4, {0x52,0x4b,0x47,0x44}, // "RKGD"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_RKCO = 79
    {
	FF_RKCO, 0, 0, "RKCO",
	".rkco", ".szs", ".rkco",
	FFT_VALID,
	4, {0x52,0x4b,0x43,0x4f}, // "RKCO"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_STATICR = 80
    {
	FF_STATICR, 0, 0, "STATICR",
	".rel", ".szs", ".rel",
	FFT_VALID | FFT_PATCH,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"A 'StaticR.rel' file"
    },

 // FF_DOL = 81
    {
	FF_DOL, 0, 0, "DOL",
	".dol", ".szs", ".dol",
	FFT_VALID | FFT_EXTRACT | FFT_PATCH,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"An executable DOL file"
    },

 // FF_GCT = 82
    {
	FF_GCT, 0, 0, "GCT",
	".gct", ".szs", ".gct",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Gecko Cheat Code"
    },

 // FF_GCH = 83
    {
	FF_GCH, 0, 0, "GCH",
	".gch", ".szs", ".gch",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Gecko Cheat Handler + coded"
    },

 // FF_WCH = 84
    {
	FF_WCH, 0, 0, "WCH",
	".wch", ".szs", ".wch",
	FFT_VALID,
	0, {0}, // no magic
	0,
	MinusString,
	MinusString,
	"Wiimms Cheat Handler + codes"
    },

 // FF_WPF = 85
    {
	FF_WPF, 0, 0, "WPF",
	".wpf", ".szs", ".wpf",
	FFT_VALID,
	4, {0x57,0x50,0x46,0x01}, // "WPF\001"
	0,
	MinusString,
	MinusString,
	"Wiimms Patch File"
    },

 // FF_XPF = 86
    {
	FF_XPF, 0, 0, "XPF",
	".xpf", ".szs", ".xpf",
	FFT_VALID,
	4, {0x58,0x50,0x46,0x01}, // "XPF\001"
	0,
	MinusString,
	MinusString,
	"Extended Patch File"
    },

 // FF_DISTRIB = 87
    {
	FF_DISTRIB, 0, 0, "DISTRIB",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT,
	8, {0x23,0x44,0x49,0x53,0x54,0x52,0x49,0x42}, // "#DISTRIB"
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_PNG = 88
    {
	FF_PNG, 0, 0, "PNG",
	".png", ".png", ".png",
	FFT_VALID | FFT_GRAPHIC | FFT_EXTERNAL | FFT_DECODE | FFT_ENCODE,
	8, {0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a}, // "PNG\r\n\032\n"
	0,
	MinusString,
	MinusString,
	"A public image format"
    },

 // FF_PORTDB = 89
    {
	FF_PORTDB, 0, 0, "PORTDB",
	".bin", ".szs", ".bin",
	FFT_VALID | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_TXT = 90
    {
	FF_TXT, 0, 0, "TXT",
	".txt", ".szs", ".txt",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_SCRIPT = 91
    {
	FF_SCRIPT, 0, 0, "SCRIPT",
	".script", ".szs", ".script",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_JSON = 92
    {
	FF_JSON, 0, 0, "JSON",
	".json", ".szs", ".json",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_SH = 93
    {
	FF_SH, 0, 0, "SH",
	".sh", ".szs", ".sh",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_BASH = 94
    {
	FF_BASH, 0, 0, "BASH",
	".sh", ".szs", ".sh",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_PHP = 95
    {
	FF_PHP, 0, 0, "PHP",
	".php", ".szs", ".php",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_MAKEDOC = 96
    {
	FF_MAKEDOC, 0, 0, "MAKEDOC",
	".md", ".szs", ".md",
	FFT_VALID | FFT_TEXT | FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	filetype_info_unknown,
	filetype_info_unknown,
	EmptyString
    },

 // FF_DIRECTORY = 97
    {
	FF_DIRECTORY, 0, 0, "DIR",
	".d", ".d", ".d",
	FFT_EXTERNAL,
	0, {0}, // no magic
	0,
	"transparent",
	"transparent",
	"Directory of filesystem"
    },

  // FF_N
	{0}
};


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////   FileTypeTab[]   //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t cmdtab_FileType[] =
{
    // INFO: cmd->opt := ff_attrib_t

    { FF_YAZ0,		"YAZ",		"YAZ0",		    0x3 },
    { FF_YAZ1,		"YAZ1",		0,		    0x3 },
    { FF_XYZ,		"XYZ",		0,		    0x3 },
    { FF_BZ,		"BZ",		0,		    0x3 },
    { FF_BZ2,		"BZ2",		0,		    0x3 },
    { FF_U8,		"U8",		0,		 0x8705 },
    { FF_WU8,		"WU8",		0,		 0x8705 },
    { FF_RARC,		"ARC",		"RARC",		  0x605 },
    { FF_BRRES,		"BRES",		"BRRES",	  0x705 },
    { FF_BREFF,		"BREFF",	"REFF",		  0x305 },
    { FF_BREFT,		"BREFT",	"REFT",		  0x705 },
    { FF_RKC,		"RKC",		0,		  0x705 },
    { FF_PACK,		"PACK",		0,		  0x705 },
    { FF_CHR,		"CHR",		"CHR0",		  0x461 },
    { FF_CLR,		"CLR",		"CLR0",		  0x461 },
    { FF_MDL,		"MDL",		"MDL0",		 0x4c61 },
    { FF_PAT,		"PAT",		"PAT0",		 0x1c21 },
    { FF_SCN,		"SCN",		"SCN0",		  0x461 },
    { FF_SHP,		"SHP",		"SHP0",		  0x461 },
    { FF_SRT,		"SRT",		"SRT0",		  0x461 },
    { FF_TEX,		"TEX",		"TEX0",		 0x1c29 },
    { FF_TEX_CT,	"TEX",		"TEX+CT",	 0x5c29 },
    { FF_TEX_CT,	"TEX-CT",	"TEX0",		 0x5c29 },
    { FF_TEX_CT,	"TEXCT",	0,		 0x5c29 },
    { FF_CTDEF,		"CT-DEF",	"CTDEF",	 0x3811 },
    { FF_CT0_CODE,	"C0CODE",	"CT0-CODE",	    0x1 },
    { FF_CT0_CODE,	"CT0CODE",	0,		    0x1 },
    { FF_CT0_DATA,	"C0DATA",	"CT0-DATA",	    0x1 },
    { FF_CT0_DATA,	"CT0DATA",	0,		    0x1 },
    { FF_CT1_CODE,	"C1CODE",	"CT1-CODE",	    0x1 },
    { FF_CT1_CODE,	"CT1CODE",	0,		    0x1 },
    { FF_CT1_DATA,	"C1DATA",	"CT1-DATA",	 0x4c01 },
    { FF_CT1_DATA,	"CT1DATA",	"CTCODE",	 0x4c01 },
    { FF_CUP1,		"CUP1",		0,		    0x1 },
    { FF_CRS1,		"CRS1",		0,		    0x1 },
    { FF_MOD1,		"MOD1",		0,		    0x1 },
    { FF_MOD2,		"MOD2",		0,		    0x1 },
    { FF_OVR1,		"OVR1",		0,		    0x1 },
    { FF_LE_BIN,	"LE-BIN",	"LEBIN",	 0x4c01 },
    { FF_LEX,		"LEX",		0,		 0x1c01 },
    { FF_LEX_TXT,	"LEX-TXT",	"LEXTXT",	 0x3811 },
    { FF_LPAR,		"LPAR",		0,		 0x3811 },
    { FF_LEDEF,		"LE-DEF",	"LEDEF",	 0x3811 },
    { FF_LEDIS,		"LE-DIS",	"LEDIS",	 0x1811 },
    { FF_LEREF,		"LE-REF",	"LEREF",	 0x1811 },
    { FF_LESTR,		"LE-STR",	"LESTR",	 0x1811 },
    { FF_SHA1REF,	"SHA1REF",	0,		 0x1011 },
    { FF_PREFIX,	"PREFIX",	0,		 0x1811 },
    { FF_MTCAT,		"MTCAT",	0,		 0x1811 },
    { FF_MDL_TXT,	"MDL-TXT",	"MDLTXT",	   0x11 },
    { FF_PAT_TXT,	"PAT-TXT",	"PATTXT",	 0x3811 },
    { FF_TPL,		"TPL",		0,		 0x1c09 },
    { FF_BTI,		"BTI",		"BTIENV",	 0x1c09 },
    { FF_BTI,		"BTIMAT",	0,		 0x1c09 },
    { FF_BREFT_IMG,	"BREFT-IMG",	"BREFTIMG",	 0x1c09 },
    { FF_BREFT_IMG,	"BT-IMG",	"BTIMG",	 0x1c09 },
    { FF_BMG,		"BMG",		"MESGBMG1",	 0x1c01 },
    { FF_BMG_TXT,	"BMG-TXT",	"BMGTXT",	 0x1811 },
    { FF_KCL,		"KCL",		0,		 0x5c01 },
    { FF_KCL_TXT,	"KCL-TXT",	"KCLTXT",	 0x3811 },
    { FF_WAV_OBJ,	"WAV-OBJ",	"WAVOBJ",	 0x3811 },
    { FF_SKP_OBJ,	"SKP-OBJ",	"SKPOBJ",	 0x3811 },
    { FF_KMP,		"KMP",		0,		 0x1c01 },
    { FF_KMP_TXT,	"KMP-TXT",	"KMPTXT",	 0x3811 },
    { FF_ITEMSLT,	"ITEMSLT",	"SLT",		    0x1 },
    { FF_ITEMSLT_TXT,	"ITEMSLT-TXT",	"ITEMSLTTXT",	   0x11 },
    { FF_KMG,		"KMG",		0,		 0x1801 },
    { FF_KMG_TXT,	"KMG-TXT",	"KMGTXT",	 0x3811 },
    { FF_KRM,		"KRM",		0,		    0x1 },
    { FF_KRM_TXT,	"KRM-TXT",	"KRMTXT",	   0x11 },
    { FF_KRT,		"KRT",		0,		    0x1 },
    { FF_KRT_TXT,	"KRT-TXT",	"KRTTXT",	   0x11 },
    { FF_OBJFLOW,	"OBFLOW",	"OBJFLOW",	 0x1801 },
    { FF_OBJFLOW_TXT,	"OBJFLOW-TXT",	"OBJFLOWTXT",	 0x3811 },
    { FF_OBJFLOW_TXT,	"OF-TXT",	"OFTXT",	 0x3811 },
    { FF_GH_ITEM,	"GH-ITEM",	"GHITEM",	 0x1801 },
    { FF_GH_ITEM_TXT,	"GH-ITEM-TXT",	"GHITEMTXT",	 0x3811 },
    { FF_GH_ITEM_TXT,	"GI-TXT",	"GITXT",	 0x3811 },
    { FF_GH_IOBJ,	"GH-IOBJ",	"GHIOBJ",	 0x1801 },
    { FF_GH_IOBJ_TXT,	"GH-IOBJ-TXT",	"GHIOBJTXT",	 0x3811 },
    { FF_GH_IOBJ_TXT,	"GIOTXT",	0,		 0x3811 },
    { FF_GH_KART,	"GH-KART",	"GHKART",	 0x1801 },
    { FF_GH_KART_TXT,	"GH-KART-TXT",	"GHKARTTXT",	 0x3811 },
    { FF_GH_KART_TXT,	"GK-TXT",	"GKTXT",	 0x3811 },
    { FF_GH_KOBJ,	"GH-KOBJ",	"GHKOBJ",	 0x1801 },
    { FF_GH_KOBJ_TXT,	"GH-KOBJ-TXT",	"GHKOBJTXT",	 0x3811 },
    { FF_GH_KOBJ_TXT,	"GKOTXT",	0,		 0x3811 },
    { FF_DRIVER,	"DRIVER",	"DRV",		    0x1 },
    { FF_VEHICLE,	"VEH",		"VEHICLE",	    0x1 },
    { FF_BRASD,		"BRASD",	0,		    0x1 },
    { FF_RKG,		"RKG",		0,		    0x1 },
    { FF_RKCO,		"RKCO",		0,		    0x1 },
    { FF_STATICR,	"REL",		"STATICR",	 0x4001 },
    { FF_DOL,		"DOL",		0,		 0x4201 },
    { FF_GCT,		"GCT",		0,		    0x1 },
    { FF_GCH,		"GCH",		0,		    0x1 },
    { FF_WCH,		"WCH",		0,		    0x1 },
    { FF_WPF,		"WPF",		0,		    0x1 },
    { FF_XPF,		"XPF",		0,		    0x1 },
    { FF_DISTRIB,	"DISTRIB",	0,		   0x11 },
    { FF_PNG,		"PNG",		0,		 0x1889 },
    { FF_PORTDB,	"PORTDB",	0,		   0x81 },
    { FF_SCRIPT,	"SCRIPT",	0,		   0x91 },
    { FF_JSON,		"JSON",		0,		   0x91 },
    { FF_SH,		"SH",		0,		   0x91 },
    { FF_BASH,		"BASH",		"SH",		   0x91 },
    { FF_PHP,		"PHP",		0,		   0x91 },
    { FF_MAKEDOC,	"MAKEDOC",	"MD",		   0x91 },
    { FF_DIRECTORY,	"DIRECTORY",	0,		   0x80 },

    {0,0,0,0}
};

//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


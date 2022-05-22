
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

// all about BMG files:
//	https://wiki.tockdom.de/index.php?title=BMG

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_STATICR_H
#define SZS_LIB_STATICR_H 1

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    enums			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[str_mode_t]]

typedef enum str_mode_t
{
    STR_M_UNKNOWN,

    STR_M_PAL,
    STR_M_USA,
    STR_M_JAP,
    STR_M_KOR,
     STR_M__N,

    //--- zero based index support
    STR_ZBI_FIRST	= STR_M_PAL,
    STR_ZBI_N		= STR_M__N - STR_ZBI_FIRST,
}
__attribute__ ((packed)) str_mode_t;

ccp GetStrModeName ( str_mode_t mode );
extern const KeywordTab_t str_mode_keyword_tab[];

//-----------------------------------------------------------------------------
// [[str_flags_t]]

typedef enum str_flags_t
{
    STR_F_IS_DOL	= 0x01,  // 0: staticr.rel, 1: main.dol
    STR_F_IS_OPTIONAL	= 0x02,  // optional, ignore at normal operation
    STR_F_IS_SPECIAL	= 0x04,  // special, hard coded for wiimmfi.de
    STR_F_RESTORE_HTTPS	= 0x08,  // restore "https://"
}
__attribute__ ((packed)) str_flags_t;

//-----------------------------------------------------------------------------
// [[str_server_t]]

typedef enum str_server_t
{
    SERV_NONE,		// special case: only 'https'

    SERV_NASW,
    SERV_NASW_DEV,
    SERV_NASW_TEST,
    SERV_MKW_RACE,
    SERV_SAKE,
    SERV_S_SAKE,
    SERV_GS,

    SERV__N

} str_server_t;

//-----------------------------------------------------------------------------
// [[str_server_list_t]]

typedef struct str_server_list_t
{
    str_server_t	serv_type;	// type of server
    ccp			name;		// short identification name
    uint		port;		// redirection port
    uint		domain_len;	// length of domain
    ccp			domain;		// full qualified domain
}
str_server_list_t;

extern const str_server_list_t ServerList[SERV__N+1];

//-----------------------------------------------------------------------------
// [[str_server_patch_t]]

typedef struct str_server_patch_t
{
    str_mode_t		mode;		// language mode
    str_flags_t		flags;		// flags
    u8			is_http;	// 0:neither, 1:http, 2:https
    str_server_t	serv_type;	// server type
    u32			offset;		// file offset of 'https://'
    uint		url_len;	// length of complete URL
    ccp			param;		// param incl. separator
}
str_server_patch_t;

extern const str_server_patch_t ServerPatch[];

//-----------------------------------------------------------------------------
// [[str_status_t]]

typedef enum str_status_t
{
    //--- shift values

    STR_S_SHIFT_ORDER_ALT	= 0,	// alternative order detected
    STR_S_SHIFT_ORDER_MULTI	= 1,	// multiple used elements detected
    STR_S_SHIFT_ORDER_BAD	= 2,	// bad alternatives detected
    STR_S_SHIFT_ORDER_DIFF	= 3,	// different alternatives detected

    //--- concrete values

    STR_S_ANALYZED		= 0x0000001, // data analyzed
    STR_S_WRONG_SIZE		= 0x0000002, // wrong file size
    STR_S_ORIG			= 0x0000004, // original file (checksum ok)
    STR_S_UNKNOWN_PATCH		= 0x0000008, // unknown patches found

    STR_S_TRACK_ORDER		= 0x0000010, // base value for tracks
     STR_S_TRACK_ORDER_ALT	= STR_S_TRACK_ORDER << STR_S_SHIFT_ORDER_ALT,
     STR_S_TRACK_ORDER_MULTI	= STR_S_TRACK_ORDER << STR_S_SHIFT_ORDER_MULTI,
     STR_S_TRACK_ORDER_BAD	= STR_S_TRACK_ORDER << STR_S_SHIFT_ORDER_BAD,
     STR_S_TRACK_ORDER_DIFF	= STR_S_TRACK_ORDER << STR_S_SHIFT_ORDER_DIFF,

    STR_S_ARENA_ORDER		= 0x0000100, // base value for arenas
     STR_S_ARENA_ORDER_ALT	= STR_S_ARENA_ORDER << STR_S_SHIFT_ORDER_ALT,
     STR_S_ARENA_ORDER_MULTI	= STR_S_ARENA_ORDER << STR_S_SHIFT_ORDER_MULTI,
     STR_S_ARENA_ORDER_BAD	= STR_S_ARENA_ORDER << STR_S_SHIFT_ORDER_BAD,
     STR_S_ARENA_ORDER_DIFF	= STR_S_ARENA_ORDER << STR_S_SHIFT_ORDER_DIFF,

    STR_S_CTGP44		= 0x00001000, // CTGP 4.4 modification found
    STR_S_CTGP44_LIKE		= 0x00002000, // CTGP 4.4 like modification found

    STR_S_VS_REGION_KNOWN	= 0x00004000, // known versus region patch found
    STR_S_VS_REGION_KNOWN_DIFF	= 0x00008000, //   "  with different values
    STR_S_VS_REGION_UNKNOWN	= 0x00010000, // unknown versus region patch found

    STR_S_BT_REGION_KNOWN	= 0x00020000, // known battle region patch found
    STR_S_BT_REGION_KNOWN_DIFF	= 0x00040000, //   "  with different values
    STR_S_BT_REGION_UNKNOWN	= 0x00080000, // unknown battle region patch found

    STR_S_ALL_RANKS_KNOWN	= 0x00100000, // known expand-fc patch found
    STR_S_ALL_RANKS_UNKNOWN	= 0x00200000, // expand-fc patch found

    STR_S_VS			= 0x00400000, // 'vs' fields modified
    STR_S_BT			= 0x00800000, // 'bt' fields modified

    STR_S_HTTPS			= 0x01000000, // https strings patched
    STR_S_VERSUS_POINTS		= 0x02000000, // versus points table modified
    STR_S_CANNON		= 0x04000000, // at least 1 cannon patched
    STR_S_MENO			= 0x08000000, // 'Menu' -> 'MenO' patched


    //--- combined values

    STR_S_TRACK_ORDER__FAIL	= STR_S_TRACK_ORDER_MULTI
				| STR_S_TRACK_ORDER_BAD
				| STR_S_TRACK_ORDER_DIFF,

    STR_S_ARENA_ORDER__FAIL	= STR_S_ARENA_ORDER_MULTI
				| STR_S_ARENA_ORDER_BAD
				| STR_S_ARENA_ORDER_DIFF,

} str_status_t;

//-----------------------------------------------------------------------------
// [[dol_status_t]]

typedef enum dol_status_t
{
    DOL_S_ANALYZED		= 0x0001, // data analyzed
    DOL_S_KNOWN			= 0x0002, // seems to be a known main.dol
    DOL_S_ORIG			= 0x0004, // original file (checksum ok)
    DOL_S_CTCODE		= 0x0010, // CT-CODE variant found (checksum ok)
    DOL_S_WIIMMFI		= 0x0020, // wiimmfi patch found (checksum ok)

    DOL_S_HTTPS			= 0x0040, // https strings patched
    DOL_S_VBI			= 0x0080, // VBI patched
    DOL_S_SDKVER		= 0x0100, // sdk version patched
    DOL_S_2HOST			= 0x0200, // second host patched

} dol_status_t;

//-----------------------------------------------------------------------------
// [[dol_info_flags_t]]

typedef enum dol_info_flags_t
{
    DIF_MODE	    = 0x03,	// str_mode_t - 1
    DIF_ORIG	    = 0x04,	// orig image found
    DIF_CTCODE	    = 0x08,	// CT-CODE patched
}
dol_info_flags_t;

//-----------------------------------------------------------------------------
// [[dol_patch_flags_t]]

typedef enum dol_patch_flags_t
{
    DPF_WCODE,
    DPF_CHEAT,
    DPF_ALLOW_USER,
    DPF_CTCODE,
    DPF__N,
}
dol_patch_flags_t;

//-----------------------------------------------------------------------------

#define MAX_REGION_PATCH 5
#define PATCHED_BY_SIZE 42

typedef enum patched_by_t
{
    PBY_OFF,
    PBY_RESET,
    PBY_WIIMMFI,
    PBY_AUTO,
    PBY_ALWAYS,
}
patched_by_t;

extern patched_by_t patched_by_mode;
int ScanOptPBMode ( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct staticr_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[staticr_t]]

typedef struct staticr_t
{
    //--- base info

    ccp			fname;			// filename of loaded file
    FileAttrib_t	fatt;			// file attribute
    file_format_t	fform;			// file format

    //--- file status

    bool		is_staticr;		// file is a 'StaticR.rel'
    bool		is_dol;			// file is a '*.dol'
    bool		is_main;		// file is a known 'main.dol'
    str_mode_t		mode;			// data mode
    str_status_t	str_status;		// data status (only StaticR)
    dol_status_t	dol_status;		// data status (only DOL files)
    ccp			dol_info;		// NULL or pointer to a dol info
    u32			dol_info_flags;		// info flags for NAS by WCODE
    bool		dol_info_flags_set;	// true, if DOF_ORIG already set
    char		dol_flags[DPF__N+1];	// flags to patch patched_by
    bool		dol_cleaned;		// TRUE if DOL was cleaned

    //--- analyze data

    int			vs_region[MAX_REGION_PATCH];	// found vs regions, -1=orig
    int			bt_region[MAX_REGION_PATCH];	// found bt regions, -1=orig
    u32			vbi_address;			// >0: VBI entry address
    char		patched_by[PATCHED_BY_SIZE+1];	// patched by version
    char		sdk_version[9];			// patched value of 'sdkver'
    u8			cannon_data[48];		// copied cannon data

    //--- raw data

    u8			* data;			// raw data
    uint		data_size;		// size of 'data'
    bool		data_alloced;		// true: 'data' must be freed

} staticr_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    interface			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeSTR ( staticr_t * str );
void ResetSTR ( staticr_t * str );

enumError LoadSTR
(
    staticr_t		* str,		// valid pointer
    bool		initialize,	// true: initialize 'str'
    ccp			fname,		// filename of source
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
);

void AnalyzeSTR
(
    staticr_t		* str		// valid pointer
);

void PrintStatusSTR
(
    FILE		* f,		// destination file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    int			long_mode	// >0: print more track details if changed
);

uint PatchSTR
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
);

enumError DumpSTR
(
    FILE		* f,		// dump to this file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    bool		use_c		// true: print in C language fragments
);

enumError ExtractSTR
(
    staticr_t		* str,		// valid pointer
    ccp			dest_dir	// valid destination directory
);

///////////////////////////////////////////////////////////////////////////////

const u32 * GetTrackOffsetTab  ( str_mode_t mode );
const u32 * GetArenaOffsetTab  ( str_mode_t mode );

///////////////////////////////////////////////////////////////////////////////
// [[VersusPointsInfo_t]]

typedef struct VersusPointsInfo_t
{
    u8		region_mode;	// related mode, type str_mode_t
    char	region_code;	// region code: E|P|J|K
    u32		off_staticr;	// offset witing StaticR.rel
    u32		cheat_base;	// offset for the cheat code
}
VersusPointsInfo_t;

extern const VersusPointsInfo_t VersusPointsInfo[STR_M__N];
extern str_mode_t opt_cheat_region;

static inline const VersusPointsInfo_t * GetVersusPointsInfo ( str_mode_t mode )
	{ return VersusPointsInfo + ( mode < STR_M__N ? mode : STR_M_UNKNOWN ); }

uint GetVersusPointsOffset ( str_mode_t mode );
int ScanOptCheatRegion ( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    rel file format		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[rel_header_t]]

typedef struct rel_header_t
{
  /*00*/  u32	id;		// Arbitrary identification number for game use.
  /*04*/  u32	unknown_04;	// padding? ignored by MKW
  /*08*/  u32	unknown_08;	// padding? ignored by MKW
  /*0c*/  u32	n_section;	// Number of sections in the file.
  /*10*/  u32	section_off;	// Offset to the start of the section table.
  /*14*/  u32	mod_name;	// NULL or Offset to ASCII string containing name of module.
  /*18*/  u32	mod_name_len;	// Length of 'mod_name' name in bytes.
  /*1c*/  u32	version;	// Version number of the REL file format.
  /*20*/  u32	bss_size;	// Size of the 'bss' section.
  /*24*/  u32	reloc_off;	// Offset to the relocation table.
  /*28*/  u32	imp_off;	// Offset to imp(?).
  /*2c*/  u32	imp_size;	// Size of imp(?).
  /*30*/  u32	flags;		// flags of some sort? ignored by MKW
  /*34*/  u32	prolog;		// ?
  /*38*/  u32	epilog;		// ?
  /*3c*/  u32	unknown_3c;	// unresolved: ?
  /*40*/  u32	align_all;	// Version e 2: Alignment constraint on all sections.
  /*44*/  u32	align_bss;	// Version e 2: Alignment constraint on all 'bss' sections.
  /*48*/  u32	fix_size;	// Version e 3: fixSize

}
__attribute__ ((packed)) rel_header_t;


///////////////////////////////////////////////////////////////////////////////
// [[rel_sect_info_t]]

typedef struct rel_sect_info_t
{
    u32		offset;		// Location in file of the section information.
				//   The last bit determines if this section is
				//   executable or not. If offset is zero,
				//   the section is an uninitialized section.
    u32		size;		// size of of the section
}
__attribute__ ((packed)) rel_sect_info_t;

///////////////////////////////////////////////////////////////////////////////
// [[rel_imp_t]]

typedef struct rel_imp_t
{
    u32		type;		// Type of the relocation entries:
				//   1 for internal, 0 for external.
    u32		offset;		// Offset from the beginning of the REL
				//   to the relocatino data.
}
__attribute__ ((packed)) rel_imp_t;

///////////////////////////////////////////////////////////////////////////////
// [[rel_data_t]]

typedef struct rel_data_t
{
    u16		skip;		// Number of bytes to skip before this relocation.
    u8		type;		// The relocation type => enum rel_type_t
    u8		section;	// The section of the symbol to locate.
    u32		address;	// Address of the symbol in the section.
}
__attribute__ ((packed)) rel_data_t;

///////////////////////////////////////////////////////////////////////////////
// [[rel_type_t]]

typedef enum rel_type_t
{
    RELT_NONE		=   0,	// Do nothing. Skip this entry.

    RELT_ADDR32		=   1, 	// Write the 32 bit address of the symbol.

    RELT_ADDR24		=   2,	// Write the 24 bit address of the symbol divided by
				// four shifted up 2 bits to the 32 bit value
				// (for relocating branch instructions).
				// Fail if address won't fit.

    RELT_ADDR16		=   3, 	// Write the 16 bit address of the symbol.
				// Fail if address more than 16 bits.

    RELT_ADDR16_LO	=   4, 	// Write the low 16 bits of the address of the symbol.

    RELT_ADDR16_HI	=   5, 	// Write the high 16 bits of the address of the symbol.

    RELT_ADDR16_HA	=   6, 	// Write the high 16 bits of the address of the symbol
				// plus 0x8000.

    RELT_ADDR14a	=   7, 	// Write the 14 bits of the address of the symbol
    RELT_ADDR14b,		// divided by four shifted up 2 bits to the 32
    RELT_ADDR14c,		// bit value (for relocating conditional branch
				// instructions). Fail if address won't fit.



    RELT_REL24		=  10, 	// Write the 24 bit address of the symbol minus the
				// address of the relocation divided by four shifted
				// up 2 bits to the 32 bit value (for relocating relative
				// branch instructions). Fail if address won't fit.

    RELT_REL14a		=  11,	// Write the 14 bit address of the symbol minus the
    RELT_REL14b,		// address of the relocation divided by four shifted
    RELT_REL14c,		// up 2 bits to the 32 bit value (for relocating
				// conditional relative branch instructions).
				// Fail if address won't fit.

    RELT__NONE		= 200, 	// Do nothing. Skip this entry.
				// Carry the address of the symbol to the next entry.

    RELT__SECT		= 201, 	// Change which section relocations are being applied
				// to. Set the offset into the section to 0.

    RELT__STOP		= 202, 	// Stop parsing the relocation table.

}
__attribute__ ((packed)) rel_type_t;

///////////////////////////////////////////////////////////////////////////////
// [[rel_section_t]]

typedef enum rel_section_t
{
    REL_SECT_HEAD,
    REL_SECT_T1,
    REL_SECT_D2,
    REL_SECT_D3,
    REL_SECT_D4,
    REL_SECT_D5,
    REL_SECT_EXT,
    REL_SECT__N
}
rel_section_t;

extern const char rel_section_name[REL_SECT__N+1][5];

///////////////////////////////////////////////////////////////////////////////
// [[rel_info_t]]

typedef struct rel_info_t
{
    u32 offset_sect;		// offset of section data, always 0xd4
    u32 fix_size;		// OSLinkFixed (and of reserved mem)
    u32 bss_size;		// size of BSS, always 0x78b0
    u32 load_addr;		// address, at which file is loaded

    dol_sect_info_t sect[REL_SECT__N+1];
				// section info, terminated with sect.section == -1
}
rel_info_t;

//-----------------------------------------------------------------------------

const rel_info_t * GetInfoREL ( str_mode_t mode );

int GetRelOffsetByAddrM
(
    str_mode_t		mode,		// region mode
    u32			addr,		// address to search
    u32			size,		// >0: wanted size
    u32			*valid_size	// not NULL: return valid size
);

u32 GetRelAddrByOffsetM
(
    str_mode_t		mode,		// region mode
    u32			off,		// offset to search
    u32			size,		// >0: wanted size
    u32			*valid_size	// not NULL: return valid size
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			WPF: Wiimms Patch File		///////////////
///////////////////////////////////////////////////////////////////////////////

#define WPF_MAGIC_NUM 0x57504601 // "WPF\1"
#define XPF_MAGIC_NUM 0x58504601 // "XPF\1"

///////////////////////////////////////////////////////////////////////////////
// [[wpf_t]]

typedef struct wpf_t
{
    // big endian!

    u8	selector;		// object selector, bit field
				//   0: end of list
				//  0x01..0x08 staticr.rel of regions (PAL/USA/JAP/KOR)
				//  0x10..0x80 main.dol of regions (PAL/USA/JAP/KOR)

    u8	patch_type;		// type of record:
				//   0:  end of list
				//  'P': patch
				//  'E': condition, true if data is equal
				//  'N': condition, true if data is not equal

    u8	clear_cond;		//   0:  do nothing
				//  'C': clear previous condition
				//  'E': else: invert condition

    u8	addr_type;		//   0:  end of list
				//  'A': address
				//  'O': file offset

    u32 addr;			// address or file offset
    u32 size;			// size of data
    u8	data[];			// data, aligned by ALIGN(size,4)
}
__attribute__ ((packed)) wpf_t;

// Note:
//  If a conditions fails,
//  then all jobs are ignored until 'clear_cond' is "E" or "C".

///////////////////////////////////////////////////////////////////////////////
// [[wpf_head_t]]

typedef struct wpf_head_t
{
    u32		magic;		// WPF_MAGIC_NUM
    u32		version;	// patch version
    u32		size;		// total size of patch
    wpf_t	wpf[];		// wpf records
}
__attribute__ ((packed)) wpf_head_t;

///////////////////////////////////////////////////////////////////////////////

uint PatchByListWPF
(
    staticr_t		*str		// valid pointer
);

int PatchByFileWPF
(
    staticr_t		*str,		// valid pointer
    ccp			fname
);

int PatchByWPF
(
    staticr_t		*str,		// valid pointer
    const wpf_t		*wpf,		// wpf patch list
    uint		size,		// size of data beginning at 'wpf'
    ccp			fname		// filename for error messages
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			address porting			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[addr_size_flags_t]]

typedef enum addr_size_mode_t
{
    ASM_VERIFIED,	// addresses verified
    ASM_EXPANDED,	// expansion of verified addresses
    ASM_UNSURE,		// unsure (not verified)
    ASM_SUSPECT,	// suspect!

    ASM_M_MODE	= 0x00000003,
    ASM_M_SIZE	= 0xfffffffc,
}
addr_size_mode_t;

extern const ccp addr_size_flags_info[];
ccp GetAddressSizeColor ( const ColorSet_t *cs, addr_size_mode_t asmode );

///////////////////////////////////////////////////////////////////////////////
// [[addr_port_t]]

typedef struct addr_port_t
{
    u32 addr[STR_ZBI_N];    // addresses for all 4 regions (never 0)
    u32 size1;		    // first size with flags (addr_size_mode_t).
    u32 size2;		    // second size with other flags. size2 is always ≥size1.
}
__attribute__ ((packed)) addr_port_t;

//-----------------------------------------------------------------------------
// [[addr_port_version_t]]

#define ADDR_PORT_DB_VERSION	1
#define ADDR_PORT_MAGIC		"PORT-DB"
#define ADDR_PORT_MAGIC_NUM	0x504f52542d444200ull

typedef struct addr_port_version_t
{
    char	magic[8];	// ADDR_PORT_MAGIC
    u32		db_version;	// ADDR_PORT_DB_VERSION
    u32		revision;	// REVISION_NUM
    u32		timestamp;	// time(0)
    u32		n_records;	// number of records
}
__attribute__ ((packed)) addr_port_version_t;

//-----------------------------------------------------------------------------

void SetupAddrPortDB();
const addr_port_version_t * GetAddrPortVersion();
extern ccp addr_port_file;

const addr_port_t * GetPortingRecord ( str_mode_t mode, u32 addr );
const addr_port_t * GetPortingRecordPAL ( u32 addr );

///////////////////////////////////////////////////////////////////////////////
// [[addr_type_t]]

typedef enum addr_type_t
{
    ADDRT_NULL,		// address is NULL
    ADDRT_INVALID,	// invalid address

     ADDRT_MEMORY_BEGIN,

    ADDRT_DOL,		// valid address of DOL
    ADDRT_BSS		= ADDRT_DOL + DOL_N_SECTIONS,

    ADDRT_REL_SECT,	// valid address of STATICR
    ADDRT_REL		= ADDRT_REL_SECT + REL_SECT__N,

    ADDRT_HEAD,		// valid address 80000000..80004000 (mirrror +0x40000000)
    ADDRT_MEM1,		// valid address 80000000..81800000 (mirrror +0x40000000)
    ADDRT_MEM2,		// valid address 90000000..94000000 (mirrror +0x40000000)

     ADDRT_MEMORY_END,

    ADDRT_GPU,		// Hollywood (GPU and more) registers cd000000..0xcd008000

    ADDRT__N,


    //--- mirror support

    ADDRT_F_MIRROR	= 0x40,
    ADDRT_MIRROR_BEGIN	= ADDRT_MEMORY_BEGIN | ADDRT_F_MIRROR,
    ADDRT_MIRROR_END	= ADDRT_MEMORY_END   | ADDRT_F_MIRROR,

    ADDRT__NN,
}
addr_type_t;

static inline bool IsAddrTypeDOL ( addr_type_t atype )
	{ atype &= ~ADDRT_F_MIRROR; return atype >= ADDRT_DOL && atype <= ADDRT_BSS; }

static inline bool IsAddrTypeREL ( addr_type_t atype )
	{ atype &= ~ADDRT_F_MIRROR; return atype >= ADDRT_REL_SECT && atype <= ADDRT_REL; }

static inline bool IsAddrTypeRELSect ( addr_type_t atype )
	{ atype &= ~ADDRT_F_MIRROR; return atype >= ADDRT_REL_SECT && atype < ADDRT_REL; }

//-----------------------------------------------------------------------------

#define MIRRORED_ADDRESS_DELTA 0x40000000

static inline bool IsMirroredAddress ( u32 addr )
{
    return addr >= 0xc0000000 && addr < 0xc1800000
	|| addr >= 0xd0000000 && addr < 0xd4000000;
}

static inline bool IsMirroredAddressType ( addr_type_t atype )
	{ return atype >= ADDRT_MIRROR_BEGIN && atype < ADDRT_MIRROR_END; }


addr_type_t GetAddressType ( str_mode_t mode, u32 addr );
ccp GetAddressTypeName ( addr_type_t atype );
ccp GetAddressTypeColor ( const ColorSet_t *col, addr_type_t atype );

const dol_sect_info_t * GetAddressTypeSection ( str_mode_t mode, addr_type_t atype );

// if !atype: Calculate it by 'addr'
// return -1 if invalid
int GetOffsetByAddrM ( str_mode_t mode, addr_type_t atype, u32 addr );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			main.dol support		///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyzeMAIN
(
    staticr_t		* str		// valid pointer
);

void PrintStatusMAIN
(
    FILE		* f,		// destination file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    int			long_mode	// >0: print more track details if changed
);

uint PatchDOL
(
    // returns: number of patches

    staticr_t		* str		// valid pointer
);

enumError DumpDOL
(
    FILE		* f,		// dump to this file
    int			indent,		// indention
    staticr_t		* str,		// valid pointer
    bool		use_c		// true: print in C language fragments
);

enumError ExtractDOL
(
    staticr_t		* str,		// valid pointer
    ccp			dest_dir	// valid destination directory
);

//-----------------------------------------------------------------------------

const dol_header_t * GetDolHeader ( str_mode_t mode );

u32 GetDolOffsetByAddrM
(
    str_mode_t		mode,		// region mode
    u32			addr,		// address to search
    u32			size,		// >0: wanted size
    u32			*valid_size	// not NULL: return valid size
);

u32 GetDolAddrByOffsetM
(
    str_mode_t		mode,		// region mode
    u32			off,		// offset to search
    u32			size,		// >0: wanted size
    u32			*valid_size	// not NULL: return valid size
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum str_region_t
{
    //--- official regions
    SREG_JAPAN,
    SREG_AMERICA,
    SREG_EUROPE,

    //--- not clear
    SREG_AUSTRALIA,
    SREG_KOREA,
    SREG_TAIWAN,
    SREG_CHINA,

    //--- special
    SREG_NONE	 = -1,	// don't change region
    SREG_RESTORE = -2,	// restore region to nintendos default
    SREG_TEST	 = -3,	// test with regions 9001, 9002 and 9003

} str_region_t;


extern bool opt_clean_dol;	// true: remove additional DOL sections
extern bool opt_add_ctcode;	// true: ad CT-CODE to main.dol
extern int  opt_all_ranks;	// -1:off, 0:don't patch, 1:on
extern bool opt_move_d8;	// true: move CT-CODE D8 section to another place
extern bool opt_full_gch;	// true: use full- of only- 'codehandler'

extern bool opt_gct_scan_sep;	// true: scan 'F0000001 0000xxxx' separators
extern bool opt_gct_sep;	// true: search cheat code separators
extern bool opt_gct_asm_sep;	// true: use beginning of ASM blocks as separator
extern uint opt_gct_list;	// >0: print a summary / >1: be verbose
extern int  opt_gct_move;	// move GCT to heap, one of KeyTab_OFF_AUTO_ON
extern u32  opt_gct_addr;	// address for opt_gct_move
extern u32  opt_gct_space;	// minimal code space for opt_gct_move
extern int  opt_allow_user_gch;	// allow user define Gecko Code Handler

extern StringField_t opt_sect_list;	// files with section data of type GCT|GCH|WCH
extern StringField_t opt_wpf_list;	// files with patches of type WPF|XPF
extern StringField_t opt_wcode_list;	// GCT files for wcode jobs

extern int  opt_str_vs_region;	// versus region patch mode
extern bool opt_str_xvs_region;	// true: patch also test offsets
extern bool opt_str_tvs_region;	// true: enable test mode with incremented regions

extern int  opt_str_bt_region;	// battle region patch mode
extern bool opt_str_xbt_region;	// true: patch also test offsets
extern bool opt_str_tbt_region;	// true: enable test mode with incremented regions

extern bool opt_meno;		// true: patch 'Menu*' to 'MenO*'

extern dol_sect_select_t section_list[];
extern uint n_section_list;

int ScanOptRegion ( ccp arg );
int ScanOptVersusRegion ( ccp arg );
int ScanOptBattleRegion ( ccp arg );
int ScanOptAllRanks ( ccp arg );
int ScanOptGctAddr ( ccp arg );
int ScanOptGctSpace ( ccp arg );
int ScanOptCreateSect ( ccp arg );

//-----------------------------------------------------------------------------

typedef enum https_mode_t
{
	HTTPS_NONE,
	HTTPS_RESTORE,
	HTTPS_HTTP,
	HTTPS_DOMAIN,
	HTTPS_SAKE0,
	HTTPS_SAKE1,

	HTTPS__N
}
https_mode_t;

extern https_mode_t opt_https;
extern bool opt_wc24;
extern const KeywordTab_t HttpsOptions[];

int ScanOptHttps ( ccp arg );
ccp GetHttpsName ( https_mode_t mode, ccp return_if_failed );

extern char mkw_domain[];
extern ccp  wifi_domain;
extern bool wifi_domain_is_wiimmfi;
int ScanOptDomain ( ccp https, ccp arg );

typedef enum wcode_t
{
	WCODE_OFF,	// wcode support disabled
	WCODE_AUTO,	// auto mode (default), set to WCODE_OFF or WCODE_ON
	WCODE_GECKO,	// enabled, but insert cheat codes
	WCODE_ON,	// enabled as wcode job and data

	WCODE_ENABLED = WCODE_GECKO, // minimal enabled code
}
wcode_t;

extern wcode_t opt_wcode;
extern const KeywordTab_t wcode_keytab[];
int ScanOptWCode ( ccp arg );

int ScanOptPatchedBy ( ccp arg );
int ScanOptCannon ( ccp arg );
int ScanOptGctMove ( ccp arg );
int ScanOptAllowUserGch ( ccp arg );

enum VsBtMode { VSBT_OFF, VSBT_PATCH, VSBT_TEST };
extern enum VsBtMode patch_vs_mode;
extern enum VsBtMode patch_bt_mode;
extern char patch_vs_str[2];
extern char patch_bt_str[2];
extern ccp opt_port_db;
extern ccp opt_order;
extern bool opt_no_0x;
extern bool opt_upper;

int ScanOptVS ( bool is_bt, bool allow_2, ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_STATICR_H


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

///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_LIB_PAT_H
#define SZS_LIB_PAT_H 1

#include "lib-std.h"

///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define PAT_MAX_NAME_LEN	20
#define PAT_MAX_ELEM		20

#define PAT_HEAD_OFFSET		0x2c

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 pat_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum pat_mode_t
{
    //--- PAT text output

    PATMD_SIMPLE	= 0x0001,		// force simple output
    PATMD_COMPLEX	= 0x0002,		// force complex output
     PATMD_M_BOTH	 = PATMD_SIMPLE
			 | PATMD_COMPLEX,

    //--- hidden options

    PATMD_LOG		= 0x00100000,		// enable pat trace log to stdout
    PATMD_SILENT	= 0x00200000,		// enable pat trace log to stdout
     PATMD_M_HIDDEN	 = PATMD_LOG
			 | PATMD_SILENT,

    //--- test modes

    PATMD_TEST0		= 0x00000000,
    PATMD_TEST1		= 0x10000000,
    PATMD_TEST2		= 0x20000000,
    PATMD_TEST3		= 0x30000000,
     PATMD_M_TEST	 = PATMD_TEST3,		// all bits!


    //--- summaries

    PATMD_F_HIDE	= 0x40000000,		// help flag to hide modes in PrintPatMode()

    PATMD_M_DEFAULT	= 0,			// default settings

    PATMD_M_ALL		= PATMD_M_BOTH		// all relevant bits
			| PATMD_M_HIDDEN
			| PATMD_M_TEST,

    PATMD_M_PRINT	= PATMD_M_ALL		// all relevant bits for obj output
			& ~PATMD_M_HIDDEN,

} pat_mode_t;

//-----------------------------------------------------------------------------

extern pat_mode_t PAT_MODE;
//extern int have_pat_patch_count;

void LoadParametersPAT
(
    ccp		log_prefix		// not NULL:
					//    print log message with prefix
);

void SetPatMode ( uint new_mode );
int  ScanOptPat ( ccp arg );
pat_mode_t GetRelevantPatMode ( pat_mode_t mode );
uint PrintPatMode ( char *buf, uint bufsize, pat_mode_t mode );
ccp  GetPatMode();

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct pat_head_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[pat_head_t]]

typedef struct pat_head_t
{
  /* 0x00 */	u16		unknown_00;	// unknown, always 0 in MKW
  /* 0x02 */	u16		unknown_02;	// unknown, always 0 in MKW
  /* 0x04 */	u16		n_frames;	// number of frames (2..179)
  /* 0x06 */	u16		n_sect0;	// number of entries in section 0 (1..2)
  /* 0x08 */	u16		n_sect1;	// number of strings in section 1 & 3 (2..12)
  /* 0x0a */	u16		unknown_0a;	// unknown, always 0 in MKW
  /* 0x0c */	u16		unknown_0c;	// unknown, always 0 in MKW
  /* 0x0e */	u16		cyclic;		// 0:disabled, 1: enabled (0..1)
  /* 0x10 */					// ---END---

}
__attribute__ ((packed)) pat_head_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PAT section 0			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[pat_s0_belem_t]]

typedef struct pat_s0_belem_t
{
  /* 0x00 */	u16		unknown_00;	// unknown, 0x19 .. 0x96
  /* 0x02 */	u16		unknown_02;	// unknown, always 0
  /* 0x04 */	u16		unknown_04;	// unknown, 0..2, >0 only if n_sect0>0
  /* 0x06 */	u16		unknown_06;	// unknown, 1..2, >1 only if n_sect0>0
  /* 0x08 */	u32		offset_name;	// offset into name table
						// relative to 'pat_s0_bhead_t'
  /* 0x0c */	u32		offset_strref;	// offset to string reference,
						// relative to 'pat_s0_bhead_t'
  /* 0x10 */					// ---END---

}
__attribute__ ((packed)) pat_s0_belem_t;

///////////////////////////////////////////////////////////////////////////////
// [[pat_s0_bhead_t]]

typedef struct pat_s0_bhead_t
{
  /* 0x00 */	u32		size;		// total size of this object
  /* 0x04 */	u16		unknown_04;	// unknown
  /* 0x06 */	u16		n_elem;		// number of elements of 'elem'
  /* 0x08 */	u16		unknown_08;	// unknown, always 0xffff in MKW.
  /* 0x0a */	u16		unknown_0a;	// unknown, always 0 in MKW.
  /* 0x0c */	u16		n_unknown;	// mostly same as 'n_elem', sometimes smaller
  /* 0x0e */	u16		unknown_0e;	// unknown, always 0 in MKW.
  /* 0x10 */	u16		unknown_10;	// unknown, always 0 in MKW.
  /* 0x12 */	u16		unknown_12;	// unknown, always 0 in MKW.
  /* 0x14 */	u16		unknown_14;	// unknown, always 0 in MKW.
  /* 0x16 */	u16		unknown_16;	// unknown, always 0 in MKW.
  /* 0x18 */	pat_s0_belem_t	elem[0];	// 'n_elem' elements

}
__attribute__ ((packed)) pat_s0_bhead_t;

///////////////////////////////////////////////////////////////////////////////
// [[pat_s0_sref_t]]

typedef struct pat_s0_sref_t
{
  /* 0x00 */	u32		offset_name;	// offset into name table
						// relative to this structure
  /* 0x04 */	u16		unknown_04;	// unknown, always 0, maybe MSB of 'type'
  /* 0x06 */	u16		type;		// type, always 5 in MKW
  /* 0x08 */	u32		offset_strlist;	// offset to string list,
						// relative to this header
  /* 0x0c */					// ---END---
}
__attribute__ ((packed)) pat_s0_sref_t;

///////////////////////////////////////////////////////////////////////////////
// [[pat_s0_selem_t]]

typedef struct pat_s0_selem_t
{
  /* 0x00 */	float		time;		// all floats in ascending order
  /* 0x04 */	u16		index;		// string index
  /* 0x06 */	u16		unknown_06;	// unknown/padding
  /* 0x08 */					// ---END---
}
__attribute__ ((packed)) pat_s0_selem_t;

///////////////////////////////////////////////////////////////////////////////
// [[pat_s0_shead_t]]

typedef struct pat_s0_shead_t
{
  /* 0x00 */	u16		n_elem;		// number of elements of 'elem'
  /* 0x02 */	u16		unknown_02;	// unknown/padding
  /* 0x04 */	float		factor;		// := 1/MAX(pat_s0_selem_t.time)
  /* 0x08 */	pat_s0_selem_t	elem[0];	// 'n_elem' elements
}
__attribute__ ((packed)) pat_s0_shead_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct pat_analyse_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[pat_analyse_t]] : analysed PAT data, setup by IsValidPAT

typedef struct pat_analyse_t
{
    //--- base data

    const u8		*data;		// analysed data
    uint		data_size;	// size of data

    //--- statistics

    valid_t		valid;		// validity status
    bool		data_complete;	// true: data is complete and valid

    uint		n_sect0;	// number of base elements
    uint		n_sect1;	// number of string ref elements
    uint		n_s0_list[PAT_MAX_ELEM];
					// number of string list elements

    //--- data structures

    pat_head_t		*head;		// pointer to header, never NULL if valid
    pat_s0_bhead_t	*s0_base;	// pointer to string ref base, not NULL if valid
    pat_s0_sref_t	*s0_sref[PAT_MAX_ELEM];
					// pointer to string ref, not NULL if valid
    pat_s0_shead_t	*s0_shead[PAT_MAX_ELEM];
					// pointer to string list, not NULL if valid

    u8	s0_sref_order[PAT_MAX_ELEM];	// memory order of 's0_sref' elements
    u8	s0_shead_order[PAT_MAX_ELEM];	// memory order of 's0_shead' elements

    u32			*s1_list;	// NULL or pointer to section 1 list
    u32			*s3_list;	// NULL or pointer to section 3 list
}
pat_analyse_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct pat_element_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[pat_element_t]]

typedef struct pat_element_t
{
    pat_s0_belem_t	belem;		// base element
    pat_s0_sref_t	sref;		// string ref element
    pat_s0_shead_t	shead;		// string list head

    pat_s0_selem_t	*selem;		// list with string elements
    uint		selem_used;	// number of used elements of 'selem'
    uint		selem_size;	// number of alloced elements of 'selem'

    ccp			bname;		// base name, alloced
    ccp			sname;		// string ref name, alloced
}
pat_element_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct pat_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[pat_t]]

typedef struct pat_t
{
    //--- base info

    ccp			fname;			// alloced filename of loaded file
    FileAttrib_t	fatt;			// file attribute
    file_format_t	fform;			// FF_PAT | FF_PAT_TXT

    //--- data

    uint		version;		// the pat version of the source
    uint		n_sect;			// number of sections
    int			revision;		// tool revision
    ccp			name;			// NULL or alloced name of the object

 #if USE_NEW_CONTAINER_PAT
    Container_t		container;		// data container
 #else
    DataContainer_t	*old_container;		// old data container support, init with NULL
 #endif


    //--- PAT data, big endian

    pat_mode_t		outmode;		// output mode:
						// 0 | PATMD_SIMPLE | PATMD_COMPLEX

    pat_head_t		head;			// copy of pat header
    pat_s0_bhead_t	base;			// PAT base data
    pat_element_t	pelem[PAT_MAX_ELEM];	// elements
    uint		n_pelem;		// number of used 'pelem'

    u8		sref_order[PAT_MAX_ELEM];	// memory order of 'pelem->sref' elements
    u8		shead_order[PAT_MAX_ELEM];	// memory order of 'pelem->shead' elements
    u8		elem_to_shead[PAT_MAX_ELEM];	// scanning: asigning indices


    //--- string support

    ParamField_t	s1_str;			// strings of section 1 with attribs of s3
						// neither sorted nor unique

    //--- raw data, created by CreateRawPAT()

    u8			* raw_data;		// NULL or data
    uint		raw_data_size;		// size of 'raw_data'

} pat_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializePAT ( pat_t * pat );
void ResetPAT ( pat_t * pat );
const VarMap_t * SetupVarsPAT();

//-----------------------------------------------------------------------------

ccp GetStringPAT
(
    pat_t		* pat,		// PAT data structure
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid	// result if invalid
);

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerPAT_t	* cdata		// NULL or container-data
);

//-----------------------------------------------------------------------------

enumError ScanTextPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

//-----------------------------------------------------------------------------

enumError ScanPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerPAT_t	* cdata,	// NULL or container-data
    CheckMode_t		mode		// not NULL: call CheckPAT(mode)
);

//-----------------------------------------------------------------------------

struct raw_data_t;

enumError ScanRawDataPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckPAT(mode)
);

///////////////////////////////////////////////////////////////////////////////

enumError LoadPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckPAT(mode)
);

//-----------------------------------------------------------------------------

enumError SaveRawPAT
(
    pat_t		* pat,		// pointer to valid PAT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

//-----------------------------------------------------------------------------

enumError SaveTextPAT
(
    pat_t		* pat,		// pointer to valid PAT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
);

///////////////////////////////////////////////////////////////////////////////

int CheckPAT
(
    // returns number of errors

    const pat_t		* pat,		// pointer to valid PAT
    CheckMode_t		mode		// print mode
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_PAT_H

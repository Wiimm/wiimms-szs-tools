
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_DB_DOL_H
#define SZS_DB_DOL_H 1

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////   definitions   ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define N_DOL_SECTION_MAP 95

//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////   struct DolSectionMap_t   //////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DolSectionMap_t]]

typedef struct DolSectionMap_t
{
    uint	mode;		// str_mode_t
    uint	dol_stat;	// dol_status_t for complete dol files
    char	sect[4];	// section name, emtpy for non sections
    uint	size;		// size of data
    ccp		info;		// info string
    sha1_hash_t	hash;		// SHA1 hash

} DolSectionMap_t;

extern const DolSectionMap_t DolSectionMap[N_DOL_SECTION_MAP];

//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////   struct DolAddressMap_t   //////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[DolAddressMap_t]]

typedef struct DolAddressMap_t
{
	u32 entry_point;         // 800060a4 800060a4 800060a4 800060a4
	u32 bss_addr;            // 802a4080 8029fd00 802a3a00 80292080
	u32 bss_size;            //    e50fc    e50fc    e50fc    e511c
	u32 shared_bss_addr;     // 802a4080 802a4080 802a4080 802a4080
	u32 shared_bss_size;     //    ceb80    ceb80    ceb80    ceb80
	u32 f_setup_register;    // 80006210 80006210 80006210 80006210
}
DolAddressMap_t;

extern const DolAddressMap_t DolAddressMapPAL;
extern const DolAddressMap_t DolAddressMapUSA;
extern const DolAddressMap_t DolAddressMapJAP;
extern const DolAddressMap_t DolAddressMapKOR;

const DolAddressMap_t * GetDolAddressMap ( uint mode );

//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_DB_DOL_H

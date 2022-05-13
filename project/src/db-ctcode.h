
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
 *                +----------------------------------------+               *
 *                |  This file is created automatically.   |               *
 *                |  Edits are lost with the next update!  |               *
 *                +----------------------------------------+               *
 *                                                                         *
 ***************************************************************************/

#ifndef SZS_DB_CTCODE_H
#define SZS_DB_CTCODE_H 1

#include "dclib/dclib-types.h"

///////////////////////////////////////////////////////////////////////////////

extern const u8 ctcode_boot_code_pal_bz2[439]; // 684 bytes decompressed
extern const u8 ctcode_boot_code_usa_bz2[438]; // 684 bytes decompressed
extern const u8 ctcode_boot_code_jap_bz2[450]; // 684 bytes decompressed

extern const u8 ctcode_boot_data_pal_bz2[238]; // 304 bytes decompressed
extern const u8 ctcode_boot_data_usa_bz2[254]; // 304 bytes decompressed
extern const u8 ctcode_boot_data_jap_bz2[245]; // 304 bytes decompressed

extern const u8 ctcode_bad1code_pal_bz2[999]; // 1824 bytes decompressed
extern const u8 ctcode_bad1code_usa_bz2[1001]; // 1824 bytes decompressed
extern const u8 ctcode_bad1code_jap_bz2[1001]; // 1824 bytes decompressed

extern const u8 ctcode_mod1_pal_bz2[2187]; // 3324 bytes decompressed
extern const u8 ctcode_mod1_usa_bz2[2180]; // 3324 bytes decompressed
extern const u8 ctcode_mod1_jap_bz2[2176]; // 3324 bytes decompressed

extern const u8 ctcode_mod2_pal_bz2[1517]; // 3128 bytes decompressed
extern const u8 ctcode_mod2_usa_bz2[1531]; // 3128 bytes decompressed
extern const u8 ctcode_mod2_jap_bz2[1538]; // 3128 bytes decompressed

extern const u8 ctcode_ovr1_pal_bz2[7364]; // 21348 bytes decompressed
extern const u8 ctcode_ovr1_usa_bz2[7335]; // 21348 bytes decompressed
extern const u8 ctcode_ovr1_jap_bz2[7364]; // 21348 bytes decompressed

///////////////////////////////////////////////////////////////////////////////

#endif // !SZS_DB_CTCODE_H


/***************************************************************************
 *                                                                         *
 *                     _____     ____                                      *
 *                    |  __ \   / __ \   _     _ _____                     *
 *                    | |  \ \ / /  \_\ | |   | |  _  \                    *
 *                    | |   \ \| |      | |   | | |_| |                    *
 *                    | |   | || |      | |   | |  ___/                    *
 *                    | |   / /| |   __ | |   | |  _  \                    *
 *                    | |__/ / \ \__/ / | |___| | |_| |                    *
 *                    |_____/   \____/  |_____|_|_____/                    *
 *                                                                         *
 *                       Wiimms source code library                        *
 *                       Support for Mario Kart Wii                        *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *        Copyright (c) 2012-2023 by Dirk Clemens <wiimm@wiimm.de>         *
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

#define _GNU_SOURCE 1
#include <string.h>
#include "lib-mkw.h"
#include "dclib-numeric.h"

#include "lib-mkw-def.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		assigned points: tables			///////////////
///////////////////////////////////////////////////////////////////////////////
// working matrix, initialized with NINTENDO

// [[24P--]]
u8 MkwPointsTab[12*12] = // active table
{
	15,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 7,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 8, 2,    0, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 9, 4, 1,    0, 0, 0, 0, 0, 0, 0, 0,
	15, 9, 5, 2, 1,    0, 0, 0, 0, 0, 0, 0,
	15,10, 6, 3, 1, 0,    0, 0, 0, 0, 0, 0,
	15,10, 7, 5, 3, 1, 0,    0, 0, 0, 0, 0,
	15,11, 8, 6, 4, 2, 1, 0,    0, 0, 0, 0,
	15,11, 8, 6, 4, 3, 2, 1, 0,    0, 0, 0,
	15,12,10, 8, 6, 4, 3, 2, 1, 0,    0, 0,
	15,12,10, 8, 6, 5, 4, 3, 2, 1, 0,    0,
	15,12,10, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// NINTENDO => Orginal Nintendo

// [[24P--]]
const u8 MkwPointsTab_NINTENDO[12*12] =
{
	15,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 7,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 8, 2,    0, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 9, 4, 1,    0, 0, 0, 0, 0, 0, 0, 0,
	15, 9, 5, 2, 1,    0, 0, 0, 0, 0, 0, 0,
	15,10, 6, 3, 1, 0,    0, 0, 0, 0, 0, 0,
	15,10, 7, 5, 3, 1, 0,    0, 0, 0, 0, 0,
	15,11, 8, 6, 4, 2, 1, 0,    0, 0, 0, 0,
	15,11, 8, 6, 4, 3, 2, 1, 0,    0, 0, 0,
	15,12,10, 8, 6, 4, 3, 2, 1, 0,    0, 0,
	15,12,10, 8, 6, 5, 4, 3, 2, 1, 0,    0,
	15,12,10, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// LINEAR => Absolute linear schema
//  - LAST := 0, INCREMENT := 1

// [[24P--]]
const u8 MkwPointsTab_LINEAR[12*12] =
{
	 0,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 1,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 2,  1,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	 3,  2,  1,  0,      0,  0,  0,  0,  0,  0,  0,  0,
	 4,  3,  2,  1,  0,      0,  0,  0,  0,  0,  0,  0,
	 5,  4,  3,  2,  1,  0,      0,  0,  0,  0,  0,  0,
	 6,  5,  4,  3,  2,  1,  0,      0,  0,  0,  0,  0,
	 7,  6,  5,  4,  3,  2,  1,  0,      0,  0,  0,  0,
	 8,  7,  6,  5,  4,  3,  2,  1,  0,      0,  0,  0,
	 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,      0,  0,
	10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,      0,
	11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
};

///////////////////////////////////////////////////////////////////////////////
// LINEAR_1 => Like LINEAR,  but LAST := 1
//  - LAST := 1,  INCREMENT := 1

// [[24P--]]
const u8 MkwPointsTab_LINEAR_1[12*12] =
{
	 1,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 2,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 3,  2,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  3,  2,  1,      0,  0,  0,  0,  0,  0,  0,  0,
	 5,  4,  3,  2,  1,      0,  0,  0,  0,  0,  0,  0,
	 6,  5,  4,  3,  2,  1,      0,  0,  0,  0,  0,  0,
	 7,  6,  5,  4,  3,  2,  1,      0,  0,  0,  0,  0,
	 8,  7,  6,  5,  4,  3,  2,  1,      0,  0,  0,  0,
	 9,  8,  7,  6,  5,  4,  3,  2,  1,      0,  0,  0,
	10,  9,  8,  7,  6,  5,  4,  3,  2,  1,      0,  0,
	11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,      0,
	12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1
};

///////////////////////////////////////////////////////////////////////////////
// LINEAR_B => Like LINEAR,  but with a bonus of 1 for a victory
//  - LAST := 0,  INCREMENT := 1,  BONUS := 1

// [[24P--]]
const u8 MkwPointsTab_LINEAR_B[12*12] =
{
	 1,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 2,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 3,  1,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  2,  1,  0,      0,  0,  0,  0,  0,  0,  0,  0,
	 5,  3,  2,  1,  0,      0,  0,  0,  0,  0,  0,  0,
	 6,  4,  3,  2,  1,  0,      0,  0,  0,  0,  0,  0,
	 7,  5,  4,  3,  2,  1,  0,      0,  0,  0,  0,  0,
	 8,  6,  5,  4,  3,  2,  1,  0,      0,  0,  0,  0,
	 9,  7,  6,  5,  4,  3,  2,  1,  0,      0,  0,  0,
	10,  8,  7,  6,  5,  4,  3,  2,  1,  0,      0,  0,
	11,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,      0,
	12, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
};

///////////////////////////////////////////////////////////////////////////////
// LINEAR_B1 => Like LINEAR_B,  but LAST := 1
//  - LAST := 1,  INCREMENT := 1,  BONUS := 1

// [[24P--]]
const u8 MkwPointsTab_LINEAR_B1[12*12] =
{
	 2,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 3,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  2,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	 5,  3,  2,  1,      0,  0,  0,  0,  0,  0,  0,  0,
	 6,  4,  3,  2,  1,      0,  0,  0,  0,  0,  0,  0,
	 7,  5,  4,  3,  2,  1,      0,  0,  0,  0,  0,  0,
	 8,  6,  5,  4,  3,  2,  1,      0,  0,  0,  0,  0,
	 9,  7,  6,  5,  4,  3,  2,  1,      0,  0,  0,  0,
	10,  8,  7,  6,  5,  4,  3,  2,  1,      0,  0,  0,
	11,  9,  8,  7,  6,  5,  4,  3,  2,  1,      0,  0,
	12, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,      0,
	13, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Test

#if MKW_POINTS_TEST_ENABLED
 // [[24P--]]
 const u8 MkwPointsTab_TEST[12*12] =
 {
	25,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 12,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 16,  8,  0,      0,  0,  0,  0,  0,  0,  0,  0,
	25, 18, 12,  6,  0,      0,  0,  0,  0,  0,  0,  0,
	25, 20, 15, 10,  5,  0,      0,  0,  0,  0,  0,  0,
	25, 20, 16, 12,  8,  4,  0,      0,  0,  0,  0,  0,
	25, 21, 17, 13,  9,  6,  3,  0,      0,  0,  0,  0,
	25, 21, 18, 15, 12,  9,  6,  3,  0,      0,  0,  0,
	25, 22, 19, 16, 13, 10,  7,  4,  2,  0,      0,  0,
	25, 22, 19, 16, 13, 10,  8,  6,  4,  2,  0,      0,
	25, 22, 19, 16, 14, 12, 10,  8,  6,  4,  2,  0,
 };
#endif // MKW_POINTS_TEST_ENABLED

///////////////////////////////////////////////////////////////////////////////
// Test/one

#if MKW_POINTS_TEST_ENABLED
 // [[24P--]]
 const u8 MkwPointsTab_TEST_1[12*12] =
 {
	25,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 13,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 17,  9,  1,      0,  0,  0,  0,  0,  0,  0,  0,
	25, 19, 13,  7,  1,      0,  0,  0,  0,  0,  0,  0,
	25, 20, 15, 10,  5,  1,      0,  0,  0,  0,  0,  0,
	25, 21, 17, 13,  9,  5,  1,      0,  0,  0,  0,  0,
	25, 21, 17, 13, 10,  7,  4,  1,      0,  0,  0,  0,
	25, 22, 19, 16, 13, 10,  7,  4,  1,      0,  0,  0,
	25, 22, 19, 16, 13, 10,  7,  5,  3,  1,      0,  0,
	25, 22, 19, 16, 13, 11,  9,  7,  5,  3,  1,      0,
	25, 22, 19, 17, 15, 13, 11,  9,  7,  5,  3,  1,
 };
#endif // MKW_POINTS_TEST_ENABLED

///////////////////////////////////////////////////////////////////////////////
// Test/bonus

#if MKW_POINTS_TEST_ENABLED
 // [[24P--]]
 const u8 MkwPointsTab_TEST_B[12*12] =
 {
	25,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 12,  0,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 16,  8,  0,      0,  0,  0,  0,  0,  0,  0,  0,
	25, 18, 12,  6,  0,      0,  0,  0,  0,  0,  0,  0,
	25, 19, 14,  9,  4,  0,      0,  0,  0,  0,  0,  0,
	25, 20, 16, 12,  8,  4,  0,      0,  0,  0,  0,  0,
	25, 20, 16, 12,  9,  6,  3,  0,      0,  0,  0,  0,
	25, 21, 18, 15, 12,  9,  6,  3,  0,      0,  0,  0,
	25, 21, 18, 15, 12,  9,  6,  4,  2,  0,      0,  0,
	25, 21, 18, 15, 12, 10,  8,  6,  4,  2,  0,      0,
	25, 21, 18, 16, 14, 12, 10,  8,  6,  4,  2,  0,
 };
#endif // MKW_POINTS_TEST_ENABLED

///////////////////////////////////////////////////////////////////////////////
// Test/bonus/one

#if MKW_POINTS_TEST_ENABLED
 // [[24P--]]
 const u8 MkwPointsTab_TEST_B1[12*12] =
 {
	25,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 12,  1,      0,  0,  0,  0,  0,  0,  0,  0,  0,
	25, 16,  8,  1,      0,  0,  0,  0,  0,  0,  0,  0,
	25, 18, 12,  6,  1,      0,  0,  0,  0,  0,  0,  0,
	25, 19, 14,  9,  5,  1,      0,  0,  0,  0,  0,  0,
	25, 20, 16, 12,  8,  4,  1,      0,  0,  0,  0,  0,
	25, 20, 16, 13, 10,  7,  4,  1,      0,  0,  0,  0,
	25, 21, 18, 15, 12,  9,  6,  3,  1,      0,  0,  0,
	25, 21, 18, 15, 12,  9,  7,  5,  3,  1,      0,  0,
	25, 21, 18, 15, 13, 11,  9,  7,  5,  3,  1,      0,
	25, 21, 19, 17, 15, 13, 11,  9,  7,  5,  3,  1,
 };
#endif // MKW_POINTS_TEST_ENABLED

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Generic tables => More belanced than NINTENDO
// * FIRST := X, LAST := 0|1
// * linear in the middle with larger gaps at the beginning

// [[24P--]]
static u8 MkwPointsTab_WIN[12*12];
static u8 MkwPointsTab_WIN_1[12*12];
static u8 MkwPointsTab_WIN_B[12*12];
static u8 MkwPointsTab_WIN_B1[12*12];

// [[24P--]]
static char MkwPointsInfo_WIN[20];
static char MkwPointsInfo_WIN_1[20];
static char MkwPointsInfo_WIN_B[20];
static char MkwPointsInfo_WIN_B1[20];

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[24P--]]
const MkwPointInfo_t MkwPointInfo[MPI__N+1] =
{
  { MPI_UNKNOWN,	3, 0,				"self defined",
	{ MPI_UNKNOWN, MPI_UNKNOWN, MPI_UNKNOWN, MPI_UNKNOWN }},

  { MPI_NINTENDO,	0, MkwPointsTab_NINTENDO,	"Nintendo",
	{ MPI_NINTENDO, MPI_NINTENDO, MPI_NINTENDO, MPI_NINTENDO }},

  { MPI_LINEAR,		1, MkwPointsTab_LINEAR,		"Linear",
	{ MPI_LINEAR, MPI_LINEAR_1, MPI_LINEAR_B, MPI_LINEAR_B1 }},
  { MPI_LINEAR_1,	1, MkwPointsTab_LINEAR_1,	"Linear, One",
	{ MPI_LINEAR, MPI_LINEAR_1, MPI_LINEAR_B, MPI_LINEAR_B1 }},
  { MPI_LINEAR_B,	1, MkwPointsTab_LINEAR_B,	"Linear, Bonus",
	{ MPI_LINEAR, MPI_LINEAR_1, MPI_LINEAR_B, MPI_LINEAR_B1 }},
  { MPI_LINEAR_B1,	1, MkwPointsTab_LINEAR_B1,	"Linear, Bonus, One",
	{ MPI_LINEAR, MPI_LINEAR_1, MPI_LINEAR_B, MPI_LINEAR_B1 }},

  { MPI_WIN,		1, MkwPointsTab_WIN,		MkwPointsInfo_WIN,
	{ MPI_WIN, MPI_WIN_1, MPI_WIN_B, MPI_WIN_B1 }},
  { MPI_WIN_1,		1, MkwPointsTab_WIN_1,		MkwPointsInfo_WIN_1,
	{ MPI_WIN, MPI_WIN_1, MPI_WIN_B, MPI_WIN_B1 }},
  { MPI_WIN_B,		1, MkwPointsTab_WIN_B,		MkwPointsInfo_WIN_B,
	{ MPI_WIN, MPI_WIN_1, MPI_WIN_B, MPI_WIN_B1 }},
  { MPI_WIN_B1,		1, MkwPointsTab_WIN_B1,		MkwPointsInfo_WIN_B1,
	{ MPI_WIN, MPI_WIN_1, MPI_WIN_B, MPI_WIN_B1 }},

#if MKW_POINTS_TEST_ENABLED
  { MPI_TEST,		2, MkwPointsTab_TEST,		"Test",
	{ MPI_TEST, MPI_TEST_1, MPI_TEST_B, MPI_TEST_B1 }},
  { MPI_TEST_1,		2, MkwPointsTab_TEST_1,		"Test, One",
	{ MPI_TEST, MPI_TEST_1, MPI_TEST_B, MPI_TEST_B1 }},
  { MPI_TEST_B,		2, MkwPointsTab_TEST_B,		"Test, Bonus",
	{ MPI_TEST, MPI_TEST_1, MPI_TEST_B, MPI_TEST_B1 }},
  { MPI_TEST_B1,	2, MkwPointsTab_TEST_B1,	"Test, Bonus, One",
	{ MPI_TEST, MPI_TEST_1, MPI_TEST_B, MPI_TEST_B1 }},
#endif

  { MPI__N,0,0 }
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////		assigned points: create generic		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

static inline void CalcSequence ( u8 *row, int np, int ni, int last_val )
{
    DASSERT(row);
    DASSERT( np >=0 && np < 12 );
    DASSERT( ni >= 0 );
    DASSERT( last_val >= 0 && last_val < 0x100 );

    int n_2do = np + 1 - ni;
    if ( n_2do && ni )
    {
	int val = row[ni-1];
	int diff = last_val - val;
	int delta = diff / n_2do;
	int bonus_count = diff - n_2do * delta, bonus_value = 1;
	if ( bonus_count < 0 )
	{
	    bonus_count = -bonus_count;
	    bonus_value = -1;
	}
	PRINT("CLOSE, 2do=%d, last=%u, delta=%d, bonus=%d+%d\n",
		n_2do,last_val,delta,bonus_count,bonus_value);

	while ( n_2do-- > 0 )
	{
	    val += delta;
	    if ( bonus_count-- > 0 )
		val += bonus_value;
	    row[ni++] = val;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

static void CreateGenericTable
(
    u8		*tab,		// victory points table, size= 12*12
    uint	win_pts,	// points of the winner
    uint	opt_bonus,	// bonus for the first player
    uint	opt_last	// points of the last player
)
{
    DASSERT(tab);
    memset(tab,0,12*12);

    if ( opt_bonus > win_pts )
	opt_bonus = win_pts;
    win_pts -= opt_bonus;
    if ( opt_last > win_pts )
	opt_last = win_pts;

    uint i;
    for ( i = 0; i < 12; i++ )
    {
	tab[0] = win_pts;
	if (i)
	    CalcSequence(tab,i,1,opt_last);
	tab[0] += opt_bonus;
	tab += 12;
    }
}


///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

static void CreateGenericWinTables ( uint win_pts )
{
    if ( win_pts > 255 )
	win_pts = 255;

    snprintf( MkwPointsInfo_WIN, sizeof(MkwPointsInfo_WIN),
		"Win %u", win_pts );
    snprintf( MkwPointsInfo_WIN_1, sizeof(MkwPointsInfo_WIN_1),
		"Win %u+One", win_pts );
    snprintf( MkwPointsInfo_WIN_B, sizeof(MkwPointsInfo_WIN_B),
		"Win %u+Bonus", win_pts );
    snprintf( MkwPointsInfo_WIN_B1, sizeof(MkwPointsInfo_WIN_B1),
		"Win %u, Bonus, One", win_pts );

    CreateGenericTable( MkwPointsTab_WIN,    win_pts, 0, 0 );
    CreateGenericTable( MkwPointsTab_WIN_1,  win_pts, 0, 1 );
    CreateGenericTable( MkwPointsTab_WIN_B,  win_pts, 1, 0 );
    CreateGenericTable( MkwPointsTab_WIN_B1, win_pts, 1, 1 );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		assigned points: ScanOptMkwPoints()	///////////////
///////////////////////////////////////////////////////////////////////////////

static ccp SkipSep ( ccp arg )
{
    while ( *arg && ( *arg <= ' ' || *arg == ',' || *arg == '+' ))
	arg++;
    return arg;
}

///////////////////////////////////////////////////////////////////////////////

bool opt_points_used = false;

// [[24P--]]
int ScanOptMkwPoints ( ccp arg, bool silent )
{
 #if DEBUG
    {
	uint i;
	for ( i = 0; i < MPI__N; i++ )
	    ASSERT_MSG( MkwPointInfo[i].id == i,
		"MkwPointInfo[%d].id = %d\n",i,MkwPointInfo[i].id);
    }
 #endif

    enum
    {
	MODE_MPI,
	MODE_WIN,
	MODE_ONE,
	MODE_BONUS,
	MODE_N,
    };

    static const KeywordTab_t key_tab[] =
    {
	{ MODE_MPI,		"NINTENDO",	"N",		MPI_NINTENDO },
	{ MODE_MPI,		"LINEAR",	"L",		MPI_LINEAR },
     #if MKW_POINTS_TEST_ENABLED
	{ MODE_MPI,		"TEST",		0,		MPI_TEST },
     #endif

	{ MODE_WIN,		"WIN",		"W",		  0 },
	{ MODE_WIN,		"WIN15",	"W15",		 15 },
	{ MODE_WIN,		"WIN25",	"W25",		 25 },
	{ MODE_WIN,		"WIIMM",	"WIIMM25",	-25 },

	{ MODE_ONE,		"ONE",		"O",		1 },
	{ MODE_ONE,		"NULL",		0,		0 },
	{ MODE_ONE,		"NO-ONE",	"NOONE",	0 },

	{ MODE_BONUS,		"BONUS",	"B",		1 },
	{ MODE_BONUS,		"NO-BONUS",	"NOBONUS",	0 },

	{ MODE_N,		"N1",		0,	/*row=*/ 0 },
	{ MODE_N,		"N2",		0,		 1 },
	{ MODE_N,		"N3",		0,		 2 },
	{ MODE_N,		"N4",		0,		 3 },
	{ MODE_N,		"N5",		0,		 4 },
	{ MODE_N,		"N6",		0,		 5 },
	{ MODE_N,		"N7",		0,		 6 },
	{ MODE_N,		"N8",		0,		 7 },
	{ MODE_N,		"N9",		0,		 8 },
	{ MODE_N,		"N10",		0,		 9 },
	{ MODE_N,		"N11",		0,		10 },
	{ MODE_N,		"N12",		0,		11 },

	{0,0,0,0}
    };

    if ( !arg || !*arg )
    {
	memcpy(MkwPointsTab,MkwPointsTab_NINTENDO,sizeof(MkwPointsTab));
	return 0;
    }

    u8 tab[12*12], row_defined[12];
    memset(tab,0,sizeof(tab));
    memset(row_defined,0,sizeof(row_defined));

    int next_np = 10, np = 11, ni = 0;
    u8 *row = tab + 12*np;

    MkwPointID opt_mpi	= MPI_UNKNOWN;
    uint opt_bonus	= 0;	// >0: add 1 point for victory
    uint opt_one	= 0;	// wanted points for last position

    int last_val	= -1;	// >=0: points of last player
    int max_val		= -1;

    uint close_sequence = 0;
    for(;;)
    {
	arg = SkipSep(arg);
	if ( !*arg || *arg == '/' )
	    close_sequence++;

	if ( close_sequence )
	{
	    if (ni)
	    {
		if ( last_val < 0 )
		    last_val = opt_one;
		CalcSequence(row,np,ni,last_val);
		row_defined[np] = 1;
	    }

	    close_sequence = 0;
	    last_val = -1;
	    ni = 0;
	    np = next_np--;
	    row = tab + 12*np;
	}

	if (!*arg)
	    break;
	if ( *arg == '/' )
	{
	    arg++;
	    continue;
	}

	//--- scan number

	u32 num;
	ccp next = ScanU32(&num,arg,10);
	if ( next > arg && !isalpha((int)*next) )
	{
	    if ( num >= 0x100 )
	    {
	     num_err:
		if (!silent)
		    ERROR0(ERR_SYNTAX,
			"Option --point: Missing number less or equal 255: %s\n",arg);
		return 1;
	    }

	    if ( max_val < (int)num )
		 max_val = num;
	    noPRINT("NUMBER: %u/%d |%s|\n",num,max_val,next);
	    if ( ni > np )
	    {
		if (!silent)
		    ERROR0(ERR_SYNTAX,
			"Option --point: Only %u numbers allowed for %u players: %s\n",
			np+1, np+1, arg );
		return 1;
	    }
	    row[ni++] = num;
	    arg = next;
	    while ( *arg > 0 && *arg <= ' ' )
		arg++;

	    if ( arg[0] == '.' && arg[1] == '.' )
	    {
		next = ScanU32(&num,arg+2,10);
		if ( next == arg || num >= 0x100 || isalpha((int)*next) )
		    goto num_err;
		if ( max_val < (int)num )
		     max_val = num;
		noPRINT("..NUMBER: %3u |%s|\n",num,next);
		arg = next;
		last_val = num;
		continue;
	    }
	    continue;
	}


	//--- scan keyword

	char key_buf[KEYWORD_NAME_MAX];
	char *dest = key_buf;
	while ( ( isalnum((int)*arg) || *arg == '-' )
		&& dest < key_buf + sizeof(key_buf) -1 )
	    *dest++ = *arg++;
	*dest = 0;
	if ( dest == key_buf )
	{
	    if (!silent)
		ERROR0(ERR_SYNTAX,"Option --point: Syntax error: %s\n",arg);
	    return 1;
	}

	int abbrev_count;
	const KeywordTab_t *key = ScanKeyword(&abbrev_count,key_buf,key_tab);
	if (!key)
	{
	    if (!silent)
		PrintKeywordError(key_tab,key_buf,abbrev_count,
					0,"keyword for --point");
	    return 1;
	}

	switch(key->id)
	{
	    case MODE_MPI:
		opt_mpi = key->opt;
		break;

	    case MODE_WIN:
		if ( key->opt < 0 )
		{
		    // WIIMM*
		    opt_one = 1;
		    opt_bonus = 1;
		    num = -key->opt;
		}
		else if (!key->opt)
		{
		    arg = SkipSep(arg);
		    next = ScanU32(&num,arg,10);
		    if ( next == arg || num >= 0x100 || isalpha((int)*next) )
			goto num_err;
		    arg = next;
		}
		else
		    num = key->opt;

		CreateGenericWinTables(num);
		opt_mpi = MPI_WIN;
		break;

	    case MODE_ONE:
		opt_one = key->opt;
		break;

	    case MODE_BONUS:
		opt_bonus = key->opt;
		break;

	    case MODE_N:
		next_np = key->opt;
		close_sequence++;
		break;

	    default:
		printf("%3llu %3llu : %s, %s\n",key->id,key->opt,key->name1,key->name2);
	}
    }
    opt_points_used = true;

    const int alt_index = ( opt_one ? MPI_I_1 : 0 ) + ( opt_bonus ? MPI_I_B : 0 );
    opt_mpi = MkwPointInfo[opt_mpi].alt[alt_index];
    const u8 *src_tab = MkwPointInfo[opt_mpi].table;
    if (!src_tab)
	src_tab = MkwPointsTab_NINTENDO;

    if ( max_val < 0 )
    {
	memcpy(MkwPointsTab,src_tab,sizeof(MkwPointsTab));
	return 0;
    }

    uint max_src = 0;
    for ( ni = 0; ni < 12*12; ni++ )
	if ( max_src < src_tab[ni] )
	     max_src = src_tab[ni];

    double factor = (double)max_val / (double)max_src;
    for ( np = 0; np < 12; np++ )
    {
	if (row_defined[np])
	    continue;

	const u8 *p1 = src_tab + 12*np;
	u8 *p2 = tab + 12*np;
	for ( ni = 0; ni < 12; ni++ )
	    *p2++ = double2int( *p1++ * factor );
    }
    memcpy(MkwPointsTab,tab,sizeof(MkwPointsTab));
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		assigned points: helpers		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

const MkwPointInfo_t * GetMkwPointInfo ( const u8 * data )
{
    CreateGenericWinTables(data[11*12]);

    const MkwPointInfo_t *ptr;
    for ( ptr = MkwPointInfo; ptr->info; ptr++ )
	if ( ptr->table && !memcmp(ptr->table,data,12*12) )
	    return ptr;

    return MkwPointInfo + MPI_UNKNOWN;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		assigned points: printing		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

void PrintMkwPointsRAW ( FILE *f, uint indent, const u8 * data )
{
    DASSERT(f);
    DASSERT(data);

    uint i, np, max = 0;
    for ( i = 0; i < 12*12; i++ )
	if ( max < data[i] )
	     max = data[i];
    const uint fw = max < 100 ? 2 : 3;

    for ( np = 1; np <= 12; np++ )
    {
	fprintf(f,"%*u:",indent+2,np);
	for ( i = 0; i < np; i++, data++ )
	    fprintf(f," %*u ",fw,*data);
	for ( ; i < 12; i++, data++ )
	    if (*data )
		fprintf(f," %*u!",fw,*data);
	    else
		fprintf(f," %*c ",fw,'-');
	fputc('\n',f);
    }
}

///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

void PrintMkwPointsC ( FILE *f, uint indent, const u8 * data )
{
    DASSERT(f);
    DASSERT(data);

    uint i, np, max = 0;
    for ( i = 0; i < 12*12; i++ )
	if ( max < data[i] )
	     max = data[i];
    const uint fw = max < 100 ? 2 : 3;

    for ( np = 1; np <= 12; np++ )
    {
	if (indent > 0 )
	    fprintf(f,"%*s",indent-1,"");
	for ( i = 0; i < 12; i++, data++ )
	{
	    if ( i == np )
		fprintf(f," %*c ",fw,' ');
	    fprintf(f," %*u,",fw,*data);
	}
	fputc('\n',f);
    }
}

///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

void PrintMkwPointsCheat
(
    FILE	*f,		// output file
    const u8	*data,		// pointer to data table
    char	region_code,	// region code: E|P|J|K
    u32		cheat_base	// value of first code
)
{
    DASSERT(f);
    DASSERT(data);

    fprintf(f,
	"RMC%c01\nMario Kart Wii\n\nVersus Points Modifier\n%08x 00000090\n",
	region_code, cheat_base );

    uint line;
    for ( line = 0; line < 12*12/8; line++ )
    {
	fprintf(f,"%02x%02x%02x%02x %02x%02x%02x%02x\n",
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7] );
	data += 8;
    }
}

///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

uint PrintMkwPointsLIST
(
    // returns the used 'mode'

    char	*buf,		// valid output buffer
    uint	buf_size,	// size of 'buf', MKW_POINTS_DEF_SIZE recommended
    const u8	*data,		// 12*12 data table
    uint	mode		// 0: pure number list
				// 1: allow 'a..b'
				// 2: allow names of permanent tables
				// 3: allow names
)
{
    DASSERT(buf);
    DASSERT(buf_size>2);
    DASSERT(data);

    if ( mode >= 2 )
    {
	const MkwPointInfo_t *mpi = GetMkwPointInfo(data);
	if ( mpi->mode <= 1 || mpi->mode == 2 && mode >= 3 )
	{
	    StringCopyS(buf,buf_size,mpi->info);
	    return mpi->mode <= 1 ? 2 : 3;
	}
    }


    //--- fall back to pure numbers

    uint ret_val = 0;
    char *dest = buf;
    char *end = buf + buf_size - 2;

    int np;
    for ( np = 11; np >= 0 && dest < end; np-- )
    {
	const u8 *row = data + 12*np;
	if ( mode >= 1 && np > 1 )
	{
	    // ranges allowed

	    int delta = (int)row[np-1] - (int)row[np];
	    if ( delta >= 0 )
	    {
		int ni = np - 1;
		while ( ni > 0 && (int)row[ni-1] - (int)row[ni] == delta )
		    ni--;
		delta++;
		while ( ni > 0 && (int)row[ni-1] - (int)row[ni] == delta )
		    ni--;
		if ( np - ni > 2 )
		{
		    int i;
		    for ( i = 0; i <= ni && dest < end; i++ )
			dest += snprintf(dest,end-dest,"%u,",row[i]);
		    dest--;
		    if ( dest < end )
			dest += snprintf(dest,end-dest,"..%u,",row[np]);
		    dest[-1] = '/';
		    ret_val = 1;
		    continue;
		}
	    }
	}

	int i;
	for ( i = 0; i <= np && dest < end; i++ )
	    dest += snprintf(dest,end-dest,"%u,",*row++);
	dest[-1] = '/';
    }
    dest[-1] = 0;
    return ret_val;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[24P--]]

void PrintMkwPoints
(
    FILE	*f,		// destination file
    uint	indent,		// indention of the output
    const u8	*data,		// data, 12*12 table
    uint	print_table,	// print the table:
				//    0: don't print
				//    1: print table as RAW
				//    2: print table as C listing
				//  A-Z: Print as cheat code with this region
    u32		cheat_base,	// not NULL: value of first param of cheat code
    uint	print_param	// print a parameter string:
				//    0: don't print
				//    1: print single line
				//    2: print all modes
)
{
    switch (print_table)
    {
	case 1:
	    PrintMkwPointsRAW(f,indent,data);
	    fputc('\n',f);
	    break;

	case 2:
	    PrintMkwPointsC(f,indent,data);
	    fputc('\n',f);
	    break;

	default:
	    if ( print_table >= 'A' && print_table <= 'Z' && cheat_base )
	    {
		PrintMkwPointsCheat(f,data,print_table,cheat_base);
		fputc('\n',f);
	    }
	    break;
    }

    char buf[MKW_POINTS_DEF_SIZE];
    if ( print_param == 1 )
    {
	PrintMkwPointsLIST(buf,sizeof(buf),data,2);
	printf("%*s%s\n\n",indent,"",buf);
    }
    else if ( print_param > 1 )
    {
	int mode;
	for ( mode = 3; mode >= 0; mode-- )
	{
	    const uint res = PrintMkwPointsLIST(buf,sizeof(buf),data,mode);
	    if ( res == mode )
		printf("%*s%s\n\n",indent,"",buf);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			GetVrDiff*()			///////////////
///////////////////////////////////////////////////////////////////////////////
// https://forum.wii-homebrew.com/index.php?page=Thread&postID=612772
// https://wiki.tockdom.com/wiki/Player_Rating
// https://www.desmos.com/calculator/gr17y7d944
///////////////////////////////////////////////////////////////////////////////

static const s16 vr_diff_tab[112] =
{
      6598,  4999,  3963,  3233,  2671,  2212,  1819,  1476,  //   0 ..   7
      1168,   889,   633,   395,   174,   -34,  -231,  -418,  //   8 ..  15
      -596,  -767,  -931, -1090, -1243, -1392, -1536, -1676,  //  16 ..  23
     -1813, -1946, -2076, -2203, -2328, -2450, -2569, -2687,  //  24 ..  31
     -2802, -2915, -3026, -3136, -3243, -3349, -3454, -3557,  //  32 ..  39
     -3658, -3759, -3857, -3955, -4051, -4147, -4241, -4334,  //  40 ..  47
     -4425, -4516, -4606, -4695, -4783, -4870, -4957, -5042,  //  48 ..  55
     -5127, -5211, -5294, -5377, -5459, -5540, -5622, -5702,  //  56 ..  63
     -5783, -5863, -5942, -6022, -6101, -6180, -6259, -6338,  //  64 ..  71
     -6417, -6495, -6574, -6653, -6731, -6810, -6889, -6968,  //  72 ..  79
     -7048, -7127, -7207, -7287, -7367, -7448, -7530, -7611,  //  80 ..  87
     -7693, -7776, -7860, -7944, -8029, -8114, -8201, -8288,  //  88 ..  95
     -8377, -8466, -8557, -8649, -8743, -8838, -8935, -9034,  //  96 .. 103
     -9135, -9239, -9345, -9455, -9568, -9684, -9806, -9933,  // 104 .. 111
};

///////////////////////////////////////////////////////////////////////////////

int GetVrDiffByTab ( int diff ) // diff = VR(winner) - VR(loser)
{
    if ( diff > 6598 )
	return 0;
    if ( diff < -9933 )
	return 112;

    const s16 search = diff;
    int i = 0, j = sizeof(vr_diff_tab) / sizeof(*vr_diff_tab) - 1;

    while ( i < j )
    {
	const int k = (i+j)/2;
	if ( search > vr_diff_tab[k] )
	    j = k-1;
	else if ( search <= vr_diff_tab[k+1] )
	    i = k+1;
	else
	    return k+1;
    }

    return i+1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			info tables			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetMkwTrackName3 ( int tid )
{
    static const char tab[][4] =
    {
	//-- tracks:  0..31 = 0x00..0x1f

	"T21", "T12", "T13", "T34",
	"T14", "T22", "T23", "T24",
	"T11", "T31", "T42", "T33",
	"T43", "T44", "T41", "T32",

	"T51", "T74", "T64", "T83",
	"T52", "T71", "T82", "T63",
	"T81", "T53", "T54", "T61",
	"T84", "T73", "T72", "T62",

	//-- arenas: 32..41 = 0x20..0x29

	"A12", "A11", "A14", "A13", "A15",
	"A24", "A25", "A21", "A22", "A23",

	//-- special: 42..67 = 0x2a..0x43

	"", "", "", "",  "", "",
	"", "", "", "",  "", "", "MIS", "WIN",
	"LOS", "DRW", "END", "",  "", "", "", "",
	"", "", "inv", "non",
    };

    if ( (uint)tid < sizeof(tab)/sizeof(*tab) && tab[tid][0] )
	return tab[tid];

    if ( tid < 0 )
	return "---";

    if ( tid == 0xff )
	return "rnd";

    char buf[20];
    const int len = snprintf(buf,sizeof(buf),"_%02x",tid) + 1;
    char * dest = GetCircBuf(len);
    memcpy(dest,buf,len);
    return dest;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetMkwMusicName3 ( int mid )
{
    static const char tab[][4] =
    {
	"T11", "t11",
	"T12", "t12",
	"T13", "t13",
	"T14", "t14",
	"T21", "t21",
	"T22", "t22",
	"T23", "t23",
	"T24", "t24",
	"T32", "t32",
	"T31", "t31",
	"T41", "t41",
	"T34", "t34",
	"T42", "t42",
	"T33", "t33",
	"T43", "t43",
	"T44", "t44",
	"T62", "t62",
	"T53", "t53",
	"T81", "t81",
	"T72", "t72",
	"T61", "t61",
	"T54", "t54",
	"T73", "t73",
	"T84", "t84",
	"T51", "t51",
	"T74", "t74",
	"T64", "t64",
	"T83", "t83",
	"T52", "t52",
	"T63", "t63",
	"T71", "t71",
	"T82", "t82",
	"A12", "a12",
	"A11", "a11",
	"A13", "a13",
	"A14", "a14",
	"A15", "a15",
	"A24", "a24",
	"A25", "a25",
	"A21", "a21",
	"A22", "a22",
	"A23", "a23",
    };

    if ( mid >= MKW_MUSIC_MIN_ID && mid < MKW_MUSIC_MIN_ID + sizeof(tab)/sizeof(*tab) )
	return tab[mid-MKW_MUSIC_MIN_ID];

    if ( mid < 0 )
	return "---";

    char buf[20];
    const int len = snprintf(buf,sizeof(buf),"_%02x",mid) + 1;
    char * dest = GetCircBuf(len);
    memcpy(dest,buf,len);
    return dest;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			engines				///////////////
///////////////////////////////////////////////////////////////////////////////

const char mkw_engine_info[MKW_ENGINE__N+1][7] =
{
    "50cc",
    "100cc",
    "150cc",
    "mirror",
    "200cc",
    "?",
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    slots, drivers and vehicles		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetSlotHex ( u8 slot )
{
    if (IsMkwSlotValid(slot))
    {
	char *dest = GetCircBuf(2);
	dest[0] = LoDigits[slot];
	dest[1] = 0;
	return dest;
    }
    return "-";
}

//-----------------------------------------------------------------------------

ccp GetSlotNum ( u8 slot )
{
    if (IsMkwSlotValid(slot))
	return PrintCircBuf("%2u",slot);
    return " -";
}

///////////////////////////////////////////////////////////////////////////////

const mkw_driver_t mkw_driver_attrib[MKW_N_DRIVER+1] =
{
	{ 1, 0, "Mari",	"Mari",	"Mario",	"Mario" },		// M0 #0
	{ 0, 0, "B.Pe",	"B.Pe",	"Baby Peach",	"Baby Peach" },		// S2
	{ 2, 0, "Walu",	"Walu",	"Waluigi",	"Waluigi" },		// L1
	{ 2, 0, "Bows",	"Bows",	"Bowser",	"Bowser" },		// L3

	{ 0, 0, "B.Da",	"B.Da",	"Baby Daisy",	"Baby Daisy" },		// S3 #4
	{ 0, 0, "DBon",	"KnTr",	"Dry Bones",	"Knochentrocken" },	// S7
	{ 0, 0, "B.Ma",	"B.Ma",	"Baby Mario",	"Baby Mario" },		// S0
	{ 1, 0, "Luig",	"Luig",	"Luigi",	"Luigi" },		// M1

	{ 0, 0, "Toad",	"Toad",	"Toad",		"Toad" },		// S4 #8
	{ 2, 0, "Donk",	"Donk",	"Donkey Kong",	"Donkey Kong" },	// L2
	{ 1, 0, "Yosh",	"Yosh",	"Yoshi",	"Yoshi" },		// M4
	{ 2, 0, "Wari",	"Wari",	"Wario",	"Wario" },		// L0

	{ 0, 0, "B.Lu",	"B.Lu",	"Baby Luigi",	"Baby Luigi" },		// S1 #12
	{ 0, 0, "Tdte",	"Tdte",	"Toadette",	"Toadette" },		// S5
	{ 0, 0, "Koop",	"Koop",	"Koopa Troopa",	"Koopa" },		// S6
	{ 1, 0, "Dais",	"Dais",	"Daisy",	"Daisy" },		// M3

	{ 1, 0, "Peac",	"Peac",	"Peach",	"Peach" },		// M2 #16
	{ 1, 0, "Bird",	"Bird",	"Birdo",	"Birdo" },		// M5
	{ 1, 0, "DidK",	"DidK",	"Diddy Kong",	"Diddy Kong" },		// M6
	{ 2, 0, "KBoo",	"BuHu",	"King Boo",	"König Buu Huu" },	// L4

	{ 1, 0, "B.Jr",	"B.Jr",	"Bowser Jr.",	"Bowser Jr." },		// M7 #20
	{ 2, 0, "DBow",	"KnBo",	"Dry Bowser",	"Knochen-Bowser" },	// L7
	{ 2, 0, "Funk",	"Funk",	"Funky Kong",	"Funky Kong" },		// L6
	{ 2, 0, "Rosa",	"Rosa",	"Rosalina",	"Rosalina" },		// L5

	{ 0, 1, "MAsm",	"MAkm",	"Mii A (small,male)",	"Mii A (klein,männ)" },	// S8 #24
	{ 0, 1, "MAsf",	"MAkw",	"Mii A (small,female)",	"Mii A (klein,weib)" },	// S9
	{ 0, 2, "MBsm",	"MBkm",	"Mii B (small,male)",	"Mii B (klein,männ)" },	// S10
	{ 0, 2, "MBsf",	"MBkw",	"Mii B (small,female)",	"Mii B (klein,weib)" },	// S11
	{ 0, 3, "MCsm",	"MCkm",	"Mii C (small,male)",	"Mii C (klein,männ)" },	// 0x11c
	{ 0, 3, "MCsf",	"MCkw",	"Mii C (small,female)",	"Mii C (klein,weib)" },	// 0x11d

	{ 1, 1, "MAmm",	"MAmm",	"Mii A (medium,male)",	"Mii A (mittel,männ)" },// M8 #30
	{ 1, 1, "MAmf",	"MAmw",	"Mii A (medium,female)","Mii A (mittel,weib)" },// M9
	{ 1, 2, "MBmm",	"MBmm",	"Mii B (medium,male)",	"Mii B (mittel,männ)" },// M10
	{ 1, 2, "MBmf",	"MBmw",	"Mii B (medium,female)","Mii B (mittel,weib)" },// M11
	{ 1, 3, "MCmm",	"MCmm",	"Mii C (medium,male)",	"Mii C (mittel,männ)" },// 0x122
	{ 1, 3, "MCmf",	"MCmw",	"Mii C (medium,female)","Mii C (mittel,weib)" },// 0x123

	{ 2, 1, "MAlm",	"MAgm",	"Mii A (large,male)",	"Mii A (groß,männ)" },	// L8 #36
	{ 2, 1, "MAlf",	"MAgw",	"Mii A (large,female)",	"Mii A (groß,weib)" },	// L9
	{ 2, 2, "MBlm",	"MBgm",	"Mii B (large,male)",	"Mii B (groß,männ)" },	// L10
	{ 2, 2, "MBlf",	"MBgw",	"Mii B (large,female)",	"Mii B (groß,weib)" },	// L11
	{ 2, 3, "MClm",	"MCgm",	"Mii C (large,male)",	"Mii C (groß,männ)" },	// 0x128
	{ 2, 3, "MClf",	"MCgw",	"Mii C (large,female)",	"Mii C (groß,weib)" },	// 0x129

	{0,0,"","",0,0} // TERM #42
};

//-----------------------------------------------------------------------------

ccp GetDriverNum ( u8 driver )
{
    switch (driver)
    {
     case  48: return "  ?";
     case 254: return "  .";
     case 255: return "  -";
    }

    return PrintCircBuf("%3u",driver);
}

//-----------------------------------------------------------------------------

ccp GetDriverName4 ( u8 driver )
{
    if ( driver < MKW_N_DRIVER )
    {
	ccp res = mkw_driver_attrib[driver].id_en;
	if (*res)
	    return res;
    }

    switch (driver)
    {
     case  48: return " ?? ";
     case 254: return " .. ";
     case 255: return " -- ";
    }

    return PrintCircBuf("%3u.",driver);
}

///////////////////////////////////////////////////////////////////////////////

const mkw_vehicle_t mkw_vehicle_attrib[MKW_N_VEHICLE+1] =
{
    { 0,0,1, "Ka-S", "MinK", "Standard Kart S",	"Mini-Kart" },		// SK0 #0
    { 1,0,1, "Ka-M", "MedK", "Standard Kart M",	"Medi-Kart" },		// MK0
    { 2,0,1, "Ka-L", "MaxK", "Standard Kart L",	"Maxi-Kart" },		// LK0
    { 0,0,0, "Boos", "Baby", "Booster Seat",	"Baby-Booster" },	// SK1
    { 1,0,0, "Drag", "Nost", "Classic Dragster","Nostalgia 1" },	// MK1
    { 2,0,0, "OffR", "OffR", "Offroader",	"Offroader" },		// LK1

    { 0,0,0, "Mini", "MinB", "Mini Beast",	"Minibiest" },		// SK2 #6
    { 1,0,0, "Wing", "Wind", "Wild Wing",	"Windschnitte" },	// MK2
    { 2,0,0, "Flam", "Feue", "Flame Flyer",	"Feuerschleuder" },	// LK2
    { 0,0,0, "Chep", "BobC", "Cheep Charger",	"Bob-Cheep" },		// SK3
    { 1,0,0, "Bloo", "Bloo", "Super Blooper",	"Blooper-Bolide" },	// MK3
    { 2,0,0, "Pira", "PPir", "Piranha Prowler",	"Pistenpiranha" },	// LK3

    { 0,0,0, "Tita", "Monz", "Tiny Titan",	"Monztruck" },		// SK4 #12
    { 1,0,0, "Dayt", "Cabr", "Daytripper",	"Cabriosa" },		// MK4
    { 2,0,0, "JetS", "Ster", "Jetsetter",	"Sterngleiter" },	// LK4
    { 0,0,0, "Falc", "Falc", "Blue Falcon",	"Blue Falcon" },	// SK5
    { 1,0,0, "Spri", "Glor", "Sprinter",	"Glory" },		// MK5
    { 2,0,0, "Hony", "Turb", "Honeycoupe",	"Turbetto" },		// LK5

    { 0,1,1, "Bi-S", "MinB", "Standard Bike S",	"Mini-Bike" },		// SB0 #18
    { 1,1,1, "Bi-M", "MedB", "Standard Bike M",	"Medi-Bike" },		// MB0
    { 2,1,1, "Bi-L", "MaxB", "Standard Bike L",	"Maxi-Bike" },		// LB0
    { 0,2,0, "Bull", "Wili", "Bullet Bike",	"Willi-Bike" },		// SB1
    { 1,2,0, "Mach", "Mach", "Mach Bike",	"Mach-Bike" },		// MB1
    { 2,2,0, "Flam", "BowB", "Flame Runner",	"Bowser-Bike" },	// LB1

    { 0,1,0, "BitB", "Blit", "Bit Bike",	"Blitzer" },		// SB2 #24
    { 1,1,0, "Suga", "Bonb", "Sugarscoot",	"Bonbon-Bike" },	// MB2
    { 2,1,0, "WarB", "WarB", "Wario Bike",	"Wario-Bike" },		// LB2
    { 0,2,0, "Quac", "Quak", "Quacker",		"Quakquak" },		// SB3
    { 1,1,0, "ZipZ", "Rapi", "Zip Zip",		"Rapido" },		// MB3
    { 2,1,0, "Shoo", "Schn", "Shooting Star",	"Schnuppe" },		// LB3

    { 0,2,0, "MKru", "Kame", "Magikruiser",	"Kameknaller" },	// SB4 #30
    { 1,2,0, "Snea", "Pist", "Sneakster",	"Pistensturm" },	// MB4
    { 2,2,0, "Spea", "Torp", "Spear",		"Torped" },		// LB4
    { 0,2,0, "JetB", "JetA", "Jet Bubble",	"Jet-A" },		// SB5
    { 1,2,0, "Dolp", "Flip", "Dolphin Dasher",	"Flippster" },		// MB5
    { 2,1,0, "Phan", "Phan", "Phantom",		"Phantom" },		// LB5

    {0,0,0,"","",0,0} // TERM #36
};

//-----------------------------------------------------------------------------

ccp GetVehicleNum ( u8 vehicle )
{
    switch (vehicle)
    {
     case  36: return "  ?";
     case 254: return "  .";
     case 255: return "  -";
    }

    return PrintCircBuf("%3u",vehicle);
}

//-----------------------------------------------------------------------------

ccp GetVehicleName4 ( u8 vehicle )
{
    if ( vehicle < MKW_N_VEHICLE )
	return mkw_vehicle_attrib[vehicle].id_en;

    switch (vehicle)
    {
     case  36: return " ?? ";
     case 254: return " .. ";
     case 255: return " -- ";
    }

    return PrintCircBuf("%3u.",vehicle);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LE-CODE track flags		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct use_color_t
{
    char	*buf;	// valid destination buffer
    char	*ptr;	// current pointer for appends, if NULL: set to 'buf' first
    uint	size;	// size of 'buf'
    ColorSet_t	*col;	// valid color definition, never NULL
    int		index;	// current color index: 0:off, 1:on, 2:bold
}
use_color_t;

//-----------------------------------------------------------------------------

static char * UseColor
(
    // returns 'ucol.ptr'
    use_color_t	*ucol,	// initialized data structure
    int		index,	// next color
    char	ch	// not NULL: Append character
)
{
    DASSERT(ucol);
    DASSERT(ucol->buf);
    DASSERT(ucol->col);
    if (!ucol->ptr)
	ucol->ptr = ucol->buf;

    ccp copy_src = 0;
    int copy_len = 0;
    if ( ucol->index != index )
    {
	ucol->index = index;
	switch (index)
	{
	    case 1:  copy_src = ucol->col->info; break;
	    case 2:  copy_src = ucol->col->hint; break;
	    default: copy_src = ucol->col->reset; break;
	}
	copy_len = strlen(copy_src);
    }

    if ( ucol->ptr + copy_len < ucol->buf + ucol->size - 1 )
    {
	if (copy_len)
	{
	    memcpy(ucol->ptr,copy_src,copy_len);
	    ucol->ptr += copy_len;
	}
	if (ch)
	    *ucol->ptr++ = ch;
    }

    *ucol->ptr = 0;
    return ucol->ptr;
}

///////////////////////////////////////////////////////////////////////////////

le_flags_t ScanLEFL ( ccp text )
{
    // automatic flags are not scannend!

    le_flags_t res = 0;
    if (text)
    {
	if ( strchr(text,'N') || strchr(text,'n') ) res |= G_LEFL_NEW;
	if ( strchr(text,'H') || strchr(text,'h') ) res |= G_LEFL_RND_HEAD;
	if ( strchr(text,'G') || strchr(text,'g') ) res |= G_LEFL_RND_GROUP;
	if ( strchr(text,'X') || strchr(text,'x') ) res |= G_LEFL_RND_HEAD | G_LEFL_RND_GROUP;
	if ( strchr(text,'A') || strchr(text,'a') ) res |= G_LEFL_ALIAS;
	if ( strchr(text,'T') || strchr(text,'t') ) res |= G_LEFL_TEXTURE;
	if ( strchr(text,'2')                     ) res |= G_LEFL_ALIAS | G_LEFL_TEXTURE;
	if ( strchr(text,'I') || strchr(text,'i') ) res |= G_LEFL_HIDDEN;
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintLEFL8 ( le_flags_t flags, bool aligned )
{
    char *buf = GetCircBuf(7);
    flags &= 0xff;

    if ( flags & ~G_LEFL__ALL )
	snprintf(buf,7,"?0x%02x ",flags);
    else if (aligned)
    {
	buf[0] = flags & G_LEFL_NEW		? 'N' : '-';
	buf[1] = flags & G_LEFL_RND_HEAD	? 'H' : '-';
	buf[2] = flags & G_LEFL_RND_GROUP	? 'G' : '-';
	buf[3] = flags & G_LEFL_ALIAS		? 'A' : '-';
	buf[4] = flags & G_LEFL_TEXTURE		? 'T' : '-';
	buf[5] = flags & G_LEFL_HIDDEN		? 'i' : '-';
	buf[6] = 0;
    }
    else
    {
	char *d = buf;
	if ( flags & G_LEFL_NEW		) *d++ = 'N';
	if ( flags & G_LEFL_RND_HEAD	) *d++ = 'H';
	if ( flags & G_LEFL_RND_GROUP	) *d++ = 'G';
	if ( flags & G_LEFL_ALIAS	) *d++ = 'A';
	if ( flags & G_LEFL_TEXTURE	) *d++ = 'T';
	if ( flags & G_LEFL_HIDDEN	) *d++ = 'i';
	if ( d == buf )
	    *d++ = '-';
	*d = 0;
    }
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintLEFL8col ( le_flags_t flags, bool aligned, ColorSet_t *col )
{
 #if 1
    return PrintLEFL8(flags,aligned);
 #else
    if (!col)
	return PrintLEFL8(flags,aligned);

    // ??? [[2do]]
    return PrintLEFL8(flags,aligned);
 #endif
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintLEFL16 ( le_flags_t flags, bool aligned )
{
    char *buf = GetCircBuf(7);

    if ( flags & ~G_LEFL__XALL )
    {
	snprintf(buf,7,"?0x%03x ",flags);
	return buf;
    }

    char *d = buf;
    switch ( flags & (G_LEFL_BATTLE|G_LEFL_VERSUS|G_LEFL_RANDOM) )
    {
	case G_LEFL_BATTLE: *d++ = 'B'; break;
	case G_LEFL_VERSUS: *d++ = 'V'; break;
	case G_LEFL_RANDOM: *d++ = 'r'; break;
	default:
	    if ( aligned )
		*d++ = '-';
    }

    switch ( flags & (G_LEFL_ORIG_CUP|G_LEFL_CUSTOM_CUP) )
    {
	case G_LEFL_ORIG_CUP:				*d++ = 'o'; break;
	case G_LEFL_CUSTOM_CUP:				*d++ = 'c'; break;
	case G_LEFL_ORIG_CUP | G_LEFL_CUSTOM_CUP:	*d++ = 'b'; break;
	default:
	    if ( aligned )
		*d++ = '-';
    }

    switch ( flags & (G_LEFL_RND_HEAD|G_LEFL_RND_GROUP) )
    {
	case G_LEFL_RND_HEAD:				*d++ = 'H'; break;
	case G_LEFL_RND_GROUP:				*d++ = 'G'; break;
	case G_LEFL_RND_HEAD | G_LEFL_RND_GROUP:	*d++ = 'X'; break;
	default:
	    if ( aligned )
		*d++ = '-';
    }

    switch ( flags & (G_LEFL_NEW|G_LEFL_TEXTURE) )
    {
	case G_LEFL_NEW:				*d++ = 'N'; break;
	case G_LEFL_TEXTURE:				*d++ = 'T'; break;
	case G_LEFL_NEW | G_LEFL_TEXTURE:		*d++ = '2'; break;
	default:
	    if ( aligned )
		*d++ = '-';
    }

    if (aligned)
    {
	*d++ = flags & G_LEFL_ALIAS	? 'A' : '-';
	*d++ = flags & G_LEFL_HIDDEN	? 'i' : '-';
    }
    else
    {
	if ( flags & G_LEFL_ALIAS	) *d++ = 'A';
	if ( flags & G_LEFL_HIDDEN	) *d++ = 'i';
	if ( d == buf )
	    *d++ = '-';
    }
    *d = 0;
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintLEFL16col ( le_flags_t flags, bool aligned, ColorSet_t *col )
{
    if ( !col || (flags & ~G_LEFL__XALL) )
	return PrintLEFL16(flags,aligned);

    char buf[300];
    use_color_t ucol = { .buf = buf, .size = sizeof(buf), .col = col };

    switch ( flags & (G_LEFL_BATTLE|G_LEFL_VERSUS|G_LEFL_RANDOM) )
    {
	case G_LEFL_BATTLE: UseColor(&ucol,1,'B'); break;
	case G_LEFL_VERSUS: UseColor(&ucol,0,'V'); break;
	case G_LEFL_RANDOM: UseColor(&ucol,2,'r'); break;
	default:
	    if ( aligned )
		UseColor(&ucol,0,'-');
    }

    switch ( flags & (G_LEFL_ORIG_CUP|G_LEFL_CUSTOM_CUP) )
    {
	case G_LEFL_ORIG_CUP:				UseColor(&ucol,0,'o'); break;
	case G_LEFL_CUSTOM_CUP:				UseColor(&ucol,1,'c'); break;
	case G_LEFL_ORIG_CUP | G_LEFL_CUSTOM_CUP:	UseColor(&ucol,2,'b'); break;
	default:
	    if ( aligned )
		UseColor(&ucol,0,'-');
    }

    switch ( flags & (G_LEFL_RND_HEAD|G_LEFL_RND_GROUP) )
    {
	case G_LEFL_RND_HEAD:				UseColor(&ucol,1,'H'); break;
	case G_LEFL_RND_GROUP:				UseColor(&ucol,0,'G'); break;
	case G_LEFL_RND_HEAD | G_LEFL_RND_GROUP:	UseColor(&ucol,2,'X'); break;
	default:
	    if ( aligned )
		UseColor(&ucol,0,'-');
    }

    switch ( flags & (G_LEFL_NEW|G_LEFL_TEXTURE) )
    {
	case G_LEFL_NEW:				UseColor(&ucol,0,'N'); break;
	case G_LEFL_TEXTURE:				UseColor(&ucol,1,'T'); break;
	case G_LEFL_NEW | G_LEFL_TEXTURE:		UseColor(&ucol,2,'2'); break;
	default:
	    if ( aligned )
		UseColor(&ucol,0,'-');
    }

    if (aligned)
    {
	UseColor(&ucol,0, flags & G_LEFL_ALIAS	? 'A' : '-' );
	UseColor(&ucol,0, flags & G_LEFL_HIDDEN	? 'i' : '-' );
    }
    else
    {
	if ( flags & G_LEFL_ALIAS	) UseColor(&ucol,0,'A');
	if ( flags & G_LEFL_HIDDEN	) UseColor(&ucol,0,'i');
	if ( ucol.ptr == buf )
	    UseColor(&ucol,0,'-');
    }

    return CopyCircBuf0(buf,ucol.ptr-buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			distribution flags		///////////////
///////////////////////////////////////////////////////////////////////////////

DistribFlags_t ScanDTA ( ccp text )
{
    // automatic flags are not scannend!

    DistribFlags_t res = 0;
    if (text)
    {
	while ( *text )
	{
	    const int ch = tolower(*text++);
	    switch (ch)
	    {
		case 'b': res |= G_DTA_BOOST; break;
		case 'n': res |= G_DTA_NEW; break;
		case 'a': res |= G_DTA_AGAIN; break;
		case 'u': res |= G_DTA_UPDATE; break;
		case 'f': res |= G_DTA_FILL; break;
		case 'd': res |= G_DTA_IS_D; break;
		case 't': res |= G_DTA_TITLE; break;
		case 'h': res |= G_DTA_HIDDEN; break;
		case 'o': res |= G_DTA_ORIGINAL; break;
	    }
	}
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintDTA ( DistribFlags_t flags, bool aligned )
{
    char *buf = GetCircBuf(10);

    if (aligned)
    {
	buf[0] = flags & G_DTA_BOOST	? 'B' : '-';
	buf[1] = flags & G_DTA_NEW	? 'N' : '-';
	buf[2] = flags & G_DTA_AGAIN	? 'A' : '-';
	buf[3] = flags & G_DTA_UPDATE	? 'U' : '-';
	buf[4] = flags & G_DTA_FILL	? 'F' : '-';
	buf[5] = flags & G_DTA_IS_D	? 'd' : '-';
	buf[6] = flags & G_DTA_TITLE	? 't' : '-';
	buf[7] = flags & G_DTA_HIDDEN	? 'h' : '-';
	buf[8] = flags & G_DTA_ORIGINAL	? 'o' : '-';
	buf[9] = 0;
    }
    else
    {
	char *d = buf;
	if ( flags & G_DTA_BOOST	) *d++ = 'B';
	if ( flags & G_DTA_NEW		) *d++ = 'N';
	if ( flags & G_DTA_AGAIN	) *d++ = 'A';
	if ( flags & G_DTA_UPDATE	) *d++ = 'U';
	if ( flags & G_DTA_FILL		) *d++ = 'F';
	if ( flags & G_DTA_IS_D		) *d++ = 'd';
	if ( flags & G_DTA_TITLE	) *d++ = 't';
	if ( flags & G_DTA_HIDDEN	) *d++ = 'a';
	if ( flags & G_DTA_ORIGINAL	) *d++ = 'o';
	if ( d == buf )
	    *d++ = '-';
	*d = 0;
    }
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			track class			///////////////
///////////////////////////////////////////////////////////////////////////////

TrackClass_t ScanTrackClass ( ccp text, int text_len )
{
    TrackClass_t res = 0;
    if (text)
    {
	ccp end = text + ( text_len < 0 ? strlen(text) : text_len );
	while ( text < end )
	{
	    const int ch = tolower(*text++);
	    switch (ch)
	    {
		case 'p': res |= G_TCLS_PRIVATE; break;
		case 't': res |= G_TCLS_TEST; break;
		case 'b': res |= G_TCLS_BAD; break;
		case 'f': res |= G_TCLS_FAIL; break;
		case 'z': res |= G_TCLS_FREEZE; break;
		case 'k': res |= G_TCLS_STOCK; break;
		case 's': res |= G_TCLS_SELECT; break;
		case 'n': res |= G_TCLS_NINTENDO; break;
		case 'o': res |= G_TCLS_BOOST; break;
		case 'i': res |= G_TCLS_INCOME; break;
		case 'w': res |= G_TCLS_WAIT; break;
	    }
	}
PRINT0("%04x <= %.*s\n",res,text_len,text-text_len);
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintTrackClass ( TrackClass_t flags, bool aligned )
{
    char *buf = GetCircBuf(12);

    if (aligned)
    {
	buf[ 0] = flags & G_TCLS_PRIVATE	? 'P' : '-';
	buf[ 1] = flags & G_TCLS_TEST		? 'T' : '-';
	buf[ 2] = flags & G_TCLS_BAD		? 'B' : '-';
	buf[ 3] = flags & G_TCLS_FAIL		? 'F' : '-';
	buf[ 4] = flags & G_TCLS_FREEZE		? 'Z' : '-';
	buf[ 5] = flags & G_TCLS_STOCK		? 'K' : '-';
	buf[ 6] = flags & G_TCLS_SELECT		? 'S' : '-';
	buf[ 7] = flags & G_TCLS_NINTENDO	? 'N' : '-';
	buf[ 8] = flags & G_TCLS_BOOST		? 'O' : '-';
	buf[ 9] = flags & G_TCLS_INCOME		? 'I' : '-';
	buf[10] = flags & G_TCLS_WAIT		? 'W' : '-';
	buf[11] = 0;
    }
    else
    {
	char *d = buf;
	if ( flags & G_TCLS_PRIVATE	) *d++ = 'P';
	if ( flags & G_TCLS_TEST	) *d++ = 'T';
	if ( flags & G_TCLS_BAD		) *d++ = 'B';
	if ( flags & G_TCLS_FAIL	) *d++ = 'F';
	if ( flags & G_TCLS_FREEZE	) *d++ = 'Z';
	if ( flags & G_TCLS_STOCK	) *d++ = 'K';
	if ( flags & G_TCLS_SELECT	) *d++ = 'S';
	if ( flags & G_TCLS_NINTENDO	) *d++ = 'N';
	if ( flags & G_TCLS_BOOST	) *d++ = 'O';
	if ( flags & G_TCLS_INCOME	) *d++ = 'I';
	if ( flags & G_TCLS_WAIT	) *d++ = 'W';
	if ( d == buf )
	    *d++ = '-';
	*d = 0;
    }
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

TrackClassIndex_t GetTrackClassIndex ( TrackClass_t flags )
{
    static const TrackClassIndex_t tab[] =
    {
	G_TCLSI_INCOME,
	G_TCLSI_WAIT,
	 G_TCLSI_PRIVATE,
	 G_TCLSI_TEST,
	G_TCLSI_FREEZE,
	G_TCLSI_FAIL,
	G_TCLSI_BAD,
	 G_TCLSI_NINTENDO,
	 G_TCLSI_SELECT,
	 G_TCLSI_BOOST,
	 G_TCLSI_STOCK,
	0,
    };

    for ( const TrackClassIndex_t *ptr = tab; *ptr; ptr++ )
	if ( 1 << *ptr & flags )
	    return *ptr;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////

ccp GetTrackClassName ( TrackClassIndex_t idx, ccp return_on_none )
{
    static const char tab[G_TCLSI__N][9] =
    {
	"UNKNOWN",
	"PRIVATE",
	"TEST",
	"BAD",
	"FAIL",
	"FREEZE",
	"STOCK",
	"SELECT",
	"NINTENDO",
	"BOOST",
	"INCOME",
	"WAIT",
    };

    return (uint)idx < G_TCLSI__N ? tab[idx] : return_on_none;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			random slots			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetLecodeRandomName ( uint slot, ccp return_on_err )
{
    _Static_assert( MKW_N_LE_RANDOM == 5,"GetLecodeRandomName()");
    static const char name[][6] = { "rTex", "rAll", "rOrig", "rCust", "rNew" };
    return IsLecodeRandom(slot)
		? name[ slot - MKW_LE_RANDOM_BEG ]
		: return_on_err;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetLecodeRandomInfo ( uint slot, ccp return_on_err )
{
    _Static_assert( MKW_N_LE_RANDOM == 5,"GetLecodeRandomInfo()");
    static ccp info[] =
    {
	    "Random: Texture Hacks",
	    "Random: All Tracks",
	    "Random: Original Tracks",
	    "Random: Custom Tracks",
	    "Random: New Tracks",
    };

    return IsLecodeRandom(slot)
		? info[ slot - MKW_LE_RANDOM_BEG ]
		: return_on_err;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetMkwExtraInfo ( uint slot, ccp return_on_err )
{
    _Static_assert( MKW_N_EXTRA == 5,"GetMkwExtraInfo()");
    static ccp info[] =
    {
	    "Extra: Galaxy Colosseum",
	    "Extra: Winning scene",
	    "Extra: Losing scene",
	    "Extra: Luigi Circuit (Credits)",
	    "Extra: Broken losing scene",
    };

    return IsMkwExtra(slot)
		? info[ slot - MKW_EXTRA_BEG ]
		: return_on_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sizeof_info_t: mkw		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sizeof_info_mkw]]

const sizeof_info_t sizeof_info_mkw[] =
{
    SIZEOF_INFO_TITLE("Mario Kart Wii")

	SIZEOF_INFO_ENTRY(mkw_std_pbits1_t)
	SIZEOF_INFO_ENTRY(mkw_std_pbits2_t)
	SIZEOF_INFO_ENTRY(mkw_ex_pbits1_t)
	SIZEOF_INFO_ENTRY(mkw_ex_pbits2_t)
	SIZEOF_INFO_ENTRY(mkw_pbits1_t)
	SIZEOF_INFO_ENTRY(mkw_pbits2_t)
	SIZEOF_INFO_ENTRY(MkwPointInfo_t)
	SIZEOF_INFO_ENTRY(mkw_engine_t)
	SIZEOF_INFO_ENTRY(mkw_engine_info)
	SIZEOF_INFO_ENTRY(mkw_driver_t)
	SIZEOF_INFO_ENTRY(mkw_driver_attrib)
	SIZEOF_INFO_ENTRY(mkw_vehicle_t)
	SIZEOF_INFO_ENTRY(mkw_vehicle_attrib)

    SIZEOF_INFO_TERM()
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


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
 *        Copyright (c) 2012-2021 by Dirk Clemens <wiimm@wiimm.de>         *
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
// http://wiki.tockdom.com/wiki/Player_Rating
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
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

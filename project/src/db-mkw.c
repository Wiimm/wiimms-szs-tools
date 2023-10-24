
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////


#include "db-mkw.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct TrackInfo_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const TrackInfo_t InvalidTrack = { 0, 99, 99, 99, 0x00, "-", "?", "?", "-", "-" };

//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////   tracks   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// track positions

const u32 track_pos_default[32] =
{
	 8, 1, 2, 4,  0, 5, 6, 7,  9,15,11, 3, 14,10,12,13,
	16,20,25,26, 27,31,23,18, 21,30,29,17, 24,22,19,28
};

u32 track_pos[32] =
{
	 8, 1, 2, 4,  0, 5, 6, 7,  9,15,11, 3, 14,10,12,13,
	16,20,25,26, 27,31,23,18, 21,30,29,17, 24,22,19,28
};

u32 track_pos_r[32] =
{
	 4, 1, 2,11,  3, 5, 6, 7,  0, 8,13,10, 14,15,12, 9,
	16,27,23,30, 17,24,29,22, 28,18,19,20, 31,26,25,21
};

u32 track_pos_swap[32] =
{
	 8, 1, 2, 4,	16,20,25,26,  0, 5, 6, 7, 27,31,23,18,
  9,15,11, 3, 21,30,29,17, 14,10,12,13, 24,22,19,28,
};

//-----------------------------------------------------------------------------
// track info

const TrackInfo_t track_info[32] =
{
    {
	0x2490,  0,  4, 21, 0x7d,	// bmg, track, index, slot, music_id
	"MC",
	"Mario Circuit",
	"Marios Piste",
	"castle_course",
	"n_Circuit32_n",
	"n_Circuit32_f",
	{{0x77,0x52,0xbb,0x51,0xed,0xbc,0x4a,0x95,0x37,0x7c,0x0a,0x05,0xb0,0xe0,0xda,0x15,0x03,0x78,0x66,0x25,0x00,0x3d,0x29,0xa0},
	 {0xdb,0x34,0xc4,0xde,0xdd,0x6a,0xa7,0x5c,0xba,0x1d,0x15,0x8e,0xdc,0x1e,0xf4,0x62,0x4e,0xba,0x57,0x55,0x00,0x3d,0x29,0xa0},
	 {0x40,0x38,0x84,0xfe,0x53,0xac,0xe0,0xb8,0x7f,0x87,0xb6,0xd2,0x93,0xb5,0x7c,0x25,0xbb,0x7b,0x91,0x18,0x00,0x2f,0xa0,0x20},
	 {0x9e,0xb3,0x7e,0xcb,0x09,0x22,0x99,0xa3,0xbb,0xe7,0xfa,0x93,0x3f,0xa5,0x44,0x9b,0x3e,0xaa,0x97,0x0b,0x00,0x2f,0xa0,0x20}}
    },
    {
	0x2491,  1,  1, 12, 0x77,	// bmg, track, index, slot, music_id
	"MMM",
	"Moo Moo Meadows",
	"Kuhmuh-Weide",
	"farm_course",
	"n_Farm_n",
	"n_Farm_F",
	{{0x90,0x72,0x0a,0x7d,0x57,0xa7,0xc7,0x6e,0x23,0x47,0x78,0x2f,0x6b,0xde,0x5d,0x22,0x34,0x2f,0xb7,0xdd,0x00,0x46,0xd7,0x80},
	 {0x4a,0x5f,0x65,0xe5,0x9c,0xb3,0xd0,0xcf,0xe8,0x9b,0x3f,0x85,0x97,0x9a,0xc1,0x7c,0xf4,0x09,0xca,0x35,0x00,0x46,0xd7,0x80},
	 {0xd7,0x8a,0xa1,0x48,0x7d,0x88,0xf4,0xcd,0x03,0xb7,0xf9,0x60,0xef,0xd2,0xfc,0xcb,0xd9,0x90,0xf9,0xb3,0x00,0x41,0x30,0x00},
	 {0x83,0x68,0x8b,0x0d,0xe7,0x3a,0xcd,0xf1,0x1f,0xbb,0x56,0x6b,0x29,0x14,0x8b,0x3b,0x6c,0xa9,0xc4,0x0e,0x00,0x41,0x30,0x00}}
    },
    {
	0x2492,  2,  2, 13, 0x79,	// bmg, track, index, slot, music_id
	"MG",
	"Mushroom Gorge",
	"Pilz-Schlucht",
	"kinoko_course",
	"n_Kinoko_n",
	"n_Kinoko_F",
	{{0x0e,0x38,0x03,0x57,0xaf,0xfc,0xfd,0x87,0x22,0x32,0x99,0x94,0x88,0x56,0x99,0xd9,0x92,0x7f,0x82,0x76,0x00,0x4a,0x92,0xa0},
	 {0xc5,0xd4,0xf5,0x06,0x77,0xe7,0x9b,0x5e,0x9c,0xe3,0x89,0x99,0xdb,0xfa,0x2b,0x7e,0xf9,0x85,0xf8,0x10,0x00,0x4a,0x92,0xa0},
	 {0x73,0x5e,0x25,0xbc,0x7b,0xaf,0xd2,0x7f,0x50,0x78,0x30,0x4e,0xb4,0xc8,0x70,0xad,0x14,0x5e,0x05,0xee,0x00,0x44,0x6a,0xa0},
	 {0x90,0x92,0x5f,0xa2,0x7a,0xe9,0x21,0x31,0x3f,0xbd,0x51,0xa8,0x07,0x0c,0xa4,0xd8,0x70,0xce,0xec,0x82,0x00,0x44,0x6a,0xa0}}
    },
    {
	0x2493,  3, 11, 34, 0x8b,	// bmg, track, index, slot, music_id
	"GV",
	"Grumble Volcano",
	"Vulkangrollen",
	"volcano_course",
	"n_Volcano32_n",
	"n_Volcano32_f",
	{{0xac,0xc0,0x88,0x3a,0xe0,0xce,0x78,0x79,0xc6,0xef,0xba,0x20,0xcf,0xe5,0xb5,0x90,0x9b,0xf7,0x84,0x1b,0x00,0x52,0xa5,0x00},
	 {0x68,0x74,0x28,0xc3,0x02,0x80,0xcd,0x27,0x69,0x08,0xeb,0x4b,0x5c,0xb0,0xfa,0x86,0x3e,0xc2,0x28,0xc9,0x00,0x52,0xa5,0x00},
	 {0xb1,0x85,0x76,0x00,0x0f,0x1d,0x80,0xf8,0xf4,0x8f,0x97,0xfe,0xe6,0xce,0x64,0xa6,0x16,0x9d,0x0e,0x34,0x00,0x52,0x8a,0x80},
	 {0xcf,0x60,0x84,0xd1,0x69,0xdb,0x3f,0x71,0xc2,0xa3,0x56,0x4d,0x6d,0xfc,0x70,0x56,0xa1,0xc1,0x42,0xd0,0x00,0x52,0x8a,0x80}}
    },
    {
	0x2494,  4,  3, 14, 0x7b,	// bmg, track, index, slot, music_id
	"TF",
	"Toad's Factory",
	"Toads Fabrik",
	"factory_course",
	"STRM_N_FACTORY_N",
	"STRM_N_FACTORY_F",
	{{0x18,0x96,0xae,0xa4,0x96,0x17,0xa5,0x71,0xc6,0x6f,0xf7,0x78,0xd8,0xf2,0xab,0xbe,0x9e,0x5d,0x74,0x79,0x00,0x44,0x03,0x60},
	 {0xe1,0x15,0x74,0x8e,0x24,0xe6,0xa9,0x32,0x4e,0x24,0xef,0x93,0x93,0xc6,0x0f,0xcc,0x7e,0xce,0x05,0x84,0x00,0x44,0x03,0x60},
	 {0x57,0xdc,0x65,0xa7,0x07,0x72,0x94,0x6d,0xd9,0x48,0xe5,0x31,0x9d,0xe2,0xad,0xc2,0x76,0x28,0x34,0x1b,0x00,0x3e,0x24,0xe0},
	 {0x63,0x40,0x99,0x08,0xd6,0xb2,0x75,0x61,0xbe,0x35,0x65,0xc7,0xe5,0xf3,0xd1,0x6b,0xd5,0x72,0x94,0xb8,0x00,0x3e,0x24,0xe0}}
    },
    {
	0x2495,  5,  5, 22, 0x7f,	// bmg, track, index, slot, music_id
	"CM",
	"Coconut Mall",
	"Kokos-Promenade",
	"shopping_course",
	"n_Shopping32_n",
	"n_Shopping32_f",
	{{0xe4,0xbf,0x36,0x4c,0xb0,0xc5,0x89,0x99,0x07,0x58,0x5d,0x73,0x16,0x21,0xca,0x93,0x0a,0x4e,0xf8,0x5c,0x00,0x4a,0x5c,0x40},
	 {0x0c,0xaa,0x76,0x8f,0xfa,0x35,0x78,0x3b,0xbc,0xb6,0xf7,0xf0,0xdd,0xfa,0xc1,0x36,0xc1,0x5c,0xc0,0x08,0x00,0x4a,0x5c,0x40},
	 {0x98,0x5c,0x6f,0x61,0x6b,0x52,0xac,0x72,0x5e,0x19,0x7f,0x2e,0x7b,0xab,0x6a,0x32,0x21,0x2e,0x40,0x43,0x00,0x3c,0x62,0x40},
	 {0xde,0x5d,0x3f,0x40,0x94,0x49,0x7a,0x34,0x91,0x98,0xc3,0xde,0xa8,0x2a,0x99,0x27,0x24,0x43,0x3a,0x55,0x00,0x3c,0x62,0x40}}
    },
    {
	0x2496,  6,  6, 23, 0x81,	// bmg, track, index, slot, music_id
	"DKS",
	"DK Summit",
	"DK Skikane",
	"boardcross_course",
	"n_Snowboard32_n",
	"n_Snowboard32_F",
	{{0xb0,0x2e,0xd7,0x2e,0x00,0xb4,0x00,0x64,0x7b,0xda,0x68,0x45,0xbe,0x38,0x7c,0x47,0xd2,0x51,0xf9,0xd1,0x00,0x36,0x67,0xc0},
	 {0x98,0xdd,0x84,0xb1,0x5b,0xc5,0x8c,0xd1,0x39,0xb2,0xd1,0x91,0xe2,0xaa,0xda,0x25,0x65,0x65,0x8e,0x75,0x00,0x36,0x67,0xc0},
	 {0},
	 {0}}
    },
    {
	0x2497,  7,  7, 24, 0x83,	// bmg, track, index, slot, music_id
	"WGM",
	"Wario's Gold Mine",
	"Warios Goldmine",
	"truck_course",
	"STRM_N_TRUCK_N",
	"STRM_N_TRUCK_F",
	{{0xd1,0xa4,0x53,0xb4,0x3d,0x69,0x20,0xa7,0x85,0x65,0xe6,0x5a,0x45,0x97,0xe3,0x53,0xb1,0x77,0xab,0xd0,0x00,0x3b,0x1a,0x60},
	 {0xba,0xcb,0xdb,0xa9,0x9d,0x9d,0x64,0xb9,0x9c,0xaf,0x59,0xef,0x8f,0x87,0x39,0x7b,0x75,0xe3,0x30,0x91,0x00,0x3b,0x1a,0x60},
	 {0xc6,0xc1,0xef,0x73,0x0a,0x59,0x9b,0xb9,0x2f,0x73,0x08,0xb5,0x3b,0xa3,0x0a,0x9b,0x34,0x30,0x75,0x03,0x00,0x37,0x2f,0xe0},
	 {0xa2,0xb6,0x7e,0xe4,0x6d,0x04,0xd9,0xab,0x89,0x7c,0xfa,0x90,0x0d,0x22,0xbb,0x3c,0xf3,0xd8,0xd3,0xbe,0x00,0x37,0x2f,0xe0}}
    },
    {
	0x2498,  8,  0, 11, 0x7d,	// bmg, track, index, slot, music_id
	"LC",
	"Luigi Circuit",
	"Luigis Piste",
	"beginner_course",
	"n_Circuit32_n",
	"n_Circuit32_f",
	{{0x1a,0xe1,0xa7,0xd8,0x94,0x96,0x0b,0x38,0xe0,0x9e,0x74,0x94,0x37,0x33,0x78,0xd8,0x73,0x05,0xa1,0x63,0x00,0x35,0x92,0xa0},
	 {0x9f,0x80,0x70,0xb1,0x7f,0x51,0x4c,0x9f,0x0c,0x08,0x32,0x7b,0xdd,0x4b,0x21,0xb0,0x65,0xbc,0x62,0xbb,0x00,0x35,0x92,0xa0},
	 {0x51,0x07,0x6d,0x0b,0x43,0xb7,0x47,0xce,0xb2,0x17,0xcd,0xc8,0x9f,0x91,0x86,0xa5,0xa4,0xc9,0xa0,0xce,0x00,0x30,0x25,0xa0},
	 {0x02,0x6e,0x04,0x8e,0x39,0xbd,0x41,0x84,0x2e,0xab,0xcb,0x99,0x7c,0x50,0x2c,0x55,0xd9,0x54,0x4e,0x1b,0x00,0x30,0x25,0xa0}}
    },
    {
	0x2499,  9,  8, 31, 0x87,	// bmg, track, index, slot, music_id
	"DC",
	"Daisy Circuit",
	"Daisys Piste",
	"senior_course",
	"n_Daisy32_n",
	"n_Daisy32_f",
	{{0x72,0xd0,0x24,0x1c,0x75,0xbe,0x4a,0x5e,0xbd,0x24,0x2b,0x9d,0x8d,0x89,0xb1,0xd6,0xfd,0x56,0xbe,0x8f,0x00,0x4d,0xa5,0xa0},
	 {0x8b,0x49,0xdb,0x78,0x58,0x6d,0x00,0x4b,0xa3,0x26,0xeb,0x90,0x7c,0x76,0x11,0x28,0x61,0xc1,0xb4,0x3f,0x00,0x4d,0xa5,0xa0},
	 {0x71,0x47,0xc8,0x74,0x88,0x06,0xaa,0xd8,0x8c,0xaa,0x31,0x62,0x9f,0xb6,0x2b,0x6a,0x54,0x34,0xce,0xad,0x00,0x4d,0x62,0xa0},
	 {0xda,0x9d,0x63,0xa6,0x29,0xb5,0x8d,0x35,0x04,0x32,0x75,0xbc,0x41,0xb3,0xb9,0x7c,0xb6,0xb2,0xfd,0x12,0x00,0x4d,0x62,0xa0}}
    },
    {
	0x249a, 10, 13, 42, 0x8d,	// bmg, track, index, slot, music_id
	"MH",
	"Moonview Highway",
	"Mondblickstraße",
	"ridgehighway_course",
	"STRM_N_RIDGEHIGHWAY_N",
	"STRM_N_RIDGEHIGHWAY_F",
	{{0xb1,0x3c,0x51,0x54,0x75,0xd7,0xda,0x20,0x7d,0xfd,0x5b,0xad,0xd8,0x86,0x98,0x61,0x47,0xb9,0x06,0xff,0x00,0x52,0x9d,0xc0},
	 {0xa4,0xa7,0x62,0x32,0x33,0x95,0x9c,0x03,0x64,0x59,0x52,0xad,0xc8,0xf2,0xb4,0x6b,0xb1,0x63,0xb8,0xd3,0x00,0x52,0x9d,0xc0},
	 {0xbc,0xad,0xf9,0x55,0x4d,0x35,0xe2,0xc9,0xdb,0xd3,0xc6,0x7d,0x66,0xac,0xa9,0x53,0xcb,0x80,0x98,0x6c,0x00,0x53,0x78,0x40},
	 {0x4f,0x06,0x59,0x55,0x63,0x04,0xde,0xe7,0x8c,0x30,0x0e,0xa7,0xae,0x73,0xd9,0xa8,0x82,0x30,0xab,0xfb,0x00,0x53,0x78,0x40}}
    },
    {
	0x249b, 11, 10, 33, 0x8f,	// bmg, track, index, slot, music_id
	"MT",
	"Maple Treeway",
	"Blätterwald",
	"treehouse_course",
	"n_maple_n",
	"n_maple_F",
	{{0x48,0xeb,0xd9,0xd6,0x44,0x13,0xc2,0xb9,0x8d,0x2b,0x92,0xe5,0xef,0xc9,0xb1,0x5e,0xcd,0x76,0xfe,0xe6,0x00,0x47,0x0a,0x60},
	 {0x64,0xb6,0x9e,0x0f,0x2c,0x7d,0xef,0xed,0x9f,0xb2,0xda,0x44,0x2c,0xa5,0xe5,0x6d,0xc9,0x5f,0x9b,0xbd,0x00,0x47,0x0a,0x60},
	 {0},
	 {0}}
    },
    {
	0x249c, 12, 14, 43, 0x91,	// bmg, track, index, slot, music_id
	"BC",
	"Bowser's Castle",
	"Bowsers Festung",
	"koopa_course",
	"STRM_N_KOOPA_N",
	"STRM_N_KOOPA_F",
	{{0xb9,0x82,0x1b,0x14,0xa8,0x93,0x81,0xf9,0xc0,0x15,0x66,0x93,0x53,0xcb,0x24,0xd7,0xdb,0x1b,0xb2,0x5d,0x00,0x4c,0xcb,0x60},
	 {0xfe,0x97,0x46,0x95,0x14,0x1f,0x58,0xcf,0x97,0xc4,0x13,0x42,0xe9,0x77,0xcc,0x29,0x41,0x18,0x23,0x38,0x00,0x4c,0xcb,0x60},
	 {0x6c,0xe3,0xaa,0x4c,0x8f,0x08,0x30,0x22,0x0f,0x34,0x60,0x22,0x2c,0xfd,0x14,0x3b,0xfe,0xfb,0xb0,0xe3,0x00,0x49,0xf9,0x00},
	 {0x6c,0x93,0x2f,0x9c,0x5a,0x3f,0xf3,0xe0,0xf7,0x11,0x0b,0x4c,0x7d,0x30,0x55,0x77,0x77,0xda,0xb8,0xa8,0x00,0x49,0xf9,0x00}}
    },
    {
	0x249d, 13, 15, 44, 0x93,	// bmg, track, index, slot, music_id
	"RR",
	"Rainbow Road",
	"Regenbogen-Boulevard",
	"rainbow_course",
	"n_Rainbow32_n",
	"n_Rainbow32_f",
	{{0xff,0xe5,0x18,0x91,0x5e,0x5f,0xaa,0xa8,0x89,0x05,0x7c,0x8a,0x3d,0x3e,0x43,0x98,0x68,0x57,0x45,0x08,0x00,0x3a,0x6d,0xe0},
	 {0x13,0x7a,0xc8,0x75,0x97,0xd1,0x0d,0x61,0xb5,0x75,0xae,0x80,0x78,0x2e,0x5f,0x9e,0xae,0x80,0xcd,0x55,0x00,0x3a,0x6d,0xe0},
	 {0xfa,0x15,0x45,0x33,0x61,0xcb,0x49,0x3e,0x89,0xf5,0x11,0x24,0xfa,0xd3,0x6e,0xce,0xae,0x7f,0x44,0xb6,0x00,0x2e,0x11,0xe0},
	 {0x04,0xf4,0xcd,0x1b,0xc4,0xf8,0xa5,0x8d,0x97,0x32,0x81,0x29,0x16,0xbf,0x96,0x53,0x03,0x91,0x72,0xea,0x00,0x2e,0x11,0xe0}}
    },
    {
	0x249e, 14, 12, 41, 0x89,	// bmg, track, index, slot, music_id
	"DDR",
	"Dry Dry Ruins",
	"Staubtrockene Ruinen",
	"desert_course",
	"STRM_N_DESERT_N",
	"STRM_N_DESERT_F",
	{{0x38,0x48,0x6c,0x4f,0x70,0x63,0x95,0x77,0x2b,0xd9,0x88,0xc1,0xac,0x5f,0xa3,0x0d,0x27,0xca,0xe0,0x98,0x00,0x34,0xb6,0xe0},
	 {0x7d,0xc1,0xfe,0x1d,0x59,0x58,0x46,0x5a,0x06,0x47,0xe9,0x4b,0x3e,0x34,0x91,0x60,0x86,0x3f,0x3b,0x1f,0x00,0x34,0xb6,0xe0},
	 {0x0c,0x03,0x7e,0xf1,0x3c,0xf1,0x07,0x78,0xc3,0x68,0x8b,0x33,0xd2,0xc3,0x83,0x71,0x87,0x01,0x29,0x04,0x00,0x2f,0x55,0x60},
	 {0xfc,0x59,0xb2,0x49,0xc8,0xf3,0x07,0x1a,0xcb,0x91,0x7f,0x40,0x4b,0x31,0x70,0xa0,0xf0,0xfc,0x5a,0xfb,0x00,0x2f,0x55,0x60}}
    },
    {
	0x249f, 15,  9, 32, 0x85,	// bmg, track, index, slot, music_id
	"KC",
	"Koopa Cape",
	"Koopa-Kap",
	"water_course",
	"STRM_N_WATER_N",
	"STRM_N_WATER_F",
	{{0x52,0xf0,0x1a,0xe3,0xae,0xd1,0xe0,0xfa,0x4c,0x74,0x59,0xa6,0x48,0x49,0x48,0x63,0xe8,0x3a,0x54,0x8c,0x00,0x50,0x73,0x20},
	 {0xe5,0xe9,0xab,0x6b,0x9f,0x5e,0x56,0x34,0x6b,0x9d,0x26,0xd3,0xf1,0xa7,0x3f,0x06,0x7e,0xe9,0x94,0x48,0x00,0x50,0x73,0x20},
	 {0xc7,0x43,0x11,0xe7,0xd2,0x75,0x7e,0x80,0x62,0x11,0x12,0x3d,0xb5,0xec,0x59,0x5e,0x4f,0xca,0x63,0x70,0x00,0x4c,0x6e,0xa0},
	 {0xdf,0x3d,0xfd,0x88,0xb9,0x8b,0xca,0x8e,0x92,0xb5,0x28,0xbb,0x95,0xa2,0x74,0x4c,0xa4,0x56,0x62,0xba,0x00,0x4c,0x6e,0xa0}}
    },
    {
	0x24a0, 16, 16, 51, 0xa5,	// bmg, track, index, slot, music_id
	"gPB",
	"GCN Peach Beach",
	"GCN Peach Beach",
	"old_peach_gc",
	"r_GC_Beach32_n",
	"r_GC_Beach32_f",
	{{0x80,0x14,0x48,0x8a,0x60,0xf4,0x42,0x8e,0xef,0x52,0xd0,0x1f,0x8c,0x58,0x61,0xca,0x95,0x65,0xe1,0xca,0x00,0x2b,0x24,0x60},
	 {0xed,0xc9,0x29,0xfa,0xbc,0x05,0x08,0x3d,0xfa,0xc0,0x01,0xb4,0x2a,0x6e,0x42,0xc5,0x89,0xcf,0x5e,0xc2,0x00,0x2b,0x24,0x60},
	 {0},
	 {0}}
    },
    {
	0x24a1, 17, 27, 74, 0xa7,	// bmg, track, index, slot, music_id
	"gMC",
	"GCN Mario Circuit",
	"GCN Marios Piste",
	"old_mario_gc",
	"r_GC_Circuit32_n",
	"r_GC_Circuit32_f",
	{{0x19,0x41,0xa2,0x9a,0xd2,0xe7,0xb7,0xbb,0xa8,0xa2,0x9e,0x64,0x40,0xc9,0x5e,0xf5,0xcf,0x76,0xb0,0x1d,0x00,0x34,0x81,0x80},
	 {0x81,0x3d,0x42,0x51,0xd3,0xfb,0x1d,0xb7,0xbc,0xb3,0xd7,0x70,0xbd,0x48,0x66,0xd7,0xb0,0xcd,0xe8,0xab,0x00,0x34,0x81,0x80},
	 {0},
	 {0}}
    },
    {
	0x24a2, 18, 23, 64, 0xa9,	// bmg, track, index, slot, music_id
	"gWS",
	"GCN Waluigi Stadium",
	"GCN Waluigi-Arena",
	"old_waluigi_gc",
	"r_GC_Stadium32_n",
	"r_GC_Stadium32_f",
	{{0x41,0x80,0x99,0x82,0x4a,0xf6,0xbf,0x1c,0xd7,0xf8,0xbb,0x44,0xf6,0x1e,0x3a,0x9c,0xc3,0x00,0x7d,0xae,0x00,0x24,0xd9,0x84},
	 {0xed,0x75,0x58,0x4f,0x51,0xb8,0xd5,0xbf,0xf3,0x56,0xd9,0x49,0x76,0x00,0x95,0xea,0xc2,0xf0,0xc1,0x95,0x00,0x24,0xd9,0xa0},
	 {0},
	 {0}}
    },
    {
	0x24a3, 19, 30, 83, 0xab,	// bmg, track, index, slot, music_id
	"gDKM",
	"GCN DK Mountain",
	"GCN DK Bergland",
	"old_donkey_gc",
	"r_GC_Mountain32_n",
	"r_GC_Mountain32_f",
	{{0xb0,0x36,0x86,0x4c,0xf0,0x01,0x6b,0xe0,0x58,0x14,0x49,0xef,0x29,0xfb,0x52,0xb2,0xe5,0x8d,0x78,0xa4,0x00,0x2c,0x3a,0xe0},
	 {0x45,0x9b,0xc6,0x3b,0x4f,0x63,0xfb,0xa9,0x3f,0x5a,0x7c,0x43,0xaa,0xd2,0x1b,0x41,0xa6,0xbf,0x1d,0xb9,0x00,0x2c,0x3a,0xe0},
	 {0},
	 {0}}
    },
    {
	0x24a4, 20, 17, 52, 0xad,	// bmg, track, index, slot, music_id
	"dYF",
	"DS Yoshi Falls",
	"DS Yoshi-Kaskaden",
	"old_falls_ds",
	"r_DS_Jungle32_n",
	"r_DS_Jungle32_f",
	{{0x8c,0x85,0x4b,0x08,0x74,0x17,0xa9,0x24,0x25,0x11,0x0c,0xc7,0x1e,0x23,0xc9,0x44,0xd6,0x99,0x78,0x06,0x00,0x1f,0x3c,0x40},
	 {0x95,0xb5,0x90,0xc1,0x87,0x9d,0x02,0x5c,0x9d,0xce,0x21,0xec,0x5f,0xf7,0x5d,0x37,0xc6,0x7f,0x46,0xe2,0x00,0x1f,0x3c,0x40},
	 {0},
	 {0}}
    },
    {
	0x24a5, 21, 24, 71, 0xb1,	// bmg, track, index, slot, music_id
	"dDH",
	"DS Desert Hills",
	"DS Glühheiße Wüste",
	"old_desert_ds",
	"r_DS_Desert32_n",
	"r_DS_Desert32_f",
	{{0x4e,0xc5,0x38,0x06,0x5f,0xdc,0x8a,0xcf,0x49,0x67,0x43,0x00,0xcb,0xde,0xc5,0xb8,0x0c,0xc0,0x5a,0x0d,0x00,0x24,0x79,0x40},
	 {0x97,0x50,0x3d,0x9b,0xcc,0xed,0x8f,0xc7,0xf0,0x38,0xad,0x54,0x4e,0xb3,0x31,0x8d,0x37,0xd8,0xef,0xed,0x00,0x24,0x79,0x40},
	 {0},
	 {0}}
    },
    {
	0x24a6, 22, 29, 82, 0xb3,	// bmg, track, index, slot, music_id
	"dPG",
	"DS Peach Gardens",
	"DS Peachs Schlossgarten",
	"old_garden_ds",
	"r_DS_Garden32_n",
	"r_DS_Garden32_f",
	{{0xf9,0xa6,0x2b,0xef,0x04,0xcc,0x8f,0x49,0x96,0x33,0xe4,0x02,0x3a,0xcc,0x76,0x75,0xa9,0x27,0x71,0xf0,0x00,0x20,0xe6,0xe0},
	 {0x98,0xdf,0xd0,0x82,0xb8,0x7b,0x32,0x2a,0x27,0x18,0x65,0x83,0xfa,0x0b,0xac,0x37,0x07,0xfd,0x3f,0x2b,0x00,0x20,0xe6,0xe0},
	 {0},
	 {0}}
    },
    {
	0x24a7, 23, 22, 63, 0xaf,	// bmg, track, index, slot, music_id
	"dDS",
	"DS Delfino Square",
	"DS Piazzale Delfino",
	"old_town_ds",
	"r_DS_Town32_n",
	"r_DS_Town32_f",
	{{0xbc,0x03,0x8e,0x16,0x3d,0x21,0xd9,0xa1,0x18,0x1b,0x60,0xcf,0x90,0xb4,0xd0,0x3e,0xfa,0xd9,0xe0,0xc5,0x00,0x3a,0x01,0x20},
	 {0x5b,0x1f,0xa7,0x15,0xda,0xf7,0x86,0x04,0xd4,0xe4,0x80,0xb3,0x75,0x1d,0xda,0x23,0x08,0x85,0x10,0xc9,0x00,0x3a,0x01,0x20},
	 {0},
	 {0}}
    },
    {
	0x24a8, 24, 28, 81, 0x99,	// bmg, track, index, slot, music_id
	"sMC3",
	"SNES Mario Circuit 3",
	"SNES Marios Piste 3",
	"old_mario_sfc",
	"r_SFC_Circuit32_n",
	"r_SFC_Circuit32_f",
	{{0x07,0x71,0x11,0xb9,0x96,0xe5,0xc4,0xf4,0x7d,0x20,0xec,0x29,0xc2,0x93,0x85,0x04,0xb5,0x3a,0x8e,0x76,0x00,0x1b,0xd2,0x60},
	 {0x92,0xed,0xd1,0x50,0xdf,0x28,0x83,0x6a,0x59,0x6d,0x05,0x68,0x2d,0xaf,0x3c,0x49,0x60,0xeb,0x93,0x0f,0x00,0x1b,0xd2,0x60},
	 {0},
	 {0}}
    },
    {
	0x24a9, 25, 18, 53, 0x97,	// bmg, track, index, slot, music_id
	"sGV2",
	"SNES Ghost Valley 2",
	"SNES Geistertal 2",
	"old_obake_sfc",
	"r_SFC_Obake32_n",
	"r_SFC_Obake32_f",
	{{0x07,0x1d,0x69,0x7c,0x4d,0xdb,0x66,0xd3,0xb2,0x10,0xf3,0x6c,0x7b,0xf8,0x78,0x50,0x2e,0x79,0x84,0x5b,0x00,0x16,0x7b,0x60},
	 {0xa0,0x8c,0x47,0x6b,0xc8,0x3a,0x74,0xf9,0x74,0xea,0x3c,0xc1,0x9a,0xad,0x61,0xb3,0xf0,0xc9,0x9f,0xb7,0x00,0x16,0x7b,0x60},
	 {0},
	 {0}}
    },
    {
	0x24aa, 26, 19, 54, 0x9f,	// bmg, track, index, slot, music_id
	"nMR",
	"N64 Mario Raceway",
	"N64 Marios Rennpiste",
	"old_mario_64",
	"r_64_Circuit32_n",
	"r_64_Circuit32_f",
	{{0x49,0x51,0x4e,0x8f,0x74,0xfe,0xa5,0x0e,0x77,0x27,0x3c,0x02,0x97,0x08,0x6d,0x67,0xe5,0x81,0x23,0xe8,0x00,0x23,0x9d,0xa0},
	 {0xb7,0xc1,0xf6,0xfd,0xbb,0xd0,0x20,0xc2,0x6c,0x25,0x30,0xb5,0x5b,0xe4,0xf1,0x0e,0x93,0x01,0x1a,0x84,0x00,0x23,0x9d,0xa0},
	 {0},
	 {0}}
    },
    {
	0x24ab, 27, 20, 61, 0x9d,	// bmg, track, index, slot, music_id
	"nSL",
	"N64 Sherbet Land",
	"N64 Sorbet-Land",
	"old_sherbet_64",
	"r_64_Sherbet32_n",
	"r_64_Sherbet32_f",
	{{0xba,0x9b,0xcf,0xb3,0x73,0x1a,0x6c,0xb1,0x7d,0xba,0x21,0x9a,0x8d,0x37,0xea,0x4d,0x52,0x33,0x22,0x56,0x00,0x27,0xa0,0x80},
	 {0xcb,0xac,0xe1,0xcc,0x59,0xc9,0x8b,0x2b,0x6f,0x26,0xb8,0xec,0xf5,0x5e,0xbd,0x48,0x99,0x4a,0x90,0x7e,0x00,0x27,0xa0,0x80},
	 {0},
	 {0}}
    },
    {
	0x24ac, 28, 31, 84, 0xa3,	// bmg, track, index, slot, music_id
	"nBC",
	"N64 Bowser's Castle",
	"N64 Bowsers Festung",
	"old_koopa_64",
	"r_64_Kuppa32_n",
	"r_64_Kuppa32_f",
	{{0x15,0xb3,0x03,0xb2,0x88,0xf4,0x70,0x7e,0x5d,0x0a,0xf2,0x83,0x67,0xc8,0xce,0x51,0xcd,0xea,0xb4,0x90,0x00,0x27,0x48,0x20},
	 {0x71,0x42,0x36,0x1a,0xb9,0x3d,0x39,0x29,0xf6,0x2a,0xa7,0x15,0x50,0x9f,0xc8,0xd1,0x37,0x9a,0xfb,0xd6,0x00,0x27,0x48,0x20},
	 {0},
	 {0}}
    },
    {
	0x24ad, 29, 26, 73, 0xa1,	// bmg, track, index, slot, music_id
	"nDKJP",
	"N64 DK's Jungle Parkway",
	"N64 DKs Dschungelpark",
	"old_donkey_64",
	"r_64_Jungle32_n",
	"r_64_Jungle32_f",
	{{0x69,0x2d,0x56,0x6b,0x05,0x43,0x4d,0x8c,0x66,0xa5,0x5b,0xdf,0xf4,0x86,0x69,0x8e,0x0f,0xc9,0x60,0x95,0x00,0x2d,0x6e,0x80},
	 {0x44,0x3c,0x13,0xb8,0x51,0x6d,0xf1,0x63,0x08,0x26,0x09,0xc7,0xc2,0x12,0xfb,0x00,0xf4,0x45,0xbf,0x34,0x00,0x2d,0x6e,0x80},
	 {0},
	 {0}}
    },
    {
	0x24ae, 30, 25, 72, 0x9b,	// bmg, track, index, slot, music_id
	"gBC3",
	"GBA Bowser Castle 3",
	"GBA Bowsers Festung 3",
	"old_koopa_gba",
	"r_AGB_Kuppa32_n",
	"r_AGB_Kuppa32_f",
	{{0xa4,0xbe,0xa4,0x1b,0xe8,0x3d,0x81,0x6f,0x79,0x3f,0x3f,0xad,0x97,0xd2,0x68,0xf7,0x1a,0xd9,0x9b,0xf9,0x00,0x1e,0x66,0xc0},
	 {0xb3,0xbd,0x45,0x88,0xb0,0x41,0xf0,0x92,0xd0,0x03,0x0c,0xff,0x23,0x4c,0x56,0x36,0x9f,0x2c,0xb8,0xfd,0x00,0x1e,0x66,0xc0},
	 {0},
	 {0}}
    },
    {
	0x24af, 31, 21, 62, 0x95,	// bmg, track, index, slot, music_id
	"gSGB",
	"GBA Shy Guy Beach",
	"GBA Shy Guy-Strand",
	"old_heyho_gba",
	"r_AGB_Beach32_n",
	"r_AGB_Beach32_f",
	{{0xe8,0xed,0x31,0x60,0x5c,0xc7,0xd6,0x66,0x06,0x91,0x99,0x8f,0x02,0x4e,0xed,0x6b,0xa8,0xb4,0xa3,0x3f,0x00,0x23,0x2a,0x20},
	 {0x32,0x10,0x01,0x53,0x9d,0x47,0x2d,0x25,0xff,0x0f,0x81,0x3a,0x52,0xad,0xb3,0xe4,0x41,0x54,0xed,0x3e,0x00,0x23,0x2a,0x20},
	 {0},
	 {0}}
    },
};

//-----------------------------------------------------------------------------
// track name table

const KeywordTab_t track_name_tab[] =
{
    {  0, "2.1",                    "21",                     0 },
    {  0, "CASTLECOURSE",           "CASTLECOURSE.SZS",       0 },
    {  0, "CASTLE_COURSE",          "CASTLE_COURSE.SZS",      0 },
    {  0, "MARIO CIRCUIT",          "MARIOCIRCUIT",           0 },
    {  0, "MARIOS PISTE",           "MARIOSPISTE",            0 },
    {  0, "MC",                     "T2.1",                   0 },
    {  0, "T21",                    0,                        0 },

    {  1, "1.2",                    "12",                     0 },
    {  1, "FARMCOURSE",             "FARMCOURSE.SZS",         0 },
    {  1, "FARM_COURSE",            "FARM_COURSE.SZS",        0 },
    {  1, "KUHMUH",                 "KUHMUH WEIDE",           0 },
    {  1, "KUHMUH-WEIDE",           "KUHMUHWEIDE",            0 },
    {  1, "MEADOWS",                "MMM",                    0 },
    {  1, "MOO",                    "MOO MEADOWS",            0 },
    {  1, "MOO MOO",                "MOO MOO MEADOWS",        0 },
    {  1, "MOOMEADOWS",             "MOOMOO",                 0 },
    {  1, "MOOMOOMEADOWS",          "NFARMF.BRSTM",           0 },
    {  1, "NFARMN.BRSTM",           "N_FARM_F.BRSTM",         0 },
    {  1, "N_FARM_N.BRSTM",         "T1.2",                   0 },
    {  1, "T12",                    "WEIDE",                  0 },

    {  2, "1.3",                    "13",                     0 },
    {  2, "GORGE",                  "KINOKOCOURSE",           0 },
    {  2, "KINOKOCOURSE.SZS",       "KINOKO_COURSE",          0 },
    {  2, "KINOKO_COURSE.SZS",      "MG",                     0 },
    {  2, "MUSHROOM",               "MUSHROOM GORGE",         0 },
    {  2, "MUSHROOMGORGE",          "NKINOKOF.BRSTM",         0 },
    {  2, "NKINOKON.BRSTM",         "N_KINOKO_F.BRSTM",       0 },
    {  2, "N_KINOKO_N.BRSTM",       "PILZ",                   0 },
    {  2, "PILZ SCHLUCHT",          "PILZ-SCHLUCHT",          0 },
    {  2, "PILZSCHLUCHT",           "SCHLUCHT",               0 },
    {  2, "T1.3",                   "T13",                    0 },

    {  3, "3.4",                    "34",                     0 },
    {  3, "GRUMBLE",                "GRUMBLE VOLCANO",        0 },
    {  3, "GRUMBLEVOLCANO",         "GV",                     0 },
    {  3, "NVOLCANO32F.BRSTM",      "NVOLCANO32N.BRSTM",      0 },
    {  3, "N_VOLCANO32_F.BRSTM",    "N_VOLCANO32_N.BRSTM",    0 },
    {  3, "T3.4",                   "T34",                    0 },
    {  3, "VOLCANO",                "VOLCANOCOURSE",          0 },
    {  3, "VOLCANOCOURSE.SZS",      "VOLCANO_COURSE",         0 },
    {  3, "VOLCANO_COURSE.SZS",     "VULKANGROLLEN",          0 },

    {  4, "1.4",                    "14",                     0 },
    {  4, "FABRIK",                 "FACTORY",                0 },
    {  4, "FACTORYCOURSE",          "FACTORYCOURSE.SZS",      0 },
    {  4, "FACTORY_COURSE",         "FACTORY_COURSE.SZS",     0 },
    {  4, "STRMNFACTORYF.BRSTM",    "STRMNFACTORYN.BRSTM",    0 },
    {  4, "STRM_N_FACTORY_F.BRSTM", "STRM_N_FACTORY_N.BRSTM", 0 },
    {  4, "T1.4",                   "T14",                    0 },
    {  4, "TF",                     "TOAD-S",                 0 },
    {  4, "TOAD-S FACTORY",         "TOADS",                  0 },
    {  4, "TOADS FABRIK",           "TOADSFABRIK",            0 },
    {  4, "TOADSFACTORY",           0,                        0 },

    {  5, "2.2",                    "22",                     0 },
    {  5, "CM",                     "COCONUT",                0 },
    {  5, "COCONUT MALL",           "COCONUTMALL",            0 },
    {  5, "KOKOS",                  "KOKOS PROMENADE",        0 },
    {  5, "KOKOS-PROMENADE",        "KOKOSPROMENADE",         0 },
    {  5, "MALL",                   "NSHOPPING32F.BRSTM",     0 },
    {  5, "NSHOPPING32N.BRSTM",     "N_SHOPPING32_F.BRSTM",   0 },
    {  5, "N_SHOPPING32_N.BRSTM",   "PROMENADE",              0 },
    {  5, "SHOPPINGCOURSE",         "SHOPPINGCOURSE.SZS",     0 },
    {  5, "SHOPPING_COURSE",        "SHOPPING_COURSE.SZS",    0 },
    {  5, "T2.2",                   "T22",                    0 },

    {  6, "2.3",                    "23",                     0 },
    {  6, "BOARDCROSSCOURSE",       "BOARDCROSSCOURSE.SZS",   0 },
    {  6, "BOARDCROSS_COURSE",      "BOARDCROSS_COURSE.SZS",  0 },
    {  6, "DK SKIKANE",             "DK SUMMIT",              0 },
    {  6, "DKS",                    "DKSKIKANE",              0 },
    {  6, "DKSUMMIT",               "DS",                     0 },
    {  6, "NSNOWBOARD32F.BRSTM",    "NSNOWBOARD32N.BRSTM",    0 },
    {  6, "N_SNOWBOARD32_F.BRSTM",  "N_SNOWBOARD32_N.BRSTM",  0 },
    {  6, "SKIKANE",                "SUMMIT",                 0 },
    {  6, "T2.3",                   "T23",                    0 },

    {  7, "2.4",                    "24",                     0 },
    {  7, "GOLD",                   "GOLD MINE",              0 },
    {  7, "GOLDMINE",               "MINE",                   0 },
    {  7, "STRMNTRUCKF.BRSTM",      "STRMNTRUCKN.BRSTM",      0 },
    {  7, "STRM_N_TRUCK_F.BRSTM",   "STRM_N_TRUCK_N.BRSTM",   0 },
    {  7, "T2.4",                   "T24",                    0 },
    {  7, "TRUCKCOURSE",            "TRUCKCOURSE.SZS",        0 },
    {  7, "TRUCK_COURSE",           "TRUCK_COURSE.SZS",       0 },
    {  7, "WARIO-S",                "WARIO-S GOLD",           0 },
    {  7, "WARIO-S GOLD MINE",      "WARIOS",                 0 },
    {  7, "WARIOS GOLDMINE",        "WARIOSGOLD",             0 },
    {  7, "WARIOSGOLDMINE",         "WGM",                    0 },

    {  8, "1.1",                    "11",                     0 },
    {  8, "BEGINNERCOURSE",         "BEGINNERCOURSE.SZS",     0 },
    {  8, "BEGINNER_COURSE",        "BEGINNER_COURSE.SZS",    0 },
    {  8, "LC",                     "LUIGI",                  0 },
    {  8, "LUIGI CIRCUIT",          "LUIGICIRCUIT",           0 },
    {  8, "LUIGIS",                 "LUIGIS PISTE",           0 },
    {  8, "LUIGISPISTE",            "T1.1",                   0 },
    {  8, "T11",                    0,                        0 },

    {  9, "3.1",                    "31",                     0 },
    {  9, "DAISY",                  "DAISY CIRCUIT",          0 },
    {  9, "DAISYCIRCUIT",           "DAISYS",                 0 },
    {  9, "DAISYS PISTE",           "DAISYSPISTE",            0 },
    {  9, "DC",                     "NDAISY32F.BRSTM",        0 },
    {  9, "NDAISY32N.BRSTM",        "N_DAISY32_F.BRSTM",      0 },
    {  9, "N_DAISY32_N.BRSTM",      "SENIORCOURSE",           0 },
    {  9, "SENIORCOURSE.SZS",       "SENIOR_COURSE",          0 },
    {  9, "SENIOR_COURSE.SZS",      "T3.1",                   0 },
    {  9, "T31",                    0,                        0 },

    { 10, "4.2",                    "42",                     0 },
    { 10, "HIGHWAY",                "MH",                     0 },
    { 10, "MONDBLICKSTRASSE",       "MOONVIEW",               0 },
    { 10, "MOONVIEW HIGHWAY",       "MOONVIEWHIGHWAY",        0 },
    { 10, "MVH",                    "RIDGEHIGHWAYCOURSE",     0 },
    { 10, "RIDGEHIGHWAYCOURSE.SZS", "RIDGEHIGHWAY_COURSE",    0 },
    { 10, "RIDGEHIGHWAY_COURSE.SZS", "STRMNRIDGEHIGHWAYF.BRSTM", 0 },
    { 10, "STRMNRIDGEHIGHWAYN.BRSTM", "STRM_N_RIDGEHIGHWAY_F.BRSTM", 0 },
    { 10, "STRM_N_RIDGEHIGHWAY_N.BRSTM", "T4.2",                   0 },
    { 10, "T42",                    0,                        0 },

    { 11, "3.3",                    "33",                     0 },
    { 11, "BLAETTERWALD",           "MAPLE",                  0 },
    { 11, "MAPLE TREEWAY",          "MAPLETREEWAY",           0 },
    { 11, "MT",                     "NMAPLEF.BRSTM",          0 },
    { 11, "NMAPLEN.BRSTM",          "N_MAPLE_F.BRSTM",        0 },
    { 11, "N_MAPLE_N.BRSTM",        "T3.3",                   0 },
    { 11, "T33",                    "TREEHOUSECOURSE",        0 },
    { 11, "TREEHOUSECOURSE.SZS",    "TREEHOUSE_COURSE",       0 },
    { 11, "TREEHOUSE_COURSE.SZS",   "TREEWAY",                0 },

    { 12, "4.3",                    "43",                     0 },
    { 12, "BC",                     "BOW.FESTUNG",            0 },
    { 12, "BOWSER-S CASTLE",        "BOWSERS FESTUNG",        0 },
    { 12, "BOWSERSCASTLE",          "BOWSERSFESTUNG",         0 },
    { 12, "KOOPACOURSE",            "KOOPACOURSE.SZS",        0 },
    { 12, "KOOPA_COURSE",           "KOOPA_COURSE.SZS",       0 },
    { 12, "STRMNKOOPAF.BRSTM",      "STRMNKOOPAN.BRSTM",      0 },
    { 12, "STRM_N_KOOPA_F.BRSTM",   "STRM_N_KOOPA_N.BRSTM",   0 },
    { 12, "T4.3",                   "T43",                    0 },

    { 13, "4.4",                    "44",                     0 },
    { 13, "BLVD",                   "NRAINBOW32F.BRSTM",      0 },
    { 13, "NRAINBOW32N.BRSTM",      "N_RAINBOW32_F.BRSTM",    0 },
    { 13, "N_RAINBOW32_N.BRSTM",    "RAINBOW",                0 },
    { 13, "RAINBOW ROAD",           "RAINBOWCOURSE",          0 },
    { 13, "RAINBOWCOURSE.SZS",      "RAINBOWROAD",            0 },
    { 13, "RAINBOW_COURSE",         "RAINBOW_COURSE.SZS",     0 },
    { 13, "REGENBOGEN",             "REGENBOGEN BLVD",        0 },
    { 13, "REGENBOGEN-BLVD",        "REGENBOGEN-BOULEVARD",   0 },
    { 13, "REGENBOGENBLVD",         "REGENBOGENBOULEVARD",    0 },
    { 13, "ROAD",                   "RR",                     0 },
    { 13, "T4.4",                   "T44",                    0 },

    { 14, "4.1",                    "41",                     0 },
    { 14, "DDR",                    "DESERTCOURSE",           0 },
    { 14, "DESERTCOURSE.SZS",       "DESERT_COURSE",          0 },
    { 14, "DESERT_COURSE.SZS",      "DRY",                    0 },
    { 14, "DRY DRY",                "DRY DRY RUINS",          0 },
    { 14, "DRY RUINS",              "DRYDRY",                 0 },
    { 14, "DRYDRYRUINS",            "DRYRUINS",               0 },
    { 14, "RUINEN",                 "RUINS",                  0 },
    { 14, "STAUBTROCKENE",          "STAUBTROCKENE RUINEN",   0 },
    { 14, "STAUBTROCKENERUINEN",    "STRMNDESERTF.BRSTM",     0 },
    { 14, "STRMNDESERTN.BRSTM",     "STRM_N_DESERT_F.BRSTM",  0 },
    { 14, "STRM_N_DESERT_N.BRSTM",  "T4.1",                   0 },
    { 14, "T41",                    "TROCKENE",               0 },
    { 14, "TROCKENE RUINEN",        "TROCKENERUINEN",         0 },

    { 15, "3.2",                    "32",                     0 },
    { 15, "CAPE",                   "KAP",                    0 },
    { 15, "KC",                     "KOOPA",                  0 },
    { 15, "KOOPA CAPE",             "KOOPA KAP",              0 },
    { 15, "KOOPA-KAP",              "KOOPACAPE",              0 },
    { 15, "KOOPAKAP",               "STRMNWATERF.BRSTM",      0 },
    { 15, "STRMNWATERN.BRSTM",      "STRM_N_WATER_F.BRSTM",   0 },
    { 15, "STRM_N_WATER_N.BRSTM",   "T3.2",                   0 },
    { 15, "T32",                    "WATERCOURSE",            0 },
    { 15, "WATERCOURSE.SZS",        "WATER_COURSE",           0 },
    { 15, "WATER_COURSE.SZS",       0,                        0 },

    { 16, "5.1",                    "51",                     0 },
    { 16, "GCN PEACH",              "GCN PEACH BEACH",        0 },
    { 16, "GCNPEACH",               "GCNPEACHBEACH",          0 },
    { 16, "GPB",                    "OLDPEACHGC",             0 },
    { 16, "OLDPEACHGC.SZS",         "OLD_PEACH_GC",           0 },
    { 16, "OLD_PEACH_GC.SZS",       "PEACH BEACH",            0 },
    { 16, "PEACHBEACH",             "RGCBEACH32F.BRSTM",      0 },
    { 16, "RGCBEACH32N.BRSTM",      "RPB",                    0 },
    { 16, "R_GC_BEACH32_F.BRSTM",   "R_GC_BEACH32_N.BRSTM",   0 },
    { 16, "T5.1",                   "T51",                    0 },

    { 17, "7.4",                    "74",                     0 },
    { 17, "GC",                     "GC MARIOS",              0 },
    { 17, "GC MARIOS PISTE",        "GCMARIOS",               0 },
    { 17, "GCMARIOSPISTE",          "GCN MARIO",              0 },
    { 17, "GCN MARIO CIRCUIT",      "GCN MARIOS",             0 },
    { 17, "GCN MARIOS PISTE",       "GCNMARIO",               0 },
    { 17, "GCNMARIOCIRCUIT",        "GCNMARIOS",              0 },
    { 17, "GCNMARIOSPISTE",         "GMC",                    0 },
    { 17, "OLDMARIOGC",             "OLDMARIOGC.SZS",         0 },
    { 17, "OLD_MARIO_GC",           "OLD_MARIO_GC.SZS",       0 },
    { 17, "RGCCIRCUIT32F.BRSTM",    "RGCCIRCUIT32N.BRSTM",    0 },
    { 17, "RMC",                    "R_GC_CIRCUIT32_F.BRSTM", 0 },
    { 17, "R_GC_CIRCUIT32_N.BRSTM", "T7.4",                   0 },
    { 17, "T74",                    0,                        0 },

    { 18, "6.4",                    "64",                     0 },
    { 18, "ARENA",                  "GCN WALUIGI",            0 },
    { 18, "GCN WALUIGI STADIUM",    "GCN WALUIGI-ARENA",      0 },
    { 18, "GCNWALUIGI",             "GCNWALUIGIARENA",        0 },
    { 18, "GCNWALUIGISTADIUM",      "GWS",                    0 },
    { 18, "OLDWALUIGIGC",           "OLDWALUIGIGC.SZS",       0 },
    { 18, "OLD_WALUIGI_GC",         "OLD_WALUIGI_GC.SZS",     0 },
    { 18, "RGCSTADIUM32F.BRSTM",    "RGCSTADIUM32N.BRSTM",    0 },
    { 18, "RWS",                    "R_GC_STADIUM32_F.BRSTM", 0 },
    { 18, "R_GC_STADIUM32_N.BRSTM", "STADIUM",                0 },
    { 18, "T6.4",                   "T64",                    0 },
    { 18, "WALUIGI",                "WALUIGI ARENA",          0 },
    { 18, "WALUIGI STADIUM",        "WALUIGI-ARENA",          0 },
    { 18, "WALUIGIARENA",           "WALUIGISTADIUM",         0 },

    { 19, "8.3",                    "83",                     0 },
    { 19, "BERGLAND",               "DK BERGLAND",            0 },
    { 19, "DK MOUNTAIN",            "DKBERGLAND",             0 },
    { 19, "DKMOUNTAIN",             "GCN DK",                 0 },
    { 19, "GCN DK BERGLAND",        "GCN DK MOUNTAIN",        0 },
    { 19, "GCNDK",                  "GCNDKBERGLAND",          0 },
    { 19, "GCNDKMOUNTAIN",          "GDKM",                   0 },
    { 19, "MOUNTAIN",               "OLDDONKEYGC",            0 },
    { 19, "OLDDONKEYGC.SZS",        "OLD_DONKEY_GC",          0 },
    { 19, "OLD_DONKEY_GC.SZS",      "RDKM",                   0 },
    { 19, "RGCMOUNTAIN32F.BRSTM",   "RGCMOUNTAIN32N.BRSTM",   0 },
    { 19, "R_GC_MOUNTAIN32_F.BRSTM", "R_GC_MOUNTAIN32_N.BRSTM", 0 },
    { 19, "T8.3",                   "T83",                    0 },

    { 20, "5.2",                    "52",                     0 },
    { 20, "DS YOSHI",               "DS YOSHI FALLS",         0 },
    { 20, "DS YOSHI-KASKADEN",      "DSYOSHI",                0 },
    { 20, "DSYOSHIFALLS",           "DSYOSHIKASKADEN",        0 },
    { 20, "DYF",                    "FALLS",                  0 },
    { 20, "KASKADEN",               "OLDFALLSDS",             0 },
    { 20, "OLDFALLSDS.SZS",         "OLD_FALLS_DS",           0 },
    { 20, "OLD_FALLS_DS.SZS",       "RDSJUNGLE32F.BRSTM",     0 },
    { 20, "RDSJUNGLE32N.BRSTM",     "RYF",                    0 },
    { 20, "R_DS_JUNGLE32_F.BRSTM",  "R_DS_JUNGLE32_N.BRSTM",  0 },
    { 20, "T5.2",                   "T52",                    0 },
    { 20, "YOSHI",                  "YOSHI FALLS",            0 },
    { 20, "YOSHI KASKADEN",         "YOSHI-KASKADEN",         0 },
    { 20, "YOSHIFALLS",             "YOSHIKASKADEN",          0 },

    { 21, "7.1",                    "71",                     0 },
    { 21, "DDH",                    "DESERT",                 0 },
    { 21, "DESERT HILLS",           "DESERTHILLS",            0 },
    { 21, "DS DESERT",              "DS DESERT HILLS",        0 },
    { 21, "DS GLUEHHEISSE",         "DS GLUEHHEISSE WUESTE",  0 },
    { 21, "DSDESERT",               "DSDESERTHILLS",          0 },
    { 21, "DSGLUEHHEISSE",          "DSGLUEHHEISSEWUESTE",    0 },
    { 21, "GLUEHHEISSE",            "GLUEHHEISSE WUESTE",     0 },
    { 21, "GLUEHHEISSEWUESTE",      "HEISSE",                 0 },
    { 21, "HEISSE WUESTE",          "HEISSEWUESTE",           0 },
    { 21, "HILLS",                  "OLDDESERTDS",            0 },
    { 21, "OLDDESERTDS.SZS",        "OLD_DESERT_DS",          0 },
    { 21, "OLD_DESERT_DS.SZS",      "RDH",                    0 },
    { 21, "RDSDESERT32F.BRSTM",     "RDSDESERT32N.BRSTM",     0 },
    { 21, "R_DS_DESERT32_F.BRSTM",  "R_DS_DESERT32_N.BRSTM",  0 },
    { 21, "T7.1",                   "T71",                    0 },
    { 21, "WUESTE",                 0,                        0 },

    { 22, "8.2",                    "82",                     0 },
    { 22, "DPG",                    "DS PEACH",               0 },
    { 22, "DS PEACH GARDENS",       "DS PEACHS",              0 },
    { 22, "DS PEACHS SCHLOSSGARTEN", "DSPEACH",                0 },
    { 22, "DSPEACHGARDENS",         "DSPEACHS",               0 },
    { 22, "DSPEACHSSCHLOSSGARTEN",  "GARDENS",                0 },
    { 22, "OLDGARDENDS",            "OLDGARDENDS.SZS",        0 },
    { 22, "OLD_GARDEN_DS",          "OLD_GARDEN_DS.SZS",      0 },
    { 22, "PEACH GARDENS",          "PEACHGARDENS",           0 },
    { 22, "PEACHS",                 "PEACHS SCHLOSSGARTEN",   0 },
    { 22, "PEACHSSCHLOSSGARTEN",    "RDSGARDEN32F.BRSTM",     0 },
    { 22, "RDSGARDEN32N.BRSTM",     "RPG",                    0 },
    { 22, "R_DS_GARDEN32_F.BRSTM",  "R_DS_GARDEN32_N.BRSTM",  0 },
    { 22, "SCHLOSSGARTEN",          "T8.2",                   0 },
    { 22, "T82",                    0,                        0 },

    { 23, "6.3",                    "63",                     0 },
    { 23, "DDS",                    "DELFINO",                0 },
    { 23, "DELFINO SQUARE",         "DELFINOSQUARE",          0 },
    { 23, "DS DELFINO",             "DS DELFINO SQUARE",      0 },
    { 23, "DS PIAZZALE",            "DS PIAZZALE DELFINO",    0 },
    { 23, "DSDELFINO",              "DSDELFINOSQUARE",        0 },
    { 23, "DSPIAZZALE",             "DSPIAZZALEDELFINO",      0 },
    { 23, "OLDTOWNDS",              "OLDTOWNDS.SZS",          0 },
    { 23, "OLD_TOWN_DS",            "OLD_TOWN_DS.SZS",        0 },
    { 23, "PIAZ",                   "PIAZ.DELFINO",           0 },
    { 23, "PIAZZALE",               "PIAZZALE DELFINO",       0 },
    { 23, "PIAZZALEDELFINO",        "RDS",                    0 },
    { 23, "RDSTOWN32F.BRSTM",       "RDSTOWN32N.BRSTM",       0 },
    { 23, "R_DS_TOWN32_F.BRSTM",    "R_DS_TOWN32_N.BRSTM",    0 },
    { 23, "SQUARE",                 "T6.3",                   0 },
    { 23, "T63",                    0,                        0 },

    { 24, "8.1",                    "81",                     0 },
    { 24, "CIRCUIT 3",              "CIRCUIT3",               0 },
    { 24, "MARIO CIRCUIT 3",        "MARIOCIRCUIT3",          0 },
    { 24, "MARIOS PISTE 3",         "MARIOSPISTE3",           0 },
    { 24, "OLDMARIOSFC",            "OLDMARIOSFC.SZS",        0 },
    { 24, "OLD_MARIO_SFC",          "OLD_MARIO_SFC.SZS",      0 },
    { 24, "PISTE 3",                "PISTE3",                 0 },
    { 24, "RMC3",                   "RSFCCIRCUIT32F.BRSTM",   0 },
    { 24, "RSFCCIRCUIT32N.BRSTM",   "R_SFC_CIRCUIT32_F.BRSTM", 0 },
    { 24, "R_SFC_CIRCUIT32_N.BRSTM", "SMC3",                   0 },
    { 24, "SNES MARIO",             "SNES MARIO CIRCUIT",     0 },
    { 24, "SNES MARIO CIRCUIT 3",   "SNES MARIOS",            0 },
    { 24, "SNES MARIOS PISTE",      "SNES MARIOS PISTE 3",    0 },
    { 24, "SNESMARIO",              "SNESMARIOCIRCUIT",       0 },
    { 24, "SNESMARIOCIRCUIT3",      "SNESMARIOS",             0 },
    { 24, "SNESMARIOSPISTE",        "SNESMARIOSPISTE3",       0 },
    { 24, "T8.1",                   "T81",                    0 },

    { 25, "5.3",                    "53",                     0 },
    { 25, "GEISTERTAL",             "GEISTERTAL 2",           0 },
    { 25, "GEISTERTAL2",            "GHOST",                  0 },
    { 25, "GHOST VALLEY",           "GHOST VALLEY 2",         0 },
    { 25, "GHOSTVALLEY",            "GHOSTVALLEY2",           0 },
    { 25, "OLDOBAKESFC",            "OLDOBAKESFC.SZS",        0 },
    { 25, "OLD_OBAKE_SFC",          "OLD_OBAKE_SFC.SZS",      0 },
    { 25, "RGV2",                   "RSFCOBAKE32F.BRSTM",     0 },
    { 25, "RSFCOBAKE32N.BRSTM",     "R_SFC_OBAKE32_F.BRSTM",  0 },
    { 25, "R_SFC_OBAKE32_N.BRSTM",  "SGV2",                   0 },
    { 25, "SNES GEISTERTAL",        "SNES GEISTERTAL 2",      0 },
    { 25, "SNES GHOST",             "SNES GHOST VALLEY",      0 },
    { 25, "SNES GHOST VALLEY 2",    "SNESGEISTERTAL",         0 },
    { 25, "SNESGEISTERTAL2",        "SNESGHOST",              0 },
    { 25, "SNESGHOSTVALLEY",        "SNESGHOSTVALLEY2",       0 },
    { 25, "T5.3",                   "T53",                    0 },
    { 25, "VALLEY",                 "VALLEY 2",               0 },
    { 25, "VALLEY2",                0,                        0 },

    { 26, "5.4",                    "54",                     0 },
    { 26, "MAR",                    "MAR.RENNPISTE",          0 },
    { 26, "MARIO RACEWAY",          "MARIORACEWAY",           0 },
    { 26, "MARIOS RENNPISTE",       "MARIOSRENNPISTE",        0 },
    { 26, "N64 MARIO",              "N64 MARIO RACEWAY",      0 },
    { 26, "N64 MARIOS",             "N64 MARIOS RENNPISTE",   0 },
    { 26, "N64MARIO",               "N64MARIORACEWAY",        0 },
    { 26, "N64MARIOS",              "N64MARIOSRENNPISTE",     0 },
    { 26, "NMR",                    "OLDMARIO64",             0 },
    { 26, "OLDMARIO64.SZS",         "OLD_MARIO_64",           0 },
    { 26, "OLD_MARIO_64.SZS",       "R64CIRCUIT32F.BRSTM",    0 },
    { 26, "R64CIRCUIT32N.BRSTM",    "RACEWAY",                0 },
    { 26, "RENNPISTE",              "RMR",                    0 },
    { 26, "R_64_CIRCUIT32_F.BRSTM", "R_64_CIRCUIT32_N.BRSTM", 0 },
    { 26, "T5.4",                   "T54",                    0 },

    { 27, "6.1",                    "61",                     0 },
    { 27, "LAND",                   "N64 SHERBET",            0 },
    { 27, "N64 SHERBET LAND",       "N64 SORBET-LAND",        0 },
    { 27, "N64SHERBET",             "N64SHERBETLAND",         0 },
    { 27, "N64SORBETLAND",          "NSL",                    0 },
    { 27, "OLDSHERBET64",           "OLDSHERBET64.SZS",       0 },
    { 27, "OLD_SHERBET_64",         "OLD_SHERBET_64.SZS",     0 },
    { 27, "R64SHERBET32F.BRSTM",    "R64SHERBET32N.BRSTM",    0 },
    { 27, "RSL",                    "R_64_SHERBET32_F.BRSTM", 0 },
    { 27, "R_64_SHERBET32_N.BRSTM", "SHERBET",                0 },
    { 27, "SHERBET LAND",           "SHERBETLAND",            0 },
    { 27, "SORBET",                 "SORBET LAND",            0 },
    { 27, "SORBET-LAND",            "SORBETLAND",             0 },
    { 27, "T6.1",                   "T61",                    0 },

    { 28, "8.4",                    "84",                     0 },
    { 28, "BOW.FESTUNG",            "N64 BOW.FESTUNG",        0 },
    { 28, "N64 BOWSER-S",           "N64 BOWSER-S CASTLE",    0 },
    { 28, "N64 BOWSERS",            "N64 BOWSERS FESTUNG",    0 },
    { 28, "N64BOW.FESTUNG",         "N64BOWSERS",             0 },
    { 28, "N64BOWSERSCASTLE",       "N64BOWSERSFESTUNG",      0 },
    { 28, "NBC",                    "OLDKOOPA64",             0 },
    { 28, "OLDKOOPA64.SZS",         "OLD_KOOPA_64",           0 },
    { 28, "OLD_KOOPA_64.SZS",       "R64KUPPA32F.BRSTM",      0 },
    { 28, "R64KUPPA32N.BRSTM",      "RBC",                    0 },
    { 28, "R_64_KUPPA32_F.BRSTM",   "R_64_KUPPA32_N.BRSTM",   0 },
    { 28, "T8.4",                   "T84",                    0 },

    { 29, "7.3",                    "73",                     0 },
    { 29, "DK-S",                   "DK-S JUNGLE",            0 },
    { 29, "DK-S JUNGLE PARKWAY",    "DKS",                    0 },
    { 29, "DKS DSCHUNGELPARK",      "DKSDSCHUNGELPARK",       0 },
    { 29, "DKSJUNGLE",              "DKSJUNGLEPARKWAY",       0 },
    { 29, "DSCHUNGELPARK",          "JUNGLE",                 0 },
    { 29, "JUNGLE PARKWAY",         "JUNGLEPARKWAY",          0 },
    { 29, "N64 DK-S",               "N64 DK-S JUNGLE",        0 },
    { 29, "N64 DK-S JUNGLE PARKWAY", "N64 DKS",                0 },
    { 29, "N64 DKS DSCHUNGELPARK",  "N64DKS",                 0 },
    { 29, "N64DKSDSCHUNGELPARK",    "N64DKSJUNGLE",           0 },
    { 29, "N64DKSJUNGLEPARKWAY",    "NDKJP",                  0 },
    { 29, "OLDDONKEY64",            "OLDDONKEY64.SZS",        0 },
    { 29, "OLD_DONKEY_64",          "OLD_DONKEY_64.SZS",      0 },
    { 29, "PARKWAY",                "R64JUNGLE32F.BRSTM",     0 },
    { 29, "R64JUNGLE32N.BRSTM",     "RDJP",                   0 },
    { 29, "RDKJP",                  "R_64_JUNGLE32_F.BRSTM",  0 },
    { 29, "R_64_JUNGLE32_N.BRSTM",  "T7.3",                   0 },
    { 29, "T73",                    0,                        0 },

    { 30, "7.2",                    "72",                     0 },
    { 30, "BOW.FESTUNG 3",          "BOW.FESTUNG3",           0 },
    { 30, "BOWSER",                 "BOWSER CASTLE",          0 },
    { 30, "BOWSER CASTLE 3",        "BOWSERCASTLE",           0 },
    { 30, "BOWSERCASTLE3",          "BOWSERS FESTUNG 3",      0 },
    { 30, "BOWSERSFESTUNG3",        "CASTLE 3",               0 },
    { 30, "CASTLE3",                "FESTUNG 3",              0 },
    { 30, "FESTUNG3",               "GBA BOWSER",             0 },
    { 30, "GBA BOWSER CASTLE",      "GBA BOWSER CASTLE 3",    0 },
    { 30, "GBA BOWSERS",            "GBA BOWSERS FESTUNG",    0 },
    { 30, "GBA BOWSERS FESTUNG 3",  "GBABOWSER",              0 },
    { 30, "GBABOWSERCASTLE",        "GBABOWSERCASTLE3",       0 },
    { 30, "GBABOWSERS",             "GBABOWSERSFESTUNG",      0 },
    { 30, "GBABOWSERSFESTUNG3",     "GBC3",                   0 },
    { 30, "OLDKOOPAGBA",            "OLDKOOPAGBA.SZS",        0 },
    { 30, "OLD_KOOPA_GBA",          "OLD_KOOPA_GBA.SZS",      0 },
    { 30, "RAGBKUPPA32F.BRSTM",     "RAGBKUPPA32N.BRSTM",     0 },
    { 30, "RBC3",                   "R_AGB_KUPPA32_F.BRSTM",  0 },
    { 30, "R_AGB_KUPPA32_N.BRSTM",  "T7.2",                   0 },
    { 30, "T72",                    0,                        0 },

    { 31, "6.2",                    "62",                     0 },
    { 31, "GBA SHY",                "GBA SHY GUY",            0 },
    { 31, "GBA SHY GUY BEACH",      "GBA SHY GUY-STRAND",     0 },
    { 31, "GBASHY",                 "GBASHYGUY",              0 },
    { 31, "GBASHYGUYBEACH",         "GBASHYGUYSTRAND",        0 },
    { 31, "GSGB",                   "GUY",                    0 },
    { 31, "GUY BEACH",              "GUY STRAND",             0 },
    { 31, "GUY-STRAND",             "GUYBEACH",               0 },
    { 31, "GUYSTRAND",              "OLDHEYHOGBA",            0 },
    { 31, "OLDHEYHOGBA.SZS",        "OLD_HEYHO_GBA",          0 },
    { 31, "OLD_HEYHO_GBA.SZS",      "RAGBBEACH32F.BRSTM",     0 },
    { 31, "RAGBBEACH32N.BRSTM",     "RSGB",                   0 },
    { 31, "R_AGB_BEACH32_F.BRSTM",  "R_AGB_BEACH32_N.BRSTM",  0 },
    { 31, "SHY",                    "SHY GUY",                0 },
    { 31, "SHY GUY BEACH",          "SHY GUY STRAND",         0 },
    { 31, "SHY GUY-STRAND",         "SHYGUY",                 0 },
    { 31, "SHYGUYBEACH",            "SHYGUYSTRAND",           0 },
    { 31, "STRAND",                 "T6.2",                   0 },
    { 31, "T62",                    0,                        0 },

    {0,0,0,0}
};

//-----------------------------------------------------------------------------

int ScanTrack ( ccp name )
{
    const KeywordTab_t * cmd = ScanKeyword(0,name,track_name_tab);
    return cmd ? cmd->id : -1;
}

//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////   arenas   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// arena positions

const u32 arena_pos_default[10] =
{
	1,0,3,2,4, 7,8,9,5,6
};

u32 arena_pos[10] =
{
	1,0,3,2,4, 7,8,9,5,6
};

u32 arena_pos_r[10] =
{
	1,0,3,2,4, 8,9,5,6,7
};

//-----------------------------------------------------------------------------
// arena info

const TrackInfo_t arena_info[10] =
{
    {
	0x24b8, 32,  1, 12, 0xb5,	// bmg, track, index, slot, music_id
	"aDP",
	"Delfino Pier",
	"Delfino-Pier",
	"venice_battle",
	"n_venice_n",
	"n_venice_F",
	{{0x45,0x6d,0x88,0x00,0xee,0x7b,0x1b,0x1e,0x2d,0x2d,0xfe,0x4d,0x46,0x54,0xa9,0xc3,0xbb,0x9b,0xd3,0x0f,0x00,0x30,0xf3,0x40},
	 {0x1a,0x26,0x94,0x74,0x30,0xfd,0x98,0xbf,0x3b,0x50,0x2b,0x2c,0x07,0x8b,0x53,0x7f,0x33,0x36,0x13,0xa5,0x00,0x30,0xf3,0x40},
	 {0x90,0x37,0x58,0x78,0x32,0x89,0x86,0xf8,0x26,0xe3,0x6c,0x8a,0x79,0xf4,0x65,0x7a,0x68,0x01,0xad,0x54,0x00,0x31,0xfa,0xc0},
	 {0x42,0x3a,0xce,0x92,0x78,0xa4,0x43,0xb8,0xd6,0x1b,0x08,0x10,0x2a,0x74,0x11,0xbc,0xae,0x45,0xf4,0x98,0x00,0x31,0xfa,0xc0}}
    },
    {
	0x24b9, 33,  0, 11, 0xb7,	// bmg, track, index, slot, music_id
	"aBP",
	"Block Plaza",
	"Block-Plaza",
	"block_battle",
	"n_block_n",
	"n_block_F",
	{{0x90,0x47,0xf6,0xe9,0xb7,0x7c,0x6a,0x44,0xac,0xcb,0x46,0xc2,0x23,0x76,0x09,0xb8,0x0e,0x45,0x9f,0xdc,0x00,0x35,0x2c,0xe0},
	 {0xd9,0x7d,0xe5,0x43,0x39,0xd4,0x44,0x46,0xee,0xe3,0xba,0x2c,0xf5,0xe6,0x8e,0xb1,0xca,0x36,0xf8,0x77,0x00,0x35,0x2c,0xe0},
	 {0},
	 {0}}
    },
    {
	0x24ba, 34,  3, 14, 0xbb,	// bmg, track, index, slot, music_id
	"aCCW",
	"Chain Chomp Wheel",
	"Kettenhund-Roulette",
	"casino_battle",
	"n_casino_n",
	"n_casino_F",
	{{0xe0,0x4e,0xec,0x80,0xbb,0xe0,0xaa,0xf8,0x17,0x7e,0x0c,0x70,0x7c,0x05,0xed,0x54,0x1f,0xb5,0x0a,0x23,0x00,0x42,0xf1,0x00},
	 {0x5a,0xe5,0x2d,0x71,0xd8,0x68,0x4b,0xba,0x21,0x1e,0x69,0xc4,0x14,0xfb,0xf0,0x18,0xae,0x99,0xb4,0xa8,0x00,0x42,0xf1,0x00},
	 {0x46,0xe9,0x1f,0xf4,0xd2,0x06,0x77,0x20,0x31,0x81,0x1b,0xe9,0x93,0x51,0x44,0x2f,0x45,0x63,0xaa,0x31,0x00,0x3f,0x7f,0x80},
	 {0x86,0x36,0x00,0x41,0x5e,0xab,0xa9,0xb5,0x39,0xbe,0x90,0xb2,0x48,0x3c,0x4a,0x50,0x59,0xb1,0x82,0x1a,0x00,0x3f,0x7f,0x80}}
    },
    {
	0x24bb, 35,  2, 13, 0xb9,	// bmg, track, index, slot, music_id
	"aFS",
	"Funky Stadium",
	"Funky-Stadion",
	"skate_battle",
	"n_skate_n",
	"n_skate_F",
	{{0x70,0x1d,0xa1,0x6b,0x75,0x95,0x05,0x0a,0xc1,0x52,0x55,0x79,0xaa,0xba,0x40,0x78,0x3e,0x49,0xcf,0x8b,0x00,0x37,0x67,0x20},
	 {0xa7,0xb5,0xfd,0x0a,0x0e,0x88,0x3d,0x3b,0x17,0xd1,0x41,0x97,0x0f,0x62,0x6f,0x66,0x15,0x34,0x1c,0x90,0x00,0x37,0x67,0x20},
	 {0},
	 {0}}
    },
    {
	0x24bc, 36,  4, 15, 0xbd,	// bmg, track, index, slot, music_id
	"aTD",
	"Thwomp Desert",
	"Steinwüste",
	"sand_battle",
	"n_ryuusa_n",
	"n_ryuusa_F",
	{{0xc4,0x9d,0x22,0x62,0xb0,0xcc,0x65,0xb6,0x49,0xfe,0x8a,0xdd,0x2d,0xaf,0x91,0xa3,0xcf,0x73,0xcc,0x45,0x00,0x2b,0x80,0x40},
	 {0x4a,0x68,0x1c,0x07,0x07,0x81,0x6f,0xb6,0xe1,0x03,0x5e,0x41,0x79,0xd8,0xa1,0x1c,0x4c,0x1e,0xab,0x0f,0x00,0x2b,0x80,0x40},
	 {0x37,0x21,0xd5,0x20,0x9e,0xc2,0xce,0x5f,0x02,0x2a,0xc3,0x94,0x4f,0xd4,0x55,0xaf,0xd1,0xa2,0xab,0x16,0x00,0x29,0xd9,0xc0},
	 {0x98,0x04,0x2a,0x57,0x0c,0x9a,0x36,0x87,0xa8,0x96,0x42,0x8d,0xe5,0x1c,0xb9,0x9f,0x46,0x54,0x67,0xf0,0x00,0x29,0xd9,0xc0}}
    },
    {
	0x24bd, 37,  8, 24, 0xbf,	// bmg, track, index, slot, music_id
	"agCL",
	"GCN Cookie Land",
	"GCN Keks-Land",
	"old_CookieLand_gc",
	"r_GC_Battle32_n",
	"r_GC_Battle32_F",
	{{0x26,0x77,0x6d,0x2b,0xd9,0x7b,0xae,0x6d,0x6a,0x8e,0x5f,0xa9,0xc1,0xfd,0x59,0xae,0x7c,0xa5,0xde,0xe5,0x00,0x0d,0x5d,0x00},
	 {0x95,0x2d,0xb0,0xfe,0x10,0x10,0xfb,0xcb,0xce,0x5a,0xda,0x22,0xd3,0x85,0xc5,0xf5,0x72,0x4a,0xac,0x9a,0x00,0x0d,0x5d,0x00},
	 {0},
	 {0}}
    },
    {
	0x24be, 38,  9, 25, 0xc1,	// bmg, track, index, slot, music_id
	"adTH",
	"DS Twilight House",
	"DS Finstervilla",
	"old_House_ds",
	"r_ds_battle_n",
	"r_ds_battle_F",
	{{0xb8,0x31,0x2b,0xac,0xb2,0x5e,0xc0,0xf0,0x85,0x60,0xfd,0x95,0xe9,0xed,0xd0,0x8e,0x82,0x1c,0xe6,0xcd,0x00,0x1d,0x59,0x00},
	 {0x66,0x19,0x10,0x53,0x13,0x4c,0xf8,0x77,0x57,0x90,0x28,0x95,0x49,0x97,0x58,0x4a,0xb6,0xc2,0x0a,0x73,0x00,0x1d,0x59,0x00},
	 {0},
	 {0}}
    },
    {
	0x24bf, 39,  5, 21, 0xc3,	// bmg, track, index, slot, music_id
	"asBC4",
	"SNES Battle Course 4",
	"SNES Kampfkurs 4",
	"old_battle4_sfc",
	"r_sfc_battle_n",
	"r_sfc_battle_F",
	{{0x79,0x13,0x71,0x0a,0xfc,0xda,0x69,0xb3,0x59,0xb5,0xf3,0xd6,0x14,0xdd,0xae,0x86,0x14,0x40,0x3e,0xf2,0x00,0x15,0xc1,0x80},
	 {0x3f,0x82,0xaa,0x67,0x67,0xf3,0xc0,0xd6,0x98,0xa3,0x7c,0xde,0x42,0xf9,0xf4,0x93,0xca,0x8b,0xa2,0xfc,0x00,0x15,0xc1,0x80},
	 {0},
	 {0}}
    },
    {
	0x24c0, 40,  6, 22, 0xc5,	// bmg, track, index, slot, music_id
	"agBC3",
	"GBA Battle Course 3",
	"GBA Kampfkurs 3",
	"old_battle3_gba",
	"r_agb_battle_n",
	"r_agb_battle_F",
	{{0xac,0xb6,0x9b,0x36,0x23,0x58,0xcd,0xfe,0x8a,0x73,0x94,0x19,0x66,0x96,0xc4,0x29,0x6a,0xcb,0xc3,0xa0,0x00,0x13,0x5b,0xe0},
	 {0xdf,0xb2,0xcd,0x15,0xfa,0xe8,0xd0,0x77,0xfc,0xd8,0x9d,0x13,0xab,0x6a,0x15,0xfc,0xff,0xbe,0xd4,0x21,0x00,0x13,0x5b,0xe0},
	 {0},
	 {0}}
    },
    {
	0x24c1, 41,  7, 23, 0xc7,	// bmg, track, index, slot, music_id
	"anSS",
	"N64 Skyscraper",
	"N64 Wolkenkratzer",
	"old_matenro_64",
	"r_64_battle_n",
	"r_64_battle_F",
	{{0x17,0x77,0x10,0xcd,0x82,0xc3,0x01,0x99,0x03,0x39,0x2a,0x77,0xaf,0xfa,0x46,0x15,0x14,0x54,0x02,0xa3,0x00,0x19,0x47,0xe0},
	 {0x80,0xe8,0x57,0xf6,0x0e,0xb5,0xce,0x54,0x1a,0x6f,0xad,0x4e,0xbf,0x13,0x43,0x04,0x87,0x6c,0x93,0x43,0x00,0x19,0x47,0xe0},
	 {0},
	 {0}}
    },
};

//-----------------------------------------------------------------------------
// arena name table

const KeywordTab_t arena_name_tab[] =
{
    {  0, "1.2",                    "12",                     0 },
    {  0, "A1.2",                   "A12",                    0 },
    {  0, "ADP",                    "DELFINOPIER",            0 },
    {  0, "NVENICEF.BRSTM",         "NVENICEN.BRSTM",         0 },
    {  0, "U1.2",                   "U12",                    0 },
    {  0, "VENICEBATTLE",           "VENICEBATTLE.SZS",       0 },

    {  1, "1.1",                    "11",                     0 },
    {  1, "A1.1",                   "A11",                    0 },
    {  1, "ABP",                    "BLOCKBATTLE",            0 },
    {  1, "BLOCKBATTLE.SZS",        "BLOCKPLAZA",             0 },
    {  1, "NBLOCKF.BRSTM",          "NBLOCKN.BRSTM",          0 },
    {  1, "U1.1",                   "U11",                    0 },

    {  2, "1.4",                    "14",                     0 },
    {  2, "A1.4",                   "A14",                    0 },
    {  2, "ACCW",                   "CASINOBATTLE",           0 },
    {  2, "CASINOBATTLE.SZS",       "CHAINCHOMPWHEEL",        0 },
    {  2, "KETTENHUNDRLT",          "KETTENHUNDROULETTE",     0 },
    {  2, "NCASINOF.BRSTM",         "NCASINON.BRSTM",         0 },
    {  2, "U1.4",                   "U14",                    0 },

    {  3, "1.3",                    "13",                     0 },
    {  3, "A1.3",                   "A13",                    0 },
    {  3, "AFS",                    "FUNKYSTADION",           0 },
    {  3, "FUNKYSTADIUM",           "NSKATEF.BRSTM",          0 },
    {  3, "NSKATEN.BRSTM",          "SKATEBATTLE",            0 },
    {  3, "SKATEBATTLE.SZS",        "U1.3",                   0 },
    {  3, "U13",                    0,                        0 },

    {  4, "1.5",                    "15",                     0 },
    {  4, "A1.5",                   "A15",                    0 },
    {  4, "ATD",                    "NRYUUSAF.BRSTM",         0 },
    {  4, "NRYUUSAN.BRSTM",         "SANDBATTLE",             0 },
    {  4, "SANDBATTLE.SZS",         "STEINWUESTE",            0 },
    {  4, "THWOMPDESERT",           "U1.5",                   0 },
    {  4, "U15",                    0,                        0 },

    {  5, "2.4",                    "24",                     0 },
    {  5, "A2.4",                   "A24",                    0 },
    {  5, "AGCL",                   "ARCL",                   0 },
    {  5, "GCNCOOKIELAND",          "GCNKEKSLAND",            0 },
    {  5, "OLDCOOKIELANDGC",        "OLDCOOKIELANDGC.SZS",    0 },
    {  5, "RGCBATTLE32F.BRSTM",     "RGCBATTLE32N.BRSTM",     0 },
    {  5, "U2.4",                   "U24",                    0 },

    {  6, "2.5",                    "25",                     0 },
    {  6, "A2.5",                   "A25",                    0 },
    {  6, "ADTH",                   "ARTH",                   0 },
    {  6, "DSFINSTERVILLA",         "DSTWILIGHTHOUSE",        0 },
    {  6, "OLDHOUSEDS",             "OLDHOUSEDS.SZS",         0 },
    {  6, "RDSBATTLEF.BRSTM",       "RDSBATTLEN.BRSTM",       0 },
    {  6, "U2.5",                   "U25",                    0 },

    {  7, "2.1",                    "21",                     0 },
    {  7, "A2.1",                   "A21",                    0 },
    {  7, "ARBC4",                  "ASBC4",                  0 },
    {  7, "OLDBATTLE4SFC",          "OLDBATTLE4SFC.SZS",      0 },
    {  7, "RSFCBATTLEF.BRSTM",      "RSFCBATTLEN.BRSTM",      0 },
    {  7, "SNESBATTLECOURSE4",      "SNESKAMPFK.4",           0 },
    {  7, "SNESKAMPFKURS4",         "U2.1",                   0 },
    {  7, "U21",                    0,                        0 },

    {  8, "2.2",                    "22",                     0 },
    {  8, "A2.2",                   "A22",                    0 },
    {  8, "AGBC3",                  "ARBC3",                  0 },
    {  8, "GBABATTLECOURSE3",       "GBAKAMPFKURS3",          0 },
    {  8, "OLDBATTLE3GBA",          "OLDBATTLE3GBA.SZS",      0 },
    {  8, "RAGBBATTLEF.BRSTM",      "RAGBBATTLEN.BRSTM",      0 },
    {  8, "U2.2",                   "U22",                    0 },

    {  9, "2.3",                    "23",                     0 },
    {  9, "A2.3",                   "A23",                    0 },
    {  9, "ANSS",                   "ARSS",                   0 },
    {  9, "N64SKYSCRAPER",          "N64WOLKENKRATZER",       0 },
    {  9, "OLDMATENRO64",           "OLDMATENRO64.SZS",       0 },
    {  9, "R64BATTLEF.BRSTM",       "R64BATTLEN.BRSTM",       0 },
    {  9, "U2.3",                   "U23",                    0 },
    {  9, "WOLKENKRATZER",          0,                        0 },

    {0,0,0,0}
};

//-----------------------------------------------------------------------------

int ScanArena ( ccp name )
{
    const KeywordTab_t * cmd = ScanKeyword(0,name,arena_name_tab);
    return cmd ? cmd->id : -1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct MusicInfo_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const MusicInfo_t music_info[MKW_N_MUSIC+1] =
{
	{ 0x75, 0, -1, "xLC" },
	{ 0x77, 1,  1, "MMM" },
	{ 0x79, 1,  2, "MG" },
	{ 0x7b, 1,  4, "TF" },
	{ 0x7d, 1,  0, "MC" },
	{ 0x7f, 1,  5, "CM" },
	{ 0x81, 1,  6, "DKS" },
	{ 0x83, 1,  7, "WGM" },
	{ 0x85, 1, 15, "KC" },
	{ 0x87, 1,  9, "DC" },
	{ 0x89, 1, 14, "DDR" },
	{ 0x8b, 1,  3, "GV" },
	{ 0x8d, 1, 10, "MH" },
	{ 0x8f, 1, 11, "MT" },
	{ 0x91, 1, 12, "BC" },
	{ 0x93, 1, 13, "RR" },
	{ 0x95, 5, 31, "gSGB" },
	{ 0x97, 5, 25, "sGV2" },
	{ 0x99, 5, 24, "sMC3" },
	{ 0x9b, 5, 30, "gBC3" },
	{ 0x9d, 5, 27, "nSL" },
	{ 0x9f, 5, 26, "nMR" },
	{ 0xa1, 5, 29, "nDKJP" },
	{ 0xa3, 5, 28, "nBC" },
	{ 0xa5, 5, 16, "gPB" },
	{ 0xa7, 5, 17, "gMC" },
	{ 0xa9, 5, 18, "gWS" },
	{ 0xab, 5, 19, "gDKM" },
	{ 0xad, 5, 20, "dYF" },
	{ 0xaf, 5, 23, "dDS" },
	{ 0xb1, 5, 21, "dDH" },
	{ 0xb3, 5, 22, "dPG" },
	{ 0xb5, 2, 32, "aDP" },
	{ 0xb7, 2, 33, "aBP" },
	{ 0xb9, 2, 35, "aFS" },
	{ 0xbb, 2, 34, "aCCW" },
	{ 0xbd, 2, 36, "aTD" },
	{ 0xbf, 6, 37, "agCL" },
	{ 0xc1, 6, 38, "adTH" },
	{ 0xc3, 6, 39, "asBC4" },
	{ 0xc5, 6, 40, "agBC3" },
	{ 0xc7, 6, 41, "anSS" },
	{0,0,-1,""}
};

const KeywordTab_t music_keyword_tab[] =
{
	{ 0x75, "MLC",  "XLC",   -1 },
	{ 0x77, "M12",  "MMM",    1 },
	{ 0x79, "M13",  "MG",     2 },
	{ 0x7b, "M14",  "TF",     4 },
	{ 0x7d, "M21",  "MC",     0 },
	{ 0x7d, "M11",  "LC",     8 },
	{ 0x7f, "M22",  "CM",     5 },
	{ 0x81, "M23",  "DKS",    6 },
	{ 0x83, "M24",  "WGM",    7 },
	{ 0x85, "M32",  "KC",    15 },
	{ 0x87, "M31",  "DC",     9 },
	{ 0x89, "M41",  "DDR",   14 },
	{ 0x8b, "M34",  "GV",     3 },
	{ 0x8d, "M42",  "MH",    10 },
	{ 0x8f, "M33",  "MT",    11 },
	{ 0x91, "M43",  "BC",    12 },
	{ 0x93, "M44",  "RR",    13 },
	{ 0x95, "M62",  "GSGB",  31 },
	{ 0x97, "M53",  "SGV2",  25 },
	{ 0x99, "M81",  "SMC3",  24 },
	{ 0x9b, "M72",  "GBC3",  30 },
	{ 0x9d, "M61",  "NSL",   27 },
	{ 0x9f, "M54",  "NMR",   26 },
	{ 0xa1, "M73",  "NDKJP", 29 },
	{ 0xa3, "M84",  "NBC",   28 },
	{ 0xa5, "M51",  "GPB",   16 },
	{ 0xa7, "M74",  "GMC",   17 },
	{ 0xa9, "M64",  "GWS",   18 },
	{ 0xab, "M83",  "GDKM",  19 },
	{ 0xad, "M52",  "DYF",   20 },
	{ 0xaf, "M63",  "DDS",   23 },
	{ 0xb1, "M71",  "DDH",   21 },
	{ 0xb3, "M82",  "DPG",   22 },
	{ 0xb5, "MA12", "ADP",   32 },
	{ 0xb7, "MA11", "ABP",   33 },
	{ 0xb9, "MA13", "AFS",   35 },
	{ 0xbb, "MA14", "ACCW",  34 },
	{ 0xbd, "MA15", "ATD",   36 },
	{ 0xbf, "MA24", "AGCL",  37 },
	{ 0xc1, "MA25", "ADTH",  38 },
	{ 0xc3, "MA21", "ASBC4", 39 },
	{ 0xc5, "MA22", "AGBC3", 40 },
	{ 0xc7, "MA23", "ANSS",  41 },
	{0,0,0,0}
};

//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



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
	"n_Circuit32_f"
    },
    {
	0x2491,  1,  1, 12, 0x77,	// bmg, track, index, slot, music_id
	"MMM",
	"Moo Moo Meadows",
	"Kuhmuh-Weide",
	"farm_course",
	"n_Farm_n",
	"n_Farm_F"
    },
    {
	0x2492,  2,  2, 13, 0x79,	// bmg, track, index, slot, music_id
	"MG",
	"Mushroom Gorge",
	"Pilz-Schlucht",
	"kinoko_course",
	"n_Kinoko_n",
	"n_Kinoko_F"
    },
    {
	0x2493,  3, 11, 34, 0x8b,	// bmg, track, index, slot, music_id
	"GV",
	"Grumble Volcano",
	"Vulkangrollen",
	"volcano_course",
	"n_Volcano32_n",
	"n_Volcano32_f"
    },
    {
	0x2494,  4,  3, 14, 0x7b,	// bmg, track, index, slot, music_id
	"TF",
	"Toad's Factory",
	"Toads Fabrik",
	"factory_course",
	"STRM_N_FACTORY_N",
	"STRM_N_FACTORY_F"
    },
    {
	0x2495,  5,  5, 22, 0x7f,	// bmg, track, index, slot, music_id
	"CM",
	"Coconut Mall",
	"Kokos-Promenade",
	"shopping_course",
	"n_Shopping32_n",
	"n_Shopping32_f"
    },
    {
	0x2496,  6,  6, 23, 0x81,	// bmg, track, index, slot, music_id
	"DKS",
	"DK Summit",
	"DK Skikane",
	"boardcross_course",
	"n_Snowboard32_n",
	"n_Snowboard32_F"
    },
    {
	0x2497,  7,  7, 24, 0x83,	// bmg, track, index, slot, music_id
	"WGM",
	"Wario's Gold Mine",
	"Warios Goldmine",
	"truck_course",
	"STRM_N_TRUCK_N",
	"STRM_N_TRUCK_F"
    },
    {
	0x2498,  8,  0, 11, 0x7d,	// bmg, track, index, slot, music_id
	"LC",
	"Luigi Circuit",
	"Luigis Piste",
	"beginner_course",
	"n_Circuit32_n",
	"n_Circuit32_f"
    },
    {
	0x2499,  9,  8, 31, 0x87,	// bmg, track, index, slot, music_id
	"DC",
	"Daisy Circuit",
	"Daisys Piste",
	"senior_course",
	"n_Daisy32_n",
	"n_Daisy32_f"
    },
    {
	0x249a, 10, 13, 42, 0x8d,	// bmg, track, index, slot, music_id
	"MH",
	"Moonview Highway",
	"Mondblickstraße",
	"ridgehighway_course",
	"STRM_N_RIDGEHIGHWAY_N",
	"STRM_N_RIDGEHIGHWAY_F"
    },
    {
	0x249b, 11, 10, 33, 0x8f,	// bmg, track, index, slot, music_id
	"MT",
	"Maple Treeway",
	"Blätterwald",
	"treehouse_course",
	"n_maple_n",
	"n_maple_F"
    },
    {
	0x249c, 12, 14, 43, 0x91,	// bmg, track, index, slot, music_id
	"BC",
	"Bowser's Castle",
	"Bowsers Festung",
	"koopa_course",
	"STRM_N_KOOPA_N",
	"STRM_N_KOOPA_F"
    },
    {
	0x249d, 13, 15, 44, 0x93,	// bmg, track, index, slot, music_id
	"RR",
	"Rainbow Road",
	"Regenbogen-Boulevard",
	"rainbow_course",
	"n_Rainbow32_n",
	"n_Rainbow32_f"
    },
    {
	0x249e, 14, 12, 41, 0x89,	// bmg, track, index, slot, music_id
	"DDR",
	"Dry Dry Ruins",
	"Staubtrockene Ruinen",
	"desert_course",
	"STRM_N_DESERT_N",
	"STRM_N_DESERT_F"
    },
    {
	0x249f, 15,  9, 32, 0x85,	// bmg, track, index, slot, music_id
	"KC",
	"Koopa Cape",
	"Koopa-Kap",
	"water_course",
	"STRM_N_WATER_N",
	"STRM_N_WATER_F"
    },
    {
	0x24a0, 16, 16, 51, 0xa5,	// bmg, track, index, slot, music_id
	"gPB",
	"GCN Peach Beach",
	"GCN Peach Beach",
	"old_peach_gc",
	"r_GC_Beach32_n",
	"r_GC_Beach32_f"
    },
    {
	0x24a1, 17, 27, 74, 0xa7,	// bmg, track, index, slot, music_id
	"gMC",
	"GCN Mario Circuit",
	"GCN Marios Piste",
	"old_mario_gc",
	"r_GC_Circuit32_n",
	"r_GC_Circuit32_f"
    },
    {
	0x24a2, 18, 23, 64, 0xa9,	// bmg, track, index, slot, music_id
	"gWS",
	"GCN Waluigi Stadium",
	"GCN Waluigi-Arena",
	"old_waluigi_gc",
	"r_GC_Stadium32_n",
	"r_GC_Stadium32_f"
    },
    {
	0x24a3, 19, 30, 83, 0xab,	// bmg, track, index, slot, music_id
	"gDKM",
	"GCN DK Mountain",
	"GCN DK Bergland",
	"old_donkey_gc",
	"r_GC_Mountain32_n",
	"r_GC_Mountain32_f"
    },
    {
	0x24a4, 20, 17, 52, 0xad,	// bmg, track, index, slot, music_id
	"dYF",
	"DS Yoshi Falls",
	"DS Yoshi-Kaskaden",
	"old_falls_ds",
	"r_DS_Jungle32_n",
	"r_DS_Jungle32_f"
    },
    {
	0x24a5, 21, 24, 71, 0xb1,	// bmg, track, index, slot, music_id
	"dDH",
	"DS Desert Hills",
	"DS Glühheiße Wüste",
	"old_desert_ds",
	"r_DS_Desert32_n",
	"r_DS_Desert32_f"
    },
    {
	0x24a6, 22, 29, 82, 0xb3,	// bmg, track, index, slot, music_id
	"dPG",
	"DS Peach Gardens",
	"DS Peachs Schlossgarten",
	"old_garden_ds",
	"r_DS_Garden32_n",
	"r_DS_Garden32_f"
    },
    {
	0x24a7, 23, 22, 63, 0xaf,	// bmg, track, index, slot, music_id
	"dDS",
	"DS Delfino Square",
	"DS Piazzale Delfino",
	"old_town_ds",
	"r_DS_Town32_n",
	"r_DS_Town32_f"
    },
    {
	0x24a8, 24, 28, 81, 0x99,	// bmg, track, index, slot, music_id
	"sMC3",
	"SNES Mario Circuit 3",
	"SNES Marios Piste 3",
	"old_mario_sfc",
	"r_SFC_Circuit32_n",
	"r_SFC_Circuit32_f"
    },
    {
	0x24a9, 25, 18, 53, 0x97,	// bmg, track, index, slot, music_id
	"sGV2",
	"SNES Ghost Valley 2",
	"SNES Geistertal 2",
	"old_obake_sfc",
	"r_SFC_Obake32_n",
	"r_SFC_Obake32_f"
    },
    {
	0x24aa, 26, 19, 54, 0x9f,	// bmg, track, index, slot, music_id
	"nMR",
	"N64 Mario Raceway",
	"N64 Marios Rennpiste",
	"old_mario_64",
	"r_64_Circuit32_n",
	"r_64_Circuit32_f"
    },
    {
	0x24ab, 27, 20, 61, 0x9d,	// bmg, track, index, slot, music_id
	"nSL",
	"N64 Sherbet Land",
	"N64 Sorbet-Land",
	"old_sherbet_64",
	"r_64_Sherbet32_n",
	"r_64_Sherbet32_f"
    },
    {
	0x24ac, 28, 31, 84, 0xa3,	// bmg, track, index, slot, music_id
	"nBC",
	"N64 Bowser's Castle",
	"N64 Bowsers Festung",
	"old_koopa_64",
	"r_64_Kuppa32_n",
	"r_64_Kuppa32_f"
    },
    {
	0x24ad, 29, 26, 73, 0xa1,	// bmg, track, index, slot, music_id
	"nDKJP",
	"N64 DK's Jungle Parkway",
	"N64 DKs Dschungelpark",
	"old_donkey_64",
	"r_64_Jungle32_n",
	"r_64_Jungle32_f"
    },
    {
	0x24ae, 30, 25, 72, 0x9b,	// bmg, track, index, slot, music_id
	"gBC3",
	"GBA Bowser Castle 3",
	"GBA Bowsers Festung 3",
	"old_koopa_gba",
	"r_AGB_Kuppa32_n",
	"r_AGB_Kuppa32_f"
    },
    {
	0x24af, 31, 21, 62, 0x95,	// bmg, track, index, slot, music_id
	"gSGB",
	"GBA Shy Guy Beach",
	"GBA Shy Guy-Strand",
	"old_heyho_gba",
	"r_AGB_Beach32_n",
	"r_AGB_Beach32_f"
    },
};

//-----------------------------------------------------------------------------
// track name table

const KeywordTab_t track_name_tab[] =
{
    {  0, "2.1",                    "21",                     0 },
    {  0, "CASTLECOURSE.SZS",       "MARIOCIRCUIT",           0 },
    {  0, "MARIOSPISTE",            "MC",                     0 },
    {  0, "T2.1",                   "T21",                    0 },

    {  1, "1.2",                    "12",                     0 },
    {  1, "FARMCOURSE.SZS",         "KUHMUH",                 0 },
    {  1, "KUHMUHWEIDE",            "MEADOWS",                0 },
    {  1, "MMM",                    "MOO",                    0 },
    {  1, "MOOMEADOWS",             "MOOMOO",                 0 },
    {  1, "MOOMOOMEADOWS",          "NFARMF.BRSTM",           0 },
    {  1, "NFARMN.BRSTM",           "T1.2",                   0 },
    {  1, "T12",                    "WEIDE",                  0 },

    {  2, "1.3",                    "13",                     0 },
    {  2, "GORGE",                  "KINOKOCOURSE.SZS",       0 },
    {  2, "MG",                     "MUSHROOM",               0 },
    {  2, "MUSHROOMGORGE",          "NKINOKOF.BRSTM",         0 },
    {  2, "NKINOKON.BRSTM",         "PILZ",                   0 },
    {  2, "PILZSCHLUCHT",           "SCHLUCHT",               0 },
    {  2, "T1.3",                   "T13",                    0 },

    {  3, "3.4",                    "34",                     0 },
    {  3, "GRUMBLE",                "GRUMBLEVOLCANO",         0 },
    {  3, "GV",                     "NVOLCANO32F.BRSTM",      0 },
    {  3, "NVOLCANO32N.BRSTM",      "T3.4",                   0 },
    {  3, "T34",                    "VOLCANO",                0 },
    {  3, "VOLCANOCOURSE.SZS",      "VULKANGROLLEN",          0 },

    {  4, "1.4",                    "14",                     0 },
    {  4, "FABRIK",                 "FACTORY",                0 },
    {  4, "FACTORYCOURSE.SZS",      "STRMNFACTORYF.BRSTM",    0 },
    {  4, "STRMNFACTORYN.BRSTM",    "T1.4",                   0 },
    {  4, "T14",                    "TF",                     0 },
    {  4, "TOADS",                  "TOADSFABRIK",            0 },
    {  4, "TOADSFACTORY",           0,                        0 },

    {  5, "2.2",                    "22",                     0 },
    {  5, "CM",                     "COCONUT",                0 },
    {  5, "COCONUTMALL",            "KOKOS",                  0 },
    {  5, "KOKOSPROMENADE",         "MALL",                   0 },
    {  5, "NSHOPPING32F.BRSTM",     "NSHOPPING32N.BRSTM",     0 },
    {  5, "PROMENADE",              "SHOPPINGCOURSE.SZS",     0 },
    {  5, "T2.2",                   "T22",                    0 },

    {  6, "2.3",                    "23",                     0 },
    {  6, "BOARDCROSSCOURSE.SZS",   "DKS",                    0 },
    {  6, "DKSKIKANE",              "DKSUMMIT",               0 },
    {  6, "DS",                     "NSNOWBOARD32F.BRSTM",    0 },
    {  6, "NSNOWBOARD32N.BRSTM",    "SKIKANE",                0 },
    {  6, "SUMMIT",                 "T2.3",                   0 },
    {  6, "T23",                    0,                        0 },

    {  7, "2.4",                    "24",                     0 },
    {  7, "GOLD",                   "GOLDMINE",               0 },
    {  7, "MINE",                   "STRMNTRUCKF.BRSTM",      0 },
    {  7, "STRMNTRUCKN.BRSTM",      "T2.4",                   0 },
    {  7, "T24",                    "TRUCKCOURSE.SZS",        0 },
    {  7, "WARIOS",                 "WARIOSGOLD",             0 },
    {  7, "WARIOSGOLDMINE",         "WGM",                    0 },

    {  8, "1.1",                    "11",                     0 },
    {  8, "BEGINNERCOURSE.SZS",     "LC",                     0 },
    {  8, "LUIGI",                  "LUIGICIRCUIT",           0 },
    {  8, "LUIGIS",                 "LUIGISPISTE",            0 },
    {  8, "T1.1",                   "T11",                    0 },

    {  9, "3.1",                    "31",                     0 },
    {  9, "DAISY",                  "DAISYCIRCUIT",           0 },
    {  9, "DAISYS",                 "DAISYSPISTE",            0 },
    {  9, "DC",                     "NDAISY32F.BRSTM",        0 },
    {  9, "NDAISY32N.BRSTM",        "SENIORCOURSE.SZS",       0 },
    {  9, "T3.1",                   "T31",                    0 },

    { 10, "4.2",                    "42",                     0 },
    { 10, "HIGHWAY",                "MH",                     0 },
    { 10, "MONDBLICKSTRASSE",       "MOONVIEW",               0 },
    { 10, "MOONVIEWHIGHWAY",        "MVH",                    0 },
    { 10, "RIDGEHIGHWAYCOURSE.SZS", "STRMNRIDGEHIGHWAYF.BRSTM", 0 },
    { 10, "STRMNRIDGEHIGHWAYN.BRSTM", "T4.2",                   0 },
    { 10, "T42",                    0,                        0 },

    { 11, "3.3",                    "33",                     0 },
    { 11, "BLAETTERWALD",           "MAPLE",                  0 },
    { 11, "MAPLETREEWAY",           "MT",                     0 },
    { 11, "NMAPLEF.BRSTM",          "NMAPLEN.BRSTM",          0 },
    { 11, "T3.3",                   "T33",                    0 },
    { 11, "TREEHOUSECOURSE.SZS",    "TREEWAY",                0 },

    { 12, "4.3",                    "43",                     0 },
    { 12, "BC",                     "BOW.FESTUNG",            0 },
    { 12, "BOWSERSCASTLE",          "BOWSERSFESTUNG",         0 },
    { 12, "KOOPACOURSE.SZS",        "STRMNKOOPAF.BRSTM",      0 },
    { 12, "STRMNKOOPAN.BRSTM",      "T4.3",                   0 },
    { 12, "T43",                    0,                        0 },

    { 13, "4.4",                    "44",                     0 },
    { 13, "BLVD",                   "NRAINBOW32F.BRSTM",      0 },
    { 13, "NRAINBOW32N.BRSTM",      "RAINBOW",                0 },
    { 13, "RAINBOWCOURSE.SZS",      "RAINBOWROAD",            0 },
    { 13, "REGENBOGEN",             "REGENBOGENBLVD",         0 },
    { 13, "REGENBOGENBOULEVARD",    "ROAD",                   0 },
    { 13, "RR",                     "T4.4",                   0 },
    { 13, "T44",                    0,                        0 },

    { 14, "4.1",                    "41",                     0 },
    { 14, "DDR",                    "DESERTCOURSE.SZS",       0 },
    { 14, "DRY",                    "DRYDRY",                 0 },
    { 14, "DRYDRYRUINS",            "DRYRUINS",               0 },
    { 14, "RUINEN",                 "RUINS",                  0 },
    { 14, "STAUBTROCKENE",          "STAUBTROCKENERUINEN",    0 },
    { 14, "STRMNDESERTF.BRSTM",     "STRMNDESERTN.BRSTM",     0 },
    { 14, "T4.1",                   "T41",                    0 },
    { 14, "TROCKENE",               "TROCKENERUINEN",         0 },

    { 15, "3.2",                    "32",                     0 },
    { 15, "CAPE",                   "KAP",                    0 },
    { 15, "KC",                     "KOOPA",                  0 },
    { 15, "KOOPACAPE",              "KOOPAKAP",               0 },
    { 15, "STRMNWATERF.BRSTM",      "STRMNWATERN.BRSTM",      0 },
    { 15, "T3.2",                   "T32",                    0 },
    { 15, "WATERCOURSE.SZS",        0,                        0 },

    { 16, "5.1",                    "51",                     0 },
    { 16, "GCNPEACH",               "GCNPEACHBEACH",          0 },
    { 16, "GPB",                    "OLDPEACHGC.SZS",         0 },
    { 16, "PEACHBEACH",             "RGCBEACH32F.BRSTM",      0 },
    { 16, "RGCBEACH32N.BRSTM",      "RPB",                    0 },
    { 16, "T5.1",                   "T51",                    0 },

    { 17, "7.4",                    "74",                     0 },
    { 17, "GC",                     "GCMARIOS",               0 },
    { 17, "GCMARIOSPISTE",          "GCNMARIO",               0 },
    { 17, "GCNMARIOCIRCUIT",        "GCNMARIOS",              0 },
    { 17, "GCNMARIOSPISTE",         "GMC",                    0 },
    { 17, "OLDMARIOGC.SZS",         "RGCCIRCUIT32F.BRSTM",    0 },
    { 17, "RGCCIRCUIT32N.BRSTM",    "RMC",                    0 },
    { 17, "T7.4",                   "T74",                    0 },

    { 18, "6.4",                    "64",                     0 },
    { 18, "ARENA",                  "GCNWALUIGI",             0 },
    { 18, "GCNWALUIGIARENA",        "GCNWALUIGISTADIUM",      0 },
    { 18, "GWS",                    "OLDWALUIGIGC.SZS",       0 },
    { 18, "RGCSTADIUM32F.BRSTM",    "RGCSTADIUM32N.BRSTM",    0 },
    { 18, "RWS",                    "STADIUM",                0 },
    { 18, "T6.4",                   "T64",                    0 },
    { 18, "WALUIGI",                "WALUIGIARENA",           0 },
    { 18, "WALUIGISTADIUM",         0,                        0 },

    { 19, "8.3",                    "83",                     0 },
    { 19, "BERGLAND",               "DKBERGLAND",             0 },
    { 19, "DKMOUNTAIN",             "GCNDK",                  0 },
    { 19, "GCNDKBERGLAND",          "GCNDKMOUNTAIN",          0 },
    { 19, "GDKM",                   "MOUNTAIN",               0 },
    { 19, "OLDDONKEYGC.SZS",        "RDKM",                   0 },
    { 19, "RGCMOUNTAIN32F.BRSTM",   "RGCMOUNTAIN32N.BRSTM",   0 },
    { 19, "T8.3",                   "T83",                    0 },

    { 20, "5.2",                    "52",                     0 },
    { 20, "DSYOSHI",                "DSYOSHIFALLS",           0 },
    { 20, "DSYOSHIKASKADEN",        "DYF",                    0 },
    { 20, "FALLS",                  "KASKADEN",               0 },
    { 20, "OLDFALLSDS.SZS",         "RDSJUNGLE32F.BRSTM",     0 },
    { 20, "RDSJUNGLE32N.BRSTM",     "RYF",                    0 },
    { 20, "T5.2",                   "T52",                    0 },
    { 20, "YOSHI",                  "YOSHIFALLS",             0 },
    { 20, "YOSHIKASKADEN",          0,                        0 },

    { 21, "7.1",                    "71",                     0 },
    { 21, "DDH",                    "DESERT",                 0 },
    { 21, "DESERTHILLS",            "DSDESERT",               0 },
    { 21, "DSDESERTHILLS",          "DSGLUEHHEISSE",          0 },
    { 21, "DSGLUEHHEISSEWUESTE",    "GLUEHHEISSE",            0 },
    { 21, "GLUEHHEISSEWUESTE",      "HEISSE",                 0 },
    { 21, "HEISSEWUESTE",           "HILLS",                  0 },
    { 21, "OLDDESERTDS.SZS",        "RDH",                    0 },
    { 21, "RDSDESERT32F.BRSTM",     "RDSDESERT32N.BRSTM",     0 },
    { 21, "T7.1",                   "T71",                    0 },
    { 21, "WUESTE",                 0,                        0 },

    { 22, "8.2",                    "82",                     0 },
    { 22, "DPG",                    "DSPEACH",                0 },
    { 22, "DSPEACHGARDENS",         "DSPEACHS",               0 },
    { 22, "DSPEACHSSCHLOSSGARTEN",  "GARDENS",                0 },
    { 22, "OLDGARDENDS.SZS",        "PEACHGARDENS",           0 },
    { 22, "PEACHS",                 "PEACHSSCHLOSSGARTEN",    0 },
    { 22, "RDSGARDEN32F.BRSTM",     "RDSGARDEN32N.BRSTM",     0 },
    { 22, "RPG",                    "SCHLOSSGARTEN",          0 },
    { 22, "T8.2",                   "T82",                    0 },

    { 23, "6.3",                    "63",                     0 },
    { 23, "DDS",                    "DELFINO",                0 },
    { 23, "DELFINOSQUARE",          "DSDELFINO",              0 },
    { 23, "DSDELFINOSQUARE",        "DSPIAZZALE",             0 },
    { 23, "DSPIAZZALEDELFINO",      "OLDTOWNDS.SZS",          0 },
    { 23, "PIAZ",                   "PIAZ.DELFINO",           0 },
    { 23, "PIAZZALE",               "PIAZZALEDELFINO",        0 },
    { 23, "RDS",                    "RDSTOWN32F.BRSTM",       0 },
    { 23, "RDSTOWN32N.BRSTM",       "SQUARE",                 0 },
    { 23, "T6.3",                   "T63",                    0 },

    { 24, "8.1",                    "81",                     0 },
    { 24, "CIRCUIT3",               "MARIOCIRCUIT3",          0 },
    { 24, "MARIOSPISTE3",           "OLDMARIOSFC.SZS",        0 },
    { 24, "PISTE3",                 "RMC3",                   0 },
    { 24, "RSFCCIRCUIT32F.BRSTM",   "RSFCCIRCUIT32N.BRSTM",   0 },
    { 24, "SMC3",                   "SNESMARIO",              0 },
    { 24, "SNESMARIOCIRCUIT",       "SNESMARIOCIRCUIT3",      0 },
    { 24, "SNESMARIOS",             "SNESMARIOSPISTE",        0 },
    { 24, "SNESMARIOSPISTE3",       "T8.1",                   0 },
    { 24, "T81",                    0,                        0 },

    { 25, "5.3",                    "53",                     0 },
    { 25, "GEISTERTAL",             "GEISTERTAL2",            0 },
    { 25, "GHOST",                  "GHOSTVALLEY",            0 },
    { 25, "GHOSTVALLEY2",           "OLDOBAKESFC.SZS",        0 },
    { 25, "RGV2",                   "RSFCOBAKE32F.BRSTM",     0 },
    { 25, "RSFCOBAKE32N.BRSTM",     "SGV2",                   0 },
    { 25, "SNESGEISTERTAL",         "SNESGEISTERTAL2",        0 },
    { 25, "SNESGHOST",              "SNESGHOSTVALLEY",        0 },
    { 25, "SNESGHOSTVALLEY2",       "T5.3",                   0 },
    { 25, "T53",                    "VALLEY",                 0 },
    { 25, "VALLEY2",                0,                        0 },

    { 26, "5.4",                    "54",                     0 },
    { 26, "MAR",                    "MAR.RENNPISTE",          0 },
    { 26, "MARIORACEWAY",           "MARIOSRENNPISTE",        0 },
    { 26, "N64MARIO",               "N64MARIORACEWAY",        0 },
    { 26, "N64MARIOS",              "N64MARIOSRENNPISTE",     0 },
    { 26, "NMR",                    "OLDMARIO64.SZS",         0 },
    { 26, "R64CIRCUIT32F.BRSTM",    "R64CIRCUIT32N.BRSTM",    0 },
    { 26, "RACEWAY",                "RENNPISTE",              0 },
    { 26, "RMR",                    "T5.4",                   0 },
    { 26, "T54",                    0,                        0 },

    { 27, "6.1",                    "61",                     0 },
    { 27, "LAND",                   "N64SHERBET",             0 },
    { 27, "N64SHERBETLAND",         "N64SORBETLAND",          0 },
    { 27, "NSL",                    "OLDSHERBET64.SZS",       0 },
    { 27, "R64SHERBET32F.BRSTM",    "R64SHERBET32N.BRSTM",    0 },
    { 27, "RSL",                    "SHERBET",                0 },
    { 27, "SHERBETLAND",            "SORBET",                 0 },
    { 27, "SORBETLAND",             "T6.1",                   0 },
    { 27, "T61",                    0,                        0 },

    { 28, "8.4",                    "84",                     0 },
    { 28, "BOW.FESTUNG",            "N64BOW.FESTUNG",         0 },
    { 28, "N64BOWSERS",             "N64BOWSERSCASTLE",       0 },
    { 28, "N64BOWSERSFESTUNG",      "NBC",                    0 },
    { 28, "OLDKOOPA64.SZS",         "R64KUPPA32F.BRSTM",      0 },
    { 28, "R64KUPPA32N.BRSTM",      "RBC",                    0 },
    { 28, "T8.4",                   "T84",                    0 },

    { 29, "7.3",                    "73",                     0 },
    { 29, "DKS",                    "DKSDSCHUNGELPARK",       0 },
    { 29, "DKSJUNGLE",              "DKSJUNGLEPARKWAY",       0 },
    { 29, "DSCHUNGELPARK",          "JUNGLE",                 0 },
    { 29, "JUNGLEPARKWAY",          "N64DKS",                 0 },
    { 29, "N64DKSDSCHUNGELPARK",    "N64DKSJUNGLE",           0 },
    { 29, "N64DKSJUNGLEPARKWAY",    "NDKJP",                  0 },
    { 29, "OLDDONKEY64.SZS",        "PARKWAY",                0 },
    { 29, "R64JUNGLE32F.BRSTM",     "R64JUNGLE32N.BRSTM",     0 },
    { 29, "RDJP",                   "RDKJP",                  0 },
    { 29, "T7.3",                   "T73",                    0 },

    { 30, "7.2",                    "72",                     0 },
    { 30, "BOW.FESTUNG3",           "BOWSER",                 0 },
    { 30, "BOWSERCASTLE",           "BOWSERCASTLE3",          0 },
    { 30, "BOWSERSFESTUNG3",        "CASTLE3",                0 },
    { 30, "FESTUNG3",               "GBABOWSER",              0 },
    { 30, "GBABOWSERCASTLE",        "GBABOWSERCASTLE3",       0 },
    { 30, "GBABOWSERS",             "GBABOWSERSFESTUNG",      0 },
    { 30, "GBABOWSERSFESTUNG3",     "GBC3",                   0 },
    { 30, "OLDKOOPAGBA.SZS",        "RAGBKUPPA32F.BRSTM",     0 },
    { 30, "RAGBKUPPA32N.BRSTM",     "RBC3",                   0 },
    { 30, "T7.2",                   "T72",                    0 },

    { 31, "6.2",                    "62",                     0 },
    { 31, "GBASHY",                 "GBASHYGUY",              0 },
    { 31, "GBASHYGUYBEACH",         "GBASHYGUYSTRAND",        0 },
    { 31, "GSGB",                   "GUY",                    0 },
    { 31, "GUYBEACH",               "GUYSTRAND",              0 },
    { 31, "OLDHEYHOGBA.SZS",        "RAGBBEACH32F.BRSTM",     0 },
    { 31, "RAGBBEACH32N.BRSTM",     "RSGB",                   0 },
    { 31, "SHY",                    "SHYGUY",                 0 },
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
	"n_venice_F"
    },
    {
	0x24b9, 33,  0, 11, 0xb7,	// bmg, track, index, slot, music_id
	"aBP",
	"Block Plaza",
	"Block-Plaza",
	"block_battle",
	"n_block_n",
	"n_block_F"
    },
    {
	0x24ba, 34,  3, 14, 0xbb,	// bmg, track, index, slot, music_id
	"aCCW",
	"Chain Chomp Wheel",
	"Kettenhund-Roulette",
	"casino_battle",
	"n_casino_n",
	"n_casino_F"
    },
    {
	0x24bb, 35,  2, 13, 0xb9,	// bmg, track, index, slot, music_id
	"aFS",
	"Funky Stadium",
	"Funky-Stadion",
	"skate_battle",
	"n_skate_n",
	"n_skate_F"
    },
    {
	0x24bc, 36,  4, 15, 0xbd,	// bmg, track, index, slot, music_id
	"aTD",
	"Thwomp Desert",
	"Steinwüste",
	"sand_battle",
	"n_ryuusa_n",
	"n_ryuusa_F"
    },
    {
	0x24bd, 37,  8, 24, 0xbf,	// bmg, track, index, slot, music_id
	"agCL",
	"GCN Cookie Land",
	"GCN Keks-Land",
	"old_CookieLand_gc",
	"r_GC_Battle32_n",
	"r_GC_Battle32_F"
    },
    {
	0x24be, 38,  9, 25, 0xc1,	// bmg, track, index, slot, music_id
	"adTH",
	"DS Twilight House",
	"DS Finstervilla",
	"old_House_ds",
	"r_ds_battle_n",
	"r_ds_battle_F"
    },
    {
	0x24bf, 39,  5, 21, 0xc3,	// bmg, track, index, slot, music_id
	"asBC4",
	"SNES Battle Course 4",
	"SNES Kampfkurs 4",
	"old_battle4_sfc",
	"r_sfc_battle_n",
	"r_sfc_battle_F"
    },
    {
	0x24c0, 40,  6, 22, 0xc5,	// bmg, track, index, slot, music_id
	"agBC3",
	"GBA Battle Course 3",
	"GBA Kampfkurs 3",
	"old_battle3_gba",
	"r_agb_battle_n",
	"r_agb_battle_F"
    },
    {
	0x24c1, 41,  7, 23, 0xc7,	// bmg, track, index, slot, music_id
	"anSS",
	"N64 Skyscraper",
	"N64 Wolkenkratzer",
	"old_matenro_64",
	"r_64_battle_n",
	"r_64_battle_F"
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
    {  0, "VENICEBATTLE.SZS",       0,                        0 },

    {  1, "1.1",                    "11",                     0 },
    {  1, "A1.1",                   "A11",                    0 },
    {  1, "ABP",                    "BLOCKBATTLE.SZS",        0 },
    {  1, "BLOCKPLAZA",             "NBLOCKF.BRSTM",          0 },
    {  1, "NBLOCKN.BRSTM",          "U1.1",                   0 },
    {  1, "U11",                    0,                        0 },

    {  2, "1.4",                    "14",                     0 },
    {  2, "A1.4",                   "A14",                    0 },
    {  2, "ACCW",                   "CASINOBATTLE.SZS",       0 },
    {  2, "CHAINCHOMPWHEEL",        "KETTENHUNDRLT",          0 },
    {  2, "KETTENHUNDROULETTE",     "NCASINOF.BRSTM",         0 },
    {  2, "NCASINON.BRSTM",         "U1.4",                   0 },
    {  2, "U14",                    0,                        0 },

    {  3, "1.3",                    "13",                     0 },
    {  3, "A1.3",                   "A13",                    0 },
    {  3, "AFS",                    "FUNKYSTADION",           0 },
    {  3, "FUNKYSTADIUM",           "NSKATEF.BRSTM",          0 },
    {  3, "NSKATEN.BRSTM",          "SKATEBATTLE.SZS",        0 },
    {  3, "U1.3",                   "U13",                    0 },

    {  4, "1.5",                    "15",                     0 },
    {  4, "A1.5",                   "A15",                    0 },
    {  4, "ATD",                    "NRYUUSAF.BRSTM",         0 },
    {  4, "NRYUUSAN.BRSTM",         "SANDBATTLE.SZS",         0 },
    {  4, "STEINWUESTE",            "THWOMPDESERT",           0 },
    {  4, "U1.5",                   "U15",                    0 },

    {  5, "2.4",                    "24",                     0 },
    {  5, "A2.4",                   "A24",                    0 },
    {  5, "AGCL",                   "ARCL",                   0 },
    {  5, "GCNCOOKIELAND",          "GCNKEKSLAND",            0 },
    {  5, "OLDCOOKIELANDGC.SZS",    "RGCBATTLE32F.BRSTM",     0 },
    {  5, "RGCBATTLE32N.BRSTM",     "U2.4",                   0 },
    {  5, "U24",                    0,                        0 },

    {  6, "2.5",                    "25",                     0 },
    {  6, "A2.5",                   "A25",                    0 },
    {  6, "ADTH",                   "ARTH",                   0 },
    {  6, "DSFINSTERVILLA",         "DSTWILIGHTHOUSE",        0 },
    {  6, "OLDHOUSEDS.SZS",         "RDSBATTLEF.BRSTM",       0 },
    {  6, "RDSBATTLEN.BRSTM",       "U2.5",                   0 },
    {  6, "U25",                    0,                        0 },

    {  7, "2.1",                    "21",                     0 },
    {  7, "A2.1",                   "A21",                    0 },
    {  7, "ARBC4",                  "ASBC4",                  0 },
    {  7, "OLDBATTLE4SFC.SZS",      "RSFCBATTLEF.BRSTM",      0 },
    {  7, "RSFCBATTLEN.BRSTM",      "SNESBATTLECOURSE4",      0 },
    {  7, "SNESKAMPFK.4",           "SNESKAMPFKURS4",         0 },
    {  7, "U2.1",                   "U21",                    0 },

    {  8, "2.2",                    "22",                     0 },
    {  8, "A2.2",                   "A22",                    0 },
    {  8, "AGBC3",                  "ARBC3",                  0 },
    {  8, "GBABATTLECOURSE3",       "GBAKAMPFKURS3",          0 },
    {  8, "OLDBATTLE3GBA.SZS",      "RAGBBATTLEF.BRSTM",      0 },
    {  8, "RAGBBATTLEN.BRSTM",      "U2.2",                   0 },
    {  8, "U22",                    0,                        0 },

    {  9, "2.3",                    "23",                     0 },
    {  9, "A2.3",                   "A23",                    0 },
    {  9, "ANSS",                   "ARSS",                   0 },
    {  9, "N64SKYSCRAPER",          "N64WOLKENKRATZER",       0 },
    {  9, "OLDMATENRO64.SZS",       "R64BATTLEF.BRSTM",       0 },
    {  9, "R64BATTLEN.BRSTM",       "U2.3",                   0 },
    {  9, "U23",                    "WOLKENKRATZER",          0 },

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


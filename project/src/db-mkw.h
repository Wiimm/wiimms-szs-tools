
//
///////////////////////////////////////////////////////////////////////////////
//////   This file is created by a script. Modifications will be lost!   //////
///////////////////////////////////////////////////////////////////////////////

#ifndef SZS_DB_MKW_H
#define SZS_DB_MKW_H 1

#include "lib-std.h"
#include "lib-mkw.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct TrackInfo_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[TrackInfo_t]] (generic code)

#define TI_SHA1_IS_D	1
#define TI_SHA1_NORM	2

typedef struct TrackInfo_t
{
    u16		bmg_mid;	// standard message id for BMG files
    u8		track_id;	// track id (0..41)
    u8		def_index;	// default index (0..31)
    u8		def_slot;	// default slot (11..84/25)
    u8		music_id;	// music ID

    ccp		abbrev;		// short name
    ccp		name_en;	// english name
    ccp		name_de;	// german name
    ccp		track_fname;	// track file name
    ccp		sound_n_fname;	// file name of normal sound
    ccp		sound_f_fname;	// file name of fast sound

    u8		sha1[4][24];	// SHA1+size, compatible to sha1_size_t
				// ... ORIG, ORIG_D, NORM, NORM_D

} TrackInfo_t;


//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////   tracks   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum // some constants
{
    FW_TRACK_NAME_EN	= 23,
    FW_TRACK_NAME_DE	= 23,
    FW_TRACK_FNAME	= 19,
    FW_TRACK_SOUND_N	= 21,
    FW_TRACK_SOUND_F	= 21,
};

//-----------------------------------------------------------------------------

extern const u32 track_pos_default[32];
extern       u32 track_pos[32];
extern       u32 track_pos_r[32];

extern       u32 track_pos_swap[32];
extern const KeywordTab_t track_name_tab[];
int ScanTrack ( ccp name );

//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////   arenas   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum // some constants
{
    FW_ARENA_NAME_EN	= 20,
    FW_ARENA_NAME_DE	= 19,
    FW_ARENA_FNAME	= 17,
    FW_ARENA_SOUND_N	= 15,
    FW_ARENA_SOUND_F	= 15,
};

//-----------------------------------------------------------------------------

extern const u32 arena_pos_default[10];
extern       u32 arena_pos[10];
extern       u32 arena_pos_r[10];

extern const KeywordTab_t arena_name_tab[];
int ScanArena ( ccp name );

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   vars and functtions   ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern const TrackInfo_t InvalidTrack;
extern const TrackInfo_t track_info[MKW_N_TRACKS];
extern const TrackInfo_t arena_info[MKW_N_ARENAS];

static inline const TrackInfo_t * GetTrackInfo ( uint tidx )
{
    return tidx < MKW_N_TRACKS
		? track_info + tidx
		: tidx < MKW_N_TRACKS + MKW_N_ARENAS
			? arena_info + tidx - MKW_N_TRACKS
			: 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////   struct MusicInfo_t   ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// [[MusicInfo_t]] (generic code)

typedef struct MusicInfo_t
{
    uint	id;		// ID
    uint	flags;		// 1:track, 2:arena, 4:retro
    int		track;		// -1:no, 0-31:track, 32-41:arena
    char	name[8];	// short name

} MusicInfo_t;

extern const MusicInfo_t music_info[MKW_N_MUSIC+1];
extern const KeywordTab_t music_keyword_tab[];


//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////   E N D   //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_DB_MKW_H

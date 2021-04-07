
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
 *   Copyright (c) 2011-2021 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include "lib-ctcode.h"
#include "lib-lecode.h"
#include "lib-szs.h"
#include "lib-xbmg.h"
#include "lib-bzip2.h"
#include "lib-image.h"
#include "dclib-utf8.h"

#include "db-ctcode.h"
#include "lib-ctdata.c"
#include "ctcode.inc"
#include <locale.h>

#define TNAME_BUF_SIZE	1000
#define HIDDEN_CUP	0x7f

//
///////////////////////////////////////////////////////////////////////////////
///////////////			helpers				///////////////
///////////////////////////////////////////////////////////////////////////////

int MusicID2TrackId ( uint music_id, int no_track, int not_found )
{
    if ( music_id >= MKW_MUSIC_MIN_ID && music_id <= MKW_MUSIC_MAX_ID )
    {
	music_id = ( music_id - MKW_MUSIC_MIN_ID ) / 2;
	DASSERT( music_id < MKW_N_MUSIC );
	const int tid = music_info[music_id].track;
	return tid < 0 ? no_track : tid;
    }
    return not_found;
}

///////////////////////////////////////////////////////////////////////////////

void SetName16 ( u16 *dest, ccp src )
{
    DASSERT(dest);
    DASSERT(src);

    memset(dest,0,CT_NAME_SIZE*sizeof(*dest));
    ScanString16BMG(dest,CT_NAME_SIZE-1,src,-1,0);
}

///////////////////////////////////////////////////////////////////////////////

static void SetTrackBMG
(
    ctcode_t	*ctcode,	// valid CTCODE
    bmg_t	*bmg,		// destination BMG
    uint	tidx,		// index of the track
    ccp		text		// any text
)
{
    DASSERT(ctcode);
    DASSERT(bmg);

    u16 *dest = ctcode->crs->data[tidx].tname;
    memset(dest,0,CT_NAME_SIZE*sizeof(*dest));

    if ( text && *text )
    {
	bmg_item_t *bi = InsertItemBMG(bmg,ctcode->ctb.track_name1.beg+tidx,0,0,0);
	DASSERT(bi);
	AssignItemScanTextBMG(bi,text,-1);

	const uint max_len = bi->len < CT_NAME_SIZE-1 ? bi->len : CT_NAME_SIZE-1;
	memcpy(dest,bi->text,max_len*sizeof(*dest));
    }
    else
    {
	bmg_item_t *bi = FindItemBMG(bmg,ctcode->ctb.track_name1.beg+tidx);
	if (bi)
	    FreeItemBMG(bi);
    }
}

//-----------------------------------------------------------------------------

static inline void SetTrackFile
(
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    ccp		text		// any text
)
{
    SetTrackBMG(ctcode,&ctcode->track_file,tidx,text);
}

//-----------------------------------------------------------------------------

static inline void SetTrackString
(
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    ccp		text		// any text
)
{
    SetTrackBMG(ctcode,&ctcode->track_string,tidx,text);
}

//-----------------------------------------------------------------------------

static inline void SetTrackIdentifier
(
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    ccp		text		// any text
)
{
    if ( !text || !*text || !strcmp(text,"-") )
	text = 0;
    SetTrackBMG(ctcode,&ctcode->track_ident,tidx,text);
}

///////////////////////////////////////////////////////////////////////////////

// [[file-name]]
static void SetTrackInfo16
(
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    const u16	*text,		// any text
    int		len		// length of 'text', or -1
)
{
    DASSERT(ctcode);
    DASSERT(text);

    if ( len < 0 )
	len = GetLength16BMG(text,1000);

    u16 *dest = ctcode->crs->data[tidx].tname;
    memset(dest,0,CT_NAME_SIZE*sizeof(*dest));

    if ( text && len )
    {
	bmg_item_t *bi = InsertItemBMG( &ctcode->track_string,
					ctcode->ctb.track_name1.beg+tidx,0,0,0);
	DASSERT(bi);
	AssignItemText16BMG(bi,text,len);

	const uint max_len = bi->len < CT_NAME_SIZE-1 ? bi->len : CT_NAME_SIZE-1;
	memcpy(dest,bi->text,max_len*sizeof(*dest));
    }
    else
    {
	bmg_item_t *bi = FindItemBMG(&ctcode->track_string,
					ctcode->ctb.track_name1.beg+tidx);
	if (bi)
	    FreeItemBMG(bi);
    }
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetRawTrackInfo
(
    char	*buf,		// dest buffer
    uint	size,		// size of 'buf'
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    const bmg_item_t *bi	// NULL or bmg text
)
{
    DASSERT(buf);
    DASSERT(size);
    DASSERT(ctcode);

// [[file-name]]
    if ( !bi || !bi->len )
	bi = FindItemBMG(&ctcode->track_string,tidx+ctcode->ctb.track_name1.beg);

    const u16 *src;
    uint len;
    if ( bi && bi->len )
    {
	src = bi->text;
	len = bi->len;
    }
    else
    {
	src = ctcode->crs->data[tidx].tname;
	len = GetLength16BMG(src,CT_NAME_SIZE);
    }

    const u16 *end = src + len;
    char *dest = buf;
    char *dest_end = buf + size - 4;
    while ( dest < dest_end && src < end )
	dest = PrintUTF8Char(dest,ntohs(*src++));
    *dest = 0;
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetTrackFile
(
    char	*buf,		// dest buffer
    uint	size,		// size of 'buf'
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    const bmg_item_t *bi	// NULL or bmg text
)
{
    DASSERT(buf);
    DASSERT(size);
    DASSERT(ctcode);

    if ( !bi || !bi->len )
	bi = FindItemBMG(&ctcode->track_file,tidx+ctcode->ctb.track_name1.beg);

    if ( bi && bi->len )
	PrintString16BMG(buf,size,bi->text,bi->len,BMG_UTF8_MAX,0,opt_bmg_colors);
    else
    {
	const u16 *src = ctcode->crs->data[tidx].tname;
	const uint len = GetLength16BMG(src,CT_NAME_SIZE);
	PrintString16BMG(buf,size,src,len,BMG_UTF8_MAX,0,opt_bmg_colors);
    }
    return buf;
}

//-----------------------------------------------------------------------------

static ccp GetTrackString
(
    char	*buf,		// dest buffer
    uint	size,		// size of 'buf'
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    const bmg_item_t *bi	// NULL or bmg text
)
{
    DASSERT(buf);
    DASSERT(size);
    DASSERT(ctcode);

    if ( !bi || !bi->len )
	bi = FindItemBMG(&ctcode->track_string,tidx+ctcode->ctb.track_name1.beg);

    if ( bi && bi->len )
	PrintString16BMG(buf,size,bi->text,bi->len,BMG_UTF8_MAX,0,opt_bmg_colors);
    else
	*buf = 0;
    return buf;
}

//-----------------------------------------------------------------------------

static ccp GetTrackIdentifier
(
    char	*buf,		// dest buffer
    uint	size,		// size of 'buf'
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    const bmg_item_t *bi	// NULL or bmg text
)
{
    DASSERT(buf);
    DASSERT(size);
    DASSERT(ctcode);

    if ( !bi || !bi->len )
	bi = FindItemBMG(&ctcode->track_ident,tidx+ctcode->ctb.track_name1.beg);

    if ( bi && bi->len )
	PrintString16BMG(buf,size,bi->text,bi->len,BMG_UTF8_MAX,0,opt_bmg_colors);
    else
	*buf = 0;
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

const u16 * GetTrackName16
(
    uint	*tlen,		// not nULL: store length of result here
    ctcode_t	*ctcode,	// valid CTCODE
    uint	tidx,		// index of the track
    const bmg_item_t *bi	// NULL or bmg text
)
{
    DASSERT(ctcode);

// [[file-name]]
    if ( !bi || !bi->len )
	bi = FindItemBMG(&ctcode->track_string,tidx+ctcode->ctb.track_name1.beg);

    if ( bi && bi->len )
    {
	if (tlen)
	    *tlen = bi->len;
	return bi->text;
    }

    const ctcode_crs1_data_t *td = ctcode->crs->data + tidx;
    if (tlen)
	*tlen = GetLength16BMG(td->tname,sizeof(td->tname)/sizeof(*td->tname));
    return td->tname;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintMusicID ( uint mid, bool force_hex )
{
    if ( !force_hex && mid >= MKW_MUSIC_MIN_ID && mid <= MKW_MUSIC_MAX_ID )
    {
	const uint mid0 = mid - MKW_MUSIC_MIN_ID;
	if (!(mid0&1))
	    return music_info[mid0/2].name;
    }

    if (!mid)
	return "0";

    const int bufsize = 8;
    char *buf = GetCircBuf(bufsize);
    snprintf(buf,bufsize,"0x%02x",mid);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintPropertyID ( uint tid, bool force_hex )
{
    if ( tid < MKW_N_TRACKS && !force_hex )
	return track_info[tid].abbrev;

    const int bufsize = 8;
    char *buf = GetCircBuf(bufsize);
    snprintf(buf,bufsize,"0x%02x",tid);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

static ctcode_cup1_data_t * GetCupData ( const ctcode_t * ctcode, uint cup_idx )
{
    DASSERT(ctcode);
    if ( cup_idx < ctcode->n_racing_cups )
	return ctcode->cup_racing + cup_idx;

    cup_idx -= ctcode->n_racing_cups;
    return cup_idx < ctcode->n_battle_cups ? ctcode->cup_battle + cup_idx : 0;
}

///////////////////////////////////////////////////////////////////////////////

static bool CheckLEFlagsUsage ( ctcode_t * ctcode )
{
    PRINT("USE_LE_FLAGS=%d\n",ctcode->use_le_flags);
    if (!ctcode->use_le_flags)
    {
	uint tid;
	for ( tid = 0; tid < CODE_MAX_TRACKS; tid++ )
	    if (ctcode->le_flags[tid])
	    {
		ctcode->use_le_flags = true;
		break;
	    }
	PRINT("USE_LE_FLAGS=%d\n",ctcode->use_le_flags);
    }

    return ctcode->use_le_flags;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CTCODE base interface		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeCTCODE ( ctcode_t * ctcode, ct_mode_t ct_mode )
{
    DASSERT(ctcode);
    memset(ctcode,0,sizeof(*ctcode));
    SetupCtBMG( &ctcode->ctb, ct_mode, opt_ct_mode );
    PRINT("CTCODE: %s\n",GetCtBMGIdentification(&ctcode->ctb,true));

    ctcode->use_lecode = lecode_enabled;
    ctcode->fname = EmptyString;
    InitializeBMG(&ctcode->track_file);
    InitializeBMG(&ctcode->track_string);
    InitializeBMG(&ctcode->track_ident);
}

///////////////////////////////////////////////////////////////////////////////

void ResetCTCODE ( ctcode_t * ctcode )
{
    DASSERT(ctcode);
    ResetBMG(&ctcode->track_file);
    ResetBMG(&ctcode->track_string);
    ResetBMG(&ctcode->track_ident);
    ResetDataCTCODE(ctcode,false);
    FreeString(ctcode->fname);
    FreeString(ctcode->next_cup_name);
    InitializeCTCODE(ctcode,ctcode->ctb.ct_mode);
}

///////////////////////////////////////////////////////////////////////////////

static void SetupUsedSlotList ( u8 *slot_list, uint n_slots, uint le_phase )
{
    DASSERT(slot_list);
    DASSERT(n_slots>=0x100);

    memset(slot_list, 0, n_slots );
    if (le_phase)
    {
	memset( slot_list + MKW_ARENA_BEG,
		2,
		LE_FIRST_LOWER_CT_SLOT - MKW_ARENA_BEG );
	if ( le_phase == 1 )
	{
	    const uint max = n_slots < LE_FIRST_UPPER_CT_SLOT
			   ? n_slots : LE_FIRST_UPPER_CT_SLOT;
	    if ( max > 0xff )
		memset(slot_list+0xff,2,max-0xff); // 0xff == random
	}
    }
    else
    {
	memset( slot_list + MKW_ARENA_BEG, 2, MKW_N_ARENAS );
	memset( slot_list + 0xff, 2, n_slots - 0xff );

	slot_list[0x36] = 2;  // ring_mission = Galaxy Colosseum (music_id=0xc9)
	slot_list[0x37] = 2;  // winningrun_demo
	slot_list[0x38] = 2;  // loser_demo
	slot_list[0x39] = 2;  // draw_demo
	slot_list[0x3a] = 2;  // ending_demo

	slot_list[0x42] = 2;  // invalid -> can be used? (99%)
	slot_list[0x43] = 2;  // no track selected
    }
}

//-----------------------------------------------------------------------------

// 0:free, 1:used, 2:reserved
u8 ctcode_used_slot[USED_SLOT_MAX] = {0x7f};

static void SetupUsedSlots()
{
    if ( ctcode_used_slot[0] == 0x7f  )
	SetupUsedSlotList(ctcode_used_slot,sizeof(ctcode_used_slot),0);
}

//-----------------------------------------------------------------------------

void SetupUsedSlotsCTCODE ( ctcode_t * ctcode )
{
    SetupUsedSlots();
    if ( ctcode->ctb.le_phase )
    {
	SetupUsedSlotList( ctcode->used_slot,
				sizeof(ctcode->used_slot), ctcode->ctb.le_phase );
	ctcode->n_tracks = LE_FIRST_LOWER_CT_SLOT;
    }
    else
    {
	SetupUsedSlotList(ctcode->used_slot,sizeof(ctcode->used_slot),0);
	memcpy(ctcode->used_slot,ctcode_used_slot,sizeof(ctcode_used_slot));
	ctcode->n_tracks = 0x3b;
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetDataCTCODE ( ctcode_t * ctcode, bool create_empty )
{
    DASSERT(ctcode);

    //--- cups (CUP1)

    if (ctcode->cup_alloced)
    {
	ctcode->cup_alloced = false;
	FREE(ctcode->cup);
    }
    ctcode->cup			= 0;
    ctcode->cup_size		= 0;

    ctcode->cup_racing		= 0;
    ctcode->n_racing_cups	= 0;
    ctcode->max_racing_cups	= 0;

    ctcode->cup_battle		= 0;
    ctcode->n_battle_cups	= 0;
    ctcode->max_battle_cups	= 0;

    ctcode->n_unused_cups	= 0;


    //--- tracks (CRS1)

    if (ctcode->crs_alloced)
    {
	ctcode->crs_alloced = false;
	FREE(ctcode->crs);
    }
    ctcode->crs			= 0;
    ctcode->crs_size		= 0;

    ctcode->n_tracks		= 0;
    ctcode->max_tracks		= 0;


    //--- misc

    if (ctcode->lpar)
    {
	FREE(ctcode->lpar);
	ctcode->lpar = 0;
    }

    SetupUsedSlotsCTCODE(ctcode);
    CalcCupRefCTCODE(ctcode,true);
 #if USE_NEW_CONTAINER_CTC
    ResetContainer(&ctcode->container);
 #else
    FreeContainer(ctcode->old_container);
    ctcode->old_container = 0;
 #endif



    //--- create empty image?

    if (!create_empty)
	return;

    char namebuf[100]; // no more chars needed


    //--- create empty cups

    const uint max_battle_cups	= 2;
    const uint max_racing_cups	= ( ctcode->ctb.le_phase
					? LE_CODE_MAX_CUPS : CT_CODE_DEFAULT_CUPS )
				- max_battle_cups ;

    ctcode->cup_size	= sizeof(ctcode_cup1_head_t)
			+ (max_racing_cups+max_battle_cups) * sizeof(ctcode_cup1_data_t);
    ctcode->cup		= CALLOC(ctcode->cup_size,1);
    ctcode->cup_racing	= ctcode->cup->data;
    ctcode->cup_battle	= ctcode->cup_racing + max_racing_cups;
    memcpy(ctcode->cup,cup1_header,sizeof(cup1_header));

    //ctcode->n_racing_cups = 0;
    ctcode->max_racing_cups = max_racing_cups;
    ctcode->n_unused_cups   = max_racing_cups;
    ctcode->n_battle_cups   = max_battle_cups;
    ctcode->max_battle_cups = max_battle_cups;

    uint cidx, tidx = MKW_ARENA_BEG;
    for ( cidx = 0; cidx < max_battle_cups; cidx++ )
    {
	ctcode_cup1_data_t *cd = ctcode->cup_battle + cidx;
	snprintf(namebuf,sizeof(namebuf),"Battle %u",cidx+1);
	SetName16(cd->name,namebuf);

	if ( !ctcode->ctb.le_phase )
	{
	    uint t;
	    for ( t = 0; t < 5; t++ )
		cd->track_id[t] = htonl(tidx++);
	}
    }


    //--- create empty tracks: data

    const uint max_tracks = ctcode->ctb.le_phase ? LE_CODE_MAX_TRACKS : CT_CODE_MAX_TRACKS;
    ctcode->crs_size = sizeof(ctcode_crs1_head_t)
		     + max_tracks * sizeof(ctcode_crs1_data_t);
    ctcode->crs = CALLOC(ctcode->crs_size,1);

    struct tab_t
    {
	u32	tidx;
	u32	music_id;
	u32	property_id;
	ccp	name;
    };

    static const struct tab_t tab[] =
    {
	{ 0x36, 0xc9, 0x36, "ring_mission" },
	{ 0x37,    0,    0, "winningrun_demo" },
	{ 0x38,    0,    0, "loser_demo" },
	{ 0x39,    0,    0, "draw_demo" },
	{ 0x3a, 0x75, 0x08, "ending_demo" },
    //	{ 0x42,    0,    0, "invalid" },
	{ 0x43,    0,    0, "- " TOOLSET_LONG " v" VERSION " r" REVISION },
	{0,0,0,0}
    };

    const struct tab_t *ptr;
    for ( ptr = tab; ptr->name; ptr++ )
    {
	ctcode_crs1_data_t *td = ctcode->crs->data + ptr->tidx;
	td->music_id = htonl(ptr->music_id);
	td->property_id = htonl(ptr->property_id);

	ccp src = ptr->name;
	if ( *src == '-' )
	    src++;
	else
	    strcpy(td->filename,ptr->name);

	char buf[200], *dest = buf;
	bool upper = true;
	while (*src)
	{
	    int ch = (uchar)*src++;
	    if ( ch == '_' )
	    {
		upper = true;
		ch = ' ';
	    }
	    else if (upper)
	    {
		upper = false;
		ch = toupper((int)ch);
	    }
	    *dest++ = ch;
	}
	*dest = 0;
	SetTrackString(ctcode,ptr->tidx,buf);
    }


    //--- create empty tracks: header

    memcpy(ctcode->crs,crs1_header,sizeof(crs1_header));

    uint i;
    const u32 val = htonl(crs1_property);
    for ( i = 0; i < 8; i++ )
	ctcode->crs->property[i] = val;

    //ctcode->n_tracks	    = 0;
    ctcode->max_tracks	    = max_tracks;
    ctcode->used_arenas     = 5 * max_battle_cups;

    ctcode_crs1_data_t *td = ctcode->crs->data;
    for ( tidx = 0; tidx < max_tracks; tidx++, td++ )
    {
	if ( tidx >= MKW_N_TRACKS && tidx < MKW_N_TRACKS + MKW_N_ARENAS )
	{
	    const TrackInfo_t *info = arena_info + (tidx-MKW_N_TRACKS);
	    SetName16(td->tname,info->name_en);
	    StringCopyS(td->filename,sizeof(td->filename),info->track_fname);

	    if ( ctcode->ctb.le_phase )
	    {
		td->music_id	= htonl(arena_info[tidx-MKW_N_TRACKS].music_id);
		td->property_id	= htonl(tidx);
	    }
	}
	else if (!ctcode->used_slot[tidx])
	{
	    snprintf(namebuf,sizeof(namebuf),"Slot %u = 0x%03x",tidx,tidx);
	    SetName16(td->tname,namebuf);
	    snprintf(namebuf,sizeof(namebuf),"slot_%02x",tidx);
	    StringCopyS(td->filename,sizeof(td->filename),namebuf);

	    td->music_id	= htonl(0x75);
	    td->property_id	= htonl(0);
	    td->cup_id		= htonl(0xff);
	}
    }


    //--- LE-CODE: battles again

    if ( ctcode->ctb.le_phase )
    {
	ctcode_cup1_data_t *cd = ctcode->cup_battle;
	uint aid;
	for ( aid = 0; aid < MKW_N_ARENAS; aid++ )
	{
	    uint tid = arena_info[aid].def_index;
	    cd[tid/5].track_id[tid%5] = htonl(aid+MKW_N_TRACKS);
	}
	//HexDump16(stdout,0,0,cd[0].track_id,sizeof(cd[0].track_id));
	//HexDump16(stdout,0,0,cd[1].track_id,sizeof(cd[1].track_id));
    }


    //--- terminate

    ctcode->cupref_dirty = false;

    PRINT("STAT/CUP: racing: %u/%u, battle: %u/%u, unused: %u\n",
		ctcode->n_racing_cups, ctcode->max_racing_cups,
		ctcode->n_battle_cups, ctcode->max_battle_cups,
		ctcode->n_unused_cups );
    PRINT("STAT/TRACK: %u/%u\n", ctcode->n_tracks, ctcode->max_tracks );
}

///////////////////////////////////////////////////////////////////////////////

void CalcCupRefCTCODE ( ctcode_t * ctcode, bool force )
{
    DASSERT(ctcode);
    if ( !force && !ctcode->cupref_dirty )
	return;


    //--- clear data

    ctcode->used_tracks = 0;
    ctcode->used_arenas = 0;
    memset(ctcode->cupref,0,sizeof(ctcode->cupref));


    //--- source available?

    if ( !ctcode->cup || !ctcode->crs )
    {
	ctcode->cupref_dirty = true;
	return;
    }
    ctcode->cupref_dirty = false;


    //--- cup loop

    uint cidx;
    for ( cidx = 0; cidx < ctcode->n_racing_cups + ctcode->n_battle_cups; cidx++ )
    {
	const bool is_battle = cidx >= ctcode->n_racing_cups;
	const ctcode_cup1_data_t *cd = GetCupData(ctcode,cidx);
	DASSERT(cd);

	uint ti;
	for ( ti = 0; ti < 4 + is_battle; ti++ )
	{
	    const uint tidx = be32(cd->track_id+ti);
	    if ( tidx < ctcode->max_tracks )
	    {
		ctcode_cupref_t *cr = ctcode->cupref + tidx;
		if (!cr->n)
		{
		    if (is_battle)
			ctcode->used_arenas++;
		    else
			ctcode->used_tracks++;
		}

		if (is_battle)
		    cr->n_battle++;
		else
		    cr->n_racing++;

		if ( cr->n < CT_CODE_MAX_CUPREF )
		{
		    cr->cup[cr->n] = is_battle
					? cidx	- ctcode->n_racing_cups
						+ ctcode->max_racing_cups
					: cidx;
		    cr->idx[cr->n] = ti;
		    cr->n++;
		}
	    }
	}
    }

    PRINT("STAT/REF: nT=%u, nA=%u, total=%u/%u\n",
		ctcode->used_tracks, ctcode->used_arenas,
		ctcode->n_tracks, ctcode->max_tracks );

    //HexDump16(stderr,0,0x16,ctcode->cupref+0,sizeof(*ctcode->cupref));
    //HexDump16(stderr,0,0x16,ctcode->cupref+1,sizeof(*ctcode->cupref));


    //--- hidden tracks: create reference

    ParamField_t ref;
    InitializeParamField(&ref);
    char namebuf[TNAME_BUF_SIZE];

    uint tidx;
    for ( tidx = 0; tidx < ctcode->n_tracks; tidx++ )
    {
	if (ctcode->hidden[tidx])
	    continue;

	GetTrackIdentifier(namebuf,sizeof(namebuf),ctcode,tidx,0);
	if (*namebuf && !FindParamField(&ref,namebuf) )
	    InsertParamField(&ref,namebuf,false,tidx,0);
    }


    //--- hidden tracks: use reference

    for ( tidx = 0; tidx < ctcode->n_tracks; tidx++ )
    {
	if (!ctcode->hidden[tidx])
	    continue;

	GetTrackIdentifier(namebuf,sizeof(namebuf),ctcode,tidx,0);
	if (*namebuf)
	{
	    const ParamFieldItem_t *it = FindParamField(&ref,namebuf);
	    if (it)
		memcpy(	ctcode->cupref+tidx,
			ctcode->cupref+it->num,
			sizeof(*ctcode->cupref) );
	}
    }

    ResetParamField(&ref);
}

///////////////////////////////////////////////////////////////////////////////

void PrepareExportCTCODE ( ctcode_t * ctcode )
{
    DASSERT(ctcode);
    DASSERT(ctcode->cup);
    DASSERT(ctcode->cup_racing);
    DASSERT(ctcode->cup_battle);
    DASSERT(ctcode->crs);

    if ( !ctcode->cup || !ctcode->cup_racing || !ctcode->cup_battle || !ctcode->crs )
	return;

    //--- setup ctcode header

    ctcode_header_t *ch = &ctcode->ct_head;
    memcpy(ch,ctcode_header,sizeof(*ch));
    u32 off = sizeof(*ch);


    //--- setup CUP1

    DASSERT( ctcode->max_racing_cups >= ctcode->n_racing_cups );
    DASSERT( ctcode->cup_racing == ctcode->cup->data );
    ctcode->max_racing_cups = ctcode->n_racing_cups;

    ctcode_cup1_data_t *battle = ctcode->cup_racing + ctcode->n_racing_cups;
    DASSERT( ctcode->cup_battle >= battle );
    if ( ctcode->cup_battle > battle )
    {
	memmove( battle,ctcode->cup_battle, ctcode->n_battle_cups * sizeof(*battle) );
	ctcode->cup_battle = battle;
	ctcode->cupref_dirty = true;
    }

    ctcode->cup->n_racing_cups = htonl(ctcode->n_racing_cups);
    ctcode->cup->n_battle_cups = htonl(ctcode->n_battle_cups);
    const u8 *end = (u8*)(battle + ctcode->n_battle_cups);
    DASSERT( end <= (u8*)ctcode->cup + ctcode->cup_size );
    ctcode->cup_size = end - (u8*)ctcode->cup;
    ctcode->cup->size = htonl(ctcode->cup_size);
    ch->sect_info[0].off = htonl(off);
    off += ctcode->cup_size;


    //--- setup CRS1

    DASSERT( ctcode->max_tracks >= ctcode->n_tracks );
    ctcode->max_tracks = ctcode->n_tracks;
    end = (u8*)( ctcode->crs->data + ctcode->n_tracks );
    DASSERT( end <= (u8*)ctcode->crs + ctcode->crs_size );
    ctcode->crs_size =  end - (u8*)ctcode->crs;
    ctcode->crs->size = htonl(ctcode->crs_size);
    ctcode->crs->n_tracks = htonl(ctcode->n_tracks);
    ch->sect_info[1].off = htonl(off);
    //off += ctcode->crs_size;
}

///////////////////////////////////////////////////////////////////////////////

void PatchNamesCTCODE ( ctcode_t * ctcode )
{
    uint cidx;
    for ( cidx = 0; cidx < ctcode->n_racing_cups + ctcode->n_battle_cups; cidx++ )
    {
	const bmg_item_t *bi
		= FindItemBMG(&opt_load_bmg,ctcode->ctb.rcup_name.beg+cidx);
	if ( bi && bi->len )
	{
	    ctcode_cup1_data_t *cd = GetCupData(ctcode,cidx);
	    memset(cd->name,0,sizeof(cd->name));
	    const uint len = bi->len < CT_NAME_SIZE-1 ? bi->len : CT_NAME_SIZE-1;
	    memcpy(cd->name,bi->text,len*sizeof(*cd->name));
	}
    }

    uint tidx;
    for ( tidx = 0; tidx < ctcode->n_tracks; tidx++ )
    {
	const bmg_item_t *bi
	    = FindItemBMG(&opt_load_bmg,ctcode->ctb.track_name1.beg+tidx);
	if ( bi && bi->len )
	    SetTrackInfo16(ctcode,tidx,bi->text,bi->len);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PrepareSaveCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp  opt_image_dir		= 0;
bool opt_dynamic_ctcode		= false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum { MAX_KEY = 200 };
typedef struct track_info_t
{
    char key[MAX_KEY];	// sort key
    uint order;		// index reflecting inital order
    uint tidx;		// track index
}
track_info_t;

static int cmp_track_info ( const track_info_t * a, const track_info_t * b )
{
    const int stat = strcoll(a->key,b->key);
    return stat ? stat : a->order < b->order ? -1 : 1;
}

///////////////////////////////////////////////////////////////////////////////

enumError ReorderTracksCTCODE ( ctcode_t * ctcode )
{
    DASSERT(ctcode);
    PRINT("ReorderTracksCTCODE()\n");
    setlocale(LC_COLLATE,"");

    if ( !opt_order_by || !*opt_order_by )
	return ERR_NOTHING_TO_DO;

    bmg_t bmg;
    enumError err = LoadXBMG(&bmg,true,opt_order_by,true,false);
    if (!err)
    {
	track_info_t info[CODE_MAX_TRACKS];
	memset(info,0,sizeof(info));

	track_info_t *ptr = info;
	uint cidx;
	for ( cidx = opt_order_all ? 0 : 8;
		cidx < ctcode->n_racing_cups; cidx++ )
	{
	    const ctcode_cup1_data_t *cd = ctcode->cup->data + cidx;

	    uint ti;
	    for ( ti = 0; ti < 4; ti++, ptr++ )
	    {
		DASSERT( ptr < info + ctcode->max_tracks );
		const uint tidx = be32(cd->track_id+ti);
		const bmg_item_t *bi
			= FindItemBMG(&bmg,tidx+ctcode->ctb.track_name1.beg);
		if ( bi && bi->len )
		{
		    *ptr->key = 'A';
		    PrintString16BMG(ptr->key+1,sizeof(ptr->key)-1,
				bi->text,bi->len,BMG_UTF8_MAX,0,opt_bmg_colors);
		}
		else
		    *ptr->key = 'B';
		ptr->tidx = tidx;
		ptr->order = ptr - info;

		noPRINT("KEY: %02x %s\n",*ptr->key,ptr->key+1);
	    }
	}

	const uint n = ptr - info;
	if ( n > 1 )
	    qsort( info, n, sizeof(*info), (qsort_func)cmp_track_info );

	track_info_t *src = info;
	for ( cidx = opt_order_all ? 0 : 8;
		cidx < ctcode->n_racing_cups; cidx++ )
	{
	    ctcode_cup1_data_t *cd = ctcode->cup->data + cidx;

	    uint ti;
	    for ( ti = 0; ti < 4; ti++, src++ )
	    {
		noPRINT("KEY: %3u,%02x: %02x %s\n",
			src->order, src->tidx, *src->key, src->key+1 );
		write_be32(cd->track_id+ti,src->tidx);
	    }
	}
    }

    ResetBMG(&bmg);
    CalcCupRefCTCODE(ctcode,true);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

void PrepareSaveCTCODE ( ctcode_t * ctcode )
{
    DASSERT(ctcode);
    PRINT("PrepareSaveCTCODE()\n");

    if (ctcode->save_prepared)
	return;
    ctcode->save_prepared++;

    ReorderTracksCTCODE(ctcode);
    if (opt_write_tracks)
	SaveTrackTableCTCODE(ctcode,opt_write_tracks,false);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			DumpCTCODE()			///////////////
///////////////////////////////////////////////////////////////////////////////

static void hexdump
	( FILE *f, uint indent, const void *vdata, uint size, ccp title )
{
    DASSERT(f);
    DASSERT(title);

    if ( !vdata || !size )
	return;

    fprintf(f,"\n%*s%s\n",indent+2,"",title);
    const u8* data = vdata;

    uint addr;
    for ( addr = 0; addr < size; )
    {
	fprintf(f,"%*x:",indent+6,addr);
	uint i;
	for ( i = 0; i < 4 && addr < size; i++, addr+=4 )
	    fprintf(f," %8x",be32(data+addr));
	fputc('\n',f);
    }
};

//-----------------------------------------------------------------------------

void DumpCTCODE ( FILE *f, uint indent, ctcode_t * ctcode )
{
    DASSERT(f);
    DASSERT(ctcode);
    indent = NormalizeIndent(indent);
    const uint numindent = indent + 5;
    char namebuf[TNAME_BUF_SIZE];


    //--- statistics

    fprintf(f,"\n%*sStatistics:\n\n",indent,"");
    fprintf(f,"%*u racing cups defined.\n",numindent,ctcode->n_racing_cups);
    fprintf(f,"%*u battle cups defined.\n",numindent,ctcode->n_battle_cups);
    fprintf(f,"%*u tracks defined.\n",numindent,ctcode->n_tracks);
    fprintf(f,"%*u track slots available.\n",numindent,ctcode->max_tracks);


    //--- track reference

    u8 count[CODE_MAX_TRACKS] = {0};
    u8 usage[CODE_MAX_TRACKS] = {0};
    uint invalid_track_ref = 0;
    const uint max = ctcode->max_tracks;

    uint cidx;
    for ( cidx = 0; cidx < ctcode->n_racing_cups; cidx++ )
    {
	const ctcode_cup1_data_t *cd = ctcode->cup_racing + cidx;
	DASSERT(cd);
	uint ti;
	for ( ti = 0; ti < 4; ti++ )
	{
	    uint tidx = be32(cd->track_id+ti);
	    if ( tidx < max )
	    {
		count[tidx]++;
		usage[tidx] |= 1;
	    }
	    else
		invalid_track_ref++;
	}
    }

    for ( cidx = 0; cidx < ctcode->n_battle_cups; cidx++ )
    {
	const ctcode_cup1_data_t *cd = ctcode->cup_battle + cidx;
	DASSERT(cd);
	uint ti;
	for ( ti = 0; ti < 5; ti++ )
	{
	    uint tidx = be32(cd->track_id+ti);
	    if ( tidx < max )
	    {
		count[tidx]++;
		usage[tidx] |= 2;
	    }
	    else
		invalid_track_ref++;
	}
    }

    if (invalid_track_ref)
	fprintf(f,"%*u invalid `cup to track´ references.\n",
			numindent, invalid_track_ref );

    fprintf(f,"\n%*sTrack usage:\n\n",indent,"");
    uint tidx = 0;
    while ( tidx < max )
    {
	uint end = tidx + 0x40;
	if ( end > max )
	    end = max;
	fprintf(f,"%*s%02x-%02x:",indent+2,"",tidx,end-1);
	uint i;
	for ( i = 0; i < 0x40 && tidx < max; i++, tidx++ )
	{
	    if (!(i%8))
		fputc(' ',f);
	    if (!usage[tidx])
	    {
	     #if 1
		fputc('-',f);
	     #else
		ctcode_crs1_data_t *td = ctcode->crs->data + tidx;
		fputc( td->filename[0] && td->filename[1] ? '+' : '-', f );
	     #endif
	    }
	    else if ( usage[tidx] == 1 )
	    {
		const uint n = count[tidx];
		if ( n < 10 )
		    fputc('0'+n,f);
		else
		    fputc('*',f);
	    }
	    else if ( usage[tidx] == 2 )
	    {
		const uint n = count[tidx] < 26 ? count[tidx] : 26;
		fputc('a'-1+n,f);
	    }
	    else
		fputc('?',f);
	}
	fputc('\n',f);
    }


    //--- memory dump

    PrepareExportCTCODE(ctcode);

    if ( ctcode->fform == FF_CT_TEXT )
	fprintf(f,"\n%*sSections in virtual binary code:\n\n",indent,"");
    else
	fprintf(f,"\n%*sSections:\n\n",indent,"");

    const ctcode_header_t *ch = &ctcode->ct_head;
    const u32 n_sect = be32(&ch->n_sect);

    uint sect;
    for ( sect = 0; sect < n_sect; sect++ )
    {
	u32 off = be32(&ch->sect_info[sect].off);
	if (!off)
	    continue;
	fprintf(f,"%*u: %6x : %s\n",
		numindent, sect,
		off, PrintID(ch->sect_info[sect].name,4,0) );
    }


    //--- hexdump of headers

    if (long_count)
    {
	if ( ctcode->fform == FF_CT_TEXT )
	    fprintf(f,"\n%*sHexdump of headers in virtual binary code:\n",indent,"");
	else
	    fprintf(f,"\n%*sHexdump of headers:\n",indent,"");

	hexdump(f,indent,&ctcode->ct_head,sizeof(ctcode->ct_head),"CT-CODE Header");
	hexdump(f,indent,ctcode->cup,sizeof(*ctcode->cup),"Section CUP1 Header");
	hexdump(f,indent,ctcode->crs,sizeof(*ctcode->crs),"Section CRS1 Header");
    }


    //--- dump a track listing

    if ( long_count > 2 )
    {
	fprintf(f,"\n%*sTechnical Track list:\n\n",indent,"");
	CalcCupRefCTCODE(ctcode,false);

	const ctcode_cupref_t *cr = ctcode->cupref;

	uint tidx;
	for ( tidx = 0; tidx < ctcode->max_tracks; tidx++, cr++ )
	{
	    if (cr->n_racing)
	    {
		const bmg_item_t *bi
			= FindItemBMG(&opt_load_bmg,tidx+ctcode->ctb.track_name1.beg);
		GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,bi);
		const ctcode_crs1_data_t *td = ctcode->crs->data + tidx;

		fprintf(f,"%*s%s [M=%s,P=%s,S=0x%02x,F=%s]\n",
			indent+2,"", namebuf,
			PrintMusicID(ntohl(td->music_id),false),
			PrintPropertyID(ntohl(td->property_id),false),
			tidx,
			td->filename );
	    }
	}
    }


    //--- terminate

    fputc('\n',f);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			setup ctcode parser		///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsCTCODE()
{
    static VarMap_t vm = {0};
    if (!vm.used)
	DefineMkwVars(&vm);
    return &vm;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan CTCODE helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanSetup ( ctcode_t *ctcode, ScanInfo_t * si )
{
    // nothing to do (ignore all yet)
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static char * ReplaceName ( char * str, ccp search, ccp replace )
{
    ccp pos = strstr(str,search);
    if ( !pos || !*search )
	return str;

    char fb_buf[1000];
    FastBuf_t *fb = InitializeFastBuf(fb_buf,sizeof(fb_buf));


    const int search_len = strlen(search);
    const int repl_len = strlen(replace);
    char *src = str;
    do
    {
	AppendFastBuf(fb,src,pos-src);
	AppendFastBuf(fb,replace,repl_len);
	src = (char*)pos + search_len;
	pos = strstr(src,search);

    } while (pos);

    AppendFastBuf(fb,src,strlen(src));
    FreeString(str);
    return MoveFromFastBufString(fb);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int InsertTrackIntoCup ( ctcode_t *ctcode, uint tidx, bool hidden );

static void AddWiimmCup ( ctcode_t *ctcode )
{
    PRINT("+++++ ADD WIIMM CUP +++++\n");
    ctcode->add_wiimm_cup = 0;

    uint slot;
    for ( slot = LE_SLOT_RND_ALL; slot <= LE_SLOT_RND_NEW; slot++ )
    {
	ctcode->used_slot[slot] = 1;
	if ( ctcode->n_tracks < slot+1 )
	    ctcode->n_tracks = slot+1;

	ctcode_crs1_data_t *td = ctcode->crs->data + slot;
	td->cup_id = htonl( InsertTrackIntoCup(ctcode,slot,false) );
    }
}

///////////////////////////////////////////////////////////////////////////////

static int InsertTrackIntoCup ( ctcode_t *ctcode, uint tidx, bool hidden )
{
    DASSERT(ctcode);
    DASSERT( tidx < ctcode->max_tracks );

// [[2do]] [[hidden]]

    if (hidden)
    {
	ctcode->tracks_defined++;
	return HIDDEN_CUP;
    }

    uint cidx = ctcode->n_racing_cups;
    if (ctcode->track_of_cup)
    {
	DASSERT( cidx > 0 );
	DASSERT( ctcode->track_of_cup < 4 );
	cidx--;
    }
    else if ( ctcode->add_wiimm_cup
		&& ctcode->ctb.le_phase
		&& cidx == 8 )
    {
	AddWiimmCup(ctcode);
	// redo this function again
	return InsertTrackIntoCup(ctcode,tidx,hidden);
    }
    else
    {
	if ( cidx == ctcode->max_racing_cups )
	{
	    ERROR0(ERR_WARNING,"No more track cups available!\n");
	    return -1;
	}
	ctcode->n_racing_cups++;

	char buf[20];
	ctcode_cup1_data_t *cd = GetCupData(ctcode,cidx);
	ccp cname = ctcode->next_cup_name;
	if ( cname && *cname )
	    SetName16(cd->name,cname);
	else
	{
	    const bmg_item_t *bi
		= FindItemBMG(&opt_load_bmg,ctcode->ctb.rcup_name.beg+cidx);
	    if ( bi && bi->len )
	    {
		memset(cd->name,0,sizeof(cd->name));
		const uint len = bi->len < CT_NAME_SIZE-1 ? bi->len : CT_NAME_SIZE-1;
		memcpy(cd->name,bi->text,len*sizeof(*cd->name));
	    }
	    else
	    {
		snprintf(buf,sizeof(buf),"Cup %02x",cidx);
		SetName16(cd->name,buf);
	    }
	}
	FreeString(ctcode->next_cup_name);
	ctcode->next_cup_name = 0;
    }

    noPRINT("INSERT TRACK %u INTO CUP %u.%u/%u\n",
		tidx, cidx, ctcode->track_of_cup, ctcode->max_racing_cups );

    ctcode->cup_racing[cidx].track_id[ctcode->track_of_cup++] = htonl(tidx);
    if ( ctcode->track_of_cup == 4 )
	ctcode->track_of_cup = 0;

    ctcode->tracks_defined++;

    return cidx;
}

///////////////////////////////////////////////////////////////////////////////

static ctcode_crs1_data_t * GetNextTrackSlot ( ctcode_t *ctcode, bool hidden )
{
    DASSERT(ctcode);

    //--- next_slot available

    uint slot = 0;
    while (ctcode->n_next_slot)
    {
	slot = ctcode->next_slot[--ctcode->n_next_slot];
	if ( !ctcode->used_slot[slot] || ctcode->force_slot[ctcode->n_next_slot] )
	{
	 found:
	    ctcode->used_slot[slot] = 1;
	    if ( ctcode->n_tracks < slot+1 )
		ctcode->n_tracks = slot+1;

	    ctcode_crs1_data_t *td = ctcode->crs->data + slot;
	    td->cup_id = htonl( InsertTrackIntoCup(ctcode,slot,hidden) );
	    return td;
	}
    }

    //--- search first free slot

    for ( slot = 0; slot < ctcode->max_tracks; slot++ )
	if (!ctcode->used_slot[slot])
	    goto found;

    ERROR0(ERR_WARNING,"No more track slots available!\n");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static ctcode_crs1_data_t * GetNextArenaSlot
	( ctcode_t *ctcode, uint prop, ScanInfo_t * si )
{
    if (!ctcode->ctb.le_phase)
    {
	static int err_done = 0;
	if (!err_done++)
	    ERROR0(ERR_WARNING,"Battle arenas only supported for LE-CODE!\n");
	return 0;
    }

    const uint arena = prop - MKW_N_TRACKS;
    if ( arena >= MKW_N_ARENAS )
    {
	ERROR0(ERR_WARNING,
		"Invalid arena slot 0x%02x, use A11..A25 (0x%x..0x%x) instead: %s @%u\n",
		prop, MKW_N_TRACKS, MKW_N_TRACKS + MKW_N_ARENAS - 1,
		si->cur_file->name, si->cur_file->line );
	return 0;
    }

    if (ctcode->arena_usage[arena])
    {
	ERROR0(ERR_WARNING,
		"Arena slot 0x%02x already used: %s @%u\n",
		prop, si->cur_file->name, si->cur_file->line );
	return 0;
    }
    ctcode->arena_usage[arena]++;
    ctcode->assigned_arenas++;

    return ctcode->crs->data + prop;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanRTL_Option ( ctcode_t *ctcode, ScanInfo_t * si )
{
    DASSERT(ctcode);
    DASSERT(si);

    ScanFile_t *sf = si->cur_file;
    ccp start = sf->ptr;

    while ( NextCharSI(si,false) == '%' )
	sf->ptr++;

    char name[50];
    const uint namelen = ScanNameSI(si,name,sizeof(name),true,true,0);
    if (!namelen)
    {
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Missing name for option: [%s @%u]: %.*s\n",
			sf->name, sf->line, (int)(eol - start), start );
	}
	return ERR_WARNING;
    }

    if ( NextCharSI(si,false) == '=' )
	sf->ptr++;

    if (!strcmp(name,"LE-PHASE")) // [[obsolete]] since 2019-12
    {
	long num;
	enumError err = ScanUValueSI(si,&num,false);
	if (err)
	    return err;
	ctcode->use_lecode = num > 0;
	PRINT("%%LE-PHASE: lecode=%d\n",ctcode->use_lecode);
    }
    else if (!strcmp(name,"LE-FLAGS"))
    {
	long num;
	enumError err = ScanUValueSI(si,&num,false);
	if (err)
	    return err;
	ctcode->use_le_flags = num > 0;
	PRINT("%%LE-FLAGS = %d\n",ctcode->use_le_flags);
    }
    else if (!strcmp(name,"REPLACE-AT"))
    {
	long num;
	enumError err = ScanUValueSI(si,&num,false);
	if (err)
	    return err;
	ctcode->replace_at = num > 0;
	PRINT("%%REPLACE-AT = %d\n",ctcode->use_le_flags);
    }
    else if (!strcmp(name,"WIIMM-CUP"))
    {
	long num;
	enumError err = ScanUValueSI(si,&num,false);
	if (err)
	    return err;
	ctcode->add_wiimm_cup = num > 0;
	PRINT("%%WIIMM-CUP = %d\n",ctcode->use_le_flags);
    }
    else
    {
	if ( si->no_warn <= 0 )
	    ERROR0(ERR_WARNING,
			"Unknown option [%s @%u]: %s\n",
			sf->name, sf->line, name );
	return ERR_WARNING;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanRTL_Nintendo ( ctcode_t *ctcode, ScanInfo_t * si )
{
    DASSERT(ctcode);
    DASSERT(si);

    if (ctcode->tracks_defined)
    {
	ccp eol = FindNextLineFeedSI(si,true);
	ScanFile_t *sf = si->cur_file;
	ERROR0(ERR_WARNING,
		"The 'N' command can only be used before"
		" the first track definition [%s @%u]: %.*s\n",
		sf->name, ctcode->tracks_defined /*sf->line*/, (int)(eol - sf->ptr), sf->ptr );
	GotoEolSI(si);
	return ERR_WARNING;
    }

    DEFINE_VAR(val);
    enumError err = ScanExprSI(si,&val);
    if (err)
	return err;
    uint mode = GetIntV(&val);
    const uint flag_hex = mode & RTL_N_F_HEX;
    const uint flag_wii = mode & RTL_N_F_WII;

    struct coldef_t { uint flag; const char name[8]; };
    static const struct coldef_t coltab[] =
    {
	{ RTL_N_F_RED1,		"red1" },
	{ RTL_N_F_RED2,		"red2" },
	{ RTL_N_F_RED3,		"red3" },
	{ RTL_N_F_RED4,		"red4" },
	{ RTL_N_F_YELLOW,	"yellow" },
	{ RTL_N_F_GREEN,	"green" },
	{ RTL_N_F_BLUE1,	"blue1" },
	{ RTL_N_F_BLUE2,	"blue2" },
	{ RTL_N_F_WHITE,	"white" },
	{ RTL_N_F_CLEAR,	"clear" },

	{ 0, "" }
    };

    ccp col = 0;
    static const struct coldef_t *colptr;
    for ( colptr = coltab; colptr->flag; colptr++ )
    {
	if ( mode & colptr->flag )
	{
	    col = colptr->name;
	    break;
	}
    }

    mode &= RTL_N_M_MODE;
    if ( mode == RTL_N_HIDE || mode == RTL_N_SHOW || mode == RTL_N_SWAP )
    {
	const TrackInfo_t *info = track_info;
	ctcode_crs1_data_t *td = ctcode->crs->data;
	const u32 *tpos = mode == RTL_N_SWAP ? track_pos_swap : track_pos;

	uint ti;
	for ( ti = 0; ti < MKW_N_TRACKS; ti++, info++, td++ )
	{
	    //--- here we insert the correct cup index ...

	    const uint tidx = tpos[ti];
	    InsertTrackIntoCup(ctcode,tidx,false);

	    //--- and here we fill the track data independent of the cup index

	    td->music_id	= htonl(info->music_id);
	    td->property_id	= htonl(ti);
	    td->cup_id		= htonl(info->def_slot/10-1);

	    char buf1[50], buf2[80];

	    ccp name = info->name_en;
	    bool have_prefix = info->def_slot >= 50;
	    if ( !have_prefix && flag_wii )
	    {
		snprintf(buf1,sizeof(buf1),"Wii %s",name);
		name = buf1;
		have_prefix = true;
	    }

	    if ( have_prefix && col )
	    {
		ccp space = strchr(name,' ');
		if (space)
		{
		    snprintf(buf2,sizeof(buf2),"\\c{%s}%.*s \\c{OFF}%s",
				col, (int)(space-name), name, space+1 );
		    name = buf2;
		}
	    }
	    SetName16(td->tname,name);
	    SetTrackFile(ctcode,ti,info->track_fname);

	    if (flag_hex)
		snprintf(td->filename,sizeof(td->filename),"%02x",ti);
	    else
		StringCopyS(td->filename,sizeof(td->filename),info->track_fname);
	}
	memset(ctcode->used_slot,1,MKW_N_TRACKS);
	ctcode->tracks_defined = MKW_N_TRACKS;
	if ( mode == RTL_N_HIDE )
	    ctcode->n_racing_cups = 0;
	ctcode->cmd_n_used = true;
    }


    if ( ctcode->ctb.le_phase )
    {
	const TrackInfo_t *info = arena_info;
	uint ti;
	for ( ti = MKW_N_TRACKS; ti < MKW_N_TRACKS+MKW_N_ARENAS; ti++, info++ )
	    SetTrackFile(ctcode,ti,info->track_fname);

	// special files
	SetTrackFile(ctcode,0x36,"ring_mission");
	SetTrackFile(ctcode,0x37,"winningrun_demo");
	SetTrackFile(ctcode,0x38,"loser_demo");
	SetTrackFile(ctcode,0x39,"draw_demo");
	SetTrackFile(ctcode,0x3a,"ending_demo");
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanRTL_Slot
	( ctcode_t *ctcode, ScanInfo_t * si, bool mark_reserved )
{
    DASSERT(ctcode);
    DASSERT(si);

    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    sf->disable_comma++;

    enumError err = ERR_OK;

    uint idx = ctcode->max_tracks;
    for(;;)
    {
	char ch = NextCharSI(si,false);
	if (!ch)
	    break;

	if ( ch == ',' )
	{
	    sf->ptr++;
	    continue;
	}

	long n1, n2;
	err = ScanUValueSI(si,&n1,0);
	if (err)
	    break;

	if ( NextCharSI(si,false) == ':' )
	{
	    sf->ptr++;
	    err = ScanUValueSI(si,&n2,0);
	    if (err)
		break;
	}
	else
	    n2 = n1;

	const bool force = NextCharSI(si,false) == '!';
	if (force)
	     sf->ptr++;

	if ( n1 < 0 )
	    n1 = 0;
	if ( ++n2 > ctcode->max_tracks )
	    n2 = ctcode->max_tracks;
	PRINT0("SLOT: %02lx:%02lx = %lu:%lu%s%s\n",n1,n2,n1,n2,
			mark_reserved ? " /X" : "",
			force ? " /FORCE" : "");

	if ( n1 >= n2 )
	    continue;

	if (mark_reserved)
	{
	    if (force)
		memset(ctcode->used_slot+n1,2,n2-n1);
	    else
		for ( ; n1 < n2; n1++ )
		    if (!ctcode->used_slot[n1])
			ctcode->used_slot[n1] = 2;
	}
	else
	    for ( ; idx > 0 && n1 < n2; n1++ )
	    {
		idx--;
		ctcode->next_slot[idx]  = n1;
		ctcode->force_slot[idx] = force;
	    }
    }

    if (!mark_reserved)
    {
	ctcode->n_next_slot = ctcode->max_tracks - idx;
	if (ctcode->n_next_slot)
	{
	    memmove( ctcode->next_slot, ctcode->next_slot + idx,
			ctcode->n_next_slot * sizeof(*ctcode->next_slot) );
	    memmove( ctcode->force_slot, ctcode->force_slot + idx,
			ctcode->n_next_slot * sizeof(*ctcode->force_slot) );
	}
    }

    sf->disable_comma--;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanRTL_Track
	( ctcode_t *ctcode, ScanInfo_t * si, bool is_arena, bool hidden )
{
    DASSERT(ctcode);
    DASSERT(si);
    //ScanFile_t *sf = si->cur_file;
    //DASSERT(sf);

    char buf[100];


    //--- scan music id

    ulong music_id;
    enumError err = ScanUValueSI(si,(long*)&music_id,0);
    if (err)
	return err;

    if ( NextCharSI(si,false) == '!' )
	si->cur_file->ptr++;
    else
    {
	if ( music_id < MKW_N_TRACKS )
	    music_id = track_info[music_id].music_id;
	else if ( music_id < MKW_N_TRACKS + MKW_N_ARENAS )
	    music_id = arena_info[music_id-MKW_N_TRACKS].music_id;

	if ( MusicID2TrackId(music_id,music_id,-1) == -1 )
	    music_id = track_info[0].music_id;
    }
    SkipCharSI(si,';');


    //--- scan property id

    ulong property_id;
    err = ScanUValueSI(si,(long*)&property_id,0);
    if (err)
	return err;

    if ( NextCharSI(si,false) == '!' )
	si->cur_file->ptr++;
    else if (!is_arena)
	property_id &= 0x1f;
    SkipCharSI(si,';');


    //--- scan le-flags

    le_flags_t le_flags = 0;
    if (ctcode->use_le_flags)
    {
	long flags;
	err = ScanUValueSI(si,&flags,0);
	if (err)
	    return err;
	le_flags = flags;
	SkipCharSI(si,';');
    }


    //--- scan track_file, track_string and track_ident => create fname

    char *track_file = 0, *track_string = 0, *track_ident = 0;
    char fname[100];

    err = ConcatStringsSI(si,&track_file,0);
    if (err)
    {
      abort:
	FreeString(track_file);
	FreeString(track_string);
	FreeString(track_ident);
	return err;
    }


    //--- setup 'fname' and remove unwanted characters

    *fname = 0;
    if (track_file)
    {

	char *src = track_file, *dest = fname;
	while ( *src && dest < fname+sizeof(fname) )
	{
	    int ch = *src++;
	    if ( isalnum(ch) || strchr("+-._",ch) )
		*dest++ = ch;
	}
	*dest = 0;
    }

    if ( NextCharSI(si,false) == ';' )
    {
	SkipCharSI(si,';');
	err = ConcatStringsSI(si,&track_string,0);
	if (err)
	    goto abort;

	if ( NextCharSI(si,false) == ';' )
	{
	    SkipCharSI(si,';');
	    err = ConcatStringsSI(si,&track_ident,0);
	    if (err)
		goto abort;
	}
    }

    CheckEolSI(si);


    //--- insert new track

    ctcode_crs1_data_t *td = is_arena
				? GetNextArenaSlot(ctcode,property_id,si)
				: GetNextTrackSlot(ctcode,hidden);
    if (!td)
	return ERR_WARNING;

    const int tidx = td - ctcode->crs->data;
    if (ctcode->replace_at)
    {
	PRINT(">>> REPLACE:  %s\n",track_string);
	snprintf(buf,sizeof(buf),"%03X",tidx);
	track_string = ReplaceName(track_string,"@SLOT@",buf);
	track_string = ReplaceName(track_string,"@@","@");
	PRINT(">>> REPLACED: %s\n",track_string);
    }

    SetTrackFile(ctcode,tidx,track_file);
    SetTrackString(ctcode,tidx,track_string);
    SetTrackIdentifier(ctcode,tidx,track_ident);

    td->music_id    = htonl(music_id);
    td->property_id = htonl(property_id);

    ctcode->property[tidx] = property_id;
    ctcode->music[tidx]	   = music_id;
    ctcode->le_flags[tidx] = le_flags;
    ctcode->hidden[tidx]   = hidden != 0;

    if (!*fname)
    {
	if (ctcode->use_lecode)
	    snprintf(fname,sizeof(fname),"%03x",tidx);
	else
	    snprintf(fname,sizeof(fname),"slot_%02x",tidx);
    }
    StringCopyS(td->filename,sizeof(td->filename),fname);

    FreeString(track_file);
    FreeString(track_string);
    FreeString(track_ident);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanRacingTrackList ( ctcode_t *ctcode, ScanInfo_t * si )
{
    DASSERT(ctcode);
    DASSERT(si);

    ctcode->use_le_flags = 0;

    enumError max_err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);

	enumError err = ERR_OK;
	switch (ch)
	{
	    case '%':
		err = ScanRTL_Option(ctcode,si);
		break;

	    case 'n':
	    case 'N':
		sf->ptr++;
		err = ScanRTL_Nintendo(ctcode,si);
		break;

	    case 'c':
	    case 'C':
		ctcode->track_of_cup = 0;
		sf->ptr++;
		err = ConcatStringsSI(si,&ctcode->next_cup_name,0);
		break;

	    case 's':
	    case 'S':
		sf->ptr++;
		err = ScanRTL_Slot(ctcode,si,false);
		break;

	    case 'x':
	    case 'X':
		sf->ptr++;
		err = ScanRTL_Slot(ctcode,si,true);
		break;

	    case 't':
	    case 'T':
		sf->ptr++;
		err = ScanRTL_Track(ctcode,si,false,false);
		break;

	    case 'h':
	    case 'H':
		sf->ptr++;
		err = ScanRTL_Track(ctcode,si,false,true);
		break;

	    case 'a':
	    case 'A':
		sf->ptr++;
		err = ScanRTL_Track(ctcode,si,true,false);
		break;

	    default:
		{
		    ccp eol = FindNextLineFeedSI(si,true);
		    err = ERROR0(ERR_WARNING,
			"Line ignored [%s @%u]: %.*s\n",
			sf->name, sf->line, (int)(eol - sf->ptr), sf->ptr );
		}
		break;
	}
	CheckEolSI(si);
	if ( max_err < err )
	     max_err = err;
    }

    CheckLEFlagsUsage(ctcode);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanSetup ( ctcode_t *ctcode, ScanInfo_t * si );
static enumError ScanRacingTrackList ( ctcode_t *ctcode, ScanInfo_t * si );

//-----------------------------------------------------------------------------

enumError ScanTextCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(ctcode);

    const uint CTCODE_VERSION = 1;

    if (ct_mode)
	InitializeCTCODE(ctcode,ct_mode);
    ResetDataCTCODE(ctcode,true);


    //--- setup data

    ctcode->n_next_slot = 0;
    ctcode->track_of_cup = 0;
    //ctcode->n_tracks = MKW_ARENA_END;
    FreeString(ctcode->next_cup_name);
    ctcode->next_cup_name = 0;
    //HEXDUMP16(6,0,ctcode->used_slot,sizeof(ctcode->used_slot));


    //--- setup parser

    enum { SECT_SETUP, SECT_RACING_TRACK_LIST, N_SECT };
    const KeywordTab_t sect_names[] =
    {
	    { SECT_SETUP,		"SETUP",		0, 0 },
	    { SECT_RACING_TRACK_LIST,	"RACING-TRACK-LIST",	0, 0 },
	    { 0,0,0,0 }
    };

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,ctcode->fname,CTCODE_VERSION);
    si.predef = SetupVarsCTCODE();


    //--- main loop

    enumError max_err = ERR_OK;

    for(;;)
    {
	char ch = NextCharSI(&si,true);
	if (!ch)
	    break;

	if ( ch != '[' )
	{
	    NextLineSI(&si,true,false);
	    continue;
	}
	ResetLocalVarsSI(&si,CTCODE_VERSION);

	si.cur_file->ptr++;
	char name[20];
	ScanNameSI(&si,name,sizeof(name),true,true,0);

	int abbrev_count;
	const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,sect_names);
	if ( !cmd || abbrev_count )
	    continue;
	NextLineSI(&si,false,false);
	PRINT("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	if ( cmd->id < 0 || cmd->id > N_SECT )
	    continue;

	enumError err = ERR_OK;
	switch (cmd->id)
	{
	    case SECT_SETUP:
		PRINT("SCAN [SETUP]\n");
		err = ScanSetup(ctcode,&si);
		break;

	    case SECT_RACING_TRACK_LIST:
		PRINT("SCAN [RACING-TRACK-LIST]\n");
		err = ScanRacingTrackList(ctcode,&si);
		break;

	    default:
		// ignore all other section without any warnings
		break;
	}

	if ( max_err < err )
	     max_err = err;
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextCTCODE) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

// [[file-name]]
    UsePatchingListBMG(&ctcode->track_string);
    CalcCupRefCTCODE(ctcode,true);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawCTCODE()			///////////////
///////////////////////////////////////////////////////////////////////////////

static int scan_raw_ctcode_func
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    ctcode_t *ctcode = it->param;
    DASSERT(ctcode);

    noPRINT("--> %3d,%3d term=%u size=%4u : %s\n",
	it->group, it->entry, term, it->size, it->path );
    if ( term || it->size <= 0 )
	return 0;

    const u8 *data = it->szs->data + it->off;
    if (!memcmp(data,"CUP1",4))
    {
	const ctcode_cup1_head_t *hd = (ctcode_cup1_head_t*)data;
	uint size = be32(&hd->size);
	if ( size > it->size )
	    size = it->size;

	const uint nc1  = be32(&hd->n_racing_cups);
	const uint nc2  = be32(&hd->n_battle_cups);

	if ( it->off + size <= it->szs->size )
	{
	 #if USE_NEW_CONTAINER_CTC
	    PRINT("CTCODE/CUP1 found, SIZEx=%x/%x/%x, Nx=%x+%x, Nd=%d+%d, IN-C=%d\n",
		size, be32(&hd->size), it->size,
		nc1, nc2, nc1, nc2, InContainerP(&ctcode->container,data) );
	 #else
	    PRINT("CTCODE/CUP1 found, SIZEx=%x/%x/%x, Nx=%x+%x, Nd=%d+%d, IN-C=%d\n",
		size, be32(&hd->size), it->size,
		nc1, nc2, nc1, nc2, InDataContainer(ctcode->old_container,data) );
	 #endif

	    if (ctcode->cup_alloced)
		FREE(ctcode->cup);

	 #if USE_NEW_CONTAINER_CTC
	    if (InContainerP(&ctcode->container,data))
	 #else
	    if (InDataContainer(ctcode->old_container,data))
	 #endif
	    {
		ctcode->cup_alloced = false;
		ctcode->cup = (ctcode_cup1_head_t*)data;
	    }
	    else
	    {
		ctcode->cup_alloced = true;
		ctcode->cup = MEMDUP(data,size);
	    }
	    ctcode->cup_size = size;

	    ctcode->cup_racing = ctcode->cup->data;
	    ctcode->cup_battle = ctcode->cup_racing + nc1;

	    ctcode->n_racing_cups   = nc1;
	    ctcode->max_racing_cups = nc1;
	    ctcode->n_battle_cups   = nc2;

	    const uint max_cups	= ( data+size - (u8*)ctcode->cup_racing )
				/ sizeof(ctcode_cup1_data_t);

	    ctcode->max_battle_cups = max_cups - nc1;
	    if ( ctcode->max_battle_cups > 2 )
		ctcode->max_battle_cups = 2;
	    if ( ctcode->max_battle_cups < nc2 )
		ctcode->max_battle_cups = nc2;

	    if ( max_cups > nc1 + ctcode->max_battle_cups )
		ctcode->n_unused_cups = max_cups - nc1 - ctcode->max_battle_cups;

	    PRINT("STAT/CUP: racing: %u/%u, battle: %u/%u, unused: %u\n",
			ctcode->n_racing_cups, ctcode->max_racing_cups,
			ctcode->n_battle_cups, ctcode->max_battle_cups,
			ctcode->n_unused_cups );
	}
    }
    else if (!memcmp(data,"CRS1",4))
    {
	const ctcode_crs1_head_t *hd = (ctcode_crs1_head_t*)data;
	uint size = be32(&hd->size);
	if ( size > it->size )
	    size = it->size;

	const uint nt   = be32(&hd->n_tracks);

	if ( it->off + size <= it->szs->size )
	{
	 #if USE_NEW_CONTAINER_CTC
	    PRINT("CTCODE/CRS1 found, SIZEx=%x/%x/%x, Nxd=%x,%d, IN-C=%d\n",
		size, be32(&hd->size), it->size,
		nt, nt, InContainerP(&ctcode->container,data) );
	 #else
	    PRINT("CTCODE/CRS1 found, SIZEx=%x/%x/%x, Nxd=%x,%d, IN-C=%d\n",
		size, be32(&hd->size), it->size,
		nt, nt, InDataContainer(ctcode->old_container,data) );
	 #endif

	    if (ctcode->crs_alloced)
		FREE(ctcode->crs);

	 #if USE_NEW_CONTAINER_CTC
	    if (InContainerP(&ctcode->container,data))
	 #else
	    if (InDataContainer(ctcode->old_container,data))
	 #endif
	    {
		ctcode->crs_alloced = false;
		ctcode->crs = (ctcode_crs1_head_t*)data;
	    }
	    else
	    {
		ctcode->crs_alloced = true;
		ctcode->crs = MEMDUP(data,size);
	    }
	    ctcode->crs_size = size;

	    ctcode->n_tracks = nt;
	    ctcode->max_tracks = ( data + size - (u8*)ctcode->crs->data )
			       / sizeof(ctcode_crs1_data_t);

	    PRINT("STAT/TRACK: %u/%u\n", ctcode->n_tracks, ctcode->max_tracks );
	}
    }
    MARK_USED(ctcode);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int IterateFilesCTCODE
(
    szs_iterator_t	*it,		// iterator struct with all infos
    bool		multi_img	// false: mipmaps, true: multi images
)
{
    DASSERT(it);
    DASSERT(it->func_it);
    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 * data = szs->data;
    DASSERT(data);

    if ( !data || szs->size < sizeof(ctcode_header_t) )
	return -1;

    const ctcode_header_t *ch = (ctcode_header_t*)data;
    const u32 n_sect = be32(&ch->n_sect);
    const u32 size = be32(&ch->size);
    const u32 min_data_off = sizeof(ctcode_header_t)
			   + n_sect * sizeof(ctcode_sect_info_t)
			   - sizeof(ch->sect_info);
    if	(  n_sect < 2 || szs->size < min_data_off || szs->size < size )
	return -1;

    it->no_recurse++;

    it->index	= 0;
    it->off	= 0;
    it->size	= min_data_off;
    it->is_dir	= 0;
    StringCopyS(it->path,sizeof(it->path),".CTCODE.header");
    int stat = it->func_it(it,false);

    uint sect;
    for ( sect = 0; sect < n_sect && !stat; sect++ )
    {
	u32 off = be32(&ch->sect_info[sect].off);
	if ( off < min_data_off || off >= szs->size )
	    continue;

	u32 max_off = szs->size;
	if ( sect+1 < n_sect )
	{
	    const u32 next_off = be32(&ch->sect_info[sect+1].off);
	    noPRINT("NEXT-OFF: %x\n",next_off);
	    if ( next_off > off && max_off > next_off )
		max_off = next_off;
	}

	it->index++;
	it->off	= off;
	it->size = max_off - off;
	ccp name = ch->sect_info[sect].name;
	if ( off + 8 < szs->size )
	{
	    u32 *ptr = (u32*)(data + off);
	    const u32 new_size = be32(ptr+1);
	    if ( it->size > new_size )
		it->size = new_size;
	    name = (ccp)ptr;
	}
	snprintf(it->path,sizeof(it->path),
			"ctcode-%i-%s.bin", sect, PrintID(name,4,0));
	noPRINT("MAX-OFF: %x..%x [%x] %x/%zx / sect %u/%u : %s\n",
			off, off+it->size, it->size,
			max_off, szs->size, sect, n_sect, it->path );
	stat = it->func_it(it,false);
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ScanSetupCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata		// NULL or container-data
)
{
    DASSERT(ctcode);

 #if USE_NEW_CONTAINER_CTC // ---------------------------------------

    if (ct_mode)
	InitializeCTCODE(ctcode,ct_mode);
    else
	ResetDataCTCODE(ctcode,false);

    CatchContainerData(&ctcode->container,0,cdata);
    AssignContainer(&ctcode->container,0,data,data_size,CPM_COPY);
  #ifdef TEST
    DumpInfoContainer(stdlog,collog,2,"CTC: ",&ctcode->container,0);
  #endif

 #else // !USE_NEW_CONTAINER_CTC // ---------------------------------

    if (ct_mode)
	InitializeCTCODE(ctcode,ct_mode);
    else
	ResetDataCTCODE(ctcode,false);
    ctcode->old_container = cdata;

 #endif // !USE_NEW_CONTAINER_CTC // --------------------------------
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanRawCTCODE_helper
(
    ctcode_t		* ctcode,	// initialized CTCODE data structure
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(ctcode);
    DASSERT( IsValidCTCODE(data,data_size,data_size,0) < VALID_ERROR );

    memcpy(&ctcode->ct_head,data,sizeof(ctcode->ct_head));

    //--- scan CTCODE header & sections

    cut_iter_func[FF_CT1_DATA] = IterateFilesCTCODE; // enable CTCODE support
// [[fname-]]
    IterateFilesData( data, data_size, scan_raw_ctcode_func, ctcode, FF_CT1_DATA, 0 );
    CalcCupRefCTCODE(ctcode,true);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata		// NULL or container-data
)
{
    DASSERT(ctcode);
    ScanSetupCTCODE(ctcode,ct_mode,data,data_size,cdata);

    if ( IsValidCTCODE(data,data_size,data_size,ctcode->fname) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid CT-CODE file: %s\n", ctcode->fname ? ctcode->fname : "?");
    }

    return ScanRawCTCODE_helper(ctcode,data,data_size);
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanTexCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata		// NULL or container-data
)
{
    DASSERT(ctcode);
    ScanSetupCTCODE(ctcode,ct_mode,data,data_size,cdata);

    if ( IsValidTEXCT(data,data_size,data_size,0,ctcode->fname) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid TEX+CT file: %s\n", ctcode->fname ? ctcode->fname : "?");
    }

    const brsub_header_t *bh = (brsub_header_t*)data;
    const u32 img_off = be32(bh->grp_offset);
    return ScanRawCTCODE_helper( ctcode,
				data + CT_CODE_TEX_OFFSET,
				img_off - CT_CODE_TEX_OFFSET );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int iter_brres_ctcode
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    if (term)
	return 0;

    szs_file_t * szs = it->szs;
    DASSERT(szs);
    const u8 * data = szs->data;
    DASSERT(data);
    data += it->off;

    ctcode_t *ctcode = it->param;
    DASSERT(ctcode);

    noPRINT("%6x %6x : %02x %02x %02x %02x : %s\n",
		it->off, it->size,
		data[0],  data[1],  data[2],  data[3], it->name );

    if ( IsValidTEXCT(data,it->size,it->size,0,ctcode->fname) <  VALID_ERROR )
    {
	const brsub_header_t *bh = (brsub_header_t*)data;
	const u32 img_off = be32(bh->grp_offset);
	ScanRawCTCODE_helper( ctcode,
				data + CT_CODE_TEX_OFFSET,
				img_off - CT_CODE_TEX_OFFSET );
	return 1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanBrresCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata		// NULL or container-data
)
{
    DASSERT(ctcode);
    ScanSetupCTCODE(ctcode,ct_mode,data,data_size,cdata);

    szs_file_t szs;
    AssignSZS(&szs,true,(void*)data,data_size,false,FF_BRRES,0);
    IterateFilesParSZS(&szs,iter_brres_ctcode,ctcode,false,false,0,-1,SORT_NONE);
    ResetSZS(&szs);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanLEBinCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanLEBinCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(ctcode);

    if (ct_mode)
    {
	ct_mode = NormalizeCtModeBMG(ct_mode,opt_ct_mode);
	InitializeCTCODE( ctcode, ct_mode < CTM_CTCODE ? CTM_CTCODE : ct_mode );
    }
    ResetDataCTCODE(ctcode,true);

    le_analyse_t ana;
    AnalyseLEBinary(&ana,data,data_size);


    //--- copy racing cups

    ctcode->n_racing_cups = 0;
    ctcode_cup1_data_t *cup = ctcode->cup_racing;
    const le_cup_track_t *cup_dest = ana.cup_track;
    if ( cup && cup_dest )
    {
	uint cup_idx, max = ctcode->max_racing_cups;
	if ( max > ana.n_cup_track )
	     max = ana.n_cup_track ;

	for ( cup_idx = 0; cup_idx < max; cup_idx++, cup++ )
	{
	    if ( cup_dest[0] || cup_dest[1] || cup_dest[2] || cup_dest[3] )
	    {
		cup->track_id[0] = *cup_dest++;
		cup->track_id[1] = *cup_dest++;
		cup->track_id[2] = *cup_dest++;
		cup->track_id[3] = *cup_dest++;
		ctcode->n_racing_cups++;
	    }
	    else
		cup_dest += 4;
	}
    }


    //--- copy battle cups

    ctcode->n_battle_cups = 0;
    cup = ctcode->cup_battle;
    cup_dest = ana.cup_arena;
    if ( cup && cup_dest )
    {
	uint cup_idx, max = ctcode->max_battle_cups;
	if ( max > ana.n_cup_arena )
	     max = ana.n_cup_arena ;

	for ( cup_idx = 0; cup_idx < max; cup_idx++, cup++ )
	{
	    if ( cup_dest[0] || cup_dest[1] || cup_dest[2] || cup_dest[3] || cup_dest[4] )
	    {
		cup->track_id[0] = *cup_dest++;
		cup->track_id[1] = *cup_dest++;
		cup->track_id[2] = *cup_dest++;
		cup->track_id[3] = *cup_dest++;
		cup->track_id[4] = *cup_dest++;
		ctcode->n_battle_cups++;
	    }
	    else
		cup_dest += 4;
	}
    }


    //--- copy track properties

    ctcode->n_tracks		= 0;
    ctcode_crs1_data_t	*td	= ctcode->crs->data;
    const le_property_t	*prop	= ana.property;
    const le_music_t	*music	= ana.music;
    const le_flags_t	*flags	= ana.flags;
    le_flags_t	     *ct_flags	= ctcode->le_flags;

    if ( td && prop && music && flags )
    {
	uint tidx, max = ctcode->max_tracks;
	if ( max > ana.n_slot )
	     max = ana.n_slot ;

	int max_tidx = -1;
	for ( tidx = 0; tidx < max;
		tidx++, td++, prop++, music++, flags++, ct_flags++ )
	{
	    if ( tidx >= 0x2a && tidx < LE_FIRST_LOWER_CT_SLOT )
	    {
		td->music_id	= htonl(0x75);
		td->property_id	= htonl(0);
		*ct_flags	= 0;
	    }
	    else
	    {
		if ( *music || *prop )
		    max_tidx = tidx;
		td->music_id	= htonl(*music);
		td->property_id	= htonl(*prop);
		*ct_flags	= *flags;
		snprintf(td->filename,sizeof(td->filename),"%03x",tidx);
	    }
	}
	ctcode->n_tracks = max_tidx+1;
    }


    //--- copy lecode parameter

    if (!ctcode->lpar)
	ctcode->lpar = MALLOC(sizeof(*ctcode->lpar));
    *ctcode->lpar = ana.lpar;


    //--- terminate

    ResetLEAnalyse(&ana);
    CalcCupRefCTCODE(ctcode,true);
    CheckLEFlagsUsage(ctcode);

    PRINT("STAT/CUP: racing: %u/%u, battle: %u/%u, unused: %u\n",
		ctcode->n_racing_cups, ctcode->max_racing_cups,
		ctcode->n_battle_cups, ctcode->max_battle_cups,
		ctcode->n_unused_cups );
    PRINT("STAT/TRACK: %u/%u\n", ctcode->n_tracks, ctcode->max_tracks );

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CTCODE scan + load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerCTC_t	* cdata,	// NULL or container-data
    CheckMode_t		mode		// not NULL: call CheckCTCODE(mode)
)
{
    DASSERT(ctcode);
    PRINT("ScanCTCODE(mode=%x) size=%u\n",mode,data_size);

    enumError err;
// [[analyse-magic]]
    switch (GetByMagicFF(data,data_size,data_size))
    {
	case FF_BRRES:
	    err = ScanBrresCTCODE(ctcode,ct_mode,data,data_size,cdata);
	    if ( !ctcode->cup && !ctcode->crs )
	    {
		ERROR0(ERR_INVALID_DATA,
		    "No CTCODE in BRRES found: %s\n", ctcode->fname ? ctcode->fname : "?");
		ResetCTCODE(ctcode);
		return ERR_INVALID_DATA;
	    }
	    ctcode->fform = FF_BRRES;
	    break;

	case FF_TEX_CT:
	    err = ScanTexCTCODE(ctcode,ct_mode,data,data_size,cdata);
	    ctcode->fform = FF_TEX_CT;
	    break;

	case FF_CT1_DATA:
	    err = ScanRawCTCODE(ctcode,ct_mode,data,data_size,cdata);
	    ctcode->fform = FF_CT1_DATA;
	    break;

	case FF_CT_TEXT:
	 #if USE_NEW_CONTAINER_CTC
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    err =  ScanTextCTCODE(ctcode,ct_mode,data,data_size);
	    ctcode->fform = FF_CT_TEXT;
	    break;

	case FF_LE_BIN:
	 #if USE_NEW_CONTAINER_CTC
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    err =  ScanLEBinCTCODE(ctcode,ct_mode,data,data_size);
	    ctcode->fform = FF_LE_BIN;
	    break;

	default:
	 #if USE_NEW_CONTAINER_CTC
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    if (ct_mode)
		InitializeCTCODE(ctcode,ct_mode);
	    return ERROR0(ERR_INVALID_DATA,
		"No CTCODE file: %s\n", ctcode->fname ? ctcode->fname : "?");
    }

    if ( !err && opt_patch_names )
	PatchNamesCTCODE(ctcode);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataCTCODE
(
    ctcode_t		* ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckCTCODE(mode)
)
{
    DASSERT(ctcode);
    DASSERT(raw);
    if (ct_mode)
	InitializeCTCODE(ctcode,ct_mode);
    else
	ResetCTCODE(ctcode);
    if (!raw->data)
	return ERR_OK;

    ctcode->fatt  = raw->fatt;
    ctcode->fname = raw->fname;
    raw->fname = 0;
 #if USE_NEW_CONTAINER_CTC
    return ScanCTCODE( ctcode, CTM_NO_INIT, raw->data, raw->data_size,
			LinkContainerRawData(raw), mode );
 #else
    return ScanCTCODE( ctcode, CTM_NO_INIT, raw->data, raw->data_size,
			ContainerRawData(raw), mode );
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadCTCODE
(
    ctcode_t		*ctcode,	// CTCODE data structure
    ct_mode_t		ct_mode,	// not CTM_NO_INIT: Initialize
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckCTCODE(mode)
)
{
    DASSERT(ctcode);
    DASSERT(fname);
    if (ct_mode)
	InitializeCTCODE(ctcode,ct_mode);
    else
	ResetCTCODE(ctcode);

    if (!strcmp(fname,"0"))
    {
	ctcode->fform = FF_CT_TEXT;
	ctcode->fname = STRDUP(fname);
	return ERR_OK;
    }

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	ctcode->fname = raw.fname;
	raw.fname = 0;
     #if USE_NEW_CONTAINER_CTC
	err = ScanCTCODE( ctcode, CTM_NO_INIT, raw.data, raw.data_size,
				LinkContainerRawData(&raw),mode);
     #else
	err = ScanCTCODE( ctcode, CTM_NO_INIT, raw.data, raw.data_size,
				ContainerRawData(&raw), mode );
     #endif
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTextCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_track
(
    FILE		*f,		// output file
    ctcode_t		*ctcode,	// pointer to valid CTCODE
    char		letter,		// record letter
    uint		tidx,		// valid track index
    bool		print_hex	// true: force hex output
)
{
    DASSERT( f );
    DASSERT( ctcode );
    DASSERT( letter == 'T' || letter == 'H' || letter == 'A' );
    DASSERT( tidx < ctcode->n_tracks );

    char namebuf[TNAME_BUF_SIZE];

    const bmg_item_t *bi
	    = FindItemBMG(&opt_load_bmg,tidx+ctcode->ctb.track_name1.beg);
    GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,bi);
    const ctcode_crs1_data_t *td = ctcode->crs->data + tidx;

    fprintf(f,"%c %s\t; %s\t;",
	letter,
	PrintMusicID(ntohl(td->music_id),print_hex),
	PrintPropertyID(ntohl(td->property_id),print_hex) );

    if (ctcode->use_le_flags)
	fprintf(f," 0x%02x\t;",ctcode->le_flags[tidx]);

    GetTrackFile(namebuf,sizeof(namebuf),ctcode,tidx,0);
    fprintf(f," \"%s\"",namebuf);

    ccp placeholder;
    GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,bi);
    if (*namebuf)
    {
	fprintf(f," ; \"%s\"",namebuf);
	placeholder = "";
    }
    else
	placeholder = " ; \"\"";

    GetTrackIdentifier(namebuf,sizeof(namebuf),ctcode,tidx,0);
    if (*namebuf)
	fprintf(f,"%s ; \"%s\"\r\n",placeholder,namebuf);
    else
	fputs("\r\n",f);
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveTextCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    const output_mode_t	* outmode	// output mode, if NULL: use defaults
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(ctcode);
    PrepareSaveCTCODE(ctcode);

    DASSERT(fname);
    PRINT("SaveTextCTCODE(%s,%d)\n",fname,set_time);

    output_mode_t omode;
    if (outmode)
	memcpy(&omode,outmode,sizeof(omode));
    else
	InitializeOutputMode(&omode);
    CalcCupRefCTCODE(ctcode,false);

    char namebuf[TNAME_BUF_SIZE], keybuf[20];

    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,ctcode->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&ctcode->fatt,0);


    //--- print file header + syntax info

    if (omode.syntax)
    {
	fprintf( F.f, text_ctcode_head_cr );
	fputs(section_sep,F.f);
    }
    else
	fprintf(F.f,"%s\r\n\r\n",CT_TEXT_MAGIC8);

    MARK_USED(keybuf,namebuf);
    fprintf( F.f, text_ctcode_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		 ctcode->ctb.ct_mode,
		ctcode->n_racing_cups,
		ctcode->max_racing_cups,
		ctcode->n_battle_cups,
		ctcode->max_battle_cups,
		 ctcode->used_tracks,
		 ctcode->used_arenas,
		 ctcode->n_tracks,
		 ctcode->max_tracks,
		ctcode->n_racing_cups,
		ctcode->max_racing_cups,
		ctcode->n_battle_cups,
		ctcode->max_battle_cups,
		ctcode->n_unused_cups );


    //--- print track list

    if ( omode.mode & 1 && ctcode->used_tracks )
    {
	fprintf(F.f,"%s[RACING-TRACK-LIST]\r\n%s",
			section_sep,
			omode.syntax
				? text_ctcode_list_syntax_full_cr
				: text_ctcode_list_syntax_brief_cr );

	if (ctcode->use_le_flags)
	    fputs("\n%LE-FLAGS = 1 # 0:disabled, 1:enabled\n",F.f);

	if (ctcode->replace_at)
	    fputs("\n%REPLACE-AT = 1"
		  " # replace @*@ codes in track names (1:enabled)\n",F.f);


	//-- print battle cups

	if (ctcode->assigned_arenas)
	{
	    fprintf(F.f,"\n#--- battle arenas\n\nC \"Wii Arenas\"\n");

	    uint arena;
	    for ( arena = 0; arena < MKW_N_ARENAS; arena++ )
	    {
		if ( arena == 5 )
		    fprintf(F.f,"\nC \"Retro Arenas\"\n");

		const uint tidx = arena_pos_default[arena] + MKW_N_TRACKS;
		print_track(F.f,ctcode,'A',tidx,omode.hex);
	    }
	    fprintf(F.f,"\n#--- racing tracks\n");
	}


	//-- racing cups

	u8 done[CODE_MAX_TRACKS];
	memset(done,0,sizeof(done));
	memset(done+0x20,1,10);
	memset(done+0x36,1, 5);
	memset(done+0x42,1, 2);

	uint cidx;
	for ( cidx = 0; cidx < ctcode->n_racing_cups; cidx++ )
	{
	    const ctcode_cup1_data_t *cd = ctcode->cup->data + cidx;
	    const bmg_item_t *bi
		= FindItemBMG(&opt_load_bmg,ctcode->ctb.rcup_name.beg+cidx);
	    if ( bi && bi->len )
		PrintString16BMG(namebuf,sizeof(namebuf),bi->text,bi->len,
				BMG_UTF8_MAX,2,opt_bmg_colors);
	    else
		PrintString16BMG(namebuf,sizeof(namebuf),cd->name,-1,
				BMG_UTF8_MAX,2,opt_bmg_colors);
	    fprintf(F.f,"\r\nC %s # 0x%02x = %2u\r\n",namebuf,cidx,cidx);

	    fprintf(F.f,"S 0x%02x!, 0x%02x!, 0x%02x!, 0x%02x!\r\n",
		be32(cd->track_id+0),
		be32(cd->track_id+1),
		be32(cd->track_id+2),
		be32(cd->track_id+3) );

	    uint ti;
	    for ( ti = 0; ti < 4; ti++ )
	    {
		const uint tidx = be32(cd->track_id+ti);
		if ( tidx < ctcode->n_tracks )
		{
		    print_track(F.f,ctcode,'T',tidx,omode.hex);
		    done[tidx] = 2;
		}
		else
		    fprintf(F.f,"T= -\r\n");
	    }
	}

	uint tidx, h_count = 0;
	DASSERT( ctcode->n_tracks <= ctcode->max_tracks );
	for ( tidx = 0; tidx < ctcode->n_tracks; tidx++ )
	{
	    if ( done[tidx] )
		continue;

	    const ctcode_crs1_data_t *td = ctcode->crs->data + tidx;
	    if ( ntohl(td->music_id) == 0x75 && !ntohl(td->property_id) )
		continue;

	    if (!h_count++)
		fprintf(F.f,"\r\n#--- hidden tracks (not linked to a cup)\r\n\r\n");

	    fprintf(F.f,"S 0x%02x!\r\n",tidx);
	    print_track(F.f,ctcode,'H',tidx,omode.hex);
	}
    }


    //--- print track reference

    if ( omode.mode & 2 && ctcode->n_tracks )
    {
	fputs(section_sep,F.f);

	const uint keyfw = 8;
	uint tidx;
	for ( tidx = 0; tidx < ctcode->n_tracks; tidx++ )
	{
	    const ctcode_crs1_data_t *td = ctcode->crs->data+tidx;
	    fprintf(F.f,"[TRACK-REF]\r\n%-*s = 0x%02x # %u\r\n",keyfw,"index",tidx,tidx);
	    const bool is_battle = IsMkwArena(tidx);
	    if (is_battle)
		fprintf(F.f,"%-*s = %2u\r\n",keyfw,"battle",tidx-MKW_ARENA_BEG);

	    GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,0);
	    fprintf(F.f,"%-*s = %s\r\n",keyfw,"name",namebuf);

	    const bmg_item_t *bi
		= FindItemBMG(&opt_load_bmg,tidx+ctcode->ctb.track_name1.beg);
	    if ( bi && bi->len )
	    {
		PrintString16BMG(namebuf,sizeof(namebuf),bi->text,bi->len,
				BMG_UTF8_MAX,0,opt_bmg_colors);
		fprintf(F.f,"%-*s = %s\r\n",keyfw,"bmg",namebuf);
	    }

	    fprintf(F.f,"%-*s = %s\r\n",keyfw,"file",td->filename);

	    GetTrackFile(namebuf,sizeof(namebuf),ctcode,tidx,0);
	    if (*namebuf)
		fprintf(F.f,"%-*s = %s\r\n",keyfw,"filename",namebuf);

	    const uint music_id = be32(&td->music_id);
	    fprintf(F.f,"%-*s = 0x%02x",keyfw,"music",music_id);
	    const uint mtrack = MusicID2TrackId(music_id,0xff,0xff);
	    if ( omode.cross_ref && mtrack < MKW_N_TRACKS + MKW_N_ARENAS )
	    {
		const TrackInfo_t *info = GetTrackInfo(mtrack);
		fprintf(F.f," # %s, %s, %s\r\n",
			info->sound_n_fname, info->sound_f_fname, info->name_en );
	    }
	    else
		fputs("\r\n",F.f);

	    const uint prop_id = be32(&td->property_id);
	    fprintf(F.f,"%-*s = 0x%02x",keyfw,"property",prop_id);
	    if ( omode.cross_ref && prop_id < ctcode->n_tracks )
	    {
		GetTrackString(namebuf,sizeof(namebuf),ctcode,prop_id,0);
		const ctcode_crs1_data_t *pd = ctcode->crs->data+prop_id;
		fprintf(F.f," # %s, %s\r\n", pd->filename, namebuf );
	    }
	    else
		fputs("\r\n",F.f);

	    const uint cup_id = be32(&td->cup_id);
	    fprintf(F.f,"%-*s = 0x%02x",keyfw,"cup",cup_id);
	    if ( omode.cross_ref && !is_battle && tidx < ctcode->max_tracks )
	    {
		ctcode_cupref_t *cr = ctcode->cupref + tidx;
		if (!cr->n)
		    fprintf(F.f," # not used in any cup!\r\n");
		else
		{
		    if ( cup_id == cr->cup[0] )
		    {
			const ctcode_cup1_data_t *cd = GetCupData(ctcode,cup_id);
			if (cd)
			{
			    const ctcode_cup1_data_t *cd = ctcode->cup->data+cup_id;
			    PrintString16BMG(namebuf,sizeof(namebuf),cd->name,-1,
						BMG_UTF8_MAX,0,opt_bmg_colors);
			    fprintf(F.f," # used in cup '%s' @%d\r\n",
				namebuf, cr->idx[0] );
			}
		    }
		    else
		    {
			const uint cup_id2 = cr->cup[0];
			const ctcode_cup1_data_t *cd = GetCupData(ctcode,cup_id2);
			if (cd)
			{
			    PrintString16BMG(namebuf,sizeof(namebuf),cd->name,-1,
						BMG_UTF8_MAX,0,opt_bmg_colors);
			    fprintf(F.f," # not used in cup 0x%02x,"
					" but in cup 0x%02x '%s' @%d\r\n",
					cup_id, cup_id2, namebuf, cr->idx[0] );
			}
		    }

		    uint i;
		    for ( i = 1; i < cr->n; i++ )
		    {
			const ctcode_cup1_data_t *cd = GetCupData(ctcode,cr->cup[i]);
			if (cd)
			{
			    PrintString16BMG(namebuf,sizeof(namebuf),cd->name,-1,
						BMG_UTF8_MAX,0,opt_bmg_colors);
			    fprintf(F.f,"%-*s = 0x%02x # Also used in cup '%s' @%d\r\n",
				    keyfw, "#cup", cr->cup[i], namebuf, cr->idx[i] );
			}
		    }
		}
	    }
	    else
		fputs("\r\n",F.f);

	    fputs("\r\n",F.f);
	}
    }


    //--- print cup reference

    if ( omode.mode & 2 && ctcode->n_racing_cups + ctcode->n_battle_cups )
    {
	fputs(section_sep,F.f);

	const uint keyfw = 7;
	uint cidx;
	for ( cidx = 0; cidx < ctcode->n_racing_cups + ctcode->n_battle_cups; cidx++ )
	{
	    const ctcode_cup1_data_t *cd = GetCupData(ctcode,cidx);
	    DASSERT(cd);

	    fprintf(F.f,"[CUP-REF]\r\n%-*s = 0x%02x # %u\r\n",keyfw,"index",cidx,cidx);
	    const bool is_battle = cidx >= ctcode->n_racing_cups;
	    if (is_battle)
		fprintf(F.f,"%-*s = %u\r\n",keyfw,"battle",cidx-ctcode->n_racing_cups);
	    PrintString16BMG(namebuf,sizeof(namebuf),cd->name,-1,
				BMG_UTF8_MAX,0,opt_bmg_colors);
	    fprintf(F.f,"%-*s = %s\r\n",keyfw,"name",namebuf);

	    const bmg_item_t *bi
		= FindItemBMG(&opt_load_bmg,cidx+ctcode->ctb.rcup_name.beg);
	    if ( bi && bi->len )
	    {
		PrintString16BMG(namebuf,sizeof(namebuf),bi->text,bi->len,
				BMG_UTF8_MAX,0,opt_bmg_colors);
		fprintf(F.f,"%-*s = %s\r\n",keyfw,"bmg",namebuf);
	    }

	    uint ti;
	    for ( ti = 0; ti < 4+is_battle; ti++ )
	    {
		snprintf(keybuf,sizeof(keybuf),"track-%u",ti);
		const uint tidx = be32(cd->track_id+ti);
		fprintf(F.f,"%-*s = 0x%02x",keyfw,keybuf,tidx);

		if ( omode.cross_ref && tidx < ctcode->n_tracks )
		{
		    const bmg_item_t *bi
			= FindItemBMG(&opt_load_bmg,tidx+ctcode->ctb.track_name1.beg);
		    GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,bi);
		    const ctcode_crs1_data_t *td = ctcode->crs->data + tidx;

		    fprintf(F.f," # M=%02x, P=%02x, %s, %s\r\n",
			be32(&td->music_id), be32(&td->property_id),
			td->filename, namebuf );
		}
		else
		    fputs("\r\n",F.f);
	    }
	    fputs("\r\n",F.f);
	}
    }


    //--- terminate

    fputs(section_end,F.f);
    ResetFile(&F,set_time);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveMessageCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveMessageCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(ctcode);
    DASSERT(fname);

    PRINT("SaveMessageCTCODE(%s,%d)\n",fname,set_time);
    PrepareSaveCTCODE(ctcode);

    const bool brief = brief_count > 0 || export_count > 0;
    if ( brief || long_count > 1 )
	CalcCupRefCTCODE(ctcode,false);

    bmg_t bmg;
    InitializeBMG(&bmg);


    //--- identification

    PatchIdentificationBMG(&bmg,&ctcode->ctb);


    //--- define cup names

    if ( long_count > 0 )
    {
	uint cidx, max = ctcode->n_racing_cups;
	if (!brief)
	    max += ctcode->n_battle_cups;
	for ( cidx = 0; cidx < max; cidx++ )
	{
	    const ctcode_cup1_data_t *cd = GetCupData(ctcode,cidx);
	    DASSERT(cd);
	    bmg_item_t *bi = InsertItemBMG(&bmg,ctcode->ctb.rcup_name.beg+cidx,0,0,0);
	    DASSERT(bi);
	    uint len = GetLength16BMG(cd->name,sizeof(cd->name)/sizeof(*cd->name));
	    AssignItemText16BMG(bi,cd->name,len);
	}
    }


    //--- define track names

    uint tidx, max = ctcode->n_tracks;
    if ( brief && max > ctcode->max_tracks )
	max = ctcode->max_tracks;

    uint begin	= brief_count > 2 || brief_count > 1 && ctcode->cmd_n_used
		? 32 : 0;

    const u16 *prev_cup = 0;
    uint prev_cup_len = 0;

    for ( tidx = begin; tidx < max; tidx++ )
    {
	if ( brief && !ctcode->cupref[tidx].n_racing )
	    continue;

	const uint mid = tidx + ctcode->ctb.track_name1.beg;
	const bmg_item_t *bi0 = FindItemBMG(&opt_load_bmg,mid);
	uint tlen;
	const u16 *text = GetTrackName16(&tlen,ctcode,tidx,bi0);
	bmg_item_t *bi = InsertItemBMG(&bmg,mid,0,0,0);
	DASSERT(bi);
	AssignItemText16BMG(bi,text,tlen);

	if ( long_count > 1 && tidx < ctcode->max_tracks )
	{
	    char buf[1000], *dest = buf;
	    ctcode_cupref_t *cr = ctcode->cupref + tidx;

	    if (cr->n)
	    {
		uint i;
		for ( i = 0; i < cr->n; i++ )
		{
		    dest += snprintf(dest,buf+sizeof(buf)-dest,
			    ",%u.%u", cr->cup[i]+1, cr->idx[i]+1 );
		}
		bmg_item_t *bi
			= InsertItemBMG(&bmg,ctcode->ctb.cup_ref.beg+tidx,0,0,0);
		DASSERT(bi);
		AssignItemTextBMG(bi,buf+1,dest-buf-1);
		prev_cup     = bi->text;
		prev_cup_len = bi->len;
	    }
	    else if ( prev_cup && tidx >= LE_FIRST_LOWER_CT_SLOT )
	    {
		bmg_item_t *bi
			= InsertItemBMG(&bmg,ctcode->ctb.cup_ref.beg+tidx,0,0,0);
		DASSERT(bi);
		AssignItemText16BMG(bi,prev_cup,prev_cup_len);
	    }
	}
    }


    //--- save messages

    long_count		= 1; // no special groups
    brief_count		= 1; // no header, but magic
    opt_bmg_single_line	= 2; // only pure messages in one line

    UsePatchingListBMG(&bmg);
    enumError err = SaveTextXBMG(&bmg,fname,set_time);
    ResetBMG(&bmg);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTrackListCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTrackListCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    uint		select,		// track selection
					//   0: only custom tracks
					//   1: standard + custom tracks
					//   2: standard + custom + battle tracks
    bool		print_slots,	// true: print music and property slots
    bool		print_head,	// true: print table header
    bool		raw_mode	// true: print raw text without escapes

)
{
    DASSERT(ctcode);
    DASSERT(fname);

    PrepareSaveCTCODE(ctcode);
    PRINT("SaveTrackListCTCODE(%s,%d)\n",fname,set_time);

    char namebuf[TNAME_BUF_SIZE];

    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,ctcode->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&ctcode->fatt,0);


    //--- print header

    if (print_head)
    {
	if (print_slots)
	    fprintf(F.f, "\n Slot\tProp\tMusic\tTrack name\n%.79s\n",Minus300);
	else
	    fprintf(F.f, "\n Slot\tTrack name\n%.63s\n",Minus300);
    }


    //--- print track list

    uint cidx, max = ctcode->n_racing_cups;
    if ( select > 1 )
	max += ctcode->n_battle_cups;
    for ( cidx = select ? 0 : 8; cidx < max; cidx++ )
    {
	const bool is_battle = cidx >= ctcode->n_racing_cups;
	const ctcode_cup1_data_t *cd = GetCupData(ctcode,cidx);
	DASSERT(cd);
	fputc('\n',F.f);

	uint ti;
	for ( ti = 0; ti < 4+is_battle; ti++ )
	{
	    const uint tidx = be32(cd->track_id+ti);
	    if ( tidx < ctcode->n_tracks )
	    {
		const ctcode_crs1_data_t *td = ctcode->crs->data + tidx;
		uint music_id = ntohl(td->music_id);
		uint property_id = ntohl(td->property_id);

		if (is_battle)
		{
		    fprintf(F.f," A%u.%u", cidx - ctcode->n_racing_cups + 1, ti+1 );

		    if (!music_id)
			music_id = arena_info[tidx-MKW_N_TRACKS].music_id;
		    if (!property_id)
			property_id = tidx;
		}
		else
		    fprintf(F.f,"%3u.%u", cidx+1, ti+1 );

		if (print_slots)
		{
		    fprintf(F.f,"\t%s\t%s",
			PrintPropertyID(property_id,false),
			PrintMusicID(music_id,false) );
		}

		const uint mid = tidx + ctcode->ctb.track_name1.beg;
		const bmg_item_t *bi = FindItemBMG(&opt_load_bmg,mid);
		if (raw_mode)
		    GetRawTrackInfo(namebuf,sizeof(namebuf),ctcode,tidx,bi);
		else
		    GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,bi);
		fprintf(F.f,"\t%s\n",namebuf);
	    }
	}
    }
    fputc('\n',F.f);


    //--- terminate

    ResetFile(&F,set_time);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTrackTableCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTrackTableCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(ctcode);
    DASSERT(fname);

    PrepareSaveCTCODE(ctcode);
    PRINT("SaveTrackTableCTCODE(%s,%d)\n",fname,set_time);

    char namebuf[TNAME_BUF_SIZE];


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,ctcode->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&ctcode->fatt,0);


    //--- print track list

    uint cidx;
    for ( cidx = 0; cidx < ctcode->n_racing_cups; cidx++ )
    {
	const ctcode_cup1_data_t *cd = ctcode->cup->data + cidx;

	uint ti;
	for ( ti = 0; ti < 4; ti++ )
	{
	    const uint tidx = be32(cd->track_id+ti);
	    if ( tidx < ctcode->n_tracks )
	    {
		const uint mid = tidx + ctcode->ctb.track_name1.beg;
		const bmg_item_t *bi = FindItemBMG(&opt_load_bmg,mid);
		GetTrackString(namebuf,sizeof(namebuf),ctcode,tidx,bi);
		fprintf(F.f,"%02u%u|%04x|%s\n",cidx,ti,mid,namebuf);
	    }
	}
    }


    //--- terminate

    ResetFile(&F,set_time);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    SaveCTCODE()		///////////////
///////////////////////////////////////////////////////////////////////////////

static uint DeactivateMOD2
(
    BZ2Source_t		*bz2s,		// data
    ctlang_save_t	lang_mode,	// language mode
    const u32		*search_pal,	// PAL list
    const u32		*search_usa,	// USA list
    const u32		*search_jap,	// JAP list
    const u32		*search_kor	// KOR list
)
{
    DASSERT(bz2s);

    uint patch_count = 0;
    u8 *data = NonConstDataBZ2S(bz2s);
    if (data)
    {
	const u32 *search_list;
	switch (lang_mode)
	{
	    case CTL_US: search_list = search_usa; break;
	    case CTL_JP: search_list = search_jap; break;
	    case CTL_KO: search_list = search_kor; break;
	    default:     search_list = search_pal; break;
	}

	if (search_list)
	{
	    u8 *end = data + bz2s->size - 0x10;
	    while ( *search_list )
	    {
		const u32 search = *search_list++;
		u8 *ptr = data + 0x20;
		for ( ; ptr < end; ptr += 0x10 )
		    if ( be32(ptr) == search && be32(ptr+8) == 4 && !be32(ptr+12) )
		    {
			ptr[11] = 0;
			patch_count++;
			break;
		    }
	    }
	}
    }
    return patch_count;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveCTCODE
(
    ctcode_t		* ctcode,	// pointer to valid CTCODE
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
    ctcode_save_t	save_mode,	// save mode
    ctlang_save_t	lang_mode,	// language mode
    const output_mode_t	* outmode	// output mode for text output
					//   if NULL: use defaults
)
{
    DASSERT(ctcode);
    PrepareSaveCTCODE(ctcode);

    char pathbuf[PATH_MAX];


    //--- define sections

    enum { BIDX_BAD0CODE, BIDX_BAD0DATA, BIDX_BAD1CODE,
		BIDX_MOD1, BIDX_MOD2, BIDX_OVR1, BIDX__N };

    #undef REF
    #define REF(p,n,a) p "/" n ";" n, a, sizeof(a)
    static BZ2Source_t srctab[] =
    {
	{ REF("rmcp","boot_code.bin", ctcode_boot_code_pal_bz2), FF_CT0_CODE },
	{ REF("rmcp","boot_data.bin", ctcode_boot_data_pal_bz2), FF_CT0_DATA },
	{ REF("rmcp","bad1code.bin",  ctcode_bad1code_pal_bz2),  FF_CT1_CODE },
	{ REF("rmcp","mod1.bin",      ctcode_mod1_pal_bz2),      FF_MOD1 },
	{ REF("rmcp","mod2.bin",      ctcode_mod2_pal_bz2),      FF_MOD2 },
	{ REF("rmcp","ovr1.bin",      ctcode_ovr1_pal_bz2),      FF_OVR1 },

	{ REF("rmce","boot_code.bin", ctcode_boot_code_usa_bz2), FF_CT0_CODE },
	{ REF("rmce","boot_data.bin", ctcode_boot_data_usa_bz2), FF_CT0_DATA },
	{ REF("rmce","bad1code.bin",  ctcode_bad1code_usa_bz2),  FF_CT1_CODE },
	{ REF("rmce","mod1.bin",      ctcode_mod1_usa_bz2),      FF_MOD1 },
	{ REF("rmce","mod2.bin",      ctcode_mod2_usa_bz2),      FF_MOD2 },
	{ REF("rmce","ovr1.bin",      ctcode_ovr1_usa_bz2),      FF_OVR1 },

	{ REF("rmcj","boot_code.bin", ctcode_boot_code_jap_bz2), FF_CT0_CODE },
	{ REF("rmcj","boot_data.bin", ctcode_boot_data_jap_bz2), FF_CT0_DATA },
	{ REF("rmcj","bad1code.bin",  ctcode_bad1code_jap_bz2),  FF_CT1_CODE },
	{ REF("rmcj","mod1.bin",      ctcode_mod1_jap_bz2),      FF_MOD1 },
	{ REF("rmcj","mod2.bin",      ctcode_mod2_jap_bz2),      FF_MOD2 },
	{ REF("rmcj","ovr1.bin",      ctcode_ovr1_jap_bz2),      FF_OVR1 },

	// [[2do]] [[ct-korea]]
	{ REF("rmck","boot_code.bin", 0), FF_CT0_CODE },
	{ REF("rmck","boot_data.bin", 0), FF_CT0_DATA },
	{ REF("rmck","bad1code.bin",  0), FF_CT1_CODE },
	{ REF("rmck","mod1.bin",      0), FF_MOD1 },
	{ REF("rmck","mod2.bin",      0), FF_MOD2 },
	{ REF("rmck","ovr1.bin",      0), FF_OVR1 },

	{0}
    };
    #undef REF

    ClearBZ2S(srctab,-1);


    //--- setup jobs

    bool copy_brres	= false;  // copy BRRES header and string pool
    bool copy_tex	= false;  // copy TEX0 header and IMAGE
    bool copy_ct0code	= false;  // copy CT0CODE = boot_code
    bool copy_ct0data	= false;  // copy CT0DATA = boot_data
    bool copy_ct1code	= false;  // copy CT1CODE
    bool copy_ct1data	= false;  // copy CT1DATA (header + CUP1 + CRS1 + MOD1 + MOD2 + OVR )
    bool copy_cup1	= false;  // copy CUP1 section
    bool copy_crs1	= false;  // copy CRS1 section
    bool copy_mod1	= false;  // copy MOD1 section
    bool copy_mod2	= false;  // copy MOD2 section
    bool copy_ovr1	= false;  // copy OVR1 section

    uint load_data	= 0;      // load data from external files, bit field: 1<<BIDX_*
    bool use_dynamic	= false;  // if true: enable dynamic CTGP_CODE size

    switch (save_mode)
    {
     case CTS_CUP1:
	copy_cup1	= true;
	break;

     case CTS_CRS1:
	copy_crs1	= true;
	break;

     case CTS_MOD1:
	copy_mod1	= true;
	load_data	= 1 << BIDX_MOD1;
	break;

     case CTS_MOD2:
	copy_mod2	= true;
	load_data	= 1 << BIDX_MOD2;
	break;

     case CTS_OVR1:
	copy_ovr1	= true;
	load_data	= 1 << BIDX_OVR1;
	break;

     case CTS_CT0CODE:
	copy_ct0code	= true;
	load_data	= 1 << BIDX_BAD0CODE;
	break;

     case CTS_CT0DATA:
	copy_ct0data	= true;
	load_data	= 1 << BIDX_BAD0DATA;
	break;

     case CTS_CT1CODE:
	copy_ct1code	= true;
	load_data	= 1 << BIDX_BAD1CODE;
	break;

     case CTS_CT1DATA:
	copy_ct1data	= true;
	load_data	= 1 << BIDX_MOD1 | 1 << BIDX_MOD2 | 1 << BIDX_OVR1;
	break;

     case CTS_BRRES:
     case CTS_SZS:
	copy_brres	= true;
	// fall through

     case CTS_TEX0:
	copy_tex	= true;
	copy_ct1data	= true;
	load_data	= 1 << BIDX_BAD1CODE
			| 1 << BIDX_MOD1
			| 1 << BIDX_MOD2
			| 1 << BIDX_OVR1;
	use_dynamic	= opt_dynamic_ctcode;
	break;

     case CTS_BMG:
	return SaveMessageCTCODE(ctcode,fname,set_time);
	break;

     case CTS_TRACKS:
	return SaveTrackListByOptCTCODE(ctcode,fname,set_time);
	break;

     case CTS_LIST:
     case CTS_REF:
     case CTS_FULL:
	{
	    output_mode_t omode;
	    if (outmode)
		memcpy(&omode,outmode,sizeof(omode));
	    else
		InitializeOutputMode(&omode);

	    switch (save_mode)
	    {
		case CTS_LIST: omode.mode = 1; break;
		case CTS_REF:  omode.mode = 2; break;
		case CTS_FULL: omode.mode = 3; break;
		default: break;
	    }
	    return SaveTextCTCODE(ctcode,fname,set_time,&omode);
	}

     default:
	return ERROR0(ERR_INTERNAL,0);
    }


    //--- setup sections

    DASSERT( CTL_US == CTL_EU + 1 );
    DASSERT( CTL_JP == CTL_EU + 2 );
    DASSERT( CTL_KO == CTL_EU + 3 );
    BZ2Source_t *bz2s = srctab + BIDX__N * ( lang_mode < CTL_EU ? 0 : lang_mode - CTL_EU );
    DASSERT( bz2s >= srctab && bz2s < srctab + sizeof(srctab)/sizeof(*srctab) );

    if (load_data)
    {
	if ( SearchListBZ2S( bz2s, BIDX__N, ct_dir_list.field,
				ct_dir_list.used, opt_ct_log ) != BIDX__N )
	{
	    uint i, count = 0;
	    char *dest = pathbuf;
	    for ( i = 0; i < BIDX__N; i++ )
	    {
		if ( load_data & 1 << i )
		{
		    BZ2Source_t *bs = bz2s + i;
		    if ( !bs->data || !bs->size )
		    {
			count++;
			ccp sem = strchr(bs->search,';');
			dest = snprintfE(dest,pathbuf+sizeof(pathbuf),", %.*s",
				(int)( sem ? sem - bs->search : strlen(bs->search) ),
				bs->search );
		    }
		}
	    }
	    if (count)
		return ERROR0(ERR_INVALID_DATA,
			    "Missing CT-CODE file%s: %s\n",
			    count == 1 ? "" : "s", pathbuf+2 );
	}


	//--- deactivate region patch

	{
	    static const u32 pal[] = { 0x8065920c, 0x80653644, 0x8065a034, 0 };
	    static const u32 usa[] = { 0x80654d84, 0x8064f1bc, 0x80655bac, 0 };
	    static const u32 jap[] = { 0x80658878, 0x80652cb0, 0x806596a0, 0 };
	    static const u32 kor[] = { 0 }; // [[2do]] [[ct-korea]]
	    DeactivateMOD2( bz2s+BIDX_MOD2, lang_mode, pal, usa, jap, kor );
	}


	//--- re-activate old spiny

	if ( old_spiny )
	{
	    static const u32 pal[] = { 0x808a5b30, 0 };
	    static const u32 usa[] = { 0x808a1058, 0 };
	    static const u32 jap[] = { 0x808a4c90, 0 };
	    static const u32 kor[] = { 0 }; // [[2do]] [[ct-korea]]
	    DeactivateMOD2( bz2s+BIDX_MOD2, lang_mode, pal, usa, jap, kor );
	}
    }


    //--- setup

    size_t cur_off = 0, off = 0, tex_off = 0;
    PrepareExportCTCODE(ctcode);


    //--- open mem file

    MemFile_t mf;
    InitializeMemFile(&mf,CT_CODE_BRRES_SIZE,0);
    SetFileAttrib(&mf.fatt,&ctcode->fatt,0);
    enumError err = ERR_OK;


    //--- setup dynamic CTGP_CODE size

    // all offsets are relative to CT_CODE_TEX_OFFSET

    u32 off_head	= 0;
    u32 off_cup		= off_head	+ sizeof(ctcode->ct_head);
    u32 off_crs		= off_cup	+ ctcode->cup_size;
    u32 off_mod1	= CT_CODE_EXTEND_OFFSET + CT_CODE_MOD1_OFFSET;
    u32 off_mod2	= CT_CODE_EXTEND_OFFSET + CT_CODE_MOD2_OFFSET;
    u32 off_ovr1	= CT_CODE_EXTEND_OFFSET + CT_CODE_OVR1_OFFSET;
    u32 off_image	= CT_CODE_TEX_IMAGE_OFF - CT_CODE_TEX_OFFSET;
    u32 off_term	= CT_CODE_TEX_SIZE - CT_CODE_TEX_OFFSET;

    if (use_dynamic)
    {
	fprintf(stderr,"off: %x %x %x %x %x %x | %x %x\n",
		off_head, off_cup, off_crs, off_mod1, off_mod2, off_ovr1,
		off_image, off_term );
    }


    //--- write strap header

    if ( !err && copy_brres )
    {
	u8 *data;
	uint size;
	DecodeBZIP2(&data,&size,0,ctcode_brres_base_bz2,sizeof(ctcode_brres_base_bz2));

	if (opt_image_dir)
	{
	    struct ftab_t
	    {
		u32 offset;
		u32 width;
		ccp fname;
	    };

	    const uint height_of_all = 456;
	    static const struct ftab_t ftab[] =
	    {
		{ 0x000140, 832, "strapA_16_9_832x456"	},
		{ 0x0b9580, 608, "strapA_608x456"	},
		{ 0x140bc0, 832, "strapB_16_9_832x456"	},
		{ 0x1fa000, 608, "strapB_608x456"	},
		{ 0x281640, 832, "strapC_832x456"	},
		{ 0x33aa80, 608, "strapC_608x456"	},
		{ 0x3c20c0, 832, "strapD_832x456"	}, // relevant image for 16:9 and 4:3
		{ 0x47b500, 608, "strapD_608x456"	},
		{0,0,0}
	    };

	    static const ccp exttab[] =
	    {
		".png", ".PNG",
		".tex", ".TEX",
		".tex0",".TEX0",
		"",
		0
	    };

	    Image_t img;
	    InitializeIMG(&img);

	    const struct ftab_t *fi;
	    for ( fi = ftab; fi->width; fi++ )
	    {
		const ccp *ext;
		for ( ext = exttab; *ext; ext++ )
		{
		    ccp path = PathCatPPE(pathbuf,sizeof(pathbuf),opt_image_dir,fi->fname,*ext);

		    if (LoadIMG(&img,false,path,0,false,false,true))
			continue;

		    if ( img.width != fi->width || img.height != height_of_all )
		    {
			if ( verbose >= 0 )
			    fprintf(stdlog," - load image [%s,%ux%u -> %ux%u]: %s\n",
				PrintFormatIMG(&img), img.width, img.height,
				fi->width, height_of_all, path );
			SmartResizeIMG(&img,false,0,fi->width,height_of_all);
		    }
		    else if ( verbose >= 0 )
			fprintf(stdlog," - load image [%s,%ux%u]: %s\n",
				PrintFormatIMG(&img), img.width, img.height, path );

		    ConvertIMG(&img,false,0,IMG_RGB565,0);
		    TRACE(" -> [%s,%ux%u]: *** %u *** %s\n",
				PrintFormatIMG(&img), img.width, img.height,
				img.data_size, path );

		    const uint size = fi->width * height_of_all * 2;
		    if ( img.iform == IMG_RGB565 && img.data_size == size )
		    {
			memcpy(data+fi->offset,img.data,size);
			break;
		    }
		}
	    }
	    ResetIMG(&img);
	}

	err = WriteMemFileAt( &mf, 0, data, size );
	FREE(data);
	off = tex_off = size;
    }


    //--- write tex header

    if ( !err && copy_tex )
    {
	err = WriteMemFileAt( &mf, off, ctcode_tex_header, sizeof(ctcode_tex_header) );
	if ( !err && bz2s[BIDX_BAD1CODE].status  )
	    err = WriteMemFileAt( &mf, off+0x40, bz2s[BIDX_BAD1CODE].data,
						 bz2s[BIDX_BAD1CODE].size );

	if (copy_brres)
	{
	    u8 *ptr = GetDataMemFile(&mf,off+12,4);
	    if (ptr)
		write_be32(ptr,0xffafd500);
	}
	off += CT_CODE_TEX_OFFSET;
    }


    //--- write CTCODE data

    if ( !err && copy_ct1data )
    {
//-----------------
if (use_dynamic) {
//-----------------
	err = WriteMemFileAt( &mf, off + off_term, "", 0 );

	if (!err)
	    err = WriteMemFileAt( &mf, off + off_head,
				&ctcode->ct_head, sizeof(ctcode->ct_head) );

	if ( !err && ctcode->cup )
	    err = WriteMemFileAt( &mf, off + off_cup, ctcode->cup, ctcode->cup_size );
PRINT("space behind cup: %x\n",off_crs-off_cup-ctcode->cup_size);

	if ( !err && ctcode->crs )
	    err = WriteMemFileAt( &mf, off + off_crs, ctcode->crs, ctcode->crs_size );
#if 1
{
    uint space = off_mod1 - off_crs - ctcode->crs_size;
    fprintf(stderr,"space behind crs: %x => (%zu) + %zu tracks\n",
	space,
	(ctcode->crs_size-sizeof(*ctcode->crs))/sizeof(ctcode_crs1_data_t),
	space/sizeof(ctcode_crs1_data_t) );
}
#endif

	if ( !err && bz2s[BIDX_MOD1].data )
	    err = WriteMemFileAt( &mf, off + off_mod1,
					bz2s[BIDX_MOD1].data, bz2s[BIDX_MOD1].size );

	if ( !err && bz2s[BIDX_MOD2].data )
	    err = WriteMemFileAt( &mf, off + off_mod2,
					bz2s[BIDX_MOD2].data, bz2s[BIDX_MOD2].size );

	if ( !err && bz2s[BIDX_OVR1].data )
	    err = WriteMemFileAt( &mf, off + off_ovr1,
					bz2s[BIDX_OVR1].data, bz2s[BIDX_OVR1].size );
//--------------
} else {
//--------------
	if (!err)
	    err = WriteMemFileAt( &mf, off + CT_CODE_EXTEND_OFFSET + CT_CODE_EXTEND_SIZE,
					"", 0 );

	err = WriteMemFileAt( &mf, off,
				&ctcode->ct_head, sizeof(ctcode->ct_head) );

	if ( !err && ctcode->cup )
	    err = WriteMemFile( &mf, ctcode->cup, ctcode->cup_size );

	if ( !err && ctcode->crs )
	    err = WriteMemFile( &mf, ctcode->crs, ctcode->crs_size );

	if ( !err && bz2s[BIDX_MOD1].data )
	    err = WriteMemFileAt( &mf, off + CT_CODE_EXTEND_OFFSET + CT_CODE_MOD1_OFFSET,
					bz2s[BIDX_MOD1].data, bz2s[BIDX_MOD1].size );

	if ( !err && bz2s[BIDX_MOD2].data )
	    err = WriteMemFileAt( &mf, off + CT_CODE_EXTEND_OFFSET + CT_CODE_MOD2_OFFSET,
					bz2s[BIDX_MOD2].data, bz2s[BIDX_MOD2].size );

	if ( !err && bz2s[BIDX_OVR1].data )
	    err = WriteMemFileAt( &mf, off + CT_CODE_EXTEND_OFFSET + CT_CODE_OVR1_OFFSET,
					bz2s[BIDX_OVR1].data, bz2s[BIDX_OVR1].size );
//--------------
}
//--------------
    }


    //--- write isolated sections

    if ( !err && copy_cup1 && ctcode->cup )
	err = WriteMemFileAt( &mf, cur_off, ctcode->cup, ctcode->cup_size );

    if ( !err && copy_crs1 && ctcode->crs )
	err = WriteMemFileAt( &mf, cur_off, ctcode->crs, ctcode->crs_size );

    if ( !err && copy_mod1 && bz2s[BIDX_MOD1].status )
	err = WriteMemFileAt( &mf, cur_off, bz2s[BIDX_MOD1].data, bz2s[BIDX_MOD1].size );

    if ( !err && copy_mod2 && bz2s[BIDX_MOD2].status )
	err = WriteMemFileAt( &mf, cur_off, bz2s[BIDX_MOD2].data, bz2s[BIDX_MOD2].size );

    if ( !err && copy_ovr1 && bz2s[BIDX_OVR1].status )
	err = WriteMemFileAt( &mf, cur_off, bz2s[BIDX_OVR1].data, bz2s[BIDX_OVR1].size );

    if ( !err && copy_ct1code && bz2s[BIDX_BAD1CODE].status  )
	err = WriteMemFileAt( &mf, cur_off, bz2s[BIDX_BAD1CODE].data, bz2s[BIDX_BAD1CODE].size );

    if ( !err && copy_ct0code && bz2s[BIDX_BAD0CODE].status  )
	err = WriteMemFileAt( &mf, cur_off, bz2s[BIDX_BAD0CODE].data, bz2s[BIDX_BAD0CODE].size );

    if ( !err && copy_ct0data && bz2s[BIDX_BAD0DATA].status  )
	err = WriteMemFileAt( &mf, cur_off, bz2s[BIDX_BAD0DATA].data, bz2s[BIDX_BAD0DATA].size );


    //--- write image

    if ( !err && copy_tex )
    {
	u8 *data;
	uint size;
	DecodeBZIP2(&data,&size,0,ctcode_image_bz2, sizeof(ctcode_image_bz2));
	err = WriteMemFileAt( &mf, tex_off+CT_CODE_TEX_IMAGE_OFF, data, size );
	FREE(data);
    }


    //--- write strap string-pool

    if ( !err && copy_brres )
    {
	if (use_dynamic)
	    fprintf(stderr,"str-pool: %x %zx\n",CT_CODE_STRINGS_OFF,off+off_term);
	u8 *data;
	uint size;
	DecodeBZIP2(&data,&size,0,ctcode_brres_strings_bz2, sizeof(ctcode_brres_strings_bz2));
	err = WriteMemFileAt( &mf, CT_CODE_STRINGS_OFF, data, size );
	FREE(data);
    }


    //--- terminate

    FileMode_t fmode = GetFileModeByOpt(testmode,0,0);
    if ( save_mode == CTS_SZS )
    {
	szs_file_t szs;
	AssignSZS(&szs,true,mf.data,mf.fend,true,FF_BRRES,0);
	mf.data = 0;
	err = SaveSZS(&szs,fname,opt_overwrite,true);
	ResetSZS(&szs);
    }
    else
	err = SaveMemFile(&mf,fname,0,fmode,false);

    ResetMemFile(&mf);
    ClearBZ2S(srctab,-1);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

int	opt_ct_log	= 0;
bool	old_spiny	= false;
u32	crs1_property	= 0x800;

///////////////////////////////////////////////////////////////////////////////

int ScanOptCtLog ( ccp arg )
{
    static const KeywordTab_t keytab[] =
    {
	{ SEA_LOG_OFF,		"OFF",		"NONE", 0 },
	{ SEA_LOG_SUCCESS,	"SUCCESS",	0, 0 },
	{ SEA_LOG_FOUND,	"FOUND",	0, 0 },
	{ SEA_LOG_ALL,		"ALL",		0, 0 },
	{0,0,0,0}
    };

    if ( !arg || !*arg )
	opt_ct_log++;
    else if ( *arg >= '0' && *arg <= '9' )
    {
	opt_ct_log = str2l(arg,0,10);
	if ( opt_ct_log < 0 )
	    opt_ct_log = 0 ;
    }
    else
    {
	int abbrev;
	const KeywordTab_t *key = ScanKeyword(&abbrev,arg,keytab);
	if (!key)
	{
	    PrintKeywordError(keytab,arg,abbrev,0,
		    "keyword for option --ct-log");
	    return 1;
	}
	opt_ct_log = key->id;
    }

    if ( opt_ct_log > SEA_LOG_ALL )
	opt_ct_log = SEA_LOG_ALL;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptCRS1 ( ccp arg )
{
    return ERR_OK != ScanSizeOptU32(
			&crs1_property,		// u32 * num
			arg,			// ccp source
			1,			// default_factor1
			0,			// int force_base
			"crs1",			// ccp opt_name
			0,			// u64 min
			0xffffffff,		// u64 max
			1,			// u32 multiple
			0,			// u32 pow2
			true			// bool print_err
			);
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptAllowSlots ( ccp arg )
{
    enum { K_TRACKS, K_ARENAS, K_SPECIAL, K_CTCODE };

    static const KeywordTab_t keytab[] =
    {
	{ K_TRACKS,	"TRACKS",		0, 0 },
	{ K_ARENAS,	"ARENAS",		0, 0 },
	{ K_SPECIAL,	"SPECIAL",		0, 0 },
	{ K_CTCODE,	"CTCODE",	"CT-CODE", 0 },
	{0,0,0,0}
    };

    SetupUsedSlots();
    if ( !arg || !*arg )
	return 0;

    for(;;)
    {
	while ( *arg == ' ' || *arg == '\t' || *arg == ',' )
	    arg++;

	const u8 value = *arg == '-' ? 2 : 0;
	if (value)
	    arg++;

	while ( *arg == ' ' || *arg == '\t' )
	    arg++;
	if (!*arg)
	    break;

	u32 stat = 0, n1 = 0, n2 = 0;
	if ( *arg >= '0' && *arg <= '9' )
	{
	    arg = ScanRangeU32(arg,&stat,&n1,&n2,0,BMG_N_CT_TRACK-1);
	}
	else
	{
	    char namebuf[50];
	    char *dest = namebuf;
	    while ( isalnum((int)*arg) )
	    {
		if ( dest < namebuf + sizeof(namebuf)-1 )
		    *dest++ = *arg;
		arg++;
	    }
	    *dest = 0;

	    int abbrev;
	    const KeywordTab_t *key = ScanKeyword(&abbrev,namebuf,keytab);
	    if (!key)
	    {
		PrintKeywordError(keytab,namebuf,abbrev,0,
			"keyword for option --allow-slots");
		return 1;
	    }
	    DASSERT(key);

	    switch(key->id)
	    {
		case K_TRACKS:
		    stat = 2;
		    n1 = 0;
		    n2 = BMG_N_TRACK-1;
		    break;

		case K_ARENAS:
		    stat = 2;
		    n1 = BMG_N_TRACK;
		    n2 = BMG_N_TRACK + BMG_N_ARENA - 1;
		    break;

		case K_CTCODE:
		    memset(ctcode_used_slot,value,BMG_N_TRACK+BMG_N_ARENA);
		    // fall through

		case K_SPECIAL:
		    stat = 2;
		    n1 = 0x36;
		    n2 = 0x3a;
		    break;
	    }
	}

	noPRINT("RANGE[%d]: 0x%02x .. 0x%02x\n",stat,n1,n2);
	if ( stat && n1 <= n2 )
	{
	    DASSERT( n2 < BMG_N_CT_TRACK );
	    memset(ctcode_used_slot + n1, value, n2+1-n1 );
	}
    }

    if ( logging > 1 ) // must be set before --allow-slots
	HexDump16(stdlog,0,0,ctcode_used_slot,sizeof(ctcode_used_slot));
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


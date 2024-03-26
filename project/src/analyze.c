
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
 *   Copyright (c) 2011-2024 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include "lib-brres.h"
#include "lib-breff.h"
#include "lib-image.h"
#include "db-kcl.h"
#include "lib-kcl.h"
#include "lib-kmp.h"
#include "lib-mdl.h"
#include "lib-pat.h"
#include "db-object.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			option support			///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    ANA_MD_DEFAULT	= 0x0001,
    ANA_MD_BRRES	= 0x0002,
    ANA_MD_MDL		= 0x0004,
    ANA_MD_PAT		= 0x0008,
    ANA_MD_TEX		= 0x0010,

    ANA_MD_BREFF	= 0x0100,
    ANA_MD_BREFT	= 0x0200,
    ANA_MD_KCL		= 0x0400,
    ANA_MD_TPL		= 0x0800,

    ANA_KMP_GOBJ	= 0x1000,
    ANA_KMP_KTPT	= 0x2000,
    ANA_MKWANA		= 0x4000,

    ANA_M_BRRES		= ANA_MD_BRRES | ANA_MD_MDL | ANA_MD_PAT | ANA_MD_TEX,
    ANA_M_KMP		= ANA_KMP_GOBJ | ANA_KMP_KTPT,

    ANA_M_ALL		= 0xffff,
};

uint opt_analyze_mode = ANA_MD_DEFAULT;

//-----------------------------------------------------------------------------

int ScanOptAnalyzeMode ( ccp arg )
{
    static const KeywordTab_t tab[] =
    {
	{ 0,			"NONE",		"-",		ANA_M_ALL },
	{ ANA_M_ALL,		"ALL",		0,		0 },

	{ ANA_MD_BRRES,		"BRRES",	"BRES",		0 },
	{ ANA_MD_MDL,		"MDL0",		0,		0 },
	{ ANA_MD_PAT,		"PAT0",		0,		0 },
	{ ANA_MD_TEX,		"TEX0",		0,		0 },

	{ ANA_MD_BREFF,		"BREFF",	"REFF",		0 },
	{ ANA_MD_BREFT,		"BREFT",	"REFT",		0 },
	{ ANA_MD_TPL,		"TPL",		0,		0 },

	{ ANA_MD_KCL,		"KCL",		0,		0 },

	{ ANA_M_KMP,		"KMP",		0,		0 },
	{ ANA_KMP_GOBJ,		"GOBJ",		0,		0 },
	{ ANA_KMP_KTPT,		"KTPT",		0,		0 },

	{ ANA_MKWANA,		"MKW_ANA",	"MKWANA",	0 },

	{ 0,0,0,0 }
    };

    int stat = ScanKeywordList(arg,tab,0,true,0,0,
				"Option --analyze-mode",ERR_SYNTAX);
    if ( stat != -1 )
    {
	opt_analyze_mode = stat;
	return 0;
    }
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			defines				///////////////
///////////////////////////////////////////////////////////////////////////////

#define DUMP_NULL_OFFSET	0   // dump null values
#define DUMP_INVALID_OFFSET	0   // dump offset, that point before string pool
#define DUMP_GROUP_POINTER	0   // dump group relative offets
#define DUMP_ENTRY_POINTER	0   // dump entry relative offets
#define DUMP_VERSION		1   // dump version info of entries

#define ANA_SPEC_STRINGS	0   // enable special strings analysis
#define ANA_SPLT_SECT		0   // try to find split sections

//--- enable special tests
#define ANA_MDL_11_0		1   // MDL vers 11 sect 0
#define ANA_MDL_11_8		1   // MDL vers 11 sect 8
#define ANA_MDL_11_8_0_VAL	1   // MDL vers 11 sect 8.0
#define ANA_MDL_11_8_3_VAL	1   // MDL vers 11 sect 8.3
#define ANA_TEX_HD		1   // TEX header

//
///////////////////////////////////////////////////////////////////////////////
///////////////			structs				///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct analyze_item_t
{
    u32			offset;		// data offset
    brres_group_t	* grp;		// pointer to group
    brres_entry_t	* entry;	// pointer to entry
    ccp			entry_name;	// pointer to entry string
    ccp			info;		// short item info

    int			index1;		// index level 1 (grp/section index)
    int			index2;		// index level 2 (entry index)
    int			index3;		// index level 3
    uint		brsub_mode;	// brsub mode

} analyze_item_t;

///////////////////////////////////////////////////////////////////////////////

#define MAX_ANA_ITEM 10000

typedef struct xanalyze_t
{
    szs_file_t		* szs;
    u8			* data;
    uint		data_size;

    file_format_t	fform;
    ccp			ff_name;
    int			version;

    analyze_item_t	list[MAX_ANA_ITEM];
    uint		used;

} xanalyze_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    AnalyzeBRSUB(), helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static int cmp_analyze_item ( const void * va, const void * vb )
{
    const analyze_item_t * a = va;
    const analyze_item_t * b = vb;

    if ( a->offset < b->offset ) return -1;
    if ( a->offset > b->offset ) return  1;
    if ( a->index1 < b->index1 ) return -1;
    if ( a->index1 > b->index1 ) return  1;
    if ( a->index2 < b->index2 ) return -1;
    if ( a->index2 > b->index2 ) return  1;
    if ( a->index3 < b->index3 ) return -1;
    if ( a->index3 > b->index3 ) return  1;
    if ( a         < b         ) return -1;
    if ( a         > b         ) return  1;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static analyze_item_t * add_item
(
    xanalyze_t		* ana,		// analyse
    ccp			info,		// short item info
    brres_group_t	* grp,		// pointer to group
    uint		grp_index,	// index of group/section
    uint		index3,		// free second index
    brres_entry_t	* entry,	// pointer to entry
    u8			* data		// pointer to entry related data
)
{
    DASSERT(ana);
    if ( ana->used < MAX_ANA_ITEM )
    {
	analyze_item_t * item = ana->list + ana->used++;

	item->offset		= data - ana->data;
	item->grp		= grp;
	item->entry		= entry;
	item->entry_name	= "-";
	item->index1		= grp_index;
	item->index2		= grp && entry ? entry - grp->entry : 0;
	item->index3		= index3;
	item->brsub_mode	= BRSUB_MODE3(ana->fform,ana->version,grp_index);
	item->info		= info;
	return item;
    }

    ERROR0(ERR_FATAL,"Analyze list full (max=%u): %s\n",MAX_ANA_ITEM,ana->szs->fname);
    ExitFixed(ERR_FATAL);
}

///////////////////////////////////////////////////////////////////////////////

static int collect_raw_data
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    u8			* data,		// data pointer
    u32			data_size	// size of 'data'
)
{
    DASSERT(it);
    xanalyze_t * ana = it->param;
    DASSERT(ana);

    // additional data
    add_item(ana,"raw data",0,-1,1,0,data);

    // ignore other
    add_item(ana,"ignore",0,0xffff,0,0,data+data_size);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int collect_group_data
(
    struct
      brsub_iterator_t	* it,		// iterator struct with all infos
    brres_group_t	* grp,		// pointer to group
    uint		grp_index	// index of group
)
{
    DASSERT(it);
    xanalyze_t * ana = it->param;
    DASSERT(ana);
    add_item(ana,"group",grp,0xffff,0,grp->entry,(u8*)grp);
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			collect_entry_data()		///////////////
///////////////////////////////////////////////////////////////////////////////

static int collect_entry_data
(
    struct
    brsub_iterator_t	* it,		// iterator struct with all infos
    brres_group_t	* grp,		// pointer to group
    uint		grp_index,	// index of group
    brres_entry_t	* entry,	// pointer to entry
    u8			* data		// pointer to entry related data
)
{
    DASSERT(it);
    xanalyze_t * ana = it->param;
    DASSERT(ana);

    analyze_item_t * item	= add_item(ana,"section",grp,grp_index,0,entry,data);
    item->entry_name		= GetStringBRRES(it->szs,grp,
					it->endian->rd32(&entry->name_off),"?");

 #if ANA_SPLT_SECT

    switch (BRSUB_MODE3(ana->fform,ana->version,grp_index))
    {
      case BRSUB_MODE3(FF_MDL,11, 1):
      case BRSUB_MODE3(FF_MDL,11, 2):
      case BRSUB_MODE3(FF_MDL,11, 3):
      case BRSUB_MODE3(FF_MDL,11, 4):
      case BRSUB_MODE3(FF_MDL,11, 5):
      case BRSUB_MODE3(FF_MDL,11, 9):
      case BRSUB_MODE3(FF_MDL,11,10):
	{
	    noPRINT(">>> %s.%u.%02u: %5x,%5x,%5x\n",
			    ana->ff_name, grp_index, (int)( entry - grp->entry ),
			    it->endian->rd32(data),
			    it->endian->rd32(data+0x08),
			    it->endian->rd32(data+0x0c));
	    add_item(ana,"end of section",grp,grp_index,99,entry,
			    data + it->endian->rd32(data));
	}
	break;

      case BRSUB_MODE3(FF_MDL,11,8):
	{
	    u32 off = it->endian->rd32(data);
	    item = add_item(ana,"end of section",grp,grp_index,99,entry,data+off);

	    uint n_lay = it->endian->rd32(data+0x2c);
	    off = it->endian->rd32(data+0x30);
	    if ( n_lay && off )
	    {
		uint i;
		for ( i = 0; i < n_lay; i++, off += 0x34 )
		    item = add_item(ana,"mdl 8 layer",grp,grp_index,1,entry,data+off);
	    }
	    else
		off = 0x418;
	    item = add_item(ana,"end of mdl 8 layer",grp,grp_index,2,entry,data+off);

	    off = it->endian->rd32(data+0x3c);
	    if (off)
		item = add_item(ana,"mdl 8 setup code",grp,grp_index,3,entry,data+off);

	    off = it->endian->rd32(data+0x28);
	    if (off)
		item = add_item(ana,"mdl 8 shader @28",grp,grp_index,4,entry,data+off);
	}
	break;

      case BRSUB_MODE3(FF_MDL,11,11):
	{
	    item = add_item(ana,"end of section",grp,grp_index,99,entry,
			data + it->endian->rd32(data) * 8 );
	}
	break;

      case BRSUB_MODE3(FF_PAT,4,0):
	{
	    uint n		= it->endian->rd16(data+0x0c);
	    u8 * d		= data + 0x14 + 8*n;
	    item		= add_item(ana,"pat",grp,grp_index,1,entry,d);

	    while ( it->endian->rd32(d) )
		d += 4;
	    item		= add_item(ana,"end of section",grp,grp_index,99,entry,d+4);
	}
	break;
    }
 #endif // ANA_SPLT_SECT

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AnalyzeBRSUB()			///////////////
///////////////////////////////////////////////////////////////////////////////
// use [[subfile]]

void AnalyzeBRSUB ( szs_file_t * szs, u8 * data, uint data_size, ccp name )
{
    if ( opt_analyze_mode != ANA_MD_DEFAULT && !(opt_analyze_mode&ANA_M_BRRES) )
	return;

    if ( IsValidBRSUB(data,data_size,data_size,0,FF_UNKNOWN,0,0,0) >= VALID_ERROR )
	return;

// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,data_size,data_size);
    ccp ff_name = GetNameFF(0,fform);
    PRINT("AnalyzeBRSUB() %s %s [size=%x]\n",ff_name,szs->fname,data_size);

    if ( opt_analyze_mode != ANA_MD_DEFAULT && !(opt_analyze_mode&ANA_MD_BRRES) )
    {
	switch (fform)
	{
	    case FF_MDL:
		if (!(opt_analyze_mode&ANA_MD_MDL))
		    return;
		break;

	    case FF_PAT:
		if (!(opt_analyze_mode&ANA_MD_PAT))
		    return;
		break;

	    case FF_TEX:
	    case FF_TEX_CT:
		if (!(opt_analyze_mode&ANA_MD_TEX))
		    return;
		fform = FF_TEX;
		break;

	    default:
		return;
	}
    }

    FILE * f = CreateAnalyzeFile();
    if (!f)
	return;

    static bool init_done = false;
    if (!init_done)
    {
	init_done = true;

	#if ANA_MDL_11_0
	    fprintf(f,"#HAVE# HAVE-MDL.11.0\n");
	#endif

	#if ANA_MDL_11_8
	    fprintf(f,"#HAVE# HAVE-MDL.11.8\n");
	    #if ANA_MDL_11_8_0_VAL
		fprintf(f,"#HAVE# HAVE-MDL.11.8.0-VAL HAVE-MDL-VAL0.11.8\n");
	    #endif
	    #if ANA_MDL_11_8_3_VAL
		fprintf(f,"#HAVE# HAVE-MDL.11.8.3-VAL HAVE-MDL-VAL3.11.8\n");
	    #endif
	#endif

	#if ANA_TEX_HD
	    fprintf(f,"#HAVE# HAVE-TEX-HD\n");
	#endif
    }

    //printf("ANALYZE %s: %s\n",ff_name,szs->fname);
    fprintf(f,"\n\f\nANALYZE %s: %s\n\n",ff_name,szs->fname);

    const endian_func_t * endian = szs->endian;
    DASSERT(endian);
    const u32 version = endian->rd32(data+8);
    const int n_sect  = GetSectionNumBRSUB(data,data_size,endian);
    //const uint brsub_mode2 = BRSUB_MODE2(fform,version);


    //--- special PAT dump

    if ( fform == FF_PAT && version == 4 )
    {
	u32 off0 = endian->rd32(data+16+0*4);
	u32 off1 = endian->rd32(data+16+1*4); if (!off1) off1 = off0;
	u32 off2 = endian->rd32(data+16+2*4); if (!off2) off2 = off1;
	u32 off3 = endian->rd32(data+16+3*4); if (!off3) off3 = off2;
	u32 off4 = endian->rd32(data+16+4*4); if (!off4) off4 = off3;
	u32 off5 = endian->rd32(data+16+5*4); if (!off5) off5 = off4;

	ccp fname = strstr(szs->fname,"/object/");
	if (fname)
	    fname += 8;
	else
	{
	    fname = strstr(szs->fname,"/track/");
	    if (fname)
		fname += 7;
	    else
		fname = name;
	}

	pat_analyse_t ana;
	IsValidPAT(data,data_size,0,szs,fname,0,&ana);
	fprintf(f,"#PAT-N# %4d %4d %4d %4d  %4d %4d %4d %4d"
		" | %4d %4d %4d %4d %4d %4d | v=%d,%d %s\n",
		endian->rd16(data+0x2c+0*2),
		endian->rd16(data+0x2c+1*2),
		endian->rd16(data+0x2c+2*2),
		endian->rd16(data+0x2c+3*2),
		endian->rd16(data+0x2c+4*2),
		endian->rd16(data+0x2c+5*2),
		endian->rd16(data+0x2c+6*2),
		endian->rd16(data+0x2c+7*2),
		off1 - off0,
		off2 - off1,
		off3 - off2,
		off4 - off3,
		off5 - off4,
		data_size - off5,
		ana.valid, ana.data_complete, fname );

	if (ana.data_complete)
	{
	    const pat_s0_bhead_t *bh = ana.s0_base;
	    fprintf(f,
		"#PAT-BH# %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x | %s\n",
		be32(&bh->size),
		be16(&bh->unknown_04),
		be16(&bh->n_elem),
		be16(&bh->unknown_08),
		be16(&bh->unknown_0a),
		be16(&bh->n_unknown),
		be16(&bh->unknown_0e),
		be16(&bh->unknown_10),
		be16(&bh->unknown_12),
		be16(&bh->unknown_14),
		be16(&bh->unknown_16),
		fname );

	    const pat_s0_belem_t *be = bh->elem;
	    uint i;
	    for ( i = 0; i < ana.n_sect0; i++, be++ )
		fprintf(f,
			"#PAT-BE.%u# %4x %4x %4x %4x %4x %4x | %s\n",
			i,
			be16(&be->unknown_00),
			be16(&be->unknown_02),
			be16(&be->unknown_04),
			be16(&be->unknown_06),
			be32(&be->offset_name),
			be32(&be->offset_strref),
			fname );

	    for ( i = 0; i < PAT_MAX_ELEM; i++ )
	    {
		const pat_s0_sref_t *sr = ana.s0_sref[i];
		if (!sr)
		    continue;
		fprintf(f,
			"#PAT-SR.%u# %4x %4x %4x %4x | %s\n",
			i,
			be32(&sr->offset_name),
			be16(&sr->unknown_04),
			be16(&sr->type),
			be32(&sr->offset_strlist),
			fname );
	    }

	    for ( i = 0; i < PAT_MAX_ELEM; i++ )
	    {
		const pat_s0_shead_t *sh = ana.s0_shead[i];
		if (!sh)
		    continue;
		const float fm = bef4(&sh->factor);
		const float f1 = bef4(&sh->elem[0].time);
		const float f2 = bef4(&sh->elem[ana.n_s0_list[i]-1].time);
		fprintf(f,
			"#PAT-SH.%u# %4x %4x %8.5f [%8.5f,%8.5f] | %s\n"
			"#PAT-SE#\n",
			i,
			be16(&sh->n_elem),
			be16(&sh->unknown_02),
			fm, fm * f2, fm * (f2-f1),
			fname );

		float last = 0.0;
		uint j;
		for ( j = 0; j < ana.n_s0_list[i]; j++ )
		{
		    const pat_s0_selem_t *se = sh->elem + j;
		    float time = bef4(&se->time);
		    fprintf(f,
			"#PAT-SE.%u.%02u# %11.5f [%8.5f] %4x %4x | %s\n",
			i, j,
			time, time-last,
			be16(&se->index),
			be16(&se->unknown_06),
			fname );
		    last = time;
		}
	    }
	}

	if ( ana.s3_list && ana.n_sect1 )
	{
	    fprintf(f,"#PAT-S3#");

	    uint i;
	    for ( i = 0; i < ana.n_sect1; i++ )
		fprintf(f," %8x",be32(ana.s3_list+i));
	    fprintf(f," | %s\n",fname);
	}
    }


    //---  version and section number test

 #if DUMP_VERSION
    {
	//--- find version and section count

	const u32 size	= endian->rd32(data+4);
	const int n1	= GetKnownSectionNumBRSUB(fform,version);
	const int n2	= GetGenericSectionNumBRSUB(data,data_size,endian);
	const int n3	= GetSectionNumBRSUB(data,data_size,endian);

	//printf("#VERSION# %s v=%02u n%c%2d,%2d,%2d : size=%05x : %s\n",
	//	ff_name, version, n1==n2 ? '=' :'?', n1, n2, n3, size, szs->fname );

	fprintf(f,"#VERSION# %s v=%02u n%c%2d,%2d,%2d : size=%05x : %s\n",
		ff_name, version, n1==n2 ? '=' :'?', n1, n2, n3, size, szs->fname );
    }
 #endif


    //--- ffname with version

    char ff_buf[20];
    snprintf(ff_buf,sizeof(ff_buf),"%s-%x",ff_name,version);
    ff_name = ff_buf;

    {
	fprintf(f,"#OFFSET# %s :",ff_name);
	uint i;
	for ( i = 0; i < n_sect; i++ )
	    fprintf(f," %5x",endian->rd32(data+0x10+4*i));
	//fprintf(f," : %6x\n",endian->rd32(data+0x10+4*n_sect));
	fprintf(f," : %5x\n",data_size);
    }


    //--- setup xanalyze_t

    xanalyze_t ana;
    memset(&ana,0,sizeof(ana));
    ana.szs		= szs;
    ana.data		= data;
    ana.data_size	= data_size;
    ana.fform		= fform;
    ana.ff_name		= ff_name;
    ana.version		= version;

    analyze_item_t * item = ana.list;
    item->offset	= data_size;	// end marker
    item->index1	= -1;
    item->index3	= 99;
    item->info		= "END OF DATA";
    item++;
    item->index1	= -1;		// header = data begin
    item->entry_name	= name;
    item->info		= "BEGIN OF DATA";
    item++;
    ana.used		= item - ana.list;

    brsub_iterator_t it;
    memset(&it,0,sizeof(it));
    it.szs		= szs;
    it.brsub		= (void*)data;
    it.file_size	= szs->data + szs->size - (u8*)data;
    it.raw_func		= collect_raw_data;
    it.grp_func		= collect_group_data;
    it.entry_func	= collect_entry_data;
    it.param		= &ana;
    it.adjust		= true; // suppress error messages

    IterateStringsBRSUB(&it);


    //--- special string

 #if ANA_SPEC_STRINGS // find special strings

    enum { MAX_SPECIAL_STR = 100 };
    ccp special_str[MAX_SPECIAL_STR];
    uint n_special_str = 0;

    {
	ccp base = (ccp)szs->data + szs->max_data_off;
	u32 off = 4;
	while ( off < szs->size )
	{
	    ccp sptr = (ccp)base + off;
	    if (!*sptr)
		break;

	    #define ADD_STR(s) if ( n_special_str < MAX_SPECIAL_STR && !strcmp(sptr,s) ) \
				special_str[n_special_str++] = sptr;

	    ADD_STR("bds_eye.0");
	    ADD_STR("bds_eye.1");
	    ADD_STR("bds_eye.2");
	    ADD_STR("catherine_eye.0");
	    ADD_STR("catherine_eye.1");
	    ADD_STR("catherine_eye.2");
	    ADD_STR("diddy_eye.0");
	    ADD_STR("diddy_eye.1");
	    ADD_STR("diddy_eye.2");
	    ADD_STR("luigi_eye.0");
	    ADD_STR("luigi_eye.1");
	    ADD_STR("luigi_eye.2");
	    ADD_STR("waluigi_eye.0");
	    ADD_STR("waluigi_eye.1");
	    ADD_STR("waluigi_eye.2");

	    #undef ADD_STR

	    off += ALIGN32(strlen(sptr)+5,4);
	    ASSERT( n_special_str < MAX_SPECIAL_STR );
	}
    }
 #endif

    //--- sort & iterate data

 #if ANA_TEX_HD0
    analyze_item_t * texhd = 0;
 #endif

    qsort(ana.list,ana.used,sizeof(*ana.list),cmp_analyze_item);
    uint i;
    for ( i = 0; i < ana.used - 1; i++ )
    {
	analyze_item_t * item = ana.list + i;

	char idxbuf1[20], idxbuf2[30];
	if ( item->index1 < 0 )
	    snprintf(idxbuf1,sizeof(idxbuf1),"%s.++.%02u",
			ff_name, item->index3 );
	else if ( item->index3 == 99 )
	    snprintf(idxbuf1,sizeof(idxbuf1),"%s.%02u.~~",
			ff_name, item->index1 );
	else
	{
	    snprintf(idxbuf1,sizeof(idxbuf1),"%s.%02u.%02u",
			ff_name, item->index1, item->index3 );
	}
	snprintf(idxbuf2,sizeof(idxbuf2),"%s.%02d",
			idxbuf1, item->index2 );

	u8 *base = ana.data + item[0].offset;
	const uint sect_size = item[1].offset - item[0].offset;
	u32 val = endian->rd32(base);
	fprintf(f,"\n#SECT# %3u. %6x..%5x, size=%x, u32=%x->%x : %s >>> %s <<<\n",
		i, item->offset, item[1].offset,
		sect_size, val, item->offset + val,
		idxbuf2, item->info );

	if ( item->index2 > 0 ) // ( && item->index3 != 99 )
	    fprintf(f,"#SECT-SIZE# %s : %2u %2u %2u : %4u = 0x%04x : %s\n",
			ff_name,
			item->index1, item->index2, item->index3,
			sect_size, sect_size, item->info );

	//HexDump(f,0,0,4,16,base,16);

	if ( item[1].offset == item[0].offset )
	    continue;
	if ( item->index1 == 0xffff )
	    continue;

	fprintf(f,"#SIZE# %s size=%5x : %s tot-off=%6x/%x : >>> %s <<<\n",
			idxbuf1, sect_size,
			idxbuf2, item[0].offset, data_size, item->info );

	fprintf(f,"### %s.-----.cnt : %s off=%5x\n",
			idxbuf1, idxbuf2, item[0].offset );

	bool found = false;
	u8 *end  = ana.data + item[1].offset, *ptr;
	for ( ptr = base; ptr < end; ptr += 4 )
	{
	    ccp string;
	    u32 offset = endian->rd32(ptr);

	    if (!offset)
	    {
 #if DUMP_NULL_OFFSET // value == null
		fprintf(f,"### %s.%05x._0_ : %s val=%8x\n",
			idxbuf1,
			(int)( ptr - base ),
			idxbuf2,
			offset );
 #endif
		continue;
	    }

 #if DUMP_INVALID_OFFSET // value == other
	    if ( item->offset + offset < data_size )
	    {
		fprintf(f,"### %s.%05x._<_ : %s val=%8x\n",
			idxbuf1,
			(int)( ptr - base ),
			idxbuf2,
			offset );
	    }
 #endif

 #if DUMP_GROUP_POINTER // group pointer
	    if ( item->grp && ptr >= (u8*)item->grp )
	    {
		string = GetStringBRRES(szs,item->grp,offset,0);
		if ( string && *string > ' ' && *string < 0x7f )
		{
		    fprintf(f,"### %s.%05x.GRP : %s val=%8x : %s\n",
			idxbuf1,
			(int)( ptr - base ),
			idxbuf2,
			offset, string );
		    found = true;
		}
	    }
 #endif

 #if DUMP_ENTRY_POINTER // entry pointer
	    if ( item->entry && ptr >= (u8*)item->entry )
	    {
		string = GetStringBRRES(szs,item->entry,offset,0);
		if ( string && *string > ' ' && *string < 0x7f )
		{
		    fprintf(f,"### %s.%05x.ENT : %s val=%8x : %s\n",
			idxbuf1,
			(int)( ptr - base ),
			idxbuf2,
			offset, string );
		    found = true;
		}
	    }
 #endif
	    string = GetStringBRRES(szs,ana.data+item->offset,offset,0);
	    if ( string && *string > ' ' && *string < 0x7f )
	    {
		fprintf(f,"### %s.%05x.DAT : %s val=%8x : %s\n",
			idxbuf1,
			(int)( ptr - base ),
			idxbuf2,
			offset, string );

		found = true;

		if ( string == item->entry_name )
		    fprintf(f,"### %s.%05x.=== : %s val=%8x : %s\n",
			idxbuf1,
			(int)( ptr - base ),
			idxbuf2,
			offset, string );

		if ( item->index1 == -1 && ptr - base >= 4*n_sect )
		    fprintf(f,"### %s.%05x.NNN : %s val=%8x : %s\n",
			idxbuf1,
			(int)( ptr - base - 4*n_sect ),
			idxbuf2,
			offset, string );
	    }

 #if ANA_SPEC_STRINGS // find special strings
	    if (n_special_str)
	    {
		u8 * p2;
		for ( p2 = ptr; p2 < end; p2 += 4 )
		{
		    u32 offset = endian->rd32(p2);
		    if (!offset)
			continue;
		    string = GetStringBRRES(szs,ptr,offset,0);
		    int i;
		    for ( i = 0; i < n_special_str; i++ )
			if ( string == special_str[i] )
			    fprintf(f,"#SPEC# %s.%05x+%05x=%05x : off=%6x : %s\n",
				idxbuf1,
				(int)( ptr - base ),
				(int)( p2  - ptr ),
				(int)( p2  - base ),
				offset, string );
		}
	    }
 #endif
	}
	if (!found)
	    fprintf(f,"### %s.xxxxx.non : %s\n", idxbuf1, idxbuf2 );

 #if 0
	if ( fform == FF_MDL && item->old_index == 0xc0 )
	{
	    u32 *p = (u32*)base;
	    uint n = endian->rd32(p++), i;
	    for ( i = 0; i < n; i++ )
	    {
		const u32 v1 = endian->rd32(p++);
		const u32 v2 = endian->rd32(p++);

		fprintf(f,"### MDL-b.%02u: %6x %6x -> %6x %6x [d=%4x]\n",
			i, v1, v2,
			item[0].offset + v1, item[0].offset + v2,
			v2 - v1 );
	    }
	}
 #endif

     #if ANA_MDL_11_0
	if ( item->brsub_mode == BRSUB_MODE3(FF_MDL,11,0) )
	{
	 #ifdef TEST0
	    FILE *f = stdout;
	 #endif
	    ccp prefix = "#MDL.s0# ";
	    fprintf(f,"%s-------------\n%s%s -> %s\n",
			prefix,
			prefix, idxbuf2, szs->fname );

	    mdl_sect0_t ms0;
	    InitializeMDLs0(&ms0,base,sect_size);
	    fprintf(f,"%sSTAT: ok=%d, n=%d, size=%u/%u\n",
		prefix, ms0.ok, ms0.n, ms0.data_size, sect_size);
	    while (NextMDLs0(&ms0))
	    {
		fprintf(f,"%sDUMP[%02x,%2u]:",prefix,*ms0.cur,ms0.cur_size);
		HexDump(f, 1, ms0.cur-ms0.data_beg, 3, -50, ms0.cur, ms0.cur_size );
	    }
	    if (ms0.fail)
	    {
		fprintf(f,"%sFAIL[%02x,*?]:",prefix,*ms0.fail);
		HexDump(f, 1, ms0.fail-ms0.data_beg, 3, -50,
				ms0.fail, ms0.data_end - ms0.fail );
	    }
	}
     #endif

     #if ANA_MDL_11_8
	if ( item->brsub_mode == BRSUB_MODE3(FF_MDL,11,8) )
	{
	    ccp prefix = "#MDL.s8# ";
	    fprintf(f,"%s-------------\n%s%s -> %s -> %s\n",
			prefix,
			prefix, idxbuf2, szs->fname, name );
	    //u8 * off_base = base;
	    //hexdump_prefix = prefix;
	    //HexDump(f,0,base-off_base,4,16,base,sect_size);
	    if ( item->index3 == 0 )
	    {
		u32 shader_off	= endian->rd32(base+0x28);
		u32 layer_cnt	= endian->rd32(base+0x2c);
		u32 layer_off	= endian->rd32(base+0x30);
		u32 layer_size	= 0x34;
		u32 grcode_off	= endian->rd32(base+0x3c);
		u32 end_layer = layer_off + layer_cnt * layer_size;
		fprintf(f,"%sMEM #=%05x %u*l=%04x-%04x free=%03x>%02x u=%04x[%04x,s=%03x]"
			  " : s=%04x[%08x]\n",
			prefix,
			endian->rd32(base),
			layer_cnt, layer_off, end_layer,
			grcode_off - end_layer, (int)(base+grcode_off-data)&0x1f,
			grcode_off, endian->rd16(base+grcode_off),
				endian->rd32(base) - grcode_off,
			shader_off, endian->rd32(base+shader_off) );

	     #if ANA_MDL_11_8_0_VAL
		for ( ptr = base; ptr < end; ptr += 4 )
		    fprintf(f,"%sVAL0 %4x : %8x\n",
			prefix, (int)(ptr-base), endian->rd32(ptr) );
	     #endif
	    }

	    if ( item->index3 == 3 )
	    {
	     #if ANA_MDL_11_8_3_VAL
		for ( ptr = base; ptr < end; ptr += 4 )
		    fprintf(f,"%sVAL3 %4x : %8x\n",
			prefix, (int)(ptr-base), endian->rd32(ptr) );
	     #endif
	    }
	}
     #endif

     #if ANA_TEX_HD0
	if ( ana.fform == FF_TEX && item->index1 < 0 && item->index3 == 1 )
	    texhd = item;

	PRINT_IF(ana.fform == FF_TEX,"TEX %d %d %d %u\n",
		item->index1,item->index2,item->index3,sect_size);
	if ( ana.fform == FF_TEX && item->index1 == 0 && texhd )
	{
	    texhd = 0;
	}
     #endif

     #if ANA_TEX_HD
	if ( ana.fform == FF_TEX && item->index1 < 0 && item->index3 == 1 )
	{
	    tex_info_t * ti = (tex_info_t*)( ana.data + item->offset - 4 );
	    const uint wd = endian->rd16(&ti->width);
	    const uint ht = endian->rd16(&ti->height);
	    const uint iform = endian->rd32(&ti->iform);
	    fprintf(f,"#TEX-HD# %-6s %4u*%-4u",GetImageFormatName(iform,"?"),wd,ht);
	    noPRINT("OFF: %x %x %x\n",item[0].offset,item[1].offset,item[2].offset);
	    const uint raw_size = item[2].offset - item[1].offset;

	    uint xwd, xht, hb, vb, img_size;
	    const ImageGeometry_t *geo
		= CalcImageGeometry(iform,wd,ht,&xwd,&xht,&hb,&vb,&img_size);
	    if (geo)
	    {
		const uint n_blk  = img_size / geo->block_size;
		const uint m_blk  = raw_size / geo->block_size;
		const uint xbytes = raw_size - m_blk * geo->block_size;
		fprintf(f," %6u %+6d b=%5u %4u.%u:",
			img_size, raw_size - img_size,
			n_blk, m_blk - n_blk, xbytes );
	    }
	    else
		fprintf(f," %6u:",raw_size);

	    u32 * ptr = (u32*)( ana.data + item->offset );
	    u32 * end = (u32*)( ana.data + item->offset + sect_size );
	    fprintf(f," %x ..",endian->rd32(ptr)); ptr += 3;
	    while ( ptr < end )
		fprintf(f," %8x",endian->rd32(ptr++));
	    fprintf(f,"\n");
	}
     #endif
    }

    fprintf(f,"--\n");
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BREFF+BREFT sub files		///////////////
///////////////////////////////////////////////////////////////////////////////

static int ana_breff_subfile
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    if ( term || it->is_dir || memcmp(it->path,"files/",6) )
	return 0;

    noPRINT("BINGO BREFF %s\n",it->path);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int ana_breft_subfile
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    DASSERT(it->szs);
    if ( term || it->is_dir || memcmp(it->path,"files/",6) )
	return 0;
    noPRINT("BINGO BREFT %s\n",it->path);

    FILE * f = it->param;
    DASSERT(f);

    fprintf(f,"#BREFF# file=%s\n",it->path);

    u32 *d = (u32*)( it->szs->data + it->off);
    fprintf(f,"#BREFF# HEAD: %8x %8x %8x %8x  %8x %8x %8x %8x [%x]\n",
		it->endian->rd32(d),
		it->endian->rd32(d+1),
		it->endian->rd32(d+2),
		it->endian->rd32(d+3),
		it->endian->rd32(d+4),
		it->endian->rd32(d+5),
		it->endian->rd32(d+6),
		it->endian->rd32(d+7),
		it->size );

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AnalyzeBREFF()			///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyzeBREFF ( szs_file_t * szs, u8 * data, uint data_size, ccp name )
{
// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,data_size,data_size);
    switch (fform)
    {
	case FF_BREFF:
	    if ( opt_analyze_mode != ANA_MD_DEFAULT && !(opt_analyze_mode&ANA_MD_BREFF) )
		return;
	    break;

	case FF_BREFT:
	    if ( opt_analyze_mode != ANA_MD_DEFAULT && !(opt_analyze_mode&ANA_MD_BREFT) )
		return;
	    break;

	default:
	    return;
    }

    ccp ff_name = GetNameFF(0,fform);
    noPRINT("AnalyzeBREFF() %s %s\n",ff_name,szs->fname);

    FILE * f = CreateAnalyzeFile();
    if (!f)
	return;

    //printf("ANALYZE %s: %s\n",ff_name,szs->fname);
    fprintf(f,"\n\f\nANALYZE %s: %s\n\n",ff_name,szs->fname);

    brres_header_t	*bh	= (brres_header_t*)data;
    breff_root_head_t	*rh	= (breff_root_head_t*)(bh+1);
    breff_root_t	*root	= (breff_root_t*)(&rh->data);

    const endian_func_t * endian = szs->endian;
    DASSERT(endian);

    fprintf(f,"#%s#N-SECT: %u\n#%s#VERSION: %u\n",
	ff_name, endian->rd16(&bh->n_sections),
	ff_name, endian->rd16(&bh->version));

    fprintf(f,"#%s#UNKNOWN: %8x %8x %4x\n",
		ff_name,
		endian->rd32(&root->unknown1),
		endian->rd32(&root->unknown2),
		endian->rd16(&root->unknown3) );


    szs_file_t szs2;
// [[fname-]]
    InitializeSubSZS(&szs2,szs,data-szs->data,szs->size,fform,0,false);
    IterateFilesParSZS(&szs2,
		fform == FF_BREFF ? ana_breff_subfile : ana_breft_subfile,
		f, false, true, false, false, -1, SORT_NONE );
    ResetSZS(&szs2);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AnalyzeTPL()			///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyzeTPL ( szs_file_t * szs, u8 * data, uint data_size, ccp name )
{
// [[analyse-magic]]
    file_format_t fform = GetByMagicFF(data,data_size,data_size);
// [[tpl-ex+]]
    if ( !IsTplFF(fform)
		|| opt_analyze_mode != ANA_MD_DEFAULT
			&& !(opt_analyze_mode&ANA_MD_TPL) )
    {
	return;
    }

    ccp ff_name = GetNameFF(0,fform);
    noPRINT("AnalyzeTPL() %s %s / %s\n",ff_name,szs->fname,name);

    FILE * f = CreateAnalyzeFile();
    if (!f)
	return;

    //printf("ANALYZE %s: %s / %s\n",ff_name,szs->fname,name);
    fprintf(f,"\n\f\nANALYZE %s: %s / %s\n\n",ff_name,szs->fname,name);

    const endian_func_t * endian = szs->endian;
    DASSERT(endian);

    const uint n_img = GetNImagesTPL(data,data_size,endian);
    uint ni;
    for ( ni = 0; ni < n_img; ni++ )
    {
	const tpl_header_t	* head;
	const tpl_imgtab_t	* tab;
	const tpl_pal_header_t	* pal;
	const tpl_img_header_t	* img;
	const u8 *pal_data, *img_data;

	if (SetupPointerTPL( data, data_size, ni,
				&head, &tab, &pal, &img,
				&pal_data, &img_data, endian ))
	{
	    noPRINT("TPL #%u: %6x %6x %6x %6x : %6x %6x : %6x\n",
			ni,
			(int)((u8*)head-data),
			(int)((u8*)tab-data),
			pal ? (int)((u8*)pal-data) : 0,
			(int)((u8*)img-data),
			pal_data ? (int)(pal_data-data) : 0,
			(int)(img_data-data),
			data_size );
	    fprintf(f,"#TPL-IMG# %u: %6x %6x %6x %6x : %6x %6x\n",
			ni,
			(int)((u8*)head-data),
			(int)((u8*)tab-data),
			pal ? (int)((u8*)pal-data) : 0,
			(int)((u8*)img-data),
			pal_data ? (int)(pal_data-data) : 0,
			(int)(img_data-data) );
	}
	else
	{
	    PRINT("TPL #%u: FAILED\n",ni);
	    fprintf(f,"#TPL-IMG# %u: FAILED\n",ni);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AnalyzeKMP()			///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyzeKMP ( const struct kmp_t * kmp, ccp base_fname )
{
    if ( opt_analyze_mode != ANA_MD_DEFAULT
			&& !(opt_analyze_mode&(ANA_M_KMP|ANA_MKWANA)) )
    {
	return;
    }

    PRINT("AnalyzeKMP(%s) %s\n",base_fname,kmp->fname);

    FILE * f = CreateAnalyzeFile();
    if (!f)
	return;

    if ( verbose >= 0 )
	fprintf(f,"\n\f\nANALYZE KMP: %s\n\n",kmp->fname);

    if ( opt_analyze_mode == ANA_MD_DEFAULT
			|| (opt_analyze_mode&ANA_KMP_GOBJ) )
    {
	const uint n = kmp->dlist[KMP_GOBJ].used;
	const kmp_gobj_entry_t * gobj = (kmp_gobj_entry_t*)kmp->dlist[KMP_GOBJ].list;
	uint i;
	for ( i = 0; i < n; i++, gobj++ )
	{
	    ccp name = "?";
	    if ( gobj->obj_id < N_KMP_GOBJ )
	    {
		const ObjectInfo_t *oi = ObjectInfo + gobj->obj_id;
		if (oi->name)
		    name = oi->name;
	    }
	    fprintf(f,"#KMP-GOBJ#     %04x %s\n", gobj->obj_id, name );
	    const u16 *s = gobj->setting;
	    fprintf(f,
		"#KMP-GOBJ-ALL#  %4x %4x  %s  %4x %4x %4x %4x %4x %4x %4x %4x %4x  %s\n",
			gobj->obj_id, gobj->ref_id,
			gobj->route_id == 0xffff ? "-" : "R",
			s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
			gobj->pflags,
			name );

	    uint p;
	    for ( p = 0; p < 8; p++ )
		fprintf(f,"#KMP-GOBJ-SET# %04x.%u.%04x\n",
		    gobj->obj_id, p+1, gobj->setting[p] );

	    fprintf(f,"#KMP-GOBJ-SET# %04x.u.%04x\n",
		    gobj->obj_id, gobj->ref_id );
	    fprintf(f,"#KMP-GOBJ-SET# %04x.r.%04x\n",
		    gobj->obj_id, gobj->route_id );
	    fprintf(f,"#KMP-GOBJ-SET# %04x.p.%04x\n",
		    gobj->obj_id, gobj->pflags );
	}
    }

    if ( opt_analyze_mode == ANA_MD_DEFAULT
			|| (opt_analyze_mode&ANA_KMP_KTPT) )
    {
	kmp_finish_t kf;
	CheckFinishLine(kmp,&kf);
	fprintf(f,"#KTPT-DIST# %11.3f %7.2f  *%u,%u  %s : %s\n",
			kf.distance, kf.dir_delta, kf.n_ckpt_0, kf.n_ktpt_m,
			kf.warn ? "WARN" : kf.hint ? "HINT" : "-   ", base_fname );
	PRINT0("#FINISH# %s\n",LogFinishLine(&kf));
    }

    if ( opt_analyze_mode & ANA_MKWANA )
    {
	if (base_fname)
	{
	    ccp slash = strrchr(base_fname,'/');
	    if (slash)
		base_fname = slash;
	}
	else
	    base_fname = kmp->fname;
	fprintf(f,"#MKWANA: ---\n#MKWANA: FILE = %s\n",base_fname);

	uint n = kmp->dlist[KMP_KTPT].used;
	if (n>0)
	{
	    const kmp_ktpt_entry_t *ktpt = (kmp_ktpt_entry_t*)kmp->dlist[KMP_KTPT].list;
	    fprintf(f,"#MKWANA: START-POS = %11.3f %11.3f %11.3f\n",
		ktpt->position[0], ktpt->position[1], ktpt->position[2] );
	    fprintf(f,"#MKWANA: START-DIR = %11.3f %11.3f %11.3f\n",
		ktpt->rotation[0], ktpt->rotation[1], ktpt->rotation[2] );
	}

	n = kmp->dlist[KMP_STGI].used;
	if (n>0)
	{
	    const kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp->dlist[KMP_STGI].list;
	    fprintf(f,"#MKWANA: POLE-POS  = 0x%02x\n",stgi->pole_pos);
	    fprintf(f,"#MKWANA: NARROW    = 0x%02x\n",stgi->narrow_start);
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			AnalyzeKCL()			///////////////
///////////////////////////////////////////////////////////////////////////////

void AnalyzeKCL ( const struct kcl_t * kcl )
{
    if ( opt_analyze_mode != ANA_MD_DEFAULT
			&& !(opt_analyze_mode&ANA_MD_KCL) )
    {
	return;
    }

    ccp fname = kcl->fname ? kcl->fname : "?";
    PRINT("AnalyzeKCL() %s\n",fname);

    FILE * f = CreateAnalyzeFile();
    if (!f)
	return;

    ccp ptr, slash = strrchr(fname,'/');
    slash = slash ? slash+1 : fname;
    uint index = 0;
    for ( ptr = fname + strlen(fname); ptr > fname; ptr-- )
	if ( *ptr == '/' || *ptr == '\\' )
	{
	    uint num = strtoul(ptr+1,0,10);
	    if ( num > 0 )
	    {
		index = num;
		break;
	    }
	}
    fprintf(f,"\n\f\nANALYZE KCL %02u: %s\n\n",index,fname);

    u32 *count = CALLOC(N_KCL_FLAG,sizeof(*count));

    const uint n = kcl->tridata.used;
    const kcl_tridata_t *td = (kcl_tridata_t*)kcl->tridata.list;
    const kcl_tridata_t *end = td + n;
    for ( ; td < end; td++ )
	count[td->in_flag]++;

    uint i;
    for (  i = 0; i < N_KCL_FLAG; i++ )
	if (count[i])
	    fprintf(f,"KCL-TYPE 0x%02x  0x%03x  0x%04x %6.2f%%  T%02u %s\n",
		i & 0x1f, i >> 5, i & ~0x1f, 100.0 * count[i] / n, index, slash );

    FREE(count);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

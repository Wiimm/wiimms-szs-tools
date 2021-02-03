
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

#include <stddef.h>

#include "dclib-xdump.h"
#include "lib-szs.h"
#include "lib-common.h"
#include "lib-numeric.h"
#include "lib-bzip2.h"

#include "geohit.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			GEOHIT setup			///////////////
///////////////////////////////////////////////////////////////////////////////

static void verify_geohit_obj ( const geohit_t * geohit, ccp identify )
{
    if ( geohit && geohit->bin )
    {
	PRINT("verify_geohit_obj(%s) USE_OBJECT_MANAGER=%d\n",
		identify, USE_OBJECT_MANAGER );

     #if USE_OBJECT_MANAGER > 1
	VerifyObjMgr(&geohit->obj_mgr,geohit->bin,geohit->bin_size,identify);
     #elif USE_OBJECT_MANAGER
	VerifyBinObjMgr(geohit->bin,geohit->bin_size,identify);
     #endif
    }
}

///////////////////////////////////////////////////////////////////////////////

void InitializeGEOHIT ( geohit_t * geohit, file_format_t fform )
{
    DASSERT(geohit);
    memset(geohit,0,sizeof(*geohit));
    geohit->fname	 = EmptyString;
    geohit->fform_source = fform;

    InitializeObjMgr(&geohit->obj_mgr,fform);
    SetupBZ2MgrGEOHIT(geohit,fform,true);
}

///////////////////////////////////////////////////////////////////////////////

void ResetGEOHIT ( geohit_t * geohit )
{
    DASSERT(geohit);
    const typeof(geohit->fform_source) fform = geohit->fform_source;

    ResetObjMgr(&geohit->obj_mgr);
    FreeString(geohit->fname);
    FREE(geohit->bin);

    InitializeGEOHIT(geohit,fform);
}

///////////////////////////////////////////////////////////////////////////////

void SetupBZ2MgrGEOHIT
	( geohit_t * geohit, file_format_t fform, bool reload_bin )
{
    DASSERT(geohit);
    if (geohit->bin)
	ScanObjType( &geohit->obj_mgr.otype,
			geohit->bin, geohit->bin_size, geohit->bin_size );

 #if USE_OBJECT_MANAGER
    uint prio = OM_PRIO_DEFAULT;
 #endif
    if ( reload_bin || !geohit->bin )
    {
	BZ2Manager_t *bz = GetCommonBZ2Manager(fform);
	if (!bz)
	{
	    const file_format_t ff = GetObjTypeFF(&geohit->obj_mgr.otype,false);
	    bz = GetCommonBZ2Manager(ff);
	}

	if (bz)
	{
	    FREE(geohit->bin);
	    geohit->bin_size = bz->size;
	    geohit->bin = MEMDUP(bz->data,geohit->bin_size);
	 #if USE_OBJECT_MANAGER
	    prio = OM_PRIO_ORIG;
	 #endif
	}
    }

    if (geohit->bin)
    {
     #if USE_OBJECT_MANAGER
	SetupObjMgrByData( &geohit->obj_mgr, geohit->bin, geohit->bin_size, prio );
     #else
	ScanObjType( &geohit->obj_mgr.otype,
		    geohit->bin, geohit->bin_size, geohit->bin_size );
     #endif

     #if USE_OBJECT_MANAGER < 3
	geohit->n_rec  = be16(geohit->bin);
	const uint max = ( geohit->bin_size - 2 ) / geohit->obj_mgr.otype.row_size;
	if ( geohit->n_rec > max )
	     geohit->n_rec = max;
     #endif
    }

    PRINT0("GEOHIT: ff=%s,%s, nr=%03d, np=%02zd, rs=%02u, kart=%d, obj=%d\n",
		GetNameFF(0,geohit->fform_source),
		GetNameFF(0,geohit->obj_mgr.otype.fform),
 #if USE_OBJECT_MANAGER < 2
		geohit->n_rec,
 #else
		-1,
 #endif
		geohit->obj_mgr.otype.row_size/sizeof(s16) - 1,
		geohit->obj_mgr.otype.row_size,
		geohit->obj_mgr.otype.is_kart, geohit->obj_mgr.otype.is_obj );

    verify_geohit_obj(geohit,"GeoHit/setup");
}

///////////////////////////////////////////////////////////////////////////////

geohit_bin_t * GetBinGEOHIT ( cvp data, uint index )
{
    DASSERT(data);
    const uint nr    = be16(data);
    const uint rsize = (be16(data+2)+1) * sizeof(u16);
    return index < nr
	? (geohit_bin_t*)( (u8*)data + 4 + index * rsize )
	: 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawGEOHIT()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(geohit);
    if (init_geohit)
	InitializeGEOHIT(geohit,FF_GH_ITEM);

    FREE(geohit->bin);
    geohit->bin_size = data_size;
    geohit->bin = MEMDUP(data,data_size);
    SetupBZ2MgrGEOHIT(geohit,FF_UNKNOWN,false);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		ScanTextGEOHIT() helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    GEOHIT_IGNORE,
    GEOHIT_SETUP,
    GEOHIT_OBJECTS,
};

//-----------------------------------------------------------------------------

static const KeywordTab_t geohit_section_name[] =
{
	{ GEOHIT_IGNORE,	"END",		0, 0 },
	{ GEOHIT_SETUP,		"SETUP",	0, 0 },
	{ GEOHIT_OBJECTS,	"OBJECTS",	0, 0 },
	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextSETUP
(
    geohit_t		* geohit,	// GEOHIT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(geohit);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    bool setup_geohit = false;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	char name[50];
	if ( !ScanNameSI(si,name,sizeof(name),true,true,0)
		|| NextCharSI(si,true) != '=' )
	{
	    CheckEolSI(si);
	    continue;
	}
	DASSERT(si->cur_file);
	si->cur_file->ptr++;
	noPRINT("### %s = %.4s...\n",name,si->cur_file->ptr);

	if (!strcmp(name,"REVISION"))
	{
	    ScanU32SI(si,(u32*)&geohit->revision,1,0);
	    si->init_revision = si->cur_file->revision = geohit->revision;
	    DefineIntVar(&si->gvar,"REVISION$SETUP",geohit->revision);
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",geohit->revision);
	}
	else if (!strcmp(name,"IS-OBJ")) // [[obsolete]] in mid 2020
	{
	    u32 num;
	    if (!ScanU32SI(si,&num,1,0))
	    {
		setup_geohit = true;
		geohit->obj_mgr.otype.is_obj = num > 0;
	    }
	}
// [[obj-type]]
     #if USE_OBJECT_MANAGER > 1
	else if (!SetupObjMgrBySI(&geohit->obj_mgr,si,name))
	    GotoEolSI(si);
     #else
	else
	    GotoEolSI(si);
     #endif
	CheckEolSI(si);
    }

    CheckLevelSI(si);
    if (setup_geohit)
	SetupBZ2MgrGEOHIT(geohit,FF_UNKNOWN,false);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static geohit_bin_t * FindObjectGEOHIT ( const geohit_t *geohit, u16 oid )
{
    oid = htons(oid);
    geohit_bin_t *bin = GetBinGEOHIT(geohit->bin,0);

    uint i;
    for ( i = 0; i < geohit->n_rec; i++ )
    {
	if ( bin->id == oid )
	    return bin;

	bin = (geohit_bin_t*)( (u8*)bin + geohit->obj_mgr.otype.row_size );
    }
    return 0;
}

//-----------------------------------------------------------------------------

static enumError ScanTextOBJECTS
(
    geohit_t		* geohit,	// GEOHIT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(geohit);
    DASSERT(geohit->bin);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    geohit_bin_t *obj = 0;
    si->point_is_null++;
    const int n_param = geohit->obj_mgr.otype.row_size/sizeof(s16) - 1;

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,0);
	    continue;
	}

	long oid;
	ScanUValueSI(si,&oid,0);
	obj = FindObjectGEOHIT(geohit,oid);
	PRINT0("OID: %lx -> %p\n",oid,obj);
	if (!obj)
	{
	    if ( si->no_warn <= 0 )
	    {
		ccp eol = FindNextLineFeedSI(si,true);
		ScanFile_t *sf = si->cur_file;
		ERROR0(ERR_WARNING,
			"Object 0x%03lx not found: [%s @%u]: %.*s\n",
			oid, sf->name, sf->line, (int)(eol-sf->ptr), sf->ptr );
	    }
	    GotoEolSI(si);
	    continue;
	}

	int i;
	for ( i = 0; i < n_param; i++ )
	{
	    const char ch = NextCharSI(si,false);
	    if (!ch)
		break;
	    if ( ch == '=' )
	    {
		si->cur_file->ptr++;
		SkipCharSI(si,',');
	    }
	    else
	    {
		long num;
		ScanUValueSI(si,&num,0);
		obj->param[i] = htons(num);
	    }
	}
	CheckEolSI(si);
    }

    si->point_is_null--;
    CheckLevelSI(si);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextGEOHIT()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    file_format_t	fform		// file format
)
{
    DASSERT(geohit);
    PRINT("ScanTextGEOHIT(init=%d)\n",init_geohit);

    if ( fform == FF_UNKNOWN )
	fform = GetByMagicFF(data,data_size,data_size);
    if (init_geohit)
	InitializeGEOHIT(geohit,fform);
    else
    {
	geohit->fform_source = fform;
	SetupBZ2MgrGEOHIT(geohit,fform,true);
    }


    ScanInfo_t si;
    InitializeSI(&si,data,data_size,geohit->fname,geohit->revision);
    si.predef = SetupVarsCOMMON();

    enumError max_err = ERR_OK;
    geohit->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN GEOHIT SECTIONS, PASS%u ...\n",geohit->is_pass2+1);

	max_err = ERR_OK;
	si.no_warn = !geohit->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", geohit->is_pass2+1 );

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
	    ResetLocalVarsSI(&si,geohit->revision);

	    si.cur_file->ptr++;
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    noPRINT("--> pass=%u: #%04u: %s\n",geohit->is_pass2+1,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,geohit_section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    noPRINT("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case GEOHIT_SETUP:
		    err = ScanTextSETUP(geohit,&si);
		    break;

		case GEOHIT_OBJECTS:
		    err = ScanTextOBJECTS(geohit,&si);
		    break;

		default:
		    // ignore all other section without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (geohit->is_pass2)
	    break;

	geohit->is_pass2 = true;

	RestartSI(&si);
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextGEOHIT) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			GEOHIT: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(geohit);

    enumError err;
// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);
    switch (fform)
    {
	case FF_GH_ITEM:
	case FF_GH_IOBJ:
	case FF_GH_KART:
	case FF_GH_KOBJ:
	    err = ScanRawGEOHIT(geohit,init_geohit,data,data_size);
	    break;

	case FF_GH_ITEM_TXT:
	case FF_GH_IOBJ_TXT:
	case FF_GH_KART_TXT:
	case FF_GH_KOBJ_TXT:
	    err =  ScanTextGEOHIT(geohit,init_geohit,data,data_size,fform);
	    break;

	default:
	    if (init_geohit)
		InitializeGEOHIT(geohit,0);
	    return ERROR0(ERR_INVALID_DATA,
		"No GEOHIT file: %s\n", geohit->fname ? geohit->fname : "?");
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadGEOHIT
(
    geohit_t		* geohit,	// GEOHIT data structure
    bool		init_geohit,	// true: initialize 'geohit' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(geohit);
    DASSERT(fname);
    if (init_geohit)
	InitializeGEOHIT(geohit,0);
    else
	ResetGEOHIT(geohit);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	geohit->fname = raw.fname;
	raw.fname = 0;
	err = ScanGEOHIT(geohit,false,raw.data,raw.data_size);
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawGEOHIT()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawGEOHIT
(
    geohit_t		* geohit,	// pointer to valid GEOHIT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(geohit);
    DASSERT(geohit->bin);
    DASSERT(fname);

    PRINT("SaveRawGEOHIT(%s,%d)\n",fname,set_time);
    verify_geohit_obj(geohit,"GeoHit/save-raw");


    //--- write to file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&geohit->fatt,0);

    if ( fwrite(geohit->bin,1,geohit->bin_size,F.f) != geohit->bin_size )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  SaveTextGEOHIT()		///////////////
///////////////////////////////////////////////////////////////////////////////

void PrintParamGEOHIT ( FILE *f, uint fw, cvp param0, cvp cmp0, int n )
{
    const u16 *param = param0;
    const u16 *cmp   = cmp0;

    int extra = 4;
    while ( n-- > 0 )
    {
	if (!extra--)
	{
	    extra = 3;
	    fputc(' ',f);
	}

	const u16 p = ntohs(*param++);
	if ( cmp && ntohs(*cmp++) == p )
	    fprintf(f," %*c",fw,'=');
	else if (p)
	    fprintf(f," %*d",fw,(s16)p);
	else
	    fprintf(f," %*c",fw,'.');
    }
}

//-----------------------------------------------------------------------------

enumError SaveTextGEOHIT
(
    const geohit_t	* geohit,	// pointer to valid GEOHIT
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
// [[minimize]]
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >9: minimize output
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(geohit);
    DASSERT(geohit->bin);
    DASSERT(fname);
    PRINT("SaveTextGEOHIT(%s,%d)\n",fname,set_time);
    verify_geohit_obj(geohit,"GeoHit/save-text");


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,geohit->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&geohit->fatt,0);


    //--- print header

    const bool explain		= print_header && !brief_count && !export_count;
    const bool tiny		= brief_count > 1 || export_count;
    const file_format_t ff_text	= GetObjTypeFF(&geohit->obj_mgr.otype,true);
    ccp magic			= GetMagicStringFF(ff_text);
    BZ2Manager_t *bz_orig	= GetCommonBZ2Manager(ff_text);

    if (geohit->obj_mgr.otype.is_kart)
    {
	if (explain)
	    fprintf(F.f,text_geohit_kart_info_cr,magic);
	else
	    fprintf(F.f,text_geohit_kart_head_cr,magic);

	if (!tiny)
	    fprintf(F.f,text_geohit_kart_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );

     #if USE_OBJECT_MANAGER > 1
	WriteObjMgrToSetupSection( F.f, &geohit->obj_mgr,
		long_count>0, tiny, explain );
     #endif
    }
    else
    {
	if (explain)
	    fprintf(F.f,text_geohit_item_info_cr,magic);
	else
	    fprintf(F.f,text_geohit_item_head_cr,magic);

	if (!tiny)
	    fprintf(F.f,text_geohit_item_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );

     #if USE_OBJECT_MANAGER > 1
	WriteObjMgrToSetupSection( F.f, &geohit->obj_mgr,
		long_count>0, tiny, explain );
     #endif
    }


    //--- iterate records

    const int row_size	= geohit->obj_mgr.otype.row_size;
    const int n_param	= row_size/sizeof(s16) - 1;
    const int fw	= n_param < 8 ? 5 : 3;

    const geohit_bin_t *bin = GetBinGEOHIT(geohit->bin,0);
    const geohit_bin_t *ref = bz_orig ? GetBinGEOHIT(bz_orig->data,0) : 0;
    if (!ref)
	minimize = 0;

    bool sect_head_done = false;
    int i, last_grp = -1;
    for ( i = 0; i < geohit->n_rec; i++ )
    {
	const int differ = ref && memcmp( bin, ref, row_size );

	if ( differ || !minimize )
	{
	    if (!sect_head_done)
	    {
		sect_head_done = true;
		fputs( brief_count < 2 ? section_sep : "\r\n", F.f );
		if (geohit->obj_mgr.otype.is_kart)
		    fprintf(F.f,text_geohit_kart_objects_cr,REVISION_NUM);
		else
		    fprintf(F.f,text_geohit_item_objects_cr,REVISION_NUM);
	    }

	    const uint obj_id = ntohs(bin->id);
	    const uint grp = obj_id >> 4;
	    if ( last_grp != grp )
	    {
		if ( last_grp != -1 )
		    fputs("\r\n",F.f);
		last_grp = grp;
	    }

	    fprintf(F.f,"\r\n 0x%03x ",obj_id);
	    PrintParamGEOHIT(F.f,fw,bin->param,0,n_param);
	    if ( obj_id < N_KMP_GOBJ )
		fprintf(F.f,"  # %s",ObjectInfo[obj_id].name);

	    if (differ)
	    {
		fputs("\r\n #ORIG:",F.f);
		PrintParamGEOHIT(F.f,fw,ref->param,bin->param,n_param);
//DEL		HexDump16(stderr,0,0,bin,row_size);
//DEL		HexDump16(stderr,0,0,ref,row_size);
	    }
	}

	bin = (geohit_bin_t*)( (u8*)bin + row_size );
	if (ref)
	    ref = (geohit_bin_t*)( (u8*)ref + row_size );
    }


    //--- terminate

    fputs("\r\n",F.f);
    fputs(  brief_count < 2 ? section_end :"\r\n", F.f );
    ResetFile(&F,set_time);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


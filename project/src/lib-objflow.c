
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
 *   Copyright (c) 2011-2023 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include "lib-szs.h"
#include "lib-common.h"
#include "lib-numeric.h"
#include "lib-bzip2.h"

#include "objflow.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			OBJFLOW setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeOBJFLOW ( objflow_t * objflow )
{
    DASSERT(objflow);
    memset(objflow,0,sizeof(*objflow));
    objflow->fname = EmptyString;

    InitializeObjMgr(&objflow->obj_mgr,FF_OBJFLOW);
    BZ2Manager_t *bz_orig = GetCommonBZ2Manager(FF_OBJFLOW);
    DASSERT(bz_orig);
    const uint copy_size = bz_orig->size < sizeof(objflow->bin)
			 ? bz_orig->size : sizeof(objflow->bin);
    memcpy( objflow->bin, bz_orig->data, copy_size );
}

///////////////////////////////////////////////////////////////////////////////

void ResetOBJFLOW ( objflow_t * objflow )
{
    DASSERT(objflow);
    ResetObjMgr(&objflow->obj_mgr);
    FreeString(objflow->fname);
    InitializeOBJFLOW(objflow);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawOBJFLOW()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(objflow);
    if (init_objflow)
	InitializeOBJFLOW(objflow);

 #if 0 // [[todo]] [[?]] ???
    if ( IsValidOBJFLOW(data,data_size,data_size,objflow->fname) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid OBJFLOW file: %s\n"
		"Add option --objflow=force or --force to ignore some validity checks.",
		objflow->fname ? objflow->fname : "?" );
    }
 #endif

    const uint copy_size = data_size < sizeof(objflow->bin)
			 ? data_size : sizeof(objflow->bin);
    memcpy(objflow->bin,data,copy_size);
    objflow->fform = FF_OBJFLOW;

// [[obj-type]]
 #if USE_OBJECT_MANAGER
    VerifyBinObjMgr(objflow->bin,sizeof(objflow->bin),"ObjFlow/raw");
 #endif
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		ScanTextOBJFLOW() helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    OBJFLOW_IGNORE,
    OBJFLOW_SETUP,
    OBJFLOW_OBJECTS,
};

//-----------------------------------------------------------------------------

static const KeywordTab_t objflow_section_name[] =
{
	{ OBJFLOW_IGNORE,	"END",		0, 0 },
	{ OBJFLOW_SETUP,	"SETUP",	0, 0 },
	{ OBJFLOW_OBJECTS,	"OBJECTS",	0, 0 },
	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTextSETUP
(
    objflow_t		* objflow,	// OBJFLOW data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(objflow);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

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
	    ScanU32SI(si,(u32*)&objflow->revision,1,0);
	    si->init_revision = si->cur_file->revision = objflow->revision;
	    DefineIntVar(&si->gvar,"REVISION$SETUP",objflow->revision);
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",objflow->revision);
	}
// [[obj-type]]
     #if USE_OBJECT_MANAGER > 1
	else if (!SetupObjMgrBySI(&objflow->obj_mgr,si,name))
	    GotoEolSI(si);
     #else
	else
	    GotoEolSI(si);
     #endif
	CheckEolSI(si);
    }

    CheckLevelSI(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static objflow_bin_t * FindObjectOBJFLOW ( cvp data, uint oid )
{
    oid = htons(oid);
    const uint N = be16(data);
    objflow_bin_t *bin = (objflow_bin_t*)((u8*)data+2);
    uint i;
    for ( i = 0; i < N; i++, bin++ )
	if ( bin->id == oid )
	    return bin;
    return 0;
}

//-----------------------------------------------------------------------------

static enumError ScanTextOBJECTS
(
    objflow_t		* objflow,	// OBJFLOW data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(objflow);
    DASSERT(si);
    TRACE("ScanTextSETUP()\n");

    objflow_bin_t *obj = 0;
    si->point_is_null++;

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

	char name[50];
	const uint namelen = ScanNameSI(si,name,sizeof(name),true,true,0);

	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	noPRINT("### %s = %.4s...\n",name,sf->ptr);

	if (!namelen)
	{
	    if ( si->no_warn <= 0 )
	    {
		ccp eol = FindNextLineFeedSI(si,true);
		ERROR0(ERR_WARNING,
			    "Missing name for option: [%s @%u]: %.*s\n",
			    sf->name, sf->line, (int)(eol-sf->ptr), sf->ptr );
	    }
	    continue;
	}

	if (!strcmp(name,"OBJECT"))
	{
	    long oid;
	    ScanUValueSI(si,&oid,0);
	    obj = FindObjectOBJFLOW(objflow->bin,oid);
	    PRINT0("OID: %lx -> %p\n",oid,obj);
	    if (!obj)
		GotoEolSI(si);
	    else
	    {
		DEFINE_VAR(var);
		if (SkipCharSI(si,','))
		{
		    ScanStringSI(si,&var);
		    memset(obj->name,0,sizeof(obj->name));
		    StringCopyS(obj->name,sizeof(obj->name),var.str);
		}

		if (SkipCharSI(si,','))
		{
		    ScanStringSI(si,&var);
		    memset(obj->resources,0,sizeof(obj->resources));
		    StringCopyS(obj->resources,sizeof(obj->resources),var.str);
		}
		FreeV(&var);
	    }
	}
	else if (!strcmp(name,"PARAM"))
	{
	    if (!obj)
	    {
		sf->line_err++;
		si->total_err++;
		if ( si->no_warn <= 0 )
		    ERROR0(ERR_WARNING,
			"No object selected [%s @%u]: %s\n",
			sf->name, sf->line, name );
		GotoEolSI(si);
	    }
	    else
	    {
		uint i;
		const uint N = sizeof(obj->param) / sizeof(*obj->param);
		for ( i = 0; i < N; i++ )
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
	    }
	}
	else
	{
	    sf->line_err++;
	    si->total_err++;
	    if ( si->no_warn <= 0 )
		ERROR0(ERR_WARNING,
			"Unknown keyword [%s @%u]: %s\n",
			sf->name, sf->line, name );
	    GotoEolSI(si);
	}
	CheckEolSI(si);
    }

    si->point_is_null--;
    CheckLevelSI(si);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextOBJFLOW()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    PRINT("ScanTextOBJFLOW(init=%d)\n",init_objflow);

    DASSERT(objflow);
    if (init_objflow)
	InitializeOBJFLOW(objflow);

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,objflow->fname,objflow->revision);
    si.predef = SetupVarsCOMMON();

    enumError max_err = ERR_OK;
    objflow->is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN OBJFLOW SECTIONS, PASS%u ...\n",objflow->is_pass2+1);

	max_err = ERR_OK;
	si.no_warn = !objflow->is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", objflow->is_pass2+1 );

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
	    ResetLocalVarsSI(&si,objflow->revision);

	    si.cur_file->ptr++;
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    noPRINT("--> pass=%u: #%04u: %s\n",objflow->is_pass2+1,si.cur_file->line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,objflow_section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);
	    noPRINT("--> %-6s #%-4u |%.3s|\n",cmd->name1,si.cur_file->line,si.cur_file->ptr);

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case OBJFLOW_SETUP:
		    err = ScanTextSETUP(objflow,&si);
		    break;

		case OBJFLOW_OBJECTS:
		    err = ScanTextOBJECTS(objflow,&si);
		    break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (objflow->is_pass2)
	    break;

	objflow->is_pass2 = true;

	RestartSI(&si);
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextOBJFLOW) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			OBJFLOW: scan and load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(objflow);

    enumError err;
// [[analyse-magic]]
// [[obj-type]]
    switch (GetByMagicFF(data,data_size,data_size))
    {
	case FF_OBJFLOW:
	    objflow->fform = FF_OBJFLOW;
	    err = ScanRawOBJFLOW(objflow,init_objflow,data,data_size);
	    break;

	case FF_OBJFLOW_TXT:
	    objflow->fform = FF_OBJFLOW_TXT;
	    err =  ScanTextOBJFLOW(objflow,init_objflow,data,data_size);
	    break;

	default:
	    if (init_objflow)
		InitializeOBJFLOW(objflow);
	    return ERROR0(ERR_INVALID_DATA,
		"No OBJFLOW file: %s\n", objflow->fname ? objflow->fname : "?");
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadOBJFLOW
(
    objflow_t		* objflow,	// OBJFLOW data structure
    bool		init_objflow,	// true: initialize 'objflow' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(objflow);
    DASSERT(fname);
    if (init_objflow)
	InitializeOBJFLOW(objflow);
    else
	ResetOBJFLOW(objflow);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	objflow->fname = raw.fname;
	raw.fname = 0;
	err = ScanOBJFLOW(objflow,false,raw.data,raw.data_size);
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawOBJFLOW()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawOBJFLOW
(
    objflow_t		* objflow,	// pointer to valid OBJFLOW
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(objflow);
    DASSERT(fname);

    PRINT("SaveRawOBJFLOW(%s,%d)\n",fname,set_time);


    //--- write to file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&objflow->fatt,0);

    if ( fwrite(objflow->bin,1,sizeof(objflow->bin),F.f) != sizeof(objflow->bin) )
	FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  SaveTextOBJFLOW()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveTextOBJFLOW
(
    const objflow_t	* objflow,	// pointer to valid OBJFLOW
    ccp			fname,		// filename of destination
    bool		set_time,	// true: set time stamps
// [[minimize]]
    int			minimize	// usually a copy of global 'minimize_level':
					//   >0: write only modified sections
					//   >9: minimize output
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(objflow);
    DASSERT(fname);
    PRINT("SaveTextOBJFLOW(%s,%d)\n",fname,set_time);


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,objflow->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&objflow->fatt,0);


    //--- print header + syntax info

    const bool explain = print_header && !brief_count && !export_count;
    if (explain)
	fputs(text_objflow_info_cr,F.f);
    else
	fputs(text_objflow_head_cr,F.f);

    const bool tiny = brief_count > 1 || export_count;
    if (!tiny)
	fprintf(F.f,text_objflow_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE );

 #if USE_OBJECT_MANAGER>1
    WriteObjMgrToSetupSection( F.f, &objflow->obj_mgr,
		long_count>0, tiny, explain );
 #endif


    //--- iterate records

    BZ2Manager_t *bz_orig = GetCommonBZ2Manager(FF_OBJFLOW);
    DASSERT(bz_orig);
    uint n_rec = be16(objflow->bin);
    if ( n_rec > be16(bz_orig->data) )
	 n_rec = be16(bz_orig->data);

    uint i;
    const objflow_bin_t *bin = (objflow_bin_t*)(objflow->bin + 2);
    const objflow_bin_t *ref = (objflow_bin_t*)(bz_orig->data + 2);

    bool sect_head_done = false;
    for ( i = 0; i < n_rec; i++, bin++, ref++ )
    {
	const int differ = memcmp( bin, ref, sizeof(*bin) );
	if ( differ || !minimize )
	{
	    if (!sect_head_done)
	    {
		sect_head_done = true;
		fputs( brief_count < 2 ? section_sep : "\r\n", F.f );
		fprintf(F.f,text_objflow_objects_cr,REVISION_NUM);
	    }

	    fprintf(F.f,"\r\nOBJECT 0x%03x, \"%s\", \"%s\"\r\n",
			ntohs(bin->id), bin->name, bin->resources );

	    if (memcmp(bin,ref,offsetof(objflow_bin_t,param)))
		fprintf(F.f,"#ORIG: 0x%03x \"%s\" \"%s\"\r\n",
			ntohs(ref->id), ref->name, ref->resources );

	    fputs("PARAM ",F.f);
	    PrintParamGEOHIT(F.f,6,bin->param,0,9);

	    if (memcmp(bin->param,ref->param,sizeof(bin->param)))
	    {
		fputs("\r\n#ORIG:",F.f);
		PrintParamGEOHIT(F.f,6,ref->param,bin->param,9);
	    }
	    fputs("\r\n",F.f);
	}
    }

    //--- terminate

    fputs(  brief_count < 2 ? section_end :"\r\n", F.f );
    ResetFile(&F,set_time);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


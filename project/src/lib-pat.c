
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
#include "lib-pat.h"

#include "pat.inc"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  PAT action log		///////////////
///////////////////////////////////////////////////////////////////////////////

static bool PAT_ACTION_LOG ( const char * format, ...)
	__attribute__ ((__format__(__printf__,1,2)));

static bool PAT_ACTION_LOG ( const char * format, ...)
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[PAT]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

    if ( verbose > 2 || PAT_MODE & PATMD_LOG )
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[PAT]%s ",colset->heading,colset->info);
	va_list arg;
	va_start(arg,format);
	vfprintf(stdlog,format,arg);
	va_end(arg);
	fputs(colset->reset,stdlog);
	fflush(stdlog);
	return true;
    }

    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    data			///////////////
///////////////////////////////////////////////////////////////////////////////

enum pat_entry_t
{
    PAT_SECT_SETUP,
    PAT_SECT_SIMPLE,
    PAT_SECT_HEADER,
    PAT_SECT_S0_BASE,
    PAT_SECT_S0_LIST,
    PAT_SECT_STRINGS,

    PAT_N_SECT	// number of PAT sections
};

const KeywordTab_t pat_section_name[] =
{
    { PAT_SECT_SETUP,	"SETUP",	0, 0 },
    { PAT_SECT_SIMPLE,	"SIMPLE",	0, 0 },
    { PAT_SECT_HEADER,	"HEADER",	0, 0 },
    { PAT_SECT_S0_BASE,	"S0_BASE",	0, 0 },
    { PAT_SECT_S0_LIST,	"S0_LIST",	0, 0 },
    { PAT_SECT_STRINGS, "STRINGS",	0, 0 },
    { 0,0,0,0 }
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 pat_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

pat_mode_t PAT_MODE = PATMD_M_DEFAULT;
int have_pat_patch_count = 0;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_pat_tab[] =
{
  { 0,			"CLEAR",	"RESET",	PATMD_M_ALL | PATMD_F_HIDE },
  { PATMD_M_DEFAULT,	"DEFAULT",	0,		PATMD_M_ALL | PATMD_F_HIDE },

  { PATMD_LOG,		"LOG",		0,		0 },
  { PATMD_SILENT,	"SILENT",	0,		0 },

  { 0,			"AUTO",		0,		PATMD_M_BOTH | PATMD_F_HIDE },
  { PATMD_M_BOTH,	"BOTH",		0,		PATMD_M_BOTH },
  { PATMD_SIMPLE,	"SIMPLE",	0,		PATMD_M_BOTH },
  { PATMD_COMPLEX,	"COMPLEX",	0,		PATMD_M_BOTH },

  { PATMD_TEST0,	"TEST0",	"T0",		PATMD_M_TEST | PATMD_F_HIDE },
  { PATMD_TEST1,	"TEST1",	"T1",		PATMD_M_TEST },
  { PATMD_TEST2,	"TEST2",	"T2",		PATMD_M_TEST },
  { PATMD_TEST3,	"TEST3",	"T3",		PATMD_M_TEST },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

void LoadParametersPAT
(
    ccp		log_prefix		// not NULL:
					//    print log message with prefix
)
{
    static bool done = false;
    if (done)
	return;
    done = true;

    SetPatMode(PAT_MODE);


    //--- logging

    if ( !PAT_ACTION_LOG("Global PAT Modes: %s\n",GetPatMode())
	&& log_prefix
	&& PAT_MODE != PATMD_M_DEFAULT
	&& !(PAT_MODE & PATMD_SILENT) )
    {
	fprintf(stdlog,"%sglobal pat modes: %s [%x]\n",log_prefix,GetPatMode(),PAT_MODE);
	fflush(stdlog);
    }
}

///////////////////////////////////////////////////////////////////////////////

pat_mode_t GetRelevantPatMode ( pat_mode_t mode )
{
    mode = mode & PATMD_M_ALL;

    if ( PATCH_FILE_MODE & PFILE_F_LOG_ALL )
	mode |= PATMD_LOG;

    return mode;
}

///////////////////////////////////////////////////////////////////////////////

void SetPatMode ( uint new_mode )
{
    noPRINT("SET-PAT/IN:  PC=%d, MPC=%d\n",
		have_patch_count, have_pat_patch_count );

//    if (have_pat_patch_count)
//	have_patch_count--, have_pat_patch_count--;

    PAT_MODE = GetRelevantPatMode(new_mode);

//    if ( PAT_MODE & PATMD_M_PATCH )
//	have_patch_count++, have_pat_patch_count++;

    noPRINT("SET-PAT/OUT: PC=%d, MPC=%d\n",
		have_patch_count, have_pat_patch_count );
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPat ( ccp arg )
{
    if (!arg)
	return 0;

    static bool done = false;
    if (!done)
    {
	done = true;
	if ( *arg != '+' && *arg != '-' )
	    PAT_MODE = 0;
    }

    s64 stat = ScanKeywordList(arg,opt_pat_tab,0,true,0,PAT_MODE,
				"Option --pat",ERR_SYNTAX);
    if ( stat != -1 )
    {
	SetPatMode(stat);
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

uint PrintPatMode ( char *buf, uint bufsize, pat_mode_t mode )
{
    DASSERT(buf);
    DASSERT(bufsize>10);
    char *dest = buf;
    char *end = buf + bufsize - 1;

    mode = GetRelevantPatMode(mode) | PATMD_F_HIDE;
    pat_mode_t mode1 = mode;

    const KeywordTab_t *ct;
    for ( ct = opt_pat_tab; ct->name1 && dest < end; ct++ )
    {
	if ( ct->opt & PATMD_F_HIDE )
	    continue;

	if ( ct->opt ? (mode & ct->opt) == ct->id : mode & ct->id )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = StringCopyE(dest,end,ct->name1);
	    mode &= ~(ct->id|ct->opt);
	}
    }

    if ( mode1 == (PATMD_M_DEFAULT|PATMD_F_HIDE) )
	dest = StringCopyE(dest,end," (default)");
    else if (!mode1)
	dest = StringCopyE(dest,end,"(none)");

    *dest = 0;
    return dest-buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetPatMode()
{
    static char buf[100] = {0};
    if (!*buf)
	PrintPatMode(buf,sizeof(buf),PAT_MODE);
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PAT0 iterator			///////////////
///////////////////////////////////////////////////////////////////////////////

static int pat_iter_func
(
    brsub_cut_t		*bcut,		// pointer to data structure
    int			grp,		// index of group, <0: grp_entry_t
    int			entry		// index of entry, -1: no entry
)
{
    DASSERT(bcut);
    szs_iterator_t *it = bcut->it;
    DASSERT(it);
    DASSERT(it->func_it);

    it->group = grp;
    it->entry = entry;

    noPRINT("PAT( %2d, %2d, %s )\n",grp,entry,it->path);

    switch (grp)
    {
      case GROUP_IDX_SUB_HEADER:
	{
	    const pat_head_t *head = (pat_head_t*)( bcut->data + it->off );
	    it->size = sizeof(*head);
	    it->entry = 0;
	    const int stat = it->func_it(it,false);
	    it->index++;
	    return stat;
	}
    }

    const int stat = it->func_it(it,false);
    it->index++;
    return stat;
}

//-----------------------------------------------------------------------------

static ccp pat_names[] =
{
	".base",		//  0
	".name",		//  1
	"",			//  2
	".attrib",		//  3
	0
};

static ccp GetSectNamePAT ( uint index )
{
    ccp res = index < sizeof(pat_names)/sizeof(*pat_names)
		? pat_names[index]
		: 0;
    if ( !res || !*res )
    {
	const int bufsize = 12;
	char *buf = GetCircBuf(bufsize);
	snprintf(buf,bufsize,".%u",index);
	res = buf;
    }
    return res;
}

//-----------------------------------------------------------------------------

int IterateFilesPAT
(
    struct szs_iterator_t	*it,	// iterator struct with all infos
    bool			term	// true: termination hint
)
{
    DASSERT(it);
    return CutFilesBRSUB( it, pat_iter_func, 0,
		pat_names, sizeof(pat_names) / sizeof(*pat_names) -1 );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PAT0 base interface		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializePAT ( pat_t * pat )
{
    DASSERT(pat);
    memset(pat,0,sizeof(*pat));

    pat->version  = 4;
    pat->n_sect   = 6;
    pat->revision = REVISION_NUM;

    pat->fname = EmptyString;
    InitializeParamField(&pat->s1_str);

    LoadParametersPAT( verbose > 0 ? "  - " : 0 );
}

///////////////////////////////////////////////////////////////////////////////

static void ClearElementsPAT ( pat_t * pat )
{
    DASSERT(pat);

    uint i;
    pat_element_t *pe = pat->pelem;
    for ( i = 0; i < pat->n_pelem; i++, pe++ )
    {
	FREE(pe->selem);
	FreeString(pe->bname);
	FreeString(pe->sname);
	memset(pe,0,sizeof(*pe));
    }
    pat->n_pelem = 0;
}

///////////////////////////////////////////////////////////////////////////////

static void ClearPAT ( pat_t * pat )
{
    DASSERT(pat);

    ClearElementsPAT(pat);
    if ( pat->outmode != PATMD_SIMPLE )
	ResetParamField(&pat->s1_str);

    FREE(pat->raw_data);
    pat->raw_data = 0;
    pat->raw_data_size = 0;
}

///////////////////////////////////////////////////////////////////////////////

void ResetPAT ( pat_t * pat )
{
    DASSERT(pat);

    ClearPAT(pat);
    ResetParamField(&pat->s1_str);
    FreeString(pat->fname);
    FreeString(pat->name);
 #if USE_NEW_CONTAINER_PAT
    ResetContainer(&pat->container);
 #else
    FreeContainer(pat->old_container);
 #endif
    InitializePAT(pat);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool IsSimplePAT ( pat_t *pat )
{
    DASSERT(pat);
    const pat_element_t *pe = pat->pelem;

    if
    (
	pat->n_pelem != 1

	|| pat->head.unknown_00
	|| pat->head.unknown_02
	|| pat->head.unknown_0a
	|| pat->head.unknown_0c

	|| pat->base.unknown_04
	|| pat->base.unknown_08	!= htons(0xffff)
	|| pat->base.unknown_0a
	|| pat->base.n_unknown	!= htons(1)
	|| pat->base.unknown_0e
	|| pat->base.unknown_10
	|| pat->base.unknown_12
	|| pat->base.unknown_14
	|| pat->base.unknown_16

	|| pe->belem.unknown_02
	|| pe->belem.unknown_04
	|| pe->belem.unknown_06	!= htons(1)

	|| pe->sref.unknown_04
	|| pe->sref.type	!= htons(5)

	|| pe->shead.unknown_02

	|| !pe->bname
	|| !pe->sname
	|| strcmp(pe->bname,pe->sname)
    )
    return false;

    uint i;
    ParamFieldItem_t *it = pat->s1_str.field;
    for ( i = 0; i < pat->s1_str.used; i++, it++ )
	if (it->num)
	    return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

static ccp GetNamePAT ( pat_t *pat )
{
    DASSERT(pat);
    if (!pat->name)
    {
	if (pat->fname)
	{
	    ccp begin = strrchr(pat->fname,'/');
	    begin = begin ? begin + 1 : pat->fname;
	    ccp end = strchr(begin,'.');
	    const uint len = end ? end-begin : strlen(begin);
	    pat->name = MEMDUP(begin,len);
	}
	else
	    pat->name = STRDUP("pat");
    }
    return pat->name;
}

///////////////////////////////////////////////////////////////////////////////

static ParamFieldItem_t * AddS1String ( pat_t *pat, ccp string, u32 num )
{
    DASSERT(pat);
    DASSERT(string);

    ParamFieldItem_t *it = AppendParamField(&pat->s1_str,string,false,num,0);
    DASSERT(it);
    return it;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetStringPAT
(
    pat_t		* pat,		// PAT data structure
    const void		* base,		// base for offset
    u32			offset,		// offset of string
    ccp			if_invalid	// result if invalid
)
{
    DASSERT(pat);
    DASSERT(base);

    if (offset)
    {
	ccp string = (ccp)base + offset;
 #if USE_NEW_CONTAINER_PAT
	if (InContainerP(&pat->container,string))
 #else
	if (InDataContainer(pat->old_container,string))
 #endif
	    return string;
    }

    return if_invalid;
}


///////////////////////////////////////////////////////////////////////////////

static float CalcFactorSHEAD
(
    pat_s0_selem_t *se,		// element list, big endian
    uint	    n_se	// number of elements in 'se'
)
{
    if ( !se || n_se < 2 )
	return 1.0;

    double min, max;
    min = max = bef4(&se->time);
    while ( --n_se > 0 )
    {
	se++;
	const double time = bef4(&se->time);
	if ( min > time ) min = time;
	if ( max < time ) max = time;
    }

    // special rounding to get little smaller values like Nintendo
    const float delta = max - min;
    return delta < 1e-6 ? 1.0 : 0.9999999/delta;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct slist_param_t
{
    float		factor;		// factor for list header
    u16			unknown_02;	// value for list header padding
}
slist_param_t;

//-----------------------------------------------------------------------------

typedef struct slist_list_t
{
    float		factor;		// factor for list header
    u16			unknown_02;	// value for list header padding

    pat_s0_selem_t	*selem;		// list with string elements
    uint		selem_used;	// number of used elements of 'selem'
    uint		selem_size;	// number of alloced elements of 'selem'
}
slist_list_t;

//-----------------------------------------------------------------------------

static void Close_SList
(
    slist_param_t	*param,		// parameter, never NULL
    slist_list_t	*list		// NULL or list element
)
{
    DASSERT(param);

    if (list)
    {
	list->unknown_02 = param->unknown_02;
	list->factor = IsNormalF(param->factor)
			? param->factor
			: CalcFactorSHEAD(list->selem,list->selem_used);
    }

    //--- reset param

    param->factor = NAN;
    param->unknown_02 = 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CreateRawPAT()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateRawPAT
(
    pat_t		* pat		// pointer to valid PAT
)
{
    DASSERT(pat);
    TRACE("CreateRawPAT()\n");

    //--- setup string pool

    string_pool_t sp;
    InitializeStringPool(&sp);
    InsertStringPool(&sp,GetNamePAT(pat),false,0);

    uint i;
    for ( i = 0; i < pat->s1_str.used; i++ )
	InsertStringPool(&sp,pat->s1_str.field[i].key,false,0);


    //--- calculate sizes

    const uint s0_offset = PAT_HEAD_OFFSET + sizeof(pat_head_t);
    uint s0_size = sizeof(pat_s0_bhead_t)
		 + pat->n_pelem
			* ( sizeof(pat_s0_belem_t)
			  + sizeof(pat_s0_sref_t)
			  + sizeof(pat_s0_shead_t) );

    pat_element_t *pe = pat->pelem;
    for ( i = 0; i < pat->n_pelem; i++, pe++ )
    {
	s0_size += pe->selem_used * sizeof(pat_s0_selem_t);
	InsertStringPool(&sp,pe->bname,false,0);
	InsertStringPool(&sp,pe->sname,false,0);
    }

    const uint s1_size = sizeof(u32) * pat->s1_str.used;
    const uint pat_size = s0_offset + s0_size + 2 * s1_size;

    CalcStringPool(&sp,pat_size,&be_func);
    uint total_size = pat_size + sp.size;

    PRINT("\e[31;1m SIZE: s0=%x, s1=%x, pat=%x, tot=%x\e[0m\n",
	s0_size, s1_size, pat_size, total_size );


    //--- setup data

    FREE(pat->raw_data);
    pat->raw_data = CALLOC(1,total_size);
    pat->raw_data_size = total_size;

    memcpy( pat->raw_data + pat_size, sp.data, sp.size );


    //--- setup brres sub header

    brsub_header_t *brh = (brsub_header_t*)pat->raw_data;
    memcpy(brh->magic,PAT_MAGIC,sizeof(brh->magic));

    brh->size	= htonl(pat_size);
    brh->version	= htonl(pat->version);
    brh->grp_offset[0] = htonl(s0_offset);

    const u32 s1_offset = s0_offset + s0_size;
    brh->grp_offset[1] = htonl(s1_offset);

    const u32 s3_offset = s1_offset + s1_size;
    brh->grp_offset[2] = brh->grp_offset[3] = htonl(s3_offset);

    brh->grp_offset[4] = htonl(s3_offset+s1_size);
    brh->grp_offset[6] = htonl(FindStringPool(&sp,GetNamePAT(pat),0));


    //--- setup header data

    pat_head_t *ph = &pat->head;
    ph->n_sect0 = htons(pat->n_pelem);
    ph->n_sect1 = htons(pat->s1_str.used);
    memcpy( pat->raw_data + PAT_HEAD_OFFSET, ph, sizeof(*ph) );


    //--- setup section 0

    pat_s0_bhead_t *bh = &pat->base;
    const uint base_size = sizeof(pat_s0_bhead_t) + pat->n_pelem * sizeof(pat_s0_belem_t);
    bh->size   = htonl(base_size);
    bh->n_elem = htons(pat->n_pelem);

    pat_s0_bhead_t *dest_bh = (pat_s0_bhead_t*)(pat->raw_data + s0_offset);
    memcpy( dest_bh, bh, sizeof(*bh) );


    //--- setup section 0: string lists

    u32 sref_offset  = s0_offset + base_size;
    u32 shead_offset = sref_offset + pat->n_pelem * sizeof(pat_s0_sref_t);

    for ( i = 0; i < pat->n_pelem; i++ )
    {
	const int idx = pat->shead_order[i];
	pat_element_t  *pe = pat->pelem + idx;
	pat_s0_shead_t *sh = &pe->shead;
	sh->n_elem = htons(pe->selem_used);

	//--- copy data

	memcpy( pat->raw_data + shead_offset, sh, sizeof(*sh) );
	pe->sref.offset_strlist
	    = htonl(shead_offset - sref_offset - sizeof(pat_s0_sref_t) * idx);
	shead_offset += sizeof(pat_s0_shead_t);

	const uint copy_size = sizeof(pat_s0_selem_t) * pe->selem_used;
	memcpy( pat->raw_data + shead_offset, pe->selem, copy_size );
	shead_offset += copy_size;
    }


    //--- setup section 0: string reference

    for ( i = 0; i < pat->n_pelem; i++ )
    {
	const int idx = pat->sref_order[i];
	pat_element_t  *pe = pat->pelem + idx;
	pat_s0_sref_t *sr = &pe->sref;
	sr->offset_name = htonl(FindStringPool(&sp,pe->sname,0)-sref_offset);

	memcpy( pat->raw_data + sref_offset, sr, sizeof(*sr) );
	pe->belem.offset_strref = htonl(sref_offset - s0_offset);
	sref_offset += sizeof(pat_s0_sref_t);
    }


    //--- setup section 0: base elements

    for ( i = 0, pe = pat->pelem; i < pat->n_pelem; i++, pe++ )
    {
	pat_s0_belem_t *be = &pe->belem;
	be->offset_name = htonl(FindStringPool(&sp,pe->bname,0)-s0_offset);

	memcpy( dest_bh->elem+i, be, sizeof(*be) );
    }


    //--- setup section 1+3

    ParamFieldItem_t *it = pat->s1_str.field;
    for ( i = 0; i < pat->s1_str.used; i++, it++ )
    {
	u32 offset = FindStringPool(&sp,it->key,0);
	if (offset)
	    offset -= s1_offset;
	write_be32( pat->raw_data + s1_offset + 4*i, offset );
	write_be32( pat->raw_data + s3_offset + 4*i, it->num );
    }


    //--- terminate

    ResetStringPool(&sp);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			setup pat parser		///////////////
///////////////////////////////////////////////////////////////////////////////

const VarMap_t * SetupVarsPAT()
{
    static VarMap_t vm = {0};
    if (!vm.used)
    {
 #if 0
	//--- setup integer variables

	struct inttab_t { ccp name; int val; };
	static const struct inttab_t inttab[] =
	{
	    {0,0}
	};

	const struct inttab_t * ip;
	for ( ip = inttab; ip->name; ip++ )
	    DefineIntVar(&vm,ip->name,ip->val);
 #endif

	//--- setup basic parser variables

	DefineParserVars(&vm);
    }

    return &vm;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    ScanTextPAT: helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_SETUP
(
    pat_t		* pat,		// PAT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(pat);
    DASSERT(si);
    TRACE("ScanText_SETUP()\n");

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
	    ScanU32SI(si,(u32*)&pat->revision,1,0);
	    si->init_revision = si->cur_file->revision = pat->revision;
	    DefineIntVar(&si->gvar,"REVISION$SETUP",pat->revision);
	    DefineIntVar(&si->gvar,"REVISION$ACTIVE",pat->revision);
	}
	else if (!strcmp(name,"NAME"))
	    ConcatStringsSI(si,(char**)&pat->name,0);
	else
	    GotoEolSI(si);
	CheckEolSI(si);
    }
    CheckLevelSI(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_SIMPLE
(
    pat_t		* pat,		// PAT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(pat);
    DASSERT(si);
    TRACE("ScanText_SIMPLE()\n");

    memset(&pat->head,0,sizeof(pat->head));
    memset(&pat->base,0,sizeof(pat->base));
    pat->base.unknown_08 = htons(0xffff);
    pat->base.n_unknown  = 1;

    ClearElementsPAT(pat);
    pat_element_t *pe = pat->pelem;
    pe->belem.unknown_06 = htons(1);
    pe->sref.type = htons(5);
    pat->n_pelem = 1;

    slist_param_t parm;
    Close_SList(&parm,0);
    slist_list_t elem;
    memset(&elem,0,sizeof(elem));
    double last_time = 0.0;

    int enabled = 1;
    ScanParam_t ptab[] =
    {
	{ "ENABLED",		SPM_INT,	&enabled },
	{ "BRRES-NAME",		SPM_STRING,	&pat->name },
	{ "PAT-NAME",		SPM_STRING,	&pe->bname },
	{ "N-FRAMES",		SPM_U16,	&pat->head.n_frames },
	{ "CYCLIC",		SPM_U16,	&pat->head.cyclic },
	{ "UNKNOWN-00",		SPM_U16,	&pe->belem.unknown_00 },
	{ "FACTOR",		SPM_FLOAT,	&parm.factor },
	{ "PADDING",		SPM_U16,	&parm.unknown_02 },
	{0}
    };

    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    if (!enabled)
	    {
		ClearPAT(pat);
		pat->outmode = PATMD_COMPLEX;
		return err;
	    }
	    continue;
	}
	pat->outmode = PATMD_SIMPLE;

	double time;
	char *str = 0;
	if ( ScanDoubleSI(si,&time,1) || ConcatStringsSI(si,&str,0) )
	    continue;

	ParamFieldItem_t *it
	    = FindInsertParamField( &pat->s1_str, str, true, 0, 0 );
	const uint sidx = it ? it - pat->s1_str.field : 0;

	if ( elem.selem_used == elem.selem_size )
	{
	    elem.selem_size += elem.selem_size + 20;
	    elem.selem = REALLOC(elem.selem,elem.selem_size*sizeof(*elem.selem));
	}
	DASSERT(elem.selem);
	DASSERT(elem.selem_used < elem.selem_size);
	pat_s0_selem_t *se = elem.selem + elem.selem_used++;
	last_time += time;
	write_bef4(&se->time,last_time);
	se->index = htons(sidx);
	se->unknown_06 = htons(0);

	CheckEolSI(si);
    }
    Close_SList(&parm,&elem);

    pe->sname = STRDUP(pe->bname);
    be16n((u16*)&pat->head,(u16*)&pat->head,sizeof(pat->head)/2);
    be16n((u16*)&pat->base,(u16*)&pat->base,sizeof(pat->base)/2);
    be16n((u16*)&pe->belem.unknown_00,(u16*)&pe->belem.unknown_00,1);
    pe->shead.unknown_02 = htons(elem.unknown_02);
    write_bef4(&pe->shead.factor,elem.factor);
    pe->selem = elem.selem;
    pe->selem_used = elem.selem_used;
    pe->selem_size = elem.selem_size;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_HEADER
(
    pat_t		* pat,		// PAT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(pat);
    DASSERT(si);
    TRACE("ScanText_HEADER()\n");

    memset(&pat->head,0,sizeof(pat->head));

    ScanParam_t ptab[] =
    {
	{ "NAME",		SPM_STRING,	&pat->name },
	{ "UNKNOWN-00",		SPM_U16,	&pat->head.unknown_00 },
	{ "UNKNOWN-02",		SPM_U16,	&pat->head.unknown_02 },
	{ "N-FRAMES",		SPM_U16,	&pat->head.n_frames },
    //	{ "N-SECT0",		SPM_U16,	&pat->head.n_sect0 },
    //	{ "N-SECT1",		SPM_U16,	&pat->head.n_sect1 },
	{ "UNKNOWN-0A",		SPM_U16,	&pat->head.unknown_0a },
	{ "UNKNOWN-0C",		SPM_U16,	&pat->head.unknown_0c },
	{ "CYCLIC",		SPM_U16,	&pat->head.cyclic },
	{0}
    };

    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	WarnIgnoreSI(si,0);
    }

    be16n((u16*)&pat->head,(u16*)&pat->head,sizeof(pat->head)/2);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_S0_BASE
(
    pat_t		* pat,		// PAT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(pat);
    DASSERT(si);
    TRACE("ScanText_S0_BASE()\n");

    memset(&pat->base,0,sizeof(pat->base));
    pat->base.unknown_08 = 0xffff;

    ScanParam_t ptab[] =
    {
    //	{ "SIZE",	SPM_U32, &pat->base.size },
	{ "UNKNOWN-04",	SPM_U16, &pat->base.unknown_04 },
    //	{ "N-ELEM",	SPM_U16, &pat->base.n_elem },
	{ "UNKNOWN-08",	SPM_U16, &pat->base.unknown_08 },
	{ "UNKNOWN-0A",	SPM_U16, &pat->base.unknown_0a },
	{ "N-UNKNOWN",	SPM_U16, &pat->base.n_unknown },
	{ "UNKNOWN-0E",	SPM_U16, &pat->base.unknown_0e },
	{ "UNKNOWN-10",	SPM_U16, &pat->base.unknown_10 },
	{ "UNKNOWN-12",	SPM_U16, &pat->base.unknown_12 },
	{ "UNKNOWN-14",	SPM_U16, &pat->base.unknown_14 },
	{ "UNKNOWN-16",	SPM_U16, &pat->base.unknown_16 },
	{0}
    };

    ClearElementsPAT(pat);
    uint n_elem = 0;
    uint n_sref = 0;
    u8 elem_to_sref[PAT_MAX_ELEM];
    u8 sref_to_shead[PAT_MAX_ELEM];
    pat_s0_sref_t sref_list[PAT_MAX_ELEM];
    memset(sref_list,0,sizeof(sref_list));
    char *sname[PAT_MAX_ELEM] = {0};

    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	if ( ch == '$' )
	{
	    char name[PAT_MAX_NAME_LEN+1];
	    ScanNameSI(si,name,sizeof(name),true,true,0);

	    if ( !strcmp(name,"$BASE") )
	    {
		if ( n_elem == PAT_MAX_ELEM )
		{
		    WarnIgnoreSI(si,
			"Maximum number of base elements (%u) already defined.\n",
			PAT_MAX_ELEM );
		    continue;
		}

		enumError err;
		ScanIndexSI(si,&err,n_elem,0,2);
		if (err)
		    continue;

		pat_element_t *pe = pat->pelem + n_elem;
		pat_s0_belem_t *be = &pe->belem;
		memset(be,0,sizeof(*be));
		ScanU16SI(si,(u16*)be,4,0);
		be16n((u16*)be,(u16*)be,4);

		ScanU8SI(si,elem_to_sref+n_elem,1,0);
		ConcatStringsSI(si,(char**)&pe->bname,0);

		n_elem++;
		CheckEolSI(si);
		continue;
	    }

	    if ( !strcmp(name,"$SREF") )
	    {
		if ( n_sref == n_elem )
		{
		    WarnIgnoreSI(si,
			"You can't define more string references as base elements (%u).\n",
			n_elem );
		    continue;
		}

		enumError err;
		ScanIndexSI(si,&err,n_sref,0,2);
		if (err)
		    continue;

		pat_s0_sref_t *sref = sref_list + n_sref;
		ScanU16SI(si,&sref->unknown_04,2,0);
		be16n(&sref->unknown_04,&sref->unknown_04,2);

		ScanU8SI(si,sref_to_shead+n_sref,1,0);
		ConcatStringsSI(si,sname+n_sref,0);

		n_sref++;
		CheckEolSI(si);
		continue;
	    }

	    si->cur_file->ptr = si->cur_file->prev_ptr;
	}

	WarnIgnoreSI(si,0);
    }

    be16n((u16*)&pat->base,(u16*)&pat->base,sizeof(pat->base)/2);
    pat->n_pelem = n_elem;

    //--- copy 'sref' and 'sname'

    if (!si->no_warn)
    {
	uint i;
	pat_element_t *pe = pat->pelem;
	for ( i = 0; i < n_elem; i++, pe++ )
	{
	    const uint s_idx = elem_to_sref[i];
	    noPRINT(">>> %u -> %u\n",i,s_idx);
	    if ( s_idx >= n_sref )
	    {
		ERROR0(ERR_WARNING,
		    "No 'string reference' for base element %u (sref=%u).\n[%s @%u]\n",
		    i, s_idx, si->cur_file->name, si->cur_file->line );
		continue;
	    }

	    memcpy( &pe->sref, sref_list+s_idx, sizeof(pe->sref) );
	    if (sname[s_idx])
		pe->sname = STRDUP(sname[s_idx]);

	    pat->sref_order[s_idx] = i;
	    pat->elem_to_shead[i] = sref_to_shead[s_idx];
	}
    }

    uint i;
    for ( i = 0; i < n_sref; i++ )
	FreeString(sname[i]);

    //PRINT("\e[33;1m elem_to_shead:\e[0m\n");
    //HEXDUMP16(15,0,pat->elem_to_shead,sizeof(pat->elem_to_shead));
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_S0_LIST
(
    pat_t		* pat,		// PAT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(pat);
    DASSERT(si);
    TRACE("ScanText_S0_LIST()\n");

    slist_param_t parm;
    Close_SList(&parm,0);

    ScanParam_t ptab[] =
    {
	{ "FACTOR",	SPM_FLOAT,	&parm.factor },
	{ "PADDING",	SPM_U16,	&parm.unknown_02 },
	{0}
    };

    slist_list_t elist[PAT_MAX_ELEM], *elem = 0;
    memset(elist,0,sizeof(elist));
    uint n_list = 0;
    bool no_elem_warned = false;

    double last_time = 0.0;

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	if ( ch == '$' )
	{
	    char name[PAT_MAX_NAME_LEN+1];
	    ScanNameSI(si,name,sizeof(name),true,true,0);
	    if (!strcmp(name,"$LIST"))
	    {
		Close_SList(&parm,elem);

		if ( n_list == PAT_MAX_ELEM )
		{
		    WarnIgnoreSI(si,
			"Maximum number of string list (%u) already defined.\n",
			PAT_MAX_ELEM );
		    elem = 0;
		    continue;
		}

		enumError err;
		ScanIndexSI(si,&err,n_list,0,2);
		if (err)
		    continue;

		elem = elist + n_list++;
		last_time = 0.0;
		CheckEolSI(si);
		continue;
	    }

	    WarnIgnoreSI(si,0);
	    continue;
	}

	if (!elem)
	{
	    if (!no_elem_warned)
	    {
		no_elem_warned = true;
		WarnIgnoreSI(si,"Define a list with '$LIST <var_name>' first.\n");
	    }
	    else
		GotoEolSI(si);
	    continue;
	}

	double time;
	u16 param[2];
	if ( ScanDoubleSI(si,&time,1) || ScanU16SI(si,param,2,0) )
	    continue;

	if ( elem->selem_used == elem->selem_size )
	{
	    elem->selem_size += elem->selem_size + 20;
	    elem->selem = REALLOC(elem->selem,elem->selem_size*sizeof(*elem->selem));
	}
	DASSERT(elem->selem);
	DASSERT(elem->selem_used < elem->selem_size);
	pat_s0_selem_t *se = elem->selem + elem->selem_used++;
	last_time += time;
	write_bef4(&se->time,last_time);
	se->index = htons(param[0]);
	se->unknown_06 = htons(param[1]);

	CheckEolSI(si);
    }
    Close_SList(&parm,elem);

    if (!si->no_warn)
    {
	uint i;
	pat_element_t *pe = pat->pelem;
	for ( i = 0; i < pat->n_pelem; i++, pe++ )
	{
	    const uint h_idx = pat->elem_to_shead[i];
	    noPRINT(">>> %u -> %u\n",i,h_idx);
	    if ( h_idx >= n_list )
	    {
		ERROR0(ERR_WARNING,
		    "No 'string list' for base element %u (index=%u).\n[%s @%u]\n",
		    i, h_idx, si->cur_file->name, si->cur_file->line );
		continue;
	    }
	    pat->shead_order[h_idx] = i;

	    slist_list_t *elem = elist + h_idx;
	    FREE(pe->selem);
	    const uint copy_size = elem->selem_used * sizeof(*elem->selem);
	    pe->selem = MALLOC(copy_size);
	    memcpy(pe->selem,elem->selem,copy_size);
	    pe->selem_used = pe->selem_size = elem->selem_used;

	    write_bef4(&pe->shead.factor, elem->factor );
	    pe->shead.unknown_02 = htons(elem->unknown_02);
	}
    }

    uint i;
    for ( i = 0; i < n_list; i++ )
	FREE(elist[i].selem);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanText_STRINGS
(
    pat_t		* pat,		// PAT data structure
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(pat);
    DASSERT(si);
    TRACE("ScanText_STRINGS()\n");

    ResetParamField(&pat->s1_str);

    ScanParam_t ptab[] =
    {
	{0}
    };

    for(;;)
    {
	char ch = NextCharSI(si,true);
	if ( !ch || ch == '[' )
	    break;

	if ( ch == '@' )
	{
	    ScanParamSI(si,ptab);
	    continue;
	}

	enumError err;
	ScanIndexSI(si,&err,pat->s1_str.used,0,2);
	if (err)
	    continue;

	uint attrib = 0;
	if (ScanU32SI(si,&attrib,1,0))
	    continue;

	char *str = 0;
	if (ConcatStringsSI(si,&str,0))
	    continue;

	AppendParamField(&pat->s1_str,str,true,attrib,0);
	CheckEolSI(si);
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanTextPAT()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanTextPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    PRINT("ScanTextPAT(init=%d)\n",init_pat);

    DASSERT(pat);
    if (init_pat)
	InitializePAT(pat);

    ScanInfo_t si;
    InitializeSI(&si,data,data_size,pat->fname,pat->revision);
    si.predef = SetupVarsPAT();

    enumError max_err = ERR_OK;
    bool is_pass2 = false;

    for(;;)
    {
	PRINT("----- SCAN PAT SECTIONS, PASS%u ...\n",is_pass2);

	ClearPAT(pat);

	max_err = ERR_OK;
	si.no_warn = !is_pass2;
	si.total_err = 0;
	DefineIntVar(&si.gvar, "$PASS", is_pass2+1 );

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
	    ResetLocalVarsSI(&si,pat->revision);

	    si.cur_file->ptr++;
	    char name[20];
	    ScanNameSI(&si,name,sizeof(name),true,true,0);
	    noPRINT("--> #%04u: %s\n",si.line,name);

	    int abbrev_count;
	    const KeywordTab_t *cmd = ScanKeyword(&abbrev_count,name,pat_section_name);
	    if ( !cmd || abbrev_count )
		continue;
	    NextLineSI(&si,false,false);

	    noPRINT("\e[44;37;1m pass %u [%s] line %u \e[0m\n",
		is_pass2+1, cmd->name1, si.cur_file->line );

	    if ( cmd->id < 0 || cmd->id >= PAT_N_SECT )
		continue;

	    enumError err = ERR_OK;
	    switch (cmd->id)
	    {
		case PAT_SECT_SETUP:
		    err = ScanText_SETUP(pat,&si);
		    break;

		case PAT_SECT_SIMPLE:
		    if ( pat->outmode != PATMD_COMPLEX )
			err = ScanText_SIMPLE(pat,&si);
		    break;

		case PAT_SECT_HEADER:
		    if ( pat->outmode != PATMD_SIMPLE )
		    {
			pat->outmode = PATMD_COMPLEX;
			err = ScanText_HEADER(pat,&si);
		    }
		    break;

		case PAT_SECT_S0_BASE:
		    if ( pat->outmode != PATMD_SIMPLE )
		    {
			pat->outmode = PATMD_COMPLEX;
			err = ScanText_S0_BASE(pat,&si);
		    }
		    break;

		case PAT_SECT_S0_LIST:
		    if ( pat->outmode != PATMD_SIMPLE )
		    {
			pat->outmode = PATMD_COMPLEX;
			err = ScanText_S0_LIST(pat,&si);
		    }
		    break;

		case PAT_SECT_STRINGS:
		    if ( pat->outmode != PATMD_SIMPLE )
		    {
			pat->outmode = PATMD_COMPLEX;
			err = ScanText_STRINGS(pat,&si);
		    }
		    break;

		default:
		    // ignore all other sections without any warnings
		    break;
	    }

	    if ( max_err < err )
		 max_err = err;
	}

	if (is_pass2)
	    break;
	is_pass2 = true;
	RestartSI(&si);
    }

 #if HAVE_PRINT0
    printf("VAR DUMP/GLOBAL:\n");
    DumpVarMap(stdout,3,&si.gvar,false);
 #endif

    CheckLevelSI(&si);
    if ( max_err < ERR_WARNING && si.total_err )
	max_err = ERR_WARNING;
    PRINT("ERR(ScanTextPAT) = %u (errcount=%u)\n", max_err, si.total_err );
    ResetSI(&si);

    CreateRawPAT(pat);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ScanRawPAT()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanRawPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerPAT_t	* cdata		// NULL or container-data
)
{
    DASSERT(pat);

 #if USE_NEW_CONTAINER_PAT // ---------------------------------------

    if (init_pat)
	InitializePAT(pat);

    CatchContainerData(&pat->container,0,cdata);
    AssignContainer(&pat->container,0,data,data_size,CPM_COPY);
 #ifdef TEST
    DumpInfoContainer(stdlog,collog,2,"PAT: ",&pat->container,0);
 #endif

 #else // !USE_NEW_CONTAINER_PAT // ---------------------------------

    if (init_pat)
	InitializePAT(pat);
    else
	FreeContainer(pat->old_container);
    pat->old_container = cdata ? cdata : NewContainer(0,data,data_size,CPM_COPY);

 #endif // !USE_NEW_CONTAINER_PAT // --------------------------------

    const endian_func_t	* endian = &be_func;
    pat_analyse_t ana;
    if ( IsValidPAT(data,data_size,data_size,0,pat->fname,endian,&ana) >= VALID_ERROR )
    {
	return ERROR0(ERR_INVALID_DATA,
		"Invalid PAT file: %s\n", pat->fname ? pat->fname : "?");
    }


    //--- scan PAT base infos

    pat->version = endian->rd32(data+8);
    pat->n_sect  = GetSectionNumBRSUB(data,data_size,endian);

    u32 offset = pat->n_sect * 4 + sizeof(brsub_header_t);
    ccp name = GetStringPAT(pat, data, be32(data+offset), 0 );
    if (name)
	pat->name = STRDUP(name);
    PRINT("PAT/NAME: %s\n",pat->name);


    //--- copy header and section 0

    memcpy( &pat->head, ana.head,    sizeof(pat->head) );
    memcpy( &pat->base, ana.s0_base, sizeof(pat->base) );

    memcpy( &pat->sref_order,  ana.s0_sref_order,  sizeof(pat->sref_order) );
    memcpy( &pat->shead_order, ana.s0_shead_order, sizeof(pat->shead_order) );

    uint i;
    pat->n_pelem = ana.n_sect0 < PAT_MAX_ELEM ? ana.n_sect0 : PAT_MAX_ELEM;
    pat_element_t *pe = pat->pelem;
    for ( i = 0; i < pat->n_pelem; i++, pe++ )
    {
	memcpy( &pe->belem, ana.s0_base->elem+i, sizeof(pe->belem) );
	memcpy( &pe->sref,  ana.s0_sref[i],	sizeof(pe->sref) );
	memcpy( &pe->shead, ana.s0_shead[i],	sizeof(pe->shead) );

	pe->bname = STRDUP(GetStringPAT( pat, ana.s0_base,
			be32(&ana.s0_base->elem[i].offset_name), "?" ));
	pe->sname = STRDUP(GetStringPAT( pat, ana.s0_sref[i],
			be32(&ana.s0_sref[i]->offset_name), "?" ));

	const uint n = ana.n_s0_list[i];
	if (n)
	{
	    pe->selem_used = pe->selem_size = n;
	    pe->selem = REALLOC(pe->selem,n*sizeof(*pe->selem));
	    memcpy( pe->selem, ana.s0_shead[i]->elem, n*sizeof(*pe->selem) );
	}
    }


    //--- analyse sections 1 and 3

    if (ana.s1_list)
    {
	uint i;
	for ( i = 0; i < ana.n_sect1; i++ )
	{
	    ccp str = GetStringPAT(pat,ana.s1_list,be32(ana.s1_list+i),"?");
	    AddS1String( pat, str, ana.s3_list ? be32(ana.s3_list+i) : 0 );
	}
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PAT0 scan + load		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    const void		* data,		// data to scan
    uint		data_size,	// size of 'data'
    ContainerPAT_t	* cdata,	// NULL or container-data
    CheckMode_t		mode		// not NULL: call CheckPAT(mode)
)
{
    DASSERT(pat);
    PRINT("ScanPAT(mode=%x) size=%u\n",mode,data_size);

    enumError err;
// [[analyse-magic]]
    switch (GetByMagicFF(data,data_size,data_size))
    {
	case FF_PAT:
	    pat->fform = FF_PAT;
	    err = ScanRawPAT(pat,init_pat,data,data_size,cdata);
	    break;

	case FF_PAT_TXT:
	 #if USE_NEW_CONTAINER_PAT
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    pat->fform = FF_PAT_TXT;
	    err =  ScanTextPAT(pat,init_pat,data,data_size);
	    break;

	default:
	 #if USE_NEW_CONTAINER_PAT
	    FreeContainerData(cdata);
	 #else
	    FreeContainer(cdata);
	 #endif
	    if (init_pat)
		InitializePAT(pat);
	    return ERROR0(ERR_INVALID_DATA,
		"No PAT file: %s\n", pat->fname ? pat->fname : "?");
    }

    if ( err <= ERR_WARNING && mode )
	CheckPAT(pat,mode);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanRawDataPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    struct raw_data_t	* raw,		// valid raw data
    CheckMode_t		mode		// not NULL: call CheckPAT(mode)
)
{
    DASSERT(pat);
    DASSERT(raw);
    if (init_pat)
	InitializePAT(pat);
    else
	ResetPAT(pat);

    pat->fatt  = raw->fatt;
    pat->fname = raw->fname;
    raw->fname = 0;
 #if USE_NEW_CONTAINER_PAT
    return ScanPAT( pat, false, raw->data, raw->data_size,
			LinkContainerRawData(raw), mode );
 #else
    return ScanPAT(pat,false,raw->data,raw->data_size,ContainerRawData(raw),mode);
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadPAT
(
    pat_t		* pat,		// PAT data structure
    bool		init_pat,	// true: initialize 'pat' first
    ccp			fname,		// valid pointer to filenname
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    CheckMode_t		mode		// not NULL: call CheckPAT(mode)
)
{
    DASSERT(pat);
    DASSERT(fname);
    if (init_pat)
	InitializePAT(pat);
    else
	ResetPAT(pat);

    //--- load and scan data

    raw_data_t raw;
    enumError err = LoadRawData(&raw,true,fname,0,ignore_no_file,0);
    if (!err)
    {
	pat->fname = raw.fname;
	raw.fname = 0;
     #if USE_NEW_CONTAINER_PAT
	err = ScanPAT( pat, false, raw.data, raw.data_size,
			LinkContainerRawData(&raw), mode );
     #else
	err = ScanPAT(pat,false,raw.data,raw.data_size,ContainerRawData(&raw),mode);
     #endif
    }

    ResetRawData(&raw);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveRawPAT()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveRawPAT
(
    pat_t		* pat,		// pointer to valid PAT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    DASSERT(pat);
    DASSERT(fname);
    PRINT("SaveRawPAT(%s,%d)\n",fname,set_time);

    //--- create raw data

    enumError err = CreateRawPAT(pat);
    if (err)
	return err;
    DASSERT(pat->raw_data);
    DASSERT(pat->raw_data_size);

    //--- write to file

    File_t F;
    err = CreateFileOpt(&F,true,fname,testmode,pat->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&pat->fatt,0);

    if ( fwrite(pat->raw_data,1,pat->raw_data_size,F.f) != pat->raw_data_size )
	err = FILEERROR1(&F,ERR_WRITE_FAILED,"Write failed: %s\n",fname);
    return ResetFile(&F,set_time);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveTextPAT()			///////////////
///////////////////////////////////////////////////////////////////////////////

void SaveSimplePAT
(
    pat_t		* pat,		// pointer to valid PAT
    FILE		*f,		// open FILE
    pat_mode_t		outmode		// output mode
)
{
    if (brief_count)
	fprintf(f,
		"%s[SIMPLE]\r\n"
		"@REVISION = %u\r\n",
		section_sep,
		REVISION_NUM );
    else
	fprintf(f,text_pat_simple_cr,REVISION_NUM);

    if ( outmode & PATMD_COMPLEX )
    {
	const bool is_simple = IsSimplePAT(pat);
	fprintf(f,
		"\r\n"
		"# 0: Ignore this section and enable COMPLEX support%s.\r\n"
		"# 1: Enable this section and disable all others%s.\r\n"
		"@ENABLED = %u\r\n",
		is_simple ? ""
			  : " (default because of complex PAT)",
		is_simple ? " (default for this simple PAT)"
			  : " (data will be lost if enabled)",
		is_simple );
    }

    const pat_element_t *pe = pat->pelem;
    fprintf(f,
		"\r\n"
		"@BRRES-NAME = \"%s\"\r\n"
		"@PAT-NAME   = \"%s\"\r\n"
		"@N-FRAMES   = %5u\r\n"
		"@CYCLIC     = %5u\r\n"
		"@UNKNOWN-00 = %5u\r\n"
		,GetNamePAT(pat)
		,pe->bname
		,ntohs(pat->head.n_frames)
		,ntohs(pat->head.cyclic)
		,ntohs(pe->belem.unknown_00)
		);

    float factor = bef4(&pe->shead.factor);
    if (!IsSameF(factor,CalcFactorSHEAD(pe->selem,pe->selem_used),2))
	fprintf(f,
	    "@FACTOR     = %8.6f  # if defined, automatic calculation is disabled\r\n",
	    factor );

    uint l, fw = 4;
    const pat_s0_selem_t *se;
    for ( l = 0, se = pe->selem; l < pe->selem_used; l++, se++ )
    {
	uint sidx = ntohs(se->index);
	if ( sidx < pat->s1_str.used )
	{
	    const uint slen = strlen(pat->s1_str.field[sidx].key);
	    if ( fw < slen )
		 fw = slen;
	}
    }
    fw += 3;

    fprintf(f,
		"\r\n"
	"    #-----------------%.*s#-----------\r\n"
	"    #  delta   string %*s#  abs_time\r\n"
	"    #-----------------%.*s#-----------\r\n",
	fw - 4, Minus300,
	fw - 4, "",
	fw - 4, Minus300 );

    float prev = 0.0;
    for ( l = 0, se = pe->selem; l < pe->selem_used; l++, se++ )
    {
	float time = bef4(&se->time);
	uint sidx = ntohs(se->index);
	ccp str = sidx < pat->s1_str.used ? pat->s1_str.field[sidx].key : "?";
	fprintf(f,"%13.4f  \"%s\" %*s# %9.4f\r\n",
	    time - prev,
	    str,
	    (int)(fw - strlen(str)), "",
	    time );
	prev = time;
    }

    fprintf(f,
	"    #-----------------%.*s#-----------\r\n",
	fw - 4, Minus300 );
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveTextPAT
(
    pat_t		* pat,		// pointer to valid PAT
    ccp			fname,		// filename of destination
    bool		set_time	// true: set time stamps
)
{
    // use DOS/Windows line format -> unix can handle it ;)

    DASSERT(pat);
    DASSERT(fname);

    pat_mode_t outmode = PAT_MODE & PATMD_M_BOTH;
    if (!outmode)
	outmode = IsSimplePAT(pat) ? PATMD_SIMPLE : PATMD_COMPLEX;
    char outmode_info[20];
    PrintPatMode(outmode_info,sizeof(outmode_info),outmode);

    PRINT("SaveTextPAT(%s,%d) outmode=%x[%s]\n",
	    fname, set_time, outmode, outmode_info );

    ccp calc_on_scan = "  # calculated on scanning";


    //--- open file

    File_t F;
    enumError err = CreateFileOpt(&F,true,fname,testmode,pat->fname);
    if ( err > ERR_WARNING || !F.f )
	return err;
    SetFileAttrib(&F.fatt,&pat->fatt,0);


    //--- print file header + syntax info

    if ( print_header && !brief_count )
	fprintf(F.f,text_pat_file_head_cr);
    else
	fprintf(F.f,"%s\r\n",PAT_TEXT_MAGIC);


    //--- print info section

    if (brief_count)
    {
	fprintf(F.f,
		"%s[SETUP]\r\n\r\n"
		"TOOL     = %s\r\n"
		"SYSTEM2  = %s\r\n"
		"VERSION  = %s\r\n"
		"REVISION = %u\r\n"
		"DATE     = %s\r\n"
		"\r\n"
		"PAT-VERSION = %2u\r\n"
		"N-SECTIONS  = %2u\r\n"
		"OUTPUT-MODE = %s\r\n"
		"\r\n",
		section_sep,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		pat->version, pat->n_sect, outmode_info );
    }
    else
    {
	fprintf(F.f, text_pat_setup_cr,
		tool_name, SYSTEM2, VERSION, REVISION_NUM, DATE,
		pat->version, pat->n_sect, outmode_info );
    }

    if ( outmode & PATMD_SIMPLE )
    {
	SaveSimplePAT(pat,F.f,outmode);
	if (!(outmode & PATMD_COMPLEX))
	{
	    fputs(section_end,F.f);
	    ResetFile(&F,set_time);
	    return err;
	}
    }


    //--- write PAT header

    if (brief_count)
	fprintf(F.f,
		"%s[HEADER]\r\n"
		"@REVISION = %u\r\n"
		"\r\n",
		section_sep,
		REVISION_NUM );
    else
	fprintf(F.f,text_pat_header_cr,REVISION_NUM);

    const pat_head_t *h = &pat->head;
    fprintf(F.f,
		"@NAME       = \"%s\"\r\n"
		"\r\n"
		"@UNKNOWN-00 = %5u\r\n"
		"@UNKNOWN-02 = %5u\r\n"
		"@N-FRAMES   = %5u\r\n"
		"# N-SECT0   = %5u%s\r\n"
		"# N-SECT1   = %5u%s\r\n"
		"@UNKNOWN-0A = %5u\r\n"
		"@UNKNOWN-0C = %5u\r\n"
		"@CYCLIC     = %5u\r\n"
		"\r\n",
		GetNamePAT(pat),
		ntohs(h->unknown_00),
		ntohs(h->unknown_02),
		ntohs(h->n_frames),
		ntohs(h->n_sect0), calc_on_scan,
		ntohs(h->n_sect1), calc_on_scan,
		ntohs(h->unknown_0a),
		ntohs(h->unknown_0c),
		ntohs(h->cyclic) );


    //--- write PAT S0 base

    if (brief_count)
	fprintf(F.f,
		"%s[S0_BASE]\r\n"
		"@REVISION = %u\r\n"
		"\r\n",
		section_sep,
		REVISION_NUM );
    else
	fprintf(F.f,text_pat_s0_base_cr,REVISION_NUM);

    const pat_s0_bhead_t *bh = &pat->base;
    fprintf(F.f,
		"# SIZE      = %#6x%s\r\n"
		"@UNKNOWN-04 = %#6x\r\n"
		"# N-ELEM    = %6u%s\r\n"
		"@UNKNOWN-08 = %#6x\r\n"
		"@UNKNOWN-0A = %#6x\r\n"
		"@N-UNKNOWN  = %6u\r\n"
		"@UNKNOWN-0E = %#6x\r\n"
		"@UNKNOWN-10 = %#6x\r\n"
		"@UNKNOWN-12 = %#6x\r\n"
		"@UNKNOWN-14 = %#6x\r\n"
		"@UNKNOWN-16 = %#6x\r\n"
		"\r\n",
		ntohl(bh->size), calc_on_scan,
		ntohs(bh->unknown_04),
		ntohs(bh->n_elem), calc_on_scan,
		ntohs(bh->unknown_08),
		ntohs(bh->unknown_0a),
		ntohs(bh->n_unknown),
		ntohs(bh->unknown_0e),
		ntohs(bh->unknown_10),
		ntohs(bh->unknown_12),
		ntohs(bh->unknown_14),
		ntohs(bh->unknown_16) );

    fputs(
	"\r\n"
	"#-------------------+\r\n"
	"# base element list |\r\n"
	"#----------+--------+-------------------+------------------------------\r\n"
	"#   index  |   p00    p02    p04    p06 | sref  name\r\n"
	"#----------+----------------------------+------------------------------\r\n"
	,F.f );

    uint i;
    const pat_element_t *pe = pat->pelem;
    for ( i = 0; i < pat->n_pelem; i++, pe++ )
    {
	fprintf(F.f," $BASE B%-2u  %#6x %#6x %#6x %#6x   R%-3u \"%s\"\r\n",
		i,
		ntohs(pe->belem.unknown_00),
		ntohs(pe->belem.unknown_02),
		ntohs(pe->belem.unknown_04),
		ntohs(pe->belem.unknown_06),
		i,
		pe->bname );
    }

    fputs(
	"#----------+----------------------------+------------------------------\r\n"
	"\r\n"
	"\r\n"
	"#-----------------------+\r\n"
	"# string reference list |\r\n"
	"#----------+------------+-+------------------------------\r\n"
	"#   index  |   p04   type | sref  name\r\n"
	"#----------+--------------+------------------------------\r\n"
	,F.f );


    for ( i = 0; i < pat->n_pelem; i++ )
    {
	const int idx = pat->sref_order[i];
	const pat_element_t *pe = pat->pelem + idx;

	fprintf(F.f," $SREF R%-2u  %#6x %#6x   L%-3u \"%s\"\r\n",
		idx,
		ntohs(pe->sref.unknown_04),
		ntohs(pe->sref.type),
		idx,
		pe->sname );
    }

    fputs(
	"#----------+--------------+------------------------------\r\n"
	,F.f );


    //--- write PAT S0 list

    if (brief_count)
	fprintf(F.f,
		"%s[S0_LIST]\r\n"
		"@REVISION = %u\r\n",
		section_sep,
		REVISION_NUM );
    else
	fprintf(F.f,text_pat_s0_list_cr,REVISION_NUM);

    for ( i = 0; i < pat->n_pelem; i++ )
    {
	const int idx = pat->shead_order[i];
	const pat_element_t *pe = pat->pelem + idx;
	const double factor = bef4(&pe->shead.factor);
	fprintf(F.f,
		"\r\n"
		" $LIST L%-2u # N=%u, factor=1/%6.4f\r\n"
		"   %c@FACTOR  = %11.6f  # if defined, automatic calculation is disabled\r\n"
		"    @PADDING = %#11x  # padding value at offset 2 of list header\r\n"
		"\r\n"
		"    #-----------------------#-----------\r\n"
		"    #  delta   string  p06  #  abs_time\r\n"
		"    #-----------------------#-----------\r\n",
		idx,
		ntohs(pe->shead.n_elem),
		1.0/factor,
		IsSameF(factor,CalcFactorSHEAD(pe->selem,pe->selem_used),1) ? '#' : ' ',
		factor, ntohs(pe->shead.unknown_02) );

	float prev = 0.0;
	uint l;
	const pat_s0_selem_t *se = pe->selem;
	for ( l = 0; l < pe->selem_used; l++, se++ )
	{
	    float tiome = bef4(&se->time);
	    fprintf(F.f,"%13.4f  S%-3u %#6x  # %9.4f\r\n",
		tiome - prev,
		ntohs(se->index),
		ntohs(se->unknown_06),
		tiome );
	    prev = tiome;
	}

	fputs("    #-----------------------#-----------\r\n",F.f);
    }


    //--- strings

   if (brief_count)
	fprintf(F.f,
		"%s[STRINGS]\r\n"
		"@REVISION = %u\r\n"
		"\r\n",
		section_sep,
		REVISION_NUM );
    else
	fprintf(F.f,text_pat_strings_cr,REVISION_NUM);

    fputs(
	"#-------------------------------------------\r\n"
	"# var  attribute  name of TEX pattern\r\n"
	"#-------------------------------------------\r\n"
	,F.f );
    ParamFieldItem_t *it = pat->s1_str.field;
    for ( i = 0; i < pat->s1_str.used; i++, it++ )
	fprintf(F.f,"  S%-2u %#10x  \"%s\"\r\n", i, it->num, it->key );


    //--- terminate

    fputs(section_end,F.f);
    ResetFile(&F,set_time);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    CkeckPAT()			///////////////
///////////////////////////////////////////////////////////////////////////////

int CheckPAT
(
    // returns number of errors

    const pat_t		* pat,		// pointer to valid PAT
    CheckMode_t		mode		// print mode
)
{
    // [[2do]] [[pat]] ???
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

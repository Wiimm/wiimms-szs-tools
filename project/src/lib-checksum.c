
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
 *   Copyright (c) 2011-2020 by Dirk Clemens <wiimm@wiimm.de>              *
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

#include <time.h>
#include <dirent.h>
#include <libgen.h>

#include "crypt.h"
#include "dclib-basics.h"
#include "dclib-numeric.h"
#include "lib-checksum.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct sha1_size_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void CreateSSChecksum ( char *buf, uint bufsize, const sha1_size_t *ss )
{
    DASSERT(buf);
    DASSERT(bufsize>1);
    DASSERT(ss);

    if (opt_db64)
    {
	PRINT("TAB64=%.65s\n",TableEncode64BySetup);
	EncodeBase64(buf,bufsize,ss,sizeof(*ss),
			TableEncode64BySetup, false,0,0 );
    }
    else if (opt_base64)
    {
	PRINT("TAB64=%.65s\n",TableEncode64BySetup);
	EncodeBase64(buf,bufsize,ss->hash,sizeof(ss->hash),
		    TableEncode64BySetup, false,0,0);
    }
    else if (opt_id)
    {
	sha1_id_t id;
	SHA1_to_ID(id,ss->hash);
	StringCopyS(buf,bufsize,(ccp)id);
    }
    else if ( bufsize > 2*sizeof(ss->hash) )
    {
	int i;
	for ( i = 0; i < sizeof(ss->hash); i++ )
	    sprintf(buf+2*i,"%02x",ss->hash[i]);
    }
    else
	*buf = 0;
}

///////////////////////////////////////////////////////////////////////////////

void CreateSSChecksumBySZS ( char *buf, uint bufsize, const szs_file_t *szs )
{
    DASSERT(buf);
    DASSERT(bufsize>10);

    if ( !szs || !szs->data )
    {
	*buf = 0;
	return;
    }

    sha1_size_t info;
    SHA1(szs->data,szs->size,info.hash);
    info.size = htonl(szs->size);
    CreateSSChecksum(buf,bufsize,&info);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// normed DB coding (independent of options)

void CreateSSChecksumDB ( char *buf, uint bufsize, const sha1_size_t *ss )
{
    DASSERT(buf);
    DASSERT(bufsize>CHECKSUM_DB_SIZE);
    DASSERT(ss);

    EncodeBase64(buf,bufsize,ss,sizeof(*ss),TableEncode64url,false,0,0);
}

///////////////////////////////////////////////////////////////////////////////

void CreateSSChecksumDBByData ( char *buf, uint bufsize, cvp data, uint size )
{
    DASSERT(data||!size);
    DASSERT(bufsize>CHECKSUM_DB_SIZE);

    if (!data)
	*buf = 0;
    else
    {
	sha1_size_t info;
	SHA1(data,size,info.hash);
	info.size = htonl(size);
	CreateSSChecksumDB(buf,bufsize,&info);
    }
}

///////////////////////////////////////////////////////////////////////////////

void CreateSSChecksumDBBySZS ( char *buf, uint bufsize, const szs_file_t *szs )
{
    DASSERT(buf);
    DASSERT(bufsize>CHECKSUM_DB_SIZE);

    if (szs)
	CreateSSChecksumDBByData(buf,bufsize,szs->data,szs->size);
    else
	*buf = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError GetSSByFile ( sha1_size_t *ss, ccp path1, ccp path2 )
{
    DASSERT(ss);
    memset(ss,0,sizeof(*ss));

    u8 *data = 0;
    uint size = 0;
    enumError err = OpenReadFILE(path1,path2,false,&data,&size,0,0);
    if (err)
	memset(ss,0,sizeof(*ss));
    else
    {
	SHA1(data,size,ss->hash);
	ss->size = htonl(size);
    }

    FREE(data);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    struct DistributionInfo_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeDistributionInfo
	( DistributionInfo_t * dinf, bool add_default_param )
{
    DASSERT(dinf);
    memset(dinf,0,sizeof(*dinf));
    InitializeParamField(&dinf->translate);
    dinf->translate.free_data = true;
    InitializeParamField(&dinf->param);
    dinf->param.free_data = true;

    uint slot;
    for ( slot = 0; slot < MAX_DISTRIBUTION_ARENA; slot++ )
    {
	ParamField_t *pf = dinf->arena + slot;
	InitializeParamField(pf);
	pf->free_data = true;
    }

    for ( slot = 0; slot < MAX_DISTRIBUTION_TRACK; slot++ )
    {
	ParamField_t *pf = dinf->track + slot;
	InitializeParamField(pf);
	pf->free_data = true;
    }

    if (add_default_param)
	AddParamDistributionInfo(dinf,true);
}

///////////////////////////////////////////////////////////////////////////////

void ResetDistributionInfo ( DistributionInfo_t * dinf )
{
    if (dinf)
    {
	ResetParamField(&dinf->translate);
	ResetParamField(&dinf->param);

	uint slot;
	for ( slot = 0; slot < MAX_DISTRIBUTION_ARENA; slot++ )
	    ResetParamField(dinf->arena+slot);
	for ( slot = 0; slot < MAX_DISTRIBUTION_TRACK; slot++ )
	    ResetParamField(dinf->track+slot);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AddParamDistributionInfo ( DistributionInfo_t * dinf, bool overwrite )
{
    DASSERT(dinf);

    char buf[100];

    //-- UUID

    uuid_buf_t uuid;
    bool create_uuid = overwrite;
    if (!create_uuid)
    {
	const ParamFieldItem_t *it = FindParamField(&dinf->param,"UUID");
	if ( !it || !it->data || ScanUUID(uuid,(ccp)it->data) == (ccp)it->data )
	    create_uuid = true;
    }

    if (create_uuid)
    {
	CreateTextUUID(buf,sizeof(buf));
	ReplaceParamField(&dinf->param,"UUID",false,0,STRDUP(buf));
    }


    //-- time stamps

    time_t tim = time(0);
    struct tm *tm = localtime(&tim);
    strftime(buf,sizeof(buf),"%F %T %z",tm);

    if ( overwrite || !FindParamField(&dinf->param,"FIRST-CREATION") )
	ReplaceParamField(&dinf->param,"FIRST-CREATION",false,0,STRDUP(buf));
    ReplaceParamField(&dinf->param,"LAST-UPDATE",false,0,STRDUP(buf));
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot translation		///////////////
///////////////////////////////////////////////////////////////////////////////

char * ScanSlot ( uint *res_slot, ccp source, bool need_point )
{
    DASSERT(res_slot);
    DASSERT(source);
    *res_slot = 0;

    char *end;
    uint slot = strtoul(source,&end,10);
    if ( !need_point
	&& slot >= MIN_DISTRIBUTION_SLOT
	&& slot <  MAX_DISTRIBUTION_TRACK
	&& (uchar)*end <= ' '
	&& ( end-source == 2 || end-source == 3 ))
    {
	*res_slot = slot;
	return end;
    }

    slot *= 10;
    if ( slot < MIN_DISTRIBUTION_SLOT || slot >= MAX_DISTRIBUTION_TRACK || *end != '.' )
	return (char*)source;

    uint track = strtoul(end+1,&end,10);
    slot += track;
    if ( track < 1 || track > 9 || slot >= MAX_DISTRIBUTION_TRACK )
	return (char*)source;

    *res_slot = slot;
    while ( *end == ' ' || *end == '\t' )
	end++;
    return end;
}

///////////////////////////////////////////////////////////////////////////////

void NormalizeSlotTranslation ( char *buf, uint bufsize, ccp name,
			char ** p_first_para, char ** p_first_brack )
{
    DASSERT(buf);
    DASSERT(bufsize>10);

    if (!name)
	name = "";

    char *dest = buf, *buf_end = buf + bufsize - 4;
    char *first_para = 0;
    char *first_brack = 0;

    uint have_space = 0;
    while ( *name && dest < buf_end )
    {
	int ch = (uchar)*name++;
	switch(ch)
	{
	    case ' ':
	    case '_':
	    case '-':
	    case '+':
		have_space++;
		break;

	    case '\'':
		break; // ignore

	    case '[':
		if (!first_brack)
		    first_brack = dest;
		goto bracket;

	    case '(':
		if (!first_para)
		    first_para = dest;
		// fall through

	    case '{':
	    bracket:
		ch = '(';
		// fall through

	    case '.':
		if (have_space)
		{
		    have_space = 0;
		    *dest++ = ' ';
		}
		*dest++ = ch;
		break;

	    case '}':
	    case ')':
	    case ']':
		*dest++ = ')';
		break;

	    default:
		ch = tolower(ch);
		if ( ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' )
		{
		    if (have_space)
		    {
			have_space = 0;
			*dest++ = ' ';
		    }
		    *dest++ = ch;
		}
		break;
	}
    }

    if ( dest > buf )
    {
	if ( dest > buf+4 && !memcmp(dest-4,".szs",4) || !memcmp(dest-4,".wbz",4) )
	    dest -= 4;
	if ( dest > buf+2 && !memcmp(dest-2,"_d",2) )
	    dest -= 2;
	*dest = 0;

	if ( first_brack)
	{
	    if ( first_brack >= dest )
		first_brack = 0;
	    else
		dest = first_brack;
	}

	if ( first_para >= dest )
	    first_para = 0;
    }
    else
	first_para = first_brack = 0;

    if (p_first_para)
	*p_first_para = first_para;
    if (p_first_brack)
	*p_first_brack = first_brack;
}

///////////////////////////////////////////////////////////////////////////////

uint DefineSlotTranslation
	( ParamField_t *translate, bool is_arena, uint slot, ccp name )
{
    DASSERT(translate);
    if (!name)
	return 0;

    if (is_arena)
    {
	if ( slot >= MAX_DISTRIBUTION_ARENA )
	    slot = 0;
	else
	    slot += DISTRIBUTION_ARENA_DELTA;
    }
    else
    {
	if ( slot >= MAX_DISTRIBUTION_TRACK )
	    slot = 0;
    }

    char buf[1000];
    char *first_para = 0, *first_brack = 0;
    NormalizeSlotTranslation(buf,sizeof(buf),name,&first_para,&first_brack);

    uint count = 0;
    if (*buf)
    {
	ReplaceParamField(translate,buf,false,slot,0);
	count++;

	if (first_brack)
	{
	    *first_brack = 0;
	    ReplaceParamField(translate,buf,false,slot,0);
	    count++;
	}

	if (first_para)
	{
	    *first_para = 0;
	    ReplaceParamField(translate,buf,false,slot,0);
	    count++;
	}
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

#undef EARLY_NUM_SLOT
#define EARLY_NUM_SLOT 1

int FindSlotByTranslation ( const ParamField_t *translate, ccp fname, ccp sha1 )
{
    DASSERT(translate);
    ccp slash = strrchr(fname,'/');
    if (slash)
	fname = slash+1;

 #if EARLY_NUM_SLOT
    uint slot;
    ScanSlot(&slot,fname,false);
    if (slot)
	return slot;
 #endif

    char buf[1000];

    char *first_para = 0, *first_brack = 0;
    NormalizeSlotTranslation(buf,sizeof(buf),fname,&first_para,&first_brack);
    const ParamFieldItem_t *it = FindParamField(translate,buf);
    if ( !it && first_brack )
    {
	*first_brack = 0;
	it = FindParamField(translate,buf);
    }

    if ( !it && first_para )
    {
	*first_para = 0;
	it = FindParamField(translate,buf);
    }

    if ( !it && sha1 )
	it = FindParamField(translate,sha1);


 #if EARLY_NUM_SLOT
    return it ? it->num : 0;
 #else
    if (it)
	return it->num;

    uint slot;
    ScanSlot(&slot,fname,false);
    return slot;
 #endif
}

#undef EARLY_NUM_SLOT

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanSlotTranslation
	( ParamField_t *translate, ParamField_t *param, ccp fname, bool ignore )
{
    DASSERT(translate);
    DASSERT(fname);
    PRINT(" - scan translation%s: %s\n", param ? "+param" : "", fname);

    szs_file_t szs;
    InitializeSZS(&szs);
    enumError err = LoadSZS(&szs,fname,true,ignore,true);
    if (err)
	return;


    //--- support for special file formats

    switch((int)szs.fform_arch)
    {
	case FF_BMG:
	    if (verbose>1)
		printf(" - scan translation [BMG]: %s\n",fname);
	    ResetSZS(&szs);
	    return;
    }


    //--- standart text file

    if ( verbose > 1 )
	printf("Scan translation%s[TEXT/%s]: %s\n",
		param ? "+param" : "",
		GetExtFF(szs.fform_file,szs.fform_arch), fname );
    else if ( verbose >= 0 )
	printf("Scan %s\n",fname);

    char *eol = (char*)szs.data;
    char *end = eol + szs.size;

    for(;;)
    {
	while ( eol < end && (uchar)*eol <= ' ' )
	    eol++;
	if ( eol == end )
	    break;

	char *src = eol;
	while ( eol < end && *eol != 0 && *eol != '\r' && *eol != '\n' )
	    eol++;
	*eol = 0;

	if ( *src == '#' || *src == '!' )
	    continue;


	//--- manage parameters

	if ( *src == '@' )
	{
	    if (!param)
		continue;

	    char name_buf[100], *name = name_buf;
	    src++;

	    for(;;)
	    {
		const char ch = *src;
		if ( !isalnum(ch) && ch != '_' && ch != '-' && ch != '.' )
		    break;
		src++;
		if ( name < name_buf + sizeof(name_buf) - 1 )
		    *name++ = toupper(ch);
	    }

	    while ( src < eol && (uchar)*src <= ' ' )
		src++;
	    if ( src < eol && *src == '=' )
	    {
		src++;
		while ( src < eol && (uchar)*src <= ' ' )
		    src++;
	    }

	    if ( name > name_buf )
	    {
		//PRINT(" %.*s = |%.*s|\n",(int)(name-name_buf),name_buf,(int)(eol-src),src);
		ReplaceParamField( param, MEMDUP(name_buf,name-name_buf),
				true, 0, MEMDUP(src,eol-src) );
	    }
	    continue;
	}

	//PRINTF("LINE: |%.*s|\n",(int)(eol-src),src);


	//--- skip "()"  |  detect SHA1 checksum

	char *sha1 = 0;
	if ( *src == '(' )
	{
	    while ( src < eol && *src != ')' )
		src++;
	    if ( src++ == eol )
		continue;
	}
	else
	{
	    // detect sha1 checksum

	    char *x = src;
	    while ( x < eol && isalnum((int)*x) )
		x++;
	    if ( x < eol && x - src == 40 )
	    {
		sha1 = src;
		src = x;
	    }
	}

	while ( src < eol && (uchar)*src <= ' ' )
	    src++;

	const bool is_arena = *src == 'a' || *src == 'A';
	if (is_arena)
	    src++;

	if (!isdigit((int)*src))
	    continue;

	uint slot;
	src = ScanSlot(&slot,src,true); // ???
	if (!slot)
	    continue;

	char *tab = strrchr(src,'\t');
	if (tab)
	    src = tab;
	while ( src < eol && (uchar)*src <= ' ' )
	    src++;

	DefineSlotTranslation(translate,is_arena,slot,src);
	if (sha1)
	{
	    sha1[40] = 0;
	    DefineSlotTranslation(translate,is_arena,slot,sha1);
	}
    }

    ResetSZS(&szs);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SZS cache support		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp		szs_cache_dir		= 0;
ParamField_t	szs_cache		= {0};

static bool	szs_cache_loaded	= false;
static bool	szs_cache_dirty		= false;
static u64	szs_cache_last_scan	= 0;

///////////////////////////////////////////////////////////////////////////////

void SetupSZSCache ( ccp dir_name, bool use_dirname )
{
    PRINT("SetupSZSCache(%s,%d)\n",dir_name,use_dirname);

    char buf[PATH_MAX];
    if ( use_dirname && dir_name && !IsDirectory(dir_name,false) )
    {
	StringCopyS(buf,sizeof(buf),dir_name);
	if (!opt_cname)
	    opt_cname = STRDUP(basename(buf));
	dir_name = dirname(buf);
	PRINT(" > new dir_name: %s\n",dir_name);
    }

    if ( dir_name && szs_cache_dir && !strcmp(dir_name,szs_cache_dir) )
	return;

    FreeString(szs_cache_dir);
    szs_cache_dir = IsDirectory(dir_name,false) ? STRDUP(dir_name) : 0;
    ResetParamField(&szs_cache);
    szs_cache.free_data = true;
    szs_cache_loaded = false;
    PRINT("SZS_CACHE_DIR = %s\n",szs_cache_dir);
}

///////////////////////////////////////////////////////////////////////////////

void ScanSZSCache ( ccp dir_name, bool purge )
{
    SetupSZSCache(dir_name,false);
    if (!szs_cache_dir)
	return;
    szs_cache_loaded = szs_cache_dirty = true;
    szs_cache_last_scan = time(0);

    DIR * dir = opendir(dir_name);
    if (dir)
    {
	for(;;)
	{
	    struct dirent * dent = readdir(dir);
	    if (!dent)
		break;

	    if	( dent->d_name && *dent->d_name != '.' )
	    {
		noPRINT("> %s\n",dent->d_name);
		StoreSZSCache(dent->d_name,0,purge,0);
	    }
	}
	closedir(dir);
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadSZSCache()
{
    if ( szs_cache_loaded || !szs_cache_dir )
	return ERR_OK;
    szs_cache_loaded = true;
    szs_cache_last_scan = 0;

    char buf[PATH_MAX];
    ccp path = PathCatPP(buf,sizeof(buf),szs_cache_dir,SZS_CACHE_FNAME);
    PRINT("LoadSZSCache() %s\n",path);

    File_t F;
    enumError err = OpenFile(&F,true,path,FM_SILENT,0,0);
    if (err)
	return err;

    while (fgets(buf,sizeof(buf)-1,F.f))
    {
	char *ptr = buf;
	while ( *(uchar*)ptr <= ' ' )
	    ptr++;
	if ( *ptr == '#' || *ptr == '!' )
	    continue;

	if ( *ptr == '@' )
	{
	    char name_buf[100], *name = name_buf;
	    ptr++;
	    for(;;)
	    {
		const char ch = *ptr;
		if ( !isalnum(ch) && ch != '_' && ch != '-' && ch != '.' )
		    break;
		ptr++;
		if ( name < name_buf + sizeof(name_buf) - 1 )
		    *name++ = toupper(ch);
	    }

	    while ( *ptr > 0 && *ptr <= ' ' )
		ptr++;
	    if ( *ptr == '=' )
	    {
		ptr++;
		while ( *ptr > 0 && *ptr <= ' ' )
		    ptr++;
	    }

	    if ( name > name_buf )
	    {
		*name = 0;
		if (!strcmp(name_buf,"LAST-CACHE-SCAN"))
		    szs_cache_last_scan = strtoul(ptr,0,10);
	    }
	    continue;
	}

	uint len = strlen(ptr) - 1;
	while ( len > 0 && (uchar)ptr[len] <= ' ' )
	    len--;
	ptr[len+1] = 0;

	if ( len <= CHECKSUM_DB_SIZE || ptr[CHECKSUM_DB_SIZE] != ' ' )
	    continue;

	char *fname = ptr + CHECKSUM_DB_SIZE;
	*fname++ = 0;
	while ( *(uchar*)fname <= ' ' )
	    fname++;
	if (!*fname)
	    continue;
	StoreSZSCache(fname,ptr,false,0);
    }

    CloseFile(&F,0);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static int cmp_szs_cache ( const void * va, const void * vb )
{
    DASSERT(va);
    DASSERT(vb);
    const ParamFieldItem_t *a = *(ParamFieldItem_t**)va;
    const ParamFieldItem_t *b = *(ParamFieldItem_t**)vb;
    DASSERT(a);
    DASSERT(b);
    return strcasecmp((ccp)a->data,(ccp)b->data);
}

//-----------------------------------------------------------------------------

enumError SaveSZSCache()
{
    if ( !szs_cache_dir || !szs_cache_dirty )
	return ERR_NOTHING_TO_DO;
    szs_cache_dirty = false;

    char path_buf[PATH_MAX];
    ccp path = PathCatPP(path_buf,sizeof(path_buf),szs_cache_dir,SZS_CACHE_FNAME);
    PRINT("SaveSZSCache() N=%u, %s\n",szs_cache.used,path);

    File_t F;
    enumError err = CreateFile(&F,true,path,FM_REMOVE);
    if (err)
	return err;

    char tbuf[50];
    if (szs_cache_last_scan)
    {
	time_t tim = szs_cache_last_scan;
	struct tm *tm = localtime(&tim);
	strftime(tbuf,sizeof(tbuf),"%F %T %z",tm);
    }
    else
	StringCopyS(tbuf,sizeof(tbuf),"-");

    fprintf(F.f,"#SZS-CACHE\n"
		"# %u file%s cached\n"
		"\n"
		"@WSZST-VERSION   = %s\n"
		"@WSZST-REVISION  = %u\n"
		"@LAST-CACHE-SCAN = %llu = %s\n"
		"\n"
		,szs_cache.used ,szs_cache.used==1 ? "" : "s"
		,VERSION
		,REVISION_NUM
		,szs_cache_last_scan,tbuf
		);

    if ( szs_cache.used )
    {
	ParamFieldItem_t **list = CALLOC(szs_cache.used,sizeof(*list));

	uint i;
	for ( i = 0; i < szs_cache.used; i++ )
	    list[i] = szs_cache.field+i;

	if ( szs_cache.used > 1 )
	    qsort( list, szs_cache.used, sizeof(*list), cmp_szs_cache );

	for ( i = 0; i < szs_cache.used; i++ )
	{
	    const ParamFieldItem_t *it = list[i];
	    fprintf(F.f,"%s %s\n",it->key,(ccp)it->data);
	}

	FREE(list);
    }

    CloseFile(&F,0);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
// only basename(fname) is used.
// if checksum==0: load file and calc checksum!

ParamFieldItem_t * StoreSZSCache
(
    // return NULL or pointer to cache item

    ccp		fname,		// NULL or filename, only basename(fname) is used
    ccp		checksum,	// DB checksum; if NULL: load file and calc checksum
    bool	rename_file,	// true: rename existing file
    bool	*r_exists	// not NULL: store a status here
				//	=> true: file result->data exists
)
{
    if (r_exists)
	*r_exists = false;
    if (!szs_cache_loaded)
	LoadSZSCache();

    if (fname)
    {
	ccp slash = strrchr(fname,'/');
	if (slash)
	    fname = slash+1;
	slash = strrchr(fname,'\\');
	if (slash)
	    fname = slash+1;
    }
    if (!*fname)
	fname = 0;

    PRINT("StoreSZSCache(%s,%s,%d,)\n",fname,checksum,rename_file);

    ccp path = 0;
    char path_buf[PATH_MAX], oldpath_buf[PATH_MAX], checksum_buf[CHECKSUM_DB_SIZE+1];

    if (!checksum)
    {
	if (!fname)
	    return 0;

	path = PathCatPP(path_buf,sizeof(path_buf),szs_cache_dir,fname);
	noPRINT(">> %s\n",path);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,path,true,true,true);
	if ( err > ERR_WARNING || err == ERR_NOT_EXISTS || !IsCompressedFF(szs.fform_file) )
	{
	    ResetSZS(&szs);
	    return 0;
	}

	CreateSSChecksumDBBySZS(checksum_buf,sizeof(checksum_buf),&szs);
	ResetSZS(&szs);
	checksum = checksum_buf;
    }

    bool found;
    ParamFieldItem_t *it = InsertParamFieldEx(&szs_cache,checksum,false,0,&found);
    if (found)
    {
	DASSERT(it);
	ccp oldpath = PathCatPP(oldpath_buf,sizeof(oldpath_buf),szs_cache_dir,(ccp)it->data);

	struct stat st;
	if ( stat(oldpath,&st) || !S_ISREG(st.st_mode) )
	    return it;

	if (r_exists)
	    *r_exists = true;

	if ( !rename_file || !fname || !strcmp(fname,(ccp)it->data) )
	    return it;

	if (!path)
	    path = PathCatPP(path_buf,sizeof(path_buf),szs_cache_dir,fname);
	PRINT("RENAME: %s -> %s\n",oldpath,path);
	if (rename(oldpath,path))
	    return it;

	FreeString(it->data);
	it->data = NULL;
    }

    if (it)
    {
	PRINT("USE: %s\n",fname);
	it->data = STRDUP(fname);
    }
    szs_cache_dirty = true;
    return it;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


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

#include <time.h>
#include <dirent.h>
#include <libgen.h>

#include "crypt.h"
#include "dclib-basics.h"
#include "dclib-numeric.h"
#include "lib-checksum.h"
#include "lib-szs.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct sha1_size_hash_t		///////////////
///////////////////////////////////////////////////////////////////////////////

void CreateSS ( cvp data, uint size, sha1_size_hash_t *dest )
{
    if ( dest && data )
    {
	SHA1(data,size,dest->hash);
	dest->size = htonl(size);
    }
    else if (dest)
	memset(dest,0,sizeof(*dest));
}

///////////////////////////////////////////////////////////////////////////////

void CreateSS64 ( cvp data, uint size, sha1_size_b64_t dest )
{
    sha1_size_hash_t sha1;
    CreateSS(data,size,&sha1);
    if (dest)
	EncodeBase64(dest,sizeof(sha1_size_b64_t),&sha1,sizeof(sha1),0,false,0,0);
}

///////////////////////////////////////////////////////////////////////////////

void CreateSSChecksum ( char *buf, uint bufsize, const sha1_size_hash_t *ss )
{
    DASSERT(buf);
    DASSERT(bufsize>1);
    DASSERT(ss);

    if (opt_db64)
    {
	PRINT("TAB64=%.65s\n",TableEncode64BySetup);
	const int coding = opt_coding64 == ENCODE_OFF ? ENCODE_BASE64URL : opt_coding64;
	EncodeByMode(buf,bufsize,(ccp)ss,sizeof(*ss),coding);
//DEL	EncodeBase64(buf,bufsize,ss,sizeof(*ss),
//DEL			TableEncode64BySetup, false,0,0 );
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

    sha1_size_hash_t info;
    SHA1(szs->data,szs->size,info.hash);
    info.size = htonl(szs->size);
    CreateSSChecksum(buf,bufsize,&info);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// normed DB coding (independent of options)

void CreateSSChecksumDB ( char *buf, uint bufsize, const sha1_size_hash_t *ss )
{
    DASSERT(buf);
    DASSERT(bufsize>CHECKSUM_DB_SIZE);
    DASSERT(ss);

    const int coding = opt_coding64 == ENCODE_OFF ? ENCODE_BASE64URL : opt_coding64;
    EncodeByMode(buf,bufsize,(ccp)ss,sizeof(*ss),coding);
//DEL    EncodeBase64(buf,bufsize,ss,sizeof(*ss),TableEncode64url,false,0,0);
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
	sha1_size_hash_t info;
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

void CreateSSXChecksumDBByData ( char *buf, uint bufsize, cvp data, uint size, file_format_t ff )
{
    DASSERT(data||!size);
    DASSERT(bufsize>CHECKSUM_DB_SIZE);

    if (!data)
	*buf = 0;
    else
    {
	sha1_size_hash_t info;
	SHA1(data,size,info.hash);
	info.size = htonl(size);
	CreateSSChecksumDB(buf,bufsize,&info);

	if ( bufsize > CHECKSUM_DB_SIZE + 2 )
	{
	    ccp append = 0;
	    switch((int)ff)
	    {
		case FF_YAZ0:	append = ".y"; break;
		case FF_BZ:	append = ".B"; break;
		case FF_BZIP2:	append = ".b"; break;
		case FF_LZ:	append = ".L"; break;
		case FF_LZMA:	append = ".l"; break;
	    }
	    if (append)
		memcpy(buf+CHECKSUM_DB_SIZE,append,3);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void CreateSSXChecksumDBBySZS ( char *buf, uint bufsize, const szs_file_t *szs )
{
    DASSERT(buf);
    DASSERT(bufsize>CHECKSUM_DB_SIZE);

    if (szs)
	CreateSSXChecksumDBByData(buf,bufsize,szs->data,szs->size,szs->fform_file);
    else
	*buf = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError GetSSByFile ( sha1_size_hash_t *ss, ccp path1, ccp path2 )
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int IsSSChecksum ( sha1_size_hash_t *res, ccp source, int slen )
{
    // slen < 0 => strlen(source)
    // returns: 0:fail, 1:SHA1, 2:DB64

    ccp end = source + ( slen < 0 ? strlen(source) : slen );

    // skip leading controls
    while ( source < end && (uchar)*source <= ' ' )
	source++;

    ccp start = source;
    while ( source < end && (uchar)*source > ' ' )
	source++;

    sha1_size_hash_t temp;
    if (!res)
	res = &temp;

    const uint len = source - start;
    if ( len == 40 )
    {
	ccp ptr;
	for ( ptr = start; ptr < source; ptr++ )
	{
	    const u8 type = TableNumbers[(u8)*ptr];
	    if ( type >= 16 )
		break;
	}

	if ( ptr == source )
	{
	    Sha1Hex2Bin(res->hash,start,source);
	    return 1;
	}
    }
    else if ( len == CHECKSUM_DB_SIZE )
    {
	char buf[sizeof(sha1_size_hash_t)+12];
	uint len64 = DecodeBase64(buf,sizeof(buf),start,len,TableDecode64url,false,0);
	if ( len64 == sizeof(sha1_size_hash_t) )
	{
	    memcpy(res,buf,sizeof(*res));
	    return 2;
	}
    }

    // no a checksum
    memset(res,0,sizeof(sha1_size_hash_t));
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SZS cache support		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp		szs_cache_dir		= 0;
ParamField_t	szs_cache		= {0};
ParamField_t	szs_cache_append	= {0};

static bool	szs_cache_loaded	= false;
static bool	szs_cache_dirty		= false;
static u64	szs_cache_last_scan	= 0;
//static u64	szs_cache_last_append	= 0;

///////////////////////////////////////////////////////////////////////////////

void LogCacheActivity ( ccp keyword, ccp format, ... )
{
    static FILE *f = 0;
    static char myid[12];

    if (!f)
    {
	if (!opt_log_cache)
	    return;
	f = strcmp(opt_log_cache,"-") ? fopen(opt_log_cache,"a") : stdlog;
	if (!f)
	{
	    ERROR1(ERR_CANT_CREATE,"Can't create log file: %s\n",opt_log_cache);
	    opt_log_cache = 0;
	    return;
	}

	char buf[4];
	write_le32(buf,getpid());
	buf[3] = GetTimeUSec(false);
	EncodeBase64(myid,sizeof(myid),buf,4,0,true,0,0);
	PRINT0("%5u %.4s -> %s\n",getpid(),buf,myid);
	myid[5] = 0;
    }

    char buf[PATH_MAX+100];
    *buf = 0;
    if (format)
    {
	va_list arg;
	va_start(arg,format);
	vsnprintf(buf,sizeof(buf),format,arg);
	va_end(arg);
    }

    fprintf(f,"%s %s %-4s %s\n",
	PrintMTimeByFormat("%F %T.@@@",GetTimeMSec(false)), myid, keyword, buf );
    fflush(f);
}

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

	    if	( *dent->d_name != '.' )
	    {
		noPRINT("> %s\n",dent->d_name);
		StoreSZSCache(dent->d_name,0,purge,0);
	    }
	}
	closedir(dir);
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadSZSCache(void)
{
    if ( szs_cache_loaded || !szs_cache_dir )
	return ERR_OK;
    szs_cache_loaded = true;
    szs_cache_last_scan = 0;

    const u_nsec_t start_time = GetTimerNSec();

    char fbuf[PATH_MAX], buf[PATH_MAX];
    ccp path = PathCatPP(fbuf,sizeof(fbuf),szs_cache_dir,SZS_CACHE_FNAME);
    PRINT0("LoadSZSCache() %s\n",path);

    File_t F;
    enumError err = OpenFile(&F,true,path,FM_SILENT,0,0);
    if (err)
    {
	LogCacheActivity("!LOAD","Loading cache failed: %s",path);
	return err;
    }

    const typeof(parallel_count) saved_parallel_count = parallel_count;
    parallel_count = 0;

    while (fgets(buf,sizeof(buf)-1,F.f))
    {
	char *ptr = buf;
	while ( *ptr > 0 && *ptr <= ' ' )
	    ptr++;
	if ( *ptr == '#' || *ptr == '!' || !*ptr )
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

	char *fname = strchr(ptr,' ');
	if (!fname)
	    continue;

	const uint cslen = fname - ptr;
	if ( cslen != CHECKSUM_DB_SIZE && cslen != CHECKSUM_DB_SIZE+2 )
	    continue;

	while ( *fname > 0 && *fname <= ' ' )
	    fname++;

	char csum[CHECKSUM_DB_SIZE+3];
	memcpy(csum,ptr,cslen);
	csum[cslen] = 0;
	if ( cslen == CHECKSUM_DB_SIZE )
	    memcpy(csum+CHECKSUM_DB_SIZE,".y",3);
	if (!*fname)
	    continue;
	StoreSZSCache(fname,csum,false,0);
    }

    CloseFile(&F,0);
    if (opt_log_cache)
	LogCacheActivity("LOAD","%u entries in %s: %s",
		szs_cache.used,
		PrintTimerNSec6(0,0,GetTimerNSec()-start_time,0),
		path );

    parallel_count = saved_parallel_count;

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

enumError SaveSZSCache ( bool force )
{
    if ( parallel_count > 0 )
	return AppendSZSCache();

    if ( !szs_cache_dir || !szs_cache_dirty && !force )
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

//-----------------------------------------------------------------------------

enumError AppendSZSCache()
{
    if ( !szs_cache_dir || !szs_cache_append.used)
	return ERR_NOTHING_TO_DO;

    char path_buf[PATH_MAX];
    ccp path = PathCatPP(path_buf,sizeof(path_buf),szs_cache_dir,SZS_CACHE_FNAME);
    PRINT("AppendSZSCache() N=%u, %s\n",szs_cache.used,path);

    File_t F;
    enumError err = CreateFile(&F,true,path,FM_APPEND);
    if (err)
	return err;

    time_t tim = time(0);
    if ( tim >= F.st.st_mtime + 10 )
    {
	char tbuf[50];
	struct tm *tm = localtime(&tim);
	strftime(tbuf,sizeof(tbuf),"%F %T %z",tm);
	fprintf(F.f,"\n@APPEND = %s\n",tbuf);
    }

    for ( int i = 0; i < szs_cache_append.used; i++ )
    {
	const ParamFieldItem_t *it = szs_cache_append.field+i;
	fprintf(F.f,"%s %s\n",it->key,(ccp)it->data);
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
    ccp		checksum,	// DB checksum + ".x"; if NULL: load file and calc checksum
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

    ccp path = 0;
    char path_buf[PATH_MAX], oldpath_buf[PATH_MAX], checksum_buf[CHECKSUM_DB_SIZE+3];

    if (!checksum)
    {
	if (!fname)
	    return 0;

	path = PathCatPP(path_buf,sizeof(path_buf),szs_cache_dir,fname);

	szs_file_t szs;
	InitializeSZS(&szs);
	enumError err = LoadSZS(&szs,path,true,true,true);
	if ( err > ERR_WARNING || err == ERR_NOT_EXISTS || !IsCompressedFF(szs.fform_file) )
	{
	    ResetSZS(&szs);
	    return 0;
	}

	CreateSSXChecksumDBBySZS(checksum_buf,sizeof(checksum_buf),&szs);
	checksum = checksum_buf;
	ResetSZS(&szs);
    }

    bool found;
    ParamFieldItem_t *it = FindInsertParamField(&szs_cache,checksum,false,0,&found);
    if ( !found && parallel_count > 0 )
	it = FindInsertParamField(&szs_cache_append,checksum,false,0,&found);
    PRINT0("N=%d, found[%s]=%d\n",szs_cache.used,checksum,found);

    if (found)
    {
	DASSERT(it);
	ccp oldpath = PathCatPP(oldpath_buf,sizeof(oldpath_buf),szs_cache_dir,(ccp)it->data);

	struct stat st;
	if ( stat(oldpath,&st) || !S_ISREG(st.st_mode) )
	    return it;

	if (r_exists)
	    *r_exists = true;

	if ( parallel_count > 0 || !rename_file || !fname || !strcmp(fname,(ccp)it->data) )
	    return it;

	if (!path)
	    path = PathCatPP(path_buf,sizeof(path_buf),szs_cache_dir,fname);
	PRINT0("RENAME: %s -> %s\n",oldpath,path);
	if (rename(oldpath,path))
	    return it;

	FreeString(it->data);
	it->data = fname ? STRDUP(fname) : 0;
    }

    if (it)
    {
	PRINT("USE: %s\n",fname);
	it->data = STRDUP(fname);
    }
    szs_cache_dirty = true;
    return it;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ResetCheckCache ( check_cache_t *cc )
{
    if (cc)
    {
	ResetSZS(&cc->cache);
	memset(cc,0,sizeof(*cc));
    }
}

///////////////////////////////////////////////////////////////////////////////

static void fix_ext ( char *buf, uint bufsize, ccp ext )
{
    char *point = strrchr(buf,'.');
    if ( !point || strcmp(point,ext) )
    {
	if ( point && !strcmp(point,".szs") )
	    *point = 0;
	StringCat2S(buf,bufsize,buf,ext);
    }
}

//-----------------------------------------------------------------------------

bool CheckSZSCache
(
    // returns true if file found in cache (same as cc->found)

    check_cache_t	*cc,		// initialized by CheckSZSCache()
    szs_file_t		*szs,		// related SZS file
    cvp			data,		// data
    uint		size,		// size of 'data'
    file_format_t	fform,		// compression format
    ccp			ext		// wanted file extension including leading '.'
)
{
    ASSERT(cc);
    ASSERT(szs);
    ASSERT(ext);

    memset(cc,0,sizeof(*cc));
    cc->fform = fform;

    LoadSZSCache();
    CreateSSXChecksumDBByData(cc->csum,sizeof(cc->csum),data,size,fform);

    ccp dest_fname;
    if ( opt_cname && *opt_cname )
    {
	dest_fname = opt_cname;
	opt_cname = 0;
    }
    else
	dest_fname = szs->dest_fname ? szs->dest_fname : szs->fname;

    char path[PATH_MAX];
    StringCopyS(path,sizeof(path),dest_fname);
    fix_ext(path,sizeof(path),ext);
    dest_fname = path;

    bool exists;
    const ParamFieldItem_t *cit
	= StoreSZSCache( dest_fname, cc->csum, true, &exists );
//PRINT1("checklsum = %s, cit=%d, exists=%d\n",cc->csum,cit!=0,exists);

    ccp cache_fname = 0;
    if ( exists && cit )
    {
	cache_fname = PathAllocPP(szs_cache_dir,cit->data);
//PRINT1("CACHE FOUND[%s]: %s\n",cit->key,cache_fname);

	InitializeSZS(&cc->cache);

	if ( !LoadSZS(&cc->cache,cache_fname,false,true,true)
	    && cc->cache.cdata
	    && cc->cache.csize
	    && cc->cache.fform_file == fform )
	{
	    PRINT("CACHE LOADED, %zu bytes\n",cc->cache.csize);
	    //HEXDUMP16(0,0,cc->cache.cdata,32);
	    DecompressSZS(&cc->cache,false,0);
	    if ( cc->cache.size == size && !memcmp(data,cc->cache.data,cc->cache.size))
	    {
		LogCacheActivity("USE","%s %s",cit->key,(ccp)cit->data);
		FreeString(cache_fname);
		return cc->found = true;
	    }
	}
	else
	    LogCacheActivity("!USE","%s %s",cit->key,(ccp)cit->data);
	ResetSZS(&cc->cache);
    }

    if (cit)
    {
	if (cache_fname)
	    StringCopyS(path,sizeof(path),cache_fname);
	else
	    PathCatPP(path,sizeof(path),szs_cache_dir,cit->data);
	fix_ext(path,sizeof(path),ext);

	FreeString(szs->cache_fname);
	szs->cache_fname = STRDUP(path);
//PRINT1("szs->cache_fname = %s\n",szs->cache_fname);
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool CheckSZSCacheSZS
(
    // returns true if file found in cache (same as cc->found)

    szs_file_t		*szs,		// related SZS file
    file_format_t	fform,		// compression format
    ccp			ext,		// wanted file extension including leading '.'
    bool		rm_uncompressed	// true: remove uncompressed data if cache is used
)
{
    ASSERT(szs);
    check_cache_t cc;
    if (CheckSZSCache(&cc,szs,szs->data,szs->size,fform,ext))
    {
	FREE(szs->cdata);
	szs->cdata = cc.cache.cdata;
	szs->csize = cc.cache.csize;
	szs->cache_used = true;
	cc.cache.cdata = 0;
	cc.cache.csize = 0;
	ResetCheckCache(&cc);
	if (rm_uncompressed)
	    ClearUncompressedSZS(szs);
	return true;
    }

    ResetCheckCache(&cc);
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

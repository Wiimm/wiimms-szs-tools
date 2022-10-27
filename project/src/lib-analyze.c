
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

#include "lib-analyze.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyze_param_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void PrintHeaderAP
(
    analyze_param_t	*ap,		// valid structure
    ccp			analyze		// analyzed object
)
{
    DASSERT(ap);
    DASSERT(analyze);

    char buf[PATH_MAX+100];
    PrintEscapedString(buf,sizeof(buf),ap->fname,-1,CHMD_UTF8,'"',0);

    PrintScriptVars(&ap->ps,1,
	"file=\"%s\"\n"
	"file_type=\"%s\"\n"
	"analyze=\"%s\"\n"
	,buf
	,GetNameFF(ap->szs.fform_file,ap->szs.fform_arch)
	,analyze
	);
}

///////////////////////////////////////////////////////////////////////////////

void PrintFooterAP
(
    analyze_param_t	*ap,		// valid structure
    bool		valid,		// true if source is valid
    u_usec_t		duration_usec,	// 0 or duration of analysis
    ccp			warn_format,	// NULL or format for warn message
    ...					// arguments for 'warn_format'
)
{
    DASSERT(ap);

    char buf[1000];

    PrintScriptVars(&ap->ps,0,"valid=%d\n",valid);
    if ( warn_format && *warn_format )
    {
	va_list arg;
	va_start(arg,warn_format);
	vsnprintf(buf,sizeof(buf),warn_format,arg);
	va_end(arg);
	PrintScriptVars(&ap->ps,0,"warning=\"%s\"\n",buf);
    }
    else
	PrintScriptVars(&ap->ps,0,"warning=\"\"\n");

    if (duration_usec)
	PrintScriptVars(&ap->ps,0,"duration_usec=%llu\n",duration_usec);
    PutScriptVars(&ap->ps,2,0);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			analyze_szs_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeAnalyzeSZS ( analyze_szs_t * as )
{
  #ifdef DEBUG
    static bool done = false;
    if (!done)
    {
	done = true;
	TRACE_SIZEOF(analyze_szs_t);
	TRACE_SIZEOF(lex_info_t);
	TRACE_SIZEOF(slot_ana_t);
	TRACE_SIZEOF(slot_info_t);
	TRACE_SIZEOF(kmp_finish_t);
	TRACE_SIZEOF(kmp_usedpos_t);
	TRACE_SIZEOF(szs_have_t);
	TRACE_SIZEOF(szs_have_t);
	TRACE_SIZEOF(have_file_mode_t);
	TRACE_SIZEOF(szs_special_t);
	TRACE_SIZEOF(kmp_special_t);
	TRACE("-\n");
    }
 #endif

    DASSERT(as);
    memset(as,0,sizeof(*as));

    InitializeLexInfo(&as->lexinfo);
    InitializeFinishLine(&as->kmp_finish);
    InitializeUsedPos(&as->used_pos);

    as->ckpt0_count	= -1;		// number of LC in CKPT
    as->lap_count	= 3;		// STGI lap counter
    as->speed_factor	= 1.0;		// STGI speed factor
}

///////////////////////////////////////////////////////////////////////////////

void ResetAnalyzeSZS ( analyze_szs_t * as )
{
    if (as)
    {
	ResetLexInfo(&as->lexinfo);
	InitializeAnalyzeSZS(as);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AnalyzeSZS
(
    analyze_szs_t	*as,		// result
    bool		init_sa,	// true: init 'as', false: reset 'as'
    szs_file_t		*szs,		// SZS file to analysze
    ccp			fname		// NULL or fname for slot analysis
)
{
    //--- setup

    const u_usec_t start_usec = GetTimerUSec();

    analyze_szs_t as0;
    if (!as)
    {
	as = &as0;
	init_sa = true;
    }

    DASSERT(as);
    if (init_sa)
	InitializeAnalyzeSZS(as);
    else
	ResetAnalyzeSZS(as);

    if (!szs)
	return;

    char *ct_dest = as->ct_attrib;
    char *ct_end  = as->ct_attrib + sizeof(as->ct_attrib);


    //--- checksums

    sha1_size_t sha1_data;
    SHA1(szs->data,szs->size,sha1_data.hash);
    Sha1Bin2Hex(as->sha1_szs,sha1_data.hash);
    memcpy(as->sha1_szs_norm,as->sha1_szs,sizeof(as->sha1_szs_norm));

    sha1_data.size = htonl(szs->size);
    CreateSSChecksumDB(as->db64,sizeof(as->db64),&sha1_data);
    FindSpecialFilesSZS(szs,false);


    //--- scan LEX

    InitializeLexInfo(&as->lexinfo);

    if (szs->course_lex_data)
    {
	lex_t lex;
	InitializeLEX(&lex);
	lex.check_only = true;
	ScanLEX(&lex,false,szs->course_lex_data,szs->course_lex_size);
	szs->have.lex_sect = lex.have_sect;
	szs->have.lex_feat = lex.have_feat;
	SetupLexInfo(&as->lexinfo,&lex);
	ResetLEX(&lex);
    }


    //--- scan slots

    AnalyzeSlot(&as->slotana,szs);
    if (fname)
	AnalyzeSlotByName(&as->slotinfo,true,MemByString(fname));
    if (as->slotana.mandatory_slot[0])
	AnalyzeSlotAttrib(&as->slotinfo,false,MemByString(as->slotana.mandatory_slot));
    FinalizeSlotInfo(&as->slotinfo,true);
    if (*as->slotinfo.slot_attrib)
	ct_dest = StringCat2E(ct_dest,ct_end,",",as->slotinfo.slot_attrib);


    //--- scan KMP

    bool valid_track = true; // use temp var because of early 'return'

    if (szs->course_kmp_data)
    {
	SHA1(szs->course_kmp_data,szs->course_kmp_size,sha1_data.hash);
	Sha1Bin2Hex(as->sha1_kmp,sha1_data.hash);
	memcpy(as->sha1_kmp_norm,as->sha1_kmp,sizeof(as->sha1_kmp_norm));
	as->sha1_kmp_slot = as->sha1_kmp_norm_slot
		= GetSha1Slot(SHA1T_KMP,sha1_data.hash);

	kmp_t kmp;
	InitializeKMP(&kmp);
	kmp.check_only = true;
	kmp.lexinfo = &as->lexinfo;
	const enumError err
	    = ScanKMP(&kmp,false,szs->course_kmp_data,szs->course_kmp_size,0);
	if ( err <= ERR_WARNING )
	{
	    if (kmp.stgi)
	    {
		kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp.stgi;
		as->lap_count = stgi->lap_count;
		as->speed_mod = stgi->speed_mod;
		stgi->lap_count = 3;
		stgi->speed_mod = 0;

		SHA1(szs->data,szs->size,sha1_data.hash);
		Sha1Bin2Hex(as->sha1_szs_norm,sha1_data.hash);

		SHA1(szs->course_kmp_data,szs->course_kmp_size,sha1_data.hash);
		Sha1Bin2Hex(as->sha1_kmp_norm,sha1_data.hash);
		as->sha1_kmp_norm_slot = GetSha1Slot(SHA1T_KMP,sha1_data.hash);

		stgi->lap_count = as->lap_count;
		stgi->speed_mod = as->speed_mod;
	    }

	    CheckFinishLine(&kmp,&as->kmp_finish);
	    CheckWarnKMP(&kmp,&as->used_pos);
	    szs->warn_bits |= kmp.warn_bits;

	    const uint n_ckpt = kmp.dlist[KMP_CKPT].used;
	    if (n_ckpt)
	    {
		const kmp_ckpt_entry_t *ckpt
		    = (kmp_ckpt_entry_t*)kmp.dlist[KMP_CKPT].list;
		uint i;
		as->ckpt0_count = 0;
		for ( i = 0; i < n_ckpt; i++, ckpt++ )
		    if (!ckpt->mode)
			as->ckpt0_count++;
	    }

	    const kmp_stgi_entry_t *stgi = (kmp_stgi_entry_t*)kmp.dlist[KMP_STGI].list;
	    if ( kmp.dlist[KMP_STGI].used > 0 )
	    {
		as->lap_count = stgi->lap_count;
		if (!szs->is_arena)
		{
		    if ( as->ckpt0_count > 1 )
		    {
			if ( as->lap_count == 1 )
			    ct_dest = StringCopyE(ct_dest,ct_end,",1lap");
			ct_dest = snprintfE(ct_dest,ct_end,",%ulc",as->ckpt0_count);
		    }
		    else if ( as->lap_count != 0 && as->lap_count != 3 )
			ct_dest = snprintfE(ct_dest,ct_end,",%ulap%s",
				    as->lap_count, as->lap_count == 1 ? "" : "s" );
		}

		if (stgi->speed_mod)
		{
		    as->speed_mod = stgi->speed_mod;
		    as->speed_factor = SpeedMod2float(stgi->speed_mod);
		    char buf[20];
		    snprintf(buf,sizeof(buf),",x%4.2f",as->speed_factor);
		    char *ptr = buf + strlen(buf) - 1;
		    while ( *ptr == '0' )
			*ptr-- = 0;
		    if ( *ptr == '.' )
			*ptr = 0;
		    if ( strcmp(buf,",x0") && strcmp(buf,",x1") )
			ct_dest = StringCopyE(ct_dest,ct_end,buf);
		}
	    }
	    else if ( !szs->is_arena && as->ckpt0_count > 1 )
		ct_dest = snprintfE(ct_dest,ct_end,",%ulc",as->ckpt0_count);

	    if ( as->kmp_finish.n_ktpt_m > 1 )
		ct_dest = StringCopyE(ct_dest,ct_end,",2ktpt");

	    kmp_ana_pflag_t ap;
	    AnalysePFlagScenarios(&ap,&kmp,GMD_M_DEFAULT);
	    snprintf(as->gobj_info,sizeof(as->gobj_info),"%d %d %d %d",
		    ap.ag.n_gobj, ap.n_version, ap.n_res_std, ap.n_res_ext );
	    ResetPFlagScenarios(&ap);
	}

	DetectSpecialKMP(&kmp,szs->have.kmp);
	if (  szs->have.kmp[HAVEKMP_WOODBOX_HT]
	   || szs->have.kmp[HAVEKMP_MUSHROOM_CAR]
	   || szs->have.kmp[HAVEKMP_PENGUIN_POS]
	   || szs->have.kmp[HAVEKMP_EPROP_SPEED]
	   || szs->have.kmp[HAVEKMP_GOOMBA_SIZE]
	)
	{
	    ct_dest = StringCopyE(ct_dest,ct_end,",gobj");
	}

	if (  szs->have.kmp[HAVEKMP_X_PFLAGS]
	   || szs->have.kmp[HAVEKMP_X_COND]
	   || szs->have.kmp[HAVEKMP_X_DEFOBJ]
	   || szs->have.kmp[HAVEKMP_X_RANDOM]
	)
	{
	    ct_dest = StringCopyE(ct_dest,ct_end,",xpf");
	}

	if (szs->have.kmp[HAVEKMP_COOB_R])
	    ct_dest = StringCopyE(ct_dest,ct_end,",coob-r");
	if (szs->have.kmp[HAVEKMP_COOB_K])
	    ct_dest = StringCopyE(ct_dest,ct_end,",coob-k");
	if (szs->have.kmp[HAVEKMP_UOOB])
	    ct_dest = StringCopyE(ct_dest,ct_end,",uoob");

	ResetKMP(&kmp);
    }
    else
	valid_track = false;


    if ( szs->have.szs[HAVESZS_ITEM_SLOT_TABLE] >= HFM_MODIFIED )
	ct_dest = StringCopyE(ct_dest,ct_end,",itemslot");

    if ( szs->have.szs[HAVESZS_OBJFLOW] >= HFM_MODIFIED )
	ct_dest = StringCopyE(ct_dest,ct_end,",objflow");

    if (  szs->have.szs[HAVESZS_GHT_ITEM]	>= HFM_MODIFIED
       || szs->have.szs[HAVESZS_GHT_ITEM_OBJ]	>= HFM_MODIFIED
       || szs->have.szs[HAVESZS_GHT_KART]	>= HFM_MODIFIED
       || szs->have.szs[HAVESZS_GHT_KART_OBJ]	>= HFM_MODIFIED
    )
    {
	ct_dest = StringCopyE(ct_dest,ct_end,",geohit");
    }

    if ( szs->have.szs[HAVESZS_MINIGAME] >= HFM_MODIFIED )
	ct_dest = StringCopyE(ct_dest,ct_end,",minigame");

    if ( szs->have.szs[HAVESZS_AIPARAM_BAA] >= HFM_ORIGINAL
	|| szs->have.szs[HAVESZS_AIPARAM_BAS] >= HFM_ORIGINAL
    )
    {
	ct_dest = StringCopyE(ct_dest,ct_end,",aiparam");
    }


    //--- scan KCL

    if (szs->course_kcl_data)
    {
	SHA1(szs->course_kcl_data,szs->course_kcl_size,sha1_data.hash);
	Sha1Bin2Hex(as->sha1_kcl,sha1_data.hash);
	as->sha1_kcl_slot = GetSha1Slot(SHA1T_KCL,sha1_data.hash);
    }
    else
	valid_track = false;


    //--- course model (course_model.brres, course_d_model.brres)

    if (szs->course_model_data)
    {
	SHA1(szs->course_model_data,szs->course_model_size,sha1_data.hash);
	Sha1Bin2Hex(as->sha1_course,sha1_data.hash);
    }
    else if (szs->course_d_model_data)
    {
	SHA1(szs->course_d_model_data,szs->course_d_model_size,sha1_data.hash);
	Sha1Bin2Hex(as->sha1_course,sha1_data.hash);
    }
    else
	valid_track = false;


    //--- vrcorn (vrcorn_model.brres)

    if (szs->vrcorn_model_data)
    {
	SHA1(szs->vrcorn_model_data,szs->vrcorn_model_size,sha1_data.hash);
	Sha1Bin2Hex(as->sha1_vrcorn,sha1_data.hash);
    }
//    else
//	valid_track = false;


    //--- minimap (map_model.brres)

    if (szs->map_model_data)
    {
	SHA1(szs->map_model_data,szs->map_model_size,sha1_data.hash);
	Sha1Bin2Hex(as->sha1_minimap,sha1_data.hash);
	as->sha1_minimap_slot = GetSha1Slot(SHA1T_MAP,sha1_data.hash);
    }
    else
	szs->warn_bits |= 1 << WARNSZS_NO_MINIMAP;


    //--- missing and modified files

    ccp miss = GetStatusMissedFile(szs->missed_file);
    if ( miss && *miss )
	ct_dest = StringCat2E(ct_dest,ct_end,",miss=",miss);

    miss = GetStatusMissedFile(szs->modified_file);
    if ( miss && *miss )
	ct_dest = StringCat2E(ct_dest,ct_end,",mod=",miss);


    //--- finalize ct_attrib by "lex" and "warn"

    if (szs->course_lex_data)
	ct_dest = StringCopyE(ct_dest,ct_end,",lex");

    if (szs->warn_bits)
	ct_dest = StringCat2E(ct_dest,ct_end,",warn=",GetWarnSZSNames(szs->warn_bits,'+'));

    szs->have.valid	= true;
    as->have		= szs->have;
    as->valid_track	= valid_track;
    as->duration_usec	= GetTimerUSec() - start_usec;
}

///////////////////////////////////////////////////////////////////////////////

enumError ExecAnalyzeSZS ( analyze_param_t *ap )
{
    DASSERT(ap);

    const u_usec_t start_usec = GetTimerUSec();


    //--- analyse szs

    szs_file_t *szs = &ap->szs;
    DASSERT(szs);

    analyze_szs_t as;
    InitializeAnalyzeSZS(&as);
    AnalyzeSZS(&as,false,szs,ap->fname);

    uint name_attrib_len = 0;
    ccp name_attrib = strrchr(ap->fname,'[');
    if (name_attrib)
    {
	name_attrib++;
	ccp end = strrchr(name_attrib,']');
	if (end)
	{
	    name_attrib_len = end - name_attrib;
	    for ( ccp ptr = name_attrib; ptr < end; )
	    {
		ccp sep = strchr(ptr,',');
		if (!sep)
		    sep = end;
		switch ( sep-ptr )
		{
		 case 4:
		    if (!memcmp(ptr,"edit",4)) szs->have.attrib |= 1 << HAVEATT_EDIT;
		    break;

		 case 5:
		    if (!memcmp(ptr,"cheat",5)) szs->have.attrib |= 1 << HAVEATT_CHEAT;
		    break;

		 case 7:
		    if (!memcmp(ptr,"reverse",7)) szs->have.attrib |= 1 << HAVEATT_REVERSE;
		    break;
		}
		ptr = sep + 1;
	    }
	}
    }
    else
	name_attrib = EmptyString;


    ccp texture_info = 0;
    PrepareCheckTextureSZS(szs);
    CheckTextureRefSZS(szs,&texture_info);

    const u_usec_t duration_usec = GetTimerUSec() - start_usec;


    //--- print result

    PrintHeaderAP(ap,"track");
    PrintScriptVars(&ap->ps,0,
	"size=%zd\n"
	 "db64=\"%s\"\n"
	 "sha1=\"%s\"\n"
	 "sha1_norm=\"%s\"\n"
	 "sha1_kcl=\"%s\"\n"
	 "sha1_kmp=\"%s\"\n"
	 "sha1_kmp_norm=\"%s\"\n"
	 "sha1_course=\"%s\"\n"
	 "sha1_vrcorn=\"%s\"\n"
	 "sha1_minimap=\"%s\"\n"
	 "sha1_kcl_slot=%u\n"
	 "sha1_kmp_slot=%u\n"
	 "sha1_kmp_norm_slot=%u\n"
	 "sha1_minimap_slot=%u\n"
	"valid_track=%u\n"
	"is_arena=\"%u %s\"\n"
	"n_ckpt0=%d\n"
	"lap_count=%d\n"
	"speed_factor=%5.3f\n"
	"slot_info=\"%s\"\n"
	"slot_attributes=\"%s\"\n"
	"race_slot=\"%u %s\"\n"
	"arena_slot=\"%u %s\"\n"
	"edit_slot=\"%u %s\"\n"
	"music_index=\"%u %s\"\n"
	 "itempos_factors=\"%5.3f %5.3f %5.3f\"\n"
	 "used_x_pos=\"%u=%s %3.2f %3.2f %3.2f %3.2f\"\n"
	 "used_y_pos=\"%u=%s %3.2f %3.2f %3.2f %3.2f\"\n"
	 "used_z_pos=\"%u=%s %3.2f %3.2f %3.2f %3.2f\"\n"
	 "used_pos_suggest=\"%s\"\n"
	"ktpt2=\"%s %4.2f %4.2f\"\n"
	"gobj=\"%s\"\n"
	"missed_subfiles=\"%s\"\n"
	"modified_subfiles=\"%s\"\n"
	"special_files=\"%s\"\n"
	"original_files=\"%s\"\n"
	"modified_files=\"%s\"\n"
	"special_attrib=\"%s\"\n"
	"special_kmp=\"%s\"\n"
	"lex_sections=\"%s\"\n"
	"lex_features=\"%s\"\n"
	"warn=\"%u=%s\"\n"
	"ct_attributes=\"%s\"\n"
	"name_attributes=\"%.*s\"\n"

	,szs->size
	 ,as.db64
	 ,as.sha1_szs
	 ,as.sha1_szs_norm
	 ,as.sha1_kcl
	 ,as.sha1_kmp
	 ,as.sha1_kmp_norm
	 ,as.sha1_course
	 ,as.sha1_vrcorn
	 ,as.sha1_minimap
	 ,as.sha1_kcl_slot
	 ,as.sha1_kmp_slot
	 ,as.sha1_kmp_norm_slot
	 ,as.sha1_minimap_slot
	,as.valid_track
	,szs->is_arena
	,is_arena_name[szs->is_arena]
	,as.ckpt0_count
	,as.lap_count
	,as.speed_factor
	,as.slotana.slot_info
	,as.slotinfo.slot_attrib
	,as.slotinfo.race_slot,as.slotinfo.race_info
	,as.slotinfo.arena_slot,as.slotinfo.arena_info
	,as.slotinfo.edit_slot,as.slotinfo.edit_info
	,as.slotinfo.music_index,as.slotinfo.music_info
	 ,as.lexinfo.item_factor.x,as.lexinfo.item_factor.y,as.lexinfo.item_factor.z
	 ,as.used_pos.orig.rating[0]
		,WarnLevelNameLo[as.used_pos.orig.rating[0]]
	 ,as.used_pos.orig.min.x,as.used_pos.orig.max.x
		,fabsf(as.used_pos.orig.max.x-as.used_pos.orig.min.x)
		,(as.used_pos.orig.min.x+as.used_pos.orig.max.x)/2
	 ,as.used_pos.orig.rating[1],WarnLevelNameLo[as.used_pos.orig.rating[1]]
	 ,as.used_pos.orig.min.y,as.used_pos.orig.max.y
		,fabsf(as.used_pos.orig.max.y-as.used_pos.orig.min.y)
		,(as.used_pos.orig.min.y+as.used_pos.orig.max.y)/2
	 ,as.used_pos.orig.rating[2],WarnLevelNameLo[as.used_pos.orig.rating[2]]
	 ,as.used_pos.orig.min.z,as.used_pos.orig.max.z
		,fabsf(as.used_pos.orig.max.z-as.used_pos.orig.min.z)
		,(as.used_pos.orig.min.z+as.used_pos.orig.max.z)/2
	 ,GetUsedPosObjSuggestion(as.used_pos.suggest,true,"")
	,as.kmp_finish.warn ? "warn" : as.kmp_finish.hint ? "hint" : "ok"
	,as.kmp_finish.distance
	,as.kmp_finish.dir_delta
	,as.gobj_info
	,GetStatusMissedFile(szs->missed_file)
	,GetStatusMissedFile(szs->modified_file)
	,CreateSpecialFileInfo(szs,~(1<<HFM_NONE),true,"")
	,CreateSpecialFileInfo(szs,1<<HFM_ORIGINAL,true,"")
	,CreateSpecialFileInfo(szs,1<<HFM_MODIFIED,true,"")
	,CreateSpecialInfoAttrib(szs->have.attrib,true,"")
	,CreateSpecialInfoKMP(szs->have.kmp,true,"")
	,CreateSectionInfoLEX(szs->have.lex_sect,true,"")
	,CreateFeatureInfoLEX(szs->have.lex_feat,true,"")
	,szs->warn_bits,GetWarnSZSNames(szs->warn_bits,' ')
	,as.ct_attrib+1
	,name_attrib_len,name_attrib
	);

    if (texture_info)
	PrintScriptVars(&ap->ps,0,"texture_hack=\"%s\"\n",texture_info);

    if ( long_count > 0 )
    {
	const uint n = szs->special_file ? szs->special_file->used : 0;
	PrintScriptVars(&ap->ps,0,"subfile_n=%u\n",n);

	uint i;
	ParamFieldItem_t *ptr = szs->special_file->field;
	for (i = 0; i < n; i++, ptr++ )
	{
	    sha1_hex_t hex;
	    Sha1Bin2Hex(hex,(u8*)ptr->data);
	    PrintScriptVars(&ap->ps,0,"subfile_%u=\"%u %s %s\"\n",
		    i, ptr->num, hex, ptr->key );
	}
    }

    PrintFooterAP(ap,true,duration_usec,0);
    ResetAnalyzeSZS(&as);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    ExecAnalyzeLECODE()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExecAnalyzeLECODE ( analyze_param_t *ap )
{
    DASSERT(ap);

    const u_usec_t start_usec = GetTimerUSec();

    le_analyze_t ana;
    AnalyzeLEBinary(&ana,ap->szs.data,ap->szs.size);

    const u_usec_t duration_usec = GetTimerUSec() - start_usec;


    //--- print result

    PrintHeaderAP(ap,"lecode");
    PrintScriptVars(&ap->ps,0,
	"phase=%d\n"
	"version=%d\n"
	"build=%d\n"
	"region=\"%s\"\n"
	"debug=%d\n"
	"header_size=%d\n"
	"file_size=%d\n"
	"creation_time=\"%u, %s\"\n"
	"edit_time=\"%u, %s\"\n"
	"required_szs_tool=\"%u, %s\"\n"
	"edit_by_szs_tool=\"%u, %s\"\n"

	,ana.head->phase
	,ana.header_vers
	,ntohl(ana.head->build_number)
	,ana.head->region == 'P' ? "PAL"
	:ana.head->region == 'E' ? "USA"
	:ana.head->region == 'J' ? "Japan"
	:ana.head->region == 'K' ? "Korea"
	: "?"
	,ana.head->debug == 'D'
	,ana.header_size
	,ntohl(ana.head->file_size)
	,ana.creation_time
	,ana.creation_time ? PrintTimeByFormat("%F %T %Z",ana.creation_time) : "-"
	,ana.edit_time
	,ana.edit_time ? PrintTimeByFormat("%F %T %Z",ana.edit_time) : "-"
	,ana.szs_version
	,ana.szs_version ? DecodeVersion(ana.szs_version) : "-"
	,ana.edit_version
	,ana.edit_version ? DecodeVersion(ana.edit_version) : "-"
	);

    PrintFooterAP(ap,ana.valid>0,duration_usec,0);
    ResetLEAnalyze(&ana);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////

void CalcHaveSZS ( szs_file_t * szs )
{
    DASSERT(szs);
    if ( szs->data && !szs->have.valid && CanBeATrackSZS(szs) )
    {
	szs_file_t temp;
	CopySZS(&temp,true,szs);
	temp.check_only = true;
	analyze_szs_t as;
	AnalyzeSZS(&as,true,&temp,temp.fname);
	szs->have = temp.have;
	ResetAnalyzeSZS(&as);
	ResetSZS(&temp);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////



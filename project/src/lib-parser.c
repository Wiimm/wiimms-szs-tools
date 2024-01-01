
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

#include "lib-std.h"
#include <math.h>
#include <stddef.h>

//
///////////////////////////////////////////////////////////////////////////////
///////////////			conversion helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

Var_t var_unset = {0};

///////////////////////////////////////////////////////////////////////////////

bool GetBoolV ( const Var_t * var )
{
    if (var)
	switch(var->mode)
	{
	    case VAR_UNSET:
		return false;

	    case VAR_INT:
		return var->i != 0;

	    case VAR_DOUBLE:
		return var->d != 0.0;

	    case VAR_VECTOR:
		return var->x || var->y || var->z;

	    case VAR_STRING:
		return var->str_len > 0;
	}
    return false;
}

//////////////////////////////////////////////////////////////////////////////

int GetIntV ( const Var_t * var )
{
    if (var)
	switch(var->mode)
	{
	    case VAR_UNSET:
		return 0;

	    case VAR_INT:
		return var->i;

	    case VAR_DOUBLE:
	    case VAR_VECTOR:
	    {
		const double d = floor ( var->d + 0.5 );
		return d < INT_MIN ? INT_MIN : d > INT_MAX ? INT_MAX : d;
	    }

	    case VAR_STRING:
		return str2l(var->str,0,10);
	}
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

double GetDoubleV ( const Var_t * var )
{
    if (var)
	switch(var->mode)
	{
	    case VAR_UNSET:
		return 0.0;

	    case VAR_INT:
		return var->i;

	    case VAR_DOUBLE:
	    case VAR_VECTOR:
		return var->d;

	    case VAR_STRING:
		return var->str_len ? strtod_comma(var->str,0) : 0.0;
	}
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////

double GetXDoubleV ( const Var_t * var )
{
    if (var)
	switch(var->mode)
	{
	    case VAR_UNSET:
		return 0.0;

	    case VAR_INT:
		return var->i;

	    case VAR_DOUBLE:
		return var->d;

	    case VAR_VECTOR:
		return var->x;

	    case VAR_STRING:
		return var->str_len ? strtod_comma(var->str,0) : 0.0;
	}
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////

double GetYDoubleV ( const Var_t * var )
{
    if (var)
	switch(var->mode)
	{
	    case VAR_UNSET:
		return 0.0;

	    case VAR_INT:
		return var->i;

	    case VAR_DOUBLE:
		return var->d;

	    case VAR_VECTOR:
		return var->y;

	    case VAR_STRING:
		return var->str_len ? strtod_comma(var->str,0) : 0.0;
	}
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////

double GetZDoubleV ( const Var_t * var )
{
    if (var)
	switch(var->mode)
	{
	    case VAR_UNSET:
		return 0.0;

	    case VAR_INT:
		return var->i;

	    case VAR_DOUBLE:
		return var->d;

	    case VAR_VECTOR:
		return var->z;

	    case VAR_STRING:
		return var->str_len ? strtod_comma(var->str,0) : 0.0;
	}
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////

double3	GetVectorV ( const Var_t * var )
{
    double3 d;
    if (!var)
	d.x = d.y = d.z = 0.0;
    else
	switch(var->mode)
	{
	    case VAR_UNSET:
		d.x = d.y = d.z = 0.0;
		break;

	    case VAR_INT:
		d.x = d.y = d.z = var->i;
		break;

	    case VAR_DOUBLE:
		d.x = d.y = d.z = var->d;
		break;

	    case VAR_VECTOR:
		return var->d3;

	    case VAR_STRING:
		{
		    DEFINE_VAR(temp);
		    ScanVectorExpr(var->str,var->name,&temp);
		    DASSERT( temp.mode == VAR_VECTOR );
		    return temp.d3;
		}
	}
    return d;
}

///////////////////////////////////////////////////////////////////////////////

float3 GetVectorFV ( const Var_t * var )
{
    float3 f;
    if (!var)
	f.x = f.y = f.z = 0.0;
    else
	switch(var->mode)
	{
	    case VAR_UNSET:
		f.x = f.y = f.z = 0.0;
		break;

	    case VAR_INT:
		f.x = f.y = f.z = var->i;
		break;

	    case VAR_DOUBLE:
		f.x = f.y = f.z = var->d;
		break;

	    case VAR_VECTOR:
		f.x = var->x;
		f.y = var->y;
		f.z = var->z;
		break;

	    case VAR_STRING:
		{
		    DEFINE_VAR(temp);
		    ScanVectorExpr(var->str,var->name,&temp);
		    DASSERT( temp.mode == VAR_VECTOR );
		    f.x  = temp.x;
		    f.y  = temp.y;
		    f.z  = temp.z;
		}
	}
    return f;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// move content;   src can be NULL or ==dest

void MoveDataV ( Var_t *dest, Var_t * src )
{
    DASSERT(dest);
    FreeV(dest);

    if (!src)
	dest->mode = VAR_UNSET;
    else if ( src != dest )
    {
	memcpy( &dest->mode, &src->mode, sizeof(*dest)-offsetof(Var_t,mode) );
	src->mode = VAR_UNSET;
    }
}

///////////////////////////////////////////////////////////////////////////////

void MoveDataInitV ( Var_t *dest, Var_t * src )
{
    DASSERT(dest);

    if (!src)
	dest->mode = VAR_UNSET;
    else if ( src != dest )
    {
	memcpy( &dest->mode, &src->mode, sizeof(*dest)-offsetof(Var_t,mode) );
	src->mode = VAR_UNSET;
    }
}

///////////////////////////////////////////////////////////////////////////////
void MoveAllV ( Var_t *dest, Var_t * src )
{
    DASSERT(dest);
    FreeV(dest);

    if (!src)
	dest->mode = VAR_UNSET;
    else if ( src != dest )
    {
	memcpy(dest,src,sizeof(*dest));
	src->name = 0;
	src->mode = VAR_UNSET;
    }
}

///////////////////////////////////////////////////////////////////////////////

void MoveAllInitV ( Var_t *dest, Var_t * src )
{
    DASSERT(dest);

    if (!src)
	dest->mode = VAR_UNSET;
    else if ( src != dest )
    {
	memcpy(dest,src,sizeof(*dest));
	src->name = 0;
	src->mode = VAR_UNSET;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// assign strings

uint AssignStringVV ( Var_t * var, const Var_t * src )
{
    DASSERT(var);

    DEFINE_VAR(temp);
    Var_t *res = GetStringV(src,&temp);
    DASSERT( res && res->mode == VAR_STRING );
    AssignStringVS(var,res->str,res->str_len);
    FreeV(&temp);
    return var->str_len;
}

///////////////////////////////////////////////////////////////////////////////

uint AppendStringVV ( Var_t * var, const Var_t * src )
{
    DASSERT(var);

    DEFINE_VAR(temp);
    Var_t *res = GetStringV(src,&temp);
    DASSERT( res && res->mode == VAR_STRING );
    AppendStringVS(var,res->str,res->str_len);
    FreeV(&temp);
    return var->str_len;
}

///////////////////////////////////////////////////////////////////////////////

uint AssignStringVS ( Var_t * var, ccp str, int len )
{
    DASSERT(var);

    SetModeStringV(var);
    var->str_len = 0;

    if (str)
    {
	if ( len < 0 )
	    len = strlen(str);
	if ( len > 0 )
	{
	    char *old = 0;
	    if ( len > var->str_size )
	    {
		old = var->str;
		var->str_size = len;
		var->str = MALLOC(var->str_size+1);
	    }
	    DASSERT( len <= var->str_size);
	    memcpy(var->str,str,len);
	    var->str[len] = 0;
	    var->str_len = len;
	    if (old)
		FREE(old);
	}
    }
    return var->str_len;
}

///////////////////////////////////////////////////////////////////////////////

uint AssignStringVS2 ( Var_t * var, ccp str1, int len1, ccp str2, int len2 )
{
    DASSERT(var);

    if ( !str1 || !len1 ) return AssignStringVS(var,str2,len2);
    if ( !str2 || !len2 ) return AssignStringVS(var,str1,len1);

    SetModeStringV(var);
    var->str_len = 0;

    if ( len1 < 0 ) len1 = strlen(str1);
    if ( len2 < 0 ) len2 = strlen(str2);
    const int total = len1 + len2;

    if ( total > 0 )
    {
	char *old = 0;
	if ( total > var->str_size )
	{
	    old = var->str;
	    var->str_size = total;
	    var->str = MALLOC(var->str_size+1);
	}
	DASSERT( total <= var->str_size);
	memcpy(var->str,str1,len1);
	memcpy(var->str+len1,str2,len2);
	var->str[total] = 0;
	var->str_len = total;
	if (old)
	    FREE(old);
    }

    return var->str_len;
}

///////////////////////////////////////////////////////////////////////////////

uint AppendStringVS ( Var_t * var, ccp str, int len )
{
    DASSERT(var);

    if ( var->mode != VAR_STRING )
	return AssignStringVS(var,str,len);

    if (str)
    {
	if ( len < 0 )
	    len = strlen(str);
	if ( len > 0 )
	{
	    char *old = 0;
	    const uint total = var->str_len + len;
	    if ( total > var->str_size )
	    {
		old = var->str;
		var->str_size = total + 20;
		var->str = MALLOC(var->str_size+1);
		memcpy(var->str,old,var->str_len);
	    }
	    DASSERT( total <= var->str_size);
	    memcpy(var->str+var->str_len,str,len);
	    var->str_len = total;
	    var->str[total] = 0;
	    if (old)
		FREE(old);
	}
    }
    return var->str_len;
}

///////////////////////////////////////////////////////////////////////////////

uint AppendStringVC  ( Var_t * var, char ch, int repeat )
{
    DASSERT(var);

    SetModeStringV(var);
    if ( repeat > 0 )
    {
	const uint total = var->str_len + repeat;
	if ( total > var->str_size )
	{
	    var->str_size = total + 20;
	    var->str = REALLOC(var->str,var->str_size+1);
	}
	DASSERT( total <= var->str_size);
	char *dest = var->str+var->str_len;
	while ( repeat-- > 0 )
	    *dest++ = ch;
	*dest = 0;
	var->str_len = total;
	DASSERT( dest - var->str == var->str_len );
    }
    return var->str_len;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * GetStringV  ( const Var_t * var, Var_t * temp )
{
    DASSERT(temp);

    if (!var)
    {
	SetModeStringV(temp);
	temp->str_len = 0;
	return temp;
    }

    if ( var->mode == VAR_STRING )
	return (Var_t*)var;

    int len = 0;
    char buf[100];

    switch (var->mode)
    {
	case VAR_INT:
	    len = snprintf(buf,sizeof(buf),"%lld",var->i);
	    break;

	case VAR_DOUBLE:
	    len = snprintf(buf,sizeof(buf),"%5.3f",var->d);
	    break;

	case VAR_VECTOR:
	    len = snprintf(buf,sizeof(buf),"%5.3f,%5.3f,%5.3f",
				var->x, var->y, var->z );
	    break;

	case VAR_STRING: // VAR_STRING will not happen
	    ASSERT(0);
	case VAR_UNSET:
	    break;
    }

    AssignStringVS(temp,buf,len);
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Var_t * ToNoneV ( Var_t * var )
{
    DASSERT(var);
    FreeV(var);
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToBoolV ( Var_t * var )
{
    DASSERT(var);
    const bool i = GetBoolV(var);
    FreeV(var);
    var->i = i;
    var->mode = VAR_INT;
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToIntV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_INT )
    {
	const int i = GetIntV(var);
	FreeV(var);
	var->i = i;
	var->mode = VAR_INT;
    }
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToDoubleV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_DOUBLE )
    {
	const double d = GetDoubleV(var);
	FreeV(var);
	var->d = d;
	var->mode = VAR_DOUBLE;
    }
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToXDoubleV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_DOUBLE )
    {
	const double d = GetXDoubleV(var);
	FreeV(var);
	var->d = d;
	var->mode = VAR_DOUBLE;
    }
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToYDoubleV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_DOUBLE )
    {
	const double d = GetYDoubleV(var);
	FreeV(var);
	var->d = d;
	var->mode = VAR_DOUBLE;
    }
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToZDoubleV ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_DOUBLE )
    {
	const double d = GetZDoubleV(var);
	FreeV(var);
	var->d = d;
	var->mode = VAR_DOUBLE;
    }
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToVectorV ( Var_t * var )
{
    switch(var->mode)
    {
	case VAR_UNSET:
	    var->x = var->y = var->z = 0.0;
	    break;

	case VAR_INT:
	    var->x = var->y = var->z = (double)var->i;
	    break;

	case VAR_DOUBLE:
	    DASSERT( &var->x == &var->d );
	    var->y = var->z = var->x;
	    break;

	case VAR_VECTOR:
	    return var;

	case VAR_STRING:
	    {
		DEFINE_VAR(temp);
		ScanVectorExpr(var->str,var->name,&temp);
		DASSERT( temp.mode == VAR_VECTOR );
		var->x = temp.x;
		var->y = temp.y;
		var->z = temp.z;
	    }
	    break;
    }
    var->mode = VAR_VECTOR;
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToVectorXV ( Var_t * var )
{
    switch(var->mode)
    {
	case VAR_UNSET:
	    var->x = var->y = var->z = 0.0;
	    break;

	case VAR_INT:
	    var->x = (double)var->i;
	    var->y = var->z = 0.0;
	    break;

	case VAR_DOUBLE:
	    DASSERT( &var->x == &var->d );
	    var->y = var->z = 0.0;
	    break;

	case VAR_VECTOR:
	    return var;

	case VAR_STRING:
	    {
		const double d = var->str_len ? strtod_comma(var->str,0) : 0.0;
		FreeV(var);
		var->x = d;
		var->y = var->z = 0.0;
	    }
	    break;
    }
    var->mode = VAR_VECTOR;
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToVectorYV ( Var_t * var )
{
    switch(var->mode)
    {
	case VAR_UNSET:
	    var->x = var->y = var->z = 0.0;
	    break;

	case VAR_INT:
	    var->y = (double)var->i;
	    var->x = var->z = 0.0;
	    break;

	case VAR_DOUBLE:
	    var->y = var->d;
	    var->x = var->z = 0.0;
	    break;

	case VAR_VECTOR:
	    return var;

	case VAR_STRING:
	    {
		const double d = var->str_len ? strtod_comma(var->str,0) : 0.0;
		FreeV(var);
		var->y = d;
		var->x = var->z = 0.0;
	    }
	    break;
    }
    var->mode = VAR_VECTOR;
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToVectorZV ( Var_t * var )
{
    switch(var->mode)
    {
	case VAR_UNSET:
	    var->x = var->y = var->z = 0.0;
	    break;

	case VAR_INT:
	    var->z = (double)var->i;
	    var->x = var->y = 0.0;
	    break;

	case VAR_DOUBLE:
	    var->z = var->d;
	    var->x = var->y = 0.0;
	    break;

	case VAR_VECTOR:
	    return var;

	case VAR_STRING:
	    {
		const double d = var->str_len ? strtod_comma(var->str,0) : 0.0;
		FreeV(var);
		var->z = d;
		var->x = var->y = 0.0;
	    }
	    break;
    }
    var->mode = VAR_VECTOR;
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToStringV  ( Var_t * var )
{
    DASSERT(var);
    if ( var->mode != VAR_STRING )
	GetStringV(var,var);
    return var;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * ToVarMode ( Var_t * var, TextCommand_t mode )
{
    DASSERT(var);
    noPRINT("ToVarMode(%u)\n",mode);

    switch (mode)
    {
	case TCMD_PDEF_I:
	case TCMD_LDEF_I:
	case TCMD_GDEF_I:
	    return ToIntV(var);

	case TCMD_PDEF_F:
	case TCMD_LDEF_F:
	case TCMD_GDEF_F:
	    return ToDoubleV(var);

	case TCMD_PDEF_S:
	case TCMD_LDEF_S:
	case TCMD_GDEF_S:
	    return ToStringV(var);

	case TCMD_PDEF_V:
	case TCMD_LDEF_V:
	case TCMD_GDEF_V:
	    return ToVectorV(var);

	case TCMD_PDEF_X:
	case TCMD_LDEF_X:
	case TCMD_GDEF_X:
	    return ToXDoubleV(var);

	case TCMD_PDEF_Y:
	case TCMD_LDEF_Y:
	case TCMD_GDEF_Y:
	    return ToYDoubleV(var);

	case TCMD_PDEF_Z:
	case TCMD_LDEF_Z:
	case TCMD_GDEF_Z:
	    return ToZDoubleV(var);

	default:
	    return var;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AssignIntV ( Var_t * var, int val )
{
    DASSERT(var);
    FreeV(var);
    var->i = val;
    var->mode = VAR_INT;
}

///////////////////////////////////////////////////////////////////////////////

void AssignDoubleV ( Var_t * var, double val )
{
    DASSERT(var);
    FreeV(var);
    var->d = val;
    var->mode = VAR_DOUBLE;
}

///////////////////////////////////////////////////////////////////////////////

void AssignXDoubleV ( Var_t * var, double val )
{
    DASSERT(var);
    FreeV(var);
    if ( var->mode != VAR_VECTOR )
    {
	var->mode = VAR_VECTOR;
	var->y = var->z = 0.0;
    }
    var->x = val;
}

///////////////////////////////////////////////////////////////////////////////

void AssignYDoubleV ( Var_t * var, double val )
{
    DASSERT(var);
    FreeV(var);
    if ( var->mode != VAR_VECTOR )
    {
	var->mode = VAR_VECTOR;
	var->x = var->z = 0.0;
    }
    var->y = val;
}

///////////////////////////////////////////////////////////////////////////////

void AssignZDoubleV ( Var_t * var, double val )
{
    DASSERT(var);
    FreeV(var);
    if ( var->mode != VAR_VECTOR )
    {
	var->mode = VAR_VECTOR;
	var->x = var->y = 0.0;
    }
    var->z = val;
}

///////////////////////////////////////////////////////////////////////////////

void AssignVectorV ( Var_t * var, const double3 * val )
{
    DASSERT(var);
    DASSERT(val);
    FreeV(var);
    memcpy(&var->d3,val,sizeof(var->d3));
    var->mode = VAR_VECTOR;
}

///////////////////////////////////////////////////////////////////////////////

void AssignVectorV3 ( Var_t * var, double x, double y, double z )
{
    DASSERT(var);
    FreeV(var);
    var->x = x;
    var->y = y;
    var->z = z;
    var->mode = VAR_VECTOR;
}

///////////////////////////////////////////////////////////////////////////////

void AssignVar
(
    Var_t		* dest,		// valid destination var
    const Var_t		* src		// NULL or valid source var
)
{
    // assertions needed for memcpy() below
    DASSERT( (u8*)&dest->name == (u8*)dest );
    DASSERT( (u8*)&dest->name + sizeof(dest->name) == (u8*)&dest->mode );

    if (src)
    {
	if ( src->mode == VAR_STRING )
	    AssignStringVV(dest,src);
	else
	    memcpy(&dest->mode,&src->mode,sizeof(*dest)-offsetof(Var_t,mode));
    }
    else
	FreeV(dest);
}

///////////////////////////////////////////////////////////////////////////////

void AssignVarMode ( Var_t * dest, const Var_t * src, TextCommand_t mode )
{
    DASSERT(dest);
    noPRINT("AssignVarMode(%u) name=%s src_mode=%d\n",
		mode, dest->name, src ? src->mode : -1 );

    // assertions needed for memcpy() below
    DASSERT( (u8*)&dest->name == (u8*)dest );
    DASSERT( (u8*)&dest->name + sizeof(dest->name) == (u8*)&dest->mode );

    switch (mode)
    {
	case TCMD_PDEF_I:
	case TCMD_LDEF_I:
	case TCMD_GDEF_I:
	    dest->i = GetIntV(src);
	    dest->mode = VAR_INT;
	    break;

	case TCMD_PDEF_F:
	case TCMD_LDEF_F:
	case TCMD_GDEF_F:
	    dest->d = GetDoubleV(src);
	    dest->mode = VAR_DOUBLE;
	    break;

	case TCMD_PDEF_S:
	case TCMD_LDEF_S:
	case TCMD_GDEF_S:
	    AssignStringVV(dest,src);
	    break;

	case TCMD_PDEF_V:
	case TCMD_LDEF_V:
	case TCMD_GDEF_V:
	    dest->d3 = GetVectorV(src);
	    dest->mode = VAR_VECTOR;
	    break;

	case TCMD_PDEF_X:
	case TCMD_LDEF_X:
	case TCMD_GDEF_X:
	    if ( dest->mode == VAR_VECTOR )
		dest->x = GetXDoubleV(src);
	    else
	    {
		dest->x = GetXDoubleV(src);
		dest->y = dest->z = 0.0;
		dest->mode = VAR_VECTOR;
	    }
	    break;

	case TCMD_PDEF_Y:
	case TCMD_LDEF_Y:
	case TCMD_GDEF_Y:
	    if ( dest->mode == VAR_VECTOR )
		dest->y = GetYDoubleV(src);
	    else
	    {
		dest->y = GetYDoubleV(src);
		dest->x = dest->z = 0.0;
		dest->mode = VAR_VECTOR;
	    }
	    break;

	case TCMD_PDEF_Z:
	case TCMD_LDEF_Z:
	case TCMD_GDEF_Z:
	    if ( dest->mode == VAR_VECTOR )
		dest->z = GetZDoubleV(src);
	    else
	    {
		dest->z = GetYDoubleV(src);
		dest->x = dest->y = 0.0;
		dest->mode = VAR_VECTOR;
	    }
	    break;

	default:
	    if (src)
		memcpy( &dest->mode, &src->mode, sizeof(*dest)-sizeof(dest->name) );
	    else
	    {
		dest->mode = VAR_UNSET;
		dest = 0;
	    }
	    break;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanValueV ( Var_t * dest, const Var_t * src, ccp name )
{
    Var_t temp;
    if ( src && src->mode == VAR_STRING )
    {
	if (!name)
	    name = src->name;

	ScanInfo_t si;
	InitializeSI(&si,src->str,src->str_len,name,0);
	if (!name)
	    si.no_warn++;

	InitializeV(&temp);
	enumError err = ScanValueSI(&si,&temp);
	if (!err)
	    CheckEolSI(&si);
	ResetSI(&si);
	src = &temp;
    }

    FreeV(dest);
    if (src)
	memcpy(&dest->mode,&src->mode,sizeof(*dest)-offsetof(Var_t,mode));
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanExprV  ( Var_t * dest, const Var_t * src, ccp name )
{
    Var_t temp;
    if ( src && src->mode == VAR_STRING )
    {
	if (!name)
	    name = src->name;

	ScanInfo_t si;
	InitializeSI(&si,src->str,src->str_len,name,0);
	if (!name)
	    si.no_warn++;

	InitializeV(&temp);
	enumError err = ScanExprSI(&si,&temp);
	if (!err)
	    CheckEolSI(&si);
	ResetSI(&si);
	src = &temp;
    }

    FreeV(dest);
    if (src)
	memcpy(&dest->mode,&src->mode,sizeof(*dest)-offsetof(Var_t,mode));
    return ERR_OK;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ResultFloat
(
    Var_t		* dest,		// valid destination var
    const float		* val,		// pointer to N floats
    uint		n_val		// number of floats in 'n'
)
{
    FreeV(dest);
    switch (n_val)
    {
	case 0:
	    dest->i = 0;
	    dest->mode = VAR_UNSET;
	    break;

	case 1:
	    dest->d = *val;
	    dest->mode = VAR_DOUBLE;
	    break;

	case 2:
	    dest->x = val[0];
	    dest->y = 0.0;
	    dest->z = val[1];
	    dest->mode = VAR_VECTOR;
	    break;

	default:
	    dest->x = val[0];
	    dest->y = val[1];
	    dest->z = val[2];
	    dest->mode = VAR_VECTOR;
	    break;
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		variables & maps: interface		///////////////
///////////////////////////////////////////////////////////////////////////////

static bool IsNum ( char ch, int hex_mode )
{
    return ch >= '0' && ch <= '9'
	|| ch == '-'
	|| ch == '+'
	|| ch == '.'
	|| hex_mode > 0 && ( ch >= 'a' && ch <= 'f' || ch >= 'A' && ch <= 'F' );
}

///////////////////////////////////////////////////////////////////////////////

void InitializeVarMap
(
    VarMap_t		* vm		// data structure to initialize
)
{
    DASSERT(vm);
    memset(vm,0,sizeof(*vm));
}

///////////////////////////////////////////////////////////////////////////////

void ResetVarMap
(
    VarMap_t		* vm		// data structure to resewt
)
{
    DASSERT(vm);
    ClearVarMap(vm);
    FREE(vm->list);
    memset(vm,0,sizeof(*vm));
}

///////////////////////////////////////////////////////////////////////////////

void ClearVarMap
(
    VarMap_t		* vm		// data structure to resewt
)
{
    DASSERT(vm);
    if (vm->used)
    {
	uint count = vm->used;
	Var_t * v = vm->list;
	while ( count-- > 0 )
	{
	    FreeV(v);
	    FreeString(v->name);
	    v++;
	}
	vm->used = 0;
	// not needed: memset(vm->list,0,sizeof(*vm->list)*vm->size);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CopyVarMap
(
    VarMap_t		* dest,		// destination var map
    bool		init_dest,	// true: initialialize 'dest' first
    const VarMap_t	* src		// source var map
)
{
    DASSERT(dest);
    DASSERT(src);

    if (init_dest)
	InitializeVarMap(dest);

    if (src->used)
    {
	Var_t * vsrc = src->list;
	// assertions needed for memcpy() below
	DASSERT( (u8*)&vsrc->name ==  (u8*)vsrc );
	DASSERT( (u8*)&vsrc->name + sizeof(vsrc->name) == (u8*)&vsrc->mode );

	uint count;
	for ( count = src->used; count > 0; count--, vsrc++ )
	{
	    Var_t * vdest = InsertVarMap( dest, vsrc->name, false, 0, 0 );
	    DASSERT(vdest);
	    if ( vsrc->mode == VAR_STRING )
		AssignStringVV(vdest,vsrc);
	    else
		memcpy(&vdest->mode,&vsrc->mode,sizeof(*vdest)-offsetof(Var_t,mode));
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void MoveVarMap
(
    VarMap_t		* dest,		// destination var map
    bool		init_dest,	// true: initialialize 'dest' first
    VarMap_t		* src		// source var map -> will be resetted
)
{
    DASSERT(dest);
    DASSERT(src);

    if (!init_dest)
	ResetVarMap(dest);
    memcpy(dest,src,sizeof(*dest));
    InitializeVarMap(src);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint FindVarMapHelper ( const VarMap_t * vm, bool * p_found, ccp name )
{
    ASSERT(vm);

    int beg = 0;
    if ( vm && name )
    {
	int end = vm->used - 1;
	while ( beg <= end )
	{
	    uint idx = (beg+end)/2;
	    int stat = strcmp(name,vm->list[idx].name);
	    if ( stat < 0 )
		end = idx - 1 ;
	    else if ( stat > 0 )
		beg = idx + 1;
	    else
	    {
		TRACE("FindVarMapHelper(%s) FOUND=%d/%d/%d\n",
			name, idx, vm->used, vm->size );
		if (p_found)
		    *p_found = true;
		return idx;
	    }
	}
    }

    TRACE("FindVarMapHelper(%s) failed=%d/%d/%d\n",
		name, beg, vm->used, vm->size );

    if (p_found)
	*p_found = false;
    return beg;
}

///////////////////////////////////////////////////////////////////////////////

int FindVarMapIndex
(
    const VarMap_t	* vm,		// valid variable map
    ccp			varname,	// variable name
    int			not_found_value	// return value if not found
)
{
    bool found;
    const int idx = FindVarMapHelper(vm,&found,varname);
    return found ? idx : not_found_value;
}

///////////////////////////////////////////////////////////////////////////////

bool RemoveVarMap
(
    // returns 'true' if var deleted

    VarMap_t		* vm,		// valid variable map
    ccp			varname		// variable name
)
{
    bool found;
    uint idx = FindVarMapHelper(vm,&found,varname);
    if (found)
    {
	vm->used--;
	ASSERT( idx <= vm->used );
	Var_t *dest = vm->list + idx;
	FreeV(dest);
	FREE((char*)dest->name);
	memmove(dest,dest+1,(vm->used-idx)*sizeof(*dest));
    }
    return found;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * InsertVarMap
(
    VarMap_t		* vm,		// valid variable map
    ccp			varname,	// varname to insert
    bool		move_varname,	// true: move 'varname', false: strdup()
    ScanInfo_t		* si,		// not null: print "already defined" message
    bool		* found		// not null: store found status
)
{
    DASSERT(vm);
    DASSERT( vm->used <= vm->size );

    if (!varname)
    {
	if (found)
	    *found = false;
	return 0;
    }

    if ( vm->force_case == LOUP_UPPER || vm->force_case == LOUP_LOWER )
    {
	char *myname = move_varname ? (char*)varname : STRDUP(varname);
	if ( vm->force_case == LOUP_UPPER )
	    StringUpper(myname);
	else
	    StringLower(myname);
	varname = myname;
	move_varname = true;
    }

    bool my_found;
    const int idx = FindVarMapHelper(vm,&my_found,varname);
    if (found)
	*found = my_found;

    Var_t * var;
    if (my_found)
    {
	var = vm->list + idx;
	if (move_varname)
	    FreeString(varname);

	if ( si && si->no_warn <= 0 )
	{
	    ScanFile_t *sf = si->cur_file;
	    DASSERT(sf);
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Variable '%s' already defined [%s @%u]: %.*s\n",
			varname, sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
    }
    else
    {
	if ( vm->used == vm->size )
	{
	    vm->size = 3*vm->size/2 + 100;
	    vm->list = REALLOC(vm->list,vm->size*sizeof(*vm->list));
	}

	DASSERT( idx <= vm->used );
	var = vm->list + idx;
	memmove(var+1,var,(vm->used-idx)*sizeof(*var));
	vm->used++;

	var->name = move_varname ? varname : STRDUP(varname);
	var->mode = VAR_UNSET;
    }

    return var;
}

///////////////////////////////////////////////////////////////////////////////

bool DefineIntVar
(
    VarMap_t		* vm,		// valid variable map
    ccp			varname,	// name of variable
    int			value		// value to assign
)
{
    DASSERT(vm);
    DASSERT(varname);

    bool found;
    Var_t * var = InsertVarMap( vm, varname, false, 0, &found );
    DASSERT(var);
    FreeV(var);
    var->mode = VAR_INT;
    var->i = value;
    return found;
}

///////////////////////////////////////////////////////////////////////////////

bool DefineDoubleVar
(
    VarMap_t		* vm,		// valid variable map
    ccp			varname,	// name of variable
    double		value		// value to assign
)
{
    DASSERT(vm);
    DASSERT(varname);

    bool found;
    Var_t * var = InsertVarMap( vm, varname, false, 0, &found );
    DASSERT(var);
    FreeV(var);
    var->mode = VAR_DOUBLE;
    var->d = value;
    return found;
}

///////////////////////////////////////////////////////////////////////////////

void PrintV
(
    FILE		* f,		// destination file
    const Var_t		* v,		// variable to print
    int			str_mode	// 0:-, 1:limit+escape, 2:limit+escape+quote
)
{
    DASSERT(f);
    DASSERT(v);

    const uint max_str = 100;
    char strbuf[5*max_str+100];

    switch(v->mode)
    {
	case VAR_UNSET:
	    fputs("          $NONE",f);
	    break;

	case VAR_INT:
	    fprintf(f,"%15lld = %#14llx", v->i, v->i & 0xffffffff );
	    if (v->int_mode)
		fprintf(f," [%s]",GetIntModeName(v->int_mode));
	    break;

	case VAR_DOUBLE:
	    if ( fabs(v->d) < 1e10 )
		fprintf(f,"%15.3f = %14.7e", v->d, v->d );
	    else
		fprintf(f,"                  %14.7e", v->d );
	    break;

	case VAR_VECTOR:
	    if ( fabs(v->x) < 1e10 )
		fprintf(f,"v( %14.3f,", v->x );
	    else
		fprintf(f,"v( %14.7e,", v->x );

	    if ( fabs(v->y) < 1e10 )
		fprintf(f," %14.3f,",v->y);
	    else
		fprintf(f," %14.7e,",v->y);

	    if ( fabs(v->z) < 1e10 )
		fprintf(f," %14.3f )",v->z);
	    else
		fprintf(f," %14.7e )",v->z);
	    break;

	case VAR_STRING:
	    if (!str_mode)
		fwrite(v->str,v->str_len,1,f);
	    else
	    {
		ccp quote = str_mode > 1 ? "\"" : "";
		PrintEscapedString( strbuf, sizeof(strbuf),
				v->str, v->str_len < max_str ? v->str_len : max_str,
				CHMD__ALL, *quote, 0 );
		if ( v->str_len > max_str )
		    fprintf(f,"[%u/%u] %s%s%s...",max_str,v->str_len,quote,strbuf,quote);
		else
		    fprintf(f,"[%u] %s%s%s",v->str_len,quote,strbuf,quote);
	    }
	    break;

	default:
	    fputs("?",f);
	    break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void DumpVarMap
(
    FILE		* f,		// destination file
    uint		indent,		// indention
    const VarMap_t	* vm,		// valid variable map
    bool		print_header	// print table header
)
{
    DASSERT(f);
    DASSERT(indent<100);
    DASSERT(vm);

    if (!vm->used)
	return;

    uint maxlen = 4;
    Var_t *v, *vend = vm->list + vm->used;
    for ( v = vm->list; v < vend; v++ )
    {
	const uint len = strlen(v->name);
	if ( maxlen < len )
	     maxlen = len;
    }

    char last_name1 = 0;
    if (print_header)
    {
	fprintf(f,"%*s%-*s =           value =          value\n"
		  "%*s%.*s\n",
		  indent,"", maxlen, "name",
		  indent,"", maxlen+36, Minus300 );
	last_name1 = vm->list->name[0];
    }

    for ( v = vm->list; v < vend; v++ )
    {
	if ( print_header && last_name1 != v->name[0] )
	{
	    last_name1 = v->name[0];
	    fputc('\n',f);
	}

	fprintf(f,"%*s%-*s = ", indent,"", maxlen, v->name );
	PrintV(f,v,2);
	fputc('\n',f);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SI var access			///////////////
///////////////////////////////////////////////////////////////////////////////

const Var_t * FindConst
(
    ccp			varname	// variable name
)
{
    bool found;
    const int idx = FindVarMapHelper(&const_map,&found,varname);
    return found ? const_map.list + idx : 0;
}

///////////////////////////////////////////////////////////////////////////////

const Var_t * FindVarMap
(
    const VarMap_t	* vm,		// NULL or variable map
    ccp			varname,	// variable name
    ScanInfo_t		* si		// not null: print "not found" message
)
{
    if (vm)
    {
	bool found;
	const int idx = FindVarMapHelper(vm,&found,varname);
	if (found)
	    return vm->list + idx;
    }

    if ( si && si->no_warn <= 0 )
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	ccp eol = FindNextLineFeedSI(si,true);
	ERROR0(ERR_WARNING,
		"Variable '%s' not found [%s @%u]: %.*s\n",
		varname, sf->name, sf->line,
		(int)(eol - sf->prev_ptr), sf->prev_ptr );
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const Var_t * FindVarSI
(
    ScanInfo_t		* si,		// not NULL: use var maps
    ccp			varname,	// variable name
    bool		print_warning	// true && 'si': print 'not found' warning
)
{
    if (si)
    {
	bool found;
	DASSERT(si->cur_file);
	int idx = FindVarMapHelper(&si->cur_file->pvar,&found,varname);
	if (found)
	    return si->cur_file->pvar.list + idx;

	if (si->param)
	{
	    idx = FindVarMapHelper(si->param,&found,varname);
	    if (found)
		return si->param->list + idx;
	}

	idx = FindVarMapHelper(&si->lvar,&found,varname);
	if (found)
	    return si->lvar.list + idx;

	idx = FindVarMapHelper(&si->gvar,&found,varname);
	if (found)
	    return si->gvar.list + idx;

	if (si->predef)
	{
	    idx = FindVarMapHelper(si->predef,&found,varname);
	    if (found)
		return si->predef->list + idx;
	}
    }

    const Var_t *res = FindConst(varname);
    if (res)
	return res;

    if (print_warning)
	FindVarMap(0,varname,si);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

Var_t * OverwriteVarSI
(
    // returns always not NULL

    ScanInfo_t		* si,		// not NULL: use var maps for search
    VarMap_t		* vm_insert,	// valid variable map, insert search failed
    ccp			varname,	// varname to insert
    int			* xyz,		// not NULL: enable .X/Y/Z support
					//   input: <0: not a X/Y/Z
					//        0..2: is a X/Y/Z
					//	    >2: unknown
					//   output: store corrected value
    bool		* found		// not null: store found status (false=NEW)
)
{
    DASSERT(vm_insert);
    DASSERT( vm_insert->used <= vm_insert->size );
    noPRINT("OverwriteVarSI(xyz=%d) %s\n",xyz?*xyz:-999,varname);

    Var_t *var = (Var_t*)FindVarSI(si,varname,false);
    if (var)
    {
	if (xyz)
	    *xyz = -1;
	if (found)
	    *found = true;
	return var;
    }

    if ( xyz && *xyz >= 0 )
    {
	uint namelen = strlen(varname);
	if ( *xyz > 2 )
	{
	    *xyz = namelen > 2
			&& varname[namelen-2] == '.'
			&& varname[namelen-1] >= 'X'
			&& varname[namelen-1] <= 'Z'
		 ? varname[namelen-1] - 'X' : -1;
	}

	if ( *xyz >= 0 && namelen > 2 && namelen <= VARNAME_SIZE )
	{
	    char basename[VARNAME_SIZE+1];
	    memcpy(basename,varname,namelen-2);
	    basename[namelen-2] = 0;
	    var = (Var_t*)FindVarSI(si,basename,0);
	    if ( var && var->mode == VAR_VECTOR )
	    {
		if (found)
		    *found = true;
		return var;
	    }
	    *xyz = -1;
	}
    }

    return InsertVarMap(vm_insert,varname,false,0,0);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    options			///////////////
///////////////////////////////////////////////////////////////////////////////

VarMap_t const_map = {0};

///////////////////////////////////////////////////////////////////////////////

void SetupConst()
{
    if (!const_map.used)
    {
	InsertVarMap(&const_map,"$NONE",false,0,0);
	InsertVarMap(&const_map,"$NULL",false,0,0);

	struct inttab_t { ccp name; int val; };
	static const struct inttab_t inttab[] =
	{
	    { "$N",			0 },
	    {0,0}
	};

	const struct inttab_t * ip;
	for ( ip = inttab; ip->name; ip++ )
	    DefineIntVar(&const_map,ip->name,ip->val);

	struct dbltab_t { ccp name; double val; };
	static const struct dbltab_t dbltab[] =
	{
	    { "INF",	INFINITY },
	    { "NAN",	NAN },
	    {0,0}
	};

	const struct dbltab_t * dp;
	for ( dp = dbltab; dp->name; dp++ )
	    DefineDoubleVar(&const_map,dp->name,dp->val);
    }
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptConst ( ccp arg )
{
    SetupConst();
    if (!arg)
	return 0;

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),"Option --const",0);
    ScanFile_t *sf = si.cur_file;
    DASSERT(sf);
    sf->disable_comma++;
    for(;;)
    {
	while ( *sf->ptr > 0 && *sf->ptr <= ' ' || *sf->ptr == ',' )
	    sf->ptr++;
	if (!*sf->ptr)
	    break;

	char varname[VARNAME_SIZE+1];
	const uint namelen = ScanNameSI(&si,varname,sizeof(varname),false,true,0);
	if (!namelen)
	{
	    ERROR0(ERR_SYNTAX,"Option --const: Missing name: %s\n",sf->prev_ptr);
	    goto abort;
	}

	if ( NextCharSI(&si,false) != '=' )
	{
	    ERROR0(ERR_SYNTAX,
		"Option --const: Missing '=' behind name: %s\n",
		sf->prev_ptr);
	    goto abort;
	}
	sf->ptr++;

	Var_t * vdest = InsertVarMap( &const_map, varname, false, 0, 0 );
	DASSERT(vdest);
	if (ScanExprSI(&si,vdest))
	    goto abort;
    }

    ResetSI(&si);
    return 0;

 abort:
    ResetSI(&si);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////			scan text			///////////////
///////////////////////////////////////////////////////////////////////////////

ScanFile_t empty_scan_file = {0}; // used as last file in ScanInfo_t

///////////////////////////////////////////////////////////////////////////////

static void InitializeSM
(
    ScanMacro_t		* sm		// data structure to setup
)
{
    DASSERT(sm);
    memset(sm,0,sizeof(*sm));
}

///////////////////////////////////////////////////////////////////////////////

static void ResetSM
(
    ScanMacro_t		* sm		// data structure to reset
)
{
    DASSERT(sm);
    if (sm->data_alloced)
	FREE((void*)sm->data);
    FreeString(sm->src_name);
    memset(sm,0,sizeof(*sm));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void InitializeSF
(
    ScanFile_t		* sf,		// data structure to setup
    ScanInfo_t		* si,		// NULL or base scan info
    ccp			data,		// data to scan
    uint		data_size,	// size of data
    ccp			name,		// NULL or name for warnings
    int			revision,	// revision number
    VarMap_t		* varmap	// NULL or new varmap (moved)
)
{
    DASSERT(sf);
    memset(sf,0,sizeof(*sf));

    sf->data		= data;
    sf->ptr		= data + GetTextBOMLen(data,data_size);
    sf->prev_ptr	= sf->ptr;
    sf->end		= data + data_size;
    sf->line		= 1;
    sf->line_err	= 0;
    sf->name		= name ? name : "?";
    sf->revision	= revision;

    if (si)
    {
	sf->next	= si->cur_file;
	si->cur_file	= sf;
	si->n_files++;
     #if HAVE_PRINT
	si->debug	= 0;
     #endif
    }

    if (varmap)
	MoveVarMap(&sf->pvar,true,varmap);
    else
    {
	InitializeVarMap(&sf->pvar);
	Var_t *var = InsertVarMap(&sf->pvar,"$N",false,0,0);
	DASSERT(var);
	AssignIntV(var,0);
    }
}

///////////////////////////////////////////////////////////////////////////////

ScanFile_t * AddSF
(
    ScanInfo_t		* si,		// base scan info
    ccp			data,		// data to scan
    uint		data_size,	// size of data
    ccp			name,		// NULL or name for warnings
    int			revision,	// revision number
    VarMap_t		* varmap	// NULL or new varmap (moved)
)
{
    DASSERT(si);
    noPRINT("AddSF(rev=%u)\n",revision);

    if ( si->n_files > MAX_SOURCE_DEPTH )
    {
	si->total_err++;
	ScanFile_t * sf = si->cur_file;
	DASSERT(sf);
	ERROR0(ERR_WARNING,
		"To many open source files and/or macros (max=%u) [%s @%u]\n",
		MAX_SOURCE_DEPTH, sf->name, sf->line );
	return 0;
    }

    ScanFile_t * sf = MALLOC(sizeof(*sf));
    InitializeSF(sf,si,data,data_size,name,revision,varmap);
    noPRINT("AddSF() %p,%p, rev=%u\n",sf,sf->next,sf->revision);
    return sf;
}

///////////////////////////////////////////////////////////////////////////////

static void ResetSF
(
    ScanFile_t		* sf		// data structure to reset
)
{
    DASSERT(sf);

    int i;
    for ( i = 0; i < MAX_LOOP_DEPTH; i++ )
	FreeString(sf->loop[i].varname);

    if (sf->data_alloced)
	FREE((void*)sf->data);

    if (sf->name_alloced)
	FreeString(sf->name);

    ResetVarMap(&sf->pvar);
    memset(sf,0,sizeof(*sf));
}

///////////////////////////////////////////////////////////////////////////////

bool DropSF
(
    ScanInfo_t		* si		// data structure
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;

    if ( !sf || sf == &empty_scan_file )
	return false;

    si->n_files--;
    si->cur_file = sf->next;
    ResetSF(sf);
    FREE(sf);
    DASSERT(si->cur_file);
    noPRINT("DROPSF() -> |%.20s|\n",si->cur_file->ptr);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeSI
(
    ScanInfo_t		* si,		// data structure to setup
    ccp			data,		// data to scan
    uint		data_size,	// size of data
    ccp			name,		// NULL or name for warnings
    int			revision	// revision number
)
{
    DASSERT(si);
    DASSERT(data);
    memset(si,0,sizeof(*si));

    si->init_data	= data;
    si->init_data_size	= data_size;
    si->init_name	= name;
    si->init_revision	= revision;

    InitializeSF(&empty_scan_file,0,0,0,EmptyString,0,0);
    si->cur_file = &empty_scan_file;
    empty_scan_file.revision = REVISION_NUM;
    AddSF(si,data,data_size,name,revision,0);

    InitializeVarMap(&si->macro);
    InitializeVarMap(&si->lvar);
    InitializeVarMap(&si->gvar);

    DefineIntVar(&si->gvar,"REVISION$SETUP",revision);
    DefineIntVar(&si->gvar,"REVISION$ACTIVE",revision);
}

///////////////////////////////////////////////////////////////////////////////

void ResetHelperSI
(
    ScanInfo_t		* si		// data structure to reset
)
{
    DASSERT(si);

    //--- close all files

    while (DropSF(si))
	;

    //--- drop all macros

    if (si->macro.used)
    {
	uint count = si->macro.used;
	Var_t * v = si->macro.list;
	while ( count-- > 0 )
	{
	    ScanMacro_t *sm = v->macro;
	    if (sm)
	    {
		v->macro = 0;
		ResetSM(sm);
		FREE(sm);
	    }
	    v++;
	}
    }
    ResetVarMap(&si->macro);

    //--- clear variables

    ResetVarMap(&si->lvar);
    si->cur_file = &empty_scan_file;
}

///////////////////////////////////////////////////////////////////////////////

void ResetSI
(
    ScanInfo_t		* si		// data structure to reset
)
{
    DASSERT(si);

    ResetHelperSI(si);
    ResetVarMap(&si->gvar);

    memset(si,0,sizeof(*si));
    si->cur_file = &empty_scan_file;
}

///////////////////////////////////////////////////////////////////////////////

void RestartSI
(
    ScanInfo_t		* si		// data structure to setup
)
{
    ResetHelperSI(si);
    AddSF(si,si->init_data,si->init_data_size,si->init_name,si->init_revision,0);
}

///////////////////////////////////////////////////////////////////////////////

void ResetLocalVarsSI
(
    ScanInfo_t		* si,		// data structure to setup
    int			revision	// revision number
)
{
    DASSERT(si);
    ResetVarMap(&si->lvar);

    ScanFile_t *sf ;
    for ( sf = si->cur_file; sf && sf != &empty_scan_file; sf = sf->next )
	sf->revision = revision;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool TestContLineSI
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    ccp ptr = sf->ptr;
    ccp end = sf->end;
    uint lines = 0;

    uchar ch = 0;
    for(;;)
    {
	if ( ptr == end )
	{
	    ch = 0;
	    break;
	}

	ch = *ptr;
	if ( ch == '#' )
	{
	    while ( ptr < end && *ptr != '\n' )
		ptr++;
	    continue;
	}

	if ( ch == '\n' )
	    lines++;

	if ( ch > ' ' )
	    break;
	ptr++;
    }

    if ( ch != '>' )
	return false;

    sf->ptr = ptr;
    if (lines)
    {
	sf->line += lines;
	sf->line_err = 0;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

char NextCharSI
(
    ScanInfo_t		* si,		// valid data
    bool		skip_lines	// true: skip empty lines
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    ccp ptr = sf->ptr;
    ccp end = sf->end;

    for(;;)
    {
	if ( ptr == end )
	{
	    sf->ptr = ptr;
	    if (!skip_lines)
	    {
		CheckLevelSI(si);
		return 0;
	    }

	    if (!NextLineSI(si,false,true))
		return 0;

	    sf = si->cur_file;
	    DASSERT(sf);
	    ptr = sf->ptr;
	    end = sf->end;
	    noPRINT("CHAR [%zu] |%.20s|\n",end-ptr,ptr);
	    continue;
	}

	const uchar ch = *ptr++;
	if ( ch == '#' )
	{
	    while ( ptr < end && *ptr != '\n' )
		ptr++;
	    continue;
	}

	if ( ch == '@' && skip_lines )
	{
	    sf->ptr = ptr;
	    const TextCommand_t tcmd = ScanTextCommand(si);
	    if (tcmd)
	    {
		ExecTextCommand(si,tcmd);
		sf = si->cur_file;
		DASSERT(sf);
		ptr = sf->ptr;
		end = sf->end;
		continue;
	    }
	}

	if ( ch > ' ' )
	{
	    sf->ptr = ptr - 1;
	    return ch;
	}

	if ( ch == '\n' )
	{
	    sf->ptr = sf->prev_ptr = ptr - 1;
	    if (TestContLineSI(si))
		ptr = sf->ptr+1;
	    else
		return skip_lines ? NextLineSI(si,true,true) : 0;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

bool IsSectionOrEotSI
(
    // Call NextCharSI() and check for new section or EOT (return true if found)
    // Section name is stored in 'last_index_name' (upper case)

    ScanInfo_t		* si,		// valid data
    bool		skip_lines	// true: skip empty lines
)
{
    DASSERT(si);

    *last_index_name = 0;
    char ch = NextCharSI(si,skip_lines);
    if (!ch)
	return true;
	
    if ( ch == '[' )
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	ccp beg = sf->ptr + 1;
    	ccp eol = beg;
	while ( eol < sf->end && *eol != '\n' )
	    eol++;
	eol--;
	while ( eol > beg && (uchar)*eol <= ' ' )
	    eol--;

	if ( *eol == ']' )
	{
	    int len = eol - beg;
	    if ( len > 0 && !memchr(beg,'[',len) && !memchr(beg,']',len) )
	    {
		mem_t name = { .ptr = beg, .len = len };
		SetIndexNameM(name);
		return true;
	    }
	}
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

char SkipCharSI
(
    // return next non skipped character

    ScanInfo_t		* si,		// valid data
    char		ch		// character to skip
)
{
    DASSERT(si);
    const char next_ch = NextCharSI(si,false);
    if ( next_ch == ch )
    {
	DASSERT(si->cur_file);
	si->cur_file->ptr++;
	return NextCharSI(si,false);
    }
    return next_ch;
}

///////////////////////////////////////////////////////////////////////////////

char NextLineSI
(
    ScanInfo_t		* si,		// valid data
    bool		skip_cont,	// true: skip continuation lines
    bool		allow_tcmd	// true: scan and execute text commands
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    char cont_ch = skip_cont ? '>' : '#';
    ccp ptr = sf->ptr;
    ccp end = sf->end;

    for(;;)
    {
	//--- find beginning of next line first (skip current line)

	while ( ptr < end )
	    if ( *ptr++ == '\n' )
	    {
		sf->line++;
		sf->line_err = 0;
		break;
	    }

	//--- now find next relevant line

	for(;;)
	{
	    if ( ptr == end )
	    {
		sf->ptr = sf->prev_ptr = ptr;
		CheckLevelSI(si);

		if ( sf != &empty_scan_file )
		    si->last_result.mode = VAR_UNSET;
		if ( !allow_tcmd || !DropSF(si) )
		    return 0;

		sf = si->cur_file;
		DASSERT(sf);
		ptr = sf->ptr;
		end = sf->end;
		cont_ch = '#';
		continue;
	    }

	    // skip blanks and controls at beginning of line

	    while ( ptr < end && (uchar)*ptr <= ' ' )
	    {
		if ( *ptr == '\n' )
		{
		    sf->line++;
		    sf->line_err = 0;
		}
		ptr++;
	    }

	    if ( ptr == end )
		continue;

	    if ( *ptr == '@' && allow_tcmd )
	    {
		sf->ptr = ptr;
		const TextCommand_t tcmd = ScanTextCommand(si);
		if (tcmd)
		{
		    ExecTextCommand(si,tcmd);
		    sf = si->cur_file;
		    DASSERT(sf);
		    ptr = sf->ptr;
		    end = sf->end;
		    continue;
		}
	    }

	    if ( *ptr != '#' && *ptr != cont_ch )
	    {
		// neither comment nor continuation line
		sf->ptr = sf->prev_ptr = ptr;
		return *ptr;
	    }
	    break;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError CheckLevelSI
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    DASSERT(sf->if_level >= 0);

    enumError stat = ERR_OK;

    if ( sf->if_level > 0 )
    {
	if ( si->no_warn <= 0 )
	{
	    si->total_err++;
	    ERROR0(ERR_WARNING,
		"End of text/section within @IF..@ENDIF level #%u [%s @%u]\n",
		sf->if_level, sf->name, sf->line );
	}
	sf->if_level = 0;
	stat = ERR_WARNING;
    }

    if ( sf->loop_level > 0 )
    {
	if ( si->no_warn <= 0 )
	{
	    si->total_err++;
	    ERROR0(ERR_WARNING,
		"End of text/section within loop level #%u [%s @%u]\n",
		sf->loop_level, sf->name, sf->line );
	}
	sf->loop_level = 0;
	stat = ERR_WARNING;
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetEolSI
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    ccp ptr = sf->ptr, prev = sf->prev_ptr;
    GotoEolSI(si);
    ccp eol	 = sf->ptr;
    sf->ptr	 = ptr;
    sf->prev_ptr = prev;
    return eol;
}

///////////////////////////////////////////////////////////////////////////////

void GotoEolSI
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    sf->prev_ptr = sf->ptr;
    while (NextCharSI(si,false))
    {
	ccp ptr = sf->ptr;
	while ( ptr < sf->end && *ptr != '\n' )
	    ptr++;
	sf->ptr = ptr;
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError CheckEolSI
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    sf->prev_ptr = sf->ptr;
    if (!NextCharSI(si,false))
	return ERR_OK;

    if ( si->no_warn <= 0 )
    {
	si->total_err++;
	if (!sf->line_err)
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"End of line expected [%s @%u]: %.*s\n",
			sf->name, sf->line, (int)(eol - sf->ptr), sf->ptr );
	}
    }
    GotoEolSI(si);
    return ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

enumError CheckWarnSI
(
    ScanInfo_t		* si,		// valid data
    char		ch,		// char to test
    enumError		err		// previous error level
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    if ( NextCharSI(si,false) == ch )
	sf->ptr++;
    else
    {
	if ( err < ERR_WARNING )
	    err = ERR_WARNING;

	si->total_err++;
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Character '%c' expected [%s @%u]: %.*s\n",
			ch, sf->name, sf->line, (int)(eol - sf->ptr), sf->ptr );
	}
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError WarnIgnoreSI
(
    ScanInfo_t		* si,		// valid data
    ccp			format,		// NULL or format of text before 'line ignored'
    ...					// arguments for 'format'
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    si->total_err++;
    if ( si->no_warn <= 0 )
    {
	char buf[1000];
	*buf = 0;

	if (format)
	{
	    va_list arg;
	    va_start(arg,format);
	    vsnprintf(buf,sizeof(buf),format,arg);
	    va_end(arg);
	}

	ccp eol = FindNextLineFeedSI(si,true);
	ERROR0(ERR_WARNING,
		"%sLine ignored [%s @%u]:\n%.*s\n",
		buf, sf->name, sf->line, (int)(eol - sf->ptr), sf->ptr );
    }

    GotoEolSI(si);
    return ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

ccp FindEndOfLineSI
(
    ScanInfo_t		* si,		// valid data
    ccp			ptr		// start search here
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    while ( ptr < sf->end && *ptr != '\n' && *ptr != '\r' && *ptr )
	ptr++;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

ccp FindLineFeedSI
(
    const ScanInfo_t	* si,		// valid data
    ccp			ptr,		// start search here
    bool		mark_error	// true: increment error counter (ignore const)
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    if (mark_error)
    {
	sf->line_err++;
	((ScanInfo_t*)si)->total_err++;
    }

    while ( ptr < sf->end && *ptr != '\n' )
	ptr++;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

ccp FindNextLineFeedSI
(
    const ScanInfo_t	* si,		// valid data
    bool		mark_error	// true: increment error counter (ignore const)
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    return FindLineFeedSI(si,si->cur_file->ptr,mark_error);
}

///////////////////////////////////////////////////////////////////////////////

mem_t GetLineSI
(
    const ScanInfo_t	* si		// valid data
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    ccp eol = FindNextLineFeedSI(si,false);
    mem_t mem;
    mem.ptr = si->cur_file->ptr;
    mem.len = eol ? eol - mem.ptr : 0;
    return mem;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint ScanNameSI
(
    // return length of name

    ScanInfo_t		* si,		// valid data
    char		* name_buf,	// destination buffer
    uint		buf_size,	// size of 'name_buf'
    bool		allow_minus,	// true: allow '-' in names
    bool		to_upper,	// true: convert letters to upper case
    int			* xyz		// not null: store vector index
					// -1: no index, 0=x, 1=y, 2=z
)
{
    DASSERT(si);
    DASSERT(name_buf);
    DASSERT(buf_size>1);
    NextCharSI(si,false);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    sf->prev_ptr = sf->ptr;
    char *dest = name_buf, *end = name_buf + buf_size - 1;

    for(;;)
    {
	ccp src = sf->ptr;
	while ( src < sf->end )
	{
	    char ch = *src;
	    if (   ch >= 'A' && ch <= 'Z'
		|| ch >= '0' && ch <= '9'
		|| ch == '_'
		|| ch == '.'
		|| ch == '$'
		|| ch == '-' && allow_minus
	       )
	    {
		// nothing to do
	    }
	    else if ( ch >= 'a' && ch <= 'z' )
	    {
		if (to_upper)
		    ch += 'A' - 'a';
	    }
	    else
		break;

	    src++;
	    if ( dest < end )
		*dest++ = ch;
	}
	sf->ptr = src;

	if ( dest == name_buf || NextCharSI(si,false) != '[' )
	    break;

	sf->ptr++;
	DEFINE_VAR(val);
	CheckWarnSI(si,']',ScanExprSI(si,&val));
	char buf[20];
	snprintf(buf,sizeof(buf),"[%d]",GetIntV(&val));
	dest = StringCopyE(dest,end,buf);
    }
    memset(dest,0,name_buf+buf_size-dest);

    if (xyz)
    {
	*xyz = dest > name_buf+2 && dest[-2] == '.' && dest[-1] >= 'X' && dest[-1] <= 'Z'
		? dest[-1] - 'X' : -1;
	noPRINT("NAME=%s XYZ=%d\n",name_buf,*xyz);
    }

    return dest - name_buf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ConcatStringsSI
(
    ScanInfo_t		* si,		// valid data
    char		** dest_str,	// not NULL: store string here, use FREE + MALLOC
    uint		* ret_len	// not NULL: store length of return string here
)
{
    DASSERT(si);

    //-- clear result

    if (dest_str)
    {
	FreeString(*dest_str);
	*dest_str = 0;
    }
    if (ret_len)
	*ret_len = 0;


    //-- scan string

// [[var-string-]] support of dynamic buffer
    char buf[5000];
    char *buf_end = buf + sizeof(buf) - 1;
    char *dest = buf, *dest_end = buf_end - 50;

    enumError err = ERR_OK;
    while ( dest < dest_end )
    {
	char ch = NextCharSI(si,false);
	if ( !ch || ch == ',' || ch == ';' )
	    break;

	if ( ch == '"' || ch == '\'' )
	{
	    DEFINE_VAR(temp);
	    err = ScanStringSI(si,&temp);
	    dest = StringCopyEM(dest,dest_end,temp.str,temp.str_len);
	    continue;
	}

// [[var-string-]] optimize?
	DEFINE_VAR(var);
	err = ScanExprSI(si,&var);
	if (err)
	    break;

	switch (var.mode)
	{
	    case VAR_INT:
		dest += snprintf(dest,buf_end-dest,"%lld",var.i);
		break;

	    case VAR_DOUBLE:
		dest += snprintf(dest,buf_end-dest,"%5.3f",var.d);
		break;

	    case VAR_VECTOR:
		dest += snprintf(dest,buf_end-dest,"%5.3f,%5.3f,%5.3f",
					var.x, var.y, var.z );
		break;

	case VAR_STRING: // [[var-string+]] NULL support missed!
		dest = StringCopyE(dest,buf_end,var.str);
	    break;

	    case VAR_UNSET:
		break;
	}
    }

    //-- store result

    const uint len = dest - buf;
    if (ret_len)
	*ret_len = len;
    if (dest_str)
    {
	*dest_str = MEMDUP(buf,len);
	noPRINT("STRING: |%s|\n",*dest_str);
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanStringSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// store result here (already initialized)
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    DASSERT(var);

    DEFINE_VAR(temp);
    temp.mode = VAR_STRING;

    enumError err = ERR_OK;
    for(;;)
    {
	const char sep = NextCharSI(si,false);
	if ( sep != '\'' && sep != '"' )
	    break;

	ccp ptr   = ++sf->ptr;
	ccp eol   = FindEndOfLineSI(si,ptr);
	ccp start = ptr;

	while ( ptr < eol && *ptr != sep )
	{
	    const char ch = *ptr++;
	    if ( ch == '\\' && ptr < eol )
		ptr++;
	}

	char buf[5000];
	const uint len
	    = ScanEscapedString( buf, sizeof(buf), start, ptr-start, true, 0, 0 );
	AppendStringVS(&temp,buf,len);

	if ( *ptr == sep )
	    sf->ptr = ptr + 1;
	else
	{
	    sf->ptr = ptr;
	    err = ERROR0(ERR_WARNING,
			"Unterminated string [%s @%u]\n", sf->name, sf->line );
	}
    }

    MoveDataV(var,&temp);
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

char last_index_name[VARNAME_SIZE+1] = {0};

int ScanIndexSI
(
    // varname is stored in 'last_index_name' (upper case)

    ScanInfo_t		* si,		// valid data
    enumError		* err,		// return value: error code
    int			index,		// index to define if name
    IndexList_t		* il,		// NULL or index list
    int			insert_mode	// 0: don't insert
					// 1: insert into global namespace
					// 2: '1' + print "already defined" error
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    enumError local_err;
    if (!err)
	err = &local_err;

    char ch = NextCharSI(si,false);
    if (IsNum(ch,0))
    {
	*last_index_name = 0;
	u32 num;
	*err = ScanU32SI(si,&num,1,0);
	if (il)
	    DefineIL(il,index,0,0);
	return num;
    }

    *err = ERR_OK;

    const uint name_len
	= ScanNameSI(si,last_index_name,sizeof(last_index_name),false,false,0);
    if (!name_len)
    {
	*err = ERR_WARNING;
	if (il)
	    DefineIL(il,index,0,0);
    }
    else if ( name_len > 1 || *last_index_name != '_' )
    {
	if (il)
	{
	    DefineIL(il,index,last_index_name,false);
	    noPRINT("DEFINE: %3u = %s\n",index,last_index_name);
	}

	char *ptr = last_index_name;
	while ( *ptr = toupper((int)*ptr) )
	    ptr++;

	if ( insert_mode > 0 )
	{
	    bool found;
	    Var_t * var = InsertVarMap( &si->gvar, last_index_name, false, 0, &found );

	    if (found)
	    {
		if ( insert_mode > 1
		    && si->no_warn <= 0
		    && ( var->mode != VAR_INT || var->i != index) )
		{
		    ccp eol = FindNextLineFeedSI(si,true);
		    ERROR0(ERR_WARNING,
				"Variable '%s' already defined [%s @%u]: %.*s\n",
				last_index_name, sf->name, sf->line,
				(int)(eol - sf->prev_ptr), sf->prev_ptr );
		}
		//*err = ERR_WARNING;
	    }
	    else
	    {
		var->mode = VAR_INT;
		var->i = index;
	    }
	}
    }
    else
    {
	*last_index_name = 0;
	if (il)
	    DefineIL(il,index,0,0);
    }

    return index;
}

//-----------------------------------------------------------------------------

#if 0 // [[2do]] [[obsolete]]
void SetIndexName ( ccp name )
{
    if (!name)
	name = "";
    char *dest = last_index_name;
    char *endbuf  = dest + sizeof(last_index_name) -1 ;
    while ( dest < endbuf && *name )
	*dest++ = toupper((int)*name++);
    *dest = 0;
}
#endif

//-----------------------------------------------------------------------------

bool DefineIndexInt
(
    // returns true, if inserted

    ScanInfo_t		* si,		// valid data
    ccp			suffix,		// name suffix
    int			value		// value to assign
)
{
    if ( *last_index_name )
    {
	char name[2*VARNAME_SIZE];
	snprintf(name,sizeof(name),"%s.%s",last_index_name,suffix);

	bool found;
	Var_t * var = InsertVarMap( &si->gvar, name, false, 0, &found );
	noPRINT("DEFINE %s = %d\n",name,value);
	var->mode = VAR_INT;
	var->i = value;
	return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

bool DefineIndexDouble
(
    // returns true, if inserted

    ScanInfo_t		* si,		// valid data
    ccp			suffix,		// name suffix
    double		value		// value to assign
)
{
    if ( *last_index_name )
    {
	char name[2*VARNAME_SIZE];
	snprintf(name,sizeof(name),"%s.%s",last_index_name,suffix);

	bool found;
	Var_t * var = InsertVarMap( &si->gvar, name, false, 0, &found );
	noPRINT("DEFINE %s = %g\n",name,value);
	var->mode = VAR_DOUBLE;
	var->d = value;
	return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanU8SI
(
    ScanInfo_t		* si,		// valid data
    u8			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	long num;
	if (!ScanUValueSI(si,&num,force_hex))
	    *dest++ = num;
	else
	{
	    while ( n-- >= 0 )
		*dest++ = 0;
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanU16SI
(
    ScanInfo_t		* si,		// valid data
    u16			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	long num;
	if (!ScanUValueSI(si,&num,force_hex))
	    *dest++ = num;
	else
	{
	    while ( n-- >= 0 )
		*dest++ = 0;
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanU32SI
(
    ScanInfo_t		* si,		// valid data
    u32			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	long num;
	if (!ScanUValueSI(si,&num,force_hex))
	    *dest++ = num;
	else
	{
	    while ( n-- >= 0 )
		*dest++ = 0;
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanBE16SI
(
    ScanInfo_t		* si,		// valid data
    u16			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	long num;
	if (!ScanUValueSI(si,&num,force_hex))
	    write_be16(dest++,num);
	else
	{
	    while ( n-- >= 0 )
		write_be16(dest++,0);
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanBE32SI
(
    ScanInfo_t		* si,		// valid data
    u32			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	long num;
	if (!ScanUValueSI(si,&num,force_hex))
	    write_be32(dest++,num);
	else
	{
	    while ( n-- >= 0 )
		write_be32(dest++,0);
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanBE32SI_swap
(
    ScanInfo_t		* si,		// valid data
    u32			* dest,		// destination list
    int			n,		// number of elements in 'dest'
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	long num;
	if (!ScanUValueSI(si,&num,force_hex))
	{
	    u16 temp[2];
	    write_be32(temp,num);
	    ((u16*)dest)[0] = be16(temp);
	    ((u16*)dest)[1] = be16(temp+1);
	    dest++;
	}
	else
	{
	    while ( n-- >= 0 )
		write_be32(dest++,0);
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanFloatSI
(
    ScanInfo_t		* si,		// valid data
    float		* dest,		// destination list
    int			n		// number of elements in 'dest'
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	double num;
	if (!ScanDValueSI(si,&num))
	    *dest++ = num;
	else
	{
	    while ( n-- >= 0 )
		*dest++ = 0;
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanFloatV2SI
(
    ScanInfo_t		* si,		// valid data
    float		* dest,		// destination list
    int			n		// number of 2d vectors in 'dest'
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    float *end = dest + 2*n;

    enumError err = ERR_OK;
    while ( n-- > 0 )
    {
	DEFINE_VAR(var);
	err = ScanValueSI(si,&var);
	if (err)
	    break;
	*dest++ = GetXDoubleV(&var);
	SkipCharSI(si,',');

	if ( var.mode == VAR_VECTOR )
	    *dest++ = GetZDoubleV(&var);
	else
	{
	    err = ScanValueSI(si,&var);
	    if (err)
		break;
	    *dest++ = GetZDoubleV(&var);
	    SkipCharSI(si,',');
	}
    }

    while ( dest < end )
	*dest++ = 0.0;

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanFloatV3SI
(
    ScanInfo_t		* si,		// valid data
    float		* dest,		// destination list
    int			n		// number of 3d vectors in 'dest'
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    float *end = dest + 3*n;

    enumError err = ERR_OK;
    while ( n-- > 0 )
    {
	DEFINE_VAR(var);
	err = ScanValueSI(si,&var);
	if (err)
	    break;
	*dest++ = GetXDoubleV(&var);
	SkipCharSI(si,',');

	if ( var.mode == VAR_VECTOR )
	{
	    *dest++ = GetYDoubleV(&var);
	    *dest++ = GetZDoubleV(&var);
	}
	else
	{
	    err = ScanValueSI(si,&var);
	    if (err)
		break;
	    *dest++ = GetYDoubleV(&var);
	    SkipCharSI(si,',');

	    err = ScanValueSI(si,&var);
	    if (err)
		break;
	    *dest++ = GetZDoubleV(&var);
	    SkipCharSI(si,',');
	}
    }

    while ( dest < end )
	*dest++ = 0.0;

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanDoubleSI
(
    ScanInfo_t		* si,		// valid data
    double		* dest,		// destination list
    int			n		// number of elements in 'dest'
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    while ( n-- > 0 )
    {
	double num;
	if (!ScanDValueSI(si,&num))
	    *dest++ = num;
	else
	{
	    while ( n-- >= 0 )
		*dest++ = 0;
	    return ERR_WARNING;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanDoubleV3SI
(
    ScanInfo_t		* si,		// valid data
    double		* dest,		// destination list
    int			n		// number of 3d vectors in 'dest'
)
{
    DASSERT(si);
    DASSERT(dest);
    DASSERT(n>0);

    double *end = dest + 3*n;

    enumError err = ERR_OK;
    while ( n-- > 0 )
    {
	DEFINE_VAR(var);
	err = ScanValueSI(si,&var);
	if (err)
	    break;
	*dest++ = GetXDoubleV(&var);
	SkipCharSI(si,',');

	if ( var.mode == VAR_VECTOR )
	{
	    *dest++ = GetYDoubleV(&var);
	    *dest++ = GetZDoubleV(&var);
	}
	else
	{
	    err = ScanValueSI(si,&var);
	    if (err)
		break;
	    *dest++ = GetYDoubleV(&var);
	    SkipCharSI(si,',');

	    err = ScanValueSI(si,&var);
	    if (err)
		break;
	    *dest++ = GetZDoubleV(&var);
	    SkipCharSI(si,',');
	}
    }

    while ( dest < end )
	*dest++ = 0.0;

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SetVarSI
(
    ScanInfo_t		* si,		// valid data
    VarMap_t		* vm,		// valid varmap
    TextCommand_t	mode		// TCMD_DEF*
)
{
    DASSERT(si);
    DASSERT(vm);

    enum EnumMode { EM_OFF, EM_ENUM, EM_SHIFT } enum_mode;
    switch (mode)
    {
	case TCMD_PDEF_ENUM:	enum_mode = EM_ENUM;  mode = TCMD_PDEF; break;
	case TCMD_PDEF_SHIFT:	enum_mode = EM_SHIFT; mode = TCMD_PDEF; break;
	case TCMD_LDEF_ENUM:	enum_mode = EM_ENUM;  mode = TCMD_LDEF; break;
	case TCMD_LDEF_SHIFT:	enum_mode = EM_SHIFT; mode = TCMD_LDEF; break;
	case TCMD_GDEF_ENUM:	enum_mode = EM_ENUM;  mode = TCMD_GDEF; break;
	case TCMD_GDEF_SHIFT:	enum_mode = EM_SHIFT; mode = TCMD_GDEF; break;
	default:		enum_mode = EM_OFF; break;
    }

    DEFINE_VAR(next_val);
    AssignIntV( &next_val, enum_mode == EM_SHIFT );

 restart:;
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    int xyz;
    char name[VARNAME_SIZE+1];
    if (!ScanNameSI(si,name,sizeof(name),false,true,&xyz))
    {
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Missing name [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	return ERR_WARNING;
    }
    if ( mode != TCMD_PDEF && mode != TCMD_LDEF && mode != TCMD_GDEF )
	xyz = -2;

    char ch = NextCharSI(si,false);
    bool have_param = ch && ch != ',';
    bool overwrite  = have_param || enum_mode != EM_OFF;
    if ( ch == '?' )
    {
	sf->ptr++;
	overwrite = false;
    }

    DEFINE_VAR(val);
    if (have_param)
    {
	if ( NextCharSI(si,false) != '=' )
	{
	    if ( si->no_warn <= 0 )
	    {
		ccp eol = FindNextLineFeedSI(si,true);
		ERROR0(ERR_WARNING,
			    "Missing '=' [%s @%u]: %.*s\n",
			    sf->name, sf->line,
			    (int)(eol - sf->prev_ptr), sf->prev_ptr );
	    }
	    return ERR_WARNING;
	}
	sf->ptr++;

	enumError err = ScanExprSI(si,&val);
	if (err)
	    return err;
    }
    else if ( enum_mode != EM_OFF )
    {
	AssignVar(&val,&next_val);
	have_param = true;
	noPRINT_IF( val.mode == VAR_INT,    "NEXT: %s = %d\n",name,GetIntV(&val));
	noPRINT_IF( val.mode == VAR_DOUBLE, "NEXT: %s ~ %g\n",name,GetDoubleV(&val));
    }
    else
	val.mode = VAR_UNSET;

    switch (enum_mode)
    {
	case EM_ENUM:
	    if ( val.mode >= VAR_DOUBLE )
		AssignDoubleV(&next_val,GetDoubleV(&val)+1);
	    else
		AssignIntV(&next_val,GetIntV(&val)+1);
	    break;

	case EM_SHIFT:
	    if ( val.mode >= VAR_DOUBLE )
		AssignDoubleV(&next_val, GetDoubleV(&val) * 2 );
	    else
		AssignIntV(&next_val, GetIntV(&val) << 1 );
	    break;

	default:
	    break;
    }

    bool found;
    Var_t *var = OverwriteVarSI( 0, vm, name, &xyz, &found );

    DASSERT(var);
    if ( overwrite || !found )
    {
	if ( xyz >= 0 )
	{
	    DASSERT( xyz <= 2 );
	    DASSERT( var->mode == VAR_VECTOR );
	    var->v[xyz] = GetDoubleV(&val);
	}
	else
	    AssignVarMode(var,&val,mode);
    }
    else if (!have_param)
	ToVarMode(var,mode);

    if ( NextCharSI(si,false) == ',' )
    {
	sf->ptr++;
	goto restart;
    }

    return CheckEolSI(si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanAssignIntSI
(
    ScanInfo_t		* si,		// valid data
    int			* value		// result: scanned value
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    DASSERT(value);

    if ( NextCharSI(si,false) == '=' )
	si->cur_file->ptr++;

    DEFINE_VAR(val);
    enumError err = ScanExprSI(si,&val);
    *value = GetIntV(&val);
    return err ? err : CheckEolSI(si);
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanAssignDoubleSI
(
    ScanInfo_t		* si,		// valid data
    double		* value		// result: scanned value
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    DASSERT(value);

    if ( NextCharSI(si,false) == '=' )
	si->cur_file->ptr++;

    DEFINE_VAR(val);
    enumError err = ScanExprSI(si,&val);
    *value = GetDoubleV(&val);
    return err ? err : CheckEolSI(si);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanParamSI
(
    ScanInfo_t		* si,		// valid data
    const ScanParam_t	* sp		// NULL or scan parameter
					// list ends with sp->name==NULL
)
{
    DASSERT(si);

    static const ScanParam_t ptab[] = {{0}};
    if (!sp)
	sp = ptab;

 #if HAVE_PRINT0
    const ScanParam_t *sp0 = sp;
 #endif

    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    char ch = NextCharSI(si,false);
    if ( ch == '@' )
    {
	sf->ptr++;
	ch = NextCharSI(si,false);
    }

    char name[VARNAME_SIZE+1];
    if (!ScanNameSI(si,name,sizeof(name),true,true,0))
    {
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Missing name [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	return ERR_WARNING;
    }

    for ( ; sp->name; sp++ )
	if (!strcmp(name,sp->name))
	    break;

    if (!sp->name)
    {
	if ( si->no_warn <= 0 )
	{
	 #if HAVE_PRINT0
	    PRINT(">>> SEARCH: |%s|\n",name);
	    for ( ; sp0->name; sp0++ )
		PRINT(">>> HAVE NAME: |%s| => %d\n",sp0->name,!strcmp(name,sp0->name));
	 #endif

	    sf->line_err++;
	    si->total_err++;
	    ERROR0(ERR_WARNING,
		    "Unknown key [%s @%u]: %s\n",
		    sf->name, sf->line, name );
	}

	GotoEolSI(si);
	return ERR_WARNING;
    }

    if ( sp->mode == SPM_INC )
    {
	// special case; increment value
	if (sp->result)
	    ((int*)sp->result)[0]++;
	if (sp->n_result)
	    *sp->n_result = 0;

	CheckEolSI(si);
	return ERR_OK;
    }

    if ( NextCharSI(si,false) == '=' )
	sf->ptr++;

    if ( sp->mode == SPM_STRING ) // special case: STRING
	return ConcatStringsSI(si,(char**)sp->result,0);

    sf->disable_comma++;

    enumError max_err = ERR_OK;
    const uint min = sp->min > 0 ? sp->min : 1;
    const uint max = sp->max > min ? sp->max : min;
    uint i, n_result = 0, skip = 0;
    for ( i = 0; i < max; i++ )
    {
	if ( i && NextCharSI(si,false) == ',' )
	    sf->ptr++;

	if ( i >= min && !NextCharSI(si,false) )
	    skip++;

	DEFINE_VAR(val);
	if (skip)
	    val.mode = VAR_UNSET;
	else
	{
	    enumError err = ScanExprSI(si,&val);
	    if ( err > ERR_WARNING )
		skip++;
	    else
		n_result++;
	    if ( max_err < err )
		 max_err = err;
	}

	if (sp->result)
	 switch(sp->mode)
	 {
	    case SPM_NONE:
	    case SPM_INC:
		break;

	    case SPM_BOOL:
		((bool*)sp->result)[i] = GetBoolV(&val);
		break;

	    case SPM_U8:
	    case SPM_S8:
		((u8*)sp->result)[i] = GetIntV(&val);
		break;

	    case SPM_U16:
	    case SPM_S16:
		((u16*)sp->result)[i] = GetIntV(&val);
		break;

	    case SPM_U32:
	    case SPM_S32:
		((u32*)sp->result)[i] = GetIntV(&val);
		break;

	    case SPM_U16_BE:
	    case SPM_S16_BE:
		((u16*)sp->result)[i] = htons(GetIntV(&val));
		break;

	    case SPM_U32_BE:
	    case SPM_S32_BE:
		((u32*)sp->result)[i] = htonl(GetIntV(&val));
		break;

	    case SPM_INT:
	    case SPM_UINT:
		((uint*)sp->result)[i] = GetIntV(&val);
		break;

	    case SPM_FLOAT:
		((float*)sp->result)[i] = GetDoubleV(&val);
		break;

	    case SPM_FLOAT_BE:
		write_bef4( ((float*)sp->result)+i, GetDoubleV(&val) );
		break;

	    case SPM_DOUBLE:
		((double*)sp->result)[i] = GetDoubleV(&val);
		break;

	    case SPM_DOUBLE_X:
		((double*)sp->result)[i] = GetXDoubleV(&val);
		break;

	    case SPM_DOUBLE_Y:
		((double*)sp->result)[i] = GetYDoubleV(&val);
		break;

	    case SPM_DOUBLE_Z:
		((double*)sp->result)[i] = GetZDoubleV(&val);
		break;

	    case SPM_FLOAT3:
		((float3*)sp->result)[i] = GetVectorFV(&val);
		break;

	    case SPM_FLOAT3_BE:
		{
		    float3 src = GetVectorFV(&val);
		    float3 *dest = ((float3*)sp->result)+i;
		    write_bef4(&dest->x,src.x);
		    write_bef4(&dest->y,src.y);
		    write_bef4(&dest->z,src.z);
		}
		break;

	    case SPM_VAR:
		AssignVar(((Var_t*)sp->result)+i,&val);
		break;

	    case SPM_STRING:
		ASSERT(0);
		break;
	 }
    }
    if (sp->n_result)
	*sp->n_result = n_result;

    sf->disable_comma--;
    CheckEolSI(si);
    return max_err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanHexlineSI
(
    ScanInfo_t		* si,		// valid data
    FastBuf_t		* dest,		// valid, append scanned data here
    bool		skip_addr	// true: line may start with address field
)
{
    DASSERT(si);
    DASSERT(dest);

    NextCharSI(si,false);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    //--- skip address field

    if (skip_addr)
    {
	ccp ptr = sf->ptr;
	while (isalnum((int)*ptr))
	    ptr++;
	if ( ptr > sf->ptr && *ptr == ':' )
	    sf->ptr = ptr + 1;
    }

    Var_t val;
    InitializeV(&val);

    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,false);
	if (!ch)
	    break;

	if ( ch == ':' || ch == '/' || ch == '#' )
	{
	    GotoEolSI(si);
	    break;
	}

	if ( ch == '(' )
	{
	    sf->ptr++;
	    err = ScanExprSI(si,&val);
	    err = CheckWarnSI(si,')',err);
	    if (err)
		break;

	    switch (val.mode)
	    {
	     case VAR_UNSET:
		// nothing to do
		break;

	     case VAR_INT:
		AppendInt64FastBuf( dest, val.i, val.int_mode ? val.int_mode : IMD_BE4 );
		break;

	     case VAR_DOUBLE:
		{
		    float f = val.d;
		    AppendBE32FastBuf(dest,*(u32*)&f);
		}
		break;

	     case VAR_VECTOR:
		{
		    float f = val.x;
		    AppendBE32FastBuf(dest,*(u32*)&f);
		    f = val.y;
		    AppendBE32FastBuf(dest,*(u32*)&f);
		    f = val.z;
		    AppendBE32FastBuf(dest,*(u32*)&f);
		}
		break;

	     case VAR_STRING:
		AppendFastBuf(dest,val.str,val.str_len);
		break;
	    }
	}
	else
	{
	    u8 data;
	    err = ScanU8SI(si,&data,1,true);
	    if (err)
		break;

	    AppendCharFastBuf(dest,data);
	}
    }

    CheckEolSI(si);
    ResetV(&val);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			text commands			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_SkipToENDIF
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    DASSERT(si->cur_file);

    const int start_level = si->cur_file->if_level;
    for(;;)
    {
	const char ch = NextLineSI(si,true,false); // don't use 'sf'
	if (!ch)
	    return ERR_WARNING;
	if ( ch != '@' )
	    continue;

	const TextCommand_t tcmd = ScanTextCommand(si);
	if ( tcmd == TCMD_IF )
	    si->cur_file->if_level++;
	else if ( tcmd == TCMD_ENDIF && --si->cur_file->if_level < start_level )
	    return ERR_OK;
    }
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_IF
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    TRACE("### IF ###\n");
    si->cur_file->if_level++;

    DEFINE_VAR(cond);
    enumError err = ScanExprSI(si,&cond);
    if (err)
	return err;

    err = CheckEolSI(si);
    if (err)
	return err;

    while (!GetBoolV(&cond))
    {
	const char ch = NextLineSI(si,true,false);
	DASSERT(si->cur_file);
	if (!ch)
	    return ERR_WARNING;
	if ( ch != '@' )
	    continue;

	const TextCommand_t tcmd = ScanTextCommand(si);
	switch (tcmd)
	{
	    case TCMD_IF:
		si->cur_file->if_level++;
		SI_SkipToENDIF(si);
		break;

	    case TCMD_ELIF:
		err = ScanExprSI(si,&cond);
		if (err)
		    return err;

		err = CheckEolSI(si);
		if (err)
		    return err;
		break;

	    case TCMD_ELSE:
		return CheckEolSI(si);

	    case TCMD_ENDIF:
		si->cur_file->if_level--;
		return CheckEolSI(si);

	    default:
		break;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ELSE
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    TRACE("### ELSE/ELIF ###\n");

    if ( si->cur_file->if_level <= 0 )
    {
	if ( si->no_warn <= 0 )
	{
	    si->total_err++;
	    ERROR0(ERR_WARNING,
		"Illegal '@ELSE' or '@ELIF' -> ignored [%s @%u]\n",
			si->cur_file->name, si->cur_file->line );
	}
	return ERR_WARNING;
    }

    return SI_SkipToENDIF(si);
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDIF
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    TRACE("### ENDIF ###\n");

    CheckEolSI(si);
    if ( sf->if_level <= 0 )
    {
	if ( si->no_warn <= 0 )
	{
	    si->total_err++;
	    ERROR0(ERR_WARNING,
		"Illegal '@ENDIF' -> ignored [%s @%u]\n",
			sf->name, sf->line );
	}
	return ERR_WARNING;
    }

    sf->if_level--;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_DOIF
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("### DOIF ###\n");

    enumError err = CheckWarnSI(si,'(',ERR_OK);
    if (!err)
    {
	DEFINE_VAR(cond);
	err = ScanExprSI(si,&cond);
	err = CheckWarnSI(si,')',err);
	if ( err || !GetBoolV(&cond) )
	    GotoEolSI(si);
    }
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// loop helpers

static ccp loop_name[] = // same order as TCMD_LOOP... & TCMD_ENDLOOP...
{
    "LOOP",
    "REPEAT",
    "FOR",
    "FOREACH",
    "WHILE"
};

//-----------------------------------------------------------------------------

static ccp GetLoopName ( TextCommand_t tcmd_end )
{
    return tcmd_end >= TCMD_LOOP && tcmd_end <= TCMD_WHILE
	? loop_name[tcmd_end-TCMD_LOOP]
	: tcmd_end >= TCMD_ENDLOOP && tcmd_end <= TCMD_ENDWHILE
		? loop_name[tcmd_end-TCMD_ENDLOOP]
		: "?";
}

//-----------------------------------------------------------------------------

static ccp GetLoopNameL ( const ScanLoop_t * loop )
{
    return loop ? GetLoopName(loop->tcmd_end) : "-";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ScanLoop_t * SI_CheckLoop
(
    ScanInfo_t		* si,		// valid data
    TextCommand_t	tcmd_end,	// loop termiantion command
    enumError		* return_stat	// store return status (never NULL)
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    DASSERT( sf->loop_level >= 0 && sf->loop_level <= MAX_LOOP_DEPTH );
    DASSERT( tcmd_end >= TCMD_ENDLOOP && tcmd_end <= TCMD_ENDWHILE );
    DASSERT(return_stat);

    noPRINT("##%s## SI_CheckLoop() line=%u, loop_level=%d\n",
		GetLoopName(tcmd_end), sf->line, sf->loop_level );

    if ( sf->loop_level <  MAX_LOOP_DEPTH )
    {
	ScanLoop_t * loop = sf->loop + sf->loop_level++;

	loop->tcmd_end	= tcmd_end;
	loop->ptr	= sf->ptr;
	loop->line	= sf->line;
	loop->if_level	= sf->if_level;

	loop->count_val	= 0;
	loop->end_val	= 1;
	loop->step_val	= 1;

	FreeString(loop->varname);
	loop->varname	= 0;
	loop->expr_ptr	= 0;
	loop->expr_line	= 0;

	*return_stat = ERR_OK;
	return loop;
    }

    if ( si->no_warn <= 0 )
    {
	si->total_err++;
	ERROR0(ERR_WARNING,
		"To many nested loops (max=%u) [%s @%u]\n",
		MAX_LOOP_DEPTH, sf->name, sf->line );
    }

    *return_stat = ERR_WARNING;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static ScanLoop_t * SI_CheckEndLoop
(
    ScanInfo_t		* si,		// valid data
    TextCommand_t	tcmd_end,	// loop termiantion command
    enumError		* return_stat	// store return status (never NULL)
)
{
    DASSERT( si );
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    DASSERT( sf->loop_level >= 0 && sf->loop_level <= MAX_LOOP_DEPTH );
    DASSERT( !tcmd_end || tcmd_end >= TCMD_ENDLOOP && tcmd_end <= TCMD_ENDWHILE );
    DASSERT( return_stat );
    noPRINT("##%s## SI_CheckEndLoop() line=%u, loop_level=%d\n",
		GetLoopName(tcmd_end), sf->line, sf->loop_level );

    enumError err = CheckEolSI(si);
    const int loop_level = sf->loop_level;
    DASSERT( loop_level >= 0 && loop_level <= MAX_LOOP_DEPTH );
    if (loop_level)
    {
	ScanLoop_t * loop = sf->loop + loop_level - 1;
	if ( !tcmd_end || loop->tcmd_end == tcmd_end )
	{
	    loop->count_val += loop->step_val;
	    *return_stat = err;
	    return loop;
	}
    }

    if ( si->no_warn <= 0 && tcmd_end )
    {
	si->total_err++;
	if (loop_level)
	{
	    ScanLoop_t * loop = sf->loop + loop_level - 1;
	    ERROR0(ERR_WARNING,
		"End of loop command '@END%s' does not match current '%s' loop [%s @%u]\n",
		GetLoopName(tcmd_end), GetLoopName(loop->tcmd_end),
		sf->name, sf->line );
	}
	else
	{
	    ERROR0(ERR_WARNING,
		"End of loop command 'END%s' not allowed outside a loop [%s @%u]\n",
		GetLoopName(tcmd_end), sf->name, sf->line );
	}
    }

    *return_stat = ERR_WARNING;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void SI_RestartLoop
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// valid loop pointer
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    DASSERT(loop);
    DASSERT( loop == sf->loop + sf->loop_level - 1 );
    noPRINT("##%s## SI_RestartLoop()\n", GetLoopName(loop->tcmd_end) );

    sf->ptr		= loop->ptr;
    sf->line		= loop->line;
    sf->if_level	= loop->if_level;
}

///////////////////////////////////////////////////////////////////////////////

static void SI_ExitLoop
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// valid loop pointer
)
{
    // exit loop at end-loop-command

    DASSERT(si);
    noPRINT("##%s## SI_ExitLoop()\n", GetLoopNameL(loop) );

    if (loop)
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	DASSERT( loop == sf->loop + sf->loop_level - 1 );

	sf->if_level = loop->if_level;
	if ( --sf->loop_level > 0 )
	{
	    loop--;
	    if ( loop >= sf->loop && loop->varname )
	    {
		Var_t *var = InsertVarMap(&si->lvar,loop->varname,false,0,0);
		var->mode  = VAR_INT;
		var->i     = loop->count_val;
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

static void SI_TerminateLoop
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// valid loop pointer
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    DASSERT(loop);
    DASSERT( loop == si->cur_file->loop + si->cur_file->loop_level - 1 );
    noPRINT("##%s## SI_TerminateLoop()\n", GetLoopName(loop->tcmd_end) );

    const TextCommand_t tcmd_beg = TCMD_LOOP - TCMD_ENDLOOP + loop->tcmd_end;
    int level = 1;
    for(;;)
    {
	const char ch = NextLineSI(si,true,false);
	DASSERT(si->cur_file);
	if (!ch)
	    return;
	if ( ch != '@' )
	    continue;

	const TextCommand_t tcmd = ScanTextCommand(si);
	if ( tcmd == tcmd_beg )
	    level++;
	else if ( tcmd == loop->tcmd_end && !--level )
	    break;
    }

    CheckEolSI(si);
    SI_ExitLoop(si,loop);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			loop commands			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_LOOP
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##LOOP(BEGIN)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckLoop(si,TCMD_ENDLOOP,&err);
    if (loop)
    {
	if (err)
	    GotoEolSI(si);
	else
	    CheckEolSI(si);
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	loop->ptr  = sf->ptr;
	loop->line = sf->line;
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDLOOP
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##LOOP(END)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckEndLoop(si,TCMD_ENDLOOP,&err);
    if (loop)
	SI_RestartLoop(si,loop);
    else
	SI_ExitLoop(si,loop);
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_REPEAT
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##REPEAT(BEGIN)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckLoop(si,TCMD_ENDREPEAT,&err);
    if (loop)
    {
	DEFINE_VAR(val);
	err = ScanExprSI(si,&val);
	loop->end_val = GetIntV(&val);
	noPRINT("-> count=%d/%d +%d\n",loop->count_val,loop->end_val,loop->step_val);

	if (err)
	    GotoEolSI(si);
	else
	    CheckEolSI(si);

	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	loop->ptr  = sf->ptr;
	loop->line = sf->line;

	if ( loop->count_val >= loop->end_val )
	    SI_TerminateLoop(si,loop);
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDREPEAT
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##REPEAT(END)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckEndLoop(si,TCMD_ENDREPEAT,&err);
    if ( loop && loop->count_val < loop->end_val )
	SI_RestartLoop(si,loop);
    else
	SI_ExitLoop(si,loop);
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError scan_for_varname
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// valid loop pointer
)
{
    DASSERT(si);
    DASSERT(loop);

    char name[VARNAME_SIZE+1];
    if (!ScanNameSI(si,name,sizeof(name),false,true,0))
    {
	if ( si->no_warn <= 0 )
	{
	    ScanFile_t *sf = si->cur_file;
	    DASSERT(sf);
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Missing name [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	return ERR_WARNING;
    }

    loop->varname = STRDUP(name);
    return CheckWarnSI(si,'=',0);
}

//-----------------------------------------------------------------------------

static enumError SI_FOR
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##FOR(BEGIN)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckLoop(si,TCMD_ENDFOR,&err);
    if (loop)
    {
	err = scan_for_varname(si,loop);
	if (err)
	    goto abort;

	DEFINE_VAR(val);
	err = ScanExprSI(si,&val);
	if (err)
	    goto abort;
	loop->count_val = GetIntV(&val);

	err = CheckWarnSI(si,';',err);
	if (err)
	    goto abort;
	err = ScanExprSI(si,&val);
	if (err)
	    goto abort;
	loop->end_val = GetIntV(&val);

	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	if ( NextCharSI(si,false) == ';' )
	{
	    sf->ptr++;
	    err = ScanExprSI(si,&val);
	    if (err)
		goto abort;
	    loop->step_val = GetIntV(&val);
	}

	if (err)
	    GotoEolSI(si);
	else
	    CheckEolSI(si);
	loop->ptr  = sf->ptr;
	loop->line = sf->line;

	noPRINT("-> FOR %s = %d -> %d, step = %d\n",
		loop->varname, loop->count_val, loop->end_val, loop->step_val );

	Var_t *var = InsertVarMap(&sf->pvar,loop->varname,false,0,0);
	var->mode = VAR_INT;
	var->i = loop->count_val;

	if (   loop->step_val > 0 && loop->count_val > loop->end_val
	    || loop->step_val < 0 && loop->count_val < loop->end_val )
		goto abort;
    }
    return err;

 abort:
    GotoEolSI(si);
    if (loop)
	SI_TerminateLoop(si,loop);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDFOR
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// not NULL: SI_CheckEndLoop() already done
					//  + call TermLoop() instead of ExitLoop()
)
{
    DASSERT(si);
    noPRINT("##FOR(END)##\n");

    enumError err = ERR_OK;
    bool use_term = loop != 0;
    if (!loop)
	loop = SI_CheckEndLoop(si,TCMD_ENDFOR,&err);

    if (loop)
    {
	if (   loop->step_val > 0 && loop->count_val > loop->end_val
	    || loop->step_val < 0 && loop->count_val < loop->end_val )
	{
	    if (use_term)
		SI_TerminateLoop(si,loop);
	    else
		SI_ExitLoop(si,loop);
	}
	else
	{
	    DASSERT(si->cur_file);
	    Var_t *var = InsertVarMap(&si->cur_file->pvar,loop->varname,false,0,0);
	    var->mode = VAR_INT;
	    var->i = loop->count_val;
	    SI_RestartLoop(si,loop);
	}
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_FOREACH
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##WHILE(BEGIN)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckLoop(si,TCMD_ENDFOREACH,&err);
    if (loop)
    {
	err = scan_for_varname(si,loop);
	if (err)
	{
	    GotoEolSI(si);
	    SI_TerminateLoop(si,loop);
	    return err;
	}

	if (!NextCharSI(si,false))
	    SI_TerminateLoop(si,loop);
	else
	{
	    ScanFile_t *sf = si->cur_file;
	    DASSERT(sf);
	    Var_t *var = InsertVarMap(&sf->pvar,loop->varname,false,0,0);
	    err = ScanExprSI(si,var);

	    loop->expr_ptr  = sf->ptr;
	    loop->expr_line = sf->line;

	    GotoEolSI(si);
	    loop->ptr  = sf->ptr;
	    loop->line = sf->line;
	}
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDFOREACH
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// not NULL: SI_CheckEndLoop() already done
					//  + call TermLoop() instead of ExitLoop()
)
{
    DASSERT(si);
    noPRINT("##WHILE(END)##\n");

    enumError err = ERR_OK;
    bool use_term = loop != 0;
    if (!loop)
	loop = SI_CheckEndLoop(si,TCMD_ENDFOREACH,&err);

    if (loop)
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	DASSERT(loop->expr_ptr);
	ccp  saved_ptr  = sf->ptr;
	uint saved_line = sf->line;
	sf->ptr  = loop->expr_ptr;
	sf->line = loop->expr_line;

	if ( NextCharSI(si,false) == ';' )
	    sf->ptr++;

	if (NextCharSI(si,false))
	{
	    Var_t *var = InsertVarMap(&sf->pvar,loop->varname,false,0,0);
	    ccp saved_ptr = sf->ptr;
	    err = ScanExprSI(si,var);
	    if ( sf->ptr > saved_ptr )
	    {
		loop->expr_ptr  = sf->ptr;
		loop->expr_line = sf->line;
		SI_RestartLoop(si,loop);
		return err;
	    }
	}

	sf->ptr  = saved_ptr;
	sf->line = saved_line;
    }

    if (use_term)
	SI_TerminateLoop(si,loop);
    else
	SI_ExitLoop(si,loop);

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_WHILE
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("##WHILE(BEGIN)##\n");

    enumError err;
    ScanLoop_t * loop = SI_CheckLoop(si,TCMD_ENDWHILE,&err);
    if (loop)
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	loop->expr_ptr  = sf->ptr;
	loop->expr_line = sf->line;

	DEFINE_VAR(val);
	err = ScanExprSI(si,&val);

	if (err)
	    GotoEolSI(si);
	else
	    CheckEolSI(si);
	loop->ptr  = sf->ptr;
	loop->line = sf->line;

	if (!GetBoolV(&val))
	    SI_TerminateLoop(si,loop);
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDWHILE
(
    ScanInfo_t		* si,		// valid data
    ScanLoop_t		* loop		// not NULL: SI_CheckEndLoop() already done
					//  + call TermLoop() instead of ExitLoop()
)
{
    DASSERT(si);
    noPRINT("##WHILE(END)##\n");

    enumError err = ERR_OK;
    bool use_term = loop != 0;
    if (!loop)
	loop = SI_CheckEndLoop(si,TCMD_ENDWHILE,&err);

    if (loop)
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	DASSERT(loop->expr_ptr);
	ccp  saved_ptr  = sf->ptr;
	uint saved_line = sf->line;
	sf->ptr  = loop->expr_ptr;
	sf->line = loop->expr_line;

	DEFINE_VAR(val);
	err = ScanExprSI(si,&val);
	if ( !err && GetBoolV(&val) )
	{
	    SI_RestartLoop(si,loop);
	    return ERR_OK;
	}

	sf->ptr  = saved_ptr;
	sf->line = saved_line;
    }

    if (use_term)
	SI_TerminateLoop(si,loop);
    else
	SI_ExitLoop(si,loop);

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_BreakLoop
(
    ScanInfo_t		* si,		// valid data
    bool		is_continue	// true: is @CONTINUE
)
{
    DASSERT(si);

    ccp tcmd_name = is_continue ? "CONTINUE" : "BREAK";
    noPRINT("##%s##\n",tcmd_name);

    enumError err = ERR_OK;
    char ch = NextCharSI(si,false);
    int level = 1;
    if ( ch && ch != '@' )
    {
	DEFINE_VAR(val);
	err = ScanExprSI(si,&val);
	level = GetIntV(&val);
	if ( level < 1 )
	    level = 1;
	ch = NextCharSI(si,false);
    }

    bool cond = true;
    if ( ch == '@' )
    {
	const TextCommand_t tcmd = ScanTextCommand(si);
	if ( tcmd != TCMD_IF )
	{
	    ScanFile_t *sf = si->cur_file;
	    DASSERT(sf);
	    ERROR0(ERR_WARNING,
		"Commandline '@%s ... @IF ...' expected [%s @%u]\n",
		tcmd_name, sf->name, sf->line );
	}
	else
	{
	    DEFINE_VAR(val);
	    err = ScanExprSI(si,&val);
	    if (!err)
		cond = GetBoolV(&val);
	}
    }
    CheckEolSI(si);
    noPRINT(""##%s##\n" -> cond = %d, level = %d\n",cond,level);
    if (!cond)
	return err;

    while ( level-- > 0 )
    {
	ScanLoop_t * loop = SI_CheckEndLoop(si,0,&err);
	if (!loop)
	    break;

	if ( !is_continue || level > 0 )
	    SI_TerminateLoop(si,loop);
	else
	{
	  // CONTINUE
	  switch(loop->tcmd_end)
	  {
	    case TCMD_ENDLOOP:
		SI_RestartLoop(si,loop);
		break;

	    case TCMD_ENDREPEAT:
		if ( loop->count_val < loop->end_val )
		    SI_RestartLoop(si,loop);
		else
		    SI_TerminateLoop(si,loop);
		break;

	    case TCMD_ENDFOR:
		SI_ENDFOR(si,loop);
		break;

	    case TCMD_ENDFOREACH:
		SI_ENDFOREACH(si,loop);
		break;

	    case TCMD_ENDWHILE:
		SI_ENDWHILE(si,loop);
		break;
	  }
	}
    }

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// exec, include and macros

static enumError SI_EXEC
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    char *str = 0;
    uint len = 0;
    enumError err = ConcatStringsSI(si,&str,&len);

    if (!err)
    {
	sf = AddSF(si,str,len,"@EXEC",si->init_revision,0);
	if (sf)
	{
	    sf->data_alloced = true;
	    NextCharSI(si,true);
	}
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_INCLUDE
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    const bool have_quote = NextCharSI(si,false) == '"';
    sf->prev_ptr = sf->ptr;

    ccp start, ptr;
    if (have_quote)
    {
	start = ptr = sf->ptr + 1;
	while ( ptr < sf->end && *ptr && *ptr != '"' )
	    ptr++;

	if ( *ptr == '"' )
	    sf->ptr = ptr + 1;
	else
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Missing '\"' [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	    sf->ptr = ptr;
	}
    }
    else
    {
	start = ptr = sf->ptr;
	while ( ptr < sf->end && (u8)*ptr > ' ' )
	    ptr++;
	sf->ptr = ptr;
    }
    CheckEolSI(si);

    char path[PATH_MAX];
    ccp slash = strrchr(sf->name,'/');
    uint len1 = slash ? slash+1 - sf->name : 0;
    if ( len1 > sizeof(path) - 1 )
	len1 = sizeof(path) - 1;
    memcpy(path,sf->name,len1);

    uint len2 = ptr - start;
    if ( len2 > sizeof(path) - len1 - 1 )
	len2 = sizeof(path) - len1 - 1;
    memcpy(path+len1,start,len2);
    path[len1+len2] = 0;

    PRINT("SI_INCLUDE(%s) quote=%d\n",path,have_quote);

    u8 *data;
    uint size;
    ccp fname;
    enumError err = OpenReadFILE(path,0,false,&data,&size,&fname,0);
    if (err)
    {
	FREE(data);
	FreeString(fname);
    }
    else
    {
	sf = AddSF(si,(ccp)data,size,fname,si->init_revision,0);
	if (sf)
	{
	    sf->data_alloced = sf->name_alloced = true;
	    noPRINT(" -> %.30s\n",sf->ptr);
	    NextCharSI(si,true);
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_RETURN
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("SI_RETURN()\n");

    si->last_result.mode = VAR_UNSET;
    enumError err = ERR_OK;
    if (NextCharSI(si,false))
    {
	err = ScanExprSI(si,&si->last_result);
	CheckEolSI(si);
    }

    DropSF(si);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_EXIT
(
    ScanInfo_t		* si,		// valid data
    bool		is_assert	// true: @ASSERT
)
{
    DASSERT(si);
    PRINT("SI_RETURN()\n");

    enumError err = ERR_OK;
    if (is_assert)
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	ccp prev = sf->ptr;

	DEFINE_VAR(cond);
	ScanExprSI(si,&cond);
	if (GetBoolV(&cond))
	{
	    CheckEolSI(si);
	    return ERR_OK;
	}

	//ccp eol = FindNextLineFeedSI(si,true);
	err = ERROR0(ERR_ERROR,
			"Assertion failed [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(sf->ptr - prev), prev );
	GotoEolSI(si);
    }

    CheckEolSI(si);
    while (DropSF(si))
	;
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError SI_MACRO
(
    ScanInfo_t		* si,		// valid data
    TextCommand_t	tcmd_beg	// command to execute
)
{
    DASSERT(si);

    char mname[VARNAME_SIZE+1];
    bool valid = ScanNameSI(si,mname,sizeof(mname),false,true,0) > 0;
    CheckEolSI(si);

    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    ccp  macro_begin = sf->ptr;
    ccp  macro_end   = macro_begin;
    uint macro_line  = sf->line;

    //--- skip macro body

    const TextCommand_t tcmd_end = TCMD_ENDMACRO - TCMD_MACRO + tcmd_beg;
    int level = 1;
    for(;;)
    {
	const char ch = NextLineSI(si,true,false);
	DASSERT(si->cur_file);
	if (!ch)
	{
	    ERROR0(ERR_WARNING,
			"End of source reached, but %s '%s' not terminated [%s @%u]\n",
			tcmd_beg == TCMD_FUNCTION ? "function" : "macro",
			mname, sf->name, macro_line );
	    macro_end = sf->ptr;
	    valid = false;
	    break;
	}
	if ( ch != '@' )
	    continue;

	macro_end = sf->ptr;
	const TextCommand_t tcmd = ScanTextCommand(si);
	if ( tcmd == tcmd_beg )
	    level++;
	else if ( tcmd == tcmd_end && !--level )
	    break;
    }

    if (valid)
    {
	bool found;
	Var_t * mvar = InsertVarMap(&si->macro,mname,false,0,&found);
	DASSERT(mvar);
	ScanMacro_t *macro;
	if (found)
	{
	    macro = mvar->macro;
	    DASSERT(macro);
	    ResetSM(macro);
	}
	else
	{
	    //DASSERT(!mvar->macro);
	    macro = MALLOC(sizeof(*macro));
	    DASSERT(macro);
	    InitializeSM(macro);
	    mvar->macro = macro;
	}

	DASSERT( sf == si->cur_file );
	DASSERT( sf->ptr > macro_begin );
	const uint macro_size	= macro_end - macro_begin;
	macro->data		= MEMDUP(macro_begin,macro_size);
	DASSERT(macro->data);
	macro->data_size	= macro_size;
	macro->data_alloced	= true;
	macro->line0		= macro_line;
	macro->map		= mvar;
	macro->src_name		= STRDUP(sf->name);
	macro->is_function	= tcmd_beg == TCMD_FUNCTION;

	PRINT("DEFINE %s '%s' : line=%u, size=%u\n",
		macro->is_function ? "FUNCTION" : "MACRO", mname,
		macro_line, macro_size );
    }

    return valid ? ERR_OK : ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_ENDMACRO
(
    ScanInfo_t		* si,		// valid data
    TextCommand_t	tcmd		// command to execute
)
{
    DASSERT(si);
    PRINT("SI_ENDMACRO(%u) func=%d\n",tcmd,tcmd==TCMD_ENDFUNCTION);
    CheckEolSI(si);

    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    if (!sf->active_macro)
    {
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
		    "No active %s, END%s command ignored [%s @%u]: %.*s\n",
		    tcmd==TCMD_ENDFUNCTION ? "function" : "macro",
		    tcmd==TCMD_ENDFUNCTION ? "FUNCTION" : "MACRO",
		    sf->name, sf->line,
		    (int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	return ERR_WARNING;
    }

    si->last_result.mode = VAR_UNSET;
    DropSF(si);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const ScanMacro_t *ScanMacroNameSI
(
    ScanInfo_t		* si,		// valid data
    char		* mname,	// pointer to name buffer
    uint		mname_size	// size of 'mname'
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);
    DASSERT(mname);
    DASSERT(mname_size>1);

    //--- scan macro name

    if (!ScanNameSI(si,mname,mname_size,false,true,0))
    {
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Missing macro name [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	GotoEolSI(si);
	return 0;
    }


    //--- find macro name

    const Var_t * mvar = FindVarMap(&si->macro,mname,0);
    if (!mvar)
    {
	if ( si->no_warn <= 0 )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    ERROR0(ERR_WARNING,
			"Macro '%s' not found [%s @%u]: %.*s\n",
			mname, sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	}
	GotoEolSI(si);
	return 0;
    }

    DASSERT(mvar->macro);
    return mvar->macro;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_CALL
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("SI_CALL()\n");

    //--- scan macro name

    char mname[VARNAME_SIZE+1];
    const ScanMacro_t *macro = ScanMacroNameSI(si,mname,sizeof(mname));
    if (!macro)
	return ERR_WARNING;


    //--- evaluate parameters

    uint n_param = 0;
    enumError err = ERR_OK;
    VarMap_t pvar;
    InitializeVarMap(&pvar);

    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    if ( NextCharSI(si,false) == '(' )
    {
	sf->ptr++;
	if ( NextCharSI(si,false) != ')' )
	{
	    sf->disable_comma++;
	    for(;;)
	    {
		char pname[20];
		snprintf(pname,sizeof(pname),"$%u",++n_param);
		Var_t *param = InsertVarMap(&pvar,pname,false,0,0);
		DASSERT(param);
		enumError err1 = ScanExprSI(si,param);
		if ( err < err1 )
		    err = err1;
		if ( NextCharSI(si,false) != ',' )
		    break;
		sf->ptr++;
	    }
	    sf->disable_comma--;
	}
	err = CheckWarnSI(si,')',err);
    }
    Var_t *var = InsertVarMap(&pvar,"$N",false,0,0);
    DASSERT(var);
    AssignIntV(var,n_param);
    CheckEolSI(si);


    //--- call macro

    sf = AddSF(si,macro->data,macro->data_size,macro->src_name,sf->revision,&pvar);
    if (sf)
    {
	DASSERT(!pvar.used);
	DASSERT(!pvar.list);
	sf->active_macro = macro;
	sf->line = macro->line0;
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError SI_PARAMETERS
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    noPRINT("SI_PARAMETERS()\n");

    enumError err = ERR_OK;
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    const Var_t *var = FindVarMap(&sf->pvar,"$N",0);
    const int n = var && var->mode != VAR_UNSET ? GetIntV(var) : 0;

    int i;
    for ( i = 1;; i++ )
    {
	while ( NextCharSI(si,false) == ',' )
	    sf->ptr++;

	if (!NextCharSI(si,false))
	    break;

	char varname[VARNAME_SIZE+1];
	int xyz;
	const uint namelen
	    = ScanNameSI(si,varname,sizeof(varname),false,true,&xyz);
	if (!namelen)
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    err = ERROR0(ERR_WARNING,
			"Missing name [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)(eol - sf->prev_ptr), sf->prev_ptr );
	    GotoEolSI(si);
	    break;
	}

	Var_t *dest = OverwriteVarSI(0,&sf->pvar,varname,&xyz,0);
	DASSERT(dest);
	dest->mode = VAR_UNSET;
	noPRINT("DEST#%u/%u: %p |%s|\n",i,n,dest,varname);

	if ( i <= n )
	{
	    char pname[20];
	    snprintf(pname,sizeof(pname),"$%u",i);
	    const Var_t *src = FindVarMap(&sf->pvar,pname,0);
	    noPRINT("SRC#%u/%u:  %p |%s|\n",i,n,src,pname);
	    if (src)
		AssignVar(dest,src);
	}
    }

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TextCommand_t GetTextCommand
(
    ccp			name		// name to scan, upper case is assumed
)
{
    static FormatField_t cmdtab = {0};
    if (!cmdtab.used)
    {
	struct tcmdtab_t { ccp name; int id; };
	static const struct tcmdtab_t def_tab[] =
	{
	    //--- messages

	    { "ECHO",		TCMD_ECHO },
	    { "WARN",		TCMD_WARN },

	    //--- reset var maps

	    { "RESET-PRIVATE",	TCMD_RESET_PRIVATE },
	    { "RESET-LOCAL",	TCMD_RESET_LOCAL },
	    { "RESET-GLOBAL",	TCMD_RESET_GLOBAL },

	    //--- dump var maps

	    { "DUMP-VAR",	TCMD_DUMP_VAR },
	    { "DUMP-PRIVATE",	TCMD_DUMP_PRIVATE },
	    { "DUMP-LOCAL",	TCMD_DUMP_LOCAL },
	    { "DUMP-GLOBAL",	TCMD_DUMP_GLOBAL },
	    { "DUMP-CONST",	TCMD_DUMP_CONST },
	    { "DUMP-PREDEF",	TCMD_DUMP_PREDEF },

	    //--- settings

	    { "REVISION",	TCMD_REVISION },
	    { "SZS-MODIFIER",	TCMD_SZS_MODIFIER },
	    { "SMOD",		TCMD_SZS_MODIFIER },
	    { "SETUP-RANDOM",	TCMD_SETUP_RANDOM },

	    //--- define variables

	    { "DEF",		TCMD_LDEF },
	    { "DEF.I",		TCMD_LDEF_I },
	    { "DEF.F",		TCMD_LDEF_F },
	    { "DEF.S",		TCMD_LDEF_S },
	    { "DEF.V",		TCMD_LDEF_V },
	    { "DEF.X",		TCMD_LDEF_X },
	    { "DEF.Y",		TCMD_LDEF_Y },
	    { "DEF.Z",		TCMD_LDEF_Z },
	    { "DEF.ENUM",	TCMD_LDEF_ENUM },
	    { "DEF.SHIFT",	TCMD_LDEF_SHIFT },

	    { "PDEF",		TCMD_PDEF },
	    { "PDEF.I",		TCMD_PDEF_I },
	    { "PDEF.F",		TCMD_PDEF_F },
	    { "PDEF.S",		TCMD_PDEF_S },
	    { "PDEF.V",		TCMD_PDEF_V },
	    { "PDEF.X",		TCMD_PDEF_X },
	    { "PDEF.Y",		TCMD_PDEF_Y },
	    { "PDEF.Z",		TCMD_PDEF_Z },
	    { "PDEF.ENUM",	TCMD_PDEF_ENUM },
	    { "PDEF.SHIFT",	TCMD_PDEF_SHIFT },

	    { "LDEF",		TCMD_LDEF },
	    { "LDEF.I",		TCMD_LDEF_I },
	    { "LDEF.F",		TCMD_LDEF_F },
	    { "LDEF.S",		TCMD_LDEF_S },
	    { "LDEF.V",		TCMD_LDEF_V },
	    { "LDEF.X",		TCMD_LDEF_X },
	    { "LDEF.Y",		TCMD_LDEF_Y },
	    { "LDEF.Z",		TCMD_LDEF_Z },
	    { "LDEF.ENUM",	TCMD_LDEF_ENUM },
	    { "LDEF.SHIFT",	TCMD_LDEF_SHIFT },

	    { "GDEF",		TCMD_GDEF },
	    { "GDEF.I",		TCMD_GDEF_I },
	    { "GDEF.F",		TCMD_GDEF_F },
	    { "GDEF.S",		TCMD_GDEF_S },
	    { "GDEF.V",		TCMD_GDEF_V },
	    { "GDEF.X",		TCMD_GDEF_X },
	    { "GDEF.Y",		TCMD_GDEF_Y },
	    { "GDEF.Z",		TCMD_GDEF_Z },
	    { "GDEF.ENUM",	TCMD_GDEF_ENUM },
	    { "GDEF.SHIFT",	TCMD_GDEF_SHIFT },

	    //--- define variables (old names)

	    { "NUM",		TCMD_LDEF },
	    { "GNUM",		TCMD_GDEF },
	    { "INT",		TCMD_LDEF_I },
	    { "GINT",		TCMD_GDEF_I },
	    { "FLOAT",		TCMD_LDEF_F },
	    { "GFLOAT",		TCMD_GDEF_F },

	    //--- if then else

	    { "IF",		TCMD_IF },
	    { "ELIF",		TCMD_ELIF },
	    { "ELSEIF",		TCMD_ELIF },
	    { "ELSE",		TCMD_ELSE },
	    { "ENDIF",		TCMD_ENDIF },
	    { "FI",		TCMD_ENDIF },

	    { "DOIF",		TCMD_DOIF },

	    //--- loops

	    { "LOOP",		TCMD_LOOP },
	    { "REPEAT",		TCMD_REPEAT },
	    { "FOR",		TCMD_FOR },
	    { "FOREACH",	TCMD_FOREACH },
	    { "WHILE",		TCMD_WHILE },

	    { "ENDLOOP",	TCMD_ENDLOOP },
	    { "ENDREPEAT",	TCMD_ENDREPEAT },
	    { "ENDFOR",		TCMD_ENDFOR },
	    { "ENDFOREACH",	TCMD_ENDFOREACH },
	    { "ENDEACH",	TCMD_ENDFOREACH },
	    { "ENDWHILE",	TCMD_ENDWHILE },

	    { "BREAK",		TCMD_BREAK },
	    { "CONTINUE",	TCMD_CONTINUE },

	    //--- include & return

	    { "EXEC",		TCMD_EXEC },
	    { "INCLUDE",	TCMD_INCLUDE },
	    { "RETURN",		TCMD_RETURN },
	    { "EXIT",		TCMD_EXIT },
	    { "ASSERT",		TCMD_ASSERT },

	    //--- macros and functions

	    { "MACRO",		TCMD_MACRO },
	    { "ENDMACRO",	TCMD_ENDMACRO },
	    { "FUNCTION",	TCMD_FUNCTION },
	    { "ENDFUNCTION",	TCMD_ENDFUNCTION },

	    { "CALL",		TCMD_CALL },
	    { "PARAMETERS",	TCMD_PARAMETERS },
	    { "PARAM",		TCMD_PARAMETERS },

	    {0,0}
	};

	const struct tcmdtab_t * cp;
	for ( cp = def_tab; cp->name; cp++ )
	    InsertFormatField(&cmdtab,cp->name,0,0,0)->num = cp->id;
    }

    FormatFieldItem_t * item = FindFormatField(&cmdtab,name);
    return item ? item->num : TCMD_NONE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TextCommand_t ScanTextCommand
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    if ( si->fast_scan <= 0 )
    {
	ScanFile_t *sf = si->cur_file;
	DASSERT(sf);
	ScanFile_t saved_sf;
	memcpy(&saved_sf,sf,sizeof(saved_sf));
	if ( NextCharSI(si,false) == '@' )
	    sf->ptr++;

	char name[VARNAME_SIZE+1];
	if (ScanNameSI(si,name,sizeof(name),true,true,0))
	{
	    const TextCommand_t tcmd = GetTextCommand(name);
	    //if ( tcmd && ( sf->no_if <= 0 || tcmd < TCMD_IF || tcmd > TCMD_ENDIF ))
	    if (tcmd)
	    {
		noPRINT("CMD FOUND @ line %u: %u %s |%.10s|\n",
			sf->line, tcmd, name, sf->ptr );
		return tcmd;
	    }
	}

	switch(NextCharSI(si,false))
	{
	    case ':':
		sf->ptr++;
		return TCMD_CALL;

	    case '>':
		sf->ptr++;
		return TCMD_EXEC;
	}

	memcpy(sf,&saved_sf,sizeof(*sf));
    }
    return TCMD_NONE;
}

///////////////////////////////////////////////////////////////////////////////

enumError ExecTextCommand
(
    ScanInfo_t		* si,		// valid data
    TextCommand_t	tcmd		// command to execute
)
{
    DASSERT(si);
    DASSERT(si->cur_file);
    TRACE("ExecTextCommand(%u)\n",tcmd);

    switch (tcmd)
    {
	case TCMD_ECHO:
	{
	    char * str = 0;
	    enumError err = ConcatStringsSI(si,&str,0);
	    if ( si->fast_scan || si->no_warn )
		err = ERR_OK;
	    else if ( str && !opt_no_echo )
		fprintf(stdlog,"%s\n",str);
	    FreeString(str);
	    return err;
	}

	case TCMD_WARN:
	{
	    char * str = 0;
	    enumError err = ConcatStringsSI(si,&str,0);
	    if ( si->fast_scan || si->no_warn )
		err = ERR_OK;
	    else if ( !err && str )
		err = ERROR0(ERR_WARNING,"%s",str);
	    FreeString(str);
	    return err;
	}

	case TCMD_RESET_PRIVATE:
	    ResetVarMap(&si->cur_file->pvar);
	    return ERR_OK;

	case TCMD_RESET_LOCAL:
	    ResetVarMap(&si->lvar);
	    return ERR_OK;

	case TCMD_RESET_GLOBAL:
	    ResetVarMap(&si->gvar);
	    return ERR_OK;

	case TCMD_DUMP_VAR:
	case TCMD_DUMP_PRIVATE:
	case TCMD_DUMP_LOCAL:
	case TCMD_DUMP_GLOBAL:
	case TCMD_DUMP_CONST:
	case TCMD_DUMP_PREDEF:
	{
	    if (NextCharSI(si,false))
	    {
		DEFINE_VAR(var);
		if ( ScanExprSI(si,&var) || !GetBoolV(&var) )
		    return ERR_OK;
	    }
	    else if (si->no_warn)
		return ERR_OK;

	    if (si->fast_scan)
		return ERR_OK;

	    if ( tcmd == TCMD_DUMP_VAR || tcmd == TCMD_DUMP_PRIVATE )
	    {
		fprintf(stdlog,"VAR DUMP/PRIVATE:\n");
		DumpVarMap(stdlog,3,&si->cur_file->pvar,false);
	    }
	    if ( tcmd == TCMD_DUMP_VAR || tcmd == TCMD_DUMP_LOCAL )
	    {
		fprintf(stdlog,"VAR DUMP/LOCAL:\n");
		DumpVarMap(stdlog,3,&si->lvar,false);
	    }
	    if ( tcmd == TCMD_DUMP_VAR || tcmd == TCMD_DUMP_GLOBAL )
	    {
		fprintf(stdlog,"VAR DUMP/GLOBAL:\n");
		DumpVarMap(stdlog,3,&si->gvar,false);
	    }
	    if ( tcmd == TCMD_DUMP_VAR || tcmd == TCMD_DUMP_CONST )
	    {
		fprintf(stdlog,"VAR DUMP/CONST:\n");
		DumpVarMap(stdlog,3,&const_map,false);
	    }
	    if ( tcmd == TCMD_DUMP_PREDEF )
	    {
		fprintf(stdlog,"VAR DUMP/PREDEF:\n");
		if (si->predef)
		    DumpVarMap(stdlog,3,si->predef,false);
	    }
	    return ERR_OK;
	}


	case TCMD_REVISION:
	    {
		ScanFile_t *sf = si->cur_file;
		DASSERT(sf);
		enumError err = ScanAssignIntSI(si,&sf->revision);
		DefineIntVar(&si->gvar,"REVISION$ACTIVE",sf->revision);
		return err;
	    }

	case TCMD_SZS_MODIFIER:
	    return ScanAssignIntSI(si,&si->cur_file->szs_modifier);

	case TCMD_SETUP_RANDOM:
	    {
		int num;
		enumError err = ScanAssignIntSI(si,&num);
		MySeed(num);
		return err;
	    }

	case TCMD_PDEF:
	case TCMD_PDEF_I:
	case TCMD_PDEF_F:
	case TCMD_PDEF_S:
	case TCMD_PDEF_V:
	case TCMD_PDEF_X:
	case TCMD_PDEF_Y:
	case TCMD_PDEF_Z:
	case TCMD_PDEF_ENUM:
	case TCMD_PDEF_SHIFT:	return SetVarSI(si,&si->cur_file->pvar,tcmd);

	case TCMD_LDEF:
	case TCMD_LDEF_I:
	case TCMD_LDEF_F:
	case TCMD_LDEF_S:
	case TCMD_LDEF_V:
	case TCMD_LDEF_X:
	case TCMD_LDEF_Y:
	case TCMD_LDEF_Z:
	case TCMD_LDEF_ENUM:
	case TCMD_LDEF_SHIFT:	return SetVarSI(si,&si->lvar,tcmd);

	case TCMD_GDEF:
	case TCMD_GDEF_I:
	case TCMD_GDEF_F:
	case TCMD_GDEF_S:
	case TCMD_GDEF_V:
	case TCMD_GDEF_X:
	case TCMD_GDEF_Y:
	case TCMD_GDEF_Z:
	case TCMD_GDEF_ENUM:
	case TCMD_GDEF_SHIFT:	return SetVarSI(si,&si->gvar,tcmd);

	case TCMD_IF:		return SI_IF(si);
	case TCMD_ELIF:
	case TCMD_ELSE:		return SI_ELSE(si);
	case TCMD_ENDIF:	return SI_ENDIF(si);

	case TCMD_DOIF:		return SI_DOIF(si);

	case TCMD_LOOP:		return SI_LOOP(si);
	case TCMD_REPEAT:	return SI_REPEAT(si);
	case TCMD_FOR:		return SI_FOR(si);
	case TCMD_FOREACH:	return SI_FOREACH(si);
	case TCMD_WHILE:	return SI_WHILE(si);

	case TCMD_ENDLOOP:	return SI_ENDLOOP(si);
	case TCMD_ENDREPEAT:	return SI_ENDREPEAT(si);
	case TCMD_ENDFOR:	return SI_ENDFOR(si,0);
	case TCMD_ENDFOREACH:	return SI_ENDFOREACH(si,0);
	case TCMD_ENDWHILE:	return SI_ENDWHILE(si,0);

	case TCMD_BREAK:	return SI_BreakLoop(si,false);
	case TCMD_CONTINUE:	return SI_BreakLoop(si,true);

	case TCMD_EXEC:		return SI_EXEC(si);
	case TCMD_INCLUDE:	return SI_INCLUDE(si);
	case TCMD_RETURN:	return SI_RETURN(si);
	case TCMD_EXIT:		return SI_EXIT(si,false);
	case TCMD_ASSERT:	return SI_EXIT(si,true);

	case TCMD_MACRO:
	case TCMD_FUNCTION:	return SI_MACRO(si,tcmd);
	case TCMD_ENDMACRO:
	case TCMD_ENDFUNCTION:	return SI_ENDMACRO(si,tcmd);

	case TCMD_CALL:		return SI_CALL(si);
	case TCMD_PARAMETERS:	return SI_PARAMETERS(si);

	default:
	    DASSERT(0);
	    return ERR_WARNING;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		expressions: definitions		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[OpCounter_t]] created by work/scripts/create-db/optab.gen

typedef enum OpCounter_t
{
	OP_MAX_LEN	=   3,
	OP_CHARMAP_SIZE	= 128,
	OP_CHARIDX_SIZE	=  15,
	OP_TAB_SIZE	=  29

} OpCounter_t;

//-----------------------------------------------------------------------------
// [[OpPrio_t]] created by work/scripts/create-db/optab.gen

typedef enum OpPrio_t
{
	OPP_NONE,	//  3*
	OPP_JOIN,	//  1*
	OPP_LOR,	//  1*
	OPP_LEOR,	//  2*
	OPP_LAND,	//  1*
	OPP_OR,		//  1*
	OPP_EOR,	//  2*
	OPP_AND,	//  1*
	OPP_EQ,		//  5*
	OPP_CMP,	//  4*
	OPP_SHIFT,	//  2*
	OPP_ADD,	//  2*
	OPP_MULT,	//  3*
	OPP_POW,	//  1*

	OPP__N // = 14

} OpPrio_t;

//-----------------------------------------------------------------------------
// [[OpMode_t]] created by work/scripts/create-db/optab.gen

typedef enum OpMode_t
{
	OPM_0,		// 27*
	OPM_AND,	//  1*
	OPM_OR,		//  1*

} OpMode_t;

///////////////////////////////////////////////////////////////////////////////
// [[Operator_t]] // created by work/scripts/create-db/optab.gen

typedef struct Operator_t
{
	OpId_t		id;
	OpPrio_t	prio;
	OpMode_t	mode;
	int		op_len;
	const char	op_name[OP_MAX_LEN+1];
	const u8	sub_index[OP_CHARIDX_SIZE];

} Operator_t;

///////////////////////////////////////////////////////////////////////////////
// [[op_tab]] // created by work/scripts/create-db/optab.gen

static const Operator_t op_tab[OP_TAB_SIZE] = // OP_TAB_SIZE := 29
{
 { OPI_NONE,   OPP_NONE,  OPM_0,   0,"",    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14} },

 { OPI_NONE,   OPP_NONE,  OPM_0,   1,"!",   {0,0,0,0,0,0,0,0,0,15,0,0,0,0,0} },
 { OPI_MOD,    OPP_MULT,  OPM_0,   1,"%",   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_AND,    OPP_AND,   OPM_0,   1,"&",   {0,0,0,16,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_MULT,   OPP_MULT,  OPM_0,   1,"*",   {0,0,0,0,17,0,0,0,0,0,0,0,0,0,0} },
 { OPI_ADD,    OPP_ADD,   OPM_0,   1,"+",   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_SUB,    OPP_ADD,   OPM_0,   1,"-",   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_DIV,    OPP_MULT,  OPM_0,   1,"/",   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_LT,     OPP_CMP,   OPM_0,   1,"<",   {0,0,0,0,0,0,0,0,18,19,20,0,0,0,0} },
 { OPI_NONE,   OPP_NONE,  OPM_0,   1,"=",   {0,0,0,0,0,0,0,0,0,21,0,0,0,0,0} },
 { OPI_GT,     OPP_CMP,   OPM_0,   1,">",   {0,0,0,0,0,0,0,0,0,22,23,0,0,0,0} },
 { OPI_JOIN,   OPP_JOIN,  OPM_0,   1,"@",   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_EOR,    OPP_EOR,   OPM_0,   1,"^",   {0,0,0,0,0,0,0,0,0,0,0,0,24,0,0} },
 { OPI_OR,     OPP_OR,    OPM_0,   1,"|",   {0,0,0,0,0,0,0,0,0,0,0,0,0,25,0} },
 { OPI_EOR,    OPP_EOR,   OPM_0,   1,"~",   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,26} },

 { OPI_NEQ,    OPP_EQ,    OPM_0,   2,"!=",  {0,0,0,0,0,0,0,0,0,27,0,0,0,0,0} },
 { OPI_LAND,   OPP_LAND,  OPM_AND, 2,"&&",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_POW,    OPP_POW,   OPM_0,   2,"**",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_LSHIFT, OPP_SHIFT, OPM_0,   2,"<<",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_LE,     OPP_CMP,   OPM_0,   2,"<=",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_NEQ,    OPP_EQ,    OPM_0,   2,"<>",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_EQ,     OPP_EQ,    OPM_0,   2,"==",  {0,0,0,0,0,0,0,0,0,28,0,0,0,0,0} },
 { OPI_GE,     OPP_CMP,   OPM_0,   2,">=",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_RSHIFT, OPP_SHIFT, OPM_0,   2,">>",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_LEOR,   OPP_LEOR,  OPM_0,   2,"^^",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_LOR,    OPP_LOR,   OPM_OR,  2,"||",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_LEOR,   OPP_LEOR,  OPM_0,   2,"~~",  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },

 { OPI_NEQNEQ, OPP_EQ,    OPM_0,   3,"!==", {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
 { OPI_EQEQ,   OPP_EQ,    OPM_0,   3,"===", {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} },
};

///////////////////////////////////////////////////////////////////////////////
// created by work/scripts/create-db/optab.gen

static const u8 op_charmap[OP_CHARMAP_SIZE] = // OP_CHARMAP_SIZE := 128
{
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  1,  0,  0,  0,  2,  3,  0,  0,  0,  4,  5,  0,  6,  0,  7,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9, 10,  0,
	11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 13,  0, 14,  0,
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////		expressions: helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static const Operator_t * FindOperator
(
    ScanInfo_t		* si		// valid data
)
{
    DASSERT(si);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    const Operator_t *op = op_tab, *found = op_tab;

    NextCharSI(si,false);
    ccp ptr = sf->prev_ptr = sf->ptr;
    while ( ptr < sf->end && (uchar)*ptr < OP_CHARMAP_SIZE )
    {
	const uint ch_idx = op_charmap[(uchar)*ptr];
	DASSERT( ch_idx < OP_CHARIDX_SIZE );
	const uint op_idx = op->sub_index[ch_idx];
	if (!op_idx)
	    break;
	ptr++;
	DASSERT( op_idx < OP_TAB_SIZE );
	op = op_tab + op_idx;
	if (op->id)
	{
	    found = op;
	    sf->ptr = ptr;
	}
	// else: a temporary step
    }
    noPRINT("FindOperator(%.*s) %zu\n",
	(int)(sf->ptr - sf->prev_ptr), sf->prev_ptr, found - op_tab );
    return found;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ShiftLeftV ( Var_t *val, int shift )
{
 #undef MAXBIT
 #define MAXBIT ((int)(CHAR_BIT*sizeof(val->i)))

    DASSERT(val);
    switch(val->mode)
    {
	case VAR_UNSET:
	    break;

	case VAR_INT:
	    if	     ( shift  >= MAXBIT )	val->i   =  0;
	     else if ( shift  >= 0 )		val->i <<= shift;
	     else if ( shift  > -MAXBIT )	val->i >>= -shift;
	     else if ( val->i < 0 )		val->i   = -1;
	     else				val->i   =  0;
	    break;

	case VAR_DOUBLE:
	    val->d = ldexp(val->d,shift);
	    break;

	case VAR_VECTOR:
	    val->x = ldexp(val->x,shift);
	    val->y = ldexp(val->y,shift);
	    val->z = ldexp(val->z,shift);
	    break;

	case VAR_STRING:
	    // ignored!
	    break;
    }

 #undef MAXBIT
}

///////////////////////////////////////////////////////////////////////////////

void CalcExpr
(
    Var_t		* dest,		// first source and destination
    const Var_t		* src2,		// second source
    OpId_t		op_id,		// operator id
    ScanInfo_t		* si		// not NULL: Use for warnings
)
{
    DASSERT(dest);
    DASSERT(src2);
    DASSERT( op_id > OPI_NONE && op_id < OPI__N );

    noPRINT("%%%%%% CalcExpr() %p,%d,%p  %s  %p,%d,%p\n",
		dest,dest->mode,dest->str, op_tab[op_id].op_name,
		src2,src2->mode,src2->str );

    dest->int_mode = IMD_UNSET;


    //--- if one unset: assign only!

    if ( src2->mode == VAR_UNSET )
	return;

    if ( dest->mode == VAR_UNSET )
    {
	AssignVar(dest,src2);
	return;
    }

    //--- OPI_JOIN is the only force-string operator

    DEFINE_VAR(temp);

    if ( op_id == OPI_JOIN )
    {
     op_join:;
	AssignStringVV(&temp,dest);
	AppendStringVV(&temp,src2);
	MoveDataV(dest,&temp);
	return;
    }


    //--- string operations

    if ( dest->mode == VAR_STRING || src2->mode == VAR_STRING )
    {
      if ( dest->mode == VAR_STRING && src2->mode == VAR_STRING )
      {
       str_cmp:;
	int res = 0;
	switch (op_id)
	{
	  case OPI_LT: // SxS
	    res = CmpMem( MemByS(dest->str,dest->str_len),
			  MemByS(src2->str,src2->str_len)) < 0;
	    break;

	  case OPI_LE: // SxS
	    res = CmpMem( MemByS(dest->str,dest->str_len),
			  MemByS(src2->str,src2->str_len)) <= 0;
	    break;

	  case OPI_GT: // SxS
	    res = CmpMem( MemByS(dest->str,dest->str_len),
			  MemByS(src2->str,src2->str_len)) > 0;
	    break;

	  case OPI_GE: // SxS
	    res = CmpMem( MemByS(dest->str,dest->str_len),
			  MemByS(src2->str,src2->str_len)) >= 0;
	    break;

	  case OPI_EQ: // SxS
	  case OPI_EQEQ: // SxS
	    res = dest->str_len == src2->str_len
			&& !memcmp(dest->str,src2->str,dest->str_len);
	    break;

	  case OPI_NEQ: // SxS
	  case OPI_NEQNEQ: // SxS
	    res = dest->str_len != src2->str_len
			|| memcmp(dest->str,src2->str,dest->str_len);
	    break;

	  case OPI_ADD: // SxS
	    goto op_join;

	  case OPI_LAND: // SxS
	    res = dest->str_len > 0 && src2->str_len > 0;
	    break;

	  case OPI_LOR: // SxS
	    res = dest->str_len > 0 || src2->str_len > 0;
	    break;

	  case OPI_LEOR: // SxS
	    res = dest->str_len > 0 != src2->str_len > 0;
	    break;

	  case OPI_POW: // SxS
	  case OPI_MULT: // SxS
	  case OPI_DIV: // SxS
	  case OPI_MOD: // SxS
	  case OPI_SUB: // SxS
	  case OPI_LSHIFT: // SxS
	  case OPI_RSHIFT: // SxS
	  case OPI_AND: // SxS
	  case OPI_OR: // SxS
	  case OPI_EOR: // SxS
	    ResetV(dest);
	    return;

	  case OPI_JOIN: // SxS
	  case OPI_NONE: // SxS
	  case OPI__N: // SxS
	    DASSERT(0); // never executed
	    ResetV(dest);
	    return;
	}

	FreeV(dest);
	dest->mode = VAR_INT;
	dest->i = res;
	return;
      }
      else // one param is not string
      {
	DASSERT( ( dest->mode == VAR_STRING ) != ( src2->mode == VAR_STRING ) );
	switch (op_id)
	{
	  case OPI_LAND: // S
	  case OPI_LOR:  // S
	  case OPI_LEOR: // S
	    if ( dest->mode == VAR_STRING )
		ToBoolV(dest);
	    else if ( src2->mode == VAR_STRING )
	    {
		temp.mode = VAR_INT;
		temp.i = src2->str_len > 0;
		src2 = &temp;
	    }
	    // continue like numeric values
	    break;

	  case OPI_EQEQ: // S
	  case OPI_NEQNEQ: // S
	    FreeV(dest);
	    dest->mode = VAR_INT;
	    dest->i = op_id == OPI_NEQNEQ;
	    return;

	  case OPI_EQ:  // S
	  case OPI_NEQ: // S
	  case OPI_LT:  // S
	  case OPI_LE:  // S
	  case OPI_GT:  // S
	  case OPI_GE:  // S
	    if ( dest->mode == VAR_STRING )
	    {
		AssignStringVV(&temp,src2);
		src2 = &temp;
	    }
	    else
		ToStringV(dest);
	    goto str_cmp;

	  default:
	    ResetV(dest);
	    return;
	}
      }
    }

    DASSERT( dest->mode != VAR_STRING );
    DASSERT( src2->mode != VAR_STRING );


    if ( dest->mode == VAR_VECTOR )
    {
      if ( src2->mode == VAR_VECTOR )
      {
	// both vectors
	switch (op_id)
	{
	  case OPI_POW: // VxV
	    dest->x = pow(dest->x,src2->x);
	    dest->y = pow(dest->y,src2->y);
	    dest->z = pow(dest->z,src2->z);
	    return;

	  case OPI_MULT: // VxV
	    dest->x *= src2->x;
	    dest->y *= src2->y;
	    dest->z *= src2->z;
	    return;

	  case OPI_DIV: // VxV
	    dest->x /= src2->x;
	    dest->y /= src2->y;
	    dest->z /= src2->z;
	    return;

	  case OPI_MOD: // VxV
	    dest->x = fmod(dest->x,src2->x);
	    dest->y = fmod(dest->y,src2->y);
	    dest->z = fmod(dest->z,src2->z);
	    return;

	  case OPI_ADD: // VxV
	    dest->x += src2->x;
	    dest->y += src2->y;
	    dest->z += src2->z;
	    return;

	  case OPI_SUB: // VxV
	    dest->x -= src2->x;
	    dest->y -= src2->y;
	    dest->z -= src2->z;
	    return;

	  case OPI_LSHIFT: // VxV
	    dest->x = ldexp(dest->x,(int)src2->x);
	    dest->y = ldexp(dest->y,(int)src2->y);
	    dest->z = ldexp(dest->z,(int)src2->z);
	    return;

	  case OPI_RSHIFT: // VxV
	    dest->x = ldexp(dest->x,(int)-src2->x);
	    dest->y = ldexp(dest->y,(int)-src2->y);
	    dest->z = ldexp(dest->z,(int)-src2->z);
	    return;

	  case OPI_LT: // VxV
	    if ( dest->x != src2->x )
		dest->i = dest->x < src2->x;
	    else if ( dest->y != src2->y )
		dest->i = dest->y < src2->y;
	    else if ( dest->z != src2->z )
		dest->i = dest->z < src2->z;
	    else
		dest->i = 0;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LE: // VxV
	    if ( dest->x != src2->x )
		dest->i = dest->x <= src2->x;
	    else if ( dest->y != src2->y )
		dest->i = dest->y <= src2->y;
	    else if ( dest->z != src2->z )
		dest->i = dest->z <= src2->z;
	    else
		dest->i = 1;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_GT: // VxV
	    if ( dest->x != src2->x )
		dest->i = dest->x > src2->x;
	    else if ( dest->y != src2->y )
		dest->i = dest->y > src2->y;
	    else if ( dest->z != src2->z )
		dest->i = dest->z > src2->z;
	    else
		dest->i = 0;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_GE: // VxV
	    if ( dest->x != src2->x )
		dest->i = dest->x >= src2->x;
	    else if ( dest->y != src2->y )
		dest->i = dest->y >= src2->y;
	    else if ( dest->z != src2->z )
		dest->i = dest->z >= src2->z;
	    else
		dest->i = 1;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_EQ: // VxV
	  case OPI_EQEQ: // VxV
	    dest->i = dest->x == src2->x && dest->y == src2->y && dest->z == src2->z;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_NEQ: // VxV
	  case OPI_NEQNEQ: // VxV
	    dest->i = dest->x != src2->x || dest->y != src2->y || dest->z == src2->z;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_AND: // VxV
	  case OPI_OR: // VxV
	  case OPI_EOR: // VxV
	    dest->mode = VAR_UNSET;
	    return;

	  case OPI_LAND: // VxV
	    dest->i = ( dest->x || dest->y || dest->z )
		  && ( src2->x || src2->y || src2->z );
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LOR: // VxV
	    dest->i = ( dest->x || dest->y || dest->z )
		  || ( src2->x || src2->y || src2->z );
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LEOR: // VxV
	    dest->i = ( dest->x || dest->y || dest->z )
		  != ( src2->x || src2->y || src2->z );
	    dest->mode = VAR_INT;
	    return;

	  case OPI_JOIN: // VxV
	  case OPI_NONE: // VxV
	  case OPI__N: // VxV
	    DASSERT(0);
	    return;
	}
      }
      else
      {
	// only dest is vector
	double d = GetDoubleV(src2);
	switch (op_id)
	{
	  case OPI_POW: // VxD
	    dest->x = pow(dest->x,d);
	    dest->y = pow(dest->y,d);
	    dest->z = pow(dest->z,d);
	    return;

	  case OPI_MULT: // VxD
	    dest->x *= d;
	    dest->y *= d;
	    dest->z *= d;
	    return;

	  case OPI_DIV: // VxD
	    dest->x /= d;
	    dest->y /= d;
	    dest->z /= d;
	    return;

	  case OPI_MOD: // VxD
	    dest->x = fmod(dest->x,d);
	    dest->y = fmod(dest->y,d);
	    dest->z = fmod(dest->z,d);
	    return;

	  case OPI_ADD: // VxD
	    dest->x += d;
	    dest->y += d;
	    dest->z += d;
	    return;

	  case OPI_SUB: // VxD
	    dest->x -= d;
	    dest->y -= d;
	    dest->z -= d;
	    return;

	  case OPI_LSHIFT: // VxD
	    {
		int shift = (int)floor(d+0.5);
		dest->x = ldexp(dest->x,shift);
		dest->y = ldexp(dest->y,shift);
		dest->z = ldexp(dest->z,shift);
	    }
	    return;

	  case OPI_RSHIFT: // VxD
	    {
		int shift = -(int)floor(d+0.5);
		dest->x = ldexp(dest->x,shift);
		dest->y = ldexp(dest->y,shift);
		dest->z = ldexp(dest->z,shift);
	    }
	    return;

	  case OPI_LT: // VxD
	    dest->i = dest->x < d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LE: // VxD
	    dest->i = dest->x <= d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_GT: // VxD
	    dest->i = dest->x > d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_GE: // VxD
	    dest->i = dest->x >= d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_EQ: // VxD
	    dest->i = dest->x == d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_EQEQ: // VxD
	    dest->i = 0;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_NEQ: // VxD
	    dest->i = dest->x != d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_NEQNEQ: // VxD
	    dest->i = 1;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_AND: // VxD
	  case OPI_OR: // VxD
	  case OPI_EOR: // VxD
	    dest->mode = VAR_UNSET;
	    return;

	  case OPI_LAND: // VxD
	    dest->i = ( dest->x || dest->y || dest->z ) && d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LOR: // VxD
	    dest->i = ( dest->x || dest->y || dest->z ) || d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LEOR: // VxD
	    dest->i = ( dest->x || dest->y || dest->z ) == !d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_JOIN: // VxD
	  case OPI_NONE: // VxD
	  case OPI__N: // VxD
	    DASSERT(0);
	    return;
	}
      }
    }
    else if ( src2->mode == VAR_VECTOR )
    {
      // only src2 is vector
      double d = GetDoubleV(dest);
      MARK_USED(d);
      switch (op_id)
      {
	  case OPI_POW: // DxV
	    dest->x = pow(d,src2->x);
	    dest->y = pow(d,src2->y);
	    dest->z = pow(d,src2->z);
	    dest->mode = VAR_VECTOR;
	    return;

	  case OPI_MULT: // DxV
	    dest->x = d * src2->x;
	    dest->y = d * src2->y;
	    dest->z = d * src2->z;
	    dest->mode = VAR_VECTOR;
	    return;

	  case OPI_DIV: // DxV
	    dest->x = d / src2->x;
	    dest->y = d / src2->y;
	    dest->z = d / src2->z;
	    dest->mode = VAR_VECTOR;
	    return;

	  case OPI_MOD: // DxV
	    dest->x = fmod(d,src2->x);
	    dest->y = fmod(d,src2->y);
	    dest->z = fmod(d,src2->z);
	    dest->mode = VAR_VECTOR;
	    return;

	  case OPI_ADD: // DxV
	    dest->x = d + src2->x;
	    dest->y = d + src2->y;
	    dest->z = d + src2->z;
	    dest->mode = VAR_VECTOR;
	    return;

	  case OPI_SUB: // DxV
	    dest->x = d - src2->x;
	    dest->y = d - src2->y;
	    dest->z = d - src2->z;
	    dest->mode = VAR_VECTOR;
	    return;

	  case OPI_LSHIFT: // DxV
	    dest->x = ldexp(d,(int)src2->x);
	    dest->y = ldexp(d,(int)src2->y);
	    dest->z = ldexp(d,(int)src2->z);
	    return;

	  case OPI_RSHIFT: // DxV
	    dest->x = ldexp(d,(int)-src2->x);
	    dest->y = ldexp(d,(int)-src2->y);
	    dest->z = ldexp(d,(int)-src2->z);
	    return;

	  case OPI_LT: // DxV
	    dest->i = dest->x < d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LE: // DxV
	    dest->i = dest->x <= d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_GT: // DxV
	    dest->i = dest->x > d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_GE: // DxV
	    dest->i = dest->x >= d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_EQ: // DxV
	    dest->i = dest->x == d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_EQEQ: // DxV
	    dest->i = 0;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_NEQ: // DxV
	    dest->i = dest->x != d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_NEQNEQ: // DxV
	    dest->i = 1;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_AND: // DxV
	  case OPI_OR: // DxV
	  case OPI_EOR: // DxV
	    dest->mode = VAR_UNSET;
	    return;

	  case OPI_LAND: // DxV
	    dest->i = ( dest->x || dest->y || dest->z ) && d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LOR: // DxV
	    dest->i = ( dest->x || dest->y || dest->z ) || d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_LEOR: // DxV
	    dest->i = ( dest->x || dest->y || dest->z ) == !d;
	    dest->mode = VAR_INT;
	    return;

	  case OPI_JOIN: // DxV
	  case OPI_NONE: // DxV
	  case OPI__N: // DxV
	    DASSERT(0);
	    return;
	}
    }
    else if ( dest->mode == VAR_DOUBLE || src2->mode == VAR_DOUBLE )
    {
      switch (op_id)
      {
	case OPI_POW: // DxD | DxI | IxD
	    dest->d = pow( dest->mode == VAR_DOUBLE ? dest->d : dest->i,
			   src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    break;

	case OPI_MULT: // DxD | DxI | IxD
	    dest->d = ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    * ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    break;

	case OPI_DIV: // DxD | DxI | IxD
	    dest->d = ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    / ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    break;

	case OPI_MOD: // DxD | DxI | IxD
	    dest->d = fmod( dest->mode == VAR_DOUBLE ? dest->d : dest->i,
			    src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    break;

	case OPI_ADD: // DxD | DxI | IxD
	    dest->d = ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    + ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    break;

	case OPI_SUB: // DxD | DxI | IxD
	    dest->d = ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    - ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    break;

	case OPI_LSHIFT: // DxD | DxI | IxD
	    ShiftLeftV(dest, GetIntV(src2));
	    break;

	case OPI_RSHIFT: // DxD | DxI | IxD
	    ShiftLeftV(dest,-GetIntV(src2));
	    break;

	case OPI_LT: // DxD | DxI | IxD
	    dest->i = ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    < ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    dest->mode = VAR_INT;
	    return;

	case OPI_LE: // DxD | DxI | IxD
	    dest->i =  ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    <= ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    dest->mode = VAR_INT;
	    return;

	case OPI_GT: // DxD | DxI | IxD
	    dest->i = ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    > ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    dest->mode = VAR_INT;
	    return;

	case OPI_GE: // DxD | DxI | IxD
	    dest->i =  ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    >= ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    dest->mode = VAR_INT;
	    return;

	case OPI_EQ: // DxD | DxI | IxD
	    dest->i =  ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    == ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    dest->mode = VAR_INT;
	    return;

	case OPI_NEQ: // DxD | DxI | IxD
	    dest->i =  ( dest->mode == VAR_DOUBLE ? dest->d : dest->i )
		    != ( src2->mode == VAR_DOUBLE ? src2->d : src2->i );
	    dest->mode = VAR_INT;
	    return;

	case OPI_EQEQ: // DxD | DxI | IxD
	    dest->i    = dest->mode == src2->mode && dest->d == src2->d;
	    dest->mode = VAR_INT;
	    return;

	case OPI_NEQNEQ: // DxD | DxI | IxD
	    dest->i    = dest->mode != src2->mode || dest->d != src2->d;
	    dest->mode = VAR_INT;
	    return;

	case OPI_AND: // DxD | DxI | IxD
	    dest->i    = GetIntV(dest) & GetIntV(src2);
	    dest->mode = VAR_INT;
	    return;

	case OPI_OR: // DxD | DxI | IxD
	    dest->i    = GetIntV(dest) | GetIntV(src2);
	    dest->mode = VAR_INT;
	    return;

	case OPI_EOR: // DxD | DxI | IxD
	    dest->i    = GetIntV(dest) ^ GetIntV(src2);
	    dest->mode = VAR_INT;
	    return;

	case OPI_LAND: // DxD | DxI | IxD
	    dest->i    = GetBoolV(dest) && GetBoolV(src2);
	    dest->mode = VAR_INT;
	    return;

	case OPI_LOR: // DxD | DxI | IxD
	    dest->i    = GetBoolV(dest) || GetBoolV(src2);
	    dest->mode = VAR_INT;
	    return;

	case OPI_LEOR: // DxD | DxI | IxD
	    dest->i    = GetBoolV(dest) != GetBoolV(src2);
	    dest->mode = VAR_INT;
	    return;

	case OPI_JOIN: // DxD | DxI | IxD
	case OPI_NONE: // DxD | DxI | IxD
	case OPI__N: // DxD | DxI | IxD
	    DASSERT(0);
	    break;
      }
      dest->mode = VAR_DOUBLE;
    }
    else
    {
      DASSERT( dest->mode == VAR_INT );
      DASSERT( src2->mode == VAR_INT );

      switch (op_id)
      {
	case OPI_POW: // IxI
	    if ( src2->i >= 0 )
	    {
		// try integer calculation first, double is fall back on overflow
		s64 res = 1;
		int n = src2->i, err = 0;
		while ( n-- > 0 )
		{
		    s64 new_res = res * dest->i;
		    if ( llabs(new_res) <= llabs(res) )
		    {
			err++;
			break;
		    }
		    res = new_res;
		}
		if (!err)
		{
		    dest->i = res;
		    break;
		}
	    }
	    dest->d = pow(dest->i,src2->i);
	    dest->mode = VAR_DOUBLE;
	    break;

	case OPI_DIV: // IxI
	    if (src2->i)
	    {
		if ( si->float_div > 0 )
		{
		    dest->d = (double)dest->i / (double)src2->i;
		    dest->mode = VAR_DOUBLE;
		}
		else
		    dest->i /= src2->i;
	    }
	    else if ( si && si->no_warn <= 0 )
	    {
		ScanFile_t *sf = si->cur_file;
		DASSERT(sf);
		sf->line_err++;
		si->total_err++;
		ERROR0(ERR_WARNING,"Division by 0 [%s @%u]\n",sf->name,sf->line);
	    }
	    break;

	case OPI_MOD: // IxI
	    if (src2->i)
		dest->i %= src2->i;
	    else if ( si && si->no_warn <= 0 )
	    {
		ScanFile_t *sf = si->cur_file;
		DASSERT(sf);
		sf->line_err++;
		si->total_err++;
		ERROR0(ERR_WARNING,"Modulo by 0 [%s @%u]\n",sf->name,sf->line);
	    }
	    break;

	// IxI
	case OPI_MULT:	 dest->i *=  src2->i; break;
	case OPI_ADD:	 dest->i +=  src2->i; break;
	case OPI_SUB:	 dest->i -=  src2->i; break;

	case OPI_LSHIFT: ShiftLeftV(dest, src2->i); break;
	case OPI_RSHIFT: ShiftLeftV(dest,-src2->i); break;

	case OPI_LT:	 dest->i = dest->i <  src2->i; break;
	case OPI_LE:	 dest->i = dest->i <= src2->i; break;
	case OPI_GT:	 dest->i = dest->i >  src2->i; break;
	case OPI_GE:	 dest->i = dest->i >= src2->i; break;
	case OPI_EQ:
	case OPI_EQEQ:	 dest->i = dest->i == src2->i; break;
	case OPI_NEQ:
	case OPI_NEQNEQ: dest->i = dest->i != src2->i; break;

	case OPI_AND:	 dest->i &= src2->i; break;
	case OPI_OR:	 dest->i |= src2->i; break;
	case OPI_EOR:	 dest->i ^= src2->i; break;
	    break;

	case OPI_LAND:	 dest->i = dest->i && src2->i; break;
	case OPI_LOR:	 dest->i = dest->i || src2->i; break;
	case OPI_LEOR:	 dest->i = !dest->i != !src2->i; break;

	case OPI_JOIN: // IxI
	case OPI_NONE: // IxI
	case OPI__N: // IxI
	    DASSERT(0);
	    break;
      }
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		   expressions: functions		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanExprSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// result: store value here
)
{
    DASSERT(si);
    DASSERT(var);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    sf->expr_depth++;
    noPRINT("ENTER ScanExprSI(), depth=%d, nowarn=%d, nocalc=%d\n",
		sf->expr_depth++, si->no_warn, si->no_calc );

    struct PARMS
    {
	const Operator_t	* op;
	Var_t			val;
	int			no_calc;

    } parm[OPP__N];
    const Operator_t *cur_op = parm->op = op_tab;
    struct PARMS *p = parm;
    p->val.mode = VAR_UNSET;

    //enum { M_VALUE, M_ALT, M_ALT1, M_ALT2 } mode = M_VALUE;

    enumError maxerr = ERR_OK;
    do
    {
	p++;
	DASSERT( p < parm + OPP__N );
	p->op = cur_op;
	p->no_calc = si->no_calc;
	p->val.mode = VAR_UNSET;
	p->val.int_mode = IMD_UNSET;
	enumError err = ScanValueSI(si,&p->val);
	if ( maxerr < err )
	     maxerr = err;
	cur_op = FindOperator(si);

	while ( p > parm+1 && cur_op->prio <= p->op->prio )
	{
	    // calc operations with higher and same priority

	    PRINT("%s: %g %s %g [%d,%d,%d] nocalc=%d,%d mode=%dx%d imode=%dx%d\n",
			p->no_calc <= 0 ? "CALC" : "SKIP",
			GetDoubleV(&p[-1].val),
			p->op->op_name,
			GetDoubleV(&p->val),
			p->op->id, p->op->prio, p->op->mode,
			p[-1].no_calc, p->no_calc,
			p[-1].val.mode, p->val.mode,
			p[-1].val.int_mode, p->val.int_mode );
	    if ( p->no_calc <= 0 )
		CalcExpr(&p[-1].val,&p->val,p->op->id,si);
	    FreeV(&p->val);
	    p--;
	    si->no_calc = p->no_calc;
	}

     #if HAVE_PRINT
	if ( cur_op->id == OPI_LAND || cur_op->id == OPI_LOR )
	    PRINT(">>OP=%u '%g %s ...'\n",
			cur_op->id, GetDoubleV(&p->val), cur_op->op_name );
     #endif

	if ( cur_op->id == OPI_LAND && !GetBoolV(&p->val) )
	{
	    PRINT("NO-CALC++ [%d->%d] (AND)\n",si->no_calc,si->no_calc+1);
	    FreeV(&p->val);
	    p->val.mode = VAR_INT;
	    p->val.i    = 0;
	    si->no_calc++;
	}
	else if ( cur_op->id == OPI_LOR && GetBoolV(&p->val) )
	{
	    PRINT("NO-CALC++ [%d->%d] (OR)\n",si->no_calc,si->no_calc+1);
	    FreeV(&p->val);
	    p->val.mode = VAR_INT;
	    p->val.i    = 1;
	    si->no_calc++;
	}

    } while ( cur_op->prio > OPP_NONE );

    if ( NextCharSI(si,false) == '?' )
    {
	sf->ptr++;
	noPRINT("NO-WARN = %d, \n",si->no_warn);
	if (GetBoolV(&p->val))
	{
	    enumError err = ScanExprSI(si,var);
	    if ( maxerr < err )
		 maxerr = err;

	    err = CheckWarnSI(si,':',ERR_OK);
	    if (!err)
	    {
		noPRINT("NO-CALC++ [%d->%d] (1?:)\n",si->no_calc,si->no_calc+1);
		si->no_calc++;
		ScanExprSI(si,&p->val);
		noPRINT("NO-CALC-- [%d->%d] (1?:)\n",si->no_calc,si->no_calc-1);
		si->no_calc--;
	    }
	    else if ( maxerr < err )
		maxerr = err;
	}
	else
	{
	    noPRINT("NO-CALC++ [%d->%d] (0?:)\n",si->no_calc,si->no_calc+1);
	    si->no_calc++;
	    ScanExprSI(si,var);
	    noPRINT("NO-CALC-- [%d->%d] (0?:)\n",si->no_calc,si->no_calc-1);
	    si->no_calc--;

	    enumError err = CheckWarnSI(si,':',ERR_OK);
	    if (!err)
		err = ScanExprSI(si,var);
	    if ( maxerr < err )
		 maxerr = err;
	}
    }
    else
	AssignVar(var,&p->val);

    noPRINT("LEAVE ScanExprSI(), depth=%d, nowarn=%d, nocalc=%d\n",
		sf->expr_depth++, si->no_warn, si->no_calc );
    sf->expr_depth--;
    return maxerr;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanVectorExprSI
(
    // scans: v | a,b | x,y,z

    ScanInfo_t		* si,		// valid data
    Var_t		* var,		// result: store value here
    double		ab_default,	// used for 'a,b' format
    uint		xyz		// for ab default: 0:yz, 1:xz, 2:xy
)
{
    DASSERT(si);
    DASSERT(var);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    sf->disable_comma++;
    enumError err = ScanExprSI(si,var);
    ToVectorV(var);

    if ( NextCharSI(si,false) == ',' )
	sf->ptr++;
    if (*sf->ptr)
    {
	DEFINE_VAR(temp);
	err = ScanExprSI(si,&temp);
	if (!err)
	{
	    if ( NextCharSI(si,false) == ',' )
		sf->ptr++;
	    if (*sf->ptr)
	    {
		var->y = GetYDoubleV(&temp);
		err = ScanExprSI(si,&temp);
		var->z = GetZDoubleV(&temp);
	    }
	    else if ( xyz == 0 )
	    {
		var->y = var->x;
		var->x = ab_default;
		var->z = GetZDoubleV(&temp);
	    }
	    else if ( xyz == 2 )
	    {
		var->y = GetYDoubleV(&temp);
		var->z = ab_default;
	    }
	    else // xyz == 1
	    {
		var->y = ab_default;
		var->z = GetZDoubleV(&temp);
	    }
	}
    }

    sf->disable_comma--;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanVectorExpr
(
    // scans: v | x,z | x,y,z

    ccp			arg,		// string to scan
    ccp			name,		// NULL or name for error messages
    Var_t		* var		// result: store value here
)
{
    DASSERT(var);

    if (!arg)
	arg = "";

    ScanInfo_t si;
    InitializeSI(&si,arg,strlen(arg),name,0);
    if (!name)
	si.no_warn++;
    const enumError err = ScanVectorExprSI(&si,var,0.0,1);
    CheckEolSI(&si);
    ResetSI(&si);
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanMinMaxExprSI
(
    // scans: val (min=-val,max=+val) | val1,val2

    ScanInfo_t		*si,		// valid data
    Var_t		*min,		// result: store min vector here
    Var_t		*max		// result: store max vector here
)
{
    DASSERT(si);
    DASSERT(min);
    DASSERT(max);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    sf->disable_comma++;
    enumError err = ScanExprSI(si,min);
    ToVectorV(min);

    if ( err || NextCharSI(si,false) != ',' )
    {
	max->x = fabs(min->x); min->x = -max->x;
	max->y = fabs(min->y); min->y = -max->y;
	max->z = fabs(min->z); min->z = -max->z;
	max->mode = VAR_VECTOR;
    }
    else
    {
	sf->ptr++;
	err = ScanExprSI(si,max);
	ToVectorV(max);

	if ( min->x > max->x ) { double d = min->x; min->x = max->x; max->x = d; }
	if ( min->y > max->y ) { double d = min->y; min->y = max->y; max->y = d; }
	if ( min->z > max->z ) { double d = min->z; min->z = max->z; max->z = d; }
    }

    sf->disable_comma--;
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ScanValueSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// result: store value here
)
{
    DASSERT(si);
    DASSERT(var);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    FreeV(var);
    var->mode = VAR_UNSET;
    var->int_mode = IMD_UNSET;
    var->i = 0;

    enumError err = ERR_OK;
    int sign = 0, neg = 0, invert = 0;
    char ch = NextCharSI(si,false);


    //--- arithmetic negation -> delayed

    for(;;)
    {
	if ( ch == '+' )
	    ;
	else if ( ch == '-' )
	    sign++;
	else
	    break;
	sf->ptr++;
	ch = NextCharSI(si,false);
    }


    //--- logical negation

    for(;;)
    {
	if ( ch == '+' )
	    ;
	else if ( ch == '!' )
	    neg++;
	else
	    break;
	sf->ptr++;
	ch = NextCharSI(si,false);
    }

    if (neg)
    {
	err = ScanValueSI(si,var);
	switch (var->mode)
	{
	    case VAR_UNSET:
		break;

	    case VAR_INT:
		var->i = var->i != 0;
		break;

	    case VAR_DOUBLE:
		var->i = var->d != 0;
		break;

	    case VAR_VECTOR:
		var->i = var->x || var->y || var->z;
		break;

	    case VAR_STRING:
		{
		    const int i = var->str_len > 0;
		    FREE(var->str);
		    var->i = i;
		}
		break;
	}
	if (neg&1)
	    var->i = !var->i;
	var->mode = VAR_INT;
	goto value_ok;
    }


    //--- invertion

    for(;;)
    {
	if ( ch == '+' )
	    ;
	else if ( ch == '~' || ch == '^' )
	    invert++;
	else
	    break;
	sf->ptr++;
	ch = NextCharSI(si,false);
    }

    if (invert)
    {
	err = ScanValueSI(si,var);
	switch (var->mode)
	{
	    case VAR_UNSET:
		break;

	    case VAR_INT:
		if (invert&1)
		    var->i = ~var->i;
		break;

	    case VAR_DOUBLE:
		var->i = ~(int)floor(var->d+0.5);
		var->mode = VAR_INT;
		break;

	    case VAR_VECTOR:
		var->mode = VAR_UNSET;
		break;

	    case VAR_STRING:
		ResetV(var);
		break;
	}
	goto value_ok;
    }


    //--- analye operand

    switch(ch)
    {
	case '*':
	    sf->ptr++;
	    var->mode = VAR_INT;
	    var->i = INT_MAX;
	    goto value_ok;

	case '!':
	case '~':
	case '^':
	    err = ScanValueSI(si,var);
	    goto value_ok;

	case '"':
	case '\'':
	    err = ScanStringSI(si,var);
	    goto value_ok;

	case '(':
	    sf->ptr++;
	    err = CheckWarnSI(si,')',ScanExprSI(si,var));
	    goto value_ok;

	case '<':
	    sf->ptr++;
	    err = CheckWarnSI(si,'>',ScanBitfieldSI(si,var));
	    goto value_ok;
    }

    if ( si->force_hex > 0 && IsNum(ch,1) )
    {
	char *end;
	var->i = strtoul( sf->ptr, &end, 16 );
	var->mode = VAR_INT;
	if ( end == sf->ptr )
	{
	    ccp eol = FindNextLineFeedSI(si,true);
	    err = ERROR0(ERR_WARNING,
			"Can't scan a hex number [%s @%u]: %.*s\n",
			sf->name, sf->line, (int)(eol - sf->ptr), sf->ptr );
	}
	sf->ptr = end;
    }
    else if (IsNum(ch,0))
    {
	ccp ptr = sf->ptr;
	if ( ptr[0] != '0' || ptr[1] != 'x' )
	{
	    // int or float

	    if ( sf->disable_comma <= 0 )
	    {
		// is this a floating point with a comma?

		ccp src = ptr;
		char buf[60], *dest = buf;
		char *dest_end = buf + sizeof(buf)/2;
		while ( *src >= '0' && *src <= '9' && dest < dest_end )
		    *dest++ = *src++;
		if ( *src == ',' && src[1] >= '0' && src[1] <= '9' )
		{
		    src++;
		    *dest++ = '.';
		    dest_end = buf + sizeof(buf);
		    while ( *src >= '0' && *src <= '9' && dest < dest_end )
			*dest++ = *src++;
		    if ( dest < dest_end )
		    {
			*dest = 0;
			var->mode = VAR_DOUBLE;
			var->d = strtod(buf,0);
			sf->ptr = src;
			goto value_ok;
		    }
		}
	    }

	    char * end;
	    double d = strtod(ptr,&end);

	    for ( ;  ptr < end; ptr++ )
		if ( *ptr < '0' || *ptr > '9' )
		{
		    // number is float
		    var->mode = VAR_DOUBLE;
		    var->d = d;
		    sf->ptr = end;
		    goto value_ok;
		}
	}

	// number is integer

	const uint base = sf->ptr[1] < '0' || sf->ptr[1] > '9' ? 0 : 10;
	ccp start = sf->ptr;
	var->i = strtoul( sf->ptr, (char**)&sf->ptr, base );
	var->mode = VAR_INT;
	if ( sf->ptr == start && *start == '.' && si->point_is_null > 0 )
	    sf->ptr++;
    }
    else
    {
	char varname[VARNAME_SIZE+1];
	int xyz;
	const uint name_len
	    = ScanNameSI(si,varname,sizeof(varname),false,true,&xyz);
	if (!name_len)
	{
	    if ( si->no_warn <= 0 )
	    {
		ccp eol = FindNextLineFeedSI(si,true);
		if ( sf->line_err < 2 )
		{
		    si->total_err++;
		    ERROR0(ERR_WARNING,
			"Neither name nor number [%s @%u]: %.*s\n",
			sf->name, sf->line,
			(int)( eol - sf->prev_ptr ), sf->prev_ptr );
		}
	    }
	    return ERR_WARNING;
	}

	// outside an expression no spaces are allowed between name and '('
	// because of ambiguous meaning with space separated parameter lists
	// like: name1 (expression2)
	if ( *sf->ptr == '(' || sf->expr_depth && NextCharSI(si,false) == '(' )
	{
	    //--- we have a function call

	    const uint saved_line = sf->line;
	    const ccp  saved_ptr  = sf->prev_ptr;

	    FuncParam_t temppar;
	    const FuncParam_t *fpar = si->no_calc > 0
					? &FuncDollar
					: GetParserFunc(varname,si,&temppar);
	    DASSERT(fpar);
	    noPRINT("FUNC %s(%u..%u)\n",varname,fpar->min_param,fpar->max_param);

	    sf->disable_comma++;
	    sf->ptr++;
	    int n_param = 0;
	    if ( NextCharSI(si,false) != ')' )
	    {
		while ( sf->param_used < STACK_SIZE )
		{
		    Var_t * dest = sf->param + sf->param_used++;
		    if ( !n_param++ && fpar->need_name )
		    {
			char vname[VARNAME_SIZE+1];
			ScanNameSI(si,vname,sizeof(vname),false,true,0);
			dest->name = vname;
		    }
		    else
		    {
			enumError err1 = ScanExprSI(si,dest);
			if ( err < err1 )
			    err = err1;
		    }
		    if ( NextCharSI(si,false) != ',' )
			break;
		    sf->ptr++;
		}
	    }

	    if ( si->no_warn <= 0 )
	    {
		if ( sf->param_used >= STACK_SIZE )
		{
		    ccp eol = FindNextLineFeedSI(si,true);
		    ERROR0(ERR_WARNING,
			    "Stack overflow [%s @%u]: %.*s\n",
			    sf->name, sf->line,
			    (int)(eol - sf->prev_ptr), sf->prev_ptr );
		}
		else if ( n_param < fpar->min_param || n_param > fpar->max_param )
		{
		    ccp eol = FindLineFeedSI(si,saved_ptr,true);
		    ccp attrib = n_param < fpar->min_param ? "few" : "many";
		    if ( fpar->min_param == fpar->max_param )
			ERROR0(ERR_WARNING,
				"To %s parameters for %s(), have %u, need %u [%s @%u]: %.*s\n",
				attrib, varname, n_param, fpar->min_param,
				sf->name, saved_line,
				(int)(eol - saved_ptr), saved_ptr );
		    else
			ERROR0(ERR_WARNING,
				"To %s parameters for %s(), have %u, need %u..%u [%s @%u]: %.*s\n",
				attrib, varname, n_param, fpar->min_param, fpar->max_param,
				sf->name, sf->line,
				(int)(eol - sf->prev_ptr), sf->prev_ptr );
		}
	    }

	    err = CheckWarnSI(si,')',err);
	    sf->disable_comma--;

	    if ( n_param < fpar->min_param )
		    fpar = &FuncDollar;

	    ResetV(var);
	    sf->param_used -= n_param;
	    enumError err1 = fpar->func(var,sf->param+sf->param_used,n_param,si,fpar);
	    if ( err < err1 )
		 err = err1;
	    goto value_ok;
	}

	if (si->no_calc)
	{
	    // don't search the variable
	    FreeV(var);
	    goto value_ok;
	}

	if ( xyz >= 0 )
	{
	    DASSERT( xyz >= 0 && xyz <= 2 );
	    const Var_t *svar = FindVarSI(si,varname,false);
	    if (svar)
	    {
		AssignVar(var,svar);
		goto value_ok;
	    }

	    varname[name_len-2] = 0;
	    svar = FindVarSI(si,varname,false);
	    if ( svar && svar->mode == VAR_VECTOR )
	    {
		var->d = svar->v[xyz];
		var->mode = VAR_DOUBLE;
		goto value_ok;
	    }
	    varname[name_len-2] = '.';
	}

	const Var_t *svar = FindVarSI(si,varname,true);
	AssignVar(var,svar);
    }

 value_ok:

    if (sign&1)
    {
	switch (var->mode)
	{
	    case VAR_UNSET:
		break;

	    case VAR_INT:
		var->i = -var->i;
		break;

	    case VAR_DOUBLE:
		var->d = -var->d;
		break;

	    case VAR_VECTOR:
		var->x = -var->x;
		var->y = -var->y;
		var->z = -var->z;
		break;

	    case VAR_STRING:
		ResetV(var);
		break;
	}
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanUValueSI
(
    ScanInfo_t		* si,		// valid data
    long		* num,		// return value
    int			force_hex	// >0: force hex for non prefixed integers
)
{
    DASSERT(si);
    DASSERT(num);

    const int old_force_hex = si->force_hex;
    si->force_hex = force_hex;

    DEFINE_VAR(var);
    enumError err = ScanValueSI(si,&var);
    si->force_hex = old_force_hex;
    SkipCharSI(si,',');

    switch(var.mode)
    {
	case VAR_UNSET:
	    *num = 0;
	    return err;

	case VAR_INT:
	    *num = var.i;
	    return err;

	case VAR_DOUBLE:
	case VAR_VECTOR:
	    *num = floor( var.d + 0.5 );
	    return err;

	case VAR_STRING:
	    *num = str2l(var.str,0,10);
	    break;
    }

    // only needed to avoid compiler warnings
    *num = 0;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanIntValueSI
(
    ScanInfo_t		* si,		// valid data
    int			* num		// return value
)
{
    DASSERT(si);
    DASSERT(num);

    DEFINE_VAR(var);
    enumError err = ScanValueSI(si,&var);
    SkipCharSI(si,',');

    switch(var.mode)
    {
	case VAR_UNSET:
	    *num = 0;
	    return err;

	case VAR_INT:
	    *num = var.i;
	    return err;

	case VAR_DOUBLE:
	case VAR_VECTOR:
	    *num = double2int(var.d);
	    return err;

	case VAR_STRING:
	    *num = var.str_len ? str2l(var.str,0,10) : 0;
	    break;
    }

    // only needed to avoid compiler warnings
    *num = 0;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanDValueSI
(
    ScanInfo_t		* si,		// valid data
    double		* num		// return value
)
{
    DASSERT(si);
    DASSERT(num);

    DEFINE_VAR(var);
    enumError err = ScanValueSI(si,&var);
    SkipCharSI(si,',');

    switch(var.mode)
    {
	case VAR_UNSET:
	    *num = 0.0;
	    return err;

	case VAR_INT:
	    *num = var.i;
	    return err;

	case VAR_DOUBLE:
	case VAR_VECTOR:
	    *num = var.d;
	    return err;

	case VAR_STRING:
	    *num = var.str_len ? strtod(var.str,0) : 0.0;
	    break;
    }

    // only needed to avoid compiler warnings
    *num = 0.0;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanBitfieldSI
(
    ScanInfo_t		* si,		// valid data
    Var_t		* var		// result: store value here
)
{
    DASSERT(si);
    DASSERT(var);
    ScanFile_t *sf = si->cur_file;
    DASSERT(sf);

    uint val = 0;
    sf->disable_comma++;

    DEFINE_VAR(temp);
    enumError err = ERR_OK;
    for(;;)
    {
	char ch = NextCharSI(si,false);
	if ( !ch || ch == '>' )
	    break;

	err = ScanValueSI(si,&temp);
	if (err)
	    break;

	int n1 = GetIntV(&temp), n2 = n1;
	if ( NextCharSI(si,false) == ':' )
	{
	    sf->ptr++;
	    err = ScanValueSI(si,&temp);
	    if (err)
		break;

	    n2 = GetIntV(&temp);
	}

	if ( n1 < 0 )
	     n1 = 0;
	if ( n2 > CHAR_BIT * sizeof(uint) - 1 )
	     n2 = CHAR_BIT * sizeof(uint) - 1;
	PRINT("BITFIELD: %u..%u => %x\n",n1,n2,val);
	while ( n1 <= n2 )
	    val |= 1u << n1++;

	if ( NextCharSI(si,false) != ',' )
	    break;
	sf->ptr++;
    }

    FreeV(&temp);
    sf->disable_comma--;
    var->mode = VAR_INT;
    var->i = val;
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct Line_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeLine ( Line_t * lin )
{
    DASSERT(lin);
    memset(lin,0,sizeof(*lin));
    lin->n = 1;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanLine
(
    Line_t	* lin,		// valid line data
    ScanInfo_t	* si,		// scanning source
    uint	n_dim,		// number of elements in each vector (<=3)
    int		mode		// user defined mode, stored in 'lin->mode'
)
{
    DASSERT(lin);
    DASSERT(si);
    DASSERT( n_dim > 0 );
    DASSERT( n_dim <= MAX_LINE_DIMEN );

    InitializeLine(lin);
    lin->line_mode = mode;
    if (!NextCharSI(si,false))
	return ERR_OK;

    long n;
    enumError err = ScanUValueSI(si,&n,false);
    if (err)
	return err;
    lin->n = ( n < 0 ? 0 : n ) + 1;

    DEFINE_VAR(var);
    while ( lin->n_pt < MAX_LINE_HELPER && NextCharSI(si,false) )
    {
	err = ScanValueSI(si,&var);
	if (err)
	    goto err;

	double *dest = lin->pt[lin->n_pt];
	*dest++ = GetXDoubleV(&var);
	SkipCharSI(si,',');

	if ( var.mode == VAR_VECTOR )
	{
	    if ( n_dim > 1 )
	    {
		if ( n_dim > 2 )
		    *dest++ = GetYDoubleV(&var);
		*dest++ = GetZDoubleV(&var);
	    }
	}
	else if ( n_dim > 1 )
	{
	    if ( n_dim > 2 )
	    {
		err = ScanValueSI(si,&var);
		if (err)
		    goto err;
		*dest++ = GetYDoubleV(&var);
		SkipCharSI(si,',');
	    }

	    err = ScanValueSI(si,&var);
	    SkipCharSI(si,',');
	    if (err)
		goto err;
	    *dest++ = GetZDoubleV(&var);
	}
	lin->n_pt++;
    }
    FreeV(&var);
    return CheckEolSI(si);

 err:
    FreeV(&var);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

void CalcLine
(
    Line_t	* lin,		// valid line data
    float	* dest,		// destination vector
    const float	* p1,		// source vector P1
    const float	* p2,		// source vector P2
    uint	n_dim,		// number of elements in each vector, n>3 possible
    uint	step		// value between 0 and lin->n inclusive
)
{
    DASSERT(lin);
    DASSERT(dest);

    if ( lin->n <= 0 )
	lin->n = 0;

    if ( step == lin->n )
    {
	// a little optimization for the most common case
	// Important: This case do not access 'p1'

	DASSERT(p2);
	memcpy(dest,p2,sizeof(*dest)*n_dim);
	return;
    }

    if (!step)
    {
	// a little optimization for the most common case
	// Important: This case do not access 'p2'

	DASSERT(p1);
	memcpy(dest,p1,sizeof(*dest)*n_dim);
	return;
    }

    DASSERT(p1);
    DASSERT(p2);
    // http://en.wikipedia.org/wiki/B%C3%A9zier_curve

    uint dim = 0;

    if ( lin->n_pt == 2 )
    {
	// Cubic bezier
	// B(t) = (1-t)^3*P1 + 3*(1-t)^2*t*Pa + 3*(1-t)*t^2*Pb + t^3*P2

	const double t1 = (double)step / (double)lin->n;
	const double t2 = 1.0 - t1;
	double *pa = lin->pt[0];
	double *pb = lin->pt[1];

	uint max_dim = n_dim < MAX_LINE_DIMEN ? n_dim : MAX_LINE_DIMEN;
	for ( ; dim < max_dim; dim++ )
	{
	    *dest++ =    t2 * t2 * t2 * *p1++
		    + 3* t1 * t2 * t2 * *pa++
		    + 3* t1 * t1 * t2 * *pb++
		    +    t1 * t1 * t1 * *p2++;
	}
    }
    else if ( lin->n_pt == 1 )
    {
	// Quadratic bezier
	// B(t) = (1-t)^2*P1 + 2*(1-t)*t*Pa + t^2*P2

	const double t1 = (double)step / (double)lin->n;
	const double t2 = 1.0 - t1;
	double *pa = lin->pt[0];

	uint max_dim = n_dim < MAX_LINE_DIMEN ? n_dim : MAX_LINE_DIMEN;
	for ( ; dim < max_dim; dim++ )
	{
	    *dest++ =    t2 * t2 * *p1++
		    + 2* t1 * t2 * *pa++
		    +    t1 * t1 * *p2++;
	}
    }

    // all other points are calculated linear: B(t) = (1-t)*P1 + t*P2

    while ( dim++ < n_dim )
    {
	*dest++ = step * ( *p2 - *p1 ) / lin->n + *p1;
	p1++;
	p2++;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////


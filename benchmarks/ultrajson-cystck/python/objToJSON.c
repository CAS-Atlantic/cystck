/*
Developed by ESN, an Electronic Arts Inc. studio. 
Copyright (c) 2014, Electronic Arts Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ESN, Electronic Arts Inc. nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS INC. BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Portions of code from MODP_ASCII - Ascii transformations (upper/lower, etc)
http://code.google.com/p/stringencoders/
Copyright (c) 2007  Nick Galbreath -- nickg [at] modp [dot] com. All rights reserved.

Numeric decoder derived from from TCL library
http://www.opensource.apple.com/source/tcl/tcl-14/tcl/license.terms
* Copyright (c) 1988-1993 The Regents of the University of California.
* Copyright (c) 1994 Sun Microsystems, Inc.
*/
#include "../../include/Cystck.h"
#include "py_defines.h"
#include <stdio.h>
#include <ultrajson.h>
 
#define EPOCH_ORD 719163
//static PyObject* type_decimal = NULL;
static Cystck_Object type_decimal = 0;
typedef void *(*PFN_PyTypeToJSON)(JSOBJ obj, JSONTypeContext *ti, void *outValue, size_t *_outLen);

#if (PY_VERSION_HEX < 0x02050000)
typedef ssize_t Py_ssize_t;
#endif

typedef struct __TypeContext
{
  JSPFN_ITEREND iterEnd;
  JSPFN_ITERNEXT iterNext;
  JSPFN_ITERGETNAME iterGetName;
  JSPFN_ITERGETVALUE iterGetValue;
  PFN_PyTypeToJSON PyTypeToJSON;
  Cystck_Object newObj;
  Cystck_Object dictObj;
  Cystck_ssize_t index;
  Cystck_ssize_t size;
  Cystck_Object itemValue;
  Cystck_Object itemName;
  Cystck_Object attrList;
  Cystck_Object iterator;

  union
  {
    Cystck_Object rawJSONValue;
    JSINT64 longValue;
    JSUINT64 unsignedLongValue;
  };
} TypeContext;

#define GET_TC(__ptrtc) ((TypeContext *)((__ptrtc)->prv))
#define GET_PyState(__ptrtc) ((Py_State *)((__ptrtc)->encoder_prv))

#define PRINTMARK()

static inline int _CystckIter_Check(Py_State *S, Cystck_Object h_obj)
{
	return Cystck_HasAttr_s(S, h_obj, "__next__");
}

static inline Cystck_Object _Cystck_GetIter(Py_State *S, Cystck_Object h_iterable)
{
	Cystck_Object h_iter = Cystck_GetAttr_s(S, h_iterable, "__iter__");
	Cystck_Object h_result = Cystck_CallTupleDict(S, h_iter, 0, 0);
	Cystck_DECREF(S, h_iter);
	return h_result;
}

static inline Cystck_Object _Cystck_Next(Py_State *S, Cystck_Object h_iter)
{
	Cystck_Object h_next;
	Cystck_Object h_result;

	if (Cystck_Err_Occurred(S)) {
	    return 0;
	}

	h_next = Cystck_GetAttr_s(S, h_iter, "__next__");
	h_result = Cystck_CallTupleDict(S, h_next, 0, 0);
	Cystck_DECREF(S, h_next);
	if (Cystck_IsNULL(h_result) && Cystck_Err_Occurred(S) && 
	        CystckErr_EXceptionMatches(S, S->Cystck_StopIteration)) {
	    CystckErr_Clear(S);
	}
	return h_result;
}

static inline Cystck_Object _CystckMapping_Keys(Py_State *S, Cystck_Object h_dict)
{
	Cystck_Object h_keys = Cystck_GetAttr_s(S, h_dict, "keys");
	Cystck_Object h_result = Cystck_CallTupleDict(S, h_keys, 0, 0);
	Cystck_DECREF(S, h_keys);
	if (CystckTypeCheck(S, h_result, S->Cystck_ListType)) {
	    return h_result;
	}
	Cystck_Object h_iter = _Cystck_GetIter(S, h_result);
    Cystck_DECREF(S, h_result);
	if (Cystck_IsNULL(h_iter)) {
	    CystckErr_SetString(S, S->Cystck_TypeError, "method 'keys' returned a non-iterable");
	    return 0;
	}

	Cystck_Object h_item;
	h_result = CystckList_New(S, 0);
	if (Cystck_IsNULL(h_result)) {
	    Cystck_DECREF(S, h_iter);
	    return 0;
	}
	for (h_item = _Cystck_Next(S, h_iter); !Cystck_IsNULL(h_item); h_item = _Cystck_Next(S, h_iter)) {
	    if (CystckList_Append(S, h_result, h_item) < 0) {
	        Cystck_DECREF(S, h_iter);
	        Cystck_DECREF(S, h_item);
	        Cystck_DECREF(S, h_result);
	        return 0;
	    }
	    Cystck_DECREF(S, h_item);
	}
	Cystck_DECREF(S, h_iter);
	return h_result;
}

static inline Cystck_Object _Cystck_Sort(Py_State *S, Cystck_Object h_seq)
{
	Cystck_Object h_sort = Cystck_GetAttr_s(S, h_seq, "sort");
	Cystck_Object h_result = Cystck_CallTupleDict(S, h_sort, 0, 0);
	Cystck_DECREF(S, h_sort);
	return h_result;
}

static inline int _CystckLong_Check(Py_State *S, Cystck_Object h_obj)
{
	return CystckTypeCheck(S, h_obj, S->Cystck_LongType);
}

void initObjToJSON(Py_State *S)
{
  Cystck_Object mod_decimal= Cystck_Import_ImportModule("decimal");
  Cystck_pushobject(S,mod_decimal);
  if (!Cystck_IsNULL(mod_decimal))
  {
    type_decimal = Cystck_GetAttr_s(S,mod_decimal,"Decimal");
    Cystck_pushobject(S,type_decimal);
    Cystck_pop(S, mod_decimal);
  }
  else
    Cystck_Err_Clear(S);
}

#ifdef _LP64
static void *PyIntToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  Cystck_Object obj = Cystck_FromVoid(_obj);
  *((JSINT64 *) outValue) = Cystck_Long_AsLong(GET_PyState(tc),obj);
  return NULL;
}
#else
static void *PyIntToINT32(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  Cystck_Object obj = Cystck_FromVoid(_obj);
  *((JSINT32 *) outValue) = Cystck_Long_AsLong(GET_PyState(tc),obj);
  return NULL;
}
#endif

static void *PyLongToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  *((JSINT64 *) outValue) = GET_TC(tc)->longValue;
  return NULL;
}

static void *PyLongToUINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  *((JSUINT64 *) outValue) = GET_TC(tc)->unsignedLongValue;
  return NULL;
}

static void *PyFloatToDOUBLE(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  Cystck_Object obj = Cystck_FromVoid(_obj);
  *((double *) outValue) = CystckFloat_AsDouble (GET_PyState(tc),obj);
  return NULL;
}

static void *PyStringToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  Py_State *S = GET_PyState(tc);
  Cystck_Object obj = Cystck_FromVoid(_obj);
  *_outLen = CystckBytes_GET_SIZE(S,obj);
  return CystckBytes_AS_STRING(S,obj);
}
static void *_CystckUnicodeToUTF8(Cystck_Object c_obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  Py_State *S = GET_PyState(tc);
  Cystck_ssize_t size;
  const char *data = CystckUnicode_AsUTF8AndSize(S,c_obj,&size);
  *_outLen = size;
  return (void *) data;
}

static void *PyUnicodeToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{

  return _CystckUnicodeToUTF8(Cystck_FromVoid(_obj),tc,outValue,_outLen);
}

static void *PyRawJSONToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  Cystck_Object obj = GET_TC(tc)->rawJSONValue;
  Py_State *S = GET_PyState(tc);
  if (CystckUnicode_Check(S,obj))
  {
    return _CystckUnicodeToUTF8(obj,tc,outValue,_outLen);
  }
  else
  {
    
    return PyStringToUTF8(Cystck_AsVoidP(obj), tc, outValue, _outLen);
  }
}

static int Tuple_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  Cystck_Object item;
  Py_State *S = GET_PyState(tc);
  Cystck_Object _obj = Cystck_FromVoid(obj);
  if (GET_TC(tc)->index >= GET_TC(tc)->size)
  {
    return 0;
  }

  item = Cystck_GetItem_i(S,_obj,GET_TC(tc)->index);
  GET_TC(tc)->itemValue = item;
  GET_TC(tc)->index ++;
  return 1;
}

static void Tuple_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
}

static JSOBJ Tuple_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return Cystck_AsVoidP(GET_TC(tc)->itemValue);
}

static char *Tuple_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  return NULL;
}

static int List_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  Py_State *S = GET_PyState(tc);
  Cystck_Object _obj = Cystck_FromVoid(obj);
  if (GET_TC(tc)->index >= GET_TC(tc)->size)
  {
    PRINTMARK();
    return 0;
  }

  GET_TC(tc)->itemValue = Cystck_GetItem_i(S,_obj,GET_TC(tc)->index);
  GET_TC(tc)->index ++;
  return 1;
}

static void List_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
}

static JSOBJ List_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return  Cystck_AsVoidP (GET_TC(tc)->itemValue);
}

static char *List_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  return NULL;
}

//=============================================================================
// Dict iteration functions
// itemName might converted to string (Python_Str). Do refCounting
// itemValue is borrowed from object (which is dict). No refCounting
//=============================================================================

static int Dict_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  Py_State *S = GET_PyState(tc);
  Cystck_Object h_temp = 0;

  if ( !Cystck_IsNULL(GET_TC(tc)->itemName))
  {
    GET_TC(tc)->itemName = 0;
  }

  if (GET_TC(tc)->itemName)
  {
    Cystck_DECREF(S,GET_TC(tc)->itemName);
    GET_TC(tc)->itemName = 0;
  }

  h_temp = _Cystck_Next(S, GET_TC(tc)->iterator);
  if (Cystck_IsNULL(h_temp))
  {
    PRINTMARK();
    return 0;
  }
  GET_TC(tc)->itemName = h_temp;
  h_temp = Cystck_GetItem(S, GET_TC(tc)->dictObj, GET_TC(tc)->itemName);
  if ( Cystck_IsNULL(h_temp))
  {
    PRINTMARK();
    return 0;
  }

  GET_TC(tc)->itemValue = h_temp;

  if (CystckUnicode_Check(S,GET_TC(tc)->itemName))
  {
    GET_TC(tc)->itemName = CystckUnicode_AsUTF8String(S,GET_TC(tc)->itemName);
    Cystck_DECREF(S, h_temp);
  }
  else
  if (!CystckBytes_Check(S,GET_TC(tc)->itemName))
  {
    if (UNLIKELY(GET_TC(tc)->itemName == CystckNone))
    {
      GET_TC(tc)->itemName = CystckUnicode_FromString(S,"null");
      return 1;
    }

    GET_TC(tc)->itemName = Cystck_Str(S,GET_TC(tc)->itemName);
  }
  else
  {
    Cystck_DECREF(S,GET_TC(tc)->itemName);
  }
  PRINTMARK();
  return 1;
}

static void Dict_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
  Py_State *S = GET_PyState(tc);
  if (!Cystck_IsNULL(GET_TC(tc)->itemName))
  {
    Cystck_DECREF(S,GET_TC(tc)->itemName);
    GET_TC(tc)->itemName = 0;
  }
  if (!Cystck_IsNULL(GET_TC(tc)->iterator))
  {
    Cystck_DECREF(S,GET_TC(tc)->iterator);
    GET_TC(tc)->itemName = 0;
  }
  Cystck_DECREF(S, GET_TC(tc)->dictObj);
  PRINTMARK();
}

static JSOBJ Dict_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return Cystck_AsVoidP(GET_TC(tc)->itemValue);
}

static char *Dict_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  Py_State *S = GET_PyState(tc);
  *outLen = CystckBytes_GET_SIZE(S,GET_TC(tc)->itemName);
  return CystckBytes_AS_STRING(S,GET_TC(tc)->itemName);
}

static int SortedDict_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  Cystck_Object items = 0, item = 0, key = 0, value = 0;
  Py_State *S = GET_PyState(tc);
  Cystck_ssize_t i, nitems;
  Cystck_Object keyTmp;

  // Upon first call, obtain a list of the keys and sort them. This follows the same logic as the
  // stanard library's _json.c sort_keys handler.
  if ( Cystck_IsNULL(GET_TC(tc)->newObj))
  {
    // Obtain the list of keys from the dictionary.
    items = _CystckMapping_Keys(S,GET_TC(tc)->dictObj);
    if ( Cystck_IsNULL(items))
    {
      goto error;
    }
    else if (!CystckList_Check(S,items))
    {
      CystckErr_SetString (S, S->Cystck_ValueError, "keys must return list");
      goto error;
    }
    Cystck_pushobject(S,items);

    // Sort the list.
    if ( Cystck_IsNULL(_Cystck_Sort(S,items)))
    {
      CystckErr_Clear(S);
      CystckErr_SetString (S, S->Cystck_ValueError, "unorderable keys");
      goto error;
    }

    // Obtain the value for each key, and pack a list of (key, value) 2-tuples.
    nitems = Cystck_Length(S, items);
    for (i = 0; i < nitems; i++)
    {
      key = Cystck_GetItem_i(S, items,i);
      Cystck_pushobject(S,key);
      value = CystckDict_GetItem (S,GET_TC(tc)->dictObj, key);
      Cystck_pushobject(S,value);
      // Subject the key to the same type restrictions and conversions as in Dict_iterGetValue.
      if (CystckUnicode_Check(S,key))
      {
        keyTmp = key;
        Cystck_pushobject(S, keyTmp);
        key = CystckUnicode_AsUTF8String(S,key);
        Cystck_pop(S, keyTmp);
      }
      else if (!CystckBytes_Check(S,key))
      {
        keyTmp = key;
        Cystck_pushobject(S, keyTmp);
        key = Cystck_Str(S,key);
        Cystck_pop(S, keyTmp);
        keyTmp = key;
        Cystck_pushobject(S, keyTmp);
        key = CystckUnicode_AsUTF8String(S,key);
        Cystck_pop(S,keyTmp);
      }
      item = Cystck_Tuple_Pack(S,2, key, value);
      Cystck_pop(S, value);
      if (Cystck_IsNULL(item) )
      {
        goto error;
      }
      Cystck_pushobject(S,item);
      if (Cystck_SetItem_i(S, items, i, item))
      {
        goto error;
      }
      Cystck_pop(S,item);
      Cystck_pop(S,key);
    }

    // Store the sorted list of tuples in the newObj slot.
    GET_TC(tc)->newObj = items;
    GET_TC(tc)->size = nitems;
  }

  if (GET_TC(tc)->index >= GET_TC(tc)->size)
  {
    PRINTMARK();
    return 0;
  }

  item = Cystck_GetItem_i(S, GET_TC(tc)->newObj, GET_TC(tc)->index);
  Cystck_pushobject(S, item);
  GET_TC(tc)->itemName = Cystck_GetItem_i(S, item, 0);
  GET_TC(tc)->itemValue = Cystck_GetItem_i(S, item, 1);
  GET_TC(tc)->index++;
  Cystck_pop(S, item);
  return 1;

error:
  Cystck_DECREF(S, item);
  Cystck_DECREF(S, key);
  Cystck_DECREF(S, value);
  Cystck_DECREF(S, items);
  return -1;
}

static void SortedDict_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
  Py_State *S = GET_PyState(tc);
  GET_TC(tc)->itemName = 0;
  GET_TC(tc)->itemValue = 0;
  Cystck_DECREF(S, GET_TC(tc)->dictObj);
  PRINTMARK();
}

static JSOBJ SortedDict_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return Cystck_AsVoidP(GET_TC(tc)->itemValue);
}

static char *SortedDict_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  Py_State *S = GET_PyState(tc);
  *outLen = CystckBytes_GET_SIZE(S,GET_TC(tc)->itemName); 
  return CystckBytes_AS_STRING(S,GET_TC(tc)->itemName);
}

static void SetupDictIter(Py_State *S, Cystck_Object dictObj, TypeContext *pc, JSONObjectEncoder *enc)
{
  pc->dictObj = dictObj;
  if (enc->sortKeys)
  {
    pc->iterEnd = SortedDict_iterEnd;
    pc->iterNext = SortedDict_iterNext;
    pc->iterGetValue = SortedDict_iterGetValue;
    pc->iterGetName = SortedDict_iterGetName;
    pc->index = 0;
  }
  else
  {
    pc->iterEnd = Dict_iterEnd;
    pc->iterNext = Dict_iterNext;
    pc->iterGetValue = Dict_iterGetValue;
    pc->iterGetName = Dict_iterGetName;
    pc->iterator = _Cystck_GetIter(S,dictObj);
  }
}

static void Object_beginTypeContext (JSOBJ _obj, JSONTypeContext *tc, JSONObjectEncoder *enc)
{
  Py_State *S = GET_PyState(tc);
  Cystck_Object obj; 
  //objRepr, exc;
  TypeContext *pc;
  PRINTMARK();
  if (!_obj)
  {
    tc->type = JT_INVALID;
    return;
  }

  obj = Cystck_FromVoid(_obj);

  tc->prv = malloc(sizeof(TypeContext));
  pc = (TypeContext *) tc->prv;
  if (!pc)
  {
    tc->type = JT_INVALID;
    CystckErr_NoMemory(S);
    return;
  }
  pc->newObj = 0;
  pc->dictObj = 0;
  pc->itemValue = 0;
  pc->itemName = 0;
  pc->iterator = 0;
  pc->attrList = 0;
  pc->index = 0;
  pc->size = 0;
  pc->longValue = 0;
  pc->rawJSONValue = 0;

  if (_CystckIter_Check(S,obj))
  {
    PRINTMARK();
    goto ISITERABLE;
  }

  if (CystckBool_check(S,obj))
  {
    PRINTMARK();
    tc->type = Cystck_IsTrue(S,obj) ? JT_TRUE : JT_FALSE;
    return;
  }
  else
  if (CystckLong_check(S,obj))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyLongToINT64;
    tc->type = JT_LONG;
    GET_TC(tc)->longValue = CystckLong_AsLongLong(S,obj);

    if (!Cystck_Err_Occurred (S) )
    {
        return;
    }

    if (CystckErr_EXceptionMatches(S, S->Cystck_OverflowError))
    {
      Cystck_Err_Clear(S);
      pc->PyTypeToJSON = PyLongToUINT64;
      tc->type = JT_ULONG;
      GET_TC(tc)->unsignedLongValue = CystckLong_AsUnsignedLongLong(S,obj);

      if (Cystck_Err_Occurred (S) && CystckErr_EXceptionMatches(S,S->Cystck_OverflowError))
      {
        PRINTMARK();
        goto INVALID;
      }
    }

    return;
  }
  else
  if (CystckLong_check(S,obj))
  {
    PRINTMARK();
#ifdef _LP64
    pc->PyTypeToJSON = PyIntToINT64; tc->type = JT_LONG;
#else
    pc->PyTypeToJSON = PyIntToINT32; tc->type = JT_INT;
#endif
    return;
  }
  else
  if (CystckBytes_Check(S,obj))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyStringToUTF8; tc->type = JT_UTF8;
    return;
  }
  else
  if (CystckUnicode_Check(S,obj))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyUnicodeToUTF8; tc->type = JT_UTF8;
    return;
  }
  else
  if (CystckFloat_check(S,obj) || (Cystck_IsNULL(type_decimal) && Cystck_IsInstance(S,obj, type_decimal)))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyFloatToDOUBLE; tc->type = JT_DOUBLE;
    return;
  }
  else
  if (obj == CystckNone)
  {
    PRINTMARK();
    tc->type = JT_NULL;
    return;
  }

ISITERABLE:
  if (CystckDict_Check(S,obj))
  {
    PRINTMARK();
    tc->type = JT_OBJECT;
    SetupDictIter(S, Cystck_Dup(S, obj), pc, enc);
    return;
  }
  else
  if (CystckList_Check(S,obj))
  {
    PRINTMARK();
    tc->type = JT_ARRAY;
    pc->iterEnd = List_iterEnd;
    pc->iterNext = List_iterNext;
    pc->iterGetValue = List_iterGetValue;
    pc->iterGetName = List_iterGetName;
    GET_TC(tc)->index =  0;
    GET_TC(tc)->size = Cystck_Length(S, obj);
    return;
  }
  else
  if (CystckTuple_Check(S,obj))
  {
    PRINTMARK();
    tc->type = JT_ARRAY;
    pc->iterEnd = Tuple_iterEnd;
    pc->iterNext = Tuple_iterNext;
    pc->iterGetValue = Tuple_iterGetValue;
    pc->iterGetName = Tuple_iterGetName;
    GET_TC(tc)->index = 0;
    GET_TC(tc)->size = Cystck_Length(S, obj);
    GET_TC(tc)->itemValue = 0;

    return;
  }

  if (UNLIKELY(Cystck_HasAttrString(S,obj, "toDict")))
  {
    
    Cystck_Object toDictFunc = Cystck_GetAttr_s(S,obj,"toDict");
    Cystck_pushobject(S,toDictFunc);
    Cystck_Object toDictResult = Cystck_CallTupleDict(S,toDictFunc, 0, 0);
    Cystck_pop(S, toDictFunc);
    Cystck_pushobject(S,toDictResult);
    if (Cystck_IsNULL(toDictResult))
    {
      goto INVALID;
    }

    if (!CystckDict_Check(S,toDictResult))
    {
      Cystck_pop(S, toDictResult);
      tc->type = JT_NULL;
      return;
    }

    PRINTMARK();
    tc->type = JT_OBJECT;
    SetupDictIter(S,toDictResult, pc, enc);
    return;
  }
  else
  if (UNLIKELY(Cystck_HasAttrString(S,obj, "__json__")))
  {
    Cystck_Object toJSONFunc = Cystck_GetAttr_String(S,obj, "__json__");
    Cystck_pushobject(S, toJSONFunc);
    Cystck_Object toJSONResult = Cystck_CallTupleDict(S,toJSONFunc, 0, 0);
    Cystck_pop(S, toJSONFunc);
    Cystck_pushobject(S, toJSONResult);


    if (Cystck_IsNULL(toJSONResult))
    {
      goto INVALID;
    }

    if (Cystck_Err_Occurred(S))
    {
      Cystck_pop(S, toJSONResult);
      goto INVALID;
    }

    if (!CystckBytes_Check(S,toJSONResult) && !CystckUnicode_Check(S,toJSONResult))
    {
      Cystck_pop(S, toJSONResult);
      CystckErr_SetString (S, S->Cystck_TypeError, "expected string");
      goto INVALID;
    }

    PRINTMARK();
    pc->PyTypeToJSON = PyRawJSONToUTF8;
    tc->type = JT_RAW;
    GET_TC(tc)->rawJSONValue = toJSONResult;
    return;
  }

  PRINTMARK();
  Cystck_Err_Clear(S);

  CystckErr_SetString (S, S->Cystck_TypeError, "?? is not JSON serializable");

INVALID:
  PRINTMARK();
  tc->type = JT_INVALID;
  free(tc->prv);
  tc->prv = NULL;
  return;
}

static void Object_endTypeContext(JSOBJ obj, JSONTypeContext *tc)
{
  
  Py_State * S = GET_PyState(tc);
  Cystck_DECREF(S, GET_TC(tc)->newObj);

  free(tc->prv);
  tc->prv = NULL;
}

static const char *Object_getStringValue(JSOBJ obj, JSONTypeContext *tc, size_t *_outLen)
{
  return GET_TC(tc)->PyTypeToJSON (obj, tc, NULL, _outLen);
}

static JSINT64 Object_getLongValue(JSOBJ obj, JSONTypeContext *tc)
{
  JSINT64 ret;
  GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);
  return ret;
}

static JSUINT64 Object_getUnsignedLongValue(JSOBJ obj, JSONTypeContext *tc)
{
  JSUINT64 ret;
  GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);
  return ret;
}

static JSINT32 Object_getIntValue(JSOBJ obj, JSONTypeContext *tc)
{
  JSINT32 ret;
  GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);
  return ret;
}

static double Object_getDoubleValue(JSOBJ obj, JSONTypeContext *tc)
{
  double ret;
  GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);
  return ret;
}

static void Object_releaseObject(JSOBJ _obj,JSONTypeContext *tc)
{
  Py_State *S = GET_PyState(tc);
  Cystck_Object obj = Cystck_FromVoid(_obj);
  Cystck_DECREF(S,obj);
}
 
static int Object_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  return GET_TC(tc)->iterNext(obj, tc);
}

static void Object_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
  GET_TC(tc)->iterEnd(obj, tc);
}

static JSOBJ Object_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return GET_TC(tc)->iterGetValue(obj, tc);
}

static char *Object_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  return GET_TC(tc)->iterGetName(obj, tc, outLen);
}

#define ENCODER_HELP_TEXT "Use ensure_ascii=false to output UTF-8. " \
    "Pass in double_precision to alter the maximum digit precision of doubles. " \
    "Set encode_html_chars=True to encode < > & as unicode escape sequences. " \
    "Set escape_forward_slashes=False to prevent escaping / characters." \
    "Set allow_nan=False to raise an exception when NaN or Inf would be serialized."

Cystck_Object objToJSON(Py_State *S, Cystck_Object args, Cystck_Object kwargs)
{
  static char *kwlist[] = { "obj", "ensure_ascii", "encode_html_chars", "escape_forward_slashes", "sort_keys", "indent", NULL };

  char buffer[65536];
  char *ret;
  Cystck_Object newobj;
  Cystck_Object oinput = 0;
  Cystck_Object oensureAscii = 0;
  Cystck_Object oencodeHTMLChars = 0;
  Cystck_Object oescapeForwardSlashes = 0;
  Cystck_Object osortKeys = 0;

  JSONObjectEncoder encoder =
  {
    Object_beginTypeContext,
    Object_endTypeContext,
    Object_getStringValue,
    Object_getLongValue,
    Object_getUnsignedLongValue,
    Object_getIntValue,
    Object_getDoubleValue,
    Object_iterNext,
    Object_iterEnd,
    Object_iterGetValue,
    Object_iterGetName,
    Object_releaseObject,
    malloc,
    realloc,
    free,
    -1, //recursionMax
    1, //forceAscii
    0, //encodeHTMLChars
    1, //escapeForwardSlashes
    0, //sortKeys
    0, //indent
    1, //allowNan
    NULL, //prv
  };
  encoder.prv =S;

  PRINTMARK();

  if (!CystckArg_parseTupleAndKeywords(S,args, kwargs, "O|OOOOi", (const char**)kwlist, &oinput, &oensureAscii, &oencodeHTMLChars, &oescapeForwardSlashes, &osortKeys, &encoder.indent))
  {
    return -1;
  }
  void *_oinput = Cystck_AsVoidP(oinput);
  
  if ( !Cystck_IsNULL(oensureAscii) && !Cystck_IsTrue(S,oensureAscii))
  {
    encoder.forceASCII = 0;
  }
  if (!Cystck_IsNULL(oencodeHTMLChars) && Cystck_IsTrue(S,oencodeHTMLChars))
  {
    encoder.encodeHTMLChars = 1;
  }
  
  if (!Cystck_IsNULL(oescapeForwardSlashes)  && !Cystck_IsTrue(S,oescapeForwardSlashes))
  {
    encoder.escapeForwardSlashes = 0;
  }
  
  if (!Cystck_IsNULL(osortKeys) && Cystck_IsTrue(S,osortKeys))
  {
    encoder.sortKeys = 1;
  }

  
  dconv_d2s_init(DCONV_D2S_EMIT_TRAILING_DECIMAL_POINT | DCONV_D2S_EMIT_TRAILING_ZERO_AFTER_POINT,
                 NULL, NULL, 'e', DCONV_DECIMAL_IN_SHORTEST_LOW, DCONV_DECIMAL_IN_SHORTEST_HIGH, 0, 0);

  PRINTMARK();
  
  ret = JSON_EncodeObject (_oinput, &encoder, buffer, sizeof (buffer));
  
  PRINTMARK();

  dconv_d2s_free();

  if (Cystck_Err_Occurred(S))
  {
    return -1;
  }

  if (encoder.errorMsg)
  {
    if (ret != buffer)
    {
      encoder.free (ret);
    }

    CystckErr_SetString (S, S->Cystck_OverflowError, encoder.errorMsg);
    return -1;
  }

  newobj = CystckUnicode_FromString(S,ret);

  if (ret != buffer)
  {
    encoder.free (ret);
  }

  PRINTMARK();
  Cystck_pushobject(S,newobj);
  return 1;
}

Cystck_Object objToJSONFile(Py_State *S, Cystck_Object args, Cystck_Object kwargs)
{
  Cystck_Object data;
  Cystck_Object file;
  Cystck_Object string;
  Cystck_Object write;
  Cystck_Object argtuple;
  Cystck_Object write_result;
  PRINTMARK();

  if (!CystckArg_parseTuple (S,args, "OO", &data, &file))
  {
    return -1;
  }
  if (!Cystck_HasAttrString (S,file, "write"))
  {
    CystckErr_SetString (S, S->Cystck_TypeError, "expected file");
    return -1;
  }

  write = Cystck_GetAttr_s (S,file, "write");
  Cystck_pushobject(S, write);// pushcFunction

  if (Cystck_IsNULL(write))
  {
    Cystck_pop(S, write);
    return -1;

  }

  if (!Cystck_Callable_Check (S,write))
  {
    Cystck_pop(S, write);
    CystckErr_SetString (S, S->Cystck_TypeError, "expected file");
    return -1;
  }
  argtuple = Cystck_Tuple_Pack(S,1, data); //support type tuple


  Cystck_Object _string = objToJSON (S, argtuple, kwargs);
  string = Cystck_FromPyObject(S, GetResults(S,_string));

  if (Cystck_IsNULL(string))
  {
    Cystck_pop(S,write);
    return -1;
  }
  Cystck_pushobject(S, string);
  argtuple = Cystck_Tuple_Pack(S, 1, string);
  Cystck_pop(S, string);
  if (Cystck_IsNULL(argtuple))
  {
    Cystck_pop(S,write);
    return -1;
  }
  Cystck_pushobject(S, argtuple);
  write_result = Cystck_CallTupleDict(S,write,argtuple, kwargs);
  Cystck_pop(S, argtuple);
  if (Cystck_IsNULL(write_result))
  {
    Cystck_pop(S,write);
    return -1;
  }
  Cystck_pop(S,write);
  
  PRINTMARK();

  Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
  
  return 1;
}
CystckDef_METH(objTo_JSON, "dumps",objToJSON, Cystck_METH_KEYWORDS, "Converts arbitrary object recursively into JSON. " ENCODER_HELP_TEXT);
CystckDef_METH(objToJSON_encode, "encode",objToJSON, Cystck_METH_KEYWORDS, "Converts arbitrary object recursively into JSON. " ENCODER_HELP_TEXT);
CystckDef_METH(objToJSON_File, "dump",objToJSONFile, Cystck_METH_KEYWORDS, "Converts arbitrary object recursively into JSON file. " ENCODER_HELP_TEXT);

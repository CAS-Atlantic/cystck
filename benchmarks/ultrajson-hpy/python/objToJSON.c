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

#include "py_defines.h"
#include <stdio.h>
#include <ultrajson.h>

#define EPOCH_ORD 719163
static HPy type_decimal = HPy_NULL;

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
  HPy newObj;
  HPy dictObj;
  HPy_ssize_t index;
  HPy_ssize_t size;
  HPy itemValue;
  HPy itemName;
  HPy attrList;
  HPy iterator;

  union
  {
    HPy rawJSONValue;
    JSINT64 longValue;
    JSUINT64 unsignedLongValue;
  };
} TypeContext;

#define GET_TC(__ptrtc) ((TypeContext *)((__ptrtc)->prv))
#define GET_HPY_CTX(__ptrtc) ((HPyContext *)((__ptrtc)->encoder_prv))

//#define PRINTMARK() fprintf(stderr, "%s: MARK(%d)\n", __FILE__, __LINE__)
#define PRINTMARK()

/* small helper since there is no HPyUnicode */

static inline int _HPyIter_Check(HPyContext *ctx, HPy h_obj)
{
	return HPy_HasAttr_s(ctx, h_obj, "__next__");
}

static inline HPy _HPy_GetIter(HPyContext *ctx, HPy h_iterable)
{
	HPy h_iter = HPy_GetAttr_s(ctx, h_iterable, "__iter__");
	HPy h_result = HPy_CallTupleDict(ctx, h_iter, HPy_NULL, HPy_NULL);
	HPy_Close(ctx, h_iter);
	return h_result;
}

static inline HPy _HPy_Next(HPyContext *ctx, HPy h_iter)
{
	HPy h_next;
	HPy h_result;

	if (HPyErr_Occurred(ctx)) {
	    return HPy_NULL;
	}

	h_next = HPy_GetAttr_s(ctx, h_iter, "__next__");
	h_result = HPy_CallTupleDict(ctx, h_next, HPy_NULL, HPy_NULL);
	HPy_Close(ctx, h_next);
    /* like in PyIter_Next, the StopIteration exception is cleared */
	if (HPy_IsNull(h_result) && HPyErr_Occurred(ctx) && 
	        HPyErr_ExceptionMatches(ctx, ctx->h_StopIteration)) {
	    HPyErr_Clear(ctx);
	}
	return h_result;
}

static inline HPy _HPyMapping_Keys(HPyContext *ctx, HPy h_dict)
{
	HPy h_keys = HPy_GetAttr_s(ctx, h_dict, "keys");
	HPy h_result = HPy_CallTupleDict(ctx, h_keys, HPy_NULL, HPy_NULL);
	HPy_Close(ctx, h_keys);
	if (HPy_TypeCheck(ctx, h_result, ctx->h_ListType)) {
	    return h_result;
	}
	HPy h_iter = _HPy_GetIter(ctx, h_result);
    HPy_Close(ctx, h_result);
	if (HPy_IsNull(h_iter)) {
	    HPyErr_SetString(ctx, ctx->h_TypeError, "method 'keys' returned a non-iterable");
	    return HPy_NULL;
	}

	HPy h_item;
	h_result = HPyList_New(ctx, 0);
	if (HPy_IsNull(h_result)) {
	    HPy_Close(ctx, h_iter);
	    return HPy_NULL;
	}
	for (h_item = _HPy_Next(ctx, h_iter); !HPy_IsNull(h_item); h_item = _HPy_Next(ctx, h_iter)) {
	    if (HPyList_Append(ctx, h_result, h_item) < 0) {
	        HPy_Close(ctx, h_iter);
	        HPy_Close(ctx, h_item);
	        HPy_Close(ctx, h_result);
	        return HPy_NULL;
	    }
	    HPy_Close(ctx, h_item);
	}
	HPy_Close(ctx, h_iter);
	return h_result;
}

static inline HPy _HPy_Sort(HPyContext *ctx, HPy h_seq)
{
	HPy h_sort = HPy_GetAttr_s(ctx, h_seq, "sort");
	HPy h_result = HPy_CallTupleDict(ctx, h_sort, HPy_NULL, HPy_NULL);
	HPy_Close(ctx, h_sort);
	return h_result;
}

static inline int _HPyLong_Check(HPyContext *ctx, HPy h_obj)
{
	return HPy_TypeCheck(ctx, h_obj, ctx->h_LongType);
}

void initObjToJSON(HPyContext *ctx)
{
  HPy mod_decimal = HPyImport_ImportModule(ctx, "decimal");
  if (!HPy_IsNull(mod_decimal))
  {
	/* TODO(fa): looks like a leak. */
    type_decimal = HPy_GetAttr_s(ctx, mod_decimal, "Decimal");
    HPy_Close(ctx, mod_decimal);
  }
  else
    HPyErr_Clear(ctx);
}

#ifdef _LP64
static void *PyIntToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  HPy obj = HPy_FromVoidP((void *)_obj);
  *((JSINT64 *) outValue) = HPyLong_AsLong(GET_HPY_CTX(tc), obj);
  return NULL;
}
#else
static void *PyIntToINT32(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  HPy obj = HPy_FromVoidP((void *)_obj);
  *((JSINT32 *) outValue) = HPyLong_AsLong(GET_HPY_CTX(tc), obj);
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
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_obj = HPy_FromVoidP((void *)_obj);
  *((double *) outValue) = HPyFloat_AsDouble(ctx, h_obj);
  return NULL;
}

static void *_HPyStringToUTF8(HPy h_obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  *_outLen = HPyBytes_GET_SIZE(ctx, h_obj);
  return (void *) HPyBytes_AS_STRING(ctx, h_obj);
}

static void *PyStringToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  return _HPyStringToUTF8(HPy_FromVoidP((void *)_obj), tc, outValue, _outLen);
}

static void *_HPyUnicodeToUTF8(HPy h_obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy_ssize_t size;
  const char *data = HPyUnicode_AsUTF8AndSize(ctx, h_obj, &size);
  *_outLen = size;
  return (void *) data;
}

static void *PyUnicodeToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  return _HPyUnicodeToUTF8(HPy_FromVoidP((void *) _obj), tc, outValue, _outLen);
}

static void *PyRawJSONToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_obj = GET_TC(tc)->rawJSONValue;
  if (HPyUnicode_Check(ctx, h_obj))
  {
    return _HPyUnicodeToUTF8(h_obj, tc, outValue, _outLen);
  }
  else
  {
    return _HPyStringToUTF8(h_obj, tc, outValue, _outLen);
  }
}

static int Tuple_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_obj = HPy_FromVoidP((void *) obj);
  HPy h_item;

  if (GET_TC(tc)->index >= GET_TC(tc)->size)
  {
    return 0;
  }

  h_item = HPy_GetItem_i(ctx, h_obj, GET_TC(tc)->index);

  GET_TC(tc)->itemValue = h_item;
  GET_TC(tc)->index ++;
  return 1;
}

static void Tuple_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
}

static JSOBJ Tuple_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return (JSOBJ) HPy_AsVoidP(GET_TC(tc)->itemValue);
}

static char *Tuple_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  return NULL;
}

static int List_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_obj = HPy_FromVoidP((void *) obj);

  if (GET_TC(tc)->index >= GET_TC(tc)->size)
  {
    PRINTMARK();
    return 0;
  }

  GET_TC(tc)->itemValue = HPy_GetItem_i(ctx, h_obj, GET_TC(tc)->index);
  GET_TC(tc)->index ++;
  return 1;
}

static void List_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
}

static JSOBJ List_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return (JSOBJ) HPy_AsVoidP(GET_TC(tc)->itemValue);
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
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPyTracker tracker;
  HPy h_tmp = HPy_NULL;

  if (!HPy_IsNull(GET_TC(tc)->itemName))
  {
    HPy_Close(ctx, GET_TC(tc)->itemName);
    GET_TC(tc)->itemName = HPy_NULL;
  }

  h_tmp = _HPy_Next(ctx, GET_TC(tc)->iterator);
  if (HPy_IsNull(h_tmp))
  {
    PRINTMARK();
    return 0;
  }
  GET_TC(tc)->itemName = h_tmp;

  h_tmp = HPy_GetItem(ctx, GET_TC(tc)->dictObj, GET_TC(tc)->itemName);
  if (HPy_IsNull(h_tmp)) {
    PRINTMARK();
    return 0;
  }
  GET_TC(tc)->itemValue = h_tmp;

  if (HPyUnicode_Check(ctx, GET_TC(tc)->itemName))
  {
	h_tmp = GET_TC(tc)->itemName;
    GET_TC(tc)->itemName = HPyUnicode_AsUTF8String (ctx, h_tmp);
    HPy_Close(ctx, h_tmp);
    h_tmp = HPy_NULL;
  }
  else
  if (!HPyBytes_Check(ctx, GET_TC(tc)->itemName))
  {
    if (UNLIKELY(HPy_Is(ctx, ctx->h_None, GET_TC(tc)->itemName)))
    {
      GET_TC(tc)->itemName = HPyUnicode_FromString(ctx, "null");
      return 1;
    }

    tracker = HPyTracker_New(ctx, 2);
	h_tmp = GET_TC(tc)->itemName;
    HPyTracker_Add(ctx, tracker, h_tmp);
    h_tmp = HPy_Str(ctx, h_tmp);
    HPyTracker_Add(ctx, tracker, h_tmp);
    GET_TC(tc)->itemName = HPyUnicode_AsUTF8String(ctx, h_tmp);
    HPyTracker_Close(ctx, tracker);
    h_tmp = HPy_NULL;
  }
  else
  {
	HPy_Close(ctx, GET_TC(tc)->itemName);
  }
  PRINTMARK();
  return 1;
}

static void Dict_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  if (!HPy_IsNull(GET_TC(tc)->itemName))
  {
	HPy_Close(ctx, GET_TC(tc)->itemName);
    GET_TC(tc)->itemName = HPy_NULL;
  }
  if (!HPy_IsNull(GET_TC(tc)->iterator)) {
    HPy_Close(ctx, GET_TC(tc)->iterator);
    GET_TC(tc)->iterator = HPy_NULL;
  }
  HPy_Close(ctx, GET_TC(tc)->dictObj);
  PRINTMARK();
}

static JSOBJ Dict_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return (JSOBJ) HPy_AsVoidP(GET_TC(tc)->itemValue);
}

static char *Dict_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  *outLen = HPyBytes_GET_SIZE(ctx, GET_TC(tc)->itemName);
  return HPyBytes_AS_STRING(ctx, GET_TC(tc)->itemName);
}

static int SortedDict_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_items = HPy_NULL, h_item = HPy_NULL, h_key = HPy_NULL, h_value = HPy_NULL;
  HPy_ssize_t i, nitems;
  HPy h_tmp;

  // Upon first call, obtain a list of the keys and sort them. This follows the same logic as the
  // stanard library's _json.c sort_keys handler.
  if (HPy_IsNull(GET_TC(tc)->newObj))
  {
    // Obtain the list of keys from the dictionary.
    h_items = _HPyMapping_Keys(ctx, GET_TC(tc)->dictObj);
    if (HPy_IsNull(h_items))
    {
      goto error;
    }
    else if (!HPyList_Check(ctx, h_items))
    {
      HPyErr_SetString(ctx, ctx->h_ValueError, "keys must return list");
      goto error;
    }

    // Sort the list.
    if (HPy_IsNull(_HPy_Sort(ctx, h_items)))
    {
      /* we need to raise a ValueError here; so ignore what error 'sort' raised */
      HPyErr_Clear(ctx);
      HPyErr_SetString(ctx, ctx->h_ValueError, "unorderable keys");
      goto error;
    }

    // Obtain the value for each key, and pack a list of (key, value) 2-tuples.

    nitems = HPy_Length(ctx, h_items);
    for (i = 0; i < nitems; i++)
    {
      h_key = HPy_GetItem_i(ctx, h_items, i);
      h_value = HPy_GetItem(ctx, GET_TC(tc)->dictObj, h_key);

      // Subject the key to the same type restrictions and conversions as in Dict_iterGetValue.
      if (HPyUnicode_Check(ctx, h_key))
      {
        h_tmp = h_key;
        h_key = HPyUnicode_AsUTF8String(ctx, h_key);
        HPy_Close(ctx, h_tmp);
      }
      else if (!HPyBytes_Check(ctx, h_key))
      {
        h_tmp = h_key;
        h_key = HPy_Str(ctx, h_key);
        HPy_Close(ctx, h_tmp);
        h_tmp = h_key;
        h_key = HPyUnicode_AsUTF8String(ctx, h_key);
        HPy_Close(ctx, h_tmp);
      }

      h_item = HPyTuple_Pack(ctx, 2, h_key, h_value);
      if (HPy_IsNull(h_item))
      {
        goto error;
      }
      if (HPy_SetItem_i(ctx, h_items, i, h_item))
      {
        goto error;
      }
      HPy_Close(ctx, h_key);
      HPy_Close(ctx, h_item);
    }

    // Store the sorted list of tuples in the newObj slot.
    GET_TC(tc)->newObj = h_items;
    GET_TC(tc)->size = nitems;
  }

  if (GET_TC(tc)->index >= GET_TC(tc)->size)
  {
    PRINTMARK();
    return 0;
  }

  h_item = HPy_GetItem_i(ctx, GET_TC(tc)->newObj, GET_TC(tc)->index);
  GET_TC(tc)->itemName = HPy_GetItem_i(ctx, h_item, 0);
  GET_TC(tc)->itemValue = HPy_GetItem_i(ctx, h_item, 1);
  GET_TC(tc)->index++;
  HPy_Close(ctx, h_item);
  return 1;

error:
   HPy_Close(ctx, h_item);
   HPy_Close(ctx, h_key);
   HPy_Close(ctx, h_value);
   HPy_Close(ctx, h_items);
  return -1;
}

static void SortedDict_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  GET_TC(tc)->itemName = HPy_NULL;
  GET_TC(tc)->itemValue = HPy_NULL;
  HPy_Close(ctx, GET_TC(tc)->dictObj);
  PRINTMARK();
}

static JSOBJ SortedDict_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
  return (JSOBJ) HPy_AsVoidP(GET_TC(tc)->itemValue);
}

static char *SortedDict_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  *outLen = HPyBytes_GET_SIZE(ctx, GET_TC(tc)->itemName);
  return HPyBytes_AS_STRING(ctx, GET_TC(tc)->itemName);
}

static void SetupDictIter(HPyContext *ctx, HPy dictObj, TypeContext *pc, JSONObjectEncoder *enc)
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
    pc->iterator = _HPy_GetIter(ctx, dictObj);
  }
}

static void Object_beginTypeContext (JSOBJ _obj, JSONTypeContext *tc, JSONObjectEncoder *enc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_obj;
  /* HPy h_objRepr, h_exc; */
  TypeContext *pc;
  PRINTMARK();
  if (!_obj)
  {
    tc->type = JT_INVALID;
    return;
  }

  h_obj = HPy_FromVoidP(_obj);

  /* TODO(fa): HPy is missing a memory management API */
  /* tc->prv = PyObject_Malloc(sizeof(TypeContext)); */
  tc->prv = malloc(sizeof(TypeContext));
  pc = (TypeContext *) tc->prv;
  if (!pc)
  {
    tc->type = JT_INVALID;
    HPyErr_NoMemory(ctx);
    return;
  }
  pc->newObj = HPy_NULL;
  pc->dictObj = HPy_NULL;
  pc->itemValue = HPy_NULL;
  pc->itemName = HPy_NULL;
  pc->iterator = HPy_NULL;
  pc->attrList = HPy_NULL;
  pc->index = 0;
  pc->size = 0;
  pc->longValue = 0;
  pc->rawJSONValue = HPy_NULL;

  if (_HPyIter_Check(ctx, h_obj))
  {
    PRINTMARK();
    goto ISITERABLE;
  }

  if (HPy_TypeCheck(ctx, h_obj, ctx->h_BoolType))
  {
    PRINTMARK();
    tc->type = HPy_IsTrue(ctx, h_obj) ? JT_TRUE : JT_FALSE;
    return;
  }
  else
  if (HPy_TypeCheck(ctx, h_obj, ctx->h_LongType))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyLongToINT64;
    tc->type = JT_LONG;
    GET_TC(tc)->longValue = HPyLong_AsLongLong(ctx, h_obj);

    if (!HPyErr_Occurred(ctx))
    {
        return;
    }

    if (HPyErr_ExceptionMatches(ctx, ctx->h_OverflowError))
    {
      HPyErr_Clear(ctx);
      pc->PyTypeToJSON = PyLongToUINT64;
      tc->type = JT_ULONG;
      GET_TC(tc)->unsignedLongValue = HPyLong_AsUnsignedLongLong(ctx, h_obj);

      if (HPyErr_Occurred(ctx) && HPyErr_ExceptionMatches(ctx, ctx->h_OverflowError))
      {
        PRINTMARK();
        goto INVALID;
      }
    }
    return;
  }
  else
  if (_HPyLong_Check(ctx, h_obj))
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
  if (HPyBytes_Check(ctx, h_obj))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyStringToUTF8; tc->type = JT_UTF8;
    return;
  }
  else
  if (HPyUnicode_Check(ctx, h_obj))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyUnicodeToUTF8; tc->type = JT_UTF8;
    return;
  }
  else
  if (HPy_TypeCheck(ctx, h_obj, ctx->h_FloatType) || (!HPy_IsNull(type_decimal) && HPy_TypeCheck(ctx, h_obj, type_decimal)))
  {
    PRINTMARK();
    pc->PyTypeToJSON = PyFloatToDOUBLE; tc->type = JT_DOUBLE;
    return;
  }
  else
  if (HPy_Is(ctx, ctx->h_None, h_obj))
  {
    PRINTMARK();
    tc->type = JT_NULL;
    return;
  }

ISITERABLE:
  if (HPyDict_Check(ctx, h_obj))
  {
    PRINTMARK();
    tc->type = JT_OBJECT;
    SetupDictIter(ctx, HPy_Dup(ctx, h_obj), pc, enc);
    return;
  }
  else
  if (HPyList_Check(ctx, h_obj))
  {
    PRINTMARK();
    tc->type = JT_ARRAY;
    pc->iterEnd = List_iterEnd;
    pc->iterNext = List_iterNext;
    pc->iterGetValue = List_iterGetValue;
    pc->iterGetName = List_iterGetName;
    GET_TC(tc)->index =  0;
    GET_TC(tc)->size = HPy_Length(ctx, h_obj);
    return;
  }
  else
  if (HPyTuple_Check(ctx, h_obj))
  {
    PRINTMARK();
    tc->type = JT_ARRAY;
    pc->iterEnd = Tuple_iterEnd;
    pc->iterNext = Tuple_iterNext;
    pc->iterGetValue = Tuple_iterGetValue;
    pc->iterGetName = Tuple_iterGetName;
    GET_TC(tc)->index = 0;
    GET_TC(tc)->size = HPy_Length(ctx, h_obj);
    GET_TC(tc)->itemValue = HPy_NULL;

    return;
  }

  if (UNLIKELY(HPy_HasAttr_s(ctx, h_obj, "toDict")))
  {
    HPy toDictFunc = HPy_GetAttr_s(ctx, h_obj, "toDict");
    HPy toDictResult = HPy_CallTupleDict(ctx, toDictFunc, HPy_NULL, HPy_NULL);
    HPy_Close(ctx, toDictFunc);

    if (HPy_IsNull(toDictResult))
    {
      goto INVALID;
    }

    if (!HPyDict_Check(ctx, toDictResult))
    {
      HPy_Close(ctx, toDictResult);
      tc->type = JT_NULL;
      return;
    }

    PRINTMARK();
    tc->type = JT_OBJECT;
    SetupDictIter(ctx, toDictResult, pc, enc);
    return;
  }
  else
  if (UNLIKELY(HPy_HasAttr_s(ctx, h_obj, "__json__")))
  {
    HPy toJSONFunc = HPy_GetAttr_s(ctx, h_obj, "__json__");
    HPy toJSONResult = HPy_CallTupleDict(ctx, toJSONFunc, HPy_NULL, HPy_NULL);
    HPy_Close(ctx, toJSONFunc);

    if (HPy_IsNull(toJSONResult))
    {
      goto INVALID;
    }

    if (HPyErr_Occurred(ctx))
    {
      HPy_Close(ctx, toJSONResult);
      goto INVALID;
    }

    if (!HPyBytes_Check(ctx, toJSONResult) && !HPyUnicode_Check(ctx, toJSONResult))
    {
      HPy_Close(ctx, toJSONResult);
      HPyErr_SetString(ctx, ctx->h_TypeError, "expected string");
      goto INVALID;
    }

    PRINTMARK();
    pc->PyTypeToJSON = PyRawJSONToUTF8;
    tc->type = JT_RAW;
    GET_TC(tc)->rawJSONValue = toJSONResult;
    return;
  }

  PRINTMARK();
  HPyErr_Clear(ctx);

  HPyErr_SetString(ctx, ctx->h_TypeError, "?? is not JSON serializable");
  /*
  h_objRepr = HPy_Repr(ctx, h_obj);
  PyErr_Format (PyExc_TypeError, "%s is not JSON serializable", PyBytes_AS_STRING(objRepr));
  HPy_Close(ctx, h_objRepr);
  */

INVALID:
  PRINTMARK();
  tc->type = JT_INVALID;
  /* TODO(fa): HPy is missing a memory management API */
  /* PyObject_Free(tc->prv); */
  free(tc->prv);
  tc->prv = NULL;
  return;
}

static void Object_endTypeContext(JSOBJ obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy_Close(ctx, GET_TC(tc)->newObj);

  /* TODO(fa): HPy is missing a memory management API */
  /* PyObject_Free(tc->prv); */
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

static void Object_releaseObject(JSOBJ _obj, JSONTypeContext *tc)
{
  HPyContext *ctx = GET_HPY_CTX(tc);
  HPy h_obj = HPy_FromVoidP((void *) _obj);
  HPy_Close(ctx, h_obj);
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


#define ENCODER_HELP_TEXT \
    "Use ensure_ascii=false to output UTF-8. Pass in double_precision to" \
    " alter the maximum digit precision of doubles. Set" \
    " encode_html_chars=True to encode < > & as unicode escape sequences." \
    " Set escape_forward_slashes=False to prevent escaping / characters."

HPyDef_METH(objToJSON, "dumps", objToJSON_impl, HPyFunc_KEYWORDS,
            .doc="Converts arbitrary object recursively into JSON. " \
                 ENCODER_HELP_TEXT);

// ujson.c does something a bit weird: it defines two Python-level methods
// ("encode" and "dumps") for the same C-level function
// ("objToJSON_impl"). HPyDef_METH does not support this use case, but we can
// define our HPyDef by hand: HPyDef_METH is just a convenience macro and the
// structure and fields of HPyDef is part of the publich API
HPyDef objToJSON_encode = {
   .kind = HPyDef_Kind_Meth,
   .meth = {
       .name = "encode",
       .impl = objToJSON_impl,
       .cpy_trampoline = objToJSON_trampoline,
       .signature = HPyFunc_KEYWORDS,
       .doc = ("Converts arbitrary object recursively into JSON. "
               ENCODER_HELP_TEXT),
   }
};

static HPy
objToJSON_impl(HPyContext *ctx, HPy self, HPy *args, HPy_ssize_t nargs, HPy kw)
{
  static const char *kwlist[] = { "obj", "ensure_ascii", "encode_html_chars", "escape_forward_slashes", "sort_keys", "indent", NULL };

  char buffer[65536];
  char *ret;
  HPy h_ret;
  HPy oinput = HPy_NULL;
  HPy oensureAscii = HPy_NULL;
  HPy oencodeHTMLChars = HPy_NULL;
  HPy oescapeForwardSlashes = HPy_NULL;
  HPy osortKeys = HPy_NULL;
  HPyTracker ht;
  void *oinput_o;

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
    /* TODO(fa): HPy is missing a memory management API
    PyObject_Malloc,
    PyObject_Realloc,
    PyObject_Free,
    */
	malloc,
	realloc,
	free,
    -1, //recursionMax
    1, //forceAscii
    0, //encodeHTMLChars
    1, //escapeForwardSlashes
    0, //sortKeys
    0, //indent
    NULL, //prv
  };

  // use encoder.prv to pass around the ctx
  encoder.prv = ctx;

  PRINTMARK();

  if (!HPyArg_ParseKeywords(ctx, &ht, args, nargs, kw, "O|OOOOi", kwlist, &oinput, &oensureAscii, &oencodeHTMLChars, &oescapeForwardSlashes, &osortKeys, &encoder.indent))
  {
     return HPy_NULL;
  }

  oinput_o = HPy_AsVoidP(oinput);

  if (!HPy_IsNull(oensureAscii) && !HPy_IsTrue(ctx, oensureAscii))
  {
    encoder.forceASCII = 0;
  }

  if (!HPy_IsNull(oencodeHTMLChars) && HPy_IsTrue(ctx, oencodeHTMLChars))
  {
    encoder.encodeHTMLChars = 1;
  }

  if (!HPy_IsNull(oescapeForwardSlashes) && !HPy_IsTrue(ctx, oescapeForwardSlashes))
  {
    encoder.escapeForwardSlashes = 0;
  }

  if (!HPy_IsNull(osortKeys) && HPy_IsTrue(ctx, osortKeys))
  {
    encoder.sortKeys = 1;
  }

  dconv_d2s_init(DCONV_D2S_EMIT_TRAILING_DECIMAL_POINT | DCONV_D2S_EMIT_TRAILING_ZERO_AFTER_POINT,
                 NULL, NULL, 'e', DCONV_DECIMAL_IN_SHORTEST_LOW, DCONV_DECIMAL_IN_SHORTEST_HIGH, 0, 0);

  PRINTMARK();
  ret = JSON_EncodeObject (oinput_o, &encoder, buffer, sizeof (buffer));
  PRINTMARK();

  dconv_d2s_free();

  if (HPyErr_Occurred(ctx))
  {
    HPyTracker_Close(ctx, ht);
    return HPy_NULL;
  }

  // XXX: FIXME set error with HPy error
  if (encoder.errorMsg)
  {
    if (ret != buffer)
    {
      encoder.free (ret);
    }

    HPyErr_SetString(ctx, ctx->h_OverflowError, encoder.errorMsg);
    HPyTracker_Close(ctx, ht);
    return HPy_NULL;
  }

  h_ret = HPyUnicode_FromString(ctx, ret);

  if (ret != buffer)
  {
    encoder.free (ret);
  }

  PRINTMARK();

  HPyTracker_Close(ctx, ht);
  return h_ret;
}


HPyDef_METH(objToJSONFile, "dump", objToJSONFile_impl, HPyFunc_KEYWORDS,
            .doc="Converts arbitrary object recursively into JSON file. " \
                 ENCODER_HELP_TEXT);
static HPy
objToJSONFile_impl(HPyContext *ctx, HPy self, HPy *args, HPy_ssize_t nargs, HPy kw)
{
  HPy data;
  HPy file;
  HPy string;
  HPy write;
  HPy argtuple;
  HPy result;
  HPyTracker ht;

  PRINTMARK();

  if (!HPyArg_Parse(ctx, &ht, args, nargs, "OO", &data, &file))
  {
    return HPy_NULL;
  }

  if (!HPy_HasAttr_s(ctx, file, "write"))
  {
    HPyTracker_Close(ctx, ht);
    HPyErr_SetString(ctx, ctx->h_TypeError, "expected file");
    return HPy_NULL;
  }

  write = HPy_GetAttr_s(ctx, file, "write");
  if (HPy_IsNull(write)) {
    HPyTracker_Close(ctx, ht);
    return HPy_NULL;
  }

  if (!HPyCallable_Check(ctx, write))
  {
    HPy_Close(ctx, write);
    HPyTracker_Close(ctx, ht);
    HPyErr_SetString(ctx, ctx->h_TypeError, "expected file");
    return HPy_NULL;
  }

  HPy objtojson_args[] = { data };
  string = objToJSON_impl(ctx, self, objtojson_args, 1, kw);
  if (HPy_IsNull(string)) {
    HPy_Close(ctx, write);
    HPyTracker_Close(ctx, ht);
    return HPy_NULL;
  }

  argtuple = HPyTuple_Pack(ctx, 1, string);
  HPy_Close(ctx, string);
  if (HPy_IsNull(argtuple))
  {
    HPy_Close(ctx, write);
    HPyTracker_Close(ctx, ht);
    return HPy_NULL;
  }

  result = HPy_CallTupleDict(ctx, write, argtuple, HPy_NULL);
  if (HPy_IsNull(result))
  {
    HPy_Close(ctx, write);
    HPy_Close(ctx, argtuple);
    HPyTracker_Close(ctx, ht);
    return HPy_NULL;
  }

  HPy_Close(ctx, write);
  HPy_Close(ctx, argtuple);
  HPy_Close(ctx, result);
  HPyTracker_Close(ctx, ht);

  PRINTMARK();

  return HPy_Dup(ctx, ctx->h_None);
}

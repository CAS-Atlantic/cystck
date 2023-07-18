/*******************************************************************************
 * Copyright (c) 1996, 2022 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License.
 *
 * [1] https://www.gnu.org/software/classpath/license.html

 *******************************************************************************/
#ifndef stack_h
#define stack_h

#include "Cystck_method.h"
#include "tree.h"

#define FIELDS_PER_FLUSH 40


typedef struct LocVar
{
  TaggedString *varname;           /* NULL signals end of scope */
  int       line;
} LocVar;



typedef struct TFunc
{
  struct TFunc	*next;
  int		marked;
  int		size;
  Byte		*code;
  int		lineDefined;
  char		*fileName;
  LocVar        *locvars;
} TFunc;
/*
* WARNING: if you change the order of this enumeration,

*/
typedef enum
{
 Cystck_None      = -9,
 Cystck_NUMBER   = -8,
 Cystck_STRING   = -7,
 Cystck_ARRAY    = -6,  /* array==table */
 Cystck_FUNCTION = -5,
 Cystck_CFUNCTION= -4,
 Cystck_MARK     = -3,
 Cystck_CMARK    = -2,
 Cystck_PyOBJ    = -1,
 Cystck_USERDATA = 0
} Cystck_Type;

#define NUM_TYPES 10


#define MULT_RET	255


typedef union
{
 Py_CFunction f;
 Real n;
 //Cystck_Object         *n;
 PyObject *pyobj;
 TaggedString *ts;
 TFunc         *tf;
 struct Hash    *a;

} Value;

typedef struct OBJECT
{
 Cystck_Type ttype;
 Value value;
} OBJECT;

typedef int StkId;  /* index to stack elements */
struct C_Python_Stack {
  StkId base;  /* when Python calls C or C calls python, points to */
               /* the first slot after the last parameter. */
  StkId python2C; /* points to first element of "array" python2C */
  int num;     /* size of "array" pyhton2C */
};

#define CommonHeader	struct GCObject *next; unsigned char tt; unsigned char marked


/* Common type for all collectable objects */
typedef struct GCObject {
  CommonHeader;
} GCObject;

struct Py_State
{
  OBJECT * Top;
  OBJECT * stack;
  OBJECT * stackLimit;
  Cystck_Object self;
  struct C_Python_Stack Cstack;  /* C2Python struct */
  struct C_Python_Stack *Cblocks;
  int numCblocks;  /* number of nested Cblocks */
  PyThreadState *ts;
  GCObject *gclist;
  /* Constants */
  Cystck_Object Cystck_None;
  Cystck_Object Cystck_True;
  Cystck_Object Cystck_False;
  Cystck_Object Cystck_NotImplemented;
  Cystck_Object Cystck_Ellipsis;
  /* Exceptions */
  Cystck_Object Cystck_BaseException;
  Cystck_Object Cystck_Exception;
  Cystck_Object Cystck_StopAsyncIteration;
  Cystck_Object Cystck_StopIteration;
  Cystck_Object Cystck_GeneratorExit;
  Cystck_Object Cystck_ArithmeticError;
  Cystck_Object Cystck_LookupError;
  Cystck_Object Cystck_AssertionError;
  Cystck_Object Cystck_AttributeError;
  Cystck_Object Cystck_BufferError;
  Cystck_Object Cystck_EOFError;
  Cystck_Object Cystck_FloatingPointError;
  Cystck_Object Cystck_OSError;
  Cystck_Object Cystck_ImportError;
  Cystck_Object Cystck_ModuleNotFoundError;
  Cystck_Object Cystck_IndexError;
  Cystck_Object Cystck_KeyError;
  Cystck_Object Cystck_KeyboardInterrupt;
  Cystck_Object Cystck_MemoryError;
  Cystck_Object Cystck_NameError;
  Cystck_Object Cystck_OverflowError;
  Cystck_Object Cystck_RuntimeError;
  Cystck_Object Cystck_RecursionError;
  Cystck_Object Cystck_NotImplementedError;
  Cystck_Object Cystck_SyntaxError;
  Cystck_Object Cystck_IndentationError;
  Cystck_Object Cystck_TabError;
  Cystck_Object Cystck_ReferenceError;
  Cystck_Object Cystck_SystemError;
  Cystck_Object Cystck_SystemExit;
  Cystck_Object Cystck_TypeError;
  Cystck_Object Cystck_UnboundLocalError;
  Cystck_Object Cystck_UnicodeError;
  Cystck_Object Cystck_UnicodeEncodeError;
  Cystck_Object Cystck_UnicodeDecodeError;
  Cystck_Object Cystck_UnicodeTranslateError;
  Cystck_Object Cystck_ValueError;
  Cystck_Object Cystck_ZeroDivisionError;
  Cystck_Object Cystck_BlockingIOError;
  Cystck_Object Cystck_BrokenPipeError;
  Cystck_Object Cystck_ChildProcessError;
  Cystck_Object Cystck_ConnectionError;
  Cystck_Object Cystck_ConnectionAbortedError;
  Cystck_Object Cystck_ConnectionRefusedError;
  Cystck_Object Cystck_ConnectionResetError;
  Cystck_Object Cystck_FileExistsError;
  Cystck_Object Cystck_FileNotFoundError;
  Cystck_Object Cystck_InterruptedError;
  Cystck_Object Cystck_IsADirectoryError;
  Cystck_Object Cystck_NotADirectoryError;
  Cystck_Object Cystck_PermissionError;
  Cystck_Object Cystck_ProcessLookupError;
  Cystck_Object Cystck_TimeoutError;
  /* Warnings */
  Cystck_Object Cystck_Warning;
  Cystck_Object Cystck_UserWarning;
  Cystck_Object Cystck_DeprecationWarning;
  Cystck_Object Cystck_PendingDeprecationWarning;
  Cystck_Object Cystck_SyntaxWarning;
  Cystck_Object Cystck_RuntimeWarning;
  Cystck_Object Cystck_FutureWarning;
  Cystck_Object Cystck_ImportWarning;
  Cystck_Object Cystck_UnicodeWarning;
  Cystck_Object Cystck_BytesWarning;
  Cystck_Object Cystck_ResourceWarning;
  /* Types */
  Cystck_Object Cystck_BaseObjectType;
  Cystck_Object Cystck_TypeType;
  Cystck_Object Cystck_BoolType;
  Cystck_Object Cystck_LongType;
  Cystck_Object Cystck_FloatType;
  Cystck_Object Cystck_UnicodeType;
  Cystck_Object Cystck_TupleType;
  Cystck_Object Cystck_ListType;
};

typedef struct node {
 OBJECT ref;
 OBJECT val;
} Node;

typedef struct Hash {
  struct Hash *next;
  Node *node;
  int nhash;
  int nuse;
  int htag;
  char mark;
} Hash;


/* Macros to access structure members */
#define ttype(o)	((o)->ttype)
#define nvalue(o)	((o)->value.n)
#define objvalue(o)	((o)->value.pyobj)
#define svalue(o)	((o)->value.ts->str)
#define tsvalue(o)	((o)->value.ts)
#define avalue(o)	((o)->value.a)
#define fvalue(o)	((o)->value.f)
void Cystck_pushobject (Py_State*S,Cystck_Object o);
void Cystck_Pushobject(Py_State *S, PyObject *o);
void Cystck_pushnone 		(Py_State*S);
void Cystck_Pushnumber 		(Py_State*S,float n);
void Cystck_Pushstring (Py_State *S,char *s);
int populate(Py_State *Py_state,int val);
Py_State* Get_State();
void freeState(Py_State *state);
void Cystck_pushstring 		(char *s);
void Cystck_pushcfunction	(Py_State *S,Py_CFunction fn);
OBJECT *Cystck_Address (Cystck_Object o);
PyObject *Py_Getobject(Cystck_Object obj);
int isInteger(float N);
void Cystck_error (char *s);

/*#define L	Py_state*/
#endif

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
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memory.h"
#include "stack.h"
#include "../include/Cystck.h"
#include <stdarg.h>
#define tonumber(o) ((ttype(o) != Cystck_NUMBER) && (Cystck_tonumber(o) != 0))
#define tostring(o) ((ttype(o) != Cystck_STRING) && (Cystck_tostring(o) != 0))

#define STACK_SIZE 	128

#ifndef STACK_LIMIT
#define STACK_LIMIT     6000 
#endif
static OBJECT initial_stack;
static OBJECT *top = &initial_stack;
static OBJECT *stackLimit = &initial_stack+1;
static OBJECT *stack = &initial_stack;
#define Address(lo)     ((lo)+stack-1)
#define Ref(st)         ((st)-stack+1)

/* macro to increment stack top. There must be always an empty slot in
*  the stack
*/
#define incr_Top(L)	if (++L->Top >= L->stackLimit) grow_stack(L)
#define incr_top	if (++top >= stackLimit) growstack()

static struct C_Python_Stack CLS_current = {0, 0, 0};
static  jmp_buf *errorJmp = NULL;
/*
** Init stack
*/
static void Initstack (void)
{
 Long maxstack = STACK_SIZE;
 stack = newvector(maxstack, OBJECT);
 stackLimit = stack+maxstack;
 top = stack;
 *(top++) = initial_stack;
}

static void Init_stack (Py_State *S)
{
 Long maxstack = STACK_SIZE;
 stack = newvector(maxstack, OBJECT);
 if (stack == NULL) PyErr_SetString(PyExc_MemoryError,"Memory allocation Failure");
 S->stackLimit = stack+maxstack;
 S->stack = stack;
 S->Top = S->stack;
 *(S->Top++) = initial_stack;
}


/*
** Check stack overflow and, if necessary, realloc vector
*/
#define checkstack(nt)  if ((nt) >= stackLimit) growstack()
#define check_stack(nt, L)  if ((nt) >= L->stackLimit) grow_stack(L)


static void growstack (void)
{
 if (stack == &initial_stack)
   Initstack();
 else
 {
  static int limit = STACK_LIMIT;
  StkId t = top-stack;
  Long stacksize = stackLimit - stack;
  stacksize = growvector(&stack, stacksize, OBJECT, stackEM, limit+100);
  stackLimit = stack+stacksize;
  top = stack + t;
  if (stacksize >= limit)
  {
    limit = stacksize;
    fprintf (stderr, stackEM);
    exit(0);
  }
 }
}

static void grow_stack (Py_State *S)
{
 if (S->stack == &initial_stack){
   Init_stack(S);}
 else
 {
  static int limit = STACK_LIMIT;
  StkId t = S->Top-S->stack;
  Long stacksize = S->stackLimit - S->stack;
  stacksize = growvector(&stack, stacksize, OBJECT, stackEM, limit+100);
  S->stack = stack;
  S->stackLimit = S->stack+stacksize;
  S->Top = S->stack + t;
  if (stacksize >= limit)
  {
    limit = stacksize;
    PyErr_SetString(PyExc_OverflowError,stackEM);
  }
 }
}
 
Cystck_Object Cystck_2C (Py_State *Py_state, int number)
{
  if (number <= 0 || number > Py_state->Cstack.num)return Cystck_NOOBJECT;
  return Py_state->Cstack.python2C+number;
}
/*
** Push a nil object
*/
void Cystck_pushnone (Py_State*S)
{
  ttype(S->Top) = Cystck_None;
  incr_Top(S);
}

/*
** Push an object (ttype=number) to stack.
*/
int isInteger(float N)
{
    int x=N;
    double temp2=N-x;
    if (temp2>0)
    {
        return 0;
    }
    return 1;
}
int Cystck_tag (Cystck_Object lo)
{
  if (lo == Cystck_NOOBJECT) return Cystck_None;
  else {
    OBJECT *o = Address(lo);
    Cystck_Type t = ttype(o);
    if (t == Cystck_USERDATA)
      return o->value.ts->tag;
    else if (t == Cystck_ARRAY)
      return o->value.a->htag;
    else return t;
  }
}
int Cystck_isstring (Cystck_Object o)
{
  int t = Cystck_tag(o);
  return (t == Cystck_STRING) || (t == Cystck_NUMBER);
}
OBJECT *Cystck_Address (Cystck_Object o)
{
  return Address(o);
}
void Cystck_pushobject(Py_State *S, Cystck_Object o)
{
  
  ttype(S->Top) = Cystck_PyOBJ;
  objvalue(S->Top)= Cystck2py(o);
  //objvalue(S->Top)= o;
  incr_Top(S);
}

void Cystck_Pushobject(Py_State *S, PyObject *o)
{
  
  ttype(S->Top) = Cystck_PyOBJ;
  objvalue(S->Top)= o;
  //objvalue(S->Top)= o;
  incr_Top(S);
}

Cystck_Object Cystck_pop(Py_State *S, Cystck_Object O)
{
  Py_XDECREF(Cystck2py(O));
  if ((S->Top-S->stack)==0)
  {
      printf("undeflow, stack empty");
  }
  int nelems = (S->Top-S->stack);
  // for (int i=0; i<nelems ; i++ )
  // {
  //   *(S->Top-i) = *(S->Top-i-1);
  // }
  S->Top-=2;
  incr_Top(S);
  
  return nelems;
}
Cystck_Object Cystck_Value(Py_State *S)
{
 PyObject *obj =objvalue(S->Top-1);
 return Py2Cystck(obj);
}

// void Cystck_Free(Py_State *S, Cystck_Object O)
// {
//    O= Cystck_pop(S, O);
// }


void Cystck_Pushnumber (Py_State*S, Real n)
{
  ttype(S->Top) = Cystck_NUMBER;
  nvalue(S->Top)=n;
  incr_Top(S);
} 
/*
** Push an object (ttype=string) to stack.
*/

void Cystck_Pushstring (Py_State *S,char *s)
{
  if (S->Top>S->stackLimit)
  {
    PyErr_SetString(PyExc_ValueError,"Stack is full"); 
  }
  if (s == NULL)
    ttype(S->Top) = Cystck_None;
  else
  {
    tsvalue(S->Top) = createstring(s);
    ttype(S->Top) = Cystck_STRING;
  }
  incr_Top(S);
}

void Cystck_pushcfunction (Py_State *S,Py_CFunction fn)
{
 ttype(S->Top) = Cystck_CFUNCTION; fvalue(S->Top) = fn;
 incr_Top(S);
}
static PyObject *Num_var(Py_State *S)
{
  PyObject *i;
  if (isInteger(nvalue(S->Top-1)))
    {
      i = PyLong_FromLong(nvalue(S->Top-1));
    }
  else
    {
      i = PyFloat_FromDouble(nvalue(S->Top-1));
    }
  S->Top--;
  return i;
}

static char *String_var(Py_State *S)
{
  char *s = tsvalue(S->Top-1)->str;
  S->Top--;
  return s;
}
PyObject *GetResults(Py_State *S, Cystck_Object ind)
{
  
  int indx = (int) ind;
  int i;
  if (indx <0) return NULL;
  if (S->Top - S->stack <0)
  {
    PyErr_SetString(PyExc_ValueError,"Stack is empty");
    return NULL;
  }
  if (indx == Cystck_NOOBJECT)
  {
    Py_RETURN_NONE;
  }
  PyObject *result;
  if (indx == 1)
  {
    if(ttype(S->Top-1)==Cystck_NUMBER)
    {
      result = Num_var(S);
    }
    else if (ttype(S->Top-1)==Cystck_PyOBJ)
    {
      result = objvalue(S->Top-1);
    }
    else
    {
      result = PyUnicode_FromString(String_var(S));
    }
    return result;
  }
    if ((S->Top - S->stack) < (long)indx)
    {
      PyErr_SetString(PyExc_ReferenceError,"Returning more values than those in stack exiting..\n");
      return NULL;
    }
  PyObject *results = PyTuple_New(indx);
  for (i=0; i<indx; i++)
  {
    if (ttype(S->Top-1)==Cystck_PyOBJ)
    {
      
      printf("BREAK POINT 7 %s\n", __func__ );
      PyTuple_SET_ITEM(results, i, objvalue(S->Top-1));
      S->Top--;
    }
    else if(ttype(S->Top-1)==Cystck_NUMBER)
    {
      printf("BREAK POINT 8 %s\n", __func__ );
      PyTuple_SET_ITEM(results, i, Num_var(S));
    }
    
    else
    {
      printf("BREAK POINT 9 %s\n", __func__ );
      PyTuple_SET_ITEM(results, i, PyUnicode_FromString(String_var(S)));
      
    }
  }
  return results;
}

/*
** Convert, if possible, to a number object.
** Return 0 if success, not 0 if error.
*/
static int Cystck_tonumber (OBJECT *obj)
{
 float t;
 char c;
 if (ttype(obj) != Cystck_STRING)
   return 1;
 else if (sscanf(svalue(obj), "%f %c",&t, &c) == 1)
 {
   nvalue(obj) = t;
   //nvalue(obj) = PyFloat_FromDouble(t);
   ttype(obj) = Cystck_NUMBER;
   return 0;
 }
 else
   return 2;
}


/*
** Convert, if possible, to a string ttype
** Return 0 in success or not 0 on error.
*/
static int Cystck_tostring (OBJECT *obj)
{
  if (ttype(obj) != Cystck_NUMBER)
    return 1;
  else {
    char s[60];
    Real f = nvalue(obj);
    //Real f = PyFloat_AsDouble(nvalue(obj));
    int i;
    if ((Real)(-MAX_INT) <= f && f <= (Real)MAX_INT && (Real)(i=(int)f) == f)
      sprintf (s, "%d", i);
    else
      sprintf (s, "%g", nvalue(obj));
      //sprintf (s, "%g", PyFloat_AsDouble(nvalue(obj))); 
    tsvalue(obj) = createstring(s);
    ttype(obj) = Cystck_STRING;
    return 0;
  }
}

void adjust_top_aux (StkId newtop)
{
  OBJECT *nt;
  checkstack(stack+newtop);
  nt = stack+newtop;   
  while (top < nt) ttype(top++) = Cystck_None;
}



static void Adjust_top_aux (Py_State *S,StkId newtop)
{
  OBJECT *nt;
  check_stack(S->stack+newtop, S);
  nt = S->stack+newtop;   
  while (S->Top < nt) ttype(S->Top++) = Cystck_None;
}

#define adjust_top(newtop)  { if (newtop <= Py_state->Top-Py_state->stack) \
                                 Py_state->Top = Py_state->stack+newtop; \
                              else Adjust_top_aux(Py_state,newtop); }

#define adjustC(nParams)	adjust_top(Py_state->Cstack.base+nParams)

int Cystck_isnumber (Cystck_Object o)
{
  return (o!= Cystck_NOOBJECT) && (tonumber(Address(o)) == 0);
}

Real Cystck_getnumber (Cystck_Object object)
{
 if (object == Cystck_NOOBJECT){ printf("either\n");return 0.0;}
 if (tonumber (Address(object))) { printf("or\n");return 0.0;}
 else return (nvalue(Address(object)));
 //else return PyFloat_AsDouble((nvalue(Address(object))));
}

/*
** Given an object handle, return its string pointer. On error, return NULL.
*/

char *Cystck_getstring (Cystck_Object object)
{
  if (object == Cystck_NOOBJECT || tostring (Address(object)))
    return NULL;
  else return (svalue(Address(object)));
}
void Cystck_error (char *s)
{
  if (errorJmp)
    longjmp(*errorJmp, 1);
  else
  {
    fprintf (stderr, "%s\n", s);
    exit(1);
  }
}

#define MAX_C_BLOCKS 10

static int numCblocks = 0;
//static struct C_Python_Stack Cblocks[MAX_C_BLOCKS];

/*
** API: starts a new block
*/

void Cystck_beginblock (Py_State *Py_state)
{
  Py_state->Cblocks=newvector(STACK_SIZE, struct C_Python_Stack);
  if (numCblocks >= MAX_C_BLOCKS)
  {  Cystck_error("`Cystck_beginblock': too many nested blocks\n");
    free(Py_state->Cblocks);}
  if (Py_state->Cblocks==NULL)
  {
    fprintf(stderr,"Access to invalid memory");
    exit(1);
  }
  Py_state->Cblocks[numCblocks] = Py_state->Cstack;
  printf(" iii%d\n", Py_state->Cstack.base);
  numCblocks++;
}

/*
** API: ends a block
*/
void Cystck_endblock (Py_State *Py_state)
{
  --numCblocks;
  Py_state->Cstack = Py_state->Cblocks[numCblocks];
  adjustC(0);
  free(Py_state->Cblocks);
}

static const char *
parse_err_fmt(const char *fmt, const char **err_fmt)
{
    const char *fmt1 = fmt;

    for (; *fmt1 != 0; fmt1++) {
        if (*fmt1 == ':' || *fmt1 == ';') {
            *err_fmt = fmt1;
            break;
        }
    }
    return fmt1;
}

static int
parse_item(Py_State *Py_state, int nelems, PyObject *current_arg, const char **fmt, va_list *vl)
{
    int z=nelems;
    //Py_state->Cstack.num=0;
    switch (*(*fmt)++) {

    case 'b': { /* unsigned byte -- very short int */
        char *output = va_arg(*vl, char *);
        if (current_arg == NULL) {break;}
        long value = PyLong_AsLong(current_arg);
        if (value == -1 && PyErr_Occurred())
            return 0;
        if (value < 0) {
            PyErr_SetString(PyExc_OverflowError,
                            "unsigned byte integer is less than minimum");
            return 0;
        }
        if (value > UCHAR_MAX) {
          PyErr_SetString(PyExc_OverflowError,
                            "unsigned byte integer is greater than maximum");
            return 0;
        }
        *output = (char) value;
        break;
    }

    case 'B': { /* byte sized bitfield - both signed and unsigned
                   values allowed */
        char *output = va_arg(*vl, char *);
        if (current_arg == NULL) {break;}
        unsigned long value = PyLong_AsUnsignedLongMask(current_arg);
        if (value == (unsigned long)-1 && PyErr_Occurred())
            return 0;
        *output = (unsigned char) value;
        break;
    }

    case 'h': { /* signed short int */
        short *output = va_arg(*vl, short *);
        if (current_arg == NULL) {break;}
        long value = PyLong_AsLong(current_arg);
        if (value == -1 && PyErr_Occurred())
            return 0;
        if (value < SHRT_MIN) {
            PyErr_SetString(PyExc_OverflowError,
                            "signed short integer is less than minimum");
            return 0;
        }
        if (value > SHRT_MAX) {
            PyErr_SetString(PyExc_OverflowError,
                            "signed short integer is greater than maximum");
            return 0;
        }
        *output = (short) value;
        break;
    }

    case 'H': { /* short int sized bitfield, both signed and
                   unsigned allowed */
        unsigned short *output = va_arg(*vl, unsigned short *);
        if (current_arg == NULL) {break;}
        unsigned long value = PyLong_AsUnsignedLongMask(current_arg);
        if (value == (unsigned long)-1 && PyErr_Occurred())
            return 0;
        *output = (unsigned short) value;
        break;
    }

    case 'i': { /* signed int */
        int *output = va_arg(*vl, int *);
        if (current_arg == NULL) {break;}
        long value = PyLong_AsLong(current_arg);
        if (value == -1 && PyErr_Occurred())
            return 0;
        if (value > INT_MAX) {
            PyErr_SetString(PyExc_OverflowError,
                            "signed integer is greater than maximum");
            return 0;
        }
        if (value < INT_MIN) {
            PyErr_SetString(PyExc_OverflowError,
                            "signed integer is less than minimum");
            return 0;
        }
        *output = (int)value;
        stack[z].ttype = Cystck_NUMBER;
        stack[z++].value.n = (float) value;
        //stack[z++].value.n = PyFloat_FromDouble ((double)value);

        Py_state->Cstack.num++;
        break;
    }

    case 'I': { /* int sized bitfield, both signed and
                   unsigned allowed */
        unsigned int *output = va_arg(*vl, unsigned int *);
        if (current_arg == NULL) {break;}
        unsigned long value = PyLong_AsUnsignedLongMask(current_arg);
        if (value == (unsigned long)-1 && PyErr_Occurred())
            return 0;
        *output = (unsigned int) value;
        break;
    }

    case 'l': {
        long *output = va_arg(*vl, long *);
        if (current_arg == NULL) {break;}
        long value = PyLong_AsLong(current_arg);
        if (value == -1 && PyErr_Occurred())
            return 0;
        *output = value;
        stack[z].ttype = Cystck_NUMBER;
        stack[z++].value.n = (float) value;
        //stack[z++].value.n = PyFloat_FromDouble ((double)value);
        Py_state->Cstack.num++;
        break;
    }

    case 'k': { /* long sized bitfield */
        unsigned long *output = va_arg(*vl, unsigned long *);
        if (current_arg == NULL) {break;}
        unsigned long value = PyLong_AsUnsignedLongMask(current_arg);
        if (value == (unsigned long)-1 )
            return 0;
        *output = value;
        break;
    }

    case 'L': { /* long long */
        long long *output = va_arg(*vl, long long *);
        if (current_arg == NULL) {break;}
        long long value = PyLong_AsLongLong(current_arg);
        if (value == (long long)-1 && PyErr_Occurred())
            return 0;
        *output = value;
        break;
    }

    case 'K': { /* long long sized bitfield */
        unsigned long long *output = va_arg(*vl, unsigned long long *);
        if (current_arg == NULL) {break;}
        unsigned long long value = PyLong_AsUnsignedLongLongMask(current_arg);
        if (value == (unsigned long long)-1)
            return 0;
        *output = value;
        break;
    }

    case 'f': { /* float */
        float *output = va_arg(*vl, float *);
        if (current_arg == NULL) {break;}
        double value = PyFloat_AsDouble(current_arg);
        if (value == -1.0 && PyErr_Occurred())
            return 0;
        *output = (float) value;
        stack[z].ttype = Cystck_NUMBER;
        stack[z++].value.n = (float) value;
        //stack[z++].value.n = PyFloat_FromDouble (value);
        Py_state->Cstack.num++;
        break;
    }

    case 'd': { /* double */
        double* output = va_arg(*vl, double *);
        if (current_arg == NULL) {break;}
        double value = PyFloat_AsDouble(current_arg);
        if (value == -1.0 && PyErr_Occurred())
            return 0;
        *output = value;
        stack[z].ttype = Cystck_NUMBER;
        stack[z++].value.n = (float) value;
        //stack[z++].value.n = PyFloat_FromDouble (value);
        Py_state->Cstack.num++;
        break;
    }
    case 'O': {
        Cystck_Object *out = va_arg(*vl, Cystck_Object *);
        if (current_arg == NULL) {break;}
        //PyObject **output = va_arg(*vl, PyObject **);
        
        if (current_arg == NULL) {break;}
        else {
            //*output = current_arg;
            *out = Py2Cystck(current_arg);
            stack[z].ttype = Cystck_PyOBJ;
            stack[z++].value.pyobj = current_arg;
            Py_state->Cstack.num++;
        }
        break;
    }
    case 's': {
        const char **output = va_arg(*vl, const char **);
        if (current_arg == NULL) {break;}
        if (!PyUnicode_Check(current_arg)) {
            PyErr_SetString(PyExc_ValueError, " A string is required");
            return 0;
        }
        Py_ssize_t size;
        const char *data = PyUnicode_AsUTF8AndSize(current_arg, &size);
        if (data == NULL) {
            printf("unicode conversion error");
            return 0;
        }
        // loop bounded by size is more robust/paranoid than strlen
        int i;
        for (i = 0; i < size; ++i) {
            if (data[i] == '\0') {
                PyErr_SetString(PyExc_ValueError, "embedded null character");
                return 0;
            }
        }
        if (data[i] != '\0') {
            PyErr_SetString(PyExc_ValueError, "missing terminating null character");
            return 0;
        }
        *output = data;
        stack[z].ttype = Cystck_STRING;
        stack[z++].value.ts= createstring((char *)data);
        Py_state->Cstack.num++;
        break;
    }

    default: {
        PyErr_SetString(PyExc_SystemError, "unknown arg format code");
        return 0;
    }

    }
    
    // for (int i=0; i<Py_state->Cstack.num; i++)
    // {
    //     if (stack[i].ttype == Cystck_STRING) 
    //     {
    //       printf ("A string Present in Array  %s\n", stack[i].value.ts->str);
    //     }
    //     if (stack[i].ttype == Cystck_NUMBER)
    //     {
    //         printf ("A number Presentin Array %f\n", stack[i].value.n);
    //     } 
    //     if (stack[i].ttype == Cystck_PyOBJ) 
    //     {
    //         printf ("A pyObject Presentin Array\n");
    //     }
        
    // }

    
    return 1;
}


/**
 * Parse positional arguments.
 */

int CystckArg_parseTupleAndKeywords(Py_State *Py_state, Cystck_Object Args, Cystck_Object KW, const char *fmt, const char *keywords[], ...)
{
     
    PyObject *args = Cystck2py(Args);
    PyObject *kw;
    if ( Cystck_IsNULL(KW))
    { 
      kw=NULL;
    }
    else  kw= Cystck2py(KW);
    const char *fmt1 = fmt;
    const char *err_fmt = NULL;
    const char *fmt_end = NULL;
    int nargs = PyTuple_GET_SIZE(args);
    int optional = 0;
    int i = 0;
    PyObject *current_arg;
    int keyword_only = 0;
    int  nkw = 0;
    if (Py_state ==NULL) Py_state = Get_State();
    int nelems = Py_state->Cstack.num;
    fmt_end = parse_err_fmt(fmt, &err_fmt);
    // int nkwargs;
    // nkwargs = (kw == NULL) ? 0 : PyDict_GET_SIZE(kw);
    // first count positional only arguments
    

    while (keywords[nkw] != NULL && !*keywords[nkw]) {
        nkw++;
    }
    // then check and count the rest
    while (keywords[nkw] != NULL) {
        if (!*keywords[nkw]) {
          PyErr_SetString(PyExc_SystemError,"empty keyword parameter name");
            return 0;
        }
        nkw++;
    }
    va_list vl;
    va_start(vl, keywords);
    
    while (fmt1 != fmt_end) {
        if (*fmt1 == '|') {
            optional = 1;
            fmt1++;
            continue;
        }
        if (*fmt1 == '$') {
            optional = 1;
            keyword_only = 1;
            fmt1++;
            continue;
        }
        if (i >= nkw) {

            PyErr_SetString(PyExc_SystemError,
                "mismatched args (too few keywords for fmt)");
            goto error;
        }
        current_arg = NULL;
        if (i < nargs) {
            if (keyword_only) {
                PyErr_SetString(PyExc_TypeError,
                    "keyword only argument passed as positional argument");
                goto error;
            }
            
          current_arg = ((Argobject*)args) -> ob_item[i];
        }
        else if (kw != NULL && *keywords[i]) {
            PyObject* key= PyUnicode_FromString(keywords[i]);
            current_arg = PyObject_GetItem(kw, key);
            if (current_arg == NULL) PyErr_Clear();
        } 
        if (current_arg !=NULL || optional) {
            if (!parse_item(Py_state,nelems,current_arg, &fmt1, &vl)) {
                goto error;
            }   
        }
        else {
          
            PyErr_SetString(PyExc_TypeError,
                "no value for required argument");
            goto error;
        }
        i++;
    }
    // if (i != nkw) {
    //     PyErr_SetString(PyExc_TypeError,"mismatched args (too many keywords for fmt)");
    //     goto error;
    // }

    va_end(vl);
    //printf("Array Items = %d\n",Py_state->Cstack.num);
    return 1;
    error:
        va_end(vl);
        PyErr_SetString(PyExc_SystemError, "Invalid argument to a function");
        return 0;

}
int CystckArg_parseTuple(Py_State *Py_state, Cystck_Object Args,const char *fmt, ...)
{
    PyObject *args;
     int nargs;
    if (Args>0)
    {
      args= Cystck2py(Args);
      nargs = PyTuple_GET_SIZE(args);
    }
    else
    {
      nargs=0;
      args = NULL;
    } 
    //int nargs= nargs = PyTuple_GET_SIZE(args);
    if (Py_state ==NULL) Py_state = Get_State();
    const char *fmt1 = fmt;
    const char *err_fmt = NULL;
    const char *fmt_end = NULL;
    int optional = 0;
    int nelems = Py_state->Cstack.num;
    int i = 0;
    PyObject *current_arg;

    fmt_end = parse_err_fmt(fmt, &err_fmt);


    va_list vl;
    va_start(vl, fmt);
    while (fmt1 != fmt_end) {
        if (*fmt1 == '|') {
            optional = 1;
            fmt1++;
            continue;
        }
        if ((int)strlen(fmt)<nargs)
        {
          PyErr_Format(PyExc_TypeError,
                             "Function takes exactly %d  arguments (%d given)\n",(int)strlen(fmt),nargs);
          break;
        }

        current_arg = NULL;
        if (nargs ==0){
          current_arg = NULL;
        }
        if (i < nargs) {
            //current_arg = args[i];
            current_arg = ((Argobject*)args) -> ob_item[i];
        }
        if (current_arg !=NULL || optional) {
            if (!parse_item(Py_state,nelems,current_arg, &fmt1, &vl)) {
                goto error;
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "required positional argument missing");
            goto error;
        }
        i++;
    }
    va_end(vl);
    //printf("Array Items = %d\n",Py_state->Cstack.num);
    return 1;

    error:
        va_end(vl);
        //PyErr_SetString(PyExc_SystemExit, "Invalid argument to a function");
        return 0;
}


Cystck_Object Cystck_Import(PyObject* module)
{
  stack[CLS_current.num++].value.pyobj = PyImport_Import(module);
  return (Cystck_Object) CLS_current.num;
}

PyObject *Cystck_GetAttrString(Cystck_Object pos , char* str)
{
  
  return PyObject_GetAttrString(objvalue(Address(pos)),"getInteger");
}

Cystck_Object Cystck_CallObject(Py_State *S,PyObject *callable, PyObject *Args)
{
  if (!PyCallable_Check(callable)) {
               PyErr_SetString(PyExc_TypeError, "parameter must be callable");
               return -1;
           }
  PyObject *p = PyObject_CallObject(callable, Args);
  if (PyUnicode_CheckExact(p))
  {
        Py_ssize_t size;
        const char *data = PyUnicode_AsUTF8AndSize(p, &size);
        if (data == NULL) {
            printf("unicode conversion error");
            return 0;
        }
        // check if a string doesnt have a newline character in between
        int i;
        for (i = 0; i < size; ++i) {
            if (data[i] == '\0') {
                printf("embedded null character");
                return 0;
            }
        }
        if (data[i] != '\0') {
            printf("missing terminating null character");
            return 0;
        }
        S->stack[S->Cstack.python2C].ttype = Cystck_STRING;
        S->stack[S->Cstack.python2C].value.ts= createstring((char *)data);
  }
  if (PyFloat_Check(p) == 1)
  {
    printf("hh%d\n", S->Cstack.python2C);
    S->stack[S->Cstack.python2C].ttype = Cystck_NUMBER;
    S->stack[S->Cstack.python2C].value.n = PyFloat_AsDouble(p);
  }
  else{
    S->stack[S->Cstack.python2C].ttype = Cystck_NUMBER;
    S->stack[S->Cstack.python2C].value.n = PyLong_AsLong(p);
  }
  S->Cstack.python2C++;
  S->Cstack.num++;
  return (Cystck_Object) S->Cstack.num;
}
//int z=0;
int populate(Py_State *Py_state,int val)
{
  // printf("Before %s %d\n", __func__,z);
  // if (CLS_current.base!=0){
  //   CLS_current.base=z;
  // }
  Py_state->stack[Py_state->Cstack.base].ttype = Cystck_NUMBER;
  // if (CLS_current.base!=0){
  //   CLS_current.base=z;
  // }
  Py_state->stack[Py_state->Cstack.base++].value.n = val;
  //z= CLS_current.base;
  printf("After %s %d\n", __func__,Py_state->Cstack.base);
  return Py_state->Cstack.base;
}
static struct Py_State  _Py_State;
Py_State * Get_State()
{
  Py_State *Py_state = &_Py_State;
  if (Py_state == NULL) return NULL;
  Py_state->Top=&initial_stack;
  Py_state->stackLimit=&initial_stack+1;
  Py_state->stack=&initial_stack;
  Py_state->Cstack.base = 0;
  Py_state->Cstack.python2C = 0;
  Py_state->Cstack.num = 0;
  Py_state->Cblocks=NULL;
  Py_state->numCblocks=0;
  Py_state->self =0;
  //Py_state->ts=PyThreadState_Get();
  Py_state->ts = NULL;
  Py_state->gclist=NULL;
  Py_state->Cystck_None = Py2Cystck(Py_None);
  Py_state->Cystck_True = Py2Cystck(Py_True);
  Py_state->Cystck_False = Py2Cystck(Py_False);
  Py_state->Cystck_NotImplemented = Py2Cystck(Py_NotImplemented);
  Py_state->Cystck_Ellipsis = Py2Cystck(Py_Ellipsis);
  /* Exceptions */
  Py_state->Cystck_BaseException = Py2Cystck(PyExc_BaseException);
  Py_state->Cystck_Exception = Py2Cystck(PyExc_Exception);
  Py_state->Cystck_StopAsyncIteration = Py2Cystck(PyExc_StopAsyncIteration);
  Py_state->Cystck_StopIteration = Py2Cystck(PyExc_StopIteration);
  Py_state->Cystck_GeneratorExit = Py2Cystck(PyExc_GeneratorExit);
  Py_state->Cystck_ArithmeticError = Py2Cystck(PyExc_ArithmeticError);
  Py_state->Cystck_LookupError = Py2Cystck(PyExc_LookupError);
  Py_state->Cystck_AssertionError = Py2Cystck(PyExc_AssertionError);
  Py_state->Cystck_AttributeError = Py2Cystck(PyExc_AttributeError);
  Py_state->Cystck_BufferError = Py2Cystck(PyExc_BufferError);
  Py_state->Cystck_EOFError = Py2Cystck(PyExc_EOFError);
  Py_state->Cystck_FloatingPointError = Py2Cystck(PyExc_FloatingPointError);
  Py_state->Cystck_OSError = Py2Cystck(PyExc_OSError);
  Py_state->Cystck_ImportError = Py2Cystck(PyExc_ImportError);
  Py_state->Cystck_ModuleNotFoundError = Py2Cystck(PyExc_ModuleNotFoundError);
  Py_state->Cystck_IndexError = Py2Cystck(PyExc_IndexError);
  Py_state->Cystck_KeyError = Py2Cystck(PyExc_KeyError);
  Py_state->Cystck_KeyboardInterrupt = Py2Cystck(PyExc_KeyboardInterrupt);
  Py_state->Cystck_MemoryError = Py2Cystck(PyExc_MemoryError);
  Py_state->Cystck_NameError = Py2Cystck(PyExc_NameError);
  Py_state->Cystck_OverflowError = Py2Cystck(PyExc_OverflowError);
  Py_state->Cystck_RuntimeError = Py2Cystck(PyExc_RuntimeError);
  Py_state->Cystck_RecursionError = Py2Cystck(PyExc_RecursionError);
  Py_state->Cystck_NotImplementedError = Py2Cystck(PyExc_NotImplementedError);
  Py_state->Cystck_SyntaxError = Py2Cystck(PyExc_SyntaxError);
  Py_state->Cystck_IndentationError = Py2Cystck(PyExc_IndentationError);
  Py_state->Cystck_TabError = Py2Cystck(PyExc_TabError);
  Py_state->Cystck_ReferenceError = Py2Cystck(PyExc_ReferenceError);
  Py_state->Cystck_SystemError = Py2Cystck(PyExc_SystemError);
  Py_state->Cystck_SystemExit = Py2Cystck(PyExc_SystemExit);
  Py_state->Cystck_TypeError = Py2Cystck(PyExc_TypeError);
  Py_state->Cystck_UnboundLocalError = Py2Cystck(PyExc_UnboundLocalError);
  Py_state->Cystck_UnicodeError = Py2Cystck(PyExc_UnicodeError);
  Py_state->Cystck_UnicodeEncodeError = Py2Cystck(PyExc_UnicodeEncodeError);
  Py_state->Cystck_UnicodeDecodeError = Py2Cystck(PyExc_UnicodeDecodeError);
  Py_state->Cystck_UnicodeTranslateError = Py2Cystck(PyExc_UnicodeTranslateError);
  Py_state->Cystck_ValueError = Py2Cystck(PyExc_ValueError);
  Py_state->Cystck_ZeroDivisionError = Py2Cystck(PyExc_ZeroDivisionError);
  Py_state->Cystck_BlockingIOError = Py2Cystck(PyExc_BlockingIOError);
  Py_state->Cystck_BrokenPipeError = Py2Cystck(PyExc_BrokenPipeError);
  Py_state->Cystck_ChildProcessError = Py2Cystck(PyExc_ChildProcessError);
  Py_state->Cystck_ConnectionError = Py2Cystck(PyExc_ConnectionError);
  Py_state->Cystck_ConnectionAbortedError = Py2Cystck(PyExc_ConnectionAbortedError);
  Py_state->Cystck_ConnectionRefusedError = Py2Cystck(PyExc_ConnectionRefusedError);
  Py_state->Cystck_ConnectionResetError = Py2Cystck(PyExc_ConnectionResetError);
  Py_state->Cystck_FileExistsError = Py2Cystck(PyExc_FileExistsError);
  Py_state->Cystck_FileNotFoundError = Py2Cystck(PyExc_FileNotFoundError);
  Py_state->Cystck_InterruptedError = Py2Cystck(PyExc_InterruptedError);
  Py_state->Cystck_IsADirectoryError = Py2Cystck(PyExc_IsADirectoryError);
  Py_state->Cystck_NotADirectoryError = Py2Cystck(PyExc_NotADirectoryError);
  Py_state->Cystck_PermissionError = Py2Cystck(PyExc_PermissionError);
  Py_state->Cystck_ProcessLookupError = Py2Cystck(PyExc_ProcessLookupError);
  Py_state->Cystck_TimeoutError = Py2Cystck(PyExc_TimeoutError);
  /* Warnings */
  Py_state->Cystck_Warning = Py2Cystck(PyExc_Warning);
  Py_state->Cystck_UserWarning = Py2Cystck(PyExc_UserWarning);
  Py_state->Cystck_DeprecationWarning = Py2Cystck(PyExc_DeprecationWarning);
  Py_state->Cystck_PendingDeprecationWarning = Py2Cystck(PyExc_PendingDeprecationWarning);
  Py_state->Cystck_SyntaxWarning = Py2Cystck(PyExc_SyntaxWarning);
  Py_state->Cystck_RuntimeWarning = Py2Cystck(PyExc_RuntimeWarning);
  Py_state->Cystck_FutureWarning = Py2Cystck(PyExc_FutureWarning);
  Py_state->Cystck_ImportWarning = Py2Cystck(PyExc_ImportWarning);
  Py_state->Cystck_UnicodeWarning = Py2Cystck(PyExc_UnicodeWarning);
  Py_state->Cystck_BytesWarning = Py2Cystck(PyExc_BytesWarning);
  Py_state->Cystck_ResourceWarning = Py2Cystck(PyExc_ResourceWarning);
  /* Types */
  Py_state->Cystck_BaseObjectType = Py2Cystck((PyObject *)&PyBaseObject_Type);
  Py_state->Cystck_TypeType = Py2Cystck((PyObject *)&PyType_Type);
  Py_state->Cystck_BoolType = Py2Cystck((PyObject *)&PyBool_Type);
  Py_state->Cystck_LongType = Py2Cystck((PyObject *)&PyLong_Type);
  Py_state->Cystck_FloatType = Py2Cystck((PyObject *)&PyFloat_Type);
  Py_state->Cystck_UnicodeType = Py2Cystck((PyObject *)&PyUnicode_Type);
  Py_state->Cystck_TupleType = Py2Cystck((PyObject *)&PyTuple_Type);
  Py_state->Cystck_ListType = Py2Cystck((PyObject *)&PyList_Type);
  return Py_state;
}
void freeState(Py_State *state)
{
  if (state != NULL)
  {
    free(state);
  }
}
void stackStatus(Py_State *S)
{
  adjust_top_aux(0);//does nothing should be removed
  printf("Available elements = %ld\n", S->Top-S->stack);
}
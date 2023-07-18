#ifndef CYSTCK_H
#define CYSTCK_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../src/Cystck_method.h"
#include "../src/Cystck_module.h"
#include "../src/stack.h"
#include "../src/args.h"
#include  "../src/tree.h"
#include "../src/Type.h"
#include "../src/CystckPort.h"
#include "../src/Typeslot.h"

#define Cystck_NOOBJECT  0

#define Cystck_ANYTAG    (-1)


					  /* In: parameters; Out: returns */

Cystck_Object     Cystck_2C 		(Py_State *Py_state,int number);
#define	       getparam(_)		Cystck_2C(_)


#define setVariable(name, variable)                     \
    PyObject *name =PyUnicode_FromString(variable);
    
Real Cystck_getnumber (Cystck_Object object);
char *Cystck_getstring (Cystck_Object object);
int Cystck_isstring (Cystck_Object o);
int Cystck_isnumber (Cystck_Object o);
#define	  Cystck_getparam(state, pos)		Cystck_2C(state,pos)
#define	  Cystck_getResult(state, pos)		Cystck_2C(state,pos)
#define Cystck_TPFLAGS_DEFAULT            ( Py_TPFLAGS_HAVE_STACKLESS_EXTENSION | 0)
#define Cystck_TPFLAGS_BASETYPE            (1UL << 10)
#define Cystck_TPFLAGS_HAVE_GC              Py_TPFLAGS_HAVE_GC
int getArgs(PyObject *args);
int CystckArg_parseTuple(Py_State *Py_state, Cystck_Object Args,const char *fmt, ...);
int CystckArg_parseTupleAndKeywords(Py_State *Py_state,Cystck_Object  Args, Cystck_Object  KW, const char *fmt, const char *keywords[], ...);
void Cystck_beginblock (Py_State *Py_state);
void Cystck_endblock (Py_State *Py_state);
Cystck_ssize_t Cystck_Length(Py_State *S, Cystck_Object obj);
Cystck_Object Cystck_Value(Py_State *S);
Cystck_Object Cystck_pop(Py_State *S, Cystck_Object O);
Cystck_ssize_t CystckBytes_GET_SIZE(Py_State *S, Cystck_Object O);
Cystck_Object Cystck_Import(PyObject* module);
int CystckList_SetItem(Py_State *S, Cystck_Object obj1, Cystck_ssize_t i, Cystck_Object obj2);
PyObject *Cystck_GetAttrString(Cystck_Object pos, char* str);
PyMethodDef *createMethods(CystckDef **methods, PyMethodDef *legacy_methods);
Cystck_Object Cystck_Tuple_Pack(Py_State *S,Cystck_ssize_t size, ...);
static inline Cystck_ssize_t CystckList_GET_SIZE(Cystck_Object o)
{
    return PyList_GET_SIZE( Cystck2py(o));
}
static inline Cystck_ssize_t CystckTuple_GET_SIZE(Cystck_Object o)
{
    return PyTuple_GET_SIZE( Cystck2py(o));

}
static inline Cystck_Object CystckTuple_GET_ITEM(Py_State *S, Cystck_Object o, int i)
{
    PyObject *res = PyTuple_GET_ITEM(Cystck2py(o),i);
    Cystck_pushobject(S, Py2Cystck(res));
    return  Py2Cystck(res);
}
Cystck_Object CystckList_New(Py_State *S, Cystck_ssize_t len);
// #define Cystck_True     Py2Cystck(Py_True)
// #define Cystck_False     Py2Cystck(Py_False)
// #define Cystck_None     Py2Cystck(Py_None)
Cystck_Object CystckUnicode_FromWideChar(Py_State *S, const wchar_t *w, Cystck_ssize_t size);
Cystck_Object CystckTuple_New(Py_State *S, Cystck_ssize_t size);
void stackStatus(Py_State *S);
Real Cystck_getnumber (Cystck_Object object);
char *Cystck_getstring (Cystck_Object object);
int Cystck_isstring (Cystck_Object o);
int Cystck_isnumber (Cystck_Object o);
#define	       Cystck_getparam(state,pos)		Cystck_2C(state,pos)
int Cystck_ParseArgs(PyObject *args);
Cystck_Object Cystck_CallObject(Py_State *S,PyObject *callable, PyObject *Args);


#ifdef __cplusplus
}
#endif
#endif

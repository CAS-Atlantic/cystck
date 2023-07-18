#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/Cystck.h"

int Cystck_IsTrue(Py_State *S, Cystck_Object o)
{
    if (o==0) return 1;
    else return PyObject_IsTrue(Cystck2py(o));
}
Cystck_Object Cystck_Import_ImportModule(const char *name)
{
    return Py2Cystck(PyImport_ImportModule(name));
}

double CystckLong_AsDouble(Py_State  *s, Cystck_Object obj)
{
    return PyLong_AsDouble(Cystck2py(obj));
}
int CystckBool_check(Py_State *S, Cystck_Object O)
{
    if (PyBool_Check(Cystck2py(O))) return 1;
    else return 0;
}

int CystckLong_check(Py_State *S, Cystck_Object O)
{
    if (PyLong_Check(Cystck2py(O))) return 1;
    else return 0;
}

int CystckFloat_check(Py_State *S, Cystck_Object O)
{
    if (PyFloat_Check(Cystck2py(O))) return 1;
    else return 0;
}

int Cystck_IsInstance(Py_State *S, Cystck_Object obj, Cystck_Object typ)
{
    return PyObject_IsInstance (Cystck2py(obj), Cystck2py(typ));
}
int CystckDict_Check(Py_State *S,Cystck_Object O)
{
    return PyDict_Check(Cystck2py(O));
}
long long CystckLong_AsLongLong(Py_State *S, Cystck_Object  O)
{
    return PyLong_AsLongLong(Cystck2py(O));
}

Cystck_Object Cystck_GetAttr_String(Py_State *S, Cystck_Object obj, const char *name)
{
    Cystck_Object O=  Py2Cystck(PyObject_GetAttrString(Cystck2py(obj), name));
    return O;
    //PyObject_GetAttrString(obj, name);
}
long Cystck_Long_AsLong(Py_State *S, Cystck_Object O)
{
    return PyLong_AsLong(Cystck2py(O));
}

int CystckNumber_Check(Py_State *S, Cystck_Object O)
{
    return PyNumber_Check(Cystck2py(O));
}

Cystck_ssize_t CystckBytes_GET_SIZE(Py_State *S, Cystck_Object O)
{
    return PyBytes_GET_SIZE(Cystck2py(O));
}
char *CystckBytes_AS_STRING(Py_State *S, Cystck_Object O)
{
    return PyBytes_AS_STRING(Cystck2py(O));
}
Cystck_Object CystckTuple_GetItem(Py_State *S, Cystck_Object O, int size)
{
    return Py2Cystck( PyTuple_GetItem( Cystck2py(O),size));
}


Cystck_Object CystckIter_Next(Py_State *S, Cystck_Object o)
{
    return Py2Cystck( PyIter_Next (Cystck2py(o)));
}

Cystck_Object Cystck_GetItem(Py_State *S, Cystck_Object o, Cystck_Object key)
{
    return Py2Cystck(PyObject_GetItem( Cystck2py(o), Cystck2py(key)));
}
Cystck_Object CystckList_GetItem(Py_State *S, Cystck_Object O, int size)
{
    return Py2Cystck( PyList_GetItem( Cystck2py(O),size));
}
int CystckUnicode_Check(Py_State *S, Cystck_Object O)
{
    return PyUnicode_Check(Cystck2py(O));
}
Cystck_Object CystckDict_GetItem (Py_State *S, Cystck_Object mp, Cystck_Object key)
{
    return Py2Cystck (PyDict_GetItem (Cystck2py(mp), Cystck2py(key)));
}

Cystck_Object CystckUnicode_AsUTF8String(Py_State *S, Cystck_Object o)
{
    return Py2Cystck(PyUnicode_AsUTF8String( Cystck2py(o)));
}
int CystckBytes_Check(Py_State *S, Cystck_Object o)
{
    return PyBytes_Check(Cystck2py(o));
}
Cystck_Object CystckUnicode_FromString(Py_State *S, const char *utf8)
{
    return Py2Cystck(PyUnicode_FromString(utf8));
}
long CystckLong_AsLong(Py_State *S, Cystck_Object obj)
{
    return PyLong_AsLong(Cystck2py(obj));
}
Cystck_Object CystckErr_NewException(Py_State *S, const char *name, Cystck_Object base, Cystck_Object dict)
{
    Cystck_pushobject(S, Py2Cystck(PyErr_NewException(name, Cystck2py(base), Cystck2py(dict))));
    return Py2Cystck(PyErr_NewException(name, Cystck2py(base), Cystck2py(dict)));
}
Cystck_ssize_t Cystck_Length(Py_State *S, Cystck_Object obj)
{
    return PyObject_Length(Cystck2py(obj));
}

int CystckList_SetItem(Py_State *S, Cystck_Object obj1, Cystck_ssize_t i, Cystck_Object obj2)
{
    return PyList_SetItem(Cystck2py(obj1), i, Cystck2py(obj2));
}

Cystck_Object Cystck_GetIter(Py_State *S, Cystck_Object O)
{
    return Py2Cystck (PyObject_GetIter( Cystck2py(O)));
}
int CystckIter_Check(Py_State *S, Cystck_Object O)
{
    return PyIter_Check(Cystck2py(O));
}
Cystck_Object CystckErr_NoMemory(Py_State *S)
{
    PyErr_NoMemory();
    return 0;
}
Cystck_Object Cystck_Str(Py_State *S, Cystck_Object obj)
{
    return Py2Cystck(PyObject_Str(Cystck2py(obj)));
}

Cystck_Object CystckMapping_Keys(Py_State *S, Cystck_Object o)
{
    return Py2Cystck (PyMapping_Keys(Cystck2py(o)));
}

int CystckDict_SetItem(Py_State *S,Cystck_Object mp, Cystck_Object key, Cystck_Object item)
{
    return PyDict_SetItem( Cystck2py(mp),Cystck2py(key),Cystck2py(item));
}
Cystck_Object CystckDict_New(Py_State *S)
{
    return Py2Cystck (PyDict_New());
}
Cystck_Object CystckList_New(Py_State *S, Cystck_ssize_t len)
{
    return Py2Cystck(PyList_New(len));
}
Cystck_Object CystckLong_FromLong(Py_State *S, long value)
{
    return Py2Cystck(PyLong_FromLong(value));
}

Cystck_Object CystckLong_FromUnsignedLong(Py_State *S, unsigned long value)
{
    return Py2Cystck(PyLong_FromUnsignedLong(value));
}

Cystck_Object CystckLong_FromLongLong(Py_State *S, long long v)
{
    return Py2Cystck(PyLong_FromLongLong(v));
}
Cystck_Object CystckFloat_FromDouble(Py_State *S, double v)
{
    return Py2Cystck(PyFloat_FromDouble(v));
}

int CystckList_Append(Py_State *S, Cystck_Object Cystck_list, Cystck_Object Cystck_item)
{
    return PyList_Append(Cystck2py(Cystck_list), Cystck2py(Cystck_item));
}
Cystck_Object CystckUnicode_FromWideChar(Py_State *S, const wchar_t *w, Cystck_ssize_t size)
{
    return Py2Cystck (PyUnicode_FromWideChar(w, size));
}


Cystck_Object CystckList_GET_ITEM(Py_State *S, Cystck_Object data, int index)
{
    Cystck_pushobject(S,Py2Cystck(PyList_GET_ITEM(Cystck2py(data), index)));
    return Py2Cystck(PyList_GET_ITEM(Cystck2py(data), index));
}

int CystckList_Check(Py_State *S, Cystck_Object o)
{
    return PyList_Check(Cystck2py(o));
}

ssize_t CystckList_Size(Py_State *S, Cystck_Object o)
{
    return PyList_Size(Cystck2py(o));
}
int CystckTuple_Check(Py_State *S, Cystck_Object O)
{
    return PyTuple_Check(Cystck2py(O));
}
Cystck_Object CystckTuple_New(Py_State *S, Cystck_ssize_t size)
{
    Cystck_pushobject(S, Py2Cystck(PyTuple_New(size)));
    return Py2Cystck(PyTuple_New(size));
}
int CystckList_Sort(Py_State *S, Cystck_Object o)
{
    return PyList_Sort (Cystck2py(o));
}

void Cystck_Err_Clear(Py_State *S)
{
    PyErr_Clear();
}
int Cystck_Callable_Check(Py_State *S, Cystck_Object O)
{
    return PyCallable_Check( Cystck2py(O));
}

int Cystck_Err_Occurred(Py_State *S) {
    return PyErr_Occurred() ? 1 : 0;
}

void CystckGlobal_Store(Py_State *S, CystckGlobal *global, Cystck_Object o)
{
    PyObject *obj = Cystck2py(o);
    Py_XDECREF(Cystck2py(*global));
    Py_XINCREF(obj);
    *global = Py2Cystck(obj);
}

Cystck_Object
Cystck_GetItem_i(Py_State *S, Cystck_Object obj, ssize_t idx) {
    PyObject* key = PyLong_FromSsize_t(idx);
    if (key == NULL)
        return -1;
    Cystck_Object result = Py2Cystck(PyObject_GetItem( Cystck2py(obj), key));
    Py_DECREF(key);
    return result;
}

CystckListBuilder
CystckListBuilder_New(Py_State *C, ssize_t initial_size)
{
    PyObject *lst = PyList_New(initial_size);
    if (lst == NULL)
        PyErr_Clear();   /* delay the MemoryError */
    return (CystckListBuilder)lst;
}

void
CystckListBuilder_Set(Py_State *S, CystckListBuilder builder,
                    ssize_t index, Cystck_Object _item)
{
    PyObject *lst = (PyObject *)builder;
    if (lst != NULL) {
        PyObject *item = Cystck2py(_item);
        assert(index >= 0 && index < PyList_GET_SIZE(lst));
        assert(PyList_GET_ITEM(lst, index) == NULL);
        Py_INCREF(item);
        PyList_SET_ITEM(lst, index, item);
    }
}

Cystck_Object
CystckListBuilder_Build(Py_State *S, CystckListBuilder builder)
{
    PyObject *lst = (PyObject *)builder;
    if (lst == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    builder = 0;
    return Py2Cystck(lst);
}

void
CystckListBuilder_Cancel(Py_State *S, CystckListBuilder builder)
{
    PyObject *lst = (PyObject *)builder;
    if (lst == NULL) {
        return;
    }
    builder = 0;
    Py_XDECREF(lst);
}

int Cystck_Is(Py_State *S, Cystck_Object _obj, Cystck_Object _other)
{
    return Cystck2py(_obj) == Cystck2py(_other);
}

void
CystckTuple_SET_ITEM(Py_State *S, Cystck_Object tp, ssize_t index, Cystck_Object _item)
{
    PyObject *tup = Cystck2py(tp);
    if (tup != NULL) {
        PyObject *item = Cystck2py(_item);
        assert(index >= 0 && index < PyTuple_GET_SIZE(tup));
        assert(PyTuple_GET_ITEM(tup, index) == NULL);
        Py_INCREF(item);
        PyTuple_SET_ITEM(tup, index, item);
    }
    tp = Py2Cystck(tup);
    Cystck_pushobject(S, tp);
}

unsigned long long CystckLong_AsUnsignedLongLong(Py_State *S, Cystck_Object O)
{
    return PyLong_AsUnsignedLongLong(Cystck2py(O));
}
int CystckErr_EXceptionMatches(Py_State*S, Cystck_Object O)
{
    return PyErr_ExceptionMatches(Cystck2py(O));
}
void CystckErr_SetString(Py_State *S, Cystck_Object o, const char *message)
{
    PyErr_SetString( Cystck2py(o), message);
    return;
}
int Cystck_SetAttrString (Py_State *S, Cystck_Object obj, const char *name, Cystck_Object value)
{
    return PyObject_SetAttrString(Cystck2py(obj), name, Cystck2py(value));
}
static PyModuleDef empty_moduledef = {
    PyModuleDef_HEAD_INIT
};

Cystck_Object CystckModule_Create(Py_State *Py_state, CyModuleDef *ModDef)
{
    if (Py_state == NULL) 
    {
        PyErr_SetString(PyExc_MemoryError,"Memory allocation Failure for State");
        return 0;
    }
    PyModuleDef *def = (PyModuleDef*)malloc(sizeof(PyModuleDef));
    if(def==NULL)
    { 
       PyErr_SetString(PyExc_MemoryError,"Memory allocation Failure");
       return 0;
    }
    memcpy(def,&empty_moduledef, sizeof(PyModuleDef));
    def->m_name = ModDef->m_name;
    def->m_doc =ModDef->m_doc;
    def->m_size = ModDef->m_size;
    //def->m_methods= ModDef->m_methods;
    def->m_methods = createMethods(ModDef->m_methods, ModDef->legacy_methods);
    Cystck_Object module = Py2Cystck(PyModule_Create(def));
    Cystck_pushobject(Py_state, module);
    return  module;
}

int CystckModule_AddObject(Py_State *S,Cystck_Object mod, const char * str, Cystck_Object value)
{
    return PyModule_AddObject(Cystck2py(mod),str, Cystck2py(value));
}

Cystck_Object CystckGlobal_Load(Py_State *S, CystckGlobal global)
{
    PyObject *obj = CystckGlobal2py(global);
    Py_INCREF(obj);
    Cystck_pushobject(S, Py2Cystck(obj));
    return Py2Cystck(obj);
}

int Cystck_Print(Py_State *S, Cystck_Object o, FILE *fp, int flags)
{
    return PyObject_Print( Cystck2py(o), fp, flags) ;
}
Cystck_Object _Cystck_New(Py_State *S, Cystck_Object type, void **data)
{
    PyTypeObject *tp = (PyTypeObject*) Cystck2py(type);
    
    assert(tp != NULL);
    if (!PyType_Check(tp)) {
        PyErr_SetString(PyExc_TypeError, "Cystck_New arg 1 must be a type");
        return Cystck_NULL;
    }

    PyObject *result;
    if (PyType_IS_GC(tp))
        result = PyObject_GC_New(PyObject, tp);
    else
        result = PyObject_New(PyObject, tp);
    // Cystck_ssize_t payload_size = tp->tp_basicsize - _Cystck_PyObject_HEAD_SIZE;
    // memset (_Cystck_PyObject_Payload(result),0,payload_size);

    if (PyType_IS_GC(tp))
        PyObject_GC_Track(result);
    if (!result)
        return 0;
    *data = (void *) result;
    //Cystck_pushobject(S,Py2Cystck(result)); did this when importing numpy
    return Py2Cystck(result);
}

CystckTupleBuilder
CystckTupleBuilder_New(Py_State *S, ssize_t initial_size)
{
    PyObject *tup = PyTuple_New(initial_size);
    if (tup == NULL) {
        PyErr_Clear();   
    }
    return (CystckTupleBuilder)tup;
}

void
CystckTupleBuilder_Set(Py_State *S, CystckTupleBuilder builder,
                     ssize_t index, Cystck_Object _item)
{
    PyObject *tup = (PyObject *)builder;
    if (tup != NULL) {
        PyObject *item = Cystck2py(_item);
        assert(index >= 0 && index < PyTuple_GET_SIZE(tup));
        assert(PyTuple_GET_ITEM(tup, index) == NULL);
        Py_INCREF(item);
        PyTuple_SET_ITEM(tup, index, item);
    }
}

Cystck_Object
CystckTupleBuilder_Build(Py_State *S, CystckTupleBuilder builder)
{
    PyObject *tup = (PyObject *)builder;
    if (tup == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    builder= 0;
    return Py2Cystck(tup);
}

void
CystckTupleBuilder_Cancel(Py_State *S, CystckTupleBuilder builder)
{
    PyObject *tup = (PyObject *)builder;
    if (tup == NULL) {
        return;
    }
    builder = 0;
    Py_XDECREF(tup);
}

Cystck_Object CystckErr_SetObject(Py_State *s, Cystck_Object type, Cystck_Object value)
{
    PyErr_SetObject(Cystck2py(type), Cystck2py(value));
    return -1;
}


Cystck_Object
CystckBytes_FromStringAndSize(Py_State *S, const char *v, ssize_t len)
{
    if (v == NULL) {
        // The CPython API allows passing a null pointer to
        // PyBytes_FromStringAndSize and returns uninitialized memory of the
        // requested size which can then be initialized after the call.        
        // after the call and so we raise an error instead.
        CystckErr_SetString(S, S->Cystck_ValueError,
                         "NULL char * passed to CystckBytes_FromStringAndSize");
        return 0;
    }
    return Py2Cystck(PyBytes_FromStringAndSize(v, len));
}


void*
Cystck_AsStruct(Py_State *S, Cystck_Object obj)
{
    return _Cystck_PyObject_Payload(Cystck2py(obj));
}
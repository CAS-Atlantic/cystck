#include "Cystck_method.h"
#include "Cystck_module.h"
#include "../include/Cystck.h"
const char *CystckUnicode_AsUTF8AndSize(Py_State *S, Cystck_Object o, Cystck_ssize_t *size)
{
    return PyUnicode_AsUTF8AndSize(Cystck2py(o), size);
}

const char *CystckUnicode_AsUTF8(Py_State *S, Cystck_Object o)
{
    return PyUnicode_AsUTF8(Cystck2py(o));
}

int Cystck_SetItem(Py_State *S, Cystck_Object o, Cystck_Object key, Cystck_Object value)
{
    return PyObject_SetItem(Cystck2py(o), Cystck2py(key), Cystck2py(value));
}

int
Cystck_SetItem_i(Py_State *S, Cystck_Object obj, Cystck_ssize_t idx, Cystck_Object value) {
    PyObject* key = PyLong_FromSsize_t(idx);
    if (key == NULL)
        return -1;
    int result = PyObject_SetItem(Cystck2py(obj), key, Cystck2py(value));
    Py_DECREF(key);
    return result;
}

 int
Cystck_SetItem_s(Py_State *S, Cystck_Object obj, const char *key, Cystck_Object value) {
    PyObject* key_o = PyUnicode_FromString(key);
    if (key_o == NULL)
        return -1;
    int result = PyObject_SetItem(Cystck2py(obj), key_o, Cystck2py(value));
    Py_DECREF(key_o);
    return result;
}

int Cystck_HasAttrString(Py_State *S, Cystck_Object O, const char *name)
{
    return PyObject_HasAttrString(Cystck2py(O), name);
}
Cystck_Object CystckTuple_FromArray(Py_State *S, Cystck_Object items[], Cystck_ssize_t n)
{
    PyObject *res = PyTuple_New(n);
    if (!res)
        return 0;
    for(Cystck_ssize_t i=0; i<n; i++) {
        PyObject *item = Cystck2py(items[i]);
        Py_INCREF(item);
        PyTuple_SET_ITEM(res, i, item);
    }
    return Py2Cystck(res);
}


Cystck_Object Cystck_Tuple_Pack(Py_State *S, Cystck_ssize_t n, ...) {
    va_list vargs;
    Cystck_ssize_t i;

    if (n == 0) {
        return CystckTuple_FromArray(S, (Cystck_Object*)NULL, n);
    }
    Cystck_Object *array = (Cystck_Object *)alloca(n * sizeof(Cystck_Object));
    va_start(vargs, n);
    if (array == NULL) {
        va_end(vargs);
        return 0;
    }
    for (i = 0; i < n; i++) {
        array[i] = va_arg(vargs, Cystck_Object);
    }
    va_end(vargs);
    return CystckTuple_FromArray(S, array, n);
}

Cystck_Object Cystck_Call_Object(Py_State *S,Cystck_Object callable, Cystck_Object Args)
{
    if (Args ==0) return Py2Cystck(PyObject_CallObject (Cystck2py(callable),NULL));
    else return Py2Cystck(PyObject_CallObject (Cystck2py(callable), Cystck2py(Args)));
}

Cystck_Object
Cystck_CallTupleDict(Py_State *S,  Cystck_Object callable, Cystck_Object args, Cystck_Object kw)
{
    PyObject *obj;
    if (!Cystck_IsNULL(args) && !CystckTuple_Check(S, args)) {
       CystckErr_SetString(S, S->Cystck_TypeError,
           "Cystck_CallTupleDict requires args to be a tuple or null object");
       return 0;
    }
    if (!Cystck_IsNULL(kw) && !CystckDict_Check(S, kw)) {
       CystckErr_SetString(S, S->Cystck_TypeError,
           "Cystck_CallTupleDict requires kw to be a dict or null object");
       return 0;
    }
    if (Cystck_IsNULL(kw)) {
        obj = PyObject_CallObject(Cystck2py(callable), Cystck2py(args));
    }
    else if (!Cystck_IsNULL(args)){
        obj = PyObject_Call(Cystck2py(callable), Cystck2py(args), Cystck2py(kw));
    }
    else {
        // args is null, but kw is not, so we need to create an empty args tuple
        // for CPython's PyObject_Call
        Cystck_Object *items = NULL;
        Cystck_Object empty_tuple = CystckTuple_FromArray(S, items, 0);
        obj = PyObject_Call(Cystck2py(callable), Cystck2py(empty_tuple), Cystck2py(kw));
        Cystck_DECREF(S, empty_tuple);
    }
    return Py2Cystck(obj);
}
 
Cystck_Object Cystck_Call(Py_State *S,Cystck_Object callable, Cystck_Object Args, Cystck_Object Kwargs)
{
    if (Kwargs ==0) return Py2Cystck(PyObject_Call (Cystck2py(callable), Cystck2py(Args), NULL));
    else return Py2Cystck(PyObject_Call (Cystck2py(callable), Cystck2py(Args), Cystck2py(Kwargs)));
}



#ifndef CYSTCKPORT_H
#define CYSTCKPORT_H

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"
#define CystckAPI_FUNC   __attribute__((unused)) static inline
typedef ssize_t         Cystck_ssize_t;
Cystck_Object Cystck_BuildValue(Py_State *S, const char *fmt, ...);
const char *CystckUnicode_AsUTF8AndSize(Py_State *S, Cystck_Object o, Cystck_ssize_t *size);
static inline Cystck_Object Cystck_FromVoid(void *p) { return (Cystck_Object )p; } // cast Cystck_object to a void pointer
static inline void* Cystck_AsVoidP(Cystck_Object o) { return (void*)o; } //// cast a void pointer to Cystck_object
int Cystck_SetItem_i(Py_State *S, Cystck_Object obj, Cystck_ssize_t idx, Cystck_Object value);
int Cystck_SetItem_s(Py_State *S, Cystck_Object obj, const char *key, Cystck_Object value);
CystckAPI_FUNC void *CystckLong_AsVoidPtr(Py_State *S, Cystck_Object obj)
{
    return PyLong_AsVoidPtr(Cystck2py(obj));
}
CystckAPI_FUNC Cystck_Object Cystck_Dup(Py_State *S, Cystck_Object object)
{
    Py_XINCREF(Cystck2py(object));
    return object;
}

CystckAPI_FUNC void  Cystck_DECREF(Py_State *S, Cystck_Object obj)
{
    Py_XDECREF(Cystck2py(obj));
}
CystckAPI_FUNC Cystck_Object CystckLong_FromVoidPtr(Py_State *S, void *obj)
{
    return Py2Cystck(PyLong_FromVoidPtr(obj));
}
CystckAPI_FUNC Cystck_Object CystckBytes_FromString(Py_State *S, const char *v)
{
    return Py2Cystck(PyBytes_FromString(v));
}

CystckAPI_FUNC Cystck_Object CystckLong_FromUnsignedLongLong(Py_State *S, unsigned long long v)
{
    return Py2Cystck(PyLong_FromUnsignedLongLong(v));
}

CystckAPI_FUNC PyObject * Cystck_AsPyObject(Py_State *S, Cystck_Object obj)
{
    PyObject *result = Cystck2py(obj);
    Py_XINCREF(result);
    return result;
}

CystckAPI_FUNC  int CystckTypeCheck(Py_State *S, Cystck_Object cy_obj, Cystck_Object cy_type)
{
    PyObject *type= Cystck2py(cy_type);
    assert(type != NULL);
    if (!PyType_Check(type)) {
        Py_FatalError("CystckTypeCheck arg 2 must be a type");
    }
    return PyObject_TypeCheck(Cystck2py(cy_obj), (PyTypeObject*)type);
}
CystckAPI_FUNC Cystck_Object CystckUnicode_EncodeFSDefault(Py_State *S, Cystck_Object obj)
{
    return Py2Cystck(PyUnicode_EncodeFSDefault(Cystck2py(obj)));
}
CystckAPI_FUNC Cystck_Object Cystck_FromPyObject(Py_State *S, PyObject *obj)
{
    Py_XINCREF(obj);
    return Py2Cystck(obj);
}
CystckAPI_FUNC Cystck_Object CystckField_Load(Py_State *S, Cystck_Object source_obj, CystckField source_field)
{
    PyObject *obj = CystckField2py(source_field);
    Py_XINCREF(obj);
    Cystck_pushobject(S, Py2Cystck(obj));
    return Py2Cystck(obj);
}
CystckAPI_FUNC void CystckField_Store(Py_State *S, CystckField target_obj,
                                CystckField *target_field, Cystck_Object o)
{
    PyObject *obj = Cystck2py(o);
    PyObject *target_py_obj = CystckField2py(*target_field);
    Py_XINCREF(obj);
    *target_field = Py2CystckField(obj);
    Py_XDECREF(target_py_obj);
}
CystckAPI_FUNC Cystck_Object CystckUnicode_AsASCIIString(Py_State *S, Cystck_Object h)
{
    return Py2Cystck(PyUnicode_AsASCIIString(Cystck2py(h)));
}
CystckAPI_FUNC double CystckFloat_AsDouble(Py_State *S, Cystck_Object O)
{
    return PyFloat_AsDouble(Cystck2py(O));
}
CystckAPI_FUNC char *CystckBytes_AsString(Py_State *S, Cystck_Object h)
{
    return PyBytes_AsString(Cystck2py(h));
}
CystckAPI_FUNC int Cystck_HasAttr_s(Py_State *S, Cystck_Object obj, const char *name)
{
    return PyObject_HasAttrString(Cystck2py(obj), name);
}
CystckAPI_FUNC Cystck_Object Cystck_GetAttr(Py_State *S, Cystck_Object obj, Cystck_Object name)
{
    return Py2Cystck(PyObject_GetAttr(Cystck2py(obj), Cystck2py(name)));
}

CystckAPI_FUNC Cystck_Object Cystck_GetAttr_s(Py_State *S, Cystck_Object obj, const char *name)
{
    return Py2Cystck(PyObject_GetAttrString(Cystck2py(obj), name));
}
CystckAPI_FUNC Cystck_ssize_t CystckBytes_Size(Py_State *S, Cystck_Object obj)
{
    return PyBytes_Size(Cystck2py(obj));
}
CystckAPI_FUNC void Cystck_CLEAR(Py_State *S, Cystck_Object _data)
{
    PyObject *data = Cystck2py(_data);
    Py_CLEAR(data);
}
CystckAPI_FUNC void CystckErr_Clear(Py_State *S)
{
    PyErr_Clear();
}

CystckAPI_FUNC uint32_t CystckUnicode_ReadChar(Py_State *S, Cystck_Object obj, Cystck_ssize_t index)
{
    return PyUnicode_ReadChar(Cystck2py(obj), index);
}


CystckAPI_FUNC Cystck_ssize_t Cystck_Unicode_GET_LENGTH(Py_State *S, Cystck_Object _data)
{
    return PyUnicode_GET_LENGTH( Cystck2py(_data) );
}
CystckAPI_FUNC Cystck_Object CystckUnicode_DecodeLatin1(Py_State *S, const char *s, Cystck_ssize_t size, const char *errors)
{
    return Py2Cystck(PyUnicode_DecodeLatin1(s, size, errors));
}
CystckAPI_FUNC void CystckErr_WriteUnraisable(Py_State *S, Cystck_Object obj)
{
    PyErr_WriteUnraisable(Cystck2py(obj));
}
CystckAPI_FUNC int CystckErr_WarnEx(Py_State *S, Cystck_Object category, const char *message, ssize_t stack_level)
{
    return PyErr_WarnEx(Cystck2py(category), message, stack_level);
}
CystckAPI_FUNC int Cystck_SetAttr_s(Py_State *S, Cystck_Object obj, const char *name, Cystck_Object value)
{
    return PyObject_SetAttrString(Cystck2py(obj), name, Cystck2py(value));
}
static inline Cystck_Object _threads2Cystck(PyThreadState *s) {
    return (Cystck_Object)s;
}

static inline PyThreadState* _Cystck2threads(Cystck_Object obj) {
    return (PyThreadState*) obj;
}

CystckAPI_FUNC void Cystck_ReenterPythonExecution(Py_State *S, Cystck_Object state)
{
    PyEval_RestoreThread(_Cystck2threads(state));
}

CystckAPI_FUNC Cystck_Object Cystck_LeavePythonExecution(Py_State *S)
{
    return _threads2Cystck(PyEval_SaveThread());
}


#define Cystck_BEGIN_LEAVE_PYTHON(context) { \
    Cystck_Object  _token;                                    \
    _token = Cystck_LeavePythonExecution(context);

#define Cystck_END_LEAVE_PYTHON(context)   \
    Cystck_ReenterPythonExecution(context, _token); \
    }



#endif /* CYSTCKPORT_H */

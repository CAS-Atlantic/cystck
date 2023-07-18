#include "Cystck_method.h"
#include "structmember.h"


#define Cystck_LT    0
#define Cystck_LE    1
#define Cystck_EQ    2
#define Cystck_NE    3
#define Cystck_GT    4
#define Cystck_GE    5
/* Types */
#define CystckMember_SHORT     0
#define CystckMember_INT       1
#define CystckMember_LONG      2
#define CystckMember_FLOAT     3
#define CystckMember_DOUBLE    4
#define CystckMember_STRING    5
#define CystckMember_OBJECT    6
#define CystckMember_CHAR      7   /* 1-character string */
#define CystckMember_BYTE      8   /* 8-bit signed int */
/* unsigned variants: */
#define CystckMember_UBYTE     9
#define CystckMember_USHORT    10
#define CystckMember_UINT      11
#define CystckMember_ULONG     12
/*Cystck*/
typedef struct{
    const char* name;
    int basicsize;
    int itemsize;
    unsigned int flags;
    void *legacy_slots;
    CystckDef **m_methods; // This additional field will help me in the conversion to the correct definition of the PyCFunction.
} Cystck_Type_Spec;

#define TypeObject_AsCystck(type, o) (type*) Cystck2py(o)
#define CystckType_HELPERS(TYPE)        \
static inline TYPE *                    \
TYPE##_AsStruct(Py_State *S, Cystck_Object obj)       \
{                                        \
    return (TYPE*) Cystck2py(obj);       \
}

#define Cystck_FromTypeObject(o) Py2Cystck( (PyObject *)o)
//method for conversion.
Cystck_Object Cystck_Type_FromSpec(Py_State *S, Cystck_Type_Slot *SLOT, Cystck_Type_Spec *spec);
Cystck_Object CystckType_GenericNew (Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kw);
int CystckHelpers_AddType(Py_State *S, Cystck_Object obj, const char *name, Cystck_Type_Spec *spec);

Cystck_Object CystckGlobal_Load(Py_State *S, CystckGlobal global);

typedef struct {
    PyObject_HEAD
    union {
        unsigned char payload[1];
        // these fields are never accessed: they are present just to ensure
        // the correct alignment of payload
        unsigned short _m_short;
        unsigned int _m_int;
        unsigned long _m_long;
        unsigned long long _m_longlong;
        float _m_float;
        double _m_double;
        long double _m_longdouble;
        void *_m_pointer;
    };
} _Cystck_FullyAlignedSpaceForPyObject_HEAD;

#define _Cystck_PyObject_HEAD_SIZE (offsetof(_Cystck_FullyAlignedSpaceForPyObject_HEAD, payload))

// Return a pointer to the area of memory AFTER the PyObject_HEAD
static inline void *_Cystck_PyObject_Payload(PyObject *obj)
{
    return (void *) ((char *) obj + _Cystck_PyObject_HEAD_SIZE);
}
void *Cystck_AsStruct(Py_State *S, Cystck_Object obj);
# ifndef Cystck_method_h
# define Cystck_method_h
#include <Python.h>

/* Flag passed to newmethodobject
The individual flags indicate either a calling convention or a binding convention.

 */

typedef unsigned long int Cystck_Object;
typedef unsigned long int CystckGlobal;
typedef unsigned long int CystckField;
typedef unsigned long int CystckListBuilder;
typedef unsigned long int CystckTupleBuilder;
#define Cystck_METH_VARARGS     0x0001
#define Cystck_METH_KEYWORDS    0x0002
#define Cystck_METH_NOARGS      0x0004
#define Cystck_METH_O           0x0008
typedef struct Py_State Py_State;
Cystck_Object CystckTuple_FromArray(Py_State *S, Cystck_Object items[], ssize_t n);
typedef struct {
    PyCFunction Cystck_CpyMeth;         // Used by CPython to call impl
} Con_Meth;
typedef Cystck_Object(*Py_CFunction)(Py_State *,Cystck_Object);
typedef Cystck_Object(*_Py_CFunction)(Py_State *,Cystck_Object,Cystck_Object);
typedef Cystck_Object(*O_Py_CFunction)(Py_State *);
struct CyMethodDef {
    const char  *ml_name;   /* The name of the built-in function/method */
    Py_CFunction ml_meth;    /* The C function that implements it */
    int         ml_flags;   /* Combination of Cystck_xxx flags, which mostlydescribe the args expected by the C func */
    const char  *ml_doc;    /* The __doc__ attribute, or NULL */
    PyCFunction Cystck_CpyMeth;
};
typedef struct CyMethodDef CyMethodDef;

typedef struct CystckMemberDef {
    const char *name;
    int type;
    ssize_t offset;
    int flags;
    const char *doc;
}CystckMemberDef;

typedef struct CystckGetSetDef {
    const char *name;
    getter get;
    setter set;
    const char *doc;
    void *closure;
} CystckGetSetDef;

typedef struct{
    int slot;    /* slot id*/
    void *pfunc; 
} Cystck_Type_Slot;


typedef enum {
    CystckDef_Kind_Slot =1,
    CystckDef_Kind_Meth =2,
    CystckDef_Kind_Member =3,
    CystckDef_Kind_GetSet =4,
} CystckDef_Kind;
typedef struct {
    CystckDef_Kind kind;
    union{
        Cystck_Type_Slot slot;
        CyMethodDef meth;
        CystckMemberDef member;
        CystckGetSetDef getset;
    };
} CystckDef;

/* buffer interface */
typedef struct{
    void *buf;
    Cystck_Object obj;        /* owned reference */
    ssize_t len;
    ssize_t itemsize;  /* This is ssize_t so it can be
                             pointed to by strides in simple case.*/
    int readonly;
    int ndim;
    char *format;
    ssize_t *shape;
    ssize_t *strides;
    ssize_t *suboffsets;
    void *internal;
} Cystck_buffer;

typedef int (*Cystck_getbufferproc)(Py_State *, Cystck_Object, Cystck_buffer *, int);

#define CystckDef_SLOT(SYM, IMPL, SLOT)        \
    SLOT##_SLOT(SYM##_cpy, IMPL)                \
    CystckDef SYM ={                            \
        .kind = CystckDef_Kind_Slot,            \
        .slot ={                                \
            .slot = SLOT,                       \
            .pfunc= (void*) SYM##_cpy           \
        }                                       \
    };
#define CystckDef_METH(SYM, NAME, IMPL, SIG, DOC)    \
    SIG##_2PY(SYM##_cpy, IMPL)        \
    CystckDef SYM ={                                  \
        .kind = CystckDef_Kind_Meth,                   \
        .meth ={                                      \
            .ml_name = NAME,                          \
            .ml_meth= NULL,                           \
            .ml_flags =SIG,                            \
            .ml_doc = DOC,                              \
            .Cystck_CpyMeth= (PyCFunction)SYM##_cpy,    \
        }                                               \
    };

#define CystckDef_MEMBER(SYM, NAME, TYPE, OFFSET, FLAG, DOC)      \
    CystckDef SYM ={                                                \
        .kind = CystckDef_Kind_Member,                              \
        .member = {                                                 \
            .name = NAME,                                           \
            .type = TYPE,                                           \
            .offset =  OFFSET,                                      \
            .flags = FLAG,                                          \
            .doc = DOC                                              \
        }                                                           \
    };

#define CystckDef_GET( SYM, NAME, GetIMPL, DOC, CLOSURE)    \
    Getter2Py(SYM##_cpy, GetIMPL)        \
    CystckDef SYM ={                                  \
        .kind = CystckDef_Kind_GetSet,                   \
        .getset ={                                      \
            .name = NAME,                          \
            .get= SYM##_cpy,                           \
            .set =0,                            \
            .doc = DOC,                              \
            .closure=  CLOSURE,    \
        }                                               \
    };

#define CystckDef_SET( SYM, NAME, GetIMPL, DOC, CLOSURE)    \
    Setter2Py(SYM##_cpy, IMPL)        \
    CystckDef SYM ={                                  \
        .kind = CystckDef_Kind_GetSet,                   \
        .getset ={                                      \
            .name = NAME,                          \
            .get= NULL,                           \
            .set =SYM##_cpy,                            \
            .doc = DOC,                              \
            .closure=  CLOSURE,    \
        }                                               \
    };

typedef Cystck_Object (*Cystck_TP_New)(Py_State *, Cystck_Object, Cystck_Object , Cystck_Object);

#define Cystck_tp_NEW_SLOT(SYM, IMPL)                                        \
    PyObject*                                                       \
    SYM(PyObject *self, PyObject *args, PyObject*Kwargs)             \
    {                                                               \
        Cystck_Object Args =Py2Cystck(args);                        \
        Cystck_Object KWARGS =Py2Cystck(Kwargs);                     \
        /*getArgs(args);  */                                        \
        _Py_CFunction func = (_Py_CFunction)IMPL;                     \
        Py_State *S = Get_State();                                  \
        S->self= Py2Cystck(self);                                          \
        Cystck_Object indx =func(S,Args, KWARGS);                    \
        PyObject *result = GetResults(S,indx);                      \
        return result;                                                \
    }

#define Cystck_tp_new_SLOT(SYM,IMPL)                               \
    static PyObject *                                               \
    SYM(PyTypeObject *type, PyObject *args, PyObject *kwargs)   \
    {                                                       \
        Cystck_Object Args =Py2Cystck(args);                \
        Cystck_Object KWARGS =Py2Cystck(kwargs);            \
        Cystck_Object _type = Py2Cystck((PyObject *)type);              \
        Cystck_TP_New func = (Cystck_TP_New)IMPL;       \
        Py_State *S = Get_State();                          \
         Cystck_Object indx = func(S, _type, Args, KWARGS);                        \
        PyObject *result = GetResults(S,indx);           \
        return result;                        \
    }

typedef int (*Cystck_initproc)(Py_State *, Cystck_Object , Cystck_Object);
#define Cystck_tp_init_SLOT(SYM,IMPL)                               \
    static int                                               \
    SYM(PyObject *self, PyObject *args, PyObject *kwargs)   \
    {                                                       \
        Cystck_Object Args =Py2Cystck(args);                \
        Cystck_Object KWARGS =Py2Cystck(kwargs);            \
        Cystck_initproc func = (Cystck_initproc)IMPL;       \
        Py_State *S = Get_State();                          \
        S->self=Py2Cystck(self);                                       \
        return func(S, Args, KWARGS);                        \
    }

typedef Cystck_Object (*Cystck_binaryfunc)(Py_State *, Cystck_Object, Cystck_Object);
//typedef PyObject *(*Cystck_binaryfunc)(Py_State *, Cystck_Object, Cystck_Object);

#define Cystck_nb_multiply_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o1, PyObject *o2)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o1);                 \
        Cystck_Object Args2 =Py2Cystck(o2);                 \
        Cystck_binaryfunc func = (Cystck_binaryfunc)IMPL;   \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1,Args2);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                           \
    }

#define Cystck_nb_add_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o1, PyObject *o2)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o1);                 \
        Cystck_Object Args2 =Py2Cystck(o2);                 \
        Cystck_binaryfunc func = (Cystck_binaryfunc)IMPL;   \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1,Args2);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                      \
    }

#define Cystck_nb_subtract_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o1, PyObject *o2)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o1);                 \
        Cystck_Object Args2 =Py2Cystck(o2);                 \
        Cystck_binaryfunc func = (Cystck_binaryfunc)IMPL;   \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1,Args2);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                      \
    }


#define Cystck_nb_true_divide_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o1, PyObject *o2)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o1);                 \
        Cystck_Object Args2 =Py2Cystck(o2);                 \
        Cystck_binaryfunc func = (Cystck_binaryfunc)IMPL;   \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1,Args2);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                       \
    }

#define Cystck_nb_or_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o1, PyObject *o2)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o1);                 \
        Cystck_Object Args2 =Py2Cystck(o2);                 \
        Cystck_binaryfunc func = (Cystck_binaryfunc)IMPL;   \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1,Args2);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                       \
    }

typedef ssize_t (*Cystck_lenfunc)(Py_State *, PyObject* );

#define Cystck_sq_length_SLOT(SYM, IMPL)                      \
    ssize_t                                              \
    SYM (PyObject  *o)                                      \
    {                                                       \
        Cystck_lenfunc func = (Cystck_lenfunc)IMPL;         \
        Py_State *S = Get_State();                          \
        return func(S,o);                                    \
    }


//typedef PyObject * (*unaryfunc)(PyObject *);
typedef Cystck_Object (*Cystck_unaryfunc)(Py_State*, Cystck_Object);
#define Cystck_nb_negative_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o)                                      \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o); \
        Cystck_unaryfunc func = (Cystck_unaryfunc)IMPL;         \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                    \
    }

typedef Cystck_Object(*Cystck_ssizeargfunc)(Py_State *, PyObject*, ssize_t);

#define Cystck_sq_item_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o1, ssize_t Size)                       \
    {                                                       \
        Cystck_ssizeargfunc func = (Cystck_ssizeargfunc)IMPL; \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, o1,Size);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                      \
    }

typedef int(*Cystck_ssizeobjargproc)(Py_State *, PyObject *, ssize_t, Cystck_Object);

#define Cystck_sq_ass_item_SLOT(SYM, IMPL)                      \
    int                                              \
    SYM (PyObject  *o, ssize_t idex, PyObject *item)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(item); \
        Cystck_ssizeobjargproc func = (Cystck_ssizeobjargproc)IMPL;                          \
        Py_State *S = Get_State();            \
        return func(S, o, idex, Args1);                \
    }

typedef void (*Cystck_destructor)(Py_State *);


#define Cystck_tp_dealloc_SLOT(SYM, IMPL)                      \
    void                                              \
    SYM (PyObject  *o)                       \
    {                                                       \
        Cystck_destructor func = (Cystck_destructor)IMPL; \
        Py_State *S = Get_State();                          \
        S->self =Py2Cystck(o);            \
        func(S);                \
        Py_TYPE(o)->tp_free(o);                             \
    }

// typedef PyObject *(*reprfunc)(PyObject *);
typedef Cystck_Object(*Cystck_reprfunc)(Py_State*, Cystck_Object);
#define Cystck_tp_repr_SLOT(SYM, IMPL)                      \
    PyObject *                                              \
    SYM (PyObject  *o)                                      \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o); \
        Cystck_reprfunc func = (Cystck_reprfunc)IMPL;         \
        Py_State *S = Get_State();                          \
        Cystck_Object indx =func(S, Args1);            \
        PyObject *result = GetResults(S,indx);               \
        return result;                                      \
    }

//typedef PyObject *(*richcmpfunc) (PyObject *, PyObject *, int);

typedef Cystck_Object (*Cystck_richcmpfunc) (Py_State *, Cystck_Object, Cystck_Object, int);
#define Cystck_tp_richcompare_SLOT(SYM, IMPL)                      \
    PyObject*                                              \
    SYM (PyObject  *o1, PyObject *o2, int i)                       \
    {                                                       \
        Cystck_Object Args1 =Py2Cystck(o1); \
        Cystck_Object Args2 =Py2Cystck(o2); \
        Cystck_richcmpfunc func = (Cystck_richcmpfunc)IMPL;                          \
        Py_State *S = Get_State();            \
        Cystck_Object indx =func(S, Args1,Args2, i);           \
        PyObject *result = GetResults(S,indx);               \
        return result;                \
    }

static inline PyObject* Cystck2py(Cystck_Object O) {
    return (PyObject*) O;
}
static inline PyObject * CystckField2py(CystckField fobj)
{
    return (PyObject*) fobj;
}

typedef int (*Cystck_visitproc)(CystckField *, void *);
typedef int (*Cystck_traverseproc)(void*, Cystck_visitproc, void *);
#define Cystck_tp_traverse_SLOT(SYM, IMPL)                      \
    int                                              \
    SYM (PyObject * obj, visitproc vist, void *p)                       \
    {                                                       \
        void* Args1 =(void*)obj; \
        Cystck_traverseproc func = (Cystck_traverseproc)IMPL;                          \
        int indx =func(Args1,(Cystck_visitproc)vist,p);           \
        return indx;                \
    }

#define Cystck_bf_getbuffer_SLOT(SYM, IMPL)                                             \
    static int                                                                          \
    SYM (PyObject *arg0, Py_buffer *arg1, int arg2)                                     \
    {                                                                                   \
        Cystck_getbufferproc func = (Cystck_getbufferproc)IMPL;                         \
        int indx =func(Get_State(), Py2Cystck(arg0), (Cystck_buffer*)arg1, arg2);       \
        return indx;                                                                     \
    }

typedef Cystck_Object(*Cystck_getter)(Py_State*, void *);
typedef int (*Cystck_setter)(Py_State*, Cystck_Object, Cystck_Object , void *);

#define Getter2Py(SYM, IMPL)                      \
    PyObject*                                              \
    SYM (PyObject  *o1, void *v)                       \
    {                                                       \
        Cystck_getter func = (Cystck_getter)IMPL;                          \
        Py_State *S = Get_State();            \
        S->self =Py2Cystck(o1);            \
        Cystck_Object indx =func(S, v);           \
        PyObject *result = GetResults(S,indx);               \
        return result;                \
    }


static inline PyObject * CystckGlobal2py(CystckGlobal gobj)
{
    Cystck_Object obj = gobj;
    return Cystck2py(obj);
}


static inline Cystck_Object Py2Cystck(PyObject *o) {
    return (Cystck_Object)o;
}
static inline CystckField Py2CystckField( PyObject *obj)
{
    Cystck_Object fobj = Py2Cystck(obj);
    return (CystckField) fobj;
}
// static inline int Cystck_IsNULL(Cystck_Object o) {
//     if (Cystck2py(o)==NULL || (int)o<=0) return 1;
//     else return 0;
// }

#define Cystck_IsNULL(o) (o==0)
#define CystckField_IsNULL(o) (o==0)

// static inline int CystckField_IsNULL(CystckField o) {
//     if (CystckField2py(o)==NULL || (int)o<=0) return 1;
//     else return 0;
// }

Cystck_Object Cystck_Call_Object(Py_State *S,Cystck_Object callable, Cystck_Object Args);
Cystck_Object Cystck_CallTupleDict(Py_State *S,  Cystck_Object callable, Cystck_Object args, Cystck_Object kw);
int Cystck_IsTrue(Py_State *S, Cystck_Object o);
void Cystck_Free(Py_State *S,Cystck_Object O);
int Cystck_SetItem(Py_State *S, Cystck_Object o, Cystck_Object key, Cystck_Object value);
Cystck_Object Cystck_Import_ImportModule(const char *name);
const char *CystckUnicode_AsUTF8(Py_State *S, Cystck_Object o);
int Cystck_Err_Occurred(Py_State *S);
void CystckErr_SetString(Py_State *S, Cystck_Object o, const char *message);
#define Cystk_Object PyObject
Cystck_Object CystckUnicode_FromString(Py_State *S, const char *utf8);
Cystck_Object Cystck_Call(Py_State *S,Cystck_Object callable, Cystck_Object Args, Cystck_Object Kwargs);
#define  Cystck_NULL  0

PyObject *getResults(Cystck_Object indx);

PyObject *GetResults(Py_State *S, Cystck_Object indx);
#define Cystck_METH_VARARGS_2PY(SYM, IMPL)                                         \
    PyObject*                                                        \
    SYM(PyObject *self, PyObject *args)                             \
    {                                                                \
        Cystck_Object Args =Py2Cystck(args);                        \
        Py_State *S = Get_State(); Cystck_Object indx;               \
        S->self= Py2Cystck(self);                                          \
        Py_CFunction func = (Py_CFunction)IMPL;                      \
        indx =func(S,Args);                                       \
        PyObject *result = GetResults(S,indx);                       \
        return result;                                                \
    } 

#define Cystck_METH_O_2PY(SYM, IMPL)                                         \
    PyObject*                                                        \
    SYM(PyObject *self, PyObject *args)                             \
    {                                                                \
        Cystck_Object Args =Py2Cystck(args);                        \
        Py_State *S = Get_State(); Cystck_Object indx;               \
        S->self= Py2Cystck(self);                                          \
        Py_CFunction func = (Py_CFunction)IMPL;                      \
        indx =func(S,Args);                                       \
        PyObject *result = GetResults(S,indx);                       \
        return result;                                                \
    }

#define Cystck_METH_NOARGS_2PY(SYM, IMPL)                                         \
    PyObject*                                                        \
    SYM(PyObject *self, PyObject *args)                             \
    {                                                                \
        Py_State *S = Get_State(); Cystck_Object indx;               \
        S->self= Py2Cystck(self);                                          \
        O_Py_CFunction func = (O_Py_CFunction)IMPL;                      \
        indx =func(S);                                       \
        PyObject *result = GetResults(S,indx);                       \
        return result;                                                \
    } 

#define Cystck_METH_KEYWORDS_2PY(SYM, IMPL)                                        \
    PyObject*                                                       \
    SYM(PyObject *self, PyObject *args, PyObject*Kwargs)             \
    {                                                               \
        Cystck_Object Args =Py2Cystck(args);                        \
        Cystck_Object KWARGS =Py2Cystck(Kwargs);                     \
        /*getArgs(args);  */                                        \
        _Py_CFunction func = (_Py_CFunction)IMPL;                     \
        Py_State *S = Get_State();                                  \
        S->self= Py2Cystck(self);                                          \
        Cystck_Object indx =func(S,Args, KWARGS);                    \
        PyObject *result = GetResults(S,indx);                      \
        return result;                                                \
    }                      

# endif /*Cystck_method_h*/
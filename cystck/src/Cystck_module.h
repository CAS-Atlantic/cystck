# ifndef Module_h
# define Module_h

#ifdef __cplusplus
extern "C" {
#endif

#include "Cystck_method.h"
int Cystck_HasAttrString(Py_State *S, Cystck_Object O, const char *name);
Cystck_Object Cystck_GetAttr_String(Py_State *S, Cystck_Object obj, const char *name);
int Cystck_Callable_Check(Py_State *S, Cystck_Object O);
long Cystck_Long_AsLong(Py_State *S, Cystck_Object O);
double CystckLong_AsDouble(Py_State  *s, Cystck_Object obj);
#define Cystck_HEAD                   Cystk_Object ob_base;

#ifdef __cplusplus
extern "C" {
#endif
Cystck_Object _Cystck_New(Py_State *S, Cystck_Object type, void **data);
Cystck_Object CystckErr_NewException(Py_State *s, const char *name, Cystck_Object base, Cystck_Object dict);
#define Cystck_New(state, _type, data) (_Cystck_New((state), (_type), ((void**)data) ))
#ifdef __cplusplus
}
#endif
Cystck_Object CystckBytes_FromStringAndSize(Py_State *S, const char *v, ssize_t len);
Cystck_Object
Cystck_CallTupleDict(Py_State *S, Cystck_Object callable, Cystck_Object args, Cystck_Object kw);
CystckTupleBuilder CystckTupleBuilder_New(Py_State *S, ssize_t initial_size);
void CystckTupleBuilder_Set(Py_State *S, CystckTupleBuilder builder, ssize_t index, Cystck_Object _item);
Cystck_Object CystckTupleBuilder_Build(Py_State *S, CystckTupleBuilder builder);
void CystckTupleBuilder_Cancel(Py_State *S, CystckTupleBuilder builder);
CystckListBuilder CystckListBuilder_New(Py_State *C, ssize_t initial_size);
void CystckListBuilder_Set(Py_State *S, CystckListBuilder builder, ssize_t index, Cystck_Object _item);
Cystck_Object CystckListBuilder_Build(Py_State *S, CystckListBuilder builder);
void CystckListBuilder_Cancel(Py_State *S, CystckListBuilder builder);
Cystck_Object Cystck_GetItem_i(Py_State *S, Cystck_Object obj, ssize_t idx);
int Cystck_Is(Py_State *S, Cystck_Object _obj, Cystck_Object _other);
char *CystckBytes_AS_STRING(Py_State *S, Cystck_Object O);
void Cystck_Err_Clear(Py_State *S);
void CystckTuple_SET_ITEM(Py_State *S, Cystck_Object tp, ssize_t index, Cystck_Object _item);
ssize_t CystckList_Size(Py_State *S, Cystck_Object o);
int CystckUnicode_Check(Py_State *S, Cystck_Object O);
Cystck_Object CystckList_GET_ITEM(Py_State *S, Cystck_Object data, int index);
int CystckNumber_Check(Py_State *S, Cystck_Object O);
long CystckLong_AsLong(Py_State *S, Cystck_Object obj);
Cystck_Object CystckTuple_GetItem(Py_State *S, Cystck_Object O, int size);
Cystck_Object CystckList_GetItem(Py_State *S, Cystck_Object O, int size);
Cystck_Object CystckIter_Next(Py_State *S, Cystck_Object o);
Cystck_Object Cystck_GetItem(Py_State *S, Cystck_Object o, Cystck_Object key);
Cystck_Object CystckUnicode_AsUTF8String(Py_State *S, Cystck_Object o);
int CystckBytes_Check(Py_State *S, Cystck_Object o);
int CystckModule_AddObject(Py_State *S, Cystck_Object mod, const char * str, Cystck_Object value);
#define CystckNone  Py2Cystck(Py_None)
Cystck_Object CystckDict_New(Py_State *S);
Cystck_Object CystckFloat_FromDouble(Py_State *S, double v);
Cystck_Object CystckLong_FromLong(Py_State *S, long value);
Cystck_Object CystckMapping_Keys(Py_State *S, Cystck_Object o);
Cystck_Object CystckLong_FromUnsignedLong(Py_State *S, unsigned long value);
Cystck_Object CystckLong_FromLongLong(Py_State *S, long long v);
int CystckDict_Check(Py_State *S,Cystck_Object O);
int CystckTuple_Check(Py_State *S, Cystck_Object O);
Cystck_Object CystckUnicode_FromString(Py_State *S, const char *utf8);
Cystck_Object Cystck_Str(Py_State *S, Cystck_Object obj);
int CystckList_Check(Py_State *S, Cystck_Object o);
int CystckList_Sort(Py_State *S, Cystck_Object o);
Cystck_Object CystckDict_GetItem (Py_State *S, Cystck_Object mp, Cystck_Object key);
int CystckDict_SetItem(Py_State *S,Cystck_Object mp, Cystck_Object key, Cystck_Object item);
int CystckList_Append(Py_State *S, Cystck_Object h_list, Cystck_Object h_item);
Cystck_Object Cystck_GetIter(Py_State *S, Cystck_Object O);
Cystck_Object CystckErr_NoMemory(Py_State *S);
int CystckIter_Check(Py_State *S, Cystck_Object O);
int CystckBool_check(Py_State *S, Cystck_Object O);
int CystckLong_check(Py_State *S, Cystck_Object O);
void CystckGlobal_Store(Py_State *S, CystckGlobal *global, Cystck_Object o);
int CystckErr_EXceptionMatches(Py_State*S, Cystck_Object O);
long long CystckLong_AsLongLong(Py_State *S, Cystck_Object  O);
unsigned long long CystckLong_AsUnsignedLongLong(Py_State *S, Cystck_Object O);
int CystckFloat_check(Py_State *S, Cystck_Object O);
int Cystck_IsInstance(Py_State *S, Cystck_Object obj, Cystck_Object typ);

#define CYSTCK_VISIT(cystckfield)                                       \
    do {                                                                \
        if (! Cystck_IsNULL(*cystckfield)) {                              \
            int vret = visit(cystckfield, arg);                            \
            if (vret)                                                   \
                return vret;                                            \
            }                                                           \
    } while (0)
typedef struct CyModuleDef{
  const char* m_name;
  const char* m_doc;
  ssize_t m_size;
  PyMethodDef *legacy_methods;
  CystckDef **m_methods;
  CystckGlobal **globals;

} CyModuleDef;
int Cystck_Print(Py_State *S, Cystck_Object o, FILE *fp, int flags);
Cystck_Object CystckErr_SetObject(Py_State *s, Cystck_Object type, Cystck_Object value);
int Cystck_SetAttrString (Py_State *S, Cystck_Object obj, const char *name, Cystck_Object value);
Cystck_Object CystckModule_Create(Py_State *Py_state,CyModuleDef *ModDef);

#define CyMODINIT_FUNC(modname)                             \
    static Cystck_Object CyInit_##modname (Py_State *Py_state);           \
    PyMODINIT_FUNC                                          \
    PyInit_##modname(void)                                  \
    {                                                       \
        return Cystck2py(CyInit_##modname(Get_State()));                      \
    }                                                       \
    Cystck_Object

#ifdef __cplusplus
}
#endif


# endif /*Module_h*/
#include "../include/Cystck.h"
#include "memory.h"

static Cystck_ssize_t
CystckDef_count(CystckDef *defs[], CystckDef_Kind kind)
{
    Cystck_ssize_t res = 0;
    if (defs == NULL)
        return res;
    for(int i=0; defs[i] != NULL; i++)
        if (defs[i]->kind == kind)
            res++;
    return res;
}

static int
sig2flags(int sig)
{
    switch(sig) {
        case Cystck_METH_VARARGS:  return METH_VARARGS;
        case Cystck_METH_KEYWORDS: return METH_VARARGS | METH_KEYWORDS;
        case Cystck_METH_NOARGS:   return METH_NOARGS;
        case Cystck_METH_O:        return METH_O;
        default:               return -1;
    }
}
      
PyMethodDef *createMethods(CystckDef *methods[], PyMethodDef *legacy_methods)
 {  
    
    Cystck_ssize_t meth_count = CystckDef_count(methods, CystckDef_Kind_Meth);
    Cystck_ssize_t legacy_count = 0;
    if ( legacy_methods !=NULL)
    {
        while( legacy_methods[legacy_count].ml_name !=NULL )
            legacy_count++;
    }
    Cystck_ssize_t total_count = meth_count + legacy_count;
    PyMethodDef *mtd = (PyMethodDef*)malloc((total_count +1)*sizeof(PyMethodDef));
    if (mtd==NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    int dst_idx = 0;
    if (methods !=NULL)
    {
        for(int i=0; methods[i] !=NULL; i++)
        {
            CystckDef *src = methods[i];
            if ( src->kind != CystckDef_Kind_Meth)
                continue;
            PyMethodDef *dst = &mtd[dst_idx++];
            dst->ml_doc  =  src->meth.ml_doc;
            dst->ml_flags= sig2flags(src->meth.ml_flags);
            dst->ml_meth = src->meth.Cystck_CpyMeth;
            dst->ml_name = src->meth.ml_name;
        }
    }
    for ( int i = 0; i<legacy_count; i++ )
        mtd[dst_idx++] = legacy_methods[i];
    mtd[dst_idx++] = (PyMethodDef){NULL, NULL, 0, NULL};
    if (dst_idx != total_count+1)
        Py_FatalError("bogus count in createMethods");
    return mtd;
}

static PyMemberDef *
create_member_defs(CystckDef *Cystckdefs[], PyMemberDef *legacy_members)
{
    Cystck_ssize_t member_count = CystckDef_count(Cystckdefs, CystckDef_Kind_Member);
    Cystck_ssize_t legacy_count = 0;
    if ( legacy_members != NULL){
        while ( legacy_members[legacy_count].name !=NULL )
            legacy_count++;
    }
    Cystck_ssize_t total_count = member_count + legacy_count;
    // allocate&fill the result
    PyMemberDef *result = (PyMemberDef*)PyMem_Calloc(total_count+1, sizeof(PyMemberDef));
    if (result == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    // copy the Cystck members
    int dst_idx = 0;
    if (Cystckdefs != NULL) {
        for(int i=0; Cystckdefs[i] != NULL; i++) {
            CystckDef *src = Cystckdefs[i];
            if (src->kind != CystckDef_Kind_Member)
                continue;
            PyMemberDef *dst = &result[dst_idx++];
            dst->name = src->member.name;
            dst->type = src->member.type;
            dst->offset = src->member.offset;
            dst->doc = src->member.doc;
            dst->flags = src->member.flags;
        }
    }
    for( int i = 0; i<legacy_count; i++)
        result[dst_idx++] = legacy_members[i];
    result[dst_idx++] = (PyMemberDef){NULL};
    if (dst_idx != total_count + 1)
        Py_FatalError("bogus count in create_member_defs");
    return result;
}


static PyGetSetDef *
create_getset_defs(CystckDef *Cystckdefs[], PyGetSetDef *legacy_getsets)
{
    Cystck_ssize_t getset_count = CystckDef_count(Cystckdefs, CystckDef_Kind_GetSet);
    Cystck_ssize_t legacy_count = 0;
    if (legacy_getsets !=NULL){
        while(legacy_getsets[legacy_count].name !=NULL)
            legacy_count++;
    }
    Cystck_ssize_t total_count = getset_count + legacy_count;
    
    PyGetSetDef *result = (PyGetSetDef*)PyMem_Calloc(total_count+1, sizeof(PyGetSetDef));
    if (result == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    // copy the Cystck members
    int dst_idx = 0;
    if (Cystckdefs != NULL) {
        for(int i=0; Cystckdefs[i] != NULL; i++) {
            CystckDef *src = Cystckdefs[i];
            if (src->kind != CystckDef_Kind_GetSet)
                continue;
            PyGetSetDef *dst = &result[dst_idx++];
            dst->name = src->getset.name;
            dst->get = src->getset.get;
            dst->set = src->getset.set;
            dst->doc = src->getset.doc;
            dst->closure = src->getset.closure;
        }
    }
    // copy the legacy members
    for (int i=0; i<legacy_count; i++)
        result[dst_idx++] = legacy_getsets[i];
    result[dst_idx++] = (PyGetSetDef){NULL};
    if (dst_idx != total_count + 1)
        Py_FatalError("bogus count in create_getset_defs");
    return result;
}


static void
legacy_slots_count(PyType_Slot slots[], Cystck_ssize_t *slot_count,
                   PyMethodDef **method_defs, PyMemberDef **member_defs,
                   PyGetSetDef **getset_defs)
{
    *slot_count = 0;
    *method_defs = NULL;
    *member_defs = NULL;
    *getset_defs = NULL;
    if (slots == NULL)
        return;
    for(int i=0; slots[i].slot != 0; i++)
        switch(slots[i].slot) {
        case Py_tp_methods:
            *method_defs = (PyMethodDef *)slots[i].pfunc;
            break;
        case Py_tp_members:
            *member_defs = (PyMemberDef *)slots[i].pfunc;
            break;
        case Py_tp_getset:
            *getset_defs = (PyGetSetDef *)slots[i].pfunc;
            break;
        default:
            (*slot_count)++;
            break;
        }
}

static PyType_Slot * Py_slot_def(Cystck_Type_Spec *spec)
{
    
    Cystck_ssize_t num = CystckDef_count(spec->m_methods, CystckDef_Kind_Slot);
    Cystck_ssize_t legacy_slot_count = 0;
    PyMethodDef *legacy_method_defs = NULL;
    PyMemberDef *legacy_member_defs = NULL;
    PyGetSetDef *legacy_getset_defs = NULL;
    legacy_slots_count((PyType_Slot*)spec->legacy_slots, &legacy_slot_count, &legacy_method_defs, &legacy_member_defs, &legacy_getset_defs);
    num++;
    num++;
    num++;
    Cystck_ssize_t total_count = num + legacy_slot_count;
    PyType_Slot * result = (PyType_Slot*)PyMem_Calloc(total_count+1, sizeof(PyType_Slot));
    if (result == NULL){
        PyErr_NoMemory();
        return NULL;
    }

    int dst_idx =0;
    
    if ( spec->m_methods != NULL )
    {
        for ( int i=0;  spec->m_methods[i] !=NULL; i++)
        {
            CystckDef *src = spec->m_methods[i];
            if (src->kind != CystckDef_Kind_Slot)
                continue;
            
            PyType_Slot *dst = &result[dst_idx++];

            dst->slot = src->slot.slot;
            dst->pfunc =src->slot.pfunc;
        }
    }
    if ( spec->legacy_slots !=NULL ){
        PyType_Slot *legacy_slots = (PyType_Slot *)spec->legacy_slots;
        for(int i =0; legacy_slots[i].slot !=0; i++){
            PyType_Slot *src = &legacy_slots[i];
            if (src->slot == Py_tp_methods || src->slot == Py_tp_members || src->slot == Py_tp_getset)
               {
                continue;
               } 
            PyType_Slot *dst = &result[dst_idx++];
            *dst = *src;
        }
    }

    PyMethodDef *pymethods = createMethods(spec->m_methods, legacy_method_defs);
    if (pymethods != NULL) {
        result[dst_idx++] = (PyType_Slot){Py_tp_methods, pymethods};
    }
    
    PyGetSetDef *pygetsets = create_getset_defs(spec->m_methods, legacy_getset_defs);
    if (pygetsets != NULL) {
        result[dst_idx++] = (PyType_Slot){Py_tp_getset, pygetsets};
    }
    PyMemberDef *pymembers = create_member_defs(spec->m_methods, legacy_member_defs);
    if (pymembers != NULL) {
        result[dst_idx++] = (PyType_Slot){Py_tp_members, pymembers};
    }
    result[dst_idx++] = (PyType_Slot){0, NULL};
    if (dst_idx != total_count + 1)
        Py_FatalError("bogus slot count in create_slot_defs");
    return result;
}
Cystck_Object CystckType_GenericNew (Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kw)
{
    PyObject *tp = Cystck2py(type);
    assert(tp != NULL);
    if (!PyType_Check(tp)) {
        PyErr_SetString(PyExc_TypeError, "Cystck_Type_GenericNew arg 1 must be a type");
        return -1;
    }

    PyObject *res = ((PyTypeObject*) tp)->tp_alloc((PyTypeObject*) tp, 0);
    Cystck_pushobject(S, Py2Cystck(res));
    return 1;
}
Cystck_Object Cystck_Type_FromSpec(Py_State *S, Cystck_Type_Slot *SLOT, Cystck_Type_Spec *spec)
{
    PyType_Spec *Pyspec = (PyType_Spec*)PyMem_Calloc(1, sizeof(PyType_Spec));
    if (Pyspec == NULL) {
        PyErr_NoMemory();
        return 0;
    }

    Pyspec->name = spec->name;
    Pyspec->basicsize = spec->basicsize;
    Pyspec->flags = spec->flags;
    Pyspec->itemsize = spec->itemsize;
    Pyspec->slots = Py_slot_def(spec);  // Py_slot_def() is my function that creats the PyType_slot in their correct format.
    Cystck_Object result = Py2Cystck(PyType_FromSpec(Pyspec));
    Cystck_pushobject(S, result);
    
    return result;// PyType_FromSpec() is CPython function 
}

int
CystckHelpers_AddType(Py_State *S, Cystck_Object obj, const char *name,
                  Cystck_Type_Spec *spec)
{
    Cystck_Object Cy_type = Cystck_Type_FromSpec(S, NULL, spec);
    if (Cystck_IsNULL(Cy_type)) {
        return 0;
    }
    if (Cystck_SetAttr_s(S, obj, name, Cy_type) != 0) {
        Cystck_pop(S, Cy_type);
        return 0;
    }
    Cystck_pop(S, Cy_type);
    return 1;
}


// typedef struct {
//     visitproc cpy_visit;
//     void *cpy_arg;
// } Cystck2cpy_visit_args_t;

// static int Cystck2cpy_visit(CystckField *f, void *v_args)
// {
//     Cystck2cpy_visit_args_t *args = (Cystck2cpy_visit_args_t *)v_args;
//     visitproc cpy_visit = args->cpy_visit;
//     void *cpy_arg = args->cpy_arg;
//     PyObject *cpy_obj = CystckField2py(*f);
//     return cpy_visit(cpy_obj, cpy_arg);
// }

// int call_traverseproc_from_trampoline(Cystck_traverseproc tp_traverse,
//                                                   PyObject *self,
//                                                   visitproc cpy_visit,
//                                                   void *cpy_arg)
// {
//     Cystck2cpy_visit_args_t args = { cpy_visit, cpy_arg };
//     return tp_traverse(self, Cystck2cpy_visit, &args);
// }

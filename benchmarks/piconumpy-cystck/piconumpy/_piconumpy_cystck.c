#include "../../include/Cystck.h"

typedef struct {
  Cystck_HEAD
      /* Type-specific fields go here. */
  double *data;
  int size;
} ArrayObject;

CystckType_HELPERS(ArrayObject)

static void ArrayDealloc(Py_State *S) {
  ArrayObject *self = ArrayObject_AsStruct(S, S->self);
  free(self->data);
}
CystckDef_SLOT(ArrayDealloc_def, ArrayDealloc,Cystck_tp_dealloc);

static int ArrayInit(Py_State *S, Cystck_Object args, Cystck_Object kwds) {
  
  ArrayObject *self = ArrayObject_AsStruct(S, S->self);
  static const char *kwlist[] = {"data", NULL};
  int index;
  Cystck_Object data = 0, item;

  if (!CystckArg_parseTupleAndKeywords(S, args, kwds, "|O", kwlist, &data))
    return -1;

  if (!CystckList_Check(S,data)) {
    CystckErr_SetString(S, S->Cystck_TypeError, "parameter must be a list");
    return -1;
  }

  self->size = (int)Cystck_Length(S, data);

  self->data = (double *)malloc(self->size * sizeof(double));
  if (self->data == NULL) {
    CystckErr_NoMemory(S);
    return -1;
  }

  for (index = 0; index < self->size; index++) {
    item = Cystck_GetItem_i (S,data, index);
    Cystck_pushobject(S, item);
    self->data[index] = CystckFloat_AsDouble(S,item);
    Cystck_pop(S, item);
  }

  return 0;
}
CystckDef_SLOT(ArrayInit_def, ArrayInit, Cystck_tp_init);
CystckDef_MEMBER(Array_size_def, "size",CystckMember_INT, offsetof(ArrayObject, size),0, "size of the array");

static Cystck_Object Array_tolist(Py_State *S)
{
  ArrayObject *self = ArrayObject_AsStruct(S, S->self);
  int index;
  CystckListBuilder builder =  CystckListBuilder_New(S,self->size);
  for (index = 0; index < self->size; index++) {
    Cystck_Object _item = CystckFloat_FromDouble(S,self->data[index]);
    Cystck_pushobject(S, _item);
    CystckListBuilder_Set(S, builder, index, _item);
    Cystck_pop(S, _item);
  }   
  Cystck_pushobject(S,CystckListBuilder_Build(S, builder));
  return 1;
}

CystckDef_METH(Array_tolist_methd, "tolist",Array_tolist, Cystck_METH_NOARGS, "Return the data as a list" );

static Cystck_Object Array_empty(Py_State *S,  int size, ArrayObject **result);

static Cystck_Object ArrayMultiply(Py_State *S, Cystck_Object o1, Cystck_Object o2) {
  int index;
  double number;
  Cystck_Object obj_number = 0;
  ArrayObject *result = NULL, *arr = NULL;
  Cystck_Object _result = 0;
  if (CystckNumber_Check(S,o2)) {
    obj_number = o2;
    arr = ArrayObject_AsStruct(S, o1);
  } else if (CystckNumber_Check(S, o1)) {
    obj_number = o1;
    arr = ArrayObject_AsStruct(S, o2);
  }

  if (CystckNumber_Check(S, o1) | CystckNumber_Check(S, o2)) {
    number = CystckFloat_AsDouble(S, obj_number);
    _result = Array_empty(S, arr->size, &result);
    for (index = 0; index < arr->size; index++) {
      result->data[index] = arr->data[index] * number;
    }
  }
  Cystck_pushobject(S, _result);
  return 1;
};
CystckDef_SLOT(ArrayMultiply_def, ArrayMultiply, Cystck_nb_multiply);

static Cystck_Object ArrayAdd(Py_State *S, Cystck_Object o1, Cystck_Object o2) {
  int index;
  ArrayObject *result = NULL, *a1, *a2;
  Cystck_Object _result = 0;
  a1 = ArrayObject_AsStruct(S, o1);
  a2 = ArrayObject_AsStruct(S, o2);

  if (a1->size != a2->size){
    return -1;
  }
  _result = Array_empty(S, a1->size, &result);
  for (index = 0; index < a1->size; index++) {
    result->data[index] = a1->data[index] + a2->data[index];
  }
  Cystck_pushobject(S, _result);
  return 1;
};

CystckDef_SLOT(ArrayAdd_def, ArrayAdd,Cystck_nb_add);

static Cystck_Object ArrayDivide(Py_State *S, Cystck_Object o1, Cystck_Object o2) {
  int index;
  double number;
  ArrayObject *result = NULL, *a1;
  Cystck_Object _result;

  if (!CystckNumber_Check(S, o2)) {
    return -1;
  }
  a1 = ArrayObject_AsStruct(S, o1);
  number = CystckFloat_AsDouble(S, o2);
  _result = Array_empty(S, a1->size, &result);
  for (index = 0; index < a1->size; index++) {
    result->data[index] = a1->data[index] / number;
  }
  Cystck_pushobject(S, _result);
  return 1;
};
CystckDef_SLOT(ArrayDivide_def, ArrayDivide, Cystck_nb_true_divide);

ssize_t ArrayLength(Py_State *S, ArrayObject *arr) {
  Cystck_ssize_t result = (Cystck_ssize_t)arr->size;
  return result;
};
CystckDef_SLOT(ArrayLength_def, ArrayLength, Cystck_sq_length);

Cystck_Object ArrayItem(Py_State *S, ArrayObject *arr, Py_ssize_t index) {
  if (index < 0 || index >= arr->size) {
      CystckErr_SetString(S, S->Cystck_IndexError, "index out of range");
      return -1;
  }
  Cystck_Pushnumber(S, arr->data[index]);
  return 1;
};
CystckDef_SLOT(ArrayItem_def, ArrayItem,Cystck_sq_item);

int ArraySetitem(Py_State *S, ArrayObject *arr, Py_ssize_t index, Cystck_Object item) {
    if (index < 0 || index >= arr->size) {
        CystckErr_SetString(S, S->Cystck_IndexError, "index out of range");
        return -1;
    }
    double value = CystckFloat_AsDouble(S,item);
    if (Cystck_Err_Occurred(S))
        return -1;
    arr->data[index] = value;
    return 0;
}
CystckDef_SLOT(ArraySetitem_def, ArraySetitem,Cystck_sq_ass_item);

CystckDef_SLOT(Array_new, CystckType_GenericNew,Cystck_tp_new);


static CystckDef *Array_defines[]={
  //slots
  &Array_new,
  &ArrayInit_def,
  &ArrayDealloc_def,
  &ArrayAdd_def,
  &ArrayMultiply_def,
  &ArrayDivide_def,
  &ArrayItem_def,
  &ArraySetitem_def,
  &ArrayLength_def,
  //members
  &Array_size_def,
  //methods
  &Array_tolist_methd,
  NULL
};

static Cystck_Type_Spec Array_type_spec = {
    .name = "_piconumpy_cystck.array",
    .basicsize = sizeof(ArrayObject),
    .itemsize = 0,
    .flags = Cystck_TPFLAGS_DEFAULT,
    .m_methods= Array_defines,
};

Cystck_Object Cystck_ArrayType;

static Cystck_Object  Array_empty(Py_State *S, int size, ArrayObject **result) 
{
  ArrayObject *new_array;
  Cystck_Object Cystck_new_array = Cystck_New(S, Cystck_ArrayType, &new_array);
  new_array->size = size;
  
  new_array->data = (double *)malloc(size * sizeof(double));
  
  if (new_array->data == NULL) {
     
     return CystckErr_NoMemory(S);
  }
  *result = new_array;
  return Cystck_new_array;
};

static Cystck_Object empty(Py_State *S, Cystck_Object arg) {
  int size;
  size = (int)CystckLong_AsLong(S, arg);
  ArrayObject *result;
  Cystck_pushobject(S, Array_empty(S, size, &result));
  return 1;
};

static Cystck_Object zeros(Py_State *S, Cystck_Object arg) {
  int size;
  size = (int)CystckLong_AsLong(S, arg);
  ArrayObject *result = NULL;
  Cystck_Object _result = Array_empty(S, size, &result);
  for(int i=0; i<size; i++)
      result->data[i] = 0;
  Cystck_pushobject(S, _result);
  return 1;
};
CystckDef_METH(empty_methd, "empty",empty, Cystck_METH_O, "Create an empty array." );
CystckDef_METH(zeros_methd, "zeros",zeros, Cystck_METH_O, "Createa zero-filled array." );

CystckDef *module_methods[]=
{
    &empty_methd,
    &zeros_methd,
    NULL
};


static CyModuleDef piconumpymodule = {
    .m_name = "piconumpy",
    .m_doc = "piconumpy implemented with the CPython C-API.", .m_size = -1,
    .m_size = -1,
    .m_methods= module_methods,
    };

CyMODINIT_FUNC (piconumpymodule)
CyInit_piconumpymodule(Py_State *Py_state)
{
    Cystck_Object m;
    m = CystckModule_Create(Py_state,&piconumpymodule);
    if (Cystck_IsNULL(m))
        return 0;

    Cystck_ArrayType =Cystck_Type_FromSpec(Py_state, NULL, &Array_type_spec);
    if (CystckModule_AddObject(Py_state, m, "array", Cystck_ArrayType)<0)
    {
      Cystck_pop(Py_state, m);
      Cystck_pop(Py_state, Cystck_ArrayType);
      return 0;
    }
    return m; 
} 
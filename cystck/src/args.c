#include <stdlib.h>
#include "args.h"

int check_number(PyObject *args, int idx)
{
    //int i = PyLong_AsLong(PyTuple_GetItem(args,idx));
    int i = ((Intobject*)((Argobject*)args) -> ob_item[idx])->ob_digit[1];
    
    return i;
}

double check_double(PyObject *args, int idx)
{
    double d =PyFloat_AsDouble(PyTuple_GetItem(args,idx));


    return d;
}



float check_float(PyObject *args)
{
    float f;
    PyArg_ParseTuple(args,"f",&f);
    return f;
}



void Cystck_arg_check(int cond, int numarg, char *extramsg)
{
  while (!cond) 
  {
    fprintf (stderr, "bad argument #%d to function (%s)\n",
                      numarg, extramsg);
    break;
  }
}

char *Cystck_check_string (Py_State *Py_state,int numArg)
{
  Cystck_Object o = Cystck_getparam(Py_state,numArg);
  Cystck_arg_check(Cystck_isstring(o), numArg, "string expected");
  return Cystck_getstring(o);
}


float Cystck_check_number (Py_State *Py_state,int numArg)
{
  Cystck_Object o = Cystck_getparam(Py_state,numArg);
  Cystck_arg_check(Cystck_isnumber(o), numArg, "number expected");
  return Cystck_getnumber(o);
}

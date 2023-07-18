//#include <Python.h>
#include "../include/Cystck.h"
#define Argobject PyTupleObject
#define Intobject PyLongObject
int check_number(PyObject *args, int idx);
char *check_string(PyObject *args, int idx);
void setArg(PyObject *args);
double check_double(PyObject *args, int idx);
char *Cystck_check_string (Py_State *Py_state,int numArg);
float Cystck_check_number (Py_State *Py_state,int numArg);

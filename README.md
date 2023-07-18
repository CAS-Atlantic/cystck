# CYSTCK

Following the success of using a stack to aid garbage collection in previous work, and the need to still provide compatibility of the Python C API by keeping the PyObject union type, we combine the stack and handles, a light-weight set of handles, to prototype a memory management amiable API for Python, which we call CyStck. The stack and light-weight handles not only serve the purpose of garbage collection but also act as a means of communication from C to Python and Python back to C.


With CyStck, extension authors can then write code that uses a stack such as below:

```
Cystck_Object c_add(Py_State *S,PyObject*args)
{
    int n;
    int m;
    CystckArg_parseTuple(S,args,"ii",&n,&m);
    int result = n+m;

    Cystck_Pushnumber(S,result);

    return 1; //  the PyObject at the top of the stack 
} 
```
Create a structure of methods being implemenhted.

```
Cystck_METH_DEF(Add_methd, "c_add", c_add, Cystck_METH_VARARGS, "method description" );
```

Implement an array of method structures that contains all the functions that are implemented by the module together with its relevant information:
```
CyMethodDef *module_methods[]=
{
    & Add_methd // structure of the method being implemented                                             
    NULL //The final string describes that the method does.
};
```
Then create a module struct:
```
struct CyModuleDef testmodule = {
    .m_name ="test",                     // Name of the Module
    .m_doc= 'Module description', //  represents your module docstring that is what the module does
    .m_size = -1,                    //the amount of memory needed to storeprogram state
    .m_methods= methods                 //is the reference to your method array. This is the array of CyMethodDef structs you defined earlier
}
```
Finally, we initialize the module to be used by the interpreter using ```CYMODINIT```. 
```c_module``` is the module we expose to the interpreter:

```
CyMODINIT_FUNC (c_module)
CyInit_Cystck_module(Py_State *S , Pystate* state)
{
    

    return CystckModule_Create(S,&Cystck_module);
}
```

**Packaging Your Python C Extension Module**

Before you can import your new module, you first need to build it. You can do this by using the Python's  ```distutils```.

You will need a file called ```setup.py``` withe following code to install your application.
```
from distutils.core import setup, Extension
module=Extension(
    "c_module", //name of the module
    ["c_module.c"]//is a list of paths to file with the source code, relative to the setup script.
)
setup(
    name="c_module", //package name
    version='1.0', //version
    description='the package description',
    ext_modules=[module]
)
```
The code block above shows the some  arguments that are passed to ```setup()```.

Finally, create the shared library using:

```
python3 setup.py build
```

Then install the module:

```
python3 setup.py install
```

Now the module can be imported in any python file. 

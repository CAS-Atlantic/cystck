/* -*- mode: c++; c-basic-offset: 4 -*- */

// Where is PIL?
//
// Many years ago, Matplotlib used to include code from PIL (the Python Imaging
// Library).  Since then, the code has changed a lot - the organizing principle
// and methods of operation are now quite different.  Because our review of
// the codebase showed that all the code that came from PIL was removed or
// rewritten, we have removed the PIL licensing information.  If you want PIL,
// you can get it at https://python-pillow.org/

#define PY_SSIZE_T_CLEAN
#include "../../include/Cystck.h"
#include "cystck_helper.h"

#ifdef _WIN32
#define WIN32_DLL
#endif
#ifdef __CYGWIN__
/*
 * Unfortunately cygwin's libdl inherits restrictions from the underlying
 * Windows OS, at least currently. Therefore, a symbol may be loaded from a
 * module by dlsym() only if it is really located in the given module,
 * dependencies are not included. So we have to use native WinAPI on Cygwin
 * also.
 */
#define WIN32_DLL
static inline PyObject *PyErr_SetFromWindowsErr(int ierr) {
    PyErr_SetString(PyExc_OSError, "Call to EnumProcessModules failed");
    return NULL;
}
#endif

#ifdef WIN32_DLL
#include <string>
#include <windows.h>
#include <commctrl.h>
#define PSAPI_VERSION 1
#include <psapi.h>  // Must be linked with 'psapi' library
#define dlsym GetProcAddress
#else
#include <dlfcn.h>
#endif

// Include our own excerpts from the Tcl / Tk headers
#include "_tkmini.h"

static int convert_voidptr(Py_State *S, Cystck_Object obj, void *p)
{
    void **val = (void **)p;
    *val = CystckLong_AsVoidPtr(S, obj);
    return *val != NULL ? 1 : !Cystck_Err_Occurred(S);
}

// Global vars for Tk functions.  We load these symbols from the tkinter
// extension module or loaded Tk libraries at run-time.
static Tk_FindPhoto_t TK_FIND_PHOTO;
static Tk_PhotoPutBlock_t TK_PHOTO_PUT_BLOCK;
// Global vars for Tcl functions.  We load these symbols from the tkinter
// extension module or loaded Tcl libraries at run-time.
static Tcl_SetVar_t TCL_SETVAR;

static Cystck_Object mpl_tk_blit(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_interp = 0, h_data_ptr = 0;
    Tcl_Interp *interp;
    char const *photo_name;
    int height, width;
    unsigned char *data_ptr;
    int comp_rule;
    int put_retval;
    int o0, o1, o2, o3;
    int x1, x2, y1, y2;
    Tk_PhotoHandle photo;
    Tk_PhotoImageBlock block;
    Cystck_Object tuple1, tuple2, tuple3;
    if (!CystckArg_parseTuple(S, args, "OsOOO:blit",
                          &h_interp, &photo_name,
                          &tuple1,
                          &tuple2,
                          &tuple3
                          )) {
        goto exit;
    }
    int ret;
    Arg_ParseTuple(ret, S, tuple1, "iiO:blit", &height, &width, &h_data_ptr)
    h_data_ptr = Cystck_Dup(S, h_data_ptr);
    Cystck_DECREF(S, tuple1);
    Cystck_pushobject(S, h_data_ptr);
    if(!ret){
        Cystck_pop(S, h_data_ptr);
        return -1;
    }
    if (!convert_voidptr(S, h_interp, &interp) || !convert_voidptr(S, h_data_ptr, &data_ptr)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "blit"); 
        Cystck_pop(S, h_data_ptr);
        return -1;
    }
    Cystck_pop(S, h_data_ptr);

    if (!(photo = TK_FIND_PHOTO(interp, photo_name))) {
        CystckErr_SetString(S, S->Cystck_ValueError, "Failed to extract Tk_PhotoHandle");
        goto exit;
    }
    Arg_ParseTupleAndClose(ret, S, tuple2, "iiii:blit", &o0, &o1, &o2, &o3)
    if (!ret) {
        return -1;
    }
    Arg_ParseTupleAndClose(ret, S, tuple3, "iiii:blit", &x1, &x2, &y1, &y2)
    if (!ret) {
        return -1;
    }
    if (0 > y1 || y1 > y2 || y2 > height || 0 > x1 || x1 > x2 || x2 > width) {
        CystckErr_SetString(S, S->Cystck_ValueError, "Attempting to draw out of bounds");
        goto exit;
    }
    if (comp_rule != TK_PHOTO_COMPOSITE_OVERLAY && comp_rule != TK_PHOTO_COMPOSITE_SET) {
        CystckErr_SetString(S, S->Cystck_ValueError, "Invalid comp_rule argument");
        goto exit;
    }
    Cystck_BEGIN_LEAVE_PYTHON(S)
    block.pixelPtr = data_ptr + 4 * ((height - y2) * width + x1);
    block.width = x2 - x1;
    block.height = y2 - y1;
    block.pitch = 4 * width;
    block.pixelSize = 4;
    block.offset[0] = o0;
    block.offset[1] = o1;
    block.offset[2] = o2;
    block.offset[3] = o3;
    put_retval = TK_PHOTO_PUT_BLOCK(
        interp, photo, &block, x1, height - y2, x2 - x1, y2 - y1, comp_rule);
    Cystck_END_LEAVE_PYTHON(S)
    if (put_retval == TCL_ERROR) {
        CystckErr_NoMemory(S);
        return -1;
    }

exit:
    if (Cystck_Err_Occurred(S)) {
        return -1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
        return 1;
    }
}

#ifdef WIN32_DLL
LRESULT CALLBACK
DpiSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {
    case WM_DPICHANGED:
        // This function is a subclassed window procedure, and so is run during
        // the Tcl/Tk event loop. Unfortunately, Tkinter has a *second* lock on
        // Tcl threading that is not exposed publicly, but is currently taken
        // while we're in the window procedure. So while we can take the GIL to
        // call Python code, we must not also call *any* Tk code from Python.
        // So stay with Tcl calls in C only.
        {
            // This variable naming must match the name used in
            // lib/matplotlib/backends/_backend_tk.py:FigureManagerTk.
            std::string var_name("window_dpi");
            var_name += std::to_string((unsigned long long)hwnd);

            // X is high word, Y is low word, but they are always equal.
            std::string dpi = std::to_string(LOWORD(wParam));

            Tcl_Interp* interp = (Tcl_Interp*)dwRefData;
            TCL_SETVAR(interp, var_name.c_str(), dpi.c_str(), 0);
        }
        return 0;
    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, DpiSubclassProc, uIdSubclass);
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}
#endif

static Cystck_Object
mpl_tk_enable_dpi_awareness(Py_State *S, Cystck_Object args)
{
    if (Cystck_Length(S, args) != 2) {
        CystckErr_SetString(S, S->Cystck_TypeError, "enable_dpi_awareness() takes 2 positional "
                            "arguments");
        return -1;
    }

#ifdef WIN32_DLL
    HWND frame_handle = NULL;
    Tcl_Interp *interp = NULL;

    if (!convert_voidptr(Cystck_AsPyObject(S,args)[0], &frame_handle)) {
        return -1;
    }
    if (!convert_voidptr(Cystck_AsPyObject(S,args)[1], &interp)) {
        return -1;
    }

#ifdef _DPI_AWARENESS_CONTEXTS_
    HMODULE user32 = LoadLibrary("user32.dll");

    typedef DPI_AWARENESS_CONTEXT (WINAPI *GetWindowDpiAwarenessContext_t)(HWND);
    GetWindowDpiAwarenessContext_t GetWindowDpiAwarenessContextPtr =
        (GetWindowDpiAwarenessContext_t)GetProcAddress(
            user32, "GetWindowDpiAwarenessContext");
    if (GetWindowDpiAwarenessContextPtr == NULL) {
        FreeLibrary(user32);
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }

    typedef BOOL (WINAPI *AreDpiAwarenessContextsEqual_t)(DPI_AWARENESS_CONTEXT,
                                                          DPI_AWARENESS_CONTEXT);
    AreDpiAwarenessContextsEqual_t AreDpiAwarenessContextsEqualPtr =
        (AreDpiAwarenessContextsEqual_t)GetProcAddress(
            user32, "AreDpiAwarenessContextsEqual");
    if (AreDpiAwarenessContextsEqualPtr == NULL) {
        FreeLibrary(user32);
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }

    DPI_AWARENESS_CONTEXT ctx = GetWindowDpiAwarenessContextPtr(frame_handle);
    bool per_monitor = (
        AreDpiAwarenessContextsEqualPtr(
            ctx, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) ||
        AreDpiAwarenessContextsEqualPtr(
            ctx, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE));

    if (per_monitor) {
        // Per monitor aware means we need to handle WM_DPICHANGED by wrapping
        // the Window Procedure, and the Python side needs to trace the Tk
        // window_dpi variable stored on interp.
        SetWindowSubclass(frame_handle, DpiSubclassProc, 0, (DWORD_PTR)interp);
    }
    FreeLibrary(user32);
    return PyBool_FromLong(per_monitor);
#endif
#endif

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}

CystckDef_METH(mpl_tk_blit_def, "blit", mpl_tk_blit, Cystck_METH_VARARGS, NULL)
CystckDef_METH(mpl_tk_enable_dpi_awareness_def, "enable_dpi_awareness", mpl_tk_enable_dpi_awareness, Cystck_METH_VARARGS, NULL)


static CystckDef *module_defines[] = {
    &mpl_tk_blit_def,
    &mpl_tk_enable_dpi_awareness_def,
    NULL
};


// Functions to fill global Tcl/Tk function pointers by dynamic loading.

template <class T>
bool load_tcl_tk(T lib)
{
    // Try to fill Tcl/Tk global vars with function pointers.  Return whether
    // all of them have been filled.
    if (auto ptr = dlsym(lib, "Tcl_SetVar")) {
        TCL_SETVAR = (Tcl_SetVar_t)ptr;
    }
    if (auto ptr = dlsym(lib, "Tk_FindPhoto")) {
        TK_FIND_PHOTO = (Tk_FindPhoto_t)ptr;
    }
    if (auto ptr = dlsym(lib, "Tk_PhotoPutBlock")) {
        TK_PHOTO_PUT_BLOCK = (Tk_PhotoPutBlock_t)ptr;
    }
    return TCL_SETVAR && TK_FIND_PHOTO && TK_PHOTO_PUT_BLOCK;
}

#ifdef WIN32_DLL

/* On Windows, we can't load the tkinter module to get the Tcl/Tk symbols,
 * because Windows does not load symbols into the library name-space of
 * importing modules. So, knowing that tkinter has already been imported by
 * Python, we scan all modules in the running process for the Tcl/Tk function
 * names.
 */

void load_tkinter_funcs(Py_State *S)
{
    HANDLE process = GetCurrentProcess();  // Pseudo-handle, doesn't need closing.
    HMODULE* modules = NULL;
    DWORD size;
    if (!EnumProcessModules(process, NULL, 0, &size)) {
        PyErr_SetFromWindowsErr(0);
        goto exit;
    }
    if (!(modules = static_cast<HMODULE*>(malloc(size)))) {
        PyErr_NoMemory();
        goto exit;
    }
    if (!EnumProcessModules(process, modules, size, &size)) {
        PyErr_SetFromWindowsErr(0);
        goto exit;
    }
    for (unsigned i = 0; i < size / sizeof(HMODULE); ++i) {
        if (load_tcl_tk(modules[i])) {
            return;
        }
    }
exit:
    free(modules);
}

#else  // not Windows

/*
 * On Unix, we can get the Tk symbols from the tkinter module, because tkinter
 * uses these symbols, and the symbols are therefore visible in the tkinter
 * dynamic library (module).
 */

void load_tkinter_funcs(Py_State *S)
{
    // Load tkinter global funcs from tkinter compiled module.
    void *main_program = NULL, *tkinter_lib = NULL;
    Cystck_Object module = 0, py_path = 0, py_path_b = 0;
    char *path;

    // Try loading from the main program namespace first.
    main_program = dlopen(NULL, RTLD_LAZY);
    if (load_tcl_tk(main_program)) {
        goto exit;
    }
    // Clear exception triggered when we didn't find symbols above.
    CystckErr_Clear(S);

    // Handle PyPy first, as that import will correctly fail on CPython.
    module = Cystck_Import_ImportModule( "_tkinter.tklib_cffi");   // PyPy
    if (Cystck_IsNULL(module)) {
        CystckErr_Clear(S);
        module = Cystck_Import_ImportModule("_tkinter");  // CPython
    }
    if (!(!Cystck_IsNULL(module) &&
          !Cystck_IsNULL(py_path = Cystck_GetAttr_s(S, module, "__file__")) &&
          !Cystck_IsNULL(py_path_b = CystckUnicode_EncodeFSDefault(S, py_path)) &&
          (path = CystckBytes_AsString(S, py_path_b)))) {
        goto exit;
    }
    tkinter_lib = dlopen(path, RTLD_LAZY);
    if (!tkinter_lib) {
        CystckErr_SetString(S, S->Cystck_RuntimeError, dlerror());
        goto exit;
    }
    if (load_tcl_tk(tkinter_lib)) {
        goto exit;
    }

exit:
    // We don't need to keep a reference open as the main program & tkinter
    // have been imported.  Use a non-short-circuiting "or" to try closing both
    // handles before handling errors.
    if ((main_program && dlclose(main_program))
        | (tkinter_lib && dlclose(tkinter_lib))) {
        PyErr_SetString(PyExc_RuntimeError, dlerror());
    }
    Cystck_DECREF(S, module);
    Cystck_DECREF(S, py_path);
    Cystck_DECREF(S, py_path_b);
}
#endif // end not Windows

static CyModuleDef moduledef = {
  .m_name = "_tkagg",
  .m_doc = "",
  .m_size = -1,
  .m_methods = module_defines,
};


#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC visibility push(default)

CyMODINIT_FUNC(_tkagg) CyInit__tkagg(Py_State *S)
{
    load_tkinter_funcs(S);
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    // Always raise ImportError (normalizing a previously set exception if
    // needed) to interact properly with backend auto-fallback.
    if (value) {
        PyErr_NormalizeException(&type, &value, &traceback);
        PyErr_SetObject(PyExc_ImportError, value);
        return 0;
    } else if (!TCL_SETVAR) {
        CystckErr_SetString(S, S->Cystck_ImportError, "Failed to load Tcl_SetVar");
        return 0;
    } else if (!TK_FIND_PHOTO) {
        CystckErr_SetString(S, S->Cystck_ImportError, "Failed to load Tk_FindPhoto");
        return 0;
    } else if (!TK_PHOTO_PUT_BLOCK) {
        CystckErr_SetString(S, S->Cystck_ImportError, "Failed to load Tk_PhotoPutBlock");
        return 0;
    }
    return CystckModule_Create(S, &moduledef);
}

#pragma GCC visibility pop

#ifdef __cplusplus
}
#endif

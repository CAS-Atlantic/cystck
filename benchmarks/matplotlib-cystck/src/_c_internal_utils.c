#define PY_SSIZE_T_CLEAN
#ifdef __linux__
#include <dlfcn.h>
#endif
#ifdef _WIN32
#include <Objbase.h>
#include <Shobjidl.h>
#include <Windows.h>
#endif
#include "../../include/Cystck.h"

static Cystck_Object
mpl_display_is_valid(Py_State *S)
{
#ifdef __linux__
    void* libX11;
    // The getenv check is redundant but helps performance as it is much faster
    // than dlopen().
    if (getenv("DISPLAY")
        && (libX11 = dlopen("libX11.so.6", RTLD_LAZY))) {
        struct Display* display = NULL;
        struct Display* (* XOpenDisplay)(char const*) =
            dlsym(libX11, "XOpenDisplay");
        int (* XCloseDisplay)(struct Display*) =
            dlsym(libX11, "XCloseDisplay");
        if (XOpenDisplay && XCloseDisplay
                && (display = XOpenDisplay(NULL))) {
            XCloseDisplay(display);
        }
        if (dlclose(libX11)) {
            CystckErr_SetString(S, S->Cystck_RuntimeError, dlerror());
            return -1;
        }
        if (display) {
            Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
            return 1;
        }
    }
    void* libwayland_client;
    if (getenv("WAYLAND_DISPLAY")
        && (libwayland_client = dlopen("libwayland-client.so.0", RTLD_LAZY))) {
        struct wl_display* display = NULL;
        struct wl_display* (* wl_display_connect)(char const*) =
            dlsym(libwayland_client, "wl_display_connect");
        void (* wl_display_disconnect)(struct wl_display*) =
            dlsym(libwayland_client, "wl_display_disconnect");
        if (wl_display_connect && wl_display_disconnect
                && (display = wl_display_connect(NULL))) {
            wl_display_disconnect(display);
        }
        if (dlclose(libwayland_client)) {
            CystckErr_SetString(S, S->Cystck_RuntimeError, dlerror());
            return 0;
        }
        if (display) {
            Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
            return 1;
        }
    }
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
    return 1;
#else
    Py_RETURN_TRUE;
#endif
}

static Cystck_Object
mpl_GetCurrentProcessExplicitAppUserModelID(Py_State *S)
{
#ifdef _WIN32
    wchar_t* appid = NULL;
    HRESULT hr = GetCurrentProcessExplicitAppUserModelID(&appid);
    if (FAILED(hr)) {
        //return PyErr_SetFromWindowsErr(hr);
        return -1;
    }
    Cystck_Object py_appid = CystckUnicode_FromWideChar(S, appid, -1);
    CoTaskMemFree(appid);
    Cystck_pushobject(S, py_appid);
    return 1;
#else
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
#endif
}

static Cystck_Object
mpl_SetCurrentProcessExplicitAppUserModelID(Py_State *S, Cystck_Object arg)
{
#ifdef _WIN32
    wchar_t* appid = CystckUnicode_AsWideCharString(S, arg, NULL);
    if (!appid) {
        return 0;
    }
    HRESULT hr = SetCurrentProcessExplicitAppUserModelID(appid);
    free(appid);
    if (FAILED(hr)) {
        //return PyErr_SetFromWindowsErr(hr);
        return -1;
    }
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
#else
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
#endif
}

static Cystck_Object
mpl_GetForegroundWindow(Py_State *S)
{
#ifdef _WIN32
  Cystck_pushobject(S, CystckLong_FromVoidPtr(S, GetForegroundWindow()))
  return 1;
#else
  Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
  return 1;
#endif
}

static Cystck_Object
mpl_SetForegroundWindow(Py_State *S,Cystck_Object arg)
{
#ifdef _WIN32
  HWND handle = CystckLong_AsVoidPtr(S, arg);
  if (Cystck_Err_Occurred(S)) {
    return -1;
  }
  if (!SetForegroundWindow(handle)) {
    CystckErr_SetString(S, S->Cystck_RuntimeError, "Error setting window");
    return -1;
  }
  Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
  return 1;
#else
  Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
  return 1;
#endif
}

// static PyObject*
// mpl_SetProcessDpiAwareness_max(PyObject* module)
// {
// #ifdef _WIN32
// #ifdef _DPI_AWARENESS_CONTEXTS_
//     // These functions and options were added in later Windows 10 updates, so
//     // must be loaded dynamically.
//     typedef BOOL (WINAPI *IsValidDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);
//     typedef BOOL (WINAPI *SetProcessDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);

//     HMODULE user32 = LoadLibrary("user32.dll");
//     IsValidDpiAwarenessContext_t IsValidDpiAwarenessContextPtr =
//         (IsValidDpiAwarenessContext_t)GetProcAddress(
//             user32, "IsValidDpiAwarenessContext");
//     SetProcessDpiAwarenessContext_t SetProcessDpiAwarenessContextPtr =
//         (SetProcessDpiAwarenessContext_t)GetProcAddress(
//             user32, "SetProcessDpiAwarenessContext");
//     DPI_AWARENESS_CONTEXT ctxs[3] = {
//         DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2,  // Win10 Creators Update
//         DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE,     // Win10
//         DPI_AWARENESS_CONTEXT_SYSTEM_AWARE};         // Win10
//     if (IsValidDpiAwarenessContextPtr != NULL
//             && SetProcessDpiAwarenessContextPtr != NULL) {
//         for (int i = 0; i < sizeof(ctxs) / sizeof(DPI_AWARENESS_CONTEXT); ++i) {
//             if (IsValidDpiAwarenessContextPtr(ctxs[i])) {
//                 SetProcessDpiAwarenessContextPtr(ctxs[i]);
//                 break;
//             }
//         }
//     } else {
//         // Added in Windows Vista.
//         SetProcessDPIAware();
//     }
//     FreeLibrary(user32);
// #else
//     // Added in Windows Vista.
//     SetProcessDPIAware();
// #endif
// #endif
//     Py_RETURN_NONE;
// }

CystckDef_METH(mpl_display_is_valid_def, "display_is_valid", mpl_display_is_valid, Cystck_METH_NOARGS,
    "display_is_valid()\n--\n\n"
    "Check whether the current X11 or Wayland display is valid.\n\n"
    "On Linux, returns True if either $DISPLAY is set and XOpenDisplay(NULL)\n"
    "succeeds, or $WAYLAND_DISPLAY is set and wl_display_connect(NULL)\n"
    "succeeds.  On other platforms, always returns True.");
CystckDef_METH(mpl_GetCurrentProcessExplicitAppUserModelID_def, "Win32_GetCurrentProcessExplicitAppUserModelID",
    mpl_GetCurrentProcessExplicitAppUserModelID, Cystck_METH_NOARGS,
    "Win32_GetCurrentProcessExplicitAppUserModelID()\n--\n\n"
    "Wrapper for Windows's GetCurrentProcessExplicitAppUserModelID.  On \n"
    "non-Windows platforms, always returns None.");
CystckDef_METH(mpl_SetCurrentProcessExplicitAppUserModelID_def, "Win32_SetCurrentProcessExplicitAppUserModelID",
    mpl_SetCurrentProcessExplicitAppUserModelID, Cystck_METH_O,
    "Win32_SetCurrentProcessExplicitAppUserModelID(appid, /)\n--\n\n"
    "Wrapper for Windows's SetCurrentProcessExplicitAppUserModelID.  On \n"
    "non-Windows platforms, a no-op.");
CystckDef_METH(mpl_GetForegroundWindow_def, "Win32_GetForegroundWindow",
    mpl_GetForegroundWindow, Cystck_METH_NOARGS,
    "Win32_GetForegroundWindow()\n--\n\n"
    "Wrapper for Windows' GetForegroundWindow.  On non-Windows platforms, \n"
    "always returns None.");
CystckDef_METH(mpl_SetForegroundWindow_def, "Win32_SetForegroundWindow",
    mpl_SetForegroundWindow, Cystck_METH_O,
    "Win32_SetForegroundWindow(hwnd, /)\n--\n\n"
    "Wrapper for Windows' SetForegroundWindow.  On non-Windows platforms, \n"
    "a no-op.");

static CystckDef *module_defines[] = {
    &mpl_display_is_valid_def,
    &mpl_GetCurrentProcessExplicitAppUserModelID_def,
    &mpl_SetCurrentProcessExplicitAppUserModelID_def,
    &mpl_GetForegroundWindow_def,
    &mpl_SetForegroundWindow_def,
    NULL
};

static CyModuleDef util_module = {
  .m_name = "_c_internal_utils",
  .m_doc = 0,
  .m_size = -1,
  .m_methods = module_defines,
};

#pragma GCC visibility push(default)
CyMODINIT_FUNC(_c_internal_utils)
CyInit__c_internal_utils(Py_State *S)
{
  Cystck_Object module = CystckModule_Create(S, &util_module);
  if (Cystck_IsNULL(module)) {
      return 0;
  }
  return module;
}

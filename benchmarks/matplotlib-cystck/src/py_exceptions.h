/* -*- mode: c++; c-basic-offset: 4 -*- */

#ifndef MPL_PY_EXCEPTIONS_H
#define MPL_PY_EXCEPTIONS_H

#include <exception>
#include <stdexcept>

namespace py
{
class exception : public std::exception
{
  public:
    const char *what() const throw()
    {
        return "python error has been set";
    }
};
}

// #ifdef CYSTCK
#include "../../include/Cystck.h"

#define CALL_CPP_FULL_CYSTCK(S, name, a, cleanup, errorcode)                           \
    try                                                                      \
    {                                                                        \
        a;                                                                   \
    }                                                                        \
    catch (const py::exception &)                                            \
    {                                                                        \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        /* return (errorcode);                                                */ \
        return -1;                                                  \
    }                                                                        \
    catch (const std::bad_alloc &)                                           \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_MemoryError, "In XX Out of memory");     \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        /* return (errorcode);                                                */ \
        return -1;                                                  \
    }                                                                        \
    catch (const std::overflow_error &e)                                     \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_OverflowError, e.what());    \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        /* return (errorcode);                                                */ \
        return -1;                                                  \
    }                                                                        \
    catch (const std::runtime_error &e)                                      \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_RuntimeError, e.what());    \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        /* return (errorcode);                                                */ \
        return -1;                                                  \
    }                                                                        \
    catch (...)                                                              \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_RuntimeError, name); \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        /* return (errorcode);                                                */ \
        return -1;                                                  \
    }


#define CALL_CPP_FULL_CYSTCK_RET_INT(S, name, a, cleanup, errorcode)                           \
    try                                                                      \
    {                                                                        \
        a;                                                                   \
    }                                                                        \
    catch (const py::exception &)                                            \
    {                                                                        \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (const std::bad_alloc &)                                           \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_MemoryError, "In XX Out of memory");     \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (const std::overflow_error &e)                                     \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_OverflowError, e.what());    \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (const std::runtime_error &e)                                      \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_RuntimeError, e.what());    \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (...)                                                              \
    {                                                                        \
        CystckErr_SetString(S, S->Cystck_RuntimeError, name); \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }

#define CALL_CPP_CLEANUP_CYSTCK(S, name, a, cleanup) CALL_CPP_FULL_CYSTCK(S, name, a, cleanup, 0)

#define CALL_CPP_CYSTCK(S, name, a) CALL_CPP_FULL_CYSTCK(S, name, a, , 0)

#define CALL_CPP_INIT_CYSTCK(S, name, a) CALL_CPP_FULL_CYSTCK_RET_INT(S, name, a, , -1)

// #else

#define CALL_CPP_FULL(name, a, cleanup, errorcode)                           \
    try                                                                      \
    {                                                                        \
        a;                                                                   \
    }                                                                        \
    catch (const py::exception &)                                            \
    {                                                                        \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (const std::bad_alloc &)                                           \
    {                                                                        \
        PyErr_Format(PyExc_MemoryError, "In %s: Out of memory", (name));     \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (const std::overflow_error &e)                                     \
    {                                                                        \
        PyErr_Format(PyExc_OverflowError, "In %s: %s", (name), e.what());    \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (const std::runtime_error &e)                                      \
    {                                                                        \
        PyErr_Format(PyExc_RuntimeError, "In %s: %s", (name), e.what());    \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }                                                                        \
    catch (...)                                                              \
    {                                                                        \
        PyErr_Format(PyExc_RuntimeError, "Unknown exception in %s", (name)); \
        {                                                                    \
            cleanup;                                                         \
        }                                                                    \
        return (errorcode);                                                  \
    }

#define CALL_CPP_CLEANUP(name, a, cleanup) CALL_CPP_FULL(name, a, cleanup, 0)

#define CALL_CPP(name, a) CALL_CPP_FULL(name, a, , 0)

#define CALL_CPP_INIT(name, a) CALL_CPP_FULL(name, a, , -1)

#endif
// #endif

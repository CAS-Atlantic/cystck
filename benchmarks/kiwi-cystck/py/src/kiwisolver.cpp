/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2021, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include <kiwi/kiwi.h>
#include "types.h"
#define PY_KIWI_VERSION "1.3.2"

namespace
{


bool ready_types(Py_State *S, Cystck_Object m)
{
    using namespace kiwisolver;
    if( !Variable::Ready(S, m) )
    {
        return false;
    }
    if( !Term::Ready(S, m) )
    {
        return false;
    }
    if( !Expression::Ready(S, m) )
    {
        return false;
    }
    if( !Constraint::Ready(S, m) )
    {
        return false;
    }
    if( !strength::Ready(S, m) )
    {
        return false;
    }
    if( !Solver::Ready(S, m) )
    {
        return false;
    }
    return true;
}

bool add_objects(Py_State *S,  Cystck_Object mod )
{
	using namespace kiwisolver;

    Cystck_Object kiwiversion = CystckUnicode_FromString(S,  KIWI_VERSION ) ;
    if( Cystck_IsNULL(kiwiversion) )
    {
        return false;
    }
    Cystck_pushobject(S, kiwiversion);
    if (Cystck_SetAttr_s(S, mod, "__kiwi_version__", kiwiversion)) {
        Cystck_pop(S, kiwiversion);
		return false;
    }
    Cystck_pop(S, kiwiversion);

    Cystck_Object pyversion =CystckUnicode_FromString(S, PY_KIWI_VERSION ) ;
    if( Cystck_IsNULL(pyversion) )
    {
        return false;
    }
    Cystck_pushobject(S, pyversion);
    if (Cystck_SetAttr_s(S, mod, "__version__", pyversion)) {
        Cystck_pop(S, pyversion);
		return false;
    }
    Cystck_pop(S, pyversion);
	
    return true;
}


int
kiwi_modexec(Py_State *S,  Cystck_Object mod )
{
    if( !ready_types(S, mod) )
    {
        return -1;
    }
    if( !kiwisolver::init_exceptions(S, mod) )
    {
        return -1;
    }
    if( !add_objects(S, mod) )
    {
        return -1;
    }


    return 0;
}



struct CyModuleDef moduledef = {
    .m_name = "kiwisolvercystck",
    .m_doc = "kiwisolver extension module",
    .m_size = 0,
};

}  // namespace


extern "C" {

CyMODINIT_FUNC(kiwisolvercystck)

CyInit_kiwisolvercystck(Py_State *S)
{
    Cystck_Object  m = CystckModule_Create(S, &moduledef);
    if (Cystck_IsNULL(m) || kiwi_modexec( S, m ) == -1 ) {
        return 0;
    }
    return m;
}
}
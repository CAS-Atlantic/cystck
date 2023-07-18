/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
//#include <cppy/cppy.h>
#include "../../../../cystck/include/Cystck.h"

#include <kiwi/kiwi.h>
#include "util.h"


#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif


namespace kiwisolver
{


namespace
{


void
strength_dealloc( Py_State *S )
{
	
}


Cystck_Object
strength_weak( Py_State *S, void *closure )
{
	Cystck_pushobject(S, CystckFloat_FromDouble(S, kiwi::strength::weak ));
	return 1;
}


Cystck_Object
strength_medium( Py_State *S, void *closure)
{
	Cystck_pushobject(S, CystckFloat_FromDouble(S, kiwi::strength::medium ));
	return 1;
}


Cystck_Object
strength_strong( Py_State *S, void *closure )
{
	Cystck_pushobject(S, CystckFloat_FromDouble(S, kiwi::strength::strong ));
	return 1;	
}


Cystck_Object
strength_required( Py_State *S, void *closure )
{
	Cystck_pushobject(S, CystckFloat_FromDouble(S, kiwi::strength::required ));
	return 1;
}


Cystck_Object
strength_create( Py_State *S, Cystck_Object args )
{
	Cystck_Object pya;
	Cystck_Object pyb;
	Cystck_Object pyc;
	Cystck_Object pyw = 0;
	if( !CystckArg_parseTuple(S, args, "OOO|O", &pya, &pyb, &pyc, &pyw ) )
		return -1;
	double a, b, c;
	double w = 1.0;
	if( !convert_to_double(S, pya, a ) )
		return -1;
	if( !convert_to_double( S, pyb, b ) )
		return -1;
	if( !convert_to_double(S, pyc, c ) )
		return -1;
	if( pyw && !convert_to_double(S, pyw, w ) )
		return -1;
	Cystck_pushobject(S, CystckFloat_FromDouble(S, kiwi::strength::create( a, b, c, w ) ));
	return  1;
}

CystckDef_GET(strength_weak_def, "weak", strength_weak, "The predefined weak strength.", NULL)
CystckDef_GET(strength_medium_def, "medium", strength_medium, "The predefined medium strength.", NULL)
CystckDef_GET(strength_strong_def, "strong", strength_strong, "The predefined strong strength.", NULL)
CystckDef_GET(strength_required_def, "required", strength_required, "The predefined required strength.", NULL)

CystckDef_METH(strength_create_def, "create", strength_create, Cystck_METH_VARARGS,
	"Create a strength from constituent values and optional weight.")


CystckDef_SLOT(strength_dealloc_def, strength_dealloc, Cystck_tp_dealloc)


static CystckDef* strength_defines[] = {
    // slots
	&strength_dealloc_def,

	// getsets
	&strength_weak_def,
	&strength_medium_def,
	&strength_strong_def,
	&strength_required_def,

	// methods
	&strength_create_def,
	NULL
};

} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
Cystck_Object strength::TypeObject = 0;


Cystck_Type_Spec strength::TypeObject_Spec = {
	.name = "kiwisolver.strength",
	.basicsize = sizeof( strength ),
	.itemsize = 0,
	.flags = Cystck_TPFLAGS_DEFAULT,
    // .legacy_slots = strength_Type_slots,
	.m_methods = strength_defines
};

bool strength::Ready(Py_State *S, Cystck_Object m)
{
    // The reference will be handled by the module to which we will add the type
    if (!CystckHelpers_AddType(S, m, "strength", &TypeObject_Spec)) {
        return false;
    }

    TypeObject = Cystck_GetAttr_s(S, m, "strength");
	if (Cystck_IsNULL(TypeObject)){
		return false;
	}
	Cystck_pushobject(S, TypeObject);
	strength* s;
	Cystck_Object h_strength = Cystck_New(S, TypeObject, &s);
	if (Cystck_IsNULL(h_strength)){
		Cystck_pop(S, TypeObject);
		return false;
	}
	Cystck_pop(S, TypeObject);
	Cystck_pushobject(S, h_strength);
    if( Cystck_SetAttr_s(S, m, "strength", h_strength) )
    {
        Cystck_pop(S, h_strength);
		return false;
    }
	Cystck_pop(S, h_strength);
    return true;
}


}  // namespace

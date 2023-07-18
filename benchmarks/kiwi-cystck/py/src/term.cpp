/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <sstream>
#include "../../../../cystck/include/Cystck.h"
// #include <cppy/cppy.h>
#include "symbolics.h"
#include "types.h"
#include "util.h"


namespace kiwisolver
{


namespace
{


Cystck_Object 
Term_new( Py_State *S, Cystck_Object type, Cystck_Object  args, Cystck_Object  kwargs )
{
	static const char *kwlist[] = { "variable", "coefficient", 0 };
	Cystck_Object pyvar;
	Cystck_Object pycoeff = 0;
	if( !CystckArg_parseTupleAndKeywords(S,
		args, kwargs, "O|O:__new__", (const char **)kwlist,
		&pyvar, &pycoeff ) )
		return -1;
	if( !Variable::TypeCheck(S, pyvar) )
	{
		CystckErr_SetString(S,  S->Cystck_TypeError, "Expected object of type `Variable`." );
        return -1;
	}
	double coefficient = 1.0;
	if(!Cystck_IsNULL(pycoeff) && !convert_to_double(S, pycoeff, coefficient ) )
		return -1;
	Term *self;
	Cystck_Object pyterm = Cystck_New(S, type, &self);
	if( Cystck_IsNULL(pyterm) )
		return -1;
	self->variable = Cystck_Dup(S, pyvar);
	self->coefficient = coefficient;
	Cystck_pushobject(S, pyterm);
	return 1;
}





int
Term_traverse( Term* self, visitproc visit, void* arg )
{
	Py_VISIT( Cystck_AsPyObject(NULL, self->variable) );
	return 0;
}



Cystck_Object 
Term_repr( Py_State *S, Cystck_Object _self )
{
	Term* self = Term::AsStruct(S, _self);
	std::stringstream stream;
	stream << self->coefficient << " * ";
	stream << Variable::AsStruct(S, self->variable )->variable.name();
	Cystck_pushobject(S, CystckUnicode_FromString(S, stream.str().c_str() ));
	return 1;
}


Cystck_Object 
Term_variable( Py_State *S )
{
	Term* self = Term::AsStruct(S, S->self);
	Cystck_pushobject(S, Cystck_Dup(S, self->variable));
	return 1;
}


Cystck_Object 
Term_coefficient( Py_State *S )
{
	Term* self = Term::AsStruct(S, S->self);
	Cystck_pushobject(S, CystckFloat_FromDouble(S, self->coefficient ));
	return 1;
}


Cystck_Object 
Term_value( Py_State *S )
{
	Term* self = Term::AsStruct(S, S->self);
	Variable* pyvar = Variable::AsStruct(S, self->variable );
	Cystck_pushobject(S, CystckFloat_FromDouble(S, self->coefficient * pyvar->variable.value() ));
	return  1;
}


Cystck_Object 
Term_add( Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryAdd, Term>()(S,  first, second));
	return 1;
}


Cystck_Object 
Term_sub( Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinarySub, Term>()(S, first, second) );
	return 1;
}


Cystck_Object 
Term_mul( Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryMul, Term>()(S,  first,  second));
	return 1;
}


Cystck_Object 
Term_div( Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryDiv, Term>()(S,  first, second));
	return 1;
}


Cystck_Object 
Term_neg( Py_State *S, Cystck_Object  value )
{
	Cystck_pushobject(S, UnaryInvoke<UnaryNeg, Term>()(S, value));
	return 1;	
}


Cystck_Object 
Term_richcmp( Py_State *S, Cystck_Object  first, Cystck_Object  second, int op )
{
	switch( op )
	{
		case Cystck_EQ:
			Cystck_pushobject(S, BinaryInvoke<CmpEQ, Term>()(S,  first, second)) ;
			return 1;
		case Cystck_LE:
			Cystck_pushobject(S, BinaryInvoke<CmpLE, Term>()(S, first, second)) ;
			return  1;
		case Cystck_GE:
			Cystck_pushobject(S, BinaryInvoke<CmpGE, Term>()( S,  first, second));
			return 1;
		default:
			break;
	}
	CystckErr_SetString(S, S->Cystck_TypeError, "unsupported operand type(s)" );
	return -1;
}

CystckDef_METH(Term_variable_def, "variable", Term_variable, Cystck_METH_NOARGS,
	"Get the variable for the term.")
CystckDef_METH(Term_coefficient_def, "coefficient", Term_coefficient, Cystck_METH_NOARGS,
	 "Get the coefficient for the term.")
CystckDef_METH(Term_value_def, "value", Term_value, Cystck_METH_NOARGS,
	 "Get the value for the term.")


CystckDef_SLOT(Term_repr_def, Term_repr, Cystck_tp_repr)            /* tp_repr */
CystckDef_SLOT(Term_richcmp_def, Term_richcmp, Cystck_tp_richcompare)  /* tp_richcompare */
CystckDef_SLOT(Term_new_def, Term_new, Cystck_tp_new)              /* tp_new */
CystckDef_SLOT(Term_add_def, Term_add, Cystck_nb_add)              /* nb_add */
CystckDef_SLOT(Term_sub_def, Term_sub, Cystck_nb_subtract)         /* nb_subatract */
CystckDef_SLOT(Term_mul_def, Term_mul, Cystck_nb_multiply)         /* nb_multiply */
CystckDef_SLOT(Term_neg_def, Term_neg, Cystck_nb_negative)         /* nb_negative */
CystckDef_SLOT(Term_div_def, Term_div, Cystck_nb_true_divide)      /* nb_true_divide */


static PyType_Slot Term_Type_slots[] = {
    { Py_tp_traverse, reinterpret_cast<void*>(  Term_traverse ) },    /* tp_traverse */
    { 0, 0 },
};

static CystckDef* Term_defines[] = {
    // slots
	&Term_repr_def,
	&Term_richcmp_def,
	&Term_new_def,
	&Term_add_def,
	&Term_sub_def,
	&Term_mul_def,
	&Term_neg_def,
	&Term_div_def,

    // methods
	&Term_variable_def,
	&Term_coefficient_def,
	&Term_value_def,
    NULL
};

} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
Cystck_Object Term::TypeObject = 0;


Cystck_Type_Spec Term::TypeObject_Spec = {
	.name = "kiwisolver.Term",
	.basicsize = sizeof( Term ),
	.itemsize = 0,
	.flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_HAVE_GC | Cystck_TPFLAGS_BASETYPE,
    .legacy_slots = Term_Type_slots,
	.m_methods = Term_defines
};



bool Term::Ready(Py_State *S, Cystck_Object m)
{
    // The reference will be handled by the module to which we will add the type
    if (!CystckHelpers_AddType(S, m, "Term", &TypeObject_Spec)) {
        return false;
    }

    TypeObject = Cystck_GetAttr_s(S, m, "Term");
    if( Cystck_IsNULL(TypeObject) )
    {
        return false;
    }
    return true;
}

}  // namespace kiwisolver

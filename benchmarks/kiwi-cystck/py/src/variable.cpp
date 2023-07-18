/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "../../../../cystck/include/Cystck.h"
// #include <cppy/cppy.h>
#include <kiwi/kiwi.h>
#include "symbolics.h"
#include "types.h"
#include "util.h"


namespace kiwisolver
{


namespace
{


Cystck_Object
Variable_new( Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kwargs )
{
	static const char *kwlist[] = { "name", "context", 0 };
	Cystck_Object context = 0;
	Cystck_Object name = 0;
	if( !CystckArg_parseTupleAndKeywords(S,
		args, kwargs, "|OO:__new__", (const char **)kwlist ,
		&name, &context ) )
	{
		
		return -1;
	}	

	Variable* self;
	Cystck_Object pyvar = Cystck_New(S, type, &self);
	if(Cystck_IsNULL(pyvar))
	{
		return -1;
	}
	Cystck_pushobject(S, pyvar);
	self->context = Cystck_Dup(S, context);

	if( !Cystck_IsNULL(name) )
	{
		if( !CystckUnicode_Check(S,  name ) )
		{
			CystckErr_SetString(S, S->Cystck_TypeError, "Expected str" );
        	Cystck_pop(S, pyvar);
			return -1;
		}
		std::string c_name;
		if( !convert_pystr_to_str(S, name, c_name) )
		{	
			Cystck_pop(S, pyvar);
			return -1;  // LCOV_EXCL_LINE
		}
		new( &self->variable ) kiwi::Variable( c_name );
	}
	else
	{
		new( &self->variable ) kiwi::Variable();
	}
	return  1;
}


int
Variable_traverse( Variable* self, visitproc visit, void* arg )
{
	Py_VISIT( Cystck_AsPyObject(NULL, self->context) );
	return 0;
}


void
Variable_dealloc( Py_State *S )
{
	Variable* self = Variable::AsStruct(S, S->self);
	self->variable.~Variable();
}


Cystck_Object
Variable_repr( Py_State *S, Cystck_Object _self )
{
	Variable* self = Variable::AsStruct(S, _self); 
	Cystck_pushobject(S, CystckUnicode_FromString(S, self->variable.name().c_str() ));
	return 1;
}


Cystck_Object
Variable_name( Py_State *S )
{
	Variable* self = Variable::AsStruct(S, S->self);
	Cystck_pushobject(S, CystckUnicode_FromString(S, self->variable.name().c_str() ));
	return 1;
}


Cystck_Object
Variable_setName( Py_State *S, Cystck_Object pystr )
{
	Variable* self = Variable::AsStruct(S, S->self);
	if( !CystckUnicode_Check(S, pystr ) )
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected str" );
        return -1;
	}
   std::string str;
   if( !convert_pystr_to_str(S, pystr, str ) )
       return -1;
   self->variable.setName( str );
   Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
   return 1;
}


Cystck_Object
Variable_context( Py_State *S )
{
	Variable* self = Variable::AsStruct(S, S->self);
	if( !Cystck_IsNULL(self->context))
	{
		Cystck_pushobject(S, Cystck_Dup (S,  self->context));
		return  1;
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object
Variable_setContext( Py_State *S, Cystck_Object value )
{
	Variable* self = Variable::AsStruct(S, S->self);
	if( Cystck_IsNULL(self->context) || !Cystck_Is(S,value, self->context) )
	{
		Cystck_Object temp = self->context;
		Cystck_pushobject(S, temp);
		self->context =  Cystck_Dup(S, value);
		Cystck_pop(S, temp);
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object
Variable_value( Py_State *S )
{
	Variable* self = Variable::AsStruct(S, S->self);
	Cystck_pushobject(S, CystckFloat_FromDouble(S, self->variable.value() ));
	return 1;
}


Cystck_Object
Variable_add( Py_State *S, Cystck_Object first, Cystck_Object second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryAdd, Variable>()( S,  first, second ));
	return 1;
}


Cystck_Object
Variable_sub( Py_State *S,  Cystck_Object first, Cystck_Object second )
{
	Cystck_pushobject(S, BinaryInvoke<BinarySub, Variable>()( S,  first, second ));
	return 1;
}


Cystck_Object
Variable_mul( Py_State *S, Cystck_Object first, Cystck_Object second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryMul, Variable>()( S,  first, second ));
	return 1;	
}


Cystck_Object
Variable_div( Py_State *S, Cystck_Object first, Cystck_Object second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryDiv, Variable>()( S,  first, second ));
	return 1;	
}


Cystck_Object
Variable_neg( Py_State *S,  Cystck_Object value )
{
	Cystck_pushobject(S, UnaryInvoke<UnaryNeg, Variable>()(S , value) );
	return 1;	
}


Cystck_Object
Variable_richcmp( Py_State *S, Cystck_Object first, Cystck_Object second, int op )
{
	switch( op )
	{
		case Cystck_EQ:
			Cystck_pushobject(S, BinaryInvoke<CmpEQ, Variable>()(S,  first, second) );
			return 1;
		case Cystck_LE:
			Cystck_pushobject(S, BinaryInvoke<CmpLE, Variable>()(S, first, second));
			return 1;
		case Cystck_GE:
			Cystck_pushobject(S, BinaryInvoke<CmpGE, Variable>()(S, first, second) );
			return 1;
		default:
			break;
	}
	CystckErr_SetString(S, S->Cystck_TypeError, "unsupported operand type(s)" );
	return -1;
}

static PyType_Slot Variable_Type_slots[] = {
    { Py_tp_traverse, reinterpret_cast<void*>(  Variable_traverse ) },    /* tp_traverse */
    { 0, 0 },
};

CystckDef_METH(Variable_name_def, "name", Variable_name, Cystck_METH_NOARGS,
	"Get the name of the variable.")
CystckDef_METH(Variable_setName_def, "setName", Variable_setName, Cystck_METH_O,
	 "Set the name of the variable.")
CystckDef_METH(Variable_context_def, "context", Variable_context, Cystck_METH_NOARGS,
	 "Get the context object associated with the variable.")
CystckDef_METH(Variable_setContext_def, "setContext", Variable_setContext, Cystck_METH_O,
	 "Set the context object associated with the variable.")
CystckDef_METH(Variable_value_def, "value", Variable_value, Cystck_METH_NOARGS,
	 "Get the current value of the variable.")


CystckDef_SLOT(Variable_repr_def, Variable_repr, Cystck_tp_repr)
CystckDef_SLOT(Variable_richcmp_def, Variable_richcmp, Cystck_tp_richcompare)
CystckDef_SLOT(Variable_new_def, Variable_new, Cystck_tp_new)
CystckDef_SLOT(Variable_add_def, Variable_add, Cystck_nb_add)
CystckDef_SLOT(Variable_sub_def, Variable_sub, Cystck_nb_subtract)
CystckDef_SLOT(Variable_mul_def, Variable_mul, Cystck_nb_multiply)
CystckDef_SLOT(Variable_neg_def, Variable_neg, Cystck_nb_negative)
CystckDef_SLOT(Variable_div_def, Variable_div, Cystck_nb_true_divide)
CystckDef_SLOT(Variable_dealloc_def, Variable_dealloc, Cystck_tp_dealloc)
static CystckDef* Variable_defines[] = {
    // slots
	&Variable_repr_def,
	&Variable_richcmp_def,
	&Variable_new_def,
	&Variable_add_def,
	&Variable_sub_def,
	&Variable_mul_def,
	&Variable_neg_def,
	&Variable_div_def,
	&Variable_dealloc_def,

    // methods
	&Variable_name_def,
	&Variable_setName_def,
	&Variable_context_def,
	&Variable_setContext_def,
	&Variable_value_def,
	NULL
};

} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
Cystck_Object Variable::TypeObject = 0;


Cystck_Type_Spec Variable::TypeObject_Spec = {
	.name = "kiwisolver.Variable",
	.basicsize = sizeof( Variable ),
	.itemsize = 0,
	.flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_HAVE_GC | Cystck_TPFLAGS_BASETYPE,
    .legacy_slots = Variable_Type_slots,
	.m_methods = Variable_defines
};


bool Variable::Ready(Py_State *S, Cystck_Object m)
{
    // The reference will be handled by the module to which we will add the type
    if (!CystckHelpers_AddType(S, m, "Variable", &TypeObject_Spec)) {
        return false;
    }

    TypeObject = Cystck_GetAttr_s(S, m, "Variable");
    if( Cystck_IsNULL(TypeObject) )
    {
        return false;
    }
    return true;
}

}  // namespace kiwisolver

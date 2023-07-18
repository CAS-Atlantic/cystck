/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "../../../../cystck/include/Cystck.h"
//#include <cppy/cppy.h>
#include <kiwi/kiwi.h>
#include "types.h"
#include "util.h"


namespace kiwisolver
{

namespace
{

Cystck_Object 
Solver_new( Py_State *S,  Cystck_Object type, Cystck_Object  args, Cystck_Object  kwargs )
{
	if( Cystck_Length(S, args) != 0 || ( !Cystck_IsNULL(kwargs) && Cystck_Length(S,kwargs ) != 0 ) )
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Solver.__new__ takes no arguments" );
        return -1;
	}
	Solver *self;
	Cystck_Object pysolver = Cystck_New(S, type, &self);
	if( Cystck_IsNULL(pysolver) )
		return -1;
	new( &self->solver ) kiwi::Solver();
	Cystck_pushobject(S, pysolver);
	return 1;
}


void
Solver_dealloc( Py_State *S )
{
	Solver* self = Solver_AsStruct(S, S->self);
	self->solver.~Solver();
}


Cystck_Object 
Solver_addConstraint( Py_State *S, Cystck_Object  other )
{
	Solver* self = Solver::AsStruct(S, S->self);
	if( !Constraint::TypeCheck(S,other) ) 
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Constraint`." );
        return -1;
	}
	Constraint* cn = Constraint::AsStruct(S, other) ;
	try
	{
		self->solver.addConstraint( cn->constraint );
	}
	catch( const kiwi::DuplicateConstraint& )
	{
		CystckErr_SetObject(S, DuplicateConstraint, other );
		return -1;
	}
	catch( const kiwi::UnsatisfiableConstraint& )
	{
		CystckErr_SetObject(S,  UnsatisfiableConstraint, other );
		return -1;
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object 
Solver_removeConstraint( Py_State *S, Cystck_Object  other )
{
	Solver* self = Solver::AsStruct(S, S->self);
	if( !Constraint::TypeCheck( S, other) ) 
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Constraint`." );
        return -1;
	}
	Constraint* cn = Constraint::AsStruct(S, other);
	try
	{
		self->solver.removeConstraint( cn->constraint );
	}
	catch( const kiwi::UnknownConstraint& )
	{
		CystckErr_SetObject(S, UnknownConstraint, other );
		return -1;
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object 
Solver_hasConstraint( Py_State *S, Cystck_Object  other )
{
	Solver* self = Solver::AsStruct(S, S->self);
	if( !Constraint::TypeCheck(S, other) ) 
	{
		CystckErr_SetString(S,  S->Cystck_TypeError, "Expected object of type `Constraint`." );
        return -1;
	}
	Constraint* cn = Constraint::AsStruct(S, other );
	Cystck_pushobject(S, Cystck_Dup(S, self->solver.hasConstraint( cn->constraint ) ? S->Cystck_True : S->Cystck_False));
	return 1;
}


Cystck_Object 
Solver_addEditVariable( Py_State *S, Cystck_Object  args )
{
	Solver* self = Solver::AsStruct(S, S->self);
	Cystck_Object pyvar;
	Cystck_Object pystrength;
	if( !CystckArg_parseTuple(S, args, "OO", &pyvar, &pystrength ) )
		return -1;
	if( !Variable::TypeCheck( S, pyvar) ) 
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Variable`." );
        return -1;
	}
	double strength;
	if( !convert_to_strength(S, pystrength, strength ) )
		return -1;
	Variable* var = Variable::AsStruct(S, pyvar);
	try
	{
		self->solver.addEditVariable( var->variable, strength );
	}
	catch( const kiwi::DuplicateEditVariable& )
	{
		CystckErr_SetObject(S, DuplicateEditVariable, pyvar );
		return -1;
	}
	catch( const kiwi::BadRequiredStrength& e )
	{
		CystckErr_SetString(S, BadRequiredStrength, e.what() );
		return -1;
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object 
Solver_removeEditVariable( Py_State *S, Cystck_Object  other )
{
	Solver* self = Solver::AsStruct(S, S->self);
	if( !Variable::TypeCheck(S,other))
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Variable`." );
        return -1;
	}
	Variable* var = TypeObject_AsCystck(Variable, other );
	try
	{
		self->solver.removeEditVariable( var->variable );
	}
	catch( const kiwi::UnknownEditVariable& )
	{
		CystckErr_SetObject(S, UnknownEditVariable,  other );
		return -1;
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}

Cystck_Object 
Solver_hasEditVariable( Py_State *S, Cystck_Object  other )
{
	Solver* self = Solver::AsStruct(S, S->self);
	if( !Variable::TypeCheck(S, other) ) 
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Variable`." );
        return -1;
	}
	Variable* var = Variable::AsStruct(S, other );
	Cystck_pushobject(S, Cystck_Dup(S, self->solver.hasEditVariable( var->variable ) ? S->Cystck_True : S->Cystck_False));
	return 1;
}


Cystck_Object 
Solver_suggestValue( Py_State *S, Cystck_Object  args )
{
	Solver* self = Solver::AsStruct(S, S->self);
	Cystck_Object pyvar;
	Cystck_Object pyvalue;
	if( !CystckArg_parseTuple(S,args, "OO", &pyvar, &pyvalue ) )
		return -1;
	if( !Variable::TypeCheck(S, pyvar) ) 
	{
		CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Variable`." );
        return -1;
	}
	double value;
	if( !convert_to_double(S, pyvalue, value ))
		return -1;
	Variable* var = Variable::AsStruct(S, pyvar);
	try
	{
		self->solver.suggestValue( var->variable, value );
	}
	catch( const kiwi::UnknownEditVariable& )
	{
		CystckErr_SetObject(S, UnknownEditVariable, pyvar );
		return -1;
	}
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object 
Solver_updateVariables( Py_State *S )
{
	Solver* self = Solver::AsStruct(S, S->self);
	self->solver.updateVariables();
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object 
Solver_reset( Py_State *S )
{
	Solver* self = Solver::AsStruct(S, S->self);
	self->solver.reset();
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}


Cystck_Object 
Solver_dump( Py_State *S )
{
	Solver* self = Solver::AsStruct(S, S->self);
	Cystck_Object dump_str =CystckUnicode_FromString(S,  self->solver.dumps().c_str() );
	Cystck_pushobject(S, dump_str);
	Cystck_Print( S, dump_str, stdout, 0 );
	Cystck_pop(S, dump_str);
	Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
	return 1;
}

Cystck_Object 
Solver_dumps( Py_State *S )
{
	Solver* self = Solver::AsStruct(S, S->self);
	Cystck_pushobject(S, CystckUnicode_FromString(S, self->solver.dumps().c_str() ));
	return 1;
}


CystckDef_METH(Solver_addConstraint_def, "addConstraint", Solver_addConstraint, Cystck_METH_O,
	"Add a constraint to the solver.")
CystckDef_METH(Solver_removeConstraint_def, "removeConstraint", Solver_removeConstraint, Cystck_METH_O,
	 "Remove a constraint from the solver.")
CystckDef_METH(Solver_hasConstraint_def, "hasConstraint", Solver_hasConstraint, Cystck_METH_O,
	 "Check whether the solver contains a constraint.")
CystckDef_METH(Solver_addEditVariable_def, "addEditVariable", Solver_addEditVariable, Cystck_METH_VARARGS,
	 "Add an edit variable to the solver.")
CystckDef_METH(Solver_removeEditVariable_def, "removeEditVariable", Solver_removeEditVariable, Cystck_METH_O,
	 "Remove an edit variable from the solver.")
CystckDef_METH(Solver_hasEditVariable_def, "hasEditVariable", Solver_hasEditVariable, Cystck_METH_O,
	 "Check whether the solver contains an edit variable.")
CystckDef_METH(Solver_suggestValue_def, "suggestValue", Solver_suggestValue, Cystck_METH_VARARGS,
	 "Suggest a desired value for an edit variable.")
CystckDef_METH(Solver_updateVariables_def, "updateVariables", Solver_updateVariables, Cystck_METH_NOARGS,
	 "Update the values of the solver variables.")
CystckDef_METH(Solver_reset_def, "reset", Solver_reset, Cystck_METH_NOARGS,
	 "Reset the solver to the initial empty starting condition.")
CystckDef_METH(Solver_dump_def, "dump", Solver_dump, Cystck_METH_NOARGS,
	 "Dump a representation of the solver internals to stdout.")
CystckDef_METH(Solver_dumps_def, "dumps", Solver_dumps, Cystck_METH_NOARGS,
	 "Dump a representation of the solver internals to a string.")


CystckDef_SLOT(Solver_new_def, Solver_new, Cystck_tp_new);
CystckDef_SLOT(Solver_dealloc_def, Solver_dealloc, Cystck_tp_dealloc);
static CystckDef* Solver_defines[] = {
	// slots
	&Solver_new_def,
	&Solver_dealloc_def,

	// methods
	&Solver_addConstraint_def,
	&Solver_removeConstraint_def,
	&Solver_hasConstraint_def,
	&Solver_addEditVariable_def,
	&Solver_removeEditVariable_def,
	&Solver_hasEditVariable_def,
	&Solver_suggestValue_def,
	&Solver_updateVariables_def,
	&Solver_reset_def,
	&Solver_dump_def,
	&Solver_dumps_def,
	NULL
};


} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
Cystck_Object  Solver::TypeObject = 0;


Cystck_Type_Spec Solver::TypeObject_Spec = {
	.name = "kiwisolver.Solver",
	.basicsize = sizeof( Solver ),
	.itemsize = 0,
	.flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_BASETYPE,
    // .legacy_slots = Solver_Type_slots,
	.m_methods = Solver_defines
};

bool Solver::Ready( Py_State *S, Cystck_Object m )
{
    // The reference will be handled by the module to which we will add the type
    if ( !CystckHelpers_AddType(S, m, "Solver", &TypeObject_Spec) ) {
        return false;
    }

    TypeObject = Cystck_GetAttr_s(S, m, "Solver" );
    if( Cystck_IsNULL( TypeObject ) )
    {
        return false;
    }
    return true;
}



Cystck_Object DuplicateConstraint;

Cystck_Object  UnsatisfiableConstraint;

Cystck_Object  UnknownConstraint;

Cystck_Object  DuplicateEditVariable;

Cystck_Object  UnknownEditVariable;

Cystck_Object  BadRequiredStrength;


bool init_exceptions( Py_State *S, Cystck_Object mod )
{
 	DuplicateConstraint = CystckErr_NewException(S, 
 		const_cast<char*>( "kiwisolver.DuplicateConstraint" ), 0, 0 );
 	if( !DuplicateConstraint )
    {
        return false;
    }

  	UnsatisfiableConstraint = CystckErr_NewException(S,
  		const_cast<char*>( "kiwisolver.UnsatisfiableConstraint" ), 0, 0 );
 	if( !UnsatisfiableConstraint )
 	{
        return false;
    }

  	UnknownConstraint = CystckErr_NewException(S,
  		const_cast<char*>( "kiwisolver.UnknownConstraint" ), 0, 0 );
 	if( !UnknownConstraint )
 	{
        return false;
    }

  	DuplicateEditVariable = CystckErr_NewException(S,
  		const_cast<char*>( "kiwisolver.DuplicateEditVariable" ), 0, 0 );
 	if( !DuplicateEditVariable )
 	{
        return false;
    }

  	UnknownEditVariable = CystckErr_NewException(S,
  		const_cast<char*>( "kiwisolver.UnknownEditVariable" ), 0, 0 );
 	if( !UnknownEditVariable )
 	{
        return false;
    }

  	BadRequiredStrength = CystckErr_NewException(S,
  		const_cast<char*>( "kiwisolver.BadRequiredStrength" ), 0, 0 );
 	if( !BadRequiredStrength )
 	{
        return false;
    }
	Cystck_SetAttr_s( S, mod, "DuplicateConstraint", DuplicateConstraint );
    Cystck_SetAttr_s( S, mod, "UnsatisfiableConstraint", UnsatisfiableConstraint );
    Cystck_SetAttr_s( S, mod, "UnknownConstraint", UnknownConstraint );
    Cystck_SetAttr_s( S, mod, "DuplicateEditVariable", DuplicateEditVariable );
    Cystck_SetAttr_s( S, mod, "UnknownEditVariable", UnknownEditVariable );
    Cystck_SetAttr_s( S, mod, "BadRequiredStrength", BadRequiredStrength );
	return true;
}

}  // namespace

/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "../../../../cystck/include/Cystck.h"
#include <kiwi/kiwi.h>

#define CystckFloat_Check( S, obj ) CystckTypeCheck(S, obj, S->Cystck_FloatType )
#define CystckLong_Check( S, obj ) CystckTypeCheck( S, obj, S->Cystck_LongType )
#define Cystck_RETURN_NOTIMPLEMENTED( S ) return Cystck_Dup(S, S->Cystck_NotImplemented)
namespace kiwisolver
{

extern Cystck_Object DuplicateConstraint;

extern Cystck_Object UnsatisfiableConstraint;

extern Cystck_Object UnknownConstraint;

extern Cystck_Object DuplicateEditVariable;

extern Cystck_Object UnknownEditVariable;

extern Cystck_Object BadRequiredStrength;


struct strength
{
	Cystck_HEAD;

    static Cystck_Type_Spec TypeObject_Spec;

    static Cystck_Object TypeObject;

	static bool Ready(Py_State *S,  Cystck_Object m);
};


struct Variable
{
	Cystck_HEAD
	Cystck_Object context;
	kiwi::Variable variable;

    static Cystck_Type_Spec TypeObject_Spec;

    static Cystck_Object TypeObject;

	static bool Ready(Py_State *S,  Cystck_Object m);

	static bool TypeCheck( Py_State *S, Cystck_Object obj )
	{
		return CystckTypeCheck(S,  obj, TypeObject ) != 0;
	}
	static Variable* AsStruct(Py_State *S, Cystck_Object obj) {
		return TypeObject_AsCystck(Variable, obj);
	}
};
CystckType_HELPERS(Variable)


struct Term
{
	Cystck_HEAD
	Cystck_Object variable;
	double coefficient;

    static Cystck_Type_Spec TypeObject_Spec;

    static Cystck_Object TypeObject;

	static bool Ready(Py_State *S,  Cystck_Object m);

	static bool TypeCheck( Py_State *S, Cystck_Object obj )
	{
		return CystckTypeCheck(S,  obj, TypeObject ) != 0;
	}
	static Term* AsStruct(Py_State *S, Cystck_Object obj) {
		return TypeObject_AsCystck(Term, obj);
	}
};

CystckType_HELPERS(Term)
struct Expression
{
	Cystck_HEAD
	Cystck_Object terms;
	double constant;

    static Cystck_Type_Spec TypeObject_Spec;

    static Cystck_Object TypeObject;

	static bool Ready(Py_State *S,  Cystck_Object m);

	static bool TypeCheck( Py_State *S, Cystck_Object obj )
	{
		return CystckTypeCheck(S,  obj, TypeObject ) != 0;
	}
	static Expression* AsStruct(Py_State *S, Cystck_Object obj) {
		return TypeObject_AsCystck(Expression, obj);
	}
};
CystckType_HELPERS(Expression)

struct Constraint
{
	Cystck_HEAD
	Cystck_Object expression;
	kiwi::Constraint constraint;

    static Cystck_Type_Spec TypeObject_Spec;

    static Cystck_Object TypeObject;

	static bool Ready(Py_State *S,  Cystck_Object m);

	static bool TypeCheck( Py_State *S, Cystck_Object obj )
	{
		return CystckTypeCheck(S,  obj, TypeObject ) != 0;
	}
	static Constraint* AsStruct(Py_State *S, Cystck_Object obj) {
		return TypeObject_AsCystck(Constraint, obj);
	}
};
CystckType_HELPERS(Constraint)


struct Solver
{
	Cystck_HEAD
	kiwi::Solver solver;

    static Cystck_Type_Spec TypeObject_Spec;

    static Cystck_Object TypeObject;

	static bool Ready(Py_State *S,  Cystck_Object m);

	static bool TypeCheck( Py_State *S, Cystck_Object obj )
	{
		return CystckTypeCheck(S,  obj, TypeObject ) != 0;
	}
	static Solver* AsStruct(Py_State *S, Cystck_Object obj) {
		return TypeObject_AsCystck(Solver, obj);
	}
};

CystckType_HELPERS(Solver)

bool init_exceptions( Py_State *S, Cystck_Object mod );


}  // namespace kiwisolver

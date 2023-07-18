/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "types.h"
#include "util.h"


namespace kiwisolver
{

template<typename Op, typename T>
struct UnaryInvoke
{
	Cystck_Object operator()( Py_State *S, Cystck_Object value )
	{
		return Op()( S, T::AsStruct(S, value ), value );
	}
};


template<typename Op, typename T>
struct BinaryInvoke
{
	Cystck_Object operator()( Py_State *S, Cystck_Object first, Cystck_Object second )
	{
		if( T::TypeCheck( S, first ) )
			return invoke<Normal>( S, T::AsStruct( S, first ), second, first, second );
		return invoke<Reverse>( S, T::AsStruct( S, second ), first, second, first );
	}

	struct Normal
	{
		template<typename U>
		Cystck_Object operator()( Py_State *S, T* primary, U secondary, Cystck_Object h_primary, Cystck_Object h_secondary )
		{
			return Op()( S, primary, secondary, h_primary, h_secondary );
		}
	};

	struct Reverse
	{
		template<typename U>
		Cystck_Object operator()( Py_State *S, T* primary, U secondary, Cystck_Object h_primary, Cystck_Object h_secondary )
		{
			return Op()( S, secondary, primary, h_secondary, h_primary );
		}
	};

	template<typename Invk>
	Cystck_Object invoke( Py_State *S, T* primary, Cystck_Object secondary, Cystck_Object h_primary, Cystck_Object h_secondary )
	{
		if( Expression::TypeCheck( S, secondary ) )
			return Invk()( S, primary, Expression_AsStruct( S, secondary ), h_primary, h_secondary );
		if( Term::TypeCheck( S, secondary ) )
			return Invk()( S, primary, Term_AsStruct( S, secondary ), h_primary, h_secondary );
		if( Variable::TypeCheck( S, secondary ) )
			return Invk()( S, primary, Variable_AsStruct( S, secondary ), h_primary, h_secondary );
		if( CystckFloat_Check( S, secondary ) )
			return Invk()( S, primary, CystckFloat_AsDouble( S, secondary ), h_primary, h_secondary );
		if( CystckLong_Check( S, secondary ) )
		{
			double v = CystckLong_AsDouble( S, secondary );
			if( v == -1 && Cystck_Err_Occurred(S) )
				return 0;
			return Invk()( S, primary, v, h_primary, 0 );
		}
		Cystck_RETURN_NOTIMPLEMENTED( S );
	}
};


struct BinaryMul
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		Cystck_RETURN_NOTIMPLEMENTED( S );
	}
};


template<> inline
Cystck_Object BinaryMul::operator()( Py_State *S, Variable* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	Term* term;
	Cystck_Object pyterm =  Cystck_New( S, Term::TypeObject, &term );
	if( Cystck_IsNULL(pyterm) )
		return 0;
	term->variable = Cystck_Dup( S, h_first );
	term->coefficient = second;
	return pyterm;
}


template<> inline
Cystck_Object BinaryMul::operator()( Py_State *S, Term* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	Term* term;
	Cystck_Object pyterm =  Cystck_New( S, Term::TypeObject, &term );
	if( Cystck_IsNULL(pyterm) )
		return 0;
	term->variable = Cystck_Dup( S, first->variable );
	term->coefficient = first->coefficient * second;
	return pyterm;
}


template<> inline
Cystck_Object BinaryMul::operator()( Py_State *S, Expression* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	Expression* expr;
	Cystck_Object pyexpr =  Cystck_New( S, Expression::TypeObject, &expr );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	Cystck_pushobject(S, pyexpr);
	CystckTupleBuilder terms = CystckTupleBuilder_New( S, Cystck_Length( S, first->terms ) );
	Cystck_ssize_t end = Cystck_Length( S, first->terms );
	for( Cystck_ssize_t i = 0; i < end; ++i )
	{
		Cystck_Object item = Cystck_GetItem_i( S, first->terms, i );
		if (Cystck_IsNULL(item))
		{
			CystckTupleBuilder_Cancel(S, terms);
			Cystck_pop(S, pyexpr);
			return 0;
		}
		Cystck_pushobject(S, item);
		Cystck_Object term = BinaryMul()( S, Term_AsStruct( S, item ), second, item, h_second );
		Cystck_pop(S, item);
		if( Cystck_IsNULL(term) ) {
			Cystck_pop(S, pyexpr);
			CystckTupleBuilder_Cancel(S, terms);
			return 0;
		}
		Cystck_pushobject(S, term);
		CystckTupleBuilder_Set( S, terms, i, term );
		Cystck_pop(S, term);
	}
	expr->terms = CystckTupleBuilder_Build( S, terms );
	if ( Cystck_IsNULL(expr->terms) )
	{
		Cystck_pop(S, pyexpr);
		return 0;
	}
	expr->constant = first->constant * second;
	return pyexpr;
}


template<> inline
Cystck_Object BinaryMul::operator()( Py_State *S, double first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


template<> inline
Cystck_Object BinaryMul::operator()( Py_State *S, double first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


template<> inline
Cystck_Object BinaryMul::operator()( Py_State *S, double first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


struct BinaryDiv
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		Cystck_RETURN_NOTIMPLEMENTED( S );
	}
};


template<> inline
Cystck_Object BinaryDiv::operator()( Py_State *S, Variable* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	if( second == 0.0 )
	{
		CystckErr_SetString( S, S->Cystck_ZeroDivisionError, "float division by zero" );
		return 0;
	}
	return BinaryMul()( S, first, 1.0 / second, h_first, 0 );
}


template<> inline
Cystck_Object BinaryDiv::operator()( Py_State *S, Term* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	if( second == 0.0 )
	{
		CystckErr_SetString( S, S->Cystck_ZeroDivisionError, "float division by zero" );
		return 0;
	}
	return BinaryMul()( S, first, 1.0 / second, h_first, 0 );
}


template<> inline
Cystck_Object BinaryDiv::operator()( Py_State *S, Expression* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	if( second == 0.0 )
	{
		CystckErr_SetString( S, S->Cystck_ZeroDivisionError, "float division by zero" );
		return 0;
	}
	return BinaryMul()( S, first, 1.0 / second, h_first, 0 );
}


struct UnaryNeg
{
	template<typename T>
	Cystck_Object operator()( Py_State *S, T value, Cystck_Object h_value )
	{
		Cystck_RETURN_NOTIMPLEMENTED( S );
	}
};


template<> inline
Cystck_Object UnaryNeg::operator()( Py_State *S, Variable* value, Cystck_Object h_value )
{
	return BinaryMul()( S, value, -1.0, h_value, 0 );
}


template<> inline
Cystck_Object UnaryNeg::operator()( Py_State *S, Term* value, Cystck_Object h_value )
{
	return BinaryMul()( S, value, -1.0, h_value, 0 );
}


template<> inline
Cystck_Object UnaryNeg::operator()( Py_State *S, Expression* value, Cystck_Object h_value )
{
	return BinaryMul()( S, value, -1.0, h_value, 0 );
}


struct BinaryAdd
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		Cystck_RETURN_NOTIMPLEMENTED( S );
	}
};


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Expression* first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Expression* expr;
	Cystck_Object pyexpr = Cystck_New( S, Expression::TypeObject, &expr );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	Cystck_pushobject(S, pyexpr);
	expr->constant = first->constant + second->constant;
	Cystck_ssize_t first_len = Cystck_Length( S, first->terms );
	Cystck_ssize_t second_len = Cystck_Length( S, second->terms );
	CystckTupleBuilder terms = CystckTupleBuilder_New( S, first_len + second_len );
	for( Cystck_ssize_t i = 0; i < first_len; ++i )
	{
		Cystck_Object item = Cystck_GetItem_i( S, first->terms, i );
		if ( Cystck_IsNULL(item) )
		{
			Cystck_pop(S, pyexpr);
			CystckTupleBuilder_Cancel(S, terms);
			return 0;
		}
		Cystck_pushobject(S, item);
		CystckTupleBuilder_Set( S, terms, i, Cystck_Dup( S, item ) );
		Cystck_pop(S, item);
	}
	for( Cystck_ssize_t j = 0; j < second_len; ++j )
	{
		Cystck_Object item = Cystck_GetItem_i( S, second->terms, j );
		if ( Cystck_IsNULL(item) )
		{
			Cystck_pop(S, pyexpr);
			CystckTupleBuilder_Cancel(S, terms);
			return 0;
		}
		Cystck_pushobject(S, item);
		CystckTupleBuilder_Set( S, terms, j + first_len, item );
		Cystck_pop(S, item);
	}
	expr->terms = CystckTupleBuilder_Build( S, terms );
	if( Cystck_IsNULL(expr->terms) )
	{
		Cystck_pop(S, pyexpr);
		return 0;
	}
		
	return pyexpr;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Expression* first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Expression* expr;
	Cystck_Object pyexpr =  Cystck_New( S, Expression::TypeObject, &expr );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	Cystck_pushobject(S, pyexpr);
	CystckTupleBuilder terms = CystckTupleBuilder_New( S, Cystck_Length( S, first->terms ) + 1 );
	Cystck_ssize_t end = Cystck_Length( S, first->terms );
	for( Cystck_ssize_t i = 0; i < end; ++i )
	{
		Cystck_Object item = Cystck_GetItem_i( S, first->terms, i );
		if( Cystck_IsNULL(item) ) {
			Cystck_pop(S, pyexpr);
			CystckTupleBuilder_Cancel( S, terms );
			return 0;
		}
		Cystck_pushobject(S, item);
		CystckTupleBuilder_Set( S, terms, i, Cystck_Dup( S, item ) );
		Cystck_pop(S, item);
	}
	CystckTupleBuilder_Set( S, terms, end, Cystck_Dup( S, h_second ) );
	expr->terms = CystckTupleBuilder_Build( S, terms );
	if ( Cystck_IsNULL(expr->terms))
	{
		Cystck_pop(S, pyexpr);
		return 0;
	}
	expr->constant = first->constant;
	return pyexpr;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Expression* first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = BinaryMul()( S, second, 1.0, h_second, 0 );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result =  operator()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Expression* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	Expression* expr;
	Cystck_Object pyexpr =  Cystck_New( S, Expression::TypeObject, &expr );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	expr->terms = Cystck_Dup( S, first->terms );
	expr->constant = first->constant + second;
	return pyexpr;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Term* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	Expression* expr;
	Cystck_Object pyexpr =  Cystck_New( S, Expression::TypeObject, &expr );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	Cystck_pushobject(S, pyexpr);
	expr->constant = second;
	Cystck_Object items[] = {
		h_first,
	};
	expr->terms = CystckTuple_FromArray( S, items, 1 );
	if( Cystck_IsNULL(expr->terms) )
	{
		Cystck_pop(S, pyexpr);
		return 0;
	}
	return pyexpr;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Term* first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Term* first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Expression* expr;
	Cystck_Object pyexpr =  Cystck_New( S, Expression::TypeObject, &expr );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	Cystck_pushobject(S, pyexpr);
	expr->constant = 0.0;
	Cystck_Object items[] = {
		h_first,
		h_second
	};
	expr->terms = CystckTuple_FromArray( S, items, 2 );
	if( Cystck_IsNULL(expr->terms) )
	{
		Cystck_pop(S, pyexpr);
		return 0;
	}
	return pyexpr;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Term* first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = BinaryMul()( S, second, 1.0, h_second, 0 );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Variable* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = BinaryMul()( S, first, 1.0, h_first, 0 );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = operator()( S, Term_AsStruct( S, temp ), second, temp, h_second );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Variable* first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = BinaryMul()( S, first, 1.0, h_first, 0 );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = operator()( S, Term_AsStruct( S, temp ), second, temp, h_second );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Variable* first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = BinaryMul()( S, first, 1.0, h_first, 0 );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = operator()( S, Term_AsStruct( S, temp ), second, temp, h_second );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, Variable* first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = BinaryMul()( S, first, 1.0, h_first, 0 );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = operator()( S, Term_AsStruct( S, temp ), second, temp, h_second );
	Cystck_pop(S, temp);
	return result; 
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, double first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, double first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


template<> inline
Cystck_Object BinaryAdd::operator()( Py_State *S, double first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	return operator()( S, second, first, h_second, h_first );
}


struct BinarySub
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		Cystck_RETURN_NOTIMPLEMENTED( S );
	}
};


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Variable* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	return BinaryAdd()( S, first, -second, h_first, 0 );
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Variable* first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Variable* first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Variable* first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
		
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Expression_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Term* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	return BinaryAdd()( S, first, -second, h_first, 0 );
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Term* first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Term* first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Term* first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Expression_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Expression* first, double second, Cystck_Object h_first, Cystck_Object h_second )
{
	return BinaryAdd()( S, first, -second, h_first, 0 );
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Expression* first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Expression* first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, Expression* first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Expression_AsStruct( S, temp ), h_first, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, double first, Variable* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), 0, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, double first, Term* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Term_AsStruct( S, temp ), 0, temp );
	Cystck_pop(S, temp);
	return result;
}


template<> inline
Cystck_Object BinarySub::operator()( Py_State *S, double first, Expression* second, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object temp = UnaryNeg()( S, second, h_second );
	if( Cystck_IsNULL(temp) )
		return 0;
	Cystck_pushobject(S, temp);
	Cystck_Object result = BinaryAdd()( S, first, Expression_AsStruct( S, temp ), 0, temp );
	Cystck_pop(S, temp);
	return result;
}


template<typename T, typename U>
Cystck_Object makecn( Py_State *S, T first, U second, kiwi::RelationalOperator op, Cystck_Object h_first, Cystck_Object h_second )
{
	Cystck_Object pyexpr = BinarySub()( S, first, second, h_first, h_second );
	if( Cystck_IsNULL(pyexpr) )
		return 0;
	Cystck_pushobject(S, pyexpr);
	Cystck_Object _pycn =  CystckType_GenericNew( S, Constraint::TypeObject, 0, 0 );
	Cystck_Object pycn = Cystck_FromPyObject(S, GetResults(S, _pycn));
	if( Cystck_IsNULL(pycn) )
	{
		Cystck_pop(S, pyexpr);
		return 0;
	}
	Constraint* cn = Constraint_AsStruct( S, pycn );
	cn->expression = reduce_expression( S, pyexpr );
	Cystck_pop(S, pyexpr);
	Cystck_pushobject(S, pycn);	
	if( Cystck_IsNULL(cn->expression) )
	{
		Cystck_pop(S, pycn);
		return 0;
	}
	kiwi::Expression expr( convert_to_kiwi_expression( S, cn->expression ) );
	new( &cn->constraint ) kiwi::Constraint( expr, op, kiwi::strength::required );
	return pycn;
}


struct CmpEQ
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		return makecn( S, first, second, kiwi::OP_EQ, h_first, h_second );
	}
};


struct CmpLE
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		return makecn( S, first, second, kiwi::OP_LE, h_first, h_second );
	}
};


struct CmpGE
{
	template<typename T, typename U>
	Cystck_Object operator()( Py_State *S, T first, U second, Cystck_Object h_first, Cystck_Object h_second )
	{
		return makecn( S, first, second, kiwi::OP_GE, h_first, h_second );
	}
};


}  // namespace kiwisolver

/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <Python.h>
#include <map>
#include <string>
//#include <cppy/cppy.h>
#include <kiwi/kiwi.h>
#include "types.h"


namespace kiwisolver
{

inline bool
convert_to_double(Py_State *S, Cystck_Object obj, double& out )
{
    if( CystckFloat_Check(S,  obj ) )
    {
        out = CystckFloat_AsDouble( S, obj );
        return true;
    }
    if( CystckLong_Check(S, obj ) )
    {
        out = CystckLong_AsDouble(S, obj );
        if( out == -1.0 && Cystck_Err_Occurred(S) )
            return false;
        return true;
    }
    CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `float, int, or long`.");
    return false;
}


inline bool
convert_pystr_to_str(Py_State *S, Cystck_Object value, std::string& out )
{
    out = CystckUnicode_AsUTF8AndSize(S, value, NULL );
    return true;
}


inline bool
convert_to_strength(Py_State *S, Cystck_Object value, double& out )
{
    if( CystckUnicode_Check(S, value ) )
    {
        std::string str;
        if( !convert_pystr_to_str(S, value, str ) )
            return false;
        if( str == "required" )
            out = kiwi::strength::required;
        else if( str == "strong" )
            out = kiwi::strength::strong;
        else if( str == "medium" )
            out = kiwi::strength::medium;
        else if( str == "weak" )
            out = kiwi::strength::weak;
        else
        {
            CystckErr_SetString(S,
                S->Cystck_ValueError,
                "string strength must be 'required', 'strong', 'medium', "
                "or 'weak'"
            );
            return false;
        }
        return true;
    }
    if( !convert_to_double(S, value, out ) )
        return false;
    return true;
}


inline bool
convert_to_relational_op(Py_State *S, Cystck_Object value, kiwi::RelationalOperator& out )
{
    if( !CystckUnicode_Check(S, value ) )
    {
        CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `str`.");
        return false;
    }
    std::string str;
    if( !convert_pystr_to_str(S, value, str ) )
        return false;
    if( str == "==" )
        out = kiwi::OP_EQ;
    else if( str == "<=" )
        out = kiwi::OP_LE;
    else if( str == ">=" )
        out = kiwi::OP_GE;
    else
    {
        CystckErr_SetString(S, S->Cystck_ValueError, "relational operator must be '==', '<=', or '>='" );
        return false;
    }
    return true;
}


inline Cystck_Object
make_terms(Py_State *S, const std::map<Cystck_Object*, double>& coeffs )
{
    typedef std::map<Cystck_Object*, double>::const_iterator iter_t;
    CystckTupleBuilder terms = CystckTupleBuilder_New(S,  coeffs.size() );
    Cystck_ssize_t i = 0;
    iter_t it = coeffs.begin();
    iter_t end = coeffs.end();
    for( ; it != end; ++it, ++i )
    {
        Term* term;
        Cystck_Object pyterm = Cystck_New(S, Term::TypeObject, &term);
        if ( Cystck_IsNULL(pyterm) )
        {
            CystckTupleBuilder_Cancel(S, terms);
            return 0;
        }
        Cystck_pushobject(S, pyterm);
        term->variable = Cystck_Dup(S, *(it->first) );
        term->coefficient = it->second;
        CystckTupleBuilder_Set(S, terms, i, pyterm );
        Cystck_pop(S, pyterm);
    }
    return CystckTupleBuilder_Build(S, terms );
}


inline Cystck_Object
reduce_expression( Py_State *S, Cystck_Object pyexpr )  // pyexpr must be an Expression
{
    Expression* expr = Expression::AsStruct(S, pyexpr);
    std::map<Cystck_Object*, double> coeffs;
    Cystck_ssize_t size = Cystck_Length( S, expr->terms );
    // bool was_error = false;
    for( Cystck_ssize_t i = 0; i < size; ++i )
    {
        Cystck_Object item = Cystck_GetItem_i( S, expr->terms, i );
        Term* term = Term::AsStruct( S, item );
        coeffs[ &term->variable ] += term->coefficient;
    }
    Cystck_Object terms = make_terms( S, coeffs );
    if( Cystck_IsNULL(terms) )
    {
        Cystck_DECREF(S, terms);
        return 0;
    }    
    Expression* newexpr;
    Cystck_Object pynewexpr = Cystck_New(S, Expression::TypeObject, &newexpr);
    if( Cystck_IsNULL(pynewexpr) )
        return 0;
    newexpr->terms = terms;
    newexpr->constant = expr->constant;
    return pynewexpr;
}


inline kiwi::Expression
convert_to_kiwi_expression(Py_State *S, Cystck_Object pyexpr )  // pyexpr must be an Expression
{
    Expression* expr = Expression::AsStruct( S, pyexpr );
    std::vector<kiwi::Term> kterms;
    Cystck_ssize_t size = Cystck_Length( S, expr->terms );
    for( Cystck_ssize_t i = 0; i < size; ++i )
    {
        Cystck_Object item = Cystck_GetItem_i( S, expr->terms, i );
        Cystck_pushobject(S, item);
        Term* term = Term::AsStruct( S, item );
        Variable* var = Variable::AsStruct( S, term->variable );
        kterms.push_back( kiwi::Term( var->variable, term->coefficient ) );
        Cystck_pop(S, item);
    }
    return kiwi::Expression( kterms, expr->constant );
}


inline const char*
pyop_str( int op )
{
    switch( op )
    {
        case Py_LT:
            return "<";
        case Py_LE:
            return "<=";
        case Py_EQ:
            return "==";
        case Py_NE:
            return "!=";
        case Py_GT:
            return ">";
        case Py_GE:
            return ">=";
        default:
            return "";
    }
}


}  // namespace kiwisolver

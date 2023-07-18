/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <sstream>
#include "../../../../cystck/include/Cystck.h"
#include "symbolics.h"
#include "types.h"
#include "util.h"


namespace kiwisolver
{

namespace
{

Cystck_Object 
Expression_new( Py_State* S, Cystck_Object type, Cystck_Object  args, Cystck_Object  kwargs )
{
    static const char *kwlist[] = { "terms", "constant", 0 };
    Cystck_Object  pyterms;
    Cystck_Object pyconstant = 0;
    if( !CystckArg_parseTupleAndKeywords(S,
        args, kwargs, "O|O:__new__", (const char **) kwlist,
        &pyterms, &pyconstant ) )
        return -1;
    Cystck_ssize_t end = Cystck_Length(S, pyterms);
    CystckTupleBuilder terms =  CystckTupleBuilder_New(S, end);
    for ( Cystck_ssize_t i =0; i<end; ++i )
    {
        Cystck_Object item = Cystck_GetItem_i(S, pyterms, i);
        if ( Cystck_IsNULL(item) || !Term::TypeCheck(S,  item))
        {
            CystckErr_SetString(S, S->Cystck_TypeError, "Expected object of type `Term`.");
            Cystck_DECREF(S, item);
            CystckTupleBuilder_Cancel(S, terms);
            return -1;
        }
        Cystck_pushobject(S, item);
        CystckTupleBuilder_Set(S, terms, i, item);
        Cystck_pop(S, item);
    }
    double constant = 0.0;
    if( !Cystck_IsNULL(pyconstant) && !convert_to_double(S,pyconstant, constant ) )
    {
        CystckTupleBuilder_Cancel(S, terms);
        return -1;
    }
    Expression *self;
    Cystck_Object pyexpr = Cystck_New(S, type, &self);
    self->terms =  CystckTupleBuilder_Build(S, terms);
    if(Cystck_IsNULL(self->terms))
    {
        Cystck_DECREF(S, pyexpr);
        return -1;
    }
        
    self->constant = constant;
    Cystck_pushobject(S, pyexpr);
    return 1;
}


int
Expression_traverse( Expression* self, visitproc visit, void* arg )
{
    Py_VISIT( Cystck_AsPyObject(NULL, self->terms) );
    return 0;
}



Cystck_Object 
Expression_repr(Py_State *S, Cystck_Object _self  )
{
    Expression* self = Expression::AsStruct(S, _self);
    std::stringstream stream;
    Cystck_ssize_t end = Cystck_Length(S,  self->terms);
    for( Cystck_ssize_t i = 0; i < end; ++i )
    {
        Cystck_Object item = Cystck_GetItem_i(S, self->terms, i );
        if ( Cystck_IsNULL( item ) )
        {
            return -1;
        }
        Cystck_pushobject(S, item);
        Term* term = Term::AsStruct(S, item);
        Cystck_pop(S, item);
        stream << term->coefficient << " * ";
        stream << Variable::AsStruct( S, term->variable )->variable.name();
        stream << " + ";
    }
    stream << self->constant;
    Cystck_pushobject(S, CystckUnicode_FromString(S, stream.str().c_str() ));
    return 1;
}


Cystck_Object 
Expression_terms( Py_State *S )
{
    Expression* self = Expression::AsStruct(S, S->self);
    Cystck_pushobject(S, Cystck_Dup(S, self->terms));
    return 1;
}


Cystck_Object 
Expression_constant( Py_State *S )
{
    Expression* self = Expression::AsStruct(S, S->self);
    Cystck_pushobject(S, CystckFloat_FromDouble(S,  self->constant ));
    return 1;
}


Cystck_Object 
Expression_value( Py_State *S )
{
    Expression* self = Expression::AsStruct(S, S->self);
    double result = self->constant;
    Cystck_ssize_t size = Cystck_Length(S,  self->terms);
    for( Cystck_ssize_t i = 0; i < size; ++i )
    {
        Cystck_Object item = Cystck_GetItem_i(S,  self->terms, i );
        if (Cystck_IsNULL(item))
        {
            return 1;
        }
        Cystck_pushobject(S, item);
        Term* term = Term::AsStruct(S, item);
        Variable* pyvar = Variable::AsStruct(S, term->variable );
        result += term->coefficient * pyvar->variable.value();
        Cystck_pop(S, item);
    }
    Cystck_pushobject(S, CystckFloat_FromDouble(S, result ) );
    return 1;
}


Cystck_Object 
Expression_add(Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S,  BinaryInvoke<BinaryAdd, Expression>()(S, first, second) );
	return 1;    
}


Cystck_Object 
Expression_sub(Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinarySub, Expression>()( S, first, second) );
	return 1;
}


Cystck_Object 
Expression_mul( Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryMul, Expression>()( S, first, second) );
	return 1;
}


Cystck_Object 
Expression_div( Py_State *S, Cystck_Object  first, Cystck_Object  second )
{
	Cystck_pushobject(S, BinaryInvoke<BinaryDiv, Expression>()(S,  first, second) );
	return 1;
}


Cystck_Object 
Expression_neg(Py_State *S,  Cystck_Object  value )
{
	Cystck_pushobject(S, UnaryInvoke<UnaryNeg, Expression>()(S, value) );
	return 1;
}


Cystck_Object 
Expression_richcmp( Py_State *S, Cystck_Object  first, Cystck_Object  second, int op )
{
	switch( op )
	{
		case Cystck_EQ:
			Cystck_pushobject(S, BinaryInvoke<CmpEQ, Expression>()(S, first, second) );
			return 1;
		case Cystck_LE:
			Cystck_pushobject(S, BinaryInvoke<CmpLE,  Expression>()(S,  first, second) );
			return 1;
		case Cystck_GE:
			Cystck_pushobject(S,  BinaryInvoke<CmpGE,  Expression>()(S,  first, second) );
			return 1;
		default:
			break;
	}
	CystckErr_SetString(S, S->Cystck_TypeError, "unsupported operand type(s)" );
	return -1;
}
CystckDef_METH(Expression_terms_def, "terms", Expression_terms, Cystck_METH_NOARGS,
     "Get the tuple of terms for the expression.")
CystckDef_METH(Expression_constant_def, "constant", Expression_constant, Cystck_METH_NOARGS,
    "Get the constant for the expression.")
CystckDef_METH(Expression_value_def, "value", Expression_value, Cystck_METH_NOARGS,
     "Get the value for the expression.")


CystckDef_SLOT(Expression_repr_def, Expression_repr, Cystck_tp_repr)              /* tp_repr */
CystckDef_SLOT(Expression_richcmp_def, Expression_richcmp, Cystck_tp_richcompare) /* tp_richcompare */
CystckDef_SLOT(Expression_new_def, Expression_new, Cystck_tp_new)                 /* tp_new */
CystckDef_SLOT(Expression_add_def, Expression_add, Cystck_nb_add)                 /* nb_add */
CystckDef_SLOT(Expression_sub_def, Expression_sub, Cystck_nb_subtract)            /* nb_sub */
CystckDef_SLOT(Expression_mul_def, Expression_mul, Cystck_nb_multiply)            /* nb_mul */
CystckDef_SLOT(Expression_neg_def, Expression_neg, Cystck_nb_negative)            /* nb_neg */
CystckDef_SLOT(Expression_div_def, Expression_div, Cystck_nb_true_divide)         /* nb_div */


static CystckDef* Expression_defines[] = {
    // slots
    &Expression_repr_def,
    &Expression_richcmp_def,
    &Expression_new_def,
    &Expression_add_def,
    &Expression_sub_def,
    &Expression_mul_def,
    &Expression_neg_def,
    &Expression_div_def,
    // &Expression_clear_def,

    // methods
    &Expression_terms_def,
    &Expression_constant_def,
    &Expression_value_def,
    NULL
};
static PyType_Slot Expression_Type_slots[] = {
    { Py_tp_traverse, reinterpret_cast<void*>(  Expression_traverse ) },    /* tp_traverse */
    { 0, 0 },
};


} // namespace


// Initialize static variables (otherwise the compiler eliminates them)
Cystck_Object Expression::TypeObject = 0;


Cystck_Type_Spec Expression::TypeObject_Spec = {
	.name = "kiwisolver.Expression",
	.basicsize = sizeof( Expression ),
	.itemsize = 0,
	.flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_HAVE_GC | Cystck_TPFLAGS_BASETYPE,
    .legacy_slots = Expression_Type_slots,
    .m_methods = Expression_defines
};

bool Expression::Ready(Py_State *S, Cystck_Object m)
{
    // The reference will be handled by the module to which we will add the type
    if (!CystckHelpers_AddType(S, m, "Expression", &TypeObject_Spec)) {
        return false;
    }

    TypeObject = Cystck_GetAttr_s(S, m, "Expression");
    if( Cystck_IsNULL(TypeObject) )
    {
        return false;
    }
    return true;
}

}  // namesapce kiwisolver

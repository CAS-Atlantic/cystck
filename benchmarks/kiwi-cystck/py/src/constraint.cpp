/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "../../../../cystck/include/Cystck.h"
#include <algorithm>
#include <sstream>
#include <kiwi/kiwi.h>
#include "types.h"
#include "util.h"

namespace kiwisolver
{

namespace
{

Cystck_Object 
Constraint_new( Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kwargs)
{
    static const char *kwlist[] = {"expression", "op", "strength", 0};
    Cystck_Object pyexpr;
    Cystck_Object pyop;
    Cystck_Object pystrength = 0;
    if (!CystckArg_parseTupleAndKeywords(S,
            args, kwargs, "OO|O:__new__", (const char **)kwlist,
            &pyexpr, &pyop, &pystrength))
        return -1;
    if (!Expression::TypeCheck(S, pyexpr))
    {
        CystckErr_SetString(S,  S->Cystck_TypeError, "Expected object of type `Expression`." );
        return -1;
   }
    kiwi::RelationalOperator op;
    if (!convert_to_relational_op(S, pyop, op))
        return -1;
    double strength = kiwi::strength::required;
    if (!(Cystck_IsNULL(pystrength ))&& !convert_to_strength(S, pystrength, strength))
        return -1;
    Constraint *cn;
    Cystck_Object pycn = Cystck_New(S, type, &cn);
    if (Cystck_IsNULL(pycn))
        return -1;
    Cystck_pushobject(S, pycn);
    cn->expression = reduce_expression(S, pyexpr);
    if (Cystck_IsNULL(cn->expression))
    {
        Cystck_pop(S, pycn);
        return -1;
    }
    Cystck_pushobject(S, cn->expression);
    kiwi::Expression expr(convert_to_kiwi_expression(S, cn->expression));
    Cystck_pop(S, cn->expression);
    new (&cn->constraint) kiwi::Constraint(expr, op, strength);
    return 1;
}


int Constraint_traverse(Constraint *self, visitproc visit, void *arg)
{
    Py_VISIT(Cystck_AsPyObject(NULL,self->expression));
    return 0;
}

void Constraint_dealloc( Py_State *S)
{
    
    Constraint *self = Constraint_AsStruct(S, S->self);
    self->constraint.~Constraint();
}

Cystck_Object 
Constraint_repr(Py_State *S, Cystck_Object h_self)
{
    Constraint *self = TypeObject_AsCystck(Constraint, h_self);
    std::stringstream stream;
    Expression *expr = Expression::AsStruct(S, self->expression);
    Cystck_ssize_t size = Cystck_Length(S, expr->terms);
    for (Cystck_ssize_t i = 0; i < size; ++i)
    {
        Cystck_Object item = Cystck_GetItem_i(S, expr->terms, i);
        if (Cystck_IsNULL(item))
        {
            return -1;
        }
        Cystck_pushobject(S, item);
        Term *term = Term::AsStruct(S, item);
        stream << term->coefficient << " * ";
        stream << Variable::AsStruct(S, term->variable)->variable.name();
        stream << " + ";
        Cystck_pop(S, item);
    }
    stream << expr->constant;
    switch (self->constraint.op())
    {
    case kiwi::OP_EQ:
        stream << " == 0";
        break;
    case kiwi::OP_LE:
        stream << " <= 0";
        break;
    case kiwi::OP_GE:
        stream << " >= 0";
        break;
    }
    stream << " | strength = " << self->constraint.strength();
    Cystck_pushobject(S, CystckUnicode_FromString(S, stream.str().c_str()));
    return 1;
}

Cystck_Object 
Constraint_expression(Py_State *S)
{
    Constraint *self = Constraint_AsStruct(S, S->self);
    Cystck_pushobject(S, Cystck_Dup(S, self->expression));
    return 1;
}

Cystck_Object 
Constraint_op(Py_State *S)
{
    Constraint *self = TypeObject_AsCystck(Constraint, S->self);
    switch (self->constraint.op())
    {
    case kiwi::OP_EQ:
        Cystck_pushobject(S, CystckUnicode_FromString(S, "=="));
        break;
    case kiwi::OP_LE:
        Cystck_pushobject(S, CystckUnicode_FromString(S, "<="));
        break;
    case kiwi::OP_GE:
        Cystck_pushobject(S, CystckUnicode_FromString(S, ">="));
        break;
    }
    return 1;
}

Cystck_Object 
Constraint_strength(Py_State *S)
{
    Constraint *self = Constraint::AsStruct(S, S->self);
    Cystck_pushobject(S, CystckFloat_FromDouble(S, self->constraint.strength()));
    return 1;
}


Cystck_Object 
Constraint_or( Py_State *S, Cystck_Object pyoldcn, Cystck_Object value)
{
    if (!Constraint::TypeCheck( S, pyoldcn))
        std::swap(pyoldcn, value);
    double strength;
    if (!convert_to_strength(S, value, strength))
        return -1;
    Constraint *newcn;
    Cystck_Object pynewcn =Cystck_New(S, Constraint::TypeObject, &newcn);
    if (Cystck_IsNULL(pynewcn))
        return -1;
    Constraint *oldcn = Constraint::AsStruct(S, pyoldcn);
    newcn->expression = Cystck_Dup(S, oldcn->expression);
    new (&newcn->constraint) kiwi::Constraint(oldcn->constraint, strength);
    Cystck_pushobject(S, pynewcn);
    return 1;
}

CystckDef_METH(Constraint_expression_def, "expression", Constraint_expression, Cystck_METH_NOARGS,
"Get the expression object for the constraint.")
CystckDef_METH(Constraint_op_def, "op", Constraint_op, Cystck_METH_NOARGS,
 "Get the relational operator for the constraint.")
CystckDef_METH(Constraint_strength_def, "strength", Constraint_strength, Cystck_METH_NOARGS,
 "Get the strength for the constraint.")


CystckDef_SLOT(Constraint_dealloc_def, Constraint_dealloc, Cystck_tp_dealloc)
// CystckDef_SLOT(Constraint_traverse_def, Constraint_traverse, Cystck_tp_traverse)
CystckDef_SLOT(Constraint_repr_def, Constraint_repr, Cystck_tp_repr)
CystckDef_SLOT(Constraint_new_def, Constraint_new, Cystck_tp_new)
CystckDef_SLOT(Constraint_or_def, Constraint_or, Cystck_nb_or)

static CystckDef* Constraint_defines[] = {
    // slots
    &Constraint_dealloc_def,
    // &Constraint_traverse_def,
    &Constraint_repr_def,
    &Constraint_new_def,
    &Constraint_or_def,

    // methods
    &Constraint_expression_def,
    &Constraint_op_def,
    &Constraint_strength_def,
    NULL
};

static PyType_Slot Constraint_Type_slots[] = {
    {Cystck_tp_traverse, reinterpret_cast<void*>(Constraint_traverse)}, /* tp_traverse */
    {0, 0},
};


} // namespace

// Initialize static variables (otherwise the compiler eliminates them)
Cystck_Object Constraint::TypeObject = 0;

Cystck_Type_Spec Constraint::TypeObject_Spec = {
    .name = "kiwisolver.Constraint", /* tp_name */
    .basicsize = sizeof(Constraint),      /* tp_basicsize */
    .itemsize = 0,                       /* tp_itemsize */
    .flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_HAVE_GC | Cystck_TPFLAGS_BASETYPE,
    .legacy_slots = Constraint_Type_slots,
    .m_methods = Constraint_defines,    /* slots */
};

bool Constraint::Ready(Py_State *S, Cystck_Object m)
{
    // The reference will be handled by the module to which we will add the type
    if (!CystckHelpers_AddType(S, m, "Constraint", &TypeObject_Spec)) {
        return false;
    }

    TypeObject = Cystck_GetAttr_s(S, m, "Constraint");
    if( Cystck_IsNULL(TypeObject) )
    {
        return false;
    }
    return true;
}

} // namespace kiwisolver

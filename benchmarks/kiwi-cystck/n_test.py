# -*- coding: utf-8 -*-
#------------------------------------------------------------------------------
# Copyright (c) 2014-2018, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
import gc
import re
import sys
import pytest

from kiwisolvercystck import Constraint, Variable, strength


"""Test the variable modification methods."""
v = Variable()
assert v.name() == ""
v.setName("γ")
assert v.name() == "γ"
v.setName("foo")
assert v.name() == "foo"
with pytest.raises(TypeError):
    v.setName(1)  # type: ignore
if sys.version_info >= (3,):
    with pytest.raises(TypeError):
        v.setName(b"r")  # type: ignore

assert v.value() == 0.0

assert v.context() is None
ctx = object()
v.setContext(ctx)
assert v.context() is ctx

assert str(v) == "foo"

# with pytest.raises(TypeError):
Variable(1)  # type: ignore


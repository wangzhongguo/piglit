# coding=utf-8
#
# Copyright © 2011 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# This source file defines a set of test vectors that can be used to
# test GLSL's built-in functions and operators.  It is intended to be
# used by Python code that generates Piglit tests.
#
# The key export is the dictionary test_suite.  It contains an entry
# for each possible overload of every pure built-in function and
# operator.  By iterating through this dictionary you can find a set
# of test vectors for testing nearly every built-in GLSL function.
#
# The following functions are not included, since they are not pure,
# so they can't be tested using simple vectors:
# - dFdx()
# - dFdy()
# - fwidth()
# - ftransform()
# - Increment and decrement operators
#
# The following functions are not included, since they need to be
# tested in specialized ways:
# - modf(): not tested because it has an out parameter
# - isnan() and isinf(): not tested because special effort is required
#   to create values that cause these functions to return true.
#
# Also not tested are array subscripting, field/method selection,
# swizzling, the function call operator, assignment, and the sequence
# operator.

import collections
import itertools
import functools
import warnings

import numpy as np


# Floating point types used by Python and numpy
FLOATING_TYPES = (float, np.float64, np.float32)

# Due to a bug in the Windows implementation of numpy, there are
# multiple int32 types (and multiple uint32 types).  So we have to
# find them all when doing isinstance checks.  The following code will
# create two-element tuples on numpy implementations that have the
# bug, and one-element tuples on numpy implementations that don't.
INT32_TYPES = tuple(set([np.int32, type(np.abs(np.int32(1)))]))
UINT32_TYPES = tuple(set([np.uint32,
                          type(np.dot(np.uint32(0), np.uint32(0)))]))


@functools.total_ordering
class GlslBuiltinType(object):
    """Class representing a GLSL built-in type."""
    def __init__(self, name, base_type, num_cols, num_rows,
                 version_introduced):
        self.__name = name
        if base_type is not None:
            self.__base_type = base_type
        else:
            self.__base_type = self
        self.__num_cols = num_cols
        self.__num_rows = num_rows
        self.__version_introduced = version_introduced

    @property
    def name(self):
        """The name of the type, as a string."""
        return self.__name

    @property
    def base_type(self):
        """For vectors and matrices, the type of data stored in each
        element.  For scalars, equal to self.
        """
        return self.__base_type

    @property
    def num_cols(self):
        """For matrices, the number of columns.  For vectors and
        scalars, 1.
        """
        return self.__num_cols

    @property
    def num_rows(self):
        """For vectors and matrices, the number of rows.  For scalars,
        1.
        """
        return self.__num_rows

    @property
    def is_scalar(self):
        return self.__num_cols == 1 and self.__num_rows == 1

    @property
    def is_vector(self):
        return self.__num_cols == 1 and self.__num_rows != 1

    @property
    def is_matrix(self):
        return self.__num_cols != 1

    @property
    def version_introduced(self):
        """The earliest version of GLSL that this type appears in (as
        a string, e.g. 110).
        """
        return self.__version_introduced

    def __eq__(self, other):
        if isinstance(other, GlslBuiltinType):
            return self.name == other.name

        return NotImplemented

    def __lt__(self, other):
        if isinstance(other, GlslBuiltinType):
            return self.name < other.name

        return NotImplemented

    def __hash__(self):
        return hash('__GLslBuiltinType_{}__'.format(self.name))

    def __str__(self):
        return self.__name

    def __repr__(self):
        return 'glsl_{0}'.format(self.__name)


# Concrete declarations of GlslBuiltinType
glsl_bool   = GlslBuiltinType('bool',   None,       1, 1, 110)
glsl_int    = GlslBuiltinType('int',    None,       1, 1, 110)
glsl_uint   = GlslBuiltinType('uint',   None,       1, 1, 130)
glsl_float  = GlslBuiltinType('float',  None,       1, 1, 110)
glsl_vec2   = GlslBuiltinType('vec2',   glsl_float, 1, 2, 110)
glsl_vec3   = GlslBuiltinType('vec3',   glsl_float, 1, 3, 110)
glsl_vec4   = GlslBuiltinType('vec4',   glsl_float, 1, 4, 110)
glsl_bvec2  = GlslBuiltinType('bvec2',  glsl_bool,  1, 2, 110)
glsl_bvec3  = GlslBuiltinType('bvec3',  glsl_bool,  1, 3, 110)
glsl_bvec4  = GlslBuiltinType('bvec4',  glsl_bool,  1, 4, 110)
glsl_ivec2  = GlslBuiltinType('ivec2',  glsl_int,   1, 2, 110)
glsl_ivec3  = GlslBuiltinType('ivec3',  glsl_int,   1, 3, 110)
glsl_ivec4  = GlslBuiltinType('ivec4',  glsl_int,   1, 4, 110)
glsl_uvec2  = GlslBuiltinType('uvec2',  glsl_uint,  1, 2, 130)
glsl_uvec3  = GlslBuiltinType('uvec3',  glsl_uint,  1, 3, 130)
glsl_uvec4  = GlslBuiltinType('uvec4',  glsl_uint,  1, 4, 130)
glsl_mat2   = GlslBuiltinType('mat2',   glsl_float, 2, 2, 110)
glsl_mat3   = GlslBuiltinType('mat3',   glsl_float, 3, 3, 110)
glsl_mat4   = GlslBuiltinType('mat4',   glsl_float, 4, 4, 110)
glsl_mat2x2 = glsl_mat2
glsl_mat3x2 = GlslBuiltinType('mat3x2', glsl_float, 3, 2, 120)
glsl_mat4x2 = GlslBuiltinType('mat4x2', glsl_float, 4, 2, 120)
glsl_mat2x3 = GlslBuiltinType('mat2x3', glsl_float, 2, 3, 120)
glsl_mat3x3 = glsl_mat3
glsl_mat4x3 = GlslBuiltinType('mat4x3', glsl_float, 4, 3, 120)
glsl_mat2x4 = GlslBuiltinType('mat2x4', glsl_float, 2, 4, 120)
glsl_mat3x4 = GlslBuiltinType('mat3x4', glsl_float, 3, 4, 120)
glsl_mat4x4 = glsl_mat4
glsl_int64_t  = GlslBuiltinType('int64_t', None,       1, 1, 400)
glsl_i64vec2  = GlslBuiltinType('i64vec2', glsl_int64_t,  1, 2, 400)
glsl_i64vec3  = GlslBuiltinType('i64vec3', glsl_int64_t,  1, 3, 400)
glsl_i64vec4  = GlslBuiltinType('i64vec4', glsl_int64_t,  1, 4, 400)
glsl_uint64_t = GlslBuiltinType('uint64_t', None,       1, 1, 400)
glsl_u64vec2  = GlslBuiltinType('u64vec2', glsl_uint64_t,  1, 2, 400)
glsl_u64vec3  = GlslBuiltinType('u64vec3', glsl_uint64_t,  1, 3, 400)
glsl_u64vec4  = GlslBuiltinType('u64vec4', glsl_uint64_t,  1, 4, 400)

# Named tuple representing the signature of a single overload of a
# built-in GLSL function or operator:
# - name is a name suitable for use in test filenames.  For functions,
#   this is the name of the function.  For operators, it is a short
#   description of the operator, beginning with "op", e.g. "op-plus".
# - template is a Python format string that can be used to construct
#   GLSL code that invokes the function or operator.
# - version_introduced earliest version of GLSL the test applies to
#   (as a string, e.g. 110).
# - rettype is the return type of the function or operator (as a
#   GlslBuiltinType).
# - argtypes is a tuple containing the types of each parameter (as
#   GlslBuiltinTypes).
#
# For example, the function
#
#   vec3 step(float edge, vec3 x)
#
# has a signature of
#
# Signature(name='step', template='step({0}, {1})',
#           version_introduced=110, rettype='vec3',
#           argtypes=('float', 'vec3'))
Signature = collections.namedtuple(
    'Signature',
    ('name', 'template', 'version_introduced', 'extension', 'rettype', 'argtypes'))


# Named tuple representing a single piece of test data for testing a
# built-in GLSL function:
# - arguments is a tuple containing the arguments to apply to the
#   function.  Each argument is of a type native to numpy (e.g.
#   numpy.float32 or numpy.ndarray)
# - result is the value the function is expected to return.  It is
#   also of a type native to numpy.
# - tolerance is a float32 representing how much deviation from the
#   result we expect, considering the floating point precision
#   requirements of GLSL and OpenGL.  The value may be zero for test
#   vectors involving booleans and integers.  If result is a vector or
#   matrix, tolerance should be interpreted as the maximum permissible
#   RMS error (as would be computed by the distance() function).
TestVector = collections.namedtuple(
    'TestVector', ('arguments', 'result', 'tolerance'))


def glsl_type_of(value):
    """Return the GLSL type corresponding to the given native numpy
    value, as a GlslBuiltinType.
    """
    if isinstance(value, FLOATING_TYPES):
        return glsl_float
    elif isinstance(value, (bool, np.bool_)):
        return glsl_bool
    elif isinstance(value, INT32_TYPES):
        return glsl_int
    elif isinstance(value, UINT32_TYPES):
        return glsl_uint
    elif isinstance(value, np.int64):
        return glsl_int64_t
    elif isinstance(value, np.uint64):
        return glsl_uint64_t
    else:
        assert isinstance(value, np.ndarray)
        if len(value.shape) == 1:
            # Vector
            vector_length = value.shape[0]
            assert 2 <= vector_length <= 4
            if value.dtype in FLOATING_TYPES:
                return (glsl_vec2, glsl_vec3, glsl_vec4)[vector_length - 2]
            elif value.dtype == np.int64:
                return (glsl_i64vec2, glsl_i64vec3, glsl_i64vec4)[vector_length - 2]
            elif value.dtype == np.uint64:
                return (glsl_u64vec2, glsl_u64vec3, glsl_u64vec4)[vector_length - 2]
            elif value.dtype == bool:
                return (glsl_bvec2, glsl_bvec3, glsl_bvec4)[vector_length - 2]
            elif value.dtype in INT32_TYPES:
                return (glsl_ivec2, glsl_ivec3, glsl_ivec4)[vector_length - 2]
            elif value.dtype in UINT32_TYPES:
                return (glsl_uvec2, glsl_uvec3, glsl_uvec4)[vector_length - 2]
            else:
                raise Exception(
                    'Unexpected vector base type {0}'.format(value.dtype))
        else:
            # Matrix
            assert value.dtype in FLOATING_TYPES
            assert len(value.shape) == 2
            matrix_rows = value.shape[0]
            assert 2 <= matrix_rows <= 4
            matrix_columns = value.shape[1]
            assert 2 <= matrix_columns <= 4
            matrix_types = ((glsl_mat2x2, glsl_mat2x3, glsl_mat2x4),
                            (glsl_mat3x2, glsl_mat3x3, glsl_mat3x4),
                            (glsl_mat4x2, glsl_mat4x3, glsl_mat4x4))
            return matrix_types[matrix_columns - 2][matrix_rows - 2]


def column_major_values(value):
    """Given a native numpy value, return a list of the scalar values
    comprising it, in column-major order."""
    if isinstance(value, np.ndarray):
        return list(np.reshape(value, -1, 'F'))
    else:
        return [value]


def glsl_constant(value):
    """Given a native numpy value, return GLSL code that constructs
    it."""
    column_major = np.reshape(np.array(value), -1, 'F')
    if column_major.dtype == bool:
        values = ['true' if x else 'false' for x in column_major]
    elif column_major.dtype == np.int64:
        values = [repr(x) + 'l' for x in column_major]
    elif column_major.dtype == np.uint64:
        values = [repr(x) + 'ul' for x in column_major]
    elif column_major.dtype in UINT32_TYPES:
        values = [repr(x) + 'u' for x in column_major]
    else:
        values = [repr(x) for x in column_major]
    if len(column_major) == 1:
        return values[0]
    else:
        return '{0}({1})'.format(glsl_type_of(value), ', '.join(values))


def round_to_32_bits(value):
    """If value is a floating point type, round it down to 32 bits.
    Otherwise return it unchanged.
    """
    if isinstance(value, float):
        return np.float32(value)
    elif isinstance(value, np.ndarray) and value.dtype == np.float64:
        return np.array(value, dtype=np.float32)
    else:
        return value


def extend_to_64_bits(value):
    """If value is a floating point type, extend it to 64 bits.
    Otherwise return it unchanged.
    """
    if isinstance(value, np.float32):
        return np.float64(value)
    elif isinstance(value, np.ndarray) and value.dtype == np.float32:
        return np.array(value, dtype=np.float64)
    else:
        return value


# Dictionary containing the test vectors.  Each entry in the
# dictionary represents a single overload of a single built-in
# function.  Its key is a Signature tuple, and its value is a list of
# TestVector tuples.
#
# Note: the dictionary is initialized to {} here, but it is filled
# with test vectors by code later in this file.
test_suite = {}


# Implementation
# ==============
#
# The functions below shouldn't be necessary to call from outside this
# file.  They exist solely to populate test_suite with test vectors.

# Functions that simulate GLSL built-in functions (in the cases where
# the GLSL built-in functions have no python or numpy equivalent, or
# in cases where there is a behavioral difference).  These functions
# return None if the behavior of the GLSL built-in is undefined for
# the given set of inputs.
def _multiply(x, y):
    x_type = glsl_type_of(x)
    y_type = glsl_type_of(y)

    if x_type.is_vector and y_type.is_vector:
        # vector * vector is done componentwise.
        return x * y
    else:
        # All other cases are standard linear algebraic
        # multiplication, which numpy calls "dot".
        return np.dot(x, y)


def _divide(x, y):
    if any(y_element == 0 for y_element in column_major_values(y)):
        # Division by zero is undefined.
        return None
    if glsl_type_of(x).base_type == glsl_int or glsl_type_of(x).base_type == glsl_int64_t:
        # The GLSL spec does not make it clear what the rounding rules
        # are when performing integer division.  C99 requires
        # round-toward-zero, so in the absence of any other
        # information, assume that's the correct behavior for GLSL.
        #
        # Python and numpy's rounding rules are inconsistent, so to
        # make sure we get round-toward-zero behavior, divide the
        # absolute values of x and y, and then fix the sign.
        return (np.abs(x) // np.abs(y)) * (np.sign(x) * np.sign(y))
    elif glsl_type_of(x).base_type == glsl_uint or glsl_type_of(x).base_type == glsl_uint64_t:
        return x // y
    else:
        return x / y


def _modulus(x, y):
    if any(x_element < 0 for x_element in column_major_values(x)):
        # Modulus operation with a negative first operand is
        # undefined.
        return None
    if any(y_element <= 0 for y_element in column_major_values(y)):
        # Modulus operation with a negative or zero second operand is
        # undefined.
        return None
    return x % y


def _lshift(x, y):
    base = glsl_type_of(x).base_type
    if base in (glsl_int64_t, glsl_uint64_t):
        bits = 64
        shift_type = glsl_int if base == glsl_int64_t else glsl_uint
    else:
        bits = 32
        shift_type = base
    if not all(0 <= y_element < bits for y_element in column_major_values(y)):
        # Shifts by less than 0 or more than the number of bits in the
        # type being shifted are undefined.
        return None
    # When the arguments to << don't have the same signedness, numpy
    # likes to promote them to int64.  To avoid this, convert y to be
    # the same type as x.
    y_orig = y
    if glsl_type_of(y).base_type != shift_type:
        y = _change_signedness(y)
    result = x << y

    # Shifting should always produce a result with the same base type
    # as the left argument.
    assert glsl_type_of(result).base_type == glsl_type_of(x).base_type

    return result


def _rshift(x, y):
    base = glsl_type_of(x).base_type
    if base in (glsl_int64_t, glsl_uint64_t):
        bits = 64
        shift_type = glsl_int if base == glsl_int64_t else glsl_uint
    else:
        bits = 32
        shift_type = base
    if not all(0 <= y_element < bits for y_element in column_major_values(y)):
        # Shifts by less than 0 or more than the number of bits in the
        # type being shifted are undefined.
        return None
    # When the arguments to >> don't have the same signedness, numpy
    # likes to promote them to int64.  To avoid this, convert y to be
    # the same type as x.
    y_orig = y
    if glsl_type_of(y).base_type != shift_type:
        y = _change_signedness(y)
    result = x >> y

    # Shifting should always produce a result with the same base type
    # as the left argument.
    assert glsl_type_of(result).base_type == glsl_type_of(x).base_type

    return result


def _equal(x, y):
    return all(column_major_values(x == y))


def _not_equal(x, y):
    return not _equal(x, y)


def _arctan2(y, x):
    if x == y == 0.0:
        return None
    return np.arctan2(y, x)


def _pow(x, y):
    if x < 0.0:
        return None
    if x == 0.0 and y <= 0.0:
        return None
    return np.power(x, y)


def _exp2(x):
    # exp2() is not available in versions of numpy < 1.3.0 so we
    # emulate it with power().
    return np.power(2, x)


def _trunc(x):
    # trunc() rounds toward zero.  It is not available in version
    # 1.2.1 of numpy so we emulate it with floor(), sign(), and abs().
    return np.sign(x) * np.floor(np.abs(x))


def _clamp(x, minVal, maxVal):
    if minVal > maxVal:
        return None
    return min(max(x, minVal), maxVal)


# Inefficient, but obvious
def _mid3(x, y, z):
    return np.sort([x, y, z])[1]

def _smoothstep(edge0, edge1, x):
    if edge0 >= edge1:
        return None
    t = _clamp((x-edge0)/(edge1-edge0), 0.0, 1.0)
    return t*t*(3.0-2.0*t)


def _normalize(x):
    return x/np.linalg.norm(x)


def _faceforward(N, I, Nref):
    if np.dot(Nref, I) < 0.0:
        return N
    else:
        return -N


def _reflect(I, N):
    return I-2*np.dot(N, I)*N


def _refract(I, N, eta):
    k = 1.0-eta*eta*(1.0-np.dot(N, I)*np.dot(N, I))
    if k < 0.0:
        return I*0.0
    else:
        return eta*I-(eta*np.dot(N, I)+np.sqrt(k))*N


def _change_signedness(x):
    """Change signed integer types to unsigned integer types and vice
    versa."""
    if isinstance(x, INT32_TYPES):
        return np.uint32(x)
    elif isinstance(x, UINT32_TYPES):
        return np.int32(x)
    elif isinstance(x, np.ndarray):
        if (x.dtype in INT32_TYPES):
            return np.array(x, dtype=np.uint32)
        elif (x.dtype in UINT32_TYPES):
            return np.array(x, dtype=np.int32)
    raise Exception('Unexpected type passed to _change_signedness')


def _argument_types_match(arguments, argument_indices_to_match):
    """Return True if all of the arguments indexed by
    argument_indices_to_match have the same GLSL type.
    """
    types = [glsl_type_of(arguments[i]) for i in argument_indices_to_match]
    return all(x == types[0] for x in types)


def _strict_tolerance(arguments, result):
    """Compute tolerance using a strict interpretation of the GLSL and
    OpenGL standards.

    From the GLSL 1.20 spec (4.1.4 "Floats"):

      "As an input value to one of the processing units, a
      floating-point variable is expected to match the IEEE single
      precision floating-point definition for precision and dynamic
      range.  It is not required that the precision of internal
      processing match the IEEE floating-point specification for
      floating-point operations, but the guidelines for precision
      established by the OpenGL 1.4 specification must be met."

    From the OpenGL 1.4 spec (2.1.1 "Floating-Point Computation"):

      "We require simply that numbers' floating-point parts contain
      enough bits ... so that individual results of floating-point
      operations are accurate to about 1 part in 10^5."

    A harsh interpretation of the above is that (a) no precision is
    lost in moving numbers into or out of the GPU, and (b) any
    built-in function constitutes a single operation, so therefore the
    error in applying any built-in function should be off by no more
    than 1e-5 times its theoretically correct value.

    This is not the only possible interpretation, however.  Certain
    built-in functions, such as the cross product, are computed by a
    formula consisting of many elementary multiplications and
    additions, in which a large amount of cancellation sometimes
    occurs.  It's possible that these rules are meant to apply to
    those elementary multiplications and additions, and not the full
    built-in function. Other built-in functions, such as the trig
    functions, are typically implemented by a series approximation, in
    which 1 part in 10^5 accuracy seems like overkill.  See below for
    the tolerance computation we use on these other functions.
    """
    return 1e-5 * np.linalg.norm(result)


def _trig_tolerance(arguments, result):
    """Compute a more lenient tolerance bound for trig functions.

    The GLSL and OpenGL specs don't provide any guidance as to the
    required accuracy of trig functions (other than the "1 part in
    10^5" general accuracy requirement, which seems like overkill for
    trig functions.

    So the tolerance here is rather arbitrarily chosen to be either 1
    part in 10^3 or 10^-4, whichever is larger.
    """
    return max(1e-4, 1e-3 * np.linalg.norm(result))


def _cross_product_tolerance(arguments, result):
    """Compute a more lenient tolerance bound for cross product.

    Since the computation of a cross product may involve a large
    amount of cancellation, an error tolerance of 1 part in 10^5
    (referred to the magnitude of the result vector) is overly tight.

    So instead we allow the error to be 1 part in 10^5 referred to the
    product of the magnitudes of the arguments.
    """
    assert len(arguments) == 2
    return 1e-5 * np.linalg.norm(arguments[0]) * np.linalg.norm(arguments[1])


def _simulate_function(test_inputs, python_equivalent, tolerance_function):
    """Construct test vectors by simulating a GLSL function on a list
    of possible inputs, and return a list of test vectors.

    test_inputs is a list of possible input sequences, each of which
    represents a set of arguments that should be applied to the
    function.

    python_equivalent is the function to simulate--it should return
    None if the GLSL function returns undefined results for the given
    set of inputs, otherwise it should return the expected result.
    Input sequences for which python_equivalent returns None are
    ignored.

    tolerance_function is the function to call to compute the
    tolerance.  It should take the set of arguments and the expected
    result as its parameters.  It is only used for functions that
    return floating point values.

    python_equivalent and tolerance_function are simulated using 64
    bit floats for maximum possible accuracy. The vector, however, is
    built with rounded to 32 bits values since that is the data type
    that we expect to get back from OpenGL.
    """
    test_vectors = []
    for inputs in test_inputs:
        expected_output = python_equivalent(
            *[extend_to_64_bits(x) for x in inputs])
        if expected_output is not None:
            if glsl_type_of(expected_output).base_type != glsl_float:
                tolerance = 0.0
            else:
                tolerance = tolerance_function(inputs, expected_output)
            test_vectors.append(TestVector(inputs,
                                           round_to_32_bits(expected_output),
                                           round_to_32_bits(tolerance)))
    return test_vectors


def _vectorize_test_vectors(test_vectors, scalar_arg_indices, vector_length):
    """Build a new set of test vectors by combining elements of
    test_vectors into vectors of length vector_length. For example,
    vectorizing the test vectors

    [TestVector((10, 20), 30, tolerance), TestVector((11, 20), 31, tolerance)]

    into vectors of length 2 would produce the result:

    [TestVector((vec2(10, 11), vec2(20, 20)), vec2(30, 31), new_tolerance)].

    Tolerances are combined in root-sum-square fashion.

    scalar_arg_indices is a sequence of argument indices which should
    not be vectorized.  So, if scalar_arg_indices is [1] in the above
    example, the result would be:

    [TestVector((vec2(10, 11), 20), vec2(30, 31), new_tolerance)].
    """
    def make_groups(test_vectors):
        """Group test vectors according to the values passed to the
        arguments that should not be vectorized.
        """
        groups = {}
        for tv in test_vectors:
            key = tuple(tv.arguments[i] for i in scalar_arg_indices)
            if key not in groups:
                groups[key] = []
            groups[key].append(tv)
        return groups

    def partition_vectors(test_vectors, partition_size):
        """Partition test_vectors into lists of length partition_size.
        If partition_size does not evenly divide the number of test
        vectors, wrap around as necessary to ensure that every input
        test vector is included.
        """
        for i in range(0, len(test_vectors), partition_size):
            partition = []
            for j in range(partition_size):
                partition.append(test_vectors[(i + j) % len(test_vectors)])
            yield partition

    def merge_vectors(test_vectors):
        """Merge the given set of test vectors (whose arguments and
        result are scalars) into a single test vector whose arguments
        and result are vectors.  For argument indices in
        scalar_arg_indices, leave the argument as a scalar.
        """
        arity = len(test_vectors[0].arguments)
        arguments = []
        for j in range(arity):
            if j in scalar_arg_indices:
                arguments.append(test_vectors[0].arguments[j])
            else:
                arguments.append(
                    np.array([tv.arguments[j] for tv in test_vectors]))
        result = np.array([tv.result for tv in test_vectors])
        tolerance = np.linalg.norm(
            [tv.tolerance for tv in test_vectors])
        return TestVector(arguments, result, tolerance)

    vectorized_test_vectors = []
    groups = make_groups(test_vectors)
    for key in sorted(groups.keys()):
        test_vectors = groups[key]
        vectorized_test_vectors.extend(
            merge_vectors(partition)
            for partition in partition_vectors(test_vectors, vector_length))
    return vectorized_test_vectors


def _store_test_vector(test_suite_dict, name, glsl_version, extension, test_vector,
                       template=None):
    """Store a test vector in the appropriate place in
    test_suite_dict.  The dictionary key (which is a Signature tuple)
    is generated by consulting the argument and return types of the
    test vector, and combining them with name and glsl_version.

    glsl_version is adjusted if necessary to reflect when the argument
    and return types were introduced into GLSL.

    If template is supplied, it is used instead as the template for the
    Signature objects generated.
    """
    if template is None:
        arg_indices = range(len(test_vector.arguments))
        template = '{0}({1})'.format(
            name, ', '.join('{{{0}}}'.format(i) for i in arg_indices))
    rettype = glsl_type_of(test_vector.result)
    argtypes = tuple(glsl_type_of(arg) for arg in test_vector.arguments)
    adjusted_glsl_version = max(
        glsl_version, rettype.version_introduced,
        *[t.version_introduced for t in argtypes])
    signature = Signature(
        name, template, adjusted_glsl_version, extension, rettype, argtypes)
    if signature not in test_suite_dict:
        test_suite_dict[signature] = []
    test_suite_dict[signature].append(test_vector)


def _store_test_vectors(test_suite_dict, name, glsl_version, extension,
                        test_vectors, template=None):
    """Store multiple test vectors in the appropriate places in
    test_suite_dict.

    If template is supplied, it is used instead as the template for the
    Signature objects generated.
    """
    for test_vector in test_vectors:
        _store_test_vector(test_suite_dict, name, glsl_version, extension,
                           test_vector, template=template)


def make_arguments(input_generators):
    """Construct a list of tuples of input arguments to test.

    input_generators is a list, the ith element of which is a sequence
    of values that are suitable for use as the ith argument of the
    function under test.

    Output is a list, each element of which is a tuple of arguments to
    be passed to the function under test.  These values are produced
    by taking the cartesian product of the input sequences.

    In addition, this function rounds floating point inputs to 32
    bits, so that there will be no rounding errors when the input
    values are passed into OpenGL.
    """
    input_generators = [
        [round_to_32_bits(x) for x in seq] for seq in input_generators]
    return list(itertools.product(*input_generators))


def _make_componentwise_test_vectors(test_suite_dict):
    """Add test vectors to test_suite_dict for GLSL built-in
    functions that operate on vectors in componentwise fashion.
    Examples include sin(), cos(), min(), max(), and clamp().
    """
    # Make sure atan(x) and atan(x,y) don't misbehave for very large
    # or very small input values.
    atan_inputs = [0.0]
    for exponent in (-10, -1, 0, 1, 10):
        atan_inputs.append(pow(10.0, exponent))
        atan_inputs.append(-pow(10.0, exponent))
    # Make a similar set of inputs for acosh(), except don't use any
    # values < 1, since acosh() is only defined for x >= 1.
    acosh_inputs = [1.0 + x for x in atan_inputs if x >= 0]
    ints = [np.int32(x) for x in [-5, -2, -1, 0, 1, 2, 5]]
    uints = [np.uint32(x) for x in [0, 1, 2, 5, 34]]
    bools = [True, False]

    def f(name, arity, glsl_version, python_equivalent,
          alternate_scalar_arg_indices, test_inputs,
          tolerance_function=_strict_tolerance,
          extension=None):
        """Create test vectors for the function with the given name
        and arity, which was introduced in the given glsl_version.

        python_equivalent is a Python function which operates on scalars,
        and simulates the GLSL function.  This function should return None
        in any case where the output of the GLSL function is undefined.

        If alternate_scalar_arg_indices is not None, also create test
        vectors for an alternate vectorized version of the function,
        in which some arguments are scalars.
        alternate_scalar_arg_indices is a sequence of the indices of
        the arguments which are scalars.

        test_inputs is a list, the ith element of which is a list of
        values that are suitable for use as the ith argument of the
        function.

        If tolerance_function is supplied, it is a function which
        should be used to compute the tolerance for the test vectors.
        Otherwise, _strict_tolerance is used.
        """
        scalar_test_vectors = _simulate_function(
            make_arguments(test_inputs), python_equivalent, tolerance_function)
        _store_test_vectors(
            test_suite_dict, name, glsl_version, extension, scalar_test_vectors)
        if alternate_scalar_arg_indices is None:
            scalar_arg_indices_list = [()]
        else:
            scalar_arg_indices_list = [(), alternate_scalar_arg_indices]
        for scalar_arg_indices in scalar_arg_indices_list:
            for vector_length in (2, 3, 4):
                _store_test_vectors(
                    test_suite_dict, name, glsl_version, extension,
                    _vectorize_test_vectors(
                        scalar_test_vectors, scalar_arg_indices,
                        vector_length))

    f('radians', 1, 110, np.radians, None, [np.linspace(-180.0, 180.0, 4)])
    f('degrees', 1, 110, np.degrees, None, [np.linspace(-np.pi, np.pi, 4)])
    f('sin', 1, 110, np.sin, None, [np.linspace(-np.pi, np.pi, 4)],
      _trig_tolerance)
    f('cos', 1, 110, np.cos, None, [np.linspace(-np.pi, np.pi, 4)],
      _trig_tolerance)
    f('tan', 1, 110, np.tan, None, [np.linspace(-np.pi, np.pi, 4)],
      _trig_tolerance)
    f('asin', 1, 110, np.arcsin, None, [np.linspace(-1.0, 1.0, 4)],
      _trig_tolerance)
    f('acos', 1, 110, np.arccos, None, [np.linspace(-1.0, 1.0, 4)],
      _trig_tolerance)
    f('atan', 1, 110, np.arctan, None, [atan_inputs], _trig_tolerance)
    f('atan', 2, 110, _arctan2, None, [atan_inputs, atan_inputs],
      _trig_tolerance)
    f('sinh', 1, 130, np.sinh, None, [np.linspace(-2.0, 2.0, 4)],
      _trig_tolerance)
    f('cosh', 1, 130, np.cosh, None, [np.linspace(-2.0, 2.0, 4)],
      _trig_tolerance)
    f('tanh', 1, 130, np.tanh, None, [np.linspace(-2.0, 2.0, 4)],
      _trig_tolerance)
    f('asinh', 1, 130, np.arcsinh, None, [atan_inputs], _trig_tolerance)
    f('acosh', 1, 130, np.arccosh, None, [acosh_inputs], _trig_tolerance)
    f('atanh', 1, 130, np.arctanh, None, [np.linspace(-0.99, 0.99, 4)],
      _trig_tolerance)
    f('pow', 2, 110, _pow, None, [np.linspace(0.0, 2.0, 4),
      np.linspace(-2.0, 2.0, 4)])
    f('exp', 1, 110, np.exp, None, [np.linspace(-2.0, 2.0, 4)])
    f('log', 1, 110, np.log, None, [np.linspace(0.01, 2.0, 4)])
    f('exp2', 1, 110, _exp2, None, [np.linspace(-2.0, 2.0, 4)])
    f('log2', 1, 110, np.log2, None, [np.linspace(0.01, 2.0, 4)])
    f('sqrt', 1, 110, np.sqrt, None, [np.linspace(0.0, 2.0, 4)])
    f('inversesqrt', 1, 110, lambda x: 1.0/np.sqrt(x), None,
      [np.linspace(0.1, 2.0, 4)])
    f('abs', 1, 110, np.abs, None, [np.linspace(-1.5, 1.5, 5)])
    f('abs', 1, 130, np.abs, None, [ints])
    f('sign', 1, 110, np.sign, None, [np.linspace(-1.5, 1.5, 5)])
    f('sign', 1, 130, np.sign, None, [ints])
    f('floor', 1, 110, np.floor, None, [np.linspace(-2.0, 2.0, 4)])

    # Note: with trunc we want to test values in which the floating
    # point exponent is < 0, > 23 or in the middle. Hence, we append
    # some numbers to cover all possible scenarios. In addition, we
    # want to check bitsize barriers (> 32, > 64, etc.) in case the
    # conversion is done with a cast to and from another int based
    # type.
    f('trunc', 1, 130, _trunc, None,
      [np.append(np.linspace(-2.0, 2.0, 8),
                 [0.0, 45027112.0, -45027112.0,
                  1.9584199e10, -1.9584199e10,
                  4.5027112e19, -4.5027112e19])])

    # Note: the direction of rounding used by round() is not specified
    # for half-integer values, so we test it over a range that doesn't
    # include exact half-integer values.  roundEven() is required to
    # round half-integer values to the nearest even integer, so we
    # test it over a range that does include exact half-integer
    # values.  In both cases, we can use numpy's round() function,
    # because it rounds half-integer values to even, and all other
    # values to nearest.
    f('round', 1, 130, np.round, None, [np.linspace(-2.0, 2.0, 8)])
    f('roundEven', 1, 130, np.round, None, [np.linspace(-2.0, 2.0, 25)])

    f('ceil', 1, 110, np.ceil, None, [np.linspace(-2.0, 2.0, 4)])
    f('fract', 1, 110, lambda x: x-np.floor(x), None,
      [np.linspace(-2.0, 2.0, 4)])
    f('mod', 2, 110, lambda x, y: x-y*np.floor(x/y), [1],
      [np.linspace(-1.9, 1.9, 4), np.linspace(-2.0, 2.0, 4)])
    f('min', 2, 110, min, [1],
      [np.linspace(-2.0, 2.0, 4), np.linspace(-2.0, 2.0, 4)])
    f('min', 2, 130, min, [1], [ints, ints])
    f('min', 2, 130, min, [1], [uints, uints])
    f('max', 2, 110, max, [1],
      [np.linspace(-2.0, 2.0, 4), np.linspace(-2.0, 2.0, 4)])
    f('max', 2, 130, max, [1], [ints, ints])
    f('max', 2, 130, max, [1], [uints, uints])
    f('min3', 2, 110, min, None,
      [np.linspace(-2.0, 2.0, 4), np.linspace(-2.0, 2.0, 4),
       np.linspace(-2.0, 2.0, 4)],
      extension="AMD_shader_trinary_minmax")
    f('min3', 2, 130, min, None, [ints, ints, ints],
      extension="AMD_shader_trinary_minmax")
    f('min3', 2, 130, min, None, [uints, uints, uints],
      extension="AMD_shader_trinary_minmax")
    f('max3', 2, 110, max, None,
      [np.linspace(-2.0, 2.0, 4), np.linspace(-2.0, 2.0, 4),
       np.linspace(-2.0, 2.0, 4)],
      extension="AMD_shader_trinary_minmax")
    f('max3', 2, 130, max, None, [ints, ints, ints],
      extension="AMD_shader_trinary_minmax")
    f('max3', 2, 130, max, None, [uints, uints, uints],
      extension="AMD_shader_trinary_minmax")
    f('mid3', 2, 110, _mid3, None,
      [np.linspace(-2.0, 2.0, 4), np.linspace(-2.0, 2.0, 4),
       np.linspace(-2.0, 2.0, 4)],
      extension="AMD_shader_trinary_minmax")
    f('mid3', 2, 130, _mid3, None, [ints, ints, ints],
      extension="AMD_shader_trinary_minmax")
    f('mid3', 2, 130, _mid3, None, [uints, uints, uints],
      extension="AMD_shader_trinary_minmax")
    f('clamp', 3, 110, _clamp, [1, 2], [np.linspace(-2.0, 2.0, 4),
      np.linspace(-1.5, 1.5, 3), np.linspace(-1.5, 1.5, 3)])
    f('clamp', 3, 130, _clamp, [1, 2], [ints, ints, ints])
    f('clamp', 3, 130, _clamp, [1, 2], [uints, uints, uints])
    f('mix', 3, 110, lambda x, y, a: x*(1-a)+y*a, [2],
      [np.linspace(-2.0, 2.0, 2), np.linspace(-3.0, 3.0, 2),
       np.linspace(0.0, 1.0, 4)])
    f('mix', 3, 130, lambda x, y, a: y if a else x, None,
      [np.linspace(-2.0, 2.0, 2), np.linspace(-3.0, 3.0, 2), bools])
    f('step', 2, 110, lambda edge, x: 0.0 if x < edge else 1.0, [0],
      [np.linspace(-2.0, 2.0, 4), np.linspace(-2.0, 2.0, 4)])
    f('smoothstep', 3, 110, _smoothstep, [0, 1],
      [np.linspace(-1.9, 1.9, 4), np.linspace(-1.9, 1.9, 4),
       np.linspace(-2.0, 2.0, 4)])

    f('abs', 1, 150, np.abs, None, [np.linspace(-10, 15, 54).astype(np.int64)],
      extension="ARB_gpu_shader_int64")
    f('sign', 1, 150, np.sign, None, [np.linspace(-15, 15, 5).astype(np.int64)],
      extension="ARB_gpu_shader_int64")
    f('min', 2, 150, min, [1],
      [np.linspace(-20, 20, 4).astype(np.int64), np.linspace(-20, 20, 4).astype(np.int64)],
      extension="ARB_gpu_shader_int64")
    f('min', 2, 150, min, [1],
      [np.linspace(20, 90, 4).astype(np.uint64), np.linspace(20, 90, 4).astype(np.uint64)],
      extension="ARB_gpu_shader_int64")
    f('max', 2, 150, max, [1],
      [np.linspace(-20, 20, 4).astype(np.int64), np.linspace(-20, 20, 4).astype(np.int64)],
      extension="ARB_gpu_shader_int64")
    f('max', 2, 150, max, [1],
      [np.linspace(20, 90, 4).astype(np.uint64), np.linspace(20, 90, 4).astype(np.uint64)],
      extension="ARB_gpu_shader_int64")
    f('clamp', 3, 150, _clamp, [1, 2], [np.linspace(-20, 20, 4).astype(np.int64),
                                   np.linspace(-15, 15, 3).astype(np.int64),
                                   np.linspace(-15, 15, 3).astype(np.int64)],
      extension="ARB_gpu_shader_int64")
    f('mix', 3, 150, lambda x, y, a: y if a else x, None,
      [np.linspace(-20, 20, 2).astype(np.int64), np.linspace(-30, 30, 2).astype(np.int64), bools],
      extension="ARB_gpu_shader_int64")
_make_componentwise_test_vectors(test_suite)


def _make_vector_relational_test_vectors(test_suite_dict):
    """Add test vectors to test_suite_dict for GLSL built-in functions
    that operate on vectors of floats, ints, or bools, but not on
    single floats, ints, or bools.  Examples include lessThan(),
    equal(), and not().
    """
    _default_inputs = {
        'v': np.linspace(-1.5, 1.5, 4),
        'i': np.array([-5, -2, -1, 0, 1, 2, 5], dtype=np.int32),
        'u': np.array([0, 1, 2, 5, 34], dtype=np.uint32),
        'b': np.array([False, True])
        }

    def f(name, arity, glsl_version, python_equivalent, arg_types,
          tolerance_function=_strict_tolerance,
          extension=None):
        """Make test vectors for the function with the given name and
        arity, which was introduced in the given glsl_version.

        python_equivalent is a Python function which operates on scalars,
        and simulates the GLSL function.

        arg_types is a string containing 'v' if the function supports
        standard "vec" inputs, 'i' if it supports "ivec" inputs, and 'b'
        if it supports "bvec" inputs.  The output type of the function is
        assumed to be the same as its input type.

        If tolerance_function is supplied, it is a function which
        should be used to compute the tolerance for the test vectors.
        Otherwise, _strict_tolerance is used.
        """
        for arg_type in arg_types:
            test_inputs = [_default_inputs[arg_type]]*arity
            scalar_test_vectors = _simulate_function(
                make_arguments(test_inputs), python_equivalent,
                tolerance_function)
            for vector_length in (2, 3, 4):
                _store_test_vectors(
                    test_suite_dict, name, glsl_version, extension,
                    _vectorize_test_vectors(
                        scalar_test_vectors, (), vector_length))

    f('lessThan', 2, 110, lambda x, y: x < y, 'viu')
    f('lessThanEqual', 2, 110, lambda x, y: x <= y, 'viu')
    f('greaterThan', 2, 110, lambda x, y: x > y, 'viu')
    f('greaterThanEqual', 2, 110, lambda x, y: x >= y, 'viu')
    f('equal', 2, 110, lambda x, y: x == y, 'viub')
    f('notEqual', 2, 110, lambda x, y: x != y, 'viub')
    f('not', 1, 110, lambda x: not x, 'b')
_make_vector_relational_test_vectors(test_suite)


def _make_vector_or_matrix_test_vectors(test_suite_dict):
    """Add test vectors to test_suite_dict for GLSL built-in functions
    that operate on vectors/matrices as a whole.  Examples include
    length(), dot(), cross(), normalize(), and refract().
    """
    def match_args(*indices):
        """Return a function that determines whether the type of the
        arguments at the given indices match.

        For example:

            match(1, 3)

        is equivalent to:

            lambda a, b, c, d: glsl_type_of(b) == glsl_type_of(d)
        """
        return lambda *args: _argument_types_match(args, indices)

    def match_simple_binop(x, y):
        """Determine whether the type of the arguments is compatible
        for a simple binary operator (such as '+').

        Arguments are compatible if one is a scalar and the other is a
        vector/matrix with the same base type, or if they are the same
        type.
        """
        x_type = glsl_type_of(x)
        y_type = glsl_type_of(y)
        if x_type.base_type != y_type.base_type:
            return False
        if x_type.is_scalar or y_type.is_scalar:
            return True
        return x_type == y_type

    def match_multiply(x, y):
        """Determine whether the type of the arguments is compatible
        for multiply.

        Arguments are compatible if they are scalars, vectors, or
        matrices with the same base type, and the vector/matrix sizes
        are properly matched.
        """
        x_type = glsl_type_of(x)
        y_type = glsl_type_of(y)
        if x_type.base_type != y_type.base_type:
            return False
        if x_type.is_scalar or y_type.is_scalar:
            return True
        if x_type.is_vector and y_type.is_matrix:
            # When multiplying vector * matrix, the vector is
            # transposed to a row vector.  So its row count must match
            # the row count of the matrix.
            return x_type.num_rows == y_type.num_rows
        elif x_type.is_vector:
            assert y_type.is_vector
            # When multiplying vector * vector, the multiplication is
            # done componentwise, so the types must match exactly.
            return x_type == y_type
        else:
            assert x_type.is_matrix
            # When multiplying matrix * matrix or matrix * vector, a
            # standard linear algebraic multiply is used, so x's
            # column count must match y's row count.
            return x_type.num_cols == y_type.num_rows

    def match_shift(x, y):
        """Determine whether the type of the arguments is compatible
        for shift operations.

        Arguments are compatible if they are the same length or the
        first one is a vector and the second is a scalar.  Their base
        types need not be the same, but they both must be integral.
        """
        x_type = glsl_type_of(x)
        y_type = glsl_type_of(y)
        if x_type.base_type not in (glsl_int, glsl_uint, glsl_int64_t, glsl_uint64_t):
            return False
        if y_type.base_type not in (glsl_int, glsl_uint):
            return False
        if y_type.is_scalar:
            return True
        assert not x_type.is_matrix
        assert not y_type.is_matrix
        return x_type.num_rows == y_type.num_rows

    def match_assignment_operators(x, y):
        """ Determine when scalar and matrix arithmetic works

        A matrix and a scalar can be combined, but only when being assigned
        into a large enough type.

        """
        x_type = glsl_type_of(x)
        y_type = glsl_type_of(y)
        if x_type.base_type != y_type.base_type:
            return False
        if y_type.is_scalar:
            return True
        return x_type == y_type

    def match_assignment_multiply(x, y):
        """Determine whether the type of the arguments is compatible
        for multiply.

        Arguments are compatible if they are scalars, vectors, or
        matrices with the same base type, and the vector/matrix sizes
        are properly matched, and that y is scalar
        """
        x_type = glsl_type_of(x)
        y_type = glsl_type_of(y)
        if x_type.base_type != y_type.base_type:
            return False
        if y_type.is_scalar:
            return True
        if x_type.is_scalar:
            return False
        if x_type.is_vector and y_type.is_matrix:
            # When multiplying vector * matrix, the vector is
            # transposed to a row vector.  So its row count must match
            # the row count of the matrix.
            return x_type.num_rows == y_type.num_rows == y_type.num_cols
        elif x_type.is_vector:
            assert y_type.is_vector
            # When multiplying vector * vector, the multiplication is
            # done componentwise, so the types must match exactly.
            return x_type == y_type
        else:
            assert x_type.is_matrix
            # When multiplying matrix * matrix or matrix * vector, a
            # standard linear algebraic multiply is used, so x's
            # column count must match y's row count.
            return (x_type.num_cols == y_type.num_rows and
                    x_type.num_cols == y_type.num_cols)

    bools = [False, True]
    bvecs = [np.array(bs) for bs in itertools.product(bools, bools)] + \
        [np.array(bs) for bs in itertools.product(bools, bools, bools)] + \
        [np.array(bs) for bs in itertools.product(bools, bools, bools, bools)]
    ints = [np.int32(x) for x in [12, -6, 76, -32, 0]]
    small_ints = \
        [np.int32(x) for x in [-31, -25, -5, -2, -1, 0, 1, 2, 5, 25, 31]]
    ivecs = [
        np.array([38, 35], dtype=np.int32),
        np.array([64, -9], dtype=np.int32),
        np.array([64, 9], dtype=np.int32),
        np.array([-36, 32, -88], dtype=np.int32),
        np.array([36, 32, 88], dtype=np.int32),
        np.array([59, 77, 68], dtype=np.int32),
        np.array([-64, 72, 88, -76], dtype=np.int32),
        np.array([64, 72, 88, 76], dtype=np.int32),
        np.array([-24, 40, -23, 64], dtype=np.int32),
        np.array([24, 40, 23, 64], dtype=np.int32),
        ]
    small_ivecs = [
        np.array([13, 26], dtype=np.int32),
        np.array([-2, 26], dtype=np.int32),
        np.array([2, 26], dtype=np.int32),
        np.array([22, -23, 4], dtype=np.int32),
        np.array([22, 23, 4], dtype=np.int32),
        np.array([-19, 1, -13], dtype=np.int32),
        np.array([19, 1, 13], dtype=np.int32),
        np.array([16, 24, -23, -25], dtype=np.int32),
        np.array([16, 24, 23, 25], dtype=np.int32),
        np.array([-23, -12, 14, 19], dtype=np.int32),
        np.array([23, 12, 14, 19], dtype=np.int32),
        ]
    uints = [np.uint32(x) for x in [0, 6, 12, 32, 74]]
    small_uints = [np.uint32(x) for x in [0, 1, 2, 5, 25, 31]]
    large_uints = [np.uint32(x) for x in [0xdeadbeef, 0xaffeaffe, 0xbadbad]]
    uvecs = [
        np.array([38, 35], dtype=np.uint32),
        np.array([64, 9], dtype=np.uint32),
        np.array([36, 32, 88], dtype=np.uint32),
        np.array([59, 77, 68], dtype=np.uint32),
        np.array([66, 72, 87, 75], dtype=np.uint32),
        np.array([24, 40, 23, 74], dtype=np.uint32)
        ]
    small_uvecs = [
        np.array([13, 26], dtype=np.uint32),
        np.array([2, 26], dtype=np.uint32),
        np.array([22, 23, 4], dtype=np.uint32),
        np.array([19, 1, 13], dtype=np.uint32),
        np.array([16, 24, 23, 25], dtype=np.uint32),
        np.array([23, 12, 14, 19], dtype=np.uint32),
        ]
    nz_floats = [-1.33, 0.85]
    floats = [0.0] + nz_floats
    vecs = [
        np.array([-0.10, -1.20]),
        np.array([-0.42, 0.48]),
        np.array([-0.03, -0.85, -0.94]),
        np.array([1.67, 0.66, 1.87]),
        np.array([-1.65, 1.33, 1.93, 0.76]),
        np.array([0.80, -0.15, -0.51, 0.0])
        ]
    nz_floats_vecs = nz_floats + vecs
    vec3s = [
        np.array([-0.03, -0.85, -0.94]),
        np.array([1.67, 0.66, 1.87]),
        ]
    norm_floats_vecs = [_normalize(x) for x in nz_floats_vecs]
    squaremats = [
        np.array([[ 1.60,  0.76],
                  [ 1.53, -1.00]]),  # mat2
        np.array([[-0.13, -0.87],
                  [-1.40,  1.40]]),  # mat2
        np.array([[-1.11,  1.67, -0.41],
                  [ 0.13,  1.09, -0.02],
                  [ 0.56,  0.95,  0.24]]),  # mat3
        np.array([[-1.69, -0.46, -0.18],
                  [-1.09,  1.75,  2.00],
                  [-1.53, -0.70, -1.47]]),  # mat3
        np.array([[-1.00, -0.55, -1.08,  1.79],
                  [ 1.77,  0.62,  0.48, -1.35],
                  [ 0.09, -0.71, -1.39, -1.21],
                  [-0.91, -1.82, -1.43,  0.72]]),  # mat4
        np.array([[ 0.06,  1.31,  1.52, -1.96],
                  [ 1.60, -0.32,  0.51, -1.84],
                  [ 1.25,  0.45,  1.90, -0.72],
                  [-0.16,  0.45, -0.88,  0.39]]),  # mat4
        ]
    mats = squaremats + [
        np.array([[ 0.09,  1.30,  1.25],
                  [-1.19,  0.08,  1.08]]),  # mat3x2
        np.array([[-0.36, -1.08, -0.60],
                  [-0.53,  0.88, -1.79]]),  # mat3x2
        np.array([[-0.46,  1.94],
                  [-0.45, -0.75],
                  [ 1.03, -0.50]]),  # mat2x3
        np.array([[ 1.38, -1.08],
                  [-1.27,  1.83],
                  [ 1.00, -0.74]]),  # mat2x3
        np.array([[ 1.81, -0.87,  0.81,  0.65],
                  [-1.16, -1.52,  0.25, -1.51]]),  # mat4x2
        np.array([[ 1.93, -1.63,  0.29,  1.60],
                  [ 0.49,  0.27,  0.14,  0.94]]),  # mat4x2
        np.array([[ 0.16, -1.69],
                  [-0.80,  0.59],
                  [-1.74, -1.43],
                  [-0.02, -1.21]]),  # mat2x4
        np.array([[-1.02,  0.74],
                  [-1.64, -0.13],
                  [-1.59,  0.47],
                  [ 0.30,  1.13]]),  # mat2x4
        np.array([[-0.27, -1.38, -1.41, -0.12],
                  [-0.17, -0.56,  1.47,  1.86],
                  [-1.85, -1.29,  1.77,  0.01]]),  # mat4x3
        np.array([[-0.47, -0.15,  1.97, -1.05],
                  [-0.20,  0.53, -1.82, -1.41],
                  [-1.39, -0.19,  1.62,  1.58]]),  # mat4x3
        np.array([[ 1.42, -0.86,  0.27],
                  [ 1.80, -1.74,  0.04],
                  [-1.88, -0.37,  0.43],
                  [ 1.37,  1.90,  0.71]]),  # mat3x4
        np.array([[-1.72,  0.09,  0.45],
                  [-0.31, -1.58,  1.92],
                  [ 0.14,  0.18, -0.56],
                  [ 0.40, -0.77,  1.76]]),  # mat3x4
        ]

    int64s = [np.int64(x) for x in [
	0,
	3,
	-1192,
	1048576,
	4251475,
	29852643761,
	-4398046511104,
	-3948976685146,
	-135763469567146206]]
    uint64s = [np.uint64(x) for x in [
	0,
	3,
	1192,
	1048576,
	4251475,
	29852643761,
	4398046511104,
	3948976685146,
	135763469567146206,
	11654173250180970009]]

    int64vecs = [
        np.array([-10, -12], dtype=np.int64),
        np.array([-42, 48], dtype=np.int64),
        np.array([-1333333333333333259, 85, 94], dtype=np.int64),
        np.array([167, 66, 187], dtype=np.int64),
        np.array([165, 133, 193, 76], dtype=np.int64),
        np.array([80, -15, -51, 0], dtype=np.int64)
        ]
    int64_i64vecs = int64s + int64vecs
    i64vec3s = [
        np.array([-3, -85, -94], dtype=np.int64),
        np.array([ 1333333333333333259, 66, 87], dtype=np.int64),
        ]

    uint64vecs = [
        np.array([10, 12], dtype=np.uint64),
        np.array([42, 48], dtype=np.uint64),
        np.array([1333333333333333259, 85, 94], dtype=np.uint64),
        np.array([167, 66, 187], dtype=np.uint64),
        np.array([165, 133, 193, 76], dtype=np.uint64),
        np.array([80, 15, 51, 0], dtype=np.uint64)
        ]
    uint64_u64vecs = uint64s + uint64vecs
    u64vec3s = [
        np.array([3, 85, 94], dtype=np.uint64),
        np.array([ 1333333333333333259, 66, 87], dtype=np.uint64),
        ]

    def f(name, arity, glsl_version, python_equivalent,
          filter, test_inputs, tolerance_function=_strict_tolerance,
          template=None,
          extension=None):
        """Make test vectors for the function with the given name and
        arity, which was introduced in the given glsl_version.

        python_equivalent is a Python function which simulates the GLSL
        function.  This function should return None in any case where the
        output of the GLSL function is undefined.  However, it need not
        check that the lengths of the input vectors are all the same.

        If filter is not None, it will be called with each set of
        arguments, and test cases will only be generated if the filter
        returns True.

        test_inputs is a list, the ith element of which is a list of
        vectors and/or scalars that are suitable for use as the ith
        argument of the function.

        If tolerance_function is supplied, it is a function which
        should be used to compute the tolerance for the test vectors.
        Otherwise, _strict_tolerance is used.

        If template is supplied, it is used instead as the template for
        the Signature objects generated.
        """
        test_inputs = make_arguments(test_inputs)
        if filter is not None:
            test_inputs = \
                [arguments for arguments in test_inputs if filter(*arguments)]
        _store_test_vectors(
            test_suite_dict, name, glsl_version, extension,
            _simulate_function(
                test_inputs, python_equivalent, tolerance_function),
            template=template)

    f('op-assign-add', 2, 110, lambda x, y: x + y, match_assignment_operators,
      [floats+vecs+mats+ints+ivecs+uints+uvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs],
      template='{0};\n  result += {1}')
    # This can generate an overflow warning, this is expected
    with warnings.catch_warnings():
        warnings.simplefilter('ignore', RuntimeWarning)
        f('op-assign-sub', 2, 110,
          lambda x, y: x - y, match_assignment_operators,
          [floats+vecs+mats+ints+ivecs+uints+uvecs,
           floats+vecs+mats+ints+ivecs+uints+uvecs],
          template='{0};\n  result -= {1}')
    f('op-assign-mult', 2, 110, _multiply, match_assignment_multiply,
      [floats+vecs+mats+ints+ivecs+uints+uvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs],
      template='{0};\n  result *= {1}')
    f('op-assign-div', 2, 110, _divide, match_assignment_operators,
      [floats+vecs+mats+ints+ivecs+uints+uvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs],
      template='{0};\n  result /= {1}')
    f('op-assign-div-large', 2, 130, _divide, match_assignment_operators,
      [large_uints, large_uints+small_uints],
      template='{0};\n  result /= {1}')
    f('op-assign-mod', 2, 130, _modulus, match_assignment_operators,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='{0};\n result %= {1}')
    f('op-assign-bitand', 2, 130, lambda x, y: x & y,
      match_assignment_operators,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='{0};  result &= {1}')
    f('op-assign-bitor', 2, 130, lambda x, y: x | y,
      match_assignment_operators,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='{0};\n  result |= {1}')
    f('op-assign-bitxor', 2, 130, lambda x, y: x ^ y,
      match_assignment_operators,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='{0};\n  result ^= {1}')
    f('op-assign-lshift', 2, 130, _lshift, match_shift,
      [small_ints+small_ivecs+small_uints+small_uvecs,
       small_ints+small_ivecs+small_uints+small_uvecs],
      template='{0};  result <<= {1}')
    f('op-assign-rshift', 2, 130, _rshift, match_shift,
      [small_ints+small_ivecs+small_uints+small_uvecs,
       small_ints+small_ivecs+small_uints+small_uvecs],
      template='{0};  result >>= {1}')
    f('op-add', 2, 110, lambda x, y: x + y, match_simple_binop,
      [floats+vecs+mats+ints+ivecs+uints+uvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs],
      template='({0} + {1})')
    # This can generate an overflow warning, this is expected
    with warnings.catch_warnings():
        warnings.simplefilter('ignore', RuntimeWarning)
        f('op-sub', 2, 110, lambda x, y: x - y, match_simple_binop,
          [floats+vecs+mats+ints+ivecs+uints+uvecs,
           floats+vecs+mats+ints+ivecs+uints+uvecs],
          template='({0} - {1})')
    f('op-mult', 2, 110, _multiply, match_multiply,
      [floats+vecs+mats+ints+ivecs+uints+uvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs],
      template='({0} * {1})')
    f('op-div', 2, 110, _divide, match_simple_binop,
      [floats+vecs+mats+ints+ivecs+uints+uvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs],
      template='({0} / {1})')
    f('op-div-large', 2, 130, _divide, match_simple_binop,
      [large_uints, large_uints+small_uints], template='({0} / {1})')
    f('op-mod', 2, 130, _modulus, match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs], template='({0} % {1})')
    f('op-uplus', 1, 110, lambda x: +x, None,
      [floats+vecs+mats+ints+ivecs+uints+uvecs], template='(+ {0})')
    f('op-neg', 1, 110, lambda x: -x, None,
      [floats+vecs+mats+ints+ivecs+uints+uvecs], template='(- {0})')
    f('op-gt', 2, 110, lambda x, y: x > y, match_args(0, 1),
      [ints+uints+floats, ints+uints+floats], template='({0} > {1})')
    f('op-lt', 2, 110, lambda x, y: x < y, match_args(0, 1),
      [ints+uints+floats, ints+uints+floats], template='({0} < {1})')
    f('op-ge', 2, 110, lambda x, y: x >= y, match_args(0, 1),
      [ints+uints+floats, ints+uints+floats], template='({0} >= {1})')
    f('op-le', 2, 110, lambda x, y: x <= y, match_args(0, 1),
      [ints+uints+floats, ints+uints+floats], template='({0} <= {1})')
    f('op-eq', 2, 110, _equal, match_args(0, 1),
      [floats+vecs+mats+ints+ivecs+uints+uvecs+bools+bvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs+bools+bvecs],
      template='({0} == {1})')
    f('op-ne', 2, 110, _not_equal, match_args(0, 1),
      [floats+vecs+mats+ints+ivecs+uints+uvecs+bools+bvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs+bools+bvecs],
      template='({0} != {1})')
    f('op-and', 2, 110, lambda x, y: x and y, None, [bools, bools],
      template='({0} && {1})')
    f('op-or', 2, 110, lambda x, y: x or y, None, [bools, bools],
      template='({0} || {1})')
    f('op-xor', 2, 110, lambda x, y: x != y, None, [bools, bools],
      template='({0} ^^ {1})')
    f('op-not', 1, 110, lambda x: not x, None, [bools], template='(! {0})')
    f('op-selection', 3, 110, lambda x, y, z: y if x else z, match_args(1, 2),
      [bools, floats+vecs+mats+ints+ivecs+uints+uvecs+bools+bvecs,
       floats+vecs+mats+ints+ivecs+uints+uvecs+bools+bvecs],
      template='({0} ? {1} : {2})')
    f('op-complement', 1, 130, lambda x: ~x, None, [ints+ivecs+uints+uvecs],
      template='(~ {0})')
    f('op-lshift', 2, 130, _lshift, match_shift,
      [small_ints+small_ivecs+small_uints+small_uvecs,
       small_ints+small_ivecs+small_uints+small_uvecs],
      template='({0} << {1})')
    f('op-rshift', 2, 130, _rshift, match_shift,
      [small_ints+small_ivecs+small_uints+small_uvecs,
       small_ints+small_ivecs+small_uints+small_uvecs],
      template='({0} >> {1})')
    f('op-bitand', 2, 130, lambda x, y: x & y, match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} & {1})')
    f('op-bitor', 2, 130, lambda x, y: x | y, match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} | {1})')
    f('op-bitxor', 2, 130, lambda x, y: x ^ y, match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} ^ {1})')
    f('op-bitand-neg', 2, 130, lambda x, y: x & (-y), match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} & (- {1}))')
    f('op-bitand-not', 2, 130, lambda x, y: x & (~y), match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} & (~ {1}))')
    f('op-bitor-neg', 2, 130, lambda x, y: x | (-y), match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} | (- {1}))')
    f('op-bitor-not', 2, 130, lambda x, y: x | (~y), match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} | (~ {1}))')
    f('op-bitxor-neg', 2, 130, lambda x, y: x ^ (-y), match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} ^ (- {1}))')
    f('op-bitxor-not', 2, 130, lambda x, y: x ^ (~y), match_simple_binop,
      [ints+ivecs+uints+uvecs, ints+ivecs+uints+uvecs],
      template='({0} ^ (~ {1}))')
    f('op-bitand-neg-abs', 2, 130, lambda x, y: x & (-abs(y)), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} & (- abs({1})))')
    f('op-bitand-not-abs', 2, 130, lambda x, y: x & (~abs(y)), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} & (~ abs({1})))')
    f('op-bitor-neg-abs', 2, 130, lambda x, y: x | (-abs(y)), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} | (- abs({1})))')
    f('op-bitor-not-abs', 2, 130, lambda x, y: x | (~abs(y)), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} | (~ abs({1})))')
    f('op-bitxor-neg-abs', 2, 130, lambda x, y: x ^ (-abs(y)), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} ^ (- abs({1})))')
    f('op-bitxor-not-abs', 2, 130, lambda x, y: x ^ (~abs(y)), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} ^ (~ abs({1})))')
    f('op-bitand-abs-neg', 2, 130, lambda x, y: x & abs(-y), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} & abs(- {1}))')
    f('op-bitand-abs-not', 2, 130, lambda x, y: x & abs(~y), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} & abs(~ {1}))')
    f('op-bitor-abs-neg', 2, 130, lambda x, y: x | abs(-y), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} | abs(- {1}))')
    f('op-bitor-abs-not', 2, 130, lambda x, y: x | abs(~y), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} | abs(~ {1}))')
    f('op-bitxor-abs-neg', 2, 130, lambda x, y: x ^ abs(-y), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} ^ abs(- {1}))')
    f('op-bitxor-abs-not', 2, 130, lambda x, y: x ^ abs(~y), match_simple_binop,
      [ints+ivecs, ints+ivecs],
      template='({0} ^ abs(~ {1}))')
    f('length', 1, 110, np.linalg.norm, None, [floats+vecs])
    f('distance', 2, 110, lambda x, y: np.linalg.norm(x-y), match_args(0, 1),
      [floats+vecs, floats+vecs])
    f('dot', 2, 110, np.dot, match_args(0, 1), [floats+vecs, floats+vecs])
    f('cross', 2, 110, np.cross, match_args(0, 1), [vec3s, vec3s],
      _cross_product_tolerance)
    f('normalize', 1, 110, _normalize, None, [nz_floats_vecs])
    f('faceforward', 3, 110, _faceforward, match_args(0, 1, 2),
      [floats+vecs, floats+vecs, floats+vecs])
    f('reflect', 2, 110, _reflect, match_args(0, 1),
      [floats+vecs, norm_floats_vecs])
    f('refract', 3, 110, _refract, match_args(0, 1),
      [norm_floats_vecs, norm_floats_vecs, [0.5, 2.0]])

    # Note: technically matrixCompMult operates componentwise.
    # However, since it is the only componentwise function to operate
    # on matrices, it is easier to generate test cases for it here
    # than to add matrix support to _make_componentwise_test_vectors.
    f('matrixCompMult', 2, 110, lambda x, y: x*y, match_args(0, 1),
      [mats, mats])

    f('outerProduct', 2, 120, np.outer, None, [vecs, vecs])
    f('transpose', 1, 120, np.transpose, None, [mats])
    f('any', 1, 110, any, None, [bvecs])
    f('all', 1, 110, all, None, [bvecs])

    f('inverse', 1, 140, np.linalg.inv, None, [squaremats])

    f('determinant', 1, 150, np.linalg.det, None, [squaremats])

    with warnings.catch_warnings():
        warnings.simplefilter('ignore', RuntimeWarning)
        f('op-add', 2, 150, lambda x, y: x + y, match_simple_binop,
          [int64s+int64vecs+uint64s+uint64vecs,
           int64s+int64vecs+uint64s+uint64vecs],
          template='({0} + {1})',
          extension="ARB_gpu_shader_int64")
    with warnings.catch_warnings():
        warnings.simplefilter('ignore', RuntimeWarning)
        f('op-sub', 2, 150, lambda x, y: x - y, match_simple_binop,
          [int64s+int64vecs+uint64s+uint64vecs,
           int64s+int64vecs+uint64s+uint64vecs],
          template='({0} - {1})',
          extension="ARB_gpu_shader_int64")
    f('op-mult', 2, 150, _multiply, match_multiply,
      [int64s+int64vecs+uint64s+uint64vecs,
       int64s+int64vecs+uint64s+uint64vecs],
      template='({0} * {1})',
      extension="ARB_gpu_shader_int64")
    f('op-div', 2, 150, _divide, match_simple_binop,
      [int64s+int64vecs+uint64s+uint64vecs,
       int64s+int64vecs+uint64s+uint64vecs],
      template='({0} / {1})',
      extension="ARB_gpu_shader_int64")
    f('op-mod', 2, 150, _modulus, match_simple_binop,
      [int64s+int64vecs+uint64s+uint64vecs,
       int64s+int64vecs+uint64s+uint64vecs],
      template='({0} % {1})',
      extension="ARB_gpu_shader_int64")
    f('op-gt', 2, 150, lambda x, y: x > y, match_args(0, 1),
      [int64s+uint64s,
       int64s+uint64s],
      template = '({0} > {1})',
      extension="ARB_gpu_shader_int64")
    f('op-lt', 2, 150, lambda x, y: x < y, match_args(0, 1),
      [int64s+uint64s,
       int64s+uint64s],
      template = '({0} < {1})',
      extension="ARB_gpu_shader_int64")
    f('op-ge', 2, 150, lambda x, y: x >= y, match_args(0, 1),
      [int64s+uint64s,
       int64s+uint64s],
      template = '({0} >= {1})',
      extension="ARB_gpu_shader_int64")
    f('op-le', 2, 150, lambda x, y: x <= y, match_args(0, 1),
      [int64s+uint64s,
       int64s+uint64s],
      template = '({0} <= {1})',
      extension="ARB_gpu_shader_int64")
    f('op-eq', 2, 150, lambda x, y: x == y, match_args(0, 1),
      [int64s+uint64s,
       int64s+uint64s],
      template = '({0} == {1})',
      extension="ARB_gpu_shader_int64")
    f('op-ne', 2, 150, lambda x, y: x != y, match_args(0, 1),
      [int64s+uint64s,
       int64s+uint64s],
      template = '({0} != {1})',
      extension="ARB_gpu_shader_int64")
    f('op-bitand', 2, 150, lambda x, y: x & y, match_simple_binop,
      [int64s+uint64s, int64s+uint64s],
      template='({0} & {1})',
      extension="ARB_gpu_shader_int64")
    f('op-bitor', 2, 150, lambda x, y: x | y, match_simple_binop,
      [int64s+uint64s, int64s+uint64s],
      template='({0} | {1})',
      extension="ARB_gpu_shader_int64")
    f('op-bitxor', 2, 150, lambda x, y: x ^ y, match_simple_binop,
      [int64s+uint64s, int64s+uint64s],
      template='({0} ^ {1})',
      extension="ARB_gpu_shader_int64")
    f('op-lshift', 2, 150, _lshift, match_shift,
      [int64s+uint64s,
       small_uints],
      template='({0} << {1})',
      extension="ARB_gpu_shader_int64")
    f('op-rshift', 2, 150, _rshift, match_shift,
      [int64s+uint64s,
       small_uints],
      template='({0} >> {1})',
      extension="ARB_gpu_shader_int64")
_make_vector_or_matrix_test_vectors(test_suite)


def _check_signature_safety(test_suite_dict):
    """As a final safety check, verify that for each possible
    combination of name and argtypes, there is exactly one
    signature.
    """
    name_argtype_combos = set()
    for signature in test_suite_dict:
        name_argtype_combo = (signature.name, signature.argtypes)
        if name_argtype_combo in name_argtype_combos:
            raise Exception(
                'Duplicate signature found for {0}'.format(name_argtype_combo))
        name_argtype_combos.add(name_argtype_combo)
_check_signature_safety(test_suite)

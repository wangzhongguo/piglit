# encoding=utf-8
# Copyright © 2016, 2019 Intel corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Extra pytest skip fixtures.

There are a number of repeated conditions for skipping, python version, os name
or class, etc.
"""

import os
import sys

import pytest

# pylint: disable=invalid-name

posix = pytest.mark.skipif(
    os.name != 'posix', reason="Test is only relevant on posix systems.")
windows = pytest.mark.skipif(
    os.name != 'nt', reason="Test is only relevant on Microsoft Windows.")
linux = pytest.mark.skipif(
    not sys.platform.startswith('linux'),
    reason="Test is only relevant on Linux OSes.")

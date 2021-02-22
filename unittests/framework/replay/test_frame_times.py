# coding=utf-8
#
# Copyright © 2020 Valve Corporation.
# Copyright © 2021 Collabora Ltd.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT


"""Tests for replayer's frame_times module."""

import pytest

import contextlib
import io

from os import path

from framework import exceptions, status
from framework.replay import backends
from framework.replay import frame_times
from framework.replay.options import OPTIONS


class TestFrameTimes(object):
    """Tests for frame_times methods."""

    @staticmethod
    def mock_profile(trace_path):
        if trace_path.endswith('KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr'):
            return None
        elif trace_path.endswith('pathfinder/demo.trace'):
            return [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        else:
            raise exceptions.PiglitFatalError(
                'Non treated trace path: {}'.format(trace_path))

    @staticmethod
    def mock_qty_load_yaml(yaml_file):
        if yaml_file == 'empty.yml':
            return {}
        elif yaml_file == 'no-device.yml':
            return {"traces":
              [{"path": "glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc",
                "expectations": [{"device": "gl-vmware-llvmpipe",
                                  "checksum": "8867f3a41f180626d0d4b7661ff5c0f4"}]},]}
        elif yaml_file == 'one-trace.yml':
            return {"traces":
              [{"path": "pathfinder/demo.trace",
                "expectations": [{"device": OPTIONS.device_name,
                                  "checksum": "e624d76c70cc3c532f4f54439e13659a"}]},]}
        elif yaml_file == 'two-traces.yml':
            return {"traces":
              [{"path": "pathfinder/demo.trace",
                "expectations": [{"device": OPTIONS.device_name,
                                  "checksum": "e624d76c70cc3c532f4f54439e13659a"}]},
               {"path": "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr",
                "expectations": [{"device": OPTIONS.device_name,
                                  "checksum": "917cbbf4f09dd62ea26d247a1c70c16e"}]},]}
        else:
            raise exceptions.PiglitFatalError(
                'Non treated YAML file: {}'.format(yaml_file))

    @pytest.fixture(autouse=True)
    def setup(self, mocker, tmpdir):
        """Setup for TestFrameTimes.

        This create the basic environment for testing.
        """

        OPTIONS.device_name = 'test-device'
        OPTIONS.db_path = tmpdir.mkdir('db-path').strpath
        OPTIONS.results_path = tmpdir.mkdir('results').strpath
        self.trace_path = 'pathfinder/demo.trace'
        self.exp_frame_times = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        self.results_partial_path = path.join('results/trace',
                                              OPTIONS.device_name)
        self.m_qty_load_yaml = mocker.patch(
            'framework.replay.frame_times.qty.load_yaml',
            side_effect=TestFrameTimes.mock_qty_load_yaml)
        self.m_ensure_file = mocker.patch(
            'framework.replay.frame_times.ensure_file',
            return_value=None)
        self.m_profile = mocker.patch(
            'framework.replay.frame_times.APITraceBackend.profile',
            side_effect=TestFrameTimes.mock_profile)
        self.tmpdir = tmpdir

    def test_from_yaml_empty(self):
        """frame_times.from_yaml: profile using an empty YAML file"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (frame_times.from_yaml('empty.yml')
                    is status.PASS)
        self.m_qty_load_yaml.assert_called_once()
        s = f.getvalue()
        assert s == ''

    def test_from_yaml_one_trace(self):
        """frame_times.from_yaml: profile using a YAML with just one trace path"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (frame_times.from_yaml('one-trace.yml')
                    is status.PASS)
        self.m_qty_load_yaml.assert_called_once()
        self.m_ensure_file.assert_called_once()
        self.m_profile.assert_called_once()
        s = f.getvalue()
        assert s == ('[frame_times] ' + str(len(self.exp_frame_times)) + '\n')

    def test_from_yaml_two_traces(self):
        """frame_times.from_yaml: profile using a YAML with more than one trace path"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (frame_times.from_yaml('two-traces.yml')
                    is status.PASS)
        self.m_qty_load_yaml.assert_called_once()
        assert self.m_ensure_file.call_count == 1
        assert self.m_profile.call_count == 1
        s = f.getvalue()
        assert s == ('[frame_times] ' + str(len(self.exp_frame_times)) + '\n')

    def test_trace_success(self):
        """frame_times.trace: profile a trace successfully"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (frame_times.trace(self.trace_path)
                    is status.PASS)
        self.m_qty_load_yaml.assert_not_called()
        self.m_ensure_file.assert_called_once()
        self.m_profile.assert_called_once()
        s = f.getvalue()
        assert s.endswith('PIGLIT: '
                          '{"images": [{'
                          '"image_desc": "' + self.trace_path + '", '
                          '"frame_times": ' + str(self.exp_frame_times) +
                          '}], "result": "pass"}\n')

    def test_trace_fail(self):
        """frame_times.trace: fail profiling a trace"""

        fail_trace_path = "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr"
        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (frame_times.trace(fail_trace_path)
                    is status.CRASH)
        self.m_qty_load_yaml.assert_not_called()
        self.m_ensure_file.assert_called_once()
        self.m_profile.assert_called_once()
        s = f.getvalue()
        assert s.endswith('PIGLIT: '
                          '{"images": [{'
                          '"image_desc": "' + fail_trace_path + '", '
                          '"frame_times": null'
                          '}], "result": "crash"}\n')

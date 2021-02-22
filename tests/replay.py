# coding=utf-8
#
# Copyright (c) 2017 Intel Corporation
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

"""A profile that runs PiglitReplayerTest instances from a YAML description
file

This profile requires some configuration in piglit.conf, or the use of
environment variables.

In piglit.conf one should set the following:
[replay]:subcommand -- Replay command to run.
[replay]:description_file -- Path to the replay description file.
[replay]:device_name -- Name of the device selected for the replay.

Alternatively (or in addition, since environment variables have precedence),
one could set:
PIGLIT_REPLAY_SUBCOMMAND -- environment equivalent of [replay]:subcommand
PIGLIT_REPLAY_DESCRIPTION_FILE -- environment equivalent of [replay]:description_file
PIGLIT_REPLAY_DEVICE_NAME -- environment equivalent of [replay]:device_name

Optionally, in piglit.conf one could set also the following:
[replay]:extra_args -- Space-separated list of extra command line arguments for replayer.
[replay]:apitrace_bin -- Path to the apitrace executable.
[replay]:eglretrace_bin -- Path to the eglretrace (apitrace) executable.
[replay]:wine_bin -- Path to the Wine executable.
[replay]:wine_apitrace_bin -- Path to the Windows' apitrace executable to be run under Wine.
[replay]:wine_d3dretrace_bin -- Path to the Windows' d3dretrace (apitrace) executable to be run under Wine.
[replay]:gfxrecon-info_bin -- Path to the gfxrecon-info (GFXReconstruct) executable.
[replay]:gfxrecon-replay_bin -- Path to the gfxrecon-replay (GFXReconstruct) executable.
[replay]:gfxrecon-replay_extra_args -- Space-separated list of extra command line arguments for gfxrecon-replay.
[replay]:loop_times -- Number of times to replay the last frame in profile mode

Alternatively (or in addition, since environment variables have precedence),
one could set:
PIGLIT_REPLAY_EXTRA_ARGS -- environment equivalent of [replay]:extra_args
PIGLIT_REPLAY_APITRACE_BINARY -- environment equivalent of [replay]:apitrace_bin
PIGLIT_REPLAY_EGLRETRACE_BINARY -- environment equivalent of [replay]:eglretrace_bin
PIGLIT_REPLAY_WINE_BINARY -- environment equivalent of [replay]:wine_bin
PIGLIT_REPLAY_WINE_APITRACE_BINARY -- environment equivalent of [replay]:wine_apitrace_bin
PIGLIT_REPLAY_WINE_D3DRETRACE_BINARY -- environment equivalent of [replay]:wine_d3dretrace_bin
PIGLIT_REPLAY_GFXRECON_INFO_BINARY -- environment equivalent of [replay]:gfxrecon-info_bin
PIGLIT_REPLAY_GFXRECON_REPLAY_BINARY -- environment equivalent of [replay]:gfxrecon-replay_bin
PIGLIT_REPLAY_GFXRECON_REPLAY_EXTRA_ARGS -- environment equivalent of [replay]:gfxrecon-replay_extra_args
PIGLIT_REPLAY_LOOP_TIMES -- environment equivalent of [replay]:loop_times

"""

import collections

from os import path

from framework import core, exceptions, grouptools, profile, status
from framework.replay import query_traces_yaml as qty
from framework.test.base import DummyTest
from framework.test.piglit_test import PiglitReplayerTest

__all__ = ['profile']

_SUBCOMMAND = core.get_option('PIGLIT_REPLAY_SUBCOMMAND',
                              ('replay', 'subcommand'),
                              default='compare')

_DESCRIPTION_FILE = core.get_option('PIGLIT_REPLAY_DESCRIPTION_FILE',
                                    ('replay', 'description_file'),
                                    required=True)

_DEVICE_NAME = core.get_option('PIGLIT_REPLAY_DEVICE_NAME',
                               ('replay', 'device_name'),
                               required=True)

_EXTRA_ARGS = core.get_option('PIGLIT_REPLAY_EXTRA_ARGS',
                              ('replay', 'extra_args'),
                              default='').split()

class ReplayProfile(object):

    def __init__(self, subcommand, filename, device_name):
        try:
            with open(filename, 'r') as f:
                self.yaml = qty.load_yaml(f)
        except FileNotFoundError:
            raise exceptions.PiglitFatalError(
                'Cannot open "{}"'.format(filename))

        self.subcommand = subcommand
        self.device_name = device_name
        self.extra_args = ['--device-name', device_name,
                           '--download-url', qty.download_url(self.yaml)] \
                           + _EXTRA_ARGS
        self.forced_test_list = []
        self.filters = profile.Filters()
        self.options = {
            'dmesg': profile.get_dmesg(False),
            'monitor': profile.Monitoring(False),
            'ignore_missing': False,
        }

    def __len__(self):
        if not (self.filters or self.forced_test_list):
            return sum(1 for _ in qty.traces(self.yaml, device_name=self.device_name))
        return sum(1 for _ in self.itertests())

    def setup(self):
        """This hook sets the PiglitReplayerTest.results_path variable.

        Setting this variable allows the files created by PiglitReplayerTest to
        be placed into the results directory.
        """
        PiglitReplayerTest.RESULTS_PATH = self.results_dir

    def teardown(self):
        pass

    def _itertests(self):
        """Always iterates tests instead of using the forced test_list."""
        def _iter():
            # When profiling, only run apitrace traces
            trace_extensions = ".trace" if self.subcommand == 'profile' else None
            for t in qty.traces(self.yaml, trace_extensions=trace_extensions, device_name=self.device_name, checksum=True):
                group_path = path.join('trace', self.device_name, t['path'])
                k = grouptools.from_path(group_path)
                trace_extra_args = [t['path']]
                if self.subcommand == 'compare':
                    trace_extra_args.append(t['checksum'])
                v = PiglitReplayerTest(self.subcommand, self.extra_args + trace_extra_args)
                yield k, v

        for k, v in self.filters.run(_iter()):
            yield k, v

    def itertests(self):
        if self.forced_test_list:
            alltests = dict(self._itertests())
            opts = collections.OrderedDict()
            for n in self.forced_test_list:
                if self.options['ignore_missing'] and n not in alltests:
                    opts[n] = DummyTest(n, status.NOTRUN)
                else:
                    opts[n] = alltests[n]
            return opts.items()
        else:
            return iter(self._itertests())


profile = ReplayProfile(_SUBCOMMAND, _DESCRIPTION_FILE, _DEVICE_NAME)

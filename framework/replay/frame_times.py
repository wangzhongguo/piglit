# coding=utf-8
#
# Copyright © 2020-2021 Collabora Ltd.
# Copyright © 2020 Valve Corporation.
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

import json  # type: ignore
from os import path

from framework import status
from framework.replay import backends
from framework.replay import query_traces_yaml as qty
from framework.replay.backends.apitrace import APITraceBackend
from framework.replay.download_utils import ensure_file
from framework.replay.options import OPTIONS

__all__ = ['from_yaml',
           'trace']


def _replay(trace_path):
    try:
        success = True
        frame_times = APITraceBackend.profile(trace_path)
    except (backends.DumpBackendNotImplementedError,
            backends.DumpBackendError) as e:
        print(e)
        success = False

    if not success:
        print("[frame_times] Trace {} couldn't be replayed. "
              "See above logs for more information.".format(trace_path))
        return None
    else:
        return frame_times


def _run_trace(trace_path):
    ensure_file(trace_path)

    json_result = {}

    frame_times = _replay(path.join(OPTIONS.db_path, trace_path))
    if frame_times is None:
        print('[frame_times] error')
    else:
        print(f'[frame_times] {format(len(frame_times))}')

    json_result['images'] = [
        {'image_desc': trace_path,
         'frame_times': frame_times}]

    if frame_times is None:
        return status.CRASH, json_result

    return status.PASS, json_result


def _print_result(result, trace_path, json_result):
    output = 'PIGLIT: '
    json_result['result'] = str(result)

    output += json.dumps(json_result)
    print(output)


def from_yaml(yaml_file):
    y = qty.load_yaml(yaml_file)

    OPTIONS.set_download_url(qty.download_url(y))

    global_result = status.PASS
    # TODO: print in subtest format
    # json_results = {}
    t_list = qty.traces(y, trace_extensions=".trace", device_name=OPTIONS.device_name)
    for t in t_list:
        result, json_result = _run_trace(t['path'])
        if result is not status.PASS and global_result is not status.CRASH:
            global_result = result
        # json_results.update(json_result)
        # _print_result(result, t['path'], json_result)

    return global_result


def trace(trace_path):
    result, json_result = _run_trace(trace_path)
    _print_result(result, trace_path, json_result)

    return result

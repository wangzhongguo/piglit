# coding=utf-8
#
# Copyright (c) 2020 Collabora Ltd
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
import os
import shutil
from glob import glob
from os import path

from framework import core, status
from framework.replay import backends
from framework.replay import query_traces_yaml as qty
from framework.replay.download_utils import ensure_file
from framework.replay.image_checksum import hexdigest_from_image
from framework.replay.options import OPTIONS

__all__ = ['from_yaml',
           'trace']


def _replay(trace_path, results_path):
    try:
        success = backends.dump(trace_path, results_path, [])
    except (backends.DumpBackendNotImplementedError,
            backends.DumpBackendError) as e:
        print(e)
        success = False

    if not success:
        print("[check_image] Trace {} couldn't be replayed. "
              "See above logs for more information.".format(trace_path))
        return None, None
    else:
        file_name = path.basename(trace_path)
        files = glob(path.join(results_path, file_name + '-*' + '.png'))
        if not files:
            print('[check_image] No dumped files found '
                  'in the results path "{}". '
                  'See above logs for more information.'.format(results_path))
            return None, None
        image_file = files[0]
        return hexdigest_from_image(image_file), image_file


def _check_trace(trace_path, expected_checksum):
    ensure_file(trace_path)

    json_result = {}

    trace_dir = path.dirname(trace_path)
    dir_in_results = path.join('trace', OPTIONS.device_name or '', trace_dir)
    results_path = path.join(OPTIONS.results_path, dir_in_results)
    core.check_dir(results_path)

    checksum, image_file = _replay(path.join(OPTIONS.db_path, trace_path),
                                   results_path)

    print('[check_image]\n'
          '    actual: {}\n'
          '  expected: {}'.format(checksum or 'error', expected_checksum))

    json_result['images'] = [
        {'image_desc': trace_path,
         'checksum_ref': expected_checksum,
         'checksum_render': None,
         'image_ref': expected_checksum + '.png',
         'image_render': None}]

    if checksum is None:
        return status.CRASH, json_result

    json_result['images'][0]['checksum_render'] = checksum
    json_result['images'][0]['image_render'] = checksum + '.png'

    if checksum == expected_checksum:
        if not OPTIONS.keep_image:
            os.remove(image_file)
        print('[check_image] Images match for:\n  {}\n'.format(trace_path))
        result = status.PASS
    else:
        print('[check_image] Images differ for:\n  {}'.format(trace_path))
        print('[check_image] For more information see '
              'https://gitlab.freedesktop.org/'
              'mesa/piglit/blob/master/replayer/README.md\n')
        result = status.FAIL

    if result is not status.PASS or OPTIONS.keep_image:
        root, ext = path.splitext(image_file)
        image_file_dest = '{}-{}{}'.format(root, checksum, ext)
        shutil.move(image_file, image_file_dest)
        json_result['images'][0]['image_render'] = image_file_dest

    return result, json_result


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
    t_list = qty.traces(y, device_name=OPTIONS.device_name, checksum=True)
    for t in t_list:
        result, json_result = _check_trace(t['path'], t['checksum'])
        if result is not status.PASS and global_result is not status.CRASH:
            global_result = result
        # json_results.update(json_result)
        # _print_result(result, t['path'], json_result)

    return global_result


def trace(trace_path, expected_checksum):
    result, json_result = _check_trace(trace_path, expected_checksum)
    _print_result(result, trace_path, json_result)

    return result

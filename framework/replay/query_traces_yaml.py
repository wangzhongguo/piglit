# coding=utf-8
#
# Copyright © 2019, 2022 Collabora Ltd
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

import yaml

from os import path

from typing import Any, Generator, Optional, Union
from framework import exceptions


__all__ = ['download_url',
           'load_yaml',
           'trace_checksum',
           'traces']


def load_yaml(y):
    try:
        return yaml.safe_load(y) or {}
    except yaml.YAMLError:
        raise exceptions.PiglitFatalError(
            'Cannot use the provided stream. Is it YAML?')


def trace_checksum(trace: Any, device_name: Optional[str]) -> str:
    '''returns checksum of trace'''
    try:
        data = trace[device_name]
        for item, val in data.items():
            if item == "checksum":
                return val
        return ''

    except StopIteration:
        return ''
    except KeyError:
        return ''


def download_url(y):
    try:
        return y['traces-db']['download-url'] if 'traces-db' in y else None
    except KeyError:
        return None


def traces(
    y: Any,
    trace_extensions: Optional[str] = None,
    device_name: Optional[str] = None,
    checksum: Union[str, bool] = False
) -> Generator[dict, None, None]:

    def _trace_extension(trace_path: str) -> str:
        name, extension = path.splitext(trace_path)

        return extension
    traces = y.get('traces', {}) or {}

    if trace_extensions is not None:
        extensions = trace_extensions.split(',')

        def _filter_trace_extension(trace: str) -> bool:
            try:
                return _trace_extension(trace) in extensions
            except KeyError:
                return False

        traces = {t: data for t, data in traces.items() if _filter_trace_extension(t)}

    for trace_file, devdata in traces.items():
        for dev, properties in devdata.items():
            if device_name and (device_name != dev):
                continue
            if (
                properties
                and "label" in properties
                and isinstance(properties["label"], list)
                and {"skip", "hang", "crash", "fail", "unsupported"}.intersection(properties["label"])
            ):
                continue

            found_trace = {"path": trace_file}
            if not checksum:
                yield found_trace
                break

            try:
                found_trace["checksum"] = trace_checksum(devdata, device_name)
                yield found_trace
            except KeyError:
                yield {}

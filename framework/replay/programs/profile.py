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

import argparse

from framework import exceptions
from framework.replay import frame_times
from framework.replay import options
from framework.programs import parsers as piglit_parsers
from . import parsers

__all__ = ['profile']


def _from_yaml(args):
    options.OPTIONS.device_name = args.device_name
    options.OPTIONS.set_download_caching_proxy_url(args.download_caching_proxy_url)
    options.OPTIONS.download['force'] = args.force_download
    options.OPTIONS.download['minio_host'] = args.download_minio_host
    options.OPTIONS.download['minio_bucket'] = args.download_minio_bucket
    options.OPTIONS.download['role_session_name'] = args.download_role_session_name
    options.OPTIONS.download['jwt'] = args.download_jwt
    options.OPTIONS.db_path = args.db_path
    options.OPTIONS.results_path = args.output

    return frame_times.from_yaml(args.yaml_file)


def _trace(args):
    options.OPTIONS.device_name = args.device_name
    options.OPTIONS.set_download_url(args.download_url)
    options.OPTIONS.set_download_caching_proxy_url(args.download_caching_proxy_url)
    options.OPTIONS.download['force'] = args.force_download
    options.OPTIONS.download['minio_host'] = args.download_minio_host
    options.OPTIONS.download['minio_bucket'] = args.download_minio_bucket
    options.OPTIONS.download['role_session_name'] = args.download_role_session_name
    options.OPTIONS.download['jwt'] = args.download_jwt
    options.OPTIONS.db_path = args.db_path
    options.OPTIONS.results_path = args.output

    return frame_times.trace(args.file_path)


@exceptions.handler
def profile(input_):
    """ Parser for replayer compare command """
    unparsed = piglit_parsers.parse_config(input_)[1]

    try:
        # Set the parent of the config to add the -f/--config message
        parser = argparse.ArgumentParser(parents=[piglit_parsers.CONFIG])
        # The "required" keyword is only available since python >= 3.7
        subparsers = parser.add_subparsers(dest='command', required=True)
    except TypeError:
        parser = argparse.ArgumentParser(parents=[piglit_parsers.CONFIG])
        # Add a destination due to
        # https://github.com/python/cpython/pull/3027#issuecomment-330910633
        subparsers = parser.add_subparsers(dest='command')

    parser_trace = subparsers.add_parser(
        'trace',
        parents=[parsers.DEVICE,
                 parsers.DOWNLOAD_URL,
                 parsers.DOWNLOAD_CACHING_PROXY_URL,
                 parsers.DOWNLOAD_FORCE,
                 parsers.DOWNLOAD_MINIO_HOST,
                 parsers.DOWNLOAD_MINIO_BUCKET,
                 parsers.DOWNLOAD_ROLE_SESSION_NAME,
                 parsers.DOWNLOAD_JWT,
                 parsers.DB_PATH,
                 parsers.RESULTS_PATH],
        help=('Profiles specific trace given a device.'))
    parser_trace.add_argument(
        'file_path',
        help=('the relative path to the trace file inside the db path. '
              'If not present and given that an URL has been provided '
              'for its download, the relative path to the file in such URL.'))
    parser_trace.set_defaults(func=_trace)

    parser_yaml = subparsers.add_parser(
        'yaml',
        parents=[parsers.DEVICE,
                 parsers.YAML,
                 parsers.DOWNLOAD_CACHING_PROXY_URL,
                 parsers.DOWNLOAD_FORCE,
                 parsers.DOWNLOAD_MINIO_HOST,
                 parsers.DOWNLOAD_MINIO_BUCKET,
                 parsers.DOWNLOAD_ROLE_SESSION_NAME,
                 parsers.DOWNLOAD_JWT,
                 parsers.DB_PATH,
                 parsers.RESULTS_PATH],
        help=('Profiles from a traces description file listing traces.'))
    parser_yaml.set_defaults(func=_from_yaml)

    args = parser.parse_args(unparsed)

    return args.func(args)

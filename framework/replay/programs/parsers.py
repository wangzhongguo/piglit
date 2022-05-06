# coding=utf-8
#
# Copyright (c) 2015-2016, 2019 Intel Corporation
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

import argparse



class FileContentType(argparse.FileType):
    """A FileType type argument which reads the file content
    and closes the file"""

    def __init__(self, bufsize=-1, encoding=None, errors=None):
        super().__init__(
            mode="r", bufsize=bufsize, encoding=encoding, errors=errors
        )

    def __call__(self, string: str):
        with super().__call__(string) as open_file:
            return open_file.read()


DEVICE = argparse.ArgumentParser(add_help=False)
DEVICE.add_argument(
    '-d', '--device-name',
    dest='device_name',
    required=False,
    default=None,
    help='the name of the graphics device used to replay traces')

KEEP_IMAGE = argparse.ArgumentParser(add_help=False)
KEEP_IMAGE.add_argument(
    '-k', '--keep-image',
    dest='keep_image',
    action='store_true',
    help=('forces keeping the image generated '
          'for the comparison.'))

YAML = argparse.ArgumentParser(add_help=False)
YAML.add_argument(
    '-y', '--yaml-file',
    dest='yaml_file',
    required=True,
    type=argparse.FileType('r'),
    help=('the name of the traces YAML description file listing traces '
          'and their checksums for each device'))

DOWNLOAD_URL = argparse.ArgumentParser(add_help=False)
DOWNLOAD_URL.add_argument(
    '-u', '--download-url',
    dest='download_url',
    required=False,
    default=None,
    help=('the URL from which to download the files'))

DOWNLOAD_CACHING_PROXY_URL = argparse.ArgumentParser(add_help=False)
DOWNLOAD_CACHING_PROXY_URL.add_argument(
    '-c', '--download-caching-proxy-url',
    dest='download_caching_proxy_url',
    required=False,
    default=None,
    help=('the URL for the caching proxy to be used as a prefix '
          'for the URL provided via --download-url'))

DOWNLOAD_FORCE = argparse.ArgumentParser(add_help=False)
DOWNLOAD_FORCE.add_argument(
    '-w', '--force-download',
    dest='force_download',
    action='store_true',
    help=('forces downloading '
          'even if the destination file already exists'))

DOWNLOAD_MINIO_HOST = argparse.ArgumentParser(add_help=False)
DOWNLOAD_MINIO_HOST.add_argument(
    '-m', '--minio_host',
    dest='download_minio_host',
    required=False,
    default=None,
    help=('name of MinIO server from which to download traces'))

DOWNLOAD_MINIO_BUCKET = argparse.ArgumentParser(add_help=False)
DOWNLOAD_MINIO_BUCKET.add_argument(
    '-b', '--minio_bucket',
    dest='download_minio_bucket',
    required=False,
    default=None,
    help=('Name of bucket in MinIO server containing the traces'))

DOWNLOAD_ROLE_SESSION_NAME = argparse.ArgumentParser(add_help=False)
DOWNLOAD_ROLE_SESSION_NAME.add_argument(
    '-r', '--role-session-name',
    dest='download_role_session_name',
    required=False,
    default=None,
    help=('role session name for authentication with MinIO'))

DOWNLOAD_JWT = argparse.ArgumentParser(add_help=False)
download_jwt_group = DOWNLOAD_JWT.add_mutually_exclusive_group()
download_jwt_group.add_argument(
    '-j', '--jwt',
    dest='download_jwt',
    required=False,
    default=None,
    help=('JWT token for authentication with MinIO'))
download_jwt_group.add_argument(
    "--jwt-file",
    dest="download_jwt",
    required=False,
    default=None,
    type=FileContentType(),
    help=("File containing JWT token for authentication with MinIO"),
)

DB_PATH = argparse.ArgumentParser(add_help=False)
DB_PATH.add_argument(
    '-p', '--db-path',
    dest='db_path',
    required=False,
    default='./replayer-db/',
    help=('the path to the objects db or where it will be created. '
          'Defaults to "./replayer-db/".'))

RESULTS_PATH = argparse.ArgumentParser(add_help=False)
RESULTS_PATH.add_argument(
    '-o', '--output',
    dest='output',
    required=False,
    default='./results/',
    help=('the path in which to place the results. Defaults to "./results/"'))

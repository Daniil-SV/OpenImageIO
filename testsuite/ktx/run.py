#!/usr/bin/env python

# Copyright Contributors to the OpenImageIO project.
# SPDX-License-Identifier: Apache-2.0
# https://github.com/AcademySoftwareFoundation/OpenImageIO

redirect = ' >> out.txt 2>&1 '
files = [
    "3dtex_7_reference_u.ktx2",
    "cubemap.ktx2",
    "gradient.ktx",
    "oiio-logo-with-alpha.ktx2",
    "orient-down-metadata.ktx",
    "orient-up-metadata.ktx",
    "oiio-logo-no-alpha-rgb32.ktx2",
    "neko-rgba16.ktx",
    "premultiplied-rgba.ktx2",
    "straight-rgba.ktx2"
]
for f in files:
    command += info_command (OIIO_TESTSUITE_IMAGEDIR + "/" + f)
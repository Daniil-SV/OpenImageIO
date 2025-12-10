// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#pragma once

#include <OpenImageIO/filesystem.h>
#include <ktx.h>

OIIO_NAMESPACE_BEGIN
namespace ktx {

ktx_error_code_e
ktxIoProxyStream_construct(ktxStream& stream, Filesystem::IOProxy* proxy);

}  // namespace ktx
OIIO_NAMESPACE_END
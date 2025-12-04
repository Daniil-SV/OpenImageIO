// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#pragma once

#include <OpenImageIO/filesystem.h>
#include <ktx.h>

OIIO_NAMESPACE_BEGIN
namespace ktx {

static void
IoProxyStream_close(ktxStream* stream);

static ktx_error_code_e
IoProxyStream_getsize(ktxStream* stream, ktx_size_t* size);

static ktx_error_code_e
IoProxyStream_setpos(ktxStream* stream, ktx_off_t pos);

static ktx_error_code_e
IoProxyStream_getpos(ktxStream* stream, ktx_off_t* pos);

static ktx_error_code_e
IoProxyStream_write(ktxStream* stream, const void* src, ktx_size_t size,
                    ktx_size_t count);

static ktx_error_code_e
IoProxyStream_skip(ktxStream* stream, ktx_size_t count);

static ktx_error_code_e
IoProxyStream_read(ktxStream* stream, void* dst, const ktx_size_t count);

ktx_error_code_e
ktxIoProxyStream_construct(ktxStream& stream, Filesystem::IOProxy* proxy);

}  // namespace ktx
OIIO_NAMESPACE_END
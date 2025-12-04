// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO


#include "stream.h"

OIIO_NAMESPACE_BEGIN
namespace ktx {

static Filesystem::IOProxy*
parent(ktxStream* stream)
{
    if (stream)
        return (Filesystem::IOProxy*)stream->data.custom_ptr.address;

    return nullptr;
}

void
IoProxyStream_close(ktxStream*)
{
}

ktx_error_code_e
IoProxyStream_getsize(ktxStream* stream, ktx_size_t* size)
{
    Filesystem::IOProxy* proxy = parent(stream);

    if (!proxy)
        return KTX_INVALID_VALUE;

    *size = proxy->size();
    return KTX_SUCCESS;
}

ktx_error_code_e
IoProxyStream_setpos(ktxStream* stream, ktx_off_t pos)
{
    Filesystem::IOProxy* proxy = parent(stream);

    if (!proxy)
        return KTX_INVALID_VALUE;

    return proxy->seek(pos, SEEK_SET) ? KTX_SUCCESS : KTX_FILE_SEEK_ERROR;
}

ktx_error_code_e
IoProxyStream_getpos(ktxStream* stream, ktx_off_t* pos)
{
    Filesystem::IOProxy* proxy = parent(stream);

    if (!proxy || !pos)
        return KTX_INVALID_VALUE;

    *pos = proxy->tell();
    return KTX_SUCCESS;
}

ktx_error_code_e
IoProxyStream_write(ktxStream* stream, const void* src, ktx_size_t size,
                    ktx_size_t count)
{
    Filesystem::IOProxy* proxy = parent(stream);

    if (!proxy || !src)
        return KTX_INVALID_VALUE;

    if (count == 0)
        return KTX_SUCCESS;

    if (proxy->mode() != Filesystem::IOProxy::Mode::Write)
        return KTX_FILE_WRITE_ERROR;

    const auto ntotal = size * count;
    return (proxy->write(src, ntotal) == ntotal) ? KTX_SUCCESS
                                                 : KTX_FILE_WRITE_ERROR;
}

ktx_error_code_e
IoProxyStream_skip(ktxStream* stream, ktx_size_t count)
{
    Filesystem::IOProxy* proxy = parent(stream);

    if (!proxy)
        return KTX_INVALID_VALUE;

    if (count == 0)
        return KTX_SUCCESS;

    return proxy->seek(count, SEEK_CUR) ? KTX_SUCCESS : KTX_FILE_SEEK_ERROR;
}

static ktx_error_code_e
IoProxyStream_read(ktxStream* stream, void* dst, const ktx_size_t count)
{
    Filesystem::IOProxy* proxy = parent(stream);
    ktx_size_t nread           = 0;

    if (!proxy || !dst)
        return KTX_INVALID_VALUE;

    if (count == 0)
        return KTX_SUCCESS;

    if (proxy->mode() != Filesystem::IOProxy::Mode::Read)
        return KTX_FILE_READ_ERROR;

    nread = proxy->read(dst, count);
    return (nread == count) ? KTX_SUCCESS : KTX_FILE_UNEXPECTED_EOF;
}

ktx_error_code_e
ktxIoProxyStream_construct(ktxStream& stream, Filesystem::IOProxy* proxy)
{
    stream.type            = eStreamTypeCustom;
    stream.closeOnDestruct = false;

    stream.data.custom_ptr.address          = (void*)proxy;
    stream.data.custom_ptr.allocatorAddress = nullptr;  // N/A
    stream.data.custom_ptr.size             = 0;        // N/A

    stream.read     = IoProxyStream_read;
    stream.skip     = IoProxyStream_skip;
    stream.write    = IoProxyStream_write;
    stream.getpos   = IoProxyStream_getpos;
    stream.setpos   = IoProxyStream_setpos;
    stream.getsize  = IoProxyStream_getsize;
    stream.destruct = IoProxyStream_close;

    return KTX_SUCCESS;
}

}  // namespace ktx
OIIO_NAMESPACE_END
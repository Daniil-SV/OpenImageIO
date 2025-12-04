// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imageio.h>

#include "stream.h"
#include <ktx.h>

#include "format.h"

OIIO_PLUGIN_NAMESPACE_BEGIN
namespace ktx {

class KtxInput final : public ImageInput {
public:
    KtxInput() {}
    ~KtxInput() override { close(); }
    const char* format_name() const override { return "ktx"; }
    int supports(string_view feature) const override
    {
        return feature == "ioproxy" || feature == "mipmap"
               || feature == "arbitrary_metadata" || feature == "multiimage";
    }

    bool valid_file(Filesystem::IOProxy* ioproxy) const override;
    bool open(const std::string& name, ImageSpec& spec) override;
    bool open(const std::string& name, ImageSpec& newspec,
              const ImageSpec& config) override;
    bool close() override;
    int current_subimage(void) const override
    {
        lock_guard lock(*this);
        return m_subimage;
    }
    int current_miplevel(void) const override
    {
        lock_guard lock(*this);
        return m_miplevel;
    }
    bool seek_subimage(int subimage, int miplevel) override;
    bool read_native_scanline(int subimage, int miplevel, int y, int z,
                              void* data) override;
    bool read_native_tile(int subimage, int miplevel, int x, int y, int z,
                          void* data) override;

private:
    void load_format_descriptor();

private:
    ktxTexture* m_tex = nullptr;

    // Whole ktx texture data buffer
    std::vector<uint8_t> m_buffer;

    // Vector with current uncompressed face data
    // Used in cases when data is compressed
    std::vector<uint8_t> m_uncompressed_buffer;

    // Pointer to current mip map level of current subimage
    // to read scanlines or tiles from
    uint8_t* m_data = nullptr;

    int m_subimage = -1;
    int m_miplevel = -1;
    glDataDescriptor m_format;
};

static bool
compare_magic(const ktx_uint8_t* ref, const ktx_uint8_t* magic, size_t length)
{
    for (size_t i = 0; length > i; i++) {
        if (ref[i] != magic[i])
            return false;
    }

    return true;
}

bool
KtxInput::valid_file(Filesystem::IOProxy* ioproxy) const
{
    if (!ioproxy || ioproxy->mode() != Filesystem::IOProxy::Mode::Read)
        return false;

    constexpr size_t ident_size                     = 12;
    constexpr ktx_uint8_t ktx_ident_ref[ident_size] = {
        0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
    };

    constexpr ktx_uint8_t ktx2_ident_ref[ident_size] = {
        0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
    };

    ktx_uint8_t magic[ident_size] {};
    const size_t numRead = ioproxy->pread(&magic, ident_size, 0);

    return numRead == ident_size
           && (compare_magic(ktx_ident_ref, magic, ident_size)
               || compare_magic(ktx2_ident_ref, magic, ident_size));
}

bool
KtxInput::open(const std::string& name, ImageSpec& newspec)
{
    if (!ioproxy_use_or_open(name))
        return false;

    ioseek(0);

    ktx_error_code_e status = KTX_SUCCESS;
    ktxStream stream;
    ktxIoProxyStream_construct(stream, ioproxy());

    status = ktxTexture_CreateFromStream(&stream, KTX_TEXTURE_CREATE_NO_FLAGS,
                                         &m_tex);
    if (status != KTX_SUCCESS)
        return false;

    size_t data_size = ktxTexture_GetDataSizeUncompressed(m_tex);
    m_buffer.resize(data_size);
    ktxTexture_LoadImageData(m_tex, (ktx_uint8_t*)m_buffer.data(), data_size);

    load_format_descriptor();
    if (m_format.channels_count == 0 || m_format.type == TypeDesc::UNKNOWN)
        return false;

    if (!seek_subimage(0, 0))
        return false;

    newspec = spec();
    return true;
}

bool
KtxInput::open(const std::string& name, ImageSpec& spec,
               const ImageSpec& config)
{
    ioproxy_retrieve_from_config(config);
    return open(name, spec);
}

bool
KtxInput::close()
{
    if (m_tex) {
        ktxTexture_Destroy(m_tex);
        m_tex = nullptr;
    }

    ioproxy_clear();
    return true;
}

bool
KtxInput::seek_subimage(int subimage, int miplevel)
{
    lock_guard lock(*this);
    ktx_error_code_e status = KTX_SUCCESS;

    if (subimage != 0)
        return false;

    // early out
    if (subimage == current_subimage() && miplevel == current_miplevel())
        return true;

    // checking for proper mip map level
    if (miplevel >= m_tex->numLevels)
        return false;

    // checking for proper sub image index
    if (subimage >= m_tex->numFaces)
        return false;

    // Mip map width and height for KTX is calculated based on base dimension size value
    ktx_uint32_t w = std::max(1u, m_tex->baseWidth >> miplevel);
    ktx_uint32_t h = std::max(1u, m_tex->baseHeight >> miplevel);
    ktx_uint32_t d = 0;
    if (m_tex->baseDepth) {
        d = std::max(1u, m_tex->baseDepth >> miplevel);
    }

    if (m_tex->isCubemap || m_tex->isArray) {
        // TODO: Cubemaps
        return false;
    } else {
        m_spec       = ImageSpec(w, h, m_format.channels_count, m_format.type);
        m_spec.depth = d;
        m_spec.channelnames = m_format.channel_order;
    }

    ktx_size_t offset = 0;
    status = ktxTexture_GetImageOffset(m_tex, miplevel, 0, subimage, &offset);
    if (status != KTX_SUCCESS)
        return false;

    m_data = m_buffer.data() + offset;
    if (m_format.compressed) {
        // TODO: Implement decompress
    }

    m_subimage = subimage;
    m_miplevel = miplevel;
    return true;
}

bool
KtxInput::read_native_scanline(int subimage, int miplevel, int y, int z,
                               void* data)
{
    lock_guard lock(*this);

    // don't proceed if a cube map - use tiles then instead
    if (m_tex->isCubemap)
        return false;

    if (y < 0 || y >= m_spec.height)  // out of range scanline
        return false;

    if (!seek_subimage(subimage, miplevel))
        return false;

    size_t size = spec().scanline_bytes();
    memcpy(data, m_data + z * m_spec.height * size + y * size, size);
    return true;
}

bool
KtxInput::read_native_tile(int subimage, int miplevel, int x, int y, int z,
                           void* data)
{
    // don't proceed if not a cube map - use scanlines then instead
    if (!m_tex->isCubemap)
        return false;

    // TODO: cubemaps
    return false;
}

void
KtxInput::load_format_descriptor()
{
    ktx_uint32_t glFormat         = 0;
    ktx_uint32_t glInternalFormat = 0;
    if (m_tex->classId == ktxTexture1_c) {
        ktxTexture1* ktx1 = (ktxTexture1*)m_tex;
        glFormat          = ktx1->glFormat;

        // Compressed textures
        if (glFormat == 0)
            glFormat = ktx1->glBaseInternalformat;

        glInternalFormat = ktx1->glInternalformat;

    } else if (m_tex->classId == ktxTexture2_c) {
        ktxTexture2* ktx2 = (ktxTexture2*)m_tex;

        // TODO: Ktx2
    }

    m_format = get_format_descriptor(glFormat, glInternalFormat);
}

}  // namespace ktx

OIIO_PLUGIN_EXPORTS_BEGIN

OIIO_EXPORT ImageInput*
ktx_input_imageio_create()
{
    return new ktx::KtxInput;
}

OIIO_EXPORT int ktx_imageio_version = OIIO_PLUGIN_VERSION;

OIIO_EXPORT const char*
ktx_imageio_library_version()
{
    return nullptr;
}

OIIO_EXPORT const char* ktx_input_extensions[] = { "ktx", "ktx2", nullptr };

OIIO_PLUGIN_EXPORTS_END


OIIO_PLUGIN_NAMESPACE_END

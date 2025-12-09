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
    bool seek_subimage(int subimage, int face, int miplevel);
    bool read_native_scanline(int subimage, int miplevel, int y, int z,
                              void* data) override;
    bool read_native_tile(int subimage, int miplevel, int x, int y, int z,
                          void* data) override;

private:
    bool load_face_data(int subimage, int face, int miplevel);
    void load_format_descriptor();

    bool is_associated_alpha();

private:
    ktxTexture* m_tex = nullptr;    ///< Texture reader
    glDataDescriptor m_format;      ///< Info about data format
    std::vector<uint8_t> m_buffer;  ///< Source texture data buffer
    int m_subimage = -1;            ///< Current array (layer) image index
    int m_face     = -1;            ///< Current face (tile) index for cubemap
    int m_miplevel = -1;            ///< Current level
    bool m_keep_unassociated_alpha
        = false;                     ///< Do not convert unassociated alpha
    bool m_associated_alpha = true;  ///< Is data has associated alpha

    std::vector<uint8_t>
        m_temp_buffer;  ///< Temp buffer with current uncompressed face data
    uint8_t* m_data
        = nullptr;  ///< Pointer to current mip map level of current subimage
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

    // Basically convert glFormat into a form understandable for OIIO
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
    if (config.get_int_attribute("oiio:UnassociatedAlpha", 0) == 1)
        m_keep_unassociated_alpha = true;

    m_associated_alpha
        = config.get_int_attribute("ktx:associated",
                                   OIIO::get_int_attribute("ktx:associated"));

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
    return seek_subimage(subimage, 0, miplevel);
}

struct ktxAnimData {
    uint32_t duration;
    uint32_t timescale;
    uint32_t loopCount;
};

bool
KtxInput::seek_subimage(int subimage, int face, int miplevel)
{
    lock_guard lock(*this);

    // early out
    if (subimage == current_subimage() && face == m_face
        && miplevel == current_miplevel())
        return true;

    // checking for proper sub image index
    if (subimage >= m_tex->numLayers)
        return false;

    // checking for proper face index
    if (face >= m_tex->numFaces)
        return false;

    // checking for proper mip map level
    if (miplevel >= m_tex->numLevels)
        return false;

    // Mip map width and height for KTX is calculated based on base dimension size value
    ktx_uint32_t w = std::max(1u, m_tex->baseWidth >> miplevel);
    ktx_uint32_t h = std::max(1u, m_tex->baseHeight >> miplevel);
    ktx_uint32_t d = std::max(1u, m_tex->baseDepth >> miplevel);

    if (m_tex->isCubemap || m_tex->isArray) {
        m_spec = ImageSpec(w, h * m_tex->numFaces, m_format.channels_count,
                           m_format.type);
        m_spec.depth      = d;
        m_spec.tile_width = m_spec.full_width = w;
        m_spec.tile_height = m_spec.full_height = h;
        m_spec.tile_depth = m_spec.full_depth = d;
    } else {
        m_spec       = ImageSpec(w, h, m_format.channels_count, m_format.type);
        m_spec.depth = d;
        m_spec.channelnames = m_format.channel_order;
    }

    // fill the imagespec
    if (m_tex->isCubemap) {
        m_spec.attribute("textureformat", "CubeFace Environment");
    } else if (m_tex->numDimensions == 3) {
        m_spec.attribute("textureformat", "Volume Texture");
    } else {
        m_spec.attribute("textureformat", "Plain Texture");
    }

    m_spec.attribute("ktx:version", m_tex->classId == ktxTexture1_c ? 1 : 2);
    m_spec.attribute("oiio:subimages", m_tex->numLayers);
    m_spec.attribute("oiio:miplevels", m_tex->numLevels);

    if (m_format.colorspace == glColorspace::sRGB) {
        m_spec.set_colorspace("srgb_rec709_scene");
    } else {
        m_spec.set_colorspace("lin_rec709_scene");
    }

    ktxHashListEntry* kvEntry = m_tex->kvDataHead;
    while (kvEntry = ktxHashList_Next(kvEntry)) {
        ktx_error_code_e status = KTX_SUCCESS;
        unsigned int keyLen     = 0;
        unsigned int valueLen   = 0;
        char* key               = nullptr;
        void* value             = nullptr;

        status = ktxHashListEntry_GetKey(kvEntry, &keyLen, &key);
        if (status != KTX_SUCCESS)
            continue;

        status = ktxHashListEntry_GetValue(kvEntry, &valueLen, &value);
        if (status != KTX_SUCCESS)
            continue;

        auto name = std::string(key, key + keyLen);
        // Optional String keys
        if (name == "KTXwriter" || name == "KTXwriterScParams"
            || "KTXastcDecodeMode") {
            std::string str = std::string((char*)value,
                                          (char*)value + valueLen);
            m_spec.extra_attribs.attribute(name, TypeDesc::STRING, str);
        }
        // Optional Uint8 keys
        else if (name == "KTXcubemapIncomplete") {
            m_spec.extra_attribs.attribute(name, TypeDesc::UINT8, value);
        } else if (name == "KTXanimData") {
            // TODO: need testing
            //ktxAnimData* anim = (ktxAnimData*)value;
            //m_spec.attribute("oiio:Movie", 1);
            //
            //int fps[2] = { anim->timescale, anim->duration };
            //m_spec.attribute("FramesPerSecond", TypeRational, &fps);
            //m_spec.attribute("oiio:LoopCount", anim->loopCount);

        } else {
            // Store in buffer in others cases
            // since we can't say for sure is it string or something other
            span<char> buf = span<char>((char*)value, (char*)value + valueLen);
            m_spec.extra_attribs.attribute(name, TypeDesc::CHAR, valueLen, buf);
        }
    }

    int orientation = 0;
    // We need to invert the axes because the orientation in oiio
    // works a bit differently than in KTX
    // Also, in ktx there is also an orientation for 3D texture, z-axis,
    // but I'm not sure if it's worth doing anything with it, so we'll just ignore it for now
    {
        ktxOrientationX X = (m_tex->orientation.x == KTX_ORIENT_X_LEFT)
                                ? KTX_ORIENT_X_RIGHT
                                : KTX_ORIENT_X_LEFT;

        ktxOrientationY Y = (m_tex->orientation.y == KTX_ORIENT_Y_UP)
                                ? KTX_ORIENT_Y_DOWN
                                : KTX_ORIENT_Y_UP;

        if (X == KTX_ORIENT_X_LEFT && Y == KTX_ORIENT_Y_UP) {
            orientation = 1;
        } else if (X == KTX_ORIENT_X_RIGHT && Y == KTX_ORIENT_Y_UP) {
            orientation = 2;
        } else if (X == KTX_ORIENT_X_RIGHT && Y == KTX_ORIENT_Y_DOWN) {
            orientation = 3;
        } else if (X == KTX_ORIENT_X_LEFT && Y == KTX_ORIENT_Y_DOWN) {
            orientation = 4;
        } else if (Y == KTX_ORIENT_Y_UP && X == KTX_ORIENT_X_LEFT) {
            orientation = 5;
        } else if (Y == KTX_ORIENT_Y_UP && X == KTX_ORIENT_X_RIGHT) {
            orientation = 6;
        } else if (Y == KTX_ORIENT_Y_DOWN && X == KTX_ORIENT_X_LEFT) {
            orientation = 8;
        } else if (Y == KTX_ORIENT_Y_DOWN && X == KTX_ORIENT_X_RIGHT) {
            orientation = 7;
        } else {
            orientation = 0;
        }
    }

    m_spec.attribute("Orientation", orientation);

    // Setup default channel names if the format does not explicitly set them
    if (m_format.channel_order.empty())
        m_spec.default_channel_names();

    // Finally, load face data and setup data pointer to it
    if (!load_face_data(subimage, face, miplevel))
        return false;

    m_subimage = subimage;
    m_face     = face;
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

    // make sure we get the right dimensions
    if (x % m_spec.tile_width || y % m_spec.tile_height
        || z % m_spec.tile_width)
        return false;

    if (!seek_subimage(subimage, y / m_spec.tile_height, miplevel))
        return false;

    memcpy(data, m_data, m_spec.tile_bytes());
    return true;
}

bool
KtxInput::load_face_data(int subimage, int face, int miplevel)
{
    ktx_error_code_e status = KTX_SUCCESS;

    ktx_size_t offset = 0;
    status = ktxTexture_GetImageOffset(m_tex, miplevel, subimage, face,
                                       &offset);
    if (status != KTX_SUCCESS)
        return false;

    m_data = m_buffer.data() + offset;
    if (m_format.compression != Compression::None) {
        return false;
        // TODO: Implement decompress
    }

    // Processing straight alpha
    if (!m_keep_unassociated_alpha && !is_associated_alpha()) {
        ImageBuf source_buffer(m_spec, span<std::byte>((std::byte*)m_data,
                                                       m_spec.image_bytes()));

        m_temp_buffer.resize(m_spec.image_bytes());
        ImageBuf destination_buffer(
            m_spec, span<std::byte>((std::byte*)m_temp_buffer.data(),
                                    m_spec.image_bytes()));

        ImageBufAlgo::premult(destination_buffer, source_buffer);
        m_data = m_temp_buffer.data();
    }

    return true;
}

void
KtxInput::load_format_descriptor()
{
    if (m_tex->classId == ktxTexture1_c) {
        ktx_uint32_t glFormat         = 0;
        ktx_uint32_t glInternalFormat = 0;

        ktxTexture1* ktx1 = (ktxTexture1*)m_tex;
        glFormat          = ktx1->glFormat;

        // Compressed textures
        if (glFormat == 0)
            glFormat = ktx1->glBaseInternalformat;

        glInternalFormat = ktx1->glInternalformat;

        m_format = get_format_descriptor(glFormat, glInternalFormat);

    } else if (m_tex->classId == ktxTexture2_c) {
        ktxTexture2* ktx2 = (ktxTexture2*)m_tex;

        m_format = get_vk_format_descriptor(ktx2->vkFormat);
    }
}

bool
KtxInput::is_associated_alpha()
{
    if (m_tex->classId == ktxTexture1_c) {
        return m_associated_alpha;
    } else if (m_tex->classId == ktxTexture2_c) {
        return ktxTexture2_GetPremultipliedAlpha((ktxTexture2*)m_tex);
    }

    return true;
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

// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#include "format.h"
#include "oiio_gl/gl_format.h"
#include "oiio_gl/vk2gl.h"

OIIO_NAMESPACE_BEGIN
namespace ktx {
int
get_format_channels(ktx_uint32_t glFormat)
{
    switch (glFormat) {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_SLUMINANCE:
    case GL_INTENSITY:
    case GL_RED_INTEGER:
    case GL_GREEN_INTEGER:
    case GL_BLUE_INTEGER:
    case GL_ALPHA_INTEGER:
    case GL_LUMINANCE_INTEGER:
    case GL_COLOR_INDEX:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT: return 1;

    case GL_RG:
    case GL_LUMINANCE_ALPHA:
    case GL_SLUMINANCE_ALPHA:
    case GL_LUMINANCE_ALPHA_INTEGER:
    case GL_RG_INTEGER:
    case GL_DEPTH_STENCIL: return 2;

    case GL_RGB:
    case GL_BGR:
    case GL_RGB_INTEGER:
    case GL_BGR_INTEGER: return 3;

    case GL_RGBA:
    case GL_BGRA:
    case GL_RGBA_INTEGER:
    case GL_BGRA_INTEGER: return 4;
    }

    return 0;
}

glColorspace
get_format_colorspace(ktx_uint32_t glInternalFormat)
{
    switch (glInternalFormat) {
    case GL_SR8:
    case GL_SRG8:
    case GL_SRGB8:
    case GL_SRGB8_ALPHA8: return glColorspace::sRGB;
    default: return glColorspace::Linear;
    }
}

TypeDesc::BASETYPE
get_format_datatype(ktx_uint32_t glInternalFormat)
{
    using Type = TypeDesc::BASETYPE;
    switch (glInternalFormat) {
    case GL_R8:
    case GL_RG8:
    case GL_RGB8:
    case GL_RGBA8:

    case GL_R8UI:
    case GL_RG8UI:
    case GL_RGB8UI:
    case GL_RGBA8UI:

    case GL_SR8:
    case GL_SRG8:
    case GL_SRGB8:
    case GL_SRGB8_ALPHA8: return Type::UINT8;

    case GL_R8_SNORM:
    case GL_RG8_SNORM:
    case GL_RGB8_SNORM:
    case GL_RGBA8_SNORM:

    case GL_R8I:
    case GL_RG8I:
    case GL_RGB8I:
    case GL_RGBA8I: return Type::INT8;

    case GL_R16:
    case GL_RG16:
    case GL_RGB16:
    case GL_RGBA16:

    case GL_R16UI:
    case GL_RG16UI:
    case GL_RGB16UI:
    case GL_RGBA16UI: return Type::UINT16;

    case GL_R16_SNORM:
    case GL_RG16_SNORM:
    case GL_RGB16_SNORM:
    case GL_RGBA16_SNORM:

    case GL_R16I:
    case GL_RG16I:
    case GL_RGB16I:
    case GL_RGBA16I: return Type::INT16;

    case GL_R16F:
    case GL_RG16F:
    case GL_RGB16F:
    case GL_RGBA16F: return Type::HALF;

    case GL_R32UI:
    case GL_RG32UI:
    case GL_RGB32UI:
    case GL_RGBA32UI: return Type::UINT32;

    case GL_R32I:
    case GL_RG32I:
    case GL_RGB32I:
    case GL_RGBA32I: return Type::INT32;

    case GL_R32F:
    case GL_RG32F:
    case GL_RGB32F:
    case GL_RGBA32F: return Type::FLOAT;

    default: return Type::UNKNOWN;
    }
}

glDataDescriptor
get_format_descriptor(ktx_uint32_t glFormat, ktx_uint32_t glInternalFormat)
{
    using Space = glColorspace;

    glDataDescriptor format {};

    format.channels_count = get_format_channels(glFormat);
    format.colorspace     = get_format_colorspace(glInternalFormat);
    format.type           = get_format_datatype(glInternalFormat);

    return format;
}

glDataDescriptor
get_vk_format_descriptor(ktx_uint32_t vkFormat)
{
    // just convert it to gl type, they are mostly compatible with each other
    ktx_uint32_t glFormat         = vkFormat2glFormat((VkFormat)vkFormat);
    ktx_uint32_t glInternalFormat = vkFormat2glInternalFormat(
        (VkFormat)vkFormat);

    return get_format_descriptor(glFormat, glInternalFormat);
}

}  // namespace ktx
OIIO_NAMESPACE_END
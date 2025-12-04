// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#include "format.h"
#include "gl/gl_format.h"

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
glDataDescriptor
get_format_descriptor(ktx_uint32_t glFormat, ktx_uint32_t glInternalFormat)
{
    using Type  = TypeDesc::BASETYPE;
    using Space = glColorspace;

    glDataDescriptor format {};

    format.channels_count = get_format_channels(glFormat);

    switch (glInternalFormat) {
    // Unsigned Byte / Linear Colorspace
    case GL_R8:
    case GL_R8UI:
    case GL_RG8:
    case GL_RG8UI:
    case GL_RGB8:
    case GL_RGB8UI:
    case GL_RGBA8:
    case GL_RGBA8UI:
        format.type       = Type::UINT8;
        format.colorspace = Space::Linear;
        break;

    // Signed Byte / Linear Colorspace
    case GL_R8_SNORM:
    case GL_RG8_SNORM:
    case GL_RGB8_SNORM:
    case GL_RGBA8_SNORM:
        format.type       = Type::INT8;
        format.colorspace = Space::Linear;
        break;

    // Unsigned Byte / sRGB Colorspace
    case GL_SR8:
    case GL_SRG8:
    case GL_SRGB8:
    case GL_SRGB8_ALPHA8:
        format.type       = Type::UINT8;
        format.colorspace = Space::sRGB;
        break;

    default: 
        format.type = Type::UNKNOWN; 
        break;
    }

    return format;
}
}  // namespace ktx
OIIO_NAMESPACE_END
// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO

#pragma once

#include <OpenImageIO/typedesc.h>
#include <ktx.h>

OIIO_NAMESPACE_BEGIN
namespace ktx {

enum class glColorspace { Linear = 0, sRGB };

enum class Compression { None = 0 };

struct glDataDescriptor {
    ///< Texture is depth or stencil data format
    bool is_volume_texture = false;

    ///< The name of pixel data compression.
    Compression compression = Compression::None;

    ///< Color space that the format has. Basically just Linear or sRGB.
    glColorspace colorspace = glColorspace::Linear;

    ///< The number of color channels that the format has.
    int channels_count = 0;

    ///< Data format of the channels.
    TypeDesc::BASETYPE type = TypeDesc::BASETYPE::UNKNOWN;

    ///< The names of each channel, in order. Typically this will be "R",
    ///< "G", "B", "A" (alpha), "Z" (depth), or other arbitrary names.
    std::vector<std::string> channel_order;
};

/// @param glFormat
///		glFormat or glBaseInternalFormat of Khronos texture
/// @return
///		Number of color channels for gl format
int
get_format_channels(ktx_uint32_t glFormat);

/// @param glFormat
///		glFormat or glBaseInternalFormat of Khronos texture
/// @param glInternalFormat
///		glInternalFormat of Khronos Texture
/// @return
///		Info about requested formats
glDataDescriptor
get_format_descriptor(ktx_uint32_t glFormat, ktx_uint32_t glInternalFormat);

/// @param VkFormat
///		vkFormat of Khronos V2 texture
/// @return
///		Info about requested format
glDataDescriptor
get_vk_format_descriptor(ktx_uint32_t vkFormat);

}  // namespace ktx
OIIO_NAMESPACE_END